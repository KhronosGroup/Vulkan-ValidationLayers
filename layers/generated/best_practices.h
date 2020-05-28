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


#include "chassis.h"
#include "best_practices_chassis.h"
#include "best_practices_validation.h"


class BestPractices : public BestPracticesAPICallHookInterface, public BestPracticesTracker {
public:

virtual bool PreCallValidateCreateInstance(
    const VkInstanceCreateInfo*                 pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkInstance*                                 pInstance) const override;


virtual void PreCallRecordCreateInstance(
    const VkInstanceCreateInfo*                 pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkInstance*                                 pInstance) override;


virtual void PostCallRecordCreateInstance(
    const VkInstanceCreateInfo*                 pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkInstance*                                 pInstance,
    VkResult                                    result) override;


virtual bool PreCallValidateDestroyInstance(
    VkInstance                                  instance,
    const VkAllocationCallbacks*                pAllocator) const override;


virtual void PreCallRecordDestroyInstance(
    VkInstance                                  instance,
    const VkAllocationCallbacks*                pAllocator) override;


virtual void PostCallRecordDestroyInstance(
    VkInstance                                  instance,
    const VkAllocationCallbacks*                pAllocator) override;


virtual bool PreCallValidateEnumeratePhysicalDevices(
    VkInstance                                  instance,
    uint32_t*                                   pPhysicalDeviceCount,
    VkPhysicalDevice*                           pPhysicalDevices) const override;


virtual void PreCallRecordEnumeratePhysicalDevices(
    VkInstance                                  instance,
    uint32_t*                                   pPhysicalDeviceCount,
    VkPhysicalDevice*                           pPhysicalDevices) override;


virtual void PostCallRecordEnumeratePhysicalDevices(
    VkInstance                                  instance,
    uint32_t*                                   pPhysicalDeviceCount,
    VkPhysicalDevice*                           pPhysicalDevices,
    VkResult                                    result) override;


virtual bool PreCallValidateGetPhysicalDeviceFeatures(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceFeatures*                   pFeatures) const override;


virtual void PreCallRecordGetPhysicalDeviceFeatures(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceFeatures*                   pFeatures) override;


virtual void PostCallRecordGetPhysicalDeviceFeatures(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceFeatures*                   pFeatures) override;


virtual bool PreCallValidateGetPhysicalDeviceFormatProperties(
    VkPhysicalDevice                            physicalDevice,
    VkFormat                                    format,
    VkFormatProperties*                         pFormatProperties) const override;


virtual void PreCallRecordGetPhysicalDeviceFormatProperties(
    VkPhysicalDevice                            physicalDevice,
    VkFormat                                    format,
    VkFormatProperties*                         pFormatProperties) override;


virtual void PostCallRecordGetPhysicalDeviceFormatProperties(
    VkPhysicalDevice                            physicalDevice,
    VkFormat                                    format,
    VkFormatProperties*                         pFormatProperties) override;


virtual bool PreCallValidateGetPhysicalDeviceImageFormatProperties(
    VkPhysicalDevice                            physicalDevice,
    VkFormat                                    format,
    VkImageType                                 type,
    VkImageTiling                               tiling,
    VkImageUsageFlags                           usage,
    VkImageCreateFlags                          flags,
    VkImageFormatProperties*                    pImageFormatProperties) const override;


virtual void PreCallRecordGetPhysicalDeviceImageFormatProperties(
    VkPhysicalDevice                            physicalDevice,
    VkFormat                                    format,
    VkImageType                                 type,
    VkImageTiling                               tiling,
    VkImageUsageFlags                           usage,
    VkImageCreateFlags                          flags,
    VkImageFormatProperties*                    pImageFormatProperties) override;


virtual void PostCallRecordGetPhysicalDeviceImageFormatProperties(
    VkPhysicalDevice                            physicalDevice,
    VkFormat                                    format,
    VkImageType                                 type,
    VkImageTiling                               tiling,
    VkImageUsageFlags                           usage,
    VkImageCreateFlags                          flags,
    VkImageFormatProperties*                    pImageFormatProperties,
    VkResult                                    result) override;


virtual bool PreCallValidateGetPhysicalDeviceProperties(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceProperties*                 pProperties) const override;


virtual void PreCallRecordGetPhysicalDeviceProperties(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceProperties*                 pProperties) override;


virtual void PostCallRecordGetPhysicalDeviceProperties(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceProperties*                 pProperties) override;


virtual bool PreCallValidateGetPhysicalDeviceQueueFamilyProperties(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pQueueFamilyPropertyCount,
    VkQueueFamilyProperties*                    pQueueFamilyProperties) const override;


virtual void PreCallRecordGetPhysicalDeviceQueueFamilyProperties(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pQueueFamilyPropertyCount,
    VkQueueFamilyProperties*                    pQueueFamilyProperties) override;


virtual void PostCallRecordGetPhysicalDeviceQueueFamilyProperties(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pQueueFamilyPropertyCount,
    VkQueueFamilyProperties*                    pQueueFamilyProperties) override;


virtual bool PreCallValidateGetPhysicalDeviceMemoryProperties(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceMemoryProperties*           pMemoryProperties) const override;


virtual void PreCallRecordGetPhysicalDeviceMemoryProperties(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceMemoryProperties*           pMemoryProperties) override;


virtual void PostCallRecordGetPhysicalDeviceMemoryProperties(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceMemoryProperties*           pMemoryProperties) override;


virtual bool PreCallValidateGetInstanceProcAddr(
    VkInstance                                  instance,
    const char*                                 pName) const override;


virtual void PreCallRecordGetInstanceProcAddr(
    VkInstance                                  instance,
    const char*                                 pName) override;


virtual void PostCallRecordGetInstanceProcAddr(
    VkInstance                                  instance,
    const char*                                 pName) override;


virtual bool PreCallValidateGetDeviceProcAddr(
    VkDevice                                    device,
    const char*                                 pName) const override;


virtual void PreCallRecordGetDeviceProcAddr(
    VkDevice                                    device,
    const char*                                 pName) override;


virtual void PostCallRecordGetDeviceProcAddr(
    VkDevice                                    device,
    const char*                                 pName) override;


virtual bool PreCallValidateCreateDevice(
    VkPhysicalDevice                            physicalDevice,
    const VkDeviceCreateInfo*                   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDevice*                                   pDevice) const override;


virtual void PreCallRecordCreateDevice(
    VkPhysicalDevice                            physicalDevice,
    const VkDeviceCreateInfo*                   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDevice*                                   pDevice) override;


virtual void PostCallRecordCreateDevice(
    VkPhysicalDevice                            physicalDevice,
    const VkDeviceCreateInfo*                   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDevice*                                   pDevice,
    VkResult                                    result) override;


virtual bool PreCallValidateDestroyDevice(
    VkDevice                                    device,
    const VkAllocationCallbacks*                pAllocator) const override;


virtual void PreCallRecordDestroyDevice(
    VkDevice                                    device,
    const VkAllocationCallbacks*                pAllocator) override;


virtual void PostCallRecordDestroyDevice(
    VkDevice                                    device,
    const VkAllocationCallbacks*                pAllocator) override;


virtual bool PreCallValidateEnumerateInstanceExtensionProperties(
    const char*                                 pLayerName,
    uint32_t*                                   pPropertyCount,
    VkExtensionProperties*                      pProperties) const override;


virtual void PreCallRecordEnumerateInstanceExtensionProperties(
    const char*                                 pLayerName,
    uint32_t*                                   pPropertyCount,
    VkExtensionProperties*                      pProperties) override;


virtual void PostCallRecordEnumerateInstanceExtensionProperties(
    const char*                                 pLayerName,
    uint32_t*                                   pPropertyCount,
    VkExtensionProperties*                      pProperties,
    VkResult                                    result) override;


virtual bool PreCallValidateEnumerateDeviceExtensionProperties(
    VkPhysicalDevice                            physicalDevice,
    const char*                                 pLayerName,
    uint32_t*                                   pPropertyCount,
    VkExtensionProperties*                      pProperties) const override;


virtual void PreCallRecordEnumerateDeviceExtensionProperties(
    VkPhysicalDevice                            physicalDevice,
    const char*                                 pLayerName,
    uint32_t*                                   pPropertyCount,
    VkExtensionProperties*                      pProperties) override;


virtual void PostCallRecordEnumerateDeviceExtensionProperties(
    VkPhysicalDevice                            physicalDevice,
    const char*                                 pLayerName,
    uint32_t*                                   pPropertyCount,
    VkExtensionProperties*                      pProperties,
    VkResult                                    result) override;


virtual bool PreCallValidateEnumerateInstanceLayerProperties(
    uint32_t*                                   pPropertyCount,
    VkLayerProperties*                          pProperties) const override;


virtual void PreCallRecordEnumerateInstanceLayerProperties(
    uint32_t*                                   pPropertyCount,
    VkLayerProperties*                          pProperties) override;


virtual void PostCallRecordEnumerateInstanceLayerProperties(
    uint32_t*                                   pPropertyCount,
    VkLayerProperties*                          pProperties,
    VkResult                                    result) override;


virtual bool PreCallValidateEnumerateDeviceLayerProperties(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pPropertyCount,
    VkLayerProperties*                          pProperties) const override;


virtual void PreCallRecordEnumerateDeviceLayerProperties(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pPropertyCount,
    VkLayerProperties*                          pProperties) override;


virtual void PostCallRecordEnumerateDeviceLayerProperties(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pPropertyCount,
    VkLayerProperties*                          pProperties,
    VkResult                                    result) override;


virtual bool PreCallValidateGetDeviceQueue(
    VkDevice                                    device,
    uint32_t                                    queueFamilyIndex,
    uint32_t                                    queueIndex,
    VkQueue*                                    pQueue) const override;


virtual void PreCallRecordGetDeviceQueue(
    VkDevice                                    device,
    uint32_t                                    queueFamilyIndex,
    uint32_t                                    queueIndex,
    VkQueue*                                    pQueue) override;


virtual void PostCallRecordGetDeviceQueue(
    VkDevice                                    device,
    uint32_t                                    queueFamilyIndex,
    uint32_t                                    queueIndex,
    VkQueue*                                    pQueue) override;


virtual bool PreCallValidateQueueSubmit(
    VkQueue                                     queue,
    uint32_t                                    submitCount,
    const VkSubmitInfo*                         pSubmits,
    VkFence                                     fence) const override;


virtual void PreCallRecordQueueSubmit(
    VkQueue                                     queue,
    uint32_t                                    submitCount,
    const VkSubmitInfo*                         pSubmits,
    VkFence                                     fence) override;


virtual void PostCallRecordQueueSubmit(
    VkQueue                                     queue,
    uint32_t                                    submitCount,
    const VkSubmitInfo*                         pSubmits,
    VkFence                                     fence,
    VkResult                                    result) override;


virtual bool PreCallValidateQueueWaitIdle(
    VkQueue                                     queue) const override;


virtual void PreCallRecordQueueWaitIdle(
    VkQueue                                     queue) override;


virtual void PostCallRecordQueueWaitIdle(
    VkQueue                                     queue,
    VkResult                                    result) override;


virtual bool PreCallValidateDeviceWaitIdle(
    VkDevice                                    device) const override;


virtual void PreCallRecordDeviceWaitIdle(
    VkDevice                                    device) override;


virtual void PostCallRecordDeviceWaitIdle(
    VkDevice                                    device,
    VkResult                                    result) override;


virtual bool PreCallValidateAllocateMemory(
    VkDevice                                    device,
    const VkMemoryAllocateInfo*                 pAllocateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDeviceMemory*                             pMemory) const override;


virtual void PreCallRecordAllocateMemory(
    VkDevice                                    device,
    const VkMemoryAllocateInfo*                 pAllocateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDeviceMemory*                             pMemory) override;


virtual void PostCallRecordAllocateMemory(
    VkDevice                                    device,
    const VkMemoryAllocateInfo*                 pAllocateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDeviceMemory*                             pMemory,
    VkResult                                    result) override;


virtual bool PreCallValidateFreeMemory(
    VkDevice                                    device,
    VkDeviceMemory                              memory,
    const VkAllocationCallbacks*                pAllocator) const override;


virtual void PreCallRecordFreeMemory(
    VkDevice                                    device,
    VkDeviceMemory                              memory,
    const VkAllocationCallbacks*                pAllocator) override;


virtual void PostCallRecordFreeMemory(
    VkDevice                                    device,
    VkDeviceMemory                              memory,
    const VkAllocationCallbacks*                pAllocator) override;


virtual bool PreCallValidateMapMemory(
    VkDevice                                    device,
    VkDeviceMemory                              memory,
    VkDeviceSize                                offset,
    VkDeviceSize                                size,
    VkMemoryMapFlags                            flags,
    void**                                      ppData) const override;


virtual void PreCallRecordMapMemory(
    VkDevice                                    device,
    VkDeviceMemory                              memory,
    VkDeviceSize                                offset,
    VkDeviceSize                                size,
    VkMemoryMapFlags                            flags,
    void**                                      ppData) override;


virtual void PostCallRecordMapMemory(
    VkDevice                                    device,
    VkDeviceMemory                              memory,
    VkDeviceSize                                offset,
    VkDeviceSize                                size,
    VkMemoryMapFlags                            flags,
    void**                                      ppData,
    VkResult                                    result) override;


virtual bool PreCallValidateUnmapMemory(
    VkDevice                                    device,
    VkDeviceMemory                              memory) const override;


virtual void PreCallRecordUnmapMemory(
    VkDevice                                    device,
    VkDeviceMemory                              memory) override;


virtual void PostCallRecordUnmapMemory(
    VkDevice                                    device,
    VkDeviceMemory                              memory) override;


virtual bool PreCallValidateFlushMappedMemoryRanges(
    VkDevice                                    device,
    uint32_t                                    memoryRangeCount,
    const VkMappedMemoryRange*                  pMemoryRanges) const override;


virtual void PreCallRecordFlushMappedMemoryRanges(
    VkDevice                                    device,
    uint32_t                                    memoryRangeCount,
    const VkMappedMemoryRange*                  pMemoryRanges) override;


virtual void PostCallRecordFlushMappedMemoryRanges(
    VkDevice                                    device,
    uint32_t                                    memoryRangeCount,
    const VkMappedMemoryRange*                  pMemoryRanges,
    VkResult                                    result) override;


virtual bool PreCallValidateInvalidateMappedMemoryRanges(
    VkDevice                                    device,
    uint32_t                                    memoryRangeCount,
    const VkMappedMemoryRange*                  pMemoryRanges) const override;


virtual void PreCallRecordInvalidateMappedMemoryRanges(
    VkDevice                                    device,
    uint32_t                                    memoryRangeCount,
    const VkMappedMemoryRange*                  pMemoryRanges) override;


virtual void PostCallRecordInvalidateMappedMemoryRanges(
    VkDevice                                    device,
    uint32_t                                    memoryRangeCount,
    const VkMappedMemoryRange*                  pMemoryRanges,
    VkResult                                    result) override;


virtual bool PreCallValidateGetDeviceMemoryCommitment(
    VkDevice                                    device,
    VkDeviceMemory                              memory,
    VkDeviceSize*                               pCommittedMemoryInBytes) const override;


virtual void PreCallRecordGetDeviceMemoryCommitment(
    VkDevice                                    device,
    VkDeviceMemory                              memory,
    VkDeviceSize*                               pCommittedMemoryInBytes) override;


virtual void PostCallRecordGetDeviceMemoryCommitment(
    VkDevice                                    device,
    VkDeviceMemory                              memory,
    VkDeviceSize*                               pCommittedMemoryInBytes) override;


virtual bool PreCallValidateBindBufferMemory(
    VkDevice                                    device,
    VkBuffer                                    buffer,
    VkDeviceMemory                              memory,
    VkDeviceSize                                memoryOffset) const override;


virtual void PreCallRecordBindBufferMemory(
    VkDevice                                    device,
    VkBuffer                                    buffer,
    VkDeviceMemory                              memory,
    VkDeviceSize                                memoryOffset) override;


virtual void PostCallRecordBindBufferMemory(
    VkDevice                                    device,
    VkBuffer                                    buffer,
    VkDeviceMemory                              memory,
    VkDeviceSize                                memoryOffset,
    VkResult                                    result) override;


virtual bool PreCallValidateBindImageMemory(
    VkDevice                                    device,
    VkImage                                     image,
    VkDeviceMemory                              memory,
    VkDeviceSize                                memoryOffset) const override;


virtual void PreCallRecordBindImageMemory(
    VkDevice                                    device,
    VkImage                                     image,
    VkDeviceMemory                              memory,
    VkDeviceSize                                memoryOffset) override;


virtual void PostCallRecordBindImageMemory(
    VkDevice                                    device,
    VkImage                                     image,
    VkDeviceMemory                              memory,
    VkDeviceSize                                memoryOffset,
    VkResult                                    result) override;


virtual bool PreCallValidateGetBufferMemoryRequirements(
    VkDevice                                    device,
    VkBuffer                                    buffer,
    VkMemoryRequirements*                       pMemoryRequirements) const override;


virtual void PreCallRecordGetBufferMemoryRequirements(
    VkDevice                                    device,
    VkBuffer                                    buffer,
    VkMemoryRequirements*                       pMemoryRequirements) override;


virtual void PostCallRecordGetBufferMemoryRequirements(
    VkDevice                                    device,
    VkBuffer                                    buffer,
    VkMemoryRequirements*                       pMemoryRequirements) override;


virtual bool PreCallValidateGetImageMemoryRequirements(
    VkDevice                                    device,
    VkImage                                     image,
    VkMemoryRequirements*                       pMemoryRequirements) const override;


virtual void PreCallRecordGetImageMemoryRequirements(
    VkDevice                                    device,
    VkImage                                     image,
    VkMemoryRequirements*                       pMemoryRequirements) override;


virtual void PostCallRecordGetImageMemoryRequirements(
    VkDevice                                    device,
    VkImage                                     image,
    VkMemoryRequirements*                       pMemoryRequirements) override;


virtual bool PreCallValidateGetImageSparseMemoryRequirements(
    VkDevice                                    device,
    VkImage                                     image,
    uint32_t*                                   pSparseMemoryRequirementCount,
    VkSparseImageMemoryRequirements*            pSparseMemoryRequirements) const override;


virtual void PreCallRecordGetImageSparseMemoryRequirements(
    VkDevice                                    device,
    VkImage                                     image,
    uint32_t*                                   pSparseMemoryRequirementCount,
    VkSparseImageMemoryRequirements*            pSparseMemoryRequirements) override;


virtual void PostCallRecordGetImageSparseMemoryRequirements(
    VkDevice                                    device,
    VkImage                                     image,
    uint32_t*                                   pSparseMemoryRequirementCount,
    VkSparseImageMemoryRequirements*            pSparseMemoryRequirements) override;


virtual bool PreCallValidateGetPhysicalDeviceSparseImageFormatProperties(
    VkPhysicalDevice                            physicalDevice,
    VkFormat                                    format,
    VkImageType                                 type,
    VkSampleCountFlagBits                       samples,
    VkImageUsageFlags                           usage,
    VkImageTiling                               tiling,
    uint32_t*                                   pPropertyCount,
    VkSparseImageFormatProperties*              pProperties) const override;


virtual void PreCallRecordGetPhysicalDeviceSparseImageFormatProperties(
    VkPhysicalDevice                            physicalDevice,
    VkFormat                                    format,
    VkImageType                                 type,
    VkSampleCountFlagBits                       samples,
    VkImageUsageFlags                           usage,
    VkImageTiling                               tiling,
    uint32_t*                                   pPropertyCount,
    VkSparseImageFormatProperties*              pProperties) override;


virtual void PostCallRecordGetPhysicalDeviceSparseImageFormatProperties(
    VkPhysicalDevice                            physicalDevice,
    VkFormat                                    format,
    VkImageType                                 type,
    VkSampleCountFlagBits                       samples,
    VkImageUsageFlags                           usage,
    VkImageTiling                               tiling,
    uint32_t*                                   pPropertyCount,
    VkSparseImageFormatProperties*              pProperties) override;


virtual bool PreCallValidateQueueBindSparse(
    VkQueue                                     queue,
    uint32_t                                    bindInfoCount,
    const VkBindSparseInfo*                     pBindInfo,
    VkFence                                     fence) const override;


virtual void PreCallRecordQueueBindSparse(
    VkQueue                                     queue,
    uint32_t                                    bindInfoCount,
    const VkBindSparseInfo*                     pBindInfo,
    VkFence                                     fence) override;


virtual void PostCallRecordQueueBindSparse(
    VkQueue                                     queue,
    uint32_t                                    bindInfoCount,
    const VkBindSparseInfo*                     pBindInfo,
    VkFence                                     fence,
    VkResult                                    result) override;


virtual bool PreCallValidateCreateFence(
    VkDevice                                    device,
    const VkFenceCreateInfo*                    pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkFence*                                    pFence) const override;


virtual void PreCallRecordCreateFence(
    VkDevice                                    device,
    const VkFenceCreateInfo*                    pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkFence*                                    pFence) override;


virtual void PostCallRecordCreateFence(
    VkDevice                                    device,
    const VkFenceCreateInfo*                    pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkFence*                                    pFence,
    VkResult                                    result) override;


virtual bool PreCallValidateDestroyFence(
    VkDevice                                    device,
    VkFence                                     fence,
    const VkAllocationCallbacks*                pAllocator) const override;


virtual void PreCallRecordDestroyFence(
    VkDevice                                    device,
    VkFence                                     fence,
    const VkAllocationCallbacks*                pAllocator) override;


virtual void PostCallRecordDestroyFence(
    VkDevice                                    device,
    VkFence                                     fence,
    const VkAllocationCallbacks*                pAllocator) override;


virtual bool PreCallValidateResetFences(
    VkDevice                                    device,
    uint32_t                                    fenceCount,
    const VkFence*                              pFences) const override;


virtual void PreCallRecordResetFences(
    VkDevice                                    device,
    uint32_t                                    fenceCount,
    const VkFence*                              pFences) override;


virtual void PostCallRecordResetFences(
    VkDevice                                    device,
    uint32_t                                    fenceCount,
    const VkFence*                              pFences,
    VkResult                                    result) override;


virtual bool PreCallValidateGetFenceStatus(
    VkDevice                                    device,
    VkFence                                     fence) const override;


virtual void PreCallRecordGetFenceStatus(
    VkDevice                                    device,
    VkFence                                     fence) override;


virtual void PostCallRecordGetFenceStatus(
    VkDevice                                    device,
    VkFence                                     fence,
    VkResult                                    result) override;


virtual bool PreCallValidateWaitForFences(
    VkDevice                                    device,
    uint32_t                                    fenceCount,
    const VkFence*                              pFences,
    VkBool32                                    waitAll,
    uint64_t                                    timeout) const override;


virtual void PreCallRecordWaitForFences(
    VkDevice                                    device,
    uint32_t                                    fenceCount,
    const VkFence*                              pFences,
    VkBool32                                    waitAll,
    uint64_t                                    timeout) override;


virtual void PostCallRecordWaitForFences(
    VkDevice                                    device,
    uint32_t                                    fenceCount,
    const VkFence*                              pFences,
    VkBool32                                    waitAll,
    uint64_t                                    timeout,
    VkResult                                    result) override;


virtual bool PreCallValidateCreateSemaphore(
    VkDevice                                    device,
    const VkSemaphoreCreateInfo*                pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSemaphore*                                pSemaphore) const override;


virtual void PreCallRecordCreateSemaphore(
    VkDevice                                    device,
    const VkSemaphoreCreateInfo*                pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSemaphore*                                pSemaphore) override;


virtual void PostCallRecordCreateSemaphore(
    VkDevice                                    device,
    const VkSemaphoreCreateInfo*                pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSemaphore*                                pSemaphore,
    VkResult                                    result) override;


virtual bool PreCallValidateDestroySemaphore(
    VkDevice                                    device,
    VkSemaphore                                 semaphore,
    const VkAllocationCallbacks*                pAllocator) const override;


virtual void PreCallRecordDestroySemaphore(
    VkDevice                                    device,
    VkSemaphore                                 semaphore,
    const VkAllocationCallbacks*                pAllocator) override;


virtual void PostCallRecordDestroySemaphore(
    VkDevice                                    device,
    VkSemaphore                                 semaphore,
    const VkAllocationCallbacks*                pAllocator) override;


virtual bool PreCallValidateCreateEvent(
    VkDevice                                    device,
    const VkEventCreateInfo*                    pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkEvent*                                    pEvent) const override;


virtual void PreCallRecordCreateEvent(
    VkDevice                                    device,
    const VkEventCreateInfo*                    pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkEvent*                                    pEvent) override;


virtual void PostCallRecordCreateEvent(
    VkDevice                                    device,
    const VkEventCreateInfo*                    pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkEvent*                                    pEvent,
    VkResult                                    result) override;


virtual bool PreCallValidateDestroyEvent(
    VkDevice                                    device,
    VkEvent                                     event,
    const VkAllocationCallbacks*                pAllocator) const override;


virtual void PreCallRecordDestroyEvent(
    VkDevice                                    device,
    VkEvent                                     event,
    const VkAllocationCallbacks*                pAllocator) override;


virtual void PostCallRecordDestroyEvent(
    VkDevice                                    device,
    VkEvent                                     event,
    const VkAllocationCallbacks*                pAllocator) override;


virtual bool PreCallValidateGetEventStatus(
    VkDevice                                    device,
    VkEvent                                     event) const override;


virtual void PreCallRecordGetEventStatus(
    VkDevice                                    device,
    VkEvent                                     event) override;


virtual void PostCallRecordGetEventStatus(
    VkDevice                                    device,
    VkEvent                                     event,
    VkResult                                    result) override;


virtual bool PreCallValidateSetEvent(
    VkDevice                                    device,
    VkEvent                                     event) const override;


virtual void PreCallRecordSetEvent(
    VkDevice                                    device,
    VkEvent                                     event) override;


virtual void PostCallRecordSetEvent(
    VkDevice                                    device,
    VkEvent                                     event,
    VkResult                                    result) override;


virtual bool PreCallValidateResetEvent(
    VkDevice                                    device,
    VkEvent                                     event) const override;


virtual void PreCallRecordResetEvent(
    VkDevice                                    device,
    VkEvent                                     event) override;


virtual void PostCallRecordResetEvent(
    VkDevice                                    device,
    VkEvent                                     event,
    VkResult                                    result) override;


virtual bool PreCallValidateCreateQueryPool(
    VkDevice                                    device,
    const VkQueryPoolCreateInfo*                pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkQueryPool*                                pQueryPool) const override;


virtual void PreCallRecordCreateQueryPool(
    VkDevice                                    device,
    const VkQueryPoolCreateInfo*                pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkQueryPool*                                pQueryPool) override;


virtual void PostCallRecordCreateQueryPool(
    VkDevice                                    device,
    const VkQueryPoolCreateInfo*                pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkQueryPool*                                pQueryPool,
    VkResult                                    result) override;


virtual bool PreCallValidateDestroyQueryPool(
    VkDevice                                    device,
    VkQueryPool                                 queryPool,
    const VkAllocationCallbacks*                pAllocator) const override;


virtual void PreCallRecordDestroyQueryPool(
    VkDevice                                    device,
    VkQueryPool                                 queryPool,
    const VkAllocationCallbacks*                pAllocator) override;


virtual void PostCallRecordDestroyQueryPool(
    VkDevice                                    device,
    VkQueryPool                                 queryPool,
    const VkAllocationCallbacks*                pAllocator) override;


virtual bool PreCallValidateGetQueryPoolResults(
    VkDevice                                    device,
    VkQueryPool                                 queryPool,
    uint32_t                                    firstQuery,
    uint32_t                                    queryCount,
    size_t                                      dataSize,
    void*                                       pData,
    VkDeviceSize                                stride,
    VkQueryResultFlags                          flags) const override;


virtual void PreCallRecordGetQueryPoolResults(
    VkDevice                                    device,
    VkQueryPool                                 queryPool,
    uint32_t                                    firstQuery,
    uint32_t                                    queryCount,
    size_t                                      dataSize,
    void*                                       pData,
    VkDeviceSize                                stride,
    VkQueryResultFlags                          flags) override;


virtual void PostCallRecordGetQueryPoolResults(
    VkDevice                                    device,
    VkQueryPool                                 queryPool,
    uint32_t                                    firstQuery,
    uint32_t                                    queryCount,
    size_t                                      dataSize,
    void*                                       pData,
    VkDeviceSize                                stride,
    VkQueryResultFlags                          flags,
    VkResult                                    result) override;


virtual bool PreCallValidateCreateBuffer(
    VkDevice                                    device,
    const VkBufferCreateInfo*                   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkBuffer*                                   pBuffer) const override;


virtual void PreCallRecordCreateBuffer(
    VkDevice                                    device,
    const VkBufferCreateInfo*                   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkBuffer*                                   pBuffer) override;


virtual void PostCallRecordCreateBuffer(
    VkDevice                                    device,
    const VkBufferCreateInfo*                   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkBuffer*                                   pBuffer,
    VkResult                                    result) override;


virtual bool PreCallValidateDestroyBuffer(
    VkDevice                                    device,
    VkBuffer                                    buffer,
    const VkAllocationCallbacks*                pAllocator) const override;


virtual void PreCallRecordDestroyBuffer(
    VkDevice                                    device,
    VkBuffer                                    buffer,
    const VkAllocationCallbacks*                pAllocator) override;


virtual void PostCallRecordDestroyBuffer(
    VkDevice                                    device,
    VkBuffer                                    buffer,
    const VkAllocationCallbacks*                pAllocator) override;


virtual bool PreCallValidateCreateBufferView(
    VkDevice                                    device,
    const VkBufferViewCreateInfo*               pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkBufferView*                               pView) const override;


virtual void PreCallRecordCreateBufferView(
    VkDevice                                    device,
    const VkBufferViewCreateInfo*               pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkBufferView*                               pView) override;


virtual void PostCallRecordCreateBufferView(
    VkDevice                                    device,
    const VkBufferViewCreateInfo*               pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkBufferView*                               pView,
    VkResult                                    result) override;


virtual bool PreCallValidateDestroyBufferView(
    VkDevice                                    device,
    VkBufferView                                bufferView,
    const VkAllocationCallbacks*                pAllocator) const override;


virtual void PreCallRecordDestroyBufferView(
    VkDevice                                    device,
    VkBufferView                                bufferView,
    const VkAllocationCallbacks*                pAllocator) override;


virtual void PostCallRecordDestroyBufferView(
    VkDevice                                    device,
    VkBufferView                                bufferView,
    const VkAllocationCallbacks*                pAllocator) override;


virtual bool PreCallValidateCreateImage(
    VkDevice                                    device,
    const VkImageCreateInfo*                    pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkImage*                                    pImage) const override;


virtual void PreCallRecordCreateImage(
    VkDevice                                    device,
    const VkImageCreateInfo*                    pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkImage*                                    pImage) override;


virtual void PostCallRecordCreateImage(
    VkDevice                                    device,
    const VkImageCreateInfo*                    pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkImage*                                    pImage,
    VkResult                                    result) override;


virtual bool PreCallValidateDestroyImage(
    VkDevice                                    device,
    VkImage                                     image,
    const VkAllocationCallbacks*                pAllocator) const override;


virtual void PreCallRecordDestroyImage(
    VkDevice                                    device,
    VkImage                                     image,
    const VkAllocationCallbacks*                pAllocator) override;


virtual void PostCallRecordDestroyImage(
    VkDevice                                    device,
    VkImage                                     image,
    const VkAllocationCallbacks*                pAllocator) override;


virtual bool PreCallValidateGetImageSubresourceLayout(
    VkDevice                                    device,
    VkImage                                     image,
    const VkImageSubresource*                   pSubresource,
    VkSubresourceLayout*                        pLayout) const override;


virtual void PreCallRecordGetImageSubresourceLayout(
    VkDevice                                    device,
    VkImage                                     image,
    const VkImageSubresource*                   pSubresource,
    VkSubresourceLayout*                        pLayout) override;


virtual void PostCallRecordGetImageSubresourceLayout(
    VkDevice                                    device,
    VkImage                                     image,
    const VkImageSubresource*                   pSubresource,
    VkSubresourceLayout*                        pLayout) override;


virtual bool PreCallValidateCreateImageView(
    VkDevice                                    device,
    const VkImageViewCreateInfo*                pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkImageView*                                pView) const override;


virtual void PreCallRecordCreateImageView(
    VkDevice                                    device,
    const VkImageViewCreateInfo*                pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkImageView*                                pView) override;


virtual void PostCallRecordCreateImageView(
    VkDevice                                    device,
    const VkImageViewCreateInfo*                pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkImageView*                                pView,
    VkResult                                    result) override;


virtual bool PreCallValidateDestroyImageView(
    VkDevice                                    device,
    VkImageView                                 imageView,
    const VkAllocationCallbacks*                pAllocator) const override;


virtual void PreCallRecordDestroyImageView(
    VkDevice                                    device,
    VkImageView                                 imageView,
    const VkAllocationCallbacks*                pAllocator) override;


virtual void PostCallRecordDestroyImageView(
    VkDevice                                    device,
    VkImageView                                 imageView,
    const VkAllocationCallbacks*                pAllocator) override;


virtual bool PreCallValidateCreateShaderModule(
    VkDevice                                    device,
    const VkShaderModuleCreateInfo*             pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkShaderModule*                             pShaderModule,
    void*                                       state_data) const override;


virtual void PreCallRecordCreateShaderModule(
    VkDevice                                    device,
    const VkShaderModuleCreateInfo*             pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkShaderModule*                             pShaderModule,
    void*                                       state_data) override;


virtual void PostCallRecordCreateShaderModule(
    VkDevice                                    device,
    const VkShaderModuleCreateInfo*             pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkShaderModule*                             pShaderModule,
    VkResult                                    result,
    void*                                       state_data) override;


virtual bool PreCallValidateDestroyShaderModule(
    VkDevice                                    device,
    VkShaderModule                              shaderModule,
    const VkAllocationCallbacks*                pAllocator) const override;


virtual void PreCallRecordDestroyShaderModule(
    VkDevice                                    device,
    VkShaderModule                              shaderModule,
    const VkAllocationCallbacks*                pAllocator) override;


virtual void PostCallRecordDestroyShaderModule(
    VkDevice                                    device,
    VkShaderModule                              shaderModule,
    const VkAllocationCallbacks*                pAllocator) override;


virtual bool PreCallValidateCreatePipelineCache(
    VkDevice                                    device,
    const VkPipelineCacheCreateInfo*            pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkPipelineCache*                            pPipelineCache) const override;


virtual void PreCallRecordCreatePipelineCache(
    VkDevice                                    device,
    const VkPipelineCacheCreateInfo*            pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkPipelineCache*                            pPipelineCache) override;


virtual void PostCallRecordCreatePipelineCache(
    VkDevice                                    device,
    const VkPipelineCacheCreateInfo*            pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkPipelineCache*                            pPipelineCache,
    VkResult                                    result) override;


virtual bool PreCallValidateDestroyPipelineCache(
    VkDevice                                    device,
    VkPipelineCache                             pipelineCache,
    const VkAllocationCallbacks*                pAllocator) const override;


virtual void PreCallRecordDestroyPipelineCache(
    VkDevice                                    device,
    VkPipelineCache                             pipelineCache,
    const VkAllocationCallbacks*                pAllocator) override;


virtual void PostCallRecordDestroyPipelineCache(
    VkDevice                                    device,
    VkPipelineCache                             pipelineCache,
    const VkAllocationCallbacks*                pAllocator) override;


virtual bool PreCallValidateGetPipelineCacheData(
    VkDevice                                    device,
    VkPipelineCache                             pipelineCache,
    size_t*                                     pDataSize,
    void*                                       pData) const override;


virtual void PreCallRecordGetPipelineCacheData(
    VkDevice                                    device,
    VkPipelineCache                             pipelineCache,
    size_t*                                     pDataSize,
    void*                                       pData) override;


virtual void PostCallRecordGetPipelineCacheData(
    VkDevice                                    device,
    VkPipelineCache                             pipelineCache,
    size_t*                                     pDataSize,
    void*                                       pData,
    VkResult                                    result) override;


virtual bool PreCallValidateMergePipelineCaches(
    VkDevice                                    device,
    VkPipelineCache                             dstCache,
    uint32_t                                    srcCacheCount,
    const VkPipelineCache*                      pSrcCaches) const override;


virtual void PreCallRecordMergePipelineCaches(
    VkDevice                                    device,
    VkPipelineCache                             dstCache,
    uint32_t                                    srcCacheCount,
    const VkPipelineCache*                      pSrcCaches) override;


virtual void PostCallRecordMergePipelineCaches(
    VkDevice                                    device,
    VkPipelineCache                             dstCache,
    uint32_t                                    srcCacheCount,
    const VkPipelineCache*                      pSrcCaches,
    VkResult                                    result) override;


virtual bool PreCallValidateCreateGraphicsPipelines(
    VkDevice                                    device,
    VkPipelineCache                             pipelineCache,
    uint32_t                                    createInfoCount,
    const VkGraphicsPipelineCreateInfo*         pCreateInfos,
    const VkAllocationCallbacks*                pAllocator,
    VkPipeline*                                 pPipelines,
    void*                                       state_data) const override;


virtual void PreCallRecordCreateGraphicsPipelines(
    VkDevice                                    device,
    VkPipelineCache                             pipelineCache,
    uint32_t                                    createInfoCount,
    const VkGraphicsPipelineCreateInfo*         pCreateInfos,
    const VkAllocationCallbacks*                pAllocator,
    VkPipeline*                                 pPipelines,
    void*                                       state_data) override;


virtual void PostCallRecordCreateGraphicsPipelines(
    VkDevice                                    device,
    VkPipelineCache                             pipelineCache,
    uint32_t                                    createInfoCount,
    const VkGraphicsPipelineCreateInfo*         pCreateInfos,
    const VkAllocationCallbacks*                pAllocator,
    VkPipeline*                                 pPipelines,
    VkResult                                    result,
    void*                                       state_data) override;


virtual bool PreCallValidateCreateComputePipelines(
    VkDevice                                    device,
    VkPipelineCache                             pipelineCache,
    uint32_t                                    createInfoCount,
    const VkComputePipelineCreateInfo*          pCreateInfos,
    const VkAllocationCallbacks*                pAllocator,
    VkPipeline*                                 pPipelines,
    void*                                       state_data) const override;


virtual void PreCallRecordCreateComputePipelines(
    VkDevice                                    device,
    VkPipelineCache                             pipelineCache,
    uint32_t                                    createInfoCount,
    const VkComputePipelineCreateInfo*          pCreateInfos,
    const VkAllocationCallbacks*                pAllocator,
    VkPipeline*                                 pPipelines,
    void*                                       state_data) override;


virtual void PostCallRecordCreateComputePipelines(
    VkDevice                                    device,
    VkPipelineCache                             pipelineCache,
    uint32_t                                    createInfoCount,
    const VkComputePipelineCreateInfo*          pCreateInfos,
    const VkAllocationCallbacks*                pAllocator,
    VkPipeline*                                 pPipelines,
    VkResult                                    result,
    void*                                       state_data) override;


virtual bool PreCallValidateDestroyPipeline(
    VkDevice                                    device,
    VkPipeline                                  pipeline,
    const VkAllocationCallbacks*                pAllocator) const override;


virtual void PreCallRecordDestroyPipeline(
    VkDevice                                    device,
    VkPipeline                                  pipeline,
    const VkAllocationCallbacks*                pAllocator) override;


virtual void PostCallRecordDestroyPipeline(
    VkDevice                                    device,
    VkPipeline                                  pipeline,
    const VkAllocationCallbacks*                pAllocator) override;


virtual bool PreCallValidateCreatePipelineLayout(
    VkDevice                                    device,
    const VkPipelineLayoutCreateInfo*           pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkPipelineLayout*                           pPipelineLayout) const override;


virtual void PreCallRecordCreatePipelineLayout(
    VkDevice                                    device,
    const VkPipelineLayoutCreateInfo*           pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkPipelineLayout*                           pPipelineLayout) override;


virtual void PostCallRecordCreatePipelineLayout(
    VkDevice                                    device,
    const VkPipelineLayoutCreateInfo*           pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkPipelineLayout*                           pPipelineLayout,
    VkResult                                    result) override;


virtual bool PreCallValidateDestroyPipelineLayout(
    VkDevice                                    device,
    VkPipelineLayout                            pipelineLayout,
    const VkAllocationCallbacks*                pAllocator) const override;


virtual void PreCallRecordDestroyPipelineLayout(
    VkDevice                                    device,
    VkPipelineLayout                            pipelineLayout,
    const VkAllocationCallbacks*                pAllocator) override;


virtual void PostCallRecordDestroyPipelineLayout(
    VkDevice                                    device,
    VkPipelineLayout                            pipelineLayout,
    const VkAllocationCallbacks*                pAllocator) override;


virtual bool PreCallValidateCreateSampler(
    VkDevice                                    device,
    const VkSamplerCreateInfo*                  pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSampler*                                  pSampler) const override;


virtual void PreCallRecordCreateSampler(
    VkDevice                                    device,
    const VkSamplerCreateInfo*                  pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSampler*                                  pSampler) override;


virtual void PostCallRecordCreateSampler(
    VkDevice                                    device,
    const VkSamplerCreateInfo*                  pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSampler*                                  pSampler,
    VkResult                                    result) override;


virtual bool PreCallValidateDestroySampler(
    VkDevice                                    device,
    VkSampler                                   sampler,
    const VkAllocationCallbacks*                pAllocator) const override;


virtual void PreCallRecordDestroySampler(
    VkDevice                                    device,
    VkSampler                                   sampler,
    const VkAllocationCallbacks*                pAllocator) override;


virtual void PostCallRecordDestroySampler(
    VkDevice                                    device,
    VkSampler                                   sampler,
    const VkAllocationCallbacks*                pAllocator) override;


virtual bool PreCallValidateCreateDescriptorSetLayout(
    VkDevice                                    device,
    const VkDescriptorSetLayoutCreateInfo*      pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDescriptorSetLayout*                      pSetLayout) const override;


virtual void PreCallRecordCreateDescriptorSetLayout(
    VkDevice                                    device,
    const VkDescriptorSetLayoutCreateInfo*      pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDescriptorSetLayout*                      pSetLayout) override;


virtual void PostCallRecordCreateDescriptorSetLayout(
    VkDevice                                    device,
    const VkDescriptorSetLayoutCreateInfo*      pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDescriptorSetLayout*                      pSetLayout,
    VkResult                                    result) override;


virtual bool PreCallValidateDestroyDescriptorSetLayout(
    VkDevice                                    device,
    VkDescriptorSetLayout                       descriptorSetLayout,
    const VkAllocationCallbacks*                pAllocator) const override;


virtual void PreCallRecordDestroyDescriptorSetLayout(
    VkDevice                                    device,
    VkDescriptorSetLayout                       descriptorSetLayout,
    const VkAllocationCallbacks*                pAllocator) override;


virtual void PostCallRecordDestroyDescriptorSetLayout(
    VkDevice                                    device,
    VkDescriptorSetLayout                       descriptorSetLayout,
    const VkAllocationCallbacks*                pAllocator) override;


virtual bool PreCallValidateCreateDescriptorPool(
    VkDevice                                    device,
    const VkDescriptorPoolCreateInfo*           pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDescriptorPool*                           pDescriptorPool) const override;


virtual void PreCallRecordCreateDescriptorPool(
    VkDevice                                    device,
    const VkDescriptorPoolCreateInfo*           pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDescriptorPool*                           pDescriptorPool) override;


virtual void PostCallRecordCreateDescriptorPool(
    VkDevice                                    device,
    const VkDescriptorPoolCreateInfo*           pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDescriptorPool*                           pDescriptorPool,
    VkResult                                    result) override;


virtual bool PreCallValidateDestroyDescriptorPool(
    VkDevice                                    device,
    VkDescriptorPool                            descriptorPool,
    const VkAllocationCallbacks*                pAllocator) const override;


virtual void PreCallRecordDestroyDescriptorPool(
    VkDevice                                    device,
    VkDescriptorPool                            descriptorPool,
    const VkAllocationCallbacks*                pAllocator) override;


virtual void PostCallRecordDestroyDescriptorPool(
    VkDevice                                    device,
    VkDescriptorPool                            descriptorPool,
    const VkAllocationCallbacks*                pAllocator) override;


virtual bool PreCallValidateResetDescriptorPool(
    VkDevice                                    device,
    VkDescriptorPool                            descriptorPool,
    VkDescriptorPoolResetFlags                  flags) const override;


virtual void PreCallRecordResetDescriptorPool(
    VkDevice                                    device,
    VkDescriptorPool                            descriptorPool,
    VkDescriptorPoolResetFlags                  flags) override;


virtual void PostCallRecordResetDescriptorPool(
    VkDevice                                    device,
    VkDescriptorPool                            descriptorPool,
    VkDescriptorPoolResetFlags                  flags,
    VkResult                                    result) override;


virtual bool PreCallValidateAllocateDescriptorSets(
    VkDevice                                    device,
    const VkDescriptorSetAllocateInfo*          pAllocateInfo,
    VkDescriptorSet*                            pDescriptorSets,
    void*                                       state_data) const override;


virtual void PreCallRecordAllocateDescriptorSets(
    VkDevice                                    device,
    const VkDescriptorSetAllocateInfo*          pAllocateInfo,
    VkDescriptorSet*                            pDescriptorSets,
    void*                                       state_data) override;


virtual void PostCallRecordAllocateDescriptorSets(
    VkDevice                                    device,
    const VkDescriptorSetAllocateInfo*          pAllocateInfo,
    VkDescriptorSet*                            pDescriptorSets,
    VkResult                                    result,
    void*                                       state_data) override;


virtual bool PreCallValidateFreeDescriptorSets(
    VkDevice                                    device,
    VkDescriptorPool                            descriptorPool,
    uint32_t                                    descriptorSetCount,
    const VkDescriptorSet*                      pDescriptorSets) const override;


virtual void PreCallRecordFreeDescriptorSets(
    VkDevice                                    device,
    VkDescriptorPool                            descriptorPool,
    uint32_t                                    descriptorSetCount,
    const VkDescriptorSet*                      pDescriptorSets) override;


virtual void PostCallRecordFreeDescriptorSets(
    VkDevice                                    device,
    VkDescriptorPool                            descriptorPool,
    uint32_t                                    descriptorSetCount,
    const VkDescriptorSet*                      pDescriptorSets,
    VkResult                                    result) override;


virtual bool PreCallValidateUpdateDescriptorSets(
    VkDevice                                    device,
    uint32_t                                    descriptorWriteCount,
    const VkWriteDescriptorSet*                 pDescriptorWrites,
    uint32_t                                    descriptorCopyCount,
    const VkCopyDescriptorSet*                  pDescriptorCopies) const override;


virtual void PreCallRecordUpdateDescriptorSets(
    VkDevice                                    device,
    uint32_t                                    descriptorWriteCount,
    const VkWriteDescriptorSet*                 pDescriptorWrites,
    uint32_t                                    descriptorCopyCount,
    const VkCopyDescriptorSet*                  pDescriptorCopies) override;


virtual void PostCallRecordUpdateDescriptorSets(
    VkDevice                                    device,
    uint32_t                                    descriptorWriteCount,
    const VkWriteDescriptorSet*                 pDescriptorWrites,
    uint32_t                                    descriptorCopyCount,
    const VkCopyDescriptorSet*                  pDescriptorCopies) override;


virtual bool PreCallValidateCreateFramebuffer(
    VkDevice                                    device,
    const VkFramebufferCreateInfo*              pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkFramebuffer*                              pFramebuffer) const override;


virtual void PreCallRecordCreateFramebuffer(
    VkDevice                                    device,
    const VkFramebufferCreateInfo*              pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkFramebuffer*                              pFramebuffer) override;


virtual void PostCallRecordCreateFramebuffer(
    VkDevice                                    device,
    const VkFramebufferCreateInfo*              pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkFramebuffer*                              pFramebuffer,
    VkResult                                    result) override;


virtual bool PreCallValidateDestroyFramebuffer(
    VkDevice                                    device,
    VkFramebuffer                               framebuffer,
    const VkAllocationCallbacks*                pAllocator) const override;


virtual void PreCallRecordDestroyFramebuffer(
    VkDevice                                    device,
    VkFramebuffer                               framebuffer,
    const VkAllocationCallbacks*                pAllocator) override;


virtual void PostCallRecordDestroyFramebuffer(
    VkDevice                                    device,
    VkFramebuffer                               framebuffer,
    const VkAllocationCallbacks*                pAllocator) override;


virtual bool PreCallValidateCreateRenderPass(
    VkDevice                                    device,
    const VkRenderPassCreateInfo*               pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkRenderPass*                               pRenderPass) const override;


virtual void PreCallRecordCreateRenderPass(
    VkDevice                                    device,
    const VkRenderPassCreateInfo*               pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkRenderPass*                               pRenderPass) override;


virtual void PostCallRecordCreateRenderPass(
    VkDevice                                    device,
    const VkRenderPassCreateInfo*               pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkRenderPass*                               pRenderPass,
    VkResult                                    result) override;


virtual bool PreCallValidateDestroyRenderPass(
    VkDevice                                    device,
    VkRenderPass                                renderPass,
    const VkAllocationCallbacks*                pAllocator) const override;


virtual void PreCallRecordDestroyRenderPass(
    VkDevice                                    device,
    VkRenderPass                                renderPass,
    const VkAllocationCallbacks*                pAllocator) override;


virtual void PostCallRecordDestroyRenderPass(
    VkDevice                                    device,
    VkRenderPass                                renderPass,
    const VkAllocationCallbacks*                pAllocator) override;


virtual bool PreCallValidateGetRenderAreaGranularity(
    VkDevice                                    device,
    VkRenderPass                                renderPass,
    VkExtent2D*                                 pGranularity) const override;


virtual void PreCallRecordGetRenderAreaGranularity(
    VkDevice                                    device,
    VkRenderPass                                renderPass,
    VkExtent2D*                                 pGranularity) override;


virtual void PostCallRecordGetRenderAreaGranularity(
    VkDevice                                    device,
    VkRenderPass                                renderPass,
    VkExtent2D*                                 pGranularity) override;


virtual bool PreCallValidateCreateCommandPool(
    VkDevice                                    device,
    const VkCommandPoolCreateInfo*              pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkCommandPool*                              pCommandPool) const override;


virtual void PreCallRecordCreateCommandPool(
    VkDevice                                    device,
    const VkCommandPoolCreateInfo*              pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkCommandPool*                              pCommandPool) override;


virtual void PostCallRecordCreateCommandPool(
    VkDevice                                    device,
    const VkCommandPoolCreateInfo*              pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkCommandPool*                              pCommandPool,
    VkResult                                    result) override;


virtual bool PreCallValidateDestroyCommandPool(
    VkDevice                                    device,
    VkCommandPool                               commandPool,
    const VkAllocationCallbacks*                pAllocator) const override;


virtual void PreCallRecordDestroyCommandPool(
    VkDevice                                    device,
    VkCommandPool                               commandPool,
    const VkAllocationCallbacks*                pAllocator) override;


virtual void PostCallRecordDestroyCommandPool(
    VkDevice                                    device,
    VkCommandPool                               commandPool,
    const VkAllocationCallbacks*                pAllocator) override;


virtual bool PreCallValidateResetCommandPool(
    VkDevice                                    device,
    VkCommandPool                               commandPool,
    VkCommandPoolResetFlags                     flags) const override;


virtual void PreCallRecordResetCommandPool(
    VkDevice                                    device,
    VkCommandPool                               commandPool,
    VkCommandPoolResetFlags                     flags) override;


virtual void PostCallRecordResetCommandPool(
    VkDevice                                    device,
    VkCommandPool                               commandPool,
    VkCommandPoolResetFlags                     flags,
    VkResult                                    result) override;


virtual bool PreCallValidateAllocateCommandBuffers(
    VkDevice                                    device,
    const VkCommandBufferAllocateInfo*          pAllocateInfo,
    VkCommandBuffer*                            pCommandBuffers) const override;


virtual void PreCallRecordAllocateCommandBuffers(
    VkDevice                                    device,
    const VkCommandBufferAllocateInfo*          pAllocateInfo,
    VkCommandBuffer*                            pCommandBuffers) override;


virtual void PostCallRecordAllocateCommandBuffers(
    VkDevice                                    device,
    const VkCommandBufferAllocateInfo*          pAllocateInfo,
    VkCommandBuffer*                            pCommandBuffers,
    VkResult                                    result) override;


virtual bool PreCallValidateFreeCommandBuffers(
    VkDevice                                    device,
    VkCommandPool                               commandPool,
    uint32_t                                    commandBufferCount,
    const VkCommandBuffer*                      pCommandBuffers) const override;


virtual void PreCallRecordFreeCommandBuffers(
    VkDevice                                    device,
    VkCommandPool                               commandPool,
    uint32_t                                    commandBufferCount,
    const VkCommandBuffer*                      pCommandBuffers) override;


virtual void PostCallRecordFreeCommandBuffers(
    VkDevice                                    device,
    VkCommandPool                               commandPool,
    uint32_t                                    commandBufferCount,
    const VkCommandBuffer*                      pCommandBuffers) override;


virtual bool PreCallValidateBeginCommandBuffer(
    VkCommandBuffer                             commandBuffer,
    const VkCommandBufferBeginInfo*             pBeginInfo) const override;


virtual void PreCallRecordBeginCommandBuffer(
    VkCommandBuffer                             commandBuffer,
    const VkCommandBufferBeginInfo*             pBeginInfo) override;


virtual void PostCallRecordBeginCommandBuffer(
    VkCommandBuffer                             commandBuffer,
    const VkCommandBufferBeginInfo*             pBeginInfo,
    VkResult                                    result) override;


virtual bool PreCallValidateEndCommandBuffer(
    VkCommandBuffer                             commandBuffer) const override;


virtual void PreCallRecordEndCommandBuffer(
    VkCommandBuffer                             commandBuffer) override;


virtual void PostCallRecordEndCommandBuffer(
    VkCommandBuffer                             commandBuffer,
    VkResult                                    result) override;


virtual bool PreCallValidateResetCommandBuffer(
    VkCommandBuffer                             commandBuffer,
    VkCommandBufferResetFlags                   flags) const override;


virtual void PreCallRecordResetCommandBuffer(
    VkCommandBuffer                             commandBuffer,
    VkCommandBufferResetFlags                   flags) override;


virtual void PostCallRecordResetCommandBuffer(
    VkCommandBuffer                             commandBuffer,
    VkCommandBufferResetFlags                   flags,
    VkResult                                    result) override;


virtual bool PreCallValidateCmdBindPipeline(
    VkCommandBuffer                             commandBuffer,
    VkPipelineBindPoint                         pipelineBindPoint,
    VkPipeline                                  pipeline) const override;


virtual void PreCallRecordCmdBindPipeline(
    VkCommandBuffer                             commandBuffer,
    VkPipelineBindPoint                         pipelineBindPoint,
    VkPipeline                                  pipeline) override;


virtual void PostCallRecordCmdBindPipeline(
    VkCommandBuffer                             commandBuffer,
    VkPipelineBindPoint                         pipelineBindPoint,
    VkPipeline                                  pipeline) override;


virtual bool PreCallValidateCmdSetViewport(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstViewport,
    uint32_t                                    viewportCount,
    const VkViewport*                           pViewports) const override;


virtual void PreCallRecordCmdSetViewport(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstViewport,
    uint32_t                                    viewportCount,
    const VkViewport*                           pViewports) override;


virtual void PostCallRecordCmdSetViewport(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstViewport,
    uint32_t                                    viewportCount,
    const VkViewport*                           pViewports) override;


virtual bool PreCallValidateCmdSetScissor(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstScissor,
    uint32_t                                    scissorCount,
    const VkRect2D*                             pScissors) const override;


virtual void PreCallRecordCmdSetScissor(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstScissor,
    uint32_t                                    scissorCount,
    const VkRect2D*                             pScissors) override;


virtual void PostCallRecordCmdSetScissor(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstScissor,
    uint32_t                                    scissorCount,
    const VkRect2D*                             pScissors) override;


virtual bool PreCallValidateCmdSetLineWidth(
    VkCommandBuffer                             commandBuffer,
    float                                       lineWidth) const override;


virtual void PreCallRecordCmdSetLineWidth(
    VkCommandBuffer                             commandBuffer,
    float                                       lineWidth) override;


virtual void PostCallRecordCmdSetLineWidth(
    VkCommandBuffer                             commandBuffer,
    float                                       lineWidth) override;


virtual bool PreCallValidateCmdSetDepthBias(
    VkCommandBuffer                             commandBuffer,
    float                                       depthBiasConstantFactor,
    float                                       depthBiasClamp,
    float                                       depthBiasSlopeFactor) const override;


virtual void PreCallRecordCmdSetDepthBias(
    VkCommandBuffer                             commandBuffer,
    float                                       depthBiasConstantFactor,
    float                                       depthBiasClamp,
    float                                       depthBiasSlopeFactor) override;


virtual void PostCallRecordCmdSetDepthBias(
    VkCommandBuffer                             commandBuffer,
    float                                       depthBiasConstantFactor,
    float                                       depthBiasClamp,
    float                                       depthBiasSlopeFactor) override;


virtual bool PreCallValidateCmdSetBlendConstants(
    VkCommandBuffer                             commandBuffer,
    const float                                 blendConstants[4]) const override;


virtual void PreCallRecordCmdSetBlendConstants(
    VkCommandBuffer                             commandBuffer,
    const float                                 blendConstants[4]) override;


virtual void PostCallRecordCmdSetBlendConstants(
    VkCommandBuffer                             commandBuffer,
    const float                                 blendConstants[4]) override;


virtual bool PreCallValidateCmdSetDepthBounds(
    VkCommandBuffer                             commandBuffer,
    float                                       minDepthBounds,
    float                                       maxDepthBounds) const override;


virtual void PreCallRecordCmdSetDepthBounds(
    VkCommandBuffer                             commandBuffer,
    float                                       minDepthBounds,
    float                                       maxDepthBounds) override;


virtual void PostCallRecordCmdSetDepthBounds(
    VkCommandBuffer                             commandBuffer,
    float                                       minDepthBounds,
    float                                       maxDepthBounds) override;


virtual bool PreCallValidateCmdSetStencilCompareMask(
    VkCommandBuffer                             commandBuffer,
    VkStencilFaceFlags                          faceMask,
    uint32_t                                    compareMask) const override;


virtual void PreCallRecordCmdSetStencilCompareMask(
    VkCommandBuffer                             commandBuffer,
    VkStencilFaceFlags                          faceMask,
    uint32_t                                    compareMask) override;


virtual void PostCallRecordCmdSetStencilCompareMask(
    VkCommandBuffer                             commandBuffer,
    VkStencilFaceFlags                          faceMask,
    uint32_t                                    compareMask) override;


virtual bool PreCallValidateCmdSetStencilWriteMask(
    VkCommandBuffer                             commandBuffer,
    VkStencilFaceFlags                          faceMask,
    uint32_t                                    writeMask) const override;


virtual void PreCallRecordCmdSetStencilWriteMask(
    VkCommandBuffer                             commandBuffer,
    VkStencilFaceFlags                          faceMask,
    uint32_t                                    writeMask) override;


virtual void PostCallRecordCmdSetStencilWriteMask(
    VkCommandBuffer                             commandBuffer,
    VkStencilFaceFlags                          faceMask,
    uint32_t                                    writeMask) override;


virtual bool PreCallValidateCmdSetStencilReference(
    VkCommandBuffer                             commandBuffer,
    VkStencilFaceFlags                          faceMask,
    uint32_t                                    reference) const override;


virtual void PreCallRecordCmdSetStencilReference(
    VkCommandBuffer                             commandBuffer,
    VkStencilFaceFlags                          faceMask,
    uint32_t                                    reference) override;


virtual void PostCallRecordCmdSetStencilReference(
    VkCommandBuffer                             commandBuffer,
    VkStencilFaceFlags                          faceMask,
    uint32_t                                    reference) override;


virtual bool PreCallValidateCmdBindDescriptorSets(
    VkCommandBuffer                             commandBuffer,
    VkPipelineBindPoint                         pipelineBindPoint,
    VkPipelineLayout                            layout,
    uint32_t                                    firstSet,
    uint32_t                                    descriptorSetCount,
    const VkDescriptorSet*                      pDescriptorSets,
    uint32_t                                    dynamicOffsetCount,
    const uint32_t*                             pDynamicOffsets) const override;


virtual void PreCallRecordCmdBindDescriptorSets(
    VkCommandBuffer                             commandBuffer,
    VkPipelineBindPoint                         pipelineBindPoint,
    VkPipelineLayout                            layout,
    uint32_t                                    firstSet,
    uint32_t                                    descriptorSetCount,
    const VkDescriptorSet*                      pDescriptorSets,
    uint32_t                                    dynamicOffsetCount,
    const uint32_t*                             pDynamicOffsets) override;


virtual void PostCallRecordCmdBindDescriptorSets(
    VkCommandBuffer                             commandBuffer,
    VkPipelineBindPoint                         pipelineBindPoint,
    VkPipelineLayout                            layout,
    uint32_t                                    firstSet,
    uint32_t                                    descriptorSetCount,
    const VkDescriptorSet*                      pDescriptorSets,
    uint32_t                                    dynamicOffsetCount,
    const uint32_t*                             pDynamicOffsets) override;


virtual bool PreCallValidateCmdBindIndexBuffer(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkIndexType                                 indexType) const override;


virtual void PreCallRecordCmdBindIndexBuffer(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkIndexType                                 indexType) override;


virtual void PostCallRecordCmdBindIndexBuffer(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkIndexType                                 indexType) override;


virtual bool PreCallValidateCmdBindVertexBuffers(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstBinding,
    uint32_t                                    bindingCount,
    const VkBuffer*                             pBuffers,
    const VkDeviceSize*                         pOffsets) const override;


virtual void PreCallRecordCmdBindVertexBuffers(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstBinding,
    uint32_t                                    bindingCount,
    const VkBuffer*                             pBuffers,
    const VkDeviceSize*                         pOffsets) override;


virtual void PostCallRecordCmdBindVertexBuffers(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstBinding,
    uint32_t                                    bindingCount,
    const VkBuffer*                             pBuffers,
    const VkDeviceSize*                         pOffsets) override;


virtual bool PreCallValidateCmdDraw(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    vertexCount,
    uint32_t                                    instanceCount,
    uint32_t                                    firstVertex,
    uint32_t                                    firstInstance) const override;


virtual void PreCallRecordCmdDraw(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    vertexCount,
    uint32_t                                    instanceCount,
    uint32_t                                    firstVertex,
    uint32_t                                    firstInstance) override;


virtual void PostCallRecordCmdDraw(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    vertexCount,
    uint32_t                                    instanceCount,
    uint32_t                                    firstVertex,
    uint32_t                                    firstInstance) override;


virtual bool PreCallValidateCmdDrawIndexed(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    indexCount,
    uint32_t                                    instanceCount,
    uint32_t                                    firstIndex,
    int32_t                                     vertexOffset,
    uint32_t                                    firstInstance) const override;


virtual void PreCallRecordCmdDrawIndexed(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    indexCount,
    uint32_t                                    instanceCount,
    uint32_t                                    firstIndex,
    int32_t                                     vertexOffset,
    uint32_t                                    firstInstance) override;


virtual void PostCallRecordCmdDrawIndexed(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    indexCount,
    uint32_t                                    instanceCount,
    uint32_t                                    firstIndex,
    int32_t                                     vertexOffset,
    uint32_t                                    firstInstance) override;


virtual bool PreCallValidateCmdDrawIndirect(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    uint32_t                                    drawCount,
    uint32_t                                    stride) const override;


virtual void PreCallRecordCmdDrawIndirect(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    uint32_t                                    drawCount,
    uint32_t                                    stride) override;


virtual void PostCallRecordCmdDrawIndirect(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    uint32_t                                    drawCount,
    uint32_t                                    stride) override;


virtual bool PreCallValidateCmdDrawIndexedIndirect(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    uint32_t                                    drawCount,
    uint32_t                                    stride) const override;


virtual void PreCallRecordCmdDrawIndexedIndirect(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    uint32_t                                    drawCount,
    uint32_t                                    stride) override;


virtual void PostCallRecordCmdDrawIndexedIndirect(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    uint32_t                                    drawCount,
    uint32_t                                    stride) override;


virtual bool PreCallValidateCmdDispatch(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    groupCountX,
    uint32_t                                    groupCountY,
    uint32_t                                    groupCountZ) const override;


virtual void PreCallRecordCmdDispatch(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    groupCountX,
    uint32_t                                    groupCountY,
    uint32_t                                    groupCountZ) override;


virtual void PostCallRecordCmdDispatch(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    groupCountX,
    uint32_t                                    groupCountY,
    uint32_t                                    groupCountZ) override;


virtual bool PreCallValidateCmdDispatchIndirect(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset) const override;


virtual void PreCallRecordCmdDispatchIndirect(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset) override;


virtual void PostCallRecordCmdDispatchIndirect(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset) override;


virtual bool PreCallValidateCmdCopyBuffer(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    srcBuffer,
    VkBuffer                                    dstBuffer,
    uint32_t                                    regionCount,
    const VkBufferCopy*                         pRegions) const override;


virtual void PreCallRecordCmdCopyBuffer(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    srcBuffer,
    VkBuffer                                    dstBuffer,
    uint32_t                                    regionCount,
    const VkBufferCopy*                         pRegions) override;


virtual void PostCallRecordCmdCopyBuffer(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    srcBuffer,
    VkBuffer                                    dstBuffer,
    uint32_t                                    regionCount,
    const VkBufferCopy*                         pRegions) override;


virtual bool PreCallValidateCmdCopyImage(
    VkCommandBuffer                             commandBuffer,
    VkImage                                     srcImage,
    VkImageLayout                               srcImageLayout,
    VkImage                                     dstImage,
    VkImageLayout                               dstImageLayout,
    uint32_t                                    regionCount,
    const VkImageCopy*                          pRegions) const override;


virtual void PreCallRecordCmdCopyImage(
    VkCommandBuffer                             commandBuffer,
    VkImage                                     srcImage,
    VkImageLayout                               srcImageLayout,
    VkImage                                     dstImage,
    VkImageLayout                               dstImageLayout,
    uint32_t                                    regionCount,
    const VkImageCopy*                          pRegions) override;


virtual void PostCallRecordCmdCopyImage(
    VkCommandBuffer                             commandBuffer,
    VkImage                                     srcImage,
    VkImageLayout                               srcImageLayout,
    VkImage                                     dstImage,
    VkImageLayout                               dstImageLayout,
    uint32_t                                    regionCount,
    const VkImageCopy*                          pRegions) override;


virtual bool PreCallValidateCmdBlitImage(
    VkCommandBuffer                             commandBuffer,
    VkImage                                     srcImage,
    VkImageLayout                               srcImageLayout,
    VkImage                                     dstImage,
    VkImageLayout                               dstImageLayout,
    uint32_t                                    regionCount,
    const VkImageBlit*                          pRegions,
    VkFilter                                    filter) const override;


virtual void PreCallRecordCmdBlitImage(
    VkCommandBuffer                             commandBuffer,
    VkImage                                     srcImage,
    VkImageLayout                               srcImageLayout,
    VkImage                                     dstImage,
    VkImageLayout                               dstImageLayout,
    uint32_t                                    regionCount,
    const VkImageBlit*                          pRegions,
    VkFilter                                    filter) override;


virtual void PostCallRecordCmdBlitImage(
    VkCommandBuffer                             commandBuffer,
    VkImage                                     srcImage,
    VkImageLayout                               srcImageLayout,
    VkImage                                     dstImage,
    VkImageLayout                               dstImageLayout,
    uint32_t                                    regionCount,
    const VkImageBlit*                          pRegions,
    VkFilter                                    filter) override;


virtual bool PreCallValidateCmdCopyBufferToImage(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    srcBuffer,
    VkImage                                     dstImage,
    VkImageLayout                               dstImageLayout,
    uint32_t                                    regionCount,
    const VkBufferImageCopy*                    pRegions) const override;


virtual void PreCallRecordCmdCopyBufferToImage(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    srcBuffer,
    VkImage                                     dstImage,
    VkImageLayout                               dstImageLayout,
    uint32_t                                    regionCount,
    const VkBufferImageCopy*                    pRegions) override;


virtual void PostCallRecordCmdCopyBufferToImage(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    srcBuffer,
    VkImage                                     dstImage,
    VkImageLayout                               dstImageLayout,
    uint32_t                                    regionCount,
    const VkBufferImageCopy*                    pRegions) override;


virtual bool PreCallValidateCmdCopyImageToBuffer(
    VkCommandBuffer                             commandBuffer,
    VkImage                                     srcImage,
    VkImageLayout                               srcImageLayout,
    VkBuffer                                    dstBuffer,
    uint32_t                                    regionCount,
    const VkBufferImageCopy*                    pRegions) const override;


virtual void PreCallRecordCmdCopyImageToBuffer(
    VkCommandBuffer                             commandBuffer,
    VkImage                                     srcImage,
    VkImageLayout                               srcImageLayout,
    VkBuffer                                    dstBuffer,
    uint32_t                                    regionCount,
    const VkBufferImageCopy*                    pRegions) override;


virtual void PostCallRecordCmdCopyImageToBuffer(
    VkCommandBuffer                             commandBuffer,
    VkImage                                     srcImage,
    VkImageLayout                               srcImageLayout,
    VkBuffer                                    dstBuffer,
    uint32_t                                    regionCount,
    const VkBufferImageCopy*                    pRegions) override;


virtual bool PreCallValidateCmdUpdateBuffer(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    dstBuffer,
    VkDeviceSize                                dstOffset,
    VkDeviceSize                                dataSize,
    const void*                                 pData) const override;


virtual void PreCallRecordCmdUpdateBuffer(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    dstBuffer,
    VkDeviceSize                                dstOffset,
    VkDeviceSize                                dataSize,
    const void*                                 pData) override;


virtual void PostCallRecordCmdUpdateBuffer(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    dstBuffer,
    VkDeviceSize                                dstOffset,
    VkDeviceSize                                dataSize,
    const void*                                 pData) override;


virtual bool PreCallValidateCmdFillBuffer(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    dstBuffer,
    VkDeviceSize                                dstOffset,
    VkDeviceSize                                size,
    uint32_t                                    data) const override;


virtual void PreCallRecordCmdFillBuffer(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    dstBuffer,
    VkDeviceSize                                dstOffset,
    VkDeviceSize                                size,
    uint32_t                                    data) override;


virtual void PostCallRecordCmdFillBuffer(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    dstBuffer,
    VkDeviceSize                                dstOffset,
    VkDeviceSize                                size,
    uint32_t                                    data) override;


virtual bool PreCallValidateCmdClearColorImage(
    VkCommandBuffer                             commandBuffer,
    VkImage                                     image,
    VkImageLayout                               imageLayout,
    const VkClearColorValue*                    pColor,
    uint32_t                                    rangeCount,
    const VkImageSubresourceRange*              pRanges) const override;


virtual void PreCallRecordCmdClearColorImage(
    VkCommandBuffer                             commandBuffer,
    VkImage                                     image,
    VkImageLayout                               imageLayout,
    const VkClearColorValue*                    pColor,
    uint32_t                                    rangeCount,
    const VkImageSubresourceRange*              pRanges) override;


virtual void PostCallRecordCmdClearColorImage(
    VkCommandBuffer                             commandBuffer,
    VkImage                                     image,
    VkImageLayout                               imageLayout,
    const VkClearColorValue*                    pColor,
    uint32_t                                    rangeCount,
    const VkImageSubresourceRange*              pRanges) override;


virtual bool PreCallValidateCmdClearDepthStencilImage(
    VkCommandBuffer                             commandBuffer,
    VkImage                                     image,
    VkImageLayout                               imageLayout,
    const VkClearDepthStencilValue*             pDepthStencil,
    uint32_t                                    rangeCount,
    const VkImageSubresourceRange*              pRanges) const override;


virtual void PreCallRecordCmdClearDepthStencilImage(
    VkCommandBuffer                             commandBuffer,
    VkImage                                     image,
    VkImageLayout                               imageLayout,
    const VkClearDepthStencilValue*             pDepthStencil,
    uint32_t                                    rangeCount,
    const VkImageSubresourceRange*              pRanges) override;


virtual void PostCallRecordCmdClearDepthStencilImage(
    VkCommandBuffer                             commandBuffer,
    VkImage                                     image,
    VkImageLayout                               imageLayout,
    const VkClearDepthStencilValue*             pDepthStencil,
    uint32_t                                    rangeCount,
    const VkImageSubresourceRange*              pRanges) override;


virtual bool PreCallValidateCmdClearAttachments(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    attachmentCount,
    const VkClearAttachment*                    pAttachments,
    uint32_t                                    rectCount,
    const VkClearRect*                          pRects) const override;


virtual void PreCallRecordCmdClearAttachments(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    attachmentCount,
    const VkClearAttachment*                    pAttachments,
    uint32_t                                    rectCount,
    const VkClearRect*                          pRects) override;


virtual void PostCallRecordCmdClearAttachments(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    attachmentCount,
    const VkClearAttachment*                    pAttachments,
    uint32_t                                    rectCount,
    const VkClearRect*                          pRects) override;


virtual bool PreCallValidateCmdResolveImage(
    VkCommandBuffer                             commandBuffer,
    VkImage                                     srcImage,
    VkImageLayout                               srcImageLayout,
    VkImage                                     dstImage,
    VkImageLayout                               dstImageLayout,
    uint32_t                                    regionCount,
    const VkImageResolve*                       pRegions) const override;


virtual void PreCallRecordCmdResolveImage(
    VkCommandBuffer                             commandBuffer,
    VkImage                                     srcImage,
    VkImageLayout                               srcImageLayout,
    VkImage                                     dstImage,
    VkImageLayout                               dstImageLayout,
    uint32_t                                    regionCount,
    const VkImageResolve*                       pRegions) override;


virtual void PostCallRecordCmdResolveImage(
    VkCommandBuffer                             commandBuffer,
    VkImage                                     srcImage,
    VkImageLayout                               srcImageLayout,
    VkImage                                     dstImage,
    VkImageLayout                               dstImageLayout,
    uint32_t                                    regionCount,
    const VkImageResolve*                       pRegions) override;


virtual bool PreCallValidateCmdSetEvent(
    VkCommandBuffer                             commandBuffer,
    VkEvent                                     event,
    VkPipelineStageFlags                        stageMask) const override;


virtual void PreCallRecordCmdSetEvent(
    VkCommandBuffer                             commandBuffer,
    VkEvent                                     event,
    VkPipelineStageFlags                        stageMask) override;


virtual void PostCallRecordCmdSetEvent(
    VkCommandBuffer                             commandBuffer,
    VkEvent                                     event,
    VkPipelineStageFlags                        stageMask) override;


virtual bool PreCallValidateCmdResetEvent(
    VkCommandBuffer                             commandBuffer,
    VkEvent                                     event,
    VkPipelineStageFlags                        stageMask) const override;


virtual void PreCallRecordCmdResetEvent(
    VkCommandBuffer                             commandBuffer,
    VkEvent                                     event,
    VkPipelineStageFlags                        stageMask) override;


virtual void PostCallRecordCmdResetEvent(
    VkCommandBuffer                             commandBuffer,
    VkEvent                                     event,
    VkPipelineStageFlags                        stageMask) override;


virtual bool PreCallValidateCmdWaitEvents(
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
    const VkImageMemoryBarrier*                 pImageMemoryBarriers) const override;


virtual void PreCallRecordCmdWaitEvents(
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
    const VkImageMemoryBarrier*                 pImageMemoryBarriers) override;


virtual void PostCallRecordCmdWaitEvents(
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
    const VkImageMemoryBarrier*                 pImageMemoryBarriers) override;


virtual bool PreCallValidateCmdPipelineBarrier(
    VkCommandBuffer                             commandBuffer,
    VkPipelineStageFlags                        srcStageMask,
    VkPipelineStageFlags                        dstStageMask,
    VkDependencyFlags                           dependencyFlags,
    uint32_t                                    memoryBarrierCount,
    const VkMemoryBarrier*                      pMemoryBarriers,
    uint32_t                                    bufferMemoryBarrierCount,
    const VkBufferMemoryBarrier*                pBufferMemoryBarriers,
    uint32_t                                    imageMemoryBarrierCount,
    const VkImageMemoryBarrier*                 pImageMemoryBarriers) const override;


virtual void PreCallRecordCmdPipelineBarrier(
    VkCommandBuffer                             commandBuffer,
    VkPipelineStageFlags                        srcStageMask,
    VkPipelineStageFlags                        dstStageMask,
    VkDependencyFlags                           dependencyFlags,
    uint32_t                                    memoryBarrierCount,
    const VkMemoryBarrier*                      pMemoryBarriers,
    uint32_t                                    bufferMemoryBarrierCount,
    const VkBufferMemoryBarrier*                pBufferMemoryBarriers,
    uint32_t                                    imageMemoryBarrierCount,
    const VkImageMemoryBarrier*                 pImageMemoryBarriers) override;


virtual void PostCallRecordCmdPipelineBarrier(
    VkCommandBuffer                             commandBuffer,
    VkPipelineStageFlags                        srcStageMask,
    VkPipelineStageFlags                        dstStageMask,
    VkDependencyFlags                           dependencyFlags,
    uint32_t                                    memoryBarrierCount,
    const VkMemoryBarrier*                      pMemoryBarriers,
    uint32_t                                    bufferMemoryBarrierCount,
    const VkBufferMemoryBarrier*                pBufferMemoryBarriers,
    uint32_t                                    imageMemoryBarrierCount,
    const VkImageMemoryBarrier*                 pImageMemoryBarriers) override;


virtual bool PreCallValidateCmdBeginQuery(
    VkCommandBuffer                             commandBuffer,
    VkQueryPool                                 queryPool,
    uint32_t                                    query,
    VkQueryControlFlags                         flags) const override;


virtual void PreCallRecordCmdBeginQuery(
    VkCommandBuffer                             commandBuffer,
    VkQueryPool                                 queryPool,
    uint32_t                                    query,
    VkQueryControlFlags                         flags) override;


virtual void PostCallRecordCmdBeginQuery(
    VkCommandBuffer                             commandBuffer,
    VkQueryPool                                 queryPool,
    uint32_t                                    query,
    VkQueryControlFlags                         flags) override;


virtual bool PreCallValidateCmdEndQuery(
    VkCommandBuffer                             commandBuffer,
    VkQueryPool                                 queryPool,
    uint32_t                                    query) const override;


virtual void PreCallRecordCmdEndQuery(
    VkCommandBuffer                             commandBuffer,
    VkQueryPool                                 queryPool,
    uint32_t                                    query) override;


virtual void PostCallRecordCmdEndQuery(
    VkCommandBuffer                             commandBuffer,
    VkQueryPool                                 queryPool,
    uint32_t                                    query) override;


virtual bool PreCallValidateCmdResetQueryPool(
    VkCommandBuffer                             commandBuffer,
    VkQueryPool                                 queryPool,
    uint32_t                                    firstQuery,
    uint32_t                                    queryCount) const override;


virtual void PreCallRecordCmdResetQueryPool(
    VkCommandBuffer                             commandBuffer,
    VkQueryPool                                 queryPool,
    uint32_t                                    firstQuery,
    uint32_t                                    queryCount) override;


virtual void PostCallRecordCmdResetQueryPool(
    VkCommandBuffer                             commandBuffer,
    VkQueryPool                                 queryPool,
    uint32_t                                    firstQuery,
    uint32_t                                    queryCount) override;


virtual bool PreCallValidateCmdWriteTimestamp(
    VkCommandBuffer                             commandBuffer,
    VkPipelineStageFlagBits                     pipelineStage,
    VkQueryPool                                 queryPool,
    uint32_t                                    query) const override;


virtual void PreCallRecordCmdWriteTimestamp(
    VkCommandBuffer                             commandBuffer,
    VkPipelineStageFlagBits                     pipelineStage,
    VkQueryPool                                 queryPool,
    uint32_t                                    query) override;


virtual void PostCallRecordCmdWriteTimestamp(
    VkCommandBuffer                             commandBuffer,
    VkPipelineStageFlagBits                     pipelineStage,
    VkQueryPool                                 queryPool,
    uint32_t                                    query) override;


virtual bool PreCallValidateCmdCopyQueryPoolResults(
    VkCommandBuffer                             commandBuffer,
    VkQueryPool                                 queryPool,
    uint32_t                                    firstQuery,
    uint32_t                                    queryCount,
    VkBuffer                                    dstBuffer,
    VkDeviceSize                                dstOffset,
    VkDeviceSize                                stride,
    VkQueryResultFlags                          flags) const override;


virtual void PreCallRecordCmdCopyQueryPoolResults(
    VkCommandBuffer                             commandBuffer,
    VkQueryPool                                 queryPool,
    uint32_t                                    firstQuery,
    uint32_t                                    queryCount,
    VkBuffer                                    dstBuffer,
    VkDeviceSize                                dstOffset,
    VkDeviceSize                                stride,
    VkQueryResultFlags                          flags) override;


virtual void PostCallRecordCmdCopyQueryPoolResults(
    VkCommandBuffer                             commandBuffer,
    VkQueryPool                                 queryPool,
    uint32_t                                    firstQuery,
    uint32_t                                    queryCount,
    VkBuffer                                    dstBuffer,
    VkDeviceSize                                dstOffset,
    VkDeviceSize                                stride,
    VkQueryResultFlags                          flags) override;


virtual bool PreCallValidateCmdPushConstants(
    VkCommandBuffer                             commandBuffer,
    VkPipelineLayout                            layout,
    VkShaderStageFlags                          stageFlags,
    uint32_t                                    offset,
    uint32_t                                    size,
    const void*                                 pValues) const override;


virtual void PreCallRecordCmdPushConstants(
    VkCommandBuffer                             commandBuffer,
    VkPipelineLayout                            layout,
    VkShaderStageFlags                          stageFlags,
    uint32_t                                    offset,
    uint32_t                                    size,
    const void*                                 pValues) override;


virtual void PostCallRecordCmdPushConstants(
    VkCommandBuffer                             commandBuffer,
    VkPipelineLayout                            layout,
    VkShaderStageFlags                          stageFlags,
    uint32_t                                    offset,
    uint32_t                                    size,
    const void*                                 pValues) override;


virtual bool PreCallValidateCmdBeginRenderPass(
    VkCommandBuffer                             commandBuffer,
    const VkRenderPassBeginInfo*                pRenderPassBegin,
    VkSubpassContents                           contents) const override;


virtual void PreCallRecordCmdBeginRenderPass(
    VkCommandBuffer                             commandBuffer,
    const VkRenderPassBeginInfo*                pRenderPassBegin,
    VkSubpassContents                           contents) override;


virtual void PostCallRecordCmdBeginRenderPass(
    VkCommandBuffer                             commandBuffer,
    const VkRenderPassBeginInfo*                pRenderPassBegin,
    VkSubpassContents                           contents) override;


virtual bool PreCallValidateCmdNextSubpass(
    VkCommandBuffer                             commandBuffer,
    VkSubpassContents                           contents) const override;


virtual void PreCallRecordCmdNextSubpass(
    VkCommandBuffer                             commandBuffer,
    VkSubpassContents                           contents) override;


virtual void PostCallRecordCmdNextSubpass(
    VkCommandBuffer                             commandBuffer,
    VkSubpassContents                           contents) override;


virtual bool PreCallValidateCmdEndRenderPass(
    VkCommandBuffer                             commandBuffer) const override;


virtual void PreCallRecordCmdEndRenderPass(
    VkCommandBuffer                             commandBuffer) override;


virtual void PostCallRecordCmdEndRenderPass(
    VkCommandBuffer                             commandBuffer) override;


virtual bool PreCallValidateCmdExecuteCommands(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    commandBufferCount,
    const VkCommandBuffer*                      pCommandBuffers) const override;


virtual void PreCallRecordCmdExecuteCommands(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    commandBufferCount,
    const VkCommandBuffer*                      pCommandBuffers) override;


virtual void PostCallRecordCmdExecuteCommands(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    commandBufferCount,
    const VkCommandBuffer*                      pCommandBuffers) override;


virtual bool PreCallValidateBindBufferMemory2(
    VkDevice                                    device,
    uint32_t                                    bindInfoCount,
    const VkBindBufferMemoryInfo*               pBindInfos) const override;


virtual void PreCallRecordBindBufferMemory2(
    VkDevice                                    device,
    uint32_t                                    bindInfoCount,
    const VkBindBufferMemoryInfo*               pBindInfos) override;


virtual void PostCallRecordBindBufferMemory2(
    VkDevice                                    device,
    uint32_t                                    bindInfoCount,
    const VkBindBufferMemoryInfo*               pBindInfos,
    VkResult                                    result) override;


virtual bool PreCallValidateBindImageMemory2(
    VkDevice                                    device,
    uint32_t                                    bindInfoCount,
    const VkBindImageMemoryInfo*                pBindInfos) const override;


virtual void PreCallRecordBindImageMemory2(
    VkDevice                                    device,
    uint32_t                                    bindInfoCount,
    const VkBindImageMemoryInfo*                pBindInfos) override;


virtual void PostCallRecordBindImageMemory2(
    VkDevice                                    device,
    uint32_t                                    bindInfoCount,
    const VkBindImageMemoryInfo*                pBindInfos,
    VkResult                                    result) override;


virtual bool PreCallValidateGetDeviceGroupPeerMemoryFeatures(
    VkDevice                                    device,
    uint32_t                                    heapIndex,
    uint32_t                                    localDeviceIndex,
    uint32_t                                    remoteDeviceIndex,
    VkPeerMemoryFeatureFlags*                   pPeerMemoryFeatures) const override;


virtual void PreCallRecordGetDeviceGroupPeerMemoryFeatures(
    VkDevice                                    device,
    uint32_t                                    heapIndex,
    uint32_t                                    localDeviceIndex,
    uint32_t                                    remoteDeviceIndex,
    VkPeerMemoryFeatureFlags*                   pPeerMemoryFeatures) override;


virtual void PostCallRecordGetDeviceGroupPeerMemoryFeatures(
    VkDevice                                    device,
    uint32_t                                    heapIndex,
    uint32_t                                    localDeviceIndex,
    uint32_t                                    remoteDeviceIndex,
    VkPeerMemoryFeatureFlags*                   pPeerMemoryFeatures) override;


virtual bool PreCallValidateCmdSetDeviceMask(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    deviceMask) const override;


virtual void PreCallRecordCmdSetDeviceMask(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    deviceMask) override;


virtual void PostCallRecordCmdSetDeviceMask(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    deviceMask) override;


virtual bool PreCallValidateCmdDispatchBase(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    baseGroupX,
    uint32_t                                    baseGroupY,
    uint32_t                                    baseGroupZ,
    uint32_t                                    groupCountX,
    uint32_t                                    groupCountY,
    uint32_t                                    groupCountZ) const override;


virtual void PreCallRecordCmdDispatchBase(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    baseGroupX,
    uint32_t                                    baseGroupY,
    uint32_t                                    baseGroupZ,
    uint32_t                                    groupCountX,
    uint32_t                                    groupCountY,
    uint32_t                                    groupCountZ) override;


virtual void PostCallRecordCmdDispatchBase(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    baseGroupX,
    uint32_t                                    baseGroupY,
    uint32_t                                    baseGroupZ,
    uint32_t                                    groupCountX,
    uint32_t                                    groupCountY,
    uint32_t                                    groupCountZ) override;


virtual bool PreCallValidateEnumeratePhysicalDeviceGroups(
    VkInstance                                  instance,
    uint32_t*                                   pPhysicalDeviceGroupCount,
    VkPhysicalDeviceGroupProperties*            pPhysicalDeviceGroupProperties) const override;


virtual void PreCallRecordEnumeratePhysicalDeviceGroups(
    VkInstance                                  instance,
    uint32_t*                                   pPhysicalDeviceGroupCount,
    VkPhysicalDeviceGroupProperties*            pPhysicalDeviceGroupProperties) override;


virtual void PostCallRecordEnumeratePhysicalDeviceGroups(
    VkInstance                                  instance,
    uint32_t*                                   pPhysicalDeviceGroupCount,
    VkPhysicalDeviceGroupProperties*            pPhysicalDeviceGroupProperties,
    VkResult                                    result) override;


virtual bool PreCallValidateGetImageMemoryRequirements2(
    VkDevice                                    device,
    const VkImageMemoryRequirementsInfo2*       pInfo,
    VkMemoryRequirements2*                      pMemoryRequirements) const override;


virtual void PreCallRecordGetImageMemoryRequirements2(
    VkDevice                                    device,
    const VkImageMemoryRequirementsInfo2*       pInfo,
    VkMemoryRequirements2*                      pMemoryRequirements) override;


virtual void PostCallRecordGetImageMemoryRequirements2(
    VkDevice                                    device,
    const VkImageMemoryRequirementsInfo2*       pInfo,
    VkMemoryRequirements2*                      pMemoryRequirements) override;


virtual bool PreCallValidateGetBufferMemoryRequirements2(
    VkDevice                                    device,
    const VkBufferMemoryRequirementsInfo2*      pInfo,
    VkMemoryRequirements2*                      pMemoryRequirements) const override;


virtual void PreCallRecordGetBufferMemoryRequirements2(
    VkDevice                                    device,
    const VkBufferMemoryRequirementsInfo2*      pInfo,
    VkMemoryRequirements2*                      pMemoryRequirements) override;


virtual void PostCallRecordGetBufferMemoryRequirements2(
    VkDevice                                    device,
    const VkBufferMemoryRequirementsInfo2*      pInfo,
    VkMemoryRequirements2*                      pMemoryRequirements) override;


virtual bool PreCallValidateGetImageSparseMemoryRequirements2(
    VkDevice                                    device,
    const VkImageSparseMemoryRequirementsInfo2* pInfo,
    uint32_t*                                   pSparseMemoryRequirementCount,
    VkSparseImageMemoryRequirements2*           pSparseMemoryRequirements) const override;


virtual void PreCallRecordGetImageSparseMemoryRequirements2(
    VkDevice                                    device,
    const VkImageSparseMemoryRequirementsInfo2* pInfo,
    uint32_t*                                   pSparseMemoryRequirementCount,
    VkSparseImageMemoryRequirements2*           pSparseMemoryRequirements) override;


virtual void PostCallRecordGetImageSparseMemoryRequirements2(
    VkDevice                                    device,
    const VkImageSparseMemoryRequirementsInfo2* pInfo,
    uint32_t*                                   pSparseMemoryRequirementCount,
    VkSparseImageMemoryRequirements2*           pSparseMemoryRequirements) override;


virtual bool PreCallValidateGetPhysicalDeviceFeatures2(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceFeatures2*                  pFeatures) const override;


virtual void PreCallRecordGetPhysicalDeviceFeatures2(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceFeatures2*                  pFeatures) override;


virtual void PostCallRecordGetPhysicalDeviceFeatures2(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceFeatures2*                  pFeatures) override;


virtual bool PreCallValidateGetPhysicalDeviceProperties2(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceProperties2*                pProperties) const override;


virtual void PreCallRecordGetPhysicalDeviceProperties2(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceProperties2*                pProperties) override;


virtual void PostCallRecordGetPhysicalDeviceProperties2(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceProperties2*                pProperties) override;


virtual bool PreCallValidateGetPhysicalDeviceFormatProperties2(
    VkPhysicalDevice                            physicalDevice,
    VkFormat                                    format,
    VkFormatProperties2*                        pFormatProperties) const override;


virtual void PreCallRecordGetPhysicalDeviceFormatProperties2(
    VkPhysicalDevice                            physicalDevice,
    VkFormat                                    format,
    VkFormatProperties2*                        pFormatProperties) override;


virtual void PostCallRecordGetPhysicalDeviceFormatProperties2(
    VkPhysicalDevice                            physicalDevice,
    VkFormat                                    format,
    VkFormatProperties2*                        pFormatProperties) override;


virtual bool PreCallValidateGetPhysicalDeviceImageFormatProperties2(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceImageFormatInfo2*     pImageFormatInfo,
    VkImageFormatProperties2*                   pImageFormatProperties) const override;


virtual void PreCallRecordGetPhysicalDeviceImageFormatProperties2(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceImageFormatInfo2*     pImageFormatInfo,
    VkImageFormatProperties2*                   pImageFormatProperties) override;


virtual void PostCallRecordGetPhysicalDeviceImageFormatProperties2(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceImageFormatInfo2*     pImageFormatInfo,
    VkImageFormatProperties2*                   pImageFormatProperties,
    VkResult                                    result) override;


virtual bool PreCallValidateGetPhysicalDeviceQueueFamilyProperties2(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pQueueFamilyPropertyCount,
    VkQueueFamilyProperties2*                   pQueueFamilyProperties) const override;


virtual void PreCallRecordGetPhysicalDeviceQueueFamilyProperties2(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pQueueFamilyPropertyCount,
    VkQueueFamilyProperties2*                   pQueueFamilyProperties) override;


virtual void PostCallRecordGetPhysicalDeviceQueueFamilyProperties2(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pQueueFamilyPropertyCount,
    VkQueueFamilyProperties2*                   pQueueFamilyProperties) override;


virtual bool PreCallValidateGetPhysicalDeviceMemoryProperties2(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceMemoryProperties2*          pMemoryProperties) const override;


virtual void PreCallRecordGetPhysicalDeviceMemoryProperties2(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceMemoryProperties2*          pMemoryProperties) override;


virtual void PostCallRecordGetPhysicalDeviceMemoryProperties2(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceMemoryProperties2*          pMemoryProperties) override;


virtual bool PreCallValidateGetPhysicalDeviceSparseImageFormatProperties2(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceSparseImageFormatInfo2* pFormatInfo,
    uint32_t*                                   pPropertyCount,
    VkSparseImageFormatProperties2*             pProperties) const override;


virtual void PreCallRecordGetPhysicalDeviceSparseImageFormatProperties2(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceSparseImageFormatInfo2* pFormatInfo,
    uint32_t*                                   pPropertyCount,
    VkSparseImageFormatProperties2*             pProperties) override;


virtual void PostCallRecordGetPhysicalDeviceSparseImageFormatProperties2(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceSparseImageFormatInfo2* pFormatInfo,
    uint32_t*                                   pPropertyCount,
    VkSparseImageFormatProperties2*             pProperties) override;


virtual bool PreCallValidateTrimCommandPool(
    VkDevice                                    device,
    VkCommandPool                               commandPool,
    VkCommandPoolTrimFlags                      flags) const override;


virtual void PreCallRecordTrimCommandPool(
    VkDevice                                    device,
    VkCommandPool                               commandPool,
    VkCommandPoolTrimFlags                      flags) override;


virtual void PostCallRecordTrimCommandPool(
    VkDevice                                    device,
    VkCommandPool                               commandPool,
    VkCommandPoolTrimFlags                      flags) override;


virtual bool PreCallValidateGetDeviceQueue2(
    VkDevice                                    device,
    const VkDeviceQueueInfo2*                   pQueueInfo,
    VkQueue*                                    pQueue) const override;


virtual void PreCallRecordGetDeviceQueue2(
    VkDevice                                    device,
    const VkDeviceQueueInfo2*                   pQueueInfo,
    VkQueue*                                    pQueue) override;


virtual void PostCallRecordGetDeviceQueue2(
    VkDevice                                    device,
    const VkDeviceQueueInfo2*                   pQueueInfo,
    VkQueue*                                    pQueue) override;


virtual bool PreCallValidateCreateSamplerYcbcrConversion(
    VkDevice                                    device,
    const VkSamplerYcbcrConversionCreateInfo*   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSamplerYcbcrConversion*                   pYcbcrConversion) const override;


virtual void PreCallRecordCreateSamplerYcbcrConversion(
    VkDevice                                    device,
    const VkSamplerYcbcrConversionCreateInfo*   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSamplerYcbcrConversion*                   pYcbcrConversion) override;


virtual void PostCallRecordCreateSamplerYcbcrConversion(
    VkDevice                                    device,
    const VkSamplerYcbcrConversionCreateInfo*   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSamplerYcbcrConversion*                   pYcbcrConversion,
    VkResult                                    result) override;


virtual bool PreCallValidateDestroySamplerYcbcrConversion(
    VkDevice                                    device,
    VkSamplerYcbcrConversion                    ycbcrConversion,
    const VkAllocationCallbacks*                pAllocator) const override;


virtual void PreCallRecordDestroySamplerYcbcrConversion(
    VkDevice                                    device,
    VkSamplerYcbcrConversion                    ycbcrConversion,
    const VkAllocationCallbacks*                pAllocator) override;


virtual void PostCallRecordDestroySamplerYcbcrConversion(
    VkDevice                                    device,
    VkSamplerYcbcrConversion                    ycbcrConversion,
    const VkAllocationCallbacks*                pAllocator) override;


virtual bool PreCallValidateCreateDescriptorUpdateTemplate(
    VkDevice                                    device,
    const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDescriptorUpdateTemplate*                 pDescriptorUpdateTemplate) const override;


virtual void PreCallRecordCreateDescriptorUpdateTemplate(
    VkDevice                                    device,
    const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDescriptorUpdateTemplate*                 pDescriptorUpdateTemplate) override;


virtual void PostCallRecordCreateDescriptorUpdateTemplate(
    VkDevice                                    device,
    const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDescriptorUpdateTemplate*                 pDescriptorUpdateTemplate,
    VkResult                                    result) override;


virtual bool PreCallValidateDestroyDescriptorUpdateTemplate(
    VkDevice                                    device,
    VkDescriptorUpdateTemplate                  descriptorUpdateTemplate,
    const VkAllocationCallbacks*                pAllocator) const override;


virtual void PreCallRecordDestroyDescriptorUpdateTemplate(
    VkDevice                                    device,
    VkDescriptorUpdateTemplate                  descriptorUpdateTemplate,
    const VkAllocationCallbacks*                pAllocator) override;


virtual void PostCallRecordDestroyDescriptorUpdateTemplate(
    VkDevice                                    device,
    VkDescriptorUpdateTemplate                  descriptorUpdateTemplate,
    const VkAllocationCallbacks*                pAllocator) override;


virtual bool PreCallValidateUpdateDescriptorSetWithTemplate(
    VkDevice                                    device,
    VkDescriptorSet                             descriptorSet,
    VkDescriptorUpdateTemplate                  descriptorUpdateTemplate,
    const void*                                 pData) const override;


virtual void PreCallRecordUpdateDescriptorSetWithTemplate(
    VkDevice                                    device,
    VkDescriptorSet                             descriptorSet,
    VkDescriptorUpdateTemplate                  descriptorUpdateTemplate,
    const void*                                 pData) override;


virtual void PostCallRecordUpdateDescriptorSetWithTemplate(
    VkDevice                                    device,
    VkDescriptorSet                             descriptorSet,
    VkDescriptorUpdateTemplate                  descriptorUpdateTemplate,
    const void*                                 pData) override;


virtual bool PreCallValidateGetPhysicalDeviceExternalBufferProperties(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceExternalBufferInfo*   pExternalBufferInfo,
    VkExternalBufferProperties*                 pExternalBufferProperties) const override;


virtual void PreCallRecordGetPhysicalDeviceExternalBufferProperties(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceExternalBufferInfo*   pExternalBufferInfo,
    VkExternalBufferProperties*                 pExternalBufferProperties) override;


virtual void PostCallRecordGetPhysicalDeviceExternalBufferProperties(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceExternalBufferInfo*   pExternalBufferInfo,
    VkExternalBufferProperties*                 pExternalBufferProperties) override;


virtual bool PreCallValidateGetPhysicalDeviceExternalFenceProperties(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceExternalFenceInfo*    pExternalFenceInfo,
    VkExternalFenceProperties*                  pExternalFenceProperties) const override;


virtual void PreCallRecordGetPhysicalDeviceExternalFenceProperties(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceExternalFenceInfo*    pExternalFenceInfo,
    VkExternalFenceProperties*                  pExternalFenceProperties) override;


virtual void PostCallRecordGetPhysicalDeviceExternalFenceProperties(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceExternalFenceInfo*    pExternalFenceInfo,
    VkExternalFenceProperties*                  pExternalFenceProperties) override;


virtual bool PreCallValidateGetPhysicalDeviceExternalSemaphoreProperties(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceExternalSemaphoreInfo* pExternalSemaphoreInfo,
    VkExternalSemaphoreProperties*              pExternalSemaphoreProperties) const override;


virtual void PreCallRecordGetPhysicalDeviceExternalSemaphoreProperties(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceExternalSemaphoreInfo* pExternalSemaphoreInfo,
    VkExternalSemaphoreProperties*              pExternalSemaphoreProperties) override;


virtual void PostCallRecordGetPhysicalDeviceExternalSemaphoreProperties(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceExternalSemaphoreInfo* pExternalSemaphoreInfo,
    VkExternalSemaphoreProperties*              pExternalSemaphoreProperties) override;


virtual bool PreCallValidateGetDescriptorSetLayoutSupport(
    VkDevice                                    device,
    const VkDescriptorSetLayoutCreateInfo*      pCreateInfo,
    VkDescriptorSetLayoutSupport*               pSupport) const override;


virtual void PreCallRecordGetDescriptorSetLayoutSupport(
    VkDevice                                    device,
    const VkDescriptorSetLayoutCreateInfo*      pCreateInfo,
    VkDescriptorSetLayoutSupport*               pSupport) override;


virtual void PostCallRecordGetDescriptorSetLayoutSupport(
    VkDevice                                    device,
    const VkDescriptorSetLayoutCreateInfo*      pCreateInfo,
    VkDescriptorSetLayoutSupport*               pSupport) override;


virtual bool PreCallValidateCmdDrawIndirectCount(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkBuffer                                    countBuffer,
    VkDeviceSize                                countBufferOffset,
    uint32_t                                    maxDrawCount,
    uint32_t                                    stride) const override;


virtual void PreCallRecordCmdDrawIndirectCount(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkBuffer                                    countBuffer,
    VkDeviceSize                                countBufferOffset,
    uint32_t                                    maxDrawCount,
    uint32_t                                    stride) override;


virtual void PostCallRecordCmdDrawIndirectCount(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkBuffer                                    countBuffer,
    VkDeviceSize                                countBufferOffset,
    uint32_t                                    maxDrawCount,
    uint32_t                                    stride) override;


virtual bool PreCallValidateCmdDrawIndexedIndirectCount(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkBuffer                                    countBuffer,
    VkDeviceSize                                countBufferOffset,
    uint32_t                                    maxDrawCount,
    uint32_t                                    stride) const override;


virtual void PreCallRecordCmdDrawIndexedIndirectCount(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkBuffer                                    countBuffer,
    VkDeviceSize                                countBufferOffset,
    uint32_t                                    maxDrawCount,
    uint32_t                                    stride) override;


virtual void PostCallRecordCmdDrawIndexedIndirectCount(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkBuffer                                    countBuffer,
    VkDeviceSize                                countBufferOffset,
    uint32_t                                    maxDrawCount,
    uint32_t                                    stride) override;


virtual bool PreCallValidateCreateRenderPass2(
    VkDevice                                    device,
    const VkRenderPassCreateInfo2*              pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkRenderPass*                               pRenderPass) const override;


virtual void PreCallRecordCreateRenderPass2(
    VkDevice                                    device,
    const VkRenderPassCreateInfo2*              pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkRenderPass*                               pRenderPass) override;


virtual void PostCallRecordCreateRenderPass2(
    VkDevice                                    device,
    const VkRenderPassCreateInfo2*              pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkRenderPass*                               pRenderPass,
    VkResult                                    result) override;


virtual bool PreCallValidateCmdBeginRenderPass2(
    VkCommandBuffer                             commandBuffer,
    const VkRenderPassBeginInfo*                pRenderPassBegin,
    const VkSubpassBeginInfo*                   pSubpassBeginInfo) const override;


virtual void PreCallRecordCmdBeginRenderPass2(
    VkCommandBuffer                             commandBuffer,
    const VkRenderPassBeginInfo*                pRenderPassBegin,
    const VkSubpassBeginInfo*                   pSubpassBeginInfo) override;


virtual void PostCallRecordCmdBeginRenderPass2(
    VkCommandBuffer                             commandBuffer,
    const VkRenderPassBeginInfo*                pRenderPassBegin,
    const VkSubpassBeginInfo*                   pSubpassBeginInfo) override;


virtual bool PreCallValidateCmdNextSubpass2(
    VkCommandBuffer                             commandBuffer,
    const VkSubpassBeginInfo*                   pSubpassBeginInfo,
    const VkSubpassEndInfo*                     pSubpassEndInfo) const override;


virtual void PreCallRecordCmdNextSubpass2(
    VkCommandBuffer                             commandBuffer,
    const VkSubpassBeginInfo*                   pSubpassBeginInfo,
    const VkSubpassEndInfo*                     pSubpassEndInfo) override;


virtual void PostCallRecordCmdNextSubpass2(
    VkCommandBuffer                             commandBuffer,
    const VkSubpassBeginInfo*                   pSubpassBeginInfo,
    const VkSubpassEndInfo*                     pSubpassEndInfo) override;


virtual bool PreCallValidateCmdEndRenderPass2(
    VkCommandBuffer                             commandBuffer,
    const VkSubpassEndInfo*                     pSubpassEndInfo) const override;


virtual void PreCallRecordCmdEndRenderPass2(
    VkCommandBuffer                             commandBuffer,
    const VkSubpassEndInfo*                     pSubpassEndInfo) override;


virtual void PostCallRecordCmdEndRenderPass2(
    VkCommandBuffer                             commandBuffer,
    const VkSubpassEndInfo*                     pSubpassEndInfo) override;


virtual bool PreCallValidateResetQueryPool(
    VkDevice                                    device,
    VkQueryPool                                 queryPool,
    uint32_t                                    firstQuery,
    uint32_t                                    queryCount) const override;


virtual void PreCallRecordResetQueryPool(
    VkDevice                                    device,
    VkQueryPool                                 queryPool,
    uint32_t                                    firstQuery,
    uint32_t                                    queryCount) override;


virtual void PostCallRecordResetQueryPool(
    VkDevice                                    device,
    VkQueryPool                                 queryPool,
    uint32_t                                    firstQuery,
    uint32_t                                    queryCount) override;


virtual bool PreCallValidateGetSemaphoreCounterValue(
    VkDevice                                    device,
    VkSemaphore                                 semaphore,
    uint64_t*                                   pValue) const override;


virtual void PreCallRecordGetSemaphoreCounterValue(
    VkDevice                                    device,
    VkSemaphore                                 semaphore,
    uint64_t*                                   pValue) override;


virtual void PostCallRecordGetSemaphoreCounterValue(
    VkDevice                                    device,
    VkSemaphore                                 semaphore,
    uint64_t*                                   pValue,
    VkResult                                    result) override;


virtual bool PreCallValidateWaitSemaphores(
    VkDevice                                    device,
    const VkSemaphoreWaitInfo*                  pWaitInfo,
    uint64_t                                    timeout) const override;


virtual void PreCallRecordWaitSemaphores(
    VkDevice                                    device,
    const VkSemaphoreWaitInfo*                  pWaitInfo,
    uint64_t                                    timeout) override;


virtual void PostCallRecordWaitSemaphores(
    VkDevice                                    device,
    const VkSemaphoreWaitInfo*                  pWaitInfo,
    uint64_t                                    timeout,
    VkResult                                    result) override;


virtual bool PreCallValidateSignalSemaphore(
    VkDevice                                    device,
    const VkSemaphoreSignalInfo*                pSignalInfo) const override;


virtual void PreCallRecordSignalSemaphore(
    VkDevice                                    device,
    const VkSemaphoreSignalInfo*                pSignalInfo) override;


virtual void PostCallRecordSignalSemaphore(
    VkDevice                                    device,
    const VkSemaphoreSignalInfo*                pSignalInfo,
    VkResult                                    result) override;


virtual bool PreCallValidateGetBufferDeviceAddress(
    VkDevice                                    device,
    const VkBufferDeviceAddressInfo*            pInfo) const override;


virtual void PreCallRecordGetBufferDeviceAddress(
    VkDevice                                    device,
    const VkBufferDeviceAddressInfo*            pInfo) override;


virtual void PostCallRecordGetBufferDeviceAddress(
    VkDevice                                    device,
    const VkBufferDeviceAddressInfo*            pInfo,
    VkResult                                    result) override;


virtual bool PreCallValidateGetBufferOpaqueCaptureAddress(
    VkDevice                                    device,
    const VkBufferDeviceAddressInfo*            pInfo) const override;


virtual void PreCallRecordGetBufferOpaqueCaptureAddress(
    VkDevice                                    device,
    const VkBufferDeviceAddressInfo*            pInfo) override;


virtual void PostCallRecordGetBufferOpaqueCaptureAddress(
    VkDevice                                    device,
    const VkBufferDeviceAddressInfo*            pInfo) override;


virtual bool PreCallValidateGetDeviceMemoryOpaqueCaptureAddress(
    VkDevice                                    device,
    const VkDeviceMemoryOpaqueCaptureAddressInfo* pInfo) const override;


virtual void PreCallRecordGetDeviceMemoryOpaqueCaptureAddress(
    VkDevice                                    device,
    const VkDeviceMemoryOpaqueCaptureAddressInfo* pInfo) override;


virtual void PostCallRecordGetDeviceMemoryOpaqueCaptureAddress(
    VkDevice                                    device,
    const VkDeviceMemoryOpaqueCaptureAddressInfo* pInfo) override;


virtual bool PreCallValidateDestroySurfaceKHR(
    VkInstance                                  instance,
    VkSurfaceKHR                                surface,
    const VkAllocationCallbacks*                pAllocator) const override;


virtual void PreCallRecordDestroySurfaceKHR(
    VkInstance                                  instance,
    VkSurfaceKHR                                surface,
    const VkAllocationCallbacks*                pAllocator) override;


virtual void PostCallRecordDestroySurfaceKHR(
    VkInstance                                  instance,
    VkSurfaceKHR                                surface,
    const VkAllocationCallbacks*                pAllocator) override;


virtual bool PreCallValidateGetPhysicalDeviceSurfaceSupportKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    queueFamilyIndex,
    VkSurfaceKHR                                surface,
    VkBool32*                                   pSupported) const override;


