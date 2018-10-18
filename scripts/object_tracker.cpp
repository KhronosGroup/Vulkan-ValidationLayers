// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See object_tracker_generator.py for modifications


/***************************************************************************
 *
 * Copyright (c) 2015-2018 The Khronos Group Inc.
 * Copyright (c) 2015-2018 Valve Corporation
 * Copyright (c) 2015-2018 LunarG, Inc.
 * Copyright (c) 2015-2018 Google Inc.
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
 * Author: Dave Houlton <daveh@lunarg.com>
 *
 ****************************************************************************/


#include "uber_layer.h"
#include "object_lifetime_validation.h"





// ObjectTracker undestroyed objects validation function
bool ObjectLifetimes::ReportUndestroyedObjects(VkDevice device, const std::string& error_code) {
    bool skip = false;
    skip |= DeviceReportUndestroyedObjects(device, kVulkanObjectTypeCommandBuffer, error_code);
    skip |= DeviceReportUndestroyedObjects(device, kVulkanObjectTypeSemaphore, error_code);
    skip |= DeviceReportUndestroyedObjects(device, kVulkanObjectTypeFence, error_code);
    skip |= DeviceReportUndestroyedObjects(device, kVulkanObjectTypeDeviceMemory, error_code);
    skip |= DeviceReportUndestroyedObjects(device, kVulkanObjectTypeBuffer, error_code);
    skip |= DeviceReportUndestroyedObjects(device, kVulkanObjectTypeImage, error_code);
    skip |= DeviceReportUndestroyedObjects(device, kVulkanObjectTypeEvent, error_code);
    skip |= DeviceReportUndestroyedObjects(device, kVulkanObjectTypeQueryPool, error_code);
    skip |= DeviceReportUndestroyedObjects(device, kVulkanObjectTypeBufferView, error_code);
    skip |= DeviceReportUndestroyedObjects(device, kVulkanObjectTypeImageView, error_code);
    skip |= DeviceReportUndestroyedObjects(device, kVulkanObjectTypeShaderModule, error_code);
    skip |= DeviceReportUndestroyedObjects(device, kVulkanObjectTypePipelineCache, error_code);
    skip |= DeviceReportUndestroyedObjects(device, kVulkanObjectTypePipelineLayout, error_code);
    skip |= DeviceReportUndestroyedObjects(device, kVulkanObjectTypeRenderPass, error_code);
    skip |= DeviceReportUndestroyedObjects(device, kVulkanObjectTypePipeline, error_code);
    skip |= DeviceReportUndestroyedObjects(device, kVulkanObjectTypeDescriptorSetLayout, error_code);
    skip |= DeviceReportUndestroyedObjects(device, kVulkanObjectTypeSampler, error_code);
    skip |= DeviceReportUndestroyedObjects(device, kVulkanObjectTypeDescriptorPool, error_code);
    skip |= DeviceReportUndestroyedObjects(device, kVulkanObjectTypeDescriptorSet, error_code);
    skip |= DeviceReportUndestroyedObjects(device, kVulkanObjectTypeFramebuffer, error_code);
    skip |= DeviceReportUndestroyedObjects(device, kVulkanObjectTypeCommandPool, error_code);
    skip |= DeviceReportUndestroyedObjects(device, kVulkanObjectTypeSamplerYcbcrConversion, error_code);
    skip |= DeviceReportUndestroyedObjects(device, kVulkanObjectTypeDescriptorUpdateTemplate, error_code);
    skip |= DeviceReportUndestroyedObjects(device, kVulkanObjectTypeSurfaceKHR, error_code);
    skip |= DeviceReportUndestroyedObjects(device, kVulkanObjectTypeSwapchainKHR, error_code);
    skip |= DeviceReportUndestroyedObjects(device, kVulkanObjectTypeDisplayKHR, error_code);
    skip |= DeviceReportUndestroyedObjects(device, kVulkanObjectTypeDisplayModeKHR, error_code);
    skip |= DeviceReportUndestroyedObjects(device, kVulkanObjectTypeDebugReportCallbackEXT, error_code);
    skip |= DeviceReportUndestroyedObjects(device, kVulkanObjectTypeObjectTableNVX, error_code);
    skip |= DeviceReportUndestroyedObjects(device, kVulkanObjectTypeIndirectCommandsLayoutNVX, error_code);
    skip |= DeviceReportUndestroyedObjects(device, kVulkanObjectTypeDebugUtilsMessengerEXT, error_code);
    skip |= DeviceReportUndestroyedObjects(device, kVulkanObjectTypeValidationCacheEXT, error_code);
    skip |= DeviceReportUndestroyedObjects(device, kVulkanObjectTypeAccelerationStructureNVX, error_code);
    return skip;
}

void ObjectLifetimes::DestroyUndestroyedObjects(VkDevice device) {
    DeviceDestroyUndestroyedObjects(device, kVulkanObjectTypeCommandBuffer);
    DeviceDestroyUndestroyedObjects(device, kVulkanObjectTypeSemaphore);
    DeviceDestroyUndestroyedObjects(device, kVulkanObjectTypeFence);
    DeviceDestroyUndestroyedObjects(device, kVulkanObjectTypeDeviceMemory);
    DeviceDestroyUndestroyedObjects(device, kVulkanObjectTypeBuffer);
    DeviceDestroyUndestroyedObjects(device, kVulkanObjectTypeImage);
    DeviceDestroyUndestroyedObjects(device, kVulkanObjectTypeEvent);
    DeviceDestroyUndestroyedObjects(device, kVulkanObjectTypeQueryPool);
    DeviceDestroyUndestroyedObjects(device, kVulkanObjectTypeBufferView);
    DeviceDestroyUndestroyedObjects(device, kVulkanObjectTypeImageView);
    DeviceDestroyUndestroyedObjects(device, kVulkanObjectTypeShaderModule);
    DeviceDestroyUndestroyedObjects(device, kVulkanObjectTypePipelineCache);
    DeviceDestroyUndestroyedObjects(device, kVulkanObjectTypePipelineLayout);
    DeviceDestroyUndestroyedObjects(device, kVulkanObjectTypeRenderPass);
    DeviceDestroyUndestroyedObjects(device, kVulkanObjectTypePipeline);
    DeviceDestroyUndestroyedObjects(device, kVulkanObjectTypeDescriptorSetLayout);
    DeviceDestroyUndestroyedObjects(device, kVulkanObjectTypeSampler);
    DeviceDestroyUndestroyedObjects(device, kVulkanObjectTypeDescriptorPool);
    DeviceDestroyUndestroyedObjects(device, kVulkanObjectTypeDescriptorSet);
    DeviceDestroyUndestroyedObjects(device, kVulkanObjectTypeFramebuffer);
    DeviceDestroyUndestroyedObjects(device, kVulkanObjectTypeCommandPool);
    DeviceDestroyUndestroyedObjects(device, kVulkanObjectTypeSamplerYcbcrConversion);
    DeviceDestroyUndestroyedObjects(device, kVulkanObjectTypeDescriptorUpdateTemplate);
    DeviceDestroyUndestroyedObjects(device, kVulkanObjectTypeSurfaceKHR);
    DeviceDestroyUndestroyedObjects(device, kVulkanObjectTypeSwapchainKHR);
    DeviceDestroyUndestroyedObjects(device, kVulkanObjectTypeDisplayKHR);
    DeviceDestroyUndestroyedObjects(device, kVulkanObjectTypeDisplayModeKHR);
    DeviceDestroyUndestroyedObjects(device, kVulkanObjectTypeDebugReportCallbackEXT);
    DeviceDestroyUndestroyedObjects(device, kVulkanObjectTypeObjectTableNVX);
    DeviceDestroyUndestroyedObjects(device, kVulkanObjectTypeIndirectCommandsLayoutNVX);
    DeviceDestroyUndestroyedObjects(device, kVulkanObjectTypeDebugUtilsMessengerEXT);
    DeviceDestroyUndestroyedObjects(device, kVulkanObjectTypeValidationCacheEXT);
    DeviceDestroyUndestroyedObjects(device, kVulkanObjectTypeAccelerationStructureNVX);
}



bool ObjectLifetimes::PreCallValidateGetPhysicalDeviceFeatures(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceFeatures*                   pFeatures) {
    bool skip = false;
    skip |= InstanceValidateObject(physicalDevice, physicalDevice, kVulkanObjectTypePhysicalDevice, false, "VUID-vkGetPhysicalDeviceFeatures-physicalDevice-parameter", kVUIDUndefined);

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetPhysicalDeviceFormatProperties(
    VkPhysicalDevice                            physicalDevice,
    VkFormat                                    format,
    VkFormatProperties*                         pFormatProperties) {
    bool skip = false;
    skip |= InstanceValidateObject(physicalDevice, physicalDevice, kVulkanObjectTypePhysicalDevice, false, "VUID-vkGetPhysicalDeviceFormatProperties-physicalDevice-parameter", kVUIDUndefined);

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetPhysicalDeviceImageFormatProperties(
    VkPhysicalDevice                            physicalDevice,
    VkFormat                                    format,
    VkImageType                                 type,
    VkImageTiling                               tiling,
    VkImageUsageFlags                           usage,
    VkImageCreateFlags                          flags,
    VkImageFormatProperties*                    pImageFormatProperties) {
    bool skip = false;
    skip |= InstanceValidateObject(physicalDevice, physicalDevice, kVulkanObjectTypePhysicalDevice, false, "VUID-vkGetPhysicalDeviceImageFormatProperties-physicalDevice-parameter", kVUIDUndefined);

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetPhysicalDeviceProperties(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceProperties*                 pProperties) {
    bool skip = false;
    skip |= InstanceValidateObject(physicalDevice, physicalDevice, kVulkanObjectTypePhysicalDevice, false, "VUID-vkGetPhysicalDeviceProperties-physicalDevice-parameter", kVUIDUndefined);

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetPhysicalDeviceMemoryProperties(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceMemoryProperties*           pMemoryProperties) {
    bool skip = false;
    skip |= InstanceValidateObject(physicalDevice, physicalDevice, kVulkanObjectTypePhysicalDevice, false, "VUID-vkGetPhysicalDeviceMemoryProperties-physicalDevice-parameter", kVUIDUndefined);

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetInstanceProcAddr(
    VkInstance                                  instance,
    const char*                                 pName) {
    bool skip = false;
    skip |= InstanceValidateObject(instance, instance, kVulkanObjectTypeInstance, true, "VUID-vkGetInstanceProcAddr-instance-parameter", kVUIDUndefined);

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetDeviceProcAddr(
    VkDevice                                    device,
    const char*                                 pName) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkGetDeviceProcAddr-device-parameter", kVUIDUndefined);

    return skip;
}

bool ObjectLifetimes::PreCallValidateCreateDevice(
    VkPhysicalDevice                            physicalDevice,
    const VkDeviceCreateInfo*                   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDevice*                                   pDevice) {
    bool skip = false;
    skip |= InstanceValidateObject(physicalDevice, physicalDevice, kVulkanObjectTypePhysicalDevice, false, "VUID-vkCreateDevice-physicalDevice-parameter", kVUIDUndefined);

    return skip;
}

void ObjectLifetimes::PostCallRecordCreateDevice(
    VkPhysicalDevice                            physicalDevice,
    const VkDeviceCreateInfo*                   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDevice*                                   pDevice) {
    InstanceCreateObject(physicalDevice, *pDevice, kVulkanObjectTypeDevice, pAllocator);

}

bool ObjectLifetimes::PreCallValidateEnumerateDeviceExtensionProperties(
    VkPhysicalDevice                            physicalDevice,
    const char*                                 pLayerName,
    uint32_t*                                   pPropertyCount,
    VkExtensionProperties*                      pProperties) {
    bool skip = false;
    skip |= InstanceValidateObject(physicalDevice, physicalDevice, kVulkanObjectTypePhysicalDevice, false, "VUID-vkEnumerateDeviceExtensionProperties-physicalDevice-parameter", kVUIDUndefined);

    return skip;
}

bool ObjectLifetimes::PreCallValidateEnumerateDeviceLayerProperties(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pPropertyCount,
    VkLayerProperties*                          pProperties) {
    bool skip = false;
    skip |= InstanceValidateObject(physicalDevice, physicalDevice, kVulkanObjectTypePhysicalDevice, false, "VUID-vkEnumerateDeviceLayerProperties-physicalDevice-parameter", kVUIDUndefined);

    return skip;
}

bool ObjectLifetimes::PreCallValidateQueueSubmit(
    VkQueue                                     queue,
    uint32_t                                    submitCount,
    const VkSubmitInfo*                         pSubmits,
    VkFence                                     fence) {
    bool skip = false;
    skip |= DeviceValidateObject(queue, queue, kVulkanObjectTypeQueue, false, "VUID-vkQueueSubmit-queue-parameter", "VUID-vkQueueSubmit-commonparent");
    if (pSubmits) {
        for (uint32_t index0 = 0; index0 < submitCount; ++index0) {
            for (uint32_t index1 = 0; index1 < pSubmits[index0].waitSemaphoreCount; ++index1) {
                skip |= DeviceValidateObject(queue, pSubmits[index0].pWaitSemaphores[index1], kVulkanObjectTypeSemaphore, false, "VUID-VkSubmitInfo-pWaitSemaphores-parameter", "VUID-VkSubmitInfo-commonparent");
            }
            for (uint32_t index1 = 0; index1 < pSubmits[index0].commandBufferCount; ++index1) {
                skip |= DeviceValidateObject(queue, pSubmits[index0].pCommandBuffers[index1], kVulkanObjectTypeCommandBuffer, false, "VUID-VkSubmitInfo-pCommandBuffers-parameter", "VUID-VkSubmitInfo-commonparent");
            }
            for (uint32_t index1 = 0; index1 < pSubmits[index0].signalSemaphoreCount; ++index1) {
                skip |= DeviceValidateObject(queue, pSubmits[index0].pSignalSemaphores[index1], kVulkanObjectTypeSemaphore, false, "VUID-VkSubmitInfo-pSignalSemaphores-parameter", "VUID-VkSubmitInfo-commonparent");
            }
        }
    }
    skip |= DeviceValidateObject(queue, fence, kVulkanObjectTypeFence, true, "VUID-vkQueueSubmit-fence-parameter", "VUID-vkQueueSubmit-commonparent");

    return skip;
}

bool ObjectLifetimes::PreCallValidateQueueWaitIdle(
    VkQueue                                     queue) {
    bool skip = false;
    skip |= DeviceValidateObject(queue, queue, kVulkanObjectTypeQueue, false, "VUID-vkQueueWaitIdle-queue-parameter", kVUIDUndefined);

    return skip;
}

bool ObjectLifetimes::PreCallValidateDeviceWaitIdle(
    VkDevice                                    device) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkDeviceWaitIdle-device-parameter", kVUIDUndefined);

    return skip;
}

bool ObjectLifetimes::PreCallValidateAllocateMemory(
    VkDevice                                    device,
    const VkMemoryAllocateInfo*                 pAllocateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDeviceMemory*                             pMemory) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkAllocateMemory-device-parameter", kVUIDUndefined);

    return skip;
}

void ObjectLifetimes::PostCallRecordAllocateMemory(
    VkDevice                                    device,
    const VkMemoryAllocateInfo*                 pAllocateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDeviceMemory*                             pMemory) {
    DeviceCreateObject(device, *pMemory, kVulkanObjectTypeDeviceMemory, pAllocator);

}

bool ObjectLifetimes::PreCallValidateFreeMemory(
    VkDevice                                    device,
    VkDeviceMemory                              memory,
    const VkAllocationCallbacks*                pAllocator) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkFreeMemory-device-parameter", kVUIDUndefined);
    skip |= DeviceValidateObject(device, memory, kVulkanObjectTypeDeviceMemory, true, "VUID-vkFreeMemory-memory-parameter", "VUID-vkFreeMemory-memory-parent");
    skip |= DeviceValidateDestroyObject(device, memory, kVulkanObjectTypeDeviceMemory, pAllocator, kVUIDUndefined, kVUIDUndefined);

    return skip;
}

void ObjectLifetimes::PreCallRecordFreeMemory(
    VkDevice                                    device,
    VkDeviceMemory                              memory,
    const VkAllocationCallbacks*                pAllocator) {
    DeviceRecordDestroyObject(device, memory, kVulkanObjectTypeDeviceMemory);

}

bool ObjectLifetimes::PreCallValidateMapMemory(
    VkDevice                                    device,
    VkDeviceMemory                              memory,
    VkDeviceSize                                offset,
    VkDeviceSize                                size,
    VkMemoryMapFlags                            flags,
    void**                                      ppData) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkMapMemory-device-parameter", kVUIDUndefined);
    skip |= DeviceValidateObject(device, memory, kVulkanObjectTypeDeviceMemory, false, "VUID-vkMapMemory-memory-parameter", "VUID-vkMapMemory-memory-parent");

    return skip;
}

bool ObjectLifetimes::PreCallValidateUnmapMemory(
    VkDevice                                    device,
    VkDeviceMemory                              memory) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkUnmapMemory-device-parameter", kVUIDUndefined);
    skip |= DeviceValidateObject(device, memory, kVulkanObjectTypeDeviceMemory, false, "VUID-vkUnmapMemory-memory-parameter", "VUID-vkUnmapMemory-memory-parent");

    return skip;
}

bool ObjectLifetimes::PreCallValidateFlushMappedMemoryRanges(
    VkDevice                                    device,
    uint32_t                                    memoryRangeCount,
    const VkMappedMemoryRange*                  pMemoryRanges) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkFlushMappedMemoryRanges-device-parameter", kVUIDUndefined);
    if (pMemoryRanges) {
        for (uint32_t index0 = 0; index0 < memoryRangeCount; ++index0) {
            skip |= DeviceValidateObject(device, pMemoryRanges[index0].memory, kVulkanObjectTypeDeviceMemory, false, "VUID-VkMappedMemoryRange-memory-parameter", kVUIDUndefined);
        }
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateInvalidateMappedMemoryRanges(
    VkDevice                                    device,
    uint32_t                                    memoryRangeCount,
    const VkMappedMemoryRange*                  pMemoryRanges) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkInvalidateMappedMemoryRanges-device-parameter", kVUIDUndefined);
    if (pMemoryRanges) {
        for (uint32_t index0 = 0; index0 < memoryRangeCount; ++index0) {
            skip |= DeviceValidateObject(device, pMemoryRanges[index0].memory, kVulkanObjectTypeDeviceMemory, false, "VUID-VkMappedMemoryRange-memory-parameter", kVUIDUndefined);
        }
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetDeviceMemoryCommitment(
    VkDevice                                    device,
    VkDeviceMemory                              memory,
    VkDeviceSize*                               pCommittedMemoryInBytes) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkGetDeviceMemoryCommitment-device-parameter", kVUIDUndefined);
    skip |= DeviceValidateObject(device, memory, kVulkanObjectTypeDeviceMemory, false, "VUID-vkGetDeviceMemoryCommitment-memory-parameter", "VUID-vkGetDeviceMemoryCommitment-memory-parent");

    return skip;
}

bool ObjectLifetimes::PreCallValidateBindBufferMemory(
    VkDevice                                    device,
    VkBuffer                                    buffer,
    VkDeviceMemory                              memory,
    VkDeviceSize                                memoryOffset) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkBindBufferMemory-device-parameter", kVUIDUndefined);
    skip |= DeviceValidateObject(device, buffer, kVulkanObjectTypeBuffer, false, "VUID-vkBindBufferMemory-buffer-parameter", "VUID-vkBindBufferMemory-buffer-parent");
    skip |= DeviceValidateObject(device, memory, kVulkanObjectTypeDeviceMemory, false, "VUID-vkBindBufferMemory-memory-parameter", "VUID-vkBindBufferMemory-memory-parent");

    return skip;
}

bool ObjectLifetimes::PreCallValidateBindImageMemory(
    VkDevice                                    device,
    VkImage                                     image,
    VkDeviceMemory                              memory,
    VkDeviceSize                                memoryOffset) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkBindImageMemory-device-parameter", kVUIDUndefined);
    skip |= DeviceValidateObject(device, image, kVulkanObjectTypeImage, false, "VUID-vkBindImageMemory-image-parameter", "VUID-vkBindImageMemory-image-parent");
    skip |= DeviceValidateObject(device, memory, kVulkanObjectTypeDeviceMemory, false, "VUID-vkBindImageMemory-memory-parameter", "VUID-vkBindImageMemory-memory-parent");

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetBufferMemoryRequirements(
    VkDevice                                    device,
    VkBuffer                                    buffer,
    VkMemoryRequirements*                       pMemoryRequirements) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkGetBufferMemoryRequirements-device-parameter", kVUIDUndefined);
    skip |= DeviceValidateObject(device, buffer, kVulkanObjectTypeBuffer, false, "VUID-vkGetBufferMemoryRequirements-buffer-parameter", "VUID-vkGetBufferMemoryRequirements-buffer-parent");

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetImageMemoryRequirements(
    VkDevice                                    device,
    VkImage                                     image,
    VkMemoryRequirements*                       pMemoryRequirements) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkGetImageMemoryRequirements-device-parameter", kVUIDUndefined);
    skip |= DeviceValidateObject(device, image, kVulkanObjectTypeImage, false, "VUID-vkGetImageMemoryRequirements-image-parameter", "VUID-vkGetImageMemoryRequirements-image-parent");

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetImageSparseMemoryRequirements(
    VkDevice                                    device,
    VkImage                                     image,
    uint32_t*                                   pSparseMemoryRequirementCount,
    VkSparseImageMemoryRequirements*            pSparseMemoryRequirements) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkGetImageSparseMemoryRequirements-device-parameter", kVUIDUndefined);
    skip |= DeviceValidateObject(device, image, kVulkanObjectTypeImage, false, "VUID-vkGetImageSparseMemoryRequirements-image-parameter", "VUID-vkGetImageSparseMemoryRequirements-image-parent");

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetPhysicalDeviceSparseImageFormatProperties(
    VkPhysicalDevice                            physicalDevice,
    VkFormat                                    format,
    VkImageType                                 type,
    VkSampleCountFlagBits                       samples,
    VkImageUsageFlags                           usage,
    VkImageTiling                               tiling,
    uint32_t*                                   pPropertyCount,
    VkSparseImageFormatProperties*              pProperties) {
    bool skip = false;
    skip |= InstanceValidateObject(physicalDevice, physicalDevice, kVulkanObjectTypePhysicalDevice, false, "VUID-vkGetPhysicalDeviceSparseImageFormatProperties-physicalDevice-parameter", kVUIDUndefined);

    return skip;
}

bool ObjectLifetimes::PreCallValidateQueueBindSparse(
    VkQueue                                     queue,
    uint32_t                                    bindInfoCount,
    const VkBindSparseInfo*                     pBindInfo,
    VkFence                                     fence) {
    bool skip = false;
    skip |= DeviceValidateObject(queue, queue, kVulkanObjectTypeQueue, false, "VUID-vkQueueBindSparse-queue-parameter", "VUID-vkQueueBindSparse-commonparent");
    if (pBindInfo) {
        for (uint32_t index0 = 0; index0 < bindInfoCount; ++index0) {
            for (uint32_t index1 = 0; index1 < pBindInfo[index0].waitSemaphoreCount; ++index1) {
                skip |= DeviceValidateObject(queue, pBindInfo[index0].pWaitSemaphores[index1], kVulkanObjectTypeSemaphore, false, "VUID-VkBindSparseInfo-pWaitSemaphores-parameter", "VUID-VkBindSparseInfo-commonparent");
            }
            if (pBindInfo[index0].pBufferBinds) {
                for (uint32_t index1 = 0; index1 < pBindInfo[index0].bufferBindCount; ++index1) {
                    skip |= DeviceValidateObject(queue, pBindInfo[index0].pBufferBinds[index1].buffer, kVulkanObjectTypeBuffer, false, "VUID-VkSparseBufferMemoryBindInfo-buffer-parameter", kVUIDUndefined);
                    if (pBindInfo[index0].pBufferBinds[index1].pBinds) {
                        for (uint32_t index2 = 0; index2 < pBindInfo[index0].pBufferBinds[index1].bindCount; ++index2) {
                            skip |= DeviceValidateObject(queue, pBindInfo[index0].pBufferBinds[index1].pBinds[index2].memory, kVulkanObjectTypeDeviceMemory, true, "VUID-VkSparseMemoryBind-memory-parameter", kVUIDUndefined);
                        }
                    }
                }
            }
            if (pBindInfo[index0].pImageOpaqueBinds) {
                for (uint32_t index1 = 0; index1 < pBindInfo[index0].imageOpaqueBindCount; ++index1) {
                    skip |= DeviceValidateObject(queue, pBindInfo[index0].pImageOpaqueBinds[index1].image, kVulkanObjectTypeImage, false, "VUID-VkSparseImageOpaqueMemoryBindInfo-image-parameter", kVUIDUndefined);
                    if (pBindInfo[index0].pImageOpaqueBinds[index1].pBinds) {
                        for (uint32_t index2 = 0; index2 < pBindInfo[index0].pImageOpaqueBinds[index1].bindCount; ++index2) {
                            skip |= DeviceValidateObject(queue, pBindInfo[index0].pImageOpaqueBinds[index1].pBinds[index2].memory, kVulkanObjectTypeDeviceMemory, true, "VUID-VkSparseMemoryBind-memory-parameter", kVUIDUndefined);
                        }
                    }
                }
            }
            if (pBindInfo[index0].pImageBinds) {
                for (uint32_t index1 = 0; index1 < pBindInfo[index0].imageBindCount; ++index1) {
                    skip |= DeviceValidateObject(queue, pBindInfo[index0].pImageBinds[index1].image, kVulkanObjectTypeImage, false, "VUID-VkSparseImageMemoryBindInfo-image-parameter", kVUIDUndefined);
                    if (pBindInfo[index0].pImageBinds[index1].pBinds) {
                        for (uint32_t index2 = 0; index2 < pBindInfo[index0].pImageBinds[index1].bindCount; ++index2) {
                            skip |= DeviceValidateObject(queue, pBindInfo[index0].pImageBinds[index1].pBinds[index2].memory, kVulkanObjectTypeDeviceMemory, true, "VUID-VkSparseImageMemoryBind-memory-parameter", kVUIDUndefined);
                        }
                    }
                }
            }
            for (uint32_t index1 = 0; index1 < pBindInfo[index0].signalSemaphoreCount; ++index1) {
                skip |= DeviceValidateObject(queue, pBindInfo[index0].pSignalSemaphores[index1], kVulkanObjectTypeSemaphore, false, "VUID-VkBindSparseInfo-pSignalSemaphores-parameter", "VUID-VkBindSparseInfo-commonparent");
            }
        }
    }
    skip |= DeviceValidateObject(queue, fence, kVulkanObjectTypeFence, true, "VUID-vkQueueBindSparse-fence-parameter", "VUID-vkQueueBindSparse-commonparent");

    return skip;
}

bool ObjectLifetimes::PreCallValidateCreateFence(
    VkDevice                                    device,
    const VkFenceCreateInfo*                    pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkFence*                                    pFence) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkCreateFence-device-parameter", kVUIDUndefined);

    return skip;
}

void ObjectLifetimes::PostCallRecordCreateFence(
    VkDevice                                    device,
    const VkFenceCreateInfo*                    pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkFence*                                    pFence) {
    DeviceCreateObject(device, *pFence, kVulkanObjectTypeFence, pAllocator);

}

bool ObjectLifetimes::PreCallValidateDestroyFence(
    VkDevice                                    device,
    VkFence                                     fence,
    const VkAllocationCallbacks*                pAllocator) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkDestroyFence-device-parameter", kVUIDUndefined);
    skip |= DeviceValidateObject(device, fence, kVulkanObjectTypeFence, true, "VUID-vkDestroyFence-fence-parameter", "VUID-vkDestroyFence-fence-parent");
    skip |= DeviceValidateDestroyObject(device, fence, kVulkanObjectTypeFence, pAllocator, "VUID-vkDestroyFence-fence-01121", "VUID-vkDestroyFence-fence-01122");

    return skip;
}

void ObjectLifetimes::PreCallRecordDestroyFence(
    VkDevice                                    device,
    VkFence                                     fence,
    const VkAllocationCallbacks*                pAllocator) {
    DeviceRecordDestroyObject(device, fence, kVulkanObjectTypeFence);

}

bool ObjectLifetimes::PreCallValidateResetFences(
    VkDevice                                    device,
    uint32_t                                    fenceCount,
    const VkFence*                              pFences) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkResetFences-device-parameter", kVUIDUndefined);
    for (uint32_t index0 = 0; index0 < fenceCount; ++index0) {
        skip |= DeviceValidateObject(device, pFences[index0], kVulkanObjectTypeFence, false, "VUID-vkResetFences-pFences-parameter", "VUID-vkResetFences-pFences-parent");
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetFenceStatus(
    VkDevice                                    device,
    VkFence                                     fence) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkGetFenceStatus-device-parameter", kVUIDUndefined);
    skip |= DeviceValidateObject(device, fence, kVulkanObjectTypeFence, false, "VUID-vkGetFenceStatus-fence-parameter", "VUID-vkGetFenceStatus-fence-parent");

    return skip;
}

