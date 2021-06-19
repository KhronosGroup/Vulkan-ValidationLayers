#pragma once
// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See dispatch_helper_generator.py for modifications

/*
 * Copyright (c) 2015-2021 The Khronos Group Inc.
 * Copyright (c) 2015-2021 Valve Corporation
 * Copyright (c) 2015-2021 LunarG, Inc.
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
 * Author: Courtney Goeltzenleuchter <courtney@LunarG.com>
 * Author: Jon Ashburn <jon@lunarg.com>
 * Author: Mark Lobodzinski <mark@lunarg.com>
 */

#include <vulkan/vulkan_sc.h>
#include <vulkan/vk_layer.h>
#include <cstring>
#include <string>
#include "vk_layer_dispatch_table.h"
#include "vk_extension_helper.h"

static VKAPI_ATTR VkResult VKAPI_CALL StubBindBufferMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindBufferMemoryInfo* pBindInfos) { return VK_SUCCESS; };
static VKAPI_ATTR VkResult VKAPI_CALL StubBindImageMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo* pBindInfos) { return VK_SUCCESS; };
static VKAPI_ATTR void VKAPI_CALL StubGetDeviceGroupPeerMemoryFeatures(VkDevice device, uint32_t heapIndex, uint32_t localDeviceIndex, uint32_t remoteDeviceIndex, VkPeerMemoryFeatureFlags* pPeerMemoryFeatures) {  };
static VKAPI_ATTR void VKAPI_CALL StubCmdSetDeviceMask(VkCommandBuffer commandBuffer, uint32_t deviceMask) {  };
static VKAPI_ATTR void VKAPI_CALL StubCmdDispatchBase(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY, uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) {  };
static VKAPI_ATTR VkResult VKAPI_CALL StubEnumeratePhysicalDeviceGroups(VkInstance instance, uint32_t* pPhysicalDeviceGroupCount, VkPhysicalDeviceGroupProperties* pPhysicalDeviceGroupProperties) { return VK_SUCCESS; };
static VKAPI_ATTR void VKAPI_CALL StubGetImageMemoryRequirements2(VkDevice device, const VkImageMemoryRequirementsInfo2* pInfo, VkMemoryRequirements2* pMemoryRequirements) {  };
static VKAPI_ATTR void VKAPI_CALL StubGetBufferMemoryRequirements2(VkDevice device, const VkBufferMemoryRequirementsInfo2* pInfo, VkMemoryRequirements2* pMemoryRequirements) {  };
static VKAPI_ATTR void VKAPI_CALL StubGetPhysicalDeviceFeatures2(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures2* pFeatures) {  };
static VKAPI_ATTR void VKAPI_CALL StubGetPhysicalDeviceProperties2(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties2* pProperties) {  };
static VKAPI_ATTR void VKAPI_CALL StubGetPhysicalDeviceFormatProperties2(VkPhysicalDevice physicalDevice, VkFormat format, VkFormatProperties2* pFormatProperties) {  };
static VKAPI_ATTR VkResult VKAPI_CALL StubGetPhysicalDeviceImageFormatProperties2(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceImageFormatInfo2* pImageFormatInfo, VkImageFormatProperties2* pImageFormatProperties) { return VK_SUCCESS; };
static VKAPI_ATTR void VKAPI_CALL StubGetPhysicalDeviceQueueFamilyProperties2(VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount, VkQueueFamilyProperties2* pQueueFamilyProperties) {  };
static VKAPI_ATTR void VKAPI_CALL StubGetPhysicalDeviceMemoryProperties2(VkPhysicalDevice physicalDevice, VkPhysicalDeviceMemoryProperties2* pMemoryProperties) {  };
static VKAPI_ATTR void VKAPI_CALL StubGetDeviceQueue2(VkDevice device, const VkDeviceQueueInfo2* pQueueInfo, VkQueue* pQueue) {  };
static VKAPI_ATTR VkResult VKAPI_CALL StubCreateSamplerYcbcrConversion(VkDevice device, const VkSamplerYcbcrConversionCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSamplerYcbcrConversion* pYcbcrConversion) { return VK_SUCCESS; };
static VKAPI_ATTR void VKAPI_CALL StubDestroySamplerYcbcrConversion(VkDevice device, VkSamplerYcbcrConversion ycbcrConversion, const VkAllocationCallbacks* pAllocator) {  };
static VKAPI_ATTR void VKAPI_CALL StubGetPhysicalDeviceExternalBufferProperties(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalBufferInfo* pExternalBufferInfo, VkExternalBufferProperties* pExternalBufferProperties) {  };
static VKAPI_ATTR void VKAPI_CALL StubGetPhysicalDeviceExternalFenceProperties(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalFenceInfo* pExternalFenceInfo, VkExternalFenceProperties* pExternalFenceProperties) {  };
static VKAPI_ATTR void VKAPI_CALL StubGetPhysicalDeviceExternalSemaphoreProperties(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalSemaphoreInfo* pExternalSemaphoreInfo, VkExternalSemaphoreProperties* pExternalSemaphoreProperties) {  };
static VKAPI_ATTR void VKAPI_CALL StubGetDescriptorSetLayoutSupport(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo, VkDescriptorSetLayoutSupport* pSupport) {  };
static VKAPI_ATTR void VKAPI_CALL StubCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride) {  };
static VKAPI_ATTR void VKAPI_CALL StubCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride) {  };
static VKAPI_ATTR VkResult VKAPI_CALL StubCreateRenderPass2(VkDevice device, const VkRenderPassCreateInfo2* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass) { return VK_SUCCESS; };
static VKAPI_ATTR void VKAPI_CALL StubCmdBeginRenderPass2(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo*      pRenderPassBegin, const VkSubpassBeginInfo*      pSubpassBeginInfo) {  };
static VKAPI_ATTR void VKAPI_CALL StubCmdNextSubpass2(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo*      pSubpassBeginInfo, const VkSubpassEndInfo*        pSubpassEndInfo) {  };
static VKAPI_ATTR void VKAPI_CALL StubCmdEndRenderPass2(VkCommandBuffer commandBuffer, const VkSubpassEndInfo*        pSubpassEndInfo) {  };
static VKAPI_ATTR void VKAPI_CALL StubResetQueryPool(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount) {  };
static VKAPI_ATTR VkResult VKAPI_CALL StubGetSemaphoreCounterValue(VkDevice device, VkSemaphore semaphore, uint64_t* pValue) { return VK_SUCCESS; };
static VKAPI_ATTR VkResult VKAPI_CALL StubWaitSemaphores(VkDevice device, const VkSemaphoreWaitInfo* pWaitInfo, uint64_t timeout) { return VK_SUCCESS; };
static VKAPI_ATTR VkResult VKAPI_CALL StubSignalSemaphore(VkDevice device, const VkSemaphoreSignalInfo* pSignalInfo) { return VK_SUCCESS; };
static VKAPI_ATTR VkDeviceAddress VKAPI_CALL StubGetBufferDeviceAddress(VkDevice device, const VkBufferDeviceAddressInfo* pInfo) { return 0; };
static VKAPI_ATTR uint64_t VKAPI_CALL StubGetBufferOpaqueCaptureAddress(VkDevice device, const VkBufferDeviceAddressInfo* pInfo) { return 0; };
static VKAPI_ATTR uint64_t VKAPI_CALL StubGetDeviceMemoryOpaqueCaptureAddress(VkDevice device, const VkDeviceMemoryOpaqueCaptureAddressInfo* pInfo) { return 0; };
static VKAPI_ATTR void VKAPI_CALL StubGetCommandPoolMemoryConsumption(VkDevice device, VkCommandPool commandPool, VkCommandBuffer commandBuffer, VkCommandPoolMemoryConsumption* pConsumption) {  };
static VKAPI_ATTR VkResult VKAPI_CALL StubGetFaultData(VkDevice device, VkFaultQueryBehavior faultQueryBehavior, VkBool32* pUnrecordedFaults, uint32_t* pFaultCount, VkFaultData* pFaults) { return VK_SUCCESS; };
static VKAPI_ATTR void VKAPI_CALL StubDestroySurfaceKHR(VkInstance instance, VkSurfaceKHR surface, const VkAllocationCallbacks* pAllocator) {  };
static VKAPI_ATTR VkResult VKAPI_CALL StubGetPhysicalDeviceSurfaceSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, VkSurfaceKHR surface, VkBool32* pSupported) { return VK_SUCCESS; };
static VKAPI_ATTR VkResult VKAPI_CALL StubGetPhysicalDeviceSurfaceCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkSurfaceCapabilitiesKHR* pSurfaceCapabilities) { return VK_SUCCESS; };
static VKAPI_ATTR VkResult VKAPI_CALL StubGetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t* pSurfaceFormatCount, VkSurfaceFormatKHR* pSurfaceFormats) { return VK_SUCCESS; };
static VKAPI_ATTR VkResult VKAPI_CALL StubGetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t* pPresentModeCount, VkPresentModeKHR* pPresentModes) { return VK_SUCCESS; };
static VKAPI_ATTR VkResult VKAPI_CALL StubCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain) { return VK_SUCCESS; };
static VKAPI_ATTR VkResult VKAPI_CALL StubGetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapchain, uint32_t* pSwapchainImageCount, VkImage* pSwapchainImages) { return VK_SUCCESS; };
static VKAPI_ATTR VkResult VKAPI_CALL StubAcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore, VkFence fence, uint32_t* pImageIndex) { return VK_SUCCESS; };
static VKAPI_ATTR VkResult VKAPI_CALL StubQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* pPresentInfo) { return VK_SUCCESS; };
static VKAPI_ATTR VkResult VKAPI_CALL StubGetDeviceGroupPresentCapabilitiesKHR(VkDevice device, VkDeviceGroupPresentCapabilitiesKHR* pDeviceGroupPresentCapabilities) { return VK_SUCCESS; };
static VKAPI_ATTR VkResult VKAPI_CALL StubGetDeviceGroupSurfacePresentModesKHR(VkDevice device, VkSurfaceKHR surface, VkDeviceGroupPresentModeFlagsKHR* pModes) { return VK_SUCCESS; };
static VKAPI_ATTR VkResult VKAPI_CALL StubGetPhysicalDevicePresentRectanglesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t* pRectCount, VkRect2D* pRects) { return VK_SUCCESS; };
static VKAPI_ATTR VkResult VKAPI_CALL StubAcquireNextImage2KHR(VkDevice device, const VkAcquireNextImageInfoKHR* pAcquireInfo, uint32_t* pImageIndex) { return VK_SUCCESS; };
static VKAPI_ATTR VkResult VKAPI_CALL StubGetPhysicalDeviceDisplayPropertiesKHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount, VkDisplayPropertiesKHR* pProperties) { return VK_SUCCESS; };
static VKAPI_ATTR VkResult VKAPI_CALL StubGetPhysicalDeviceDisplayPlanePropertiesKHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount, VkDisplayPlanePropertiesKHR* pProperties) { return VK_SUCCESS; };
static VKAPI_ATTR VkResult VKAPI_CALL StubGetDisplayPlaneSupportedDisplaysKHR(VkPhysicalDevice physicalDevice, uint32_t planeIndex, uint32_t* pDisplayCount, VkDisplayKHR* pDisplays) { return VK_SUCCESS; };
static VKAPI_ATTR VkResult VKAPI_CALL StubGetDisplayModePropertiesKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display, uint32_t* pPropertyCount, VkDisplayModePropertiesKHR* pProperties) { return VK_SUCCESS; };
static VKAPI_ATTR VkResult VKAPI_CALL StubCreateDisplayModeKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display, const VkDisplayModeCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDisplayModeKHR* pMode) { return VK_SUCCESS; };
static VKAPI_ATTR VkResult VKAPI_CALL StubGetDisplayPlaneCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkDisplayModeKHR mode, uint32_t planeIndex, VkDisplayPlaneCapabilitiesKHR* pCapabilities) { return VK_SUCCESS; };
static VKAPI_ATTR VkResult VKAPI_CALL StubCreateDisplayPlaneSurfaceKHR(VkInstance instance, const VkDisplaySurfaceCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) { return VK_SUCCESS; };
static VKAPI_ATTR VkResult VKAPI_CALL StubCreateSharedSwapchainsKHR(VkDevice device, uint32_t swapchainCount, const VkSwapchainCreateInfoKHR* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchains) { return VK_SUCCESS; };
static VKAPI_ATTR VkResult VKAPI_CALL StubGetMemoryFdKHR(VkDevice device, const VkMemoryGetFdInfoKHR* pGetFdInfo, int* pFd) { return VK_SUCCESS; };
static VKAPI_ATTR VkResult VKAPI_CALL StubGetMemoryFdPropertiesKHR(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType, int fd, VkMemoryFdPropertiesKHR* pMemoryFdProperties) { return VK_SUCCESS; };
static VKAPI_ATTR VkResult VKAPI_CALL StubImportSemaphoreFdKHR(VkDevice device, const VkImportSemaphoreFdInfoKHR* pImportSemaphoreFdInfo) { return VK_SUCCESS; };
static VKAPI_ATTR VkResult VKAPI_CALL StubGetSemaphoreFdKHR(VkDevice device, const VkSemaphoreGetFdInfoKHR* pGetFdInfo, int* pFd) { return VK_SUCCESS; };
static VKAPI_ATTR VkResult VKAPI_CALL StubGetSwapchainStatusKHR(VkDevice device, VkSwapchainKHR swapchain) { return VK_SUCCESS; };
static VKAPI_ATTR VkResult VKAPI_CALL StubImportFenceFdKHR(VkDevice device, const VkImportFenceFdInfoKHR* pImportFenceFdInfo) { return VK_SUCCESS; };
static VKAPI_ATTR VkResult VKAPI_CALL StubGetFenceFdKHR(VkDevice device, const VkFenceGetFdInfoKHR* pGetFdInfo, int* pFd) { return VK_SUCCESS; };
static VKAPI_ATTR VkResult VKAPI_CALL StubEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, uint32_t* pCounterCount, VkPerformanceCounterKHR* pCounters, VkPerformanceCounterDescriptionKHR* pCounterDescriptions) { return VK_SUCCESS; };
static VKAPI_ATTR void VKAPI_CALL StubGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR(VkPhysicalDevice physicalDevice, const VkQueryPoolPerformanceCreateInfoKHR* pPerformanceQueryCreateInfo, uint32_t* pNumPasses) {  };
static VKAPI_ATTR VkResult VKAPI_CALL StubAcquireProfilingLockKHR(VkDevice device, const VkAcquireProfilingLockInfoKHR* pInfo) { return VK_SUCCESS; };
static VKAPI_ATTR void VKAPI_CALL StubReleaseProfilingLockKHR(VkDevice device) {  };
static VKAPI_ATTR VkResult VKAPI_CALL StubGetPhysicalDeviceSurfaceCapabilities2KHR(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo, VkSurfaceCapabilities2KHR* pSurfaceCapabilities) { return VK_SUCCESS; };
static VKAPI_ATTR VkResult VKAPI_CALL StubGetPhysicalDeviceSurfaceFormats2KHR(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo, uint32_t* pSurfaceFormatCount, VkSurfaceFormat2KHR* pSurfaceFormats) { return VK_SUCCESS; };
static VKAPI_ATTR VkResult VKAPI_CALL StubGetPhysicalDeviceDisplayProperties2KHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount, VkDisplayProperties2KHR* pProperties) { return VK_SUCCESS; };
static VKAPI_ATTR VkResult VKAPI_CALL StubGetPhysicalDeviceDisplayPlaneProperties2KHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount, VkDisplayPlaneProperties2KHR* pProperties) { return VK_SUCCESS; };
static VKAPI_ATTR VkResult VKAPI_CALL StubGetDisplayModeProperties2KHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display, uint32_t* pPropertyCount, VkDisplayModeProperties2KHR* pProperties) { return VK_SUCCESS; };
static VKAPI_ATTR VkResult VKAPI_CALL StubGetDisplayPlaneCapabilities2KHR(VkPhysicalDevice physicalDevice, const VkDisplayPlaneInfo2KHR* pDisplayPlaneInfo, VkDisplayPlaneCapabilities2KHR* pCapabilities) { return VK_SUCCESS; };
static VKAPI_ATTR VkResult VKAPI_CALL StubGetPhysicalDeviceFragmentShadingRatesKHR(VkPhysicalDevice physicalDevice, uint32_t* pFragmentShadingRateCount, VkPhysicalDeviceFragmentShadingRateKHR* pFragmentShadingRates) { return VK_SUCCESS; };
static VKAPI_ATTR void VKAPI_CALL StubCmdSetFragmentShadingRateKHR(VkCommandBuffer           commandBuffer, const VkExtent2D*                           pFragmentSize, const VkFragmentShadingRateCombinerOpKHR    combinerOps[2]) {  };
static VKAPI_ATTR void VKAPI_CALL StubCmdRefreshObjectsKHR(VkCommandBuffer commandBuffer, const VkRefreshObjectListKHR* pRefreshObjects) {  };
static VKAPI_ATTR VkResult VKAPI_CALL StubGetPhysicalDeviceRefreshableObjectTypesKHR(VkPhysicalDevice physicalDevice, uint32_t* pRefreshableObjectTypeCount, VkObjectType* pRefreshableObjectTypes) { return VK_SUCCESS; };
static VKAPI_ATTR void VKAPI_CALL StubCmdSetEvent2KHR(VkCommandBuffer                   commandBuffer, VkEvent                                             event, const VkDependencyInfoKHR*                          pDependencyInfo) {  };
static VKAPI_ATTR void VKAPI_CALL StubCmdResetEvent2KHR(VkCommandBuffer                   commandBuffer, VkEvent                                             event, VkPipelineStageFlags2KHR                            stageMask) {  };
static VKAPI_ATTR void VKAPI_CALL StubCmdWaitEvents2KHR(VkCommandBuffer                   commandBuffer, uint32_t                                            eventCount, const VkEvent*                     pEvents, const VkDependencyInfoKHR*         pDependencyInfos) {  };
static VKAPI_ATTR void VKAPI_CALL StubCmdPipelineBarrier2KHR(VkCommandBuffer                   commandBuffer, const VkDependencyInfoKHR*                                pDependencyInfo) {  };
static VKAPI_ATTR void VKAPI_CALL StubCmdWriteTimestamp2KHR(VkCommandBuffer                   commandBuffer, VkPipelineStageFlags2KHR                            stage, VkQueryPool                                         queryPool, uint32_t                                            query) {  };
static VKAPI_ATTR VkResult VKAPI_CALL StubQueueSubmit2KHR(VkQueue                           queue, uint32_t                            submitCount, const VkSubmitInfo2KHR*           pSubmits, VkFence           fence) { return VK_SUCCESS; };
static VKAPI_ATTR void VKAPI_CALL StubCmdWriteBufferMarker2AMD(VkCommandBuffer                   commandBuffer, VkPipelineStageFlags2KHR                            stage, VkBuffer                                            dstBuffer, VkDeviceSize                                        dstOffset, uint32_t                                            marker) {  };
static VKAPI_ATTR void VKAPI_CALL StubGetQueueCheckpointData2NV(VkQueue queue, uint32_t* pCheckpointDataCount, VkCheckpointData2NV* pCheckpointData) {  };
static VKAPI_ATTR void VKAPI_CALL StubCmdCopyBuffer2KHR(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2KHR* pCopyBufferInfo) {  };
static VKAPI_ATTR void VKAPI_CALL StubCmdCopyImage2KHR(VkCommandBuffer commandBuffer, const VkCopyImageInfo2KHR* pCopyImageInfo) {  };
static VKAPI_ATTR void VKAPI_CALL StubCmdCopyBufferToImage2KHR(VkCommandBuffer commandBuffer, const VkCopyBufferToImageInfo2KHR* pCopyBufferToImageInfo) {  };
static VKAPI_ATTR void VKAPI_CALL StubCmdCopyImageToBuffer2KHR(VkCommandBuffer commandBuffer, const VkCopyImageToBufferInfo2KHR* pCopyImageToBufferInfo) {  };
static VKAPI_ATTR void VKAPI_CALL StubCmdBlitImage2KHR(VkCommandBuffer commandBuffer, const VkBlitImageInfo2KHR* pBlitImageInfo) {  };
static VKAPI_ATTR void VKAPI_CALL StubCmdResolveImage2KHR(VkCommandBuffer commandBuffer, const VkResolveImageInfo2KHR* pResolveImageInfo) {  };
static VKAPI_ATTR VkResult VKAPI_CALL StubReleaseDisplayEXT(VkPhysicalDevice physicalDevice, VkDisplayKHR display) { return VK_SUCCESS; };
static VKAPI_ATTR VkResult VKAPI_CALL StubGetPhysicalDeviceSurfaceCapabilities2EXT(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkSurfaceCapabilities2EXT* pSurfaceCapabilities) { return VK_SUCCESS; };
static VKAPI_ATTR VkResult VKAPI_CALL StubDisplayPowerControlEXT(VkDevice device, VkDisplayKHR display, const VkDisplayPowerInfoEXT* pDisplayPowerInfo) { return VK_SUCCESS; };
static VKAPI_ATTR VkResult VKAPI_CALL StubRegisterDeviceEventEXT(VkDevice device, const VkDeviceEventInfoEXT* pDeviceEventInfo, const VkAllocationCallbacks* pAllocator, VkFence* pFence) { return VK_SUCCESS; };
static VKAPI_ATTR VkResult VKAPI_CALL StubRegisterDisplayEventEXT(VkDevice device, VkDisplayKHR display, const VkDisplayEventInfoEXT* pDisplayEventInfo, const VkAllocationCallbacks* pAllocator, VkFence* pFence) { return VK_SUCCESS; };
static VKAPI_ATTR VkResult VKAPI_CALL StubGetSwapchainCounterEXT(VkDevice device, VkSwapchainKHR swapchain, VkSurfaceCounterFlagBitsEXT counter, uint64_t* pCounterValue) { return VK_SUCCESS; };
static VKAPI_ATTR void VKAPI_CALL StubCmdSetDiscardRectangleEXT(VkCommandBuffer commandBuffer, uint32_t firstDiscardRectangle, uint32_t discardRectangleCount, const VkRect2D* pDiscardRectangles) {  };
static VKAPI_ATTR void VKAPI_CALL StubSetHdrMetadataEXT(VkDevice device, uint32_t swapchainCount, const VkSwapchainKHR* pSwapchains, const VkHdrMetadataEXT* pMetadata) {  };
static VKAPI_ATTR VkResult VKAPI_CALL StubSetDebugUtilsObjectNameEXT(VkDevice device, const VkDebugUtilsObjectNameInfoEXT* pNameInfo) { return VK_SUCCESS; };
static VKAPI_ATTR VkResult VKAPI_CALL StubSetDebugUtilsObjectTagEXT(VkDevice device, const VkDebugUtilsObjectTagInfoEXT* pTagInfo) { return VK_SUCCESS; };
static VKAPI_ATTR void VKAPI_CALL StubQueueBeginDebugUtilsLabelEXT(VkQueue queue, const VkDebugUtilsLabelEXT* pLabelInfo) {  };
static VKAPI_ATTR void VKAPI_CALL StubQueueEndDebugUtilsLabelEXT(VkQueue queue) {  };
static VKAPI_ATTR void VKAPI_CALL StubQueueInsertDebugUtilsLabelEXT(VkQueue queue, const VkDebugUtilsLabelEXT* pLabelInfo) {  };
static VKAPI_ATTR void VKAPI_CALL StubCmdBeginDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT* pLabelInfo) {  };
static VKAPI_ATTR void VKAPI_CALL StubCmdEndDebugUtilsLabelEXT(VkCommandBuffer commandBuffer) {  };
static VKAPI_ATTR void VKAPI_CALL StubCmdInsertDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT* pLabelInfo) {  };
static VKAPI_ATTR VkResult VKAPI_CALL StubCreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pMessenger) { return VK_SUCCESS; };
static VKAPI_ATTR void VKAPI_CALL StubDestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT messenger, const VkAllocationCallbacks* pAllocator) {  };
static VKAPI_ATTR void VKAPI_CALL StubSubmitDebugUtilsMessageEXT(VkInstance instance, VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageTypes, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData) {  };
static VKAPI_ATTR void VKAPI_CALL StubCmdSetSampleLocationsEXT(VkCommandBuffer commandBuffer, const VkSampleLocationsInfoEXT* pSampleLocationsInfo) {  };
static VKAPI_ATTR void VKAPI_CALL StubGetPhysicalDeviceMultisamplePropertiesEXT(VkPhysicalDevice physicalDevice, VkSampleCountFlagBits samples, VkMultisamplePropertiesEXT* pMultisampleProperties) {  };
static VKAPI_ATTR VkResult VKAPI_CALL StubGetImageDrmFormatModifierPropertiesEXT(VkDevice device, VkImage image, VkImageDrmFormatModifierPropertiesEXT* pProperties) { return VK_SUCCESS; };
static VKAPI_ATTR VkResult VKAPI_CALL StubGetMemoryHostPointerPropertiesEXT(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType, const void* pHostPointer, VkMemoryHostPointerPropertiesEXT* pMemoryHostPointerProperties) { return VK_SUCCESS; };
static VKAPI_ATTR VkResult VKAPI_CALL StubGetPhysicalDeviceCalibrateableTimeDomainsEXT(VkPhysicalDevice physicalDevice, uint32_t* pTimeDomainCount, VkTimeDomainEXT* pTimeDomains) { return VK_SUCCESS; };
static VKAPI_ATTR VkResult VKAPI_CALL StubGetCalibratedTimestampsEXT(VkDevice device, uint32_t timestampCount, const VkCalibratedTimestampInfoEXT* pTimestampInfos, uint64_t* pTimestamps, uint64_t* pMaxDeviation) { return VK_SUCCESS; };
static VKAPI_ATTR VkResult VKAPI_CALL StubCreateHeadlessSurfaceEXT(VkInstance instance, const VkHeadlessSurfaceCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) { return VK_SUCCESS; };
static VKAPI_ATTR void VKAPI_CALL StubCmdSetLineStippleEXT(VkCommandBuffer commandBuffer, uint32_t lineStippleFactor, uint16_t lineStipplePattern) {  };
static VKAPI_ATTR void VKAPI_CALL StubCmdSetCullModeEXT(VkCommandBuffer commandBuffer, VkCullModeFlags cullMode) {  };
static VKAPI_ATTR void VKAPI_CALL StubCmdSetFrontFaceEXT(VkCommandBuffer commandBuffer, VkFrontFace frontFace) {  };
static VKAPI_ATTR void VKAPI_CALL StubCmdSetPrimitiveTopologyEXT(VkCommandBuffer commandBuffer, VkPrimitiveTopology primitiveTopology) {  };
static VKAPI_ATTR void VKAPI_CALL StubCmdSetViewportWithCountEXT(VkCommandBuffer commandBuffer, uint32_t viewportCount, const VkViewport* pViewports) {  };
static VKAPI_ATTR void VKAPI_CALL StubCmdSetScissorWithCountEXT(VkCommandBuffer commandBuffer, uint32_t scissorCount, const VkRect2D* pScissors) {  };
static VKAPI_ATTR void VKAPI_CALL StubCmdBindVertexBuffers2EXT(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount, const VkBuffer* pBuffers, const VkDeviceSize* pOffsets, const VkDeviceSize* pSizes, const VkDeviceSize* pStrides) {  };
static VKAPI_ATTR void VKAPI_CALL StubCmdSetDepthTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthTestEnable) {  };
static VKAPI_ATTR void VKAPI_CALL StubCmdSetDepthWriteEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthWriteEnable) {  };
static VKAPI_ATTR void VKAPI_CALL StubCmdSetDepthCompareOpEXT(VkCommandBuffer commandBuffer, VkCompareOp depthCompareOp) {  };
static VKAPI_ATTR void VKAPI_CALL StubCmdSetDepthBoundsTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthBoundsTestEnable) {  };
static VKAPI_ATTR void VKAPI_CALL StubCmdSetStencilTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 stencilTestEnable) {  };
static VKAPI_ATTR void VKAPI_CALL StubCmdSetStencilOpEXT(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, VkStencilOp failOp, VkStencilOp passOp, VkStencilOp depthFailOp, VkCompareOp compareOp) {  };
static VKAPI_ATTR void VKAPI_CALL StubCmdSetVertexInputEXT(VkCommandBuffer commandBuffer, uint32_t vertexBindingDescriptionCount, const VkVertexInputBindingDescription2EXT* pVertexBindingDescriptions, uint32_t vertexAttributeDescriptionCount, const VkVertexInputAttributeDescription2EXT* pVertexAttributeDescriptions) {  };
static VKAPI_ATTR void VKAPI_CALL StubCmdSetPatchControlPointsEXT(VkCommandBuffer commandBuffer, uint32_t patchControlPoints) {  };
static VKAPI_ATTR void VKAPI_CALL StubCmdSetRasterizerDiscardEnableEXT(VkCommandBuffer commandBuffer, VkBool32 rasterizerDiscardEnable) {  };
static VKAPI_ATTR void VKAPI_CALL StubCmdSetDepthBiasEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthBiasEnable) {  };
static VKAPI_ATTR void VKAPI_CALL StubCmdSetLogicOpEXT(VkCommandBuffer commandBuffer, VkLogicOp logicOp) {  };
static VKAPI_ATTR void VKAPI_CALL StubCmdSetPrimitiveRestartEnableEXT(VkCommandBuffer commandBuffer, VkBool32 primitiveRestartEnable) {  };
static VKAPI_ATTR void                                    VKAPI_CALL StubCmdSetColorWriteEnableEXT(VkCommandBuffer       commandBuffer, uint32_t                                attachmentCount, const VkBool32*   pColorWriteEnables) {  };



