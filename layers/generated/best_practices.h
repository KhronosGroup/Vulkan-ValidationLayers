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


void PostCallRecordvkCreateInstance(
    const VkInstanceCreateInfo*                 pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkInstance*                                 pInstance,
    VkResult                                    result);


void PostCallRecordvkEnumeratePhysicalDevices(
    VkInstance                                  instance,
    uint32_t*                                   pPhysicalDeviceCount,
    VkPhysicalDevice*                           pPhysicalDevices,
    VkResult                                    result);


void PostCallRecordvkGetPhysicalDeviceImageFormatProperties(
    VkPhysicalDevice                            physicalDevice,
    VkFormat                                    format,
    VkImageType                                 type,
    VkImageTiling                               tiling,
    VkImageUsageFlags                           usage,
    VkImageCreateFlags                          flags,
    VkImageFormatProperties*                    pImageFormatProperties,
    VkResult                                    result);


void PostCallRecordvkCreateDevice(
    VkPhysicalDevice                            physicalDevice,
    const VkDeviceCreateInfo*                   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDevice*                                   pDevice,
    VkResult                                    result);


void PostCallRecordvkEnumerateInstanceExtensionProperties(
    const char*                                 pLayerName,
    uint32_t*                                   pPropertyCount,
    VkExtensionProperties*                      pProperties,
    VkResult                                    result);


void PostCallRecordvkEnumerateDeviceExtensionProperties(
    VkPhysicalDevice                            physicalDevice,
    const char*                                 pLayerName,
    uint32_t*                                   pPropertyCount,
    VkExtensionProperties*                      pProperties,
    VkResult                                    result);


void PostCallRecordvkEnumerateInstanceLayerProperties(
    uint32_t*                                   pPropertyCount,
    VkLayerProperties*                          pProperties,
    VkResult                                    result);


void PostCallRecordvkEnumerateDeviceLayerProperties(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pPropertyCount,
    VkLayerProperties*                          pProperties,
    VkResult                                    result);


void PostCallRecordvkQueueSubmit(
    VkQueue                                     queue,
    uint32_t                                    submitCount,
    const VkSubmitInfo*                         pSubmits,
    VkFence                                     fence,
    VkResult                                    result);


void PostCallRecordvkQueueWaitIdle(
    VkQueue                                     queue,
    VkResult                                    result);


void PostCallRecordvkDeviceWaitIdle(
    VkDevice                                    device,
    VkResult                                    result);


void PostCallRecordvkMapMemory(
    VkDevice                                    device,
    VkDeviceMemory                              memory,
    VkDeviceSize                                offset,
    VkDeviceSize                                size,
    VkMemoryMapFlags                            flags,
    void**                                      ppData,
    VkResult                                    result);


void PostCallRecordvkFlushMappedMemoryRanges(
    VkDevice                                    device,
    uint32_t                                    memoryRangeCount,
    const VkMappedMemoryRange*                  pMemoryRanges,
    VkResult                                    result);


void PostCallRecordvkInvalidateMappedMemoryRanges(
    VkDevice                                    device,
    uint32_t                                    memoryRangeCount,
    const VkMappedMemoryRange*                  pMemoryRanges,
    VkResult                                    result);


void PostCallRecordvkBindBufferMemory(
    VkDevice                                    device,
    VkBuffer                                    buffer,
    VkDeviceMemory                              memory,
    VkDeviceSize                                memoryOffset,
    VkResult                                    result);


void PostCallRecordvkBindImageMemory(
    VkDevice                                    device,
    VkImage                                     image,
    VkDeviceMemory                              memory,
    VkDeviceSize                                memoryOffset,
    VkResult                                    result);


void PostCallRecordvkCreateFence(
    VkDevice                                    device,
    const VkFenceCreateInfo*                    pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkFence*                                    pFence,
    VkResult                                    result);


void PostCallRecordvkResetFences(
    VkDevice                                    device,
    uint32_t                                    fenceCount,
    const VkFence*                              pFences,
    VkResult                                    result);


void PostCallRecordvkGetFenceStatus(
    VkDevice                                    device,
    VkFence                                     fence,
    VkResult                                    result);


void PostCallRecordvkWaitForFences(
    VkDevice                                    device,
    uint32_t                                    fenceCount,
    const VkFence*                              pFences,
    VkBool32                                    waitAll,
    uint64_t                                    timeout,
    VkResult                                    result);


void PostCallRecordvkCreateSemaphore(
    VkDevice                                    device,
    const VkSemaphoreCreateInfo*                pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSemaphore*                                pSemaphore,
    VkResult                                    result);


void PostCallRecordvkCreateEvent(
    VkDevice                                    device,
    const VkEventCreateInfo*                    pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkEvent*                                    pEvent,
    VkResult                                    result);


void PostCallRecordvkGetEventStatus(
    VkDevice                                    device,
    VkEvent                                     event,
    VkResult                                    result);


void PostCallRecordvkSetEvent(
    VkDevice                                    device,
    VkEvent                                     event,
    VkResult                                    result);


void PostCallRecordvkResetEvent(
    VkDevice                                    device,
    VkEvent                                     event,
    VkResult                                    result);


void PostCallRecordvkCreateQueryPool(
    VkDevice                                    device,
    const VkQueryPoolCreateInfo*                pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkQueryPool*                                pQueryPool,
    VkResult                                    result);


void PostCallRecordvkGetQueryPoolResults(
    VkDevice                                    device,
    VkQueryPool                                 queryPool,
    uint32_t                                    firstQuery,
    uint32_t                                    queryCount,
    size_t                                      dataSize,
    void*                                       pData,
    VkDeviceSize                                stride,
    VkQueryResultFlags                          flags,
    VkResult                                    result);


