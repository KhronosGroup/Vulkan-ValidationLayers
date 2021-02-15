// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See helper_file_generator.py for modifications


/***************************************************************************
 *
 * Copyright (c) 2015-2021 The Khronos Group Inc.
 * Copyright (c) 2015-2021 Valve Corporation
 * Copyright (c) 2015-2021 LunarG, Inc.
 * Copyright (c) 2015-2021 Google Inc.
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
 * Author: Courtney Goeltzenleuchter <courtneygo@google.com>
 * Author: Tobin Ehlis <tobine@google.com>
 * Author: Chris Forbes <chrisforbes@google.com>
 * Author: John Zulauf<jzulauf@lunarg.com>
 *
 ****************************************************************************/



#include "core_validation.h"
#include "corechecks_optick_instrumentation.h"

#ifdef INSTRUMENT_OPTICK

// Manually written intercepts
void CoreChecksOptickInstrumented::PostCallRecordQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* pPresentInfo, VkResult result) {
    OPTICK_FRAME("CPU FRAME");
    OPTICK_EVENT();
    CoreChecks::PostCallRecordQueuePresentKHR(queue, pPresentInfo, result);
};

bool CoreChecksOptickInstrumented::PreCallValidateQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* pPresentInfo) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateQueuePresentKHR(queue, pPresentInfo);
    return result;
};

void CoreChecksOptickInstrumented::PreCallRecordQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* pPresentInfo) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordQueuePresentKHR(queue, pPresentInfo);
};

// Code-generated intercepts
bool CoreChecksOptickInstrumented::PreCallValidateCreateInstance(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkInstance* pInstance) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCreateInstance(pCreateInfo, pAllocator, pInstance);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCreateInstance(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkInstance* pInstance) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCreateInstance(pCreateInfo, pAllocator, pInstance);
}

void CoreChecksOptickInstrumented::PostCallRecordCreateInstance(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkInstance* pInstance, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCreateInstance(pCreateInfo, pAllocator, pInstance, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateDestroyInstance(VkInstance instance, const VkAllocationCallbacks* pAllocator) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateDestroyInstance(instance, pAllocator);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordDestroyInstance(VkInstance instance, const VkAllocationCallbacks* pAllocator) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordDestroyInstance(instance, pAllocator);
}

void CoreChecksOptickInstrumented::PostCallRecordDestroyInstance(VkInstance instance, const VkAllocationCallbacks* pAllocator) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordDestroyInstance(instance, pAllocator);
}

bool CoreChecksOptickInstrumented::PreCallValidateEnumeratePhysicalDevices(VkInstance instance, uint32_t* pPhysicalDeviceCount, VkPhysicalDevice* pPhysicalDevices) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateEnumeratePhysicalDevices(instance, pPhysicalDeviceCount, pPhysicalDevices);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordEnumeratePhysicalDevices(VkInstance instance, uint32_t* pPhysicalDeviceCount, VkPhysicalDevice* pPhysicalDevices) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordEnumeratePhysicalDevices(instance, pPhysicalDeviceCount, pPhysicalDevices);
}

void CoreChecksOptickInstrumented::PostCallRecordEnumeratePhysicalDevices(VkInstance instance, uint32_t* pPhysicalDeviceCount, VkPhysicalDevice* pPhysicalDevices, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordEnumeratePhysicalDevices(instance, pPhysicalDeviceCount, pPhysicalDevices, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetPhysicalDeviceFeatures(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures* pFeatures) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetPhysicalDeviceFeatures(physicalDevice, pFeatures);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetPhysicalDeviceFeatures(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures* pFeatures) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetPhysicalDeviceFeatures(physicalDevice, pFeatures);
}

void CoreChecksOptickInstrumented::PostCallRecordGetPhysicalDeviceFeatures(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures* pFeatures) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetPhysicalDeviceFeatures(physicalDevice, pFeatures);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetPhysicalDeviceFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format, VkFormatProperties* pFormatProperties) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetPhysicalDeviceFormatProperties(physicalDevice, format, pFormatProperties);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetPhysicalDeviceFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format, VkFormatProperties* pFormatProperties) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetPhysicalDeviceFormatProperties(physicalDevice, format, pFormatProperties);
}

void CoreChecksOptickInstrumented::PostCallRecordGetPhysicalDeviceFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format, VkFormatProperties* pFormatProperties) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetPhysicalDeviceFormatProperties(physicalDevice, format, pFormatProperties);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetPhysicalDeviceImageFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type, VkImageTiling tiling, VkImageUsageFlags usage, VkImageCreateFlags flags, VkImageFormatProperties* pImageFormatProperties) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetPhysicalDeviceImageFormatProperties(physicalDevice, format, type, tiling, usage, flags, pImageFormatProperties);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetPhysicalDeviceImageFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type, VkImageTiling tiling, VkImageUsageFlags usage, VkImageCreateFlags flags, VkImageFormatProperties* pImageFormatProperties) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetPhysicalDeviceImageFormatProperties(physicalDevice, format, type, tiling, usage, flags, pImageFormatProperties);
}

void CoreChecksOptickInstrumented::PostCallRecordGetPhysicalDeviceImageFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type, VkImageTiling tiling, VkImageUsageFlags usage, VkImageCreateFlags flags, VkImageFormatProperties* pImageFormatProperties, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetPhysicalDeviceImageFormatProperties(physicalDevice, format, type, tiling, usage, flags, pImageFormatProperties, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetPhysicalDeviceProperties(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties* pProperties) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetPhysicalDeviceProperties(physicalDevice, pProperties);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetPhysicalDeviceProperties(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties* pProperties) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetPhysicalDeviceProperties(physicalDevice, pProperties);
}

void CoreChecksOptickInstrumented::PostCallRecordGetPhysicalDeviceProperties(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties* pProperties) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetPhysicalDeviceProperties(physicalDevice, pProperties);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount, VkQueueFamilyProperties* pQueueFamilyProperties) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetPhysicalDeviceQueueFamilyProperties(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount, VkQueueFamilyProperties* pQueueFamilyProperties) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetPhysicalDeviceQueueFamilyProperties(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
}

void CoreChecksOptickInstrumented::PostCallRecordGetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount, VkQueueFamilyProperties* pQueueFamilyProperties) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetPhysicalDeviceQueueFamilyProperties(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetPhysicalDeviceMemoryProperties(VkPhysicalDevice physicalDevice, VkPhysicalDeviceMemoryProperties* pMemoryProperties) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetPhysicalDeviceMemoryProperties(physicalDevice, pMemoryProperties);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetPhysicalDeviceMemoryProperties(VkPhysicalDevice physicalDevice, VkPhysicalDeviceMemoryProperties* pMemoryProperties) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetPhysicalDeviceMemoryProperties(physicalDevice, pMemoryProperties);
}

void CoreChecksOptickInstrumented::PostCallRecordGetPhysicalDeviceMemoryProperties(VkPhysicalDevice physicalDevice, VkPhysicalDeviceMemoryProperties* pMemoryProperties) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetPhysicalDeviceMemoryProperties(physicalDevice, pMemoryProperties);
}

bool CoreChecksOptickInstrumented::PreCallValidateCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDevice* pDevice) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCreateDevice(physicalDevice, pCreateInfo, pAllocator, pDevice);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDevice* pDevice, void* extra_data) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCreateDevice(physicalDevice, pCreateInfo, pAllocator, pDevice, extra_data);
}

void CoreChecksOptickInstrumented::PostCallRecordCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDevice* pDevice, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCreateDevice(physicalDevice, pCreateInfo, pAllocator, pDevice, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateDestroyDevice(VkDevice device, const VkAllocationCallbacks* pAllocator) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateDestroyDevice(device, pAllocator);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordDestroyDevice(VkDevice device, const VkAllocationCallbacks* pAllocator) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordDestroyDevice(device, pAllocator);
}

void CoreChecksOptickInstrumented::PostCallRecordDestroyDevice(VkDevice device, const VkAllocationCallbacks* pAllocator) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordDestroyDevice(device, pAllocator);
}

bool CoreChecksOptickInstrumented::PreCallValidateEnumerateInstanceExtensionProperties(const char* pLayerName, uint32_t* pPropertyCount, VkExtensionProperties* pProperties) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateEnumerateInstanceExtensionProperties(pLayerName, pPropertyCount, pProperties);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordEnumerateInstanceExtensionProperties(const char* pLayerName, uint32_t* pPropertyCount, VkExtensionProperties* pProperties) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordEnumerateInstanceExtensionProperties(pLayerName, pPropertyCount, pProperties);
}

void CoreChecksOptickInstrumented::PostCallRecordEnumerateInstanceExtensionProperties(const char* pLayerName, uint32_t* pPropertyCount, VkExtensionProperties* pProperties, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordEnumerateInstanceExtensionProperties(pLayerName, pPropertyCount, pProperties, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateEnumerateDeviceExtensionProperties(VkPhysicalDevice physicalDevice, const char* pLayerName, uint32_t* pPropertyCount, VkExtensionProperties* pProperties) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateEnumerateDeviceExtensionProperties(physicalDevice, pLayerName, pPropertyCount, pProperties);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordEnumerateDeviceExtensionProperties(VkPhysicalDevice physicalDevice, const char* pLayerName, uint32_t* pPropertyCount, VkExtensionProperties* pProperties) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordEnumerateDeviceExtensionProperties(physicalDevice, pLayerName, pPropertyCount, pProperties);
}

void CoreChecksOptickInstrumented::PostCallRecordEnumerateDeviceExtensionProperties(VkPhysicalDevice physicalDevice, const char* pLayerName, uint32_t* pPropertyCount, VkExtensionProperties* pProperties, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordEnumerateDeviceExtensionProperties(physicalDevice, pLayerName, pPropertyCount, pProperties, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateEnumerateInstanceLayerProperties(uint32_t* pPropertyCount, VkLayerProperties* pProperties) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateEnumerateInstanceLayerProperties(pPropertyCount, pProperties);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordEnumerateInstanceLayerProperties(uint32_t* pPropertyCount, VkLayerProperties* pProperties) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordEnumerateInstanceLayerProperties(pPropertyCount, pProperties);
}

void CoreChecksOptickInstrumented::PostCallRecordEnumerateInstanceLayerProperties(uint32_t* pPropertyCount, VkLayerProperties* pProperties, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordEnumerateInstanceLayerProperties(pPropertyCount, pProperties, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateEnumerateDeviceLayerProperties(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount, VkLayerProperties* pProperties) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateEnumerateDeviceLayerProperties(physicalDevice, pPropertyCount, pProperties);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordEnumerateDeviceLayerProperties(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount, VkLayerProperties* pProperties) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordEnumerateDeviceLayerProperties(physicalDevice, pPropertyCount, pProperties);
}

void CoreChecksOptickInstrumented::PostCallRecordEnumerateDeviceLayerProperties(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount, VkLayerProperties* pProperties, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordEnumerateDeviceLayerProperties(physicalDevice, pPropertyCount, pProperties, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetDeviceQueue(VkDevice device, uint32_t queueFamilyIndex, uint32_t queueIndex, VkQueue* pQueue) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetDeviceQueue(device, queueFamilyIndex, queueIndex, pQueue);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetDeviceQueue(VkDevice device, uint32_t queueFamilyIndex, uint32_t queueIndex, VkQueue* pQueue) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetDeviceQueue(device, queueFamilyIndex, queueIndex, pQueue);
}

void CoreChecksOptickInstrumented::PostCallRecordGetDeviceQueue(VkDevice device, uint32_t queueFamilyIndex, uint32_t queueIndex, VkQueue* pQueue) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetDeviceQueue(device, queueFamilyIndex, queueIndex, pQueue);
}

bool CoreChecksOptickInstrumented::PreCallValidateQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateQueueSubmit(queue, submitCount, pSubmits, fence);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordQueueSubmit(queue, submitCount, pSubmits, fence);
}

void CoreChecksOptickInstrumented::PostCallRecordQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordQueueSubmit(queue, submitCount, pSubmits, fence, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateQueueWaitIdle(VkQueue queue) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateQueueWaitIdle(queue);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordQueueWaitIdle(VkQueue queue) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordQueueWaitIdle(queue);
}

void CoreChecksOptickInstrumented::PostCallRecordQueueWaitIdle(VkQueue queue, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordQueueWaitIdle(queue, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateDeviceWaitIdle(VkDevice device) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateDeviceWaitIdle(device);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordDeviceWaitIdle(VkDevice device) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordDeviceWaitIdle(device);
}

void CoreChecksOptickInstrumented::PostCallRecordDeviceWaitIdle(VkDevice device, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordDeviceWaitIdle(device, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateAllocateMemory(VkDevice device, const VkMemoryAllocateInfo* pAllocateInfo, const VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateAllocateMemory(device, pAllocateInfo, pAllocator, pMemory);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordAllocateMemory(VkDevice device, const VkMemoryAllocateInfo* pAllocateInfo, const VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordAllocateMemory(device, pAllocateInfo, pAllocator, pMemory);
}

void CoreChecksOptickInstrumented::PostCallRecordAllocateMemory(VkDevice device, const VkMemoryAllocateInfo* pAllocateInfo, const VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordAllocateMemory(device, pAllocateInfo, pAllocator, pMemory, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateFreeMemory(VkDevice device, VkDeviceMemory memory, const VkAllocationCallbacks* pAllocator) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateFreeMemory(device, memory, pAllocator);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordFreeMemory(VkDevice device, VkDeviceMemory memory, const VkAllocationCallbacks* pAllocator) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordFreeMemory(device, memory, pAllocator);
}

void CoreChecksOptickInstrumented::PostCallRecordFreeMemory(VkDevice device, VkDeviceMemory memory, const VkAllocationCallbacks* pAllocator) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordFreeMemory(device, memory, pAllocator);
}

bool CoreChecksOptickInstrumented::PreCallValidateMapMemory(VkDevice device, VkDeviceMemory memory, VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags, void** ppData) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateMapMemory(device, memory, offset, size, flags, ppData);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordMapMemory(VkDevice device, VkDeviceMemory memory, VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags, void** ppData) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordMapMemory(device, memory, offset, size, flags, ppData);
}

void CoreChecksOptickInstrumented::PostCallRecordMapMemory(VkDevice device, VkDeviceMemory memory, VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags, void** ppData, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordMapMemory(device, memory, offset, size, flags, ppData, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateUnmapMemory(VkDevice device, VkDeviceMemory memory) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateUnmapMemory(device, memory);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordUnmapMemory(VkDevice device, VkDeviceMemory memory) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordUnmapMemory(device, memory);
}

void CoreChecksOptickInstrumented::PostCallRecordUnmapMemory(VkDevice device, VkDeviceMemory memory) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordUnmapMemory(device, memory);
}

bool CoreChecksOptickInstrumented::PreCallValidateFlushMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount, const VkMappedMemoryRange* pMemoryRanges) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateFlushMappedMemoryRanges(device, memoryRangeCount, pMemoryRanges);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordFlushMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount, const VkMappedMemoryRange* pMemoryRanges) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordFlushMappedMemoryRanges(device, memoryRangeCount, pMemoryRanges);
}

void CoreChecksOptickInstrumented::PostCallRecordFlushMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount, const VkMappedMemoryRange* pMemoryRanges, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordFlushMappedMemoryRanges(device, memoryRangeCount, pMemoryRanges, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateInvalidateMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount, const VkMappedMemoryRange* pMemoryRanges) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateInvalidateMappedMemoryRanges(device, memoryRangeCount, pMemoryRanges);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordInvalidateMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount, const VkMappedMemoryRange* pMemoryRanges) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordInvalidateMappedMemoryRanges(device, memoryRangeCount, pMemoryRanges);
}

void CoreChecksOptickInstrumented::PostCallRecordInvalidateMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount, const VkMappedMemoryRange* pMemoryRanges, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordInvalidateMappedMemoryRanges(device, memoryRangeCount, pMemoryRanges, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetDeviceMemoryCommitment(VkDevice device, VkDeviceMemory memory, VkDeviceSize* pCommittedMemoryInBytes) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetDeviceMemoryCommitment(device, memory, pCommittedMemoryInBytes);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetDeviceMemoryCommitment(VkDevice device, VkDeviceMemory memory, VkDeviceSize* pCommittedMemoryInBytes) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetDeviceMemoryCommitment(device, memory, pCommittedMemoryInBytes);
}

void CoreChecksOptickInstrumented::PostCallRecordGetDeviceMemoryCommitment(VkDevice device, VkDeviceMemory memory, VkDeviceSize* pCommittedMemoryInBytes) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetDeviceMemoryCommitment(device, memory, pCommittedMemoryInBytes);
}

bool CoreChecksOptickInstrumented::PreCallValidateBindBufferMemory(VkDevice device, VkBuffer buffer, VkDeviceMemory memory, VkDeviceSize memoryOffset) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateBindBufferMemory(device, buffer, memory, memoryOffset);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordBindBufferMemory(VkDevice device, VkBuffer buffer, VkDeviceMemory memory, VkDeviceSize memoryOffset) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordBindBufferMemory(device, buffer, memory, memoryOffset);
}

void CoreChecksOptickInstrumented::PostCallRecordBindBufferMemory(VkDevice device, VkBuffer buffer, VkDeviceMemory memory, VkDeviceSize memoryOffset, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordBindBufferMemory(device, buffer, memory, memoryOffset, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateBindImageMemory(VkDevice device, VkImage image, VkDeviceMemory memory, VkDeviceSize memoryOffset) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateBindImageMemory(device, image, memory, memoryOffset);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordBindImageMemory(VkDevice device, VkImage image, VkDeviceMemory memory, VkDeviceSize memoryOffset) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordBindImageMemory(device, image, memory, memoryOffset);
}

void CoreChecksOptickInstrumented::PostCallRecordBindImageMemory(VkDevice device, VkImage image, VkDeviceMemory memory, VkDeviceSize memoryOffset, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordBindImageMemory(device, image, memory, memoryOffset, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetBufferMemoryRequirements(VkDevice device, VkBuffer buffer, VkMemoryRequirements* pMemoryRequirements) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetBufferMemoryRequirements(device, buffer, pMemoryRequirements);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetBufferMemoryRequirements(VkDevice device, VkBuffer buffer, VkMemoryRequirements* pMemoryRequirements) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetBufferMemoryRequirements(device, buffer, pMemoryRequirements);
}

void CoreChecksOptickInstrumented::PostCallRecordGetBufferMemoryRequirements(VkDevice device, VkBuffer buffer, VkMemoryRequirements* pMemoryRequirements) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetBufferMemoryRequirements(device, buffer, pMemoryRequirements);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetImageMemoryRequirements(VkDevice device, VkImage image, VkMemoryRequirements* pMemoryRequirements) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetImageMemoryRequirements(device, image, pMemoryRequirements);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetImageMemoryRequirements(VkDevice device, VkImage image, VkMemoryRequirements* pMemoryRequirements) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetImageMemoryRequirements(device, image, pMemoryRequirements);
}

void CoreChecksOptickInstrumented::PostCallRecordGetImageMemoryRequirements(VkDevice device, VkImage image, VkMemoryRequirements* pMemoryRequirements) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetImageMemoryRequirements(device, image, pMemoryRequirements);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetImageSparseMemoryRequirements(VkDevice device, VkImage image, uint32_t* pSparseMemoryRequirementCount, VkSparseImageMemoryRequirements* pSparseMemoryRequirements) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetImageSparseMemoryRequirements(device, image, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetImageSparseMemoryRequirements(VkDevice device, VkImage image, uint32_t* pSparseMemoryRequirementCount, VkSparseImageMemoryRequirements* pSparseMemoryRequirements) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetImageSparseMemoryRequirements(device, image, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
}

void CoreChecksOptickInstrumented::PostCallRecordGetImageSparseMemoryRequirements(VkDevice device, VkImage image, uint32_t* pSparseMemoryRequirementCount, VkSparseImageMemoryRequirements* pSparseMemoryRequirements) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetImageSparseMemoryRequirements(device, image, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetPhysicalDeviceSparseImageFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type, VkSampleCountFlagBits samples, VkImageUsageFlags usage, VkImageTiling tiling, uint32_t* pPropertyCount, VkSparseImageFormatProperties* pProperties) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetPhysicalDeviceSparseImageFormatProperties(physicalDevice, format, type, samples, usage, tiling, pPropertyCount, pProperties);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetPhysicalDeviceSparseImageFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type, VkSampleCountFlagBits samples, VkImageUsageFlags usage, VkImageTiling tiling, uint32_t* pPropertyCount, VkSparseImageFormatProperties* pProperties) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetPhysicalDeviceSparseImageFormatProperties(physicalDevice, format, type, samples, usage, tiling, pPropertyCount, pProperties);
}

void CoreChecksOptickInstrumented::PostCallRecordGetPhysicalDeviceSparseImageFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type, VkSampleCountFlagBits samples, VkImageUsageFlags usage, VkImageTiling tiling, uint32_t* pPropertyCount, VkSparseImageFormatProperties* pProperties) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetPhysicalDeviceSparseImageFormatProperties(physicalDevice, format, type, samples, usage, tiling, pPropertyCount, pProperties);
}

bool CoreChecksOptickInstrumented::PreCallValidateQueueBindSparse(VkQueue queue, uint32_t bindInfoCount, const VkBindSparseInfo* pBindInfo, VkFence fence) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateQueueBindSparse(queue, bindInfoCount, pBindInfo, fence);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordQueueBindSparse(VkQueue queue, uint32_t bindInfoCount, const VkBindSparseInfo* pBindInfo, VkFence fence) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordQueueBindSparse(queue, bindInfoCount, pBindInfo, fence);
}

void CoreChecksOptickInstrumented::PostCallRecordQueueBindSparse(VkQueue queue, uint32_t bindInfoCount, const VkBindSparseInfo* pBindInfo, VkFence fence, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordQueueBindSparse(queue, bindInfoCount, pBindInfo, fence, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateCreateFence(VkDevice device, const VkFenceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkFence* pFence) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCreateFence(device, pCreateInfo, pAllocator, pFence);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCreateFence(VkDevice device, const VkFenceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkFence* pFence) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCreateFence(device, pCreateInfo, pAllocator, pFence);
}

void CoreChecksOptickInstrumented::PostCallRecordCreateFence(VkDevice device, const VkFenceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkFence* pFence, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCreateFence(device, pCreateInfo, pAllocator, pFence, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateDestroyFence(VkDevice device, VkFence fence, const VkAllocationCallbacks* pAllocator) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateDestroyFence(device, fence, pAllocator);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordDestroyFence(VkDevice device, VkFence fence, const VkAllocationCallbacks* pAllocator) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordDestroyFence(device, fence, pAllocator);
}

void CoreChecksOptickInstrumented::PostCallRecordDestroyFence(VkDevice device, VkFence fence, const VkAllocationCallbacks* pAllocator) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordDestroyFence(device, fence, pAllocator);
}

bool CoreChecksOptickInstrumented::PreCallValidateResetFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateResetFences(device, fenceCount, pFences);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordResetFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordResetFences(device, fenceCount, pFences);
}

void CoreChecksOptickInstrumented::PostCallRecordResetFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordResetFences(device, fenceCount, pFences, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetFenceStatus(VkDevice device, VkFence fence) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetFenceStatus(device, fence);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetFenceStatus(VkDevice device, VkFence fence) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetFenceStatus(device, fence);
}

void CoreChecksOptickInstrumented::PostCallRecordGetFenceStatus(VkDevice device, VkFence fence, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetFenceStatus(device, fence, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateWaitForFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences, VkBool32 waitAll, uint64_t timeout) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateWaitForFences(device, fenceCount, pFences, waitAll, timeout);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordWaitForFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences, VkBool32 waitAll, uint64_t timeout) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordWaitForFences(device, fenceCount, pFences, waitAll, timeout);
}

void CoreChecksOptickInstrumented::PostCallRecordWaitForFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences, VkBool32 waitAll, uint64_t timeout, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordWaitForFences(device, fenceCount, pFences, waitAll, timeout, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateCreateSemaphore(VkDevice device, const VkSemaphoreCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSemaphore* pSemaphore) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCreateSemaphore(device, pCreateInfo, pAllocator, pSemaphore);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCreateSemaphore(VkDevice device, const VkSemaphoreCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSemaphore* pSemaphore) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCreateSemaphore(device, pCreateInfo, pAllocator, pSemaphore);
}

void CoreChecksOptickInstrumented::PostCallRecordCreateSemaphore(VkDevice device, const VkSemaphoreCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSemaphore* pSemaphore, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCreateSemaphore(device, pCreateInfo, pAllocator, pSemaphore, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateDestroySemaphore(VkDevice device, VkSemaphore semaphore, const VkAllocationCallbacks* pAllocator) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateDestroySemaphore(device, semaphore, pAllocator);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordDestroySemaphore(VkDevice device, VkSemaphore semaphore, const VkAllocationCallbacks* pAllocator) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordDestroySemaphore(device, semaphore, pAllocator);
}

void CoreChecksOptickInstrumented::PostCallRecordDestroySemaphore(VkDevice device, VkSemaphore semaphore, const VkAllocationCallbacks* pAllocator) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordDestroySemaphore(device, semaphore, pAllocator);
}

bool CoreChecksOptickInstrumented::PreCallValidateCreateEvent(VkDevice device, const VkEventCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkEvent* pEvent) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCreateEvent(device, pCreateInfo, pAllocator, pEvent);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCreateEvent(VkDevice device, const VkEventCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkEvent* pEvent) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCreateEvent(device, pCreateInfo, pAllocator, pEvent);
}

void CoreChecksOptickInstrumented::PostCallRecordCreateEvent(VkDevice device, const VkEventCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkEvent* pEvent, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCreateEvent(device, pCreateInfo, pAllocator, pEvent, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateDestroyEvent(VkDevice device, VkEvent event, const VkAllocationCallbacks* pAllocator) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateDestroyEvent(device, event, pAllocator);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordDestroyEvent(VkDevice device, VkEvent event, const VkAllocationCallbacks* pAllocator) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordDestroyEvent(device, event, pAllocator);
}

void CoreChecksOptickInstrumented::PostCallRecordDestroyEvent(VkDevice device, VkEvent event, const VkAllocationCallbacks* pAllocator) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordDestroyEvent(device, event, pAllocator);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetEventStatus(VkDevice device, VkEvent event) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetEventStatus(device, event);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetEventStatus(VkDevice device, VkEvent event) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetEventStatus(device, event);
}

void CoreChecksOptickInstrumented::PostCallRecordGetEventStatus(VkDevice device, VkEvent event, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetEventStatus(device, event, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateSetEvent(VkDevice device, VkEvent event) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateSetEvent(device, event);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordSetEvent(VkDevice device, VkEvent event) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordSetEvent(device, event);
}

void CoreChecksOptickInstrumented::PostCallRecordSetEvent(VkDevice device, VkEvent event, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordSetEvent(device, event, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateResetEvent(VkDevice device, VkEvent event) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateResetEvent(device, event);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordResetEvent(VkDevice device, VkEvent event) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordResetEvent(device, event);
}

void CoreChecksOptickInstrumented::PostCallRecordResetEvent(VkDevice device, VkEvent event, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordResetEvent(device, event, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateCreateQueryPool(VkDevice device, const VkQueryPoolCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkQueryPool* pQueryPool) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCreateQueryPool(device, pCreateInfo, pAllocator, pQueryPool);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCreateQueryPool(VkDevice device, const VkQueryPoolCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkQueryPool* pQueryPool) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCreateQueryPool(device, pCreateInfo, pAllocator, pQueryPool);
}

void CoreChecksOptickInstrumented::PostCallRecordCreateQueryPool(VkDevice device, const VkQueryPoolCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkQueryPool* pQueryPool, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCreateQueryPool(device, pCreateInfo, pAllocator, pQueryPool, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateDestroyQueryPool(VkDevice device, VkQueryPool queryPool, const VkAllocationCallbacks* pAllocator) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateDestroyQueryPool(device, queryPool, pAllocator);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordDestroyQueryPool(VkDevice device, VkQueryPool queryPool, const VkAllocationCallbacks* pAllocator) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordDestroyQueryPool(device, queryPool, pAllocator);
}

void CoreChecksOptickInstrumented::PostCallRecordDestroyQueryPool(VkDevice device, VkQueryPool queryPool, const VkAllocationCallbacks* pAllocator) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordDestroyQueryPool(device, queryPool, pAllocator);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetQueryPoolResults(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount, size_t dataSize, void* pData, VkDeviceSize stride, VkQueryResultFlags flags) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetQueryPoolResults(device, queryPool, firstQuery, queryCount, dataSize, pData, stride, flags);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetQueryPoolResults(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount, size_t dataSize, void* pData, VkDeviceSize stride, VkQueryResultFlags flags) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetQueryPoolResults(device, queryPool, firstQuery, queryCount, dataSize, pData, stride, flags);
}

void CoreChecksOptickInstrumented::PostCallRecordGetQueryPoolResults(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount, size_t dataSize, void* pData, VkDeviceSize stride, VkQueryResultFlags flags, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetQueryPoolResults(device, queryPool, firstQuery, queryCount, dataSize, pData, stride, flags, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateCreateBuffer(VkDevice device, const VkBufferCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkBuffer* pBuffer) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCreateBuffer(device, pCreateInfo, pAllocator, pBuffer);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCreateBuffer(VkDevice device, const VkBufferCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkBuffer* pBuffer, void* extra_data) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCreateBuffer(device, pCreateInfo, pAllocator, pBuffer, extra_data);
}

void CoreChecksOptickInstrumented::PostCallRecordCreateBuffer(VkDevice device, const VkBufferCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkBuffer* pBuffer, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCreateBuffer(device, pCreateInfo, pAllocator, pBuffer, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateDestroyBuffer(VkDevice device, VkBuffer buffer, const VkAllocationCallbacks* pAllocator) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateDestroyBuffer(device, buffer, pAllocator);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordDestroyBuffer(VkDevice device, VkBuffer buffer, const VkAllocationCallbacks* pAllocator) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordDestroyBuffer(device, buffer, pAllocator);
}

void CoreChecksOptickInstrumented::PostCallRecordDestroyBuffer(VkDevice device, VkBuffer buffer, const VkAllocationCallbacks* pAllocator) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordDestroyBuffer(device, buffer, pAllocator);
}

bool CoreChecksOptickInstrumented::PreCallValidateCreateBufferView(VkDevice device, const VkBufferViewCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkBufferView* pView) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCreateBufferView(device, pCreateInfo, pAllocator, pView);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCreateBufferView(VkDevice device, const VkBufferViewCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkBufferView* pView) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCreateBufferView(device, pCreateInfo, pAllocator, pView);
}

void CoreChecksOptickInstrumented::PostCallRecordCreateBufferView(VkDevice device, const VkBufferViewCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkBufferView* pView, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCreateBufferView(device, pCreateInfo, pAllocator, pView, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateDestroyBufferView(VkDevice device, VkBufferView bufferView, const VkAllocationCallbacks* pAllocator) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateDestroyBufferView(device, bufferView, pAllocator);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordDestroyBufferView(VkDevice device, VkBufferView bufferView, const VkAllocationCallbacks* pAllocator) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordDestroyBufferView(device, bufferView, pAllocator);
}

void CoreChecksOptickInstrumented::PostCallRecordDestroyBufferView(VkDevice device, VkBufferView bufferView, const VkAllocationCallbacks* pAllocator) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordDestroyBufferView(device, bufferView, pAllocator);
}

bool CoreChecksOptickInstrumented::PreCallValidateCreateImage(VkDevice device, const VkImageCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkImage* pImage) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCreateImage(device, pCreateInfo, pAllocator, pImage);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCreateImage(VkDevice device, const VkImageCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkImage* pImage) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCreateImage(device, pCreateInfo, pAllocator, pImage);
}

void CoreChecksOptickInstrumented::PostCallRecordCreateImage(VkDevice device, const VkImageCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkImage* pImage, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCreateImage(device, pCreateInfo, pAllocator, pImage, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateDestroyImage(VkDevice device, VkImage image, const VkAllocationCallbacks* pAllocator) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateDestroyImage(device, image, pAllocator);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordDestroyImage(VkDevice device, VkImage image, const VkAllocationCallbacks* pAllocator) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordDestroyImage(device, image, pAllocator);
}

void CoreChecksOptickInstrumented::PostCallRecordDestroyImage(VkDevice device, VkImage image, const VkAllocationCallbacks* pAllocator) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordDestroyImage(device, image, pAllocator);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetImageSubresourceLayout(VkDevice device, VkImage image, const VkImageSubresource* pSubresource, VkSubresourceLayout* pLayout) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetImageSubresourceLayout(device, image, pSubresource, pLayout);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetImageSubresourceLayout(VkDevice device, VkImage image, const VkImageSubresource* pSubresource, VkSubresourceLayout* pLayout) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetImageSubresourceLayout(device, image, pSubresource, pLayout);
}

void CoreChecksOptickInstrumented::PostCallRecordGetImageSubresourceLayout(VkDevice device, VkImage image, const VkImageSubresource* pSubresource, VkSubresourceLayout* pLayout) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetImageSubresourceLayout(device, image, pSubresource, pLayout);
}

bool CoreChecksOptickInstrumented::PreCallValidateCreateImageView(VkDevice device, const VkImageViewCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkImageView* pView) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCreateImageView(device, pCreateInfo, pAllocator, pView);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCreateImageView(VkDevice device, const VkImageViewCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkImageView* pView) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCreateImageView(device, pCreateInfo, pAllocator, pView);
}

void CoreChecksOptickInstrumented::PostCallRecordCreateImageView(VkDevice device, const VkImageViewCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkImageView* pView, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCreateImageView(device, pCreateInfo, pAllocator, pView, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateDestroyImageView(VkDevice device, VkImageView imageView, const VkAllocationCallbacks* pAllocator) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateDestroyImageView(device, imageView, pAllocator);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordDestroyImageView(VkDevice device, VkImageView imageView, const VkAllocationCallbacks* pAllocator) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordDestroyImageView(device, imageView, pAllocator);
}

void CoreChecksOptickInstrumented::PostCallRecordDestroyImageView(VkDevice device, VkImageView imageView, const VkAllocationCallbacks* pAllocator) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordDestroyImageView(device, imageView, pAllocator);
}

bool CoreChecksOptickInstrumented::PreCallValidateCreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkShaderModule* pShaderModule) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCreateShaderModule(device, pCreateInfo, pAllocator, pShaderModule);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkShaderModule* pShaderModule, void* extra_data) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCreateShaderModule(device, pCreateInfo, pAllocator, pShaderModule, extra_data);
}

void CoreChecksOptickInstrumented::PostCallRecordCreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkShaderModule* pShaderModule, VkResult result, void* extra_data) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCreateShaderModule(device, pCreateInfo, pAllocator, pShaderModule, result, extra_data);
}

bool CoreChecksOptickInstrumented::PreCallValidateDestroyShaderModule(VkDevice device, VkShaderModule shaderModule, const VkAllocationCallbacks* pAllocator) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateDestroyShaderModule(device, shaderModule, pAllocator);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordDestroyShaderModule(VkDevice device, VkShaderModule shaderModule, const VkAllocationCallbacks* pAllocator) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordDestroyShaderModule(device, shaderModule, pAllocator);
}

void CoreChecksOptickInstrumented::PostCallRecordDestroyShaderModule(VkDevice device, VkShaderModule shaderModule, const VkAllocationCallbacks* pAllocator) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordDestroyShaderModule(device, shaderModule, pAllocator);
}

bool CoreChecksOptickInstrumented::PreCallValidateCreatePipelineCache(VkDevice device, const VkPipelineCacheCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkPipelineCache* pPipelineCache) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCreatePipelineCache(device, pCreateInfo, pAllocator, pPipelineCache);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCreatePipelineCache(VkDevice device, const VkPipelineCacheCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkPipelineCache* pPipelineCache) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCreatePipelineCache(device, pCreateInfo, pAllocator, pPipelineCache);
}

void CoreChecksOptickInstrumented::PostCallRecordCreatePipelineCache(VkDevice device, const VkPipelineCacheCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkPipelineCache* pPipelineCache, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCreatePipelineCache(device, pCreateInfo, pAllocator, pPipelineCache, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateDestroyPipelineCache(VkDevice device, VkPipelineCache pipelineCache, const VkAllocationCallbacks* pAllocator) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateDestroyPipelineCache(device, pipelineCache, pAllocator);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordDestroyPipelineCache(VkDevice device, VkPipelineCache pipelineCache, const VkAllocationCallbacks* pAllocator) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordDestroyPipelineCache(device, pipelineCache, pAllocator);
}

void CoreChecksOptickInstrumented::PostCallRecordDestroyPipelineCache(VkDevice device, VkPipelineCache pipelineCache, const VkAllocationCallbacks* pAllocator) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordDestroyPipelineCache(device, pipelineCache, pAllocator);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetPipelineCacheData(VkDevice device, VkPipelineCache pipelineCache, size_t* pDataSize, void* pData) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetPipelineCacheData(device, pipelineCache, pDataSize, pData);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetPipelineCacheData(VkDevice device, VkPipelineCache pipelineCache, size_t* pDataSize, void* pData) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetPipelineCacheData(device, pipelineCache, pDataSize, pData);
}

void CoreChecksOptickInstrumented::PostCallRecordGetPipelineCacheData(VkDevice device, VkPipelineCache pipelineCache, size_t* pDataSize, void* pData, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetPipelineCacheData(device, pipelineCache, pDataSize, pData, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateMergePipelineCaches(VkDevice device, VkPipelineCache dstCache, uint32_t srcCacheCount, const VkPipelineCache* pSrcCaches) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateMergePipelineCaches(device, dstCache, srcCacheCount, pSrcCaches);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordMergePipelineCaches(VkDevice device, VkPipelineCache dstCache, uint32_t srcCacheCount, const VkPipelineCache* pSrcCaches) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordMergePipelineCaches(device, dstCache, srcCacheCount, pSrcCaches);
}

void CoreChecksOptickInstrumented::PostCallRecordMergePipelineCaches(VkDevice device, VkPipelineCache dstCache, uint32_t srcCacheCount, const VkPipelineCache* pSrcCaches, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordMergePipelineCaches(device, dstCache, srcCacheCount, pSrcCaches, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkGraphicsPipelineCreateInfo* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines, void* extra_data) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCreateGraphicsPipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines, extra_data);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkGraphicsPipelineCreateInfo* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines, void* extra_data) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCreateGraphicsPipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines, extra_data);
}

void CoreChecksOptickInstrumented::PostCallRecordCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkGraphicsPipelineCreateInfo* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines, VkResult result, void* extra_data) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCreateGraphicsPipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines, result, extra_data);
}

