/* Copyright (c) 2019 The Khronos Group Inc.
 * Copyright (c) 2019 Valve Corporation
 * Copyright (c) 2019 LunarG, Inc.
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
 * Author: Mike Weiblen <mikew@lunarg.com>
 */

//#pragma message( "this is : #pragma message1")

#include <stdio.h>
#include "chassis.h"
#include "portability_validation.h"

bool PortabilityValidation::PreCallValidateCreateInstance(const VkInstanceCreateInfo* pCreateInfo,
                                                          const VkAllocationCallbacks* pAllocator, VkInstance* pInstance) {
    printf("%s @ %d\n", __FUNCTION__, __LINE__);
    return false;
}

void PortabilityValidation::PreCallRecordCreateInstance(const VkInstanceCreateInfo* pCreateInfo,
                                                        const VkAllocationCallbacks* pAllocator, VkInstance* pInstance) {
    printf("%s @ %d\n", __FUNCTION__, __LINE__);
}

void PortabilityValidation::PostCallRecordCreateInstance(const VkInstanceCreateInfo* pCreateInfo,
                                                         const VkAllocationCallbacks* pAllocator, VkInstance* pInstance,
                                                         VkResult result) {
    printf("%s @ %d\n", __FUNCTION__, __LINE__);
}

bool PortabilityValidation::PreCallValidateEnumeratePhysicalDevices(VkInstance instance, uint32_t* pPhysicalDeviceCount,
                                                                    VkPhysicalDevice* pPhysicalDevices) {
    printf("%s @ %d\n", __FUNCTION__, __LINE__);
    return false;
}

void PortabilityValidation::PreCallRecordEnumeratePhysicalDevices(VkInstance instance, uint32_t* pPhysicalDeviceCount,
                                                                  VkPhysicalDevice* pPhysicalDevices) {
    printf("%s @ %d\n", __FUNCTION__, __LINE__);
}

void PortabilityValidation::PostCallRecordEnumeratePhysicalDevices(VkInstance instance, uint32_t* pPhysicalDeviceCount,
                                                                   VkPhysicalDevice* pPhysicalDevices, VkResult result) {
    printf("%s @ %d\n", __FUNCTION__, __LINE__);
}

bool PortabilityValidation::PreCallValidateCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo,
                                                        const VkAllocationCallbacks* pAllocator, VkDevice* pDevice) {
    printf("%s @ %d\n", __FUNCTION__, __LINE__);
    return false;
}

void PortabilityValidation::PreCallRecordCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo,
                                                      const VkAllocationCallbacks* pAllocator, VkDevice* pDevice) {
    printf("%s @ %d\n", __FUNCTION__, __LINE__);
}

void PortabilityValidation::PostCallRecordCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo,
                                                       const VkAllocationCallbacks* pAllocator, VkDevice* pDevice,
                                                       VkResult result) {
    printf("%s @ %d\n", __FUNCTION__, __LINE__);
}

bool PortabilityValidation::PreCallValidateEnumerateInstanceExtensionProperties(const char* pLayerName, uint32_t* pPropertyCount,
                                                                                VkExtensionProperties* pProperties) {
    printf("%s @ %d\n", __FUNCTION__, __LINE__);
    return false;
}

void PortabilityValidation::PreCallRecordEnumerateInstanceExtensionProperties(const char* pLayerName, uint32_t* pPropertyCount,
                                                                              VkExtensionProperties* pProperties) {
    printf("%s @ %d\n", __FUNCTION__, __LINE__);
}

void PortabilityValidation::PostCallRecordEnumerateInstanceExtensionProperties(const char* pLayerName, uint32_t* pPropertyCount,
                                                                               VkExtensionProperties* pProperties,
                                                                               VkResult result) {
    printf("%s @ %d\n", __FUNCTION__, __LINE__);
}

bool PortabilityValidation::PreCallValidateEnumerateDeviceExtensionProperties(VkPhysicalDevice physicalDevice,
                                                                              const char* pLayerName, uint32_t* pPropertyCount,
                                                                              VkExtensionProperties* pProperties) {
    printf("%s @ %d\n", __FUNCTION__, __LINE__);
    return false;
}

void PortabilityValidation::PreCallRecordEnumerateDeviceExtensionProperties(VkPhysicalDevice physicalDevice, const char* pLayerName,
                                                                            uint32_t* pPropertyCount,
                                                                            VkExtensionProperties* pProperties) {
    printf("%s @ %d\n", __FUNCTION__, __LINE__);
}

