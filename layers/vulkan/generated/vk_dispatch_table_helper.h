// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See dispatch_table_helper_generator.py for modifications

/***************************************************************************
 *
 * Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
 * Copyright (c) 2015-2023 Google Inc.
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
// NOLINTBEGIN
#pragma once

#include <vulkan/vulkan.h>
#include <vulkan/vk_layer.h>
#include <cstring>
#include <string>
#include "vk_layer_dispatch_table.h"
#include "vk_extension_helper.h"

static VKAPI_ATTR VkResult VKAPI_CALL StubBindBufferMemory2(VkDevice device, uint32_t bindInfoCount,
                                                            const VkBindBufferMemoryInfo* pBindInfos) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubBindImageMemory2(VkDevice device, uint32_t bindInfoCount,
                                                           const VkBindImageMemoryInfo* pBindInfos) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubGetDeviceGroupPeerMemoryFeatures(VkDevice device, uint32_t heapIndex,
                                                                       uint32_t localDeviceIndex, uint32_t remoteDeviceIndex,
                                                                       VkPeerMemoryFeatureFlags* pPeerMemoryFeatures) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetDeviceMask(VkCommandBuffer commandBuffer, uint32_t deviceMask) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdDispatchBase(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY,
                                                      uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY,
                                                      uint32_t groupCountZ) {}
static VKAPI_ATTR VkResult VKAPI_CALL StubEnumeratePhysicalDeviceGroups(
    VkInstance instance, uint32_t* pPhysicalDeviceGroupCount, VkPhysicalDeviceGroupProperties* pPhysicalDeviceGroupProperties) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubGetImageMemoryRequirements2(VkDevice device, const VkImageMemoryRequirementsInfo2* pInfo,
                                                                  VkMemoryRequirements2* pMemoryRequirements) {}
static VKAPI_ATTR void VKAPI_CALL StubGetBufferMemoryRequirements2(VkDevice device, const VkBufferMemoryRequirementsInfo2* pInfo,
                                                                   VkMemoryRequirements2* pMemoryRequirements) {}
static VKAPI_ATTR void VKAPI_CALL StubGetImageSparseMemoryRequirements2(
    VkDevice device, const VkImageSparseMemoryRequirementsInfo2* pInfo, uint32_t* pSparseMemoryRequirementCount,
    VkSparseImageMemoryRequirements2* pSparseMemoryRequirements) {}
static VKAPI_ATTR void VKAPI_CALL StubGetPhysicalDeviceFeatures2(VkPhysicalDevice physicalDevice,
                                                                 VkPhysicalDeviceFeatures2* pFeatures) {}
static VKAPI_ATTR void VKAPI_CALL StubGetPhysicalDeviceProperties2(VkPhysicalDevice physicalDevice,
                                                                   VkPhysicalDeviceProperties2* pProperties) {}
static VKAPI_ATTR void VKAPI_CALL StubGetPhysicalDeviceFormatProperties2(VkPhysicalDevice physicalDevice, VkFormat format,
                                                                         VkFormatProperties2* pFormatProperties) {}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetPhysicalDeviceImageFormatProperties2(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceImageFormatInfo2* pImageFormatInfo,
    VkImageFormatProperties2* pImageFormatProperties) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubGetPhysicalDeviceQueueFamilyProperties2(VkPhysicalDevice physicalDevice,
                                                                              uint32_t* pQueueFamilyPropertyCount,
                                                                              VkQueueFamilyProperties2* pQueueFamilyProperties) {}
static VKAPI_ATTR void VKAPI_CALL StubGetPhysicalDeviceMemoryProperties2(VkPhysicalDevice physicalDevice,
                                                                         VkPhysicalDeviceMemoryProperties2* pMemoryProperties) {}
static VKAPI_ATTR void VKAPI_CALL StubGetPhysicalDeviceSparseImageFormatProperties2(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSparseImageFormatInfo2* pFormatInfo, uint32_t* pPropertyCount,
    VkSparseImageFormatProperties2* pProperties) {}
static VKAPI_ATTR void VKAPI_CALL StubTrimCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolTrimFlags flags) {}
static VKAPI_ATTR void VKAPI_CALL StubGetDeviceQueue2(VkDevice device, const VkDeviceQueueInfo2* pQueueInfo, VkQueue* pQueue) {}
static VKAPI_ATTR VkResult VKAPI_CALL StubCreateSamplerYcbcrConversion(VkDevice device,
                                                                       const VkSamplerYcbcrConversionCreateInfo* pCreateInfo,
                                                                       const VkAllocationCallbacks* pAllocator,
                                                                       VkSamplerYcbcrConversion* pYcbcrConversion) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubDestroySamplerYcbcrConversion(VkDevice device, VkSamplerYcbcrConversion ycbcrConversion,
                                                                    const VkAllocationCallbacks* pAllocator) {}
static VKAPI_ATTR VkResult VKAPI_CALL StubCreateDescriptorUpdateTemplate(VkDevice device,
                                                                         const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo,
                                                                         const VkAllocationCallbacks* pAllocator,
                                                                         VkDescriptorUpdateTemplate* pDescriptorUpdateTemplate) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubDestroyDescriptorUpdateTemplate(VkDevice device,
                                                                      VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                                      const VkAllocationCallbacks* pAllocator) {}
static VKAPI_ATTR void VKAPI_CALL StubUpdateDescriptorSetWithTemplate(VkDevice device, VkDescriptorSet descriptorSet,
                                                                      VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                                      const void* pData) {}
static VKAPI_ATTR void VKAPI_CALL StubGetPhysicalDeviceExternalBufferProperties(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalBufferInfo* pExternalBufferInfo,
    VkExternalBufferProperties* pExternalBufferProperties) {}
static VKAPI_ATTR void VKAPI_CALL StubGetPhysicalDeviceExternalFenceProperties(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalFenceInfo* pExternalFenceInfo,
    VkExternalFenceProperties* pExternalFenceProperties) {}
static VKAPI_ATTR void VKAPI_CALL StubGetPhysicalDeviceExternalSemaphoreProperties(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalSemaphoreInfo* pExternalSemaphoreInfo,
    VkExternalSemaphoreProperties* pExternalSemaphoreProperties) {}
static VKAPI_ATTR void VKAPI_CALL StubGetDescriptorSetLayoutSupport(VkDevice device,
                                                                    const VkDescriptorSetLayoutCreateInfo* pCreateInfo,
                                                                    VkDescriptorSetLayoutSupport* pSupport) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                           VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                           uint32_t maxDrawCount, uint32_t stride) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                  VkDeviceSize offset, VkBuffer countBuffer,
                                                                  VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                                  uint32_t stride) {}
static VKAPI_ATTR VkResult VKAPI_CALL StubCreateRenderPass2(VkDevice device, const VkRenderPassCreateInfo2* pCreateInfo,
                                                            const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubCmdBeginRenderPass2(VkCommandBuffer commandBuffer,
                                                          const VkRenderPassBeginInfo* pRenderPassBegin,
                                                          const VkSubpassBeginInfo* pSubpassBeginInfo) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdNextSubpass2(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo* pSubpassBeginInfo,
                                                      const VkSubpassEndInfo* pSubpassEndInfo) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdEndRenderPass2(VkCommandBuffer commandBuffer, const VkSubpassEndInfo* pSubpassEndInfo) {}
static VKAPI_ATTR void VKAPI_CALL StubResetQueryPool(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery,
                                                     uint32_t queryCount) {}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetSemaphoreCounterValue(VkDevice device, VkSemaphore semaphore, uint64_t* pValue) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubWaitSemaphores(VkDevice device, const VkSemaphoreWaitInfo* pWaitInfo, uint64_t timeout) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubSignalSemaphore(VkDevice device, const VkSemaphoreSignalInfo* pSignalInfo) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkDeviceAddress VKAPI_CALL StubGetBufferDeviceAddress(VkDevice device, const VkBufferDeviceAddressInfo* pInfo) {
    return 0;
}
static VKAPI_ATTR uint64_t VKAPI_CALL StubGetBufferOpaqueCaptureAddress(VkDevice device, const VkBufferDeviceAddressInfo* pInfo) {
    return 0;
}
static VKAPI_ATTR uint64_t VKAPI_CALL StubGetDeviceMemoryOpaqueCaptureAddress(VkDevice device,
                                                                              const VkDeviceMemoryOpaqueCaptureAddressInfo* pInfo) {
    return 0;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetPhysicalDeviceToolProperties(VkPhysicalDevice physicalDevice, uint32_t* pToolCount,
                                                                          VkPhysicalDeviceToolProperties* pToolProperties) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubCreatePrivateDataSlot(VkDevice device, const VkPrivateDataSlotCreateInfo* pCreateInfo,
                                                                const VkAllocationCallbacks* pAllocator,
                                                                VkPrivateDataSlot* pPrivateDataSlot) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubDestroyPrivateDataSlot(VkDevice device, VkPrivateDataSlot privateDataSlot,
                                                             const VkAllocationCallbacks* pAllocator) {}
static VKAPI_ATTR VkResult VKAPI_CALL StubSetPrivateData(VkDevice device, VkObjectType objectType, uint64_t objectHandle,
                                                         VkPrivateDataSlot privateDataSlot, uint64_t data) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubGetPrivateData(VkDevice device, VkObjectType objectType, uint64_t objectHandle,
                                                     VkPrivateDataSlot privateDataSlot, uint64_t* pData) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetEvent2(VkCommandBuffer commandBuffer, VkEvent event,
                                                   const VkDependencyInfo* pDependencyInfo) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdResetEvent2(VkCommandBuffer commandBuffer, VkEvent event,
                                                     VkPipelineStageFlags2 stageMask) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdWaitEvents2(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                                     const VkDependencyInfo* pDependencyInfos) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdPipelineBarrier2(VkCommandBuffer commandBuffer, const VkDependencyInfo* pDependencyInfo) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdWriteTimestamp2(VkCommandBuffer commandBuffer, VkPipelineStageFlags2 stage,
                                                         VkQueryPool queryPool, uint32_t query) {}
static VKAPI_ATTR VkResult VKAPI_CALL StubQueueSubmit2(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2* pSubmits,
                                                       VkFence fence) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubCmdCopyBuffer2(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2* pCopyBufferInfo) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdCopyImage2(VkCommandBuffer commandBuffer, const VkCopyImageInfo2* pCopyImageInfo) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdCopyBufferToImage2(VkCommandBuffer commandBuffer,
                                                            const VkCopyBufferToImageInfo2* pCopyBufferToImageInfo) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdCopyImageToBuffer2(VkCommandBuffer commandBuffer,
                                                            const VkCopyImageToBufferInfo2* pCopyImageToBufferInfo) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdBlitImage2(VkCommandBuffer commandBuffer, const VkBlitImageInfo2* pBlitImageInfo) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdResolveImage2(VkCommandBuffer commandBuffer,
                                                       const VkResolveImageInfo2* pResolveImageInfo) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdBeginRendering(VkCommandBuffer commandBuffer, const VkRenderingInfo* pRenderingInfo) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdEndRendering(VkCommandBuffer commandBuffer) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetCullMode(VkCommandBuffer commandBuffer, VkCullModeFlags cullMode) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetFrontFace(VkCommandBuffer commandBuffer, VkFrontFace frontFace) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetPrimitiveTopology(VkCommandBuffer commandBuffer,
                                                              VkPrimitiveTopology primitiveTopology) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetViewportWithCount(VkCommandBuffer commandBuffer, uint32_t viewportCount,
                                                              const VkViewport* pViewports) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetScissorWithCount(VkCommandBuffer commandBuffer, uint32_t scissorCount,
                                                             const VkRect2D* pScissors) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdBindVertexBuffers2(VkCommandBuffer commandBuffer, uint32_t firstBinding,
                                                            uint32_t bindingCount, const VkBuffer* pBuffers,
                                                            const VkDeviceSize* pOffsets, const VkDeviceSize* pSizes,
                                                            const VkDeviceSize* pStrides) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetDepthTestEnable(VkCommandBuffer commandBuffer, VkBool32 depthTestEnable) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetDepthWriteEnable(VkCommandBuffer commandBuffer, VkBool32 depthWriteEnable) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetDepthCompareOp(VkCommandBuffer commandBuffer, VkCompareOp depthCompareOp) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetDepthBoundsTestEnable(VkCommandBuffer commandBuffer, VkBool32 depthBoundsTestEnable) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetStencilTestEnable(VkCommandBuffer commandBuffer, VkBool32 stencilTestEnable) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetStencilOp(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask,
                                                      VkStencilOp failOp, VkStencilOp passOp, VkStencilOp depthFailOp,
                                                      VkCompareOp compareOp) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetRasterizerDiscardEnable(VkCommandBuffer commandBuffer,
                                                                    VkBool32 rasterizerDiscardEnable) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetDepthBiasEnable(VkCommandBuffer commandBuffer, VkBool32 depthBiasEnable) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetPrimitiveRestartEnable(VkCommandBuffer commandBuffer, VkBool32 primitiveRestartEnable) {
}
static VKAPI_ATTR void VKAPI_CALL StubGetDeviceBufferMemoryRequirements(VkDevice device,
                                                                        const VkDeviceBufferMemoryRequirements* pInfo,
                                                                        VkMemoryRequirements2* pMemoryRequirements) {}
static VKAPI_ATTR void VKAPI_CALL StubGetDeviceImageMemoryRequirements(VkDevice device,
                                                                       const VkDeviceImageMemoryRequirements* pInfo,
                                                                       VkMemoryRequirements2* pMemoryRequirements) {}
static VKAPI_ATTR void VKAPI_CALL StubGetDeviceImageSparseMemoryRequirements(
    VkDevice device, const VkDeviceImageMemoryRequirements* pInfo, uint32_t* pSparseMemoryRequirementCount,
    VkSparseImageMemoryRequirements2* pSparseMemoryRequirements) {}
static VKAPI_ATTR void VKAPI_CALL StubDestroySurfaceKHR(VkInstance instance, VkSurfaceKHR surface,
                                                        const VkAllocationCallbacks* pAllocator) {}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetPhysicalDeviceSurfaceSupportKHR(VkPhysicalDevice physicalDevice,
                                                                             uint32_t queueFamilyIndex, VkSurfaceKHR surface,
                                                                             VkBool32* pSupported) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetPhysicalDeviceSurfaceCapabilitiesKHR(VkPhysicalDevice physicalDevice,
                                                                                  VkSurfaceKHR surface,
                                                                                  VkSurfaceCapabilitiesKHR* pSurfaceCapabilities) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                             uint32_t* pSurfaceFormatCount,
                                                                             VkSurfaceFormatKHR* pSurfaceFormats) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice physicalDevice,
                                                                                  VkSurfaceKHR surface, uint32_t* pPresentModeCount,
                                                                                  VkPresentModeKHR* pPresentModes) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR* pCreateInfo,
                                                             const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubDestroySwapchainKHR(VkDevice device, VkSwapchainKHR swapchain,
                                                          const VkAllocationCallbacks* pAllocator) {}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapchain,
                                                                uint32_t* pSwapchainImageCount, VkImage* pSwapchainImages) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubAcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout,
                                                              VkSemaphore semaphore, VkFence fence, uint32_t* pImageIndex) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* pPresentInfo) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL
StubGetDeviceGroupPresentCapabilitiesKHR(VkDevice device, VkDeviceGroupPresentCapabilitiesKHR* pDeviceGroupPresentCapabilities) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetDeviceGroupSurfacePresentModesKHR(VkDevice device, VkSurfaceKHR surface,
                                                                               VkDeviceGroupPresentModeFlagsKHR* pModes) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetPhysicalDevicePresentRectanglesKHR(VkPhysicalDevice physicalDevice,
                                                                                VkSurfaceKHR surface, uint32_t* pRectCount,
                                                                                VkRect2D* pRects) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubAcquireNextImage2KHR(VkDevice device, const VkAcquireNextImageInfoKHR* pAcquireInfo,
                                                               uint32_t* pImageIndex) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetPhysicalDeviceDisplayPropertiesKHR(VkPhysicalDevice physicalDevice,
                                                                                uint32_t* pPropertyCount,
                                                                                VkDisplayPropertiesKHR* pProperties) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetPhysicalDeviceDisplayPlanePropertiesKHR(VkPhysicalDevice physicalDevice,
                                                                                     uint32_t* pPropertyCount,
                                                                                     VkDisplayPlanePropertiesKHR* pProperties) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetDisplayPlaneSupportedDisplaysKHR(VkPhysicalDevice physicalDevice, uint32_t planeIndex,
                                                                              uint32_t* pDisplayCount, VkDisplayKHR* pDisplays) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetDisplayModePropertiesKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display,
                                                                      uint32_t* pPropertyCount,
                                                                      VkDisplayModePropertiesKHR* pProperties) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubCreateDisplayModeKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display,
                                                               const VkDisplayModeCreateInfoKHR* pCreateInfo,
                                                               const VkAllocationCallbacks* pAllocator, VkDisplayModeKHR* pMode) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetDisplayPlaneCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkDisplayModeKHR mode,
                                                                         uint32_t planeIndex,
                                                                         VkDisplayPlaneCapabilitiesKHR* pCapabilities) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubCreateDisplayPlaneSurfaceKHR(VkInstance instance,
                                                                       const VkDisplaySurfaceCreateInfoKHR* pCreateInfo,
                                                                       const VkAllocationCallbacks* pAllocator,
                                                                       VkSurfaceKHR* pSurface) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubCreateSharedSwapchainsKHR(VkDevice device, uint32_t swapchainCount,
                                                                    const VkSwapchainCreateInfoKHR* pCreateInfos,
                                                                    const VkAllocationCallbacks* pAllocator,
                                                                    VkSwapchainKHR* pSwapchains) {
    return VK_SUCCESS;
}
#ifdef VK_USE_PLATFORM_XLIB_KHR
static VKAPI_ATTR VkResult VKAPI_CALL StubCreateXlibSurfaceKHR(VkInstance instance, const VkXlibSurfaceCreateInfoKHR* pCreateInfo,
                                                               const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    return VK_SUCCESS;
}
#endif  // VK_USE_PLATFORM_XLIB_KHR
#ifdef VK_USE_PLATFORM_XLIB_KHR
static VKAPI_ATTR VkBool32 VKAPI_CALL StubGetPhysicalDeviceXlibPresentationSupportKHR(VkPhysicalDevice physicalDevice,
                                                                                      uint32_t queueFamilyIndex, Display* dpy,
                                                                                      VisualID visualID) {
    return VK_FALSE;
}
#endif  // VK_USE_PLATFORM_XLIB_KHR
#ifdef VK_USE_PLATFORM_XCB_KHR
static VKAPI_ATTR VkResult VKAPI_CALL StubCreateXcbSurfaceKHR(VkInstance instance, const VkXcbSurfaceCreateInfoKHR* pCreateInfo,
                                                              const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    return VK_SUCCESS;
}
#endif  // VK_USE_PLATFORM_XCB_KHR
#ifdef VK_USE_PLATFORM_XCB_KHR
static VKAPI_ATTR VkBool32 VKAPI_CALL StubGetPhysicalDeviceXcbPresentationSupportKHR(VkPhysicalDevice physicalDevice,
                                                                                     uint32_t queueFamilyIndex,
                                                                                     xcb_connection_t* connection,
                                                                                     xcb_visualid_t visual_id) {
    return VK_FALSE;
}
#endif  // VK_USE_PLATFORM_XCB_KHR
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
static VKAPI_ATTR VkResult VKAPI_CALL StubCreateWaylandSurfaceKHR(VkInstance instance,
                                                                  const VkWaylandSurfaceCreateInfoKHR* pCreateInfo,
                                                                  const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    return VK_SUCCESS;
}
#endif  // VK_USE_PLATFORM_WAYLAND_KHR
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
static VKAPI_ATTR VkBool32 VKAPI_CALL StubGetPhysicalDeviceWaylandPresentationSupportKHR(VkPhysicalDevice physicalDevice,
                                                                                         uint32_t queueFamilyIndex,
                                                                                         struct wl_display* display) {
    return VK_FALSE;
}
#endif  // VK_USE_PLATFORM_WAYLAND_KHR
#ifdef VK_USE_PLATFORM_ANDROID_KHR
static VKAPI_ATTR VkResult VKAPI_CALL StubCreateAndroidSurfaceKHR(VkInstance instance,
                                                                  const VkAndroidSurfaceCreateInfoKHR* pCreateInfo,
                                                                  const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    return VK_SUCCESS;
}
#endif  // VK_USE_PLATFORM_ANDROID_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
static VKAPI_ATTR VkResult VKAPI_CALL StubCreateWin32SurfaceKHR(VkInstance instance, const VkWin32SurfaceCreateInfoKHR* pCreateInfo,
                                                                const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    return VK_SUCCESS;
}
#endif  // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
static VKAPI_ATTR VkBool32 VKAPI_CALL StubGetPhysicalDeviceWin32PresentationSupportKHR(VkPhysicalDevice physicalDevice,
                                                                                       uint32_t queueFamilyIndex) {
    return VK_FALSE;
}
#endif  // VK_USE_PLATFORM_WIN32_KHR
static VKAPI_ATTR VkResult VKAPI_CALL StubGetPhysicalDeviceVideoCapabilitiesKHR(VkPhysicalDevice physicalDevice,
                                                                                const VkVideoProfileInfoKHR* pVideoProfile,
                                                                                VkVideoCapabilitiesKHR* pCapabilities) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetPhysicalDeviceVideoFormatPropertiesKHR(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceVideoFormatInfoKHR* pVideoFormatInfo,
    uint32_t* pVideoFormatPropertyCount, VkVideoFormatPropertiesKHR* pVideoFormatProperties) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubCreateVideoSessionKHR(VkDevice device, const VkVideoSessionCreateInfoKHR* pCreateInfo,
                                                                const VkAllocationCallbacks* pAllocator,
                                                                VkVideoSessionKHR* pVideoSession) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubDestroyVideoSessionKHR(VkDevice device, VkVideoSessionKHR videoSession,
                                                             const VkAllocationCallbacks* pAllocator) {}
static VKAPI_ATTR VkResult VKAPI_CALL
StubGetVideoSessionMemoryRequirementsKHR(VkDevice device, VkVideoSessionKHR videoSession, uint32_t* pMemoryRequirementsCount,
                                         VkVideoSessionMemoryRequirementsKHR* pMemoryRequirements) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL
StubBindVideoSessionMemoryKHR(VkDevice device, VkVideoSessionKHR videoSession, uint32_t bindSessionMemoryInfoCount,
                              const VkBindVideoSessionMemoryInfoKHR* pBindSessionMemoryInfos) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubCreateVideoSessionParametersKHR(VkDevice device,
                                                                          const VkVideoSessionParametersCreateInfoKHR* pCreateInfo,
                                                                          const VkAllocationCallbacks* pAllocator,
                                                                          VkVideoSessionParametersKHR* pVideoSessionParameters) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubUpdateVideoSessionParametersKHR(
    VkDevice device, VkVideoSessionParametersKHR videoSessionParameters, const VkVideoSessionParametersUpdateInfoKHR* pUpdateInfo) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubDestroyVideoSessionParametersKHR(VkDevice device,
                                                                       VkVideoSessionParametersKHR videoSessionParameters,
                                                                       const VkAllocationCallbacks* pAllocator) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdBeginVideoCodingKHR(VkCommandBuffer commandBuffer,
                                                             const VkVideoBeginCodingInfoKHR* pBeginInfo) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdEndVideoCodingKHR(VkCommandBuffer commandBuffer,
                                                           const VkVideoEndCodingInfoKHR* pEndCodingInfo) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdControlVideoCodingKHR(VkCommandBuffer commandBuffer,
                                                               const VkVideoCodingControlInfoKHR* pCodingControlInfo) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdDecodeVideoKHR(VkCommandBuffer commandBuffer, const VkVideoDecodeInfoKHR* pDecodeInfo) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdBeginRenderingKHR(VkCommandBuffer commandBuffer, const VkRenderingInfo* pRenderingInfo) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdEndRenderingKHR(VkCommandBuffer commandBuffer) {}
static VKAPI_ATTR void VKAPI_CALL StubGetPhysicalDeviceFeatures2KHR(VkPhysicalDevice physicalDevice,
                                                                    VkPhysicalDeviceFeatures2* pFeatures) {}
static VKAPI_ATTR void VKAPI_CALL StubGetPhysicalDeviceProperties2KHR(VkPhysicalDevice physicalDevice,
                                                                      VkPhysicalDeviceProperties2* pProperties) {}
static VKAPI_ATTR void VKAPI_CALL StubGetPhysicalDeviceFormatProperties2KHR(VkPhysicalDevice physicalDevice, VkFormat format,
                                                                            VkFormatProperties2* pFormatProperties) {}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetPhysicalDeviceImageFormatProperties2KHR(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceImageFormatInfo2* pImageFormatInfo,
    VkImageFormatProperties2* pImageFormatProperties) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubGetPhysicalDeviceQueueFamilyProperties2KHR(VkPhysicalDevice physicalDevice,
                                                                                 uint32_t* pQueueFamilyPropertyCount,
                                                                                 VkQueueFamilyProperties2* pQueueFamilyProperties) {
}
static VKAPI_ATTR void VKAPI_CALL StubGetPhysicalDeviceMemoryProperties2KHR(VkPhysicalDevice physicalDevice,
                                                                            VkPhysicalDeviceMemoryProperties2* pMemoryProperties) {}
static VKAPI_ATTR void VKAPI_CALL StubGetPhysicalDeviceSparseImageFormatProperties2KHR(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSparseImageFormatInfo2* pFormatInfo, uint32_t* pPropertyCount,
    VkSparseImageFormatProperties2* pProperties) {}
static VKAPI_ATTR void VKAPI_CALL StubGetDeviceGroupPeerMemoryFeaturesKHR(VkDevice device, uint32_t heapIndex,
                                                                          uint32_t localDeviceIndex, uint32_t remoteDeviceIndex,
                                                                          VkPeerMemoryFeatureFlags* pPeerMemoryFeatures) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetDeviceMaskKHR(VkCommandBuffer commandBuffer, uint32_t deviceMask) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdDispatchBaseKHR(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY,
                                                         uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY,
                                                         uint32_t groupCountZ) {}
static VKAPI_ATTR void VKAPI_CALL StubTrimCommandPoolKHR(VkDevice device, VkCommandPool commandPool, VkCommandPoolTrimFlags flags) {
}
static VKAPI_ATTR VkResult VKAPI_CALL StubEnumeratePhysicalDeviceGroupsKHR(
    VkInstance instance, uint32_t* pPhysicalDeviceGroupCount, VkPhysicalDeviceGroupProperties* pPhysicalDeviceGroupProperties) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubGetPhysicalDeviceExternalBufferPropertiesKHR(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalBufferInfo* pExternalBufferInfo,
    VkExternalBufferProperties* pExternalBufferProperties) {}
#ifdef VK_USE_PLATFORM_WIN32_KHR
static VKAPI_ATTR VkResult VKAPI_CALL StubGetMemoryWin32HandleKHR(VkDevice device,
                                                                  const VkMemoryGetWin32HandleInfoKHR* pGetWin32HandleInfo,
                                                                  HANDLE* pHandle) {
    return VK_SUCCESS;
}
#endif  // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
static VKAPI_ATTR VkResult VKAPI_CALL
StubGetMemoryWin32HandlePropertiesKHR(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType, HANDLE handle,
                                      VkMemoryWin32HandlePropertiesKHR* pMemoryWin32HandleProperties) {
    return VK_SUCCESS;
}
#endif  // VK_USE_PLATFORM_WIN32_KHR
static VKAPI_ATTR VkResult VKAPI_CALL StubGetMemoryFdKHR(VkDevice device, const VkMemoryGetFdInfoKHR* pGetFdInfo, int* pFd) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetMemoryFdPropertiesKHR(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType,
                                                                   int fd, VkMemoryFdPropertiesKHR* pMemoryFdProperties) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubGetPhysicalDeviceExternalSemaphorePropertiesKHR(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalSemaphoreInfo* pExternalSemaphoreInfo,
    VkExternalSemaphoreProperties* pExternalSemaphoreProperties) {}
#ifdef VK_USE_PLATFORM_WIN32_KHR
static VKAPI_ATTR VkResult VKAPI_CALL
StubImportSemaphoreWin32HandleKHR(VkDevice device, const VkImportSemaphoreWin32HandleInfoKHR* pImportSemaphoreWin32HandleInfo) {
    return VK_SUCCESS;
}
#endif  // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
static VKAPI_ATTR VkResult VKAPI_CALL StubGetSemaphoreWin32HandleKHR(VkDevice device,
                                                                     const VkSemaphoreGetWin32HandleInfoKHR* pGetWin32HandleInfo,
                                                                     HANDLE* pHandle) {
    return VK_SUCCESS;
}
#endif  // VK_USE_PLATFORM_WIN32_KHR
static VKAPI_ATTR VkResult VKAPI_CALL StubImportSemaphoreFdKHR(VkDevice device,
                                                               const VkImportSemaphoreFdInfoKHR* pImportSemaphoreFdInfo) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetSemaphoreFdKHR(VkDevice device, const VkSemaphoreGetFdInfoKHR* pGetFdInfo, int* pFd) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubCmdPushDescriptorSetKHR(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                              VkPipelineLayout layout, uint32_t set, uint32_t descriptorWriteCount,
                                                              const VkWriteDescriptorSet* pDescriptorWrites) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdPushDescriptorSetWithTemplateKHR(VkCommandBuffer commandBuffer,
                                                                          VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                                          VkPipelineLayout layout, uint32_t set,
                                                                          const void* pData) {}
static VKAPI_ATTR VkResult VKAPI_CALL StubCreateDescriptorUpdateTemplateKHR(VkDevice device,
                                                                            const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo,
                                                                            const VkAllocationCallbacks* pAllocator,
                                                                            VkDescriptorUpdateTemplate* pDescriptorUpdateTemplate) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubDestroyDescriptorUpdateTemplateKHR(VkDevice device,
                                                                         VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                                         const VkAllocationCallbacks* pAllocator) {}
static VKAPI_ATTR void VKAPI_CALL StubUpdateDescriptorSetWithTemplateKHR(VkDevice device, VkDescriptorSet descriptorSet,
                                                                         VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                                         const void* pData) {}
static VKAPI_ATTR VkResult VKAPI_CALL StubCreateRenderPass2KHR(VkDevice device, const VkRenderPassCreateInfo2* pCreateInfo,
                                                               const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubCmdBeginRenderPass2KHR(VkCommandBuffer commandBuffer,
                                                             const VkRenderPassBeginInfo* pRenderPassBegin,
                                                             const VkSubpassBeginInfo* pSubpassBeginInfo) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdNextSubpass2KHR(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo* pSubpassBeginInfo,
                                                         const VkSubpassEndInfo* pSubpassEndInfo) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdEndRenderPass2KHR(VkCommandBuffer commandBuffer, const VkSubpassEndInfo* pSubpassEndInfo) {
}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetSwapchainStatusKHR(VkDevice device, VkSwapchainKHR swapchain) { return VK_SUCCESS; }
static VKAPI_ATTR void VKAPI_CALL StubGetPhysicalDeviceExternalFencePropertiesKHR(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalFenceInfo* pExternalFenceInfo,
    VkExternalFenceProperties* pExternalFenceProperties) {}
#ifdef VK_USE_PLATFORM_WIN32_KHR
static VKAPI_ATTR VkResult VKAPI_CALL
StubImportFenceWin32HandleKHR(VkDevice device, const VkImportFenceWin32HandleInfoKHR* pImportFenceWin32HandleInfo) {
    return VK_SUCCESS;
}
#endif  // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
static VKAPI_ATTR VkResult VKAPI_CALL StubGetFenceWin32HandleKHR(VkDevice device,
                                                                 const VkFenceGetWin32HandleInfoKHR* pGetWin32HandleInfo,
                                                                 HANDLE* pHandle) {
    return VK_SUCCESS;
}
#endif  // VK_USE_PLATFORM_WIN32_KHR
static VKAPI_ATTR VkResult VKAPI_CALL StubImportFenceFdKHR(VkDevice device, const VkImportFenceFdInfoKHR* pImportFenceFdInfo) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetFenceFdKHR(VkDevice device, const VkFenceGetFdInfoKHR* pGetFdInfo, int* pFd) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(
    VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, uint32_t* pCounterCount, VkPerformanceCounterKHR* pCounters,
    VkPerformanceCounterDescriptionKHR* pCounterDescriptions) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR(
    VkPhysicalDevice physicalDevice, const VkQueryPoolPerformanceCreateInfoKHR* pPerformanceQueryCreateInfo, uint32_t* pNumPasses) {
}
static VKAPI_ATTR VkResult VKAPI_CALL StubAcquireProfilingLockKHR(VkDevice device, const VkAcquireProfilingLockInfoKHR* pInfo) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubReleaseProfilingLockKHR(VkDevice device) {}
static VKAPI_ATTR VkResult VKAPI_CALL
StubGetPhysicalDeviceSurfaceCapabilities2KHR(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo,
                                             VkSurfaceCapabilities2KHR* pSurfaceCapabilities) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetPhysicalDeviceSurfaceFormats2KHR(VkPhysicalDevice physicalDevice,
                                                                              const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo,
                                                                              uint32_t* pSurfaceFormatCount,
                                                                              VkSurfaceFormat2KHR* pSurfaceFormats) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetPhysicalDeviceDisplayProperties2KHR(VkPhysicalDevice physicalDevice,
                                                                                 uint32_t* pPropertyCount,
                                                                                 VkDisplayProperties2KHR* pProperties) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetPhysicalDeviceDisplayPlaneProperties2KHR(VkPhysicalDevice physicalDevice,
                                                                                      uint32_t* pPropertyCount,
                                                                                      VkDisplayPlaneProperties2KHR* pProperties) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetDisplayModeProperties2KHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display,
                                                                       uint32_t* pPropertyCount,
                                                                       VkDisplayModeProperties2KHR* pProperties) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetDisplayPlaneCapabilities2KHR(VkPhysicalDevice physicalDevice,
                                                                          const VkDisplayPlaneInfo2KHR* pDisplayPlaneInfo,
                                                                          VkDisplayPlaneCapabilities2KHR* pCapabilities) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubGetImageMemoryRequirements2KHR(VkDevice device, const VkImageMemoryRequirementsInfo2* pInfo,
                                                                     VkMemoryRequirements2* pMemoryRequirements) {}
static VKAPI_ATTR void VKAPI_CALL StubGetBufferMemoryRequirements2KHR(VkDevice device, const VkBufferMemoryRequirementsInfo2* pInfo,
                                                                      VkMemoryRequirements2* pMemoryRequirements) {}
static VKAPI_ATTR void VKAPI_CALL StubGetImageSparseMemoryRequirements2KHR(
    VkDevice device, const VkImageSparseMemoryRequirementsInfo2* pInfo, uint32_t* pSparseMemoryRequirementCount,
    VkSparseImageMemoryRequirements2* pSparseMemoryRequirements) {}
static VKAPI_ATTR VkResult VKAPI_CALL StubCreateSamplerYcbcrConversionKHR(VkDevice device,
                                                                          const VkSamplerYcbcrConversionCreateInfo* pCreateInfo,
                                                                          const VkAllocationCallbacks* pAllocator,
                                                                          VkSamplerYcbcrConversion* pYcbcrConversion) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubDestroySamplerYcbcrConversionKHR(VkDevice device, VkSamplerYcbcrConversion ycbcrConversion,
                                                                       const VkAllocationCallbacks* pAllocator) {}
static VKAPI_ATTR VkResult VKAPI_CALL StubBindBufferMemory2KHR(VkDevice device, uint32_t bindInfoCount,
                                                               const VkBindBufferMemoryInfo* pBindInfos) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubBindImageMemory2KHR(VkDevice device, uint32_t bindInfoCount,
                                                              const VkBindImageMemoryInfo* pBindInfos) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubGetDescriptorSetLayoutSupportKHR(VkDevice device,
                                                                       const VkDescriptorSetLayoutCreateInfo* pCreateInfo,
                                                                       VkDescriptorSetLayoutSupport* pSupport) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdDrawIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                              VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                              uint32_t maxDrawCount, uint32_t stride) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                     VkDeviceSize offset, VkBuffer countBuffer,
                                                                     VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                                     uint32_t stride) {}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetSemaphoreCounterValueKHR(VkDevice device, VkSemaphore semaphore, uint64_t* pValue) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubWaitSemaphoresKHR(VkDevice device, const VkSemaphoreWaitInfo* pWaitInfo,
                                                            uint64_t timeout) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubSignalSemaphoreKHR(VkDevice device, const VkSemaphoreSignalInfo* pSignalInfo) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL
StubGetPhysicalDeviceFragmentShadingRatesKHR(VkPhysicalDevice physicalDevice, uint32_t* pFragmentShadingRateCount,
                                             VkPhysicalDeviceFragmentShadingRateKHR* pFragmentShadingRates) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetFragmentShadingRateKHR(VkCommandBuffer commandBuffer, const VkExtent2D* pFragmentSize,
                                                                   const VkFragmentShadingRateCombinerOpKHR combinerOps[2]) {}
static VKAPI_ATTR VkResult VKAPI_CALL StubWaitForPresentKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t presentId,
                                                            uint64_t timeout) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkDeviceAddress VKAPI_CALL StubGetBufferDeviceAddressKHR(VkDevice device,
                                                                           const VkBufferDeviceAddressInfo* pInfo) {
    return 0;
}
static VKAPI_ATTR uint64_t VKAPI_CALL StubGetBufferOpaqueCaptureAddressKHR(VkDevice device,
                                                                           const VkBufferDeviceAddressInfo* pInfo) {
    return 0;
}
static VKAPI_ATTR uint64_t VKAPI_CALL
StubGetDeviceMemoryOpaqueCaptureAddressKHR(VkDevice device, const VkDeviceMemoryOpaqueCaptureAddressInfo* pInfo) {
    return 0;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubCreateDeferredOperationKHR(VkDevice device, const VkAllocationCallbacks* pAllocator,
                                                                     VkDeferredOperationKHR* pDeferredOperation) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubDestroyDeferredOperationKHR(VkDevice device, VkDeferredOperationKHR operation,
                                                                  const VkAllocationCallbacks* pAllocator) {}
static VKAPI_ATTR uint32_t VKAPI_CALL StubGetDeferredOperationMaxConcurrencyKHR(VkDevice device, VkDeferredOperationKHR operation) {
    return 0;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetDeferredOperationResultKHR(VkDevice device, VkDeferredOperationKHR operation) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubDeferredOperationJoinKHR(VkDevice device, VkDeferredOperationKHR operation) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetPipelineExecutablePropertiesKHR(VkDevice device,
                                                                             const VkPipelineInfoKHR* pPipelineInfo,
                                                                             uint32_t* pExecutableCount,
                                                                             VkPipelineExecutablePropertiesKHR* pProperties) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetPipelineExecutableStatisticsKHR(VkDevice device,
                                                                             const VkPipelineExecutableInfoKHR* pExecutableInfo,
                                                                             uint32_t* pStatisticCount,
                                                                             VkPipelineExecutableStatisticKHR* pStatistics) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetPipelineExecutableInternalRepresentationsKHR(
    VkDevice device, const VkPipelineExecutableInfoKHR* pExecutableInfo, uint32_t* pInternalRepresentationCount,
    VkPipelineExecutableInternalRepresentationKHR* pInternalRepresentations) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubMapMemory2KHR(VkDevice device, const VkMemoryMapInfoKHR* pMemoryMapInfo, void** ppData) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubUnmapMemory2KHR(VkDevice device, const VkMemoryUnmapInfoKHR* pMemoryUnmapInfo) {
    return VK_SUCCESS;
}
#ifdef VK_ENABLE_BETA_EXTENSIONS
static VKAPI_ATTR VkResult VKAPI_CALL StubGetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceVideoEncodeQualityLevelInfoKHR* pQualityLevelInfo,
    VkVideoEncodeQualityLevelPropertiesKHR* pQualityLevelProperties) {
    return VK_SUCCESS;
}
#endif  // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
static VKAPI_ATTR VkResult VKAPI_CALL StubGetEncodedVideoSessionParametersKHR(
    VkDevice device, const VkVideoEncodeSessionParametersGetInfoKHR* pVideoSessionParametersInfo,
    VkVideoEncodeSessionParametersFeedbackInfoKHR* pFeedbackInfo, size_t* pDataSize, void* pData) {
    return VK_SUCCESS;
}
#endif  // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
static VKAPI_ATTR void VKAPI_CALL StubCmdEncodeVideoKHR(VkCommandBuffer commandBuffer, const VkVideoEncodeInfoKHR* pEncodeInfo) {}
#endif  // VK_ENABLE_BETA_EXTENSIONS
static VKAPI_ATTR void VKAPI_CALL StubCmdSetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event,
                                                      const VkDependencyInfo* pDependencyInfo) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdResetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event,
                                                        VkPipelineStageFlags2 stageMask) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdWaitEvents2KHR(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                                        const VkDependencyInfo* pDependencyInfos) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdPipelineBarrier2KHR(VkCommandBuffer commandBuffer,
                                                             const VkDependencyInfo* pDependencyInfo) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdWriteTimestamp2KHR(VkCommandBuffer commandBuffer, VkPipelineStageFlags2 stage,
                                                            VkQueryPool queryPool, uint32_t query) {}
static VKAPI_ATTR VkResult VKAPI_CALL StubQueueSubmit2KHR(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2* pSubmits,
                                                          VkFence fence) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubCmdWriteBufferMarker2AMD(VkCommandBuffer commandBuffer, VkPipelineStageFlags2 stage,
                                                               VkBuffer dstBuffer, VkDeviceSize dstOffset, uint32_t marker) {}
static VKAPI_ATTR void VKAPI_CALL StubGetQueueCheckpointData2NV(VkQueue queue, uint32_t* pCheckpointDataCount,
                                                                VkCheckpointData2NV* pCheckpointData) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdCopyBuffer2KHR(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2* pCopyBufferInfo) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdCopyImage2KHR(VkCommandBuffer commandBuffer, const VkCopyImageInfo2* pCopyImageInfo) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdCopyBufferToImage2KHR(VkCommandBuffer commandBuffer,
                                                               const VkCopyBufferToImageInfo2* pCopyBufferToImageInfo) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdCopyImageToBuffer2KHR(VkCommandBuffer commandBuffer,
                                                               const VkCopyImageToBufferInfo2* pCopyImageToBufferInfo) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdBlitImage2KHR(VkCommandBuffer commandBuffer, const VkBlitImageInfo2* pBlitImageInfo) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdResolveImage2KHR(VkCommandBuffer commandBuffer,
                                                          const VkResolveImageInfo2* pResolveImageInfo) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdTraceRaysIndirect2KHR(VkCommandBuffer commandBuffer,
                                                               VkDeviceAddress indirectDeviceAddress) {}
static VKAPI_ATTR void VKAPI_CALL StubGetDeviceBufferMemoryRequirementsKHR(VkDevice device,
                                                                           const VkDeviceBufferMemoryRequirements* pInfo,
                                                                           VkMemoryRequirements2* pMemoryRequirements) {}
static VKAPI_ATTR void VKAPI_CALL StubGetDeviceImageMemoryRequirementsKHR(VkDevice device,
                                                                          const VkDeviceImageMemoryRequirements* pInfo,
                                                                          VkMemoryRequirements2* pMemoryRequirements) {}
static VKAPI_ATTR void VKAPI_CALL StubGetDeviceImageSparseMemoryRequirementsKHR(
    VkDevice device, const VkDeviceImageMemoryRequirements* pInfo, uint32_t* pSparseMemoryRequirementCount,
    VkSparseImageMemoryRequirements2* pSparseMemoryRequirements) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdBindIndexBuffer2KHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                             VkDeviceSize size, VkIndexType indexType) {}
static VKAPI_ATTR void VKAPI_CALL StubGetRenderingAreaGranularityKHR(VkDevice device,
                                                                     const VkRenderingAreaInfoKHR* pRenderingAreaInfo,
                                                                     VkExtent2D* pGranularity) {}
static VKAPI_ATTR void VKAPI_CALL StubGetDeviceImageSubresourceLayoutKHR(VkDevice device,
                                                                         const VkDeviceImageSubresourceInfoKHR* pInfo,
                                                                         VkSubresourceLayout2KHR* pLayout) {}
static VKAPI_ATTR void VKAPI_CALL StubGetImageSubresourceLayout2KHR(VkDevice device, VkImage image,
                                                                    const VkImageSubresource2KHR* pSubresource,
                                                                    VkSubresourceLayout2KHR* pLayout) {}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetPhysicalDeviceCooperativeMatrixPropertiesKHR(
    VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount, VkCooperativeMatrixPropertiesKHR* pProperties) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubCreateDebugReportCallbackEXT(VkInstance instance,
                                                                       const VkDebugReportCallbackCreateInfoEXT* pCreateInfo,
                                                                       const VkAllocationCallbacks* pAllocator,
                                                                       VkDebugReportCallbackEXT* pCallback) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubDestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback,
                                                                    const VkAllocationCallbacks* pAllocator) {}
static VKAPI_ATTR void VKAPI_CALL StubDebugReportMessageEXT(VkInstance instance, VkDebugReportFlagsEXT flags,
                                                            VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location,
                                                            int32_t messageCode, const char* pLayerPrefix, const char* pMessage) {}
static VKAPI_ATTR VkResult VKAPI_CALL StubDebugMarkerSetObjectTagEXT(VkDevice device,
                                                                     const VkDebugMarkerObjectTagInfoEXT* pTagInfo) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubDebugMarkerSetObjectNameEXT(VkDevice device,
                                                                      const VkDebugMarkerObjectNameInfoEXT* pNameInfo) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubCmdDebugMarkerBeginEXT(VkCommandBuffer commandBuffer,
                                                             const VkDebugMarkerMarkerInfoEXT* pMarkerInfo) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdDebugMarkerEndEXT(VkCommandBuffer commandBuffer) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdDebugMarkerInsertEXT(VkCommandBuffer commandBuffer,
                                                              const VkDebugMarkerMarkerInfoEXT* pMarkerInfo) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdBindTransformFeedbackBuffersEXT(VkCommandBuffer commandBuffer, uint32_t firstBinding,
                                                                         uint32_t bindingCount, const VkBuffer* pBuffers,
                                                                         const VkDeviceSize* pOffsets, const VkDeviceSize* pSizes) {
}
static VKAPI_ATTR void VKAPI_CALL StubCmdBeginTransformFeedbackEXT(VkCommandBuffer commandBuffer, uint32_t firstCounterBuffer,
                                                                   uint32_t counterBufferCount, const VkBuffer* pCounterBuffers,
                                                                   const VkDeviceSize* pCounterBufferOffsets) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdEndTransformFeedbackEXT(VkCommandBuffer commandBuffer, uint32_t firstCounterBuffer,
                                                                 uint32_t counterBufferCount, const VkBuffer* pCounterBuffers,
                                                                 const VkDeviceSize* pCounterBufferOffsets) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdBeginQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query,
                                                              VkQueryControlFlags flags, uint32_t index) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdEndQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query,
                                                            uint32_t index) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdDrawIndirectByteCountEXT(VkCommandBuffer commandBuffer, uint32_t instanceCount,
                                                                  uint32_t firstInstance, VkBuffer counterBuffer,
                                                                  VkDeviceSize counterBufferOffset, uint32_t counterOffset,
                                                                  uint32_t vertexStride) {}
static VKAPI_ATTR VkResult VKAPI_CALL StubCreateCuModuleNVX(VkDevice device, const VkCuModuleCreateInfoNVX* pCreateInfo,
                                                            const VkAllocationCallbacks* pAllocator, VkCuModuleNVX* pModule) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubCreateCuFunctionNVX(VkDevice device, const VkCuFunctionCreateInfoNVX* pCreateInfo,
                                                              const VkAllocationCallbacks* pAllocator, VkCuFunctionNVX* pFunction) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubDestroyCuModuleNVX(VkDevice device, VkCuModuleNVX module,
                                                         const VkAllocationCallbacks* pAllocator) {}
static VKAPI_ATTR void VKAPI_CALL StubDestroyCuFunctionNVX(VkDevice device, VkCuFunctionNVX function,
                                                           const VkAllocationCallbacks* pAllocator) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdCuLaunchKernelNVX(VkCommandBuffer commandBuffer, const VkCuLaunchInfoNVX* pLaunchInfo) {}
static VKAPI_ATTR uint32_t VKAPI_CALL StubGetImageViewHandleNVX(VkDevice device, const VkImageViewHandleInfoNVX* pInfo) {
    return 0;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetImageViewAddressNVX(VkDevice device, VkImageView imageView,
                                                                 VkImageViewAddressPropertiesNVX* pProperties) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubCmdDrawIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                              VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                              uint32_t maxDrawCount, uint32_t stride) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdDrawIndexedIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                     VkDeviceSize offset, VkBuffer countBuffer,
                                                                     VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                                     uint32_t stride) {}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetShaderInfoAMD(VkDevice device, VkPipeline pipeline, VkShaderStageFlagBits shaderStage,
                                                           VkShaderInfoTypeAMD infoType, size_t* pInfoSize, void* pInfo) {
    return VK_SUCCESS;
}
#ifdef VK_USE_PLATFORM_GGP
static VKAPI_ATTR VkResult VKAPI_CALL
StubCreateStreamDescriptorSurfaceGGP(VkInstance instance, const VkStreamDescriptorSurfaceCreateInfoGGP* pCreateInfo,
                                     const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    return VK_SUCCESS;
}
#endif  // VK_USE_PLATFORM_GGP
static VKAPI_ATTR VkResult VKAPI_CALL StubGetPhysicalDeviceExternalImageFormatPropertiesNV(
    VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type, VkImageTiling tiling, VkImageUsageFlags usage,
    VkImageCreateFlags flags, VkExternalMemoryHandleTypeFlagsNV externalHandleType,
    VkExternalImageFormatPropertiesNV* pExternalImageFormatProperties) {
    return VK_SUCCESS;
}
#ifdef VK_USE_PLATFORM_WIN32_KHR
static VKAPI_ATTR VkResult VKAPI_CALL StubGetMemoryWin32HandleNV(VkDevice device, VkDeviceMemory memory,
                                                                 VkExternalMemoryHandleTypeFlagsNV handleType, HANDLE* pHandle) {
    return VK_SUCCESS;
}
#endif  // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_VI_NN
static VKAPI_ATTR VkResult VKAPI_CALL StubCreateViSurfaceNN(VkInstance instance, const VkViSurfaceCreateInfoNN* pCreateInfo,
                                                            const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    return VK_SUCCESS;
}
#endif  // VK_USE_PLATFORM_VI_NN
static VKAPI_ATTR void VKAPI_CALL StubCmdBeginConditionalRenderingEXT(
    VkCommandBuffer commandBuffer, const VkConditionalRenderingBeginInfoEXT* pConditionalRenderingBegin) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdEndConditionalRenderingEXT(VkCommandBuffer commandBuffer) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetViewportWScalingNV(VkCommandBuffer commandBuffer, uint32_t firstViewport,
                                                               uint32_t viewportCount,
                                                               const VkViewportWScalingNV* pViewportWScalings) {}
static VKAPI_ATTR VkResult VKAPI_CALL StubReleaseDisplayEXT(VkPhysicalDevice physicalDevice, VkDisplayKHR display) {
    return VK_SUCCESS;
}
#ifdef VK_USE_PLATFORM_XLIB_XRANDR_EXT
static VKAPI_ATTR VkResult VKAPI_CALL StubAcquireXlibDisplayEXT(VkPhysicalDevice physicalDevice, Display* dpy,
                                                                VkDisplayKHR display) {
    return VK_SUCCESS;
}
#endif  // VK_USE_PLATFORM_XLIB_XRANDR_EXT
#ifdef VK_USE_PLATFORM_XLIB_XRANDR_EXT
static VKAPI_ATTR VkResult VKAPI_CALL StubGetRandROutputDisplayEXT(VkPhysicalDevice physicalDevice, Display* dpy, RROutput rrOutput,
                                                                   VkDisplayKHR* pDisplay) {
    return VK_SUCCESS;
}
#endif  // VK_USE_PLATFORM_XLIB_XRANDR_EXT
static VKAPI_ATTR VkResult VKAPI_CALL StubGetPhysicalDeviceSurfaceCapabilities2EXT(
    VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkSurfaceCapabilities2EXT* pSurfaceCapabilities) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubDisplayPowerControlEXT(VkDevice device, VkDisplayKHR display,
                                                                 const VkDisplayPowerInfoEXT* pDisplayPowerInfo) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubRegisterDeviceEventEXT(VkDevice device, const VkDeviceEventInfoEXT* pDeviceEventInfo,
                                                                 const VkAllocationCallbacks* pAllocator, VkFence* pFence) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubRegisterDisplayEventEXT(VkDevice device, VkDisplayKHR display,
                                                                  const VkDisplayEventInfoEXT* pDisplayEventInfo,
                                                                  const VkAllocationCallbacks* pAllocator, VkFence* pFence) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetSwapchainCounterEXT(VkDevice device, VkSwapchainKHR swapchain,
                                                                 VkSurfaceCounterFlagBitsEXT counter, uint64_t* pCounterValue) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetRefreshCycleDurationGOOGLE(VkDevice device, VkSwapchainKHR swapchain,
                                                                        VkRefreshCycleDurationGOOGLE* pDisplayTimingProperties) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetPastPresentationTimingGOOGLE(VkDevice device, VkSwapchainKHR swapchain,
                                                                          uint32_t* pPresentationTimingCount,
                                                                          VkPastPresentationTimingGOOGLE* pPresentationTimings) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetDiscardRectangleEXT(VkCommandBuffer commandBuffer, uint32_t firstDiscardRectangle,
                                                                uint32_t discardRectangleCount,
                                                                const VkRect2D* pDiscardRectangles) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetDiscardRectangleEnableEXT(VkCommandBuffer commandBuffer,
                                                                      VkBool32 discardRectangleEnable) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetDiscardRectangleModeEXT(VkCommandBuffer commandBuffer,
                                                                    VkDiscardRectangleModeEXT discardRectangleMode) {}
static VKAPI_ATTR void VKAPI_CALL StubSetHdrMetadataEXT(VkDevice device, uint32_t swapchainCount, const VkSwapchainKHR* pSwapchains,
                                                        const VkHdrMetadataEXT* pMetadata) {}
#ifdef VK_USE_PLATFORM_IOS_MVK
static VKAPI_ATTR VkResult VKAPI_CALL StubCreateIOSSurfaceMVK(VkInstance instance, const VkIOSSurfaceCreateInfoMVK* pCreateInfo,
                                                              const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    return VK_SUCCESS;
}
#endif  // VK_USE_PLATFORM_IOS_MVK
#ifdef VK_USE_PLATFORM_MACOS_MVK
static VKAPI_ATTR VkResult VKAPI_CALL StubCreateMacOSSurfaceMVK(VkInstance instance, const VkMacOSSurfaceCreateInfoMVK* pCreateInfo,
                                                                const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    return VK_SUCCESS;
}
#endif  // VK_USE_PLATFORM_MACOS_MVK
static VKAPI_ATTR VkResult VKAPI_CALL StubSetDebugUtilsObjectNameEXT(VkDevice device,
                                                                     const VkDebugUtilsObjectNameInfoEXT* pNameInfo) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubSetDebugUtilsObjectTagEXT(VkDevice device, const VkDebugUtilsObjectTagInfoEXT* pTagInfo) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubQueueBeginDebugUtilsLabelEXT(VkQueue queue, const VkDebugUtilsLabelEXT* pLabelInfo) {}
static VKAPI_ATTR void VKAPI_CALL StubQueueEndDebugUtilsLabelEXT(VkQueue queue) {}
static VKAPI_ATTR void VKAPI_CALL StubQueueInsertDebugUtilsLabelEXT(VkQueue queue, const VkDebugUtilsLabelEXT* pLabelInfo) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdBeginDebugUtilsLabelEXT(VkCommandBuffer commandBuffer,
                                                                 const VkDebugUtilsLabelEXT* pLabelInfo) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdEndDebugUtilsLabelEXT(VkCommandBuffer commandBuffer) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdInsertDebugUtilsLabelEXT(VkCommandBuffer commandBuffer,
                                                                  const VkDebugUtilsLabelEXT* pLabelInfo) {}
static VKAPI_ATTR VkResult VKAPI_CALL StubCreateDebugUtilsMessengerEXT(VkInstance instance,
                                                                       const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                                                       const VkAllocationCallbacks* pAllocator,
                                                                       VkDebugUtilsMessengerEXT* pMessenger) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubDestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT messenger,
                                                                    const VkAllocationCallbacks* pAllocator) {}
static VKAPI_ATTR void VKAPI_CALL StubSubmitDebugUtilsMessageEXT(VkInstance instance,
                                                                 VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                                 VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                                                                 const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData) {}
#ifdef VK_USE_PLATFORM_ANDROID_KHR
static VKAPI_ATTR VkResult VKAPI_CALL StubGetAndroidHardwareBufferPropertiesANDROID(
    VkDevice device, const struct AHardwareBuffer* buffer, VkAndroidHardwareBufferPropertiesANDROID* pProperties) {
    return VK_SUCCESS;
}
#endif  // VK_USE_PLATFORM_ANDROID_KHR
#ifdef VK_USE_PLATFORM_ANDROID_KHR
static VKAPI_ATTR VkResult VKAPI_CALL StubGetMemoryAndroidHardwareBufferANDROID(
    VkDevice device, const VkMemoryGetAndroidHardwareBufferInfoANDROID* pInfo, struct AHardwareBuffer** pBuffer) {
    return VK_SUCCESS;
}
#endif  // VK_USE_PLATFORM_ANDROID_KHR
#ifdef VK_ENABLE_BETA_EXTENSIONS
static VKAPI_ATTR VkResult VKAPI_CALL StubCreateExecutionGraphPipelinesAMDX(
    VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
    const VkExecutionGraphPipelineCreateInfoAMDX* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines) {
    return VK_SUCCESS;
}
#endif  // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
static VKAPI_ATTR VkResult VKAPI_CALL StubGetExecutionGraphPipelineScratchSizeAMDX(
    VkDevice device, VkPipeline executionGraph, VkExecutionGraphPipelineScratchSizeAMDX* pSizeInfo) {
    return VK_SUCCESS;
}
#endif  // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
static VKAPI_ATTR VkResult VKAPI_CALL StubGetExecutionGraphPipelineNodeIndexAMDX(
    VkDevice device, VkPipeline executionGraph, const VkPipelineShaderStageNodeCreateInfoAMDX* pNodeInfo, uint32_t* pNodeIndex) {
    return VK_SUCCESS;
}
#endif  // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
static VKAPI_ATTR void VKAPI_CALL StubCmdInitializeGraphScratchMemoryAMDX(VkCommandBuffer commandBuffer, VkDeviceAddress scratch) {}
#endif  // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
static VKAPI_ATTR void VKAPI_CALL StubCmdDispatchGraphAMDX(VkCommandBuffer commandBuffer, VkDeviceAddress scratch,
                                                           const VkDispatchGraphCountInfoAMDX* pCountInfo) {}
#endif  // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
static VKAPI_ATTR void VKAPI_CALL StubCmdDispatchGraphIndirectAMDX(VkCommandBuffer commandBuffer, VkDeviceAddress scratch,
                                                                   const VkDispatchGraphCountInfoAMDX* pCountInfo) {}
#endif  // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
static VKAPI_ATTR void VKAPI_CALL StubCmdDispatchGraphIndirectCountAMDX(VkCommandBuffer commandBuffer, VkDeviceAddress scratch,
                                                                        VkDeviceAddress countInfo) {}
#endif  // VK_ENABLE_BETA_EXTENSIONS
static VKAPI_ATTR void VKAPI_CALL StubCmdSetSampleLocationsEXT(VkCommandBuffer commandBuffer,
                                                               const VkSampleLocationsInfoEXT* pSampleLocationsInfo) {}
static VKAPI_ATTR void VKAPI_CALL StubGetPhysicalDeviceMultisamplePropertiesEXT(
    VkPhysicalDevice physicalDevice, VkSampleCountFlagBits samples, VkMultisamplePropertiesEXT* pMultisampleProperties) {}
static VKAPI_ATTR VkResult VKAPI_CALL
StubGetImageDrmFormatModifierPropertiesEXT(VkDevice device, VkImage image, VkImageDrmFormatModifierPropertiesEXT* pProperties) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubCreateValidationCacheEXT(VkDevice device,
                                                                   const VkValidationCacheCreateInfoEXT* pCreateInfo,
                                                                   const VkAllocationCallbacks* pAllocator,
                                                                   VkValidationCacheEXT* pValidationCache) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubDestroyValidationCacheEXT(VkDevice device, VkValidationCacheEXT validationCache,
                                                                const VkAllocationCallbacks* pAllocator) {}
static VKAPI_ATTR VkResult VKAPI_CALL StubMergeValidationCachesEXT(VkDevice device, VkValidationCacheEXT dstCache,
                                                                   uint32_t srcCacheCount, const VkValidationCacheEXT* pSrcCaches) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetValidationCacheDataEXT(VkDevice device, VkValidationCacheEXT validationCache,
                                                                    size_t* pDataSize, void* pData) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubCmdBindShadingRateImageNV(VkCommandBuffer commandBuffer, VkImageView imageView,
                                                                VkImageLayout imageLayout) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetViewportShadingRatePaletteNV(VkCommandBuffer commandBuffer, uint32_t firstViewport,
                                                                         uint32_t viewportCount,
                                                                         const VkShadingRatePaletteNV* pShadingRatePalettes) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetCoarseSampleOrderNV(VkCommandBuffer commandBuffer,
                                                                VkCoarseSampleOrderTypeNV sampleOrderType,
                                                                uint32_t customSampleOrderCount,
                                                                const VkCoarseSampleOrderCustomNV* pCustomSampleOrders) {}
static VKAPI_ATTR VkResult VKAPI_CALL StubCreateAccelerationStructureNV(VkDevice device,
                                                                        const VkAccelerationStructureCreateInfoNV* pCreateInfo,
                                                                        const VkAllocationCallbacks* pAllocator,
                                                                        VkAccelerationStructureNV* pAccelerationStructure) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubDestroyAccelerationStructureNV(VkDevice device,
                                                                     VkAccelerationStructureNV accelerationStructure,
                                                                     const VkAllocationCallbacks* pAllocator) {}
static VKAPI_ATTR void VKAPI_CALL StubGetAccelerationStructureMemoryRequirementsNV(
    VkDevice device, const VkAccelerationStructureMemoryRequirementsInfoNV* pInfo, VkMemoryRequirements2KHR* pMemoryRequirements) {}
static VKAPI_ATTR VkResult VKAPI_CALL StubBindAccelerationStructureMemoryNV(
    VkDevice device, uint32_t bindInfoCount, const VkBindAccelerationStructureMemoryInfoNV* pBindInfos) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubCmdBuildAccelerationStructureNV(
    VkCommandBuffer commandBuffer, const VkAccelerationStructureInfoNV* pInfo, VkBuffer instanceData, VkDeviceSize instanceOffset,
    VkBool32 update, VkAccelerationStructureNV dst, VkAccelerationStructureNV src, VkBuffer scratch, VkDeviceSize scratchOffset) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdCopyAccelerationStructureNV(VkCommandBuffer commandBuffer, VkAccelerationStructureNV dst,
                                                                     VkAccelerationStructureNV src,
                                                                     VkCopyAccelerationStructureModeKHR mode) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdTraceRaysNV(VkCommandBuffer commandBuffer, VkBuffer raygenShaderBindingTableBuffer,
                                                     VkDeviceSize raygenShaderBindingOffset, VkBuffer missShaderBindingTableBuffer,
                                                     VkDeviceSize missShaderBindingOffset, VkDeviceSize missShaderBindingStride,
                                                     VkBuffer hitShaderBindingTableBuffer, VkDeviceSize hitShaderBindingOffset,
                                                     VkDeviceSize hitShaderBindingStride, VkBuffer callableShaderBindingTableBuffer,
                                                     VkDeviceSize callableShaderBindingOffset,
                                                     VkDeviceSize callableShaderBindingStride, uint32_t width, uint32_t height,
                                                     uint32_t depth) {}
static VKAPI_ATTR VkResult VKAPI_CALL StubCreateRayTracingPipelinesNV(VkDevice device, VkPipelineCache pipelineCache,
                                                                      uint32_t createInfoCount,
                                                                      const VkRayTracingPipelineCreateInfoNV* pCreateInfos,
                                                                      const VkAllocationCallbacks* pAllocator,
                                                                      VkPipeline* pPipelines) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetRayTracingShaderGroupHandlesKHR(VkDevice device, VkPipeline pipeline,
                                                                             uint32_t firstGroup, uint32_t groupCount,
                                                                             size_t dataSize, void* pData) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetRayTracingShaderGroupHandlesNV(VkDevice device, VkPipeline pipeline,
                                                                            uint32_t firstGroup, uint32_t groupCount,
                                                                            size_t dataSize, void* pData) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetAccelerationStructureHandleNV(VkDevice device,
                                                                           VkAccelerationStructureNV accelerationStructure,
                                                                           size_t dataSize, void* pData) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubCmdWriteAccelerationStructuresPropertiesNV(
    VkCommandBuffer commandBuffer, uint32_t accelerationStructureCount, const VkAccelerationStructureNV* pAccelerationStructures,
    VkQueryType queryType, VkQueryPool queryPool, uint32_t firstQuery) {}
static VKAPI_ATTR VkResult VKAPI_CALL StubCompileDeferredNV(VkDevice device, VkPipeline pipeline, uint32_t shader) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL
StubGetMemoryHostPointerPropertiesEXT(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType, const void* pHostPointer,
                                      VkMemoryHostPointerPropertiesEXT* pMemoryHostPointerProperties) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubCmdWriteBufferMarkerAMD(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage,
                                                              VkBuffer dstBuffer, VkDeviceSize dstOffset, uint32_t marker) {}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetPhysicalDeviceCalibrateableTimeDomainsEXT(VkPhysicalDevice physicalDevice,
                                                                                       uint32_t* pTimeDomainCount,
                                                                                       VkTimeDomainEXT* pTimeDomains) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetCalibratedTimestampsEXT(VkDevice device, uint32_t timestampCount,
                                                                     const VkCalibratedTimestampInfoEXT* pTimestampInfos,
                                                                     uint64_t* pTimestamps, uint64_t* pMaxDeviation) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubCmdDrawMeshTasksNV(VkCommandBuffer commandBuffer, uint32_t taskCount, uint32_t firstTask) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdDrawMeshTasksIndirectNV(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                 VkDeviceSize offset, uint32_t drawCount, uint32_t stride) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdDrawMeshTasksIndirectCountNV(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                      VkDeviceSize offset, VkBuffer countBuffer,
                                                                      VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                                      uint32_t stride) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetExclusiveScissorEnableNV(VkCommandBuffer commandBuffer, uint32_t firstExclusiveScissor,
                                                                     uint32_t exclusiveScissorCount,
                                                                     const VkBool32* pExclusiveScissorEnables) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetExclusiveScissorNV(VkCommandBuffer commandBuffer, uint32_t firstExclusiveScissor,
                                                               uint32_t exclusiveScissorCount, const VkRect2D* pExclusiveScissors) {
}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetCheckpointNV(VkCommandBuffer commandBuffer, const void* pCheckpointMarker) {}
static VKAPI_ATTR void VKAPI_CALL StubGetQueueCheckpointDataNV(VkQueue queue, uint32_t* pCheckpointDataCount,
                                                               VkCheckpointDataNV* pCheckpointData) {}
static VKAPI_ATTR VkResult VKAPI_CALL
StubInitializePerformanceApiINTEL(VkDevice device, const VkInitializePerformanceApiInfoINTEL* pInitializeInfo) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubUninitializePerformanceApiINTEL(VkDevice device) {}
static VKAPI_ATTR VkResult VKAPI_CALL StubCmdSetPerformanceMarkerINTEL(VkCommandBuffer commandBuffer,
                                                                       const VkPerformanceMarkerInfoINTEL* pMarkerInfo) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL
StubCmdSetPerformanceStreamMarkerINTEL(VkCommandBuffer commandBuffer, const VkPerformanceStreamMarkerInfoINTEL* pMarkerInfo) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubCmdSetPerformanceOverrideINTEL(VkCommandBuffer commandBuffer,
                                                                         const VkPerformanceOverrideInfoINTEL* pOverrideInfo) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL
StubAcquirePerformanceConfigurationINTEL(VkDevice device, const VkPerformanceConfigurationAcquireInfoINTEL* pAcquireInfo,
                                         VkPerformanceConfigurationINTEL* pConfiguration) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubReleasePerformanceConfigurationINTEL(VkDevice device,
                                                                               VkPerformanceConfigurationINTEL configuration) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubQueueSetPerformanceConfigurationINTEL(VkQueue queue,
                                                                                VkPerformanceConfigurationINTEL configuration) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetPerformanceParameterINTEL(VkDevice device, VkPerformanceParameterTypeINTEL parameter,
                                                                       VkPerformanceValueINTEL* pValue) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubSetLocalDimmingAMD(VkDevice device, VkSwapchainKHR swapChain, VkBool32 localDimmingEnable) {}
#ifdef VK_USE_PLATFORM_FUCHSIA
static VKAPI_ATTR VkResult VKAPI_CALL StubCreateImagePipeSurfaceFUCHSIA(VkInstance instance,
                                                                        const VkImagePipeSurfaceCreateInfoFUCHSIA* pCreateInfo,
                                                                        const VkAllocationCallbacks* pAllocator,
                                                                        VkSurfaceKHR* pSurface) {
    return VK_SUCCESS;
}
#endif  // VK_USE_PLATFORM_FUCHSIA
#ifdef VK_USE_PLATFORM_METAL_EXT
static VKAPI_ATTR VkResult VKAPI_CALL StubCreateMetalSurfaceEXT(VkInstance instance, const VkMetalSurfaceCreateInfoEXT* pCreateInfo,
                                                                const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    return VK_SUCCESS;
}
#endif  // VK_USE_PLATFORM_METAL_EXT
static VKAPI_ATTR VkDeviceAddress VKAPI_CALL StubGetBufferDeviceAddressEXT(VkDevice device,
                                                                           const VkBufferDeviceAddressInfo* pInfo) {
    return 0;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetPhysicalDeviceToolPropertiesEXT(VkPhysicalDevice physicalDevice, uint32_t* pToolCount,
                                                                             VkPhysicalDeviceToolProperties* pToolProperties) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetPhysicalDeviceCooperativeMatrixPropertiesNV(
    VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount, VkCooperativeMatrixPropertiesNV* pProperties) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV(
    VkPhysicalDevice physicalDevice, uint32_t* pCombinationCount, VkFramebufferMixedSamplesCombinationNV* pCombinations) {
    return VK_SUCCESS;
}
#ifdef VK_USE_PLATFORM_WIN32_KHR
static VKAPI_ATTR VkResult VKAPI_CALL
StubGetPhysicalDeviceSurfacePresentModes2EXT(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo,
                                             uint32_t* pPresentModeCount, VkPresentModeKHR* pPresentModes) {
    return VK_SUCCESS;
}
#endif  // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
static VKAPI_ATTR VkResult VKAPI_CALL StubAcquireFullScreenExclusiveModeEXT(VkDevice device, VkSwapchainKHR swapchain) {
    return VK_SUCCESS;
}
#endif  // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
static VKAPI_ATTR VkResult VKAPI_CALL StubReleaseFullScreenExclusiveModeEXT(VkDevice device, VkSwapchainKHR swapchain) {
    return VK_SUCCESS;
}
#endif  // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
static VKAPI_ATTR VkResult VKAPI_CALL StubGetDeviceGroupSurfacePresentModes2EXT(VkDevice device,
                                                                                const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo,
                                                                                VkDeviceGroupPresentModeFlagsKHR* pModes) {
    return VK_SUCCESS;
}
#endif  // VK_USE_PLATFORM_WIN32_KHR
static VKAPI_ATTR VkResult VKAPI_CALL StubCreateHeadlessSurfaceEXT(VkInstance instance,
                                                                   const VkHeadlessSurfaceCreateInfoEXT* pCreateInfo,
                                                                   const VkAllocationCallbacks* pAllocator,
                                                                   VkSurfaceKHR* pSurface) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetLineStippleEXT(VkCommandBuffer commandBuffer, uint32_t lineStippleFactor,
                                                           uint16_t lineStipplePattern) {}
static VKAPI_ATTR void VKAPI_CALL StubResetQueryPoolEXT(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery,
                                                        uint32_t queryCount) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetCullModeEXT(VkCommandBuffer commandBuffer, VkCullModeFlags cullMode) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetFrontFaceEXT(VkCommandBuffer commandBuffer, VkFrontFace frontFace) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetPrimitiveTopologyEXT(VkCommandBuffer commandBuffer,
                                                                 VkPrimitiveTopology primitiveTopology) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetViewportWithCountEXT(VkCommandBuffer commandBuffer, uint32_t viewportCount,
                                                                 const VkViewport* pViewports) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetScissorWithCountEXT(VkCommandBuffer commandBuffer, uint32_t scissorCount,
                                                                const VkRect2D* pScissors) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdBindVertexBuffers2EXT(VkCommandBuffer commandBuffer, uint32_t firstBinding,
                                                               uint32_t bindingCount, const VkBuffer* pBuffers,
                                                               const VkDeviceSize* pOffsets, const VkDeviceSize* pSizes,
                                                               const VkDeviceSize* pStrides) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetDepthTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthTestEnable) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetDepthWriteEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthWriteEnable) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetDepthCompareOpEXT(VkCommandBuffer commandBuffer, VkCompareOp depthCompareOp) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetDepthBoundsTestEnableEXT(VkCommandBuffer commandBuffer,
                                                                     VkBool32 depthBoundsTestEnable) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetStencilTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 stencilTestEnable) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetStencilOpEXT(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask,
                                                         VkStencilOp failOp, VkStencilOp passOp, VkStencilOp depthFailOp,
                                                         VkCompareOp compareOp) {}
static VKAPI_ATTR VkResult VKAPI_CALL StubCopyMemoryToImageEXT(VkDevice device,
                                                               const VkCopyMemoryToImageInfoEXT* pCopyMemoryToImageInfo) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubCopyImageToMemoryEXT(VkDevice device,
                                                               const VkCopyImageToMemoryInfoEXT* pCopyImageToMemoryInfo) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubCopyImageToImageEXT(VkDevice device,
                                                              const VkCopyImageToImageInfoEXT* pCopyImageToImageInfo) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubTransitionImageLayoutEXT(VkDevice device, uint32_t transitionCount,
                                                                   const VkHostImageLayoutTransitionInfoEXT* pTransitions) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubGetImageSubresourceLayout2EXT(VkDevice device, VkImage image,
                                                                    const VkImageSubresource2KHR* pSubresource,
                                                                    VkSubresourceLayout2KHR* pLayout) {}
static VKAPI_ATTR VkResult VKAPI_CALL StubReleaseSwapchainImagesEXT(VkDevice device,
                                                                    const VkReleaseSwapchainImagesInfoEXT* pReleaseInfo) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubGetGeneratedCommandsMemoryRequirementsNV(
    VkDevice device, const VkGeneratedCommandsMemoryRequirementsInfoNV* pInfo, VkMemoryRequirements2* pMemoryRequirements) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdPreprocessGeneratedCommandsNV(VkCommandBuffer commandBuffer,
                                                                       const VkGeneratedCommandsInfoNV* pGeneratedCommandsInfo) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdExecuteGeneratedCommandsNV(VkCommandBuffer commandBuffer, VkBool32 isPreprocessed,
                                                                    const VkGeneratedCommandsInfoNV* pGeneratedCommandsInfo) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdBindPipelineShaderGroupNV(VkCommandBuffer commandBuffer,
                                                                   VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline,
                                                                   uint32_t groupIndex) {}
static VKAPI_ATTR VkResult VKAPI_CALL StubCreateIndirectCommandsLayoutNV(VkDevice device,
                                                                         const VkIndirectCommandsLayoutCreateInfoNV* pCreateInfo,
                                                                         const VkAllocationCallbacks* pAllocator,
                                                                         VkIndirectCommandsLayoutNV* pIndirectCommandsLayout) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubDestroyIndirectCommandsLayoutNV(VkDevice device,
                                                                      VkIndirectCommandsLayoutNV indirectCommandsLayout,
                                                                      const VkAllocationCallbacks* pAllocator) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetDepthBias2EXT(VkCommandBuffer commandBuffer, const VkDepthBiasInfoEXT* pDepthBiasInfo) {
}
static VKAPI_ATTR VkResult VKAPI_CALL StubAcquireDrmDisplayEXT(VkPhysicalDevice physicalDevice, int32_t drmFd,
                                                               VkDisplayKHR display) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetDrmDisplayEXT(VkPhysicalDevice physicalDevice, int32_t drmFd, uint32_t connectorId,
                                                           VkDisplayKHR* display) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubCreatePrivateDataSlotEXT(VkDevice device, const VkPrivateDataSlotCreateInfo* pCreateInfo,
                                                                   const VkAllocationCallbacks* pAllocator,
                                                                   VkPrivateDataSlot* pPrivateDataSlot) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubDestroyPrivateDataSlotEXT(VkDevice device, VkPrivateDataSlot privateDataSlot,
                                                                const VkAllocationCallbacks* pAllocator) {}
static VKAPI_ATTR VkResult VKAPI_CALL StubSetPrivateDataEXT(VkDevice device, VkObjectType objectType, uint64_t objectHandle,
                                                            VkPrivateDataSlot privateDataSlot, uint64_t data) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubGetPrivateDataEXT(VkDevice device, VkObjectType objectType, uint64_t objectHandle,
                                                        VkPrivateDataSlot privateDataSlot, uint64_t* pData) {}
#ifdef VK_USE_PLATFORM_METAL_EXT
static VKAPI_ATTR void VKAPI_CALL StubExportMetalObjectsEXT(VkDevice device, VkExportMetalObjectsInfoEXT* pMetalObjectsInfo) {}
#endif  // VK_USE_PLATFORM_METAL_EXT
static VKAPI_ATTR void VKAPI_CALL StubGetDescriptorSetLayoutSizeEXT(VkDevice device, VkDescriptorSetLayout layout,
                                                                    VkDeviceSize* pLayoutSizeInBytes) {}
static VKAPI_ATTR void VKAPI_CALL StubGetDescriptorSetLayoutBindingOffsetEXT(VkDevice device, VkDescriptorSetLayout layout,
                                                                             uint32_t binding, VkDeviceSize* pOffset) {}
static VKAPI_ATTR void VKAPI_CALL StubGetDescriptorEXT(VkDevice device, const VkDescriptorGetInfoEXT* pDescriptorInfo,
                                                       size_t dataSize, void* pDescriptor) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdBindDescriptorBuffersEXT(VkCommandBuffer commandBuffer, uint32_t bufferCount,
                                                                  const VkDescriptorBufferBindingInfoEXT* pBindingInfos) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetDescriptorBufferOffsetsEXT(VkCommandBuffer commandBuffer,
                                                                       VkPipelineBindPoint pipelineBindPoint,
                                                                       VkPipelineLayout layout, uint32_t firstSet,
                                                                       uint32_t setCount, const uint32_t* pBufferIndices,
                                                                       const VkDeviceSize* pOffsets) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdBindDescriptorBufferEmbeddedSamplersEXT(VkCommandBuffer commandBuffer,
                                                                                 VkPipelineBindPoint pipelineBindPoint,
                                                                                 VkPipelineLayout layout, uint32_t set) {}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetBufferOpaqueCaptureDescriptorDataEXT(VkDevice device,
                                                                                  const VkBufferCaptureDescriptorDataInfoEXT* pInfo,
                                                                                  void* pData) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetImageOpaqueCaptureDescriptorDataEXT(VkDevice device,
                                                                                 const VkImageCaptureDescriptorDataInfoEXT* pInfo,
                                                                                 void* pData) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL
StubGetImageViewOpaqueCaptureDescriptorDataEXT(VkDevice device, const VkImageViewCaptureDescriptorDataInfoEXT* pInfo, void* pData) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL
StubGetSamplerOpaqueCaptureDescriptorDataEXT(VkDevice device, const VkSamplerCaptureDescriptorDataInfoEXT* pInfo, void* pData) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetAccelerationStructureOpaqueCaptureDescriptorDataEXT(
    VkDevice device, const VkAccelerationStructureCaptureDescriptorDataInfoEXT* pInfo, void* pData) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetFragmentShadingRateEnumNV(VkCommandBuffer commandBuffer,
                                                                      VkFragmentShadingRateNV shadingRate,
                                                                      const VkFragmentShadingRateCombinerOpKHR combinerOps[2]) {}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetDeviceFaultInfoEXT(VkDevice device, VkDeviceFaultCountsEXT* pFaultCounts,
                                                                VkDeviceFaultInfoEXT* pFaultInfo) {
    return VK_SUCCESS;
}
#ifdef VK_USE_PLATFORM_WIN32_KHR
static VKAPI_ATTR VkResult VKAPI_CALL StubAcquireWinrtDisplayNV(VkPhysicalDevice physicalDevice, VkDisplayKHR display) {
    return VK_SUCCESS;
}
#endif  // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
static VKAPI_ATTR VkResult VKAPI_CALL StubGetWinrtDisplayNV(VkPhysicalDevice physicalDevice, uint32_t deviceRelativeId,
                                                            VkDisplayKHR* pDisplay) {
    return VK_SUCCESS;
}
#endif  // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_DIRECTFB_EXT
static VKAPI_ATTR VkResult VKAPI_CALL StubCreateDirectFBSurfaceEXT(VkInstance instance,
                                                                   const VkDirectFBSurfaceCreateInfoEXT* pCreateInfo,
                                                                   const VkAllocationCallbacks* pAllocator,
                                                                   VkSurfaceKHR* pSurface) {
    return VK_SUCCESS;
}
#endif  // VK_USE_PLATFORM_DIRECTFB_EXT
#ifdef VK_USE_PLATFORM_DIRECTFB_EXT
static VKAPI_ATTR VkBool32 VKAPI_CALL StubGetPhysicalDeviceDirectFBPresentationSupportEXT(VkPhysicalDevice physicalDevice,
                                                                                          uint32_t queueFamilyIndex,
                                                                                          IDirectFB* dfb) {
    return VK_FALSE;
}
#endif  // VK_USE_PLATFORM_DIRECTFB_EXT
static VKAPI_ATTR void VKAPI_CALL StubCmdSetVertexInputEXT(
    VkCommandBuffer commandBuffer, uint32_t vertexBindingDescriptionCount,
    const VkVertexInputBindingDescription2EXT* pVertexBindingDescriptions, uint32_t vertexAttributeDescriptionCount,
    const VkVertexInputAttributeDescription2EXT* pVertexAttributeDescriptions) {}
#ifdef VK_USE_PLATFORM_FUCHSIA
static VKAPI_ATTR VkResult VKAPI_CALL StubGetMemoryZirconHandleFUCHSIA(
    VkDevice device, const VkMemoryGetZirconHandleInfoFUCHSIA* pGetZirconHandleInfo, zx_handle_t* pZirconHandle) {
    return VK_SUCCESS;
}
#endif  // VK_USE_PLATFORM_FUCHSIA
#ifdef VK_USE_PLATFORM_FUCHSIA
static VKAPI_ATTR VkResult VKAPI_CALL
StubGetMemoryZirconHandlePropertiesFUCHSIA(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType, zx_handle_t zirconHandle,
                                           VkMemoryZirconHandlePropertiesFUCHSIA* pMemoryZirconHandleProperties) {
    return VK_SUCCESS;
}
#endif  // VK_USE_PLATFORM_FUCHSIA
#ifdef VK_USE_PLATFORM_FUCHSIA
static VKAPI_ATTR VkResult VKAPI_CALL StubImportSemaphoreZirconHandleFUCHSIA(
    VkDevice device, const VkImportSemaphoreZirconHandleInfoFUCHSIA* pImportSemaphoreZirconHandleInfo) {
    return VK_SUCCESS;
}
#endif  // VK_USE_PLATFORM_FUCHSIA
#ifdef VK_USE_PLATFORM_FUCHSIA
static VKAPI_ATTR VkResult VKAPI_CALL StubGetSemaphoreZirconHandleFUCHSIA(
    VkDevice device, const VkSemaphoreGetZirconHandleInfoFUCHSIA* pGetZirconHandleInfo, zx_handle_t* pZirconHandle) {
    return VK_SUCCESS;
}
#endif  // VK_USE_PLATFORM_FUCHSIA
#ifdef VK_USE_PLATFORM_FUCHSIA
static VKAPI_ATTR VkResult VKAPI_CALL StubCreateBufferCollectionFUCHSIA(VkDevice device,
                                                                        const VkBufferCollectionCreateInfoFUCHSIA* pCreateInfo,
                                                                        const VkAllocationCallbacks* pAllocator,
                                                                        VkBufferCollectionFUCHSIA* pCollection) {
    return VK_SUCCESS;
}
#endif  // VK_USE_PLATFORM_FUCHSIA
#ifdef VK_USE_PLATFORM_FUCHSIA
static VKAPI_ATTR VkResult VKAPI_CALL StubSetBufferCollectionImageConstraintsFUCHSIA(
    VkDevice device, VkBufferCollectionFUCHSIA collection, const VkImageConstraintsInfoFUCHSIA* pImageConstraintsInfo) {
    return VK_SUCCESS;
}
#endif  // VK_USE_PLATFORM_FUCHSIA
#ifdef VK_USE_PLATFORM_FUCHSIA
static VKAPI_ATTR VkResult VKAPI_CALL StubSetBufferCollectionBufferConstraintsFUCHSIA(
    VkDevice device, VkBufferCollectionFUCHSIA collection, const VkBufferConstraintsInfoFUCHSIA* pBufferConstraintsInfo) {
    return VK_SUCCESS;
}
#endif  // VK_USE_PLATFORM_FUCHSIA
#ifdef VK_USE_PLATFORM_FUCHSIA
static VKAPI_ATTR void VKAPI_CALL StubDestroyBufferCollectionFUCHSIA(VkDevice device, VkBufferCollectionFUCHSIA collection,
                                                                     const VkAllocationCallbacks* pAllocator) {}
#endif  // VK_USE_PLATFORM_FUCHSIA
#ifdef VK_USE_PLATFORM_FUCHSIA
static VKAPI_ATTR VkResult VKAPI_CALL StubGetBufferCollectionPropertiesFUCHSIA(VkDevice device,
                                                                               VkBufferCollectionFUCHSIA collection,
                                                                               VkBufferCollectionPropertiesFUCHSIA* pProperties) {
    return VK_SUCCESS;
}
#endif  // VK_USE_PLATFORM_FUCHSIA
static VKAPI_ATTR VkResult VKAPI_CALL StubGetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI(VkDevice device, VkRenderPass renderpass,
                                                                                        VkExtent2D* pMaxWorkgroupSize) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubCmdSubpassShadingHUAWEI(VkCommandBuffer commandBuffer) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdBindInvocationMaskHUAWEI(VkCommandBuffer commandBuffer, VkImageView imageView,
                                                                  VkImageLayout imageLayout) {}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetMemoryRemoteAddressNV(
    VkDevice device, const VkMemoryGetRemoteAddressInfoNV* pMemoryGetRemoteAddressInfo, VkRemoteAddressNV* pAddress) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetPipelinePropertiesEXT(VkDevice device, const VkPipelineInfoEXT* pPipelineInfo,
                                                                   VkBaseOutStructure* pPipelineProperties) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetPatchControlPointsEXT(VkCommandBuffer commandBuffer, uint32_t patchControlPoints) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetRasterizerDiscardEnableEXT(VkCommandBuffer commandBuffer,
                                                                       VkBool32 rasterizerDiscardEnable) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetDepthBiasEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthBiasEnable) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetLogicOpEXT(VkCommandBuffer commandBuffer, VkLogicOp logicOp) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetPrimitiveRestartEnableEXT(VkCommandBuffer commandBuffer,
                                                                      VkBool32 primitiveRestartEnable) {}
#ifdef VK_USE_PLATFORM_SCREEN_QNX
static VKAPI_ATTR VkResult VKAPI_CALL StubCreateScreenSurfaceQNX(VkInstance instance,
                                                                 const VkScreenSurfaceCreateInfoQNX* pCreateInfo,
                                                                 const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    return VK_SUCCESS;
}
#endif  // VK_USE_PLATFORM_SCREEN_QNX
#ifdef VK_USE_PLATFORM_SCREEN_QNX
static VKAPI_ATTR VkBool32 VKAPI_CALL StubGetPhysicalDeviceScreenPresentationSupportQNX(VkPhysicalDevice physicalDevice,
                                                                                        uint32_t queueFamilyIndex,
                                                                                        struct _screen_window* window) {
    return VK_FALSE;
}
#endif  // VK_USE_PLATFORM_SCREEN_QNX
static VKAPI_ATTR void VKAPI_CALL StubCmdSetColorWriteEnableEXT(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
                                                                const VkBool32* pColorWriteEnables) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdDrawMultiEXT(VkCommandBuffer commandBuffer, uint32_t drawCount,
                                                      const VkMultiDrawInfoEXT* pVertexInfo, uint32_t instanceCount,
                                                      uint32_t firstInstance, uint32_t stride) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdDrawMultiIndexedEXT(VkCommandBuffer commandBuffer, uint32_t drawCount,
                                                             const VkMultiDrawIndexedInfoEXT* pIndexInfo, uint32_t instanceCount,
                                                             uint32_t firstInstance, uint32_t stride,
                                                             const int32_t* pVertexOffset) {}
static VKAPI_ATTR VkResult VKAPI_CALL StubCreateMicromapEXT(VkDevice device, const VkMicromapCreateInfoEXT* pCreateInfo,
                                                            const VkAllocationCallbacks* pAllocator, VkMicromapEXT* pMicromap) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubDestroyMicromapEXT(VkDevice device, VkMicromapEXT micromap,
                                                         const VkAllocationCallbacks* pAllocator) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdBuildMicromapsEXT(VkCommandBuffer commandBuffer, uint32_t infoCount,
                                                           const VkMicromapBuildInfoEXT* pInfos) {}
static VKAPI_ATTR VkResult VKAPI_CALL StubBuildMicromapsEXT(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                            uint32_t infoCount, const VkMicromapBuildInfoEXT* pInfos) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubCopyMicromapEXT(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                          const VkCopyMicromapInfoEXT* pInfo) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubCopyMicromapToMemoryEXT(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                                  const VkCopyMicromapToMemoryInfoEXT* pInfo) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubCopyMemoryToMicromapEXT(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                                  const VkCopyMemoryToMicromapInfoEXT* pInfo) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubWriteMicromapsPropertiesEXT(VkDevice device, uint32_t micromapCount,
                                                                      const VkMicromapEXT* pMicromaps, VkQueryType queryType,
                                                                      size_t dataSize, void* pData, size_t stride) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubCmdCopyMicromapEXT(VkCommandBuffer commandBuffer, const VkCopyMicromapInfoEXT* pInfo) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdCopyMicromapToMemoryEXT(VkCommandBuffer commandBuffer,
                                                                 const VkCopyMicromapToMemoryInfoEXT* pInfo) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdCopyMemoryToMicromapEXT(VkCommandBuffer commandBuffer,
                                                                 const VkCopyMemoryToMicromapInfoEXT* pInfo) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdWriteMicromapsPropertiesEXT(VkCommandBuffer commandBuffer, uint32_t micromapCount,
                                                                     const VkMicromapEXT* pMicromaps, VkQueryType queryType,
                                                                     VkQueryPool queryPool, uint32_t firstQuery) {}
static VKAPI_ATTR void VKAPI_CALL StubGetDeviceMicromapCompatibilityEXT(VkDevice device,
                                                                        const VkMicromapVersionInfoEXT* pVersionInfo,
                                                                        VkAccelerationStructureCompatibilityKHR* pCompatibility) {}
static VKAPI_ATTR void VKAPI_CALL StubGetMicromapBuildSizesEXT(VkDevice device, VkAccelerationStructureBuildTypeKHR buildType,
                                                               const VkMicromapBuildInfoEXT* pBuildInfo,
                                                               VkMicromapBuildSizesInfoEXT* pSizeInfo) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdDrawClusterHUAWEI(VkCommandBuffer commandBuffer, uint32_t groupCountX,
                                                           uint32_t groupCountY, uint32_t groupCountZ) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdDrawClusterIndirectHUAWEI(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                   VkDeviceSize offset) {}
static VKAPI_ATTR void VKAPI_CALL StubSetDeviceMemoryPriorityEXT(VkDevice device, VkDeviceMemory memory, float priority) {}
static VKAPI_ATTR void VKAPI_CALL
StubGetDescriptorSetLayoutHostMappingInfoVALVE(VkDevice device, const VkDescriptorSetBindingReferenceVALVE* pBindingReference,
                                               VkDescriptorSetLayoutHostMappingInfoVALVE* pHostMapping) {}
static VKAPI_ATTR void VKAPI_CALL StubGetDescriptorSetHostMappingVALVE(VkDevice device, VkDescriptorSet descriptorSet,
                                                                       void** ppData) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdCopyMemoryIndirectNV(VkCommandBuffer commandBuffer, VkDeviceAddress copyBufferAddress,
                                                              uint32_t copyCount, uint32_t stride) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdCopyMemoryToImageIndirectNV(VkCommandBuffer commandBuffer,
                                                                     VkDeviceAddress copyBufferAddress, uint32_t copyCount,
                                                                     uint32_t stride, VkImage dstImage,
                                                                     VkImageLayout dstImageLayout,
                                                                     const VkImageSubresourceLayers* pImageSubresources) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdDecompressMemoryNV(VkCommandBuffer commandBuffer, uint32_t decompressRegionCount,
                                                            const VkDecompressMemoryRegionNV* pDecompressMemoryRegions) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdDecompressMemoryIndirectCountNV(VkCommandBuffer commandBuffer,
                                                                         VkDeviceAddress indirectCommandsAddress,
                                                                         VkDeviceAddress indirectCommandsCountAddress,
                                                                         uint32_t stride) {}
static VKAPI_ATTR void VKAPI_CALL StubGetPipelineIndirectMemoryRequirementsNV(VkDevice device,
                                                                              const VkComputePipelineCreateInfo* pCreateInfo,
                                                                              VkMemoryRequirements2* pMemoryRequirements) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdUpdatePipelineIndirectBufferNV(VkCommandBuffer commandBuffer,
                                                                        VkPipelineBindPoint pipelineBindPoint,
                                                                        VkPipeline pipeline) {}
static VKAPI_ATTR VkDeviceAddress VKAPI_CALL
StubGetPipelineIndirectDeviceAddressNV(VkDevice device, const VkPipelineIndirectDeviceAddressInfoNV* pInfo) {
    return 0;
}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetTessellationDomainOriginEXT(VkCommandBuffer commandBuffer,
                                                                        VkTessellationDomainOrigin domainOrigin) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetDepthClampEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthClampEnable) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetPolygonModeEXT(VkCommandBuffer commandBuffer, VkPolygonMode polygonMode) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetRasterizationSamplesEXT(VkCommandBuffer commandBuffer,
                                                                    VkSampleCountFlagBits rasterizationSamples) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetSampleMaskEXT(VkCommandBuffer commandBuffer, VkSampleCountFlagBits samples,
                                                          const VkSampleMask* pSampleMask) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetAlphaToCoverageEnableEXT(VkCommandBuffer commandBuffer,
                                                                     VkBool32 alphaToCoverageEnable) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetAlphaToOneEnableEXT(VkCommandBuffer commandBuffer, VkBool32 alphaToOneEnable) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetLogicOpEnableEXT(VkCommandBuffer commandBuffer, VkBool32 logicOpEnable) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetColorBlendEnableEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment,
                                                                uint32_t attachmentCount, const VkBool32* pColorBlendEnables) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetColorBlendEquationEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment,
                                                                  uint32_t attachmentCount,
                                                                  const VkColorBlendEquationEXT* pColorBlendEquations) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetColorWriteMaskEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment,
                                                              uint32_t attachmentCount,
                                                              const VkColorComponentFlags* pColorWriteMasks) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetRasterizationStreamEXT(VkCommandBuffer commandBuffer, uint32_t rasterizationStream) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetConservativeRasterizationModeEXT(
    VkCommandBuffer commandBuffer, VkConservativeRasterizationModeEXT conservativeRasterizationMode) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetExtraPrimitiveOverestimationSizeEXT(VkCommandBuffer commandBuffer,
                                                                                float extraPrimitiveOverestimationSize) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetDepthClipEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthClipEnable) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetSampleLocationsEnableEXT(VkCommandBuffer commandBuffer,
                                                                     VkBool32 sampleLocationsEnable) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetColorBlendAdvancedEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment,
                                                                  uint32_t attachmentCount,
                                                                  const VkColorBlendAdvancedEXT* pColorBlendAdvanced) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetProvokingVertexModeEXT(VkCommandBuffer commandBuffer,
                                                                   VkProvokingVertexModeEXT provokingVertexMode) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetLineRasterizationModeEXT(VkCommandBuffer commandBuffer,
                                                                     VkLineRasterizationModeEXT lineRasterizationMode) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetLineStippleEnableEXT(VkCommandBuffer commandBuffer, VkBool32 stippledLineEnable) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetDepthClipNegativeOneToOneEXT(VkCommandBuffer commandBuffer, VkBool32 negativeOneToOne) {
}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetViewportWScalingEnableNV(VkCommandBuffer commandBuffer,
                                                                     VkBool32 viewportWScalingEnable) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetViewportSwizzleNV(VkCommandBuffer commandBuffer, uint32_t firstViewport,
                                                              uint32_t viewportCount,
                                                              const VkViewportSwizzleNV* pViewportSwizzles) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetCoverageToColorEnableNV(VkCommandBuffer commandBuffer, VkBool32 coverageToColorEnable) {
}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetCoverageToColorLocationNV(VkCommandBuffer commandBuffer,
                                                                      uint32_t coverageToColorLocation) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetCoverageModulationModeNV(VkCommandBuffer commandBuffer,
                                                                     VkCoverageModulationModeNV coverageModulationMode) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetCoverageModulationTableEnableNV(VkCommandBuffer commandBuffer,
                                                                            VkBool32 coverageModulationTableEnable) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetCoverageModulationTableNV(VkCommandBuffer commandBuffer,
                                                                      uint32_t coverageModulationTableCount,
                                                                      const float* pCoverageModulationTable) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetShadingRateImageEnableNV(VkCommandBuffer commandBuffer,
                                                                     VkBool32 shadingRateImageEnable) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetRepresentativeFragmentTestEnableNV(VkCommandBuffer commandBuffer,
                                                                               VkBool32 representativeFragmentTestEnable) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetCoverageReductionModeNV(VkCommandBuffer commandBuffer,
                                                                    VkCoverageReductionModeNV coverageReductionMode) {}
static VKAPI_ATTR void VKAPI_CALL StubGetShaderModuleIdentifierEXT(VkDevice device, VkShaderModule shaderModule,
                                                                   VkShaderModuleIdentifierEXT* pIdentifier) {}
static VKAPI_ATTR void VKAPI_CALL StubGetShaderModuleCreateInfoIdentifierEXT(VkDevice device,
                                                                             const VkShaderModuleCreateInfo* pCreateInfo,
                                                                             VkShaderModuleIdentifierEXT* pIdentifier) {}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetPhysicalDeviceOpticalFlowImageFormatsNV(
    VkPhysicalDevice physicalDevice, const VkOpticalFlowImageFormatInfoNV* pOpticalFlowImageFormatInfo, uint32_t* pFormatCount,
    VkOpticalFlowImageFormatPropertiesNV* pImageFormatProperties) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubCreateOpticalFlowSessionNV(VkDevice device,
                                                                     const VkOpticalFlowSessionCreateInfoNV* pCreateInfo,
                                                                     const VkAllocationCallbacks* pAllocator,
                                                                     VkOpticalFlowSessionNV* pSession) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubDestroyOpticalFlowSessionNV(VkDevice device, VkOpticalFlowSessionNV session,
                                                                  const VkAllocationCallbacks* pAllocator) {}
static VKAPI_ATTR VkResult VKAPI_CALL StubBindOpticalFlowSessionImageNV(VkDevice device, VkOpticalFlowSessionNV session,
                                                                        VkOpticalFlowSessionBindingPointNV bindingPoint,
                                                                        VkImageView view, VkImageLayout layout) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubCmdOpticalFlowExecuteNV(VkCommandBuffer commandBuffer, VkOpticalFlowSessionNV session,
                                                              const VkOpticalFlowExecuteInfoNV* pExecuteInfo) {}
static VKAPI_ATTR VkResult VKAPI_CALL StubCreateShadersEXT(VkDevice device, uint32_t createInfoCount,
                                                           const VkShaderCreateInfoEXT* pCreateInfos,
                                                           const VkAllocationCallbacks* pAllocator, VkShaderEXT* pShaders) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubDestroyShaderEXT(VkDevice device, VkShaderEXT shader,
                                                       const VkAllocationCallbacks* pAllocator) {}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetShaderBinaryDataEXT(VkDevice device, VkShaderEXT shader, size_t* pDataSize,
                                                                 void* pData) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubCmdBindShadersEXT(VkCommandBuffer commandBuffer, uint32_t stageCount,
                                                        const VkShaderStageFlagBits* pStages, const VkShaderEXT* pShaders) {}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetFramebufferTilePropertiesQCOM(VkDevice device, VkFramebuffer framebuffer,
                                                                           uint32_t* pPropertiesCount,
                                                                           VkTilePropertiesQCOM* pProperties) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetDynamicRenderingTilePropertiesQCOM(VkDevice device,
                                                                                const VkRenderingInfo* pRenderingInfo,
                                                                                VkTilePropertiesQCOM* pProperties) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubSetLatencySleepModeNV(VkDevice device, VkSwapchainKHR swapchain,
                                                                const VkLatencySleepModeInfoNV* pSleepModeInfo) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubLatencySleepNV(VkDevice device, VkSwapchainKHR swapchain,
                                                         const VkLatencySleepInfoNV* pSleepInfo) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubSetLatencyMarkerNV(VkDevice device, VkSwapchainKHR swapchain,
                                                         const VkSetLatencyMarkerInfoNV* pLatencyMarkerInfo) {}
static VKAPI_ATTR void VKAPI_CALL StubGetLatencyTimingsNV(VkDevice device, VkSwapchainKHR swapchain, uint32_t* pTimingCount,
                                                          VkGetLatencyMarkerInfoNV* pLatencyMarkerInfo) {}
static VKAPI_ATTR void VKAPI_CALL StubQueueNotifyOutOfBandNV(VkQueue queue, const VkOutOfBandQueueTypeInfoNV* pQueueTypeInfo) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetAttachmentFeedbackLoopEnableEXT(VkCommandBuffer commandBuffer,
                                                                            VkImageAspectFlags aspectMask) {}
#ifdef VK_USE_PLATFORM_SCREEN_QNX
static VKAPI_ATTR VkResult VKAPI_CALL StubGetScreenBufferPropertiesQNX(VkDevice device, const struct _screen_buffer* buffer,
                                                                       VkScreenBufferPropertiesQNX* pProperties) {
    return VK_SUCCESS;
}
#endif  // VK_USE_PLATFORM_SCREEN_QNX
static VKAPI_ATTR VkResult VKAPI_CALL StubCreateAccelerationStructureKHR(VkDevice device,
                                                                         const VkAccelerationStructureCreateInfoKHR* pCreateInfo,
                                                                         const VkAllocationCallbacks* pAllocator,
                                                                         VkAccelerationStructureKHR* pAccelerationStructure) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubDestroyAccelerationStructureKHR(VkDevice device,
                                                                      VkAccelerationStructureKHR accelerationStructure,
                                                                      const VkAllocationCallbacks* pAllocator) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdBuildAccelerationStructuresKHR(
    VkCommandBuffer commandBuffer, uint32_t infoCount, const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
    const VkAccelerationStructureBuildRangeInfoKHR* const* ppBuildRangeInfos) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdBuildAccelerationStructuresIndirectKHR(
    VkCommandBuffer commandBuffer, uint32_t infoCount, const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
    const VkDeviceAddress* pIndirectDeviceAddresses, const uint32_t* pIndirectStrides,
    const uint32_t* const* ppMaxPrimitiveCounts) {}
static VKAPI_ATTR VkResult VKAPI_CALL
StubBuildAccelerationStructuresKHR(VkDevice device, VkDeferredOperationKHR deferredOperation, uint32_t infoCount,
                                   const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
                                   const VkAccelerationStructureBuildRangeInfoKHR* const* ppBuildRangeInfos) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubCopyAccelerationStructureKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                                       const VkCopyAccelerationStructureInfoKHR* pInfo) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubCopyAccelerationStructureToMemoryKHR(
    VkDevice device, VkDeferredOperationKHR deferredOperation, const VkCopyAccelerationStructureToMemoryInfoKHR* pInfo) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubCopyMemoryToAccelerationStructureKHR(
    VkDevice device, VkDeferredOperationKHR deferredOperation, const VkCopyMemoryToAccelerationStructureInfoKHR* pInfo) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubWriteAccelerationStructuresPropertiesKHR(
    VkDevice device, uint32_t accelerationStructureCount, const VkAccelerationStructureKHR* pAccelerationStructures,
    VkQueryType queryType, size_t dataSize, void* pData, size_t stride) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubCmdCopyAccelerationStructureKHR(VkCommandBuffer commandBuffer,
                                                                      const VkCopyAccelerationStructureInfoKHR* pInfo) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdCopyAccelerationStructureToMemoryKHR(
    VkCommandBuffer commandBuffer, const VkCopyAccelerationStructureToMemoryInfoKHR* pInfo) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdCopyMemoryToAccelerationStructureKHR(
    VkCommandBuffer commandBuffer, const VkCopyMemoryToAccelerationStructureInfoKHR* pInfo) {}
static VKAPI_ATTR VkDeviceAddress VKAPI_CALL
StubGetAccelerationStructureDeviceAddressKHR(VkDevice device, const VkAccelerationStructureDeviceAddressInfoKHR* pInfo) {
    return 0;
}
static VKAPI_ATTR void VKAPI_CALL StubCmdWriteAccelerationStructuresPropertiesKHR(
    VkCommandBuffer commandBuffer, uint32_t accelerationStructureCount, const VkAccelerationStructureKHR* pAccelerationStructures,
    VkQueryType queryType, VkQueryPool queryPool, uint32_t firstQuery) {}
static VKAPI_ATTR void VKAPI_CALL
StubGetDeviceAccelerationStructureCompatibilityKHR(VkDevice device, const VkAccelerationStructureVersionInfoKHR* pVersionInfo,
                                                   VkAccelerationStructureCompatibilityKHR* pCompatibility) {}
static VKAPI_ATTR void VKAPI_CALL StubGetAccelerationStructureBuildSizesKHR(
    VkDevice device, VkAccelerationStructureBuildTypeKHR buildType, const VkAccelerationStructureBuildGeometryInfoKHR* pBuildInfo,
    const uint32_t* pMaxPrimitiveCounts, VkAccelerationStructureBuildSizesInfoKHR* pSizeInfo) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdTraceRaysKHR(VkCommandBuffer commandBuffer,
                                                      const VkStridedDeviceAddressRegionKHR* pRaygenShaderBindingTable,
                                                      const VkStridedDeviceAddressRegionKHR* pMissShaderBindingTable,
                                                      const VkStridedDeviceAddressRegionKHR* pHitShaderBindingTable,
                                                      const VkStridedDeviceAddressRegionKHR* pCallableShaderBindingTable,
                                                      uint32_t width, uint32_t height, uint32_t depth) {}
static VKAPI_ATTR VkResult VKAPI_CALL StubCreateRayTracingPipelinesKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                                       VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                                       const VkRayTracingPipelineCreateInfoKHR* pCreateInfos,
                                                                       const VkAllocationCallbacks* pAllocator,
                                                                       VkPipeline* pPipelines) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetRayTracingCaptureReplayShaderGroupHandlesKHR(VkDevice device, VkPipeline pipeline,
                                                                                          uint32_t firstGroup, uint32_t groupCount,
                                                                                          size_t dataSize, void* pData) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubCmdTraceRaysIndirectKHR(VkCommandBuffer commandBuffer,
                                                              const VkStridedDeviceAddressRegionKHR* pRaygenShaderBindingTable,
                                                              const VkStridedDeviceAddressRegionKHR* pMissShaderBindingTable,
                                                              const VkStridedDeviceAddressRegionKHR* pHitShaderBindingTable,
                                                              const VkStridedDeviceAddressRegionKHR* pCallableShaderBindingTable,
                                                              VkDeviceAddress indirectDeviceAddress) {}
static VKAPI_ATTR VkDeviceSize VKAPI_CALL StubGetRayTracingShaderGroupStackSizeKHR(VkDevice device, VkPipeline pipeline,
                                                                                   uint32_t group,
                                                                                   VkShaderGroupShaderKHR groupShader) {
    return 0;
}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetRayTracingPipelineStackSizeKHR(VkCommandBuffer commandBuffer,
                                                                           uint32_t pipelineStackSize) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdDrawMeshTasksEXT(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY,
                                                          uint32_t groupCountZ) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdDrawMeshTasksIndirectEXT(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                  VkDeviceSize offset, uint32_t drawCount, uint32_t stride) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdDrawMeshTasksIndirectCountEXT(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                       VkDeviceSize offset, VkBuffer countBuffer,
                                                                       VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                                       uint32_t stride) {}

const vvl::unordered_map<std::string, small_vector<std::string, 2, size_t>> api_extension_map{
    {"vkBindBufferMemory2", {"VK_VERSION_1_1"}},
    {"vkBindImageMemory2", {"VK_VERSION_1_1"}},
    {"vkGetDeviceGroupPeerMemoryFeatures", {"VK_VERSION_1_1"}},
    {"vkCmdSetDeviceMask", {"VK_VERSION_1_1"}},
    {"vkCmdDispatchBase", {"VK_VERSION_1_1"}},
    {"vkGetImageMemoryRequirements2", {"VK_VERSION_1_1"}},
    {"vkGetBufferMemoryRequirements2", {"VK_VERSION_1_1"}},
    {"vkGetImageSparseMemoryRequirements2", {"VK_VERSION_1_1"}},
    {"vkTrimCommandPool", {"VK_VERSION_1_1"}},
    {"vkGetDeviceQueue2", {"VK_VERSION_1_1"}},
    {"vkCreateSamplerYcbcrConversion", {"VK_VERSION_1_1"}},
    {"vkDestroySamplerYcbcrConversion", {"VK_VERSION_1_1"}},
    {"vkCreateDescriptorUpdateTemplate", {"VK_VERSION_1_1"}},
    {"vkDestroyDescriptorUpdateTemplate", {"VK_VERSION_1_1"}},
    {"vkUpdateDescriptorSetWithTemplate", {"VK_VERSION_1_1"}},
    {"vkGetDescriptorSetLayoutSupport", {"VK_VERSION_1_1"}},
    {"vkCmdDrawIndirectCount", {"VK_VERSION_1_2"}},
    {"vkCmdDrawIndexedIndirectCount", {"VK_VERSION_1_2"}},
    {"vkCreateRenderPass2", {"VK_VERSION_1_2"}},
    {"vkCmdBeginRenderPass2", {"VK_VERSION_1_2"}},
    {"vkCmdNextSubpass2", {"VK_VERSION_1_2"}},
    {"vkCmdEndRenderPass2", {"VK_VERSION_1_2"}},
    {"vkResetQueryPool", {"VK_VERSION_1_2"}},
    {"vkGetSemaphoreCounterValue", {"VK_VERSION_1_2"}},
    {"vkWaitSemaphores", {"VK_VERSION_1_2"}},
    {"vkSignalSemaphore", {"VK_VERSION_1_2"}},
    {"vkGetBufferDeviceAddress", {"VK_VERSION_1_2"}},
    {"vkGetBufferOpaqueCaptureAddress", {"VK_VERSION_1_2"}},
    {"vkGetDeviceMemoryOpaqueCaptureAddress", {"VK_VERSION_1_2"}},
    {"vkCreatePrivateDataSlot", {"VK_VERSION_1_3"}},
    {"vkDestroyPrivateDataSlot", {"VK_VERSION_1_3"}},
    {"vkSetPrivateData", {"VK_VERSION_1_3"}},
    {"vkGetPrivateData", {"VK_VERSION_1_3"}},
    {"vkCmdSetEvent2", {"VK_VERSION_1_3"}},
    {"vkCmdResetEvent2", {"VK_VERSION_1_3"}},
    {"vkCmdWaitEvents2", {"VK_VERSION_1_3"}},
    {"vkCmdPipelineBarrier2", {"VK_VERSION_1_3"}},
    {"vkCmdWriteTimestamp2", {"VK_VERSION_1_3"}},
    {"vkQueueSubmit2", {"VK_VERSION_1_3"}},
    {"vkCmdCopyBuffer2", {"VK_VERSION_1_3"}},
    {"vkCmdCopyImage2", {"VK_VERSION_1_3"}},
    {"vkCmdCopyBufferToImage2", {"VK_VERSION_1_3"}},
    {"vkCmdCopyImageToBuffer2", {"VK_VERSION_1_3"}},
    {"vkCmdBlitImage2", {"VK_VERSION_1_3"}},
    {"vkCmdResolveImage2", {"VK_VERSION_1_3"}},
    {"vkCmdBeginRendering", {"VK_VERSION_1_3"}},
    {"vkCmdEndRendering", {"VK_VERSION_1_3"}},
    {"vkCmdSetCullMode", {"VK_VERSION_1_3"}},
    {"vkCmdSetFrontFace", {"VK_VERSION_1_3"}},
    {"vkCmdSetPrimitiveTopology", {"VK_VERSION_1_3"}},
    {"vkCmdSetViewportWithCount", {"VK_VERSION_1_3"}},
    {"vkCmdSetScissorWithCount", {"VK_VERSION_1_3"}},
    {"vkCmdBindVertexBuffers2", {"VK_VERSION_1_3"}},
    {"vkCmdSetDepthTestEnable", {"VK_VERSION_1_3"}},
    {"vkCmdSetDepthWriteEnable", {"VK_VERSION_1_3"}},
    {"vkCmdSetDepthCompareOp", {"VK_VERSION_1_3"}},
    {"vkCmdSetDepthBoundsTestEnable", {"VK_VERSION_1_3"}},
    {"vkCmdSetStencilTestEnable", {"VK_VERSION_1_3"}},
    {"vkCmdSetStencilOp", {"VK_VERSION_1_3"}},
    {"vkCmdSetRasterizerDiscardEnable", {"VK_VERSION_1_3"}},
    {"vkCmdSetDepthBiasEnable", {"VK_VERSION_1_3"}},
    {"vkCmdSetPrimitiveRestartEnable", {"VK_VERSION_1_3"}},
    {"vkGetDeviceBufferMemoryRequirements", {"VK_VERSION_1_3"}},
    {"vkGetDeviceImageMemoryRequirements", {"VK_VERSION_1_3"}},
    {"vkGetDeviceImageSparseMemoryRequirements", {"VK_VERSION_1_3"}},
    {"vkCreateSwapchainKHR", {"VK_KHR_swapchain"}},
    {"vkDestroySwapchainKHR", {"VK_KHR_swapchain"}},
    {"vkGetSwapchainImagesKHR", {"VK_KHR_swapchain"}},
    {"vkAcquireNextImageKHR", {"VK_KHR_swapchain"}},
    {"vkQueuePresentKHR", {"VK_KHR_swapchain"}},
    {"vkGetDeviceGroupPresentCapabilitiesKHR", {"VK_KHR_swapchain", "VK_KHR_device_group"}},
    {"vkGetDeviceGroupSurfacePresentModesKHR", {"VK_KHR_swapchain", "VK_KHR_device_group"}},
    {"vkAcquireNextImage2KHR", {"VK_KHR_swapchain", "VK_KHR_device_group"}},
    {"vkCreateSharedSwapchainsKHR", {"VK_KHR_display_swapchain"}},
    {"vkCreateVideoSessionKHR", {"VK_KHR_video_queue"}},
    {"vkDestroyVideoSessionKHR", {"VK_KHR_video_queue"}},
    {"vkGetVideoSessionMemoryRequirementsKHR", {"VK_KHR_video_queue"}},
    {"vkBindVideoSessionMemoryKHR", {"VK_KHR_video_queue"}},
    {"vkCreateVideoSessionParametersKHR", {"VK_KHR_video_queue"}},
    {"vkUpdateVideoSessionParametersKHR", {"VK_KHR_video_queue"}},
    {"vkDestroyVideoSessionParametersKHR", {"VK_KHR_video_queue"}},
    {"vkCmdBeginVideoCodingKHR", {"VK_KHR_video_queue"}},
    {"vkCmdEndVideoCodingKHR", {"VK_KHR_video_queue"}},
    {"vkCmdControlVideoCodingKHR", {"VK_KHR_video_queue"}},
    {"vkCmdDecodeVideoKHR", {"VK_KHR_video_decode_queue"}},
    {"vkCmdBeginRenderingKHR", {"VK_KHR_dynamic_rendering"}},
    {"vkCmdEndRenderingKHR", {"VK_KHR_dynamic_rendering"}},
    {"vkGetDeviceGroupPeerMemoryFeaturesKHR", {"VK_KHR_device_group"}},
    {"vkCmdSetDeviceMaskKHR", {"VK_KHR_device_group"}},
    {"vkCmdDispatchBaseKHR", {"VK_KHR_device_group"}},
    {"vkTrimCommandPoolKHR", {"VK_KHR_maintenance1"}},
    {"vkGetMemoryWin32HandleKHR", {"VK_KHR_external_memory_win32"}},
    {"vkGetMemoryWin32HandlePropertiesKHR", {"VK_KHR_external_memory_win32"}},
    {"vkGetMemoryFdKHR", {"VK_KHR_external_memory_fd"}},
    {"vkGetMemoryFdPropertiesKHR", {"VK_KHR_external_memory_fd"}},
    {"vkImportSemaphoreWin32HandleKHR", {"VK_KHR_external_semaphore_win32"}},
    {"vkGetSemaphoreWin32HandleKHR", {"VK_KHR_external_semaphore_win32"}},
    {"vkImportSemaphoreFdKHR", {"VK_KHR_external_semaphore_fd"}},
    {"vkGetSemaphoreFdKHR", {"VK_KHR_external_semaphore_fd"}},
    {"vkCmdPushDescriptorSetKHR", {"VK_KHR_push_descriptor"}},
    {"vkCmdPushDescriptorSetWithTemplateKHR", {"VK_KHR_push_descriptor", "VK_KHR_descriptor_update_template"}},
    {"vkCreateDescriptorUpdateTemplateKHR", {"VK_KHR_descriptor_update_template"}},
    {"vkDestroyDescriptorUpdateTemplateKHR", {"VK_KHR_descriptor_update_template"}},
    {"vkUpdateDescriptorSetWithTemplateKHR", {"VK_KHR_descriptor_update_template"}},
    {"vkCreateRenderPass2KHR", {"VK_KHR_create_renderpass2"}},
    {"vkCmdBeginRenderPass2KHR", {"VK_KHR_create_renderpass2"}},
    {"vkCmdNextSubpass2KHR", {"VK_KHR_create_renderpass2"}},
    {"vkCmdEndRenderPass2KHR", {"VK_KHR_create_renderpass2"}},
    {"vkGetSwapchainStatusKHR", {"VK_KHR_shared_presentable_image"}},
    {"vkImportFenceWin32HandleKHR", {"VK_KHR_external_fence_win32"}},
    {"vkGetFenceWin32HandleKHR", {"VK_KHR_external_fence_win32"}},
    {"vkImportFenceFdKHR", {"VK_KHR_external_fence_fd"}},
    {"vkGetFenceFdKHR", {"VK_KHR_external_fence_fd"}},
    {"vkAcquireProfilingLockKHR", {"VK_KHR_performance_query"}},
    {"vkReleaseProfilingLockKHR", {"VK_KHR_performance_query"}},
    {"vkGetImageMemoryRequirements2KHR", {"VK_KHR_get_memory_requirements2"}},
    {"vkGetBufferMemoryRequirements2KHR", {"VK_KHR_get_memory_requirements2"}},
    {"vkGetImageSparseMemoryRequirements2KHR", {"VK_KHR_get_memory_requirements2"}},
    {"vkCreateSamplerYcbcrConversionKHR", {"VK_KHR_sampler_ycbcr_conversion"}},
    {"vkDestroySamplerYcbcrConversionKHR", {"VK_KHR_sampler_ycbcr_conversion"}},
    {"vkBindBufferMemory2KHR", {"VK_KHR_bind_memory2"}},
    {"vkBindImageMemory2KHR", {"VK_KHR_bind_memory2"}},
    {"vkGetDescriptorSetLayoutSupportKHR", {"VK_KHR_maintenance3"}},
    {"vkCmdDrawIndirectCountKHR", {"VK_KHR_draw_indirect_count"}},
    {"vkCmdDrawIndexedIndirectCountKHR", {"VK_KHR_draw_indirect_count"}},
    {"vkGetSemaphoreCounterValueKHR", {"VK_KHR_timeline_semaphore"}},
    {"vkWaitSemaphoresKHR", {"VK_KHR_timeline_semaphore"}},
    {"vkSignalSemaphoreKHR", {"VK_KHR_timeline_semaphore"}},
    {"vkCmdSetFragmentShadingRateKHR", {"VK_KHR_fragment_shading_rate"}},
    {"vkWaitForPresentKHR", {"VK_KHR_present_wait"}},
    {"vkGetBufferDeviceAddressKHR", {"VK_KHR_buffer_device_address"}},
    {"vkGetBufferOpaqueCaptureAddressKHR", {"VK_KHR_buffer_device_address"}},
    {"vkGetDeviceMemoryOpaqueCaptureAddressKHR", {"VK_KHR_buffer_device_address"}},
    {"vkCreateDeferredOperationKHR", {"VK_KHR_deferred_host_operations"}},
    {"vkDestroyDeferredOperationKHR", {"VK_KHR_deferred_host_operations"}},
    {"vkGetDeferredOperationMaxConcurrencyKHR", {"VK_KHR_deferred_host_operations"}},
    {"vkGetDeferredOperationResultKHR", {"VK_KHR_deferred_host_operations"}},
    {"vkDeferredOperationJoinKHR", {"VK_KHR_deferred_host_operations"}},
    {"vkGetPipelineExecutablePropertiesKHR", {"VK_KHR_pipeline_executable_properties"}},
    {"vkGetPipelineExecutableStatisticsKHR", {"VK_KHR_pipeline_executable_properties"}},
    {"vkGetPipelineExecutableInternalRepresentationsKHR", {"VK_KHR_pipeline_executable_properties"}},
    {"vkMapMemory2KHR", {"VK_KHR_map_memory2"}},
    {"vkUnmapMemory2KHR", {"VK_KHR_map_memory2"}},
    {"vkGetEncodedVideoSessionParametersKHR", {"VK_KHR_video_encode_queue"}},
    {"vkCmdEncodeVideoKHR", {"VK_KHR_video_encode_queue"}},
    {"vkCmdSetEvent2KHR", {"VK_KHR_synchronization2"}},
    {"vkCmdResetEvent2KHR", {"VK_KHR_synchronization2"}},
    {"vkCmdWaitEvents2KHR", {"VK_KHR_synchronization2"}},
    {"vkCmdPipelineBarrier2KHR", {"VK_KHR_synchronization2"}},
    {"vkCmdWriteTimestamp2KHR", {"VK_KHR_synchronization2"}},
    {"vkQueueSubmit2KHR", {"VK_KHR_synchronization2"}},
    {"vkCmdWriteBufferMarker2AMD", {"VK_KHR_synchronization2"}},
    {"vkGetQueueCheckpointData2NV", {"VK_KHR_synchronization2"}},
    {"vkCmdCopyBuffer2KHR", {"VK_KHR_copy_commands2"}},
    {"vkCmdCopyImage2KHR", {"VK_KHR_copy_commands2"}},
    {"vkCmdCopyBufferToImage2KHR", {"VK_KHR_copy_commands2"}},
    {"vkCmdCopyImageToBuffer2KHR", {"VK_KHR_copy_commands2"}},
    {"vkCmdBlitImage2KHR", {"VK_KHR_copy_commands2"}},
    {"vkCmdResolveImage2KHR", {"VK_KHR_copy_commands2"}},
    {"vkCmdTraceRaysIndirect2KHR", {"VK_KHR_ray_tracing_maintenance1"}},
    {"vkGetDeviceBufferMemoryRequirementsKHR", {"VK_KHR_maintenance4"}},
    {"vkGetDeviceImageMemoryRequirementsKHR", {"VK_KHR_maintenance4"}},
    {"vkGetDeviceImageSparseMemoryRequirementsKHR", {"VK_KHR_maintenance4"}},
    {"vkCmdBindIndexBuffer2KHR", {"VK_KHR_maintenance5"}},
    {"vkGetRenderingAreaGranularityKHR", {"VK_KHR_maintenance5"}},
    {"vkGetDeviceImageSubresourceLayoutKHR", {"VK_KHR_maintenance5"}},
    {"vkGetImageSubresourceLayout2KHR", {"VK_KHR_maintenance5"}},
    {"vkDebugMarkerSetObjectTagEXT", {"VK_EXT_debug_marker"}},
    {"vkDebugMarkerSetObjectNameEXT", {"VK_EXT_debug_marker"}},
    {"vkCmdDebugMarkerBeginEXT", {"VK_EXT_debug_marker"}},
    {"vkCmdDebugMarkerEndEXT", {"VK_EXT_debug_marker"}},
    {"vkCmdDebugMarkerInsertEXT", {"VK_EXT_debug_marker"}},
    {"vkCmdBindTransformFeedbackBuffersEXT", {"VK_EXT_transform_feedback"}},
    {"vkCmdBeginTransformFeedbackEXT", {"VK_EXT_transform_feedback"}},
    {"vkCmdEndTransformFeedbackEXT", {"VK_EXT_transform_feedback"}},
    {"vkCmdBeginQueryIndexedEXT", {"VK_EXT_transform_feedback"}},
    {"vkCmdEndQueryIndexedEXT", {"VK_EXT_transform_feedback"}},
    {"vkCmdDrawIndirectByteCountEXT", {"VK_EXT_transform_feedback"}},
    {"vkCreateCuModuleNVX", {"VK_NVX_binary_import"}},
    {"vkCreateCuFunctionNVX", {"VK_NVX_binary_import"}},
    {"vkDestroyCuModuleNVX", {"VK_NVX_binary_import"}},
    {"vkDestroyCuFunctionNVX", {"VK_NVX_binary_import"}},
    {"vkCmdCuLaunchKernelNVX", {"VK_NVX_binary_import"}},
    {"vkGetImageViewHandleNVX", {"VK_NVX_image_view_handle"}},
    {"vkGetImageViewAddressNVX", {"VK_NVX_image_view_handle"}},
    {"vkCmdDrawIndirectCountAMD", {"VK_AMD_draw_indirect_count"}},
    {"vkCmdDrawIndexedIndirectCountAMD", {"VK_AMD_draw_indirect_count"}},
    {"vkGetShaderInfoAMD", {"VK_AMD_shader_info"}},
    {"vkGetMemoryWin32HandleNV", {"VK_NV_external_memory_win32"}},
    {"vkCmdBeginConditionalRenderingEXT", {"VK_EXT_conditional_rendering"}},
    {"vkCmdEndConditionalRenderingEXT", {"VK_EXT_conditional_rendering"}},
    {"vkCmdSetViewportWScalingNV", {"VK_NV_clip_space_w_scaling"}},
    {"vkDisplayPowerControlEXT", {"VK_EXT_display_control"}},
    {"vkRegisterDeviceEventEXT", {"VK_EXT_display_control"}},
    {"vkRegisterDisplayEventEXT", {"VK_EXT_display_control"}},
    {"vkGetSwapchainCounterEXT", {"VK_EXT_display_control"}},
    {"vkGetRefreshCycleDurationGOOGLE", {"VK_GOOGLE_display_timing"}},
    {"vkGetPastPresentationTimingGOOGLE", {"VK_GOOGLE_display_timing"}},
    {"vkCmdSetDiscardRectangleEXT", {"VK_EXT_discard_rectangles"}},
    {"vkCmdSetDiscardRectangleEnableEXT", {"VK_EXT_discard_rectangles"}},
    {"vkCmdSetDiscardRectangleModeEXT", {"VK_EXT_discard_rectangles"}},
    {"vkSetHdrMetadataEXT", {"VK_EXT_hdr_metadata"}},
    {"vkSetDebugUtilsObjectNameEXT", {"VK_EXT_debug_utils"}},
    {"vkSetDebugUtilsObjectTagEXT", {"VK_EXT_debug_utils"}},
    {"vkQueueBeginDebugUtilsLabelEXT", {"VK_EXT_debug_utils"}},
    {"vkQueueEndDebugUtilsLabelEXT", {"VK_EXT_debug_utils"}},
    {"vkQueueInsertDebugUtilsLabelEXT", {"VK_EXT_debug_utils"}},
    {"vkCmdBeginDebugUtilsLabelEXT", {"VK_EXT_debug_utils"}},
    {"vkCmdEndDebugUtilsLabelEXT", {"VK_EXT_debug_utils"}},
    {"vkCmdInsertDebugUtilsLabelEXT", {"VK_EXT_debug_utils"}},
    {"vkGetAndroidHardwareBufferPropertiesANDROID", {"VK_ANDROID_external_memory_android_hardware_buffer"}},
    {"vkGetMemoryAndroidHardwareBufferANDROID", {"VK_ANDROID_external_memory_android_hardware_buffer"}},
    {"vkCreateExecutionGraphPipelinesAMDX", {"VK_AMDX_shader_enqueue"}},
    {"vkGetExecutionGraphPipelineScratchSizeAMDX", {"VK_AMDX_shader_enqueue"}},
    {"vkGetExecutionGraphPipelineNodeIndexAMDX", {"VK_AMDX_shader_enqueue"}},
    {"vkCmdInitializeGraphScratchMemoryAMDX", {"VK_AMDX_shader_enqueue"}},
    {"vkCmdDispatchGraphAMDX", {"VK_AMDX_shader_enqueue"}},
    {"vkCmdDispatchGraphIndirectAMDX", {"VK_AMDX_shader_enqueue"}},
    {"vkCmdDispatchGraphIndirectCountAMDX", {"VK_AMDX_shader_enqueue"}},
    {"vkCmdSetSampleLocationsEXT", {"VK_EXT_sample_locations"}},
    {"vkGetImageDrmFormatModifierPropertiesEXT", {"VK_EXT_image_drm_format_modifier"}},
    {"vkCreateValidationCacheEXT", {"VK_EXT_validation_cache"}},
    {"vkDestroyValidationCacheEXT", {"VK_EXT_validation_cache"}},
    {"vkMergeValidationCachesEXT", {"VK_EXT_validation_cache"}},
    {"vkGetValidationCacheDataEXT", {"VK_EXT_validation_cache"}},
    {"vkCmdBindShadingRateImageNV", {"VK_NV_shading_rate_image"}},
    {"vkCmdSetViewportShadingRatePaletteNV", {"VK_NV_shading_rate_image"}},
    {"vkCmdSetCoarseSampleOrderNV", {"VK_NV_shading_rate_image"}},
    {"vkCreateAccelerationStructureNV", {"VK_NV_ray_tracing"}},
    {"vkDestroyAccelerationStructureNV", {"VK_NV_ray_tracing"}},
    {"vkGetAccelerationStructureMemoryRequirementsNV", {"VK_NV_ray_tracing"}},
    {"vkBindAccelerationStructureMemoryNV", {"VK_NV_ray_tracing"}},
    {"vkCmdBuildAccelerationStructureNV", {"VK_NV_ray_tracing"}},
    {"vkCmdCopyAccelerationStructureNV", {"VK_NV_ray_tracing"}},
    {"vkCmdTraceRaysNV", {"VK_NV_ray_tracing"}},
    {"vkCreateRayTracingPipelinesNV", {"VK_NV_ray_tracing"}},
    {"vkGetRayTracingShaderGroupHandlesKHR", {"VK_KHR_ray_tracing_pipeline"}},
    {"vkGetRayTracingShaderGroupHandlesNV", {"VK_NV_ray_tracing"}},
    {"vkGetAccelerationStructureHandleNV", {"VK_NV_ray_tracing"}},
    {"vkCmdWriteAccelerationStructuresPropertiesNV", {"VK_NV_ray_tracing"}},
    {"vkCompileDeferredNV", {"VK_NV_ray_tracing"}},
    {"vkGetMemoryHostPointerPropertiesEXT", {"VK_EXT_external_memory_host"}},
    {"vkCmdWriteBufferMarkerAMD", {"VK_AMD_buffer_marker"}},
    {"vkGetCalibratedTimestampsEXT", {"VK_EXT_calibrated_timestamps"}},
    {"vkCmdDrawMeshTasksNV", {"VK_NV_mesh_shader"}},
    {"vkCmdDrawMeshTasksIndirectNV", {"VK_NV_mesh_shader"}},
    {"vkCmdDrawMeshTasksIndirectCountNV", {"VK_NV_mesh_shader"}},
    {"vkCmdSetExclusiveScissorEnableNV", {"VK_NV_scissor_exclusive"}},
    {"vkCmdSetExclusiveScissorNV", {"VK_NV_scissor_exclusive"}},
    {"vkCmdSetCheckpointNV", {"VK_NV_device_diagnostic_checkpoints"}},
    {"vkGetQueueCheckpointDataNV", {"VK_NV_device_diagnostic_checkpoints"}},
    {"vkInitializePerformanceApiINTEL", {"VK_INTEL_performance_query"}},
    {"vkUninitializePerformanceApiINTEL", {"VK_INTEL_performance_query"}},
    {"vkCmdSetPerformanceMarkerINTEL", {"VK_INTEL_performance_query"}},
    {"vkCmdSetPerformanceStreamMarkerINTEL", {"VK_INTEL_performance_query"}},
    {"vkCmdSetPerformanceOverrideINTEL", {"VK_INTEL_performance_query"}},
    {"vkAcquirePerformanceConfigurationINTEL", {"VK_INTEL_performance_query"}},
    {"vkReleasePerformanceConfigurationINTEL", {"VK_INTEL_performance_query"}},
    {"vkQueueSetPerformanceConfigurationINTEL", {"VK_INTEL_performance_query"}},
    {"vkGetPerformanceParameterINTEL", {"VK_INTEL_performance_query"}},
    {"vkSetLocalDimmingAMD", {"VK_AMD_display_native_hdr"}},
    {"vkGetBufferDeviceAddressEXT", {"VK_EXT_buffer_device_address"}},
    {"vkAcquireFullScreenExclusiveModeEXT", {"VK_EXT_full_screen_exclusive"}},
    {"vkReleaseFullScreenExclusiveModeEXT", {"VK_EXT_full_screen_exclusive"}},
    {"vkGetDeviceGroupSurfacePresentModes2EXT", {"VK_EXT_full_screen_exclusive"}},
    {"vkCmdSetLineStippleEXT", {"VK_EXT_line_rasterization"}},
    {"vkResetQueryPoolEXT", {"VK_EXT_host_query_reset"}},
    {"vkCmdSetCullModeEXT", {"VK_EXT_extended_dynamic_state", "VK_EXT_shader_object"}},
    {"vkCmdSetFrontFaceEXT", {"VK_EXT_extended_dynamic_state", "VK_EXT_shader_object"}},
    {"vkCmdSetPrimitiveTopologyEXT", {"VK_EXT_extended_dynamic_state", "VK_EXT_shader_object"}},
    {"vkCmdSetViewportWithCountEXT", {"VK_EXT_extended_dynamic_state", "VK_EXT_shader_object"}},
    {"vkCmdSetScissorWithCountEXT", {"VK_EXT_extended_dynamic_state", "VK_EXT_shader_object"}},
    {"vkCmdBindVertexBuffers2EXT", {"VK_EXT_extended_dynamic_state", "VK_EXT_shader_object"}},
    {"vkCmdSetDepthTestEnableEXT", {"VK_EXT_extended_dynamic_state", "VK_EXT_shader_object"}},
    {"vkCmdSetDepthWriteEnableEXT", {"VK_EXT_extended_dynamic_state", "VK_EXT_shader_object"}},
    {"vkCmdSetDepthCompareOpEXT", {"VK_EXT_extended_dynamic_state", "VK_EXT_shader_object"}},
    {"vkCmdSetDepthBoundsTestEnableEXT", {"VK_EXT_extended_dynamic_state", "VK_EXT_shader_object"}},
    {"vkCmdSetStencilTestEnableEXT", {"VK_EXT_extended_dynamic_state", "VK_EXT_shader_object"}},
    {"vkCmdSetStencilOpEXT", {"VK_EXT_extended_dynamic_state", "VK_EXT_shader_object"}},
    {"vkCopyMemoryToImageEXT", {"VK_EXT_host_image_copy"}},
    {"vkCopyImageToMemoryEXT", {"VK_EXT_host_image_copy"}},
    {"vkCopyImageToImageEXT", {"VK_EXT_host_image_copy"}},
    {"vkTransitionImageLayoutEXT", {"VK_EXT_host_image_copy"}},
    {"vkGetImageSubresourceLayout2EXT", {"VK_EXT_host_image_copy", "VK_EXT_image_compression_control"}},
    {"vkReleaseSwapchainImagesEXT", {"VK_EXT_swapchain_maintenance1"}},
    {"vkGetGeneratedCommandsMemoryRequirementsNV", {"VK_NV_device_generated_commands"}},
    {"vkCmdPreprocessGeneratedCommandsNV", {"VK_NV_device_generated_commands"}},
    {"vkCmdExecuteGeneratedCommandsNV", {"VK_NV_device_generated_commands"}},
    {"vkCmdBindPipelineShaderGroupNV", {"VK_NV_device_generated_commands"}},
    {"vkCreateIndirectCommandsLayoutNV", {"VK_NV_device_generated_commands"}},
    {"vkDestroyIndirectCommandsLayoutNV", {"VK_NV_device_generated_commands"}},
    {"vkCmdSetDepthBias2EXT", {"VK_EXT_depth_bias_control"}},
    {"vkCreatePrivateDataSlotEXT", {"VK_EXT_private_data"}},
    {"vkDestroyPrivateDataSlotEXT", {"VK_EXT_private_data"}},
    {"vkSetPrivateDataEXT", {"VK_EXT_private_data"}},
    {"vkGetPrivateDataEXT", {"VK_EXT_private_data"}},
    {"vkExportMetalObjectsEXT", {"VK_EXT_metal_objects"}},
    {"vkGetDescriptorSetLayoutSizeEXT", {"VK_EXT_descriptor_buffer"}},
    {"vkGetDescriptorSetLayoutBindingOffsetEXT", {"VK_EXT_descriptor_buffer"}},
    {"vkGetDescriptorEXT", {"VK_EXT_descriptor_buffer"}},
    {"vkCmdBindDescriptorBuffersEXT", {"VK_EXT_descriptor_buffer"}},
    {"vkCmdSetDescriptorBufferOffsetsEXT", {"VK_EXT_descriptor_buffer"}},
    {"vkCmdBindDescriptorBufferEmbeddedSamplersEXT", {"VK_EXT_descriptor_buffer"}},
    {"vkGetBufferOpaqueCaptureDescriptorDataEXT", {"VK_EXT_descriptor_buffer"}},
    {"vkGetImageOpaqueCaptureDescriptorDataEXT", {"VK_EXT_descriptor_buffer"}},
    {"vkGetImageViewOpaqueCaptureDescriptorDataEXT", {"VK_EXT_descriptor_buffer"}},
    {"vkGetSamplerOpaqueCaptureDescriptorDataEXT", {"VK_EXT_descriptor_buffer"}},
    {"vkGetAccelerationStructureOpaqueCaptureDescriptorDataEXT", {"VK_EXT_descriptor_buffer"}},
    {"vkCmdSetFragmentShadingRateEnumNV", {"VK_NV_fragment_shading_rate_enums"}},
    {"vkGetDeviceFaultInfoEXT", {"VK_EXT_device_fault"}},
    {"vkCmdSetVertexInputEXT", {"VK_EXT_vertex_input_dynamic_state", "VK_EXT_shader_object"}},
    {"vkGetMemoryZirconHandleFUCHSIA", {"VK_FUCHSIA_external_memory"}},
    {"vkGetMemoryZirconHandlePropertiesFUCHSIA", {"VK_FUCHSIA_external_memory"}},
    {"vkImportSemaphoreZirconHandleFUCHSIA", {"VK_FUCHSIA_external_semaphore"}},
    {"vkGetSemaphoreZirconHandleFUCHSIA", {"VK_FUCHSIA_external_semaphore"}},
    {"vkCreateBufferCollectionFUCHSIA", {"VK_FUCHSIA_buffer_collection"}},
    {"vkSetBufferCollectionImageConstraintsFUCHSIA", {"VK_FUCHSIA_buffer_collection"}},
    {"vkSetBufferCollectionBufferConstraintsFUCHSIA", {"VK_FUCHSIA_buffer_collection"}},
    {"vkDestroyBufferCollectionFUCHSIA", {"VK_FUCHSIA_buffer_collection"}},
    {"vkGetBufferCollectionPropertiesFUCHSIA", {"VK_FUCHSIA_buffer_collection"}},
    {"vkGetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI", {"VK_HUAWEI_subpass_shading"}},
    {"vkCmdSubpassShadingHUAWEI", {"VK_HUAWEI_subpass_shading"}},
    {"vkCmdBindInvocationMaskHUAWEI", {"VK_HUAWEI_invocation_mask"}},
    {"vkGetMemoryRemoteAddressNV", {"VK_NV_external_memory_rdma"}},
    {"vkGetPipelinePropertiesEXT", {"VK_EXT_pipeline_properties"}},
    {"vkCmdSetPatchControlPointsEXT", {"VK_EXT_extended_dynamic_state2", "VK_EXT_shader_object"}},
    {"vkCmdSetRasterizerDiscardEnableEXT", {"VK_EXT_extended_dynamic_state2", "VK_EXT_shader_object"}},
    {"vkCmdSetDepthBiasEnableEXT", {"VK_EXT_extended_dynamic_state2", "VK_EXT_shader_object"}},
    {"vkCmdSetLogicOpEXT", {"VK_EXT_extended_dynamic_state2", "VK_EXT_shader_object"}},
    {"vkCmdSetPrimitiveRestartEnableEXT", {"VK_EXT_extended_dynamic_state2", "VK_EXT_shader_object"}},
    {"vkCmdSetColorWriteEnableEXT", {"VK_EXT_color_write_enable"}},
    {"vkCmdDrawMultiEXT", {"VK_EXT_multi_draw"}},
    {"vkCmdDrawMultiIndexedEXT", {"VK_EXT_multi_draw"}},
    {"vkCreateMicromapEXT", {"VK_EXT_opacity_micromap"}},
    {"vkDestroyMicromapEXT", {"VK_EXT_opacity_micromap"}},
    {"vkCmdBuildMicromapsEXT", {"VK_EXT_opacity_micromap"}},
    {"vkBuildMicromapsEXT", {"VK_EXT_opacity_micromap"}},
    {"vkCopyMicromapEXT", {"VK_EXT_opacity_micromap"}},
    {"vkCopyMicromapToMemoryEXT", {"VK_EXT_opacity_micromap"}},
    {"vkCopyMemoryToMicromapEXT", {"VK_EXT_opacity_micromap"}},
    {"vkWriteMicromapsPropertiesEXT", {"VK_EXT_opacity_micromap"}},
    {"vkCmdCopyMicromapEXT", {"VK_EXT_opacity_micromap"}},
    {"vkCmdCopyMicromapToMemoryEXT", {"VK_EXT_opacity_micromap"}},
    {"vkCmdCopyMemoryToMicromapEXT", {"VK_EXT_opacity_micromap"}},
    {"vkCmdWriteMicromapsPropertiesEXT", {"VK_EXT_opacity_micromap"}},
    {"vkGetDeviceMicromapCompatibilityEXT", {"VK_EXT_opacity_micromap"}},
    {"vkGetMicromapBuildSizesEXT", {"VK_EXT_opacity_micromap"}},
    {"vkCmdDrawClusterHUAWEI", {"VK_HUAWEI_cluster_culling_shader"}},
    {"vkCmdDrawClusterIndirectHUAWEI", {"VK_HUAWEI_cluster_culling_shader"}},
    {"vkSetDeviceMemoryPriorityEXT", {"VK_EXT_pageable_device_local_memory"}},
    {"vkGetDescriptorSetLayoutHostMappingInfoVALVE", {"VK_VALVE_descriptor_set_host_mapping"}},
    {"vkGetDescriptorSetHostMappingVALVE", {"VK_VALVE_descriptor_set_host_mapping"}},
    {"vkCmdCopyMemoryIndirectNV", {"VK_NV_copy_memory_indirect"}},
    {"vkCmdCopyMemoryToImageIndirectNV", {"VK_NV_copy_memory_indirect"}},
    {"vkCmdDecompressMemoryNV", {"VK_NV_memory_decompression"}},
    {"vkCmdDecompressMemoryIndirectCountNV", {"VK_NV_memory_decompression"}},
    {"vkGetPipelineIndirectMemoryRequirementsNV", {"VK_NV_device_generated_commands_compute"}},
    {"vkCmdUpdatePipelineIndirectBufferNV", {"VK_NV_device_generated_commands_compute"}},
    {"vkGetPipelineIndirectDeviceAddressNV", {"VK_NV_device_generated_commands_compute"}},
    {"vkCmdSetTessellationDomainOriginEXT", {"VK_EXT_extended_dynamic_state3", "VK_EXT_shader_object"}},
    {"vkCmdSetDepthClampEnableEXT", {"VK_EXT_extended_dynamic_state3", "VK_EXT_shader_object"}},
    {"vkCmdSetPolygonModeEXT", {"VK_EXT_extended_dynamic_state3", "VK_EXT_shader_object"}},
    {"vkCmdSetRasterizationSamplesEXT", {"VK_EXT_extended_dynamic_state3", "VK_EXT_shader_object"}},
    {"vkCmdSetSampleMaskEXT", {"VK_EXT_extended_dynamic_state3", "VK_EXT_shader_object"}},
    {"vkCmdSetAlphaToCoverageEnableEXT", {"VK_EXT_extended_dynamic_state3", "VK_EXT_shader_object"}},
    {"vkCmdSetAlphaToOneEnableEXT", {"VK_EXT_extended_dynamic_state3", "VK_EXT_shader_object"}},
    {"vkCmdSetLogicOpEnableEXT", {"VK_EXT_extended_dynamic_state3", "VK_EXT_shader_object"}},
    {"vkCmdSetColorBlendEnableEXT", {"VK_EXT_extended_dynamic_state3", "VK_EXT_shader_object"}},
    {"vkCmdSetColorBlendEquationEXT", {"VK_EXT_extended_dynamic_state3", "VK_EXT_shader_object"}},
    {"vkCmdSetColorWriteMaskEXT", {"VK_EXT_extended_dynamic_state3", "VK_EXT_shader_object"}},
    {"vkCmdSetRasterizationStreamEXT", {"VK_EXT_extended_dynamic_state3", "VK_EXT_shader_object"}},
    {"vkCmdSetConservativeRasterizationModeEXT", {"VK_EXT_extended_dynamic_state3", "VK_EXT_shader_object"}},
    {"vkCmdSetExtraPrimitiveOverestimationSizeEXT", {"VK_EXT_extended_dynamic_state3", "VK_EXT_shader_object"}},
    {"vkCmdSetDepthClipEnableEXT", {"VK_EXT_extended_dynamic_state3", "VK_EXT_shader_object"}},
    {"vkCmdSetSampleLocationsEnableEXT", {"VK_EXT_extended_dynamic_state3", "VK_EXT_shader_object"}},
    {"vkCmdSetColorBlendAdvancedEXT", {"VK_EXT_extended_dynamic_state3", "VK_EXT_shader_object"}},
    {"vkCmdSetProvokingVertexModeEXT", {"VK_EXT_extended_dynamic_state3", "VK_EXT_shader_object"}},
    {"vkCmdSetLineRasterizationModeEXT", {"VK_EXT_extended_dynamic_state3", "VK_EXT_shader_object"}},
    {"vkCmdSetLineStippleEnableEXT", {"VK_EXT_extended_dynamic_state3", "VK_EXT_shader_object"}},
    {"vkCmdSetDepthClipNegativeOneToOneEXT", {"VK_EXT_extended_dynamic_state3", "VK_EXT_shader_object"}},
    {"vkCmdSetViewportWScalingEnableNV", {"VK_EXT_extended_dynamic_state3", "VK_EXT_shader_object"}},
    {"vkCmdSetViewportSwizzleNV", {"VK_EXT_extended_dynamic_state3", "VK_EXT_shader_object"}},
    {"vkCmdSetCoverageToColorEnableNV", {"VK_EXT_extended_dynamic_state3", "VK_EXT_shader_object"}},
    {"vkCmdSetCoverageToColorLocationNV", {"VK_EXT_extended_dynamic_state3", "VK_EXT_shader_object"}},
    {"vkCmdSetCoverageModulationModeNV", {"VK_EXT_extended_dynamic_state3", "VK_EXT_shader_object"}},
    {"vkCmdSetCoverageModulationTableEnableNV", {"VK_EXT_extended_dynamic_state3", "VK_EXT_shader_object"}},
    {"vkCmdSetCoverageModulationTableNV", {"VK_EXT_extended_dynamic_state3", "VK_EXT_shader_object"}},
    {"vkCmdSetShadingRateImageEnableNV", {"VK_EXT_extended_dynamic_state3", "VK_EXT_shader_object"}},
    {"vkCmdSetRepresentativeFragmentTestEnableNV", {"VK_EXT_extended_dynamic_state3", "VK_EXT_shader_object"}},
    {"vkCmdSetCoverageReductionModeNV", {"VK_EXT_extended_dynamic_state3", "VK_EXT_shader_object"}},
    {"vkGetShaderModuleIdentifierEXT", {"VK_EXT_shader_module_identifier"}},
    {"vkGetShaderModuleCreateInfoIdentifierEXT", {"VK_EXT_shader_module_identifier"}},
    {"vkCreateOpticalFlowSessionNV", {"VK_NV_optical_flow"}},
    {"vkDestroyOpticalFlowSessionNV", {"VK_NV_optical_flow"}},
    {"vkBindOpticalFlowSessionImageNV", {"VK_NV_optical_flow"}},
    {"vkCmdOpticalFlowExecuteNV", {"VK_NV_optical_flow"}},
    {"vkCreateShadersEXT", {"VK_EXT_shader_object"}},
    {"vkDestroyShaderEXT", {"VK_EXT_shader_object"}},
    {"vkGetShaderBinaryDataEXT", {"VK_EXT_shader_object"}},
    {"vkCmdBindShadersEXT", {"VK_EXT_shader_object"}},
    {"vkGetFramebufferTilePropertiesQCOM", {"VK_QCOM_tile_properties"}},
    {"vkGetDynamicRenderingTilePropertiesQCOM", {"VK_QCOM_tile_properties"}},
    {"vkSetLatencySleepModeNV", {"VK_NV_low_latency2"}},
    {"vkLatencySleepNV", {"VK_NV_low_latency2"}},
    {"vkSetLatencyMarkerNV", {"VK_NV_low_latency2"}},
    {"vkGetLatencyTimingsNV", {"VK_NV_low_latency2"}},
    {"vkQueueNotifyOutOfBandNV", {"VK_NV_low_latency2"}},
    {"vkCmdSetAttachmentFeedbackLoopEnableEXT", {"VK_EXT_attachment_feedback_loop_dynamic_state"}},
    {"vkGetScreenBufferPropertiesQNX", {"VK_QNX_external_memory_screen_buffer"}},
    {"vkCreateAccelerationStructureKHR", {"VK_KHR_acceleration_structure"}},
    {"vkDestroyAccelerationStructureKHR", {"VK_KHR_acceleration_structure"}},
    {"vkCmdBuildAccelerationStructuresKHR", {"VK_KHR_acceleration_structure"}},
    {"vkCmdBuildAccelerationStructuresIndirectKHR", {"VK_KHR_acceleration_structure"}},
    {"vkBuildAccelerationStructuresKHR", {"VK_KHR_acceleration_structure"}},
    {"vkCopyAccelerationStructureKHR", {"VK_KHR_acceleration_structure"}},
    {"vkCopyAccelerationStructureToMemoryKHR", {"VK_KHR_acceleration_structure"}},
    {"vkCopyMemoryToAccelerationStructureKHR", {"VK_KHR_acceleration_structure"}},
    {"vkWriteAccelerationStructuresPropertiesKHR", {"VK_KHR_acceleration_structure"}},
    {"vkCmdCopyAccelerationStructureKHR", {"VK_KHR_acceleration_structure"}},
    {"vkCmdCopyAccelerationStructureToMemoryKHR", {"VK_KHR_acceleration_structure"}},
    {"vkCmdCopyMemoryToAccelerationStructureKHR", {"VK_KHR_acceleration_structure"}},
    {"vkGetAccelerationStructureDeviceAddressKHR", {"VK_KHR_acceleration_structure"}},
    {"vkCmdWriteAccelerationStructuresPropertiesKHR", {"VK_KHR_acceleration_structure"}},
    {"vkGetDeviceAccelerationStructureCompatibilityKHR", {"VK_KHR_acceleration_structure"}},
    {"vkGetAccelerationStructureBuildSizesKHR", {"VK_KHR_acceleration_structure"}},
    {"vkCmdTraceRaysKHR", {"VK_KHR_ray_tracing_pipeline"}},
    {"vkCreateRayTracingPipelinesKHR", {"VK_KHR_ray_tracing_pipeline"}},
    {"vkGetRayTracingCaptureReplayShaderGroupHandlesKHR", {"VK_KHR_ray_tracing_pipeline"}},
    {"vkCmdTraceRaysIndirectKHR", {"VK_KHR_ray_tracing_pipeline"}},
    {"vkGetRayTracingShaderGroupStackSizeKHR", {"VK_KHR_ray_tracing_pipeline"}},
    {"vkCmdSetRayTracingPipelineStackSizeKHR", {"VK_KHR_ray_tracing_pipeline"}},
    {"vkCmdDrawMeshTasksEXT", {"VK_EXT_mesh_shader"}},
    {"vkCmdDrawMeshTasksIndirectEXT", {"VK_EXT_mesh_shader"}},
    {"vkCmdDrawMeshTasksIndirectCountEXT", {"VK_EXT_mesh_shader"}},
};

// Using the above code-generated map of APINames-to-parent extension names, this function will:
//   o  Determine if the API has an associated extension
//   o  If it does, determine if that extension name is present in the passed-in set of device or instance enabled_ext_names
//   If the APIname has no parent extension, OR its parent extension name is IN one of the sets, return TRUE, else FALSE
static inline bool ApiParentExtensionEnabled(const std::string api_name, const DeviceExtensions* device_extension_info) {
    auto has_ext = api_extension_map.find(api_name);
    // Is this API part of an extension or feature group?
    if (has_ext != api_extension_map.end()) {
        // Was the extension for this API enabled in the CreateDevice call?
        for (const auto& ext : has_ext->second) {
            auto info = device_extension_info->get_info(ext.c_str());
            if (info.state) {
                if (device_extension_info->*(info.state) == kEnabledByCreateinfo ||
                    device_extension_info->*(info.state) == kEnabledByInteraction) {
                    return true;
                }
            }
        }

        // Was the extension for this API enabled in the CreateInstance call?
        auto instance_extension_info = static_cast<const InstanceExtensions*>(device_extension_info);
        for (const auto& ext : has_ext->second) {
            auto inst_info = instance_extension_info->get_info(ext.c_str());
            if (inst_info.state) {
                if (instance_extension_info->*(inst_info.state) == kEnabledByCreateinfo ||
                    instance_extension_info->*(inst_info.state) == kEnabledByInteraction) {
                    return true;
                }
            }
        }
        return false;
    }
    return true;
}

static inline void layer_init_device_dispatch_table(VkDevice device, VkLayerDispatchTable* table, PFN_vkGetDeviceProcAddr gpa) {
    memset(table, 0, sizeof(*table));
    // Device function pointers
    table->GetDeviceProcAddr = gpa;
    table->DestroyDevice = (PFN_vkDestroyDevice)gpa(device, "vkDestroyDevice");
    table->GetDeviceQueue = (PFN_vkGetDeviceQueue)gpa(device, "vkGetDeviceQueue");
    table->QueueSubmit = (PFN_vkQueueSubmit)gpa(device, "vkQueueSubmit");
    table->QueueWaitIdle = (PFN_vkQueueWaitIdle)gpa(device, "vkQueueWaitIdle");
    table->DeviceWaitIdle = (PFN_vkDeviceWaitIdle)gpa(device, "vkDeviceWaitIdle");
    table->AllocateMemory = (PFN_vkAllocateMemory)gpa(device, "vkAllocateMemory");
    table->FreeMemory = (PFN_vkFreeMemory)gpa(device, "vkFreeMemory");
    table->MapMemory = (PFN_vkMapMemory)gpa(device, "vkMapMemory");
    table->UnmapMemory = (PFN_vkUnmapMemory)gpa(device, "vkUnmapMemory");
    table->FlushMappedMemoryRanges = (PFN_vkFlushMappedMemoryRanges)gpa(device, "vkFlushMappedMemoryRanges");
    table->InvalidateMappedMemoryRanges = (PFN_vkInvalidateMappedMemoryRanges)gpa(device, "vkInvalidateMappedMemoryRanges");
    table->GetDeviceMemoryCommitment = (PFN_vkGetDeviceMemoryCommitment)gpa(device, "vkGetDeviceMemoryCommitment");
    table->BindBufferMemory = (PFN_vkBindBufferMemory)gpa(device, "vkBindBufferMemory");
    table->BindImageMemory = (PFN_vkBindImageMemory)gpa(device, "vkBindImageMemory");
    table->GetBufferMemoryRequirements = (PFN_vkGetBufferMemoryRequirements)gpa(device, "vkGetBufferMemoryRequirements");
    table->GetImageMemoryRequirements = (PFN_vkGetImageMemoryRequirements)gpa(device, "vkGetImageMemoryRequirements");
    table->GetImageSparseMemoryRequirements =
        (PFN_vkGetImageSparseMemoryRequirements)gpa(device, "vkGetImageSparseMemoryRequirements");
    table->QueueBindSparse = (PFN_vkQueueBindSparse)gpa(device, "vkQueueBindSparse");
    table->CreateFence = (PFN_vkCreateFence)gpa(device, "vkCreateFence");
    table->DestroyFence = (PFN_vkDestroyFence)gpa(device, "vkDestroyFence");
    table->ResetFences = (PFN_vkResetFences)gpa(device, "vkResetFences");
    table->GetFenceStatus = (PFN_vkGetFenceStatus)gpa(device, "vkGetFenceStatus");
    table->WaitForFences = (PFN_vkWaitForFences)gpa(device, "vkWaitForFences");
    table->CreateSemaphore = (PFN_vkCreateSemaphore)gpa(device, "vkCreateSemaphore");
    table->DestroySemaphore = (PFN_vkDestroySemaphore)gpa(device, "vkDestroySemaphore");
    table->CreateEvent = (PFN_vkCreateEvent)gpa(device, "vkCreateEvent");
    table->DestroyEvent = (PFN_vkDestroyEvent)gpa(device, "vkDestroyEvent");
    table->GetEventStatus = (PFN_vkGetEventStatus)gpa(device, "vkGetEventStatus");
    table->SetEvent = (PFN_vkSetEvent)gpa(device, "vkSetEvent");
    table->ResetEvent = (PFN_vkResetEvent)gpa(device, "vkResetEvent");
    table->CreateQueryPool = (PFN_vkCreateQueryPool)gpa(device, "vkCreateQueryPool");
    table->DestroyQueryPool = (PFN_vkDestroyQueryPool)gpa(device, "vkDestroyQueryPool");
    table->GetQueryPoolResults = (PFN_vkGetQueryPoolResults)gpa(device, "vkGetQueryPoolResults");
    table->CreateBuffer = (PFN_vkCreateBuffer)gpa(device, "vkCreateBuffer");
    table->DestroyBuffer = (PFN_vkDestroyBuffer)gpa(device, "vkDestroyBuffer");
    table->CreateBufferView = (PFN_vkCreateBufferView)gpa(device, "vkCreateBufferView");
    table->DestroyBufferView = (PFN_vkDestroyBufferView)gpa(device, "vkDestroyBufferView");
    table->CreateImage = (PFN_vkCreateImage)gpa(device, "vkCreateImage");
    table->DestroyImage = (PFN_vkDestroyImage)gpa(device, "vkDestroyImage");
    table->GetImageSubresourceLayout = (PFN_vkGetImageSubresourceLayout)gpa(device, "vkGetImageSubresourceLayout");
    table->CreateImageView = (PFN_vkCreateImageView)gpa(device, "vkCreateImageView");
    table->DestroyImageView = (PFN_vkDestroyImageView)gpa(device, "vkDestroyImageView");
    table->CreateShaderModule = (PFN_vkCreateShaderModule)gpa(device, "vkCreateShaderModule");
    table->DestroyShaderModule = (PFN_vkDestroyShaderModule)gpa(device, "vkDestroyShaderModule");
    table->CreatePipelineCache = (PFN_vkCreatePipelineCache)gpa(device, "vkCreatePipelineCache");
    table->DestroyPipelineCache = (PFN_vkDestroyPipelineCache)gpa(device, "vkDestroyPipelineCache");
    table->GetPipelineCacheData = (PFN_vkGetPipelineCacheData)gpa(device, "vkGetPipelineCacheData");
    table->MergePipelineCaches = (PFN_vkMergePipelineCaches)gpa(device, "vkMergePipelineCaches");
    table->CreateGraphicsPipelines = (PFN_vkCreateGraphicsPipelines)gpa(device, "vkCreateGraphicsPipelines");
    table->CreateComputePipelines = (PFN_vkCreateComputePipelines)gpa(device, "vkCreateComputePipelines");
    table->DestroyPipeline = (PFN_vkDestroyPipeline)gpa(device, "vkDestroyPipeline");
    table->CreatePipelineLayout = (PFN_vkCreatePipelineLayout)gpa(device, "vkCreatePipelineLayout");
    table->DestroyPipelineLayout = (PFN_vkDestroyPipelineLayout)gpa(device, "vkDestroyPipelineLayout");
    table->CreateSampler = (PFN_vkCreateSampler)gpa(device, "vkCreateSampler");
    table->DestroySampler = (PFN_vkDestroySampler)gpa(device, "vkDestroySampler");
    table->CreateDescriptorSetLayout = (PFN_vkCreateDescriptorSetLayout)gpa(device, "vkCreateDescriptorSetLayout");
    table->DestroyDescriptorSetLayout = (PFN_vkDestroyDescriptorSetLayout)gpa(device, "vkDestroyDescriptorSetLayout");
    table->CreateDescriptorPool = (PFN_vkCreateDescriptorPool)gpa(device, "vkCreateDescriptorPool");
    table->DestroyDescriptorPool = (PFN_vkDestroyDescriptorPool)gpa(device, "vkDestroyDescriptorPool");
    table->ResetDescriptorPool = (PFN_vkResetDescriptorPool)gpa(device, "vkResetDescriptorPool");
    table->AllocateDescriptorSets = (PFN_vkAllocateDescriptorSets)gpa(device, "vkAllocateDescriptorSets");
    table->FreeDescriptorSets = (PFN_vkFreeDescriptorSets)gpa(device, "vkFreeDescriptorSets");
    table->UpdateDescriptorSets = (PFN_vkUpdateDescriptorSets)gpa(device, "vkUpdateDescriptorSets");
    table->CreateFramebuffer = (PFN_vkCreateFramebuffer)gpa(device, "vkCreateFramebuffer");
    table->DestroyFramebuffer = (PFN_vkDestroyFramebuffer)gpa(device, "vkDestroyFramebuffer");
    table->CreateRenderPass = (PFN_vkCreateRenderPass)gpa(device, "vkCreateRenderPass");
    table->DestroyRenderPass = (PFN_vkDestroyRenderPass)gpa(device, "vkDestroyRenderPass");
    table->GetRenderAreaGranularity = (PFN_vkGetRenderAreaGranularity)gpa(device, "vkGetRenderAreaGranularity");
    table->CreateCommandPool = (PFN_vkCreateCommandPool)gpa(device, "vkCreateCommandPool");
    table->DestroyCommandPool = (PFN_vkDestroyCommandPool)gpa(device, "vkDestroyCommandPool");
    table->ResetCommandPool = (PFN_vkResetCommandPool)gpa(device, "vkResetCommandPool");
    table->AllocateCommandBuffers = (PFN_vkAllocateCommandBuffers)gpa(device, "vkAllocateCommandBuffers");
    table->FreeCommandBuffers = (PFN_vkFreeCommandBuffers)gpa(device, "vkFreeCommandBuffers");
    table->BeginCommandBuffer = (PFN_vkBeginCommandBuffer)gpa(device, "vkBeginCommandBuffer");
    table->EndCommandBuffer = (PFN_vkEndCommandBuffer)gpa(device, "vkEndCommandBuffer");
    table->ResetCommandBuffer = (PFN_vkResetCommandBuffer)gpa(device, "vkResetCommandBuffer");
    table->CmdBindPipeline = (PFN_vkCmdBindPipeline)gpa(device, "vkCmdBindPipeline");
    table->CmdSetViewport = (PFN_vkCmdSetViewport)gpa(device, "vkCmdSetViewport");
    table->CmdSetScissor = (PFN_vkCmdSetScissor)gpa(device, "vkCmdSetScissor");
    table->CmdSetLineWidth = (PFN_vkCmdSetLineWidth)gpa(device, "vkCmdSetLineWidth");
    table->CmdSetDepthBias = (PFN_vkCmdSetDepthBias)gpa(device, "vkCmdSetDepthBias");
    table->CmdSetBlendConstants = (PFN_vkCmdSetBlendConstants)gpa(device, "vkCmdSetBlendConstants");
    table->CmdSetDepthBounds = (PFN_vkCmdSetDepthBounds)gpa(device, "vkCmdSetDepthBounds");
    table->CmdSetStencilCompareMask = (PFN_vkCmdSetStencilCompareMask)gpa(device, "vkCmdSetStencilCompareMask");
    table->CmdSetStencilWriteMask = (PFN_vkCmdSetStencilWriteMask)gpa(device, "vkCmdSetStencilWriteMask");
    table->CmdSetStencilReference = (PFN_vkCmdSetStencilReference)gpa(device, "vkCmdSetStencilReference");
    table->CmdBindDescriptorSets = (PFN_vkCmdBindDescriptorSets)gpa(device, "vkCmdBindDescriptorSets");
    table->CmdBindIndexBuffer = (PFN_vkCmdBindIndexBuffer)gpa(device, "vkCmdBindIndexBuffer");
    table->CmdBindVertexBuffers = (PFN_vkCmdBindVertexBuffers)gpa(device, "vkCmdBindVertexBuffers");
    table->CmdDraw = (PFN_vkCmdDraw)gpa(device, "vkCmdDraw");
    table->CmdDrawIndexed = (PFN_vkCmdDrawIndexed)gpa(device, "vkCmdDrawIndexed");
    table->CmdDrawIndirect = (PFN_vkCmdDrawIndirect)gpa(device, "vkCmdDrawIndirect");
    table->CmdDrawIndexedIndirect = (PFN_vkCmdDrawIndexedIndirect)gpa(device, "vkCmdDrawIndexedIndirect");
    table->CmdDispatch = (PFN_vkCmdDispatch)gpa(device, "vkCmdDispatch");
    table->CmdDispatchIndirect = (PFN_vkCmdDispatchIndirect)gpa(device, "vkCmdDispatchIndirect");
    table->CmdCopyBuffer = (PFN_vkCmdCopyBuffer)gpa(device, "vkCmdCopyBuffer");
    table->CmdCopyImage = (PFN_vkCmdCopyImage)gpa(device, "vkCmdCopyImage");
    table->CmdBlitImage = (PFN_vkCmdBlitImage)gpa(device, "vkCmdBlitImage");
    table->CmdCopyBufferToImage = (PFN_vkCmdCopyBufferToImage)gpa(device, "vkCmdCopyBufferToImage");
    table->CmdCopyImageToBuffer = (PFN_vkCmdCopyImageToBuffer)gpa(device, "vkCmdCopyImageToBuffer");
    table->CmdUpdateBuffer = (PFN_vkCmdUpdateBuffer)gpa(device, "vkCmdUpdateBuffer");
    table->CmdFillBuffer = (PFN_vkCmdFillBuffer)gpa(device, "vkCmdFillBuffer");
    table->CmdClearColorImage = (PFN_vkCmdClearColorImage)gpa(device, "vkCmdClearColorImage");
    table->CmdClearDepthStencilImage = (PFN_vkCmdClearDepthStencilImage)gpa(device, "vkCmdClearDepthStencilImage");
    table->CmdClearAttachments = (PFN_vkCmdClearAttachments)gpa(device, "vkCmdClearAttachments");
    table->CmdResolveImage = (PFN_vkCmdResolveImage)gpa(device, "vkCmdResolveImage");
    table->CmdSetEvent = (PFN_vkCmdSetEvent)gpa(device, "vkCmdSetEvent");
    table->CmdResetEvent = (PFN_vkCmdResetEvent)gpa(device, "vkCmdResetEvent");
    table->CmdWaitEvents = (PFN_vkCmdWaitEvents)gpa(device, "vkCmdWaitEvents");
    table->CmdPipelineBarrier = (PFN_vkCmdPipelineBarrier)gpa(device, "vkCmdPipelineBarrier");
    table->CmdBeginQuery = (PFN_vkCmdBeginQuery)gpa(device, "vkCmdBeginQuery");
    table->CmdEndQuery = (PFN_vkCmdEndQuery)gpa(device, "vkCmdEndQuery");
    table->CmdResetQueryPool = (PFN_vkCmdResetQueryPool)gpa(device, "vkCmdResetQueryPool");
    table->CmdWriteTimestamp = (PFN_vkCmdWriteTimestamp)gpa(device, "vkCmdWriteTimestamp");
    table->CmdCopyQueryPoolResults = (PFN_vkCmdCopyQueryPoolResults)gpa(device, "vkCmdCopyQueryPoolResults");
    table->CmdPushConstants = (PFN_vkCmdPushConstants)gpa(device, "vkCmdPushConstants");
    table->CmdBeginRenderPass = (PFN_vkCmdBeginRenderPass)gpa(device, "vkCmdBeginRenderPass");
    table->CmdNextSubpass = (PFN_vkCmdNextSubpass)gpa(device, "vkCmdNextSubpass");
    table->CmdEndRenderPass = (PFN_vkCmdEndRenderPass)gpa(device, "vkCmdEndRenderPass");
    table->CmdExecuteCommands = (PFN_vkCmdExecuteCommands)gpa(device, "vkCmdExecuteCommands");
    table->BindBufferMemory2 = (PFN_vkBindBufferMemory2)gpa(device, "vkBindBufferMemory2");
    if (table->BindBufferMemory2 == nullptr) {
        table->BindBufferMemory2 = (PFN_vkBindBufferMemory2)StubBindBufferMemory2;
    }
    table->BindImageMemory2 = (PFN_vkBindImageMemory2)gpa(device, "vkBindImageMemory2");
    if (table->BindImageMemory2 == nullptr) {
        table->BindImageMemory2 = (PFN_vkBindImageMemory2)StubBindImageMemory2;
    }
    table->GetDeviceGroupPeerMemoryFeatures =
        (PFN_vkGetDeviceGroupPeerMemoryFeatures)gpa(device, "vkGetDeviceGroupPeerMemoryFeatures");
    if (table->GetDeviceGroupPeerMemoryFeatures == nullptr) {
        table->GetDeviceGroupPeerMemoryFeatures = (PFN_vkGetDeviceGroupPeerMemoryFeatures)StubGetDeviceGroupPeerMemoryFeatures;
    }
    table->CmdSetDeviceMask = (PFN_vkCmdSetDeviceMask)gpa(device, "vkCmdSetDeviceMask");
    if (table->CmdSetDeviceMask == nullptr) {
        table->CmdSetDeviceMask = (PFN_vkCmdSetDeviceMask)StubCmdSetDeviceMask;
    }
    table->CmdDispatchBase = (PFN_vkCmdDispatchBase)gpa(device, "vkCmdDispatchBase");
    if (table->CmdDispatchBase == nullptr) {
        table->CmdDispatchBase = (PFN_vkCmdDispatchBase)StubCmdDispatchBase;
    }
    table->GetImageMemoryRequirements2 = (PFN_vkGetImageMemoryRequirements2)gpa(device, "vkGetImageMemoryRequirements2");
    if (table->GetImageMemoryRequirements2 == nullptr) {
        table->GetImageMemoryRequirements2 = (PFN_vkGetImageMemoryRequirements2)StubGetImageMemoryRequirements2;
    }
    table->GetBufferMemoryRequirements2 = (PFN_vkGetBufferMemoryRequirements2)gpa(device, "vkGetBufferMemoryRequirements2");
    if (table->GetBufferMemoryRequirements2 == nullptr) {
        table->GetBufferMemoryRequirements2 = (PFN_vkGetBufferMemoryRequirements2)StubGetBufferMemoryRequirements2;
    }
    table->GetImageSparseMemoryRequirements2 =
        (PFN_vkGetImageSparseMemoryRequirements2)gpa(device, "vkGetImageSparseMemoryRequirements2");
    if (table->GetImageSparseMemoryRequirements2 == nullptr) {
        table->GetImageSparseMemoryRequirements2 = (PFN_vkGetImageSparseMemoryRequirements2)StubGetImageSparseMemoryRequirements2;
    }
    table->TrimCommandPool = (PFN_vkTrimCommandPool)gpa(device, "vkTrimCommandPool");
    if (table->TrimCommandPool == nullptr) {
        table->TrimCommandPool = (PFN_vkTrimCommandPool)StubTrimCommandPool;
    }
    table->GetDeviceQueue2 = (PFN_vkGetDeviceQueue2)gpa(device, "vkGetDeviceQueue2");
    if (table->GetDeviceQueue2 == nullptr) {
        table->GetDeviceQueue2 = (PFN_vkGetDeviceQueue2)StubGetDeviceQueue2;
    }
    table->CreateSamplerYcbcrConversion = (PFN_vkCreateSamplerYcbcrConversion)gpa(device, "vkCreateSamplerYcbcrConversion");
    if (table->CreateSamplerYcbcrConversion == nullptr) {
        table->CreateSamplerYcbcrConversion = (PFN_vkCreateSamplerYcbcrConversion)StubCreateSamplerYcbcrConversion;
    }
    table->DestroySamplerYcbcrConversion = (PFN_vkDestroySamplerYcbcrConversion)gpa(device, "vkDestroySamplerYcbcrConversion");
    if (table->DestroySamplerYcbcrConversion == nullptr) {
        table->DestroySamplerYcbcrConversion = (PFN_vkDestroySamplerYcbcrConversion)StubDestroySamplerYcbcrConversion;
    }
    table->CreateDescriptorUpdateTemplate = (PFN_vkCreateDescriptorUpdateTemplate)gpa(device, "vkCreateDescriptorUpdateTemplate");
    if (table->CreateDescriptorUpdateTemplate == nullptr) {
        table->CreateDescriptorUpdateTemplate = (PFN_vkCreateDescriptorUpdateTemplate)StubCreateDescriptorUpdateTemplate;
    }
    table->DestroyDescriptorUpdateTemplate =
        (PFN_vkDestroyDescriptorUpdateTemplate)gpa(device, "vkDestroyDescriptorUpdateTemplate");
    if (table->DestroyDescriptorUpdateTemplate == nullptr) {
        table->DestroyDescriptorUpdateTemplate = (PFN_vkDestroyDescriptorUpdateTemplate)StubDestroyDescriptorUpdateTemplate;
    }
    table->UpdateDescriptorSetWithTemplate =
        (PFN_vkUpdateDescriptorSetWithTemplate)gpa(device, "vkUpdateDescriptorSetWithTemplate");
    if (table->UpdateDescriptorSetWithTemplate == nullptr) {
        table->UpdateDescriptorSetWithTemplate = (PFN_vkUpdateDescriptorSetWithTemplate)StubUpdateDescriptorSetWithTemplate;
    }
    table->GetDescriptorSetLayoutSupport = (PFN_vkGetDescriptorSetLayoutSupport)gpa(device, "vkGetDescriptorSetLayoutSupport");
    if (table->GetDescriptorSetLayoutSupport == nullptr) {
        table->GetDescriptorSetLayoutSupport = (PFN_vkGetDescriptorSetLayoutSupport)StubGetDescriptorSetLayoutSupport;
    }
    table->CmdDrawIndirectCount = (PFN_vkCmdDrawIndirectCount)gpa(device, "vkCmdDrawIndirectCount");
    if (table->CmdDrawIndirectCount == nullptr) {
        table->CmdDrawIndirectCount = (PFN_vkCmdDrawIndirectCount)StubCmdDrawIndirectCount;
    }
    table->CmdDrawIndexedIndirectCount = (PFN_vkCmdDrawIndexedIndirectCount)gpa(device, "vkCmdDrawIndexedIndirectCount");
    if (table->CmdDrawIndexedIndirectCount == nullptr) {
        table->CmdDrawIndexedIndirectCount = (PFN_vkCmdDrawIndexedIndirectCount)StubCmdDrawIndexedIndirectCount;
    }
    table->CreateRenderPass2 = (PFN_vkCreateRenderPass2)gpa(device, "vkCreateRenderPass2");
    if (table->CreateRenderPass2 == nullptr) {
        table->CreateRenderPass2 = (PFN_vkCreateRenderPass2)StubCreateRenderPass2;
    }
    table->CmdBeginRenderPass2 = (PFN_vkCmdBeginRenderPass2)gpa(device, "vkCmdBeginRenderPass2");
    if (table->CmdBeginRenderPass2 == nullptr) {
        table->CmdBeginRenderPass2 = (PFN_vkCmdBeginRenderPass2)StubCmdBeginRenderPass2;
    }
    table->CmdNextSubpass2 = (PFN_vkCmdNextSubpass2)gpa(device, "vkCmdNextSubpass2");
    if (table->CmdNextSubpass2 == nullptr) {
        table->CmdNextSubpass2 = (PFN_vkCmdNextSubpass2)StubCmdNextSubpass2;
    }
    table->CmdEndRenderPass2 = (PFN_vkCmdEndRenderPass2)gpa(device, "vkCmdEndRenderPass2");
    if (table->CmdEndRenderPass2 == nullptr) {
        table->CmdEndRenderPass2 = (PFN_vkCmdEndRenderPass2)StubCmdEndRenderPass2;
    }
    table->ResetQueryPool = (PFN_vkResetQueryPool)gpa(device, "vkResetQueryPool");
    if (table->ResetQueryPool == nullptr) {
        table->ResetQueryPool = (PFN_vkResetQueryPool)StubResetQueryPool;
    }
    table->GetSemaphoreCounterValue = (PFN_vkGetSemaphoreCounterValue)gpa(device, "vkGetSemaphoreCounterValue");
    if (table->GetSemaphoreCounterValue == nullptr) {
        table->GetSemaphoreCounterValue = (PFN_vkGetSemaphoreCounterValue)StubGetSemaphoreCounterValue;
    }
    table->WaitSemaphores = (PFN_vkWaitSemaphores)gpa(device, "vkWaitSemaphores");
    if (table->WaitSemaphores == nullptr) {
        table->WaitSemaphores = (PFN_vkWaitSemaphores)StubWaitSemaphores;
    }
    table->SignalSemaphore = (PFN_vkSignalSemaphore)gpa(device, "vkSignalSemaphore");
    if (table->SignalSemaphore == nullptr) {
        table->SignalSemaphore = (PFN_vkSignalSemaphore)StubSignalSemaphore;
    }
    table->GetBufferDeviceAddress = (PFN_vkGetBufferDeviceAddress)gpa(device, "vkGetBufferDeviceAddress");
    if (table->GetBufferDeviceAddress == nullptr) {
        table->GetBufferDeviceAddress = (PFN_vkGetBufferDeviceAddress)StubGetBufferDeviceAddress;
    }
    table->GetBufferOpaqueCaptureAddress = (PFN_vkGetBufferOpaqueCaptureAddress)gpa(device, "vkGetBufferOpaqueCaptureAddress");
    if (table->GetBufferOpaqueCaptureAddress == nullptr) {
        table->GetBufferOpaqueCaptureAddress = (PFN_vkGetBufferOpaqueCaptureAddress)StubGetBufferOpaqueCaptureAddress;
    }
    table->GetDeviceMemoryOpaqueCaptureAddress =
        (PFN_vkGetDeviceMemoryOpaqueCaptureAddress)gpa(device, "vkGetDeviceMemoryOpaqueCaptureAddress");
    if (table->GetDeviceMemoryOpaqueCaptureAddress == nullptr) {
        table->GetDeviceMemoryOpaqueCaptureAddress =
            (PFN_vkGetDeviceMemoryOpaqueCaptureAddress)StubGetDeviceMemoryOpaqueCaptureAddress;
    }
    table->CreatePrivateDataSlot = (PFN_vkCreatePrivateDataSlot)gpa(device, "vkCreatePrivateDataSlot");
    if (table->CreatePrivateDataSlot == nullptr) {
        table->CreatePrivateDataSlot = (PFN_vkCreatePrivateDataSlot)StubCreatePrivateDataSlot;
    }
    table->DestroyPrivateDataSlot = (PFN_vkDestroyPrivateDataSlot)gpa(device, "vkDestroyPrivateDataSlot");
    if (table->DestroyPrivateDataSlot == nullptr) {
        table->DestroyPrivateDataSlot = (PFN_vkDestroyPrivateDataSlot)StubDestroyPrivateDataSlot;
    }
    table->SetPrivateData = (PFN_vkSetPrivateData)gpa(device, "vkSetPrivateData");
    if (table->SetPrivateData == nullptr) {
        table->SetPrivateData = (PFN_vkSetPrivateData)StubSetPrivateData;
    }
    table->GetPrivateData = (PFN_vkGetPrivateData)gpa(device, "vkGetPrivateData");
    if (table->GetPrivateData == nullptr) {
        table->GetPrivateData = (PFN_vkGetPrivateData)StubGetPrivateData;
    }
    table->CmdSetEvent2 = (PFN_vkCmdSetEvent2)gpa(device, "vkCmdSetEvent2");
    if (table->CmdSetEvent2 == nullptr) {
        table->CmdSetEvent2 = (PFN_vkCmdSetEvent2)StubCmdSetEvent2;
    }
    table->CmdResetEvent2 = (PFN_vkCmdResetEvent2)gpa(device, "vkCmdResetEvent2");
    if (table->CmdResetEvent2 == nullptr) {
        table->CmdResetEvent2 = (PFN_vkCmdResetEvent2)StubCmdResetEvent2;
    }
    table->CmdWaitEvents2 = (PFN_vkCmdWaitEvents2)gpa(device, "vkCmdWaitEvents2");
    if (table->CmdWaitEvents2 == nullptr) {
        table->CmdWaitEvents2 = (PFN_vkCmdWaitEvents2)StubCmdWaitEvents2;
    }
    table->CmdPipelineBarrier2 = (PFN_vkCmdPipelineBarrier2)gpa(device, "vkCmdPipelineBarrier2");
    if (table->CmdPipelineBarrier2 == nullptr) {
        table->CmdPipelineBarrier2 = (PFN_vkCmdPipelineBarrier2)StubCmdPipelineBarrier2;
    }
    table->CmdWriteTimestamp2 = (PFN_vkCmdWriteTimestamp2)gpa(device, "vkCmdWriteTimestamp2");
    if (table->CmdWriteTimestamp2 == nullptr) {
        table->CmdWriteTimestamp2 = (PFN_vkCmdWriteTimestamp2)StubCmdWriteTimestamp2;
    }
    table->QueueSubmit2 = (PFN_vkQueueSubmit2)gpa(device, "vkQueueSubmit2");
    if (table->QueueSubmit2 == nullptr) {
        table->QueueSubmit2 = (PFN_vkQueueSubmit2)StubQueueSubmit2;
    }
    table->CmdCopyBuffer2 = (PFN_vkCmdCopyBuffer2)gpa(device, "vkCmdCopyBuffer2");
    if (table->CmdCopyBuffer2 == nullptr) {
        table->CmdCopyBuffer2 = (PFN_vkCmdCopyBuffer2)StubCmdCopyBuffer2;
    }
    table->CmdCopyImage2 = (PFN_vkCmdCopyImage2)gpa(device, "vkCmdCopyImage2");
    if (table->CmdCopyImage2 == nullptr) {
        table->CmdCopyImage2 = (PFN_vkCmdCopyImage2)StubCmdCopyImage2;
    }
    table->CmdCopyBufferToImage2 = (PFN_vkCmdCopyBufferToImage2)gpa(device, "vkCmdCopyBufferToImage2");
    if (table->CmdCopyBufferToImage2 == nullptr) {
        table->CmdCopyBufferToImage2 = (PFN_vkCmdCopyBufferToImage2)StubCmdCopyBufferToImage2;
    }
    table->CmdCopyImageToBuffer2 = (PFN_vkCmdCopyImageToBuffer2)gpa(device, "vkCmdCopyImageToBuffer2");
    if (table->CmdCopyImageToBuffer2 == nullptr) {
        table->CmdCopyImageToBuffer2 = (PFN_vkCmdCopyImageToBuffer2)StubCmdCopyImageToBuffer2;
    }
    table->CmdBlitImage2 = (PFN_vkCmdBlitImage2)gpa(device, "vkCmdBlitImage2");
    if (table->CmdBlitImage2 == nullptr) {
        table->CmdBlitImage2 = (PFN_vkCmdBlitImage2)StubCmdBlitImage2;
    }
    table->CmdResolveImage2 = (PFN_vkCmdResolveImage2)gpa(device, "vkCmdResolveImage2");
    if (table->CmdResolveImage2 == nullptr) {
        table->CmdResolveImage2 = (PFN_vkCmdResolveImage2)StubCmdResolveImage2;
    }
    table->CmdBeginRendering = (PFN_vkCmdBeginRendering)gpa(device, "vkCmdBeginRendering");
    if (table->CmdBeginRendering == nullptr) {
        table->CmdBeginRendering = (PFN_vkCmdBeginRendering)StubCmdBeginRendering;
    }
    table->CmdEndRendering = (PFN_vkCmdEndRendering)gpa(device, "vkCmdEndRendering");
    if (table->CmdEndRendering == nullptr) {
        table->CmdEndRendering = (PFN_vkCmdEndRendering)StubCmdEndRendering;
    }
    table->CmdSetCullMode = (PFN_vkCmdSetCullMode)gpa(device, "vkCmdSetCullMode");
    if (table->CmdSetCullMode == nullptr) {
        table->CmdSetCullMode = (PFN_vkCmdSetCullMode)StubCmdSetCullMode;
    }
    table->CmdSetFrontFace = (PFN_vkCmdSetFrontFace)gpa(device, "vkCmdSetFrontFace");
    if (table->CmdSetFrontFace == nullptr) {
        table->CmdSetFrontFace = (PFN_vkCmdSetFrontFace)StubCmdSetFrontFace;
    }
    table->CmdSetPrimitiveTopology = (PFN_vkCmdSetPrimitiveTopology)gpa(device, "vkCmdSetPrimitiveTopology");
    if (table->CmdSetPrimitiveTopology == nullptr) {
        table->CmdSetPrimitiveTopology = (PFN_vkCmdSetPrimitiveTopology)StubCmdSetPrimitiveTopology;
    }
    table->CmdSetViewportWithCount = (PFN_vkCmdSetViewportWithCount)gpa(device, "vkCmdSetViewportWithCount");
    if (table->CmdSetViewportWithCount == nullptr) {
        table->CmdSetViewportWithCount = (PFN_vkCmdSetViewportWithCount)StubCmdSetViewportWithCount;
    }
    table->CmdSetScissorWithCount = (PFN_vkCmdSetScissorWithCount)gpa(device, "vkCmdSetScissorWithCount");
    if (table->CmdSetScissorWithCount == nullptr) {
        table->CmdSetScissorWithCount = (PFN_vkCmdSetScissorWithCount)StubCmdSetScissorWithCount;
    }
    table->CmdBindVertexBuffers2 = (PFN_vkCmdBindVertexBuffers2)gpa(device, "vkCmdBindVertexBuffers2");
    if (table->CmdBindVertexBuffers2 == nullptr) {
        table->CmdBindVertexBuffers2 = (PFN_vkCmdBindVertexBuffers2)StubCmdBindVertexBuffers2;
    }
    table->CmdSetDepthTestEnable = (PFN_vkCmdSetDepthTestEnable)gpa(device, "vkCmdSetDepthTestEnable");
    if (table->CmdSetDepthTestEnable == nullptr) {
        table->CmdSetDepthTestEnable = (PFN_vkCmdSetDepthTestEnable)StubCmdSetDepthTestEnable;
    }
    table->CmdSetDepthWriteEnable = (PFN_vkCmdSetDepthWriteEnable)gpa(device, "vkCmdSetDepthWriteEnable");
    if (table->CmdSetDepthWriteEnable == nullptr) {
        table->CmdSetDepthWriteEnable = (PFN_vkCmdSetDepthWriteEnable)StubCmdSetDepthWriteEnable;
    }
    table->CmdSetDepthCompareOp = (PFN_vkCmdSetDepthCompareOp)gpa(device, "vkCmdSetDepthCompareOp");
    if (table->CmdSetDepthCompareOp == nullptr) {
        table->CmdSetDepthCompareOp = (PFN_vkCmdSetDepthCompareOp)StubCmdSetDepthCompareOp;
    }
    table->CmdSetDepthBoundsTestEnable = (PFN_vkCmdSetDepthBoundsTestEnable)gpa(device, "vkCmdSetDepthBoundsTestEnable");
    if (table->CmdSetDepthBoundsTestEnable == nullptr) {
        table->CmdSetDepthBoundsTestEnable = (PFN_vkCmdSetDepthBoundsTestEnable)StubCmdSetDepthBoundsTestEnable;
    }
    table->CmdSetStencilTestEnable = (PFN_vkCmdSetStencilTestEnable)gpa(device, "vkCmdSetStencilTestEnable");
    if (table->CmdSetStencilTestEnable == nullptr) {
        table->CmdSetStencilTestEnable = (PFN_vkCmdSetStencilTestEnable)StubCmdSetStencilTestEnable;
    }
    table->CmdSetStencilOp = (PFN_vkCmdSetStencilOp)gpa(device, "vkCmdSetStencilOp");
    if (table->CmdSetStencilOp == nullptr) {
        table->CmdSetStencilOp = (PFN_vkCmdSetStencilOp)StubCmdSetStencilOp;
    }
    table->CmdSetRasterizerDiscardEnable = (PFN_vkCmdSetRasterizerDiscardEnable)gpa(device, "vkCmdSetRasterizerDiscardEnable");
    if (table->CmdSetRasterizerDiscardEnable == nullptr) {
        table->CmdSetRasterizerDiscardEnable = (PFN_vkCmdSetRasterizerDiscardEnable)StubCmdSetRasterizerDiscardEnable;
    }
    table->CmdSetDepthBiasEnable = (PFN_vkCmdSetDepthBiasEnable)gpa(device, "vkCmdSetDepthBiasEnable");
    if (table->CmdSetDepthBiasEnable == nullptr) {
        table->CmdSetDepthBiasEnable = (PFN_vkCmdSetDepthBiasEnable)StubCmdSetDepthBiasEnable;
    }
    table->CmdSetPrimitiveRestartEnable = (PFN_vkCmdSetPrimitiveRestartEnable)gpa(device, "vkCmdSetPrimitiveRestartEnable");
    if (table->CmdSetPrimitiveRestartEnable == nullptr) {
        table->CmdSetPrimitiveRestartEnable = (PFN_vkCmdSetPrimitiveRestartEnable)StubCmdSetPrimitiveRestartEnable;
    }
    table->GetDeviceBufferMemoryRequirements =
        (PFN_vkGetDeviceBufferMemoryRequirements)gpa(device, "vkGetDeviceBufferMemoryRequirements");
    if (table->GetDeviceBufferMemoryRequirements == nullptr) {
        table->GetDeviceBufferMemoryRequirements = (PFN_vkGetDeviceBufferMemoryRequirements)StubGetDeviceBufferMemoryRequirements;
    }
    table->GetDeviceImageMemoryRequirements =
        (PFN_vkGetDeviceImageMemoryRequirements)gpa(device, "vkGetDeviceImageMemoryRequirements");
    if (table->GetDeviceImageMemoryRequirements == nullptr) {
        table->GetDeviceImageMemoryRequirements = (PFN_vkGetDeviceImageMemoryRequirements)StubGetDeviceImageMemoryRequirements;
    }
    table->GetDeviceImageSparseMemoryRequirements =
        (PFN_vkGetDeviceImageSparseMemoryRequirements)gpa(device, "vkGetDeviceImageSparseMemoryRequirements");
    if (table->GetDeviceImageSparseMemoryRequirements == nullptr) {
        table->GetDeviceImageSparseMemoryRequirements =
            (PFN_vkGetDeviceImageSparseMemoryRequirements)StubGetDeviceImageSparseMemoryRequirements;
    }
    table->CreateSwapchainKHR = (PFN_vkCreateSwapchainKHR)gpa(device, "vkCreateSwapchainKHR");
    if (table->CreateSwapchainKHR == nullptr) {
        table->CreateSwapchainKHR = (PFN_vkCreateSwapchainKHR)StubCreateSwapchainKHR;
    }
    table->DestroySwapchainKHR = (PFN_vkDestroySwapchainKHR)gpa(device, "vkDestroySwapchainKHR");
    if (table->DestroySwapchainKHR == nullptr) {
        table->DestroySwapchainKHR = (PFN_vkDestroySwapchainKHR)StubDestroySwapchainKHR;
    }
    table->GetSwapchainImagesKHR = (PFN_vkGetSwapchainImagesKHR)gpa(device, "vkGetSwapchainImagesKHR");
    if (table->GetSwapchainImagesKHR == nullptr) {
        table->GetSwapchainImagesKHR = (PFN_vkGetSwapchainImagesKHR)StubGetSwapchainImagesKHR;
    }
    table->AcquireNextImageKHR = (PFN_vkAcquireNextImageKHR)gpa(device, "vkAcquireNextImageKHR");
    if (table->AcquireNextImageKHR == nullptr) {
        table->AcquireNextImageKHR = (PFN_vkAcquireNextImageKHR)StubAcquireNextImageKHR;
    }
    table->QueuePresentKHR = (PFN_vkQueuePresentKHR)gpa(device, "vkQueuePresentKHR");
    if (table->QueuePresentKHR == nullptr) {
        table->QueuePresentKHR = (PFN_vkQueuePresentKHR)StubQueuePresentKHR;
    }
    table->GetDeviceGroupPresentCapabilitiesKHR =
        (PFN_vkGetDeviceGroupPresentCapabilitiesKHR)gpa(device, "vkGetDeviceGroupPresentCapabilitiesKHR");
    if (table->GetDeviceGroupPresentCapabilitiesKHR == nullptr) {
        table->GetDeviceGroupPresentCapabilitiesKHR =
            (PFN_vkGetDeviceGroupPresentCapabilitiesKHR)StubGetDeviceGroupPresentCapabilitiesKHR;
    }
    table->GetDeviceGroupSurfacePresentModesKHR =
        (PFN_vkGetDeviceGroupSurfacePresentModesKHR)gpa(device, "vkGetDeviceGroupSurfacePresentModesKHR");
    if (table->GetDeviceGroupSurfacePresentModesKHR == nullptr) {
        table->GetDeviceGroupSurfacePresentModesKHR =
            (PFN_vkGetDeviceGroupSurfacePresentModesKHR)StubGetDeviceGroupSurfacePresentModesKHR;
    }
    table->AcquireNextImage2KHR = (PFN_vkAcquireNextImage2KHR)gpa(device, "vkAcquireNextImage2KHR");
    if (table->AcquireNextImage2KHR == nullptr) {
        table->AcquireNextImage2KHR = (PFN_vkAcquireNextImage2KHR)StubAcquireNextImage2KHR;
    }
    table->CreateSharedSwapchainsKHR = (PFN_vkCreateSharedSwapchainsKHR)gpa(device, "vkCreateSharedSwapchainsKHR");
    if (table->CreateSharedSwapchainsKHR == nullptr) {
        table->CreateSharedSwapchainsKHR = (PFN_vkCreateSharedSwapchainsKHR)StubCreateSharedSwapchainsKHR;
    }
    table->CreateVideoSessionKHR = (PFN_vkCreateVideoSessionKHR)gpa(device, "vkCreateVideoSessionKHR");
    if (table->CreateVideoSessionKHR == nullptr) {
        table->CreateVideoSessionKHR = (PFN_vkCreateVideoSessionKHR)StubCreateVideoSessionKHR;
    }
    table->DestroyVideoSessionKHR = (PFN_vkDestroyVideoSessionKHR)gpa(device, "vkDestroyVideoSessionKHR");
    if (table->DestroyVideoSessionKHR == nullptr) {
        table->DestroyVideoSessionKHR = (PFN_vkDestroyVideoSessionKHR)StubDestroyVideoSessionKHR;
    }
    table->GetVideoSessionMemoryRequirementsKHR =
        (PFN_vkGetVideoSessionMemoryRequirementsKHR)gpa(device, "vkGetVideoSessionMemoryRequirementsKHR");
    if (table->GetVideoSessionMemoryRequirementsKHR == nullptr) {
        table->GetVideoSessionMemoryRequirementsKHR =
            (PFN_vkGetVideoSessionMemoryRequirementsKHR)StubGetVideoSessionMemoryRequirementsKHR;
    }
    table->BindVideoSessionMemoryKHR = (PFN_vkBindVideoSessionMemoryKHR)gpa(device, "vkBindVideoSessionMemoryKHR");
    if (table->BindVideoSessionMemoryKHR == nullptr) {
        table->BindVideoSessionMemoryKHR = (PFN_vkBindVideoSessionMemoryKHR)StubBindVideoSessionMemoryKHR;
    }
    table->CreateVideoSessionParametersKHR =
        (PFN_vkCreateVideoSessionParametersKHR)gpa(device, "vkCreateVideoSessionParametersKHR");
    if (table->CreateVideoSessionParametersKHR == nullptr) {
        table->CreateVideoSessionParametersKHR = (PFN_vkCreateVideoSessionParametersKHR)StubCreateVideoSessionParametersKHR;
    }
    table->UpdateVideoSessionParametersKHR =
        (PFN_vkUpdateVideoSessionParametersKHR)gpa(device, "vkUpdateVideoSessionParametersKHR");
    if (table->UpdateVideoSessionParametersKHR == nullptr) {
        table->UpdateVideoSessionParametersKHR = (PFN_vkUpdateVideoSessionParametersKHR)StubUpdateVideoSessionParametersKHR;
    }
    table->DestroyVideoSessionParametersKHR =
        (PFN_vkDestroyVideoSessionParametersKHR)gpa(device, "vkDestroyVideoSessionParametersKHR");
    if (table->DestroyVideoSessionParametersKHR == nullptr) {
        table->DestroyVideoSessionParametersKHR = (PFN_vkDestroyVideoSessionParametersKHR)StubDestroyVideoSessionParametersKHR;
    }
    table->CmdBeginVideoCodingKHR = (PFN_vkCmdBeginVideoCodingKHR)gpa(device, "vkCmdBeginVideoCodingKHR");
    if (table->CmdBeginVideoCodingKHR == nullptr) {
        table->CmdBeginVideoCodingKHR = (PFN_vkCmdBeginVideoCodingKHR)StubCmdBeginVideoCodingKHR;
    }
    table->CmdEndVideoCodingKHR = (PFN_vkCmdEndVideoCodingKHR)gpa(device, "vkCmdEndVideoCodingKHR");
    if (table->CmdEndVideoCodingKHR == nullptr) {
        table->CmdEndVideoCodingKHR = (PFN_vkCmdEndVideoCodingKHR)StubCmdEndVideoCodingKHR;
    }
    table->CmdControlVideoCodingKHR = (PFN_vkCmdControlVideoCodingKHR)gpa(device, "vkCmdControlVideoCodingKHR");
    if (table->CmdControlVideoCodingKHR == nullptr) {
        table->CmdControlVideoCodingKHR = (PFN_vkCmdControlVideoCodingKHR)StubCmdControlVideoCodingKHR;
    }
    table->CmdDecodeVideoKHR = (PFN_vkCmdDecodeVideoKHR)gpa(device, "vkCmdDecodeVideoKHR");
    if (table->CmdDecodeVideoKHR == nullptr) {
        table->CmdDecodeVideoKHR = (PFN_vkCmdDecodeVideoKHR)StubCmdDecodeVideoKHR;
    }
    table->CmdBeginRenderingKHR = (PFN_vkCmdBeginRenderingKHR)gpa(device, "vkCmdBeginRenderingKHR");
    if (table->CmdBeginRenderingKHR == nullptr) {
        table->CmdBeginRenderingKHR = (PFN_vkCmdBeginRenderingKHR)StubCmdBeginRenderingKHR;
    }
    table->CmdEndRenderingKHR = (PFN_vkCmdEndRenderingKHR)gpa(device, "vkCmdEndRenderingKHR");
    if (table->CmdEndRenderingKHR == nullptr) {
        table->CmdEndRenderingKHR = (PFN_vkCmdEndRenderingKHR)StubCmdEndRenderingKHR;
    }
    table->GetDeviceGroupPeerMemoryFeaturesKHR =
        (PFN_vkGetDeviceGroupPeerMemoryFeaturesKHR)gpa(device, "vkGetDeviceGroupPeerMemoryFeaturesKHR");
    if (table->GetDeviceGroupPeerMemoryFeaturesKHR == nullptr) {
        table->GetDeviceGroupPeerMemoryFeaturesKHR =
            (PFN_vkGetDeviceGroupPeerMemoryFeaturesKHR)StubGetDeviceGroupPeerMemoryFeaturesKHR;
    }
    table->CmdSetDeviceMaskKHR = (PFN_vkCmdSetDeviceMaskKHR)gpa(device, "vkCmdSetDeviceMaskKHR");
    if (table->CmdSetDeviceMaskKHR == nullptr) {
        table->CmdSetDeviceMaskKHR = (PFN_vkCmdSetDeviceMaskKHR)StubCmdSetDeviceMaskKHR;
    }
    table->CmdDispatchBaseKHR = (PFN_vkCmdDispatchBaseKHR)gpa(device, "vkCmdDispatchBaseKHR");
    if (table->CmdDispatchBaseKHR == nullptr) {
        table->CmdDispatchBaseKHR = (PFN_vkCmdDispatchBaseKHR)StubCmdDispatchBaseKHR;
    }
    table->TrimCommandPoolKHR = (PFN_vkTrimCommandPoolKHR)gpa(device, "vkTrimCommandPoolKHR");
    if (table->TrimCommandPoolKHR == nullptr) {
        table->TrimCommandPoolKHR = (PFN_vkTrimCommandPoolKHR)StubTrimCommandPoolKHR;
    }
#ifdef VK_USE_PLATFORM_WIN32_KHR
    table->GetMemoryWin32HandleKHR = (PFN_vkGetMemoryWin32HandleKHR)gpa(device, "vkGetMemoryWin32HandleKHR");
    if (table->GetMemoryWin32HandleKHR == nullptr) {
        table->GetMemoryWin32HandleKHR = (PFN_vkGetMemoryWin32HandleKHR)StubGetMemoryWin32HandleKHR;
    }
#endif  // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
    table->GetMemoryWin32HandlePropertiesKHR =
        (PFN_vkGetMemoryWin32HandlePropertiesKHR)gpa(device, "vkGetMemoryWin32HandlePropertiesKHR");
    if (table->GetMemoryWin32HandlePropertiesKHR == nullptr) {
        table->GetMemoryWin32HandlePropertiesKHR = (PFN_vkGetMemoryWin32HandlePropertiesKHR)StubGetMemoryWin32HandlePropertiesKHR;
    }
#endif  // VK_USE_PLATFORM_WIN32_KHR
    table->GetMemoryFdKHR = (PFN_vkGetMemoryFdKHR)gpa(device, "vkGetMemoryFdKHR");
    if (table->GetMemoryFdKHR == nullptr) {
        table->GetMemoryFdKHR = (PFN_vkGetMemoryFdKHR)StubGetMemoryFdKHR;
    }
    table->GetMemoryFdPropertiesKHR = (PFN_vkGetMemoryFdPropertiesKHR)gpa(device, "vkGetMemoryFdPropertiesKHR");
    if (table->GetMemoryFdPropertiesKHR == nullptr) {
        table->GetMemoryFdPropertiesKHR = (PFN_vkGetMemoryFdPropertiesKHR)StubGetMemoryFdPropertiesKHR;
    }
#ifdef VK_USE_PLATFORM_WIN32_KHR
    table->ImportSemaphoreWin32HandleKHR = (PFN_vkImportSemaphoreWin32HandleKHR)gpa(device, "vkImportSemaphoreWin32HandleKHR");
    if (table->ImportSemaphoreWin32HandleKHR == nullptr) {
        table->ImportSemaphoreWin32HandleKHR = (PFN_vkImportSemaphoreWin32HandleKHR)StubImportSemaphoreWin32HandleKHR;
    }
#endif  // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
    table->GetSemaphoreWin32HandleKHR = (PFN_vkGetSemaphoreWin32HandleKHR)gpa(device, "vkGetSemaphoreWin32HandleKHR");
    if (table->GetSemaphoreWin32HandleKHR == nullptr) {
        table->GetSemaphoreWin32HandleKHR = (PFN_vkGetSemaphoreWin32HandleKHR)StubGetSemaphoreWin32HandleKHR;
    }
#endif  // VK_USE_PLATFORM_WIN32_KHR
    table->ImportSemaphoreFdKHR = (PFN_vkImportSemaphoreFdKHR)gpa(device, "vkImportSemaphoreFdKHR");
    if (table->ImportSemaphoreFdKHR == nullptr) {
        table->ImportSemaphoreFdKHR = (PFN_vkImportSemaphoreFdKHR)StubImportSemaphoreFdKHR;
    }
    table->GetSemaphoreFdKHR = (PFN_vkGetSemaphoreFdKHR)gpa(device, "vkGetSemaphoreFdKHR");
    if (table->GetSemaphoreFdKHR == nullptr) {
        table->GetSemaphoreFdKHR = (PFN_vkGetSemaphoreFdKHR)StubGetSemaphoreFdKHR;
    }
    table->CmdPushDescriptorSetKHR = (PFN_vkCmdPushDescriptorSetKHR)gpa(device, "vkCmdPushDescriptorSetKHR");
    if (table->CmdPushDescriptorSetKHR == nullptr) {
        table->CmdPushDescriptorSetKHR = (PFN_vkCmdPushDescriptorSetKHR)StubCmdPushDescriptorSetKHR;
    }
    table->CmdPushDescriptorSetWithTemplateKHR =
        (PFN_vkCmdPushDescriptorSetWithTemplateKHR)gpa(device, "vkCmdPushDescriptorSetWithTemplateKHR");
    if (table->CmdPushDescriptorSetWithTemplateKHR == nullptr) {
        table->CmdPushDescriptorSetWithTemplateKHR =
            (PFN_vkCmdPushDescriptorSetWithTemplateKHR)StubCmdPushDescriptorSetWithTemplateKHR;
    }
    table->CreateDescriptorUpdateTemplateKHR =
        (PFN_vkCreateDescriptorUpdateTemplateKHR)gpa(device, "vkCreateDescriptorUpdateTemplateKHR");
    if (table->CreateDescriptorUpdateTemplateKHR == nullptr) {
        table->CreateDescriptorUpdateTemplateKHR = (PFN_vkCreateDescriptorUpdateTemplateKHR)StubCreateDescriptorUpdateTemplateKHR;
    }
    table->DestroyDescriptorUpdateTemplateKHR =
        (PFN_vkDestroyDescriptorUpdateTemplateKHR)gpa(device, "vkDestroyDescriptorUpdateTemplateKHR");
    if (table->DestroyDescriptorUpdateTemplateKHR == nullptr) {
        table->DestroyDescriptorUpdateTemplateKHR =
            (PFN_vkDestroyDescriptorUpdateTemplateKHR)StubDestroyDescriptorUpdateTemplateKHR;
    }
    table->UpdateDescriptorSetWithTemplateKHR =
        (PFN_vkUpdateDescriptorSetWithTemplateKHR)gpa(device, "vkUpdateDescriptorSetWithTemplateKHR");
    if (table->UpdateDescriptorSetWithTemplateKHR == nullptr) {
        table->UpdateDescriptorSetWithTemplateKHR =
            (PFN_vkUpdateDescriptorSetWithTemplateKHR)StubUpdateDescriptorSetWithTemplateKHR;
    }
    table->CreateRenderPass2KHR = (PFN_vkCreateRenderPass2KHR)gpa(device, "vkCreateRenderPass2KHR");
    if (table->CreateRenderPass2KHR == nullptr) {
        table->CreateRenderPass2KHR = (PFN_vkCreateRenderPass2KHR)StubCreateRenderPass2KHR;
    }
    table->CmdBeginRenderPass2KHR = (PFN_vkCmdBeginRenderPass2KHR)gpa(device, "vkCmdBeginRenderPass2KHR");
    if (table->CmdBeginRenderPass2KHR == nullptr) {
        table->CmdBeginRenderPass2KHR = (PFN_vkCmdBeginRenderPass2KHR)StubCmdBeginRenderPass2KHR;
    }
    table->CmdNextSubpass2KHR = (PFN_vkCmdNextSubpass2KHR)gpa(device, "vkCmdNextSubpass2KHR");
    if (table->CmdNextSubpass2KHR == nullptr) {
        table->CmdNextSubpass2KHR = (PFN_vkCmdNextSubpass2KHR)StubCmdNextSubpass2KHR;
    }
    table->CmdEndRenderPass2KHR = (PFN_vkCmdEndRenderPass2KHR)gpa(device, "vkCmdEndRenderPass2KHR");
    if (table->CmdEndRenderPass2KHR == nullptr) {
        table->CmdEndRenderPass2KHR = (PFN_vkCmdEndRenderPass2KHR)StubCmdEndRenderPass2KHR;
    }
    table->GetSwapchainStatusKHR = (PFN_vkGetSwapchainStatusKHR)gpa(device, "vkGetSwapchainStatusKHR");
    if (table->GetSwapchainStatusKHR == nullptr) {
        table->GetSwapchainStatusKHR = (PFN_vkGetSwapchainStatusKHR)StubGetSwapchainStatusKHR;
    }
#ifdef VK_USE_PLATFORM_WIN32_KHR
    table->ImportFenceWin32HandleKHR = (PFN_vkImportFenceWin32HandleKHR)gpa(device, "vkImportFenceWin32HandleKHR");
    if (table->ImportFenceWin32HandleKHR == nullptr) {
        table->ImportFenceWin32HandleKHR = (PFN_vkImportFenceWin32HandleKHR)StubImportFenceWin32HandleKHR;
    }
#endif  // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
    table->GetFenceWin32HandleKHR = (PFN_vkGetFenceWin32HandleKHR)gpa(device, "vkGetFenceWin32HandleKHR");
    if (table->GetFenceWin32HandleKHR == nullptr) {
        table->GetFenceWin32HandleKHR = (PFN_vkGetFenceWin32HandleKHR)StubGetFenceWin32HandleKHR;
    }
#endif  // VK_USE_PLATFORM_WIN32_KHR
    table->ImportFenceFdKHR = (PFN_vkImportFenceFdKHR)gpa(device, "vkImportFenceFdKHR");
    if (table->ImportFenceFdKHR == nullptr) {
        table->ImportFenceFdKHR = (PFN_vkImportFenceFdKHR)StubImportFenceFdKHR;
    }
    table->GetFenceFdKHR = (PFN_vkGetFenceFdKHR)gpa(device, "vkGetFenceFdKHR");
    if (table->GetFenceFdKHR == nullptr) {
        table->GetFenceFdKHR = (PFN_vkGetFenceFdKHR)StubGetFenceFdKHR;
    }
    table->AcquireProfilingLockKHR = (PFN_vkAcquireProfilingLockKHR)gpa(device, "vkAcquireProfilingLockKHR");
    if (table->AcquireProfilingLockKHR == nullptr) {
        table->AcquireProfilingLockKHR = (PFN_vkAcquireProfilingLockKHR)StubAcquireProfilingLockKHR;
    }
    table->ReleaseProfilingLockKHR = (PFN_vkReleaseProfilingLockKHR)gpa(device, "vkReleaseProfilingLockKHR");
    if (table->ReleaseProfilingLockKHR == nullptr) {
        table->ReleaseProfilingLockKHR = (PFN_vkReleaseProfilingLockKHR)StubReleaseProfilingLockKHR;
    }
    table->GetImageMemoryRequirements2KHR = (PFN_vkGetImageMemoryRequirements2KHR)gpa(device, "vkGetImageMemoryRequirements2KHR");
    if (table->GetImageMemoryRequirements2KHR == nullptr) {
        table->GetImageMemoryRequirements2KHR = (PFN_vkGetImageMemoryRequirements2KHR)StubGetImageMemoryRequirements2KHR;
    }
    table->GetBufferMemoryRequirements2KHR =
        (PFN_vkGetBufferMemoryRequirements2KHR)gpa(device, "vkGetBufferMemoryRequirements2KHR");
    if (table->GetBufferMemoryRequirements2KHR == nullptr) {
        table->GetBufferMemoryRequirements2KHR = (PFN_vkGetBufferMemoryRequirements2KHR)StubGetBufferMemoryRequirements2KHR;
    }
    table->GetImageSparseMemoryRequirements2KHR =
        (PFN_vkGetImageSparseMemoryRequirements2KHR)gpa(device, "vkGetImageSparseMemoryRequirements2KHR");
    if (table->GetImageSparseMemoryRequirements2KHR == nullptr) {
        table->GetImageSparseMemoryRequirements2KHR =
            (PFN_vkGetImageSparseMemoryRequirements2KHR)StubGetImageSparseMemoryRequirements2KHR;
    }
    table->CreateSamplerYcbcrConversionKHR =
        (PFN_vkCreateSamplerYcbcrConversionKHR)gpa(device, "vkCreateSamplerYcbcrConversionKHR");
    if (table->CreateSamplerYcbcrConversionKHR == nullptr) {
        table->CreateSamplerYcbcrConversionKHR = (PFN_vkCreateSamplerYcbcrConversionKHR)StubCreateSamplerYcbcrConversionKHR;
    }
    table->DestroySamplerYcbcrConversionKHR =
        (PFN_vkDestroySamplerYcbcrConversionKHR)gpa(device, "vkDestroySamplerYcbcrConversionKHR");
    if (table->DestroySamplerYcbcrConversionKHR == nullptr) {
        table->DestroySamplerYcbcrConversionKHR = (PFN_vkDestroySamplerYcbcrConversionKHR)StubDestroySamplerYcbcrConversionKHR;
    }
    table->BindBufferMemory2KHR = (PFN_vkBindBufferMemory2KHR)gpa(device, "vkBindBufferMemory2KHR");
    if (table->BindBufferMemory2KHR == nullptr) {
        table->BindBufferMemory2KHR = (PFN_vkBindBufferMemory2KHR)StubBindBufferMemory2KHR;
    }
    table->BindImageMemory2KHR = (PFN_vkBindImageMemory2KHR)gpa(device, "vkBindImageMemory2KHR");
    if (table->BindImageMemory2KHR == nullptr) {
        table->BindImageMemory2KHR = (PFN_vkBindImageMemory2KHR)StubBindImageMemory2KHR;
    }
    table->GetDescriptorSetLayoutSupportKHR =
        (PFN_vkGetDescriptorSetLayoutSupportKHR)gpa(device, "vkGetDescriptorSetLayoutSupportKHR");
    if (table->GetDescriptorSetLayoutSupportKHR == nullptr) {
        table->GetDescriptorSetLayoutSupportKHR = (PFN_vkGetDescriptorSetLayoutSupportKHR)StubGetDescriptorSetLayoutSupportKHR;
    }
    table->CmdDrawIndirectCountKHR = (PFN_vkCmdDrawIndirectCountKHR)gpa(device, "vkCmdDrawIndirectCountKHR");
    if (table->CmdDrawIndirectCountKHR == nullptr) {
        table->CmdDrawIndirectCountKHR = (PFN_vkCmdDrawIndirectCountKHR)StubCmdDrawIndirectCountKHR;
    }
    table->CmdDrawIndexedIndirectCountKHR = (PFN_vkCmdDrawIndexedIndirectCountKHR)gpa(device, "vkCmdDrawIndexedIndirectCountKHR");
    if (table->CmdDrawIndexedIndirectCountKHR == nullptr) {
        table->CmdDrawIndexedIndirectCountKHR = (PFN_vkCmdDrawIndexedIndirectCountKHR)StubCmdDrawIndexedIndirectCountKHR;
    }
    table->GetSemaphoreCounterValueKHR = (PFN_vkGetSemaphoreCounterValueKHR)gpa(device, "vkGetSemaphoreCounterValueKHR");
    if (table->GetSemaphoreCounterValueKHR == nullptr) {
        table->GetSemaphoreCounterValueKHR = (PFN_vkGetSemaphoreCounterValueKHR)StubGetSemaphoreCounterValueKHR;
    }
    table->WaitSemaphoresKHR = (PFN_vkWaitSemaphoresKHR)gpa(device, "vkWaitSemaphoresKHR");
    if (table->WaitSemaphoresKHR == nullptr) {
        table->WaitSemaphoresKHR = (PFN_vkWaitSemaphoresKHR)StubWaitSemaphoresKHR;
    }
    table->SignalSemaphoreKHR = (PFN_vkSignalSemaphoreKHR)gpa(device, "vkSignalSemaphoreKHR");
    if (table->SignalSemaphoreKHR == nullptr) {
        table->SignalSemaphoreKHR = (PFN_vkSignalSemaphoreKHR)StubSignalSemaphoreKHR;
    }
    table->CmdSetFragmentShadingRateKHR = (PFN_vkCmdSetFragmentShadingRateKHR)gpa(device, "vkCmdSetFragmentShadingRateKHR");
    if (table->CmdSetFragmentShadingRateKHR == nullptr) {
        table->CmdSetFragmentShadingRateKHR = (PFN_vkCmdSetFragmentShadingRateKHR)StubCmdSetFragmentShadingRateKHR;
    }
    table->WaitForPresentKHR = (PFN_vkWaitForPresentKHR)gpa(device, "vkWaitForPresentKHR");
    if (table->WaitForPresentKHR == nullptr) {
        table->WaitForPresentKHR = (PFN_vkWaitForPresentKHR)StubWaitForPresentKHR;
    }
    table->GetBufferDeviceAddressKHR = (PFN_vkGetBufferDeviceAddressKHR)gpa(device, "vkGetBufferDeviceAddressKHR");
    if (table->GetBufferDeviceAddressKHR == nullptr) {
        table->GetBufferDeviceAddressKHR = (PFN_vkGetBufferDeviceAddressKHR)StubGetBufferDeviceAddressKHR;
    }
    table->GetBufferOpaqueCaptureAddressKHR =
        (PFN_vkGetBufferOpaqueCaptureAddressKHR)gpa(device, "vkGetBufferOpaqueCaptureAddressKHR");
    if (table->GetBufferOpaqueCaptureAddressKHR == nullptr) {
        table->GetBufferOpaqueCaptureAddressKHR = (PFN_vkGetBufferOpaqueCaptureAddressKHR)StubGetBufferOpaqueCaptureAddressKHR;
    }
    table->GetDeviceMemoryOpaqueCaptureAddressKHR =
        (PFN_vkGetDeviceMemoryOpaqueCaptureAddressKHR)gpa(device, "vkGetDeviceMemoryOpaqueCaptureAddressKHR");
    if (table->GetDeviceMemoryOpaqueCaptureAddressKHR == nullptr) {
        table->GetDeviceMemoryOpaqueCaptureAddressKHR =
            (PFN_vkGetDeviceMemoryOpaqueCaptureAddressKHR)StubGetDeviceMemoryOpaqueCaptureAddressKHR;
    }
    table->CreateDeferredOperationKHR = (PFN_vkCreateDeferredOperationKHR)gpa(device, "vkCreateDeferredOperationKHR");
    if (table->CreateDeferredOperationKHR == nullptr) {
        table->CreateDeferredOperationKHR = (PFN_vkCreateDeferredOperationKHR)StubCreateDeferredOperationKHR;
    }
    table->DestroyDeferredOperationKHR = (PFN_vkDestroyDeferredOperationKHR)gpa(device, "vkDestroyDeferredOperationKHR");
    if (table->DestroyDeferredOperationKHR == nullptr) {
        table->DestroyDeferredOperationKHR = (PFN_vkDestroyDeferredOperationKHR)StubDestroyDeferredOperationKHR;
    }
    table->GetDeferredOperationMaxConcurrencyKHR =
        (PFN_vkGetDeferredOperationMaxConcurrencyKHR)gpa(device, "vkGetDeferredOperationMaxConcurrencyKHR");
    if (table->GetDeferredOperationMaxConcurrencyKHR == nullptr) {
        table->GetDeferredOperationMaxConcurrencyKHR =
            (PFN_vkGetDeferredOperationMaxConcurrencyKHR)StubGetDeferredOperationMaxConcurrencyKHR;
    }
    table->GetDeferredOperationResultKHR = (PFN_vkGetDeferredOperationResultKHR)gpa(device, "vkGetDeferredOperationResultKHR");
    if (table->GetDeferredOperationResultKHR == nullptr) {
        table->GetDeferredOperationResultKHR = (PFN_vkGetDeferredOperationResultKHR)StubGetDeferredOperationResultKHR;
    }
    table->DeferredOperationJoinKHR = (PFN_vkDeferredOperationJoinKHR)gpa(device, "vkDeferredOperationJoinKHR");
    if (table->DeferredOperationJoinKHR == nullptr) {
        table->DeferredOperationJoinKHR = (PFN_vkDeferredOperationJoinKHR)StubDeferredOperationJoinKHR;
    }
    table->GetPipelineExecutablePropertiesKHR =
        (PFN_vkGetPipelineExecutablePropertiesKHR)gpa(device, "vkGetPipelineExecutablePropertiesKHR");
    if (table->GetPipelineExecutablePropertiesKHR == nullptr) {
        table->GetPipelineExecutablePropertiesKHR =
            (PFN_vkGetPipelineExecutablePropertiesKHR)StubGetPipelineExecutablePropertiesKHR;
    }
    table->GetPipelineExecutableStatisticsKHR =
        (PFN_vkGetPipelineExecutableStatisticsKHR)gpa(device, "vkGetPipelineExecutableStatisticsKHR");
    if (table->GetPipelineExecutableStatisticsKHR == nullptr) {
        table->GetPipelineExecutableStatisticsKHR =
            (PFN_vkGetPipelineExecutableStatisticsKHR)StubGetPipelineExecutableStatisticsKHR;
    }
    table->GetPipelineExecutableInternalRepresentationsKHR =
        (PFN_vkGetPipelineExecutableInternalRepresentationsKHR)gpa(device, "vkGetPipelineExecutableInternalRepresentationsKHR");
    if (table->GetPipelineExecutableInternalRepresentationsKHR == nullptr) {
        table->GetPipelineExecutableInternalRepresentationsKHR =
            (PFN_vkGetPipelineExecutableInternalRepresentationsKHR)StubGetPipelineExecutableInternalRepresentationsKHR;
    }
    table->MapMemory2KHR = (PFN_vkMapMemory2KHR)gpa(device, "vkMapMemory2KHR");
    if (table->MapMemory2KHR == nullptr) {
        table->MapMemory2KHR = (PFN_vkMapMemory2KHR)StubMapMemory2KHR;
    }
    table->UnmapMemory2KHR = (PFN_vkUnmapMemory2KHR)gpa(device, "vkUnmapMemory2KHR");
    if (table->UnmapMemory2KHR == nullptr) {
        table->UnmapMemory2KHR = (PFN_vkUnmapMemory2KHR)StubUnmapMemory2KHR;
    }
#ifdef VK_ENABLE_BETA_EXTENSIONS
    table->GetEncodedVideoSessionParametersKHR =
        (PFN_vkGetEncodedVideoSessionParametersKHR)gpa(device, "vkGetEncodedVideoSessionParametersKHR");
    if (table->GetEncodedVideoSessionParametersKHR == nullptr) {
        table->GetEncodedVideoSessionParametersKHR =
            (PFN_vkGetEncodedVideoSessionParametersKHR)StubGetEncodedVideoSessionParametersKHR;
    }
#endif  // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
    table->CmdEncodeVideoKHR = (PFN_vkCmdEncodeVideoKHR)gpa(device, "vkCmdEncodeVideoKHR");
    if (table->CmdEncodeVideoKHR == nullptr) {
        table->CmdEncodeVideoKHR = (PFN_vkCmdEncodeVideoKHR)StubCmdEncodeVideoKHR;
    }
#endif  // VK_ENABLE_BETA_EXTENSIONS
    table->CmdSetEvent2KHR = (PFN_vkCmdSetEvent2KHR)gpa(device, "vkCmdSetEvent2KHR");
    if (table->CmdSetEvent2KHR == nullptr) {
        table->CmdSetEvent2KHR = (PFN_vkCmdSetEvent2KHR)StubCmdSetEvent2KHR;
    }
    table->CmdResetEvent2KHR = (PFN_vkCmdResetEvent2KHR)gpa(device, "vkCmdResetEvent2KHR");
    if (table->CmdResetEvent2KHR == nullptr) {
        table->CmdResetEvent2KHR = (PFN_vkCmdResetEvent2KHR)StubCmdResetEvent2KHR;
    }
    table->CmdWaitEvents2KHR = (PFN_vkCmdWaitEvents2KHR)gpa(device, "vkCmdWaitEvents2KHR");
    if (table->CmdWaitEvents2KHR == nullptr) {
        table->CmdWaitEvents2KHR = (PFN_vkCmdWaitEvents2KHR)StubCmdWaitEvents2KHR;
    }
    table->CmdPipelineBarrier2KHR = (PFN_vkCmdPipelineBarrier2KHR)gpa(device, "vkCmdPipelineBarrier2KHR");
    if (table->CmdPipelineBarrier2KHR == nullptr) {
        table->CmdPipelineBarrier2KHR = (PFN_vkCmdPipelineBarrier2KHR)StubCmdPipelineBarrier2KHR;
    }
    table->CmdWriteTimestamp2KHR = (PFN_vkCmdWriteTimestamp2KHR)gpa(device, "vkCmdWriteTimestamp2KHR");
    if (table->CmdWriteTimestamp2KHR == nullptr) {
        table->CmdWriteTimestamp2KHR = (PFN_vkCmdWriteTimestamp2KHR)StubCmdWriteTimestamp2KHR;
    }
    table->QueueSubmit2KHR = (PFN_vkQueueSubmit2KHR)gpa(device, "vkQueueSubmit2KHR");
    if (table->QueueSubmit2KHR == nullptr) {
        table->QueueSubmit2KHR = (PFN_vkQueueSubmit2KHR)StubQueueSubmit2KHR;
    }
    table->CmdWriteBufferMarker2AMD = (PFN_vkCmdWriteBufferMarker2AMD)gpa(device, "vkCmdWriteBufferMarker2AMD");
    if (table->CmdWriteBufferMarker2AMD == nullptr) {
        table->CmdWriteBufferMarker2AMD = (PFN_vkCmdWriteBufferMarker2AMD)StubCmdWriteBufferMarker2AMD;
    }
    table->GetQueueCheckpointData2NV = (PFN_vkGetQueueCheckpointData2NV)gpa(device, "vkGetQueueCheckpointData2NV");
    if (table->GetQueueCheckpointData2NV == nullptr) {
        table->GetQueueCheckpointData2NV = (PFN_vkGetQueueCheckpointData2NV)StubGetQueueCheckpointData2NV;
    }
    table->CmdCopyBuffer2KHR = (PFN_vkCmdCopyBuffer2KHR)gpa(device, "vkCmdCopyBuffer2KHR");
    if (table->CmdCopyBuffer2KHR == nullptr) {
        table->CmdCopyBuffer2KHR = (PFN_vkCmdCopyBuffer2KHR)StubCmdCopyBuffer2KHR;
    }
    table->CmdCopyImage2KHR = (PFN_vkCmdCopyImage2KHR)gpa(device, "vkCmdCopyImage2KHR");
    if (table->CmdCopyImage2KHR == nullptr) {
        table->CmdCopyImage2KHR = (PFN_vkCmdCopyImage2KHR)StubCmdCopyImage2KHR;
    }
    table->CmdCopyBufferToImage2KHR = (PFN_vkCmdCopyBufferToImage2KHR)gpa(device, "vkCmdCopyBufferToImage2KHR");
    if (table->CmdCopyBufferToImage2KHR == nullptr) {
        table->CmdCopyBufferToImage2KHR = (PFN_vkCmdCopyBufferToImage2KHR)StubCmdCopyBufferToImage2KHR;
    }
    table->CmdCopyImageToBuffer2KHR = (PFN_vkCmdCopyImageToBuffer2KHR)gpa(device, "vkCmdCopyImageToBuffer2KHR");
    if (table->CmdCopyImageToBuffer2KHR == nullptr) {
        table->CmdCopyImageToBuffer2KHR = (PFN_vkCmdCopyImageToBuffer2KHR)StubCmdCopyImageToBuffer2KHR;
    }
    table->CmdBlitImage2KHR = (PFN_vkCmdBlitImage2KHR)gpa(device, "vkCmdBlitImage2KHR");
    if (table->CmdBlitImage2KHR == nullptr) {
        table->CmdBlitImage2KHR = (PFN_vkCmdBlitImage2KHR)StubCmdBlitImage2KHR;
    }
    table->CmdResolveImage2KHR = (PFN_vkCmdResolveImage2KHR)gpa(device, "vkCmdResolveImage2KHR");
    if (table->CmdResolveImage2KHR == nullptr) {
        table->CmdResolveImage2KHR = (PFN_vkCmdResolveImage2KHR)StubCmdResolveImage2KHR;
    }
    table->CmdTraceRaysIndirect2KHR = (PFN_vkCmdTraceRaysIndirect2KHR)gpa(device, "vkCmdTraceRaysIndirect2KHR");
    if (table->CmdTraceRaysIndirect2KHR == nullptr) {
        table->CmdTraceRaysIndirect2KHR = (PFN_vkCmdTraceRaysIndirect2KHR)StubCmdTraceRaysIndirect2KHR;
    }
    table->GetDeviceBufferMemoryRequirementsKHR =
        (PFN_vkGetDeviceBufferMemoryRequirementsKHR)gpa(device, "vkGetDeviceBufferMemoryRequirementsKHR");
    if (table->GetDeviceBufferMemoryRequirementsKHR == nullptr) {
        table->GetDeviceBufferMemoryRequirementsKHR =
            (PFN_vkGetDeviceBufferMemoryRequirementsKHR)StubGetDeviceBufferMemoryRequirementsKHR;
    }
    table->GetDeviceImageMemoryRequirementsKHR =
        (PFN_vkGetDeviceImageMemoryRequirementsKHR)gpa(device, "vkGetDeviceImageMemoryRequirementsKHR");
    if (table->GetDeviceImageMemoryRequirementsKHR == nullptr) {
        table->GetDeviceImageMemoryRequirementsKHR =
            (PFN_vkGetDeviceImageMemoryRequirementsKHR)StubGetDeviceImageMemoryRequirementsKHR;
    }
    table->GetDeviceImageSparseMemoryRequirementsKHR =
        (PFN_vkGetDeviceImageSparseMemoryRequirementsKHR)gpa(device, "vkGetDeviceImageSparseMemoryRequirementsKHR");
    if (table->GetDeviceImageSparseMemoryRequirementsKHR == nullptr) {
        table->GetDeviceImageSparseMemoryRequirementsKHR =
            (PFN_vkGetDeviceImageSparseMemoryRequirementsKHR)StubGetDeviceImageSparseMemoryRequirementsKHR;
    }
    table->CmdBindIndexBuffer2KHR = (PFN_vkCmdBindIndexBuffer2KHR)gpa(device, "vkCmdBindIndexBuffer2KHR");
    if (table->CmdBindIndexBuffer2KHR == nullptr) {
        table->CmdBindIndexBuffer2KHR = (PFN_vkCmdBindIndexBuffer2KHR)StubCmdBindIndexBuffer2KHR;
    }
    table->GetRenderingAreaGranularityKHR = (PFN_vkGetRenderingAreaGranularityKHR)gpa(device, "vkGetRenderingAreaGranularityKHR");
    if (table->GetRenderingAreaGranularityKHR == nullptr) {
        table->GetRenderingAreaGranularityKHR = (PFN_vkGetRenderingAreaGranularityKHR)StubGetRenderingAreaGranularityKHR;
    }
    table->GetDeviceImageSubresourceLayoutKHR =
        (PFN_vkGetDeviceImageSubresourceLayoutKHR)gpa(device, "vkGetDeviceImageSubresourceLayoutKHR");
    if (table->GetDeviceImageSubresourceLayoutKHR == nullptr) {
        table->GetDeviceImageSubresourceLayoutKHR =
            (PFN_vkGetDeviceImageSubresourceLayoutKHR)StubGetDeviceImageSubresourceLayoutKHR;
    }
    table->GetImageSubresourceLayout2KHR = (PFN_vkGetImageSubresourceLayout2KHR)gpa(device, "vkGetImageSubresourceLayout2KHR");
    if (table->GetImageSubresourceLayout2KHR == nullptr) {
        table->GetImageSubresourceLayout2KHR = (PFN_vkGetImageSubresourceLayout2KHR)StubGetImageSubresourceLayout2KHR;
    }
    table->DebugMarkerSetObjectTagEXT = (PFN_vkDebugMarkerSetObjectTagEXT)gpa(device, "vkDebugMarkerSetObjectTagEXT");
    if (table->DebugMarkerSetObjectTagEXT == nullptr) {
        table->DebugMarkerSetObjectTagEXT = (PFN_vkDebugMarkerSetObjectTagEXT)StubDebugMarkerSetObjectTagEXT;
    }
    table->DebugMarkerSetObjectNameEXT = (PFN_vkDebugMarkerSetObjectNameEXT)gpa(device, "vkDebugMarkerSetObjectNameEXT");
    if (table->DebugMarkerSetObjectNameEXT == nullptr) {
        table->DebugMarkerSetObjectNameEXT = (PFN_vkDebugMarkerSetObjectNameEXT)StubDebugMarkerSetObjectNameEXT;
    }
    table->CmdDebugMarkerBeginEXT = (PFN_vkCmdDebugMarkerBeginEXT)gpa(device, "vkCmdDebugMarkerBeginEXT");
    if (table->CmdDebugMarkerBeginEXT == nullptr) {
        table->CmdDebugMarkerBeginEXT = (PFN_vkCmdDebugMarkerBeginEXT)StubCmdDebugMarkerBeginEXT;
    }
    table->CmdDebugMarkerEndEXT = (PFN_vkCmdDebugMarkerEndEXT)gpa(device, "vkCmdDebugMarkerEndEXT");
    if (table->CmdDebugMarkerEndEXT == nullptr) {
        table->CmdDebugMarkerEndEXT = (PFN_vkCmdDebugMarkerEndEXT)StubCmdDebugMarkerEndEXT;
    }
    table->CmdDebugMarkerInsertEXT = (PFN_vkCmdDebugMarkerInsertEXT)gpa(device, "vkCmdDebugMarkerInsertEXT");
    if (table->CmdDebugMarkerInsertEXT == nullptr) {
        table->CmdDebugMarkerInsertEXT = (PFN_vkCmdDebugMarkerInsertEXT)StubCmdDebugMarkerInsertEXT;
    }
    table->CmdBindTransformFeedbackBuffersEXT =
        (PFN_vkCmdBindTransformFeedbackBuffersEXT)gpa(device, "vkCmdBindTransformFeedbackBuffersEXT");
    if (table->CmdBindTransformFeedbackBuffersEXT == nullptr) {
        table->CmdBindTransformFeedbackBuffersEXT =
            (PFN_vkCmdBindTransformFeedbackBuffersEXT)StubCmdBindTransformFeedbackBuffersEXT;
    }
    table->CmdBeginTransformFeedbackEXT = (PFN_vkCmdBeginTransformFeedbackEXT)gpa(device, "vkCmdBeginTransformFeedbackEXT");
    if (table->CmdBeginTransformFeedbackEXT == nullptr) {
        table->CmdBeginTransformFeedbackEXT = (PFN_vkCmdBeginTransformFeedbackEXT)StubCmdBeginTransformFeedbackEXT;
    }
    table->CmdEndTransformFeedbackEXT = (PFN_vkCmdEndTransformFeedbackEXT)gpa(device, "vkCmdEndTransformFeedbackEXT");
    if (table->CmdEndTransformFeedbackEXT == nullptr) {
        table->CmdEndTransformFeedbackEXT = (PFN_vkCmdEndTransformFeedbackEXT)StubCmdEndTransformFeedbackEXT;
    }
    table->CmdBeginQueryIndexedEXT = (PFN_vkCmdBeginQueryIndexedEXT)gpa(device, "vkCmdBeginQueryIndexedEXT");
    if (table->CmdBeginQueryIndexedEXT == nullptr) {
        table->CmdBeginQueryIndexedEXT = (PFN_vkCmdBeginQueryIndexedEXT)StubCmdBeginQueryIndexedEXT;
    }
    table->CmdEndQueryIndexedEXT = (PFN_vkCmdEndQueryIndexedEXT)gpa(device, "vkCmdEndQueryIndexedEXT");
    if (table->CmdEndQueryIndexedEXT == nullptr) {
        table->CmdEndQueryIndexedEXT = (PFN_vkCmdEndQueryIndexedEXT)StubCmdEndQueryIndexedEXT;
    }
    table->CmdDrawIndirectByteCountEXT = (PFN_vkCmdDrawIndirectByteCountEXT)gpa(device, "vkCmdDrawIndirectByteCountEXT");
    if (table->CmdDrawIndirectByteCountEXT == nullptr) {
        table->CmdDrawIndirectByteCountEXT = (PFN_vkCmdDrawIndirectByteCountEXT)StubCmdDrawIndirectByteCountEXT;
    }
    table->CreateCuModuleNVX = (PFN_vkCreateCuModuleNVX)gpa(device, "vkCreateCuModuleNVX");
    if (table->CreateCuModuleNVX == nullptr) {
        table->CreateCuModuleNVX = (PFN_vkCreateCuModuleNVX)StubCreateCuModuleNVX;
    }
    table->CreateCuFunctionNVX = (PFN_vkCreateCuFunctionNVX)gpa(device, "vkCreateCuFunctionNVX");
    if (table->CreateCuFunctionNVX == nullptr) {
        table->CreateCuFunctionNVX = (PFN_vkCreateCuFunctionNVX)StubCreateCuFunctionNVX;
    }
    table->DestroyCuModuleNVX = (PFN_vkDestroyCuModuleNVX)gpa(device, "vkDestroyCuModuleNVX");
    if (table->DestroyCuModuleNVX == nullptr) {
        table->DestroyCuModuleNVX = (PFN_vkDestroyCuModuleNVX)StubDestroyCuModuleNVX;
    }
    table->DestroyCuFunctionNVX = (PFN_vkDestroyCuFunctionNVX)gpa(device, "vkDestroyCuFunctionNVX");
    if (table->DestroyCuFunctionNVX == nullptr) {
        table->DestroyCuFunctionNVX = (PFN_vkDestroyCuFunctionNVX)StubDestroyCuFunctionNVX;
    }
    table->CmdCuLaunchKernelNVX = (PFN_vkCmdCuLaunchKernelNVX)gpa(device, "vkCmdCuLaunchKernelNVX");
    if (table->CmdCuLaunchKernelNVX == nullptr) {
        table->CmdCuLaunchKernelNVX = (PFN_vkCmdCuLaunchKernelNVX)StubCmdCuLaunchKernelNVX;
    }
    table->GetImageViewHandleNVX = (PFN_vkGetImageViewHandleNVX)gpa(device, "vkGetImageViewHandleNVX");
    if (table->GetImageViewHandleNVX == nullptr) {
        table->GetImageViewHandleNVX = (PFN_vkGetImageViewHandleNVX)StubGetImageViewHandleNVX;
    }
    table->GetImageViewAddressNVX = (PFN_vkGetImageViewAddressNVX)gpa(device, "vkGetImageViewAddressNVX");
    if (table->GetImageViewAddressNVX == nullptr) {
        table->GetImageViewAddressNVX = (PFN_vkGetImageViewAddressNVX)StubGetImageViewAddressNVX;
    }
    table->CmdDrawIndirectCountAMD = (PFN_vkCmdDrawIndirectCountAMD)gpa(device, "vkCmdDrawIndirectCountAMD");
    if (table->CmdDrawIndirectCountAMD == nullptr) {
        table->CmdDrawIndirectCountAMD = (PFN_vkCmdDrawIndirectCountAMD)StubCmdDrawIndirectCountAMD;
    }
    table->CmdDrawIndexedIndirectCountAMD = (PFN_vkCmdDrawIndexedIndirectCountAMD)gpa(device, "vkCmdDrawIndexedIndirectCountAMD");
    if (table->CmdDrawIndexedIndirectCountAMD == nullptr) {
        table->CmdDrawIndexedIndirectCountAMD = (PFN_vkCmdDrawIndexedIndirectCountAMD)StubCmdDrawIndexedIndirectCountAMD;
    }
    table->GetShaderInfoAMD = (PFN_vkGetShaderInfoAMD)gpa(device, "vkGetShaderInfoAMD");
    if (table->GetShaderInfoAMD == nullptr) {
        table->GetShaderInfoAMD = (PFN_vkGetShaderInfoAMD)StubGetShaderInfoAMD;
    }
#ifdef VK_USE_PLATFORM_WIN32_KHR
    table->GetMemoryWin32HandleNV = (PFN_vkGetMemoryWin32HandleNV)gpa(device, "vkGetMemoryWin32HandleNV");
    if (table->GetMemoryWin32HandleNV == nullptr) {
        table->GetMemoryWin32HandleNV = (PFN_vkGetMemoryWin32HandleNV)StubGetMemoryWin32HandleNV;
    }
#endif  // VK_USE_PLATFORM_WIN32_KHR
    table->CmdBeginConditionalRenderingEXT =
        (PFN_vkCmdBeginConditionalRenderingEXT)gpa(device, "vkCmdBeginConditionalRenderingEXT");
    if (table->CmdBeginConditionalRenderingEXT == nullptr) {
        table->CmdBeginConditionalRenderingEXT = (PFN_vkCmdBeginConditionalRenderingEXT)StubCmdBeginConditionalRenderingEXT;
    }
    table->CmdEndConditionalRenderingEXT = (PFN_vkCmdEndConditionalRenderingEXT)gpa(device, "vkCmdEndConditionalRenderingEXT");
    if (table->CmdEndConditionalRenderingEXT == nullptr) {
        table->CmdEndConditionalRenderingEXT = (PFN_vkCmdEndConditionalRenderingEXT)StubCmdEndConditionalRenderingEXT;
    }
    table->CmdSetViewportWScalingNV = (PFN_vkCmdSetViewportWScalingNV)gpa(device, "vkCmdSetViewportWScalingNV");
    if (table->CmdSetViewportWScalingNV == nullptr) {
        table->CmdSetViewportWScalingNV = (PFN_vkCmdSetViewportWScalingNV)StubCmdSetViewportWScalingNV;
    }
    table->DisplayPowerControlEXT = (PFN_vkDisplayPowerControlEXT)gpa(device, "vkDisplayPowerControlEXT");
    if (table->DisplayPowerControlEXT == nullptr) {
        table->DisplayPowerControlEXT = (PFN_vkDisplayPowerControlEXT)StubDisplayPowerControlEXT;
    }
    table->RegisterDeviceEventEXT = (PFN_vkRegisterDeviceEventEXT)gpa(device, "vkRegisterDeviceEventEXT");
    if (table->RegisterDeviceEventEXT == nullptr) {
        table->RegisterDeviceEventEXT = (PFN_vkRegisterDeviceEventEXT)StubRegisterDeviceEventEXT;
    }
    table->RegisterDisplayEventEXT = (PFN_vkRegisterDisplayEventEXT)gpa(device, "vkRegisterDisplayEventEXT");
    if (table->RegisterDisplayEventEXT == nullptr) {
        table->RegisterDisplayEventEXT = (PFN_vkRegisterDisplayEventEXT)StubRegisterDisplayEventEXT;
    }
    table->GetSwapchainCounterEXT = (PFN_vkGetSwapchainCounterEXT)gpa(device, "vkGetSwapchainCounterEXT");
    if (table->GetSwapchainCounterEXT == nullptr) {
        table->GetSwapchainCounterEXT = (PFN_vkGetSwapchainCounterEXT)StubGetSwapchainCounterEXT;
    }
    table->GetRefreshCycleDurationGOOGLE = (PFN_vkGetRefreshCycleDurationGOOGLE)gpa(device, "vkGetRefreshCycleDurationGOOGLE");
    if (table->GetRefreshCycleDurationGOOGLE == nullptr) {
        table->GetRefreshCycleDurationGOOGLE = (PFN_vkGetRefreshCycleDurationGOOGLE)StubGetRefreshCycleDurationGOOGLE;
    }
    table->GetPastPresentationTimingGOOGLE =
        (PFN_vkGetPastPresentationTimingGOOGLE)gpa(device, "vkGetPastPresentationTimingGOOGLE");
    if (table->GetPastPresentationTimingGOOGLE == nullptr) {
        table->GetPastPresentationTimingGOOGLE = (PFN_vkGetPastPresentationTimingGOOGLE)StubGetPastPresentationTimingGOOGLE;
    }
    table->CmdSetDiscardRectangleEXT = (PFN_vkCmdSetDiscardRectangleEXT)gpa(device, "vkCmdSetDiscardRectangleEXT");
    if (table->CmdSetDiscardRectangleEXT == nullptr) {
        table->CmdSetDiscardRectangleEXT = (PFN_vkCmdSetDiscardRectangleEXT)StubCmdSetDiscardRectangleEXT;
    }
    table->CmdSetDiscardRectangleEnableEXT =
        (PFN_vkCmdSetDiscardRectangleEnableEXT)gpa(device, "vkCmdSetDiscardRectangleEnableEXT");
    if (table->CmdSetDiscardRectangleEnableEXT == nullptr) {
        table->CmdSetDiscardRectangleEnableEXT = (PFN_vkCmdSetDiscardRectangleEnableEXT)StubCmdSetDiscardRectangleEnableEXT;
    }
    table->CmdSetDiscardRectangleModeEXT = (PFN_vkCmdSetDiscardRectangleModeEXT)gpa(device, "vkCmdSetDiscardRectangleModeEXT");
    if (table->CmdSetDiscardRectangleModeEXT == nullptr) {
        table->CmdSetDiscardRectangleModeEXT = (PFN_vkCmdSetDiscardRectangleModeEXT)StubCmdSetDiscardRectangleModeEXT;
    }
    table->SetHdrMetadataEXT = (PFN_vkSetHdrMetadataEXT)gpa(device, "vkSetHdrMetadataEXT");
    if (table->SetHdrMetadataEXT == nullptr) {
        table->SetHdrMetadataEXT = (PFN_vkSetHdrMetadataEXT)StubSetHdrMetadataEXT;
    }
    table->SetDebugUtilsObjectNameEXT = (PFN_vkSetDebugUtilsObjectNameEXT)gpa(device, "vkSetDebugUtilsObjectNameEXT");
    if (table->SetDebugUtilsObjectNameEXT == nullptr) {
        table->SetDebugUtilsObjectNameEXT = (PFN_vkSetDebugUtilsObjectNameEXT)StubSetDebugUtilsObjectNameEXT;
    }
    table->SetDebugUtilsObjectTagEXT = (PFN_vkSetDebugUtilsObjectTagEXT)gpa(device, "vkSetDebugUtilsObjectTagEXT");
    if (table->SetDebugUtilsObjectTagEXT == nullptr) {
        table->SetDebugUtilsObjectTagEXT = (PFN_vkSetDebugUtilsObjectTagEXT)StubSetDebugUtilsObjectTagEXT;
    }
    table->QueueBeginDebugUtilsLabelEXT = (PFN_vkQueueBeginDebugUtilsLabelEXT)gpa(device, "vkQueueBeginDebugUtilsLabelEXT");
    if (table->QueueBeginDebugUtilsLabelEXT == nullptr) {
        table->QueueBeginDebugUtilsLabelEXT = (PFN_vkQueueBeginDebugUtilsLabelEXT)StubQueueBeginDebugUtilsLabelEXT;
    }
    table->QueueEndDebugUtilsLabelEXT = (PFN_vkQueueEndDebugUtilsLabelEXT)gpa(device, "vkQueueEndDebugUtilsLabelEXT");
    if (table->QueueEndDebugUtilsLabelEXT == nullptr) {
        table->QueueEndDebugUtilsLabelEXT = (PFN_vkQueueEndDebugUtilsLabelEXT)StubQueueEndDebugUtilsLabelEXT;
    }
    table->QueueInsertDebugUtilsLabelEXT = (PFN_vkQueueInsertDebugUtilsLabelEXT)gpa(device, "vkQueueInsertDebugUtilsLabelEXT");
    if (table->QueueInsertDebugUtilsLabelEXT == nullptr) {
        table->QueueInsertDebugUtilsLabelEXT = (PFN_vkQueueInsertDebugUtilsLabelEXT)StubQueueInsertDebugUtilsLabelEXT;
    }
    table->CmdBeginDebugUtilsLabelEXT = (PFN_vkCmdBeginDebugUtilsLabelEXT)gpa(device, "vkCmdBeginDebugUtilsLabelEXT");
    if (table->CmdBeginDebugUtilsLabelEXT == nullptr) {
        table->CmdBeginDebugUtilsLabelEXT = (PFN_vkCmdBeginDebugUtilsLabelEXT)StubCmdBeginDebugUtilsLabelEXT;
    }
    table->CmdEndDebugUtilsLabelEXT = (PFN_vkCmdEndDebugUtilsLabelEXT)gpa(device, "vkCmdEndDebugUtilsLabelEXT");
    if (table->CmdEndDebugUtilsLabelEXT == nullptr) {
        table->CmdEndDebugUtilsLabelEXT = (PFN_vkCmdEndDebugUtilsLabelEXT)StubCmdEndDebugUtilsLabelEXT;
    }
    table->CmdInsertDebugUtilsLabelEXT = (PFN_vkCmdInsertDebugUtilsLabelEXT)gpa(device, "vkCmdInsertDebugUtilsLabelEXT");
    if (table->CmdInsertDebugUtilsLabelEXT == nullptr) {
        table->CmdInsertDebugUtilsLabelEXT = (PFN_vkCmdInsertDebugUtilsLabelEXT)StubCmdInsertDebugUtilsLabelEXT;
    }
#ifdef VK_USE_PLATFORM_ANDROID_KHR
    table->GetAndroidHardwareBufferPropertiesANDROID =
        (PFN_vkGetAndroidHardwareBufferPropertiesANDROID)gpa(device, "vkGetAndroidHardwareBufferPropertiesANDROID");
    if (table->GetAndroidHardwareBufferPropertiesANDROID == nullptr) {
        table->GetAndroidHardwareBufferPropertiesANDROID =
            (PFN_vkGetAndroidHardwareBufferPropertiesANDROID)StubGetAndroidHardwareBufferPropertiesANDROID;
    }
#endif  // VK_USE_PLATFORM_ANDROID_KHR
#ifdef VK_USE_PLATFORM_ANDROID_KHR
    table->GetMemoryAndroidHardwareBufferANDROID =
        (PFN_vkGetMemoryAndroidHardwareBufferANDROID)gpa(device, "vkGetMemoryAndroidHardwareBufferANDROID");
    if (table->GetMemoryAndroidHardwareBufferANDROID == nullptr) {
        table->GetMemoryAndroidHardwareBufferANDROID =
            (PFN_vkGetMemoryAndroidHardwareBufferANDROID)StubGetMemoryAndroidHardwareBufferANDROID;
    }
#endif  // VK_USE_PLATFORM_ANDROID_KHR
#ifdef VK_ENABLE_BETA_EXTENSIONS
    table->CreateExecutionGraphPipelinesAMDX =
        (PFN_vkCreateExecutionGraphPipelinesAMDX)gpa(device, "vkCreateExecutionGraphPipelinesAMDX");
    if (table->CreateExecutionGraphPipelinesAMDX == nullptr) {
        table->CreateExecutionGraphPipelinesAMDX = (PFN_vkCreateExecutionGraphPipelinesAMDX)StubCreateExecutionGraphPipelinesAMDX;
    }
#endif  // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
    table->GetExecutionGraphPipelineScratchSizeAMDX =
        (PFN_vkGetExecutionGraphPipelineScratchSizeAMDX)gpa(device, "vkGetExecutionGraphPipelineScratchSizeAMDX");
    if (table->GetExecutionGraphPipelineScratchSizeAMDX == nullptr) {
        table->GetExecutionGraphPipelineScratchSizeAMDX =
            (PFN_vkGetExecutionGraphPipelineScratchSizeAMDX)StubGetExecutionGraphPipelineScratchSizeAMDX;
    }
#endif  // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
    table->GetExecutionGraphPipelineNodeIndexAMDX =
        (PFN_vkGetExecutionGraphPipelineNodeIndexAMDX)gpa(device, "vkGetExecutionGraphPipelineNodeIndexAMDX");
    if (table->GetExecutionGraphPipelineNodeIndexAMDX == nullptr) {
        table->GetExecutionGraphPipelineNodeIndexAMDX =
            (PFN_vkGetExecutionGraphPipelineNodeIndexAMDX)StubGetExecutionGraphPipelineNodeIndexAMDX;
    }
#endif  // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
    table->CmdInitializeGraphScratchMemoryAMDX =
        (PFN_vkCmdInitializeGraphScratchMemoryAMDX)gpa(device, "vkCmdInitializeGraphScratchMemoryAMDX");
    if (table->CmdInitializeGraphScratchMemoryAMDX == nullptr) {
        table->CmdInitializeGraphScratchMemoryAMDX =
            (PFN_vkCmdInitializeGraphScratchMemoryAMDX)StubCmdInitializeGraphScratchMemoryAMDX;
    }
#endif  // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
    table->CmdDispatchGraphAMDX = (PFN_vkCmdDispatchGraphAMDX)gpa(device, "vkCmdDispatchGraphAMDX");
    if (table->CmdDispatchGraphAMDX == nullptr) {
        table->CmdDispatchGraphAMDX = (PFN_vkCmdDispatchGraphAMDX)StubCmdDispatchGraphAMDX;
    }
#endif  // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
    table->CmdDispatchGraphIndirectAMDX = (PFN_vkCmdDispatchGraphIndirectAMDX)gpa(device, "vkCmdDispatchGraphIndirectAMDX");
    if (table->CmdDispatchGraphIndirectAMDX == nullptr) {
        table->CmdDispatchGraphIndirectAMDX = (PFN_vkCmdDispatchGraphIndirectAMDX)StubCmdDispatchGraphIndirectAMDX;
    }
#endif  // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
    table->CmdDispatchGraphIndirectCountAMDX =
        (PFN_vkCmdDispatchGraphIndirectCountAMDX)gpa(device, "vkCmdDispatchGraphIndirectCountAMDX");
    if (table->CmdDispatchGraphIndirectCountAMDX == nullptr) {
        table->CmdDispatchGraphIndirectCountAMDX = (PFN_vkCmdDispatchGraphIndirectCountAMDX)StubCmdDispatchGraphIndirectCountAMDX;
    }
#endif  // VK_ENABLE_BETA_EXTENSIONS
    table->CmdSetSampleLocationsEXT = (PFN_vkCmdSetSampleLocationsEXT)gpa(device, "vkCmdSetSampleLocationsEXT");
    if (table->CmdSetSampleLocationsEXT == nullptr) {
        table->CmdSetSampleLocationsEXT = (PFN_vkCmdSetSampleLocationsEXT)StubCmdSetSampleLocationsEXT;
    }
    table->GetImageDrmFormatModifierPropertiesEXT =
        (PFN_vkGetImageDrmFormatModifierPropertiesEXT)gpa(device, "vkGetImageDrmFormatModifierPropertiesEXT");
    if (table->GetImageDrmFormatModifierPropertiesEXT == nullptr) {
        table->GetImageDrmFormatModifierPropertiesEXT =
            (PFN_vkGetImageDrmFormatModifierPropertiesEXT)StubGetImageDrmFormatModifierPropertiesEXT;
    }
    table->CreateValidationCacheEXT = (PFN_vkCreateValidationCacheEXT)gpa(device, "vkCreateValidationCacheEXT");
    if (table->CreateValidationCacheEXT == nullptr) {
        table->CreateValidationCacheEXT = (PFN_vkCreateValidationCacheEXT)StubCreateValidationCacheEXT;
    }
    table->DestroyValidationCacheEXT = (PFN_vkDestroyValidationCacheEXT)gpa(device, "vkDestroyValidationCacheEXT");
    if (table->DestroyValidationCacheEXT == nullptr) {
        table->DestroyValidationCacheEXT = (PFN_vkDestroyValidationCacheEXT)StubDestroyValidationCacheEXT;
    }
    table->MergeValidationCachesEXT = (PFN_vkMergeValidationCachesEXT)gpa(device, "vkMergeValidationCachesEXT");
    if (table->MergeValidationCachesEXT == nullptr) {
        table->MergeValidationCachesEXT = (PFN_vkMergeValidationCachesEXT)StubMergeValidationCachesEXT;
    }
    table->GetValidationCacheDataEXT = (PFN_vkGetValidationCacheDataEXT)gpa(device, "vkGetValidationCacheDataEXT");
    if (table->GetValidationCacheDataEXT == nullptr) {
        table->GetValidationCacheDataEXT = (PFN_vkGetValidationCacheDataEXT)StubGetValidationCacheDataEXT;
    }
    table->CmdBindShadingRateImageNV = (PFN_vkCmdBindShadingRateImageNV)gpa(device, "vkCmdBindShadingRateImageNV");
    if (table->CmdBindShadingRateImageNV == nullptr) {
        table->CmdBindShadingRateImageNV = (PFN_vkCmdBindShadingRateImageNV)StubCmdBindShadingRateImageNV;
    }
    table->CmdSetViewportShadingRatePaletteNV =
        (PFN_vkCmdSetViewportShadingRatePaletteNV)gpa(device, "vkCmdSetViewportShadingRatePaletteNV");
    if (table->CmdSetViewportShadingRatePaletteNV == nullptr) {
        table->CmdSetViewportShadingRatePaletteNV =
            (PFN_vkCmdSetViewportShadingRatePaletteNV)StubCmdSetViewportShadingRatePaletteNV;
    }
    table->CmdSetCoarseSampleOrderNV = (PFN_vkCmdSetCoarseSampleOrderNV)gpa(device, "vkCmdSetCoarseSampleOrderNV");
    if (table->CmdSetCoarseSampleOrderNV == nullptr) {
        table->CmdSetCoarseSampleOrderNV = (PFN_vkCmdSetCoarseSampleOrderNV)StubCmdSetCoarseSampleOrderNV;
    }
    table->CreateAccelerationStructureNV = (PFN_vkCreateAccelerationStructureNV)gpa(device, "vkCreateAccelerationStructureNV");
    if (table->CreateAccelerationStructureNV == nullptr) {
        table->CreateAccelerationStructureNV = (PFN_vkCreateAccelerationStructureNV)StubCreateAccelerationStructureNV;
    }
    table->DestroyAccelerationStructureNV = (PFN_vkDestroyAccelerationStructureNV)gpa(device, "vkDestroyAccelerationStructureNV");
    if (table->DestroyAccelerationStructureNV == nullptr) {
        table->DestroyAccelerationStructureNV = (PFN_vkDestroyAccelerationStructureNV)StubDestroyAccelerationStructureNV;
    }
    table->GetAccelerationStructureMemoryRequirementsNV =
        (PFN_vkGetAccelerationStructureMemoryRequirementsNV)gpa(device, "vkGetAccelerationStructureMemoryRequirementsNV");
    if (table->GetAccelerationStructureMemoryRequirementsNV == nullptr) {
        table->GetAccelerationStructureMemoryRequirementsNV =
            (PFN_vkGetAccelerationStructureMemoryRequirementsNV)StubGetAccelerationStructureMemoryRequirementsNV;
    }
    table->BindAccelerationStructureMemoryNV =
        (PFN_vkBindAccelerationStructureMemoryNV)gpa(device, "vkBindAccelerationStructureMemoryNV");
    if (table->BindAccelerationStructureMemoryNV == nullptr) {
        table->BindAccelerationStructureMemoryNV = (PFN_vkBindAccelerationStructureMemoryNV)StubBindAccelerationStructureMemoryNV;
    }
    table->CmdBuildAccelerationStructureNV =
        (PFN_vkCmdBuildAccelerationStructureNV)gpa(device, "vkCmdBuildAccelerationStructureNV");
    if (table->CmdBuildAccelerationStructureNV == nullptr) {
        table->CmdBuildAccelerationStructureNV = (PFN_vkCmdBuildAccelerationStructureNV)StubCmdBuildAccelerationStructureNV;
    }
    table->CmdCopyAccelerationStructureNV = (PFN_vkCmdCopyAccelerationStructureNV)gpa(device, "vkCmdCopyAccelerationStructureNV");
    if (table->CmdCopyAccelerationStructureNV == nullptr) {
        table->CmdCopyAccelerationStructureNV = (PFN_vkCmdCopyAccelerationStructureNV)StubCmdCopyAccelerationStructureNV;
    }
    table->CmdTraceRaysNV = (PFN_vkCmdTraceRaysNV)gpa(device, "vkCmdTraceRaysNV");
    if (table->CmdTraceRaysNV == nullptr) {
        table->CmdTraceRaysNV = (PFN_vkCmdTraceRaysNV)StubCmdTraceRaysNV;
    }
    table->CreateRayTracingPipelinesNV = (PFN_vkCreateRayTracingPipelinesNV)gpa(device, "vkCreateRayTracingPipelinesNV");
    if (table->CreateRayTracingPipelinesNV == nullptr) {
        table->CreateRayTracingPipelinesNV = (PFN_vkCreateRayTracingPipelinesNV)StubCreateRayTracingPipelinesNV;
    }
    table->GetRayTracingShaderGroupHandlesKHR =
        (PFN_vkGetRayTracingShaderGroupHandlesKHR)gpa(device, "vkGetRayTracingShaderGroupHandlesKHR");
    if (table->GetRayTracingShaderGroupHandlesKHR == nullptr) {
        table->GetRayTracingShaderGroupHandlesKHR =
            (PFN_vkGetRayTracingShaderGroupHandlesKHR)StubGetRayTracingShaderGroupHandlesKHR;
    }
    table->GetRayTracingShaderGroupHandlesNV =
        (PFN_vkGetRayTracingShaderGroupHandlesNV)gpa(device, "vkGetRayTracingShaderGroupHandlesNV");
    if (table->GetRayTracingShaderGroupHandlesNV == nullptr) {
        table->GetRayTracingShaderGroupHandlesNV = (PFN_vkGetRayTracingShaderGroupHandlesNV)StubGetRayTracingShaderGroupHandlesNV;
    }
    table->GetAccelerationStructureHandleNV =
        (PFN_vkGetAccelerationStructureHandleNV)gpa(device, "vkGetAccelerationStructureHandleNV");
    if (table->GetAccelerationStructureHandleNV == nullptr) {
        table->GetAccelerationStructureHandleNV = (PFN_vkGetAccelerationStructureHandleNV)StubGetAccelerationStructureHandleNV;
    }
    table->CmdWriteAccelerationStructuresPropertiesNV =
        (PFN_vkCmdWriteAccelerationStructuresPropertiesNV)gpa(device, "vkCmdWriteAccelerationStructuresPropertiesNV");
    if (table->CmdWriteAccelerationStructuresPropertiesNV == nullptr) {
        table->CmdWriteAccelerationStructuresPropertiesNV =
            (PFN_vkCmdWriteAccelerationStructuresPropertiesNV)StubCmdWriteAccelerationStructuresPropertiesNV;
    }
    table->CompileDeferredNV = (PFN_vkCompileDeferredNV)gpa(device, "vkCompileDeferredNV");
    if (table->CompileDeferredNV == nullptr) {
        table->CompileDeferredNV = (PFN_vkCompileDeferredNV)StubCompileDeferredNV;
    }
    table->GetMemoryHostPointerPropertiesEXT =
        (PFN_vkGetMemoryHostPointerPropertiesEXT)gpa(device, "vkGetMemoryHostPointerPropertiesEXT");
    if (table->GetMemoryHostPointerPropertiesEXT == nullptr) {
        table->GetMemoryHostPointerPropertiesEXT = (PFN_vkGetMemoryHostPointerPropertiesEXT)StubGetMemoryHostPointerPropertiesEXT;
    }
    table->CmdWriteBufferMarkerAMD = (PFN_vkCmdWriteBufferMarkerAMD)gpa(device, "vkCmdWriteBufferMarkerAMD");
    if (table->CmdWriteBufferMarkerAMD == nullptr) {
        table->CmdWriteBufferMarkerAMD = (PFN_vkCmdWriteBufferMarkerAMD)StubCmdWriteBufferMarkerAMD;
    }
    table->GetCalibratedTimestampsEXT = (PFN_vkGetCalibratedTimestampsEXT)gpa(device, "vkGetCalibratedTimestampsEXT");
    if (table->GetCalibratedTimestampsEXT == nullptr) {
        table->GetCalibratedTimestampsEXT = (PFN_vkGetCalibratedTimestampsEXT)StubGetCalibratedTimestampsEXT;
    }
    table->CmdDrawMeshTasksNV = (PFN_vkCmdDrawMeshTasksNV)gpa(device, "vkCmdDrawMeshTasksNV");
    if (table->CmdDrawMeshTasksNV == nullptr) {
        table->CmdDrawMeshTasksNV = (PFN_vkCmdDrawMeshTasksNV)StubCmdDrawMeshTasksNV;
    }
    table->CmdDrawMeshTasksIndirectNV = (PFN_vkCmdDrawMeshTasksIndirectNV)gpa(device, "vkCmdDrawMeshTasksIndirectNV");
    if (table->CmdDrawMeshTasksIndirectNV == nullptr) {
        table->CmdDrawMeshTasksIndirectNV = (PFN_vkCmdDrawMeshTasksIndirectNV)StubCmdDrawMeshTasksIndirectNV;
    }
    table->CmdDrawMeshTasksIndirectCountNV =
        (PFN_vkCmdDrawMeshTasksIndirectCountNV)gpa(device, "vkCmdDrawMeshTasksIndirectCountNV");
    if (table->CmdDrawMeshTasksIndirectCountNV == nullptr) {
        table->CmdDrawMeshTasksIndirectCountNV = (PFN_vkCmdDrawMeshTasksIndirectCountNV)StubCmdDrawMeshTasksIndirectCountNV;
    }
    table->CmdSetExclusiveScissorEnableNV = (PFN_vkCmdSetExclusiveScissorEnableNV)gpa(device, "vkCmdSetExclusiveScissorEnableNV");
    if (table->CmdSetExclusiveScissorEnableNV == nullptr) {
        table->CmdSetExclusiveScissorEnableNV = (PFN_vkCmdSetExclusiveScissorEnableNV)StubCmdSetExclusiveScissorEnableNV;
    }
    table->CmdSetExclusiveScissorNV = (PFN_vkCmdSetExclusiveScissorNV)gpa(device, "vkCmdSetExclusiveScissorNV");
    if (table->CmdSetExclusiveScissorNV == nullptr) {
        table->CmdSetExclusiveScissorNV = (PFN_vkCmdSetExclusiveScissorNV)StubCmdSetExclusiveScissorNV;
    }
    table->CmdSetCheckpointNV = (PFN_vkCmdSetCheckpointNV)gpa(device, "vkCmdSetCheckpointNV");
    if (table->CmdSetCheckpointNV == nullptr) {
        table->CmdSetCheckpointNV = (PFN_vkCmdSetCheckpointNV)StubCmdSetCheckpointNV;
    }
    table->GetQueueCheckpointDataNV = (PFN_vkGetQueueCheckpointDataNV)gpa(device, "vkGetQueueCheckpointDataNV");
    if (table->GetQueueCheckpointDataNV == nullptr) {
        table->GetQueueCheckpointDataNV = (PFN_vkGetQueueCheckpointDataNV)StubGetQueueCheckpointDataNV;
    }
    table->InitializePerformanceApiINTEL = (PFN_vkInitializePerformanceApiINTEL)gpa(device, "vkInitializePerformanceApiINTEL");
    if (table->InitializePerformanceApiINTEL == nullptr) {
        table->InitializePerformanceApiINTEL = (PFN_vkInitializePerformanceApiINTEL)StubInitializePerformanceApiINTEL;
    }
    table->UninitializePerformanceApiINTEL =
        (PFN_vkUninitializePerformanceApiINTEL)gpa(device, "vkUninitializePerformanceApiINTEL");
    if (table->UninitializePerformanceApiINTEL == nullptr) {
        table->UninitializePerformanceApiINTEL = (PFN_vkUninitializePerformanceApiINTEL)StubUninitializePerformanceApiINTEL;
    }
    table->CmdSetPerformanceMarkerINTEL = (PFN_vkCmdSetPerformanceMarkerINTEL)gpa(device, "vkCmdSetPerformanceMarkerINTEL");
    if (table->CmdSetPerformanceMarkerINTEL == nullptr) {
        table->CmdSetPerformanceMarkerINTEL = (PFN_vkCmdSetPerformanceMarkerINTEL)StubCmdSetPerformanceMarkerINTEL;
    }
    table->CmdSetPerformanceStreamMarkerINTEL =
        (PFN_vkCmdSetPerformanceStreamMarkerINTEL)gpa(device, "vkCmdSetPerformanceStreamMarkerINTEL");
    if (table->CmdSetPerformanceStreamMarkerINTEL == nullptr) {
        table->CmdSetPerformanceStreamMarkerINTEL =
            (PFN_vkCmdSetPerformanceStreamMarkerINTEL)StubCmdSetPerformanceStreamMarkerINTEL;
    }
    table->CmdSetPerformanceOverrideINTEL = (PFN_vkCmdSetPerformanceOverrideINTEL)gpa(device, "vkCmdSetPerformanceOverrideINTEL");
    if (table->CmdSetPerformanceOverrideINTEL == nullptr) {
        table->CmdSetPerformanceOverrideINTEL = (PFN_vkCmdSetPerformanceOverrideINTEL)StubCmdSetPerformanceOverrideINTEL;
    }
    table->AcquirePerformanceConfigurationINTEL =
        (PFN_vkAcquirePerformanceConfigurationINTEL)gpa(device, "vkAcquirePerformanceConfigurationINTEL");
    if (table->AcquirePerformanceConfigurationINTEL == nullptr) {
        table->AcquirePerformanceConfigurationINTEL =
            (PFN_vkAcquirePerformanceConfigurationINTEL)StubAcquirePerformanceConfigurationINTEL;
    }
    table->ReleasePerformanceConfigurationINTEL =
        (PFN_vkReleasePerformanceConfigurationINTEL)gpa(device, "vkReleasePerformanceConfigurationINTEL");
    if (table->ReleasePerformanceConfigurationINTEL == nullptr) {
        table->ReleasePerformanceConfigurationINTEL =
            (PFN_vkReleasePerformanceConfigurationINTEL)StubReleasePerformanceConfigurationINTEL;
    }
    table->QueueSetPerformanceConfigurationINTEL =
        (PFN_vkQueueSetPerformanceConfigurationINTEL)gpa(device, "vkQueueSetPerformanceConfigurationINTEL");
    if (table->QueueSetPerformanceConfigurationINTEL == nullptr) {
        table->QueueSetPerformanceConfigurationINTEL =
            (PFN_vkQueueSetPerformanceConfigurationINTEL)StubQueueSetPerformanceConfigurationINTEL;
    }
    table->GetPerformanceParameterINTEL = (PFN_vkGetPerformanceParameterINTEL)gpa(device, "vkGetPerformanceParameterINTEL");
    if (table->GetPerformanceParameterINTEL == nullptr) {
        table->GetPerformanceParameterINTEL = (PFN_vkGetPerformanceParameterINTEL)StubGetPerformanceParameterINTEL;
    }
    table->SetLocalDimmingAMD = (PFN_vkSetLocalDimmingAMD)gpa(device, "vkSetLocalDimmingAMD");
    if (table->SetLocalDimmingAMD == nullptr) {
        table->SetLocalDimmingAMD = (PFN_vkSetLocalDimmingAMD)StubSetLocalDimmingAMD;
    }
    table->GetBufferDeviceAddressEXT = (PFN_vkGetBufferDeviceAddressEXT)gpa(device, "vkGetBufferDeviceAddressEXT");
    if (table->GetBufferDeviceAddressEXT == nullptr) {
        table->GetBufferDeviceAddressEXT = (PFN_vkGetBufferDeviceAddressEXT)StubGetBufferDeviceAddressEXT;
    }
#ifdef VK_USE_PLATFORM_WIN32_KHR
    table->AcquireFullScreenExclusiveModeEXT =
        (PFN_vkAcquireFullScreenExclusiveModeEXT)gpa(device, "vkAcquireFullScreenExclusiveModeEXT");
    if (table->AcquireFullScreenExclusiveModeEXT == nullptr) {
        table->AcquireFullScreenExclusiveModeEXT = (PFN_vkAcquireFullScreenExclusiveModeEXT)StubAcquireFullScreenExclusiveModeEXT;
    }
#endif  // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
    table->ReleaseFullScreenExclusiveModeEXT =
        (PFN_vkReleaseFullScreenExclusiveModeEXT)gpa(device, "vkReleaseFullScreenExclusiveModeEXT");
    if (table->ReleaseFullScreenExclusiveModeEXT == nullptr) {
        table->ReleaseFullScreenExclusiveModeEXT = (PFN_vkReleaseFullScreenExclusiveModeEXT)StubReleaseFullScreenExclusiveModeEXT;
    }
#endif  // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
    table->GetDeviceGroupSurfacePresentModes2EXT =
        (PFN_vkGetDeviceGroupSurfacePresentModes2EXT)gpa(device, "vkGetDeviceGroupSurfacePresentModes2EXT");
    if (table->GetDeviceGroupSurfacePresentModes2EXT == nullptr) {
        table->GetDeviceGroupSurfacePresentModes2EXT =
            (PFN_vkGetDeviceGroupSurfacePresentModes2EXT)StubGetDeviceGroupSurfacePresentModes2EXT;
    }
#endif  // VK_USE_PLATFORM_WIN32_KHR
    table->CmdSetLineStippleEXT = (PFN_vkCmdSetLineStippleEXT)gpa(device, "vkCmdSetLineStippleEXT");
    if (table->CmdSetLineStippleEXT == nullptr) {
        table->CmdSetLineStippleEXT = (PFN_vkCmdSetLineStippleEXT)StubCmdSetLineStippleEXT;
    }
    table->ResetQueryPoolEXT = (PFN_vkResetQueryPoolEXT)gpa(device, "vkResetQueryPoolEXT");
    if (table->ResetQueryPoolEXT == nullptr) {
        table->ResetQueryPoolEXT = (PFN_vkResetQueryPoolEXT)StubResetQueryPoolEXT;
    }
    table->CmdSetCullModeEXT = (PFN_vkCmdSetCullModeEXT)gpa(device, "vkCmdSetCullModeEXT");
    if (table->CmdSetCullModeEXT == nullptr) {
        table->CmdSetCullModeEXT = (PFN_vkCmdSetCullModeEXT)StubCmdSetCullModeEXT;
    }
    table->CmdSetFrontFaceEXT = (PFN_vkCmdSetFrontFaceEXT)gpa(device, "vkCmdSetFrontFaceEXT");
    if (table->CmdSetFrontFaceEXT == nullptr) {
        table->CmdSetFrontFaceEXT = (PFN_vkCmdSetFrontFaceEXT)StubCmdSetFrontFaceEXT;
    }
    table->CmdSetPrimitiveTopologyEXT = (PFN_vkCmdSetPrimitiveTopologyEXT)gpa(device, "vkCmdSetPrimitiveTopologyEXT");
    if (table->CmdSetPrimitiveTopologyEXT == nullptr) {
        table->CmdSetPrimitiveTopologyEXT = (PFN_vkCmdSetPrimitiveTopologyEXT)StubCmdSetPrimitiveTopologyEXT;
    }
    table->CmdSetViewportWithCountEXT = (PFN_vkCmdSetViewportWithCountEXT)gpa(device, "vkCmdSetViewportWithCountEXT");
    if (table->CmdSetViewportWithCountEXT == nullptr) {
        table->CmdSetViewportWithCountEXT = (PFN_vkCmdSetViewportWithCountEXT)StubCmdSetViewportWithCountEXT;
    }
    table->CmdSetScissorWithCountEXT = (PFN_vkCmdSetScissorWithCountEXT)gpa(device, "vkCmdSetScissorWithCountEXT");
    if (table->CmdSetScissorWithCountEXT == nullptr) {
        table->CmdSetScissorWithCountEXT = (PFN_vkCmdSetScissorWithCountEXT)StubCmdSetScissorWithCountEXT;
    }
    table->CmdBindVertexBuffers2EXT = (PFN_vkCmdBindVertexBuffers2EXT)gpa(device, "vkCmdBindVertexBuffers2EXT");
    if (table->CmdBindVertexBuffers2EXT == nullptr) {
        table->CmdBindVertexBuffers2EXT = (PFN_vkCmdBindVertexBuffers2EXT)StubCmdBindVertexBuffers2EXT;
    }
    table->CmdSetDepthTestEnableEXT = (PFN_vkCmdSetDepthTestEnableEXT)gpa(device, "vkCmdSetDepthTestEnableEXT");
    if (table->CmdSetDepthTestEnableEXT == nullptr) {
        table->CmdSetDepthTestEnableEXT = (PFN_vkCmdSetDepthTestEnableEXT)StubCmdSetDepthTestEnableEXT;
    }
    table->CmdSetDepthWriteEnableEXT = (PFN_vkCmdSetDepthWriteEnableEXT)gpa(device, "vkCmdSetDepthWriteEnableEXT");
    if (table->CmdSetDepthWriteEnableEXT == nullptr) {
        table->CmdSetDepthWriteEnableEXT = (PFN_vkCmdSetDepthWriteEnableEXT)StubCmdSetDepthWriteEnableEXT;
    }
    table->CmdSetDepthCompareOpEXT = (PFN_vkCmdSetDepthCompareOpEXT)gpa(device, "vkCmdSetDepthCompareOpEXT");
    if (table->CmdSetDepthCompareOpEXT == nullptr) {
        table->CmdSetDepthCompareOpEXT = (PFN_vkCmdSetDepthCompareOpEXT)StubCmdSetDepthCompareOpEXT;
    }
    table->CmdSetDepthBoundsTestEnableEXT = (PFN_vkCmdSetDepthBoundsTestEnableEXT)gpa(device, "vkCmdSetDepthBoundsTestEnableEXT");
    if (table->CmdSetDepthBoundsTestEnableEXT == nullptr) {
        table->CmdSetDepthBoundsTestEnableEXT = (PFN_vkCmdSetDepthBoundsTestEnableEXT)StubCmdSetDepthBoundsTestEnableEXT;
    }
    table->CmdSetStencilTestEnableEXT = (PFN_vkCmdSetStencilTestEnableEXT)gpa(device, "vkCmdSetStencilTestEnableEXT");
    if (table->CmdSetStencilTestEnableEXT == nullptr) {
        table->CmdSetStencilTestEnableEXT = (PFN_vkCmdSetStencilTestEnableEXT)StubCmdSetStencilTestEnableEXT;
    }
    table->CmdSetStencilOpEXT = (PFN_vkCmdSetStencilOpEXT)gpa(device, "vkCmdSetStencilOpEXT");
    if (table->CmdSetStencilOpEXT == nullptr) {
        table->CmdSetStencilOpEXT = (PFN_vkCmdSetStencilOpEXT)StubCmdSetStencilOpEXT;
    }
    table->CopyMemoryToImageEXT = (PFN_vkCopyMemoryToImageEXT)gpa(device, "vkCopyMemoryToImageEXT");
    if (table->CopyMemoryToImageEXT == nullptr) {
        table->CopyMemoryToImageEXT = (PFN_vkCopyMemoryToImageEXT)StubCopyMemoryToImageEXT;
    }
    table->CopyImageToMemoryEXT = (PFN_vkCopyImageToMemoryEXT)gpa(device, "vkCopyImageToMemoryEXT");
    if (table->CopyImageToMemoryEXT == nullptr) {
        table->CopyImageToMemoryEXT = (PFN_vkCopyImageToMemoryEXT)StubCopyImageToMemoryEXT;
    }
    table->CopyImageToImageEXT = (PFN_vkCopyImageToImageEXT)gpa(device, "vkCopyImageToImageEXT");
    if (table->CopyImageToImageEXT == nullptr) {
        table->CopyImageToImageEXT = (PFN_vkCopyImageToImageEXT)StubCopyImageToImageEXT;
    }
    table->TransitionImageLayoutEXT = (PFN_vkTransitionImageLayoutEXT)gpa(device, "vkTransitionImageLayoutEXT");
    if (table->TransitionImageLayoutEXT == nullptr) {
        table->TransitionImageLayoutEXT = (PFN_vkTransitionImageLayoutEXT)StubTransitionImageLayoutEXT;
    }
    table->GetImageSubresourceLayout2EXT = (PFN_vkGetImageSubresourceLayout2EXT)gpa(device, "vkGetImageSubresourceLayout2EXT");
    if (table->GetImageSubresourceLayout2EXT == nullptr) {
        table->GetImageSubresourceLayout2EXT = (PFN_vkGetImageSubresourceLayout2EXT)StubGetImageSubresourceLayout2EXT;
    }
    table->ReleaseSwapchainImagesEXT = (PFN_vkReleaseSwapchainImagesEXT)gpa(device, "vkReleaseSwapchainImagesEXT");
    if (table->ReleaseSwapchainImagesEXT == nullptr) {
        table->ReleaseSwapchainImagesEXT = (PFN_vkReleaseSwapchainImagesEXT)StubReleaseSwapchainImagesEXT;
    }
    table->GetGeneratedCommandsMemoryRequirementsNV =
        (PFN_vkGetGeneratedCommandsMemoryRequirementsNV)gpa(device, "vkGetGeneratedCommandsMemoryRequirementsNV");
    if (table->GetGeneratedCommandsMemoryRequirementsNV == nullptr) {
        table->GetGeneratedCommandsMemoryRequirementsNV =
            (PFN_vkGetGeneratedCommandsMemoryRequirementsNV)StubGetGeneratedCommandsMemoryRequirementsNV;
    }
    table->CmdPreprocessGeneratedCommandsNV =
        (PFN_vkCmdPreprocessGeneratedCommandsNV)gpa(device, "vkCmdPreprocessGeneratedCommandsNV");
    if (table->CmdPreprocessGeneratedCommandsNV == nullptr) {
        table->CmdPreprocessGeneratedCommandsNV = (PFN_vkCmdPreprocessGeneratedCommandsNV)StubCmdPreprocessGeneratedCommandsNV;
    }
    table->CmdExecuteGeneratedCommandsNV = (PFN_vkCmdExecuteGeneratedCommandsNV)gpa(device, "vkCmdExecuteGeneratedCommandsNV");
    if (table->CmdExecuteGeneratedCommandsNV == nullptr) {
        table->CmdExecuteGeneratedCommandsNV = (PFN_vkCmdExecuteGeneratedCommandsNV)StubCmdExecuteGeneratedCommandsNV;
    }
    table->CmdBindPipelineShaderGroupNV = (PFN_vkCmdBindPipelineShaderGroupNV)gpa(device, "vkCmdBindPipelineShaderGroupNV");
    if (table->CmdBindPipelineShaderGroupNV == nullptr) {
        table->CmdBindPipelineShaderGroupNV = (PFN_vkCmdBindPipelineShaderGroupNV)StubCmdBindPipelineShaderGroupNV;
    }
    table->CreateIndirectCommandsLayoutNV = (PFN_vkCreateIndirectCommandsLayoutNV)gpa(device, "vkCreateIndirectCommandsLayoutNV");
    if (table->CreateIndirectCommandsLayoutNV == nullptr) {
        table->CreateIndirectCommandsLayoutNV = (PFN_vkCreateIndirectCommandsLayoutNV)StubCreateIndirectCommandsLayoutNV;
    }
    table->DestroyIndirectCommandsLayoutNV =
        (PFN_vkDestroyIndirectCommandsLayoutNV)gpa(device, "vkDestroyIndirectCommandsLayoutNV");
    if (table->DestroyIndirectCommandsLayoutNV == nullptr) {
        table->DestroyIndirectCommandsLayoutNV = (PFN_vkDestroyIndirectCommandsLayoutNV)StubDestroyIndirectCommandsLayoutNV;
    }
    table->CmdSetDepthBias2EXT = (PFN_vkCmdSetDepthBias2EXT)gpa(device, "vkCmdSetDepthBias2EXT");
    if (table->CmdSetDepthBias2EXT == nullptr) {
        table->CmdSetDepthBias2EXT = (PFN_vkCmdSetDepthBias2EXT)StubCmdSetDepthBias2EXT;
    }
    table->CreatePrivateDataSlotEXT = (PFN_vkCreatePrivateDataSlotEXT)gpa(device, "vkCreatePrivateDataSlotEXT");
    if (table->CreatePrivateDataSlotEXT == nullptr) {
        table->CreatePrivateDataSlotEXT = (PFN_vkCreatePrivateDataSlotEXT)StubCreatePrivateDataSlotEXT;
    }
    table->DestroyPrivateDataSlotEXT = (PFN_vkDestroyPrivateDataSlotEXT)gpa(device, "vkDestroyPrivateDataSlotEXT");
    if (table->DestroyPrivateDataSlotEXT == nullptr) {
        table->DestroyPrivateDataSlotEXT = (PFN_vkDestroyPrivateDataSlotEXT)StubDestroyPrivateDataSlotEXT;
    }
    table->SetPrivateDataEXT = (PFN_vkSetPrivateDataEXT)gpa(device, "vkSetPrivateDataEXT");
    if (table->SetPrivateDataEXT == nullptr) {
        table->SetPrivateDataEXT = (PFN_vkSetPrivateDataEXT)StubSetPrivateDataEXT;
    }
    table->GetPrivateDataEXT = (PFN_vkGetPrivateDataEXT)gpa(device, "vkGetPrivateDataEXT");
    if (table->GetPrivateDataEXT == nullptr) {
        table->GetPrivateDataEXT = (PFN_vkGetPrivateDataEXT)StubGetPrivateDataEXT;
    }
#ifdef VK_USE_PLATFORM_METAL_EXT
    table->ExportMetalObjectsEXT = (PFN_vkExportMetalObjectsEXT)gpa(device, "vkExportMetalObjectsEXT");
    if (table->ExportMetalObjectsEXT == nullptr) {
        table->ExportMetalObjectsEXT = (PFN_vkExportMetalObjectsEXT)StubExportMetalObjectsEXT;
    }
#endif  // VK_USE_PLATFORM_METAL_EXT
    table->GetDescriptorSetLayoutSizeEXT = (PFN_vkGetDescriptorSetLayoutSizeEXT)gpa(device, "vkGetDescriptorSetLayoutSizeEXT");
    if (table->GetDescriptorSetLayoutSizeEXT == nullptr) {
        table->GetDescriptorSetLayoutSizeEXT = (PFN_vkGetDescriptorSetLayoutSizeEXT)StubGetDescriptorSetLayoutSizeEXT;
    }
    table->GetDescriptorSetLayoutBindingOffsetEXT =
        (PFN_vkGetDescriptorSetLayoutBindingOffsetEXT)gpa(device, "vkGetDescriptorSetLayoutBindingOffsetEXT");
    if (table->GetDescriptorSetLayoutBindingOffsetEXT == nullptr) {
        table->GetDescriptorSetLayoutBindingOffsetEXT =
            (PFN_vkGetDescriptorSetLayoutBindingOffsetEXT)StubGetDescriptorSetLayoutBindingOffsetEXT;
    }
    table->GetDescriptorEXT = (PFN_vkGetDescriptorEXT)gpa(device, "vkGetDescriptorEXT");
    if (table->GetDescriptorEXT == nullptr) {
        table->GetDescriptorEXT = (PFN_vkGetDescriptorEXT)StubGetDescriptorEXT;
    }
    table->CmdBindDescriptorBuffersEXT = (PFN_vkCmdBindDescriptorBuffersEXT)gpa(device, "vkCmdBindDescriptorBuffersEXT");
    if (table->CmdBindDescriptorBuffersEXT == nullptr) {
        table->CmdBindDescriptorBuffersEXT = (PFN_vkCmdBindDescriptorBuffersEXT)StubCmdBindDescriptorBuffersEXT;
    }
    table->CmdSetDescriptorBufferOffsetsEXT =
        (PFN_vkCmdSetDescriptorBufferOffsetsEXT)gpa(device, "vkCmdSetDescriptorBufferOffsetsEXT");
    if (table->CmdSetDescriptorBufferOffsetsEXT == nullptr) {
        table->CmdSetDescriptorBufferOffsetsEXT = (PFN_vkCmdSetDescriptorBufferOffsetsEXT)StubCmdSetDescriptorBufferOffsetsEXT;
    }
    table->CmdBindDescriptorBufferEmbeddedSamplersEXT =
        (PFN_vkCmdBindDescriptorBufferEmbeddedSamplersEXT)gpa(device, "vkCmdBindDescriptorBufferEmbeddedSamplersEXT");
    if (table->CmdBindDescriptorBufferEmbeddedSamplersEXT == nullptr) {
        table->CmdBindDescriptorBufferEmbeddedSamplersEXT =
            (PFN_vkCmdBindDescriptorBufferEmbeddedSamplersEXT)StubCmdBindDescriptorBufferEmbeddedSamplersEXT;
    }
    table->GetBufferOpaqueCaptureDescriptorDataEXT =
        (PFN_vkGetBufferOpaqueCaptureDescriptorDataEXT)gpa(device, "vkGetBufferOpaqueCaptureDescriptorDataEXT");
    if (table->GetBufferOpaqueCaptureDescriptorDataEXT == nullptr) {
        table->GetBufferOpaqueCaptureDescriptorDataEXT =
            (PFN_vkGetBufferOpaqueCaptureDescriptorDataEXT)StubGetBufferOpaqueCaptureDescriptorDataEXT;
    }
    table->GetImageOpaqueCaptureDescriptorDataEXT =
        (PFN_vkGetImageOpaqueCaptureDescriptorDataEXT)gpa(device, "vkGetImageOpaqueCaptureDescriptorDataEXT");
    if (table->GetImageOpaqueCaptureDescriptorDataEXT == nullptr) {
        table->GetImageOpaqueCaptureDescriptorDataEXT =
            (PFN_vkGetImageOpaqueCaptureDescriptorDataEXT)StubGetImageOpaqueCaptureDescriptorDataEXT;
    }
    table->GetImageViewOpaqueCaptureDescriptorDataEXT =
        (PFN_vkGetImageViewOpaqueCaptureDescriptorDataEXT)gpa(device, "vkGetImageViewOpaqueCaptureDescriptorDataEXT");
    if (table->GetImageViewOpaqueCaptureDescriptorDataEXT == nullptr) {
        table->GetImageViewOpaqueCaptureDescriptorDataEXT =
            (PFN_vkGetImageViewOpaqueCaptureDescriptorDataEXT)StubGetImageViewOpaqueCaptureDescriptorDataEXT;
    }
    table->GetSamplerOpaqueCaptureDescriptorDataEXT =
        (PFN_vkGetSamplerOpaqueCaptureDescriptorDataEXT)gpa(device, "vkGetSamplerOpaqueCaptureDescriptorDataEXT");
    if (table->GetSamplerOpaqueCaptureDescriptorDataEXT == nullptr) {
        table->GetSamplerOpaqueCaptureDescriptorDataEXT =
            (PFN_vkGetSamplerOpaqueCaptureDescriptorDataEXT)StubGetSamplerOpaqueCaptureDescriptorDataEXT;
    }
    table->GetAccelerationStructureOpaqueCaptureDescriptorDataEXT =
        (PFN_vkGetAccelerationStructureOpaqueCaptureDescriptorDataEXT)gpa(
            device, "vkGetAccelerationStructureOpaqueCaptureDescriptorDataEXT");
    if (table->GetAccelerationStructureOpaqueCaptureDescriptorDataEXT == nullptr) {
        table->GetAccelerationStructureOpaqueCaptureDescriptorDataEXT =
            (PFN_vkGetAccelerationStructureOpaqueCaptureDescriptorDataEXT)
                StubGetAccelerationStructureOpaqueCaptureDescriptorDataEXT;
    }
    table->CmdSetFragmentShadingRateEnumNV =
        (PFN_vkCmdSetFragmentShadingRateEnumNV)gpa(device, "vkCmdSetFragmentShadingRateEnumNV");
    if (table->CmdSetFragmentShadingRateEnumNV == nullptr) {
        table->CmdSetFragmentShadingRateEnumNV = (PFN_vkCmdSetFragmentShadingRateEnumNV)StubCmdSetFragmentShadingRateEnumNV;
    }
    table->GetDeviceFaultInfoEXT = (PFN_vkGetDeviceFaultInfoEXT)gpa(device, "vkGetDeviceFaultInfoEXT");
    if (table->GetDeviceFaultInfoEXT == nullptr) {
        table->GetDeviceFaultInfoEXT = (PFN_vkGetDeviceFaultInfoEXT)StubGetDeviceFaultInfoEXT;
    }
    table->CmdSetVertexInputEXT = (PFN_vkCmdSetVertexInputEXT)gpa(device, "vkCmdSetVertexInputEXT");
    if (table->CmdSetVertexInputEXT == nullptr) {
        table->CmdSetVertexInputEXT = (PFN_vkCmdSetVertexInputEXT)StubCmdSetVertexInputEXT;
    }
#ifdef VK_USE_PLATFORM_FUCHSIA
    table->GetMemoryZirconHandleFUCHSIA = (PFN_vkGetMemoryZirconHandleFUCHSIA)gpa(device, "vkGetMemoryZirconHandleFUCHSIA");
    if (table->GetMemoryZirconHandleFUCHSIA == nullptr) {
        table->GetMemoryZirconHandleFUCHSIA = (PFN_vkGetMemoryZirconHandleFUCHSIA)StubGetMemoryZirconHandleFUCHSIA;
    }
#endif  // VK_USE_PLATFORM_FUCHSIA
#ifdef VK_USE_PLATFORM_FUCHSIA
    table->GetMemoryZirconHandlePropertiesFUCHSIA =
        (PFN_vkGetMemoryZirconHandlePropertiesFUCHSIA)gpa(device, "vkGetMemoryZirconHandlePropertiesFUCHSIA");
    if (table->GetMemoryZirconHandlePropertiesFUCHSIA == nullptr) {
        table->GetMemoryZirconHandlePropertiesFUCHSIA =
            (PFN_vkGetMemoryZirconHandlePropertiesFUCHSIA)StubGetMemoryZirconHandlePropertiesFUCHSIA;
    }
#endif  // VK_USE_PLATFORM_FUCHSIA
#ifdef VK_USE_PLATFORM_FUCHSIA
    table->ImportSemaphoreZirconHandleFUCHSIA =
        (PFN_vkImportSemaphoreZirconHandleFUCHSIA)gpa(device, "vkImportSemaphoreZirconHandleFUCHSIA");
    if (table->ImportSemaphoreZirconHandleFUCHSIA == nullptr) {
        table->ImportSemaphoreZirconHandleFUCHSIA =
            (PFN_vkImportSemaphoreZirconHandleFUCHSIA)StubImportSemaphoreZirconHandleFUCHSIA;
    }
#endif  // VK_USE_PLATFORM_FUCHSIA
#ifdef VK_USE_PLATFORM_FUCHSIA
    table->GetSemaphoreZirconHandleFUCHSIA =
        (PFN_vkGetSemaphoreZirconHandleFUCHSIA)gpa(device, "vkGetSemaphoreZirconHandleFUCHSIA");
    if (table->GetSemaphoreZirconHandleFUCHSIA == nullptr) {
        table->GetSemaphoreZirconHandleFUCHSIA = (PFN_vkGetSemaphoreZirconHandleFUCHSIA)StubGetSemaphoreZirconHandleFUCHSIA;
    }
#endif  // VK_USE_PLATFORM_FUCHSIA
#ifdef VK_USE_PLATFORM_FUCHSIA
    table->CreateBufferCollectionFUCHSIA = (PFN_vkCreateBufferCollectionFUCHSIA)gpa(device, "vkCreateBufferCollectionFUCHSIA");
    if (table->CreateBufferCollectionFUCHSIA == nullptr) {
        table->CreateBufferCollectionFUCHSIA = (PFN_vkCreateBufferCollectionFUCHSIA)StubCreateBufferCollectionFUCHSIA;
    }
#endif  // VK_USE_PLATFORM_FUCHSIA
#ifdef VK_USE_PLATFORM_FUCHSIA
    table->SetBufferCollectionImageConstraintsFUCHSIA =
        (PFN_vkSetBufferCollectionImageConstraintsFUCHSIA)gpa(device, "vkSetBufferCollectionImageConstraintsFUCHSIA");
    if (table->SetBufferCollectionImageConstraintsFUCHSIA == nullptr) {
        table->SetBufferCollectionImageConstraintsFUCHSIA =
            (PFN_vkSetBufferCollectionImageConstraintsFUCHSIA)StubSetBufferCollectionImageConstraintsFUCHSIA;
    }
#endif  // VK_USE_PLATFORM_FUCHSIA
#ifdef VK_USE_PLATFORM_FUCHSIA
    table->SetBufferCollectionBufferConstraintsFUCHSIA =
        (PFN_vkSetBufferCollectionBufferConstraintsFUCHSIA)gpa(device, "vkSetBufferCollectionBufferConstraintsFUCHSIA");
    if (table->SetBufferCollectionBufferConstraintsFUCHSIA == nullptr) {
        table->SetBufferCollectionBufferConstraintsFUCHSIA =
            (PFN_vkSetBufferCollectionBufferConstraintsFUCHSIA)StubSetBufferCollectionBufferConstraintsFUCHSIA;
    }
#endif  // VK_USE_PLATFORM_FUCHSIA
#ifdef VK_USE_PLATFORM_FUCHSIA
    table->DestroyBufferCollectionFUCHSIA = (PFN_vkDestroyBufferCollectionFUCHSIA)gpa(device, "vkDestroyBufferCollectionFUCHSIA");
    if (table->DestroyBufferCollectionFUCHSIA == nullptr) {
        table->DestroyBufferCollectionFUCHSIA = (PFN_vkDestroyBufferCollectionFUCHSIA)StubDestroyBufferCollectionFUCHSIA;
    }
#endif  // VK_USE_PLATFORM_FUCHSIA
#ifdef VK_USE_PLATFORM_FUCHSIA
    table->GetBufferCollectionPropertiesFUCHSIA =
        (PFN_vkGetBufferCollectionPropertiesFUCHSIA)gpa(device, "vkGetBufferCollectionPropertiesFUCHSIA");
    if (table->GetBufferCollectionPropertiesFUCHSIA == nullptr) {
        table->GetBufferCollectionPropertiesFUCHSIA =
            (PFN_vkGetBufferCollectionPropertiesFUCHSIA)StubGetBufferCollectionPropertiesFUCHSIA;
    }
#endif  // VK_USE_PLATFORM_FUCHSIA
    table->GetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI =
        (PFN_vkGetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI)gpa(device, "vkGetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI");
    if (table->GetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI == nullptr) {
        table->GetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI =
            (PFN_vkGetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI)StubGetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI;
    }
    table->CmdSubpassShadingHUAWEI = (PFN_vkCmdSubpassShadingHUAWEI)gpa(device, "vkCmdSubpassShadingHUAWEI");
    if (table->CmdSubpassShadingHUAWEI == nullptr) {
        table->CmdSubpassShadingHUAWEI = (PFN_vkCmdSubpassShadingHUAWEI)StubCmdSubpassShadingHUAWEI;
    }
    table->CmdBindInvocationMaskHUAWEI = (PFN_vkCmdBindInvocationMaskHUAWEI)gpa(device, "vkCmdBindInvocationMaskHUAWEI");
    if (table->CmdBindInvocationMaskHUAWEI == nullptr) {
        table->CmdBindInvocationMaskHUAWEI = (PFN_vkCmdBindInvocationMaskHUAWEI)StubCmdBindInvocationMaskHUAWEI;
    }
    table->GetMemoryRemoteAddressNV = (PFN_vkGetMemoryRemoteAddressNV)gpa(device, "vkGetMemoryRemoteAddressNV");
    if (table->GetMemoryRemoteAddressNV == nullptr) {
        table->GetMemoryRemoteAddressNV = (PFN_vkGetMemoryRemoteAddressNV)StubGetMemoryRemoteAddressNV;
    }
    table->GetPipelinePropertiesEXT = (PFN_vkGetPipelinePropertiesEXT)gpa(device, "vkGetPipelinePropertiesEXT");
    if (table->GetPipelinePropertiesEXT == nullptr) {
        table->GetPipelinePropertiesEXT = (PFN_vkGetPipelinePropertiesEXT)StubGetPipelinePropertiesEXT;
    }
    table->CmdSetPatchControlPointsEXT = (PFN_vkCmdSetPatchControlPointsEXT)gpa(device, "vkCmdSetPatchControlPointsEXT");
    if (table->CmdSetPatchControlPointsEXT == nullptr) {
        table->CmdSetPatchControlPointsEXT = (PFN_vkCmdSetPatchControlPointsEXT)StubCmdSetPatchControlPointsEXT;
    }
    table->CmdSetRasterizerDiscardEnableEXT =
        (PFN_vkCmdSetRasterizerDiscardEnableEXT)gpa(device, "vkCmdSetRasterizerDiscardEnableEXT");
    if (table->CmdSetRasterizerDiscardEnableEXT == nullptr) {
        table->CmdSetRasterizerDiscardEnableEXT = (PFN_vkCmdSetRasterizerDiscardEnableEXT)StubCmdSetRasterizerDiscardEnableEXT;
    }
    table->CmdSetDepthBiasEnableEXT = (PFN_vkCmdSetDepthBiasEnableEXT)gpa(device, "vkCmdSetDepthBiasEnableEXT");
    if (table->CmdSetDepthBiasEnableEXT == nullptr) {
        table->CmdSetDepthBiasEnableEXT = (PFN_vkCmdSetDepthBiasEnableEXT)StubCmdSetDepthBiasEnableEXT;
    }
    table->CmdSetLogicOpEXT = (PFN_vkCmdSetLogicOpEXT)gpa(device, "vkCmdSetLogicOpEXT");
    if (table->CmdSetLogicOpEXT == nullptr) {
        table->CmdSetLogicOpEXT = (PFN_vkCmdSetLogicOpEXT)StubCmdSetLogicOpEXT;
    }
    table->CmdSetPrimitiveRestartEnableEXT =
        (PFN_vkCmdSetPrimitiveRestartEnableEXT)gpa(device, "vkCmdSetPrimitiveRestartEnableEXT");
    if (table->CmdSetPrimitiveRestartEnableEXT == nullptr) {
        table->CmdSetPrimitiveRestartEnableEXT = (PFN_vkCmdSetPrimitiveRestartEnableEXT)StubCmdSetPrimitiveRestartEnableEXT;
    }
    table->CmdSetColorWriteEnableEXT = (PFN_vkCmdSetColorWriteEnableEXT)gpa(device, "vkCmdSetColorWriteEnableEXT");
    if (table->CmdSetColorWriteEnableEXT == nullptr) {
        table->CmdSetColorWriteEnableEXT = (PFN_vkCmdSetColorWriteEnableEXT)StubCmdSetColorWriteEnableEXT;
    }
    table->CmdDrawMultiEXT = (PFN_vkCmdDrawMultiEXT)gpa(device, "vkCmdDrawMultiEXT");
    if (table->CmdDrawMultiEXT == nullptr) {
        table->CmdDrawMultiEXT = (PFN_vkCmdDrawMultiEXT)StubCmdDrawMultiEXT;
    }
    table->CmdDrawMultiIndexedEXT = (PFN_vkCmdDrawMultiIndexedEXT)gpa(device, "vkCmdDrawMultiIndexedEXT");
    if (table->CmdDrawMultiIndexedEXT == nullptr) {
        table->CmdDrawMultiIndexedEXT = (PFN_vkCmdDrawMultiIndexedEXT)StubCmdDrawMultiIndexedEXT;
    }
    table->CreateMicromapEXT = (PFN_vkCreateMicromapEXT)gpa(device, "vkCreateMicromapEXT");
    if (table->CreateMicromapEXT == nullptr) {
        table->CreateMicromapEXT = (PFN_vkCreateMicromapEXT)StubCreateMicromapEXT;
    }
    table->DestroyMicromapEXT = (PFN_vkDestroyMicromapEXT)gpa(device, "vkDestroyMicromapEXT");
    if (table->DestroyMicromapEXT == nullptr) {
        table->DestroyMicromapEXT = (PFN_vkDestroyMicromapEXT)StubDestroyMicromapEXT;
    }
    table->CmdBuildMicromapsEXT = (PFN_vkCmdBuildMicromapsEXT)gpa(device, "vkCmdBuildMicromapsEXT");
    if (table->CmdBuildMicromapsEXT == nullptr) {
        table->CmdBuildMicromapsEXT = (PFN_vkCmdBuildMicromapsEXT)StubCmdBuildMicromapsEXT;
    }
    table->BuildMicromapsEXT = (PFN_vkBuildMicromapsEXT)gpa(device, "vkBuildMicromapsEXT");
    if (table->BuildMicromapsEXT == nullptr) {
        table->BuildMicromapsEXT = (PFN_vkBuildMicromapsEXT)StubBuildMicromapsEXT;
    }
    table->CopyMicromapEXT = (PFN_vkCopyMicromapEXT)gpa(device, "vkCopyMicromapEXT");
    if (table->CopyMicromapEXT == nullptr) {
        table->CopyMicromapEXT = (PFN_vkCopyMicromapEXT)StubCopyMicromapEXT;
    }
    table->CopyMicromapToMemoryEXT = (PFN_vkCopyMicromapToMemoryEXT)gpa(device, "vkCopyMicromapToMemoryEXT");
    if (table->CopyMicromapToMemoryEXT == nullptr) {
        table->CopyMicromapToMemoryEXT = (PFN_vkCopyMicromapToMemoryEXT)StubCopyMicromapToMemoryEXT;
    }
    table->CopyMemoryToMicromapEXT = (PFN_vkCopyMemoryToMicromapEXT)gpa(device, "vkCopyMemoryToMicromapEXT");
    if (table->CopyMemoryToMicromapEXT == nullptr) {
        table->CopyMemoryToMicromapEXT = (PFN_vkCopyMemoryToMicromapEXT)StubCopyMemoryToMicromapEXT;
    }
    table->WriteMicromapsPropertiesEXT = (PFN_vkWriteMicromapsPropertiesEXT)gpa(device, "vkWriteMicromapsPropertiesEXT");
    if (table->WriteMicromapsPropertiesEXT == nullptr) {
        table->WriteMicromapsPropertiesEXT = (PFN_vkWriteMicromapsPropertiesEXT)StubWriteMicromapsPropertiesEXT;
    }
    table->CmdCopyMicromapEXT = (PFN_vkCmdCopyMicromapEXT)gpa(device, "vkCmdCopyMicromapEXT");
    if (table->CmdCopyMicromapEXT == nullptr) {
        table->CmdCopyMicromapEXT = (PFN_vkCmdCopyMicromapEXT)StubCmdCopyMicromapEXT;
    }
    table->CmdCopyMicromapToMemoryEXT = (PFN_vkCmdCopyMicromapToMemoryEXT)gpa(device, "vkCmdCopyMicromapToMemoryEXT");
    if (table->CmdCopyMicromapToMemoryEXT == nullptr) {
        table->CmdCopyMicromapToMemoryEXT = (PFN_vkCmdCopyMicromapToMemoryEXT)StubCmdCopyMicromapToMemoryEXT;
    }
    table->CmdCopyMemoryToMicromapEXT = (PFN_vkCmdCopyMemoryToMicromapEXT)gpa(device, "vkCmdCopyMemoryToMicromapEXT");
    if (table->CmdCopyMemoryToMicromapEXT == nullptr) {
        table->CmdCopyMemoryToMicromapEXT = (PFN_vkCmdCopyMemoryToMicromapEXT)StubCmdCopyMemoryToMicromapEXT;
    }
    table->CmdWriteMicromapsPropertiesEXT = (PFN_vkCmdWriteMicromapsPropertiesEXT)gpa(device, "vkCmdWriteMicromapsPropertiesEXT");
    if (table->CmdWriteMicromapsPropertiesEXT == nullptr) {
        table->CmdWriteMicromapsPropertiesEXT = (PFN_vkCmdWriteMicromapsPropertiesEXT)StubCmdWriteMicromapsPropertiesEXT;
    }
    table->GetDeviceMicromapCompatibilityEXT =
        (PFN_vkGetDeviceMicromapCompatibilityEXT)gpa(device, "vkGetDeviceMicromapCompatibilityEXT");
    if (table->GetDeviceMicromapCompatibilityEXT == nullptr) {
        table->GetDeviceMicromapCompatibilityEXT = (PFN_vkGetDeviceMicromapCompatibilityEXT)StubGetDeviceMicromapCompatibilityEXT;
    }
    table->GetMicromapBuildSizesEXT = (PFN_vkGetMicromapBuildSizesEXT)gpa(device, "vkGetMicromapBuildSizesEXT");
    if (table->GetMicromapBuildSizesEXT == nullptr) {
        table->GetMicromapBuildSizesEXT = (PFN_vkGetMicromapBuildSizesEXT)StubGetMicromapBuildSizesEXT;
    }
    table->CmdDrawClusterHUAWEI = (PFN_vkCmdDrawClusterHUAWEI)gpa(device, "vkCmdDrawClusterHUAWEI");
    if (table->CmdDrawClusterHUAWEI == nullptr) {
        table->CmdDrawClusterHUAWEI = (PFN_vkCmdDrawClusterHUAWEI)StubCmdDrawClusterHUAWEI;
    }
    table->CmdDrawClusterIndirectHUAWEI = (PFN_vkCmdDrawClusterIndirectHUAWEI)gpa(device, "vkCmdDrawClusterIndirectHUAWEI");
    if (table->CmdDrawClusterIndirectHUAWEI == nullptr) {
        table->CmdDrawClusterIndirectHUAWEI = (PFN_vkCmdDrawClusterIndirectHUAWEI)StubCmdDrawClusterIndirectHUAWEI;
    }
    table->SetDeviceMemoryPriorityEXT = (PFN_vkSetDeviceMemoryPriorityEXT)gpa(device, "vkSetDeviceMemoryPriorityEXT");
    if (table->SetDeviceMemoryPriorityEXT == nullptr) {
        table->SetDeviceMemoryPriorityEXT = (PFN_vkSetDeviceMemoryPriorityEXT)StubSetDeviceMemoryPriorityEXT;
    }
    table->GetDescriptorSetLayoutHostMappingInfoVALVE =
        (PFN_vkGetDescriptorSetLayoutHostMappingInfoVALVE)gpa(device, "vkGetDescriptorSetLayoutHostMappingInfoVALVE");
    if (table->GetDescriptorSetLayoutHostMappingInfoVALVE == nullptr) {
        table->GetDescriptorSetLayoutHostMappingInfoVALVE =
            (PFN_vkGetDescriptorSetLayoutHostMappingInfoVALVE)StubGetDescriptorSetLayoutHostMappingInfoVALVE;
    }
    table->GetDescriptorSetHostMappingVALVE =
        (PFN_vkGetDescriptorSetHostMappingVALVE)gpa(device, "vkGetDescriptorSetHostMappingVALVE");
    if (table->GetDescriptorSetHostMappingVALVE == nullptr) {
        table->GetDescriptorSetHostMappingVALVE = (PFN_vkGetDescriptorSetHostMappingVALVE)StubGetDescriptorSetHostMappingVALVE;
    }
    table->CmdCopyMemoryIndirectNV = (PFN_vkCmdCopyMemoryIndirectNV)gpa(device, "vkCmdCopyMemoryIndirectNV");
    if (table->CmdCopyMemoryIndirectNV == nullptr) {
        table->CmdCopyMemoryIndirectNV = (PFN_vkCmdCopyMemoryIndirectNV)StubCmdCopyMemoryIndirectNV;
    }
    table->CmdCopyMemoryToImageIndirectNV = (PFN_vkCmdCopyMemoryToImageIndirectNV)gpa(device, "vkCmdCopyMemoryToImageIndirectNV");
    if (table->CmdCopyMemoryToImageIndirectNV == nullptr) {
        table->CmdCopyMemoryToImageIndirectNV = (PFN_vkCmdCopyMemoryToImageIndirectNV)StubCmdCopyMemoryToImageIndirectNV;
    }
    table->CmdDecompressMemoryNV = (PFN_vkCmdDecompressMemoryNV)gpa(device, "vkCmdDecompressMemoryNV");
    if (table->CmdDecompressMemoryNV == nullptr) {
        table->CmdDecompressMemoryNV = (PFN_vkCmdDecompressMemoryNV)StubCmdDecompressMemoryNV;
    }
    table->CmdDecompressMemoryIndirectCountNV =
        (PFN_vkCmdDecompressMemoryIndirectCountNV)gpa(device, "vkCmdDecompressMemoryIndirectCountNV");
    if (table->CmdDecompressMemoryIndirectCountNV == nullptr) {
        table->CmdDecompressMemoryIndirectCountNV =
            (PFN_vkCmdDecompressMemoryIndirectCountNV)StubCmdDecompressMemoryIndirectCountNV;
    }
    table->GetPipelineIndirectMemoryRequirementsNV =
        (PFN_vkGetPipelineIndirectMemoryRequirementsNV)gpa(device, "vkGetPipelineIndirectMemoryRequirementsNV");
    if (table->GetPipelineIndirectMemoryRequirementsNV == nullptr) {
        table->GetPipelineIndirectMemoryRequirementsNV =
            (PFN_vkGetPipelineIndirectMemoryRequirementsNV)StubGetPipelineIndirectMemoryRequirementsNV;
    }
    table->CmdUpdatePipelineIndirectBufferNV =
        (PFN_vkCmdUpdatePipelineIndirectBufferNV)gpa(device, "vkCmdUpdatePipelineIndirectBufferNV");
    if (table->CmdUpdatePipelineIndirectBufferNV == nullptr) {
        table->CmdUpdatePipelineIndirectBufferNV = (PFN_vkCmdUpdatePipelineIndirectBufferNV)StubCmdUpdatePipelineIndirectBufferNV;
    }
    table->GetPipelineIndirectDeviceAddressNV =
        (PFN_vkGetPipelineIndirectDeviceAddressNV)gpa(device, "vkGetPipelineIndirectDeviceAddressNV");
    if (table->GetPipelineIndirectDeviceAddressNV == nullptr) {
        table->GetPipelineIndirectDeviceAddressNV =
            (PFN_vkGetPipelineIndirectDeviceAddressNV)StubGetPipelineIndirectDeviceAddressNV;
    }
    table->CmdSetTessellationDomainOriginEXT =
        (PFN_vkCmdSetTessellationDomainOriginEXT)gpa(device, "vkCmdSetTessellationDomainOriginEXT");
    if (table->CmdSetTessellationDomainOriginEXT == nullptr) {
        table->CmdSetTessellationDomainOriginEXT = (PFN_vkCmdSetTessellationDomainOriginEXT)StubCmdSetTessellationDomainOriginEXT;
    }
    table->CmdSetDepthClampEnableEXT = (PFN_vkCmdSetDepthClampEnableEXT)gpa(device, "vkCmdSetDepthClampEnableEXT");
    if (table->CmdSetDepthClampEnableEXT == nullptr) {
        table->CmdSetDepthClampEnableEXT = (PFN_vkCmdSetDepthClampEnableEXT)StubCmdSetDepthClampEnableEXT;
    }
    table->CmdSetPolygonModeEXT = (PFN_vkCmdSetPolygonModeEXT)gpa(device, "vkCmdSetPolygonModeEXT");
    if (table->CmdSetPolygonModeEXT == nullptr) {
        table->CmdSetPolygonModeEXT = (PFN_vkCmdSetPolygonModeEXT)StubCmdSetPolygonModeEXT;
    }
    table->CmdSetRasterizationSamplesEXT = (PFN_vkCmdSetRasterizationSamplesEXT)gpa(device, "vkCmdSetRasterizationSamplesEXT");
    if (table->CmdSetRasterizationSamplesEXT == nullptr) {
        table->CmdSetRasterizationSamplesEXT = (PFN_vkCmdSetRasterizationSamplesEXT)StubCmdSetRasterizationSamplesEXT;
    }
    table->CmdSetSampleMaskEXT = (PFN_vkCmdSetSampleMaskEXT)gpa(device, "vkCmdSetSampleMaskEXT");
    if (table->CmdSetSampleMaskEXT == nullptr) {
        table->CmdSetSampleMaskEXT = (PFN_vkCmdSetSampleMaskEXT)StubCmdSetSampleMaskEXT;
    }
    table->CmdSetAlphaToCoverageEnableEXT = (PFN_vkCmdSetAlphaToCoverageEnableEXT)gpa(device, "vkCmdSetAlphaToCoverageEnableEXT");
    if (table->CmdSetAlphaToCoverageEnableEXT == nullptr) {
        table->CmdSetAlphaToCoverageEnableEXT = (PFN_vkCmdSetAlphaToCoverageEnableEXT)StubCmdSetAlphaToCoverageEnableEXT;
    }
    table->CmdSetAlphaToOneEnableEXT = (PFN_vkCmdSetAlphaToOneEnableEXT)gpa(device, "vkCmdSetAlphaToOneEnableEXT");
    if (table->CmdSetAlphaToOneEnableEXT == nullptr) {
        table->CmdSetAlphaToOneEnableEXT = (PFN_vkCmdSetAlphaToOneEnableEXT)StubCmdSetAlphaToOneEnableEXT;
    }
    table->CmdSetLogicOpEnableEXT = (PFN_vkCmdSetLogicOpEnableEXT)gpa(device, "vkCmdSetLogicOpEnableEXT");
    if (table->CmdSetLogicOpEnableEXT == nullptr) {
        table->CmdSetLogicOpEnableEXT = (PFN_vkCmdSetLogicOpEnableEXT)StubCmdSetLogicOpEnableEXT;
    }
    table->CmdSetColorBlendEnableEXT = (PFN_vkCmdSetColorBlendEnableEXT)gpa(device, "vkCmdSetColorBlendEnableEXT");
    if (table->CmdSetColorBlendEnableEXT == nullptr) {
        table->CmdSetColorBlendEnableEXT = (PFN_vkCmdSetColorBlendEnableEXT)StubCmdSetColorBlendEnableEXT;
    }
    table->CmdSetColorBlendEquationEXT = (PFN_vkCmdSetColorBlendEquationEXT)gpa(device, "vkCmdSetColorBlendEquationEXT");
    if (table->CmdSetColorBlendEquationEXT == nullptr) {
        table->CmdSetColorBlendEquationEXT = (PFN_vkCmdSetColorBlendEquationEXT)StubCmdSetColorBlendEquationEXT;
    }
    table->CmdSetColorWriteMaskEXT = (PFN_vkCmdSetColorWriteMaskEXT)gpa(device, "vkCmdSetColorWriteMaskEXT");
    if (table->CmdSetColorWriteMaskEXT == nullptr) {
        table->CmdSetColorWriteMaskEXT = (PFN_vkCmdSetColorWriteMaskEXT)StubCmdSetColorWriteMaskEXT;
    }
    table->CmdSetRasterizationStreamEXT = (PFN_vkCmdSetRasterizationStreamEXT)gpa(device, "vkCmdSetRasterizationStreamEXT");
    if (table->CmdSetRasterizationStreamEXT == nullptr) {
        table->CmdSetRasterizationStreamEXT = (PFN_vkCmdSetRasterizationStreamEXT)StubCmdSetRasterizationStreamEXT;
    }
    table->CmdSetConservativeRasterizationModeEXT =
        (PFN_vkCmdSetConservativeRasterizationModeEXT)gpa(device, "vkCmdSetConservativeRasterizationModeEXT");
    if (table->CmdSetConservativeRasterizationModeEXT == nullptr) {
        table->CmdSetConservativeRasterizationModeEXT =
            (PFN_vkCmdSetConservativeRasterizationModeEXT)StubCmdSetConservativeRasterizationModeEXT;
    }
    table->CmdSetExtraPrimitiveOverestimationSizeEXT =
        (PFN_vkCmdSetExtraPrimitiveOverestimationSizeEXT)gpa(device, "vkCmdSetExtraPrimitiveOverestimationSizeEXT");
    if (table->CmdSetExtraPrimitiveOverestimationSizeEXT == nullptr) {
        table->CmdSetExtraPrimitiveOverestimationSizeEXT =
            (PFN_vkCmdSetExtraPrimitiveOverestimationSizeEXT)StubCmdSetExtraPrimitiveOverestimationSizeEXT;
    }
    table->CmdSetDepthClipEnableEXT = (PFN_vkCmdSetDepthClipEnableEXT)gpa(device, "vkCmdSetDepthClipEnableEXT");
    if (table->CmdSetDepthClipEnableEXT == nullptr) {
        table->CmdSetDepthClipEnableEXT = (PFN_vkCmdSetDepthClipEnableEXT)StubCmdSetDepthClipEnableEXT;
    }
    table->CmdSetSampleLocationsEnableEXT = (PFN_vkCmdSetSampleLocationsEnableEXT)gpa(device, "vkCmdSetSampleLocationsEnableEXT");
    if (table->CmdSetSampleLocationsEnableEXT == nullptr) {
        table->CmdSetSampleLocationsEnableEXT = (PFN_vkCmdSetSampleLocationsEnableEXT)StubCmdSetSampleLocationsEnableEXT;
    }
    table->CmdSetColorBlendAdvancedEXT = (PFN_vkCmdSetColorBlendAdvancedEXT)gpa(device, "vkCmdSetColorBlendAdvancedEXT");
    if (table->CmdSetColorBlendAdvancedEXT == nullptr) {
        table->CmdSetColorBlendAdvancedEXT = (PFN_vkCmdSetColorBlendAdvancedEXT)StubCmdSetColorBlendAdvancedEXT;
    }
    table->CmdSetProvokingVertexModeEXT = (PFN_vkCmdSetProvokingVertexModeEXT)gpa(device, "vkCmdSetProvokingVertexModeEXT");
    if (table->CmdSetProvokingVertexModeEXT == nullptr) {
        table->CmdSetProvokingVertexModeEXT = (PFN_vkCmdSetProvokingVertexModeEXT)StubCmdSetProvokingVertexModeEXT;
    }
    table->CmdSetLineRasterizationModeEXT = (PFN_vkCmdSetLineRasterizationModeEXT)gpa(device, "vkCmdSetLineRasterizationModeEXT");
    if (table->CmdSetLineRasterizationModeEXT == nullptr) {
        table->CmdSetLineRasterizationModeEXT = (PFN_vkCmdSetLineRasterizationModeEXT)StubCmdSetLineRasterizationModeEXT;
    }
    table->CmdSetLineStippleEnableEXT = (PFN_vkCmdSetLineStippleEnableEXT)gpa(device, "vkCmdSetLineStippleEnableEXT");
    if (table->CmdSetLineStippleEnableEXT == nullptr) {
        table->CmdSetLineStippleEnableEXT = (PFN_vkCmdSetLineStippleEnableEXT)StubCmdSetLineStippleEnableEXT;
    }
    table->CmdSetDepthClipNegativeOneToOneEXT =
        (PFN_vkCmdSetDepthClipNegativeOneToOneEXT)gpa(device, "vkCmdSetDepthClipNegativeOneToOneEXT");
    if (table->CmdSetDepthClipNegativeOneToOneEXT == nullptr) {
        table->CmdSetDepthClipNegativeOneToOneEXT =
            (PFN_vkCmdSetDepthClipNegativeOneToOneEXT)StubCmdSetDepthClipNegativeOneToOneEXT;
    }
    table->CmdSetViewportWScalingEnableNV = (PFN_vkCmdSetViewportWScalingEnableNV)gpa(device, "vkCmdSetViewportWScalingEnableNV");
    if (table->CmdSetViewportWScalingEnableNV == nullptr) {
        table->CmdSetViewportWScalingEnableNV = (PFN_vkCmdSetViewportWScalingEnableNV)StubCmdSetViewportWScalingEnableNV;
    }
    table->CmdSetViewportSwizzleNV = (PFN_vkCmdSetViewportSwizzleNV)gpa(device, "vkCmdSetViewportSwizzleNV");
    if (table->CmdSetViewportSwizzleNV == nullptr) {
        table->CmdSetViewportSwizzleNV = (PFN_vkCmdSetViewportSwizzleNV)StubCmdSetViewportSwizzleNV;
    }
    table->CmdSetCoverageToColorEnableNV = (PFN_vkCmdSetCoverageToColorEnableNV)gpa(device, "vkCmdSetCoverageToColorEnableNV");
    if (table->CmdSetCoverageToColorEnableNV == nullptr) {
        table->CmdSetCoverageToColorEnableNV = (PFN_vkCmdSetCoverageToColorEnableNV)StubCmdSetCoverageToColorEnableNV;
    }
    table->CmdSetCoverageToColorLocationNV =
        (PFN_vkCmdSetCoverageToColorLocationNV)gpa(device, "vkCmdSetCoverageToColorLocationNV");
    if (table->CmdSetCoverageToColorLocationNV == nullptr) {
        table->CmdSetCoverageToColorLocationNV = (PFN_vkCmdSetCoverageToColorLocationNV)StubCmdSetCoverageToColorLocationNV;
    }
    table->CmdSetCoverageModulationModeNV = (PFN_vkCmdSetCoverageModulationModeNV)gpa(device, "vkCmdSetCoverageModulationModeNV");
    if (table->CmdSetCoverageModulationModeNV == nullptr) {
        table->CmdSetCoverageModulationModeNV = (PFN_vkCmdSetCoverageModulationModeNV)StubCmdSetCoverageModulationModeNV;
    }
    table->CmdSetCoverageModulationTableEnableNV =
        (PFN_vkCmdSetCoverageModulationTableEnableNV)gpa(device, "vkCmdSetCoverageModulationTableEnableNV");
    if (table->CmdSetCoverageModulationTableEnableNV == nullptr) {
        table->CmdSetCoverageModulationTableEnableNV =
            (PFN_vkCmdSetCoverageModulationTableEnableNV)StubCmdSetCoverageModulationTableEnableNV;
    }
    table->CmdSetCoverageModulationTableNV =
        (PFN_vkCmdSetCoverageModulationTableNV)gpa(device, "vkCmdSetCoverageModulationTableNV");
    if (table->CmdSetCoverageModulationTableNV == nullptr) {
        table->CmdSetCoverageModulationTableNV = (PFN_vkCmdSetCoverageModulationTableNV)StubCmdSetCoverageModulationTableNV;
    }
    table->CmdSetShadingRateImageEnableNV = (PFN_vkCmdSetShadingRateImageEnableNV)gpa(device, "vkCmdSetShadingRateImageEnableNV");
    if (table->CmdSetShadingRateImageEnableNV == nullptr) {
        table->CmdSetShadingRateImageEnableNV = (PFN_vkCmdSetShadingRateImageEnableNV)StubCmdSetShadingRateImageEnableNV;
    }
    table->CmdSetRepresentativeFragmentTestEnableNV =
        (PFN_vkCmdSetRepresentativeFragmentTestEnableNV)gpa(device, "vkCmdSetRepresentativeFragmentTestEnableNV");
    if (table->CmdSetRepresentativeFragmentTestEnableNV == nullptr) {
        table->CmdSetRepresentativeFragmentTestEnableNV =
            (PFN_vkCmdSetRepresentativeFragmentTestEnableNV)StubCmdSetRepresentativeFragmentTestEnableNV;
    }
    table->CmdSetCoverageReductionModeNV = (PFN_vkCmdSetCoverageReductionModeNV)gpa(device, "vkCmdSetCoverageReductionModeNV");
    if (table->CmdSetCoverageReductionModeNV == nullptr) {
        table->CmdSetCoverageReductionModeNV = (PFN_vkCmdSetCoverageReductionModeNV)StubCmdSetCoverageReductionModeNV;
    }
    table->GetShaderModuleIdentifierEXT = (PFN_vkGetShaderModuleIdentifierEXT)gpa(device, "vkGetShaderModuleIdentifierEXT");
    if (table->GetShaderModuleIdentifierEXT == nullptr) {
        table->GetShaderModuleIdentifierEXT = (PFN_vkGetShaderModuleIdentifierEXT)StubGetShaderModuleIdentifierEXT;
    }
    table->GetShaderModuleCreateInfoIdentifierEXT =
        (PFN_vkGetShaderModuleCreateInfoIdentifierEXT)gpa(device, "vkGetShaderModuleCreateInfoIdentifierEXT");
    if (table->GetShaderModuleCreateInfoIdentifierEXT == nullptr) {
        table->GetShaderModuleCreateInfoIdentifierEXT =
            (PFN_vkGetShaderModuleCreateInfoIdentifierEXT)StubGetShaderModuleCreateInfoIdentifierEXT;
    }
    table->CreateOpticalFlowSessionNV = (PFN_vkCreateOpticalFlowSessionNV)gpa(device, "vkCreateOpticalFlowSessionNV");
    if (table->CreateOpticalFlowSessionNV == nullptr) {
        table->CreateOpticalFlowSessionNV = (PFN_vkCreateOpticalFlowSessionNV)StubCreateOpticalFlowSessionNV;
    }
    table->DestroyOpticalFlowSessionNV = (PFN_vkDestroyOpticalFlowSessionNV)gpa(device, "vkDestroyOpticalFlowSessionNV");
    if (table->DestroyOpticalFlowSessionNV == nullptr) {
        table->DestroyOpticalFlowSessionNV = (PFN_vkDestroyOpticalFlowSessionNV)StubDestroyOpticalFlowSessionNV;
    }
    table->BindOpticalFlowSessionImageNV = (PFN_vkBindOpticalFlowSessionImageNV)gpa(device, "vkBindOpticalFlowSessionImageNV");
    if (table->BindOpticalFlowSessionImageNV == nullptr) {
        table->BindOpticalFlowSessionImageNV = (PFN_vkBindOpticalFlowSessionImageNV)StubBindOpticalFlowSessionImageNV;
    }
    table->CmdOpticalFlowExecuteNV = (PFN_vkCmdOpticalFlowExecuteNV)gpa(device, "vkCmdOpticalFlowExecuteNV");
    if (table->CmdOpticalFlowExecuteNV == nullptr) {
        table->CmdOpticalFlowExecuteNV = (PFN_vkCmdOpticalFlowExecuteNV)StubCmdOpticalFlowExecuteNV;
    }
    table->CreateShadersEXT = (PFN_vkCreateShadersEXT)gpa(device, "vkCreateShadersEXT");
    if (table->CreateShadersEXT == nullptr) {
        table->CreateShadersEXT = (PFN_vkCreateShadersEXT)StubCreateShadersEXT;
    }
    table->DestroyShaderEXT = (PFN_vkDestroyShaderEXT)gpa(device, "vkDestroyShaderEXT");
    if (table->DestroyShaderEXT == nullptr) {
        table->DestroyShaderEXT = (PFN_vkDestroyShaderEXT)StubDestroyShaderEXT;
    }
    table->GetShaderBinaryDataEXT = (PFN_vkGetShaderBinaryDataEXT)gpa(device, "vkGetShaderBinaryDataEXT");
    if (table->GetShaderBinaryDataEXT == nullptr) {
        table->GetShaderBinaryDataEXT = (PFN_vkGetShaderBinaryDataEXT)StubGetShaderBinaryDataEXT;
    }
    table->CmdBindShadersEXT = (PFN_vkCmdBindShadersEXT)gpa(device, "vkCmdBindShadersEXT");
    if (table->CmdBindShadersEXT == nullptr) {
        table->CmdBindShadersEXT = (PFN_vkCmdBindShadersEXT)StubCmdBindShadersEXT;
    }
    table->GetFramebufferTilePropertiesQCOM =
        (PFN_vkGetFramebufferTilePropertiesQCOM)gpa(device, "vkGetFramebufferTilePropertiesQCOM");
    if (table->GetFramebufferTilePropertiesQCOM == nullptr) {
        table->GetFramebufferTilePropertiesQCOM = (PFN_vkGetFramebufferTilePropertiesQCOM)StubGetFramebufferTilePropertiesQCOM;
    }
    table->GetDynamicRenderingTilePropertiesQCOM =
        (PFN_vkGetDynamicRenderingTilePropertiesQCOM)gpa(device, "vkGetDynamicRenderingTilePropertiesQCOM");
    if (table->GetDynamicRenderingTilePropertiesQCOM == nullptr) {
        table->GetDynamicRenderingTilePropertiesQCOM =
            (PFN_vkGetDynamicRenderingTilePropertiesQCOM)StubGetDynamicRenderingTilePropertiesQCOM;
    }
    table->SetLatencySleepModeNV = (PFN_vkSetLatencySleepModeNV)gpa(device, "vkSetLatencySleepModeNV");
    if (table->SetLatencySleepModeNV == nullptr) {
        table->SetLatencySleepModeNV = (PFN_vkSetLatencySleepModeNV)StubSetLatencySleepModeNV;
    }
    table->LatencySleepNV = (PFN_vkLatencySleepNV)gpa(device, "vkLatencySleepNV");
    if (table->LatencySleepNV == nullptr) {
        table->LatencySleepNV = (PFN_vkLatencySleepNV)StubLatencySleepNV;
    }
    table->SetLatencyMarkerNV = (PFN_vkSetLatencyMarkerNV)gpa(device, "vkSetLatencyMarkerNV");
    if (table->SetLatencyMarkerNV == nullptr) {
        table->SetLatencyMarkerNV = (PFN_vkSetLatencyMarkerNV)StubSetLatencyMarkerNV;
    }
    table->GetLatencyTimingsNV = (PFN_vkGetLatencyTimingsNV)gpa(device, "vkGetLatencyTimingsNV");
    if (table->GetLatencyTimingsNV == nullptr) {
        table->GetLatencyTimingsNV = (PFN_vkGetLatencyTimingsNV)StubGetLatencyTimingsNV;
    }
    table->QueueNotifyOutOfBandNV = (PFN_vkQueueNotifyOutOfBandNV)gpa(device, "vkQueueNotifyOutOfBandNV");
    if (table->QueueNotifyOutOfBandNV == nullptr) {
        table->QueueNotifyOutOfBandNV = (PFN_vkQueueNotifyOutOfBandNV)StubQueueNotifyOutOfBandNV;
    }
    table->CmdSetAttachmentFeedbackLoopEnableEXT =
        (PFN_vkCmdSetAttachmentFeedbackLoopEnableEXT)gpa(device, "vkCmdSetAttachmentFeedbackLoopEnableEXT");
    if (table->CmdSetAttachmentFeedbackLoopEnableEXT == nullptr) {
        table->CmdSetAttachmentFeedbackLoopEnableEXT =
            (PFN_vkCmdSetAttachmentFeedbackLoopEnableEXT)StubCmdSetAttachmentFeedbackLoopEnableEXT;
    }
#ifdef VK_USE_PLATFORM_SCREEN_QNX
    table->GetScreenBufferPropertiesQNX = (PFN_vkGetScreenBufferPropertiesQNX)gpa(device, "vkGetScreenBufferPropertiesQNX");
    if (table->GetScreenBufferPropertiesQNX == nullptr) {
        table->GetScreenBufferPropertiesQNX = (PFN_vkGetScreenBufferPropertiesQNX)StubGetScreenBufferPropertiesQNX;
    }
#endif  // VK_USE_PLATFORM_SCREEN_QNX
    table->CreateAccelerationStructureKHR = (PFN_vkCreateAccelerationStructureKHR)gpa(device, "vkCreateAccelerationStructureKHR");
    if (table->CreateAccelerationStructureKHR == nullptr) {
        table->CreateAccelerationStructureKHR = (PFN_vkCreateAccelerationStructureKHR)StubCreateAccelerationStructureKHR;
    }
    table->DestroyAccelerationStructureKHR =
        (PFN_vkDestroyAccelerationStructureKHR)gpa(device, "vkDestroyAccelerationStructureKHR");
    if (table->DestroyAccelerationStructureKHR == nullptr) {
        table->DestroyAccelerationStructureKHR = (PFN_vkDestroyAccelerationStructureKHR)StubDestroyAccelerationStructureKHR;
    }
    table->CmdBuildAccelerationStructuresKHR =
        (PFN_vkCmdBuildAccelerationStructuresKHR)gpa(device, "vkCmdBuildAccelerationStructuresKHR");
    if (table->CmdBuildAccelerationStructuresKHR == nullptr) {
        table->CmdBuildAccelerationStructuresKHR = (PFN_vkCmdBuildAccelerationStructuresKHR)StubCmdBuildAccelerationStructuresKHR;
    }
    table->CmdBuildAccelerationStructuresIndirectKHR =
        (PFN_vkCmdBuildAccelerationStructuresIndirectKHR)gpa(device, "vkCmdBuildAccelerationStructuresIndirectKHR");
    if (table->CmdBuildAccelerationStructuresIndirectKHR == nullptr) {
        table->CmdBuildAccelerationStructuresIndirectKHR =
            (PFN_vkCmdBuildAccelerationStructuresIndirectKHR)StubCmdBuildAccelerationStructuresIndirectKHR;
    }
    table->BuildAccelerationStructuresKHR = (PFN_vkBuildAccelerationStructuresKHR)gpa(device, "vkBuildAccelerationStructuresKHR");
    if (table->BuildAccelerationStructuresKHR == nullptr) {
        table->BuildAccelerationStructuresKHR = (PFN_vkBuildAccelerationStructuresKHR)StubBuildAccelerationStructuresKHR;
    }
    table->CopyAccelerationStructureKHR = (PFN_vkCopyAccelerationStructureKHR)gpa(device, "vkCopyAccelerationStructureKHR");
    if (table->CopyAccelerationStructureKHR == nullptr) {
        table->CopyAccelerationStructureKHR = (PFN_vkCopyAccelerationStructureKHR)StubCopyAccelerationStructureKHR;
    }
    table->CopyAccelerationStructureToMemoryKHR =
        (PFN_vkCopyAccelerationStructureToMemoryKHR)gpa(device, "vkCopyAccelerationStructureToMemoryKHR");
    if (table->CopyAccelerationStructureToMemoryKHR == nullptr) {
        table->CopyAccelerationStructureToMemoryKHR =
            (PFN_vkCopyAccelerationStructureToMemoryKHR)StubCopyAccelerationStructureToMemoryKHR;
    }
    table->CopyMemoryToAccelerationStructureKHR =
        (PFN_vkCopyMemoryToAccelerationStructureKHR)gpa(device, "vkCopyMemoryToAccelerationStructureKHR");
    if (table->CopyMemoryToAccelerationStructureKHR == nullptr) {
        table->CopyMemoryToAccelerationStructureKHR =
            (PFN_vkCopyMemoryToAccelerationStructureKHR)StubCopyMemoryToAccelerationStructureKHR;
    }
    table->WriteAccelerationStructuresPropertiesKHR =
        (PFN_vkWriteAccelerationStructuresPropertiesKHR)gpa(device, "vkWriteAccelerationStructuresPropertiesKHR");
    if (table->WriteAccelerationStructuresPropertiesKHR == nullptr) {
        table->WriteAccelerationStructuresPropertiesKHR =
            (PFN_vkWriteAccelerationStructuresPropertiesKHR)StubWriteAccelerationStructuresPropertiesKHR;
    }
    table->CmdCopyAccelerationStructureKHR =
        (PFN_vkCmdCopyAccelerationStructureKHR)gpa(device, "vkCmdCopyAccelerationStructureKHR");
    if (table->CmdCopyAccelerationStructureKHR == nullptr) {
        table->CmdCopyAccelerationStructureKHR = (PFN_vkCmdCopyAccelerationStructureKHR)StubCmdCopyAccelerationStructureKHR;
    }
    table->CmdCopyAccelerationStructureToMemoryKHR =
        (PFN_vkCmdCopyAccelerationStructureToMemoryKHR)gpa(device, "vkCmdCopyAccelerationStructureToMemoryKHR");
    if (table->CmdCopyAccelerationStructureToMemoryKHR == nullptr) {
        table->CmdCopyAccelerationStructureToMemoryKHR =
            (PFN_vkCmdCopyAccelerationStructureToMemoryKHR)StubCmdCopyAccelerationStructureToMemoryKHR;
    }
    table->CmdCopyMemoryToAccelerationStructureKHR =
        (PFN_vkCmdCopyMemoryToAccelerationStructureKHR)gpa(device, "vkCmdCopyMemoryToAccelerationStructureKHR");
    if (table->CmdCopyMemoryToAccelerationStructureKHR == nullptr) {
        table->CmdCopyMemoryToAccelerationStructureKHR =
            (PFN_vkCmdCopyMemoryToAccelerationStructureKHR)StubCmdCopyMemoryToAccelerationStructureKHR;
    }
    table->GetAccelerationStructureDeviceAddressKHR =
        (PFN_vkGetAccelerationStructureDeviceAddressKHR)gpa(device, "vkGetAccelerationStructureDeviceAddressKHR");
    if (table->GetAccelerationStructureDeviceAddressKHR == nullptr) {
        table->GetAccelerationStructureDeviceAddressKHR =
            (PFN_vkGetAccelerationStructureDeviceAddressKHR)StubGetAccelerationStructureDeviceAddressKHR;
    }
    table->CmdWriteAccelerationStructuresPropertiesKHR =
        (PFN_vkCmdWriteAccelerationStructuresPropertiesKHR)gpa(device, "vkCmdWriteAccelerationStructuresPropertiesKHR");
    if (table->CmdWriteAccelerationStructuresPropertiesKHR == nullptr) {
        table->CmdWriteAccelerationStructuresPropertiesKHR =
            (PFN_vkCmdWriteAccelerationStructuresPropertiesKHR)StubCmdWriteAccelerationStructuresPropertiesKHR;
    }
    table->GetDeviceAccelerationStructureCompatibilityKHR =
        (PFN_vkGetDeviceAccelerationStructureCompatibilityKHR)gpa(device, "vkGetDeviceAccelerationStructureCompatibilityKHR");
    if (table->GetDeviceAccelerationStructureCompatibilityKHR == nullptr) {
        table->GetDeviceAccelerationStructureCompatibilityKHR =
            (PFN_vkGetDeviceAccelerationStructureCompatibilityKHR)StubGetDeviceAccelerationStructureCompatibilityKHR;
    }
    table->GetAccelerationStructureBuildSizesKHR =
        (PFN_vkGetAccelerationStructureBuildSizesKHR)gpa(device, "vkGetAccelerationStructureBuildSizesKHR");
    if (table->GetAccelerationStructureBuildSizesKHR == nullptr) {
        table->GetAccelerationStructureBuildSizesKHR =
            (PFN_vkGetAccelerationStructureBuildSizesKHR)StubGetAccelerationStructureBuildSizesKHR;
    }
    table->CmdTraceRaysKHR = (PFN_vkCmdTraceRaysKHR)gpa(device, "vkCmdTraceRaysKHR");
    if (table->CmdTraceRaysKHR == nullptr) {
        table->CmdTraceRaysKHR = (PFN_vkCmdTraceRaysKHR)StubCmdTraceRaysKHR;
    }
    table->CreateRayTracingPipelinesKHR = (PFN_vkCreateRayTracingPipelinesKHR)gpa(device, "vkCreateRayTracingPipelinesKHR");
    if (table->CreateRayTracingPipelinesKHR == nullptr) {
        table->CreateRayTracingPipelinesKHR = (PFN_vkCreateRayTracingPipelinesKHR)StubCreateRayTracingPipelinesKHR;
    }
    table->GetRayTracingCaptureReplayShaderGroupHandlesKHR =
        (PFN_vkGetRayTracingCaptureReplayShaderGroupHandlesKHR)gpa(device, "vkGetRayTracingCaptureReplayShaderGroupHandlesKHR");
    if (table->GetRayTracingCaptureReplayShaderGroupHandlesKHR == nullptr) {
        table->GetRayTracingCaptureReplayShaderGroupHandlesKHR =
            (PFN_vkGetRayTracingCaptureReplayShaderGroupHandlesKHR)StubGetRayTracingCaptureReplayShaderGroupHandlesKHR;
    }
    table->CmdTraceRaysIndirectKHR = (PFN_vkCmdTraceRaysIndirectKHR)gpa(device, "vkCmdTraceRaysIndirectKHR");
    if (table->CmdTraceRaysIndirectKHR == nullptr) {
        table->CmdTraceRaysIndirectKHR = (PFN_vkCmdTraceRaysIndirectKHR)StubCmdTraceRaysIndirectKHR;
    }
    table->GetRayTracingShaderGroupStackSizeKHR =
        (PFN_vkGetRayTracingShaderGroupStackSizeKHR)gpa(device, "vkGetRayTracingShaderGroupStackSizeKHR");
    if (table->GetRayTracingShaderGroupStackSizeKHR == nullptr) {
        table->GetRayTracingShaderGroupStackSizeKHR =
            (PFN_vkGetRayTracingShaderGroupStackSizeKHR)StubGetRayTracingShaderGroupStackSizeKHR;
    }
    table->CmdSetRayTracingPipelineStackSizeKHR =
        (PFN_vkCmdSetRayTracingPipelineStackSizeKHR)gpa(device, "vkCmdSetRayTracingPipelineStackSizeKHR");
    if (table->CmdSetRayTracingPipelineStackSizeKHR == nullptr) {
        table->CmdSetRayTracingPipelineStackSizeKHR =
            (PFN_vkCmdSetRayTracingPipelineStackSizeKHR)StubCmdSetRayTracingPipelineStackSizeKHR;
    }
    table->CmdDrawMeshTasksEXT = (PFN_vkCmdDrawMeshTasksEXT)gpa(device, "vkCmdDrawMeshTasksEXT");
    if (table->CmdDrawMeshTasksEXT == nullptr) {
        table->CmdDrawMeshTasksEXT = (PFN_vkCmdDrawMeshTasksEXT)StubCmdDrawMeshTasksEXT;
    }
    table->CmdDrawMeshTasksIndirectEXT = (PFN_vkCmdDrawMeshTasksIndirectEXT)gpa(device, "vkCmdDrawMeshTasksIndirectEXT");
    if (table->CmdDrawMeshTasksIndirectEXT == nullptr) {
        table->CmdDrawMeshTasksIndirectEXT = (PFN_vkCmdDrawMeshTasksIndirectEXT)StubCmdDrawMeshTasksIndirectEXT;
    }
    table->CmdDrawMeshTasksIndirectCountEXT =
        (PFN_vkCmdDrawMeshTasksIndirectCountEXT)gpa(device, "vkCmdDrawMeshTasksIndirectCountEXT");
    if (table->CmdDrawMeshTasksIndirectCountEXT == nullptr) {
        table->CmdDrawMeshTasksIndirectCountEXT = (PFN_vkCmdDrawMeshTasksIndirectCountEXT)StubCmdDrawMeshTasksIndirectCountEXT;
    }
}

static inline void layer_init_instance_dispatch_table(VkInstance instance, VkLayerInstanceDispatchTable* table,
                                                      PFN_vkGetInstanceProcAddr gpa) {
    memset(table, 0, sizeof(*table));
    // Instance function pointers
    table->GetInstanceProcAddr = gpa;
    table->GetPhysicalDeviceProcAddr = (PFN_GetPhysicalDeviceProcAddr)gpa(instance, "vk_layerGetPhysicalDeviceProcAddr");
    table->DestroyInstance = (PFN_vkDestroyInstance)gpa(instance, "vkDestroyInstance");
    table->EnumeratePhysicalDevices = (PFN_vkEnumeratePhysicalDevices)gpa(instance, "vkEnumeratePhysicalDevices");
    table->GetPhysicalDeviceFeatures = (PFN_vkGetPhysicalDeviceFeatures)gpa(instance, "vkGetPhysicalDeviceFeatures");
    table->GetPhysicalDeviceFormatProperties =
        (PFN_vkGetPhysicalDeviceFormatProperties)gpa(instance, "vkGetPhysicalDeviceFormatProperties");
    table->GetPhysicalDeviceImageFormatProperties =
        (PFN_vkGetPhysicalDeviceImageFormatProperties)gpa(instance, "vkGetPhysicalDeviceImageFormatProperties");
    table->GetPhysicalDeviceProperties = (PFN_vkGetPhysicalDeviceProperties)gpa(instance, "vkGetPhysicalDeviceProperties");
    table->GetPhysicalDeviceQueueFamilyProperties =
        (PFN_vkGetPhysicalDeviceQueueFamilyProperties)gpa(instance, "vkGetPhysicalDeviceQueueFamilyProperties");
    table->GetPhysicalDeviceMemoryProperties =
        (PFN_vkGetPhysicalDeviceMemoryProperties)gpa(instance, "vkGetPhysicalDeviceMemoryProperties");
    table->EnumerateDeviceExtensionProperties =
        (PFN_vkEnumerateDeviceExtensionProperties)gpa(instance, "vkEnumerateDeviceExtensionProperties");
    table->EnumerateDeviceLayerProperties = (PFN_vkEnumerateDeviceLayerProperties)gpa(instance, "vkEnumerateDeviceLayerProperties");
    table->GetPhysicalDeviceSparseImageFormatProperties =
        (PFN_vkGetPhysicalDeviceSparseImageFormatProperties)gpa(instance, "vkGetPhysicalDeviceSparseImageFormatProperties");
    table->EnumeratePhysicalDeviceGroups = (PFN_vkEnumeratePhysicalDeviceGroups)gpa(instance, "vkEnumeratePhysicalDeviceGroups");
    if (table->EnumeratePhysicalDeviceGroups == nullptr) {
        table->EnumeratePhysicalDeviceGroups = (PFN_vkEnumeratePhysicalDeviceGroups)StubEnumeratePhysicalDeviceGroups;
    }
    table->GetPhysicalDeviceFeatures2 = (PFN_vkGetPhysicalDeviceFeatures2)gpa(instance, "vkGetPhysicalDeviceFeatures2");
    if (table->GetPhysicalDeviceFeatures2 == nullptr) {
        table->GetPhysicalDeviceFeatures2 = (PFN_vkGetPhysicalDeviceFeatures2)StubGetPhysicalDeviceFeatures2;
    }
    table->GetPhysicalDeviceProperties2 = (PFN_vkGetPhysicalDeviceProperties2)gpa(instance, "vkGetPhysicalDeviceProperties2");
    if (table->GetPhysicalDeviceProperties2 == nullptr) {
        table->GetPhysicalDeviceProperties2 = (PFN_vkGetPhysicalDeviceProperties2)StubGetPhysicalDeviceProperties2;
    }
    table->GetPhysicalDeviceFormatProperties2 =
        (PFN_vkGetPhysicalDeviceFormatProperties2)gpa(instance, "vkGetPhysicalDeviceFormatProperties2");
    if (table->GetPhysicalDeviceFormatProperties2 == nullptr) {
        table->GetPhysicalDeviceFormatProperties2 =
            (PFN_vkGetPhysicalDeviceFormatProperties2)StubGetPhysicalDeviceFormatProperties2;
    }
    table->GetPhysicalDeviceImageFormatProperties2 =
        (PFN_vkGetPhysicalDeviceImageFormatProperties2)gpa(instance, "vkGetPhysicalDeviceImageFormatProperties2");
    if (table->GetPhysicalDeviceImageFormatProperties2 == nullptr) {
        table->GetPhysicalDeviceImageFormatProperties2 =
            (PFN_vkGetPhysicalDeviceImageFormatProperties2)StubGetPhysicalDeviceImageFormatProperties2;
    }
    table->GetPhysicalDeviceQueueFamilyProperties2 =
        (PFN_vkGetPhysicalDeviceQueueFamilyProperties2)gpa(instance, "vkGetPhysicalDeviceQueueFamilyProperties2");
    if (table->GetPhysicalDeviceQueueFamilyProperties2 == nullptr) {
        table->GetPhysicalDeviceQueueFamilyProperties2 =
            (PFN_vkGetPhysicalDeviceQueueFamilyProperties2)StubGetPhysicalDeviceQueueFamilyProperties2;
    }
    table->GetPhysicalDeviceMemoryProperties2 =
        (PFN_vkGetPhysicalDeviceMemoryProperties2)gpa(instance, "vkGetPhysicalDeviceMemoryProperties2");
    if (table->GetPhysicalDeviceMemoryProperties2 == nullptr) {
        table->GetPhysicalDeviceMemoryProperties2 =
            (PFN_vkGetPhysicalDeviceMemoryProperties2)StubGetPhysicalDeviceMemoryProperties2;
    }
    table->GetPhysicalDeviceSparseImageFormatProperties2 =
        (PFN_vkGetPhysicalDeviceSparseImageFormatProperties2)gpa(instance, "vkGetPhysicalDeviceSparseImageFormatProperties2");
    if (table->GetPhysicalDeviceSparseImageFormatProperties2 == nullptr) {
        table->GetPhysicalDeviceSparseImageFormatProperties2 =
            (PFN_vkGetPhysicalDeviceSparseImageFormatProperties2)StubGetPhysicalDeviceSparseImageFormatProperties2;
    }
    table->GetPhysicalDeviceExternalBufferProperties =
        (PFN_vkGetPhysicalDeviceExternalBufferProperties)gpa(instance, "vkGetPhysicalDeviceExternalBufferProperties");
    if (table->GetPhysicalDeviceExternalBufferProperties == nullptr) {
        table->GetPhysicalDeviceExternalBufferProperties =
            (PFN_vkGetPhysicalDeviceExternalBufferProperties)StubGetPhysicalDeviceExternalBufferProperties;
    }
    table->GetPhysicalDeviceExternalFenceProperties =
        (PFN_vkGetPhysicalDeviceExternalFenceProperties)gpa(instance, "vkGetPhysicalDeviceExternalFenceProperties");
    if (table->GetPhysicalDeviceExternalFenceProperties == nullptr) {
        table->GetPhysicalDeviceExternalFenceProperties =
            (PFN_vkGetPhysicalDeviceExternalFenceProperties)StubGetPhysicalDeviceExternalFenceProperties;
    }
    table->GetPhysicalDeviceExternalSemaphoreProperties =
        (PFN_vkGetPhysicalDeviceExternalSemaphoreProperties)gpa(instance, "vkGetPhysicalDeviceExternalSemaphoreProperties");
    if (table->GetPhysicalDeviceExternalSemaphoreProperties == nullptr) {
        table->GetPhysicalDeviceExternalSemaphoreProperties =
            (PFN_vkGetPhysicalDeviceExternalSemaphoreProperties)StubGetPhysicalDeviceExternalSemaphoreProperties;
    }
    table->GetPhysicalDeviceToolProperties =
        (PFN_vkGetPhysicalDeviceToolProperties)gpa(instance, "vkGetPhysicalDeviceToolProperties");
    if (table->GetPhysicalDeviceToolProperties == nullptr) {
        table->GetPhysicalDeviceToolProperties = (PFN_vkGetPhysicalDeviceToolProperties)StubGetPhysicalDeviceToolProperties;
    }
    table->DestroySurfaceKHR = (PFN_vkDestroySurfaceKHR)gpa(instance, "vkDestroySurfaceKHR");
    if (table->DestroySurfaceKHR == nullptr) {
        table->DestroySurfaceKHR = (PFN_vkDestroySurfaceKHR)StubDestroySurfaceKHR;
    }
    table->GetPhysicalDeviceSurfaceSupportKHR =
        (PFN_vkGetPhysicalDeviceSurfaceSupportKHR)gpa(instance, "vkGetPhysicalDeviceSurfaceSupportKHR");
    if (table->GetPhysicalDeviceSurfaceSupportKHR == nullptr) {
        table->GetPhysicalDeviceSurfaceSupportKHR =
            (PFN_vkGetPhysicalDeviceSurfaceSupportKHR)StubGetPhysicalDeviceSurfaceSupportKHR;
    }
    table->GetPhysicalDeviceSurfaceCapabilitiesKHR =
        (PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR)gpa(instance, "vkGetPhysicalDeviceSurfaceCapabilitiesKHR");
    if (table->GetPhysicalDeviceSurfaceCapabilitiesKHR == nullptr) {
        table->GetPhysicalDeviceSurfaceCapabilitiesKHR =
            (PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR)StubGetPhysicalDeviceSurfaceCapabilitiesKHR;
    }
    table->GetPhysicalDeviceSurfaceFormatsKHR =
        (PFN_vkGetPhysicalDeviceSurfaceFormatsKHR)gpa(instance, "vkGetPhysicalDeviceSurfaceFormatsKHR");
    if (table->GetPhysicalDeviceSurfaceFormatsKHR == nullptr) {
        table->GetPhysicalDeviceSurfaceFormatsKHR =
            (PFN_vkGetPhysicalDeviceSurfaceFormatsKHR)StubGetPhysicalDeviceSurfaceFormatsKHR;
    }
    table->GetPhysicalDeviceSurfacePresentModesKHR =
        (PFN_vkGetPhysicalDeviceSurfacePresentModesKHR)gpa(instance, "vkGetPhysicalDeviceSurfacePresentModesKHR");
    if (table->GetPhysicalDeviceSurfacePresentModesKHR == nullptr) {
        table->GetPhysicalDeviceSurfacePresentModesKHR =
            (PFN_vkGetPhysicalDeviceSurfacePresentModesKHR)StubGetPhysicalDeviceSurfacePresentModesKHR;
    }
    table->GetPhysicalDevicePresentRectanglesKHR =
        (PFN_vkGetPhysicalDevicePresentRectanglesKHR)gpa(instance, "vkGetPhysicalDevicePresentRectanglesKHR");
    if (table->GetPhysicalDevicePresentRectanglesKHR == nullptr) {
        table->GetPhysicalDevicePresentRectanglesKHR =
            (PFN_vkGetPhysicalDevicePresentRectanglesKHR)StubGetPhysicalDevicePresentRectanglesKHR;
    }
    table->GetPhysicalDeviceDisplayPropertiesKHR =
        (PFN_vkGetPhysicalDeviceDisplayPropertiesKHR)gpa(instance, "vkGetPhysicalDeviceDisplayPropertiesKHR");
    if (table->GetPhysicalDeviceDisplayPropertiesKHR == nullptr) {
        table->GetPhysicalDeviceDisplayPropertiesKHR =
            (PFN_vkGetPhysicalDeviceDisplayPropertiesKHR)StubGetPhysicalDeviceDisplayPropertiesKHR;
    }
    table->GetPhysicalDeviceDisplayPlanePropertiesKHR =
        (PFN_vkGetPhysicalDeviceDisplayPlanePropertiesKHR)gpa(instance, "vkGetPhysicalDeviceDisplayPlanePropertiesKHR");
    if (table->GetPhysicalDeviceDisplayPlanePropertiesKHR == nullptr) {
        table->GetPhysicalDeviceDisplayPlanePropertiesKHR =
            (PFN_vkGetPhysicalDeviceDisplayPlanePropertiesKHR)StubGetPhysicalDeviceDisplayPlanePropertiesKHR;
    }
    table->GetDisplayPlaneSupportedDisplaysKHR =
        (PFN_vkGetDisplayPlaneSupportedDisplaysKHR)gpa(instance, "vkGetDisplayPlaneSupportedDisplaysKHR");
    if (table->GetDisplayPlaneSupportedDisplaysKHR == nullptr) {
        table->GetDisplayPlaneSupportedDisplaysKHR =
            (PFN_vkGetDisplayPlaneSupportedDisplaysKHR)StubGetDisplayPlaneSupportedDisplaysKHR;
    }
    table->GetDisplayModePropertiesKHR = (PFN_vkGetDisplayModePropertiesKHR)gpa(instance, "vkGetDisplayModePropertiesKHR");
    if (table->GetDisplayModePropertiesKHR == nullptr) {
        table->GetDisplayModePropertiesKHR = (PFN_vkGetDisplayModePropertiesKHR)StubGetDisplayModePropertiesKHR;
    }
    table->CreateDisplayModeKHR = (PFN_vkCreateDisplayModeKHR)gpa(instance, "vkCreateDisplayModeKHR");
    if (table->CreateDisplayModeKHR == nullptr) {
        table->CreateDisplayModeKHR = (PFN_vkCreateDisplayModeKHR)StubCreateDisplayModeKHR;
    }
    table->GetDisplayPlaneCapabilitiesKHR = (PFN_vkGetDisplayPlaneCapabilitiesKHR)gpa(instance, "vkGetDisplayPlaneCapabilitiesKHR");
    if (table->GetDisplayPlaneCapabilitiesKHR == nullptr) {
        table->GetDisplayPlaneCapabilitiesKHR = (PFN_vkGetDisplayPlaneCapabilitiesKHR)StubGetDisplayPlaneCapabilitiesKHR;
    }
    table->CreateDisplayPlaneSurfaceKHR = (PFN_vkCreateDisplayPlaneSurfaceKHR)gpa(instance, "vkCreateDisplayPlaneSurfaceKHR");
    if (table->CreateDisplayPlaneSurfaceKHR == nullptr) {
        table->CreateDisplayPlaneSurfaceKHR = (PFN_vkCreateDisplayPlaneSurfaceKHR)StubCreateDisplayPlaneSurfaceKHR;
    }
#ifdef VK_USE_PLATFORM_XLIB_KHR
    table->CreateXlibSurfaceKHR = (PFN_vkCreateXlibSurfaceKHR)gpa(instance, "vkCreateXlibSurfaceKHR");
    if (table->CreateXlibSurfaceKHR == nullptr) {
        table->CreateXlibSurfaceKHR = (PFN_vkCreateXlibSurfaceKHR)StubCreateXlibSurfaceKHR;
    }
#endif  // VK_USE_PLATFORM_XLIB_KHR
#ifdef VK_USE_PLATFORM_XLIB_KHR
    table->GetPhysicalDeviceXlibPresentationSupportKHR =
        (PFN_vkGetPhysicalDeviceXlibPresentationSupportKHR)gpa(instance, "vkGetPhysicalDeviceXlibPresentationSupportKHR");
    if (table->GetPhysicalDeviceXlibPresentationSupportKHR == nullptr) {
        table->GetPhysicalDeviceXlibPresentationSupportKHR =
            (PFN_vkGetPhysicalDeviceXlibPresentationSupportKHR)StubGetPhysicalDeviceXlibPresentationSupportKHR;
    }
#endif  // VK_USE_PLATFORM_XLIB_KHR
#ifdef VK_USE_PLATFORM_XCB_KHR
    table->CreateXcbSurfaceKHR = (PFN_vkCreateXcbSurfaceKHR)gpa(instance, "vkCreateXcbSurfaceKHR");
    if (table->CreateXcbSurfaceKHR == nullptr) {
        table->CreateXcbSurfaceKHR = (PFN_vkCreateXcbSurfaceKHR)StubCreateXcbSurfaceKHR;
    }
#endif  // VK_USE_PLATFORM_XCB_KHR
#ifdef VK_USE_PLATFORM_XCB_KHR
    table->GetPhysicalDeviceXcbPresentationSupportKHR =
        (PFN_vkGetPhysicalDeviceXcbPresentationSupportKHR)gpa(instance, "vkGetPhysicalDeviceXcbPresentationSupportKHR");
    if (table->GetPhysicalDeviceXcbPresentationSupportKHR == nullptr) {
        table->GetPhysicalDeviceXcbPresentationSupportKHR =
            (PFN_vkGetPhysicalDeviceXcbPresentationSupportKHR)StubGetPhysicalDeviceXcbPresentationSupportKHR;
    }
#endif  // VK_USE_PLATFORM_XCB_KHR
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
    table->CreateWaylandSurfaceKHR = (PFN_vkCreateWaylandSurfaceKHR)gpa(instance, "vkCreateWaylandSurfaceKHR");
    if (table->CreateWaylandSurfaceKHR == nullptr) {
        table->CreateWaylandSurfaceKHR = (PFN_vkCreateWaylandSurfaceKHR)StubCreateWaylandSurfaceKHR;
    }
#endif  // VK_USE_PLATFORM_WAYLAND_KHR
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
    table->GetPhysicalDeviceWaylandPresentationSupportKHR =
        (PFN_vkGetPhysicalDeviceWaylandPresentationSupportKHR)gpa(instance, "vkGetPhysicalDeviceWaylandPresentationSupportKHR");
    if (table->GetPhysicalDeviceWaylandPresentationSupportKHR == nullptr) {
        table->GetPhysicalDeviceWaylandPresentationSupportKHR =
            (PFN_vkGetPhysicalDeviceWaylandPresentationSupportKHR)StubGetPhysicalDeviceWaylandPresentationSupportKHR;
    }
#endif  // VK_USE_PLATFORM_WAYLAND_KHR
#ifdef VK_USE_PLATFORM_ANDROID_KHR
    table->CreateAndroidSurfaceKHR = (PFN_vkCreateAndroidSurfaceKHR)gpa(instance, "vkCreateAndroidSurfaceKHR");
    if (table->CreateAndroidSurfaceKHR == nullptr) {
        table->CreateAndroidSurfaceKHR = (PFN_vkCreateAndroidSurfaceKHR)StubCreateAndroidSurfaceKHR;
    }
#endif  // VK_USE_PLATFORM_ANDROID_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
    table->CreateWin32SurfaceKHR = (PFN_vkCreateWin32SurfaceKHR)gpa(instance, "vkCreateWin32SurfaceKHR");
    if (table->CreateWin32SurfaceKHR == nullptr) {
        table->CreateWin32SurfaceKHR = (PFN_vkCreateWin32SurfaceKHR)StubCreateWin32SurfaceKHR;
    }
#endif  // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
    table->GetPhysicalDeviceWin32PresentationSupportKHR =
        (PFN_vkGetPhysicalDeviceWin32PresentationSupportKHR)gpa(instance, "vkGetPhysicalDeviceWin32PresentationSupportKHR");
    if (table->GetPhysicalDeviceWin32PresentationSupportKHR == nullptr) {
        table->GetPhysicalDeviceWin32PresentationSupportKHR =
            (PFN_vkGetPhysicalDeviceWin32PresentationSupportKHR)StubGetPhysicalDeviceWin32PresentationSupportKHR;
    }
#endif  // VK_USE_PLATFORM_WIN32_KHR
    table->GetPhysicalDeviceVideoCapabilitiesKHR =
        (PFN_vkGetPhysicalDeviceVideoCapabilitiesKHR)gpa(instance, "vkGetPhysicalDeviceVideoCapabilitiesKHR");
    if (table->GetPhysicalDeviceVideoCapabilitiesKHR == nullptr) {
        table->GetPhysicalDeviceVideoCapabilitiesKHR =
            (PFN_vkGetPhysicalDeviceVideoCapabilitiesKHR)StubGetPhysicalDeviceVideoCapabilitiesKHR;
    }
    table->GetPhysicalDeviceVideoFormatPropertiesKHR =
        (PFN_vkGetPhysicalDeviceVideoFormatPropertiesKHR)gpa(instance, "vkGetPhysicalDeviceVideoFormatPropertiesKHR");
    if (table->GetPhysicalDeviceVideoFormatPropertiesKHR == nullptr) {
        table->GetPhysicalDeviceVideoFormatPropertiesKHR =
            (PFN_vkGetPhysicalDeviceVideoFormatPropertiesKHR)StubGetPhysicalDeviceVideoFormatPropertiesKHR;
    }
    table->GetPhysicalDeviceFeatures2KHR = (PFN_vkGetPhysicalDeviceFeatures2KHR)gpa(instance, "vkGetPhysicalDeviceFeatures2KHR");
    if (table->GetPhysicalDeviceFeatures2KHR == nullptr) {
        table->GetPhysicalDeviceFeatures2KHR = (PFN_vkGetPhysicalDeviceFeatures2KHR)StubGetPhysicalDeviceFeatures2KHR;
    }
    table->GetPhysicalDeviceProperties2KHR =
        (PFN_vkGetPhysicalDeviceProperties2KHR)gpa(instance, "vkGetPhysicalDeviceProperties2KHR");
    if (table->GetPhysicalDeviceProperties2KHR == nullptr) {
        table->GetPhysicalDeviceProperties2KHR = (PFN_vkGetPhysicalDeviceProperties2KHR)StubGetPhysicalDeviceProperties2KHR;
    }
    table->GetPhysicalDeviceFormatProperties2KHR =
        (PFN_vkGetPhysicalDeviceFormatProperties2KHR)gpa(instance, "vkGetPhysicalDeviceFormatProperties2KHR");
    if (table->GetPhysicalDeviceFormatProperties2KHR == nullptr) {
        table->GetPhysicalDeviceFormatProperties2KHR =
            (PFN_vkGetPhysicalDeviceFormatProperties2KHR)StubGetPhysicalDeviceFormatProperties2KHR;
    }
    table->GetPhysicalDeviceImageFormatProperties2KHR =
        (PFN_vkGetPhysicalDeviceImageFormatProperties2KHR)gpa(instance, "vkGetPhysicalDeviceImageFormatProperties2KHR");
    if (table->GetPhysicalDeviceImageFormatProperties2KHR == nullptr) {
        table->GetPhysicalDeviceImageFormatProperties2KHR =
            (PFN_vkGetPhysicalDeviceImageFormatProperties2KHR)StubGetPhysicalDeviceImageFormatProperties2KHR;
    }
    table->GetPhysicalDeviceQueueFamilyProperties2KHR =
        (PFN_vkGetPhysicalDeviceQueueFamilyProperties2KHR)gpa(instance, "vkGetPhysicalDeviceQueueFamilyProperties2KHR");
    if (table->GetPhysicalDeviceQueueFamilyProperties2KHR == nullptr) {
        table->GetPhysicalDeviceQueueFamilyProperties2KHR =
            (PFN_vkGetPhysicalDeviceQueueFamilyProperties2KHR)StubGetPhysicalDeviceQueueFamilyProperties2KHR;
    }
    table->GetPhysicalDeviceMemoryProperties2KHR =
        (PFN_vkGetPhysicalDeviceMemoryProperties2KHR)gpa(instance, "vkGetPhysicalDeviceMemoryProperties2KHR");
    if (table->GetPhysicalDeviceMemoryProperties2KHR == nullptr) {
        table->GetPhysicalDeviceMemoryProperties2KHR =
            (PFN_vkGetPhysicalDeviceMemoryProperties2KHR)StubGetPhysicalDeviceMemoryProperties2KHR;
    }
    table->GetPhysicalDeviceSparseImageFormatProperties2KHR =
        (PFN_vkGetPhysicalDeviceSparseImageFormatProperties2KHR)gpa(instance, "vkGetPhysicalDeviceSparseImageFormatProperties2KHR");
    if (table->GetPhysicalDeviceSparseImageFormatProperties2KHR == nullptr) {
        table->GetPhysicalDeviceSparseImageFormatProperties2KHR =
            (PFN_vkGetPhysicalDeviceSparseImageFormatProperties2KHR)StubGetPhysicalDeviceSparseImageFormatProperties2KHR;
    }
    table->EnumeratePhysicalDeviceGroupsKHR =
        (PFN_vkEnumeratePhysicalDeviceGroupsKHR)gpa(instance, "vkEnumeratePhysicalDeviceGroupsKHR");
    if (table->EnumeratePhysicalDeviceGroupsKHR == nullptr) {
        table->EnumeratePhysicalDeviceGroupsKHR = (PFN_vkEnumeratePhysicalDeviceGroupsKHR)StubEnumeratePhysicalDeviceGroupsKHR;
    }
    table->GetPhysicalDeviceExternalBufferPropertiesKHR =
        (PFN_vkGetPhysicalDeviceExternalBufferPropertiesKHR)gpa(instance, "vkGetPhysicalDeviceExternalBufferPropertiesKHR");
    if (table->GetPhysicalDeviceExternalBufferPropertiesKHR == nullptr) {
        table->GetPhysicalDeviceExternalBufferPropertiesKHR =
            (PFN_vkGetPhysicalDeviceExternalBufferPropertiesKHR)StubGetPhysicalDeviceExternalBufferPropertiesKHR;
    }
    table->GetPhysicalDeviceExternalSemaphorePropertiesKHR =
        (PFN_vkGetPhysicalDeviceExternalSemaphorePropertiesKHR)gpa(instance, "vkGetPhysicalDeviceExternalSemaphorePropertiesKHR");
    if (table->GetPhysicalDeviceExternalSemaphorePropertiesKHR == nullptr) {
        table->GetPhysicalDeviceExternalSemaphorePropertiesKHR =
            (PFN_vkGetPhysicalDeviceExternalSemaphorePropertiesKHR)StubGetPhysicalDeviceExternalSemaphorePropertiesKHR;
    }
    table->GetPhysicalDeviceExternalFencePropertiesKHR =
        (PFN_vkGetPhysicalDeviceExternalFencePropertiesKHR)gpa(instance, "vkGetPhysicalDeviceExternalFencePropertiesKHR");
    if (table->GetPhysicalDeviceExternalFencePropertiesKHR == nullptr) {
        table->GetPhysicalDeviceExternalFencePropertiesKHR =
            (PFN_vkGetPhysicalDeviceExternalFencePropertiesKHR)StubGetPhysicalDeviceExternalFencePropertiesKHR;
    }
    table->EnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR =
        (PFN_vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR)gpa(
            instance, "vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR");
    if (table->EnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR == nullptr) {
        table->EnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR =
            (PFN_vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR)
                StubEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR;
    }
    table->GetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR = (PFN_vkGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR)gpa(
        instance, "vkGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR");
    if (table->GetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR == nullptr) {
        table->GetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR =
            (PFN_vkGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR)StubGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR;
    }
    table->GetPhysicalDeviceSurfaceCapabilities2KHR =
        (PFN_vkGetPhysicalDeviceSurfaceCapabilities2KHR)gpa(instance, "vkGetPhysicalDeviceSurfaceCapabilities2KHR");
    if (table->GetPhysicalDeviceSurfaceCapabilities2KHR == nullptr) {
        table->GetPhysicalDeviceSurfaceCapabilities2KHR =
            (PFN_vkGetPhysicalDeviceSurfaceCapabilities2KHR)StubGetPhysicalDeviceSurfaceCapabilities2KHR;
    }
    table->GetPhysicalDeviceSurfaceFormats2KHR =
        (PFN_vkGetPhysicalDeviceSurfaceFormats2KHR)gpa(instance, "vkGetPhysicalDeviceSurfaceFormats2KHR");
    if (table->GetPhysicalDeviceSurfaceFormats2KHR == nullptr) {
        table->GetPhysicalDeviceSurfaceFormats2KHR =
            (PFN_vkGetPhysicalDeviceSurfaceFormats2KHR)StubGetPhysicalDeviceSurfaceFormats2KHR;
    }
    table->GetPhysicalDeviceDisplayProperties2KHR =
        (PFN_vkGetPhysicalDeviceDisplayProperties2KHR)gpa(instance, "vkGetPhysicalDeviceDisplayProperties2KHR");
    if (table->GetPhysicalDeviceDisplayProperties2KHR == nullptr) {
        table->GetPhysicalDeviceDisplayProperties2KHR =
            (PFN_vkGetPhysicalDeviceDisplayProperties2KHR)StubGetPhysicalDeviceDisplayProperties2KHR;
    }
    table->GetPhysicalDeviceDisplayPlaneProperties2KHR =
        (PFN_vkGetPhysicalDeviceDisplayPlaneProperties2KHR)gpa(instance, "vkGetPhysicalDeviceDisplayPlaneProperties2KHR");
    if (table->GetPhysicalDeviceDisplayPlaneProperties2KHR == nullptr) {
        table->GetPhysicalDeviceDisplayPlaneProperties2KHR =
            (PFN_vkGetPhysicalDeviceDisplayPlaneProperties2KHR)StubGetPhysicalDeviceDisplayPlaneProperties2KHR;
    }
    table->GetDisplayModeProperties2KHR = (PFN_vkGetDisplayModeProperties2KHR)gpa(instance, "vkGetDisplayModeProperties2KHR");
    if (table->GetDisplayModeProperties2KHR == nullptr) {
        table->GetDisplayModeProperties2KHR = (PFN_vkGetDisplayModeProperties2KHR)StubGetDisplayModeProperties2KHR;
    }
    table->GetDisplayPlaneCapabilities2KHR =
        (PFN_vkGetDisplayPlaneCapabilities2KHR)gpa(instance, "vkGetDisplayPlaneCapabilities2KHR");
    if (table->GetDisplayPlaneCapabilities2KHR == nullptr) {
        table->GetDisplayPlaneCapabilities2KHR = (PFN_vkGetDisplayPlaneCapabilities2KHR)StubGetDisplayPlaneCapabilities2KHR;
    }
    table->GetPhysicalDeviceFragmentShadingRatesKHR =
        (PFN_vkGetPhysicalDeviceFragmentShadingRatesKHR)gpa(instance, "vkGetPhysicalDeviceFragmentShadingRatesKHR");
    if (table->GetPhysicalDeviceFragmentShadingRatesKHR == nullptr) {
        table->GetPhysicalDeviceFragmentShadingRatesKHR =
            (PFN_vkGetPhysicalDeviceFragmentShadingRatesKHR)StubGetPhysicalDeviceFragmentShadingRatesKHR;
    }
#ifdef VK_ENABLE_BETA_EXTENSIONS
    table->GetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR = (PFN_vkGetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR)gpa(
        instance, "vkGetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR");
    if (table->GetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR == nullptr) {
        table->GetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR =
            (PFN_vkGetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR)StubGetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR;
    }
#endif  // VK_ENABLE_BETA_EXTENSIONS
    table->GetPhysicalDeviceCooperativeMatrixPropertiesKHR =
        (PFN_vkGetPhysicalDeviceCooperativeMatrixPropertiesKHR)gpa(instance, "vkGetPhysicalDeviceCooperativeMatrixPropertiesKHR");
    if (table->GetPhysicalDeviceCooperativeMatrixPropertiesKHR == nullptr) {
        table->GetPhysicalDeviceCooperativeMatrixPropertiesKHR =
            (PFN_vkGetPhysicalDeviceCooperativeMatrixPropertiesKHR)StubGetPhysicalDeviceCooperativeMatrixPropertiesKHR;
    }
    table->CreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT)gpa(instance, "vkCreateDebugReportCallbackEXT");
    if (table->CreateDebugReportCallbackEXT == nullptr) {
        table->CreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT)StubCreateDebugReportCallbackEXT;
    }
    table->DestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT)gpa(instance, "vkDestroyDebugReportCallbackEXT");
    if (table->DestroyDebugReportCallbackEXT == nullptr) {
        table->DestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT)StubDestroyDebugReportCallbackEXT;
    }
    table->DebugReportMessageEXT = (PFN_vkDebugReportMessageEXT)gpa(instance, "vkDebugReportMessageEXT");
    if (table->DebugReportMessageEXT == nullptr) {
        table->DebugReportMessageEXT = (PFN_vkDebugReportMessageEXT)StubDebugReportMessageEXT;
    }
#ifdef VK_USE_PLATFORM_GGP
    table->CreateStreamDescriptorSurfaceGGP =
        (PFN_vkCreateStreamDescriptorSurfaceGGP)gpa(instance, "vkCreateStreamDescriptorSurfaceGGP");
    if (table->CreateStreamDescriptorSurfaceGGP == nullptr) {
        table->CreateStreamDescriptorSurfaceGGP = (PFN_vkCreateStreamDescriptorSurfaceGGP)StubCreateStreamDescriptorSurfaceGGP;
    }
#endif  // VK_USE_PLATFORM_GGP
    table->GetPhysicalDeviceExternalImageFormatPropertiesNV =
        (PFN_vkGetPhysicalDeviceExternalImageFormatPropertiesNV)gpa(instance, "vkGetPhysicalDeviceExternalImageFormatPropertiesNV");
    if (table->GetPhysicalDeviceExternalImageFormatPropertiesNV == nullptr) {
        table->GetPhysicalDeviceExternalImageFormatPropertiesNV =
            (PFN_vkGetPhysicalDeviceExternalImageFormatPropertiesNV)StubGetPhysicalDeviceExternalImageFormatPropertiesNV;
    }
#ifdef VK_USE_PLATFORM_VI_NN
    table->CreateViSurfaceNN = (PFN_vkCreateViSurfaceNN)gpa(instance, "vkCreateViSurfaceNN");
    if (table->CreateViSurfaceNN == nullptr) {
        table->CreateViSurfaceNN = (PFN_vkCreateViSurfaceNN)StubCreateViSurfaceNN;
    }
#endif  // VK_USE_PLATFORM_VI_NN
    table->ReleaseDisplayEXT = (PFN_vkReleaseDisplayEXT)gpa(instance, "vkReleaseDisplayEXT");
    if (table->ReleaseDisplayEXT == nullptr) {
        table->ReleaseDisplayEXT = (PFN_vkReleaseDisplayEXT)StubReleaseDisplayEXT;
    }
#ifdef VK_USE_PLATFORM_XLIB_XRANDR_EXT
    table->AcquireXlibDisplayEXT = (PFN_vkAcquireXlibDisplayEXT)gpa(instance, "vkAcquireXlibDisplayEXT");
    if (table->AcquireXlibDisplayEXT == nullptr) {
        table->AcquireXlibDisplayEXT = (PFN_vkAcquireXlibDisplayEXT)StubAcquireXlibDisplayEXT;
    }
#endif  // VK_USE_PLATFORM_XLIB_XRANDR_EXT
#ifdef VK_USE_PLATFORM_XLIB_XRANDR_EXT
    table->GetRandROutputDisplayEXT = (PFN_vkGetRandROutputDisplayEXT)gpa(instance, "vkGetRandROutputDisplayEXT");
    if (table->GetRandROutputDisplayEXT == nullptr) {
        table->GetRandROutputDisplayEXT = (PFN_vkGetRandROutputDisplayEXT)StubGetRandROutputDisplayEXT;
    }
#endif  // VK_USE_PLATFORM_XLIB_XRANDR_EXT
    table->GetPhysicalDeviceSurfaceCapabilities2EXT =
        (PFN_vkGetPhysicalDeviceSurfaceCapabilities2EXT)gpa(instance, "vkGetPhysicalDeviceSurfaceCapabilities2EXT");
    if (table->GetPhysicalDeviceSurfaceCapabilities2EXT == nullptr) {
        table->GetPhysicalDeviceSurfaceCapabilities2EXT =
            (PFN_vkGetPhysicalDeviceSurfaceCapabilities2EXT)StubGetPhysicalDeviceSurfaceCapabilities2EXT;
    }
#ifdef VK_USE_PLATFORM_IOS_MVK
    table->CreateIOSSurfaceMVK = (PFN_vkCreateIOSSurfaceMVK)gpa(instance, "vkCreateIOSSurfaceMVK");
    if (table->CreateIOSSurfaceMVK == nullptr) {
        table->CreateIOSSurfaceMVK = (PFN_vkCreateIOSSurfaceMVK)StubCreateIOSSurfaceMVK;
    }
#endif  // VK_USE_PLATFORM_IOS_MVK
#ifdef VK_USE_PLATFORM_MACOS_MVK
    table->CreateMacOSSurfaceMVK = (PFN_vkCreateMacOSSurfaceMVK)gpa(instance, "vkCreateMacOSSurfaceMVK");
    if (table->CreateMacOSSurfaceMVK == nullptr) {
        table->CreateMacOSSurfaceMVK = (PFN_vkCreateMacOSSurfaceMVK)StubCreateMacOSSurfaceMVK;
    }
#endif  // VK_USE_PLATFORM_MACOS_MVK
    table->CreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)gpa(instance, "vkCreateDebugUtilsMessengerEXT");
    if (table->CreateDebugUtilsMessengerEXT == nullptr) {
        table->CreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)StubCreateDebugUtilsMessengerEXT;
    }
    table->DestroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT)gpa(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (table->DestroyDebugUtilsMessengerEXT == nullptr) {
        table->DestroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT)StubDestroyDebugUtilsMessengerEXT;
    }
    table->SubmitDebugUtilsMessageEXT = (PFN_vkSubmitDebugUtilsMessageEXT)gpa(instance, "vkSubmitDebugUtilsMessageEXT");
    if (table->SubmitDebugUtilsMessageEXT == nullptr) {
        table->SubmitDebugUtilsMessageEXT = (PFN_vkSubmitDebugUtilsMessageEXT)StubSubmitDebugUtilsMessageEXT;
    }
    table->GetPhysicalDeviceMultisamplePropertiesEXT =
        (PFN_vkGetPhysicalDeviceMultisamplePropertiesEXT)gpa(instance, "vkGetPhysicalDeviceMultisamplePropertiesEXT");
    if (table->GetPhysicalDeviceMultisamplePropertiesEXT == nullptr) {
        table->GetPhysicalDeviceMultisamplePropertiesEXT =
            (PFN_vkGetPhysicalDeviceMultisamplePropertiesEXT)StubGetPhysicalDeviceMultisamplePropertiesEXT;
    }
    table->GetPhysicalDeviceCalibrateableTimeDomainsEXT =
        (PFN_vkGetPhysicalDeviceCalibrateableTimeDomainsEXT)gpa(instance, "vkGetPhysicalDeviceCalibrateableTimeDomainsEXT");
    if (table->GetPhysicalDeviceCalibrateableTimeDomainsEXT == nullptr) {
        table->GetPhysicalDeviceCalibrateableTimeDomainsEXT =
            (PFN_vkGetPhysicalDeviceCalibrateableTimeDomainsEXT)StubGetPhysicalDeviceCalibrateableTimeDomainsEXT;
    }
#ifdef VK_USE_PLATFORM_FUCHSIA
    table->CreateImagePipeSurfaceFUCHSIA = (PFN_vkCreateImagePipeSurfaceFUCHSIA)gpa(instance, "vkCreateImagePipeSurfaceFUCHSIA");
    if (table->CreateImagePipeSurfaceFUCHSIA == nullptr) {
        table->CreateImagePipeSurfaceFUCHSIA = (PFN_vkCreateImagePipeSurfaceFUCHSIA)StubCreateImagePipeSurfaceFUCHSIA;
    }
#endif  // VK_USE_PLATFORM_FUCHSIA
#ifdef VK_USE_PLATFORM_METAL_EXT
    table->CreateMetalSurfaceEXT = (PFN_vkCreateMetalSurfaceEXT)gpa(instance, "vkCreateMetalSurfaceEXT");
    if (table->CreateMetalSurfaceEXT == nullptr) {
        table->CreateMetalSurfaceEXT = (PFN_vkCreateMetalSurfaceEXT)StubCreateMetalSurfaceEXT;
    }
#endif  // VK_USE_PLATFORM_METAL_EXT
    table->GetPhysicalDeviceToolPropertiesEXT =
        (PFN_vkGetPhysicalDeviceToolPropertiesEXT)gpa(instance, "vkGetPhysicalDeviceToolPropertiesEXT");
    if (table->GetPhysicalDeviceToolPropertiesEXT == nullptr) {
        table->GetPhysicalDeviceToolPropertiesEXT =
            (PFN_vkGetPhysicalDeviceToolPropertiesEXT)StubGetPhysicalDeviceToolPropertiesEXT;
    }
    table->GetPhysicalDeviceCooperativeMatrixPropertiesNV =
        (PFN_vkGetPhysicalDeviceCooperativeMatrixPropertiesNV)gpa(instance, "vkGetPhysicalDeviceCooperativeMatrixPropertiesNV");
    if (table->GetPhysicalDeviceCooperativeMatrixPropertiesNV == nullptr) {
        table->GetPhysicalDeviceCooperativeMatrixPropertiesNV =
            (PFN_vkGetPhysicalDeviceCooperativeMatrixPropertiesNV)StubGetPhysicalDeviceCooperativeMatrixPropertiesNV;
    }
    table->GetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV =
        (PFN_vkGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV)gpa(
            instance, "vkGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV");
    if (table->GetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV == nullptr) {
        table->GetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV =
            (PFN_vkGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV)
                StubGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV;
    }
#ifdef VK_USE_PLATFORM_WIN32_KHR
    table->GetPhysicalDeviceSurfacePresentModes2EXT =
        (PFN_vkGetPhysicalDeviceSurfacePresentModes2EXT)gpa(instance, "vkGetPhysicalDeviceSurfacePresentModes2EXT");
    if (table->GetPhysicalDeviceSurfacePresentModes2EXT == nullptr) {
        table->GetPhysicalDeviceSurfacePresentModes2EXT =
            (PFN_vkGetPhysicalDeviceSurfacePresentModes2EXT)StubGetPhysicalDeviceSurfacePresentModes2EXT;
    }
#endif  // VK_USE_PLATFORM_WIN32_KHR
    table->CreateHeadlessSurfaceEXT = (PFN_vkCreateHeadlessSurfaceEXT)gpa(instance, "vkCreateHeadlessSurfaceEXT");
    if (table->CreateHeadlessSurfaceEXT == nullptr) {
        table->CreateHeadlessSurfaceEXT = (PFN_vkCreateHeadlessSurfaceEXT)StubCreateHeadlessSurfaceEXT;
    }
    table->AcquireDrmDisplayEXT = (PFN_vkAcquireDrmDisplayEXT)gpa(instance, "vkAcquireDrmDisplayEXT");
    if (table->AcquireDrmDisplayEXT == nullptr) {
        table->AcquireDrmDisplayEXT = (PFN_vkAcquireDrmDisplayEXT)StubAcquireDrmDisplayEXT;
    }
    table->GetDrmDisplayEXT = (PFN_vkGetDrmDisplayEXT)gpa(instance, "vkGetDrmDisplayEXT");
    if (table->GetDrmDisplayEXT == nullptr) {
        table->GetDrmDisplayEXT = (PFN_vkGetDrmDisplayEXT)StubGetDrmDisplayEXT;
    }
#ifdef VK_USE_PLATFORM_WIN32_KHR
    table->AcquireWinrtDisplayNV = (PFN_vkAcquireWinrtDisplayNV)gpa(instance, "vkAcquireWinrtDisplayNV");
    if (table->AcquireWinrtDisplayNV == nullptr) {
        table->AcquireWinrtDisplayNV = (PFN_vkAcquireWinrtDisplayNV)StubAcquireWinrtDisplayNV;
    }
#endif  // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
    table->GetWinrtDisplayNV = (PFN_vkGetWinrtDisplayNV)gpa(instance, "vkGetWinrtDisplayNV");
    if (table->GetWinrtDisplayNV == nullptr) {
        table->GetWinrtDisplayNV = (PFN_vkGetWinrtDisplayNV)StubGetWinrtDisplayNV;
    }
#endif  // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_DIRECTFB_EXT
    table->CreateDirectFBSurfaceEXT = (PFN_vkCreateDirectFBSurfaceEXT)gpa(instance, "vkCreateDirectFBSurfaceEXT");
    if (table->CreateDirectFBSurfaceEXT == nullptr) {
        table->CreateDirectFBSurfaceEXT = (PFN_vkCreateDirectFBSurfaceEXT)StubCreateDirectFBSurfaceEXT;
    }
#endif  // VK_USE_PLATFORM_DIRECTFB_EXT
#ifdef VK_USE_PLATFORM_DIRECTFB_EXT
    table->GetPhysicalDeviceDirectFBPresentationSupportEXT =
        (PFN_vkGetPhysicalDeviceDirectFBPresentationSupportEXT)gpa(instance, "vkGetPhysicalDeviceDirectFBPresentationSupportEXT");
    if (table->GetPhysicalDeviceDirectFBPresentationSupportEXT == nullptr) {
        table->GetPhysicalDeviceDirectFBPresentationSupportEXT =
            (PFN_vkGetPhysicalDeviceDirectFBPresentationSupportEXT)StubGetPhysicalDeviceDirectFBPresentationSupportEXT;
    }
#endif  // VK_USE_PLATFORM_DIRECTFB_EXT
#ifdef VK_USE_PLATFORM_SCREEN_QNX
    table->CreateScreenSurfaceQNX = (PFN_vkCreateScreenSurfaceQNX)gpa(instance, "vkCreateScreenSurfaceQNX");
    if (table->CreateScreenSurfaceQNX == nullptr) {
        table->CreateScreenSurfaceQNX = (PFN_vkCreateScreenSurfaceQNX)StubCreateScreenSurfaceQNX;
    }
#endif  // VK_USE_PLATFORM_SCREEN_QNX
#ifdef VK_USE_PLATFORM_SCREEN_QNX
    table->GetPhysicalDeviceScreenPresentationSupportQNX =
        (PFN_vkGetPhysicalDeviceScreenPresentationSupportQNX)gpa(instance, "vkGetPhysicalDeviceScreenPresentationSupportQNX");
    if (table->GetPhysicalDeviceScreenPresentationSupportQNX == nullptr) {
        table->GetPhysicalDeviceScreenPresentationSupportQNX =
            (PFN_vkGetPhysicalDeviceScreenPresentationSupportQNX)StubGetPhysicalDeviceScreenPresentationSupportQNX;
    }
#endif  // VK_USE_PLATFORM_SCREEN_QNX
    table->GetPhysicalDeviceOpticalFlowImageFormatsNV =
        (PFN_vkGetPhysicalDeviceOpticalFlowImageFormatsNV)gpa(instance, "vkGetPhysicalDeviceOpticalFlowImageFormatsNV");
    if (table->GetPhysicalDeviceOpticalFlowImageFormatsNV == nullptr) {
        table->GetPhysicalDeviceOpticalFlowImageFormatsNV =
            (PFN_vkGetPhysicalDeviceOpticalFlowImageFormatsNV)StubGetPhysicalDeviceOpticalFlowImageFormatsNV;
    }
}
// NOLINTEND