bool CoreChecksOptickInstrumented::PreCallValidateCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkComputePipelineCreateInfo* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines, void* extra_data) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCreateComputePipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines, extra_data);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkComputePipelineCreateInfo* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines, void* extra_data) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCreateComputePipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines, extra_data);
}

void CoreChecksOptickInstrumented::PostCallRecordCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkComputePipelineCreateInfo* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines, VkResult result, void* extra_data) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCreateComputePipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines, result, extra_data);
}

bool CoreChecksOptickInstrumented::PreCallValidateDestroyPipeline(VkDevice device, VkPipeline pipeline, const VkAllocationCallbacks* pAllocator) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateDestroyPipeline(device, pipeline, pAllocator);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordDestroyPipeline(VkDevice device, VkPipeline pipeline, const VkAllocationCallbacks* pAllocator) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordDestroyPipeline(device, pipeline, pAllocator);
}

void CoreChecksOptickInstrumented::PostCallRecordDestroyPipeline(VkDevice device, VkPipeline pipeline, const VkAllocationCallbacks* pAllocator) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordDestroyPipeline(device, pipeline, pAllocator);
}

bool CoreChecksOptickInstrumented::PreCallValidateCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkPipelineLayout* pPipelineLayout) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCreatePipelineLayout(device, pCreateInfo, pAllocator, pPipelineLayout);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkPipelineLayout* pPipelineLayout, void* extra_data) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCreatePipelineLayout(device, pCreateInfo, pAllocator, pPipelineLayout, extra_data);
}

void CoreChecksOptickInstrumented::PostCallRecordCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkPipelineLayout* pPipelineLayout, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCreatePipelineLayout(device, pCreateInfo, pAllocator, pPipelineLayout, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateDestroyPipelineLayout(VkDevice device, VkPipelineLayout pipelineLayout, const VkAllocationCallbacks* pAllocator) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateDestroyPipelineLayout(device, pipelineLayout, pAllocator);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordDestroyPipelineLayout(VkDevice device, VkPipelineLayout pipelineLayout, const VkAllocationCallbacks* pAllocator) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordDestroyPipelineLayout(device, pipelineLayout, pAllocator);
}

void CoreChecksOptickInstrumented::PostCallRecordDestroyPipelineLayout(VkDevice device, VkPipelineLayout pipelineLayout, const VkAllocationCallbacks* pAllocator) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordDestroyPipelineLayout(device, pipelineLayout, pAllocator);
}

bool CoreChecksOptickInstrumented::PreCallValidateCreateSampler(VkDevice device, const VkSamplerCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSampler* pSampler) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCreateSampler(device, pCreateInfo, pAllocator, pSampler);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCreateSampler(VkDevice device, const VkSamplerCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSampler* pSampler) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCreateSampler(device, pCreateInfo, pAllocator, pSampler);
}

void CoreChecksOptickInstrumented::PostCallRecordCreateSampler(VkDevice device, const VkSamplerCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSampler* pSampler, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCreateSampler(device, pCreateInfo, pAllocator, pSampler, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateDestroySampler(VkDevice device, VkSampler sampler, const VkAllocationCallbacks* pAllocator) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateDestroySampler(device, sampler, pAllocator);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordDestroySampler(VkDevice device, VkSampler sampler, const VkAllocationCallbacks* pAllocator) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordDestroySampler(device, sampler, pAllocator);
}

void CoreChecksOptickInstrumented::PostCallRecordDestroySampler(VkDevice device, VkSampler sampler, const VkAllocationCallbacks* pAllocator) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordDestroySampler(device, sampler, pAllocator);
}

bool CoreChecksOptickInstrumented::PreCallValidateCreateDescriptorSetLayout(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDescriptorSetLayout* pSetLayout) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCreateDescriptorSetLayout(device, pCreateInfo, pAllocator, pSetLayout);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCreateDescriptorSetLayout(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDescriptorSetLayout* pSetLayout) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCreateDescriptorSetLayout(device, pCreateInfo, pAllocator, pSetLayout);
}

void CoreChecksOptickInstrumented::PostCallRecordCreateDescriptorSetLayout(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDescriptorSetLayout* pSetLayout, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCreateDescriptorSetLayout(device, pCreateInfo, pAllocator, pSetLayout, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateDestroyDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout descriptorSetLayout, const VkAllocationCallbacks* pAllocator) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateDestroyDescriptorSetLayout(device, descriptorSetLayout, pAllocator);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordDestroyDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout descriptorSetLayout, const VkAllocationCallbacks* pAllocator) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordDestroyDescriptorSetLayout(device, descriptorSetLayout, pAllocator);
}

void CoreChecksOptickInstrumented::PostCallRecordDestroyDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout descriptorSetLayout, const VkAllocationCallbacks* pAllocator) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordDestroyDescriptorSetLayout(device, descriptorSetLayout, pAllocator);
}

bool CoreChecksOptickInstrumented::PreCallValidateCreateDescriptorPool(VkDevice device, const VkDescriptorPoolCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDescriptorPool* pDescriptorPool) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCreateDescriptorPool(device, pCreateInfo, pAllocator, pDescriptorPool);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCreateDescriptorPool(VkDevice device, const VkDescriptorPoolCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDescriptorPool* pDescriptorPool) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCreateDescriptorPool(device, pCreateInfo, pAllocator, pDescriptorPool);
}

void CoreChecksOptickInstrumented::PostCallRecordCreateDescriptorPool(VkDevice device, const VkDescriptorPoolCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDescriptorPool* pDescriptorPool, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCreateDescriptorPool(device, pCreateInfo, pAllocator, pDescriptorPool, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateDestroyDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool, const VkAllocationCallbacks* pAllocator) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateDestroyDescriptorPool(device, descriptorPool, pAllocator);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordDestroyDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool, const VkAllocationCallbacks* pAllocator) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordDestroyDescriptorPool(device, descriptorPool, pAllocator);
}

void CoreChecksOptickInstrumented::PostCallRecordDestroyDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool, const VkAllocationCallbacks* pAllocator) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordDestroyDescriptorPool(device, descriptorPool, pAllocator);
}

bool CoreChecksOptickInstrumented::PreCallValidateResetDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool, VkDescriptorPoolResetFlags flags) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateResetDescriptorPool(device, descriptorPool, flags);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordResetDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool, VkDescriptorPoolResetFlags flags) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordResetDescriptorPool(device, descriptorPool, flags);
}

void CoreChecksOptickInstrumented::PostCallRecordResetDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool, VkDescriptorPoolResetFlags flags, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordResetDescriptorPool(device, descriptorPool, flags, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateAllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo* pAllocateInfo, VkDescriptorSet* pDescriptorSets, void* extra_data) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateAllocateDescriptorSets(device, pAllocateInfo, pDescriptorSets, extra_data);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordAllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo* pAllocateInfo, VkDescriptorSet* pDescriptorSets) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordAllocateDescriptorSets(device, pAllocateInfo, pDescriptorSets);
}

void CoreChecksOptickInstrumented::PostCallRecordAllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo* pAllocateInfo, VkDescriptorSet* pDescriptorSets, VkResult result, void* extra_data) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordAllocateDescriptorSets(device, pAllocateInfo, pDescriptorSets, result, extra_data);
}

bool CoreChecksOptickInstrumented::PreCallValidateFreeDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool, uint32_t descriptorSetCount, const VkDescriptorSet* pDescriptorSets) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateFreeDescriptorSets(device, descriptorPool, descriptorSetCount, pDescriptorSets);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordFreeDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool, uint32_t descriptorSetCount, const VkDescriptorSet* pDescriptorSets) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordFreeDescriptorSets(device, descriptorPool, descriptorSetCount, pDescriptorSets);
}

void CoreChecksOptickInstrumented::PostCallRecordFreeDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool, uint32_t descriptorSetCount, const VkDescriptorSet* pDescriptorSets, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordFreeDescriptorSets(device, descriptorPool, descriptorSetCount, pDescriptorSets, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateUpdateDescriptorSets(VkDevice device, uint32_t descriptorWriteCount, const VkWriteDescriptorSet* pDescriptorWrites, uint32_t descriptorCopyCount, const VkCopyDescriptorSet* pDescriptorCopies) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateUpdateDescriptorSets(device, descriptorWriteCount, pDescriptorWrites, descriptorCopyCount, pDescriptorCopies);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordUpdateDescriptorSets(VkDevice device, uint32_t descriptorWriteCount, const VkWriteDescriptorSet* pDescriptorWrites, uint32_t descriptorCopyCount, const VkCopyDescriptorSet* pDescriptorCopies) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordUpdateDescriptorSets(device, descriptorWriteCount, pDescriptorWrites, descriptorCopyCount, pDescriptorCopies);
}

void CoreChecksOptickInstrumented::PostCallRecordUpdateDescriptorSets(VkDevice device, uint32_t descriptorWriteCount, const VkWriteDescriptorSet* pDescriptorWrites, uint32_t descriptorCopyCount, const VkCopyDescriptorSet* pDescriptorCopies) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordUpdateDescriptorSets(device, descriptorWriteCount, pDescriptorWrites, descriptorCopyCount, pDescriptorCopies);
}

bool CoreChecksOptickInstrumented::PreCallValidateCreateFramebuffer(VkDevice device, const VkFramebufferCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkFramebuffer* pFramebuffer) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCreateFramebuffer(device, pCreateInfo, pAllocator, pFramebuffer);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCreateFramebuffer(VkDevice device, const VkFramebufferCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkFramebuffer* pFramebuffer) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCreateFramebuffer(device, pCreateInfo, pAllocator, pFramebuffer);
}

void CoreChecksOptickInstrumented::PostCallRecordCreateFramebuffer(VkDevice device, const VkFramebufferCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkFramebuffer* pFramebuffer, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCreateFramebuffer(device, pCreateInfo, pAllocator, pFramebuffer, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateDestroyFramebuffer(VkDevice device, VkFramebuffer framebuffer, const VkAllocationCallbacks* pAllocator) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateDestroyFramebuffer(device, framebuffer, pAllocator);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordDestroyFramebuffer(VkDevice device, VkFramebuffer framebuffer, const VkAllocationCallbacks* pAllocator) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordDestroyFramebuffer(device, framebuffer, pAllocator);
}

void CoreChecksOptickInstrumented::PostCallRecordDestroyFramebuffer(VkDevice device, VkFramebuffer framebuffer, const VkAllocationCallbacks* pAllocator) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordDestroyFramebuffer(device, framebuffer, pAllocator);
}

bool CoreChecksOptickInstrumented::PreCallValidateCreateRenderPass(VkDevice device, const VkRenderPassCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCreateRenderPass(device, pCreateInfo, pAllocator, pRenderPass);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCreateRenderPass(VkDevice device, const VkRenderPassCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCreateRenderPass(device, pCreateInfo, pAllocator, pRenderPass);
}

void CoreChecksOptickInstrumented::PostCallRecordCreateRenderPass(VkDevice device, const VkRenderPassCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCreateRenderPass(device, pCreateInfo, pAllocator, pRenderPass, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateDestroyRenderPass(VkDevice device, VkRenderPass renderPass, const VkAllocationCallbacks* pAllocator) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateDestroyRenderPass(device, renderPass, pAllocator);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordDestroyRenderPass(VkDevice device, VkRenderPass renderPass, const VkAllocationCallbacks* pAllocator) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordDestroyRenderPass(device, renderPass, pAllocator);
}

void CoreChecksOptickInstrumented::PostCallRecordDestroyRenderPass(VkDevice device, VkRenderPass renderPass, const VkAllocationCallbacks* pAllocator) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordDestroyRenderPass(device, renderPass, pAllocator);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetRenderAreaGranularity(VkDevice device, VkRenderPass renderPass, VkExtent2D* pGranularity) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetRenderAreaGranularity(device, renderPass, pGranularity);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetRenderAreaGranularity(VkDevice device, VkRenderPass renderPass, VkExtent2D* pGranularity) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetRenderAreaGranularity(device, renderPass, pGranularity);
}

void CoreChecksOptickInstrumented::PostCallRecordGetRenderAreaGranularity(VkDevice device, VkRenderPass renderPass, VkExtent2D* pGranularity) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetRenderAreaGranularity(device, renderPass, pGranularity);
}

bool CoreChecksOptickInstrumented::PreCallValidateCreateCommandPool(VkDevice device, const VkCommandPoolCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkCommandPool* pCommandPool) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCreateCommandPool(device, pCreateInfo, pAllocator, pCommandPool);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCreateCommandPool(VkDevice device, const VkCommandPoolCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkCommandPool* pCommandPool) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCreateCommandPool(device, pCreateInfo, pAllocator, pCommandPool);
}

void CoreChecksOptickInstrumented::PostCallRecordCreateCommandPool(VkDevice device, const VkCommandPoolCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkCommandPool* pCommandPool, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCreateCommandPool(device, pCreateInfo, pAllocator, pCommandPool, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateDestroyCommandPool(VkDevice device, VkCommandPool commandPool, const VkAllocationCallbacks* pAllocator) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateDestroyCommandPool(device, commandPool, pAllocator);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordDestroyCommandPool(VkDevice device, VkCommandPool commandPool, const VkAllocationCallbacks* pAllocator) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordDestroyCommandPool(device, commandPool, pAllocator);
}

void CoreChecksOptickInstrumented::PostCallRecordDestroyCommandPool(VkDevice device, VkCommandPool commandPool, const VkAllocationCallbacks* pAllocator) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordDestroyCommandPool(device, commandPool, pAllocator);
}

bool CoreChecksOptickInstrumented::PreCallValidateResetCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolResetFlags flags) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateResetCommandPool(device, commandPool, flags);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordResetCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolResetFlags flags) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordResetCommandPool(device, commandPool, flags);
}

void CoreChecksOptickInstrumented::PostCallRecordResetCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolResetFlags flags, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordResetCommandPool(device, commandPool, flags, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateAllocateCommandBuffers(VkDevice device, const VkCommandBufferAllocateInfo* pAllocateInfo, VkCommandBuffer* pCommandBuffers) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateAllocateCommandBuffers(device, pAllocateInfo, pCommandBuffers);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordAllocateCommandBuffers(VkDevice device, const VkCommandBufferAllocateInfo* pAllocateInfo, VkCommandBuffer* pCommandBuffers) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordAllocateCommandBuffers(device, pAllocateInfo, pCommandBuffers);
}

void CoreChecksOptickInstrumented::PostCallRecordAllocateCommandBuffers(VkDevice device, const VkCommandBufferAllocateInfo* pAllocateInfo, VkCommandBuffer* pCommandBuffers, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordAllocateCommandBuffers(device, pAllocateInfo, pCommandBuffers, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateFreeCommandBuffers(VkDevice device, VkCommandPool commandPool, uint32_t commandBufferCount, const VkCommandBuffer* pCommandBuffers) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateFreeCommandBuffers(device, commandPool, commandBufferCount, pCommandBuffers);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordFreeCommandBuffers(VkDevice device, VkCommandPool commandPool, uint32_t commandBufferCount, const VkCommandBuffer* pCommandBuffers) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordFreeCommandBuffers(device, commandPool, commandBufferCount, pCommandBuffers);
}

void CoreChecksOptickInstrumented::PostCallRecordFreeCommandBuffers(VkDevice device, VkCommandPool commandPool, uint32_t commandBufferCount, const VkCommandBuffer* pCommandBuffers) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordFreeCommandBuffers(device, commandPool, commandBufferCount, pCommandBuffers);
}

bool CoreChecksOptickInstrumented::PreCallValidateBeginCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo* pBeginInfo) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateBeginCommandBuffer(commandBuffer, pBeginInfo);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordBeginCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo* pBeginInfo) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordBeginCommandBuffer(commandBuffer, pBeginInfo);
}

void CoreChecksOptickInstrumented::PostCallRecordBeginCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo* pBeginInfo, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordBeginCommandBuffer(commandBuffer, pBeginInfo, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateEndCommandBuffer(VkCommandBuffer commandBuffer) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateEndCommandBuffer(commandBuffer);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordEndCommandBuffer(VkCommandBuffer commandBuffer) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordEndCommandBuffer(commandBuffer);
}

void CoreChecksOptickInstrumented::PostCallRecordEndCommandBuffer(VkCommandBuffer commandBuffer, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordEndCommandBuffer(commandBuffer, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateResetCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferResetFlags flags) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateResetCommandBuffer(commandBuffer, flags);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordResetCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferResetFlags flags) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordResetCommandBuffer(commandBuffer, flags);
}

void CoreChecksOptickInstrumented::PostCallRecordResetCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferResetFlags flags, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordResetCommandBuffer(commandBuffer, flags, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdBindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdBindPipeline(commandBuffer, pipelineBindPoint, pipeline);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdBindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdBindPipeline(commandBuffer, pipelineBindPoint, pipeline);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdBindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdBindPipeline(commandBuffer, pipelineBindPoint, pipeline);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdSetViewport(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount, const VkViewport* pViewports) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdSetViewport(commandBuffer, firstViewport, viewportCount, pViewports);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdSetViewport(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount, const VkViewport* pViewports) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdSetViewport(commandBuffer, firstViewport, viewportCount, pViewports);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdSetViewport(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount, const VkViewport* pViewports) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdSetViewport(commandBuffer, firstViewport, viewportCount, pViewports);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdSetScissor(VkCommandBuffer commandBuffer, uint32_t firstScissor, uint32_t scissorCount, const VkRect2D* pScissors) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdSetScissor(commandBuffer, firstScissor, scissorCount, pScissors);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdSetScissor(VkCommandBuffer commandBuffer, uint32_t firstScissor, uint32_t scissorCount, const VkRect2D* pScissors) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdSetScissor(commandBuffer, firstScissor, scissorCount, pScissors);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdSetScissor(VkCommandBuffer commandBuffer, uint32_t firstScissor, uint32_t scissorCount, const VkRect2D* pScissors) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdSetScissor(commandBuffer, firstScissor, scissorCount, pScissors);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdSetLineWidth(VkCommandBuffer commandBuffer, float lineWidth) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdSetLineWidth(commandBuffer, lineWidth);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdSetLineWidth(VkCommandBuffer commandBuffer, float lineWidth) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdSetLineWidth(commandBuffer, lineWidth);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdSetLineWidth(VkCommandBuffer commandBuffer, float lineWidth) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdSetLineWidth(commandBuffer, lineWidth);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdSetDepthBias(VkCommandBuffer commandBuffer, float depthBiasConstantFactor, float depthBiasClamp, float depthBiasSlopeFactor) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdSetDepthBias(commandBuffer, depthBiasConstantFactor, depthBiasClamp, depthBiasSlopeFactor);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdSetDepthBias(VkCommandBuffer commandBuffer, float depthBiasConstantFactor, float depthBiasClamp, float depthBiasSlopeFactor) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdSetDepthBias(commandBuffer, depthBiasConstantFactor, depthBiasClamp, depthBiasSlopeFactor);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdSetDepthBias(VkCommandBuffer commandBuffer, float depthBiasConstantFactor, float depthBiasClamp, float depthBiasSlopeFactor) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdSetDepthBias(commandBuffer, depthBiasConstantFactor, depthBiasClamp, depthBiasSlopeFactor);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdSetBlendConstants(VkCommandBuffer commandBuffer, const float blendConstants[4]) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdSetBlendConstants(commandBuffer, blendConstants);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdSetBlendConstants(VkCommandBuffer commandBuffer, const float blendConstants[4]) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdSetBlendConstants(commandBuffer, blendConstants);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdSetBlendConstants(VkCommandBuffer commandBuffer, const float blendConstants[4]) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdSetBlendConstants(commandBuffer, blendConstants);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdSetDepthBounds(VkCommandBuffer commandBuffer, float minDepthBounds, float maxDepthBounds) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdSetDepthBounds(commandBuffer, minDepthBounds, maxDepthBounds);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdSetDepthBounds(VkCommandBuffer commandBuffer, float minDepthBounds, float maxDepthBounds) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdSetDepthBounds(commandBuffer, minDepthBounds, maxDepthBounds);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdSetDepthBounds(VkCommandBuffer commandBuffer, float minDepthBounds, float maxDepthBounds) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdSetDepthBounds(commandBuffer, minDepthBounds, maxDepthBounds);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdSetStencilCompareMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t compareMask) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdSetStencilCompareMask(commandBuffer, faceMask, compareMask);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdSetStencilCompareMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t compareMask) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdSetStencilCompareMask(commandBuffer, faceMask, compareMask);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdSetStencilCompareMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t compareMask) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdSetStencilCompareMask(commandBuffer, faceMask, compareMask);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdSetStencilWriteMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t writeMask) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdSetStencilWriteMask(commandBuffer, faceMask, writeMask);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdSetStencilWriteMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t writeMask) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdSetStencilWriteMask(commandBuffer, faceMask, writeMask);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdSetStencilWriteMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t writeMask) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdSetStencilWriteMask(commandBuffer, faceMask, writeMask);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdSetStencilReference(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t reference) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdSetStencilReference(commandBuffer, faceMask, reference);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdSetStencilReference(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t reference) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdSetStencilReference(commandBuffer, faceMask, reference);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdSetStencilReference(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t reference) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdSetStencilReference(commandBuffer, faceMask, reference);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdBindDescriptorSets(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout, uint32_t firstSet, uint32_t descriptorSetCount, const VkDescriptorSet* pDescriptorSets, uint32_t dynamicOffsetCount, const uint32_t* pDynamicOffsets) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdBindDescriptorSets(commandBuffer, pipelineBindPoint, layout, firstSet, descriptorSetCount, pDescriptorSets, dynamicOffsetCount, pDynamicOffsets);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdBindDescriptorSets(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout, uint32_t firstSet, uint32_t descriptorSetCount, const VkDescriptorSet* pDescriptorSets, uint32_t dynamicOffsetCount, const uint32_t* pDynamicOffsets) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdBindDescriptorSets(commandBuffer, pipelineBindPoint, layout, firstSet, descriptorSetCount, pDescriptorSets, dynamicOffsetCount, pDynamicOffsets);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdBindDescriptorSets(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout, uint32_t firstSet, uint32_t descriptorSetCount, const VkDescriptorSet* pDescriptorSets, uint32_t dynamicOffsetCount, const uint32_t* pDynamicOffsets) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdBindDescriptorSets(commandBuffer, pipelineBindPoint, layout, firstSet, descriptorSetCount, pDescriptorSets, dynamicOffsetCount, pDynamicOffsets);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdBindIndexBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkIndexType indexType) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdBindIndexBuffer(commandBuffer, buffer, offset, indexType);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdBindIndexBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkIndexType indexType) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdBindIndexBuffer(commandBuffer, buffer, offset, indexType);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdBindIndexBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkIndexType indexType) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdBindIndexBuffer(commandBuffer, buffer, offset, indexType);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdBindVertexBuffers(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount, const VkBuffer* pBuffers, const VkDeviceSize* pOffsets) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdBindVertexBuffers(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdBindVertexBuffers(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount, const VkBuffer* pBuffers, const VkDeviceSize* pOffsets) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdBindVertexBuffers(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdBindVertexBuffers(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount, const VkBuffer* pBuffers, const VkDeviceSize* pOffsets) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdBindVertexBuffers(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdDraw(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdDraw(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdDraw(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdDrawIndexed(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdDrawIndexed(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdDrawIndexed(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdDrawIndirect(commandBuffer, buffer, offset, drawCount, stride);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdDrawIndirect(commandBuffer, buffer, offset, drawCount, stride);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdDrawIndirect(commandBuffer, buffer, offset, drawCount, stride);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdDrawIndexedIndirect(commandBuffer, buffer, offset, drawCount, stride);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdDrawIndexedIndirect(commandBuffer, buffer, offset, drawCount, stride);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdDrawIndexedIndirect(commandBuffer, buffer, offset, drawCount, stride);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdDispatch(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdDispatch(commandBuffer, groupCountX, groupCountY, groupCountZ);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdDispatch(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdDispatch(commandBuffer, groupCountX, groupCountY, groupCountZ);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdDispatch(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdDispatch(commandBuffer, groupCountX, groupCountY, groupCountZ);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdDispatchIndirect(commandBuffer, buffer, offset);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdDispatchIndirect(commandBuffer, buffer, offset);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdDispatchIndirect(commandBuffer, buffer, offset);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t regionCount, const VkBufferCopy* pRegions) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, regionCount, pRegions);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t regionCount, const VkBufferCopy* pRegions) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, regionCount, pRegions);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t regionCount, const VkBufferCopy* pRegions) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, regionCount, pRegions);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageCopy* pRegions) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdCopyImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageCopy* pRegions) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdCopyImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageCopy* pRegions) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdCopyImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageBlit* pRegions, VkFilter filter) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdBlitImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions, filter);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageBlit* pRegions, VkFilter filter) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdBlitImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions, filter);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageBlit* pRegions, VkFilter filter) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdBlitImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions, filter);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkBufferImageCopy* pRegions) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdCopyBufferToImage(commandBuffer, srcBuffer, dstImage, dstImageLayout, regionCount, pRegions);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkBufferImageCopy* pRegions) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdCopyBufferToImage(commandBuffer, srcBuffer, dstImage, dstImageLayout, regionCount, pRegions);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkBufferImageCopy* pRegions) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdCopyBufferToImage(commandBuffer, srcBuffer, dstImage, dstImageLayout, regionCount, pRegions);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkBuffer dstBuffer, uint32_t regionCount, const VkBufferImageCopy* pRegions) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdCopyImageToBuffer(commandBuffer, srcImage, srcImageLayout, dstBuffer, regionCount, pRegions);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkBuffer dstBuffer, uint32_t regionCount, const VkBufferImageCopy* pRegions) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdCopyImageToBuffer(commandBuffer, srcImage, srcImageLayout, dstBuffer, regionCount, pRegions);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkBuffer dstBuffer, uint32_t regionCount, const VkBufferImageCopy* pRegions) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdCopyImageToBuffer(commandBuffer, srcImage, srcImageLayout, dstBuffer, regionCount, pRegions);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize dataSize, const void* pData) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdUpdateBuffer(commandBuffer, dstBuffer, dstOffset, dataSize, pData);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize dataSize, const void* pData) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdUpdateBuffer(commandBuffer, dstBuffer, dstOffset, dataSize, pData);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize dataSize, const void* pData) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdUpdateBuffer(commandBuffer, dstBuffer, dstOffset, dataSize, pData);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize size, uint32_t data) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdFillBuffer(commandBuffer, dstBuffer, dstOffset, size, data);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize size, uint32_t data) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdFillBuffer(commandBuffer, dstBuffer, dstOffset, size, data);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize size, uint32_t data) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdFillBuffer(commandBuffer, dstBuffer, dstOffset, size, data);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout, const VkClearColorValue* pColor, uint32_t rangeCount, const VkImageSubresourceRange* pRanges) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdClearColorImage(commandBuffer, image, imageLayout, pColor, rangeCount, pRanges);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout, const VkClearColorValue* pColor, uint32_t rangeCount, const VkImageSubresourceRange* pRanges) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdClearColorImage(commandBuffer, image, imageLayout, pColor, rangeCount, pRanges);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout, const VkClearColorValue* pColor, uint32_t rangeCount, const VkImageSubresourceRange* pRanges) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdClearColorImage(commandBuffer, image, imageLayout, pColor, rangeCount, pRanges);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout, const VkClearDepthStencilValue* pDepthStencil, uint32_t rangeCount, const VkImageSubresourceRange* pRanges) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdClearDepthStencilImage(commandBuffer, image, imageLayout, pDepthStencil, rangeCount, pRanges);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout, const VkClearDepthStencilValue* pDepthStencil, uint32_t rangeCount, const VkImageSubresourceRange* pRanges) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdClearDepthStencilImage(commandBuffer, image, imageLayout, pDepthStencil, rangeCount, pRanges);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout, const VkClearDepthStencilValue* pDepthStencil, uint32_t rangeCount, const VkImageSubresourceRange* pRanges) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdClearDepthStencilImage(commandBuffer, image, imageLayout, pDepthStencil, rangeCount, pRanges);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount, const VkClearAttachment* pAttachments, uint32_t rectCount, const VkClearRect* pRects) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdClearAttachments(commandBuffer, attachmentCount, pAttachments, rectCount, pRects);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount, const VkClearAttachment* pAttachments, uint32_t rectCount, const VkClearRect* pRects) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdClearAttachments(commandBuffer, attachmentCount, pAttachments, rectCount, pRects);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount, const VkClearAttachment* pAttachments, uint32_t rectCount, const VkClearRect* pRects) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdClearAttachments(commandBuffer, attachmentCount, pAttachments, rectCount, pRects);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageResolve* pRegions) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdResolveImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageResolve* pRegions) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdResolveImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageResolve* pRegions) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdResolveImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdSetEvent(commandBuffer, event, stageMask);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdSetEvent(commandBuffer, event, stageMask);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdSetEvent(commandBuffer, event, stageMask);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdResetEvent(commandBuffer, event, stageMask);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdResetEvent(commandBuffer, event, stageMask);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdResetEvent(commandBuffer, event, stageMask);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers, uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdWaitEvents(commandBuffer, eventCount, pEvents, srcStageMask, dstStageMask, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers, uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdWaitEvents(commandBuffer, eventCount, pEvents, srcStageMask, dstStageMask, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers, uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdWaitEvents(commandBuffer, eventCount, pEvents, srcStageMask, dstStageMask, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags, uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers, uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdPipelineBarrier(commandBuffer, srcStageMask, dstStageMask, dependencyFlags, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags, uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers, uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdPipelineBarrier(commandBuffer, srcStageMask, dstStageMask, dependencyFlags, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags, uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers, uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdPipelineBarrier(commandBuffer, srcStageMask, dstStageMask, dependencyFlags, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdBeginQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query, VkQueryControlFlags flags) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdBeginQuery(commandBuffer, queryPool, query, flags);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdBeginQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query, VkQueryControlFlags flags) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdBeginQuery(commandBuffer, queryPool, query, flags);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdBeginQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query, VkQueryControlFlags flags) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdBeginQuery(commandBuffer, queryPool, query, flags);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdEndQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdEndQuery(commandBuffer, queryPool, query);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdEndQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdEndQuery(commandBuffer, queryPool, query);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdEndQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdEndQuery(commandBuffer, queryPool, query);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdResetQueryPool(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdResetQueryPool(commandBuffer, queryPool, firstQuery, queryCount);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdResetQueryPool(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdResetQueryPool(commandBuffer, queryPool, firstQuery, queryCount);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdResetQueryPool(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdResetQueryPool(commandBuffer, queryPool, firstQuery, queryCount);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdWriteTimestamp(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage, VkQueryPool queryPool, uint32_t query) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdWriteTimestamp(commandBuffer, pipelineStage, queryPool, query);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdWriteTimestamp(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage, VkQueryPool queryPool, uint32_t query) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdWriteTimestamp(commandBuffer, pipelineStage, queryPool, query);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdWriteTimestamp(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage, VkQueryPool queryPool, uint32_t query) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdWriteTimestamp(commandBuffer, pipelineStage, queryPool, query);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdCopyQueryPoolResults(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize stride, VkQueryResultFlags flags) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdCopyQueryPoolResults(commandBuffer, queryPool, firstQuery, queryCount, dstBuffer, dstOffset, stride, flags);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdCopyQueryPoolResults(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize stride, VkQueryResultFlags flags) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdCopyQueryPoolResults(commandBuffer, queryPool, firstQuery, queryCount, dstBuffer, dstOffset, stride, flags);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdCopyQueryPoolResults(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize stride, VkQueryResultFlags flags) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdCopyQueryPoolResults(commandBuffer, queryPool, firstQuery, queryCount, dstBuffer, dstOffset, stride, flags);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdPushConstants(VkCommandBuffer commandBuffer, VkPipelineLayout layout, VkShaderStageFlags stageFlags, uint32_t offset, uint32_t size, const void* pValues) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdPushConstants(commandBuffer, layout, stageFlags, offset, size, pValues);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdPushConstants(VkCommandBuffer commandBuffer, VkPipelineLayout layout, VkShaderStageFlags stageFlags, uint32_t offset, uint32_t size, const void* pValues) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdPushConstants(commandBuffer, layout, stageFlags, offset, size, pValues);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdPushConstants(VkCommandBuffer commandBuffer, VkPipelineLayout layout, VkShaderStageFlags stageFlags, uint32_t offset, uint32_t size, const void* pValues) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdPushConstants(commandBuffer, layout, stageFlags, offset, size, pValues);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin, VkSubpassContents contents) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdBeginRenderPass(commandBuffer, pRenderPassBegin, contents);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin, VkSubpassContents contents) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdBeginRenderPass(commandBuffer, pRenderPassBegin, contents);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin, VkSubpassContents contents) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdBeginRenderPass(commandBuffer, pRenderPassBegin, contents);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdNextSubpass(commandBuffer, contents);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdNextSubpass(commandBuffer, contents);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdNextSubpass(commandBuffer, contents);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdEndRenderPass(VkCommandBuffer commandBuffer) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdEndRenderPass(commandBuffer);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdEndRenderPass(VkCommandBuffer commandBuffer) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdEndRenderPass(commandBuffer);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdEndRenderPass(VkCommandBuffer commandBuffer) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdEndRenderPass(commandBuffer);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBufferCount, const VkCommandBuffer* pCommandBuffers) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdExecuteCommands(commandBuffer, commandBufferCount, pCommandBuffers);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBufferCount, const VkCommandBuffer* pCommandBuffers) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdExecuteCommands(commandBuffer, commandBufferCount, pCommandBuffers);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBufferCount, const VkCommandBuffer* pCommandBuffers) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdExecuteCommands(commandBuffer, commandBufferCount, pCommandBuffers);
}

bool CoreChecksOptickInstrumented::PreCallValidateBindBufferMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindBufferMemoryInfo* pBindInfos) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateBindBufferMemory2(device, bindInfoCount, pBindInfos);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordBindBufferMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindBufferMemoryInfo* pBindInfos) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordBindBufferMemory2(device, bindInfoCount, pBindInfos);
}

void CoreChecksOptickInstrumented::PostCallRecordBindBufferMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindBufferMemoryInfo* pBindInfos, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordBindBufferMemory2(device, bindInfoCount, pBindInfos, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateBindImageMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo* pBindInfos) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateBindImageMemory2(device, bindInfoCount, pBindInfos);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordBindImageMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo* pBindInfos) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordBindImageMemory2(device, bindInfoCount, pBindInfos);
}

void CoreChecksOptickInstrumented::PostCallRecordBindImageMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo* pBindInfos, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordBindImageMemory2(device, bindInfoCount, pBindInfos, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetDeviceGroupPeerMemoryFeatures(VkDevice device, uint32_t heapIndex, uint32_t localDeviceIndex, uint32_t remoteDeviceIndex, VkPeerMemoryFeatureFlags* pPeerMemoryFeatures) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetDeviceGroupPeerMemoryFeatures(device, heapIndex, localDeviceIndex, remoteDeviceIndex, pPeerMemoryFeatures);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetDeviceGroupPeerMemoryFeatures(VkDevice device, uint32_t heapIndex, uint32_t localDeviceIndex, uint32_t remoteDeviceIndex, VkPeerMemoryFeatureFlags* pPeerMemoryFeatures) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetDeviceGroupPeerMemoryFeatures(device, heapIndex, localDeviceIndex, remoteDeviceIndex, pPeerMemoryFeatures);
}