void PostCallRecordvkCreateBuffer(
    VkDevice                                    device,
    const VkBufferCreateInfo*                   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkBuffer*                                   pBuffer,
    VkResult                                    result);


void PostCallRecordvkCreateBufferView(
    VkDevice                                    device,
    const VkBufferViewCreateInfo*               pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkBufferView*                               pView,
    VkResult                                    result);


void PostCallRecordvkCreateImage(
    VkDevice                                    device,
    const VkImageCreateInfo*                    pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkImage*                                    pImage,
    VkResult                                    result);


void PostCallRecordvkCreateImageView(
    VkDevice                                    device,
    const VkImageViewCreateInfo*                pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkImageView*                                pView,
    VkResult                                    result);


void PostCallRecordvkCreateShaderModule(
    VkDevice                                    device,
    const VkShaderModuleCreateInfo*             pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkShaderModule*                             pShaderModule,
    VkResult                                    result,
    void*                                       state_data);


void PostCallRecordvkCreatePipelineCache(
    VkDevice                                    device,
    const VkPipelineCacheCreateInfo*            pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkPipelineCache*                            pPipelineCache,
    VkResult                                    result);


void PostCallRecordvkGetPipelineCacheData(
    VkDevice                                    device,
    VkPipelineCache                             pipelineCache,
    size_t*                                     pDataSize,
    void*                                       pData,
    VkResult                                    result);


void PostCallRecordvkMergePipelineCaches(
    VkDevice                                    device,
    VkPipelineCache                             dstCache,
    uint32_t                                    srcCacheCount,
    const VkPipelineCache*                      pSrcCaches,
    VkResult                                    result);


void PostCallRecordvkCreateGraphicsPipelines(
    VkDevice                                    device,
    VkPipelineCache                             pipelineCache,
    uint32_t                                    createInfoCount,
    const VkGraphicsPipelineCreateInfo*         pCreateInfos,
    const VkAllocationCallbacks*                pAllocator,
    VkPipeline*                                 pPipelines,
    VkResult                                    result,
    void*                                       state_data);


void PostCallRecordvkCreateComputePipelines(
    VkDevice                                    device,
    VkPipelineCache                             pipelineCache,
    uint32_t                                    createInfoCount,
    const VkComputePipelineCreateInfo*          pCreateInfos,
    const VkAllocationCallbacks*                pAllocator,
    VkPipeline*                                 pPipelines,
    VkResult                                    result,
    void*                                       state_data);


void PostCallRecordvkCreatePipelineLayout(
    VkDevice                                    device,
    const VkPipelineLayoutCreateInfo*           pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkPipelineLayout*                           pPipelineLayout,
    VkResult                                    result);


void PostCallRecordvkCreateSampler(
    VkDevice                                    device,
    const VkSamplerCreateInfo*                  pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSampler*                                  pSampler,
    VkResult                                    result);


void PostCallRecordvkCreateDescriptorSetLayout(
    VkDevice                                    device,
    const VkDescriptorSetLayoutCreateInfo*      pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDescriptorSetLayout*                      pSetLayout,
    VkResult                                    result);


void PostCallRecordvkCreateDescriptorPool(
    VkDevice                                    device,
    const VkDescriptorPoolCreateInfo*           pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDescriptorPool*                           pDescriptorPool,
    VkResult                                    result);


void PostCallRecordvkAllocateDescriptorSets(
    VkDevice                                    device,
    const VkDescriptorSetAllocateInfo*          pAllocateInfo,
    VkDescriptorSet*                            pDescriptorSets,
    VkResult                                    result,
    void*                                       state_data);


void PostCallRecordvkCreateFramebuffer(
    VkDevice                                    device,
    const VkFramebufferCreateInfo*              pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkFramebuffer*                              pFramebuffer,
    VkResult                                    result);


void PostCallRecordvkCreateRenderPass(
    VkDevice                                    device,
    const VkRenderPassCreateInfo*               pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkRenderPass*                               pRenderPass,
    VkResult                                    result);


void PostCallRecordvkCreateCommandPool(
    VkDevice                                    device,
    const VkCommandPoolCreateInfo*              pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkCommandPool*                              pCommandPool,
    VkResult                                    result);


void PostCallRecordvkResetCommandPool(
    VkDevice                                    device,
    VkCommandPool                               commandPool,
    VkCommandPoolResetFlags                     flags,
    VkResult                                    result);


void PostCallRecordvkAllocateCommandBuffers(
    VkDevice                                    device,
    const VkCommandBufferAllocateInfo*          pAllocateInfo,
    VkCommandBuffer*                            pCommandBuffers,
    VkResult                                    result);


void PostCallRecordvkBeginCommandBuffer(
    VkCommandBuffer                             commandBuffer,
    const VkCommandBufferBeginInfo*             pBeginInfo,
    VkResult                                    result);


void PostCallRecordvkEndCommandBuffer(
    VkCommandBuffer                             commandBuffer,
    VkResult                                    result);


void PostCallRecordvkResetCommandBuffer(
    VkCommandBuffer                             commandBuffer,
    VkCommandBufferResetFlags                   flags,
    VkResult                                    result);


void PostCallRecordvkBindBufferMemory2(
    VkDevice                                    device,
    uint32_t                                    bindInfoCount,
    const VkBindBufferMemoryInfo*               pBindInfos,
    VkResult                                    result);