const layer_data::unordered_map<std::string, std::string> api_extension_map {
    {"vkAcquireFullScreenExclusiveModeEXT", "VK_EXT_full_screen_exclusive"},
    {"vkAcquireNextImage2KHR", "VK_KHR_swapchain"},
    {"vkAcquireNextImageKHR", "VK_KHR_swapchain"},
    {"vkAcquirePerformanceConfigurationINTEL", "VK_INTEL_performance_query"},
    {"vkAcquireProfilingLockKHR", "VK_KHR_performance_query"},
    {"vkBindAccelerationStructureMemoryNV", "VK_NV_ray_tracing"},
    {"vkBindBufferMemory2", "VK_VERSION_1_1"},
    {"vkBindBufferMemory2KHR", "VK_KHR_bind_memory2"},
    {"vkBindImageMemory2", "VK_VERSION_1_1"},
    {"vkBindImageMemory2KHR", "VK_KHR_bind_memory2"},
    {"vkBindVideoSessionMemoryKHR", "VK_KHR_video_queue"},
    {"vkBuildAccelerationStructuresKHR", "VK_KHR_acceleration_structure"},
    {"vkCmdBeginConditionalRenderingEXT", "VK_EXT_conditional_rendering"},
    {"vkCmdBeginDebugUtilsLabelEXT", "VK_EXT_debug_utils"},
    {"vkCmdBeginQueryIndexedEXT", "VK_EXT_transform_feedback"},
    {"vkCmdBeginRenderPass2", "VK_VERSION_1_2"},
    {"vkCmdBeginRenderPass2KHR", "VK_KHR_create_renderpass2"},
    {"vkCmdBeginTransformFeedbackEXT", "VK_EXT_transform_feedback"},
    {"vkCmdBeginVideoCodingKHR", "VK_KHR_video_queue"},
    {"vkCmdBindInvocationMaskHUAWEI", "VK_HUAWEI_invocation_mask"},
    {"vkCmdBindPipelineShaderGroupNV", "VK_NV_device_generated_commands"},
    {"vkCmdBindShadingRateImageNV", "VK_NV_shading_rate_image"},
    {"vkCmdBindTransformFeedbackBuffersEXT", "VK_EXT_transform_feedback"},
    {"vkCmdBindVertexBuffers2EXT", "VK_EXT_extended_dynamic_state"},
    {"vkCmdBlitImage2KHR", "VK_KHR_copy_commands2"},
    {"vkCmdBuildAccelerationStructureNV", "VK_NV_ray_tracing"},
    {"vkCmdBuildAccelerationStructuresIndirectKHR", "VK_KHR_acceleration_structure"},
    {"vkCmdBuildAccelerationStructuresKHR", "VK_KHR_acceleration_structure"},
    {"vkCmdControlVideoCodingKHR", "VK_KHR_video_queue"},
    {"vkCmdCopyAccelerationStructureKHR", "VK_KHR_acceleration_structure"},
    {"vkCmdCopyAccelerationStructureNV", "VK_NV_ray_tracing"},
    {"vkCmdCopyAccelerationStructureToMemoryKHR", "VK_KHR_acceleration_structure"},
    {"vkCmdCopyBuffer2KHR", "VK_KHR_copy_commands2"},
    {"vkCmdCopyBufferToImage2KHR", "VK_KHR_copy_commands2"},
    {"vkCmdCopyImage2KHR", "VK_KHR_copy_commands2"},
    {"vkCmdCopyImageToBuffer2KHR", "VK_KHR_copy_commands2"},
    {"vkCmdCopyMemoryToAccelerationStructureKHR", "VK_KHR_acceleration_structure"},
    {"vkCmdCuLaunchKernelNVX", "VK_NVX_binary_import"},
    {"vkCmdDebugMarkerBeginEXT", "VK_EXT_debug_marker"},
    {"vkCmdDebugMarkerEndEXT", "VK_EXT_debug_marker"},
    {"vkCmdDebugMarkerInsertEXT", "VK_EXT_debug_marker"},
    {"vkCmdDecodeVideoKHR", "VK_KHR_video_decode_queue"},
    {"vkCmdDispatchBase", "VK_VERSION_1_1"},
    {"vkCmdDispatchBaseKHR", "VK_KHR_device_group"},
    {"vkCmdDrawIndexedIndirectCount", "VK_VERSION_1_2"},
    {"vkCmdDrawIndexedIndirectCountAMD", "VK_AMD_draw_indirect_count"},
    {"vkCmdDrawIndexedIndirectCountKHR", "VK_KHR_draw_indirect_count"},
    {"vkCmdDrawIndirectByteCountEXT", "VK_EXT_transform_feedback"},
    {"vkCmdDrawIndirectCount", "VK_VERSION_1_2"},
    {"vkCmdDrawIndirectCountAMD", "VK_AMD_draw_indirect_count"},
    {"vkCmdDrawIndirectCountKHR", "VK_KHR_draw_indirect_count"},
    {"vkCmdDrawMeshTasksIndirectCountNV", "VK_NV_mesh_shader"},
    {"vkCmdDrawMeshTasksIndirectNV", "VK_NV_mesh_shader"},
    {"vkCmdDrawMeshTasksNV", "VK_NV_mesh_shader"},
    {"vkCmdDrawMultiEXT", "VK_EXT_multi_draw"},
    {"vkCmdDrawMultiIndexedEXT", "VK_EXT_multi_draw"},
    {"vkCmdEncodeVideoKHR", "VK_KHR_video_encode_queue"},
    {"vkCmdEndConditionalRenderingEXT", "VK_EXT_conditional_rendering"},
    {"vkCmdEndDebugUtilsLabelEXT", "VK_EXT_debug_utils"},
    {"vkCmdEndQueryIndexedEXT", "VK_EXT_transform_feedback"},
    {"vkCmdEndRenderPass2", "VK_VERSION_1_2"},
    {"vkCmdEndRenderPass2KHR", "VK_KHR_create_renderpass2"},
    {"vkCmdEndTransformFeedbackEXT", "VK_EXT_transform_feedback"},
    {"vkCmdEndVideoCodingKHR", "VK_KHR_video_queue"},
    {"vkCmdExecuteGeneratedCommandsNV", "VK_NV_device_generated_commands"},
    {"vkCmdInsertDebugUtilsLabelEXT", "VK_EXT_debug_utils"},
    {"vkCmdNextSubpass2", "VK_VERSION_1_2"},
    {"vkCmdNextSubpass2KHR", "VK_KHR_create_renderpass2"},
    {"vkCmdPipelineBarrier2KHR", "VK_KHR_synchronization2"},
    {"vkCmdPreprocessGeneratedCommandsNV", "VK_NV_device_generated_commands"},
    {"vkCmdPushDescriptorSetKHR", "VK_KHR_push_descriptor"},
    {"vkCmdPushDescriptorSetWithTemplateKHR", "VK_KHR_push_descriptor"},
    {"vkCmdRefreshObjectsKHR", "VK_KHR_object_refresh"},
    {"vkCmdResetEvent2KHR", "VK_KHR_synchronization2"},
    {"vkCmdResolveImage2KHR", "VK_KHR_copy_commands2"},
    {"vkCmdSetCheckpointNV", "VK_NV_device_diagnostic_checkpoints"},
    {"vkCmdSetCoarseSampleOrderNV", "VK_NV_shading_rate_image"},
    {"vkCmdSetColorWriteEnableEXT", "VK_EXT_color_write_enable"},
    {"vkCmdSetCullModeEXT", "VK_EXT_extended_dynamic_state"},
    {"vkCmdSetDepthBiasEnableEXT", "VK_EXT_extended_dynamic_state2"},
    {"vkCmdSetDepthBoundsTestEnableEXT", "VK_EXT_extended_dynamic_state"},
    {"vkCmdSetDepthCompareOpEXT", "VK_EXT_extended_dynamic_state"},
    {"vkCmdSetDepthTestEnableEXT", "VK_EXT_extended_dynamic_state"},
    {"vkCmdSetDepthWriteEnableEXT", "VK_EXT_extended_dynamic_state"},
    {"vkCmdSetDeviceMask", "VK_VERSION_1_1"},
    {"vkCmdSetDeviceMaskKHR", "VK_KHR_device_group"},
    {"vkCmdSetDiscardRectangleEXT", "VK_EXT_discard_rectangles"},
    {"vkCmdSetEvent2KHR", "VK_KHR_synchronization2"},
    {"vkCmdSetExclusiveScissorNV", "VK_NV_scissor_exclusive"},
    {"vkCmdSetFragmentShadingRateEnumNV", "VK_NV_fragment_shading_rate_enums"},
    {"vkCmdSetFragmentShadingRateKHR", "VK_KHR_fragment_shading_rate"},
    {"vkCmdSetFrontFaceEXT", "VK_EXT_extended_dynamic_state"},
    {"vkCmdSetLineStippleEXT", "VK_EXT_line_rasterization"},
    {"vkCmdSetLogicOpEXT", "VK_EXT_extended_dynamic_state2"},
    {"vkCmdSetPatchControlPointsEXT", "VK_EXT_extended_dynamic_state2"},
    {"vkCmdSetPerformanceMarkerINTEL", "VK_INTEL_performance_query"},
    {"vkCmdSetPerformanceOverrideINTEL", "VK_INTEL_performance_query"},
    {"vkCmdSetPerformanceStreamMarkerINTEL", "VK_INTEL_performance_query"},
    {"vkCmdSetPrimitiveRestartEnableEXT", "VK_EXT_extended_dynamic_state2"},
    {"vkCmdSetPrimitiveTopologyEXT", "VK_EXT_extended_dynamic_state"},
    {"vkCmdSetRasterizerDiscardEnableEXT", "VK_EXT_extended_dynamic_state2"},
    {"vkCmdSetRayTracingPipelineStackSizeKHR", "VK_KHR_ray_tracing_pipeline"},
    {"vkCmdSetSampleLocationsEXT", "VK_EXT_sample_locations"},
    {"vkCmdSetScissorWithCountEXT", "VK_EXT_extended_dynamic_state"},
    {"vkCmdSetStencilOpEXT", "VK_EXT_extended_dynamic_state"},
    {"vkCmdSetStencilTestEnableEXT", "VK_EXT_extended_dynamic_state"},
    {"vkCmdSetVertexInputEXT", "VK_EXT_vertex_input_dynamic_state"},
    {"vkCmdSetViewportShadingRatePaletteNV", "VK_NV_shading_rate_image"},
    {"vkCmdSetViewportWScalingNV", "VK_NV_clip_space_w_scaling"},
    {"vkCmdSetViewportWithCountEXT", "VK_EXT_extended_dynamic_state"},
    {"vkCmdSubpassShadingHUAWEI", "VK_HUAWEI_subpass_shading"},
    {"vkCmdTraceRaysIndirectKHR", "VK_KHR_ray_tracing_pipeline"},
    {"vkCmdTraceRaysKHR", "VK_KHR_ray_tracing_pipeline"},
    {"vkCmdTraceRaysNV", "VK_NV_ray_tracing"},
    {"vkCmdWaitEvents2KHR", "VK_KHR_synchronization2"},
    {"vkCmdWriteAccelerationStructuresPropertiesKHR", "VK_KHR_acceleration_structure"},
    {"vkCmdWriteAccelerationStructuresPropertiesNV", "VK_NV_ray_tracing"},
    {"vkCmdWriteBufferMarker2AMD", "VK_KHR_synchronization2"},
    {"vkCmdWriteBufferMarkerAMD", "VK_AMD_buffer_marker"},
    {"vkCmdWriteTimestamp2KHR", "VK_KHR_synchronization2"},
    {"vkCompileDeferredNV", "VK_NV_ray_tracing"},
    {"vkCopyAccelerationStructureKHR", "VK_KHR_acceleration_structure"},
    {"vkCopyAccelerationStructureToMemoryKHR", "VK_KHR_acceleration_structure"},
    {"vkCopyMemoryToAccelerationStructureKHR", "VK_KHR_acceleration_structure"},
    {"vkCreateAccelerationStructureKHR", "VK_KHR_acceleration_structure"},
    {"vkCreateAccelerationStructureNV", "VK_NV_ray_tracing"},
    {"vkCreateCuFunctionNVX", "VK_NVX_binary_import"},
    {"vkCreateCuModuleNVX", "VK_NVX_binary_import"},
    {"vkCreateDeferredOperationKHR", "VK_KHR_deferred_host_operations"},
    {"vkCreateDescriptorUpdateTemplate", "VK_VERSION_1_1"},
    {"vkCreateDescriptorUpdateTemplateKHR", "VK_KHR_descriptor_update_template"},
    {"vkCreateIndirectCommandsLayoutNV", "VK_NV_device_generated_commands"},
    {"vkCreatePrivateDataSlotEXT", "VK_EXT_private_data"},
    {"vkCreateRayTracingPipelinesKHR", "VK_KHR_ray_tracing_pipeline"},
    {"vkCreateRayTracingPipelinesNV", "VK_NV_ray_tracing"},
    {"vkCreateRenderPass2", "VK_VERSION_1_2"},
    {"vkCreateRenderPass2KHR", "VK_KHR_create_renderpass2"},
    {"vkCreateSamplerYcbcrConversion", "VK_VERSION_1_1"},
    {"vkCreateSamplerYcbcrConversionKHR", "VK_KHR_sampler_ycbcr_conversion"},
    {"vkCreateSharedSwapchainsKHR", "VK_KHR_display_swapchain"},
    {"vkCreateSwapchainKHR", "VK_KHR_swapchain"},
    {"vkCreateValidationCacheEXT", "VK_EXT_validation_cache"},
    {"vkCreateVideoSessionKHR", "VK_KHR_video_queue"},
    {"vkCreateVideoSessionParametersKHR", "VK_KHR_video_queue"},
    {"vkDebugMarkerSetObjectNameEXT", "VK_EXT_debug_marker"},
    {"vkDebugMarkerSetObjectTagEXT", "VK_EXT_debug_marker"},
    {"vkDeferredOperationJoinKHR", "VK_KHR_deferred_host_operations"},
    {"vkDestroyAccelerationStructureKHR", "VK_KHR_acceleration_structure"},
    {"vkDestroyAccelerationStructureNV", "VK_NV_ray_tracing"},
    {"vkDestroyCuFunctionNVX", "VK_NVX_binary_import"},
    {"vkDestroyCuModuleNVX", "VK_NVX_binary_import"},
    {"vkDestroyDeferredOperationKHR", "VK_KHR_deferred_host_operations"},
    {"vkDestroyDescriptorUpdateTemplate", "VK_VERSION_1_1"},
    {"vkDestroyDescriptorUpdateTemplateKHR", "VK_KHR_descriptor_update_template"},
    {"vkDestroyIndirectCommandsLayoutNV", "VK_NV_device_generated_commands"},
    {"vkDestroyPrivateDataSlotEXT", "VK_EXT_private_data"},
    {"vkDestroySamplerYcbcrConversion", "VK_VERSION_1_1"},
    {"vkDestroySamplerYcbcrConversionKHR", "VK_KHR_sampler_ycbcr_conversion"},
    {"vkDestroySwapchainKHR", "VK_KHR_swapchain"},
    {"vkDestroyValidationCacheEXT", "VK_EXT_validation_cache"},
    {"vkDestroyVideoSessionKHR", "VK_KHR_video_queue"},
    {"vkDestroyVideoSessionParametersKHR", "VK_KHR_video_queue"},
    {"vkDisplayPowerControlEXT", "VK_EXT_display_control"},
    {"vkGetAccelerationStructureBuildSizesKHR", "VK_KHR_acceleration_structure"},
    {"vkGetAccelerationStructureDeviceAddressKHR", "VK_KHR_acceleration_structure"},
    {"vkGetAccelerationStructureHandleNV", "VK_NV_ray_tracing"},
    {"vkGetAccelerationStructureMemoryRequirementsNV", "VK_NV_ray_tracing"},
    {"vkGetAndroidHardwareBufferPropertiesANDROID", "VK_ANDROID_external_memory_android_hardware_buffer"},
    {"vkGetBufferDeviceAddress", "VK_VERSION_1_2"},
    {"vkGetBufferDeviceAddressEXT", "VK_EXT_buffer_device_address"},
    {"vkGetBufferDeviceAddressKHR", "VK_KHR_buffer_device_address"},
    {"vkGetBufferMemoryRequirements2", "VK_VERSION_1_1"},
    {"vkGetBufferMemoryRequirements2KHR", "VK_KHR_get_memory_requirements2"},
    {"vkGetBufferOpaqueCaptureAddress", "VK_VERSION_1_2"},
    {"vkGetBufferOpaqueCaptureAddressKHR", "VK_KHR_buffer_device_address"},
    {"vkGetCalibratedTimestampsEXT", "VK_EXT_calibrated_timestamps"},
    {"vkGetCommandPoolMemoryConsumption", "VKSC_VERSION_1_0"},
    {"vkGetDeferredOperationMaxConcurrencyKHR", "VK_KHR_deferred_host_operations"},
    {"vkGetDeferredOperationResultKHR", "VK_KHR_deferred_host_operations"},
    {"vkGetDescriptorSetLayoutSupport", "VK_VERSION_1_1"},
    {"vkGetDescriptorSetLayoutSupportKHR", "VK_KHR_maintenance3"},
    {"vkGetDeviceAccelerationStructureCompatibilityKHR", "VK_KHR_acceleration_structure"},
    {"vkGetDeviceGroupPeerMemoryFeatures", "VK_VERSION_1_1"},
    {"vkGetDeviceGroupPeerMemoryFeaturesKHR", "VK_KHR_device_group"},
    {"vkGetDeviceGroupPresentCapabilitiesKHR", "VK_KHR_swapchain"},
    {"vkGetDeviceGroupSurfacePresentModes2EXT", "VK_EXT_full_screen_exclusive"},
    {"vkGetDeviceGroupSurfacePresentModesKHR", "VK_KHR_swapchain"},
    {"vkGetDeviceMemoryOpaqueCaptureAddress", "VK_VERSION_1_2"},
    {"vkGetDeviceMemoryOpaqueCaptureAddressKHR", "VK_KHR_buffer_device_address"},
    {"vkGetDeviceQueue2", "VK_VERSION_1_1"},
    {"vkGetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI", "VK_HUAWEI_subpass_shading"},
    {"vkGetFaultData", "VKSC_VERSION_1_0"},
    {"vkGetFenceFdKHR", "VK_KHR_external_fence_fd"},
    {"vkGetFenceWin32HandleKHR", "VK_KHR_external_fence_win32"},
    {"vkGetGeneratedCommandsMemoryRequirementsNV", "VK_NV_device_generated_commands"},
    {"vkGetImageDrmFormatModifierPropertiesEXT", "VK_EXT_image_drm_format_modifier"},
    {"vkGetImageMemoryRequirements2", "VK_VERSION_1_1"},
    {"vkGetImageMemoryRequirements2KHR", "VK_KHR_get_memory_requirements2"},
    {"vkGetImageSparseMemoryRequirements2", "VK_VERSION_1_1"},
    {"vkGetImageSparseMemoryRequirements2KHR", "VK_KHR_get_memory_requirements2"},
    {"vkGetImageViewAddressNVX", "VK_NVX_image_view_handle"},
    {"vkGetImageViewHandleNVX", "VK_NVX_image_view_handle"},
    {"vkGetMemoryAndroidHardwareBufferANDROID", "VK_ANDROID_external_memory_android_hardware_buffer"},
    {"vkGetMemoryFdKHR", "VK_KHR_external_memory_fd"},
    {"vkGetMemoryFdPropertiesKHR", "VK_KHR_external_memory_fd"},
    {"vkGetMemoryHostPointerPropertiesEXT", "VK_EXT_external_memory_host"},
    {"vkGetMemoryRemoteAddressNV", "VK_NV_external_memory_rdma"},
    {"vkGetMemoryWin32HandleKHR", "VK_KHR_external_memory_win32"},
    {"vkGetMemoryWin32HandleNV", "VK_NV_external_memory_win32"},
    {"vkGetMemoryWin32HandlePropertiesKHR", "VK_KHR_external_memory_win32"},
    {"vkGetMemoryZirconHandleFUCHSIA", "VK_FUCHSIA_external_memory"},
    {"vkGetMemoryZirconHandlePropertiesFUCHSIA", "VK_FUCHSIA_external_memory"},
    {"vkGetPastPresentationTimingGOOGLE", "VK_GOOGLE_display_timing"},
    {"vkGetPerformanceParameterINTEL", "VK_INTEL_performance_query"},
    {"vkGetPipelineExecutableInternalRepresentationsKHR", "VK_KHR_pipeline_executable_properties"},
    {"vkGetPipelineExecutablePropertiesKHR", "VK_KHR_pipeline_executable_properties"},
    {"vkGetPipelineExecutableStatisticsKHR", "VK_KHR_pipeline_executable_properties"},
    {"vkGetPrivateDataEXT", "VK_EXT_private_data"},
    {"vkGetQueueCheckpointData2NV", "VK_KHR_synchronization2"},
    {"vkGetQueueCheckpointDataNV", "VK_NV_device_diagnostic_checkpoints"},
    {"vkGetRayTracingCaptureReplayShaderGroupHandlesKHR", "VK_KHR_ray_tracing_pipeline"},
    {"vkGetRayTracingShaderGroupHandlesKHR", "VK_KHR_ray_tracing_pipeline"},
    {"vkGetRayTracingShaderGroupHandlesNV", "VK_NV_ray_tracing"},
    {"vkGetRayTracingShaderGroupStackSizeKHR", "VK_KHR_ray_tracing_pipeline"},
    {"vkGetRefreshCycleDurationGOOGLE", "VK_GOOGLE_display_timing"},
    {"vkGetSemaphoreCounterValue", "VK_VERSION_1_2"},
    {"vkGetSemaphoreCounterValueKHR", "VK_KHR_timeline_semaphore"},
    {"vkGetSemaphoreFdKHR", "VK_KHR_external_semaphore_fd"},
    {"vkGetSemaphoreWin32HandleKHR", "VK_KHR_external_semaphore_win32"},
    {"vkGetSemaphoreZirconHandleFUCHSIA", "VK_FUCHSIA_external_semaphore"},
    {"vkGetShaderInfoAMD", "VK_AMD_shader_info"},
    {"vkGetSwapchainCounterEXT", "VK_EXT_display_control"},
    {"vkGetSwapchainImagesKHR", "VK_KHR_swapchain"},
    {"vkGetSwapchainStatusKHR", "VK_KHR_shared_presentable_image"},
    {"vkGetValidationCacheDataEXT", "VK_EXT_validation_cache"},
    {"vkGetVideoSessionMemoryRequirementsKHR", "VK_KHR_video_queue"},
    {"vkImportFenceFdKHR", "VK_KHR_external_fence_fd"},
    {"vkImportFenceWin32HandleKHR", "VK_KHR_external_fence_win32"},
    {"vkImportSemaphoreFdKHR", "VK_KHR_external_semaphore_fd"},
    {"vkImportSemaphoreWin32HandleKHR", "VK_KHR_external_semaphore_win32"},
    {"vkImportSemaphoreZirconHandleFUCHSIA", "VK_FUCHSIA_external_semaphore"},
    {"vkInitializePerformanceApiINTEL", "VK_INTEL_performance_query"},
    {"vkMergeValidationCachesEXT", "VK_EXT_validation_cache"},
    {"vkQueueBeginDebugUtilsLabelEXT", "VK_EXT_debug_utils"},
    {"vkQueueEndDebugUtilsLabelEXT", "VK_EXT_debug_utils"},
    {"vkQueueInsertDebugUtilsLabelEXT", "VK_EXT_debug_utils"},
    {"vkQueuePresentKHR", "VK_KHR_swapchain"},
    {"vkQueueSetPerformanceConfigurationINTEL", "VK_INTEL_performance_query"},
    {"vkQueueSubmit2KHR", "VK_KHR_synchronization2"},
    {"vkRegisterDeviceEventEXT", "VK_EXT_display_control"},
    {"vkRegisterDisplayEventEXT", "VK_EXT_display_control"},
    {"vkReleaseFullScreenExclusiveModeEXT", "VK_EXT_full_screen_exclusive"},
    {"vkReleasePerformanceConfigurationINTEL", "VK_INTEL_performance_query"},
    {"vkReleaseProfilingLockKHR", "VK_KHR_performance_query"},
    {"vkResetQueryPool", "VK_VERSION_1_2"},
    {"vkResetQueryPoolEXT", "VK_EXT_host_query_reset"},
    {"vkSetDebugUtilsObjectNameEXT", "VK_EXT_debug_utils"},
    {"vkSetDebugUtilsObjectTagEXT", "VK_EXT_debug_utils"},
    {"vkSetHdrMetadataEXT", "VK_EXT_hdr_metadata"},
    {"vkSetLocalDimmingAMD", "VK_AMD_display_native_hdr"},
    {"vkSetPrivateDataEXT", "VK_EXT_private_data"},
    {"vkSignalSemaphore", "VK_VERSION_1_2"},
    {"vkSignalSemaphoreKHR", "VK_KHR_timeline_semaphore"},
    {"vkTrimCommandPool", "VK_VERSION_1_1"},
    {"vkTrimCommandPoolKHR", "VK_KHR_maintenance1"},
    {"vkUninitializePerformanceApiINTEL", "VK_INTEL_performance_query"},
    {"vkUpdateDescriptorSetWithTemplate", "VK_VERSION_1_1"},
    {"vkUpdateDescriptorSetWithTemplateKHR", "VK_KHR_descriptor_update_template"},
    {"vkUpdateVideoSessionParametersKHR", "VK_KHR_video_queue"},
    {"vkWaitForPresentKHR", "VK_KHR_present_wait"},
    {"vkWaitSemaphores", "VK_VERSION_1_2"},
    {"vkWaitSemaphoresKHR", "VK_KHR_timeline_semaphore"},
    {"vkWriteAccelerationStructuresPropertiesKHR", "VK_KHR_acceleration_structure"},
};