void CoreChecksOptickInstrumented::PostCallRecordGetDeviceGroupPeerMemoryFeatures(VkDevice device, uint32_t heapIndex, uint32_t localDeviceIndex, uint32_t remoteDeviceIndex, VkPeerMemoryFeatureFlags* pPeerMemoryFeatures) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetDeviceGroupPeerMemoryFeatures(device, heapIndex, localDeviceIndex, remoteDeviceIndex, pPeerMemoryFeatures);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdSetDeviceMask(VkCommandBuffer commandBuffer, uint32_t deviceMask) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdSetDeviceMask(commandBuffer, deviceMask);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdSetDeviceMask(VkCommandBuffer commandBuffer, uint32_t deviceMask) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdSetDeviceMask(commandBuffer, deviceMask);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdSetDeviceMask(VkCommandBuffer commandBuffer, uint32_t deviceMask) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdSetDeviceMask(commandBuffer, deviceMask);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdDispatchBase(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY, uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdDispatchBase(commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdDispatchBase(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY, uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdDispatchBase(commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdDispatchBase(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY, uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdDispatchBase(commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ);
}

bool CoreChecksOptickInstrumented::PreCallValidateEnumeratePhysicalDeviceGroups(VkInstance instance, uint32_t* pPhysicalDeviceGroupCount, VkPhysicalDeviceGroupProperties* pPhysicalDeviceGroupProperties) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateEnumeratePhysicalDeviceGroups(instance, pPhysicalDeviceGroupCount, pPhysicalDeviceGroupProperties);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordEnumeratePhysicalDeviceGroups(VkInstance instance, uint32_t* pPhysicalDeviceGroupCount, VkPhysicalDeviceGroupProperties* pPhysicalDeviceGroupProperties) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordEnumeratePhysicalDeviceGroups(instance, pPhysicalDeviceGroupCount, pPhysicalDeviceGroupProperties);
}

void CoreChecksOptickInstrumented::PostCallRecordEnumeratePhysicalDeviceGroups(VkInstance instance, uint32_t* pPhysicalDeviceGroupCount, VkPhysicalDeviceGroupProperties* pPhysicalDeviceGroupProperties, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordEnumeratePhysicalDeviceGroups(instance, pPhysicalDeviceGroupCount, pPhysicalDeviceGroupProperties, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetImageMemoryRequirements2(VkDevice device, const VkImageMemoryRequirementsInfo2* pInfo, VkMemoryRequirements2* pMemoryRequirements) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetImageMemoryRequirements2(device, pInfo, pMemoryRequirements);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetImageMemoryRequirements2(VkDevice device, const VkImageMemoryRequirementsInfo2* pInfo, VkMemoryRequirements2* pMemoryRequirements) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetImageMemoryRequirements2(device, pInfo, pMemoryRequirements);
}

void CoreChecksOptickInstrumented::PostCallRecordGetImageMemoryRequirements2(VkDevice device, const VkImageMemoryRequirementsInfo2* pInfo, VkMemoryRequirements2* pMemoryRequirements) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetImageMemoryRequirements2(device, pInfo, pMemoryRequirements);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetBufferMemoryRequirements2(VkDevice device, const VkBufferMemoryRequirementsInfo2* pInfo, VkMemoryRequirements2* pMemoryRequirements) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetBufferMemoryRequirements2(device, pInfo, pMemoryRequirements);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetBufferMemoryRequirements2(VkDevice device, const VkBufferMemoryRequirementsInfo2* pInfo, VkMemoryRequirements2* pMemoryRequirements) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetBufferMemoryRequirements2(device, pInfo, pMemoryRequirements);
}

void CoreChecksOptickInstrumented::PostCallRecordGetBufferMemoryRequirements2(VkDevice device, const VkBufferMemoryRequirementsInfo2* pInfo, VkMemoryRequirements2* pMemoryRequirements) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetBufferMemoryRequirements2(device, pInfo, pMemoryRequirements);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetImageSparseMemoryRequirements2(VkDevice device, const VkImageSparseMemoryRequirementsInfo2* pInfo, uint32_t* pSparseMemoryRequirementCount, VkSparseImageMemoryRequirements2* pSparseMemoryRequirements) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetImageSparseMemoryRequirements2(device, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetImageSparseMemoryRequirements2(VkDevice device, const VkImageSparseMemoryRequirementsInfo2* pInfo, uint32_t* pSparseMemoryRequirementCount, VkSparseImageMemoryRequirements2* pSparseMemoryRequirements) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetImageSparseMemoryRequirements2(device, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
}

void CoreChecksOptickInstrumented::PostCallRecordGetImageSparseMemoryRequirements2(VkDevice device, const VkImageSparseMemoryRequirementsInfo2* pInfo, uint32_t* pSparseMemoryRequirementCount, VkSparseImageMemoryRequirements2* pSparseMemoryRequirements) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetImageSparseMemoryRequirements2(device, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetPhysicalDeviceFeatures2(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures2* pFeatures) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetPhysicalDeviceFeatures2(physicalDevice, pFeatures);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetPhysicalDeviceFeatures2(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures2* pFeatures) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetPhysicalDeviceFeatures2(physicalDevice, pFeatures);
}

void CoreChecksOptickInstrumented::PostCallRecordGetPhysicalDeviceFeatures2(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures2* pFeatures) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetPhysicalDeviceFeatures2(physicalDevice, pFeatures);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetPhysicalDeviceProperties2(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties2* pProperties) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetPhysicalDeviceProperties2(physicalDevice, pProperties);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetPhysicalDeviceProperties2(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties2* pProperties) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetPhysicalDeviceProperties2(physicalDevice, pProperties);
}

void CoreChecksOptickInstrumented::PostCallRecordGetPhysicalDeviceProperties2(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties2* pProperties) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetPhysicalDeviceProperties2(physicalDevice, pProperties);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetPhysicalDeviceFormatProperties2(VkPhysicalDevice physicalDevice, VkFormat format, VkFormatProperties2* pFormatProperties) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetPhysicalDeviceFormatProperties2(physicalDevice, format, pFormatProperties);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetPhysicalDeviceFormatProperties2(VkPhysicalDevice physicalDevice, VkFormat format, VkFormatProperties2* pFormatProperties) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetPhysicalDeviceFormatProperties2(physicalDevice, format, pFormatProperties);
}

void CoreChecksOptickInstrumented::PostCallRecordGetPhysicalDeviceFormatProperties2(VkPhysicalDevice physicalDevice, VkFormat format, VkFormatProperties2* pFormatProperties) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetPhysicalDeviceFormatProperties2(physicalDevice, format, pFormatProperties);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetPhysicalDeviceImageFormatProperties2(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceImageFormatInfo2* pImageFormatInfo, VkImageFormatProperties2* pImageFormatProperties) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetPhysicalDeviceImageFormatProperties2(physicalDevice, pImageFormatInfo, pImageFormatProperties);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetPhysicalDeviceImageFormatProperties2(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceImageFormatInfo2* pImageFormatInfo, VkImageFormatProperties2* pImageFormatProperties) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetPhysicalDeviceImageFormatProperties2(physicalDevice, pImageFormatInfo, pImageFormatProperties);
}

void CoreChecksOptickInstrumented::PostCallRecordGetPhysicalDeviceImageFormatProperties2(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceImageFormatInfo2* pImageFormatInfo, VkImageFormatProperties2* pImageFormatProperties, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetPhysicalDeviceImageFormatProperties2(physicalDevice, pImageFormatInfo, pImageFormatProperties, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetPhysicalDeviceQueueFamilyProperties2(VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount, VkQueueFamilyProperties2* pQueueFamilyProperties) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetPhysicalDeviceQueueFamilyProperties2(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetPhysicalDeviceQueueFamilyProperties2(VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount, VkQueueFamilyProperties2* pQueueFamilyProperties) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetPhysicalDeviceQueueFamilyProperties2(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
}

void CoreChecksOptickInstrumented::PostCallRecordGetPhysicalDeviceQueueFamilyProperties2(VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount, VkQueueFamilyProperties2* pQueueFamilyProperties) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetPhysicalDeviceQueueFamilyProperties2(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetPhysicalDeviceMemoryProperties2(VkPhysicalDevice physicalDevice, VkPhysicalDeviceMemoryProperties2* pMemoryProperties) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetPhysicalDeviceMemoryProperties2(physicalDevice, pMemoryProperties);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetPhysicalDeviceMemoryProperties2(VkPhysicalDevice physicalDevice, VkPhysicalDeviceMemoryProperties2* pMemoryProperties) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetPhysicalDeviceMemoryProperties2(physicalDevice, pMemoryProperties);
}

void CoreChecksOptickInstrumented::PostCallRecordGetPhysicalDeviceMemoryProperties2(VkPhysicalDevice physicalDevice, VkPhysicalDeviceMemoryProperties2* pMemoryProperties) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetPhysicalDeviceMemoryProperties2(physicalDevice, pMemoryProperties);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetPhysicalDeviceSparseImageFormatProperties2(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSparseImageFormatInfo2* pFormatInfo, uint32_t* pPropertyCount, VkSparseImageFormatProperties2* pProperties) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetPhysicalDeviceSparseImageFormatProperties2(physicalDevice, pFormatInfo, pPropertyCount, pProperties);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetPhysicalDeviceSparseImageFormatProperties2(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSparseImageFormatInfo2* pFormatInfo, uint32_t* pPropertyCount, VkSparseImageFormatProperties2* pProperties) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetPhysicalDeviceSparseImageFormatProperties2(physicalDevice, pFormatInfo, pPropertyCount, pProperties);
}

void CoreChecksOptickInstrumented::PostCallRecordGetPhysicalDeviceSparseImageFormatProperties2(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSparseImageFormatInfo2* pFormatInfo, uint32_t* pPropertyCount, VkSparseImageFormatProperties2* pProperties) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetPhysicalDeviceSparseImageFormatProperties2(physicalDevice, pFormatInfo, pPropertyCount, pProperties);
}

bool CoreChecksOptickInstrumented::PreCallValidateTrimCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolTrimFlags flags) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateTrimCommandPool(device, commandPool, flags);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordTrimCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolTrimFlags flags) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordTrimCommandPool(device, commandPool, flags);
}

void CoreChecksOptickInstrumented::PostCallRecordTrimCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolTrimFlags flags) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordTrimCommandPool(device, commandPool, flags);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetDeviceQueue2(VkDevice device, const VkDeviceQueueInfo2* pQueueInfo, VkQueue* pQueue) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetDeviceQueue2(device, pQueueInfo, pQueue);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetDeviceQueue2(VkDevice device, const VkDeviceQueueInfo2* pQueueInfo, VkQueue* pQueue) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetDeviceQueue2(device, pQueueInfo, pQueue);
}

void CoreChecksOptickInstrumented::PostCallRecordGetDeviceQueue2(VkDevice device, const VkDeviceQueueInfo2* pQueueInfo, VkQueue* pQueue) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetDeviceQueue2(device, pQueueInfo, pQueue);
}

bool CoreChecksOptickInstrumented::PreCallValidateCreateSamplerYcbcrConversion(VkDevice device, const VkSamplerYcbcrConversionCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSamplerYcbcrConversion* pYcbcrConversion) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCreateSamplerYcbcrConversion(device, pCreateInfo, pAllocator, pYcbcrConversion);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCreateSamplerYcbcrConversion(VkDevice device, const VkSamplerYcbcrConversionCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSamplerYcbcrConversion* pYcbcrConversion) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCreateSamplerYcbcrConversion(device, pCreateInfo, pAllocator, pYcbcrConversion);
}

void CoreChecksOptickInstrumented::PostCallRecordCreateSamplerYcbcrConversion(VkDevice device, const VkSamplerYcbcrConversionCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSamplerYcbcrConversion* pYcbcrConversion, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCreateSamplerYcbcrConversion(device, pCreateInfo, pAllocator, pYcbcrConversion, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateDestroySamplerYcbcrConversion(VkDevice device, VkSamplerYcbcrConversion ycbcrConversion, const VkAllocationCallbacks* pAllocator) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateDestroySamplerYcbcrConversion(device, ycbcrConversion, pAllocator);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordDestroySamplerYcbcrConversion(VkDevice device, VkSamplerYcbcrConversion ycbcrConversion, const VkAllocationCallbacks* pAllocator) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordDestroySamplerYcbcrConversion(device, ycbcrConversion, pAllocator);
}

void CoreChecksOptickInstrumented::PostCallRecordDestroySamplerYcbcrConversion(VkDevice device, VkSamplerYcbcrConversion ycbcrConversion, const VkAllocationCallbacks* pAllocator) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordDestroySamplerYcbcrConversion(device, ycbcrConversion, pAllocator);
}

bool CoreChecksOptickInstrumented::PreCallValidateCreateDescriptorUpdateTemplate(VkDevice device, const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDescriptorUpdateTemplate* pDescriptorUpdateTemplate) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCreateDescriptorUpdateTemplate(device, pCreateInfo, pAllocator, pDescriptorUpdateTemplate);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCreateDescriptorUpdateTemplate(VkDevice device, const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDescriptorUpdateTemplate* pDescriptorUpdateTemplate) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCreateDescriptorUpdateTemplate(device, pCreateInfo, pAllocator, pDescriptorUpdateTemplate);
}

void CoreChecksOptickInstrumented::PostCallRecordCreateDescriptorUpdateTemplate(VkDevice device, const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDescriptorUpdateTemplate* pDescriptorUpdateTemplate, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCreateDescriptorUpdateTemplate(device, pCreateInfo, pAllocator, pDescriptorUpdateTemplate, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateDestroyDescriptorUpdateTemplate(VkDevice device, VkDescriptorUpdateTemplate descriptorUpdateTemplate, const VkAllocationCallbacks* pAllocator) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateDestroyDescriptorUpdateTemplate(device, descriptorUpdateTemplate, pAllocator);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordDestroyDescriptorUpdateTemplate(VkDevice device, VkDescriptorUpdateTemplate descriptorUpdateTemplate, const VkAllocationCallbacks* pAllocator) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordDestroyDescriptorUpdateTemplate(device, descriptorUpdateTemplate, pAllocator);
}

void CoreChecksOptickInstrumented::PostCallRecordDestroyDescriptorUpdateTemplate(VkDevice device, VkDescriptorUpdateTemplate descriptorUpdateTemplate, const VkAllocationCallbacks* pAllocator) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordDestroyDescriptorUpdateTemplate(device, descriptorUpdateTemplate, pAllocator);
}

bool CoreChecksOptickInstrumented::PreCallValidateUpdateDescriptorSetWithTemplate(VkDevice device, VkDescriptorSet descriptorSet, VkDescriptorUpdateTemplate descriptorUpdateTemplate, const void* pData) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateUpdateDescriptorSetWithTemplate(device, descriptorSet, descriptorUpdateTemplate, pData);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordUpdateDescriptorSetWithTemplate(VkDevice device, VkDescriptorSet descriptorSet, VkDescriptorUpdateTemplate descriptorUpdateTemplate, const void* pData) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordUpdateDescriptorSetWithTemplate(device, descriptorSet, descriptorUpdateTemplate, pData);
}

void CoreChecksOptickInstrumented::PostCallRecordUpdateDescriptorSetWithTemplate(VkDevice device, VkDescriptorSet descriptorSet, VkDescriptorUpdateTemplate descriptorUpdateTemplate, const void* pData) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordUpdateDescriptorSetWithTemplate(device, descriptorSet, descriptorUpdateTemplate, pData);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetPhysicalDeviceExternalBufferProperties(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalBufferInfo* pExternalBufferInfo, VkExternalBufferProperties* pExternalBufferProperties) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetPhysicalDeviceExternalBufferProperties(physicalDevice, pExternalBufferInfo, pExternalBufferProperties);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetPhysicalDeviceExternalBufferProperties(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalBufferInfo* pExternalBufferInfo, VkExternalBufferProperties* pExternalBufferProperties) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetPhysicalDeviceExternalBufferProperties(physicalDevice, pExternalBufferInfo, pExternalBufferProperties);
}

void CoreChecksOptickInstrumented::PostCallRecordGetPhysicalDeviceExternalBufferProperties(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalBufferInfo* pExternalBufferInfo, VkExternalBufferProperties* pExternalBufferProperties) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetPhysicalDeviceExternalBufferProperties(physicalDevice, pExternalBufferInfo, pExternalBufferProperties);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetPhysicalDeviceExternalFenceProperties(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalFenceInfo* pExternalFenceInfo, VkExternalFenceProperties* pExternalFenceProperties) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetPhysicalDeviceExternalFenceProperties(physicalDevice, pExternalFenceInfo, pExternalFenceProperties);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetPhysicalDeviceExternalFenceProperties(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalFenceInfo* pExternalFenceInfo, VkExternalFenceProperties* pExternalFenceProperties) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetPhysicalDeviceExternalFenceProperties(physicalDevice, pExternalFenceInfo, pExternalFenceProperties);
}

void CoreChecksOptickInstrumented::PostCallRecordGetPhysicalDeviceExternalFenceProperties(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalFenceInfo* pExternalFenceInfo, VkExternalFenceProperties* pExternalFenceProperties) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetPhysicalDeviceExternalFenceProperties(physicalDevice, pExternalFenceInfo, pExternalFenceProperties);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetPhysicalDeviceExternalSemaphoreProperties(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalSemaphoreInfo* pExternalSemaphoreInfo, VkExternalSemaphoreProperties* pExternalSemaphoreProperties) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetPhysicalDeviceExternalSemaphoreProperties(physicalDevice, pExternalSemaphoreInfo, pExternalSemaphoreProperties);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetPhysicalDeviceExternalSemaphoreProperties(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalSemaphoreInfo* pExternalSemaphoreInfo, VkExternalSemaphoreProperties* pExternalSemaphoreProperties) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetPhysicalDeviceExternalSemaphoreProperties(physicalDevice, pExternalSemaphoreInfo, pExternalSemaphoreProperties);
}

void CoreChecksOptickInstrumented::PostCallRecordGetPhysicalDeviceExternalSemaphoreProperties(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalSemaphoreInfo* pExternalSemaphoreInfo, VkExternalSemaphoreProperties* pExternalSemaphoreProperties) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetPhysicalDeviceExternalSemaphoreProperties(physicalDevice, pExternalSemaphoreInfo, pExternalSemaphoreProperties);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetDescriptorSetLayoutSupport(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo, VkDescriptorSetLayoutSupport* pSupport) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetDescriptorSetLayoutSupport(device, pCreateInfo, pSupport);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetDescriptorSetLayoutSupport(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo, VkDescriptorSetLayoutSupport* pSupport) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetDescriptorSetLayoutSupport(device, pCreateInfo, pSupport);
}

void CoreChecksOptickInstrumented::PostCallRecordGetDescriptorSetLayoutSupport(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo, VkDescriptorSetLayoutSupport* pSupport) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetDescriptorSetLayoutSupport(device, pCreateInfo, pSupport);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
}

bool CoreChecksOptickInstrumented::PreCallValidateCreateRenderPass2(VkDevice device, const VkRenderPassCreateInfo2* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCreateRenderPass2(device, pCreateInfo, pAllocator, pRenderPass);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCreateRenderPass2(VkDevice device, const VkRenderPassCreateInfo2* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCreateRenderPass2(device, pCreateInfo, pAllocator, pRenderPass);
}

void CoreChecksOptickInstrumented::PostCallRecordCreateRenderPass2(VkDevice device, const VkRenderPassCreateInfo2* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCreateRenderPass2(device, pCreateInfo, pAllocator, pRenderPass, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdBeginRenderPass2(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo*      pRenderPassBegin, const VkSubpassBeginInfo*      pSubpassBeginInfo) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdBeginRenderPass2(commandBuffer, pRenderPassBegin, pSubpassBeginInfo);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdBeginRenderPass2(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo*      pRenderPassBegin, const VkSubpassBeginInfo*      pSubpassBeginInfo) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdBeginRenderPass2(commandBuffer, pRenderPassBegin, pSubpassBeginInfo);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdBeginRenderPass2(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo*      pRenderPassBegin, const VkSubpassBeginInfo*      pSubpassBeginInfo) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdBeginRenderPass2(commandBuffer, pRenderPassBegin, pSubpassBeginInfo);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdNextSubpass2(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo*      pSubpassBeginInfo, const VkSubpassEndInfo*        pSubpassEndInfo) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdNextSubpass2(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdNextSubpass2(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo*      pSubpassBeginInfo, const VkSubpassEndInfo*        pSubpassEndInfo) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdNextSubpass2(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdNextSubpass2(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo*      pSubpassBeginInfo, const VkSubpassEndInfo*        pSubpassEndInfo) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdNextSubpass2(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdEndRenderPass2(VkCommandBuffer commandBuffer, const VkSubpassEndInfo*        pSubpassEndInfo) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdEndRenderPass2(commandBuffer, pSubpassEndInfo);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdEndRenderPass2(VkCommandBuffer commandBuffer, const VkSubpassEndInfo*        pSubpassEndInfo) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdEndRenderPass2(commandBuffer, pSubpassEndInfo);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdEndRenderPass2(VkCommandBuffer commandBuffer, const VkSubpassEndInfo*        pSubpassEndInfo) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdEndRenderPass2(commandBuffer, pSubpassEndInfo);
}

bool CoreChecksOptickInstrumented::PreCallValidateResetQueryPool(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateResetQueryPool(device, queryPool, firstQuery, queryCount);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordResetQueryPool(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordResetQueryPool(device, queryPool, firstQuery, queryCount);
}

void CoreChecksOptickInstrumented::PostCallRecordResetQueryPool(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordResetQueryPool(device, queryPool, firstQuery, queryCount);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetSemaphoreCounterValue(VkDevice device, VkSemaphore semaphore, uint64_t* pValue) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetSemaphoreCounterValue(device, semaphore, pValue);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetSemaphoreCounterValue(VkDevice device, VkSemaphore semaphore, uint64_t* pValue) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetSemaphoreCounterValue(device, semaphore, pValue);
}

void CoreChecksOptickInstrumented::PostCallRecordGetSemaphoreCounterValue(VkDevice device, VkSemaphore semaphore, uint64_t* pValue, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetSemaphoreCounterValue(device, semaphore, pValue, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateWaitSemaphores(VkDevice device, const VkSemaphoreWaitInfo* pWaitInfo, uint64_t timeout) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateWaitSemaphores(device, pWaitInfo, timeout);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordWaitSemaphores(VkDevice device, const VkSemaphoreWaitInfo* pWaitInfo, uint64_t timeout) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordWaitSemaphores(device, pWaitInfo, timeout);
}

void CoreChecksOptickInstrumented::PostCallRecordWaitSemaphores(VkDevice device, const VkSemaphoreWaitInfo* pWaitInfo, uint64_t timeout, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordWaitSemaphores(device, pWaitInfo, timeout, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateSignalSemaphore(VkDevice device, const VkSemaphoreSignalInfo* pSignalInfo) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateSignalSemaphore(device, pSignalInfo);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordSignalSemaphore(VkDevice device, const VkSemaphoreSignalInfo* pSignalInfo) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordSignalSemaphore(device, pSignalInfo);
}

void CoreChecksOptickInstrumented::PostCallRecordSignalSemaphore(VkDevice device, const VkSemaphoreSignalInfo* pSignalInfo, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordSignalSemaphore(device, pSignalInfo, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetBufferDeviceAddress(VkDevice device, const VkBufferDeviceAddressInfo* pInfo) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetBufferDeviceAddress(device, pInfo);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetBufferDeviceAddress(VkDevice device, const VkBufferDeviceAddressInfo* pInfo) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetBufferDeviceAddress(device, pInfo);
}

void CoreChecksOptickInstrumented::PostCallRecordGetBufferDeviceAddress(VkDevice device, const VkBufferDeviceAddressInfo* pInfo, VkDeviceAddress result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetBufferDeviceAddress(device, pInfo, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetBufferOpaqueCaptureAddress(VkDevice device, const VkBufferDeviceAddressInfo* pInfo) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetBufferOpaqueCaptureAddress(device, pInfo);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetBufferOpaqueCaptureAddress(VkDevice device, const VkBufferDeviceAddressInfo* pInfo) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetBufferOpaqueCaptureAddress(device, pInfo);
}

void CoreChecksOptickInstrumented::PostCallRecordGetBufferOpaqueCaptureAddress(VkDevice device, const VkBufferDeviceAddressInfo* pInfo) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetBufferOpaqueCaptureAddress(device, pInfo);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetDeviceMemoryOpaqueCaptureAddress(VkDevice device, const VkDeviceMemoryOpaqueCaptureAddressInfo* pInfo) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetDeviceMemoryOpaqueCaptureAddress(device, pInfo);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetDeviceMemoryOpaqueCaptureAddress(VkDevice device, const VkDeviceMemoryOpaqueCaptureAddressInfo* pInfo) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetDeviceMemoryOpaqueCaptureAddress(device, pInfo);
}

void CoreChecksOptickInstrumented::PostCallRecordGetDeviceMemoryOpaqueCaptureAddress(VkDevice device, const VkDeviceMemoryOpaqueCaptureAddressInfo* pInfo) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetDeviceMemoryOpaqueCaptureAddress(device, pInfo);
}

bool CoreChecksOptickInstrumented::PreCallValidateDestroySurfaceKHR(VkInstance instance, VkSurfaceKHR surface, const VkAllocationCallbacks* pAllocator) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateDestroySurfaceKHR(instance, surface, pAllocator);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordDestroySurfaceKHR(VkInstance instance, VkSurfaceKHR surface, const VkAllocationCallbacks* pAllocator) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordDestroySurfaceKHR(instance, surface, pAllocator);
}

void CoreChecksOptickInstrumented::PostCallRecordDestroySurfaceKHR(VkInstance instance, VkSurfaceKHR surface, const VkAllocationCallbacks* pAllocator) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordDestroySurfaceKHR(instance, surface, pAllocator);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetPhysicalDeviceSurfaceSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, VkSurfaceKHR surface, VkBool32* pSupported) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, queueFamilyIndex, surface, pSupported);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetPhysicalDeviceSurfaceSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, VkSurfaceKHR surface, VkBool32* pSupported) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, queueFamilyIndex, surface, pSupported);
}

void CoreChecksOptickInstrumented::PostCallRecordGetPhysicalDeviceSurfaceSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, VkSurfaceKHR surface, VkBool32* pSupported, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, queueFamilyIndex, surface, pSupported, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetPhysicalDeviceSurfaceCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkSurfaceCapabilitiesKHR* pSurfaceCapabilities) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, pSurfaceCapabilities);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetPhysicalDeviceSurfaceCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkSurfaceCapabilitiesKHR* pSurfaceCapabilities) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, pSurfaceCapabilities);
}

void CoreChecksOptickInstrumented::PostCallRecordGetPhysicalDeviceSurfaceCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkSurfaceCapabilitiesKHR* pSurfaceCapabilities, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, pSurfaceCapabilities, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t* pSurfaceFormatCount, VkSurfaceFormatKHR* pSurfaceFormats) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, pSurfaceFormatCount, pSurfaceFormats);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t* pSurfaceFormatCount, VkSurfaceFormatKHR* pSurfaceFormats) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, pSurfaceFormatCount, pSurfaceFormats);
}

void CoreChecksOptickInstrumented::PostCallRecordGetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t* pSurfaceFormatCount, VkSurfaceFormatKHR* pSurfaceFormats, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, pSurfaceFormatCount, pSurfaceFormats, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t* pPresentModeCount, VkPresentModeKHR* pPresentModes) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, pPresentModeCount, pPresentModes);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t* pPresentModeCount, VkPresentModeKHR* pPresentModes) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, pPresentModeCount, pPresentModes);
}

void CoreChecksOptickInstrumented::PostCallRecordGetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t* pPresentModeCount, VkPresentModeKHR* pPresentModes, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, pPresentModeCount, pPresentModes, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCreateSwapchainKHR(device, pCreateInfo, pAllocator, pSwapchain);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCreateSwapchainKHR(device, pCreateInfo, pAllocator, pSwapchain);
}

void CoreChecksOptickInstrumented::PostCallRecordCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCreateSwapchainKHR(device, pCreateInfo, pAllocator, pSwapchain, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateDestroySwapchainKHR(VkDevice device, VkSwapchainKHR swapchain, const VkAllocationCallbacks* pAllocator) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateDestroySwapchainKHR(device, swapchain, pAllocator);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordDestroySwapchainKHR(VkDevice device, VkSwapchainKHR swapchain, const VkAllocationCallbacks* pAllocator) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordDestroySwapchainKHR(device, swapchain, pAllocator);
}

void CoreChecksOptickInstrumented::PostCallRecordDestroySwapchainKHR(VkDevice device, VkSwapchainKHR swapchain, const VkAllocationCallbacks* pAllocator) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordDestroySwapchainKHR(device, swapchain, pAllocator);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapchain, uint32_t* pSwapchainImageCount, VkImage* pSwapchainImages) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetSwapchainImagesKHR(device, swapchain, pSwapchainImageCount, pSwapchainImages);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapchain, uint32_t* pSwapchainImageCount, VkImage* pSwapchainImages) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetSwapchainImagesKHR(device, swapchain, pSwapchainImageCount, pSwapchainImages);
}

void CoreChecksOptickInstrumented::PostCallRecordGetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapchain, uint32_t* pSwapchainImageCount, VkImage* pSwapchainImages, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetSwapchainImagesKHR(device, swapchain, pSwapchainImageCount, pSwapchainImages, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateAcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore, VkFence fence, uint32_t* pImageIndex) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateAcquireNextImageKHR(device, swapchain, timeout, semaphore, fence, pImageIndex);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordAcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore, VkFence fence, uint32_t* pImageIndex) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordAcquireNextImageKHR(device, swapchain, timeout, semaphore, fence, pImageIndex);
}

void CoreChecksOptickInstrumented::PostCallRecordAcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore, VkFence fence, uint32_t* pImageIndex, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordAcquireNextImageKHR(device, swapchain, timeout, semaphore, fence, pImageIndex, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetDeviceGroupPresentCapabilitiesKHR(VkDevice device, VkDeviceGroupPresentCapabilitiesKHR* pDeviceGroupPresentCapabilities) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetDeviceGroupPresentCapabilitiesKHR(device, pDeviceGroupPresentCapabilities);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetDeviceGroupPresentCapabilitiesKHR(VkDevice device, VkDeviceGroupPresentCapabilitiesKHR* pDeviceGroupPresentCapabilities) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetDeviceGroupPresentCapabilitiesKHR(device, pDeviceGroupPresentCapabilities);
}

void CoreChecksOptickInstrumented::PostCallRecordGetDeviceGroupPresentCapabilitiesKHR(VkDevice device, VkDeviceGroupPresentCapabilitiesKHR* pDeviceGroupPresentCapabilities, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetDeviceGroupPresentCapabilitiesKHR(device, pDeviceGroupPresentCapabilities, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetDeviceGroupSurfacePresentModesKHR(VkDevice device, VkSurfaceKHR surface, VkDeviceGroupPresentModeFlagsKHR* pModes) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetDeviceGroupSurfacePresentModesKHR(device, surface, pModes);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetDeviceGroupSurfacePresentModesKHR(VkDevice device, VkSurfaceKHR surface, VkDeviceGroupPresentModeFlagsKHR* pModes) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetDeviceGroupSurfacePresentModesKHR(device, surface, pModes);
}

void CoreChecksOptickInstrumented::PostCallRecordGetDeviceGroupSurfacePresentModesKHR(VkDevice device, VkSurfaceKHR surface, VkDeviceGroupPresentModeFlagsKHR* pModes, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetDeviceGroupSurfacePresentModesKHR(device, surface, pModes, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetPhysicalDevicePresentRectanglesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t* pRectCount, VkRect2D* pRects) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetPhysicalDevicePresentRectanglesKHR(physicalDevice, surface, pRectCount, pRects);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetPhysicalDevicePresentRectanglesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t* pRectCount, VkRect2D* pRects) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetPhysicalDevicePresentRectanglesKHR(physicalDevice, surface, pRectCount, pRects);
}

void CoreChecksOptickInstrumented::PostCallRecordGetPhysicalDevicePresentRectanglesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t* pRectCount, VkRect2D* pRects, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetPhysicalDevicePresentRectanglesKHR(physicalDevice, surface, pRectCount, pRects, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateAcquireNextImage2KHR(VkDevice device, const VkAcquireNextImageInfoKHR* pAcquireInfo, uint32_t* pImageIndex) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateAcquireNextImage2KHR(device, pAcquireInfo, pImageIndex);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordAcquireNextImage2KHR(VkDevice device, const VkAcquireNextImageInfoKHR* pAcquireInfo, uint32_t* pImageIndex) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordAcquireNextImage2KHR(device, pAcquireInfo, pImageIndex);
}

void CoreChecksOptickInstrumented::PostCallRecordAcquireNextImage2KHR(VkDevice device, const VkAcquireNextImageInfoKHR* pAcquireInfo, uint32_t* pImageIndex, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordAcquireNextImage2KHR(device, pAcquireInfo, pImageIndex, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetPhysicalDeviceDisplayPropertiesKHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount, VkDisplayPropertiesKHR* pProperties) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetPhysicalDeviceDisplayPropertiesKHR(physicalDevice, pPropertyCount, pProperties);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetPhysicalDeviceDisplayPropertiesKHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount, VkDisplayPropertiesKHR* pProperties) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetPhysicalDeviceDisplayPropertiesKHR(physicalDevice, pPropertyCount, pProperties);
}

void CoreChecksOptickInstrumented::PostCallRecordGetPhysicalDeviceDisplayPropertiesKHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount, VkDisplayPropertiesKHR* pProperties, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetPhysicalDeviceDisplayPropertiesKHR(physicalDevice, pPropertyCount, pProperties, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetPhysicalDeviceDisplayPlanePropertiesKHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount, VkDisplayPlanePropertiesKHR* pProperties) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetPhysicalDeviceDisplayPlanePropertiesKHR(physicalDevice, pPropertyCount, pProperties);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetPhysicalDeviceDisplayPlanePropertiesKHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount, VkDisplayPlanePropertiesKHR* pProperties) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetPhysicalDeviceDisplayPlanePropertiesKHR(physicalDevice, pPropertyCount, pProperties);
}

void CoreChecksOptickInstrumented::PostCallRecordGetPhysicalDeviceDisplayPlanePropertiesKHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount, VkDisplayPlanePropertiesKHR* pProperties, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetPhysicalDeviceDisplayPlanePropertiesKHR(physicalDevice, pPropertyCount, pProperties, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetDisplayPlaneSupportedDisplaysKHR(VkPhysicalDevice physicalDevice, uint32_t planeIndex, uint32_t* pDisplayCount, VkDisplayKHR* pDisplays) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetDisplayPlaneSupportedDisplaysKHR(physicalDevice, planeIndex, pDisplayCount, pDisplays);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetDisplayPlaneSupportedDisplaysKHR(VkPhysicalDevice physicalDevice, uint32_t planeIndex, uint32_t* pDisplayCount, VkDisplayKHR* pDisplays) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetDisplayPlaneSupportedDisplaysKHR(physicalDevice, planeIndex, pDisplayCount, pDisplays);
}

void CoreChecksOptickInstrumented::PostCallRecordGetDisplayPlaneSupportedDisplaysKHR(VkPhysicalDevice physicalDevice, uint32_t planeIndex, uint32_t* pDisplayCount, VkDisplayKHR* pDisplays, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetDisplayPlaneSupportedDisplaysKHR(physicalDevice, planeIndex, pDisplayCount, pDisplays, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetDisplayModePropertiesKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display, uint32_t* pPropertyCount, VkDisplayModePropertiesKHR* pProperties) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetDisplayModePropertiesKHR(physicalDevice, display, pPropertyCount, pProperties);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetDisplayModePropertiesKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display, uint32_t* pPropertyCount, VkDisplayModePropertiesKHR* pProperties) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetDisplayModePropertiesKHR(physicalDevice, display, pPropertyCount, pProperties);
}

void CoreChecksOptickInstrumented::PostCallRecordGetDisplayModePropertiesKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display, uint32_t* pPropertyCount, VkDisplayModePropertiesKHR* pProperties, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetDisplayModePropertiesKHR(physicalDevice, display, pPropertyCount, pProperties, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateCreateDisplayModeKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display, const VkDisplayModeCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDisplayModeKHR* pMode) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCreateDisplayModeKHR(physicalDevice, display, pCreateInfo, pAllocator, pMode);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCreateDisplayModeKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display, const VkDisplayModeCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDisplayModeKHR* pMode) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCreateDisplayModeKHR(physicalDevice, display, pCreateInfo, pAllocator, pMode);
}

void CoreChecksOptickInstrumented::PostCallRecordCreateDisplayModeKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display, const VkDisplayModeCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDisplayModeKHR* pMode, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCreateDisplayModeKHR(physicalDevice, display, pCreateInfo, pAllocator, pMode, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetDisplayPlaneCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkDisplayModeKHR mode, uint32_t planeIndex, VkDisplayPlaneCapabilitiesKHR* pCapabilities) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetDisplayPlaneCapabilitiesKHR(physicalDevice, mode, planeIndex, pCapabilities);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetDisplayPlaneCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkDisplayModeKHR mode, uint32_t planeIndex, VkDisplayPlaneCapabilitiesKHR* pCapabilities) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetDisplayPlaneCapabilitiesKHR(physicalDevice, mode, planeIndex, pCapabilities);
}

void CoreChecksOptickInstrumented::PostCallRecordGetDisplayPlaneCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkDisplayModeKHR mode, uint32_t planeIndex, VkDisplayPlaneCapabilitiesKHR* pCapabilities, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetDisplayPlaneCapabilitiesKHR(physicalDevice, mode, planeIndex, pCapabilities, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateCreateDisplayPlaneSurfaceKHR(VkInstance instance, const VkDisplaySurfaceCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCreateDisplayPlaneSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCreateDisplayPlaneSurfaceKHR(VkInstance instance, const VkDisplaySurfaceCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCreateDisplayPlaneSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
}

void CoreChecksOptickInstrumented::PostCallRecordCreateDisplayPlaneSurfaceKHR(VkInstance instance, const VkDisplaySurfaceCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCreateDisplayPlaneSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateCreateSharedSwapchainsKHR(VkDevice device, uint32_t swapchainCount, const VkSwapchainCreateInfoKHR* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchains) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCreateSharedSwapchainsKHR(device, swapchainCount, pCreateInfos, pAllocator, pSwapchains);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCreateSharedSwapchainsKHR(VkDevice device, uint32_t swapchainCount, const VkSwapchainCreateInfoKHR* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchains) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCreateSharedSwapchainsKHR(device, swapchainCount, pCreateInfos, pAllocator, pSwapchains);
}

void CoreChecksOptickInstrumented::PostCallRecordCreateSharedSwapchainsKHR(VkDevice device, uint32_t swapchainCount, const VkSwapchainCreateInfoKHR* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchains, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCreateSharedSwapchainsKHR(device, swapchainCount, pCreateInfos, pAllocator, pSwapchains, result);
}

#ifdef VK_USE_PLATFORM_XLIB_KHR
bool CoreChecksOptickInstrumented::PreCallValidateCreateXlibSurfaceKHR(VkInstance instance, const VkXlibSurfaceCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCreateXlibSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCreateXlibSurfaceKHR(VkInstance instance, const VkXlibSurfaceCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCreateXlibSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
}

void CoreChecksOptickInstrumented::PostCallRecordCreateXlibSurfaceKHR(VkInstance instance, const VkXlibSurfaceCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCreateXlibSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface, result);
}

#endif // VK_USE_PLATFORM_XLIB_KHR
#ifdef VK_USE_PLATFORM_XLIB_KHR
bool CoreChecksOptickInstrumented::PreCallValidateGetPhysicalDeviceXlibPresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, Display* dpy, VisualID visualID) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetPhysicalDeviceXlibPresentationSupportKHR(physicalDevice, queueFamilyIndex, dpy, visualID);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetPhysicalDeviceXlibPresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, Display* dpy, VisualID visualID) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetPhysicalDeviceXlibPresentationSupportKHR(physicalDevice, queueFamilyIndex, dpy, visualID);
}

void CoreChecksOptickInstrumented::PostCallRecordGetPhysicalDeviceXlibPresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, Display* dpy, VisualID visualID) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetPhysicalDeviceXlibPresentationSupportKHR(physicalDevice, queueFamilyIndex, dpy, visualID);
}

#endif // VK_USE_PLATFORM_XLIB_KHR
#ifdef VK_USE_PLATFORM_XCB_KHR
bool CoreChecksOptickInstrumented::PreCallValidateCreateXcbSurfaceKHR(VkInstance instance, const VkXcbSurfaceCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCreateXcbSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCreateXcbSurfaceKHR(VkInstance instance, const VkXcbSurfaceCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCreateXcbSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
}

void CoreChecksOptickInstrumented::PostCallRecordCreateXcbSurfaceKHR(VkInstance instance, const VkXcbSurfaceCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCreateXcbSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface, result);
}

#endif // VK_USE_PLATFORM_XCB_KHR
#ifdef VK_USE_PLATFORM_XCB_KHR
bool CoreChecksOptickInstrumented::PreCallValidateGetPhysicalDeviceXcbPresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, xcb_connection_t* connection, xcb_visualid_t visual_id) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetPhysicalDeviceXcbPresentationSupportKHR(physicalDevice, queueFamilyIndex, connection, visual_id);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetPhysicalDeviceXcbPresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, xcb_connection_t* connection, xcb_visualid_t visual_id) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetPhysicalDeviceXcbPresentationSupportKHR(physicalDevice, queueFamilyIndex, connection, visual_id);
}

void CoreChecksOptickInstrumented::PostCallRecordGetPhysicalDeviceXcbPresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, xcb_connection_t* connection, xcb_visualid_t visual_id) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetPhysicalDeviceXcbPresentationSupportKHR(physicalDevice, queueFamilyIndex, connection, visual_id);
}

#endif // VK_USE_PLATFORM_XCB_KHR
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
bool CoreChecksOptickInstrumented::PreCallValidateCreateWaylandSurfaceKHR(VkInstance instance, const VkWaylandSurfaceCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCreateWaylandSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCreateWaylandSurfaceKHR(VkInstance instance, const VkWaylandSurfaceCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCreateWaylandSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
}

void CoreChecksOptickInstrumented::PostCallRecordCreateWaylandSurfaceKHR(VkInstance instance, const VkWaylandSurfaceCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCreateWaylandSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface, result);
}