void PostCallRecordvkBindImageMemory2(
    VkDevice                                    device,
    uint32_t                                    bindInfoCount,
    const VkBindImageMemoryInfo*                pBindInfos,
    VkResult                                    result);


void PostCallRecordvkEnumeratePhysicalDeviceGroups(
    VkInstance                                  instance,
    uint32_t*                                   pPhysicalDeviceGroupCount,
    VkPhysicalDeviceGroupProperties*            pPhysicalDeviceGroupProperties,
    VkResult                                    result);


void PostCallRecordvkGetPhysicalDeviceImageFormatProperties2(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceImageFormatInfo2*     pImageFormatInfo,
    VkImageFormatProperties2*                   pImageFormatProperties,
    VkResult                                    result);


void PostCallRecordvkCreateSamplerYcbcrConversion(
    VkDevice                                    device,
    const VkSamplerYcbcrConversionCreateInfo*   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSamplerYcbcrConversion*                   pYcbcrConversion,
    VkResult                                    result);


void PostCallRecordvkCreateDescriptorUpdateTemplate(
    VkDevice                                    device,
    const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDescriptorUpdateTemplate*                 pDescriptorUpdateTemplate,
    VkResult                                    result);


void PostCallRecordvkCreateRenderPass2(
    VkDevice                                    device,
    const VkRenderPassCreateInfo2*              pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkRenderPass*                               pRenderPass,
    VkResult                                    result);


void PostCallRecordvkGetSemaphoreCounterValue(
    VkDevice                                    device,
    VkSemaphore                                 semaphore,
    uint64_t*                                   pValue,
    VkResult                                    result);


void PostCallRecordvkWaitSemaphores(
    VkDevice                                    device,
    const VkSemaphoreWaitInfo*                  pWaitInfo,
    uint64_t                                    timeout,
    VkResult                                    result);


void PostCallRecordvkSignalSemaphore(
    VkDevice                                    device,
    const VkSemaphoreSignalInfo*                pSignalInfo,
    VkResult                                    result);


void PostCallRecordvkGetPhysicalDeviceSurfaceSupportKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    queueFamilyIndex,
    VkSurfaceKHR                                surface,
    VkBool32*                                   pSupported,
    VkResult                                    result);


void PostCallRecordvkGetPhysicalDeviceSurfaceCapabilitiesKHR(
    VkPhysicalDevice                            physicalDevice,
    VkSurfaceKHR                                surface,
    VkSurfaceCapabilitiesKHR*                   pSurfaceCapabilities,
    VkResult                                    result);


void PostCallRecordvkGetPhysicalDeviceSurfaceFormatsKHR(
    VkPhysicalDevice                            physicalDevice,
    VkSurfaceKHR                                surface,
    uint32_t*                                   pSurfaceFormatCount,
    VkSurfaceFormatKHR*                         pSurfaceFormats,
    VkResult                                    result);


void PostCallRecordvkGetPhysicalDeviceSurfacePresentModesKHR(
    VkPhysicalDevice                            physicalDevice,
    VkSurfaceKHR                                surface,
    uint32_t*                                   pPresentModeCount,
    VkPresentModeKHR*                           pPresentModes,
    VkResult                                    result);


void PostCallRecordvkCreateSwapchainKHR(
    VkDevice                                    device,
    const VkSwapchainCreateInfoKHR*             pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSwapchainKHR*                             pSwapchain,
    VkResult                                    result);


void PostCallRecordvkGetSwapchainImagesKHR(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    uint32_t*                                   pSwapchainImageCount,
    VkImage*                                    pSwapchainImages,
    VkResult                                    result);


void PostCallRecordvkAcquireNextImageKHR(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    uint64_t                                    timeout,
    VkSemaphore                                 semaphore,
    VkFence                                     fence,
    uint32_t*                                   pImageIndex,
    VkResult                                    result);


void PostCallRecordvkQueuePresentKHR(
    VkQueue                                     queue,
    const VkPresentInfoKHR*                     pPresentInfo,
    VkResult                                    result);


void PostCallRecordvkGetDeviceGroupPresentCapabilitiesKHR(
    VkDevice                                    device,
    VkDeviceGroupPresentCapabilitiesKHR*        pDeviceGroupPresentCapabilities,
    VkResult                                    result);


void PostCallRecordvkGetDeviceGroupSurfacePresentModesKHR(
    VkDevice                                    device,
    VkSurfaceKHR                                surface,
    VkDeviceGroupPresentModeFlagsKHR*           pModes,
    VkResult                                    result);


void PostCallRecordvkGetPhysicalDevicePresentRectanglesKHR(
    VkPhysicalDevice                            physicalDevice,
    VkSurfaceKHR                                surface,
    uint32_t*                                   pRectCount,
    VkRect2D*                                   pRects,
    VkResult                                    result);


void PostCallRecordvkAcquireNextImage2KHR(
    VkDevice                                    device,
    const VkAcquireNextImageInfoKHR*            pAcquireInfo,
    uint32_t*                                   pImageIndex,
    VkResult                                    result);


void PostCallRecordvkGetPhysicalDeviceDisplayPropertiesKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pPropertyCount,
    VkDisplayPropertiesKHR*                     pProperties,
    VkResult                                    result);


void PostCallRecordvkGetPhysicalDeviceDisplayPlanePropertiesKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pPropertyCount,
    VkDisplayPlanePropertiesKHR*                pProperties,
    VkResult                                    result);