virtual void PreCallRecordGetPhysicalDeviceSurfaceSupportKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    queueFamilyIndex,
    VkSurfaceKHR                                surface,
    VkBool32*                                   pSupported) override;


virtual void PostCallRecordGetPhysicalDeviceSurfaceSupportKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    queueFamilyIndex,
    VkSurfaceKHR                                surface,
    VkBool32*                                   pSupported,
    VkResult                                    result) override;


virtual bool PreCallValidateGetPhysicalDeviceSurfaceCapabilitiesKHR(
    VkPhysicalDevice                            physicalDevice,
    VkSurfaceKHR                                surface,
    VkSurfaceCapabilitiesKHR*                   pSurfaceCapabilities) const override;


virtual void PreCallRecordGetPhysicalDeviceSurfaceCapabilitiesKHR(
    VkPhysicalDevice                            physicalDevice,
    VkSurfaceKHR                                surface,
    VkSurfaceCapabilitiesKHR*                   pSurfaceCapabilities) override;


virtual void PostCallRecordGetPhysicalDeviceSurfaceCapabilitiesKHR(
    VkPhysicalDevice                            physicalDevice,
    VkSurfaceKHR                                surface,
    VkSurfaceCapabilitiesKHR*                   pSurfaceCapabilities,
    VkResult                                    result) override;