// Using the above code-generated map of APINames-to-parent extension names, this function will:
//   o  Determine if the API has an associated extension
//   o  If it does, determine if that extension name is present in the passed-in set of enabled_ext_names 
//   If the APIname has no parent extension, OR its parent extension name is IN the set, return TRUE, else FALSE
static inline bool ApiParentExtensionEnabled(const std::string api_name, const DeviceExtensions *device_extension_info) {
    auto has_ext = api_extension_map.find(api_name);
    // Is this API part of an extension or feature group?
    if (has_ext != api_extension_map.end()) {
        // Was the extension for this API enabled in the CreateDevice call?
        auto info = device_extension_info->get_info(has_ext->second.c_str());
        if ((!info.state) || (device_extension_info->*(info.state) != kEnabledByCreateinfo)) {
            return false;
        }
    }
    return true;
}



static inline void layer_init_device_dispatch_table(VkDevice device, VkLayerDispatchTable *table, PFN_vkGetDeviceProcAddr gpa) {
    memset(table, 0, sizeof(*table));
    // Device function pointers
    table->GetDeviceProcAddr = gpa;
    table->DestroyDevice = (PFN_vkDestroyDevice) gpa(device, "vkDestroyDevice");
    table->GetDeviceQueue = (PFN_vkGetDeviceQueue) gpa(device, "vkGetDeviceQueue");
    table->QueueSubmit = (PFN_vkQueueSubmit) gpa(device, "vkQueueSubmit");
    table->QueueWaitIdle = (PFN_vkQueueWaitIdle) gpa(device, "vkQueueWaitIdle");
    table->DeviceWaitIdle = (PFN_vkDeviceWaitIdle) gpa(device, "vkDeviceWaitIdle");
    table->AllocateMemory = (PFN_vkAllocateMemory) gpa(device, "vkAllocateMemory");
    table->MapMemory = (PFN_vkMapMemory) gpa(device, "vkMapMemory");
    table->UnmapMemory = (PFN_vkUnmapMemory) gpa(device, "vkUnmapMemory");
    table->FlushMappedMemoryRanges = (PFN_vkFlushMappedMemoryRanges) gpa(device, "vkFlushMappedMemoryRanges");
    table->InvalidateMappedMemoryRanges = (PFN_vkInvalidateMappedMemoryRanges) gpa(device, "vkInvalidateMappedMemoryRanges");
    table->GetDeviceMemoryCommitment = (PFN_vkGetDeviceMemoryCommitment) gpa(device, "vkGetDeviceMemoryCommitment");
    table->BindBufferMemory = (PFN_vkBindBufferMemory) gpa(device, "vkBindBufferMemory");
    table->BindImageMemory = (PFN_vkBindImageMemory) gpa(device, "vkBindImageMemory");
    table->GetBufferMemoryRequirements = (PFN_vkGetBufferMemoryRequirements) gpa(device, "vkGetBufferMemoryRequirements");
    table->GetImageMemoryRequirements = (PFN_vkGetImageMemoryRequirements) gpa(device, "vkGetImageMemoryRequirements");
    table->CreateFence = (PFN_vkCreateFence) gpa(device, "vkCreateFence");
    table->DestroyFence = (PFN_vkDestroyFence) gpa(device, "vkDestroyFence");
    table->ResetFences = (PFN_vkResetFences) gpa(device, "vkResetFences");
    table->GetFenceStatus = (PFN_vkGetFenceStatus) gpa(device, "vkGetFenceStatus");
    table->WaitForFences = (PFN_vkWaitForFences) gpa(device, "vkWaitForFences");
    table->CreateSemaphore = (PFN_vkCreateSemaphore) gpa(device, "vkCreateSemaphore");
    table->DestroySemaphore = (PFN_vkDestroySemaphore) gpa(device, "vkDestroySemaphore");
    table->CreateEvent = (PFN_vkCreateEvent) gpa(device, "vkCreateEvent");
    table->DestroyEvent = (PFN_vkDestroyEvent) gpa(device, "vkDestroyEvent");
    table->GetEventStatus = (PFN_vkGetEventStatus) gpa(device, "vkGetEventStatus");
    table->SetEvent = (PFN_vkSetEvent) gpa(device, "vkSetEvent");
    table->ResetEvent = (PFN_vkResetEvent) gpa(device, "vkResetEvent");
    table->CreateQueryPool = (PFN_vkCreateQueryPool) gpa(device, "vkCreateQueryPool");
    table->GetQueryPoolResults = (PFN_vkGetQueryPoolResults) gpa(device, "vkGetQueryPoolResults");
    table->CreateBuffer = (PFN_vkCreateBuffer) gpa(device, "vkCreateBuffer");
    table->DestroyBuffer = (PFN_vkDestroyBuffer) gpa(device, "vkDestroyBuffer");
    table->CreateBufferView = (PFN_vkCreateBufferView) gpa(device, "vkCreateBufferView");
    table->DestroyBufferView = (PFN_vkDestroyBufferView) gpa(device, "vkDestroyBufferView");
    table->CreateImage = (PFN_vkCreateImage) gpa(device, "vkCreateImage");
    table->DestroyImage = (PFN_vkDestroyImage) gpa(device, "vkDestroyImage");
    table->GetImageSubresourceLayout = (PFN_vkGetImageSubresourceLayout) gpa(device, "vkGetImageSubresourceLayout");
    table->CreateImageView = (PFN_vkCreateImageView) gpa(device, "vkCreateImageView");
    table->DestroyImageView = (PFN_vkDestroyImageView) gpa(device, "vkDestroyImageView");
    table->CreatePipelineCache = (PFN_vkCreatePipelineCache) gpa(device, "vkCreatePipelineCache");
    table->DestroyPipelineCache = (PFN_vkDestroyPipelineCache) gpa(device, "vkDestroyPipelineCache");
    table->CreateGraphicsPipelines = (PFN_vkCreateGraphicsPipelines) gpa(device, "vkCreateGraphicsPipelines");
    table->CreateComputePipelines = (PFN_vkCreateComputePipelines) gpa(device, "vkCreateComputePipelines");
    table->DestroyPipeline = (PFN_vkDestroyPipeline) gpa(device, "vkDestroyPipeline");
    table->CreatePipelineLayout = (PFN_vkCreatePipelineLayout) gpa(device, "vkCreatePipelineLayout");
    table->DestroyPipelineLayout = (PFN_vkDestroyPipelineLayout) gpa(device, "vkDestroyPipelineLayout");
    table->CreateSampler = (PFN_vkCreateSampler) gpa(device, "vkCreateSampler");
    table->DestroySampler = (PFN_vkDestroySampler) gpa(device, "vkDestroySampler");
    table->CreateDescriptorSetLayout = (PFN_vkCreateDescriptorSetLayout) gpa(device, "vkCreateDescriptorSetLayout");
    table->DestroyDescriptorSetLayout = (PFN_vkDestroyDescriptorSetLayout) gpa(device, "vkDestroyDescriptorSetLayout");
    table->CreateDescriptorPool = (PFN_vkCreateDescriptorPool) gpa(device, "vkCreateDescriptorPool");
    table->ResetDescriptorPool = (PFN_vkResetDescriptorPool) gpa(device, "vkResetDescriptorPool");
    table->AllocateDescriptorSets = (PFN_vkAllocateDescriptorSets) gpa(device, "vkAllocateDescriptorSets");
    table->FreeDescriptorSets = (PFN_vkFreeDescriptorSets) gpa(device, "vkFreeDescriptorSets");
    table->UpdateDescriptorSets = (PFN_vkUpdateDescriptorSets) gpa(device, "vkUpdateDescriptorSets");
    table->CreateFramebuffer = (PFN_vkCreateFramebuffer) gpa(device, "vkCreateFramebuffer");
    table->DestroyFramebuffer = (PFN_vkDestroyFramebuffer) gpa(device, "vkDestroyFramebuffer");
    table->CreateRenderPass = (PFN_vkCreateRenderPass) gpa(device, "vkCreateRenderPass");
    table->DestroyRenderPass = (PFN_vkDestroyRenderPass) gpa(device, "vkDestroyRenderPass");
    table->GetRenderAreaGranularity = (PFN_vkGetRenderAreaGranularity) gpa(device, "vkGetRenderAreaGranularity");
    table->CreateCommandPool = (PFN_vkCreateCommandPool) gpa(device, "vkCreateCommandPool");
    table->ResetCommandPool = (PFN_vkResetCommandPool) gpa(device, "vkResetCommandPool");
    table->AllocateCommandBuffers = (PFN_vkAllocateCommandBuffers) gpa(device, "vkAllocateCommandBuffers");
    table->FreeCommandBuffers = (PFN_vkFreeCommandBuffers) gpa(device, "vkFreeCommandBuffers");
    table->BeginCommandBuffer = (PFN_vkBeginCommandBuffer) gpa(device, "vkBeginCommandBuffer");
    table->EndCommandBuffer = (PFN_vkEndCommandBuffer) gpa(device, "vkEndCommandBuffer");
    table->ResetCommandBuffer = (PFN_vkResetCommandBuffer) gpa(device, "vkResetCommandBuffer");
    table->CmdBindPipeline = (PFN_vkCmdBindPipeline) gpa(device, "vkCmdBindPipeline");
    table->CmdSetViewport = (PFN_vkCmdSetViewport) gpa(device, "vkCmdSetViewport");
    table->CmdSetScissor = (PFN_vkCmdSetScissor) gpa(device, "vkCmdSetScissor");
    table->CmdSetLineWidth = (PFN_vkCmdSetLineWidth) gpa(device, "vkCmdSetLineWidth");
    table->CmdSetDepthBias = (PFN_vkCmdSetDepthBias) gpa(device, "vkCmdSetDepthBias");
    table->CmdSetBlendConstants = (PFN_vkCmdSetBlendConstants) gpa(device, "vkCmdSetBlendConstants");
    table->CmdSetDepthBounds = (PFN_vkCmdSetDepthBounds) gpa(device, "vkCmdSetDepthBounds");
    table->CmdSetStencilCompareMask = (PFN_vkCmdSetStencilCompareMask) gpa(device, "vkCmdSetStencilCompareMask");
    table->CmdSetStencilWriteMask = (PFN_vkCmdSetStencilWriteMask) gpa(device, "vkCmdSetStencilWriteMask");
    table->CmdSetStencilReference = (PFN_vkCmdSetStencilReference) gpa(device, "vkCmdSetStencilReference");
    table->CmdBindDescriptorSets = (PFN_vkCmdBindDescriptorSets) gpa(device, "vkCmdBindDescriptorSets");
    table->CmdBindIndexBuffer = (PFN_vkCmdBindIndexBuffer) gpa(device, "vkCmdBindIndexBuffer");
    table->CmdBindVertexBuffers = (PFN_vkCmdBindVertexBuffers) gpa(device, "vkCmdBindVertexBuffers");
    table->CmdDraw = (PFN_vkCmdDraw) gpa(device, "vkCmdDraw");
    table->CmdDrawIndexed = (PFN_vkCmdDrawIndexed) gpa(device, "vkCmdDrawIndexed");
    table->CmdDrawIndirect = (PFN_vkCmdDrawIndirect) gpa(device, "vkCmdDrawIndirect");
    table->CmdDrawIndexedIndirect = (PFN_vkCmdDrawIndexedIndirect) gpa(device, "vkCmdDrawIndexedIndirect");
    table->CmdDispatch = (PFN_vkCmdDispatch) gpa(device, "vkCmdDispatch");
    table->CmdDispatchIndirect = (PFN_vkCmdDispatchIndirect) gpa(device, "vkCmdDispatchIndirect");
    table->CmdCopyBuffer = (PFN_vkCmdCopyBuffer) gpa(device, "vkCmdCopyBuffer");
    table->CmdCopyImage = (PFN_vkCmdCopyImage) gpa(device, "vkCmdCopyImage");
    table->CmdBlitImage = (PFN_vkCmdBlitImage) gpa(device, "vkCmdBlitImage");
    table->CmdCopyBufferToImage = (PFN_vkCmdCopyBufferToImage) gpa(device, "vkCmdCopyBufferToImage");
    table->CmdCopyImageToBuffer = (PFN_vkCmdCopyImageToBuffer) gpa(device, "vkCmdCopyImageToBuffer");
    table->CmdUpdateBuffer = (PFN_vkCmdUpdateBuffer) gpa(device, "vkCmdUpdateBuffer");
    table->CmdFillBuffer = (PFN_vkCmdFillBuffer) gpa(device, "vkCmdFillBuffer");
    table->CmdClearColorImage = (PFN_vkCmdClearColorImage) gpa(device, "vkCmdClearColorImage");
    table->CmdClearDepthStencilImage = (PFN_vkCmdClearDepthStencilImage) gpa(device, "vkCmdClearDepthStencilImage");
    table->CmdClearAttachments = (PFN_vkCmdClearAttachments) gpa(device, "vkCmdClearAttachments");
    table->CmdResolveImage = (PFN_vkCmdResolveImage) gpa(device, "vkCmdResolveImage");
    table->CmdSetEvent = (PFN_vkCmdSetEvent) gpa(device, "vkCmdSetEvent");
    table->CmdResetEvent = (PFN_vkCmdResetEvent) gpa(device, "vkCmdResetEvent");
    table->CmdWaitEvents = (PFN_vkCmdWaitEvents) gpa(device, "vkCmdWaitEvents");
    table->CmdPipelineBarrier = (PFN_vkCmdPipelineBarrier) gpa(device, "vkCmdPipelineBarrier");
    table->CmdBeginQuery = (PFN_vkCmdBeginQuery) gpa(device, "vkCmdBeginQuery");
    table->CmdEndQuery = (PFN_vkCmdEndQuery) gpa(device, "vkCmdEndQuery");
    table->CmdResetQueryPool = (PFN_vkCmdResetQueryPool) gpa(device, "vkCmdResetQueryPool");
    table->CmdWriteTimestamp = (PFN_vkCmdWriteTimestamp) gpa(device, "vkCmdWriteTimestamp");
    table->CmdCopyQueryPoolResults = (PFN_vkCmdCopyQueryPoolResults) gpa(device, "vkCmdCopyQueryPoolResults");
    table->CmdPushConstants = (PFN_vkCmdPushConstants) gpa(device, "vkCmdPushConstants");
    table->CmdBeginRenderPass = (PFN_vkCmdBeginRenderPass) gpa(device, "vkCmdBeginRenderPass");
    table->CmdNextSubpass = (PFN_vkCmdNextSubpass) gpa(device, "vkCmdNextSubpass");
    table->CmdEndRenderPass = (PFN_vkCmdEndRenderPass) gpa(device, "vkCmdEndRenderPass");
    table->CmdExecuteCommands = (PFN_vkCmdExecuteCommands) gpa(device, "vkCmdExecuteCommands");
    table->BindBufferMemory2 = (PFN_vkBindBufferMemory2) gpa(device, "vkBindBufferMemory2");
    if (table->BindBufferMemory2 == nullptr) { table->BindBufferMemory2 = (PFN_vkBindBufferMemory2)StubBindBufferMemory2; }
    table->BindImageMemory2 = (PFN_vkBindImageMemory2) gpa(device, "vkBindImageMemory2");
    if (table->BindImageMemory2 == nullptr) { table->BindImageMemory2 = (PFN_vkBindImageMemory2)StubBindImageMemory2; }
    table->GetDeviceGroupPeerMemoryFeatures = (PFN_vkGetDeviceGroupPeerMemoryFeatures) gpa(device, "vkGetDeviceGroupPeerMemoryFeatures");
    if (table->GetDeviceGroupPeerMemoryFeatures == nullptr) { table->GetDeviceGroupPeerMemoryFeatures = (PFN_vkGetDeviceGroupPeerMemoryFeatures)StubGetDeviceGroupPeerMemoryFeatures; }
    table->CmdSetDeviceMask = (PFN_vkCmdSetDeviceMask) gpa(device, "vkCmdSetDeviceMask");
    if (table->CmdSetDeviceMask == nullptr) { table->CmdSetDeviceMask = (PFN_vkCmdSetDeviceMask)StubCmdSetDeviceMask; }
    table->CmdDispatchBase = (PFN_vkCmdDispatchBase) gpa(device, "vkCmdDispatchBase");
    if (table->CmdDispatchBase == nullptr) { table->CmdDispatchBase = (PFN_vkCmdDispatchBase)StubCmdDispatchBase; }
    table->GetImageMemoryRequirements2 = (PFN_vkGetImageMemoryRequirements2) gpa(device, "vkGetImageMemoryRequirements2");
    if (table->GetImageMemoryRequirements2 == nullptr) { table->GetImageMemoryRequirements2 = (PFN_vkGetImageMemoryRequirements2)StubGetImageMemoryRequirements2; }
    table->GetBufferMemoryRequirements2 = (PFN_vkGetBufferMemoryRequirements2) gpa(device, "vkGetBufferMemoryRequirements2");
    if (table->GetBufferMemoryRequirements2 == nullptr) { table->GetBufferMemoryRequirements2 = (PFN_vkGetBufferMemoryRequirements2)StubGetBufferMemoryRequirements2; }
    table->GetDeviceQueue2 = (PFN_vkGetDeviceQueue2) gpa(device, "vkGetDeviceQueue2");
    if (table->GetDeviceQueue2 == nullptr) { table->GetDeviceQueue2 = (PFN_vkGetDeviceQueue2)StubGetDeviceQueue2; }
    table->CreateSamplerYcbcrConversion = (PFN_vkCreateSamplerYcbcrConversion) gpa(device, "vkCreateSamplerYcbcrConversion");
    if (table->CreateSamplerYcbcrConversion == nullptr) { table->CreateSamplerYcbcrConversion = (PFN_vkCreateSamplerYcbcrConversion)StubCreateSamplerYcbcrConversion; }
    table->DestroySamplerYcbcrConversion = (PFN_vkDestroySamplerYcbcrConversion) gpa(device, "vkDestroySamplerYcbcrConversion");
    if (table->DestroySamplerYcbcrConversion == nullptr) { table->DestroySamplerYcbcrConversion = (PFN_vkDestroySamplerYcbcrConversion)StubDestroySamplerYcbcrConversion; }
    table->GetDescriptorSetLayoutSupport = (PFN_vkGetDescriptorSetLayoutSupport) gpa(device, "vkGetDescriptorSetLayoutSupport");
    if (table->GetDescriptorSetLayoutSupport == nullptr) { table->GetDescriptorSetLayoutSupport = (PFN_vkGetDescriptorSetLayoutSupport)StubGetDescriptorSetLayoutSupport; }
    table->CmdDrawIndirectCount = (PFN_vkCmdDrawIndirectCount) gpa(device, "vkCmdDrawIndirectCount");
    if (table->CmdDrawIndirectCount == nullptr) { table->CmdDrawIndirectCount = (PFN_vkCmdDrawIndirectCount)StubCmdDrawIndirectCount; }
    table->CmdDrawIndexedIndirectCount = (PFN_vkCmdDrawIndexedIndirectCount) gpa(device, "vkCmdDrawIndexedIndirectCount");
    if (table->CmdDrawIndexedIndirectCount == nullptr) { table->CmdDrawIndexedIndirectCount = (PFN_vkCmdDrawIndexedIndirectCount)StubCmdDrawIndexedIndirectCount; }
    table->CreateRenderPass2 = (PFN_vkCreateRenderPass2) gpa(device, "vkCreateRenderPass2");
    if (table->CreateRenderPass2 == nullptr) { table->CreateRenderPass2 = (PFN_vkCreateRenderPass2)StubCreateRenderPass2; }
    table->CmdBeginRenderPass2 = (PFN_vkCmdBeginRenderPass2) gpa(device, "vkCmdBeginRenderPass2");
    if (table->CmdBeginRenderPass2 == nullptr) { table->CmdBeginRenderPass2 = (PFN_vkCmdBeginRenderPass2)StubCmdBeginRenderPass2; }
    table->CmdNextSubpass2 = (PFN_vkCmdNextSubpass2) gpa(device, "vkCmdNextSubpass2");
    if (table->CmdNextSubpass2 == nullptr) { table->CmdNextSubpass2 = (PFN_vkCmdNextSubpass2)StubCmdNextSubpass2; }
    table->CmdEndRenderPass2 = (PFN_vkCmdEndRenderPass2) gpa(device, "vkCmdEndRenderPass2");
    if (table->CmdEndRenderPass2 == nullptr) { table->CmdEndRenderPass2 = (PFN_vkCmdEndRenderPass2)StubCmdEndRenderPass2; }
    table->ResetQueryPool = (PFN_vkResetQueryPool) gpa(device, "vkResetQueryPool");
    if (table->ResetQueryPool == nullptr) { table->ResetQueryPool = (PFN_vkResetQueryPool)StubResetQueryPool; }
    table->GetSemaphoreCounterValue = (PFN_vkGetSemaphoreCounterValue) gpa(device, "vkGetSemaphoreCounterValue");
    if (table->GetSemaphoreCounterValue == nullptr) { table->GetSemaphoreCounterValue = (PFN_vkGetSemaphoreCounterValue)StubGetSemaphoreCounterValue; }
    table->WaitSemaphores = (PFN_vkWaitSemaphores) gpa(device, "vkWaitSemaphores");
    if (table->WaitSemaphores == nullptr) { table->WaitSemaphores = (PFN_vkWaitSemaphores)StubWaitSemaphores; }
    table->SignalSemaphore = (PFN_vkSignalSemaphore) gpa(device, "vkSignalSemaphore");
    if (table->SignalSemaphore == nullptr) { table->SignalSemaphore = (PFN_vkSignalSemaphore)StubSignalSemaphore; }
    table->GetBufferDeviceAddress = (PFN_vkGetBufferDeviceAddress) gpa(device, "vkGetBufferDeviceAddress");
    if (table->GetBufferDeviceAddress == nullptr) { table->GetBufferDeviceAddress = (PFN_vkGetBufferDeviceAddress)StubGetBufferDeviceAddress; }
    table->GetBufferOpaqueCaptureAddress = (PFN_vkGetBufferOpaqueCaptureAddress) gpa(device, "vkGetBufferOpaqueCaptureAddress");
    if (table->GetBufferOpaqueCaptureAddress == nullptr) { table->GetBufferOpaqueCaptureAddress = (PFN_vkGetBufferOpaqueCaptureAddress)StubGetBufferOpaqueCaptureAddress; }
    table->GetDeviceMemoryOpaqueCaptureAddress = (PFN_vkGetDeviceMemoryOpaqueCaptureAddress) gpa(device, "vkGetDeviceMemoryOpaqueCaptureAddress");
    if (table->GetDeviceMemoryOpaqueCaptureAddress == nullptr) { table->GetDeviceMemoryOpaqueCaptureAddress = (PFN_vkGetDeviceMemoryOpaqueCaptureAddress)StubGetDeviceMemoryOpaqueCaptureAddress; }
    table->GetCommandPoolMemoryConsumption = (PFN_vkGetCommandPoolMemoryConsumption) gpa(device, "vkGetCommandPoolMemoryConsumption");
    if (table->GetCommandPoolMemoryConsumption == nullptr) { table->GetCommandPoolMemoryConsumption = (PFN_vkGetCommandPoolMemoryConsumption)StubGetCommandPoolMemoryConsumption; }
    table->GetFaultData = (PFN_vkGetFaultData) gpa(device, "vkGetFaultData");
    if (table->GetFaultData == nullptr) { table->GetFaultData = (PFN_vkGetFaultData)StubGetFaultData; }
    table->CreateSwapchainKHR = (PFN_vkCreateSwapchainKHR) gpa(device, "vkCreateSwapchainKHR");
    if (table->CreateSwapchainKHR == nullptr) { table->CreateSwapchainKHR = (PFN_vkCreateSwapchainKHR)StubCreateSwapchainKHR; }
    table->GetSwapchainImagesKHR = (PFN_vkGetSwapchainImagesKHR) gpa(device, "vkGetSwapchainImagesKHR");
    if (table->GetSwapchainImagesKHR == nullptr) { table->GetSwapchainImagesKHR = (PFN_vkGetSwapchainImagesKHR)StubGetSwapchainImagesKHR; }
    table->AcquireNextImageKHR = (PFN_vkAcquireNextImageKHR) gpa(device, "vkAcquireNextImageKHR");
    if (table->AcquireNextImageKHR == nullptr) { table->AcquireNextImageKHR = (PFN_vkAcquireNextImageKHR)StubAcquireNextImageKHR; }
    table->QueuePresentKHR = (PFN_vkQueuePresentKHR) gpa(device, "vkQueuePresentKHR");
    if (table->QueuePresentKHR == nullptr) { table->QueuePresentKHR = (PFN_vkQueuePresentKHR)StubQueuePresentKHR; }
    table->GetDeviceGroupPresentCapabilitiesKHR = (PFN_vkGetDeviceGroupPresentCapabilitiesKHR) gpa(device, "vkGetDeviceGroupPresentCapabilitiesKHR");
    if (table->GetDeviceGroupPresentCapabilitiesKHR == nullptr) { table->GetDeviceGroupPresentCapabilitiesKHR = (PFN_vkGetDeviceGroupPresentCapabilitiesKHR)StubGetDeviceGroupPresentCapabilitiesKHR; }
    table->GetDeviceGroupSurfacePresentModesKHR = (PFN_vkGetDeviceGroupSurfacePresentModesKHR) gpa(device, "vkGetDeviceGroupSurfacePresentModesKHR");
    if (table->GetDeviceGroupSurfacePresentModesKHR == nullptr) { table->GetDeviceGroupSurfacePresentModesKHR = (PFN_vkGetDeviceGroupSurfacePresentModesKHR)StubGetDeviceGroupSurfacePresentModesKHR; }
    table->AcquireNextImage2KHR = (PFN_vkAcquireNextImage2KHR) gpa(device, "vkAcquireNextImage2KHR");
    if (table->AcquireNextImage2KHR == nullptr) { table->AcquireNextImage2KHR = (PFN_vkAcquireNextImage2KHR)StubAcquireNextImage2KHR; }
    table->CreateSharedSwapchainsKHR = (PFN_vkCreateSharedSwapchainsKHR) gpa(device, "vkCreateSharedSwapchainsKHR");
    if (table->CreateSharedSwapchainsKHR == nullptr) { table->CreateSharedSwapchainsKHR = (PFN_vkCreateSharedSwapchainsKHR)StubCreateSharedSwapchainsKHR; }
    table->GetMemoryFdKHR = (PFN_vkGetMemoryFdKHR) gpa(device, "vkGetMemoryFdKHR");
    if (table->GetMemoryFdKHR == nullptr) { table->GetMemoryFdKHR = (PFN_vkGetMemoryFdKHR)StubGetMemoryFdKHR; }
    table->GetMemoryFdPropertiesKHR = (PFN_vkGetMemoryFdPropertiesKHR) gpa(device, "vkGetMemoryFdPropertiesKHR");
    if (table->GetMemoryFdPropertiesKHR == nullptr) { table->GetMemoryFdPropertiesKHR = (PFN_vkGetMemoryFdPropertiesKHR)StubGetMemoryFdPropertiesKHR; }
    table->ImportSemaphoreFdKHR = (PFN_vkImportSemaphoreFdKHR) gpa(device, "vkImportSemaphoreFdKHR");
    if (table->ImportSemaphoreFdKHR == nullptr) { table->ImportSemaphoreFdKHR = (PFN_vkImportSemaphoreFdKHR)StubImportSemaphoreFdKHR; }
    table->GetSemaphoreFdKHR = (PFN_vkGetSemaphoreFdKHR) gpa(device, "vkGetSemaphoreFdKHR");
    if (table->GetSemaphoreFdKHR == nullptr) { table->GetSemaphoreFdKHR = (PFN_vkGetSemaphoreFdKHR)StubGetSemaphoreFdKHR; }
    table->GetSwapchainStatusKHR = (PFN_vkGetSwapchainStatusKHR) gpa(device, "vkGetSwapchainStatusKHR");
    if (table->GetSwapchainStatusKHR == nullptr) { table->GetSwapchainStatusKHR = (PFN_vkGetSwapchainStatusKHR)StubGetSwapchainStatusKHR; }
    table->ImportFenceFdKHR = (PFN_vkImportFenceFdKHR) gpa(device, "vkImportFenceFdKHR");
    if (table->ImportFenceFdKHR == nullptr) { table->ImportFenceFdKHR = (PFN_vkImportFenceFdKHR)StubImportFenceFdKHR; }
    table->GetFenceFdKHR = (PFN_vkGetFenceFdKHR) gpa(device, "vkGetFenceFdKHR");
    if (table->GetFenceFdKHR == nullptr) { table->GetFenceFdKHR = (PFN_vkGetFenceFdKHR)StubGetFenceFdKHR; }
    table->AcquireProfilingLockKHR = (PFN_vkAcquireProfilingLockKHR) gpa(device, "vkAcquireProfilingLockKHR");
    if (table->AcquireProfilingLockKHR == nullptr) { table->AcquireProfilingLockKHR = (PFN_vkAcquireProfilingLockKHR)StubAcquireProfilingLockKHR; }
    table->ReleaseProfilingLockKHR = (PFN_vkReleaseProfilingLockKHR) gpa(device, "vkReleaseProfilingLockKHR");
    if (table->ReleaseProfilingLockKHR == nullptr) { table->ReleaseProfilingLockKHR = (PFN_vkReleaseProfilingLockKHR)StubReleaseProfilingLockKHR; }
    table->CmdSetFragmentShadingRateKHR = (PFN_vkCmdSetFragmentShadingRateKHR) gpa(device, "vkCmdSetFragmentShadingRateKHR");
    if (table->CmdSetFragmentShadingRateKHR == nullptr) { table->CmdSetFragmentShadingRateKHR = (PFN_vkCmdSetFragmentShadingRateKHR)StubCmdSetFragmentShadingRateKHR; }
    table->CmdRefreshObjectsKHR = (PFN_vkCmdRefreshObjectsKHR) gpa(device, "vkCmdRefreshObjectsKHR");
    if (table->CmdRefreshObjectsKHR == nullptr) { table->CmdRefreshObjectsKHR = (PFN_vkCmdRefreshObjectsKHR)StubCmdRefreshObjectsKHR; }
    table->CmdSetEvent2KHR = (PFN_vkCmdSetEvent2KHR) gpa(device, "vkCmdSetEvent2KHR");
    if (table->CmdSetEvent2KHR == nullptr) { table->CmdSetEvent2KHR = (PFN_vkCmdSetEvent2KHR)StubCmdSetEvent2KHR; }
    table->CmdResetEvent2KHR = (PFN_vkCmdResetEvent2KHR) gpa(device, "vkCmdResetEvent2KHR");
    if (table->CmdResetEvent2KHR == nullptr) { table->CmdResetEvent2KHR = (PFN_vkCmdResetEvent2KHR)StubCmdResetEvent2KHR; }
    table->CmdWaitEvents2KHR = (PFN_vkCmdWaitEvents2KHR) gpa(device, "vkCmdWaitEvents2KHR");
    if (table->CmdWaitEvents2KHR == nullptr) { table->CmdWaitEvents2KHR = (PFN_vkCmdWaitEvents2KHR)StubCmdWaitEvents2KHR; }
    table->CmdPipelineBarrier2KHR = (PFN_vkCmdPipelineBarrier2KHR) gpa(device, "vkCmdPipelineBarrier2KHR");
    if (table->CmdPipelineBarrier2KHR == nullptr) { table->CmdPipelineBarrier2KHR = (PFN_vkCmdPipelineBarrier2KHR)StubCmdPipelineBarrier2KHR; }
    table->CmdWriteTimestamp2KHR = (PFN_vkCmdWriteTimestamp2KHR) gpa(device, "vkCmdWriteTimestamp2KHR");
    if (table->CmdWriteTimestamp2KHR == nullptr) { table->CmdWriteTimestamp2KHR = (PFN_vkCmdWriteTimestamp2KHR)StubCmdWriteTimestamp2KHR; }
    table->QueueSubmit2KHR = (PFN_vkQueueSubmit2KHR) gpa(device, "vkQueueSubmit2KHR");
    if (table->QueueSubmit2KHR == nullptr) { table->QueueSubmit2KHR = (PFN_vkQueueSubmit2KHR)StubQueueSubmit2KHR; }
    table->CmdWriteBufferMarker2AMD = (PFN_vkCmdWriteBufferMarker2AMD) gpa(device, "vkCmdWriteBufferMarker2AMD");
    if (table->CmdWriteBufferMarker2AMD == nullptr) { table->CmdWriteBufferMarker2AMD = (PFN_vkCmdWriteBufferMarker2AMD)StubCmdWriteBufferMarker2AMD; }
    table->GetQueueCheckpointData2NV = (PFN_vkGetQueueCheckpointData2NV) gpa(device, "vkGetQueueCheckpointData2NV");
    if (table->GetQueueCheckpointData2NV == nullptr) { table->GetQueueCheckpointData2NV = (PFN_vkGetQueueCheckpointData2NV)StubGetQueueCheckpointData2NV; }
    table->CmdCopyBuffer2KHR = (PFN_vkCmdCopyBuffer2KHR) gpa(device, "vkCmdCopyBuffer2KHR");
    if (table->CmdCopyBuffer2KHR == nullptr) { table->CmdCopyBuffer2KHR = (PFN_vkCmdCopyBuffer2KHR)StubCmdCopyBuffer2KHR; }
    table->CmdCopyImage2KHR = (PFN_vkCmdCopyImage2KHR) gpa(device, "vkCmdCopyImage2KHR");
    if (table->CmdCopyImage2KHR == nullptr) { table->CmdCopyImage2KHR = (PFN_vkCmdCopyImage2KHR)StubCmdCopyImage2KHR; }
    table->CmdCopyBufferToImage2KHR = (PFN_vkCmdCopyBufferToImage2KHR) gpa(device, "vkCmdCopyBufferToImage2KHR");
    if (table->CmdCopyBufferToImage2KHR == nullptr) { table->CmdCopyBufferToImage2KHR = (PFN_vkCmdCopyBufferToImage2KHR)StubCmdCopyBufferToImage2KHR; }
    table->CmdCopyImageToBuffer2KHR = (PFN_vkCmdCopyImageToBuffer2KHR) gpa(device, "vkCmdCopyImageToBuffer2KHR");
    if (table->CmdCopyImageToBuffer2KHR == nullptr) { table->CmdCopyImageToBuffer2KHR = (PFN_vkCmdCopyImageToBuffer2KHR)StubCmdCopyImageToBuffer2KHR; }
    table->CmdBlitImage2KHR = (PFN_vkCmdBlitImage2KHR) gpa(device, "vkCmdBlitImage2KHR");
    if (table->CmdBlitImage2KHR == nullptr) { table->CmdBlitImage2KHR = (PFN_vkCmdBlitImage2KHR)StubCmdBlitImage2KHR; }
    table->CmdResolveImage2KHR = (PFN_vkCmdResolveImage2KHR) gpa(device, "vkCmdResolveImage2KHR");
    if (table->CmdResolveImage2KHR == nullptr) { table->CmdResolveImage2KHR = (PFN_vkCmdResolveImage2KHR)StubCmdResolveImage2KHR; }
    table->DisplayPowerControlEXT = (PFN_vkDisplayPowerControlEXT) gpa(device, "vkDisplayPowerControlEXT");
    if (table->DisplayPowerControlEXT == nullptr) { table->DisplayPowerControlEXT = (PFN_vkDisplayPowerControlEXT)StubDisplayPowerControlEXT; }
    table->RegisterDeviceEventEXT = (PFN_vkRegisterDeviceEventEXT) gpa(device, "vkRegisterDeviceEventEXT");
    if (table->RegisterDeviceEventEXT == nullptr) { table->RegisterDeviceEventEXT = (PFN_vkRegisterDeviceEventEXT)StubRegisterDeviceEventEXT; }
    table->RegisterDisplayEventEXT = (PFN_vkRegisterDisplayEventEXT) gpa(device, "vkRegisterDisplayEventEXT");
    if (table->RegisterDisplayEventEXT == nullptr) { table->RegisterDisplayEventEXT = (PFN_vkRegisterDisplayEventEXT)StubRegisterDisplayEventEXT; }
    table->GetSwapchainCounterEXT = (PFN_vkGetSwapchainCounterEXT) gpa(device, "vkGetSwapchainCounterEXT");
    if (table->GetSwapchainCounterEXT == nullptr) { table->GetSwapchainCounterEXT = (PFN_vkGetSwapchainCounterEXT)StubGetSwapchainCounterEXT; }
    table->CmdSetDiscardRectangleEXT = (PFN_vkCmdSetDiscardRectangleEXT) gpa(device, "vkCmdSetDiscardRectangleEXT");
    if (table->CmdSetDiscardRectangleEXT == nullptr) { table->CmdSetDiscardRectangleEXT = (PFN_vkCmdSetDiscardRectangleEXT)StubCmdSetDiscardRectangleEXT; }
    table->SetHdrMetadataEXT = (PFN_vkSetHdrMetadataEXT) gpa(device, "vkSetHdrMetadataEXT");
    if (table->SetHdrMetadataEXT == nullptr) { table->SetHdrMetadataEXT = (PFN_vkSetHdrMetadataEXT)StubSetHdrMetadataEXT; }
    table->SetDebugUtilsObjectNameEXT = (PFN_vkSetDebugUtilsObjectNameEXT) gpa(device, "vkSetDebugUtilsObjectNameEXT");
    if (table->SetDebugUtilsObjectNameEXT == nullptr) { table->SetDebugUtilsObjectNameEXT = (PFN_vkSetDebugUtilsObjectNameEXT)StubSetDebugUtilsObjectNameEXT; }
    table->SetDebugUtilsObjectTagEXT = (PFN_vkSetDebugUtilsObjectTagEXT) gpa(device, "vkSetDebugUtilsObjectTagEXT");
    if (table->SetDebugUtilsObjectTagEXT == nullptr) { table->SetDebugUtilsObjectTagEXT = (PFN_vkSetDebugUtilsObjectTagEXT)StubSetDebugUtilsObjectTagEXT; }
    table->QueueBeginDebugUtilsLabelEXT = (PFN_vkQueueBeginDebugUtilsLabelEXT) gpa(device, "vkQueueBeginDebugUtilsLabelEXT");
    if (table->QueueBeginDebugUtilsLabelEXT == nullptr) { table->QueueBeginDebugUtilsLabelEXT = (PFN_vkQueueBeginDebugUtilsLabelEXT)StubQueueBeginDebugUtilsLabelEXT; }
    table->QueueEndDebugUtilsLabelEXT = (PFN_vkQueueEndDebugUtilsLabelEXT) gpa(device, "vkQueueEndDebugUtilsLabelEXT");
    if (table->QueueEndDebugUtilsLabelEXT == nullptr) { table->QueueEndDebugUtilsLabelEXT = (PFN_vkQueueEndDebugUtilsLabelEXT)StubQueueEndDebugUtilsLabelEXT; }
    table->QueueInsertDebugUtilsLabelEXT = (PFN_vkQueueInsertDebugUtilsLabelEXT) gpa(device, "vkQueueInsertDebugUtilsLabelEXT");
    if (table->QueueInsertDebugUtilsLabelEXT == nullptr) { table->QueueInsertDebugUtilsLabelEXT = (PFN_vkQueueInsertDebugUtilsLabelEXT)StubQueueInsertDebugUtilsLabelEXT; }
    table->CmdBeginDebugUtilsLabelEXT = (PFN_vkCmdBeginDebugUtilsLabelEXT) gpa(device, "vkCmdBeginDebugUtilsLabelEXT");
    if (table->CmdBeginDebugUtilsLabelEXT == nullptr) { table->CmdBeginDebugUtilsLabelEXT = (PFN_vkCmdBeginDebugUtilsLabelEXT)StubCmdBeginDebugUtilsLabelEXT; }
    table->CmdEndDebugUtilsLabelEXT = (PFN_vkCmdEndDebugUtilsLabelEXT) gpa(device, "vkCmdEndDebugUtilsLabelEXT");
    if (table->CmdEndDebugUtilsLabelEXT == nullptr) { table->CmdEndDebugUtilsLabelEXT = (PFN_vkCmdEndDebugUtilsLabelEXT)StubCmdEndDebugUtilsLabelEXT; }
    table->CmdInsertDebugUtilsLabelEXT = (PFN_vkCmdInsertDebugUtilsLabelEXT) gpa(device, "vkCmdInsertDebugUtilsLabelEXT");
    if (table->CmdInsertDebugUtilsLabelEXT == nullptr) { table->CmdInsertDebugUtilsLabelEXT = (PFN_vkCmdInsertDebugUtilsLabelEXT)StubCmdInsertDebugUtilsLabelEXT; }
    table->CmdSetSampleLocationsEXT = (PFN_vkCmdSetSampleLocationsEXT) gpa(device, "vkCmdSetSampleLocationsEXT");
    if (table->CmdSetSampleLocationsEXT == nullptr) { table->CmdSetSampleLocationsEXT = (PFN_vkCmdSetSampleLocationsEXT)StubCmdSetSampleLocationsEXT; }
    table->GetImageDrmFormatModifierPropertiesEXT = (PFN_vkGetImageDrmFormatModifierPropertiesEXT) gpa(device, "vkGetImageDrmFormatModifierPropertiesEXT");
    if (table->GetImageDrmFormatModifierPropertiesEXT == nullptr) { table->GetImageDrmFormatModifierPropertiesEXT = (PFN_vkGetImageDrmFormatModifierPropertiesEXT)StubGetImageDrmFormatModifierPropertiesEXT; }
    table->GetMemoryHostPointerPropertiesEXT = (PFN_vkGetMemoryHostPointerPropertiesEXT) gpa(device, "vkGetMemoryHostPointerPropertiesEXT");
    if (table->GetMemoryHostPointerPropertiesEXT == nullptr) { table->GetMemoryHostPointerPropertiesEXT = (PFN_vkGetMemoryHostPointerPropertiesEXT)StubGetMemoryHostPointerPropertiesEXT; }
    table->GetCalibratedTimestampsEXT = (PFN_vkGetCalibratedTimestampsEXT) gpa(device, "vkGetCalibratedTimestampsEXT");
    if (table->GetCalibratedTimestampsEXT == nullptr) { table->GetCalibratedTimestampsEXT = (PFN_vkGetCalibratedTimestampsEXT)StubGetCalibratedTimestampsEXT; }
    table->CmdSetLineStippleEXT = (PFN_vkCmdSetLineStippleEXT) gpa(device, "vkCmdSetLineStippleEXT");
    if (table->CmdSetLineStippleEXT == nullptr) { table->CmdSetLineStippleEXT = (PFN_vkCmdSetLineStippleEXT)StubCmdSetLineStippleEXT; }
    table->CmdSetCullModeEXT = (PFN_vkCmdSetCullModeEXT) gpa(device, "vkCmdSetCullModeEXT");
    if (table->CmdSetCullModeEXT == nullptr) { table->CmdSetCullModeEXT = (PFN_vkCmdSetCullModeEXT)StubCmdSetCullModeEXT; }
    table->CmdSetFrontFaceEXT = (PFN_vkCmdSetFrontFaceEXT) gpa(device, "vkCmdSetFrontFaceEXT");
    if (table->CmdSetFrontFaceEXT == nullptr) { table->CmdSetFrontFaceEXT = (PFN_vkCmdSetFrontFaceEXT)StubCmdSetFrontFaceEXT; }
    table->CmdSetPrimitiveTopologyEXT = (PFN_vkCmdSetPrimitiveTopologyEXT) gpa(device, "vkCmdSetPrimitiveTopologyEXT");
    if (table->CmdSetPrimitiveTopologyEXT == nullptr) { table->CmdSetPrimitiveTopologyEXT = (PFN_vkCmdSetPrimitiveTopologyEXT)StubCmdSetPrimitiveTopologyEXT; }
    table->CmdSetViewportWithCountEXT = (PFN_vkCmdSetViewportWithCountEXT) gpa(device, "vkCmdSetViewportWithCountEXT");
    if (table->CmdSetViewportWithCountEXT == nullptr) { table->CmdSetViewportWithCountEXT = (PFN_vkCmdSetViewportWithCountEXT)StubCmdSetViewportWithCountEXT; }
    table->CmdSetScissorWithCountEXT = (PFN_vkCmdSetScissorWithCountEXT) gpa(device, "vkCmdSetScissorWithCountEXT");
    if (table->CmdSetScissorWithCountEXT == nullptr) { table->CmdSetScissorWithCountEXT = (PFN_vkCmdSetScissorWithCountEXT)StubCmdSetScissorWithCountEXT; }
    table->CmdBindVertexBuffers2EXT = (PFN_vkCmdBindVertexBuffers2EXT) gpa(device, "vkCmdBindVertexBuffers2EXT");
    if (table->CmdBindVertexBuffers2EXT == nullptr) { table->CmdBindVertexBuffers2EXT = (PFN_vkCmdBindVertexBuffers2EXT)StubCmdBindVertexBuffers2EXT; }
    table->CmdSetDepthTestEnableEXT = (PFN_vkCmdSetDepthTestEnableEXT) gpa(device, "vkCmdSetDepthTestEnableEXT");
    if (table->CmdSetDepthTestEnableEXT == nullptr) { table->CmdSetDepthTestEnableEXT = (PFN_vkCmdSetDepthTestEnableEXT)StubCmdSetDepthTestEnableEXT; }
    table->CmdSetDepthWriteEnableEXT = (PFN_vkCmdSetDepthWriteEnableEXT) gpa(device, "vkCmdSetDepthWriteEnableEXT");
    if (table->CmdSetDepthWriteEnableEXT == nullptr) { table->CmdSetDepthWriteEnableEXT = (PFN_vkCmdSetDepthWriteEnableEXT)StubCmdSetDepthWriteEnableEXT; }
    table->CmdSetDepthCompareOpEXT = (PFN_vkCmdSetDepthCompareOpEXT) gpa(device, "vkCmdSetDepthCompareOpEXT");
    if (table->CmdSetDepthCompareOpEXT == nullptr) { table->CmdSetDepthCompareOpEXT = (PFN_vkCmdSetDepthCompareOpEXT)StubCmdSetDepthCompareOpEXT; }
    table->CmdSetDepthBoundsTestEnableEXT = (PFN_vkCmdSetDepthBoundsTestEnableEXT) gpa(device, "vkCmdSetDepthBoundsTestEnableEXT");
    if (table->CmdSetDepthBoundsTestEnableEXT == nullptr) { table->CmdSetDepthBoundsTestEnableEXT = (PFN_vkCmdSetDepthBoundsTestEnableEXT)StubCmdSetDepthBoundsTestEnableEXT; }
    table->CmdSetStencilTestEnableEXT = (PFN_vkCmdSetStencilTestEnableEXT) gpa(device, "vkCmdSetStencilTestEnableEXT");
    if (table->CmdSetStencilTestEnableEXT == nullptr) { table->CmdSetStencilTestEnableEXT = (PFN_vkCmdSetStencilTestEnableEXT)StubCmdSetStencilTestEnableEXT; }
    table->CmdSetStencilOpEXT = (PFN_vkCmdSetStencilOpEXT) gpa(device, "vkCmdSetStencilOpEXT");
    if (table->CmdSetStencilOpEXT == nullptr) { table->CmdSetStencilOpEXT = (PFN_vkCmdSetStencilOpEXT)StubCmdSetStencilOpEXT; }
    table->CmdSetVertexInputEXT = (PFN_vkCmdSetVertexInputEXT) gpa(device, "vkCmdSetVertexInputEXT");
    if (table->CmdSetVertexInputEXT == nullptr) { table->CmdSetVertexInputEXT = (PFN_vkCmdSetVertexInputEXT)StubCmdSetVertexInputEXT; }
    table->CmdSetPatchControlPointsEXT = (PFN_vkCmdSetPatchControlPointsEXT) gpa(device, "vkCmdSetPatchControlPointsEXT");
    if (table->CmdSetPatchControlPointsEXT == nullptr) { table->CmdSetPatchControlPointsEXT = (PFN_vkCmdSetPatchControlPointsEXT)StubCmdSetPatchControlPointsEXT; }
    table->CmdSetRasterizerDiscardEnableEXT = (PFN_vkCmdSetRasterizerDiscardEnableEXT) gpa(device, "vkCmdSetRasterizerDiscardEnableEXT");
    if (table->CmdSetRasterizerDiscardEnableEXT == nullptr) { table->CmdSetRasterizerDiscardEnableEXT = (PFN_vkCmdSetRasterizerDiscardEnableEXT)StubCmdSetRasterizerDiscardEnableEXT; }
    table->CmdSetDepthBiasEnableEXT = (PFN_vkCmdSetDepthBiasEnableEXT) gpa(device, "vkCmdSetDepthBiasEnableEXT");
    if (table->CmdSetDepthBiasEnableEXT == nullptr) { table->CmdSetDepthBiasEnableEXT = (PFN_vkCmdSetDepthBiasEnableEXT)StubCmdSetDepthBiasEnableEXT; }
    table->CmdSetLogicOpEXT = (PFN_vkCmdSetLogicOpEXT) gpa(device, "vkCmdSetLogicOpEXT");
    if (table->CmdSetLogicOpEXT == nullptr) { table->CmdSetLogicOpEXT = (PFN_vkCmdSetLogicOpEXT)StubCmdSetLogicOpEXT; }
    table->CmdSetPrimitiveRestartEnableEXT = (PFN_vkCmdSetPrimitiveRestartEnableEXT) gpa(device, "vkCmdSetPrimitiveRestartEnableEXT");
    if (table->CmdSetPrimitiveRestartEnableEXT == nullptr) { table->CmdSetPrimitiveRestartEnableEXT = (PFN_vkCmdSetPrimitiveRestartEnableEXT)StubCmdSetPrimitiveRestartEnableEXT; }
    table->CmdSetColorWriteEnableEXT = (PFN_vkCmdSetColorWriteEnableEXT) gpa(device, "vkCmdSetColorWriteEnableEXT");
    if (table->CmdSetColorWriteEnableEXT == nullptr) { table->CmdSetColorWriteEnableEXT = (PFN_vkCmdSetColorWriteEnableEXT)StubCmdSetColorWriteEnableEXT; }
}