void PostCallRecordvkGetDisplayPlaneSupportedDisplaysKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    planeIndex,
    uint32_t*                                   pDisplayCount,
    VkDisplayKHR*                               pDisplays,
    VkResult                                    result);


void PostCallRecordvkGetDisplayModePropertiesKHR(
    VkPhysicalDevice                            physicalDevice,
    VkDisplayKHR                                display,
    uint32_t*                                   pPropertyCount,
    VkDisplayModePropertiesKHR*                 pProperties,
    VkResult                                    result);


void PostCallRecordvkCreateDisplayModeKHR(
    VkPhysicalDevice                            physicalDevice,
    VkDisplayKHR                                display,
    const VkDisplayModeCreateInfoKHR*           pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDisplayModeKHR*                           pMode,
    VkResult                                    result);


void PostCallRecordvkGetDisplayPlaneCapabilitiesKHR(
    VkPhysicalDevice                            physicalDevice,
    VkDisplayModeKHR                            mode,
    uint32_t                                    planeIndex,
    VkDisplayPlaneCapabilitiesKHR*              pCapabilities,
    VkResult                                    result);


void PostCallRecordvkCreateDisplayPlaneSurfaceKHR(
    VkInstance                                  instance,
    const VkDisplaySurfaceCreateInfoKHR*        pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface,
    VkResult                                    result);


void PostCallRecordvkCreateSharedSwapchainsKHR(
    VkDevice                                    device,
    uint32_t                                    swapchainCount,
    const VkSwapchainCreateInfoKHR*             pCreateInfos,
    const VkAllocationCallbacks*                pAllocator,
    VkSwapchainKHR*                             pSwapchains,
    VkResult                                    result);


#ifdef VK_USE_PLATFORM_XLIB_KHR

void PostCallRecordvkCreateXlibSurfaceKHR(
    VkInstance                                  instance,
    const VkXlibSurfaceCreateInfoKHR*           pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface,
    VkResult                                    result);


#endif // VK_USE_PLATFORM_XLIB_KHR

#ifdef VK_USE_PLATFORM_XCB_KHR

void PostCallRecordvkCreateXcbSurfaceKHR(
    VkInstance                                  instance,
    const VkXcbSurfaceCreateInfoKHR*            pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface,
    VkResult                                    result);


#endif // VK_USE_PLATFORM_XCB_KHR

#ifdef VK_USE_PLATFORM_WAYLAND_KHR

void PostCallRecordvkCreateWaylandSurfaceKHR(
    VkInstance                                  instance,
    const VkWaylandSurfaceCreateInfoKHR*        pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface,
    VkResult                                    result);


#endif // VK_USE_PLATFORM_WAYLAND_KHR

#ifdef VK_USE_PLATFORM_ANDROID_KHR

void PostCallRecordvkCreateAndroidSurfaceKHR(
    VkInstance                                  instance,
    const VkAndroidSurfaceCreateInfoKHR*        pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface,
    VkResult                                    result);


#endif // VK_USE_PLATFORM_ANDROID_KHR

#ifdef VK_USE_PLATFORM_WIN32_KHR

void PostCallRecordvkCreateWin32SurfaceKHR(
    VkInstance                                  instance,
    const VkWin32SurfaceCreateInfoKHR*          pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface,
    VkResult                                    result);


#endif // VK_USE_PLATFORM_WIN32_KHR

void PostCallRecordvkGetPhysicalDeviceImageFormatProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceImageFormatInfo2*     pImageFormatInfo,
    VkImageFormatProperties2*                   pImageFormatProperties,
    VkResult                                    result);


void PostCallRecordvkEnumeratePhysicalDeviceGroupsKHR(
    VkInstance                                  instance,
    uint32_t*                                   pPhysicalDeviceGroupCount,
    VkPhysicalDeviceGroupProperties*            pPhysicalDeviceGroupProperties,
    VkResult                                    result);


#ifdef VK_USE_PLATFORM_WIN32_KHR

void PostCallRecordvkGetMemoryWin32HandleKHR(
    VkDevice                                    device,
    const VkMemoryGetWin32HandleInfoKHR*        pGetWin32HandleInfo,
    HANDLE*                                     pHandle,
    VkResult                                    result);


#endif // VK_USE_PLATFORM_WIN32_KHR

#ifdef VK_USE_PLATFORM_WIN32_KHR

void PostCallRecordvkGetMemoryWin32HandlePropertiesKHR(
    VkDevice                                    device,
    VkExternalMemoryHandleTypeFlagBits          handleType,
    HANDLE                                      handle,
    VkMemoryWin32HandlePropertiesKHR*           pMemoryWin32HandleProperties,
    VkResult                                    result);


#endif // VK_USE_PLATFORM_WIN32_KHR

void PostCallRecordvkGetMemoryFdKHR(
    VkDevice                                    device,
    const VkMemoryGetFdInfoKHR*                 pGetFdInfo,
    int*                                        pFd,
    VkResult                                    result);


void PostCallRecordvkGetMemoryFdPropertiesKHR(
    VkDevice                                    device,
    VkExternalMemoryHandleTypeFlagBits          handleType,
    int                                         fd,
    VkMemoryFdPropertiesKHR*                    pMemoryFdProperties,
    VkResult                                    result);


#ifdef VK_USE_PLATFORM_WIN32_KHR

void PostCallRecordvkImportSemaphoreWin32HandleKHR(
    VkDevice                                    device,
    const VkImportSemaphoreWin32HandleInfoKHR*  pImportSemaphoreWin32HandleInfo,
    VkResult                                    result);


#endif // VK_USE_PLATFORM_WIN32_KHR