virtual bool PreCallValidateGetPhysicalDeviceSurfaceFormatsKHR(
    VkPhysicalDevice                            physicalDevice,
    VkSurfaceKHR                                surface,
    uint32_t*                                   pSurfaceFormatCount,
    VkSurfaceFormatKHR*                         pSurfaceFormats) const override;


virtual void PreCallRecordGetPhysicalDeviceSurfaceFormatsKHR(
    VkPhysicalDevice                            physicalDevice,
    VkSurfaceKHR                                surface,
    uint32_t*                                   pSurfaceFormatCount,
    VkSurfaceFormatKHR*                         pSurfaceFormats) override;


virtual void PostCallRecordGetPhysicalDeviceSurfaceFormatsKHR(
    VkPhysicalDevice                            physicalDevice,
    VkSurfaceKHR                                surface,
    uint32_t*                                   pSurfaceFormatCount,
    VkSurfaceFormatKHR*                         pSurfaceFormats,
    VkResult                                    result) override;


virtual bool PreCallValidateGetPhysicalDeviceSurfacePresentModesKHR(
    VkPhysicalDevice                            physicalDevice,
    VkSurfaceKHR                                surface,
    uint32_t*                                   pPresentModeCount,
    VkPresentModeKHR*                           pPresentModes) const override;


