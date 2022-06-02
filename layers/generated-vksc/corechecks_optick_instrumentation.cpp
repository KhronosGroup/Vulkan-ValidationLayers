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

bool CoreChecksOptickInstrumented::PreCallValidateGetCommandPoolMemoryConsumption(VkDevice device, VkCommandPool commandPool, VkCommandBuffer commandBuffer, VkCommandPoolMemoryConsumption* pConsumption) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetCommandPoolMemoryConsumption(device, commandPool, commandBuffer, pConsumption);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetCommandPoolMemoryConsumption(VkDevice device, VkCommandPool commandPool, VkCommandBuffer commandBuffer, VkCommandPoolMemoryConsumption* pConsumption) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetCommandPoolMemoryConsumption(device, commandPool, commandBuffer, pConsumption);
}

void CoreChecksOptickInstrumented::PostCallRecordGetCommandPoolMemoryConsumption(VkDevice device, VkCommandPool commandPool, VkCommandBuffer commandBuffer, VkCommandPoolMemoryConsumption* pConsumption) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetCommandPoolMemoryConsumption(device, commandPool, commandBuffer, pConsumption);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetFaultData(VkDevice device, VkFaultQueryBehavior faultQueryBehavior, VkBool32* pUnrecordedFaults, uint32_t* pFaultCount, VkFaultData* pFaults) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetFaultData(device, faultQueryBehavior, pUnrecordedFaults, pFaultCount, pFaults);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetFaultData(VkDevice device, VkFaultQueryBehavior faultQueryBehavior, VkBool32* pUnrecordedFaults, uint32_t* pFaultCount, VkFaultData* pFaults) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetFaultData(device, faultQueryBehavior, pUnrecordedFaults, pFaultCount, pFaults);
}

void CoreChecksOptickInstrumented::PostCallRecordGetFaultData(VkDevice device, VkFaultQueryBehavior faultQueryBehavior, VkBool32* pUnrecordedFaults, uint32_t* pFaultCount, VkFaultData* pFaults, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetFaultData(device, faultQueryBehavior, pUnrecordedFaults, pFaultCount, pFaults, result);
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

bool CoreChecksOptickInstrumented::PreCallValidateCmdRefreshObjectsKHR(VkCommandBuffer commandBuffer, const VkRefreshObjectListKHR* pRefreshObjects) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdRefreshObjectsKHR(commandBuffer, pRefreshObjects);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdRefreshObjectsKHR(VkCommandBuffer commandBuffer, const VkRefreshObjectListKHR* pRefreshObjects) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdRefreshObjectsKHR(commandBuffer, pRefreshObjects);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdRefreshObjectsKHR(VkCommandBuffer commandBuffer, const VkRefreshObjectListKHR* pRefreshObjects) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdRefreshObjectsKHR(commandBuffer, pRefreshObjects);
}

bool CoreChecksOptickInstrumented::PreCallValidateGetPhysicalDeviceRefreshableObjectTypesKHR(VkPhysicalDevice physicalDevice, uint32_t* pRefreshableObjectTypeCount, VkObjectType* pRefreshableObjectTypes) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetPhysicalDeviceRefreshableObjectTypesKHR(physicalDevice, pRefreshableObjectTypeCount, pRefreshableObjectTypes);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetPhysicalDeviceRefreshableObjectTypesKHR(VkPhysicalDevice physicalDevice, uint32_t* pRefreshableObjectTypeCount, VkObjectType* pRefreshableObjectTypes) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetPhysicalDeviceRefreshableObjectTypesKHR(physicalDevice, pRefreshableObjectTypeCount, pRefreshableObjectTypes);
}