#ifdef VK_USE_PLATFORM_WIN32_KHR

void PostCallRecordvkGetSemaphoreWin32HandleKHR(
    VkDevice                                    device,
    const VkSemaphoreGetWin32HandleInfoKHR*     pGetWin32HandleInfo,
    HANDLE*                                     pHandle,
    VkResult                                    result);


#endif // VK_USE_PLATFORM_WIN32_KHR

void PostCallRecordvkImportSemaphoreFdKHR(
    VkDevice                                    device,
    const VkImportSemaphoreFdInfoKHR*           pImportSemaphoreFdInfo,
    VkResult                                    result);


void PostCallRecordvkGetSemaphoreFdKHR(
    VkDevice                                    device,
    const VkSemaphoreGetFdInfoKHR*              pGetFdInfo,
    int*                                        pFd,
    VkResult                                    result);


void PostCallRecordvkCreateDescriptorUpdateTemplateKHR(
    VkDevice                                    device,
    const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDescriptorUpdateTemplate*                 pDescriptorUpdateTemplate,
    VkResult                                    result);


void PostCallRecordvkCreateRenderPass2KHR(
    VkDevice                                    device,
    const VkRenderPassCreateInfo2*              pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkRenderPass*                               pRenderPass,
    VkResult                                    result);


void PostCallRecordvkGetSwapchainStatusKHR(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    VkResult                                    result);


#ifdef VK_USE_PLATFORM_WIN32_KHR

void PostCallRecordvkImportFenceWin32HandleKHR(
    VkDevice                                    device,
    const VkImportFenceWin32HandleInfoKHR*      pImportFenceWin32HandleInfo,
    VkResult                                    result);


#endif // VK_USE_PLATFORM_WIN32_KHR

#ifdef VK_USE_PLATFORM_WIN32_KHR

void PostCallRecordvkGetFenceWin32HandleKHR(
    VkDevice                                    device,
    const VkFenceGetWin32HandleInfoKHR*         pGetWin32HandleInfo,
    HANDLE*                                     pHandle,
    VkResult                                    result);


#endif // VK_USE_PLATFORM_WIN32_KHR

void PostCallRecordvkImportFenceFdKHR(
    VkDevice                                    device,
    const VkImportFenceFdInfoKHR*               pImportFenceFdInfo,
    VkResult                                    result);


void PostCallRecordvkGetFenceFdKHR(
    VkDevice                                    device,
    const VkFenceGetFdInfoKHR*                  pGetFdInfo,
    int*                                        pFd,
    VkResult                                    result);


void PostCallRecordvkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    queueFamilyIndex,
    uint32_t*                                   pCounterCount,
    VkPerformanceCounterKHR*                    pCounters,
    VkPerformanceCounterDescriptionKHR*         pCounterDescriptions,
    VkResult                                    result);


void PostCallRecordvkAcquireProfilingLockKHR(
    VkDevice                                    device,
    const VkAcquireProfilingLockInfoKHR*        pInfo,
    VkResult                                    result);


void PostCallRecordvkGetPhysicalDeviceSurfaceCapabilities2KHR(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceSurfaceInfo2KHR*      pSurfaceInfo,
    VkSurfaceCapabilities2KHR*                  pSurfaceCapabilities,
    VkResult                                    result);


void PostCallRecordvkGetPhysicalDeviceSurfaceFormats2KHR(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceSurfaceInfo2KHR*      pSurfaceInfo,
    uint32_t*                                   pSurfaceFormatCount,
    VkSurfaceFormat2KHR*                        pSurfaceFormats,
    VkResult                                    result);


void PostCallRecordvkGetPhysicalDeviceDisplayProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pPropertyCount,
    VkDisplayProperties2KHR*                    pProperties,
    VkResult                                    result);


void PostCallRecordvkGetPhysicalDeviceDisplayPlaneProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pPropertyCount,
    VkDisplayPlaneProperties2KHR*               pProperties,
    VkResult                                    result);


void PostCallRecordvkGetDisplayModeProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    VkDisplayKHR                                display,
    uint32_t*                                   pPropertyCount,
    VkDisplayModeProperties2KHR*                pProperties,
    VkResult                                    result);


void PostCallRecordvkGetDisplayPlaneCapabilities2KHR(
    VkPhysicalDevice                            physicalDevice,
    const VkDisplayPlaneInfo2KHR*               pDisplayPlaneInfo,
    VkDisplayPlaneCapabilities2KHR*             pCapabilities,
    VkResult                                    result);


void PostCallRecordvkCreateSamplerYcbcrConversionKHR(
    VkDevice                                    device,
    const VkSamplerYcbcrConversionCreateInfo*   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSamplerYcbcrConversion*                   pYcbcrConversion,
    VkResult                                    result);


void PostCallRecordvkBindBufferMemory2KHR(
    VkDevice                                    device,
    uint32_t                                    bindInfoCount,
    const VkBindBufferMemoryInfo*               pBindInfos,
    VkResult                                    result);


void PostCallRecordvkBindImageMemory2KHR(
    VkDevice                                    device,
    uint32_t                                    bindInfoCount,
    const VkBindImageMemoryInfo*                pBindInfos,
    VkResult                                    result);


void PostCallRecordvkGetSemaphoreCounterValueKHR(
    VkDevice                                    device,
    VkSemaphore                                 semaphore,
    uint64_t*                                   pValue,
    VkResult                                    result);


void PostCallRecordvkWaitSemaphoresKHR(
    VkDevice                                    device,
    const VkSemaphoreWaitInfo*                  pWaitInfo,
    uint64_t                                    timeout,
    VkResult                                    result);