virtual void PreCallRecordGetPhysicalDeviceSurfacePresentModesKHR(
    VkPhysicalDevice                            physicalDevice,
    VkSurfaceKHR                                surface,
    uint32_t*                                   pPresentModeCount,
    VkPresentModeKHR*                           pPresentModes) override;


virtual void PostCallRecordGetPhysicalDeviceSurfacePresentModesKHR(
    VkPhysicalDevice                            physicalDevice,
    VkSurfaceKHR                                surface,
    uint32_t*                                   pPresentModeCount,
    VkPresentModeKHR*                           pPresentModes,
    VkResult                                    result) override;


virtual bool PreCallValidateCreateSwapchainKHR(
    VkDevice                                    device,
    const VkSwapchainCreateInfoKHR*             pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSwapchainKHR*                             pSwapchain) const override;


virtual void PreCallRecordCreateSwapchainKHR(
    VkDevice                                    device,
    const VkSwapchainCreateInfoKHR*             pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSwapchainKHR*                             pSwapchain) override;


virtual void PostCallRecordCreateSwapchainKHR(
    VkDevice                                    device,
    const VkSwapchainCreateInfoKHR*             pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSwapchainKHR*                             pSwapchain,
    VkResult                                    result) override;


virtual bool PreCallValidateDestroySwapchainKHR(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    const VkAllocationCallbacks*                pAllocator) const override;


virtual void PreCallRecordDestroySwapchainKHR(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    const VkAllocationCallbacks*                pAllocator) override;


virtual void PostCallRecordDestroySwapchainKHR(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    const VkAllocationCallbacks*                pAllocator) override;


virtual bool PreCallValidateGetSwapchainImagesKHR(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    uint32_t*                                   pSwapchainImageCount,
    VkImage*                                    pSwapchainImages) const override;


virtual void PreCallRecordGetSwapchainImagesKHR(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    uint32_t*                                   pSwapchainImageCount,
    VkImage*                                    pSwapchainImages) override;


virtual void PostCallRecordGetSwapchainImagesKHR(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    uint32_t*                                   pSwapchainImageCount,
    VkImage*                                    pSwapchainImages,
    VkResult                                    result) override;


virtual bool PreCallValidateAcquireNextImageKHR(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    uint64_t                                    timeout,
    VkSemaphore                                 semaphore,
    VkFence                                     fence,
    uint32_t*                                   pImageIndex) const override;


virtual void PreCallRecordAcquireNextImageKHR(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    uint64_t                                    timeout,
    VkSemaphore                                 semaphore,
    VkFence                                     fence,
    uint32_t*                                   pImageIndex) override;


virtual void PostCallRecordAcquireNextImageKHR(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    uint64_t                                    timeout,
    VkSemaphore                                 semaphore,
    VkFence                                     fence,
    uint32_t*                                   pImageIndex,
    VkResult                                    result) override;


virtual bool PreCallValidateQueuePresentKHR(
    VkQueue                                     queue,
    const VkPresentInfoKHR*                     pPresentInfo) const override;


virtual void PreCallRecordQueuePresentKHR(
    VkQueue                                     queue,
    const VkPresentInfoKHR*                     pPresentInfo) override;


virtual void PostCallRecordQueuePresentKHR(
    VkQueue                                     queue,
    const VkPresentInfoKHR*                     pPresentInfo,
    VkResult                                    result) override;


virtual bool PreCallValidateGetDeviceGroupPresentCapabilitiesKHR(
    VkDevice                                    device,
    VkDeviceGroupPresentCapabilitiesKHR*        pDeviceGroupPresentCapabilities) const override;


virtual void PreCallRecordGetDeviceGroupPresentCapabilitiesKHR(
    VkDevice                                    device,
    VkDeviceGroupPresentCapabilitiesKHR*        pDeviceGroupPresentCapabilities) override;


virtual void PostCallRecordGetDeviceGroupPresentCapabilitiesKHR(
    VkDevice                                    device,
    VkDeviceGroupPresentCapabilitiesKHR*        pDeviceGroupPresentCapabilities,
    VkResult                                    result) override;


virtual bool PreCallValidateGetDeviceGroupSurfacePresentModesKHR(
    VkDevice                                    device,
    VkSurfaceKHR                                surface,
    VkDeviceGroupPresentModeFlagsKHR*           pModes) const override;


virtual void PreCallRecordGetDeviceGroupSurfacePresentModesKHR(
    VkDevice                                    device,
    VkSurfaceKHR                                surface,
    VkDeviceGroupPresentModeFlagsKHR*           pModes) override;


virtual void PostCallRecordGetDeviceGroupSurfacePresentModesKHR(
    VkDevice                                    device,
    VkSurfaceKHR                                surface,
    VkDeviceGroupPresentModeFlagsKHR*           pModes,
    VkResult                                    result) override;


virtual bool PreCallValidateGetPhysicalDevicePresentRectanglesKHR(
    VkPhysicalDevice                            physicalDevice,
    VkSurfaceKHR                                surface,
    uint32_t*                                   pRectCount,
    VkRect2D*                                   pRects) const override;


virtual void PreCallRecordGetPhysicalDevicePresentRectanglesKHR(
    VkPhysicalDevice                            physicalDevice,
    VkSurfaceKHR                                surface,
    uint32_t*                                   pRectCount,
    VkRect2D*                                   pRects) override;


virtual void PostCallRecordGetPhysicalDevicePresentRectanglesKHR(
    VkPhysicalDevice                            physicalDevice,
    VkSurfaceKHR                                surface,
    uint32_t*                                   pRectCount,
    VkRect2D*                                   pRects,
    VkResult                                    result) override;


virtual bool PreCallValidateAcquireNextImage2KHR(
    VkDevice                                    device,
    const VkAcquireNextImageInfoKHR*            pAcquireInfo,
    uint32_t*                                   pImageIndex) const override;


virtual void PreCallRecordAcquireNextImage2KHR(
    VkDevice                                    device,
    const VkAcquireNextImageInfoKHR*            pAcquireInfo,
    uint32_t*                                   pImageIndex) override;


virtual void PostCallRecordAcquireNextImage2KHR(
    VkDevice                                    device,
    const VkAcquireNextImageInfoKHR*            pAcquireInfo,
    uint32_t*                                   pImageIndex,
    VkResult                                    result) override;


virtual bool PreCallValidateGetPhysicalDeviceDisplayPropertiesKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pPropertyCount,
    VkDisplayPropertiesKHR*                     pProperties) const override;


virtual void PreCallRecordGetPhysicalDeviceDisplayPropertiesKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pPropertyCount,
    VkDisplayPropertiesKHR*                     pProperties) override;


virtual void PostCallRecordGetPhysicalDeviceDisplayPropertiesKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pPropertyCount,
    VkDisplayPropertiesKHR*                     pProperties,
    VkResult                                    result) override;


virtual bool PreCallValidateGetPhysicalDeviceDisplayPlanePropertiesKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pPropertyCount,
    VkDisplayPlanePropertiesKHR*                pProperties) const override;


virtual void PreCallRecordGetPhysicalDeviceDisplayPlanePropertiesKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pPropertyCount,
    VkDisplayPlanePropertiesKHR*                pProperties) override;


virtual void PostCallRecordGetPhysicalDeviceDisplayPlanePropertiesKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pPropertyCount,
    VkDisplayPlanePropertiesKHR*                pProperties,
    VkResult                                    result) override;


virtual bool PreCallValidateGetDisplayPlaneSupportedDisplaysKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    planeIndex,
    uint32_t*                                   pDisplayCount,
    VkDisplayKHR*                               pDisplays) const override;


virtual void PreCallRecordGetDisplayPlaneSupportedDisplaysKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    planeIndex,
    uint32_t*                                   pDisplayCount,
    VkDisplayKHR*                               pDisplays) override;


virtual void PostCallRecordGetDisplayPlaneSupportedDisplaysKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    planeIndex,
    uint32_t*                                   pDisplayCount,
    VkDisplayKHR*                               pDisplays,
    VkResult                                    result) override;


virtual bool PreCallValidateGetDisplayModePropertiesKHR(
    VkPhysicalDevice                            physicalDevice,
    VkDisplayKHR                                display,
    uint32_t*                                   pPropertyCount,
    VkDisplayModePropertiesKHR*                 pProperties) const override;


virtual void PreCallRecordGetDisplayModePropertiesKHR(
    VkPhysicalDevice                            physicalDevice,
    VkDisplayKHR                                display,
    uint32_t*                                   pPropertyCount,
    VkDisplayModePropertiesKHR*                 pProperties) override;


virtual void PostCallRecordGetDisplayModePropertiesKHR(
    VkPhysicalDevice                            physicalDevice,
    VkDisplayKHR                                display,
    uint32_t*                                   pPropertyCount,
    VkDisplayModePropertiesKHR*                 pProperties,
    VkResult                                    result) override;


virtual bool PreCallValidateCreateDisplayModeKHR(
    VkPhysicalDevice                            physicalDevice,
    VkDisplayKHR                                display,
    const VkDisplayModeCreateInfoKHR*           pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDisplayModeKHR*                           pMode) const override;


virtual void PreCallRecordCreateDisplayModeKHR(
    VkPhysicalDevice                            physicalDevice,
    VkDisplayKHR                                display,
    const VkDisplayModeCreateInfoKHR*           pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDisplayModeKHR*                           pMode) override;


virtual void PostCallRecordCreateDisplayModeKHR(
    VkPhysicalDevice                            physicalDevice,
    VkDisplayKHR                                display,
    const VkDisplayModeCreateInfoKHR*           pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDisplayModeKHR*                           pMode,
    VkResult                                    result) override;


virtual bool PreCallValidateGetDisplayPlaneCapabilitiesKHR(
    VkPhysicalDevice                            physicalDevice,
    VkDisplayModeKHR                            mode,
    uint32_t                                    planeIndex,
    VkDisplayPlaneCapabilitiesKHR*              pCapabilities) const override;


virtual void PreCallRecordGetDisplayPlaneCapabilitiesKHR(
    VkPhysicalDevice                            physicalDevice,
    VkDisplayModeKHR                            mode,
    uint32_t                                    planeIndex,
    VkDisplayPlaneCapabilitiesKHR*              pCapabilities) override;


virtual void PostCallRecordGetDisplayPlaneCapabilitiesKHR(
    VkPhysicalDevice                            physicalDevice,
    VkDisplayModeKHR                            mode,
    uint32_t                                    planeIndex,
    VkDisplayPlaneCapabilitiesKHR*              pCapabilities,
    VkResult                                    result) override;


virtual bool PreCallValidateCreateDisplayPlaneSurfaceKHR(
    VkInstance                                  instance,
    const VkDisplaySurfaceCreateInfoKHR*        pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) const override;


virtual void PreCallRecordCreateDisplayPlaneSurfaceKHR(
    VkInstance                                  instance,
    const VkDisplaySurfaceCreateInfoKHR*        pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) override;


virtual void PostCallRecordCreateDisplayPlaneSurfaceKHR(
    VkInstance                                  instance,
    const VkDisplaySurfaceCreateInfoKHR*        pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface,
    VkResult                                    result) override;


virtual bool PreCallValidateCreateSharedSwapchainsKHR(
    VkDevice                                    device,
    uint32_t                                    swapchainCount,
    const VkSwapchainCreateInfoKHR*             pCreateInfos,
    const VkAllocationCallbacks*                pAllocator,
    VkSwapchainKHR*                             pSwapchains) const override;


virtual void PreCallRecordCreateSharedSwapchainsKHR(
    VkDevice                                    device,
    uint32_t                                    swapchainCount,
    const VkSwapchainCreateInfoKHR*             pCreateInfos,
    const VkAllocationCallbacks*                pAllocator,
    VkSwapchainKHR*                             pSwapchains) override;


virtual void PostCallRecordCreateSharedSwapchainsKHR(
    VkDevice                                    device,
    uint32_t                                    swapchainCount,
    const VkSwapchainCreateInfoKHR*             pCreateInfos,
    const VkAllocationCallbacks*                pAllocator,
    VkSwapchainKHR*                             pSwapchains,
    VkResult                                    result) override;


#ifdef VK_USE_PLATFORM_XLIB_KHR

virtual bool PreCallValidateCreateXlibSurfaceKHR(
    VkInstance                                  instance,
    const VkXlibSurfaceCreateInfoKHR*           pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) const override;


virtual void PreCallRecordCreateXlibSurfaceKHR(
    VkInstance                                  instance,
    const VkXlibSurfaceCreateInfoKHR*           pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) override;


virtual void PostCallRecordCreateXlibSurfaceKHR(
    VkInstance                                  instance,
    const VkXlibSurfaceCreateInfoKHR*           pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface,
    VkResult                                    result) override;


#endif // VK_USE_PLATFORM_XLIB_KHR

#ifdef VK_USE_PLATFORM_XLIB_KHR

virtual bool PreCallValidateGetPhysicalDeviceXlibPresentationSupportKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    queueFamilyIndex,
    Display*                                    dpy,
    VisualID                                    visualID) const override;


virtual void PreCallRecordGetPhysicalDeviceXlibPresentationSupportKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    queueFamilyIndex,
    Display*                                    dpy,
    VisualID                                    visualID) override;


virtual void PostCallRecordGetPhysicalDeviceXlibPresentationSupportKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    queueFamilyIndex,
    Display*                                    dpy,
    VisualID                                    visualID) override;


#endif // VK_USE_PLATFORM_XLIB_KHR

#ifdef VK_USE_PLATFORM_XCB_KHR

virtual bool PreCallValidateCreateXcbSurfaceKHR(
    VkInstance                                  instance,
    const VkXcbSurfaceCreateInfoKHR*            pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) const override;


virtual void PreCallRecordCreateXcbSurfaceKHR(
    VkInstance                                  instance,
    const VkXcbSurfaceCreateInfoKHR*            pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) override;


virtual void PostCallRecordCreateXcbSurfaceKHR(
    VkInstance                                  instance,
    const VkXcbSurfaceCreateInfoKHR*            pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface,
    VkResult                                    result) override;


#endif // VK_USE_PLATFORM_XCB_KHR

#ifdef VK_USE_PLATFORM_XCB_KHR

virtual bool PreCallValidateGetPhysicalDeviceXcbPresentationSupportKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    queueFamilyIndex,
    xcb_connection_t*                           connection,
    xcb_visualid_t                              visual_id) const override;


virtual void PreCallRecordGetPhysicalDeviceXcbPresentationSupportKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    queueFamilyIndex,
    xcb_connection_t*                           connection,
    xcb_visualid_t                              visual_id) override;


virtual void PostCallRecordGetPhysicalDeviceXcbPresentationSupportKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    queueFamilyIndex,
    xcb_connection_t*                           connection,
    xcb_visualid_t                              visual_id) override;


#endif // VK_USE_PLATFORM_XCB_KHR

#ifdef VK_USE_PLATFORM_WAYLAND_KHR

virtual bool PreCallValidateCreateWaylandSurfaceKHR(
    VkInstance                                  instance,
    const VkWaylandSurfaceCreateInfoKHR*        pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) const override;


virtual void PreCallRecordCreateWaylandSurfaceKHR(
    VkInstance                                  instance,
    const VkWaylandSurfaceCreateInfoKHR*        pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) override;


virtual void PostCallRecordCreateWaylandSurfaceKHR(
    VkInstance                                  instance,
    const VkWaylandSurfaceCreateInfoKHR*        pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface,
    VkResult                                    result) override;


#endif // VK_USE_PLATFORM_WAYLAND_KHR

#ifdef VK_USE_PLATFORM_WAYLAND_KHR

virtual bool PreCallValidateGetPhysicalDeviceWaylandPresentationSupportKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    queueFamilyIndex,
    struct wl_display*                          display) const override;


virtual void PreCallRecordGetPhysicalDeviceWaylandPresentationSupportKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    queueFamilyIndex,
    struct wl_display*                          display) override;


virtual void PostCallRecordGetPhysicalDeviceWaylandPresentationSupportKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    queueFamilyIndex,
    struct wl_display*                          display) override;


#endif // VK_USE_PLATFORM_WAYLAND_KHR

#ifdef VK_USE_PLATFORM_ANDROID_KHR

virtual bool PreCallValidateCreateAndroidSurfaceKHR(
    VkInstance                                  instance,
    const VkAndroidSurfaceCreateInfoKHR*        pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) const override;


virtual void PreCallRecordCreateAndroidSurfaceKHR(
    VkInstance                                  instance,
    const VkAndroidSurfaceCreateInfoKHR*        pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) override;


virtual void PostCallRecordCreateAndroidSurfaceKHR(
    VkInstance                                  instance,
    const VkAndroidSurfaceCreateInfoKHR*        pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface,
    VkResult                                    result) override;


#endif // VK_USE_PLATFORM_ANDROID_KHR

#ifdef VK_USE_PLATFORM_WIN32_KHR

virtual bool PreCallValidateCreateWin32SurfaceKHR(
    VkInstance                                  instance,
    const VkWin32SurfaceCreateInfoKHR*          pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) const override;


virtual void PreCallRecordCreateWin32SurfaceKHR(
    VkInstance                                  instance,
    const VkWin32SurfaceCreateInfoKHR*          pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) override;


virtual void PostCallRecordCreateWin32SurfaceKHR(
    VkInstance                                  instance,
    const VkWin32SurfaceCreateInfoKHR*          pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface,
    VkResult                                    result) override;


#endif // VK_USE_PLATFORM_WIN32_KHR

#ifdef VK_USE_PLATFORM_WIN32_KHR

virtual bool PreCallValidateGetPhysicalDeviceWin32PresentationSupportKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    queueFamilyIndex) const override;


virtual void PreCallRecordGetPhysicalDeviceWin32PresentationSupportKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    queueFamilyIndex) override;


virtual void PostCallRecordGetPhysicalDeviceWin32PresentationSupportKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    queueFamilyIndex) override;


#endif // VK_USE_PLATFORM_WIN32_KHR

virtual bool PreCallValidateGetPhysicalDeviceFeatures2KHR(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceFeatures2*                  pFeatures) const override;


virtual void PreCallRecordGetPhysicalDeviceFeatures2KHR(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceFeatures2*                  pFeatures) override;


virtual void PostCallRecordGetPhysicalDeviceFeatures2KHR(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceFeatures2*                  pFeatures) override;


virtual bool PreCallValidateGetPhysicalDeviceProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceProperties2*                pProperties) const override;


virtual void PreCallRecordGetPhysicalDeviceProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceProperties2*                pProperties) override;


virtual void PostCallRecordGetPhysicalDeviceProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceProperties2*                pProperties) override;


virtual bool PreCallValidateGetPhysicalDeviceFormatProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    VkFormat                                    format,
    VkFormatProperties2*                        pFormatProperties) const override;


virtual void PreCallRecordGetPhysicalDeviceFormatProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    VkFormat                                    format,
    VkFormatProperties2*                        pFormatProperties) override;


virtual void PostCallRecordGetPhysicalDeviceFormatProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    VkFormat                                    format,
    VkFormatProperties2*                        pFormatProperties) override;


virtual bool PreCallValidateGetPhysicalDeviceImageFormatProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceImageFormatInfo2*     pImageFormatInfo,
    VkImageFormatProperties2*                   pImageFormatProperties) const override;


virtual void PreCallRecordGetPhysicalDeviceImageFormatProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceImageFormatInfo2*     pImageFormatInfo,
    VkImageFormatProperties2*                   pImageFormatProperties) override;


virtual void PostCallRecordGetPhysicalDeviceImageFormatProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceImageFormatInfo2*     pImageFormatInfo,
    VkImageFormatProperties2*                   pImageFormatProperties,
    VkResult                                    result) override;


virtual bool PreCallValidateGetPhysicalDeviceQueueFamilyProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pQueueFamilyPropertyCount,
    VkQueueFamilyProperties2*                   pQueueFamilyProperties) const override;


virtual void PreCallRecordGetPhysicalDeviceQueueFamilyProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pQueueFamilyPropertyCount,
    VkQueueFamilyProperties2*                   pQueueFamilyProperties) override;


virtual void PostCallRecordGetPhysicalDeviceQueueFamilyProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pQueueFamilyPropertyCount,
    VkQueueFamilyProperties2*                   pQueueFamilyProperties) override;


virtual bool PreCallValidateGetPhysicalDeviceMemoryProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceMemoryProperties2*          pMemoryProperties) const override;


virtual void PreCallRecordGetPhysicalDeviceMemoryProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceMemoryProperties2*          pMemoryProperties) override;


virtual void PostCallRecordGetPhysicalDeviceMemoryProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceMemoryProperties2*          pMemoryProperties) override;


virtual bool PreCallValidateGetPhysicalDeviceSparseImageFormatProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceSparseImageFormatInfo2* pFormatInfo,
    uint32_t*                                   pPropertyCount,
    VkSparseImageFormatProperties2*             pProperties) const override;


virtual void PreCallRecordGetPhysicalDeviceSparseImageFormatProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceSparseImageFormatInfo2* pFormatInfo,
    uint32_t*                                   pPropertyCount,
    VkSparseImageFormatProperties2*             pProperties) override;


virtual void PostCallRecordGetPhysicalDeviceSparseImageFormatProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceSparseImageFormatInfo2* pFormatInfo,
    uint32_t*                                   pPropertyCount,
    VkSparseImageFormatProperties2*             pProperties) override;


virtual bool PreCallValidateGetDeviceGroupPeerMemoryFeaturesKHR(
    VkDevice                                    device,
    uint32_t                                    heapIndex,
    uint32_t                                    localDeviceIndex,
    uint32_t                                    remoteDeviceIndex,
    VkPeerMemoryFeatureFlags*                   pPeerMemoryFeatures) const override;


virtual void PreCallRecordGetDeviceGroupPeerMemoryFeaturesKHR(
    VkDevice                                    device,
    uint32_t                                    heapIndex,
    uint32_t                                    localDeviceIndex,
    uint32_t                                    remoteDeviceIndex,
    VkPeerMemoryFeatureFlags*                   pPeerMemoryFeatures) override;


virtual void PostCallRecordGetDeviceGroupPeerMemoryFeaturesKHR(
    VkDevice                                    device,
    uint32_t                                    heapIndex,
    uint32_t                                    localDeviceIndex,
    uint32_t                                    remoteDeviceIndex,
    VkPeerMemoryFeatureFlags*                   pPeerMemoryFeatures) override;


virtual bool PreCallValidateCmdSetDeviceMaskKHR(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    deviceMask) const override;


virtual void PreCallRecordCmdSetDeviceMaskKHR(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    deviceMask) override;


virtual void PostCallRecordCmdSetDeviceMaskKHR(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    deviceMask) override;


virtual bool PreCallValidateCmdDispatchBaseKHR(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    baseGroupX,
    uint32_t                                    baseGroupY,
    uint32_t                                    baseGroupZ,
    uint32_t                                    groupCountX,
    uint32_t                                    groupCountY,
    uint32_t                                    groupCountZ) const override;


virtual void PreCallRecordCmdDispatchBaseKHR(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    baseGroupX,
    uint32_t                                    baseGroupY,
    uint32_t                                    baseGroupZ,
    uint32_t                                    groupCountX,
    uint32_t                                    groupCountY,
    uint32_t                                    groupCountZ) override;


virtual void PostCallRecordCmdDispatchBaseKHR(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    baseGroupX,
    uint32_t                                    baseGroupY,
    uint32_t                                    baseGroupZ,
    uint32_t                                    groupCountX,
    uint32_t                                    groupCountY,
    uint32_t                                    groupCountZ) override;


virtual bool PreCallValidateTrimCommandPoolKHR(
    VkDevice                                    device,
    VkCommandPool                               commandPool,
    VkCommandPoolTrimFlags                      flags) const override;


virtual void PreCallRecordTrimCommandPoolKHR(
    VkDevice                                    device,
    VkCommandPool                               commandPool,
    VkCommandPoolTrimFlags                      flags) override;


virtual void PostCallRecordTrimCommandPoolKHR(
    VkDevice                                    device,
    VkCommandPool                               commandPool,
    VkCommandPoolTrimFlags                      flags) override;


virtual bool PreCallValidateEnumeratePhysicalDeviceGroupsKHR(
    VkInstance                                  instance,
    uint32_t*                                   pPhysicalDeviceGroupCount,
    VkPhysicalDeviceGroupProperties*            pPhysicalDeviceGroupProperties) const override;


virtual void PreCallRecordEnumeratePhysicalDeviceGroupsKHR(
    VkInstance                                  instance,
    uint32_t*                                   pPhysicalDeviceGroupCount,
    VkPhysicalDeviceGroupProperties*            pPhysicalDeviceGroupProperties) override;


virtual void PostCallRecordEnumeratePhysicalDeviceGroupsKHR(
    VkInstance                                  instance,
    uint32_t*                                   pPhysicalDeviceGroupCount,
    VkPhysicalDeviceGroupProperties*            pPhysicalDeviceGroupProperties,
    VkResult                                    result) override;


virtual bool PreCallValidateGetPhysicalDeviceExternalBufferPropertiesKHR(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceExternalBufferInfo*   pExternalBufferInfo,
    VkExternalBufferProperties*                 pExternalBufferProperties) const override;


virtual void PreCallRecordGetPhysicalDeviceExternalBufferPropertiesKHR(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceExternalBufferInfo*   pExternalBufferInfo,
    VkExternalBufferProperties*                 pExternalBufferProperties) override;


virtual void PostCallRecordGetPhysicalDeviceExternalBufferPropertiesKHR(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceExternalBufferInfo*   pExternalBufferInfo,
    VkExternalBufferProperties*                 pExternalBufferProperties) override;


#ifdef VK_USE_PLATFORM_WIN32_KHR

virtual bool PreCallValidateGetMemoryWin32HandleKHR(
    VkDevice                                    device,
    const VkMemoryGetWin32HandleInfoKHR*        pGetWin32HandleInfo,
    HANDLE*                                     pHandle) const override;


virtual void PreCallRecordGetMemoryWin32HandleKHR(
    VkDevice                                    device,
    const VkMemoryGetWin32HandleInfoKHR*        pGetWin32HandleInfo,
    HANDLE*                                     pHandle) override;


virtual void PostCallRecordGetMemoryWin32HandleKHR(
    VkDevice                                    device,
    const VkMemoryGetWin32HandleInfoKHR*        pGetWin32HandleInfo,
    HANDLE*                                     pHandle,
    VkResult                                    result) override;


#endif // VK_USE_PLATFORM_WIN32_KHR

#ifdef VK_USE_PLATFORM_WIN32_KHR

virtual bool PreCallValidateGetMemoryWin32HandlePropertiesKHR(
    VkDevice                                    device,
    VkExternalMemoryHandleTypeFlagBits          handleType,
    HANDLE                                      handle,
    VkMemoryWin32HandlePropertiesKHR*           pMemoryWin32HandleProperties) const override;


virtual void PreCallRecordGetMemoryWin32HandlePropertiesKHR(
    VkDevice                                    device,
    VkExternalMemoryHandleTypeFlagBits          handleType,
    HANDLE                                      handle,
    VkMemoryWin32HandlePropertiesKHR*           pMemoryWin32HandleProperties) override;


virtual void PostCallRecordGetMemoryWin32HandlePropertiesKHR(
    VkDevice                                    device,
    VkExternalMemoryHandleTypeFlagBits          handleType,
    HANDLE                                      handle,
    VkMemoryWin32HandlePropertiesKHR*           pMemoryWin32HandleProperties,
    VkResult                                    result) override;


#endif // VK_USE_PLATFORM_WIN32_KHR

virtual bool PreCallValidateGetMemoryFdKHR(
    VkDevice                                    device,
    const VkMemoryGetFdInfoKHR*                 pGetFdInfo,
    int*                                        pFd) const override;


virtual void PreCallRecordGetMemoryFdKHR(
    VkDevice                                    device,
    const VkMemoryGetFdInfoKHR*                 pGetFdInfo,
    int*                                        pFd) override;


virtual void PostCallRecordGetMemoryFdKHR(
    VkDevice                                    device,
    const VkMemoryGetFdInfoKHR*                 pGetFdInfo,
    int*                                        pFd,
    VkResult                                    result) override;


virtual bool PreCallValidateGetMemoryFdPropertiesKHR(
    VkDevice                                    device,
    VkExternalMemoryHandleTypeFlagBits          handleType,
    int                                         fd,
    VkMemoryFdPropertiesKHR*                    pMemoryFdProperties) const override;


virtual void PreCallRecordGetMemoryFdPropertiesKHR(
    VkDevice                                    device,
    VkExternalMemoryHandleTypeFlagBits          handleType,
    int                                         fd,
    VkMemoryFdPropertiesKHR*                    pMemoryFdProperties) override;


virtual void PostCallRecordGetMemoryFdPropertiesKHR(
    VkDevice                                    device,
    VkExternalMemoryHandleTypeFlagBits          handleType,
    int                                         fd,
    VkMemoryFdPropertiesKHR*                    pMemoryFdProperties,
    VkResult                                    result) override;