void CoreChecksOptickInstrumented::PostCallRecordGetPhysicalDeviceRefreshableObjectTypesKHR(VkPhysicalDevice physicalDevice, uint32_t* pRefreshableObjectTypeCount, VkObjectType* pRefreshableObjectTypes, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetPhysicalDeviceRefreshableObjectTypesKHR(physicalDevice, pRefreshableObjectTypeCount, pRefreshableObjectTypes, result);
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

bool CoreChecksOptickInstrumented::PreCallValidateCmdResetEvent2KHR(VkCommandBuffer                   commandBuffer, VkEvent                                             event, VkPipelineStageFlags2KHR            stageMask) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdResetEvent2KHR(commandBuffer, event, stageMask);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdResetEvent2KHR(VkCommandBuffer                   commandBuffer, VkEvent                                             event, VkPipelineStageFlags2KHR            stageMask) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdResetEvent2KHR(commandBuffer, event, stageMask);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdResetEvent2KHR(VkCommandBuffer                   commandBuffer, VkEvent                                             event, VkPipelineStageFlags2KHR            stageMask) {
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

bool CoreChecksOptickInstrumented::PreCallValidateCmdWriteTimestamp2KHR(VkCommandBuffer                   commandBuffer, VkPipelineStageFlags2KHR            stage, VkQueryPool                                         queryPool, uint32_t                                            query) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdWriteTimestamp2KHR(commandBuffer, stage, queryPool, query);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdWriteTimestamp2KHR(VkCommandBuffer                   commandBuffer, VkPipelineStageFlags2KHR            stage, VkQueryPool                                         queryPool, uint32_t                                            query) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdWriteTimestamp2KHR(commandBuffer, stage, queryPool, query);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdWriteTimestamp2KHR(VkCommandBuffer                   commandBuffer, VkPipelineStageFlags2KHR            stage, VkQueryPool                                         queryPool, uint32_t                                            query) {
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

bool CoreChecksOptickInstrumented::PreCallValidateCmdWriteBufferMarker2AMD(VkCommandBuffer                   commandBuffer, VkPipelineStageFlags2KHR            stage, VkBuffer                                            dstBuffer, VkDeviceSize                                        dstOffset, uint32_t                                            marker) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdWriteBufferMarker2AMD(commandBuffer, stage, dstBuffer, dstOffset, marker);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdWriteBufferMarker2AMD(VkCommandBuffer                   commandBuffer, VkPipelineStageFlags2KHR            stage, VkBuffer                                            dstBuffer, VkDeviceSize                                        dstOffset, uint32_t                                            marker) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdWriteBufferMarker2AMD(commandBuffer, stage, dstBuffer, dstOffset, marker);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdWriteBufferMarker2AMD(VkCommandBuffer                   commandBuffer, VkPipelineStageFlags2KHR            stage, VkBuffer                                            dstBuffer, VkDeviceSize                                        dstOffset, uint32_t                                            marker) {
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

bool CoreChecksOptickInstrumented::PreCallValidateCmdSetVertexInputEXT(VkCommandBuffer commandBuffer, uint32_t vertexBindingDescriptionCount, const VkVertexInputBindingDescription2EXT* pVertexBindingDescriptions, uint32_t vertexAttributeDescriptionCount, const VkVertexInputAttributeDescription2EXT* pVertexAttributeDescriptions) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdSetVertexInputEXT(commandBuffer, vertexBindingDescriptionCount, pVertexBindingDescriptions, vertexAttributeDescriptionCount, pVertexAttributeDescriptions);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdSetVertexInputEXT(VkCommandBuffer commandBuffer, uint32_t vertexBindingDescriptionCount, const VkVertexInputBindingDescription2EXT* pVertexBindingDescriptions, uint32_t vertexAttributeDescriptionCount, const VkVertexInputAttributeDescription2EXT* pVertexAttributeDescriptions) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdSetVertexInputEXT(commandBuffer, vertexBindingDescriptionCount, pVertexBindingDescriptions, vertexAttributeDescriptionCount, pVertexAttributeDescriptions);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdSetVertexInputEXT(VkCommandBuffer commandBuffer, uint32_t vertexBindingDescriptionCount, const VkVertexInputBindingDescription2EXT* pVertexBindingDescriptions, uint32_t vertexAttributeDescriptionCount, const VkVertexInputAttributeDescription2EXT* pVertexAttributeDescriptions) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdSetVertexInputEXT(commandBuffer, vertexBindingDescriptionCount, pVertexBindingDescriptions, vertexAttributeDescriptionCount, pVertexAttributeDescriptions);
}

#ifdef VK_USE_PLATFORM_SCI
bool CoreChecksOptickInstrumented::PreCallValidateGetFenceSciSyncFenceNV(VkDevice device, const VkFenceGetSciSyncInfoNV* pGetSciSyncHandleInfo, void* pHandle) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetFenceSciSyncFenceNV(device, pGetSciSyncHandleInfo, pHandle);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetFenceSciSyncFenceNV(VkDevice device, const VkFenceGetSciSyncInfoNV* pGetSciSyncHandleInfo, void* pHandle) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetFenceSciSyncFenceNV(device, pGetSciSyncHandleInfo, pHandle);
}

void CoreChecksOptickInstrumented::PostCallRecordGetFenceSciSyncFenceNV(VkDevice device, const VkFenceGetSciSyncInfoNV* pGetSciSyncHandleInfo, void* pHandle, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetFenceSciSyncFenceNV(device, pGetSciSyncHandleInfo, pHandle, result);
}

#endif // VK_USE_PLATFORM_SCI
#ifdef VK_USE_PLATFORM_SCI
bool CoreChecksOptickInstrumented::PreCallValidateGetFenceSciSyncObjNV(VkDevice device, const VkFenceGetSciSyncInfoNV* pGetSciSyncHandleInfo, void* pHandle) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetFenceSciSyncObjNV(device, pGetSciSyncHandleInfo, pHandle);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetFenceSciSyncObjNV(VkDevice device, const VkFenceGetSciSyncInfoNV* pGetSciSyncHandleInfo, void* pHandle) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetFenceSciSyncObjNV(device, pGetSciSyncHandleInfo, pHandle);
}

void CoreChecksOptickInstrumented::PostCallRecordGetFenceSciSyncObjNV(VkDevice device, const VkFenceGetSciSyncInfoNV* pGetSciSyncHandleInfo, void* pHandle, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetFenceSciSyncObjNV(device, pGetSciSyncHandleInfo, pHandle, result);
}

#endif // VK_USE_PLATFORM_SCI
#ifdef VK_USE_PLATFORM_SCI
bool CoreChecksOptickInstrumented::PreCallValidateImportFenceSciSyncFenceNV(VkDevice device, const VkImportFenceSciSyncInfoNV* pImportFenceSciSyncInfo) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateImportFenceSciSyncFenceNV(device, pImportFenceSciSyncInfo);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordImportFenceSciSyncFenceNV(VkDevice device, const VkImportFenceSciSyncInfoNV* pImportFenceSciSyncInfo) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordImportFenceSciSyncFenceNV(device, pImportFenceSciSyncInfo);
}