#endif // VK_USE_PLATFORM_WAYLAND_KHR
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
bool CoreChecksOptickInstrumented::PreCallValidateGetPhysicalDeviceWaylandPresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, struct wl_display* display) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetPhysicalDeviceWaylandPresentationSupportKHR(physicalDevice, queueFamilyIndex, display);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetPhysicalDeviceWaylandPresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, struct wl_display* display) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetPhysicalDeviceWaylandPresentationSupportKHR(physicalDevice, queueFamilyIndex, display);
}

void CoreChecksOptickInstrumented::PostCallRecordGetPhysicalDeviceWaylandPresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, struct wl_display* display) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetPhysicalDeviceWaylandPresentationSupportKHR(physicalDevice, queueFamilyIndex, display);
}

#endif // VK_USE_PLATFORM_WAYLAND_KHR
#ifdef VK_USE_PLATFORM_ANDROID_KHR
bool CoreChecksOptickInstrumented::PreCallValidateCreateAndroidSurfaceKHR(VkInstance instance, const VkAndroidSurfaceCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCreateAndroidSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCreateAndroidSurfaceKHR(VkInstance instance, const VkAndroidSurfaceCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCreateAndroidSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
}

void CoreChecksOptickInstrumented::PostCallRecordCreateAndroidSurfaceKHR(VkInstance instance, const VkAndroidSurfaceCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCreateAndroidSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface, result);
}

#endif // VK_USE_PLATFORM_ANDROID_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
bool CoreChecksOptickInstrumented::PreCallValidateCreateWin32SurfaceKHR(VkInstance instance, const VkWin32SurfaceCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCreateWin32SurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCreateWin32SurfaceKHR(VkInstance instance, const VkWin32SurfaceCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCreateWin32SurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
}

void CoreChecksOptickInstrumented::PostCallRecordCreateWin32SurfaceKHR(VkInstance instance, const VkWin32SurfaceCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCreateWin32SurfaceKHR(instance, pCreateInfo, pAllocator, pSurface, result);
}

#endif // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
bool CoreChecksOptickInstrumented::PreCallValidateGetPhysicalDeviceWin32PresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetPhysicalDeviceWin32PresentationSupportKHR(physicalDevice, queueFamilyIndex);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetPhysicalDeviceWin32PresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetPhysicalDeviceWin32PresentationSupportKHR(physicalDevice, queueFamilyIndex);
}

void CoreChecksOptickInstrumented::PostCallRecordGetPhysicalDeviceWin32PresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetPhysicalDeviceWin32PresentationSupportKHR(physicalDevice, queueFamilyIndex);
}

#endif // VK_USE_PLATFORM_WIN32_KHR
bool CoreChecksOptickInstrumented::PreCallValidateGetPhysicalDeviceFeatures2KHR(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures2* pFeatures) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetPhysicalDeviceFeatures2KHR(physicalDevice, pFeatures);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetPhysicalDeviceFeatures2KHR(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures2* pFeatures) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetPhysicalDeviceFeatures2KHR(physicalDevice, pFeatures);
}

void CoreChecksOptickInstrumented::PostCallRecordGetPhysicalDeviceFeatures2KHR(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures2* pFeatures) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetPhysicalDeviceFeatures2KHR(physicalDevice, pFeatures);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetPhysicalDeviceProperties2KHR(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties2* pProperties) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetPhysicalDeviceProperties2KHR(physicalDevice, pProperties);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetPhysicalDeviceProperties2KHR(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties2* pProperties) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetPhysicalDeviceProperties2KHR(physicalDevice, pProperties);
}

void CoreChecksOptickInstrumented::PostCallRecordGetPhysicalDeviceProperties2KHR(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties2* pProperties) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetPhysicalDeviceProperties2KHR(physicalDevice, pProperties);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetPhysicalDeviceFormatProperties2KHR(VkPhysicalDevice physicalDevice, VkFormat format, VkFormatProperties2* pFormatProperties) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetPhysicalDeviceFormatProperties2KHR(physicalDevice, format, pFormatProperties);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetPhysicalDeviceFormatProperties2KHR(VkPhysicalDevice physicalDevice, VkFormat format, VkFormatProperties2* pFormatProperties) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetPhysicalDeviceFormatProperties2KHR(physicalDevice, format, pFormatProperties);
}

void CoreChecksOptickInstrumented::PostCallRecordGetPhysicalDeviceFormatProperties2KHR(VkPhysicalDevice physicalDevice, VkFormat format, VkFormatProperties2* pFormatProperties) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetPhysicalDeviceFormatProperties2KHR(physicalDevice, format, pFormatProperties);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetPhysicalDeviceImageFormatProperties2KHR(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceImageFormatInfo2* pImageFormatInfo, VkImageFormatProperties2* pImageFormatProperties) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetPhysicalDeviceImageFormatProperties2KHR(physicalDevice, pImageFormatInfo, pImageFormatProperties);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetPhysicalDeviceImageFormatProperties2KHR(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceImageFormatInfo2* pImageFormatInfo, VkImageFormatProperties2* pImageFormatProperties) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetPhysicalDeviceImageFormatProperties2KHR(physicalDevice, pImageFormatInfo, pImageFormatProperties);
}

void CoreChecksOptickInstrumented::PostCallRecordGetPhysicalDeviceImageFormatProperties2KHR(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceImageFormatInfo2* pImageFormatInfo, VkImageFormatProperties2* pImageFormatProperties, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetPhysicalDeviceImageFormatProperties2KHR(physicalDevice, pImageFormatInfo, pImageFormatProperties, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetPhysicalDeviceQueueFamilyProperties2KHR(VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount, VkQueueFamilyProperties2* pQueueFamilyProperties) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetPhysicalDeviceQueueFamilyProperties2KHR(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetPhysicalDeviceQueueFamilyProperties2KHR(VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount, VkQueueFamilyProperties2* pQueueFamilyProperties) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetPhysicalDeviceQueueFamilyProperties2KHR(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
}

void CoreChecksOptickInstrumented::PostCallRecordGetPhysicalDeviceQueueFamilyProperties2KHR(VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount, VkQueueFamilyProperties2* pQueueFamilyProperties) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetPhysicalDeviceQueueFamilyProperties2KHR(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetPhysicalDeviceMemoryProperties2KHR(VkPhysicalDevice physicalDevice, VkPhysicalDeviceMemoryProperties2* pMemoryProperties) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetPhysicalDeviceMemoryProperties2KHR(physicalDevice, pMemoryProperties);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetPhysicalDeviceMemoryProperties2KHR(VkPhysicalDevice physicalDevice, VkPhysicalDeviceMemoryProperties2* pMemoryProperties) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetPhysicalDeviceMemoryProperties2KHR(physicalDevice, pMemoryProperties);
}

void CoreChecksOptickInstrumented::PostCallRecordGetPhysicalDeviceMemoryProperties2KHR(VkPhysicalDevice physicalDevice, VkPhysicalDeviceMemoryProperties2* pMemoryProperties) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetPhysicalDeviceMemoryProperties2KHR(physicalDevice, pMemoryProperties);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetPhysicalDeviceSparseImageFormatProperties2KHR(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSparseImageFormatInfo2* pFormatInfo, uint32_t* pPropertyCount, VkSparseImageFormatProperties2* pProperties) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetPhysicalDeviceSparseImageFormatProperties2KHR(physicalDevice, pFormatInfo, pPropertyCount, pProperties);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetPhysicalDeviceSparseImageFormatProperties2KHR(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSparseImageFormatInfo2* pFormatInfo, uint32_t* pPropertyCount, VkSparseImageFormatProperties2* pProperties) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetPhysicalDeviceSparseImageFormatProperties2KHR(physicalDevice, pFormatInfo, pPropertyCount, pProperties);
}

void CoreChecksOptickInstrumented::PostCallRecordGetPhysicalDeviceSparseImageFormatProperties2KHR(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSparseImageFormatInfo2* pFormatInfo, uint32_t* pPropertyCount, VkSparseImageFormatProperties2* pProperties) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetPhysicalDeviceSparseImageFormatProperties2KHR(physicalDevice, pFormatInfo, pPropertyCount, pProperties);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetDeviceGroupPeerMemoryFeaturesKHR(VkDevice device, uint32_t heapIndex, uint32_t localDeviceIndex, uint32_t remoteDeviceIndex, VkPeerMemoryFeatureFlags* pPeerMemoryFeatures) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetDeviceGroupPeerMemoryFeaturesKHR(device, heapIndex, localDeviceIndex, remoteDeviceIndex, pPeerMemoryFeatures);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetDeviceGroupPeerMemoryFeaturesKHR(VkDevice device, uint32_t heapIndex, uint32_t localDeviceIndex, uint32_t remoteDeviceIndex, VkPeerMemoryFeatureFlags* pPeerMemoryFeatures) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetDeviceGroupPeerMemoryFeaturesKHR(device, heapIndex, localDeviceIndex, remoteDeviceIndex, pPeerMemoryFeatures);
}

void CoreChecksOptickInstrumented::PostCallRecordGetDeviceGroupPeerMemoryFeaturesKHR(VkDevice device, uint32_t heapIndex, uint32_t localDeviceIndex, uint32_t remoteDeviceIndex, VkPeerMemoryFeatureFlags* pPeerMemoryFeatures) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetDeviceGroupPeerMemoryFeaturesKHR(device, heapIndex, localDeviceIndex, remoteDeviceIndex, pPeerMemoryFeatures);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdSetDeviceMaskKHR(VkCommandBuffer commandBuffer, uint32_t deviceMask) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdSetDeviceMaskKHR(commandBuffer, deviceMask);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdSetDeviceMaskKHR(VkCommandBuffer commandBuffer, uint32_t deviceMask) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdSetDeviceMaskKHR(commandBuffer, deviceMask);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdSetDeviceMaskKHR(VkCommandBuffer commandBuffer, uint32_t deviceMask) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdSetDeviceMaskKHR(commandBuffer, deviceMask);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdDispatchBaseKHR(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY, uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdDispatchBaseKHR(commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdDispatchBaseKHR(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY, uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdDispatchBaseKHR(commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdDispatchBaseKHR(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY, uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdDispatchBaseKHR(commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ);
}

bool CoreChecksOptickInstrumented::PreCallValidateTrimCommandPoolKHR(VkDevice device, VkCommandPool commandPool, VkCommandPoolTrimFlags flags) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateTrimCommandPoolKHR(device, commandPool, flags);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordTrimCommandPoolKHR(VkDevice device, VkCommandPool commandPool, VkCommandPoolTrimFlags flags) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordTrimCommandPoolKHR(device, commandPool, flags);
}

void CoreChecksOptickInstrumented::PostCallRecordTrimCommandPoolKHR(VkDevice device, VkCommandPool commandPool, VkCommandPoolTrimFlags flags) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordTrimCommandPoolKHR(device, commandPool, flags);
}

bool CoreChecksOptickInstrumented::PreCallValidateEnumeratePhysicalDeviceGroupsKHR(VkInstance instance, uint32_t* pPhysicalDeviceGroupCount, VkPhysicalDeviceGroupProperties* pPhysicalDeviceGroupProperties) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateEnumeratePhysicalDeviceGroupsKHR(instance, pPhysicalDeviceGroupCount, pPhysicalDeviceGroupProperties);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordEnumeratePhysicalDeviceGroupsKHR(VkInstance instance, uint32_t* pPhysicalDeviceGroupCount, VkPhysicalDeviceGroupProperties* pPhysicalDeviceGroupProperties) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordEnumeratePhysicalDeviceGroupsKHR(instance, pPhysicalDeviceGroupCount, pPhysicalDeviceGroupProperties);
}

void CoreChecksOptickInstrumented::PostCallRecordEnumeratePhysicalDeviceGroupsKHR(VkInstance instance, uint32_t* pPhysicalDeviceGroupCount, VkPhysicalDeviceGroupProperties* pPhysicalDeviceGroupProperties, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordEnumeratePhysicalDeviceGroupsKHR(instance, pPhysicalDeviceGroupCount, pPhysicalDeviceGroupProperties, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetPhysicalDeviceExternalBufferPropertiesKHR(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalBufferInfo* pExternalBufferInfo, VkExternalBufferProperties* pExternalBufferProperties) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetPhysicalDeviceExternalBufferPropertiesKHR(physicalDevice, pExternalBufferInfo, pExternalBufferProperties);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetPhysicalDeviceExternalBufferPropertiesKHR(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalBufferInfo* pExternalBufferInfo, VkExternalBufferProperties* pExternalBufferProperties) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetPhysicalDeviceExternalBufferPropertiesKHR(physicalDevice, pExternalBufferInfo, pExternalBufferProperties);
}

void CoreChecksOptickInstrumented::PostCallRecordGetPhysicalDeviceExternalBufferPropertiesKHR(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalBufferInfo* pExternalBufferInfo, VkExternalBufferProperties* pExternalBufferProperties) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetPhysicalDeviceExternalBufferPropertiesKHR(physicalDevice, pExternalBufferInfo, pExternalBufferProperties);
}

#ifdef VK_USE_PLATFORM_WIN32_KHR
bool CoreChecksOptickInstrumented::PreCallValidateGetMemoryWin32HandleKHR(VkDevice device, const VkMemoryGetWin32HandleInfoKHR* pGetWin32HandleInfo, HANDLE* pHandle) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetMemoryWin32HandleKHR(device, pGetWin32HandleInfo, pHandle);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetMemoryWin32HandleKHR(VkDevice device, const VkMemoryGetWin32HandleInfoKHR* pGetWin32HandleInfo, HANDLE* pHandle) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetMemoryWin32HandleKHR(device, pGetWin32HandleInfo, pHandle);
}

void CoreChecksOptickInstrumented::PostCallRecordGetMemoryWin32HandleKHR(VkDevice device, const VkMemoryGetWin32HandleInfoKHR* pGetWin32HandleInfo, HANDLE* pHandle, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetMemoryWin32HandleKHR(device, pGetWin32HandleInfo, pHandle, result);
}

#endif // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
bool CoreChecksOptickInstrumented::PreCallValidateGetMemoryWin32HandlePropertiesKHR(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType, HANDLE handle, VkMemoryWin32HandlePropertiesKHR* pMemoryWin32HandleProperties) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetMemoryWin32HandlePropertiesKHR(device, handleType, handle, pMemoryWin32HandleProperties);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetMemoryWin32HandlePropertiesKHR(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType, HANDLE handle, VkMemoryWin32HandlePropertiesKHR* pMemoryWin32HandleProperties) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetMemoryWin32HandlePropertiesKHR(device, handleType, handle, pMemoryWin32HandleProperties);
}

void CoreChecksOptickInstrumented::PostCallRecordGetMemoryWin32HandlePropertiesKHR(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType, HANDLE handle, VkMemoryWin32HandlePropertiesKHR* pMemoryWin32HandleProperties, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetMemoryWin32HandlePropertiesKHR(device, handleType, handle, pMemoryWin32HandleProperties, result);
}

#endif // VK_USE_PLATFORM_WIN32_KHR
bool CoreChecksOptickInstrumented::PreCallValidateGetMemoryFdKHR(VkDevice device, const VkMemoryGetFdInfoKHR* pGetFdInfo, int* pFd) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetMemoryFdKHR(device, pGetFdInfo, pFd);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetMemoryFdKHR(VkDevice device, const VkMemoryGetFdInfoKHR* pGetFdInfo, int* pFd) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetMemoryFdKHR(device, pGetFdInfo, pFd);
}

void CoreChecksOptickInstrumented::PostCallRecordGetMemoryFdKHR(VkDevice device, const VkMemoryGetFdInfoKHR* pGetFdInfo, int* pFd, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetMemoryFdKHR(device, pGetFdInfo, pFd, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetMemoryFdPropertiesKHR(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType, int fd, VkMemoryFdPropertiesKHR* pMemoryFdProperties) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetMemoryFdPropertiesKHR(device, handleType, fd, pMemoryFdProperties);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetMemoryFdPropertiesKHR(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType, int fd, VkMemoryFdPropertiesKHR* pMemoryFdProperties) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetMemoryFdPropertiesKHR(device, handleType, fd, pMemoryFdProperties);
}

void CoreChecksOptickInstrumented::PostCallRecordGetMemoryFdPropertiesKHR(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType, int fd, VkMemoryFdPropertiesKHR* pMemoryFdProperties, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetMemoryFdPropertiesKHR(device, handleType, fd, pMemoryFdProperties, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetPhysicalDeviceExternalSemaphorePropertiesKHR(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalSemaphoreInfo* pExternalSemaphoreInfo, VkExternalSemaphoreProperties* pExternalSemaphoreProperties) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetPhysicalDeviceExternalSemaphorePropertiesKHR(physicalDevice, pExternalSemaphoreInfo, pExternalSemaphoreProperties);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetPhysicalDeviceExternalSemaphorePropertiesKHR(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalSemaphoreInfo* pExternalSemaphoreInfo, VkExternalSemaphoreProperties* pExternalSemaphoreProperties) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetPhysicalDeviceExternalSemaphorePropertiesKHR(physicalDevice, pExternalSemaphoreInfo, pExternalSemaphoreProperties);
}

void CoreChecksOptickInstrumented::PostCallRecordGetPhysicalDeviceExternalSemaphorePropertiesKHR(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalSemaphoreInfo* pExternalSemaphoreInfo, VkExternalSemaphoreProperties* pExternalSemaphoreProperties) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetPhysicalDeviceExternalSemaphorePropertiesKHR(physicalDevice, pExternalSemaphoreInfo, pExternalSemaphoreProperties);
}

#ifdef VK_USE_PLATFORM_WIN32_KHR
bool CoreChecksOptickInstrumented::PreCallValidateImportSemaphoreWin32HandleKHR(VkDevice device, const VkImportSemaphoreWin32HandleInfoKHR* pImportSemaphoreWin32HandleInfo) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateImportSemaphoreWin32HandleKHR(device, pImportSemaphoreWin32HandleInfo);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordImportSemaphoreWin32HandleKHR(VkDevice device, const VkImportSemaphoreWin32HandleInfoKHR* pImportSemaphoreWin32HandleInfo) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordImportSemaphoreWin32HandleKHR(device, pImportSemaphoreWin32HandleInfo);
}

void CoreChecksOptickInstrumented::PostCallRecordImportSemaphoreWin32HandleKHR(VkDevice device, const VkImportSemaphoreWin32HandleInfoKHR* pImportSemaphoreWin32HandleInfo, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordImportSemaphoreWin32HandleKHR(device, pImportSemaphoreWin32HandleInfo, result);
}

#endif // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
bool CoreChecksOptickInstrumented::PreCallValidateGetSemaphoreWin32HandleKHR(VkDevice device, const VkSemaphoreGetWin32HandleInfoKHR* pGetWin32HandleInfo, HANDLE* pHandle) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetSemaphoreWin32HandleKHR(device, pGetWin32HandleInfo, pHandle);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetSemaphoreWin32HandleKHR(VkDevice device, const VkSemaphoreGetWin32HandleInfoKHR* pGetWin32HandleInfo, HANDLE* pHandle) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetSemaphoreWin32HandleKHR(device, pGetWin32HandleInfo, pHandle);
}

void CoreChecksOptickInstrumented::PostCallRecordGetSemaphoreWin32HandleKHR(VkDevice device, const VkSemaphoreGetWin32HandleInfoKHR* pGetWin32HandleInfo, HANDLE* pHandle, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetSemaphoreWin32HandleKHR(device, pGetWin32HandleInfo, pHandle, result);
}

#endif // VK_USE_PLATFORM_WIN32_KHR
bool CoreChecksOptickInstrumented::PreCallValidateImportSemaphoreFdKHR(VkDevice device, const VkImportSemaphoreFdInfoKHR* pImportSemaphoreFdInfo) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateImportSemaphoreFdKHR(device, pImportSemaphoreFdInfo);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordImportSemaphoreFdKHR(VkDevice device, const VkImportSemaphoreFdInfoKHR* pImportSemaphoreFdInfo) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordImportSemaphoreFdKHR(device, pImportSemaphoreFdInfo);
}

void CoreChecksOptickInstrumented::PostCallRecordImportSemaphoreFdKHR(VkDevice device, const VkImportSemaphoreFdInfoKHR* pImportSemaphoreFdInfo, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordImportSemaphoreFdKHR(device, pImportSemaphoreFdInfo, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetSemaphoreFdKHR(VkDevice device, const VkSemaphoreGetFdInfoKHR* pGetFdInfo, int* pFd) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetSemaphoreFdKHR(device, pGetFdInfo, pFd);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetSemaphoreFdKHR(VkDevice device, const VkSemaphoreGetFdInfoKHR* pGetFdInfo, int* pFd) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetSemaphoreFdKHR(device, pGetFdInfo, pFd);
}

void CoreChecksOptickInstrumented::PostCallRecordGetSemaphoreFdKHR(VkDevice device, const VkSemaphoreGetFdInfoKHR* pGetFdInfo, int* pFd, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetSemaphoreFdKHR(device, pGetFdInfo, pFd, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdPushDescriptorSetKHR(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout, uint32_t set, uint32_t descriptorWriteCount, const VkWriteDescriptorSet* pDescriptorWrites) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdPushDescriptorSetKHR(commandBuffer, pipelineBindPoint, layout, set, descriptorWriteCount, pDescriptorWrites);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdPushDescriptorSetKHR(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout, uint32_t set, uint32_t descriptorWriteCount, const VkWriteDescriptorSet* pDescriptorWrites) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdPushDescriptorSetKHR(commandBuffer, pipelineBindPoint, layout, set, descriptorWriteCount, pDescriptorWrites);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdPushDescriptorSetKHR(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout, uint32_t set, uint32_t descriptorWriteCount, const VkWriteDescriptorSet* pDescriptorWrites) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdPushDescriptorSetKHR(commandBuffer, pipelineBindPoint, layout, set, descriptorWriteCount, pDescriptorWrites);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdPushDescriptorSetWithTemplateKHR(VkCommandBuffer commandBuffer, VkDescriptorUpdateTemplate descriptorUpdateTemplate, VkPipelineLayout layout, uint32_t set, const void* pData) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdPushDescriptorSetWithTemplateKHR(commandBuffer, descriptorUpdateTemplate, layout, set, pData);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdPushDescriptorSetWithTemplateKHR(VkCommandBuffer commandBuffer, VkDescriptorUpdateTemplate descriptorUpdateTemplate, VkPipelineLayout layout, uint32_t set, const void* pData) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdPushDescriptorSetWithTemplateKHR(commandBuffer, descriptorUpdateTemplate, layout, set, pData);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdPushDescriptorSetWithTemplateKHR(VkCommandBuffer commandBuffer, VkDescriptorUpdateTemplate descriptorUpdateTemplate, VkPipelineLayout layout, uint32_t set, const void* pData) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdPushDescriptorSetWithTemplateKHR(commandBuffer, descriptorUpdateTemplate, layout, set, pData);
}

bool CoreChecksOptickInstrumented::PreCallValidateCreateDescriptorUpdateTemplateKHR(VkDevice device, const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDescriptorUpdateTemplate* pDescriptorUpdateTemplate) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCreateDescriptorUpdateTemplateKHR(device, pCreateInfo, pAllocator, pDescriptorUpdateTemplate);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCreateDescriptorUpdateTemplateKHR(VkDevice device, const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDescriptorUpdateTemplate* pDescriptorUpdateTemplate) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCreateDescriptorUpdateTemplateKHR(device, pCreateInfo, pAllocator, pDescriptorUpdateTemplate);
}

void CoreChecksOptickInstrumented::PostCallRecordCreateDescriptorUpdateTemplateKHR(VkDevice device, const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDescriptorUpdateTemplate* pDescriptorUpdateTemplate, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCreateDescriptorUpdateTemplateKHR(device, pCreateInfo, pAllocator, pDescriptorUpdateTemplate, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateDestroyDescriptorUpdateTemplateKHR(VkDevice device, VkDescriptorUpdateTemplate descriptorUpdateTemplate, const VkAllocationCallbacks* pAllocator) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateDestroyDescriptorUpdateTemplateKHR(device, descriptorUpdateTemplate, pAllocator);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordDestroyDescriptorUpdateTemplateKHR(VkDevice device, VkDescriptorUpdateTemplate descriptorUpdateTemplate, const VkAllocationCallbacks* pAllocator) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordDestroyDescriptorUpdateTemplateKHR(device, descriptorUpdateTemplate, pAllocator);
}

void CoreChecksOptickInstrumented::PostCallRecordDestroyDescriptorUpdateTemplateKHR(VkDevice device, VkDescriptorUpdateTemplate descriptorUpdateTemplate, const VkAllocationCallbacks* pAllocator) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordDestroyDescriptorUpdateTemplateKHR(device, descriptorUpdateTemplate, pAllocator);
}

bool CoreChecksOptickInstrumented::PreCallValidateUpdateDescriptorSetWithTemplateKHR(VkDevice device, VkDescriptorSet descriptorSet, VkDescriptorUpdateTemplate descriptorUpdateTemplate, const void* pData) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateUpdateDescriptorSetWithTemplateKHR(device, descriptorSet, descriptorUpdateTemplate, pData);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordUpdateDescriptorSetWithTemplateKHR(VkDevice device, VkDescriptorSet descriptorSet, VkDescriptorUpdateTemplate descriptorUpdateTemplate, const void* pData) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordUpdateDescriptorSetWithTemplateKHR(device, descriptorSet, descriptorUpdateTemplate, pData);
}

void CoreChecksOptickInstrumented::PostCallRecordUpdateDescriptorSetWithTemplateKHR(VkDevice device, VkDescriptorSet descriptorSet, VkDescriptorUpdateTemplate descriptorUpdateTemplate, const void* pData) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordUpdateDescriptorSetWithTemplateKHR(device, descriptorSet, descriptorUpdateTemplate, pData);
}

bool CoreChecksOptickInstrumented::PreCallValidateCreateRenderPass2KHR(VkDevice device, const VkRenderPassCreateInfo2* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCreateRenderPass2KHR(device, pCreateInfo, pAllocator, pRenderPass);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCreateRenderPass2KHR(VkDevice device, const VkRenderPassCreateInfo2* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCreateRenderPass2KHR(device, pCreateInfo, pAllocator, pRenderPass);
}

void CoreChecksOptickInstrumented::PostCallRecordCreateRenderPass2KHR(VkDevice device, const VkRenderPassCreateInfo2* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCreateRenderPass2KHR(device, pCreateInfo, pAllocator, pRenderPass, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdBeginRenderPass2KHR(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo*      pRenderPassBegin, const VkSubpassBeginInfo*      pSubpassBeginInfo) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdBeginRenderPass2KHR(commandBuffer, pRenderPassBegin, pSubpassBeginInfo);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdBeginRenderPass2KHR(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo*      pRenderPassBegin, const VkSubpassBeginInfo*      pSubpassBeginInfo) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdBeginRenderPass2KHR(commandBuffer, pRenderPassBegin, pSubpassBeginInfo);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdBeginRenderPass2KHR(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo*      pRenderPassBegin, const VkSubpassBeginInfo*      pSubpassBeginInfo) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdBeginRenderPass2KHR(commandBuffer, pRenderPassBegin, pSubpassBeginInfo);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdNextSubpass2KHR(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo*      pSubpassBeginInfo, const VkSubpassEndInfo*        pSubpassEndInfo) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdNextSubpass2KHR(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdNextSubpass2KHR(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo*      pSubpassBeginInfo, const VkSubpassEndInfo*        pSubpassEndInfo) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdNextSubpass2KHR(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdNextSubpass2KHR(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo*      pSubpassBeginInfo, const VkSubpassEndInfo*        pSubpassEndInfo) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdNextSubpass2KHR(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdEndRenderPass2KHR(VkCommandBuffer commandBuffer, const VkSubpassEndInfo*        pSubpassEndInfo) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdEndRenderPass2KHR(commandBuffer, pSubpassEndInfo);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdEndRenderPass2KHR(VkCommandBuffer commandBuffer, const VkSubpassEndInfo*        pSubpassEndInfo) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdEndRenderPass2KHR(commandBuffer, pSubpassEndInfo);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdEndRenderPass2KHR(VkCommandBuffer commandBuffer, const VkSubpassEndInfo*        pSubpassEndInfo) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdEndRenderPass2KHR(commandBuffer, pSubpassEndInfo);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetSwapchainStatusKHR(VkDevice device, VkSwapchainKHR swapchain) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetSwapchainStatusKHR(device, swapchain);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetSwapchainStatusKHR(VkDevice device, VkSwapchainKHR swapchain) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetSwapchainStatusKHR(device, swapchain);
}

void CoreChecksOptickInstrumented::PostCallRecordGetSwapchainStatusKHR(VkDevice device, VkSwapchainKHR swapchain, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetSwapchainStatusKHR(device, swapchain, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetPhysicalDeviceExternalFencePropertiesKHR(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalFenceInfo* pExternalFenceInfo, VkExternalFenceProperties* pExternalFenceProperties) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetPhysicalDeviceExternalFencePropertiesKHR(physicalDevice, pExternalFenceInfo, pExternalFenceProperties);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetPhysicalDeviceExternalFencePropertiesKHR(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalFenceInfo* pExternalFenceInfo, VkExternalFenceProperties* pExternalFenceProperties) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetPhysicalDeviceExternalFencePropertiesKHR(physicalDevice, pExternalFenceInfo, pExternalFenceProperties);
}

void CoreChecksOptickInstrumented::PostCallRecordGetPhysicalDeviceExternalFencePropertiesKHR(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalFenceInfo* pExternalFenceInfo, VkExternalFenceProperties* pExternalFenceProperties) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetPhysicalDeviceExternalFencePropertiesKHR(physicalDevice, pExternalFenceInfo, pExternalFenceProperties);
}

#ifdef VK_USE_PLATFORM_WIN32_KHR
bool CoreChecksOptickInstrumented::PreCallValidateImportFenceWin32HandleKHR(VkDevice device, const VkImportFenceWin32HandleInfoKHR* pImportFenceWin32HandleInfo) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateImportFenceWin32HandleKHR(device, pImportFenceWin32HandleInfo);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordImportFenceWin32HandleKHR(VkDevice device, const VkImportFenceWin32HandleInfoKHR* pImportFenceWin32HandleInfo) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordImportFenceWin32HandleKHR(device, pImportFenceWin32HandleInfo);
}

void CoreChecksOptickInstrumented::PostCallRecordImportFenceWin32HandleKHR(VkDevice device, const VkImportFenceWin32HandleInfoKHR* pImportFenceWin32HandleInfo, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordImportFenceWin32HandleKHR(device, pImportFenceWin32HandleInfo, result);
}

#endif // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
bool CoreChecksOptickInstrumented::PreCallValidateGetFenceWin32HandleKHR(VkDevice device, const VkFenceGetWin32HandleInfoKHR* pGetWin32HandleInfo, HANDLE* pHandle) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetFenceWin32HandleKHR(device, pGetWin32HandleInfo, pHandle);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetFenceWin32HandleKHR(VkDevice device, const VkFenceGetWin32HandleInfoKHR* pGetWin32HandleInfo, HANDLE* pHandle) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetFenceWin32HandleKHR(device, pGetWin32HandleInfo, pHandle);
}

void CoreChecksOptickInstrumented::PostCallRecordGetFenceWin32HandleKHR(VkDevice device, const VkFenceGetWin32HandleInfoKHR* pGetWin32HandleInfo, HANDLE* pHandle, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetFenceWin32HandleKHR(device, pGetWin32HandleInfo, pHandle, result);
}

#endif // VK_USE_PLATFORM_WIN32_KHR
bool CoreChecksOptickInstrumented::PreCallValidateImportFenceFdKHR(VkDevice device, const VkImportFenceFdInfoKHR* pImportFenceFdInfo) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateImportFenceFdKHR(device, pImportFenceFdInfo);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordImportFenceFdKHR(VkDevice device, const VkImportFenceFdInfoKHR* pImportFenceFdInfo) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordImportFenceFdKHR(device, pImportFenceFdInfo);
}

void CoreChecksOptickInstrumented::PostCallRecordImportFenceFdKHR(VkDevice device, const VkImportFenceFdInfoKHR* pImportFenceFdInfo, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordImportFenceFdKHR(device, pImportFenceFdInfo, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetFenceFdKHR(VkDevice device, const VkFenceGetFdInfoKHR* pGetFdInfo, int* pFd) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetFenceFdKHR(device, pGetFdInfo, pFd);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetFenceFdKHR(VkDevice device, const VkFenceGetFdInfoKHR* pGetFdInfo, int* pFd) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetFenceFdKHR(device, pGetFdInfo, pFd);
}

void CoreChecksOptickInstrumented::PostCallRecordGetFenceFdKHR(VkDevice device, const VkFenceGetFdInfoKHR* pGetFdInfo, int* pFd, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetFenceFdKHR(device, pGetFdInfo, pFd, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, uint32_t* pCounterCount, VkPerformanceCounterKHR* pCounters, VkPerformanceCounterDescriptionKHR* pCounterDescriptions) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(physicalDevice, queueFamilyIndex, pCounterCount, pCounters, pCounterDescriptions);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, uint32_t* pCounterCount, VkPerformanceCounterKHR* pCounters, VkPerformanceCounterDescriptionKHR* pCounterDescriptions) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(physicalDevice, queueFamilyIndex, pCounterCount, pCounters, pCounterDescriptions);
}

void CoreChecksOptickInstrumented::PostCallRecordEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, uint32_t* pCounterCount, VkPerformanceCounterKHR* pCounters, VkPerformanceCounterDescriptionKHR* pCounterDescriptions, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(physicalDevice, queueFamilyIndex, pCounterCount, pCounters, pCounterDescriptions, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR(VkPhysicalDevice physicalDevice, const VkQueryPoolPerformanceCreateInfoKHR* pPerformanceQueryCreateInfo, uint32_t* pNumPasses) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR(physicalDevice, pPerformanceQueryCreateInfo, pNumPasses);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR(VkPhysicalDevice physicalDevice, const VkQueryPoolPerformanceCreateInfoKHR* pPerformanceQueryCreateInfo, uint32_t* pNumPasses) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR(physicalDevice, pPerformanceQueryCreateInfo, pNumPasses);
}

void CoreChecksOptickInstrumented::PostCallRecordGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR(VkPhysicalDevice physicalDevice, const VkQueryPoolPerformanceCreateInfoKHR* pPerformanceQueryCreateInfo, uint32_t* pNumPasses) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR(physicalDevice, pPerformanceQueryCreateInfo, pNumPasses);
}

bool CoreChecksOptickInstrumented::PreCallValidateAcquireProfilingLockKHR(VkDevice device, const VkAcquireProfilingLockInfoKHR* pInfo) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateAcquireProfilingLockKHR(device, pInfo);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordAcquireProfilingLockKHR(VkDevice device, const VkAcquireProfilingLockInfoKHR* pInfo) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordAcquireProfilingLockKHR(device, pInfo);
}

void CoreChecksOptickInstrumented::PostCallRecordAcquireProfilingLockKHR(VkDevice device, const VkAcquireProfilingLockInfoKHR* pInfo, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordAcquireProfilingLockKHR(device, pInfo, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateReleaseProfilingLockKHR(VkDevice device) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateReleaseProfilingLockKHR(device);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordReleaseProfilingLockKHR(VkDevice device) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordReleaseProfilingLockKHR(device);
}

void CoreChecksOptickInstrumented::PostCallRecordReleaseProfilingLockKHR(VkDevice device) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordReleaseProfilingLockKHR(device);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetPhysicalDeviceSurfaceCapabilities2KHR(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo, VkSurfaceCapabilities2KHR* pSurfaceCapabilities) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetPhysicalDeviceSurfaceCapabilities2KHR(physicalDevice, pSurfaceInfo, pSurfaceCapabilities);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetPhysicalDeviceSurfaceCapabilities2KHR(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo, VkSurfaceCapabilities2KHR* pSurfaceCapabilities) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetPhysicalDeviceSurfaceCapabilities2KHR(physicalDevice, pSurfaceInfo, pSurfaceCapabilities);
}

void CoreChecksOptickInstrumented::PostCallRecordGetPhysicalDeviceSurfaceCapabilities2KHR(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo, VkSurfaceCapabilities2KHR* pSurfaceCapabilities, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetPhysicalDeviceSurfaceCapabilities2KHR(physicalDevice, pSurfaceInfo, pSurfaceCapabilities, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetPhysicalDeviceSurfaceFormats2KHR(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo, uint32_t* pSurfaceFormatCount, VkSurfaceFormat2KHR* pSurfaceFormats) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetPhysicalDeviceSurfaceFormats2KHR(physicalDevice, pSurfaceInfo, pSurfaceFormatCount, pSurfaceFormats);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetPhysicalDeviceSurfaceFormats2KHR(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo, uint32_t* pSurfaceFormatCount, VkSurfaceFormat2KHR* pSurfaceFormats) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetPhysicalDeviceSurfaceFormats2KHR(physicalDevice, pSurfaceInfo, pSurfaceFormatCount, pSurfaceFormats);
}

void CoreChecksOptickInstrumented::PostCallRecordGetPhysicalDeviceSurfaceFormats2KHR(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo, uint32_t* pSurfaceFormatCount, VkSurfaceFormat2KHR* pSurfaceFormats, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetPhysicalDeviceSurfaceFormats2KHR(physicalDevice, pSurfaceInfo, pSurfaceFormatCount, pSurfaceFormats, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetPhysicalDeviceDisplayProperties2KHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount, VkDisplayProperties2KHR* pProperties) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetPhysicalDeviceDisplayProperties2KHR(physicalDevice, pPropertyCount, pProperties);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetPhysicalDeviceDisplayProperties2KHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount, VkDisplayProperties2KHR* pProperties) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetPhysicalDeviceDisplayProperties2KHR(physicalDevice, pPropertyCount, pProperties);
}

void CoreChecksOptickInstrumented::PostCallRecordGetPhysicalDeviceDisplayProperties2KHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount, VkDisplayProperties2KHR* pProperties, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetPhysicalDeviceDisplayProperties2KHR(physicalDevice, pPropertyCount, pProperties, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetPhysicalDeviceDisplayPlaneProperties2KHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount, VkDisplayPlaneProperties2KHR* pProperties) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetPhysicalDeviceDisplayPlaneProperties2KHR(physicalDevice, pPropertyCount, pProperties);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetPhysicalDeviceDisplayPlaneProperties2KHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount, VkDisplayPlaneProperties2KHR* pProperties) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetPhysicalDeviceDisplayPlaneProperties2KHR(physicalDevice, pPropertyCount, pProperties);
}

void CoreChecksOptickInstrumented::PostCallRecordGetPhysicalDeviceDisplayPlaneProperties2KHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount, VkDisplayPlaneProperties2KHR* pProperties, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetPhysicalDeviceDisplayPlaneProperties2KHR(physicalDevice, pPropertyCount, pProperties, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetDisplayModeProperties2KHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display, uint32_t* pPropertyCount, VkDisplayModeProperties2KHR* pProperties) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetDisplayModeProperties2KHR(physicalDevice, display, pPropertyCount, pProperties);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetDisplayModeProperties2KHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display, uint32_t* pPropertyCount, VkDisplayModeProperties2KHR* pProperties) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetDisplayModeProperties2KHR(physicalDevice, display, pPropertyCount, pProperties);
}