void PortabilityValidation::PostCallRecordEnumerateDeviceExtensionProperties(VkPhysicalDevice physicalDevice,
                                                                             const char* pLayerName, uint32_t* pPropertyCount,
                                                                             VkExtensionProperties* pProperties, VkResult result) {
    printf("%s @ %d\n", __FUNCTION__, __LINE__);
}

bool PortabilityValidation::PreCallValidateEnumerateInstanceLayerProperties(uint32_t* pPropertyCount,
                                                                            VkLayerProperties* pProperties) {
    printf("%s @ %d\n", __FUNCTION__, __LINE__);
    return false;
}

void PortabilityValidation::PreCallRecordEnumerateInstanceLayerProperties(uint32_t* pPropertyCount,
                                                                          VkLayerProperties* pProperties) {
    printf("%s @ %d\n", __FUNCTION__, __LINE__);
}

void PortabilityValidation::PostCallRecordEnumerateInstanceLayerProperties(uint32_t* pPropertyCount, VkLayerProperties* pProperties,
                                                                           VkResult result) {
    printf("%s @ %d\n", __FUNCTION__, __LINE__);
}

bool PortabilityValidation::PreCallValidateEnumerateDeviceLayerProperties(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount,
                                                                          VkLayerProperties* pProperties) {
    printf("%s @ %d\n", __FUNCTION__, __LINE__);
    return false;
}

void PortabilityValidation::PreCallRecordEnumerateDeviceLayerProperties(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount,
                                                                        VkLayerProperties* pProperties) {
    printf("%s @ %d\n", __FUNCTION__, __LINE__);
}

void PortabilityValidation::PostCallRecordEnumerateDeviceLayerProperties(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount,
                                                                         VkLayerProperties* pProperties, VkResult result) {
    printf("%s @ %d\n", __FUNCTION__, __LINE__);
}

bool PortabilityValidation::PreCallValidateGetPhysicalDeviceFeatures2KHR(VkPhysicalDevice physicalDevice,
                                                                         VkPhysicalDeviceFeatures2* pFeatures) {
    printf("%s @ %d\n", __FUNCTION__, __LINE__);
    return false;
}

void PortabilityValidation::PreCallRecordGetPhysicalDeviceFeatures2KHR(VkPhysicalDevice physicalDevice,
                                                                       VkPhysicalDeviceFeatures2* pFeatures) {
    printf("%s @ %d\n", __FUNCTION__, __LINE__);
}

void PortabilityValidation::PostCallRecordGetPhysicalDeviceFeatures2KHR(VkPhysicalDevice physicalDevice,
                                                                        VkPhysicalDeviceFeatures2* pFeatures) {
    printf("%s @ %d\n", __FUNCTION__, __LINE__);
}

bool PortabilityValidation::PreCallValidateGetPhysicalDeviceProperties2KHR(VkPhysicalDevice physicalDevice,
                                                                           VkPhysicalDeviceProperties2* pProperties) {
    printf("%s @ %d\n", __FUNCTION__, __LINE__);
    return false;
}

void PortabilityValidation::PreCallRecordGetPhysicalDeviceProperties2KHR(VkPhysicalDevice physicalDevice,
                                                                         VkPhysicalDeviceProperties2* pProperties) {
    printf("%s @ %d\n", __FUNCTION__, __LINE__);
}

void PortabilityValidation::PostCallRecordGetPhysicalDeviceProperties2KHR(VkPhysicalDevice physicalDevice,
                                                                          VkPhysicalDeviceProperties2* pProperties) {
    printf("%s @ %d\n", __FUNCTION__, __LINE__);
}

bool PortabilityValidation::PreCallValidateGetPhysicalDeviceImageFormatProperties2KHR(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceImageFormatInfo2* pImageFormatInfo,
    VkImageFormatProperties2* pImageFormatProperties) {
    printf("%s @ %d\n", __FUNCTION__, __LINE__);
    return false;
}

void PortabilityValidation::PreCallRecordGetPhysicalDeviceImageFormatProperties2KHR(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceImageFormatInfo2* pImageFormatInfo,
    VkImageFormatProperties2* pImageFormatProperties) {
    printf("%s @ %d\n", __FUNCTION__, __LINE__);
}

void PortabilityValidation::PostCallRecordGetPhysicalDeviceImageFormatProperties2KHR(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceImageFormatInfo2* pImageFormatInfo,
    VkImageFormatProperties2* pImageFormatProperties, VkResult result) {
    printf("%s @ %d\n", __FUNCTION__, __LINE__);
}