bool ObjectLifetimes::PreCallValidateWaitForFences(
    VkDevice                                    device,
    uint32_t                                    fenceCount,
    const VkFence*                              pFences,
    VkBool32                                    waitAll,
    uint64_t                                    timeout) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkWaitForFences-device-parameter", kVUIDUndefined);
    for (uint32_t index0 = 0; index0 < fenceCount; ++index0) {
        skip |= DeviceValidateObject(device, pFences[index0], kVulkanObjectTypeFence, false, "VUID-vkWaitForFences-pFences-parameter", "VUID-vkWaitForFences-pFences-parent");
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateCreateSemaphore(
    VkDevice                                    device,
    const VkSemaphoreCreateInfo*                pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSemaphore*                                pSemaphore) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkCreateSemaphore-device-parameter", kVUIDUndefined);

    return skip;
}

void ObjectLifetimes::PostCallRecordCreateSemaphore(
    VkDevice                                    device,
    const VkSemaphoreCreateInfo*                pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSemaphore*                                pSemaphore) {
    DeviceCreateObject(device, *pSemaphore, kVulkanObjectTypeSemaphore, pAllocator);

}

bool ObjectLifetimes::PreCallValidateDestroySemaphore(
    VkDevice                                    device,
    VkSemaphore                                 semaphore,
    const VkAllocationCallbacks*                pAllocator) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkDestroySemaphore-device-parameter", kVUIDUndefined);
    skip |= DeviceValidateObject(device, semaphore, kVulkanObjectTypeSemaphore, true, "VUID-vkDestroySemaphore-semaphore-parameter", "VUID-vkDestroySemaphore-semaphore-parent");
    skip |= DeviceValidateDestroyObject(device, semaphore, kVulkanObjectTypeSemaphore, pAllocator, "VUID-vkDestroySemaphore-semaphore-01138", "VUID-vkDestroySemaphore-semaphore-01139");

    return skip;
}

void ObjectLifetimes::PreCallRecordDestroySemaphore(
    VkDevice                                    device,
    VkSemaphore                                 semaphore,
    const VkAllocationCallbacks*                pAllocator) {
    DeviceRecordDestroyObject(device, semaphore, kVulkanObjectTypeSemaphore);

}

bool ObjectLifetimes::PreCallValidateCreateEvent(
    VkDevice                                    device,
    const VkEventCreateInfo*                    pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkEvent*                                    pEvent) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkCreateEvent-device-parameter", kVUIDUndefined);

    return skip;
}

void ObjectLifetimes::PostCallRecordCreateEvent(
    VkDevice                                    device,
    const VkEventCreateInfo*                    pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkEvent*                                    pEvent) {
    DeviceCreateObject(device, *pEvent, kVulkanObjectTypeEvent, pAllocator);

}

bool ObjectLifetimes::PreCallValidateDestroyEvent(
    VkDevice                                    device,
    VkEvent                                     event,
    const VkAllocationCallbacks*                pAllocator) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkDestroyEvent-device-parameter", kVUIDUndefined);
    skip |= DeviceValidateObject(device, event, kVulkanObjectTypeEvent, true, "VUID-vkDestroyEvent-event-parameter", "VUID-vkDestroyEvent-event-parent");
    skip |= DeviceValidateDestroyObject(device, event, kVulkanObjectTypeEvent, pAllocator, "VUID-vkDestroyEvent-event-01146", "VUID-vkDestroyEvent-event-01147");

    return skip;
}

void ObjectLifetimes::PreCallRecordDestroyEvent(
    VkDevice                                    device,
    VkEvent                                     event,
    const VkAllocationCallbacks*                pAllocator) {
    DeviceRecordDestroyObject(device, event, kVulkanObjectTypeEvent);

}

bool ObjectLifetimes::PreCallValidateGetEventStatus(
    VkDevice                                    device,
    VkEvent                                     event) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkGetEventStatus-device-parameter", kVUIDUndefined);
    skip |= DeviceValidateObject(device, event, kVulkanObjectTypeEvent, false, "VUID-vkGetEventStatus-event-parameter", "VUID-vkGetEventStatus-event-parent");

    return skip;
}

bool ObjectLifetimes::PreCallValidateSetEvent(
    VkDevice                                    device,
    VkEvent                                     event) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkSetEvent-device-parameter", kVUIDUndefined);
    skip |= DeviceValidateObject(device, event, kVulkanObjectTypeEvent, false, "VUID-vkSetEvent-event-parameter", "VUID-vkSetEvent-event-parent");

    return skip;
}

bool ObjectLifetimes::PreCallValidateResetEvent(
    VkDevice                                    device,
    VkEvent                                     event) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkResetEvent-device-parameter", kVUIDUndefined);
    skip |= DeviceValidateObject(device, event, kVulkanObjectTypeEvent, false, "VUID-vkResetEvent-event-parameter", "VUID-vkResetEvent-event-parent");

    return skip;
}

bool ObjectLifetimes::PreCallValidateCreateQueryPool(
    VkDevice                                    device,
    const VkQueryPoolCreateInfo*                pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkQueryPool*                                pQueryPool) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkCreateQueryPool-device-parameter", kVUIDUndefined);

    return skip;
}

void ObjectLifetimes::PostCallRecordCreateQueryPool(
    VkDevice                                    device,
    const VkQueryPoolCreateInfo*                pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkQueryPool*                                pQueryPool) {
    DeviceCreateObject(device, *pQueryPool, kVulkanObjectTypeQueryPool, pAllocator);

}

bool ObjectLifetimes::PreCallValidateDestroyQueryPool(
    VkDevice                                    device,
    VkQueryPool                                 queryPool,
    const VkAllocationCallbacks*                pAllocator) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkDestroyQueryPool-device-parameter", kVUIDUndefined);
    skip |= DeviceValidateObject(device, queryPool, kVulkanObjectTypeQueryPool, true, "VUID-vkDestroyQueryPool-queryPool-parameter", "VUID-vkDestroyQueryPool-queryPool-parent");
    skip |= DeviceValidateDestroyObject(device, queryPool, kVulkanObjectTypeQueryPool, pAllocator, "VUID-vkDestroyQueryPool-queryPool-00794", "VUID-vkDestroyQueryPool-queryPool-00795");

    return skip;
}

void ObjectLifetimes::PreCallRecordDestroyQueryPool(
    VkDevice                                    device,
    VkQueryPool                                 queryPool,
    const VkAllocationCallbacks*                pAllocator) {
    DeviceRecordDestroyObject(device, queryPool, kVulkanObjectTypeQueryPool);

}

bool ObjectLifetimes::PreCallValidateGetQueryPoolResults(
    VkDevice                                    device,
    VkQueryPool                                 queryPool,
    uint32_t                                    firstQuery,
    uint32_t                                    queryCount,
    size_t                                      dataSize,
    void*                                       pData,
    VkDeviceSize                                stride,
    VkQueryResultFlags                          flags) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkGetQueryPoolResults-device-parameter", kVUIDUndefined);
    skip |= DeviceValidateObject(device, queryPool, kVulkanObjectTypeQueryPool, false, "VUID-vkGetQueryPoolResults-queryPool-parameter", "VUID-vkGetQueryPoolResults-queryPool-parent");

    return skip;
}

bool ObjectLifetimes::PreCallValidateCreateBuffer(
    VkDevice                                    device,
    const VkBufferCreateInfo*                   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkBuffer*                                   pBuffer) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkCreateBuffer-device-parameter", kVUIDUndefined);

    return skip;
}

void ObjectLifetimes::PostCallRecordCreateBuffer(
    VkDevice                                    device,
    const VkBufferCreateInfo*                   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkBuffer*                                   pBuffer) {
    DeviceCreateObject(device, *pBuffer, kVulkanObjectTypeBuffer, pAllocator);

}

bool ObjectLifetimes::PreCallValidateDestroyBuffer(
    VkDevice                                    device,
    VkBuffer                                    buffer,
    const VkAllocationCallbacks*                pAllocator) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkDestroyBuffer-device-parameter", kVUIDUndefined);
    skip |= DeviceValidateObject(device, buffer, kVulkanObjectTypeBuffer, true, "VUID-vkDestroyBuffer-buffer-parameter", "VUID-vkDestroyBuffer-buffer-parent");
    skip |= DeviceValidateDestroyObject(device, buffer, kVulkanObjectTypeBuffer, pAllocator, "VUID-vkDestroyBuffer-buffer-00923", "VUID-vkDestroyBuffer-buffer-00924");

    return skip;
}

void ObjectLifetimes::PreCallRecordDestroyBuffer(
    VkDevice                                    device,
    VkBuffer                                    buffer,
    const VkAllocationCallbacks*                pAllocator) {
    DeviceRecordDestroyObject(device, buffer, kVulkanObjectTypeBuffer);

}

bool ObjectLifetimes::PreCallValidateCreateBufferView(
    VkDevice                                    device,
    const VkBufferViewCreateInfo*               pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkBufferView*                               pView) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkCreateBufferView-device-parameter", kVUIDUndefined);
    if (pCreateInfo) {
        skip |= DeviceValidateObject(device, pCreateInfo->buffer, kVulkanObjectTypeBuffer, false, "VUID-VkBufferViewCreateInfo-buffer-parameter", kVUIDUndefined);
    }

    return skip;
}

void ObjectLifetimes::PostCallRecordCreateBufferView(
    VkDevice                                    device,
    const VkBufferViewCreateInfo*               pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkBufferView*                               pView) {
    DeviceCreateObject(device, *pView, kVulkanObjectTypeBufferView, pAllocator);

}

bool ObjectLifetimes::PreCallValidateDestroyBufferView(
    VkDevice                                    device,
    VkBufferView                                bufferView,
    const VkAllocationCallbacks*                pAllocator) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkDestroyBufferView-device-parameter", kVUIDUndefined);
    skip |= DeviceValidateObject(device, bufferView, kVulkanObjectTypeBufferView, true, "VUID-vkDestroyBufferView-bufferView-parameter", "VUID-vkDestroyBufferView-bufferView-parent");
    skip |= DeviceValidateDestroyObject(device, bufferView, kVulkanObjectTypeBufferView, pAllocator, "VUID-vkDestroyBufferView-bufferView-00937", "VUID-vkDestroyBufferView-bufferView-00938");

    return skip;
}

void ObjectLifetimes::PreCallRecordDestroyBufferView(
    VkDevice                                    device,
    VkBufferView                                bufferView,
    const VkAllocationCallbacks*                pAllocator) {
    DeviceRecordDestroyObject(device, bufferView, kVulkanObjectTypeBufferView);

}

bool ObjectLifetimes::PreCallValidateCreateImage(
    VkDevice                                    device,
    const VkImageCreateInfo*                    pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkImage*                                    pImage) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkCreateImage-device-parameter", kVUIDUndefined);

    return skip;
}

void ObjectLifetimes::PostCallRecordCreateImage(
    VkDevice                                    device,
    const VkImageCreateInfo*                    pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkImage*                                    pImage) {
    DeviceCreateObject(device, *pImage, kVulkanObjectTypeImage, pAllocator);

}

bool ObjectLifetimes::PreCallValidateDestroyImage(
    VkDevice                                    device,
    VkImage                                     image,
    const VkAllocationCallbacks*                pAllocator) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkDestroyImage-device-parameter", kVUIDUndefined);
    skip |= DeviceValidateObject(device, image, kVulkanObjectTypeImage, true, "VUID-vkDestroyImage-image-parameter", "VUID-vkDestroyImage-image-parent");
    skip |= DeviceValidateDestroyObject(device, image, kVulkanObjectTypeImage, pAllocator, "VUID-vkDestroyImage-image-01001", "VUID-vkDestroyImage-image-01002");

    return skip;
}

void ObjectLifetimes::PreCallRecordDestroyImage(
    VkDevice                                    device,
    VkImage                                     image,
    const VkAllocationCallbacks*                pAllocator) {
    DeviceRecordDestroyObject(device, image, kVulkanObjectTypeImage);

}

bool ObjectLifetimes::PreCallValidateGetImageSubresourceLayout(
    VkDevice                                    device,
    VkImage                                     image,
    const VkImageSubresource*                   pSubresource,
    VkSubresourceLayout*                        pLayout) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkGetImageSubresourceLayout-device-parameter", kVUIDUndefined);
    skip |= DeviceValidateObject(device, image, kVulkanObjectTypeImage, false, "VUID-vkGetImageSubresourceLayout-image-parameter", "VUID-vkGetImageSubresourceLayout-image-parent");

    return skip;
}

bool ObjectLifetimes::PreCallValidateCreateImageView(
    VkDevice                                    device,
    const VkImageViewCreateInfo*                pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkImageView*                                pView) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkCreateImageView-device-parameter", kVUIDUndefined);
    if (pCreateInfo) {
        skip |= DeviceValidateObject(device, pCreateInfo->image, kVulkanObjectTypeImage, false, "VUID-VkImageViewCreateInfo-image-parameter", kVUIDUndefined);
    }

    return skip;
}

void ObjectLifetimes::PostCallRecordCreateImageView(
    VkDevice                                    device,
    const VkImageViewCreateInfo*                pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkImageView*                                pView) {
    DeviceCreateObject(device, *pView, kVulkanObjectTypeImageView, pAllocator);

}

bool ObjectLifetimes::PreCallValidateDestroyImageView(
    VkDevice                                    device,
    VkImageView                                 imageView,
    const VkAllocationCallbacks*                pAllocator) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkDestroyImageView-device-parameter", kVUIDUndefined);
    skip |= DeviceValidateObject(device, imageView, kVulkanObjectTypeImageView, true, "VUID-vkDestroyImageView-imageView-parameter", "VUID-vkDestroyImageView-imageView-parent");
    skip |= DeviceValidateDestroyObject(device, imageView, kVulkanObjectTypeImageView, pAllocator, "VUID-vkDestroyImageView-imageView-01027", "VUID-vkDestroyImageView-imageView-01028");

    return skip;
}

void ObjectLifetimes::PreCallRecordDestroyImageView(
    VkDevice                                    device,
    VkImageView                                 imageView,
    const VkAllocationCallbacks*                pAllocator) {
    DeviceRecordDestroyObject(device, imageView, kVulkanObjectTypeImageView);

}

bool ObjectLifetimes::PreCallValidateCreateShaderModule(
    VkDevice                                    device,
    const VkShaderModuleCreateInfo*             pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkShaderModule*                             pShaderModule) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkCreateShaderModule-device-parameter", kVUIDUndefined);

    return skip;
}

void ObjectLifetimes::PostCallRecordCreateShaderModule(
    VkDevice                                    device,
    const VkShaderModuleCreateInfo*             pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkShaderModule*                             pShaderModule) {
    DeviceCreateObject(device, *pShaderModule, kVulkanObjectTypeShaderModule, pAllocator);

}

bool ObjectLifetimes::PreCallValidateDestroyShaderModule(
    VkDevice                                    device,
    VkShaderModule                              shaderModule,
    const VkAllocationCallbacks*                pAllocator) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkDestroyShaderModule-device-parameter", kVUIDUndefined);
    skip |= DeviceValidateObject(device, shaderModule, kVulkanObjectTypeShaderModule, true, "VUID-vkDestroyShaderModule-shaderModule-parameter", "VUID-vkDestroyShaderModule-shaderModule-parent");
    skip |= DeviceValidateDestroyObject(device, shaderModule, kVulkanObjectTypeShaderModule, pAllocator, "VUID-vkDestroyShaderModule-shaderModule-01092", "VUID-vkDestroyShaderModule-shaderModule-01093");

    return skip;
}

void ObjectLifetimes::PreCallRecordDestroyShaderModule(
    VkDevice                                    device,
    VkShaderModule                              shaderModule,
    const VkAllocationCallbacks*                pAllocator) {
    DeviceRecordDestroyObject(device, shaderModule, kVulkanObjectTypeShaderModule);

}

bool ObjectLifetimes::PreCallValidateCreatePipelineCache(
    VkDevice                                    device,
    const VkPipelineCacheCreateInfo*            pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkPipelineCache*                            pPipelineCache) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkCreatePipelineCache-device-parameter", kVUIDUndefined);

    return skip;
}

void ObjectLifetimes::PostCallRecordCreatePipelineCache(
    VkDevice                                    device,
    const VkPipelineCacheCreateInfo*            pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkPipelineCache*                            pPipelineCache) {
    DeviceCreateObject(device, *pPipelineCache, kVulkanObjectTypePipelineCache, pAllocator);

}

bool ObjectLifetimes::PreCallValidateDestroyPipelineCache(
    VkDevice                                    device,
    VkPipelineCache                             pipelineCache,
    const VkAllocationCallbacks*                pAllocator) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkDestroyPipelineCache-device-parameter", kVUIDUndefined);
    skip |= DeviceValidateObject(device, pipelineCache, kVulkanObjectTypePipelineCache, true, "VUID-vkDestroyPipelineCache-pipelineCache-parameter", "VUID-vkDestroyPipelineCache-pipelineCache-parent");
    skip |= DeviceValidateDestroyObject(device, pipelineCache, kVulkanObjectTypePipelineCache, pAllocator, "VUID-vkDestroyPipelineCache-pipelineCache-00771", "VUID-vkDestroyPipelineCache-pipelineCache-00772");

    return skip;
}

void ObjectLifetimes::PreCallRecordDestroyPipelineCache(
    VkDevice                                    device,
    VkPipelineCache                             pipelineCache,
    const VkAllocationCallbacks*                pAllocator) {
    DeviceRecordDestroyObject(device, pipelineCache, kVulkanObjectTypePipelineCache);

}

bool ObjectLifetimes::PreCallValidateGetPipelineCacheData(
    VkDevice                                    device,
    VkPipelineCache                             pipelineCache,
    size_t*                                     pDataSize,
    void*                                       pData) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkGetPipelineCacheData-device-parameter", kVUIDUndefined);
    skip |= DeviceValidateObject(device, pipelineCache, kVulkanObjectTypePipelineCache, false, "VUID-vkGetPipelineCacheData-pipelineCache-parameter", "VUID-vkGetPipelineCacheData-pipelineCache-parent");

    return skip;
}

bool ObjectLifetimes::PreCallValidateMergePipelineCaches(
    VkDevice                                    device,
    VkPipelineCache                             dstCache,
    uint32_t                                    srcCacheCount,
    const VkPipelineCache*                      pSrcCaches) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkMergePipelineCaches-device-parameter", kVUIDUndefined);
    skip |= DeviceValidateObject(device, dstCache, kVulkanObjectTypePipelineCache, false, "VUID-vkMergePipelineCaches-dstCache-parameter", "VUID-vkMergePipelineCaches-dstCache-parent");
    for (uint32_t index0 = 0; index0 < srcCacheCount; ++index0) {
        skip |= DeviceValidateObject(device, pSrcCaches[index0], kVulkanObjectTypePipelineCache, false, "VUID-vkMergePipelineCaches-pSrcCaches-parameter", "VUID-vkMergePipelineCaches-pSrcCaches-parent");
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateCreateGraphicsPipelines(
    VkDevice                                    device,
    VkPipelineCache                             pipelineCache,
    uint32_t                                    createInfoCount,
    const VkGraphicsPipelineCreateInfo*         pCreateInfos,
    const VkAllocationCallbacks*                pAllocator,
    VkPipeline*                                 pPipelines) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkCreateGraphicsPipelines-device-parameter", kVUIDUndefined);
    skip |= DeviceValidateObject(device, pipelineCache, kVulkanObjectTypePipelineCache, true, "VUID-vkCreateGraphicsPipelines-pipelineCache-parameter", "VUID-vkCreateGraphicsPipelines-pipelineCache-parent");
    if (pCreateInfos) {
        for (uint32_t index0 = 0; index0 < createInfoCount; ++index0) {
            if (pCreateInfos[index0].pStages) {
                for (uint32_t index1 = 0; index1 < pCreateInfos[index0].stageCount; ++index1) {
                    skip |= DeviceValidateObject(device, pCreateInfos[index0].pStages[index1].module, kVulkanObjectTypeShaderModule, false, "VUID-VkPipelineShaderStageCreateInfo-module-parameter", kVUIDUndefined);
                }
            }
            skip |= DeviceValidateObject(device, pCreateInfos[index0].layout, kVulkanObjectTypePipelineLayout, false, "VUID-VkGraphicsPipelineCreateInfo-layout-parameter", "VUID-VkGraphicsPipelineCreateInfo-commonparent");
            skip |= DeviceValidateObject(device, pCreateInfos[index0].renderPass, kVulkanObjectTypeRenderPass, false, "VUID-VkGraphicsPipelineCreateInfo-renderPass-parameter", "VUID-VkGraphicsPipelineCreateInfo-commonparent");
            skip |= DeviceValidateObject(device, pCreateInfos[index0].basePipelineHandle, kVulkanObjectTypePipeline, true, kVUIDUndefined, "VUID-VkGraphicsPipelineCreateInfo-commonparent");
        }
    }

    return skip;
}

void ObjectLifetimes::PostCallRecordCreateGraphicsPipelines(
    VkDevice                                    device,
    VkPipelineCache                             pipelineCache,
    uint32_t                                    createInfoCount,
    const VkGraphicsPipelineCreateInfo*         pCreateInfos,
    const VkAllocationCallbacks*                pAllocator,
    VkPipeline*                                 pPipelines) {
    for (uint32_t index = 0; index < createInfoCount; index++) {
        DeviceCreateObject(device, pPipelines[index], kVulkanObjectTypePipeline, pAllocator);
    }

}

bool ObjectLifetimes::PreCallValidateCreateComputePipelines(
    VkDevice                                    device,
    VkPipelineCache                             pipelineCache,
    uint32_t                                    createInfoCount,
    const VkComputePipelineCreateInfo*          pCreateInfos,
    const VkAllocationCallbacks*                pAllocator,
    VkPipeline*                                 pPipelines) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkCreateComputePipelines-device-parameter", kVUIDUndefined);
    skip |= DeviceValidateObject(device, pipelineCache, kVulkanObjectTypePipelineCache, true, "VUID-vkCreateComputePipelines-pipelineCache-parameter", "VUID-vkCreateComputePipelines-pipelineCache-parent");
    if (pCreateInfos) {
        for (uint32_t index0 = 0; index0 < createInfoCount; ++index0) {
            skip |= DeviceValidateObject(device, pCreateInfos[index0].layout, kVulkanObjectTypePipelineLayout, false, "VUID-VkComputePipelineCreateInfo-layout-parameter", "VUID-VkComputePipelineCreateInfo-commonparent");
            skip |= DeviceValidateObject(device, pCreateInfos[index0].basePipelineHandle, kVulkanObjectTypePipeline, true, kVUIDUndefined, "VUID-VkComputePipelineCreateInfo-commonparent");
        }
    }

    return skip;
}

void ObjectLifetimes::PostCallRecordCreateComputePipelines(
    VkDevice                                    device,
    VkPipelineCache                             pipelineCache,
    uint32_t                                    createInfoCount,
    const VkComputePipelineCreateInfo*          pCreateInfos,
    const VkAllocationCallbacks*                pAllocator,
    VkPipeline*                                 pPipelines) {
    for (uint32_t index = 0; index < createInfoCount; index++) {
        DeviceCreateObject(device, pPipelines[index], kVulkanObjectTypePipeline, pAllocator);
    }

}

bool ObjectLifetimes::PreCallValidateDestroyPipeline(
    VkDevice                                    device,
    VkPipeline                                  pipeline,
    const VkAllocationCallbacks*                pAllocator) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkDestroyPipeline-device-parameter", kVUIDUndefined);
    skip |= DeviceValidateObject(device, pipeline, kVulkanObjectTypePipeline, true, "VUID-vkDestroyPipeline-pipeline-parameter", "VUID-vkDestroyPipeline-pipeline-parent");
    skip |= DeviceValidateDestroyObject(device, pipeline, kVulkanObjectTypePipeline, pAllocator, "VUID-vkDestroyPipeline-pipeline-00766", "VUID-vkDestroyPipeline-pipeline-00767");

    return skip;
}

void ObjectLifetimes::PreCallRecordDestroyPipeline(
    VkDevice                                    device,
    VkPipeline                                  pipeline,
    const VkAllocationCallbacks*                pAllocator) {
    DeviceRecordDestroyObject(device, pipeline, kVulkanObjectTypePipeline);

}

bool ObjectLifetimes::PreCallValidateCreatePipelineLayout(
    VkDevice                                    device,
    const VkPipelineLayoutCreateInfo*           pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkPipelineLayout*                           pPipelineLayout) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkCreatePipelineLayout-device-parameter", kVUIDUndefined);
    if (pCreateInfo) {
        for (uint32_t index1 = 0; index1 < pCreateInfo->setLayoutCount; ++index1) {
            skip |= DeviceValidateObject(device, pCreateInfo->pSetLayouts[index1], kVulkanObjectTypeDescriptorSetLayout, false, "VUID-VkPipelineLayoutCreateInfo-pSetLayouts-parameter", kVUIDUndefined);
        }
    }

    return skip;
}

void ObjectLifetimes::PostCallRecordCreatePipelineLayout(
    VkDevice                                    device,
    const VkPipelineLayoutCreateInfo*           pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkPipelineLayout*                           pPipelineLayout) {
    DeviceCreateObject(device, *pPipelineLayout, kVulkanObjectTypePipelineLayout, pAllocator);

}

bool ObjectLifetimes::PreCallValidateDestroyPipelineLayout(
    VkDevice                                    device,
    VkPipelineLayout                            pipelineLayout,
    const VkAllocationCallbacks*                pAllocator) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkDestroyPipelineLayout-device-parameter", kVUIDUndefined);
    skip |= DeviceValidateObject(device, pipelineLayout, kVulkanObjectTypePipelineLayout, true, "VUID-vkDestroyPipelineLayout-pipelineLayout-parameter", "VUID-vkDestroyPipelineLayout-pipelineLayout-parent");
    skip |= DeviceValidateDestroyObject(device, pipelineLayout, kVulkanObjectTypePipelineLayout, pAllocator, "VUID-vkDestroyPipelineLayout-pipelineLayout-00299", "VUID-vkDestroyPipelineLayout-pipelineLayout-00300");

    return skip;
}

void ObjectLifetimes::PreCallRecordDestroyPipelineLayout(
    VkDevice                                    device,
    VkPipelineLayout                            pipelineLayout,
    const VkAllocationCallbacks*                pAllocator) {
    DeviceRecordDestroyObject(device, pipelineLayout, kVulkanObjectTypePipelineLayout);

}

bool ObjectLifetimes::PreCallValidateCreateSampler(
    VkDevice                                    device,
    const VkSamplerCreateInfo*                  pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSampler*                                  pSampler) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkCreateSampler-device-parameter", kVUIDUndefined);

    return skip;
}

void ObjectLifetimes::PostCallRecordCreateSampler(
    VkDevice                                    device,
    const VkSamplerCreateInfo*                  pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSampler*                                  pSampler) {
    DeviceCreateObject(device, *pSampler, kVulkanObjectTypeSampler, pAllocator);

}

bool ObjectLifetimes::PreCallValidateDestroySampler(
    VkDevice                                    device,
    VkSampler                                   sampler,
    const VkAllocationCallbacks*                pAllocator) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkDestroySampler-device-parameter", kVUIDUndefined);
    skip |= DeviceValidateObject(device, sampler, kVulkanObjectTypeSampler, true, "VUID-vkDestroySampler-sampler-parameter", "VUID-vkDestroySampler-sampler-parent");
    skip |= DeviceValidateDestroyObject(device, sampler, kVulkanObjectTypeSampler, pAllocator, "VUID-vkDestroySampler-sampler-01083", "VUID-vkDestroySampler-sampler-01084");

    return skip;
}

void ObjectLifetimes::PreCallRecordDestroySampler(
    VkDevice                                    device,
    VkSampler                                   sampler,
    const VkAllocationCallbacks*                pAllocator) {
    DeviceRecordDestroyObject(device, sampler, kVulkanObjectTypeSampler);

}

bool ObjectLifetimes::PreCallValidateDestroyDescriptorSetLayout(
    VkDevice                                    device,
    VkDescriptorSetLayout                       descriptorSetLayout,
    const VkAllocationCallbacks*                pAllocator) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkDestroyDescriptorSetLayout-device-parameter", kVUIDUndefined);
    skip |= DeviceValidateObject(device, descriptorSetLayout, kVulkanObjectTypeDescriptorSetLayout, true, "VUID-vkDestroyDescriptorSetLayout-descriptorSetLayout-parameter", "VUID-vkDestroyDescriptorSetLayout-descriptorSetLayout-parent");
    skip |= DeviceValidateDestroyObject(device, descriptorSetLayout, kVulkanObjectTypeDescriptorSetLayout, pAllocator, "VUID-vkDestroyDescriptorSetLayout-descriptorSetLayout-00284", "VUID-vkDestroyDescriptorSetLayout-descriptorSetLayout-00285");

    return skip;
}