void CoreChecksOptickInstrumented::PostCallRecordImportFenceSciSyncFenceNV(VkDevice device, const VkImportFenceSciSyncInfoNV* pImportFenceSciSyncInfo, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordImportFenceSciSyncFenceNV(device, pImportFenceSciSyncInfo, result);
}

#endif // VK_USE_PLATFORM_SCI
#ifdef VK_USE_PLATFORM_SCI
bool CoreChecksOptickInstrumented::PreCallValidateImportFenceSciSyncObjNV(VkDevice device, const VkImportFenceSciSyncInfoNV* pImportFenceSciSyncInfo) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateImportFenceSciSyncObjNV(device, pImportFenceSciSyncInfo);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordImportFenceSciSyncObjNV(VkDevice device, const VkImportFenceSciSyncInfoNV* pImportFenceSciSyncInfo) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordImportFenceSciSyncObjNV(device, pImportFenceSciSyncInfo);
}

void CoreChecksOptickInstrumented::PostCallRecordImportFenceSciSyncObjNV(VkDevice device, const VkImportFenceSciSyncInfoNV* pImportFenceSciSyncInfo, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordImportFenceSciSyncObjNV(device, pImportFenceSciSyncInfo, result);
}

#endif // VK_USE_PLATFORM_SCI
#ifdef VK_USE_PLATFORM_SCI
bool CoreChecksOptickInstrumented::PreCallValidateGetPhysicalDeviceSciSyncAttributesNV(VkPhysicalDevice physicalDevice, const VkSciSyncAttributesInfoNV* pSciSyncAttributesInfo, NvSciSyncAttrList pAttributes) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetPhysicalDeviceSciSyncAttributesNV(physicalDevice, pSciSyncAttributesInfo, pAttributes);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetPhysicalDeviceSciSyncAttributesNV(VkPhysicalDevice physicalDevice, const VkSciSyncAttributesInfoNV* pSciSyncAttributesInfo, NvSciSyncAttrList pAttributes) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetPhysicalDeviceSciSyncAttributesNV(physicalDevice, pSciSyncAttributesInfo, pAttributes);
}

void CoreChecksOptickInstrumented::PostCallRecordGetPhysicalDeviceSciSyncAttributesNV(VkPhysicalDevice physicalDevice, const VkSciSyncAttributesInfoNV* pSciSyncAttributesInfo, NvSciSyncAttrList pAttributes, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetPhysicalDeviceSciSyncAttributesNV(physicalDevice, pSciSyncAttributesInfo, pAttributes, result);
}

#endif // VK_USE_PLATFORM_SCI
#ifdef VK_USE_PLATFORM_SCI
bool CoreChecksOptickInstrumented::PreCallValidateGetSemaphoreSciSyncObjNV(VkDevice device, const VkSemaphoreGetSciSyncInfoNV* pGetSciSyncInfo, void* pHandle) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetSemaphoreSciSyncObjNV(device, pGetSciSyncInfo, pHandle);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetSemaphoreSciSyncObjNV(VkDevice device, const VkSemaphoreGetSciSyncInfoNV* pGetSciSyncInfo, void* pHandle) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetSemaphoreSciSyncObjNV(device, pGetSciSyncInfo, pHandle);
}