bool PortabilityValidation::PreCallValidateCreateEvent(VkDevice device, const VkEventCreateInfo* pCreateInfo,
                                                       const VkAllocationCallbacks* pAllocator, VkEvent* pEvent) {
    printf("%s @ %d\n", __FUNCTION__, __LINE__);
    return false;
}

void PortabilityValidation::PreCallRecordCreateEvent(VkDevice device, const VkEventCreateInfo* pCreateInfo,
                                                     const VkAllocationCallbacks* pAllocator, VkEvent* pEvent) {
    printf("%s @ %d\n", __FUNCTION__, __LINE__);
}

void PortabilityValidation::PostCallRecordCreateEvent(VkDevice device, const VkEventCreateInfo* pCreateInfo,
                                                      const VkAllocationCallbacks* pAllocator, VkEvent* pEvent, VkResult result) {
    printf("%s @ %d\n", __FUNCTION__, __LINE__);
}

bool PortabilityValidation::PreCallValidateDestroyEvent(VkDevice device, VkEvent event, const VkAllocationCallbacks* pAllocator) {
    printf("%s @ %d\n", __FUNCTION__, __LINE__);
    return false;
}

void PortabilityValidation::PreCallRecordDestroyEvent(VkDevice device, VkEvent event, const VkAllocationCallbacks* pAllocator) {
    printf("%s @ %d\n", __FUNCTION__, __LINE__);
}

void PortabilityValidation::PostCallRecordDestroyEvent(VkDevice device, VkEvent event, const VkAllocationCallbacks* pAllocator) {
    printf("%s @ %d\n", __FUNCTION__, __LINE__);
}

bool PortabilityValidation::PreCallValidateGetEventStatus(VkDevice device, VkEvent event) {
    printf("%s @ %d\n", __FUNCTION__, __LINE__);
    return false;
}

void PortabilityValidation::PreCallRecordGetEventStatus(VkDevice device, VkEvent event) {
    printf("%s @ %d\n", __FUNCTION__, __LINE__);
}

void PortabilityValidation::PostCallRecordGetEventStatus(VkDevice device, VkEvent event, VkResult result) {
    printf("%s @ %d\n", __FUNCTION__, __LINE__);
}

bool PortabilityValidation::PreCallValidateSetEvent(VkDevice device, VkEvent event) {
    printf("%s @ %d\n", __FUNCTION__, __LINE__);
    return false;
}

void PortabilityValidation::PreCallRecordSetEvent(VkDevice device, VkEvent event) {
    printf("%s @ %d\n", __FUNCTION__, __LINE__);
}

void PortabilityValidation::PostCallRecordSetEvent(VkDevice device, VkEvent event, VkResult result) {
    printf("%s @ %d\n", __FUNCTION__, __LINE__);
}

bool PortabilityValidation::PreCallValidateResetEvent(VkDevice device, VkEvent event) {
    printf("%s @ %d\n", __FUNCTION__, __LINE__);
    return false;
}

void PortabilityValidation::PreCallRecordResetEvent(VkDevice device, VkEvent event) {
    printf("%s @ %d\n", __FUNCTION__, __LINE__);
}

void PortabilityValidation::PostCallRecordResetEvent(VkDevice device, VkEvent event, VkResult result) {
    printf("%s @ %d\n", __FUNCTION__, __LINE__);
}

bool PortabilityValidation::PreCallValidateCmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event,
                                                       VkPipelineStageFlags stageMask) {
    printf("%s @ %d\n", __FUNCTION__, __LINE__);
    return false;
}

void PortabilityValidation::PreCallRecordCmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask) {
    printf("%s @ %d\n", __FUNCTION__, __LINE__);
}

void PortabilityValidation::PostCallRecordCmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event,
                                                      VkPipelineStageFlags stageMask) {
    printf("%s @ %d\n", __FUNCTION__, __LINE__);
}

bool PortabilityValidation::PreCallValidateCmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event,
                                                         VkPipelineStageFlags stageMask) {
    printf("%s @ %d\n", __FUNCTION__, __LINE__);
    return false;
}

void PortabilityValidation::PreCallRecordCmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event,
                                                       VkPipelineStageFlags stageMask) {
    printf("%s @ %d\n", __FUNCTION__, __LINE__);
}