void ObjectLifetimes::PreCallRecordDestroyDescriptorSetLayout(
    VkDevice                                    device,
    VkDescriptorSetLayout                       descriptorSetLayout,
    const VkAllocationCallbacks*                pAllocator) {
    DeviceRecordDestroyObject(device, descriptorSetLayout, kVulkanObjectTypeDescriptorSetLayout);

}

bool ObjectLifetimes::PreCallValidateCreateDescriptorPool(
    VkDevice                                    device,
    const VkDescriptorPoolCreateInfo*           pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDescriptorPool*                           pDescriptorPool) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkCreateDescriptorPool-device-parameter", kVUIDUndefined);

    return skip;
}

void ObjectLifetimes::PostCallRecordCreateDescriptorPool(
    VkDevice                                    device,
    const VkDescriptorPoolCreateInfo*           pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDescriptorPool*                           pDescriptorPool) {
    DeviceCreateObject(device, *pDescriptorPool, kVulkanObjectTypeDescriptorPool, pAllocator);

}

bool ObjectLifetimes::PreCallValidateCreateFramebuffer(
    VkDevice                                    device,
    const VkFramebufferCreateInfo*              pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkFramebuffer*                              pFramebuffer) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkCreateFramebuffer-device-parameter", kVUIDUndefined);
    if (pCreateInfo) {
        skip |= DeviceValidateObject(device, pCreateInfo->renderPass, kVulkanObjectTypeRenderPass, false, "VUID-VkFramebufferCreateInfo-renderPass-parameter", "VUID-VkFramebufferCreateInfo-commonparent");
        for (uint32_t index1 = 0; index1 < pCreateInfo->attachmentCount; ++index1) {
            skip |= DeviceValidateObject(device, pCreateInfo->pAttachments[index1], kVulkanObjectTypeImageView, false, "VUID-VkFramebufferCreateInfo-pAttachments-parameter", "VUID-VkFramebufferCreateInfo-commonparent");
        }
    }

    return skip;
}

void ObjectLifetimes::PostCallRecordCreateFramebuffer(
    VkDevice                                    device,
    const VkFramebufferCreateInfo*              pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkFramebuffer*                              pFramebuffer) {
    DeviceCreateObject(device, *pFramebuffer, kVulkanObjectTypeFramebuffer, pAllocator);

}

bool ObjectLifetimes::PreCallValidateDestroyFramebuffer(
    VkDevice                                    device,
    VkFramebuffer                               framebuffer,
    const VkAllocationCallbacks*                pAllocator) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkDestroyFramebuffer-device-parameter", kVUIDUndefined);
    skip |= DeviceValidateObject(device, framebuffer, kVulkanObjectTypeFramebuffer, true, "VUID-vkDestroyFramebuffer-framebuffer-parameter", "VUID-vkDestroyFramebuffer-framebuffer-parent");
    skip |= DeviceValidateDestroyObject(device, framebuffer, kVulkanObjectTypeFramebuffer, pAllocator, "VUID-vkDestroyFramebuffer-framebuffer-00893", "VUID-vkDestroyFramebuffer-framebuffer-00894");

    return skip;
}

void ObjectLifetimes::PreCallRecordDestroyFramebuffer(
    VkDevice                                    device,
    VkFramebuffer                               framebuffer,
    const VkAllocationCallbacks*                pAllocator) {
    DeviceRecordDestroyObject(device, framebuffer, kVulkanObjectTypeFramebuffer);

}

bool ObjectLifetimes::PreCallValidateCreateRenderPass(
    VkDevice                                    device,
    const VkRenderPassCreateInfo*               pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkRenderPass*                               pRenderPass) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkCreateRenderPass-device-parameter", kVUIDUndefined);

    return skip;
}

void ObjectLifetimes::PostCallRecordCreateRenderPass(
    VkDevice                                    device,
    const VkRenderPassCreateInfo*               pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkRenderPass*                               pRenderPass) {
    DeviceCreateObject(device, *pRenderPass, kVulkanObjectTypeRenderPass, pAllocator);

}

bool ObjectLifetimes::PreCallValidateDestroyRenderPass(
    VkDevice                                    device,
    VkRenderPass                                renderPass,
    const VkAllocationCallbacks*                pAllocator) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkDestroyRenderPass-device-parameter", kVUIDUndefined);
    skip |= DeviceValidateObject(device, renderPass, kVulkanObjectTypeRenderPass, true, "VUID-vkDestroyRenderPass-renderPass-parameter", "VUID-vkDestroyRenderPass-renderPass-parent");
    skip |= DeviceValidateDestroyObject(device, renderPass, kVulkanObjectTypeRenderPass, pAllocator, "VUID-vkDestroyRenderPass-renderPass-00874", "VUID-vkDestroyRenderPass-renderPass-00875");

    return skip;
}

void ObjectLifetimes::PreCallRecordDestroyRenderPass(
    VkDevice                                    device,
    VkRenderPass                                renderPass,
    const VkAllocationCallbacks*                pAllocator) {
    DeviceRecordDestroyObject(device, renderPass, kVulkanObjectTypeRenderPass);

}

bool ObjectLifetimes::PreCallValidateGetRenderAreaGranularity(
    VkDevice                                    device,
    VkRenderPass                                renderPass,
    VkExtent2D*                                 pGranularity) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkGetRenderAreaGranularity-device-parameter", kVUIDUndefined);
    skip |= DeviceValidateObject(device, renderPass, kVulkanObjectTypeRenderPass, false, "VUID-vkGetRenderAreaGranularity-renderPass-parameter", "VUID-vkGetRenderAreaGranularity-renderPass-parent");

    return skip;
}

bool ObjectLifetimes::PreCallValidateCreateCommandPool(
    VkDevice                                    device,
    const VkCommandPoolCreateInfo*              pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkCommandPool*                              pCommandPool) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkCreateCommandPool-device-parameter", kVUIDUndefined);

    return skip;
}

void ObjectLifetimes::PostCallRecordCreateCommandPool(
    VkDevice                                    device,
    const VkCommandPoolCreateInfo*              pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkCommandPool*                              pCommandPool) {
    DeviceCreateObject(device, *pCommandPool, kVulkanObjectTypeCommandPool, pAllocator);

}

bool ObjectLifetimes::PreCallValidateResetCommandPool(
    VkDevice                                    device,
    VkCommandPool                               commandPool,
    VkCommandPoolResetFlags                     flags) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkResetCommandPool-device-parameter", kVUIDUndefined);
    skip |= DeviceValidateObject(device, commandPool, kVulkanObjectTypeCommandPool, false, "VUID-vkResetCommandPool-commandPool-parameter", "VUID-vkResetCommandPool-commandPool-parent");

    return skip;
}

bool ObjectLifetimes::PreCallValidateEndCommandBuffer(
    VkCommandBuffer                             commandBuffer) {
    bool skip = false;
    skip |= DeviceValidateObject(commandBuffer, commandBuffer, kVulkanObjectTypeCommandBuffer, false, "VUID-vkEndCommandBuffer-commandBuffer-parameter", kVUIDUndefined);

    return skip;
}