void CoreChecksOptickInstrumented::PostCallRecordGetSemaphoreSciSyncObjNV(VkDevice device, const VkSemaphoreGetSciSyncInfoNV* pGetSciSyncInfo, void* pHandle, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetSemaphoreSciSyncObjNV(device, pGetSciSyncInfo, pHandle, result);
}

#endif // VK_USE_PLATFORM_SCI
#ifdef VK_USE_PLATFORM_SCI
bool CoreChecksOptickInstrumented::PreCallValidateImportSemaphoreSciSyncObjNV(VkDevice device, const VkImportSemaphoreSciSyncInfoNV* pImportSemaphoreSciSyncInfo) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateImportSemaphoreSciSyncObjNV(device, pImportSemaphoreSciSyncInfo);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordImportSemaphoreSciSyncObjNV(VkDevice device, const VkImportSemaphoreSciSyncInfoNV* pImportSemaphoreSciSyncInfo) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordImportSemaphoreSciSyncObjNV(device, pImportSemaphoreSciSyncInfo);
}

void CoreChecksOptickInstrumented::PostCallRecordImportSemaphoreSciSyncObjNV(VkDevice device, const VkImportSemaphoreSciSyncInfoNV* pImportSemaphoreSciSyncInfo, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordImportSemaphoreSciSyncObjNV(device, pImportSemaphoreSciSyncInfo, result);
}

#endif // VK_USE_PLATFORM_SCI
#ifdef VK_USE_PLATFORM_SCI
bool CoreChecksOptickInstrumented::PreCallValidateGetMemorySciBufNV(VkDevice device, const VkMemoryGetSciBufInfoNV* pGetSciBufInfo, NvSciBufObj* pHandle) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetMemorySciBufNV(device, pGetSciBufInfo, pHandle);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetMemorySciBufNV(VkDevice device, const VkMemoryGetSciBufInfoNV* pGetSciBufInfo, NvSciBufObj* pHandle) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetMemorySciBufNV(device, pGetSciBufInfo, pHandle);
}

void CoreChecksOptickInstrumented::PostCallRecordGetMemorySciBufNV(VkDevice device, const VkMemoryGetSciBufInfoNV* pGetSciBufInfo, NvSciBufObj* pHandle, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetMemorySciBufNV(device, pGetSciBufInfo, pHandle, result);
}

#endif // VK_USE_PLATFORM_SCI
#ifdef VK_USE_PLATFORM_SCI
bool CoreChecksOptickInstrumented::PreCallValidateGetPhysicalDeviceExternalMemorySciBufPropertiesNV(VkPhysicalDevice physicalDevice, VkExternalMemoryHandleTypeFlagBits handleType, NvSciBufObj handle, VkMemorySciBufPropertiesNV* pMemorySciBufProperties) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetPhysicalDeviceExternalMemorySciBufPropertiesNV(physicalDevice, handleType, handle, pMemorySciBufProperties);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetPhysicalDeviceExternalMemorySciBufPropertiesNV(VkPhysicalDevice physicalDevice, VkExternalMemoryHandleTypeFlagBits handleType, NvSciBufObj handle, VkMemorySciBufPropertiesNV* pMemorySciBufProperties) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetPhysicalDeviceExternalMemorySciBufPropertiesNV(physicalDevice, handleType, handle, pMemorySciBufProperties);
}

void CoreChecksOptickInstrumented::PostCallRecordGetPhysicalDeviceExternalMemorySciBufPropertiesNV(VkPhysicalDevice physicalDevice, VkExternalMemoryHandleTypeFlagBits handleType, NvSciBufObj handle, VkMemorySciBufPropertiesNV* pMemorySciBufProperties, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetPhysicalDeviceExternalMemorySciBufPropertiesNV(physicalDevice, handleType, handle, pMemorySciBufProperties, result);
}

#endif // VK_USE_PLATFORM_SCI
#ifdef VK_USE_PLATFORM_SCI
bool CoreChecksOptickInstrumented::PreCallValidateGetPhysicalDeviceSciBufAttributesNV(VkPhysicalDevice physicalDevice, NvSciBufAttrList pAttributes) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateGetPhysicalDeviceSciBufAttributesNV(physicalDevice, pAttributes);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordGetPhysicalDeviceSciBufAttributesNV(VkPhysicalDevice physicalDevice, NvSciBufAttrList pAttributes) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordGetPhysicalDeviceSciBufAttributesNV(physicalDevice, pAttributes);
}