void CoreChecksOptickInstrumented::PostCallRecordGetDisplayModeProperties2KHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display, uint32_t* pPropertyCount, VkDisplayModeProperties2KHR* pProperties, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetDisplayModeProperties2KHR(physicalDevice, display, pPropertyCount, pProperties, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetDisplayPlaneCapabilities2KHR(VkPhysicalDevice physicalDevice, const VkDisplayPlaneInfo2KHR* pDisplayPlaneInfo, VkDisplayPlaneCapabilities2KHR* pCapabilities) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetDisplayPlaneCapabilities2KHR(physicalDevice, pDisplayPlaneInfo, pCapabilities);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetDisplayPlaneCapabilities2KHR(VkPhysicalDevice physicalDevice, const VkDisplayPlaneInfo2KHR* pDisplayPlaneInfo, VkDisplayPlaneCapabilities2KHR* pCapabilities) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetDisplayPlaneCapabilities2KHR(physicalDevice, pDisplayPlaneInfo, pCapabilities);
}

void CoreChecksOptickInstrumented::PostCallRecordGetDisplayPlaneCapabilities2KHR(VkPhysicalDevice physicalDevice, const VkDisplayPlaneInfo2KHR* pDisplayPlaneInfo, VkDisplayPlaneCapabilities2KHR* pCapabilities, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetDisplayPlaneCapabilities2KHR(physicalDevice, pDisplayPlaneInfo, pCapabilities, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetImageMemoryRequirements2KHR(VkDevice device, const VkImageMemoryRequirementsInfo2* pInfo, VkMemoryRequirements2* pMemoryRequirements) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetImageMemoryRequirements2KHR(device, pInfo, pMemoryRequirements);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetImageMemoryRequirements2KHR(VkDevice device, const VkImageMemoryRequirementsInfo2* pInfo, VkMemoryRequirements2* pMemoryRequirements) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetImageMemoryRequirements2KHR(device, pInfo, pMemoryRequirements);
}

void CoreChecksOptickInstrumented::PostCallRecordGetImageMemoryRequirements2KHR(VkDevice device, const VkImageMemoryRequirementsInfo2* pInfo, VkMemoryRequirements2* pMemoryRequirements) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetImageMemoryRequirements2KHR(device, pInfo, pMemoryRequirements);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetBufferMemoryRequirements2KHR(VkDevice device, const VkBufferMemoryRequirementsInfo2* pInfo, VkMemoryRequirements2* pMemoryRequirements) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetBufferMemoryRequirements2KHR(device, pInfo, pMemoryRequirements);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetBufferMemoryRequirements2KHR(VkDevice device, const VkBufferMemoryRequirementsInfo2* pInfo, VkMemoryRequirements2* pMemoryRequirements) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetBufferMemoryRequirements2KHR(device, pInfo, pMemoryRequirements);
}

void CoreChecksOptickInstrumented::PostCallRecordGetBufferMemoryRequirements2KHR(VkDevice device, const VkBufferMemoryRequirementsInfo2* pInfo, VkMemoryRequirements2* pMemoryRequirements) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetBufferMemoryRequirements2KHR(device, pInfo, pMemoryRequirements);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetImageSparseMemoryRequirements2KHR(VkDevice device, const VkImageSparseMemoryRequirementsInfo2* pInfo, uint32_t* pSparseMemoryRequirementCount, VkSparseImageMemoryRequirements2* pSparseMemoryRequirements) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetImageSparseMemoryRequirements2KHR(device, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetImageSparseMemoryRequirements2KHR(VkDevice device, const VkImageSparseMemoryRequirementsInfo2* pInfo, uint32_t* pSparseMemoryRequirementCount, VkSparseImageMemoryRequirements2* pSparseMemoryRequirements) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetImageSparseMemoryRequirements2KHR(device, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
}

void CoreChecksOptickInstrumented::PostCallRecordGetImageSparseMemoryRequirements2KHR(VkDevice device, const VkImageSparseMemoryRequirementsInfo2* pInfo, uint32_t* pSparseMemoryRequirementCount, VkSparseImageMemoryRequirements2* pSparseMemoryRequirements) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetImageSparseMemoryRequirements2KHR(device, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
}

bool CoreChecksOptickInstrumented::PreCallValidateCreateSamplerYcbcrConversionKHR(VkDevice device, const VkSamplerYcbcrConversionCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSamplerYcbcrConversion* pYcbcrConversion) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCreateSamplerYcbcrConversionKHR(device, pCreateInfo, pAllocator, pYcbcrConversion);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCreateSamplerYcbcrConversionKHR(VkDevice device, const VkSamplerYcbcrConversionCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSamplerYcbcrConversion* pYcbcrConversion) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCreateSamplerYcbcrConversionKHR(device, pCreateInfo, pAllocator, pYcbcrConversion);
}

void CoreChecksOptickInstrumented::PostCallRecordCreateSamplerYcbcrConversionKHR(VkDevice device, const VkSamplerYcbcrConversionCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSamplerYcbcrConversion* pYcbcrConversion, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCreateSamplerYcbcrConversionKHR(device, pCreateInfo, pAllocator, pYcbcrConversion, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateDestroySamplerYcbcrConversionKHR(VkDevice device, VkSamplerYcbcrConversion ycbcrConversion, const VkAllocationCallbacks* pAllocator) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateDestroySamplerYcbcrConversionKHR(device, ycbcrConversion, pAllocator);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordDestroySamplerYcbcrConversionKHR(VkDevice device, VkSamplerYcbcrConversion ycbcrConversion, const VkAllocationCallbacks* pAllocator) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordDestroySamplerYcbcrConversionKHR(device, ycbcrConversion, pAllocator);
}

void CoreChecksOptickInstrumented::PostCallRecordDestroySamplerYcbcrConversionKHR(VkDevice device, VkSamplerYcbcrConversion ycbcrConversion, const VkAllocationCallbacks* pAllocator) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordDestroySamplerYcbcrConversionKHR(device, ycbcrConversion, pAllocator);
}

bool CoreChecksOptickInstrumented::PreCallValidateBindBufferMemory2KHR(VkDevice device, uint32_t bindInfoCount, const VkBindBufferMemoryInfo* pBindInfos) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateBindBufferMemory2KHR(device, bindInfoCount, pBindInfos);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordBindBufferMemory2KHR(VkDevice device, uint32_t bindInfoCount, const VkBindBufferMemoryInfo* pBindInfos) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordBindBufferMemory2KHR(device, bindInfoCount, pBindInfos);
}

void CoreChecksOptickInstrumented::PostCallRecordBindBufferMemory2KHR(VkDevice device, uint32_t bindInfoCount, const VkBindBufferMemoryInfo* pBindInfos, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordBindBufferMemory2KHR(device, bindInfoCount, pBindInfos, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateBindImageMemory2KHR(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo* pBindInfos) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateBindImageMemory2KHR(device, bindInfoCount, pBindInfos);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordBindImageMemory2KHR(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo* pBindInfos) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordBindImageMemory2KHR(device, bindInfoCount, pBindInfos);
}

void CoreChecksOptickInstrumented::PostCallRecordBindImageMemory2KHR(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo* pBindInfos, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordBindImageMemory2KHR(device, bindInfoCount, pBindInfos, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetDescriptorSetLayoutSupportKHR(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo, VkDescriptorSetLayoutSupport* pSupport) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetDescriptorSetLayoutSupportKHR(device, pCreateInfo, pSupport);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetDescriptorSetLayoutSupportKHR(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo, VkDescriptorSetLayoutSupport* pSupport) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetDescriptorSetLayoutSupportKHR(device, pCreateInfo, pSupport);
}

void CoreChecksOptickInstrumented::PostCallRecordGetDescriptorSetLayoutSupportKHR(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo, VkDescriptorSetLayoutSupport* pSupport) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetDescriptorSetLayoutSupportKHR(device, pCreateInfo, pSupport);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdDrawIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdDrawIndirectCountKHR(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdDrawIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdDrawIndirectCountKHR(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdDrawIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdDrawIndirectCountKHR(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdDrawIndexedIndirectCountKHR(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdDrawIndexedIndirectCountKHR(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdDrawIndexedIndirectCountKHR(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetSemaphoreCounterValueKHR(VkDevice device, VkSemaphore semaphore, uint64_t* pValue) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetSemaphoreCounterValueKHR(device, semaphore, pValue);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetSemaphoreCounterValueKHR(VkDevice device, VkSemaphore semaphore, uint64_t* pValue) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetSemaphoreCounterValueKHR(device, semaphore, pValue);
}

void CoreChecksOptickInstrumented::PostCallRecordGetSemaphoreCounterValueKHR(VkDevice device, VkSemaphore semaphore, uint64_t* pValue, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetSemaphoreCounterValueKHR(device, semaphore, pValue, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateWaitSemaphoresKHR(VkDevice device, const VkSemaphoreWaitInfo* pWaitInfo, uint64_t timeout) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateWaitSemaphoresKHR(device, pWaitInfo, timeout);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordWaitSemaphoresKHR(VkDevice device, const VkSemaphoreWaitInfo* pWaitInfo, uint64_t timeout) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordWaitSemaphoresKHR(device, pWaitInfo, timeout);
}

void CoreChecksOptickInstrumented::PostCallRecordWaitSemaphoresKHR(VkDevice device, const VkSemaphoreWaitInfo* pWaitInfo, uint64_t timeout, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordWaitSemaphoresKHR(device, pWaitInfo, timeout, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateSignalSemaphoreKHR(VkDevice device, const VkSemaphoreSignalInfo* pSignalInfo) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateSignalSemaphoreKHR(device, pSignalInfo);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordSignalSemaphoreKHR(VkDevice device, const VkSemaphoreSignalInfo* pSignalInfo) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordSignalSemaphoreKHR(device, pSignalInfo);
}

void CoreChecksOptickInstrumented::PostCallRecordSignalSemaphoreKHR(VkDevice device, const VkSemaphoreSignalInfo* pSignalInfo, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordSignalSemaphoreKHR(device, pSignalInfo, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetPhysicalDeviceFragmentShadingRatesKHR(VkPhysicalDevice physicalDevice, uint32_t* pFragmentShadingRateCount, VkPhysicalDeviceFragmentShadingRateKHR* pFragmentShadingRates) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetPhysicalDeviceFragmentShadingRatesKHR(physicalDevice, pFragmentShadingRateCount, pFragmentShadingRates);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetPhysicalDeviceFragmentShadingRatesKHR(VkPhysicalDevice physicalDevice, uint32_t* pFragmentShadingRateCount, VkPhysicalDeviceFragmentShadingRateKHR* pFragmentShadingRates) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetPhysicalDeviceFragmentShadingRatesKHR(physicalDevice, pFragmentShadingRateCount, pFragmentShadingRates);
}

void CoreChecksOptickInstrumented::PostCallRecordGetPhysicalDeviceFragmentShadingRatesKHR(VkPhysicalDevice physicalDevice, uint32_t* pFragmentShadingRateCount, VkPhysicalDeviceFragmentShadingRateKHR* pFragmentShadingRates, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetPhysicalDeviceFragmentShadingRatesKHR(physicalDevice, pFragmentShadingRateCount, pFragmentShadingRates, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdSetFragmentShadingRateKHR(VkCommandBuffer           commandBuffer, const VkExtent2D*                           pFragmentSize, const VkFragmentShadingRateCombinerOpKHR    combinerOps[2]) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdSetFragmentShadingRateKHR(commandBuffer, pFragmentSize, combinerOps);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdSetFragmentShadingRateKHR(VkCommandBuffer           commandBuffer, const VkExtent2D*                           pFragmentSize, const VkFragmentShadingRateCombinerOpKHR    combinerOps[2]) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdSetFragmentShadingRateKHR(commandBuffer, pFragmentSize, combinerOps);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdSetFragmentShadingRateKHR(VkCommandBuffer           commandBuffer, const VkExtent2D*                           pFragmentSize, const VkFragmentShadingRateCombinerOpKHR    combinerOps[2]) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdSetFragmentShadingRateKHR(commandBuffer, pFragmentSize, combinerOps);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetBufferDeviceAddressKHR(VkDevice device, const VkBufferDeviceAddressInfo* pInfo) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetBufferDeviceAddressKHR(device, pInfo);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetBufferDeviceAddressKHR(VkDevice device, const VkBufferDeviceAddressInfo* pInfo) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetBufferDeviceAddressKHR(device, pInfo);
}

void CoreChecksOptickInstrumented::PostCallRecordGetBufferDeviceAddressKHR(VkDevice device, const VkBufferDeviceAddressInfo* pInfo, VkDeviceAddress result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetBufferDeviceAddressKHR(device, pInfo, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetBufferOpaqueCaptureAddressKHR(VkDevice device, const VkBufferDeviceAddressInfo* pInfo) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetBufferOpaqueCaptureAddressKHR(device, pInfo);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetBufferOpaqueCaptureAddressKHR(VkDevice device, const VkBufferDeviceAddressInfo* pInfo) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetBufferOpaqueCaptureAddressKHR(device, pInfo);
}

void CoreChecksOptickInstrumented::PostCallRecordGetBufferOpaqueCaptureAddressKHR(VkDevice device, const VkBufferDeviceAddressInfo* pInfo) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetBufferOpaqueCaptureAddressKHR(device, pInfo);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetDeviceMemoryOpaqueCaptureAddressKHR(VkDevice device, const VkDeviceMemoryOpaqueCaptureAddressInfo* pInfo) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetDeviceMemoryOpaqueCaptureAddressKHR(device, pInfo);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetDeviceMemoryOpaqueCaptureAddressKHR(VkDevice device, const VkDeviceMemoryOpaqueCaptureAddressInfo* pInfo) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetDeviceMemoryOpaqueCaptureAddressKHR(device, pInfo);
}

void CoreChecksOptickInstrumented::PostCallRecordGetDeviceMemoryOpaqueCaptureAddressKHR(VkDevice device, const VkDeviceMemoryOpaqueCaptureAddressInfo* pInfo) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetDeviceMemoryOpaqueCaptureAddressKHR(device, pInfo);
}

bool CoreChecksOptickInstrumented::PreCallValidateCreateDeferredOperationKHR(VkDevice device, const VkAllocationCallbacks* pAllocator, VkDeferredOperationKHR* pDeferredOperation) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCreateDeferredOperationKHR(device, pAllocator, pDeferredOperation);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCreateDeferredOperationKHR(VkDevice device, const VkAllocationCallbacks* pAllocator, VkDeferredOperationKHR* pDeferredOperation) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCreateDeferredOperationKHR(device, pAllocator, pDeferredOperation);
}

void CoreChecksOptickInstrumented::PostCallRecordCreateDeferredOperationKHR(VkDevice device, const VkAllocationCallbacks* pAllocator, VkDeferredOperationKHR* pDeferredOperation, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCreateDeferredOperationKHR(device, pAllocator, pDeferredOperation, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateDestroyDeferredOperationKHR(VkDevice device, VkDeferredOperationKHR operation, const VkAllocationCallbacks* pAllocator) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateDestroyDeferredOperationKHR(device, operation, pAllocator);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordDestroyDeferredOperationKHR(VkDevice device, VkDeferredOperationKHR operation, const VkAllocationCallbacks* pAllocator) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordDestroyDeferredOperationKHR(device, operation, pAllocator);
}

void CoreChecksOptickInstrumented::PostCallRecordDestroyDeferredOperationKHR(VkDevice device, VkDeferredOperationKHR operation, const VkAllocationCallbacks* pAllocator) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordDestroyDeferredOperationKHR(device, operation, pAllocator);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetDeferredOperationMaxConcurrencyKHR(VkDevice device, VkDeferredOperationKHR operation) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetDeferredOperationMaxConcurrencyKHR(device, operation);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetDeferredOperationMaxConcurrencyKHR(VkDevice device, VkDeferredOperationKHR operation) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetDeferredOperationMaxConcurrencyKHR(device, operation);
}

void CoreChecksOptickInstrumented::PostCallRecordGetDeferredOperationMaxConcurrencyKHR(VkDevice device, VkDeferredOperationKHR operation) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetDeferredOperationMaxConcurrencyKHR(device, operation);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetDeferredOperationResultKHR(VkDevice device, VkDeferredOperationKHR operation) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetDeferredOperationResultKHR(device, operation);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetDeferredOperationResultKHR(VkDevice device, VkDeferredOperationKHR operation) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetDeferredOperationResultKHR(device, operation);
}

void CoreChecksOptickInstrumented::PostCallRecordGetDeferredOperationResultKHR(VkDevice device, VkDeferredOperationKHR operation, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetDeferredOperationResultKHR(device, operation, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateDeferredOperationJoinKHR(VkDevice device, VkDeferredOperationKHR operation) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateDeferredOperationJoinKHR(device, operation);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordDeferredOperationJoinKHR(VkDevice device, VkDeferredOperationKHR operation) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordDeferredOperationJoinKHR(device, operation);
}

void CoreChecksOptickInstrumented::PostCallRecordDeferredOperationJoinKHR(VkDevice device, VkDeferredOperationKHR operation, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordDeferredOperationJoinKHR(device, operation, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetPipelineExecutablePropertiesKHR(VkDevice                        device, const VkPipelineInfoKHR*        pPipelineInfo, uint32_t* pExecutableCount, VkPipelineExecutablePropertiesKHR* pProperties) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetPipelineExecutablePropertiesKHR(device, pPipelineInfo, pExecutableCount, pProperties);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetPipelineExecutablePropertiesKHR(VkDevice                        device, const VkPipelineInfoKHR*        pPipelineInfo, uint32_t* pExecutableCount, VkPipelineExecutablePropertiesKHR* pProperties) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetPipelineExecutablePropertiesKHR(device, pPipelineInfo, pExecutableCount, pProperties);
}

void CoreChecksOptickInstrumented::PostCallRecordGetPipelineExecutablePropertiesKHR(VkDevice                        device, const VkPipelineInfoKHR*        pPipelineInfo, uint32_t* pExecutableCount, VkPipelineExecutablePropertiesKHR* pProperties, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetPipelineExecutablePropertiesKHR(device, pPipelineInfo, pExecutableCount, pProperties, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetPipelineExecutableStatisticsKHR(VkDevice                        device, const VkPipelineExecutableInfoKHR*  pExecutableInfo, uint32_t* pStatisticCount, VkPipelineExecutableStatisticKHR* pStatistics) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetPipelineExecutableStatisticsKHR(device, pExecutableInfo, pStatisticCount, pStatistics);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetPipelineExecutableStatisticsKHR(VkDevice                        device, const VkPipelineExecutableInfoKHR*  pExecutableInfo, uint32_t* pStatisticCount, VkPipelineExecutableStatisticKHR* pStatistics) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetPipelineExecutableStatisticsKHR(device, pExecutableInfo, pStatisticCount, pStatistics);
}

void CoreChecksOptickInstrumented::PostCallRecordGetPipelineExecutableStatisticsKHR(VkDevice                        device, const VkPipelineExecutableInfoKHR*  pExecutableInfo, uint32_t* pStatisticCount, VkPipelineExecutableStatisticKHR* pStatistics, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetPipelineExecutableStatisticsKHR(device, pExecutableInfo, pStatisticCount, pStatistics, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetPipelineExecutableInternalRepresentationsKHR(VkDevice                        device, const VkPipelineExecutableInfoKHR*  pExecutableInfo, uint32_t* pInternalRepresentationCount, VkPipelineExecutableInternalRepresentationKHR* pInternalRepresentations) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetPipelineExecutableInternalRepresentationsKHR(device, pExecutableInfo, pInternalRepresentationCount, pInternalRepresentations);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetPipelineExecutableInternalRepresentationsKHR(VkDevice                        device, const VkPipelineExecutableInfoKHR*  pExecutableInfo, uint32_t* pInternalRepresentationCount, VkPipelineExecutableInternalRepresentationKHR* pInternalRepresentations) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetPipelineExecutableInternalRepresentationsKHR(device, pExecutableInfo, pInternalRepresentationCount, pInternalRepresentations);
}

void CoreChecksOptickInstrumented::PostCallRecordGetPipelineExecutableInternalRepresentationsKHR(VkDevice                        device, const VkPipelineExecutableInfoKHR*  pExecutableInfo, uint32_t* pInternalRepresentationCount, VkPipelineExecutableInternalRepresentationKHR* pInternalRepresentations, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetPipelineExecutableInternalRepresentationsKHR(device, pExecutableInfo, pInternalRepresentationCount, pInternalRepresentations, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdSetEvent2KHR(VkCommandBuffer                   commandBuffer, VkEvent                                             event, const VkDependencyInfoKHR*                          pDependencyInfo) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdSetEvent2KHR(commandBuffer, event, pDependencyInfo);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdSetEvent2KHR(VkCommandBuffer                   commandBuffer, VkEvent                                             event, const VkDependencyInfoKHR*                          pDependencyInfo) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdSetEvent2KHR(commandBuffer, event, pDependencyInfo);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdSetEvent2KHR(VkCommandBuffer                   commandBuffer, VkEvent                                             event, const VkDependencyInfoKHR*                          pDependencyInfo) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdSetEvent2KHR(commandBuffer, event, pDependencyInfo);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdResetEvent2KHR(VkCommandBuffer                   commandBuffer, VkEvent                                             event, VkPipelineStageFlags2KHR                            stageMask) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdResetEvent2KHR(commandBuffer, event, stageMask);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdResetEvent2KHR(VkCommandBuffer                   commandBuffer, VkEvent                                             event, VkPipelineStageFlags2KHR                            stageMask) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdResetEvent2KHR(commandBuffer, event, stageMask);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdResetEvent2KHR(VkCommandBuffer                   commandBuffer, VkEvent                                             event, VkPipelineStageFlags2KHR                            stageMask) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdResetEvent2KHR(commandBuffer, event, stageMask);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdWaitEvents2KHR(VkCommandBuffer                   commandBuffer, uint32_t                                            eventCount, const VkEvent*                     pEvents, const VkDependencyInfoKHR*         pDependencyInfos) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdWaitEvents2KHR(commandBuffer, eventCount, pEvents, pDependencyInfos);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdWaitEvents2KHR(VkCommandBuffer                   commandBuffer, uint32_t                                            eventCount, const VkEvent*                     pEvents, const VkDependencyInfoKHR*         pDependencyInfos) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdWaitEvents2KHR(commandBuffer, eventCount, pEvents, pDependencyInfos);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdWaitEvents2KHR(VkCommandBuffer                   commandBuffer, uint32_t                                            eventCount, const VkEvent*                     pEvents, const VkDependencyInfoKHR*         pDependencyInfos) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdWaitEvents2KHR(commandBuffer, eventCount, pEvents, pDependencyInfos);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdPipelineBarrier2KHR(VkCommandBuffer                   commandBuffer, const VkDependencyInfoKHR*                                pDependencyInfo) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdPipelineBarrier2KHR(commandBuffer, pDependencyInfo);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdPipelineBarrier2KHR(VkCommandBuffer                   commandBuffer, const VkDependencyInfoKHR*                                pDependencyInfo) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdPipelineBarrier2KHR(commandBuffer, pDependencyInfo);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdPipelineBarrier2KHR(VkCommandBuffer                   commandBuffer, const VkDependencyInfoKHR*                                pDependencyInfo) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdPipelineBarrier2KHR(commandBuffer, pDependencyInfo);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdWriteTimestamp2KHR(VkCommandBuffer                   commandBuffer, VkPipelineStageFlags2KHR                            stage, VkQueryPool                                         queryPool, uint32_t                                            query) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdWriteTimestamp2KHR(commandBuffer, stage, queryPool, query);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdWriteTimestamp2KHR(VkCommandBuffer                   commandBuffer, VkPipelineStageFlags2KHR                            stage, VkQueryPool                                         queryPool, uint32_t                                            query) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdWriteTimestamp2KHR(commandBuffer, stage, queryPool, query);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdWriteTimestamp2KHR(VkCommandBuffer                   commandBuffer, VkPipelineStageFlags2KHR                            stage, VkQueryPool                                         queryPool, uint32_t                                            query) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdWriteTimestamp2KHR(commandBuffer, stage, queryPool, query);
}

bool CoreChecksOptickInstrumented::PreCallValidateQueueSubmit2KHR(VkQueue                           queue, uint32_t                            submitCount, const VkSubmitInfo2KHR*           pSubmits, VkFence           fence) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateQueueSubmit2KHR(queue, submitCount, pSubmits, fence);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordQueueSubmit2KHR(VkQueue                           queue, uint32_t                            submitCount, const VkSubmitInfo2KHR*           pSubmits, VkFence           fence) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordQueueSubmit2KHR(queue, submitCount, pSubmits, fence);
}

void CoreChecksOptickInstrumented::PostCallRecordQueueSubmit2KHR(VkQueue                           queue, uint32_t                            submitCount, const VkSubmitInfo2KHR*           pSubmits, VkFence           fence, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordQueueSubmit2KHR(queue, submitCount, pSubmits, fence, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdWriteBufferMarker2AMD(VkCommandBuffer                   commandBuffer, VkPipelineStageFlags2KHR                            stage, VkBuffer                                            dstBuffer, VkDeviceSize                                        dstOffset, uint32_t                                            marker) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdWriteBufferMarker2AMD(commandBuffer, stage, dstBuffer, dstOffset, marker);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdWriteBufferMarker2AMD(VkCommandBuffer                   commandBuffer, VkPipelineStageFlags2KHR                            stage, VkBuffer                                            dstBuffer, VkDeviceSize                                        dstOffset, uint32_t                                            marker) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdWriteBufferMarker2AMD(commandBuffer, stage, dstBuffer, dstOffset, marker);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdWriteBufferMarker2AMD(VkCommandBuffer                   commandBuffer, VkPipelineStageFlags2KHR                            stage, VkBuffer                                            dstBuffer, VkDeviceSize                                        dstOffset, uint32_t                                            marker) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdWriteBufferMarker2AMD(commandBuffer, stage, dstBuffer, dstOffset, marker);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetQueueCheckpointData2NV(VkQueue queue, uint32_t* pCheckpointDataCount, VkCheckpointData2NV* pCheckpointData) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetQueueCheckpointData2NV(queue, pCheckpointDataCount, pCheckpointData);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetQueueCheckpointData2NV(VkQueue queue, uint32_t* pCheckpointDataCount, VkCheckpointData2NV* pCheckpointData) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetQueueCheckpointData2NV(queue, pCheckpointDataCount, pCheckpointData);
}

void CoreChecksOptickInstrumented::PostCallRecordGetQueueCheckpointData2NV(VkQueue queue, uint32_t* pCheckpointDataCount, VkCheckpointData2NV* pCheckpointData) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetQueueCheckpointData2NV(queue, pCheckpointDataCount, pCheckpointData);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdCopyBuffer2KHR(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2KHR* pCopyBufferInfo) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdCopyBuffer2KHR(commandBuffer, pCopyBufferInfo);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdCopyBuffer2KHR(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2KHR* pCopyBufferInfo) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdCopyBuffer2KHR(commandBuffer, pCopyBufferInfo);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdCopyBuffer2KHR(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2KHR* pCopyBufferInfo) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdCopyBuffer2KHR(commandBuffer, pCopyBufferInfo);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdCopyImage2KHR(VkCommandBuffer commandBuffer, const VkCopyImageInfo2KHR* pCopyImageInfo) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdCopyImage2KHR(commandBuffer, pCopyImageInfo);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdCopyImage2KHR(VkCommandBuffer commandBuffer, const VkCopyImageInfo2KHR* pCopyImageInfo) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdCopyImage2KHR(commandBuffer, pCopyImageInfo);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdCopyImage2KHR(VkCommandBuffer commandBuffer, const VkCopyImageInfo2KHR* pCopyImageInfo) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdCopyImage2KHR(commandBuffer, pCopyImageInfo);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdCopyBufferToImage2KHR(VkCommandBuffer commandBuffer, const VkCopyBufferToImageInfo2KHR* pCopyBufferToImageInfo) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdCopyBufferToImage2KHR(commandBuffer, pCopyBufferToImageInfo);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdCopyBufferToImage2KHR(VkCommandBuffer commandBuffer, const VkCopyBufferToImageInfo2KHR* pCopyBufferToImageInfo) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdCopyBufferToImage2KHR(commandBuffer, pCopyBufferToImageInfo);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdCopyBufferToImage2KHR(VkCommandBuffer commandBuffer, const VkCopyBufferToImageInfo2KHR* pCopyBufferToImageInfo) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdCopyBufferToImage2KHR(commandBuffer, pCopyBufferToImageInfo);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdCopyImageToBuffer2KHR(VkCommandBuffer commandBuffer, const VkCopyImageToBufferInfo2KHR* pCopyImageToBufferInfo) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdCopyImageToBuffer2KHR(commandBuffer, pCopyImageToBufferInfo);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdCopyImageToBuffer2KHR(VkCommandBuffer commandBuffer, const VkCopyImageToBufferInfo2KHR* pCopyImageToBufferInfo) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdCopyImageToBuffer2KHR(commandBuffer, pCopyImageToBufferInfo);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdCopyImageToBuffer2KHR(VkCommandBuffer commandBuffer, const VkCopyImageToBufferInfo2KHR* pCopyImageToBufferInfo) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdCopyImageToBuffer2KHR(commandBuffer, pCopyImageToBufferInfo);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdBlitImage2KHR(VkCommandBuffer commandBuffer, const VkBlitImageInfo2KHR* pBlitImageInfo) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdBlitImage2KHR(commandBuffer, pBlitImageInfo);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdBlitImage2KHR(VkCommandBuffer commandBuffer, const VkBlitImageInfo2KHR* pBlitImageInfo) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdBlitImage2KHR(commandBuffer, pBlitImageInfo);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdBlitImage2KHR(VkCommandBuffer commandBuffer, const VkBlitImageInfo2KHR* pBlitImageInfo) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdBlitImage2KHR(commandBuffer, pBlitImageInfo);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdResolveImage2KHR(VkCommandBuffer commandBuffer, const VkResolveImageInfo2KHR* pResolveImageInfo) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdResolveImage2KHR(commandBuffer, pResolveImageInfo);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdResolveImage2KHR(VkCommandBuffer commandBuffer, const VkResolveImageInfo2KHR* pResolveImageInfo) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdResolveImage2KHR(commandBuffer, pResolveImageInfo);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdResolveImage2KHR(VkCommandBuffer commandBuffer, const VkResolveImageInfo2KHR* pResolveImageInfo) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdResolveImage2KHR(commandBuffer, pResolveImageInfo);
}

bool CoreChecksOptickInstrumented::PreCallValidateCreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCreateDebugReportCallbackEXT(instance, pCreateInfo, pAllocator, pCallback);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCreateDebugReportCallbackEXT(instance, pCreateInfo, pAllocator, pCallback);
}

void CoreChecksOptickInstrumented::PostCallRecordCreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCreateDebugReportCallbackEXT(instance, pCreateInfo, pAllocator, pCallback, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateDestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* pAllocator) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateDestroyDebugReportCallbackEXT(instance, callback, pAllocator);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordDestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* pAllocator) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordDestroyDebugReportCallbackEXT(instance, callback, pAllocator);
}

void CoreChecksOptickInstrumented::PostCallRecordDestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* pAllocator) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordDestroyDebugReportCallbackEXT(instance, callback, pAllocator);
}

bool CoreChecksOptickInstrumented::PreCallValidateDebugReportMessageEXT(VkInstance instance, VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateDebugReportMessageEXT(instance, flags, objectType, object, location, messageCode, pLayerPrefix, pMessage);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordDebugReportMessageEXT(VkInstance instance, VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordDebugReportMessageEXT(instance, flags, objectType, object, location, messageCode, pLayerPrefix, pMessage);
}

void CoreChecksOptickInstrumented::PostCallRecordDebugReportMessageEXT(VkInstance instance, VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordDebugReportMessageEXT(instance, flags, objectType, object, location, messageCode, pLayerPrefix, pMessage);
}

bool CoreChecksOptickInstrumented::PreCallValidateDebugMarkerSetObjectTagEXT(VkDevice device, const VkDebugMarkerObjectTagInfoEXT* pTagInfo) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateDebugMarkerSetObjectTagEXT(device, pTagInfo);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordDebugMarkerSetObjectTagEXT(VkDevice device, const VkDebugMarkerObjectTagInfoEXT* pTagInfo) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordDebugMarkerSetObjectTagEXT(device, pTagInfo);
}

void CoreChecksOptickInstrumented::PostCallRecordDebugMarkerSetObjectTagEXT(VkDevice device, const VkDebugMarkerObjectTagInfoEXT* pTagInfo, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordDebugMarkerSetObjectTagEXT(device, pTagInfo, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateDebugMarkerSetObjectNameEXT(VkDevice device, const VkDebugMarkerObjectNameInfoEXT* pNameInfo) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateDebugMarkerSetObjectNameEXT(device, pNameInfo);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordDebugMarkerSetObjectNameEXT(VkDevice device, const VkDebugMarkerObjectNameInfoEXT* pNameInfo) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordDebugMarkerSetObjectNameEXT(device, pNameInfo);
}

void CoreChecksOptickInstrumented::PostCallRecordDebugMarkerSetObjectNameEXT(VkDevice device, const VkDebugMarkerObjectNameInfoEXT* pNameInfo, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordDebugMarkerSetObjectNameEXT(device, pNameInfo, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdDebugMarkerBeginEXT(VkCommandBuffer commandBuffer, const VkDebugMarkerMarkerInfoEXT* pMarkerInfo) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdDebugMarkerBeginEXT(commandBuffer, pMarkerInfo);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdDebugMarkerBeginEXT(VkCommandBuffer commandBuffer, const VkDebugMarkerMarkerInfoEXT* pMarkerInfo) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdDebugMarkerBeginEXT(commandBuffer, pMarkerInfo);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdDebugMarkerBeginEXT(VkCommandBuffer commandBuffer, const VkDebugMarkerMarkerInfoEXT* pMarkerInfo) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdDebugMarkerBeginEXT(commandBuffer, pMarkerInfo);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdDebugMarkerEndEXT(VkCommandBuffer commandBuffer) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdDebugMarkerEndEXT(commandBuffer);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdDebugMarkerEndEXT(VkCommandBuffer commandBuffer) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdDebugMarkerEndEXT(commandBuffer);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdDebugMarkerEndEXT(VkCommandBuffer commandBuffer) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdDebugMarkerEndEXT(commandBuffer);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdDebugMarkerInsertEXT(VkCommandBuffer commandBuffer, const VkDebugMarkerMarkerInfoEXT* pMarkerInfo) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdDebugMarkerInsertEXT(commandBuffer, pMarkerInfo);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdDebugMarkerInsertEXT(VkCommandBuffer commandBuffer, const VkDebugMarkerMarkerInfoEXT* pMarkerInfo) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdDebugMarkerInsertEXT(commandBuffer, pMarkerInfo);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdDebugMarkerInsertEXT(VkCommandBuffer commandBuffer, const VkDebugMarkerMarkerInfoEXT* pMarkerInfo) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdDebugMarkerInsertEXT(commandBuffer, pMarkerInfo);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdBindTransformFeedbackBuffersEXT(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount, const VkBuffer* pBuffers, const VkDeviceSize* pOffsets, const VkDeviceSize* pSizes) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdBindTransformFeedbackBuffersEXT(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets, pSizes);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdBindTransformFeedbackBuffersEXT(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount, const VkBuffer* pBuffers, const VkDeviceSize* pOffsets, const VkDeviceSize* pSizes) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdBindTransformFeedbackBuffersEXT(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets, pSizes);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdBindTransformFeedbackBuffersEXT(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount, const VkBuffer* pBuffers, const VkDeviceSize* pOffsets, const VkDeviceSize* pSizes) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdBindTransformFeedbackBuffersEXT(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets, pSizes);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdBeginTransformFeedbackEXT(VkCommandBuffer commandBuffer, uint32_t firstCounterBuffer, uint32_t counterBufferCount, const VkBuffer* pCounterBuffers, const VkDeviceSize* pCounterBufferOffsets) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdBeginTransformFeedbackEXT(commandBuffer, firstCounterBuffer, counterBufferCount, pCounterBuffers, pCounterBufferOffsets);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdBeginTransformFeedbackEXT(VkCommandBuffer commandBuffer, uint32_t firstCounterBuffer, uint32_t counterBufferCount, const VkBuffer* pCounterBuffers, const VkDeviceSize* pCounterBufferOffsets) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdBeginTransformFeedbackEXT(commandBuffer, firstCounterBuffer, counterBufferCount, pCounterBuffers, pCounterBufferOffsets);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdBeginTransformFeedbackEXT(VkCommandBuffer commandBuffer, uint32_t firstCounterBuffer, uint32_t counterBufferCount, const VkBuffer* pCounterBuffers, const VkDeviceSize* pCounterBufferOffsets) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdBeginTransformFeedbackEXT(commandBuffer, firstCounterBuffer, counterBufferCount, pCounterBuffers, pCounterBufferOffsets);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdEndTransformFeedbackEXT(VkCommandBuffer commandBuffer, uint32_t firstCounterBuffer, uint32_t counterBufferCount, const VkBuffer* pCounterBuffers, const VkDeviceSize* pCounterBufferOffsets) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdEndTransformFeedbackEXT(commandBuffer, firstCounterBuffer, counterBufferCount, pCounterBuffers, pCounterBufferOffsets);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdEndTransformFeedbackEXT(VkCommandBuffer commandBuffer, uint32_t firstCounterBuffer, uint32_t counterBufferCount, const VkBuffer* pCounterBuffers, const VkDeviceSize* pCounterBufferOffsets) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdEndTransformFeedbackEXT(commandBuffer, firstCounterBuffer, counterBufferCount, pCounterBuffers, pCounterBufferOffsets);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdEndTransformFeedbackEXT(VkCommandBuffer commandBuffer, uint32_t firstCounterBuffer, uint32_t counterBufferCount, const VkBuffer* pCounterBuffers, const VkDeviceSize* pCounterBufferOffsets) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdEndTransformFeedbackEXT(commandBuffer, firstCounterBuffer, counterBufferCount, pCounterBuffers, pCounterBufferOffsets);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdBeginQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query, VkQueryControlFlags flags, uint32_t index) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdBeginQueryIndexedEXT(commandBuffer, queryPool, query, flags, index);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdBeginQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query, VkQueryControlFlags flags, uint32_t index) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdBeginQueryIndexedEXT(commandBuffer, queryPool, query, flags, index);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdBeginQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query, VkQueryControlFlags flags, uint32_t index) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdBeginQueryIndexedEXT(commandBuffer, queryPool, query, flags, index);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdEndQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query, uint32_t index) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdEndQueryIndexedEXT(commandBuffer, queryPool, query, index);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdEndQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query, uint32_t index) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdEndQueryIndexedEXT(commandBuffer, queryPool, query, index);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdEndQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query, uint32_t index) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdEndQueryIndexedEXT(commandBuffer, queryPool, query, index);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdDrawIndirectByteCountEXT(VkCommandBuffer commandBuffer, uint32_t instanceCount, uint32_t firstInstance, VkBuffer counterBuffer, VkDeviceSize counterBufferOffset, uint32_t counterOffset, uint32_t vertexStride) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdDrawIndirectByteCountEXT(commandBuffer, instanceCount, firstInstance, counterBuffer, counterBufferOffset, counterOffset, vertexStride);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdDrawIndirectByteCountEXT(VkCommandBuffer commandBuffer, uint32_t instanceCount, uint32_t firstInstance, VkBuffer counterBuffer, VkDeviceSize counterBufferOffset, uint32_t counterOffset, uint32_t vertexStride) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdDrawIndirectByteCountEXT(commandBuffer, instanceCount, firstInstance, counterBuffer, counterBufferOffset, counterOffset, vertexStride);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdDrawIndirectByteCountEXT(VkCommandBuffer commandBuffer, uint32_t instanceCount, uint32_t firstInstance, VkBuffer counterBuffer, VkDeviceSize counterBufferOffset, uint32_t counterOffset, uint32_t vertexStride) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdDrawIndirectByteCountEXT(commandBuffer, instanceCount, firstInstance, counterBuffer, counterBufferOffset, counterOffset, vertexStride);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetImageViewHandleNVX(VkDevice device, const VkImageViewHandleInfoNVX* pInfo) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetImageViewHandleNVX(device, pInfo);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetImageViewHandleNVX(VkDevice device, const VkImageViewHandleInfoNVX* pInfo) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetImageViewHandleNVX(device, pInfo);
}

void CoreChecksOptickInstrumented::PostCallRecordGetImageViewHandleNVX(VkDevice device, const VkImageViewHandleInfoNVX* pInfo) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetImageViewHandleNVX(device, pInfo);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetImageViewAddressNVX(VkDevice device, VkImageView imageView, VkImageViewAddressPropertiesNVX* pProperties) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetImageViewAddressNVX(device, imageView, pProperties);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetImageViewAddressNVX(VkDevice device, VkImageView imageView, VkImageViewAddressPropertiesNVX* pProperties) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetImageViewAddressNVX(device, imageView, pProperties);
}

void CoreChecksOptickInstrumented::PostCallRecordGetImageViewAddressNVX(VkDevice device, VkImageView imageView, VkImageViewAddressPropertiesNVX* pProperties, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetImageViewAddressNVX(device, imageView, pProperties, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdDrawIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdDrawIndirectCountAMD(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdDrawIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdDrawIndirectCountAMD(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdDrawIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdDrawIndirectCountAMD(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdDrawIndexedIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdDrawIndexedIndirectCountAMD(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdDrawIndexedIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdDrawIndexedIndirectCountAMD(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdDrawIndexedIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdDrawIndexedIndirectCountAMD(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetShaderInfoAMD(VkDevice device, VkPipeline pipeline, VkShaderStageFlagBits shaderStage, VkShaderInfoTypeAMD infoType, size_t* pInfoSize, void* pInfo) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetShaderInfoAMD(device, pipeline, shaderStage, infoType, pInfoSize, pInfo);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetShaderInfoAMD(VkDevice device, VkPipeline pipeline, VkShaderStageFlagBits shaderStage, VkShaderInfoTypeAMD infoType, size_t* pInfoSize, void* pInfo) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetShaderInfoAMD(device, pipeline, shaderStage, infoType, pInfoSize, pInfo);
}

void CoreChecksOptickInstrumented::PostCallRecordGetShaderInfoAMD(VkDevice device, VkPipeline pipeline, VkShaderStageFlagBits shaderStage, VkShaderInfoTypeAMD infoType, size_t* pInfoSize, void* pInfo, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetShaderInfoAMD(device, pipeline, shaderStage, infoType, pInfoSize, pInfo, result);
}

#ifdef VK_USE_PLATFORM_GGP
bool CoreChecksOptickInstrumented::PreCallValidateCreateStreamDescriptorSurfaceGGP(VkInstance instance, const VkStreamDescriptorSurfaceCreateInfoGGP* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCreateStreamDescriptorSurfaceGGP(instance, pCreateInfo, pAllocator, pSurface);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCreateStreamDescriptorSurfaceGGP(VkInstance instance, const VkStreamDescriptorSurfaceCreateInfoGGP* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCreateStreamDescriptorSurfaceGGP(instance, pCreateInfo, pAllocator, pSurface);
}

void CoreChecksOptickInstrumented::PostCallRecordCreateStreamDescriptorSurfaceGGP(VkInstance instance, const VkStreamDescriptorSurfaceCreateInfoGGP* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCreateStreamDescriptorSurfaceGGP(instance, pCreateInfo, pAllocator, pSurface, result);
}

#endif // VK_USE_PLATFORM_GGP
bool CoreChecksOptickInstrumented::PreCallValidateGetPhysicalDeviceExternalImageFormatPropertiesNV(VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type, VkImageTiling tiling, VkImageUsageFlags usage, VkImageCreateFlags flags, VkExternalMemoryHandleTypeFlagsNV externalHandleType, VkExternalImageFormatPropertiesNV* pExternalImageFormatProperties) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetPhysicalDeviceExternalImageFormatPropertiesNV(physicalDevice, format, type, tiling, usage, flags, externalHandleType, pExternalImageFormatProperties);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetPhysicalDeviceExternalImageFormatPropertiesNV(VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type, VkImageTiling tiling, VkImageUsageFlags usage, VkImageCreateFlags flags, VkExternalMemoryHandleTypeFlagsNV externalHandleType, VkExternalImageFormatPropertiesNV* pExternalImageFormatProperties) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetPhysicalDeviceExternalImageFormatPropertiesNV(physicalDevice, format, type, tiling, usage, flags, externalHandleType, pExternalImageFormatProperties);
}

void CoreChecksOptickInstrumented::PostCallRecordGetPhysicalDeviceExternalImageFormatPropertiesNV(VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type, VkImageTiling tiling, VkImageUsageFlags usage, VkImageCreateFlags flags, VkExternalMemoryHandleTypeFlagsNV externalHandleType, VkExternalImageFormatPropertiesNV* pExternalImageFormatProperties, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetPhysicalDeviceExternalImageFormatPropertiesNV(physicalDevice, format, type, tiling, usage, flags, externalHandleType, pExternalImageFormatProperties, result);
}

#ifdef VK_USE_PLATFORM_WIN32_KHR
bool CoreChecksOptickInstrumented::PreCallValidateGetMemoryWin32HandleNV(VkDevice device, VkDeviceMemory memory, VkExternalMemoryHandleTypeFlagsNV handleType, HANDLE* pHandle) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetMemoryWin32HandleNV(device, memory, handleType, pHandle);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetMemoryWin32HandleNV(VkDevice device, VkDeviceMemory memory, VkExternalMemoryHandleTypeFlagsNV handleType, HANDLE* pHandle) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetMemoryWin32HandleNV(device, memory, handleType, pHandle);
}

void CoreChecksOptickInstrumented::PostCallRecordGetMemoryWin32HandleNV(VkDevice device, VkDeviceMemory memory, VkExternalMemoryHandleTypeFlagsNV handleType, HANDLE* pHandle, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetMemoryWin32HandleNV(device, memory, handleType, pHandle, result);
}

#endif // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_VI_NN
bool CoreChecksOptickInstrumented::PreCallValidateCreateViSurfaceNN(VkInstance instance, const VkViSurfaceCreateInfoNN* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCreateViSurfaceNN(instance, pCreateInfo, pAllocator, pSurface);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCreateViSurfaceNN(VkInstance instance, const VkViSurfaceCreateInfoNN* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCreateViSurfaceNN(instance, pCreateInfo, pAllocator, pSurface);
}

void CoreChecksOptickInstrumented::PostCallRecordCreateViSurfaceNN(VkInstance instance, const VkViSurfaceCreateInfoNN* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCreateViSurfaceNN(instance, pCreateInfo, pAllocator, pSurface, result);
}

#endif // VK_USE_PLATFORM_VI_NN
bool CoreChecksOptickInstrumented::PreCallValidateCmdBeginConditionalRenderingEXT(VkCommandBuffer commandBuffer, const VkConditionalRenderingBeginInfoEXT* pConditionalRenderingBegin) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdBeginConditionalRenderingEXT(commandBuffer, pConditionalRenderingBegin);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdBeginConditionalRenderingEXT(VkCommandBuffer commandBuffer, const VkConditionalRenderingBeginInfoEXT* pConditionalRenderingBegin) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdBeginConditionalRenderingEXT(commandBuffer, pConditionalRenderingBegin);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdBeginConditionalRenderingEXT(VkCommandBuffer commandBuffer, const VkConditionalRenderingBeginInfoEXT* pConditionalRenderingBegin) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdBeginConditionalRenderingEXT(commandBuffer, pConditionalRenderingBegin);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdEndConditionalRenderingEXT(VkCommandBuffer commandBuffer) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdEndConditionalRenderingEXT(commandBuffer);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdEndConditionalRenderingEXT(VkCommandBuffer commandBuffer) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdEndConditionalRenderingEXT(commandBuffer);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdEndConditionalRenderingEXT(VkCommandBuffer commandBuffer) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdEndConditionalRenderingEXT(commandBuffer);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdSetViewportWScalingNV(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount, const VkViewportWScalingNV* pViewportWScalings) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdSetViewportWScalingNV(commandBuffer, firstViewport, viewportCount, pViewportWScalings);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdSetViewportWScalingNV(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount, const VkViewportWScalingNV* pViewportWScalings) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdSetViewportWScalingNV(commandBuffer, firstViewport, viewportCount, pViewportWScalings);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdSetViewportWScalingNV(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount, const VkViewportWScalingNV* pViewportWScalings) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdSetViewportWScalingNV(commandBuffer, firstViewport, viewportCount, pViewportWScalings);
}

bool CoreChecksOptickInstrumented::PreCallValidateReleaseDisplayEXT(VkPhysicalDevice physicalDevice, VkDisplayKHR display) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateReleaseDisplayEXT(physicalDevice, display);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordReleaseDisplayEXT(VkPhysicalDevice physicalDevice, VkDisplayKHR display) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordReleaseDisplayEXT(physicalDevice, display);
}

void CoreChecksOptickInstrumented::PostCallRecordReleaseDisplayEXT(VkPhysicalDevice physicalDevice, VkDisplayKHR display, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordReleaseDisplayEXT(physicalDevice, display, result);
}

#ifdef VK_USE_PLATFORM_XLIB_XRANDR_EXT
bool CoreChecksOptickInstrumented::PreCallValidateAcquireXlibDisplayEXT(VkPhysicalDevice physicalDevice, Display* dpy, VkDisplayKHR display) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateAcquireXlibDisplayEXT(physicalDevice, dpy, display);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordAcquireXlibDisplayEXT(VkPhysicalDevice physicalDevice, Display* dpy, VkDisplayKHR display) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordAcquireXlibDisplayEXT(physicalDevice, dpy, display);
}

void CoreChecksOptickInstrumented::PostCallRecordAcquireXlibDisplayEXT(VkPhysicalDevice physicalDevice, Display* dpy, VkDisplayKHR display, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordAcquireXlibDisplayEXT(physicalDevice, dpy, display, result);
}

#endif // VK_USE_PLATFORM_XLIB_XRANDR_EXT
#ifdef VK_USE_PLATFORM_XLIB_XRANDR_EXT
bool CoreChecksOptickInstrumented::PreCallValidateGetRandROutputDisplayEXT(VkPhysicalDevice physicalDevice, Display* dpy, RROutput rrOutput, VkDisplayKHR* pDisplay) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetRandROutputDisplayEXT(physicalDevice, dpy, rrOutput, pDisplay);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetRandROutputDisplayEXT(VkPhysicalDevice physicalDevice, Display* dpy, RROutput rrOutput, VkDisplayKHR* pDisplay) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetRandROutputDisplayEXT(physicalDevice, dpy, rrOutput, pDisplay);
}

void CoreChecksOptickInstrumented::PostCallRecordGetRandROutputDisplayEXT(VkPhysicalDevice physicalDevice, Display* dpy, RROutput rrOutput, VkDisplayKHR* pDisplay, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetRandROutputDisplayEXT(physicalDevice, dpy, rrOutput, pDisplay, result);
}

#endif // VK_USE_PLATFORM_XLIB_XRANDR_EXT
bool CoreChecksOptickInstrumented::PreCallValidateGetPhysicalDeviceSurfaceCapabilities2EXT(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkSurfaceCapabilities2EXT* pSurfaceCapabilities) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetPhysicalDeviceSurfaceCapabilities2EXT(physicalDevice, surface, pSurfaceCapabilities);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetPhysicalDeviceSurfaceCapabilities2EXT(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkSurfaceCapabilities2EXT* pSurfaceCapabilities) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetPhysicalDeviceSurfaceCapabilities2EXT(physicalDevice, surface, pSurfaceCapabilities);
}

void CoreChecksOptickInstrumented::PostCallRecordGetPhysicalDeviceSurfaceCapabilities2EXT(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkSurfaceCapabilities2EXT* pSurfaceCapabilities, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetPhysicalDeviceSurfaceCapabilities2EXT(physicalDevice, surface, pSurfaceCapabilities, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateDisplayPowerControlEXT(VkDevice device, VkDisplayKHR display, const VkDisplayPowerInfoEXT* pDisplayPowerInfo) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateDisplayPowerControlEXT(device, display, pDisplayPowerInfo);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordDisplayPowerControlEXT(VkDevice device, VkDisplayKHR display, const VkDisplayPowerInfoEXT* pDisplayPowerInfo) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordDisplayPowerControlEXT(device, display, pDisplayPowerInfo);
}

void CoreChecksOptickInstrumented::PostCallRecordDisplayPowerControlEXT(VkDevice device, VkDisplayKHR display, const VkDisplayPowerInfoEXT* pDisplayPowerInfo, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordDisplayPowerControlEXT(device, display, pDisplayPowerInfo, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateRegisterDeviceEventEXT(VkDevice device, const VkDeviceEventInfoEXT* pDeviceEventInfo, const VkAllocationCallbacks* pAllocator, VkFence* pFence) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateRegisterDeviceEventEXT(device, pDeviceEventInfo, pAllocator, pFence);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordRegisterDeviceEventEXT(VkDevice device, const VkDeviceEventInfoEXT* pDeviceEventInfo, const VkAllocationCallbacks* pAllocator, VkFence* pFence) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordRegisterDeviceEventEXT(device, pDeviceEventInfo, pAllocator, pFence);
}

void CoreChecksOptickInstrumented::PostCallRecordRegisterDeviceEventEXT(VkDevice device, const VkDeviceEventInfoEXT* pDeviceEventInfo, const VkAllocationCallbacks* pAllocator, VkFence* pFence, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordRegisterDeviceEventEXT(device, pDeviceEventInfo, pAllocator, pFence, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateRegisterDisplayEventEXT(VkDevice device, VkDisplayKHR display, const VkDisplayEventInfoEXT* pDisplayEventInfo, const VkAllocationCallbacks* pAllocator, VkFence* pFence) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateRegisterDisplayEventEXT(device, display, pDisplayEventInfo, pAllocator, pFence);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordRegisterDisplayEventEXT(VkDevice device, VkDisplayKHR display, const VkDisplayEventInfoEXT* pDisplayEventInfo, const VkAllocationCallbacks* pAllocator, VkFence* pFence) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordRegisterDisplayEventEXT(device, display, pDisplayEventInfo, pAllocator, pFence);
}

void CoreChecksOptickInstrumented::PostCallRecordRegisterDisplayEventEXT(VkDevice device, VkDisplayKHR display, const VkDisplayEventInfoEXT* pDisplayEventInfo, const VkAllocationCallbacks* pAllocator, VkFence* pFence, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordRegisterDisplayEventEXT(device, display, pDisplayEventInfo, pAllocator, pFence, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetSwapchainCounterEXT(VkDevice device, VkSwapchainKHR swapchain, VkSurfaceCounterFlagBitsEXT counter, uint64_t* pCounterValue) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetSwapchainCounterEXT(device, swapchain, counter, pCounterValue);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetSwapchainCounterEXT(VkDevice device, VkSwapchainKHR swapchain, VkSurfaceCounterFlagBitsEXT counter, uint64_t* pCounterValue) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetSwapchainCounterEXT(device, swapchain, counter, pCounterValue);
}

void CoreChecksOptickInstrumented::PostCallRecordGetSwapchainCounterEXT(VkDevice device, VkSwapchainKHR swapchain, VkSurfaceCounterFlagBitsEXT counter, uint64_t* pCounterValue, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetSwapchainCounterEXT(device, swapchain, counter, pCounterValue, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetRefreshCycleDurationGOOGLE(VkDevice device, VkSwapchainKHR swapchain, VkRefreshCycleDurationGOOGLE* pDisplayTimingProperties) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetRefreshCycleDurationGOOGLE(device, swapchain, pDisplayTimingProperties);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetRefreshCycleDurationGOOGLE(VkDevice device, VkSwapchainKHR swapchain, VkRefreshCycleDurationGOOGLE* pDisplayTimingProperties) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetRefreshCycleDurationGOOGLE(device, swapchain, pDisplayTimingProperties);
}

void CoreChecksOptickInstrumented::PostCallRecordGetRefreshCycleDurationGOOGLE(VkDevice device, VkSwapchainKHR swapchain, VkRefreshCycleDurationGOOGLE* pDisplayTimingProperties, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetRefreshCycleDurationGOOGLE(device, swapchain, pDisplayTimingProperties, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetPastPresentationTimingGOOGLE(VkDevice device, VkSwapchainKHR swapchain, uint32_t* pPresentationTimingCount, VkPastPresentationTimingGOOGLE* pPresentationTimings) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetPastPresentationTimingGOOGLE(device, swapchain, pPresentationTimingCount, pPresentationTimings);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetPastPresentationTimingGOOGLE(VkDevice device, VkSwapchainKHR swapchain, uint32_t* pPresentationTimingCount, VkPastPresentationTimingGOOGLE* pPresentationTimings) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetPastPresentationTimingGOOGLE(device, swapchain, pPresentationTimingCount, pPresentationTimings);
}

void CoreChecksOptickInstrumented::PostCallRecordGetPastPresentationTimingGOOGLE(VkDevice device, VkSwapchainKHR swapchain, uint32_t* pPresentationTimingCount, VkPastPresentationTimingGOOGLE* pPresentationTimings, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetPastPresentationTimingGOOGLE(device, swapchain, pPresentationTimingCount, pPresentationTimings, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdSetDiscardRectangleEXT(VkCommandBuffer commandBuffer, uint32_t firstDiscardRectangle, uint32_t discardRectangleCount, const VkRect2D* pDiscardRectangles) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdSetDiscardRectangleEXT(commandBuffer, firstDiscardRectangle, discardRectangleCount, pDiscardRectangles);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdSetDiscardRectangleEXT(VkCommandBuffer commandBuffer, uint32_t firstDiscardRectangle, uint32_t discardRectangleCount, const VkRect2D* pDiscardRectangles) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdSetDiscardRectangleEXT(commandBuffer, firstDiscardRectangle, discardRectangleCount, pDiscardRectangles);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdSetDiscardRectangleEXT(VkCommandBuffer commandBuffer, uint32_t firstDiscardRectangle, uint32_t discardRectangleCount, const VkRect2D* pDiscardRectangles) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdSetDiscardRectangleEXT(commandBuffer, firstDiscardRectangle, discardRectangleCount, pDiscardRectangles);
}

bool CoreChecksOptickInstrumented::PreCallValidateSetHdrMetadataEXT(VkDevice device, uint32_t swapchainCount, const VkSwapchainKHR* pSwapchains, const VkHdrMetadataEXT* pMetadata) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateSetHdrMetadataEXT(device, swapchainCount, pSwapchains, pMetadata);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordSetHdrMetadataEXT(VkDevice device, uint32_t swapchainCount, const VkSwapchainKHR* pSwapchains, const VkHdrMetadataEXT* pMetadata) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordSetHdrMetadataEXT(device, swapchainCount, pSwapchains, pMetadata);
}

void CoreChecksOptickInstrumented::PostCallRecordSetHdrMetadataEXT(VkDevice device, uint32_t swapchainCount, const VkSwapchainKHR* pSwapchains, const VkHdrMetadataEXT* pMetadata) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordSetHdrMetadataEXT(device, swapchainCount, pSwapchains, pMetadata);
}

#ifdef VK_USE_PLATFORM_IOS_MVK
bool CoreChecksOptickInstrumented::PreCallValidateCreateIOSSurfaceMVK(VkInstance instance, const VkIOSSurfaceCreateInfoMVK* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCreateIOSSurfaceMVK(instance, pCreateInfo, pAllocator, pSurface);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCreateIOSSurfaceMVK(VkInstance instance, const VkIOSSurfaceCreateInfoMVK* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCreateIOSSurfaceMVK(instance, pCreateInfo, pAllocator, pSurface);
}

void CoreChecksOptickInstrumented::PostCallRecordCreateIOSSurfaceMVK(VkInstance instance, const VkIOSSurfaceCreateInfoMVK* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCreateIOSSurfaceMVK(instance, pCreateInfo, pAllocator, pSurface, result);
}

#endif // VK_USE_PLATFORM_IOS_MVK
#ifdef VK_USE_PLATFORM_MACOS_MVK
bool CoreChecksOptickInstrumented::PreCallValidateCreateMacOSSurfaceMVK(VkInstance instance, const VkMacOSSurfaceCreateInfoMVK* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCreateMacOSSurfaceMVK(instance, pCreateInfo, pAllocator, pSurface);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCreateMacOSSurfaceMVK(VkInstance instance, const VkMacOSSurfaceCreateInfoMVK* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCreateMacOSSurfaceMVK(instance, pCreateInfo, pAllocator, pSurface);
}

void CoreChecksOptickInstrumented::PostCallRecordCreateMacOSSurfaceMVK(VkInstance instance, const VkMacOSSurfaceCreateInfoMVK* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCreateMacOSSurfaceMVK(instance, pCreateInfo, pAllocator, pSurface, result);
}

#endif // VK_USE_PLATFORM_MACOS_MVK
bool CoreChecksOptickInstrumented::PreCallValidateSetDebugUtilsObjectNameEXT(VkDevice device, const VkDebugUtilsObjectNameInfoEXT* pNameInfo) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateSetDebugUtilsObjectNameEXT(device, pNameInfo);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordSetDebugUtilsObjectNameEXT(VkDevice device, const VkDebugUtilsObjectNameInfoEXT* pNameInfo) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordSetDebugUtilsObjectNameEXT(device, pNameInfo);
}

void CoreChecksOptickInstrumented::PostCallRecordSetDebugUtilsObjectNameEXT(VkDevice device, const VkDebugUtilsObjectNameInfoEXT* pNameInfo, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordSetDebugUtilsObjectNameEXT(device, pNameInfo, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateSetDebugUtilsObjectTagEXT(VkDevice device, const VkDebugUtilsObjectTagInfoEXT* pTagInfo) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateSetDebugUtilsObjectTagEXT(device, pTagInfo);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordSetDebugUtilsObjectTagEXT(VkDevice device, const VkDebugUtilsObjectTagInfoEXT* pTagInfo) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordSetDebugUtilsObjectTagEXT(device, pTagInfo);
}

void CoreChecksOptickInstrumented::PostCallRecordSetDebugUtilsObjectTagEXT(VkDevice device, const VkDebugUtilsObjectTagInfoEXT* pTagInfo, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordSetDebugUtilsObjectTagEXT(device, pTagInfo, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateQueueBeginDebugUtilsLabelEXT(VkQueue queue, const VkDebugUtilsLabelEXT* pLabelInfo) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateQueueBeginDebugUtilsLabelEXT(queue, pLabelInfo);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordQueueBeginDebugUtilsLabelEXT(VkQueue queue, const VkDebugUtilsLabelEXT* pLabelInfo) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordQueueBeginDebugUtilsLabelEXT(queue, pLabelInfo);
}

void CoreChecksOptickInstrumented::PostCallRecordQueueBeginDebugUtilsLabelEXT(VkQueue queue, const VkDebugUtilsLabelEXT* pLabelInfo) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordQueueBeginDebugUtilsLabelEXT(queue, pLabelInfo);
}

bool CoreChecksOptickInstrumented::PreCallValidateQueueEndDebugUtilsLabelEXT(VkQueue queue) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateQueueEndDebugUtilsLabelEXT(queue);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordQueueEndDebugUtilsLabelEXT(VkQueue queue) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordQueueEndDebugUtilsLabelEXT(queue);
}

void CoreChecksOptickInstrumented::PostCallRecordQueueEndDebugUtilsLabelEXT(VkQueue queue) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordQueueEndDebugUtilsLabelEXT(queue);
}

bool CoreChecksOptickInstrumented::PreCallValidateQueueInsertDebugUtilsLabelEXT(VkQueue queue, const VkDebugUtilsLabelEXT* pLabelInfo) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateQueueInsertDebugUtilsLabelEXT(queue, pLabelInfo);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordQueueInsertDebugUtilsLabelEXT(VkQueue queue, const VkDebugUtilsLabelEXT* pLabelInfo) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordQueueInsertDebugUtilsLabelEXT(queue, pLabelInfo);
}

void CoreChecksOptickInstrumented::PostCallRecordQueueInsertDebugUtilsLabelEXT(VkQueue queue, const VkDebugUtilsLabelEXT* pLabelInfo) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordQueueInsertDebugUtilsLabelEXT(queue, pLabelInfo);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdBeginDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT* pLabelInfo) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdBeginDebugUtilsLabelEXT(commandBuffer, pLabelInfo);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdBeginDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT* pLabelInfo) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdBeginDebugUtilsLabelEXT(commandBuffer, pLabelInfo);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdBeginDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT* pLabelInfo) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdBeginDebugUtilsLabelEXT(commandBuffer, pLabelInfo);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdEndDebugUtilsLabelEXT(VkCommandBuffer commandBuffer) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdEndDebugUtilsLabelEXT(commandBuffer);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdEndDebugUtilsLabelEXT(VkCommandBuffer commandBuffer) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdEndDebugUtilsLabelEXT(commandBuffer);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdEndDebugUtilsLabelEXT(VkCommandBuffer commandBuffer) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdEndDebugUtilsLabelEXT(commandBuffer);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdInsertDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT* pLabelInfo) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdInsertDebugUtilsLabelEXT(commandBuffer, pLabelInfo);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdInsertDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT* pLabelInfo) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdInsertDebugUtilsLabelEXT(commandBuffer, pLabelInfo);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdInsertDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT* pLabelInfo) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdInsertDebugUtilsLabelEXT(commandBuffer, pLabelInfo);
}

bool CoreChecksOptickInstrumented::PreCallValidateCreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pMessenger) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCreateDebugUtilsMessengerEXT(instance, pCreateInfo, pAllocator, pMessenger);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pMessenger) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCreateDebugUtilsMessengerEXT(instance, pCreateInfo, pAllocator, pMessenger);
}