virtual bool PreCallValidateGetPhysicalDeviceExternalSemaphorePropertiesKHR(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceExternalSemaphoreInfo* pExternalSemaphoreInfo,
    VkExternalSemaphoreProperties*              pExternalSemaphoreProperties) const override;


virtual void PreCallRecordGetPhysicalDeviceExternalSemaphorePropertiesKHR(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceExternalSemaphoreInfo* pExternalSemaphoreInfo,
    VkExternalSemaphoreProperties*              pExternalSemaphoreProperties) override;


virtual void PostCallRecordGetPhysicalDeviceExternalSemaphorePropertiesKHR(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceExternalSemaphoreInfo* pExternalSemaphoreInfo,
    VkExternalSemaphoreProperties*              pExternalSemaphoreProperties) override;


#ifdef VK_USE_PLATFORM_WIN32_KHR

virtual bool PreCallValidateImportSemaphoreWin32HandleKHR(
    VkDevice                                    device,
    const VkImportSemaphoreWin32HandleInfoKHR*  pImportSemaphoreWin32HandleInfo) const override;


virtual void PreCallRecordImportSemaphoreWin32HandleKHR(
    VkDevice                                    device,
    const VkImportSemaphoreWin32HandleInfoKHR*  pImportSemaphoreWin32HandleInfo) override;


virtual void PostCallRecordImportSemaphoreWin32HandleKHR(
    VkDevice                                    device,
    const VkImportSemaphoreWin32HandleInfoKHR*  pImportSemaphoreWin32HandleInfo,
    VkResult                                    result) override;


#endif // VK_USE_PLATFORM_WIN32_KHR

#ifdef VK_USE_PLATFORM_WIN32_KHR

virtual bool PreCallValidateGetSemaphoreWin32HandleKHR(
    VkDevice                                    device,
    const VkSemaphoreGetWin32HandleInfoKHR*     pGetWin32HandleInfo,
    HANDLE*                                     pHandle) const override;


virtual void PreCallRecordGetSemaphoreWin32HandleKHR(
    VkDevice                                    device,
    const VkSemaphoreGetWin32HandleInfoKHR*     pGetWin32HandleInfo,
    HANDLE*                                     pHandle) override;


virtual void PostCallRecordGetSemaphoreWin32HandleKHR(
    VkDevice                                    device,
    const VkSemaphoreGetWin32HandleInfoKHR*     pGetWin32HandleInfo,
    HANDLE*                                     pHandle,
    VkResult                                    result) override;


#endif // VK_USE_PLATFORM_WIN32_KHR

virtual bool PreCallValidateImportSemaphoreFdKHR(
    VkDevice                                    device,
    const VkImportSemaphoreFdInfoKHR*           pImportSemaphoreFdInfo) const override;


virtual void PreCallRecordImportSemaphoreFdKHR(
    VkDevice                                    device,
    const VkImportSemaphoreFdInfoKHR*           pImportSemaphoreFdInfo) override;


virtual void PostCallRecordImportSemaphoreFdKHR(
    VkDevice                                    device,
    const VkImportSemaphoreFdInfoKHR*           pImportSemaphoreFdInfo,
    VkResult                                    result) override;


virtual bool PreCallValidateGetSemaphoreFdKHR(
    VkDevice                                    device,
    const VkSemaphoreGetFdInfoKHR*              pGetFdInfo,
    int*                                        pFd) const override;


virtual void PreCallRecordGetSemaphoreFdKHR(
    VkDevice                                    device,
    const VkSemaphoreGetFdInfoKHR*              pGetFdInfo,
    int*                                        pFd) override;


virtual void PostCallRecordGetSemaphoreFdKHR(
    VkDevice                                    device,
    const VkSemaphoreGetFdInfoKHR*              pGetFdInfo,
    int*                                        pFd,
    VkResult                                    result) override;


virtual bool PreCallValidateCmdPushDescriptorSetKHR(
    VkCommandBuffer                             commandBuffer,
    VkPipelineBindPoint                         pipelineBindPoint,
    VkPipelineLayout                            layout,
    uint32_t                                    set,
    uint32_t                                    descriptorWriteCount,
    const VkWriteDescriptorSet*                 pDescriptorWrites) const override;


virtual void PreCallRecordCmdPushDescriptorSetKHR(
    VkCommandBuffer                             commandBuffer,
    VkPipelineBindPoint                         pipelineBindPoint,
    VkPipelineLayout                            layout,
    uint32_t                                    set,
    uint32_t                                    descriptorWriteCount,
    const VkWriteDescriptorSet*                 pDescriptorWrites) override;


virtual void PostCallRecordCmdPushDescriptorSetKHR(
    VkCommandBuffer                             commandBuffer,
    VkPipelineBindPoint                         pipelineBindPoint,
    VkPipelineLayout                            layout,
    uint32_t                                    set,
    uint32_t                                    descriptorWriteCount,
    const VkWriteDescriptorSet*                 pDescriptorWrites) override;


virtual bool PreCallValidateCmdPushDescriptorSetWithTemplateKHR(
    VkCommandBuffer                             commandBuffer,
    VkDescriptorUpdateTemplate                  descriptorUpdateTemplate,
    VkPipelineLayout                            layout,
    uint32_t                                    set,
    const void*                                 pData) const override;


virtual void PreCallRecordCmdPushDescriptorSetWithTemplateKHR(
    VkCommandBuffer                             commandBuffer,
    VkDescriptorUpdateTemplate                  descriptorUpdateTemplate,
    VkPipelineLayout                            layout,
    uint32_t                                    set,
    const void*                                 pData) override;


virtual void PostCallRecordCmdPushDescriptorSetWithTemplateKHR(
    VkCommandBuffer                             commandBuffer,
    VkDescriptorUpdateTemplate                  descriptorUpdateTemplate,
    VkPipelineLayout                            layout,
    uint32_t                                    set,
    const void*                                 pData) override;


virtual bool PreCallValidateCreateDescriptorUpdateTemplateKHR(
    VkDevice                                    device,
    const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDescriptorUpdateTemplate*                 pDescriptorUpdateTemplate) const override;


virtual void PreCallRecordCreateDescriptorUpdateTemplateKHR(
    VkDevice                                    device,
    const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDescriptorUpdateTemplate*                 pDescriptorUpdateTemplate) override;


virtual void PostCallRecordCreateDescriptorUpdateTemplateKHR(
    VkDevice                                    device,
    const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDescriptorUpdateTemplate*                 pDescriptorUpdateTemplate,
    VkResult                                    result) override;


virtual bool PreCallValidateDestroyDescriptorUpdateTemplateKHR(
    VkDevice                                    device,
    VkDescriptorUpdateTemplate                  descriptorUpdateTemplate,
    const VkAllocationCallbacks*                pAllocator) const override;


virtual void PreCallRecordDestroyDescriptorUpdateTemplateKHR(
    VkDevice                                    device,
    VkDescriptorUpdateTemplate                  descriptorUpdateTemplate,
    const VkAllocationCallbacks*                pAllocator) override;


virtual void PostCallRecordDestroyDescriptorUpdateTemplateKHR(
    VkDevice                                    device,
    VkDescriptorUpdateTemplate                  descriptorUpdateTemplate,
    const VkAllocationCallbacks*                pAllocator) override;


virtual bool PreCallValidateUpdateDescriptorSetWithTemplateKHR(
    VkDevice                                    device,
    VkDescriptorSet                             descriptorSet,
    VkDescriptorUpdateTemplate                  descriptorUpdateTemplate,
    const void*                                 pData) const override;


virtual void PreCallRecordUpdateDescriptorSetWithTemplateKHR(
    VkDevice                                    device,
    VkDescriptorSet                             descriptorSet,
    VkDescriptorUpdateTemplate                  descriptorUpdateTemplate,
    const void*                                 pData) override;


virtual void PostCallRecordUpdateDescriptorSetWithTemplateKHR(
    VkDevice                                    device,
    VkDescriptorSet                             descriptorSet,
    VkDescriptorUpdateTemplate                  descriptorUpdateTemplate,
    const void*                                 pData) override;


virtual bool PreCallValidateCreateRenderPass2KHR(
    VkDevice                                    device,
    const VkRenderPassCreateInfo2*              pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkRenderPass*                               pRenderPass) const override;


virtual void PreCallRecordCreateRenderPass2KHR(
    VkDevice                                    device,
    const VkRenderPassCreateInfo2*              pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkRenderPass*                               pRenderPass) override;


virtual void PostCallRecordCreateRenderPass2KHR(
    VkDevice                                    device,
    const VkRenderPassCreateInfo2*              pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkRenderPass*                               pRenderPass,
    VkResult                                    result) override;


virtual bool PreCallValidateCmdBeginRenderPass2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkRenderPassBeginInfo*                pRenderPassBegin,
    const VkSubpassBeginInfo*                   pSubpassBeginInfo) const override;


virtual void PreCallRecordCmdBeginRenderPass2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkRenderPassBeginInfo*                pRenderPassBegin,
    const VkSubpassBeginInfo*                   pSubpassBeginInfo) override;


virtual void PostCallRecordCmdBeginRenderPass2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkRenderPassBeginInfo*                pRenderPassBegin,
    const VkSubpassBeginInfo*                   pSubpassBeginInfo) override;


virtual bool PreCallValidateCmdNextSubpass2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkSubpassBeginInfo*                   pSubpassBeginInfo,
    const VkSubpassEndInfo*                     pSubpassEndInfo) const override;


virtual void PreCallRecordCmdNextSubpass2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkSubpassBeginInfo*                   pSubpassBeginInfo,
    const VkSubpassEndInfo*                     pSubpassEndInfo) override;


virtual void PostCallRecordCmdNextSubpass2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkSubpassBeginInfo*                   pSubpassBeginInfo,
    const VkSubpassEndInfo*                     pSubpassEndInfo) override;


virtual bool PreCallValidateCmdEndRenderPass2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkSubpassEndInfo*                     pSubpassEndInfo) const override;


virtual void PreCallRecordCmdEndRenderPass2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkSubpassEndInfo*                     pSubpassEndInfo) override;


virtual void PostCallRecordCmdEndRenderPass2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkSubpassEndInfo*                     pSubpassEndInfo) override;


virtual bool PreCallValidateGetSwapchainStatusKHR(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain) const override;


virtual void PreCallRecordGetSwapchainStatusKHR(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain) override;


virtual void PostCallRecordGetSwapchainStatusKHR(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    VkResult                                    result) override;


virtual bool PreCallValidateGetPhysicalDeviceExternalFencePropertiesKHR(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceExternalFenceInfo*    pExternalFenceInfo,
    VkExternalFenceProperties*                  pExternalFenceProperties) const override;


virtual void PreCallRecordGetPhysicalDeviceExternalFencePropertiesKHR(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceExternalFenceInfo*    pExternalFenceInfo,
    VkExternalFenceProperties*                  pExternalFenceProperties) override;


virtual void PostCallRecordGetPhysicalDeviceExternalFencePropertiesKHR(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceExternalFenceInfo*    pExternalFenceInfo,
    VkExternalFenceProperties*                  pExternalFenceProperties) override;


#ifdef VK_USE_PLATFORM_WIN32_KHR

virtual bool PreCallValidateImportFenceWin32HandleKHR(
    VkDevice                                    device,
    const VkImportFenceWin32HandleInfoKHR*      pImportFenceWin32HandleInfo) const override;


virtual void PreCallRecordImportFenceWin32HandleKHR(
    VkDevice                                    device,
    const VkImportFenceWin32HandleInfoKHR*      pImportFenceWin32HandleInfo) override;


virtual void PostCallRecordImportFenceWin32HandleKHR(
    VkDevice                                    device,
    const VkImportFenceWin32HandleInfoKHR*      pImportFenceWin32HandleInfo,
    VkResult                                    result) override;


#endif // VK_USE_PLATFORM_WIN32_KHR

#ifdef VK_USE_PLATFORM_WIN32_KHR

virtual bool PreCallValidateGetFenceWin32HandleKHR(
    VkDevice                                    device,
    const VkFenceGetWin32HandleInfoKHR*         pGetWin32HandleInfo,
    HANDLE*                                     pHandle) const override;


virtual void PreCallRecordGetFenceWin32HandleKHR(
    VkDevice                                    device,
    const VkFenceGetWin32HandleInfoKHR*         pGetWin32HandleInfo,
    HANDLE*                                     pHandle) override;


virtual void PostCallRecordGetFenceWin32HandleKHR(
    VkDevice                                    device,
    const VkFenceGetWin32HandleInfoKHR*         pGetWin32HandleInfo,
    HANDLE*                                     pHandle,
    VkResult                                    result) override;


#endif // VK_USE_PLATFORM_WIN32_KHR

virtual bool PreCallValidateImportFenceFdKHR(
    VkDevice                                    device,
    const VkImportFenceFdInfoKHR*               pImportFenceFdInfo) const override;


virtual void PreCallRecordImportFenceFdKHR(
    VkDevice                                    device,
    const VkImportFenceFdInfoKHR*               pImportFenceFdInfo) override;


virtual void PostCallRecordImportFenceFdKHR(
    VkDevice                                    device,
    const VkImportFenceFdInfoKHR*               pImportFenceFdInfo,
    VkResult                                    result) override;


virtual bool PreCallValidateGetFenceFdKHR(
    VkDevice                                    device,
    const VkFenceGetFdInfoKHR*                  pGetFdInfo,
    int*                                        pFd) const override;


virtual void PreCallRecordGetFenceFdKHR(
    VkDevice                                    device,
    const VkFenceGetFdInfoKHR*                  pGetFdInfo,
    int*                                        pFd) override;


virtual void PostCallRecordGetFenceFdKHR(
    VkDevice                                    device,
    const VkFenceGetFdInfoKHR*                  pGetFdInfo,
    int*                                        pFd,
    VkResult                                    result) override;


virtual bool PreCallValidateEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    queueFamilyIndex,
    uint32_t*                                   pCounterCount,
    VkPerformanceCounterKHR*                    pCounters,
    VkPerformanceCounterDescriptionKHR*         pCounterDescriptions) const override;


virtual void PreCallRecordEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    queueFamilyIndex,
    uint32_t*                                   pCounterCount,
    VkPerformanceCounterKHR*                    pCounters,
    VkPerformanceCounterDescriptionKHR*         pCounterDescriptions) override;


virtual void PostCallRecordEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    queueFamilyIndex,
    uint32_t*                                   pCounterCount,
    VkPerformanceCounterKHR*                    pCounters,
    VkPerformanceCounterDescriptionKHR*         pCounterDescriptions,
    VkResult                                    result) override;


virtual bool PreCallValidateGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR(
    VkPhysicalDevice                            physicalDevice,
    const VkQueryPoolPerformanceCreateInfoKHR*  pPerformanceQueryCreateInfo,
    uint32_t*                                   pNumPasses) const override;


virtual void PreCallRecordGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR(
    VkPhysicalDevice                            physicalDevice,
    const VkQueryPoolPerformanceCreateInfoKHR*  pPerformanceQueryCreateInfo,
    uint32_t*                                   pNumPasses) override;


virtual void PostCallRecordGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR(
    VkPhysicalDevice                            physicalDevice,
    const VkQueryPoolPerformanceCreateInfoKHR*  pPerformanceQueryCreateInfo,
    uint32_t*                                   pNumPasses) override;


virtual bool PreCallValidateAcquireProfilingLockKHR(
    VkDevice                                    device,
    const VkAcquireProfilingLockInfoKHR*        pInfo) const override;


virtual void PreCallRecordAcquireProfilingLockKHR(
    VkDevice                                    device,
    const VkAcquireProfilingLockInfoKHR*        pInfo) override;


virtual void PostCallRecordAcquireProfilingLockKHR(
    VkDevice                                    device,
    const VkAcquireProfilingLockInfoKHR*        pInfo,
    VkResult                                    result) override;


virtual bool PreCallValidateReleaseProfilingLockKHR(
    VkDevice                                    device) const override;


virtual void PreCallRecordReleaseProfilingLockKHR(
    VkDevice                                    device) override;


virtual void PostCallRecordReleaseProfilingLockKHR(
    VkDevice                                    device) override;


virtual bool PreCallValidateGetPhysicalDeviceSurfaceCapabilities2KHR(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceSurfaceInfo2KHR*      pSurfaceInfo,
    VkSurfaceCapabilities2KHR*                  pSurfaceCapabilities) const override;


virtual void PreCallRecordGetPhysicalDeviceSurfaceCapabilities2KHR(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceSurfaceInfo2KHR*      pSurfaceInfo,
    VkSurfaceCapabilities2KHR*                  pSurfaceCapabilities) override;


virtual void PostCallRecordGetPhysicalDeviceSurfaceCapabilities2KHR(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceSurfaceInfo2KHR*      pSurfaceInfo,
    VkSurfaceCapabilities2KHR*                  pSurfaceCapabilities,
    VkResult                                    result) override;


virtual bool PreCallValidateGetPhysicalDeviceSurfaceFormats2KHR(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceSurfaceInfo2KHR*      pSurfaceInfo,
    uint32_t*                                   pSurfaceFormatCount,
    VkSurfaceFormat2KHR*                        pSurfaceFormats) const override;


virtual void PreCallRecordGetPhysicalDeviceSurfaceFormats2KHR(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceSurfaceInfo2KHR*      pSurfaceInfo,
    uint32_t*                                   pSurfaceFormatCount,
    VkSurfaceFormat2KHR*                        pSurfaceFormats) override;


virtual void PostCallRecordGetPhysicalDeviceSurfaceFormats2KHR(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceSurfaceInfo2KHR*      pSurfaceInfo,
    uint32_t*                                   pSurfaceFormatCount,
    VkSurfaceFormat2KHR*                        pSurfaceFormats,
    VkResult                                    result) override;


virtual bool PreCallValidateGetPhysicalDeviceDisplayProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pPropertyCount,
    VkDisplayProperties2KHR*                    pProperties) const override;


virtual void PreCallRecordGetPhysicalDeviceDisplayProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pPropertyCount,
    VkDisplayProperties2KHR*                    pProperties) override;


virtual void PostCallRecordGetPhysicalDeviceDisplayProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pPropertyCount,
    VkDisplayProperties2KHR*                    pProperties,
    VkResult                                    result) override;


virtual bool PreCallValidateGetPhysicalDeviceDisplayPlaneProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pPropertyCount,
    VkDisplayPlaneProperties2KHR*               pProperties) const override;


virtual void PreCallRecordGetPhysicalDeviceDisplayPlaneProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pPropertyCount,
    VkDisplayPlaneProperties2KHR*               pProperties) override;


virtual void PostCallRecordGetPhysicalDeviceDisplayPlaneProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pPropertyCount,
    VkDisplayPlaneProperties2KHR*               pProperties,
    VkResult                                    result) override;


virtual bool PreCallValidateGetDisplayModeProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    VkDisplayKHR                                display,
    uint32_t*                                   pPropertyCount,
    VkDisplayModeProperties2KHR*                pProperties) const override;


virtual void PreCallRecordGetDisplayModeProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    VkDisplayKHR                                display,
    uint32_t*                                   pPropertyCount,
    VkDisplayModeProperties2KHR*                pProperties) override;


virtual void PostCallRecordGetDisplayModeProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    VkDisplayKHR                                display,
    uint32_t*                                   pPropertyCount,
    VkDisplayModeProperties2KHR*                pProperties,
    VkResult                                    result) override;


virtual bool PreCallValidateGetDisplayPlaneCapabilities2KHR(
    VkPhysicalDevice                            physicalDevice,
    const VkDisplayPlaneInfo2KHR*               pDisplayPlaneInfo,
    VkDisplayPlaneCapabilities2KHR*             pCapabilities) const override;


virtual void PreCallRecordGetDisplayPlaneCapabilities2KHR(
    VkPhysicalDevice                            physicalDevice,
    const VkDisplayPlaneInfo2KHR*               pDisplayPlaneInfo,
    VkDisplayPlaneCapabilities2KHR*             pCapabilities) override;


virtual void PostCallRecordGetDisplayPlaneCapabilities2KHR(
    VkPhysicalDevice                            physicalDevice,
    const VkDisplayPlaneInfo2KHR*               pDisplayPlaneInfo,
    VkDisplayPlaneCapabilities2KHR*             pCapabilities,
    VkResult                                    result) override;


virtual bool PreCallValidateGetImageMemoryRequirements2KHR(
    VkDevice                                    device,
    const VkImageMemoryRequirementsInfo2*       pInfo,
    VkMemoryRequirements2*                      pMemoryRequirements) const override;


virtual void PreCallRecordGetImageMemoryRequirements2KHR(
    VkDevice                                    device,
    const VkImageMemoryRequirementsInfo2*       pInfo,
    VkMemoryRequirements2*                      pMemoryRequirements) override;


virtual void PostCallRecordGetImageMemoryRequirements2KHR(
    VkDevice                                    device,
    const VkImageMemoryRequirementsInfo2*       pInfo,
    VkMemoryRequirements2*                      pMemoryRequirements) override;


virtual bool PreCallValidateGetBufferMemoryRequirements2KHR(
    VkDevice                                    device,
    const VkBufferMemoryRequirementsInfo2*      pInfo,
    VkMemoryRequirements2*                      pMemoryRequirements) const override;


virtual void PreCallRecordGetBufferMemoryRequirements2KHR(
    VkDevice                                    device,
    const VkBufferMemoryRequirementsInfo2*      pInfo,
    VkMemoryRequirements2*                      pMemoryRequirements) override;


virtual void PostCallRecordGetBufferMemoryRequirements2KHR(
    VkDevice                                    device,
    const VkBufferMemoryRequirementsInfo2*      pInfo,
    VkMemoryRequirements2*                      pMemoryRequirements) override;


virtual bool PreCallValidateGetImageSparseMemoryRequirements2KHR(
    VkDevice                                    device,
    const VkImageSparseMemoryRequirementsInfo2* pInfo,
    uint32_t*                                   pSparseMemoryRequirementCount,
    VkSparseImageMemoryRequirements2*           pSparseMemoryRequirements) const override;


virtual void PreCallRecordGetImageSparseMemoryRequirements2KHR(
    VkDevice                                    device,
    const VkImageSparseMemoryRequirementsInfo2* pInfo,
    uint32_t*                                   pSparseMemoryRequirementCount,
    VkSparseImageMemoryRequirements2*           pSparseMemoryRequirements) override;


virtual void PostCallRecordGetImageSparseMemoryRequirements2KHR(
    VkDevice                                    device,
    const VkImageSparseMemoryRequirementsInfo2* pInfo,
    uint32_t*                                   pSparseMemoryRequirementCount,
    VkSparseImageMemoryRequirements2*           pSparseMemoryRequirements) override;


virtual bool PreCallValidateCreateSamplerYcbcrConversionKHR(
    VkDevice                                    device,
    const VkSamplerYcbcrConversionCreateInfo*   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSamplerYcbcrConversion*                   pYcbcrConversion) const override;


virtual void PreCallRecordCreateSamplerYcbcrConversionKHR(
    VkDevice                                    device,
    const VkSamplerYcbcrConversionCreateInfo*   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSamplerYcbcrConversion*                   pYcbcrConversion) override;


virtual void PostCallRecordCreateSamplerYcbcrConversionKHR(
    VkDevice                                    device,
    const VkSamplerYcbcrConversionCreateInfo*   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSamplerYcbcrConversion*                   pYcbcrConversion,
    VkResult                                    result) override;


virtual bool PreCallValidateDestroySamplerYcbcrConversionKHR(
    VkDevice                                    device,
    VkSamplerYcbcrConversion                    ycbcrConversion,
    const VkAllocationCallbacks*                pAllocator) const override;


virtual void PreCallRecordDestroySamplerYcbcrConversionKHR(
    VkDevice                                    device,
    VkSamplerYcbcrConversion                    ycbcrConversion,
    const VkAllocationCallbacks*                pAllocator) override;


virtual void PostCallRecordDestroySamplerYcbcrConversionKHR(
    VkDevice                                    device,
    VkSamplerYcbcrConversion                    ycbcrConversion,
    const VkAllocationCallbacks*                pAllocator) override;


virtual bool PreCallValidateBindBufferMemory2KHR(
    VkDevice                                    device,
    uint32_t                                    bindInfoCount,
    const VkBindBufferMemoryInfo*               pBindInfos) const override;


virtual void PreCallRecordBindBufferMemory2KHR(
    VkDevice                                    device,
    uint32_t                                    bindInfoCount,
    const VkBindBufferMemoryInfo*               pBindInfos) override;


virtual void PostCallRecordBindBufferMemory2KHR(
    VkDevice                                    device,
    uint32_t                                    bindInfoCount,
    const VkBindBufferMemoryInfo*               pBindInfos,
    VkResult                                    result) override;


virtual bool PreCallValidateBindImageMemory2KHR(
    VkDevice                                    device,
    uint32_t                                    bindInfoCount,
    const VkBindImageMemoryInfo*                pBindInfos) const override;


virtual void PreCallRecordBindImageMemory2KHR(
    VkDevice                                    device,
    uint32_t                                    bindInfoCount,
    const VkBindImageMemoryInfo*                pBindInfos) override;


virtual void PostCallRecordBindImageMemory2KHR(
    VkDevice                                    device,
    uint32_t                                    bindInfoCount,
    const VkBindImageMemoryInfo*                pBindInfos,
    VkResult                                    result) override;


virtual bool PreCallValidateGetDescriptorSetLayoutSupportKHR(
    VkDevice                                    device,
    const VkDescriptorSetLayoutCreateInfo*      pCreateInfo,
    VkDescriptorSetLayoutSupport*               pSupport) const override;


virtual void PreCallRecordGetDescriptorSetLayoutSupportKHR(
    VkDevice                                    device,
    const VkDescriptorSetLayoutCreateInfo*      pCreateInfo,
    VkDescriptorSetLayoutSupport*               pSupport) override;


virtual void PostCallRecordGetDescriptorSetLayoutSupportKHR(
    VkDevice                                    device,
    const VkDescriptorSetLayoutCreateInfo*      pCreateInfo,
    VkDescriptorSetLayoutSupport*               pSupport) override;


virtual bool PreCallValidateCmdDrawIndirectCountKHR(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkBuffer                                    countBuffer,
    VkDeviceSize                                countBufferOffset,
    uint32_t                                    maxDrawCount,
    uint32_t                                    stride) const override;


virtual void PreCallRecordCmdDrawIndirectCountKHR(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkBuffer                                    countBuffer,
    VkDeviceSize                                countBufferOffset,
    uint32_t                                    maxDrawCount,
    uint32_t                                    stride) override;


virtual void PostCallRecordCmdDrawIndirectCountKHR(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkBuffer                                    countBuffer,
    VkDeviceSize                                countBufferOffset,
    uint32_t                                    maxDrawCount,
    uint32_t                                    stride) override;


virtual bool PreCallValidateCmdDrawIndexedIndirectCountKHR(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkBuffer                                    countBuffer,
    VkDeviceSize                                countBufferOffset,
    uint32_t                                    maxDrawCount,
    uint32_t                                    stride) const override;


virtual void PreCallRecordCmdDrawIndexedIndirectCountKHR(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkBuffer                                    countBuffer,
    VkDeviceSize                                countBufferOffset,
    uint32_t                                    maxDrawCount,
    uint32_t                                    stride) override;


virtual void PostCallRecordCmdDrawIndexedIndirectCountKHR(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkBuffer                                    countBuffer,
    VkDeviceSize                                countBufferOffset,
    uint32_t                                    maxDrawCount,
    uint32_t                                    stride) override;


virtual bool PreCallValidateGetSemaphoreCounterValueKHR(
    VkDevice                                    device,
    VkSemaphore                                 semaphore,
    uint64_t*                                   pValue) const override;


virtual void PreCallRecordGetSemaphoreCounterValueKHR(
    VkDevice                                    device,
    VkSemaphore                                 semaphore,
    uint64_t*                                   pValue) override;


virtual void PostCallRecordGetSemaphoreCounterValueKHR(
    VkDevice                                    device,
    VkSemaphore                                 semaphore,
    uint64_t*                                   pValue,
    VkResult                                    result) override;


virtual bool PreCallValidateWaitSemaphoresKHR(
    VkDevice                                    device,
    const VkSemaphoreWaitInfo*                  pWaitInfo,
    uint64_t                                    timeout) const override;


virtual void PreCallRecordWaitSemaphoresKHR(
    VkDevice                                    device,
    const VkSemaphoreWaitInfo*                  pWaitInfo,
    uint64_t                                    timeout) override;


virtual void PostCallRecordWaitSemaphoresKHR(
    VkDevice                                    device,
    const VkSemaphoreWaitInfo*                  pWaitInfo,
    uint64_t                                    timeout,
    VkResult                                    result) override;


virtual bool PreCallValidateSignalSemaphoreKHR(
    VkDevice                                    device,
    const VkSemaphoreSignalInfo*                pSignalInfo) const override;


virtual void PreCallRecordSignalSemaphoreKHR(
    VkDevice                                    device,
    const VkSemaphoreSignalInfo*                pSignalInfo) override;


virtual void PostCallRecordSignalSemaphoreKHR(
    VkDevice                                    device,
    const VkSemaphoreSignalInfo*                pSignalInfo,
    VkResult                                    result) override;


virtual bool PreCallValidateGetBufferDeviceAddressKHR(
    VkDevice                                    device,
    const VkBufferDeviceAddressInfo*            pInfo) const override;


virtual void PreCallRecordGetBufferDeviceAddressKHR(
    VkDevice                                    device,
    const VkBufferDeviceAddressInfo*            pInfo) override;


virtual void PostCallRecordGetBufferDeviceAddressKHR(
    VkDevice                                    device,
    const VkBufferDeviceAddressInfo*            pInfo,
    VkResult                                    result) override;


virtual bool PreCallValidateGetBufferOpaqueCaptureAddressKHR(
    VkDevice                                    device,
    const VkBufferDeviceAddressInfo*            pInfo) const override;


virtual void PreCallRecordGetBufferOpaqueCaptureAddressKHR(
    VkDevice                                    device,
    const VkBufferDeviceAddressInfo*            pInfo) override;


virtual void PostCallRecordGetBufferOpaqueCaptureAddressKHR(
    VkDevice                                    device,
    const VkBufferDeviceAddressInfo*            pInfo) override;


virtual bool PreCallValidateGetDeviceMemoryOpaqueCaptureAddressKHR(
    VkDevice                                    device,
    const VkDeviceMemoryOpaqueCaptureAddressInfo* pInfo) const override;


virtual void PreCallRecordGetDeviceMemoryOpaqueCaptureAddressKHR(
    VkDevice                                    device,
    const VkDeviceMemoryOpaqueCaptureAddressInfo* pInfo) override;


virtual void PostCallRecordGetDeviceMemoryOpaqueCaptureAddressKHR(
    VkDevice                                    device,
    const VkDeviceMemoryOpaqueCaptureAddressInfo* pInfo) override;


#ifdef VK_ENABLE_BETA_EXTENSIONS

virtual bool PreCallValidateCreateDeferredOperationKHR(
    VkDevice                                    device,
    const VkAllocationCallbacks*                pAllocator,
    VkDeferredOperationKHR*                     pDeferredOperation) const override;


virtual void PreCallRecordCreateDeferredOperationKHR(
    VkDevice                                    device,
    const VkAllocationCallbacks*                pAllocator,
    VkDeferredOperationKHR*                     pDeferredOperation) override;


virtual void PostCallRecordCreateDeferredOperationKHR(
    VkDevice                                    device,
    const VkAllocationCallbacks*                pAllocator,
    VkDeferredOperationKHR*                     pDeferredOperation,
    VkResult                                    result) override;


#endif // VK_ENABLE_BETA_EXTENSIONS

#ifdef VK_ENABLE_BETA_EXTENSIONS

virtual bool PreCallValidateDestroyDeferredOperationKHR(
    VkDevice                                    device,
    VkDeferredOperationKHR                      operation,
    const VkAllocationCallbacks*                pAllocator) const override;


virtual void PreCallRecordDestroyDeferredOperationKHR(
    VkDevice                                    device,
    VkDeferredOperationKHR                      operation,
    const VkAllocationCallbacks*                pAllocator) override;


virtual void PostCallRecordDestroyDeferredOperationKHR(
    VkDevice                                    device,
    VkDeferredOperationKHR                      operation,
    const VkAllocationCallbacks*                pAllocator) override;


#endif // VK_ENABLE_BETA_EXTENSIONS

#ifdef VK_ENABLE_BETA_EXTENSIONS

virtual bool PreCallValidateGetDeferredOperationMaxConcurrencyKHR(
    VkDevice                                    device,
    VkDeferredOperationKHR                      operation) const override;


virtual void PreCallRecordGetDeferredOperationMaxConcurrencyKHR(
    VkDevice                                    device,
    VkDeferredOperationKHR                      operation) override;


virtual void PostCallRecordGetDeferredOperationMaxConcurrencyKHR(
    VkDevice                                    device,
    VkDeferredOperationKHR                      operation) override;


#endif // VK_ENABLE_BETA_EXTENSIONS

#ifdef VK_ENABLE_BETA_EXTENSIONS

virtual bool PreCallValidateGetDeferredOperationResultKHR(
    VkDevice                                    device,
    VkDeferredOperationKHR                      operation) const override;


virtual void PreCallRecordGetDeferredOperationResultKHR(
    VkDevice                                    device,
    VkDeferredOperationKHR                      operation) override;


virtual void PostCallRecordGetDeferredOperationResultKHR(
    VkDevice                                    device,
    VkDeferredOperationKHR                      operation,
    VkResult                                    result) override;


#endif // VK_ENABLE_BETA_EXTENSIONS

#ifdef VK_ENABLE_BETA_EXTENSIONS

virtual bool PreCallValidateDeferredOperationJoinKHR(
    VkDevice                                    device,
    VkDeferredOperationKHR                      operation) const override;


virtual void PreCallRecordDeferredOperationJoinKHR(
    VkDevice                                    device,
    VkDeferredOperationKHR                      operation) override;


virtual void PostCallRecordDeferredOperationJoinKHR(
    VkDevice                                    device,
    VkDeferredOperationKHR                      operation,
    VkResult                                    result) override;


#endif // VK_ENABLE_BETA_EXTENSIONS

virtual bool PreCallValidateGetPipelineExecutablePropertiesKHR(
    VkDevice                                    device,
    const VkPipelineInfoKHR*                    pPipelineInfo,
    uint32_t*                                   pExecutableCount,
    VkPipelineExecutablePropertiesKHR*          pProperties) const override;


virtual void PreCallRecordGetPipelineExecutablePropertiesKHR(
    VkDevice                                    device,
    const VkPipelineInfoKHR*                    pPipelineInfo,
    uint32_t*                                   pExecutableCount,
    VkPipelineExecutablePropertiesKHR*          pProperties) override;


virtual void PostCallRecordGetPipelineExecutablePropertiesKHR(
    VkDevice                                    device,
    const VkPipelineInfoKHR*                    pPipelineInfo,
    uint32_t*                                   pExecutableCount,
    VkPipelineExecutablePropertiesKHR*          pProperties,
    VkResult                                    result) override;


virtual bool PreCallValidateGetPipelineExecutableStatisticsKHR(
    VkDevice                                    device,
    const VkPipelineExecutableInfoKHR*          pExecutableInfo,
    uint32_t*                                   pStatisticCount,
    VkPipelineExecutableStatisticKHR*           pStatistics) const override;