void PostCallRecordvkSignalSemaphoreKHR(
    VkDevice                                    device,
    const VkSemaphoreSignalInfo*                pSignalInfo,
    VkResult                                    result);


void PostCallRecordvkGetPipelineExecutablePropertiesKHR(
    VkDevice                                    device,
    const VkPipelineInfoKHR*                    pPipelineInfo,
    uint32_t*                                   pExecutableCount,
    VkPipelineExecutablePropertiesKHR*          pProperties,
    VkResult                                    result);


void PostCallRecordvkGetPipelineExecutableStatisticsKHR(
    VkDevice                                    device,
    const VkPipelineExecutableInfoKHR*          pExecutableInfo,
    uint32_t*                                   pStatisticCount,
    VkPipelineExecutableStatisticKHR*           pStatistics,
    VkResult                                    result);


void PostCallRecordvkGetPipelineExecutableInternalRepresentationsKHR(
    VkDevice                                    device,
    const VkPipelineExecutableInfoKHR*          pExecutableInfo,
    uint32_t*                                   pInternalRepresentationCount,
    VkPipelineExecutableInternalRepresentationKHR* pInternalRepresentations,
    VkResult                                    result);


void PostCallRecordvkCreateDebugReportCallbackEXT(
    VkInstance                                  instance,
    const VkDebugReportCallbackCreateInfoEXT*   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDebugReportCallbackEXT*                   pCallback,
    VkResult                                    result);


void PostCallRecordvkDebugMarkerSetObjectTagEXT(
    VkDevice                                    device,
    const VkDebugMarkerObjectTagInfoEXT*        pTagInfo,
    VkResult                                    result);


void PostCallRecordvkDebugMarkerSetObjectNameEXT(
    VkDevice                                    device,
    const VkDebugMarkerObjectNameInfoEXT*       pNameInfo,
    VkResult                                    result);


void PostCallRecordvkGetShaderInfoAMD(
    VkDevice                                    device,
    VkPipeline                                  pipeline,
    VkShaderStageFlagBits                       shaderStage,
    VkShaderInfoTypeAMD                         infoType,
    size_t*                                     pInfoSize,
    void*                                       pInfo,
    VkResult                                    result);


#ifdef VK_USE_PLATFORM_GGP

void PostCallRecordvkCreateStreamDescriptorSurfaceGGP(
    VkInstance                                  instance,
    const VkStreamDescriptorSurfaceCreateInfoGGP* pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface,
    VkResult                                    result);


#endif // VK_USE_PLATFORM_GGP

void PostCallRecordvkGetPhysicalDeviceExternalImageFormatPropertiesNV(
    VkPhysicalDevice                            physicalDevice,
    VkFormat                                    format,
    VkImageType                                 type,
    VkImageTiling                               tiling,
    VkImageUsageFlags                           usage,
    VkImageCreateFlags                          flags,
    VkExternalMemoryHandleTypeFlagsNV           externalHandleType,
    VkExternalImageFormatPropertiesNV*          pExternalImageFormatProperties,
    VkResult                                    result);


#ifdef VK_USE_PLATFORM_WIN32_KHR

void PostCallRecordvkGetMemoryWin32HandleNV(
    VkDevice                                    device,
    VkDeviceMemory                              memory,
    VkExternalMemoryHandleTypeFlagsNV           handleType,
    HANDLE*                                     pHandle,
    VkResult                                    result);


#endif // VK_USE_PLATFORM_WIN32_KHR

#ifdef VK_USE_PLATFORM_VI_NN

void PostCallRecordvkCreateViSurfaceNN(
    VkInstance                                  instance,
    const VkViSurfaceCreateInfoNN*              pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface,
    VkResult                                    result);


#endif // VK_USE_PLATFORM_VI_NN

void PostCallRecordvkCreateIndirectCommandsLayoutNVX(
    VkDevice                                    device,
    const VkIndirectCommandsLayoutCreateInfoNVX* pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkIndirectCommandsLayoutNVX*                pIndirectCommandsLayout,
    VkResult                                    result);


void PostCallRecordvkCreateObjectTableNVX(
    VkDevice                                    device,
    const VkObjectTableCreateInfoNVX*           pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkObjectTableNVX*                           pObjectTable,
    VkResult                                    result);


void PostCallRecordvkRegisterObjectsNVX(
    VkDevice                                    device,
    VkObjectTableNVX                            objectTable,
    uint32_t                                    objectCount,
    const VkObjectTableEntryNVX* const*         ppObjectTableEntries,
    const uint32_t*                             pObjectIndices,
    VkResult                                    result);


void PostCallRecordvkUnregisterObjectsNVX(
    VkDevice                                    device,
    VkObjectTableNVX                            objectTable,
    uint32_t                                    objectCount,
    const VkObjectEntryTypeNVX*                 pObjectEntryTypes,
    const uint32_t*                             pObjectIndices,
    VkResult                                    result);


#ifdef VK_USE_PLATFORM_XLIB_XRANDR_EXT

void PostCallRecordvkAcquireXlibDisplayEXT(
    VkPhysicalDevice                            physicalDevice,
    Display*                                    dpy,
    VkDisplayKHR                                display,
    VkResult                                    result);


#endif // VK_USE_PLATFORM_XLIB_XRANDR_EXT

void PostCallRecordvkGetPhysicalDeviceSurfaceCapabilities2EXT(
    VkPhysicalDevice                            physicalDevice,
    VkSurfaceKHR                                surface,
    VkSurfaceCapabilities2EXT*                  pSurfaceCapabilities,
    VkResult                                    result);