void CoreChecksOptickInstrumented::PostCallRecordCreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pMessenger, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCreateDebugUtilsMessengerEXT(instance, pCreateInfo, pAllocator, pMessenger, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateDestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT messenger, const VkAllocationCallbacks* pAllocator) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateDestroyDebugUtilsMessengerEXT(instance, messenger, pAllocator);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordDestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT messenger, const VkAllocationCallbacks* pAllocator) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordDestroyDebugUtilsMessengerEXT(instance, messenger, pAllocator);
}

void CoreChecksOptickInstrumented::PostCallRecordDestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT messenger, const VkAllocationCallbacks* pAllocator) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordDestroyDebugUtilsMessengerEXT(instance, messenger, pAllocator);
}

bool CoreChecksOptickInstrumented::PreCallValidateSubmitDebugUtilsMessageEXT(VkInstance instance, VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageTypes, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateSubmitDebugUtilsMessageEXT(instance, messageSeverity, messageTypes, pCallbackData);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordSubmitDebugUtilsMessageEXT(VkInstance instance, VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageTypes, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordSubmitDebugUtilsMessageEXT(instance, messageSeverity, messageTypes, pCallbackData);
}

void CoreChecksOptickInstrumented::PostCallRecordSubmitDebugUtilsMessageEXT(VkInstance instance, VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageTypes, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordSubmitDebugUtilsMessageEXT(instance, messageSeverity, messageTypes, pCallbackData);
}

#ifdef VK_USE_PLATFORM_ANDROID_KHR
bool CoreChecksOptickInstrumented::PreCallValidateGetAndroidHardwareBufferPropertiesANDROID(VkDevice device, const struct AHardwareBuffer* buffer, VkAndroidHardwareBufferPropertiesANDROID* pProperties) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetAndroidHardwareBufferPropertiesANDROID(device, buffer, pProperties);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetAndroidHardwareBufferPropertiesANDROID(VkDevice device, const struct AHardwareBuffer* buffer, VkAndroidHardwareBufferPropertiesANDROID* pProperties) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetAndroidHardwareBufferPropertiesANDROID(device, buffer, pProperties);
}

void CoreChecksOptickInstrumented::PostCallRecordGetAndroidHardwareBufferPropertiesANDROID(VkDevice device, const struct AHardwareBuffer* buffer, VkAndroidHardwareBufferPropertiesANDROID* pProperties, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetAndroidHardwareBufferPropertiesANDROID(device, buffer, pProperties, result);
}

#endif // VK_USE_PLATFORM_ANDROID_KHR
#ifdef VK_USE_PLATFORM_ANDROID_KHR
bool CoreChecksOptickInstrumented::PreCallValidateGetMemoryAndroidHardwareBufferANDROID(VkDevice device, const VkMemoryGetAndroidHardwareBufferInfoANDROID* pInfo, struct AHardwareBuffer** pBuffer) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetMemoryAndroidHardwareBufferANDROID(device, pInfo, pBuffer);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetMemoryAndroidHardwareBufferANDROID(VkDevice device, const VkMemoryGetAndroidHardwareBufferInfoANDROID* pInfo, struct AHardwareBuffer** pBuffer) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetMemoryAndroidHardwareBufferANDROID(device, pInfo, pBuffer);
}

void CoreChecksOptickInstrumented::PostCallRecordGetMemoryAndroidHardwareBufferANDROID(VkDevice device, const VkMemoryGetAndroidHardwareBufferInfoANDROID* pInfo, struct AHardwareBuffer** pBuffer, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetMemoryAndroidHardwareBufferANDROID(device, pInfo, pBuffer, result);
}

#endif // VK_USE_PLATFORM_ANDROID_KHR
bool CoreChecksOptickInstrumented::PreCallValidateCmdSetSampleLocationsEXT(VkCommandBuffer commandBuffer, const VkSampleLocationsInfoEXT* pSampleLocationsInfo) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdSetSampleLocationsEXT(commandBuffer, pSampleLocationsInfo);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdSetSampleLocationsEXT(VkCommandBuffer commandBuffer, const VkSampleLocationsInfoEXT* pSampleLocationsInfo) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdSetSampleLocationsEXT(commandBuffer, pSampleLocationsInfo);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdSetSampleLocationsEXT(VkCommandBuffer commandBuffer, const VkSampleLocationsInfoEXT* pSampleLocationsInfo) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdSetSampleLocationsEXT(commandBuffer, pSampleLocationsInfo);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetPhysicalDeviceMultisamplePropertiesEXT(VkPhysicalDevice physicalDevice, VkSampleCountFlagBits samples, VkMultisamplePropertiesEXT* pMultisampleProperties) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetPhysicalDeviceMultisamplePropertiesEXT(physicalDevice, samples, pMultisampleProperties);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetPhysicalDeviceMultisamplePropertiesEXT(VkPhysicalDevice physicalDevice, VkSampleCountFlagBits samples, VkMultisamplePropertiesEXT* pMultisampleProperties) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetPhysicalDeviceMultisamplePropertiesEXT(physicalDevice, samples, pMultisampleProperties);
}

void CoreChecksOptickInstrumented::PostCallRecordGetPhysicalDeviceMultisamplePropertiesEXT(VkPhysicalDevice physicalDevice, VkSampleCountFlagBits samples, VkMultisamplePropertiesEXT* pMultisampleProperties) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetPhysicalDeviceMultisamplePropertiesEXT(physicalDevice, samples, pMultisampleProperties);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetImageDrmFormatModifierPropertiesEXT(VkDevice device, VkImage image, VkImageDrmFormatModifierPropertiesEXT* pProperties) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetImageDrmFormatModifierPropertiesEXT(device, image, pProperties);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetImageDrmFormatModifierPropertiesEXT(VkDevice device, VkImage image, VkImageDrmFormatModifierPropertiesEXT* pProperties) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetImageDrmFormatModifierPropertiesEXT(device, image, pProperties);
}

void CoreChecksOptickInstrumented::PostCallRecordGetImageDrmFormatModifierPropertiesEXT(VkDevice device, VkImage image, VkImageDrmFormatModifierPropertiesEXT* pProperties, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetImageDrmFormatModifierPropertiesEXT(device, image, pProperties, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdBindShadingRateImageNV(VkCommandBuffer commandBuffer, VkImageView imageView, VkImageLayout imageLayout) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdBindShadingRateImageNV(commandBuffer, imageView, imageLayout);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdBindShadingRateImageNV(VkCommandBuffer commandBuffer, VkImageView imageView, VkImageLayout imageLayout) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdBindShadingRateImageNV(commandBuffer, imageView, imageLayout);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdBindShadingRateImageNV(VkCommandBuffer commandBuffer, VkImageView imageView, VkImageLayout imageLayout) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdBindShadingRateImageNV(commandBuffer, imageView, imageLayout);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdSetViewportShadingRatePaletteNV(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount, const VkShadingRatePaletteNV* pShadingRatePalettes) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdSetViewportShadingRatePaletteNV(commandBuffer, firstViewport, viewportCount, pShadingRatePalettes);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdSetViewportShadingRatePaletteNV(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount, const VkShadingRatePaletteNV* pShadingRatePalettes) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdSetViewportShadingRatePaletteNV(commandBuffer, firstViewport, viewportCount, pShadingRatePalettes);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdSetViewportShadingRatePaletteNV(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount, const VkShadingRatePaletteNV* pShadingRatePalettes) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdSetViewportShadingRatePaletteNV(commandBuffer, firstViewport, viewportCount, pShadingRatePalettes);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdSetCoarseSampleOrderNV(VkCommandBuffer commandBuffer, VkCoarseSampleOrderTypeNV sampleOrderType, uint32_t customSampleOrderCount, const VkCoarseSampleOrderCustomNV* pCustomSampleOrders) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdSetCoarseSampleOrderNV(commandBuffer, sampleOrderType, customSampleOrderCount, pCustomSampleOrders);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdSetCoarseSampleOrderNV(VkCommandBuffer commandBuffer, VkCoarseSampleOrderTypeNV sampleOrderType, uint32_t customSampleOrderCount, const VkCoarseSampleOrderCustomNV* pCustomSampleOrders) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdSetCoarseSampleOrderNV(commandBuffer, sampleOrderType, customSampleOrderCount, pCustomSampleOrders);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdSetCoarseSampleOrderNV(VkCommandBuffer commandBuffer, VkCoarseSampleOrderTypeNV sampleOrderType, uint32_t customSampleOrderCount, const VkCoarseSampleOrderCustomNV* pCustomSampleOrders) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdSetCoarseSampleOrderNV(commandBuffer, sampleOrderType, customSampleOrderCount, pCustomSampleOrders);
}

bool CoreChecksOptickInstrumented::PreCallValidateCreateAccelerationStructureNV(VkDevice device, const VkAccelerationStructureCreateInfoNV* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkAccelerationStructureNV* pAccelerationStructure) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCreateAccelerationStructureNV(device, pCreateInfo, pAllocator, pAccelerationStructure);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCreateAccelerationStructureNV(VkDevice device, const VkAccelerationStructureCreateInfoNV* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkAccelerationStructureNV* pAccelerationStructure) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCreateAccelerationStructureNV(device, pCreateInfo, pAllocator, pAccelerationStructure);
}

void CoreChecksOptickInstrumented::PostCallRecordCreateAccelerationStructureNV(VkDevice device, const VkAccelerationStructureCreateInfoNV* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkAccelerationStructureNV* pAccelerationStructure, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCreateAccelerationStructureNV(device, pCreateInfo, pAllocator, pAccelerationStructure, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateDestroyAccelerationStructureNV(VkDevice device, VkAccelerationStructureNV accelerationStructure, const VkAllocationCallbacks* pAllocator) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateDestroyAccelerationStructureNV(device, accelerationStructure, pAllocator);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordDestroyAccelerationStructureNV(VkDevice device, VkAccelerationStructureNV accelerationStructure, const VkAllocationCallbacks* pAllocator) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordDestroyAccelerationStructureNV(device, accelerationStructure, pAllocator);
}

void CoreChecksOptickInstrumented::PostCallRecordDestroyAccelerationStructureNV(VkDevice device, VkAccelerationStructureNV accelerationStructure, const VkAllocationCallbacks* pAllocator) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordDestroyAccelerationStructureNV(device, accelerationStructure, pAllocator);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetAccelerationStructureMemoryRequirementsNV(VkDevice device, const VkAccelerationStructureMemoryRequirementsInfoNV* pInfo, VkMemoryRequirements2KHR* pMemoryRequirements) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetAccelerationStructureMemoryRequirementsNV(device, pInfo, pMemoryRequirements);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetAccelerationStructureMemoryRequirementsNV(VkDevice device, const VkAccelerationStructureMemoryRequirementsInfoNV* pInfo, VkMemoryRequirements2KHR* pMemoryRequirements) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetAccelerationStructureMemoryRequirementsNV(device, pInfo, pMemoryRequirements);
}

void CoreChecksOptickInstrumented::PostCallRecordGetAccelerationStructureMemoryRequirementsNV(VkDevice device, const VkAccelerationStructureMemoryRequirementsInfoNV* pInfo, VkMemoryRequirements2KHR* pMemoryRequirements) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetAccelerationStructureMemoryRequirementsNV(device, pInfo, pMemoryRequirements);
}

bool CoreChecksOptickInstrumented::PreCallValidateBindAccelerationStructureMemoryNV(VkDevice device, uint32_t bindInfoCount, const VkBindAccelerationStructureMemoryInfoNV* pBindInfos) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateBindAccelerationStructureMemoryNV(device, bindInfoCount, pBindInfos);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordBindAccelerationStructureMemoryNV(VkDevice device, uint32_t bindInfoCount, const VkBindAccelerationStructureMemoryInfoNV* pBindInfos) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordBindAccelerationStructureMemoryNV(device, bindInfoCount, pBindInfos);
}