virtual void PreCallRecordGetPipelineExecutableStatisticsKHR(
    VkDevice                                    device,
    const VkPipelineExecutableInfoKHR*          pExecutableInfo,
    uint32_t*                                   pStatisticCount,
    VkPipelineExecutableStatisticKHR*           pStatistics) override;


virtual void PostCallRecordGetPipelineExecutableStatisticsKHR(
    VkDevice                                    device,
    const VkPipelineExecutableInfoKHR*          pExecutableInfo,
    uint32_t*                                   pStatisticCount,
    VkPipelineExecutableStatisticKHR*           pStatistics,
    VkResult                                    result) override;


virtual bool PreCallValidateGetPipelineExecutableInternalRepresentationsKHR(
    VkDevice                                    device,
    const VkPipelineExecutableInfoKHR*          pExecutableInfo,
    uint32_t*                                   pInternalRepresentationCount,
    VkPipelineExecutableInternalRepresentationKHR* pInternalRepresentations) const override;


virtual void PreCallRecordGetPipelineExecutableInternalRepresentationsKHR(
    VkDevice                                    device,
    const VkPipelineExecutableInfoKHR*          pExecutableInfo,
    uint32_t*                                   pInternalRepresentationCount,
    VkPipelineExecutableInternalRepresentationKHR* pInternalRepresentations) override;


virtual void PostCallRecordGetPipelineExecutableInternalRepresentationsKHR(
    VkDevice                                    device,
    const VkPipelineExecutableInfoKHR*          pExecutableInfo,
    uint32_t*                                   pInternalRepresentationCount,
    VkPipelineExecutableInternalRepresentationKHR* pInternalRepresentations,
    VkResult                                    result) override;


virtual bool PreCallValidateCreateDebugReportCallbackEXT(
    VkInstance                                  instance,
    const VkDebugReportCallbackCreateInfoEXT*   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDebugReportCallbackEXT*                   pCallback) const override;


virtual void PreCallRecordCreateDebugReportCallbackEXT(
    VkInstance                                  instance,
    const VkDebugReportCallbackCreateInfoEXT*   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDebugReportCallbackEXT*                   pCallback) override;


virtual void PostCallRecordCreateDebugReportCallbackEXT(
    VkInstance                                  instance,
    const VkDebugReportCallbackCreateInfoEXT*   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDebugReportCallbackEXT*                   pCallback,
    VkResult                                    result) override;


virtual bool PreCallValidateDestroyDebugReportCallbackEXT(
    VkInstance                                  instance,
    VkDebugReportCallbackEXT                    callback,
    const VkAllocationCallbacks*                pAllocator) const override;


virtual void PreCallRecordDestroyDebugReportCallbackEXT(
    VkInstance                                  instance,
    VkDebugReportCallbackEXT                    callback,
    const VkAllocationCallbacks*                pAllocator) override;


virtual void PostCallRecordDestroyDebugReportCallbackEXT(
    VkInstance                                  instance,
    VkDebugReportCallbackEXT                    callback,
    const VkAllocationCallbacks*                pAllocator) override;


virtual bool PreCallValidateDebugReportMessageEXT(
    VkInstance                                  instance,
    VkDebugReportFlagsEXT                       flags,
    VkDebugReportObjectTypeEXT                  objectType,
    uint64_t                                    object,
    size_t                                      location,
    int32_t                                     messageCode,
    const char*                                 pLayerPrefix,
    const char*                                 pMessage) const override;


virtual void PreCallRecordDebugReportMessageEXT(
    VkInstance                                  instance,
    VkDebugReportFlagsEXT                       flags,
    VkDebugReportObjectTypeEXT                  objectType,
    uint64_t                                    object,
    size_t                                      location,
    int32_t                                     messageCode,
    const char*                                 pLayerPrefix,
    const char*                                 pMessage) override;


virtual void PostCallRecordDebugReportMessageEXT(
    VkInstance                                  instance,
    VkDebugReportFlagsEXT                       flags,
    VkDebugReportObjectTypeEXT                  objectType,
    uint64_t                                    object,
    size_t                                      location,
    int32_t                                     messageCode,
    const char*                                 pLayerPrefix,
    const char*                                 pMessage) override;


virtual bool PreCallValidateDebugMarkerSetObjectTagEXT(
    VkDevice                                    device,
    const VkDebugMarkerObjectTagInfoEXT*        pTagInfo) const override;


virtual void PreCallRecordDebugMarkerSetObjectTagEXT(
    VkDevice                                    device,
    const VkDebugMarkerObjectTagInfoEXT*        pTagInfo) override;


virtual void PostCallRecordDebugMarkerSetObjectTagEXT(
    VkDevice                                    device,
    const VkDebugMarkerObjectTagInfoEXT*        pTagInfo,
    VkResult                                    result) override;


virtual bool PreCallValidateDebugMarkerSetObjectNameEXT(
    VkDevice                                    device,
    const VkDebugMarkerObjectNameInfoEXT*       pNameInfo) const override;


virtual void PreCallRecordDebugMarkerSetObjectNameEXT(
    VkDevice                                    device,
    const VkDebugMarkerObjectNameInfoEXT*       pNameInfo) override;


virtual void PostCallRecordDebugMarkerSetObjectNameEXT(
    VkDevice                                    device,
    const VkDebugMarkerObjectNameInfoEXT*       pNameInfo,
    VkResult                                    result) override;


virtual bool PreCallValidateCmdDebugMarkerBeginEXT(
    VkCommandBuffer                             commandBuffer,
    const VkDebugMarkerMarkerInfoEXT*           pMarkerInfo) const override;


virtual void PreCallRecordCmdDebugMarkerBeginEXT(
    VkCommandBuffer                             commandBuffer,
    const VkDebugMarkerMarkerInfoEXT*           pMarkerInfo) override;


virtual void PostCallRecordCmdDebugMarkerBeginEXT(
    VkCommandBuffer                             commandBuffer,
    const VkDebugMarkerMarkerInfoEXT*           pMarkerInfo) override;


virtual bool PreCallValidateCmdDebugMarkerEndEXT(
    VkCommandBuffer                             commandBuffer) const override;


virtual void PreCallRecordCmdDebugMarkerEndEXT(
    VkCommandBuffer                             commandBuffer) override;


virtual void PostCallRecordCmdDebugMarkerEndEXT(
    VkCommandBuffer                             commandBuffer) override;


virtual bool PreCallValidateCmdDebugMarkerInsertEXT(
    VkCommandBuffer                             commandBuffer,
    const VkDebugMarkerMarkerInfoEXT*           pMarkerInfo) const override;


virtual void PreCallRecordCmdDebugMarkerInsertEXT(
    VkCommandBuffer                             commandBuffer,
    const VkDebugMarkerMarkerInfoEXT*           pMarkerInfo) override;


virtual void PostCallRecordCmdDebugMarkerInsertEXT(
    VkCommandBuffer                             commandBuffer,
    const VkDebugMarkerMarkerInfoEXT*           pMarkerInfo) override;


virtual bool PreCallValidateCmdBindTransformFeedbackBuffersEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstBinding,
    uint32_t                                    bindingCount,
    const VkBuffer*                             pBuffers,
    const VkDeviceSize*                         pOffsets,
    const VkDeviceSize*                         pSizes) const override;


virtual void PreCallRecordCmdBindTransformFeedbackBuffersEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstBinding,
    uint32_t                                    bindingCount,
    const VkBuffer*                             pBuffers,
    const VkDeviceSize*                         pOffsets,
    const VkDeviceSize*                         pSizes) override;


virtual void PostCallRecordCmdBindTransformFeedbackBuffersEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstBinding,
    uint32_t                                    bindingCount,
    const VkBuffer*                             pBuffers,
    const VkDeviceSize*                         pOffsets,
    const VkDeviceSize*                         pSizes) override;


virtual bool PreCallValidateCmdBeginTransformFeedbackEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstCounterBuffer,
    uint32_t                                    counterBufferCount,
    const VkBuffer*                             pCounterBuffers,
    const VkDeviceSize*                         pCounterBufferOffsets) const override;


virtual void PreCallRecordCmdBeginTransformFeedbackEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstCounterBuffer,
    uint32_t                                    counterBufferCount,
    const VkBuffer*                             pCounterBuffers,
    const VkDeviceSize*                         pCounterBufferOffsets) override;


virtual void PostCallRecordCmdBeginTransformFeedbackEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstCounterBuffer,
    uint32_t                                    counterBufferCount,
    const VkBuffer*                             pCounterBuffers,
    const VkDeviceSize*                         pCounterBufferOffsets) override;


virtual bool PreCallValidateCmdEndTransformFeedbackEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstCounterBuffer,
    uint32_t                                    counterBufferCount,
    const VkBuffer*                             pCounterBuffers,
    const VkDeviceSize*                         pCounterBufferOffsets) const override;


virtual void PreCallRecordCmdEndTransformFeedbackEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstCounterBuffer,
    uint32_t                                    counterBufferCount,
    const VkBuffer*                             pCounterBuffers,
    const VkDeviceSize*                         pCounterBufferOffsets) override;


virtual void PostCallRecordCmdEndTransformFeedbackEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstCounterBuffer,
    uint32_t                                    counterBufferCount,
    const VkBuffer*                             pCounterBuffers,
    const VkDeviceSize*                         pCounterBufferOffsets) override;


virtual bool PreCallValidateCmdBeginQueryIndexedEXT(
    VkCommandBuffer                             commandBuffer,
    VkQueryPool                                 queryPool,
    uint32_t                                    query,
    VkQueryControlFlags                         flags,
    uint32_t                                    index) const override;


virtual void PreCallRecordCmdBeginQueryIndexedEXT(
    VkCommandBuffer                             commandBuffer,
    VkQueryPool                                 queryPool,
    uint32_t                                    query,
    VkQueryControlFlags                         flags,
    uint32_t                                    index) override;


virtual void PostCallRecordCmdBeginQueryIndexedEXT(
    VkCommandBuffer                             commandBuffer,
    VkQueryPool                                 queryPool,
    uint32_t                                    query,
    VkQueryControlFlags                         flags,
    uint32_t                                    index) override;


virtual bool PreCallValidateCmdEndQueryIndexedEXT(
    VkCommandBuffer                             commandBuffer,
    VkQueryPool                                 queryPool,
    uint32_t                                    query,
    uint32_t                                    index) const override;


virtual void PreCallRecordCmdEndQueryIndexedEXT(
    VkCommandBuffer                             commandBuffer,
    VkQueryPool                                 queryPool,
    uint32_t                                    query,
    uint32_t                                    index) override;


virtual void PostCallRecordCmdEndQueryIndexedEXT(
    VkCommandBuffer                             commandBuffer,
    VkQueryPool                                 queryPool,
    uint32_t                                    query,
    uint32_t                                    index) override;


virtual bool PreCallValidateCmdDrawIndirectByteCountEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    instanceCount,
    uint32_t                                    firstInstance,
    VkBuffer                                    counterBuffer,
    VkDeviceSize                                counterBufferOffset,
    uint32_t                                    counterOffset,
    uint32_t                                    vertexStride) const override;


virtual void PreCallRecordCmdDrawIndirectByteCountEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    instanceCount,
    uint32_t                                    firstInstance,
    VkBuffer                                    counterBuffer,
    VkDeviceSize                                counterBufferOffset,
    uint32_t                                    counterOffset,
    uint32_t                                    vertexStride) override;


virtual void PostCallRecordCmdDrawIndirectByteCountEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    instanceCount,
    uint32_t                                    firstInstance,
    VkBuffer                                    counterBuffer,
    VkDeviceSize                                counterBufferOffset,
    uint32_t                                    counterOffset,
    uint32_t                                    vertexStride) override;


virtual bool PreCallValidateGetImageViewHandleNVX(
    VkDevice                                    device,
    const VkImageViewHandleInfoNVX*             pInfo) const override;


virtual void PreCallRecordGetImageViewHandleNVX(
    VkDevice                                    device,
    const VkImageViewHandleInfoNVX*             pInfo) override;


virtual void PostCallRecordGetImageViewHandleNVX(
    VkDevice                                    device,
    const VkImageViewHandleInfoNVX*             pInfo) override;


virtual bool PreCallValidateGetImageViewAddressNVX(
    VkDevice                                    device,
    VkImageView                                 imageView,
    VkImageViewAddressPropertiesNVX*            pProperties) const override;


virtual void PreCallRecordGetImageViewAddressNVX(
    VkDevice                                    device,
    VkImageView                                 imageView,
    VkImageViewAddressPropertiesNVX*            pProperties) override;


virtual void PostCallRecordGetImageViewAddressNVX(
    VkDevice                                    device,
    VkImageView                                 imageView,
    VkImageViewAddressPropertiesNVX*            pProperties,
    VkResult                                    result) override;


virtual bool PreCallValidateCmdDrawIndirectCountAMD(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkBuffer                                    countBuffer,
    VkDeviceSize                                countBufferOffset,
    uint32_t                                    maxDrawCount,
    uint32_t                                    stride) const override;


virtual void PreCallRecordCmdDrawIndirectCountAMD(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkBuffer                                    countBuffer,
    VkDeviceSize                                countBufferOffset,
    uint32_t                                    maxDrawCount,
    uint32_t                                    stride) override;


virtual void PostCallRecordCmdDrawIndirectCountAMD(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkBuffer                                    countBuffer,
    VkDeviceSize                                countBufferOffset,
    uint32_t                                    maxDrawCount,
    uint32_t                                    stride) override;


virtual bool PreCallValidateCmdDrawIndexedIndirectCountAMD(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkBuffer                                    countBuffer,
    VkDeviceSize                                countBufferOffset,
    uint32_t                                    maxDrawCount,
    uint32_t                                    stride) const override;


virtual void PreCallRecordCmdDrawIndexedIndirectCountAMD(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkBuffer                                    countBuffer,
    VkDeviceSize                                countBufferOffset,
    uint32_t                                    maxDrawCount,
    uint32_t                                    stride) override;


virtual void PostCallRecordCmdDrawIndexedIndirectCountAMD(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkBuffer                                    countBuffer,
    VkDeviceSize                                countBufferOffset,
    uint32_t                                    maxDrawCount,
    uint32_t                                    stride) override;


virtual bool PreCallValidateGetShaderInfoAMD(
    VkDevice                                    device,
    VkPipeline                                  pipeline,
    VkShaderStageFlagBits                       shaderStage,
    VkShaderInfoTypeAMD                         infoType,
    size_t*                                     pInfoSize,
    void*                                       pInfo) const override;


virtual void PreCallRecordGetShaderInfoAMD(
    VkDevice                                    device,
    VkPipeline                                  pipeline,
    VkShaderStageFlagBits                       shaderStage,
    VkShaderInfoTypeAMD                         infoType,
    size_t*                                     pInfoSize,
    void*                                       pInfo) override;


virtual void PostCallRecordGetShaderInfoAMD(
    VkDevice                                    device,
    VkPipeline                                  pipeline,
    VkShaderStageFlagBits                       shaderStage,
    VkShaderInfoTypeAMD                         infoType,
    size_t*                                     pInfoSize,
    void*                                       pInfo,
    VkResult                                    result) override;


#ifdef VK_USE_PLATFORM_GGP

virtual bool PreCallValidateCreateStreamDescriptorSurfaceGGP(
    VkInstance                                  instance,
    const VkStreamDescriptorSurfaceCreateInfoGGP* pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) const override;


virtual void PreCallRecordCreateStreamDescriptorSurfaceGGP(
    VkInstance                                  instance,
    const VkStreamDescriptorSurfaceCreateInfoGGP* pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) override;


virtual void PostCallRecordCreateStreamDescriptorSurfaceGGP(
    VkInstance                                  instance,
    const VkStreamDescriptorSurfaceCreateInfoGGP* pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface,
    VkResult                                    result) override;


#endif // VK_USE_PLATFORM_GGP

virtual bool PreCallValidateGetPhysicalDeviceExternalImageFormatPropertiesNV(
    VkPhysicalDevice                            physicalDevice,
    VkFormat                                    format,
    VkImageType                                 type,
    VkImageTiling                               tiling,
    VkImageUsageFlags                           usage,
    VkImageCreateFlags                          flags,
    VkExternalMemoryHandleTypeFlagsNV           externalHandleType,
    VkExternalImageFormatPropertiesNV*          pExternalImageFormatProperties) const override;


virtual void PreCallRecordGetPhysicalDeviceExternalImageFormatPropertiesNV(
    VkPhysicalDevice                            physicalDevice,
    VkFormat                                    format,
    VkImageType                                 type,
    VkImageTiling                               tiling,
    VkImageUsageFlags                           usage,
    VkImageCreateFlags                          flags,
    VkExternalMemoryHandleTypeFlagsNV           externalHandleType,
    VkExternalImageFormatPropertiesNV*          pExternalImageFormatProperties) override;


virtual void PostCallRecordGetPhysicalDeviceExternalImageFormatPropertiesNV(
    VkPhysicalDevice                            physicalDevice,
    VkFormat                                    format,
    VkImageType                                 type,
    VkImageTiling                               tiling,
    VkImageUsageFlags                           usage,
    VkImageCreateFlags                          flags,
    VkExternalMemoryHandleTypeFlagsNV           externalHandleType,
    VkExternalImageFormatPropertiesNV*          pExternalImageFormatProperties,
    VkResult                                    result) override;


#ifdef VK_USE_PLATFORM_WIN32_KHR

virtual bool PreCallValidateGetMemoryWin32HandleNV(
    VkDevice                                    device,
    VkDeviceMemory                              memory,
    VkExternalMemoryHandleTypeFlagsNV           handleType,
    HANDLE*                                     pHandle) const override;


virtual void PreCallRecordGetMemoryWin32HandleNV(
    VkDevice                                    device,
    VkDeviceMemory                              memory,
    VkExternalMemoryHandleTypeFlagsNV           handleType,
    HANDLE*                                     pHandle) override;


virtual void PostCallRecordGetMemoryWin32HandleNV(
    VkDevice                                    device,
    VkDeviceMemory                              memory,
    VkExternalMemoryHandleTypeFlagsNV           handleType,
    HANDLE*                                     pHandle,
    VkResult                                    result) override;


#endif // VK_USE_PLATFORM_WIN32_KHR

#ifdef VK_USE_PLATFORM_VI_NN

virtual bool PreCallValidateCreateViSurfaceNN(
    VkInstance                                  instance,
    const VkViSurfaceCreateInfoNN*              pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) const override;


virtual void PreCallRecordCreateViSurfaceNN(
    VkInstance                                  instance,
    const VkViSurfaceCreateInfoNN*              pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) override;


virtual void PostCallRecordCreateViSurfaceNN(
    VkInstance                                  instance,
    const VkViSurfaceCreateInfoNN*              pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface,
    VkResult                                    result) override;


#endif // VK_USE_PLATFORM_VI_NN

virtual bool PreCallValidateCmdBeginConditionalRenderingEXT(
    VkCommandBuffer                             commandBuffer,
    const VkConditionalRenderingBeginInfoEXT*   pConditionalRenderingBegin) const override;


virtual void PreCallRecordCmdBeginConditionalRenderingEXT(
    VkCommandBuffer                             commandBuffer,
    const VkConditionalRenderingBeginInfoEXT*   pConditionalRenderingBegin) override;


virtual void PostCallRecordCmdBeginConditionalRenderingEXT(
    VkCommandBuffer                             commandBuffer,
    const VkConditionalRenderingBeginInfoEXT*   pConditionalRenderingBegin) override;


virtual bool PreCallValidateCmdEndConditionalRenderingEXT(
    VkCommandBuffer                             commandBuffer) const override;


virtual void PreCallRecordCmdEndConditionalRenderingEXT(
    VkCommandBuffer                             commandBuffer) override;


virtual void PostCallRecordCmdEndConditionalRenderingEXT(
    VkCommandBuffer                             commandBuffer) override;


virtual bool PreCallValidateCmdSetViewportWScalingNV(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstViewport,
    uint32_t                                    viewportCount,
    const VkViewportWScalingNV*                 pViewportWScalings) const override;


virtual void PreCallRecordCmdSetViewportWScalingNV(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstViewport,
    uint32_t                                    viewportCount,
    const VkViewportWScalingNV*                 pViewportWScalings) override;


virtual void PostCallRecordCmdSetViewportWScalingNV(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstViewport,
    uint32_t                                    viewportCount,
    const VkViewportWScalingNV*                 pViewportWScalings) override;


virtual bool PreCallValidateReleaseDisplayEXT(
    VkPhysicalDevice                            physicalDevice,
    VkDisplayKHR                                display) const override;


virtual void PreCallRecordReleaseDisplayEXT(
    VkPhysicalDevice                            physicalDevice,
    VkDisplayKHR                                display) override;


virtual void PostCallRecordReleaseDisplayEXT(
    VkPhysicalDevice                            physicalDevice,
    VkDisplayKHR                                display,
    VkResult                                    result) override;


#ifdef VK_USE_PLATFORM_XLIB_XRANDR_EXT

virtual bool PreCallValidateAcquireXlibDisplayEXT(
    VkPhysicalDevice                            physicalDevice,
    Display*                                    dpy,
    VkDisplayKHR                                display) const override;


virtual void PreCallRecordAcquireXlibDisplayEXT(
    VkPhysicalDevice                            physicalDevice,
    Display*                                    dpy,
    VkDisplayKHR                                display) override;


virtual void PostCallRecordAcquireXlibDisplayEXT(
    VkPhysicalDevice                            physicalDevice,
    Display*                                    dpy,
    VkDisplayKHR                                display,
    VkResult                                    result) override;


#endif // VK_USE_PLATFORM_XLIB_XRANDR_EXT

#ifdef VK_USE_PLATFORM_XLIB_XRANDR_EXT

virtual bool PreCallValidateGetRandROutputDisplayEXT(
    VkPhysicalDevice                            physicalDevice,
    Display*                                    dpy,
    RROutput                                    rrOutput,
    VkDisplayKHR*                               pDisplay) const override;


virtual void PreCallRecordGetRandROutputDisplayEXT(
    VkPhysicalDevice                            physicalDevice,
    Display*                                    dpy,
    RROutput                                    rrOutput,
    VkDisplayKHR*                               pDisplay) override;


virtual void PostCallRecordGetRandROutputDisplayEXT(
    VkPhysicalDevice                            physicalDevice,
    Display*                                    dpy,
    RROutput                                    rrOutput,
    VkDisplayKHR*                               pDisplay,
    VkResult                                    result) override;


#endif // VK_USE_PLATFORM_XLIB_XRANDR_EXT

virtual bool PreCallValidateGetPhysicalDeviceSurfaceCapabilities2EXT(
    VkPhysicalDevice                            physicalDevice,
    VkSurfaceKHR                                surface,
    VkSurfaceCapabilities2EXT*                  pSurfaceCapabilities) const override;


virtual void PreCallRecordGetPhysicalDeviceSurfaceCapabilities2EXT(
    VkPhysicalDevice                            physicalDevice,
    VkSurfaceKHR                                surface,
    VkSurfaceCapabilities2EXT*                  pSurfaceCapabilities) override;


virtual void PostCallRecordGetPhysicalDeviceSurfaceCapabilities2EXT(
    VkPhysicalDevice                            physicalDevice,
    VkSurfaceKHR                                surface,
    VkSurfaceCapabilities2EXT*                  pSurfaceCapabilities,
    VkResult                                    result) override;


virtual bool PreCallValidateDisplayPowerControlEXT(
    VkDevice                                    device,
    VkDisplayKHR                                display,
    const VkDisplayPowerInfoEXT*                pDisplayPowerInfo) const override;


virtual void PreCallRecordDisplayPowerControlEXT(
    VkDevice                                    device,
    VkDisplayKHR                                display,
    const VkDisplayPowerInfoEXT*                pDisplayPowerInfo) override;


virtual void PostCallRecordDisplayPowerControlEXT(
    VkDevice                                    device,
    VkDisplayKHR                                display,
    const VkDisplayPowerInfoEXT*                pDisplayPowerInfo,
    VkResult                                    result) override;


virtual bool PreCallValidateRegisterDeviceEventEXT(
    VkDevice                                    device,
    const VkDeviceEventInfoEXT*                 pDeviceEventInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkFence*                                    pFence) const override;


virtual void PreCallRecordRegisterDeviceEventEXT(
    VkDevice                                    device,
    const VkDeviceEventInfoEXT*                 pDeviceEventInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkFence*                                    pFence) override;


virtual void PostCallRecordRegisterDeviceEventEXT(
    VkDevice                                    device,
    const VkDeviceEventInfoEXT*                 pDeviceEventInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkFence*                                    pFence,
    VkResult                                    result) override;


virtual bool PreCallValidateRegisterDisplayEventEXT(
    VkDevice                                    device,
    VkDisplayKHR                                display,
    const VkDisplayEventInfoEXT*                pDisplayEventInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkFence*                                    pFence) const override;


virtual void PreCallRecordRegisterDisplayEventEXT(
    VkDevice                                    device,
    VkDisplayKHR                                display,
    const VkDisplayEventInfoEXT*                pDisplayEventInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkFence*                                    pFence) override;


virtual void PostCallRecordRegisterDisplayEventEXT(
    VkDevice                                    device,
    VkDisplayKHR                                display,
    const VkDisplayEventInfoEXT*                pDisplayEventInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkFence*                                    pFence,
    VkResult                                    result) override;


virtual bool PreCallValidateGetSwapchainCounterEXT(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    VkSurfaceCounterFlagBitsEXT                 counter,
    uint64_t*                                   pCounterValue) const override;


virtual void PreCallRecordGetSwapchainCounterEXT(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    VkSurfaceCounterFlagBitsEXT                 counter,
    uint64_t*                                   pCounterValue) override;


virtual void PostCallRecordGetSwapchainCounterEXT(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    VkSurfaceCounterFlagBitsEXT                 counter,
    uint64_t*                                   pCounterValue,
    VkResult                                    result) override;


virtual bool PreCallValidateGetRefreshCycleDurationGOOGLE(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    VkRefreshCycleDurationGOOGLE*               pDisplayTimingProperties) const override;


virtual void PreCallRecordGetRefreshCycleDurationGOOGLE(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    VkRefreshCycleDurationGOOGLE*               pDisplayTimingProperties) override;


virtual void PostCallRecordGetRefreshCycleDurationGOOGLE(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    VkRefreshCycleDurationGOOGLE*               pDisplayTimingProperties,
    VkResult                                    result) override;


virtual bool PreCallValidateGetPastPresentationTimingGOOGLE(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    uint32_t*                                   pPresentationTimingCount,
    VkPastPresentationTimingGOOGLE*             pPresentationTimings) const override;


virtual void PreCallRecordGetPastPresentationTimingGOOGLE(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    uint32_t*                                   pPresentationTimingCount,
    VkPastPresentationTimingGOOGLE*             pPresentationTimings) override;


virtual void PostCallRecordGetPastPresentationTimingGOOGLE(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    uint32_t*                                   pPresentationTimingCount,
    VkPastPresentationTimingGOOGLE*             pPresentationTimings,
    VkResult                                    result) override;


virtual bool PreCallValidateCmdSetDiscardRectangleEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstDiscardRectangle,
    uint32_t                                    discardRectangleCount,
    const VkRect2D*                             pDiscardRectangles) const override;


virtual void PreCallRecordCmdSetDiscardRectangleEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstDiscardRectangle,
    uint32_t                                    discardRectangleCount,
    const VkRect2D*                             pDiscardRectangles) override;


virtual void PostCallRecordCmdSetDiscardRectangleEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstDiscardRectangle,
    uint32_t                                    discardRectangleCount,
    const VkRect2D*                             pDiscardRectangles) override;


virtual bool PreCallValidateSetHdrMetadataEXT(
    VkDevice                                    device,
    uint32_t                                    swapchainCount,
    const VkSwapchainKHR*                       pSwapchains,
    const VkHdrMetadataEXT*                     pMetadata) const override;


virtual void PreCallRecordSetHdrMetadataEXT(
    VkDevice                                    device,
    uint32_t                                    swapchainCount,
    const VkSwapchainKHR*                       pSwapchains,
    const VkHdrMetadataEXT*                     pMetadata) override;


virtual void PostCallRecordSetHdrMetadataEXT(
    VkDevice                                    device,
    uint32_t                                    swapchainCount,
    const VkSwapchainKHR*                       pSwapchains,
    const VkHdrMetadataEXT*                     pMetadata) override;


#ifdef VK_USE_PLATFORM_IOS_MVK

virtual bool PreCallValidateCreateIOSSurfaceMVK(
    VkInstance                                  instance,
    const VkIOSSurfaceCreateInfoMVK*            pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) const override;


virtual void PreCallRecordCreateIOSSurfaceMVK(
    VkInstance                                  instance,
    const VkIOSSurfaceCreateInfoMVK*            pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) override;


virtual void PostCallRecordCreateIOSSurfaceMVK(
    VkInstance                                  instance,
    const VkIOSSurfaceCreateInfoMVK*            pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface,
    VkResult                                    result) override;


#endif // VK_USE_PLATFORM_IOS_MVK

#ifdef VK_USE_PLATFORM_MACOS_MVK

virtual bool PreCallValidateCreateMacOSSurfaceMVK(
    VkInstance                                  instance,
    const VkMacOSSurfaceCreateInfoMVK*          pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) const override;


virtual void PreCallRecordCreateMacOSSurfaceMVK(
    VkInstance                                  instance,
    const VkMacOSSurfaceCreateInfoMVK*          pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) override;


virtual void PostCallRecordCreateMacOSSurfaceMVK(
    VkInstance                                  instance,
    const VkMacOSSurfaceCreateInfoMVK*          pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface,
    VkResult                                    result) override;


#endif // VK_USE_PLATFORM_MACOS_MVK

virtual bool PreCallValidateSetDebugUtilsObjectNameEXT(
    VkDevice                                    device,
    const VkDebugUtilsObjectNameInfoEXT*        pNameInfo) const override;


virtual void PreCallRecordSetDebugUtilsObjectNameEXT(
    VkDevice                                    device,
    const VkDebugUtilsObjectNameInfoEXT*        pNameInfo) override;


virtual void PostCallRecordSetDebugUtilsObjectNameEXT(
    VkDevice                                    device,
    const VkDebugUtilsObjectNameInfoEXT*        pNameInfo,
    VkResult                                    result) override;


virtual bool PreCallValidateSetDebugUtilsObjectTagEXT(
    VkDevice                                    device,
    const VkDebugUtilsObjectTagInfoEXT*         pTagInfo) const override;


virtual void PreCallRecordSetDebugUtilsObjectTagEXT(
    VkDevice                                    device,
    const VkDebugUtilsObjectTagInfoEXT*         pTagInfo) override;


virtual void PostCallRecordSetDebugUtilsObjectTagEXT(
    VkDevice                                    device,
    const VkDebugUtilsObjectTagInfoEXT*         pTagInfo,
    VkResult                                    result) override;


virtual bool PreCallValidateQueueBeginDebugUtilsLabelEXT(
    VkQueue                                     queue,
    const VkDebugUtilsLabelEXT*                 pLabelInfo) const override;


virtual void PreCallRecordQueueBeginDebugUtilsLabelEXT(
    VkQueue                                     queue,
    const VkDebugUtilsLabelEXT*                 pLabelInfo) override;


virtual void PostCallRecordQueueBeginDebugUtilsLabelEXT(
    VkQueue                                     queue,
    const VkDebugUtilsLabelEXT*                 pLabelInfo) override;


virtual bool PreCallValidateQueueEndDebugUtilsLabelEXT(
    VkQueue                                     queue) const override;


virtual void PreCallRecordQueueEndDebugUtilsLabelEXT(
    VkQueue                                     queue) override;


virtual void PostCallRecordQueueEndDebugUtilsLabelEXT(
    VkQueue                                     queue) override;


virtual bool PreCallValidateQueueInsertDebugUtilsLabelEXT(
    VkQueue                                     queue,
    const VkDebugUtilsLabelEXT*                 pLabelInfo) const override;


virtual void PreCallRecordQueueInsertDebugUtilsLabelEXT(
    VkQueue                                     queue,
    const VkDebugUtilsLabelEXT*                 pLabelInfo) override;


virtual void PostCallRecordQueueInsertDebugUtilsLabelEXT(
    VkQueue                                     queue,
    const VkDebugUtilsLabelEXT*                 pLabelInfo) override;


virtual bool PreCallValidateCmdBeginDebugUtilsLabelEXT(
    VkCommandBuffer                             commandBuffer,
    const VkDebugUtilsLabelEXT*                 pLabelInfo) const override;


virtual void PreCallRecordCmdBeginDebugUtilsLabelEXT(
    VkCommandBuffer                             commandBuffer,
    const VkDebugUtilsLabelEXT*                 pLabelInfo) override;


virtual void PostCallRecordCmdBeginDebugUtilsLabelEXT(
    VkCommandBuffer                             commandBuffer,
    const VkDebugUtilsLabelEXT*                 pLabelInfo) override;


virtual bool PreCallValidateCmdEndDebugUtilsLabelEXT(
    VkCommandBuffer                             commandBuffer) const override;


virtual void PreCallRecordCmdEndDebugUtilsLabelEXT(
    VkCommandBuffer                             commandBuffer) override;


virtual void PostCallRecordCmdEndDebugUtilsLabelEXT(
    VkCommandBuffer                             commandBuffer) override;


virtual bool PreCallValidateCmdInsertDebugUtilsLabelEXT(
    VkCommandBuffer                             commandBuffer,
    const VkDebugUtilsLabelEXT*                 pLabelInfo) const override;


virtual void PreCallRecordCmdInsertDebugUtilsLabelEXT(
    VkCommandBuffer                             commandBuffer,
    const VkDebugUtilsLabelEXT*                 pLabelInfo) override;


virtual void PostCallRecordCmdInsertDebugUtilsLabelEXT(
    VkCommandBuffer                             commandBuffer,
    const VkDebugUtilsLabelEXT*                 pLabelInfo) override;


virtual bool PreCallValidateCreateDebugUtilsMessengerEXT(
    VkInstance                                  instance,
    const VkDebugUtilsMessengerCreateInfoEXT*   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDebugUtilsMessengerEXT*                   pMessenger) const override;


virtual void PreCallRecordCreateDebugUtilsMessengerEXT(
    VkInstance                                  instance,
    const VkDebugUtilsMessengerCreateInfoEXT*   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDebugUtilsMessengerEXT*                   pMessenger) override;


virtual void PostCallRecordCreateDebugUtilsMessengerEXT(
    VkInstance                                  instance,
    const VkDebugUtilsMessengerCreateInfoEXT*   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDebugUtilsMessengerEXT*                   pMessenger,
    VkResult                                    result) override;


virtual bool PreCallValidateDestroyDebugUtilsMessengerEXT(
    VkInstance                                  instance,
    VkDebugUtilsMessengerEXT                    messenger,
    const VkAllocationCallbacks*                pAllocator) const override;


virtual void PreCallRecordDestroyDebugUtilsMessengerEXT(
    VkInstance                                  instance,
    VkDebugUtilsMessengerEXT                    messenger,
    const VkAllocationCallbacks*                pAllocator) override;


virtual void PostCallRecordDestroyDebugUtilsMessengerEXT(
    VkInstance                                  instance,
    VkDebugUtilsMessengerEXT                    messenger,
    const VkAllocationCallbacks*                pAllocator) override;


virtual bool PreCallValidateSubmitDebugUtilsMessageEXT(
    VkInstance                                  instance,
    VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT             messageTypes,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData) const override;


virtual void PreCallRecordSubmitDebugUtilsMessageEXT(
    VkInstance                                  instance,
    VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT             messageTypes,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData) override;