void PostCallRecordvkGetSwapchainCounterEXT(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    VkSurfaceCounterFlagBitsEXT                 counter,
    uint64_t*                                   pCounterValue,
    VkResult                                    result);


void PostCallRecordvkGetRefreshCycleDurationGOOGLE(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    VkRefreshCycleDurationGOOGLE*               pDisplayTimingProperties,
    VkResult                                    result);


void PostCallRecordvkGetPastPresentationTimingGOOGLE(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    uint32_t*                                   pPresentationTimingCount,
    VkPastPresentationTimingGOOGLE*             pPresentationTimings,
    VkResult                                    result);


#ifdef VK_USE_PLATFORM_IOS_MVK

void PostCallRecordvkCreateIOSSurfaceMVK(
    VkInstance                                  instance,
    const VkIOSSurfaceCreateInfoMVK*            pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface,
    VkResult                                    result);


#endif // VK_USE_PLATFORM_IOS_MVK

#ifdef VK_USE_PLATFORM_MACOS_MVK

void PostCallRecordvkCreateMacOSSurfaceMVK(
    VkInstance                                  instance,
    const VkMacOSSurfaceCreateInfoMVK*          pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface,
    VkResult                                    result);


#endif // VK_USE_PLATFORM_MACOS_MVK

void PostCallRecordvkSetDebugUtilsObjectNameEXT(
    VkDevice                                    device,
    const VkDebugUtilsObjectNameInfoEXT*        pNameInfo,
    VkResult                                    result);


void PostCallRecordvkSetDebugUtilsObjectTagEXT(
    VkDevice                                    device,
    const VkDebugUtilsObjectTagInfoEXT*         pTagInfo,
    VkResult                                    result);


void PostCallRecordvkCreateDebugUtilsMessengerEXT(
    VkInstance                                  instance,
    const VkDebugUtilsMessengerCreateInfoEXT*   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDebugUtilsMessengerEXT*                   pMessenger,
    VkResult                                    result);


#ifdef VK_USE_PLATFORM_ANDROID_KHR

void PostCallRecordvkGetAndroidHardwareBufferPropertiesANDROID(
    VkDevice                                    device,
    const struct AHardwareBuffer*               buffer,
    VkAndroidHardwareBufferPropertiesANDROID*   pProperties,
    VkResult                                    result);


#endif // VK_USE_PLATFORM_ANDROID_KHR

#ifdef VK_USE_PLATFORM_ANDROID_KHR

void PostCallRecordvkGetMemoryAndroidHardwareBufferANDROID(
    VkDevice                                    device,
    const VkMemoryGetAndroidHardwareBufferInfoANDROID* pInfo,
    struct AHardwareBuffer**                    pBuffer,
    VkResult                                    result);


#endif // VK_USE_PLATFORM_ANDROID_KHR

void PostCallRecordvkCreateAccelerationStructureNV(
    VkDevice                                    device,
    const VkAccelerationStructureCreateInfoNV*  pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkAccelerationStructureNV*                  pAccelerationStructure,
    VkResult                                    result);


void PostCallRecordvkBindAccelerationStructureMemoryNV(
    VkDevice                                    device,
    uint32_t                                    bindInfoCount,
    const VkBindAccelerationStructureMemoryInfoNV* pBindInfos,
    VkResult                                    result);


void PostCallRecordvkCreateRayTracingPipelinesNV(
    VkDevice                                    device,
    VkPipelineCache                             pipelineCache,
    uint32_t                                    createInfoCount,
    const VkRayTracingPipelineCreateInfoNV*     pCreateInfos,
    const VkAllocationCallbacks*                pAllocator,
    VkPipeline*                                 pPipelines,
    VkResult                                    result,
    void*                                       state_data);


void PostCallRecordvkGetRayTracingShaderGroupHandlesNV(
    VkDevice                                    device,
    VkPipeline                                  pipeline,
    uint32_t                                    firstGroup,
    uint32_t                                    groupCount,
    size_t                                      dataSize,
    void*                                       pData,
    VkResult                                    result);


void PostCallRecordvkGetAccelerationStructureHandleNV(
    VkDevice                                    device,
    VkAccelerationStructureNV                   accelerationStructure,
    size_t                                      dataSize,
    void*                                       pData,
    VkResult                                    result);


void PostCallRecordvkCompileDeferredNV(
    VkDevice                                    device,
    VkPipeline                                  pipeline,
    uint32_t                                    shader,
    VkResult                                    result);


void PostCallRecordvkGetMemoryHostPointerPropertiesEXT(
    VkDevice                                    device,
    VkExternalMemoryHandleTypeFlagBits          handleType,
    const void*                                 pHostPointer,
    VkMemoryHostPointerPropertiesEXT*           pMemoryHostPointerProperties,
    VkResult                                    result);


void PostCallRecordvkGetPhysicalDeviceCalibrateableTimeDomainsEXT(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pTimeDomainCount,
    VkTimeDomainEXT*                            pTimeDomains,
    VkResult                                    result);


void PostCallRecordvkGetCalibratedTimestampsEXT(
    VkDevice                                    device,
    uint32_t                                    timestampCount,
    const VkCalibratedTimestampInfoEXT*         pTimestampInfos,
    uint64_t*                                   pTimestamps,
    uint64_t*                                   pMaxDeviation,
    VkResult                                    result);