void CoreChecksOptickInstrumented::PostCallRecordBindAccelerationStructureMemoryNV(VkDevice device, uint32_t bindInfoCount, const VkBindAccelerationStructureMemoryInfoNV* pBindInfos, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordBindAccelerationStructureMemoryNV(device, bindInfoCount, pBindInfos, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdBuildAccelerationStructureNV(VkCommandBuffer commandBuffer, const VkAccelerationStructureInfoNV* pInfo, VkBuffer instanceData, VkDeviceSize instanceOffset, VkBool32 update, VkAccelerationStructureNV dst, VkAccelerationStructureNV src, VkBuffer scratch, VkDeviceSize scratchOffset) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdBuildAccelerationStructureNV(commandBuffer, pInfo, instanceData, instanceOffset, update, dst, src, scratch, scratchOffset);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdBuildAccelerationStructureNV(VkCommandBuffer commandBuffer, const VkAccelerationStructureInfoNV* pInfo, VkBuffer instanceData, VkDeviceSize instanceOffset, VkBool32 update, VkAccelerationStructureNV dst, VkAccelerationStructureNV src, VkBuffer scratch, VkDeviceSize scratchOffset) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdBuildAccelerationStructureNV(commandBuffer, pInfo, instanceData, instanceOffset, update, dst, src, scratch, scratchOffset);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdBuildAccelerationStructureNV(VkCommandBuffer commandBuffer, const VkAccelerationStructureInfoNV* pInfo, VkBuffer instanceData, VkDeviceSize instanceOffset, VkBool32 update, VkAccelerationStructureNV dst, VkAccelerationStructureNV src, VkBuffer scratch, VkDeviceSize scratchOffset) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdBuildAccelerationStructureNV(commandBuffer, pInfo, instanceData, instanceOffset, update, dst, src, scratch, scratchOffset);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdCopyAccelerationStructureNV(VkCommandBuffer commandBuffer, VkAccelerationStructureNV dst, VkAccelerationStructureNV src, VkCopyAccelerationStructureModeKHR mode) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdCopyAccelerationStructureNV(commandBuffer, dst, src, mode);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdCopyAccelerationStructureNV(VkCommandBuffer commandBuffer, VkAccelerationStructureNV dst, VkAccelerationStructureNV src, VkCopyAccelerationStructureModeKHR mode) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdCopyAccelerationStructureNV(commandBuffer, dst, src, mode);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdCopyAccelerationStructureNV(VkCommandBuffer commandBuffer, VkAccelerationStructureNV dst, VkAccelerationStructureNV src, VkCopyAccelerationStructureModeKHR mode) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdCopyAccelerationStructureNV(commandBuffer, dst, src, mode);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdTraceRaysNV(VkCommandBuffer commandBuffer, VkBuffer raygenShaderBindingTableBuffer, VkDeviceSize raygenShaderBindingOffset, VkBuffer missShaderBindingTableBuffer, VkDeviceSize missShaderBindingOffset, VkDeviceSize missShaderBindingStride, VkBuffer hitShaderBindingTableBuffer, VkDeviceSize hitShaderBindingOffset, VkDeviceSize hitShaderBindingStride, VkBuffer callableShaderBindingTableBuffer, VkDeviceSize callableShaderBindingOffset, VkDeviceSize callableShaderBindingStride, uint32_t width, uint32_t height, uint32_t depth) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdTraceRaysNV(commandBuffer, raygenShaderBindingTableBuffer, raygenShaderBindingOffset, missShaderBindingTableBuffer, missShaderBindingOffset, missShaderBindingStride, hitShaderBindingTableBuffer, hitShaderBindingOffset, hitShaderBindingStride, callableShaderBindingTableBuffer, callableShaderBindingOffset, callableShaderBindingStride, width, height, depth);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdTraceRaysNV(VkCommandBuffer commandBuffer, VkBuffer raygenShaderBindingTableBuffer, VkDeviceSize raygenShaderBindingOffset, VkBuffer missShaderBindingTableBuffer, VkDeviceSize missShaderBindingOffset, VkDeviceSize missShaderBindingStride, VkBuffer hitShaderBindingTableBuffer, VkDeviceSize hitShaderBindingOffset, VkDeviceSize hitShaderBindingStride, VkBuffer callableShaderBindingTableBuffer, VkDeviceSize callableShaderBindingOffset, VkDeviceSize callableShaderBindingStride, uint32_t width, uint32_t height, uint32_t depth) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdTraceRaysNV(commandBuffer, raygenShaderBindingTableBuffer, raygenShaderBindingOffset, missShaderBindingTableBuffer, missShaderBindingOffset, missShaderBindingStride, hitShaderBindingTableBuffer, hitShaderBindingOffset, hitShaderBindingStride, callableShaderBindingTableBuffer, callableShaderBindingOffset, callableShaderBindingStride, width, height, depth);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdTraceRaysNV(VkCommandBuffer commandBuffer, VkBuffer raygenShaderBindingTableBuffer, VkDeviceSize raygenShaderBindingOffset, VkBuffer missShaderBindingTableBuffer, VkDeviceSize missShaderBindingOffset, VkDeviceSize missShaderBindingStride, VkBuffer hitShaderBindingTableBuffer, VkDeviceSize hitShaderBindingOffset, VkDeviceSize hitShaderBindingStride, VkBuffer callableShaderBindingTableBuffer, VkDeviceSize callableShaderBindingOffset, VkDeviceSize callableShaderBindingStride, uint32_t width, uint32_t height, uint32_t depth) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdTraceRaysNV(commandBuffer, raygenShaderBindingTableBuffer, raygenShaderBindingOffset, missShaderBindingTableBuffer, missShaderBindingOffset, missShaderBindingStride, hitShaderBindingTableBuffer, hitShaderBindingOffset, hitShaderBindingStride, callableShaderBindingTableBuffer, callableShaderBindingOffset, callableShaderBindingStride, width, height, depth);
}

bool CoreChecksOptickInstrumented::PreCallValidateCreateRayTracingPipelinesNV(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkRayTracingPipelineCreateInfoNV* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines, void* extra_data) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCreateRayTracingPipelinesNV(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines, extra_data);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCreateRayTracingPipelinesNV(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkRayTracingPipelineCreateInfoNV* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines, void* extra_data) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCreateRayTracingPipelinesNV(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines, extra_data);
}

void CoreChecksOptickInstrumented::PostCallRecordCreateRayTracingPipelinesNV(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkRayTracingPipelineCreateInfoNV* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines, VkResult result, void* extra_data) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCreateRayTracingPipelinesNV(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines, result, extra_data);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetRayTracingShaderGroupHandlesKHR(VkDevice device, VkPipeline pipeline, uint32_t firstGroup, uint32_t groupCount, size_t dataSize, void* pData) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetRayTracingShaderGroupHandlesKHR(device, pipeline, firstGroup, groupCount, dataSize, pData);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetRayTracingShaderGroupHandlesKHR(VkDevice device, VkPipeline pipeline, uint32_t firstGroup, uint32_t groupCount, size_t dataSize, void* pData) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetRayTracingShaderGroupHandlesKHR(device, pipeline, firstGroup, groupCount, dataSize, pData);
}

void CoreChecksOptickInstrumented::PostCallRecordGetRayTracingShaderGroupHandlesKHR(VkDevice device, VkPipeline pipeline, uint32_t firstGroup, uint32_t groupCount, size_t dataSize, void* pData, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetRayTracingShaderGroupHandlesKHR(device, pipeline, firstGroup, groupCount, dataSize, pData, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetRayTracingShaderGroupHandlesNV(VkDevice device, VkPipeline pipeline, uint32_t firstGroup, uint32_t groupCount, size_t dataSize, void* pData) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetRayTracingShaderGroupHandlesNV(device, pipeline, firstGroup, groupCount, dataSize, pData);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetRayTracingShaderGroupHandlesNV(VkDevice device, VkPipeline pipeline, uint32_t firstGroup, uint32_t groupCount, size_t dataSize, void* pData) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetRayTracingShaderGroupHandlesNV(device, pipeline, firstGroup, groupCount, dataSize, pData);
}

void CoreChecksOptickInstrumented::PostCallRecordGetRayTracingShaderGroupHandlesNV(VkDevice device, VkPipeline pipeline, uint32_t firstGroup, uint32_t groupCount, size_t dataSize, void* pData, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetRayTracingShaderGroupHandlesNV(device, pipeline, firstGroup, groupCount, dataSize, pData, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetAccelerationStructureHandleNV(VkDevice device, VkAccelerationStructureNV accelerationStructure, size_t dataSize, void* pData) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetAccelerationStructureHandleNV(device, accelerationStructure, dataSize, pData);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetAccelerationStructureHandleNV(VkDevice device, VkAccelerationStructureNV accelerationStructure, size_t dataSize, void* pData) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetAccelerationStructureHandleNV(device, accelerationStructure, dataSize, pData);
}

void CoreChecksOptickInstrumented::PostCallRecordGetAccelerationStructureHandleNV(VkDevice device, VkAccelerationStructureNV accelerationStructure, size_t dataSize, void* pData, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetAccelerationStructureHandleNV(device, accelerationStructure, dataSize, pData, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdWriteAccelerationStructuresPropertiesNV(VkCommandBuffer commandBuffer, uint32_t accelerationStructureCount, const VkAccelerationStructureNV* pAccelerationStructures, VkQueryType queryType, VkQueryPool queryPool, uint32_t firstQuery) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdWriteAccelerationStructuresPropertiesNV(commandBuffer, accelerationStructureCount, pAccelerationStructures, queryType, queryPool, firstQuery);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdWriteAccelerationStructuresPropertiesNV(VkCommandBuffer commandBuffer, uint32_t accelerationStructureCount, const VkAccelerationStructureNV* pAccelerationStructures, VkQueryType queryType, VkQueryPool queryPool, uint32_t firstQuery) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdWriteAccelerationStructuresPropertiesNV(commandBuffer, accelerationStructureCount, pAccelerationStructures, queryType, queryPool, firstQuery);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdWriteAccelerationStructuresPropertiesNV(VkCommandBuffer commandBuffer, uint32_t accelerationStructureCount, const VkAccelerationStructureNV* pAccelerationStructures, VkQueryType queryType, VkQueryPool queryPool, uint32_t firstQuery) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdWriteAccelerationStructuresPropertiesNV(commandBuffer, accelerationStructureCount, pAccelerationStructures, queryType, queryPool, firstQuery);
}

bool CoreChecksOptickInstrumented::PreCallValidateCompileDeferredNV(VkDevice device, VkPipeline pipeline, uint32_t shader) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCompileDeferredNV(device, pipeline, shader);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCompileDeferredNV(VkDevice device, VkPipeline pipeline, uint32_t shader) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCompileDeferredNV(device, pipeline, shader);
}

void CoreChecksOptickInstrumented::PostCallRecordCompileDeferredNV(VkDevice device, VkPipeline pipeline, uint32_t shader, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCompileDeferredNV(device, pipeline, shader, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetMemoryHostPointerPropertiesEXT(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType, const void* pHostPointer, VkMemoryHostPointerPropertiesEXT* pMemoryHostPointerProperties) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetMemoryHostPointerPropertiesEXT(device, handleType, pHostPointer, pMemoryHostPointerProperties);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetMemoryHostPointerPropertiesEXT(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType, const void* pHostPointer, VkMemoryHostPointerPropertiesEXT* pMemoryHostPointerProperties) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetMemoryHostPointerPropertiesEXT(device, handleType, pHostPointer, pMemoryHostPointerProperties);
}

void CoreChecksOptickInstrumented::PostCallRecordGetMemoryHostPointerPropertiesEXT(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType, const void* pHostPointer, VkMemoryHostPointerPropertiesEXT* pMemoryHostPointerProperties, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetMemoryHostPointerPropertiesEXT(device, handleType, pHostPointer, pMemoryHostPointerProperties, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdWriteBufferMarkerAMD(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage, VkBuffer dstBuffer, VkDeviceSize dstOffset, uint32_t marker) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdWriteBufferMarkerAMD(commandBuffer, pipelineStage, dstBuffer, dstOffset, marker);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdWriteBufferMarkerAMD(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage, VkBuffer dstBuffer, VkDeviceSize dstOffset, uint32_t marker) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdWriteBufferMarkerAMD(commandBuffer, pipelineStage, dstBuffer, dstOffset, marker);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdWriteBufferMarkerAMD(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage, VkBuffer dstBuffer, VkDeviceSize dstOffset, uint32_t marker) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdWriteBufferMarkerAMD(commandBuffer, pipelineStage, dstBuffer, dstOffset, marker);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetPhysicalDeviceCalibrateableTimeDomainsEXT(VkPhysicalDevice physicalDevice, uint32_t* pTimeDomainCount, VkTimeDomainEXT* pTimeDomains) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetPhysicalDeviceCalibrateableTimeDomainsEXT(physicalDevice, pTimeDomainCount, pTimeDomains);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetPhysicalDeviceCalibrateableTimeDomainsEXT(VkPhysicalDevice physicalDevice, uint32_t* pTimeDomainCount, VkTimeDomainEXT* pTimeDomains) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetPhysicalDeviceCalibrateableTimeDomainsEXT(physicalDevice, pTimeDomainCount, pTimeDomains);
}

void CoreChecksOptickInstrumented::PostCallRecordGetPhysicalDeviceCalibrateableTimeDomainsEXT(VkPhysicalDevice physicalDevice, uint32_t* pTimeDomainCount, VkTimeDomainEXT* pTimeDomains, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetPhysicalDeviceCalibrateableTimeDomainsEXT(physicalDevice, pTimeDomainCount, pTimeDomains, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetCalibratedTimestampsEXT(VkDevice device, uint32_t timestampCount, const VkCalibratedTimestampInfoEXT* pTimestampInfos, uint64_t* pTimestamps, uint64_t* pMaxDeviation) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetCalibratedTimestampsEXT(device, timestampCount, pTimestampInfos, pTimestamps, pMaxDeviation);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetCalibratedTimestampsEXT(VkDevice device, uint32_t timestampCount, const VkCalibratedTimestampInfoEXT* pTimestampInfos, uint64_t* pTimestamps, uint64_t* pMaxDeviation) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetCalibratedTimestampsEXT(device, timestampCount, pTimestampInfos, pTimestamps, pMaxDeviation);
}

void CoreChecksOptickInstrumented::PostCallRecordGetCalibratedTimestampsEXT(VkDevice device, uint32_t timestampCount, const VkCalibratedTimestampInfoEXT* pTimestampInfos, uint64_t* pTimestamps, uint64_t* pMaxDeviation, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetCalibratedTimestampsEXT(device, timestampCount, pTimestampInfos, pTimestamps, pMaxDeviation, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdDrawMeshTasksNV(VkCommandBuffer commandBuffer, uint32_t taskCount, uint32_t firstTask) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdDrawMeshTasksNV(commandBuffer, taskCount, firstTask);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdDrawMeshTasksNV(VkCommandBuffer commandBuffer, uint32_t taskCount, uint32_t firstTask) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdDrawMeshTasksNV(commandBuffer, taskCount, firstTask);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdDrawMeshTasksNV(VkCommandBuffer commandBuffer, uint32_t taskCount, uint32_t firstTask) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdDrawMeshTasksNV(commandBuffer, taskCount, firstTask);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdDrawMeshTasksIndirectNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdDrawMeshTasksIndirectNV(commandBuffer, buffer, offset, drawCount, stride);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdDrawMeshTasksIndirectNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdDrawMeshTasksIndirectNV(commandBuffer, buffer, offset, drawCount, stride);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdDrawMeshTasksIndirectNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdDrawMeshTasksIndirectNV(commandBuffer, buffer, offset, drawCount, stride);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdDrawMeshTasksIndirectCountNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdDrawMeshTasksIndirectCountNV(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdDrawMeshTasksIndirectCountNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdDrawMeshTasksIndirectCountNV(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdDrawMeshTasksIndirectCountNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdDrawMeshTasksIndirectCountNV(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdSetExclusiveScissorNV(VkCommandBuffer commandBuffer, uint32_t firstExclusiveScissor, uint32_t exclusiveScissorCount, const VkRect2D* pExclusiveScissors) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdSetExclusiveScissorNV(commandBuffer, firstExclusiveScissor, exclusiveScissorCount, pExclusiveScissors);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdSetExclusiveScissorNV(VkCommandBuffer commandBuffer, uint32_t firstExclusiveScissor, uint32_t exclusiveScissorCount, const VkRect2D* pExclusiveScissors) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdSetExclusiveScissorNV(commandBuffer, firstExclusiveScissor, exclusiveScissorCount, pExclusiveScissors);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdSetExclusiveScissorNV(VkCommandBuffer commandBuffer, uint32_t firstExclusiveScissor, uint32_t exclusiveScissorCount, const VkRect2D* pExclusiveScissors) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdSetExclusiveScissorNV(commandBuffer, firstExclusiveScissor, exclusiveScissorCount, pExclusiveScissors);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdSetCheckpointNV(VkCommandBuffer commandBuffer, const void* pCheckpointMarker) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdSetCheckpointNV(commandBuffer, pCheckpointMarker);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdSetCheckpointNV(VkCommandBuffer commandBuffer, const void* pCheckpointMarker) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdSetCheckpointNV(commandBuffer, pCheckpointMarker);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdSetCheckpointNV(VkCommandBuffer commandBuffer, const void* pCheckpointMarker) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdSetCheckpointNV(commandBuffer, pCheckpointMarker);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetQueueCheckpointDataNV(VkQueue queue, uint32_t* pCheckpointDataCount, VkCheckpointDataNV* pCheckpointData) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetQueueCheckpointDataNV(queue, pCheckpointDataCount, pCheckpointData);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetQueueCheckpointDataNV(VkQueue queue, uint32_t* pCheckpointDataCount, VkCheckpointDataNV* pCheckpointData) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetQueueCheckpointDataNV(queue, pCheckpointDataCount, pCheckpointData);
}

void CoreChecksOptickInstrumented::PostCallRecordGetQueueCheckpointDataNV(VkQueue queue, uint32_t* pCheckpointDataCount, VkCheckpointDataNV* pCheckpointData) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetQueueCheckpointDataNV(queue, pCheckpointDataCount, pCheckpointData);
}

bool CoreChecksOptickInstrumented::PreCallValidateInitializePerformanceApiINTEL(VkDevice device, const VkInitializePerformanceApiInfoINTEL* pInitializeInfo) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateInitializePerformanceApiINTEL(device, pInitializeInfo);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordInitializePerformanceApiINTEL(VkDevice device, const VkInitializePerformanceApiInfoINTEL* pInitializeInfo) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordInitializePerformanceApiINTEL(device, pInitializeInfo);
}

void CoreChecksOptickInstrumented::PostCallRecordInitializePerformanceApiINTEL(VkDevice device, const VkInitializePerformanceApiInfoINTEL* pInitializeInfo, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordInitializePerformanceApiINTEL(device, pInitializeInfo, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateUninitializePerformanceApiINTEL(VkDevice device) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateUninitializePerformanceApiINTEL(device);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordUninitializePerformanceApiINTEL(VkDevice device) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordUninitializePerformanceApiINTEL(device);
}

void CoreChecksOptickInstrumented::PostCallRecordUninitializePerformanceApiINTEL(VkDevice device) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordUninitializePerformanceApiINTEL(device);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdSetPerformanceMarkerINTEL(VkCommandBuffer commandBuffer, const VkPerformanceMarkerInfoINTEL* pMarkerInfo) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdSetPerformanceMarkerINTEL(commandBuffer, pMarkerInfo);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdSetPerformanceMarkerINTEL(VkCommandBuffer commandBuffer, const VkPerformanceMarkerInfoINTEL* pMarkerInfo) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdSetPerformanceMarkerINTEL(commandBuffer, pMarkerInfo);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdSetPerformanceMarkerINTEL(VkCommandBuffer commandBuffer, const VkPerformanceMarkerInfoINTEL* pMarkerInfo, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdSetPerformanceMarkerINTEL(commandBuffer, pMarkerInfo, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdSetPerformanceStreamMarkerINTEL(VkCommandBuffer commandBuffer, const VkPerformanceStreamMarkerInfoINTEL* pMarkerInfo) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdSetPerformanceStreamMarkerINTEL(commandBuffer, pMarkerInfo);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdSetPerformanceStreamMarkerINTEL(VkCommandBuffer commandBuffer, const VkPerformanceStreamMarkerInfoINTEL* pMarkerInfo) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdSetPerformanceStreamMarkerINTEL(commandBuffer, pMarkerInfo);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdSetPerformanceStreamMarkerINTEL(VkCommandBuffer commandBuffer, const VkPerformanceStreamMarkerInfoINTEL* pMarkerInfo, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdSetPerformanceStreamMarkerINTEL(commandBuffer, pMarkerInfo, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdSetPerformanceOverrideINTEL(VkCommandBuffer commandBuffer, const VkPerformanceOverrideInfoINTEL* pOverrideInfo) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdSetPerformanceOverrideINTEL(commandBuffer, pOverrideInfo);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdSetPerformanceOverrideINTEL(VkCommandBuffer commandBuffer, const VkPerformanceOverrideInfoINTEL* pOverrideInfo) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdSetPerformanceOverrideINTEL(commandBuffer, pOverrideInfo);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdSetPerformanceOverrideINTEL(VkCommandBuffer commandBuffer, const VkPerformanceOverrideInfoINTEL* pOverrideInfo, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdSetPerformanceOverrideINTEL(commandBuffer, pOverrideInfo, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateAcquirePerformanceConfigurationINTEL(VkDevice device, const VkPerformanceConfigurationAcquireInfoINTEL* pAcquireInfo, VkPerformanceConfigurationINTEL* pConfiguration) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateAcquirePerformanceConfigurationINTEL(device, pAcquireInfo, pConfiguration);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordAcquirePerformanceConfigurationINTEL(VkDevice device, const VkPerformanceConfigurationAcquireInfoINTEL* pAcquireInfo, VkPerformanceConfigurationINTEL* pConfiguration) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordAcquirePerformanceConfigurationINTEL(device, pAcquireInfo, pConfiguration);
}

void CoreChecksOptickInstrumented::PostCallRecordAcquirePerformanceConfigurationINTEL(VkDevice device, const VkPerformanceConfigurationAcquireInfoINTEL* pAcquireInfo, VkPerformanceConfigurationINTEL* pConfiguration, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordAcquirePerformanceConfigurationINTEL(device, pAcquireInfo, pConfiguration, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateReleasePerformanceConfigurationINTEL(VkDevice device, VkPerformanceConfigurationINTEL configuration) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateReleasePerformanceConfigurationINTEL(device, configuration);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordReleasePerformanceConfigurationINTEL(VkDevice device, VkPerformanceConfigurationINTEL configuration) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordReleasePerformanceConfigurationINTEL(device, configuration);
}

void CoreChecksOptickInstrumented::PostCallRecordReleasePerformanceConfigurationINTEL(VkDevice device, VkPerformanceConfigurationINTEL configuration, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordReleasePerformanceConfigurationINTEL(device, configuration, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateQueueSetPerformanceConfigurationINTEL(VkQueue queue, VkPerformanceConfigurationINTEL configuration) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateQueueSetPerformanceConfigurationINTEL(queue, configuration);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordQueueSetPerformanceConfigurationINTEL(VkQueue queue, VkPerformanceConfigurationINTEL configuration) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordQueueSetPerformanceConfigurationINTEL(queue, configuration);
}

void CoreChecksOptickInstrumented::PostCallRecordQueueSetPerformanceConfigurationINTEL(VkQueue queue, VkPerformanceConfigurationINTEL configuration, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordQueueSetPerformanceConfigurationINTEL(queue, configuration, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetPerformanceParameterINTEL(VkDevice device, VkPerformanceParameterTypeINTEL parameter, VkPerformanceValueINTEL* pValue) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetPerformanceParameterINTEL(device, parameter, pValue);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetPerformanceParameterINTEL(VkDevice device, VkPerformanceParameterTypeINTEL parameter, VkPerformanceValueINTEL* pValue) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetPerformanceParameterINTEL(device, parameter, pValue);
}

void CoreChecksOptickInstrumented::PostCallRecordGetPerformanceParameterINTEL(VkDevice device, VkPerformanceParameterTypeINTEL parameter, VkPerformanceValueINTEL* pValue, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetPerformanceParameterINTEL(device, parameter, pValue, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateSetLocalDimmingAMD(VkDevice device, VkSwapchainKHR swapChain, VkBool32 localDimmingEnable) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateSetLocalDimmingAMD(device, swapChain, localDimmingEnable);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordSetLocalDimmingAMD(VkDevice device, VkSwapchainKHR swapChain, VkBool32 localDimmingEnable) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordSetLocalDimmingAMD(device, swapChain, localDimmingEnable);
}

void CoreChecksOptickInstrumented::PostCallRecordSetLocalDimmingAMD(VkDevice device, VkSwapchainKHR swapChain, VkBool32 localDimmingEnable) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordSetLocalDimmingAMD(device, swapChain, localDimmingEnable);
}

#ifdef VK_USE_PLATFORM_FUCHSIA
bool CoreChecksOptickInstrumented::PreCallValidateCreateImagePipeSurfaceFUCHSIA(VkInstance instance, const VkImagePipeSurfaceCreateInfoFUCHSIA* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCreateImagePipeSurfaceFUCHSIA(instance, pCreateInfo, pAllocator, pSurface);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCreateImagePipeSurfaceFUCHSIA(VkInstance instance, const VkImagePipeSurfaceCreateInfoFUCHSIA* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCreateImagePipeSurfaceFUCHSIA(instance, pCreateInfo, pAllocator, pSurface);
}

void CoreChecksOptickInstrumented::PostCallRecordCreateImagePipeSurfaceFUCHSIA(VkInstance instance, const VkImagePipeSurfaceCreateInfoFUCHSIA* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCreateImagePipeSurfaceFUCHSIA(instance, pCreateInfo, pAllocator, pSurface, result);
}

#endif // VK_USE_PLATFORM_FUCHSIA
#ifdef VK_USE_PLATFORM_METAL_EXT
bool CoreChecksOptickInstrumented::PreCallValidateCreateMetalSurfaceEXT(VkInstance instance, const VkMetalSurfaceCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCreateMetalSurfaceEXT(instance, pCreateInfo, pAllocator, pSurface);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCreateMetalSurfaceEXT(VkInstance instance, const VkMetalSurfaceCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCreateMetalSurfaceEXT(instance, pCreateInfo, pAllocator, pSurface);
}

void CoreChecksOptickInstrumented::PostCallRecordCreateMetalSurfaceEXT(VkInstance instance, const VkMetalSurfaceCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCreateMetalSurfaceEXT(instance, pCreateInfo, pAllocator, pSurface, result);
}

#endif // VK_USE_PLATFORM_METAL_EXT
bool CoreChecksOptickInstrumented::PreCallValidateGetBufferDeviceAddressEXT(VkDevice device, const VkBufferDeviceAddressInfo* pInfo) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetBufferDeviceAddressEXT(device, pInfo);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetBufferDeviceAddressEXT(VkDevice device, const VkBufferDeviceAddressInfo* pInfo) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetBufferDeviceAddressEXT(device, pInfo);
}

void CoreChecksOptickInstrumented::PostCallRecordGetBufferDeviceAddressEXT(VkDevice device, const VkBufferDeviceAddressInfo* pInfo, VkDeviceAddress result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetBufferDeviceAddressEXT(device, pInfo, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetPhysicalDeviceToolPropertiesEXT(VkPhysicalDevice physicalDevice, uint32_t* pToolCount, VkPhysicalDeviceToolPropertiesEXT* pToolProperties) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetPhysicalDeviceToolPropertiesEXT(physicalDevice, pToolCount, pToolProperties);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetPhysicalDeviceToolPropertiesEXT(VkPhysicalDevice physicalDevice, uint32_t* pToolCount, VkPhysicalDeviceToolPropertiesEXT* pToolProperties) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetPhysicalDeviceToolPropertiesEXT(physicalDevice, pToolCount, pToolProperties);
}

void CoreChecksOptickInstrumented::PostCallRecordGetPhysicalDeviceToolPropertiesEXT(VkPhysicalDevice physicalDevice, uint32_t* pToolCount, VkPhysicalDeviceToolPropertiesEXT* pToolProperties, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetPhysicalDeviceToolPropertiesEXT(physicalDevice, pToolCount, pToolProperties, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetPhysicalDeviceCooperativeMatrixPropertiesNV(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount, VkCooperativeMatrixPropertiesNV* pProperties) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetPhysicalDeviceCooperativeMatrixPropertiesNV(physicalDevice, pPropertyCount, pProperties);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetPhysicalDeviceCooperativeMatrixPropertiesNV(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount, VkCooperativeMatrixPropertiesNV* pProperties) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetPhysicalDeviceCooperativeMatrixPropertiesNV(physicalDevice, pPropertyCount, pProperties);
}

void CoreChecksOptickInstrumented::PostCallRecordGetPhysicalDeviceCooperativeMatrixPropertiesNV(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount, VkCooperativeMatrixPropertiesNV* pProperties, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetPhysicalDeviceCooperativeMatrixPropertiesNV(physicalDevice, pPropertyCount, pProperties, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV(VkPhysicalDevice physicalDevice, uint32_t* pCombinationCount, VkFramebufferMixedSamplesCombinationNV* pCombinations) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV(physicalDevice, pCombinationCount, pCombinations);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV(VkPhysicalDevice physicalDevice, uint32_t* pCombinationCount, VkFramebufferMixedSamplesCombinationNV* pCombinations) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV(physicalDevice, pCombinationCount, pCombinations);
}

void CoreChecksOptickInstrumented::PostCallRecordGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV(VkPhysicalDevice physicalDevice, uint32_t* pCombinationCount, VkFramebufferMixedSamplesCombinationNV* pCombinations, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV(physicalDevice, pCombinationCount, pCombinations, result);
}

#ifdef VK_USE_PLATFORM_WIN32_KHR
bool CoreChecksOptickInstrumented::PreCallValidateGetPhysicalDeviceSurfacePresentModes2EXT(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo, uint32_t* pPresentModeCount, VkPresentModeKHR* pPresentModes) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetPhysicalDeviceSurfacePresentModes2EXT(physicalDevice, pSurfaceInfo, pPresentModeCount, pPresentModes);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetPhysicalDeviceSurfacePresentModes2EXT(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo, uint32_t* pPresentModeCount, VkPresentModeKHR* pPresentModes) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetPhysicalDeviceSurfacePresentModes2EXT(physicalDevice, pSurfaceInfo, pPresentModeCount, pPresentModes);
}

void CoreChecksOptickInstrumented::PostCallRecordGetPhysicalDeviceSurfacePresentModes2EXT(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo, uint32_t* pPresentModeCount, VkPresentModeKHR* pPresentModes, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetPhysicalDeviceSurfacePresentModes2EXT(physicalDevice, pSurfaceInfo, pPresentModeCount, pPresentModes, result);
}

#endif // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
bool CoreChecksOptickInstrumented::PreCallValidateAcquireFullScreenExclusiveModeEXT(VkDevice device, VkSwapchainKHR swapchain) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateAcquireFullScreenExclusiveModeEXT(device, swapchain);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordAcquireFullScreenExclusiveModeEXT(VkDevice device, VkSwapchainKHR swapchain) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordAcquireFullScreenExclusiveModeEXT(device, swapchain);
}

void CoreChecksOptickInstrumented::PostCallRecordAcquireFullScreenExclusiveModeEXT(VkDevice device, VkSwapchainKHR swapchain, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordAcquireFullScreenExclusiveModeEXT(device, swapchain, result);
}

#endif // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
bool CoreChecksOptickInstrumented::PreCallValidateReleaseFullScreenExclusiveModeEXT(VkDevice device, VkSwapchainKHR swapchain) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateReleaseFullScreenExclusiveModeEXT(device, swapchain);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordReleaseFullScreenExclusiveModeEXT(VkDevice device, VkSwapchainKHR swapchain) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordReleaseFullScreenExclusiveModeEXT(device, swapchain);
}

void CoreChecksOptickInstrumented::PostCallRecordReleaseFullScreenExclusiveModeEXT(VkDevice device, VkSwapchainKHR swapchain, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordReleaseFullScreenExclusiveModeEXT(device, swapchain, result);
}

#endif // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
bool CoreChecksOptickInstrumented::PreCallValidateGetDeviceGroupSurfacePresentModes2EXT(VkDevice device, const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo, VkDeviceGroupPresentModeFlagsKHR* pModes) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetDeviceGroupSurfacePresentModes2EXT(device, pSurfaceInfo, pModes);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetDeviceGroupSurfacePresentModes2EXT(VkDevice device, const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo, VkDeviceGroupPresentModeFlagsKHR* pModes) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetDeviceGroupSurfacePresentModes2EXT(device, pSurfaceInfo, pModes);
}

void CoreChecksOptickInstrumented::PostCallRecordGetDeviceGroupSurfacePresentModes2EXT(VkDevice device, const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo, VkDeviceGroupPresentModeFlagsKHR* pModes, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetDeviceGroupSurfacePresentModes2EXT(device, pSurfaceInfo, pModes, result);
}

#endif // VK_USE_PLATFORM_WIN32_KHR
bool CoreChecksOptickInstrumented::PreCallValidateCreateHeadlessSurfaceEXT(VkInstance instance, const VkHeadlessSurfaceCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCreateHeadlessSurfaceEXT(instance, pCreateInfo, pAllocator, pSurface);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCreateHeadlessSurfaceEXT(VkInstance instance, const VkHeadlessSurfaceCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCreateHeadlessSurfaceEXT(instance, pCreateInfo, pAllocator, pSurface);
}

void CoreChecksOptickInstrumented::PostCallRecordCreateHeadlessSurfaceEXT(VkInstance instance, const VkHeadlessSurfaceCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCreateHeadlessSurfaceEXT(instance, pCreateInfo, pAllocator, pSurface, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdSetLineStippleEXT(VkCommandBuffer commandBuffer, uint32_t lineStippleFactor, uint16_t lineStipplePattern) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdSetLineStippleEXT(commandBuffer, lineStippleFactor, lineStipplePattern);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdSetLineStippleEXT(VkCommandBuffer commandBuffer, uint32_t lineStippleFactor, uint16_t lineStipplePattern) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdSetLineStippleEXT(commandBuffer, lineStippleFactor, lineStipplePattern);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdSetLineStippleEXT(VkCommandBuffer commandBuffer, uint32_t lineStippleFactor, uint16_t lineStipplePattern) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdSetLineStippleEXT(commandBuffer, lineStippleFactor, lineStipplePattern);
}

bool CoreChecksOptickInstrumented::PreCallValidateResetQueryPoolEXT(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateResetQueryPoolEXT(device, queryPool, firstQuery, queryCount);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordResetQueryPoolEXT(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordResetQueryPoolEXT(device, queryPool, firstQuery, queryCount);
}

void CoreChecksOptickInstrumented::PostCallRecordResetQueryPoolEXT(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordResetQueryPoolEXT(device, queryPool, firstQuery, queryCount);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdSetCullModeEXT(VkCommandBuffer commandBuffer, VkCullModeFlags cullMode) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdSetCullModeEXT(commandBuffer, cullMode);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdSetCullModeEXT(VkCommandBuffer commandBuffer, VkCullModeFlags cullMode) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdSetCullModeEXT(commandBuffer, cullMode);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdSetCullModeEXT(VkCommandBuffer commandBuffer, VkCullModeFlags cullMode) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdSetCullModeEXT(commandBuffer, cullMode);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdSetFrontFaceEXT(VkCommandBuffer commandBuffer, VkFrontFace frontFace) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdSetFrontFaceEXT(commandBuffer, frontFace);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdSetFrontFaceEXT(VkCommandBuffer commandBuffer, VkFrontFace frontFace) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdSetFrontFaceEXT(commandBuffer, frontFace);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdSetFrontFaceEXT(VkCommandBuffer commandBuffer, VkFrontFace frontFace) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdSetFrontFaceEXT(commandBuffer, frontFace);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdSetPrimitiveTopologyEXT(VkCommandBuffer commandBuffer, VkPrimitiveTopology primitiveTopology) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdSetPrimitiveTopologyEXT(commandBuffer, primitiveTopology);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdSetPrimitiveTopologyEXT(VkCommandBuffer commandBuffer, VkPrimitiveTopology primitiveTopology) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdSetPrimitiveTopologyEXT(commandBuffer, primitiveTopology);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdSetPrimitiveTopologyEXT(VkCommandBuffer commandBuffer, VkPrimitiveTopology primitiveTopology) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdSetPrimitiveTopologyEXT(commandBuffer, primitiveTopology);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdSetViewportWithCountEXT(VkCommandBuffer commandBuffer, uint32_t viewportCount, const VkViewport* pViewports) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdSetViewportWithCountEXT(commandBuffer, viewportCount, pViewports);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdSetViewportWithCountEXT(VkCommandBuffer commandBuffer, uint32_t viewportCount, const VkViewport* pViewports) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdSetViewportWithCountEXT(commandBuffer, viewportCount, pViewports);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdSetViewportWithCountEXT(VkCommandBuffer commandBuffer, uint32_t viewportCount, const VkViewport* pViewports) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdSetViewportWithCountEXT(commandBuffer, viewportCount, pViewports);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdSetScissorWithCountEXT(VkCommandBuffer commandBuffer, uint32_t scissorCount, const VkRect2D* pScissors) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdSetScissorWithCountEXT(commandBuffer, scissorCount, pScissors);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdSetScissorWithCountEXT(VkCommandBuffer commandBuffer, uint32_t scissorCount, const VkRect2D* pScissors) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdSetScissorWithCountEXT(commandBuffer, scissorCount, pScissors);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdSetScissorWithCountEXT(VkCommandBuffer commandBuffer, uint32_t scissorCount, const VkRect2D* pScissors) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdSetScissorWithCountEXT(commandBuffer, scissorCount, pScissors);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdBindVertexBuffers2EXT(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount, const VkBuffer* pBuffers, const VkDeviceSize* pOffsets, const VkDeviceSize* pSizes, const VkDeviceSize* pStrides) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdBindVertexBuffers2EXT(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets, pSizes, pStrides);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdBindVertexBuffers2EXT(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount, const VkBuffer* pBuffers, const VkDeviceSize* pOffsets, const VkDeviceSize* pSizes, const VkDeviceSize* pStrides) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdBindVertexBuffers2EXT(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets, pSizes, pStrides);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdBindVertexBuffers2EXT(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount, const VkBuffer* pBuffers, const VkDeviceSize* pOffsets, const VkDeviceSize* pSizes, const VkDeviceSize* pStrides) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdBindVertexBuffers2EXT(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets, pSizes, pStrides);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdSetDepthTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthTestEnable) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdSetDepthTestEnableEXT(commandBuffer, depthTestEnable);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdSetDepthTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthTestEnable) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdSetDepthTestEnableEXT(commandBuffer, depthTestEnable);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdSetDepthTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthTestEnable) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdSetDepthTestEnableEXT(commandBuffer, depthTestEnable);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdSetDepthWriteEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthWriteEnable) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdSetDepthWriteEnableEXT(commandBuffer, depthWriteEnable);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdSetDepthWriteEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthWriteEnable) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdSetDepthWriteEnableEXT(commandBuffer, depthWriteEnable);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdSetDepthWriteEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthWriteEnable) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdSetDepthWriteEnableEXT(commandBuffer, depthWriteEnable);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdSetDepthCompareOpEXT(VkCommandBuffer commandBuffer, VkCompareOp depthCompareOp) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdSetDepthCompareOpEXT(commandBuffer, depthCompareOp);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdSetDepthCompareOpEXT(VkCommandBuffer commandBuffer, VkCompareOp depthCompareOp) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdSetDepthCompareOpEXT(commandBuffer, depthCompareOp);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdSetDepthCompareOpEXT(VkCommandBuffer commandBuffer, VkCompareOp depthCompareOp) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdSetDepthCompareOpEXT(commandBuffer, depthCompareOp);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdSetDepthBoundsTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthBoundsTestEnable) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdSetDepthBoundsTestEnableEXT(commandBuffer, depthBoundsTestEnable);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdSetDepthBoundsTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthBoundsTestEnable) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdSetDepthBoundsTestEnableEXT(commandBuffer, depthBoundsTestEnable);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdSetDepthBoundsTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthBoundsTestEnable) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdSetDepthBoundsTestEnableEXT(commandBuffer, depthBoundsTestEnable);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdSetStencilTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 stencilTestEnable) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdSetStencilTestEnableEXT(commandBuffer, stencilTestEnable);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdSetStencilTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 stencilTestEnable) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdSetStencilTestEnableEXT(commandBuffer, stencilTestEnable);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdSetStencilTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 stencilTestEnable) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdSetStencilTestEnableEXT(commandBuffer, stencilTestEnable);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdSetStencilOpEXT(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, VkStencilOp failOp, VkStencilOp passOp, VkStencilOp depthFailOp, VkCompareOp compareOp) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdSetStencilOpEXT(commandBuffer, faceMask, failOp, passOp, depthFailOp, compareOp);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdSetStencilOpEXT(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, VkStencilOp failOp, VkStencilOp passOp, VkStencilOp depthFailOp, VkCompareOp compareOp) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdSetStencilOpEXT(commandBuffer, faceMask, failOp, passOp, depthFailOp, compareOp);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdSetStencilOpEXT(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, VkStencilOp failOp, VkStencilOp passOp, VkStencilOp depthFailOp, VkCompareOp compareOp) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdSetStencilOpEXT(commandBuffer, faceMask, failOp, passOp, depthFailOp, compareOp);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetGeneratedCommandsMemoryRequirementsNV(VkDevice device, const VkGeneratedCommandsMemoryRequirementsInfoNV* pInfo, VkMemoryRequirements2* pMemoryRequirements) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetGeneratedCommandsMemoryRequirementsNV(device, pInfo, pMemoryRequirements);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetGeneratedCommandsMemoryRequirementsNV(VkDevice device, const VkGeneratedCommandsMemoryRequirementsInfoNV* pInfo, VkMemoryRequirements2* pMemoryRequirements) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetGeneratedCommandsMemoryRequirementsNV(device, pInfo, pMemoryRequirements);
}

void CoreChecksOptickInstrumented::PostCallRecordGetGeneratedCommandsMemoryRequirementsNV(VkDevice device, const VkGeneratedCommandsMemoryRequirementsInfoNV* pInfo, VkMemoryRequirements2* pMemoryRequirements) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetGeneratedCommandsMemoryRequirementsNV(device, pInfo, pMemoryRequirements);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdPreprocessGeneratedCommandsNV(VkCommandBuffer commandBuffer, const VkGeneratedCommandsInfoNV* pGeneratedCommandsInfo) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdPreprocessGeneratedCommandsNV(commandBuffer, pGeneratedCommandsInfo);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdPreprocessGeneratedCommandsNV(VkCommandBuffer commandBuffer, const VkGeneratedCommandsInfoNV* pGeneratedCommandsInfo) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdPreprocessGeneratedCommandsNV(commandBuffer, pGeneratedCommandsInfo);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdPreprocessGeneratedCommandsNV(VkCommandBuffer commandBuffer, const VkGeneratedCommandsInfoNV* pGeneratedCommandsInfo) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdPreprocessGeneratedCommandsNV(commandBuffer, pGeneratedCommandsInfo);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdExecuteGeneratedCommandsNV(VkCommandBuffer commandBuffer, VkBool32 isPreprocessed, const VkGeneratedCommandsInfoNV* pGeneratedCommandsInfo) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdExecuteGeneratedCommandsNV(commandBuffer, isPreprocessed, pGeneratedCommandsInfo);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdExecuteGeneratedCommandsNV(VkCommandBuffer commandBuffer, VkBool32 isPreprocessed, const VkGeneratedCommandsInfoNV* pGeneratedCommandsInfo) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdExecuteGeneratedCommandsNV(commandBuffer, isPreprocessed, pGeneratedCommandsInfo);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdExecuteGeneratedCommandsNV(VkCommandBuffer commandBuffer, VkBool32 isPreprocessed, const VkGeneratedCommandsInfoNV* pGeneratedCommandsInfo) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdExecuteGeneratedCommandsNV(commandBuffer, isPreprocessed, pGeneratedCommandsInfo);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdBindPipelineShaderGroupNV(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline, uint32_t groupIndex) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdBindPipelineShaderGroupNV(commandBuffer, pipelineBindPoint, pipeline, groupIndex);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdBindPipelineShaderGroupNV(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline, uint32_t groupIndex) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdBindPipelineShaderGroupNV(commandBuffer, pipelineBindPoint, pipeline, groupIndex);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdBindPipelineShaderGroupNV(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline, uint32_t groupIndex) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdBindPipelineShaderGroupNV(commandBuffer, pipelineBindPoint, pipeline, groupIndex);
}

bool CoreChecksOptickInstrumented::PreCallValidateCreateIndirectCommandsLayoutNV(VkDevice device, const VkIndirectCommandsLayoutCreateInfoNV* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkIndirectCommandsLayoutNV* pIndirectCommandsLayout) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCreateIndirectCommandsLayoutNV(device, pCreateInfo, pAllocator, pIndirectCommandsLayout);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCreateIndirectCommandsLayoutNV(VkDevice device, const VkIndirectCommandsLayoutCreateInfoNV* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkIndirectCommandsLayoutNV* pIndirectCommandsLayout) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCreateIndirectCommandsLayoutNV(device, pCreateInfo, pAllocator, pIndirectCommandsLayout);
}

void CoreChecksOptickInstrumented::PostCallRecordCreateIndirectCommandsLayoutNV(VkDevice device, const VkIndirectCommandsLayoutCreateInfoNV* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkIndirectCommandsLayoutNV* pIndirectCommandsLayout, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCreateIndirectCommandsLayoutNV(device, pCreateInfo, pAllocator, pIndirectCommandsLayout, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateDestroyIndirectCommandsLayoutNV(VkDevice device, VkIndirectCommandsLayoutNV indirectCommandsLayout, const VkAllocationCallbacks* pAllocator) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateDestroyIndirectCommandsLayoutNV(device, indirectCommandsLayout, pAllocator);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordDestroyIndirectCommandsLayoutNV(VkDevice device, VkIndirectCommandsLayoutNV indirectCommandsLayout, const VkAllocationCallbacks* pAllocator) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordDestroyIndirectCommandsLayoutNV(device, indirectCommandsLayout, pAllocator);
}

void CoreChecksOptickInstrumented::PostCallRecordDestroyIndirectCommandsLayoutNV(VkDevice device, VkIndirectCommandsLayoutNV indirectCommandsLayout, const VkAllocationCallbacks* pAllocator) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordDestroyIndirectCommandsLayoutNV(device, indirectCommandsLayout, pAllocator);
}

bool CoreChecksOptickInstrumented::PreCallValidateCreatePrivateDataSlotEXT(VkDevice device, const VkPrivateDataSlotCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkPrivateDataSlotEXT* pPrivateDataSlot) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCreatePrivateDataSlotEXT(device, pCreateInfo, pAllocator, pPrivateDataSlot);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCreatePrivateDataSlotEXT(VkDevice device, const VkPrivateDataSlotCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkPrivateDataSlotEXT* pPrivateDataSlot) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCreatePrivateDataSlotEXT(device, pCreateInfo, pAllocator, pPrivateDataSlot);
}

void CoreChecksOptickInstrumented::PostCallRecordCreatePrivateDataSlotEXT(VkDevice device, const VkPrivateDataSlotCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkPrivateDataSlotEXT* pPrivateDataSlot, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCreatePrivateDataSlotEXT(device, pCreateInfo, pAllocator, pPrivateDataSlot, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateDestroyPrivateDataSlotEXT(VkDevice device, VkPrivateDataSlotEXT privateDataSlot, const VkAllocationCallbacks* pAllocator) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateDestroyPrivateDataSlotEXT(device, privateDataSlot, pAllocator);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordDestroyPrivateDataSlotEXT(VkDevice device, VkPrivateDataSlotEXT privateDataSlot, const VkAllocationCallbacks* pAllocator) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordDestroyPrivateDataSlotEXT(device, privateDataSlot, pAllocator);
}

void CoreChecksOptickInstrumented::PostCallRecordDestroyPrivateDataSlotEXT(VkDevice device, VkPrivateDataSlotEXT privateDataSlot, const VkAllocationCallbacks* pAllocator) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordDestroyPrivateDataSlotEXT(device, privateDataSlot, pAllocator);
}

bool CoreChecksOptickInstrumented::PreCallValidateSetPrivateDataEXT(VkDevice device, VkObjectType objectType, uint64_t objectHandle, VkPrivateDataSlotEXT privateDataSlot, uint64_t data) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateSetPrivateDataEXT(device, objectType, objectHandle, privateDataSlot, data);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordSetPrivateDataEXT(VkDevice device, VkObjectType objectType, uint64_t objectHandle, VkPrivateDataSlotEXT privateDataSlot, uint64_t data) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordSetPrivateDataEXT(device, objectType, objectHandle, privateDataSlot, data);
}

void CoreChecksOptickInstrumented::PostCallRecordSetPrivateDataEXT(VkDevice device, VkObjectType objectType, uint64_t objectHandle, VkPrivateDataSlotEXT privateDataSlot, uint64_t data, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordSetPrivateDataEXT(device, objectType, objectHandle, privateDataSlot, data, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetPrivateDataEXT(VkDevice device, VkObjectType objectType, uint64_t objectHandle, VkPrivateDataSlotEXT privateDataSlot, uint64_t* pData) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetPrivateDataEXT(device, objectType, objectHandle, privateDataSlot, pData);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetPrivateDataEXT(VkDevice device, VkObjectType objectType, uint64_t objectHandle, VkPrivateDataSlotEXT privateDataSlot, uint64_t* pData) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetPrivateDataEXT(device, objectType, objectHandle, privateDataSlot, pData);
}

void CoreChecksOptickInstrumented::PostCallRecordGetPrivateDataEXT(VkDevice device, VkObjectType objectType, uint64_t objectHandle, VkPrivateDataSlotEXT privateDataSlot, uint64_t* pData) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetPrivateDataEXT(device, objectType, objectHandle, privateDataSlot, pData);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdSetFragmentShadingRateEnumNV(VkCommandBuffer           commandBuffer, VkFragmentShadingRateNV                     shadingRate, const VkFragmentShadingRateCombinerOpKHR    combinerOps[2]) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdSetFragmentShadingRateEnumNV(commandBuffer, shadingRate, combinerOps);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdSetFragmentShadingRateEnumNV(VkCommandBuffer           commandBuffer, VkFragmentShadingRateNV                     shadingRate, const VkFragmentShadingRateCombinerOpKHR    combinerOps[2]) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdSetFragmentShadingRateEnumNV(commandBuffer, shadingRate, combinerOps);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdSetFragmentShadingRateEnumNV(VkCommandBuffer           commandBuffer, VkFragmentShadingRateNV                     shadingRate, const VkFragmentShadingRateCombinerOpKHR    combinerOps[2]) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdSetFragmentShadingRateEnumNV(commandBuffer, shadingRate, combinerOps);
}

#ifdef VK_USE_PLATFORM_WIN32_KHR
bool CoreChecksOptickInstrumented::PreCallValidateAcquireWinrtDisplayNV(VkPhysicalDevice physicalDevice, VkDisplayKHR display) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateAcquireWinrtDisplayNV(physicalDevice, display);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordAcquireWinrtDisplayNV(VkPhysicalDevice physicalDevice, VkDisplayKHR display) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordAcquireWinrtDisplayNV(physicalDevice, display);
}

void CoreChecksOptickInstrumented::PostCallRecordAcquireWinrtDisplayNV(VkPhysicalDevice physicalDevice, VkDisplayKHR display, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordAcquireWinrtDisplayNV(physicalDevice, display, result);
}

#endif // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
bool CoreChecksOptickInstrumented::PreCallValidateGetWinrtDisplayNV(VkPhysicalDevice physicalDevice, uint32_t deviceRelativeId, VkDisplayKHR* pDisplay) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetWinrtDisplayNV(physicalDevice, deviceRelativeId, pDisplay);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetWinrtDisplayNV(VkPhysicalDevice physicalDevice, uint32_t deviceRelativeId, VkDisplayKHR* pDisplay) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetWinrtDisplayNV(physicalDevice, deviceRelativeId, pDisplay);
}

void CoreChecksOptickInstrumented::PostCallRecordGetWinrtDisplayNV(VkPhysicalDevice physicalDevice, uint32_t deviceRelativeId, VkDisplayKHR* pDisplay, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetWinrtDisplayNV(physicalDevice, deviceRelativeId, pDisplay, result);
}

#endif // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_DIRECTFB_EXT
bool CoreChecksOptickInstrumented::PreCallValidateCreateDirectFBSurfaceEXT(VkInstance instance, const VkDirectFBSurfaceCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCreateDirectFBSurfaceEXT(instance, pCreateInfo, pAllocator, pSurface);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCreateDirectFBSurfaceEXT(VkInstance instance, const VkDirectFBSurfaceCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCreateDirectFBSurfaceEXT(instance, pCreateInfo, pAllocator, pSurface);
}

void CoreChecksOptickInstrumented::PostCallRecordCreateDirectFBSurfaceEXT(VkInstance instance, const VkDirectFBSurfaceCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCreateDirectFBSurfaceEXT(instance, pCreateInfo, pAllocator, pSurface, result);
}

#endif // VK_USE_PLATFORM_DIRECTFB_EXT
#ifdef VK_USE_PLATFORM_DIRECTFB_EXT
bool CoreChecksOptickInstrumented::PreCallValidateGetPhysicalDeviceDirectFBPresentationSupportEXT(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, IDirectFB* dfb) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetPhysicalDeviceDirectFBPresentationSupportEXT(physicalDevice, queueFamilyIndex, dfb);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetPhysicalDeviceDirectFBPresentationSupportEXT(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, IDirectFB* dfb) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetPhysicalDeviceDirectFBPresentationSupportEXT(physicalDevice, queueFamilyIndex, dfb);
}

void CoreChecksOptickInstrumented::PostCallRecordGetPhysicalDeviceDirectFBPresentationSupportEXT(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, IDirectFB* dfb) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetPhysicalDeviceDirectFBPresentationSupportEXT(physicalDevice, queueFamilyIndex, dfb);
}