virtual void PostCallRecordSubmitDebugUtilsMessageEXT(
    VkInstance                                  instance,
    VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT             messageTypes,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData) override;


#ifdef VK_USE_PLATFORM_ANDROID_KHR

virtual bool PreCallValidateGetAndroidHardwareBufferPropertiesANDROID(
    VkDevice                                    device,
    const struct AHardwareBuffer*               buffer,
    VkAndroidHardwareBufferPropertiesANDROID*   pProperties) const override;


virtual void PreCallRecordGetAndroidHardwareBufferPropertiesANDROID(
    VkDevice                                    device,
    const struct AHardwareBuffer*               buffer,
    VkAndroidHardwareBufferPropertiesANDROID*   pProperties) override;


virtual void PostCallRecordGetAndroidHardwareBufferPropertiesANDROID(
    VkDevice                                    device,
    const struct AHardwareBuffer*               buffer,
    VkAndroidHardwareBufferPropertiesANDROID*   pProperties,
    VkResult                                    result) override;


#endif // VK_USE_PLATFORM_ANDROID_KHR

#ifdef VK_USE_PLATFORM_ANDROID_KHR

virtual bool PreCallValidateGetMemoryAndroidHardwareBufferANDROID(
    VkDevice                                    device,
    const VkMemoryGetAndroidHardwareBufferInfoANDROID* pInfo,
    struct AHardwareBuffer**                    pBuffer) const override;


virtual void PreCallRecordGetMemoryAndroidHardwareBufferANDROID(
    VkDevice                                    device,
    const VkMemoryGetAndroidHardwareBufferInfoANDROID* pInfo,
    struct AHardwareBuffer**                    pBuffer) override;


virtual void PostCallRecordGetMemoryAndroidHardwareBufferANDROID(
    VkDevice                                    device,
    const VkMemoryGetAndroidHardwareBufferInfoANDROID* pInfo,
    struct AHardwareBuffer**                    pBuffer,
    VkResult                                    result) override;


#endif // VK_USE_PLATFORM_ANDROID_KHR

virtual bool PreCallValidateCmdSetSampleLocationsEXT(
    VkCommandBuffer                             commandBuffer,
    const VkSampleLocationsInfoEXT*             pSampleLocationsInfo) const override;


virtual void PreCallRecordCmdSetSampleLocationsEXT(
    VkCommandBuffer                             commandBuffer,
    const VkSampleLocationsInfoEXT*             pSampleLocationsInfo) override;


virtual void PostCallRecordCmdSetSampleLocationsEXT(
    VkCommandBuffer                             commandBuffer,
    const VkSampleLocationsInfoEXT*             pSampleLocationsInfo) override;


virtual bool PreCallValidateGetPhysicalDeviceMultisamplePropertiesEXT(
    VkPhysicalDevice                            physicalDevice,
    VkSampleCountFlagBits                       samples,
    VkMultisamplePropertiesEXT*                 pMultisampleProperties) const override;


virtual void PreCallRecordGetPhysicalDeviceMultisamplePropertiesEXT(
    VkPhysicalDevice                            physicalDevice,
    VkSampleCountFlagBits                       samples,
    VkMultisamplePropertiesEXT*                 pMultisampleProperties) override;


virtual void PostCallRecordGetPhysicalDeviceMultisamplePropertiesEXT(
    VkPhysicalDevice                            physicalDevice,
    VkSampleCountFlagBits                       samples,
    VkMultisamplePropertiesEXT*                 pMultisampleProperties) override;


virtual bool PreCallValidateGetImageDrmFormatModifierPropertiesEXT(
    VkDevice                                    device,
    VkImage                                     image,
    VkImageDrmFormatModifierPropertiesEXT*      pProperties) const override;


virtual void PreCallRecordGetImageDrmFormatModifierPropertiesEXT(
    VkDevice                                    device,
    VkImage                                     image,
    VkImageDrmFormatModifierPropertiesEXT*      pProperties) override;


virtual void PostCallRecordGetImageDrmFormatModifierPropertiesEXT(
    VkDevice                                    device,
    VkImage                                     image,
    VkImageDrmFormatModifierPropertiesEXT*      pProperties,
    VkResult                                    result) override;


virtual bool PreCallValidateCmdBindShadingRateImageNV(
    VkCommandBuffer                             commandBuffer,
    VkImageView                                 imageView,
    VkImageLayout                               imageLayout) const override;


virtual void PreCallRecordCmdBindShadingRateImageNV(
    VkCommandBuffer                             commandBuffer,
    VkImageView                                 imageView,
    VkImageLayout                               imageLayout) override;


virtual void PostCallRecordCmdBindShadingRateImageNV(
    VkCommandBuffer                             commandBuffer,
    VkImageView                                 imageView,
    VkImageLayout                               imageLayout) override;


virtual bool PreCallValidateCmdSetViewportShadingRatePaletteNV(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstViewport,
    uint32_t                                    viewportCount,
    const VkShadingRatePaletteNV*               pShadingRatePalettes) const override;


virtual void PreCallRecordCmdSetViewportShadingRatePaletteNV(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstViewport,
    uint32_t                                    viewportCount,
    const VkShadingRatePaletteNV*               pShadingRatePalettes) override;


virtual void PostCallRecordCmdSetViewportShadingRatePaletteNV(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstViewport,
    uint32_t                                    viewportCount,
    const VkShadingRatePaletteNV*               pShadingRatePalettes) override;


virtual bool PreCallValidateCmdSetCoarseSampleOrderNV(
    VkCommandBuffer                             commandBuffer,
    VkCoarseSampleOrderTypeNV                   sampleOrderType,
    uint32_t                                    customSampleOrderCount,
    const VkCoarseSampleOrderCustomNV*          pCustomSampleOrders) const override;


virtual void PreCallRecordCmdSetCoarseSampleOrderNV(
    VkCommandBuffer                             commandBuffer,
    VkCoarseSampleOrderTypeNV                   sampleOrderType,
    uint32_t                                    customSampleOrderCount,
    const VkCoarseSampleOrderCustomNV*          pCustomSampleOrders) override;


virtual void PostCallRecordCmdSetCoarseSampleOrderNV(
    VkCommandBuffer                             commandBuffer,
    VkCoarseSampleOrderTypeNV                   sampleOrderType,
    uint32_t                                    customSampleOrderCount,
    const VkCoarseSampleOrderCustomNV*          pCustomSampleOrders) override;


virtual bool PreCallValidateCreateAccelerationStructureNV(
    VkDevice                                    device,
    const VkAccelerationStructureCreateInfoNV*  pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkAccelerationStructureNV*                  pAccelerationStructure) const override;


virtual void PreCallRecordCreateAccelerationStructureNV(
    VkDevice                                    device,
    const VkAccelerationStructureCreateInfoNV*  pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkAccelerationStructureNV*                  pAccelerationStructure) override;


virtual void PostCallRecordCreateAccelerationStructureNV(
    VkDevice                                    device,
    const VkAccelerationStructureCreateInfoNV*  pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkAccelerationStructureNV*                  pAccelerationStructure,
    VkResult                                    result) override;


virtual bool PreCallValidateDestroyAccelerationStructureKHR(
    VkDevice                                    device,
    VkAccelerationStructureKHR                  accelerationStructure,
    const VkAllocationCallbacks*                pAllocator) const override;


virtual void PreCallRecordDestroyAccelerationStructureKHR(
    VkDevice                                    device,
    VkAccelerationStructureKHR                  accelerationStructure,
    const VkAllocationCallbacks*                pAllocator) override;


virtual void PostCallRecordDestroyAccelerationStructureKHR(
    VkDevice                                    device,
    VkAccelerationStructureKHR                  accelerationStructure,
    const VkAllocationCallbacks*                pAllocator) override;


virtual bool PreCallValidateDestroyAccelerationStructureNV(
    VkDevice                                    device,
    VkAccelerationStructureKHR                  accelerationStructure,
    const VkAllocationCallbacks*                pAllocator) const override;


virtual void PreCallRecordDestroyAccelerationStructureNV(
    VkDevice                                    device,
    VkAccelerationStructureKHR                  accelerationStructure,
    const VkAllocationCallbacks*                pAllocator) override;


virtual void PostCallRecordDestroyAccelerationStructureNV(
    VkDevice                                    device,
    VkAccelerationStructureKHR                  accelerationStructure,
    const VkAllocationCallbacks*                pAllocator) override;


virtual bool PreCallValidateGetAccelerationStructureMemoryRequirementsNV(
    VkDevice                                    device,
    const VkAccelerationStructureMemoryRequirementsInfoNV* pInfo,
    VkMemoryRequirements2KHR*                   pMemoryRequirements) const override;


virtual void PreCallRecordGetAccelerationStructureMemoryRequirementsNV(
    VkDevice                                    device,
    const VkAccelerationStructureMemoryRequirementsInfoNV* pInfo,
    VkMemoryRequirements2KHR*                   pMemoryRequirements) override;


virtual void PostCallRecordGetAccelerationStructureMemoryRequirementsNV(
    VkDevice                                    device,
    const VkAccelerationStructureMemoryRequirementsInfoNV* pInfo,
    VkMemoryRequirements2KHR*                   pMemoryRequirements) override;


virtual bool PreCallValidateBindAccelerationStructureMemoryKHR(
    VkDevice                                    device,
    uint32_t                                    bindInfoCount,
    const VkBindAccelerationStructureMemoryInfoKHR* pBindInfos) const override;


virtual void PreCallRecordBindAccelerationStructureMemoryKHR(
    VkDevice                                    device,
    uint32_t                                    bindInfoCount,
    const VkBindAccelerationStructureMemoryInfoKHR* pBindInfos) override;


virtual void PostCallRecordBindAccelerationStructureMemoryKHR(
    VkDevice                                    device,
    uint32_t                                    bindInfoCount,
    const VkBindAccelerationStructureMemoryInfoKHR* pBindInfos,
    VkResult                                    result) override;


virtual bool PreCallValidateBindAccelerationStructureMemoryNV(
    VkDevice                                    device,
    uint32_t                                    bindInfoCount,
    const VkBindAccelerationStructureMemoryInfoKHR* pBindInfos) const override;


virtual void PreCallRecordBindAccelerationStructureMemoryNV(
    VkDevice                                    device,
    uint32_t                                    bindInfoCount,
    const VkBindAccelerationStructureMemoryInfoKHR* pBindInfos) override;


virtual void PostCallRecordBindAccelerationStructureMemoryNV(
    VkDevice                                    device,
    uint32_t                                    bindInfoCount,
    const VkBindAccelerationStructureMemoryInfoKHR* pBindInfos,
    VkResult                                    result) override;


virtual bool PreCallValidateCmdBuildAccelerationStructureNV(
    VkCommandBuffer                             commandBuffer,
    const VkAccelerationStructureInfoNV*        pInfo,
    VkBuffer                                    instanceData,
    VkDeviceSize                                instanceOffset,
    VkBool32                                    update,
    VkAccelerationStructureKHR                  dst,
    VkAccelerationStructureKHR                  src,
    VkBuffer                                    scratch,
    VkDeviceSize                                scratchOffset) const override;


virtual void PreCallRecordCmdBuildAccelerationStructureNV(
    VkCommandBuffer                             commandBuffer,
    const VkAccelerationStructureInfoNV*        pInfo,
    VkBuffer                                    instanceData,
    VkDeviceSize                                instanceOffset,
    VkBool32                                    update,
    VkAccelerationStructureKHR                  dst,
    VkAccelerationStructureKHR                  src,
    VkBuffer                                    scratch,
    VkDeviceSize                                scratchOffset) override;


virtual void PostCallRecordCmdBuildAccelerationStructureNV(
    VkCommandBuffer                             commandBuffer,
    const VkAccelerationStructureInfoNV*        pInfo,
    VkBuffer                                    instanceData,
    VkDeviceSize                                instanceOffset,
    VkBool32                                    update,
    VkAccelerationStructureKHR                  dst,
    VkAccelerationStructureKHR                  src,
    VkBuffer                                    scratch,
    VkDeviceSize                                scratchOffset) override;


virtual bool PreCallValidateCmdCopyAccelerationStructureNV(
    VkCommandBuffer                             commandBuffer,
    VkAccelerationStructureKHR                  dst,
    VkAccelerationStructureKHR                  src,
    VkCopyAccelerationStructureModeKHR          mode) const override;


virtual void PreCallRecordCmdCopyAccelerationStructureNV(
    VkCommandBuffer                             commandBuffer,
    VkAccelerationStructureKHR                  dst,
    VkAccelerationStructureKHR                  src,
    VkCopyAccelerationStructureModeKHR          mode) override;


virtual void PostCallRecordCmdCopyAccelerationStructureNV(
    VkCommandBuffer                             commandBuffer,
    VkAccelerationStructureKHR                  dst,
    VkAccelerationStructureKHR                  src,
    VkCopyAccelerationStructureModeKHR          mode) override;


virtual bool PreCallValidateCmdTraceRaysNV(
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
    uint32_t                                    depth) const override;


virtual void PreCallRecordCmdTraceRaysNV(
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
    uint32_t                                    depth) override;


virtual void PostCallRecordCmdTraceRaysNV(
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
    uint32_t                                    depth) override;


virtual bool PreCallValidateCreateRayTracingPipelinesNV(
    VkDevice                                    device,
    VkPipelineCache                             pipelineCache,
    uint32_t                                    createInfoCount,
    const VkRayTracingPipelineCreateInfoNV*     pCreateInfos,
    const VkAllocationCallbacks*                pAllocator,
    VkPipeline*                                 pPipelines,
    void*                                       state_data) const override;


virtual void PreCallRecordCreateRayTracingPipelinesNV(
    VkDevice                                    device,
    VkPipelineCache                             pipelineCache,
    uint32_t                                    createInfoCount,
    const VkRayTracingPipelineCreateInfoNV*     pCreateInfos,
    const VkAllocationCallbacks*                pAllocator,
    VkPipeline*                                 pPipelines,
    void*                                       state_data) override;


virtual void PostCallRecordCreateRayTracingPipelinesNV(
    VkDevice                                    device,
    VkPipelineCache                             pipelineCache,
    uint32_t                                    createInfoCount,
    const VkRayTracingPipelineCreateInfoNV*     pCreateInfos,
    const VkAllocationCallbacks*                pAllocator,
    VkPipeline*                                 pPipelines,
    VkResult                                    result,
    void*                                       state_data) override;


virtual bool PreCallValidateGetRayTracingShaderGroupHandlesKHR(
    VkDevice                                    device,
    VkPipeline                                  pipeline,
    uint32_t                                    firstGroup,
    uint32_t                                    groupCount,
    size_t                                      dataSize,
    void*                                       pData) const override;


virtual void PreCallRecordGetRayTracingShaderGroupHandlesKHR(
    VkDevice                                    device,
    VkPipeline                                  pipeline,
    uint32_t                                    firstGroup,
    uint32_t                                    groupCount,
    size_t                                      dataSize,
    void*                                       pData) override;


virtual void PostCallRecordGetRayTracingShaderGroupHandlesKHR(
    VkDevice                                    device,
    VkPipeline                                  pipeline,
    uint32_t                                    firstGroup,
    uint32_t                                    groupCount,
    size_t                                      dataSize,
    void*                                       pData,
    VkResult                                    result) override;


virtual bool PreCallValidateGetRayTracingShaderGroupHandlesNV(
    VkDevice                                    device,
    VkPipeline                                  pipeline,
    uint32_t                                    firstGroup,
    uint32_t                                    groupCount,
    size_t                                      dataSize,
    void*                                       pData) const override;


virtual void PreCallRecordGetRayTracingShaderGroupHandlesNV(
    VkDevice                                    device,
    VkPipeline                                  pipeline,
    uint32_t                                    firstGroup,
    uint32_t                                    groupCount,
    size_t                                      dataSize,
    void*                                       pData) override;


virtual void PostCallRecordGetRayTracingShaderGroupHandlesNV(
    VkDevice                                    device,
    VkPipeline                                  pipeline,
    uint32_t                                    firstGroup,
    uint32_t                                    groupCount,
    size_t                                      dataSize,
    void*                                       pData,
    VkResult                                    result) override;


virtual bool PreCallValidateGetAccelerationStructureHandleNV(
    VkDevice                                    device,
    VkAccelerationStructureKHR                  accelerationStructure,
    size_t                                      dataSize,
    void*                                       pData) const override;


virtual void PreCallRecordGetAccelerationStructureHandleNV(
    VkDevice                                    device,
    VkAccelerationStructureKHR                  accelerationStructure,
    size_t                                      dataSize,
    void*                                       pData) override;


virtual void PostCallRecordGetAccelerationStructureHandleNV(
    VkDevice                                    device,
    VkAccelerationStructureKHR                  accelerationStructure,
    size_t                                      dataSize,
    void*                                       pData,
    VkResult                                    result) override;


virtual bool PreCallValidateCmdWriteAccelerationStructuresPropertiesKHR(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    accelerationStructureCount,
    const VkAccelerationStructureKHR*           pAccelerationStructures,
    VkQueryType                                 queryType,
    VkQueryPool                                 queryPool,
    uint32_t                                    firstQuery) const override;


virtual void PreCallRecordCmdWriteAccelerationStructuresPropertiesKHR(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    accelerationStructureCount,
    const VkAccelerationStructureKHR*           pAccelerationStructures,
    VkQueryType                                 queryType,
    VkQueryPool                                 queryPool,
    uint32_t                                    firstQuery) override;


virtual void PostCallRecordCmdWriteAccelerationStructuresPropertiesKHR(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    accelerationStructureCount,
    const VkAccelerationStructureKHR*           pAccelerationStructures,
    VkQueryType                                 queryType,
    VkQueryPool                                 queryPool,
    uint32_t                                    firstQuery) override;


virtual bool PreCallValidateCmdWriteAccelerationStructuresPropertiesNV(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    accelerationStructureCount,
    const VkAccelerationStructureKHR*           pAccelerationStructures,
    VkQueryType                                 queryType,
    VkQueryPool                                 queryPool,
    uint32_t                                    firstQuery) const override;


virtual void PreCallRecordCmdWriteAccelerationStructuresPropertiesNV(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    accelerationStructureCount,
    const VkAccelerationStructureKHR*           pAccelerationStructures,
    VkQueryType                                 queryType,
    VkQueryPool                                 queryPool,
    uint32_t                                    firstQuery) override;


virtual void PostCallRecordCmdWriteAccelerationStructuresPropertiesNV(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    accelerationStructureCount,
    const VkAccelerationStructureKHR*           pAccelerationStructures,
    VkQueryType                                 queryType,
    VkQueryPool                                 queryPool,
    uint32_t                                    firstQuery) override;


virtual bool PreCallValidateCompileDeferredNV(
    VkDevice                                    device,
    VkPipeline                                  pipeline,
    uint32_t                                    shader) const override;


virtual void PreCallRecordCompileDeferredNV(
    VkDevice                                    device,
    VkPipeline                                  pipeline,
    uint32_t                                    shader) override;


virtual void PostCallRecordCompileDeferredNV(
    VkDevice                                    device,
    VkPipeline                                  pipeline,
    uint32_t                                    shader,
    VkResult                                    result) override;


virtual bool PreCallValidateGetMemoryHostPointerPropertiesEXT(
    VkDevice                                    device,
    VkExternalMemoryHandleTypeFlagBits          handleType,
    const void*                                 pHostPointer,
    VkMemoryHostPointerPropertiesEXT*           pMemoryHostPointerProperties) const override;


virtual void PreCallRecordGetMemoryHostPointerPropertiesEXT(
    VkDevice                                    device,
    VkExternalMemoryHandleTypeFlagBits          handleType,
    const void*                                 pHostPointer,
    VkMemoryHostPointerPropertiesEXT*           pMemoryHostPointerProperties) override;


virtual void PostCallRecordGetMemoryHostPointerPropertiesEXT(
    VkDevice                                    device,
    VkExternalMemoryHandleTypeFlagBits          handleType,
    const void*                                 pHostPointer,
    VkMemoryHostPointerPropertiesEXT*           pMemoryHostPointerProperties,
    VkResult                                    result) override;


virtual bool PreCallValidateCmdWriteBufferMarkerAMD(
    VkCommandBuffer                             commandBuffer,
    VkPipelineStageFlagBits                     pipelineStage,
    VkBuffer                                    dstBuffer,
    VkDeviceSize                                dstOffset,
    uint32_t                                    marker) const override;


virtual void PreCallRecordCmdWriteBufferMarkerAMD(
    VkCommandBuffer                             commandBuffer,
    VkPipelineStageFlagBits                     pipelineStage,
    VkBuffer                                    dstBuffer,
    VkDeviceSize                                dstOffset,
    uint32_t                                    marker) override;


virtual void PostCallRecordCmdWriteBufferMarkerAMD(
    VkCommandBuffer                             commandBuffer,
    VkPipelineStageFlagBits                     pipelineStage,
    VkBuffer                                    dstBuffer,
    VkDeviceSize                                dstOffset,
    uint32_t                                    marker) override;


virtual bool PreCallValidateGetPhysicalDeviceCalibrateableTimeDomainsEXT(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pTimeDomainCount,
    VkTimeDomainEXT*                            pTimeDomains) const override;


virtual void PreCallRecordGetPhysicalDeviceCalibrateableTimeDomainsEXT(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pTimeDomainCount,
    VkTimeDomainEXT*                            pTimeDomains) override;


virtual void PostCallRecordGetPhysicalDeviceCalibrateableTimeDomainsEXT(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pTimeDomainCount,
    VkTimeDomainEXT*                            pTimeDomains,
    VkResult                                    result) override;


virtual bool PreCallValidateGetCalibratedTimestampsEXT(
    VkDevice                                    device,
    uint32_t                                    timestampCount,
    const VkCalibratedTimestampInfoEXT*         pTimestampInfos,
    uint64_t*                                   pTimestamps,
    uint64_t*                                   pMaxDeviation) const override;


virtual void PreCallRecordGetCalibratedTimestampsEXT(
    VkDevice                                    device,
    uint32_t                                    timestampCount,
    const VkCalibratedTimestampInfoEXT*         pTimestampInfos,
    uint64_t*                                   pTimestamps,
    uint64_t*                                   pMaxDeviation) override;


virtual void PostCallRecordGetCalibratedTimestampsEXT(
    VkDevice                                    device,
    uint32_t                                    timestampCount,
    const VkCalibratedTimestampInfoEXT*         pTimestampInfos,
    uint64_t*                                   pTimestamps,
    uint64_t*                                   pMaxDeviation,
    VkResult                                    result) override;


virtual bool PreCallValidateCmdDrawMeshTasksNV(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    taskCount,
    uint32_t                                    firstTask) const override;


virtual void PreCallRecordCmdDrawMeshTasksNV(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    taskCount,
    uint32_t                                    firstTask) override;


virtual void PostCallRecordCmdDrawMeshTasksNV(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    taskCount,
    uint32_t                                    firstTask) override;


virtual bool PreCallValidateCmdDrawMeshTasksIndirectNV(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    uint32_t                                    drawCount,
    uint32_t                                    stride) const override;


virtual void PreCallRecordCmdDrawMeshTasksIndirectNV(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    uint32_t                                    drawCount,
    uint32_t                                    stride) override;


virtual void PostCallRecordCmdDrawMeshTasksIndirectNV(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    uint32_t                                    drawCount,
    uint32_t                                    stride) override;


virtual bool PreCallValidateCmdDrawMeshTasksIndirectCountNV(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkBuffer                                    countBuffer,
    VkDeviceSize                                countBufferOffset,
    uint32_t                                    maxDrawCount,
    uint32_t                                    stride) const override;


virtual void PreCallRecordCmdDrawMeshTasksIndirectCountNV(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkBuffer                                    countBuffer,
    VkDeviceSize                                countBufferOffset,
    uint32_t                                    maxDrawCount,
    uint32_t                                    stride) override;


virtual void PostCallRecordCmdDrawMeshTasksIndirectCountNV(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkBuffer                                    countBuffer,
    VkDeviceSize                                countBufferOffset,
    uint32_t                                    maxDrawCount,
    uint32_t                                    stride) override;


virtual bool PreCallValidateCmdSetExclusiveScissorNV(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstExclusiveScissor,
    uint32_t                                    exclusiveScissorCount,
    const VkRect2D*                             pExclusiveScissors) const override;


virtual void PreCallRecordCmdSetExclusiveScissorNV(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstExclusiveScissor,
    uint32_t                                    exclusiveScissorCount,
    const VkRect2D*                             pExclusiveScissors) override;


virtual void PostCallRecordCmdSetExclusiveScissorNV(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstExclusiveScissor,
    uint32_t                                    exclusiveScissorCount,
    const VkRect2D*                             pExclusiveScissors) override;


virtual bool PreCallValidateCmdSetCheckpointNV(
    VkCommandBuffer                             commandBuffer,
    const void*                                 pCheckpointMarker) const override;


virtual void PreCallRecordCmdSetCheckpointNV(
    VkCommandBuffer                             commandBuffer,
    const void*                                 pCheckpointMarker) override;


virtual void PostCallRecordCmdSetCheckpointNV(
    VkCommandBuffer                             commandBuffer,
    const void*                                 pCheckpointMarker) override;


virtual bool PreCallValidateGetQueueCheckpointDataNV(
    VkQueue                                     queue,
    uint32_t*                                   pCheckpointDataCount,
    VkCheckpointDataNV*                         pCheckpointData) const override;


virtual void PreCallRecordGetQueueCheckpointDataNV(
    VkQueue                                     queue,
    uint32_t*                                   pCheckpointDataCount,
    VkCheckpointDataNV*                         pCheckpointData) override;


virtual void PostCallRecordGetQueueCheckpointDataNV(
    VkQueue                                     queue,
    uint32_t*                                   pCheckpointDataCount,
    VkCheckpointDataNV*                         pCheckpointData) override;


virtual bool PreCallValidateInitializePerformanceApiINTEL(
    VkDevice                                    device,
    const VkInitializePerformanceApiInfoINTEL*  pInitializeInfo) const override;


virtual void PreCallRecordInitializePerformanceApiINTEL(
    VkDevice                                    device,
    const VkInitializePerformanceApiInfoINTEL*  pInitializeInfo) override;


virtual void PostCallRecordInitializePerformanceApiINTEL(
    VkDevice                                    device,
    const VkInitializePerformanceApiInfoINTEL*  pInitializeInfo,
    VkResult                                    result) override;


virtual bool PreCallValidateUninitializePerformanceApiINTEL(
    VkDevice                                    device) const override;


virtual void PreCallRecordUninitializePerformanceApiINTEL(
    VkDevice                                    device) override;


virtual void PostCallRecordUninitializePerformanceApiINTEL(
    VkDevice                                    device) override;


virtual bool PreCallValidateCmdSetPerformanceMarkerINTEL(
    VkCommandBuffer                             commandBuffer,
    const VkPerformanceMarkerInfoINTEL*         pMarkerInfo) const override;


virtual void PreCallRecordCmdSetPerformanceMarkerINTEL(
    VkCommandBuffer                             commandBuffer,
    const VkPerformanceMarkerInfoINTEL*         pMarkerInfo) override;


virtual void PostCallRecordCmdSetPerformanceMarkerINTEL(
    VkCommandBuffer                             commandBuffer,
    const VkPerformanceMarkerInfoINTEL*         pMarkerInfo,
    VkResult                                    result) override;


virtual bool PreCallValidateCmdSetPerformanceStreamMarkerINTEL(
    VkCommandBuffer                             commandBuffer,
    const VkPerformanceStreamMarkerInfoINTEL*   pMarkerInfo) const override;


virtual void PreCallRecordCmdSetPerformanceStreamMarkerINTEL(
    VkCommandBuffer                             commandBuffer,
    const VkPerformanceStreamMarkerInfoINTEL*   pMarkerInfo) override;


virtual void PostCallRecordCmdSetPerformanceStreamMarkerINTEL(
    VkCommandBuffer                             commandBuffer,
    const VkPerformanceStreamMarkerInfoINTEL*   pMarkerInfo,
    VkResult                                    result) override;


virtual bool PreCallValidateCmdSetPerformanceOverrideINTEL(
    VkCommandBuffer                             commandBuffer,
    const VkPerformanceOverrideInfoINTEL*       pOverrideInfo) const override;


virtual void PreCallRecordCmdSetPerformanceOverrideINTEL(
    VkCommandBuffer                             commandBuffer,
    const VkPerformanceOverrideInfoINTEL*       pOverrideInfo) override;


virtual void PostCallRecordCmdSetPerformanceOverrideINTEL(
    VkCommandBuffer                             commandBuffer,
    const VkPerformanceOverrideInfoINTEL*       pOverrideInfo,
    VkResult                                    result) override;


virtual bool PreCallValidateAcquirePerformanceConfigurationINTEL(
    VkDevice                                    device,
    const VkPerformanceConfigurationAcquireInfoINTEL* pAcquireInfo,
    VkPerformanceConfigurationINTEL*            pConfiguration) const override;


virtual void PreCallRecordAcquirePerformanceConfigurationINTEL(
    VkDevice                                    device,
    const VkPerformanceConfigurationAcquireInfoINTEL* pAcquireInfo,
    VkPerformanceConfigurationINTEL*            pConfiguration) override;


virtual void PostCallRecordAcquirePerformanceConfigurationINTEL(
    VkDevice                                    device,
    const VkPerformanceConfigurationAcquireInfoINTEL* pAcquireInfo,
    VkPerformanceConfigurationINTEL*            pConfiguration,
    VkResult                                    result) override;


virtual bool PreCallValidateReleasePerformanceConfigurationINTEL(
    VkDevice                                    device,
    VkPerformanceConfigurationINTEL             configuration) const override;


virtual void PreCallRecordReleasePerformanceConfigurationINTEL(
    VkDevice                                    device,
    VkPerformanceConfigurationINTEL             configuration) override;


virtual void PostCallRecordReleasePerformanceConfigurationINTEL(
    VkDevice                                    device,
    VkPerformanceConfigurationINTEL             configuration,
    VkResult                                    result) override;


virtual bool PreCallValidateQueueSetPerformanceConfigurationINTEL(
    VkQueue                                     queue,
    VkPerformanceConfigurationINTEL             configuration) const override;


virtual void PreCallRecordQueueSetPerformanceConfigurationINTEL(
    VkQueue                                     queue,
    VkPerformanceConfigurationINTEL             configuration) override;


virtual void PostCallRecordQueueSetPerformanceConfigurationINTEL(
    VkQueue                                     queue,
    VkPerformanceConfigurationINTEL             configuration,
    VkResult                                    result) override;


virtual bool PreCallValidateGetPerformanceParameterINTEL(
    VkDevice                                    device,
    VkPerformanceParameterTypeINTEL             parameter,
    VkPerformanceValueINTEL*                    pValue) const override;


virtual void PreCallRecordGetPerformanceParameterINTEL(
    VkDevice                                    device,
    VkPerformanceParameterTypeINTEL             parameter,
    VkPerformanceValueINTEL*                    pValue) override;


virtual void PostCallRecordGetPerformanceParameterINTEL(
    VkDevice                                    device,
    VkPerformanceParameterTypeINTEL             parameter,
    VkPerformanceValueINTEL*                    pValue,
    VkResult                                    result) override;


virtual bool PreCallValidateSetLocalDimmingAMD(
    VkDevice                                    device,
    VkSwapchainKHR                              swapChain,
    VkBool32                                    localDimmingEnable) const override;


virtual void PreCallRecordSetLocalDimmingAMD(
    VkDevice                                    device,
    VkSwapchainKHR                              swapChain,
    VkBool32                                    localDimmingEnable) override;


virtual void PostCallRecordSetLocalDimmingAMD(
    VkDevice                                    device,
    VkSwapchainKHR                              swapChain,
    VkBool32                                    localDimmingEnable) override;


#ifdef VK_USE_PLATFORM_FUCHSIA

virtual bool PreCallValidateCreateImagePipeSurfaceFUCHSIA(
    VkInstance                                  instance,
    const VkImagePipeSurfaceCreateInfoFUCHSIA*  pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) const override;


virtual void PreCallRecordCreateImagePipeSurfaceFUCHSIA(
    VkInstance                                  instance,
    const VkImagePipeSurfaceCreateInfoFUCHSIA*  pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) override;


virtual void PostCallRecordCreateImagePipeSurfaceFUCHSIA(
    VkInstance                                  instance,
    const VkImagePipeSurfaceCreateInfoFUCHSIA*  pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface,
    VkResult                                    result) override;


#endif // VK_USE_PLATFORM_FUCHSIA

#ifdef VK_USE_PLATFORM_METAL_EXT

virtual bool PreCallValidateCreateMetalSurfaceEXT(
    VkInstance                                  instance,
    const VkMetalSurfaceCreateInfoEXT*          pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) const override;


virtual void PreCallRecordCreateMetalSurfaceEXT(
    VkInstance                                  instance,
    const VkMetalSurfaceCreateInfoEXT*          pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) override;


virtual void PostCallRecordCreateMetalSurfaceEXT(
    VkInstance                                  instance,
    const VkMetalSurfaceCreateInfoEXT*          pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface,
    VkResult                                    result) override;


#endif // VK_USE_PLATFORM_METAL_EXT

virtual bool PreCallValidateGetBufferDeviceAddressEXT(
    VkDevice                                    device,
    const VkBufferDeviceAddressInfo*            pInfo) const override;


virtual void PreCallRecordGetBufferDeviceAddressEXT(
    VkDevice                                    device,
    const VkBufferDeviceAddressInfo*            pInfo) override;


virtual void PostCallRecordGetBufferDeviceAddressEXT(
    VkDevice                                    device,
    const VkBufferDeviceAddressInfo*            pInfo,
    VkResult                                    result) override;


virtual bool PreCallValidateGetPhysicalDeviceToolPropertiesEXT(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pToolCount,
    VkPhysicalDeviceToolPropertiesEXT*          pToolProperties) const override;


virtual void PreCallRecordGetPhysicalDeviceToolPropertiesEXT(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pToolCount,
    VkPhysicalDeviceToolPropertiesEXT*          pToolProperties) override;


virtual void PostCallRecordGetPhysicalDeviceToolPropertiesEXT(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pToolCount,
    VkPhysicalDeviceToolPropertiesEXT*          pToolProperties,
    VkResult                                    result) override;


virtual bool PreCallValidateGetPhysicalDeviceCooperativeMatrixPropertiesNV(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pPropertyCount,
    VkCooperativeMatrixPropertiesNV*            pProperties) const override;


virtual void PreCallRecordGetPhysicalDeviceCooperativeMatrixPropertiesNV(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pPropertyCount,
    VkCooperativeMatrixPropertiesNV*            pProperties) override;


virtual void PostCallRecordGetPhysicalDeviceCooperativeMatrixPropertiesNV(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pPropertyCount,
    VkCooperativeMatrixPropertiesNV*            pProperties,
    VkResult                                    result) override;


virtual bool PreCallValidateGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pCombinationCount,
    VkFramebufferMixedSamplesCombinationNV*     pCombinations) const override;


virtual void PreCallRecordGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pCombinationCount,
    VkFramebufferMixedSamplesCombinationNV*     pCombinations) override;


virtual void PostCallRecordGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pCombinationCount,
    VkFramebufferMixedSamplesCombinationNV*     pCombinations,
    VkResult                                    result) override;


#ifdef VK_USE_PLATFORM_WIN32_KHR

virtual bool PreCallValidateGetPhysicalDeviceSurfacePresentModes2EXT(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceSurfaceInfo2KHR*      pSurfaceInfo,
    uint32_t*                                   pPresentModeCount,
    VkPresentModeKHR*                           pPresentModes) const override;


