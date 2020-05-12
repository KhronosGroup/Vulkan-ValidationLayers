// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See best_practices_generator.py for modifications


/***************************************************************************
 *
 * Copyright (c) 2015-2020 The Khronos Group Inc.
 * Copyright (c) 2015-2020 Valve Corporation
 * Copyright (c) 2015-2020 LunarG, Inc.
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
 * Author: Mark Lobodzinski <mark@lunarg.com>
 *
 ****************************************************************************/


#include "best_practices.h"

bool BestPractices::PreCallValidateCreateInstance(
    const VkInstanceCreateInfo*                 pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkInstance*                                 pInstance) const {
    bool skip = ValidationStateTracker::PreCallValidateCreateInstance(pCreateInfo, pAllocator, pInstance);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCreateInstance(pCreateInfo, pAllocator, pInstance); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCreateInstance(
    const VkInstanceCreateInfo*                 pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkInstance*                                 pInstance) {
    ValidationStateTracker::PreCallRecordCreateInstance(pCreateInfo, pAllocator, pInstance);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCreateInstance(pCreateInfo, pAllocator, pInstance); });
}

void BestPractices::PostCallRecordCreateInstance(
    const VkInstanceCreateInfo*                 pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkInstance*                                 pInstance,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCreateInstance(pCreateInfo, pAllocator, pInstance, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCreateInstance(pCreateInfo, pAllocator, pInstance, result); });
}

bool BestPractices::PreCallValidateDestroyInstance(
    VkInstance                                  instance,
    const VkAllocationCallbacks*                pAllocator) const {
    bool skip = ValidationStateTracker::PreCallValidateDestroyInstance(instance, pAllocator);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateDestroyInstance(instance, pAllocator); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordDestroyInstance(
    VkInstance                                  instance,
    const VkAllocationCallbacks*                pAllocator) {
    ValidationStateTracker::PreCallRecordDestroyInstance(instance, pAllocator);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordDestroyInstance(instance, pAllocator); });
}

void BestPractices::PostCallRecordDestroyInstance(
    VkInstance                                  instance,
    const VkAllocationCallbacks*                pAllocator) {
    ValidationStateTracker::PostCallRecordDestroyInstance(instance, pAllocator);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordDestroyInstance(instance, pAllocator); });
}

bool BestPractices::PreCallValidateEnumeratePhysicalDevices(
    VkInstance                                  instance,
    uint32_t*                                   pPhysicalDeviceCount,
    VkPhysicalDevice*                           pPhysicalDevices) const {
    bool skip = ValidationStateTracker::PreCallValidateEnumeratePhysicalDevices(instance, pPhysicalDeviceCount, pPhysicalDevices);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateEnumeratePhysicalDevices(instance, pPhysicalDeviceCount, pPhysicalDevices); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordEnumeratePhysicalDevices(
    VkInstance                                  instance,
    uint32_t*                                   pPhysicalDeviceCount,
    VkPhysicalDevice*                           pPhysicalDevices) {
    ValidationStateTracker::PreCallRecordEnumeratePhysicalDevices(instance, pPhysicalDeviceCount, pPhysicalDevices);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordEnumeratePhysicalDevices(instance, pPhysicalDeviceCount, pPhysicalDevices); });
}

void BestPractices::PostCallRecordEnumeratePhysicalDevices(
    VkInstance                                  instance,
    uint32_t*                                   pPhysicalDeviceCount,
    VkPhysicalDevice*                           pPhysicalDevices,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordEnumeratePhysicalDevices(instance, pPhysicalDeviceCount, pPhysicalDevices, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordEnumeratePhysicalDevices(instance, pPhysicalDeviceCount, pPhysicalDevices, result); });
}

bool BestPractices::PreCallValidateGetPhysicalDeviceFeatures(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceFeatures*                   pFeatures) const {
    bool skip = ValidationStateTracker::PreCallValidateGetPhysicalDeviceFeatures(physicalDevice, pFeatures);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetPhysicalDeviceFeatures(physicalDevice, pFeatures); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetPhysicalDeviceFeatures(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceFeatures*                   pFeatures) {
    ValidationStateTracker::PreCallRecordGetPhysicalDeviceFeatures(physicalDevice, pFeatures);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetPhysicalDeviceFeatures(physicalDevice, pFeatures); });
}

void BestPractices::PostCallRecordGetPhysicalDeviceFeatures(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceFeatures*                   pFeatures) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceFeatures(physicalDevice, pFeatures);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetPhysicalDeviceFeatures(physicalDevice, pFeatures); });
}

bool BestPractices::PreCallValidateGetPhysicalDeviceFormatProperties(
    VkPhysicalDevice                            physicalDevice,
    VkFormat                                    format,
    VkFormatProperties*                         pFormatProperties) const {
    bool skip = ValidationStateTracker::PreCallValidateGetPhysicalDeviceFormatProperties(physicalDevice, format, pFormatProperties);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetPhysicalDeviceFormatProperties(physicalDevice, format, pFormatProperties); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetPhysicalDeviceFormatProperties(
    VkPhysicalDevice                            physicalDevice,
    VkFormat                                    format,
    VkFormatProperties*                         pFormatProperties) {
    ValidationStateTracker::PreCallRecordGetPhysicalDeviceFormatProperties(physicalDevice, format, pFormatProperties);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetPhysicalDeviceFormatProperties(physicalDevice, format, pFormatProperties); });
}

void BestPractices::PostCallRecordGetPhysicalDeviceFormatProperties(
    VkPhysicalDevice                            physicalDevice,
    VkFormat                                    format,
    VkFormatProperties*                         pFormatProperties) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceFormatProperties(physicalDevice, format, pFormatProperties);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetPhysicalDeviceFormatProperties(physicalDevice, format, pFormatProperties); });
}

bool BestPractices::PreCallValidateGetPhysicalDeviceImageFormatProperties(
    VkPhysicalDevice                            physicalDevice,
    VkFormat                                    format,
    VkImageType                                 type,
    VkImageTiling                               tiling,
    VkImageUsageFlags                           usage,
    VkImageCreateFlags                          flags,
    VkImageFormatProperties*                    pImageFormatProperties) const {
    bool skip = ValidationStateTracker::PreCallValidateGetPhysicalDeviceImageFormatProperties(physicalDevice, format, type, tiling, usage, flags, pImageFormatProperties);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetPhysicalDeviceImageFormatProperties(physicalDevice, format, type, tiling, usage, flags, pImageFormatProperties); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetPhysicalDeviceImageFormatProperties(
    VkPhysicalDevice                            physicalDevice,
    VkFormat                                    format,
    VkImageType                                 type,
    VkImageTiling                               tiling,
    VkImageUsageFlags                           usage,
    VkImageCreateFlags                          flags,
    VkImageFormatProperties*                    pImageFormatProperties) {
    ValidationStateTracker::PreCallRecordGetPhysicalDeviceImageFormatProperties(physicalDevice, format, type, tiling, usage, flags, pImageFormatProperties);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetPhysicalDeviceImageFormatProperties(physicalDevice, format, type, tiling, usage, flags, pImageFormatProperties); });
}

void BestPractices::PostCallRecordGetPhysicalDeviceImageFormatProperties(
    VkPhysicalDevice                            physicalDevice,
    VkFormat                                    format,
    VkImageType                                 type,
    VkImageTiling                               tiling,
    VkImageUsageFlags                           usage,
    VkImageCreateFlags                          flags,
    VkImageFormatProperties*                    pImageFormatProperties,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceImageFormatProperties(physicalDevice, format, type, tiling, usage, flags, pImageFormatProperties, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetPhysicalDeviceImageFormatProperties(physicalDevice, format, type, tiling, usage, flags, pImageFormatProperties, result); });
}

bool BestPractices::PreCallValidateGetPhysicalDeviceProperties(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceProperties*                 pProperties) const {
    bool skip = ValidationStateTracker::PreCallValidateGetPhysicalDeviceProperties(physicalDevice, pProperties);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetPhysicalDeviceProperties(physicalDevice, pProperties); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetPhysicalDeviceProperties(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceProperties*                 pProperties) {
    ValidationStateTracker::PreCallRecordGetPhysicalDeviceProperties(physicalDevice, pProperties);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetPhysicalDeviceProperties(physicalDevice, pProperties); });
}

void BestPractices::PostCallRecordGetPhysicalDeviceProperties(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceProperties*                 pProperties) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceProperties(physicalDevice, pProperties);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetPhysicalDeviceProperties(physicalDevice, pProperties); });
}

bool BestPractices::PreCallValidateGetPhysicalDeviceQueueFamilyProperties(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pQueueFamilyPropertyCount,
    VkQueueFamilyProperties*                    pQueueFamilyProperties) const {
    bool skip = ValidationStateTracker::PreCallValidateGetPhysicalDeviceQueueFamilyProperties(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetPhysicalDeviceQueueFamilyProperties(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetPhysicalDeviceQueueFamilyProperties(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pQueueFamilyPropertyCount,
    VkQueueFamilyProperties*                    pQueueFamilyProperties) {
    ValidationStateTracker::PreCallRecordGetPhysicalDeviceQueueFamilyProperties(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetPhysicalDeviceQueueFamilyProperties(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties); });
}

void BestPractices::PostCallRecordGetPhysicalDeviceQueueFamilyProperties(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pQueueFamilyPropertyCount,
    VkQueueFamilyProperties*                    pQueueFamilyProperties) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceQueueFamilyProperties(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetPhysicalDeviceQueueFamilyProperties(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties); });
}

bool BestPractices::PreCallValidateGetPhysicalDeviceMemoryProperties(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceMemoryProperties*           pMemoryProperties) const {
    bool skip = ValidationStateTracker::PreCallValidateGetPhysicalDeviceMemoryProperties(physicalDevice, pMemoryProperties);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetPhysicalDeviceMemoryProperties(physicalDevice, pMemoryProperties); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetPhysicalDeviceMemoryProperties(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceMemoryProperties*           pMemoryProperties) {
    ValidationStateTracker::PreCallRecordGetPhysicalDeviceMemoryProperties(physicalDevice, pMemoryProperties);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetPhysicalDeviceMemoryProperties(physicalDevice, pMemoryProperties); });
}

void BestPractices::PostCallRecordGetPhysicalDeviceMemoryProperties(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceMemoryProperties*           pMemoryProperties) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceMemoryProperties(physicalDevice, pMemoryProperties);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetPhysicalDeviceMemoryProperties(physicalDevice, pMemoryProperties); });
}

bool BestPractices::PreCallValidateGetInstanceProcAddr(
    VkInstance                                  instance,
    const char*                                 pName) const {
    bool skip = ValidationStateTracker::PreCallValidateGetInstanceProcAddr(instance, pName);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetInstanceProcAddr(instance, pName); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetInstanceProcAddr(
    VkInstance                                  instance,
    const char*                                 pName) {
    ValidationStateTracker::PreCallRecordGetInstanceProcAddr(instance, pName);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetInstanceProcAddr(instance, pName); });
}

void BestPractices::PostCallRecordGetInstanceProcAddr(
    VkInstance                                  instance,
    const char*                                 pName) {
    ValidationStateTracker::PostCallRecordGetInstanceProcAddr(instance, pName);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetInstanceProcAddr(instance, pName); });
}

bool BestPractices::PreCallValidateGetDeviceProcAddr(
    VkDevice                                    device,
    const char*                                 pName) const {
    bool skip = ValidationStateTracker::PreCallValidateGetDeviceProcAddr(device, pName);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetDeviceProcAddr(device, pName); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetDeviceProcAddr(
    VkDevice                                    device,
    const char*                                 pName) {
    ValidationStateTracker::PreCallRecordGetDeviceProcAddr(device, pName);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetDeviceProcAddr(device, pName); });
}

void BestPractices::PostCallRecordGetDeviceProcAddr(
    VkDevice                                    device,
    const char*                                 pName) {
    ValidationStateTracker::PostCallRecordGetDeviceProcAddr(device, pName);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetDeviceProcAddr(device, pName); });
}

bool BestPractices::PreCallValidateCreateDevice(
    VkPhysicalDevice                            physicalDevice,
    const VkDeviceCreateInfo*                   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDevice*                                   pDevice) const {
    bool skip = ValidationStateTracker::PreCallValidateCreateDevice(physicalDevice, pCreateInfo, pAllocator, pDevice);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCreateDevice(physicalDevice, pCreateInfo, pAllocator, pDevice); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCreateDevice(
    VkPhysicalDevice                            physicalDevice,
    const VkDeviceCreateInfo*                   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDevice*                                   pDevice) {
    ValidationStateTracker::PreCallRecordCreateDevice(physicalDevice, pCreateInfo, pAllocator, pDevice);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCreateDevice(physicalDevice, pCreateInfo, pAllocator, pDevice); });
}

void BestPractices::PostCallRecordCreateDevice(
    VkPhysicalDevice                            physicalDevice,
    const VkDeviceCreateInfo*                   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDevice*                                   pDevice,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCreateDevice(physicalDevice, pCreateInfo, pAllocator, pDevice, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCreateDevice(physicalDevice, pCreateInfo, pAllocator, pDevice, result); });
}

bool BestPractices::PreCallValidateDestroyDevice(
    VkDevice                                    device,
    const VkAllocationCallbacks*                pAllocator) const {
    bool skip = ValidationStateTracker::PreCallValidateDestroyDevice(device, pAllocator);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateDestroyDevice(device, pAllocator); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordDestroyDevice(
    VkDevice                                    device,
    const VkAllocationCallbacks*                pAllocator) {
    ValidationStateTracker::PreCallRecordDestroyDevice(device, pAllocator);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordDestroyDevice(device, pAllocator); });
}

void BestPractices::PostCallRecordDestroyDevice(
    VkDevice                                    device,
    const VkAllocationCallbacks*                pAllocator) {
    ValidationStateTracker::PostCallRecordDestroyDevice(device, pAllocator);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordDestroyDevice(device, pAllocator); });
}

bool BestPractices::PreCallValidateEnumerateInstanceExtensionProperties(
    const char*                                 pLayerName,
    uint32_t*                                   pPropertyCount,
    VkExtensionProperties*                      pProperties) const {
    bool skip = ValidationStateTracker::PreCallValidateEnumerateInstanceExtensionProperties(pLayerName, pPropertyCount, pProperties);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateEnumerateInstanceExtensionProperties(pLayerName, pPropertyCount, pProperties); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordEnumerateInstanceExtensionProperties(
    const char*                                 pLayerName,
    uint32_t*                                   pPropertyCount,
    VkExtensionProperties*                      pProperties) {
    ValidationStateTracker::PreCallRecordEnumerateInstanceExtensionProperties(pLayerName, pPropertyCount, pProperties);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordEnumerateInstanceExtensionProperties(pLayerName, pPropertyCount, pProperties); });
}

void BestPractices::PostCallRecordEnumerateInstanceExtensionProperties(
    const char*                                 pLayerName,
    uint32_t*                                   pPropertyCount,
    VkExtensionProperties*                      pProperties,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordEnumerateInstanceExtensionProperties(pLayerName, pPropertyCount, pProperties, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordEnumerateInstanceExtensionProperties(pLayerName, pPropertyCount, pProperties, result); });
}

bool BestPractices::PreCallValidateEnumerateDeviceExtensionProperties(
    VkPhysicalDevice                            physicalDevice,
    const char*                                 pLayerName,
    uint32_t*                                   pPropertyCount,
    VkExtensionProperties*                      pProperties) const {
    bool skip = ValidationStateTracker::PreCallValidateEnumerateDeviceExtensionProperties(physicalDevice, pLayerName, pPropertyCount, pProperties);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateEnumerateDeviceExtensionProperties(physicalDevice, pLayerName, pPropertyCount, pProperties); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordEnumerateDeviceExtensionProperties(
    VkPhysicalDevice                            physicalDevice,
    const char*                                 pLayerName,
    uint32_t*                                   pPropertyCount,
    VkExtensionProperties*                      pProperties) {
    ValidationStateTracker::PreCallRecordEnumerateDeviceExtensionProperties(physicalDevice, pLayerName, pPropertyCount, pProperties);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordEnumerateDeviceExtensionProperties(physicalDevice, pLayerName, pPropertyCount, pProperties); });
}

void BestPractices::PostCallRecordEnumerateDeviceExtensionProperties(
    VkPhysicalDevice                            physicalDevice,
    const char*                                 pLayerName,
    uint32_t*                                   pPropertyCount,
    VkExtensionProperties*                      pProperties,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordEnumerateDeviceExtensionProperties(physicalDevice, pLayerName, pPropertyCount, pProperties, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordEnumerateDeviceExtensionProperties(physicalDevice, pLayerName, pPropertyCount, pProperties, result); });
}

bool BestPractices::PreCallValidateEnumerateInstanceLayerProperties(
    uint32_t*                                   pPropertyCount,
    VkLayerProperties*                          pProperties) const {
    bool skip = ValidationStateTracker::PreCallValidateEnumerateInstanceLayerProperties(pPropertyCount, pProperties);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateEnumerateInstanceLayerProperties(pPropertyCount, pProperties); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordEnumerateInstanceLayerProperties(
    uint32_t*                                   pPropertyCount,
    VkLayerProperties*                          pProperties) {
    ValidationStateTracker::PreCallRecordEnumerateInstanceLayerProperties(pPropertyCount, pProperties);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordEnumerateInstanceLayerProperties(pPropertyCount, pProperties); });
}

void BestPractices::PostCallRecordEnumerateInstanceLayerProperties(
    uint32_t*                                   pPropertyCount,
    VkLayerProperties*                          pProperties,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordEnumerateInstanceLayerProperties(pPropertyCount, pProperties, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordEnumerateInstanceLayerProperties(pPropertyCount, pProperties, result); });
}

bool BestPractices::PreCallValidateEnumerateDeviceLayerProperties(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pPropertyCount,
    VkLayerProperties*                          pProperties) const {
    bool skip = ValidationStateTracker::PreCallValidateEnumerateDeviceLayerProperties(physicalDevice, pPropertyCount, pProperties);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateEnumerateDeviceLayerProperties(physicalDevice, pPropertyCount, pProperties); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordEnumerateDeviceLayerProperties(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pPropertyCount,
    VkLayerProperties*                          pProperties) {
    ValidationStateTracker::PreCallRecordEnumerateDeviceLayerProperties(physicalDevice, pPropertyCount, pProperties);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordEnumerateDeviceLayerProperties(physicalDevice, pPropertyCount, pProperties); });
}

void BestPractices::PostCallRecordEnumerateDeviceLayerProperties(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pPropertyCount,
    VkLayerProperties*                          pProperties,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordEnumerateDeviceLayerProperties(physicalDevice, pPropertyCount, pProperties, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordEnumerateDeviceLayerProperties(physicalDevice, pPropertyCount, pProperties, result); });
}

bool BestPractices::PreCallValidateGetDeviceQueue(
    VkDevice                                    device,
    uint32_t                                    queueFamilyIndex,
    uint32_t                                    queueIndex,
    VkQueue*                                    pQueue) const {
    bool skip = ValidationStateTracker::PreCallValidateGetDeviceQueue(device, queueFamilyIndex, queueIndex, pQueue);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetDeviceQueue(device, queueFamilyIndex, queueIndex, pQueue); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetDeviceQueue(
    VkDevice                                    device,
    uint32_t                                    queueFamilyIndex,
    uint32_t                                    queueIndex,
    VkQueue*                                    pQueue) {
    ValidationStateTracker::PreCallRecordGetDeviceQueue(device, queueFamilyIndex, queueIndex, pQueue);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetDeviceQueue(device, queueFamilyIndex, queueIndex, pQueue); });
}

void BestPractices::PostCallRecordGetDeviceQueue(
    VkDevice                                    device,
    uint32_t                                    queueFamilyIndex,
    uint32_t                                    queueIndex,
    VkQueue*                                    pQueue) {
    ValidationStateTracker::PostCallRecordGetDeviceQueue(device, queueFamilyIndex, queueIndex, pQueue);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetDeviceQueue(device, queueFamilyIndex, queueIndex, pQueue); });
}

bool BestPractices::PreCallValidateQueueSubmit(
    VkQueue                                     queue,
    uint32_t                                    submitCount,
    const VkSubmitInfo*                         pSubmits,
    VkFence                                     fence) const {
    bool skip = ValidationStateTracker::PreCallValidateQueueSubmit(queue, submitCount, pSubmits, fence);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateQueueSubmit(queue, submitCount, pSubmits, fence); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordQueueSubmit(
    VkQueue                                     queue,
    uint32_t                                    submitCount,
    const VkSubmitInfo*                         pSubmits,
    VkFence                                     fence) {
    ValidationStateTracker::PreCallRecordQueueSubmit(queue, submitCount, pSubmits, fence);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordQueueSubmit(queue, submitCount, pSubmits, fence); });
}

void BestPractices::PostCallRecordQueueSubmit(
    VkQueue                                     queue,
    uint32_t                                    submitCount,
    const VkSubmitInfo*                         pSubmits,
    VkFence                                     fence,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordQueueSubmit(queue, submitCount, pSubmits, fence, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordQueueSubmit(queue, submitCount, pSubmits, fence, result); });
}

bool BestPractices::PreCallValidateQueueWaitIdle(
    VkQueue                                     queue) const {
    bool skip = ValidationStateTracker::PreCallValidateQueueWaitIdle(queue);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateQueueWaitIdle(queue); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordQueueWaitIdle(
    VkQueue                                     queue) {
    ValidationStateTracker::PreCallRecordQueueWaitIdle(queue);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordQueueWaitIdle(queue); });
}

void BestPractices::PostCallRecordQueueWaitIdle(
    VkQueue                                     queue,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordQueueWaitIdle(queue, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordQueueWaitIdle(queue, result); });
}

bool BestPractices::PreCallValidateDeviceWaitIdle(
    VkDevice                                    device) const {
    bool skip = ValidationStateTracker::PreCallValidateDeviceWaitIdle(device);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateDeviceWaitIdle(device); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordDeviceWaitIdle(
    VkDevice                                    device) {
    ValidationStateTracker::PreCallRecordDeviceWaitIdle(device);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordDeviceWaitIdle(device); });
}

void BestPractices::PostCallRecordDeviceWaitIdle(
    VkDevice                                    device,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordDeviceWaitIdle(device, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordDeviceWaitIdle(device, result); });
}

bool BestPractices::PreCallValidateAllocateMemory(
    VkDevice                                    device,
    const VkMemoryAllocateInfo*                 pAllocateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDeviceMemory*                             pMemory) const {
    bool skip = ValidationStateTracker::PreCallValidateAllocateMemory(device, pAllocateInfo, pAllocator, pMemory);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateAllocateMemory(device, pAllocateInfo, pAllocator, pMemory); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordAllocateMemory(
    VkDevice                                    device,
    const VkMemoryAllocateInfo*                 pAllocateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDeviceMemory*                             pMemory) {
    ValidationStateTracker::PreCallRecordAllocateMemory(device, pAllocateInfo, pAllocator, pMemory);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordAllocateMemory(device, pAllocateInfo, pAllocator, pMemory); });
}

void BestPractices::PostCallRecordAllocateMemory(
    VkDevice                                    device,
    const VkMemoryAllocateInfo*                 pAllocateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDeviceMemory*                             pMemory,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordAllocateMemory(device, pAllocateInfo, pAllocator, pMemory, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordAllocateMemory(device, pAllocateInfo, pAllocator, pMemory, result); });
}

bool BestPractices::PreCallValidateFreeMemory(
    VkDevice                                    device,
    VkDeviceMemory                              memory,
    const VkAllocationCallbacks*                pAllocator) const {
    bool skip = ValidationStateTracker::PreCallValidateFreeMemory(device, memory, pAllocator);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateFreeMemory(device, memory, pAllocator); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordFreeMemory(
    VkDevice                                    device,
    VkDeviceMemory                              memory,
    const VkAllocationCallbacks*                pAllocator) {
    ValidationStateTracker::PreCallRecordFreeMemory(device, memory, pAllocator);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordFreeMemory(device, memory, pAllocator); });
}

void BestPractices::PostCallRecordFreeMemory(
    VkDevice                                    device,
    VkDeviceMemory                              memory,
    const VkAllocationCallbacks*                pAllocator) {
    ValidationStateTracker::PostCallRecordFreeMemory(device, memory, pAllocator);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordFreeMemory(device, memory, pAllocator); });
}

bool BestPractices::PreCallValidateMapMemory(
    VkDevice                                    device,
    VkDeviceMemory                              memory,
    VkDeviceSize                                offset,
    VkDeviceSize                                size,
    VkMemoryMapFlags                            flags,
    void**                                      ppData) const {
    bool skip = ValidationStateTracker::PreCallValidateMapMemory(device, memory, offset, size, flags, ppData);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateMapMemory(device, memory, offset, size, flags, ppData); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordMapMemory(
    VkDevice                                    device,
    VkDeviceMemory                              memory,
    VkDeviceSize                                offset,
    VkDeviceSize                                size,
    VkMemoryMapFlags                            flags,
    void**                                      ppData) {
    ValidationStateTracker::PreCallRecordMapMemory(device, memory, offset, size, flags, ppData);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordMapMemory(device, memory, offset, size, flags, ppData); });
}

void BestPractices::PostCallRecordMapMemory(
    VkDevice                                    device,
    VkDeviceMemory                              memory,
    VkDeviceSize                                offset,
    VkDeviceSize                                size,
    VkMemoryMapFlags                            flags,
    void**                                      ppData,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordMapMemory(device, memory, offset, size, flags, ppData, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordMapMemory(device, memory, offset, size, flags, ppData, result); });
}

bool BestPractices::PreCallValidateUnmapMemory(
    VkDevice                                    device,
    VkDeviceMemory                              memory) const {
    bool skip = ValidationStateTracker::PreCallValidateUnmapMemory(device, memory);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateUnmapMemory(device, memory); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordUnmapMemory(
    VkDevice                                    device,
    VkDeviceMemory                              memory) {
    ValidationStateTracker::PreCallRecordUnmapMemory(device, memory);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordUnmapMemory(device, memory); });
}

void BestPractices::PostCallRecordUnmapMemory(
    VkDevice                                    device,
    VkDeviceMemory                              memory) {
    ValidationStateTracker::PostCallRecordUnmapMemory(device, memory);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordUnmapMemory(device, memory); });
}

bool BestPractices::PreCallValidateFlushMappedMemoryRanges(
    VkDevice                                    device,
    uint32_t                                    memoryRangeCount,
    const VkMappedMemoryRange*                  pMemoryRanges) const {
    bool skip = ValidationStateTracker::PreCallValidateFlushMappedMemoryRanges(device, memoryRangeCount, pMemoryRanges);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateFlushMappedMemoryRanges(device, memoryRangeCount, pMemoryRanges); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordFlushMappedMemoryRanges(
    VkDevice                                    device,
    uint32_t                                    memoryRangeCount,
    const VkMappedMemoryRange*                  pMemoryRanges) {
    ValidationStateTracker::PreCallRecordFlushMappedMemoryRanges(device, memoryRangeCount, pMemoryRanges);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordFlushMappedMemoryRanges(device, memoryRangeCount, pMemoryRanges); });
}

void BestPractices::PostCallRecordFlushMappedMemoryRanges(
    VkDevice                                    device,
    uint32_t                                    memoryRangeCount,
    const VkMappedMemoryRange*                  pMemoryRanges,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordFlushMappedMemoryRanges(device, memoryRangeCount, pMemoryRanges, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordFlushMappedMemoryRanges(device, memoryRangeCount, pMemoryRanges, result); });
}

bool BestPractices::PreCallValidateInvalidateMappedMemoryRanges(
    VkDevice                                    device,
    uint32_t                                    memoryRangeCount,
    const VkMappedMemoryRange*                  pMemoryRanges) const {
    bool skip = ValidationStateTracker::PreCallValidateInvalidateMappedMemoryRanges(device, memoryRangeCount, pMemoryRanges);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateInvalidateMappedMemoryRanges(device, memoryRangeCount, pMemoryRanges); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordInvalidateMappedMemoryRanges(
    VkDevice                                    device,
    uint32_t                                    memoryRangeCount,
    const VkMappedMemoryRange*                  pMemoryRanges) {
    ValidationStateTracker::PreCallRecordInvalidateMappedMemoryRanges(device, memoryRangeCount, pMemoryRanges);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordInvalidateMappedMemoryRanges(device, memoryRangeCount, pMemoryRanges); });
}

void BestPractices::PostCallRecordInvalidateMappedMemoryRanges(
    VkDevice                                    device,
    uint32_t                                    memoryRangeCount,
    const VkMappedMemoryRange*                  pMemoryRanges,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordInvalidateMappedMemoryRanges(device, memoryRangeCount, pMemoryRanges, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordInvalidateMappedMemoryRanges(device, memoryRangeCount, pMemoryRanges, result); });
}

bool BestPractices::PreCallValidateGetDeviceMemoryCommitment(
    VkDevice                                    device,
    VkDeviceMemory                              memory,
    VkDeviceSize*                               pCommittedMemoryInBytes) const {
    bool skip = ValidationStateTracker::PreCallValidateGetDeviceMemoryCommitment(device, memory, pCommittedMemoryInBytes);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetDeviceMemoryCommitment(device, memory, pCommittedMemoryInBytes); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetDeviceMemoryCommitment(
    VkDevice                                    device,
    VkDeviceMemory                              memory,
    VkDeviceSize*                               pCommittedMemoryInBytes) {
    ValidationStateTracker::PreCallRecordGetDeviceMemoryCommitment(device, memory, pCommittedMemoryInBytes);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetDeviceMemoryCommitment(device, memory, pCommittedMemoryInBytes); });
}

void BestPractices::PostCallRecordGetDeviceMemoryCommitment(
    VkDevice                                    device,
    VkDeviceMemory                              memory,
    VkDeviceSize*                               pCommittedMemoryInBytes) {
    ValidationStateTracker::PostCallRecordGetDeviceMemoryCommitment(device, memory, pCommittedMemoryInBytes);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetDeviceMemoryCommitment(device, memory, pCommittedMemoryInBytes); });
}

bool BestPractices::PreCallValidateBindBufferMemory(
    VkDevice                                    device,
    VkBuffer                                    buffer,
    VkDeviceMemory                              memory,
    VkDeviceSize                                memoryOffset) const {
    bool skip = ValidationStateTracker::PreCallValidateBindBufferMemory(device, buffer, memory, memoryOffset);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateBindBufferMemory(device, buffer, memory, memoryOffset); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordBindBufferMemory(
    VkDevice                                    device,
    VkBuffer                                    buffer,
    VkDeviceMemory                              memory,
    VkDeviceSize                                memoryOffset) {
    ValidationStateTracker::PreCallRecordBindBufferMemory(device, buffer, memory, memoryOffset);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordBindBufferMemory(device, buffer, memory, memoryOffset); });
}

void BestPractices::PostCallRecordBindBufferMemory(
    VkDevice                                    device,
    VkBuffer                                    buffer,
    VkDeviceMemory                              memory,
    VkDeviceSize                                memoryOffset,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordBindBufferMemory(device, buffer, memory, memoryOffset, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordBindBufferMemory(device, buffer, memory, memoryOffset, result); });
}

bool BestPractices::PreCallValidateBindImageMemory(
    VkDevice                                    device,
    VkImage                                     image,
    VkDeviceMemory                              memory,
    VkDeviceSize                                memoryOffset) const {
    bool skip = ValidationStateTracker::PreCallValidateBindImageMemory(device, image, memory, memoryOffset);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateBindImageMemory(device, image, memory, memoryOffset); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordBindImageMemory(
    VkDevice                                    device,
    VkImage                                     image,
    VkDeviceMemory                              memory,
    VkDeviceSize                                memoryOffset) {
    ValidationStateTracker::PreCallRecordBindImageMemory(device, image, memory, memoryOffset);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordBindImageMemory(device, image, memory, memoryOffset); });
}

void BestPractices::PostCallRecordBindImageMemory(
    VkDevice                                    device,
    VkImage                                     image,
    VkDeviceMemory                              memory,
    VkDeviceSize                                memoryOffset,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordBindImageMemory(device, image, memory, memoryOffset, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordBindImageMemory(device, image, memory, memoryOffset, result); });
}

bool BestPractices::PreCallValidateGetBufferMemoryRequirements(
    VkDevice                                    device,
    VkBuffer                                    buffer,
    VkMemoryRequirements*                       pMemoryRequirements) const {
    bool skip = ValidationStateTracker::PreCallValidateGetBufferMemoryRequirements(device, buffer, pMemoryRequirements);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetBufferMemoryRequirements(device, buffer, pMemoryRequirements); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetBufferMemoryRequirements(
    VkDevice                                    device,
    VkBuffer                                    buffer,
    VkMemoryRequirements*                       pMemoryRequirements) {
    ValidationStateTracker::PreCallRecordGetBufferMemoryRequirements(device, buffer, pMemoryRequirements);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetBufferMemoryRequirements(device, buffer, pMemoryRequirements); });
}

void BestPractices::PostCallRecordGetBufferMemoryRequirements(
    VkDevice                                    device,
    VkBuffer                                    buffer,
    VkMemoryRequirements*                       pMemoryRequirements) {
    ValidationStateTracker::PostCallRecordGetBufferMemoryRequirements(device, buffer, pMemoryRequirements);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetBufferMemoryRequirements(device, buffer, pMemoryRequirements); });
}

bool BestPractices::PreCallValidateGetImageMemoryRequirements(
    VkDevice                                    device,
    VkImage                                     image,
    VkMemoryRequirements*                       pMemoryRequirements) const {
    bool skip = ValidationStateTracker::PreCallValidateGetImageMemoryRequirements(device, image, pMemoryRequirements);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetImageMemoryRequirements(device, image, pMemoryRequirements); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetImageMemoryRequirements(
    VkDevice                                    device,
    VkImage                                     image,
    VkMemoryRequirements*                       pMemoryRequirements) {
    ValidationStateTracker::PreCallRecordGetImageMemoryRequirements(device, image, pMemoryRequirements);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetImageMemoryRequirements(device, image, pMemoryRequirements); });
}

void BestPractices::PostCallRecordGetImageMemoryRequirements(
    VkDevice                                    device,
    VkImage                                     image,
    VkMemoryRequirements*                       pMemoryRequirements) {
    ValidationStateTracker::PostCallRecordGetImageMemoryRequirements(device, image, pMemoryRequirements);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetImageMemoryRequirements(device, image, pMemoryRequirements); });
}

bool BestPractices::PreCallValidateGetImageSparseMemoryRequirements(
    VkDevice                                    device,
    VkImage                                     image,
    uint32_t*                                   pSparseMemoryRequirementCount,
    VkSparseImageMemoryRequirements*            pSparseMemoryRequirements) const {
    bool skip = ValidationStateTracker::PreCallValidateGetImageSparseMemoryRequirements(device, image, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetImageSparseMemoryRequirements(device, image, pSparseMemoryRequirementCount, pSparseMemoryRequirements); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetImageSparseMemoryRequirements(
    VkDevice                                    device,
    VkImage                                     image,
    uint32_t*                                   pSparseMemoryRequirementCount,
    VkSparseImageMemoryRequirements*            pSparseMemoryRequirements) {
    ValidationStateTracker::PreCallRecordGetImageSparseMemoryRequirements(device, image, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetImageSparseMemoryRequirements(device, image, pSparseMemoryRequirementCount, pSparseMemoryRequirements); });
}

void BestPractices::PostCallRecordGetImageSparseMemoryRequirements(
    VkDevice                                    device,
    VkImage                                     image,
    uint32_t*                                   pSparseMemoryRequirementCount,
    VkSparseImageMemoryRequirements*            pSparseMemoryRequirements) {
    ValidationStateTracker::PostCallRecordGetImageSparseMemoryRequirements(device, image, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetImageSparseMemoryRequirements(device, image, pSparseMemoryRequirementCount, pSparseMemoryRequirements); });
}

bool BestPractices::PreCallValidateGetPhysicalDeviceSparseImageFormatProperties(
    VkPhysicalDevice                            physicalDevice,
    VkFormat                                    format,
    VkImageType                                 type,
    VkSampleCountFlagBits                       samples,
    VkImageUsageFlags                           usage,
    VkImageTiling                               tiling,
    uint32_t*                                   pPropertyCount,
    VkSparseImageFormatProperties*              pProperties) const {
    bool skip = ValidationStateTracker::PreCallValidateGetPhysicalDeviceSparseImageFormatProperties(physicalDevice, format, type, samples, usage, tiling, pPropertyCount, pProperties);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetPhysicalDeviceSparseImageFormatProperties(physicalDevice, format, type, samples, usage, tiling, pPropertyCount, pProperties); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetPhysicalDeviceSparseImageFormatProperties(
    VkPhysicalDevice                            physicalDevice,
    VkFormat                                    format,
    VkImageType                                 type,
    VkSampleCountFlagBits                       samples,
    VkImageUsageFlags                           usage,
    VkImageTiling                               tiling,
    uint32_t*                                   pPropertyCount,
    VkSparseImageFormatProperties*              pProperties) {
    ValidationStateTracker::PreCallRecordGetPhysicalDeviceSparseImageFormatProperties(physicalDevice, format, type, samples, usage, tiling, pPropertyCount, pProperties);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetPhysicalDeviceSparseImageFormatProperties(physicalDevice, format, type, samples, usage, tiling, pPropertyCount, pProperties); });
}

void BestPractices::PostCallRecordGetPhysicalDeviceSparseImageFormatProperties(
    VkPhysicalDevice                            physicalDevice,
    VkFormat                                    format,
    VkImageType                                 type,
    VkSampleCountFlagBits                       samples,
    VkImageUsageFlags                           usage,
    VkImageTiling                               tiling,
    uint32_t*                                   pPropertyCount,
    VkSparseImageFormatProperties*              pProperties) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceSparseImageFormatProperties(physicalDevice, format, type, samples, usage, tiling, pPropertyCount, pProperties);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetPhysicalDeviceSparseImageFormatProperties(physicalDevice, format, type, samples, usage, tiling, pPropertyCount, pProperties); });
}

bool BestPractices::PreCallValidateQueueBindSparse(
    VkQueue                                     queue,
    uint32_t                                    bindInfoCount,
    const VkBindSparseInfo*                     pBindInfo,
    VkFence                                     fence) const {
    bool skip = ValidationStateTracker::PreCallValidateQueueBindSparse(queue, bindInfoCount, pBindInfo, fence);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateQueueBindSparse(queue, bindInfoCount, pBindInfo, fence); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordQueueBindSparse(
    VkQueue                                     queue,
    uint32_t                                    bindInfoCount,
    const VkBindSparseInfo*                     pBindInfo,
    VkFence                                     fence) {
    ValidationStateTracker::PreCallRecordQueueBindSparse(queue, bindInfoCount, pBindInfo, fence);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordQueueBindSparse(queue, bindInfoCount, pBindInfo, fence); });
}

void BestPractices::PostCallRecordQueueBindSparse(
    VkQueue                                     queue,
    uint32_t                                    bindInfoCount,
    const VkBindSparseInfo*                     pBindInfo,
    VkFence                                     fence,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordQueueBindSparse(queue, bindInfoCount, pBindInfo, fence, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordQueueBindSparse(queue, bindInfoCount, pBindInfo, fence, result); });
}

bool BestPractices::PreCallValidateCreateFence(
    VkDevice                                    device,
    const VkFenceCreateInfo*                    pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkFence*                                    pFence) const {
    bool skip = ValidationStateTracker::PreCallValidateCreateFence(device, pCreateInfo, pAllocator, pFence);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCreateFence(device, pCreateInfo, pAllocator, pFence); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCreateFence(
    VkDevice                                    device,
    const VkFenceCreateInfo*                    pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkFence*                                    pFence) {
    ValidationStateTracker::PreCallRecordCreateFence(device, pCreateInfo, pAllocator, pFence);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCreateFence(device, pCreateInfo, pAllocator, pFence); });
}

void BestPractices::PostCallRecordCreateFence(
    VkDevice                                    device,
    const VkFenceCreateInfo*                    pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkFence*                                    pFence,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCreateFence(device, pCreateInfo, pAllocator, pFence, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCreateFence(device, pCreateInfo, pAllocator, pFence, result); });
}

bool BestPractices::PreCallValidateDestroyFence(
    VkDevice                                    device,
    VkFence                                     fence,
    const VkAllocationCallbacks*                pAllocator) const {
    bool skip = ValidationStateTracker::PreCallValidateDestroyFence(device, fence, pAllocator);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateDestroyFence(device, fence, pAllocator); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordDestroyFence(
    VkDevice                                    device,
    VkFence                                     fence,
    const VkAllocationCallbacks*                pAllocator) {
    ValidationStateTracker::PreCallRecordDestroyFence(device, fence, pAllocator);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordDestroyFence(device, fence, pAllocator); });
}

void BestPractices::PostCallRecordDestroyFence(
    VkDevice                                    device,
    VkFence                                     fence,
    const VkAllocationCallbacks*                pAllocator) {
    ValidationStateTracker::PostCallRecordDestroyFence(device, fence, pAllocator);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordDestroyFence(device, fence, pAllocator); });
}

bool BestPractices::PreCallValidateResetFences(
    VkDevice                                    device,
    uint32_t                                    fenceCount,
    const VkFence*                              pFences) const {
    bool skip = ValidationStateTracker::PreCallValidateResetFences(device, fenceCount, pFences);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateResetFences(device, fenceCount, pFences); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordResetFences(
    VkDevice                                    device,
    uint32_t                                    fenceCount,
    const VkFence*                              pFences) {
    ValidationStateTracker::PreCallRecordResetFences(device, fenceCount, pFences);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordResetFences(device, fenceCount, pFences); });
}

void BestPractices::PostCallRecordResetFences(
    VkDevice                                    device,
    uint32_t                                    fenceCount,
    const VkFence*                              pFences,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordResetFences(device, fenceCount, pFences, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordResetFences(device, fenceCount, pFences, result); });
}

bool BestPractices::PreCallValidateGetFenceStatus(
    VkDevice                                    device,
    VkFence                                     fence) const {
    bool skip = ValidationStateTracker::PreCallValidateGetFenceStatus(device, fence);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetFenceStatus(device, fence); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetFenceStatus(
    VkDevice                                    device,
    VkFence                                     fence) {
    ValidationStateTracker::PreCallRecordGetFenceStatus(device, fence);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetFenceStatus(device, fence); });
}

void BestPractices::PostCallRecordGetFenceStatus(
    VkDevice                                    device,
    VkFence                                     fence,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetFenceStatus(device, fence, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetFenceStatus(device, fence, result); });
}

bool BestPractices::PreCallValidateWaitForFences(
    VkDevice                                    device,
    uint32_t                                    fenceCount,
    const VkFence*                              pFences,
    VkBool32                                    waitAll,
    uint64_t                                    timeout) const {
    bool skip = ValidationStateTracker::PreCallValidateWaitForFences(device, fenceCount, pFences, waitAll, timeout);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateWaitForFences(device, fenceCount, pFences, waitAll, timeout); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordWaitForFences(
    VkDevice                                    device,
    uint32_t                                    fenceCount,
    const VkFence*                              pFences,
    VkBool32                                    waitAll,
    uint64_t                                    timeout) {
    ValidationStateTracker::PreCallRecordWaitForFences(device, fenceCount, pFences, waitAll, timeout);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordWaitForFences(device, fenceCount, pFences, waitAll, timeout); });
}

void BestPractices::PostCallRecordWaitForFences(
    VkDevice                                    device,
    uint32_t                                    fenceCount,
    const VkFence*                              pFences,
    VkBool32                                    waitAll,
    uint64_t                                    timeout,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordWaitForFences(device, fenceCount, pFences, waitAll, timeout, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordWaitForFences(device, fenceCount, pFences, waitAll, timeout, result); });
}

bool BestPractices::PreCallValidateCreateSemaphore(
    VkDevice                                    device,
    const VkSemaphoreCreateInfo*                pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSemaphore*                                pSemaphore) const {
    bool skip = ValidationStateTracker::PreCallValidateCreateSemaphore(device, pCreateInfo, pAllocator, pSemaphore);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCreateSemaphore(device, pCreateInfo, pAllocator, pSemaphore); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCreateSemaphore(
    VkDevice                                    device,
    const VkSemaphoreCreateInfo*                pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSemaphore*                                pSemaphore) {
    ValidationStateTracker::PreCallRecordCreateSemaphore(device, pCreateInfo, pAllocator, pSemaphore);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCreateSemaphore(device, pCreateInfo, pAllocator, pSemaphore); });
}

void BestPractices::PostCallRecordCreateSemaphore(
    VkDevice                                    device,
    const VkSemaphoreCreateInfo*                pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSemaphore*                                pSemaphore,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCreateSemaphore(device, pCreateInfo, pAllocator, pSemaphore, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCreateSemaphore(device, pCreateInfo, pAllocator, pSemaphore, result); });
}

bool BestPractices::PreCallValidateDestroySemaphore(
    VkDevice                                    device,
    VkSemaphore                                 semaphore,
    const VkAllocationCallbacks*                pAllocator) const {
    bool skip = ValidationStateTracker::PreCallValidateDestroySemaphore(device, semaphore, pAllocator);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateDestroySemaphore(device, semaphore, pAllocator); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordDestroySemaphore(
    VkDevice                                    device,
    VkSemaphore                                 semaphore,
    const VkAllocationCallbacks*                pAllocator) {
    ValidationStateTracker::PreCallRecordDestroySemaphore(device, semaphore, pAllocator);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordDestroySemaphore(device, semaphore, pAllocator); });
}

void BestPractices::PostCallRecordDestroySemaphore(
    VkDevice                                    device,
    VkSemaphore                                 semaphore,
    const VkAllocationCallbacks*                pAllocator) {
    ValidationStateTracker::PostCallRecordDestroySemaphore(device, semaphore, pAllocator);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordDestroySemaphore(device, semaphore, pAllocator); });
}

bool BestPractices::PreCallValidateCreateEvent(
    VkDevice                                    device,
    const VkEventCreateInfo*                    pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkEvent*                                    pEvent) const {
    bool skip = ValidationStateTracker::PreCallValidateCreateEvent(device, pCreateInfo, pAllocator, pEvent);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCreateEvent(device, pCreateInfo, pAllocator, pEvent); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCreateEvent(
    VkDevice                                    device,
    const VkEventCreateInfo*                    pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkEvent*                                    pEvent) {
    ValidationStateTracker::PreCallRecordCreateEvent(device, pCreateInfo, pAllocator, pEvent);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCreateEvent(device, pCreateInfo, pAllocator, pEvent); });
}

void BestPractices::PostCallRecordCreateEvent(
    VkDevice                                    device,
    const VkEventCreateInfo*                    pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkEvent*                                    pEvent,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCreateEvent(device, pCreateInfo, pAllocator, pEvent, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCreateEvent(device, pCreateInfo, pAllocator, pEvent, result); });
}

bool BestPractices::PreCallValidateDestroyEvent(
    VkDevice                                    device,
    VkEvent                                     event,
    const VkAllocationCallbacks*                pAllocator) const {
    bool skip = ValidationStateTracker::PreCallValidateDestroyEvent(device, event, pAllocator);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateDestroyEvent(device, event, pAllocator); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordDestroyEvent(
    VkDevice                                    device,
    VkEvent                                     event,
    const VkAllocationCallbacks*                pAllocator) {
    ValidationStateTracker::PreCallRecordDestroyEvent(device, event, pAllocator);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordDestroyEvent(device, event, pAllocator); });
}

void BestPractices::PostCallRecordDestroyEvent(
    VkDevice                                    device,
    VkEvent                                     event,
    const VkAllocationCallbacks*                pAllocator) {
    ValidationStateTracker::PostCallRecordDestroyEvent(device, event, pAllocator);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordDestroyEvent(device, event, pAllocator); });
}

bool BestPractices::PreCallValidateGetEventStatus(
    VkDevice                                    device,
    VkEvent                                     event) const {
    bool skip = ValidationStateTracker::PreCallValidateGetEventStatus(device, event);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetEventStatus(device, event); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetEventStatus(
    VkDevice                                    device,
    VkEvent                                     event) {
    ValidationStateTracker::PreCallRecordGetEventStatus(device, event);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetEventStatus(device, event); });
}

void BestPractices::PostCallRecordGetEventStatus(
    VkDevice                                    device,
    VkEvent                                     event,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetEventStatus(device, event, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetEventStatus(device, event, result); });
}

bool BestPractices::PreCallValidateSetEvent(
    VkDevice                                    device,
    VkEvent                                     event) const {
    bool skip = ValidationStateTracker::PreCallValidateSetEvent(device, event);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateSetEvent(device, event); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordSetEvent(
    VkDevice                                    device,
    VkEvent                                     event) {
    ValidationStateTracker::PreCallRecordSetEvent(device, event);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordSetEvent(device, event); });
}

void BestPractices::PostCallRecordSetEvent(
    VkDevice                                    device,
    VkEvent                                     event,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordSetEvent(device, event, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordSetEvent(device, event, result); });
}

bool BestPractices::PreCallValidateResetEvent(
    VkDevice                                    device,
    VkEvent                                     event) const {
    bool skip = ValidationStateTracker::PreCallValidateResetEvent(device, event);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateResetEvent(device, event); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordResetEvent(
    VkDevice                                    device,
    VkEvent                                     event) {
    ValidationStateTracker::PreCallRecordResetEvent(device, event);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordResetEvent(device, event); });
}

void BestPractices::PostCallRecordResetEvent(
    VkDevice                                    device,
    VkEvent                                     event,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordResetEvent(device, event, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordResetEvent(device, event, result); });
}

bool BestPractices::PreCallValidateCreateQueryPool(
    VkDevice                                    device,
    const VkQueryPoolCreateInfo*                pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkQueryPool*                                pQueryPool) const {
    bool skip = ValidationStateTracker::PreCallValidateCreateQueryPool(device, pCreateInfo, pAllocator, pQueryPool);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCreateQueryPool(device, pCreateInfo, pAllocator, pQueryPool); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCreateQueryPool(
    VkDevice                                    device,
    const VkQueryPoolCreateInfo*                pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkQueryPool*                                pQueryPool) {
    ValidationStateTracker::PreCallRecordCreateQueryPool(device, pCreateInfo, pAllocator, pQueryPool);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCreateQueryPool(device, pCreateInfo, pAllocator, pQueryPool); });
}

void BestPractices::PostCallRecordCreateQueryPool(
    VkDevice                                    device,
    const VkQueryPoolCreateInfo*                pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkQueryPool*                                pQueryPool,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCreateQueryPool(device, pCreateInfo, pAllocator, pQueryPool, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCreateQueryPool(device, pCreateInfo, pAllocator, pQueryPool, result); });
}

bool BestPractices::PreCallValidateDestroyQueryPool(
    VkDevice                                    device,
    VkQueryPool                                 queryPool,
    const VkAllocationCallbacks*                pAllocator) const {
    bool skip = ValidationStateTracker::PreCallValidateDestroyQueryPool(device, queryPool, pAllocator);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateDestroyQueryPool(device, queryPool, pAllocator); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordDestroyQueryPool(
    VkDevice                                    device,
    VkQueryPool                                 queryPool,
    const VkAllocationCallbacks*                pAllocator) {
    ValidationStateTracker::PreCallRecordDestroyQueryPool(device, queryPool, pAllocator);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordDestroyQueryPool(device, queryPool, pAllocator); });
}

void BestPractices::PostCallRecordDestroyQueryPool(
    VkDevice                                    device,
    VkQueryPool                                 queryPool,
    const VkAllocationCallbacks*                pAllocator) {
    ValidationStateTracker::PostCallRecordDestroyQueryPool(device, queryPool, pAllocator);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordDestroyQueryPool(device, queryPool, pAllocator); });
}

bool BestPractices::PreCallValidateGetQueryPoolResults(
    VkDevice                                    device,
    VkQueryPool                                 queryPool,
    uint32_t                                    firstQuery,
    uint32_t                                    queryCount,
    size_t                                      dataSize,
    void*                                       pData,
    VkDeviceSize                                stride,
    VkQueryResultFlags                          flags) const {
    bool skip = ValidationStateTracker::PreCallValidateGetQueryPoolResults(device, queryPool, firstQuery, queryCount, dataSize, pData, stride, flags);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetQueryPoolResults(device, queryPool, firstQuery, queryCount, dataSize, pData, stride, flags); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetQueryPoolResults(
    VkDevice                                    device,
    VkQueryPool                                 queryPool,
    uint32_t                                    firstQuery,
    uint32_t                                    queryCount,
    size_t                                      dataSize,
    void*                                       pData,
    VkDeviceSize                                stride,
    VkQueryResultFlags                          flags) {
    ValidationStateTracker::PreCallRecordGetQueryPoolResults(device, queryPool, firstQuery, queryCount, dataSize, pData, stride, flags);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetQueryPoolResults(device, queryPool, firstQuery, queryCount, dataSize, pData, stride, flags); });
}

void BestPractices::PostCallRecordGetQueryPoolResults(
    VkDevice                                    device,
    VkQueryPool                                 queryPool,
    uint32_t                                    firstQuery,
    uint32_t                                    queryCount,
    size_t                                      dataSize,
    void*                                       pData,
    VkDeviceSize                                stride,
    VkQueryResultFlags                          flags,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetQueryPoolResults(device, queryPool, firstQuery, queryCount, dataSize, pData, stride, flags, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetQueryPoolResults(device, queryPool, firstQuery, queryCount, dataSize, pData, stride, flags, result); });
}

bool BestPractices::PreCallValidateCreateBuffer(
    VkDevice                                    device,
    const VkBufferCreateInfo*                   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkBuffer*                                   pBuffer) const {
    bool skip = ValidationStateTracker::PreCallValidateCreateBuffer(device, pCreateInfo, pAllocator, pBuffer);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCreateBuffer(device, pCreateInfo, pAllocator, pBuffer); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCreateBuffer(
    VkDevice                                    device,
    const VkBufferCreateInfo*                   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkBuffer*                                   pBuffer) {
    ValidationStateTracker::PreCallRecordCreateBuffer(device, pCreateInfo, pAllocator, pBuffer);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCreateBuffer(device, pCreateInfo, pAllocator, pBuffer); });
}

void BestPractices::PostCallRecordCreateBuffer(
    VkDevice                                    device,
    const VkBufferCreateInfo*                   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkBuffer*                                   pBuffer,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCreateBuffer(device, pCreateInfo, pAllocator, pBuffer, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCreateBuffer(device, pCreateInfo, pAllocator, pBuffer, result); });
}

bool BestPractices::PreCallValidateDestroyBuffer(
    VkDevice                                    device,
    VkBuffer                                    buffer,
    const VkAllocationCallbacks*                pAllocator) const {
    bool skip = ValidationStateTracker::PreCallValidateDestroyBuffer(device, buffer, pAllocator);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateDestroyBuffer(device, buffer, pAllocator); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordDestroyBuffer(
    VkDevice                                    device,
    VkBuffer                                    buffer,
    const VkAllocationCallbacks*                pAllocator) {
    ValidationStateTracker::PreCallRecordDestroyBuffer(device, buffer, pAllocator);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordDestroyBuffer(device, buffer, pAllocator); });
}

void BestPractices::PostCallRecordDestroyBuffer(
    VkDevice                                    device,
    VkBuffer                                    buffer,
    const VkAllocationCallbacks*                pAllocator) {
    ValidationStateTracker::PostCallRecordDestroyBuffer(device, buffer, pAllocator);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordDestroyBuffer(device, buffer, pAllocator); });
}

bool BestPractices::PreCallValidateCreateBufferView(
    VkDevice                                    device,
    const VkBufferViewCreateInfo*               pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkBufferView*                               pView) const {
    bool skip = ValidationStateTracker::PreCallValidateCreateBufferView(device, pCreateInfo, pAllocator, pView);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCreateBufferView(device, pCreateInfo, pAllocator, pView); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCreateBufferView(
    VkDevice                                    device,
    const VkBufferViewCreateInfo*               pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkBufferView*                               pView) {
    ValidationStateTracker::PreCallRecordCreateBufferView(device, pCreateInfo, pAllocator, pView);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCreateBufferView(device, pCreateInfo, pAllocator, pView); });
}

void BestPractices::PostCallRecordCreateBufferView(
    VkDevice                                    device,
    const VkBufferViewCreateInfo*               pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkBufferView*                               pView,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCreateBufferView(device, pCreateInfo, pAllocator, pView, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCreateBufferView(device, pCreateInfo, pAllocator, pView, result); });
}

bool BestPractices::PreCallValidateDestroyBufferView(
    VkDevice                                    device,
    VkBufferView                                bufferView,
    const VkAllocationCallbacks*                pAllocator) const {
    bool skip = ValidationStateTracker::PreCallValidateDestroyBufferView(device, bufferView, pAllocator);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateDestroyBufferView(device, bufferView, pAllocator); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordDestroyBufferView(
    VkDevice                                    device,
    VkBufferView                                bufferView,
    const VkAllocationCallbacks*                pAllocator) {
    ValidationStateTracker::PreCallRecordDestroyBufferView(device, bufferView, pAllocator);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordDestroyBufferView(device, bufferView, pAllocator); });
}

void BestPractices::PostCallRecordDestroyBufferView(
    VkDevice                                    device,
    VkBufferView                                bufferView,
    const VkAllocationCallbacks*                pAllocator) {
    ValidationStateTracker::PostCallRecordDestroyBufferView(device, bufferView, pAllocator);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordDestroyBufferView(device, bufferView, pAllocator); });
}

bool BestPractices::PreCallValidateCreateImage(
    VkDevice                                    device,
    const VkImageCreateInfo*                    pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkImage*                                    pImage) const {
    bool skip = ValidationStateTracker::PreCallValidateCreateImage(device, pCreateInfo, pAllocator, pImage);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCreateImage(device, pCreateInfo, pAllocator, pImage); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCreateImage(
    VkDevice                                    device,
    const VkImageCreateInfo*                    pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkImage*                                    pImage) {
    ValidationStateTracker::PreCallRecordCreateImage(device, pCreateInfo, pAllocator, pImage);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCreateImage(device, pCreateInfo, pAllocator, pImage); });
}

void BestPractices::PostCallRecordCreateImage(
    VkDevice                                    device,
    const VkImageCreateInfo*                    pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkImage*                                    pImage,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCreateImage(device, pCreateInfo, pAllocator, pImage, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCreateImage(device, pCreateInfo, pAllocator, pImage, result); });
}

bool BestPractices::PreCallValidateDestroyImage(
    VkDevice                                    device,
    VkImage                                     image,
    const VkAllocationCallbacks*                pAllocator) const {
    bool skip = ValidationStateTracker::PreCallValidateDestroyImage(device, image, pAllocator);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateDestroyImage(device, image, pAllocator); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordDestroyImage(
    VkDevice                                    device,
    VkImage                                     image,
    const VkAllocationCallbacks*                pAllocator) {
    ValidationStateTracker::PreCallRecordDestroyImage(device, image, pAllocator);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordDestroyImage(device, image, pAllocator); });
}

void BestPractices::PostCallRecordDestroyImage(
    VkDevice                                    device,
    VkImage                                     image,
    const VkAllocationCallbacks*                pAllocator) {
    ValidationStateTracker::PostCallRecordDestroyImage(device, image, pAllocator);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordDestroyImage(device, image, pAllocator); });
}

bool BestPractices::PreCallValidateGetImageSubresourceLayout(
    VkDevice                                    device,
    VkImage                                     image,
    const VkImageSubresource*                   pSubresource,
    VkSubresourceLayout*                        pLayout) const {
    bool skip = ValidationStateTracker::PreCallValidateGetImageSubresourceLayout(device, image, pSubresource, pLayout);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetImageSubresourceLayout(device, image, pSubresource, pLayout); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetImageSubresourceLayout(
    VkDevice                                    device,
    VkImage                                     image,
    const VkImageSubresource*                   pSubresource,
    VkSubresourceLayout*                        pLayout) {
    ValidationStateTracker::PreCallRecordGetImageSubresourceLayout(device, image, pSubresource, pLayout);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetImageSubresourceLayout(device, image, pSubresource, pLayout); });
}

void BestPractices::PostCallRecordGetImageSubresourceLayout(
    VkDevice                                    device,
    VkImage                                     image,
    const VkImageSubresource*                   pSubresource,
    VkSubresourceLayout*                        pLayout) {
    ValidationStateTracker::PostCallRecordGetImageSubresourceLayout(device, image, pSubresource, pLayout);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetImageSubresourceLayout(device, image, pSubresource, pLayout); });
}

bool BestPractices::PreCallValidateCreateImageView(
    VkDevice                                    device,
    const VkImageViewCreateInfo*                pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkImageView*                                pView) const {
    bool skip = ValidationStateTracker::PreCallValidateCreateImageView(device, pCreateInfo, pAllocator, pView);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCreateImageView(device, pCreateInfo, pAllocator, pView); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCreateImageView(
    VkDevice                                    device,
    const VkImageViewCreateInfo*                pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkImageView*                                pView) {
    ValidationStateTracker::PreCallRecordCreateImageView(device, pCreateInfo, pAllocator, pView);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCreateImageView(device, pCreateInfo, pAllocator, pView); });
}

void BestPractices::PostCallRecordCreateImageView(
    VkDevice                                    device,
    const VkImageViewCreateInfo*                pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkImageView*                                pView,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCreateImageView(device, pCreateInfo, pAllocator, pView, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCreateImageView(device, pCreateInfo, pAllocator, pView, result); });
}

bool BestPractices::PreCallValidateDestroyImageView(
    VkDevice                                    device,
    VkImageView                                 imageView,
    const VkAllocationCallbacks*                pAllocator) const {
    bool skip = ValidationStateTracker::PreCallValidateDestroyImageView(device, imageView, pAllocator);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateDestroyImageView(device, imageView, pAllocator); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordDestroyImageView(
    VkDevice                                    device,
    VkImageView                                 imageView,
    const VkAllocationCallbacks*                pAllocator) {
    ValidationStateTracker::PreCallRecordDestroyImageView(device, imageView, pAllocator);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordDestroyImageView(device, imageView, pAllocator); });
}

void BestPractices::PostCallRecordDestroyImageView(
    VkDevice                                    device,
    VkImageView                                 imageView,
    const VkAllocationCallbacks*                pAllocator) {
    ValidationStateTracker::PostCallRecordDestroyImageView(device, imageView, pAllocator);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordDestroyImageView(device, imageView, pAllocator); });
}

bool BestPractices::PreCallValidateCreateShaderModule(
    VkDevice                                    device,
    const VkShaderModuleCreateInfo*             pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkShaderModule*                             pShaderModule,
    void*                                       state_data) const {
    bool skip = ValidationStateTracker::PreCallValidateCreateShaderModule(device, pCreateInfo, pAllocator, pShaderModule, state_data);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCreateShaderModule(device, pCreateInfo, pAllocator, pShaderModule, state_data); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCreateShaderModule(
    VkDevice                                    device,
    const VkShaderModuleCreateInfo*             pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkShaderModule*                             pShaderModule,
    void*                                       state_data) {
    ValidationStateTracker::PreCallRecordCreateShaderModule(device, pCreateInfo, pAllocator, pShaderModule, state_data);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCreateShaderModule(device, pCreateInfo, pAllocator, pShaderModule, state_data); });
}

void BestPractices::PostCallRecordCreateShaderModule(
    VkDevice                                    device,
    const VkShaderModuleCreateInfo*             pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkShaderModule*                             pShaderModule,
    VkResult                                    result,
    void*                                       state_data) {
    ValidationStateTracker::PostCallRecordCreateShaderModule(device, pCreateInfo, pAllocator, pShaderModule, result, state_data);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCreateShaderModule(device, pCreateInfo, pAllocator, pShaderModule, result, state_data); });
}

bool BestPractices::PreCallValidateDestroyShaderModule(
    VkDevice                                    device,
    VkShaderModule                              shaderModule,
    const VkAllocationCallbacks*                pAllocator) const {
    bool skip = ValidationStateTracker::PreCallValidateDestroyShaderModule(device, shaderModule, pAllocator);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateDestroyShaderModule(device, shaderModule, pAllocator); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordDestroyShaderModule(
    VkDevice                                    device,
    VkShaderModule                              shaderModule,
    const VkAllocationCallbacks*                pAllocator) {
    ValidationStateTracker::PreCallRecordDestroyShaderModule(device, shaderModule, pAllocator);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordDestroyShaderModule(device, shaderModule, pAllocator); });
}

void BestPractices::PostCallRecordDestroyShaderModule(
    VkDevice                                    device,
    VkShaderModule                              shaderModule,
    const VkAllocationCallbacks*                pAllocator) {
    ValidationStateTracker::PostCallRecordDestroyShaderModule(device, shaderModule, pAllocator);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordDestroyShaderModule(device, shaderModule, pAllocator); });
}

bool BestPractices::PreCallValidateCreatePipelineCache(
    VkDevice                                    device,
    const VkPipelineCacheCreateInfo*            pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkPipelineCache*                            pPipelineCache) const {
    bool skip = ValidationStateTracker::PreCallValidateCreatePipelineCache(device, pCreateInfo, pAllocator, pPipelineCache);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCreatePipelineCache(device, pCreateInfo, pAllocator, pPipelineCache); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCreatePipelineCache(
    VkDevice                                    device,
    const VkPipelineCacheCreateInfo*            pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkPipelineCache*                            pPipelineCache) {
    ValidationStateTracker::PreCallRecordCreatePipelineCache(device, pCreateInfo, pAllocator, pPipelineCache);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCreatePipelineCache(device, pCreateInfo, pAllocator, pPipelineCache); });
}

void BestPractices::PostCallRecordCreatePipelineCache(
    VkDevice                                    device,
    const VkPipelineCacheCreateInfo*            pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkPipelineCache*                            pPipelineCache,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCreatePipelineCache(device, pCreateInfo, pAllocator, pPipelineCache, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCreatePipelineCache(device, pCreateInfo, pAllocator, pPipelineCache, result); });
}

bool BestPractices::PreCallValidateDestroyPipelineCache(
    VkDevice                                    device,
    VkPipelineCache                             pipelineCache,
    const VkAllocationCallbacks*                pAllocator) const {
    bool skip = ValidationStateTracker::PreCallValidateDestroyPipelineCache(device, pipelineCache, pAllocator);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateDestroyPipelineCache(device, pipelineCache, pAllocator); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordDestroyPipelineCache(
    VkDevice                                    device,
    VkPipelineCache                             pipelineCache,
    const VkAllocationCallbacks*                pAllocator) {
    ValidationStateTracker::PreCallRecordDestroyPipelineCache(device, pipelineCache, pAllocator);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordDestroyPipelineCache(device, pipelineCache, pAllocator); });
}

void BestPractices::PostCallRecordDestroyPipelineCache(
    VkDevice                                    device,
    VkPipelineCache                             pipelineCache,
    const VkAllocationCallbacks*                pAllocator) {
    ValidationStateTracker::PostCallRecordDestroyPipelineCache(device, pipelineCache, pAllocator);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordDestroyPipelineCache(device, pipelineCache, pAllocator); });
}

bool BestPractices::PreCallValidateGetPipelineCacheData(
    VkDevice                                    device,
    VkPipelineCache                             pipelineCache,
    size_t*                                     pDataSize,
    void*                                       pData) const {
    bool skip = ValidationStateTracker::PreCallValidateGetPipelineCacheData(device, pipelineCache, pDataSize, pData);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetPipelineCacheData(device, pipelineCache, pDataSize, pData); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetPipelineCacheData(
    VkDevice                                    device,
    VkPipelineCache                             pipelineCache,
    size_t*                                     pDataSize,
    void*                                       pData) {
    ValidationStateTracker::PreCallRecordGetPipelineCacheData(device, pipelineCache, pDataSize, pData);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetPipelineCacheData(device, pipelineCache, pDataSize, pData); });
}

void BestPractices::PostCallRecordGetPipelineCacheData(
    VkDevice                                    device,
    VkPipelineCache                             pipelineCache,
    size_t*                                     pDataSize,
    void*                                       pData,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetPipelineCacheData(device, pipelineCache, pDataSize, pData, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetPipelineCacheData(device, pipelineCache, pDataSize, pData, result); });
}

bool BestPractices::PreCallValidateMergePipelineCaches(
    VkDevice                                    device,
    VkPipelineCache                             dstCache,
    uint32_t                                    srcCacheCount,
    const VkPipelineCache*                      pSrcCaches) const {
    bool skip = ValidationStateTracker::PreCallValidateMergePipelineCaches(device, dstCache, srcCacheCount, pSrcCaches);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateMergePipelineCaches(device, dstCache, srcCacheCount, pSrcCaches); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordMergePipelineCaches(
    VkDevice                                    device,
    VkPipelineCache                             dstCache,
    uint32_t                                    srcCacheCount,
    const VkPipelineCache*                      pSrcCaches) {
    ValidationStateTracker::PreCallRecordMergePipelineCaches(device, dstCache, srcCacheCount, pSrcCaches);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordMergePipelineCaches(device, dstCache, srcCacheCount, pSrcCaches); });
}

void BestPractices::PostCallRecordMergePipelineCaches(
    VkDevice                                    device,
    VkPipelineCache                             dstCache,
    uint32_t                                    srcCacheCount,
    const VkPipelineCache*                      pSrcCaches,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordMergePipelineCaches(device, dstCache, srcCacheCount, pSrcCaches, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordMergePipelineCaches(device, dstCache, srcCacheCount, pSrcCaches, result); });
}

bool BestPractices::PreCallValidateCreateGraphicsPipelines(
    VkDevice                                    device,
    VkPipelineCache                             pipelineCache,
    uint32_t                                    createInfoCount,
    const VkGraphicsPipelineCreateInfo*         pCreateInfos,
    const VkAllocationCallbacks*                pAllocator,
    VkPipeline*                                 pPipelines,
    void*                                       state_data) const {
    bool skip = ValidationStateTracker::PreCallValidateCreateGraphicsPipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines, state_data);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCreateGraphicsPipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines, state_data); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCreateGraphicsPipelines(
    VkDevice                                    device,
    VkPipelineCache                             pipelineCache,
    uint32_t                                    createInfoCount,
    const VkGraphicsPipelineCreateInfo*         pCreateInfos,
    const VkAllocationCallbacks*                pAllocator,
    VkPipeline*                                 pPipelines,
    void*                                       state_data) {
    ValidationStateTracker::PreCallRecordCreateGraphicsPipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines, state_data);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCreateGraphicsPipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines, state_data); });
}

void BestPractices::PostCallRecordCreateGraphicsPipelines(
    VkDevice                                    device,
    VkPipelineCache                             pipelineCache,
    uint32_t                                    createInfoCount,
    const VkGraphicsPipelineCreateInfo*         pCreateInfos,
    const VkAllocationCallbacks*                pAllocator,
    VkPipeline*                                 pPipelines,
    VkResult                                    result,
    void*                                       state_data) {
    ValidationStateTracker::PostCallRecordCreateGraphicsPipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines, result, state_data);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCreateGraphicsPipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines, result, state_data); });
}

bool BestPractices::PreCallValidateCreateComputePipelines(
    VkDevice                                    device,
    VkPipelineCache                             pipelineCache,
    uint32_t                                    createInfoCount,
    const VkComputePipelineCreateInfo*          pCreateInfos,
    const VkAllocationCallbacks*                pAllocator,
    VkPipeline*                                 pPipelines,
    void*                                       state_data) const {
    bool skip = ValidationStateTracker::PreCallValidateCreateComputePipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines, state_data);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCreateComputePipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines, state_data); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCreateComputePipelines(
    VkDevice                                    device,
    VkPipelineCache                             pipelineCache,
    uint32_t                                    createInfoCount,
    const VkComputePipelineCreateInfo*          pCreateInfos,
    const VkAllocationCallbacks*                pAllocator,
    VkPipeline*                                 pPipelines,
    void*                                       state_data) {
    ValidationStateTracker::PreCallRecordCreateComputePipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines, state_data);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCreateComputePipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines, state_data); });
}

void BestPractices::PostCallRecordCreateComputePipelines(
    VkDevice                                    device,
    VkPipelineCache                             pipelineCache,
    uint32_t                                    createInfoCount,
    const VkComputePipelineCreateInfo*          pCreateInfos,
    const VkAllocationCallbacks*                pAllocator,
    VkPipeline*                                 pPipelines,
    VkResult                                    result,
    void*                                       state_data) {
    ValidationStateTracker::PostCallRecordCreateComputePipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines, result, state_data);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCreateComputePipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines, result, state_data); });
}

bool BestPractices::PreCallValidateDestroyPipeline(
    VkDevice                                    device,
    VkPipeline                                  pipeline,
    const VkAllocationCallbacks*                pAllocator) const {
    bool skip = ValidationStateTracker::PreCallValidateDestroyPipeline(device, pipeline, pAllocator);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateDestroyPipeline(device, pipeline, pAllocator); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordDestroyPipeline(
    VkDevice                                    device,
    VkPipeline                                  pipeline,
    const VkAllocationCallbacks*                pAllocator) {
    ValidationStateTracker::PreCallRecordDestroyPipeline(device, pipeline, pAllocator);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordDestroyPipeline(device, pipeline, pAllocator); });
}

void BestPractices::PostCallRecordDestroyPipeline(
    VkDevice                                    device,
    VkPipeline                                  pipeline,
    const VkAllocationCallbacks*                pAllocator) {
    ValidationStateTracker::PostCallRecordDestroyPipeline(device, pipeline, pAllocator);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordDestroyPipeline(device, pipeline, pAllocator); });
}

bool BestPractices::PreCallValidateCreatePipelineLayout(
    VkDevice                                    device,
    const VkPipelineLayoutCreateInfo*           pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkPipelineLayout*                           pPipelineLayout) const {
    bool skip = ValidationStateTracker::PreCallValidateCreatePipelineLayout(device, pCreateInfo, pAllocator, pPipelineLayout);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCreatePipelineLayout(device, pCreateInfo, pAllocator, pPipelineLayout); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCreatePipelineLayout(
    VkDevice                                    device,
    const VkPipelineLayoutCreateInfo*           pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkPipelineLayout*                           pPipelineLayout) {
    ValidationStateTracker::PreCallRecordCreatePipelineLayout(device, pCreateInfo, pAllocator, pPipelineLayout);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCreatePipelineLayout(device, pCreateInfo, pAllocator, pPipelineLayout); });
}

void BestPractices::PostCallRecordCreatePipelineLayout(
    VkDevice                                    device,
    const VkPipelineLayoutCreateInfo*           pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkPipelineLayout*                           pPipelineLayout,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCreatePipelineLayout(device, pCreateInfo, pAllocator, pPipelineLayout, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCreatePipelineLayout(device, pCreateInfo, pAllocator, pPipelineLayout, result); });
}

bool BestPractices::PreCallValidateDestroyPipelineLayout(
    VkDevice                                    device,
    VkPipelineLayout                            pipelineLayout,
    const VkAllocationCallbacks*                pAllocator) const {
    bool skip = ValidationStateTracker::PreCallValidateDestroyPipelineLayout(device, pipelineLayout, pAllocator);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateDestroyPipelineLayout(device, pipelineLayout, pAllocator); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordDestroyPipelineLayout(
    VkDevice                                    device,
    VkPipelineLayout                            pipelineLayout,
    const VkAllocationCallbacks*                pAllocator) {
    ValidationStateTracker::PreCallRecordDestroyPipelineLayout(device, pipelineLayout, pAllocator);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordDestroyPipelineLayout(device, pipelineLayout, pAllocator); });
}

void BestPractices::PostCallRecordDestroyPipelineLayout(
    VkDevice                                    device,
    VkPipelineLayout                            pipelineLayout,
    const VkAllocationCallbacks*                pAllocator) {
    ValidationStateTracker::PostCallRecordDestroyPipelineLayout(device, pipelineLayout, pAllocator);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordDestroyPipelineLayout(device, pipelineLayout, pAllocator); });
}

bool BestPractices::PreCallValidateCreateSampler(
    VkDevice                                    device,
    const VkSamplerCreateInfo*                  pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSampler*                                  pSampler) const {
    bool skip = ValidationStateTracker::PreCallValidateCreateSampler(device, pCreateInfo, pAllocator, pSampler);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCreateSampler(device, pCreateInfo, pAllocator, pSampler); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCreateSampler(
    VkDevice                                    device,
    const VkSamplerCreateInfo*                  pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSampler*                                  pSampler) {
    ValidationStateTracker::PreCallRecordCreateSampler(device, pCreateInfo, pAllocator, pSampler);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCreateSampler(device, pCreateInfo, pAllocator, pSampler); });
}

void BestPractices::PostCallRecordCreateSampler(
    VkDevice                                    device,
    const VkSamplerCreateInfo*                  pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSampler*                                  pSampler,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCreateSampler(device, pCreateInfo, pAllocator, pSampler, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCreateSampler(device, pCreateInfo, pAllocator, pSampler, result); });
}

bool BestPractices::PreCallValidateDestroySampler(
    VkDevice                                    device,
    VkSampler                                   sampler,
    const VkAllocationCallbacks*                pAllocator) const {
    bool skip = ValidationStateTracker::PreCallValidateDestroySampler(device, sampler, pAllocator);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateDestroySampler(device, sampler, pAllocator); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordDestroySampler(
    VkDevice                                    device,
    VkSampler                                   sampler,
    const VkAllocationCallbacks*                pAllocator) {
    ValidationStateTracker::PreCallRecordDestroySampler(device, sampler, pAllocator);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordDestroySampler(device, sampler, pAllocator); });
}

void BestPractices::PostCallRecordDestroySampler(
    VkDevice                                    device,
    VkSampler                                   sampler,
    const VkAllocationCallbacks*                pAllocator) {
    ValidationStateTracker::PostCallRecordDestroySampler(device, sampler, pAllocator);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordDestroySampler(device, sampler, pAllocator); });
}

bool BestPractices::PreCallValidateCreateDescriptorSetLayout(
    VkDevice                                    device,
    const VkDescriptorSetLayoutCreateInfo*      pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDescriptorSetLayout*                      pSetLayout) const {
    bool skip = ValidationStateTracker::PreCallValidateCreateDescriptorSetLayout(device, pCreateInfo, pAllocator, pSetLayout);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCreateDescriptorSetLayout(device, pCreateInfo, pAllocator, pSetLayout); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCreateDescriptorSetLayout(
    VkDevice                                    device,
    const VkDescriptorSetLayoutCreateInfo*      pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDescriptorSetLayout*                      pSetLayout) {
    ValidationStateTracker::PreCallRecordCreateDescriptorSetLayout(device, pCreateInfo, pAllocator, pSetLayout);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCreateDescriptorSetLayout(device, pCreateInfo, pAllocator, pSetLayout); });
}

void BestPractices::PostCallRecordCreateDescriptorSetLayout(
    VkDevice                                    device,
    const VkDescriptorSetLayoutCreateInfo*      pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDescriptorSetLayout*                      pSetLayout,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCreateDescriptorSetLayout(device, pCreateInfo, pAllocator, pSetLayout, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCreateDescriptorSetLayout(device, pCreateInfo, pAllocator, pSetLayout, result); });
}

bool BestPractices::PreCallValidateDestroyDescriptorSetLayout(
    VkDevice                                    device,
    VkDescriptorSetLayout                       descriptorSetLayout,
    const VkAllocationCallbacks*                pAllocator) const {
    bool skip = ValidationStateTracker::PreCallValidateDestroyDescriptorSetLayout(device, descriptorSetLayout, pAllocator);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateDestroyDescriptorSetLayout(device, descriptorSetLayout, pAllocator); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordDestroyDescriptorSetLayout(
    VkDevice                                    device,
    VkDescriptorSetLayout                       descriptorSetLayout,
    const VkAllocationCallbacks*                pAllocator) {
    ValidationStateTracker::PreCallRecordDestroyDescriptorSetLayout(device, descriptorSetLayout, pAllocator);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordDestroyDescriptorSetLayout(device, descriptorSetLayout, pAllocator); });
}

void BestPractices::PostCallRecordDestroyDescriptorSetLayout(
    VkDevice                                    device,
    VkDescriptorSetLayout                       descriptorSetLayout,
    const VkAllocationCallbacks*                pAllocator) {
    ValidationStateTracker::PostCallRecordDestroyDescriptorSetLayout(device, descriptorSetLayout, pAllocator);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordDestroyDescriptorSetLayout(device, descriptorSetLayout, pAllocator); });
}

bool BestPractices::PreCallValidateCreateDescriptorPool(
    VkDevice                                    device,
    const VkDescriptorPoolCreateInfo*           pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDescriptorPool*                           pDescriptorPool) const {
    bool skip = ValidationStateTracker::PreCallValidateCreateDescriptorPool(device, pCreateInfo, pAllocator, pDescriptorPool);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCreateDescriptorPool(device, pCreateInfo, pAllocator, pDescriptorPool); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCreateDescriptorPool(
    VkDevice                                    device,
    const VkDescriptorPoolCreateInfo*           pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDescriptorPool*                           pDescriptorPool) {
    ValidationStateTracker::PreCallRecordCreateDescriptorPool(device, pCreateInfo, pAllocator, pDescriptorPool);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCreateDescriptorPool(device, pCreateInfo, pAllocator, pDescriptorPool); });
}

void BestPractices::PostCallRecordCreateDescriptorPool(
    VkDevice                                    device,
    const VkDescriptorPoolCreateInfo*           pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDescriptorPool*                           pDescriptorPool,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCreateDescriptorPool(device, pCreateInfo, pAllocator, pDescriptorPool, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCreateDescriptorPool(device, pCreateInfo, pAllocator, pDescriptorPool, result); });
}

bool BestPractices::PreCallValidateDestroyDescriptorPool(
    VkDevice                                    device,
    VkDescriptorPool                            descriptorPool,
    const VkAllocationCallbacks*                pAllocator) const {
    bool skip = ValidationStateTracker::PreCallValidateDestroyDescriptorPool(device, descriptorPool, pAllocator);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateDestroyDescriptorPool(device, descriptorPool, pAllocator); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordDestroyDescriptorPool(
    VkDevice                                    device,
    VkDescriptorPool                            descriptorPool,
    const VkAllocationCallbacks*                pAllocator) {
    ValidationStateTracker::PreCallRecordDestroyDescriptorPool(device, descriptorPool, pAllocator);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordDestroyDescriptorPool(device, descriptorPool, pAllocator); });
}

void BestPractices::PostCallRecordDestroyDescriptorPool(
    VkDevice                                    device,
    VkDescriptorPool                            descriptorPool,
    const VkAllocationCallbacks*                pAllocator) {
    ValidationStateTracker::PostCallRecordDestroyDescriptorPool(device, descriptorPool, pAllocator);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordDestroyDescriptorPool(device, descriptorPool, pAllocator); });
}

bool BestPractices::PreCallValidateResetDescriptorPool(
    VkDevice                                    device,
    VkDescriptorPool                            descriptorPool,
    VkDescriptorPoolResetFlags                  flags) const {
    bool skip = ValidationStateTracker::PreCallValidateResetDescriptorPool(device, descriptorPool, flags);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateResetDescriptorPool(device, descriptorPool, flags); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordResetDescriptorPool(
    VkDevice                                    device,
    VkDescriptorPool                            descriptorPool,
    VkDescriptorPoolResetFlags                  flags) {
    ValidationStateTracker::PreCallRecordResetDescriptorPool(device, descriptorPool, flags);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordResetDescriptorPool(device, descriptorPool, flags); });
}

void BestPractices::PostCallRecordResetDescriptorPool(
    VkDevice                                    device,
    VkDescriptorPool                            descriptorPool,
    VkDescriptorPoolResetFlags                  flags,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordResetDescriptorPool(device, descriptorPool, flags, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordResetDescriptorPool(device, descriptorPool, flags, result); });
}

bool BestPractices::PreCallValidateAllocateDescriptorSets(
    VkDevice                                    device,
    const VkDescriptorSetAllocateInfo*          pAllocateInfo,
    VkDescriptorSet*                            pDescriptorSets,
    void*                                       state_data) const {
    bool skip = ValidationStateTracker::PreCallValidateAllocateDescriptorSets(device, pAllocateInfo, pDescriptorSets, state_data);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateAllocateDescriptorSets(device, pAllocateInfo, pDescriptorSets, state_data); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordAllocateDescriptorSets(
    VkDevice                                    device,
    const VkDescriptorSetAllocateInfo*          pAllocateInfo,
    VkDescriptorSet*                            pDescriptorSets,
    void*                                       state_data) {
    ValidationStateTracker::PreCallRecordAllocateDescriptorSets(device, pAllocateInfo, pDescriptorSets, state_data);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordAllocateDescriptorSets(device, pAllocateInfo, pDescriptorSets, state_data); });
}

void BestPractices::PostCallRecordAllocateDescriptorSets(
    VkDevice                                    device,
    const VkDescriptorSetAllocateInfo*          pAllocateInfo,
    VkDescriptorSet*                            pDescriptorSets,
    VkResult                                    result,
    void*                                       state_data) {
    ValidationStateTracker::PostCallRecordAllocateDescriptorSets(device, pAllocateInfo, pDescriptorSets, result, state_data);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordAllocateDescriptorSets(device, pAllocateInfo, pDescriptorSets, result, state_data); });
}

bool BestPractices::PreCallValidateFreeDescriptorSets(
    VkDevice                                    device,
    VkDescriptorPool                            descriptorPool,
    uint32_t                                    descriptorSetCount,
    const VkDescriptorSet*                      pDescriptorSets) const {
    bool skip = ValidationStateTracker::PreCallValidateFreeDescriptorSets(device, descriptorPool, descriptorSetCount, pDescriptorSets);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateFreeDescriptorSets(device, descriptorPool, descriptorSetCount, pDescriptorSets); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordFreeDescriptorSets(
    VkDevice                                    device,
    VkDescriptorPool                            descriptorPool,
    uint32_t                                    descriptorSetCount,
    const VkDescriptorSet*                      pDescriptorSets) {
    ValidationStateTracker::PreCallRecordFreeDescriptorSets(device, descriptorPool, descriptorSetCount, pDescriptorSets);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordFreeDescriptorSets(device, descriptorPool, descriptorSetCount, pDescriptorSets); });
}

void BestPractices::PostCallRecordFreeDescriptorSets(
    VkDevice                                    device,
    VkDescriptorPool                            descriptorPool,
    uint32_t                                    descriptorSetCount,
    const VkDescriptorSet*                      pDescriptorSets,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordFreeDescriptorSets(device, descriptorPool, descriptorSetCount, pDescriptorSets, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordFreeDescriptorSets(device, descriptorPool, descriptorSetCount, pDescriptorSets, result); });
}

bool BestPractices::PreCallValidateUpdateDescriptorSets(
    VkDevice                                    device,
    uint32_t                                    descriptorWriteCount,
    const VkWriteDescriptorSet*                 pDescriptorWrites,
    uint32_t                                    descriptorCopyCount,
    const VkCopyDescriptorSet*                  pDescriptorCopies) const {
    bool skip = ValidationStateTracker::PreCallValidateUpdateDescriptorSets(device, descriptorWriteCount, pDescriptorWrites, descriptorCopyCount, pDescriptorCopies);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateUpdateDescriptorSets(device, descriptorWriteCount, pDescriptorWrites, descriptorCopyCount, pDescriptorCopies); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordUpdateDescriptorSets(
    VkDevice                                    device,
    uint32_t                                    descriptorWriteCount,
    const VkWriteDescriptorSet*                 pDescriptorWrites,
    uint32_t                                    descriptorCopyCount,
    const VkCopyDescriptorSet*                  pDescriptorCopies) {
    ValidationStateTracker::PreCallRecordUpdateDescriptorSets(device, descriptorWriteCount, pDescriptorWrites, descriptorCopyCount, pDescriptorCopies);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordUpdateDescriptorSets(device, descriptorWriteCount, pDescriptorWrites, descriptorCopyCount, pDescriptorCopies); });
}

void BestPractices::PostCallRecordUpdateDescriptorSets(
    VkDevice                                    device,
    uint32_t                                    descriptorWriteCount,
    const VkWriteDescriptorSet*                 pDescriptorWrites,
    uint32_t                                    descriptorCopyCount,
    const VkCopyDescriptorSet*                  pDescriptorCopies) {
    ValidationStateTracker::PostCallRecordUpdateDescriptorSets(device, descriptorWriteCount, pDescriptorWrites, descriptorCopyCount, pDescriptorCopies);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordUpdateDescriptorSets(device, descriptorWriteCount, pDescriptorWrites, descriptorCopyCount, pDescriptorCopies); });
}

bool BestPractices::PreCallValidateCreateFramebuffer(
    VkDevice                                    device,
    const VkFramebufferCreateInfo*              pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkFramebuffer*                              pFramebuffer) const {
    bool skip = ValidationStateTracker::PreCallValidateCreateFramebuffer(device, pCreateInfo, pAllocator, pFramebuffer);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCreateFramebuffer(device, pCreateInfo, pAllocator, pFramebuffer); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCreateFramebuffer(
    VkDevice                                    device,
    const VkFramebufferCreateInfo*              pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkFramebuffer*                              pFramebuffer) {
    ValidationStateTracker::PreCallRecordCreateFramebuffer(device, pCreateInfo, pAllocator, pFramebuffer);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCreateFramebuffer(device, pCreateInfo, pAllocator, pFramebuffer); });
}

void BestPractices::PostCallRecordCreateFramebuffer(
    VkDevice                                    device,
    const VkFramebufferCreateInfo*              pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkFramebuffer*                              pFramebuffer,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCreateFramebuffer(device, pCreateInfo, pAllocator, pFramebuffer, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCreateFramebuffer(device, pCreateInfo, pAllocator, pFramebuffer, result); });
}

bool BestPractices::PreCallValidateDestroyFramebuffer(
    VkDevice                                    device,
    VkFramebuffer                               framebuffer,
    const VkAllocationCallbacks*                pAllocator) const {
    bool skip = ValidationStateTracker::PreCallValidateDestroyFramebuffer(device, framebuffer, pAllocator);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateDestroyFramebuffer(device, framebuffer, pAllocator); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordDestroyFramebuffer(
    VkDevice                                    device,
    VkFramebuffer                               framebuffer,
    const VkAllocationCallbacks*                pAllocator) {
    ValidationStateTracker::PreCallRecordDestroyFramebuffer(device, framebuffer, pAllocator);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordDestroyFramebuffer(device, framebuffer, pAllocator); });
}

void BestPractices::PostCallRecordDestroyFramebuffer(
    VkDevice                                    device,
    VkFramebuffer                               framebuffer,
    const VkAllocationCallbacks*                pAllocator) {
    ValidationStateTracker::PostCallRecordDestroyFramebuffer(device, framebuffer, pAllocator);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordDestroyFramebuffer(device, framebuffer, pAllocator); });
}

bool BestPractices::PreCallValidateCreateRenderPass(
    VkDevice                                    device,
    const VkRenderPassCreateInfo*               pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkRenderPass*                               pRenderPass) const {
    bool skip = ValidationStateTracker::PreCallValidateCreateRenderPass(device, pCreateInfo, pAllocator, pRenderPass);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCreateRenderPass(device, pCreateInfo, pAllocator, pRenderPass); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCreateRenderPass(
    VkDevice                                    device,
    const VkRenderPassCreateInfo*               pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkRenderPass*                               pRenderPass) {
    ValidationStateTracker::PreCallRecordCreateRenderPass(device, pCreateInfo, pAllocator, pRenderPass);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCreateRenderPass(device, pCreateInfo, pAllocator, pRenderPass); });
}

void BestPractices::PostCallRecordCreateRenderPass(
    VkDevice                                    device,
    const VkRenderPassCreateInfo*               pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkRenderPass*                               pRenderPass,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCreateRenderPass(device, pCreateInfo, pAllocator, pRenderPass, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCreateRenderPass(device, pCreateInfo, pAllocator, pRenderPass, result); });
}

bool BestPractices::PreCallValidateDestroyRenderPass(
    VkDevice                                    device,
    VkRenderPass                                renderPass,
    const VkAllocationCallbacks*                pAllocator) const {
    bool skip = ValidationStateTracker::PreCallValidateDestroyRenderPass(device, renderPass, pAllocator);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateDestroyRenderPass(device, renderPass, pAllocator); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordDestroyRenderPass(
    VkDevice                                    device,
    VkRenderPass                                renderPass,
    const VkAllocationCallbacks*                pAllocator) {
    ValidationStateTracker::PreCallRecordDestroyRenderPass(device, renderPass, pAllocator);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordDestroyRenderPass(device, renderPass, pAllocator); });
}

void BestPractices::PostCallRecordDestroyRenderPass(
    VkDevice                                    device,
    VkRenderPass                                renderPass,
    const VkAllocationCallbacks*                pAllocator) {
    ValidationStateTracker::PostCallRecordDestroyRenderPass(device, renderPass, pAllocator);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordDestroyRenderPass(device, renderPass, pAllocator); });
}

bool BestPractices::PreCallValidateGetRenderAreaGranularity(
    VkDevice                                    device,
    VkRenderPass                                renderPass,
    VkExtent2D*                                 pGranularity) const {
    bool skip = ValidationStateTracker::PreCallValidateGetRenderAreaGranularity(device, renderPass, pGranularity);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetRenderAreaGranularity(device, renderPass, pGranularity); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetRenderAreaGranularity(
    VkDevice                                    device,
    VkRenderPass                                renderPass,
    VkExtent2D*                                 pGranularity) {
    ValidationStateTracker::PreCallRecordGetRenderAreaGranularity(device, renderPass, pGranularity);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetRenderAreaGranularity(device, renderPass, pGranularity); });
}

void BestPractices::PostCallRecordGetRenderAreaGranularity(
    VkDevice                                    device,
    VkRenderPass                                renderPass,
    VkExtent2D*                                 pGranularity) {
    ValidationStateTracker::PostCallRecordGetRenderAreaGranularity(device, renderPass, pGranularity);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetRenderAreaGranularity(device, renderPass, pGranularity); });
}

bool BestPractices::PreCallValidateCreateCommandPool(
    VkDevice                                    device,
    const VkCommandPoolCreateInfo*              pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkCommandPool*                              pCommandPool) const {
    bool skip = ValidationStateTracker::PreCallValidateCreateCommandPool(device, pCreateInfo, pAllocator, pCommandPool);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCreateCommandPool(device, pCreateInfo, pAllocator, pCommandPool); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCreateCommandPool(
    VkDevice                                    device,
    const VkCommandPoolCreateInfo*              pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkCommandPool*                              pCommandPool) {
    ValidationStateTracker::PreCallRecordCreateCommandPool(device, pCreateInfo, pAllocator, pCommandPool);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCreateCommandPool(device, pCreateInfo, pAllocator, pCommandPool); });
}

void BestPractices::PostCallRecordCreateCommandPool(
    VkDevice                                    device,
    const VkCommandPoolCreateInfo*              pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkCommandPool*                              pCommandPool,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCreateCommandPool(device, pCreateInfo, pAllocator, pCommandPool, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCreateCommandPool(device, pCreateInfo, pAllocator, pCommandPool, result); });
}

bool BestPractices::PreCallValidateDestroyCommandPool(
    VkDevice                                    device,
    VkCommandPool                               commandPool,
    const VkAllocationCallbacks*                pAllocator) const {
    bool skip = ValidationStateTracker::PreCallValidateDestroyCommandPool(device, commandPool, pAllocator);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateDestroyCommandPool(device, commandPool, pAllocator); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordDestroyCommandPool(
    VkDevice                                    device,
    VkCommandPool                               commandPool,
    const VkAllocationCallbacks*                pAllocator) {
    ValidationStateTracker::PreCallRecordDestroyCommandPool(device, commandPool, pAllocator);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordDestroyCommandPool(device, commandPool, pAllocator); });
}

void BestPractices::PostCallRecordDestroyCommandPool(
    VkDevice                                    device,
    VkCommandPool                               commandPool,
    const VkAllocationCallbacks*                pAllocator) {
    ValidationStateTracker::PostCallRecordDestroyCommandPool(device, commandPool, pAllocator);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordDestroyCommandPool(device, commandPool, pAllocator); });
}

bool BestPractices::PreCallValidateResetCommandPool(
    VkDevice                                    device,
    VkCommandPool                               commandPool,
    VkCommandPoolResetFlags                     flags) const {
    bool skip = ValidationStateTracker::PreCallValidateResetCommandPool(device, commandPool, flags);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateResetCommandPool(device, commandPool, flags); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordResetCommandPool(
    VkDevice                                    device,
    VkCommandPool                               commandPool,
    VkCommandPoolResetFlags                     flags) {
    ValidationStateTracker::PreCallRecordResetCommandPool(device, commandPool, flags);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordResetCommandPool(device, commandPool, flags); });
}

void BestPractices::PostCallRecordResetCommandPool(
    VkDevice                                    device,
    VkCommandPool                               commandPool,
    VkCommandPoolResetFlags                     flags,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordResetCommandPool(device, commandPool, flags, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordResetCommandPool(device, commandPool, flags, result); });
}

bool BestPractices::PreCallValidateAllocateCommandBuffers(
    VkDevice                                    device,
    const VkCommandBufferAllocateInfo*          pAllocateInfo,
    VkCommandBuffer*                            pCommandBuffers) const {
    bool skip = ValidationStateTracker::PreCallValidateAllocateCommandBuffers(device, pAllocateInfo, pCommandBuffers);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateAllocateCommandBuffers(device, pAllocateInfo, pCommandBuffers); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordAllocateCommandBuffers(
    VkDevice                                    device,
    const VkCommandBufferAllocateInfo*          pAllocateInfo,
    VkCommandBuffer*                            pCommandBuffers) {
    ValidationStateTracker::PreCallRecordAllocateCommandBuffers(device, pAllocateInfo, pCommandBuffers);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordAllocateCommandBuffers(device, pAllocateInfo, pCommandBuffers); });
}

void BestPractices::PostCallRecordAllocateCommandBuffers(
    VkDevice                                    device,
    const VkCommandBufferAllocateInfo*          pAllocateInfo,
    VkCommandBuffer*                            pCommandBuffers,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordAllocateCommandBuffers(device, pAllocateInfo, pCommandBuffers, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordAllocateCommandBuffers(device, pAllocateInfo, pCommandBuffers, result); });
}

bool BestPractices::PreCallValidateFreeCommandBuffers(
    VkDevice                                    device,
    VkCommandPool                               commandPool,
    uint32_t                                    commandBufferCount,
    const VkCommandBuffer*                      pCommandBuffers) const {
    bool skip = ValidationStateTracker::PreCallValidateFreeCommandBuffers(device, commandPool, commandBufferCount, pCommandBuffers);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateFreeCommandBuffers(device, commandPool, commandBufferCount, pCommandBuffers); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordFreeCommandBuffers(
    VkDevice                                    device,
    VkCommandPool                               commandPool,
    uint32_t                                    commandBufferCount,
    const VkCommandBuffer*                      pCommandBuffers) {
    ValidationStateTracker::PreCallRecordFreeCommandBuffers(device, commandPool, commandBufferCount, pCommandBuffers);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordFreeCommandBuffers(device, commandPool, commandBufferCount, pCommandBuffers); });
}

void BestPractices::PostCallRecordFreeCommandBuffers(
    VkDevice                                    device,
    VkCommandPool                               commandPool,
    uint32_t                                    commandBufferCount,
    const VkCommandBuffer*                      pCommandBuffers) {
    ValidationStateTracker::PostCallRecordFreeCommandBuffers(device, commandPool, commandBufferCount, pCommandBuffers);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordFreeCommandBuffers(device, commandPool, commandBufferCount, pCommandBuffers); });
}

bool BestPractices::PreCallValidateBeginCommandBuffer(
    VkCommandBuffer                             commandBuffer,
    const VkCommandBufferBeginInfo*             pBeginInfo) const {
    bool skip = ValidationStateTracker::PreCallValidateBeginCommandBuffer(commandBuffer, pBeginInfo);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateBeginCommandBuffer(commandBuffer, pBeginInfo); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordBeginCommandBuffer(
    VkCommandBuffer                             commandBuffer,
    const VkCommandBufferBeginInfo*             pBeginInfo) {
    ValidationStateTracker::PreCallRecordBeginCommandBuffer(commandBuffer, pBeginInfo);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordBeginCommandBuffer(commandBuffer, pBeginInfo); });
}

void BestPractices::PostCallRecordBeginCommandBuffer(
    VkCommandBuffer                             commandBuffer,
    const VkCommandBufferBeginInfo*             pBeginInfo,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordBeginCommandBuffer(commandBuffer, pBeginInfo, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordBeginCommandBuffer(commandBuffer, pBeginInfo, result); });
}

bool BestPractices::PreCallValidateEndCommandBuffer(
    VkCommandBuffer                             commandBuffer) const {
    bool skip = ValidationStateTracker::PreCallValidateEndCommandBuffer(commandBuffer);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateEndCommandBuffer(commandBuffer); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordEndCommandBuffer(
    VkCommandBuffer                             commandBuffer) {
    ValidationStateTracker::PreCallRecordEndCommandBuffer(commandBuffer);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordEndCommandBuffer(commandBuffer); });
}

void BestPractices::PostCallRecordEndCommandBuffer(
    VkCommandBuffer                             commandBuffer,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordEndCommandBuffer(commandBuffer, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordEndCommandBuffer(commandBuffer, result); });
}

bool BestPractices::PreCallValidateResetCommandBuffer(
    VkCommandBuffer                             commandBuffer,
    VkCommandBufferResetFlags                   flags) const {
    bool skip = ValidationStateTracker::PreCallValidateResetCommandBuffer(commandBuffer, flags);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateResetCommandBuffer(commandBuffer, flags); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordResetCommandBuffer(
    VkCommandBuffer                             commandBuffer,
    VkCommandBufferResetFlags                   flags) {
    ValidationStateTracker::PreCallRecordResetCommandBuffer(commandBuffer, flags);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordResetCommandBuffer(commandBuffer, flags); });
}

void BestPractices::PostCallRecordResetCommandBuffer(
    VkCommandBuffer                             commandBuffer,
    VkCommandBufferResetFlags                   flags,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordResetCommandBuffer(commandBuffer, flags, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordResetCommandBuffer(commandBuffer, flags, result); });
}

bool BestPractices::PreCallValidateCmdBindPipeline(
    VkCommandBuffer                             commandBuffer,
    VkPipelineBindPoint                         pipelineBindPoint,
    VkPipeline                                  pipeline) const {
    bool skip = ValidationStateTracker::PreCallValidateCmdBindPipeline(commandBuffer, pipelineBindPoint, pipeline);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCmdBindPipeline(commandBuffer, pipelineBindPoint, pipeline); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCmdBindPipeline(
    VkCommandBuffer                             commandBuffer,
    VkPipelineBindPoint                         pipelineBindPoint,
    VkPipeline                                  pipeline) {
    ValidationStateTracker::PreCallRecordCmdBindPipeline(commandBuffer, pipelineBindPoint, pipeline);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCmdBindPipeline(commandBuffer, pipelineBindPoint, pipeline); });
}

void BestPractices::PostCallRecordCmdBindPipeline(
    VkCommandBuffer                             commandBuffer,
    VkPipelineBindPoint                         pipelineBindPoint,
    VkPipeline                                  pipeline) {
    ValidationStateTracker::PostCallRecordCmdBindPipeline(commandBuffer, pipelineBindPoint, pipeline);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCmdBindPipeline(commandBuffer, pipelineBindPoint, pipeline); });
}

bool BestPractices::PreCallValidateCmdSetViewport(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstViewport,
    uint32_t                                    viewportCount,
    const VkViewport*                           pViewports) const {
    bool skip = ValidationStateTracker::PreCallValidateCmdSetViewport(commandBuffer, firstViewport, viewportCount, pViewports);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCmdSetViewport(commandBuffer, firstViewport, viewportCount, pViewports); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCmdSetViewport(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstViewport,
    uint32_t                                    viewportCount,
    const VkViewport*                           pViewports) {
    ValidationStateTracker::PreCallRecordCmdSetViewport(commandBuffer, firstViewport, viewportCount, pViewports);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCmdSetViewport(commandBuffer, firstViewport, viewportCount, pViewports); });
}

void BestPractices::PostCallRecordCmdSetViewport(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstViewport,
    uint32_t                                    viewportCount,
    const VkViewport*                           pViewports) {
    ValidationStateTracker::PostCallRecordCmdSetViewport(commandBuffer, firstViewport, viewportCount, pViewports);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCmdSetViewport(commandBuffer, firstViewport, viewportCount, pViewports); });
}

bool BestPractices::PreCallValidateCmdSetScissor(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstScissor,
    uint32_t                                    scissorCount,
    const VkRect2D*                             pScissors) const {
    bool skip = ValidationStateTracker::PreCallValidateCmdSetScissor(commandBuffer, firstScissor, scissorCount, pScissors);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCmdSetScissor(commandBuffer, firstScissor, scissorCount, pScissors); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCmdSetScissor(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstScissor,
    uint32_t                                    scissorCount,
    const VkRect2D*                             pScissors) {
    ValidationStateTracker::PreCallRecordCmdSetScissor(commandBuffer, firstScissor, scissorCount, pScissors);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCmdSetScissor(commandBuffer, firstScissor, scissorCount, pScissors); });
}

void BestPractices::PostCallRecordCmdSetScissor(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstScissor,
    uint32_t                                    scissorCount,
    const VkRect2D*                             pScissors) {
    ValidationStateTracker::PostCallRecordCmdSetScissor(commandBuffer, firstScissor, scissorCount, pScissors);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCmdSetScissor(commandBuffer, firstScissor, scissorCount, pScissors); });
}

bool BestPractices::PreCallValidateCmdSetLineWidth(
    VkCommandBuffer                             commandBuffer,
    float                                       lineWidth) const {
    bool skip = ValidationStateTracker::PreCallValidateCmdSetLineWidth(commandBuffer, lineWidth);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCmdSetLineWidth(commandBuffer, lineWidth); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCmdSetLineWidth(
    VkCommandBuffer                             commandBuffer,
    float                                       lineWidth) {
    ValidationStateTracker::PreCallRecordCmdSetLineWidth(commandBuffer, lineWidth);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCmdSetLineWidth(commandBuffer, lineWidth); });
}

void BestPractices::PostCallRecordCmdSetLineWidth(
    VkCommandBuffer                             commandBuffer,
    float                                       lineWidth) {
    ValidationStateTracker::PostCallRecordCmdSetLineWidth(commandBuffer, lineWidth);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCmdSetLineWidth(commandBuffer, lineWidth); });
}

bool BestPractices::PreCallValidateCmdSetDepthBias(
    VkCommandBuffer                             commandBuffer,
    float                                       depthBiasConstantFactor,
    float                                       depthBiasClamp,
    float                                       depthBiasSlopeFactor) const {
    bool skip = ValidationStateTracker::PreCallValidateCmdSetDepthBias(commandBuffer, depthBiasConstantFactor, depthBiasClamp, depthBiasSlopeFactor);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCmdSetDepthBias(commandBuffer, depthBiasConstantFactor, depthBiasClamp, depthBiasSlopeFactor); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCmdSetDepthBias(
    VkCommandBuffer                             commandBuffer,
    float                                       depthBiasConstantFactor,
    float                                       depthBiasClamp,
    float                                       depthBiasSlopeFactor) {
    ValidationStateTracker::PreCallRecordCmdSetDepthBias(commandBuffer, depthBiasConstantFactor, depthBiasClamp, depthBiasSlopeFactor);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCmdSetDepthBias(commandBuffer, depthBiasConstantFactor, depthBiasClamp, depthBiasSlopeFactor); });
}

void BestPractices::PostCallRecordCmdSetDepthBias(
    VkCommandBuffer                             commandBuffer,
    float                                       depthBiasConstantFactor,
    float                                       depthBiasClamp,
    float                                       depthBiasSlopeFactor) {
    ValidationStateTracker::PostCallRecordCmdSetDepthBias(commandBuffer, depthBiasConstantFactor, depthBiasClamp, depthBiasSlopeFactor);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCmdSetDepthBias(commandBuffer, depthBiasConstantFactor, depthBiasClamp, depthBiasSlopeFactor); });
}

bool BestPractices::PreCallValidateCmdSetBlendConstants(
    VkCommandBuffer                             commandBuffer,
    const float                                 blendConstants[4]) const {
    bool skip = ValidationStateTracker::PreCallValidateCmdSetBlendConstants(commandBuffer, blendConstants);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCmdSetBlendConstants(commandBuffer, blendConstants); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCmdSetBlendConstants(
    VkCommandBuffer                             commandBuffer,
    const float                                 blendConstants[4]) {
    ValidationStateTracker::PreCallRecordCmdSetBlendConstants(commandBuffer, blendConstants);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCmdSetBlendConstants(commandBuffer, blendConstants); });
}

void BestPractices::PostCallRecordCmdSetBlendConstants(
    VkCommandBuffer                             commandBuffer,
    const float                                 blendConstants[4]) {
    ValidationStateTracker::PostCallRecordCmdSetBlendConstants(commandBuffer, blendConstants);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCmdSetBlendConstants(commandBuffer, blendConstants); });
}

bool BestPractices::PreCallValidateCmdSetDepthBounds(
    VkCommandBuffer                             commandBuffer,
    float                                       minDepthBounds,
    float                                       maxDepthBounds) const {
    bool skip = ValidationStateTracker::PreCallValidateCmdSetDepthBounds(commandBuffer, minDepthBounds, maxDepthBounds);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCmdSetDepthBounds(commandBuffer, minDepthBounds, maxDepthBounds); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCmdSetDepthBounds(
    VkCommandBuffer                             commandBuffer,
    float                                       minDepthBounds,
    float                                       maxDepthBounds) {
    ValidationStateTracker::PreCallRecordCmdSetDepthBounds(commandBuffer, minDepthBounds, maxDepthBounds);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCmdSetDepthBounds(commandBuffer, minDepthBounds, maxDepthBounds); });
}

void BestPractices::PostCallRecordCmdSetDepthBounds(
    VkCommandBuffer                             commandBuffer,
    float                                       minDepthBounds,
    float                                       maxDepthBounds) {
    ValidationStateTracker::PostCallRecordCmdSetDepthBounds(commandBuffer, minDepthBounds, maxDepthBounds);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCmdSetDepthBounds(commandBuffer, minDepthBounds, maxDepthBounds); });
}

bool BestPractices::PreCallValidateCmdSetStencilCompareMask(
    VkCommandBuffer                             commandBuffer,
    VkStencilFaceFlags                          faceMask,
    uint32_t                                    compareMask) const {
    bool skip = ValidationStateTracker::PreCallValidateCmdSetStencilCompareMask(commandBuffer, faceMask, compareMask);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCmdSetStencilCompareMask(commandBuffer, faceMask, compareMask); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCmdSetStencilCompareMask(
    VkCommandBuffer                             commandBuffer,
    VkStencilFaceFlags                          faceMask,
    uint32_t                                    compareMask) {
    ValidationStateTracker::PreCallRecordCmdSetStencilCompareMask(commandBuffer, faceMask, compareMask);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCmdSetStencilCompareMask(commandBuffer, faceMask, compareMask); });
}

void BestPractices::PostCallRecordCmdSetStencilCompareMask(
    VkCommandBuffer                             commandBuffer,
    VkStencilFaceFlags                          faceMask,
    uint32_t                                    compareMask) {
    ValidationStateTracker::PostCallRecordCmdSetStencilCompareMask(commandBuffer, faceMask, compareMask);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCmdSetStencilCompareMask(commandBuffer, faceMask, compareMask); });
}

bool BestPractices::PreCallValidateCmdSetStencilWriteMask(
    VkCommandBuffer                             commandBuffer,
    VkStencilFaceFlags                          faceMask,
    uint32_t                                    writeMask) const {
    bool skip = ValidationStateTracker::PreCallValidateCmdSetStencilWriteMask(commandBuffer, faceMask, writeMask);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCmdSetStencilWriteMask(commandBuffer, faceMask, writeMask); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCmdSetStencilWriteMask(
    VkCommandBuffer                             commandBuffer,
    VkStencilFaceFlags                          faceMask,
    uint32_t                                    writeMask) {
    ValidationStateTracker::PreCallRecordCmdSetStencilWriteMask(commandBuffer, faceMask, writeMask);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCmdSetStencilWriteMask(commandBuffer, faceMask, writeMask); });
}

void BestPractices::PostCallRecordCmdSetStencilWriteMask(
    VkCommandBuffer                             commandBuffer,
    VkStencilFaceFlags                          faceMask,
    uint32_t                                    writeMask) {
    ValidationStateTracker::PostCallRecordCmdSetStencilWriteMask(commandBuffer, faceMask, writeMask);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCmdSetStencilWriteMask(commandBuffer, faceMask, writeMask); });
}

bool BestPractices::PreCallValidateCmdSetStencilReference(
    VkCommandBuffer                             commandBuffer,
    VkStencilFaceFlags                          faceMask,
    uint32_t                                    reference) const {
    bool skip = ValidationStateTracker::PreCallValidateCmdSetStencilReference(commandBuffer, faceMask, reference);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCmdSetStencilReference(commandBuffer, faceMask, reference); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCmdSetStencilReference(
    VkCommandBuffer                             commandBuffer,
    VkStencilFaceFlags                          faceMask,
    uint32_t                                    reference) {
    ValidationStateTracker::PreCallRecordCmdSetStencilReference(commandBuffer, faceMask, reference);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCmdSetStencilReference(commandBuffer, faceMask, reference); });
}

void BestPractices::PostCallRecordCmdSetStencilReference(
    VkCommandBuffer                             commandBuffer,
    VkStencilFaceFlags                          faceMask,
    uint32_t                                    reference) {
    ValidationStateTracker::PostCallRecordCmdSetStencilReference(commandBuffer, faceMask, reference);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCmdSetStencilReference(commandBuffer, faceMask, reference); });
}

bool BestPractices::PreCallValidateCmdBindDescriptorSets(
    VkCommandBuffer                             commandBuffer,
    VkPipelineBindPoint                         pipelineBindPoint,
    VkPipelineLayout                            layout,
    uint32_t                                    firstSet,
    uint32_t                                    descriptorSetCount,
    const VkDescriptorSet*                      pDescriptorSets,
    uint32_t                                    dynamicOffsetCount,
    const uint32_t*                             pDynamicOffsets) const {
    bool skip = ValidationStateTracker::PreCallValidateCmdBindDescriptorSets(commandBuffer, pipelineBindPoint, layout, firstSet, descriptorSetCount, pDescriptorSets, dynamicOffsetCount, pDynamicOffsets);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCmdBindDescriptorSets(commandBuffer, pipelineBindPoint, layout, firstSet, descriptorSetCount, pDescriptorSets, dynamicOffsetCount, pDynamicOffsets); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCmdBindDescriptorSets(
    VkCommandBuffer                             commandBuffer,
    VkPipelineBindPoint                         pipelineBindPoint,
    VkPipelineLayout                            layout,
    uint32_t                                    firstSet,
    uint32_t                                    descriptorSetCount,
    const VkDescriptorSet*                      pDescriptorSets,
    uint32_t                                    dynamicOffsetCount,
    const uint32_t*                             pDynamicOffsets) {
    ValidationStateTracker::PreCallRecordCmdBindDescriptorSets(commandBuffer, pipelineBindPoint, layout, firstSet, descriptorSetCount, pDescriptorSets, dynamicOffsetCount, pDynamicOffsets);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCmdBindDescriptorSets(commandBuffer, pipelineBindPoint, layout, firstSet, descriptorSetCount, pDescriptorSets, dynamicOffsetCount, pDynamicOffsets); });
}

void BestPractices::PostCallRecordCmdBindDescriptorSets(
    VkCommandBuffer                             commandBuffer,
    VkPipelineBindPoint                         pipelineBindPoint,
    VkPipelineLayout                            layout,
    uint32_t                                    firstSet,
    uint32_t                                    descriptorSetCount,
    const VkDescriptorSet*                      pDescriptorSets,
    uint32_t                                    dynamicOffsetCount,
    const uint32_t*                             pDynamicOffsets) {
    ValidationStateTracker::PostCallRecordCmdBindDescriptorSets(commandBuffer, pipelineBindPoint, layout, firstSet, descriptorSetCount, pDescriptorSets, dynamicOffsetCount, pDynamicOffsets);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCmdBindDescriptorSets(commandBuffer, pipelineBindPoint, layout, firstSet, descriptorSetCount, pDescriptorSets, dynamicOffsetCount, pDynamicOffsets); });
}

bool BestPractices::PreCallValidateCmdBindIndexBuffer(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkIndexType                                 indexType) const {
    bool skip = ValidationStateTracker::PreCallValidateCmdBindIndexBuffer(commandBuffer, buffer, offset, indexType);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCmdBindIndexBuffer(commandBuffer, buffer, offset, indexType); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCmdBindIndexBuffer(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkIndexType                                 indexType) {
    ValidationStateTracker::PreCallRecordCmdBindIndexBuffer(commandBuffer, buffer, offset, indexType);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCmdBindIndexBuffer(commandBuffer, buffer, offset, indexType); });
}

void BestPractices::PostCallRecordCmdBindIndexBuffer(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkIndexType                                 indexType) {
    ValidationStateTracker::PostCallRecordCmdBindIndexBuffer(commandBuffer, buffer, offset, indexType);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCmdBindIndexBuffer(commandBuffer, buffer, offset, indexType); });
}

bool BestPractices::PreCallValidateCmdBindVertexBuffers(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstBinding,
    uint32_t                                    bindingCount,
    const VkBuffer*                             pBuffers,
    const VkDeviceSize*                         pOffsets) const {
    bool skip = ValidationStateTracker::PreCallValidateCmdBindVertexBuffers(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCmdBindVertexBuffers(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCmdBindVertexBuffers(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstBinding,
    uint32_t                                    bindingCount,
    const VkBuffer*                             pBuffers,
    const VkDeviceSize*                         pOffsets) {
    ValidationStateTracker::PreCallRecordCmdBindVertexBuffers(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCmdBindVertexBuffers(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets); });
}

void BestPractices::PostCallRecordCmdBindVertexBuffers(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstBinding,
    uint32_t                                    bindingCount,
    const VkBuffer*                             pBuffers,
    const VkDeviceSize*                         pOffsets) {
    ValidationStateTracker::PostCallRecordCmdBindVertexBuffers(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCmdBindVertexBuffers(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets); });
}

bool BestPractices::PreCallValidateCmdDraw(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    vertexCount,
    uint32_t                                    instanceCount,
    uint32_t                                    firstVertex,
    uint32_t                                    firstInstance) const {
    bool skip = ValidationStateTracker::PreCallValidateCmdDraw(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCmdDraw(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCmdDraw(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    vertexCount,
    uint32_t                                    instanceCount,
    uint32_t                                    firstVertex,
    uint32_t                                    firstInstance) {
    ValidationStateTracker::PreCallRecordCmdDraw(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCmdDraw(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance); });
}

void BestPractices::PostCallRecordCmdDraw(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    vertexCount,
    uint32_t                                    instanceCount,
    uint32_t                                    firstVertex,
    uint32_t                                    firstInstance) {
    ValidationStateTracker::PostCallRecordCmdDraw(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCmdDraw(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance); });
}

bool BestPractices::PreCallValidateCmdDrawIndexed(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    indexCount,
    uint32_t                                    instanceCount,
    uint32_t                                    firstIndex,
    int32_t                                     vertexOffset,
    uint32_t                                    firstInstance) const {
    bool skip = ValidationStateTracker::PreCallValidateCmdDrawIndexed(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCmdDrawIndexed(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCmdDrawIndexed(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    indexCount,
    uint32_t                                    instanceCount,
    uint32_t                                    firstIndex,
    int32_t                                     vertexOffset,
    uint32_t                                    firstInstance) {
    ValidationStateTracker::PreCallRecordCmdDrawIndexed(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCmdDrawIndexed(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance); });
}

void BestPractices::PostCallRecordCmdDrawIndexed(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    indexCount,
    uint32_t                                    instanceCount,
    uint32_t                                    firstIndex,
    int32_t                                     vertexOffset,
    uint32_t                                    firstInstance) {
    ValidationStateTracker::PostCallRecordCmdDrawIndexed(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCmdDrawIndexed(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance); });
}

bool BestPractices::PreCallValidateCmdDrawIndirect(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    uint32_t                                    drawCount,
    uint32_t                                    stride) const {
    bool skip = ValidationStateTracker::PreCallValidateCmdDrawIndirect(commandBuffer, buffer, offset, drawCount, stride);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCmdDrawIndirect(commandBuffer, buffer, offset, drawCount, stride); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCmdDrawIndirect(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    uint32_t                                    drawCount,
    uint32_t                                    stride) {
    ValidationStateTracker::PreCallRecordCmdDrawIndirect(commandBuffer, buffer, offset, drawCount, stride);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCmdDrawIndirect(commandBuffer, buffer, offset, drawCount, stride); });
}

void BestPractices::PostCallRecordCmdDrawIndirect(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    uint32_t                                    drawCount,
    uint32_t                                    stride) {
    ValidationStateTracker::PostCallRecordCmdDrawIndirect(commandBuffer, buffer, offset, drawCount, stride);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCmdDrawIndirect(commandBuffer, buffer, offset, drawCount, stride); });
}

bool BestPractices::PreCallValidateCmdDrawIndexedIndirect(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    uint32_t                                    drawCount,
    uint32_t                                    stride) const {
    bool skip = ValidationStateTracker::PreCallValidateCmdDrawIndexedIndirect(commandBuffer, buffer, offset, drawCount, stride);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCmdDrawIndexedIndirect(commandBuffer, buffer, offset, drawCount, stride); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCmdDrawIndexedIndirect(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    uint32_t                                    drawCount,
    uint32_t                                    stride) {
    ValidationStateTracker::PreCallRecordCmdDrawIndexedIndirect(commandBuffer, buffer, offset, drawCount, stride);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCmdDrawIndexedIndirect(commandBuffer, buffer, offset, drawCount, stride); });
}

void BestPractices::PostCallRecordCmdDrawIndexedIndirect(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    uint32_t                                    drawCount,
    uint32_t                                    stride) {
    ValidationStateTracker::PostCallRecordCmdDrawIndexedIndirect(commandBuffer, buffer, offset, drawCount, stride);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCmdDrawIndexedIndirect(commandBuffer, buffer, offset, drawCount, stride); });
}

bool BestPractices::PreCallValidateCmdDispatch(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    groupCountX,
    uint32_t                                    groupCountY,
    uint32_t                                    groupCountZ) const {
    bool skip = ValidationStateTracker::PreCallValidateCmdDispatch(commandBuffer, groupCountX, groupCountY, groupCountZ);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCmdDispatch(commandBuffer, groupCountX, groupCountY, groupCountZ); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCmdDispatch(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    groupCountX,
    uint32_t                                    groupCountY,
    uint32_t                                    groupCountZ) {
    ValidationStateTracker::PreCallRecordCmdDispatch(commandBuffer, groupCountX, groupCountY, groupCountZ);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCmdDispatch(commandBuffer, groupCountX, groupCountY, groupCountZ); });
}

void BestPractices::PostCallRecordCmdDispatch(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    groupCountX,
    uint32_t                                    groupCountY,
    uint32_t                                    groupCountZ) {
    ValidationStateTracker::PostCallRecordCmdDispatch(commandBuffer, groupCountX, groupCountY, groupCountZ);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCmdDispatch(commandBuffer, groupCountX, groupCountY, groupCountZ); });
}

bool BestPractices::PreCallValidateCmdDispatchIndirect(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset) const {
    bool skip = ValidationStateTracker::PreCallValidateCmdDispatchIndirect(commandBuffer, buffer, offset);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCmdDispatchIndirect(commandBuffer, buffer, offset); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCmdDispatchIndirect(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset) {
    ValidationStateTracker::PreCallRecordCmdDispatchIndirect(commandBuffer, buffer, offset);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCmdDispatchIndirect(commandBuffer, buffer, offset); });
}

void BestPractices::PostCallRecordCmdDispatchIndirect(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset) {
    ValidationStateTracker::PostCallRecordCmdDispatchIndirect(commandBuffer, buffer, offset);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCmdDispatchIndirect(commandBuffer, buffer, offset); });
}

bool BestPractices::PreCallValidateCmdCopyBuffer(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    srcBuffer,
    VkBuffer                                    dstBuffer,
    uint32_t                                    regionCount,
    const VkBufferCopy*                         pRegions) const {
    bool skip = ValidationStateTracker::PreCallValidateCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, regionCount, pRegions);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, regionCount, pRegions); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCmdCopyBuffer(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    srcBuffer,
    VkBuffer                                    dstBuffer,
    uint32_t                                    regionCount,
    const VkBufferCopy*                         pRegions) {
    ValidationStateTracker::PreCallRecordCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, regionCount, pRegions);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, regionCount, pRegions); });
}

void BestPractices::PostCallRecordCmdCopyBuffer(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    srcBuffer,
    VkBuffer                                    dstBuffer,
    uint32_t                                    regionCount,
    const VkBufferCopy*                         pRegions) {
    ValidationStateTracker::PostCallRecordCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, regionCount, pRegions);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, regionCount, pRegions); });
}

bool BestPractices::PreCallValidateCmdCopyImage(
    VkCommandBuffer                             commandBuffer,
    VkImage                                     srcImage,
    VkImageLayout                               srcImageLayout,
    VkImage                                     dstImage,
    VkImageLayout                               dstImageLayout,
    uint32_t                                    regionCount,
    const VkImageCopy*                          pRegions) const {
    bool skip = ValidationStateTracker::PreCallValidateCmdCopyImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCmdCopyImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCmdCopyImage(
    VkCommandBuffer                             commandBuffer,
    VkImage                                     srcImage,
    VkImageLayout                               srcImageLayout,
    VkImage                                     dstImage,
    VkImageLayout                               dstImageLayout,
    uint32_t                                    regionCount,
    const VkImageCopy*                          pRegions) {
    ValidationStateTracker::PreCallRecordCmdCopyImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCmdCopyImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions); });
}

void BestPractices::PostCallRecordCmdCopyImage(
    VkCommandBuffer                             commandBuffer,
    VkImage                                     srcImage,
    VkImageLayout                               srcImageLayout,
    VkImage                                     dstImage,
    VkImageLayout                               dstImageLayout,
    uint32_t                                    regionCount,
    const VkImageCopy*                          pRegions) {
    ValidationStateTracker::PostCallRecordCmdCopyImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCmdCopyImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions); });
}

bool BestPractices::PreCallValidateCmdBlitImage(
    VkCommandBuffer                             commandBuffer,
    VkImage                                     srcImage,
    VkImageLayout                               srcImageLayout,
    VkImage                                     dstImage,
    VkImageLayout                               dstImageLayout,
    uint32_t                                    regionCount,
    const VkImageBlit*                          pRegions,
    VkFilter                                    filter) const {
    bool skip = ValidationStateTracker::PreCallValidateCmdBlitImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions, filter);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCmdBlitImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions, filter); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCmdBlitImage(
    VkCommandBuffer                             commandBuffer,
    VkImage                                     srcImage,
    VkImageLayout                               srcImageLayout,
    VkImage                                     dstImage,
    VkImageLayout                               dstImageLayout,
    uint32_t                                    regionCount,
    const VkImageBlit*                          pRegions,
    VkFilter                                    filter) {
    ValidationStateTracker::PreCallRecordCmdBlitImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions, filter);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCmdBlitImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions, filter); });
}

void BestPractices::PostCallRecordCmdBlitImage(
    VkCommandBuffer                             commandBuffer,
    VkImage                                     srcImage,
    VkImageLayout                               srcImageLayout,
    VkImage                                     dstImage,
    VkImageLayout                               dstImageLayout,
    uint32_t                                    regionCount,
    const VkImageBlit*                          pRegions,
    VkFilter                                    filter) {
    ValidationStateTracker::PostCallRecordCmdBlitImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions, filter);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCmdBlitImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions, filter); });
}

bool BestPractices::PreCallValidateCmdCopyBufferToImage(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    srcBuffer,
    VkImage                                     dstImage,
    VkImageLayout                               dstImageLayout,
    uint32_t                                    regionCount,
    const VkBufferImageCopy*                    pRegions) const {
    bool skip = ValidationStateTracker::PreCallValidateCmdCopyBufferToImage(commandBuffer, srcBuffer, dstImage, dstImageLayout, regionCount, pRegions);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCmdCopyBufferToImage(commandBuffer, srcBuffer, dstImage, dstImageLayout, regionCount, pRegions); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCmdCopyBufferToImage(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    srcBuffer,
    VkImage                                     dstImage,
    VkImageLayout                               dstImageLayout,
    uint32_t                                    regionCount,
    const VkBufferImageCopy*                    pRegions) {
    ValidationStateTracker::PreCallRecordCmdCopyBufferToImage(commandBuffer, srcBuffer, dstImage, dstImageLayout, regionCount, pRegions);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCmdCopyBufferToImage(commandBuffer, srcBuffer, dstImage, dstImageLayout, regionCount, pRegions); });
}

void BestPractices::PostCallRecordCmdCopyBufferToImage(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    srcBuffer,
    VkImage                                     dstImage,
    VkImageLayout                               dstImageLayout,
    uint32_t                                    regionCount,
    const VkBufferImageCopy*                    pRegions) {
    ValidationStateTracker::PostCallRecordCmdCopyBufferToImage(commandBuffer, srcBuffer, dstImage, dstImageLayout, regionCount, pRegions);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCmdCopyBufferToImage(commandBuffer, srcBuffer, dstImage, dstImageLayout, regionCount, pRegions); });
}

bool BestPractices::PreCallValidateCmdCopyImageToBuffer(
    VkCommandBuffer                             commandBuffer,
    VkImage                                     srcImage,
    VkImageLayout                               srcImageLayout,
    VkBuffer                                    dstBuffer,
    uint32_t                                    regionCount,
    const VkBufferImageCopy*                    pRegions) const {
    bool skip = ValidationStateTracker::PreCallValidateCmdCopyImageToBuffer(commandBuffer, srcImage, srcImageLayout, dstBuffer, regionCount, pRegions);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCmdCopyImageToBuffer(commandBuffer, srcImage, srcImageLayout, dstBuffer, regionCount, pRegions); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCmdCopyImageToBuffer(
    VkCommandBuffer                             commandBuffer,
    VkImage                                     srcImage,
    VkImageLayout                               srcImageLayout,
    VkBuffer                                    dstBuffer,
    uint32_t                                    regionCount,
    const VkBufferImageCopy*                    pRegions) {
    ValidationStateTracker::PreCallRecordCmdCopyImageToBuffer(commandBuffer, srcImage, srcImageLayout, dstBuffer, regionCount, pRegions);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCmdCopyImageToBuffer(commandBuffer, srcImage, srcImageLayout, dstBuffer, regionCount, pRegions); });
}

void BestPractices::PostCallRecordCmdCopyImageToBuffer(
    VkCommandBuffer                             commandBuffer,
    VkImage                                     srcImage,
    VkImageLayout                               srcImageLayout,
    VkBuffer                                    dstBuffer,
    uint32_t                                    regionCount,
    const VkBufferImageCopy*                    pRegions) {
    ValidationStateTracker::PostCallRecordCmdCopyImageToBuffer(commandBuffer, srcImage, srcImageLayout, dstBuffer, regionCount, pRegions);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCmdCopyImageToBuffer(commandBuffer, srcImage, srcImageLayout, dstBuffer, regionCount, pRegions); });
}

bool BestPractices::PreCallValidateCmdUpdateBuffer(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    dstBuffer,
    VkDeviceSize                                dstOffset,
    VkDeviceSize                                dataSize,
    const void*                                 pData) const {
    bool skip = ValidationStateTracker::PreCallValidateCmdUpdateBuffer(commandBuffer, dstBuffer, dstOffset, dataSize, pData);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCmdUpdateBuffer(commandBuffer, dstBuffer, dstOffset, dataSize, pData); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCmdUpdateBuffer(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    dstBuffer,
    VkDeviceSize                                dstOffset,
    VkDeviceSize                                dataSize,
    const void*                                 pData) {
    ValidationStateTracker::PreCallRecordCmdUpdateBuffer(commandBuffer, dstBuffer, dstOffset, dataSize, pData);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCmdUpdateBuffer(commandBuffer, dstBuffer, dstOffset, dataSize, pData); });
}

void BestPractices::PostCallRecordCmdUpdateBuffer(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    dstBuffer,
    VkDeviceSize                                dstOffset,
    VkDeviceSize                                dataSize,
    const void*                                 pData) {
    ValidationStateTracker::PostCallRecordCmdUpdateBuffer(commandBuffer, dstBuffer, dstOffset, dataSize, pData);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCmdUpdateBuffer(commandBuffer, dstBuffer, dstOffset, dataSize, pData); });
}

bool BestPractices::PreCallValidateCmdFillBuffer(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    dstBuffer,
    VkDeviceSize                                dstOffset,
    VkDeviceSize                                size,
    uint32_t                                    data) const {
    bool skip = ValidationStateTracker::PreCallValidateCmdFillBuffer(commandBuffer, dstBuffer, dstOffset, size, data);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCmdFillBuffer(commandBuffer, dstBuffer, dstOffset, size, data); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCmdFillBuffer(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    dstBuffer,
    VkDeviceSize                                dstOffset,
    VkDeviceSize                                size,
    uint32_t                                    data) {
    ValidationStateTracker::PreCallRecordCmdFillBuffer(commandBuffer, dstBuffer, dstOffset, size, data);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCmdFillBuffer(commandBuffer, dstBuffer, dstOffset, size, data); });
}

void BestPractices::PostCallRecordCmdFillBuffer(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    dstBuffer,
    VkDeviceSize                                dstOffset,
    VkDeviceSize                                size,
    uint32_t                                    data) {
    ValidationStateTracker::PostCallRecordCmdFillBuffer(commandBuffer, dstBuffer, dstOffset, size, data);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCmdFillBuffer(commandBuffer, dstBuffer, dstOffset, size, data); });
}

bool BestPractices::PreCallValidateCmdClearColorImage(
    VkCommandBuffer                             commandBuffer,
    VkImage                                     image,
    VkImageLayout                               imageLayout,
    const VkClearColorValue*                    pColor,
    uint32_t                                    rangeCount,
    const VkImageSubresourceRange*              pRanges) const {
    bool skip = ValidationStateTracker::PreCallValidateCmdClearColorImage(commandBuffer, image, imageLayout, pColor, rangeCount, pRanges);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCmdClearColorImage(commandBuffer, image, imageLayout, pColor, rangeCount, pRanges); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCmdClearColorImage(
    VkCommandBuffer                             commandBuffer,
    VkImage                                     image,
    VkImageLayout                               imageLayout,
    const VkClearColorValue*                    pColor,
    uint32_t                                    rangeCount,
    const VkImageSubresourceRange*              pRanges) {
    ValidationStateTracker::PreCallRecordCmdClearColorImage(commandBuffer, image, imageLayout, pColor, rangeCount, pRanges);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCmdClearColorImage(commandBuffer, image, imageLayout, pColor, rangeCount, pRanges); });
}

void BestPractices::PostCallRecordCmdClearColorImage(
    VkCommandBuffer                             commandBuffer,
    VkImage                                     image,
    VkImageLayout                               imageLayout,
    const VkClearColorValue*                    pColor,
    uint32_t                                    rangeCount,
    const VkImageSubresourceRange*              pRanges) {
    ValidationStateTracker::PostCallRecordCmdClearColorImage(commandBuffer, image, imageLayout, pColor, rangeCount, pRanges);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCmdClearColorImage(commandBuffer, image, imageLayout, pColor, rangeCount, pRanges); });
}

bool BestPractices::PreCallValidateCmdClearDepthStencilImage(
    VkCommandBuffer                             commandBuffer,
    VkImage                                     image,
    VkImageLayout                               imageLayout,
    const VkClearDepthStencilValue*             pDepthStencil,
    uint32_t                                    rangeCount,
    const VkImageSubresourceRange*              pRanges) const {
    bool skip = ValidationStateTracker::PreCallValidateCmdClearDepthStencilImage(commandBuffer, image, imageLayout, pDepthStencil, rangeCount, pRanges);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCmdClearDepthStencilImage(commandBuffer, image, imageLayout, pDepthStencil, rangeCount, pRanges); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCmdClearDepthStencilImage(
    VkCommandBuffer                             commandBuffer,
    VkImage                                     image,
    VkImageLayout                               imageLayout,
    const VkClearDepthStencilValue*             pDepthStencil,
    uint32_t                                    rangeCount,
    const VkImageSubresourceRange*              pRanges) {
    ValidationStateTracker::PreCallRecordCmdClearDepthStencilImage(commandBuffer, image, imageLayout, pDepthStencil, rangeCount, pRanges);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCmdClearDepthStencilImage(commandBuffer, image, imageLayout, pDepthStencil, rangeCount, pRanges); });
}

void BestPractices::PostCallRecordCmdClearDepthStencilImage(
    VkCommandBuffer                             commandBuffer,
    VkImage                                     image,
    VkImageLayout                               imageLayout,
    const VkClearDepthStencilValue*             pDepthStencil,
    uint32_t                                    rangeCount,
    const VkImageSubresourceRange*              pRanges) {
    ValidationStateTracker::PostCallRecordCmdClearDepthStencilImage(commandBuffer, image, imageLayout, pDepthStencil, rangeCount, pRanges);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCmdClearDepthStencilImage(commandBuffer, image, imageLayout, pDepthStencil, rangeCount, pRanges); });
}

bool BestPractices::PreCallValidateCmdClearAttachments(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    attachmentCount,
    const VkClearAttachment*                    pAttachments,
    uint32_t                                    rectCount,
    const VkClearRect*                          pRects) const {
    bool skip = ValidationStateTracker::PreCallValidateCmdClearAttachments(commandBuffer, attachmentCount, pAttachments, rectCount, pRects);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCmdClearAttachments(commandBuffer, attachmentCount, pAttachments, rectCount, pRects); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCmdClearAttachments(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    attachmentCount,
    const VkClearAttachment*                    pAttachments,
    uint32_t                                    rectCount,
    const VkClearRect*                          pRects) {
    ValidationStateTracker::PreCallRecordCmdClearAttachments(commandBuffer, attachmentCount, pAttachments, rectCount, pRects);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCmdClearAttachments(commandBuffer, attachmentCount, pAttachments, rectCount, pRects); });
}

void BestPractices::PostCallRecordCmdClearAttachments(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    attachmentCount,
    const VkClearAttachment*                    pAttachments,
    uint32_t                                    rectCount,
    const VkClearRect*                          pRects) {
    ValidationStateTracker::PostCallRecordCmdClearAttachments(commandBuffer, attachmentCount, pAttachments, rectCount, pRects);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCmdClearAttachments(commandBuffer, attachmentCount, pAttachments, rectCount, pRects); });
}

bool BestPractices::PreCallValidateCmdResolveImage(
    VkCommandBuffer                             commandBuffer,
    VkImage                                     srcImage,
    VkImageLayout                               srcImageLayout,
    VkImage                                     dstImage,
    VkImageLayout                               dstImageLayout,
    uint32_t                                    regionCount,
    const VkImageResolve*                       pRegions) const {
    bool skip = ValidationStateTracker::PreCallValidateCmdResolveImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCmdResolveImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCmdResolveImage(
    VkCommandBuffer                             commandBuffer,
    VkImage                                     srcImage,
    VkImageLayout                               srcImageLayout,
    VkImage                                     dstImage,
    VkImageLayout                               dstImageLayout,
    uint32_t                                    regionCount,
    const VkImageResolve*                       pRegions) {
    ValidationStateTracker::PreCallRecordCmdResolveImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCmdResolveImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions); });
}

void BestPractices::PostCallRecordCmdResolveImage(
    VkCommandBuffer                             commandBuffer,
    VkImage                                     srcImage,
    VkImageLayout                               srcImageLayout,
    VkImage                                     dstImage,
    VkImageLayout                               dstImageLayout,
    uint32_t                                    regionCount,
    const VkImageResolve*                       pRegions) {
    ValidationStateTracker::PostCallRecordCmdResolveImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCmdResolveImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions); });
}

bool BestPractices::PreCallValidateCmdSetEvent(
    VkCommandBuffer                             commandBuffer,
    VkEvent                                     event,
    VkPipelineStageFlags                        stageMask) const {
    bool skip = ValidationStateTracker::PreCallValidateCmdSetEvent(commandBuffer, event, stageMask);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCmdSetEvent(commandBuffer, event, stageMask); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCmdSetEvent(
    VkCommandBuffer                             commandBuffer,
    VkEvent                                     event,
    VkPipelineStageFlags                        stageMask) {
    ValidationStateTracker::PreCallRecordCmdSetEvent(commandBuffer, event, stageMask);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCmdSetEvent(commandBuffer, event, stageMask); });
}

void BestPractices::PostCallRecordCmdSetEvent(
    VkCommandBuffer                             commandBuffer,
    VkEvent                                     event,
    VkPipelineStageFlags                        stageMask) {
    ValidationStateTracker::PostCallRecordCmdSetEvent(commandBuffer, event, stageMask);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCmdSetEvent(commandBuffer, event, stageMask); });
}

bool BestPractices::PreCallValidateCmdResetEvent(
    VkCommandBuffer                             commandBuffer,
    VkEvent                                     event,
    VkPipelineStageFlags                        stageMask) const {
    bool skip = ValidationStateTracker::PreCallValidateCmdResetEvent(commandBuffer, event, stageMask);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCmdResetEvent(commandBuffer, event, stageMask); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCmdResetEvent(
    VkCommandBuffer                             commandBuffer,
    VkEvent                                     event,
    VkPipelineStageFlags                        stageMask) {
    ValidationStateTracker::PreCallRecordCmdResetEvent(commandBuffer, event, stageMask);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCmdResetEvent(commandBuffer, event, stageMask); });
}

void BestPractices::PostCallRecordCmdResetEvent(
    VkCommandBuffer                             commandBuffer,
    VkEvent                                     event,
    VkPipelineStageFlags                        stageMask) {
    ValidationStateTracker::PostCallRecordCmdResetEvent(commandBuffer, event, stageMask);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCmdResetEvent(commandBuffer, event, stageMask); });
}

bool BestPractices::PreCallValidateCmdWaitEvents(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    eventCount,
    const VkEvent*                              pEvents,
    VkPipelineStageFlags                        srcStageMask,
    VkPipelineStageFlags                        dstStageMask,
    uint32_t                                    memoryBarrierCount,
    const VkMemoryBarrier*                      pMemoryBarriers,
    uint32_t                                    bufferMemoryBarrierCount,
    const VkBufferMemoryBarrier*                pBufferMemoryBarriers,
    uint32_t                                    imageMemoryBarrierCount,
    const VkImageMemoryBarrier*                 pImageMemoryBarriers) const {
    bool skip = ValidationStateTracker::PreCallValidateCmdWaitEvents(commandBuffer, eventCount, pEvents, srcStageMask, dstStageMask, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCmdWaitEvents(commandBuffer, eventCount, pEvents, srcStageMask, dstStageMask, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCmdWaitEvents(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    eventCount,
    const VkEvent*                              pEvents,
    VkPipelineStageFlags                        srcStageMask,
    VkPipelineStageFlags                        dstStageMask,
    uint32_t                                    memoryBarrierCount,
    const VkMemoryBarrier*                      pMemoryBarriers,
    uint32_t                                    bufferMemoryBarrierCount,
    const VkBufferMemoryBarrier*                pBufferMemoryBarriers,
    uint32_t                                    imageMemoryBarrierCount,
    const VkImageMemoryBarrier*                 pImageMemoryBarriers) {
    ValidationStateTracker::PreCallRecordCmdWaitEvents(commandBuffer, eventCount, pEvents, srcStageMask, dstStageMask, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCmdWaitEvents(commandBuffer, eventCount, pEvents, srcStageMask, dstStageMask, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers); });
}

void BestPractices::PostCallRecordCmdWaitEvents(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    eventCount,
    const VkEvent*                              pEvents,
    VkPipelineStageFlags                        srcStageMask,
    VkPipelineStageFlags                        dstStageMask,
    uint32_t                                    memoryBarrierCount,
    const VkMemoryBarrier*                      pMemoryBarriers,
    uint32_t                                    bufferMemoryBarrierCount,
    const VkBufferMemoryBarrier*                pBufferMemoryBarriers,
    uint32_t                                    imageMemoryBarrierCount,
    const VkImageMemoryBarrier*                 pImageMemoryBarriers) {
    ValidationStateTracker::PostCallRecordCmdWaitEvents(commandBuffer, eventCount, pEvents, srcStageMask, dstStageMask, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCmdWaitEvents(commandBuffer, eventCount, pEvents, srcStageMask, dstStageMask, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers); });
}

bool BestPractices::PreCallValidateCmdPipelineBarrier(
    VkCommandBuffer                             commandBuffer,
    VkPipelineStageFlags                        srcStageMask,
    VkPipelineStageFlags                        dstStageMask,
    VkDependencyFlags                           dependencyFlags,
    uint32_t                                    memoryBarrierCount,
    const VkMemoryBarrier*                      pMemoryBarriers,
    uint32_t                                    bufferMemoryBarrierCount,
    const VkBufferMemoryBarrier*                pBufferMemoryBarriers,
    uint32_t                                    imageMemoryBarrierCount,
    const VkImageMemoryBarrier*                 pImageMemoryBarriers) const {
    bool skip = ValidationStateTracker::PreCallValidateCmdPipelineBarrier(commandBuffer, srcStageMask, dstStageMask, dependencyFlags, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCmdPipelineBarrier(commandBuffer, srcStageMask, dstStageMask, dependencyFlags, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCmdPipelineBarrier(
    VkCommandBuffer                             commandBuffer,
    VkPipelineStageFlags                        srcStageMask,
    VkPipelineStageFlags                        dstStageMask,
    VkDependencyFlags                           dependencyFlags,
    uint32_t                                    memoryBarrierCount,
    const VkMemoryBarrier*                      pMemoryBarriers,
    uint32_t                                    bufferMemoryBarrierCount,
    const VkBufferMemoryBarrier*                pBufferMemoryBarriers,
    uint32_t                                    imageMemoryBarrierCount,
    const VkImageMemoryBarrier*                 pImageMemoryBarriers) {
    ValidationStateTracker::PreCallRecordCmdPipelineBarrier(commandBuffer, srcStageMask, dstStageMask, dependencyFlags, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCmdPipelineBarrier(commandBuffer, srcStageMask, dstStageMask, dependencyFlags, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers); });
}

void BestPractices::PostCallRecordCmdPipelineBarrier(
    VkCommandBuffer                             commandBuffer,
    VkPipelineStageFlags                        srcStageMask,
    VkPipelineStageFlags                        dstStageMask,
    VkDependencyFlags                           dependencyFlags,
    uint32_t                                    memoryBarrierCount,
    const VkMemoryBarrier*                      pMemoryBarriers,
    uint32_t                                    bufferMemoryBarrierCount,
    const VkBufferMemoryBarrier*                pBufferMemoryBarriers,
    uint32_t                                    imageMemoryBarrierCount,
    const VkImageMemoryBarrier*                 pImageMemoryBarriers) {
    ValidationStateTracker::PostCallRecordCmdPipelineBarrier(commandBuffer, srcStageMask, dstStageMask, dependencyFlags, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCmdPipelineBarrier(commandBuffer, srcStageMask, dstStageMask, dependencyFlags, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers); });
}

bool BestPractices::PreCallValidateCmdBeginQuery(
    VkCommandBuffer                             commandBuffer,
    VkQueryPool                                 queryPool,
    uint32_t                                    query,
    VkQueryControlFlags                         flags) const {
    bool skip = ValidationStateTracker::PreCallValidateCmdBeginQuery(commandBuffer, queryPool, query, flags);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCmdBeginQuery(commandBuffer, queryPool, query, flags); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCmdBeginQuery(
    VkCommandBuffer                             commandBuffer,
    VkQueryPool                                 queryPool,
    uint32_t                                    query,
    VkQueryControlFlags                         flags) {
    ValidationStateTracker::PreCallRecordCmdBeginQuery(commandBuffer, queryPool, query, flags);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCmdBeginQuery(commandBuffer, queryPool, query, flags); });
}

void BestPractices::PostCallRecordCmdBeginQuery(
    VkCommandBuffer                             commandBuffer,
    VkQueryPool                                 queryPool,
    uint32_t                                    query,
    VkQueryControlFlags                         flags) {
    ValidationStateTracker::PostCallRecordCmdBeginQuery(commandBuffer, queryPool, query, flags);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCmdBeginQuery(commandBuffer, queryPool, query, flags); });
}

bool BestPractices::PreCallValidateCmdEndQuery(
    VkCommandBuffer                             commandBuffer,
    VkQueryPool                                 queryPool,
    uint32_t                                    query) const {
    bool skip = ValidationStateTracker::PreCallValidateCmdEndQuery(commandBuffer, queryPool, query);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCmdEndQuery(commandBuffer, queryPool, query); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCmdEndQuery(
    VkCommandBuffer                             commandBuffer,
    VkQueryPool                                 queryPool,
    uint32_t                                    query) {
    ValidationStateTracker::PreCallRecordCmdEndQuery(commandBuffer, queryPool, query);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCmdEndQuery(commandBuffer, queryPool, query); });
}

void BestPractices::PostCallRecordCmdEndQuery(
    VkCommandBuffer                             commandBuffer,
    VkQueryPool                                 queryPool,
    uint32_t                                    query) {
    ValidationStateTracker::PostCallRecordCmdEndQuery(commandBuffer, queryPool, query);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCmdEndQuery(commandBuffer, queryPool, query); });
}

bool BestPractices::PreCallValidateCmdResetQueryPool(
    VkCommandBuffer                             commandBuffer,
    VkQueryPool                                 queryPool,
    uint32_t                                    firstQuery,
    uint32_t                                    queryCount) const {
    bool skip = ValidationStateTracker::PreCallValidateCmdResetQueryPool(commandBuffer, queryPool, firstQuery, queryCount);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCmdResetQueryPool(commandBuffer, queryPool, firstQuery, queryCount); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCmdResetQueryPool(
    VkCommandBuffer                             commandBuffer,
    VkQueryPool                                 queryPool,
    uint32_t                                    firstQuery,
    uint32_t                                    queryCount) {
    ValidationStateTracker::PreCallRecordCmdResetQueryPool(commandBuffer, queryPool, firstQuery, queryCount);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCmdResetQueryPool(commandBuffer, queryPool, firstQuery, queryCount); });
}

void BestPractices::PostCallRecordCmdResetQueryPool(
    VkCommandBuffer                             commandBuffer,
    VkQueryPool                                 queryPool,
    uint32_t                                    firstQuery,
    uint32_t                                    queryCount) {
    ValidationStateTracker::PostCallRecordCmdResetQueryPool(commandBuffer, queryPool, firstQuery, queryCount);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCmdResetQueryPool(commandBuffer, queryPool, firstQuery, queryCount); });
}

bool BestPractices::PreCallValidateCmdWriteTimestamp(
    VkCommandBuffer                             commandBuffer,
    VkPipelineStageFlagBits                     pipelineStage,
    VkQueryPool                                 queryPool,
    uint32_t                                    query) const {
    bool skip = ValidationStateTracker::PreCallValidateCmdWriteTimestamp(commandBuffer, pipelineStage, queryPool, query);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCmdWriteTimestamp(commandBuffer, pipelineStage, queryPool, query); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCmdWriteTimestamp(
    VkCommandBuffer                             commandBuffer,
    VkPipelineStageFlagBits                     pipelineStage,
    VkQueryPool                                 queryPool,
    uint32_t                                    query) {
    ValidationStateTracker::PreCallRecordCmdWriteTimestamp(commandBuffer, pipelineStage, queryPool, query);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCmdWriteTimestamp(commandBuffer, pipelineStage, queryPool, query); });
}

void BestPractices::PostCallRecordCmdWriteTimestamp(
    VkCommandBuffer                             commandBuffer,
    VkPipelineStageFlagBits                     pipelineStage,
    VkQueryPool                                 queryPool,
    uint32_t                                    query) {
    ValidationStateTracker::PostCallRecordCmdWriteTimestamp(commandBuffer, pipelineStage, queryPool, query);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCmdWriteTimestamp(commandBuffer, pipelineStage, queryPool, query); });
}

bool BestPractices::PreCallValidateCmdCopyQueryPoolResults(
    VkCommandBuffer                             commandBuffer,
    VkQueryPool                                 queryPool,
    uint32_t                                    firstQuery,
    uint32_t                                    queryCount,
    VkBuffer                                    dstBuffer,
    VkDeviceSize                                dstOffset,
    VkDeviceSize                                stride,
    VkQueryResultFlags                          flags) const {
    bool skip = ValidationStateTracker::PreCallValidateCmdCopyQueryPoolResults(commandBuffer, queryPool, firstQuery, queryCount, dstBuffer, dstOffset, stride, flags);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCmdCopyQueryPoolResults(commandBuffer, queryPool, firstQuery, queryCount, dstBuffer, dstOffset, stride, flags); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCmdCopyQueryPoolResults(
    VkCommandBuffer                             commandBuffer,
    VkQueryPool                                 queryPool,
    uint32_t                                    firstQuery,
    uint32_t                                    queryCount,
    VkBuffer                                    dstBuffer,
    VkDeviceSize                                dstOffset,
    VkDeviceSize                                stride,
    VkQueryResultFlags                          flags) {
    ValidationStateTracker::PreCallRecordCmdCopyQueryPoolResults(commandBuffer, queryPool, firstQuery, queryCount, dstBuffer, dstOffset, stride, flags);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCmdCopyQueryPoolResults(commandBuffer, queryPool, firstQuery, queryCount, dstBuffer, dstOffset, stride, flags); });
}

void BestPractices::PostCallRecordCmdCopyQueryPoolResults(
    VkCommandBuffer                             commandBuffer,
    VkQueryPool                                 queryPool,
    uint32_t                                    firstQuery,
    uint32_t                                    queryCount,
    VkBuffer                                    dstBuffer,
    VkDeviceSize                                dstOffset,
    VkDeviceSize                                stride,
    VkQueryResultFlags                          flags) {
    ValidationStateTracker::PostCallRecordCmdCopyQueryPoolResults(commandBuffer, queryPool, firstQuery, queryCount, dstBuffer, dstOffset, stride, flags);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCmdCopyQueryPoolResults(commandBuffer, queryPool, firstQuery, queryCount, dstBuffer, dstOffset, stride, flags); });
}

bool BestPractices::PreCallValidateCmdPushConstants(
    VkCommandBuffer                             commandBuffer,
    VkPipelineLayout                            layout,
    VkShaderStageFlags                          stageFlags,
    uint32_t                                    offset,
    uint32_t                                    size,
    const void*                                 pValues) const {
    bool skip = ValidationStateTracker::PreCallValidateCmdPushConstants(commandBuffer, layout, stageFlags, offset, size, pValues);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCmdPushConstants(commandBuffer, layout, stageFlags, offset, size, pValues); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCmdPushConstants(
    VkCommandBuffer                             commandBuffer,
    VkPipelineLayout                            layout,
    VkShaderStageFlags                          stageFlags,
    uint32_t                                    offset,
    uint32_t                                    size,
    const void*                                 pValues) {
    ValidationStateTracker::PreCallRecordCmdPushConstants(commandBuffer, layout, stageFlags, offset, size, pValues);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCmdPushConstants(commandBuffer, layout, stageFlags, offset, size, pValues); });
}

void BestPractices::PostCallRecordCmdPushConstants(
    VkCommandBuffer                             commandBuffer,
    VkPipelineLayout                            layout,
    VkShaderStageFlags                          stageFlags,
    uint32_t                                    offset,
    uint32_t                                    size,
    const void*                                 pValues) {
    ValidationStateTracker::PostCallRecordCmdPushConstants(commandBuffer, layout, stageFlags, offset, size, pValues);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCmdPushConstants(commandBuffer, layout, stageFlags, offset, size, pValues); });
}

bool BestPractices::PreCallValidateCmdBeginRenderPass(
    VkCommandBuffer                             commandBuffer,
    const VkRenderPassBeginInfo*                pRenderPassBegin,
    VkSubpassContents                           contents) const {
    bool skip = ValidationStateTracker::PreCallValidateCmdBeginRenderPass(commandBuffer, pRenderPassBegin, contents);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCmdBeginRenderPass(commandBuffer, pRenderPassBegin, contents); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCmdBeginRenderPass(
    VkCommandBuffer                             commandBuffer,
    const VkRenderPassBeginInfo*                pRenderPassBegin,
    VkSubpassContents                           contents) {
    ValidationStateTracker::PreCallRecordCmdBeginRenderPass(commandBuffer, pRenderPassBegin, contents);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCmdBeginRenderPass(commandBuffer, pRenderPassBegin, contents); });
}

void BestPractices::PostCallRecordCmdBeginRenderPass(
    VkCommandBuffer                             commandBuffer,
    const VkRenderPassBeginInfo*                pRenderPassBegin,
    VkSubpassContents                           contents) {
    ValidationStateTracker::PostCallRecordCmdBeginRenderPass(commandBuffer, pRenderPassBegin, contents);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCmdBeginRenderPass(commandBuffer, pRenderPassBegin, contents); });
}

bool BestPractices::PreCallValidateCmdNextSubpass(
    VkCommandBuffer                             commandBuffer,
    VkSubpassContents                           contents) const {
    bool skip = ValidationStateTracker::PreCallValidateCmdNextSubpass(commandBuffer, contents);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCmdNextSubpass(commandBuffer, contents); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCmdNextSubpass(
    VkCommandBuffer                             commandBuffer,
    VkSubpassContents                           contents) {
    ValidationStateTracker::PreCallRecordCmdNextSubpass(commandBuffer, contents);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCmdNextSubpass(commandBuffer, contents); });
}

void BestPractices::PostCallRecordCmdNextSubpass(
    VkCommandBuffer                             commandBuffer,
    VkSubpassContents                           contents) {
    ValidationStateTracker::PostCallRecordCmdNextSubpass(commandBuffer, contents);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCmdNextSubpass(commandBuffer, contents); });
}

bool BestPractices::PreCallValidateCmdEndRenderPass(
    VkCommandBuffer                             commandBuffer) const {
    bool skip = ValidationStateTracker::PreCallValidateCmdEndRenderPass(commandBuffer);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCmdEndRenderPass(commandBuffer); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCmdEndRenderPass(
    VkCommandBuffer                             commandBuffer) {
    ValidationStateTracker::PreCallRecordCmdEndRenderPass(commandBuffer);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCmdEndRenderPass(commandBuffer); });
}

void BestPractices::PostCallRecordCmdEndRenderPass(
    VkCommandBuffer                             commandBuffer) {
    ValidationStateTracker::PostCallRecordCmdEndRenderPass(commandBuffer);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCmdEndRenderPass(commandBuffer); });
}

bool BestPractices::PreCallValidateCmdExecuteCommands(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    commandBufferCount,
    const VkCommandBuffer*                      pCommandBuffers) const {
    bool skip = ValidationStateTracker::PreCallValidateCmdExecuteCommands(commandBuffer, commandBufferCount, pCommandBuffers);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCmdExecuteCommands(commandBuffer, commandBufferCount, pCommandBuffers); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCmdExecuteCommands(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    commandBufferCount,
    const VkCommandBuffer*                      pCommandBuffers) {
    ValidationStateTracker::PreCallRecordCmdExecuteCommands(commandBuffer, commandBufferCount, pCommandBuffers);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCmdExecuteCommands(commandBuffer, commandBufferCount, pCommandBuffers); });
}

void BestPractices::PostCallRecordCmdExecuteCommands(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    commandBufferCount,
    const VkCommandBuffer*                      pCommandBuffers) {
    ValidationStateTracker::PostCallRecordCmdExecuteCommands(commandBuffer, commandBufferCount, pCommandBuffers);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCmdExecuteCommands(commandBuffer, commandBufferCount, pCommandBuffers); });
}

bool BestPractices::PreCallValidateBindBufferMemory2(
    VkDevice                                    device,
    uint32_t                                    bindInfoCount,
    const VkBindBufferMemoryInfo*               pBindInfos) const {
    bool skip = ValidationStateTracker::PreCallValidateBindBufferMemory2(device, bindInfoCount, pBindInfos);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateBindBufferMemory2(device, bindInfoCount, pBindInfos); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordBindBufferMemory2(
    VkDevice                                    device,
    uint32_t                                    bindInfoCount,
    const VkBindBufferMemoryInfo*               pBindInfos) {
    ValidationStateTracker::PreCallRecordBindBufferMemory2(device, bindInfoCount, pBindInfos);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordBindBufferMemory2(device, bindInfoCount, pBindInfos); });
}

void BestPractices::PostCallRecordBindBufferMemory2(
    VkDevice                                    device,
    uint32_t                                    bindInfoCount,
    const VkBindBufferMemoryInfo*               pBindInfos,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordBindBufferMemory2(device, bindInfoCount, pBindInfos, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordBindBufferMemory2(device, bindInfoCount, pBindInfos, result); });
}

bool BestPractices::PreCallValidateBindImageMemory2(
    VkDevice                                    device,
    uint32_t                                    bindInfoCount,
    const VkBindImageMemoryInfo*                pBindInfos) const {
    bool skip = ValidationStateTracker::PreCallValidateBindImageMemory2(device, bindInfoCount, pBindInfos);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateBindImageMemory2(device, bindInfoCount, pBindInfos); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordBindImageMemory2(
    VkDevice                                    device,
    uint32_t                                    bindInfoCount,
    const VkBindImageMemoryInfo*                pBindInfos) {
    ValidationStateTracker::PreCallRecordBindImageMemory2(device, bindInfoCount, pBindInfos);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordBindImageMemory2(device, bindInfoCount, pBindInfos); });
}

void BestPractices::PostCallRecordBindImageMemory2(
    VkDevice                                    device,
    uint32_t                                    bindInfoCount,
    const VkBindImageMemoryInfo*                pBindInfos,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordBindImageMemory2(device, bindInfoCount, pBindInfos, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordBindImageMemory2(device, bindInfoCount, pBindInfos, result); });
}

bool BestPractices::PreCallValidateGetDeviceGroupPeerMemoryFeatures(
    VkDevice                                    device,
    uint32_t                                    heapIndex,
    uint32_t                                    localDeviceIndex,
    uint32_t                                    remoteDeviceIndex,
    VkPeerMemoryFeatureFlags*                   pPeerMemoryFeatures) const {
    bool skip = ValidationStateTracker::PreCallValidateGetDeviceGroupPeerMemoryFeatures(device, heapIndex, localDeviceIndex, remoteDeviceIndex, pPeerMemoryFeatures);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetDeviceGroupPeerMemoryFeatures(device, heapIndex, localDeviceIndex, remoteDeviceIndex, pPeerMemoryFeatures); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetDeviceGroupPeerMemoryFeatures(
    VkDevice                                    device,
    uint32_t                                    heapIndex,
    uint32_t                                    localDeviceIndex,
    uint32_t                                    remoteDeviceIndex,
    VkPeerMemoryFeatureFlags*                   pPeerMemoryFeatures) {
    ValidationStateTracker::PreCallRecordGetDeviceGroupPeerMemoryFeatures(device, heapIndex, localDeviceIndex, remoteDeviceIndex, pPeerMemoryFeatures);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetDeviceGroupPeerMemoryFeatures(device, heapIndex, localDeviceIndex, remoteDeviceIndex, pPeerMemoryFeatures); });
}

void BestPractices::PostCallRecordGetDeviceGroupPeerMemoryFeatures(
    VkDevice                                    device,
    uint32_t                                    heapIndex,
    uint32_t                                    localDeviceIndex,
    uint32_t                                    remoteDeviceIndex,
    VkPeerMemoryFeatureFlags*                   pPeerMemoryFeatures) {
    ValidationStateTracker::PostCallRecordGetDeviceGroupPeerMemoryFeatures(device, heapIndex, localDeviceIndex, remoteDeviceIndex, pPeerMemoryFeatures);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetDeviceGroupPeerMemoryFeatures(device, heapIndex, localDeviceIndex, remoteDeviceIndex, pPeerMemoryFeatures); });
}

bool BestPractices::PreCallValidateCmdSetDeviceMask(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    deviceMask) const {
    bool skip = ValidationStateTracker::PreCallValidateCmdSetDeviceMask(commandBuffer, deviceMask);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCmdSetDeviceMask(commandBuffer, deviceMask); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCmdSetDeviceMask(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    deviceMask) {
    ValidationStateTracker::PreCallRecordCmdSetDeviceMask(commandBuffer, deviceMask);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCmdSetDeviceMask(commandBuffer, deviceMask); });
}

void BestPractices::PostCallRecordCmdSetDeviceMask(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    deviceMask) {
    ValidationStateTracker::PostCallRecordCmdSetDeviceMask(commandBuffer, deviceMask);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCmdSetDeviceMask(commandBuffer, deviceMask); });
}

bool BestPractices::PreCallValidateCmdDispatchBase(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    baseGroupX,
    uint32_t                                    baseGroupY,
    uint32_t                                    baseGroupZ,
    uint32_t                                    groupCountX,
    uint32_t                                    groupCountY,
    uint32_t                                    groupCountZ) const {
    bool skip = ValidationStateTracker::PreCallValidateCmdDispatchBase(commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCmdDispatchBase(commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCmdDispatchBase(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    baseGroupX,
    uint32_t                                    baseGroupY,
    uint32_t                                    baseGroupZ,
    uint32_t                                    groupCountX,
    uint32_t                                    groupCountY,
    uint32_t                                    groupCountZ) {
    ValidationStateTracker::PreCallRecordCmdDispatchBase(commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCmdDispatchBase(commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ); });
}

void BestPractices::PostCallRecordCmdDispatchBase(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    baseGroupX,
    uint32_t                                    baseGroupY,
    uint32_t                                    baseGroupZ,
    uint32_t                                    groupCountX,
    uint32_t                                    groupCountY,
    uint32_t                                    groupCountZ) {
    ValidationStateTracker::PostCallRecordCmdDispatchBase(commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCmdDispatchBase(commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ); });
}

bool BestPractices::PreCallValidateEnumeratePhysicalDeviceGroups(
    VkInstance                                  instance,
    uint32_t*                                   pPhysicalDeviceGroupCount,
    VkPhysicalDeviceGroupProperties*            pPhysicalDeviceGroupProperties) const {
    bool skip = ValidationStateTracker::PreCallValidateEnumeratePhysicalDeviceGroups(instance, pPhysicalDeviceGroupCount, pPhysicalDeviceGroupProperties);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateEnumeratePhysicalDeviceGroups(instance, pPhysicalDeviceGroupCount, pPhysicalDeviceGroupProperties); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordEnumeratePhysicalDeviceGroups(
    VkInstance                                  instance,
    uint32_t*                                   pPhysicalDeviceGroupCount,
    VkPhysicalDeviceGroupProperties*            pPhysicalDeviceGroupProperties) {
    ValidationStateTracker::PreCallRecordEnumeratePhysicalDeviceGroups(instance, pPhysicalDeviceGroupCount, pPhysicalDeviceGroupProperties);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordEnumeratePhysicalDeviceGroups(instance, pPhysicalDeviceGroupCount, pPhysicalDeviceGroupProperties); });
}

void BestPractices::PostCallRecordEnumeratePhysicalDeviceGroups(
    VkInstance                                  instance,
    uint32_t*                                   pPhysicalDeviceGroupCount,
    VkPhysicalDeviceGroupProperties*            pPhysicalDeviceGroupProperties,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordEnumeratePhysicalDeviceGroups(instance, pPhysicalDeviceGroupCount, pPhysicalDeviceGroupProperties, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordEnumeratePhysicalDeviceGroups(instance, pPhysicalDeviceGroupCount, pPhysicalDeviceGroupProperties, result); });
}

bool BestPractices::PreCallValidateGetImageMemoryRequirements2(
    VkDevice                                    device,
    const VkImageMemoryRequirementsInfo2*       pInfo,
    VkMemoryRequirements2*                      pMemoryRequirements) const {
    bool skip = ValidationStateTracker::PreCallValidateGetImageMemoryRequirements2(device, pInfo, pMemoryRequirements);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetImageMemoryRequirements2(device, pInfo, pMemoryRequirements); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetImageMemoryRequirements2(
    VkDevice                                    device,
    const VkImageMemoryRequirementsInfo2*       pInfo,
    VkMemoryRequirements2*                      pMemoryRequirements) {
    ValidationStateTracker::PreCallRecordGetImageMemoryRequirements2(device, pInfo, pMemoryRequirements);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetImageMemoryRequirements2(device, pInfo, pMemoryRequirements); });
}

void BestPractices::PostCallRecordGetImageMemoryRequirements2(
    VkDevice                                    device,
    const VkImageMemoryRequirementsInfo2*       pInfo,
    VkMemoryRequirements2*                      pMemoryRequirements) {
    ValidationStateTracker::PostCallRecordGetImageMemoryRequirements2(device, pInfo, pMemoryRequirements);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetImageMemoryRequirements2(device, pInfo, pMemoryRequirements); });
}

bool BestPractices::PreCallValidateGetBufferMemoryRequirements2(
    VkDevice                                    device,
    const VkBufferMemoryRequirementsInfo2*      pInfo,
    VkMemoryRequirements2*                      pMemoryRequirements) const {
    bool skip = ValidationStateTracker::PreCallValidateGetBufferMemoryRequirements2(device, pInfo, pMemoryRequirements);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetBufferMemoryRequirements2(device, pInfo, pMemoryRequirements); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetBufferMemoryRequirements2(
    VkDevice                                    device,
    const VkBufferMemoryRequirementsInfo2*      pInfo,
    VkMemoryRequirements2*                      pMemoryRequirements) {
    ValidationStateTracker::PreCallRecordGetBufferMemoryRequirements2(device, pInfo, pMemoryRequirements);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetBufferMemoryRequirements2(device, pInfo, pMemoryRequirements); });
}

void BestPractices::PostCallRecordGetBufferMemoryRequirements2(
    VkDevice                                    device,
    const VkBufferMemoryRequirementsInfo2*      pInfo,
    VkMemoryRequirements2*                      pMemoryRequirements) {
    ValidationStateTracker::PostCallRecordGetBufferMemoryRequirements2(device, pInfo, pMemoryRequirements);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetBufferMemoryRequirements2(device, pInfo, pMemoryRequirements); });
}

bool BestPractices::PreCallValidateGetImageSparseMemoryRequirements2(
    VkDevice                                    device,
    const VkImageSparseMemoryRequirementsInfo2* pInfo,
    uint32_t*                                   pSparseMemoryRequirementCount,
    VkSparseImageMemoryRequirements2*           pSparseMemoryRequirements) const {
    bool skip = ValidationStateTracker::PreCallValidateGetImageSparseMemoryRequirements2(device, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetImageSparseMemoryRequirements2(device, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetImageSparseMemoryRequirements2(
    VkDevice                                    device,
    const VkImageSparseMemoryRequirementsInfo2* pInfo,
    uint32_t*                                   pSparseMemoryRequirementCount,
    VkSparseImageMemoryRequirements2*           pSparseMemoryRequirements) {
    ValidationStateTracker::PreCallRecordGetImageSparseMemoryRequirements2(device, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetImageSparseMemoryRequirements2(device, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements); });
}

void BestPractices::PostCallRecordGetImageSparseMemoryRequirements2(
    VkDevice                                    device,
    const VkImageSparseMemoryRequirementsInfo2* pInfo,
    uint32_t*                                   pSparseMemoryRequirementCount,
    VkSparseImageMemoryRequirements2*           pSparseMemoryRequirements) {
    ValidationStateTracker::PostCallRecordGetImageSparseMemoryRequirements2(device, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetImageSparseMemoryRequirements2(device, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements); });
}

bool BestPractices::PreCallValidateGetPhysicalDeviceFeatures2(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceFeatures2*                  pFeatures) const {
    bool skip = ValidationStateTracker::PreCallValidateGetPhysicalDeviceFeatures2(physicalDevice, pFeatures);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetPhysicalDeviceFeatures2(physicalDevice, pFeatures); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetPhysicalDeviceFeatures2(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceFeatures2*                  pFeatures) {
    ValidationStateTracker::PreCallRecordGetPhysicalDeviceFeatures2(physicalDevice, pFeatures);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetPhysicalDeviceFeatures2(physicalDevice, pFeatures); });
}

void BestPractices::PostCallRecordGetPhysicalDeviceFeatures2(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceFeatures2*                  pFeatures) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceFeatures2(physicalDevice, pFeatures);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetPhysicalDeviceFeatures2(physicalDevice, pFeatures); });
}

bool BestPractices::PreCallValidateGetPhysicalDeviceProperties2(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceProperties2*                pProperties) const {
    bool skip = ValidationStateTracker::PreCallValidateGetPhysicalDeviceProperties2(physicalDevice, pProperties);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetPhysicalDeviceProperties2(physicalDevice, pProperties); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetPhysicalDeviceProperties2(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceProperties2*                pProperties) {
    ValidationStateTracker::PreCallRecordGetPhysicalDeviceProperties2(physicalDevice, pProperties);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetPhysicalDeviceProperties2(physicalDevice, pProperties); });
}

void BestPractices::PostCallRecordGetPhysicalDeviceProperties2(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceProperties2*                pProperties) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceProperties2(physicalDevice, pProperties);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetPhysicalDeviceProperties2(physicalDevice, pProperties); });
}

bool BestPractices::PreCallValidateGetPhysicalDeviceFormatProperties2(
    VkPhysicalDevice                            physicalDevice,
    VkFormat                                    format,
    VkFormatProperties2*                        pFormatProperties) const {
    bool skip = ValidationStateTracker::PreCallValidateGetPhysicalDeviceFormatProperties2(physicalDevice, format, pFormatProperties);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetPhysicalDeviceFormatProperties2(physicalDevice, format, pFormatProperties); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetPhysicalDeviceFormatProperties2(
    VkPhysicalDevice                            physicalDevice,
    VkFormat                                    format,
    VkFormatProperties2*                        pFormatProperties) {
    ValidationStateTracker::PreCallRecordGetPhysicalDeviceFormatProperties2(physicalDevice, format, pFormatProperties);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetPhysicalDeviceFormatProperties2(physicalDevice, format, pFormatProperties); });
}

void BestPractices::PostCallRecordGetPhysicalDeviceFormatProperties2(
    VkPhysicalDevice                            physicalDevice,
    VkFormat                                    format,
    VkFormatProperties2*                        pFormatProperties) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceFormatProperties2(physicalDevice, format, pFormatProperties);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetPhysicalDeviceFormatProperties2(physicalDevice, format, pFormatProperties); });
}

bool BestPractices::PreCallValidateGetPhysicalDeviceImageFormatProperties2(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceImageFormatInfo2*     pImageFormatInfo,
    VkImageFormatProperties2*                   pImageFormatProperties) const {
    bool skip = ValidationStateTracker::PreCallValidateGetPhysicalDeviceImageFormatProperties2(physicalDevice, pImageFormatInfo, pImageFormatProperties);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetPhysicalDeviceImageFormatProperties2(physicalDevice, pImageFormatInfo, pImageFormatProperties); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetPhysicalDeviceImageFormatProperties2(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceImageFormatInfo2*     pImageFormatInfo,
    VkImageFormatProperties2*                   pImageFormatProperties) {
    ValidationStateTracker::PreCallRecordGetPhysicalDeviceImageFormatProperties2(physicalDevice, pImageFormatInfo, pImageFormatProperties);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetPhysicalDeviceImageFormatProperties2(physicalDevice, pImageFormatInfo, pImageFormatProperties); });
}

void BestPractices::PostCallRecordGetPhysicalDeviceImageFormatProperties2(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceImageFormatInfo2*     pImageFormatInfo,
    VkImageFormatProperties2*                   pImageFormatProperties,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceImageFormatProperties2(physicalDevice, pImageFormatInfo, pImageFormatProperties, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetPhysicalDeviceImageFormatProperties2(physicalDevice, pImageFormatInfo, pImageFormatProperties, result); });
}

bool BestPractices::PreCallValidateGetPhysicalDeviceQueueFamilyProperties2(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pQueueFamilyPropertyCount,
    VkQueueFamilyProperties2*                   pQueueFamilyProperties) const {
    bool skip = ValidationStateTracker::PreCallValidateGetPhysicalDeviceQueueFamilyProperties2(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetPhysicalDeviceQueueFamilyProperties2(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetPhysicalDeviceQueueFamilyProperties2(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pQueueFamilyPropertyCount,
    VkQueueFamilyProperties2*                   pQueueFamilyProperties) {
    ValidationStateTracker::PreCallRecordGetPhysicalDeviceQueueFamilyProperties2(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetPhysicalDeviceQueueFamilyProperties2(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties); });
}

void BestPractices::PostCallRecordGetPhysicalDeviceQueueFamilyProperties2(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pQueueFamilyPropertyCount,
    VkQueueFamilyProperties2*                   pQueueFamilyProperties) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceQueueFamilyProperties2(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetPhysicalDeviceQueueFamilyProperties2(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties); });
}

bool BestPractices::PreCallValidateGetPhysicalDeviceMemoryProperties2(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceMemoryProperties2*          pMemoryProperties) const {
    bool skip = ValidationStateTracker::PreCallValidateGetPhysicalDeviceMemoryProperties2(physicalDevice, pMemoryProperties);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetPhysicalDeviceMemoryProperties2(physicalDevice, pMemoryProperties); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetPhysicalDeviceMemoryProperties2(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceMemoryProperties2*          pMemoryProperties) {
    ValidationStateTracker::PreCallRecordGetPhysicalDeviceMemoryProperties2(physicalDevice, pMemoryProperties);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetPhysicalDeviceMemoryProperties2(physicalDevice, pMemoryProperties); });
}

void BestPractices::PostCallRecordGetPhysicalDeviceMemoryProperties2(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceMemoryProperties2*          pMemoryProperties) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceMemoryProperties2(physicalDevice, pMemoryProperties);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetPhysicalDeviceMemoryProperties2(physicalDevice, pMemoryProperties); });
}

bool BestPractices::PreCallValidateGetPhysicalDeviceSparseImageFormatProperties2(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceSparseImageFormatInfo2* pFormatInfo,
    uint32_t*                                   pPropertyCount,
    VkSparseImageFormatProperties2*             pProperties) const {
    bool skip = ValidationStateTracker::PreCallValidateGetPhysicalDeviceSparseImageFormatProperties2(physicalDevice, pFormatInfo, pPropertyCount, pProperties);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetPhysicalDeviceSparseImageFormatProperties2(physicalDevice, pFormatInfo, pPropertyCount, pProperties); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetPhysicalDeviceSparseImageFormatProperties2(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceSparseImageFormatInfo2* pFormatInfo,
    uint32_t*                                   pPropertyCount,
    VkSparseImageFormatProperties2*             pProperties) {
    ValidationStateTracker::PreCallRecordGetPhysicalDeviceSparseImageFormatProperties2(physicalDevice, pFormatInfo, pPropertyCount, pProperties);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetPhysicalDeviceSparseImageFormatProperties2(physicalDevice, pFormatInfo, pPropertyCount, pProperties); });
}

void BestPractices::PostCallRecordGetPhysicalDeviceSparseImageFormatProperties2(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceSparseImageFormatInfo2* pFormatInfo,
    uint32_t*                                   pPropertyCount,
    VkSparseImageFormatProperties2*             pProperties) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceSparseImageFormatProperties2(physicalDevice, pFormatInfo, pPropertyCount, pProperties);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetPhysicalDeviceSparseImageFormatProperties2(physicalDevice, pFormatInfo, pPropertyCount, pProperties); });
}

bool BestPractices::PreCallValidateTrimCommandPool(
    VkDevice                                    device,
    VkCommandPool                               commandPool,
    VkCommandPoolTrimFlags                      flags) const {
    bool skip = ValidationStateTracker::PreCallValidateTrimCommandPool(device, commandPool, flags);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateTrimCommandPool(device, commandPool, flags); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordTrimCommandPool(
    VkDevice                                    device,
    VkCommandPool                               commandPool,
    VkCommandPoolTrimFlags                      flags) {
    ValidationStateTracker::PreCallRecordTrimCommandPool(device, commandPool, flags);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordTrimCommandPool(device, commandPool, flags); });
}

void BestPractices::PostCallRecordTrimCommandPool(
    VkDevice                                    device,
    VkCommandPool                               commandPool,
    VkCommandPoolTrimFlags                      flags) {
    ValidationStateTracker::PostCallRecordTrimCommandPool(device, commandPool, flags);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordTrimCommandPool(device, commandPool, flags); });
}

bool BestPractices::PreCallValidateGetDeviceQueue2(
    VkDevice                                    device,
    const VkDeviceQueueInfo2*                   pQueueInfo,
    VkQueue*                                    pQueue) const {
    bool skip = ValidationStateTracker::PreCallValidateGetDeviceQueue2(device, pQueueInfo, pQueue);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetDeviceQueue2(device, pQueueInfo, pQueue); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetDeviceQueue2(
    VkDevice                                    device,
    const VkDeviceQueueInfo2*                   pQueueInfo,
    VkQueue*                                    pQueue) {
    ValidationStateTracker::PreCallRecordGetDeviceQueue2(device, pQueueInfo, pQueue);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetDeviceQueue2(device, pQueueInfo, pQueue); });
}

void BestPractices::PostCallRecordGetDeviceQueue2(
    VkDevice                                    device,
    const VkDeviceQueueInfo2*                   pQueueInfo,
    VkQueue*                                    pQueue) {
    ValidationStateTracker::PostCallRecordGetDeviceQueue2(device, pQueueInfo, pQueue);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetDeviceQueue2(device, pQueueInfo, pQueue); });
}

bool BestPractices::PreCallValidateCreateSamplerYcbcrConversion(
    VkDevice                                    device,
    const VkSamplerYcbcrConversionCreateInfo*   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSamplerYcbcrConversion*                   pYcbcrConversion) const {
    bool skip = ValidationStateTracker::PreCallValidateCreateSamplerYcbcrConversion(device, pCreateInfo, pAllocator, pYcbcrConversion);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCreateSamplerYcbcrConversion(device, pCreateInfo, pAllocator, pYcbcrConversion); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCreateSamplerYcbcrConversion(
    VkDevice                                    device,
    const VkSamplerYcbcrConversionCreateInfo*   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSamplerYcbcrConversion*                   pYcbcrConversion) {
    ValidationStateTracker::PreCallRecordCreateSamplerYcbcrConversion(device, pCreateInfo, pAllocator, pYcbcrConversion);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCreateSamplerYcbcrConversion(device, pCreateInfo, pAllocator, pYcbcrConversion); });
}

void BestPractices::PostCallRecordCreateSamplerYcbcrConversion(
    VkDevice                                    device,
    const VkSamplerYcbcrConversionCreateInfo*   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSamplerYcbcrConversion*                   pYcbcrConversion,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCreateSamplerYcbcrConversion(device, pCreateInfo, pAllocator, pYcbcrConversion, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCreateSamplerYcbcrConversion(device, pCreateInfo, pAllocator, pYcbcrConversion, result); });
}

bool BestPractices::PreCallValidateDestroySamplerYcbcrConversion(
    VkDevice                                    device,
    VkSamplerYcbcrConversion                    ycbcrConversion,
    const VkAllocationCallbacks*                pAllocator) const {
    bool skip = ValidationStateTracker::PreCallValidateDestroySamplerYcbcrConversion(device, ycbcrConversion, pAllocator);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateDestroySamplerYcbcrConversion(device, ycbcrConversion, pAllocator); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordDestroySamplerYcbcrConversion(
    VkDevice                                    device,
    VkSamplerYcbcrConversion                    ycbcrConversion,
    const VkAllocationCallbacks*                pAllocator) {
    ValidationStateTracker::PreCallRecordDestroySamplerYcbcrConversion(device, ycbcrConversion, pAllocator);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordDestroySamplerYcbcrConversion(device, ycbcrConversion, pAllocator); });
}

void BestPractices::PostCallRecordDestroySamplerYcbcrConversion(
    VkDevice                                    device,
    VkSamplerYcbcrConversion                    ycbcrConversion,
    const VkAllocationCallbacks*                pAllocator) {
    ValidationStateTracker::PostCallRecordDestroySamplerYcbcrConversion(device, ycbcrConversion, pAllocator);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordDestroySamplerYcbcrConversion(device, ycbcrConversion, pAllocator); });
}

bool BestPractices::PreCallValidateCreateDescriptorUpdateTemplate(
    VkDevice                                    device,
    const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDescriptorUpdateTemplate*                 pDescriptorUpdateTemplate) const {
    bool skip = ValidationStateTracker::PreCallValidateCreateDescriptorUpdateTemplate(device, pCreateInfo, pAllocator, pDescriptorUpdateTemplate);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCreateDescriptorUpdateTemplate(device, pCreateInfo, pAllocator, pDescriptorUpdateTemplate); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCreateDescriptorUpdateTemplate(
    VkDevice                                    device,
    const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDescriptorUpdateTemplate*                 pDescriptorUpdateTemplate) {
    ValidationStateTracker::PreCallRecordCreateDescriptorUpdateTemplate(device, pCreateInfo, pAllocator, pDescriptorUpdateTemplate);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCreateDescriptorUpdateTemplate(device, pCreateInfo, pAllocator, pDescriptorUpdateTemplate); });
}

void BestPractices::PostCallRecordCreateDescriptorUpdateTemplate(
    VkDevice                                    device,
    const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDescriptorUpdateTemplate*                 pDescriptorUpdateTemplate,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCreateDescriptorUpdateTemplate(device, pCreateInfo, pAllocator, pDescriptorUpdateTemplate, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCreateDescriptorUpdateTemplate(device, pCreateInfo, pAllocator, pDescriptorUpdateTemplate, result); });
}

bool BestPractices::PreCallValidateDestroyDescriptorUpdateTemplate(
    VkDevice                                    device,
    VkDescriptorUpdateTemplate                  descriptorUpdateTemplate,
    const VkAllocationCallbacks*                pAllocator) const {
    bool skip = ValidationStateTracker::PreCallValidateDestroyDescriptorUpdateTemplate(device, descriptorUpdateTemplate, pAllocator);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateDestroyDescriptorUpdateTemplate(device, descriptorUpdateTemplate, pAllocator); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordDestroyDescriptorUpdateTemplate(
    VkDevice                                    device,
    VkDescriptorUpdateTemplate                  descriptorUpdateTemplate,
    const VkAllocationCallbacks*                pAllocator) {
    ValidationStateTracker::PreCallRecordDestroyDescriptorUpdateTemplate(device, descriptorUpdateTemplate, pAllocator);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordDestroyDescriptorUpdateTemplate(device, descriptorUpdateTemplate, pAllocator); });
}

void BestPractices::PostCallRecordDestroyDescriptorUpdateTemplate(
    VkDevice                                    device,
    VkDescriptorUpdateTemplate                  descriptorUpdateTemplate,
    const VkAllocationCallbacks*                pAllocator) {
    ValidationStateTracker::PostCallRecordDestroyDescriptorUpdateTemplate(device, descriptorUpdateTemplate, pAllocator);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordDestroyDescriptorUpdateTemplate(device, descriptorUpdateTemplate, pAllocator); });
}

bool BestPractices::PreCallValidateUpdateDescriptorSetWithTemplate(
    VkDevice                                    device,
    VkDescriptorSet                             descriptorSet,
    VkDescriptorUpdateTemplate                  descriptorUpdateTemplate,
    const void*                                 pData) const {
    bool skip = ValidationStateTracker::PreCallValidateUpdateDescriptorSetWithTemplate(device, descriptorSet, descriptorUpdateTemplate, pData);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateUpdateDescriptorSetWithTemplate(device, descriptorSet, descriptorUpdateTemplate, pData); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordUpdateDescriptorSetWithTemplate(
    VkDevice                                    device,
    VkDescriptorSet                             descriptorSet,
    VkDescriptorUpdateTemplate                  descriptorUpdateTemplate,
    const void*                                 pData) {
    ValidationStateTracker::PreCallRecordUpdateDescriptorSetWithTemplate(device, descriptorSet, descriptorUpdateTemplate, pData);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordUpdateDescriptorSetWithTemplate(device, descriptorSet, descriptorUpdateTemplate, pData); });
}

void BestPractices::PostCallRecordUpdateDescriptorSetWithTemplate(
    VkDevice                                    device,
    VkDescriptorSet                             descriptorSet,
    VkDescriptorUpdateTemplate                  descriptorUpdateTemplate,
    const void*                                 pData) {
    ValidationStateTracker::PostCallRecordUpdateDescriptorSetWithTemplate(device, descriptorSet, descriptorUpdateTemplate, pData);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordUpdateDescriptorSetWithTemplate(device, descriptorSet, descriptorUpdateTemplate, pData); });
}

bool BestPractices::PreCallValidateGetPhysicalDeviceExternalBufferProperties(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceExternalBufferInfo*   pExternalBufferInfo,
    VkExternalBufferProperties*                 pExternalBufferProperties) const {
    bool skip = ValidationStateTracker::PreCallValidateGetPhysicalDeviceExternalBufferProperties(physicalDevice, pExternalBufferInfo, pExternalBufferProperties);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetPhysicalDeviceExternalBufferProperties(physicalDevice, pExternalBufferInfo, pExternalBufferProperties); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetPhysicalDeviceExternalBufferProperties(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceExternalBufferInfo*   pExternalBufferInfo,
    VkExternalBufferProperties*                 pExternalBufferProperties) {
    ValidationStateTracker::PreCallRecordGetPhysicalDeviceExternalBufferProperties(physicalDevice, pExternalBufferInfo, pExternalBufferProperties);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetPhysicalDeviceExternalBufferProperties(physicalDevice, pExternalBufferInfo, pExternalBufferProperties); });
}

void BestPractices::PostCallRecordGetPhysicalDeviceExternalBufferProperties(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceExternalBufferInfo*   pExternalBufferInfo,
    VkExternalBufferProperties*                 pExternalBufferProperties) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceExternalBufferProperties(physicalDevice, pExternalBufferInfo, pExternalBufferProperties);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetPhysicalDeviceExternalBufferProperties(physicalDevice, pExternalBufferInfo, pExternalBufferProperties); });
}

bool BestPractices::PreCallValidateGetPhysicalDeviceExternalFenceProperties(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceExternalFenceInfo*    pExternalFenceInfo,
    VkExternalFenceProperties*                  pExternalFenceProperties) const {
    bool skip = ValidationStateTracker::PreCallValidateGetPhysicalDeviceExternalFenceProperties(physicalDevice, pExternalFenceInfo, pExternalFenceProperties);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetPhysicalDeviceExternalFenceProperties(physicalDevice, pExternalFenceInfo, pExternalFenceProperties); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetPhysicalDeviceExternalFenceProperties(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceExternalFenceInfo*    pExternalFenceInfo,
    VkExternalFenceProperties*                  pExternalFenceProperties) {
    ValidationStateTracker::PreCallRecordGetPhysicalDeviceExternalFenceProperties(physicalDevice, pExternalFenceInfo, pExternalFenceProperties);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetPhysicalDeviceExternalFenceProperties(physicalDevice, pExternalFenceInfo, pExternalFenceProperties); });
}

void BestPractices::PostCallRecordGetPhysicalDeviceExternalFenceProperties(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceExternalFenceInfo*    pExternalFenceInfo,
    VkExternalFenceProperties*                  pExternalFenceProperties) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceExternalFenceProperties(physicalDevice, pExternalFenceInfo, pExternalFenceProperties);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetPhysicalDeviceExternalFenceProperties(physicalDevice, pExternalFenceInfo, pExternalFenceProperties); });
}

bool BestPractices::PreCallValidateGetPhysicalDeviceExternalSemaphoreProperties(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceExternalSemaphoreInfo* pExternalSemaphoreInfo,
    VkExternalSemaphoreProperties*              pExternalSemaphoreProperties) const {
    bool skip = ValidationStateTracker::PreCallValidateGetPhysicalDeviceExternalSemaphoreProperties(physicalDevice, pExternalSemaphoreInfo, pExternalSemaphoreProperties);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetPhysicalDeviceExternalSemaphoreProperties(physicalDevice, pExternalSemaphoreInfo, pExternalSemaphoreProperties); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetPhysicalDeviceExternalSemaphoreProperties(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceExternalSemaphoreInfo* pExternalSemaphoreInfo,
    VkExternalSemaphoreProperties*              pExternalSemaphoreProperties) {
    ValidationStateTracker::PreCallRecordGetPhysicalDeviceExternalSemaphoreProperties(physicalDevice, pExternalSemaphoreInfo, pExternalSemaphoreProperties);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetPhysicalDeviceExternalSemaphoreProperties(physicalDevice, pExternalSemaphoreInfo, pExternalSemaphoreProperties); });
}

void BestPractices::PostCallRecordGetPhysicalDeviceExternalSemaphoreProperties(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceExternalSemaphoreInfo* pExternalSemaphoreInfo,
    VkExternalSemaphoreProperties*              pExternalSemaphoreProperties) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceExternalSemaphoreProperties(physicalDevice, pExternalSemaphoreInfo, pExternalSemaphoreProperties);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetPhysicalDeviceExternalSemaphoreProperties(physicalDevice, pExternalSemaphoreInfo, pExternalSemaphoreProperties); });
}

bool BestPractices::PreCallValidateGetDescriptorSetLayoutSupport(
    VkDevice                                    device,
    const VkDescriptorSetLayoutCreateInfo*      pCreateInfo,
    VkDescriptorSetLayoutSupport*               pSupport) const {
    bool skip = ValidationStateTracker::PreCallValidateGetDescriptorSetLayoutSupport(device, pCreateInfo, pSupport);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetDescriptorSetLayoutSupport(device, pCreateInfo, pSupport); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetDescriptorSetLayoutSupport(
    VkDevice                                    device,
    const VkDescriptorSetLayoutCreateInfo*      pCreateInfo,
    VkDescriptorSetLayoutSupport*               pSupport) {
    ValidationStateTracker::PreCallRecordGetDescriptorSetLayoutSupport(device, pCreateInfo, pSupport);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetDescriptorSetLayoutSupport(device, pCreateInfo, pSupport); });
}

void BestPractices::PostCallRecordGetDescriptorSetLayoutSupport(
    VkDevice                                    device,
    const VkDescriptorSetLayoutCreateInfo*      pCreateInfo,
    VkDescriptorSetLayoutSupport*               pSupport) {
    ValidationStateTracker::PostCallRecordGetDescriptorSetLayoutSupport(device, pCreateInfo, pSupport);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetDescriptorSetLayoutSupport(device, pCreateInfo, pSupport); });
}

bool BestPractices::PreCallValidateCmdDrawIndirectCount(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkBuffer                                    countBuffer,
    VkDeviceSize                                countBufferOffset,
    uint32_t                                    maxDrawCount,
    uint32_t                                    stride) const {
    bool skip = ValidationStateTracker::PreCallValidateCmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCmdDrawIndirectCount(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkBuffer                                    countBuffer,
    VkDeviceSize                                countBufferOffset,
    uint32_t                                    maxDrawCount,
    uint32_t                                    stride) {
    ValidationStateTracker::PreCallRecordCmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride); });
}

void BestPractices::PostCallRecordCmdDrawIndirectCount(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkBuffer                                    countBuffer,
    VkDeviceSize                                countBufferOffset,
    uint32_t                                    maxDrawCount,
    uint32_t                                    stride) {
    ValidationStateTracker::PostCallRecordCmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride); });
}

bool BestPractices::PreCallValidateCmdDrawIndexedIndirectCount(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkBuffer                                    countBuffer,
    VkDeviceSize                                countBufferOffset,
    uint32_t                                    maxDrawCount,
    uint32_t                                    stride) const {
    bool skip = ValidationStateTracker::PreCallValidateCmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCmdDrawIndexedIndirectCount(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkBuffer                                    countBuffer,
    VkDeviceSize                                countBufferOffset,
    uint32_t                                    maxDrawCount,
    uint32_t                                    stride) {
    ValidationStateTracker::PreCallRecordCmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride); });
}

void BestPractices::PostCallRecordCmdDrawIndexedIndirectCount(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkBuffer                                    countBuffer,
    VkDeviceSize                                countBufferOffset,
    uint32_t                                    maxDrawCount,
    uint32_t                                    stride) {
    ValidationStateTracker::PostCallRecordCmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride); });
}

bool BestPractices::PreCallValidateCreateRenderPass2(
    VkDevice                                    device,
    const VkRenderPassCreateInfo2*              pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkRenderPass*                               pRenderPass) const {
    bool skip = ValidationStateTracker::PreCallValidateCreateRenderPass2(device, pCreateInfo, pAllocator, pRenderPass);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCreateRenderPass2(device, pCreateInfo, pAllocator, pRenderPass); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCreateRenderPass2(
    VkDevice                                    device,
    const VkRenderPassCreateInfo2*              pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkRenderPass*                               pRenderPass) {
    ValidationStateTracker::PreCallRecordCreateRenderPass2(device, pCreateInfo, pAllocator, pRenderPass);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCreateRenderPass2(device, pCreateInfo, pAllocator, pRenderPass); });
}

void BestPractices::PostCallRecordCreateRenderPass2(
    VkDevice                                    device,
    const VkRenderPassCreateInfo2*              pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkRenderPass*                               pRenderPass,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCreateRenderPass2(device, pCreateInfo, pAllocator, pRenderPass, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCreateRenderPass2(device, pCreateInfo, pAllocator, pRenderPass, result); });
}

bool BestPractices::PreCallValidateCmdBeginRenderPass2(
    VkCommandBuffer                             commandBuffer,
    const VkRenderPassBeginInfo*                pRenderPassBegin,
    const VkSubpassBeginInfo*                   pSubpassBeginInfo) const {
    bool skip = ValidationStateTracker::PreCallValidateCmdBeginRenderPass2(commandBuffer, pRenderPassBegin, pSubpassBeginInfo);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCmdBeginRenderPass2(commandBuffer, pRenderPassBegin, pSubpassBeginInfo); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCmdBeginRenderPass2(
    VkCommandBuffer                             commandBuffer,
    const VkRenderPassBeginInfo*                pRenderPassBegin,
    const VkSubpassBeginInfo*                   pSubpassBeginInfo) {
    ValidationStateTracker::PreCallRecordCmdBeginRenderPass2(commandBuffer, pRenderPassBegin, pSubpassBeginInfo);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCmdBeginRenderPass2(commandBuffer, pRenderPassBegin, pSubpassBeginInfo); });
}

void BestPractices::PostCallRecordCmdBeginRenderPass2(
    VkCommandBuffer                             commandBuffer,
    const VkRenderPassBeginInfo*                pRenderPassBegin,
    const VkSubpassBeginInfo*                   pSubpassBeginInfo) {
    ValidationStateTracker::PostCallRecordCmdBeginRenderPass2(commandBuffer, pRenderPassBegin, pSubpassBeginInfo);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCmdBeginRenderPass2(commandBuffer, pRenderPassBegin, pSubpassBeginInfo); });
}

bool BestPractices::PreCallValidateCmdNextSubpass2(
    VkCommandBuffer                             commandBuffer,
    const VkSubpassBeginInfo*                   pSubpassBeginInfo,
    const VkSubpassEndInfo*                     pSubpassEndInfo) const {
    bool skip = ValidationStateTracker::PreCallValidateCmdNextSubpass2(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCmdNextSubpass2(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCmdNextSubpass2(
    VkCommandBuffer                             commandBuffer,
    const VkSubpassBeginInfo*                   pSubpassBeginInfo,
    const VkSubpassEndInfo*                     pSubpassEndInfo) {
    ValidationStateTracker::PreCallRecordCmdNextSubpass2(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCmdNextSubpass2(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo); });
}

void BestPractices::PostCallRecordCmdNextSubpass2(
    VkCommandBuffer                             commandBuffer,
    const VkSubpassBeginInfo*                   pSubpassBeginInfo,
    const VkSubpassEndInfo*                     pSubpassEndInfo) {
    ValidationStateTracker::PostCallRecordCmdNextSubpass2(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCmdNextSubpass2(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo); });
}

bool BestPractices::PreCallValidateCmdEndRenderPass2(
    VkCommandBuffer                             commandBuffer,
    const VkSubpassEndInfo*                     pSubpassEndInfo) const {
    bool skip = ValidationStateTracker::PreCallValidateCmdEndRenderPass2(commandBuffer, pSubpassEndInfo);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCmdEndRenderPass2(commandBuffer, pSubpassEndInfo); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCmdEndRenderPass2(
    VkCommandBuffer                             commandBuffer,
    const VkSubpassEndInfo*                     pSubpassEndInfo) {
    ValidationStateTracker::PreCallRecordCmdEndRenderPass2(commandBuffer, pSubpassEndInfo);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCmdEndRenderPass2(commandBuffer, pSubpassEndInfo); });
}

void BestPractices::PostCallRecordCmdEndRenderPass2(
    VkCommandBuffer                             commandBuffer,
    const VkSubpassEndInfo*                     pSubpassEndInfo) {
    ValidationStateTracker::PostCallRecordCmdEndRenderPass2(commandBuffer, pSubpassEndInfo);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCmdEndRenderPass2(commandBuffer, pSubpassEndInfo); });
}

bool BestPractices::PreCallValidateResetQueryPool(
    VkDevice                                    device,
    VkQueryPool                                 queryPool,
    uint32_t                                    firstQuery,
    uint32_t                                    queryCount) const {
    bool skip = ValidationStateTracker::PreCallValidateResetQueryPool(device, queryPool, firstQuery, queryCount);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateResetQueryPool(device, queryPool, firstQuery, queryCount); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordResetQueryPool(
    VkDevice                                    device,
    VkQueryPool                                 queryPool,
    uint32_t                                    firstQuery,
    uint32_t                                    queryCount) {
    ValidationStateTracker::PreCallRecordResetQueryPool(device, queryPool, firstQuery, queryCount);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordResetQueryPool(device, queryPool, firstQuery, queryCount); });
}

void BestPractices::PostCallRecordResetQueryPool(
    VkDevice                                    device,
    VkQueryPool                                 queryPool,
    uint32_t                                    firstQuery,
    uint32_t                                    queryCount) {
    ValidationStateTracker::PostCallRecordResetQueryPool(device, queryPool, firstQuery, queryCount);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordResetQueryPool(device, queryPool, firstQuery, queryCount); });
}

bool BestPractices::PreCallValidateGetSemaphoreCounterValue(
    VkDevice                                    device,
    VkSemaphore                                 semaphore,
    uint64_t*                                   pValue) const {
    bool skip = ValidationStateTracker::PreCallValidateGetSemaphoreCounterValue(device, semaphore, pValue);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetSemaphoreCounterValue(device, semaphore, pValue); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetSemaphoreCounterValue(
    VkDevice                                    device,
    VkSemaphore                                 semaphore,
    uint64_t*                                   pValue) {
    ValidationStateTracker::PreCallRecordGetSemaphoreCounterValue(device, semaphore, pValue);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetSemaphoreCounterValue(device, semaphore, pValue); });
}

void BestPractices::PostCallRecordGetSemaphoreCounterValue(
    VkDevice                                    device,
    VkSemaphore                                 semaphore,
    uint64_t*                                   pValue,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetSemaphoreCounterValue(device, semaphore, pValue, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetSemaphoreCounterValue(device, semaphore, pValue, result); });
}

bool BestPractices::PreCallValidateWaitSemaphores(
    VkDevice                                    device,
    const VkSemaphoreWaitInfo*                  pWaitInfo,
    uint64_t                                    timeout) const {
    bool skip = ValidationStateTracker::PreCallValidateWaitSemaphores(device, pWaitInfo, timeout);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateWaitSemaphores(device, pWaitInfo, timeout); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordWaitSemaphores(
    VkDevice                                    device,
    const VkSemaphoreWaitInfo*                  pWaitInfo,
    uint64_t                                    timeout) {
    ValidationStateTracker::PreCallRecordWaitSemaphores(device, pWaitInfo, timeout);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordWaitSemaphores(device, pWaitInfo, timeout); });
}

void BestPractices::PostCallRecordWaitSemaphores(
    VkDevice                                    device,
    const VkSemaphoreWaitInfo*                  pWaitInfo,
    uint64_t                                    timeout,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordWaitSemaphores(device, pWaitInfo, timeout, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordWaitSemaphores(device, pWaitInfo, timeout, result); });
}

bool BestPractices::PreCallValidateSignalSemaphore(
    VkDevice                                    device,
    const VkSemaphoreSignalInfo*                pSignalInfo) const {
    bool skip = ValidationStateTracker::PreCallValidateSignalSemaphore(device, pSignalInfo);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateSignalSemaphore(device, pSignalInfo); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordSignalSemaphore(
    VkDevice                                    device,
    const VkSemaphoreSignalInfo*                pSignalInfo) {
    ValidationStateTracker::PreCallRecordSignalSemaphore(device, pSignalInfo);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordSignalSemaphore(device, pSignalInfo); });
}

void BestPractices::PostCallRecordSignalSemaphore(
    VkDevice                                    device,
    const VkSemaphoreSignalInfo*                pSignalInfo,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordSignalSemaphore(device, pSignalInfo, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordSignalSemaphore(device, pSignalInfo, result); });
}

bool BestPractices::PreCallValidateGetBufferDeviceAddress(
    VkDevice                                    device,
    const VkBufferDeviceAddressInfo*            pInfo) const {
    bool skip = ValidationStateTracker::PreCallValidateGetBufferDeviceAddress(device, pInfo);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetBufferDeviceAddress(device, pInfo); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetBufferDeviceAddress(
    VkDevice                                    device,
    const VkBufferDeviceAddressInfo*            pInfo) {
    ValidationStateTracker::PreCallRecordGetBufferDeviceAddress(device, pInfo);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetBufferDeviceAddress(device, pInfo); });
}

void BestPractices::PostCallRecordGetBufferDeviceAddress(
    VkDevice                                    device,
    const VkBufferDeviceAddressInfo*            pInfo,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetBufferDeviceAddress(device, pInfo, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetBufferDeviceAddress(device, pInfo, result); });
}

bool BestPractices::PreCallValidateGetBufferOpaqueCaptureAddress(
    VkDevice                                    device,
    const VkBufferDeviceAddressInfo*            pInfo) const {
    bool skip = ValidationStateTracker::PreCallValidateGetBufferOpaqueCaptureAddress(device, pInfo);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetBufferOpaqueCaptureAddress(device, pInfo); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetBufferOpaqueCaptureAddress(
    VkDevice                                    device,
    const VkBufferDeviceAddressInfo*            pInfo) {
    ValidationStateTracker::PreCallRecordGetBufferOpaqueCaptureAddress(device, pInfo);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetBufferOpaqueCaptureAddress(device, pInfo); });
}

void BestPractices::PostCallRecordGetBufferOpaqueCaptureAddress(
    VkDevice                                    device,
    const VkBufferDeviceAddressInfo*            pInfo) {
    ValidationStateTracker::PostCallRecordGetBufferOpaqueCaptureAddress(device, pInfo);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetBufferOpaqueCaptureAddress(device, pInfo); });
}

bool BestPractices::PreCallValidateGetDeviceMemoryOpaqueCaptureAddress(
    VkDevice                                    device,
    const VkDeviceMemoryOpaqueCaptureAddressInfo* pInfo) const {
    bool skip = ValidationStateTracker::PreCallValidateGetDeviceMemoryOpaqueCaptureAddress(device, pInfo);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetDeviceMemoryOpaqueCaptureAddress(device, pInfo); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetDeviceMemoryOpaqueCaptureAddress(
    VkDevice                                    device,
    const VkDeviceMemoryOpaqueCaptureAddressInfo* pInfo) {
    ValidationStateTracker::PreCallRecordGetDeviceMemoryOpaqueCaptureAddress(device, pInfo);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetDeviceMemoryOpaqueCaptureAddress(device, pInfo); });
}

void BestPractices::PostCallRecordGetDeviceMemoryOpaqueCaptureAddress(
    VkDevice                                    device,
    const VkDeviceMemoryOpaqueCaptureAddressInfo* pInfo) {
    ValidationStateTracker::PostCallRecordGetDeviceMemoryOpaqueCaptureAddress(device, pInfo);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetDeviceMemoryOpaqueCaptureAddress(device, pInfo); });
}

bool BestPractices::PreCallValidateDestroySurfaceKHR(
    VkInstance                                  instance,
    VkSurfaceKHR                                surface,
    const VkAllocationCallbacks*                pAllocator) const {
    bool skip = ValidationStateTracker::PreCallValidateDestroySurfaceKHR(instance, surface, pAllocator);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateDestroySurfaceKHR(instance, surface, pAllocator); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordDestroySurfaceKHR(
    VkInstance                                  instance,
    VkSurfaceKHR                                surface,
    const VkAllocationCallbacks*                pAllocator) {
    ValidationStateTracker::PreCallRecordDestroySurfaceKHR(instance, surface, pAllocator);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordDestroySurfaceKHR(instance, surface, pAllocator); });
}

void BestPractices::PostCallRecordDestroySurfaceKHR(
    VkInstance                                  instance,
    VkSurfaceKHR                                surface,
    const VkAllocationCallbacks*                pAllocator) {
    ValidationStateTracker::PostCallRecordDestroySurfaceKHR(instance, surface, pAllocator);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordDestroySurfaceKHR(instance, surface, pAllocator); });
}

bool BestPractices::PreCallValidateGetPhysicalDeviceSurfaceSupportKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    queueFamilyIndex,
    VkSurfaceKHR                                surface,
    VkBool32*                                   pSupported) const {
    bool skip = ValidationStateTracker::PreCallValidateGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, queueFamilyIndex, surface, pSupported);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, queueFamilyIndex, surface, pSupported); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetPhysicalDeviceSurfaceSupportKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    queueFamilyIndex,
    VkSurfaceKHR                                surface,
    VkBool32*                                   pSupported) {
    ValidationStateTracker::PreCallRecordGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, queueFamilyIndex, surface, pSupported);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, queueFamilyIndex, surface, pSupported); });
}

void BestPractices::PostCallRecordGetPhysicalDeviceSurfaceSupportKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    queueFamilyIndex,
    VkSurfaceKHR                                surface,
    VkBool32*                                   pSupported,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, queueFamilyIndex, surface, pSupported, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, queueFamilyIndex, surface, pSupported, result); });
}

bool BestPractices::PreCallValidateGetPhysicalDeviceSurfaceCapabilitiesKHR(
    VkPhysicalDevice                            physicalDevice,
    VkSurfaceKHR                                surface,
    VkSurfaceCapabilitiesKHR*                   pSurfaceCapabilities) const {
    bool skip = ValidationStateTracker::PreCallValidateGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, pSurfaceCapabilities);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, pSurfaceCapabilities); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetPhysicalDeviceSurfaceCapabilitiesKHR(
    VkPhysicalDevice                            physicalDevice,
    VkSurfaceKHR                                surface,
    VkSurfaceCapabilitiesKHR*                   pSurfaceCapabilities) {
    ValidationStateTracker::PreCallRecordGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, pSurfaceCapabilities);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, pSurfaceCapabilities); });
}

void BestPractices::PostCallRecordGetPhysicalDeviceSurfaceCapabilitiesKHR(
    VkPhysicalDevice                            physicalDevice,
    VkSurfaceKHR                                surface,
    VkSurfaceCapabilitiesKHR*                   pSurfaceCapabilities,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, pSurfaceCapabilities, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, pSurfaceCapabilities, result); });
}

bool BestPractices::PreCallValidateGetPhysicalDeviceSurfaceFormatsKHR(
    VkPhysicalDevice                            physicalDevice,
    VkSurfaceKHR                                surface,
    uint32_t*                                   pSurfaceFormatCount,
    VkSurfaceFormatKHR*                         pSurfaceFormats) const {
    bool skip = ValidationStateTracker::PreCallValidateGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, pSurfaceFormatCount, pSurfaceFormats);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, pSurfaceFormatCount, pSurfaceFormats); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetPhysicalDeviceSurfaceFormatsKHR(
    VkPhysicalDevice                            physicalDevice,
    VkSurfaceKHR                                surface,
    uint32_t*                                   pSurfaceFormatCount,
    VkSurfaceFormatKHR*                         pSurfaceFormats) {
    ValidationStateTracker::PreCallRecordGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, pSurfaceFormatCount, pSurfaceFormats);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, pSurfaceFormatCount, pSurfaceFormats); });
}

void BestPractices::PostCallRecordGetPhysicalDeviceSurfaceFormatsKHR(
    VkPhysicalDevice                            physicalDevice,
    VkSurfaceKHR                                surface,
    uint32_t*                                   pSurfaceFormatCount,
    VkSurfaceFormatKHR*                         pSurfaceFormats,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, pSurfaceFormatCount, pSurfaceFormats, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, pSurfaceFormatCount, pSurfaceFormats, result); });
}

bool BestPractices::PreCallValidateGetPhysicalDeviceSurfacePresentModesKHR(
    VkPhysicalDevice                            physicalDevice,
    VkSurfaceKHR                                surface,
    uint32_t*                                   pPresentModeCount,
    VkPresentModeKHR*                           pPresentModes) const {
    bool skip = ValidationStateTracker::PreCallValidateGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, pPresentModeCount, pPresentModes);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, pPresentModeCount, pPresentModes); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetPhysicalDeviceSurfacePresentModesKHR(
    VkPhysicalDevice                            physicalDevice,
    VkSurfaceKHR                                surface,
    uint32_t*                                   pPresentModeCount,
    VkPresentModeKHR*                           pPresentModes) {
    ValidationStateTracker::PreCallRecordGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, pPresentModeCount, pPresentModes);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, pPresentModeCount, pPresentModes); });
}

void BestPractices::PostCallRecordGetPhysicalDeviceSurfacePresentModesKHR(
    VkPhysicalDevice                            physicalDevice,
    VkSurfaceKHR                                surface,
    uint32_t*                                   pPresentModeCount,
    VkPresentModeKHR*                           pPresentModes,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, pPresentModeCount, pPresentModes, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, pPresentModeCount, pPresentModes, result); });
}

bool BestPractices::PreCallValidateCreateSwapchainKHR(
    VkDevice                                    device,
    const VkSwapchainCreateInfoKHR*             pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSwapchainKHR*                             pSwapchain) const {
    bool skip = ValidationStateTracker::PreCallValidateCreateSwapchainKHR(device, pCreateInfo, pAllocator, pSwapchain);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCreateSwapchainKHR(device, pCreateInfo, pAllocator, pSwapchain); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCreateSwapchainKHR(
    VkDevice                                    device,
    const VkSwapchainCreateInfoKHR*             pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSwapchainKHR*                             pSwapchain) {
    ValidationStateTracker::PreCallRecordCreateSwapchainKHR(device, pCreateInfo, pAllocator, pSwapchain);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCreateSwapchainKHR(device, pCreateInfo, pAllocator, pSwapchain); });
}

void BestPractices::PostCallRecordCreateSwapchainKHR(
    VkDevice                                    device,
    const VkSwapchainCreateInfoKHR*             pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSwapchainKHR*                             pSwapchain,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCreateSwapchainKHR(device, pCreateInfo, pAllocator, pSwapchain, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCreateSwapchainKHR(device, pCreateInfo, pAllocator, pSwapchain, result); });
}

bool BestPractices::PreCallValidateDestroySwapchainKHR(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    const VkAllocationCallbacks*                pAllocator) const {
    bool skip = ValidationStateTracker::PreCallValidateDestroySwapchainKHR(device, swapchain, pAllocator);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateDestroySwapchainKHR(device, swapchain, pAllocator); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordDestroySwapchainKHR(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    const VkAllocationCallbacks*                pAllocator) {
    ValidationStateTracker::PreCallRecordDestroySwapchainKHR(device, swapchain, pAllocator);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordDestroySwapchainKHR(device, swapchain, pAllocator); });
}

void BestPractices::PostCallRecordDestroySwapchainKHR(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    const VkAllocationCallbacks*                pAllocator) {
    ValidationStateTracker::PostCallRecordDestroySwapchainKHR(device, swapchain, pAllocator);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordDestroySwapchainKHR(device, swapchain, pAllocator); });
}

bool BestPractices::PreCallValidateGetSwapchainImagesKHR(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    uint32_t*                                   pSwapchainImageCount,
    VkImage*                                    pSwapchainImages) const {
    bool skip = ValidationStateTracker::PreCallValidateGetSwapchainImagesKHR(device, swapchain, pSwapchainImageCount, pSwapchainImages);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetSwapchainImagesKHR(device, swapchain, pSwapchainImageCount, pSwapchainImages); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetSwapchainImagesKHR(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    uint32_t*                                   pSwapchainImageCount,
    VkImage*                                    pSwapchainImages) {
    ValidationStateTracker::PreCallRecordGetSwapchainImagesKHR(device, swapchain, pSwapchainImageCount, pSwapchainImages);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetSwapchainImagesKHR(device, swapchain, pSwapchainImageCount, pSwapchainImages); });
}

void BestPractices::PostCallRecordGetSwapchainImagesKHR(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    uint32_t*                                   pSwapchainImageCount,
    VkImage*                                    pSwapchainImages,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetSwapchainImagesKHR(device, swapchain, pSwapchainImageCount, pSwapchainImages, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetSwapchainImagesKHR(device, swapchain, pSwapchainImageCount, pSwapchainImages, result); });
}

bool BestPractices::PreCallValidateAcquireNextImageKHR(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    uint64_t                                    timeout,
    VkSemaphore                                 semaphore,
    VkFence                                     fence,
    uint32_t*                                   pImageIndex) const {
    bool skip = ValidationStateTracker::PreCallValidateAcquireNextImageKHR(device, swapchain, timeout, semaphore, fence, pImageIndex);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateAcquireNextImageKHR(device, swapchain, timeout, semaphore, fence, pImageIndex); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordAcquireNextImageKHR(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    uint64_t                                    timeout,
    VkSemaphore                                 semaphore,
    VkFence                                     fence,
    uint32_t*                                   pImageIndex) {
    ValidationStateTracker::PreCallRecordAcquireNextImageKHR(device, swapchain, timeout, semaphore, fence, pImageIndex);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordAcquireNextImageKHR(device, swapchain, timeout, semaphore, fence, pImageIndex); });
}

void BestPractices::PostCallRecordAcquireNextImageKHR(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    uint64_t                                    timeout,
    VkSemaphore                                 semaphore,
    VkFence                                     fence,
    uint32_t*                                   pImageIndex,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordAcquireNextImageKHR(device, swapchain, timeout, semaphore, fence, pImageIndex, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordAcquireNextImageKHR(device, swapchain, timeout, semaphore, fence, pImageIndex, result); });
}

bool BestPractices::PreCallValidateQueuePresentKHR(
    VkQueue                                     queue,
    const VkPresentInfoKHR*                     pPresentInfo) const {
    bool skip = ValidationStateTracker::PreCallValidateQueuePresentKHR(queue, pPresentInfo);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateQueuePresentKHR(queue, pPresentInfo); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordQueuePresentKHR(
    VkQueue                                     queue,
    const VkPresentInfoKHR*                     pPresentInfo) {
    ValidationStateTracker::PreCallRecordQueuePresentKHR(queue, pPresentInfo);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordQueuePresentKHR(queue, pPresentInfo); });
}

void BestPractices::PostCallRecordQueuePresentKHR(
    VkQueue                                     queue,
    const VkPresentInfoKHR*                     pPresentInfo,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordQueuePresentKHR(queue, pPresentInfo, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordQueuePresentKHR(queue, pPresentInfo, result); });
}

bool BestPractices::PreCallValidateGetDeviceGroupPresentCapabilitiesKHR(
    VkDevice                                    device,
    VkDeviceGroupPresentCapabilitiesKHR*        pDeviceGroupPresentCapabilities) const {
    bool skip = ValidationStateTracker::PreCallValidateGetDeviceGroupPresentCapabilitiesKHR(device, pDeviceGroupPresentCapabilities);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetDeviceGroupPresentCapabilitiesKHR(device, pDeviceGroupPresentCapabilities); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetDeviceGroupPresentCapabilitiesKHR(
    VkDevice                                    device,
    VkDeviceGroupPresentCapabilitiesKHR*        pDeviceGroupPresentCapabilities) {
    ValidationStateTracker::PreCallRecordGetDeviceGroupPresentCapabilitiesKHR(device, pDeviceGroupPresentCapabilities);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetDeviceGroupPresentCapabilitiesKHR(device, pDeviceGroupPresentCapabilities); });
}

void BestPractices::PostCallRecordGetDeviceGroupPresentCapabilitiesKHR(
    VkDevice                                    device,
    VkDeviceGroupPresentCapabilitiesKHR*        pDeviceGroupPresentCapabilities,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetDeviceGroupPresentCapabilitiesKHR(device, pDeviceGroupPresentCapabilities, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetDeviceGroupPresentCapabilitiesKHR(device, pDeviceGroupPresentCapabilities, result); });
}

bool BestPractices::PreCallValidateGetDeviceGroupSurfacePresentModesKHR(
    VkDevice                                    device,
    VkSurfaceKHR                                surface,
    VkDeviceGroupPresentModeFlagsKHR*           pModes) const {
    bool skip = ValidationStateTracker::PreCallValidateGetDeviceGroupSurfacePresentModesKHR(device, surface, pModes);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetDeviceGroupSurfacePresentModesKHR(device, surface, pModes); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetDeviceGroupSurfacePresentModesKHR(
    VkDevice                                    device,
    VkSurfaceKHR                                surface,
    VkDeviceGroupPresentModeFlagsKHR*           pModes) {
    ValidationStateTracker::PreCallRecordGetDeviceGroupSurfacePresentModesKHR(device, surface, pModes);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetDeviceGroupSurfacePresentModesKHR(device, surface, pModes); });
}

void BestPractices::PostCallRecordGetDeviceGroupSurfacePresentModesKHR(
    VkDevice                                    device,
    VkSurfaceKHR                                surface,
    VkDeviceGroupPresentModeFlagsKHR*           pModes,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetDeviceGroupSurfacePresentModesKHR(device, surface, pModes, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetDeviceGroupSurfacePresentModesKHR(device, surface, pModes, result); });
}

bool BestPractices::PreCallValidateGetPhysicalDevicePresentRectanglesKHR(
    VkPhysicalDevice                            physicalDevice,
    VkSurfaceKHR                                surface,
    uint32_t*                                   pRectCount,
    VkRect2D*                                   pRects) const {
    bool skip = ValidationStateTracker::PreCallValidateGetPhysicalDevicePresentRectanglesKHR(physicalDevice, surface, pRectCount, pRects);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetPhysicalDevicePresentRectanglesKHR(physicalDevice, surface, pRectCount, pRects); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetPhysicalDevicePresentRectanglesKHR(
    VkPhysicalDevice                            physicalDevice,
    VkSurfaceKHR                                surface,
    uint32_t*                                   pRectCount,
    VkRect2D*                                   pRects) {
    ValidationStateTracker::PreCallRecordGetPhysicalDevicePresentRectanglesKHR(physicalDevice, surface, pRectCount, pRects);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetPhysicalDevicePresentRectanglesKHR(physicalDevice, surface, pRectCount, pRects); });
}

void BestPractices::PostCallRecordGetPhysicalDevicePresentRectanglesKHR(
    VkPhysicalDevice                            physicalDevice,
    VkSurfaceKHR                                surface,
    uint32_t*                                   pRectCount,
    VkRect2D*                                   pRects,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetPhysicalDevicePresentRectanglesKHR(physicalDevice, surface, pRectCount, pRects, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetPhysicalDevicePresentRectanglesKHR(physicalDevice, surface, pRectCount, pRects, result); });
}

bool BestPractices::PreCallValidateAcquireNextImage2KHR(
    VkDevice                                    device,
    const VkAcquireNextImageInfoKHR*            pAcquireInfo,
    uint32_t*                                   pImageIndex) const {
    bool skip = ValidationStateTracker::PreCallValidateAcquireNextImage2KHR(device, pAcquireInfo, pImageIndex);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateAcquireNextImage2KHR(device, pAcquireInfo, pImageIndex); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordAcquireNextImage2KHR(
    VkDevice                                    device,
    const VkAcquireNextImageInfoKHR*            pAcquireInfo,
    uint32_t*                                   pImageIndex) {
    ValidationStateTracker::PreCallRecordAcquireNextImage2KHR(device, pAcquireInfo, pImageIndex);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordAcquireNextImage2KHR(device, pAcquireInfo, pImageIndex); });
}

void BestPractices::PostCallRecordAcquireNextImage2KHR(
    VkDevice                                    device,
    const VkAcquireNextImageInfoKHR*            pAcquireInfo,
    uint32_t*                                   pImageIndex,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordAcquireNextImage2KHR(device, pAcquireInfo, pImageIndex, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordAcquireNextImage2KHR(device, pAcquireInfo, pImageIndex, result); });
}

bool BestPractices::PreCallValidateGetPhysicalDeviceDisplayPropertiesKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pPropertyCount,
    VkDisplayPropertiesKHR*                     pProperties) const {
    bool skip = ValidationStateTracker::PreCallValidateGetPhysicalDeviceDisplayPropertiesKHR(physicalDevice, pPropertyCount, pProperties);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetPhysicalDeviceDisplayPropertiesKHR(physicalDevice, pPropertyCount, pProperties); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetPhysicalDeviceDisplayPropertiesKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pPropertyCount,
    VkDisplayPropertiesKHR*                     pProperties) {
    ValidationStateTracker::PreCallRecordGetPhysicalDeviceDisplayPropertiesKHR(physicalDevice, pPropertyCount, pProperties);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetPhysicalDeviceDisplayPropertiesKHR(physicalDevice, pPropertyCount, pProperties); });
}

void BestPractices::PostCallRecordGetPhysicalDeviceDisplayPropertiesKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pPropertyCount,
    VkDisplayPropertiesKHR*                     pProperties,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceDisplayPropertiesKHR(physicalDevice, pPropertyCount, pProperties, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetPhysicalDeviceDisplayPropertiesKHR(physicalDevice, pPropertyCount, pProperties, result); });
}

bool BestPractices::PreCallValidateGetPhysicalDeviceDisplayPlanePropertiesKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pPropertyCount,
    VkDisplayPlanePropertiesKHR*                pProperties) const {
    bool skip = ValidationStateTracker::PreCallValidateGetPhysicalDeviceDisplayPlanePropertiesKHR(physicalDevice, pPropertyCount, pProperties);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetPhysicalDeviceDisplayPlanePropertiesKHR(physicalDevice, pPropertyCount, pProperties); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetPhysicalDeviceDisplayPlanePropertiesKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pPropertyCount,
    VkDisplayPlanePropertiesKHR*                pProperties) {
    ValidationStateTracker::PreCallRecordGetPhysicalDeviceDisplayPlanePropertiesKHR(physicalDevice, pPropertyCount, pProperties);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetPhysicalDeviceDisplayPlanePropertiesKHR(physicalDevice, pPropertyCount, pProperties); });
}

void BestPractices::PostCallRecordGetPhysicalDeviceDisplayPlanePropertiesKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pPropertyCount,
    VkDisplayPlanePropertiesKHR*                pProperties,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceDisplayPlanePropertiesKHR(physicalDevice, pPropertyCount, pProperties, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetPhysicalDeviceDisplayPlanePropertiesKHR(physicalDevice, pPropertyCount, pProperties, result); });
}

bool BestPractices::PreCallValidateGetDisplayPlaneSupportedDisplaysKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    planeIndex,
    uint32_t*                                   pDisplayCount,
    VkDisplayKHR*                               pDisplays) const {
    bool skip = ValidationStateTracker::PreCallValidateGetDisplayPlaneSupportedDisplaysKHR(physicalDevice, planeIndex, pDisplayCount, pDisplays);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetDisplayPlaneSupportedDisplaysKHR(physicalDevice, planeIndex, pDisplayCount, pDisplays); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetDisplayPlaneSupportedDisplaysKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    planeIndex,
    uint32_t*                                   pDisplayCount,
    VkDisplayKHR*                               pDisplays) {
    ValidationStateTracker::PreCallRecordGetDisplayPlaneSupportedDisplaysKHR(physicalDevice, planeIndex, pDisplayCount, pDisplays);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetDisplayPlaneSupportedDisplaysKHR(physicalDevice, planeIndex, pDisplayCount, pDisplays); });
}

void BestPractices::PostCallRecordGetDisplayPlaneSupportedDisplaysKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    planeIndex,
    uint32_t*                                   pDisplayCount,
    VkDisplayKHR*                               pDisplays,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetDisplayPlaneSupportedDisplaysKHR(physicalDevice, planeIndex, pDisplayCount, pDisplays, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetDisplayPlaneSupportedDisplaysKHR(physicalDevice, planeIndex, pDisplayCount, pDisplays, result); });
}

bool BestPractices::PreCallValidateGetDisplayModePropertiesKHR(
    VkPhysicalDevice                            physicalDevice,
    VkDisplayKHR                                display,
    uint32_t*                                   pPropertyCount,
    VkDisplayModePropertiesKHR*                 pProperties) const {
    bool skip = ValidationStateTracker::PreCallValidateGetDisplayModePropertiesKHR(physicalDevice, display, pPropertyCount, pProperties);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetDisplayModePropertiesKHR(physicalDevice, display, pPropertyCount, pProperties); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetDisplayModePropertiesKHR(
    VkPhysicalDevice                            physicalDevice,
    VkDisplayKHR                                display,
    uint32_t*                                   pPropertyCount,
    VkDisplayModePropertiesKHR*                 pProperties) {
    ValidationStateTracker::PreCallRecordGetDisplayModePropertiesKHR(physicalDevice, display, pPropertyCount, pProperties);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetDisplayModePropertiesKHR(physicalDevice, display, pPropertyCount, pProperties); });
}

void BestPractices::PostCallRecordGetDisplayModePropertiesKHR(
    VkPhysicalDevice                            physicalDevice,
    VkDisplayKHR                                display,
    uint32_t*                                   pPropertyCount,
    VkDisplayModePropertiesKHR*                 pProperties,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetDisplayModePropertiesKHR(physicalDevice, display, pPropertyCount, pProperties, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetDisplayModePropertiesKHR(physicalDevice, display, pPropertyCount, pProperties, result); });
}

bool BestPractices::PreCallValidateCreateDisplayModeKHR(
    VkPhysicalDevice                            physicalDevice,
    VkDisplayKHR                                display,
    const VkDisplayModeCreateInfoKHR*           pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDisplayModeKHR*                           pMode) const {
    bool skip = ValidationStateTracker::PreCallValidateCreateDisplayModeKHR(physicalDevice, display, pCreateInfo, pAllocator, pMode);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCreateDisplayModeKHR(physicalDevice, display, pCreateInfo, pAllocator, pMode); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCreateDisplayModeKHR(
    VkPhysicalDevice                            physicalDevice,
    VkDisplayKHR                                display,
    const VkDisplayModeCreateInfoKHR*           pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDisplayModeKHR*                           pMode) {
    ValidationStateTracker::PreCallRecordCreateDisplayModeKHR(physicalDevice, display, pCreateInfo, pAllocator, pMode);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCreateDisplayModeKHR(physicalDevice, display, pCreateInfo, pAllocator, pMode); });
}

void BestPractices::PostCallRecordCreateDisplayModeKHR(
    VkPhysicalDevice                            physicalDevice,
    VkDisplayKHR                                display,
    const VkDisplayModeCreateInfoKHR*           pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDisplayModeKHR*                           pMode,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCreateDisplayModeKHR(physicalDevice, display, pCreateInfo, pAllocator, pMode, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCreateDisplayModeKHR(physicalDevice, display, pCreateInfo, pAllocator, pMode, result); });
}

bool BestPractices::PreCallValidateGetDisplayPlaneCapabilitiesKHR(
    VkPhysicalDevice                            physicalDevice,
    VkDisplayModeKHR                            mode,
    uint32_t                                    planeIndex,
    VkDisplayPlaneCapabilitiesKHR*              pCapabilities) const {
    bool skip = ValidationStateTracker::PreCallValidateGetDisplayPlaneCapabilitiesKHR(physicalDevice, mode, planeIndex, pCapabilities);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetDisplayPlaneCapabilitiesKHR(physicalDevice, mode, planeIndex, pCapabilities); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetDisplayPlaneCapabilitiesKHR(
    VkPhysicalDevice                            physicalDevice,
    VkDisplayModeKHR                            mode,
    uint32_t                                    planeIndex,
    VkDisplayPlaneCapabilitiesKHR*              pCapabilities) {
    ValidationStateTracker::PreCallRecordGetDisplayPlaneCapabilitiesKHR(physicalDevice, mode, planeIndex, pCapabilities);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetDisplayPlaneCapabilitiesKHR(physicalDevice, mode, planeIndex, pCapabilities); });
}

void BestPractices::PostCallRecordGetDisplayPlaneCapabilitiesKHR(
    VkPhysicalDevice                            physicalDevice,
    VkDisplayModeKHR                            mode,
    uint32_t                                    planeIndex,
    VkDisplayPlaneCapabilitiesKHR*              pCapabilities,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetDisplayPlaneCapabilitiesKHR(physicalDevice, mode, planeIndex, pCapabilities, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetDisplayPlaneCapabilitiesKHR(physicalDevice, mode, planeIndex, pCapabilities, result); });
}

bool BestPractices::PreCallValidateCreateDisplayPlaneSurfaceKHR(
    VkInstance                                  instance,
    const VkDisplaySurfaceCreateInfoKHR*        pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) const {
    bool skip = ValidationStateTracker::PreCallValidateCreateDisplayPlaneSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCreateDisplayPlaneSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCreateDisplayPlaneSurfaceKHR(
    VkInstance                                  instance,
    const VkDisplaySurfaceCreateInfoKHR*        pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) {
    ValidationStateTracker::PreCallRecordCreateDisplayPlaneSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCreateDisplayPlaneSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface); });
}

void BestPractices::PostCallRecordCreateDisplayPlaneSurfaceKHR(
    VkInstance                                  instance,
    const VkDisplaySurfaceCreateInfoKHR*        pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCreateDisplayPlaneSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCreateDisplayPlaneSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface, result); });
}

bool BestPractices::PreCallValidateCreateSharedSwapchainsKHR(
    VkDevice                                    device,
    uint32_t                                    swapchainCount,
    const VkSwapchainCreateInfoKHR*             pCreateInfos,
    const VkAllocationCallbacks*                pAllocator,
    VkSwapchainKHR*                             pSwapchains) const {
    bool skip = ValidationStateTracker::PreCallValidateCreateSharedSwapchainsKHR(device, swapchainCount, pCreateInfos, pAllocator, pSwapchains);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCreateSharedSwapchainsKHR(device, swapchainCount, pCreateInfos, pAllocator, pSwapchains); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCreateSharedSwapchainsKHR(
    VkDevice                                    device,
    uint32_t                                    swapchainCount,
    const VkSwapchainCreateInfoKHR*             pCreateInfos,
    const VkAllocationCallbacks*                pAllocator,
    VkSwapchainKHR*                             pSwapchains) {
    ValidationStateTracker::PreCallRecordCreateSharedSwapchainsKHR(device, swapchainCount, pCreateInfos, pAllocator, pSwapchains);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCreateSharedSwapchainsKHR(device, swapchainCount, pCreateInfos, pAllocator, pSwapchains); });
}

void BestPractices::PostCallRecordCreateSharedSwapchainsKHR(
    VkDevice                                    device,
    uint32_t                                    swapchainCount,
    const VkSwapchainCreateInfoKHR*             pCreateInfos,
    const VkAllocationCallbacks*                pAllocator,
    VkSwapchainKHR*                             pSwapchains,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCreateSharedSwapchainsKHR(device, swapchainCount, pCreateInfos, pAllocator, pSwapchains, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCreateSharedSwapchainsKHR(device, swapchainCount, pCreateInfos, pAllocator, pSwapchains, result); });
}

bool BestPractices::PreCallValidateCreateXlibSurfaceKHR(
    VkInstance                                  instance,
    const VkXlibSurfaceCreateInfoKHR*           pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) const {
    bool skip = ValidationStateTracker::PreCallValidateCreateXlibSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCreateXlibSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCreateXlibSurfaceKHR(
    VkInstance                                  instance,
    const VkXlibSurfaceCreateInfoKHR*           pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) {
    ValidationStateTracker::PreCallRecordCreateXlibSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCreateXlibSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface); });
}

void BestPractices::PostCallRecordCreateXlibSurfaceKHR(
    VkInstance                                  instance,
    const VkXlibSurfaceCreateInfoKHR*           pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCreateXlibSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCreateXlibSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface, result); });
}

bool BestPractices::PreCallValidateGetPhysicalDeviceXlibPresentationSupportKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    queueFamilyIndex,
    Display*                                    dpy,
    VisualID                                    visualID) const {
    bool skip = ValidationStateTracker::PreCallValidateGetPhysicalDeviceXlibPresentationSupportKHR(physicalDevice, queueFamilyIndex, dpy, visualID);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetPhysicalDeviceXlibPresentationSupportKHR(physicalDevice, queueFamilyIndex, dpy, visualID); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetPhysicalDeviceXlibPresentationSupportKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    queueFamilyIndex,
    Display*                                    dpy,
    VisualID                                    visualID) {
    ValidationStateTracker::PreCallRecordGetPhysicalDeviceXlibPresentationSupportKHR(physicalDevice, queueFamilyIndex, dpy, visualID);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetPhysicalDeviceXlibPresentationSupportKHR(physicalDevice, queueFamilyIndex, dpy, visualID); });
}

void BestPractices::PostCallRecordGetPhysicalDeviceXlibPresentationSupportKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    queueFamilyIndex,
    Display*                                    dpy,
    VisualID                                    visualID) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceXlibPresentationSupportKHR(physicalDevice, queueFamilyIndex, dpy, visualID);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetPhysicalDeviceXlibPresentationSupportKHR(physicalDevice, queueFamilyIndex, dpy, visualID); });
}

bool BestPractices::PreCallValidateCreateXcbSurfaceKHR(
    VkInstance                                  instance,
    const VkXcbSurfaceCreateInfoKHR*            pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) const {
    bool skip = ValidationStateTracker::PreCallValidateCreateXcbSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCreateXcbSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCreateXcbSurfaceKHR(
    VkInstance                                  instance,
    const VkXcbSurfaceCreateInfoKHR*            pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) {
    ValidationStateTracker::PreCallRecordCreateXcbSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCreateXcbSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface); });
}

void BestPractices::PostCallRecordCreateXcbSurfaceKHR(
    VkInstance                                  instance,
    const VkXcbSurfaceCreateInfoKHR*            pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCreateXcbSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCreateXcbSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface, result); });
}

bool BestPractices::PreCallValidateGetPhysicalDeviceXcbPresentationSupportKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    queueFamilyIndex,
    xcb_connection_t*                           connection,
    xcb_visualid_t                              visual_id) const {
    bool skip = ValidationStateTracker::PreCallValidateGetPhysicalDeviceXcbPresentationSupportKHR(physicalDevice, queueFamilyIndex, connection, visual_id);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetPhysicalDeviceXcbPresentationSupportKHR(physicalDevice, queueFamilyIndex, connection, visual_id); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetPhysicalDeviceXcbPresentationSupportKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    queueFamilyIndex,
    xcb_connection_t*                           connection,
    xcb_visualid_t                              visual_id) {
    ValidationStateTracker::PreCallRecordGetPhysicalDeviceXcbPresentationSupportKHR(physicalDevice, queueFamilyIndex, connection, visual_id);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetPhysicalDeviceXcbPresentationSupportKHR(physicalDevice, queueFamilyIndex, connection, visual_id); });
}

void BestPractices::PostCallRecordGetPhysicalDeviceXcbPresentationSupportKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    queueFamilyIndex,
    xcb_connection_t*                           connection,
    xcb_visualid_t                              visual_id) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceXcbPresentationSupportKHR(physicalDevice, queueFamilyIndex, connection, visual_id);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetPhysicalDeviceXcbPresentationSupportKHR(physicalDevice, queueFamilyIndex, connection, visual_id); });
}

bool BestPractices::PreCallValidateCreateWaylandSurfaceKHR(
    VkInstance                                  instance,
    const VkWaylandSurfaceCreateInfoKHR*        pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) const {
    bool skip = ValidationStateTracker::PreCallValidateCreateWaylandSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCreateWaylandSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCreateWaylandSurfaceKHR(
    VkInstance                                  instance,
    const VkWaylandSurfaceCreateInfoKHR*        pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) {
    ValidationStateTracker::PreCallRecordCreateWaylandSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCreateWaylandSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface); });
}

void BestPractices::PostCallRecordCreateWaylandSurfaceKHR(
    VkInstance                                  instance,
    const VkWaylandSurfaceCreateInfoKHR*        pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCreateWaylandSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCreateWaylandSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface, result); });
}

bool BestPractices::PreCallValidateGetPhysicalDeviceWaylandPresentationSupportKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    queueFamilyIndex,
    struct wl_display*                          display) const {
    bool skip = ValidationStateTracker::PreCallValidateGetPhysicalDeviceWaylandPresentationSupportKHR(physicalDevice, queueFamilyIndex, display);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetPhysicalDeviceWaylandPresentationSupportKHR(physicalDevice, queueFamilyIndex, display); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetPhysicalDeviceWaylandPresentationSupportKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    queueFamilyIndex,
    struct wl_display*                          display) {
    ValidationStateTracker::PreCallRecordGetPhysicalDeviceWaylandPresentationSupportKHR(physicalDevice, queueFamilyIndex, display);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetPhysicalDeviceWaylandPresentationSupportKHR(physicalDevice, queueFamilyIndex, display); });
}

void BestPractices::PostCallRecordGetPhysicalDeviceWaylandPresentationSupportKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    queueFamilyIndex,
    struct wl_display*                          display) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceWaylandPresentationSupportKHR(physicalDevice, queueFamilyIndex, display);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetPhysicalDeviceWaylandPresentationSupportKHR(physicalDevice, queueFamilyIndex, display); });
}

bool BestPractices::PreCallValidateCreateAndroidSurfaceKHR(
    VkInstance                                  instance,
    const VkAndroidSurfaceCreateInfoKHR*        pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) const {
    bool skip = ValidationStateTracker::PreCallValidateCreateAndroidSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCreateAndroidSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCreateAndroidSurfaceKHR(
    VkInstance                                  instance,
    const VkAndroidSurfaceCreateInfoKHR*        pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) {
    ValidationStateTracker::PreCallRecordCreateAndroidSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCreateAndroidSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface); });
}

void BestPractices::PostCallRecordCreateAndroidSurfaceKHR(
    VkInstance                                  instance,
    const VkAndroidSurfaceCreateInfoKHR*        pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCreateAndroidSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCreateAndroidSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface, result); });
}

bool BestPractices::PreCallValidateCreateWin32SurfaceKHR(
    VkInstance                                  instance,
    const VkWin32SurfaceCreateInfoKHR*          pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) const {
    bool skip = ValidationStateTracker::PreCallValidateCreateWin32SurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCreateWin32SurfaceKHR(instance, pCreateInfo, pAllocator, pSurface); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCreateWin32SurfaceKHR(
    VkInstance                                  instance,
    const VkWin32SurfaceCreateInfoKHR*          pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) {
    ValidationStateTracker::PreCallRecordCreateWin32SurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCreateWin32SurfaceKHR(instance, pCreateInfo, pAllocator, pSurface); });
}

void BestPractices::PostCallRecordCreateWin32SurfaceKHR(
    VkInstance                                  instance,
    const VkWin32SurfaceCreateInfoKHR*          pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCreateWin32SurfaceKHR(instance, pCreateInfo, pAllocator, pSurface, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCreateWin32SurfaceKHR(instance, pCreateInfo, pAllocator, pSurface, result); });
}

bool BestPractices::PreCallValidateGetPhysicalDeviceWin32PresentationSupportKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    queueFamilyIndex) const {
    bool skip = ValidationStateTracker::PreCallValidateGetPhysicalDeviceWin32PresentationSupportKHR(physicalDevice, queueFamilyIndex);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetPhysicalDeviceWin32PresentationSupportKHR(physicalDevice, queueFamilyIndex); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetPhysicalDeviceWin32PresentationSupportKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    queueFamilyIndex) {
    ValidationStateTracker::PreCallRecordGetPhysicalDeviceWin32PresentationSupportKHR(physicalDevice, queueFamilyIndex);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetPhysicalDeviceWin32PresentationSupportKHR(physicalDevice, queueFamilyIndex); });
}

void BestPractices::PostCallRecordGetPhysicalDeviceWin32PresentationSupportKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    queueFamilyIndex) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceWin32PresentationSupportKHR(physicalDevice, queueFamilyIndex);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetPhysicalDeviceWin32PresentationSupportKHR(physicalDevice, queueFamilyIndex); });
}

bool BestPractices::PreCallValidateGetPhysicalDeviceFeatures2KHR(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceFeatures2*                  pFeatures) const {
    bool skip = ValidationStateTracker::PreCallValidateGetPhysicalDeviceFeatures2KHR(physicalDevice, pFeatures);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetPhysicalDeviceFeatures2KHR(physicalDevice, pFeatures); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetPhysicalDeviceFeatures2KHR(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceFeatures2*                  pFeatures) {
    ValidationStateTracker::PreCallRecordGetPhysicalDeviceFeatures2KHR(physicalDevice, pFeatures);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetPhysicalDeviceFeatures2KHR(physicalDevice, pFeatures); });
}

void BestPractices::PostCallRecordGetPhysicalDeviceFeatures2KHR(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceFeatures2*                  pFeatures) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceFeatures2KHR(physicalDevice, pFeatures);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetPhysicalDeviceFeatures2KHR(physicalDevice, pFeatures); });
}

bool BestPractices::PreCallValidateGetPhysicalDeviceProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceProperties2*                pProperties) const {
    bool skip = ValidationStateTracker::PreCallValidateGetPhysicalDeviceProperties2KHR(physicalDevice, pProperties);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetPhysicalDeviceProperties2KHR(physicalDevice, pProperties); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetPhysicalDeviceProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceProperties2*                pProperties) {
    ValidationStateTracker::PreCallRecordGetPhysicalDeviceProperties2KHR(physicalDevice, pProperties);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetPhysicalDeviceProperties2KHR(physicalDevice, pProperties); });
}

void BestPractices::PostCallRecordGetPhysicalDeviceProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceProperties2*                pProperties) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceProperties2KHR(physicalDevice, pProperties);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetPhysicalDeviceProperties2KHR(physicalDevice, pProperties); });
}

bool BestPractices::PreCallValidateGetPhysicalDeviceFormatProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    VkFormat                                    format,
    VkFormatProperties2*                        pFormatProperties) const {
    bool skip = ValidationStateTracker::PreCallValidateGetPhysicalDeviceFormatProperties2KHR(physicalDevice, format, pFormatProperties);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetPhysicalDeviceFormatProperties2KHR(physicalDevice, format, pFormatProperties); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetPhysicalDeviceFormatProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    VkFormat                                    format,
    VkFormatProperties2*                        pFormatProperties) {
    ValidationStateTracker::PreCallRecordGetPhysicalDeviceFormatProperties2KHR(physicalDevice, format, pFormatProperties);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetPhysicalDeviceFormatProperties2KHR(physicalDevice, format, pFormatProperties); });
}

void BestPractices::PostCallRecordGetPhysicalDeviceFormatProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    VkFormat                                    format,
    VkFormatProperties2*                        pFormatProperties) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceFormatProperties2KHR(physicalDevice, format, pFormatProperties);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetPhysicalDeviceFormatProperties2KHR(physicalDevice, format, pFormatProperties); });
}

bool BestPractices::PreCallValidateGetPhysicalDeviceImageFormatProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceImageFormatInfo2*     pImageFormatInfo,
    VkImageFormatProperties2*                   pImageFormatProperties) const {
    bool skip = ValidationStateTracker::PreCallValidateGetPhysicalDeviceImageFormatProperties2KHR(physicalDevice, pImageFormatInfo, pImageFormatProperties);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetPhysicalDeviceImageFormatProperties2KHR(physicalDevice, pImageFormatInfo, pImageFormatProperties); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetPhysicalDeviceImageFormatProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceImageFormatInfo2*     pImageFormatInfo,
    VkImageFormatProperties2*                   pImageFormatProperties) {
    ValidationStateTracker::PreCallRecordGetPhysicalDeviceImageFormatProperties2KHR(physicalDevice, pImageFormatInfo, pImageFormatProperties);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetPhysicalDeviceImageFormatProperties2KHR(physicalDevice, pImageFormatInfo, pImageFormatProperties); });
}

void BestPractices::PostCallRecordGetPhysicalDeviceImageFormatProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceImageFormatInfo2*     pImageFormatInfo,
    VkImageFormatProperties2*                   pImageFormatProperties,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceImageFormatProperties2KHR(physicalDevice, pImageFormatInfo, pImageFormatProperties, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetPhysicalDeviceImageFormatProperties2KHR(physicalDevice, pImageFormatInfo, pImageFormatProperties, result); });
}

bool BestPractices::PreCallValidateGetPhysicalDeviceQueueFamilyProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pQueueFamilyPropertyCount,
    VkQueueFamilyProperties2*                   pQueueFamilyProperties) const {
    bool skip = ValidationStateTracker::PreCallValidateGetPhysicalDeviceQueueFamilyProperties2KHR(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetPhysicalDeviceQueueFamilyProperties2KHR(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetPhysicalDeviceQueueFamilyProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pQueueFamilyPropertyCount,
    VkQueueFamilyProperties2*                   pQueueFamilyProperties) {
    ValidationStateTracker::PreCallRecordGetPhysicalDeviceQueueFamilyProperties2KHR(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetPhysicalDeviceQueueFamilyProperties2KHR(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties); });
}

void BestPractices::PostCallRecordGetPhysicalDeviceQueueFamilyProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pQueueFamilyPropertyCount,
    VkQueueFamilyProperties2*                   pQueueFamilyProperties) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceQueueFamilyProperties2KHR(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetPhysicalDeviceQueueFamilyProperties2KHR(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties); });
}

bool BestPractices::PreCallValidateGetPhysicalDeviceMemoryProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceMemoryProperties2*          pMemoryProperties) const {
    bool skip = ValidationStateTracker::PreCallValidateGetPhysicalDeviceMemoryProperties2KHR(physicalDevice, pMemoryProperties);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetPhysicalDeviceMemoryProperties2KHR(physicalDevice, pMemoryProperties); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetPhysicalDeviceMemoryProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceMemoryProperties2*          pMemoryProperties) {
    ValidationStateTracker::PreCallRecordGetPhysicalDeviceMemoryProperties2KHR(physicalDevice, pMemoryProperties);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetPhysicalDeviceMemoryProperties2KHR(physicalDevice, pMemoryProperties); });
}

void BestPractices::PostCallRecordGetPhysicalDeviceMemoryProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceMemoryProperties2*          pMemoryProperties) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceMemoryProperties2KHR(physicalDevice, pMemoryProperties);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetPhysicalDeviceMemoryProperties2KHR(physicalDevice, pMemoryProperties); });
}

bool BestPractices::PreCallValidateGetPhysicalDeviceSparseImageFormatProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceSparseImageFormatInfo2* pFormatInfo,
    uint32_t*                                   pPropertyCount,
    VkSparseImageFormatProperties2*             pProperties) const {
    bool skip = ValidationStateTracker::PreCallValidateGetPhysicalDeviceSparseImageFormatProperties2KHR(physicalDevice, pFormatInfo, pPropertyCount, pProperties);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetPhysicalDeviceSparseImageFormatProperties2KHR(physicalDevice, pFormatInfo, pPropertyCount, pProperties); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetPhysicalDeviceSparseImageFormatProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceSparseImageFormatInfo2* pFormatInfo,
    uint32_t*                                   pPropertyCount,
    VkSparseImageFormatProperties2*             pProperties) {
    ValidationStateTracker::PreCallRecordGetPhysicalDeviceSparseImageFormatProperties2KHR(physicalDevice, pFormatInfo, pPropertyCount, pProperties);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetPhysicalDeviceSparseImageFormatProperties2KHR(physicalDevice, pFormatInfo, pPropertyCount, pProperties); });
}

void BestPractices::PostCallRecordGetPhysicalDeviceSparseImageFormatProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceSparseImageFormatInfo2* pFormatInfo,
    uint32_t*                                   pPropertyCount,
    VkSparseImageFormatProperties2*             pProperties) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceSparseImageFormatProperties2KHR(physicalDevice, pFormatInfo, pPropertyCount, pProperties);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetPhysicalDeviceSparseImageFormatProperties2KHR(physicalDevice, pFormatInfo, pPropertyCount, pProperties); });
}

bool BestPractices::PreCallValidateGetDeviceGroupPeerMemoryFeaturesKHR(
    VkDevice                                    device,
    uint32_t                                    heapIndex,
    uint32_t                                    localDeviceIndex,
    uint32_t                                    remoteDeviceIndex,
    VkPeerMemoryFeatureFlags*                   pPeerMemoryFeatures) const {
    bool skip = ValidationStateTracker::PreCallValidateGetDeviceGroupPeerMemoryFeaturesKHR(device, heapIndex, localDeviceIndex, remoteDeviceIndex, pPeerMemoryFeatures);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetDeviceGroupPeerMemoryFeaturesKHR(device, heapIndex, localDeviceIndex, remoteDeviceIndex, pPeerMemoryFeatures); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetDeviceGroupPeerMemoryFeaturesKHR(
    VkDevice                                    device,
    uint32_t                                    heapIndex,
    uint32_t                                    localDeviceIndex,
    uint32_t                                    remoteDeviceIndex,
    VkPeerMemoryFeatureFlags*                   pPeerMemoryFeatures) {
    ValidationStateTracker::PreCallRecordGetDeviceGroupPeerMemoryFeaturesKHR(device, heapIndex, localDeviceIndex, remoteDeviceIndex, pPeerMemoryFeatures);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetDeviceGroupPeerMemoryFeaturesKHR(device, heapIndex, localDeviceIndex, remoteDeviceIndex, pPeerMemoryFeatures); });
}

void BestPractices::PostCallRecordGetDeviceGroupPeerMemoryFeaturesKHR(
    VkDevice                                    device,
    uint32_t                                    heapIndex,
    uint32_t                                    localDeviceIndex,
    uint32_t                                    remoteDeviceIndex,
    VkPeerMemoryFeatureFlags*                   pPeerMemoryFeatures) {
    ValidationStateTracker::PostCallRecordGetDeviceGroupPeerMemoryFeaturesKHR(device, heapIndex, localDeviceIndex, remoteDeviceIndex, pPeerMemoryFeatures);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetDeviceGroupPeerMemoryFeaturesKHR(device, heapIndex, localDeviceIndex, remoteDeviceIndex, pPeerMemoryFeatures); });
}

bool BestPractices::PreCallValidateCmdSetDeviceMaskKHR(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    deviceMask) const {
    bool skip = ValidationStateTracker::PreCallValidateCmdSetDeviceMaskKHR(commandBuffer, deviceMask);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCmdSetDeviceMaskKHR(commandBuffer, deviceMask); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCmdSetDeviceMaskKHR(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    deviceMask) {
    ValidationStateTracker::PreCallRecordCmdSetDeviceMaskKHR(commandBuffer, deviceMask);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCmdSetDeviceMaskKHR(commandBuffer, deviceMask); });
}

void BestPractices::PostCallRecordCmdSetDeviceMaskKHR(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    deviceMask) {
    ValidationStateTracker::PostCallRecordCmdSetDeviceMaskKHR(commandBuffer, deviceMask);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCmdSetDeviceMaskKHR(commandBuffer, deviceMask); });
}

bool BestPractices::PreCallValidateCmdDispatchBaseKHR(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    baseGroupX,
    uint32_t                                    baseGroupY,
    uint32_t                                    baseGroupZ,
    uint32_t                                    groupCountX,
    uint32_t                                    groupCountY,
    uint32_t                                    groupCountZ) const {
    bool skip = ValidationStateTracker::PreCallValidateCmdDispatchBaseKHR(commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCmdDispatchBaseKHR(commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCmdDispatchBaseKHR(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    baseGroupX,
    uint32_t                                    baseGroupY,
    uint32_t                                    baseGroupZ,
    uint32_t                                    groupCountX,
    uint32_t                                    groupCountY,
    uint32_t                                    groupCountZ) {
    ValidationStateTracker::PreCallRecordCmdDispatchBaseKHR(commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCmdDispatchBaseKHR(commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ); });
}

void BestPractices::PostCallRecordCmdDispatchBaseKHR(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    baseGroupX,
    uint32_t                                    baseGroupY,
    uint32_t                                    baseGroupZ,
    uint32_t                                    groupCountX,
    uint32_t                                    groupCountY,
    uint32_t                                    groupCountZ) {
    ValidationStateTracker::PostCallRecordCmdDispatchBaseKHR(commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCmdDispatchBaseKHR(commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ); });
}

bool BestPractices::PreCallValidateTrimCommandPoolKHR(
    VkDevice                                    device,
    VkCommandPool                               commandPool,
    VkCommandPoolTrimFlags                      flags) const {
    bool skip = ValidationStateTracker::PreCallValidateTrimCommandPoolKHR(device, commandPool, flags);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateTrimCommandPoolKHR(device, commandPool, flags); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordTrimCommandPoolKHR(
    VkDevice                                    device,
    VkCommandPool                               commandPool,
    VkCommandPoolTrimFlags                      flags) {
    ValidationStateTracker::PreCallRecordTrimCommandPoolKHR(device, commandPool, flags);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordTrimCommandPoolKHR(device, commandPool, flags); });
}

void BestPractices::PostCallRecordTrimCommandPoolKHR(
    VkDevice                                    device,
    VkCommandPool                               commandPool,
    VkCommandPoolTrimFlags                      flags) {
    ValidationStateTracker::PostCallRecordTrimCommandPoolKHR(device, commandPool, flags);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordTrimCommandPoolKHR(device, commandPool, flags); });
}

bool BestPractices::PreCallValidateEnumeratePhysicalDeviceGroupsKHR(
    VkInstance                                  instance,
    uint32_t*                                   pPhysicalDeviceGroupCount,
    VkPhysicalDeviceGroupProperties*            pPhysicalDeviceGroupProperties) const {
    bool skip = ValidationStateTracker::PreCallValidateEnumeratePhysicalDeviceGroupsKHR(instance, pPhysicalDeviceGroupCount, pPhysicalDeviceGroupProperties);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateEnumeratePhysicalDeviceGroupsKHR(instance, pPhysicalDeviceGroupCount, pPhysicalDeviceGroupProperties); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordEnumeratePhysicalDeviceGroupsKHR(
    VkInstance                                  instance,
    uint32_t*                                   pPhysicalDeviceGroupCount,
    VkPhysicalDeviceGroupProperties*            pPhysicalDeviceGroupProperties) {
    ValidationStateTracker::PreCallRecordEnumeratePhysicalDeviceGroupsKHR(instance, pPhysicalDeviceGroupCount, pPhysicalDeviceGroupProperties);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordEnumeratePhysicalDeviceGroupsKHR(instance, pPhysicalDeviceGroupCount, pPhysicalDeviceGroupProperties); });
}

void BestPractices::PostCallRecordEnumeratePhysicalDeviceGroupsKHR(
    VkInstance                                  instance,
    uint32_t*                                   pPhysicalDeviceGroupCount,
    VkPhysicalDeviceGroupProperties*            pPhysicalDeviceGroupProperties,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordEnumeratePhysicalDeviceGroupsKHR(instance, pPhysicalDeviceGroupCount, pPhysicalDeviceGroupProperties, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordEnumeratePhysicalDeviceGroupsKHR(instance, pPhysicalDeviceGroupCount, pPhysicalDeviceGroupProperties, result); });
}

bool BestPractices::PreCallValidateGetPhysicalDeviceExternalBufferPropertiesKHR(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceExternalBufferInfo*   pExternalBufferInfo,
    VkExternalBufferProperties*                 pExternalBufferProperties) const {
    bool skip = ValidationStateTracker::PreCallValidateGetPhysicalDeviceExternalBufferPropertiesKHR(physicalDevice, pExternalBufferInfo, pExternalBufferProperties);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetPhysicalDeviceExternalBufferPropertiesKHR(physicalDevice, pExternalBufferInfo, pExternalBufferProperties); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetPhysicalDeviceExternalBufferPropertiesKHR(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceExternalBufferInfo*   pExternalBufferInfo,
    VkExternalBufferProperties*                 pExternalBufferProperties) {
    ValidationStateTracker::PreCallRecordGetPhysicalDeviceExternalBufferPropertiesKHR(physicalDevice, pExternalBufferInfo, pExternalBufferProperties);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetPhysicalDeviceExternalBufferPropertiesKHR(physicalDevice, pExternalBufferInfo, pExternalBufferProperties); });
}

void BestPractices::PostCallRecordGetPhysicalDeviceExternalBufferPropertiesKHR(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceExternalBufferInfo*   pExternalBufferInfo,
    VkExternalBufferProperties*                 pExternalBufferProperties) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceExternalBufferPropertiesKHR(physicalDevice, pExternalBufferInfo, pExternalBufferProperties);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetPhysicalDeviceExternalBufferPropertiesKHR(physicalDevice, pExternalBufferInfo, pExternalBufferProperties); });
}

bool BestPractices::PreCallValidateGetMemoryWin32HandleKHR(
    VkDevice                                    device,
    const VkMemoryGetWin32HandleInfoKHR*        pGetWin32HandleInfo,
    HANDLE*                                     pHandle) const {
    bool skip = ValidationStateTracker::PreCallValidateGetMemoryWin32HandleKHR(device, pGetWin32HandleInfo, pHandle);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetMemoryWin32HandleKHR(device, pGetWin32HandleInfo, pHandle); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetMemoryWin32HandleKHR(
    VkDevice                                    device,
    const VkMemoryGetWin32HandleInfoKHR*        pGetWin32HandleInfo,
    HANDLE*                                     pHandle) {
    ValidationStateTracker::PreCallRecordGetMemoryWin32HandleKHR(device, pGetWin32HandleInfo, pHandle);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetMemoryWin32HandleKHR(device, pGetWin32HandleInfo, pHandle); });
}

void BestPractices::PostCallRecordGetMemoryWin32HandleKHR(
    VkDevice                                    device,
    const VkMemoryGetWin32HandleInfoKHR*        pGetWin32HandleInfo,
    HANDLE*                                     pHandle,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetMemoryWin32HandleKHR(device, pGetWin32HandleInfo, pHandle, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetMemoryWin32HandleKHR(device, pGetWin32HandleInfo, pHandle, result); });
}

bool BestPractices::PreCallValidateGetMemoryWin32HandlePropertiesKHR(
    VkDevice                                    device,
    VkExternalMemoryHandleTypeFlagBits          handleType,
    HANDLE                                      handle,
    VkMemoryWin32HandlePropertiesKHR*           pMemoryWin32HandleProperties) const {
    bool skip = ValidationStateTracker::PreCallValidateGetMemoryWin32HandlePropertiesKHR(device, handleType, handle, pMemoryWin32HandleProperties);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetMemoryWin32HandlePropertiesKHR(device, handleType, handle, pMemoryWin32HandleProperties); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetMemoryWin32HandlePropertiesKHR(
    VkDevice                                    device,
    VkExternalMemoryHandleTypeFlagBits          handleType,
    HANDLE                                      handle,
    VkMemoryWin32HandlePropertiesKHR*           pMemoryWin32HandleProperties) {
    ValidationStateTracker::PreCallRecordGetMemoryWin32HandlePropertiesKHR(device, handleType, handle, pMemoryWin32HandleProperties);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetMemoryWin32HandlePropertiesKHR(device, handleType, handle, pMemoryWin32HandleProperties); });
}

void BestPractices::PostCallRecordGetMemoryWin32HandlePropertiesKHR(
    VkDevice                                    device,
    VkExternalMemoryHandleTypeFlagBits          handleType,
    HANDLE                                      handle,
    VkMemoryWin32HandlePropertiesKHR*           pMemoryWin32HandleProperties,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetMemoryWin32HandlePropertiesKHR(device, handleType, handle, pMemoryWin32HandleProperties, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetMemoryWin32HandlePropertiesKHR(device, handleType, handle, pMemoryWin32HandleProperties, result); });
}

bool BestPractices::PreCallValidateGetMemoryFdKHR(
    VkDevice                                    device,
    const VkMemoryGetFdInfoKHR*                 pGetFdInfo,
    int*                                        pFd) const {
    bool skip = ValidationStateTracker::PreCallValidateGetMemoryFdKHR(device, pGetFdInfo, pFd);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetMemoryFdKHR(device, pGetFdInfo, pFd); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetMemoryFdKHR(
    VkDevice                                    device,
    const VkMemoryGetFdInfoKHR*                 pGetFdInfo,
    int*                                        pFd) {
    ValidationStateTracker::PreCallRecordGetMemoryFdKHR(device, pGetFdInfo, pFd);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetMemoryFdKHR(device, pGetFdInfo, pFd); });
}

void BestPractices::PostCallRecordGetMemoryFdKHR(
    VkDevice                                    device,
    const VkMemoryGetFdInfoKHR*                 pGetFdInfo,
    int*                                        pFd,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetMemoryFdKHR(device, pGetFdInfo, pFd, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetMemoryFdKHR(device, pGetFdInfo, pFd, result); });
}

bool BestPractices::PreCallValidateGetMemoryFdPropertiesKHR(
    VkDevice                                    device,
    VkExternalMemoryHandleTypeFlagBits          handleType,
    int                                         fd,
    VkMemoryFdPropertiesKHR*                    pMemoryFdProperties) const {
    bool skip = ValidationStateTracker::PreCallValidateGetMemoryFdPropertiesKHR(device, handleType, fd, pMemoryFdProperties);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetMemoryFdPropertiesKHR(device, handleType, fd, pMemoryFdProperties); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetMemoryFdPropertiesKHR(
    VkDevice                                    device,
    VkExternalMemoryHandleTypeFlagBits          handleType,
    int                                         fd,
    VkMemoryFdPropertiesKHR*                    pMemoryFdProperties) {
    ValidationStateTracker::PreCallRecordGetMemoryFdPropertiesKHR(device, handleType, fd, pMemoryFdProperties);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetMemoryFdPropertiesKHR(device, handleType, fd, pMemoryFdProperties); });
}

void BestPractices::PostCallRecordGetMemoryFdPropertiesKHR(
    VkDevice                                    device,
    VkExternalMemoryHandleTypeFlagBits          handleType,
    int                                         fd,
    VkMemoryFdPropertiesKHR*                    pMemoryFdProperties,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetMemoryFdPropertiesKHR(device, handleType, fd, pMemoryFdProperties, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetMemoryFdPropertiesKHR(device, handleType, fd, pMemoryFdProperties, result); });
}

bool BestPractices::PreCallValidateGetPhysicalDeviceExternalSemaphorePropertiesKHR(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceExternalSemaphoreInfo* pExternalSemaphoreInfo,
    VkExternalSemaphoreProperties*              pExternalSemaphoreProperties) const {
    bool skip = ValidationStateTracker::PreCallValidateGetPhysicalDeviceExternalSemaphorePropertiesKHR(physicalDevice, pExternalSemaphoreInfo, pExternalSemaphoreProperties);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetPhysicalDeviceExternalSemaphorePropertiesKHR(physicalDevice, pExternalSemaphoreInfo, pExternalSemaphoreProperties); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetPhysicalDeviceExternalSemaphorePropertiesKHR(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceExternalSemaphoreInfo* pExternalSemaphoreInfo,
    VkExternalSemaphoreProperties*              pExternalSemaphoreProperties) {
    ValidationStateTracker::PreCallRecordGetPhysicalDeviceExternalSemaphorePropertiesKHR(physicalDevice, pExternalSemaphoreInfo, pExternalSemaphoreProperties);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetPhysicalDeviceExternalSemaphorePropertiesKHR(physicalDevice, pExternalSemaphoreInfo, pExternalSemaphoreProperties); });
}

void BestPractices::PostCallRecordGetPhysicalDeviceExternalSemaphorePropertiesKHR(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceExternalSemaphoreInfo* pExternalSemaphoreInfo,
    VkExternalSemaphoreProperties*              pExternalSemaphoreProperties) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceExternalSemaphorePropertiesKHR(physicalDevice, pExternalSemaphoreInfo, pExternalSemaphoreProperties);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetPhysicalDeviceExternalSemaphorePropertiesKHR(physicalDevice, pExternalSemaphoreInfo, pExternalSemaphoreProperties); });
}

bool BestPractices::PreCallValidateImportSemaphoreWin32HandleKHR(
    VkDevice                                    device,
    const VkImportSemaphoreWin32HandleInfoKHR*  pImportSemaphoreWin32HandleInfo) const {
    bool skip = ValidationStateTracker::PreCallValidateImportSemaphoreWin32HandleKHR(device, pImportSemaphoreWin32HandleInfo);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateImportSemaphoreWin32HandleKHR(device, pImportSemaphoreWin32HandleInfo); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordImportSemaphoreWin32HandleKHR(
    VkDevice                                    device,
    const VkImportSemaphoreWin32HandleInfoKHR*  pImportSemaphoreWin32HandleInfo) {
    ValidationStateTracker::PreCallRecordImportSemaphoreWin32HandleKHR(device, pImportSemaphoreWin32HandleInfo);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordImportSemaphoreWin32HandleKHR(device, pImportSemaphoreWin32HandleInfo); });
}

void BestPractices::PostCallRecordImportSemaphoreWin32HandleKHR(
    VkDevice                                    device,
    const VkImportSemaphoreWin32HandleInfoKHR*  pImportSemaphoreWin32HandleInfo,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordImportSemaphoreWin32HandleKHR(device, pImportSemaphoreWin32HandleInfo, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordImportSemaphoreWin32HandleKHR(device, pImportSemaphoreWin32HandleInfo, result); });
}

bool BestPractices::PreCallValidateGetSemaphoreWin32HandleKHR(
    VkDevice                                    device,
    const VkSemaphoreGetWin32HandleInfoKHR*     pGetWin32HandleInfo,
    HANDLE*                                     pHandle) const {
    bool skip = ValidationStateTracker::PreCallValidateGetSemaphoreWin32HandleKHR(device, pGetWin32HandleInfo, pHandle);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetSemaphoreWin32HandleKHR(device, pGetWin32HandleInfo, pHandle); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetSemaphoreWin32HandleKHR(
    VkDevice                                    device,
    const VkSemaphoreGetWin32HandleInfoKHR*     pGetWin32HandleInfo,
    HANDLE*                                     pHandle) {
    ValidationStateTracker::PreCallRecordGetSemaphoreWin32HandleKHR(device, pGetWin32HandleInfo, pHandle);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetSemaphoreWin32HandleKHR(device, pGetWin32HandleInfo, pHandle); });
}

void BestPractices::PostCallRecordGetSemaphoreWin32HandleKHR(
    VkDevice                                    device,
    const VkSemaphoreGetWin32HandleInfoKHR*     pGetWin32HandleInfo,
    HANDLE*                                     pHandle,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetSemaphoreWin32HandleKHR(device, pGetWin32HandleInfo, pHandle, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetSemaphoreWin32HandleKHR(device, pGetWin32HandleInfo, pHandle, result); });
}

bool BestPractices::PreCallValidateImportSemaphoreFdKHR(
    VkDevice                                    device,
    const VkImportSemaphoreFdInfoKHR*           pImportSemaphoreFdInfo) const {
    bool skip = ValidationStateTracker::PreCallValidateImportSemaphoreFdKHR(device, pImportSemaphoreFdInfo);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateImportSemaphoreFdKHR(device, pImportSemaphoreFdInfo); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordImportSemaphoreFdKHR(
    VkDevice                                    device,
    const VkImportSemaphoreFdInfoKHR*           pImportSemaphoreFdInfo) {
    ValidationStateTracker::PreCallRecordImportSemaphoreFdKHR(device, pImportSemaphoreFdInfo);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordImportSemaphoreFdKHR(device, pImportSemaphoreFdInfo); });
}

void BestPractices::PostCallRecordImportSemaphoreFdKHR(
    VkDevice                                    device,
    const VkImportSemaphoreFdInfoKHR*           pImportSemaphoreFdInfo,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordImportSemaphoreFdKHR(device, pImportSemaphoreFdInfo, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordImportSemaphoreFdKHR(device, pImportSemaphoreFdInfo, result); });
}

bool BestPractices::PreCallValidateGetSemaphoreFdKHR(
    VkDevice                                    device,
    const VkSemaphoreGetFdInfoKHR*              pGetFdInfo,
    int*                                        pFd) const {
    bool skip = ValidationStateTracker::PreCallValidateGetSemaphoreFdKHR(device, pGetFdInfo, pFd);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetSemaphoreFdKHR(device, pGetFdInfo, pFd); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetSemaphoreFdKHR(
    VkDevice                                    device,
    const VkSemaphoreGetFdInfoKHR*              pGetFdInfo,
    int*                                        pFd) {
    ValidationStateTracker::PreCallRecordGetSemaphoreFdKHR(device, pGetFdInfo, pFd);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetSemaphoreFdKHR(device, pGetFdInfo, pFd); });
}

void BestPractices::PostCallRecordGetSemaphoreFdKHR(
    VkDevice                                    device,
    const VkSemaphoreGetFdInfoKHR*              pGetFdInfo,
    int*                                        pFd,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetSemaphoreFdKHR(device, pGetFdInfo, pFd, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetSemaphoreFdKHR(device, pGetFdInfo, pFd, result); });
}

bool BestPractices::PreCallValidateCmdPushDescriptorSetKHR(
    VkCommandBuffer                             commandBuffer,
    VkPipelineBindPoint                         pipelineBindPoint,
    VkPipelineLayout                            layout,
    uint32_t                                    set,
    uint32_t                                    descriptorWriteCount,
    const VkWriteDescriptorSet*                 pDescriptorWrites) const {
    bool skip = ValidationStateTracker::PreCallValidateCmdPushDescriptorSetKHR(commandBuffer, pipelineBindPoint, layout, set, descriptorWriteCount, pDescriptorWrites);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCmdPushDescriptorSetKHR(commandBuffer, pipelineBindPoint, layout, set, descriptorWriteCount, pDescriptorWrites); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCmdPushDescriptorSetKHR(
    VkCommandBuffer                             commandBuffer,
    VkPipelineBindPoint                         pipelineBindPoint,
    VkPipelineLayout                            layout,
    uint32_t                                    set,
    uint32_t                                    descriptorWriteCount,
    const VkWriteDescriptorSet*                 pDescriptorWrites) {
    ValidationStateTracker::PreCallRecordCmdPushDescriptorSetKHR(commandBuffer, pipelineBindPoint, layout, set, descriptorWriteCount, pDescriptorWrites);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCmdPushDescriptorSetKHR(commandBuffer, pipelineBindPoint, layout, set, descriptorWriteCount, pDescriptorWrites); });
}

void BestPractices::PostCallRecordCmdPushDescriptorSetKHR(
    VkCommandBuffer                             commandBuffer,
    VkPipelineBindPoint                         pipelineBindPoint,
    VkPipelineLayout                            layout,
    uint32_t                                    set,
    uint32_t                                    descriptorWriteCount,
    const VkWriteDescriptorSet*                 pDescriptorWrites) {
    ValidationStateTracker::PostCallRecordCmdPushDescriptorSetKHR(commandBuffer, pipelineBindPoint, layout, set, descriptorWriteCount, pDescriptorWrites);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCmdPushDescriptorSetKHR(commandBuffer, pipelineBindPoint, layout, set, descriptorWriteCount, pDescriptorWrites); });
}

bool BestPractices::PreCallValidateCmdPushDescriptorSetWithTemplateKHR(
    VkCommandBuffer                             commandBuffer,
    VkDescriptorUpdateTemplate                  descriptorUpdateTemplate,
    VkPipelineLayout                            layout,
    uint32_t                                    set,
    const void*                                 pData) const {
    bool skip = ValidationStateTracker::PreCallValidateCmdPushDescriptorSetWithTemplateKHR(commandBuffer, descriptorUpdateTemplate, layout, set, pData);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCmdPushDescriptorSetWithTemplateKHR(commandBuffer, descriptorUpdateTemplate, layout, set, pData); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCmdPushDescriptorSetWithTemplateKHR(
    VkCommandBuffer                             commandBuffer,
    VkDescriptorUpdateTemplate                  descriptorUpdateTemplate,
    VkPipelineLayout                            layout,
    uint32_t                                    set,
    const void*                                 pData) {
    ValidationStateTracker::PreCallRecordCmdPushDescriptorSetWithTemplateKHR(commandBuffer, descriptorUpdateTemplate, layout, set, pData);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCmdPushDescriptorSetWithTemplateKHR(commandBuffer, descriptorUpdateTemplate, layout, set, pData); });
}

void BestPractices::PostCallRecordCmdPushDescriptorSetWithTemplateKHR(
    VkCommandBuffer                             commandBuffer,
    VkDescriptorUpdateTemplate                  descriptorUpdateTemplate,
    VkPipelineLayout                            layout,
    uint32_t                                    set,
    const void*                                 pData) {
    ValidationStateTracker::PostCallRecordCmdPushDescriptorSetWithTemplateKHR(commandBuffer, descriptorUpdateTemplate, layout, set, pData);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCmdPushDescriptorSetWithTemplateKHR(commandBuffer, descriptorUpdateTemplate, layout, set, pData); });
}

bool BestPractices::PreCallValidateCreateDescriptorUpdateTemplateKHR(
    VkDevice                                    device,
    const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDescriptorUpdateTemplate*                 pDescriptorUpdateTemplate) const {
    bool skip = ValidationStateTracker::PreCallValidateCreateDescriptorUpdateTemplateKHR(device, pCreateInfo, pAllocator, pDescriptorUpdateTemplate);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCreateDescriptorUpdateTemplateKHR(device, pCreateInfo, pAllocator, pDescriptorUpdateTemplate); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCreateDescriptorUpdateTemplateKHR(
    VkDevice                                    device,
    const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDescriptorUpdateTemplate*                 pDescriptorUpdateTemplate) {
    ValidationStateTracker::PreCallRecordCreateDescriptorUpdateTemplateKHR(device, pCreateInfo, pAllocator, pDescriptorUpdateTemplate);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCreateDescriptorUpdateTemplateKHR(device, pCreateInfo, pAllocator, pDescriptorUpdateTemplate); });
}

void BestPractices::PostCallRecordCreateDescriptorUpdateTemplateKHR(
    VkDevice                                    device,
    const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDescriptorUpdateTemplate*                 pDescriptorUpdateTemplate,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCreateDescriptorUpdateTemplateKHR(device, pCreateInfo, pAllocator, pDescriptorUpdateTemplate, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCreateDescriptorUpdateTemplateKHR(device, pCreateInfo, pAllocator, pDescriptorUpdateTemplate, result); });
}

bool BestPractices::PreCallValidateDestroyDescriptorUpdateTemplateKHR(
    VkDevice                                    device,
    VkDescriptorUpdateTemplate                  descriptorUpdateTemplate,
    const VkAllocationCallbacks*                pAllocator) const {
    bool skip = ValidationStateTracker::PreCallValidateDestroyDescriptorUpdateTemplateKHR(device, descriptorUpdateTemplate, pAllocator);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateDestroyDescriptorUpdateTemplateKHR(device, descriptorUpdateTemplate, pAllocator); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordDestroyDescriptorUpdateTemplateKHR(
    VkDevice                                    device,
    VkDescriptorUpdateTemplate                  descriptorUpdateTemplate,
    const VkAllocationCallbacks*                pAllocator) {
    ValidationStateTracker::PreCallRecordDestroyDescriptorUpdateTemplateKHR(device, descriptorUpdateTemplate, pAllocator);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordDestroyDescriptorUpdateTemplateKHR(device, descriptorUpdateTemplate, pAllocator); });
}

void BestPractices::PostCallRecordDestroyDescriptorUpdateTemplateKHR(
    VkDevice                                    device,
    VkDescriptorUpdateTemplate                  descriptorUpdateTemplate,
    const VkAllocationCallbacks*                pAllocator) {
    ValidationStateTracker::PostCallRecordDestroyDescriptorUpdateTemplateKHR(device, descriptorUpdateTemplate, pAllocator);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordDestroyDescriptorUpdateTemplateKHR(device, descriptorUpdateTemplate, pAllocator); });
}

bool BestPractices::PreCallValidateUpdateDescriptorSetWithTemplateKHR(
    VkDevice                                    device,
    VkDescriptorSet                             descriptorSet,
    VkDescriptorUpdateTemplate                  descriptorUpdateTemplate,
    const void*                                 pData) const {
    bool skip = ValidationStateTracker::PreCallValidateUpdateDescriptorSetWithTemplateKHR(device, descriptorSet, descriptorUpdateTemplate, pData);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateUpdateDescriptorSetWithTemplateKHR(device, descriptorSet, descriptorUpdateTemplate, pData); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordUpdateDescriptorSetWithTemplateKHR(
    VkDevice                                    device,
    VkDescriptorSet                             descriptorSet,
    VkDescriptorUpdateTemplate                  descriptorUpdateTemplate,
    const void*                                 pData) {
    ValidationStateTracker::PreCallRecordUpdateDescriptorSetWithTemplateKHR(device, descriptorSet, descriptorUpdateTemplate, pData);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordUpdateDescriptorSetWithTemplateKHR(device, descriptorSet, descriptorUpdateTemplate, pData); });
}

void BestPractices::PostCallRecordUpdateDescriptorSetWithTemplateKHR(
    VkDevice                                    device,
    VkDescriptorSet                             descriptorSet,
    VkDescriptorUpdateTemplate                  descriptorUpdateTemplate,
    const void*                                 pData) {
    ValidationStateTracker::PostCallRecordUpdateDescriptorSetWithTemplateKHR(device, descriptorSet, descriptorUpdateTemplate, pData);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordUpdateDescriptorSetWithTemplateKHR(device, descriptorSet, descriptorUpdateTemplate, pData); });
}

bool BestPractices::PreCallValidateCreateRenderPass2KHR(
    VkDevice                                    device,
    const VkRenderPassCreateInfo2*              pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkRenderPass*                               pRenderPass) const {
    bool skip = ValidationStateTracker::PreCallValidateCreateRenderPass2KHR(device, pCreateInfo, pAllocator, pRenderPass);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCreateRenderPass2KHR(device, pCreateInfo, pAllocator, pRenderPass); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCreateRenderPass2KHR(
    VkDevice                                    device,
    const VkRenderPassCreateInfo2*              pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkRenderPass*                               pRenderPass) {
    ValidationStateTracker::PreCallRecordCreateRenderPass2KHR(device, pCreateInfo, pAllocator, pRenderPass);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCreateRenderPass2KHR(device, pCreateInfo, pAllocator, pRenderPass); });
}

void BestPractices::PostCallRecordCreateRenderPass2KHR(
    VkDevice                                    device,
    const VkRenderPassCreateInfo2*              pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkRenderPass*                               pRenderPass,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCreateRenderPass2KHR(device, pCreateInfo, pAllocator, pRenderPass, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCreateRenderPass2KHR(device, pCreateInfo, pAllocator, pRenderPass, result); });
}

bool BestPractices::PreCallValidateCmdBeginRenderPass2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkRenderPassBeginInfo*                pRenderPassBegin,
    const VkSubpassBeginInfo*                   pSubpassBeginInfo) const {
    bool skip = ValidationStateTracker::PreCallValidateCmdBeginRenderPass2KHR(commandBuffer, pRenderPassBegin, pSubpassBeginInfo);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCmdBeginRenderPass2KHR(commandBuffer, pRenderPassBegin, pSubpassBeginInfo); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCmdBeginRenderPass2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkRenderPassBeginInfo*                pRenderPassBegin,
    const VkSubpassBeginInfo*                   pSubpassBeginInfo) {
    ValidationStateTracker::PreCallRecordCmdBeginRenderPass2KHR(commandBuffer, pRenderPassBegin, pSubpassBeginInfo);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCmdBeginRenderPass2KHR(commandBuffer, pRenderPassBegin, pSubpassBeginInfo); });
}

void BestPractices::PostCallRecordCmdBeginRenderPass2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkRenderPassBeginInfo*                pRenderPassBegin,
    const VkSubpassBeginInfo*                   pSubpassBeginInfo) {
    ValidationStateTracker::PostCallRecordCmdBeginRenderPass2KHR(commandBuffer, pRenderPassBegin, pSubpassBeginInfo);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCmdBeginRenderPass2KHR(commandBuffer, pRenderPassBegin, pSubpassBeginInfo); });
}

bool BestPractices::PreCallValidateCmdNextSubpass2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkSubpassBeginInfo*                   pSubpassBeginInfo,
    const VkSubpassEndInfo*                     pSubpassEndInfo) const {
    bool skip = ValidationStateTracker::PreCallValidateCmdNextSubpass2KHR(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCmdNextSubpass2KHR(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCmdNextSubpass2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkSubpassBeginInfo*                   pSubpassBeginInfo,
    const VkSubpassEndInfo*                     pSubpassEndInfo) {
    ValidationStateTracker::PreCallRecordCmdNextSubpass2KHR(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCmdNextSubpass2KHR(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo); });
}

void BestPractices::PostCallRecordCmdNextSubpass2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkSubpassBeginInfo*                   pSubpassBeginInfo,
    const VkSubpassEndInfo*                     pSubpassEndInfo) {
    ValidationStateTracker::PostCallRecordCmdNextSubpass2KHR(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCmdNextSubpass2KHR(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo); });
}

bool BestPractices::PreCallValidateCmdEndRenderPass2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkSubpassEndInfo*                     pSubpassEndInfo) const {
    bool skip = ValidationStateTracker::PreCallValidateCmdEndRenderPass2KHR(commandBuffer, pSubpassEndInfo);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCmdEndRenderPass2KHR(commandBuffer, pSubpassEndInfo); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCmdEndRenderPass2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkSubpassEndInfo*                     pSubpassEndInfo) {
    ValidationStateTracker::PreCallRecordCmdEndRenderPass2KHR(commandBuffer, pSubpassEndInfo);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCmdEndRenderPass2KHR(commandBuffer, pSubpassEndInfo); });
}

void BestPractices::PostCallRecordCmdEndRenderPass2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkSubpassEndInfo*                     pSubpassEndInfo) {
    ValidationStateTracker::PostCallRecordCmdEndRenderPass2KHR(commandBuffer, pSubpassEndInfo);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCmdEndRenderPass2KHR(commandBuffer, pSubpassEndInfo); });
}

bool BestPractices::PreCallValidateGetSwapchainStatusKHR(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain) const {
    bool skip = ValidationStateTracker::PreCallValidateGetSwapchainStatusKHR(device, swapchain);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetSwapchainStatusKHR(device, swapchain); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetSwapchainStatusKHR(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain) {
    ValidationStateTracker::PreCallRecordGetSwapchainStatusKHR(device, swapchain);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetSwapchainStatusKHR(device, swapchain); });
}

void BestPractices::PostCallRecordGetSwapchainStatusKHR(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetSwapchainStatusKHR(device, swapchain, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetSwapchainStatusKHR(device, swapchain, result); });
}

bool BestPractices::PreCallValidateGetPhysicalDeviceExternalFencePropertiesKHR(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceExternalFenceInfo*    pExternalFenceInfo,
    VkExternalFenceProperties*                  pExternalFenceProperties) const {
    bool skip = ValidationStateTracker::PreCallValidateGetPhysicalDeviceExternalFencePropertiesKHR(physicalDevice, pExternalFenceInfo, pExternalFenceProperties);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetPhysicalDeviceExternalFencePropertiesKHR(physicalDevice, pExternalFenceInfo, pExternalFenceProperties); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetPhysicalDeviceExternalFencePropertiesKHR(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceExternalFenceInfo*    pExternalFenceInfo,
    VkExternalFenceProperties*                  pExternalFenceProperties) {
    ValidationStateTracker::PreCallRecordGetPhysicalDeviceExternalFencePropertiesKHR(physicalDevice, pExternalFenceInfo, pExternalFenceProperties);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetPhysicalDeviceExternalFencePropertiesKHR(physicalDevice, pExternalFenceInfo, pExternalFenceProperties); });
}

void BestPractices::PostCallRecordGetPhysicalDeviceExternalFencePropertiesKHR(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceExternalFenceInfo*    pExternalFenceInfo,
    VkExternalFenceProperties*                  pExternalFenceProperties) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceExternalFencePropertiesKHR(physicalDevice, pExternalFenceInfo, pExternalFenceProperties);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetPhysicalDeviceExternalFencePropertiesKHR(physicalDevice, pExternalFenceInfo, pExternalFenceProperties); });
}

bool BestPractices::PreCallValidateImportFenceWin32HandleKHR(
    VkDevice                                    device,
    const VkImportFenceWin32HandleInfoKHR*      pImportFenceWin32HandleInfo) const {
    bool skip = ValidationStateTracker::PreCallValidateImportFenceWin32HandleKHR(device, pImportFenceWin32HandleInfo);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateImportFenceWin32HandleKHR(device, pImportFenceWin32HandleInfo); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordImportFenceWin32HandleKHR(
    VkDevice                                    device,
    const VkImportFenceWin32HandleInfoKHR*      pImportFenceWin32HandleInfo) {
    ValidationStateTracker::PreCallRecordImportFenceWin32HandleKHR(device, pImportFenceWin32HandleInfo);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordImportFenceWin32HandleKHR(device, pImportFenceWin32HandleInfo); });
}

void BestPractices::PostCallRecordImportFenceWin32HandleKHR(
    VkDevice                                    device,
    const VkImportFenceWin32HandleInfoKHR*      pImportFenceWin32HandleInfo,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordImportFenceWin32HandleKHR(device, pImportFenceWin32HandleInfo, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordImportFenceWin32HandleKHR(device, pImportFenceWin32HandleInfo, result); });
}

bool BestPractices::PreCallValidateGetFenceWin32HandleKHR(
    VkDevice                                    device,
    const VkFenceGetWin32HandleInfoKHR*         pGetWin32HandleInfo,
    HANDLE*                                     pHandle) const {
    bool skip = ValidationStateTracker::PreCallValidateGetFenceWin32HandleKHR(device, pGetWin32HandleInfo, pHandle);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetFenceWin32HandleKHR(device, pGetWin32HandleInfo, pHandle); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetFenceWin32HandleKHR(
    VkDevice                                    device,
    const VkFenceGetWin32HandleInfoKHR*         pGetWin32HandleInfo,
    HANDLE*                                     pHandle) {
    ValidationStateTracker::PreCallRecordGetFenceWin32HandleKHR(device, pGetWin32HandleInfo, pHandle);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetFenceWin32HandleKHR(device, pGetWin32HandleInfo, pHandle); });
}

void BestPractices::PostCallRecordGetFenceWin32HandleKHR(
    VkDevice                                    device,
    const VkFenceGetWin32HandleInfoKHR*         pGetWin32HandleInfo,
    HANDLE*                                     pHandle,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetFenceWin32HandleKHR(device, pGetWin32HandleInfo, pHandle, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetFenceWin32HandleKHR(device, pGetWin32HandleInfo, pHandle, result); });
}

bool BestPractices::PreCallValidateImportFenceFdKHR(
    VkDevice                                    device,
    const VkImportFenceFdInfoKHR*               pImportFenceFdInfo) const {
    bool skip = ValidationStateTracker::PreCallValidateImportFenceFdKHR(device, pImportFenceFdInfo);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateImportFenceFdKHR(device, pImportFenceFdInfo); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordImportFenceFdKHR(
    VkDevice                                    device,
    const VkImportFenceFdInfoKHR*               pImportFenceFdInfo) {
    ValidationStateTracker::PreCallRecordImportFenceFdKHR(device, pImportFenceFdInfo);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordImportFenceFdKHR(device, pImportFenceFdInfo); });
}

void BestPractices::PostCallRecordImportFenceFdKHR(
    VkDevice                                    device,
    const VkImportFenceFdInfoKHR*               pImportFenceFdInfo,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordImportFenceFdKHR(device, pImportFenceFdInfo, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordImportFenceFdKHR(device, pImportFenceFdInfo, result); });
}

bool BestPractices::PreCallValidateGetFenceFdKHR(
    VkDevice                                    device,
    const VkFenceGetFdInfoKHR*                  pGetFdInfo,
    int*                                        pFd) const {
    bool skip = ValidationStateTracker::PreCallValidateGetFenceFdKHR(device, pGetFdInfo, pFd);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetFenceFdKHR(device, pGetFdInfo, pFd); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetFenceFdKHR(
    VkDevice                                    device,
    const VkFenceGetFdInfoKHR*                  pGetFdInfo,
    int*                                        pFd) {
    ValidationStateTracker::PreCallRecordGetFenceFdKHR(device, pGetFdInfo, pFd);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetFenceFdKHR(device, pGetFdInfo, pFd); });
}

void BestPractices::PostCallRecordGetFenceFdKHR(
    VkDevice                                    device,
    const VkFenceGetFdInfoKHR*                  pGetFdInfo,
    int*                                        pFd,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetFenceFdKHR(device, pGetFdInfo, pFd, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetFenceFdKHR(device, pGetFdInfo, pFd, result); });
}

bool BestPractices::PreCallValidateEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    queueFamilyIndex,
    uint32_t*                                   pCounterCount,
    VkPerformanceCounterKHR*                    pCounters,
    VkPerformanceCounterDescriptionKHR*         pCounterDescriptions) const {
    bool skip = ValidationStateTracker::PreCallValidateEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(physicalDevice, queueFamilyIndex, pCounterCount, pCounters, pCounterDescriptions);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(physicalDevice, queueFamilyIndex, pCounterCount, pCounters, pCounterDescriptions); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    queueFamilyIndex,
    uint32_t*                                   pCounterCount,
    VkPerformanceCounterKHR*                    pCounters,
    VkPerformanceCounterDescriptionKHR*         pCounterDescriptions) {
    ValidationStateTracker::PreCallRecordEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(physicalDevice, queueFamilyIndex, pCounterCount, pCounters, pCounterDescriptions);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(physicalDevice, queueFamilyIndex, pCounterCount, pCounters, pCounterDescriptions); });
}

void BestPractices::PostCallRecordEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    queueFamilyIndex,
    uint32_t*                                   pCounterCount,
    VkPerformanceCounterKHR*                    pCounters,
    VkPerformanceCounterDescriptionKHR*         pCounterDescriptions,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(physicalDevice, queueFamilyIndex, pCounterCount, pCounters, pCounterDescriptions, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(physicalDevice, queueFamilyIndex, pCounterCount, pCounters, pCounterDescriptions, result); });
}

bool BestPractices::PreCallValidateGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR(
    VkPhysicalDevice                            physicalDevice,
    const VkQueryPoolPerformanceCreateInfoKHR*  pPerformanceQueryCreateInfo,
    uint32_t*                                   pNumPasses) const {
    bool skip = ValidationStateTracker::PreCallValidateGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR(physicalDevice, pPerformanceQueryCreateInfo, pNumPasses);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR(physicalDevice, pPerformanceQueryCreateInfo, pNumPasses); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR(
    VkPhysicalDevice                            physicalDevice,
    const VkQueryPoolPerformanceCreateInfoKHR*  pPerformanceQueryCreateInfo,
    uint32_t*                                   pNumPasses) {
    ValidationStateTracker::PreCallRecordGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR(physicalDevice, pPerformanceQueryCreateInfo, pNumPasses);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR(physicalDevice, pPerformanceQueryCreateInfo, pNumPasses); });
}

void BestPractices::PostCallRecordGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR(
    VkPhysicalDevice                            physicalDevice,
    const VkQueryPoolPerformanceCreateInfoKHR*  pPerformanceQueryCreateInfo,
    uint32_t*                                   pNumPasses) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR(physicalDevice, pPerformanceQueryCreateInfo, pNumPasses);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR(physicalDevice, pPerformanceQueryCreateInfo, pNumPasses); });
}

bool BestPractices::PreCallValidateAcquireProfilingLockKHR(
    VkDevice                                    device,
    const VkAcquireProfilingLockInfoKHR*        pInfo) const {
    bool skip = ValidationStateTracker::PreCallValidateAcquireProfilingLockKHR(device, pInfo);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateAcquireProfilingLockKHR(device, pInfo); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordAcquireProfilingLockKHR(
    VkDevice                                    device,
    const VkAcquireProfilingLockInfoKHR*        pInfo) {
    ValidationStateTracker::PreCallRecordAcquireProfilingLockKHR(device, pInfo);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordAcquireProfilingLockKHR(device, pInfo); });
}

void BestPractices::PostCallRecordAcquireProfilingLockKHR(
    VkDevice                                    device,
    const VkAcquireProfilingLockInfoKHR*        pInfo,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordAcquireProfilingLockKHR(device, pInfo, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordAcquireProfilingLockKHR(device, pInfo, result); });
}

bool BestPractices::PreCallValidateReleaseProfilingLockKHR(
    VkDevice                                    device) const {
    bool skip = ValidationStateTracker::PreCallValidateReleaseProfilingLockKHR(device);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateReleaseProfilingLockKHR(device); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordReleaseProfilingLockKHR(
    VkDevice                                    device) {
    ValidationStateTracker::PreCallRecordReleaseProfilingLockKHR(device);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordReleaseProfilingLockKHR(device); });
}

void BestPractices::PostCallRecordReleaseProfilingLockKHR(
    VkDevice                                    device) {
    ValidationStateTracker::PostCallRecordReleaseProfilingLockKHR(device);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordReleaseProfilingLockKHR(device); });
}

bool BestPractices::PreCallValidateGetPhysicalDeviceSurfaceCapabilities2KHR(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceSurfaceInfo2KHR*      pSurfaceInfo,
    VkSurfaceCapabilities2KHR*                  pSurfaceCapabilities) const {
    bool skip = ValidationStateTracker::PreCallValidateGetPhysicalDeviceSurfaceCapabilities2KHR(physicalDevice, pSurfaceInfo, pSurfaceCapabilities);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetPhysicalDeviceSurfaceCapabilities2KHR(physicalDevice, pSurfaceInfo, pSurfaceCapabilities); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetPhysicalDeviceSurfaceCapabilities2KHR(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceSurfaceInfo2KHR*      pSurfaceInfo,
    VkSurfaceCapabilities2KHR*                  pSurfaceCapabilities) {
    ValidationStateTracker::PreCallRecordGetPhysicalDeviceSurfaceCapabilities2KHR(physicalDevice, pSurfaceInfo, pSurfaceCapabilities);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetPhysicalDeviceSurfaceCapabilities2KHR(physicalDevice, pSurfaceInfo, pSurfaceCapabilities); });
}

void BestPractices::PostCallRecordGetPhysicalDeviceSurfaceCapabilities2KHR(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceSurfaceInfo2KHR*      pSurfaceInfo,
    VkSurfaceCapabilities2KHR*                  pSurfaceCapabilities,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceSurfaceCapabilities2KHR(physicalDevice, pSurfaceInfo, pSurfaceCapabilities, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetPhysicalDeviceSurfaceCapabilities2KHR(physicalDevice, pSurfaceInfo, pSurfaceCapabilities, result); });
}

bool BestPractices::PreCallValidateGetPhysicalDeviceSurfaceFormats2KHR(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceSurfaceInfo2KHR*      pSurfaceInfo,
    uint32_t*                                   pSurfaceFormatCount,
    VkSurfaceFormat2KHR*                        pSurfaceFormats) const {
    bool skip = ValidationStateTracker::PreCallValidateGetPhysicalDeviceSurfaceFormats2KHR(physicalDevice, pSurfaceInfo, pSurfaceFormatCount, pSurfaceFormats);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetPhysicalDeviceSurfaceFormats2KHR(physicalDevice, pSurfaceInfo, pSurfaceFormatCount, pSurfaceFormats); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetPhysicalDeviceSurfaceFormats2KHR(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceSurfaceInfo2KHR*      pSurfaceInfo,
    uint32_t*                                   pSurfaceFormatCount,
    VkSurfaceFormat2KHR*                        pSurfaceFormats) {
    ValidationStateTracker::PreCallRecordGetPhysicalDeviceSurfaceFormats2KHR(physicalDevice, pSurfaceInfo, pSurfaceFormatCount, pSurfaceFormats);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetPhysicalDeviceSurfaceFormats2KHR(physicalDevice, pSurfaceInfo, pSurfaceFormatCount, pSurfaceFormats); });
}

void BestPractices::PostCallRecordGetPhysicalDeviceSurfaceFormats2KHR(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceSurfaceInfo2KHR*      pSurfaceInfo,
    uint32_t*                                   pSurfaceFormatCount,
    VkSurfaceFormat2KHR*                        pSurfaceFormats,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceSurfaceFormats2KHR(physicalDevice, pSurfaceInfo, pSurfaceFormatCount, pSurfaceFormats, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetPhysicalDeviceSurfaceFormats2KHR(physicalDevice, pSurfaceInfo, pSurfaceFormatCount, pSurfaceFormats, result); });
}

bool BestPractices::PreCallValidateGetPhysicalDeviceDisplayProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pPropertyCount,
    VkDisplayProperties2KHR*                    pProperties) const {
    bool skip = ValidationStateTracker::PreCallValidateGetPhysicalDeviceDisplayProperties2KHR(physicalDevice, pPropertyCount, pProperties);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetPhysicalDeviceDisplayProperties2KHR(physicalDevice, pPropertyCount, pProperties); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetPhysicalDeviceDisplayProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pPropertyCount,
    VkDisplayProperties2KHR*                    pProperties) {
    ValidationStateTracker::PreCallRecordGetPhysicalDeviceDisplayProperties2KHR(physicalDevice, pPropertyCount, pProperties);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetPhysicalDeviceDisplayProperties2KHR(physicalDevice, pPropertyCount, pProperties); });
}

void BestPractices::PostCallRecordGetPhysicalDeviceDisplayProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pPropertyCount,
    VkDisplayProperties2KHR*                    pProperties,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceDisplayProperties2KHR(physicalDevice, pPropertyCount, pProperties, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetPhysicalDeviceDisplayProperties2KHR(physicalDevice, pPropertyCount, pProperties, result); });
}

bool BestPractices::PreCallValidateGetPhysicalDeviceDisplayPlaneProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pPropertyCount,
    VkDisplayPlaneProperties2KHR*               pProperties) const {
    bool skip = ValidationStateTracker::PreCallValidateGetPhysicalDeviceDisplayPlaneProperties2KHR(physicalDevice, pPropertyCount, pProperties);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetPhysicalDeviceDisplayPlaneProperties2KHR(physicalDevice, pPropertyCount, pProperties); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetPhysicalDeviceDisplayPlaneProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pPropertyCount,
    VkDisplayPlaneProperties2KHR*               pProperties) {
    ValidationStateTracker::PreCallRecordGetPhysicalDeviceDisplayPlaneProperties2KHR(physicalDevice, pPropertyCount, pProperties);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetPhysicalDeviceDisplayPlaneProperties2KHR(physicalDevice, pPropertyCount, pProperties); });
}

void BestPractices::PostCallRecordGetPhysicalDeviceDisplayPlaneProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pPropertyCount,
    VkDisplayPlaneProperties2KHR*               pProperties,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceDisplayPlaneProperties2KHR(physicalDevice, pPropertyCount, pProperties, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetPhysicalDeviceDisplayPlaneProperties2KHR(physicalDevice, pPropertyCount, pProperties, result); });
}

bool BestPractices::PreCallValidateGetDisplayModeProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    VkDisplayKHR                                display,
    uint32_t*                                   pPropertyCount,
    VkDisplayModeProperties2KHR*                pProperties) const {
    bool skip = ValidationStateTracker::PreCallValidateGetDisplayModeProperties2KHR(physicalDevice, display, pPropertyCount, pProperties);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetDisplayModeProperties2KHR(physicalDevice, display, pPropertyCount, pProperties); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetDisplayModeProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    VkDisplayKHR                                display,
    uint32_t*                                   pPropertyCount,
    VkDisplayModeProperties2KHR*                pProperties) {
    ValidationStateTracker::PreCallRecordGetDisplayModeProperties2KHR(physicalDevice, display, pPropertyCount, pProperties);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetDisplayModeProperties2KHR(physicalDevice, display, pPropertyCount, pProperties); });
}

void BestPractices::PostCallRecordGetDisplayModeProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    VkDisplayKHR                                display,
    uint32_t*                                   pPropertyCount,
    VkDisplayModeProperties2KHR*                pProperties,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetDisplayModeProperties2KHR(physicalDevice, display, pPropertyCount, pProperties, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetDisplayModeProperties2KHR(physicalDevice, display, pPropertyCount, pProperties, result); });
}

bool BestPractices::PreCallValidateGetDisplayPlaneCapabilities2KHR(
    VkPhysicalDevice                            physicalDevice,
    const VkDisplayPlaneInfo2KHR*               pDisplayPlaneInfo,
    VkDisplayPlaneCapabilities2KHR*             pCapabilities) const {
    bool skip = ValidationStateTracker::PreCallValidateGetDisplayPlaneCapabilities2KHR(physicalDevice, pDisplayPlaneInfo, pCapabilities);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetDisplayPlaneCapabilities2KHR(physicalDevice, pDisplayPlaneInfo, pCapabilities); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetDisplayPlaneCapabilities2KHR(
    VkPhysicalDevice                            physicalDevice,
    const VkDisplayPlaneInfo2KHR*               pDisplayPlaneInfo,
    VkDisplayPlaneCapabilities2KHR*             pCapabilities) {
    ValidationStateTracker::PreCallRecordGetDisplayPlaneCapabilities2KHR(physicalDevice, pDisplayPlaneInfo, pCapabilities);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetDisplayPlaneCapabilities2KHR(physicalDevice, pDisplayPlaneInfo, pCapabilities); });
}

void BestPractices::PostCallRecordGetDisplayPlaneCapabilities2KHR(
    VkPhysicalDevice                            physicalDevice,
    const VkDisplayPlaneInfo2KHR*               pDisplayPlaneInfo,
    VkDisplayPlaneCapabilities2KHR*             pCapabilities,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetDisplayPlaneCapabilities2KHR(physicalDevice, pDisplayPlaneInfo, pCapabilities, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetDisplayPlaneCapabilities2KHR(physicalDevice, pDisplayPlaneInfo, pCapabilities, result); });
}

bool BestPractices::PreCallValidateGetImageMemoryRequirements2KHR(
    VkDevice                                    device,
    const VkImageMemoryRequirementsInfo2*       pInfo,
    VkMemoryRequirements2*                      pMemoryRequirements) const {
    bool skip = ValidationStateTracker::PreCallValidateGetImageMemoryRequirements2KHR(device, pInfo, pMemoryRequirements);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetImageMemoryRequirements2KHR(device, pInfo, pMemoryRequirements); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetImageMemoryRequirements2KHR(
    VkDevice                                    device,
    const VkImageMemoryRequirementsInfo2*       pInfo,
    VkMemoryRequirements2*                      pMemoryRequirements) {
    ValidationStateTracker::PreCallRecordGetImageMemoryRequirements2KHR(device, pInfo, pMemoryRequirements);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetImageMemoryRequirements2KHR(device, pInfo, pMemoryRequirements); });
}

void BestPractices::PostCallRecordGetImageMemoryRequirements2KHR(
    VkDevice                                    device,
    const VkImageMemoryRequirementsInfo2*       pInfo,
    VkMemoryRequirements2*                      pMemoryRequirements) {
    ValidationStateTracker::PostCallRecordGetImageMemoryRequirements2KHR(device, pInfo, pMemoryRequirements);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetImageMemoryRequirements2KHR(device, pInfo, pMemoryRequirements); });
}

bool BestPractices::PreCallValidateGetBufferMemoryRequirements2KHR(
    VkDevice                                    device,
    const VkBufferMemoryRequirementsInfo2*      pInfo,
    VkMemoryRequirements2*                      pMemoryRequirements) const {
    bool skip = ValidationStateTracker::PreCallValidateGetBufferMemoryRequirements2KHR(device, pInfo, pMemoryRequirements);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetBufferMemoryRequirements2KHR(device, pInfo, pMemoryRequirements); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetBufferMemoryRequirements2KHR(
    VkDevice                                    device,
    const VkBufferMemoryRequirementsInfo2*      pInfo,
    VkMemoryRequirements2*                      pMemoryRequirements) {
    ValidationStateTracker::PreCallRecordGetBufferMemoryRequirements2KHR(device, pInfo, pMemoryRequirements);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetBufferMemoryRequirements2KHR(device, pInfo, pMemoryRequirements); });
}

void BestPractices::PostCallRecordGetBufferMemoryRequirements2KHR(
    VkDevice                                    device,
    const VkBufferMemoryRequirementsInfo2*      pInfo,
    VkMemoryRequirements2*                      pMemoryRequirements) {
    ValidationStateTracker::PostCallRecordGetBufferMemoryRequirements2KHR(device, pInfo, pMemoryRequirements);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetBufferMemoryRequirements2KHR(device, pInfo, pMemoryRequirements); });
}

bool BestPractices::PreCallValidateGetImageSparseMemoryRequirements2KHR(
    VkDevice                                    device,
    const VkImageSparseMemoryRequirementsInfo2* pInfo,
    uint32_t*                                   pSparseMemoryRequirementCount,
    VkSparseImageMemoryRequirements2*           pSparseMemoryRequirements) const {
    bool skip = ValidationStateTracker::PreCallValidateGetImageSparseMemoryRequirements2KHR(device, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetImageSparseMemoryRequirements2KHR(device, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetImageSparseMemoryRequirements2KHR(
    VkDevice                                    device,
    const VkImageSparseMemoryRequirementsInfo2* pInfo,
    uint32_t*                                   pSparseMemoryRequirementCount,
    VkSparseImageMemoryRequirements2*           pSparseMemoryRequirements) {
    ValidationStateTracker::PreCallRecordGetImageSparseMemoryRequirements2KHR(device, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetImageSparseMemoryRequirements2KHR(device, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements); });
}

void BestPractices::PostCallRecordGetImageSparseMemoryRequirements2KHR(
    VkDevice                                    device,
    const VkImageSparseMemoryRequirementsInfo2* pInfo,
    uint32_t*                                   pSparseMemoryRequirementCount,
    VkSparseImageMemoryRequirements2*           pSparseMemoryRequirements) {
    ValidationStateTracker::PostCallRecordGetImageSparseMemoryRequirements2KHR(device, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetImageSparseMemoryRequirements2KHR(device, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements); });
}

bool BestPractices::PreCallValidateCreateSamplerYcbcrConversionKHR(
    VkDevice                                    device,
    const VkSamplerYcbcrConversionCreateInfo*   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSamplerYcbcrConversion*                   pYcbcrConversion) const {
    bool skip = ValidationStateTracker::PreCallValidateCreateSamplerYcbcrConversionKHR(device, pCreateInfo, pAllocator, pYcbcrConversion);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCreateSamplerYcbcrConversionKHR(device, pCreateInfo, pAllocator, pYcbcrConversion); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCreateSamplerYcbcrConversionKHR(
    VkDevice                                    device,
    const VkSamplerYcbcrConversionCreateInfo*   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSamplerYcbcrConversion*                   pYcbcrConversion) {
    ValidationStateTracker::PreCallRecordCreateSamplerYcbcrConversionKHR(device, pCreateInfo, pAllocator, pYcbcrConversion);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCreateSamplerYcbcrConversionKHR(device, pCreateInfo, pAllocator, pYcbcrConversion); });
}

void BestPractices::PostCallRecordCreateSamplerYcbcrConversionKHR(
    VkDevice                                    device,
    const VkSamplerYcbcrConversionCreateInfo*   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSamplerYcbcrConversion*                   pYcbcrConversion,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCreateSamplerYcbcrConversionKHR(device, pCreateInfo, pAllocator, pYcbcrConversion, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCreateSamplerYcbcrConversionKHR(device, pCreateInfo, pAllocator, pYcbcrConversion, result); });
}

bool BestPractices::PreCallValidateDestroySamplerYcbcrConversionKHR(
    VkDevice                                    device,
    VkSamplerYcbcrConversion                    ycbcrConversion,
    const VkAllocationCallbacks*                pAllocator) const {
    bool skip = ValidationStateTracker::PreCallValidateDestroySamplerYcbcrConversionKHR(device, ycbcrConversion, pAllocator);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateDestroySamplerYcbcrConversionKHR(device, ycbcrConversion, pAllocator); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordDestroySamplerYcbcrConversionKHR(
    VkDevice                                    device,
    VkSamplerYcbcrConversion                    ycbcrConversion,
    const VkAllocationCallbacks*                pAllocator) {
    ValidationStateTracker::PreCallRecordDestroySamplerYcbcrConversionKHR(device, ycbcrConversion, pAllocator);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordDestroySamplerYcbcrConversionKHR(device, ycbcrConversion, pAllocator); });
}

void BestPractices::PostCallRecordDestroySamplerYcbcrConversionKHR(
    VkDevice                                    device,
    VkSamplerYcbcrConversion                    ycbcrConversion,
    const VkAllocationCallbacks*                pAllocator) {
    ValidationStateTracker::PostCallRecordDestroySamplerYcbcrConversionKHR(device, ycbcrConversion, pAllocator);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordDestroySamplerYcbcrConversionKHR(device, ycbcrConversion, pAllocator); });
}

bool BestPractices::PreCallValidateBindBufferMemory2KHR(
    VkDevice                                    device,
    uint32_t                                    bindInfoCount,
    const VkBindBufferMemoryInfo*               pBindInfos) const {
    bool skip = ValidationStateTracker::PreCallValidateBindBufferMemory2KHR(device, bindInfoCount, pBindInfos);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateBindBufferMemory2KHR(device, bindInfoCount, pBindInfos); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordBindBufferMemory2KHR(
    VkDevice                                    device,
    uint32_t                                    bindInfoCount,
    const VkBindBufferMemoryInfo*               pBindInfos) {
    ValidationStateTracker::PreCallRecordBindBufferMemory2KHR(device, bindInfoCount, pBindInfos);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordBindBufferMemory2KHR(device, bindInfoCount, pBindInfos); });
}

void BestPractices::PostCallRecordBindBufferMemory2KHR(
    VkDevice                                    device,
    uint32_t                                    bindInfoCount,
    const VkBindBufferMemoryInfo*               pBindInfos,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordBindBufferMemory2KHR(device, bindInfoCount, pBindInfos, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordBindBufferMemory2KHR(device, bindInfoCount, pBindInfos, result); });
}

bool BestPractices::PreCallValidateBindImageMemory2KHR(
    VkDevice                                    device,
    uint32_t                                    bindInfoCount,
    const VkBindImageMemoryInfo*                pBindInfos) const {
    bool skip = ValidationStateTracker::PreCallValidateBindImageMemory2KHR(device, bindInfoCount, pBindInfos);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateBindImageMemory2KHR(device, bindInfoCount, pBindInfos); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordBindImageMemory2KHR(
    VkDevice                                    device,
    uint32_t                                    bindInfoCount,
    const VkBindImageMemoryInfo*                pBindInfos) {
    ValidationStateTracker::PreCallRecordBindImageMemory2KHR(device, bindInfoCount, pBindInfos);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordBindImageMemory2KHR(device, bindInfoCount, pBindInfos); });
}

void BestPractices::PostCallRecordBindImageMemory2KHR(
    VkDevice                                    device,
    uint32_t                                    bindInfoCount,
    const VkBindImageMemoryInfo*                pBindInfos,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordBindImageMemory2KHR(device, bindInfoCount, pBindInfos, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordBindImageMemory2KHR(device, bindInfoCount, pBindInfos, result); });
}

bool BestPractices::PreCallValidateGetDescriptorSetLayoutSupportKHR(
    VkDevice                                    device,
    const VkDescriptorSetLayoutCreateInfo*      pCreateInfo,
    VkDescriptorSetLayoutSupport*               pSupport) const {
    bool skip = ValidationStateTracker::PreCallValidateGetDescriptorSetLayoutSupportKHR(device, pCreateInfo, pSupport);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetDescriptorSetLayoutSupportKHR(device, pCreateInfo, pSupport); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetDescriptorSetLayoutSupportKHR(
    VkDevice                                    device,
    const VkDescriptorSetLayoutCreateInfo*      pCreateInfo,
    VkDescriptorSetLayoutSupport*               pSupport) {
    ValidationStateTracker::PreCallRecordGetDescriptorSetLayoutSupportKHR(device, pCreateInfo, pSupport);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetDescriptorSetLayoutSupportKHR(device, pCreateInfo, pSupport); });
}

void BestPractices::PostCallRecordGetDescriptorSetLayoutSupportKHR(
    VkDevice                                    device,
    const VkDescriptorSetLayoutCreateInfo*      pCreateInfo,
    VkDescriptorSetLayoutSupport*               pSupport) {
    ValidationStateTracker::PostCallRecordGetDescriptorSetLayoutSupportKHR(device, pCreateInfo, pSupport);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetDescriptorSetLayoutSupportKHR(device, pCreateInfo, pSupport); });
}

bool BestPractices::PreCallValidateCmdDrawIndirectCountKHR(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkBuffer                                    countBuffer,
    VkDeviceSize                                countBufferOffset,
    uint32_t                                    maxDrawCount,
    uint32_t                                    stride) const {
    bool skip = ValidationStateTracker::PreCallValidateCmdDrawIndirectCountKHR(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCmdDrawIndirectCountKHR(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCmdDrawIndirectCountKHR(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkBuffer                                    countBuffer,
    VkDeviceSize                                countBufferOffset,
    uint32_t                                    maxDrawCount,
    uint32_t                                    stride) {
    ValidationStateTracker::PreCallRecordCmdDrawIndirectCountKHR(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCmdDrawIndirectCountKHR(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride); });
}

void BestPractices::PostCallRecordCmdDrawIndirectCountKHR(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkBuffer                                    countBuffer,
    VkDeviceSize                                countBufferOffset,
    uint32_t                                    maxDrawCount,
    uint32_t                                    stride) {
    ValidationStateTracker::PostCallRecordCmdDrawIndirectCountKHR(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCmdDrawIndirectCountKHR(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride); });
}

bool BestPractices::PreCallValidateCmdDrawIndexedIndirectCountKHR(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkBuffer                                    countBuffer,
    VkDeviceSize                                countBufferOffset,
    uint32_t                                    maxDrawCount,
    uint32_t                                    stride) const {
    bool skip = ValidationStateTracker::PreCallValidateCmdDrawIndexedIndirectCountKHR(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCmdDrawIndexedIndirectCountKHR(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCmdDrawIndexedIndirectCountKHR(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkBuffer                                    countBuffer,
    VkDeviceSize                                countBufferOffset,
    uint32_t                                    maxDrawCount,
    uint32_t                                    stride) {
    ValidationStateTracker::PreCallRecordCmdDrawIndexedIndirectCountKHR(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCmdDrawIndexedIndirectCountKHR(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride); });
}

void BestPractices::PostCallRecordCmdDrawIndexedIndirectCountKHR(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkBuffer                                    countBuffer,
    VkDeviceSize                                countBufferOffset,
    uint32_t                                    maxDrawCount,
    uint32_t                                    stride) {
    ValidationStateTracker::PostCallRecordCmdDrawIndexedIndirectCountKHR(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCmdDrawIndexedIndirectCountKHR(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride); });
}

bool BestPractices::PreCallValidateGetSemaphoreCounterValueKHR(
    VkDevice                                    device,
    VkSemaphore                                 semaphore,
    uint64_t*                                   pValue) const {
    bool skip = ValidationStateTracker::PreCallValidateGetSemaphoreCounterValueKHR(device, semaphore, pValue);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetSemaphoreCounterValueKHR(device, semaphore, pValue); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetSemaphoreCounterValueKHR(
    VkDevice                                    device,
    VkSemaphore                                 semaphore,
    uint64_t*                                   pValue) {
    ValidationStateTracker::PreCallRecordGetSemaphoreCounterValueKHR(device, semaphore, pValue);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetSemaphoreCounterValueKHR(device, semaphore, pValue); });
}

void BestPractices::PostCallRecordGetSemaphoreCounterValueKHR(
    VkDevice                                    device,
    VkSemaphore                                 semaphore,
    uint64_t*                                   pValue,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetSemaphoreCounterValueKHR(device, semaphore, pValue, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetSemaphoreCounterValueKHR(device, semaphore, pValue, result); });
}

bool BestPractices::PreCallValidateWaitSemaphoresKHR(
    VkDevice                                    device,
    const VkSemaphoreWaitInfo*                  pWaitInfo,
    uint64_t                                    timeout) const {
    bool skip = ValidationStateTracker::PreCallValidateWaitSemaphoresKHR(device, pWaitInfo, timeout);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateWaitSemaphoresKHR(device, pWaitInfo, timeout); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordWaitSemaphoresKHR(
    VkDevice                                    device,
    const VkSemaphoreWaitInfo*                  pWaitInfo,
    uint64_t                                    timeout) {
    ValidationStateTracker::PreCallRecordWaitSemaphoresKHR(device, pWaitInfo, timeout);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordWaitSemaphoresKHR(device, pWaitInfo, timeout); });
}

void BestPractices::PostCallRecordWaitSemaphoresKHR(
    VkDevice                                    device,
    const VkSemaphoreWaitInfo*                  pWaitInfo,
    uint64_t                                    timeout,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordWaitSemaphoresKHR(device, pWaitInfo, timeout, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordWaitSemaphoresKHR(device, pWaitInfo, timeout, result); });
}

bool BestPractices::PreCallValidateSignalSemaphoreKHR(
    VkDevice                                    device,
    const VkSemaphoreSignalInfo*                pSignalInfo) const {
    bool skip = ValidationStateTracker::PreCallValidateSignalSemaphoreKHR(device, pSignalInfo);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateSignalSemaphoreKHR(device, pSignalInfo); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordSignalSemaphoreKHR(
    VkDevice                                    device,
    const VkSemaphoreSignalInfo*                pSignalInfo) {
    ValidationStateTracker::PreCallRecordSignalSemaphoreKHR(device, pSignalInfo);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordSignalSemaphoreKHR(device, pSignalInfo); });
}

void BestPractices::PostCallRecordSignalSemaphoreKHR(
    VkDevice                                    device,
    const VkSemaphoreSignalInfo*                pSignalInfo,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordSignalSemaphoreKHR(device, pSignalInfo, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordSignalSemaphoreKHR(device, pSignalInfo, result); });
}

bool BestPractices::PreCallValidateGetBufferDeviceAddressKHR(
    VkDevice                                    device,
    const VkBufferDeviceAddressInfo*            pInfo) const {
    bool skip = ValidationStateTracker::PreCallValidateGetBufferDeviceAddressKHR(device, pInfo);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetBufferDeviceAddressKHR(device, pInfo); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetBufferDeviceAddressKHR(
    VkDevice                                    device,
    const VkBufferDeviceAddressInfo*            pInfo) {
    ValidationStateTracker::PreCallRecordGetBufferDeviceAddressKHR(device, pInfo);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetBufferDeviceAddressKHR(device, pInfo); });
}

void BestPractices::PostCallRecordGetBufferDeviceAddressKHR(
    VkDevice                                    device,
    const VkBufferDeviceAddressInfo*            pInfo,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetBufferDeviceAddressKHR(device, pInfo, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetBufferDeviceAddressKHR(device, pInfo, result); });
}

bool BestPractices::PreCallValidateGetBufferOpaqueCaptureAddressKHR(
    VkDevice                                    device,
    const VkBufferDeviceAddressInfo*            pInfo) const {
    bool skip = ValidationStateTracker::PreCallValidateGetBufferOpaqueCaptureAddressKHR(device, pInfo);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetBufferOpaqueCaptureAddressKHR(device, pInfo); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetBufferOpaqueCaptureAddressKHR(
    VkDevice                                    device,
    const VkBufferDeviceAddressInfo*            pInfo) {
    ValidationStateTracker::PreCallRecordGetBufferOpaqueCaptureAddressKHR(device, pInfo);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetBufferOpaqueCaptureAddressKHR(device, pInfo); });
}

void BestPractices::PostCallRecordGetBufferOpaqueCaptureAddressKHR(
    VkDevice                                    device,
    const VkBufferDeviceAddressInfo*            pInfo) {
    ValidationStateTracker::PostCallRecordGetBufferOpaqueCaptureAddressKHR(device, pInfo);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetBufferOpaqueCaptureAddressKHR(device, pInfo); });
}

bool BestPractices::PreCallValidateGetDeviceMemoryOpaqueCaptureAddressKHR(
    VkDevice                                    device,
    const VkDeviceMemoryOpaqueCaptureAddressInfo* pInfo) const {
    bool skip = ValidationStateTracker::PreCallValidateGetDeviceMemoryOpaqueCaptureAddressKHR(device, pInfo);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetDeviceMemoryOpaqueCaptureAddressKHR(device, pInfo); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetDeviceMemoryOpaqueCaptureAddressKHR(
    VkDevice                                    device,
    const VkDeviceMemoryOpaqueCaptureAddressInfo* pInfo) {
    ValidationStateTracker::PreCallRecordGetDeviceMemoryOpaqueCaptureAddressKHR(device, pInfo);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetDeviceMemoryOpaqueCaptureAddressKHR(device, pInfo); });
}

void BestPractices::PostCallRecordGetDeviceMemoryOpaqueCaptureAddressKHR(
    VkDevice                                    device,
    const VkDeviceMemoryOpaqueCaptureAddressInfo* pInfo) {
    ValidationStateTracker::PostCallRecordGetDeviceMemoryOpaqueCaptureAddressKHR(device, pInfo);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetDeviceMemoryOpaqueCaptureAddressKHR(device, pInfo); });
}

bool BestPractices::PreCallValidateCreateDeferredOperationKHR(
    VkDevice                                    device,
    const VkAllocationCallbacks*                pAllocator,
    VkDeferredOperationKHR*                     pDeferredOperation) const {
    bool skip = ValidationStateTracker::PreCallValidateCreateDeferredOperationKHR(device, pAllocator, pDeferredOperation);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCreateDeferredOperationKHR(device, pAllocator, pDeferredOperation); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCreateDeferredOperationKHR(
    VkDevice                                    device,
    const VkAllocationCallbacks*                pAllocator,
    VkDeferredOperationKHR*                     pDeferredOperation) {
    ValidationStateTracker::PreCallRecordCreateDeferredOperationKHR(device, pAllocator, pDeferredOperation);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCreateDeferredOperationKHR(device, pAllocator, pDeferredOperation); });
}

void BestPractices::PostCallRecordCreateDeferredOperationKHR(
    VkDevice                                    device,
    const VkAllocationCallbacks*                pAllocator,
    VkDeferredOperationKHR*                     pDeferredOperation,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCreateDeferredOperationKHR(device, pAllocator, pDeferredOperation, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCreateDeferredOperationKHR(device, pAllocator, pDeferredOperation, result); });
}

bool BestPractices::PreCallValidateDestroyDeferredOperationKHR(
    VkDevice                                    device,
    VkDeferredOperationKHR                      operation,
    const VkAllocationCallbacks*                pAllocator) const {
    bool skip = ValidationStateTracker::PreCallValidateDestroyDeferredOperationKHR(device, operation, pAllocator);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateDestroyDeferredOperationKHR(device, operation, pAllocator); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordDestroyDeferredOperationKHR(
    VkDevice                                    device,
    VkDeferredOperationKHR                      operation,
    const VkAllocationCallbacks*                pAllocator) {
    ValidationStateTracker::PreCallRecordDestroyDeferredOperationKHR(device, operation, pAllocator);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordDestroyDeferredOperationKHR(device, operation, pAllocator); });
}

void BestPractices::PostCallRecordDestroyDeferredOperationKHR(
    VkDevice                                    device,
    VkDeferredOperationKHR                      operation,
    const VkAllocationCallbacks*                pAllocator) {
    ValidationStateTracker::PostCallRecordDestroyDeferredOperationKHR(device, operation, pAllocator);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordDestroyDeferredOperationKHR(device, operation, pAllocator); });
}

bool BestPractices::PreCallValidateGetDeferredOperationMaxConcurrencyKHR(
    VkDevice                                    device,
    VkDeferredOperationKHR                      operation) const {
    bool skip = ValidationStateTracker::PreCallValidateGetDeferredOperationMaxConcurrencyKHR(device, operation);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetDeferredOperationMaxConcurrencyKHR(device, operation); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetDeferredOperationMaxConcurrencyKHR(
    VkDevice                                    device,
    VkDeferredOperationKHR                      operation) {
    ValidationStateTracker::PreCallRecordGetDeferredOperationMaxConcurrencyKHR(device, operation);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetDeferredOperationMaxConcurrencyKHR(device, operation); });
}

void BestPractices::PostCallRecordGetDeferredOperationMaxConcurrencyKHR(
    VkDevice                                    device,
    VkDeferredOperationKHR                      operation) {
    ValidationStateTracker::PostCallRecordGetDeferredOperationMaxConcurrencyKHR(device, operation);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetDeferredOperationMaxConcurrencyKHR(device, operation); });
}

bool BestPractices::PreCallValidateGetDeferredOperationResultKHR(
    VkDevice                                    device,
    VkDeferredOperationKHR                      operation) const {
    bool skip = ValidationStateTracker::PreCallValidateGetDeferredOperationResultKHR(device, operation);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetDeferredOperationResultKHR(device, operation); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetDeferredOperationResultKHR(
    VkDevice                                    device,
    VkDeferredOperationKHR                      operation) {
    ValidationStateTracker::PreCallRecordGetDeferredOperationResultKHR(device, operation);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetDeferredOperationResultKHR(device, operation); });
}

void BestPractices::PostCallRecordGetDeferredOperationResultKHR(
    VkDevice                                    device,
    VkDeferredOperationKHR                      operation,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetDeferredOperationResultKHR(device, operation, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetDeferredOperationResultKHR(device, operation, result); });
}

bool BestPractices::PreCallValidateDeferredOperationJoinKHR(
    VkDevice                                    device,
    VkDeferredOperationKHR                      operation) const {
    bool skip = ValidationStateTracker::PreCallValidateDeferredOperationJoinKHR(device, operation);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateDeferredOperationJoinKHR(device, operation); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordDeferredOperationJoinKHR(
    VkDevice                                    device,
    VkDeferredOperationKHR                      operation) {
    ValidationStateTracker::PreCallRecordDeferredOperationJoinKHR(device, operation);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordDeferredOperationJoinKHR(device, operation); });
}

void BestPractices::PostCallRecordDeferredOperationJoinKHR(
    VkDevice                                    device,
    VkDeferredOperationKHR                      operation,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordDeferredOperationJoinKHR(device, operation, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordDeferredOperationJoinKHR(device, operation, result); });
}

bool BestPractices::PreCallValidateGetPipelineExecutablePropertiesKHR(
    VkDevice                                    device,
    const VkPipelineInfoKHR*                    pPipelineInfo,
    uint32_t*                                   pExecutableCount,
    VkPipelineExecutablePropertiesKHR*          pProperties) const {
    bool skip = ValidationStateTracker::PreCallValidateGetPipelineExecutablePropertiesKHR(device, pPipelineInfo, pExecutableCount, pProperties);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetPipelineExecutablePropertiesKHR(device, pPipelineInfo, pExecutableCount, pProperties); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetPipelineExecutablePropertiesKHR(
    VkDevice                                    device,
    const VkPipelineInfoKHR*                    pPipelineInfo,
    uint32_t*                                   pExecutableCount,
    VkPipelineExecutablePropertiesKHR*          pProperties) {
    ValidationStateTracker::PreCallRecordGetPipelineExecutablePropertiesKHR(device, pPipelineInfo, pExecutableCount, pProperties);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetPipelineExecutablePropertiesKHR(device, pPipelineInfo, pExecutableCount, pProperties); });
}

void BestPractices::PostCallRecordGetPipelineExecutablePropertiesKHR(
    VkDevice                                    device,
    const VkPipelineInfoKHR*                    pPipelineInfo,
    uint32_t*                                   pExecutableCount,
    VkPipelineExecutablePropertiesKHR*          pProperties,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetPipelineExecutablePropertiesKHR(device, pPipelineInfo, pExecutableCount, pProperties, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetPipelineExecutablePropertiesKHR(device, pPipelineInfo, pExecutableCount, pProperties, result); });
}

bool BestPractices::PreCallValidateGetPipelineExecutableStatisticsKHR(
    VkDevice                                    device,
    const VkPipelineExecutableInfoKHR*          pExecutableInfo,
    uint32_t*                                   pStatisticCount,
    VkPipelineExecutableStatisticKHR*           pStatistics) const {
    bool skip = ValidationStateTracker::PreCallValidateGetPipelineExecutableStatisticsKHR(device, pExecutableInfo, pStatisticCount, pStatistics);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetPipelineExecutableStatisticsKHR(device, pExecutableInfo, pStatisticCount, pStatistics); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetPipelineExecutableStatisticsKHR(
    VkDevice                                    device,
    const VkPipelineExecutableInfoKHR*          pExecutableInfo,
    uint32_t*                                   pStatisticCount,
    VkPipelineExecutableStatisticKHR*           pStatistics) {
    ValidationStateTracker::PreCallRecordGetPipelineExecutableStatisticsKHR(device, pExecutableInfo, pStatisticCount, pStatistics);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetPipelineExecutableStatisticsKHR(device, pExecutableInfo, pStatisticCount, pStatistics); });
}

void BestPractices::PostCallRecordGetPipelineExecutableStatisticsKHR(
    VkDevice                                    device,
    const VkPipelineExecutableInfoKHR*          pExecutableInfo,
    uint32_t*                                   pStatisticCount,
    VkPipelineExecutableStatisticKHR*           pStatistics,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetPipelineExecutableStatisticsKHR(device, pExecutableInfo, pStatisticCount, pStatistics, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetPipelineExecutableStatisticsKHR(device, pExecutableInfo, pStatisticCount, pStatistics, result); });
}

bool BestPractices::PreCallValidateGetPipelineExecutableInternalRepresentationsKHR(
    VkDevice                                    device,
    const VkPipelineExecutableInfoKHR*          pExecutableInfo,
    uint32_t*                                   pInternalRepresentationCount,
    VkPipelineExecutableInternalRepresentationKHR* pInternalRepresentations) const {
    bool skip = ValidationStateTracker::PreCallValidateGetPipelineExecutableInternalRepresentationsKHR(device, pExecutableInfo, pInternalRepresentationCount, pInternalRepresentations);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetPipelineExecutableInternalRepresentationsKHR(device, pExecutableInfo, pInternalRepresentationCount, pInternalRepresentations); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetPipelineExecutableInternalRepresentationsKHR(
    VkDevice                                    device,
    const VkPipelineExecutableInfoKHR*          pExecutableInfo,
    uint32_t*                                   pInternalRepresentationCount,
    VkPipelineExecutableInternalRepresentationKHR* pInternalRepresentations) {
    ValidationStateTracker::PreCallRecordGetPipelineExecutableInternalRepresentationsKHR(device, pExecutableInfo, pInternalRepresentationCount, pInternalRepresentations);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetPipelineExecutableInternalRepresentationsKHR(device, pExecutableInfo, pInternalRepresentationCount, pInternalRepresentations); });
}

void BestPractices::PostCallRecordGetPipelineExecutableInternalRepresentationsKHR(
    VkDevice                                    device,
    const VkPipelineExecutableInfoKHR*          pExecutableInfo,
    uint32_t*                                   pInternalRepresentationCount,
    VkPipelineExecutableInternalRepresentationKHR* pInternalRepresentations,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetPipelineExecutableInternalRepresentationsKHR(device, pExecutableInfo, pInternalRepresentationCount, pInternalRepresentations, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetPipelineExecutableInternalRepresentationsKHR(device, pExecutableInfo, pInternalRepresentationCount, pInternalRepresentations, result); });
}

bool BestPractices::PreCallValidateCreateDebugReportCallbackEXT(
    VkInstance                                  instance,
    const VkDebugReportCallbackCreateInfoEXT*   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDebugReportCallbackEXT*                   pCallback) const {
    bool skip = ValidationStateTracker::PreCallValidateCreateDebugReportCallbackEXT(instance, pCreateInfo, pAllocator, pCallback);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCreateDebugReportCallbackEXT(instance, pCreateInfo, pAllocator, pCallback); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCreateDebugReportCallbackEXT(
    VkInstance                                  instance,
    const VkDebugReportCallbackCreateInfoEXT*   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDebugReportCallbackEXT*                   pCallback) {
    ValidationStateTracker::PreCallRecordCreateDebugReportCallbackEXT(instance, pCreateInfo, pAllocator, pCallback);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCreateDebugReportCallbackEXT(instance, pCreateInfo, pAllocator, pCallback); });
}

void BestPractices::PostCallRecordCreateDebugReportCallbackEXT(
    VkInstance                                  instance,
    const VkDebugReportCallbackCreateInfoEXT*   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDebugReportCallbackEXT*                   pCallback,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCreateDebugReportCallbackEXT(instance, pCreateInfo, pAllocator, pCallback, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCreateDebugReportCallbackEXT(instance, pCreateInfo, pAllocator, pCallback, result); });
}

bool BestPractices::PreCallValidateDestroyDebugReportCallbackEXT(
    VkInstance                                  instance,
    VkDebugReportCallbackEXT                    callback,
    const VkAllocationCallbacks*                pAllocator) const {
    bool skip = ValidationStateTracker::PreCallValidateDestroyDebugReportCallbackEXT(instance, callback, pAllocator);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateDestroyDebugReportCallbackEXT(instance, callback, pAllocator); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordDestroyDebugReportCallbackEXT(
    VkInstance                                  instance,
    VkDebugReportCallbackEXT                    callback,
    const VkAllocationCallbacks*                pAllocator) {
    ValidationStateTracker::PreCallRecordDestroyDebugReportCallbackEXT(instance, callback, pAllocator);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordDestroyDebugReportCallbackEXT(instance, callback, pAllocator); });
}

void BestPractices::PostCallRecordDestroyDebugReportCallbackEXT(
    VkInstance                                  instance,
    VkDebugReportCallbackEXT                    callback,
    const VkAllocationCallbacks*                pAllocator) {
    ValidationStateTracker::PostCallRecordDestroyDebugReportCallbackEXT(instance, callback, pAllocator);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordDestroyDebugReportCallbackEXT(instance, callback, pAllocator); });
}

bool BestPractices::PreCallValidateDebugReportMessageEXT(
    VkInstance                                  instance,
    VkDebugReportFlagsEXT                       flags,
    VkDebugReportObjectTypeEXT                  objectType,
    uint64_t                                    object,
    size_t                                      location,
    int32_t                                     messageCode,
    const char*                                 pLayerPrefix,
    const char*                                 pMessage) const {
    bool skip = ValidationStateTracker::PreCallValidateDebugReportMessageEXT(instance, flags, objectType, object, location, messageCode, pLayerPrefix, pMessage);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateDebugReportMessageEXT(instance, flags, objectType, object, location, messageCode, pLayerPrefix, pMessage); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordDebugReportMessageEXT(
    VkInstance                                  instance,
    VkDebugReportFlagsEXT                       flags,
    VkDebugReportObjectTypeEXT                  objectType,
    uint64_t                                    object,
    size_t                                      location,
    int32_t                                     messageCode,
    const char*                                 pLayerPrefix,
    const char*                                 pMessage) {
    ValidationStateTracker::PreCallRecordDebugReportMessageEXT(instance, flags, objectType, object, location, messageCode, pLayerPrefix, pMessage);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordDebugReportMessageEXT(instance, flags, objectType, object, location, messageCode, pLayerPrefix, pMessage); });
}

void BestPractices::PostCallRecordDebugReportMessageEXT(
    VkInstance                                  instance,
    VkDebugReportFlagsEXT                       flags,
    VkDebugReportObjectTypeEXT                  objectType,
    uint64_t                                    object,
    size_t                                      location,
    int32_t                                     messageCode,
    const char*                                 pLayerPrefix,
    const char*                                 pMessage) {
    ValidationStateTracker::PostCallRecordDebugReportMessageEXT(instance, flags, objectType, object, location, messageCode, pLayerPrefix, pMessage);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordDebugReportMessageEXT(instance, flags, objectType, object, location, messageCode, pLayerPrefix, pMessage); });
}

bool BestPractices::PreCallValidateDebugMarkerSetObjectTagEXT(
    VkDevice                                    device,
    const VkDebugMarkerObjectTagInfoEXT*        pTagInfo) const {
    bool skip = ValidationStateTracker::PreCallValidateDebugMarkerSetObjectTagEXT(device, pTagInfo);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateDebugMarkerSetObjectTagEXT(device, pTagInfo); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordDebugMarkerSetObjectTagEXT(
    VkDevice                                    device,
    const VkDebugMarkerObjectTagInfoEXT*        pTagInfo) {
    ValidationStateTracker::PreCallRecordDebugMarkerSetObjectTagEXT(device, pTagInfo);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordDebugMarkerSetObjectTagEXT(device, pTagInfo); });
}

void BestPractices::PostCallRecordDebugMarkerSetObjectTagEXT(
    VkDevice                                    device,
    const VkDebugMarkerObjectTagInfoEXT*        pTagInfo,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordDebugMarkerSetObjectTagEXT(device, pTagInfo, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordDebugMarkerSetObjectTagEXT(device, pTagInfo, result); });
}

bool BestPractices::PreCallValidateDebugMarkerSetObjectNameEXT(
    VkDevice                                    device,
    const VkDebugMarkerObjectNameInfoEXT*       pNameInfo) const {
    bool skip = ValidationStateTracker::PreCallValidateDebugMarkerSetObjectNameEXT(device, pNameInfo);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateDebugMarkerSetObjectNameEXT(device, pNameInfo); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordDebugMarkerSetObjectNameEXT(
    VkDevice                                    device,
    const VkDebugMarkerObjectNameInfoEXT*       pNameInfo) {
    ValidationStateTracker::PreCallRecordDebugMarkerSetObjectNameEXT(device, pNameInfo);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordDebugMarkerSetObjectNameEXT(device, pNameInfo); });
}

void BestPractices::PostCallRecordDebugMarkerSetObjectNameEXT(
    VkDevice                                    device,
    const VkDebugMarkerObjectNameInfoEXT*       pNameInfo,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordDebugMarkerSetObjectNameEXT(device, pNameInfo, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordDebugMarkerSetObjectNameEXT(device, pNameInfo, result); });
}

bool BestPractices::PreCallValidateCmdDebugMarkerBeginEXT(
    VkCommandBuffer                             commandBuffer,
    const VkDebugMarkerMarkerInfoEXT*           pMarkerInfo) const {
    bool skip = ValidationStateTracker::PreCallValidateCmdDebugMarkerBeginEXT(commandBuffer, pMarkerInfo);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCmdDebugMarkerBeginEXT(commandBuffer, pMarkerInfo); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCmdDebugMarkerBeginEXT(
    VkCommandBuffer                             commandBuffer,
    const VkDebugMarkerMarkerInfoEXT*           pMarkerInfo) {
    ValidationStateTracker::PreCallRecordCmdDebugMarkerBeginEXT(commandBuffer, pMarkerInfo);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCmdDebugMarkerBeginEXT(commandBuffer, pMarkerInfo); });
}

void BestPractices::PostCallRecordCmdDebugMarkerBeginEXT(
    VkCommandBuffer                             commandBuffer,
    const VkDebugMarkerMarkerInfoEXT*           pMarkerInfo) {
    ValidationStateTracker::PostCallRecordCmdDebugMarkerBeginEXT(commandBuffer, pMarkerInfo);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCmdDebugMarkerBeginEXT(commandBuffer, pMarkerInfo); });
}

bool BestPractices::PreCallValidateCmdDebugMarkerEndEXT(
    VkCommandBuffer                             commandBuffer) const {
    bool skip = ValidationStateTracker::PreCallValidateCmdDebugMarkerEndEXT(commandBuffer);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCmdDebugMarkerEndEXT(commandBuffer); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCmdDebugMarkerEndEXT(
    VkCommandBuffer                             commandBuffer) {
    ValidationStateTracker::PreCallRecordCmdDebugMarkerEndEXT(commandBuffer);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCmdDebugMarkerEndEXT(commandBuffer); });
}

void BestPractices::PostCallRecordCmdDebugMarkerEndEXT(
    VkCommandBuffer                             commandBuffer) {
    ValidationStateTracker::PostCallRecordCmdDebugMarkerEndEXT(commandBuffer);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCmdDebugMarkerEndEXT(commandBuffer); });
}

bool BestPractices::PreCallValidateCmdDebugMarkerInsertEXT(
    VkCommandBuffer                             commandBuffer,
    const VkDebugMarkerMarkerInfoEXT*           pMarkerInfo) const {
    bool skip = ValidationStateTracker::PreCallValidateCmdDebugMarkerInsertEXT(commandBuffer, pMarkerInfo);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCmdDebugMarkerInsertEXT(commandBuffer, pMarkerInfo); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCmdDebugMarkerInsertEXT(
    VkCommandBuffer                             commandBuffer,
    const VkDebugMarkerMarkerInfoEXT*           pMarkerInfo) {
    ValidationStateTracker::PreCallRecordCmdDebugMarkerInsertEXT(commandBuffer, pMarkerInfo);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCmdDebugMarkerInsertEXT(commandBuffer, pMarkerInfo); });
}

void BestPractices::PostCallRecordCmdDebugMarkerInsertEXT(
    VkCommandBuffer                             commandBuffer,
    const VkDebugMarkerMarkerInfoEXT*           pMarkerInfo) {
    ValidationStateTracker::PostCallRecordCmdDebugMarkerInsertEXT(commandBuffer, pMarkerInfo);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCmdDebugMarkerInsertEXT(commandBuffer, pMarkerInfo); });
}

bool BestPractices::PreCallValidateCmdBindTransformFeedbackBuffersEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstBinding,
    uint32_t                                    bindingCount,
    const VkBuffer*                             pBuffers,
    const VkDeviceSize*                         pOffsets,
    const VkDeviceSize*                         pSizes) const {
    bool skip = ValidationStateTracker::PreCallValidateCmdBindTransformFeedbackBuffersEXT(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets, pSizes);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCmdBindTransformFeedbackBuffersEXT(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets, pSizes); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCmdBindTransformFeedbackBuffersEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstBinding,
    uint32_t                                    bindingCount,
    const VkBuffer*                             pBuffers,
    const VkDeviceSize*                         pOffsets,
    const VkDeviceSize*                         pSizes) {
    ValidationStateTracker::PreCallRecordCmdBindTransformFeedbackBuffersEXT(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets, pSizes);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCmdBindTransformFeedbackBuffersEXT(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets, pSizes); });
}

void BestPractices::PostCallRecordCmdBindTransformFeedbackBuffersEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstBinding,
    uint32_t                                    bindingCount,
    const VkBuffer*                             pBuffers,
    const VkDeviceSize*                         pOffsets,
    const VkDeviceSize*                         pSizes) {
    ValidationStateTracker::PostCallRecordCmdBindTransformFeedbackBuffersEXT(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets, pSizes);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCmdBindTransformFeedbackBuffersEXT(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets, pSizes); });
}

bool BestPractices::PreCallValidateCmdBeginTransformFeedbackEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstCounterBuffer,
    uint32_t                                    counterBufferCount,
    const VkBuffer*                             pCounterBuffers,
    const VkDeviceSize*                         pCounterBufferOffsets) const {
    bool skip = ValidationStateTracker::PreCallValidateCmdBeginTransformFeedbackEXT(commandBuffer, firstCounterBuffer, counterBufferCount, pCounterBuffers, pCounterBufferOffsets);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCmdBeginTransformFeedbackEXT(commandBuffer, firstCounterBuffer, counterBufferCount, pCounterBuffers, pCounterBufferOffsets); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCmdBeginTransformFeedbackEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstCounterBuffer,
    uint32_t                                    counterBufferCount,
    const VkBuffer*                             pCounterBuffers,
    const VkDeviceSize*                         pCounterBufferOffsets) {
    ValidationStateTracker::PreCallRecordCmdBeginTransformFeedbackEXT(commandBuffer, firstCounterBuffer, counterBufferCount, pCounterBuffers, pCounterBufferOffsets);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCmdBeginTransformFeedbackEXT(commandBuffer, firstCounterBuffer, counterBufferCount, pCounterBuffers, pCounterBufferOffsets); });
}

void BestPractices::PostCallRecordCmdBeginTransformFeedbackEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstCounterBuffer,
    uint32_t                                    counterBufferCount,
    const VkBuffer*                             pCounterBuffers,
    const VkDeviceSize*                         pCounterBufferOffsets) {
    ValidationStateTracker::PostCallRecordCmdBeginTransformFeedbackEXT(commandBuffer, firstCounterBuffer, counterBufferCount, pCounterBuffers, pCounterBufferOffsets);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCmdBeginTransformFeedbackEXT(commandBuffer, firstCounterBuffer, counterBufferCount, pCounterBuffers, pCounterBufferOffsets); });
}

bool BestPractices::PreCallValidateCmdEndTransformFeedbackEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstCounterBuffer,
    uint32_t                                    counterBufferCount,
    const VkBuffer*                             pCounterBuffers,
    const VkDeviceSize*                         pCounterBufferOffsets) const {
    bool skip = ValidationStateTracker::PreCallValidateCmdEndTransformFeedbackEXT(commandBuffer, firstCounterBuffer, counterBufferCount, pCounterBuffers, pCounterBufferOffsets);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCmdEndTransformFeedbackEXT(commandBuffer, firstCounterBuffer, counterBufferCount, pCounterBuffers, pCounterBufferOffsets); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCmdEndTransformFeedbackEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstCounterBuffer,
    uint32_t                                    counterBufferCount,
    const VkBuffer*                             pCounterBuffers,
    const VkDeviceSize*                         pCounterBufferOffsets) {
    ValidationStateTracker::PreCallRecordCmdEndTransformFeedbackEXT(commandBuffer, firstCounterBuffer, counterBufferCount, pCounterBuffers, pCounterBufferOffsets);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCmdEndTransformFeedbackEXT(commandBuffer, firstCounterBuffer, counterBufferCount, pCounterBuffers, pCounterBufferOffsets); });
}

void BestPractices::PostCallRecordCmdEndTransformFeedbackEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstCounterBuffer,
    uint32_t                                    counterBufferCount,
    const VkBuffer*                             pCounterBuffers,
    const VkDeviceSize*                         pCounterBufferOffsets) {
    ValidationStateTracker::PostCallRecordCmdEndTransformFeedbackEXT(commandBuffer, firstCounterBuffer, counterBufferCount, pCounterBuffers, pCounterBufferOffsets);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCmdEndTransformFeedbackEXT(commandBuffer, firstCounterBuffer, counterBufferCount, pCounterBuffers, pCounterBufferOffsets); });
}

bool BestPractices::PreCallValidateCmdBeginQueryIndexedEXT(
    VkCommandBuffer                             commandBuffer,
    VkQueryPool                                 queryPool,
    uint32_t                                    query,
    VkQueryControlFlags                         flags,
    uint32_t                                    index) const {
    bool skip = ValidationStateTracker::PreCallValidateCmdBeginQueryIndexedEXT(commandBuffer, queryPool, query, flags, index);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCmdBeginQueryIndexedEXT(commandBuffer, queryPool, query, flags, index); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCmdBeginQueryIndexedEXT(
    VkCommandBuffer                             commandBuffer,
    VkQueryPool                                 queryPool,
    uint32_t                                    query,
    VkQueryControlFlags                         flags,
    uint32_t                                    index) {
    ValidationStateTracker::PreCallRecordCmdBeginQueryIndexedEXT(commandBuffer, queryPool, query, flags, index);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCmdBeginQueryIndexedEXT(commandBuffer, queryPool, query, flags, index); });
}

void BestPractices::PostCallRecordCmdBeginQueryIndexedEXT(
    VkCommandBuffer                             commandBuffer,
    VkQueryPool                                 queryPool,
    uint32_t                                    query,
    VkQueryControlFlags                         flags,
    uint32_t                                    index) {
    ValidationStateTracker::PostCallRecordCmdBeginQueryIndexedEXT(commandBuffer, queryPool, query, flags, index);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCmdBeginQueryIndexedEXT(commandBuffer, queryPool, query, flags, index); });
}

bool BestPractices::PreCallValidateCmdEndQueryIndexedEXT(
    VkCommandBuffer                             commandBuffer,
    VkQueryPool                                 queryPool,
    uint32_t                                    query,
    uint32_t                                    index) const {
    bool skip = ValidationStateTracker::PreCallValidateCmdEndQueryIndexedEXT(commandBuffer, queryPool, query, index);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCmdEndQueryIndexedEXT(commandBuffer, queryPool, query, index); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCmdEndQueryIndexedEXT(
    VkCommandBuffer                             commandBuffer,
    VkQueryPool                                 queryPool,
    uint32_t                                    query,
    uint32_t                                    index) {
    ValidationStateTracker::PreCallRecordCmdEndQueryIndexedEXT(commandBuffer, queryPool, query, index);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCmdEndQueryIndexedEXT(commandBuffer, queryPool, query, index); });
}

void BestPractices::PostCallRecordCmdEndQueryIndexedEXT(
    VkCommandBuffer                             commandBuffer,
    VkQueryPool                                 queryPool,
    uint32_t                                    query,
    uint32_t                                    index) {
    ValidationStateTracker::PostCallRecordCmdEndQueryIndexedEXT(commandBuffer, queryPool, query, index);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCmdEndQueryIndexedEXT(commandBuffer, queryPool, query, index); });
}

bool BestPractices::PreCallValidateCmdDrawIndirectByteCountEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    instanceCount,
    uint32_t                                    firstInstance,
    VkBuffer                                    counterBuffer,
    VkDeviceSize                                counterBufferOffset,
    uint32_t                                    counterOffset,
    uint32_t                                    vertexStride) const {
    bool skip = ValidationStateTracker::PreCallValidateCmdDrawIndirectByteCountEXT(commandBuffer, instanceCount, firstInstance, counterBuffer, counterBufferOffset, counterOffset, vertexStride);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCmdDrawIndirectByteCountEXT(commandBuffer, instanceCount, firstInstance, counterBuffer, counterBufferOffset, counterOffset, vertexStride); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCmdDrawIndirectByteCountEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    instanceCount,
    uint32_t                                    firstInstance,
    VkBuffer                                    counterBuffer,
    VkDeviceSize                                counterBufferOffset,
    uint32_t                                    counterOffset,
    uint32_t                                    vertexStride) {
    ValidationStateTracker::PreCallRecordCmdDrawIndirectByteCountEXT(commandBuffer, instanceCount, firstInstance, counterBuffer, counterBufferOffset, counterOffset, vertexStride);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCmdDrawIndirectByteCountEXT(commandBuffer, instanceCount, firstInstance, counterBuffer, counterBufferOffset, counterOffset, vertexStride); });
}

void BestPractices::PostCallRecordCmdDrawIndirectByteCountEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    instanceCount,
    uint32_t                                    firstInstance,
    VkBuffer                                    counterBuffer,
    VkDeviceSize                                counterBufferOffset,
    uint32_t                                    counterOffset,
    uint32_t                                    vertexStride) {
    ValidationStateTracker::PostCallRecordCmdDrawIndirectByteCountEXT(commandBuffer, instanceCount, firstInstance, counterBuffer, counterBufferOffset, counterOffset, vertexStride);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCmdDrawIndirectByteCountEXT(commandBuffer, instanceCount, firstInstance, counterBuffer, counterBufferOffset, counterOffset, vertexStride); });
}

bool BestPractices::PreCallValidateGetImageViewHandleNVX(
    VkDevice                                    device,
    const VkImageViewHandleInfoNVX*             pInfo) const {
    bool skip = ValidationStateTracker::PreCallValidateGetImageViewHandleNVX(device, pInfo);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetImageViewHandleNVX(device, pInfo); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetImageViewHandleNVX(
    VkDevice                                    device,
    const VkImageViewHandleInfoNVX*             pInfo) {
    ValidationStateTracker::PreCallRecordGetImageViewHandleNVX(device, pInfo);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetImageViewHandleNVX(device, pInfo); });
}

void BestPractices::PostCallRecordGetImageViewHandleNVX(
    VkDevice                                    device,
    const VkImageViewHandleInfoNVX*             pInfo) {
    ValidationStateTracker::PostCallRecordGetImageViewHandleNVX(device, pInfo);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetImageViewHandleNVX(device, pInfo); });
}

bool BestPractices::PreCallValidateGetImageViewAddressNVX(
    VkDevice                                    device,
    VkImageView                                 imageView,
    VkImageViewAddressPropertiesNVX*            pProperties) const {
    bool skip = ValidationStateTracker::PreCallValidateGetImageViewAddressNVX(device, imageView, pProperties);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetImageViewAddressNVX(device, imageView, pProperties); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetImageViewAddressNVX(
    VkDevice                                    device,
    VkImageView                                 imageView,
    VkImageViewAddressPropertiesNVX*            pProperties) {
    ValidationStateTracker::PreCallRecordGetImageViewAddressNVX(device, imageView, pProperties);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetImageViewAddressNVX(device, imageView, pProperties); });
}

void BestPractices::PostCallRecordGetImageViewAddressNVX(
    VkDevice                                    device,
    VkImageView                                 imageView,
    VkImageViewAddressPropertiesNVX*            pProperties,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetImageViewAddressNVX(device, imageView, pProperties, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetImageViewAddressNVX(device, imageView, pProperties, result); });
}

bool BestPractices::PreCallValidateCmdDrawIndirectCountAMD(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkBuffer                                    countBuffer,
    VkDeviceSize                                countBufferOffset,
    uint32_t                                    maxDrawCount,
    uint32_t                                    stride) const {
    bool skip = ValidationStateTracker::PreCallValidateCmdDrawIndirectCountAMD(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCmdDrawIndirectCountAMD(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCmdDrawIndirectCountAMD(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkBuffer                                    countBuffer,
    VkDeviceSize                                countBufferOffset,
    uint32_t                                    maxDrawCount,
    uint32_t                                    stride) {
    ValidationStateTracker::PreCallRecordCmdDrawIndirectCountAMD(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCmdDrawIndirectCountAMD(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride); });
}

void BestPractices::PostCallRecordCmdDrawIndirectCountAMD(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkBuffer                                    countBuffer,
    VkDeviceSize                                countBufferOffset,
    uint32_t                                    maxDrawCount,
    uint32_t                                    stride) {
    ValidationStateTracker::PostCallRecordCmdDrawIndirectCountAMD(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCmdDrawIndirectCountAMD(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride); });
}

bool BestPractices::PreCallValidateCmdDrawIndexedIndirectCountAMD(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkBuffer                                    countBuffer,
    VkDeviceSize                                countBufferOffset,
    uint32_t                                    maxDrawCount,
    uint32_t                                    stride) const {
    bool skip = ValidationStateTracker::PreCallValidateCmdDrawIndexedIndirectCountAMD(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCmdDrawIndexedIndirectCountAMD(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCmdDrawIndexedIndirectCountAMD(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkBuffer                                    countBuffer,
    VkDeviceSize                                countBufferOffset,
    uint32_t                                    maxDrawCount,
    uint32_t                                    stride) {
    ValidationStateTracker::PreCallRecordCmdDrawIndexedIndirectCountAMD(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCmdDrawIndexedIndirectCountAMD(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride); });
}

void BestPractices::PostCallRecordCmdDrawIndexedIndirectCountAMD(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkBuffer                                    countBuffer,
    VkDeviceSize                                countBufferOffset,
    uint32_t                                    maxDrawCount,
    uint32_t                                    stride) {
    ValidationStateTracker::PostCallRecordCmdDrawIndexedIndirectCountAMD(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCmdDrawIndexedIndirectCountAMD(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride); });
}

bool BestPractices::PreCallValidateGetShaderInfoAMD(
    VkDevice                                    device,
    VkPipeline                                  pipeline,
    VkShaderStageFlagBits                       shaderStage,
    VkShaderInfoTypeAMD                         infoType,
    size_t*                                     pInfoSize,
    void*                                       pInfo) const {
    bool skip = ValidationStateTracker::PreCallValidateGetShaderInfoAMD(device, pipeline, shaderStage, infoType, pInfoSize, pInfo);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetShaderInfoAMD(device, pipeline, shaderStage, infoType, pInfoSize, pInfo); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetShaderInfoAMD(
    VkDevice                                    device,
    VkPipeline                                  pipeline,
    VkShaderStageFlagBits                       shaderStage,
    VkShaderInfoTypeAMD                         infoType,
    size_t*                                     pInfoSize,
    void*                                       pInfo) {
    ValidationStateTracker::PreCallRecordGetShaderInfoAMD(device, pipeline, shaderStage, infoType, pInfoSize, pInfo);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetShaderInfoAMD(device, pipeline, shaderStage, infoType, pInfoSize, pInfo); });
}

void BestPractices::PostCallRecordGetShaderInfoAMD(
    VkDevice                                    device,
    VkPipeline                                  pipeline,
    VkShaderStageFlagBits                       shaderStage,
    VkShaderInfoTypeAMD                         infoType,
    size_t*                                     pInfoSize,
    void*                                       pInfo,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetShaderInfoAMD(device, pipeline, shaderStage, infoType, pInfoSize, pInfo, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetShaderInfoAMD(device, pipeline, shaderStage, infoType, pInfoSize, pInfo, result); });
}

bool BestPractices::PreCallValidateCreateStreamDescriptorSurfaceGGP(
    VkInstance                                  instance,
    const VkStreamDescriptorSurfaceCreateInfoGGP* pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) const {
    bool skip = ValidationStateTracker::PreCallValidateCreateStreamDescriptorSurfaceGGP(instance, pCreateInfo, pAllocator, pSurface);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCreateStreamDescriptorSurfaceGGP(instance, pCreateInfo, pAllocator, pSurface); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCreateStreamDescriptorSurfaceGGP(
    VkInstance                                  instance,
    const VkStreamDescriptorSurfaceCreateInfoGGP* pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) {
    ValidationStateTracker::PreCallRecordCreateStreamDescriptorSurfaceGGP(instance, pCreateInfo, pAllocator, pSurface);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCreateStreamDescriptorSurfaceGGP(instance, pCreateInfo, pAllocator, pSurface); });
}

void BestPractices::PostCallRecordCreateStreamDescriptorSurfaceGGP(
    VkInstance                                  instance,
    const VkStreamDescriptorSurfaceCreateInfoGGP* pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCreateStreamDescriptorSurfaceGGP(instance, pCreateInfo, pAllocator, pSurface, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCreateStreamDescriptorSurfaceGGP(instance, pCreateInfo, pAllocator, pSurface, result); });
}

bool BestPractices::PreCallValidateGetPhysicalDeviceExternalImageFormatPropertiesNV(
    VkPhysicalDevice                            physicalDevice,
    VkFormat                                    format,
    VkImageType                                 type,
    VkImageTiling                               tiling,
    VkImageUsageFlags                           usage,
    VkImageCreateFlags                          flags,
    VkExternalMemoryHandleTypeFlagsNV           externalHandleType,
    VkExternalImageFormatPropertiesNV*          pExternalImageFormatProperties) const {
    bool skip = ValidationStateTracker::PreCallValidateGetPhysicalDeviceExternalImageFormatPropertiesNV(physicalDevice, format, type, tiling, usage, flags, externalHandleType, pExternalImageFormatProperties);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetPhysicalDeviceExternalImageFormatPropertiesNV(physicalDevice, format, type, tiling, usage, flags, externalHandleType, pExternalImageFormatProperties); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetPhysicalDeviceExternalImageFormatPropertiesNV(
    VkPhysicalDevice                            physicalDevice,
    VkFormat                                    format,
    VkImageType                                 type,
    VkImageTiling                               tiling,
    VkImageUsageFlags                           usage,
    VkImageCreateFlags                          flags,
    VkExternalMemoryHandleTypeFlagsNV           externalHandleType,
    VkExternalImageFormatPropertiesNV*          pExternalImageFormatProperties) {
    ValidationStateTracker::PreCallRecordGetPhysicalDeviceExternalImageFormatPropertiesNV(physicalDevice, format, type, tiling, usage, flags, externalHandleType, pExternalImageFormatProperties);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetPhysicalDeviceExternalImageFormatPropertiesNV(physicalDevice, format, type, tiling, usage, flags, externalHandleType, pExternalImageFormatProperties); });
}

void BestPractices::PostCallRecordGetPhysicalDeviceExternalImageFormatPropertiesNV(
    VkPhysicalDevice                            physicalDevice,
    VkFormat                                    format,
    VkImageType                                 type,
    VkImageTiling                               tiling,
    VkImageUsageFlags                           usage,
    VkImageCreateFlags                          flags,
    VkExternalMemoryHandleTypeFlagsNV           externalHandleType,
    VkExternalImageFormatPropertiesNV*          pExternalImageFormatProperties,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceExternalImageFormatPropertiesNV(physicalDevice, format, type, tiling, usage, flags, externalHandleType, pExternalImageFormatProperties, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetPhysicalDeviceExternalImageFormatPropertiesNV(physicalDevice, format, type, tiling, usage, flags, externalHandleType, pExternalImageFormatProperties, result); });
}

bool BestPractices::PreCallValidateGetMemoryWin32HandleNV(
    VkDevice                                    device,
    VkDeviceMemory                              memory,
    VkExternalMemoryHandleTypeFlagsNV           handleType,
    HANDLE*                                     pHandle) const {
    bool skip = ValidationStateTracker::PreCallValidateGetMemoryWin32HandleNV(device, memory, handleType, pHandle);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetMemoryWin32HandleNV(device, memory, handleType, pHandle); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetMemoryWin32HandleNV(
    VkDevice                                    device,
    VkDeviceMemory                              memory,
    VkExternalMemoryHandleTypeFlagsNV           handleType,
    HANDLE*                                     pHandle) {
    ValidationStateTracker::PreCallRecordGetMemoryWin32HandleNV(device, memory, handleType, pHandle);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetMemoryWin32HandleNV(device, memory, handleType, pHandle); });
}

void BestPractices::PostCallRecordGetMemoryWin32HandleNV(
    VkDevice                                    device,
    VkDeviceMemory                              memory,
    VkExternalMemoryHandleTypeFlagsNV           handleType,
    HANDLE*                                     pHandle,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetMemoryWin32HandleNV(device, memory, handleType, pHandle, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetMemoryWin32HandleNV(device, memory, handleType, pHandle, result); });
}

bool BestPractices::PreCallValidateCreateViSurfaceNN(
    VkInstance                                  instance,
    const VkViSurfaceCreateInfoNN*              pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) const {
    bool skip = ValidationStateTracker::PreCallValidateCreateViSurfaceNN(instance, pCreateInfo, pAllocator, pSurface);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCreateViSurfaceNN(instance, pCreateInfo, pAllocator, pSurface); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCreateViSurfaceNN(
    VkInstance                                  instance,
    const VkViSurfaceCreateInfoNN*              pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) {
    ValidationStateTracker::PreCallRecordCreateViSurfaceNN(instance, pCreateInfo, pAllocator, pSurface);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCreateViSurfaceNN(instance, pCreateInfo, pAllocator, pSurface); });
}

void BestPractices::PostCallRecordCreateViSurfaceNN(
    VkInstance                                  instance,
    const VkViSurfaceCreateInfoNN*              pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCreateViSurfaceNN(instance, pCreateInfo, pAllocator, pSurface, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCreateViSurfaceNN(instance, pCreateInfo, pAllocator, pSurface, result); });
}

bool BestPractices::PreCallValidateCmdBeginConditionalRenderingEXT(
    VkCommandBuffer                             commandBuffer,
    const VkConditionalRenderingBeginInfoEXT*   pConditionalRenderingBegin) const {
    bool skip = ValidationStateTracker::PreCallValidateCmdBeginConditionalRenderingEXT(commandBuffer, pConditionalRenderingBegin);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCmdBeginConditionalRenderingEXT(commandBuffer, pConditionalRenderingBegin); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCmdBeginConditionalRenderingEXT(
    VkCommandBuffer                             commandBuffer,
    const VkConditionalRenderingBeginInfoEXT*   pConditionalRenderingBegin) {
    ValidationStateTracker::PreCallRecordCmdBeginConditionalRenderingEXT(commandBuffer, pConditionalRenderingBegin);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCmdBeginConditionalRenderingEXT(commandBuffer, pConditionalRenderingBegin); });
}

void BestPractices::PostCallRecordCmdBeginConditionalRenderingEXT(
    VkCommandBuffer                             commandBuffer,
    const VkConditionalRenderingBeginInfoEXT*   pConditionalRenderingBegin) {
    ValidationStateTracker::PostCallRecordCmdBeginConditionalRenderingEXT(commandBuffer, pConditionalRenderingBegin);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCmdBeginConditionalRenderingEXT(commandBuffer, pConditionalRenderingBegin); });
}

bool BestPractices::PreCallValidateCmdEndConditionalRenderingEXT(
    VkCommandBuffer                             commandBuffer) const {
    bool skip = ValidationStateTracker::PreCallValidateCmdEndConditionalRenderingEXT(commandBuffer);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCmdEndConditionalRenderingEXT(commandBuffer); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCmdEndConditionalRenderingEXT(
    VkCommandBuffer                             commandBuffer) {
    ValidationStateTracker::PreCallRecordCmdEndConditionalRenderingEXT(commandBuffer);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCmdEndConditionalRenderingEXT(commandBuffer); });
}

void BestPractices::PostCallRecordCmdEndConditionalRenderingEXT(
    VkCommandBuffer                             commandBuffer) {
    ValidationStateTracker::PostCallRecordCmdEndConditionalRenderingEXT(commandBuffer);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCmdEndConditionalRenderingEXT(commandBuffer); });
}

bool BestPractices::PreCallValidateCmdSetViewportWScalingNV(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstViewport,
    uint32_t                                    viewportCount,
    const VkViewportWScalingNV*                 pViewportWScalings) const {
    bool skip = ValidationStateTracker::PreCallValidateCmdSetViewportWScalingNV(commandBuffer, firstViewport, viewportCount, pViewportWScalings);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCmdSetViewportWScalingNV(commandBuffer, firstViewport, viewportCount, pViewportWScalings); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCmdSetViewportWScalingNV(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstViewport,
    uint32_t                                    viewportCount,
    const VkViewportWScalingNV*                 pViewportWScalings) {
    ValidationStateTracker::PreCallRecordCmdSetViewportWScalingNV(commandBuffer, firstViewport, viewportCount, pViewportWScalings);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCmdSetViewportWScalingNV(commandBuffer, firstViewport, viewportCount, pViewportWScalings); });
}

void BestPractices::PostCallRecordCmdSetViewportWScalingNV(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstViewport,
    uint32_t                                    viewportCount,
    const VkViewportWScalingNV*                 pViewportWScalings) {
    ValidationStateTracker::PostCallRecordCmdSetViewportWScalingNV(commandBuffer, firstViewport, viewportCount, pViewportWScalings);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCmdSetViewportWScalingNV(commandBuffer, firstViewport, viewportCount, pViewportWScalings); });
}

bool BestPractices::PreCallValidateReleaseDisplayEXT(
    VkPhysicalDevice                            physicalDevice,
    VkDisplayKHR                                display) const {
    bool skip = ValidationStateTracker::PreCallValidateReleaseDisplayEXT(physicalDevice, display);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateReleaseDisplayEXT(physicalDevice, display); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordReleaseDisplayEXT(
    VkPhysicalDevice                            physicalDevice,
    VkDisplayKHR                                display) {
    ValidationStateTracker::PreCallRecordReleaseDisplayEXT(physicalDevice, display);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordReleaseDisplayEXT(physicalDevice, display); });
}

void BestPractices::PostCallRecordReleaseDisplayEXT(
    VkPhysicalDevice                            physicalDevice,
    VkDisplayKHR                                display,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordReleaseDisplayEXT(physicalDevice, display, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordReleaseDisplayEXT(physicalDevice, display, result); });
}

bool BestPractices::PreCallValidateAcquireXlibDisplayEXT(
    VkPhysicalDevice                            physicalDevice,
    Display*                                    dpy,
    VkDisplayKHR                                display) const {
    bool skip = ValidationStateTracker::PreCallValidateAcquireXlibDisplayEXT(physicalDevice, dpy, display);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateAcquireXlibDisplayEXT(physicalDevice, dpy, display); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordAcquireXlibDisplayEXT(
    VkPhysicalDevice                            physicalDevice,
    Display*                                    dpy,
    VkDisplayKHR                                display) {
    ValidationStateTracker::PreCallRecordAcquireXlibDisplayEXT(physicalDevice, dpy, display);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordAcquireXlibDisplayEXT(physicalDevice, dpy, display); });
}

void BestPractices::PostCallRecordAcquireXlibDisplayEXT(
    VkPhysicalDevice                            physicalDevice,
    Display*                                    dpy,
    VkDisplayKHR                                display,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordAcquireXlibDisplayEXT(physicalDevice, dpy, display, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordAcquireXlibDisplayEXT(physicalDevice, dpy, display, result); });
}

bool BestPractices::PreCallValidateGetRandROutputDisplayEXT(
    VkPhysicalDevice                            physicalDevice,
    Display*                                    dpy,
    RROutput                                    rrOutput,
    VkDisplayKHR*                               pDisplay) const {
    bool skip = ValidationStateTracker::PreCallValidateGetRandROutputDisplayEXT(physicalDevice, dpy, rrOutput, pDisplay);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetRandROutputDisplayEXT(physicalDevice, dpy, rrOutput, pDisplay); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetRandROutputDisplayEXT(
    VkPhysicalDevice                            physicalDevice,
    Display*                                    dpy,
    RROutput                                    rrOutput,
    VkDisplayKHR*                               pDisplay) {
    ValidationStateTracker::PreCallRecordGetRandROutputDisplayEXT(physicalDevice, dpy, rrOutput, pDisplay);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetRandROutputDisplayEXT(physicalDevice, dpy, rrOutput, pDisplay); });
}

void BestPractices::PostCallRecordGetRandROutputDisplayEXT(
    VkPhysicalDevice                            physicalDevice,
    Display*                                    dpy,
    RROutput                                    rrOutput,
    VkDisplayKHR*                               pDisplay,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetRandROutputDisplayEXT(physicalDevice, dpy, rrOutput, pDisplay, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetRandROutputDisplayEXT(physicalDevice, dpy, rrOutput, pDisplay, result); });
}

bool BestPractices::PreCallValidateGetPhysicalDeviceSurfaceCapabilities2EXT(
    VkPhysicalDevice                            physicalDevice,
    VkSurfaceKHR                                surface,
    VkSurfaceCapabilities2EXT*                  pSurfaceCapabilities) const {
    bool skip = ValidationStateTracker::PreCallValidateGetPhysicalDeviceSurfaceCapabilities2EXT(physicalDevice, surface, pSurfaceCapabilities);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetPhysicalDeviceSurfaceCapabilities2EXT(physicalDevice, surface, pSurfaceCapabilities); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetPhysicalDeviceSurfaceCapabilities2EXT(
    VkPhysicalDevice                            physicalDevice,
    VkSurfaceKHR                                surface,
    VkSurfaceCapabilities2EXT*                  pSurfaceCapabilities) {
    ValidationStateTracker::PreCallRecordGetPhysicalDeviceSurfaceCapabilities2EXT(physicalDevice, surface, pSurfaceCapabilities);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetPhysicalDeviceSurfaceCapabilities2EXT(physicalDevice, surface, pSurfaceCapabilities); });
}

void BestPractices::PostCallRecordGetPhysicalDeviceSurfaceCapabilities2EXT(
    VkPhysicalDevice                            physicalDevice,
    VkSurfaceKHR                                surface,
    VkSurfaceCapabilities2EXT*                  pSurfaceCapabilities,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceSurfaceCapabilities2EXT(physicalDevice, surface, pSurfaceCapabilities, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetPhysicalDeviceSurfaceCapabilities2EXT(physicalDevice, surface, pSurfaceCapabilities, result); });
}

bool BestPractices::PreCallValidateDisplayPowerControlEXT(
    VkDevice                                    device,
    VkDisplayKHR                                display,
    const VkDisplayPowerInfoEXT*                pDisplayPowerInfo) const {
    bool skip = ValidationStateTracker::PreCallValidateDisplayPowerControlEXT(device, display, pDisplayPowerInfo);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateDisplayPowerControlEXT(device, display, pDisplayPowerInfo); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordDisplayPowerControlEXT(
    VkDevice                                    device,
    VkDisplayKHR                                display,
    const VkDisplayPowerInfoEXT*                pDisplayPowerInfo) {
    ValidationStateTracker::PreCallRecordDisplayPowerControlEXT(device, display, pDisplayPowerInfo);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordDisplayPowerControlEXT(device, display, pDisplayPowerInfo); });
}

void BestPractices::PostCallRecordDisplayPowerControlEXT(
    VkDevice                                    device,
    VkDisplayKHR                                display,
    const VkDisplayPowerInfoEXT*                pDisplayPowerInfo,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordDisplayPowerControlEXT(device, display, pDisplayPowerInfo, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordDisplayPowerControlEXT(device, display, pDisplayPowerInfo, result); });
}

bool BestPractices::PreCallValidateRegisterDeviceEventEXT(
    VkDevice                                    device,
    const VkDeviceEventInfoEXT*                 pDeviceEventInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkFence*                                    pFence) const {
    bool skip = ValidationStateTracker::PreCallValidateRegisterDeviceEventEXT(device, pDeviceEventInfo, pAllocator, pFence);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateRegisterDeviceEventEXT(device, pDeviceEventInfo, pAllocator, pFence); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordRegisterDeviceEventEXT(
    VkDevice                                    device,
    const VkDeviceEventInfoEXT*                 pDeviceEventInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkFence*                                    pFence) {
    ValidationStateTracker::PreCallRecordRegisterDeviceEventEXT(device, pDeviceEventInfo, pAllocator, pFence);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordRegisterDeviceEventEXT(device, pDeviceEventInfo, pAllocator, pFence); });
}

void BestPractices::PostCallRecordRegisterDeviceEventEXT(
    VkDevice                                    device,
    const VkDeviceEventInfoEXT*                 pDeviceEventInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkFence*                                    pFence,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordRegisterDeviceEventEXT(device, pDeviceEventInfo, pAllocator, pFence, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordRegisterDeviceEventEXT(device, pDeviceEventInfo, pAllocator, pFence, result); });
}

bool BestPractices::PreCallValidateRegisterDisplayEventEXT(
    VkDevice                                    device,
    VkDisplayKHR                                display,
    const VkDisplayEventInfoEXT*                pDisplayEventInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkFence*                                    pFence) const {
    bool skip = ValidationStateTracker::PreCallValidateRegisterDisplayEventEXT(device, display, pDisplayEventInfo, pAllocator, pFence);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateRegisterDisplayEventEXT(device, display, pDisplayEventInfo, pAllocator, pFence); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordRegisterDisplayEventEXT(
    VkDevice                                    device,
    VkDisplayKHR                                display,
    const VkDisplayEventInfoEXT*                pDisplayEventInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkFence*                                    pFence) {
    ValidationStateTracker::PreCallRecordRegisterDisplayEventEXT(device, display, pDisplayEventInfo, pAllocator, pFence);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordRegisterDisplayEventEXT(device, display, pDisplayEventInfo, pAllocator, pFence); });
}

void BestPractices::PostCallRecordRegisterDisplayEventEXT(
    VkDevice                                    device,
    VkDisplayKHR                                display,
    const VkDisplayEventInfoEXT*                pDisplayEventInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkFence*                                    pFence,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordRegisterDisplayEventEXT(device, display, pDisplayEventInfo, pAllocator, pFence, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordRegisterDisplayEventEXT(device, display, pDisplayEventInfo, pAllocator, pFence, result); });
}

bool BestPractices::PreCallValidateGetSwapchainCounterEXT(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    VkSurfaceCounterFlagBitsEXT                 counter,
    uint64_t*                                   pCounterValue) const {
    bool skip = ValidationStateTracker::PreCallValidateGetSwapchainCounterEXT(device, swapchain, counter, pCounterValue);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetSwapchainCounterEXT(device, swapchain, counter, pCounterValue); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetSwapchainCounterEXT(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    VkSurfaceCounterFlagBitsEXT                 counter,
    uint64_t*                                   pCounterValue) {
    ValidationStateTracker::PreCallRecordGetSwapchainCounterEXT(device, swapchain, counter, pCounterValue);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetSwapchainCounterEXT(device, swapchain, counter, pCounterValue); });
}

void BestPractices::PostCallRecordGetSwapchainCounterEXT(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    VkSurfaceCounterFlagBitsEXT                 counter,
    uint64_t*                                   pCounterValue,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetSwapchainCounterEXT(device, swapchain, counter, pCounterValue, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetSwapchainCounterEXT(device, swapchain, counter, pCounterValue, result); });
}

bool BestPractices::PreCallValidateGetRefreshCycleDurationGOOGLE(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    VkRefreshCycleDurationGOOGLE*               pDisplayTimingProperties) const {
    bool skip = ValidationStateTracker::PreCallValidateGetRefreshCycleDurationGOOGLE(device, swapchain, pDisplayTimingProperties);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetRefreshCycleDurationGOOGLE(device, swapchain, pDisplayTimingProperties); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetRefreshCycleDurationGOOGLE(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    VkRefreshCycleDurationGOOGLE*               pDisplayTimingProperties) {
    ValidationStateTracker::PreCallRecordGetRefreshCycleDurationGOOGLE(device, swapchain, pDisplayTimingProperties);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetRefreshCycleDurationGOOGLE(device, swapchain, pDisplayTimingProperties); });
}

void BestPractices::PostCallRecordGetRefreshCycleDurationGOOGLE(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    VkRefreshCycleDurationGOOGLE*               pDisplayTimingProperties,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetRefreshCycleDurationGOOGLE(device, swapchain, pDisplayTimingProperties, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetRefreshCycleDurationGOOGLE(device, swapchain, pDisplayTimingProperties, result); });
}

bool BestPractices::PreCallValidateGetPastPresentationTimingGOOGLE(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    uint32_t*                                   pPresentationTimingCount,
    VkPastPresentationTimingGOOGLE*             pPresentationTimings) const {
    bool skip = ValidationStateTracker::PreCallValidateGetPastPresentationTimingGOOGLE(device, swapchain, pPresentationTimingCount, pPresentationTimings);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetPastPresentationTimingGOOGLE(device, swapchain, pPresentationTimingCount, pPresentationTimings); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetPastPresentationTimingGOOGLE(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    uint32_t*                                   pPresentationTimingCount,
    VkPastPresentationTimingGOOGLE*             pPresentationTimings) {
    ValidationStateTracker::PreCallRecordGetPastPresentationTimingGOOGLE(device, swapchain, pPresentationTimingCount, pPresentationTimings);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetPastPresentationTimingGOOGLE(device, swapchain, pPresentationTimingCount, pPresentationTimings); });
}

void BestPractices::PostCallRecordGetPastPresentationTimingGOOGLE(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    uint32_t*                                   pPresentationTimingCount,
    VkPastPresentationTimingGOOGLE*             pPresentationTimings,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetPastPresentationTimingGOOGLE(device, swapchain, pPresentationTimingCount, pPresentationTimings, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetPastPresentationTimingGOOGLE(device, swapchain, pPresentationTimingCount, pPresentationTimings, result); });
}

bool BestPractices::PreCallValidateCmdSetDiscardRectangleEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstDiscardRectangle,
    uint32_t                                    discardRectangleCount,
    const VkRect2D*                             pDiscardRectangles) const {
    bool skip = ValidationStateTracker::PreCallValidateCmdSetDiscardRectangleEXT(commandBuffer, firstDiscardRectangle, discardRectangleCount, pDiscardRectangles);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCmdSetDiscardRectangleEXT(commandBuffer, firstDiscardRectangle, discardRectangleCount, pDiscardRectangles); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCmdSetDiscardRectangleEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstDiscardRectangle,
    uint32_t                                    discardRectangleCount,
    const VkRect2D*                             pDiscardRectangles) {
    ValidationStateTracker::PreCallRecordCmdSetDiscardRectangleEXT(commandBuffer, firstDiscardRectangle, discardRectangleCount, pDiscardRectangles);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCmdSetDiscardRectangleEXT(commandBuffer, firstDiscardRectangle, discardRectangleCount, pDiscardRectangles); });
}

void BestPractices::PostCallRecordCmdSetDiscardRectangleEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstDiscardRectangle,
    uint32_t                                    discardRectangleCount,
    const VkRect2D*                             pDiscardRectangles) {
    ValidationStateTracker::PostCallRecordCmdSetDiscardRectangleEXT(commandBuffer, firstDiscardRectangle, discardRectangleCount, pDiscardRectangles);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCmdSetDiscardRectangleEXT(commandBuffer, firstDiscardRectangle, discardRectangleCount, pDiscardRectangles); });
}

bool BestPractices::PreCallValidateSetHdrMetadataEXT(
    VkDevice                                    device,
    uint32_t                                    swapchainCount,
    const VkSwapchainKHR*                       pSwapchains,
    const VkHdrMetadataEXT*                     pMetadata) const {
    bool skip = ValidationStateTracker::PreCallValidateSetHdrMetadataEXT(device, swapchainCount, pSwapchains, pMetadata);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateSetHdrMetadataEXT(device, swapchainCount, pSwapchains, pMetadata); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordSetHdrMetadataEXT(
    VkDevice                                    device,
    uint32_t                                    swapchainCount,
    const VkSwapchainKHR*                       pSwapchains,
    const VkHdrMetadataEXT*                     pMetadata) {
    ValidationStateTracker::PreCallRecordSetHdrMetadataEXT(device, swapchainCount, pSwapchains, pMetadata);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordSetHdrMetadataEXT(device, swapchainCount, pSwapchains, pMetadata); });
}

void BestPractices::PostCallRecordSetHdrMetadataEXT(
    VkDevice                                    device,
    uint32_t                                    swapchainCount,
    const VkSwapchainKHR*                       pSwapchains,
    const VkHdrMetadataEXT*                     pMetadata) {
    ValidationStateTracker::PostCallRecordSetHdrMetadataEXT(device, swapchainCount, pSwapchains, pMetadata);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordSetHdrMetadataEXT(device, swapchainCount, pSwapchains, pMetadata); });
}

bool BestPractices::PreCallValidateCreateIOSSurfaceMVK(
    VkInstance                                  instance,
    const VkIOSSurfaceCreateInfoMVK*            pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) const {
    bool skip = ValidationStateTracker::PreCallValidateCreateIOSSurfaceMVK(instance, pCreateInfo, pAllocator, pSurface);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCreateIOSSurfaceMVK(instance, pCreateInfo, pAllocator, pSurface); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCreateIOSSurfaceMVK(
    VkInstance                                  instance,
    const VkIOSSurfaceCreateInfoMVK*            pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) {
    ValidationStateTracker::PreCallRecordCreateIOSSurfaceMVK(instance, pCreateInfo, pAllocator, pSurface);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCreateIOSSurfaceMVK(instance, pCreateInfo, pAllocator, pSurface); });
}

void BestPractices::PostCallRecordCreateIOSSurfaceMVK(
    VkInstance                                  instance,
    const VkIOSSurfaceCreateInfoMVK*            pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCreateIOSSurfaceMVK(instance, pCreateInfo, pAllocator, pSurface, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCreateIOSSurfaceMVK(instance, pCreateInfo, pAllocator, pSurface, result); });
}

bool BestPractices::PreCallValidateCreateMacOSSurfaceMVK(
    VkInstance                                  instance,
    const VkMacOSSurfaceCreateInfoMVK*          pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) const {
    bool skip = ValidationStateTracker::PreCallValidateCreateMacOSSurfaceMVK(instance, pCreateInfo, pAllocator, pSurface);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCreateMacOSSurfaceMVK(instance, pCreateInfo, pAllocator, pSurface); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCreateMacOSSurfaceMVK(
    VkInstance                                  instance,
    const VkMacOSSurfaceCreateInfoMVK*          pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) {
    ValidationStateTracker::PreCallRecordCreateMacOSSurfaceMVK(instance, pCreateInfo, pAllocator, pSurface);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCreateMacOSSurfaceMVK(instance, pCreateInfo, pAllocator, pSurface); });
}

void BestPractices::PostCallRecordCreateMacOSSurfaceMVK(
    VkInstance                                  instance,
    const VkMacOSSurfaceCreateInfoMVK*          pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCreateMacOSSurfaceMVK(instance, pCreateInfo, pAllocator, pSurface, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCreateMacOSSurfaceMVK(instance, pCreateInfo, pAllocator, pSurface, result); });
}

bool BestPractices::PreCallValidateSetDebugUtilsObjectNameEXT(
    VkDevice                                    device,
    const VkDebugUtilsObjectNameInfoEXT*        pNameInfo) const {
    bool skip = ValidationStateTracker::PreCallValidateSetDebugUtilsObjectNameEXT(device, pNameInfo);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateSetDebugUtilsObjectNameEXT(device, pNameInfo); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordSetDebugUtilsObjectNameEXT(
    VkDevice                                    device,
    const VkDebugUtilsObjectNameInfoEXT*        pNameInfo) {
    ValidationStateTracker::PreCallRecordSetDebugUtilsObjectNameEXT(device, pNameInfo);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordSetDebugUtilsObjectNameEXT(device, pNameInfo); });
}

void BestPractices::PostCallRecordSetDebugUtilsObjectNameEXT(
    VkDevice                                    device,
    const VkDebugUtilsObjectNameInfoEXT*        pNameInfo,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordSetDebugUtilsObjectNameEXT(device, pNameInfo, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordSetDebugUtilsObjectNameEXT(device, pNameInfo, result); });
}

bool BestPractices::PreCallValidateSetDebugUtilsObjectTagEXT(
    VkDevice                                    device,
    const VkDebugUtilsObjectTagInfoEXT*         pTagInfo) const {
    bool skip = ValidationStateTracker::PreCallValidateSetDebugUtilsObjectTagEXT(device, pTagInfo);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateSetDebugUtilsObjectTagEXT(device, pTagInfo); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordSetDebugUtilsObjectTagEXT(
    VkDevice                                    device,
    const VkDebugUtilsObjectTagInfoEXT*         pTagInfo) {
    ValidationStateTracker::PreCallRecordSetDebugUtilsObjectTagEXT(device, pTagInfo);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordSetDebugUtilsObjectTagEXT(device, pTagInfo); });
}

void BestPractices::PostCallRecordSetDebugUtilsObjectTagEXT(
    VkDevice                                    device,
    const VkDebugUtilsObjectTagInfoEXT*         pTagInfo,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordSetDebugUtilsObjectTagEXT(device, pTagInfo, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordSetDebugUtilsObjectTagEXT(device, pTagInfo, result); });
}

bool BestPractices::PreCallValidateQueueBeginDebugUtilsLabelEXT(
    VkQueue                                     queue,
    const VkDebugUtilsLabelEXT*                 pLabelInfo) const {
    bool skip = ValidationStateTracker::PreCallValidateQueueBeginDebugUtilsLabelEXT(queue, pLabelInfo);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateQueueBeginDebugUtilsLabelEXT(queue, pLabelInfo); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordQueueBeginDebugUtilsLabelEXT(
    VkQueue                                     queue,
    const VkDebugUtilsLabelEXT*                 pLabelInfo) {
    ValidationStateTracker::PreCallRecordQueueBeginDebugUtilsLabelEXT(queue, pLabelInfo);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordQueueBeginDebugUtilsLabelEXT(queue, pLabelInfo); });
}

void BestPractices::PostCallRecordQueueBeginDebugUtilsLabelEXT(
    VkQueue                                     queue,
    const VkDebugUtilsLabelEXT*                 pLabelInfo) {
    ValidationStateTracker::PostCallRecordQueueBeginDebugUtilsLabelEXT(queue, pLabelInfo);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordQueueBeginDebugUtilsLabelEXT(queue, pLabelInfo); });
}

bool BestPractices::PreCallValidateQueueEndDebugUtilsLabelEXT(
    VkQueue                                     queue) const {
    bool skip = ValidationStateTracker::PreCallValidateQueueEndDebugUtilsLabelEXT(queue);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateQueueEndDebugUtilsLabelEXT(queue); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordQueueEndDebugUtilsLabelEXT(
    VkQueue                                     queue) {
    ValidationStateTracker::PreCallRecordQueueEndDebugUtilsLabelEXT(queue);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordQueueEndDebugUtilsLabelEXT(queue); });
}

void BestPractices::PostCallRecordQueueEndDebugUtilsLabelEXT(
    VkQueue                                     queue) {
    ValidationStateTracker::PostCallRecordQueueEndDebugUtilsLabelEXT(queue);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordQueueEndDebugUtilsLabelEXT(queue); });
}

bool BestPractices::PreCallValidateQueueInsertDebugUtilsLabelEXT(
    VkQueue                                     queue,
    const VkDebugUtilsLabelEXT*                 pLabelInfo) const {
    bool skip = ValidationStateTracker::PreCallValidateQueueInsertDebugUtilsLabelEXT(queue, pLabelInfo);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateQueueInsertDebugUtilsLabelEXT(queue, pLabelInfo); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordQueueInsertDebugUtilsLabelEXT(
    VkQueue                                     queue,
    const VkDebugUtilsLabelEXT*                 pLabelInfo) {
    ValidationStateTracker::PreCallRecordQueueInsertDebugUtilsLabelEXT(queue, pLabelInfo);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordQueueInsertDebugUtilsLabelEXT(queue, pLabelInfo); });
}

void BestPractices::PostCallRecordQueueInsertDebugUtilsLabelEXT(
    VkQueue                                     queue,
    const VkDebugUtilsLabelEXT*                 pLabelInfo) {
    ValidationStateTracker::PostCallRecordQueueInsertDebugUtilsLabelEXT(queue, pLabelInfo);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordQueueInsertDebugUtilsLabelEXT(queue, pLabelInfo); });
}

bool BestPractices::PreCallValidateCmdBeginDebugUtilsLabelEXT(
    VkCommandBuffer                             commandBuffer,
    const VkDebugUtilsLabelEXT*                 pLabelInfo) const {
    bool skip = ValidationStateTracker::PreCallValidateCmdBeginDebugUtilsLabelEXT(commandBuffer, pLabelInfo);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCmdBeginDebugUtilsLabelEXT(commandBuffer, pLabelInfo); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCmdBeginDebugUtilsLabelEXT(
    VkCommandBuffer                             commandBuffer,
    const VkDebugUtilsLabelEXT*                 pLabelInfo) {
    ValidationStateTracker::PreCallRecordCmdBeginDebugUtilsLabelEXT(commandBuffer, pLabelInfo);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCmdBeginDebugUtilsLabelEXT(commandBuffer, pLabelInfo); });
}

void BestPractices::PostCallRecordCmdBeginDebugUtilsLabelEXT(
    VkCommandBuffer                             commandBuffer,
    const VkDebugUtilsLabelEXT*                 pLabelInfo) {
    ValidationStateTracker::PostCallRecordCmdBeginDebugUtilsLabelEXT(commandBuffer, pLabelInfo);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCmdBeginDebugUtilsLabelEXT(commandBuffer, pLabelInfo); });
}

bool BestPractices::PreCallValidateCmdEndDebugUtilsLabelEXT(
    VkCommandBuffer                             commandBuffer) const {
    bool skip = ValidationStateTracker::PreCallValidateCmdEndDebugUtilsLabelEXT(commandBuffer);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCmdEndDebugUtilsLabelEXT(commandBuffer); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCmdEndDebugUtilsLabelEXT(
    VkCommandBuffer                             commandBuffer) {
    ValidationStateTracker::PreCallRecordCmdEndDebugUtilsLabelEXT(commandBuffer);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCmdEndDebugUtilsLabelEXT(commandBuffer); });
}

void BestPractices::PostCallRecordCmdEndDebugUtilsLabelEXT(
    VkCommandBuffer                             commandBuffer) {
    ValidationStateTracker::PostCallRecordCmdEndDebugUtilsLabelEXT(commandBuffer);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCmdEndDebugUtilsLabelEXT(commandBuffer); });
}

bool BestPractices::PreCallValidateCmdInsertDebugUtilsLabelEXT(
    VkCommandBuffer                             commandBuffer,
    const VkDebugUtilsLabelEXT*                 pLabelInfo) const {
    bool skip = ValidationStateTracker::PreCallValidateCmdInsertDebugUtilsLabelEXT(commandBuffer, pLabelInfo);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCmdInsertDebugUtilsLabelEXT(commandBuffer, pLabelInfo); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCmdInsertDebugUtilsLabelEXT(
    VkCommandBuffer                             commandBuffer,
    const VkDebugUtilsLabelEXT*                 pLabelInfo) {
    ValidationStateTracker::PreCallRecordCmdInsertDebugUtilsLabelEXT(commandBuffer, pLabelInfo);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCmdInsertDebugUtilsLabelEXT(commandBuffer, pLabelInfo); });
}

void BestPractices::PostCallRecordCmdInsertDebugUtilsLabelEXT(
    VkCommandBuffer                             commandBuffer,
    const VkDebugUtilsLabelEXT*                 pLabelInfo) {
    ValidationStateTracker::PostCallRecordCmdInsertDebugUtilsLabelEXT(commandBuffer, pLabelInfo);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCmdInsertDebugUtilsLabelEXT(commandBuffer, pLabelInfo); });
}

bool BestPractices::PreCallValidateCreateDebugUtilsMessengerEXT(
    VkInstance                                  instance,
    const VkDebugUtilsMessengerCreateInfoEXT*   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDebugUtilsMessengerEXT*                   pMessenger) const {
    bool skip = ValidationStateTracker::PreCallValidateCreateDebugUtilsMessengerEXT(instance, pCreateInfo, pAllocator, pMessenger);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCreateDebugUtilsMessengerEXT(instance, pCreateInfo, pAllocator, pMessenger); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCreateDebugUtilsMessengerEXT(
    VkInstance                                  instance,
    const VkDebugUtilsMessengerCreateInfoEXT*   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDebugUtilsMessengerEXT*                   pMessenger) {
    ValidationStateTracker::PreCallRecordCreateDebugUtilsMessengerEXT(instance, pCreateInfo, pAllocator, pMessenger);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCreateDebugUtilsMessengerEXT(instance, pCreateInfo, pAllocator, pMessenger); });
}

void BestPractices::PostCallRecordCreateDebugUtilsMessengerEXT(
    VkInstance                                  instance,
    const VkDebugUtilsMessengerCreateInfoEXT*   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDebugUtilsMessengerEXT*                   pMessenger,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCreateDebugUtilsMessengerEXT(instance, pCreateInfo, pAllocator, pMessenger, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCreateDebugUtilsMessengerEXT(instance, pCreateInfo, pAllocator, pMessenger, result); });
}

bool BestPractices::PreCallValidateDestroyDebugUtilsMessengerEXT(
    VkInstance                                  instance,
    VkDebugUtilsMessengerEXT                    messenger,
    const VkAllocationCallbacks*                pAllocator) const {
    bool skip = ValidationStateTracker::PreCallValidateDestroyDebugUtilsMessengerEXT(instance, messenger, pAllocator);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateDestroyDebugUtilsMessengerEXT(instance, messenger, pAllocator); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordDestroyDebugUtilsMessengerEXT(
    VkInstance                                  instance,
    VkDebugUtilsMessengerEXT                    messenger,
    const VkAllocationCallbacks*                pAllocator) {
    ValidationStateTracker::PreCallRecordDestroyDebugUtilsMessengerEXT(instance, messenger, pAllocator);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordDestroyDebugUtilsMessengerEXT(instance, messenger, pAllocator); });
}

void BestPractices::PostCallRecordDestroyDebugUtilsMessengerEXT(
    VkInstance                                  instance,
    VkDebugUtilsMessengerEXT                    messenger,
    const VkAllocationCallbacks*                pAllocator) {
    ValidationStateTracker::PostCallRecordDestroyDebugUtilsMessengerEXT(instance, messenger, pAllocator);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordDestroyDebugUtilsMessengerEXT(instance, messenger, pAllocator); });
}

bool BestPractices::PreCallValidateSubmitDebugUtilsMessageEXT(
    VkInstance                                  instance,
    VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT             messageTypes,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData) const {
    bool skip = ValidationStateTracker::PreCallValidateSubmitDebugUtilsMessageEXT(instance, messageSeverity, messageTypes, pCallbackData);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateSubmitDebugUtilsMessageEXT(instance, messageSeverity, messageTypes, pCallbackData); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordSubmitDebugUtilsMessageEXT(
    VkInstance                                  instance,
    VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT             messageTypes,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData) {
    ValidationStateTracker::PreCallRecordSubmitDebugUtilsMessageEXT(instance, messageSeverity, messageTypes, pCallbackData);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordSubmitDebugUtilsMessageEXT(instance, messageSeverity, messageTypes, pCallbackData); });
}

void BestPractices::PostCallRecordSubmitDebugUtilsMessageEXT(
    VkInstance                                  instance,
    VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT             messageTypes,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData) {
    ValidationStateTracker::PostCallRecordSubmitDebugUtilsMessageEXT(instance, messageSeverity, messageTypes, pCallbackData);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordSubmitDebugUtilsMessageEXT(instance, messageSeverity, messageTypes, pCallbackData); });
}

bool BestPractices::PreCallValidateGetAndroidHardwareBufferPropertiesANDROID(
    VkDevice                                    device,
    const struct AHardwareBuffer*               buffer,
    VkAndroidHardwareBufferPropertiesANDROID*   pProperties) const {
    bool skip = ValidationStateTracker::PreCallValidateGetAndroidHardwareBufferPropertiesANDROID(device, buffer, pProperties);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetAndroidHardwareBufferPropertiesANDROID(device, buffer, pProperties); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetAndroidHardwareBufferPropertiesANDROID(
    VkDevice                                    device,
    const struct AHardwareBuffer*               buffer,
    VkAndroidHardwareBufferPropertiesANDROID*   pProperties) {
    ValidationStateTracker::PreCallRecordGetAndroidHardwareBufferPropertiesANDROID(device, buffer, pProperties);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetAndroidHardwareBufferPropertiesANDROID(device, buffer, pProperties); });
}

void BestPractices::PostCallRecordGetAndroidHardwareBufferPropertiesANDROID(
    VkDevice                                    device,
    const struct AHardwareBuffer*               buffer,
    VkAndroidHardwareBufferPropertiesANDROID*   pProperties,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetAndroidHardwareBufferPropertiesANDROID(device, buffer, pProperties, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetAndroidHardwareBufferPropertiesANDROID(device, buffer, pProperties, result); });
}

bool BestPractices::PreCallValidateGetMemoryAndroidHardwareBufferANDROID(
    VkDevice                                    device,
    const VkMemoryGetAndroidHardwareBufferInfoANDROID* pInfo,
    struct AHardwareBuffer**                    pBuffer) const {
    bool skip = ValidationStateTracker::PreCallValidateGetMemoryAndroidHardwareBufferANDROID(device, pInfo, pBuffer);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetMemoryAndroidHardwareBufferANDROID(device, pInfo, pBuffer); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetMemoryAndroidHardwareBufferANDROID(
    VkDevice                                    device,
    const VkMemoryGetAndroidHardwareBufferInfoANDROID* pInfo,
    struct AHardwareBuffer**                    pBuffer) {
    ValidationStateTracker::PreCallRecordGetMemoryAndroidHardwareBufferANDROID(device, pInfo, pBuffer);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetMemoryAndroidHardwareBufferANDROID(device, pInfo, pBuffer); });
}

void BestPractices::PostCallRecordGetMemoryAndroidHardwareBufferANDROID(
    VkDevice                                    device,
    const VkMemoryGetAndroidHardwareBufferInfoANDROID* pInfo,
    struct AHardwareBuffer**                    pBuffer,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetMemoryAndroidHardwareBufferANDROID(device, pInfo, pBuffer, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetMemoryAndroidHardwareBufferANDROID(device, pInfo, pBuffer, result); });
}

bool BestPractices::PreCallValidateCmdSetSampleLocationsEXT(
    VkCommandBuffer                             commandBuffer,
    const VkSampleLocationsInfoEXT*             pSampleLocationsInfo) const {
    bool skip = ValidationStateTracker::PreCallValidateCmdSetSampleLocationsEXT(commandBuffer, pSampleLocationsInfo);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCmdSetSampleLocationsEXT(commandBuffer, pSampleLocationsInfo); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCmdSetSampleLocationsEXT(
    VkCommandBuffer                             commandBuffer,
    const VkSampleLocationsInfoEXT*             pSampleLocationsInfo) {
    ValidationStateTracker::PreCallRecordCmdSetSampleLocationsEXT(commandBuffer, pSampleLocationsInfo);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCmdSetSampleLocationsEXT(commandBuffer, pSampleLocationsInfo); });
}

void BestPractices::PostCallRecordCmdSetSampleLocationsEXT(
    VkCommandBuffer                             commandBuffer,
    const VkSampleLocationsInfoEXT*             pSampleLocationsInfo) {
    ValidationStateTracker::PostCallRecordCmdSetSampleLocationsEXT(commandBuffer, pSampleLocationsInfo);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCmdSetSampleLocationsEXT(commandBuffer, pSampleLocationsInfo); });
}

bool BestPractices::PreCallValidateGetPhysicalDeviceMultisamplePropertiesEXT(
    VkPhysicalDevice                            physicalDevice,
    VkSampleCountFlagBits                       samples,
    VkMultisamplePropertiesEXT*                 pMultisampleProperties) const {
    bool skip = ValidationStateTracker::PreCallValidateGetPhysicalDeviceMultisamplePropertiesEXT(physicalDevice, samples, pMultisampleProperties);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetPhysicalDeviceMultisamplePropertiesEXT(physicalDevice, samples, pMultisampleProperties); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetPhysicalDeviceMultisamplePropertiesEXT(
    VkPhysicalDevice                            physicalDevice,
    VkSampleCountFlagBits                       samples,
    VkMultisamplePropertiesEXT*                 pMultisampleProperties) {
    ValidationStateTracker::PreCallRecordGetPhysicalDeviceMultisamplePropertiesEXT(physicalDevice, samples, pMultisampleProperties);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetPhysicalDeviceMultisamplePropertiesEXT(physicalDevice, samples, pMultisampleProperties); });
}

void BestPractices::PostCallRecordGetPhysicalDeviceMultisamplePropertiesEXT(
    VkPhysicalDevice                            physicalDevice,
    VkSampleCountFlagBits                       samples,
    VkMultisamplePropertiesEXT*                 pMultisampleProperties) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceMultisamplePropertiesEXT(physicalDevice, samples, pMultisampleProperties);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetPhysicalDeviceMultisamplePropertiesEXT(physicalDevice, samples, pMultisampleProperties); });
}

bool BestPractices::PreCallValidateGetImageDrmFormatModifierPropertiesEXT(
    VkDevice                                    device,
    VkImage                                     image,
    VkImageDrmFormatModifierPropertiesEXT*      pProperties) const {
    bool skip = ValidationStateTracker::PreCallValidateGetImageDrmFormatModifierPropertiesEXT(device, image, pProperties);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetImageDrmFormatModifierPropertiesEXT(device, image, pProperties); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetImageDrmFormatModifierPropertiesEXT(
    VkDevice                                    device,
    VkImage                                     image,
    VkImageDrmFormatModifierPropertiesEXT*      pProperties) {
    ValidationStateTracker::PreCallRecordGetImageDrmFormatModifierPropertiesEXT(device, image, pProperties);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetImageDrmFormatModifierPropertiesEXT(device, image, pProperties); });
}

void BestPractices::PostCallRecordGetImageDrmFormatModifierPropertiesEXT(
    VkDevice                                    device,
    VkImage                                     image,
    VkImageDrmFormatModifierPropertiesEXT*      pProperties,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetImageDrmFormatModifierPropertiesEXT(device, image, pProperties, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetImageDrmFormatModifierPropertiesEXT(device, image, pProperties, result); });
}

bool BestPractices::PreCallValidateCreateValidationCacheEXT(
    VkDevice                                    device,
    const VkValidationCacheCreateInfoEXT*       pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkValidationCacheEXT*                       pValidationCache) const {
    bool skip = ValidationStateTracker::PreCallValidateCreateValidationCacheEXT(device, pCreateInfo, pAllocator, pValidationCache);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCreateValidationCacheEXT(device, pCreateInfo, pAllocator, pValidationCache); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCreateValidationCacheEXT(
    VkDevice                                    device,
    const VkValidationCacheCreateInfoEXT*       pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkValidationCacheEXT*                       pValidationCache) {
    ValidationStateTracker::PreCallRecordCreateValidationCacheEXT(device, pCreateInfo, pAllocator, pValidationCache);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCreateValidationCacheEXT(device, pCreateInfo, pAllocator, pValidationCache); });
}

// Skipping vkCreateValidationCacheEXT for autogen as it has a manually created custom function or ignored.

bool BestPractices::PreCallValidateDestroyValidationCacheEXT(
    VkDevice                                    device,
    VkValidationCacheEXT                        validationCache,
    const VkAllocationCallbacks*                pAllocator) const {
    bool skip = ValidationStateTracker::PreCallValidateDestroyValidationCacheEXT(device, validationCache, pAllocator);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateDestroyValidationCacheEXT(device, validationCache, pAllocator); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordDestroyValidationCacheEXT(
    VkDevice                                    device,
    VkValidationCacheEXT                        validationCache,
    const VkAllocationCallbacks*                pAllocator) {
    ValidationStateTracker::PreCallRecordDestroyValidationCacheEXT(device, validationCache, pAllocator);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordDestroyValidationCacheEXT(device, validationCache, pAllocator); });
}

// Skipping vkDestroyValidationCacheEXT for autogen as it has a manually created custom function or ignored.

bool BestPractices::PreCallValidateMergeValidationCachesEXT(
    VkDevice                                    device,
    VkValidationCacheEXT                        dstCache,
    uint32_t                                    srcCacheCount,
    const VkValidationCacheEXT*                 pSrcCaches) const {
    bool skip = ValidationStateTracker::PreCallValidateMergeValidationCachesEXT(device, dstCache, srcCacheCount, pSrcCaches);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateMergeValidationCachesEXT(device, dstCache, srcCacheCount, pSrcCaches); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordMergeValidationCachesEXT(
    VkDevice                                    device,
    VkValidationCacheEXT                        dstCache,
    uint32_t                                    srcCacheCount,
    const VkValidationCacheEXT*                 pSrcCaches) {
    ValidationStateTracker::PreCallRecordMergeValidationCachesEXT(device, dstCache, srcCacheCount, pSrcCaches);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordMergeValidationCachesEXT(device, dstCache, srcCacheCount, pSrcCaches); });
}

// Skipping vkMergeValidationCachesEXT for autogen as it has a manually created custom function or ignored.

bool BestPractices::PreCallValidateGetValidationCacheDataEXT(
    VkDevice                                    device,
    VkValidationCacheEXT                        validationCache,
    size_t*                                     pDataSize,
    void*                                       pData) const {
    bool skip = ValidationStateTracker::PreCallValidateGetValidationCacheDataEXT(device, validationCache, pDataSize, pData);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetValidationCacheDataEXT(device, validationCache, pDataSize, pData); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetValidationCacheDataEXT(
    VkDevice                                    device,
    VkValidationCacheEXT                        validationCache,
    size_t*                                     pDataSize,
    void*                                       pData) {
    ValidationStateTracker::PreCallRecordGetValidationCacheDataEXT(device, validationCache, pDataSize, pData);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetValidationCacheDataEXT(device, validationCache, pDataSize, pData); });
}

// Skipping vkGetValidationCacheDataEXT for autogen as it has a manually created custom function or ignored.

bool BestPractices::PreCallValidateCmdBindShadingRateImageNV(
    VkCommandBuffer                             commandBuffer,
    VkImageView                                 imageView,
    VkImageLayout                               imageLayout) const {
    bool skip = ValidationStateTracker::PreCallValidateCmdBindShadingRateImageNV(commandBuffer, imageView, imageLayout);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCmdBindShadingRateImageNV(commandBuffer, imageView, imageLayout); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCmdBindShadingRateImageNV(
    VkCommandBuffer                             commandBuffer,
    VkImageView                                 imageView,
    VkImageLayout                               imageLayout) {
    ValidationStateTracker::PreCallRecordCmdBindShadingRateImageNV(commandBuffer, imageView, imageLayout);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCmdBindShadingRateImageNV(commandBuffer, imageView, imageLayout); });
}

void BestPractices::PostCallRecordCmdBindShadingRateImageNV(
    VkCommandBuffer                             commandBuffer,
    VkImageView                                 imageView,
    VkImageLayout                               imageLayout) {
    ValidationStateTracker::PostCallRecordCmdBindShadingRateImageNV(commandBuffer, imageView, imageLayout);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCmdBindShadingRateImageNV(commandBuffer, imageView, imageLayout); });
}

bool BestPractices::PreCallValidateCmdSetViewportShadingRatePaletteNV(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstViewport,
    uint32_t                                    viewportCount,
    const VkShadingRatePaletteNV*               pShadingRatePalettes) const {
    bool skip = ValidationStateTracker::PreCallValidateCmdSetViewportShadingRatePaletteNV(commandBuffer, firstViewport, viewportCount, pShadingRatePalettes);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCmdSetViewportShadingRatePaletteNV(commandBuffer, firstViewport, viewportCount, pShadingRatePalettes); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCmdSetViewportShadingRatePaletteNV(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstViewport,
    uint32_t                                    viewportCount,
    const VkShadingRatePaletteNV*               pShadingRatePalettes) {
    ValidationStateTracker::PreCallRecordCmdSetViewportShadingRatePaletteNV(commandBuffer, firstViewport, viewportCount, pShadingRatePalettes);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCmdSetViewportShadingRatePaletteNV(commandBuffer, firstViewport, viewportCount, pShadingRatePalettes); });
}

void BestPractices::PostCallRecordCmdSetViewportShadingRatePaletteNV(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstViewport,
    uint32_t                                    viewportCount,
    const VkShadingRatePaletteNV*               pShadingRatePalettes) {
    ValidationStateTracker::PostCallRecordCmdSetViewportShadingRatePaletteNV(commandBuffer, firstViewport, viewportCount, pShadingRatePalettes);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCmdSetViewportShadingRatePaletteNV(commandBuffer, firstViewport, viewportCount, pShadingRatePalettes); });
}

bool BestPractices::PreCallValidateCmdSetCoarseSampleOrderNV(
    VkCommandBuffer                             commandBuffer,
    VkCoarseSampleOrderTypeNV                   sampleOrderType,
    uint32_t                                    customSampleOrderCount,
    const VkCoarseSampleOrderCustomNV*          pCustomSampleOrders) const {
    bool skip = ValidationStateTracker::PreCallValidateCmdSetCoarseSampleOrderNV(commandBuffer, sampleOrderType, customSampleOrderCount, pCustomSampleOrders);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCmdSetCoarseSampleOrderNV(commandBuffer, sampleOrderType, customSampleOrderCount, pCustomSampleOrders); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCmdSetCoarseSampleOrderNV(
    VkCommandBuffer                             commandBuffer,
    VkCoarseSampleOrderTypeNV                   sampleOrderType,
    uint32_t                                    customSampleOrderCount,
    const VkCoarseSampleOrderCustomNV*          pCustomSampleOrders) {
    ValidationStateTracker::PreCallRecordCmdSetCoarseSampleOrderNV(commandBuffer, sampleOrderType, customSampleOrderCount, pCustomSampleOrders);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCmdSetCoarseSampleOrderNV(commandBuffer, sampleOrderType, customSampleOrderCount, pCustomSampleOrders); });
}

void BestPractices::PostCallRecordCmdSetCoarseSampleOrderNV(
    VkCommandBuffer                             commandBuffer,
    VkCoarseSampleOrderTypeNV                   sampleOrderType,
    uint32_t                                    customSampleOrderCount,
    const VkCoarseSampleOrderCustomNV*          pCustomSampleOrders) {
    ValidationStateTracker::PostCallRecordCmdSetCoarseSampleOrderNV(commandBuffer, sampleOrderType, customSampleOrderCount, pCustomSampleOrders);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCmdSetCoarseSampleOrderNV(commandBuffer, sampleOrderType, customSampleOrderCount, pCustomSampleOrders); });
}

bool BestPractices::PreCallValidateCreateAccelerationStructureNV(
    VkDevice                                    device,
    const VkAccelerationStructureCreateInfoNV*  pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkAccelerationStructureNV*                  pAccelerationStructure) const {
    bool skip = ValidationStateTracker::PreCallValidateCreateAccelerationStructureNV(device, pCreateInfo, pAllocator, pAccelerationStructure);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCreateAccelerationStructureNV(device, pCreateInfo, pAllocator, pAccelerationStructure); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCreateAccelerationStructureNV(
    VkDevice                                    device,
    const VkAccelerationStructureCreateInfoNV*  pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkAccelerationStructureNV*                  pAccelerationStructure) {
    ValidationStateTracker::PreCallRecordCreateAccelerationStructureNV(device, pCreateInfo, pAllocator, pAccelerationStructure);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCreateAccelerationStructureNV(device, pCreateInfo, pAllocator, pAccelerationStructure); });
}

void BestPractices::PostCallRecordCreateAccelerationStructureNV(
    VkDevice                                    device,
    const VkAccelerationStructureCreateInfoNV*  pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkAccelerationStructureNV*                  pAccelerationStructure,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCreateAccelerationStructureNV(device, pCreateInfo, pAllocator, pAccelerationStructure, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCreateAccelerationStructureNV(device, pCreateInfo, pAllocator, pAccelerationStructure, result); });
}

bool BestPractices::PreCallValidateDestroyAccelerationStructureKHR(
    VkDevice                                    device,
    VkAccelerationStructureKHR                  accelerationStructure,
    const VkAllocationCallbacks*                pAllocator) const {
    bool skip = ValidationStateTracker::PreCallValidateDestroyAccelerationStructureKHR(device, accelerationStructure, pAllocator);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateDestroyAccelerationStructureKHR(device, accelerationStructure, pAllocator); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordDestroyAccelerationStructureKHR(
    VkDevice                                    device,
    VkAccelerationStructureKHR                  accelerationStructure,
    const VkAllocationCallbacks*                pAllocator) {
    ValidationStateTracker::PreCallRecordDestroyAccelerationStructureKHR(device, accelerationStructure, pAllocator);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordDestroyAccelerationStructureKHR(device, accelerationStructure, pAllocator); });
}

void BestPractices::PostCallRecordDestroyAccelerationStructureKHR(
    VkDevice                                    device,
    VkAccelerationStructureKHR                  accelerationStructure,
    const VkAllocationCallbacks*                pAllocator) {
    ValidationStateTracker::PostCallRecordDestroyAccelerationStructureKHR(device, accelerationStructure, pAllocator);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordDestroyAccelerationStructureKHR(device, accelerationStructure, pAllocator); });
}

bool BestPractices::PreCallValidateDestroyAccelerationStructureNV(
    VkDevice                                    device,
    VkAccelerationStructureKHR                  accelerationStructure,
    const VkAllocationCallbacks*                pAllocator) const {
    bool skip = ValidationStateTracker::PreCallValidateDestroyAccelerationStructureNV(device, accelerationStructure, pAllocator);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateDestroyAccelerationStructureNV(device, accelerationStructure, pAllocator); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordDestroyAccelerationStructureNV(
    VkDevice                                    device,
    VkAccelerationStructureKHR                  accelerationStructure,
    const VkAllocationCallbacks*                pAllocator) {
    ValidationStateTracker::PreCallRecordDestroyAccelerationStructureNV(device, accelerationStructure, pAllocator);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordDestroyAccelerationStructureNV(device, accelerationStructure, pAllocator); });
}

void BestPractices::PostCallRecordDestroyAccelerationStructureNV(
    VkDevice                                    device,
    VkAccelerationStructureKHR                  accelerationStructure,
    const VkAllocationCallbacks*                pAllocator) {
    ValidationStateTracker::PostCallRecordDestroyAccelerationStructureNV(device, accelerationStructure, pAllocator);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordDestroyAccelerationStructureNV(device, accelerationStructure, pAllocator); });
}

bool BestPractices::PreCallValidateGetAccelerationStructureMemoryRequirementsNV(
    VkDevice                                    device,
    const VkAccelerationStructureMemoryRequirementsInfoNV* pInfo,
    VkMemoryRequirements2KHR*                   pMemoryRequirements) const {
    bool skip = ValidationStateTracker::PreCallValidateGetAccelerationStructureMemoryRequirementsNV(device, pInfo, pMemoryRequirements);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetAccelerationStructureMemoryRequirementsNV(device, pInfo, pMemoryRequirements); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetAccelerationStructureMemoryRequirementsNV(
    VkDevice                                    device,
    const VkAccelerationStructureMemoryRequirementsInfoNV* pInfo,
    VkMemoryRequirements2KHR*                   pMemoryRequirements) {
    ValidationStateTracker::PreCallRecordGetAccelerationStructureMemoryRequirementsNV(device, pInfo, pMemoryRequirements);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetAccelerationStructureMemoryRequirementsNV(device, pInfo, pMemoryRequirements); });
}

void BestPractices::PostCallRecordGetAccelerationStructureMemoryRequirementsNV(
    VkDevice                                    device,
    const VkAccelerationStructureMemoryRequirementsInfoNV* pInfo,
    VkMemoryRequirements2KHR*                   pMemoryRequirements) {
    ValidationStateTracker::PostCallRecordGetAccelerationStructureMemoryRequirementsNV(device, pInfo, pMemoryRequirements);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetAccelerationStructureMemoryRequirementsNV(device, pInfo, pMemoryRequirements); });
}

bool BestPractices::PreCallValidateBindAccelerationStructureMemoryKHR(
    VkDevice                                    device,
    uint32_t                                    bindInfoCount,
    const VkBindAccelerationStructureMemoryInfoKHR* pBindInfos) const {
    bool skip = ValidationStateTracker::PreCallValidateBindAccelerationStructureMemoryKHR(device, bindInfoCount, pBindInfos);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateBindAccelerationStructureMemoryKHR(device, bindInfoCount, pBindInfos); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordBindAccelerationStructureMemoryKHR(
    VkDevice                                    device,
    uint32_t                                    bindInfoCount,
    const VkBindAccelerationStructureMemoryInfoKHR* pBindInfos) {
    ValidationStateTracker::PreCallRecordBindAccelerationStructureMemoryKHR(device, bindInfoCount, pBindInfos);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordBindAccelerationStructureMemoryKHR(device, bindInfoCount, pBindInfos); });
}

void BestPractices::PostCallRecordBindAccelerationStructureMemoryKHR(
    VkDevice                                    device,
    uint32_t                                    bindInfoCount,
    const VkBindAccelerationStructureMemoryInfoKHR* pBindInfos,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordBindAccelerationStructureMemoryKHR(device, bindInfoCount, pBindInfos, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordBindAccelerationStructureMemoryKHR(device, bindInfoCount, pBindInfos, result); });
}

bool BestPractices::PreCallValidateBindAccelerationStructureMemoryNV(
    VkDevice                                    device,
    uint32_t                                    bindInfoCount,
    const VkBindAccelerationStructureMemoryInfoKHR* pBindInfos) const {
    bool skip = ValidationStateTracker::PreCallValidateBindAccelerationStructureMemoryNV(device, bindInfoCount, pBindInfos);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateBindAccelerationStructureMemoryNV(device, bindInfoCount, pBindInfos); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordBindAccelerationStructureMemoryNV(
    VkDevice                                    device,
    uint32_t                                    bindInfoCount,
    const VkBindAccelerationStructureMemoryInfoKHR* pBindInfos) {
    ValidationStateTracker::PreCallRecordBindAccelerationStructureMemoryNV(device, bindInfoCount, pBindInfos);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordBindAccelerationStructureMemoryNV(device, bindInfoCount, pBindInfos); });
}

void BestPractices::PostCallRecordBindAccelerationStructureMemoryNV(
    VkDevice                                    device,
    uint32_t                                    bindInfoCount,
    const VkBindAccelerationStructureMemoryInfoKHR* pBindInfos,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordBindAccelerationStructureMemoryNV(device, bindInfoCount, pBindInfos, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordBindAccelerationStructureMemoryNV(device, bindInfoCount, pBindInfos, result); });
}

bool BestPractices::PreCallValidateCmdBuildAccelerationStructureNV(
    VkCommandBuffer                             commandBuffer,
    const VkAccelerationStructureInfoNV*        pInfo,
    VkBuffer                                    instanceData,
    VkDeviceSize                                instanceOffset,
    VkBool32                                    update,
    VkAccelerationStructureKHR                  dst,
    VkAccelerationStructureKHR                  src,
    VkBuffer                                    scratch,
    VkDeviceSize                                scratchOffset) const {
    bool skip = ValidationStateTracker::PreCallValidateCmdBuildAccelerationStructureNV(commandBuffer, pInfo, instanceData, instanceOffset, update, dst, src, scratch, scratchOffset);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCmdBuildAccelerationStructureNV(commandBuffer, pInfo, instanceData, instanceOffset, update, dst, src, scratch, scratchOffset); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCmdBuildAccelerationStructureNV(
    VkCommandBuffer                             commandBuffer,
    const VkAccelerationStructureInfoNV*        pInfo,
    VkBuffer                                    instanceData,
    VkDeviceSize                                instanceOffset,
    VkBool32                                    update,
    VkAccelerationStructureKHR                  dst,
    VkAccelerationStructureKHR                  src,
    VkBuffer                                    scratch,
    VkDeviceSize                                scratchOffset) {
    ValidationStateTracker::PreCallRecordCmdBuildAccelerationStructureNV(commandBuffer, pInfo, instanceData, instanceOffset, update, dst, src, scratch, scratchOffset);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCmdBuildAccelerationStructureNV(commandBuffer, pInfo, instanceData, instanceOffset, update, dst, src, scratch, scratchOffset); });
}

void BestPractices::PostCallRecordCmdBuildAccelerationStructureNV(
    VkCommandBuffer                             commandBuffer,
    const VkAccelerationStructureInfoNV*        pInfo,
    VkBuffer                                    instanceData,
    VkDeviceSize                                instanceOffset,
    VkBool32                                    update,
    VkAccelerationStructureKHR                  dst,
    VkAccelerationStructureKHR                  src,
    VkBuffer                                    scratch,
    VkDeviceSize                                scratchOffset) {
    ValidationStateTracker::PostCallRecordCmdBuildAccelerationStructureNV(commandBuffer, pInfo, instanceData, instanceOffset, update, dst, src, scratch, scratchOffset);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCmdBuildAccelerationStructureNV(commandBuffer, pInfo, instanceData, instanceOffset, update, dst, src, scratch, scratchOffset); });
}

bool BestPractices::PreCallValidateCmdCopyAccelerationStructureNV(
    VkCommandBuffer                             commandBuffer,
    VkAccelerationStructureKHR                  dst,
    VkAccelerationStructureKHR                  src,
    VkCopyAccelerationStructureModeKHR          mode) const {
    bool skip = ValidationStateTracker::PreCallValidateCmdCopyAccelerationStructureNV(commandBuffer, dst, src, mode);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCmdCopyAccelerationStructureNV(commandBuffer, dst, src, mode); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCmdCopyAccelerationStructureNV(
    VkCommandBuffer                             commandBuffer,
    VkAccelerationStructureKHR                  dst,
    VkAccelerationStructureKHR                  src,
    VkCopyAccelerationStructureModeKHR          mode) {
    ValidationStateTracker::PreCallRecordCmdCopyAccelerationStructureNV(commandBuffer, dst, src, mode);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCmdCopyAccelerationStructureNV(commandBuffer, dst, src, mode); });
}

void BestPractices::PostCallRecordCmdCopyAccelerationStructureNV(
    VkCommandBuffer                             commandBuffer,
    VkAccelerationStructureKHR                  dst,
    VkAccelerationStructureKHR                  src,
    VkCopyAccelerationStructureModeKHR          mode) {
    ValidationStateTracker::PostCallRecordCmdCopyAccelerationStructureNV(commandBuffer, dst, src, mode);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCmdCopyAccelerationStructureNV(commandBuffer, dst, src, mode); });
}

bool BestPractices::PreCallValidateCmdTraceRaysNV(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    raygenShaderBindingTableBuffer,
    VkDeviceSize                                raygenShaderBindingOffset,
    VkBuffer                                    missShaderBindingTableBuffer,
    VkDeviceSize                                missShaderBindingOffset,
    VkDeviceSize                                missShaderBindingStride,
    VkBuffer                                    hitShaderBindingTableBuffer,
    VkDeviceSize                                hitShaderBindingOffset,
    VkDeviceSize                                hitShaderBindingStride,
    VkBuffer                                    callableShaderBindingTableBuffer,
    VkDeviceSize                                callableShaderBindingOffset,
    VkDeviceSize                                callableShaderBindingStride,
    uint32_t                                    width,
    uint32_t                                    height,
    uint32_t                                    depth) const {
    bool skip = ValidationStateTracker::PreCallValidateCmdTraceRaysNV(commandBuffer, raygenShaderBindingTableBuffer, raygenShaderBindingOffset, missShaderBindingTableBuffer, missShaderBindingOffset, missShaderBindingStride, hitShaderBindingTableBuffer, hitShaderBindingOffset, hitShaderBindingStride, callableShaderBindingTableBuffer, callableShaderBindingOffset, callableShaderBindingStride, width, height, depth);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCmdTraceRaysNV(commandBuffer, raygenShaderBindingTableBuffer, raygenShaderBindingOffset, missShaderBindingTableBuffer, missShaderBindingOffset, missShaderBindingStride, hitShaderBindingTableBuffer, hitShaderBindingOffset, hitShaderBindingStride, callableShaderBindingTableBuffer, callableShaderBindingOffset, callableShaderBindingStride, width, height, depth); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCmdTraceRaysNV(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    raygenShaderBindingTableBuffer,
    VkDeviceSize                                raygenShaderBindingOffset,
    VkBuffer                                    missShaderBindingTableBuffer,
    VkDeviceSize                                missShaderBindingOffset,
    VkDeviceSize                                missShaderBindingStride,
    VkBuffer                                    hitShaderBindingTableBuffer,
    VkDeviceSize                                hitShaderBindingOffset,
    VkDeviceSize                                hitShaderBindingStride,
    VkBuffer                                    callableShaderBindingTableBuffer,
    VkDeviceSize                                callableShaderBindingOffset,
    VkDeviceSize                                callableShaderBindingStride,
    uint32_t                                    width,
    uint32_t                                    height,
    uint32_t                                    depth) {
    ValidationStateTracker::PreCallRecordCmdTraceRaysNV(commandBuffer, raygenShaderBindingTableBuffer, raygenShaderBindingOffset, missShaderBindingTableBuffer, missShaderBindingOffset, missShaderBindingStride, hitShaderBindingTableBuffer, hitShaderBindingOffset, hitShaderBindingStride, callableShaderBindingTableBuffer, callableShaderBindingOffset, callableShaderBindingStride, width, height, depth);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCmdTraceRaysNV(commandBuffer, raygenShaderBindingTableBuffer, raygenShaderBindingOffset, missShaderBindingTableBuffer, missShaderBindingOffset, missShaderBindingStride, hitShaderBindingTableBuffer, hitShaderBindingOffset, hitShaderBindingStride, callableShaderBindingTableBuffer, callableShaderBindingOffset, callableShaderBindingStride, width, height, depth); });
}

void BestPractices::PostCallRecordCmdTraceRaysNV(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    raygenShaderBindingTableBuffer,
    VkDeviceSize                                raygenShaderBindingOffset,
    VkBuffer                                    missShaderBindingTableBuffer,
    VkDeviceSize                                missShaderBindingOffset,
    VkDeviceSize                                missShaderBindingStride,
    VkBuffer                                    hitShaderBindingTableBuffer,
    VkDeviceSize                                hitShaderBindingOffset,
    VkDeviceSize                                hitShaderBindingStride,
    VkBuffer                                    callableShaderBindingTableBuffer,
    VkDeviceSize                                callableShaderBindingOffset,
    VkDeviceSize                                callableShaderBindingStride,
    uint32_t                                    width,
    uint32_t                                    height,
    uint32_t                                    depth) {
    ValidationStateTracker::PostCallRecordCmdTraceRaysNV(commandBuffer, raygenShaderBindingTableBuffer, raygenShaderBindingOffset, missShaderBindingTableBuffer, missShaderBindingOffset, missShaderBindingStride, hitShaderBindingTableBuffer, hitShaderBindingOffset, hitShaderBindingStride, callableShaderBindingTableBuffer, callableShaderBindingOffset, callableShaderBindingStride, width, height, depth);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCmdTraceRaysNV(commandBuffer, raygenShaderBindingTableBuffer, raygenShaderBindingOffset, missShaderBindingTableBuffer, missShaderBindingOffset, missShaderBindingStride, hitShaderBindingTableBuffer, hitShaderBindingOffset, hitShaderBindingStride, callableShaderBindingTableBuffer, callableShaderBindingOffset, callableShaderBindingStride, width, height, depth); });
}

bool BestPractices::PreCallValidateCreateRayTracingPipelinesNV(
    VkDevice                                    device,
    VkPipelineCache                             pipelineCache,
    uint32_t                                    createInfoCount,
    const VkRayTracingPipelineCreateInfoNV*     pCreateInfos,
    const VkAllocationCallbacks*                pAllocator,
    VkPipeline*                                 pPipelines,
    void*                                       state_data) const {
    bool skip = ValidationStateTracker::PreCallValidateCreateRayTracingPipelinesNV(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines, state_data);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCreateRayTracingPipelinesNV(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines, state_data); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCreateRayTracingPipelinesNV(
    VkDevice                                    device,
    VkPipelineCache                             pipelineCache,
    uint32_t                                    createInfoCount,
    const VkRayTracingPipelineCreateInfoNV*     pCreateInfos,
    const VkAllocationCallbacks*                pAllocator,
    VkPipeline*                                 pPipelines,
    void*                                       state_data) {
    ValidationStateTracker::PreCallRecordCreateRayTracingPipelinesNV(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines, state_data);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCreateRayTracingPipelinesNV(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines, state_data); });
}

void BestPractices::PostCallRecordCreateRayTracingPipelinesNV(
    VkDevice                                    device,
    VkPipelineCache                             pipelineCache,
    uint32_t                                    createInfoCount,
    const VkRayTracingPipelineCreateInfoNV*     pCreateInfos,
    const VkAllocationCallbacks*                pAllocator,
    VkPipeline*                                 pPipelines,
    VkResult                                    result,
    void*                                       state_data) {
    ValidationStateTracker::PostCallRecordCreateRayTracingPipelinesNV(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines, result, state_data);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCreateRayTracingPipelinesNV(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines, result, state_data); });
}

bool BestPractices::PreCallValidateGetRayTracingShaderGroupHandlesKHR(
    VkDevice                                    device,
    VkPipeline                                  pipeline,
    uint32_t                                    firstGroup,
    uint32_t                                    groupCount,
    size_t                                      dataSize,
    void*                                       pData) const {
    bool skip = ValidationStateTracker::PreCallValidateGetRayTracingShaderGroupHandlesKHR(device, pipeline, firstGroup, groupCount, dataSize, pData);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetRayTracingShaderGroupHandlesKHR(device, pipeline, firstGroup, groupCount, dataSize, pData); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetRayTracingShaderGroupHandlesKHR(
    VkDevice                                    device,
    VkPipeline                                  pipeline,
    uint32_t                                    firstGroup,
    uint32_t                                    groupCount,
    size_t                                      dataSize,
    void*                                       pData) {
    ValidationStateTracker::PreCallRecordGetRayTracingShaderGroupHandlesKHR(device, pipeline, firstGroup, groupCount, dataSize, pData);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetRayTracingShaderGroupHandlesKHR(device, pipeline, firstGroup, groupCount, dataSize, pData); });
}

void BestPractices::PostCallRecordGetRayTracingShaderGroupHandlesKHR(
    VkDevice                                    device,
    VkPipeline                                  pipeline,
    uint32_t                                    firstGroup,
    uint32_t                                    groupCount,
    size_t                                      dataSize,
    void*                                       pData,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetRayTracingShaderGroupHandlesKHR(device, pipeline, firstGroup, groupCount, dataSize, pData, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetRayTracingShaderGroupHandlesKHR(device, pipeline, firstGroup, groupCount, dataSize, pData, result); });
}

bool BestPractices::PreCallValidateGetRayTracingShaderGroupHandlesNV(
    VkDevice                                    device,
    VkPipeline                                  pipeline,
    uint32_t                                    firstGroup,
    uint32_t                                    groupCount,
    size_t                                      dataSize,
    void*                                       pData) const {
    bool skip = ValidationStateTracker::PreCallValidateGetRayTracingShaderGroupHandlesNV(device, pipeline, firstGroup, groupCount, dataSize, pData);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetRayTracingShaderGroupHandlesNV(device, pipeline, firstGroup, groupCount, dataSize, pData); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetRayTracingShaderGroupHandlesNV(
    VkDevice                                    device,
    VkPipeline                                  pipeline,
    uint32_t                                    firstGroup,
    uint32_t                                    groupCount,
    size_t                                      dataSize,
    void*                                       pData) {
    ValidationStateTracker::PreCallRecordGetRayTracingShaderGroupHandlesNV(device, pipeline, firstGroup, groupCount, dataSize, pData);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetRayTracingShaderGroupHandlesNV(device, pipeline, firstGroup, groupCount, dataSize, pData); });
}

void BestPractices::PostCallRecordGetRayTracingShaderGroupHandlesNV(
    VkDevice                                    device,
    VkPipeline                                  pipeline,
    uint32_t                                    firstGroup,
    uint32_t                                    groupCount,
    size_t                                      dataSize,
    void*                                       pData,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetRayTracingShaderGroupHandlesNV(device, pipeline, firstGroup, groupCount, dataSize, pData, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetRayTracingShaderGroupHandlesNV(device, pipeline, firstGroup, groupCount, dataSize, pData, result); });
}

bool BestPractices::PreCallValidateGetAccelerationStructureHandleNV(
    VkDevice                                    device,
    VkAccelerationStructureKHR                  accelerationStructure,
    size_t                                      dataSize,
    void*                                       pData) const {
    bool skip = ValidationStateTracker::PreCallValidateGetAccelerationStructureHandleNV(device, accelerationStructure, dataSize, pData);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetAccelerationStructureHandleNV(device, accelerationStructure, dataSize, pData); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetAccelerationStructureHandleNV(
    VkDevice                                    device,
    VkAccelerationStructureKHR                  accelerationStructure,
    size_t                                      dataSize,
    void*                                       pData) {
    ValidationStateTracker::PreCallRecordGetAccelerationStructureHandleNV(device, accelerationStructure, dataSize, pData);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetAccelerationStructureHandleNV(device, accelerationStructure, dataSize, pData); });
}

void BestPractices::PostCallRecordGetAccelerationStructureHandleNV(
    VkDevice                                    device,
    VkAccelerationStructureKHR                  accelerationStructure,
    size_t                                      dataSize,
    void*                                       pData,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetAccelerationStructureHandleNV(device, accelerationStructure, dataSize, pData, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetAccelerationStructureHandleNV(device, accelerationStructure, dataSize, pData, result); });
}

bool BestPractices::PreCallValidateCmdWriteAccelerationStructuresPropertiesKHR(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    accelerationStructureCount,
    const VkAccelerationStructureKHR*           pAccelerationStructures,
    VkQueryType                                 queryType,
    VkQueryPool                                 queryPool,
    uint32_t                                    firstQuery) const {
    bool skip = ValidationStateTracker::PreCallValidateCmdWriteAccelerationStructuresPropertiesKHR(commandBuffer, accelerationStructureCount, pAccelerationStructures, queryType, queryPool, firstQuery);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCmdWriteAccelerationStructuresPropertiesKHR(commandBuffer, accelerationStructureCount, pAccelerationStructures, queryType, queryPool, firstQuery); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCmdWriteAccelerationStructuresPropertiesKHR(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    accelerationStructureCount,
    const VkAccelerationStructureKHR*           pAccelerationStructures,
    VkQueryType                                 queryType,
    VkQueryPool                                 queryPool,
    uint32_t                                    firstQuery) {
    ValidationStateTracker::PreCallRecordCmdWriteAccelerationStructuresPropertiesKHR(commandBuffer, accelerationStructureCount, pAccelerationStructures, queryType, queryPool, firstQuery);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCmdWriteAccelerationStructuresPropertiesKHR(commandBuffer, accelerationStructureCount, pAccelerationStructures, queryType, queryPool, firstQuery); });
}

void BestPractices::PostCallRecordCmdWriteAccelerationStructuresPropertiesKHR(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    accelerationStructureCount,
    const VkAccelerationStructureKHR*           pAccelerationStructures,
    VkQueryType                                 queryType,
    VkQueryPool                                 queryPool,
    uint32_t                                    firstQuery) {
    ValidationStateTracker::PostCallRecordCmdWriteAccelerationStructuresPropertiesKHR(commandBuffer, accelerationStructureCount, pAccelerationStructures, queryType, queryPool, firstQuery);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCmdWriteAccelerationStructuresPropertiesKHR(commandBuffer, accelerationStructureCount, pAccelerationStructures, queryType, queryPool, firstQuery); });
}

bool BestPractices::PreCallValidateCmdWriteAccelerationStructuresPropertiesNV(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    accelerationStructureCount,
    const VkAccelerationStructureKHR*           pAccelerationStructures,
    VkQueryType                                 queryType,
    VkQueryPool                                 queryPool,
    uint32_t                                    firstQuery) const {
    bool skip = ValidationStateTracker::PreCallValidateCmdWriteAccelerationStructuresPropertiesNV(commandBuffer, accelerationStructureCount, pAccelerationStructures, queryType, queryPool, firstQuery);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCmdWriteAccelerationStructuresPropertiesNV(commandBuffer, accelerationStructureCount, pAccelerationStructures, queryType, queryPool, firstQuery); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCmdWriteAccelerationStructuresPropertiesNV(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    accelerationStructureCount,
    const VkAccelerationStructureKHR*           pAccelerationStructures,
    VkQueryType                                 queryType,
    VkQueryPool                                 queryPool,
    uint32_t                                    firstQuery) {
    ValidationStateTracker::PreCallRecordCmdWriteAccelerationStructuresPropertiesNV(commandBuffer, accelerationStructureCount, pAccelerationStructures, queryType, queryPool, firstQuery);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCmdWriteAccelerationStructuresPropertiesNV(commandBuffer, accelerationStructureCount, pAccelerationStructures, queryType, queryPool, firstQuery); });
}

void BestPractices::PostCallRecordCmdWriteAccelerationStructuresPropertiesNV(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    accelerationStructureCount,
    const VkAccelerationStructureKHR*           pAccelerationStructures,
    VkQueryType                                 queryType,
    VkQueryPool                                 queryPool,
    uint32_t                                    firstQuery) {
    ValidationStateTracker::PostCallRecordCmdWriteAccelerationStructuresPropertiesNV(commandBuffer, accelerationStructureCount, pAccelerationStructures, queryType, queryPool, firstQuery);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCmdWriteAccelerationStructuresPropertiesNV(commandBuffer, accelerationStructureCount, pAccelerationStructures, queryType, queryPool, firstQuery); });
}

bool BestPractices::PreCallValidateCompileDeferredNV(
    VkDevice                                    device,
    VkPipeline                                  pipeline,
    uint32_t                                    shader) const {
    bool skip = ValidationStateTracker::PreCallValidateCompileDeferredNV(device, pipeline, shader);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCompileDeferredNV(device, pipeline, shader); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCompileDeferredNV(
    VkDevice                                    device,
    VkPipeline                                  pipeline,
    uint32_t                                    shader) {
    ValidationStateTracker::PreCallRecordCompileDeferredNV(device, pipeline, shader);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCompileDeferredNV(device, pipeline, shader); });
}

void BestPractices::PostCallRecordCompileDeferredNV(
    VkDevice                                    device,
    VkPipeline                                  pipeline,
    uint32_t                                    shader,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCompileDeferredNV(device, pipeline, shader, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCompileDeferredNV(device, pipeline, shader, result); });
}

bool BestPractices::PreCallValidateGetMemoryHostPointerPropertiesEXT(
    VkDevice                                    device,
    VkExternalMemoryHandleTypeFlagBits          handleType,
    const void*                                 pHostPointer,
    VkMemoryHostPointerPropertiesEXT*           pMemoryHostPointerProperties) const {
    bool skip = ValidationStateTracker::PreCallValidateGetMemoryHostPointerPropertiesEXT(device, handleType, pHostPointer, pMemoryHostPointerProperties);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetMemoryHostPointerPropertiesEXT(device, handleType, pHostPointer, pMemoryHostPointerProperties); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetMemoryHostPointerPropertiesEXT(
    VkDevice                                    device,
    VkExternalMemoryHandleTypeFlagBits          handleType,
    const void*                                 pHostPointer,
    VkMemoryHostPointerPropertiesEXT*           pMemoryHostPointerProperties) {
    ValidationStateTracker::PreCallRecordGetMemoryHostPointerPropertiesEXT(device, handleType, pHostPointer, pMemoryHostPointerProperties);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetMemoryHostPointerPropertiesEXT(device, handleType, pHostPointer, pMemoryHostPointerProperties); });
}

void BestPractices::PostCallRecordGetMemoryHostPointerPropertiesEXT(
    VkDevice                                    device,
    VkExternalMemoryHandleTypeFlagBits          handleType,
    const void*                                 pHostPointer,
    VkMemoryHostPointerPropertiesEXT*           pMemoryHostPointerProperties,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetMemoryHostPointerPropertiesEXT(device, handleType, pHostPointer, pMemoryHostPointerProperties, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetMemoryHostPointerPropertiesEXT(device, handleType, pHostPointer, pMemoryHostPointerProperties, result); });
}

bool BestPractices::PreCallValidateCmdWriteBufferMarkerAMD(
    VkCommandBuffer                             commandBuffer,
    VkPipelineStageFlagBits                     pipelineStage,
    VkBuffer                                    dstBuffer,
    VkDeviceSize                                dstOffset,
    uint32_t                                    marker) const {
    bool skip = ValidationStateTracker::PreCallValidateCmdWriteBufferMarkerAMD(commandBuffer, pipelineStage, dstBuffer, dstOffset, marker);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCmdWriteBufferMarkerAMD(commandBuffer, pipelineStage, dstBuffer, dstOffset, marker); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCmdWriteBufferMarkerAMD(
    VkCommandBuffer                             commandBuffer,
    VkPipelineStageFlagBits                     pipelineStage,
    VkBuffer                                    dstBuffer,
    VkDeviceSize                                dstOffset,
    uint32_t                                    marker) {
    ValidationStateTracker::PreCallRecordCmdWriteBufferMarkerAMD(commandBuffer, pipelineStage, dstBuffer, dstOffset, marker);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCmdWriteBufferMarkerAMD(commandBuffer, pipelineStage, dstBuffer, dstOffset, marker); });
}

void BestPractices::PostCallRecordCmdWriteBufferMarkerAMD(
    VkCommandBuffer                             commandBuffer,
    VkPipelineStageFlagBits                     pipelineStage,
    VkBuffer                                    dstBuffer,
    VkDeviceSize                                dstOffset,
    uint32_t                                    marker) {
    ValidationStateTracker::PostCallRecordCmdWriteBufferMarkerAMD(commandBuffer, pipelineStage, dstBuffer, dstOffset, marker);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCmdWriteBufferMarkerAMD(commandBuffer, pipelineStage, dstBuffer, dstOffset, marker); });
}

bool BestPractices::PreCallValidateGetPhysicalDeviceCalibrateableTimeDomainsEXT(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pTimeDomainCount,
    VkTimeDomainEXT*                            pTimeDomains) const {
    bool skip = ValidationStateTracker::PreCallValidateGetPhysicalDeviceCalibrateableTimeDomainsEXT(physicalDevice, pTimeDomainCount, pTimeDomains);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetPhysicalDeviceCalibrateableTimeDomainsEXT(physicalDevice, pTimeDomainCount, pTimeDomains); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetPhysicalDeviceCalibrateableTimeDomainsEXT(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pTimeDomainCount,
    VkTimeDomainEXT*                            pTimeDomains) {
    ValidationStateTracker::PreCallRecordGetPhysicalDeviceCalibrateableTimeDomainsEXT(physicalDevice, pTimeDomainCount, pTimeDomains);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetPhysicalDeviceCalibrateableTimeDomainsEXT(physicalDevice, pTimeDomainCount, pTimeDomains); });
}

void BestPractices::PostCallRecordGetPhysicalDeviceCalibrateableTimeDomainsEXT(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pTimeDomainCount,
    VkTimeDomainEXT*                            pTimeDomains,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceCalibrateableTimeDomainsEXT(physicalDevice, pTimeDomainCount, pTimeDomains, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetPhysicalDeviceCalibrateableTimeDomainsEXT(physicalDevice, pTimeDomainCount, pTimeDomains, result); });
}

bool BestPractices::PreCallValidateGetCalibratedTimestampsEXT(
    VkDevice                                    device,
    uint32_t                                    timestampCount,
    const VkCalibratedTimestampInfoEXT*         pTimestampInfos,
    uint64_t*                                   pTimestamps,
    uint64_t*                                   pMaxDeviation) const {
    bool skip = ValidationStateTracker::PreCallValidateGetCalibratedTimestampsEXT(device, timestampCount, pTimestampInfos, pTimestamps, pMaxDeviation);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetCalibratedTimestampsEXT(device, timestampCount, pTimestampInfos, pTimestamps, pMaxDeviation); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetCalibratedTimestampsEXT(
    VkDevice                                    device,
    uint32_t                                    timestampCount,
    const VkCalibratedTimestampInfoEXT*         pTimestampInfos,
    uint64_t*                                   pTimestamps,
    uint64_t*                                   pMaxDeviation) {
    ValidationStateTracker::PreCallRecordGetCalibratedTimestampsEXT(device, timestampCount, pTimestampInfos, pTimestamps, pMaxDeviation);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetCalibratedTimestampsEXT(device, timestampCount, pTimestampInfos, pTimestamps, pMaxDeviation); });
}

void BestPractices::PostCallRecordGetCalibratedTimestampsEXT(
    VkDevice                                    device,
    uint32_t                                    timestampCount,
    const VkCalibratedTimestampInfoEXT*         pTimestampInfos,
    uint64_t*                                   pTimestamps,
    uint64_t*                                   pMaxDeviation,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetCalibratedTimestampsEXT(device, timestampCount, pTimestampInfos, pTimestamps, pMaxDeviation, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetCalibratedTimestampsEXT(device, timestampCount, pTimestampInfos, pTimestamps, pMaxDeviation, result); });
}

bool BestPractices::PreCallValidateCmdDrawMeshTasksNV(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    taskCount,
    uint32_t                                    firstTask) const {
    bool skip = ValidationStateTracker::PreCallValidateCmdDrawMeshTasksNV(commandBuffer, taskCount, firstTask);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCmdDrawMeshTasksNV(commandBuffer, taskCount, firstTask); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCmdDrawMeshTasksNV(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    taskCount,
    uint32_t                                    firstTask) {
    ValidationStateTracker::PreCallRecordCmdDrawMeshTasksNV(commandBuffer, taskCount, firstTask);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCmdDrawMeshTasksNV(commandBuffer, taskCount, firstTask); });
}

void BestPractices::PostCallRecordCmdDrawMeshTasksNV(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    taskCount,
    uint32_t                                    firstTask) {
    ValidationStateTracker::PostCallRecordCmdDrawMeshTasksNV(commandBuffer, taskCount, firstTask);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCmdDrawMeshTasksNV(commandBuffer, taskCount, firstTask); });
}

bool BestPractices::PreCallValidateCmdDrawMeshTasksIndirectNV(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    uint32_t                                    drawCount,
    uint32_t                                    stride) const {
    bool skip = ValidationStateTracker::PreCallValidateCmdDrawMeshTasksIndirectNV(commandBuffer, buffer, offset, drawCount, stride);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCmdDrawMeshTasksIndirectNV(commandBuffer, buffer, offset, drawCount, stride); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCmdDrawMeshTasksIndirectNV(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    uint32_t                                    drawCount,
    uint32_t                                    stride) {
    ValidationStateTracker::PreCallRecordCmdDrawMeshTasksIndirectNV(commandBuffer, buffer, offset, drawCount, stride);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCmdDrawMeshTasksIndirectNV(commandBuffer, buffer, offset, drawCount, stride); });
}

void BestPractices::PostCallRecordCmdDrawMeshTasksIndirectNV(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    uint32_t                                    drawCount,
    uint32_t                                    stride) {
    ValidationStateTracker::PostCallRecordCmdDrawMeshTasksIndirectNV(commandBuffer, buffer, offset, drawCount, stride);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCmdDrawMeshTasksIndirectNV(commandBuffer, buffer, offset, drawCount, stride); });
}

bool BestPractices::PreCallValidateCmdDrawMeshTasksIndirectCountNV(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkBuffer                                    countBuffer,
    VkDeviceSize                                countBufferOffset,
    uint32_t                                    maxDrawCount,
    uint32_t                                    stride) const {
    bool skip = ValidationStateTracker::PreCallValidateCmdDrawMeshTasksIndirectCountNV(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCmdDrawMeshTasksIndirectCountNV(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCmdDrawMeshTasksIndirectCountNV(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkBuffer                                    countBuffer,
    VkDeviceSize                                countBufferOffset,
    uint32_t                                    maxDrawCount,
    uint32_t                                    stride) {
    ValidationStateTracker::PreCallRecordCmdDrawMeshTasksIndirectCountNV(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCmdDrawMeshTasksIndirectCountNV(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride); });
}

void BestPractices::PostCallRecordCmdDrawMeshTasksIndirectCountNV(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkBuffer                                    countBuffer,
    VkDeviceSize                                countBufferOffset,
    uint32_t                                    maxDrawCount,
    uint32_t                                    stride) {
    ValidationStateTracker::PostCallRecordCmdDrawMeshTasksIndirectCountNV(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCmdDrawMeshTasksIndirectCountNV(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride); });
}

bool BestPractices::PreCallValidateCmdSetExclusiveScissorNV(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstExclusiveScissor,
    uint32_t                                    exclusiveScissorCount,
    const VkRect2D*                             pExclusiveScissors) const {
    bool skip = ValidationStateTracker::PreCallValidateCmdSetExclusiveScissorNV(commandBuffer, firstExclusiveScissor, exclusiveScissorCount, pExclusiveScissors);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCmdSetExclusiveScissorNV(commandBuffer, firstExclusiveScissor, exclusiveScissorCount, pExclusiveScissors); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCmdSetExclusiveScissorNV(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstExclusiveScissor,
    uint32_t                                    exclusiveScissorCount,
    const VkRect2D*                             pExclusiveScissors) {
    ValidationStateTracker::PreCallRecordCmdSetExclusiveScissorNV(commandBuffer, firstExclusiveScissor, exclusiveScissorCount, pExclusiveScissors);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCmdSetExclusiveScissorNV(commandBuffer, firstExclusiveScissor, exclusiveScissorCount, pExclusiveScissors); });
}

void BestPractices::PostCallRecordCmdSetExclusiveScissorNV(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstExclusiveScissor,
    uint32_t                                    exclusiveScissorCount,
    const VkRect2D*                             pExclusiveScissors) {
    ValidationStateTracker::PostCallRecordCmdSetExclusiveScissorNV(commandBuffer, firstExclusiveScissor, exclusiveScissorCount, pExclusiveScissors);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCmdSetExclusiveScissorNV(commandBuffer, firstExclusiveScissor, exclusiveScissorCount, pExclusiveScissors); });
}

bool BestPractices::PreCallValidateCmdSetCheckpointNV(
    VkCommandBuffer                             commandBuffer,
    const void*                                 pCheckpointMarker) const {
    bool skip = ValidationStateTracker::PreCallValidateCmdSetCheckpointNV(commandBuffer, pCheckpointMarker);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCmdSetCheckpointNV(commandBuffer, pCheckpointMarker); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCmdSetCheckpointNV(
    VkCommandBuffer                             commandBuffer,
    const void*                                 pCheckpointMarker) {
    ValidationStateTracker::PreCallRecordCmdSetCheckpointNV(commandBuffer, pCheckpointMarker);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCmdSetCheckpointNV(commandBuffer, pCheckpointMarker); });
}

void BestPractices::PostCallRecordCmdSetCheckpointNV(
    VkCommandBuffer                             commandBuffer,
    const void*                                 pCheckpointMarker) {
    ValidationStateTracker::PostCallRecordCmdSetCheckpointNV(commandBuffer, pCheckpointMarker);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCmdSetCheckpointNV(commandBuffer, pCheckpointMarker); });
}

bool BestPractices::PreCallValidateGetQueueCheckpointDataNV(
    VkQueue                                     queue,
    uint32_t*                                   pCheckpointDataCount,
    VkCheckpointDataNV*                         pCheckpointData) const {
    bool skip = ValidationStateTracker::PreCallValidateGetQueueCheckpointDataNV(queue, pCheckpointDataCount, pCheckpointData);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetQueueCheckpointDataNV(queue, pCheckpointDataCount, pCheckpointData); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetQueueCheckpointDataNV(
    VkQueue                                     queue,
    uint32_t*                                   pCheckpointDataCount,
    VkCheckpointDataNV*                         pCheckpointData) {
    ValidationStateTracker::PreCallRecordGetQueueCheckpointDataNV(queue, pCheckpointDataCount, pCheckpointData);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetQueueCheckpointDataNV(queue, pCheckpointDataCount, pCheckpointData); });
}

void BestPractices::PostCallRecordGetQueueCheckpointDataNV(
    VkQueue                                     queue,
    uint32_t*                                   pCheckpointDataCount,
    VkCheckpointDataNV*                         pCheckpointData) {
    ValidationStateTracker::PostCallRecordGetQueueCheckpointDataNV(queue, pCheckpointDataCount, pCheckpointData);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetQueueCheckpointDataNV(queue, pCheckpointDataCount, pCheckpointData); });
}

bool BestPractices::PreCallValidateInitializePerformanceApiINTEL(
    VkDevice                                    device,
    const VkInitializePerformanceApiInfoINTEL*  pInitializeInfo) const {
    bool skip = ValidationStateTracker::PreCallValidateInitializePerformanceApiINTEL(device, pInitializeInfo);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateInitializePerformanceApiINTEL(device, pInitializeInfo); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordInitializePerformanceApiINTEL(
    VkDevice                                    device,
    const VkInitializePerformanceApiInfoINTEL*  pInitializeInfo) {
    ValidationStateTracker::PreCallRecordInitializePerformanceApiINTEL(device, pInitializeInfo);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordInitializePerformanceApiINTEL(device, pInitializeInfo); });
}

void BestPractices::PostCallRecordInitializePerformanceApiINTEL(
    VkDevice                                    device,
    const VkInitializePerformanceApiInfoINTEL*  pInitializeInfo,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordInitializePerformanceApiINTEL(device, pInitializeInfo, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordInitializePerformanceApiINTEL(device, pInitializeInfo, result); });
}

bool BestPractices::PreCallValidateUninitializePerformanceApiINTEL(
    VkDevice                                    device) const {
    bool skip = ValidationStateTracker::PreCallValidateUninitializePerformanceApiINTEL(device);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateUninitializePerformanceApiINTEL(device); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordUninitializePerformanceApiINTEL(
    VkDevice                                    device) {
    ValidationStateTracker::PreCallRecordUninitializePerformanceApiINTEL(device);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordUninitializePerformanceApiINTEL(device); });
}

void BestPractices::PostCallRecordUninitializePerformanceApiINTEL(
    VkDevice                                    device) {
    ValidationStateTracker::PostCallRecordUninitializePerformanceApiINTEL(device);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordUninitializePerformanceApiINTEL(device); });
}

bool BestPractices::PreCallValidateCmdSetPerformanceMarkerINTEL(
    VkCommandBuffer                             commandBuffer,
    const VkPerformanceMarkerInfoINTEL*         pMarkerInfo) const {
    bool skip = ValidationStateTracker::PreCallValidateCmdSetPerformanceMarkerINTEL(commandBuffer, pMarkerInfo);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCmdSetPerformanceMarkerINTEL(commandBuffer, pMarkerInfo); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCmdSetPerformanceMarkerINTEL(
    VkCommandBuffer                             commandBuffer,
    const VkPerformanceMarkerInfoINTEL*         pMarkerInfo) {
    ValidationStateTracker::PreCallRecordCmdSetPerformanceMarkerINTEL(commandBuffer, pMarkerInfo);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCmdSetPerformanceMarkerINTEL(commandBuffer, pMarkerInfo); });
}

void BestPractices::PostCallRecordCmdSetPerformanceMarkerINTEL(
    VkCommandBuffer                             commandBuffer,
    const VkPerformanceMarkerInfoINTEL*         pMarkerInfo,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCmdSetPerformanceMarkerINTEL(commandBuffer, pMarkerInfo, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCmdSetPerformanceMarkerINTEL(commandBuffer, pMarkerInfo, result); });
}

bool BestPractices::PreCallValidateCmdSetPerformanceStreamMarkerINTEL(
    VkCommandBuffer                             commandBuffer,
    const VkPerformanceStreamMarkerInfoINTEL*   pMarkerInfo) const {
    bool skip = ValidationStateTracker::PreCallValidateCmdSetPerformanceStreamMarkerINTEL(commandBuffer, pMarkerInfo);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCmdSetPerformanceStreamMarkerINTEL(commandBuffer, pMarkerInfo); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCmdSetPerformanceStreamMarkerINTEL(
    VkCommandBuffer                             commandBuffer,
    const VkPerformanceStreamMarkerInfoINTEL*   pMarkerInfo) {
    ValidationStateTracker::PreCallRecordCmdSetPerformanceStreamMarkerINTEL(commandBuffer, pMarkerInfo);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCmdSetPerformanceStreamMarkerINTEL(commandBuffer, pMarkerInfo); });
}

void BestPractices::PostCallRecordCmdSetPerformanceStreamMarkerINTEL(
    VkCommandBuffer                             commandBuffer,
    const VkPerformanceStreamMarkerInfoINTEL*   pMarkerInfo,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCmdSetPerformanceStreamMarkerINTEL(commandBuffer, pMarkerInfo, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCmdSetPerformanceStreamMarkerINTEL(commandBuffer, pMarkerInfo, result); });
}

bool BestPractices::PreCallValidateCmdSetPerformanceOverrideINTEL(
    VkCommandBuffer                             commandBuffer,
    const VkPerformanceOverrideInfoINTEL*       pOverrideInfo) const {
    bool skip = ValidationStateTracker::PreCallValidateCmdSetPerformanceOverrideINTEL(commandBuffer, pOverrideInfo);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCmdSetPerformanceOverrideINTEL(commandBuffer, pOverrideInfo); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCmdSetPerformanceOverrideINTEL(
    VkCommandBuffer                             commandBuffer,
    const VkPerformanceOverrideInfoINTEL*       pOverrideInfo) {
    ValidationStateTracker::PreCallRecordCmdSetPerformanceOverrideINTEL(commandBuffer, pOverrideInfo);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCmdSetPerformanceOverrideINTEL(commandBuffer, pOverrideInfo); });
}

void BestPractices::PostCallRecordCmdSetPerformanceOverrideINTEL(
    VkCommandBuffer                             commandBuffer,
    const VkPerformanceOverrideInfoINTEL*       pOverrideInfo,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCmdSetPerformanceOverrideINTEL(commandBuffer, pOverrideInfo, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCmdSetPerformanceOverrideINTEL(commandBuffer, pOverrideInfo, result); });
}

bool BestPractices::PreCallValidateAcquirePerformanceConfigurationINTEL(
    VkDevice                                    device,
    const VkPerformanceConfigurationAcquireInfoINTEL* pAcquireInfo,
    VkPerformanceConfigurationINTEL*            pConfiguration) const {
    bool skip = ValidationStateTracker::PreCallValidateAcquirePerformanceConfigurationINTEL(device, pAcquireInfo, pConfiguration);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateAcquirePerformanceConfigurationINTEL(device, pAcquireInfo, pConfiguration); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordAcquirePerformanceConfigurationINTEL(
    VkDevice                                    device,
    const VkPerformanceConfigurationAcquireInfoINTEL* pAcquireInfo,
    VkPerformanceConfigurationINTEL*            pConfiguration) {
    ValidationStateTracker::PreCallRecordAcquirePerformanceConfigurationINTEL(device, pAcquireInfo, pConfiguration);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordAcquirePerformanceConfigurationINTEL(device, pAcquireInfo, pConfiguration); });
}

void BestPractices::PostCallRecordAcquirePerformanceConfigurationINTEL(
    VkDevice                                    device,
    const VkPerformanceConfigurationAcquireInfoINTEL* pAcquireInfo,
    VkPerformanceConfigurationINTEL*            pConfiguration,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordAcquirePerformanceConfigurationINTEL(device, pAcquireInfo, pConfiguration, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordAcquirePerformanceConfigurationINTEL(device, pAcquireInfo, pConfiguration, result); });
}

bool BestPractices::PreCallValidateReleasePerformanceConfigurationINTEL(
    VkDevice                                    device,
    VkPerformanceConfigurationINTEL             configuration) const {
    bool skip = ValidationStateTracker::PreCallValidateReleasePerformanceConfigurationINTEL(device, configuration);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateReleasePerformanceConfigurationINTEL(device, configuration); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordReleasePerformanceConfigurationINTEL(
    VkDevice                                    device,
    VkPerformanceConfigurationINTEL             configuration) {
    ValidationStateTracker::PreCallRecordReleasePerformanceConfigurationINTEL(device, configuration);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordReleasePerformanceConfigurationINTEL(device, configuration); });
}

void BestPractices::PostCallRecordReleasePerformanceConfigurationINTEL(
    VkDevice                                    device,
    VkPerformanceConfigurationINTEL             configuration,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordReleasePerformanceConfigurationINTEL(device, configuration, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordReleasePerformanceConfigurationINTEL(device, configuration, result); });
}

bool BestPractices::PreCallValidateQueueSetPerformanceConfigurationINTEL(
    VkQueue                                     queue,
    VkPerformanceConfigurationINTEL             configuration) const {
    bool skip = ValidationStateTracker::PreCallValidateQueueSetPerformanceConfigurationINTEL(queue, configuration);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateQueueSetPerformanceConfigurationINTEL(queue, configuration); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordQueueSetPerformanceConfigurationINTEL(
    VkQueue                                     queue,
    VkPerformanceConfigurationINTEL             configuration) {
    ValidationStateTracker::PreCallRecordQueueSetPerformanceConfigurationINTEL(queue, configuration);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordQueueSetPerformanceConfigurationINTEL(queue, configuration); });
}

void BestPractices::PostCallRecordQueueSetPerformanceConfigurationINTEL(
    VkQueue                                     queue,
    VkPerformanceConfigurationINTEL             configuration,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordQueueSetPerformanceConfigurationINTEL(queue, configuration, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordQueueSetPerformanceConfigurationINTEL(queue, configuration, result); });
}

bool BestPractices::PreCallValidateGetPerformanceParameterINTEL(
    VkDevice                                    device,
    VkPerformanceParameterTypeINTEL             parameter,
    VkPerformanceValueINTEL*                    pValue) const {
    bool skip = ValidationStateTracker::PreCallValidateGetPerformanceParameterINTEL(device, parameter, pValue);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetPerformanceParameterINTEL(device, parameter, pValue); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetPerformanceParameterINTEL(
    VkDevice                                    device,
    VkPerformanceParameterTypeINTEL             parameter,
    VkPerformanceValueINTEL*                    pValue) {
    ValidationStateTracker::PreCallRecordGetPerformanceParameterINTEL(device, parameter, pValue);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetPerformanceParameterINTEL(device, parameter, pValue); });
}

void BestPractices::PostCallRecordGetPerformanceParameterINTEL(
    VkDevice                                    device,
    VkPerformanceParameterTypeINTEL             parameter,
    VkPerformanceValueINTEL*                    pValue,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetPerformanceParameterINTEL(device, parameter, pValue, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetPerformanceParameterINTEL(device, parameter, pValue, result); });
}

bool BestPractices::PreCallValidateSetLocalDimmingAMD(
    VkDevice                                    device,
    VkSwapchainKHR                              swapChain,
    VkBool32                                    localDimmingEnable) const {
    bool skip = ValidationStateTracker::PreCallValidateSetLocalDimmingAMD(device, swapChain, localDimmingEnable);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateSetLocalDimmingAMD(device, swapChain, localDimmingEnable); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordSetLocalDimmingAMD(
    VkDevice                                    device,
    VkSwapchainKHR                              swapChain,
    VkBool32                                    localDimmingEnable) {
    ValidationStateTracker::PreCallRecordSetLocalDimmingAMD(device, swapChain, localDimmingEnable);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordSetLocalDimmingAMD(device, swapChain, localDimmingEnable); });
}

void BestPractices::PostCallRecordSetLocalDimmingAMD(
    VkDevice                                    device,
    VkSwapchainKHR                              swapChain,
    VkBool32                                    localDimmingEnable) {
    ValidationStateTracker::PostCallRecordSetLocalDimmingAMD(device, swapChain, localDimmingEnable);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordSetLocalDimmingAMD(device, swapChain, localDimmingEnable); });
}

bool BestPractices::PreCallValidateCreateImagePipeSurfaceFUCHSIA(
    VkInstance                                  instance,
    const VkImagePipeSurfaceCreateInfoFUCHSIA*  pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) const {
    bool skip = ValidationStateTracker::PreCallValidateCreateImagePipeSurfaceFUCHSIA(instance, pCreateInfo, pAllocator, pSurface);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCreateImagePipeSurfaceFUCHSIA(instance, pCreateInfo, pAllocator, pSurface); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCreateImagePipeSurfaceFUCHSIA(
    VkInstance                                  instance,
    const VkImagePipeSurfaceCreateInfoFUCHSIA*  pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) {
    ValidationStateTracker::PreCallRecordCreateImagePipeSurfaceFUCHSIA(instance, pCreateInfo, pAllocator, pSurface);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCreateImagePipeSurfaceFUCHSIA(instance, pCreateInfo, pAllocator, pSurface); });
}

void BestPractices::PostCallRecordCreateImagePipeSurfaceFUCHSIA(
    VkInstance                                  instance,
    const VkImagePipeSurfaceCreateInfoFUCHSIA*  pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCreateImagePipeSurfaceFUCHSIA(instance, pCreateInfo, pAllocator, pSurface, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCreateImagePipeSurfaceFUCHSIA(instance, pCreateInfo, pAllocator, pSurface, result); });
}

bool BestPractices::PreCallValidateCreateMetalSurfaceEXT(
    VkInstance                                  instance,
    const VkMetalSurfaceCreateInfoEXT*          pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) const {
    bool skip = ValidationStateTracker::PreCallValidateCreateMetalSurfaceEXT(instance, pCreateInfo, pAllocator, pSurface);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCreateMetalSurfaceEXT(instance, pCreateInfo, pAllocator, pSurface); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCreateMetalSurfaceEXT(
    VkInstance                                  instance,
    const VkMetalSurfaceCreateInfoEXT*          pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) {
    ValidationStateTracker::PreCallRecordCreateMetalSurfaceEXT(instance, pCreateInfo, pAllocator, pSurface);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCreateMetalSurfaceEXT(instance, pCreateInfo, pAllocator, pSurface); });
}

void BestPractices::PostCallRecordCreateMetalSurfaceEXT(
    VkInstance                                  instance,
    const VkMetalSurfaceCreateInfoEXT*          pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCreateMetalSurfaceEXT(instance, pCreateInfo, pAllocator, pSurface, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCreateMetalSurfaceEXT(instance, pCreateInfo, pAllocator, pSurface, result); });
}

bool BestPractices::PreCallValidateGetBufferDeviceAddressEXT(
    VkDevice                                    device,
    const VkBufferDeviceAddressInfo*            pInfo) const {
    bool skip = ValidationStateTracker::PreCallValidateGetBufferDeviceAddressEXT(device, pInfo);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetBufferDeviceAddressEXT(device, pInfo); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetBufferDeviceAddressEXT(
    VkDevice                                    device,
    const VkBufferDeviceAddressInfo*            pInfo) {
    ValidationStateTracker::PreCallRecordGetBufferDeviceAddressEXT(device, pInfo);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetBufferDeviceAddressEXT(device, pInfo); });
}

void BestPractices::PostCallRecordGetBufferDeviceAddressEXT(
    VkDevice                                    device,
    const VkBufferDeviceAddressInfo*            pInfo,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetBufferDeviceAddressEXT(device, pInfo, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetBufferDeviceAddressEXT(device, pInfo, result); });
}

bool BestPractices::PreCallValidateGetPhysicalDeviceToolPropertiesEXT(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pToolCount,
    VkPhysicalDeviceToolPropertiesEXT*          pToolProperties) const {
    bool skip = ValidationStateTracker::PreCallValidateGetPhysicalDeviceToolPropertiesEXT(physicalDevice, pToolCount, pToolProperties);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetPhysicalDeviceToolPropertiesEXT(physicalDevice, pToolCount, pToolProperties); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetPhysicalDeviceToolPropertiesEXT(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pToolCount,
    VkPhysicalDeviceToolPropertiesEXT*          pToolProperties) {
    ValidationStateTracker::PreCallRecordGetPhysicalDeviceToolPropertiesEXT(physicalDevice, pToolCount, pToolProperties);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetPhysicalDeviceToolPropertiesEXT(physicalDevice, pToolCount, pToolProperties); });
}

void BestPractices::PostCallRecordGetPhysicalDeviceToolPropertiesEXT(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pToolCount,
    VkPhysicalDeviceToolPropertiesEXT*          pToolProperties,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceToolPropertiesEXT(physicalDevice, pToolCount, pToolProperties, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetPhysicalDeviceToolPropertiesEXT(physicalDevice, pToolCount, pToolProperties, result); });
}

bool BestPractices::PreCallValidateGetPhysicalDeviceCooperativeMatrixPropertiesNV(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pPropertyCount,
    VkCooperativeMatrixPropertiesNV*            pProperties) const {
    bool skip = ValidationStateTracker::PreCallValidateGetPhysicalDeviceCooperativeMatrixPropertiesNV(physicalDevice, pPropertyCount, pProperties);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetPhysicalDeviceCooperativeMatrixPropertiesNV(physicalDevice, pPropertyCount, pProperties); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetPhysicalDeviceCooperativeMatrixPropertiesNV(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pPropertyCount,
    VkCooperativeMatrixPropertiesNV*            pProperties) {
    ValidationStateTracker::PreCallRecordGetPhysicalDeviceCooperativeMatrixPropertiesNV(physicalDevice, pPropertyCount, pProperties);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetPhysicalDeviceCooperativeMatrixPropertiesNV(physicalDevice, pPropertyCount, pProperties); });
}

void BestPractices::PostCallRecordGetPhysicalDeviceCooperativeMatrixPropertiesNV(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pPropertyCount,
    VkCooperativeMatrixPropertiesNV*            pProperties,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceCooperativeMatrixPropertiesNV(physicalDevice, pPropertyCount, pProperties, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetPhysicalDeviceCooperativeMatrixPropertiesNV(physicalDevice, pPropertyCount, pProperties, result); });
}

bool BestPractices::PreCallValidateGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pCombinationCount,
    VkFramebufferMixedSamplesCombinationNV*     pCombinations) const {
    bool skip = ValidationStateTracker::PreCallValidateGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV(physicalDevice, pCombinationCount, pCombinations);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV(physicalDevice, pCombinationCount, pCombinations); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pCombinationCount,
    VkFramebufferMixedSamplesCombinationNV*     pCombinations) {
    ValidationStateTracker::PreCallRecordGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV(physicalDevice, pCombinationCount, pCombinations);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV(physicalDevice, pCombinationCount, pCombinations); });
}

void BestPractices::PostCallRecordGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pCombinationCount,
    VkFramebufferMixedSamplesCombinationNV*     pCombinations,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV(physicalDevice, pCombinationCount, pCombinations, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV(physicalDevice, pCombinationCount, pCombinations, result); });
}

bool BestPractices::PreCallValidateGetPhysicalDeviceSurfacePresentModes2EXT(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceSurfaceInfo2KHR*      pSurfaceInfo,
    uint32_t*                                   pPresentModeCount,
    VkPresentModeKHR*                           pPresentModes) const {
    bool skip = ValidationStateTracker::PreCallValidateGetPhysicalDeviceSurfacePresentModes2EXT(physicalDevice, pSurfaceInfo, pPresentModeCount, pPresentModes);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetPhysicalDeviceSurfacePresentModes2EXT(physicalDevice, pSurfaceInfo, pPresentModeCount, pPresentModes); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetPhysicalDeviceSurfacePresentModes2EXT(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceSurfaceInfo2KHR*      pSurfaceInfo,
    uint32_t*                                   pPresentModeCount,
    VkPresentModeKHR*                           pPresentModes) {
    ValidationStateTracker::PreCallRecordGetPhysicalDeviceSurfacePresentModes2EXT(physicalDevice, pSurfaceInfo, pPresentModeCount, pPresentModes);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetPhysicalDeviceSurfacePresentModes2EXT(physicalDevice, pSurfaceInfo, pPresentModeCount, pPresentModes); });
}

void BestPractices::PostCallRecordGetPhysicalDeviceSurfacePresentModes2EXT(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceSurfaceInfo2KHR*      pSurfaceInfo,
    uint32_t*                                   pPresentModeCount,
    VkPresentModeKHR*                           pPresentModes,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceSurfacePresentModes2EXT(physicalDevice, pSurfaceInfo, pPresentModeCount, pPresentModes, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetPhysicalDeviceSurfacePresentModes2EXT(physicalDevice, pSurfaceInfo, pPresentModeCount, pPresentModes, result); });
}

bool BestPractices::PreCallValidateAcquireFullScreenExclusiveModeEXT(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain) const {
    bool skip = ValidationStateTracker::PreCallValidateAcquireFullScreenExclusiveModeEXT(device, swapchain);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateAcquireFullScreenExclusiveModeEXT(device, swapchain); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordAcquireFullScreenExclusiveModeEXT(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain) {
    ValidationStateTracker::PreCallRecordAcquireFullScreenExclusiveModeEXT(device, swapchain);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordAcquireFullScreenExclusiveModeEXT(device, swapchain); });
}

void BestPractices::PostCallRecordAcquireFullScreenExclusiveModeEXT(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordAcquireFullScreenExclusiveModeEXT(device, swapchain, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordAcquireFullScreenExclusiveModeEXT(device, swapchain, result); });
}

bool BestPractices::PreCallValidateReleaseFullScreenExclusiveModeEXT(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain) const {
    bool skip = ValidationStateTracker::PreCallValidateReleaseFullScreenExclusiveModeEXT(device, swapchain);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateReleaseFullScreenExclusiveModeEXT(device, swapchain); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordReleaseFullScreenExclusiveModeEXT(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain) {
    ValidationStateTracker::PreCallRecordReleaseFullScreenExclusiveModeEXT(device, swapchain);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordReleaseFullScreenExclusiveModeEXT(device, swapchain); });
}

void BestPractices::PostCallRecordReleaseFullScreenExclusiveModeEXT(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordReleaseFullScreenExclusiveModeEXT(device, swapchain, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordReleaseFullScreenExclusiveModeEXT(device, swapchain, result); });
}

bool BestPractices::PreCallValidateGetDeviceGroupSurfacePresentModes2EXT(
    VkDevice                                    device,
    const VkPhysicalDeviceSurfaceInfo2KHR*      pSurfaceInfo,
    VkDeviceGroupPresentModeFlagsKHR*           pModes) const {
    bool skip = ValidationStateTracker::PreCallValidateGetDeviceGroupSurfacePresentModes2EXT(device, pSurfaceInfo, pModes);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetDeviceGroupSurfacePresentModes2EXT(device, pSurfaceInfo, pModes); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetDeviceGroupSurfacePresentModes2EXT(
    VkDevice                                    device,
    const VkPhysicalDeviceSurfaceInfo2KHR*      pSurfaceInfo,
    VkDeviceGroupPresentModeFlagsKHR*           pModes) {
    ValidationStateTracker::PreCallRecordGetDeviceGroupSurfacePresentModes2EXT(device, pSurfaceInfo, pModes);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetDeviceGroupSurfacePresentModes2EXT(device, pSurfaceInfo, pModes); });
}

void BestPractices::PostCallRecordGetDeviceGroupSurfacePresentModes2EXT(
    VkDevice                                    device,
    const VkPhysicalDeviceSurfaceInfo2KHR*      pSurfaceInfo,
    VkDeviceGroupPresentModeFlagsKHR*           pModes,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetDeviceGroupSurfacePresentModes2EXT(device, pSurfaceInfo, pModes, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetDeviceGroupSurfacePresentModes2EXT(device, pSurfaceInfo, pModes, result); });
}

bool BestPractices::PreCallValidateCreateHeadlessSurfaceEXT(
    VkInstance                                  instance,
    const VkHeadlessSurfaceCreateInfoEXT*       pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) const {
    bool skip = ValidationStateTracker::PreCallValidateCreateHeadlessSurfaceEXT(instance, pCreateInfo, pAllocator, pSurface);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCreateHeadlessSurfaceEXT(instance, pCreateInfo, pAllocator, pSurface); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCreateHeadlessSurfaceEXT(
    VkInstance                                  instance,
    const VkHeadlessSurfaceCreateInfoEXT*       pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) {
    ValidationStateTracker::PreCallRecordCreateHeadlessSurfaceEXT(instance, pCreateInfo, pAllocator, pSurface);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCreateHeadlessSurfaceEXT(instance, pCreateInfo, pAllocator, pSurface); });
}

void BestPractices::PostCallRecordCreateHeadlessSurfaceEXT(
    VkInstance                                  instance,
    const VkHeadlessSurfaceCreateInfoEXT*       pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCreateHeadlessSurfaceEXT(instance, pCreateInfo, pAllocator, pSurface, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCreateHeadlessSurfaceEXT(instance, pCreateInfo, pAllocator, pSurface, result); });
}

bool BestPractices::PreCallValidateCmdSetLineStippleEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    lineStippleFactor,
    uint16_t                                    lineStipplePattern) const {
    bool skip = ValidationStateTracker::PreCallValidateCmdSetLineStippleEXT(commandBuffer, lineStippleFactor, lineStipplePattern);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCmdSetLineStippleEXT(commandBuffer, lineStippleFactor, lineStipplePattern); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCmdSetLineStippleEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    lineStippleFactor,
    uint16_t                                    lineStipplePattern) {
    ValidationStateTracker::PreCallRecordCmdSetLineStippleEXT(commandBuffer, lineStippleFactor, lineStipplePattern);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCmdSetLineStippleEXT(commandBuffer, lineStippleFactor, lineStipplePattern); });
}

void BestPractices::PostCallRecordCmdSetLineStippleEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    lineStippleFactor,
    uint16_t                                    lineStipplePattern) {
    ValidationStateTracker::PostCallRecordCmdSetLineStippleEXT(commandBuffer, lineStippleFactor, lineStipplePattern);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCmdSetLineStippleEXT(commandBuffer, lineStippleFactor, lineStipplePattern); });
}

bool BestPractices::PreCallValidateResetQueryPoolEXT(
    VkDevice                                    device,
    VkQueryPool                                 queryPool,
    uint32_t                                    firstQuery,
    uint32_t                                    queryCount) const {
    bool skip = ValidationStateTracker::PreCallValidateResetQueryPoolEXT(device, queryPool, firstQuery, queryCount);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateResetQueryPoolEXT(device, queryPool, firstQuery, queryCount); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordResetQueryPoolEXT(
    VkDevice                                    device,
    VkQueryPool                                 queryPool,
    uint32_t                                    firstQuery,
    uint32_t                                    queryCount) {
    ValidationStateTracker::PreCallRecordResetQueryPoolEXT(device, queryPool, firstQuery, queryCount);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordResetQueryPoolEXT(device, queryPool, firstQuery, queryCount); });
}

void BestPractices::PostCallRecordResetQueryPoolEXT(
    VkDevice                                    device,
    VkQueryPool                                 queryPool,
    uint32_t                                    firstQuery,
    uint32_t                                    queryCount) {
    ValidationStateTracker::PostCallRecordResetQueryPoolEXT(device, queryPool, firstQuery, queryCount);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordResetQueryPoolEXT(device, queryPool, firstQuery, queryCount); });
}

bool BestPractices::PreCallValidateGetGeneratedCommandsMemoryRequirementsNV(
    VkDevice                                    device,
    const VkGeneratedCommandsMemoryRequirementsInfoNV* pInfo,
    VkMemoryRequirements2*                      pMemoryRequirements) const {
    bool skip = ValidationStateTracker::PreCallValidateGetGeneratedCommandsMemoryRequirementsNV(device, pInfo, pMemoryRequirements);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetGeneratedCommandsMemoryRequirementsNV(device, pInfo, pMemoryRequirements); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetGeneratedCommandsMemoryRequirementsNV(
    VkDevice                                    device,
    const VkGeneratedCommandsMemoryRequirementsInfoNV* pInfo,
    VkMemoryRequirements2*                      pMemoryRequirements) {
    ValidationStateTracker::PreCallRecordGetGeneratedCommandsMemoryRequirementsNV(device, pInfo, pMemoryRequirements);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetGeneratedCommandsMemoryRequirementsNV(device, pInfo, pMemoryRequirements); });
}

void BestPractices::PostCallRecordGetGeneratedCommandsMemoryRequirementsNV(
    VkDevice                                    device,
    const VkGeneratedCommandsMemoryRequirementsInfoNV* pInfo,
    VkMemoryRequirements2*                      pMemoryRequirements) {
    ValidationStateTracker::PostCallRecordGetGeneratedCommandsMemoryRequirementsNV(device, pInfo, pMemoryRequirements);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetGeneratedCommandsMemoryRequirementsNV(device, pInfo, pMemoryRequirements); });
}

bool BestPractices::PreCallValidateCmdPreprocessGeneratedCommandsNV(
    VkCommandBuffer                             commandBuffer,
    const VkGeneratedCommandsInfoNV*            pGeneratedCommandsInfo) const {
    bool skip = ValidationStateTracker::PreCallValidateCmdPreprocessGeneratedCommandsNV(commandBuffer, pGeneratedCommandsInfo);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCmdPreprocessGeneratedCommandsNV(commandBuffer, pGeneratedCommandsInfo); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCmdPreprocessGeneratedCommandsNV(
    VkCommandBuffer                             commandBuffer,
    const VkGeneratedCommandsInfoNV*            pGeneratedCommandsInfo) {
    ValidationStateTracker::PreCallRecordCmdPreprocessGeneratedCommandsNV(commandBuffer, pGeneratedCommandsInfo);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCmdPreprocessGeneratedCommandsNV(commandBuffer, pGeneratedCommandsInfo); });
}

void BestPractices::PostCallRecordCmdPreprocessGeneratedCommandsNV(
    VkCommandBuffer                             commandBuffer,
    const VkGeneratedCommandsInfoNV*            pGeneratedCommandsInfo) {
    ValidationStateTracker::PostCallRecordCmdPreprocessGeneratedCommandsNV(commandBuffer, pGeneratedCommandsInfo);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCmdPreprocessGeneratedCommandsNV(commandBuffer, pGeneratedCommandsInfo); });
}

bool BestPractices::PreCallValidateCmdExecuteGeneratedCommandsNV(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    isPreprocessed,
    const VkGeneratedCommandsInfoNV*            pGeneratedCommandsInfo) const {
    bool skip = ValidationStateTracker::PreCallValidateCmdExecuteGeneratedCommandsNV(commandBuffer, isPreprocessed, pGeneratedCommandsInfo);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCmdExecuteGeneratedCommandsNV(commandBuffer, isPreprocessed, pGeneratedCommandsInfo); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCmdExecuteGeneratedCommandsNV(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    isPreprocessed,
    const VkGeneratedCommandsInfoNV*            pGeneratedCommandsInfo) {
    ValidationStateTracker::PreCallRecordCmdExecuteGeneratedCommandsNV(commandBuffer, isPreprocessed, pGeneratedCommandsInfo);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCmdExecuteGeneratedCommandsNV(commandBuffer, isPreprocessed, pGeneratedCommandsInfo); });
}

void BestPractices::PostCallRecordCmdExecuteGeneratedCommandsNV(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    isPreprocessed,
    const VkGeneratedCommandsInfoNV*            pGeneratedCommandsInfo) {
    ValidationStateTracker::PostCallRecordCmdExecuteGeneratedCommandsNV(commandBuffer, isPreprocessed, pGeneratedCommandsInfo);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCmdExecuteGeneratedCommandsNV(commandBuffer, isPreprocessed, pGeneratedCommandsInfo); });
}

bool BestPractices::PreCallValidateCmdBindPipelineShaderGroupNV(
    VkCommandBuffer                             commandBuffer,
    VkPipelineBindPoint                         pipelineBindPoint,
    VkPipeline                                  pipeline,
    uint32_t                                    groupIndex) const {
    bool skip = ValidationStateTracker::PreCallValidateCmdBindPipelineShaderGroupNV(commandBuffer, pipelineBindPoint, pipeline, groupIndex);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCmdBindPipelineShaderGroupNV(commandBuffer, pipelineBindPoint, pipeline, groupIndex); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCmdBindPipelineShaderGroupNV(
    VkCommandBuffer                             commandBuffer,
    VkPipelineBindPoint                         pipelineBindPoint,
    VkPipeline                                  pipeline,
    uint32_t                                    groupIndex) {
    ValidationStateTracker::PreCallRecordCmdBindPipelineShaderGroupNV(commandBuffer, pipelineBindPoint, pipeline, groupIndex);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCmdBindPipelineShaderGroupNV(commandBuffer, pipelineBindPoint, pipeline, groupIndex); });
}

void BestPractices::PostCallRecordCmdBindPipelineShaderGroupNV(
    VkCommandBuffer                             commandBuffer,
    VkPipelineBindPoint                         pipelineBindPoint,
    VkPipeline                                  pipeline,
    uint32_t                                    groupIndex) {
    ValidationStateTracker::PostCallRecordCmdBindPipelineShaderGroupNV(commandBuffer, pipelineBindPoint, pipeline, groupIndex);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCmdBindPipelineShaderGroupNV(commandBuffer, pipelineBindPoint, pipeline, groupIndex); });
}

bool BestPractices::PreCallValidateCreateIndirectCommandsLayoutNV(
    VkDevice                                    device,
    const VkIndirectCommandsLayoutCreateInfoNV* pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkIndirectCommandsLayoutNV*                 pIndirectCommandsLayout) const {
    bool skip = ValidationStateTracker::PreCallValidateCreateIndirectCommandsLayoutNV(device, pCreateInfo, pAllocator, pIndirectCommandsLayout);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCreateIndirectCommandsLayoutNV(device, pCreateInfo, pAllocator, pIndirectCommandsLayout); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCreateIndirectCommandsLayoutNV(
    VkDevice                                    device,
    const VkIndirectCommandsLayoutCreateInfoNV* pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkIndirectCommandsLayoutNV*                 pIndirectCommandsLayout) {
    ValidationStateTracker::PreCallRecordCreateIndirectCommandsLayoutNV(device, pCreateInfo, pAllocator, pIndirectCommandsLayout);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCreateIndirectCommandsLayoutNV(device, pCreateInfo, pAllocator, pIndirectCommandsLayout); });
}

void BestPractices::PostCallRecordCreateIndirectCommandsLayoutNV(
    VkDevice                                    device,
    const VkIndirectCommandsLayoutCreateInfoNV* pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkIndirectCommandsLayoutNV*                 pIndirectCommandsLayout,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCreateIndirectCommandsLayoutNV(device, pCreateInfo, pAllocator, pIndirectCommandsLayout, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCreateIndirectCommandsLayoutNV(device, pCreateInfo, pAllocator, pIndirectCommandsLayout, result); });
}

bool BestPractices::PreCallValidateDestroyIndirectCommandsLayoutNV(
    VkDevice                                    device,
    VkIndirectCommandsLayoutNV                  indirectCommandsLayout,
    const VkAllocationCallbacks*                pAllocator) const {
    bool skip = ValidationStateTracker::PreCallValidateDestroyIndirectCommandsLayoutNV(device, indirectCommandsLayout, pAllocator);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateDestroyIndirectCommandsLayoutNV(device, indirectCommandsLayout, pAllocator); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordDestroyIndirectCommandsLayoutNV(
    VkDevice                                    device,
    VkIndirectCommandsLayoutNV                  indirectCommandsLayout,
    const VkAllocationCallbacks*                pAllocator) {
    ValidationStateTracker::PreCallRecordDestroyIndirectCommandsLayoutNV(device, indirectCommandsLayout, pAllocator);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordDestroyIndirectCommandsLayoutNV(device, indirectCommandsLayout, pAllocator); });
}

void BestPractices::PostCallRecordDestroyIndirectCommandsLayoutNV(
    VkDevice                                    device,
    VkIndirectCommandsLayoutNV                  indirectCommandsLayout,
    const VkAllocationCallbacks*                pAllocator) {
    ValidationStateTracker::PostCallRecordDestroyIndirectCommandsLayoutNV(device, indirectCommandsLayout, pAllocator);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordDestroyIndirectCommandsLayoutNV(device, indirectCommandsLayout, pAllocator); });
}

bool BestPractices::PreCallValidateCreatePrivateDataSlotEXT(
    VkDevice                                    device,
    const VkPrivateDataSlotCreateInfoEXT*       pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkPrivateDataSlotEXT*                       pPrivateDataSlot) const {
    bool skip = ValidationStateTracker::PreCallValidateCreatePrivateDataSlotEXT(device, pCreateInfo, pAllocator, pPrivateDataSlot);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCreatePrivateDataSlotEXT(device, pCreateInfo, pAllocator, pPrivateDataSlot); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCreatePrivateDataSlotEXT(
    VkDevice                                    device,
    const VkPrivateDataSlotCreateInfoEXT*       pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkPrivateDataSlotEXT*                       pPrivateDataSlot) {
    ValidationStateTracker::PreCallRecordCreatePrivateDataSlotEXT(device, pCreateInfo, pAllocator, pPrivateDataSlot);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCreatePrivateDataSlotEXT(device, pCreateInfo, pAllocator, pPrivateDataSlot); });
}

void BestPractices::PostCallRecordCreatePrivateDataSlotEXT(
    VkDevice                                    device,
    const VkPrivateDataSlotCreateInfoEXT*       pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkPrivateDataSlotEXT*                       pPrivateDataSlot,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCreatePrivateDataSlotEXT(device, pCreateInfo, pAllocator, pPrivateDataSlot, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCreatePrivateDataSlotEXT(device, pCreateInfo, pAllocator, pPrivateDataSlot, result); });
}

bool BestPractices::PreCallValidateDestroyPrivateDataSlotEXT(
    VkDevice                                    device,
    VkPrivateDataSlotEXT                        privateDataSlot,
    const VkAllocationCallbacks*                pAllocator) const {
    bool skip = ValidationStateTracker::PreCallValidateDestroyPrivateDataSlotEXT(device, privateDataSlot, pAllocator);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateDestroyPrivateDataSlotEXT(device, privateDataSlot, pAllocator); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordDestroyPrivateDataSlotEXT(
    VkDevice                                    device,
    VkPrivateDataSlotEXT                        privateDataSlot,
    const VkAllocationCallbacks*                pAllocator) {
    ValidationStateTracker::PreCallRecordDestroyPrivateDataSlotEXT(device, privateDataSlot, pAllocator);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordDestroyPrivateDataSlotEXT(device, privateDataSlot, pAllocator); });
}

void BestPractices::PostCallRecordDestroyPrivateDataSlotEXT(
    VkDevice                                    device,
    VkPrivateDataSlotEXT                        privateDataSlot,
    const VkAllocationCallbacks*                pAllocator) {
    ValidationStateTracker::PostCallRecordDestroyPrivateDataSlotEXT(device, privateDataSlot, pAllocator);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordDestroyPrivateDataSlotEXT(device, privateDataSlot, pAllocator); });
}

bool BestPractices::PreCallValidateSetPrivateDataEXT(
    VkDevice                                    device,
    VkObjectType                                objectType,
    uint64_t                                    objectHandle,
    VkPrivateDataSlotEXT                        privateDataSlot,
    uint64_t                                    data) const {
    bool skip = ValidationStateTracker::PreCallValidateSetPrivateDataEXT(device, objectType, objectHandle, privateDataSlot, data);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateSetPrivateDataEXT(device, objectType, objectHandle, privateDataSlot, data); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordSetPrivateDataEXT(
    VkDevice                                    device,
    VkObjectType                                objectType,
    uint64_t                                    objectHandle,
    VkPrivateDataSlotEXT                        privateDataSlot,
    uint64_t                                    data) {
    ValidationStateTracker::PreCallRecordSetPrivateDataEXT(device, objectType, objectHandle, privateDataSlot, data);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordSetPrivateDataEXT(device, objectType, objectHandle, privateDataSlot, data); });
}

void BestPractices::PostCallRecordSetPrivateDataEXT(
    VkDevice                                    device,
    VkObjectType                                objectType,
    uint64_t                                    objectHandle,
    VkPrivateDataSlotEXT                        privateDataSlot,
    uint64_t                                    data,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordSetPrivateDataEXT(device, objectType, objectHandle, privateDataSlot, data, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordSetPrivateDataEXT(device, objectType, objectHandle, privateDataSlot, data, result); });
}

bool BestPractices::PreCallValidateGetPrivateDataEXT(
    VkDevice                                    device,
    VkObjectType                                objectType,
    uint64_t                                    objectHandle,
    VkPrivateDataSlotEXT                        privateDataSlot,
    uint64_t*                                   pData) const {
    bool skip = ValidationStateTracker::PreCallValidateGetPrivateDataEXT(device, objectType, objectHandle, privateDataSlot, pData);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetPrivateDataEXT(device, objectType, objectHandle, privateDataSlot, pData); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetPrivateDataEXT(
    VkDevice                                    device,
    VkObjectType                                objectType,
    uint64_t                                    objectHandle,
    VkPrivateDataSlotEXT                        privateDataSlot,
    uint64_t*                                   pData) {
    ValidationStateTracker::PreCallRecordGetPrivateDataEXT(device, objectType, objectHandle, privateDataSlot, pData);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetPrivateDataEXT(device, objectType, objectHandle, privateDataSlot, pData); });
}

void BestPractices::PostCallRecordGetPrivateDataEXT(
    VkDevice                                    device,
    VkObjectType                                objectType,
    uint64_t                                    objectHandle,
    VkPrivateDataSlotEXT                        privateDataSlot,
    uint64_t*                                   pData) {
    ValidationStateTracker::PostCallRecordGetPrivateDataEXT(device, objectType, objectHandle, privateDataSlot, pData);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetPrivateDataEXT(device, objectType, objectHandle, privateDataSlot, pData); });
}

bool BestPractices::PreCallValidateCreateAccelerationStructureKHR(
    VkDevice                                    device,
    const VkAccelerationStructureCreateInfoKHR* pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkAccelerationStructureKHR*                 pAccelerationStructure) const {
    bool skip = ValidationStateTracker::PreCallValidateCreateAccelerationStructureKHR(device, pCreateInfo, pAllocator, pAccelerationStructure);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCreateAccelerationStructureKHR(device, pCreateInfo, pAllocator, pAccelerationStructure); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCreateAccelerationStructureKHR(
    VkDevice                                    device,
    const VkAccelerationStructureCreateInfoKHR* pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkAccelerationStructureKHR*                 pAccelerationStructure) {
    ValidationStateTracker::PreCallRecordCreateAccelerationStructureKHR(device, pCreateInfo, pAllocator, pAccelerationStructure);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCreateAccelerationStructureKHR(device, pCreateInfo, pAllocator, pAccelerationStructure); });
}

void BestPractices::PostCallRecordCreateAccelerationStructureKHR(
    VkDevice                                    device,
    const VkAccelerationStructureCreateInfoKHR* pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkAccelerationStructureKHR*                 pAccelerationStructure,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCreateAccelerationStructureKHR(device, pCreateInfo, pAllocator, pAccelerationStructure, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCreateAccelerationStructureKHR(device, pCreateInfo, pAllocator, pAccelerationStructure, result); });
}

bool BestPractices::PreCallValidateGetAccelerationStructureMemoryRequirementsKHR(
    VkDevice                                    device,
    const VkAccelerationStructureMemoryRequirementsInfoKHR* pInfo,
    VkMemoryRequirements2*                      pMemoryRequirements) const {
    bool skip = ValidationStateTracker::PreCallValidateGetAccelerationStructureMemoryRequirementsKHR(device, pInfo, pMemoryRequirements);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetAccelerationStructureMemoryRequirementsKHR(device, pInfo, pMemoryRequirements); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetAccelerationStructureMemoryRequirementsKHR(
    VkDevice                                    device,
    const VkAccelerationStructureMemoryRequirementsInfoKHR* pInfo,
    VkMemoryRequirements2*                      pMemoryRequirements) {
    ValidationStateTracker::PreCallRecordGetAccelerationStructureMemoryRequirementsKHR(device, pInfo, pMemoryRequirements);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetAccelerationStructureMemoryRequirementsKHR(device, pInfo, pMemoryRequirements); });
}

void BestPractices::PostCallRecordGetAccelerationStructureMemoryRequirementsKHR(
    VkDevice                                    device,
    const VkAccelerationStructureMemoryRequirementsInfoKHR* pInfo,
    VkMemoryRequirements2*                      pMemoryRequirements) {
    ValidationStateTracker::PostCallRecordGetAccelerationStructureMemoryRequirementsKHR(device, pInfo, pMemoryRequirements);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetAccelerationStructureMemoryRequirementsKHR(device, pInfo, pMemoryRequirements); });
}

bool BestPractices::PreCallValidateCmdBuildAccelerationStructureKHR(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    infoCount,
    const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
    const VkAccelerationStructureBuildOffsetInfoKHR* const* ppOffsetInfos) const {
    bool skip = ValidationStateTracker::PreCallValidateCmdBuildAccelerationStructureKHR(commandBuffer, infoCount, pInfos, ppOffsetInfos);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCmdBuildAccelerationStructureKHR(commandBuffer, infoCount, pInfos, ppOffsetInfos); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCmdBuildAccelerationStructureKHR(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    infoCount,
    const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
    const VkAccelerationStructureBuildOffsetInfoKHR* const* ppOffsetInfos) {
    ValidationStateTracker::PreCallRecordCmdBuildAccelerationStructureKHR(commandBuffer, infoCount, pInfos, ppOffsetInfos);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCmdBuildAccelerationStructureKHR(commandBuffer, infoCount, pInfos, ppOffsetInfos); });
}

void BestPractices::PostCallRecordCmdBuildAccelerationStructureKHR(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    infoCount,
    const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
    const VkAccelerationStructureBuildOffsetInfoKHR* const* ppOffsetInfos) {
    ValidationStateTracker::PostCallRecordCmdBuildAccelerationStructureKHR(commandBuffer, infoCount, pInfos, ppOffsetInfos);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCmdBuildAccelerationStructureKHR(commandBuffer, infoCount, pInfos, ppOffsetInfos); });
}

bool BestPractices::PreCallValidateCmdBuildAccelerationStructureIndirectKHR(
    VkCommandBuffer                             commandBuffer,
    const VkAccelerationStructureBuildGeometryInfoKHR* pInfo,
    VkBuffer                                    indirectBuffer,
    VkDeviceSize                                indirectOffset,
    uint32_t                                    indirectStride) const {
    bool skip = ValidationStateTracker::PreCallValidateCmdBuildAccelerationStructureIndirectKHR(commandBuffer, pInfo, indirectBuffer, indirectOffset, indirectStride);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCmdBuildAccelerationStructureIndirectKHR(commandBuffer, pInfo, indirectBuffer, indirectOffset, indirectStride); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCmdBuildAccelerationStructureIndirectKHR(
    VkCommandBuffer                             commandBuffer,
    const VkAccelerationStructureBuildGeometryInfoKHR* pInfo,
    VkBuffer                                    indirectBuffer,
    VkDeviceSize                                indirectOffset,
    uint32_t                                    indirectStride) {
    ValidationStateTracker::PreCallRecordCmdBuildAccelerationStructureIndirectKHR(commandBuffer, pInfo, indirectBuffer, indirectOffset, indirectStride);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCmdBuildAccelerationStructureIndirectKHR(commandBuffer, pInfo, indirectBuffer, indirectOffset, indirectStride); });
}

void BestPractices::PostCallRecordCmdBuildAccelerationStructureIndirectKHR(
    VkCommandBuffer                             commandBuffer,
    const VkAccelerationStructureBuildGeometryInfoKHR* pInfo,
    VkBuffer                                    indirectBuffer,
    VkDeviceSize                                indirectOffset,
    uint32_t                                    indirectStride) {
    ValidationStateTracker::PostCallRecordCmdBuildAccelerationStructureIndirectKHR(commandBuffer, pInfo, indirectBuffer, indirectOffset, indirectStride);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCmdBuildAccelerationStructureIndirectKHR(commandBuffer, pInfo, indirectBuffer, indirectOffset, indirectStride); });
}

bool BestPractices::PreCallValidateBuildAccelerationStructureKHR(
    VkDevice                                    device,
    uint32_t                                    infoCount,
    const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
    const VkAccelerationStructureBuildOffsetInfoKHR* const* ppOffsetInfos) const {
    bool skip = ValidationStateTracker::PreCallValidateBuildAccelerationStructureKHR(device, infoCount, pInfos, ppOffsetInfos);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateBuildAccelerationStructureKHR(device, infoCount, pInfos, ppOffsetInfos); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordBuildAccelerationStructureKHR(
    VkDevice                                    device,
    uint32_t                                    infoCount,
    const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
    const VkAccelerationStructureBuildOffsetInfoKHR* const* ppOffsetInfos) {
    ValidationStateTracker::PreCallRecordBuildAccelerationStructureKHR(device, infoCount, pInfos, ppOffsetInfos);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordBuildAccelerationStructureKHR(device, infoCount, pInfos, ppOffsetInfos); });
}

void BestPractices::PostCallRecordBuildAccelerationStructureKHR(
    VkDevice                                    device,
    uint32_t                                    infoCount,
    const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
    const VkAccelerationStructureBuildOffsetInfoKHR* const* ppOffsetInfos,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordBuildAccelerationStructureKHR(device, infoCount, pInfos, ppOffsetInfos, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordBuildAccelerationStructureKHR(device, infoCount, pInfos, ppOffsetInfos, result); });
}

bool BestPractices::PreCallValidateCopyAccelerationStructureKHR(
    VkDevice                                    device,
    const VkCopyAccelerationStructureInfoKHR*   pInfo) const {
    bool skip = ValidationStateTracker::PreCallValidateCopyAccelerationStructureKHR(device, pInfo);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCopyAccelerationStructureKHR(device, pInfo); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCopyAccelerationStructureKHR(
    VkDevice                                    device,
    const VkCopyAccelerationStructureInfoKHR*   pInfo) {
    ValidationStateTracker::PreCallRecordCopyAccelerationStructureKHR(device, pInfo);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCopyAccelerationStructureKHR(device, pInfo); });
}

void BestPractices::PostCallRecordCopyAccelerationStructureKHR(
    VkDevice                                    device,
    const VkCopyAccelerationStructureInfoKHR*   pInfo,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCopyAccelerationStructureKHR(device, pInfo, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCopyAccelerationStructureKHR(device, pInfo, result); });
}

bool BestPractices::PreCallValidateCopyAccelerationStructureToMemoryKHR(
    VkDevice                                    device,
    const VkCopyAccelerationStructureToMemoryInfoKHR* pInfo) const {
    bool skip = ValidationStateTracker::PreCallValidateCopyAccelerationStructureToMemoryKHR(device, pInfo);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCopyAccelerationStructureToMemoryKHR(device, pInfo); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCopyAccelerationStructureToMemoryKHR(
    VkDevice                                    device,
    const VkCopyAccelerationStructureToMemoryInfoKHR* pInfo) {
    ValidationStateTracker::PreCallRecordCopyAccelerationStructureToMemoryKHR(device, pInfo);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCopyAccelerationStructureToMemoryKHR(device, pInfo); });
}

void BestPractices::PostCallRecordCopyAccelerationStructureToMemoryKHR(
    VkDevice                                    device,
    const VkCopyAccelerationStructureToMemoryInfoKHR* pInfo,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCopyAccelerationStructureToMemoryKHR(device, pInfo, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCopyAccelerationStructureToMemoryKHR(device, pInfo, result); });
}

bool BestPractices::PreCallValidateCopyMemoryToAccelerationStructureKHR(
    VkDevice                                    device,
    const VkCopyMemoryToAccelerationStructureInfoKHR* pInfo) const {
    bool skip = ValidationStateTracker::PreCallValidateCopyMemoryToAccelerationStructureKHR(device, pInfo);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCopyMemoryToAccelerationStructureKHR(device, pInfo); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCopyMemoryToAccelerationStructureKHR(
    VkDevice                                    device,
    const VkCopyMemoryToAccelerationStructureInfoKHR* pInfo) {
    ValidationStateTracker::PreCallRecordCopyMemoryToAccelerationStructureKHR(device, pInfo);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCopyMemoryToAccelerationStructureKHR(device, pInfo); });
}

void BestPractices::PostCallRecordCopyMemoryToAccelerationStructureKHR(
    VkDevice                                    device,
    const VkCopyMemoryToAccelerationStructureInfoKHR* pInfo,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCopyMemoryToAccelerationStructureKHR(device, pInfo, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCopyMemoryToAccelerationStructureKHR(device, pInfo, result); });
}

bool BestPractices::PreCallValidateWriteAccelerationStructuresPropertiesKHR(
    VkDevice                                    device,
    uint32_t                                    accelerationStructureCount,
    const VkAccelerationStructureKHR*           pAccelerationStructures,
    VkQueryType                                 queryType,
    size_t                                      dataSize,
    void*                                       pData,
    size_t                                      stride) const {
    bool skip = ValidationStateTracker::PreCallValidateWriteAccelerationStructuresPropertiesKHR(device, accelerationStructureCount, pAccelerationStructures, queryType, dataSize, pData, stride);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateWriteAccelerationStructuresPropertiesKHR(device, accelerationStructureCount, pAccelerationStructures, queryType, dataSize, pData, stride); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordWriteAccelerationStructuresPropertiesKHR(
    VkDevice                                    device,
    uint32_t                                    accelerationStructureCount,
    const VkAccelerationStructureKHR*           pAccelerationStructures,
    VkQueryType                                 queryType,
    size_t                                      dataSize,
    void*                                       pData,
    size_t                                      stride) {
    ValidationStateTracker::PreCallRecordWriteAccelerationStructuresPropertiesKHR(device, accelerationStructureCount, pAccelerationStructures, queryType, dataSize, pData, stride);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordWriteAccelerationStructuresPropertiesKHR(device, accelerationStructureCount, pAccelerationStructures, queryType, dataSize, pData, stride); });
}

void BestPractices::PostCallRecordWriteAccelerationStructuresPropertiesKHR(
    VkDevice                                    device,
    uint32_t                                    accelerationStructureCount,
    const VkAccelerationStructureKHR*           pAccelerationStructures,
    VkQueryType                                 queryType,
    size_t                                      dataSize,
    void*                                       pData,
    size_t                                      stride,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordWriteAccelerationStructuresPropertiesKHR(device, accelerationStructureCount, pAccelerationStructures, queryType, dataSize, pData, stride, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordWriteAccelerationStructuresPropertiesKHR(device, accelerationStructureCount, pAccelerationStructures, queryType, dataSize, pData, stride, result); });
}

bool BestPractices::PreCallValidateCmdCopyAccelerationStructureKHR(
    VkCommandBuffer                             commandBuffer,
    const VkCopyAccelerationStructureInfoKHR*   pInfo) const {
    bool skip = ValidationStateTracker::PreCallValidateCmdCopyAccelerationStructureKHR(commandBuffer, pInfo);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCmdCopyAccelerationStructureKHR(commandBuffer, pInfo); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCmdCopyAccelerationStructureKHR(
    VkCommandBuffer                             commandBuffer,
    const VkCopyAccelerationStructureInfoKHR*   pInfo) {
    ValidationStateTracker::PreCallRecordCmdCopyAccelerationStructureKHR(commandBuffer, pInfo);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCmdCopyAccelerationStructureKHR(commandBuffer, pInfo); });
}

void BestPractices::PostCallRecordCmdCopyAccelerationStructureKHR(
    VkCommandBuffer                             commandBuffer,
    const VkCopyAccelerationStructureInfoKHR*   pInfo) {
    ValidationStateTracker::PostCallRecordCmdCopyAccelerationStructureKHR(commandBuffer, pInfo);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCmdCopyAccelerationStructureKHR(commandBuffer, pInfo); });
}

bool BestPractices::PreCallValidateCmdCopyAccelerationStructureToMemoryKHR(
    VkCommandBuffer                             commandBuffer,
    const VkCopyAccelerationStructureToMemoryInfoKHR* pInfo) const {
    bool skip = ValidationStateTracker::PreCallValidateCmdCopyAccelerationStructureToMemoryKHR(commandBuffer, pInfo);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCmdCopyAccelerationStructureToMemoryKHR(commandBuffer, pInfo); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCmdCopyAccelerationStructureToMemoryKHR(
    VkCommandBuffer                             commandBuffer,
    const VkCopyAccelerationStructureToMemoryInfoKHR* pInfo) {
    ValidationStateTracker::PreCallRecordCmdCopyAccelerationStructureToMemoryKHR(commandBuffer, pInfo);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCmdCopyAccelerationStructureToMemoryKHR(commandBuffer, pInfo); });
}

void BestPractices::PostCallRecordCmdCopyAccelerationStructureToMemoryKHR(
    VkCommandBuffer                             commandBuffer,
    const VkCopyAccelerationStructureToMemoryInfoKHR* pInfo) {
    ValidationStateTracker::PostCallRecordCmdCopyAccelerationStructureToMemoryKHR(commandBuffer, pInfo);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCmdCopyAccelerationStructureToMemoryKHR(commandBuffer, pInfo); });
}

bool BestPractices::PreCallValidateCmdCopyMemoryToAccelerationStructureKHR(
    VkCommandBuffer                             commandBuffer,
    const VkCopyMemoryToAccelerationStructureInfoKHR* pInfo) const {
    bool skip = ValidationStateTracker::PreCallValidateCmdCopyMemoryToAccelerationStructureKHR(commandBuffer, pInfo);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCmdCopyMemoryToAccelerationStructureKHR(commandBuffer, pInfo); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCmdCopyMemoryToAccelerationStructureKHR(
    VkCommandBuffer                             commandBuffer,
    const VkCopyMemoryToAccelerationStructureInfoKHR* pInfo) {
    ValidationStateTracker::PreCallRecordCmdCopyMemoryToAccelerationStructureKHR(commandBuffer, pInfo);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCmdCopyMemoryToAccelerationStructureKHR(commandBuffer, pInfo); });
}

void BestPractices::PostCallRecordCmdCopyMemoryToAccelerationStructureKHR(
    VkCommandBuffer                             commandBuffer,
    const VkCopyMemoryToAccelerationStructureInfoKHR* pInfo) {
    ValidationStateTracker::PostCallRecordCmdCopyMemoryToAccelerationStructureKHR(commandBuffer, pInfo);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCmdCopyMemoryToAccelerationStructureKHR(commandBuffer, pInfo); });
}

bool BestPractices::PreCallValidateCmdTraceRaysKHR(
    VkCommandBuffer                             commandBuffer,
    const VkStridedBufferRegionKHR*             pRaygenShaderBindingTable,
    const VkStridedBufferRegionKHR*             pMissShaderBindingTable,
    const VkStridedBufferRegionKHR*             pHitShaderBindingTable,
    const VkStridedBufferRegionKHR*             pCallableShaderBindingTable,
    uint32_t                                    width,
    uint32_t                                    height,
    uint32_t                                    depth) const {
    bool skip = ValidationStateTracker::PreCallValidateCmdTraceRaysKHR(commandBuffer, pRaygenShaderBindingTable, pMissShaderBindingTable, pHitShaderBindingTable, pCallableShaderBindingTable, width, height, depth);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCmdTraceRaysKHR(commandBuffer, pRaygenShaderBindingTable, pMissShaderBindingTable, pHitShaderBindingTable, pCallableShaderBindingTable, width, height, depth); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCmdTraceRaysKHR(
    VkCommandBuffer                             commandBuffer,
    const VkStridedBufferRegionKHR*             pRaygenShaderBindingTable,
    const VkStridedBufferRegionKHR*             pMissShaderBindingTable,
    const VkStridedBufferRegionKHR*             pHitShaderBindingTable,
    const VkStridedBufferRegionKHR*             pCallableShaderBindingTable,
    uint32_t                                    width,
    uint32_t                                    height,
    uint32_t                                    depth) {
    ValidationStateTracker::PreCallRecordCmdTraceRaysKHR(commandBuffer, pRaygenShaderBindingTable, pMissShaderBindingTable, pHitShaderBindingTable, pCallableShaderBindingTable, width, height, depth);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCmdTraceRaysKHR(commandBuffer, pRaygenShaderBindingTable, pMissShaderBindingTable, pHitShaderBindingTable, pCallableShaderBindingTable, width, height, depth); });
}

void BestPractices::PostCallRecordCmdTraceRaysKHR(
    VkCommandBuffer                             commandBuffer,
    const VkStridedBufferRegionKHR*             pRaygenShaderBindingTable,
    const VkStridedBufferRegionKHR*             pMissShaderBindingTable,
    const VkStridedBufferRegionKHR*             pHitShaderBindingTable,
    const VkStridedBufferRegionKHR*             pCallableShaderBindingTable,
    uint32_t                                    width,
    uint32_t                                    height,
    uint32_t                                    depth) {
    ValidationStateTracker::PostCallRecordCmdTraceRaysKHR(commandBuffer, pRaygenShaderBindingTable, pMissShaderBindingTable, pHitShaderBindingTable, pCallableShaderBindingTable, width, height, depth);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCmdTraceRaysKHR(commandBuffer, pRaygenShaderBindingTable, pMissShaderBindingTable, pHitShaderBindingTable, pCallableShaderBindingTable, width, height, depth); });
}

bool BestPractices::PreCallValidateCreateRayTracingPipelinesKHR(
    VkDevice                                    device,
    VkPipelineCache                             pipelineCache,
    uint32_t                                    createInfoCount,
    const VkRayTracingPipelineCreateInfoKHR*    pCreateInfos,
    const VkAllocationCallbacks*                pAllocator,
    VkPipeline*                                 pPipelines,
    void*                                       state_data) const {
    bool skip = ValidationStateTracker::PreCallValidateCreateRayTracingPipelinesKHR(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines, state_data);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCreateRayTracingPipelinesKHR(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines, state_data); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCreateRayTracingPipelinesKHR(
    VkDevice                                    device,
    VkPipelineCache                             pipelineCache,
    uint32_t                                    createInfoCount,
    const VkRayTracingPipelineCreateInfoKHR*    pCreateInfos,
    const VkAllocationCallbacks*                pAllocator,
    VkPipeline*                                 pPipelines,
    void*                                       state_data) {
    ValidationStateTracker::PreCallRecordCreateRayTracingPipelinesKHR(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines, state_data);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCreateRayTracingPipelinesKHR(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines, state_data); });
}

void BestPractices::PostCallRecordCreateRayTracingPipelinesKHR(
    VkDevice                                    device,
    VkPipelineCache                             pipelineCache,
    uint32_t                                    createInfoCount,
    const VkRayTracingPipelineCreateInfoKHR*    pCreateInfos,
    const VkAllocationCallbacks*                pAllocator,
    VkPipeline*                                 pPipelines,
    VkResult                                    result,
    void*                                       state_data) {
    ValidationStateTracker::PostCallRecordCreateRayTracingPipelinesKHR(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines, result, state_data);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCreateRayTracingPipelinesKHR(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines, result, state_data); });
}

bool BestPractices::PreCallValidateGetAccelerationStructureDeviceAddressKHR(
    VkDevice                                    device,
    const VkAccelerationStructureDeviceAddressInfoKHR* pInfo) const {
    bool skip = ValidationStateTracker::PreCallValidateGetAccelerationStructureDeviceAddressKHR(device, pInfo);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetAccelerationStructureDeviceAddressKHR(device, pInfo); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetAccelerationStructureDeviceAddressKHR(
    VkDevice                                    device,
    const VkAccelerationStructureDeviceAddressInfoKHR* pInfo) {
    ValidationStateTracker::PreCallRecordGetAccelerationStructureDeviceAddressKHR(device, pInfo);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetAccelerationStructureDeviceAddressKHR(device, pInfo); });
}

void BestPractices::PostCallRecordGetAccelerationStructureDeviceAddressKHR(
    VkDevice                                    device,
    const VkAccelerationStructureDeviceAddressInfoKHR* pInfo,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetAccelerationStructureDeviceAddressKHR(device, pInfo, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetAccelerationStructureDeviceAddressKHR(device, pInfo, result); });
}

bool BestPractices::PreCallValidateGetRayTracingCaptureReplayShaderGroupHandlesKHR(
    VkDevice                                    device,
    VkPipeline                                  pipeline,
    uint32_t                                    firstGroup,
    uint32_t                                    groupCount,
    size_t                                      dataSize,
    void*                                       pData) const {
    bool skip = ValidationStateTracker::PreCallValidateGetRayTracingCaptureReplayShaderGroupHandlesKHR(device, pipeline, firstGroup, groupCount, dataSize, pData);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetRayTracingCaptureReplayShaderGroupHandlesKHR(device, pipeline, firstGroup, groupCount, dataSize, pData); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetRayTracingCaptureReplayShaderGroupHandlesKHR(
    VkDevice                                    device,
    VkPipeline                                  pipeline,
    uint32_t                                    firstGroup,
    uint32_t                                    groupCount,
    size_t                                      dataSize,
    void*                                       pData) {
    ValidationStateTracker::PreCallRecordGetRayTracingCaptureReplayShaderGroupHandlesKHR(device, pipeline, firstGroup, groupCount, dataSize, pData);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetRayTracingCaptureReplayShaderGroupHandlesKHR(device, pipeline, firstGroup, groupCount, dataSize, pData); });
}

void BestPractices::PostCallRecordGetRayTracingCaptureReplayShaderGroupHandlesKHR(
    VkDevice                                    device,
    VkPipeline                                  pipeline,
    uint32_t                                    firstGroup,
    uint32_t                                    groupCount,
    size_t                                      dataSize,
    void*                                       pData,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetRayTracingCaptureReplayShaderGroupHandlesKHR(device, pipeline, firstGroup, groupCount, dataSize, pData, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetRayTracingCaptureReplayShaderGroupHandlesKHR(device, pipeline, firstGroup, groupCount, dataSize, pData, result); });
}

bool BestPractices::PreCallValidateCmdTraceRaysIndirectKHR(
    VkCommandBuffer                             commandBuffer,
    const VkStridedBufferRegionKHR*             pRaygenShaderBindingTable,
    const VkStridedBufferRegionKHR*             pMissShaderBindingTable,
    const VkStridedBufferRegionKHR*             pHitShaderBindingTable,
    const VkStridedBufferRegionKHR*             pCallableShaderBindingTable,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset) const {
    bool skip = ValidationStateTracker::PreCallValidateCmdTraceRaysIndirectKHR(commandBuffer, pRaygenShaderBindingTable, pMissShaderBindingTable, pHitShaderBindingTable, pCallableShaderBindingTable, buffer, offset);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateCmdTraceRaysIndirectKHR(commandBuffer, pRaygenShaderBindingTable, pMissShaderBindingTable, pHitShaderBindingTable, pCallableShaderBindingTable, buffer, offset); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordCmdTraceRaysIndirectKHR(
    VkCommandBuffer                             commandBuffer,
    const VkStridedBufferRegionKHR*             pRaygenShaderBindingTable,
    const VkStridedBufferRegionKHR*             pMissShaderBindingTable,
    const VkStridedBufferRegionKHR*             pHitShaderBindingTable,
    const VkStridedBufferRegionKHR*             pCallableShaderBindingTable,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset) {
    ValidationStateTracker::PreCallRecordCmdTraceRaysIndirectKHR(commandBuffer, pRaygenShaderBindingTable, pMissShaderBindingTable, pHitShaderBindingTable, pCallableShaderBindingTable, buffer, offset);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordCmdTraceRaysIndirectKHR(commandBuffer, pRaygenShaderBindingTable, pMissShaderBindingTable, pHitShaderBindingTable, pCallableShaderBindingTable, buffer, offset); });
}

void BestPractices::PostCallRecordCmdTraceRaysIndirectKHR(
    VkCommandBuffer                             commandBuffer,
    const VkStridedBufferRegionKHR*             pRaygenShaderBindingTable,
    const VkStridedBufferRegionKHR*             pMissShaderBindingTable,
    const VkStridedBufferRegionKHR*             pHitShaderBindingTable,
    const VkStridedBufferRegionKHR*             pCallableShaderBindingTable,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset) {
    ValidationStateTracker::PostCallRecordCmdTraceRaysIndirectKHR(commandBuffer, pRaygenShaderBindingTable, pMissShaderBindingTable, pHitShaderBindingTable, pCallableShaderBindingTable, buffer, offset);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordCmdTraceRaysIndirectKHR(commandBuffer, pRaygenShaderBindingTable, pMissShaderBindingTable, pHitShaderBindingTable, pCallableShaderBindingTable, buffer, offset); });
}

bool BestPractices::PreCallValidateGetDeviceAccelerationStructureCompatibilityKHR(
    VkDevice                                    device,
    const VkAccelerationStructureVersionKHR*    version) const {
    bool skip = ValidationStateTracker::PreCallValidateGetDeviceAccelerationStructureCompatibilityKHR(device, version);
    std::function<bool(BestPracticeBase&)> f = [=](BestPracticeBase& practice) { return practice.PreCallValidateGetDeviceAccelerationStructureCompatibilityKHR(device, version); };
    skip |= foreachPractice(f);
    return skip;
}

void BestPractices::PreCallRecordGetDeviceAccelerationStructureCompatibilityKHR(
    VkDevice                                    device,
    const VkAccelerationStructureVersionKHR*    version) {
    ValidationStateTracker::PreCallRecordGetDeviceAccelerationStructureCompatibilityKHR(device, version);
    foreachPractice([=](BestPracticeBase& practice) { practice.PreCallRecordGetDeviceAccelerationStructureCompatibilityKHR(device, version); });
}

void BestPractices::PostCallRecordGetDeviceAccelerationStructureCompatibilityKHR(
    VkDevice                                    device,
    const VkAccelerationStructureVersionKHR*    version,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetDeviceAccelerationStructureCompatibilityKHR(device, version, result);
    foreachPractice([=](BestPracticeBase& practice) { practice.PostCallRecordGetDeviceAccelerationStructureCompatibilityKHR(device, version, result); });
}