#endif // VK_USE_PLATFORM_DIRECTFB_EXT
bool CoreChecksOptickInstrumented::PreCallValidateCreateAccelerationStructureKHR(VkDevice                                           device, const VkAccelerationStructureCreateInfoKHR*        pCreateInfo, const VkAllocationCallbacks*       pAllocator, VkAccelerationStructureKHR*                        pAccelerationStructure) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCreateAccelerationStructureKHR(device, pCreateInfo, pAllocator, pAccelerationStructure);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCreateAccelerationStructureKHR(VkDevice                                           device, const VkAccelerationStructureCreateInfoKHR*        pCreateInfo, const VkAllocationCallbacks*       pAllocator, VkAccelerationStructureKHR*                        pAccelerationStructure) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCreateAccelerationStructureKHR(device, pCreateInfo, pAllocator, pAccelerationStructure);
}

void CoreChecksOptickInstrumented::PostCallRecordCreateAccelerationStructureKHR(VkDevice                                           device, const VkAccelerationStructureCreateInfoKHR*        pCreateInfo, const VkAllocationCallbacks*       pAllocator, VkAccelerationStructureKHR*                        pAccelerationStructure, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCreateAccelerationStructureKHR(device, pCreateInfo, pAllocator, pAccelerationStructure, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateDestroyAccelerationStructureKHR(VkDevice device, VkAccelerationStructureKHR accelerationStructure, const VkAllocationCallbacks* pAllocator) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateDestroyAccelerationStructureKHR(device, accelerationStructure, pAllocator);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordDestroyAccelerationStructureKHR(VkDevice device, VkAccelerationStructureKHR accelerationStructure, const VkAllocationCallbacks* pAllocator) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordDestroyAccelerationStructureKHR(device, accelerationStructure, pAllocator);
}

void CoreChecksOptickInstrumented::PostCallRecordDestroyAccelerationStructureKHR(VkDevice device, VkAccelerationStructureKHR accelerationStructure, const VkAllocationCallbacks* pAllocator) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordDestroyAccelerationStructureKHR(device, accelerationStructure, pAllocator);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdBuildAccelerationStructuresKHR(VkCommandBuffer                                    commandBuffer, uint32_t infoCount, const VkAccelerationStructureBuildGeometryInfoKHR* pInfos, const VkAccelerationStructureBuildRangeInfoKHR* const* ppBuildRangeInfos) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdBuildAccelerationStructuresKHR(commandBuffer, infoCount, pInfos, ppBuildRangeInfos);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdBuildAccelerationStructuresKHR(VkCommandBuffer                                    commandBuffer, uint32_t infoCount, const VkAccelerationStructureBuildGeometryInfoKHR* pInfos, const VkAccelerationStructureBuildRangeInfoKHR* const* ppBuildRangeInfos) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdBuildAccelerationStructuresKHR(commandBuffer, infoCount, pInfos, ppBuildRangeInfos);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdBuildAccelerationStructuresKHR(VkCommandBuffer                                    commandBuffer, uint32_t infoCount, const VkAccelerationStructureBuildGeometryInfoKHR* pInfos, const VkAccelerationStructureBuildRangeInfoKHR* const* ppBuildRangeInfos) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdBuildAccelerationStructuresKHR(commandBuffer, infoCount, pInfos, ppBuildRangeInfos);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdBuildAccelerationStructuresIndirectKHR(VkCommandBuffer                  commandBuffer, uint32_t                                           infoCount, const VkAccelerationStructureBuildGeometryInfoKHR* pInfos, const VkDeviceAddress*             pIndirectDeviceAddresses, const uint32_t*                    pIndirectStrides, const uint32_t* const*             ppMaxPrimitiveCounts) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdBuildAccelerationStructuresIndirectKHR(commandBuffer, infoCount, pInfos, pIndirectDeviceAddresses, pIndirectStrides, ppMaxPrimitiveCounts);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdBuildAccelerationStructuresIndirectKHR(VkCommandBuffer                  commandBuffer, uint32_t                                           infoCount, const VkAccelerationStructureBuildGeometryInfoKHR* pInfos, const VkDeviceAddress*             pIndirectDeviceAddresses, const uint32_t*                    pIndirectStrides, const uint32_t* const*             ppMaxPrimitiveCounts) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdBuildAccelerationStructuresIndirectKHR(commandBuffer, infoCount, pInfos, pIndirectDeviceAddresses, pIndirectStrides, ppMaxPrimitiveCounts);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdBuildAccelerationStructuresIndirectKHR(VkCommandBuffer                  commandBuffer, uint32_t                                           infoCount, const VkAccelerationStructureBuildGeometryInfoKHR* pInfos, const VkDeviceAddress*             pIndirectDeviceAddresses, const uint32_t*                    pIndirectStrides, const uint32_t* const*             ppMaxPrimitiveCounts) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdBuildAccelerationStructuresIndirectKHR(commandBuffer, infoCount, pInfos, pIndirectDeviceAddresses, pIndirectStrides, ppMaxPrimitiveCounts);
}

bool CoreChecksOptickInstrumented::PreCallValidateBuildAccelerationStructuresKHR(VkDevice                                           device, VkDeferredOperationKHR deferredOperation, uint32_t infoCount, const VkAccelerationStructureBuildGeometryInfoKHR* pInfos, const VkAccelerationStructureBuildRangeInfoKHR* const* ppBuildRangeInfos) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateBuildAccelerationStructuresKHR(device, deferredOperation, infoCount, pInfos, ppBuildRangeInfos);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordBuildAccelerationStructuresKHR(VkDevice                                           device, VkDeferredOperationKHR deferredOperation, uint32_t infoCount, const VkAccelerationStructureBuildGeometryInfoKHR* pInfos, const VkAccelerationStructureBuildRangeInfoKHR* const* ppBuildRangeInfos) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordBuildAccelerationStructuresKHR(device, deferredOperation, infoCount, pInfos, ppBuildRangeInfos);
}

void CoreChecksOptickInstrumented::PostCallRecordBuildAccelerationStructuresKHR(VkDevice                                           device, VkDeferredOperationKHR deferredOperation, uint32_t infoCount, const VkAccelerationStructureBuildGeometryInfoKHR* pInfos, const VkAccelerationStructureBuildRangeInfoKHR* const* ppBuildRangeInfos, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordBuildAccelerationStructuresKHR(device, deferredOperation, infoCount, pInfos, ppBuildRangeInfos, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateCopyAccelerationStructureKHR(VkDevice device, VkDeferredOperationKHR deferredOperation, const VkCopyAccelerationStructureInfoKHR* pInfo) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCopyAccelerationStructureKHR(device, deferredOperation, pInfo);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCopyAccelerationStructureKHR(VkDevice device, VkDeferredOperationKHR deferredOperation, const VkCopyAccelerationStructureInfoKHR* pInfo) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCopyAccelerationStructureKHR(device, deferredOperation, pInfo);
}

void CoreChecksOptickInstrumented::PostCallRecordCopyAccelerationStructureKHR(VkDevice device, VkDeferredOperationKHR deferredOperation, const VkCopyAccelerationStructureInfoKHR* pInfo, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCopyAccelerationStructureKHR(device, deferredOperation, pInfo, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateCopyAccelerationStructureToMemoryKHR(VkDevice device, VkDeferredOperationKHR deferredOperation, const VkCopyAccelerationStructureToMemoryInfoKHR* pInfo) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCopyAccelerationStructureToMemoryKHR(device, deferredOperation, pInfo);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCopyAccelerationStructureToMemoryKHR(VkDevice device, VkDeferredOperationKHR deferredOperation, const VkCopyAccelerationStructureToMemoryInfoKHR* pInfo) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCopyAccelerationStructureToMemoryKHR(device, deferredOperation, pInfo);
}

void CoreChecksOptickInstrumented::PostCallRecordCopyAccelerationStructureToMemoryKHR(VkDevice device, VkDeferredOperationKHR deferredOperation, const VkCopyAccelerationStructureToMemoryInfoKHR* pInfo, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCopyAccelerationStructureToMemoryKHR(device, deferredOperation, pInfo, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateCopyMemoryToAccelerationStructureKHR(VkDevice device, VkDeferredOperationKHR deferredOperation, const VkCopyMemoryToAccelerationStructureInfoKHR* pInfo) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCopyMemoryToAccelerationStructureKHR(device, deferredOperation, pInfo);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCopyMemoryToAccelerationStructureKHR(VkDevice device, VkDeferredOperationKHR deferredOperation, const VkCopyMemoryToAccelerationStructureInfoKHR* pInfo) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCopyMemoryToAccelerationStructureKHR(device, deferredOperation, pInfo);
}

void CoreChecksOptickInstrumented::PostCallRecordCopyMemoryToAccelerationStructureKHR(VkDevice device, VkDeferredOperationKHR deferredOperation, const VkCopyMemoryToAccelerationStructureInfoKHR* pInfo, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCopyMemoryToAccelerationStructureKHR(device, deferredOperation, pInfo, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateWriteAccelerationStructuresPropertiesKHR(VkDevice device, uint32_t accelerationStructureCount, const VkAccelerationStructureKHR* pAccelerationStructures, VkQueryType  queryType, size_t       dataSize, void* pData, size_t stride) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateWriteAccelerationStructuresPropertiesKHR(device, accelerationStructureCount, pAccelerationStructures, queryType, dataSize, pData, stride);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordWriteAccelerationStructuresPropertiesKHR(VkDevice device, uint32_t accelerationStructureCount, const VkAccelerationStructureKHR* pAccelerationStructures, VkQueryType  queryType, size_t       dataSize, void* pData, size_t stride) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordWriteAccelerationStructuresPropertiesKHR(device, accelerationStructureCount, pAccelerationStructures, queryType, dataSize, pData, stride);
}

void CoreChecksOptickInstrumented::PostCallRecordWriteAccelerationStructuresPropertiesKHR(VkDevice device, uint32_t accelerationStructureCount, const VkAccelerationStructureKHR* pAccelerationStructures, VkQueryType  queryType, size_t       dataSize, void* pData, size_t stride, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordWriteAccelerationStructuresPropertiesKHR(device, accelerationStructureCount, pAccelerationStructures, queryType, dataSize, pData, stride, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdCopyAccelerationStructureKHR(VkCommandBuffer commandBuffer, const VkCopyAccelerationStructureInfoKHR* pInfo) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdCopyAccelerationStructureKHR(commandBuffer, pInfo);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdCopyAccelerationStructureKHR(VkCommandBuffer commandBuffer, const VkCopyAccelerationStructureInfoKHR* pInfo) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdCopyAccelerationStructureKHR(commandBuffer, pInfo);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdCopyAccelerationStructureKHR(VkCommandBuffer commandBuffer, const VkCopyAccelerationStructureInfoKHR* pInfo) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdCopyAccelerationStructureKHR(commandBuffer, pInfo);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdCopyAccelerationStructureToMemoryKHR(VkCommandBuffer commandBuffer, const VkCopyAccelerationStructureToMemoryInfoKHR* pInfo) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdCopyAccelerationStructureToMemoryKHR(commandBuffer, pInfo);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdCopyAccelerationStructureToMemoryKHR(VkCommandBuffer commandBuffer, const VkCopyAccelerationStructureToMemoryInfoKHR* pInfo) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdCopyAccelerationStructureToMemoryKHR(commandBuffer, pInfo);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdCopyAccelerationStructureToMemoryKHR(VkCommandBuffer commandBuffer, const VkCopyAccelerationStructureToMemoryInfoKHR* pInfo) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdCopyAccelerationStructureToMemoryKHR(commandBuffer, pInfo);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdCopyMemoryToAccelerationStructureKHR(VkCommandBuffer commandBuffer, const VkCopyMemoryToAccelerationStructureInfoKHR* pInfo) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdCopyMemoryToAccelerationStructureKHR(commandBuffer, pInfo);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdCopyMemoryToAccelerationStructureKHR(VkCommandBuffer commandBuffer, const VkCopyMemoryToAccelerationStructureInfoKHR* pInfo) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdCopyMemoryToAccelerationStructureKHR(commandBuffer, pInfo);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdCopyMemoryToAccelerationStructureKHR(VkCommandBuffer commandBuffer, const VkCopyMemoryToAccelerationStructureInfoKHR* pInfo) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdCopyMemoryToAccelerationStructureKHR(commandBuffer, pInfo);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetAccelerationStructureDeviceAddressKHR(VkDevice device, const VkAccelerationStructureDeviceAddressInfoKHR* pInfo) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetAccelerationStructureDeviceAddressKHR(device, pInfo);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetAccelerationStructureDeviceAddressKHR(VkDevice device, const VkAccelerationStructureDeviceAddressInfoKHR* pInfo) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetAccelerationStructureDeviceAddressKHR(device, pInfo);
}

void CoreChecksOptickInstrumented::PostCallRecordGetAccelerationStructureDeviceAddressKHR(VkDevice device, const VkAccelerationStructureDeviceAddressInfoKHR* pInfo, VkDeviceAddress result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetAccelerationStructureDeviceAddressKHR(device, pInfo, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdWriteAccelerationStructuresPropertiesKHR(VkCommandBuffer commandBuffer, uint32_t accelerationStructureCount, const VkAccelerationStructureKHR* pAccelerationStructures, VkQueryType queryType, VkQueryPool queryPool, uint32_t firstQuery) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdWriteAccelerationStructuresPropertiesKHR(commandBuffer, accelerationStructureCount, pAccelerationStructures, queryType, queryPool, firstQuery);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdWriteAccelerationStructuresPropertiesKHR(VkCommandBuffer commandBuffer, uint32_t accelerationStructureCount, const VkAccelerationStructureKHR* pAccelerationStructures, VkQueryType queryType, VkQueryPool queryPool, uint32_t firstQuery) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdWriteAccelerationStructuresPropertiesKHR(commandBuffer, accelerationStructureCount, pAccelerationStructures, queryType, queryPool, firstQuery);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdWriteAccelerationStructuresPropertiesKHR(VkCommandBuffer commandBuffer, uint32_t accelerationStructureCount, const VkAccelerationStructureKHR* pAccelerationStructures, VkQueryType queryType, VkQueryPool queryPool, uint32_t firstQuery) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdWriteAccelerationStructuresPropertiesKHR(commandBuffer, accelerationStructureCount, pAccelerationStructures, queryType, queryPool, firstQuery);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetDeviceAccelerationStructureCompatibilityKHR(VkDevice device, const VkAccelerationStructureVersionInfoKHR* pVersionInfo, VkAccelerationStructureCompatibilityKHR* pCompatibility) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetDeviceAccelerationStructureCompatibilityKHR(device, pVersionInfo, pCompatibility);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetDeviceAccelerationStructureCompatibilityKHR(VkDevice device, const VkAccelerationStructureVersionInfoKHR* pVersionInfo, VkAccelerationStructureCompatibilityKHR* pCompatibility) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetDeviceAccelerationStructureCompatibilityKHR(device, pVersionInfo, pCompatibility);
}

void CoreChecksOptickInstrumented::PostCallRecordGetDeviceAccelerationStructureCompatibilityKHR(VkDevice device, const VkAccelerationStructureVersionInfoKHR* pVersionInfo, VkAccelerationStructureCompatibilityKHR* pCompatibility) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetDeviceAccelerationStructureCompatibilityKHR(device, pVersionInfo, pCompatibility);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetAccelerationStructureBuildSizesKHR(VkDevice                                            device, VkAccelerationStructureBuildTypeKHR                 buildType, const VkAccelerationStructureBuildGeometryInfoKHR*  pBuildInfo, const uint32_t*  pMaxPrimitiveCounts, VkAccelerationStructureBuildSizesInfoKHR*           pSizeInfo) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetAccelerationStructureBuildSizesKHR(device, buildType, pBuildInfo, pMaxPrimitiveCounts, pSizeInfo);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetAccelerationStructureBuildSizesKHR(VkDevice                                            device, VkAccelerationStructureBuildTypeKHR                 buildType, const VkAccelerationStructureBuildGeometryInfoKHR*  pBuildInfo, const uint32_t*  pMaxPrimitiveCounts, VkAccelerationStructureBuildSizesInfoKHR*           pSizeInfo) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetAccelerationStructureBuildSizesKHR(device, buildType, pBuildInfo, pMaxPrimitiveCounts, pSizeInfo);
}

void CoreChecksOptickInstrumented::PostCallRecordGetAccelerationStructureBuildSizesKHR(VkDevice                                            device, VkAccelerationStructureBuildTypeKHR                 buildType, const VkAccelerationStructureBuildGeometryInfoKHR*  pBuildInfo, const uint32_t*  pMaxPrimitiveCounts, VkAccelerationStructureBuildSizesInfoKHR*           pSizeInfo) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetAccelerationStructureBuildSizesKHR(device, buildType, pBuildInfo, pMaxPrimitiveCounts, pSizeInfo);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdTraceRaysKHR(VkCommandBuffer commandBuffer, const VkStridedDeviceAddressRegionKHR* pRaygenShaderBindingTable, const VkStridedDeviceAddressRegionKHR* pMissShaderBindingTable, const VkStridedDeviceAddressRegionKHR* pHitShaderBindingTable, const VkStridedDeviceAddressRegionKHR* pCallableShaderBindingTable, uint32_t width, uint32_t height, uint32_t depth) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdTraceRaysKHR(commandBuffer, pRaygenShaderBindingTable, pMissShaderBindingTable, pHitShaderBindingTable, pCallableShaderBindingTable, width, height, depth);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdTraceRaysKHR(VkCommandBuffer commandBuffer, const VkStridedDeviceAddressRegionKHR* pRaygenShaderBindingTable, const VkStridedDeviceAddressRegionKHR* pMissShaderBindingTable, const VkStridedDeviceAddressRegionKHR* pHitShaderBindingTable, const VkStridedDeviceAddressRegionKHR* pCallableShaderBindingTable, uint32_t width, uint32_t height, uint32_t depth) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdTraceRaysKHR(commandBuffer, pRaygenShaderBindingTable, pMissShaderBindingTable, pHitShaderBindingTable, pCallableShaderBindingTable, width, height, depth);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdTraceRaysKHR(VkCommandBuffer commandBuffer, const VkStridedDeviceAddressRegionKHR* pRaygenShaderBindingTable, const VkStridedDeviceAddressRegionKHR* pMissShaderBindingTable, const VkStridedDeviceAddressRegionKHR* pHitShaderBindingTable, const VkStridedDeviceAddressRegionKHR* pCallableShaderBindingTable, uint32_t width, uint32_t height, uint32_t depth) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdTraceRaysKHR(commandBuffer, pRaygenShaderBindingTable, pMissShaderBindingTable, pHitShaderBindingTable, pCallableShaderBindingTable, width, height, depth);
}

bool CoreChecksOptickInstrumented::PreCallValidateCreateRayTracingPipelinesKHR(VkDevice device, VkDeferredOperationKHR deferredOperation, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkRayTracingPipelineCreateInfoKHR* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines, void* extra_data) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCreateRayTracingPipelinesKHR(device, deferredOperation, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines, extra_data);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCreateRayTracingPipelinesKHR(VkDevice device, VkDeferredOperationKHR deferredOperation, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkRayTracingPipelineCreateInfoKHR* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines, void* extra_data) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCreateRayTracingPipelinesKHR(device, deferredOperation, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines, extra_data);
}

void CoreChecksOptickInstrumented::PostCallRecordCreateRayTracingPipelinesKHR(VkDevice device, VkDeferredOperationKHR deferredOperation, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkRayTracingPipelineCreateInfoKHR* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines, VkResult result, void* extra_data) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCreateRayTracingPipelinesKHR(device, deferredOperation, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines, result, extra_data);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetRayTracingCaptureReplayShaderGroupHandlesKHR(VkDevice device, VkPipeline pipeline, uint32_t firstGroup, uint32_t groupCount, size_t dataSize, void* pData) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetRayTracingCaptureReplayShaderGroupHandlesKHR(device, pipeline, firstGroup, groupCount, dataSize, pData);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetRayTracingCaptureReplayShaderGroupHandlesKHR(VkDevice device, VkPipeline pipeline, uint32_t firstGroup, uint32_t groupCount, size_t dataSize, void* pData) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetRayTracingCaptureReplayShaderGroupHandlesKHR(device, pipeline, firstGroup, groupCount, dataSize, pData);
}

void CoreChecksOptickInstrumented::PostCallRecordGetRayTracingCaptureReplayShaderGroupHandlesKHR(VkDevice device, VkPipeline pipeline, uint32_t firstGroup, uint32_t groupCount, size_t dataSize, void* pData, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetRayTracingCaptureReplayShaderGroupHandlesKHR(device, pipeline, firstGroup, groupCount, dataSize, pData, result);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdTraceRaysIndirectKHR(VkCommandBuffer commandBuffer, const VkStridedDeviceAddressRegionKHR* pRaygenShaderBindingTable, const VkStridedDeviceAddressRegionKHR* pMissShaderBindingTable, const VkStridedDeviceAddressRegionKHR* pHitShaderBindingTable, const VkStridedDeviceAddressRegionKHR* pCallableShaderBindingTable, VkDeviceAddress indirectDeviceAddress) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdTraceRaysIndirectKHR(commandBuffer, pRaygenShaderBindingTable, pMissShaderBindingTable, pHitShaderBindingTable, pCallableShaderBindingTable, indirectDeviceAddress);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdTraceRaysIndirectKHR(VkCommandBuffer commandBuffer, const VkStridedDeviceAddressRegionKHR* pRaygenShaderBindingTable, const VkStridedDeviceAddressRegionKHR* pMissShaderBindingTable, const VkStridedDeviceAddressRegionKHR* pHitShaderBindingTable, const VkStridedDeviceAddressRegionKHR* pCallableShaderBindingTable, VkDeviceAddress indirectDeviceAddress) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdTraceRaysIndirectKHR(commandBuffer, pRaygenShaderBindingTable, pMissShaderBindingTable, pHitShaderBindingTable, pCallableShaderBindingTable, indirectDeviceAddress);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdTraceRaysIndirectKHR(VkCommandBuffer commandBuffer, const VkStridedDeviceAddressRegionKHR* pRaygenShaderBindingTable, const VkStridedDeviceAddressRegionKHR* pMissShaderBindingTable, const VkStridedDeviceAddressRegionKHR* pHitShaderBindingTable, const VkStridedDeviceAddressRegionKHR* pCallableShaderBindingTable, VkDeviceAddress indirectDeviceAddress) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdTraceRaysIndirectKHR(commandBuffer, pRaygenShaderBindingTable, pMissShaderBindingTable, pHitShaderBindingTable, pCallableShaderBindingTable, indirectDeviceAddress);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetRayTracingShaderGroupStackSizeKHR(VkDevice device, VkPipeline pipeline, uint32_t group, VkShaderGroupShaderKHR groupShader) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetRayTracingShaderGroupStackSizeKHR(device, pipeline, group, groupShader);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetRayTracingShaderGroupStackSizeKHR(VkDevice device, VkPipeline pipeline, uint32_t group, VkShaderGroupShaderKHR groupShader) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetRayTracingShaderGroupStackSizeKHR(device, pipeline, group, groupShader);
}

void CoreChecksOptickInstrumented::PostCallRecordGetRayTracingShaderGroupStackSizeKHR(VkDevice device, VkPipeline pipeline, uint32_t group, VkShaderGroupShaderKHR groupShader) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetRayTracingShaderGroupStackSizeKHR(device, pipeline, group, groupShader);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdSetRayTracingPipelineStackSizeKHR(VkCommandBuffer commandBuffer, uint32_t pipelineStackSize) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdSetRayTracingPipelineStackSizeKHR(commandBuffer, pipelineStackSize);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdSetRayTracingPipelineStackSizeKHR(VkCommandBuffer commandBuffer, uint32_t pipelineStackSize) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdSetRayTracingPipelineStackSizeKHR(commandBuffer, pipelineStackSize);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdSetRayTracingPipelineStackSizeKHR(VkCommandBuffer commandBuffer, uint32_t pipelineStackSize) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdSetRayTracingPipelineStackSizeKHR(commandBuffer, pipelineStackSize);
}

#endif // INSTRUMENT_OPTICK