virtual void PreCallRecordGetPhysicalDeviceSurfacePresentModes2EXT(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceSurfaceInfo2KHR*      pSurfaceInfo,
    uint32_t*                                   pPresentModeCount,
    VkPresentModeKHR*                           pPresentModes) override;


virtual void PostCallRecordGetPhysicalDeviceSurfacePresentModes2EXT(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceSurfaceInfo2KHR*      pSurfaceInfo,
    uint32_t*                                   pPresentModeCount,
    VkPresentModeKHR*                           pPresentModes,
    VkResult                                    result) override;


#endif // VK_USE_PLATFORM_WIN32_KHR

#ifdef VK_USE_PLATFORM_WIN32_KHR

virtual bool PreCallValidateAcquireFullScreenExclusiveModeEXT(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain) const override;


virtual void PreCallRecordAcquireFullScreenExclusiveModeEXT(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain) override;


virtual void PostCallRecordAcquireFullScreenExclusiveModeEXT(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    VkResult                                    result) override;


#endif // VK_USE_PLATFORM_WIN32_KHR

#ifdef VK_USE_PLATFORM_WIN32_KHR

virtual bool PreCallValidateReleaseFullScreenExclusiveModeEXT(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain) const override;


virtual void PreCallRecordReleaseFullScreenExclusiveModeEXT(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain) override;


virtual void PostCallRecordReleaseFullScreenExclusiveModeEXT(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    VkResult                                    result) override;


#endif // VK_USE_PLATFORM_WIN32_KHR

#ifdef VK_USE_PLATFORM_WIN32_KHR

virtual bool PreCallValidateGetDeviceGroupSurfacePresentModes2EXT(
    VkDevice                                    device,
    const VkPhysicalDeviceSurfaceInfo2KHR*      pSurfaceInfo,
    VkDeviceGroupPresentModeFlagsKHR*           pModes) const override;


virtual void PreCallRecordGetDeviceGroupSurfacePresentModes2EXT(
    VkDevice                                    device,
    const VkPhysicalDeviceSurfaceInfo2KHR*      pSurfaceInfo,
    VkDeviceGroupPresentModeFlagsKHR*           pModes) override;


virtual void PostCallRecordGetDeviceGroupSurfacePresentModes2EXT(
    VkDevice                                    device,
    const VkPhysicalDeviceSurfaceInfo2KHR*      pSurfaceInfo,
    VkDeviceGroupPresentModeFlagsKHR*           pModes,
    VkResult                                    result) override;


#endif // VK_USE_PLATFORM_WIN32_KHR

virtual bool PreCallValidateCreateHeadlessSurfaceEXT(
    VkInstance                                  instance,
    const VkHeadlessSurfaceCreateInfoEXT*       pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) const override;


virtual void PreCallRecordCreateHeadlessSurfaceEXT(
    VkInstance                                  instance,
    const VkHeadlessSurfaceCreateInfoEXT*       pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) override;


virtual void PostCallRecordCreateHeadlessSurfaceEXT(
    VkInstance                                  instance,
    const VkHeadlessSurfaceCreateInfoEXT*       pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface,
    VkResult                                    result) override;


virtual bool PreCallValidateCmdSetLineStippleEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    lineStippleFactor,
    uint16_t                                    lineStipplePattern) const override;


virtual void PreCallRecordCmdSetLineStippleEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    lineStippleFactor,
    uint16_t                                    lineStipplePattern) override;


virtual void PostCallRecordCmdSetLineStippleEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    lineStippleFactor,
    uint16_t                                    lineStipplePattern) override;


virtual bool PreCallValidateResetQueryPoolEXT(
    VkDevice                                    device,
    VkQueryPool                                 queryPool,
    uint32_t                                    firstQuery,
    uint32_t                                    queryCount) const override;


virtual void PreCallRecordResetQueryPoolEXT(
    VkDevice                                    device,
    VkQueryPool                                 queryPool,
    uint32_t                                    firstQuery,
    uint32_t                                    queryCount) override;


virtual void PostCallRecordResetQueryPoolEXT(
    VkDevice                                    device,
    VkQueryPool                                 queryPool,
    uint32_t                                    firstQuery,
    uint32_t                                    queryCount) override;


virtual bool PreCallValidateGetGeneratedCommandsMemoryRequirementsNV(
    VkDevice                                    device,
    const VkGeneratedCommandsMemoryRequirementsInfoNV* pInfo,
    VkMemoryRequirements2*                      pMemoryRequirements) const override;


virtual void PreCallRecordGetGeneratedCommandsMemoryRequirementsNV(
    VkDevice                                    device,
    const VkGeneratedCommandsMemoryRequirementsInfoNV* pInfo,
    VkMemoryRequirements2*                      pMemoryRequirements) override;


virtual void PostCallRecordGetGeneratedCommandsMemoryRequirementsNV(
    VkDevice                                    device,
    const VkGeneratedCommandsMemoryRequirementsInfoNV* pInfo,
    VkMemoryRequirements2*                      pMemoryRequirements) override;


virtual bool PreCallValidateCmdPreprocessGeneratedCommandsNV(
    VkCommandBuffer                             commandBuffer,
    const VkGeneratedCommandsInfoNV*            pGeneratedCommandsInfo) const override;


virtual void PreCallRecordCmdPreprocessGeneratedCommandsNV(
    VkCommandBuffer                             commandBuffer,
    const VkGeneratedCommandsInfoNV*            pGeneratedCommandsInfo) override;


virtual void PostCallRecordCmdPreprocessGeneratedCommandsNV(
    VkCommandBuffer                             commandBuffer,
    const VkGeneratedCommandsInfoNV*            pGeneratedCommandsInfo) override;


virtual bool PreCallValidateCmdExecuteGeneratedCommandsNV(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    isPreprocessed,
    const VkGeneratedCommandsInfoNV*            pGeneratedCommandsInfo) const override;


virtual void PreCallRecordCmdExecuteGeneratedCommandsNV(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    isPreprocessed,
    const VkGeneratedCommandsInfoNV*            pGeneratedCommandsInfo) override;


virtual void PostCallRecordCmdExecuteGeneratedCommandsNV(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    isPreprocessed,
    const VkGeneratedCommandsInfoNV*            pGeneratedCommandsInfo) override;


virtual bool PreCallValidateCmdBindPipelineShaderGroupNV(
    VkCommandBuffer                             commandBuffer,
    VkPipelineBindPoint                         pipelineBindPoint,
    VkPipeline                                  pipeline,
    uint32_t                                    groupIndex) const override;


virtual void PreCallRecordCmdBindPipelineShaderGroupNV(
    VkCommandBuffer                             commandBuffer,
    VkPipelineBindPoint                         pipelineBindPoint,
    VkPipeline                                  pipeline,
    uint32_t                                    groupIndex) override;


virtual void PostCallRecordCmdBindPipelineShaderGroupNV(
    VkCommandBuffer                             commandBuffer,
    VkPipelineBindPoint                         pipelineBindPoint,
    VkPipeline                                  pipeline,
    uint32_t                                    groupIndex) override;


virtual bool PreCallValidateCreateIndirectCommandsLayoutNV(
    VkDevice                                    device,
    const VkIndirectCommandsLayoutCreateInfoNV* pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkIndirectCommandsLayoutNV*                 pIndirectCommandsLayout) const override;


virtual void PreCallRecordCreateIndirectCommandsLayoutNV(
    VkDevice                                    device,
    const VkIndirectCommandsLayoutCreateInfoNV* pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkIndirectCommandsLayoutNV*                 pIndirectCommandsLayout) override;


virtual void PostCallRecordCreateIndirectCommandsLayoutNV(
    VkDevice                                    device,
    const VkIndirectCommandsLayoutCreateInfoNV* pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkIndirectCommandsLayoutNV*                 pIndirectCommandsLayout,
    VkResult                                    result) override;


virtual bool PreCallValidateDestroyIndirectCommandsLayoutNV(
    VkDevice                                    device,
    VkIndirectCommandsLayoutNV                  indirectCommandsLayout,
    const VkAllocationCallbacks*                pAllocator) const override;


virtual void PreCallRecordDestroyIndirectCommandsLayoutNV(
    VkDevice                                    device,
    VkIndirectCommandsLayoutNV                  indirectCommandsLayout,
    const VkAllocationCallbacks*                pAllocator) override;


virtual void PostCallRecordDestroyIndirectCommandsLayoutNV(
    VkDevice                                    device,
    VkIndirectCommandsLayoutNV                  indirectCommandsLayout,
    const VkAllocationCallbacks*                pAllocator) override;


virtual bool PreCallValidateCreatePrivateDataSlotEXT(
    VkDevice                                    device,
    const VkPrivateDataSlotCreateInfoEXT*       pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkPrivateDataSlotEXT*                       pPrivateDataSlot) const override;


virtual void PreCallRecordCreatePrivateDataSlotEXT(
    VkDevice                                    device,
    const VkPrivateDataSlotCreateInfoEXT*       pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkPrivateDataSlotEXT*                       pPrivateDataSlot) override;


virtual void PostCallRecordCreatePrivateDataSlotEXT(
    VkDevice                                    device,
    const VkPrivateDataSlotCreateInfoEXT*       pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkPrivateDataSlotEXT*                       pPrivateDataSlot,
    VkResult                                    result) override;


virtual bool PreCallValidateDestroyPrivateDataSlotEXT(
    VkDevice                                    device,
    VkPrivateDataSlotEXT                        privateDataSlot,
    const VkAllocationCallbacks*                pAllocator) const override;


virtual void PreCallRecordDestroyPrivateDataSlotEXT(
    VkDevice                                    device,
    VkPrivateDataSlotEXT                        privateDataSlot,
    const VkAllocationCallbacks*                pAllocator) override;


virtual void PostCallRecordDestroyPrivateDataSlotEXT(
    VkDevice                                    device,
    VkPrivateDataSlotEXT                        privateDataSlot,
    const VkAllocationCallbacks*                pAllocator) override;


virtual bool PreCallValidateSetPrivateDataEXT(
    VkDevice                                    device,
    VkObjectType                                objectType,
    uint64_t                                    objectHandle,
    VkPrivateDataSlotEXT                        privateDataSlot,
    uint64_t                                    data) const override;


virtual void PreCallRecordSetPrivateDataEXT(
    VkDevice                                    device,
    VkObjectType                                objectType,
    uint64_t                                    objectHandle,
    VkPrivateDataSlotEXT                        privateDataSlot,
    uint64_t                                    data) override;


virtual void PostCallRecordSetPrivateDataEXT(
    VkDevice                                    device,
    VkObjectType                                objectType,
    uint64_t                                    objectHandle,
    VkPrivateDataSlotEXT                        privateDataSlot,
    uint64_t                                    data,
    VkResult                                    result) override;


virtual bool PreCallValidateGetPrivateDataEXT(
    VkDevice                                    device,
    VkObjectType                                objectType,
    uint64_t                                    objectHandle,
    VkPrivateDataSlotEXT                        privateDataSlot,
    uint64_t*                                   pData) const override;


virtual void PreCallRecordGetPrivateDataEXT(
    VkDevice                                    device,
    VkObjectType                                objectType,
    uint64_t                                    objectHandle,
    VkPrivateDataSlotEXT                        privateDataSlot,
    uint64_t*                                   pData) override;


virtual void PostCallRecordGetPrivateDataEXT(
    VkDevice                                    device,
    VkObjectType                                objectType,
    uint64_t                                    objectHandle,
    VkPrivateDataSlotEXT                        privateDataSlot,
    uint64_t*                                   pData) override;


#ifdef VK_ENABLE_BETA_EXTENSIONS

virtual bool PreCallValidateCreateAccelerationStructureKHR(
    VkDevice                                    device,
    const VkAccelerationStructureCreateInfoKHR* pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkAccelerationStructureKHR*                 pAccelerationStructure) const override;


virtual void PreCallRecordCreateAccelerationStructureKHR(
    VkDevice                                    device,
    const VkAccelerationStructureCreateInfoKHR* pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkAccelerationStructureKHR*                 pAccelerationStructure) override;


virtual void PostCallRecordCreateAccelerationStructureKHR(
    VkDevice                                    device,
    const VkAccelerationStructureCreateInfoKHR* pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkAccelerationStructureKHR*                 pAccelerationStructure,
    VkResult                                    result) override;


#endif // VK_ENABLE_BETA_EXTENSIONS

#ifdef VK_ENABLE_BETA_EXTENSIONS

virtual bool PreCallValidateGetAccelerationStructureMemoryRequirementsKHR(
    VkDevice                                    device,
    const VkAccelerationStructureMemoryRequirementsInfoKHR* pInfo,
    VkMemoryRequirements2*                      pMemoryRequirements) const override;


virtual void PreCallRecordGetAccelerationStructureMemoryRequirementsKHR(
    VkDevice                                    device,
    const VkAccelerationStructureMemoryRequirementsInfoKHR* pInfo,
    VkMemoryRequirements2*                      pMemoryRequirements) override;


virtual void PostCallRecordGetAccelerationStructureMemoryRequirementsKHR(
    VkDevice                                    device,
    const VkAccelerationStructureMemoryRequirementsInfoKHR* pInfo,
    VkMemoryRequirements2*                      pMemoryRequirements) override;


#endif // VK_ENABLE_BETA_EXTENSIONS

#ifdef VK_ENABLE_BETA_EXTENSIONS

virtual bool PreCallValidateCmdBuildAccelerationStructureKHR(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    infoCount,
    const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
    const VkAccelerationStructureBuildOffsetInfoKHR* const* ppOffsetInfos) const override;


virtual void PreCallRecordCmdBuildAccelerationStructureKHR(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    infoCount,
    const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
    const VkAccelerationStructureBuildOffsetInfoKHR* const* ppOffsetInfos) override;


virtual void PostCallRecordCmdBuildAccelerationStructureKHR(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    infoCount,
    const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
    const VkAccelerationStructureBuildOffsetInfoKHR* const* ppOffsetInfos) override;


#endif // VK_ENABLE_BETA_EXTENSIONS

#ifdef VK_ENABLE_BETA_EXTENSIONS

virtual bool PreCallValidateCmdBuildAccelerationStructureIndirectKHR(
    VkCommandBuffer                             commandBuffer,
    const VkAccelerationStructureBuildGeometryInfoKHR* pInfo,
    VkBuffer                                    indirectBuffer,
    VkDeviceSize                                indirectOffset,
    uint32_t                                    indirectStride) const override;


virtual void PreCallRecordCmdBuildAccelerationStructureIndirectKHR(
    VkCommandBuffer                             commandBuffer,
    const VkAccelerationStructureBuildGeometryInfoKHR* pInfo,
    VkBuffer                                    indirectBuffer,
    VkDeviceSize                                indirectOffset,
    uint32_t                                    indirectStride) override;


virtual void PostCallRecordCmdBuildAccelerationStructureIndirectKHR(
    VkCommandBuffer                             commandBuffer,
    const VkAccelerationStructureBuildGeometryInfoKHR* pInfo,
    VkBuffer                                    indirectBuffer,
    VkDeviceSize                                indirectOffset,
    uint32_t                                    indirectStride) override;


#endif // VK_ENABLE_BETA_EXTENSIONS

#ifdef VK_ENABLE_BETA_EXTENSIONS

virtual bool PreCallValidateBuildAccelerationStructureKHR(
    VkDevice                                    device,
    uint32_t                                    infoCount,
    const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
    const VkAccelerationStructureBuildOffsetInfoKHR* const* ppOffsetInfos) const override;


virtual void PreCallRecordBuildAccelerationStructureKHR(
    VkDevice                                    device,
    uint32_t                                    infoCount,
    const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
    const VkAccelerationStructureBuildOffsetInfoKHR* const* ppOffsetInfos) override;


virtual void PostCallRecordBuildAccelerationStructureKHR(
    VkDevice                                    device,
    uint32_t                                    infoCount,
    const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
    const VkAccelerationStructureBuildOffsetInfoKHR* const* ppOffsetInfos,
    VkResult                                    result) override;


#endif // VK_ENABLE_BETA_EXTENSIONS

#ifdef VK_ENABLE_BETA_EXTENSIONS

virtual bool PreCallValidateCopyAccelerationStructureKHR(
    VkDevice                                    device,
    const VkCopyAccelerationStructureInfoKHR*   pInfo) const override;


virtual void PreCallRecordCopyAccelerationStructureKHR(
    VkDevice                                    device,
    const VkCopyAccelerationStructureInfoKHR*   pInfo) override;


virtual void PostCallRecordCopyAccelerationStructureKHR(
    VkDevice                                    device,
    const VkCopyAccelerationStructureInfoKHR*   pInfo,
    VkResult                                    result) override;


#endif // VK_ENABLE_BETA_EXTENSIONS

#ifdef VK_ENABLE_BETA_EXTENSIONS

virtual bool PreCallValidateCopyAccelerationStructureToMemoryKHR(
    VkDevice                                    device,
    const VkCopyAccelerationStructureToMemoryInfoKHR* pInfo) const override;


virtual void PreCallRecordCopyAccelerationStructureToMemoryKHR(
    VkDevice                                    device,
    const VkCopyAccelerationStructureToMemoryInfoKHR* pInfo) override;


virtual void PostCallRecordCopyAccelerationStructureToMemoryKHR(
    VkDevice                                    device,
    const VkCopyAccelerationStructureToMemoryInfoKHR* pInfo,
    VkResult                                    result) override;


#endif // VK_ENABLE_BETA_EXTENSIONS

#ifdef VK_ENABLE_BETA_EXTENSIONS

virtual bool PreCallValidateCopyMemoryToAccelerationStructureKHR(
    VkDevice                                    device,
    const VkCopyMemoryToAccelerationStructureInfoKHR* pInfo) const override;


virtual void PreCallRecordCopyMemoryToAccelerationStructureKHR(
    VkDevice                                    device,
    const VkCopyMemoryToAccelerationStructureInfoKHR* pInfo) override;


virtual void PostCallRecordCopyMemoryToAccelerationStructureKHR(
    VkDevice                                    device,
    const VkCopyMemoryToAccelerationStructureInfoKHR* pInfo,
    VkResult                                    result) override;


#endif // VK_ENABLE_BETA_EXTENSIONS

#ifdef VK_ENABLE_BETA_EXTENSIONS

virtual bool PreCallValidateWriteAccelerationStructuresPropertiesKHR(
    VkDevice                                    device,
    uint32_t                                    accelerationStructureCount,
    const VkAccelerationStructureKHR*           pAccelerationStructures,
    VkQueryType                                 queryType,
    size_t                                      dataSize,
    void*                                       pData,
    size_t                                      stride) const override;


virtual void PreCallRecordWriteAccelerationStructuresPropertiesKHR(
    VkDevice                                    device,
    uint32_t                                    accelerationStructureCount,
    const VkAccelerationStructureKHR*           pAccelerationStructures,
    VkQueryType                                 queryType,
    size_t                                      dataSize,
    void*                                       pData,
    size_t                                      stride) override;


virtual void PostCallRecordWriteAccelerationStructuresPropertiesKHR(
    VkDevice                                    device,
    uint32_t                                    accelerationStructureCount,
    const VkAccelerationStructureKHR*           pAccelerationStructures,
    VkQueryType                                 queryType,
    size_t                                      dataSize,
    void*                                       pData,
    size_t                                      stride,
    VkResult                                    result) override;


#endif // VK_ENABLE_BETA_EXTENSIONS

#ifdef VK_ENABLE_BETA_EXTENSIONS

virtual bool PreCallValidateCmdCopyAccelerationStructureKHR(
    VkCommandBuffer                             commandBuffer,
    const VkCopyAccelerationStructureInfoKHR*   pInfo) const override;


virtual void PreCallRecordCmdCopyAccelerationStructureKHR(
    VkCommandBuffer                             commandBuffer,
    const VkCopyAccelerationStructureInfoKHR*   pInfo) override;


virtual void PostCallRecordCmdCopyAccelerationStructureKHR(
    VkCommandBuffer                             commandBuffer,
    const VkCopyAccelerationStructureInfoKHR*   pInfo) override;


#endif // VK_ENABLE_BETA_EXTENSIONS

#ifdef VK_ENABLE_BETA_EXTENSIONS

virtual bool PreCallValidateCmdCopyAccelerationStructureToMemoryKHR(
    VkCommandBuffer                             commandBuffer,
    const VkCopyAccelerationStructureToMemoryInfoKHR* pInfo) const override;


virtual void PreCallRecordCmdCopyAccelerationStructureToMemoryKHR(
    VkCommandBuffer                             commandBuffer,
    const VkCopyAccelerationStructureToMemoryInfoKHR* pInfo) override;


virtual void PostCallRecordCmdCopyAccelerationStructureToMemoryKHR(
    VkCommandBuffer                             commandBuffer,
    const VkCopyAccelerationStructureToMemoryInfoKHR* pInfo) override;


#endif // VK_ENABLE_BETA_EXTENSIONS

#ifdef VK_ENABLE_BETA_EXTENSIONS

virtual bool PreCallValidateCmdCopyMemoryToAccelerationStructureKHR(
    VkCommandBuffer                             commandBuffer,
    const VkCopyMemoryToAccelerationStructureInfoKHR* pInfo) const override;


virtual void PreCallRecordCmdCopyMemoryToAccelerationStructureKHR(
    VkCommandBuffer                             commandBuffer,
    const VkCopyMemoryToAccelerationStructureInfoKHR* pInfo) override;


virtual void PostCallRecordCmdCopyMemoryToAccelerationStructureKHR(
    VkCommandBuffer                             commandBuffer,
    const VkCopyMemoryToAccelerationStructureInfoKHR* pInfo) override;


#endif // VK_ENABLE_BETA_EXTENSIONS

#ifdef VK_ENABLE_BETA_EXTENSIONS

virtual bool PreCallValidateCmdTraceRaysKHR(
    VkCommandBuffer                             commandBuffer,
    const VkStridedBufferRegionKHR*             pRaygenShaderBindingTable,
    const VkStridedBufferRegionKHR*             pMissShaderBindingTable,
    const VkStridedBufferRegionKHR*             pHitShaderBindingTable,
    const VkStridedBufferRegionKHR*             pCallableShaderBindingTable,
    uint32_t                                    width,
    uint32_t                                    height,
    uint32_t                                    depth) const override;


virtual void PreCallRecordCmdTraceRaysKHR(
    VkCommandBuffer                             commandBuffer,
    const VkStridedBufferRegionKHR*             pRaygenShaderBindingTable,
    const VkStridedBufferRegionKHR*             pMissShaderBindingTable,
    const VkStridedBufferRegionKHR*             pHitShaderBindingTable,
    const VkStridedBufferRegionKHR*             pCallableShaderBindingTable,
    uint32_t                                    width,
    uint32_t                                    height,
    uint32_t                                    depth) override;


virtual void PostCallRecordCmdTraceRaysKHR(
    VkCommandBuffer                             commandBuffer,
    const VkStridedBufferRegionKHR*             pRaygenShaderBindingTable,
    const VkStridedBufferRegionKHR*             pMissShaderBindingTable,
    const VkStridedBufferRegionKHR*             pHitShaderBindingTable,
    const VkStridedBufferRegionKHR*             pCallableShaderBindingTable,
    uint32_t                                    width,
    uint32_t                                    height,
    uint32_t                                    depth) override;


#endif // VK_ENABLE_BETA_EXTENSIONS

#ifdef VK_ENABLE_BETA_EXTENSIONS

virtual bool PreCallValidateCreateRayTracingPipelinesKHR(
    VkDevice                                    device,
    VkPipelineCache                             pipelineCache,
    uint32_t                                    createInfoCount,
    const VkRayTracingPipelineCreateInfoKHR*    pCreateInfos,
    const VkAllocationCallbacks*                pAllocator,
    VkPipeline*                                 pPipelines,
    void*                                       state_data) const override;


virtual void PreCallRecordCreateRayTracingPipelinesKHR(
    VkDevice                                    device,
    VkPipelineCache                             pipelineCache,
    uint32_t                                    createInfoCount,
    const VkRayTracingPipelineCreateInfoKHR*    pCreateInfos,
    const VkAllocationCallbacks*                pAllocator,
    VkPipeline*                                 pPipelines,
    void*                                       state_data) override;


virtual void PostCallRecordCreateRayTracingPipelinesKHR(
    VkDevice                                    device,
    VkPipelineCache                             pipelineCache,
    uint32_t                                    createInfoCount,
    const VkRayTracingPipelineCreateInfoKHR*    pCreateInfos,
    const VkAllocationCallbacks*                pAllocator,
    VkPipeline*                                 pPipelines,
    VkResult                                    result,
    void*                                       state_data) override;


#endif // VK_ENABLE_BETA_EXTENSIONS

#ifdef VK_ENABLE_BETA_EXTENSIONS

virtual bool PreCallValidateGetAccelerationStructureDeviceAddressKHR(
    VkDevice                                    device,
    const VkAccelerationStructureDeviceAddressInfoKHR* pInfo) const override;


virtual void PreCallRecordGetAccelerationStructureDeviceAddressKHR(
    VkDevice                                    device,
    const VkAccelerationStructureDeviceAddressInfoKHR* pInfo) override;


virtual void PostCallRecordGetAccelerationStructureDeviceAddressKHR(
    VkDevice                                    device,
    const VkAccelerationStructureDeviceAddressInfoKHR* pInfo,
    VkResult                                    result) override;


#endif // VK_ENABLE_BETA_EXTENSIONS

#ifdef VK_ENABLE_BETA_EXTENSIONS

virtual bool PreCallValidateGetRayTracingCaptureReplayShaderGroupHandlesKHR(
    VkDevice                                    device,
    VkPipeline                                  pipeline,
    uint32_t                                    firstGroup,
    uint32_t                                    groupCount,
    size_t                                      dataSize,
    void*                                       pData) const override;


virtual void PreCallRecordGetRayTracingCaptureReplayShaderGroupHandlesKHR(
    VkDevice                                    device,
    VkPipeline                                  pipeline,
    uint32_t                                    firstGroup,
    uint32_t                                    groupCount,
    size_t                                      dataSize,
    void*                                       pData) override;


virtual void PostCallRecordGetRayTracingCaptureReplayShaderGroupHandlesKHR(
    VkDevice                                    device,
    VkPipeline                                  pipeline,
    uint32_t                                    firstGroup,
    uint32_t                                    groupCount,
    size_t                                      dataSize,
    void*                                       pData,
    VkResult                                    result) override;


#endif // VK_ENABLE_BETA_EXTENSIONS

#ifdef VK_ENABLE_BETA_EXTENSIONS

virtual bool PreCallValidateCmdTraceRaysIndirectKHR(
    VkCommandBuffer                             commandBuffer,
    const VkStridedBufferRegionKHR*             pRaygenShaderBindingTable,
    const VkStridedBufferRegionKHR*             pMissShaderBindingTable,
    const VkStridedBufferRegionKHR*             pHitShaderBindingTable,
    const VkStridedBufferRegionKHR*             pCallableShaderBindingTable,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset) const override;


virtual void PreCallRecordCmdTraceRaysIndirectKHR(
    VkCommandBuffer                             commandBuffer,
    const VkStridedBufferRegionKHR*             pRaygenShaderBindingTable,
    const VkStridedBufferRegionKHR*             pMissShaderBindingTable,
    const VkStridedBufferRegionKHR*             pHitShaderBindingTable,
    const VkStridedBufferRegionKHR*             pCallableShaderBindingTable,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset) override;


virtual void PostCallRecordCmdTraceRaysIndirectKHR(
    VkCommandBuffer                             commandBuffer,
    const VkStridedBufferRegionKHR*             pRaygenShaderBindingTable,
    const VkStridedBufferRegionKHR*             pMissShaderBindingTable,
    const VkStridedBufferRegionKHR*             pHitShaderBindingTable,
    const VkStridedBufferRegionKHR*             pCallableShaderBindingTable,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset) override;


#endif // VK_ENABLE_BETA_EXTENSIONS

#ifdef VK_ENABLE_BETA_EXTENSIONS

virtual bool PreCallValidateGetDeviceAccelerationStructureCompatibilityKHR(
    VkDevice                                    device,
    const VkAccelerationStructureVersionKHR*    version) const override;


virtual void PreCallRecordGetDeviceAccelerationStructureCompatibilityKHR(
    VkDevice                                    device,
    const VkAccelerationStructureVersionKHR*    version) override;


virtual void PostCallRecordGetDeviceAccelerationStructureCompatibilityKHR(
    VkDevice                                    device,
    const VkAccelerationStructureVersionKHR*    version,
    VkResult                                    result) override;


#endif // VK_ENABLE_BETA_EXTENSIONS





const std::unordered_map<std::string, DeprecationData>  deprecated_extensions = {
    {"VK_AMD_draw_indirect_count", {kExtPromoted, "VK_KHR_draw_indirect_count"}},
    {"VK_AMD_gpu_shader_half_float", {kExtDeprecated, "VK_KHR_shader_float16_int8"}},
    {"VK_AMD_gpu_shader_int16", {kExtDeprecated, "VK_KHR_shader_float16_int8"}},
    {"VK_AMD_negative_viewport_height", {kExtObsoleted, "VK_KHR_maintenance1"}},
    {"VK_EXT_buffer_device_address", {kExtDeprecated, "VK_KHR_buffer_device_address"}},
    {"VK_EXT_debug_marker", {kExtPromoted, "VK_EXT_debug_utils"}},
    {"VK_EXT_debug_report", {kExtDeprecated, "VK_EXT_debug_utils"}},
    {"VK_EXT_descriptor_indexing", {kExtPromoted, "VK_VERSION_1_2"}},
    {"VK_EXT_host_query_reset", {kExtPromoted, "VK_VERSION_1_2"}},
    {"VK_EXT_sampler_filter_minmax", {kExtPromoted, "VK_VERSION_1_2"}},
    {"VK_EXT_scalar_block_layout", {kExtPromoted, "VK_VERSION_1_2"}},
    {"VK_EXT_separate_stencil_usage", {kExtPromoted, "VK_VERSION_1_2"}},
    {"VK_EXT_shader_subgroup_ballot", {kExtDeprecated, "VK_VERSION_1_2"}},
    {"VK_EXT_shader_subgroup_vote", {kExtDeprecated, "VK_VERSION_1_1"}},
    {"VK_EXT_shader_viewport_index_layer", {kExtPromoted, "VK_VERSION_1_2"}},
    {"VK_EXT_validation_flags", {kExtDeprecated, "VK_EXT_validation_features"}},
    {"VK_KHR_16bit_storage", {kExtPromoted, "VK_VERSION_1_1"}},
    {"VK_KHR_8bit_storage", {kExtPromoted, "VK_VERSION_1_2"}},
    {"VK_KHR_bind_memory2", {kExtPromoted, "VK_VERSION_1_1"}},
    {"VK_KHR_buffer_device_address", {kExtPromoted, "VK_VERSION_1_2"}},
    {"VK_KHR_create_renderpass2", {kExtPromoted, "VK_VERSION_1_2"}},
    {"VK_KHR_dedicated_allocation", {kExtPromoted, "VK_VERSION_1_1"}},
    {"VK_KHR_depth_stencil_resolve", {kExtPromoted, "VK_VERSION_1_2"}},
    {"VK_KHR_descriptor_update_template", {kExtPromoted, "VK_VERSION_1_1"}},
    {"VK_KHR_device_group", {kExtPromoted, "VK_VERSION_1_1"}},
    {"VK_KHR_device_group_creation", {kExtPromoted, "VK_VERSION_1_1"}},
    {"VK_KHR_draw_indirect_count", {kExtPromoted, "VK_VERSION_1_2"}},
    {"VK_KHR_driver_properties", {kExtPromoted, "VK_VERSION_1_2"}},
    {"VK_KHR_external_fence", {kExtPromoted, "VK_VERSION_1_1"}},
    {"VK_KHR_external_fence_capabilities", {kExtPromoted, "VK_VERSION_1_1"}},
    {"VK_KHR_external_memory", {kExtPromoted, "VK_VERSION_1_1"}},
    {"VK_KHR_external_memory_capabilities", {kExtPromoted, "VK_VERSION_1_1"}},
    {"VK_KHR_external_semaphore", {kExtPromoted, "VK_VERSION_1_1"}},
    {"VK_KHR_external_semaphore_capabilities", {kExtPromoted, "VK_VERSION_1_1"}},
    {"VK_KHR_get_memory_requirements2", {kExtPromoted, "VK_VERSION_1_1"}},
    {"VK_KHR_get_physical_device_properties2", {kExtPromoted, "VK_VERSION_1_1"}},
    {"VK_KHR_image_format_list", {kExtPromoted, "VK_VERSION_1_2"}},
    {"VK_KHR_imageless_framebuffer", {kExtPromoted, "VK_VERSION_1_2"}},
    {"VK_KHR_maintenance1", {kExtPromoted, "VK_VERSION_1_1"}},
    {"VK_KHR_maintenance2", {kExtPromoted, "VK_VERSION_1_1"}},
    {"VK_KHR_maintenance3", {kExtPromoted, "VK_VERSION_1_1"}},
    {"VK_KHR_multiview", {kExtPromoted, "VK_VERSION_1_1"}},
    {"VK_KHR_relaxed_block_layout", {kExtPromoted, "VK_VERSION_1_1"}},
    {"VK_KHR_sampler_mirror_clamp_to_edge", {kExtPromoted, "VK_VERSION_1_2"}},
    {"VK_KHR_sampler_ycbcr_conversion", {kExtPromoted, "VK_VERSION_1_1"}},
    {"VK_KHR_separate_depth_stencil_layouts", {kExtPromoted, "VK_VERSION_1_2"}},
    {"VK_KHR_shader_atomic_int64", {kExtPromoted, "VK_VERSION_1_2"}},
    {"VK_KHR_shader_draw_parameters", {kExtPromoted, "VK_VERSION_1_1"}},
    {"VK_KHR_shader_float16_int8", {kExtPromoted, "VK_VERSION_1_2"}},
    {"VK_KHR_shader_float_controls", {kExtPromoted, "VK_VERSION_1_2"}},
    {"VK_KHR_shader_subgroup_extended_types", {kExtPromoted, "VK_VERSION_1_2"}},
    {"VK_KHR_spirv_1_4", {kExtPromoted, "VK_VERSION_1_2"}},
    {"VK_KHR_storage_buffer_storage_class", {kExtPromoted, "VK_VERSION_1_1"}},
    {"VK_KHR_timeline_semaphore", {kExtPromoted, "VK_VERSION_1_2"}},
    {"VK_KHR_uniform_buffer_standard_layout", {kExtPromoted, "VK_VERSION_1_2"}},
    {"VK_KHR_variable_pointers", {kExtPromoted, "VK_VERSION_1_1"}},
    {"VK_KHR_vulkan_memory_model", {kExtPromoted, "VK_VERSION_1_2"}},
    {"VK_NV_dedicated_allocation", {kExtDeprecated, "VK_KHR_dedicated_allocation"}},
    {"VK_NV_external_memory", {kExtDeprecated, "VK_KHR_external_memory"}},
    {"VK_NV_external_memory_capabilities", {kExtDeprecated, "VK_KHR_external_memory_capabilities"}},
    {"VK_NV_external_memory_win32", {kExtDeprecated, "VK_KHR_external_memory_win32"}},
    {"VK_NV_glsl_shader", {kExtDeprecated, ""}},
    {"VK_NV_win32_keyed_mutex", {kExtPromoted, "VK_KHR_win32_keyed_mutex"}},
};

};