bool ObjectLifetimes::PreCallValidateResetCommandBuffer(
    VkCommandBuffer                             commandBuffer,
    VkCommandBufferResetFlags                   flags) {
    bool skip = false;
    skip |= DeviceValidateObject(commandBuffer, commandBuffer, kVulkanObjectTypeCommandBuffer, false, "VUID-vkResetCommandBuffer-commandBuffer-parameter", kVUIDUndefined);

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdBindPipeline(
    VkCommandBuffer                             commandBuffer,
    VkPipelineBindPoint                         pipelineBindPoint,
    VkPipeline                                  pipeline) {
    bool skip = false;
    skip |= DeviceValidateObject(commandBuffer, commandBuffer, kVulkanObjectTypeCommandBuffer, false, "VUID-vkCmdBindPipeline-commandBuffer-parameter", "VUID-vkCmdBindPipeline-commonparent");
    skip |= DeviceValidateObject(commandBuffer, pipeline, kVulkanObjectTypePipeline, false, "VUID-vkCmdBindPipeline-pipeline-parameter", "VUID-vkCmdBindPipeline-commonparent");

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdSetViewport(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstViewport,
    uint32_t                                    viewportCount,
    const VkViewport*                           pViewports) {
    bool skip = false;
    skip |= DeviceValidateObject(commandBuffer, commandBuffer, kVulkanObjectTypeCommandBuffer, false, "VUID-vkCmdSetViewport-commandBuffer-parameter", kVUIDUndefined);

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdSetScissor(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstScissor,
    uint32_t                                    scissorCount,
    const VkRect2D*                             pScissors) {
    bool skip = false;
    skip |= DeviceValidateObject(commandBuffer, commandBuffer, kVulkanObjectTypeCommandBuffer, false, "VUID-vkCmdSetScissor-commandBuffer-parameter", kVUIDUndefined);

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdSetLineWidth(
    VkCommandBuffer                             commandBuffer,
    float                                       lineWidth) {
    bool skip = false;
    skip |= DeviceValidateObject(commandBuffer, commandBuffer, kVulkanObjectTypeCommandBuffer, false, "VUID-vkCmdSetLineWidth-commandBuffer-parameter", kVUIDUndefined);

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdSetDepthBias(
    VkCommandBuffer                             commandBuffer,
    float                                       depthBiasConstantFactor,
    float                                       depthBiasClamp,
    float                                       depthBiasSlopeFactor) {
    bool skip = false;
    skip |= DeviceValidateObject(commandBuffer, commandBuffer, kVulkanObjectTypeCommandBuffer, false, "VUID-vkCmdSetDepthBias-commandBuffer-parameter", kVUIDUndefined);

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdSetBlendConstants(
    VkCommandBuffer                             commandBuffer,
    const float                                 blendConstants[4]) {
    bool skip = false;
    skip |= DeviceValidateObject(commandBuffer, commandBuffer, kVulkanObjectTypeCommandBuffer, false, "VUID-vkCmdSetBlendConstants-commandBuffer-parameter", kVUIDUndefined);

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdSetDepthBounds(
    VkCommandBuffer                             commandBuffer,
    float                                       minDepthBounds,
    float                                       maxDepthBounds) {
    bool skip = false;
    skip |= DeviceValidateObject(commandBuffer, commandBuffer, kVulkanObjectTypeCommandBuffer, false, "VUID-vkCmdSetDepthBounds-commandBuffer-parameter", kVUIDUndefined);

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdSetStencilCompareMask(
    VkCommandBuffer                             commandBuffer,
    VkStencilFaceFlags                          faceMask,
    uint32_t                                    compareMask) {
    bool skip = false;
    skip |= DeviceValidateObject(commandBuffer, commandBuffer, kVulkanObjectTypeCommandBuffer, false, "VUID-vkCmdSetStencilCompareMask-commandBuffer-parameter", kVUIDUndefined);

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdSetStencilWriteMask(
    VkCommandBuffer                             commandBuffer,
    VkStencilFaceFlags                          faceMask,
    uint32_t                                    writeMask) {
    bool skip = false;
    skip |= DeviceValidateObject(commandBuffer, commandBuffer, kVulkanObjectTypeCommandBuffer, false, "VUID-vkCmdSetStencilWriteMask-commandBuffer-parameter", kVUIDUndefined);

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdSetStencilReference(
    VkCommandBuffer                             commandBuffer,
    VkStencilFaceFlags                          faceMask,
    uint32_t                                    reference) {
    bool skip = false;
    skip |= DeviceValidateObject(commandBuffer, commandBuffer, kVulkanObjectTypeCommandBuffer, false, "VUID-vkCmdSetStencilReference-commandBuffer-parameter", kVUIDUndefined);

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdBindDescriptorSets(
    VkCommandBuffer                             commandBuffer,
    VkPipelineBindPoint                         pipelineBindPoint,
    VkPipelineLayout                            layout,
    uint32_t                                    firstSet,
    uint32_t                                    descriptorSetCount,
    const VkDescriptorSet*                      pDescriptorSets,
    uint32_t                                    dynamicOffsetCount,
    const uint32_t*                             pDynamicOffsets) {
    bool skip = false;
    skip |= DeviceValidateObject(commandBuffer, commandBuffer, kVulkanObjectTypeCommandBuffer, false, "VUID-vkCmdBindDescriptorSets-commandBuffer-parameter", "VUID-vkCmdBindDescriptorSets-commonparent");
    skip |= DeviceValidateObject(commandBuffer, layout, kVulkanObjectTypePipelineLayout, false, "VUID-vkCmdBindDescriptorSets-layout-parameter", "VUID-vkCmdBindDescriptorSets-commonparent");
    for (uint32_t index0 = 0; index0 < descriptorSetCount; ++index0) {
        skip |= DeviceValidateObject(commandBuffer, pDescriptorSets[index0], kVulkanObjectTypeDescriptorSet, false, "VUID-vkCmdBindDescriptorSets-pDescriptorSets-parameter", "VUID-vkCmdBindDescriptorSets-commonparent");
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdBindIndexBuffer(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkIndexType                                 indexType) {
    bool skip = false;
    skip |= DeviceValidateObject(commandBuffer, commandBuffer, kVulkanObjectTypeCommandBuffer, false, "VUID-vkCmdBindIndexBuffer-commandBuffer-parameter", "VUID-vkCmdBindIndexBuffer-commonparent");
    skip |= DeviceValidateObject(commandBuffer, buffer, kVulkanObjectTypeBuffer, false, "VUID-vkCmdBindIndexBuffer-buffer-parameter", "VUID-vkCmdBindIndexBuffer-commonparent");

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdBindVertexBuffers(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstBinding,
    uint32_t                                    bindingCount,
    const VkBuffer*                             pBuffers,
    const VkDeviceSize*                         pOffsets) {
    bool skip = false;
    skip |= DeviceValidateObject(commandBuffer, commandBuffer, kVulkanObjectTypeCommandBuffer, false, "VUID-vkCmdBindVertexBuffers-commandBuffer-parameter", "VUID-vkCmdBindVertexBuffers-commonparent");
    for (uint32_t index0 = 0; index0 < bindingCount; ++index0) {
        skip |= DeviceValidateObject(commandBuffer, pBuffers[index0], kVulkanObjectTypeBuffer, false, "VUID-vkCmdBindVertexBuffers-pBuffers-parameter", "VUID-vkCmdBindVertexBuffers-commonparent");
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdDraw(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    vertexCount,
    uint32_t                                    instanceCount,
    uint32_t                                    firstVertex,
    uint32_t                                    firstInstance) {
    bool skip = false;
    skip |= DeviceValidateObject(commandBuffer, commandBuffer, kVulkanObjectTypeCommandBuffer, false, "VUID-vkCmdDraw-commandBuffer-parameter", kVUIDUndefined);

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdDrawIndexed(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    indexCount,
    uint32_t                                    instanceCount,
    uint32_t                                    firstIndex,
    int32_t                                     vertexOffset,
    uint32_t                                    firstInstance) {
    bool skip = false;
    skip |= DeviceValidateObject(commandBuffer, commandBuffer, kVulkanObjectTypeCommandBuffer, false, "VUID-vkCmdDrawIndexed-commandBuffer-parameter", kVUIDUndefined);

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdDrawIndirect(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    uint32_t                                    drawCount,
    uint32_t                                    stride) {
    bool skip = false;
    skip |= DeviceValidateObject(commandBuffer, commandBuffer, kVulkanObjectTypeCommandBuffer, false, "VUID-vkCmdDrawIndirect-commandBuffer-parameter", "VUID-vkCmdDrawIndirect-commonparent");
    skip |= DeviceValidateObject(commandBuffer, buffer, kVulkanObjectTypeBuffer, false, "VUID-vkCmdDrawIndirect-buffer-parameter", "VUID-vkCmdDrawIndirect-commonparent");

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdDrawIndexedIndirect(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    uint32_t                                    drawCount,
    uint32_t                                    stride) {
    bool skip = false;
    skip |= DeviceValidateObject(commandBuffer, commandBuffer, kVulkanObjectTypeCommandBuffer, false, "VUID-vkCmdDrawIndexedIndirect-commandBuffer-parameter", "VUID-vkCmdDrawIndexedIndirect-commonparent");
    skip |= DeviceValidateObject(commandBuffer, buffer, kVulkanObjectTypeBuffer, false, "VUID-vkCmdDrawIndexedIndirect-buffer-parameter", "VUID-vkCmdDrawIndexedIndirect-commonparent");

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdDispatch(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    groupCountX,
    uint32_t                                    groupCountY,
    uint32_t                                    groupCountZ) {
    bool skip = false;
    skip |= DeviceValidateObject(commandBuffer, commandBuffer, kVulkanObjectTypeCommandBuffer, false, "VUID-vkCmdDispatch-commandBuffer-parameter", kVUIDUndefined);

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdDispatchIndirect(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset) {
    bool skip = false;
    skip |= DeviceValidateObject(commandBuffer, commandBuffer, kVulkanObjectTypeCommandBuffer, false, "VUID-vkCmdDispatchIndirect-commandBuffer-parameter", "VUID-vkCmdDispatchIndirect-commonparent");
    skip |= DeviceValidateObject(commandBuffer, buffer, kVulkanObjectTypeBuffer, false, "VUID-vkCmdDispatchIndirect-buffer-parameter", "VUID-vkCmdDispatchIndirect-commonparent");

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdCopyBuffer(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    srcBuffer,
    VkBuffer                                    dstBuffer,
    uint32_t                                    regionCount,
    const VkBufferCopy*                         pRegions) {
    bool skip = false;
    skip |= DeviceValidateObject(commandBuffer, commandBuffer, kVulkanObjectTypeCommandBuffer, false, "VUID-vkCmdCopyBuffer-commandBuffer-parameter", "VUID-vkCmdCopyBuffer-commonparent");
    skip |= DeviceValidateObject(commandBuffer, srcBuffer, kVulkanObjectTypeBuffer, false, "VUID-vkCmdCopyBuffer-srcBuffer-parameter", "VUID-vkCmdCopyBuffer-commonparent");
    skip |= DeviceValidateObject(commandBuffer, dstBuffer, kVulkanObjectTypeBuffer, false, "VUID-vkCmdCopyBuffer-dstBuffer-parameter", "VUID-vkCmdCopyBuffer-commonparent");

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdCopyImage(
    VkCommandBuffer                             commandBuffer,
    VkImage                                     srcImage,
    VkImageLayout                               srcImageLayout,
    VkImage                                     dstImage,
    VkImageLayout                               dstImageLayout,
    uint32_t                                    regionCount,
    const VkImageCopy*                          pRegions) {
    bool skip = false;
    skip |= DeviceValidateObject(commandBuffer, commandBuffer, kVulkanObjectTypeCommandBuffer, false, "VUID-vkCmdCopyImage-commandBuffer-parameter", "VUID-vkCmdCopyImage-commonparent");
    skip |= DeviceValidateObject(commandBuffer, srcImage, kVulkanObjectTypeImage, false, "VUID-vkCmdCopyImage-srcImage-parameter", "VUID-vkCmdCopyImage-commonparent");
    skip |= DeviceValidateObject(commandBuffer, dstImage, kVulkanObjectTypeImage, false, "VUID-vkCmdCopyImage-dstImage-parameter", "VUID-vkCmdCopyImage-commonparent");

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdBlitImage(
    VkCommandBuffer                             commandBuffer,
    VkImage                                     srcImage,
    VkImageLayout                               srcImageLayout,
    VkImage                                     dstImage,
    VkImageLayout                               dstImageLayout,
    uint32_t                                    regionCount,
    const VkImageBlit*                          pRegions,
    VkFilter                                    filter) {
    bool skip = false;
    skip |= DeviceValidateObject(commandBuffer, commandBuffer, kVulkanObjectTypeCommandBuffer, false, "VUID-vkCmdBlitImage-commandBuffer-parameter", "VUID-vkCmdBlitImage-commonparent");
    skip |= DeviceValidateObject(commandBuffer, srcImage, kVulkanObjectTypeImage, false, "VUID-vkCmdBlitImage-srcImage-parameter", "VUID-vkCmdBlitImage-commonparent");
    skip |= DeviceValidateObject(commandBuffer, dstImage, kVulkanObjectTypeImage, false, "VUID-vkCmdBlitImage-dstImage-parameter", "VUID-vkCmdBlitImage-commonparent");

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdCopyBufferToImage(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    srcBuffer,
    VkImage                                     dstImage,
    VkImageLayout                               dstImageLayout,
    uint32_t                                    regionCount,
    const VkBufferImageCopy*                    pRegions) {
    bool skip = false;
    skip |= DeviceValidateObject(commandBuffer, commandBuffer, kVulkanObjectTypeCommandBuffer, false, "VUID-vkCmdCopyBufferToImage-commandBuffer-parameter", "VUID-vkCmdCopyBufferToImage-commonparent");
    skip |= DeviceValidateObject(commandBuffer, srcBuffer, kVulkanObjectTypeBuffer, false, "VUID-vkCmdCopyBufferToImage-srcBuffer-parameter", "VUID-vkCmdCopyBufferToImage-commonparent");
    skip |= DeviceValidateObject(commandBuffer, dstImage, kVulkanObjectTypeImage, false, "VUID-vkCmdCopyBufferToImage-dstImage-parameter", "VUID-vkCmdCopyBufferToImage-commonparent");

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdCopyImageToBuffer(
    VkCommandBuffer                             commandBuffer,
    VkImage                                     srcImage,
    VkImageLayout                               srcImageLayout,
    VkBuffer                                    dstBuffer,
    uint32_t                                    regionCount,
    const VkBufferImageCopy*                    pRegions) {
    bool skip = false;
    skip |= DeviceValidateObject(commandBuffer, commandBuffer, kVulkanObjectTypeCommandBuffer, false, "VUID-vkCmdCopyImageToBuffer-commandBuffer-parameter", "VUID-vkCmdCopyImageToBuffer-commonparent");
    skip |= DeviceValidateObject(commandBuffer, srcImage, kVulkanObjectTypeImage, false, "VUID-vkCmdCopyImageToBuffer-srcImage-parameter", "VUID-vkCmdCopyImageToBuffer-commonparent");
    skip |= DeviceValidateObject(commandBuffer, dstBuffer, kVulkanObjectTypeBuffer, false, "VUID-vkCmdCopyImageToBuffer-dstBuffer-parameter", "VUID-vkCmdCopyImageToBuffer-commonparent");

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdUpdateBuffer(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    dstBuffer,
    VkDeviceSize                                dstOffset,
    VkDeviceSize                                dataSize,
    const void*                                 pData) {
    bool skip = false;
    skip |= DeviceValidateObject(commandBuffer, commandBuffer, kVulkanObjectTypeCommandBuffer, false, "VUID-vkCmdUpdateBuffer-commandBuffer-parameter", "VUID-vkCmdUpdateBuffer-commonparent");
    skip |= DeviceValidateObject(commandBuffer, dstBuffer, kVulkanObjectTypeBuffer, false, "VUID-vkCmdUpdateBuffer-dstBuffer-parameter", "VUID-vkCmdUpdateBuffer-commonparent");

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdFillBuffer(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    dstBuffer,
    VkDeviceSize                                dstOffset,
    VkDeviceSize                                size,
    uint32_t                                    data) {
    bool skip = false;
    skip |= DeviceValidateObject(commandBuffer, commandBuffer, kVulkanObjectTypeCommandBuffer, false, "VUID-vkCmdFillBuffer-commandBuffer-parameter", "VUID-vkCmdFillBuffer-commonparent");
    skip |= DeviceValidateObject(commandBuffer, dstBuffer, kVulkanObjectTypeBuffer, false, "VUID-vkCmdFillBuffer-dstBuffer-parameter", "VUID-vkCmdFillBuffer-commonparent");

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdClearColorImage(
    VkCommandBuffer                             commandBuffer,
    VkImage                                     image,
    VkImageLayout                               imageLayout,
    const VkClearColorValue*                    pColor,
    uint32_t                                    rangeCount,
    const VkImageSubresourceRange*              pRanges) {
    bool skip = false;
    skip |= DeviceValidateObject(commandBuffer, commandBuffer, kVulkanObjectTypeCommandBuffer, false, "VUID-vkCmdClearColorImage-commandBuffer-parameter", "VUID-vkCmdClearColorImage-commonparent");
    skip |= DeviceValidateObject(commandBuffer, image, kVulkanObjectTypeImage, false, "VUID-vkCmdClearColorImage-image-parameter", "VUID-vkCmdClearColorImage-commonparent");

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdClearDepthStencilImage(
    VkCommandBuffer                             commandBuffer,
    VkImage                                     image,
    VkImageLayout                               imageLayout,
    const VkClearDepthStencilValue*             pDepthStencil,
    uint32_t                                    rangeCount,
    const VkImageSubresourceRange*              pRanges) {
    bool skip = false;
    skip |= DeviceValidateObject(commandBuffer, commandBuffer, kVulkanObjectTypeCommandBuffer, false, "VUID-vkCmdClearDepthStencilImage-commandBuffer-parameter", "VUID-vkCmdClearDepthStencilImage-commonparent");
    skip |= DeviceValidateObject(commandBuffer, image, kVulkanObjectTypeImage, false, "VUID-vkCmdClearDepthStencilImage-image-parameter", "VUID-vkCmdClearDepthStencilImage-commonparent");

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdClearAttachments(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    attachmentCount,
    const VkClearAttachment*                    pAttachments,
    uint32_t                                    rectCount,
    const VkClearRect*                          pRects) {
    bool skip = false;
    skip |= DeviceValidateObject(commandBuffer, commandBuffer, kVulkanObjectTypeCommandBuffer, false, "VUID-vkCmdClearAttachments-commandBuffer-parameter", kVUIDUndefined);

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdResolveImage(
    VkCommandBuffer                             commandBuffer,
    VkImage                                     srcImage,
    VkImageLayout                               srcImageLayout,
    VkImage                                     dstImage,
    VkImageLayout                               dstImageLayout,
    uint32_t                                    regionCount,
    const VkImageResolve*                       pRegions) {
    bool skip = false;
    skip |= DeviceValidateObject(commandBuffer, commandBuffer, kVulkanObjectTypeCommandBuffer, false, "VUID-vkCmdResolveImage-commandBuffer-parameter", "VUID-vkCmdResolveImage-commonparent");
    skip |= DeviceValidateObject(commandBuffer, srcImage, kVulkanObjectTypeImage, false, "VUID-vkCmdResolveImage-srcImage-parameter", "VUID-vkCmdResolveImage-commonparent");
    skip |= DeviceValidateObject(commandBuffer, dstImage, kVulkanObjectTypeImage, false, "VUID-vkCmdResolveImage-dstImage-parameter", "VUID-vkCmdResolveImage-commonparent");

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdSetEvent(
    VkCommandBuffer                             commandBuffer,
    VkEvent                                     event,
    VkPipelineStageFlags                        stageMask) {
    bool skip = false;
    skip |= DeviceValidateObject(commandBuffer, commandBuffer, kVulkanObjectTypeCommandBuffer, false, "VUID-vkCmdSetEvent-commandBuffer-parameter", "VUID-vkCmdSetEvent-commonparent");
    skip |= DeviceValidateObject(commandBuffer, event, kVulkanObjectTypeEvent, false, "VUID-vkCmdSetEvent-event-parameter", "VUID-vkCmdSetEvent-commonparent");

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdResetEvent(
    VkCommandBuffer                             commandBuffer,
    VkEvent                                     event,
    VkPipelineStageFlags                        stageMask) {
    bool skip = false;
    skip |= DeviceValidateObject(commandBuffer, commandBuffer, kVulkanObjectTypeCommandBuffer, false, "VUID-vkCmdResetEvent-commandBuffer-parameter", "VUID-vkCmdResetEvent-commonparent");
    skip |= DeviceValidateObject(commandBuffer, event, kVulkanObjectTypeEvent, false, "VUID-vkCmdResetEvent-event-parameter", "VUID-vkCmdResetEvent-commonparent");

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdWaitEvents(
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
    bool skip = false;
    skip |= DeviceValidateObject(commandBuffer, commandBuffer, kVulkanObjectTypeCommandBuffer, false, "VUID-vkCmdWaitEvents-commandBuffer-parameter", "VUID-vkCmdWaitEvents-commonparent");
    for (uint32_t index0 = 0; index0 < eventCount; ++index0) {
        skip |= DeviceValidateObject(commandBuffer, pEvents[index0], kVulkanObjectTypeEvent, false, "VUID-vkCmdWaitEvents-pEvents-parameter", "VUID-vkCmdWaitEvents-commonparent");
    }
    if (pBufferMemoryBarriers) {
        for (uint32_t index0 = 0; index0 < bufferMemoryBarrierCount; ++index0) {
            skip |= DeviceValidateObject(commandBuffer, pBufferMemoryBarriers[index0].buffer, kVulkanObjectTypeBuffer, false, "VUID-VkBufferMemoryBarrier-buffer-parameter", kVUIDUndefined);
        }
    }
    if (pImageMemoryBarriers) {
        for (uint32_t index0 = 0; index0 < imageMemoryBarrierCount; ++index0) {
            skip |= DeviceValidateObject(commandBuffer, pImageMemoryBarriers[index0].image, kVulkanObjectTypeImage, false, "VUID-VkImageMemoryBarrier-image-parameter", kVUIDUndefined);
        }
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdPipelineBarrier(
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
    bool skip = false;
    skip |= DeviceValidateObject(commandBuffer, commandBuffer, kVulkanObjectTypeCommandBuffer, false, "VUID-vkCmdPipelineBarrier-commandBuffer-parameter", kVUIDUndefined);
    if (pBufferMemoryBarriers) {
        for (uint32_t index0 = 0; index0 < bufferMemoryBarrierCount; ++index0) {
            skip |= DeviceValidateObject(commandBuffer, pBufferMemoryBarriers[index0].buffer, kVulkanObjectTypeBuffer, false, "VUID-VkBufferMemoryBarrier-buffer-parameter", kVUIDUndefined);
        }
    }
    if (pImageMemoryBarriers) {
        for (uint32_t index0 = 0; index0 < imageMemoryBarrierCount; ++index0) {
            skip |= DeviceValidateObject(commandBuffer, pImageMemoryBarriers[index0].image, kVulkanObjectTypeImage, false, "VUID-VkImageMemoryBarrier-image-parameter", kVUIDUndefined);
        }
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdBeginQuery(
    VkCommandBuffer                             commandBuffer,
    VkQueryPool                                 queryPool,
    uint32_t                                    query,
    VkQueryControlFlags                         flags) {
    bool skip = false;
    skip |= DeviceValidateObject(commandBuffer, commandBuffer, kVulkanObjectTypeCommandBuffer, false, "VUID-vkCmdBeginQuery-commandBuffer-parameter", "VUID-vkCmdBeginQuery-commonparent");
    skip |= DeviceValidateObject(commandBuffer, queryPool, kVulkanObjectTypeQueryPool, false, "VUID-vkCmdBeginQuery-queryPool-parameter", "VUID-vkCmdBeginQuery-commonparent");

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdEndQuery(
    VkCommandBuffer                             commandBuffer,
    VkQueryPool                                 queryPool,
    uint32_t                                    query) {
    bool skip = false;
    skip |= DeviceValidateObject(commandBuffer, commandBuffer, kVulkanObjectTypeCommandBuffer, false, "VUID-vkCmdEndQuery-commandBuffer-parameter", "VUID-vkCmdEndQuery-commonparent");
    skip |= DeviceValidateObject(commandBuffer, queryPool, kVulkanObjectTypeQueryPool, false, "VUID-vkCmdEndQuery-queryPool-parameter", "VUID-vkCmdEndQuery-commonparent");

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdResetQueryPool(
    VkCommandBuffer                             commandBuffer,
    VkQueryPool                                 queryPool,
    uint32_t                                    firstQuery,
    uint32_t                                    queryCount) {
    bool skip = false;
    skip |= DeviceValidateObject(commandBuffer, commandBuffer, kVulkanObjectTypeCommandBuffer, false, "VUID-vkCmdResetQueryPool-commandBuffer-parameter", "VUID-vkCmdResetQueryPool-commonparent");
    skip |= DeviceValidateObject(commandBuffer, queryPool, kVulkanObjectTypeQueryPool, false, "VUID-vkCmdResetQueryPool-queryPool-parameter", "VUID-vkCmdResetQueryPool-commonparent");

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdWriteTimestamp(
    VkCommandBuffer                             commandBuffer,
    VkPipelineStageFlagBits                     pipelineStage,
    VkQueryPool                                 queryPool,
    uint32_t                                    query) {
    bool skip = false;
    skip |= DeviceValidateObject(commandBuffer, commandBuffer, kVulkanObjectTypeCommandBuffer, false, "VUID-vkCmdWriteTimestamp-commandBuffer-parameter", "VUID-vkCmdWriteTimestamp-commonparent");
    skip |= DeviceValidateObject(commandBuffer, queryPool, kVulkanObjectTypeQueryPool, false, "VUID-vkCmdWriteTimestamp-queryPool-parameter", "VUID-vkCmdWriteTimestamp-commonparent");

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdCopyQueryPoolResults(
    VkCommandBuffer                             commandBuffer,
    VkQueryPool                                 queryPool,
    uint32_t                                    firstQuery,
    uint32_t                                    queryCount,
    VkBuffer                                    dstBuffer,
    VkDeviceSize                                dstOffset,
    VkDeviceSize                                stride,
    VkQueryResultFlags                          flags) {
    bool skip = false;
    skip |= DeviceValidateObject(commandBuffer, commandBuffer, kVulkanObjectTypeCommandBuffer, false, "VUID-vkCmdCopyQueryPoolResults-commandBuffer-parameter", "VUID-vkCmdCopyQueryPoolResults-commonparent");
    skip |= DeviceValidateObject(commandBuffer, queryPool, kVulkanObjectTypeQueryPool, false, "VUID-vkCmdCopyQueryPoolResults-queryPool-parameter", "VUID-vkCmdCopyQueryPoolResults-commonparent");
    skip |= DeviceValidateObject(commandBuffer, dstBuffer, kVulkanObjectTypeBuffer, false, "VUID-vkCmdCopyQueryPoolResults-dstBuffer-parameter", "VUID-vkCmdCopyQueryPoolResults-commonparent");

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdPushConstants(
    VkCommandBuffer                             commandBuffer,
    VkPipelineLayout                            layout,
    VkShaderStageFlags                          stageFlags,
    uint32_t                                    offset,
    uint32_t                                    size,
    const void*                                 pValues) {
    bool skip = false;
    skip |= DeviceValidateObject(commandBuffer, commandBuffer, kVulkanObjectTypeCommandBuffer, false, "VUID-vkCmdPushConstants-commandBuffer-parameter", "VUID-vkCmdPushConstants-commonparent");
    skip |= DeviceValidateObject(commandBuffer, layout, kVulkanObjectTypePipelineLayout, false, "VUID-vkCmdPushConstants-layout-parameter", "VUID-vkCmdPushConstants-commonparent");

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdBeginRenderPass(
    VkCommandBuffer                             commandBuffer,
    const VkRenderPassBeginInfo*                pRenderPassBegin,
    VkSubpassContents                           contents) {
    bool skip = false;
    skip |= DeviceValidateObject(commandBuffer, commandBuffer, kVulkanObjectTypeCommandBuffer, false, "VUID-vkCmdBeginRenderPass-commandBuffer-parameter", kVUIDUndefined);
    if (pRenderPassBegin) {
        skip |= DeviceValidateObject(commandBuffer, pRenderPassBegin->renderPass, kVulkanObjectTypeRenderPass, false, "VUID-VkRenderPassBeginInfo-renderPass-parameter", "VUID-VkRenderPassBeginInfo-commonparent");
        skip |= DeviceValidateObject(commandBuffer, pRenderPassBegin->framebuffer, kVulkanObjectTypeFramebuffer, false, "VUID-VkRenderPassBeginInfo-framebuffer-parameter", "VUID-VkRenderPassBeginInfo-commonparent");
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdNextSubpass(
    VkCommandBuffer                             commandBuffer,
    VkSubpassContents                           contents) {
    bool skip = false;
    skip |= DeviceValidateObject(commandBuffer, commandBuffer, kVulkanObjectTypeCommandBuffer, false, "VUID-vkCmdNextSubpass-commandBuffer-parameter", kVUIDUndefined);

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdEndRenderPass(
    VkCommandBuffer                             commandBuffer) {
    bool skip = false;
    skip |= DeviceValidateObject(commandBuffer, commandBuffer, kVulkanObjectTypeCommandBuffer, false, "VUID-vkCmdEndRenderPass-commandBuffer-parameter", kVUIDUndefined);

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdExecuteCommands(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    commandBufferCount,
    const VkCommandBuffer*                      pCommandBuffers) {
    bool skip = false;
    skip |= DeviceValidateObject(commandBuffer, commandBuffer, kVulkanObjectTypeCommandBuffer, false, "VUID-vkCmdExecuteCommands-commandBuffer-parameter", "VUID-vkCmdExecuteCommands-commonparent");
    for (uint32_t index0 = 0; index0 < commandBufferCount; ++index0) {
        skip |= DeviceValidateObject(commandBuffer, pCommandBuffers[index0], kVulkanObjectTypeCommandBuffer, false, "VUID-vkCmdExecuteCommands-pCommandBuffers-parameter", "VUID-vkCmdExecuteCommands-commonparent");
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateBindBufferMemory2(
    VkDevice                                    device,
    uint32_t                                    bindInfoCount,
    const VkBindBufferMemoryInfo*               pBindInfos) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkBindBufferMemory2-device-parameter", kVUIDUndefined);
    if (pBindInfos) {
        for (uint32_t index0 = 0; index0 < bindInfoCount; ++index0) {
            skip |= DeviceValidateObject(device, pBindInfos[index0].buffer, kVulkanObjectTypeBuffer, false, "VUID-VkBindBufferMemoryInfo-buffer-parameter", "VUID-VkBindBufferMemoryInfo-commonparent");
            skip |= DeviceValidateObject(device, pBindInfos[index0].memory, kVulkanObjectTypeDeviceMemory, false, "VUID-VkBindBufferMemoryInfo-memory-parameter", "VUID-VkBindBufferMemoryInfo-commonparent");
        }
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateBindImageMemory2(
    VkDevice                                    device,
    uint32_t                                    bindInfoCount,
    const VkBindImageMemoryInfo*                pBindInfos) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkBindImageMemory2-device-parameter", kVUIDUndefined);
    if (pBindInfos) {
        for (uint32_t index0 = 0; index0 < bindInfoCount; ++index0) {
            skip |= DeviceValidateObject(device, pBindInfos[index0].image, kVulkanObjectTypeImage, false, "VUID-VkBindImageMemoryInfo-image-parameter", "VUID-VkBindImageMemoryInfo-commonparent");
            skip |= DeviceValidateObject(device, pBindInfos[index0].memory, kVulkanObjectTypeDeviceMemory, true, kVUIDUndefined, "VUID-VkBindImageMemoryInfo-commonparent");
        }
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetDeviceGroupPeerMemoryFeatures(
    VkDevice                                    device,
    uint32_t                                    heapIndex,
    uint32_t                                    localDeviceIndex,
    uint32_t                                    remoteDeviceIndex,
    VkPeerMemoryFeatureFlags*                   pPeerMemoryFeatures) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkGetDeviceGroupPeerMemoryFeatures-device-parameter", kVUIDUndefined);

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdSetDeviceMask(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    deviceMask) {
    bool skip = false;
    skip |= DeviceValidateObject(commandBuffer, commandBuffer, kVulkanObjectTypeCommandBuffer, false, "VUID-vkCmdSetDeviceMask-commandBuffer-parameter", kVUIDUndefined);

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdDispatchBase(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    baseGroupX,
    uint32_t                                    baseGroupY,
    uint32_t                                    baseGroupZ,
    uint32_t                                    groupCountX,
    uint32_t                                    groupCountY,
    uint32_t                                    groupCountZ) {
    bool skip = false;
    skip |= DeviceValidateObject(commandBuffer, commandBuffer, kVulkanObjectTypeCommandBuffer, false, "VUID-vkCmdDispatchBase-commandBuffer-parameter", kVUIDUndefined);

    return skip;
}

bool ObjectLifetimes::PreCallValidateEnumeratePhysicalDeviceGroups(
    VkInstance                                  instance,
    uint32_t*                                   pPhysicalDeviceGroupCount,
    VkPhysicalDeviceGroupProperties*            pPhysicalDeviceGroupProperties) {
    bool skip = false;
    skip |= InstanceValidateObject(instance, instance, kVulkanObjectTypeInstance, false, "VUID-vkEnumeratePhysicalDeviceGroups-instance-parameter", kVUIDUndefined);

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetImageMemoryRequirements2(
    VkDevice                                    device,
    const VkImageMemoryRequirementsInfo2*       pInfo,
    VkMemoryRequirements2*                      pMemoryRequirements) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkGetImageMemoryRequirements2-device-parameter", kVUIDUndefined);
    if (pInfo) {
        skip |= DeviceValidateObject(device, pInfo->image, kVulkanObjectTypeImage, false, "VUID-VkImageMemoryRequirementsInfo2-image-parameter", kVUIDUndefined);
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetBufferMemoryRequirements2(
    VkDevice                                    device,
    const VkBufferMemoryRequirementsInfo2*      pInfo,
    VkMemoryRequirements2*                      pMemoryRequirements) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkGetBufferMemoryRequirements2-device-parameter", kVUIDUndefined);
    if (pInfo) {
        skip |= DeviceValidateObject(device, pInfo->buffer, kVulkanObjectTypeBuffer, false, "VUID-VkBufferMemoryRequirementsInfo2-buffer-parameter", kVUIDUndefined);
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetImageSparseMemoryRequirements2(
    VkDevice                                    device,
    const VkImageSparseMemoryRequirementsInfo2* pInfo,
    uint32_t*                                   pSparseMemoryRequirementCount,
    VkSparseImageMemoryRequirements2*           pSparseMemoryRequirements) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkGetImageSparseMemoryRequirements2-device-parameter", kVUIDUndefined);
    if (pInfo) {
        skip |= DeviceValidateObject(device, pInfo->image, kVulkanObjectTypeImage, false, "VUID-VkImageSparseMemoryRequirementsInfo2-image-parameter", kVUIDUndefined);
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetPhysicalDeviceFeatures2(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceFeatures2*                  pFeatures) {
    bool skip = false;
    skip |= InstanceValidateObject(physicalDevice, physicalDevice, kVulkanObjectTypePhysicalDevice, false, "VUID-vkGetPhysicalDeviceFeatures2-physicalDevice-parameter", kVUIDUndefined);

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetPhysicalDeviceProperties2(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceProperties2*                pProperties) {
    bool skip = false;
    skip |= InstanceValidateObject(physicalDevice, physicalDevice, kVulkanObjectTypePhysicalDevice, false, "VUID-vkGetPhysicalDeviceProperties2-physicalDevice-parameter", kVUIDUndefined);

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetPhysicalDeviceFormatProperties2(
    VkPhysicalDevice                            physicalDevice,
    VkFormat                                    format,
    VkFormatProperties2*                        pFormatProperties) {
    bool skip = false;
    skip |= InstanceValidateObject(physicalDevice, physicalDevice, kVulkanObjectTypePhysicalDevice, false, "VUID-vkGetPhysicalDeviceFormatProperties2-physicalDevice-parameter", kVUIDUndefined);

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetPhysicalDeviceImageFormatProperties2(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceImageFormatInfo2*     pImageFormatInfo,
    VkImageFormatProperties2*                   pImageFormatProperties) {
    bool skip = false;
    skip |= InstanceValidateObject(physicalDevice, physicalDevice, kVulkanObjectTypePhysicalDevice, false, "VUID-vkGetPhysicalDeviceImageFormatProperties2-physicalDevice-parameter", kVUIDUndefined);

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetPhysicalDeviceMemoryProperties2(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceMemoryProperties2*          pMemoryProperties) {
    bool skip = false;
    skip |= InstanceValidateObject(physicalDevice, physicalDevice, kVulkanObjectTypePhysicalDevice, false, "VUID-vkGetPhysicalDeviceMemoryProperties2-physicalDevice-parameter", kVUIDUndefined);

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetPhysicalDeviceSparseImageFormatProperties2(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceSparseImageFormatInfo2* pFormatInfo,
    uint32_t*                                   pPropertyCount,
    VkSparseImageFormatProperties2*             pProperties) {
    bool skip = false;
    skip |= InstanceValidateObject(physicalDevice, physicalDevice, kVulkanObjectTypePhysicalDevice, false, "VUID-vkGetPhysicalDeviceSparseImageFormatProperties2-physicalDevice-parameter", kVUIDUndefined);

    return skip;
}

bool ObjectLifetimes::PreCallValidateTrimCommandPool(
    VkDevice                                    device,
    VkCommandPool                               commandPool,
    VkCommandPoolTrimFlags                      flags) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkTrimCommandPool-device-parameter", kVUIDUndefined);
    skip |= DeviceValidateObject(device, commandPool, kVulkanObjectTypeCommandPool, false, "VUID-vkTrimCommandPool-commandPool-parameter", "VUID-vkTrimCommandPool-commandPool-parent");

    return skip;
}

bool ObjectLifetimes::PreCallValidateCreateSamplerYcbcrConversion(
    VkDevice                                    device,
    const VkSamplerYcbcrConversionCreateInfo*   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSamplerYcbcrConversion*                   pYcbcrConversion) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkCreateSamplerYcbcrConversion-device-parameter", kVUIDUndefined);

    return skip;
}

void ObjectLifetimes::PostCallRecordCreateSamplerYcbcrConversion(
    VkDevice                                    device,
    const VkSamplerYcbcrConversionCreateInfo*   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSamplerYcbcrConversion*                   pYcbcrConversion) {
    DeviceCreateObject(device, *pYcbcrConversion, kVulkanObjectTypeSamplerYcbcrConversion, pAllocator);

}

bool ObjectLifetimes::PreCallValidateDestroySamplerYcbcrConversion(
    VkDevice                                    device,
    VkSamplerYcbcrConversion                    ycbcrConversion,
    const VkAllocationCallbacks*                pAllocator) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkDestroySamplerYcbcrConversion-device-parameter", kVUIDUndefined);
    skip |= DeviceValidateObject(device, ycbcrConversion, kVulkanObjectTypeSamplerYcbcrConversion, true, "VUID-vkDestroySamplerYcbcrConversion-ycbcrConversion-parameter", "VUID-vkDestroySamplerYcbcrConversion-ycbcrConversion-parent");
    skip |= DeviceValidateDestroyObject(device, ycbcrConversion, kVulkanObjectTypeSamplerYcbcrConversion, pAllocator, kVUIDUndefined, kVUIDUndefined);

    return skip;
}

void ObjectLifetimes::PreCallRecordDestroySamplerYcbcrConversion(
    VkDevice                                    device,
    VkSamplerYcbcrConversion                    ycbcrConversion,
    const VkAllocationCallbacks*                pAllocator) {
    DeviceRecordDestroyObject(device, ycbcrConversion, kVulkanObjectTypeSamplerYcbcrConversion);

}

bool ObjectLifetimes::PreCallValidateCreateDescriptorUpdateTemplate(
    VkDevice                                    device,
    const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDescriptorUpdateTemplate*                 pDescriptorUpdateTemplate) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkCreateDescriptorUpdateTemplate-device-parameter", kVUIDUndefined);
    if (pCreateInfo) {
        skip |= DeviceValidateObject(device, pCreateInfo->descriptorSetLayout, kVulkanObjectTypeDescriptorSetLayout, true, "VUID-VkDescriptorUpdateTemplateCreateInfo-descriptorSetLayout-parameter", "VUID-VkDescriptorUpdateTemplateCreateInfo-commonparent");
        skip |= DeviceValidateObject(device, pCreateInfo->pipelineLayout, kVulkanObjectTypePipelineLayout, true, kVUIDUndefined, "VUID-VkDescriptorUpdateTemplateCreateInfo-commonparent");
    }

    return skip;
}

void ObjectLifetimes::PostCallRecordCreateDescriptorUpdateTemplate(
    VkDevice                                    device,
    const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDescriptorUpdateTemplate*                 pDescriptorUpdateTemplate) {
    DeviceCreateObject(device, *pDescriptorUpdateTemplate, kVulkanObjectTypeDescriptorUpdateTemplate, pAllocator);

}

bool ObjectLifetimes::PreCallValidateDestroyDescriptorUpdateTemplate(
    VkDevice                                    device,
    VkDescriptorUpdateTemplate                  descriptorUpdateTemplate,
    const VkAllocationCallbacks*                pAllocator) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkDestroyDescriptorUpdateTemplate-device-parameter", kVUIDUndefined);
    skip |= DeviceValidateObject(device, descriptorUpdateTemplate, kVulkanObjectTypeDescriptorUpdateTemplate, true, "VUID-vkDestroyDescriptorUpdateTemplate-descriptorUpdateTemplate-parameter", "VUID-vkDestroyDescriptorUpdateTemplate-descriptorUpdateTemplate-parent");
    skip |= DeviceValidateDestroyObject(device, descriptorUpdateTemplate, kVulkanObjectTypeDescriptorUpdateTemplate, pAllocator, "VUID-vkDestroyDescriptorUpdateTemplate-descriptorSetLayout-00356", "VUID-vkDestroyDescriptorUpdateTemplate-descriptorSetLayout-00357");

    return skip;
}

void ObjectLifetimes::PreCallRecordDestroyDescriptorUpdateTemplate(
    VkDevice                                    device,
    VkDescriptorUpdateTemplate                  descriptorUpdateTemplate,
    const VkAllocationCallbacks*                pAllocator) {
    DeviceRecordDestroyObject(device, descriptorUpdateTemplate, kVulkanObjectTypeDescriptorUpdateTemplate);

}

bool ObjectLifetimes::PreCallValidateUpdateDescriptorSetWithTemplate(
    VkDevice                                    device,
    VkDescriptorSet                             descriptorSet,
    VkDescriptorUpdateTemplate                  descriptorUpdateTemplate,
    const void*                                 pData) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkUpdateDescriptorSetWithTemplate-device-parameter", kVUIDUndefined);
    skip |= DeviceValidateObject(device, descriptorSet, kVulkanObjectTypeDescriptorSet, false, "VUID-vkUpdateDescriptorSetWithTemplate-descriptorSet-parameter", kVUIDUndefined);
    skip |= DeviceValidateObject(device, descriptorUpdateTemplate, kVulkanObjectTypeDescriptorUpdateTemplate, false, "VUID-vkUpdateDescriptorSetWithTemplate-descriptorUpdateTemplate-parameter", "VUID-vkUpdateDescriptorSetWithTemplate-descriptorUpdateTemplate-parent");

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetPhysicalDeviceExternalBufferProperties(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceExternalBufferInfo*   pExternalBufferInfo,
    VkExternalBufferProperties*                 pExternalBufferProperties) {
    bool skip = false;
    skip |= InstanceValidateObject(physicalDevice, physicalDevice, kVulkanObjectTypePhysicalDevice, false, "VUID-vkGetPhysicalDeviceExternalBufferProperties-physicalDevice-parameter", kVUIDUndefined);

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetPhysicalDeviceExternalFenceProperties(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceExternalFenceInfo*    pExternalFenceInfo,
    VkExternalFenceProperties*                  pExternalFenceProperties) {
    bool skip = false;
    skip |= InstanceValidateObject(physicalDevice, physicalDevice, kVulkanObjectTypePhysicalDevice, false, "VUID-vkGetPhysicalDeviceExternalFenceProperties-physicalDevice-parameter", kVUIDUndefined);

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetPhysicalDeviceExternalSemaphoreProperties(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceExternalSemaphoreInfo* pExternalSemaphoreInfo,
    VkExternalSemaphoreProperties*              pExternalSemaphoreProperties) {
    bool skip = false;
    skip |= InstanceValidateObject(physicalDevice, physicalDevice, kVulkanObjectTypePhysicalDevice, false, "VUID-vkGetPhysicalDeviceExternalSemaphoreProperties-physicalDevice-parameter", kVUIDUndefined);

    return skip;
}

bool ObjectLifetimes::PreCallValidateDestroySurfaceKHR(
    VkInstance                                  instance,
    VkSurfaceKHR                                surface,
    const VkAllocationCallbacks*                pAllocator) {
    bool skip = false;
    skip |= InstanceValidateObject(instance, instance, kVulkanObjectTypeInstance, false, "VUID-vkDestroySurfaceKHR-instance-parameter", kVUIDUndefined);
    skip |= InstanceValidateObject(instance, surface, kVulkanObjectTypeSurfaceKHR, true, "VUID-vkDestroySurfaceKHR-surface-parameter", "VUID-vkDestroySurfaceKHR-surface-parent");
    skip |= InstanceValidateDestroyObject(instance, surface, kVulkanObjectTypeSurfaceKHR, pAllocator, "VUID-vkDestroySurfaceKHR-surface-01267", "VUID-vkDestroySurfaceKHR-surface-01268");

    return skip;
}

void ObjectLifetimes::PreCallRecordDestroySurfaceKHR(
    VkInstance                                  instance,
    VkSurfaceKHR                                surface,
    const VkAllocationCallbacks*                pAllocator) {
    InstanceRecordDestroyObject(instance, surface, kVulkanObjectTypeSurfaceKHR);

}

bool ObjectLifetimes::PreCallValidateGetPhysicalDeviceSurfaceSupportKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    queueFamilyIndex,
    VkSurfaceKHR                                surface,
    VkBool32*                                   pSupported) {
    bool skip = false;
    skip |= InstanceValidateObject(physicalDevice, physicalDevice, kVulkanObjectTypePhysicalDevice, false, "VUID-vkGetPhysicalDeviceSurfaceSupportKHR-physicalDevice-parameter", "VUID-vkGetPhysicalDeviceSurfaceSupportKHR-commonparent");
    skip |= InstanceValidateObject(physicalDevice, surface, kVulkanObjectTypeSurfaceKHR, false, "VUID-vkGetPhysicalDeviceSurfaceSupportKHR-surface-parameter", "VUID-vkGetPhysicalDeviceSurfaceSupportKHR-commonparent");

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetPhysicalDeviceSurfaceCapabilitiesKHR(
    VkPhysicalDevice                            physicalDevice,
    VkSurfaceKHR                                surface,
    VkSurfaceCapabilitiesKHR*                   pSurfaceCapabilities) {
    bool skip = false;
    skip |= InstanceValidateObject(physicalDevice, physicalDevice, kVulkanObjectTypePhysicalDevice, false, "VUID-vkGetPhysicalDeviceSurfaceCapabilitiesKHR-physicalDevice-parameter", "VUID-vkGetPhysicalDeviceSurfaceCapabilitiesKHR-commonparent");
    skip |= InstanceValidateObject(physicalDevice, surface, kVulkanObjectTypeSurfaceKHR, false, "VUID-vkGetPhysicalDeviceSurfaceCapabilitiesKHR-surface-parameter", "VUID-vkGetPhysicalDeviceSurfaceCapabilitiesKHR-commonparent");

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetPhysicalDeviceSurfaceFormatsKHR(
    VkPhysicalDevice                            physicalDevice,
    VkSurfaceKHR                                surface,
    uint32_t*                                   pSurfaceFormatCount,
    VkSurfaceFormatKHR*                         pSurfaceFormats) {
    bool skip = false;
    skip |= InstanceValidateObject(physicalDevice, physicalDevice, kVulkanObjectTypePhysicalDevice, false, "VUID-vkGetPhysicalDeviceSurfaceFormatsKHR-physicalDevice-parameter", "VUID-vkGetPhysicalDeviceSurfaceFormatsKHR-commonparent");
    skip |= InstanceValidateObject(physicalDevice, surface, kVulkanObjectTypeSurfaceKHR, false, "VUID-vkGetPhysicalDeviceSurfaceFormatsKHR-surface-parameter", "VUID-vkGetPhysicalDeviceSurfaceFormatsKHR-commonparent");

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetPhysicalDeviceSurfacePresentModesKHR(
    VkPhysicalDevice                            physicalDevice,
    VkSurfaceKHR                                surface,
    uint32_t*                                   pPresentModeCount,
    VkPresentModeKHR*                           pPresentModes) {
    bool skip = false;
    skip |= InstanceValidateObject(physicalDevice, physicalDevice, kVulkanObjectTypePhysicalDevice, false, "VUID-vkGetPhysicalDeviceSurfacePresentModesKHR-physicalDevice-parameter", "VUID-vkGetPhysicalDeviceSurfacePresentModesKHR-commonparent");
    skip |= InstanceValidateObject(physicalDevice, surface, kVulkanObjectTypeSurfaceKHR, false, "VUID-vkGetPhysicalDeviceSurfacePresentModesKHR-surface-parameter", "VUID-vkGetPhysicalDeviceSurfacePresentModesKHR-commonparent");

    return skip;
}

bool ObjectLifetimes::PreCallValidateCreateSwapchainKHR(
    VkDevice                                    device,
    const VkSwapchainCreateInfoKHR*             pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSwapchainKHR*                             pSwapchain) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkCreateSwapchainKHR-device-parameter", kVUIDUndefined);
    if (pCreateInfo) {
        skip |= DeviceValidateObject(device, pCreateInfo->surface, kVulkanObjectTypeSurfaceKHR, false, "VUID-VkSwapchainCreateInfoKHR-surface-parameter", "VUID-VkSwapchainCreateInfoKHR-commonparent");
        skip |= DeviceValidateObject(device, pCreateInfo->oldSwapchain, kVulkanObjectTypeSwapchainKHR, true, "VUID-VkSwapchainCreateInfoKHR-oldSwapchain-parameter", "VUID-VkSwapchainCreateInfoKHR-oldSwapchain-parent");
    }

    return skip;
}

void ObjectLifetimes::PostCallRecordCreateSwapchainKHR(
    VkDevice                                    device,
    const VkSwapchainCreateInfoKHR*             pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSwapchainKHR*                             pSwapchain) {
    DeviceCreateObject(device, *pSwapchain, kVulkanObjectTypeSwapchainKHR, pAllocator);

}

bool ObjectLifetimes::PreCallValidateAcquireNextImageKHR(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    uint64_t                                    timeout,
    VkSemaphore                                 semaphore,
    VkFence                                     fence,
    uint32_t*                                   pImageIndex) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkAcquireNextImageKHR-device-parameter", "VUID-vkAcquireNextImageKHR-commonparent");
    skip |= DeviceValidateObject(device, swapchain, kVulkanObjectTypeSwapchainKHR, false, "VUID-vkAcquireNextImageKHR-swapchain-parameter", "VUID-vkAcquireNextImageKHR-commonparent");
    skip |= DeviceValidateObject(device, semaphore, kVulkanObjectTypeSemaphore, true, "VUID-vkAcquireNextImageKHR-semaphore-parameter", "VUID-vkAcquireNextImageKHR-semaphore-parent");
    skip |= DeviceValidateObject(device, fence, kVulkanObjectTypeFence, true, "VUID-vkAcquireNextImageKHR-fence-parameter", "VUID-vkAcquireNextImageKHR-fence-parent");

    return skip;
}

bool ObjectLifetimes::PreCallValidateQueuePresentKHR(
    VkQueue                                     queue,
    const VkPresentInfoKHR*                     pPresentInfo) {
    bool skip = false;
    skip |= DeviceValidateObject(queue, queue, kVulkanObjectTypeQueue, false, "VUID-vkQueuePresentKHR-queue-parameter", kVUIDUndefined);
    if (pPresentInfo) {
        for (uint32_t index1 = 0; index1 < pPresentInfo->waitSemaphoreCount; ++index1) {
            skip |= DeviceValidateObject(queue, pPresentInfo->pWaitSemaphores[index1], kVulkanObjectTypeSemaphore, false, "VUID-VkPresentInfoKHR-pWaitSemaphores-parameter", "VUID-VkPresentInfoKHR-commonparent");
        }
        for (uint32_t index1 = 0; index1 < pPresentInfo->swapchainCount; ++index1) {
            skip |= DeviceValidateObject(queue, pPresentInfo->pSwapchains[index1], kVulkanObjectTypeSwapchainKHR, false, "VUID-VkPresentInfoKHR-pSwapchains-parameter", "VUID-VkPresentInfoKHR-commonparent");
        }
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetDeviceGroupPresentCapabilitiesKHR(
    VkDevice                                    device,
    VkDeviceGroupPresentCapabilitiesKHR*        pDeviceGroupPresentCapabilities) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkGetDeviceGroupPresentCapabilitiesKHR-device-parameter", kVUIDUndefined);

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetDeviceGroupSurfacePresentModesKHR(
    VkDevice                                    device,
    VkSurfaceKHR                                surface,
    VkDeviceGroupPresentModeFlagsKHR*           pModes) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkGetDeviceGroupSurfacePresentModesKHR-device-parameter", "VUID-vkGetDeviceGroupSurfacePresentModesKHR-commonparent");
    skip |= DeviceValidateObject(device, surface, kVulkanObjectTypeSurfaceKHR, false, "VUID-vkGetDeviceGroupSurfacePresentModesKHR-surface-parameter", "VUID-vkGetDeviceGroupSurfacePresentModesKHR-commonparent");

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetPhysicalDevicePresentRectanglesKHR(
    VkPhysicalDevice                            physicalDevice,
    VkSurfaceKHR                                surface,
    uint32_t*                                   pRectCount,
    VkRect2D*                                   pRects) {
    bool skip = false;
    skip |= InstanceValidateObject(physicalDevice, physicalDevice, kVulkanObjectTypePhysicalDevice, false, "VUID-vkGetPhysicalDevicePresentRectanglesKHR-physicalDevice-parameter", "VUID-vkGetPhysicalDevicePresentRectanglesKHR-commonparent");
    skip |= InstanceValidateObject(physicalDevice, surface, kVulkanObjectTypeSurfaceKHR, false, "VUID-vkGetPhysicalDevicePresentRectanglesKHR-surface-parameter", "VUID-vkGetPhysicalDevicePresentRectanglesKHR-commonparent");

    return skip;
}

bool ObjectLifetimes::PreCallValidateAcquireNextImage2KHR(
    VkDevice                                    device,
    const VkAcquireNextImageInfoKHR*            pAcquireInfo,
    uint32_t*                                   pImageIndex) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkAcquireNextImage2KHR-device-parameter", kVUIDUndefined);
    if (pAcquireInfo) {
        skip |= DeviceValidateObject(device, pAcquireInfo->swapchain, kVulkanObjectTypeSwapchainKHR, false, "VUID-VkAcquireNextImageInfoKHR-swapchain-parameter", "VUID-VkAcquireNextImageInfoKHR-commonparent");
        skip |= DeviceValidateObject(device, pAcquireInfo->semaphore, kVulkanObjectTypeSemaphore, true, "VUID-VkAcquireNextImageInfoKHR-semaphore-parameter", "VUID-VkAcquireNextImageInfoKHR-commonparent");
        skip |= DeviceValidateObject(device, pAcquireInfo->fence, kVulkanObjectTypeFence, true, "VUID-VkAcquireNextImageInfoKHR-fence-parameter", "VUID-VkAcquireNextImageInfoKHR-commonparent");
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetPhysicalDeviceDisplayPlanePropertiesKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pPropertyCount,
    VkDisplayPlanePropertiesKHR*                pProperties) {
    bool skip = false;
    skip |= InstanceValidateObject(physicalDevice, physicalDevice, kVulkanObjectTypePhysicalDevice, false, "VUID-vkGetPhysicalDeviceDisplayPlanePropertiesKHR-physicalDevice-parameter", kVUIDUndefined);

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetDisplayPlaneSupportedDisplaysKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    planeIndex,
    uint32_t*                                   pDisplayCount,
    VkDisplayKHR*                               pDisplays) {
    bool skip = false;
    skip |= InstanceValidateObject(physicalDevice, physicalDevice, kVulkanObjectTypePhysicalDevice, false, "VUID-vkGetDisplayPlaneSupportedDisplaysKHR-physicalDevice-parameter", kVUIDUndefined);

    return skip;
}

void ObjectLifetimes::PostCallRecordGetDisplayPlaneSupportedDisplaysKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    planeIndex,
    uint32_t*                                   pDisplayCount,
    VkDisplayKHR*                               pDisplays) {
    for (uint32_t index = 0; index < *pDisplayCount; index++) {
        InstanceCreateObject(physicalDevice, pDisplays[index], kVulkanObjectTypeDisplayKHR, nullptr);
    }

}

bool ObjectLifetimes::PreCallValidateCreateDisplayModeKHR(
    VkPhysicalDevice                            physicalDevice,
    VkDisplayKHR                                display,
    const VkDisplayModeCreateInfoKHR*           pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDisplayModeKHR*                           pMode) {
    bool skip = false;
    skip |= InstanceValidateObject(physicalDevice, physicalDevice, kVulkanObjectTypePhysicalDevice, false, "VUID-vkCreateDisplayModeKHR-physicalDevice-parameter", kVUIDUndefined);
    skip |= InstanceValidateObject(physicalDevice, display, kVulkanObjectTypeDisplayKHR, false, "VUID-vkCreateDisplayModeKHR-display-parameter", kVUIDUndefined);

    return skip;
}

void ObjectLifetimes::PostCallRecordCreateDisplayModeKHR(
    VkPhysicalDevice                            physicalDevice,
    VkDisplayKHR                                display,
    const VkDisplayModeCreateInfoKHR*           pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDisplayModeKHR*                           pMode) {
    InstanceCreateObject(physicalDevice, *pMode, kVulkanObjectTypeDisplayModeKHR, pAllocator);

}

bool ObjectLifetimes::PreCallValidateGetDisplayPlaneCapabilitiesKHR(
    VkPhysicalDevice                            physicalDevice,
    VkDisplayModeKHR                            mode,
    uint32_t                                    planeIndex,
    VkDisplayPlaneCapabilitiesKHR*              pCapabilities) {
    bool skip = false;
    skip |= InstanceValidateObject(physicalDevice, physicalDevice, kVulkanObjectTypePhysicalDevice, false, "VUID-vkGetDisplayPlaneCapabilitiesKHR-physicalDevice-parameter", kVUIDUndefined);
    skip |= InstanceValidateObject(physicalDevice, mode, kVulkanObjectTypeDisplayModeKHR, false, "VUID-vkGetDisplayPlaneCapabilitiesKHR-mode-parameter", kVUIDUndefined);

    return skip;
}

bool ObjectLifetimes::PreCallValidateCreateDisplayPlaneSurfaceKHR(
    VkInstance                                  instance,
    const VkDisplaySurfaceCreateInfoKHR*        pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) {
    bool skip = false;
    skip |= InstanceValidateObject(instance, instance, kVulkanObjectTypeInstance, false, "VUID-vkCreateDisplayPlaneSurfaceKHR-instance-parameter", kVUIDUndefined);
    if (pCreateInfo) {
        skip |= InstanceValidateObject(instance, pCreateInfo->displayMode, kVulkanObjectTypeDisplayModeKHR, false, "VUID-VkDisplaySurfaceCreateInfoKHR-displayMode-parameter", kVUIDUndefined);
    }

    return skip;
}

void ObjectLifetimes::PostCallRecordCreateDisplayPlaneSurfaceKHR(
    VkInstance                                  instance,
    const VkDisplaySurfaceCreateInfoKHR*        pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) {
    InstanceCreateObject(instance, *pSurface, kVulkanObjectTypeSurfaceKHR, pAllocator);

}

bool ObjectLifetimes::PreCallValidateCreateSharedSwapchainsKHR(
    VkDevice                                    device,
    uint32_t                                    swapchainCount,
    const VkSwapchainCreateInfoKHR*             pCreateInfos,
    const VkAllocationCallbacks*                pAllocator,
    VkSwapchainKHR*                             pSwapchains) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkCreateSharedSwapchainsKHR-device-parameter", kVUIDUndefined);
    if (pCreateInfos) {
        for (uint32_t index0 = 0; index0 < swapchainCount; ++index0) {
            skip |= DeviceValidateObject(device, pCreateInfos[index0].surface, kVulkanObjectTypeSurfaceKHR, false, "VUID-VkSwapchainCreateInfoKHR-surface-parameter", "VUID-VkSwapchainCreateInfoKHR-commonparent");
            skip |= DeviceValidateObject(device, pCreateInfos[index0].oldSwapchain, kVulkanObjectTypeSwapchainKHR, true, "VUID-VkSwapchainCreateInfoKHR-oldSwapchain-parameter", "VUID-VkSwapchainCreateInfoKHR-oldSwapchain-parent");
        }
    }

    return skip;
}

void ObjectLifetimes::PostCallRecordCreateSharedSwapchainsKHR(
    VkDevice                                    device,
    uint32_t                                    swapchainCount,
    const VkSwapchainCreateInfoKHR*             pCreateInfos,
    const VkAllocationCallbacks*                pAllocator,
    VkSwapchainKHR*                             pSwapchains) {
    for (uint32_t index = 0; index < swapchainCount; index++) {
        DeviceCreateObject(device, pSwapchains[index], kVulkanObjectTypeSwapchainKHR, pAllocator);
    }

}

#ifdef VK_USE_PLATFORM_XLIB_KHR

bool ObjectLifetimes::PreCallValidateCreateXlibSurfaceKHR(
    VkInstance                                  instance,
    const VkXlibSurfaceCreateInfoKHR*           pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) {
    bool skip = false;
    skip |= InstanceValidateObject(instance, instance, kVulkanObjectTypeInstance, false, "VUID-vkCreateXlibSurfaceKHR-instance-parameter", kVUIDUndefined);

    return skip;
}

void ObjectLifetimes::PostCallRecordCreateXlibSurfaceKHR(
    VkInstance                                  instance,
    const VkXlibSurfaceCreateInfoKHR*           pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) {
    InstanceCreateObject(instance, *pSurface, kVulkanObjectTypeSurfaceKHR, pAllocator);

}
#endif // VK_USE_PLATFORM_XLIB_KHR

#ifdef VK_USE_PLATFORM_XLIB_KHR

bool ObjectLifetimes::PreCallValidateGetPhysicalDeviceXlibPresentationSupportKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    queueFamilyIndex,
    Display*                                    dpy,
    VisualID                                    visualID) {
    bool skip = false;
    skip |= InstanceValidateObject(physicalDevice, physicalDevice, kVulkanObjectTypePhysicalDevice, false, "VUID-vkGetPhysicalDeviceXlibPresentationSupportKHR-physicalDevice-parameter", kVUIDUndefined);

    return skip;
}
#endif // VK_USE_PLATFORM_XLIB_KHR

#ifdef VK_USE_PLATFORM_XCB_KHR

bool ObjectLifetimes::PreCallValidateCreateXcbSurfaceKHR(
    VkInstance                                  instance,
    const VkXcbSurfaceCreateInfoKHR*            pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) {
    bool skip = false;
    skip |= InstanceValidateObject(instance, instance, kVulkanObjectTypeInstance, false, "VUID-vkCreateXcbSurfaceKHR-instance-parameter", kVUIDUndefined);

    return skip;
}

void ObjectLifetimes::PostCallRecordCreateXcbSurfaceKHR(
    VkInstance                                  instance,
    const VkXcbSurfaceCreateInfoKHR*            pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) {
    InstanceCreateObject(instance, *pSurface, kVulkanObjectTypeSurfaceKHR, pAllocator);

}
#endif // VK_USE_PLATFORM_XCB_KHR

#ifdef VK_USE_PLATFORM_XCB_KHR

bool ObjectLifetimes::PreCallValidateGetPhysicalDeviceXcbPresentationSupportKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    queueFamilyIndex,
    xcb_connection_t*                           connection,
    xcb_visualid_t                              visual_id) {
    bool skip = false;
    skip |= InstanceValidateObject(physicalDevice, physicalDevice, kVulkanObjectTypePhysicalDevice, false, "VUID-vkGetPhysicalDeviceXcbPresentationSupportKHR-physicalDevice-parameter", kVUIDUndefined);

    return skip;
}
#endif // VK_USE_PLATFORM_XCB_KHR

#ifdef VK_USE_PLATFORM_WAYLAND_KHR

bool ObjectLifetimes::PreCallValidateCreateWaylandSurfaceKHR(
    VkInstance                                  instance,
    const VkWaylandSurfaceCreateInfoKHR*        pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) {
    bool skip = false;
    skip |= InstanceValidateObject(instance, instance, kVulkanObjectTypeInstance, false, "VUID-vkCreateWaylandSurfaceKHR-instance-parameter", kVUIDUndefined);

    return skip;
}

void ObjectLifetimes::PostCallRecordCreateWaylandSurfaceKHR(
    VkInstance                                  instance,
    const VkWaylandSurfaceCreateInfoKHR*        pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) {
    InstanceCreateObject(instance, *pSurface, kVulkanObjectTypeSurfaceKHR, pAllocator);

}
#endif // VK_USE_PLATFORM_WAYLAND_KHR

#ifdef VK_USE_PLATFORM_WAYLAND_KHR

bool ObjectLifetimes::PreCallValidateGetPhysicalDeviceWaylandPresentationSupportKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    queueFamilyIndex,
    struct wl_display*                          display) {
    bool skip = false;
    skip |= InstanceValidateObject(physicalDevice, physicalDevice, kVulkanObjectTypePhysicalDevice, false, "VUID-vkGetPhysicalDeviceWaylandPresentationSupportKHR-physicalDevice-parameter", kVUIDUndefined);

    return skip;
}
#endif // VK_USE_PLATFORM_WAYLAND_KHR

#ifdef VK_USE_PLATFORM_MIR_KHR

bool ObjectLifetimes::PreCallValidateCreateMirSurfaceKHR(
    VkInstance                                  instance,
    const VkMirSurfaceCreateInfoKHR*            pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) {
    bool skip = false;
    skip |= InstanceValidateObject(instance, instance, kVulkanObjectTypeInstance, false, "VUID-vkCreateMirSurfaceKHR-instance-parameter", kVUIDUndefined);

    return skip;
}

void ObjectLifetimes::PostCallRecordCreateMirSurfaceKHR(
    VkInstance                                  instance,
    const VkMirSurfaceCreateInfoKHR*            pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) {
    InstanceCreateObject(instance, *pSurface, kVulkanObjectTypeSurfaceKHR, pAllocator);

}
#endif // VK_USE_PLATFORM_MIR_KHR

#ifdef VK_USE_PLATFORM_MIR_KHR

bool ObjectLifetimes::PreCallValidateGetPhysicalDeviceMirPresentationSupportKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    queueFamilyIndex,
    MirConnection*                              connection) {
    bool skip = false;
    skip |= InstanceValidateObject(physicalDevice, physicalDevice, kVulkanObjectTypePhysicalDevice, false, "VUID-vkGetPhysicalDeviceMirPresentationSupportKHR-physicalDevice-parameter", kVUIDUndefined);

    return skip;
}
#endif // VK_USE_PLATFORM_MIR_KHR

#ifdef VK_USE_PLATFORM_ANDROID_KHR

bool ObjectLifetimes::PreCallValidateCreateAndroidSurfaceKHR(
    VkInstance                                  instance,
    const VkAndroidSurfaceCreateInfoKHR*        pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) {
    bool skip = false;
    skip |= InstanceValidateObject(instance, instance, kVulkanObjectTypeInstance, false, "VUID-vkCreateAndroidSurfaceKHR-instance-parameter", kVUIDUndefined);

    return skip;
}

void ObjectLifetimes::PostCallRecordCreateAndroidSurfaceKHR(
    VkInstance                                  instance,
    const VkAndroidSurfaceCreateInfoKHR*        pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) {
    InstanceCreateObject(instance, *pSurface, kVulkanObjectTypeSurfaceKHR, pAllocator);

}
#endif // VK_USE_PLATFORM_ANDROID_KHR

#ifdef VK_USE_PLATFORM_WIN32_KHR

bool ObjectLifetimes::PreCallValidateCreateWin32SurfaceKHR(
    VkInstance                                  instance,
    const VkWin32SurfaceCreateInfoKHR*          pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) {
    bool skip = false;
    skip |= InstanceValidateObject(instance, instance, kVulkanObjectTypeInstance, false, "VUID-vkCreateWin32SurfaceKHR-instance-parameter", kVUIDUndefined);

    return skip;
}

void ObjectLifetimes::PostCallRecordCreateWin32SurfaceKHR(
    VkInstance                                  instance,
    const VkWin32SurfaceCreateInfoKHR*          pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) {
    InstanceCreateObject(instance, *pSurface, kVulkanObjectTypeSurfaceKHR, pAllocator);

}
#endif // VK_USE_PLATFORM_WIN32_KHR

#ifdef VK_USE_PLATFORM_WIN32_KHR

bool ObjectLifetimes::PreCallValidateGetPhysicalDeviceWin32PresentationSupportKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    queueFamilyIndex) {
    bool skip = false;
    skip |= InstanceValidateObject(physicalDevice, physicalDevice, kVulkanObjectTypePhysicalDevice, false, "VUID-vkGetPhysicalDeviceWin32PresentationSupportKHR-physicalDevice-parameter", kVUIDUndefined);

    return skip;
}
#endif // VK_USE_PLATFORM_WIN32_KHR

bool ObjectLifetimes::PreCallValidateGetPhysicalDeviceFeatures2KHR(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceFeatures2*                  pFeatures) {
    bool skip = false;
    skip |= InstanceValidateObject(physicalDevice, physicalDevice, kVulkanObjectTypePhysicalDevice, false, "VUID-vkGetPhysicalDeviceFeatures2KHR-physicalDevice-parameter", kVUIDUndefined);

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetPhysicalDeviceProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceProperties2*                pProperties) {
    bool skip = false;
    skip |= InstanceValidateObject(physicalDevice, physicalDevice, kVulkanObjectTypePhysicalDevice, false, "VUID-vkGetPhysicalDeviceProperties2KHR-physicalDevice-parameter", kVUIDUndefined);

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetPhysicalDeviceFormatProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    VkFormat                                    format,
    VkFormatProperties2*                        pFormatProperties) {
    bool skip = false;
    skip |= InstanceValidateObject(physicalDevice, physicalDevice, kVulkanObjectTypePhysicalDevice, false, "VUID-vkGetPhysicalDeviceFormatProperties2KHR-physicalDevice-parameter", kVUIDUndefined);

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetPhysicalDeviceImageFormatProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceImageFormatInfo2*     pImageFormatInfo,
    VkImageFormatProperties2*                   pImageFormatProperties) {
    bool skip = false;
    skip |= InstanceValidateObject(physicalDevice, physicalDevice, kVulkanObjectTypePhysicalDevice, false, "VUID-vkGetPhysicalDeviceImageFormatProperties2KHR-physicalDevice-parameter", kVUIDUndefined);

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetPhysicalDeviceMemoryProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceMemoryProperties2*          pMemoryProperties) {
    bool skip = false;
    skip |= InstanceValidateObject(physicalDevice, physicalDevice, kVulkanObjectTypePhysicalDevice, false, "VUID-vkGetPhysicalDeviceMemoryProperties2KHR-physicalDevice-parameter", kVUIDUndefined);

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetPhysicalDeviceSparseImageFormatProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceSparseImageFormatInfo2* pFormatInfo,
    uint32_t*                                   pPropertyCount,
    VkSparseImageFormatProperties2*             pProperties) {
    bool skip = false;
    skip |= InstanceValidateObject(physicalDevice, physicalDevice, kVulkanObjectTypePhysicalDevice, false, "VUID-vkGetPhysicalDeviceSparseImageFormatProperties2KHR-physicalDevice-parameter", kVUIDUndefined);

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetDeviceGroupPeerMemoryFeaturesKHR(
    VkDevice                                    device,
    uint32_t                                    heapIndex,
    uint32_t                                    localDeviceIndex,
    uint32_t                                    remoteDeviceIndex,
    VkPeerMemoryFeatureFlags*                   pPeerMemoryFeatures) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkGetDeviceGroupPeerMemoryFeaturesKHR-device-parameter", kVUIDUndefined);

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdSetDeviceMaskKHR(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    deviceMask) {
    bool skip = false;
    skip |= DeviceValidateObject(commandBuffer, commandBuffer, kVulkanObjectTypeCommandBuffer, false, "VUID-vkCmdSetDeviceMaskKHR-commandBuffer-parameter", kVUIDUndefined);

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdDispatchBaseKHR(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    baseGroupX,
    uint32_t                                    baseGroupY,
    uint32_t                                    baseGroupZ,
    uint32_t                                    groupCountX,
    uint32_t                                    groupCountY,
    uint32_t                                    groupCountZ) {
    bool skip = false;
    skip |= DeviceValidateObject(commandBuffer, commandBuffer, kVulkanObjectTypeCommandBuffer, false, "VUID-vkCmdDispatchBaseKHR-commandBuffer-parameter", kVUIDUndefined);

    return skip;
}

bool ObjectLifetimes::PreCallValidateTrimCommandPoolKHR(
    VkDevice                                    device,
    VkCommandPool                               commandPool,
    VkCommandPoolTrimFlags                      flags) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkTrimCommandPoolKHR-device-parameter", kVUIDUndefined);
    skip |= DeviceValidateObject(device, commandPool, kVulkanObjectTypeCommandPool, false, "VUID-vkTrimCommandPoolKHR-commandPool-parameter", "VUID-vkTrimCommandPoolKHR-commandPool-parent");

    return skip;
}

bool ObjectLifetimes::PreCallValidateEnumeratePhysicalDeviceGroupsKHR(
    VkInstance                                  instance,
    uint32_t*                                   pPhysicalDeviceGroupCount,
    VkPhysicalDeviceGroupProperties*            pPhysicalDeviceGroupProperties) {
    bool skip = false;
    skip |= InstanceValidateObject(instance, instance, kVulkanObjectTypeInstance, false, "VUID-vkEnumeratePhysicalDeviceGroupsKHR-instance-parameter", kVUIDUndefined);

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetPhysicalDeviceExternalBufferPropertiesKHR(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceExternalBufferInfo*   pExternalBufferInfo,
    VkExternalBufferProperties*                 pExternalBufferProperties) {
    bool skip = false;
    skip |= InstanceValidateObject(physicalDevice, physicalDevice, kVulkanObjectTypePhysicalDevice, false, "VUID-vkGetPhysicalDeviceExternalBufferPropertiesKHR-physicalDevice-parameter", kVUIDUndefined);

    return skip;
}

#ifdef VK_USE_PLATFORM_WIN32_KHR

bool ObjectLifetimes::PreCallValidateGetMemoryWin32HandleKHR(
    VkDevice                                    device,
    const VkMemoryGetWin32HandleInfoKHR*        pGetWin32HandleInfo,
    HANDLE*                                     pHandle) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkGetMemoryWin32HandleKHR-device-parameter", kVUIDUndefined);
    if (pGetWin32HandleInfo) {
        skip |= DeviceValidateObject(device, pGetWin32HandleInfo->memory, kVulkanObjectTypeDeviceMemory, false, "VUID-VkMemoryGetWin32HandleInfoKHR-memory-parameter", kVUIDUndefined);
    }

    return skip;
}
#endif // VK_USE_PLATFORM_WIN32_KHR

#ifdef VK_USE_PLATFORM_WIN32_KHR

bool ObjectLifetimes::PreCallValidateGetMemoryWin32HandlePropertiesKHR(
    VkDevice                                    device,
    VkExternalMemoryHandleTypeFlagBits          handleType,
    HANDLE                                      handle,
    VkMemoryWin32HandlePropertiesKHR*           pMemoryWin32HandleProperties) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkGetMemoryWin32HandlePropertiesKHR-device-parameter", kVUIDUndefined);

    return skip;
}
#endif // VK_USE_PLATFORM_WIN32_KHR

bool ObjectLifetimes::PreCallValidateGetMemoryFdKHR(
    VkDevice                                    device,
    const VkMemoryGetFdInfoKHR*                 pGetFdInfo,
    int*                                        pFd) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkGetMemoryFdKHR-device-parameter", kVUIDUndefined);
    if (pGetFdInfo) {
        skip |= DeviceValidateObject(device, pGetFdInfo->memory, kVulkanObjectTypeDeviceMemory, false, "VUID-VkMemoryGetFdInfoKHR-memory-parameter", kVUIDUndefined);
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetMemoryFdPropertiesKHR(
    VkDevice                                    device,
    VkExternalMemoryHandleTypeFlagBits          handleType,
    int                                         fd,
    VkMemoryFdPropertiesKHR*                    pMemoryFdProperties) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkGetMemoryFdPropertiesKHR-device-parameter", kVUIDUndefined);

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetPhysicalDeviceExternalSemaphorePropertiesKHR(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceExternalSemaphoreInfo* pExternalSemaphoreInfo,
    VkExternalSemaphoreProperties*              pExternalSemaphoreProperties) {
    bool skip = false;
    skip |= InstanceValidateObject(physicalDevice, physicalDevice, kVulkanObjectTypePhysicalDevice, false, "VUID-vkGetPhysicalDeviceExternalSemaphorePropertiesKHR-physicalDevice-parameter", kVUIDUndefined);

    return skip;
}

#ifdef VK_USE_PLATFORM_WIN32_KHR

bool ObjectLifetimes::PreCallValidateImportSemaphoreWin32HandleKHR(
    VkDevice                                    device,
    const VkImportSemaphoreWin32HandleInfoKHR*  pImportSemaphoreWin32HandleInfo) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkImportSemaphoreWin32HandleKHR-device-parameter", kVUIDUndefined);
    if (pImportSemaphoreWin32HandleInfo) {
        skip |= DeviceValidateObject(device, pImportSemaphoreWin32HandleInfo->semaphore, kVulkanObjectTypeSemaphore, false, "VUID-VkImportSemaphoreWin32HandleInfoKHR-semaphore-parameter", kVUIDUndefined);
    }

    return skip;
}
#endif // VK_USE_PLATFORM_WIN32_KHR

#ifdef VK_USE_PLATFORM_WIN32_KHR

bool ObjectLifetimes::PreCallValidateGetSemaphoreWin32HandleKHR(
    VkDevice                                    device,
    const VkSemaphoreGetWin32HandleInfoKHR*     pGetWin32HandleInfo,
    HANDLE*                                     pHandle) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkGetSemaphoreWin32HandleKHR-device-parameter", kVUIDUndefined);
    if (pGetWin32HandleInfo) {
        skip |= DeviceValidateObject(device, pGetWin32HandleInfo->semaphore, kVulkanObjectTypeSemaphore, false, "VUID-VkSemaphoreGetWin32HandleInfoKHR-semaphore-parameter", kVUIDUndefined);
    }

    return skip;
}
#endif // VK_USE_PLATFORM_WIN32_KHR

bool ObjectLifetimes::PreCallValidateImportSemaphoreFdKHR(
    VkDevice                                    device,
    const VkImportSemaphoreFdInfoKHR*           pImportSemaphoreFdInfo) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkImportSemaphoreFdKHR-device-parameter", kVUIDUndefined);
    if (pImportSemaphoreFdInfo) {
        skip |= DeviceValidateObject(device, pImportSemaphoreFdInfo->semaphore, kVulkanObjectTypeSemaphore, false, "VUID-VkImportSemaphoreFdInfoKHR-semaphore-parameter", kVUIDUndefined);
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetSemaphoreFdKHR(
    VkDevice                                    device,
    const VkSemaphoreGetFdInfoKHR*              pGetFdInfo,
    int*                                        pFd) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkGetSemaphoreFdKHR-device-parameter", kVUIDUndefined);
    if (pGetFdInfo) {
        skip |= DeviceValidateObject(device, pGetFdInfo->semaphore, kVulkanObjectTypeSemaphore, false, "VUID-VkSemaphoreGetFdInfoKHR-semaphore-parameter", kVUIDUndefined);
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdPushDescriptorSetWithTemplateKHR(
    VkCommandBuffer                             commandBuffer,
    VkDescriptorUpdateTemplate                  descriptorUpdateTemplate,
    VkPipelineLayout                            layout,
    uint32_t                                    set,
    const void*                                 pData) {
    bool skip = false;
    skip |= DeviceValidateObject(commandBuffer, commandBuffer, kVulkanObjectTypeCommandBuffer, false, "VUID-vkCmdPushDescriptorSetWithTemplateKHR-commandBuffer-parameter", "VUID-vkCmdPushDescriptorSetWithTemplateKHR-commonparent");
    skip |= DeviceValidateObject(commandBuffer, descriptorUpdateTemplate, kVulkanObjectTypeDescriptorUpdateTemplate, false, "VUID-vkCmdPushDescriptorSetWithTemplateKHR-descriptorUpdateTemplate-parameter", "VUID-vkCmdPushDescriptorSetWithTemplateKHR-commonparent");
    skip |= DeviceValidateObject(commandBuffer, layout, kVulkanObjectTypePipelineLayout, false, "VUID-vkCmdPushDescriptorSetWithTemplateKHR-layout-parameter", "VUID-vkCmdPushDescriptorSetWithTemplateKHR-commonparent");

    return skip;
}

bool ObjectLifetimes::PreCallValidateCreateDescriptorUpdateTemplateKHR(
    VkDevice                                    device,
    const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDescriptorUpdateTemplate*                 pDescriptorUpdateTemplate) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkCreateDescriptorUpdateTemplateKHR-device-parameter", kVUIDUndefined);
    if (pCreateInfo) {
        skip |= DeviceValidateObject(device, pCreateInfo->descriptorSetLayout, kVulkanObjectTypeDescriptorSetLayout, true, "VUID-VkDescriptorUpdateTemplateCreateInfo-descriptorSetLayout-parameter", "VUID-VkDescriptorUpdateTemplateCreateInfo-commonparent");
        skip |= DeviceValidateObject(device, pCreateInfo->pipelineLayout, kVulkanObjectTypePipelineLayout, true, kVUIDUndefined, "VUID-VkDescriptorUpdateTemplateCreateInfo-commonparent");
    }

    return skip;
}

void ObjectLifetimes::PostCallRecordCreateDescriptorUpdateTemplateKHR(
    VkDevice                                    device,
    const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDescriptorUpdateTemplate*                 pDescriptorUpdateTemplate) {
    DeviceCreateObject(device, *pDescriptorUpdateTemplate, kVulkanObjectTypeDescriptorUpdateTemplate, pAllocator);

}

bool ObjectLifetimes::PreCallValidateDestroyDescriptorUpdateTemplateKHR(
    VkDevice                                    device,
    VkDescriptorUpdateTemplate                  descriptorUpdateTemplate,
    const VkAllocationCallbacks*                pAllocator) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkDestroyDescriptorUpdateTemplateKHR-device-parameter", kVUIDUndefined);
    skip |= DeviceValidateObject(device, descriptorUpdateTemplate, kVulkanObjectTypeDescriptorUpdateTemplate, true, "VUID-vkDestroyDescriptorUpdateTemplateKHR-descriptorUpdateTemplate-parameter", "VUID-vkDestroyDescriptorUpdateTemplateKHR-descriptorUpdateTemplate-parent");
    skip |= DeviceValidateDestroyObject(device, descriptorUpdateTemplate, kVulkanObjectTypeDescriptorUpdateTemplate, pAllocator, "VUID-vkDestroyDescriptorUpdateTemplate-descriptorSetLayout-00356", "VUID-vkDestroyDescriptorUpdateTemplate-descriptorSetLayout-00357");

    return skip;
}

void ObjectLifetimes::PreCallRecordDestroyDescriptorUpdateTemplateKHR(
    VkDevice                                    device,
    VkDescriptorUpdateTemplate                  descriptorUpdateTemplate,
    const VkAllocationCallbacks*                pAllocator) {
    DeviceRecordDestroyObject(device, descriptorUpdateTemplate, kVulkanObjectTypeDescriptorUpdateTemplate);

}

bool ObjectLifetimes::PreCallValidateUpdateDescriptorSetWithTemplateKHR(
    VkDevice                                    device,
    VkDescriptorSet                             descriptorSet,
    VkDescriptorUpdateTemplate                  descriptorUpdateTemplate,
    const void*                                 pData) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkUpdateDescriptorSetWithTemplateKHR-device-parameter", kVUIDUndefined);
    skip |= DeviceValidateObject(device, descriptorSet, kVulkanObjectTypeDescriptorSet, false, "VUID-vkUpdateDescriptorSetWithTemplateKHR-descriptorSet-parameter", kVUIDUndefined);
    skip |= DeviceValidateObject(device, descriptorUpdateTemplate, kVulkanObjectTypeDescriptorUpdateTemplate, false, "VUID-vkUpdateDescriptorSetWithTemplateKHR-descriptorUpdateTemplate-parameter", "VUID-vkUpdateDescriptorSetWithTemplateKHR-descriptorUpdateTemplate-parent");

    return skip;
}

bool ObjectLifetimes::PreCallValidateCreateRenderPass2KHR(
    VkDevice                                    device,
    const VkRenderPassCreateInfo2KHR*           pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkRenderPass*                               pRenderPass) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkCreateRenderPass2KHR-device-parameter", kVUIDUndefined);

    return skip;
}

void ObjectLifetimes::PostCallRecordCreateRenderPass2KHR(
    VkDevice                                    device,
    const VkRenderPassCreateInfo2KHR*           pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkRenderPass*                               pRenderPass) {
    DeviceCreateObject(device, *pRenderPass, kVulkanObjectTypeRenderPass, pAllocator);

}

bool ObjectLifetimes::PreCallValidateCmdBeginRenderPass2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkRenderPassBeginInfo*                pRenderPassBegin,
    const VkSubpassBeginInfoKHR*                pSubpassBeginInfo) {
    bool skip = false;
    skip |= DeviceValidateObject(commandBuffer, commandBuffer, kVulkanObjectTypeCommandBuffer, false, "VUID-vkCmdBeginRenderPass2KHR-commandBuffer-parameter", kVUIDUndefined);
    if (pRenderPassBegin) {
        skip |= DeviceValidateObject(commandBuffer, pRenderPassBegin->renderPass, kVulkanObjectTypeRenderPass, false, "VUID-VkRenderPassBeginInfo-renderPass-parameter", "VUID-VkRenderPassBeginInfo-commonparent");
        skip |= DeviceValidateObject(commandBuffer, pRenderPassBegin->framebuffer, kVulkanObjectTypeFramebuffer, false, "VUID-VkRenderPassBeginInfo-framebuffer-parameter", "VUID-VkRenderPassBeginInfo-commonparent");
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdNextSubpass2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkSubpassBeginInfoKHR*                pSubpassBeginInfo,
    const VkSubpassEndInfoKHR*                  pSubpassEndInfo) {
    bool skip = false;
    skip |= DeviceValidateObject(commandBuffer, commandBuffer, kVulkanObjectTypeCommandBuffer, false, "VUID-vkCmdNextSubpass2KHR-commandBuffer-parameter", kVUIDUndefined);

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdEndRenderPass2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkSubpassEndInfoKHR*                  pSubpassEndInfo) {
    bool skip = false;
    skip |= DeviceValidateObject(commandBuffer, commandBuffer, kVulkanObjectTypeCommandBuffer, false, "VUID-vkCmdEndRenderPass2KHR-commandBuffer-parameter", kVUIDUndefined);

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetSwapchainStatusKHR(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkGetSwapchainStatusKHR-device-parameter", "VUID-vkGetSwapchainStatusKHR-commonparent");
    skip |= DeviceValidateObject(device, swapchain, kVulkanObjectTypeSwapchainKHR, false, "VUID-vkGetSwapchainStatusKHR-swapchain-parameter", "VUID-vkGetSwapchainStatusKHR-commonparent");

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetPhysicalDeviceExternalFencePropertiesKHR(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceExternalFenceInfo*    pExternalFenceInfo,
    VkExternalFenceProperties*                  pExternalFenceProperties) {
    bool skip = false;
    skip |= InstanceValidateObject(physicalDevice, physicalDevice, kVulkanObjectTypePhysicalDevice, false, "VUID-vkGetPhysicalDeviceExternalFencePropertiesKHR-physicalDevice-parameter", kVUIDUndefined);

    return skip;
}

#ifdef VK_USE_PLATFORM_WIN32_KHR

bool ObjectLifetimes::PreCallValidateImportFenceWin32HandleKHR(
    VkDevice                                    device,
    const VkImportFenceWin32HandleInfoKHR*      pImportFenceWin32HandleInfo) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkImportFenceWin32HandleKHR-device-parameter", kVUIDUndefined);
    if (pImportFenceWin32HandleInfo) {
        skip |= DeviceValidateObject(device, pImportFenceWin32HandleInfo->fence, kVulkanObjectTypeFence, false, "VUID-VkImportFenceWin32HandleInfoKHR-fence-parameter", kVUIDUndefined);
    }

    return skip;
}
#endif // VK_USE_PLATFORM_WIN32_KHR

#ifdef VK_USE_PLATFORM_WIN32_KHR

bool ObjectLifetimes::PreCallValidateGetFenceWin32HandleKHR(
    VkDevice                                    device,
    const VkFenceGetWin32HandleInfoKHR*         pGetWin32HandleInfo,
    HANDLE*                                     pHandle) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkGetFenceWin32HandleKHR-device-parameter", kVUIDUndefined);
    if (pGetWin32HandleInfo) {
        skip |= DeviceValidateObject(device, pGetWin32HandleInfo->fence, kVulkanObjectTypeFence, false, "VUID-VkFenceGetWin32HandleInfoKHR-fence-parameter", kVUIDUndefined);
    }

    return skip;
}
#endif // VK_USE_PLATFORM_WIN32_KHR

bool ObjectLifetimes::PreCallValidateImportFenceFdKHR(
    VkDevice                                    device,
    const VkImportFenceFdInfoKHR*               pImportFenceFdInfo) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkImportFenceFdKHR-device-parameter", kVUIDUndefined);
    if (pImportFenceFdInfo) {
        skip |= DeviceValidateObject(device, pImportFenceFdInfo->fence, kVulkanObjectTypeFence, false, "VUID-VkImportFenceFdInfoKHR-fence-parameter", kVUIDUndefined);
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetFenceFdKHR(
    VkDevice                                    device,
    const VkFenceGetFdInfoKHR*                  pGetFdInfo,
    int*                                        pFd) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkGetFenceFdKHR-device-parameter", kVUIDUndefined);
    if (pGetFdInfo) {
        skip |= DeviceValidateObject(device, pGetFdInfo->fence, kVulkanObjectTypeFence, false, "VUID-VkFenceGetFdInfoKHR-fence-parameter", kVUIDUndefined);
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetPhysicalDeviceSurfaceCapabilities2KHR(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceSurfaceInfo2KHR*      pSurfaceInfo,
    VkSurfaceCapabilities2KHR*                  pSurfaceCapabilities) {
    bool skip = false;
    skip |= InstanceValidateObject(physicalDevice, physicalDevice, kVulkanObjectTypePhysicalDevice, false, "VUID-vkGetPhysicalDeviceSurfaceCapabilities2KHR-physicalDevice-parameter", kVUIDUndefined);
    if (pSurfaceInfo) {
        skip |= InstanceValidateObject(physicalDevice, pSurfaceInfo->surface, kVulkanObjectTypeSurfaceKHR, false, "VUID-VkPhysicalDeviceSurfaceInfo2KHR-surface-parameter", kVUIDUndefined);
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetPhysicalDeviceSurfaceFormats2KHR(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceSurfaceInfo2KHR*      pSurfaceInfo,
    uint32_t*                                   pSurfaceFormatCount,
    VkSurfaceFormat2KHR*                        pSurfaceFormats) {
    bool skip = false;
    skip |= InstanceValidateObject(physicalDevice, physicalDevice, kVulkanObjectTypePhysicalDevice, false, "VUID-vkGetPhysicalDeviceSurfaceFormats2KHR-physicalDevice-parameter", kVUIDUndefined);
    if (pSurfaceInfo) {
        skip |= InstanceValidateObject(physicalDevice, pSurfaceInfo->surface, kVulkanObjectTypeSurfaceKHR, false, "VUID-VkPhysicalDeviceSurfaceInfo2KHR-surface-parameter", kVUIDUndefined);
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetPhysicalDeviceDisplayPlaneProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pPropertyCount,
    VkDisplayPlaneProperties2KHR*               pProperties) {
    bool skip = false;
    skip |= InstanceValidateObject(physicalDevice, physicalDevice, kVulkanObjectTypePhysicalDevice, false, "VUID-vkGetPhysicalDeviceDisplayPlaneProperties2KHR-physicalDevice-parameter", kVUIDUndefined);

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetDisplayPlaneCapabilities2KHR(
    VkPhysicalDevice                            physicalDevice,
    const VkDisplayPlaneInfo2KHR*               pDisplayPlaneInfo,
    VkDisplayPlaneCapabilities2KHR*             pCapabilities) {
    bool skip = false;
    skip |= InstanceValidateObject(physicalDevice, physicalDevice, kVulkanObjectTypePhysicalDevice, false, "VUID-vkGetDisplayPlaneCapabilities2KHR-physicalDevice-parameter", kVUIDUndefined);
    if (pDisplayPlaneInfo) {
        skip |= InstanceValidateObject(physicalDevice, pDisplayPlaneInfo->mode, kVulkanObjectTypeDisplayModeKHR, false, "VUID-VkDisplayPlaneInfo2KHR-mode-parameter", kVUIDUndefined);
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetImageMemoryRequirements2KHR(
    VkDevice                                    device,
    const VkImageMemoryRequirementsInfo2*       pInfo,
    VkMemoryRequirements2*                      pMemoryRequirements) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkGetImageMemoryRequirements2KHR-device-parameter", kVUIDUndefined);
    if (pInfo) {
        skip |= DeviceValidateObject(device, pInfo->image, kVulkanObjectTypeImage, false, "VUID-VkImageMemoryRequirementsInfo2-image-parameter", kVUIDUndefined);
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetBufferMemoryRequirements2KHR(
    VkDevice                                    device,
    const VkBufferMemoryRequirementsInfo2*      pInfo,
    VkMemoryRequirements2*                      pMemoryRequirements) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkGetBufferMemoryRequirements2KHR-device-parameter", kVUIDUndefined);
    if (pInfo) {
        skip |= DeviceValidateObject(device, pInfo->buffer, kVulkanObjectTypeBuffer, false, "VUID-VkBufferMemoryRequirementsInfo2-buffer-parameter", kVUIDUndefined);
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetImageSparseMemoryRequirements2KHR(
    VkDevice                                    device,
    const VkImageSparseMemoryRequirementsInfo2* pInfo,
    uint32_t*                                   pSparseMemoryRequirementCount,
    VkSparseImageMemoryRequirements2*           pSparseMemoryRequirements) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkGetImageSparseMemoryRequirements2KHR-device-parameter", kVUIDUndefined);
    if (pInfo) {
        skip |= DeviceValidateObject(device, pInfo->image, kVulkanObjectTypeImage, false, "VUID-VkImageSparseMemoryRequirementsInfo2-image-parameter", kVUIDUndefined);
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateCreateSamplerYcbcrConversionKHR(
    VkDevice                                    device,
    const VkSamplerYcbcrConversionCreateInfo*   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSamplerYcbcrConversion*                   pYcbcrConversion) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkCreateSamplerYcbcrConversionKHR-device-parameter", kVUIDUndefined);

    return skip;
}

void ObjectLifetimes::PostCallRecordCreateSamplerYcbcrConversionKHR(
    VkDevice                                    device,
    const VkSamplerYcbcrConversionCreateInfo*   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSamplerYcbcrConversion*                   pYcbcrConversion) {
    DeviceCreateObject(device, *pYcbcrConversion, kVulkanObjectTypeSamplerYcbcrConversion, pAllocator);

}

bool ObjectLifetimes::PreCallValidateDestroySamplerYcbcrConversionKHR(
    VkDevice                                    device,
    VkSamplerYcbcrConversion                    ycbcrConversion,
    const VkAllocationCallbacks*                pAllocator) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkDestroySamplerYcbcrConversionKHR-device-parameter", kVUIDUndefined);
    skip |= DeviceValidateObject(device, ycbcrConversion, kVulkanObjectTypeSamplerYcbcrConversion, true, "VUID-vkDestroySamplerYcbcrConversionKHR-ycbcrConversion-parameter", "VUID-vkDestroySamplerYcbcrConversionKHR-ycbcrConversion-parent");
    skip |= DeviceValidateDestroyObject(device, ycbcrConversion, kVulkanObjectTypeSamplerYcbcrConversion, pAllocator, kVUIDUndefined, kVUIDUndefined);

    return skip;
}

void ObjectLifetimes::PreCallRecordDestroySamplerYcbcrConversionKHR(
    VkDevice                                    device,
    VkSamplerYcbcrConversion                    ycbcrConversion,
    const VkAllocationCallbacks*                pAllocator) {
    DeviceRecordDestroyObject(device, ycbcrConversion, kVulkanObjectTypeSamplerYcbcrConversion);

}

bool ObjectLifetimes::PreCallValidateBindBufferMemory2KHR(
    VkDevice                                    device,
    uint32_t                                    bindInfoCount,
    const VkBindBufferMemoryInfo*               pBindInfos) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkBindBufferMemory2KHR-device-parameter", kVUIDUndefined);
    if (pBindInfos) {
        for (uint32_t index0 = 0; index0 < bindInfoCount; ++index0) {
            skip |= DeviceValidateObject(device, pBindInfos[index0].buffer, kVulkanObjectTypeBuffer, false, "VUID-VkBindBufferMemoryInfo-buffer-parameter", "VUID-VkBindBufferMemoryInfo-commonparent");
            skip |= DeviceValidateObject(device, pBindInfos[index0].memory, kVulkanObjectTypeDeviceMemory, false, "VUID-VkBindBufferMemoryInfo-memory-parameter", "VUID-VkBindBufferMemoryInfo-commonparent");
        }
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateBindImageMemory2KHR(
    VkDevice                                    device,
    uint32_t                                    bindInfoCount,
    const VkBindImageMemoryInfo*                pBindInfos) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkBindImageMemory2KHR-device-parameter", kVUIDUndefined);
    if (pBindInfos) {
        for (uint32_t index0 = 0; index0 < bindInfoCount; ++index0) {
            skip |= DeviceValidateObject(device, pBindInfos[index0].image, kVulkanObjectTypeImage, false, "VUID-VkBindImageMemoryInfo-image-parameter", "VUID-VkBindImageMemoryInfo-commonparent");
            skip |= DeviceValidateObject(device, pBindInfos[index0].memory, kVulkanObjectTypeDeviceMemory, true, kVUIDUndefined, "VUID-VkBindImageMemoryInfo-commonparent");
        }
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdDrawIndirectCountKHR(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkBuffer                                    countBuffer,
    VkDeviceSize                                countBufferOffset,
    uint32_t                                    maxDrawCount,
    uint32_t                                    stride) {
    bool skip = false;
    skip |= DeviceValidateObject(commandBuffer, commandBuffer, kVulkanObjectTypeCommandBuffer, false, "VUID-vkCmdDrawIndirectCountKHR-commandBuffer-parameter", "VUID-vkCmdDrawIndirectCountKHR-commonparent");
    skip |= DeviceValidateObject(commandBuffer, buffer, kVulkanObjectTypeBuffer, false, "VUID-vkCmdDrawIndirectCountKHR-buffer-parameter", "VUID-vkCmdDrawIndirectCountKHR-commonparent");
    skip |= DeviceValidateObject(commandBuffer, countBuffer, kVulkanObjectTypeBuffer, false, "VUID-vkCmdDrawIndirectCountKHR-countBuffer-parameter", "VUID-vkCmdDrawIndirectCountKHR-commonparent");

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdDrawIndexedIndirectCountKHR(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkBuffer                                    countBuffer,
    VkDeviceSize                                countBufferOffset,
    uint32_t                                    maxDrawCount,
    uint32_t                                    stride) {
    bool skip = false;
    skip |= DeviceValidateObject(commandBuffer, commandBuffer, kVulkanObjectTypeCommandBuffer, false, "VUID-vkCmdDrawIndexedIndirectCountKHR-commandBuffer-parameter", "VUID-vkCmdDrawIndexedIndirectCountKHR-commonparent");
    skip |= DeviceValidateObject(commandBuffer, buffer, kVulkanObjectTypeBuffer, false, "VUID-vkCmdDrawIndexedIndirectCountKHR-buffer-parameter", "VUID-vkCmdDrawIndexedIndirectCountKHR-commonparent");
    skip |= DeviceValidateObject(commandBuffer, countBuffer, kVulkanObjectTypeBuffer, false, "VUID-vkCmdDrawIndexedIndirectCountKHR-countBuffer-parameter", "VUID-vkCmdDrawIndexedIndirectCountKHR-commonparent");

    return skip;
}

bool ObjectLifetimes::PreCallValidateCreateDebugReportCallbackEXT(
    VkInstance                                  instance,
    const VkDebugReportCallbackCreateInfoEXT*   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDebugReportCallbackEXT*                   pCallback) {
    bool skip = false;
    skip |= InstanceValidateObject(instance, instance, kVulkanObjectTypeInstance, false, "VUID-vkCreateDebugReportCallbackEXT-instance-parameter", kVUIDUndefined);

    return skip;
}

void ObjectLifetimes::PostCallRecordCreateDebugReportCallbackEXT(
    VkInstance                                  instance,
    const VkDebugReportCallbackCreateInfoEXT*   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDebugReportCallbackEXT*                   pCallback) {
    InstanceCreateObject(instance, *pCallback, kVulkanObjectTypeDebugReportCallbackEXT, pAllocator);

}

bool ObjectLifetimes::PreCallValidateDestroyDebugReportCallbackEXT(
    VkInstance                                  instance,
    VkDebugReportCallbackEXT                    callback,
    const VkAllocationCallbacks*                pAllocator) {
    bool skip = false;
    skip |= InstanceValidateObject(instance, instance, kVulkanObjectTypeInstance, false, "VUID-vkDestroyDebugReportCallbackEXT-instance-parameter", kVUIDUndefined);
    skip |= InstanceValidateObject(instance, callback, kVulkanObjectTypeDebugReportCallbackEXT, false, "VUID-vkDestroyDebugReportCallbackEXT-callback-parameter", "VUID-vkDestroyDebugReportCallbackEXT-callback-parent");
    skip |= InstanceValidateDestroyObject(instance, callback, kVulkanObjectTypeDebugReportCallbackEXT, pAllocator, kVUIDUndefined, kVUIDUndefined);

    return skip;
}

void ObjectLifetimes::PreCallRecordDestroyDebugReportCallbackEXT(
    VkInstance                                  instance,
    VkDebugReportCallbackEXT                    callback,
    const VkAllocationCallbacks*                pAllocator) {
    InstanceRecordDestroyObject(instance, callback, kVulkanObjectTypeDebugReportCallbackEXT);

}

bool ObjectLifetimes::PreCallValidateDebugReportMessageEXT(
    VkInstance                                  instance,
    VkDebugReportFlagsEXT                       flags,
    VkDebugReportObjectTypeEXT                  objectType,
    uint64_t                                    object,
    size_t                                      location,
    int32_t                                     messageCode,
    const char*                                 pLayerPrefix,
    const char*                                 pMessage) {
    bool skip = false;
    skip |= InstanceValidateObject(instance, instance, kVulkanObjectTypeInstance, false, "VUID-vkDebugReportMessageEXT-instance-parameter", kVUIDUndefined);

    return skip;
}

bool ObjectLifetimes::PreCallValidateDebugMarkerSetObjectTagEXT(
    VkDevice                                    device,
    const VkDebugMarkerObjectTagInfoEXT*        pTagInfo) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkDebugMarkerSetObjectTagEXT-device-parameter", kVUIDUndefined);

    return skip;
}

bool ObjectLifetimes::PreCallValidateDebugMarkerSetObjectNameEXT(
    VkDevice                                    device,
    const VkDebugMarkerObjectNameInfoEXT*       pNameInfo) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkDebugMarkerSetObjectNameEXT-device-parameter", kVUIDUndefined);

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdDebugMarkerBeginEXT(
    VkCommandBuffer                             commandBuffer,
    const VkDebugMarkerMarkerInfoEXT*           pMarkerInfo) {
    bool skip = false;
    skip |= DeviceValidateObject(commandBuffer, commandBuffer, kVulkanObjectTypeCommandBuffer, false, "VUID-vkCmdDebugMarkerBeginEXT-commandBuffer-parameter", kVUIDUndefined);

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdDebugMarkerEndEXT(
    VkCommandBuffer                             commandBuffer) {
    bool skip = false;
    skip |= DeviceValidateObject(commandBuffer, commandBuffer, kVulkanObjectTypeCommandBuffer, false, "VUID-vkCmdDebugMarkerEndEXT-commandBuffer-parameter", kVUIDUndefined);

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdDebugMarkerInsertEXT(
    VkCommandBuffer                             commandBuffer,
    const VkDebugMarkerMarkerInfoEXT*           pMarkerInfo) {
    bool skip = false;
    skip |= DeviceValidateObject(commandBuffer, commandBuffer, kVulkanObjectTypeCommandBuffer, false, "VUID-vkCmdDebugMarkerInsertEXT-commandBuffer-parameter", kVUIDUndefined);

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdDrawIndirectCountAMD(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkBuffer                                    countBuffer,
    VkDeviceSize                                countBufferOffset,
    uint32_t                                    maxDrawCount,
    uint32_t                                    stride) {
    bool skip = false;
    skip |= DeviceValidateObject(commandBuffer, commandBuffer, kVulkanObjectTypeCommandBuffer, false, "VUID-vkCmdDrawIndirectCountAMD-commandBuffer-parameter", "VUID-vkCmdDrawIndirectCountAMD-commonparent");
    skip |= DeviceValidateObject(commandBuffer, buffer, kVulkanObjectTypeBuffer, false, "VUID-vkCmdDrawIndirectCountAMD-buffer-parameter", "VUID-vkCmdDrawIndirectCountAMD-commonparent");
    skip |= DeviceValidateObject(commandBuffer, countBuffer, kVulkanObjectTypeBuffer, false, "VUID-vkCmdDrawIndirectCountAMD-countBuffer-parameter", "VUID-vkCmdDrawIndirectCountAMD-commonparent");

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdDrawIndexedIndirectCountAMD(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkBuffer                                    countBuffer,
    VkDeviceSize                                countBufferOffset,
    uint32_t                                    maxDrawCount,
    uint32_t                                    stride) {
    bool skip = false;
    skip |= DeviceValidateObject(commandBuffer, commandBuffer, kVulkanObjectTypeCommandBuffer, false, "VUID-vkCmdDrawIndexedIndirectCountAMD-commandBuffer-parameter", "VUID-vkCmdDrawIndexedIndirectCountAMD-commonparent");
    skip |= DeviceValidateObject(commandBuffer, buffer, kVulkanObjectTypeBuffer, false, "VUID-vkCmdDrawIndexedIndirectCountAMD-buffer-parameter", "VUID-vkCmdDrawIndexedIndirectCountAMD-commonparent");
    skip |= DeviceValidateObject(commandBuffer, countBuffer, kVulkanObjectTypeBuffer, false, "VUID-vkCmdDrawIndexedIndirectCountAMD-countBuffer-parameter", "VUID-vkCmdDrawIndexedIndirectCountAMD-commonparent");

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetShaderInfoAMD(
    VkDevice                                    device,
    VkPipeline                                  pipeline,
    VkShaderStageFlagBits                       shaderStage,
    VkShaderInfoTypeAMD                         infoType,
    size_t*                                     pInfoSize,
    void*                                       pInfo) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkGetShaderInfoAMD-device-parameter", kVUIDUndefined);
    skip |= DeviceValidateObject(device, pipeline, kVulkanObjectTypePipeline, false, "VUID-vkGetShaderInfoAMD-pipeline-parameter", "VUID-vkGetShaderInfoAMD-pipeline-parent");

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetPhysicalDeviceExternalImageFormatPropertiesNV(
    VkPhysicalDevice                            physicalDevice,
    VkFormat                                    format,
    VkImageType                                 type,
    VkImageTiling                               tiling,
    VkImageUsageFlags                           usage,
    VkImageCreateFlags                          flags,
    VkExternalMemoryHandleTypeFlagsNV           externalHandleType,
    VkExternalImageFormatPropertiesNV*          pExternalImageFormatProperties) {
    bool skip = false;
    skip |= InstanceValidateObject(physicalDevice, physicalDevice, kVulkanObjectTypePhysicalDevice, false, "VUID-vkGetPhysicalDeviceExternalImageFormatPropertiesNV-physicalDevice-parameter", kVUIDUndefined);

    return skip;
}

#ifdef VK_USE_PLATFORM_WIN32_KHR

bool ObjectLifetimes::PreCallValidateGetMemoryWin32HandleNV(
    VkDevice                                    device,
    VkDeviceMemory                              memory,
    VkExternalMemoryHandleTypeFlagsNV           handleType,
    HANDLE*                                     pHandle) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkGetMemoryWin32HandleNV-device-parameter", kVUIDUndefined);
    skip |= DeviceValidateObject(device, memory, kVulkanObjectTypeDeviceMemory, false, "VUID-vkGetMemoryWin32HandleNV-memory-parameter", "VUID-vkGetMemoryWin32HandleNV-memory-parent");

    return skip;
}
#endif // VK_USE_PLATFORM_WIN32_KHR

#ifdef VK_USE_PLATFORM_VI_NN

bool ObjectLifetimes::PreCallValidateCreateViSurfaceNN(
    VkInstance                                  instance,
    const VkViSurfaceCreateInfoNN*              pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) {
    bool skip = false;
    skip |= InstanceValidateObject(instance, instance, kVulkanObjectTypeInstance, false, "VUID-vkCreateViSurfaceNN-instance-parameter", kVUIDUndefined);

    return skip;
}

void ObjectLifetimes::PostCallRecordCreateViSurfaceNN(
    VkInstance                                  instance,
    const VkViSurfaceCreateInfoNN*              pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) {
    InstanceCreateObject(instance, *pSurface, kVulkanObjectTypeSurfaceKHR, pAllocator);

}
#endif // VK_USE_PLATFORM_VI_NN

bool ObjectLifetimes::PreCallValidateCmdBeginConditionalRenderingEXT(
    VkCommandBuffer                             commandBuffer,
    const VkConditionalRenderingBeginInfoEXT*   pConditionalRenderingBegin) {
    bool skip = false;
    skip |= DeviceValidateObject(commandBuffer, commandBuffer, kVulkanObjectTypeCommandBuffer, false, "VUID-vkCmdBeginConditionalRenderingEXT-commandBuffer-parameter", kVUIDUndefined);
    if (pConditionalRenderingBegin) {
        skip |= DeviceValidateObject(commandBuffer, pConditionalRenderingBegin->buffer, kVulkanObjectTypeBuffer, false, kVUIDUndefined, kVUIDUndefined);
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdEndConditionalRenderingEXT(
    VkCommandBuffer                             commandBuffer) {
    bool skip = false;
    skip |= DeviceValidateObject(commandBuffer, commandBuffer, kVulkanObjectTypeCommandBuffer, false, "VUID-vkCmdEndConditionalRenderingEXT-commandBuffer-parameter", kVUIDUndefined);

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdProcessCommandsNVX(
    VkCommandBuffer                             commandBuffer,
    const VkCmdProcessCommandsInfoNVX*          pProcessCommandsInfo) {
    bool skip = false;
    skip |= DeviceValidateObject(commandBuffer, commandBuffer, kVulkanObjectTypeCommandBuffer, false, "VUID-vkCmdProcessCommandsNVX-commandBuffer-parameter", kVUIDUndefined);
    if (pProcessCommandsInfo) {
        skip |= DeviceValidateObject(commandBuffer, pProcessCommandsInfo->objectTable, kVulkanObjectTypeObjectTableNVX, false, "VUID-VkCmdProcessCommandsInfoNVX-objectTable-parameter", "VUID-VkCmdProcessCommandsInfoNVX-commonparent");
        skip |= DeviceValidateObject(commandBuffer, pProcessCommandsInfo->indirectCommandsLayout, kVulkanObjectTypeIndirectCommandsLayoutNVX, false, "VUID-VkCmdProcessCommandsInfoNVX-indirectCommandsLayout-parameter", "VUID-VkCmdProcessCommandsInfoNVX-commonparent");
        if (pProcessCommandsInfo->pIndirectCommandsTokens) {
            for (uint32_t index1 = 0; index1 < pProcessCommandsInfo->indirectCommandsTokenCount; ++index1) {
                skip |= DeviceValidateObject(commandBuffer, pProcessCommandsInfo->pIndirectCommandsTokens[index1].buffer, kVulkanObjectTypeBuffer, false, "VUID-VkIndirectCommandsTokenNVX-buffer-parameter", kVUIDUndefined);
            }
        }
        skip |= DeviceValidateObject(commandBuffer, pProcessCommandsInfo->targetCommandBuffer, kVulkanObjectTypeCommandBuffer, true, "VUID-VkCmdProcessCommandsInfoNVX-targetCommandBuffer-parameter", "VUID-VkCmdProcessCommandsInfoNVX-commonparent");
        skip |= DeviceValidateObject(commandBuffer, pProcessCommandsInfo->sequencesCountBuffer, kVulkanObjectTypeBuffer, true, "VUID-VkCmdProcessCommandsInfoNVX-sequencesCountBuffer-parameter", "VUID-VkCmdProcessCommandsInfoNVX-commonparent");
        skip |= DeviceValidateObject(commandBuffer, pProcessCommandsInfo->sequencesIndexBuffer, kVulkanObjectTypeBuffer, true, "VUID-VkCmdProcessCommandsInfoNVX-sequencesIndexBuffer-parameter", "VUID-VkCmdProcessCommandsInfoNVX-commonparent");
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdReserveSpaceForCommandsNVX(
    VkCommandBuffer                             commandBuffer,
    const VkCmdReserveSpaceForCommandsInfoNVX*  pReserveSpaceInfo) {
    bool skip = false;
    skip |= DeviceValidateObject(commandBuffer, commandBuffer, kVulkanObjectTypeCommandBuffer, false, "VUID-vkCmdReserveSpaceForCommandsNVX-commandBuffer-parameter", kVUIDUndefined);
    if (pReserveSpaceInfo) {
        skip |= DeviceValidateObject(commandBuffer, pReserveSpaceInfo->objectTable, kVulkanObjectTypeObjectTableNVX, false, "VUID-VkCmdReserveSpaceForCommandsInfoNVX-objectTable-parameter", "VUID-VkCmdReserveSpaceForCommandsInfoNVX-commonparent");
        skip |= DeviceValidateObject(commandBuffer, pReserveSpaceInfo->indirectCommandsLayout, kVulkanObjectTypeIndirectCommandsLayoutNVX, false, "VUID-VkCmdReserveSpaceForCommandsInfoNVX-indirectCommandsLayout-parameter", "VUID-VkCmdReserveSpaceForCommandsInfoNVX-commonparent");
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateCreateIndirectCommandsLayoutNVX(
    VkDevice                                    device,
    const VkIndirectCommandsLayoutCreateInfoNVX* pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkIndirectCommandsLayoutNVX*                pIndirectCommandsLayout) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkCreateIndirectCommandsLayoutNVX-device-parameter", kVUIDUndefined);

    return skip;
}

void ObjectLifetimes::PostCallRecordCreateIndirectCommandsLayoutNVX(
    VkDevice                                    device,
    const VkIndirectCommandsLayoutCreateInfoNVX* pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkIndirectCommandsLayoutNVX*                pIndirectCommandsLayout) {
    DeviceCreateObject(device, *pIndirectCommandsLayout, kVulkanObjectTypeIndirectCommandsLayoutNVX, pAllocator);

}

bool ObjectLifetimes::PreCallValidateDestroyIndirectCommandsLayoutNVX(
    VkDevice                                    device,
    VkIndirectCommandsLayoutNVX                 indirectCommandsLayout,
    const VkAllocationCallbacks*                pAllocator) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkDestroyIndirectCommandsLayoutNVX-device-parameter", kVUIDUndefined);
    skip |= DeviceValidateObject(device, indirectCommandsLayout, kVulkanObjectTypeIndirectCommandsLayoutNVX, false, "VUID-vkDestroyIndirectCommandsLayoutNVX-indirectCommandsLayout-parameter", "VUID-vkDestroyIndirectCommandsLayoutNVX-indirectCommandsLayout-parent");
    skip |= DeviceValidateDestroyObject(device, indirectCommandsLayout, kVulkanObjectTypeIndirectCommandsLayoutNVX, pAllocator, kVUIDUndefined, kVUIDUndefined);

    return skip;
}

void ObjectLifetimes::PreCallRecordDestroyIndirectCommandsLayoutNVX(
    VkDevice                                    device,
    VkIndirectCommandsLayoutNVX                 indirectCommandsLayout,
    const VkAllocationCallbacks*                pAllocator) {
    DeviceRecordDestroyObject(device, indirectCommandsLayout, kVulkanObjectTypeIndirectCommandsLayoutNVX);

}

bool ObjectLifetimes::PreCallValidateCreateObjectTableNVX(
    VkDevice                                    device,
    const VkObjectTableCreateInfoNVX*           pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkObjectTableNVX*                           pObjectTable) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkCreateObjectTableNVX-device-parameter", kVUIDUndefined);

    return skip;
}

void ObjectLifetimes::PostCallRecordCreateObjectTableNVX(
    VkDevice                                    device,
    const VkObjectTableCreateInfoNVX*           pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkObjectTableNVX*                           pObjectTable) {
    DeviceCreateObject(device, *pObjectTable, kVulkanObjectTypeObjectTableNVX, pAllocator);

}

bool ObjectLifetimes::PreCallValidateDestroyObjectTableNVX(
    VkDevice                                    device,
    VkObjectTableNVX                            objectTable,
    const VkAllocationCallbacks*                pAllocator) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkDestroyObjectTableNVX-device-parameter", kVUIDUndefined);
    skip |= DeviceValidateObject(device, objectTable, kVulkanObjectTypeObjectTableNVX, false, "VUID-vkDestroyObjectTableNVX-objectTable-parameter", "VUID-vkDestroyObjectTableNVX-objectTable-parent");
    skip |= DeviceValidateDestroyObject(device, objectTable, kVulkanObjectTypeObjectTableNVX, pAllocator, kVUIDUndefined, kVUIDUndefined);

    return skip;
}

void ObjectLifetimes::PreCallRecordDestroyObjectTableNVX(
    VkDevice                                    device,
    VkObjectTableNVX                            objectTable,
    const VkAllocationCallbacks*                pAllocator) {
    DeviceRecordDestroyObject(device, objectTable, kVulkanObjectTypeObjectTableNVX);

}

bool ObjectLifetimes::PreCallValidateRegisterObjectsNVX(
    VkDevice                                    device,
    VkObjectTableNVX                            objectTable,
    uint32_t                                    objectCount,
    const VkObjectTableEntryNVX* const*         ppObjectTableEntries,
    const uint32_t*                             pObjectIndices) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkRegisterObjectsNVX-device-parameter", kVUIDUndefined);
    skip |= DeviceValidateObject(device, objectTable, kVulkanObjectTypeObjectTableNVX, false, "VUID-vkRegisterObjectsNVX-objectTable-parameter", "VUID-vkRegisterObjectsNVX-objectTable-parent");

    return skip;
}

bool ObjectLifetimes::PreCallValidateUnregisterObjectsNVX(
    VkDevice                                    device,
    VkObjectTableNVX                            objectTable,
    uint32_t                                    objectCount,
    const VkObjectEntryTypeNVX*                 pObjectEntryTypes,
    const uint32_t*                             pObjectIndices) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkUnregisterObjectsNVX-device-parameter", kVUIDUndefined);
    skip |= DeviceValidateObject(device, objectTable, kVulkanObjectTypeObjectTableNVX, false, "VUID-vkUnregisterObjectsNVX-objectTable-parameter", "VUID-vkUnregisterObjectsNVX-objectTable-parent");

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetPhysicalDeviceGeneratedCommandsPropertiesNVX(
    VkPhysicalDevice                            physicalDevice,
    VkDeviceGeneratedCommandsFeaturesNVX*       pFeatures,
    VkDeviceGeneratedCommandsLimitsNVX*         pLimits) {
    bool skip = false;
    skip |= InstanceValidateObject(physicalDevice, physicalDevice, kVulkanObjectTypePhysicalDevice, false, "VUID-vkGetPhysicalDeviceGeneratedCommandsPropertiesNVX-physicalDevice-parameter", kVUIDUndefined);

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdSetViewportWScalingNV(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstViewport,
    uint32_t                                    viewportCount,
    const VkViewportWScalingNV*                 pViewportWScalings) {
    bool skip = false;
    skip |= DeviceValidateObject(commandBuffer, commandBuffer, kVulkanObjectTypeCommandBuffer, false, "VUID-vkCmdSetViewportWScalingNV-commandBuffer-parameter", kVUIDUndefined);

    return skip;
}

bool ObjectLifetimes::PreCallValidateReleaseDisplayEXT(
    VkPhysicalDevice                            physicalDevice,
    VkDisplayKHR                                display) {
    bool skip = false;
    skip |= InstanceValidateObject(physicalDevice, physicalDevice, kVulkanObjectTypePhysicalDevice, false, "VUID-vkReleaseDisplayEXT-physicalDevice-parameter", kVUIDUndefined);
    skip |= InstanceValidateObject(physicalDevice, display, kVulkanObjectTypeDisplayKHR, false, "VUID-vkReleaseDisplayEXT-display-parameter", kVUIDUndefined);

    return skip;
}

#ifdef VK_USE_PLATFORM_XLIB_XRANDR_EXT

bool ObjectLifetimes::PreCallValidateAcquireXlibDisplayEXT(
    VkPhysicalDevice                            physicalDevice,
    Display*                                    dpy,
    VkDisplayKHR                                display) {
    bool skip = false;
    skip |= InstanceValidateObject(physicalDevice, physicalDevice, kVulkanObjectTypePhysicalDevice, false, "VUID-vkAcquireXlibDisplayEXT-physicalDevice-parameter", kVUIDUndefined);
    skip |= InstanceValidateObject(physicalDevice, display, kVulkanObjectTypeDisplayKHR, false, "VUID-vkAcquireXlibDisplayEXT-display-parameter", kVUIDUndefined);

    return skip;
}
#endif // VK_USE_PLATFORM_XLIB_XRANDR_EXT

#ifdef VK_USE_PLATFORM_XLIB_XRANDR_EXT

bool ObjectLifetimes::PreCallValidateGetRandROutputDisplayEXT(
    VkPhysicalDevice                            physicalDevice,
    Display*                                    dpy,
    RROutput                                    rrOutput,
    VkDisplayKHR*                               pDisplay) {
    bool skip = false;
    skip |= InstanceValidateObject(physicalDevice, physicalDevice, kVulkanObjectTypePhysicalDevice, false, "VUID-vkGetRandROutputDisplayEXT-physicalDevice-parameter", kVUIDUndefined);

    return skip;
}

void ObjectLifetimes::PostCallRecordGetRandROutputDisplayEXT(
    VkPhysicalDevice                            physicalDevice,
    Display*                                    dpy,
    RROutput                                    rrOutput,
    VkDisplayKHR*                               pDisplay) {
    InstanceCreateObject(physicalDevice, *pDisplay, kVulkanObjectTypeDisplayKHR, nullptr);

}
#endif // VK_USE_PLATFORM_XLIB_XRANDR_EXT

bool ObjectLifetimes::PreCallValidateGetPhysicalDeviceSurfaceCapabilities2EXT(
    VkPhysicalDevice                            physicalDevice,
    VkSurfaceKHR                                surface,
    VkSurfaceCapabilities2EXT*                  pSurfaceCapabilities) {
    bool skip = false;
    skip |= InstanceValidateObject(physicalDevice, physicalDevice, kVulkanObjectTypePhysicalDevice, false, "VUID-vkGetPhysicalDeviceSurfaceCapabilities2EXT-physicalDevice-parameter", "VUID-vkGetPhysicalDeviceSurfaceCapabilities2EXT-commonparent");
    skip |= InstanceValidateObject(physicalDevice, surface, kVulkanObjectTypeSurfaceKHR, false, "VUID-vkGetPhysicalDeviceSurfaceCapabilities2EXT-surface-parameter", "VUID-vkGetPhysicalDeviceSurfaceCapabilities2EXT-commonparent");

    return skip;
}

bool ObjectLifetimes::PreCallValidateDisplayPowerControlEXT(
    VkDevice                                    device,
    VkDisplayKHR                                display,
    const VkDisplayPowerInfoEXT*                pDisplayPowerInfo) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkDisplayPowerControlEXT-device-parameter", kVUIDUndefined);
    skip |= DeviceValidateObject(device, display, kVulkanObjectTypeDisplayKHR, false, "VUID-vkDisplayPowerControlEXT-display-parameter", kVUIDUndefined);

    return skip;
}

bool ObjectLifetimes::PreCallValidateRegisterDeviceEventEXT(
    VkDevice                                    device,
    const VkDeviceEventInfoEXT*                 pDeviceEventInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkFence*                                    pFence) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkRegisterDeviceEventEXT-device-parameter", kVUIDUndefined);

    return skip;
}

void ObjectLifetimes::PostCallRecordRegisterDeviceEventEXT(
    VkDevice                                    device,
    const VkDeviceEventInfoEXT*                 pDeviceEventInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkFence*                                    pFence) {
    DeviceCreateObject(device, *pFence, kVulkanObjectTypeFence, pAllocator);

}

bool ObjectLifetimes::PreCallValidateRegisterDisplayEventEXT(
    VkDevice                                    device,
    VkDisplayKHR                                display,
    const VkDisplayEventInfoEXT*                pDisplayEventInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkFence*                                    pFence) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkRegisterDisplayEventEXT-device-parameter", kVUIDUndefined);
    skip |= DeviceValidateObject(device, display, kVulkanObjectTypeDisplayKHR, false, "VUID-vkRegisterDisplayEventEXT-display-parameter", kVUIDUndefined);

    return skip;
}

void ObjectLifetimes::PostCallRecordRegisterDisplayEventEXT(
    VkDevice                                    device,
    VkDisplayKHR                                display,
    const VkDisplayEventInfoEXT*                pDisplayEventInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkFence*                                    pFence) {
    DeviceCreateObject(device, *pFence, kVulkanObjectTypeFence, pAllocator);

}

bool ObjectLifetimes::PreCallValidateGetSwapchainCounterEXT(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    VkSurfaceCounterFlagBitsEXT                 counter,
    uint64_t*                                   pCounterValue) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkGetSwapchainCounterEXT-device-parameter", "VUID-vkGetSwapchainCounterEXT-commonparent");
    skip |= DeviceValidateObject(device, swapchain, kVulkanObjectTypeSwapchainKHR, false, "VUID-vkGetSwapchainCounterEXT-swapchain-parameter", "VUID-vkGetSwapchainCounterEXT-commonparent");

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetRefreshCycleDurationGOOGLE(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    VkRefreshCycleDurationGOOGLE*               pDisplayTimingProperties) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkGetRefreshCycleDurationGOOGLE-device-parameter", "VUID-vkGetRefreshCycleDurationGOOGLE-commonparent");
    skip |= DeviceValidateObject(device, swapchain, kVulkanObjectTypeSwapchainKHR, false, "VUID-vkGetRefreshCycleDurationGOOGLE-swapchain-parameter", "VUID-vkGetRefreshCycleDurationGOOGLE-commonparent");

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetPastPresentationTimingGOOGLE(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    uint32_t*                                   pPresentationTimingCount,
    VkPastPresentationTimingGOOGLE*             pPresentationTimings) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkGetPastPresentationTimingGOOGLE-device-parameter", "VUID-vkGetPastPresentationTimingGOOGLE-commonparent");
    skip |= DeviceValidateObject(device, swapchain, kVulkanObjectTypeSwapchainKHR, false, "VUID-vkGetPastPresentationTimingGOOGLE-swapchain-parameter", "VUID-vkGetPastPresentationTimingGOOGLE-commonparent");

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdSetDiscardRectangleEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstDiscardRectangle,
    uint32_t                                    discardRectangleCount,
    const VkRect2D*                             pDiscardRectangles) {
    bool skip = false;
    skip |= DeviceValidateObject(commandBuffer, commandBuffer, kVulkanObjectTypeCommandBuffer, false, "VUID-vkCmdSetDiscardRectangleEXT-commandBuffer-parameter", kVUIDUndefined);

    return skip;
}

bool ObjectLifetimes::PreCallValidateSetHdrMetadataEXT(
    VkDevice                                    device,
    uint32_t                                    swapchainCount,
    const VkSwapchainKHR*                       pSwapchains,
    const VkHdrMetadataEXT*                     pMetadata) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkSetHdrMetadataEXT-device-parameter", "VUID-vkSetHdrMetadataEXT-commonparent");
    for (uint32_t index0 = 0; index0 < swapchainCount; ++index0) {
        skip |= DeviceValidateObject(device, pSwapchains[index0], kVulkanObjectTypeSwapchainKHR, false, "VUID-vkSetHdrMetadataEXT-pSwapchains-parameter", "VUID-vkSetHdrMetadataEXT-commonparent");
    }

    return skip;
}

#ifdef VK_USE_PLATFORM_IOS_MVK

bool ObjectLifetimes::PreCallValidateCreateIOSSurfaceMVK(
    VkInstance                                  instance,
    const VkIOSSurfaceCreateInfoMVK*            pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) {
    bool skip = false;
    skip |= InstanceValidateObject(instance, instance, kVulkanObjectTypeInstance, false, "VUID-vkCreateIOSSurfaceMVK-instance-parameter", kVUIDUndefined);

    return skip;
}

void ObjectLifetimes::PostCallRecordCreateIOSSurfaceMVK(
    VkInstance                                  instance,
    const VkIOSSurfaceCreateInfoMVK*            pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) {
    InstanceCreateObject(instance, *pSurface, kVulkanObjectTypeSurfaceKHR, pAllocator);

}
#endif // VK_USE_PLATFORM_IOS_MVK

#ifdef VK_USE_PLATFORM_MACOS_MVK

bool ObjectLifetimes::PreCallValidateCreateMacOSSurfaceMVK(
    VkInstance                                  instance,
    const VkMacOSSurfaceCreateInfoMVK*          pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) {
    bool skip = false;
    skip |= InstanceValidateObject(instance, instance, kVulkanObjectTypeInstance, false, "VUID-vkCreateMacOSSurfaceMVK-instance-parameter", kVUIDUndefined);

    return skip;
}

void ObjectLifetimes::PostCallRecordCreateMacOSSurfaceMVK(
    VkInstance                                  instance,
    const VkMacOSSurfaceCreateInfoMVK*          pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) {
    InstanceCreateObject(instance, *pSurface, kVulkanObjectTypeSurfaceKHR, pAllocator);

}
#endif // VK_USE_PLATFORM_MACOS_MVK

bool ObjectLifetimes::PreCallValidateSetDebugUtilsObjectNameEXT(
    VkDevice                                    device,
    const VkDebugUtilsObjectNameInfoEXT*        pNameInfo) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkSetDebugUtilsObjectNameEXT-device-parameter", kVUIDUndefined);

    return skip;
}

bool ObjectLifetimes::PreCallValidateSetDebugUtilsObjectTagEXT(
    VkDevice                                    device,
    const VkDebugUtilsObjectTagInfoEXT*         pTagInfo) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkSetDebugUtilsObjectTagEXT-device-parameter", kVUIDUndefined);

    return skip;
}

bool ObjectLifetimes::PreCallValidateQueueBeginDebugUtilsLabelEXT(
    VkQueue                                     queue,
    const VkDebugUtilsLabelEXT*                 pLabelInfo) {
    bool skip = false;
    skip |= DeviceValidateObject(queue, queue, kVulkanObjectTypeQueue, false, "VUID-vkQueueBeginDebugUtilsLabelEXT-queue-parameter", kVUIDUndefined);

    return skip;
}

bool ObjectLifetimes::PreCallValidateQueueEndDebugUtilsLabelEXT(
    VkQueue                                     queue) {
    bool skip = false;
    skip |= DeviceValidateObject(queue, queue, kVulkanObjectTypeQueue, false, "VUID-vkQueueEndDebugUtilsLabelEXT-queue-parameter", kVUIDUndefined);

    return skip;
}

bool ObjectLifetimes::PreCallValidateQueueInsertDebugUtilsLabelEXT(
    VkQueue                                     queue,
    const VkDebugUtilsLabelEXT*                 pLabelInfo) {
    bool skip = false;
    skip |= DeviceValidateObject(queue, queue, kVulkanObjectTypeQueue, false, "VUID-vkQueueInsertDebugUtilsLabelEXT-queue-parameter", kVUIDUndefined);

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdBeginDebugUtilsLabelEXT(
    VkCommandBuffer                             commandBuffer,
    const VkDebugUtilsLabelEXT*                 pLabelInfo) {
    bool skip = false;
    skip |= DeviceValidateObject(commandBuffer, commandBuffer, kVulkanObjectTypeCommandBuffer, false, "VUID-vkCmdBeginDebugUtilsLabelEXT-commandBuffer-parameter", kVUIDUndefined);

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdEndDebugUtilsLabelEXT(
    VkCommandBuffer                             commandBuffer) {
    bool skip = false;
    skip |= DeviceValidateObject(commandBuffer, commandBuffer, kVulkanObjectTypeCommandBuffer, false, "VUID-vkCmdEndDebugUtilsLabelEXT-commandBuffer-parameter", kVUIDUndefined);

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdInsertDebugUtilsLabelEXT(
    VkCommandBuffer                             commandBuffer,
    const VkDebugUtilsLabelEXT*                 pLabelInfo) {
    bool skip = false;
    skip |= DeviceValidateObject(commandBuffer, commandBuffer, kVulkanObjectTypeCommandBuffer, false, "VUID-vkCmdInsertDebugUtilsLabelEXT-commandBuffer-parameter", kVUIDUndefined);

    return skip;
}

bool ObjectLifetimes::PreCallValidateCreateDebugUtilsMessengerEXT(
    VkInstance                                  instance,
    const VkDebugUtilsMessengerCreateInfoEXT*   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDebugUtilsMessengerEXT*                   pMessenger) {
    bool skip = false;
    skip |= InstanceValidateObject(instance, instance, kVulkanObjectTypeInstance, false, "VUID-vkCreateDebugUtilsMessengerEXT-instance-parameter", kVUIDUndefined);

    return skip;
}

void ObjectLifetimes::PostCallRecordCreateDebugUtilsMessengerEXT(
    VkInstance                                  instance,
    const VkDebugUtilsMessengerCreateInfoEXT*   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDebugUtilsMessengerEXT*                   pMessenger) {
    InstanceCreateObject(instance, *pMessenger, kVulkanObjectTypeDebugUtilsMessengerEXT, pAllocator);

}

bool ObjectLifetimes::PreCallValidateDestroyDebugUtilsMessengerEXT(
    VkInstance                                  instance,
    VkDebugUtilsMessengerEXT                    messenger,
    const VkAllocationCallbacks*                pAllocator) {
    bool skip = false;
    skip |= InstanceValidateObject(instance, instance, kVulkanObjectTypeInstance, false, "VUID-vkDestroyDebugUtilsMessengerEXT-instance-parameter", kVUIDUndefined);
    skip |= InstanceValidateObject(instance, messenger, kVulkanObjectTypeDebugUtilsMessengerEXT, false, "VUID-vkDestroyDebugUtilsMessengerEXT-messenger-parameter", "VUID-vkDestroyDebugUtilsMessengerEXT-messenger-parent");
    skip |= InstanceValidateDestroyObject(instance, messenger, kVulkanObjectTypeDebugUtilsMessengerEXT, pAllocator, kVUIDUndefined, kVUIDUndefined);

    return skip;
}

void ObjectLifetimes::PreCallRecordDestroyDebugUtilsMessengerEXT(
    VkInstance                                  instance,
    VkDebugUtilsMessengerEXT                    messenger,
    const VkAllocationCallbacks*                pAllocator) {
    InstanceRecordDestroyObject(instance, messenger, kVulkanObjectTypeDebugUtilsMessengerEXT);

}

bool ObjectLifetimes::PreCallValidateSubmitDebugUtilsMessageEXT(
    VkInstance                                  instance,
    VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT             messageTypes,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData) {
    bool skip = false;
    skip |= InstanceValidateObject(instance, instance, kVulkanObjectTypeInstance, false, "VUID-vkSubmitDebugUtilsMessageEXT-instance-parameter", kVUIDUndefined);

    return skip;
}

#ifdef VK_USE_PLATFORM_ANDROID_KHR

bool ObjectLifetimes::PreCallValidateGetAndroidHardwareBufferPropertiesANDROID(
    VkDevice                                    device,
    const struct AHardwareBuffer*               buffer,
    VkAndroidHardwareBufferPropertiesANDROID*   pProperties) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkGetAndroidHardwareBufferPropertiesANDROID-device-parameter", kVUIDUndefined);

    return skip;
}
#endif // VK_USE_PLATFORM_ANDROID_KHR

#ifdef VK_USE_PLATFORM_ANDROID_KHR

bool ObjectLifetimes::PreCallValidateGetMemoryAndroidHardwareBufferANDROID(
    VkDevice                                    device,
    const VkMemoryGetAndroidHardwareBufferInfoANDROID* pInfo,
    struct AHardwareBuffer**                    pBuffer) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkGetMemoryAndroidHardwareBufferANDROID-device-parameter", kVUIDUndefined);
    if (pInfo) {
        skip |= DeviceValidateObject(device, pInfo->memory, kVulkanObjectTypeDeviceMemory, false, kVUIDUndefined, kVUIDUndefined);
    }

    return skip;
}
#endif // VK_USE_PLATFORM_ANDROID_KHR

bool ObjectLifetimes::PreCallValidateCmdSetSampleLocationsEXT(
    VkCommandBuffer                             commandBuffer,
    const VkSampleLocationsInfoEXT*             pSampleLocationsInfo) {
    bool skip = false;
    skip |= DeviceValidateObject(commandBuffer, commandBuffer, kVulkanObjectTypeCommandBuffer, false, "VUID-vkCmdSetSampleLocationsEXT-commandBuffer-parameter", kVUIDUndefined);

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetPhysicalDeviceMultisamplePropertiesEXT(
    VkPhysicalDevice                            physicalDevice,
    VkSampleCountFlagBits                       samples,
    VkMultisamplePropertiesEXT*                 pMultisampleProperties) {
    bool skip = false;
    skip |= InstanceValidateObject(physicalDevice, physicalDevice, kVulkanObjectTypePhysicalDevice, false, "VUID-vkGetPhysicalDeviceMultisamplePropertiesEXT-physicalDevice-parameter", kVUIDUndefined);

    return skip;
}

bool ObjectLifetimes::PreCallValidateCreateValidationCacheEXT(
    VkDevice                                    device,
    const VkValidationCacheCreateInfoEXT*       pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkValidationCacheEXT*                       pValidationCache) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkCreateValidationCacheEXT-device-parameter", kVUIDUndefined);

    return skip;
}

void ObjectLifetimes::PostCallRecordCreateValidationCacheEXT(
    VkDevice                                    device,
    const VkValidationCacheCreateInfoEXT*       pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkValidationCacheEXT*                       pValidationCache) {
    DeviceCreateObject(device, *pValidationCache, kVulkanObjectTypeValidationCacheEXT, pAllocator);

}

bool ObjectLifetimes::PreCallValidateDestroyValidationCacheEXT(
    VkDevice                                    device,
    VkValidationCacheEXT                        validationCache,
    const VkAllocationCallbacks*                pAllocator) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkDestroyValidationCacheEXT-device-parameter", kVUIDUndefined);
    skip |= DeviceValidateObject(device, validationCache, kVulkanObjectTypeValidationCacheEXT, true, "VUID-vkDestroyValidationCacheEXT-validationCache-parameter", "VUID-vkDestroyValidationCacheEXT-validationCache-parent");
    skip |= DeviceValidateDestroyObject(device, validationCache, kVulkanObjectTypeValidationCacheEXT, pAllocator, kVUIDUndefined, kVUIDUndefined);

    return skip;
}

void ObjectLifetimes::PreCallRecordDestroyValidationCacheEXT(
    VkDevice                                    device,
    VkValidationCacheEXT                        validationCache,
    const VkAllocationCallbacks*                pAllocator) {
    DeviceRecordDestroyObject(device, validationCache, kVulkanObjectTypeValidationCacheEXT);

}

bool ObjectLifetimes::PreCallValidateMergeValidationCachesEXT(
    VkDevice                                    device,
    VkValidationCacheEXT                        dstCache,
    uint32_t                                    srcCacheCount,
    const VkValidationCacheEXT*                 pSrcCaches) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkMergeValidationCachesEXT-device-parameter", kVUIDUndefined);
    skip |= DeviceValidateObject(device, dstCache, kVulkanObjectTypeValidationCacheEXT, false, "VUID-vkMergeValidationCachesEXT-dstCache-parameter", "VUID-vkMergeValidationCachesEXT-dstCache-parent");
    for (uint32_t index0 = 0; index0 < srcCacheCount; ++index0) {
        skip |= DeviceValidateObject(device, pSrcCaches[index0], kVulkanObjectTypeValidationCacheEXT, false, "VUID-vkMergeValidationCachesEXT-pSrcCaches-parameter", "VUID-vkMergeValidationCachesEXT-pSrcCaches-parent");
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetValidationCacheDataEXT(
    VkDevice                                    device,
    VkValidationCacheEXT                        validationCache,
    size_t*                                     pDataSize,
    void*                                       pData) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkGetValidationCacheDataEXT-device-parameter", kVUIDUndefined);
    skip |= DeviceValidateObject(device, validationCache, kVulkanObjectTypeValidationCacheEXT, false, "VUID-vkGetValidationCacheDataEXT-validationCache-parameter", "VUID-vkGetValidationCacheDataEXT-validationCache-parent");

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdBindShadingRateImageNV(
    VkCommandBuffer                             commandBuffer,
    VkImageView                                 imageView,
    VkImageLayout                               imageLayout) {
    bool skip = false;
    skip |= DeviceValidateObject(commandBuffer, commandBuffer, kVulkanObjectTypeCommandBuffer, false, "VUID-vkCmdBindShadingRateImageNV-commandBuffer-parameter", "VUID-vkCmdBindShadingRateImageNV-commonparent");
    skip |= DeviceValidateObject(commandBuffer, imageView, kVulkanObjectTypeImageView, false, "VUID-vkCmdBindShadingRateImageNV-imageView-parameter", "VUID-vkCmdBindShadingRateImageNV-commonparent");

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdSetViewportShadingRatePaletteNV(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstViewport,
    uint32_t                                    viewportCount,
    const VkShadingRatePaletteNV*               pShadingRatePalettes) {
    bool skip = false;
    skip |= DeviceValidateObject(commandBuffer, commandBuffer, kVulkanObjectTypeCommandBuffer, false, "VUID-vkCmdSetViewportShadingRatePaletteNV-commandBuffer-parameter", kVUIDUndefined);

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdSetCoarseSampleOrderNV(
    VkCommandBuffer                             commandBuffer,
    VkCoarseSampleOrderTypeNV                   sampleOrderType,
    uint32_t                                    customSampleOrderCount,
    const VkCoarseSampleOrderCustomNV*          pCustomSampleOrders) {
    bool skip = false;
    skip |= DeviceValidateObject(commandBuffer, commandBuffer, kVulkanObjectTypeCommandBuffer, false, "VUID-vkCmdSetCoarseSampleOrderNV-commandBuffer-parameter", kVUIDUndefined);

    return skip;
}

bool ObjectLifetimes::PreCallValidateCreateAccelerationStructureNVX(
    VkDevice                                    device,
    const VkAccelerationStructureCreateInfoNVX* pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkAccelerationStructureNVX*                 pAccelerationStructure) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkCreateAccelerationStructureNVX-device-parameter", kVUIDUndefined);
    if (pCreateInfo) {
        if (pCreateInfo->pGeometries) {
            for (uint32_t index1 = 0; index1 < pCreateInfo->geometryCount; ++index1) {
            }
        }
    }

    return skip;
}

void ObjectLifetimes::PostCallRecordCreateAccelerationStructureNVX(
    VkDevice                                    device,
    const VkAccelerationStructureCreateInfoNVX* pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkAccelerationStructureNVX*                 pAccelerationStructure) {
    DeviceCreateObject(device, *pAccelerationStructure, kVulkanObjectTypeAccelerationStructureNVX, pAllocator);

}

bool ObjectLifetimes::PreCallValidateDestroyAccelerationStructureNVX(
    VkDevice                                    device,
    VkAccelerationStructureNVX                  accelerationStructure,
    const VkAllocationCallbacks*                pAllocator) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkDestroyAccelerationStructureNVX-device-parameter", kVUIDUndefined);
    skip |= DeviceValidateObject(device, accelerationStructure, kVulkanObjectTypeAccelerationStructureNVX, false, "VUID-vkDestroyAccelerationStructureNVX-accelerationStructure-parameter", "VUID-vkDestroyAccelerationStructureNVX-accelerationStructure-parent");
    skip |= DeviceValidateDestroyObject(device, accelerationStructure, kVulkanObjectTypeAccelerationStructureNVX, pAllocator, kVUIDUndefined, kVUIDUndefined);

    return skip;
}

void ObjectLifetimes::PreCallRecordDestroyAccelerationStructureNVX(
    VkDevice                                    device,
    VkAccelerationStructureNVX                  accelerationStructure,
    const VkAllocationCallbacks*                pAllocator) {
    DeviceRecordDestroyObject(device, accelerationStructure, kVulkanObjectTypeAccelerationStructureNVX);

}

bool ObjectLifetimes::PreCallValidateGetAccelerationStructureMemoryRequirementsNVX(
    VkDevice                                    device,
    const VkAccelerationStructureMemoryRequirementsInfoNVX* pInfo,
    VkMemoryRequirements2KHR*                   pMemoryRequirements) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkGetAccelerationStructureMemoryRequirementsNVX-device-parameter", kVUIDUndefined);
    if (pInfo) {
        skip |= DeviceValidateObject(device, pInfo->accelerationStructure, kVulkanObjectTypeAccelerationStructureNVX, false, "VUID-VkAccelerationStructureMemoryRequirementsInfoNVX-accelerationStructure-parameter", kVUIDUndefined);
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetAccelerationStructureScratchMemoryRequirementsNVX(
    VkDevice                                    device,
    const VkAccelerationStructureMemoryRequirementsInfoNVX* pInfo,
    VkMemoryRequirements2KHR*                   pMemoryRequirements) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkGetAccelerationStructureScratchMemoryRequirementsNVX-device-parameter", kVUIDUndefined);
    if (pInfo) {
        skip |= DeviceValidateObject(device, pInfo->accelerationStructure, kVulkanObjectTypeAccelerationStructureNVX, false, "VUID-VkAccelerationStructureMemoryRequirementsInfoNVX-accelerationStructure-parameter", kVUIDUndefined);
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateBindAccelerationStructureMemoryNVX(
    VkDevice                                    device,
    uint32_t                                    bindInfoCount,
    const VkBindAccelerationStructureMemoryInfoNVX* pBindInfos) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkBindAccelerationStructureMemoryNVX-device-parameter", kVUIDUndefined);
    if (pBindInfos) {
        for (uint32_t index0 = 0; index0 < bindInfoCount; ++index0) {
            skip |= DeviceValidateObject(device, pBindInfos[index0].accelerationStructure, kVulkanObjectTypeAccelerationStructureNVX, false, "VUID-VkBindAccelerationStructureMemoryInfoNVX-accelerationStructure-parameter", "VUID-VkBindAccelerationStructureMemoryInfoNVX-commonparent");
            skip |= DeviceValidateObject(device, pBindInfos[index0].memory, kVulkanObjectTypeDeviceMemory, false, "VUID-VkBindAccelerationStructureMemoryInfoNVX-memory-parameter", "VUID-VkBindAccelerationStructureMemoryInfoNVX-commonparent");
        }
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdBuildAccelerationStructureNVX(
    VkCommandBuffer                             commandBuffer,
    VkAccelerationStructureTypeNVX              type,
    uint32_t                                    instanceCount,
    VkBuffer                                    instanceData,
    VkDeviceSize                                instanceOffset,
    uint32_t                                    geometryCount,
    const VkGeometryNVX*                        pGeometries,
    VkBuildAccelerationStructureFlagsNVX        flags,
    VkBool32                                    update,
    VkAccelerationStructureNVX                  dst,
    VkAccelerationStructureNVX                  src,
    VkBuffer                                    scratch,
    VkDeviceSize                                scratchOffset) {
    bool skip = false;
    skip |= DeviceValidateObject(commandBuffer, commandBuffer, kVulkanObjectTypeCommandBuffer, false, "VUID-vkCmdBuildAccelerationStructureNVX-commandBuffer-parameter", "VUID-vkCmdBuildAccelerationStructureNVX-commonparent");
    skip |= DeviceValidateObject(commandBuffer, instanceData, kVulkanObjectTypeBuffer, true, "VUID-vkCmdBuildAccelerationStructureNVX-instanceData-parameter", "VUID-vkCmdBuildAccelerationStructureNVX-commonparent");
    if (pGeometries) {
        for (uint32_t index0 = 0; index0 < geometryCount; ++index0) {
        }
    }
    skip |= DeviceValidateObject(commandBuffer, dst, kVulkanObjectTypeAccelerationStructureNVX, false, "VUID-vkCmdBuildAccelerationStructureNVX-dst-parameter", "VUID-vkCmdBuildAccelerationStructureNVX-commonparent");
    skip |= DeviceValidateObject(commandBuffer, src, kVulkanObjectTypeAccelerationStructureNVX, true, "VUID-vkCmdBuildAccelerationStructureNVX-src-parameter", "VUID-vkCmdBuildAccelerationStructureNVX-commonparent");
    skip |= DeviceValidateObject(commandBuffer, scratch, kVulkanObjectTypeBuffer, false, "VUID-vkCmdBuildAccelerationStructureNVX-scratch-parameter", "VUID-vkCmdBuildAccelerationStructureNVX-commonparent");

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdCopyAccelerationStructureNVX(
    VkCommandBuffer                             commandBuffer,
    VkAccelerationStructureNVX                  dst,
    VkAccelerationStructureNVX                  src,
    VkCopyAccelerationStructureModeNVX          mode) {
    bool skip = false;
    skip |= DeviceValidateObject(commandBuffer, commandBuffer, kVulkanObjectTypeCommandBuffer, false, "VUID-vkCmdCopyAccelerationStructureNVX-commandBuffer-parameter", "VUID-vkCmdCopyAccelerationStructureNVX-commonparent");
    skip |= DeviceValidateObject(commandBuffer, dst, kVulkanObjectTypeAccelerationStructureNVX, false, "VUID-vkCmdCopyAccelerationStructureNVX-dst-parameter", "VUID-vkCmdCopyAccelerationStructureNVX-commonparent");
    skip |= DeviceValidateObject(commandBuffer, src, kVulkanObjectTypeAccelerationStructureNVX, false, "VUID-vkCmdCopyAccelerationStructureNVX-src-parameter", "VUID-vkCmdCopyAccelerationStructureNVX-commonparent");

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdTraceRaysNVX(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    raygenShaderBindingTableBuffer,
    VkDeviceSize                                raygenShaderBindingOffset,
    VkBuffer                                    missShaderBindingTableBuffer,
    VkDeviceSize                                missShaderBindingOffset,
    VkDeviceSize                                missShaderBindingStride,
    VkBuffer                                    hitShaderBindingTableBuffer,
    VkDeviceSize                                hitShaderBindingOffset,
    VkDeviceSize                                hitShaderBindingStride,
    uint32_t                                    width,
    uint32_t                                    height) {
    bool skip = false;
    skip |= DeviceValidateObject(commandBuffer, commandBuffer, kVulkanObjectTypeCommandBuffer, false, "VUID-vkCmdTraceRaysNVX-commandBuffer-parameter", "VUID-vkCmdTraceRaysNVX-commonparent");
    skip |= DeviceValidateObject(commandBuffer, raygenShaderBindingTableBuffer, kVulkanObjectTypeBuffer, false, "VUID-vkCmdTraceRaysNVX-raygenShaderBindingTableBuffer-parameter", "VUID-vkCmdTraceRaysNVX-commonparent");
    skip |= DeviceValidateObject(commandBuffer, missShaderBindingTableBuffer, kVulkanObjectTypeBuffer, false, "VUID-vkCmdTraceRaysNVX-missShaderBindingTableBuffer-parameter", "VUID-vkCmdTraceRaysNVX-commonparent");
    skip |= DeviceValidateObject(commandBuffer, hitShaderBindingTableBuffer, kVulkanObjectTypeBuffer, false, "VUID-vkCmdTraceRaysNVX-hitShaderBindingTableBuffer-parameter", "VUID-vkCmdTraceRaysNVX-commonparent");

    return skip;
}

bool ObjectLifetimes::PreCallValidateCreateRaytracingPipelinesNVX(
    VkDevice                                    device,
    VkPipelineCache                             pipelineCache,
    uint32_t                                    createInfoCount,
    const VkRaytracingPipelineCreateInfoNVX*    pCreateInfos,
    const VkAllocationCallbacks*                pAllocator,
    VkPipeline*                                 pPipelines) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkCreateRaytracingPipelinesNVX-device-parameter", kVUIDUndefined);
    skip |= DeviceValidateObject(device, pipelineCache, kVulkanObjectTypePipelineCache, true, "VUID-vkCreateRaytracingPipelinesNVX-pipelineCache-parameter", "VUID-vkCreateRaytracingPipelinesNVX-pipelineCache-parent");
    if (pCreateInfos) {
        for (uint32_t index0 = 0; index0 < createInfoCount; ++index0) {
            if (pCreateInfos[index0].pStages) {
                for (uint32_t index1 = 0; index1 < pCreateInfos[index0].stageCount; ++index1) {
                    skip |= DeviceValidateObject(device, pCreateInfos[index0].pStages[index1].module, kVulkanObjectTypeShaderModule, false, "VUID-VkPipelineShaderStageCreateInfo-module-parameter", kVUIDUndefined);
                }
            }
            skip |= DeviceValidateObject(device, pCreateInfos[index0].layout, kVulkanObjectTypePipelineLayout, false, "VUID-VkRaytracingPipelineCreateInfoNVX-layout-parameter", "VUID-VkRaytracingPipelineCreateInfoNVX-commonparent");
            skip |= DeviceValidateObject(device, pCreateInfos[index0].basePipelineHandle, kVulkanObjectTypePipeline, true, kVUIDUndefined, "VUID-VkRaytracingPipelineCreateInfoNVX-commonparent");
        }
    }

    return skip;
}

void ObjectLifetimes::PostCallRecordCreateRaytracingPipelinesNVX(
    VkDevice                                    device,
    VkPipelineCache                             pipelineCache,
    uint32_t                                    createInfoCount,
    const VkRaytracingPipelineCreateInfoNVX*    pCreateInfos,
    const VkAllocationCallbacks*                pAllocator,
    VkPipeline*                                 pPipelines) {
    for (uint32_t index = 0; index < createInfoCount; index++) {
        DeviceCreateObject(device, pPipelines[index], kVulkanObjectTypePipeline, pAllocator);
    }

}

bool ObjectLifetimes::PreCallValidateGetRaytracingShaderHandlesNVX(
    VkDevice                                    device,
    VkPipeline                                  pipeline,
    uint32_t                                    firstGroup,
    uint32_t                                    groupCount,
    size_t                                      dataSize,
    void*                                       pData) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkGetRaytracingShaderHandlesNVX-device-parameter", kVUIDUndefined);
    skip |= DeviceValidateObject(device, pipeline, kVulkanObjectTypePipeline, false, "VUID-vkGetRaytracingShaderHandlesNVX-pipeline-parameter", "VUID-vkGetRaytracingShaderHandlesNVX-pipeline-parent");

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetAccelerationStructureHandleNVX(
    VkDevice                                    device,
    VkAccelerationStructureNVX                  accelerationStructure,
    size_t                                      dataSize,
    void*                                       pData) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkGetAccelerationStructureHandleNVX-device-parameter", kVUIDUndefined);
    skip |= DeviceValidateObject(device, accelerationStructure, kVulkanObjectTypeAccelerationStructureNVX, false, "VUID-vkGetAccelerationStructureHandleNVX-accelerationStructure-parameter", "VUID-vkGetAccelerationStructureHandleNVX-accelerationStructure-parent");

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdWriteAccelerationStructurePropertiesNVX(
    VkCommandBuffer                             commandBuffer,
    VkAccelerationStructureNVX                  accelerationStructure,
    VkQueryType                                 queryType,
    VkQueryPool                                 queryPool,
    uint32_t                                    query) {
    bool skip = false;
    skip |= DeviceValidateObject(commandBuffer, commandBuffer, kVulkanObjectTypeCommandBuffer, false, "VUID-vkCmdWriteAccelerationStructurePropertiesNVX-commandBuffer-parameter", "VUID-vkCmdWriteAccelerationStructurePropertiesNVX-commonparent");
    skip |= DeviceValidateObject(commandBuffer, accelerationStructure, kVulkanObjectTypeAccelerationStructureNVX, false, "VUID-vkCmdWriteAccelerationStructurePropertiesNVX-accelerationStructure-parameter", "VUID-vkCmdWriteAccelerationStructurePropertiesNVX-commonparent");
    skip |= DeviceValidateObject(commandBuffer, queryPool, kVulkanObjectTypeQueryPool, false, "VUID-vkCmdWriteAccelerationStructurePropertiesNVX-queryPool-parameter", "VUID-vkCmdWriteAccelerationStructurePropertiesNVX-commonparent");

    return skip;
}

bool ObjectLifetimes::PreCallValidateCompileDeferredNVX(
    VkDevice                                    device,
    VkPipeline                                  pipeline,
    uint32_t                                    shader) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkCompileDeferredNVX-device-parameter", kVUIDUndefined);
    skip |= DeviceValidateObject(device, pipeline, kVulkanObjectTypePipeline, false, "VUID-vkCompileDeferredNVX-pipeline-parameter", "VUID-vkCompileDeferredNVX-pipeline-parent");

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetMemoryHostPointerPropertiesEXT(
    VkDevice                                    device,
    VkExternalMemoryHandleTypeFlagBits          handleType,
    const void*                                 pHostPointer,
    VkMemoryHostPointerPropertiesEXT*           pMemoryHostPointerProperties) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkGetMemoryHostPointerPropertiesEXT-device-parameter", kVUIDUndefined);

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdWriteBufferMarkerAMD(
    VkCommandBuffer                             commandBuffer,
    VkPipelineStageFlagBits                     pipelineStage,
    VkBuffer                                    dstBuffer,
    VkDeviceSize                                dstOffset,
    uint32_t                                    marker) {
    bool skip = false;
    skip |= DeviceValidateObject(commandBuffer, commandBuffer, kVulkanObjectTypeCommandBuffer, false, "VUID-vkCmdWriteBufferMarkerAMD-commandBuffer-parameter", "VUID-vkCmdWriteBufferMarkerAMD-commonparent");
    skip |= DeviceValidateObject(commandBuffer, dstBuffer, kVulkanObjectTypeBuffer, false, "VUID-vkCmdWriteBufferMarkerAMD-dstBuffer-parameter", "VUID-vkCmdWriteBufferMarkerAMD-commonparent");

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdDrawMeshTasksNV(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    taskCount,
    uint32_t                                    firstTask) {
    bool skip = false;
    skip |= DeviceValidateObject(commandBuffer, commandBuffer, kVulkanObjectTypeCommandBuffer, false, "VUID-vkCmdDrawMeshTasksNV-commandBuffer-parameter", kVUIDUndefined);

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdDrawMeshTasksIndirectNV(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    uint32_t                                    drawCount,
    uint32_t                                    stride) {
    bool skip = false;
    skip |= DeviceValidateObject(commandBuffer, commandBuffer, kVulkanObjectTypeCommandBuffer, false, "VUID-vkCmdDrawMeshTasksIndirectNV-commandBuffer-parameter", "VUID-vkCmdDrawMeshTasksIndirectNV-commonparent");
    skip |= DeviceValidateObject(commandBuffer, buffer, kVulkanObjectTypeBuffer, false, "VUID-vkCmdDrawMeshTasksIndirectNV-buffer-parameter", "VUID-vkCmdDrawMeshTasksIndirectNV-commonparent");

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdDrawMeshTasksIndirectCountNV(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkBuffer                                    countBuffer,
    VkDeviceSize                                countBufferOffset,
    uint32_t                                    maxDrawCount,
    uint32_t                                    stride) {
    bool skip = false;
    skip |= DeviceValidateObject(commandBuffer, commandBuffer, kVulkanObjectTypeCommandBuffer, false, "VUID-vkCmdDrawMeshTasksIndirectCountNV-commandBuffer-parameter", "VUID-vkCmdDrawMeshTasksIndirectCountNV-commonparent");
    skip |= DeviceValidateObject(commandBuffer, buffer, kVulkanObjectTypeBuffer, false, "VUID-vkCmdDrawMeshTasksIndirectCountNV-buffer-parameter", "VUID-vkCmdDrawMeshTasksIndirectCountNV-commonparent");
    skip |= DeviceValidateObject(commandBuffer, countBuffer, kVulkanObjectTypeBuffer, false, "VUID-vkCmdDrawMeshTasksIndirectCountNV-countBuffer-parameter", "VUID-vkCmdDrawMeshTasksIndirectCountNV-commonparent");

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdSetExclusiveScissorNV(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstExclusiveScissor,
    uint32_t                                    exclusiveScissorCount,
    const VkRect2D*                             pExclusiveScissors) {
    bool skip = false;
    skip |= DeviceValidateObject(commandBuffer, commandBuffer, kVulkanObjectTypeCommandBuffer, false, "VUID-vkCmdSetExclusiveScissorNV-commandBuffer-parameter", kVUIDUndefined);

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdSetCheckpointNV(
    VkCommandBuffer                             commandBuffer,
    const void*                                 pCheckpointMarker) {
    bool skip = false;
    skip |= DeviceValidateObject(commandBuffer, commandBuffer, kVulkanObjectTypeCommandBuffer, false, "VUID-vkCmdSetCheckpointNV-commandBuffer-parameter", kVUIDUndefined);

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetQueueCheckpointDataNV(
    VkQueue                                     queue,
    uint32_t*                                   pCheckpointDataCount,
    VkCheckpointDataNV*                         pCheckpointData) {
    bool skip = false;
    skip |= DeviceValidateObject(queue, queue, kVulkanObjectTypeQueue, false, "VUID-vkGetQueueCheckpointDataNV-queue-parameter", kVUIDUndefined);

    return skip;
}

#ifdef VK_USE_PLATFORM_FUCHSIA

bool ObjectLifetimes::PreCallValidateCreateImagePipeSurfaceFUCHSIA(
    VkInstance                                  instance,
    const VkImagePipeSurfaceCreateInfoFUCHSIA*  pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) {
    bool skip = false;
    skip |= InstanceValidateObject(instance, instance, kVulkanObjectTypeInstance, false, "VUID-vkCreateImagePipeSurfaceFUCHSIA-instance-parameter", kVUIDUndefined);

    return skip;
}

void ObjectLifetimes::PostCallRecordCreateImagePipeSurfaceFUCHSIA(
    VkInstance                                  instance,
    const VkImagePipeSurfaceCreateInfoFUCHSIA*  pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) {
    InstanceCreateObject(instance, *pSurface, kVulkanObjectTypeSurfaceKHR, pAllocator);

}
#endif // VK_USE_PLATFORM_FUCHSIA