void CoreChecksOptickInstrumented::PostCallRecordGetPhysicalDeviceSciBufAttributesNV(VkPhysicalDevice physicalDevice, NvSciBufAttrList pAttributes, VkResult result) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordGetPhysicalDeviceSciBufAttributesNV(physicalDevice, pAttributes, result);
}

#endif // VK_USE_PLATFORM_SCI
bool CoreChecksOptickInstrumented::PreCallValidateCmdSetPatchControlPointsEXT(VkCommandBuffer commandBuffer, uint32_t patchControlPoints) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdSetPatchControlPointsEXT(commandBuffer, patchControlPoints);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdSetPatchControlPointsEXT(VkCommandBuffer commandBuffer, uint32_t patchControlPoints) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdSetPatchControlPointsEXT(commandBuffer, patchControlPoints);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdSetPatchControlPointsEXT(VkCommandBuffer commandBuffer, uint32_t patchControlPoints) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdSetPatchControlPointsEXT(commandBuffer, patchControlPoints);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdSetRasterizerDiscardEnableEXT(VkCommandBuffer commandBuffer, VkBool32 rasterizerDiscardEnable) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdSetRasterizerDiscardEnableEXT(commandBuffer, rasterizerDiscardEnable);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdSetRasterizerDiscardEnableEXT(VkCommandBuffer commandBuffer, VkBool32 rasterizerDiscardEnable) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdSetRasterizerDiscardEnableEXT(commandBuffer, rasterizerDiscardEnable);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdSetRasterizerDiscardEnableEXT(VkCommandBuffer commandBuffer, VkBool32 rasterizerDiscardEnable) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdSetRasterizerDiscardEnableEXT(commandBuffer, rasterizerDiscardEnable);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdSetDepthBiasEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthBiasEnable) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdSetDepthBiasEnableEXT(commandBuffer, depthBiasEnable);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdSetDepthBiasEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthBiasEnable) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdSetDepthBiasEnableEXT(commandBuffer, depthBiasEnable);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdSetDepthBiasEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthBiasEnable) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdSetDepthBiasEnableEXT(commandBuffer, depthBiasEnable);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdSetLogicOpEXT(VkCommandBuffer commandBuffer, VkLogicOp logicOp) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdSetLogicOpEXT(commandBuffer, logicOp);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdSetLogicOpEXT(VkCommandBuffer commandBuffer, VkLogicOp logicOp) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdSetLogicOpEXT(commandBuffer, logicOp);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdSetLogicOpEXT(VkCommandBuffer commandBuffer, VkLogicOp logicOp) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdSetLogicOpEXT(commandBuffer, logicOp);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdSetPrimitiveRestartEnableEXT(VkCommandBuffer commandBuffer, VkBool32 primitiveRestartEnable) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdSetPrimitiveRestartEnableEXT(commandBuffer, primitiveRestartEnable);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdSetPrimitiveRestartEnableEXT(VkCommandBuffer commandBuffer, VkBool32 primitiveRestartEnable) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdSetPrimitiveRestartEnableEXT(commandBuffer, primitiveRestartEnable);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdSetPrimitiveRestartEnableEXT(VkCommandBuffer commandBuffer, VkBool32 primitiveRestartEnable) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdSetPrimitiveRestartEnableEXT(commandBuffer, primitiveRestartEnable);
}

bool CoreChecksOptickInstrumented::PreCallValidateCmdSetColorWriteEnableEXT(VkCommandBuffer       commandBuffer, uint32_t                                attachmentCount, const VkBool32*   pColorWriteEnables) const {
    OPTICK_EVENT();
    auto result = CoreChecks::PreCallValidateCmdSetColorWriteEnableEXT(commandBuffer, attachmentCount, pColorWriteEnables);
    return result;
}

void CoreChecksOptickInstrumented::PreCallRecordCmdSetColorWriteEnableEXT(VkCommandBuffer       commandBuffer, uint32_t                                attachmentCount, const VkBool32*   pColorWriteEnables) {
    OPTICK_EVENT();
    CoreChecks::PreCallRecordCmdSetColorWriteEnableEXT(commandBuffer, attachmentCount, pColorWriteEnables);
}

void CoreChecksOptickInstrumented::PostCallRecordCmdSetColorWriteEnableEXT(VkCommandBuffer       commandBuffer, uint32_t                                attachmentCount, const VkBool32*   pColorWriteEnables) {
    OPTICK_EVENT();
    CoreChecks::PostCallRecordCmdSetColorWriteEnableEXT(commandBuffer, attachmentCount, pColorWriteEnables);
}

#endif // INSTRUMENT_OPTICK