void PostCallRecordvkInitializePerformanceApiINTEL(
    VkDevice                                    device,
    const VkInitializePerformanceApiInfoINTEL*  pInitializeInfo,
    VkResult                                    result);


void PostCallRecordvkCmdSetPerformanceMarkerINTEL(
    VkCommandBuffer                             commandBuffer,
    const VkPerformanceMarkerInfoINTEL*         pMarkerInfo,
    VkResult                                    result);


void PostCallRecordvkCmdSetPerformanceStreamMarkerINTEL(
    VkCommandBuffer                             commandBuffer,
    const VkPerformanceStreamMarkerInfoINTEL*   pMarkerInfo,
    VkResult                                    result);


void PostCallRecordvkCmdSetPerformanceOverrideINTEL(
    VkCommandBuffer                             commandBuffer,
    const VkPerformanceOverrideInfoINTEL*       pOverrideInfo,
    VkResult                                    result);


void PostCallRecordvkAcquirePerformanceConfigurationINTEL(
    VkDevice                                    device,
    const VkPerformanceConfigurationAcquireInfoINTEL* pAcquireInfo,
    VkPerformanceConfigurationINTEL*            pConfiguration,
    VkResult                                    result);


void PostCallRecordvkReleasePerformanceConfigurationINTEL(
    VkDevice                                    device,
    VkPerformanceConfigurationINTEL             configuration,
    VkResult                                    result);


void PostCallRecordvkQueueSetPerformanceConfigurationINTEL(
    VkQueue                                     queue,
    VkPerformanceConfigurationINTEL             configuration,
    VkResult                                    result);


void PostCallRecordvkGetPerformanceParameterINTEL(
    VkDevice                                    device,
    VkPerformanceParameterTypeINTEL             parameter,
    VkPerformanceValueINTEL*                    pValue,
    VkResult                                    result);


#ifdef VK_USE_PLATFORM_FUCHSIA

void PostCallRecordvkCreateImagePipeSurfaceFUCHSIA(
    VkInstance                                  instance,
    const VkImagePipeSurfaceCreateInfoFUCHSIA*  pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface,
    VkResult                                    result);


#endif // VK_USE_PLATFORM_FUCHSIA

#ifdef VK_USE_PLATFORM_METAL_EXT

void PostCallRecordvkCreateMetalSurfaceEXT(
    VkInstance                                  instance,
    const VkMetalSurfaceCreateInfoEXT*          pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface,
    VkResult                                    result);


#endif // VK_USE_PLATFORM_METAL_EXT

void PostCallRecordvkGetPhysicalDeviceToolPropertiesEXT(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pToolCount,
    VkPhysicalDeviceToolPropertiesEXT*          pToolProperties,
    VkResult                                    result);


void PostCallRecordvkGetPhysicalDeviceCooperativeMatrixPropertiesNV(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pPropertyCount,
    VkCooperativeMatrixPropertiesNV*            pProperties,
    VkResult                                    result);


void PostCallRecordvkGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pCombinationCount,
    VkFramebufferMixedSamplesCombinationNV*     pCombinations,
    VkResult                                    result);


#ifdef VK_USE_PLATFORM_WIN32_KHR

void PostCallRecordvkGetPhysicalDeviceSurfacePresentModes2EXT(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceSurfaceInfo2KHR*      pSurfaceInfo,
    uint32_t*                                   pPresentModeCount,
    VkPresentModeKHR*                           pPresentModes,
    VkResult                                    result);


#endif // VK_USE_PLATFORM_WIN32_KHR

#ifdef VK_USE_PLATFORM_WIN32_KHR

void PostCallRecordvkAcquireFullScreenExclusiveModeEXT(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    VkResult                                    result);


#endif // VK_USE_PLATFORM_WIN32_KHR

#ifdef VK_USE_PLATFORM_WIN32_KHR

void PostCallRecordvkReleaseFullScreenExclusiveModeEXT(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    VkResult                                    result);


#endif // VK_USE_PLATFORM_WIN32_KHR

#ifdef VK_USE_PLATFORM_WIN32_KHR

void PostCallRecordvkGetDeviceGroupSurfacePresentModes2EXT(
    VkDevice                                    device,
    const VkPhysicalDeviceSurfaceInfo2KHR*      pSurfaceInfo,
    VkDeviceGroupPresentModeFlagsKHR*           pModes,
    VkResult                                    result);


#endif // VK_USE_PLATFORM_WIN32_KHR

void PostCallRecordvkCreateHeadlessSurfaceEXT(
    VkInstance                                  instance,
    const VkHeadlessSurfaceCreateInfoEXT*       pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface,
    VkResult                                    result);






const std::unordered_map<std::string, DeprecationData>  deprecated_extensions = {
    {"VK_AMD_draw_indirect_count", {kExtPromoted, "VK_KHR_draw_indirect_count"}},
    {"VK_AMD_gpu_shader_half_float", {kExtDeprecated, "VK_KHR_shader_float16_int8"}},
    {"VK_AMD_gpu_shader_int16", {kExtDeprecated, "VK_KHR_shader_float16_int8"}},
    {"VK_AMD_negative_viewport_height", {kExtObsoleted, "VK_KHR_maintenance1"}},
    {"VK_EXT_buffer_device_address", {kExtDeprecated, "VK_KHR_buffer_device_address"}},
    {"VK_EXT_debug_marker", {kExtPromoted, "VK_EXT_debug_utils"}},
// ADD BACK AFTER LAYER TESTS SWITCH TO DEBUG_UTILS!       {"VK_EXT_debug_report", {kExtDeprecated, "VK_EXT_debug_utils"}},
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