static inline void layer_init_instance_dispatch_table(VkInstance instance, VkLayerInstanceDispatchTable *table, PFN_vkGetInstanceProcAddr gpa) {
    memset(table, 0, sizeof(*table));
    // Instance function pointers
    table->DestroyInstance = (PFN_vkDestroyInstance) gpa(instance, "vkDestroyInstance");
    table->EnumeratePhysicalDevices = (PFN_vkEnumeratePhysicalDevices) gpa(instance, "vkEnumeratePhysicalDevices");
    table->GetPhysicalDeviceFeatures = (PFN_vkGetPhysicalDeviceFeatures) gpa(instance, "vkGetPhysicalDeviceFeatures");
    table->GetPhysicalDeviceFormatProperties = (PFN_vkGetPhysicalDeviceFormatProperties) gpa(instance, "vkGetPhysicalDeviceFormatProperties");
    table->GetPhysicalDeviceImageFormatProperties = (PFN_vkGetPhysicalDeviceImageFormatProperties) gpa(instance, "vkGetPhysicalDeviceImageFormatProperties");
    table->GetPhysicalDeviceProperties = (PFN_vkGetPhysicalDeviceProperties) gpa(instance, "vkGetPhysicalDeviceProperties");
    table->GetPhysicalDeviceQueueFamilyProperties = (PFN_vkGetPhysicalDeviceQueueFamilyProperties) gpa(instance, "vkGetPhysicalDeviceQueueFamilyProperties");
    table->GetPhysicalDeviceMemoryProperties = (PFN_vkGetPhysicalDeviceMemoryProperties) gpa(instance, "vkGetPhysicalDeviceMemoryProperties");
    table->GetInstanceProcAddr = gpa;
    table->EnumerateDeviceExtensionProperties = (PFN_vkEnumerateDeviceExtensionProperties) gpa(instance, "vkEnumerateDeviceExtensionProperties");
    table->EnumerateDeviceLayerProperties = (PFN_vkEnumerateDeviceLayerProperties) gpa(instance, "vkEnumerateDeviceLayerProperties");
    table->EnumeratePhysicalDeviceGroups = (PFN_vkEnumeratePhysicalDeviceGroups) gpa(instance, "vkEnumeratePhysicalDeviceGroups");
    if (table->EnumeratePhysicalDeviceGroups == nullptr) { table->EnumeratePhysicalDeviceGroups = (PFN_vkEnumeratePhysicalDeviceGroups)StubEnumeratePhysicalDeviceGroups; }
    table->GetPhysicalDeviceFeatures2 = (PFN_vkGetPhysicalDeviceFeatures2) gpa(instance, "vkGetPhysicalDeviceFeatures2");
    if (table->GetPhysicalDeviceFeatures2 == nullptr) { table->GetPhysicalDeviceFeatures2 = (PFN_vkGetPhysicalDeviceFeatures2)StubGetPhysicalDeviceFeatures2; }
    table->GetPhysicalDeviceProperties2 = (PFN_vkGetPhysicalDeviceProperties2) gpa(instance, "vkGetPhysicalDeviceProperties2");
    if (table->GetPhysicalDeviceProperties2 == nullptr) { table->GetPhysicalDeviceProperties2 = (PFN_vkGetPhysicalDeviceProperties2)StubGetPhysicalDeviceProperties2; }
    table->GetPhysicalDeviceFormatProperties2 = (PFN_vkGetPhysicalDeviceFormatProperties2) gpa(instance, "vkGetPhysicalDeviceFormatProperties2");
    if (table->GetPhysicalDeviceFormatProperties2 == nullptr) { table->GetPhysicalDeviceFormatProperties2 = (PFN_vkGetPhysicalDeviceFormatProperties2)StubGetPhysicalDeviceFormatProperties2; }
    table->GetPhysicalDeviceImageFormatProperties2 = (PFN_vkGetPhysicalDeviceImageFormatProperties2) gpa(instance, "vkGetPhysicalDeviceImageFormatProperties2");
    if (table->GetPhysicalDeviceImageFormatProperties2 == nullptr) { table->GetPhysicalDeviceImageFormatProperties2 = (PFN_vkGetPhysicalDeviceImageFormatProperties2)StubGetPhysicalDeviceImageFormatProperties2; }
    table->GetPhysicalDeviceQueueFamilyProperties2 = (PFN_vkGetPhysicalDeviceQueueFamilyProperties2) gpa(instance, "vkGetPhysicalDeviceQueueFamilyProperties2");
    if (table->GetPhysicalDeviceQueueFamilyProperties2 == nullptr) { table->GetPhysicalDeviceQueueFamilyProperties2 = (PFN_vkGetPhysicalDeviceQueueFamilyProperties2)StubGetPhysicalDeviceQueueFamilyProperties2; }
    table->GetPhysicalDeviceMemoryProperties2 = (PFN_vkGetPhysicalDeviceMemoryProperties2) gpa(instance, "vkGetPhysicalDeviceMemoryProperties2");
    if (table->GetPhysicalDeviceMemoryProperties2 == nullptr) { table->GetPhysicalDeviceMemoryProperties2 = (PFN_vkGetPhysicalDeviceMemoryProperties2)StubGetPhysicalDeviceMemoryProperties2; }
    table->GetPhysicalDeviceExternalBufferProperties = (PFN_vkGetPhysicalDeviceExternalBufferProperties) gpa(instance, "vkGetPhysicalDeviceExternalBufferProperties");
    if (table->GetPhysicalDeviceExternalBufferProperties == nullptr) { table->GetPhysicalDeviceExternalBufferProperties = (PFN_vkGetPhysicalDeviceExternalBufferProperties)StubGetPhysicalDeviceExternalBufferProperties; }
    table->GetPhysicalDeviceExternalFenceProperties = (PFN_vkGetPhysicalDeviceExternalFenceProperties) gpa(instance, "vkGetPhysicalDeviceExternalFenceProperties");
    if (table->GetPhysicalDeviceExternalFenceProperties == nullptr) { table->GetPhysicalDeviceExternalFenceProperties = (PFN_vkGetPhysicalDeviceExternalFenceProperties)StubGetPhysicalDeviceExternalFenceProperties; }
    table->GetPhysicalDeviceExternalSemaphoreProperties = (PFN_vkGetPhysicalDeviceExternalSemaphoreProperties) gpa(instance, "vkGetPhysicalDeviceExternalSemaphoreProperties");
    if (table->GetPhysicalDeviceExternalSemaphoreProperties == nullptr) { table->GetPhysicalDeviceExternalSemaphoreProperties = (PFN_vkGetPhysicalDeviceExternalSemaphoreProperties)StubGetPhysicalDeviceExternalSemaphoreProperties; }
    table->DestroySurfaceKHR = (PFN_vkDestroySurfaceKHR) gpa(instance, "vkDestroySurfaceKHR");
    if (table->DestroySurfaceKHR == nullptr) { table->DestroySurfaceKHR = (PFN_vkDestroySurfaceKHR)StubDestroySurfaceKHR; }
    table->GetPhysicalDeviceSurfaceSupportKHR = (PFN_vkGetPhysicalDeviceSurfaceSupportKHR) gpa(instance, "vkGetPhysicalDeviceSurfaceSupportKHR");
    if (table->GetPhysicalDeviceSurfaceSupportKHR == nullptr) { table->GetPhysicalDeviceSurfaceSupportKHR = (PFN_vkGetPhysicalDeviceSurfaceSupportKHR)StubGetPhysicalDeviceSurfaceSupportKHR; }
    table->GetPhysicalDeviceSurfaceCapabilitiesKHR = (PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR) gpa(instance, "vkGetPhysicalDeviceSurfaceCapabilitiesKHR");
    if (table->GetPhysicalDeviceSurfaceCapabilitiesKHR == nullptr) { table->GetPhysicalDeviceSurfaceCapabilitiesKHR = (PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR)StubGetPhysicalDeviceSurfaceCapabilitiesKHR; }
    table->GetPhysicalDeviceSurfaceFormatsKHR = (PFN_vkGetPhysicalDeviceSurfaceFormatsKHR) gpa(instance, "vkGetPhysicalDeviceSurfaceFormatsKHR");
    if (table->GetPhysicalDeviceSurfaceFormatsKHR == nullptr) { table->GetPhysicalDeviceSurfaceFormatsKHR = (PFN_vkGetPhysicalDeviceSurfaceFormatsKHR)StubGetPhysicalDeviceSurfaceFormatsKHR; }
    table->GetPhysicalDeviceSurfacePresentModesKHR = (PFN_vkGetPhysicalDeviceSurfacePresentModesKHR) gpa(instance, "vkGetPhysicalDeviceSurfacePresentModesKHR");
    if (table->GetPhysicalDeviceSurfacePresentModesKHR == nullptr) { table->GetPhysicalDeviceSurfacePresentModesKHR = (PFN_vkGetPhysicalDeviceSurfacePresentModesKHR)StubGetPhysicalDeviceSurfacePresentModesKHR; }
    table->GetPhysicalDevicePresentRectanglesKHR = (PFN_vkGetPhysicalDevicePresentRectanglesKHR) gpa(instance, "vkGetPhysicalDevicePresentRectanglesKHR");
    if (table->GetPhysicalDevicePresentRectanglesKHR == nullptr) { table->GetPhysicalDevicePresentRectanglesKHR = (PFN_vkGetPhysicalDevicePresentRectanglesKHR)StubGetPhysicalDevicePresentRectanglesKHR; }
    table->GetPhysicalDeviceDisplayPropertiesKHR = (PFN_vkGetPhysicalDeviceDisplayPropertiesKHR) gpa(instance, "vkGetPhysicalDeviceDisplayPropertiesKHR");
    if (table->GetPhysicalDeviceDisplayPropertiesKHR == nullptr) { table->GetPhysicalDeviceDisplayPropertiesKHR = (PFN_vkGetPhysicalDeviceDisplayPropertiesKHR)StubGetPhysicalDeviceDisplayPropertiesKHR; }
    table->GetPhysicalDeviceDisplayPlanePropertiesKHR = (PFN_vkGetPhysicalDeviceDisplayPlanePropertiesKHR) gpa(instance, "vkGetPhysicalDeviceDisplayPlanePropertiesKHR");
    if (table->GetPhysicalDeviceDisplayPlanePropertiesKHR == nullptr) { table->GetPhysicalDeviceDisplayPlanePropertiesKHR = (PFN_vkGetPhysicalDeviceDisplayPlanePropertiesKHR)StubGetPhysicalDeviceDisplayPlanePropertiesKHR; }
    table->GetDisplayPlaneSupportedDisplaysKHR = (PFN_vkGetDisplayPlaneSupportedDisplaysKHR) gpa(instance, "vkGetDisplayPlaneSupportedDisplaysKHR");
    if (table->GetDisplayPlaneSupportedDisplaysKHR == nullptr) { table->GetDisplayPlaneSupportedDisplaysKHR = (PFN_vkGetDisplayPlaneSupportedDisplaysKHR)StubGetDisplayPlaneSupportedDisplaysKHR; }
    table->GetDisplayModePropertiesKHR = (PFN_vkGetDisplayModePropertiesKHR) gpa(instance, "vkGetDisplayModePropertiesKHR");
    if (table->GetDisplayModePropertiesKHR == nullptr) { table->GetDisplayModePropertiesKHR = (PFN_vkGetDisplayModePropertiesKHR)StubGetDisplayModePropertiesKHR; }
    table->CreateDisplayModeKHR = (PFN_vkCreateDisplayModeKHR) gpa(instance, "vkCreateDisplayModeKHR");
    if (table->CreateDisplayModeKHR == nullptr) { table->CreateDisplayModeKHR = (PFN_vkCreateDisplayModeKHR)StubCreateDisplayModeKHR; }
    table->GetDisplayPlaneCapabilitiesKHR = (PFN_vkGetDisplayPlaneCapabilitiesKHR) gpa(instance, "vkGetDisplayPlaneCapabilitiesKHR");
    if (table->GetDisplayPlaneCapabilitiesKHR == nullptr) { table->GetDisplayPlaneCapabilitiesKHR = (PFN_vkGetDisplayPlaneCapabilitiesKHR)StubGetDisplayPlaneCapabilitiesKHR; }
    table->CreateDisplayPlaneSurfaceKHR = (PFN_vkCreateDisplayPlaneSurfaceKHR) gpa(instance, "vkCreateDisplayPlaneSurfaceKHR");
    if (table->CreateDisplayPlaneSurfaceKHR == nullptr) { table->CreateDisplayPlaneSurfaceKHR = (PFN_vkCreateDisplayPlaneSurfaceKHR)StubCreateDisplayPlaneSurfaceKHR; }
    table->EnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR = (PFN_vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR) gpa(instance, "vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR");
    if (table->EnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR == nullptr) { table->EnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR = (PFN_vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR)StubEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR; }
    table->GetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR = (PFN_vkGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR) gpa(instance, "vkGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR");
    if (table->GetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR == nullptr) { table->GetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR = (PFN_vkGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR)StubGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR; }
    table->GetPhysicalDeviceSurfaceCapabilities2KHR = (PFN_vkGetPhysicalDeviceSurfaceCapabilities2KHR) gpa(instance, "vkGetPhysicalDeviceSurfaceCapabilities2KHR");
    if (table->GetPhysicalDeviceSurfaceCapabilities2KHR == nullptr) { table->GetPhysicalDeviceSurfaceCapabilities2KHR = (PFN_vkGetPhysicalDeviceSurfaceCapabilities2KHR)StubGetPhysicalDeviceSurfaceCapabilities2KHR; }
    table->GetPhysicalDeviceSurfaceFormats2KHR = (PFN_vkGetPhysicalDeviceSurfaceFormats2KHR) gpa(instance, "vkGetPhysicalDeviceSurfaceFormats2KHR");
    if (table->GetPhysicalDeviceSurfaceFormats2KHR == nullptr) { table->GetPhysicalDeviceSurfaceFormats2KHR = (PFN_vkGetPhysicalDeviceSurfaceFormats2KHR)StubGetPhysicalDeviceSurfaceFormats2KHR; }
    table->GetPhysicalDeviceDisplayProperties2KHR = (PFN_vkGetPhysicalDeviceDisplayProperties2KHR) gpa(instance, "vkGetPhysicalDeviceDisplayProperties2KHR");
    if (table->GetPhysicalDeviceDisplayProperties2KHR == nullptr) { table->GetPhysicalDeviceDisplayProperties2KHR = (PFN_vkGetPhysicalDeviceDisplayProperties2KHR)StubGetPhysicalDeviceDisplayProperties2KHR; }
    table->GetPhysicalDeviceDisplayPlaneProperties2KHR = (PFN_vkGetPhysicalDeviceDisplayPlaneProperties2KHR) gpa(instance, "vkGetPhysicalDeviceDisplayPlaneProperties2KHR");
    if (table->GetPhysicalDeviceDisplayPlaneProperties2KHR == nullptr) { table->GetPhysicalDeviceDisplayPlaneProperties2KHR = (PFN_vkGetPhysicalDeviceDisplayPlaneProperties2KHR)StubGetPhysicalDeviceDisplayPlaneProperties2KHR; }
    table->GetDisplayModeProperties2KHR = (PFN_vkGetDisplayModeProperties2KHR) gpa(instance, "vkGetDisplayModeProperties2KHR");
    if (table->GetDisplayModeProperties2KHR == nullptr) { table->GetDisplayModeProperties2KHR = (PFN_vkGetDisplayModeProperties2KHR)StubGetDisplayModeProperties2KHR; }
    table->GetDisplayPlaneCapabilities2KHR = (PFN_vkGetDisplayPlaneCapabilities2KHR) gpa(instance, "vkGetDisplayPlaneCapabilities2KHR");
    if (table->GetDisplayPlaneCapabilities2KHR == nullptr) { table->GetDisplayPlaneCapabilities2KHR = (PFN_vkGetDisplayPlaneCapabilities2KHR)StubGetDisplayPlaneCapabilities2KHR; }
    table->GetPhysicalDeviceFragmentShadingRatesKHR = (PFN_vkGetPhysicalDeviceFragmentShadingRatesKHR) gpa(instance, "vkGetPhysicalDeviceFragmentShadingRatesKHR");
    if (table->GetPhysicalDeviceFragmentShadingRatesKHR == nullptr) { table->GetPhysicalDeviceFragmentShadingRatesKHR = (PFN_vkGetPhysicalDeviceFragmentShadingRatesKHR)StubGetPhysicalDeviceFragmentShadingRatesKHR; }
    table->GetPhysicalDeviceRefreshableObjectTypesKHR = (PFN_vkGetPhysicalDeviceRefreshableObjectTypesKHR) gpa(instance, "vkGetPhysicalDeviceRefreshableObjectTypesKHR");
    if (table->GetPhysicalDeviceRefreshableObjectTypesKHR == nullptr) { table->GetPhysicalDeviceRefreshableObjectTypesKHR = (PFN_vkGetPhysicalDeviceRefreshableObjectTypesKHR)StubGetPhysicalDeviceRefreshableObjectTypesKHR; }
    table->ReleaseDisplayEXT = (PFN_vkReleaseDisplayEXT) gpa(instance, "vkReleaseDisplayEXT");
    if (table->ReleaseDisplayEXT == nullptr) { table->ReleaseDisplayEXT = (PFN_vkReleaseDisplayEXT)StubReleaseDisplayEXT; }
    table->GetPhysicalDeviceSurfaceCapabilities2EXT = (PFN_vkGetPhysicalDeviceSurfaceCapabilities2EXT) gpa(instance, "vkGetPhysicalDeviceSurfaceCapabilities2EXT");
    if (table->GetPhysicalDeviceSurfaceCapabilities2EXT == nullptr) { table->GetPhysicalDeviceSurfaceCapabilities2EXT = (PFN_vkGetPhysicalDeviceSurfaceCapabilities2EXT)StubGetPhysicalDeviceSurfaceCapabilities2EXT; }
    table->CreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT) gpa(instance, "vkCreateDebugUtilsMessengerEXT");
    if (table->CreateDebugUtilsMessengerEXT == nullptr) { table->CreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)StubCreateDebugUtilsMessengerEXT; }
    table->DestroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT) gpa(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (table->DestroyDebugUtilsMessengerEXT == nullptr) { table->DestroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT)StubDestroyDebugUtilsMessengerEXT; }
    table->SubmitDebugUtilsMessageEXT = (PFN_vkSubmitDebugUtilsMessageEXT) gpa(instance, "vkSubmitDebugUtilsMessageEXT");
    if (table->SubmitDebugUtilsMessageEXT == nullptr) { table->SubmitDebugUtilsMessageEXT = (PFN_vkSubmitDebugUtilsMessageEXT)StubSubmitDebugUtilsMessageEXT; }
    table->GetPhysicalDeviceMultisamplePropertiesEXT = (PFN_vkGetPhysicalDeviceMultisamplePropertiesEXT) gpa(instance, "vkGetPhysicalDeviceMultisamplePropertiesEXT");
    if (table->GetPhysicalDeviceMultisamplePropertiesEXT == nullptr) { table->GetPhysicalDeviceMultisamplePropertiesEXT = (PFN_vkGetPhysicalDeviceMultisamplePropertiesEXT)StubGetPhysicalDeviceMultisamplePropertiesEXT; }
    table->GetPhysicalDeviceCalibrateableTimeDomainsEXT = (PFN_vkGetPhysicalDeviceCalibrateableTimeDomainsEXT) gpa(instance, "vkGetPhysicalDeviceCalibrateableTimeDomainsEXT");
    if (table->GetPhysicalDeviceCalibrateableTimeDomainsEXT == nullptr) { table->GetPhysicalDeviceCalibrateableTimeDomainsEXT = (PFN_vkGetPhysicalDeviceCalibrateableTimeDomainsEXT)StubGetPhysicalDeviceCalibrateableTimeDomainsEXT; }
    table->CreateHeadlessSurfaceEXT = (PFN_vkCreateHeadlessSurfaceEXT) gpa(instance, "vkCreateHeadlessSurfaceEXT");
    if (table->CreateHeadlessSurfaceEXT == nullptr) { table->CreateHeadlessSurfaceEXT = (PFN_vkCreateHeadlessSurfaceEXT)StubCreateHeadlessSurfaceEXT; }
}