void PortabilityValidation::PostCallRecordCmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event,
                                                        VkPipelineStageFlags stageMask) {
    printf("%s @ %d\n", __FUNCTION__, __LINE__);
}

bool PortabilityValidation::PreCallValidateCmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                                         VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask,
                                                         uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers,
                                                         uint32_t bufferMemoryBarrierCount,
                                                         const VkBufferMemoryBarrier* pBufferMemoryBarriers,
                                                         uint32_t imageMemoryBarrierCount,
                                                         const VkImageMemoryBarrier* pImageMemoryBarriers) {
    printf("%s @ %d\n", __FUNCTION__, __LINE__);
    return false;
}

void PortabilityValidation::PreCallRecordCmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                                       VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask,
                                                       uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers,
                                                       uint32_t bufferMemoryBarrierCount,
                                                       const VkBufferMemoryBarrier* pBufferMemoryBarriers,
                                                       uint32_t imageMemoryBarrierCount,
                                                       const VkImageMemoryBarrier* pImageMemoryBarriers) {
    printf("%s @ %d\n", __FUNCTION__, __LINE__);
}

void PortabilityValidation::PostCallRecordCmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                                        VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask,
                                                        uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers,
                                                        uint32_t bufferMemoryBarrierCount,
                                                        const VkBufferMemoryBarrier* pBufferMemoryBarriers,
                                                        uint32_t imageMemoryBarrierCount,
                                                        const VkImageMemoryBarrier* pImageMemoryBarriers) {
    printf("%s @ %d\n", __FUNCTION__, __LINE__);
}

bool PortabilityValidation::PreCallValidateCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache,
                                                                   uint32_t createInfoCount,
                                                                   const VkGraphicsPipelineCreateInfo* pCreateInfos,
                                                                   const VkAllocationCallbacks* pAllocator,
                                                                   VkPipeline* pPipelines) {
    printf("%s @ %d\n", __FUNCTION__, __LINE__);
    return false;
}

void PortabilityValidation::PreCallRecordCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache,
                                                                 uint32_t createInfoCount,
                                                                 const VkGraphicsPipelineCreateInfo* pCreateInfos,
                                                                 const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines) {
    printf("%s @ %d\n", __FUNCTION__, __LINE__);
}

void PortabilityValidation::PostCallRecordCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache,
                                                                  uint32_t createInfoCount,
                                                                  const VkGraphicsPipelineCreateInfo* pCreateInfos,
                                                                  const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                                  VkResult result) {
    printf("%s @ %d\n", __FUNCTION__, __LINE__);
}

bool PortabilityValidation::PreCallValidateCreateImageView(VkDevice device, const VkImageViewCreateInfo* pCreateInfo,
                                                           const VkAllocationCallbacks* pAllocator, VkImageView* pView) {
    printf("%s @ %d\n", __FUNCTION__, __LINE__);
    return false;
}

void PortabilityValidation::PreCallRecordCreateImageView(VkDevice device, const VkImageViewCreateInfo* pCreateInfo,
                                                         const VkAllocationCallbacks* pAllocator, VkImageView* pView) {
    printf("%s @ %d\n", __FUNCTION__, __LINE__);
}

void PortabilityValidation::PostCallRecordCreateImageView(VkDevice device, const VkImageViewCreateInfo* pCreateInfo,
                                                          const VkAllocationCallbacks* pAllocator, VkImageView* pView,
                                                          VkResult result) {
    printf("%s @ %d\n", __FUNCTION__, __LINE__);
}

bool PortabilityValidation::PreCallValidateCreateSampler(VkDevice device, const VkSamplerCreateInfo* pCreateInfo,
                                                         const VkAllocationCallbacks* pAllocator, VkSampler* pSampler) {
    printf("%s @ %d\n", __FUNCTION__, __LINE__);
    return false;
}

void PortabilityValidation::PreCallRecordCreateSampler(VkDevice device, const VkSamplerCreateInfo* pCreateInfo,
                                                       const VkAllocationCallbacks* pAllocator, VkSampler* pSampler) {
    printf("%s @ %d\n", __FUNCTION__, __LINE__);
}

void PortabilityValidation::PostCallRecordCreateSampler(VkDevice device, const VkSamplerCreateInfo* pCreateInfo,
                                                        const VkAllocationCallbacks* pAllocator, VkSampler* pSampler,
                                                        VkResult result) {
    printf("%s @ %d\n", __FUNCTION__, __LINE__);
}

// vim: set sw=4 ts=8 et ic ai:
