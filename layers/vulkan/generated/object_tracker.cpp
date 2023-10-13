// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See object_tracker_generator.py for modifications

/***************************************************************************
 *
 * Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
 * Copyright (c) 2015-2023 Google Inc.
 * Copyright (c) 2015-2023 RasterGrid Kft.
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
// clang-format off
#include "chassis.h"
#include "object_tracker/object_lifetime_validation.h"
ReadLockGuard ObjectLifetimes::ReadLock() const { return ReadLockGuard(validation_object_mutex, std::defer_lock); }
WriteLockGuard ObjectLifetimes::WriteLock() { return WriteLockGuard(validation_object_mutex, std::defer_lock); }

// ObjectTracker undestroyed objects validation function
bool ObjectLifetimes::ReportUndestroyedInstanceObjects(VkInstance instance, const Location& loc) const {
    bool skip = false;
    const std::string error_code = "VUID-vkDestroyInstance-instance-00629";
    skip |= ReportLeakedInstanceObjects(instance, kVulkanObjectTypeSurfaceKHR, error_code, loc);
    // No destroy API or implicitly freed/destroyed -- do not report: skip |= ReportLeakedInstanceObjects(instance, kVulkanObjectTypeDisplayKHR, error_code, loc);
    // No destroy API or implicitly freed/destroyed -- do not report: skip |= ReportLeakedInstanceObjects(instance, kVulkanObjectTypeDisplayModeKHR, error_code, loc);
    skip |= ReportLeakedInstanceObjects(instance, kVulkanObjectTypeDebugReportCallbackEXT, error_code, loc);
    skip |= ReportLeakedInstanceObjects(instance, kVulkanObjectTypeDebugUtilsMessengerEXT, error_code, loc);
    return skip;
}

bool ObjectLifetimes::ReportUndestroyedDeviceObjects(VkDevice device, const Location& loc) const {
    bool skip = false;
    const std::string error_code = "VUID-vkDestroyDevice-device-05137";
    skip |= ReportLeakedDeviceObjects(device, kVulkanObjectTypeCommandBuffer, error_code, loc);
    skip |= ReportLeakedDeviceObjects(device, kVulkanObjectTypeBuffer, error_code, loc);
    skip |= ReportLeakedDeviceObjects(device, kVulkanObjectTypeImage, error_code, loc);
    skip |= ReportLeakedDeviceObjects(device, kVulkanObjectTypeSemaphore, error_code, loc);
    skip |= ReportLeakedDeviceObjects(device, kVulkanObjectTypeFence, error_code, loc);
    skip |= ReportLeakedDeviceObjects(device, kVulkanObjectTypeDeviceMemory, error_code, loc);
    skip |= ReportLeakedDeviceObjects(device, kVulkanObjectTypeEvent, error_code, loc);
    skip |= ReportLeakedDeviceObjects(device, kVulkanObjectTypeQueryPool, error_code, loc);
    skip |= ReportLeakedDeviceObjects(device, kVulkanObjectTypeBufferView, error_code, loc);
    skip |= ReportLeakedDeviceObjects(device, kVulkanObjectTypeImageView, error_code, loc);
    skip |= ReportLeakedDeviceObjects(device, kVulkanObjectTypeShaderModule, error_code, loc);
    skip |= ReportLeakedDeviceObjects(device, kVulkanObjectTypePipelineCache, error_code, loc);
    skip |= ReportLeakedDeviceObjects(device, kVulkanObjectTypePipelineLayout, error_code, loc);
    skip |= ReportLeakedDeviceObjects(device, kVulkanObjectTypePipeline, error_code, loc);
    skip |= ReportLeakedDeviceObjects(device, kVulkanObjectTypeRenderPass, error_code, loc);
    skip |= ReportLeakedDeviceObjects(device, kVulkanObjectTypeDescriptorSetLayout, error_code, loc);
    skip |= ReportLeakedDeviceObjects(device, kVulkanObjectTypeSampler, error_code, loc);
    skip |= ReportLeakedDeviceObjects(device, kVulkanObjectTypeDescriptorSet, error_code, loc);
    skip |= ReportLeakedDeviceObjects(device, kVulkanObjectTypeDescriptorPool, error_code, loc);
    skip |= ReportLeakedDeviceObjects(device, kVulkanObjectTypeFramebuffer, error_code, loc);
    skip |= ReportLeakedDeviceObjects(device, kVulkanObjectTypeCommandPool, error_code, loc);
    skip |= ReportLeakedDeviceObjects(device, kVulkanObjectTypeSamplerYcbcrConversion, error_code, loc);
    skip |= ReportLeakedDeviceObjects(device, kVulkanObjectTypeDescriptorUpdateTemplate, error_code, loc);
    skip |= ReportLeakedDeviceObjects(device, kVulkanObjectTypePrivateDataSlot, error_code, loc);
    skip |= ReportLeakedDeviceObjects(device, kVulkanObjectTypeSwapchainKHR, error_code, loc);
    skip |= ReportLeakedDeviceObjects(device, kVulkanObjectTypeVideoSessionKHR, error_code, loc);
    skip |= ReportLeakedDeviceObjects(device, kVulkanObjectTypeVideoSessionParametersKHR, error_code, loc);
    skip |= ReportLeakedDeviceObjects(device, kVulkanObjectTypeDeferredOperationKHR, error_code, loc);
    skip |= ReportLeakedDeviceObjects(device, kVulkanObjectTypeCuModuleNVX, error_code, loc);
    skip |= ReportLeakedDeviceObjects(device, kVulkanObjectTypeCuFunctionNVX, error_code, loc);
    skip |= ReportLeakedDeviceObjects(device, kVulkanObjectTypeValidationCacheEXT, error_code, loc);
    skip |= ReportLeakedDeviceObjects(device, kVulkanObjectTypeAccelerationStructureNV, error_code, loc);
    skip |= ReportLeakedDeviceObjects(device, kVulkanObjectTypePerformanceConfigurationINTEL, error_code, loc);
    skip |= ReportLeakedDeviceObjects(device, kVulkanObjectTypeIndirectCommandsLayoutNV, error_code, loc);
    skip |= ReportLeakedDeviceObjects(device, kVulkanObjectTypeAccelerationStructureKHR, error_code, loc);
    skip |= ReportLeakedDeviceObjects(device, kVulkanObjectTypeBufferCollectionFUCHSIA, error_code, loc);
    skip |= ReportLeakedDeviceObjects(device, kVulkanObjectTypeMicromapEXT, error_code, loc);
    skip |= ReportLeakedDeviceObjects(device, kVulkanObjectTypeOpticalFlowSessionNV, error_code, loc);
    skip |= ReportLeakedDeviceObjects(device, kVulkanObjectTypeShaderEXT, error_code, loc);
    return skip;
}

void ObjectLifetimes::DestroyLeakedInstanceObjects() {
    DestroyUndestroyedObjects(kVulkanObjectTypeSurfaceKHR);
    DestroyUndestroyedObjects(kVulkanObjectTypeDisplayKHR);
    DestroyUndestroyedObjects(kVulkanObjectTypeDisplayModeKHR);
    DestroyUndestroyedObjects(kVulkanObjectTypeDebugReportCallbackEXT);
    DestroyUndestroyedObjects(kVulkanObjectTypeDebugUtilsMessengerEXT);
}

void ObjectLifetimes::DestroyLeakedDeviceObjects() {
    DestroyUndestroyedObjects(kVulkanObjectTypeCommandBuffer);
    DestroyUndestroyedObjects(kVulkanObjectTypeBuffer);
    DestroyUndestroyedObjects(kVulkanObjectTypeImage);
    DestroyUndestroyedObjects(kVulkanObjectTypeSemaphore);
    DestroyUndestroyedObjects(kVulkanObjectTypeFence);
    DestroyUndestroyedObjects(kVulkanObjectTypeDeviceMemory);
    DestroyUndestroyedObjects(kVulkanObjectTypeEvent);
    DestroyUndestroyedObjects(kVulkanObjectTypeQueryPool);
    DestroyUndestroyedObjects(kVulkanObjectTypeBufferView);
    DestroyUndestroyedObjects(kVulkanObjectTypeImageView);
    DestroyUndestroyedObjects(kVulkanObjectTypeShaderModule);
    DestroyUndestroyedObjects(kVulkanObjectTypePipelineCache);
    DestroyUndestroyedObjects(kVulkanObjectTypePipelineLayout);
    DestroyUndestroyedObjects(kVulkanObjectTypePipeline);
    DestroyUndestroyedObjects(kVulkanObjectTypeRenderPass);
    DestroyUndestroyedObjects(kVulkanObjectTypeDescriptorSetLayout);
    DestroyUndestroyedObjects(kVulkanObjectTypeSampler);
    DestroyUndestroyedObjects(kVulkanObjectTypeDescriptorSet);
    DestroyUndestroyedObjects(kVulkanObjectTypeDescriptorPool);
    DestroyUndestroyedObjects(kVulkanObjectTypeFramebuffer);
    DestroyUndestroyedObjects(kVulkanObjectTypeCommandPool);
    DestroyUndestroyedObjects(kVulkanObjectTypeSamplerYcbcrConversion);
    DestroyUndestroyedObjects(kVulkanObjectTypeDescriptorUpdateTemplate);
    DestroyUndestroyedObjects(kVulkanObjectTypePrivateDataSlot);
    DestroyUndestroyedObjects(kVulkanObjectTypeSwapchainKHR);
    DestroyUndestroyedObjects(kVulkanObjectTypeVideoSessionKHR);
    DestroyUndestroyedObjects(kVulkanObjectTypeVideoSessionParametersKHR);
    DestroyUndestroyedObjects(kVulkanObjectTypeDeferredOperationKHR);
    DestroyUndestroyedObjects(kVulkanObjectTypeCuModuleNVX);
    DestroyUndestroyedObjects(kVulkanObjectTypeCuFunctionNVX);
    DestroyUndestroyedObjects(kVulkanObjectTypeValidationCacheEXT);
    DestroyUndestroyedObjects(kVulkanObjectTypeAccelerationStructureNV);
    DestroyUndestroyedObjects(kVulkanObjectTypePerformanceConfigurationINTEL);
    DestroyUndestroyedObjects(kVulkanObjectTypeIndirectCommandsLayoutNV);
    DestroyUndestroyedObjects(kVulkanObjectTypeAccelerationStructureKHR);
    DestroyUndestroyedObjects(kVulkanObjectTypeBufferCollectionFUCHSIA);
    DestroyUndestroyedObjects(kVulkanObjectTypeMicromapEXT);
    DestroyUndestroyedObjects(kVulkanObjectTypeOpticalFlowSessionNV);
    DestroyUndestroyedObjects(kVulkanObjectTypeShaderEXT);
}
// clang-format on
// vkGetPhysicalDeviceFeatures:
// Checked by chassis: physicalDevice: "VUID-vkGetPhysicalDeviceFeatures-physicalDevice-parameter"

// vkGetPhysicalDeviceFormatProperties:
// Checked by chassis: physicalDevice: "VUID-vkGetPhysicalDeviceFormatProperties-physicalDevice-parameter"

// vkGetPhysicalDeviceImageFormatProperties:
// Checked by chassis: physicalDevice: "VUID-vkGetPhysicalDeviceImageFormatProperties-physicalDevice-parameter"

// vkGetPhysicalDeviceProperties:
// Checked by chassis: physicalDevice: "VUID-vkGetPhysicalDeviceProperties-physicalDevice-parameter"

// vkGetPhysicalDeviceMemoryProperties:
// Checked by chassis: physicalDevice: "VUID-vkGetPhysicalDeviceMemoryProperties-physicalDevice-parameter"

// vkGetInstanceProcAddr:
// Checked by chassis: instance: "VUID-vkGetInstanceProcAddr-instance-parameter"

// vkGetDeviceProcAddr:
// Checked by chassis: device: "VUID-vkGetDeviceProcAddr-device-parameter"

// vkEnumerateDeviceExtensionProperties:
// Checked by chassis: physicalDevice: "VUID-vkEnumerateDeviceExtensionProperties-physicalDevice-parameter"

// vkEnumerateDeviceLayerProperties:
// Checked by chassis: physicalDevice: "VUID-vkEnumerateDeviceLayerProperties-physicalDevice-parameter"

bool ObjectLifetimes::PreCallValidateQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence,
                                                 const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: queue: "VUID-vkQueueSubmit-queue-parameter"
    // Checked by chassis: queue: "VUID-vkQueueSubmit-commonparent"
    if (pSubmits) {
        for (uint32_t index0 = 0; index0 < submitCount; ++index0) {
            [[maybe_unused]] const Location index0_loc = error_obj.location.dot(Field::pSubmits, index0);

            if ((pSubmits[index0].waitSemaphoreCount > 0) && (pSubmits[index0].pWaitSemaphores)) {
                for (uint32_t index1 = 0; index1 < pSubmits[index0].waitSemaphoreCount; ++index1) {
                    skip |= ValidateObject(pSubmits[index0].pWaitSemaphores[index1], kVulkanObjectTypeSemaphore, false,
                                           "VUID-VkSubmitInfo-pWaitSemaphores-parameter", "VUID-VkSubmitInfo-commonparent",
                                           index0_loc.dot(Field::pWaitSemaphores, index1));
                }
            }

            if ((pSubmits[index0].commandBufferCount > 0) && (pSubmits[index0].pCommandBuffers)) {
                for (uint32_t index1 = 0; index1 < pSubmits[index0].commandBufferCount; ++index1) {
                    skip |= ValidateObject(pSubmits[index0].pCommandBuffers[index1], kVulkanObjectTypeCommandBuffer, false,
                                           "VUID-VkSubmitInfo-pCommandBuffers-parameter", "VUID-VkSubmitInfo-commonparent",
                                           index0_loc.dot(Field::pCommandBuffers, index1));
                }
            }

            if ((pSubmits[index0].signalSemaphoreCount > 0) && (pSubmits[index0].pSignalSemaphores)) {
                for (uint32_t index1 = 0; index1 < pSubmits[index0].signalSemaphoreCount; ++index1) {
                    skip |= ValidateObject(pSubmits[index0].pSignalSemaphores[index1], kVulkanObjectTypeSemaphore, false,
                                           "VUID-VkSubmitInfo-pSignalSemaphores-parameter", "VUID-VkSubmitInfo-commonparent",
                                           index0_loc.dot(Field::pSignalSemaphores, index1));
                }
            }
            if (auto pNext = vku::FindStructInPNextChain<VkFrameBoundaryEXT>(pSubmits[index0].pNext)) {
                const Location pNext_loc = index0_loc.pNext(Struct::VkFrameBoundaryEXT);

                if ((pNext->imageCount > 0) && (pNext->pImages)) {
                    for (uint32_t index2 = 0; index2 < pNext->imageCount; ++index2) {
                        skip |= ValidateObject(pNext->pImages[index2], kVulkanObjectTypeImage, false,
                                               "VUID-VkFrameBoundaryEXT-pImages-parameter", "VUID-VkFrameBoundaryEXT-commonparent",
                                               pNext_loc.dot(Field::pImages, index2));
                    }
                }

                if ((pNext->bufferCount > 0) && (pNext->pBuffers)) {
                    for (uint32_t index2 = 0; index2 < pNext->bufferCount; ++index2) {
                        skip |= ValidateObject(pNext->pBuffers[index2], kVulkanObjectTypeBuffer, false,
                                               "VUID-VkFrameBoundaryEXT-pBuffers-parameter", "VUID-VkFrameBoundaryEXT-commonparent",
                                               pNext_loc.dot(Field::pBuffers, index2));
                    }
                }
            }
#ifdef VK_USE_PLATFORM_WIN32_KHR
            if (auto pNext = vku::FindStructInPNextChain<VkWin32KeyedMutexAcquireReleaseInfoKHR>(pSubmits[index0].pNext)) {
                const Location pNext_loc = index0_loc.pNext(Struct::VkWin32KeyedMutexAcquireReleaseInfoKHR);

                if ((pNext->acquireCount > 0) && (pNext->pAcquireSyncs)) {
                    for (uint32_t index2 = 0; index2 < pNext->acquireCount; ++index2) {
                        skip |= ValidateObject(pNext->pAcquireSyncs[index2], kVulkanObjectTypeDeviceMemory, false,
                                               "VUID-VkWin32KeyedMutexAcquireReleaseInfoKHR-pAcquireSyncs-parameter",
                                               "VUID-VkWin32KeyedMutexAcquireReleaseInfoKHR-commonparent",
                                               pNext_loc.dot(Field::pAcquireSyncs, index2));
                    }
                }

                if ((pNext->releaseCount > 0) && (pNext->pReleaseSyncs)) {
                    for (uint32_t index2 = 0; index2 < pNext->releaseCount; ++index2) {
                        skip |= ValidateObject(pNext->pReleaseSyncs[index2], kVulkanObjectTypeDeviceMemory, false,
                                               "VUID-VkWin32KeyedMutexAcquireReleaseInfoKHR-pReleaseSyncs-parameter",
                                               "VUID-VkWin32KeyedMutexAcquireReleaseInfoKHR-commonparent",
                                               pNext_loc.dot(Field::pReleaseSyncs, index2));
                    }
                }
            }
#endif  // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
            if (auto pNext = vku::FindStructInPNextChain<VkWin32KeyedMutexAcquireReleaseInfoNV>(pSubmits[index0].pNext)) {
                const Location pNext_loc = index0_loc.pNext(Struct::VkWin32KeyedMutexAcquireReleaseInfoNV);

                if ((pNext->acquireCount > 0) && (pNext->pAcquireSyncs)) {
                    for (uint32_t index2 = 0; index2 < pNext->acquireCount; ++index2) {
                        skip |= ValidateObject(pNext->pAcquireSyncs[index2], kVulkanObjectTypeDeviceMemory, false,
                                               "VUID-VkWin32KeyedMutexAcquireReleaseInfoNV-pAcquireSyncs-parameter",
                                               "VUID-VkWin32KeyedMutexAcquireReleaseInfoNV-commonparent",
                                               pNext_loc.dot(Field::pAcquireSyncs, index2));
                    }
                }

                if ((pNext->releaseCount > 0) && (pNext->pReleaseSyncs)) {
                    for (uint32_t index2 = 0; index2 < pNext->releaseCount; ++index2) {
                        skip |= ValidateObject(pNext->pReleaseSyncs[index2], kVulkanObjectTypeDeviceMemory, false,
                                               "VUID-VkWin32KeyedMutexAcquireReleaseInfoNV-pReleaseSyncs-parameter",
                                               "VUID-VkWin32KeyedMutexAcquireReleaseInfoNV-commonparent",
                                               pNext_loc.dot(Field::pReleaseSyncs, index2));
                    }
                }
            }
#endif  // VK_USE_PLATFORM_WIN32_KHR
        }
    }
    skip |= ValidateObject(fence, kVulkanObjectTypeFence, true, "VUID-vkQueueSubmit-fence-parameter",
                           "VUID-vkQueueSubmit-commonparent", error_obj.location.dot(Field::fence));

    return skip;
}

// vkQueueWaitIdle:
// Checked by chassis: queue: "VUID-vkQueueWaitIdle-queue-parameter"

// vkDeviceWaitIdle:
// Checked by chassis: device: "VUID-vkDeviceWaitIdle-device-parameter"

bool ObjectLifetimes::PreCallValidateAllocateMemory(VkDevice device, const VkMemoryAllocateInfo* pAllocateInfo,
                                                    const VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory,
                                                    const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkAllocateMemory-device-parameter"
    if (pAllocateInfo) {
        [[maybe_unused]] const Location pAllocateInfo_loc = error_obj.location.dot(Field::pAllocateInfo);
        if (auto pNext = vku::FindStructInPNextChain<VkDedicatedAllocationMemoryAllocateInfoNV>(pAllocateInfo->pNext)) {
            const Location pNext_loc = pAllocateInfo_loc.pNext(Struct::VkDedicatedAllocationMemoryAllocateInfoNV);
            skip |= ValidateObject(pNext->image, kVulkanObjectTypeImage, true,
                                   "VUID-VkDedicatedAllocationMemoryAllocateInfoNV-image-parameter",
                                   "VUID-VkDedicatedAllocationMemoryAllocateInfoNV-commonparent", pNext_loc.dot(Field::image));
            skip |= ValidateObject(pNext->buffer, kVulkanObjectTypeBuffer, true,
                                   "VUID-VkDedicatedAllocationMemoryAllocateInfoNV-buffer-parameter",
                                   "VUID-VkDedicatedAllocationMemoryAllocateInfoNV-commonparent", pNext_loc.dot(Field::buffer));
        }
#ifdef VK_USE_PLATFORM_FUCHSIA
        if (auto pNext = vku::FindStructInPNextChain<VkImportMemoryBufferCollectionFUCHSIA>(pAllocateInfo->pNext)) {
            const Location pNext_loc = pAllocateInfo_loc.pNext(Struct::VkImportMemoryBufferCollectionFUCHSIA);
            skip |= ValidateObject(pNext->collection, kVulkanObjectTypeBufferCollectionFUCHSIA, false,
                                   "VUID-VkImportMemoryBufferCollectionFUCHSIA-collection-parameter", kVUIDUndefined,
                                   pNext_loc.dot(Field::collection));
        }
#endif  // VK_USE_PLATFORM_FUCHSIA
        if (auto pNext = vku::FindStructInPNextChain<VkMemoryDedicatedAllocateInfo>(pAllocateInfo->pNext)) {
            const Location pNext_loc = pAllocateInfo_loc.pNext(Struct::VkMemoryDedicatedAllocateInfo);
            skip |= ValidateObject(pNext->image, kVulkanObjectTypeImage, true, "VUID-VkMemoryDedicatedAllocateInfo-image-parameter",
                                   "VUID-VkMemoryDedicatedAllocateInfo-commonparent", pNext_loc.dot(Field::image));
            skip |=
                ValidateObject(pNext->buffer, kVulkanObjectTypeBuffer, true, "VUID-VkMemoryDedicatedAllocateInfo-buffer-parameter",
                               "VUID-VkMemoryDedicatedAllocateInfo-commonparent", pNext_loc.dot(Field::buffer));
        }
    }

    return skip;
}

void ObjectLifetimes::PostCallRecordAllocateMemory(VkDevice device, const VkMemoryAllocateInfo* pAllocateInfo,
                                                   const VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory,
                                                   const RecordObject& record_obj) {
    if (record_obj.result != VK_SUCCESS) return;
    CreateObject(*pMemory, kVulkanObjectTypeDeviceMemory, pAllocator);
}

bool ObjectLifetimes::PreCallValidateFreeMemory(VkDevice device, VkDeviceMemory memory, const VkAllocationCallbacks* pAllocator,
                                                const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkFreeMemory-device-parameter"
    skip |= ValidateObject(memory, kVulkanObjectTypeDeviceMemory, true, "VUID-vkFreeMemory-memory-parameter",
                           "VUID-vkFreeMemory-memory-parent", error_obj.location.dot(Field::memory));
    skip |= ValidateDestroyObject(memory, kVulkanObjectTypeDeviceMemory, pAllocator, kVUIDUndefined, kVUIDUndefined,
                                  error_obj.location);

    return skip;
}

void ObjectLifetimes::PreCallRecordFreeMemory(VkDevice device, VkDeviceMemory memory, const VkAllocationCallbacks* pAllocator) {
    RecordDestroyObject(memory, kVulkanObjectTypeDeviceMemory);
}

bool ObjectLifetimes::PreCallValidateMapMemory(VkDevice device, VkDeviceMemory memory, VkDeviceSize offset, VkDeviceSize size,
                                               VkMemoryMapFlags flags, void** ppData, const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkMapMemory-device-parameter"
    skip |= ValidateObject(memory, kVulkanObjectTypeDeviceMemory, false, "VUID-vkMapMemory-memory-parameter",
                           "VUID-vkMapMemory-memory-parent", error_obj.location.dot(Field::memory));

    return skip;
}

bool ObjectLifetimes::PreCallValidateUnmapMemory(VkDevice device, VkDeviceMemory memory, const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkUnmapMemory-device-parameter"
    skip |= ValidateObject(memory, kVulkanObjectTypeDeviceMemory, false, "VUID-vkUnmapMemory-memory-parameter",
                           "VUID-vkUnmapMemory-memory-parent", error_obj.location.dot(Field::memory));

    return skip;
}

bool ObjectLifetimes::PreCallValidateFlushMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount,
                                                             const VkMappedMemoryRange* pMemoryRanges,
                                                             const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkFlushMappedMemoryRanges-device-parameter"
    if (pMemoryRanges) {
        for (uint32_t index0 = 0; index0 < memoryRangeCount; ++index0) {
            [[maybe_unused]] const Location index0_loc = error_obj.location.dot(Field::pMemoryRanges, index0);
            skip |= ValidateObject(pMemoryRanges[index0].memory, kVulkanObjectTypeDeviceMemory, false,
                                   "VUID-VkMappedMemoryRange-memory-parameter", kVUIDUndefined, index0_loc.dot(Field::memory));
        }
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateInvalidateMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount,
                                                                  const VkMappedMemoryRange* pMemoryRanges,
                                                                  const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkInvalidateMappedMemoryRanges-device-parameter"
    if (pMemoryRanges) {
        for (uint32_t index0 = 0; index0 < memoryRangeCount; ++index0) {
            [[maybe_unused]] const Location index0_loc = error_obj.location.dot(Field::pMemoryRanges, index0);
            skip |= ValidateObject(pMemoryRanges[index0].memory, kVulkanObjectTypeDeviceMemory, false,
                                   "VUID-VkMappedMemoryRange-memory-parameter", kVUIDUndefined, index0_loc.dot(Field::memory));
        }
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetDeviceMemoryCommitment(VkDevice device, VkDeviceMemory memory,
                                                               VkDeviceSize* pCommittedMemoryInBytes,
                                                               const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkGetDeviceMemoryCommitment-device-parameter"
    skip |= ValidateObject(memory, kVulkanObjectTypeDeviceMemory, false, "VUID-vkGetDeviceMemoryCommitment-memory-parameter",
                           "VUID-vkGetDeviceMemoryCommitment-memory-parent", error_obj.location.dot(Field::memory));

    return skip;
}

bool ObjectLifetimes::PreCallValidateBindBufferMemory(VkDevice device, VkBuffer buffer, VkDeviceMemory memory,
                                                      VkDeviceSize memoryOffset, const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkBindBufferMemory-device-parameter"
    skip |= ValidateObject(buffer, kVulkanObjectTypeBuffer, false, "VUID-vkBindBufferMemory-buffer-parameter",
                           "VUID-vkBindBufferMemory-buffer-parent", error_obj.location.dot(Field::buffer));
    skip |= ValidateObject(memory, kVulkanObjectTypeDeviceMemory, false, "VUID-vkBindBufferMemory-memory-parameter",
                           "VUID-vkBindBufferMemory-memory-parent", error_obj.location.dot(Field::memory));

    return skip;
}

bool ObjectLifetimes::PreCallValidateBindImageMemory(VkDevice device, VkImage image, VkDeviceMemory memory,
                                                     VkDeviceSize memoryOffset, const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkBindImageMemory-device-parameter"
    skip |= ValidateObject(image, kVulkanObjectTypeImage, false, "VUID-vkBindImageMemory-image-parameter",
                           "VUID-vkBindImageMemory-image-parent", error_obj.location.dot(Field::image));
    skip |= ValidateObject(memory, kVulkanObjectTypeDeviceMemory, false, "VUID-vkBindImageMemory-memory-parameter",
                           "VUID-vkBindImageMemory-memory-parent", error_obj.location.dot(Field::memory));

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetBufferMemoryRequirements(VkDevice device, VkBuffer buffer,
                                                                 VkMemoryRequirements* pMemoryRequirements,
                                                                 const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkGetBufferMemoryRequirements-device-parameter"
    skip |= ValidateObject(buffer, kVulkanObjectTypeBuffer, false, "VUID-vkGetBufferMemoryRequirements-buffer-parameter",
                           "VUID-vkGetBufferMemoryRequirements-buffer-parent", error_obj.location.dot(Field::buffer));

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetImageMemoryRequirements(VkDevice device, VkImage image,
                                                                VkMemoryRequirements* pMemoryRequirements,
                                                                const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkGetImageMemoryRequirements-device-parameter"
    skip |= ValidateObject(image, kVulkanObjectTypeImage, false, "VUID-vkGetImageMemoryRequirements-image-parameter",
                           "VUID-vkGetImageMemoryRequirements-image-parent", error_obj.location.dot(Field::image));

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetImageSparseMemoryRequirements(VkDevice device, VkImage image,
                                                                      uint32_t* pSparseMemoryRequirementCount,
                                                                      VkSparseImageMemoryRequirements* pSparseMemoryRequirements,
                                                                      const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkGetImageSparseMemoryRequirements-device-parameter"
    skip |= ValidateObject(image, kVulkanObjectTypeImage, false, "VUID-vkGetImageSparseMemoryRequirements-image-parameter",
                           "VUID-vkGetImageSparseMemoryRequirements-image-parent", error_obj.location.dot(Field::image));

    return skip;
}

// vkGetPhysicalDeviceSparseImageFormatProperties:
// Checked by chassis: physicalDevice: "VUID-vkGetPhysicalDeviceSparseImageFormatProperties-physicalDevice-parameter"

bool ObjectLifetimes::PreCallValidateQueueBindSparse(VkQueue queue, uint32_t bindInfoCount, const VkBindSparseInfo* pBindInfo,
                                                     VkFence fence, const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: queue: "VUID-vkQueueBindSparse-queue-parameter"
    // Checked by chassis: queue: "VUID-vkQueueBindSparse-commonparent"
    if (pBindInfo) {
        for (uint32_t index0 = 0; index0 < bindInfoCount; ++index0) {
            [[maybe_unused]] const Location index0_loc = error_obj.location.dot(Field::pBindInfo, index0);

            if ((pBindInfo[index0].waitSemaphoreCount > 0) && (pBindInfo[index0].pWaitSemaphores)) {
                for (uint32_t index1 = 0; index1 < pBindInfo[index0].waitSemaphoreCount; ++index1) {
                    skip |= ValidateObject(pBindInfo[index0].pWaitSemaphores[index1], kVulkanObjectTypeSemaphore, false,
                                           "VUID-VkBindSparseInfo-pWaitSemaphores-parameter", "VUID-VkBindSparseInfo-commonparent",
                                           index0_loc.dot(Field::pWaitSemaphores, index1));
                }
            }
            if (pBindInfo[index0].pBufferBinds) {
                for (uint32_t index1 = 0; index1 < pBindInfo[index0].bufferBindCount; ++index1) {
                    [[maybe_unused]] const Location index1_loc = index0_loc.dot(Field::pBufferBinds, index1);
                    skip |= ValidateObject(pBindInfo[index0].pBufferBinds[index1].buffer, kVulkanObjectTypeBuffer, false,
                                           "VUID-VkSparseBufferMemoryBindInfo-buffer-parameter", kVUIDUndefined,
                                           index1_loc.dot(Field::buffer));
                    if (pBindInfo[index0].pBufferBinds[index1].pBinds) {
                        for (uint32_t index2 = 0; index2 < pBindInfo[index0].pBufferBinds[index1].bindCount; ++index2) {
                            [[maybe_unused]] const Location index2_loc = index1_loc.dot(Field::pBinds, index2);
                            skip |= ValidateObject(pBindInfo[index0].pBufferBinds[index1].pBinds[index2].memory,
                                                   kVulkanObjectTypeDeviceMemory, true, "VUID-VkSparseMemoryBind-memory-parameter",
                                                   kVUIDUndefined, index2_loc.dot(Field::memory));
                        }
                    }
                }
            }
            if (pBindInfo[index0].pImageOpaqueBinds) {
                for (uint32_t index1 = 0; index1 < pBindInfo[index0].imageOpaqueBindCount; ++index1) {
                    [[maybe_unused]] const Location index1_loc = index0_loc.dot(Field::pImageOpaqueBinds, index1);
                    skip |= ValidateObject(pBindInfo[index0].pImageOpaqueBinds[index1].image, kVulkanObjectTypeImage, false,
                                           "VUID-VkSparseImageOpaqueMemoryBindInfo-image-parameter", kVUIDUndefined,
                                           index1_loc.dot(Field::image));
                    if (pBindInfo[index0].pImageOpaqueBinds[index1].pBinds) {
                        for (uint32_t index2 = 0; index2 < pBindInfo[index0].pImageOpaqueBinds[index1].bindCount; ++index2) {
                            [[maybe_unused]] const Location index2_loc = index1_loc.dot(Field::pBinds, index2);
                            skip |= ValidateObject(pBindInfo[index0].pImageOpaqueBinds[index1].pBinds[index2].memory,
                                                   kVulkanObjectTypeDeviceMemory, true, "VUID-VkSparseMemoryBind-memory-parameter",
                                                   kVUIDUndefined, index2_loc.dot(Field::memory));
                        }
                    }
                }
            }
            if (pBindInfo[index0].pImageBinds) {
                for (uint32_t index1 = 0; index1 < pBindInfo[index0].imageBindCount; ++index1) {
                    [[maybe_unused]] const Location index1_loc = index0_loc.dot(Field::pImageBinds, index1);
                    skip |= ValidateObject(pBindInfo[index0].pImageBinds[index1].image, kVulkanObjectTypeImage, false,
                                           "VUID-VkSparseImageMemoryBindInfo-image-parameter", kVUIDUndefined,
                                           index1_loc.dot(Field::image));
                    if (pBindInfo[index0].pImageBinds[index1].pBinds) {
                        for (uint32_t index2 = 0; index2 < pBindInfo[index0].pImageBinds[index1].bindCount; ++index2) {
                            [[maybe_unused]] const Location index2_loc = index1_loc.dot(Field::pBinds, index2);
                            skip |= ValidateObject(
                                pBindInfo[index0].pImageBinds[index1].pBinds[index2].memory, kVulkanObjectTypeDeviceMemory, true,
                                "VUID-VkSparseImageMemoryBind-memory-parameter", kVUIDUndefined, index2_loc.dot(Field::memory));
                        }
                    }
                }
            }

            if ((pBindInfo[index0].signalSemaphoreCount > 0) && (pBindInfo[index0].pSignalSemaphores)) {
                for (uint32_t index1 = 0; index1 < pBindInfo[index0].signalSemaphoreCount; ++index1) {
                    skip |= ValidateObject(pBindInfo[index0].pSignalSemaphores[index1], kVulkanObjectTypeSemaphore, false,
                                           "VUID-VkBindSparseInfo-pSignalSemaphores-parameter",
                                           "VUID-VkBindSparseInfo-commonparent", index0_loc.dot(Field::pSignalSemaphores, index1));
                }
            }
            if (auto pNext = vku::FindStructInPNextChain<VkFrameBoundaryEXT>(pBindInfo[index0].pNext)) {
                const Location pNext_loc = index0_loc.pNext(Struct::VkFrameBoundaryEXT);

                if ((pNext->imageCount > 0) && (pNext->pImages)) {
                    for (uint32_t index2 = 0; index2 < pNext->imageCount; ++index2) {
                        skip |= ValidateObject(pNext->pImages[index2], kVulkanObjectTypeImage, false,
                                               "VUID-VkFrameBoundaryEXT-pImages-parameter", "VUID-VkFrameBoundaryEXT-commonparent",
                                               pNext_loc.dot(Field::pImages, index2));
                    }
                }

                if ((pNext->bufferCount > 0) && (pNext->pBuffers)) {
                    for (uint32_t index2 = 0; index2 < pNext->bufferCount; ++index2) {
                        skip |= ValidateObject(pNext->pBuffers[index2], kVulkanObjectTypeBuffer, false,
                                               "VUID-VkFrameBoundaryEXT-pBuffers-parameter", "VUID-VkFrameBoundaryEXT-commonparent",
                                               pNext_loc.dot(Field::pBuffers, index2));
                    }
                }
            }
        }
    }
    skip |= ValidateObject(fence, kVulkanObjectTypeFence, true, "VUID-vkQueueBindSparse-fence-parameter",
                           "VUID-vkQueueBindSparse-commonparent", error_obj.location.dot(Field::fence));

    return skip;
}

// vkCreateFence:
// Checked by chassis: device: "VUID-vkCreateFence-device-parameter"

void ObjectLifetimes::PostCallRecordCreateFence(VkDevice device, const VkFenceCreateInfo* pCreateInfo,
                                                const VkAllocationCallbacks* pAllocator, VkFence* pFence,
                                                const RecordObject& record_obj) {
    if (record_obj.result != VK_SUCCESS) return;
    CreateObject(*pFence, kVulkanObjectTypeFence, pAllocator);
}

bool ObjectLifetimes::PreCallValidateDestroyFence(VkDevice device, VkFence fence, const VkAllocationCallbacks* pAllocator,
                                                  const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkDestroyFence-device-parameter"
    skip |= ValidateObject(fence, kVulkanObjectTypeFence, true, "VUID-vkDestroyFence-fence-parameter",
                           "VUID-vkDestroyFence-fence-parent", error_obj.location.dot(Field::fence));
    skip |= ValidateDestroyObject(fence, kVulkanObjectTypeFence, pAllocator, "VUID-vkDestroyFence-fence-01121",
                                  "VUID-vkDestroyFence-fence-01122", error_obj.location);

    return skip;
}

void ObjectLifetimes::PreCallRecordDestroyFence(VkDevice device, VkFence fence, const VkAllocationCallbacks* pAllocator) {
    RecordDestroyObject(fence, kVulkanObjectTypeFence);
}

bool ObjectLifetimes::PreCallValidateResetFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences,
                                                 const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkResetFences-device-parameter"

    if ((fenceCount > 0) && (pFences)) {
        for (uint32_t index0 = 0; index0 < fenceCount; ++index0) {
            skip |= ValidateObject(pFences[index0], kVulkanObjectTypeFence, false, "VUID-vkResetFences-pFences-parameter",
                                   "VUID-vkResetFences-pFences-parent", error_obj.location.dot(Field::pFences, index0));
        }
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetFenceStatus(VkDevice device, VkFence fence, const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkGetFenceStatus-device-parameter"
    skip |= ValidateObject(fence, kVulkanObjectTypeFence, false, "VUID-vkGetFenceStatus-fence-parameter",
                           "VUID-vkGetFenceStatus-fence-parent", error_obj.location.dot(Field::fence));

    return skip;
}

bool ObjectLifetimes::PreCallValidateWaitForFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences, VkBool32 waitAll,
                                                   uint64_t timeout, const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkWaitForFences-device-parameter"

    if ((fenceCount > 0) && (pFences)) {
        for (uint32_t index0 = 0; index0 < fenceCount; ++index0) {
            skip |= ValidateObject(pFences[index0], kVulkanObjectTypeFence, false, "VUID-vkWaitForFences-pFences-parameter",
                                   "VUID-vkWaitForFences-pFences-parent", error_obj.location.dot(Field::pFences, index0));
        }
    }

    return skip;
}

// vkCreateSemaphore:
// Checked by chassis: device: "VUID-vkCreateSemaphore-device-parameter"

void ObjectLifetimes::PostCallRecordCreateSemaphore(VkDevice device, const VkSemaphoreCreateInfo* pCreateInfo,
                                                    const VkAllocationCallbacks* pAllocator, VkSemaphore* pSemaphore,
                                                    const RecordObject& record_obj) {
    if (record_obj.result != VK_SUCCESS) return;
    CreateObject(*pSemaphore, kVulkanObjectTypeSemaphore, pAllocator);
}

bool ObjectLifetimes::PreCallValidateDestroySemaphore(VkDevice device, VkSemaphore semaphore,
                                                      const VkAllocationCallbacks* pAllocator, const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkDestroySemaphore-device-parameter"
    skip |= ValidateObject(semaphore, kVulkanObjectTypeSemaphore, true, "VUID-vkDestroySemaphore-semaphore-parameter",
                           "VUID-vkDestroySemaphore-semaphore-parent", error_obj.location.dot(Field::semaphore));
    skip |= ValidateDestroyObject(semaphore, kVulkanObjectTypeSemaphore, pAllocator, "VUID-vkDestroySemaphore-semaphore-01138",
                                  "VUID-vkDestroySemaphore-semaphore-01139", error_obj.location);

    return skip;
}

void ObjectLifetimes::PreCallRecordDestroySemaphore(VkDevice device, VkSemaphore semaphore,
                                                    const VkAllocationCallbacks* pAllocator) {
    RecordDestroyObject(semaphore, kVulkanObjectTypeSemaphore);
}

// vkCreateEvent:
// Checked by chassis: device: "VUID-vkCreateEvent-device-parameter"

void ObjectLifetimes::PostCallRecordCreateEvent(VkDevice device, const VkEventCreateInfo* pCreateInfo,
                                                const VkAllocationCallbacks* pAllocator, VkEvent* pEvent,
                                                const RecordObject& record_obj) {
    if (record_obj.result != VK_SUCCESS) return;
    CreateObject(*pEvent, kVulkanObjectTypeEvent, pAllocator);
}

bool ObjectLifetimes::PreCallValidateDestroyEvent(VkDevice device, VkEvent event, const VkAllocationCallbacks* pAllocator,
                                                  const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkDestroyEvent-device-parameter"
    skip |= ValidateObject(event, kVulkanObjectTypeEvent, true, "VUID-vkDestroyEvent-event-parameter",
                           "VUID-vkDestroyEvent-event-parent", error_obj.location.dot(Field::event));
    skip |= ValidateDestroyObject(event, kVulkanObjectTypeEvent, pAllocator, "VUID-vkDestroyEvent-event-01146",
                                  "VUID-vkDestroyEvent-event-01147", error_obj.location);

    return skip;
}

void ObjectLifetimes::PreCallRecordDestroyEvent(VkDevice device, VkEvent event, const VkAllocationCallbacks* pAllocator) {
    RecordDestroyObject(event, kVulkanObjectTypeEvent);
}

bool ObjectLifetimes::PreCallValidateGetEventStatus(VkDevice device, VkEvent event, const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkGetEventStatus-device-parameter"
    skip |= ValidateObject(event, kVulkanObjectTypeEvent, false, "VUID-vkGetEventStatus-event-parameter",
                           "VUID-vkGetEventStatus-event-parent", error_obj.location.dot(Field::event));

    return skip;
}

bool ObjectLifetimes::PreCallValidateSetEvent(VkDevice device, VkEvent event, const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkSetEvent-device-parameter"
    skip |= ValidateObject(event, kVulkanObjectTypeEvent, false, "VUID-vkSetEvent-event-parameter", "VUID-vkSetEvent-event-parent",
                           error_obj.location.dot(Field::event));

    return skip;
}

bool ObjectLifetimes::PreCallValidateResetEvent(VkDevice device, VkEvent event, const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkResetEvent-device-parameter"
    skip |= ValidateObject(event, kVulkanObjectTypeEvent, false, "VUID-vkResetEvent-event-parameter",
                           "VUID-vkResetEvent-event-parent", error_obj.location.dot(Field::event));

    return skip;
}

// vkCreateQueryPool:
// Checked by chassis: device: "VUID-vkCreateQueryPool-device-parameter"

void ObjectLifetimes::PostCallRecordCreateQueryPool(VkDevice device, const VkQueryPoolCreateInfo* pCreateInfo,
                                                    const VkAllocationCallbacks* pAllocator, VkQueryPool* pQueryPool,
                                                    const RecordObject& record_obj) {
    if (record_obj.result != VK_SUCCESS) return;
    CreateObject(*pQueryPool, kVulkanObjectTypeQueryPool, pAllocator);
}

bool ObjectLifetimes::PreCallValidateDestroyQueryPool(VkDevice device, VkQueryPool queryPool,
                                                      const VkAllocationCallbacks* pAllocator, const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkDestroyQueryPool-device-parameter"
    skip |= ValidateObject(queryPool, kVulkanObjectTypeQueryPool, true, "VUID-vkDestroyQueryPool-queryPool-parameter",
                           "VUID-vkDestroyQueryPool-queryPool-parent", error_obj.location.dot(Field::queryPool));
    skip |= ValidateDestroyObject(queryPool, kVulkanObjectTypeQueryPool, pAllocator, "VUID-vkDestroyQueryPool-queryPool-00794",
                                  "VUID-vkDestroyQueryPool-queryPool-00795", error_obj.location);

    return skip;
}

void ObjectLifetimes::PreCallRecordDestroyQueryPool(VkDevice device, VkQueryPool queryPool,
                                                    const VkAllocationCallbacks* pAllocator) {
    RecordDestroyObject(queryPool, kVulkanObjectTypeQueryPool);
}

bool ObjectLifetimes::PreCallValidateGetQueryPoolResults(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery,
                                                         uint32_t queryCount, size_t dataSize, void* pData, VkDeviceSize stride,
                                                         VkQueryResultFlags flags, const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkGetQueryPoolResults-device-parameter"
    skip |= ValidateObject(queryPool, kVulkanObjectTypeQueryPool, false, "VUID-vkGetQueryPoolResults-queryPool-parameter",
                           "VUID-vkGetQueryPoolResults-queryPool-parent", error_obj.location.dot(Field::queryPool));

    return skip;
}

bool ObjectLifetimes::PreCallValidateCreateBuffer(VkDevice device, const VkBufferCreateInfo* pCreateInfo,
                                                  const VkAllocationCallbacks* pAllocator, VkBuffer* pBuffer,
                                                  const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkCreateBuffer-device-parameter"
    if (pCreateInfo) {
        [[maybe_unused]] const Location pCreateInfo_loc = error_obj.location.dot(Field::pCreateInfo);
#ifdef VK_USE_PLATFORM_FUCHSIA
        if (auto pNext = vku::FindStructInPNextChain<VkBufferCollectionBufferCreateInfoFUCHSIA>(pCreateInfo->pNext)) {
            const Location pNext_loc = pCreateInfo_loc.pNext(Struct::VkBufferCollectionBufferCreateInfoFUCHSIA);
            skip |= ValidateObject(pNext->collection, kVulkanObjectTypeBufferCollectionFUCHSIA, false,
                                   "VUID-VkBufferCollectionBufferCreateInfoFUCHSIA-collection-parameter", kVUIDUndefined,
                                   pNext_loc.dot(Field::collection));
        }
#endif  // VK_USE_PLATFORM_FUCHSIA
    }

    return skip;
}

void ObjectLifetimes::PostCallRecordCreateBuffer(VkDevice device, const VkBufferCreateInfo* pCreateInfo,
                                                 const VkAllocationCallbacks* pAllocator, VkBuffer* pBuffer,
                                                 const RecordObject& record_obj) {
    if (record_obj.result != VK_SUCCESS) return;
    CreateObject(*pBuffer, kVulkanObjectTypeBuffer, pAllocator);
}

bool ObjectLifetimes::PreCallValidateDestroyBuffer(VkDevice device, VkBuffer buffer, const VkAllocationCallbacks* pAllocator,
                                                   const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkDestroyBuffer-device-parameter"
    skip |= ValidateObject(buffer, kVulkanObjectTypeBuffer, true, "VUID-vkDestroyBuffer-buffer-parameter",
                           "VUID-vkDestroyBuffer-buffer-parent", error_obj.location.dot(Field::buffer));
    skip |= ValidateDestroyObject(buffer, kVulkanObjectTypeBuffer, pAllocator, "VUID-vkDestroyBuffer-buffer-00923",
                                  "VUID-vkDestroyBuffer-buffer-00924", error_obj.location);

    return skip;
}

void ObjectLifetimes::PreCallRecordDestroyBuffer(VkDevice device, VkBuffer buffer, const VkAllocationCallbacks* pAllocator) {
    RecordDestroyObject(buffer, kVulkanObjectTypeBuffer);
}

bool ObjectLifetimes::PreCallValidateCreateBufferView(VkDevice device, const VkBufferViewCreateInfo* pCreateInfo,
                                                      const VkAllocationCallbacks* pAllocator, VkBufferView* pView,
                                                      const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkCreateBufferView-device-parameter"
    if (pCreateInfo) {
        [[maybe_unused]] const Location pCreateInfo_loc = error_obj.location.dot(Field::pCreateInfo);
        skip |= ValidateObject(pCreateInfo->buffer, kVulkanObjectTypeBuffer, false, "VUID-VkBufferViewCreateInfo-buffer-parameter",
                               kVUIDUndefined, pCreateInfo_loc.dot(Field::buffer));
    }

    return skip;
}

void ObjectLifetimes::PostCallRecordCreateBufferView(VkDevice device, const VkBufferViewCreateInfo* pCreateInfo,
                                                     const VkAllocationCallbacks* pAllocator, VkBufferView* pView,
                                                     const RecordObject& record_obj) {
    if (record_obj.result != VK_SUCCESS) return;
    CreateObject(*pView, kVulkanObjectTypeBufferView, pAllocator);
}

bool ObjectLifetimes::PreCallValidateDestroyBufferView(VkDevice device, VkBufferView bufferView,
                                                       const VkAllocationCallbacks* pAllocator,
                                                       const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkDestroyBufferView-device-parameter"
    skip |= ValidateObject(bufferView, kVulkanObjectTypeBufferView, true, "VUID-vkDestroyBufferView-bufferView-parameter",
                           "VUID-vkDestroyBufferView-bufferView-parent", error_obj.location.dot(Field::bufferView));
    skip |= ValidateDestroyObject(bufferView, kVulkanObjectTypeBufferView, pAllocator, "VUID-vkDestroyBufferView-bufferView-00937",
                                  "VUID-vkDestroyBufferView-bufferView-00938", error_obj.location);

    return skip;
}

void ObjectLifetimes::PreCallRecordDestroyBufferView(VkDevice device, VkBufferView bufferView,
                                                     const VkAllocationCallbacks* pAllocator) {
    RecordDestroyObject(bufferView, kVulkanObjectTypeBufferView);
}

bool ObjectLifetimes::PreCallValidateCreateImage(VkDevice device, const VkImageCreateInfo* pCreateInfo,
                                                 const VkAllocationCallbacks* pAllocator, VkImage* pImage,
                                                 const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkCreateImage-device-parameter"
    if (pCreateInfo) {
        [[maybe_unused]] const Location pCreateInfo_loc = error_obj.location.dot(Field::pCreateInfo);
#ifdef VK_USE_PLATFORM_FUCHSIA
        if (auto pNext = vku::FindStructInPNextChain<VkBufferCollectionImageCreateInfoFUCHSIA>(pCreateInfo->pNext)) {
            const Location pNext_loc = pCreateInfo_loc.pNext(Struct::VkBufferCollectionImageCreateInfoFUCHSIA);
            skip |= ValidateObject(pNext->collection, kVulkanObjectTypeBufferCollectionFUCHSIA, false,
                                   "VUID-VkBufferCollectionImageCreateInfoFUCHSIA-collection-parameter", kVUIDUndefined,
                                   pNext_loc.dot(Field::collection));
        }
#endif  // VK_USE_PLATFORM_FUCHSIA
        if (auto pNext = vku::FindStructInPNextChain<VkImageSwapchainCreateInfoKHR>(pCreateInfo->pNext)) {
            const Location pNext_loc = pCreateInfo_loc.pNext(Struct::VkImageSwapchainCreateInfoKHR);
            skip |= ValidateObject(pNext->swapchain, kVulkanObjectTypeSwapchainKHR, true,
                                   "VUID-VkImageSwapchainCreateInfoKHR-swapchain-parameter", kVUIDUndefined,
                                   pNext_loc.dot(Field::swapchain));
        }
    }

    return skip;
}

void ObjectLifetimes::PostCallRecordCreateImage(VkDevice device, const VkImageCreateInfo* pCreateInfo,
                                                const VkAllocationCallbacks* pAllocator, VkImage* pImage,
                                                const RecordObject& record_obj) {
    if (record_obj.result != VK_SUCCESS) return;
    CreateObject(*pImage, kVulkanObjectTypeImage, pAllocator);
}

bool ObjectLifetimes::PreCallValidateDestroyImage(VkDevice device, VkImage image, const VkAllocationCallbacks* pAllocator,
                                                  const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkDestroyImage-device-parameter"
    skip |= ValidateObject(image, kVulkanObjectTypeImage, true, "VUID-vkDestroyImage-image-parameter",
                           "VUID-vkDestroyImage-image-parent", error_obj.location.dot(Field::image));
    skip |= ValidateDestroyObject(image, kVulkanObjectTypeImage, pAllocator, "VUID-vkDestroyImage-image-01001",
                                  "VUID-vkDestroyImage-image-01002", error_obj.location);

    return skip;
}

void ObjectLifetimes::PreCallRecordDestroyImage(VkDevice device, VkImage image, const VkAllocationCallbacks* pAllocator) {
    RecordDestroyObject(image, kVulkanObjectTypeImage);
}

bool ObjectLifetimes::PreCallValidateGetImageSubresourceLayout(VkDevice device, VkImage image,
                                                               const VkImageSubresource* pSubresource, VkSubresourceLayout* pLayout,
                                                               const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkGetImageSubresourceLayout-device-parameter"
    skip |= ValidateObject(image, kVulkanObjectTypeImage, false, "VUID-vkGetImageSubresourceLayout-image-parameter",
                           "VUID-vkGetImageSubresourceLayout-image-parent", error_obj.location.dot(Field::image));

    return skip;
}

bool ObjectLifetimes::PreCallValidateCreateImageView(VkDevice device, const VkImageViewCreateInfo* pCreateInfo,
                                                     const VkAllocationCallbacks* pAllocator, VkImageView* pView,
                                                     const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkCreateImageView-device-parameter"
    if (pCreateInfo) {
        [[maybe_unused]] const Location pCreateInfo_loc = error_obj.location.dot(Field::pCreateInfo);
        skip |= ValidateObject(pCreateInfo->image, kVulkanObjectTypeImage, false, "VUID-VkImageViewCreateInfo-image-parameter",
                               "VUID-vkCreateImageView-image-09179", pCreateInfo_loc.dot(Field::image));
        if (auto pNext = vku::FindStructInPNextChain<VkSamplerYcbcrConversionInfo>(pCreateInfo->pNext)) {
            const Location pNext_loc = pCreateInfo_loc.pNext(Struct::VkSamplerYcbcrConversionInfo);
            skip |= ValidateObject(pNext->conversion, kVulkanObjectTypeSamplerYcbcrConversion, false,
                                   "VUID-VkSamplerYcbcrConversionInfo-conversion-parameter", kVUIDUndefined,
                                   pNext_loc.dot(Field::conversion));
        }
    }

    return skip;
}

void ObjectLifetimes::PostCallRecordCreateImageView(VkDevice device, const VkImageViewCreateInfo* pCreateInfo,
                                                    const VkAllocationCallbacks* pAllocator, VkImageView* pView,
                                                    const RecordObject& record_obj) {
    if (record_obj.result != VK_SUCCESS) return;
    CreateObject(*pView, kVulkanObjectTypeImageView, pAllocator);
}

bool ObjectLifetimes::PreCallValidateDestroyImageView(VkDevice device, VkImageView imageView,
                                                      const VkAllocationCallbacks* pAllocator, const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkDestroyImageView-device-parameter"
    skip |= ValidateObject(imageView, kVulkanObjectTypeImageView, true, "VUID-vkDestroyImageView-imageView-parameter",
                           "VUID-vkDestroyImageView-imageView-parent", error_obj.location.dot(Field::imageView));
    skip |= ValidateDestroyObject(imageView, kVulkanObjectTypeImageView, pAllocator, "VUID-vkDestroyImageView-imageView-01027",
                                  "VUID-vkDestroyImageView-imageView-01028", error_obj.location);

    return skip;
}

void ObjectLifetimes::PreCallRecordDestroyImageView(VkDevice device, VkImageView imageView,
                                                    const VkAllocationCallbacks* pAllocator) {
    RecordDestroyObject(imageView, kVulkanObjectTypeImageView);
}

bool ObjectLifetimes::PreCallValidateCreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo,
                                                        const VkAllocationCallbacks* pAllocator, VkShaderModule* pShaderModule,
                                                        const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkCreateShaderModule-device-parameter"
    if (pCreateInfo) {
        [[maybe_unused]] const Location pCreateInfo_loc = error_obj.location.dot(Field::pCreateInfo);
        if (auto pNext = vku::FindStructInPNextChain<VkShaderModuleValidationCacheCreateInfoEXT>(pCreateInfo->pNext)) {
            const Location pNext_loc = pCreateInfo_loc.pNext(Struct::VkShaderModuleValidationCacheCreateInfoEXT);
            skip |= ValidateObject(pNext->validationCache, kVulkanObjectTypeValidationCacheEXT, false,
                                   "VUID-VkShaderModuleValidationCacheCreateInfoEXT-validationCache-parameter", kVUIDUndefined,
                                   pNext_loc.dot(Field::validationCache));
        }
    }

    return skip;
}

void ObjectLifetimes::PostCallRecordCreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo,
                                                       const VkAllocationCallbacks* pAllocator, VkShaderModule* pShaderModule,
                                                       const RecordObject& record_obj) {
    if (record_obj.result != VK_SUCCESS) return;
    CreateObject(*pShaderModule, kVulkanObjectTypeShaderModule, pAllocator);
}

bool ObjectLifetimes::PreCallValidateDestroyShaderModule(VkDevice device, VkShaderModule shaderModule,
                                                         const VkAllocationCallbacks* pAllocator,
                                                         const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkDestroyShaderModule-device-parameter"
    skip |= ValidateObject(shaderModule, kVulkanObjectTypeShaderModule, true, "VUID-vkDestroyShaderModule-shaderModule-parameter",
                           "VUID-vkDestroyShaderModule-shaderModule-parent", error_obj.location.dot(Field::shaderModule));
    skip |= ValidateDestroyObject(shaderModule, kVulkanObjectTypeShaderModule, pAllocator,
                                  "VUID-vkDestroyShaderModule-shaderModule-01092", "VUID-vkDestroyShaderModule-shaderModule-01093",
                                  error_obj.location);

    return skip;
}

void ObjectLifetimes::PreCallRecordDestroyShaderModule(VkDevice device, VkShaderModule shaderModule,
                                                       const VkAllocationCallbacks* pAllocator) {
    RecordDestroyObject(shaderModule, kVulkanObjectTypeShaderModule);
}

// vkCreatePipelineCache:
// Checked by chassis: device: "VUID-vkCreatePipelineCache-device-parameter"

void ObjectLifetimes::PostCallRecordCreatePipelineCache(VkDevice device, const VkPipelineCacheCreateInfo* pCreateInfo,
                                                        const VkAllocationCallbacks* pAllocator, VkPipelineCache* pPipelineCache,
                                                        const RecordObject& record_obj) {
    if (record_obj.result != VK_SUCCESS) return;
    CreateObject(*pPipelineCache, kVulkanObjectTypePipelineCache, pAllocator);
}

bool ObjectLifetimes::PreCallValidateDestroyPipelineCache(VkDevice device, VkPipelineCache pipelineCache,
                                                          const VkAllocationCallbacks* pAllocator,
                                                          const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkDestroyPipelineCache-device-parameter"
    skip |=
        ValidateObject(pipelineCache, kVulkanObjectTypePipelineCache, true, "VUID-vkDestroyPipelineCache-pipelineCache-parameter",
                       "VUID-vkDestroyPipelineCache-pipelineCache-parent", error_obj.location.dot(Field::pipelineCache));
    skip |= ValidateDestroyObject(pipelineCache, kVulkanObjectTypePipelineCache, pAllocator,
                                  "VUID-vkDestroyPipelineCache-pipelineCache-00771",
                                  "VUID-vkDestroyPipelineCache-pipelineCache-00772", error_obj.location);

    return skip;
}

void ObjectLifetimes::PreCallRecordDestroyPipelineCache(VkDevice device, VkPipelineCache pipelineCache,
                                                        const VkAllocationCallbacks* pAllocator) {
    RecordDestroyObject(pipelineCache, kVulkanObjectTypePipelineCache);
}

bool ObjectLifetimes::PreCallValidateGetPipelineCacheData(VkDevice device, VkPipelineCache pipelineCache, size_t* pDataSize,
                                                          void* pData, const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkGetPipelineCacheData-device-parameter"
    skip |=
        ValidateObject(pipelineCache, kVulkanObjectTypePipelineCache, false, "VUID-vkGetPipelineCacheData-pipelineCache-parameter",
                       "VUID-vkGetPipelineCacheData-pipelineCache-parent", error_obj.location.dot(Field::pipelineCache));

    return skip;
}

bool ObjectLifetimes::PreCallValidateMergePipelineCaches(VkDevice device, VkPipelineCache dstCache, uint32_t srcCacheCount,
                                                         const VkPipelineCache* pSrcCaches, const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkMergePipelineCaches-device-parameter"
    skip |= ValidateObject(dstCache, kVulkanObjectTypePipelineCache, false, "VUID-vkMergePipelineCaches-dstCache-parameter",
                           "VUID-vkMergePipelineCaches-dstCache-parent", error_obj.location.dot(Field::dstCache));

    if ((srcCacheCount > 0) && (pSrcCaches)) {
        for (uint32_t index0 = 0; index0 < srcCacheCount; ++index0) {
            skip |= ValidateObject(
                pSrcCaches[index0], kVulkanObjectTypePipelineCache, false, "VUID-vkMergePipelineCaches-pSrcCaches-parameter",
                "VUID-vkMergePipelineCaches-pSrcCaches-parent", error_obj.location.dot(Field::pSrcCaches, index0));
        }
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache,
                                                             uint32_t createInfoCount,
                                                             const VkGraphicsPipelineCreateInfo* pCreateInfos,
                                                             const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                             const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkCreateGraphicsPipelines-device-parameter"
    skip |= ValidateObject(pipelineCache, kVulkanObjectTypePipelineCache, true,
                           "VUID-vkCreateGraphicsPipelines-pipelineCache-parameter",
                           "VUID-vkCreateGraphicsPipelines-pipelineCache-parent", error_obj.location.dot(Field::pipelineCache));
    if (pCreateInfos) {
        for (uint32_t index0 = 0; index0 < createInfoCount; ++index0) {
            [[maybe_unused]] const Location index0_loc = error_obj.location.dot(Field::pCreateInfos, index0);
            if (pCreateInfos[index0].pStages) {
                for (uint32_t index1 = 0; index1 < pCreateInfos[index0].stageCount; ++index1) {
                    [[maybe_unused]] const Location index1_loc = index0_loc.dot(Field::pStages, index1);
                    skip |= ValidateObject(pCreateInfos[index0].pStages[index1].module, kVulkanObjectTypeShaderModule, true,
                                           "VUID-VkPipelineShaderStageCreateInfo-module-parameter", kVUIDUndefined,
                                           index1_loc.dot(Field::module));
                    if (auto pNext = vku::FindStructInPNextChain<VkShaderModuleValidationCacheCreateInfoEXT>(
                            pCreateInfos[index0].pStages[index1].pNext)) {
                        const Location pNext_loc = index1_loc.pNext(Struct::VkShaderModuleValidationCacheCreateInfoEXT);
                        skip |= ValidateObject(pNext->validationCache, kVulkanObjectTypeValidationCacheEXT, false,
                                               "VUID-VkShaderModuleValidationCacheCreateInfoEXT-validationCache-parameter",
                                               kVUIDUndefined, pNext_loc.dot(Field::validationCache));
                    }
                }
            }
            skip |= ValidateObject(pCreateInfos[index0].layout, kVulkanObjectTypePipelineLayout, true, kVUIDUndefined,
                                   "VUID-VkGraphicsPipelineCreateInfo-commonparent", index0_loc.dot(Field::layout));
            skip |= ValidateObject(pCreateInfos[index0].renderPass, kVulkanObjectTypeRenderPass, true, kVUIDUndefined,
                                   "VUID-VkGraphicsPipelineCreateInfo-commonparent", index0_loc.dot(Field::renderPass));
            if ((pCreateInfos[index0].flags & VK_PIPELINE_CREATE_DERIVATIVE_BIT) && (pCreateInfos[index0].basePipelineIndex == -1))
                skip |= ValidateObject(pCreateInfos[index0].basePipelineHandle, kVulkanObjectTypePipeline, false,
                                       "VUID-VkGraphicsPipelineCreateInfo-flags-07984",
                                       "VUID-VkGraphicsPipelineCreateInfo-commonparent", error_obj.location);
            if (auto pNext = vku::FindStructInPNextChain<VkGraphicsPipelineShaderGroupsCreateInfoNV>(pCreateInfos[index0].pNext)) {
                const Location pNext_loc = index0_loc.pNext(Struct::VkGraphicsPipelineShaderGroupsCreateInfoNV);

                if ((pNext->pipelineCount > 0) && (pNext->pPipelines)) {
                    for (uint32_t index2 = 0; index2 < pNext->pipelineCount; ++index2) {
                        skip |= ValidateObject(pNext->pPipelines[index2], kVulkanObjectTypePipeline, false,
                                               "VUID-VkGraphicsPipelineShaderGroupsCreateInfoNV-pPipelines-parameter",
                                               kVUIDUndefined, pNext_loc.dot(Field::pPipelines, index2));
                    }
                }
            }
            if (auto pNext = vku::FindStructInPNextChain<VkPipelineLibraryCreateInfoKHR>(pCreateInfos[index0].pNext)) {
                const Location pNext_loc = index0_loc.pNext(Struct::VkPipelineLibraryCreateInfoKHR);

                if ((pNext->libraryCount > 0) && (pNext->pLibraries)) {
                    for (uint32_t index2 = 0; index2 < pNext->libraryCount; ++index2) {
                        skip |= ValidateObject(pNext->pLibraries[index2], kVulkanObjectTypePipeline, false,
                                               "VUID-VkPipelineLibraryCreateInfoKHR-pLibraries-parameter", kVUIDUndefined,
                                               pNext_loc.dot(Field::pLibraries, index2));
                    }
                }
            }
        }
    }

    return skip;
}

void ObjectLifetimes::PostCallRecordCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache,
                                                            uint32_t createInfoCount,
                                                            const VkGraphicsPipelineCreateInfo* pCreateInfos,
                                                            const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                            const RecordObject& record_obj) {
    if (VK_ERROR_VALIDATION_FAILED_EXT == record_obj.result) return;
    if (pPipelines) {
        for (uint32_t index = 0; index < createInfoCount; index++) {
            if (!pPipelines[index]) continue;
            CreateObject(pPipelines[index], kVulkanObjectTypePipeline, pAllocator);
        }
    }
}

bool ObjectLifetimes::PreCallValidateCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache,
                                                            uint32_t createInfoCount,
                                                            const VkComputePipelineCreateInfo* pCreateInfos,
                                                            const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                            const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkCreateComputePipelines-device-parameter"
    skip |=
        ValidateObject(pipelineCache, kVulkanObjectTypePipelineCache, true, "VUID-vkCreateComputePipelines-pipelineCache-parameter",
                       "VUID-vkCreateComputePipelines-pipelineCache-parent", error_obj.location.dot(Field::pipelineCache));
    if (pCreateInfos) {
        for (uint32_t index0 = 0; index0 < createInfoCount; ++index0) {
            [[maybe_unused]] const Location index0_loc = error_obj.location.dot(Field::pCreateInfos, index0);
            [[maybe_unused]] const Location stage_loc = index0_loc.dot(Field::stage);
            skip |= ValidateObject(pCreateInfos[index0].stage.module, kVulkanObjectTypeShaderModule, true,
                                   "VUID-VkPipelineShaderStageCreateInfo-module-parameter", kVUIDUndefined,
                                   stage_loc.dot(Field::module));
            if (auto pNext =
                    vku::FindStructInPNextChain<VkShaderModuleValidationCacheCreateInfoEXT>(pCreateInfos[index0].stage.pNext)) {
                const Location pNext_loc = stage_loc.pNext(Struct::VkShaderModuleValidationCacheCreateInfoEXT);
                skip |= ValidateObject(pNext->validationCache, kVulkanObjectTypeValidationCacheEXT, false,
                                       "VUID-VkShaderModuleValidationCacheCreateInfoEXT-validationCache-parameter", kVUIDUndefined,
                                       pNext_loc.dot(Field::validationCache));
            }
            skip |= ValidateObject(pCreateInfos[index0].layout, kVulkanObjectTypePipelineLayout, false,
                                   "VUID-VkComputePipelineCreateInfo-layout-parameter",
                                   "VUID-VkComputePipelineCreateInfo-commonparent", index0_loc.dot(Field::layout));
            if ((pCreateInfos[index0].flags & VK_PIPELINE_CREATE_DERIVATIVE_BIT) && (pCreateInfos[index0].basePipelineIndex == -1))
                skip |= ValidateObject(pCreateInfos[index0].basePipelineHandle, kVulkanObjectTypePipeline, false,
                                       "VUID-VkComputePipelineCreateInfo-flags-07984",
                                       "VUID-VkComputePipelineCreateInfo-commonparent", error_obj.location);
            if (auto pNext = vku::FindStructInPNextChain<VkSubpassShadingPipelineCreateInfoHUAWEI>(pCreateInfos[index0].pNext)) {
                const Location pNext_loc = index0_loc.pNext(Struct::VkSubpassShadingPipelineCreateInfoHUAWEI);
                skip |= ValidateObject(pNext->renderPass, kVulkanObjectTypeRenderPass, false, kVUIDUndefined, kVUIDUndefined,
                                       pNext_loc.dot(Field::renderPass));
            }
        }
    }

    return skip;
}

void ObjectLifetimes::PostCallRecordCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                           const VkComputePipelineCreateInfo* pCreateInfos,
                                                           const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                           const RecordObject& record_obj) {
    if (VK_ERROR_VALIDATION_FAILED_EXT == record_obj.result) return;
    if (pPipelines) {
        for (uint32_t index = 0; index < createInfoCount; index++) {
            if (!pPipelines[index]) continue;
            CreateObject(pPipelines[index], kVulkanObjectTypePipeline, pAllocator);
        }
    }
}

bool ObjectLifetimes::PreCallValidateDestroyPipeline(VkDevice device, VkPipeline pipeline, const VkAllocationCallbacks* pAllocator,
                                                     const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkDestroyPipeline-device-parameter"
    skip |= ValidateObject(pipeline, kVulkanObjectTypePipeline, true, "VUID-vkDestroyPipeline-pipeline-parameter",
                           "VUID-vkDestroyPipeline-pipeline-parent", error_obj.location.dot(Field::pipeline));
    skip |= ValidateDestroyObject(pipeline, kVulkanObjectTypePipeline, pAllocator, "VUID-vkDestroyPipeline-pipeline-00766",
                                  "VUID-vkDestroyPipeline-pipeline-00767", error_obj.location);

    return skip;
}

void ObjectLifetimes::PreCallRecordDestroyPipeline(VkDevice device, VkPipeline pipeline, const VkAllocationCallbacks* pAllocator) {
    RecordDestroyObject(pipeline, kVulkanObjectTypePipeline);
}

bool ObjectLifetimes::PreCallValidateCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo* pCreateInfo,
                                                          const VkAllocationCallbacks* pAllocator,
                                                          VkPipelineLayout* pPipelineLayout, const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkCreatePipelineLayout-device-parameter"
    if (pCreateInfo) {
        [[maybe_unused]] const Location pCreateInfo_loc = error_obj.location.dot(Field::pCreateInfo);

        if ((pCreateInfo->setLayoutCount > 0) && (pCreateInfo->pSetLayouts)) {
            for (uint32_t index1 = 0; index1 < pCreateInfo->setLayoutCount; ++index1) {
                skip |= ValidateObject(pCreateInfo->pSetLayouts[index1], kVulkanObjectTypeDescriptorSetLayout, true,
                                       "VUID-VkPipelineLayoutCreateInfo-pSetLayouts-parameter", kVUIDUndefined,
                                       pCreateInfo_loc.dot(Field::pSetLayouts, index1));
            }
        }
    }

    return skip;
}

void ObjectLifetimes::PostCallRecordCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo* pCreateInfo,
                                                         const VkAllocationCallbacks* pAllocator, VkPipelineLayout* pPipelineLayout,
                                                         const RecordObject& record_obj) {
    if (record_obj.result != VK_SUCCESS) return;
    CreateObject(*pPipelineLayout, kVulkanObjectTypePipelineLayout, pAllocator);
}

bool ObjectLifetimes::PreCallValidateDestroyPipelineLayout(VkDevice device, VkPipelineLayout pipelineLayout,
                                                           const VkAllocationCallbacks* pAllocator,
                                                           const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkDestroyPipelineLayout-device-parameter"
    skip |= ValidateObject(pipelineLayout, kVulkanObjectTypePipelineLayout, true,
                           "VUID-vkDestroyPipelineLayout-pipelineLayout-parameter",
                           "VUID-vkDestroyPipelineLayout-pipelineLayout-parent", error_obj.location.dot(Field::pipelineLayout));
    skip |= ValidateDestroyObject(pipelineLayout, kVulkanObjectTypePipelineLayout, pAllocator,
                                  "VUID-vkDestroyPipelineLayout-pipelineLayout-00299",
                                  "VUID-vkDestroyPipelineLayout-pipelineLayout-00300", error_obj.location);

    return skip;
}

void ObjectLifetimes::PreCallRecordDestroyPipelineLayout(VkDevice device, VkPipelineLayout pipelineLayout,
                                                         const VkAllocationCallbacks* pAllocator) {
    RecordDestroyObject(pipelineLayout, kVulkanObjectTypePipelineLayout);
}

bool ObjectLifetimes::PreCallValidateCreateSampler(VkDevice device, const VkSamplerCreateInfo* pCreateInfo,
                                                   const VkAllocationCallbacks* pAllocator, VkSampler* pSampler,
                                                   const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkCreateSampler-device-parameter"
    if (pCreateInfo) {
        [[maybe_unused]] const Location pCreateInfo_loc = error_obj.location.dot(Field::pCreateInfo);
        if (auto pNext = vku::FindStructInPNextChain<VkSamplerYcbcrConversionInfo>(pCreateInfo->pNext)) {
            const Location pNext_loc = pCreateInfo_loc.pNext(Struct::VkSamplerYcbcrConversionInfo);
            skip |= ValidateObject(pNext->conversion, kVulkanObjectTypeSamplerYcbcrConversion, false,
                                   "VUID-VkSamplerYcbcrConversionInfo-conversion-parameter", kVUIDUndefined,
                                   pNext_loc.dot(Field::conversion));
        }
    }

    return skip;
}

void ObjectLifetimes::PostCallRecordCreateSampler(VkDevice device, const VkSamplerCreateInfo* pCreateInfo,
                                                  const VkAllocationCallbacks* pAllocator, VkSampler* pSampler,
                                                  const RecordObject& record_obj) {
    if (record_obj.result != VK_SUCCESS) return;
    CreateObject(*pSampler, kVulkanObjectTypeSampler, pAllocator);
}

bool ObjectLifetimes::PreCallValidateDestroySampler(VkDevice device, VkSampler sampler, const VkAllocationCallbacks* pAllocator,
                                                    const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkDestroySampler-device-parameter"
    skip |= ValidateObject(sampler, kVulkanObjectTypeSampler, true, "VUID-vkDestroySampler-sampler-parameter",
                           "VUID-vkDestroySampler-sampler-parent", error_obj.location.dot(Field::sampler));
    skip |= ValidateDestroyObject(sampler, kVulkanObjectTypeSampler, pAllocator, "VUID-vkDestroySampler-sampler-01083",
                                  "VUID-vkDestroySampler-sampler-01084", error_obj.location);

    return skip;
}

void ObjectLifetimes::PreCallRecordDestroySampler(VkDevice device, VkSampler sampler, const VkAllocationCallbacks* pAllocator) {
    RecordDestroyObject(sampler, kVulkanObjectTypeSampler);
}

bool ObjectLifetimes::PreCallValidateDestroyDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout descriptorSetLayout,
                                                                const VkAllocationCallbacks* pAllocator,
                                                                const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkDestroyDescriptorSetLayout-device-parameter"
    skip |= ValidateObject(descriptorSetLayout, kVulkanObjectTypeDescriptorSetLayout, true,
                           "VUID-vkDestroyDescriptorSetLayout-descriptorSetLayout-parameter",
                           "VUID-vkDestroyDescriptorSetLayout-descriptorSetLayout-parent",
                           error_obj.location.dot(Field::descriptorSetLayout));
    skip |= ValidateDestroyObject(descriptorSetLayout, kVulkanObjectTypeDescriptorSetLayout, pAllocator,
                                  "VUID-vkDestroyDescriptorSetLayout-descriptorSetLayout-00284",
                                  "VUID-vkDestroyDescriptorSetLayout-descriptorSetLayout-00285", error_obj.location);

    return skip;
}

void ObjectLifetimes::PreCallRecordDestroyDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout descriptorSetLayout,
                                                              const VkAllocationCallbacks* pAllocator) {
    RecordDestroyObject(descriptorSetLayout, kVulkanObjectTypeDescriptorSetLayout);
}

// vkCreateDescriptorPool:
// Checked by chassis: device: "VUID-vkCreateDescriptorPool-device-parameter"

void ObjectLifetimes::PostCallRecordCreateDescriptorPool(VkDevice device, const VkDescriptorPoolCreateInfo* pCreateInfo,
                                                         const VkAllocationCallbacks* pAllocator, VkDescriptorPool* pDescriptorPool,
                                                         const RecordObject& record_obj) {
    if (record_obj.result != VK_SUCCESS) return;
    CreateObject(*pDescriptorPool, kVulkanObjectTypeDescriptorPool, pAllocator);
}

bool ObjectLifetimes::PreCallValidateDestroyFramebuffer(VkDevice device, VkFramebuffer framebuffer,
                                                        const VkAllocationCallbacks* pAllocator,
                                                        const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkDestroyFramebuffer-device-parameter"
    skip |= ValidateObject(framebuffer, kVulkanObjectTypeFramebuffer, true, "VUID-vkDestroyFramebuffer-framebuffer-parameter",
                           "VUID-vkDestroyFramebuffer-framebuffer-parent", error_obj.location.dot(Field::framebuffer));
    skip |=
        ValidateDestroyObject(framebuffer, kVulkanObjectTypeFramebuffer, pAllocator, "VUID-vkDestroyFramebuffer-framebuffer-00893",
                              "VUID-vkDestroyFramebuffer-framebuffer-00894", error_obj.location);

    return skip;
}

void ObjectLifetimes::PreCallRecordDestroyFramebuffer(VkDevice device, VkFramebuffer framebuffer,
                                                      const VkAllocationCallbacks* pAllocator) {
    RecordDestroyObject(framebuffer, kVulkanObjectTypeFramebuffer);
}

// vkCreateRenderPass:
// Checked by chassis: device: "VUID-vkCreateRenderPass-device-parameter"

void ObjectLifetimes::PostCallRecordCreateRenderPass(VkDevice device, const VkRenderPassCreateInfo* pCreateInfo,
                                                     const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass,
                                                     const RecordObject& record_obj) {
    if (record_obj.result != VK_SUCCESS) return;
    CreateObject(*pRenderPass, kVulkanObjectTypeRenderPass, pAllocator);
}

bool ObjectLifetimes::PreCallValidateDestroyRenderPass(VkDevice device, VkRenderPass renderPass,
                                                       const VkAllocationCallbacks* pAllocator,
                                                       const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkDestroyRenderPass-device-parameter"
    skip |= ValidateObject(renderPass, kVulkanObjectTypeRenderPass, true, "VUID-vkDestroyRenderPass-renderPass-parameter",
                           "VUID-vkDestroyRenderPass-renderPass-parent", error_obj.location.dot(Field::renderPass));
    skip |= ValidateDestroyObject(renderPass, kVulkanObjectTypeRenderPass, pAllocator, "VUID-vkDestroyRenderPass-renderPass-00874",
                                  "VUID-vkDestroyRenderPass-renderPass-00875", error_obj.location);

    return skip;
}

void ObjectLifetimes::PreCallRecordDestroyRenderPass(VkDevice device, VkRenderPass renderPass,
                                                     const VkAllocationCallbacks* pAllocator) {
    RecordDestroyObject(renderPass, kVulkanObjectTypeRenderPass);
}

bool ObjectLifetimes::PreCallValidateGetRenderAreaGranularity(VkDevice device, VkRenderPass renderPass, VkExtent2D* pGranularity,
                                                              const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkGetRenderAreaGranularity-device-parameter"
    skip |= ValidateObject(renderPass, kVulkanObjectTypeRenderPass, false, "VUID-vkGetRenderAreaGranularity-renderPass-parameter",
                           "VUID-vkGetRenderAreaGranularity-renderPass-parent", error_obj.location.dot(Field::renderPass));

    return skip;
}

// vkCreateCommandPool:
// Checked by chassis: device: "VUID-vkCreateCommandPool-device-parameter"

void ObjectLifetimes::PostCallRecordCreateCommandPool(VkDevice device, const VkCommandPoolCreateInfo* pCreateInfo,
                                                      const VkAllocationCallbacks* pAllocator, VkCommandPool* pCommandPool,
                                                      const RecordObject& record_obj) {
    if (record_obj.result != VK_SUCCESS) return;
    CreateObject(*pCommandPool, kVulkanObjectTypeCommandPool, pAllocator);
}

bool ObjectLifetimes::PreCallValidateResetCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolResetFlags flags,
                                                      const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkResetCommandPool-device-parameter"
    skip |= ValidateObject(commandPool, kVulkanObjectTypeCommandPool, false, "VUID-vkResetCommandPool-commandPool-parameter",
                           "VUID-vkResetCommandPool-commandPool-parent", error_obj.location.dot(Field::commandPool));

    return skip;
}

// vkEndCommandBuffer:
// Checked by chassis: commandBuffer: "VUID-vkEndCommandBuffer-commandBuffer-parameter"

// vkResetCommandBuffer:
// Checked by chassis: commandBuffer: "VUID-vkResetCommandBuffer-commandBuffer-parameter"

bool ObjectLifetimes::PreCallValidateCmdBindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                     VkPipeline pipeline, const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: commandBuffer: "VUID-vkCmdBindPipeline-commandBuffer-parameter"
    // Checked by chassis: commandBuffer: "VUID-vkCmdBindPipeline-commonparent"
    skip |= ValidateObject(pipeline, kVulkanObjectTypePipeline, false, "VUID-vkCmdBindPipeline-pipeline-parameter",
                           "VUID-vkCmdBindPipeline-commonparent", error_obj.location.dot(Field::pipeline));

    return skip;
}

// vkCmdSetViewport:
// Checked by chassis: commandBuffer: "VUID-vkCmdSetViewport-commandBuffer-parameter"

// vkCmdSetScissor:
// Checked by chassis: commandBuffer: "VUID-vkCmdSetScissor-commandBuffer-parameter"

// vkCmdSetLineWidth:
// Checked by chassis: commandBuffer: "VUID-vkCmdSetLineWidth-commandBuffer-parameter"

// vkCmdSetDepthBias:
// Checked by chassis: commandBuffer: "VUID-vkCmdSetDepthBias-commandBuffer-parameter"

// vkCmdSetBlendConstants:
// Checked by chassis: commandBuffer: "VUID-vkCmdSetBlendConstants-commandBuffer-parameter"

// vkCmdSetDepthBounds:
// Checked by chassis: commandBuffer: "VUID-vkCmdSetDepthBounds-commandBuffer-parameter"

// vkCmdSetStencilCompareMask:
// Checked by chassis: commandBuffer: "VUID-vkCmdSetStencilCompareMask-commandBuffer-parameter"

// vkCmdSetStencilWriteMask:
// Checked by chassis: commandBuffer: "VUID-vkCmdSetStencilWriteMask-commandBuffer-parameter"

// vkCmdSetStencilReference:
// Checked by chassis: commandBuffer: "VUID-vkCmdSetStencilReference-commandBuffer-parameter"

bool ObjectLifetimes::PreCallValidateCmdBindDescriptorSets(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                           VkPipelineLayout layout, uint32_t firstSet, uint32_t descriptorSetCount,
                                                           const VkDescriptorSet* pDescriptorSets, uint32_t dynamicOffsetCount,
                                                           const uint32_t* pDynamicOffsets, const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: commandBuffer: "VUID-vkCmdBindDescriptorSets-commandBuffer-parameter"
    // Checked by chassis: commandBuffer: "VUID-vkCmdBindDescriptorSets-commonparent"
    skip |= ValidateObject(layout, kVulkanObjectTypePipelineLayout, false, "VUID-vkCmdBindDescriptorSets-layout-parameter",
                           "VUID-vkCmdBindDescriptorSets-commonparent", error_obj.location.dot(Field::layout));

    if ((descriptorSetCount > 0) && (pDescriptorSets)) {
        for (uint32_t index0 = 0; index0 < descriptorSetCount; ++index0) {
            skip |=
                ValidateObject(pDescriptorSets[index0], kVulkanObjectTypeDescriptorSet, true,
                               "VUID-vkCmdBindDescriptorSets-pDescriptorSets-parameter",
                               "VUID-vkCmdBindDescriptorSets-commonparent", error_obj.location.dot(Field::pDescriptorSets, index0));
        }
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdBindIndexBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                        VkIndexType indexType, const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: commandBuffer: "VUID-vkCmdBindIndexBuffer-commandBuffer-parameter"
    // Checked by chassis: commandBuffer: "VUID-vkCmdBindIndexBuffer-commonparent"
    skip |= ValidateObject(buffer, kVulkanObjectTypeBuffer, false, "VUID-vkCmdBindIndexBuffer-buffer-parameter",
                           "VUID-vkCmdBindIndexBuffer-commonparent", error_obj.location.dot(Field::buffer));

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdBindVertexBuffers(VkCommandBuffer commandBuffer, uint32_t firstBinding,
                                                          uint32_t bindingCount, const VkBuffer* pBuffers,
                                                          const VkDeviceSize* pOffsets, const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: commandBuffer: "VUID-vkCmdBindVertexBuffers-commandBuffer-parameter"
    // Checked by chassis: commandBuffer: "VUID-vkCmdBindVertexBuffers-commonparent"

    if ((bindingCount > 0) && (pBuffers)) {
        for (uint32_t index0 = 0; index0 < bindingCount; ++index0) {
            skip |=
                ValidateObject(pBuffers[index0], kVulkanObjectTypeBuffer, true, "VUID-vkCmdBindVertexBuffers-pBuffers-parameter",
                               "VUID-vkCmdBindVertexBuffers-commonparent", error_obj.location.dot(Field::pBuffers, index0));
        }
    }

    return skip;
}

// vkCmdDraw:
// Checked by chassis: commandBuffer: "VUID-vkCmdDraw-commandBuffer-parameter"

// vkCmdDrawIndexed:
// Checked by chassis: commandBuffer: "VUID-vkCmdDrawIndexed-commandBuffer-parameter"

bool ObjectLifetimes::PreCallValidateCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                     uint32_t drawCount, uint32_t stride, const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: commandBuffer: "VUID-vkCmdDrawIndirect-commandBuffer-parameter"
    // Checked by chassis: commandBuffer: "VUID-vkCmdDrawIndirect-commonparent"
    skip |= ValidateObject(buffer, kVulkanObjectTypeBuffer, false, "VUID-vkCmdDrawIndirect-buffer-parameter",
                           "VUID-vkCmdDrawIndirect-commonparent", error_obj.location.dot(Field::buffer));

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                            uint32_t drawCount, uint32_t stride,
                                                            const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: commandBuffer: "VUID-vkCmdDrawIndexedIndirect-commandBuffer-parameter"
    // Checked by chassis: commandBuffer: "VUID-vkCmdDrawIndexedIndirect-commonparent"
    skip |= ValidateObject(buffer, kVulkanObjectTypeBuffer, false, "VUID-vkCmdDrawIndexedIndirect-buffer-parameter",
                           "VUID-vkCmdDrawIndexedIndirect-commonparent", error_obj.location.dot(Field::buffer));

    return skip;
}

// vkCmdDispatch:
// Checked by chassis: commandBuffer: "VUID-vkCmdDispatch-commandBuffer-parameter"

bool ObjectLifetimes::PreCallValidateCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                         const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: commandBuffer: "VUID-vkCmdDispatchIndirect-commandBuffer-parameter"
    // Checked by chassis: commandBuffer: "VUID-vkCmdDispatchIndirect-commonparent"
    skip |= ValidateObject(buffer, kVulkanObjectTypeBuffer, false, "VUID-vkCmdDispatchIndirect-buffer-parameter",
                           "VUID-vkCmdDispatchIndirect-commonparent", error_obj.location.dot(Field::buffer));

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer,
                                                   uint32_t regionCount, const VkBufferCopy* pRegions,
                                                   const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: commandBuffer: "VUID-vkCmdCopyBuffer-commandBuffer-parameter"
    // Checked by chassis: commandBuffer: "VUID-vkCmdCopyBuffer-commonparent"
    skip |= ValidateObject(srcBuffer, kVulkanObjectTypeBuffer, false, "VUID-vkCmdCopyBuffer-srcBuffer-parameter",
                           "VUID-vkCmdCopyBuffer-commonparent", error_obj.location.dot(Field::srcBuffer));
    skip |= ValidateObject(dstBuffer, kVulkanObjectTypeBuffer, false, "VUID-vkCmdCopyBuffer-dstBuffer-parameter",
                           "VUID-vkCmdCopyBuffer-commonparent", error_obj.location.dot(Field::dstBuffer));

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                                  VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                                  const VkImageCopy* pRegions, const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: commandBuffer: "VUID-vkCmdCopyImage-commandBuffer-parameter"
    // Checked by chassis: commandBuffer: "VUID-vkCmdCopyImage-commonparent"
    skip |= ValidateObject(srcImage, kVulkanObjectTypeImage, false, "VUID-vkCmdCopyImage-srcImage-parameter",
                           "VUID-vkCmdCopyImage-commonparent", error_obj.location.dot(Field::srcImage));
    skip |= ValidateObject(dstImage, kVulkanObjectTypeImage, false, "VUID-vkCmdCopyImage-dstImage-parameter",
                           "VUID-vkCmdCopyImage-commonparent", error_obj.location.dot(Field::dstImage));

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                                  VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                                  const VkImageBlit* pRegions, VkFilter filter,
                                                  const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: commandBuffer: "VUID-vkCmdBlitImage-commandBuffer-parameter"
    // Checked by chassis: commandBuffer: "VUID-vkCmdBlitImage-commonparent"
    skip |= ValidateObject(srcImage, kVulkanObjectTypeImage, false, "VUID-vkCmdBlitImage-srcImage-parameter",
                           "VUID-vkCmdBlitImage-commonparent", error_obj.location.dot(Field::srcImage));
    skip |= ValidateObject(dstImage, kVulkanObjectTypeImage, false, "VUID-vkCmdBlitImage-dstImage-parameter",
                           "VUID-vkCmdBlitImage-commonparent", error_obj.location.dot(Field::dstImage));

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage,
                                                          VkImageLayout dstImageLayout, uint32_t regionCount,
                                                          const VkBufferImageCopy* pRegions, const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: commandBuffer: "VUID-vkCmdCopyBufferToImage-commandBuffer-parameter"
    // Checked by chassis: commandBuffer: "VUID-vkCmdCopyBufferToImage-commonparent"
    skip |= ValidateObject(srcBuffer, kVulkanObjectTypeBuffer, false, "VUID-vkCmdCopyBufferToImage-srcBuffer-parameter",
                           "VUID-vkCmdCopyBufferToImage-commonparent", error_obj.location.dot(Field::srcBuffer));
    skip |= ValidateObject(dstImage, kVulkanObjectTypeImage, false, "VUID-vkCmdCopyBufferToImage-dstImage-parameter",
                           "VUID-vkCmdCopyBufferToImage-commonparent", error_obj.location.dot(Field::dstImage));

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage,
                                                          VkImageLayout srcImageLayout, VkBuffer dstBuffer, uint32_t regionCount,
                                                          const VkBufferImageCopy* pRegions, const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: commandBuffer: "VUID-vkCmdCopyImageToBuffer-commandBuffer-parameter"
    // Checked by chassis: commandBuffer: "VUID-vkCmdCopyImageToBuffer-commonparent"
    skip |= ValidateObject(srcImage, kVulkanObjectTypeImage, false, "VUID-vkCmdCopyImageToBuffer-srcImage-parameter",
                           "VUID-vkCmdCopyImageToBuffer-commonparent", error_obj.location.dot(Field::srcImage));
    skip |= ValidateObject(dstBuffer, kVulkanObjectTypeBuffer, false, "VUID-vkCmdCopyImageToBuffer-dstBuffer-parameter",
                           "VUID-vkCmdCopyImageToBuffer-commonparent", error_obj.location.dot(Field::dstBuffer));

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                                     VkDeviceSize dataSize, const void* pData, const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: commandBuffer: "VUID-vkCmdUpdateBuffer-commandBuffer-parameter"
    // Checked by chassis: commandBuffer: "VUID-vkCmdUpdateBuffer-commonparent"
    skip |= ValidateObject(dstBuffer, kVulkanObjectTypeBuffer, false, "VUID-vkCmdUpdateBuffer-dstBuffer-parameter",
                           "VUID-vkCmdUpdateBuffer-commonparent", error_obj.location.dot(Field::dstBuffer));

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                                   VkDeviceSize size, uint32_t data, const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: commandBuffer: "VUID-vkCmdFillBuffer-commandBuffer-parameter"
    // Checked by chassis: commandBuffer: "VUID-vkCmdFillBuffer-commonparent"
    skip |= ValidateObject(dstBuffer, kVulkanObjectTypeBuffer, false, "VUID-vkCmdFillBuffer-dstBuffer-parameter",
                           "VUID-vkCmdFillBuffer-commonparent", error_obj.location.dot(Field::dstBuffer));

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                                        const VkClearColorValue* pColor, uint32_t rangeCount,
                                                        const VkImageSubresourceRange* pRanges,
                                                        const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: commandBuffer: "VUID-vkCmdClearColorImage-commandBuffer-parameter"
    // Checked by chassis: commandBuffer: "VUID-vkCmdClearColorImage-commonparent"
    skip |= ValidateObject(image, kVulkanObjectTypeImage, false, "VUID-vkCmdClearColorImage-image-parameter",
                           "VUID-vkCmdClearColorImage-commonparent", error_obj.location.dot(Field::image));

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image,
                                                               VkImageLayout imageLayout,
                                                               const VkClearDepthStencilValue* pDepthStencil, uint32_t rangeCount,
                                                               const VkImageSubresourceRange* pRanges,
                                                               const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: commandBuffer: "VUID-vkCmdClearDepthStencilImage-commandBuffer-parameter"
    // Checked by chassis: commandBuffer: "VUID-vkCmdClearDepthStencilImage-commonparent"
    skip |= ValidateObject(image, kVulkanObjectTypeImage, false, "VUID-vkCmdClearDepthStencilImage-image-parameter",
                           "VUID-vkCmdClearDepthStencilImage-commonparent", error_obj.location.dot(Field::image));

    return skip;
}

// vkCmdClearAttachments:
// Checked by chassis: commandBuffer: "VUID-vkCmdClearAttachments-commandBuffer-parameter"

bool ObjectLifetimes::PreCallValidateCmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                                     VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                                     const VkImageResolve* pRegions, const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: commandBuffer: "VUID-vkCmdResolveImage-commandBuffer-parameter"
    // Checked by chassis: commandBuffer: "VUID-vkCmdResolveImage-commonparent"
    skip |= ValidateObject(srcImage, kVulkanObjectTypeImage, false, "VUID-vkCmdResolveImage-srcImage-parameter",
                           "VUID-vkCmdResolveImage-commonparent", error_obj.location.dot(Field::srcImage));
    skip |= ValidateObject(dstImage, kVulkanObjectTypeImage, false, "VUID-vkCmdResolveImage-dstImage-parameter",
                           "VUID-vkCmdResolveImage-commonparent", error_obj.location.dot(Field::dstImage));

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask,
                                                 const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: commandBuffer: "VUID-vkCmdSetEvent-commandBuffer-parameter"
    // Checked by chassis: commandBuffer: "VUID-vkCmdSetEvent-commonparent"
    skip |= ValidateObject(event, kVulkanObjectTypeEvent, false, "VUID-vkCmdSetEvent-event-parameter",
                           "VUID-vkCmdSetEvent-commonparent", error_obj.location.dot(Field::event));

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask,
                                                   const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: commandBuffer: "VUID-vkCmdResetEvent-commandBuffer-parameter"
    // Checked by chassis: commandBuffer: "VUID-vkCmdResetEvent-commonparent"
    skip |= ValidateObject(event, kVulkanObjectTypeEvent, false, "VUID-vkCmdResetEvent-event-parameter",
                           "VUID-vkCmdResetEvent-commonparent", error_obj.location.dot(Field::event));

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdWaitEvents(
    VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents, VkPipelineStageFlags srcStageMask,
    VkPipelineStageFlags dstStageMask, uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers,
    uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount,
    const VkImageMemoryBarrier* pImageMemoryBarriers, const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: commandBuffer: "VUID-vkCmdWaitEvents-commandBuffer-parameter"
    // Checked by chassis: commandBuffer: "VUID-vkCmdWaitEvents-commonparent"

    if ((eventCount > 0) && (pEvents)) {
        for (uint32_t index0 = 0; index0 < eventCount; ++index0) {
            skip |= ValidateObject(pEvents[index0], kVulkanObjectTypeEvent, false, "VUID-vkCmdWaitEvents-pEvents-parameter",
                                   "VUID-vkCmdWaitEvents-commonparent", error_obj.location.dot(Field::pEvents, index0));
        }
    }
    if (pBufferMemoryBarriers) {
        for (uint32_t index0 = 0; index0 < bufferMemoryBarrierCount; ++index0) {
            [[maybe_unused]] const Location index0_loc = error_obj.location.dot(Field::pBufferMemoryBarriers, index0);
            skip |= ValidateObject(pBufferMemoryBarriers[index0].buffer, kVulkanObjectTypeBuffer, false,
                                   "VUID-VkBufferMemoryBarrier-buffer-parameter", kVUIDUndefined, index0_loc.dot(Field::buffer));
        }
    }
    if (pImageMemoryBarriers) {
        for (uint32_t index0 = 0; index0 < imageMemoryBarrierCount; ++index0) {
            [[maybe_unused]] const Location index0_loc = error_obj.location.dot(Field::pImageMemoryBarriers, index0);
            skip |= ValidateObject(pImageMemoryBarriers[index0].image, kVulkanObjectTypeImage, false,
                                   "VUID-VkImageMemoryBarrier-image-parameter", kVUIDUndefined, index0_loc.dot(Field::image));
        }
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdPipelineBarrier(
    VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask,
    VkDependencyFlags dependencyFlags, uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers,
    uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount,
    const VkImageMemoryBarrier* pImageMemoryBarriers, const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: commandBuffer: "VUID-vkCmdPipelineBarrier-commandBuffer-parameter"
    if (pBufferMemoryBarriers) {
        for (uint32_t index0 = 0; index0 < bufferMemoryBarrierCount; ++index0) {
            [[maybe_unused]] const Location index0_loc = error_obj.location.dot(Field::pBufferMemoryBarriers, index0);
            skip |= ValidateObject(pBufferMemoryBarriers[index0].buffer, kVulkanObjectTypeBuffer, false,
                                   "VUID-VkBufferMemoryBarrier-buffer-parameter", kVUIDUndefined, index0_loc.dot(Field::buffer));
        }
    }
    if (pImageMemoryBarriers) {
        for (uint32_t index0 = 0; index0 < imageMemoryBarrierCount; ++index0) {
            [[maybe_unused]] const Location index0_loc = error_obj.location.dot(Field::pImageMemoryBarriers, index0);
            skip |= ValidateObject(pImageMemoryBarriers[index0].image, kVulkanObjectTypeImage, false,
                                   "VUID-VkImageMemoryBarrier-image-parameter", kVUIDUndefined, index0_loc.dot(Field::image));
        }
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdBeginQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query,
                                                   VkQueryControlFlags flags, const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: commandBuffer: "VUID-vkCmdBeginQuery-commandBuffer-parameter"
    // Checked by chassis: commandBuffer: "VUID-vkCmdBeginQuery-commonparent"
    skip |= ValidateObject(queryPool, kVulkanObjectTypeQueryPool, false, "VUID-vkCmdBeginQuery-queryPool-parameter",
                           "VUID-vkCmdBeginQuery-commonparent", error_obj.location.dot(Field::queryPool));

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdEndQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query,
                                                 const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: commandBuffer: "VUID-vkCmdEndQuery-commandBuffer-parameter"
    // Checked by chassis: commandBuffer: "VUID-vkCmdEndQuery-commonparent"
    skip |= ValidateObject(queryPool, kVulkanObjectTypeQueryPool, false, "VUID-vkCmdEndQuery-queryPool-parameter",
                           "VUID-vkCmdEndQuery-commonparent", error_obj.location.dot(Field::queryPool));

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdResetQueryPool(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery,
                                                       uint32_t queryCount, const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: commandBuffer: "VUID-vkCmdResetQueryPool-commandBuffer-parameter"
    // Checked by chassis: commandBuffer: "VUID-vkCmdResetQueryPool-commonparent"
    skip |= ValidateObject(queryPool, kVulkanObjectTypeQueryPool, false, "VUID-vkCmdResetQueryPool-queryPool-parameter",
                           "VUID-vkCmdResetQueryPool-commonparent", error_obj.location.dot(Field::queryPool));

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdWriteTimestamp(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage,
                                                       VkQueryPool queryPool, uint32_t query, const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: commandBuffer: "VUID-vkCmdWriteTimestamp-commandBuffer-parameter"
    // Checked by chassis: commandBuffer: "VUID-vkCmdWriteTimestamp-commonparent"
    skip |= ValidateObject(queryPool, kVulkanObjectTypeQueryPool, false, "VUID-vkCmdWriteTimestamp-queryPool-parameter",
                           "VUID-vkCmdWriteTimestamp-commonparent", error_obj.location.dot(Field::queryPool));

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdCopyQueryPoolResults(VkCommandBuffer commandBuffer, VkQueryPool queryPool,
                                                             uint32_t firstQuery, uint32_t queryCount, VkBuffer dstBuffer,
                                                             VkDeviceSize dstOffset, VkDeviceSize stride, VkQueryResultFlags flags,
                                                             const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: commandBuffer: "VUID-vkCmdCopyQueryPoolResults-commandBuffer-parameter"
    // Checked by chassis: commandBuffer: "VUID-vkCmdCopyQueryPoolResults-commonparent"
    skip |= ValidateObject(queryPool, kVulkanObjectTypeQueryPool, false, "VUID-vkCmdCopyQueryPoolResults-queryPool-parameter",
                           "VUID-vkCmdCopyQueryPoolResults-commonparent", error_obj.location.dot(Field::queryPool));
    skip |= ValidateObject(dstBuffer, kVulkanObjectTypeBuffer, false, "VUID-vkCmdCopyQueryPoolResults-dstBuffer-parameter",
                           "VUID-vkCmdCopyQueryPoolResults-commonparent", error_obj.location.dot(Field::dstBuffer));

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdPushConstants(VkCommandBuffer commandBuffer, VkPipelineLayout layout,
                                                      VkShaderStageFlags stageFlags, uint32_t offset, uint32_t size,
                                                      const void* pValues, const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: commandBuffer: "VUID-vkCmdPushConstants-commandBuffer-parameter"
    // Checked by chassis: commandBuffer: "VUID-vkCmdPushConstants-commonparent"
    skip |= ValidateObject(layout, kVulkanObjectTypePipelineLayout, false, "VUID-vkCmdPushConstants-layout-parameter",
                           "VUID-vkCmdPushConstants-commonparent", error_obj.location.dot(Field::layout));

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdBeginRenderPass(VkCommandBuffer commandBuffer,
                                                        const VkRenderPassBeginInfo* pRenderPassBegin, VkSubpassContents contents,
                                                        const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: commandBuffer: "VUID-vkCmdBeginRenderPass-commandBuffer-parameter"
    if (pRenderPassBegin) {
        [[maybe_unused]] const Location pRenderPassBegin_loc = error_obj.location.dot(Field::pRenderPassBegin);
        skip |= ValidateObject(pRenderPassBegin->renderPass, kVulkanObjectTypeRenderPass, false,
                               "VUID-VkRenderPassBeginInfo-renderPass-parameter", "VUID-VkRenderPassBeginInfo-commonparent",
                               pRenderPassBegin_loc.dot(Field::renderPass));
        skip |= ValidateObject(pRenderPassBegin->framebuffer, kVulkanObjectTypeFramebuffer, false,
                               "VUID-VkRenderPassBeginInfo-framebuffer-parameter", "VUID-VkRenderPassBeginInfo-commonparent",
                               pRenderPassBegin_loc.dot(Field::framebuffer));
        if (auto pNext = vku::FindStructInPNextChain<VkRenderPassAttachmentBeginInfo>(pRenderPassBegin->pNext)) {
            const Location pNext_loc = pRenderPassBegin_loc.pNext(Struct::VkRenderPassAttachmentBeginInfo);

            if ((pNext->attachmentCount > 0) && (pNext->pAttachments)) {
                for (uint32_t index2 = 0; index2 < pNext->attachmentCount; ++index2) {
                    skip |= ValidateObject(pNext->pAttachments[index2], kVulkanObjectTypeImageView, false,
                                           "VUID-VkRenderPassAttachmentBeginInfo-pAttachments-parameter", kVUIDUndefined,
                                           pNext_loc.dot(Field::pAttachments, index2));
                }
            }
        }
    }

    return skip;
}

// vkCmdNextSubpass:
// Checked by chassis: commandBuffer: "VUID-vkCmdNextSubpass-commandBuffer-parameter"

// vkCmdEndRenderPass:
// Checked by chassis: commandBuffer: "VUID-vkCmdEndRenderPass-commandBuffer-parameter"

bool ObjectLifetimes::PreCallValidateCmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBufferCount,
                                                        const VkCommandBuffer* pCommandBuffers,
                                                        const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: commandBuffer: "VUID-vkCmdExecuteCommands-commandBuffer-parameter"
    // Checked by chassis: commandBuffer: "VUID-vkCmdExecuteCommands-commonparent"

    if ((commandBufferCount > 0) && (pCommandBuffers)) {
        for (uint32_t index0 = 0; index0 < commandBufferCount; ++index0) {
            skip |= ValidateObject(pCommandBuffers[index0], kVulkanObjectTypeCommandBuffer, false,
                                   "VUID-vkCmdExecuteCommands-pCommandBuffers-parameter", "VUID-vkCmdExecuteCommands-commonparent",
                                   error_obj.location.dot(Field::pCommandBuffers, index0));
        }
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateBindBufferMemory2(VkDevice device, uint32_t bindInfoCount,
                                                       const VkBindBufferMemoryInfo* pBindInfos,
                                                       const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkBindBufferMemory2-device-parameter"
    if (pBindInfos) {
        for (uint32_t index0 = 0; index0 < bindInfoCount; ++index0) {
            [[maybe_unused]] const Location index0_loc = error_obj.location.dot(Field::pBindInfos, index0);
            skip |= ValidateObject(pBindInfos[index0].buffer, kVulkanObjectTypeBuffer, false,
                                   "VUID-VkBindBufferMemoryInfo-buffer-parameter", "VUID-VkBindBufferMemoryInfo-commonparent",
                                   index0_loc.dot(Field::buffer));
            skip |= ValidateObject(pBindInfos[index0].memory, kVulkanObjectTypeDeviceMemory, false,
                                   "VUID-VkBindBufferMemoryInfo-memory-parameter", "VUID-VkBindBufferMemoryInfo-commonparent",
                                   index0_loc.dot(Field::memory));
        }
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateBindImageMemory2(VkDevice device, uint32_t bindInfoCount,
                                                      const VkBindImageMemoryInfo* pBindInfos, const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkBindImageMemory2-device-parameter"
    if (pBindInfos) {
        for (uint32_t index0 = 0; index0 < bindInfoCount; ++index0) {
            [[maybe_unused]] const Location index0_loc = error_obj.location.dot(Field::pBindInfos, index0);
            skip |= ValidateObject(pBindInfos[index0].image, kVulkanObjectTypeImage, false,
                                   "VUID-VkBindImageMemoryInfo-image-parameter", "VUID-VkBindImageMemoryInfo-commonparent",
                                   index0_loc.dot(Field::image));
            skip |= ValidateObject(pBindInfos[index0].memory, kVulkanObjectTypeDeviceMemory, true, kVUIDUndefined,
                                   "VUID-VkBindImageMemoryInfo-commonparent", index0_loc.dot(Field::memory));
            if (auto pNext = vku::FindStructInPNextChain<VkBindImageMemorySwapchainInfoKHR>(pBindInfos[index0].pNext)) {
                const Location pNext_loc = index0_loc.pNext(Struct::VkBindImageMemorySwapchainInfoKHR);
                skip |= ValidateObject(pNext->swapchain, kVulkanObjectTypeSwapchainKHR, false,
                                       "VUID-VkBindImageMemorySwapchainInfoKHR-swapchain-parameter", kVUIDUndefined,
                                       pNext_loc.dot(Field::swapchain));
            }
        }
    }

    return skip;
}

// vkGetDeviceGroupPeerMemoryFeatures:
// Checked by chassis: device: "VUID-vkGetDeviceGroupPeerMemoryFeatures-device-parameter"

// vkCmdSetDeviceMask:
// Checked by chassis: commandBuffer: "VUID-vkCmdSetDeviceMask-commandBuffer-parameter"

// vkCmdDispatchBase:
// Checked by chassis: commandBuffer: "VUID-vkCmdDispatchBase-commandBuffer-parameter"

// vkEnumeratePhysicalDeviceGroups:
// Checked by chassis: instance: "VUID-vkEnumeratePhysicalDeviceGroups-instance-parameter"

void ObjectLifetimes::PostCallRecordEnumeratePhysicalDeviceGroups(VkInstance instance, uint32_t* pPhysicalDeviceGroupCount,
                                                                  VkPhysicalDeviceGroupProperties* pPhysicalDeviceGroupProperties,
                                                                  const RecordObject& record_obj) {
    if (record_obj.result != VK_SUCCESS && record_obj.result != VK_INCOMPLETE) return;

    if (pPhysicalDeviceGroupProperties) {
        const RecordObject record_obj(vvl::Func::vkEnumeratePhysicalDevices, VK_SUCCESS);
        for (uint32_t device_group_index = 0; device_group_index < *pPhysicalDeviceGroupCount; device_group_index++) {
            PostCallRecordEnumeratePhysicalDevices(instance,
                                                   &pPhysicalDeviceGroupProperties[device_group_index].physicalDeviceCount,
                                                   pPhysicalDeviceGroupProperties[device_group_index].physicalDevices, record_obj);
        }
    }
}

bool ObjectLifetimes::PreCallValidateGetImageMemoryRequirements2(VkDevice device, const VkImageMemoryRequirementsInfo2* pInfo,
                                                                 VkMemoryRequirements2* pMemoryRequirements,
                                                                 const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkGetImageMemoryRequirements2-device-parameter"
    if (pInfo) {
        [[maybe_unused]] const Location pInfo_loc = error_obj.location.dot(Field::pInfo);
        skip |= ValidateObject(pInfo->image, kVulkanObjectTypeImage, false, "VUID-VkImageMemoryRequirementsInfo2-image-parameter",
                               kVUIDUndefined, pInfo_loc.dot(Field::image));
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetBufferMemoryRequirements2(VkDevice device, const VkBufferMemoryRequirementsInfo2* pInfo,
                                                                  VkMemoryRequirements2* pMemoryRequirements,
                                                                  const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkGetBufferMemoryRequirements2-device-parameter"
    if (pInfo) {
        [[maybe_unused]] const Location pInfo_loc = error_obj.location.dot(Field::pInfo);
        skip |=
            ValidateObject(pInfo->buffer, kVulkanObjectTypeBuffer, false, "VUID-VkBufferMemoryRequirementsInfo2-buffer-parameter",
                           kVUIDUndefined, pInfo_loc.dot(Field::buffer));
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetImageSparseMemoryRequirements2(VkDevice device,
                                                                       const VkImageSparseMemoryRequirementsInfo2* pInfo,
                                                                       uint32_t* pSparseMemoryRequirementCount,
                                                                       VkSparseImageMemoryRequirements2* pSparseMemoryRequirements,
                                                                       const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkGetImageSparseMemoryRequirements2-device-parameter"
    if (pInfo) {
        [[maybe_unused]] const Location pInfo_loc = error_obj.location.dot(Field::pInfo);
        skip |=
            ValidateObject(pInfo->image, kVulkanObjectTypeImage, false, "VUID-VkImageSparseMemoryRequirementsInfo2-image-parameter",
                           kVUIDUndefined, pInfo_loc.dot(Field::image));
    }

    return skip;
}

// vkGetPhysicalDeviceFeatures2:
// Checked by chassis: physicalDevice: "VUID-vkGetPhysicalDeviceFeatures2-physicalDevice-parameter"

// vkGetPhysicalDeviceProperties2:
// Checked by chassis: physicalDevice: "VUID-vkGetPhysicalDeviceProperties2-physicalDevice-parameter"

// vkGetPhysicalDeviceFormatProperties2:
// Checked by chassis: physicalDevice: "VUID-vkGetPhysicalDeviceFormatProperties2-physicalDevice-parameter"

// vkGetPhysicalDeviceImageFormatProperties2:
// Checked by chassis: physicalDevice: "VUID-vkGetPhysicalDeviceImageFormatProperties2-physicalDevice-parameter"

// vkGetPhysicalDeviceMemoryProperties2:
// Checked by chassis: physicalDevice: "VUID-vkGetPhysicalDeviceMemoryProperties2-physicalDevice-parameter"

// vkGetPhysicalDeviceSparseImageFormatProperties2:
// Checked by chassis: physicalDevice: "VUID-vkGetPhysicalDeviceSparseImageFormatProperties2-physicalDevice-parameter"

bool ObjectLifetimes::PreCallValidateTrimCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolTrimFlags flags,
                                                     const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkTrimCommandPool-device-parameter"
    skip |= ValidateObject(commandPool, kVulkanObjectTypeCommandPool, false, "VUID-vkTrimCommandPool-commandPool-parameter",
                           "VUID-vkTrimCommandPool-commandPool-parent", error_obj.location.dot(Field::commandPool));

    return skip;
}

// vkCreateSamplerYcbcrConversion:
// Checked by chassis: device: "VUID-vkCreateSamplerYcbcrConversion-device-parameter"

void ObjectLifetimes::PostCallRecordCreateSamplerYcbcrConversion(VkDevice device,
                                                                 const VkSamplerYcbcrConversionCreateInfo* pCreateInfo,
                                                                 const VkAllocationCallbacks* pAllocator,
                                                                 VkSamplerYcbcrConversion* pYcbcrConversion,
                                                                 const RecordObject& record_obj) {
    if (record_obj.result != VK_SUCCESS) return;
    CreateObject(*pYcbcrConversion, kVulkanObjectTypeSamplerYcbcrConversion, pAllocator);
}

bool ObjectLifetimes::PreCallValidateDestroySamplerYcbcrConversion(VkDevice device, VkSamplerYcbcrConversion ycbcrConversion,
                                                                   const VkAllocationCallbacks* pAllocator,
                                                                   const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkDestroySamplerYcbcrConversion-device-parameter"
    skip |= ValidateObject(ycbcrConversion, kVulkanObjectTypeSamplerYcbcrConversion, true,
                           "VUID-vkDestroySamplerYcbcrConversion-ycbcrConversion-parameter",
                           "VUID-vkDestroySamplerYcbcrConversion-ycbcrConversion-parent",
                           error_obj.location.dot(Field::ycbcrConversion));
    skip |= ValidateDestroyObject(ycbcrConversion, kVulkanObjectTypeSamplerYcbcrConversion, pAllocator, kVUIDUndefined,
                                  kVUIDUndefined, error_obj.location);

    return skip;
}

void ObjectLifetimes::PreCallRecordDestroySamplerYcbcrConversion(VkDevice device, VkSamplerYcbcrConversion ycbcrConversion,
                                                                 const VkAllocationCallbacks* pAllocator) {
    RecordDestroyObject(ycbcrConversion, kVulkanObjectTypeSamplerYcbcrConversion);
}

bool ObjectLifetimes::PreCallValidateDestroyDescriptorUpdateTemplate(VkDevice device,
                                                                     VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                                     const VkAllocationCallbacks* pAllocator,
                                                                     const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkDestroyDescriptorUpdateTemplate-device-parameter"
    skip |= ValidateObject(descriptorUpdateTemplate, kVulkanObjectTypeDescriptorUpdateTemplate, true,
                           "VUID-vkDestroyDescriptorUpdateTemplate-descriptorUpdateTemplate-parameter",
                           "VUID-vkDestroyDescriptorUpdateTemplate-descriptorUpdateTemplate-parent",
                           error_obj.location.dot(Field::descriptorUpdateTemplate));
    skip |= ValidateDestroyObject(descriptorUpdateTemplate, kVulkanObjectTypeDescriptorUpdateTemplate, pAllocator,
                                  "VUID-vkDestroyDescriptorUpdateTemplate-descriptorSetLayout-00356",
                                  "VUID-vkDestroyDescriptorUpdateTemplate-descriptorSetLayout-00357", error_obj.location);

    return skip;
}

void ObjectLifetimes::PreCallRecordDestroyDescriptorUpdateTemplate(VkDevice device,
                                                                   VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                                   const VkAllocationCallbacks* pAllocator) {
    RecordDestroyObject(descriptorUpdateTemplate, kVulkanObjectTypeDescriptorUpdateTemplate);
}

bool ObjectLifetimes::PreCallValidateUpdateDescriptorSetWithTemplate(VkDevice device, VkDescriptorSet descriptorSet,
                                                                     VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                                     const void* pData, const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkUpdateDescriptorSetWithTemplate-device-parameter"
    skip |= ValidateObject(
        descriptorSet, kVulkanObjectTypeDescriptorSet, false, "VUID-vkUpdateDescriptorSetWithTemplate-descriptorSet-parameter",
        "VUID-vkUpdateDescriptorSetWithTemplate-descriptorSet-parent", error_obj.location.dot(Field::descriptorSet));
    skip |= ValidateObject(descriptorUpdateTemplate, kVulkanObjectTypeDescriptorUpdateTemplate, false,
                           "VUID-vkUpdateDescriptorSetWithTemplate-descriptorUpdateTemplate-parameter",
                           "VUID-vkUpdateDescriptorSetWithTemplate-descriptorUpdateTemplate-parent",
                           error_obj.location.dot(Field::descriptorUpdateTemplate));

    return skip;
}

// vkGetPhysicalDeviceExternalBufferProperties:
// Checked by chassis: physicalDevice: "VUID-vkGetPhysicalDeviceExternalBufferProperties-physicalDevice-parameter"

// vkGetPhysicalDeviceExternalFenceProperties:
// Checked by chassis: physicalDevice: "VUID-vkGetPhysicalDeviceExternalFenceProperties-physicalDevice-parameter"

// vkGetPhysicalDeviceExternalSemaphoreProperties:
// Checked by chassis: physicalDevice: "VUID-vkGetPhysicalDeviceExternalSemaphoreProperties-physicalDevice-parameter"

bool ObjectLifetimes::PreCallValidateCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                          VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                          uint32_t maxDrawCount, uint32_t stride,
                                                          const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: commandBuffer: "VUID-vkCmdDrawIndirectCount-commandBuffer-parameter"
    // Checked by chassis: commandBuffer: "VUID-vkCmdDrawIndirectCount-commonparent"
    skip |= ValidateObject(buffer, kVulkanObjectTypeBuffer, false, "VUID-vkCmdDrawIndirectCount-buffer-parameter",
                           "VUID-vkCmdDrawIndirectCount-commonparent", error_obj.location.dot(Field::buffer));
    skip |= ValidateObject(countBuffer, kVulkanObjectTypeBuffer, false, "VUID-vkCmdDrawIndirectCount-countBuffer-parameter",
                           "VUID-vkCmdDrawIndirectCount-commonparent", error_obj.location.dot(Field::countBuffer));

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                 VkDeviceSize offset, VkBuffer countBuffer,
                                                                 VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                                 uint32_t stride, const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: commandBuffer: "VUID-vkCmdDrawIndexedIndirectCount-commandBuffer-parameter"
    // Checked by chassis: commandBuffer: "VUID-vkCmdDrawIndexedIndirectCount-commonparent"
    skip |= ValidateObject(buffer, kVulkanObjectTypeBuffer, false, "VUID-vkCmdDrawIndexedIndirectCount-buffer-parameter",
                           "VUID-vkCmdDrawIndexedIndirectCount-commonparent", error_obj.location.dot(Field::buffer));
    skip |= ValidateObject(countBuffer, kVulkanObjectTypeBuffer, false, "VUID-vkCmdDrawIndexedIndirectCount-countBuffer-parameter",
                           "VUID-vkCmdDrawIndexedIndirectCount-commonparent", error_obj.location.dot(Field::countBuffer));

    return skip;
}

// vkCreateRenderPass2:
// Checked by chassis: device: "VUID-vkCreateRenderPass2-device-parameter"

void ObjectLifetimes::PostCallRecordCreateRenderPass2(VkDevice device, const VkRenderPassCreateInfo2* pCreateInfo,
                                                      const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass,
                                                      const RecordObject& record_obj) {
    if (record_obj.result != VK_SUCCESS) return;
    CreateObject(*pRenderPass, kVulkanObjectTypeRenderPass, pAllocator);
}

bool ObjectLifetimes::PreCallValidateCmdBeginRenderPass2(VkCommandBuffer commandBuffer,
                                                         const VkRenderPassBeginInfo* pRenderPassBegin,
                                                         const VkSubpassBeginInfo* pSubpassBeginInfo,
                                                         const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: commandBuffer: "VUID-vkCmdBeginRenderPass2-commandBuffer-parameter"
    if (pRenderPassBegin) {
        [[maybe_unused]] const Location pRenderPassBegin_loc = error_obj.location.dot(Field::pRenderPassBegin);
        skip |= ValidateObject(pRenderPassBegin->renderPass, kVulkanObjectTypeRenderPass, false,
                               "VUID-VkRenderPassBeginInfo-renderPass-parameter", "VUID-VkRenderPassBeginInfo-commonparent",
                               pRenderPassBegin_loc.dot(Field::renderPass));
        skip |= ValidateObject(pRenderPassBegin->framebuffer, kVulkanObjectTypeFramebuffer, false,
                               "VUID-VkRenderPassBeginInfo-framebuffer-parameter", "VUID-VkRenderPassBeginInfo-commonparent",
                               pRenderPassBegin_loc.dot(Field::framebuffer));
        if (auto pNext = vku::FindStructInPNextChain<VkRenderPassAttachmentBeginInfo>(pRenderPassBegin->pNext)) {
            const Location pNext_loc = pRenderPassBegin_loc.pNext(Struct::VkRenderPassAttachmentBeginInfo);

            if ((pNext->attachmentCount > 0) && (pNext->pAttachments)) {
                for (uint32_t index2 = 0; index2 < pNext->attachmentCount; ++index2) {
                    skip |= ValidateObject(pNext->pAttachments[index2], kVulkanObjectTypeImageView, false,
                                           "VUID-VkRenderPassAttachmentBeginInfo-pAttachments-parameter", kVUIDUndefined,
                                           pNext_loc.dot(Field::pAttachments, index2));
                }
            }
        }
    }

    return skip;
}

// vkCmdNextSubpass2:
// Checked by chassis: commandBuffer: "VUID-vkCmdNextSubpass2-commandBuffer-parameter"

// vkCmdEndRenderPass2:
// Checked by chassis: commandBuffer: "VUID-vkCmdEndRenderPass2-commandBuffer-parameter"

bool ObjectLifetimes::PreCallValidateResetQueryPool(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery,
                                                    uint32_t queryCount, const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkResetQueryPool-device-parameter"
    skip |= ValidateObject(queryPool, kVulkanObjectTypeQueryPool, false, "VUID-vkResetQueryPool-queryPool-parameter",
                           "VUID-vkResetQueryPool-queryPool-parent", error_obj.location.dot(Field::queryPool));

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetSemaphoreCounterValue(VkDevice device, VkSemaphore semaphore, uint64_t* pValue,
                                                              const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkGetSemaphoreCounterValue-device-parameter"
    skip |= ValidateObject(semaphore, kVulkanObjectTypeSemaphore, false, "VUID-vkGetSemaphoreCounterValue-semaphore-parameter",
                           "VUID-vkGetSemaphoreCounterValue-semaphore-parent", error_obj.location.dot(Field::semaphore));

    return skip;
}

bool ObjectLifetimes::PreCallValidateWaitSemaphores(VkDevice device, const VkSemaphoreWaitInfo* pWaitInfo, uint64_t timeout,
                                                    const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkWaitSemaphores-device-parameter"
    if (pWaitInfo) {
        [[maybe_unused]] const Location pWaitInfo_loc = error_obj.location.dot(Field::pWaitInfo);

        if ((pWaitInfo->semaphoreCount > 0) && (pWaitInfo->pSemaphores)) {
            for (uint32_t index1 = 0; index1 < pWaitInfo->semaphoreCount; ++index1) {
                skip |= ValidateObject(pWaitInfo->pSemaphores[index1], kVulkanObjectTypeSemaphore, false,
                                       "VUID-VkSemaphoreWaitInfo-pSemaphores-parameter", kVUIDUndefined,
                                       pWaitInfo_loc.dot(Field::pSemaphores, index1));
            }
        }
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateSignalSemaphore(VkDevice device, const VkSemaphoreSignalInfo* pSignalInfo,
                                                     const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkSignalSemaphore-device-parameter"
    if (pSignalInfo) {
        [[maybe_unused]] const Location pSignalInfo_loc = error_obj.location.dot(Field::pSignalInfo);
        skip |=
            ValidateObject(pSignalInfo->semaphore, kVulkanObjectTypeSemaphore, false,
                           "VUID-VkSemaphoreSignalInfo-semaphore-parameter", kVUIDUndefined, pSignalInfo_loc.dot(Field::semaphore));
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetBufferDeviceAddress(VkDevice device, const VkBufferDeviceAddressInfo* pInfo,
                                                            const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkGetBufferDeviceAddress-device-parameter"
    if (pInfo) {
        [[maybe_unused]] const Location pInfo_loc = error_obj.location.dot(Field::pInfo);
        skip |= ValidateObject(pInfo->buffer, kVulkanObjectTypeBuffer, false, "VUID-VkBufferDeviceAddressInfo-buffer-parameter",
                               kVUIDUndefined, pInfo_loc.dot(Field::buffer));
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetBufferOpaqueCaptureAddress(VkDevice device, const VkBufferDeviceAddressInfo* pInfo,
                                                                   const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkGetBufferOpaqueCaptureAddress-device-parameter"
    if (pInfo) {
        [[maybe_unused]] const Location pInfo_loc = error_obj.location.dot(Field::pInfo);
        skip |= ValidateObject(pInfo->buffer, kVulkanObjectTypeBuffer, false, "VUID-VkBufferDeviceAddressInfo-buffer-parameter",
                               kVUIDUndefined, pInfo_loc.dot(Field::buffer));
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetDeviceMemoryOpaqueCaptureAddress(VkDevice device,
                                                                         const VkDeviceMemoryOpaqueCaptureAddressInfo* pInfo,
                                                                         const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkGetDeviceMemoryOpaqueCaptureAddress-device-parameter"
    if (pInfo) {
        [[maybe_unused]] const Location pInfo_loc = error_obj.location.dot(Field::pInfo);
        skip |= ValidateObject(pInfo->memory, kVulkanObjectTypeDeviceMemory, false,
                               "VUID-VkDeviceMemoryOpaqueCaptureAddressInfo-memory-parameter", kVUIDUndefined,
                               pInfo_loc.dot(Field::memory));
    }

    return skip;
}

// vkGetPhysicalDeviceToolProperties:
// Checked by chassis: physicalDevice: "VUID-vkGetPhysicalDeviceToolProperties-physicalDevice-parameter"

// vkCreatePrivateDataSlot:
// Checked by chassis: device: "VUID-vkCreatePrivateDataSlot-device-parameter"

void ObjectLifetimes::PostCallRecordCreatePrivateDataSlot(VkDevice device, const VkPrivateDataSlotCreateInfo* pCreateInfo,
                                                          const VkAllocationCallbacks* pAllocator,
                                                          VkPrivateDataSlot* pPrivateDataSlot, const RecordObject& record_obj) {
    if (record_obj.result != VK_SUCCESS) return;
    CreateObject(*pPrivateDataSlot, kVulkanObjectTypePrivateDataSlot, pAllocator);
}

bool ObjectLifetimes::PreCallValidateDestroyPrivateDataSlot(VkDevice device, VkPrivateDataSlot privateDataSlot,
                                                            const VkAllocationCallbacks* pAllocator,
                                                            const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkDestroyPrivateDataSlot-device-parameter"
    skip |= ValidateObject(privateDataSlot, kVulkanObjectTypePrivateDataSlot, true,
                           "VUID-vkDestroyPrivateDataSlot-privateDataSlot-parameter",
                           "VUID-vkDestroyPrivateDataSlot-privateDataSlot-parent", error_obj.location.dot(Field::privateDataSlot));
    skip |= ValidateDestroyObject(privateDataSlot, kVulkanObjectTypePrivateDataSlot, pAllocator, kVUIDUndefined, kVUIDUndefined,
                                  error_obj.location);

    return skip;
}

void ObjectLifetimes::PreCallRecordDestroyPrivateDataSlot(VkDevice device, VkPrivateDataSlot privateDataSlot,
                                                          const VkAllocationCallbacks* pAllocator) {
    RecordDestroyObject(privateDataSlot, kVulkanObjectTypePrivateDataSlot);
}

bool ObjectLifetimes::PreCallValidateSetPrivateData(VkDevice device, VkObjectType objectType, uint64_t objectHandle,
                                                    VkPrivateDataSlot privateDataSlot, uint64_t data,
                                                    const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkSetPrivateData-device-parameter"
    skip |=
        ValidateObject(privateDataSlot, kVulkanObjectTypePrivateDataSlot, false, "VUID-vkSetPrivateData-privateDataSlot-parameter",
                       "VUID-vkSetPrivateData-privateDataSlot-parent", error_obj.location.dot(Field::privateDataSlot));

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetPrivateData(VkDevice device, VkObjectType objectType, uint64_t objectHandle,
                                                    VkPrivateDataSlot privateDataSlot, uint64_t* pData,
                                                    const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkGetPrivateData-device-parameter"
    skip |=
        ValidateObject(privateDataSlot, kVulkanObjectTypePrivateDataSlot, false, "VUID-vkGetPrivateData-privateDataSlot-parameter",
                       "VUID-vkGetPrivateData-privateDataSlot-parent", error_obj.location.dot(Field::privateDataSlot));

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdSetEvent2(VkCommandBuffer commandBuffer, VkEvent event,
                                                  const VkDependencyInfo* pDependencyInfo, const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: commandBuffer: "VUID-vkCmdSetEvent2-commandBuffer-parameter"
    // Checked by chassis: commandBuffer: "VUID-vkCmdSetEvent2-commonparent"
    skip |= ValidateObject(event, kVulkanObjectTypeEvent, false, "VUID-vkCmdSetEvent2-event-parameter",
                           "VUID-vkCmdSetEvent2-commonparent", error_obj.location.dot(Field::event));
    if (pDependencyInfo) {
        [[maybe_unused]] const Location pDependencyInfo_loc = error_obj.location.dot(Field::pDependencyInfo);
        if (pDependencyInfo->pBufferMemoryBarriers) {
            for (uint32_t index1 = 0; index1 < pDependencyInfo->bufferMemoryBarrierCount; ++index1) {
                [[maybe_unused]] const Location index1_loc = pDependencyInfo_loc.dot(Field::pBufferMemoryBarriers, index1);
                skip |=
                    ValidateObject(pDependencyInfo->pBufferMemoryBarriers[index1].buffer, kVulkanObjectTypeBuffer, false,
                                   "VUID-VkBufferMemoryBarrier2-buffer-parameter", kVUIDUndefined, index1_loc.dot(Field::buffer));
            }
        }
        if (pDependencyInfo->pImageMemoryBarriers) {
            for (uint32_t index1 = 0; index1 < pDependencyInfo->imageMemoryBarrierCount; ++index1) {
                [[maybe_unused]] const Location index1_loc = pDependencyInfo_loc.dot(Field::pImageMemoryBarriers, index1);
                skip |= ValidateObject(pDependencyInfo->pImageMemoryBarriers[index1].image, kVulkanObjectTypeImage, false,
                                       "VUID-VkImageMemoryBarrier2-image-parameter", kVUIDUndefined, index1_loc.dot(Field::image));
            }
        }
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdResetEvent2(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags2 stageMask,
                                                    const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: commandBuffer: "VUID-vkCmdResetEvent2-commandBuffer-parameter"
    // Checked by chassis: commandBuffer: "VUID-vkCmdResetEvent2-commonparent"
    skip |= ValidateObject(event, kVulkanObjectTypeEvent, false, "VUID-vkCmdResetEvent2-event-parameter",
                           "VUID-vkCmdResetEvent2-commonparent", error_obj.location.dot(Field::event));

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdWaitEvents2(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                                    const VkDependencyInfo* pDependencyInfos, const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: commandBuffer: "VUID-vkCmdWaitEvents2-commandBuffer-parameter"
    // Checked by chassis: commandBuffer: "VUID-vkCmdWaitEvents2-commonparent"

    if ((eventCount > 0) && (pEvents)) {
        for (uint32_t index0 = 0; index0 < eventCount; ++index0) {
            skip |= ValidateObject(pEvents[index0], kVulkanObjectTypeEvent, false, "VUID-vkCmdWaitEvents2-pEvents-parameter",
                                   "VUID-vkCmdWaitEvents2-commonparent", error_obj.location.dot(Field::pEvents, index0));
        }
    }
    if (pDependencyInfos) {
        for (uint32_t index0 = 0; index0 < eventCount; ++index0) {
            [[maybe_unused]] const Location index0_loc = error_obj.location.dot(Field::pDependencyInfos, index0);
            if (pDependencyInfos[index0].pBufferMemoryBarriers) {
                for (uint32_t index1 = 0; index1 < pDependencyInfos[index0].bufferMemoryBarrierCount; ++index1) {
                    [[maybe_unused]] const Location index1_loc = index0_loc.dot(Field::pBufferMemoryBarriers, index1);
                    skip |= ValidateObject(pDependencyInfos[index0].pBufferMemoryBarriers[index1].buffer, kVulkanObjectTypeBuffer,
                                           false, "VUID-VkBufferMemoryBarrier2-buffer-parameter", kVUIDUndefined,
                                           index1_loc.dot(Field::buffer));
                }
            }
            if (pDependencyInfos[index0].pImageMemoryBarriers) {
                for (uint32_t index1 = 0; index1 < pDependencyInfos[index0].imageMemoryBarrierCount; ++index1) {
                    [[maybe_unused]] const Location index1_loc = index0_loc.dot(Field::pImageMemoryBarriers, index1);
                    skip |=
                        ValidateObject(pDependencyInfos[index0].pImageMemoryBarriers[index1].image, kVulkanObjectTypeImage, false,
                                       "VUID-VkImageMemoryBarrier2-image-parameter", kVUIDUndefined, index1_loc.dot(Field::image));
                }
            }
        }
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdPipelineBarrier2(VkCommandBuffer commandBuffer, const VkDependencyInfo* pDependencyInfo,
                                                         const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: commandBuffer: "VUID-vkCmdPipelineBarrier2-commandBuffer-parameter"
    if (pDependencyInfo) {
        [[maybe_unused]] const Location pDependencyInfo_loc = error_obj.location.dot(Field::pDependencyInfo);
        if (pDependencyInfo->pBufferMemoryBarriers) {
            for (uint32_t index1 = 0; index1 < pDependencyInfo->bufferMemoryBarrierCount; ++index1) {
                [[maybe_unused]] const Location index1_loc = pDependencyInfo_loc.dot(Field::pBufferMemoryBarriers, index1);
                skip |=
                    ValidateObject(pDependencyInfo->pBufferMemoryBarriers[index1].buffer, kVulkanObjectTypeBuffer, false,
                                   "VUID-VkBufferMemoryBarrier2-buffer-parameter", kVUIDUndefined, index1_loc.dot(Field::buffer));
            }
        }
        if (pDependencyInfo->pImageMemoryBarriers) {
            for (uint32_t index1 = 0; index1 < pDependencyInfo->imageMemoryBarrierCount; ++index1) {
                [[maybe_unused]] const Location index1_loc = pDependencyInfo_loc.dot(Field::pImageMemoryBarriers, index1);
                skip |= ValidateObject(pDependencyInfo->pImageMemoryBarriers[index1].image, kVulkanObjectTypeImage, false,
                                       "VUID-VkImageMemoryBarrier2-image-parameter", kVUIDUndefined, index1_loc.dot(Field::image));
            }
        }
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdWriteTimestamp2(VkCommandBuffer commandBuffer, VkPipelineStageFlags2 stage,
                                                        VkQueryPool queryPool, uint32_t query, const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: commandBuffer: "VUID-vkCmdWriteTimestamp2-commandBuffer-parameter"
    // Checked by chassis: commandBuffer: "VUID-vkCmdWriteTimestamp2-commonparent"
    skip |= ValidateObject(queryPool, kVulkanObjectTypeQueryPool, false, "VUID-vkCmdWriteTimestamp2-queryPool-parameter",
                           "VUID-vkCmdWriteTimestamp2-commonparent", error_obj.location.dot(Field::queryPool));

    return skip;
}

bool ObjectLifetimes::PreCallValidateQueueSubmit2(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2* pSubmits, VkFence fence,
                                                  const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: queue: "VUID-vkQueueSubmit2-queue-parameter"
    // Checked by chassis: queue: "VUID-vkQueueSubmit2-commonparent"
    if (pSubmits) {
        for (uint32_t index0 = 0; index0 < submitCount; ++index0) {
            [[maybe_unused]] const Location index0_loc = error_obj.location.dot(Field::pSubmits, index0);
            if (pSubmits[index0].pWaitSemaphoreInfos) {
                for (uint32_t index1 = 0; index1 < pSubmits[index0].waitSemaphoreInfoCount; ++index1) {
                    [[maybe_unused]] const Location index1_loc = index0_loc.dot(Field::pWaitSemaphoreInfos, index1);
                    skip |= ValidateObject(pSubmits[index0].pWaitSemaphoreInfos[index1].semaphore, kVulkanObjectTypeSemaphore,
                                           false, "VUID-VkSemaphoreSubmitInfo-semaphore-parameter", kVUIDUndefined,
                                           index1_loc.dot(Field::semaphore));
                }
            }
            if (pSubmits[index0].pCommandBufferInfos) {
                for (uint32_t index1 = 0; index1 < pSubmits[index0].commandBufferInfoCount; ++index1) {
                    [[maybe_unused]] const Location index1_loc = index0_loc.dot(Field::pCommandBufferInfos, index1);
                    skip |=
                        ValidateObject(pSubmits[index0].pCommandBufferInfos[index1].commandBuffer, kVulkanObjectTypeCommandBuffer,
                                       false, "VUID-VkCommandBufferSubmitInfo-commandBuffer-parameter", kVUIDUndefined,
                                       index1_loc.dot(Field::commandBuffer));
                }
            }
            if (pSubmits[index0].pSignalSemaphoreInfos) {
                for (uint32_t index1 = 0; index1 < pSubmits[index0].signalSemaphoreInfoCount; ++index1) {
                    [[maybe_unused]] const Location index1_loc = index0_loc.dot(Field::pSignalSemaphoreInfos, index1);
                    skip |= ValidateObject(pSubmits[index0].pSignalSemaphoreInfos[index1].semaphore, kVulkanObjectTypeSemaphore,
                                           false, "VUID-VkSemaphoreSubmitInfo-semaphore-parameter", kVUIDUndefined,
                                           index1_loc.dot(Field::semaphore));
                }
            }
            if (auto pNext = vku::FindStructInPNextChain<VkFrameBoundaryEXT>(pSubmits[index0].pNext)) {
                const Location pNext_loc = index0_loc.pNext(Struct::VkFrameBoundaryEXT);

                if ((pNext->imageCount > 0) && (pNext->pImages)) {
                    for (uint32_t index2 = 0; index2 < pNext->imageCount; ++index2) {
                        skip |= ValidateObject(pNext->pImages[index2], kVulkanObjectTypeImage, false,
                                               "VUID-VkFrameBoundaryEXT-pImages-parameter", "VUID-VkFrameBoundaryEXT-commonparent",
                                               pNext_loc.dot(Field::pImages, index2));
                    }
                }

                if ((pNext->bufferCount > 0) && (pNext->pBuffers)) {
                    for (uint32_t index2 = 0; index2 < pNext->bufferCount; ++index2) {
                        skip |= ValidateObject(pNext->pBuffers[index2], kVulkanObjectTypeBuffer, false,
                                               "VUID-VkFrameBoundaryEXT-pBuffers-parameter", "VUID-VkFrameBoundaryEXT-commonparent",
                                               pNext_loc.dot(Field::pBuffers, index2));
                    }
                }
            }
#ifdef VK_USE_PLATFORM_WIN32_KHR
            if (auto pNext = vku::FindStructInPNextChain<VkWin32KeyedMutexAcquireReleaseInfoKHR>(pSubmits[index0].pNext)) {
                const Location pNext_loc = index0_loc.pNext(Struct::VkWin32KeyedMutexAcquireReleaseInfoKHR);

                if ((pNext->acquireCount > 0) && (pNext->pAcquireSyncs)) {
                    for (uint32_t index2 = 0; index2 < pNext->acquireCount; ++index2) {
                        skip |= ValidateObject(pNext->pAcquireSyncs[index2], kVulkanObjectTypeDeviceMemory, false,
                                               "VUID-VkWin32KeyedMutexAcquireReleaseInfoKHR-pAcquireSyncs-parameter",
                                               "VUID-VkWin32KeyedMutexAcquireReleaseInfoKHR-commonparent",
                                               pNext_loc.dot(Field::pAcquireSyncs, index2));
                    }
                }

                if ((pNext->releaseCount > 0) && (pNext->pReleaseSyncs)) {
                    for (uint32_t index2 = 0; index2 < pNext->releaseCount; ++index2) {
                        skip |= ValidateObject(pNext->pReleaseSyncs[index2], kVulkanObjectTypeDeviceMemory, false,
                                               "VUID-VkWin32KeyedMutexAcquireReleaseInfoKHR-pReleaseSyncs-parameter",
                                               "VUID-VkWin32KeyedMutexAcquireReleaseInfoKHR-commonparent",
                                               pNext_loc.dot(Field::pReleaseSyncs, index2));
                    }
                }
            }
#endif  // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
            if (auto pNext = vku::FindStructInPNextChain<VkWin32KeyedMutexAcquireReleaseInfoNV>(pSubmits[index0].pNext)) {
                const Location pNext_loc = index0_loc.pNext(Struct::VkWin32KeyedMutexAcquireReleaseInfoNV);

                if ((pNext->acquireCount > 0) && (pNext->pAcquireSyncs)) {
                    for (uint32_t index2 = 0; index2 < pNext->acquireCount; ++index2) {
                        skip |= ValidateObject(pNext->pAcquireSyncs[index2], kVulkanObjectTypeDeviceMemory, false,
                                               "VUID-VkWin32KeyedMutexAcquireReleaseInfoNV-pAcquireSyncs-parameter",
                                               "VUID-VkWin32KeyedMutexAcquireReleaseInfoNV-commonparent",
                                               pNext_loc.dot(Field::pAcquireSyncs, index2));
                    }
                }

                if ((pNext->releaseCount > 0) && (pNext->pReleaseSyncs)) {
                    for (uint32_t index2 = 0; index2 < pNext->releaseCount; ++index2) {
                        skip |= ValidateObject(pNext->pReleaseSyncs[index2], kVulkanObjectTypeDeviceMemory, false,
                                               "VUID-VkWin32KeyedMutexAcquireReleaseInfoNV-pReleaseSyncs-parameter",
                                               "VUID-VkWin32KeyedMutexAcquireReleaseInfoNV-commonparent",
                                               pNext_loc.dot(Field::pReleaseSyncs, index2));
                    }
                }
            }
#endif  // VK_USE_PLATFORM_WIN32_KHR
        }
    }
    skip |= ValidateObject(fence, kVulkanObjectTypeFence, true, "VUID-vkQueueSubmit2-fence-parameter",
                           "VUID-vkQueueSubmit2-commonparent", error_obj.location.dot(Field::fence));

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdCopyBuffer2(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2* pCopyBufferInfo,
                                                    const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: commandBuffer: "VUID-vkCmdCopyBuffer2-commandBuffer-parameter"
    if (pCopyBufferInfo) {
        [[maybe_unused]] const Location pCopyBufferInfo_loc = error_obj.location.dot(Field::pCopyBufferInfo);
        skip |=
            ValidateObject(pCopyBufferInfo->srcBuffer, kVulkanObjectTypeBuffer, false, "VUID-VkCopyBufferInfo2-srcBuffer-parameter",
                           "VUID-VkCopyBufferInfo2-commonparent", pCopyBufferInfo_loc.dot(Field::srcBuffer));
        skip |=
            ValidateObject(pCopyBufferInfo->dstBuffer, kVulkanObjectTypeBuffer, false, "VUID-VkCopyBufferInfo2-dstBuffer-parameter",
                           "VUID-VkCopyBufferInfo2-commonparent", pCopyBufferInfo_loc.dot(Field::dstBuffer));
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdCopyImage2(VkCommandBuffer commandBuffer, const VkCopyImageInfo2* pCopyImageInfo,
                                                   const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: commandBuffer: "VUID-vkCmdCopyImage2-commandBuffer-parameter"
    if (pCopyImageInfo) {
        [[maybe_unused]] const Location pCopyImageInfo_loc = error_obj.location.dot(Field::pCopyImageInfo);
        skip |= ValidateObject(pCopyImageInfo->srcImage, kVulkanObjectTypeImage, false, "VUID-VkCopyImageInfo2-srcImage-parameter",
                               "VUID-VkCopyImageInfo2-commonparent", pCopyImageInfo_loc.dot(Field::srcImage));
        skip |= ValidateObject(pCopyImageInfo->dstImage, kVulkanObjectTypeImage, false, "VUID-VkCopyImageInfo2-dstImage-parameter",
                               "VUID-VkCopyImageInfo2-commonparent", pCopyImageInfo_loc.dot(Field::dstImage));
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdCopyBufferToImage2(VkCommandBuffer commandBuffer,
                                                           const VkCopyBufferToImageInfo2* pCopyBufferToImageInfo,
                                                           const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: commandBuffer: "VUID-vkCmdCopyBufferToImage2-commandBuffer-parameter"
    if (pCopyBufferToImageInfo) {
        [[maybe_unused]] const Location pCopyBufferToImageInfo_loc = error_obj.location.dot(Field::pCopyBufferToImageInfo);
        skip |= ValidateObject(pCopyBufferToImageInfo->srcBuffer, kVulkanObjectTypeBuffer, false,
                               "VUID-VkCopyBufferToImageInfo2-srcBuffer-parameter", "VUID-VkCopyBufferToImageInfo2-commonparent",
                               pCopyBufferToImageInfo_loc.dot(Field::srcBuffer));
        skip |= ValidateObject(pCopyBufferToImageInfo->dstImage, kVulkanObjectTypeImage, false,
                               "VUID-VkCopyBufferToImageInfo2-dstImage-parameter", "VUID-VkCopyBufferToImageInfo2-commonparent",
                               pCopyBufferToImageInfo_loc.dot(Field::dstImage));
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdCopyImageToBuffer2(VkCommandBuffer commandBuffer,
                                                           const VkCopyImageToBufferInfo2* pCopyImageToBufferInfo,
                                                           const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: commandBuffer: "VUID-vkCmdCopyImageToBuffer2-commandBuffer-parameter"
    if (pCopyImageToBufferInfo) {
        [[maybe_unused]] const Location pCopyImageToBufferInfo_loc = error_obj.location.dot(Field::pCopyImageToBufferInfo);
        skip |= ValidateObject(pCopyImageToBufferInfo->srcImage, kVulkanObjectTypeImage, false,
                               "VUID-VkCopyImageToBufferInfo2-srcImage-parameter", "VUID-VkCopyImageToBufferInfo2-commonparent",
                               pCopyImageToBufferInfo_loc.dot(Field::srcImage));
        skip |= ValidateObject(pCopyImageToBufferInfo->dstBuffer, kVulkanObjectTypeBuffer, false,
                               "VUID-VkCopyImageToBufferInfo2-dstBuffer-parameter", "VUID-VkCopyImageToBufferInfo2-commonparent",
                               pCopyImageToBufferInfo_loc.dot(Field::dstBuffer));
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdBlitImage2(VkCommandBuffer commandBuffer, const VkBlitImageInfo2* pBlitImageInfo,
                                                   const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: commandBuffer: "VUID-vkCmdBlitImage2-commandBuffer-parameter"
    if (pBlitImageInfo) {
        [[maybe_unused]] const Location pBlitImageInfo_loc = error_obj.location.dot(Field::pBlitImageInfo);
        skip |= ValidateObject(pBlitImageInfo->srcImage, kVulkanObjectTypeImage, false, "VUID-VkBlitImageInfo2-srcImage-parameter",
                               "VUID-VkBlitImageInfo2-commonparent", pBlitImageInfo_loc.dot(Field::srcImage));
        skip |= ValidateObject(pBlitImageInfo->dstImage, kVulkanObjectTypeImage, false, "VUID-VkBlitImageInfo2-dstImage-parameter",
                               "VUID-VkBlitImageInfo2-commonparent", pBlitImageInfo_loc.dot(Field::dstImage));
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdResolveImage2(VkCommandBuffer commandBuffer, const VkResolveImageInfo2* pResolveImageInfo,
                                                      const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: commandBuffer: "VUID-vkCmdResolveImage2-commandBuffer-parameter"
    if (pResolveImageInfo) {
        [[maybe_unused]] const Location pResolveImageInfo_loc = error_obj.location.dot(Field::pResolveImageInfo);
        skip |= ValidateObject(pResolveImageInfo->srcImage, kVulkanObjectTypeImage, false,
                               "VUID-VkResolveImageInfo2-srcImage-parameter", "VUID-VkResolveImageInfo2-commonparent",
                               pResolveImageInfo_loc.dot(Field::srcImage));
        skip |= ValidateObject(pResolveImageInfo->dstImage, kVulkanObjectTypeImage, false,
                               "VUID-VkResolveImageInfo2-dstImage-parameter", "VUID-VkResolveImageInfo2-commonparent",
                               pResolveImageInfo_loc.dot(Field::dstImage));
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdBeginRendering(VkCommandBuffer commandBuffer, const VkRenderingInfo* pRenderingInfo,
                                                       const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: commandBuffer: "VUID-vkCmdBeginRendering-commandBuffer-parameter"
    if (pRenderingInfo) {
        [[maybe_unused]] const Location pRenderingInfo_loc = error_obj.location.dot(Field::pRenderingInfo);
        if (pRenderingInfo->pColorAttachments) {
            for (uint32_t index1 = 0; index1 < pRenderingInfo->colorAttachmentCount; ++index1) {
                [[maybe_unused]] const Location index1_loc = pRenderingInfo_loc.dot(Field::pColorAttachments, index1);
                skip |= ValidateObject(pRenderingInfo->pColorAttachments[index1].imageView, kVulkanObjectTypeImageView, true,
                                       "VUID-VkRenderingAttachmentInfo-imageView-parameter",
                                       "VUID-VkRenderingAttachmentInfo-commonparent", index1_loc.dot(Field::imageView));
                skip |= ValidateObject(pRenderingInfo->pColorAttachments[index1].resolveImageView, kVulkanObjectTypeImageView, true,
                                       "VUID-VkRenderingAttachmentInfo-resolveImageView-parameter",
                                       "VUID-VkRenderingAttachmentInfo-commonparent", index1_loc.dot(Field::resolveImageView));
            }
        }
        if (pRenderingInfo->pDepthAttachment) {
            [[maybe_unused]] const Location pDepthAttachment_loc = pRenderingInfo_loc.dot(Field::pDepthAttachment);
            skip |= ValidateObject(pRenderingInfo->pDepthAttachment->imageView, kVulkanObjectTypeImageView, true,
                                   "VUID-VkRenderingAttachmentInfo-imageView-parameter",
                                   "VUID-VkRenderingAttachmentInfo-commonparent", pDepthAttachment_loc.dot(Field::imageView));
            skip |=
                ValidateObject(pRenderingInfo->pDepthAttachment->resolveImageView, kVulkanObjectTypeImageView, true,
                               "VUID-VkRenderingAttachmentInfo-resolveImageView-parameter",
                               "VUID-VkRenderingAttachmentInfo-commonparent", pDepthAttachment_loc.dot(Field::resolveImageView));
        }
        if (pRenderingInfo->pStencilAttachment) {
            [[maybe_unused]] const Location pStencilAttachment_loc = pRenderingInfo_loc.dot(Field::pStencilAttachment);
            skip |= ValidateObject(pRenderingInfo->pStencilAttachment->imageView, kVulkanObjectTypeImageView, true,
                                   "VUID-VkRenderingAttachmentInfo-imageView-parameter",
                                   "VUID-VkRenderingAttachmentInfo-commonparent", pStencilAttachment_loc.dot(Field::imageView));
            skip |=
                ValidateObject(pRenderingInfo->pStencilAttachment->resolveImageView, kVulkanObjectTypeImageView, true,
                               "VUID-VkRenderingAttachmentInfo-resolveImageView-parameter",
                               "VUID-VkRenderingAttachmentInfo-commonparent", pStencilAttachment_loc.dot(Field::resolveImageView));
        }
        if (auto pNext = vku::FindStructInPNextChain<VkRenderingFragmentDensityMapAttachmentInfoEXT>(pRenderingInfo->pNext)) {
            const Location pNext_loc = pRenderingInfo_loc.pNext(Struct::VkRenderingFragmentDensityMapAttachmentInfoEXT);
            skip |= ValidateObject(pNext->imageView, kVulkanObjectTypeImageView, false,
                                   "VUID-VkRenderingFragmentDensityMapAttachmentInfoEXT-imageView-parameter", kVUIDUndefined,
                                   pNext_loc.dot(Field::imageView));
        }
        if (auto pNext = vku::FindStructInPNextChain<VkRenderingFragmentShadingRateAttachmentInfoKHR>(pRenderingInfo->pNext)) {
            const Location pNext_loc = pRenderingInfo_loc.pNext(Struct::VkRenderingFragmentShadingRateAttachmentInfoKHR);
            skip |= ValidateObject(pNext->imageView, kVulkanObjectTypeImageView, true,
                                   "VUID-VkRenderingFragmentShadingRateAttachmentInfoKHR-imageView-parameter", kVUIDUndefined,
                                   pNext_loc.dot(Field::imageView));
        }
    }

    return skip;
}

// vkCmdEndRendering:
// Checked by chassis: commandBuffer: "VUID-vkCmdEndRendering-commandBuffer-parameter"

// vkCmdSetCullMode:
// Checked by chassis: commandBuffer: "VUID-vkCmdSetCullMode-commandBuffer-parameter"

// vkCmdSetFrontFace:
// Checked by chassis: commandBuffer: "VUID-vkCmdSetFrontFace-commandBuffer-parameter"

// vkCmdSetPrimitiveTopology:
// Checked by chassis: commandBuffer: "VUID-vkCmdSetPrimitiveTopology-commandBuffer-parameter"

// vkCmdSetViewportWithCount:
// Checked by chassis: commandBuffer: "VUID-vkCmdSetViewportWithCount-commandBuffer-parameter"

// vkCmdSetScissorWithCount:
// Checked by chassis: commandBuffer: "VUID-vkCmdSetScissorWithCount-commandBuffer-parameter"

bool ObjectLifetimes::PreCallValidateCmdBindVertexBuffers2(VkCommandBuffer commandBuffer, uint32_t firstBinding,
                                                           uint32_t bindingCount, const VkBuffer* pBuffers,
                                                           const VkDeviceSize* pOffsets, const VkDeviceSize* pSizes,
                                                           const VkDeviceSize* pStrides, const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: commandBuffer: "VUID-vkCmdBindVertexBuffers2-commandBuffer-parameter"
    // Checked by chassis: commandBuffer: "VUID-vkCmdBindVertexBuffers2-commonparent"

    if ((bindingCount > 0) && (pBuffers)) {
        for (uint32_t index0 = 0; index0 < bindingCount; ++index0) {
            skip |=
                ValidateObject(pBuffers[index0], kVulkanObjectTypeBuffer, true, "VUID-vkCmdBindVertexBuffers2-pBuffers-parameter",
                               "VUID-vkCmdBindVertexBuffers2-commonparent", error_obj.location.dot(Field::pBuffers, index0));
        }
    }

    return skip;
}

// vkCmdSetDepthTestEnable:
// Checked by chassis: commandBuffer: "VUID-vkCmdSetDepthTestEnable-commandBuffer-parameter"

// vkCmdSetDepthWriteEnable:
// Checked by chassis: commandBuffer: "VUID-vkCmdSetDepthWriteEnable-commandBuffer-parameter"

// vkCmdSetDepthCompareOp:
// Checked by chassis: commandBuffer: "VUID-vkCmdSetDepthCompareOp-commandBuffer-parameter"

// vkCmdSetDepthBoundsTestEnable:
// Checked by chassis: commandBuffer: "VUID-vkCmdSetDepthBoundsTestEnable-commandBuffer-parameter"

// vkCmdSetStencilTestEnable:
// Checked by chassis: commandBuffer: "VUID-vkCmdSetStencilTestEnable-commandBuffer-parameter"

// vkCmdSetStencilOp:
// Checked by chassis: commandBuffer: "VUID-vkCmdSetStencilOp-commandBuffer-parameter"

// vkCmdSetRasterizerDiscardEnable:
// Checked by chassis: commandBuffer: "VUID-vkCmdSetRasterizerDiscardEnable-commandBuffer-parameter"

// vkCmdSetDepthBiasEnable:
// Checked by chassis: commandBuffer: "VUID-vkCmdSetDepthBiasEnable-commandBuffer-parameter"

// vkCmdSetPrimitiveRestartEnable:
// Checked by chassis: commandBuffer: "VUID-vkCmdSetPrimitiveRestartEnable-commandBuffer-parameter"

// vkGetDeviceBufferMemoryRequirements:
// Checked by chassis: device: "VUID-vkGetDeviceBufferMemoryRequirements-device-parameter"

// vkGetDeviceImageMemoryRequirements:
// Checked by chassis: device: "VUID-vkGetDeviceImageMemoryRequirements-device-parameter"

// vkGetDeviceImageSparseMemoryRequirements:
// Checked by chassis: device: "VUID-vkGetDeviceImageSparseMemoryRequirements-device-parameter"

bool ObjectLifetimes::PreCallValidateDestroySurfaceKHR(VkInstance instance, VkSurfaceKHR surface,
                                                       const VkAllocationCallbacks* pAllocator,
                                                       const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: instance: "VUID-vkDestroySurfaceKHR-instance-parameter"
    skip |= ValidateObject(surface, kVulkanObjectTypeSurfaceKHR, true, "VUID-vkDestroySurfaceKHR-surface-parameter",
                           "VUID-vkDestroySurfaceKHR-surface-parent", error_obj.location.dot(Field::surface),
                           kVulkanObjectTypeInstance);
    skip |= ValidateDestroyObject(surface, kVulkanObjectTypeSurfaceKHR, pAllocator, "VUID-vkDestroySurfaceKHR-surface-01267",
                                  "VUID-vkDestroySurfaceKHR-surface-01268", error_obj.location);

    return skip;
}

void ObjectLifetimes::PreCallRecordDestroySurfaceKHR(VkInstance instance, VkSurfaceKHR surface,
                                                     const VkAllocationCallbacks* pAllocator) {
    RecordDestroyObject(surface, kVulkanObjectTypeSurfaceKHR);
}

bool ObjectLifetimes::PreCallValidateGetPhysicalDeviceSurfaceSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
                                                                        VkSurfaceKHR surface, VkBool32* pSupported,
                                                                        const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: physicalDevice: "VUID-vkGetPhysicalDeviceSurfaceSupportKHR-physicalDevice-parameter"
    // Checked by chassis: physicalDevice: "VUID-vkGetPhysicalDeviceSurfaceSupportKHR-commonparent"
    skip |=
        ValidateObject(surface, kVulkanObjectTypeSurfaceKHR, false, "VUID-vkGetPhysicalDeviceSurfaceSupportKHR-surface-parameter",
                       "VUID-vkGetPhysicalDeviceSurfaceSupportKHR-commonparent", error_obj.location.dot(Field::surface),
                       kVulkanObjectTypeInstance);

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetPhysicalDeviceSurfaceCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                             VkSurfaceCapabilitiesKHR* pSurfaceCapabilities,
                                                                             const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: physicalDevice: "VUID-vkGetPhysicalDeviceSurfaceCapabilitiesKHR-physicalDevice-parameter"
    // Checked by chassis: physicalDevice: "VUID-vkGetPhysicalDeviceSurfaceCapabilitiesKHR-commonparent"
    skip |= ValidateObject(surface, kVulkanObjectTypeSurfaceKHR, false,
                           "VUID-vkGetPhysicalDeviceSurfaceCapabilitiesKHR-surface-parameter",
                           "VUID-vkGetPhysicalDeviceSurfaceCapabilitiesKHR-commonparent", error_obj.location.dot(Field::surface),
                           kVulkanObjectTypeInstance);

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                        uint32_t* pSurfaceFormatCount,
                                                                        VkSurfaceFormatKHR* pSurfaceFormats,
                                                                        const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: physicalDevice: "VUID-vkGetPhysicalDeviceSurfaceFormatsKHR-physicalDevice-parameter"
    // Checked by chassis: physicalDevice: "VUID-vkGetPhysicalDeviceSurfaceFormatsKHR-commonparent"
    skip |=
        ValidateObject(surface, kVulkanObjectTypeSurfaceKHR, true, "VUID-vkGetPhysicalDeviceSurfaceFormatsKHR-surface-parameter",
                       "VUID-vkGetPhysicalDeviceSurfaceFormatsKHR-commonparent", error_obj.location.dot(Field::surface),
                       kVulkanObjectTypeInstance);

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                             uint32_t* pPresentModeCount,
                                                                             VkPresentModeKHR* pPresentModes,
                                                                             const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: physicalDevice: "VUID-vkGetPhysicalDeviceSurfacePresentModesKHR-physicalDevice-parameter"
    // Checked by chassis: physicalDevice: "VUID-vkGetPhysicalDeviceSurfacePresentModesKHR-commonparent"
    skip |= ValidateObject(surface, kVulkanObjectTypeSurfaceKHR, true,
                           "VUID-vkGetPhysicalDeviceSurfacePresentModesKHR-surface-parameter",
                           "VUID-vkGetPhysicalDeviceSurfacePresentModesKHR-commonparent", error_obj.location.dot(Field::surface),
                           kVulkanObjectTypeInstance);

    return skip;
}

bool ObjectLifetimes::PreCallValidateCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR* pCreateInfo,
                                                        const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain,
                                                        const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkCreateSwapchainKHR-device-parameter"
    if (pCreateInfo) {
        [[maybe_unused]] const Location pCreateInfo_loc = error_obj.location.dot(Field::pCreateInfo);
        auto instance_data = GetLayerDataPtr(get_dispatch_key(instance), layer_data_map);
        auto instance_object_lifetimes = instance_data->GetValidationObject<ObjectLifetimes>();
        skip |= instance_object_lifetimes->ValidateObject(
            pCreateInfo->surface, kVulkanObjectTypeSurfaceKHR, false, "VUID-VkSwapchainCreateInfoKHR-surface-parameter",
            "VUID-VkSwapchainCreateInfoKHR-commonparent", pCreateInfo_loc.dot(Field::surface), kVulkanObjectTypeInstance);
        skip |= ValidateObject(pCreateInfo->oldSwapchain, kVulkanObjectTypeSwapchainKHR, true,
                               "VUID-VkSwapchainCreateInfoKHR-oldSwapchain-parameter", "VUID-VkSwapchainCreateInfoKHR-commonparent",
                               pCreateInfo_loc.dot(Field::oldSwapchain), kVulkanObjectTypeDevice);
    }

    return skip;
}

void ObjectLifetimes::PostCallRecordCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR* pCreateInfo,
                                                       const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain,
                                                       const RecordObject& record_obj) {
    if (record_obj.result != VK_SUCCESS) return;
    CreateObject(*pSwapchain, kVulkanObjectTypeSwapchainKHR, pAllocator);
}

bool ObjectLifetimes::PreCallValidateAcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout,
                                                         VkSemaphore semaphore, VkFence fence, uint32_t* pImageIndex,
                                                         const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkAcquireNextImageKHR-device-parameter"
    skip |= ValidateObject(swapchain, kVulkanObjectTypeSwapchainKHR, false, "VUID-vkAcquireNextImageKHR-swapchain-parameter",
                           "VUID-vkAcquireNextImageKHR-swapchain-parent", error_obj.location.dot(Field::swapchain));
    skip |= ValidateObject(semaphore, kVulkanObjectTypeSemaphore, true, "VUID-vkAcquireNextImageKHR-semaphore-parameter",
                           "VUID-vkAcquireNextImageKHR-semaphore-parent", error_obj.location.dot(Field::semaphore));
    skip |= ValidateObject(fence, kVulkanObjectTypeFence, true, "VUID-vkAcquireNextImageKHR-fence-parameter",
                           "VUID-vkAcquireNextImageKHR-fence-parent", error_obj.location.dot(Field::fence));

    return skip;
}

bool ObjectLifetimes::PreCallValidateQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* pPresentInfo,
                                                     const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: queue: "VUID-vkQueuePresentKHR-queue-parameter"
    if (pPresentInfo) {
        [[maybe_unused]] const Location pPresentInfo_loc = error_obj.location.dot(Field::pPresentInfo);

        if ((pPresentInfo->waitSemaphoreCount > 0) && (pPresentInfo->pWaitSemaphores)) {
            for (uint32_t index1 = 0; index1 < pPresentInfo->waitSemaphoreCount; ++index1) {
                skip |= ValidateObject(pPresentInfo->pWaitSemaphores[index1], kVulkanObjectTypeSemaphore, false,
                                       "VUID-VkPresentInfoKHR-pWaitSemaphores-parameter", "VUID-VkPresentInfoKHR-commonparent",
                                       pPresentInfo_loc.dot(Field::pWaitSemaphores, index1));
            }
        }

        if ((pPresentInfo->swapchainCount > 0) && (pPresentInfo->pSwapchains)) {
            for (uint32_t index1 = 0; index1 < pPresentInfo->swapchainCount; ++index1) {
                skip |= ValidateObject(pPresentInfo->pSwapchains[index1], kVulkanObjectTypeSwapchainKHR, false,
                                       "VUID-VkPresentInfoKHR-pSwapchains-parameter", "VUID-VkPresentInfoKHR-commonparent",
                                       pPresentInfo_loc.dot(Field::pSwapchains, index1));
            }
        }
        if (auto pNext = vku::FindStructInPNextChain<VkFrameBoundaryEXT>(pPresentInfo->pNext)) {
            const Location pNext_loc = pPresentInfo_loc.pNext(Struct::VkFrameBoundaryEXT);

            if ((pNext->imageCount > 0) && (pNext->pImages)) {
                for (uint32_t index2 = 0; index2 < pNext->imageCount; ++index2) {
                    skip |= ValidateObject(pNext->pImages[index2], kVulkanObjectTypeImage, false,
                                           "VUID-VkFrameBoundaryEXT-pImages-parameter", "VUID-VkFrameBoundaryEXT-commonparent",
                                           pNext_loc.dot(Field::pImages, index2));
                }
            }

            if ((pNext->bufferCount > 0) && (pNext->pBuffers)) {
                for (uint32_t index2 = 0; index2 < pNext->bufferCount; ++index2) {
                    skip |= ValidateObject(pNext->pBuffers[index2], kVulkanObjectTypeBuffer, false,
                                           "VUID-VkFrameBoundaryEXT-pBuffers-parameter", "VUID-VkFrameBoundaryEXT-commonparent",
                                           pNext_loc.dot(Field::pBuffers, index2));
                }
            }
        }
        if (auto pNext = vku::FindStructInPNextChain<VkSwapchainPresentFenceInfoEXT>(pPresentInfo->pNext)) {
            const Location pNext_loc = pPresentInfo_loc.pNext(Struct::VkSwapchainPresentFenceInfoEXT);

            if ((pNext->swapchainCount > 0) && (pNext->pFences)) {
                for (uint32_t index2 = 0; index2 < pNext->swapchainCount; ++index2) {
                    skip |= ValidateObject(pNext->pFences[index2], kVulkanObjectTypeFence, false,
                                           "VUID-VkSwapchainPresentFenceInfoEXT-pFences-parameter", kVUIDUndefined,
                                           pNext_loc.dot(Field::pFences, index2));
                }
            }
        }
    }

    return skip;
}

// vkGetDeviceGroupPresentCapabilitiesKHR:
// Checked by chassis: device: "VUID-vkGetDeviceGroupPresentCapabilitiesKHR-device-parameter"

bool ObjectLifetimes::PreCallValidateGetDeviceGroupSurfacePresentModesKHR(VkDevice device, VkSurfaceKHR surface,
                                                                          VkDeviceGroupPresentModeFlagsKHR* pModes,
                                                                          const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkGetDeviceGroupSurfacePresentModesKHR-device-parameter"
    // Checked by chassis: device: "VUID-vkGetDeviceGroupSurfacePresentModesKHR-commonparent"
    auto instance_data = GetLayerDataPtr(get_dispatch_key(instance), layer_data_map);
    auto instance_object_lifetimes = instance_data->GetValidationObject<ObjectLifetimes>();
    skip |= instance_object_lifetimes->ValidateObject(surface, kVulkanObjectTypeSurfaceKHR, false,
                                                      "VUID-vkGetDeviceGroupSurfacePresentModesKHR-surface-parameter",
                                                      "VUID-vkGetDeviceGroupSurfacePresentModesKHR-commonparent",
                                                      error_obj.location.dot(Field::surface), kVulkanObjectTypeInstance);

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetPhysicalDevicePresentRectanglesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                           uint32_t* pRectCount, VkRect2D* pRects,
                                                                           const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: physicalDevice: "VUID-vkGetPhysicalDevicePresentRectanglesKHR-physicalDevice-parameter"
    // Checked by chassis: physicalDevice: "VUID-vkGetPhysicalDevicePresentRectanglesKHR-commonparent"
    skip |= ValidateObject(surface, kVulkanObjectTypeSurfaceKHR, false,
                           "VUID-vkGetPhysicalDevicePresentRectanglesKHR-surface-parameter",
                           "VUID-vkGetPhysicalDevicePresentRectanglesKHR-commonparent", error_obj.location.dot(Field::surface),
                           kVulkanObjectTypeInstance);

    return skip;
}

bool ObjectLifetimes::PreCallValidateAcquireNextImage2KHR(VkDevice device, const VkAcquireNextImageInfoKHR* pAcquireInfo,
                                                          uint32_t* pImageIndex, const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkAcquireNextImage2KHR-device-parameter"
    if (pAcquireInfo) {
        [[maybe_unused]] const Location pAcquireInfo_loc = error_obj.location.dot(Field::pAcquireInfo);
        skip |= ValidateObject(pAcquireInfo->swapchain, kVulkanObjectTypeSwapchainKHR, false,
                               "VUID-VkAcquireNextImageInfoKHR-swapchain-parameter", "VUID-VkAcquireNextImageInfoKHR-commonparent",
                               pAcquireInfo_loc.dot(Field::swapchain));
        skip |= ValidateObject(pAcquireInfo->semaphore, kVulkanObjectTypeSemaphore, true,
                               "VUID-VkAcquireNextImageInfoKHR-semaphore-parameter", "VUID-VkAcquireNextImageInfoKHR-commonparent",
                               pAcquireInfo_loc.dot(Field::semaphore));
        skip |= ValidateObject(pAcquireInfo->fence, kVulkanObjectTypeFence, true, "VUID-VkAcquireNextImageInfoKHR-fence-parameter",
                               "VUID-VkAcquireNextImageInfoKHR-commonparent", pAcquireInfo_loc.dot(Field::fence));
    }

    return skip;
}

// vkGetPhysicalDeviceDisplayPlanePropertiesKHR:
// Checked by chassis: physicalDevice: "VUID-vkGetPhysicalDeviceDisplayPlanePropertiesKHR-physicalDevice-parameter"

// vkGetDisplayPlaneSupportedDisplaysKHR:
// Checked by chassis: physicalDevice: "VUID-vkGetDisplayPlaneSupportedDisplaysKHR-physicalDevice-parameter"

void ObjectLifetimes::PostCallRecordGetDisplayPlaneSupportedDisplaysKHR(VkPhysicalDevice physicalDevice, uint32_t planeIndex,
                                                                        uint32_t* pDisplayCount, VkDisplayKHR* pDisplays,
                                                                        const RecordObject& record_obj) {
    if (record_obj.result != VK_SUCCESS) return;
    if (pDisplays) {
        for (uint32_t index = 0; index < *pDisplayCount; index++) {
            CreateObject(pDisplays[index], kVulkanObjectTypeDisplayKHR, nullptr);
        }
    }
}

bool ObjectLifetimes::PreCallValidateCreateDisplayModeKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display,
                                                          const VkDisplayModeCreateInfoKHR* pCreateInfo,
                                                          const VkAllocationCallbacks* pAllocator, VkDisplayModeKHR* pMode,
                                                          const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: physicalDevice: "VUID-vkCreateDisplayModeKHR-physicalDevice-parameter"
    skip |= ValidateObject(display, kVulkanObjectTypeDisplayKHR, false, "VUID-vkCreateDisplayModeKHR-display-parameter",
                           "VUID-vkCreateDisplayModeKHR-display-parent", error_obj.location.dot(Field::display),
                           kVulkanObjectTypePhysicalDevice);

    return skip;
}

void ObjectLifetimes::PostCallRecordCreateDisplayModeKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display,
                                                         const VkDisplayModeCreateInfoKHR* pCreateInfo,
                                                         const VkAllocationCallbacks* pAllocator, VkDisplayModeKHR* pMode,
                                                         const RecordObject& record_obj) {
    if (record_obj.result != VK_SUCCESS) return;
    CreateObject(*pMode, kVulkanObjectTypeDisplayModeKHR, pAllocator);
}

bool ObjectLifetimes::PreCallValidateGetDisplayPlaneCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkDisplayModeKHR mode,
                                                                    uint32_t planeIndex,
                                                                    VkDisplayPlaneCapabilitiesKHR* pCapabilities,
                                                                    const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: physicalDevice: "VUID-vkGetDisplayPlaneCapabilitiesKHR-physicalDevice-parameter"
    skip |= ValidateObject(mode, kVulkanObjectTypeDisplayModeKHR, false, "VUID-vkGetDisplayPlaneCapabilitiesKHR-mode-parameter",
                           "VUID-vkGetDisplayPlaneCapabilitiesKHR-mode-parent", error_obj.location.dot(Field::mode),
                           kVulkanObjectTypePhysicalDevice);

    return skip;
}

bool ObjectLifetimes::PreCallValidateCreateDisplayPlaneSurfaceKHR(VkInstance instance,
                                                                  const VkDisplaySurfaceCreateInfoKHR* pCreateInfo,
                                                                  const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                                  const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: instance: "VUID-vkCreateDisplayPlaneSurfaceKHR-instance-parameter"
    if (pCreateInfo) {
        [[maybe_unused]] const Location pCreateInfo_loc = error_obj.location.dot(Field::pCreateInfo);
        skip |= ValidateObject(pCreateInfo->displayMode, kVulkanObjectTypeDisplayModeKHR, false,
                               "VUID-VkDisplaySurfaceCreateInfoKHR-displayMode-parameter", kVUIDUndefined,
                               pCreateInfo_loc.dot(Field::displayMode));
    }

    return skip;
}

void ObjectLifetimes::PostCallRecordCreateDisplayPlaneSurfaceKHR(VkInstance instance,
                                                                 const VkDisplaySurfaceCreateInfoKHR* pCreateInfo,
                                                                 const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                                 const RecordObject& record_obj) {
    if (record_obj.result != VK_SUCCESS) return;
    CreateObject(*pSurface, kVulkanObjectTypeSurfaceKHR, pAllocator);
}

bool ObjectLifetimes::PreCallValidateCreateSharedSwapchainsKHR(VkDevice device, uint32_t swapchainCount,
                                                               const VkSwapchainCreateInfoKHR* pCreateInfos,
                                                               const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchains,
                                                               const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkCreateSharedSwapchainsKHR-device-parameter"
    if (pCreateInfos) {
        for (uint32_t index0 = 0; index0 < swapchainCount; ++index0) {
            [[maybe_unused]] const Location index0_loc = error_obj.location.dot(Field::pCreateInfos, index0);
            auto instance_data = GetLayerDataPtr(get_dispatch_key(instance), layer_data_map);
            auto instance_object_lifetimes = instance_data->GetValidationObject<ObjectLifetimes>();
            skip |= instance_object_lifetimes->ValidateObject(
                pCreateInfos[index0].surface, kVulkanObjectTypeSurfaceKHR, false, "VUID-VkSwapchainCreateInfoKHR-surface-parameter",
                "VUID-VkSwapchainCreateInfoKHR-commonparent", index0_loc.dot(Field::surface), kVulkanObjectTypeInstance);
            skip |=
                ValidateObject(pCreateInfos[index0].oldSwapchain, kVulkanObjectTypeSwapchainKHR, true,
                               "VUID-VkSwapchainCreateInfoKHR-oldSwapchain-parameter", "VUID-VkSwapchainCreateInfoKHR-commonparent",
                               index0_loc.dot(Field::oldSwapchain), kVulkanObjectTypeDevice);
        }
    }

    return skip;
}

void ObjectLifetimes::PostCallRecordCreateSharedSwapchainsKHR(VkDevice device, uint32_t swapchainCount,
                                                              const VkSwapchainCreateInfoKHR* pCreateInfos,
                                                              const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchains,
                                                              const RecordObject& record_obj) {
    if (record_obj.result != VK_SUCCESS) return;
    if (pSwapchains) {
        for (uint32_t index = 0; index < swapchainCount; index++) {
            CreateObject(pSwapchains[index], kVulkanObjectTypeSwapchainKHR, pAllocator);
        }
    }
}
#ifdef VK_USE_PLATFORM_XLIB_KHR

// vkCreateXlibSurfaceKHR:
// Checked by chassis: instance: "VUID-vkCreateXlibSurfaceKHR-instance-parameter"

void ObjectLifetimes::PostCallRecordCreateXlibSurfaceKHR(VkInstance instance, const VkXlibSurfaceCreateInfoKHR* pCreateInfo,
                                                         const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                         const RecordObject& record_obj) {
    if (record_obj.result != VK_SUCCESS) return;
    CreateObject(*pSurface, kVulkanObjectTypeSurfaceKHR, pAllocator);
}
#endif  // VK_USE_PLATFORM_XLIB_KHR
#ifdef VK_USE_PLATFORM_XLIB_KHR

// vkGetPhysicalDeviceXlibPresentationSupportKHR:
// Checked by chassis: physicalDevice: "VUID-vkGetPhysicalDeviceXlibPresentationSupportKHR-physicalDevice-parameter"

#endif  // VK_USE_PLATFORM_XLIB_KHR
#ifdef VK_USE_PLATFORM_XCB_KHR

// vkCreateXcbSurfaceKHR:
// Checked by chassis: instance: "VUID-vkCreateXcbSurfaceKHR-instance-parameter"

void ObjectLifetimes::PostCallRecordCreateXcbSurfaceKHR(VkInstance instance, const VkXcbSurfaceCreateInfoKHR* pCreateInfo,
                                                        const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                        const RecordObject& record_obj) {
    if (record_obj.result != VK_SUCCESS) return;
    CreateObject(*pSurface, kVulkanObjectTypeSurfaceKHR, pAllocator);
}
#endif  // VK_USE_PLATFORM_XCB_KHR
#ifdef VK_USE_PLATFORM_XCB_KHR

// vkGetPhysicalDeviceXcbPresentationSupportKHR:
// Checked by chassis: physicalDevice: "VUID-vkGetPhysicalDeviceXcbPresentationSupportKHR-physicalDevice-parameter"

#endif  // VK_USE_PLATFORM_XCB_KHR
#ifdef VK_USE_PLATFORM_WAYLAND_KHR

// vkCreateWaylandSurfaceKHR:
// Checked by chassis: instance: "VUID-vkCreateWaylandSurfaceKHR-instance-parameter"

void ObjectLifetimes::PostCallRecordCreateWaylandSurfaceKHR(VkInstance instance, const VkWaylandSurfaceCreateInfoKHR* pCreateInfo,
                                                            const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                            const RecordObject& record_obj) {
    if (record_obj.result != VK_SUCCESS) return;
    CreateObject(*pSurface, kVulkanObjectTypeSurfaceKHR, pAllocator);
}
#endif  // VK_USE_PLATFORM_WAYLAND_KHR
#ifdef VK_USE_PLATFORM_WAYLAND_KHR

// vkGetPhysicalDeviceWaylandPresentationSupportKHR:
// Checked by chassis: physicalDevice: "VUID-vkGetPhysicalDeviceWaylandPresentationSupportKHR-physicalDevice-parameter"

#endif  // VK_USE_PLATFORM_WAYLAND_KHR
#ifdef VK_USE_PLATFORM_ANDROID_KHR

// vkCreateAndroidSurfaceKHR:
// Checked by chassis: instance: "VUID-vkCreateAndroidSurfaceKHR-instance-parameter"

void ObjectLifetimes::PostCallRecordCreateAndroidSurfaceKHR(VkInstance instance, const VkAndroidSurfaceCreateInfoKHR* pCreateInfo,
                                                            const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                            const RecordObject& record_obj) {
    if (record_obj.result != VK_SUCCESS) return;
    CreateObject(*pSurface, kVulkanObjectTypeSurfaceKHR, pAllocator);
}
#endif  // VK_USE_PLATFORM_ANDROID_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR

// vkCreateWin32SurfaceKHR:
// Checked by chassis: instance: "VUID-vkCreateWin32SurfaceKHR-instance-parameter"

void ObjectLifetimes::PostCallRecordCreateWin32SurfaceKHR(VkInstance instance, const VkWin32SurfaceCreateInfoKHR* pCreateInfo,
                                                          const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                          const RecordObject& record_obj) {
    if (record_obj.result != VK_SUCCESS) return;
    CreateObject(*pSurface, kVulkanObjectTypeSurfaceKHR, pAllocator);
}
#endif  // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR

// vkGetPhysicalDeviceWin32PresentationSupportKHR:
// Checked by chassis: physicalDevice: "VUID-vkGetPhysicalDeviceWin32PresentationSupportKHR-physicalDevice-parameter"

#endif  // VK_USE_PLATFORM_WIN32_KHR

// vkGetPhysicalDeviceVideoCapabilitiesKHR:
// Checked by chassis: physicalDevice: "VUID-vkGetPhysicalDeviceVideoCapabilitiesKHR-physicalDevice-parameter"

// vkGetPhysicalDeviceVideoFormatPropertiesKHR:
// Checked by chassis: physicalDevice: "VUID-vkGetPhysicalDeviceVideoFormatPropertiesKHR-physicalDevice-parameter"

// vkCreateVideoSessionKHR:
// Checked by chassis: device: "VUID-vkCreateVideoSessionKHR-device-parameter"

void ObjectLifetimes::PostCallRecordCreateVideoSessionKHR(VkDevice device, const VkVideoSessionCreateInfoKHR* pCreateInfo,
                                                          const VkAllocationCallbacks* pAllocator, VkVideoSessionKHR* pVideoSession,
                                                          const RecordObject& record_obj) {
    if (record_obj.result != VK_SUCCESS) return;
    CreateObject(*pVideoSession, kVulkanObjectTypeVideoSessionKHR, pAllocator);
}

bool ObjectLifetimes::PreCallValidateDestroyVideoSessionKHR(VkDevice device, VkVideoSessionKHR videoSession,
                                                            const VkAllocationCallbacks* pAllocator,
                                                            const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkDestroyVideoSessionKHR-device-parameter"
    skip |=
        ValidateObject(videoSession, kVulkanObjectTypeVideoSessionKHR, true, "VUID-vkDestroyVideoSessionKHR-videoSession-parameter",
                       "VUID-vkDestroyVideoSessionKHR-videoSession-parent", error_obj.location.dot(Field::videoSession));
    skip |= ValidateDestroyObject(videoSession, kVulkanObjectTypeVideoSessionKHR, pAllocator,
                                  "VUID-vkDestroyVideoSessionKHR-videoSession-07193",
                                  "VUID-vkDestroyVideoSessionKHR-videoSession-07194", error_obj.location);

    return skip;
}

void ObjectLifetimes::PreCallRecordDestroyVideoSessionKHR(VkDevice device, VkVideoSessionKHR videoSession,
                                                          const VkAllocationCallbacks* pAllocator) {
    RecordDestroyObject(videoSession, kVulkanObjectTypeVideoSessionKHR);
}

bool ObjectLifetimes::PreCallValidateGetVideoSessionMemoryRequirementsKHR(VkDevice device, VkVideoSessionKHR videoSession,
                                                                          uint32_t* pMemoryRequirementsCount,
                                                                          VkVideoSessionMemoryRequirementsKHR* pMemoryRequirements,
                                                                          const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkGetVideoSessionMemoryRequirementsKHR-device-parameter"
    skip |= ValidateObject(
        videoSession, kVulkanObjectTypeVideoSessionKHR, false, "VUID-vkGetVideoSessionMemoryRequirementsKHR-videoSession-parameter",
        "VUID-vkGetVideoSessionMemoryRequirementsKHR-videoSession-parent", error_obj.location.dot(Field::videoSession));

    return skip;
}

bool ObjectLifetimes::PreCallValidateBindVideoSessionMemoryKHR(VkDevice device, VkVideoSessionKHR videoSession,
                                                               uint32_t bindSessionMemoryInfoCount,
                                                               const VkBindVideoSessionMemoryInfoKHR* pBindSessionMemoryInfos,
                                                               const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkBindVideoSessionMemoryKHR-device-parameter"
    skip |= ValidateObject(videoSession, kVulkanObjectTypeVideoSessionKHR, false,
                           "VUID-vkBindVideoSessionMemoryKHR-videoSession-parameter",
                           "VUID-vkBindVideoSessionMemoryKHR-videoSession-parent", error_obj.location.dot(Field::videoSession));
    if (pBindSessionMemoryInfos) {
        for (uint32_t index0 = 0; index0 < bindSessionMemoryInfoCount; ++index0) {
            [[maybe_unused]] const Location index0_loc = error_obj.location.dot(Field::pBindSessionMemoryInfos, index0);
            skip |= ValidateObject(pBindSessionMemoryInfos[index0].memory, kVulkanObjectTypeDeviceMemory, false,
                                   "VUID-VkBindVideoSessionMemoryInfoKHR-memory-parameter", kVUIDUndefined,
                                   index0_loc.dot(Field::memory));
        }
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateCreateVideoSessionParametersKHR(VkDevice device,
                                                                     const VkVideoSessionParametersCreateInfoKHR* pCreateInfo,
                                                                     const VkAllocationCallbacks* pAllocator,
                                                                     VkVideoSessionParametersKHR* pVideoSessionParameters,
                                                                     const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkCreateVideoSessionParametersKHR-device-parameter"
    if (pCreateInfo) {
        [[maybe_unused]] const Location pCreateInfo_loc = error_obj.location.dot(Field::pCreateInfo);
        skip |= ValidateObject(pCreateInfo->videoSessionParametersTemplate, kVulkanObjectTypeVideoSessionParametersKHR, true,
                               "VUID-VkVideoSessionParametersCreateInfoKHR-videoSessionParametersTemplate-parameter",
                               "VUID-VkVideoSessionParametersCreateInfoKHR-commonparent",
                               pCreateInfo_loc.dot(Field::videoSessionParametersTemplate));
        skip |= ValidateObject(pCreateInfo->videoSession, kVulkanObjectTypeVideoSessionKHR, false,
                               "VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-parameter",
                               "VUID-VkVideoSessionParametersCreateInfoKHR-commonparent", pCreateInfo_loc.dot(Field::videoSession));
    }

    return skip;
}

void ObjectLifetimes::PostCallRecordCreateVideoSessionParametersKHR(VkDevice device,
                                                                    const VkVideoSessionParametersCreateInfoKHR* pCreateInfo,
                                                                    const VkAllocationCallbacks* pAllocator,
                                                                    VkVideoSessionParametersKHR* pVideoSessionParameters,
                                                                    const RecordObject& record_obj) {
    if (record_obj.result != VK_SUCCESS) return;
    CreateObject(*pVideoSessionParameters, kVulkanObjectTypeVideoSessionParametersKHR, pAllocator);
}

bool ObjectLifetimes::PreCallValidateUpdateVideoSessionParametersKHR(VkDevice device,
                                                                     VkVideoSessionParametersKHR videoSessionParameters,
                                                                     const VkVideoSessionParametersUpdateInfoKHR* pUpdateInfo,
                                                                     const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkUpdateVideoSessionParametersKHR-device-parameter"
    skip |= ValidateObject(videoSessionParameters, kVulkanObjectTypeVideoSessionParametersKHR, false,
                           "VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-parameter",
                           "VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-parent",
                           error_obj.location.dot(Field::videoSessionParameters));

    return skip;
}

bool ObjectLifetimes::PreCallValidateDestroyVideoSessionParametersKHR(VkDevice device,
                                                                      VkVideoSessionParametersKHR videoSessionParameters,
                                                                      const VkAllocationCallbacks* pAllocator,
                                                                      const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkDestroyVideoSessionParametersKHR-device-parameter"
    skip |= ValidateObject(videoSessionParameters, kVulkanObjectTypeVideoSessionParametersKHR, true,
                           "VUID-vkDestroyVideoSessionParametersKHR-videoSessionParameters-parameter",
                           "VUID-vkDestroyVideoSessionParametersKHR-videoSessionParameters-parent",
                           error_obj.location.dot(Field::videoSessionParameters));
    skip |= ValidateDestroyObject(videoSessionParameters, kVulkanObjectTypeVideoSessionParametersKHR, pAllocator,
                                  "VUID-vkDestroyVideoSessionParametersKHR-videoSessionParameters-07213",
                                  "VUID-vkDestroyVideoSessionParametersKHR-videoSessionParameters-07214", error_obj.location);

    return skip;
}

void ObjectLifetimes::PreCallRecordDestroyVideoSessionParametersKHR(VkDevice device,
                                                                    VkVideoSessionParametersKHR videoSessionParameters,
                                                                    const VkAllocationCallbacks* pAllocator) {
    RecordDestroyObject(videoSessionParameters, kVulkanObjectTypeVideoSessionParametersKHR);
}

bool ObjectLifetimes::PreCallValidateCmdBeginVideoCodingKHR(VkCommandBuffer commandBuffer,
                                                            const VkVideoBeginCodingInfoKHR* pBeginInfo,
                                                            const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: commandBuffer: "VUID-vkCmdBeginVideoCodingKHR-commandBuffer-parameter"
    if (pBeginInfo) {
        [[maybe_unused]] const Location pBeginInfo_loc = error_obj.location.dot(Field::pBeginInfo);
        skip |= ValidateObject(pBeginInfo->videoSession, kVulkanObjectTypeVideoSessionKHR, false,
                               "VUID-VkVideoBeginCodingInfoKHR-videoSession-parameter",
                               "VUID-VkVideoBeginCodingInfoKHR-commonparent", pBeginInfo_loc.dot(Field::videoSession));
        skip |= ValidateObject(pBeginInfo->videoSessionParameters, kVulkanObjectTypeVideoSessionParametersKHR, true,
                               "VUID-VkVideoBeginCodingInfoKHR-videoSessionParameters-parameter",
                               "VUID-VkVideoBeginCodingInfoKHR-commonparent", pBeginInfo_loc.dot(Field::videoSessionParameters));
        if (pBeginInfo->pReferenceSlots) {
            for (uint32_t index1 = 0; index1 < pBeginInfo->referenceSlotCount; ++index1) {
                [[maybe_unused]] const Location index1_loc = pBeginInfo_loc.dot(Field::pReferenceSlots, index1);
                if (pBeginInfo->pReferenceSlots[index1].pPictureResource) {
                    [[maybe_unused]] const Location pPictureResource_loc = index1_loc.dot(Field::pPictureResource);
                    skip |= ValidateObject(pBeginInfo->pReferenceSlots[index1].pPictureResource->imageViewBinding,
                                           kVulkanObjectTypeImageView, false,
                                           "VUID-VkVideoPictureResourceInfoKHR-imageViewBinding-parameter", kVUIDUndefined,
                                           pPictureResource_loc.dot(Field::imageViewBinding));
                }
            }
        }
    }

    return skip;
}

// vkCmdEndVideoCodingKHR:
// Checked by chassis: commandBuffer: "VUID-vkCmdEndVideoCodingKHR-commandBuffer-parameter"

// vkCmdControlVideoCodingKHR:
// Checked by chassis: commandBuffer: "VUID-vkCmdControlVideoCodingKHR-commandBuffer-parameter"

bool ObjectLifetimes::PreCallValidateCmdDecodeVideoKHR(VkCommandBuffer commandBuffer, const VkVideoDecodeInfoKHR* pDecodeInfo,
                                                       const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: commandBuffer: "VUID-vkCmdDecodeVideoKHR-commandBuffer-parameter"
    if (pDecodeInfo) {
        [[maybe_unused]] const Location pDecodeInfo_loc = error_obj.location.dot(Field::pDecodeInfo);
        skip |=
            ValidateObject(pDecodeInfo->srcBuffer, kVulkanObjectTypeBuffer, false, "VUID-VkVideoDecodeInfoKHR-srcBuffer-parameter",
                           kVUIDUndefined, pDecodeInfo_loc.dot(Field::srcBuffer));
        [[maybe_unused]] const Location dstPictureResource_loc = pDecodeInfo_loc.dot(Field::dstPictureResource);
        skip |= ValidateObject(pDecodeInfo->dstPictureResource.imageViewBinding, kVulkanObjectTypeImageView, false,
                               "VUID-VkVideoPictureResourceInfoKHR-imageViewBinding-parameter", kVUIDUndefined,
                               dstPictureResource_loc.dot(Field::imageViewBinding));
        if (pDecodeInfo->pSetupReferenceSlot) {
            [[maybe_unused]] const Location pSetupReferenceSlot_loc = pDecodeInfo_loc.dot(Field::pSetupReferenceSlot);
            if (pDecodeInfo->pSetupReferenceSlot->pPictureResource) {
                [[maybe_unused]] const Location pPictureResource_loc = pSetupReferenceSlot_loc.dot(Field::pPictureResource);
                skip |=
                    ValidateObject(pDecodeInfo->pSetupReferenceSlot->pPictureResource->imageViewBinding, kVulkanObjectTypeImageView,
                                   false, "VUID-VkVideoPictureResourceInfoKHR-imageViewBinding-parameter", kVUIDUndefined,
                                   pPictureResource_loc.dot(Field::imageViewBinding));
            }
        }
        if (pDecodeInfo->pReferenceSlots) {
            for (uint32_t index1 = 0; index1 < pDecodeInfo->referenceSlotCount; ++index1) {
                [[maybe_unused]] const Location index1_loc = pDecodeInfo_loc.dot(Field::pReferenceSlots, index1);
                if (pDecodeInfo->pReferenceSlots[index1].pPictureResource) {
                    [[maybe_unused]] const Location pPictureResource_loc = index1_loc.dot(Field::pPictureResource);
                    skip |= ValidateObject(pDecodeInfo->pReferenceSlots[index1].pPictureResource->imageViewBinding,
                                           kVulkanObjectTypeImageView, false,
                                           "VUID-VkVideoPictureResourceInfoKHR-imageViewBinding-parameter", kVUIDUndefined,
                                           pPictureResource_loc.dot(Field::imageViewBinding));
                }
            }
        }
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdBeginRenderingKHR(VkCommandBuffer commandBuffer, const VkRenderingInfo* pRenderingInfo,
                                                          const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: commandBuffer: "VUID-vkCmdBeginRendering-commandBuffer-parameter"
    if (pRenderingInfo) {
        [[maybe_unused]] const Location pRenderingInfo_loc = error_obj.location.dot(Field::pRenderingInfo);
        if (pRenderingInfo->pColorAttachments) {
            for (uint32_t index1 = 0; index1 < pRenderingInfo->colorAttachmentCount; ++index1) {
                [[maybe_unused]] const Location index1_loc = pRenderingInfo_loc.dot(Field::pColorAttachments, index1);
                skip |= ValidateObject(pRenderingInfo->pColorAttachments[index1].imageView, kVulkanObjectTypeImageView, true,
                                       "VUID-VkRenderingAttachmentInfo-imageView-parameter",
                                       "VUID-VkRenderingAttachmentInfo-commonparent", index1_loc.dot(Field::imageView));
                skip |= ValidateObject(pRenderingInfo->pColorAttachments[index1].resolveImageView, kVulkanObjectTypeImageView, true,
                                       "VUID-VkRenderingAttachmentInfo-resolveImageView-parameter",
                                       "VUID-VkRenderingAttachmentInfo-commonparent", index1_loc.dot(Field::resolveImageView));
            }
        }
        if (pRenderingInfo->pDepthAttachment) {
            [[maybe_unused]] const Location pDepthAttachment_loc = pRenderingInfo_loc.dot(Field::pDepthAttachment);
            skip |= ValidateObject(pRenderingInfo->pDepthAttachment->imageView, kVulkanObjectTypeImageView, true,
                                   "VUID-VkRenderingAttachmentInfo-imageView-parameter",
                                   "VUID-VkRenderingAttachmentInfo-commonparent", pDepthAttachment_loc.dot(Field::imageView));
            skip |=
                ValidateObject(pRenderingInfo->pDepthAttachment->resolveImageView, kVulkanObjectTypeImageView, true,
                               "VUID-VkRenderingAttachmentInfo-resolveImageView-parameter",
                               "VUID-VkRenderingAttachmentInfo-commonparent", pDepthAttachment_loc.dot(Field::resolveImageView));
        }
        if (pRenderingInfo->pStencilAttachment) {
            [[maybe_unused]] const Location pStencilAttachment_loc = pRenderingInfo_loc.dot(Field::pStencilAttachment);
            skip |= ValidateObject(pRenderingInfo->pStencilAttachment->imageView, kVulkanObjectTypeImageView, true,
                                   "VUID-VkRenderingAttachmentInfo-imageView-parameter",
                                   "VUID-VkRenderingAttachmentInfo-commonparent", pStencilAttachment_loc.dot(Field::imageView));
            skip |=
                ValidateObject(pRenderingInfo->pStencilAttachment->resolveImageView, kVulkanObjectTypeImageView, true,
                               "VUID-VkRenderingAttachmentInfo-resolveImageView-parameter",
                               "VUID-VkRenderingAttachmentInfo-commonparent", pStencilAttachment_loc.dot(Field::resolveImageView));
        }
        if (auto pNext = vku::FindStructInPNextChain<VkRenderingFragmentDensityMapAttachmentInfoEXT>(pRenderingInfo->pNext)) {
            const Location pNext_loc = pRenderingInfo_loc.pNext(Struct::VkRenderingFragmentDensityMapAttachmentInfoEXT);
            skip |= ValidateObject(pNext->imageView, kVulkanObjectTypeImageView, false,
                                   "VUID-VkRenderingFragmentDensityMapAttachmentInfoEXT-imageView-parameter", kVUIDUndefined,
                                   pNext_loc.dot(Field::imageView));
        }
        if (auto pNext = vku::FindStructInPNextChain<VkRenderingFragmentShadingRateAttachmentInfoKHR>(pRenderingInfo->pNext)) {
            const Location pNext_loc = pRenderingInfo_loc.pNext(Struct::VkRenderingFragmentShadingRateAttachmentInfoKHR);
            skip |= ValidateObject(pNext->imageView, kVulkanObjectTypeImageView, true,
                                   "VUID-VkRenderingFragmentShadingRateAttachmentInfoKHR-imageView-parameter", kVUIDUndefined,
                                   pNext_loc.dot(Field::imageView));
        }
    }

    return skip;
}

// vkCmdEndRenderingKHR:
// Checked by chassis: commandBuffer: "VUID-vkCmdEndRendering-commandBuffer-parameter"

// vkGetPhysicalDeviceFeatures2KHR:
// Checked by chassis: physicalDevice: "VUID-vkGetPhysicalDeviceFeatures2-physicalDevice-parameter"

// vkGetPhysicalDeviceProperties2KHR:
// Checked by chassis: physicalDevice: "VUID-vkGetPhysicalDeviceProperties2-physicalDevice-parameter"

// vkGetPhysicalDeviceFormatProperties2KHR:
// Checked by chassis: physicalDevice: "VUID-vkGetPhysicalDeviceFormatProperties2-physicalDevice-parameter"

// vkGetPhysicalDeviceImageFormatProperties2KHR:
// Checked by chassis: physicalDevice: "VUID-vkGetPhysicalDeviceImageFormatProperties2-physicalDevice-parameter"

// vkGetPhysicalDeviceMemoryProperties2KHR:
// Checked by chassis: physicalDevice: "VUID-vkGetPhysicalDeviceMemoryProperties2-physicalDevice-parameter"

// vkGetPhysicalDeviceSparseImageFormatProperties2KHR:
// Checked by chassis: physicalDevice: "VUID-vkGetPhysicalDeviceSparseImageFormatProperties2-physicalDevice-parameter"

// vkGetDeviceGroupPeerMemoryFeaturesKHR:
// Checked by chassis: device: "VUID-vkGetDeviceGroupPeerMemoryFeatures-device-parameter"

// vkCmdSetDeviceMaskKHR:
// Checked by chassis: commandBuffer: "VUID-vkCmdSetDeviceMask-commandBuffer-parameter"

// vkCmdDispatchBaseKHR:
// Checked by chassis: commandBuffer: "VUID-vkCmdDispatchBase-commandBuffer-parameter"

bool ObjectLifetimes::PreCallValidateTrimCommandPoolKHR(VkDevice device, VkCommandPool commandPool, VkCommandPoolTrimFlags flags,
                                                        const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkTrimCommandPool-device-parameter"
    skip |= ValidateObject(commandPool, kVulkanObjectTypeCommandPool, false, "VUID-vkTrimCommandPool-commandPool-parameter",
                           "VUID-vkTrimCommandPool-commandPool-parent", error_obj.location.dot(Field::commandPool));

    return skip;
}

// vkEnumeratePhysicalDeviceGroupsKHR:
// Checked by chassis: instance: "VUID-vkEnumeratePhysicalDeviceGroups-instance-parameter"

void ObjectLifetimes::PostCallRecordEnumeratePhysicalDeviceGroupsKHR(
    VkInstance instance, uint32_t* pPhysicalDeviceGroupCount, VkPhysicalDeviceGroupProperties* pPhysicalDeviceGroupProperties,
    const RecordObject& record_obj) {
    if (record_obj.result != VK_SUCCESS && record_obj.result != VK_INCOMPLETE) return;

    if (pPhysicalDeviceGroupProperties) {
        const RecordObject record_obj(vvl::Func::vkEnumeratePhysicalDevices, VK_SUCCESS);
        for (uint32_t device_group_index = 0; device_group_index < *pPhysicalDeviceGroupCount; device_group_index++) {
            PostCallRecordEnumeratePhysicalDevices(instance,
                                                   &pPhysicalDeviceGroupProperties[device_group_index].physicalDeviceCount,
                                                   pPhysicalDeviceGroupProperties[device_group_index].physicalDevices, record_obj);
        }
    }
}

// vkGetPhysicalDeviceExternalBufferPropertiesKHR:
// Checked by chassis: physicalDevice: "VUID-vkGetPhysicalDeviceExternalBufferProperties-physicalDevice-parameter"

#ifdef VK_USE_PLATFORM_WIN32_KHR

bool ObjectLifetimes::PreCallValidateGetMemoryWin32HandleKHR(VkDevice device,
                                                             const VkMemoryGetWin32HandleInfoKHR* pGetWin32HandleInfo,
                                                             HANDLE* pHandle, const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkGetMemoryWin32HandleKHR-device-parameter"
    if (pGetWin32HandleInfo) {
        [[maybe_unused]] const Location pGetWin32HandleInfo_loc = error_obj.location.dot(Field::pGetWin32HandleInfo);
        skip |= ValidateObject(pGetWin32HandleInfo->memory, kVulkanObjectTypeDeviceMemory, false,
                               "VUID-VkMemoryGetWin32HandleInfoKHR-memory-parameter", kVUIDUndefined,
                               pGetWin32HandleInfo_loc.dot(Field::memory));
    }

    return skip;
}
#endif  // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR

// vkGetMemoryWin32HandlePropertiesKHR:
// Checked by chassis: device: "VUID-vkGetMemoryWin32HandlePropertiesKHR-device-parameter"

#endif  // VK_USE_PLATFORM_WIN32_KHR

bool ObjectLifetimes::PreCallValidateGetMemoryFdKHR(VkDevice device, const VkMemoryGetFdInfoKHR* pGetFdInfo, int* pFd,
                                                    const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkGetMemoryFdKHR-device-parameter"
    if (pGetFdInfo) {
        [[maybe_unused]] const Location pGetFdInfo_loc = error_obj.location.dot(Field::pGetFdInfo);
        skip |= ValidateObject(pGetFdInfo->memory, kVulkanObjectTypeDeviceMemory, false,
                               "VUID-VkMemoryGetFdInfoKHR-memory-parameter", kVUIDUndefined, pGetFdInfo_loc.dot(Field::memory));
    }

    return skip;
}

// vkGetMemoryFdPropertiesKHR:
// Checked by chassis: device: "VUID-vkGetMemoryFdPropertiesKHR-device-parameter"

// vkGetPhysicalDeviceExternalSemaphorePropertiesKHR:
// Checked by chassis: physicalDevice: "VUID-vkGetPhysicalDeviceExternalSemaphoreProperties-physicalDevice-parameter"

#ifdef VK_USE_PLATFORM_WIN32_KHR

bool ObjectLifetimes::PreCallValidateImportSemaphoreWin32HandleKHR(
    VkDevice device, const VkImportSemaphoreWin32HandleInfoKHR* pImportSemaphoreWin32HandleInfo,
    const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkImportSemaphoreWin32HandleKHR-device-parameter"
    if (pImportSemaphoreWin32HandleInfo) {
        [[maybe_unused]] const Location pImportSemaphoreWin32HandleInfo_loc =
            error_obj.location.dot(Field::pImportSemaphoreWin32HandleInfo);
        skip |= ValidateObject(pImportSemaphoreWin32HandleInfo->semaphore, kVulkanObjectTypeSemaphore, false,
                               "VUID-VkImportSemaphoreWin32HandleInfoKHR-semaphore-parameter", kVUIDUndefined,
                               pImportSemaphoreWin32HandleInfo_loc.dot(Field::semaphore));
    }

    return skip;
}
#endif  // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR

bool ObjectLifetimes::PreCallValidateGetSemaphoreWin32HandleKHR(VkDevice device,
                                                                const VkSemaphoreGetWin32HandleInfoKHR* pGetWin32HandleInfo,
                                                                HANDLE* pHandle, const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkGetSemaphoreWin32HandleKHR-device-parameter"
    if (pGetWin32HandleInfo) {
        [[maybe_unused]] const Location pGetWin32HandleInfo_loc = error_obj.location.dot(Field::pGetWin32HandleInfo);
        skip |= ValidateObject(pGetWin32HandleInfo->semaphore, kVulkanObjectTypeSemaphore, false,
                               "VUID-VkSemaphoreGetWin32HandleInfoKHR-semaphore-parameter", kVUIDUndefined,
                               pGetWin32HandleInfo_loc.dot(Field::semaphore));
    }

    return skip;
}
#endif  // VK_USE_PLATFORM_WIN32_KHR

bool ObjectLifetimes::PreCallValidateImportSemaphoreFdKHR(VkDevice device, const VkImportSemaphoreFdInfoKHR* pImportSemaphoreFdInfo,
                                                          const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkImportSemaphoreFdKHR-device-parameter"
    if (pImportSemaphoreFdInfo) {
        [[maybe_unused]] const Location pImportSemaphoreFdInfo_loc = error_obj.location.dot(Field::pImportSemaphoreFdInfo);
        skip |= ValidateObject(pImportSemaphoreFdInfo->semaphore, kVulkanObjectTypeSemaphore, false,
                               "VUID-VkImportSemaphoreFdInfoKHR-semaphore-parameter", kVUIDUndefined,
                               pImportSemaphoreFdInfo_loc.dot(Field::semaphore));
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetSemaphoreFdKHR(VkDevice device, const VkSemaphoreGetFdInfoKHR* pGetFdInfo, int* pFd,
                                                       const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkGetSemaphoreFdKHR-device-parameter"
    if (pGetFdInfo) {
        [[maybe_unused]] const Location pGetFdInfo_loc = error_obj.location.dot(Field::pGetFdInfo);
        skip |= ValidateObject(pGetFdInfo->semaphore, kVulkanObjectTypeSemaphore, false,
                               "VUID-VkSemaphoreGetFdInfoKHR-semaphore-parameter", kVUIDUndefined,
                               pGetFdInfo_loc.dot(Field::semaphore));
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdPushDescriptorSetWithTemplateKHR(VkCommandBuffer commandBuffer,
                                                                         VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                                         VkPipelineLayout layout, uint32_t set, const void* pData,
                                                                         const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: commandBuffer: "VUID-vkCmdPushDescriptorSetWithTemplateKHR-commandBuffer-parameter"
    // Checked by chassis: commandBuffer: "VUID-vkCmdPushDescriptorSetWithTemplateKHR-commonparent"
    skip |= ValidateObject(descriptorUpdateTemplate, kVulkanObjectTypeDescriptorUpdateTemplate, false,
                           "VUID-vkCmdPushDescriptorSetWithTemplateKHR-descriptorUpdateTemplate-parameter",
                           "VUID-vkCmdPushDescriptorSetWithTemplateKHR-commonparent",
                           error_obj.location.dot(Field::descriptorUpdateTemplate));
    skip |= ValidateObject(layout, kVulkanObjectTypePipelineLayout, false,
                           "VUID-vkCmdPushDescriptorSetWithTemplateKHR-layout-parameter",
                           "VUID-vkCmdPushDescriptorSetWithTemplateKHR-commonparent", error_obj.location.dot(Field::layout));

    return skip;
}

bool ObjectLifetimes::PreCallValidateDestroyDescriptorUpdateTemplateKHR(VkDevice device,
                                                                        VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                                        const VkAllocationCallbacks* pAllocator,
                                                                        const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkDestroyDescriptorUpdateTemplate-device-parameter"
    skip |= ValidateObject(descriptorUpdateTemplate, kVulkanObjectTypeDescriptorUpdateTemplate, true,
                           "VUID-vkDestroyDescriptorUpdateTemplate-descriptorUpdateTemplate-parameter",
                           "VUID-vkDestroyDescriptorUpdateTemplate-descriptorUpdateTemplate-parent",
                           error_obj.location.dot(Field::descriptorUpdateTemplate));
    skip |= ValidateDestroyObject(descriptorUpdateTemplate, kVulkanObjectTypeDescriptorUpdateTemplate, pAllocator,
                                  "VUID-vkDestroyDescriptorUpdateTemplate-descriptorSetLayout-00356",
                                  "VUID-vkDestroyDescriptorUpdateTemplate-descriptorSetLayout-00357", error_obj.location);

    return skip;
}

void ObjectLifetimes::PreCallRecordDestroyDescriptorUpdateTemplateKHR(VkDevice device,
                                                                      VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                                      const VkAllocationCallbacks* pAllocator) {
    RecordDestroyObject(descriptorUpdateTemplate, kVulkanObjectTypeDescriptorUpdateTemplate);
}

bool ObjectLifetimes::PreCallValidateUpdateDescriptorSetWithTemplateKHR(VkDevice device, VkDescriptorSet descriptorSet,
                                                                        VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                                        const void* pData, const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkUpdateDescriptorSetWithTemplate-device-parameter"
    skip |= ValidateObject(
        descriptorSet, kVulkanObjectTypeDescriptorSet, false, "VUID-vkUpdateDescriptorSetWithTemplate-descriptorSet-parameter",
        "VUID-vkUpdateDescriptorSetWithTemplate-descriptorSet-parent", error_obj.location.dot(Field::descriptorSet));
    skip |= ValidateObject(descriptorUpdateTemplate, kVulkanObjectTypeDescriptorUpdateTemplate, false,
                           "VUID-vkUpdateDescriptorSetWithTemplate-descriptorUpdateTemplate-parameter",
                           "VUID-vkUpdateDescriptorSetWithTemplate-descriptorUpdateTemplate-parent",
                           error_obj.location.dot(Field::descriptorUpdateTemplate));

    return skip;
}

// vkCreateRenderPass2KHR:
// Checked by chassis: device: "VUID-vkCreateRenderPass2-device-parameter"

void ObjectLifetimes::PostCallRecordCreateRenderPass2KHR(VkDevice device, const VkRenderPassCreateInfo2* pCreateInfo,
                                                         const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass,
                                                         const RecordObject& record_obj) {
    if (record_obj.result != VK_SUCCESS) return;
    CreateObject(*pRenderPass, kVulkanObjectTypeRenderPass, pAllocator);
}

bool ObjectLifetimes::PreCallValidateCmdBeginRenderPass2KHR(VkCommandBuffer commandBuffer,
                                                            const VkRenderPassBeginInfo* pRenderPassBegin,
                                                            const VkSubpassBeginInfo* pSubpassBeginInfo,
                                                            const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: commandBuffer: "VUID-vkCmdBeginRenderPass2-commandBuffer-parameter"
    if (pRenderPassBegin) {
        [[maybe_unused]] const Location pRenderPassBegin_loc = error_obj.location.dot(Field::pRenderPassBegin);
        skip |= ValidateObject(pRenderPassBegin->renderPass, kVulkanObjectTypeRenderPass, false,
                               "VUID-VkRenderPassBeginInfo-renderPass-parameter", "VUID-VkRenderPassBeginInfo-commonparent",
                               pRenderPassBegin_loc.dot(Field::renderPass));
        skip |= ValidateObject(pRenderPassBegin->framebuffer, kVulkanObjectTypeFramebuffer, false,
                               "VUID-VkRenderPassBeginInfo-framebuffer-parameter", "VUID-VkRenderPassBeginInfo-commonparent",
                               pRenderPassBegin_loc.dot(Field::framebuffer));
        if (auto pNext = vku::FindStructInPNextChain<VkRenderPassAttachmentBeginInfo>(pRenderPassBegin->pNext)) {
            const Location pNext_loc = pRenderPassBegin_loc.pNext(Struct::VkRenderPassAttachmentBeginInfo);

            if ((pNext->attachmentCount > 0) && (pNext->pAttachments)) {
                for (uint32_t index2 = 0; index2 < pNext->attachmentCount; ++index2) {
                    skip |= ValidateObject(pNext->pAttachments[index2], kVulkanObjectTypeImageView, false,
                                           "VUID-VkRenderPassAttachmentBeginInfo-pAttachments-parameter", kVUIDUndefined,
                                           pNext_loc.dot(Field::pAttachments, index2));
                }
            }
        }
    }

    return skip;
}

// vkCmdNextSubpass2KHR:
// Checked by chassis: commandBuffer: "VUID-vkCmdNextSubpass2-commandBuffer-parameter"

// vkCmdEndRenderPass2KHR:
// Checked by chassis: commandBuffer: "VUID-vkCmdEndRenderPass2-commandBuffer-parameter"

bool ObjectLifetimes::PreCallValidateGetSwapchainStatusKHR(VkDevice device, VkSwapchainKHR swapchain,
                                                           const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkGetSwapchainStatusKHR-device-parameter"
    skip |= ValidateObject(swapchain, kVulkanObjectTypeSwapchainKHR, false, "VUID-vkGetSwapchainStatusKHR-swapchain-parameter",
                           "VUID-vkGetSwapchainStatusKHR-swapchain-parent", error_obj.location.dot(Field::swapchain));

    return skip;
}

// vkGetPhysicalDeviceExternalFencePropertiesKHR:
// Checked by chassis: physicalDevice: "VUID-vkGetPhysicalDeviceExternalFenceProperties-physicalDevice-parameter"

#ifdef VK_USE_PLATFORM_WIN32_KHR

bool ObjectLifetimes::PreCallValidateImportFenceWin32HandleKHR(VkDevice device,
                                                               const VkImportFenceWin32HandleInfoKHR* pImportFenceWin32HandleInfo,
                                                               const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkImportFenceWin32HandleKHR-device-parameter"
    if (pImportFenceWin32HandleInfo) {
        [[maybe_unused]] const Location pImportFenceWin32HandleInfo_loc =
            error_obj.location.dot(Field::pImportFenceWin32HandleInfo);
        skip |= ValidateObject(pImportFenceWin32HandleInfo->fence, kVulkanObjectTypeFence, false,
                               "VUID-VkImportFenceWin32HandleInfoKHR-fence-parameter", kVUIDUndefined,
                               pImportFenceWin32HandleInfo_loc.dot(Field::fence));
    }

    return skip;
}
#endif  // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR

bool ObjectLifetimes::PreCallValidateGetFenceWin32HandleKHR(VkDevice device,
                                                            const VkFenceGetWin32HandleInfoKHR* pGetWin32HandleInfo,
                                                            HANDLE* pHandle, const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkGetFenceWin32HandleKHR-device-parameter"
    if (pGetWin32HandleInfo) {
        [[maybe_unused]] const Location pGetWin32HandleInfo_loc = error_obj.location.dot(Field::pGetWin32HandleInfo);
        skip |= ValidateObject(pGetWin32HandleInfo->fence, kVulkanObjectTypeFence, false,
                               "VUID-VkFenceGetWin32HandleInfoKHR-fence-parameter", kVUIDUndefined,
                               pGetWin32HandleInfo_loc.dot(Field::fence));
    }

    return skip;
}
#endif  // VK_USE_PLATFORM_WIN32_KHR

bool ObjectLifetimes::PreCallValidateImportFenceFdKHR(VkDevice device, const VkImportFenceFdInfoKHR* pImportFenceFdInfo,
                                                      const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkImportFenceFdKHR-device-parameter"
    if (pImportFenceFdInfo) {
        [[maybe_unused]] const Location pImportFenceFdInfo_loc = error_obj.location.dot(Field::pImportFenceFdInfo);
        skip |=
            ValidateObject(pImportFenceFdInfo->fence, kVulkanObjectTypeFence, false, "VUID-VkImportFenceFdInfoKHR-fence-parameter",
                           kVUIDUndefined, pImportFenceFdInfo_loc.dot(Field::fence));
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetFenceFdKHR(VkDevice device, const VkFenceGetFdInfoKHR* pGetFdInfo, int* pFd,
                                                   const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkGetFenceFdKHR-device-parameter"
    if (pGetFdInfo) {
        [[maybe_unused]] const Location pGetFdInfo_loc = error_obj.location.dot(Field::pGetFdInfo);
        skip |= ValidateObject(pGetFdInfo->fence, kVulkanObjectTypeFence, false, "VUID-VkFenceGetFdInfoKHR-fence-parameter",
                               kVUIDUndefined, pGetFdInfo_loc.dot(Field::fence));
    }

    return skip;
}

// vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR:
// Checked by chassis: physicalDevice:
// "VUID-vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR-physicalDevice-parameter"

// vkGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR:
// Checked by chassis: physicalDevice: "VUID-vkGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR-physicalDevice-parameter"

// vkAcquireProfilingLockKHR:
// Checked by chassis: device: "VUID-vkAcquireProfilingLockKHR-device-parameter"

// vkReleaseProfilingLockKHR:
// Checked by chassis: device: "VUID-vkReleaseProfilingLockKHR-device-parameter"

bool ObjectLifetimes::PreCallValidateGetPhysicalDeviceSurfaceCapabilities2KHR(VkPhysicalDevice physicalDevice,
                                                                              const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo,
                                                                              VkSurfaceCapabilities2KHR* pSurfaceCapabilities,
                                                                              const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: physicalDevice: "VUID-vkGetPhysicalDeviceSurfaceCapabilities2KHR-physicalDevice-parameter"
    if (pSurfaceInfo) {
        [[maybe_unused]] const Location pSurfaceInfo_loc = error_obj.location.dot(Field::pSurfaceInfo);
        skip |= ValidateObject(pSurfaceInfo->surface, kVulkanObjectTypeSurfaceKHR, true,
                               "VUID-VkPhysicalDeviceSurfaceInfo2KHR-surface-parameter", kVUIDUndefined,
                               pSurfaceInfo_loc.dot(Field::surface));
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetPhysicalDeviceSurfaceFormats2KHR(VkPhysicalDevice physicalDevice,
                                                                         const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo,
                                                                         uint32_t* pSurfaceFormatCount,
                                                                         VkSurfaceFormat2KHR* pSurfaceFormats,
                                                                         const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: physicalDevice: "VUID-vkGetPhysicalDeviceSurfaceFormats2KHR-physicalDevice-parameter"
    if (pSurfaceInfo) {
        [[maybe_unused]] const Location pSurfaceInfo_loc = error_obj.location.dot(Field::pSurfaceInfo);
        skip |= ValidateObject(pSurfaceInfo->surface, kVulkanObjectTypeSurfaceKHR, true,
                               "VUID-VkPhysicalDeviceSurfaceInfo2KHR-surface-parameter", kVUIDUndefined,
                               pSurfaceInfo_loc.dot(Field::surface));
    }

    return skip;
}

// vkGetPhysicalDeviceDisplayPlaneProperties2KHR:
// Checked by chassis: physicalDevice: "VUID-vkGetPhysicalDeviceDisplayPlaneProperties2KHR-physicalDevice-parameter"

bool ObjectLifetimes::PreCallValidateGetDisplayPlaneCapabilities2KHR(VkPhysicalDevice physicalDevice,
                                                                     const VkDisplayPlaneInfo2KHR* pDisplayPlaneInfo,
                                                                     VkDisplayPlaneCapabilities2KHR* pCapabilities,
                                                                     const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: physicalDevice: "VUID-vkGetDisplayPlaneCapabilities2KHR-physicalDevice-parameter"
    if (pDisplayPlaneInfo) {
        [[maybe_unused]] const Location pDisplayPlaneInfo_loc = error_obj.location.dot(Field::pDisplayPlaneInfo);
        skip |=
            ValidateObject(pDisplayPlaneInfo->mode, kVulkanObjectTypeDisplayModeKHR, false,
                           "VUID-VkDisplayPlaneInfo2KHR-mode-parameter", kVUIDUndefined, pDisplayPlaneInfo_loc.dot(Field::mode));
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetImageMemoryRequirements2KHR(VkDevice device, const VkImageMemoryRequirementsInfo2* pInfo,
                                                                    VkMemoryRequirements2* pMemoryRequirements,
                                                                    const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkGetImageMemoryRequirements2-device-parameter"
    if (pInfo) {
        [[maybe_unused]] const Location pInfo_loc = error_obj.location.dot(Field::pInfo);
        skip |= ValidateObject(pInfo->image, kVulkanObjectTypeImage, false, "VUID-VkImageMemoryRequirementsInfo2-image-parameter",
                               kVUIDUndefined, pInfo_loc.dot(Field::image));
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetBufferMemoryRequirements2KHR(VkDevice device, const VkBufferMemoryRequirementsInfo2* pInfo,
                                                                     VkMemoryRequirements2* pMemoryRequirements,
                                                                     const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkGetBufferMemoryRequirements2-device-parameter"
    if (pInfo) {
        [[maybe_unused]] const Location pInfo_loc = error_obj.location.dot(Field::pInfo);
        skip |=
            ValidateObject(pInfo->buffer, kVulkanObjectTypeBuffer, false, "VUID-VkBufferMemoryRequirementsInfo2-buffer-parameter",
                           kVUIDUndefined, pInfo_loc.dot(Field::buffer));
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetImageSparseMemoryRequirements2KHR(
    VkDevice device, const VkImageSparseMemoryRequirementsInfo2* pInfo, uint32_t* pSparseMemoryRequirementCount,
    VkSparseImageMemoryRequirements2* pSparseMemoryRequirements, const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkGetImageSparseMemoryRequirements2-device-parameter"
    if (pInfo) {
        [[maybe_unused]] const Location pInfo_loc = error_obj.location.dot(Field::pInfo);
        skip |=
            ValidateObject(pInfo->image, kVulkanObjectTypeImage, false, "VUID-VkImageSparseMemoryRequirementsInfo2-image-parameter",
                           kVUIDUndefined, pInfo_loc.dot(Field::image));
    }

    return skip;
}

// vkCreateSamplerYcbcrConversionKHR:
// Checked by chassis: device: "VUID-vkCreateSamplerYcbcrConversion-device-parameter"

void ObjectLifetimes::PostCallRecordCreateSamplerYcbcrConversionKHR(VkDevice device,
                                                                    const VkSamplerYcbcrConversionCreateInfo* pCreateInfo,
                                                                    const VkAllocationCallbacks* pAllocator,
                                                                    VkSamplerYcbcrConversion* pYcbcrConversion,
                                                                    const RecordObject& record_obj) {
    if (record_obj.result != VK_SUCCESS) return;
    CreateObject(*pYcbcrConversion, kVulkanObjectTypeSamplerYcbcrConversion, pAllocator);
}

bool ObjectLifetimes::PreCallValidateDestroySamplerYcbcrConversionKHR(VkDevice device, VkSamplerYcbcrConversion ycbcrConversion,
                                                                      const VkAllocationCallbacks* pAllocator,
                                                                      const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkDestroySamplerYcbcrConversion-device-parameter"
    skip |= ValidateObject(ycbcrConversion, kVulkanObjectTypeSamplerYcbcrConversion, true,
                           "VUID-vkDestroySamplerYcbcrConversion-ycbcrConversion-parameter",
                           "VUID-vkDestroySamplerYcbcrConversion-ycbcrConversion-parent",
                           error_obj.location.dot(Field::ycbcrConversion));
    skip |= ValidateDestroyObject(ycbcrConversion, kVulkanObjectTypeSamplerYcbcrConversion, pAllocator, kVUIDUndefined,
                                  kVUIDUndefined, error_obj.location);

    return skip;
}

void ObjectLifetimes::PreCallRecordDestroySamplerYcbcrConversionKHR(VkDevice device, VkSamplerYcbcrConversion ycbcrConversion,
                                                                    const VkAllocationCallbacks* pAllocator) {
    RecordDestroyObject(ycbcrConversion, kVulkanObjectTypeSamplerYcbcrConversion);
}

bool ObjectLifetimes::PreCallValidateBindBufferMemory2KHR(VkDevice device, uint32_t bindInfoCount,
                                                          const VkBindBufferMemoryInfo* pBindInfos,
                                                          const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkBindBufferMemory2-device-parameter"
    if (pBindInfos) {
        for (uint32_t index0 = 0; index0 < bindInfoCount; ++index0) {
            [[maybe_unused]] const Location index0_loc = error_obj.location.dot(Field::pBindInfos, index0);
            skip |= ValidateObject(pBindInfos[index0].buffer, kVulkanObjectTypeBuffer, false,
                                   "VUID-VkBindBufferMemoryInfo-buffer-parameter", "VUID-VkBindBufferMemoryInfo-commonparent",
                                   index0_loc.dot(Field::buffer));
            skip |= ValidateObject(pBindInfos[index0].memory, kVulkanObjectTypeDeviceMemory, false,
                                   "VUID-VkBindBufferMemoryInfo-memory-parameter", "VUID-VkBindBufferMemoryInfo-commonparent",
                                   index0_loc.dot(Field::memory));
        }
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateBindImageMemory2KHR(VkDevice device, uint32_t bindInfoCount,
                                                         const VkBindImageMemoryInfo* pBindInfos,
                                                         const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkBindImageMemory2-device-parameter"
    if (pBindInfos) {
        for (uint32_t index0 = 0; index0 < bindInfoCount; ++index0) {
            [[maybe_unused]] const Location index0_loc = error_obj.location.dot(Field::pBindInfos, index0);
            skip |= ValidateObject(pBindInfos[index0].image, kVulkanObjectTypeImage, false,
                                   "VUID-VkBindImageMemoryInfo-image-parameter", "VUID-VkBindImageMemoryInfo-commonparent",
                                   index0_loc.dot(Field::image));
            skip |= ValidateObject(pBindInfos[index0].memory, kVulkanObjectTypeDeviceMemory, true, kVUIDUndefined,
                                   "VUID-VkBindImageMemoryInfo-commonparent", index0_loc.dot(Field::memory));
            if (auto pNext = vku::FindStructInPNextChain<VkBindImageMemorySwapchainInfoKHR>(pBindInfos[index0].pNext)) {
                const Location pNext_loc = index0_loc.pNext(Struct::VkBindImageMemorySwapchainInfoKHR);
                skip |= ValidateObject(pNext->swapchain, kVulkanObjectTypeSwapchainKHR, false,
                                       "VUID-VkBindImageMemorySwapchainInfoKHR-swapchain-parameter", kVUIDUndefined,
                                       pNext_loc.dot(Field::swapchain));
            }
        }
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdDrawIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                             VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                             uint32_t maxDrawCount, uint32_t stride,
                                                             const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: commandBuffer: "VUID-vkCmdDrawIndirectCount-commandBuffer-parameter"
    // Checked by chassis: commandBuffer: "VUID-vkCmdDrawIndirectCount-commonparent"
    skip |= ValidateObject(buffer, kVulkanObjectTypeBuffer, false, "VUID-vkCmdDrawIndirectCount-buffer-parameter",
                           "VUID-vkCmdDrawIndirectCount-commonparent", error_obj.location.dot(Field::buffer));
    skip |= ValidateObject(countBuffer, kVulkanObjectTypeBuffer, false, "VUID-vkCmdDrawIndirectCount-countBuffer-parameter",
                           "VUID-vkCmdDrawIndirectCount-commonparent", error_obj.location.dot(Field::countBuffer));

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                    VkDeviceSize offset, VkBuffer countBuffer,
                                                                    VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                                    uint32_t stride, const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: commandBuffer: "VUID-vkCmdDrawIndexedIndirectCount-commandBuffer-parameter"
    // Checked by chassis: commandBuffer: "VUID-vkCmdDrawIndexedIndirectCount-commonparent"
    skip |= ValidateObject(buffer, kVulkanObjectTypeBuffer, false, "VUID-vkCmdDrawIndexedIndirectCount-buffer-parameter",
                           "VUID-vkCmdDrawIndexedIndirectCount-commonparent", error_obj.location.dot(Field::buffer));
    skip |= ValidateObject(countBuffer, kVulkanObjectTypeBuffer, false, "VUID-vkCmdDrawIndexedIndirectCount-countBuffer-parameter",
                           "VUID-vkCmdDrawIndexedIndirectCount-commonparent", error_obj.location.dot(Field::countBuffer));

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetSemaphoreCounterValueKHR(VkDevice device, VkSemaphore semaphore, uint64_t* pValue,
                                                                 const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkGetSemaphoreCounterValue-device-parameter"
    skip |= ValidateObject(semaphore, kVulkanObjectTypeSemaphore, false, "VUID-vkGetSemaphoreCounterValue-semaphore-parameter",
                           "VUID-vkGetSemaphoreCounterValue-semaphore-parent", error_obj.location.dot(Field::semaphore));

    return skip;
}

bool ObjectLifetimes::PreCallValidateWaitSemaphoresKHR(VkDevice device, const VkSemaphoreWaitInfo* pWaitInfo, uint64_t timeout,
                                                       const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkWaitSemaphores-device-parameter"
    if (pWaitInfo) {
        [[maybe_unused]] const Location pWaitInfo_loc = error_obj.location.dot(Field::pWaitInfo);

        if ((pWaitInfo->semaphoreCount > 0) && (pWaitInfo->pSemaphores)) {
            for (uint32_t index1 = 0; index1 < pWaitInfo->semaphoreCount; ++index1) {
                skip |= ValidateObject(pWaitInfo->pSemaphores[index1], kVulkanObjectTypeSemaphore, false,
                                       "VUID-VkSemaphoreWaitInfo-pSemaphores-parameter", kVUIDUndefined,
                                       pWaitInfo_loc.dot(Field::pSemaphores, index1));
            }
        }
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateSignalSemaphoreKHR(VkDevice device, const VkSemaphoreSignalInfo* pSignalInfo,
                                                        const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkSignalSemaphore-device-parameter"
    if (pSignalInfo) {
        [[maybe_unused]] const Location pSignalInfo_loc = error_obj.location.dot(Field::pSignalInfo);
        skip |=
            ValidateObject(pSignalInfo->semaphore, kVulkanObjectTypeSemaphore, false,
                           "VUID-VkSemaphoreSignalInfo-semaphore-parameter", kVUIDUndefined, pSignalInfo_loc.dot(Field::semaphore));
    }

    return skip;
}

// vkGetPhysicalDeviceFragmentShadingRatesKHR:
// Checked by chassis: physicalDevice: "VUID-vkGetPhysicalDeviceFragmentShadingRatesKHR-physicalDevice-parameter"

// vkCmdSetFragmentShadingRateKHR:
// Checked by chassis: commandBuffer: "VUID-vkCmdSetFragmentShadingRateKHR-commandBuffer-parameter"

bool ObjectLifetimes::PreCallValidateWaitForPresentKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t presentId,
                                                       uint64_t timeout, const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkWaitForPresentKHR-device-parameter"
    skip |= ValidateObject(swapchain, kVulkanObjectTypeSwapchainKHR, false, "VUID-vkWaitForPresentKHR-swapchain-parameter",
                           "VUID-vkWaitForPresentKHR-swapchain-parent", error_obj.location.dot(Field::swapchain));

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetBufferDeviceAddressKHR(VkDevice device, const VkBufferDeviceAddressInfo* pInfo,
                                                               const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkGetBufferDeviceAddress-device-parameter"
    if (pInfo) {
        [[maybe_unused]] const Location pInfo_loc = error_obj.location.dot(Field::pInfo);
        skip |= ValidateObject(pInfo->buffer, kVulkanObjectTypeBuffer, false, "VUID-VkBufferDeviceAddressInfo-buffer-parameter",
                               kVUIDUndefined, pInfo_loc.dot(Field::buffer));
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetBufferOpaqueCaptureAddressKHR(VkDevice device, const VkBufferDeviceAddressInfo* pInfo,
                                                                      const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkGetBufferOpaqueCaptureAddress-device-parameter"
    if (pInfo) {
        [[maybe_unused]] const Location pInfo_loc = error_obj.location.dot(Field::pInfo);
        skip |= ValidateObject(pInfo->buffer, kVulkanObjectTypeBuffer, false, "VUID-VkBufferDeviceAddressInfo-buffer-parameter",
                               kVUIDUndefined, pInfo_loc.dot(Field::buffer));
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetDeviceMemoryOpaqueCaptureAddressKHR(VkDevice device,
                                                                            const VkDeviceMemoryOpaqueCaptureAddressInfo* pInfo,
                                                                            const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkGetDeviceMemoryOpaqueCaptureAddress-device-parameter"
    if (pInfo) {
        [[maybe_unused]] const Location pInfo_loc = error_obj.location.dot(Field::pInfo);
        skip |= ValidateObject(pInfo->memory, kVulkanObjectTypeDeviceMemory, false,
                               "VUID-VkDeviceMemoryOpaqueCaptureAddressInfo-memory-parameter", kVUIDUndefined,
                               pInfo_loc.dot(Field::memory));
    }

    return skip;
}

// vkCreateDeferredOperationKHR:
// Checked by chassis: device: "VUID-vkCreateDeferredOperationKHR-device-parameter"

void ObjectLifetimes::PostCallRecordCreateDeferredOperationKHR(VkDevice device, const VkAllocationCallbacks* pAllocator,
                                                               VkDeferredOperationKHR* pDeferredOperation,
                                                               const RecordObject& record_obj) {
    if (record_obj.result != VK_SUCCESS) return;
    CreateObject(*pDeferredOperation, kVulkanObjectTypeDeferredOperationKHR, pAllocator);
}

bool ObjectLifetimes::PreCallValidateDestroyDeferredOperationKHR(VkDevice device, VkDeferredOperationKHR operation,
                                                                 const VkAllocationCallbacks* pAllocator,
                                                                 const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkDestroyDeferredOperationKHR-device-parameter"
    skip |= ValidateObject(operation, kVulkanObjectTypeDeferredOperationKHR, true,
                           "VUID-vkDestroyDeferredOperationKHR-operation-parameter",
                           "VUID-vkDestroyDeferredOperationKHR-operation-parent", error_obj.location.dot(Field::operation));
    skip |= ValidateDestroyObject(operation, kVulkanObjectTypeDeferredOperationKHR, pAllocator, kVUIDUndefined, kVUIDUndefined,
                                  error_obj.location);

    return skip;
}

void ObjectLifetimes::PreCallRecordDestroyDeferredOperationKHR(VkDevice device, VkDeferredOperationKHR operation,
                                                               const VkAllocationCallbacks* pAllocator) {
    RecordDestroyObject(operation, kVulkanObjectTypeDeferredOperationKHR);
}

bool ObjectLifetimes::PreCallValidateGetDeferredOperationMaxConcurrencyKHR(VkDevice device, VkDeferredOperationKHR operation,
                                                                           const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkGetDeferredOperationMaxConcurrencyKHR-device-parameter"
    skip |= ValidateObject(
        operation, kVulkanObjectTypeDeferredOperationKHR, false, "VUID-vkGetDeferredOperationMaxConcurrencyKHR-operation-parameter",
        "VUID-vkGetDeferredOperationMaxConcurrencyKHR-operation-parent", error_obj.location.dot(Field::operation));

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetDeferredOperationResultKHR(VkDevice device, VkDeferredOperationKHR operation,
                                                                   const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkGetDeferredOperationResultKHR-device-parameter"
    skip |= ValidateObject(operation, kVulkanObjectTypeDeferredOperationKHR, false,
                           "VUID-vkGetDeferredOperationResultKHR-operation-parameter",
                           "VUID-vkGetDeferredOperationResultKHR-operation-parent", error_obj.location.dot(Field::operation));

    return skip;
}

bool ObjectLifetimes::PreCallValidateDeferredOperationJoinKHR(VkDevice device, VkDeferredOperationKHR operation,
                                                              const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkDeferredOperationJoinKHR-device-parameter"
    skip |= ValidateObject(operation, kVulkanObjectTypeDeferredOperationKHR, false,
                           "VUID-vkDeferredOperationJoinKHR-operation-parameter",
                           "VUID-vkDeferredOperationJoinKHR-operation-parent", error_obj.location.dot(Field::operation));

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetPipelineExecutablePropertiesKHR(VkDevice device, const VkPipelineInfoKHR* pPipelineInfo,
                                                                        uint32_t* pExecutableCount,
                                                                        VkPipelineExecutablePropertiesKHR* pProperties,
                                                                        const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkGetPipelineExecutablePropertiesKHR-device-parameter"
    if (pPipelineInfo) {
        [[maybe_unused]] const Location pPipelineInfo_loc = error_obj.location.dot(Field::pPipelineInfo);
        skip |= ValidateObject(pPipelineInfo->pipeline, kVulkanObjectTypePipeline, false,
                               "VUID-VkPipelineInfoKHR-pipeline-parameter", kVUIDUndefined, pPipelineInfo_loc.dot(Field::pipeline));
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetPipelineExecutableStatisticsKHR(VkDevice device,
                                                                        const VkPipelineExecutableInfoKHR* pExecutableInfo,
                                                                        uint32_t* pStatisticCount,
                                                                        VkPipelineExecutableStatisticKHR* pStatistics,
                                                                        const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkGetPipelineExecutableStatisticsKHR-device-parameter"
    if (pExecutableInfo) {
        [[maybe_unused]] const Location pExecutableInfo_loc = error_obj.location.dot(Field::pExecutableInfo);
        skip |= ValidateObject(pExecutableInfo->pipeline, kVulkanObjectTypePipeline, false,
                               "VUID-VkPipelineExecutableInfoKHR-pipeline-parameter", kVUIDUndefined,
                               pExecutableInfo_loc.dot(Field::pipeline));
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetPipelineExecutableInternalRepresentationsKHR(
    VkDevice device, const VkPipelineExecutableInfoKHR* pExecutableInfo, uint32_t* pInternalRepresentationCount,
    VkPipelineExecutableInternalRepresentationKHR* pInternalRepresentations, const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkGetPipelineExecutableInternalRepresentationsKHR-device-parameter"
    if (pExecutableInfo) {
        [[maybe_unused]] const Location pExecutableInfo_loc = error_obj.location.dot(Field::pExecutableInfo);
        skip |= ValidateObject(pExecutableInfo->pipeline, kVulkanObjectTypePipeline, false,
                               "VUID-VkPipelineExecutableInfoKHR-pipeline-parameter", kVUIDUndefined,
                               pExecutableInfo_loc.dot(Field::pipeline));
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateMapMemory2KHR(VkDevice device, const VkMemoryMapInfoKHR* pMemoryMapInfo, void** ppData,
                                                   const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkMapMemory2KHR-device-parameter"
    if (pMemoryMapInfo) {
        [[maybe_unused]] const Location pMemoryMapInfo_loc = error_obj.location.dot(Field::pMemoryMapInfo);
        skip |= ValidateObject(pMemoryMapInfo->memory, kVulkanObjectTypeDeviceMemory, false,
                               "VUID-VkMemoryMapInfoKHR-memory-parameter", kVUIDUndefined, pMemoryMapInfo_loc.dot(Field::memory));
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateUnmapMemory2KHR(VkDevice device, const VkMemoryUnmapInfoKHR* pMemoryUnmapInfo,
                                                     const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkUnmapMemory2KHR-device-parameter"
    if (pMemoryUnmapInfo) {
        [[maybe_unused]] const Location pMemoryUnmapInfo_loc = error_obj.location.dot(Field::pMemoryUnmapInfo);
        skip |=
            ValidateObject(pMemoryUnmapInfo->memory, kVulkanObjectTypeDeviceMemory, false,
                           "VUID-VkMemoryUnmapInfoKHR-memory-parameter", kVUIDUndefined, pMemoryUnmapInfo_loc.dot(Field::memory));
    }

    return skip;
}
#ifdef VK_ENABLE_BETA_EXTENSIONS

// vkGetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR:
// Checked by chassis: physicalDevice: "VUID-vkGetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR-physicalDevice-parameter"

#endif  // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS

bool ObjectLifetimes::PreCallValidateGetEncodedVideoSessionParametersKHR(
    VkDevice device, const VkVideoEncodeSessionParametersGetInfoKHR* pVideoSessionParametersInfo,
    VkVideoEncodeSessionParametersFeedbackInfoKHR* pFeedbackInfo, size_t* pDataSize, void* pData,
    const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkGetEncodedVideoSessionParametersKHR-device-parameter"
    if (pVideoSessionParametersInfo) {
        [[maybe_unused]] const Location pVideoSessionParametersInfo_loc =
            error_obj.location.dot(Field::pVideoSessionParametersInfo);
        skip |= ValidateObject(pVideoSessionParametersInfo->videoSessionParameters, kVulkanObjectTypeVideoSessionParametersKHR,
                               false, "VUID-VkVideoEncodeSessionParametersGetInfoKHR-videoSessionParameters-parameter",
                               kVUIDUndefined, pVideoSessionParametersInfo_loc.dot(Field::videoSessionParameters));
    }

    return skip;
}
#endif  // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS

bool ObjectLifetimes::PreCallValidateCmdEncodeVideoKHR(VkCommandBuffer commandBuffer, const VkVideoEncodeInfoKHR* pEncodeInfo,
                                                       const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: commandBuffer: "VUID-vkCmdEncodeVideoKHR-commandBuffer-parameter"
    if (pEncodeInfo) {
        [[maybe_unused]] const Location pEncodeInfo_loc = error_obj.location.dot(Field::pEncodeInfo);
        skip |=
            ValidateObject(pEncodeInfo->dstBuffer, kVulkanObjectTypeBuffer, false, "VUID-VkVideoEncodeInfoKHR-dstBuffer-parameter",
                           kVUIDUndefined, pEncodeInfo_loc.dot(Field::dstBuffer));
        [[maybe_unused]] const Location srcPictureResource_loc = pEncodeInfo_loc.dot(Field::srcPictureResource);
        skip |= ValidateObject(pEncodeInfo->srcPictureResource.imageViewBinding, kVulkanObjectTypeImageView, false,
                               "VUID-VkVideoPictureResourceInfoKHR-imageViewBinding-parameter", kVUIDUndefined,
                               srcPictureResource_loc.dot(Field::imageViewBinding));
        if (pEncodeInfo->pSetupReferenceSlot) {
            [[maybe_unused]] const Location pSetupReferenceSlot_loc = pEncodeInfo_loc.dot(Field::pSetupReferenceSlot);
            if (pEncodeInfo->pSetupReferenceSlot->pPictureResource) {
                [[maybe_unused]] const Location pPictureResource_loc = pSetupReferenceSlot_loc.dot(Field::pPictureResource);
                skip |=
                    ValidateObject(pEncodeInfo->pSetupReferenceSlot->pPictureResource->imageViewBinding, kVulkanObjectTypeImageView,
                                   false, "VUID-VkVideoPictureResourceInfoKHR-imageViewBinding-parameter", kVUIDUndefined,
                                   pPictureResource_loc.dot(Field::imageViewBinding));
            }
        }
        if (pEncodeInfo->pReferenceSlots) {
            for (uint32_t index1 = 0; index1 < pEncodeInfo->referenceSlotCount; ++index1) {
                [[maybe_unused]] const Location index1_loc = pEncodeInfo_loc.dot(Field::pReferenceSlots, index1);
                if (pEncodeInfo->pReferenceSlots[index1].pPictureResource) {
                    [[maybe_unused]] const Location pPictureResource_loc = index1_loc.dot(Field::pPictureResource);
                    skip |= ValidateObject(pEncodeInfo->pReferenceSlots[index1].pPictureResource->imageViewBinding,
                                           kVulkanObjectTypeImageView, false,
                                           "VUID-VkVideoPictureResourceInfoKHR-imageViewBinding-parameter", kVUIDUndefined,
                                           pPictureResource_loc.dot(Field::imageViewBinding));
                }
            }
        }
    }

    return skip;
}
#endif  // VK_ENABLE_BETA_EXTENSIONS

bool ObjectLifetimes::PreCallValidateCmdSetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event,
                                                     const VkDependencyInfo* pDependencyInfo, const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: commandBuffer: "VUID-vkCmdSetEvent2-commandBuffer-parameter"
    // Checked by chassis: commandBuffer: "VUID-vkCmdSetEvent2-commonparent"
    skip |= ValidateObject(event, kVulkanObjectTypeEvent, false, "VUID-vkCmdSetEvent2-event-parameter",
                           "VUID-vkCmdSetEvent2-commonparent", error_obj.location.dot(Field::event));
    if (pDependencyInfo) {
        [[maybe_unused]] const Location pDependencyInfo_loc = error_obj.location.dot(Field::pDependencyInfo);
        if (pDependencyInfo->pBufferMemoryBarriers) {
            for (uint32_t index1 = 0; index1 < pDependencyInfo->bufferMemoryBarrierCount; ++index1) {
                [[maybe_unused]] const Location index1_loc = pDependencyInfo_loc.dot(Field::pBufferMemoryBarriers, index1);
                skip |=
                    ValidateObject(pDependencyInfo->pBufferMemoryBarriers[index1].buffer, kVulkanObjectTypeBuffer, false,
                                   "VUID-VkBufferMemoryBarrier2-buffer-parameter", kVUIDUndefined, index1_loc.dot(Field::buffer));
            }
        }
        if (pDependencyInfo->pImageMemoryBarriers) {
            for (uint32_t index1 = 0; index1 < pDependencyInfo->imageMemoryBarrierCount; ++index1) {
                [[maybe_unused]] const Location index1_loc = pDependencyInfo_loc.dot(Field::pImageMemoryBarriers, index1);
                skip |= ValidateObject(pDependencyInfo->pImageMemoryBarriers[index1].image, kVulkanObjectTypeImage, false,
                                       "VUID-VkImageMemoryBarrier2-image-parameter", kVUIDUndefined, index1_loc.dot(Field::image));
            }
        }
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdResetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event,
                                                       VkPipelineStageFlags2 stageMask, const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: commandBuffer: "VUID-vkCmdResetEvent2-commandBuffer-parameter"
    // Checked by chassis: commandBuffer: "VUID-vkCmdResetEvent2-commonparent"
    skip |= ValidateObject(event, kVulkanObjectTypeEvent, false, "VUID-vkCmdResetEvent2-event-parameter",
                           "VUID-vkCmdResetEvent2-commonparent", error_obj.location.dot(Field::event));

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdWaitEvents2KHR(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                                       const VkDependencyInfo* pDependencyInfos,
                                                       const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: commandBuffer: "VUID-vkCmdWaitEvents2-commandBuffer-parameter"
    // Checked by chassis: commandBuffer: "VUID-vkCmdWaitEvents2-commonparent"

    if ((eventCount > 0) && (pEvents)) {
        for (uint32_t index0 = 0; index0 < eventCount; ++index0) {
            skip |= ValidateObject(pEvents[index0], kVulkanObjectTypeEvent, false, "VUID-vkCmdWaitEvents2-pEvents-parameter",
                                   "VUID-vkCmdWaitEvents2-commonparent", error_obj.location.dot(Field::pEvents, index0));
        }
    }
    if (pDependencyInfos) {
        for (uint32_t index0 = 0; index0 < eventCount; ++index0) {
            [[maybe_unused]] const Location index0_loc = error_obj.location.dot(Field::pDependencyInfos, index0);
            if (pDependencyInfos[index0].pBufferMemoryBarriers) {
                for (uint32_t index1 = 0; index1 < pDependencyInfos[index0].bufferMemoryBarrierCount; ++index1) {
                    [[maybe_unused]] const Location index1_loc = index0_loc.dot(Field::pBufferMemoryBarriers, index1);
                    skip |= ValidateObject(pDependencyInfos[index0].pBufferMemoryBarriers[index1].buffer, kVulkanObjectTypeBuffer,
                                           false, "VUID-VkBufferMemoryBarrier2-buffer-parameter", kVUIDUndefined,
                                           index1_loc.dot(Field::buffer));
                }
            }
            if (pDependencyInfos[index0].pImageMemoryBarriers) {
                for (uint32_t index1 = 0; index1 < pDependencyInfos[index0].imageMemoryBarrierCount; ++index1) {
                    [[maybe_unused]] const Location index1_loc = index0_loc.dot(Field::pImageMemoryBarriers, index1);
                    skip |=
                        ValidateObject(pDependencyInfos[index0].pImageMemoryBarriers[index1].image, kVulkanObjectTypeImage, false,
                                       "VUID-VkImageMemoryBarrier2-image-parameter", kVUIDUndefined, index1_loc.dot(Field::image));
                }
            }
        }
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdPipelineBarrier2KHR(VkCommandBuffer commandBuffer, const VkDependencyInfo* pDependencyInfo,
                                                            const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: commandBuffer: "VUID-vkCmdPipelineBarrier2-commandBuffer-parameter"
    if (pDependencyInfo) {
        [[maybe_unused]] const Location pDependencyInfo_loc = error_obj.location.dot(Field::pDependencyInfo);
        if (pDependencyInfo->pBufferMemoryBarriers) {
            for (uint32_t index1 = 0; index1 < pDependencyInfo->bufferMemoryBarrierCount; ++index1) {
                [[maybe_unused]] const Location index1_loc = pDependencyInfo_loc.dot(Field::pBufferMemoryBarriers, index1);
                skip |=
                    ValidateObject(pDependencyInfo->pBufferMemoryBarriers[index1].buffer, kVulkanObjectTypeBuffer, false,
                                   "VUID-VkBufferMemoryBarrier2-buffer-parameter", kVUIDUndefined, index1_loc.dot(Field::buffer));
            }
        }
        if (pDependencyInfo->pImageMemoryBarriers) {
            for (uint32_t index1 = 0; index1 < pDependencyInfo->imageMemoryBarrierCount; ++index1) {
                [[maybe_unused]] const Location index1_loc = pDependencyInfo_loc.dot(Field::pImageMemoryBarriers, index1);
                skip |= ValidateObject(pDependencyInfo->pImageMemoryBarriers[index1].image, kVulkanObjectTypeImage, false,
                                       "VUID-VkImageMemoryBarrier2-image-parameter", kVUIDUndefined, index1_loc.dot(Field::image));
            }
        }
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdWriteTimestamp2KHR(VkCommandBuffer commandBuffer, VkPipelineStageFlags2 stage,
                                                           VkQueryPool queryPool, uint32_t query,
                                                           const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: commandBuffer: "VUID-vkCmdWriteTimestamp2-commandBuffer-parameter"
    // Checked by chassis: commandBuffer: "VUID-vkCmdWriteTimestamp2-commonparent"
    skip |= ValidateObject(queryPool, kVulkanObjectTypeQueryPool, false, "VUID-vkCmdWriteTimestamp2-queryPool-parameter",
                           "VUID-vkCmdWriteTimestamp2-commonparent", error_obj.location.dot(Field::queryPool));

    return skip;
}

bool ObjectLifetimes::PreCallValidateQueueSubmit2KHR(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2* pSubmits,
                                                     VkFence fence, const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: queue: "VUID-vkQueueSubmit2-queue-parameter"
    // Checked by chassis: queue: "VUID-vkQueueSubmit2-commonparent"
    if (pSubmits) {
        for (uint32_t index0 = 0; index0 < submitCount; ++index0) {
            [[maybe_unused]] const Location index0_loc = error_obj.location.dot(Field::pSubmits, index0);
            if (pSubmits[index0].pWaitSemaphoreInfos) {
                for (uint32_t index1 = 0; index1 < pSubmits[index0].waitSemaphoreInfoCount; ++index1) {
                    [[maybe_unused]] const Location index1_loc = index0_loc.dot(Field::pWaitSemaphoreInfos, index1);
                    skip |= ValidateObject(pSubmits[index0].pWaitSemaphoreInfos[index1].semaphore, kVulkanObjectTypeSemaphore,
                                           false, "VUID-VkSemaphoreSubmitInfo-semaphore-parameter", kVUIDUndefined,
                                           index1_loc.dot(Field::semaphore));
                }
            }
            if (pSubmits[index0].pCommandBufferInfos) {
                for (uint32_t index1 = 0; index1 < pSubmits[index0].commandBufferInfoCount; ++index1) {
                    [[maybe_unused]] const Location index1_loc = index0_loc.dot(Field::pCommandBufferInfos, index1);
                    skip |=
                        ValidateObject(pSubmits[index0].pCommandBufferInfos[index1].commandBuffer, kVulkanObjectTypeCommandBuffer,
                                       false, "VUID-VkCommandBufferSubmitInfo-commandBuffer-parameter", kVUIDUndefined,
                                       index1_loc.dot(Field::commandBuffer));
                }
            }
            if (pSubmits[index0].pSignalSemaphoreInfos) {
                for (uint32_t index1 = 0; index1 < pSubmits[index0].signalSemaphoreInfoCount; ++index1) {
                    [[maybe_unused]] const Location index1_loc = index0_loc.dot(Field::pSignalSemaphoreInfos, index1);
                    skip |= ValidateObject(pSubmits[index0].pSignalSemaphoreInfos[index1].semaphore, kVulkanObjectTypeSemaphore,
                                           false, "VUID-VkSemaphoreSubmitInfo-semaphore-parameter", kVUIDUndefined,
                                           index1_loc.dot(Field::semaphore));
                }
            }
            if (auto pNext = vku::FindStructInPNextChain<VkFrameBoundaryEXT>(pSubmits[index0].pNext)) {
                const Location pNext_loc = index0_loc.pNext(Struct::VkFrameBoundaryEXT);

                if ((pNext->imageCount > 0) && (pNext->pImages)) {
                    for (uint32_t index2 = 0; index2 < pNext->imageCount; ++index2) {
                        skip |= ValidateObject(pNext->pImages[index2], kVulkanObjectTypeImage, false,
                                               "VUID-VkFrameBoundaryEXT-pImages-parameter", "VUID-VkFrameBoundaryEXT-commonparent",
                                               pNext_loc.dot(Field::pImages, index2));
                    }
                }

                if ((pNext->bufferCount > 0) && (pNext->pBuffers)) {
                    for (uint32_t index2 = 0; index2 < pNext->bufferCount; ++index2) {
                        skip |= ValidateObject(pNext->pBuffers[index2], kVulkanObjectTypeBuffer, false,
                                               "VUID-VkFrameBoundaryEXT-pBuffers-parameter", "VUID-VkFrameBoundaryEXT-commonparent",
                                               pNext_loc.dot(Field::pBuffers, index2));
                    }
                }
            }
#ifdef VK_USE_PLATFORM_WIN32_KHR
            if (auto pNext = vku::FindStructInPNextChain<VkWin32KeyedMutexAcquireReleaseInfoKHR>(pSubmits[index0].pNext)) {
                const Location pNext_loc = index0_loc.pNext(Struct::VkWin32KeyedMutexAcquireReleaseInfoKHR);

                if ((pNext->acquireCount > 0) && (pNext->pAcquireSyncs)) {
                    for (uint32_t index2 = 0; index2 < pNext->acquireCount; ++index2) {
                        skip |= ValidateObject(pNext->pAcquireSyncs[index2], kVulkanObjectTypeDeviceMemory, false,
                                               "VUID-VkWin32KeyedMutexAcquireReleaseInfoKHR-pAcquireSyncs-parameter",
                                               "VUID-VkWin32KeyedMutexAcquireReleaseInfoKHR-commonparent",
                                               pNext_loc.dot(Field::pAcquireSyncs, index2));
                    }
                }

                if ((pNext->releaseCount > 0) && (pNext->pReleaseSyncs)) {
                    for (uint32_t index2 = 0; index2 < pNext->releaseCount; ++index2) {
                        skip |= ValidateObject(pNext->pReleaseSyncs[index2], kVulkanObjectTypeDeviceMemory, false,
                                               "VUID-VkWin32KeyedMutexAcquireReleaseInfoKHR-pReleaseSyncs-parameter",
                                               "VUID-VkWin32KeyedMutexAcquireReleaseInfoKHR-commonparent",
                                               pNext_loc.dot(Field::pReleaseSyncs, index2));
                    }
                }
            }
#endif  // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
            if (auto pNext = vku::FindStructInPNextChain<VkWin32KeyedMutexAcquireReleaseInfoNV>(pSubmits[index0].pNext)) {
                const Location pNext_loc = index0_loc.pNext(Struct::VkWin32KeyedMutexAcquireReleaseInfoNV);

                if ((pNext->acquireCount > 0) && (pNext->pAcquireSyncs)) {
                    for (uint32_t index2 = 0; index2 < pNext->acquireCount; ++index2) {
                        skip |= ValidateObject(pNext->pAcquireSyncs[index2], kVulkanObjectTypeDeviceMemory, false,
                                               "VUID-VkWin32KeyedMutexAcquireReleaseInfoNV-pAcquireSyncs-parameter",
                                               "VUID-VkWin32KeyedMutexAcquireReleaseInfoNV-commonparent",
                                               pNext_loc.dot(Field::pAcquireSyncs, index2));
                    }
                }

                if ((pNext->releaseCount > 0) && (pNext->pReleaseSyncs)) {
                    for (uint32_t index2 = 0; index2 < pNext->releaseCount; ++index2) {
                        skip |= ValidateObject(pNext->pReleaseSyncs[index2], kVulkanObjectTypeDeviceMemory, false,
                                               "VUID-VkWin32KeyedMutexAcquireReleaseInfoNV-pReleaseSyncs-parameter",
                                               "VUID-VkWin32KeyedMutexAcquireReleaseInfoNV-commonparent",
                                               pNext_loc.dot(Field::pReleaseSyncs, index2));
                    }
                }
            }
#endif  // VK_USE_PLATFORM_WIN32_KHR
        }
    }
    skip |= ValidateObject(fence, kVulkanObjectTypeFence, true, "VUID-vkQueueSubmit2-fence-parameter",
                           "VUID-vkQueueSubmit2-commonparent", error_obj.location.dot(Field::fence));

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdWriteBufferMarker2AMD(VkCommandBuffer commandBuffer, VkPipelineStageFlags2 stage,
                                                              VkBuffer dstBuffer, VkDeviceSize dstOffset, uint32_t marker,
                                                              const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: commandBuffer: "VUID-vkCmdWriteBufferMarker2AMD-commandBuffer-parameter"
    // Checked by chassis: commandBuffer: "VUID-vkCmdWriteBufferMarker2AMD-commonparent"
    skip |= ValidateObject(dstBuffer, kVulkanObjectTypeBuffer, false, "VUID-vkCmdWriteBufferMarker2AMD-dstBuffer-parameter",
                           "VUID-vkCmdWriteBufferMarker2AMD-commonparent", error_obj.location.dot(Field::dstBuffer));

    return skip;
}

// vkGetQueueCheckpointData2NV:
// Checked by chassis: queue: "VUID-vkGetQueueCheckpointData2NV-queue-parameter"

bool ObjectLifetimes::PreCallValidateCmdCopyBuffer2KHR(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2* pCopyBufferInfo,
                                                       const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: commandBuffer: "VUID-vkCmdCopyBuffer2-commandBuffer-parameter"
    if (pCopyBufferInfo) {
        [[maybe_unused]] const Location pCopyBufferInfo_loc = error_obj.location.dot(Field::pCopyBufferInfo);
        skip |=
            ValidateObject(pCopyBufferInfo->srcBuffer, kVulkanObjectTypeBuffer, false, "VUID-VkCopyBufferInfo2-srcBuffer-parameter",
                           "VUID-VkCopyBufferInfo2-commonparent", pCopyBufferInfo_loc.dot(Field::srcBuffer));
        skip |=
            ValidateObject(pCopyBufferInfo->dstBuffer, kVulkanObjectTypeBuffer, false, "VUID-VkCopyBufferInfo2-dstBuffer-parameter",
                           "VUID-VkCopyBufferInfo2-commonparent", pCopyBufferInfo_loc.dot(Field::dstBuffer));
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdCopyImage2KHR(VkCommandBuffer commandBuffer, const VkCopyImageInfo2* pCopyImageInfo,
                                                      const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: commandBuffer: "VUID-vkCmdCopyImage2-commandBuffer-parameter"
    if (pCopyImageInfo) {
        [[maybe_unused]] const Location pCopyImageInfo_loc = error_obj.location.dot(Field::pCopyImageInfo);
        skip |= ValidateObject(pCopyImageInfo->srcImage, kVulkanObjectTypeImage, false, "VUID-VkCopyImageInfo2-srcImage-parameter",
                               "VUID-VkCopyImageInfo2-commonparent", pCopyImageInfo_loc.dot(Field::srcImage));
        skip |= ValidateObject(pCopyImageInfo->dstImage, kVulkanObjectTypeImage, false, "VUID-VkCopyImageInfo2-dstImage-parameter",
                               "VUID-VkCopyImageInfo2-commonparent", pCopyImageInfo_loc.dot(Field::dstImage));
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdCopyBufferToImage2KHR(VkCommandBuffer commandBuffer,
                                                              const VkCopyBufferToImageInfo2* pCopyBufferToImageInfo,
                                                              const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: commandBuffer: "VUID-vkCmdCopyBufferToImage2-commandBuffer-parameter"
    if (pCopyBufferToImageInfo) {
        [[maybe_unused]] const Location pCopyBufferToImageInfo_loc = error_obj.location.dot(Field::pCopyBufferToImageInfo);
        skip |= ValidateObject(pCopyBufferToImageInfo->srcBuffer, kVulkanObjectTypeBuffer, false,
                               "VUID-VkCopyBufferToImageInfo2-srcBuffer-parameter", "VUID-VkCopyBufferToImageInfo2-commonparent",
                               pCopyBufferToImageInfo_loc.dot(Field::srcBuffer));
        skip |= ValidateObject(pCopyBufferToImageInfo->dstImage, kVulkanObjectTypeImage, false,
                               "VUID-VkCopyBufferToImageInfo2-dstImage-parameter", "VUID-VkCopyBufferToImageInfo2-commonparent",
                               pCopyBufferToImageInfo_loc.dot(Field::dstImage));
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdCopyImageToBuffer2KHR(VkCommandBuffer commandBuffer,
                                                              const VkCopyImageToBufferInfo2* pCopyImageToBufferInfo,
                                                              const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: commandBuffer: "VUID-vkCmdCopyImageToBuffer2-commandBuffer-parameter"
    if (pCopyImageToBufferInfo) {
        [[maybe_unused]] const Location pCopyImageToBufferInfo_loc = error_obj.location.dot(Field::pCopyImageToBufferInfo);
        skip |= ValidateObject(pCopyImageToBufferInfo->srcImage, kVulkanObjectTypeImage, false,
                               "VUID-VkCopyImageToBufferInfo2-srcImage-parameter", "VUID-VkCopyImageToBufferInfo2-commonparent",
                               pCopyImageToBufferInfo_loc.dot(Field::srcImage));
        skip |= ValidateObject(pCopyImageToBufferInfo->dstBuffer, kVulkanObjectTypeBuffer, false,
                               "VUID-VkCopyImageToBufferInfo2-dstBuffer-parameter", "VUID-VkCopyImageToBufferInfo2-commonparent",
                               pCopyImageToBufferInfo_loc.dot(Field::dstBuffer));
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdBlitImage2KHR(VkCommandBuffer commandBuffer, const VkBlitImageInfo2* pBlitImageInfo,
                                                      const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: commandBuffer: "VUID-vkCmdBlitImage2-commandBuffer-parameter"
    if (pBlitImageInfo) {
        [[maybe_unused]] const Location pBlitImageInfo_loc = error_obj.location.dot(Field::pBlitImageInfo);
        skip |= ValidateObject(pBlitImageInfo->srcImage, kVulkanObjectTypeImage, false, "VUID-VkBlitImageInfo2-srcImage-parameter",
                               "VUID-VkBlitImageInfo2-commonparent", pBlitImageInfo_loc.dot(Field::srcImage));
        skip |= ValidateObject(pBlitImageInfo->dstImage, kVulkanObjectTypeImage, false, "VUID-VkBlitImageInfo2-dstImage-parameter",
                               "VUID-VkBlitImageInfo2-commonparent", pBlitImageInfo_loc.dot(Field::dstImage));
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdResolveImage2KHR(VkCommandBuffer commandBuffer,
                                                         const VkResolveImageInfo2* pResolveImageInfo,
                                                         const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: commandBuffer: "VUID-vkCmdResolveImage2-commandBuffer-parameter"
    if (pResolveImageInfo) {
        [[maybe_unused]] const Location pResolveImageInfo_loc = error_obj.location.dot(Field::pResolveImageInfo);
        skip |= ValidateObject(pResolveImageInfo->srcImage, kVulkanObjectTypeImage, false,
                               "VUID-VkResolveImageInfo2-srcImage-parameter", "VUID-VkResolveImageInfo2-commonparent",
                               pResolveImageInfo_loc.dot(Field::srcImage));
        skip |= ValidateObject(pResolveImageInfo->dstImage, kVulkanObjectTypeImage, false,
                               "VUID-VkResolveImageInfo2-dstImage-parameter", "VUID-VkResolveImageInfo2-commonparent",
                               pResolveImageInfo_loc.dot(Field::dstImage));
    }

    return skip;
}

// vkCmdTraceRaysIndirect2KHR:
// Checked by chassis: commandBuffer: "VUID-vkCmdTraceRaysIndirect2KHR-commandBuffer-parameter"

// vkGetDeviceBufferMemoryRequirementsKHR:
// Checked by chassis: device: "VUID-vkGetDeviceBufferMemoryRequirements-device-parameter"

// vkGetDeviceImageMemoryRequirementsKHR:
// Checked by chassis: device: "VUID-vkGetDeviceImageMemoryRequirements-device-parameter"

// vkGetDeviceImageSparseMemoryRequirementsKHR:
// Checked by chassis: device: "VUID-vkGetDeviceImageSparseMemoryRequirements-device-parameter"

bool ObjectLifetimes::PreCallValidateCmdBindIndexBuffer2KHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                            VkDeviceSize size, VkIndexType indexType,
                                                            const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: commandBuffer: "VUID-vkCmdBindIndexBuffer2KHR-commandBuffer-parameter"
    // Checked by chassis: commandBuffer: "VUID-vkCmdBindIndexBuffer2KHR-commonparent"
    skip |= ValidateObject(buffer, kVulkanObjectTypeBuffer, false, "VUID-vkCmdBindIndexBuffer2KHR-buffer-parameter",
                           "VUID-vkCmdBindIndexBuffer2KHR-commonparent", error_obj.location.dot(Field::buffer));

    return skip;
}

// vkGetRenderingAreaGranularityKHR:
// Checked by chassis: device: "VUID-vkGetRenderingAreaGranularityKHR-device-parameter"

// vkGetDeviceImageSubresourceLayoutKHR:
// Checked by chassis: device: "VUID-vkGetDeviceImageSubresourceLayoutKHR-device-parameter"

bool ObjectLifetimes::PreCallValidateGetImageSubresourceLayout2KHR(VkDevice device, VkImage image,
                                                                   const VkImageSubresource2KHR* pSubresource,
                                                                   VkSubresourceLayout2KHR* pLayout,
                                                                   const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkGetImageSubresourceLayout2KHR-device-parameter"
    skip |= ValidateObject(image, kVulkanObjectTypeImage, false, "VUID-vkGetImageSubresourceLayout2KHR-image-parameter",
                           "VUID-vkGetImageSubresourceLayout2KHR-image-parent", error_obj.location.dot(Field::image));

    return skip;
}

// vkGetPhysicalDeviceCooperativeMatrixPropertiesKHR:
// Checked by chassis: physicalDevice: "VUID-vkGetPhysicalDeviceCooperativeMatrixPropertiesKHR-physicalDevice-parameter"

// vkCreateDebugReportCallbackEXT:
// Checked by chassis: instance: "VUID-vkCreateDebugReportCallbackEXT-instance-parameter"

void ObjectLifetimes::PostCallRecordCreateDebugReportCallbackEXT(VkInstance instance,
                                                                 const VkDebugReportCallbackCreateInfoEXT* pCreateInfo,
                                                                 const VkAllocationCallbacks* pAllocator,
                                                                 VkDebugReportCallbackEXT* pCallback,
                                                                 const RecordObject& record_obj) {
    if (record_obj.result != VK_SUCCESS) return;
    CreateObject(*pCallback, kVulkanObjectTypeDebugReportCallbackEXT, pAllocator);
}

bool ObjectLifetimes::PreCallValidateDestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback,
                                                                   const VkAllocationCallbacks* pAllocator,
                                                                   const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: instance: "VUID-vkDestroyDebugReportCallbackEXT-instance-parameter"
    skip |= ValidateObject(
        callback, kVulkanObjectTypeDebugReportCallbackEXT, true, "VUID-vkDestroyDebugReportCallbackEXT-callback-parameter",
        "VUID-vkDestroyDebugReportCallbackEXT-callback-parent", error_obj.location.dot(Field::callback), kVulkanObjectTypeInstance);
    skip |= ValidateDestroyObject(callback, kVulkanObjectTypeDebugReportCallbackEXT, pAllocator, kVUIDUndefined, kVUIDUndefined,
                                  error_obj.location);

    return skip;
}

void ObjectLifetimes::PreCallRecordDestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback,
                                                                 const VkAllocationCallbacks* pAllocator) {
    RecordDestroyObject(callback, kVulkanObjectTypeDebugReportCallbackEXT);
}

// vkDebugReportMessageEXT:
// Checked by chassis: instance: "VUID-vkDebugReportMessageEXT-instance-parameter"

// vkDebugMarkerSetObjectTagEXT:
// Checked by chassis: device: "VUID-vkDebugMarkerSetObjectTagEXT-device-parameter"

// vkDebugMarkerSetObjectNameEXT:
// Checked by chassis: device: "VUID-vkDebugMarkerSetObjectNameEXT-device-parameter"

// vkCmdDebugMarkerBeginEXT:
// Checked by chassis: commandBuffer: "VUID-vkCmdDebugMarkerBeginEXT-commandBuffer-parameter"

// vkCmdDebugMarkerEndEXT:
// Checked by chassis: commandBuffer: "VUID-vkCmdDebugMarkerEndEXT-commandBuffer-parameter"

// vkCmdDebugMarkerInsertEXT:
// Checked by chassis: commandBuffer: "VUID-vkCmdDebugMarkerInsertEXT-commandBuffer-parameter"

bool ObjectLifetimes::PreCallValidateCmdBindTransformFeedbackBuffersEXT(VkCommandBuffer commandBuffer, uint32_t firstBinding,
                                                                        uint32_t bindingCount, const VkBuffer* pBuffers,
                                                                        const VkDeviceSize* pOffsets, const VkDeviceSize* pSizes,
                                                                        const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: commandBuffer: "VUID-vkCmdBindTransformFeedbackBuffersEXT-commandBuffer-parameter"
    // Checked by chassis: commandBuffer: "VUID-vkCmdBindTransformFeedbackBuffersEXT-commonparent"

    if ((bindingCount > 0) && (pBuffers)) {
        for (uint32_t index0 = 0; index0 < bindingCount; ++index0) {
            skip |= ValidateObject(
                pBuffers[index0], kVulkanObjectTypeBuffer, false, "VUID-vkCmdBindTransformFeedbackBuffersEXT-pBuffers-parameter",
                "VUID-vkCmdBindTransformFeedbackBuffersEXT-commonparent", error_obj.location.dot(Field::pBuffers, index0));
        }
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdBeginTransformFeedbackEXT(VkCommandBuffer commandBuffer, uint32_t firstCounterBuffer,
                                                                  uint32_t counterBufferCount, const VkBuffer* pCounterBuffers,
                                                                  const VkDeviceSize* pCounterBufferOffsets,
                                                                  const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: commandBuffer: "VUID-vkCmdBeginTransformFeedbackEXT-commandBuffer-parameter"
    // Checked by chassis: commandBuffer: "VUID-vkCmdBeginTransformFeedbackEXT-commonparent"

    if ((counterBufferCount > 0) && (pCounterBuffers)) {
        for (uint32_t index0 = 0; index0 < counterBufferCount; ++index0) {
            skip |= ValidateObject(pCounterBuffers[index0], kVulkanObjectTypeBuffer, true, kVUIDUndefined,
                                   "VUID-vkCmdBeginTransformFeedbackEXT-commonparent",
                                   error_obj.location.dot(Field::pCounterBuffers, index0));
        }
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdEndTransformFeedbackEXT(VkCommandBuffer commandBuffer, uint32_t firstCounterBuffer,
                                                                uint32_t counterBufferCount, const VkBuffer* pCounterBuffers,
                                                                const VkDeviceSize* pCounterBufferOffsets,
                                                                const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: commandBuffer: "VUID-vkCmdEndTransformFeedbackEXT-commandBuffer-parameter"
    // Checked by chassis: commandBuffer: "VUID-vkCmdEndTransformFeedbackEXT-commonparent"

    if ((counterBufferCount > 0) && (pCounterBuffers)) {
        for (uint32_t index0 = 0; index0 < counterBufferCount; ++index0) {
            skip |= ValidateObject(pCounterBuffers[index0], kVulkanObjectTypeBuffer, true, kVUIDUndefined,
                                   "VUID-vkCmdEndTransformFeedbackEXT-commonparent",
                                   error_obj.location.dot(Field::pCounterBuffers, index0));
        }
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdBeginQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query,
                                                             VkQueryControlFlags flags, uint32_t index,
                                                             const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: commandBuffer: "VUID-vkCmdBeginQueryIndexedEXT-commandBuffer-parameter"
    // Checked by chassis: commandBuffer: "VUID-vkCmdBeginQueryIndexedEXT-commonparent"
    skip |= ValidateObject(queryPool, kVulkanObjectTypeQueryPool, false, "VUID-vkCmdBeginQueryIndexedEXT-queryPool-parameter",
                           "VUID-vkCmdBeginQueryIndexedEXT-commonparent", error_obj.location.dot(Field::queryPool));

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdEndQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query,
                                                           uint32_t index, const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: commandBuffer: "VUID-vkCmdEndQueryIndexedEXT-commandBuffer-parameter"
    // Checked by chassis: commandBuffer: "VUID-vkCmdEndQueryIndexedEXT-commonparent"
    skip |= ValidateObject(queryPool, kVulkanObjectTypeQueryPool, false, "VUID-vkCmdEndQueryIndexedEXT-queryPool-parameter",
                           "VUID-vkCmdEndQueryIndexedEXT-commonparent", error_obj.location.dot(Field::queryPool));

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdDrawIndirectByteCountEXT(VkCommandBuffer commandBuffer, uint32_t instanceCount,
                                                                 uint32_t firstInstance, VkBuffer counterBuffer,
                                                                 VkDeviceSize counterBufferOffset, uint32_t counterOffset,
                                                                 uint32_t vertexStride, const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: commandBuffer: "VUID-vkCmdDrawIndirectByteCountEXT-commandBuffer-parameter"
    // Checked by chassis: commandBuffer: "VUID-vkCmdDrawIndirectByteCountEXT-commonparent"
    skip |=
        ValidateObject(counterBuffer, kVulkanObjectTypeBuffer, false, "VUID-vkCmdDrawIndirectByteCountEXT-counterBuffer-parameter",
                       "VUID-vkCmdDrawIndirectByteCountEXT-commonparent", error_obj.location.dot(Field::counterBuffer));

    return skip;
}

// vkCreateCuModuleNVX:
// Checked by chassis: device: "VUID-vkCreateCuModuleNVX-device-parameter"

void ObjectLifetimes::PostCallRecordCreateCuModuleNVX(VkDevice device, const VkCuModuleCreateInfoNVX* pCreateInfo,
                                                      const VkAllocationCallbacks* pAllocator, VkCuModuleNVX* pModule,
                                                      const RecordObject& record_obj) {
    if (record_obj.result != VK_SUCCESS) return;
    CreateObject(*pModule, kVulkanObjectTypeCuModuleNVX, pAllocator);
}

bool ObjectLifetimes::PreCallValidateCreateCuFunctionNVX(VkDevice device, const VkCuFunctionCreateInfoNVX* pCreateInfo,
                                                         const VkAllocationCallbacks* pAllocator, VkCuFunctionNVX* pFunction,
                                                         const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkCreateCuFunctionNVX-device-parameter"
    if (pCreateInfo) {
        [[maybe_unused]] const Location pCreateInfo_loc = error_obj.location.dot(Field::pCreateInfo);
        skip |=
            ValidateObject(pCreateInfo->module, kVulkanObjectTypeCuModuleNVX, false,
                           "VUID-VkCuFunctionCreateInfoNVX-module-parameter", kVUIDUndefined, pCreateInfo_loc.dot(Field::module));
    }

    return skip;
}

void ObjectLifetimes::PostCallRecordCreateCuFunctionNVX(VkDevice device, const VkCuFunctionCreateInfoNVX* pCreateInfo,
                                                        const VkAllocationCallbacks* pAllocator, VkCuFunctionNVX* pFunction,
                                                        const RecordObject& record_obj) {
    if (record_obj.result != VK_SUCCESS) return;
    CreateObject(*pFunction, kVulkanObjectTypeCuFunctionNVX, pAllocator);
}

bool ObjectLifetimes::PreCallValidateDestroyCuModuleNVX(VkDevice device, VkCuModuleNVX module,
                                                        const VkAllocationCallbacks* pAllocator,
                                                        const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkDestroyCuModuleNVX-device-parameter"
    skip |= ValidateObject(module, kVulkanObjectTypeCuModuleNVX, false, "VUID-vkDestroyCuModuleNVX-module-parameter",
                           "VUID-vkDestroyCuModuleNVX-module-parent", error_obj.location.dot(Field::module));
    skip |=
        ValidateDestroyObject(module, kVulkanObjectTypeCuModuleNVX, pAllocator, kVUIDUndefined, kVUIDUndefined, error_obj.location);

    return skip;
}

void ObjectLifetimes::PreCallRecordDestroyCuModuleNVX(VkDevice device, VkCuModuleNVX module,
                                                      const VkAllocationCallbacks* pAllocator) {
    RecordDestroyObject(module, kVulkanObjectTypeCuModuleNVX);
}

bool ObjectLifetimes::PreCallValidateDestroyCuFunctionNVX(VkDevice device, VkCuFunctionNVX function,
                                                          const VkAllocationCallbacks* pAllocator,
                                                          const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkDestroyCuFunctionNVX-device-parameter"
    skip |= ValidateObject(function, kVulkanObjectTypeCuFunctionNVX, false, "VUID-vkDestroyCuFunctionNVX-function-parameter",
                           "VUID-vkDestroyCuFunctionNVX-function-parent", error_obj.location.dot(Field::function));
    skip |= ValidateDestroyObject(function, kVulkanObjectTypeCuFunctionNVX, pAllocator, kVUIDUndefined, kVUIDUndefined,
                                  error_obj.location);

    return skip;
}

void ObjectLifetimes::PreCallRecordDestroyCuFunctionNVX(VkDevice device, VkCuFunctionNVX function,
                                                        const VkAllocationCallbacks* pAllocator) {
    RecordDestroyObject(function, kVulkanObjectTypeCuFunctionNVX);
}

bool ObjectLifetimes::PreCallValidateCmdCuLaunchKernelNVX(VkCommandBuffer commandBuffer, const VkCuLaunchInfoNVX* pLaunchInfo,
                                                          const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: commandBuffer: "VUID-vkCmdCuLaunchKernelNVX-commandBuffer-parameter"
    if (pLaunchInfo) {
        [[maybe_unused]] const Location pLaunchInfo_loc = error_obj.location.dot(Field::pLaunchInfo);
        skip |= ValidateObject(pLaunchInfo->function, kVulkanObjectTypeCuFunctionNVX, false,
                               "VUID-VkCuLaunchInfoNVX-function-parameter", kVUIDUndefined, pLaunchInfo_loc.dot(Field::function));
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetImageViewHandleNVX(VkDevice device, const VkImageViewHandleInfoNVX* pInfo,
                                                           const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkGetImageViewHandleNVX-device-parameter"
    if (pInfo) {
        [[maybe_unused]] const Location pInfo_loc = error_obj.location.dot(Field::pInfo);
        skip |=
            ValidateObject(pInfo->imageView, kVulkanObjectTypeImageView, false, "VUID-VkImageViewHandleInfoNVX-imageView-parameter",
                           "VUID-VkImageViewHandleInfoNVX-commonparent", pInfo_loc.dot(Field::imageView));
        skip |= ValidateObject(pInfo->sampler, kVulkanObjectTypeSampler, true, "VUID-VkImageViewHandleInfoNVX-sampler-parameter",
                               "VUID-VkImageViewHandleInfoNVX-commonparent", pInfo_loc.dot(Field::sampler));
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetImageViewAddressNVX(VkDevice device, VkImageView imageView,
                                                            VkImageViewAddressPropertiesNVX* pProperties,
                                                            const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkGetImageViewAddressNVX-device-parameter"
    skip |= ValidateObject(imageView, kVulkanObjectTypeImageView, false, "VUID-vkGetImageViewAddressNVX-imageView-parameter",
                           "VUID-vkGetImageViewAddressNVX-imageView-parent", error_obj.location.dot(Field::imageView));

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdDrawIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                             VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                             uint32_t maxDrawCount, uint32_t stride,
                                                             const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: commandBuffer: "VUID-vkCmdDrawIndirectCount-commandBuffer-parameter"
    // Checked by chassis: commandBuffer: "VUID-vkCmdDrawIndirectCount-commonparent"
    skip |= ValidateObject(buffer, kVulkanObjectTypeBuffer, false, "VUID-vkCmdDrawIndirectCount-buffer-parameter",
                           "VUID-vkCmdDrawIndirectCount-commonparent", error_obj.location.dot(Field::buffer));
    skip |= ValidateObject(countBuffer, kVulkanObjectTypeBuffer, false, "VUID-vkCmdDrawIndirectCount-countBuffer-parameter",
                           "VUID-vkCmdDrawIndirectCount-commonparent", error_obj.location.dot(Field::countBuffer));

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdDrawIndexedIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                    VkDeviceSize offset, VkBuffer countBuffer,
                                                                    VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                                    uint32_t stride, const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: commandBuffer: "VUID-vkCmdDrawIndexedIndirectCount-commandBuffer-parameter"
    // Checked by chassis: commandBuffer: "VUID-vkCmdDrawIndexedIndirectCount-commonparent"
    skip |= ValidateObject(buffer, kVulkanObjectTypeBuffer, false, "VUID-vkCmdDrawIndexedIndirectCount-buffer-parameter",
                           "VUID-vkCmdDrawIndexedIndirectCount-commonparent", error_obj.location.dot(Field::buffer));
    skip |= ValidateObject(countBuffer, kVulkanObjectTypeBuffer, false, "VUID-vkCmdDrawIndexedIndirectCount-countBuffer-parameter",
                           "VUID-vkCmdDrawIndexedIndirectCount-commonparent", error_obj.location.dot(Field::countBuffer));

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetShaderInfoAMD(VkDevice device, VkPipeline pipeline, VkShaderStageFlagBits shaderStage,
                                                      VkShaderInfoTypeAMD infoType, size_t* pInfoSize, void* pInfo,
                                                      const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkGetShaderInfoAMD-device-parameter"
    skip |= ValidateObject(pipeline, kVulkanObjectTypePipeline, false, "VUID-vkGetShaderInfoAMD-pipeline-parameter",
                           "VUID-vkGetShaderInfoAMD-pipeline-parent", error_obj.location.dot(Field::pipeline));

    return skip;
}
#ifdef VK_USE_PLATFORM_GGP

// vkCreateStreamDescriptorSurfaceGGP:
// Checked by chassis: instance: "VUID-vkCreateStreamDescriptorSurfaceGGP-instance-parameter"

void ObjectLifetimes::PostCallRecordCreateStreamDescriptorSurfaceGGP(VkInstance instance,
                                                                     const VkStreamDescriptorSurfaceCreateInfoGGP* pCreateInfo,
                                                                     const VkAllocationCallbacks* pAllocator,
                                                                     VkSurfaceKHR* pSurface, const RecordObject& record_obj) {
    if (record_obj.result != VK_SUCCESS) return;
    CreateObject(*pSurface, kVulkanObjectTypeSurfaceKHR, pAllocator);
}
#endif  // VK_USE_PLATFORM_GGP

// vkGetPhysicalDeviceExternalImageFormatPropertiesNV:
// Checked by chassis: physicalDevice: "VUID-vkGetPhysicalDeviceExternalImageFormatPropertiesNV-physicalDevice-parameter"

#ifdef VK_USE_PLATFORM_WIN32_KHR

bool ObjectLifetimes::PreCallValidateGetMemoryWin32HandleNV(VkDevice device, VkDeviceMemory memory,
                                                            VkExternalMemoryHandleTypeFlagsNV handleType, HANDLE* pHandle,
                                                            const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkGetMemoryWin32HandleNV-device-parameter"
    skip |= ValidateObject(memory, kVulkanObjectTypeDeviceMemory, false, "VUID-vkGetMemoryWin32HandleNV-memory-parameter",
                           "VUID-vkGetMemoryWin32HandleNV-memory-parent", error_obj.location.dot(Field::memory));

    return skip;
}
#endif  // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_VI_NN

// vkCreateViSurfaceNN:
// Checked by chassis: instance: "VUID-vkCreateViSurfaceNN-instance-parameter"

void ObjectLifetimes::PostCallRecordCreateViSurfaceNN(VkInstance instance, const VkViSurfaceCreateInfoNN* pCreateInfo,
                                                      const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                      const RecordObject& record_obj) {
    if (record_obj.result != VK_SUCCESS) return;
    CreateObject(*pSurface, kVulkanObjectTypeSurfaceKHR, pAllocator);
}
#endif  // VK_USE_PLATFORM_VI_NN

bool ObjectLifetimes::PreCallValidateCmdBeginConditionalRenderingEXT(
    VkCommandBuffer commandBuffer, const VkConditionalRenderingBeginInfoEXT* pConditionalRenderingBegin,
    const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: commandBuffer: "VUID-vkCmdBeginConditionalRenderingEXT-commandBuffer-parameter"
    if (pConditionalRenderingBegin) {
        [[maybe_unused]] const Location pConditionalRenderingBegin_loc = error_obj.location.dot(Field::pConditionalRenderingBegin);
        skip |= ValidateObject(pConditionalRenderingBegin->buffer, kVulkanObjectTypeBuffer, false,
                               "VUID-VkConditionalRenderingBeginInfoEXT-buffer-parameter", kVUIDUndefined,
                               pConditionalRenderingBegin_loc.dot(Field::buffer));
    }

    return skip;
}

// vkCmdEndConditionalRenderingEXT:
// Checked by chassis: commandBuffer: "VUID-vkCmdEndConditionalRenderingEXT-commandBuffer-parameter"

// vkCmdSetViewportWScalingNV:
// Checked by chassis: commandBuffer: "VUID-vkCmdSetViewportWScalingNV-commandBuffer-parameter"

bool ObjectLifetimes::PreCallValidateReleaseDisplayEXT(VkPhysicalDevice physicalDevice, VkDisplayKHR display,
                                                       const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: physicalDevice: "VUID-vkReleaseDisplayEXT-physicalDevice-parameter"
    skip |= ValidateObject(display, kVulkanObjectTypeDisplayKHR, false, "VUID-vkReleaseDisplayEXT-display-parameter",
                           "VUID-vkReleaseDisplayEXT-display-parent", error_obj.location.dot(Field::display),
                           kVulkanObjectTypePhysicalDevice);

    return skip;
}
#ifdef VK_USE_PLATFORM_XLIB_XRANDR_EXT

bool ObjectLifetimes::PreCallValidateAcquireXlibDisplayEXT(VkPhysicalDevice physicalDevice, Display* dpy, VkDisplayKHR display,
                                                           const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: physicalDevice: "VUID-vkAcquireXlibDisplayEXT-physicalDevice-parameter"
    skip |= ValidateObject(display, kVulkanObjectTypeDisplayKHR, false, "VUID-vkAcquireXlibDisplayEXT-display-parameter",
                           "VUID-vkAcquireXlibDisplayEXT-display-parent", error_obj.location.dot(Field::display),
                           kVulkanObjectTypePhysicalDevice);

    return skip;
}
#endif  // VK_USE_PLATFORM_XLIB_XRANDR_EXT
#ifdef VK_USE_PLATFORM_XLIB_XRANDR_EXT

// vkGetRandROutputDisplayEXT:
// Checked by chassis: physicalDevice: "VUID-vkGetRandROutputDisplayEXT-physicalDevice-parameter"

void ObjectLifetimes::PostCallRecordGetRandROutputDisplayEXT(VkPhysicalDevice physicalDevice, Display* dpy, RROutput rrOutput,
                                                             VkDisplayKHR* pDisplay, const RecordObject& record_obj) {
    if (record_obj.result != VK_SUCCESS) return;
    CreateObject(*pDisplay, kVulkanObjectTypeDisplayKHR, nullptr);
}
#endif  // VK_USE_PLATFORM_XLIB_XRANDR_EXT

bool ObjectLifetimes::PreCallValidateGetPhysicalDeviceSurfaceCapabilities2EXT(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                              VkSurfaceCapabilities2EXT* pSurfaceCapabilities,
                                                                              const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: physicalDevice: "VUID-vkGetPhysicalDeviceSurfaceCapabilities2EXT-physicalDevice-parameter"
    // Checked by chassis: physicalDevice: "VUID-vkGetPhysicalDeviceSurfaceCapabilities2EXT-commonparent"
    skip |= ValidateObject(surface, kVulkanObjectTypeSurfaceKHR, false,
                           "VUID-vkGetPhysicalDeviceSurfaceCapabilities2EXT-surface-parameter",
                           "VUID-vkGetPhysicalDeviceSurfaceCapabilities2EXT-commonparent", error_obj.location.dot(Field::surface),
                           kVulkanObjectTypeInstance);

    return skip;
}

bool ObjectLifetimes::PreCallValidateDisplayPowerControlEXT(VkDevice device, VkDisplayKHR display,
                                                            const VkDisplayPowerInfoEXT* pDisplayPowerInfo,
                                                            const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkDisplayPowerControlEXT-device-parameter"
    // Checked by chassis: device: "VUID-vkDisplayPowerControlEXT-commonparent"
    skip |= ValidateObject(display, kVulkanObjectTypeDisplayKHR, false, "VUID-vkDisplayPowerControlEXT-display-parameter",
                           "VUID-vkDisplayPowerControlEXT-commonparent", error_obj.location.dot(Field::display),
                           kVulkanObjectTypePhysicalDevice);

    return skip;
}

// vkRegisterDeviceEventEXT:
// Checked by chassis: device: "VUID-vkRegisterDeviceEventEXT-device-parameter"

void ObjectLifetimes::PostCallRecordRegisterDeviceEventEXT(VkDevice device, const VkDeviceEventInfoEXT* pDeviceEventInfo,
                                                           const VkAllocationCallbacks* pAllocator, VkFence* pFence,
                                                           const RecordObject& record_obj) {
    if (record_obj.result != VK_SUCCESS) return;
    CreateObject(*pFence, kVulkanObjectTypeFence, pAllocator);
}

bool ObjectLifetimes::PreCallValidateRegisterDisplayEventEXT(VkDevice device, VkDisplayKHR display,
                                                             const VkDisplayEventInfoEXT* pDisplayEventInfo,
                                                             const VkAllocationCallbacks* pAllocator, VkFence* pFence,
                                                             const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkRegisterDisplayEventEXT-device-parameter"
    // Checked by chassis: device: "VUID-vkRegisterDisplayEventEXT-commonparent"
    skip |= ValidateObject(display, kVulkanObjectTypeDisplayKHR, false, "VUID-vkRegisterDisplayEventEXT-display-parameter",
                           "VUID-vkRegisterDisplayEventEXT-commonparent", error_obj.location.dot(Field::display),
                           kVulkanObjectTypePhysicalDevice);

    return skip;
}

void ObjectLifetimes::PostCallRecordRegisterDisplayEventEXT(VkDevice device, VkDisplayKHR display,
                                                            const VkDisplayEventInfoEXT* pDisplayEventInfo,
                                                            const VkAllocationCallbacks* pAllocator, VkFence* pFence,
                                                            const RecordObject& record_obj) {
    if (record_obj.result != VK_SUCCESS) return;
    CreateObject(*pFence, kVulkanObjectTypeFence, pAllocator);
}

bool ObjectLifetimes::PreCallValidateGetSwapchainCounterEXT(VkDevice device, VkSwapchainKHR swapchain,
                                                            VkSurfaceCounterFlagBitsEXT counter, uint64_t* pCounterValue,
                                                            const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkGetSwapchainCounterEXT-device-parameter"
    skip |= ValidateObject(swapchain, kVulkanObjectTypeSwapchainKHR, false, "VUID-vkGetSwapchainCounterEXT-swapchain-parameter",
                           "VUID-vkGetSwapchainCounterEXT-swapchain-parent", error_obj.location.dot(Field::swapchain));

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetRefreshCycleDurationGOOGLE(VkDevice device, VkSwapchainKHR swapchain,
                                                                   VkRefreshCycleDurationGOOGLE* pDisplayTimingProperties,
                                                                   const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkGetRefreshCycleDurationGOOGLE-device-parameter"
    skip |=
        ValidateObject(swapchain, kVulkanObjectTypeSwapchainKHR, false, "VUID-vkGetRefreshCycleDurationGOOGLE-swapchain-parameter",
                       "VUID-vkGetRefreshCycleDurationGOOGLE-swapchain-parent", error_obj.location.dot(Field::swapchain));

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetPastPresentationTimingGOOGLE(VkDevice device, VkSwapchainKHR swapchain,
                                                                     uint32_t* pPresentationTimingCount,
                                                                     VkPastPresentationTimingGOOGLE* pPresentationTimings,
                                                                     const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkGetPastPresentationTimingGOOGLE-device-parameter"
    skip |= ValidateObject(swapchain, kVulkanObjectTypeSwapchainKHR, false,
                           "VUID-vkGetPastPresentationTimingGOOGLE-swapchain-parameter",
                           "VUID-vkGetPastPresentationTimingGOOGLE-swapchain-parent", error_obj.location.dot(Field::swapchain));

    return skip;
}

// vkCmdSetDiscardRectangleEXT:
// Checked by chassis: commandBuffer: "VUID-vkCmdSetDiscardRectangleEXT-commandBuffer-parameter"

// vkCmdSetDiscardRectangleEnableEXT:
// Checked by chassis: commandBuffer: "VUID-vkCmdSetDiscardRectangleEnableEXT-commandBuffer-parameter"

// vkCmdSetDiscardRectangleModeEXT:
// Checked by chassis: commandBuffer: "VUID-vkCmdSetDiscardRectangleModeEXT-commandBuffer-parameter"

bool ObjectLifetimes::PreCallValidateSetHdrMetadataEXT(VkDevice device, uint32_t swapchainCount, const VkSwapchainKHR* pSwapchains,
                                                       const VkHdrMetadataEXT* pMetadata, const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkSetHdrMetadataEXT-device-parameter"

    if ((swapchainCount > 0) && (pSwapchains)) {
        for (uint32_t index0 = 0; index0 < swapchainCount; ++index0) {
            skip |= ValidateObject(pSwapchains[index0], kVulkanObjectTypeSwapchainKHR, false,
                                   "VUID-vkSetHdrMetadataEXT-pSwapchains-parameter", "VUID-vkSetHdrMetadataEXT-pSwapchains-parent",
                                   error_obj.location.dot(Field::pSwapchains, index0));
        }
    }

    return skip;
}
#ifdef VK_USE_PLATFORM_IOS_MVK

// vkCreateIOSSurfaceMVK:
// Checked by chassis: instance: "VUID-vkCreateIOSSurfaceMVK-instance-parameter"

void ObjectLifetimes::PostCallRecordCreateIOSSurfaceMVK(VkInstance instance, const VkIOSSurfaceCreateInfoMVK* pCreateInfo,
                                                        const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                        const RecordObject& record_obj) {
    if (record_obj.result != VK_SUCCESS) return;
    CreateObject(*pSurface, kVulkanObjectTypeSurfaceKHR, pAllocator);
}
#endif  // VK_USE_PLATFORM_IOS_MVK
#ifdef VK_USE_PLATFORM_MACOS_MVK

// vkCreateMacOSSurfaceMVK:
// Checked by chassis: instance: "VUID-vkCreateMacOSSurfaceMVK-instance-parameter"

void ObjectLifetimes::PostCallRecordCreateMacOSSurfaceMVK(VkInstance instance, const VkMacOSSurfaceCreateInfoMVK* pCreateInfo,
                                                          const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                          const RecordObject& record_obj) {
    if (record_obj.result != VK_SUCCESS) return;
    CreateObject(*pSurface, kVulkanObjectTypeSurfaceKHR, pAllocator);
}
#endif  // VK_USE_PLATFORM_MACOS_MVK

// vkQueueBeginDebugUtilsLabelEXT:
// Checked by chassis: queue: "VUID-vkQueueBeginDebugUtilsLabelEXT-queue-parameter"

// vkQueueEndDebugUtilsLabelEXT:
// Checked by chassis: queue: "VUID-vkQueueEndDebugUtilsLabelEXT-queue-parameter"

// vkQueueInsertDebugUtilsLabelEXT:
// Checked by chassis: queue: "VUID-vkQueueInsertDebugUtilsLabelEXT-queue-parameter"

// vkCmdBeginDebugUtilsLabelEXT:
// Checked by chassis: commandBuffer: "VUID-vkCmdBeginDebugUtilsLabelEXT-commandBuffer-parameter"

// vkCmdEndDebugUtilsLabelEXT:
// Checked by chassis: commandBuffer: "VUID-vkCmdEndDebugUtilsLabelEXT-commandBuffer-parameter"

// vkCmdInsertDebugUtilsLabelEXT:
// Checked by chassis: commandBuffer: "VUID-vkCmdInsertDebugUtilsLabelEXT-commandBuffer-parameter"

// vkCreateDebugUtilsMessengerEXT:
// Checked by chassis: instance: "VUID-vkCreateDebugUtilsMessengerEXT-instance-parameter"

void ObjectLifetimes::PostCallRecordCreateDebugUtilsMessengerEXT(VkInstance instance,
                                                                 const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                                                 const VkAllocationCallbacks* pAllocator,
                                                                 VkDebugUtilsMessengerEXT* pMessenger,
                                                                 const RecordObject& record_obj) {
    if (record_obj.result != VK_SUCCESS) return;
    CreateObject(*pMessenger, kVulkanObjectTypeDebugUtilsMessengerEXT, pAllocator);
}

bool ObjectLifetimes::PreCallValidateDestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT messenger,
                                                                   const VkAllocationCallbacks* pAllocator,
                                                                   const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: instance: "VUID-vkDestroyDebugUtilsMessengerEXT-instance-parameter"
    skip |= ValidateObject(messenger, kVulkanObjectTypeDebugUtilsMessengerEXT, true,
                           "VUID-vkDestroyDebugUtilsMessengerEXT-messenger-parameter",
                           "VUID-vkDestroyDebugUtilsMessengerEXT-messenger-parent", error_obj.location.dot(Field::messenger),
                           kVulkanObjectTypeInstance);
    skip |= ValidateDestroyObject(messenger, kVulkanObjectTypeDebugUtilsMessengerEXT, pAllocator, kVUIDUndefined, kVUIDUndefined,
                                  error_obj.location);

    return skip;
}

void ObjectLifetimes::PreCallRecordDestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT messenger,
                                                                 const VkAllocationCallbacks* pAllocator) {
    RecordDestroyObject(messenger, kVulkanObjectTypeDebugUtilsMessengerEXT);
}

// vkSubmitDebugUtilsMessageEXT:
// Checked by chassis: instance: "VUID-vkSubmitDebugUtilsMessageEXT-instance-parameter"

#ifdef VK_USE_PLATFORM_ANDROID_KHR

// vkGetAndroidHardwareBufferPropertiesANDROID:
// Checked by chassis: device: "VUID-vkGetAndroidHardwareBufferPropertiesANDROID-device-parameter"

#endif  // VK_USE_PLATFORM_ANDROID_KHR
#ifdef VK_USE_PLATFORM_ANDROID_KHR

bool ObjectLifetimes::PreCallValidateGetMemoryAndroidHardwareBufferANDROID(VkDevice device,
                                                                           const VkMemoryGetAndroidHardwareBufferInfoANDROID* pInfo,
                                                                           struct AHardwareBuffer** pBuffer,
                                                                           const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkGetMemoryAndroidHardwareBufferANDROID-device-parameter"
    if (pInfo) {
        [[maybe_unused]] const Location pInfo_loc = error_obj.location.dot(Field::pInfo);
        skip |= ValidateObject(pInfo->memory, kVulkanObjectTypeDeviceMemory, false,
                               "VUID-VkMemoryGetAndroidHardwareBufferInfoANDROID-memory-parameter", kVUIDUndefined,
                               pInfo_loc.dot(Field::memory));
    }

    return skip;
}
#endif  // VK_USE_PLATFORM_ANDROID_KHR
#ifdef VK_ENABLE_BETA_EXTENSIONS

bool ObjectLifetimes::PreCallValidateCreateExecutionGraphPipelinesAMDX(VkDevice device, VkPipelineCache pipelineCache,
                                                                       uint32_t createInfoCount,
                                                                       const VkExecutionGraphPipelineCreateInfoAMDX* pCreateInfos,
                                                                       const VkAllocationCallbacks* pAllocator,
                                                                       VkPipeline* pPipelines, const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkCreateExecutionGraphPipelinesAMDX-device-parameter"
    skip |= ValidateObject(
        pipelineCache, kVulkanObjectTypePipelineCache, true, "VUID-vkCreateExecutionGraphPipelinesAMDX-pipelineCache-parameter",
        "VUID-vkCreateExecutionGraphPipelinesAMDX-pipelineCache-parent", error_obj.location.dot(Field::pipelineCache));
    if (pCreateInfos) {
        for (uint32_t index0 = 0; index0 < createInfoCount; ++index0) {
            [[maybe_unused]] const Location index0_loc = error_obj.location.dot(Field::pCreateInfos, index0);
            if (pCreateInfos[index0].pStages) {
                for (uint32_t index1 = 0; index1 < pCreateInfos[index0].stageCount; ++index1) {
                    [[maybe_unused]] const Location index1_loc = index0_loc.dot(Field::pStages, index1);
                    skip |= ValidateObject(pCreateInfos[index0].pStages[index1].module, kVulkanObjectTypeShaderModule, true,
                                           "VUID-VkPipelineShaderStageCreateInfo-module-parameter", kVUIDUndefined,
                                           index1_loc.dot(Field::module));
                    if (auto pNext = vku::FindStructInPNextChain<VkShaderModuleValidationCacheCreateInfoEXT>(
                            pCreateInfos[index0].pStages[index1].pNext)) {
                        const Location pNext_loc = index1_loc.pNext(Struct::VkShaderModuleValidationCacheCreateInfoEXT);
                        skip |= ValidateObject(pNext->validationCache, kVulkanObjectTypeValidationCacheEXT, false,
                                               "VUID-VkShaderModuleValidationCacheCreateInfoEXT-validationCache-parameter",
                                               kVUIDUndefined, pNext_loc.dot(Field::validationCache));
                    }
                }
            }
            if (pCreateInfos[index0].pLibraryInfo) {
                [[maybe_unused]] const Location pLibraryInfo_loc = index0_loc.dot(Field::pLibraryInfo);

                if ((pCreateInfos[index0].pLibraryInfo->libraryCount > 0) && (pCreateInfos[index0].pLibraryInfo->pLibraries)) {
                    for (uint32_t index2 = 0; index2 < pCreateInfos[index0].pLibraryInfo->libraryCount; ++index2) {
                        skip |= ValidateObject(pCreateInfos[index0].pLibraryInfo->pLibraries[index2], kVulkanObjectTypePipeline,
                                               false, "VUID-VkPipelineLibraryCreateInfoKHR-pLibraries-parameter", kVUIDUndefined,
                                               pLibraryInfo_loc.dot(Field::pLibraries, index2));
                    }
                }
            }
            skip |= ValidateObject(pCreateInfos[index0].layout, kVulkanObjectTypePipelineLayout, false,
                                   "VUID-VkExecutionGraphPipelineCreateInfoAMDX-layout-parameter",
                                   "VUID-VkExecutionGraphPipelineCreateInfoAMDX-commonparent", index0_loc.dot(Field::layout));
            if ((pCreateInfos[index0].flags & VK_PIPELINE_CREATE_DERIVATIVE_BIT) && (pCreateInfos[index0].basePipelineIndex == -1))
                skip |= ValidateObject(pCreateInfos[index0].basePipelineHandle, kVulkanObjectTypePipeline, false, kVUIDUndefined,
                                       "VUID-VkExecutionGraphPipelineCreateInfoAMDX-commonparent", error_obj.location);
        }
    }

    return skip;
}

void ObjectLifetimes::PostCallRecordCreateExecutionGraphPipelinesAMDX(VkDevice device, VkPipelineCache pipelineCache,
                                                                      uint32_t createInfoCount,
                                                                      const VkExecutionGraphPipelineCreateInfoAMDX* pCreateInfos,
                                                                      const VkAllocationCallbacks* pAllocator,
                                                                      VkPipeline* pPipelines, const RecordObject& record_obj) {
    if (record_obj.result != VK_SUCCESS) return;
    if (pPipelines) {
        for (uint32_t index = 0; index < createInfoCount; index++) {
            CreateObject(pPipelines[index], kVulkanObjectTypePipeline, pAllocator);
        }
    }
}
#endif  // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS

bool ObjectLifetimes::PreCallValidateGetExecutionGraphPipelineScratchSizeAMDX(VkDevice device, VkPipeline executionGraph,
                                                                              VkExecutionGraphPipelineScratchSizeAMDX* pSizeInfo,
                                                                              const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkGetExecutionGraphPipelineScratchSizeAMDX-device-parameter"
    skip |= ValidateObject(executionGraph, kVulkanObjectTypePipeline, false,
                           "VUID-vkGetExecutionGraphPipelineScratchSizeAMDX-executionGraph-parameter",
                           "VUID-vkGetExecutionGraphPipelineScratchSizeAMDX-executionGraph-parent",
                           error_obj.location.dot(Field::executionGraph));

    return skip;
}
#endif  // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS

bool ObjectLifetimes::PreCallValidateGetExecutionGraphPipelineNodeIndexAMDX(
    VkDevice device, VkPipeline executionGraph, const VkPipelineShaderStageNodeCreateInfoAMDX* pNodeInfo, uint32_t* pNodeIndex,
    const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkGetExecutionGraphPipelineNodeIndexAMDX-device-parameter"
    skip |= ValidateObject(
        executionGraph, kVulkanObjectTypePipeline, false, "VUID-vkGetExecutionGraphPipelineNodeIndexAMDX-executionGraph-parameter",
        "VUID-vkGetExecutionGraphPipelineNodeIndexAMDX-executionGraph-parent", error_obj.location.dot(Field::executionGraph));

    return skip;
}
#endif  // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS

// vkCmdInitializeGraphScratchMemoryAMDX:
// Checked by chassis: commandBuffer: "VUID-vkCmdInitializeGraphScratchMemoryAMDX-commandBuffer-parameter"

#endif  // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS

// vkCmdDispatchGraphAMDX:
// Checked by chassis: commandBuffer: "VUID-vkCmdDispatchGraphAMDX-commandBuffer-parameter"

#endif  // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS

// vkCmdDispatchGraphIndirectAMDX:
// Checked by chassis: commandBuffer: "VUID-vkCmdDispatchGraphIndirectAMDX-commandBuffer-parameter"

#endif  // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS

// vkCmdDispatchGraphIndirectCountAMDX:
// Checked by chassis: commandBuffer: "VUID-vkCmdDispatchGraphIndirectCountAMDX-commandBuffer-parameter"

#endif  // VK_ENABLE_BETA_EXTENSIONS

// vkCmdSetSampleLocationsEXT:
// Checked by chassis: commandBuffer: "VUID-vkCmdSetSampleLocationsEXT-commandBuffer-parameter"

// vkGetPhysicalDeviceMultisamplePropertiesEXT:
// Checked by chassis: physicalDevice: "VUID-vkGetPhysicalDeviceMultisamplePropertiesEXT-physicalDevice-parameter"

bool ObjectLifetimes::PreCallValidateGetImageDrmFormatModifierPropertiesEXT(VkDevice device, VkImage image,
                                                                            VkImageDrmFormatModifierPropertiesEXT* pProperties,
                                                                            const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkGetImageDrmFormatModifierPropertiesEXT-device-parameter"
    skip |= ValidateObject(image, kVulkanObjectTypeImage, false, "VUID-vkGetImageDrmFormatModifierPropertiesEXT-image-parameter",
                           "VUID-vkGetImageDrmFormatModifierPropertiesEXT-image-parent", error_obj.location.dot(Field::image));

    return skip;
}

// vkCreateValidationCacheEXT:
// Checked by chassis: device: "VUID-vkCreateValidationCacheEXT-device-parameter"

void ObjectLifetimes::PostCallRecordCreateValidationCacheEXT(VkDevice device, const VkValidationCacheCreateInfoEXT* pCreateInfo,
                                                             const VkAllocationCallbacks* pAllocator,
                                                             VkValidationCacheEXT* pValidationCache,
                                                             const RecordObject& record_obj) {
    if (record_obj.result != VK_SUCCESS) return;
    CreateObject(*pValidationCache, kVulkanObjectTypeValidationCacheEXT, pAllocator);
}

bool ObjectLifetimes::PreCallValidateDestroyValidationCacheEXT(VkDevice device, VkValidationCacheEXT validationCache,
                                                               const VkAllocationCallbacks* pAllocator,
                                                               const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkDestroyValidationCacheEXT-device-parameter"
    skip |= ValidateObject(
        validationCache, kVulkanObjectTypeValidationCacheEXT, true, "VUID-vkDestroyValidationCacheEXT-validationCache-parameter",
        "VUID-vkDestroyValidationCacheEXT-validationCache-parent", error_obj.location.dot(Field::validationCache));
    skip |= ValidateDestroyObject(validationCache, kVulkanObjectTypeValidationCacheEXT, pAllocator, kVUIDUndefined, kVUIDUndefined,
                                  error_obj.location);

    return skip;
}

void ObjectLifetimes::PreCallRecordDestroyValidationCacheEXT(VkDevice device, VkValidationCacheEXT validationCache,
                                                             const VkAllocationCallbacks* pAllocator) {
    RecordDestroyObject(validationCache, kVulkanObjectTypeValidationCacheEXT);
}

bool ObjectLifetimes::PreCallValidateMergeValidationCachesEXT(VkDevice device, VkValidationCacheEXT dstCache,
                                                              uint32_t srcCacheCount, const VkValidationCacheEXT* pSrcCaches,
                                                              const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkMergeValidationCachesEXT-device-parameter"
    skip |=
        ValidateObject(dstCache, kVulkanObjectTypeValidationCacheEXT, false, "VUID-vkMergeValidationCachesEXT-dstCache-parameter",
                       "VUID-vkMergeValidationCachesEXT-dstCache-parent", error_obj.location.dot(Field::dstCache));

    if ((srcCacheCount > 0) && (pSrcCaches)) {
        for (uint32_t index0 = 0; index0 < srcCacheCount; ++index0) {
            skip |= ValidateObject(pSrcCaches[index0], kVulkanObjectTypeValidationCacheEXT, false,
                                   "VUID-vkMergeValidationCachesEXT-pSrcCaches-parameter",
                                   "VUID-vkMergeValidationCachesEXT-pSrcCaches-parent",
                                   error_obj.location.dot(Field::pSrcCaches, index0));
        }
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetValidationCacheDataEXT(VkDevice device, VkValidationCacheEXT validationCache,
                                                               size_t* pDataSize, void* pData, const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkGetValidationCacheDataEXT-device-parameter"
    skip |= ValidateObject(
        validationCache, kVulkanObjectTypeValidationCacheEXT, false, "VUID-vkGetValidationCacheDataEXT-validationCache-parameter",
        "VUID-vkGetValidationCacheDataEXT-validationCache-parent", error_obj.location.dot(Field::validationCache));

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdBindShadingRateImageNV(VkCommandBuffer commandBuffer, VkImageView imageView,
                                                               VkImageLayout imageLayout, const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: commandBuffer: "VUID-vkCmdBindShadingRateImageNV-commandBuffer-parameter"
    // Checked by chassis: commandBuffer: "VUID-vkCmdBindShadingRateImageNV-commonparent"
    skip |= ValidateObject(imageView, kVulkanObjectTypeImageView, true, "VUID-vkCmdBindShadingRateImageNV-imageView-parameter",
                           "VUID-vkCmdBindShadingRateImageNV-commonparent", error_obj.location.dot(Field::imageView));

    return skip;
}

// vkCmdSetViewportShadingRatePaletteNV:
// Checked by chassis: commandBuffer: "VUID-vkCmdSetViewportShadingRatePaletteNV-commandBuffer-parameter"

// vkCmdSetCoarseSampleOrderNV:
// Checked by chassis: commandBuffer: "VUID-vkCmdSetCoarseSampleOrderNV-commandBuffer-parameter"

bool ObjectLifetimes::PreCallValidateCreateAccelerationStructureNV(VkDevice device,
                                                                   const VkAccelerationStructureCreateInfoNV* pCreateInfo,
                                                                   const VkAllocationCallbacks* pAllocator,
                                                                   VkAccelerationStructureNV* pAccelerationStructure,
                                                                   const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkCreateAccelerationStructureNV-device-parameter"
    if (pCreateInfo) {
        [[maybe_unused]] const Location pCreateInfo_loc = error_obj.location.dot(Field::pCreateInfo);
        [[maybe_unused]] const Location info_loc = pCreateInfo_loc.dot(Field::info);
        if (pCreateInfo->info.pGeometries) {
            for (uint32_t index2 = 0; index2 < pCreateInfo->info.geometryCount; ++index2) {
                [[maybe_unused]] const Location index2_loc = info_loc.dot(Field::pGeometries, index2);
                [[maybe_unused]] const Location geometry_loc = index2_loc.dot(Field::geometry);
                [[maybe_unused]] const Location triangles_loc = geometry_loc.dot(Field::triangles);
                skip |= ValidateObject(pCreateInfo->info.pGeometries[index2].geometry.triangles.vertexData, kVulkanObjectTypeBuffer,
                                       true, "VUID-VkGeometryTrianglesNV-vertexData-parameter",
                                       "VUID-VkGeometryTrianglesNV-commonparent", triangles_loc.dot(Field::vertexData));
                skip |= ValidateObject(pCreateInfo->info.pGeometries[index2].geometry.triangles.indexData, kVulkanObjectTypeBuffer,
                                       true, "VUID-VkGeometryTrianglesNV-indexData-parameter",
                                       "VUID-VkGeometryTrianglesNV-commonparent", triangles_loc.dot(Field::indexData));
                skip |= ValidateObject(pCreateInfo->info.pGeometries[index2].geometry.triangles.transformData,
                                       kVulkanObjectTypeBuffer, true, "VUID-VkGeometryTrianglesNV-transformData-parameter",
                                       "VUID-VkGeometryTrianglesNV-commonparent", triangles_loc.dot(Field::transformData));
                [[maybe_unused]] const Location aabbs_loc = geometry_loc.dot(Field::aabbs);
                skip |= ValidateObject(pCreateInfo->info.pGeometries[index2].geometry.aabbs.aabbData, kVulkanObjectTypeBuffer, true,
                                       "VUID-VkGeometryAABBNV-aabbData-parameter", kVUIDUndefined, aabbs_loc.dot(Field::aabbData));
            }
        }
    }

    return skip;
}

void ObjectLifetimes::PostCallRecordCreateAccelerationStructureNV(VkDevice device,
                                                                  const VkAccelerationStructureCreateInfoNV* pCreateInfo,
                                                                  const VkAllocationCallbacks* pAllocator,
                                                                  VkAccelerationStructureNV* pAccelerationStructure,
                                                                  const RecordObject& record_obj) {
    if (record_obj.result != VK_SUCCESS) return;
    CreateObject(*pAccelerationStructure, kVulkanObjectTypeAccelerationStructureNV, pAllocator);
}

bool ObjectLifetimes::PreCallValidateDestroyAccelerationStructureNV(VkDevice device,
                                                                    VkAccelerationStructureNV accelerationStructure,
                                                                    const VkAllocationCallbacks* pAllocator,
                                                                    const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkDestroyAccelerationStructureNV-device-parameter"
    skip |= ValidateObject(accelerationStructure, kVulkanObjectTypeAccelerationStructureNV, true,
                           "VUID-vkDestroyAccelerationStructureNV-accelerationStructure-parameter",
                           "VUID-vkDestroyAccelerationStructureNV-accelerationStructure-parent",
                           error_obj.location.dot(Field::accelerationStructure));
    skip |= ValidateDestroyObject(accelerationStructure, kVulkanObjectTypeAccelerationStructureNV, pAllocator,
                                  "VUID-vkDestroyAccelerationStructureNV-accelerationStructure-03753",
                                  "VUID-vkDestroyAccelerationStructureNV-accelerationStructure-03754", error_obj.location);

    return skip;
}

void ObjectLifetimes::PreCallRecordDestroyAccelerationStructureNV(VkDevice device, VkAccelerationStructureNV accelerationStructure,
                                                                  const VkAllocationCallbacks* pAllocator) {
    RecordDestroyObject(accelerationStructure, kVulkanObjectTypeAccelerationStructureNV);
}

bool ObjectLifetimes::PreCallValidateGetAccelerationStructureMemoryRequirementsNV(
    VkDevice device, const VkAccelerationStructureMemoryRequirementsInfoNV* pInfo, VkMemoryRequirements2KHR* pMemoryRequirements,
    const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkGetAccelerationStructureMemoryRequirementsNV-device-parameter"
    if (pInfo) {
        [[maybe_unused]] const Location pInfo_loc = error_obj.location.dot(Field::pInfo);
        skip |= ValidateObject(pInfo->accelerationStructure, kVulkanObjectTypeAccelerationStructureNV, false,
                               "VUID-VkAccelerationStructureMemoryRequirementsInfoNV-accelerationStructure-parameter",
                               kVUIDUndefined, pInfo_loc.dot(Field::accelerationStructure));
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateBindAccelerationStructureMemoryNV(VkDevice device, uint32_t bindInfoCount,
                                                                       const VkBindAccelerationStructureMemoryInfoNV* pBindInfos,
                                                                       const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkBindAccelerationStructureMemoryNV-device-parameter"
    if (pBindInfos) {
        for (uint32_t index0 = 0; index0 < bindInfoCount; ++index0) {
            [[maybe_unused]] const Location index0_loc = error_obj.location.dot(Field::pBindInfos, index0);
            skip |= ValidateObject(pBindInfos[index0].accelerationStructure, kVulkanObjectTypeAccelerationStructureNV, false,
                                   "VUID-VkBindAccelerationStructureMemoryInfoNV-accelerationStructure-parameter",
                                   "VUID-VkBindAccelerationStructureMemoryInfoNV-commonparent",
                                   index0_loc.dot(Field::accelerationStructure));
            skip |= ValidateObject(pBindInfos[index0].memory, kVulkanObjectTypeDeviceMemory, false,
                                   "VUID-VkBindAccelerationStructureMemoryInfoNV-memory-parameter",
                                   "VUID-VkBindAccelerationStructureMemoryInfoNV-commonparent", index0_loc.dot(Field::memory));
        }
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdBuildAccelerationStructureNV(
    VkCommandBuffer commandBuffer, const VkAccelerationStructureInfoNV* pInfo, VkBuffer instanceData, VkDeviceSize instanceOffset,
    VkBool32 update, VkAccelerationStructureNV dst, VkAccelerationStructureNV src, VkBuffer scratch, VkDeviceSize scratchOffset,
    const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: commandBuffer: "VUID-vkCmdBuildAccelerationStructureNV-commandBuffer-parameter"
    // Checked by chassis: commandBuffer: "VUID-vkCmdBuildAccelerationStructureNV-commonparent"
    if (pInfo) {
        [[maybe_unused]] const Location pInfo_loc = error_obj.location.dot(Field::pInfo);
        if (pInfo->pGeometries) {
            for (uint32_t index1 = 0; index1 < pInfo->geometryCount; ++index1) {
                [[maybe_unused]] const Location index1_loc = pInfo_loc.dot(Field::pGeometries, index1);
                [[maybe_unused]] const Location geometry_loc = index1_loc.dot(Field::geometry);
                [[maybe_unused]] const Location triangles_loc = geometry_loc.dot(Field::triangles);
                skip |= ValidateObject(pInfo->pGeometries[index1].geometry.triangles.vertexData, kVulkanObjectTypeBuffer, true,
                                       "VUID-VkGeometryTrianglesNV-vertexData-parameter", "VUID-VkGeometryTrianglesNV-commonparent",
                                       triangles_loc.dot(Field::vertexData));
                skip |= ValidateObject(pInfo->pGeometries[index1].geometry.triangles.indexData, kVulkanObjectTypeBuffer, true,
                                       "VUID-VkGeometryTrianglesNV-indexData-parameter", "VUID-VkGeometryTrianglesNV-commonparent",
                                       triangles_loc.dot(Field::indexData));
                skip |= ValidateObject(pInfo->pGeometries[index1].geometry.triangles.transformData, kVulkanObjectTypeBuffer, true,
                                       "VUID-VkGeometryTrianglesNV-transformData-parameter",
                                       "VUID-VkGeometryTrianglesNV-commonparent", triangles_loc.dot(Field::transformData));
                [[maybe_unused]] const Location aabbs_loc = geometry_loc.dot(Field::aabbs);
                skip |= ValidateObject(pInfo->pGeometries[index1].geometry.aabbs.aabbData, kVulkanObjectTypeBuffer, true,
                                       "VUID-VkGeometryAABBNV-aabbData-parameter", kVUIDUndefined, aabbs_loc.dot(Field::aabbData));
            }
        }
    }
    skip |=
        ValidateObject(instanceData, kVulkanObjectTypeBuffer, true, "VUID-vkCmdBuildAccelerationStructureNV-instanceData-parameter",
                       "VUID-vkCmdBuildAccelerationStructureNV-commonparent", error_obj.location.dot(Field::instanceData));
    skip |=
        ValidateObject(dst, kVulkanObjectTypeAccelerationStructureNV, false, "VUID-vkCmdBuildAccelerationStructureNV-dst-parameter",
                       "VUID-vkCmdBuildAccelerationStructureNV-commonparent", error_obj.location.dot(Field::dst));
    skip |=
        ValidateObject(src, kVulkanObjectTypeAccelerationStructureNV, true, "VUID-vkCmdBuildAccelerationStructureNV-src-parameter",
                       "VUID-vkCmdBuildAccelerationStructureNV-commonparent", error_obj.location.dot(Field::src));
    skip |= ValidateObject(scratch, kVulkanObjectTypeBuffer, false, "VUID-vkCmdBuildAccelerationStructureNV-scratch-parameter",
                           "VUID-vkCmdBuildAccelerationStructureNV-commonparent", error_obj.location.dot(Field::scratch));

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdCopyAccelerationStructureNV(VkCommandBuffer commandBuffer, VkAccelerationStructureNV dst,
                                                                    VkAccelerationStructureNV src,
                                                                    VkCopyAccelerationStructureModeKHR mode,
                                                                    const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: commandBuffer: "VUID-vkCmdCopyAccelerationStructureNV-commandBuffer-parameter"
    // Checked by chassis: commandBuffer: "VUID-vkCmdCopyAccelerationStructureNV-commonparent"
    skip |=
        ValidateObject(dst, kVulkanObjectTypeAccelerationStructureNV, false, "VUID-vkCmdCopyAccelerationStructureNV-dst-parameter",
                       "VUID-vkCmdCopyAccelerationStructureNV-commonparent", error_obj.location.dot(Field::dst));
    skip |=
        ValidateObject(src, kVulkanObjectTypeAccelerationStructureNV, false, "VUID-vkCmdCopyAccelerationStructureNV-src-parameter",
                       "VUID-vkCmdCopyAccelerationStructureNV-commonparent", error_obj.location.dot(Field::src));

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdTraceRaysNV(VkCommandBuffer commandBuffer, VkBuffer raygenShaderBindingTableBuffer,
                                                    VkDeviceSize raygenShaderBindingOffset, VkBuffer missShaderBindingTableBuffer,
                                                    VkDeviceSize missShaderBindingOffset, VkDeviceSize missShaderBindingStride,
                                                    VkBuffer hitShaderBindingTableBuffer, VkDeviceSize hitShaderBindingOffset,
                                                    VkDeviceSize hitShaderBindingStride, VkBuffer callableShaderBindingTableBuffer,
                                                    VkDeviceSize callableShaderBindingOffset,
                                                    VkDeviceSize callableShaderBindingStride, uint32_t width, uint32_t height,
                                                    uint32_t depth, const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: commandBuffer: "VUID-vkCmdTraceRaysNV-commandBuffer-parameter"
    // Checked by chassis: commandBuffer: "VUID-vkCmdTraceRaysNV-commonparent"
    skip |= ValidateObject(raygenShaderBindingTableBuffer, kVulkanObjectTypeBuffer, false,
                           "VUID-vkCmdTraceRaysNV-raygenShaderBindingTableBuffer-parameter", "VUID-vkCmdTraceRaysNV-commonparent",
                           error_obj.location.dot(Field::raygenShaderBindingTableBuffer));
    skip |= ValidateObject(missShaderBindingTableBuffer, kVulkanObjectTypeBuffer, true,
                           "VUID-vkCmdTraceRaysNV-missShaderBindingTableBuffer-parameter", "VUID-vkCmdTraceRaysNV-commonparent",
                           error_obj.location.dot(Field::missShaderBindingTableBuffer));
    skip |= ValidateObject(hitShaderBindingTableBuffer, kVulkanObjectTypeBuffer, true,
                           "VUID-vkCmdTraceRaysNV-hitShaderBindingTableBuffer-parameter", "VUID-vkCmdTraceRaysNV-commonparent",
                           error_obj.location.dot(Field::hitShaderBindingTableBuffer));
    skip |= ValidateObject(callableShaderBindingTableBuffer, kVulkanObjectTypeBuffer, true,
                           "VUID-vkCmdTraceRaysNV-callableShaderBindingTableBuffer-parameter", "VUID-vkCmdTraceRaysNV-commonparent",
                           error_obj.location.dot(Field::callableShaderBindingTableBuffer));

    return skip;
}

bool ObjectLifetimes::PreCallValidateCreateRayTracingPipelinesNV(VkDevice device, VkPipelineCache pipelineCache,
                                                                 uint32_t createInfoCount,
                                                                 const VkRayTracingPipelineCreateInfoNV* pCreateInfos,
                                                                 const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                                 const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkCreateRayTracingPipelinesNV-device-parameter"
    skip |= ValidateObject(pipelineCache, kVulkanObjectTypePipelineCache, true,
                           "VUID-vkCreateRayTracingPipelinesNV-pipelineCache-parameter",
                           "VUID-vkCreateRayTracingPipelinesNV-pipelineCache-parent", error_obj.location.dot(Field::pipelineCache));
    if (pCreateInfos) {
        for (uint32_t index0 = 0; index0 < createInfoCount; ++index0) {
            [[maybe_unused]] const Location index0_loc = error_obj.location.dot(Field::pCreateInfos, index0);
            if (pCreateInfos[index0].pStages) {
                for (uint32_t index1 = 0; index1 < pCreateInfos[index0].stageCount; ++index1) {
                    [[maybe_unused]] const Location index1_loc = index0_loc.dot(Field::pStages, index1);
                    skip |= ValidateObject(pCreateInfos[index0].pStages[index1].module, kVulkanObjectTypeShaderModule, true,
                                           "VUID-VkPipelineShaderStageCreateInfo-module-parameter", kVUIDUndefined,
                                           index1_loc.dot(Field::module));
                    if (auto pNext = vku::FindStructInPNextChain<VkShaderModuleValidationCacheCreateInfoEXT>(
                            pCreateInfos[index0].pStages[index1].pNext)) {
                        const Location pNext_loc = index1_loc.pNext(Struct::VkShaderModuleValidationCacheCreateInfoEXT);
                        skip |= ValidateObject(pNext->validationCache, kVulkanObjectTypeValidationCacheEXT, false,
                                               "VUID-VkShaderModuleValidationCacheCreateInfoEXT-validationCache-parameter",
                                               kVUIDUndefined, pNext_loc.dot(Field::validationCache));
                    }
                }
            }
            skip |= ValidateObject(pCreateInfos[index0].layout, kVulkanObjectTypePipelineLayout, false,
                                   "VUID-VkRayTracingPipelineCreateInfoNV-layout-parameter",
                                   "VUID-VkRayTracingPipelineCreateInfoNV-commonparent", index0_loc.dot(Field::layout));
            if ((pCreateInfos[index0].flags & VK_PIPELINE_CREATE_DERIVATIVE_BIT) && (pCreateInfos[index0].basePipelineIndex == -1))
                skip |= ValidateObject(pCreateInfos[index0].basePipelineHandle, kVulkanObjectTypePipeline, false,
                                       "VUID-VkRayTracingPipelineCreateInfoNV-flags-07984",
                                       "VUID-VkRayTracingPipelineCreateInfoNV-commonparent", error_obj.location);
        }
    }

    return skip;
}

void ObjectLifetimes::PostCallRecordCreateRayTracingPipelinesNV(VkDevice device, VkPipelineCache pipelineCache,
                                                                uint32_t createInfoCount,
                                                                const VkRayTracingPipelineCreateInfoNV* pCreateInfos,
                                                                const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                                const RecordObject& record_obj) {
    if (VK_ERROR_VALIDATION_FAILED_EXT == record_obj.result) return;
    if (pPipelines) {
        for (uint32_t index = 0; index < createInfoCount; index++) {
            if (!pPipelines[index]) continue;
            CreateObject(pPipelines[index], kVulkanObjectTypePipeline, pAllocator);
        }
    }
}

bool ObjectLifetimes::PreCallValidateGetRayTracingShaderGroupHandlesKHR(VkDevice device, VkPipeline pipeline, uint32_t firstGroup,
                                                                        uint32_t groupCount, size_t dataSize, void* pData,
                                                                        const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkGetRayTracingShaderGroupHandlesKHR-device-parameter"
    skip |=
        ValidateObject(pipeline, kVulkanObjectTypePipeline, false, "VUID-vkGetRayTracingShaderGroupHandlesKHR-pipeline-parameter",
                       "VUID-vkGetRayTracingShaderGroupHandlesKHR-pipeline-parent", error_obj.location.dot(Field::pipeline));

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetRayTracingShaderGroupHandlesNV(VkDevice device, VkPipeline pipeline, uint32_t firstGroup,
                                                                       uint32_t groupCount, size_t dataSize, void* pData,
                                                                       const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkGetRayTracingShaderGroupHandlesKHR-device-parameter"
    skip |=
        ValidateObject(pipeline, kVulkanObjectTypePipeline, false, "VUID-vkGetRayTracingShaderGroupHandlesKHR-pipeline-parameter",
                       "VUID-vkGetRayTracingShaderGroupHandlesKHR-pipeline-parent", error_obj.location.dot(Field::pipeline));

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetAccelerationStructureHandleNV(VkDevice device,
                                                                      VkAccelerationStructureNV accelerationStructure,
                                                                      size_t dataSize, void* pData,
                                                                      const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkGetAccelerationStructureHandleNV-device-parameter"
    skip |= ValidateObject(accelerationStructure, kVulkanObjectTypeAccelerationStructureNV, false,
                           "VUID-vkGetAccelerationStructureHandleNV-accelerationStructure-parameter",
                           "VUID-vkGetAccelerationStructureHandleNV-accelerationStructure-parent",
                           error_obj.location.dot(Field::accelerationStructure));

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdWriteAccelerationStructuresPropertiesNV(
    VkCommandBuffer commandBuffer, uint32_t accelerationStructureCount, const VkAccelerationStructureNV* pAccelerationStructures,
    VkQueryType queryType, VkQueryPool queryPool, uint32_t firstQuery, const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: commandBuffer: "VUID-vkCmdWriteAccelerationStructuresPropertiesNV-commandBuffer-parameter"
    // Checked by chassis: commandBuffer: "VUID-vkCmdWriteAccelerationStructuresPropertiesNV-commonparent"

    if ((accelerationStructureCount > 0) && (pAccelerationStructures)) {
        for (uint32_t index0 = 0; index0 < accelerationStructureCount; ++index0) {
            skip |= ValidateObject(pAccelerationStructures[index0], kVulkanObjectTypeAccelerationStructureNV, false,
                                   "VUID-vkCmdWriteAccelerationStructuresPropertiesNV-pAccelerationStructures-parameter",
                                   "VUID-vkCmdWriteAccelerationStructuresPropertiesNV-commonparent",
                                   error_obj.location.dot(Field::pAccelerationStructures, index0));
        }
    }
    skip |= ValidateObject(
        queryPool, kVulkanObjectTypeQueryPool, false, "VUID-vkCmdWriteAccelerationStructuresPropertiesNV-queryPool-parameter",
        "VUID-vkCmdWriteAccelerationStructuresPropertiesNV-commonparent", error_obj.location.dot(Field::queryPool));

    return skip;
}

bool ObjectLifetimes::PreCallValidateCompileDeferredNV(VkDevice device, VkPipeline pipeline, uint32_t shader,
                                                       const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkCompileDeferredNV-device-parameter"
    skip |= ValidateObject(pipeline, kVulkanObjectTypePipeline, false, "VUID-vkCompileDeferredNV-pipeline-parameter",
                           "VUID-vkCompileDeferredNV-pipeline-parent", error_obj.location.dot(Field::pipeline));

    return skip;
}

// vkGetMemoryHostPointerPropertiesEXT:
// Checked by chassis: device: "VUID-vkGetMemoryHostPointerPropertiesEXT-device-parameter"

bool ObjectLifetimes::PreCallValidateCmdWriteBufferMarkerAMD(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage,
                                                             VkBuffer dstBuffer, VkDeviceSize dstOffset, uint32_t marker,
                                                             const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: commandBuffer: "VUID-vkCmdWriteBufferMarkerAMD-commandBuffer-parameter"
    // Checked by chassis: commandBuffer: "VUID-vkCmdWriteBufferMarkerAMD-commonparent"
    skip |= ValidateObject(dstBuffer, kVulkanObjectTypeBuffer, false, "VUID-vkCmdWriteBufferMarkerAMD-dstBuffer-parameter",
                           "VUID-vkCmdWriteBufferMarkerAMD-commonparent", error_obj.location.dot(Field::dstBuffer));

    return skip;
}

// vkGetPhysicalDeviceCalibrateableTimeDomainsEXT:
// Checked by chassis: physicalDevice: "VUID-vkGetPhysicalDeviceCalibrateableTimeDomainsEXT-physicalDevice-parameter"

// vkGetCalibratedTimestampsEXT:
// Checked by chassis: device: "VUID-vkGetCalibratedTimestampsEXT-device-parameter"

// vkCmdDrawMeshTasksNV:
// Checked by chassis: commandBuffer: "VUID-vkCmdDrawMeshTasksNV-commandBuffer-parameter"

bool ObjectLifetimes::PreCallValidateCmdDrawMeshTasksIndirectNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                                uint32_t drawCount, uint32_t stride,
                                                                const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: commandBuffer: "VUID-vkCmdDrawMeshTasksIndirectNV-commandBuffer-parameter"
    // Checked by chassis: commandBuffer: "VUID-vkCmdDrawMeshTasksIndirectNV-commonparent"
    skip |= ValidateObject(buffer, kVulkanObjectTypeBuffer, false, "VUID-vkCmdDrawMeshTasksIndirectNV-buffer-parameter",
                           "VUID-vkCmdDrawMeshTasksIndirectNV-commonparent", error_obj.location.dot(Field::buffer));

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdDrawMeshTasksIndirectCountNV(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                     VkDeviceSize offset, VkBuffer countBuffer,
                                                                     VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                                     uint32_t stride, const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: commandBuffer: "VUID-vkCmdDrawMeshTasksIndirectCountNV-commandBuffer-parameter"
    // Checked by chassis: commandBuffer: "VUID-vkCmdDrawMeshTasksIndirectCountNV-commonparent"
    skip |= ValidateObject(buffer, kVulkanObjectTypeBuffer, false, "VUID-vkCmdDrawMeshTasksIndirectCountNV-buffer-parameter",
                           "VUID-vkCmdDrawMeshTasksIndirectCountNV-commonparent", error_obj.location.dot(Field::buffer));
    skip |=
        ValidateObject(countBuffer, kVulkanObjectTypeBuffer, false, "VUID-vkCmdDrawMeshTasksIndirectCountNV-countBuffer-parameter",
                       "VUID-vkCmdDrawMeshTasksIndirectCountNV-commonparent", error_obj.location.dot(Field::countBuffer));

    return skip;
}

// vkCmdSetExclusiveScissorEnableNV:
// Checked by chassis: commandBuffer: "VUID-vkCmdSetExclusiveScissorEnableNV-commandBuffer-parameter"

// vkCmdSetExclusiveScissorNV:
// Checked by chassis: commandBuffer: "VUID-vkCmdSetExclusiveScissorNV-commandBuffer-parameter"

// vkCmdSetCheckpointNV:
// Checked by chassis: commandBuffer: "VUID-vkCmdSetCheckpointNV-commandBuffer-parameter"

// vkGetQueueCheckpointDataNV:
// Checked by chassis: queue: "VUID-vkGetQueueCheckpointDataNV-queue-parameter"

// vkInitializePerformanceApiINTEL:
// Checked by chassis: device: "VUID-vkInitializePerformanceApiINTEL-device-parameter"

// vkUninitializePerformanceApiINTEL:
// Checked by chassis: device: "VUID-vkUninitializePerformanceApiINTEL-device-parameter"

// vkCmdSetPerformanceMarkerINTEL:
// Checked by chassis: commandBuffer: "VUID-vkCmdSetPerformanceMarkerINTEL-commandBuffer-parameter"

// vkCmdSetPerformanceStreamMarkerINTEL:
// Checked by chassis: commandBuffer: "VUID-vkCmdSetPerformanceStreamMarkerINTEL-commandBuffer-parameter"

// vkCmdSetPerformanceOverrideINTEL:
// Checked by chassis: commandBuffer: "VUID-vkCmdSetPerformanceOverrideINTEL-commandBuffer-parameter"

// vkAcquirePerformanceConfigurationINTEL:
// Checked by chassis: device: "VUID-vkAcquirePerformanceConfigurationINTEL-device-parameter"

void ObjectLifetimes::PostCallRecordAcquirePerformanceConfigurationINTEL(
    VkDevice device, const VkPerformanceConfigurationAcquireInfoINTEL* pAcquireInfo,
    VkPerformanceConfigurationINTEL* pConfiguration, const RecordObject& record_obj) {
    if (record_obj.result != VK_SUCCESS) return;
    CreateObject(*pConfiguration, kVulkanObjectTypePerformanceConfigurationINTEL, nullptr);
}

bool ObjectLifetimes::PreCallValidateReleasePerformanceConfigurationINTEL(VkDevice device,
                                                                          VkPerformanceConfigurationINTEL configuration,
                                                                          const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkReleasePerformanceConfigurationINTEL-device-parameter"
    skip |= ValidateObject(configuration, kVulkanObjectTypePerformanceConfigurationINTEL, true,
                           "VUID-vkReleasePerformanceConfigurationINTEL-configuration-parameter",
                           "VUID-vkReleasePerformanceConfigurationINTEL-configuration-parent",
                           error_obj.location.dot(Field::configuration));
    skip |= ValidateDestroyObject(configuration, kVulkanObjectTypePerformanceConfigurationINTEL, nullptr, kVUIDUndefined,
                                  kVUIDUndefined, error_obj.location);

    return skip;
}

void ObjectLifetimes::PreCallRecordReleasePerformanceConfigurationINTEL(VkDevice device,
                                                                        VkPerformanceConfigurationINTEL configuration) {
    RecordDestroyObject(configuration, kVulkanObjectTypePerformanceConfigurationINTEL);
}

bool ObjectLifetimes::PreCallValidateQueueSetPerformanceConfigurationINTEL(VkQueue queue,
                                                                           VkPerformanceConfigurationINTEL configuration,
                                                                           const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: queue: "VUID-vkQueueSetPerformanceConfigurationINTEL-queue-parameter"
    // Checked by chassis: queue: "VUID-vkQueueSetPerformanceConfigurationINTEL-commonparent"
    skip |=
        ValidateObject(configuration, kVulkanObjectTypePerformanceConfigurationINTEL, false,
                       "VUID-vkQueueSetPerformanceConfigurationINTEL-configuration-parameter",
                       "VUID-vkQueueSetPerformanceConfigurationINTEL-commonparent", error_obj.location.dot(Field::configuration));

    return skip;
}

// vkGetPerformanceParameterINTEL:
// Checked by chassis: device: "VUID-vkGetPerformanceParameterINTEL-device-parameter"

bool ObjectLifetimes::PreCallValidateSetLocalDimmingAMD(VkDevice device, VkSwapchainKHR swapChain, VkBool32 localDimmingEnable,
                                                        const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkSetLocalDimmingAMD-device-parameter"
    skip |= ValidateObject(swapChain, kVulkanObjectTypeSwapchainKHR, false, "VUID-vkSetLocalDimmingAMD-swapChain-parameter",
                           "VUID-vkSetLocalDimmingAMD-swapChain-parent", error_obj.location.dot(Field::swapChain));

    return skip;
}
#ifdef VK_USE_PLATFORM_FUCHSIA

// vkCreateImagePipeSurfaceFUCHSIA:
// Checked by chassis: instance: "VUID-vkCreateImagePipeSurfaceFUCHSIA-instance-parameter"

void ObjectLifetimes::PostCallRecordCreateImagePipeSurfaceFUCHSIA(VkInstance instance,
                                                                  const VkImagePipeSurfaceCreateInfoFUCHSIA* pCreateInfo,
                                                                  const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                                  const RecordObject& record_obj) {
    if (record_obj.result != VK_SUCCESS) return;
    CreateObject(*pSurface, kVulkanObjectTypeSurfaceKHR, pAllocator);
}
#endif  // VK_USE_PLATFORM_FUCHSIA
#ifdef VK_USE_PLATFORM_METAL_EXT

// vkCreateMetalSurfaceEXT:
// Checked by chassis: instance: "VUID-vkCreateMetalSurfaceEXT-instance-parameter"

void ObjectLifetimes::PostCallRecordCreateMetalSurfaceEXT(VkInstance instance, const VkMetalSurfaceCreateInfoEXT* pCreateInfo,
                                                          const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                          const RecordObject& record_obj) {
    if (record_obj.result != VK_SUCCESS) return;
    CreateObject(*pSurface, kVulkanObjectTypeSurfaceKHR, pAllocator);
}
#endif  // VK_USE_PLATFORM_METAL_EXT

bool ObjectLifetimes::PreCallValidateGetBufferDeviceAddressEXT(VkDevice device, const VkBufferDeviceAddressInfo* pInfo,
                                                               const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkGetBufferDeviceAddress-device-parameter"
    if (pInfo) {
        [[maybe_unused]] const Location pInfo_loc = error_obj.location.dot(Field::pInfo);
        skip |= ValidateObject(pInfo->buffer, kVulkanObjectTypeBuffer, false, "VUID-VkBufferDeviceAddressInfo-buffer-parameter",
                               kVUIDUndefined, pInfo_loc.dot(Field::buffer));
    }

    return skip;
}

// vkGetPhysicalDeviceToolPropertiesEXT:
// Checked by chassis: physicalDevice: "VUID-vkGetPhysicalDeviceToolProperties-physicalDevice-parameter"

// vkGetPhysicalDeviceCooperativeMatrixPropertiesNV:
// Checked by chassis: physicalDevice: "VUID-vkGetPhysicalDeviceCooperativeMatrixPropertiesNV-physicalDevice-parameter"

// vkGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV:
// Checked by chassis: physicalDevice:
// "VUID-vkGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV-physicalDevice-parameter"

#ifdef VK_USE_PLATFORM_WIN32_KHR

bool ObjectLifetimes::PreCallValidateGetPhysicalDeviceSurfacePresentModes2EXT(VkPhysicalDevice physicalDevice,
                                                                              const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo,
                                                                              uint32_t* pPresentModeCount,
                                                                              VkPresentModeKHR* pPresentModes,
                                                                              const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: physicalDevice: "VUID-vkGetPhysicalDeviceSurfacePresentModes2EXT-physicalDevice-parameter"
    if (pSurfaceInfo) {
        [[maybe_unused]] const Location pSurfaceInfo_loc = error_obj.location.dot(Field::pSurfaceInfo);
        skip |= ValidateObject(pSurfaceInfo->surface, kVulkanObjectTypeSurfaceKHR, true,
                               "VUID-VkPhysicalDeviceSurfaceInfo2KHR-surface-parameter", kVUIDUndefined,
                               pSurfaceInfo_loc.dot(Field::surface));
    }

    return skip;
}
#endif  // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR

bool ObjectLifetimes::PreCallValidateAcquireFullScreenExclusiveModeEXT(VkDevice device, VkSwapchainKHR swapchain,
                                                                       const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkAcquireFullScreenExclusiveModeEXT-device-parameter"
    skip |= ValidateObject(swapchain, kVulkanObjectTypeSwapchainKHR, false,
                           "VUID-vkAcquireFullScreenExclusiveModeEXT-swapchain-parameter",
                           "VUID-vkAcquireFullScreenExclusiveModeEXT-swapchain-parent", error_obj.location.dot(Field::swapchain));

    return skip;
}
#endif  // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR

bool ObjectLifetimes::PreCallValidateReleaseFullScreenExclusiveModeEXT(VkDevice device, VkSwapchainKHR swapchain,
                                                                       const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: kVUIDUndefined
    skip |= ValidateObject(swapchain, kVulkanObjectTypeSwapchainKHR, false, kVUIDUndefined, kVUIDUndefined,
                           error_obj.location.dot(Field::swapchain));

    return skip;
}
#endif  // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR

bool ObjectLifetimes::PreCallValidateGetDeviceGroupSurfacePresentModes2EXT(VkDevice device,
                                                                           const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo,
                                                                           VkDeviceGroupPresentModeFlagsKHR* pModes,
                                                                           const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkGetDeviceGroupSurfacePresentModes2EXT-device-parameter"
    if (pSurfaceInfo) {
        [[maybe_unused]] const Location pSurfaceInfo_loc = error_obj.location.dot(Field::pSurfaceInfo);
        auto instance_data = GetLayerDataPtr(get_dispatch_key(instance), layer_data_map);
        auto instance_object_lifetimes = instance_data->GetValidationObject<ObjectLifetimes>();
        skip |= instance_object_lifetimes->ValidateObject(pSurfaceInfo->surface, kVulkanObjectTypeSurfaceKHR, true,
                                                          "VUID-VkPhysicalDeviceSurfaceInfo2KHR-surface-parameter", kVUIDUndefined,
                                                          pSurfaceInfo_loc.dot(Field::surface));
    }

    return skip;
}
#endif  // VK_USE_PLATFORM_WIN32_KHR

// vkCreateHeadlessSurfaceEXT:
// Checked by chassis: instance: "VUID-vkCreateHeadlessSurfaceEXT-instance-parameter"

void ObjectLifetimes::PostCallRecordCreateHeadlessSurfaceEXT(VkInstance instance, const VkHeadlessSurfaceCreateInfoEXT* pCreateInfo,
                                                             const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                             const RecordObject& record_obj) {
    if (record_obj.result != VK_SUCCESS) return;
    CreateObject(*pSurface, kVulkanObjectTypeSurfaceKHR, pAllocator);
}

// vkCmdSetLineStippleEXT:
// Checked by chassis: commandBuffer: "VUID-vkCmdSetLineStippleEXT-commandBuffer-parameter"

bool ObjectLifetimes::PreCallValidateResetQueryPoolEXT(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery,
                                                       uint32_t queryCount, const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkResetQueryPool-device-parameter"
    skip |= ValidateObject(queryPool, kVulkanObjectTypeQueryPool, false, "VUID-vkResetQueryPool-queryPool-parameter",
                           "VUID-vkResetQueryPool-queryPool-parent", error_obj.location.dot(Field::queryPool));

    return skip;
}

// vkCmdSetCullModeEXT:
// Checked by chassis: commandBuffer: "VUID-vkCmdSetCullMode-commandBuffer-parameter"

// vkCmdSetFrontFaceEXT:
// Checked by chassis: commandBuffer: "VUID-vkCmdSetFrontFace-commandBuffer-parameter"

// vkCmdSetPrimitiveTopologyEXT:
// Checked by chassis: commandBuffer: "VUID-vkCmdSetPrimitiveTopology-commandBuffer-parameter"

// vkCmdSetViewportWithCountEXT:
// Checked by chassis: commandBuffer: "VUID-vkCmdSetViewportWithCount-commandBuffer-parameter"

// vkCmdSetScissorWithCountEXT:
// Checked by chassis: commandBuffer: "VUID-vkCmdSetScissorWithCount-commandBuffer-parameter"

bool ObjectLifetimes::PreCallValidateCmdBindVertexBuffers2EXT(VkCommandBuffer commandBuffer, uint32_t firstBinding,
                                                              uint32_t bindingCount, const VkBuffer* pBuffers,
                                                              const VkDeviceSize* pOffsets, const VkDeviceSize* pSizes,
                                                              const VkDeviceSize* pStrides, const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: commandBuffer: "VUID-vkCmdBindVertexBuffers2-commandBuffer-parameter"
    // Checked by chassis: commandBuffer: "VUID-vkCmdBindVertexBuffers2-commonparent"

    if ((bindingCount > 0) && (pBuffers)) {
        for (uint32_t index0 = 0; index0 < bindingCount; ++index0) {
            skip |=
                ValidateObject(pBuffers[index0], kVulkanObjectTypeBuffer, true, "VUID-vkCmdBindVertexBuffers2-pBuffers-parameter",
                               "VUID-vkCmdBindVertexBuffers2-commonparent", error_obj.location.dot(Field::pBuffers, index0));
        }
    }

    return skip;
}

// vkCmdSetDepthTestEnableEXT:
// Checked by chassis: commandBuffer: "VUID-vkCmdSetDepthTestEnable-commandBuffer-parameter"

// vkCmdSetDepthWriteEnableEXT:
// Checked by chassis: commandBuffer: "VUID-vkCmdSetDepthWriteEnable-commandBuffer-parameter"

// vkCmdSetDepthCompareOpEXT:
// Checked by chassis: commandBuffer: "VUID-vkCmdSetDepthCompareOp-commandBuffer-parameter"

// vkCmdSetDepthBoundsTestEnableEXT:
// Checked by chassis: commandBuffer: "VUID-vkCmdSetDepthBoundsTestEnable-commandBuffer-parameter"

// vkCmdSetStencilTestEnableEXT:
// Checked by chassis: commandBuffer: "VUID-vkCmdSetStencilTestEnable-commandBuffer-parameter"

// vkCmdSetStencilOpEXT:
// Checked by chassis: commandBuffer: "VUID-vkCmdSetStencilOp-commandBuffer-parameter"

bool ObjectLifetimes::PreCallValidateCopyMemoryToImageEXT(VkDevice device, const VkCopyMemoryToImageInfoEXT* pCopyMemoryToImageInfo,
                                                          const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkCopyMemoryToImageEXT-device-parameter"
    if (pCopyMemoryToImageInfo) {
        [[maybe_unused]] const Location pCopyMemoryToImageInfo_loc = error_obj.location.dot(Field::pCopyMemoryToImageInfo);
        skip |= ValidateObject(pCopyMemoryToImageInfo->dstImage, kVulkanObjectTypeImage, false,
                               "VUID-VkCopyMemoryToImageInfoEXT-dstImage-parameter", kVUIDUndefined,
                               pCopyMemoryToImageInfo_loc.dot(Field::dstImage));
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateCopyImageToMemoryEXT(VkDevice device, const VkCopyImageToMemoryInfoEXT* pCopyImageToMemoryInfo,
                                                          const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkCopyImageToMemoryEXT-device-parameter"
    if (pCopyImageToMemoryInfo) {
        [[maybe_unused]] const Location pCopyImageToMemoryInfo_loc = error_obj.location.dot(Field::pCopyImageToMemoryInfo);
        skip |= ValidateObject(pCopyImageToMemoryInfo->srcImage, kVulkanObjectTypeImage, false,
                               "VUID-VkCopyImageToMemoryInfoEXT-srcImage-parameter", kVUIDUndefined,
                               pCopyImageToMemoryInfo_loc.dot(Field::srcImage));
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateCopyImageToImageEXT(VkDevice device, const VkCopyImageToImageInfoEXT* pCopyImageToImageInfo,
                                                         const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkCopyImageToImageEXT-device-parameter"
    if (pCopyImageToImageInfo) {
        [[maybe_unused]] const Location pCopyImageToImageInfo_loc = error_obj.location.dot(Field::pCopyImageToImageInfo);
        skip |= ValidateObject(pCopyImageToImageInfo->srcImage, kVulkanObjectTypeImage, false,
                               "VUID-VkCopyImageToImageInfoEXT-srcImage-parameter", "VUID-VkCopyImageToImageInfoEXT-commonparent",
                               pCopyImageToImageInfo_loc.dot(Field::srcImage));
        skip |= ValidateObject(pCopyImageToImageInfo->dstImage, kVulkanObjectTypeImage, false,
                               "VUID-VkCopyImageToImageInfoEXT-dstImage-parameter", "VUID-VkCopyImageToImageInfoEXT-commonparent",
                               pCopyImageToImageInfo_loc.dot(Field::dstImage));
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateTransitionImageLayoutEXT(VkDevice device, uint32_t transitionCount,
                                                              const VkHostImageLayoutTransitionInfoEXT* pTransitions,
                                                              const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkTransitionImageLayoutEXT-device-parameter"
    if (pTransitions) {
        for (uint32_t index0 = 0; index0 < transitionCount; ++index0) {
            [[maybe_unused]] const Location index0_loc = error_obj.location.dot(Field::pTransitions, index0);
            skip |= ValidateObject(pTransitions[index0].image, kVulkanObjectTypeImage, false,
                                   "VUID-VkHostImageLayoutTransitionInfoEXT-image-parameter", kVUIDUndefined,
                                   index0_loc.dot(Field::image));
        }
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetImageSubresourceLayout2EXT(VkDevice device, VkImage image,
                                                                   const VkImageSubresource2KHR* pSubresource,
                                                                   VkSubresourceLayout2KHR* pLayout,
                                                                   const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkGetImageSubresourceLayout2KHR-device-parameter"
    skip |= ValidateObject(image, kVulkanObjectTypeImage, false, "VUID-vkGetImageSubresourceLayout2KHR-image-parameter",
                           "VUID-vkGetImageSubresourceLayout2KHR-image-parent", error_obj.location.dot(Field::image));

    return skip;
}

bool ObjectLifetimes::PreCallValidateReleaseSwapchainImagesEXT(VkDevice device, const VkReleaseSwapchainImagesInfoEXT* pReleaseInfo,
                                                               const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkReleaseSwapchainImagesEXT-device-parameter"
    if (pReleaseInfo) {
        [[maybe_unused]] const Location pReleaseInfo_loc = error_obj.location.dot(Field::pReleaseInfo);
        skip |= ValidateObject(pReleaseInfo->swapchain, kVulkanObjectTypeSwapchainKHR, false,
                               "VUID-VkReleaseSwapchainImagesInfoEXT-swapchain-parameter", kVUIDUndefined,
                               pReleaseInfo_loc.dot(Field::swapchain));
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetGeneratedCommandsMemoryRequirementsNV(
    VkDevice device, const VkGeneratedCommandsMemoryRequirementsInfoNV* pInfo, VkMemoryRequirements2* pMemoryRequirements,
    const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkGetGeneratedCommandsMemoryRequirementsNV-device-parameter"
    if (pInfo) {
        [[maybe_unused]] const Location pInfo_loc = error_obj.location.dot(Field::pInfo);
        skip |= ValidateObject(pInfo->pipeline, kVulkanObjectTypePipeline, true,
                               "VUID-VkGeneratedCommandsMemoryRequirementsInfoNV-pipeline-parameter",
                               "VUID-VkGeneratedCommandsMemoryRequirementsInfoNV-commonparent", pInfo_loc.dot(Field::pipeline));
        skip |= ValidateObject(pInfo->indirectCommandsLayout, kVulkanObjectTypeIndirectCommandsLayoutNV, false,
                               "VUID-VkGeneratedCommandsMemoryRequirementsInfoNV-indirectCommandsLayout-parameter",
                               "VUID-VkGeneratedCommandsMemoryRequirementsInfoNV-commonparent",
                               pInfo_loc.dot(Field::indirectCommandsLayout));
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdPreprocessGeneratedCommandsNV(VkCommandBuffer commandBuffer,
                                                                      const VkGeneratedCommandsInfoNV* pGeneratedCommandsInfo,
                                                                      const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: commandBuffer: "VUID-vkCmdPreprocessGeneratedCommandsNV-commandBuffer-parameter"
    if (pGeneratedCommandsInfo) {
        [[maybe_unused]] const Location pGeneratedCommandsInfo_loc = error_obj.location.dot(Field::pGeneratedCommandsInfo);
        skip |= ValidateObject(pGeneratedCommandsInfo->pipeline, kVulkanObjectTypePipeline, false,
                               "VUID-VkGeneratedCommandsInfoNV-pipeline-parameter", "VUID-VkGeneratedCommandsInfoNV-commonparent",
                               pGeneratedCommandsInfo_loc.dot(Field::pipeline));
        skip |= ValidateObject(pGeneratedCommandsInfo->indirectCommandsLayout, kVulkanObjectTypeIndirectCommandsLayoutNV, false,
                               "VUID-VkGeneratedCommandsInfoNV-indirectCommandsLayout-parameter",
                               "VUID-VkGeneratedCommandsInfoNV-commonparent",
                               pGeneratedCommandsInfo_loc.dot(Field::indirectCommandsLayout));
        if (pGeneratedCommandsInfo->pStreams) {
            for (uint32_t index1 = 0; index1 < pGeneratedCommandsInfo->streamCount; ++index1) {
                [[maybe_unused]] const Location index1_loc = pGeneratedCommandsInfo_loc.dot(Field::pStreams, index1);
                skip |= ValidateObject(pGeneratedCommandsInfo->pStreams[index1].buffer, kVulkanObjectTypeBuffer, false,
                                       "VUID-VkIndirectCommandsStreamNV-buffer-parameter", kVUIDUndefined,
                                       index1_loc.dot(Field::buffer));
            }
        }
        skip |=
            ValidateObject(pGeneratedCommandsInfo->preprocessBuffer, kVulkanObjectTypeBuffer, false,
                           "VUID-VkGeneratedCommandsInfoNV-preprocessBuffer-parameter",
                           "VUID-VkGeneratedCommandsInfoNV-commonparent", pGeneratedCommandsInfo_loc.dot(Field::preprocessBuffer));
        skip |= ValidateObject(pGeneratedCommandsInfo->sequencesCountBuffer, kVulkanObjectTypeBuffer, true,
                               "VUID-VkGeneratedCommandsInfoNV-sequencesCountBuffer-parameter",
                               "VUID-VkGeneratedCommandsInfoNV-commonparent",
                               pGeneratedCommandsInfo_loc.dot(Field::sequencesCountBuffer));
        skip |= ValidateObject(pGeneratedCommandsInfo->sequencesIndexBuffer, kVulkanObjectTypeBuffer, true,
                               "VUID-VkGeneratedCommandsInfoNV-sequencesIndexBuffer-parameter",
                               "VUID-VkGeneratedCommandsInfoNV-commonparent",
                               pGeneratedCommandsInfo_loc.dot(Field::sequencesIndexBuffer));
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdExecuteGeneratedCommandsNV(VkCommandBuffer commandBuffer, VkBool32 isPreprocessed,
                                                                   const VkGeneratedCommandsInfoNV* pGeneratedCommandsInfo,
                                                                   const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: commandBuffer: "VUID-vkCmdExecuteGeneratedCommandsNV-commandBuffer-parameter"
    if (pGeneratedCommandsInfo) {
        [[maybe_unused]] const Location pGeneratedCommandsInfo_loc = error_obj.location.dot(Field::pGeneratedCommandsInfo);
        skip |= ValidateObject(pGeneratedCommandsInfo->pipeline, kVulkanObjectTypePipeline, false,
                               "VUID-VkGeneratedCommandsInfoNV-pipeline-parameter", "VUID-VkGeneratedCommandsInfoNV-commonparent",
                               pGeneratedCommandsInfo_loc.dot(Field::pipeline));
        skip |= ValidateObject(pGeneratedCommandsInfo->indirectCommandsLayout, kVulkanObjectTypeIndirectCommandsLayoutNV, false,
                               "VUID-VkGeneratedCommandsInfoNV-indirectCommandsLayout-parameter",
                               "VUID-VkGeneratedCommandsInfoNV-commonparent",
                               pGeneratedCommandsInfo_loc.dot(Field::indirectCommandsLayout));
        if (pGeneratedCommandsInfo->pStreams) {
            for (uint32_t index1 = 0; index1 < pGeneratedCommandsInfo->streamCount; ++index1) {
                [[maybe_unused]] const Location index1_loc = pGeneratedCommandsInfo_loc.dot(Field::pStreams, index1);
                skip |= ValidateObject(pGeneratedCommandsInfo->pStreams[index1].buffer, kVulkanObjectTypeBuffer, false,
                                       "VUID-VkIndirectCommandsStreamNV-buffer-parameter", kVUIDUndefined,
                                       index1_loc.dot(Field::buffer));
            }
        }
        skip |=
            ValidateObject(pGeneratedCommandsInfo->preprocessBuffer, kVulkanObjectTypeBuffer, false,
                           "VUID-VkGeneratedCommandsInfoNV-preprocessBuffer-parameter",
                           "VUID-VkGeneratedCommandsInfoNV-commonparent", pGeneratedCommandsInfo_loc.dot(Field::preprocessBuffer));
        skip |= ValidateObject(pGeneratedCommandsInfo->sequencesCountBuffer, kVulkanObjectTypeBuffer, true,
                               "VUID-VkGeneratedCommandsInfoNV-sequencesCountBuffer-parameter",
                               "VUID-VkGeneratedCommandsInfoNV-commonparent",
                               pGeneratedCommandsInfo_loc.dot(Field::sequencesCountBuffer));
        skip |= ValidateObject(pGeneratedCommandsInfo->sequencesIndexBuffer, kVulkanObjectTypeBuffer, true,
                               "VUID-VkGeneratedCommandsInfoNV-sequencesIndexBuffer-parameter",
                               "VUID-VkGeneratedCommandsInfoNV-commonparent",
                               pGeneratedCommandsInfo_loc.dot(Field::sequencesIndexBuffer));
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdBindPipelineShaderGroupNV(VkCommandBuffer commandBuffer,
                                                                  VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline,
                                                                  uint32_t groupIndex, const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: commandBuffer: "VUID-vkCmdBindPipelineShaderGroupNV-commandBuffer-parameter"
    // Checked by chassis: commandBuffer: "VUID-vkCmdBindPipelineShaderGroupNV-commonparent"
    skip |= ValidateObject(pipeline, kVulkanObjectTypePipeline, false, "VUID-vkCmdBindPipelineShaderGroupNV-pipeline-parameter",
                           "VUID-vkCmdBindPipelineShaderGroupNV-commonparent", error_obj.location.dot(Field::pipeline));

    return skip;
}

bool ObjectLifetimes::PreCallValidateCreateIndirectCommandsLayoutNV(VkDevice device,
                                                                    const VkIndirectCommandsLayoutCreateInfoNV* pCreateInfo,
                                                                    const VkAllocationCallbacks* pAllocator,
                                                                    VkIndirectCommandsLayoutNV* pIndirectCommandsLayout,
                                                                    const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkCreateIndirectCommandsLayoutNV-device-parameter"
    if (pCreateInfo) {
        [[maybe_unused]] const Location pCreateInfo_loc = error_obj.location.dot(Field::pCreateInfo);
        if (pCreateInfo->pTokens) {
            for (uint32_t index1 = 0; index1 < pCreateInfo->tokenCount; ++index1) {
                [[maybe_unused]] const Location index1_loc = pCreateInfo_loc.dot(Field::pTokens, index1);
                skip |= ValidateObject(pCreateInfo->pTokens[index1].pushconstantPipelineLayout, kVulkanObjectTypePipelineLayout,
                                       true, "VUID-VkIndirectCommandsLayoutTokenNV-pushconstantPipelineLayout-parameter",
                                       kVUIDUndefined, index1_loc.dot(Field::pushconstantPipelineLayout));
            }
        }
    }

    return skip;
}

void ObjectLifetimes::PostCallRecordCreateIndirectCommandsLayoutNV(VkDevice device,
                                                                   const VkIndirectCommandsLayoutCreateInfoNV* pCreateInfo,
                                                                   const VkAllocationCallbacks* pAllocator,
                                                                   VkIndirectCommandsLayoutNV* pIndirectCommandsLayout,
                                                                   const RecordObject& record_obj) {
    if (record_obj.result != VK_SUCCESS) return;
    CreateObject(*pIndirectCommandsLayout, kVulkanObjectTypeIndirectCommandsLayoutNV, pAllocator);
}

bool ObjectLifetimes::PreCallValidateDestroyIndirectCommandsLayoutNV(VkDevice device,
                                                                     VkIndirectCommandsLayoutNV indirectCommandsLayout,
                                                                     const VkAllocationCallbacks* pAllocator,
                                                                     const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkDestroyIndirectCommandsLayoutNV-device-parameter"
    skip |= ValidateObject(indirectCommandsLayout, kVulkanObjectTypeIndirectCommandsLayoutNV, true,
                           "VUID-vkDestroyIndirectCommandsLayoutNV-indirectCommandsLayout-parameter",
                           "VUID-vkDestroyIndirectCommandsLayoutNV-indirectCommandsLayout-parent",
                           error_obj.location.dot(Field::indirectCommandsLayout));
    skip |= ValidateDestroyObject(indirectCommandsLayout, kVulkanObjectTypeIndirectCommandsLayoutNV, pAllocator, kVUIDUndefined,
                                  kVUIDUndefined, error_obj.location);

    return skip;
}

void ObjectLifetimes::PreCallRecordDestroyIndirectCommandsLayoutNV(VkDevice device,
                                                                   VkIndirectCommandsLayoutNV indirectCommandsLayout,
                                                                   const VkAllocationCallbacks* pAllocator) {
    RecordDestroyObject(indirectCommandsLayout, kVulkanObjectTypeIndirectCommandsLayoutNV);
}

// vkCmdSetDepthBias2EXT:
// Checked by chassis: commandBuffer: "VUID-vkCmdSetDepthBias2EXT-commandBuffer-parameter"

bool ObjectLifetimes::PreCallValidateAcquireDrmDisplayEXT(VkPhysicalDevice physicalDevice, int32_t drmFd, VkDisplayKHR display,
                                                          const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: physicalDevice: "VUID-vkAcquireDrmDisplayEXT-physicalDevice-parameter"
    skip |= ValidateObject(display, kVulkanObjectTypeDisplayKHR, false, "VUID-vkAcquireDrmDisplayEXT-display-parameter",
                           "VUID-vkAcquireDrmDisplayEXT-display-parent", error_obj.location.dot(Field::display),
                           kVulkanObjectTypePhysicalDevice);

    return skip;
}

// vkGetDrmDisplayEXT:
// Checked by chassis: physicalDevice: "VUID-vkGetDrmDisplayEXT-physicalDevice-parameter"

void ObjectLifetimes::PostCallRecordGetDrmDisplayEXT(VkPhysicalDevice physicalDevice, int32_t drmFd, uint32_t connectorId,
                                                     VkDisplayKHR* display, const RecordObject& record_obj) {
    if (record_obj.result != VK_SUCCESS) return;
    CreateObject(*display, kVulkanObjectTypeDisplayKHR, nullptr);
}

// vkCreatePrivateDataSlotEXT:
// Checked by chassis: device: "VUID-vkCreatePrivateDataSlot-device-parameter"

void ObjectLifetimes::PostCallRecordCreatePrivateDataSlotEXT(VkDevice device, const VkPrivateDataSlotCreateInfo* pCreateInfo,
                                                             const VkAllocationCallbacks* pAllocator,
                                                             VkPrivateDataSlot* pPrivateDataSlot, const RecordObject& record_obj) {
    if (record_obj.result != VK_SUCCESS) return;
    CreateObject(*pPrivateDataSlot, kVulkanObjectTypePrivateDataSlot, pAllocator);
}

bool ObjectLifetimes::PreCallValidateDestroyPrivateDataSlotEXT(VkDevice device, VkPrivateDataSlot privateDataSlot,
                                                               const VkAllocationCallbacks* pAllocator,
                                                               const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkDestroyPrivateDataSlot-device-parameter"
    skip |= ValidateObject(privateDataSlot, kVulkanObjectTypePrivateDataSlot, true,
                           "VUID-vkDestroyPrivateDataSlot-privateDataSlot-parameter",
                           "VUID-vkDestroyPrivateDataSlot-privateDataSlot-parent", error_obj.location.dot(Field::privateDataSlot));
    skip |= ValidateDestroyObject(privateDataSlot, kVulkanObjectTypePrivateDataSlot, pAllocator, kVUIDUndefined, kVUIDUndefined,
                                  error_obj.location);

    return skip;
}

void ObjectLifetimes::PreCallRecordDestroyPrivateDataSlotEXT(VkDevice device, VkPrivateDataSlot privateDataSlot,
                                                             const VkAllocationCallbacks* pAllocator) {
    RecordDestroyObject(privateDataSlot, kVulkanObjectTypePrivateDataSlot);
}

bool ObjectLifetimes::PreCallValidateSetPrivateDataEXT(VkDevice device, VkObjectType objectType, uint64_t objectHandle,
                                                       VkPrivateDataSlot privateDataSlot, uint64_t data,
                                                       const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkSetPrivateData-device-parameter"
    skip |=
        ValidateObject(privateDataSlot, kVulkanObjectTypePrivateDataSlot, false, "VUID-vkSetPrivateData-privateDataSlot-parameter",
                       "VUID-vkSetPrivateData-privateDataSlot-parent", error_obj.location.dot(Field::privateDataSlot));

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetPrivateDataEXT(VkDevice device, VkObjectType objectType, uint64_t objectHandle,
                                                       VkPrivateDataSlot privateDataSlot, uint64_t* pData,
                                                       const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkGetPrivateData-device-parameter"
    skip |=
        ValidateObject(privateDataSlot, kVulkanObjectTypePrivateDataSlot, false, "VUID-vkGetPrivateData-privateDataSlot-parameter",
                       "VUID-vkGetPrivateData-privateDataSlot-parent", error_obj.location.dot(Field::privateDataSlot));

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetDescriptorSetLayoutSizeEXT(VkDevice device, VkDescriptorSetLayout layout,
                                                                   VkDeviceSize* pLayoutSizeInBytes,
                                                                   const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkGetDescriptorSetLayoutSizeEXT-device-parameter"
    skip |=
        ValidateObject(layout, kVulkanObjectTypeDescriptorSetLayout, false, "VUID-vkGetDescriptorSetLayoutSizeEXT-layout-parameter",
                       "VUID-vkGetDescriptorSetLayoutSizeEXT-layout-parent", error_obj.location.dot(Field::layout));

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetDescriptorSetLayoutBindingOffsetEXT(VkDevice device, VkDescriptorSetLayout layout,
                                                                            uint32_t binding, VkDeviceSize* pOffset,
                                                                            const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkGetDescriptorSetLayoutBindingOffsetEXT-device-parameter"
    skip |= ValidateObject(layout, kVulkanObjectTypeDescriptorSetLayout, false,
                           "VUID-vkGetDescriptorSetLayoutBindingOffsetEXT-layout-parameter",
                           "VUID-vkGetDescriptorSetLayoutBindingOffsetEXT-layout-parent", error_obj.location.dot(Field::layout));

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdBindDescriptorBuffersEXT(VkCommandBuffer commandBuffer, uint32_t bufferCount,
                                                                 const VkDescriptorBufferBindingInfoEXT* pBindingInfos,
                                                                 const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: commandBuffer: "VUID-vkCmdBindDescriptorBuffersEXT-commandBuffer-parameter"
    if (pBindingInfos) {
        for (uint32_t index0 = 0; index0 < bufferCount; ++index0) {
            [[maybe_unused]] const Location index0_loc = error_obj.location.dot(Field::pBindingInfos, index0);
            if (auto pNext = vku::FindStructInPNextChain<VkDescriptorBufferBindingPushDescriptorBufferHandleEXT>(
                    pBindingInfos[index0].pNext)) {
                const Location pNext_loc = index0_loc.pNext(Struct::VkDescriptorBufferBindingPushDescriptorBufferHandleEXT);
                skip |= ValidateObject(pNext->buffer, kVulkanObjectTypeBuffer, false,
                                       "VUID-VkDescriptorBufferBindingPushDescriptorBufferHandleEXT-buffer-parameter",
                                       kVUIDUndefined, pNext_loc.dot(Field::buffer));
            }
        }
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdSetDescriptorBufferOffsetsEXT(VkCommandBuffer commandBuffer,
                                                                      VkPipelineBindPoint pipelineBindPoint,
                                                                      VkPipelineLayout layout, uint32_t firstSet, uint32_t setCount,
                                                                      const uint32_t* pBufferIndices, const VkDeviceSize* pOffsets,
                                                                      const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: commandBuffer: "VUID-vkCmdSetDescriptorBufferOffsetsEXT-commandBuffer-parameter"
    // Checked by chassis: commandBuffer: "VUID-vkCmdSetDescriptorBufferOffsetsEXT-commonparent"
    skip |=
        ValidateObject(layout, kVulkanObjectTypePipelineLayout, false, "VUID-vkCmdSetDescriptorBufferOffsetsEXT-layout-parameter",
                       "VUID-vkCmdSetDescriptorBufferOffsetsEXT-commonparent", error_obj.location.dot(Field::layout));

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdBindDescriptorBufferEmbeddedSamplersEXT(VkCommandBuffer commandBuffer,
                                                                                VkPipelineBindPoint pipelineBindPoint,
                                                                                VkPipelineLayout layout, uint32_t set,
                                                                                const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: commandBuffer: "VUID-vkCmdBindDescriptorBufferEmbeddedSamplersEXT-commandBuffer-parameter"
    // Checked by chassis: commandBuffer: "VUID-vkCmdBindDescriptorBufferEmbeddedSamplersEXT-commonparent"
    skip |= ValidateObject(layout, kVulkanObjectTypePipelineLayout, false,
                           "VUID-vkCmdBindDescriptorBufferEmbeddedSamplersEXT-layout-parameter",
                           "VUID-vkCmdBindDescriptorBufferEmbeddedSamplersEXT-commonparent", error_obj.location.dot(Field::layout));

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetBufferOpaqueCaptureDescriptorDataEXT(VkDevice device,
                                                                             const VkBufferCaptureDescriptorDataInfoEXT* pInfo,
                                                                             void* pData, const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkGetBufferOpaqueCaptureDescriptorDataEXT-device-parameter"
    if (pInfo) {
        [[maybe_unused]] const Location pInfo_loc = error_obj.location.dot(Field::pInfo);
        skip |= ValidateObject(pInfo->buffer, kVulkanObjectTypeBuffer, false,
                               "VUID-VkBufferCaptureDescriptorDataInfoEXT-buffer-parameter", kVUIDUndefined,
                               pInfo_loc.dot(Field::buffer));
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetImageOpaqueCaptureDescriptorDataEXT(VkDevice device,
                                                                            const VkImageCaptureDescriptorDataInfoEXT* pInfo,
                                                                            void* pData, const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkGetImageOpaqueCaptureDescriptorDataEXT-device-parameter"
    if (pInfo) {
        [[maybe_unused]] const Location pInfo_loc = error_obj.location.dot(Field::pInfo);
        skip |=
            ValidateObject(pInfo->image, kVulkanObjectTypeImage, false, "VUID-VkImageCaptureDescriptorDataInfoEXT-image-parameter",
                           kVUIDUndefined, pInfo_loc.dot(Field::image));
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetImageViewOpaqueCaptureDescriptorDataEXT(
    VkDevice device, const VkImageViewCaptureDescriptorDataInfoEXT* pInfo, void* pData, const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkGetImageViewOpaqueCaptureDescriptorDataEXT-device-parameter"
    if (pInfo) {
        [[maybe_unused]] const Location pInfo_loc = error_obj.location.dot(Field::pInfo);
        skip |= ValidateObject(pInfo->imageView, kVulkanObjectTypeImageView, false,
                               "VUID-VkImageViewCaptureDescriptorDataInfoEXT-imageView-parameter", kVUIDUndefined,
                               pInfo_loc.dot(Field::imageView));
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetSamplerOpaqueCaptureDescriptorDataEXT(VkDevice device,
                                                                              const VkSamplerCaptureDescriptorDataInfoEXT* pInfo,
                                                                              void* pData, const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkGetSamplerOpaqueCaptureDescriptorDataEXT-device-parameter"
    if (pInfo) {
        [[maybe_unused]] const Location pInfo_loc = error_obj.location.dot(Field::pInfo);
        skip |= ValidateObject(pInfo->sampler, kVulkanObjectTypeSampler, false,
                               "VUID-VkSamplerCaptureDescriptorDataInfoEXT-sampler-parameter", kVUIDUndefined,
                               pInfo_loc.dot(Field::sampler));
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetAccelerationStructureOpaqueCaptureDescriptorDataEXT(
    VkDevice device, const VkAccelerationStructureCaptureDescriptorDataInfoEXT* pInfo, void* pData,
    const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkGetAccelerationStructureOpaqueCaptureDescriptorDataEXT-device-parameter"
    if (pInfo) {
        [[maybe_unused]] const Location pInfo_loc = error_obj.location.dot(Field::pInfo);
        skip |= ValidateObject(pInfo->accelerationStructure, kVulkanObjectTypeAccelerationStructureKHR, true,
                               "VUID-VkAccelerationStructureCaptureDescriptorDataInfoEXT-accelerationStructure-parameter",
                               "VUID-VkAccelerationStructureCaptureDescriptorDataInfoEXT-commonparent",
                               pInfo_loc.dot(Field::accelerationStructure));
        skip |= ValidateObject(pInfo->accelerationStructureNV, kVulkanObjectTypeAccelerationStructureNV, true,
                               "VUID-VkAccelerationStructureCaptureDescriptorDataInfoEXT-accelerationStructureNV-parameter",
                               "VUID-VkAccelerationStructureCaptureDescriptorDataInfoEXT-commonparent",
                               pInfo_loc.dot(Field::accelerationStructureNV));
    }

    return skip;
}

// vkCmdSetFragmentShadingRateEnumNV:
// Checked by chassis: commandBuffer: "VUID-vkCmdSetFragmentShadingRateEnumNV-commandBuffer-parameter"

// vkGetDeviceFaultInfoEXT:
// Checked by chassis: device: "VUID-vkGetDeviceFaultInfoEXT-device-parameter"

#ifdef VK_USE_PLATFORM_WIN32_KHR

bool ObjectLifetimes::PreCallValidateAcquireWinrtDisplayNV(VkPhysicalDevice physicalDevice, VkDisplayKHR display,
                                                           const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: physicalDevice: "VUID-vkAcquireWinrtDisplayNV-physicalDevice-parameter"
    skip |= ValidateObject(display, kVulkanObjectTypeDisplayKHR, false, "VUID-vkAcquireWinrtDisplayNV-display-parameter",
                           "VUID-vkAcquireWinrtDisplayNV-display-parent", error_obj.location.dot(Field::display),
                           kVulkanObjectTypePhysicalDevice);

    return skip;
}
#endif  // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR

// vkGetWinrtDisplayNV:
// Checked by chassis: physicalDevice: "VUID-vkGetWinrtDisplayNV-physicalDevice-parameter"

void ObjectLifetimes::PostCallRecordGetWinrtDisplayNV(VkPhysicalDevice physicalDevice, uint32_t deviceRelativeId,
                                                      VkDisplayKHR* pDisplay, const RecordObject& record_obj) {
    if (record_obj.result != VK_SUCCESS) return;
    CreateObject(*pDisplay, kVulkanObjectTypeDisplayKHR, nullptr);
}
#endif  // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_DIRECTFB_EXT

// vkCreateDirectFBSurfaceEXT:
// Checked by chassis: instance: "VUID-vkCreateDirectFBSurfaceEXT-instance-parameter"

void ObjectLifetimes::PostCallRecordCreateDirectFBSurfaceEXT(VkInstance instance, const VkDirectFBSurfaceCreateInfoEXT* pCreateInfo,
                                                             const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                             const RecordObject& record_obj) {
    if (record_obj.result != VK_SUCCESS) return;
    CreateObject(*pSurface, kVulkanObjectTypeSurfaceKHR, pAllocator);
}
#endif  // VK_USE_PLATFORM_DIRECTFB_EXT
#ifdef VK_USE_PLATFORM_DIRECTFB_EXT

// vkGetPhysicalDeviceDirectFBPresentationSupportEXT:
// Checked by chassis: physicalDevice: "VUID-vkGetPhysicalDeviceDirectFBPresentationSupportEXT-physicalDevice-parameter"

#endif  // VK_USE_PLATFORM_DIRECTFB_EXT

// vkCmdSetVertexInputEXT:
// Checked by chassis: commandBuffer: "VUID-vkCmdSetVertexInputEXT-commandBuffer-parameter"

#ifdef VK_USE_PLATFORM_FUCHSIA

bool ObjectLifetimes::PreCallValidateGetMemoryZirconHandleFUCHSIA(VkDevice device,
                                                                  const VkMemoryGetZirconHandleInfoFUCHSIA* pGetZirconHandleInfo,
                                                                  zx_handle_t* pZirconHandle, const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkGetMemoryZirconHandleFUCHSIA-device-parameter"
    if (pGetZirconHandleInfo) {
        [[maybe_unused]] const Location pGetZirconHandleInfo_loc = error_obj.location.dot(Field::pGetZirconHandleInfo);
        skip |= ValidateObject(pGetZirconHandleInfo->memory, kVulkanObjectTypeDeviceMemory, false,
                               "VUID-VkMemoryGetZirconHandleInfoFUCHSIA-memory-parameter", kVUIDUndefined,
                               pGetZirconHandleInfo_loc.dot(Field::memory));
    }

    return skip;
}
#endif  // VK_USE_PLATFORM_FUCHSIA
#ifdef VK_USE_PLATFORM_FUCHSIA

// vkGetMemoryZirconHandlePropertiesFUCHSIA:
// Checked by chassis: device: "VUID-vkGetMemoryZirconHandlePropertiesFUCHSIA-device-parameter"

#endif  // VK_USE_PLATFORM_FUCHSIA
#ifdef VK_USE_PLATFORM_FUCHSIA

bool ObjectLifetimes::PreCallValidateImportSemaphoreZirconHandleFUCHSIA(
    VkDevice device, const VkImportSemaphoreZirconHandleInfoFUCHSIA* pImportSemaphoreZirconHandleInfo,
    const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkImportSemaphoreZirconHandleFUCHSIA-device-parameter"
    if (pImportSemaphoreZirconHandleInfo) {
        [[maybe_unused]] const Location pImportSemaphoreZirconHandleInfo_loc =
            error_obj.location.dot(Field::pImportSemaphoreZirconHandleInfo);
        skip |= ValidateObject(pImportSemaphoreZirconHandleInfo->semaphore, kVulkanObjectTypeSemaphore, false,
                               "VUID-VkImportSemaphoreZirconHandleInfoFUCHSIA-semaphore-parameter", kVUIDUndefined,
                               pImportSemaphoreZirconHandleInfo_loc.dot(Field::semaphore));
    }

    return skip;
}
#endif  // VK_USE_PLATFORM_FUCHSIA
#ifdef VK_USE_PLATFORM_FUCHSIA

bool ObjectLifetimes::PreCallValidateGetSemaphoreZirconHandleFUCHSIA(
    VkDevice device, const VkSemaphoreGetZirconHandleInfoFUCHSIA* pGetZirconHandleInfo, zx_handle_t* pZirconHandle,
    const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkGetSemaphoreZirconHandleFUCHSIA-device-parameter"
    if (pGetZirconHandleInfo) {
        [[maybe_unused]] const Location pGetZirconHandleInfo_loc = error_obj.location.dot(Field::pGetZirconHandleInfo);
        skip |= ValidateObject(pGetZirconHandleInfo->semaphore, kVulkanObjectTypeSemaphore, false,
                               "VUID-VkSemaphoreGetZirconHandleInfoFUCHSIA-semaphore-parameter", kVUIDUndefined,
                               pGetZirconHandleInfo_loc.dot(Field::semaphore));
    }

    return skip;
}
#endif  // VK_USE_PLATFORM_FUCHSIA
#ifdef VK_USE_PLATFORM_FUCHSIA

// vkCreateBufferCollectionFUCHSIA:
// Checked by chassis: device: "VUID-vkCreateBufferCollectionFUCHSIA-device-parameter"

void ObjectLifetimes::PostCallRecordCreateBufferCollectionFUCHSIA(VkDevice device,
                                                                  const VkBufferCollectionCreateInfoFUCHSIA* pCreateInfo,
                                                                  const VkAllocationCallbacks* pAllocator,
                                                                  VkBufferCollectionFUCHSIA* pCollection,
                                                                  const RecordObject& record_obj) {
    if (record_obj.result != VK_SUCCESS) return;
    CreateObject(*pCollection, kVulkanObjectTypeBufferCollectionFUCHSIA, pAllocator);
}
#endif  // VK_USE_PLATFORM_FUCHSIA
#ifdef VK_USE_PLATFORM_FUCHSIA

bool ObjectLifetimes::PreCallValidateSetBufferCollectionImageConstraintsFUCHSIA(
    VkDevice device, VkBufferCollectionFUCHSIA collection, const VkImageConstraintsInfoFUCHSIA* pImageConstraintsInfo,
    const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkSetBufferCollectionImageConstraintsFUCHSIA-device-parameter"
    skip |= ValidateObject(collection, kVulkanObjectTypeBufferCollectionFUCHSIA, false,
                           "VUID-vkSetBufferCollectionImageConstraintsFUCHSIA-collection-parameter",
                           "VUID-vkSetBufferCollectionImageConstraintsFUCHSIA-collection-parent",
                           error_obj.location.dot(Field::collection));

    return skip;
}
#endif  // VK_USE_PLATFORM_FUCHSIA
#ifdef VK_USE_PLATFORM_FUCHSIA

bool ObjectLifetimes::PreCallValidateSetBufferCollectionBufferConstraintsFUCHSIA(
    VkDevice device, VkBufferCollectionFUCHSIA collection, const VkBufferConstraintsInfoFUCHSIA* pBufferConstraintsInfo,
    const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkSetBufferCollectionBufferConstraintsFUCHSIA-device-parameter"
    skip |= ValidateObject(collection, kVulkanObjectTypeBufferCollectionFUCHSIA, false,
                           "VUID-vkSetBufferCollectionBufferConstraintsFUCHSIA-collection-parameter",
                           "VUID-vkSetBufferCollectionBufferConstraintsFUCHSIA-collection-parent",
                           error_obj.location.dot(Field::collection));

    return skip;
}
#endif  // VK_USE_PLATFORM_FUCHSIA
#ifdef VK_USE_PLATFORM_FUCHSIA

bool ObjectLifetimes::PreCallValidateDestroyBufferCollectionFUCHSIA(VkDevice device, VkBufferCollectionFUCHSIA collection,
                                                                    const VkAllocationCallbacks* pAllocator,
                                                                    const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkDestroyBufferCollectionFUCHSIA-device-parameter"
    skip |= ValidateObject(collection, kVulkanObjectTypeBufferCollectionFUCHSIA, false,
                           "VUID-vkDestroyBufferCollectionFUCHSIA-collection-parameter",
                           "VUID-vkDestroyBufferCollectionFUCHSIA-collection-parent", error_obj.location.dot(Field::collection));
    skip |= ValidateDestroyObject(collection, kVulkanObjectTypeBufferCollectionFUCHSIA, pAllocator, kVUIDUndefined, kVUIDUndefined,
                                  error_obj.location);

    return skip;
}

void ObjectLifetimes::PreCallRecordDestroyBufferCollectionFUCHSIA(VkDevice device, VkBufferCollectionFUCHSIA collection,
                                                                  const VkAllocationCallbacks* pAllocator) {
    RecordDestroyObject(collection, kVulkanObjectTypeBufferCollectionFUCHSIA);
}
#endif  // VK_USE_PLATFORM_FUCHSIA
#ifdef VK_USE_PLATFORM_FUCHSIA

bool ObjectLifetimes::PreCallValidateGetBufferCollectionPropertiesFUCHSIA(VkDevice device, VkBufferCollectionFUCHSIA collection,
                                                                          VkBufferCollectionPropertiesFUCHSIA* pProperties,
                                                                          const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkGetBufferCollectionPropertiesFUCHSIA-device-parameter"
    skip |=
        ValidateObject(collection, kVulkanObjectTypeBufferCollectionFUCHSIA, false,
                       "VUID-vkGetBufferCollectionPropertiesFUCHSIA-collection-parameter",
                       "VUID-vkGetBufferCollectionPropertiesFUCHSIA-collection-parent", error_obj.location.dot(Field::collection));

    return skip;
}
#endif  // VK_USE_PLATFORM_FUCHSIA

bool ObjectLifetimes::PreCallValidateGetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI(VkDevice device, VkRenderPass renderpass,
                                                                                   VkExtent2D* pMaxWorkgroupSize,
                                                                                   const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkGetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI-device-parameter"
    skip |= ValidateObject(
        renderpass, kVulkanObjectTypeRenderPass, false, "VUID-vkGetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI-renderpass-parameter",
        "VUID-vkGetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI-renderpass-parent", error_obj.location.dot(Field::renderpass));

    return skip;
}

// vkCmdSubpassShadingHUAWEI:
// Checked by chassis: commandBuffer: "VUID-vkCmdSubpassShadingHUAWEI-commandBuffer-parameter"

bool ObjectLifetimes::PreCallValidateCmdBindInvocationMaskHUAWEI(VkCommandBuffer commandBuffer, VkImageView imageView,
                                                                 VkImageLayout imageLayout, const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: commandBuffer: "VUID-vkCmdBindInvocationMaskHUAWEI-commandBuffer-parameter"
    // Checked by chassis: commandBuffer: "VUID-vkCmdBindInvocationMaskHUAWEI-commonparent"
    skip |= ValidateObject(imageView, kVulkanObjectTypeImageView, true, "VUID-vkCmdBindInvocationMaskHUAWEI-imageView-parameter",
                           "VUID-vkCmdBindInvocationMaskHUAWEI-commonparent", error_obj.location.dot(Field::imageView));

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetMemoryRemoteAddressNV(VkDevice device,
                                                              const VkMemoryGetRemoteAddressInfoNV* pMemoryGetRemoteAddressInfo,
                                                              VkRemoteAddressNV* pAddress, const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkGetMemoryRemoteAddressNV-device-parameter"
    if (pMemoryGetRemoteAddressInfo) {
        [[maybe_unused]] const Location pMemoryGetRemoteAddressInfo_loc =
            error_obj.location.dot(Field::pMemoryGetRemoteAddressInfo);
        skip |= ValidateObject(pMemoryGetRemoteAddressInfo->memory, kVulkanObjectTypeDeviceMemory, false,
                               "VUID-VkMemoryGetRemoteAddressInfoNV-memory-parameter", kVUIDUndefined,
                               pMemoryGetRemoteAddressInfo_loc.dot(Field::memory));
    }

    return skip;
}

// vkGetPipelinePropertiesEXT:
// Checked by chassis: device: "VUID-vkGetPipelinePropertiesEXT-device-parameter"

// vkCmdSetPatchControlPointsEXT:
// Checked by chassis: commandBuffer: "VUID-vkCmdSetPatchControlPointsEXT-commandBuffer-parameter"

// vkCmdSetRasterizerDiscardEnableEXT:
// Checked by chassis: commandBuffer: "VUID-vkCmdSetRasterizerDiscardEnable-commandBuffer-parameter"

// vkCmdSetDepthBiasEnableEXT:
// Checked by chassis: commandBuffer: "VUID-vkCmdSetDepthBiasEnable-commandBuffer-parameter"

// vkCmdSetLogicOpEXT:
// Checked by chassis: commandBuffer: "VUID-vkCmdSetLogicOpEXT-commandBuffer-parameter"

// vkCmdSetPrimitiveRestartEnableEXT:
// Checked by chassis: commandBuffer: "VUID-vkCmdSetPrimitiveRestartEnable-commandBuffer-parameter"

#ifdef VK_USE_PLATFORM_SCREEN_QNX

// vkCreateScreenSurfaceQNX:
// Checked by chassis: instance: "VUID-vkCreateScreenSurfaceQNX-instance-parameter"

void ObjectLifetimes::PostCallRecordCreateScreenSurfaceQNX(VkInstance instance, const VkScreenSurfaceCreateInfoQNX* pCreateInfo,
                                                           const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                           const RecordObject& record_obj) {
    if (record_obj.result != VK_SUCCESS) return;
    CreateObject(*pSurface, kVulkanObjectTypeSurfaceKHR, pAllocator);
}
#endif  // VK_USE_PLATFORM_SCREEN_QNX
#ifdef VK_USE_PLATFORM_SCREEN_QNX

// vkGetPhysicalDeviceScreenPresentationSupportQNX:
// Checked by chassis: physicalDevice: "VUID-vkGetPhysicalDeviceScreenPresentationSupportQNX-physicalDevice-parameter"

#endif  // VK_USE_PLATFORM_SCREEN_QNX

// vkCmdSetColorWriteEnableEXT:
// Checked by chassis: commandBuffer: "VUID-vkCmdSetColorWriteEnableEXT-commandBuffer-parameter"

// vkCmdDrawMultiEXT:
// Checked by chassis: commandBuffer: "VUID-vkCmdDrawMultiEXT-commandBuffer-parameter"

// vkCmdDrawMultiIndexedEXT:
// Checked by chassis: commandBuffer: "VUID-vkCmdDrawMultiIndexedEXT-commandBuffer-parameter"

bool ObjectLifetimes::PreCallValidateCreateMicromapEXT(VkDevice device, const VkMicromapCreateInfoEXT* pCreateInfo,
                                                       const VkAllocationCallbacks* pAllocator, VkMicromapEXT* pMicromap,
                                                       const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkCreateMicromapEXT-device-parameter"
    if (pCreateInfo) {
        [[maybe_unused]] const Location pCreateInfo_loc = error_obj.location.dot(Field::pCreateInfo);
        skip |= ValidateObject(pCreateInfo->buffer, kVulkanObjectTypeBuffer, false, "VUID-VkMicromapCreateInfoEXT-buffer-parameter",
                               kVUIDUndefined, pCreateInfo_loc.dot(Field::buffer));
    }

    return skip;
}

void ObjectLifetimes::PostCallRecordCreateMicromapEXT(VkDevice device, const VkMicromapCreateInfoEXT* pCreateInfo,
                                                      const VkAllocationCallbacks* pAllocator, VkMicromapEXT* pMicromap,
                                                      const RecordObject& record_obj) {
    if (record_obj.result != VK_SUCCESS) return;
    CreateObject(*pMicromap, kVulkanObjectTypeMicromapEXT, pAllocator);
}

bool ObjectLifetimes::PreCallValidateDestroyMicromapEXT(VkDevice device, VkMicromapEXT micromap,
                                                        const VkAllocationCallbacks* pAllocator,
                                                        const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkDestroyMicromapEXT-device-parameter"
    skip |= ValidateObject(micromap, kVulkanObjectTypeMicromapEXT, true, "VUID-vkDestroyMicromapEXT-micromap-parameter",
                           "VUID-vkDestroyMicromapEXT-micromap-parent", error_obj.location.dot(Field::micromap));
    skip |= ValidateDestroyObject(micromap, kVulkanObjectTypeMicromapEXT, pAllocator, kVUIDUndefined, kVUIDUndefined,
                                  error_obj.location);

    return skip;
}

void ObjectLifetimes::PreCallRecordDestroyMicromapEXT(VkDevice device, VkMicromapEXT micromap,
                                                      const VkAllocationCallbacks* pAllocator) {
    RecordDestroyObject(micromap, kVulkanObjectTypeMicromapEXT);
}

bool ObjectLifetimes::PreCallValidateCmdBuildMicromapsEXT(VkCommandBuffer commandBuffer, uint32_t infoCount,
                                                          const VkMicromapBuildInfoEXT* pInfos,
                                                          const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: commandBuffer: "VUID-vkCmdBuildMicromapsEXT-commandBuffer-parameter"
    if (pInfos) {
        for (uint32_t index0 = 0; index0 < infoCount; ++index0) {
            [[maybe_unused]] const Location index0_loc = error_obj.location.dot(Field::pInfos, index0);
            skip |= ValidateObject(pInfos[index0].dstMicromap, kVulkanObjectTypeMicromapEXT, true, kVUIDUndefined, kVUIDUndefined,
                                   index0_loc.dot(Field::dstMicromap));
        }
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateBuildMicromapsEXT(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                       uint32_t infoCount, const VkMicromapBuildInfoEXT* pInfos,
                                                       const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkBuildMicromapsEXT-device-parameter"
    skip |= ValidateObject(deferredOperation, kVulkanObjectTypeDeferredOperationKHR, true,
                           "VUID-vkBuildMicromapsEXT-deferredOperation-parameter",
                           "VUID-vkBuildMicromapsEXT-deferredOperation-parent", error_obj.location.dot(Field::deferredOperation));
    if (pInfos) {
        for (uint32_t index0 = 0; index0 < infoCount; ++index0) {
            [[maybe_unused]] const Location index0_loc = error_obj.location.dot(Field::pInfos, index0);
            skip |= ValidateObject(pInfos[index0].dstMicromap, kVulkanObjectTypeMicromapEXT, true, kVUIDUndefined, kVUIDUndefined,
                                   index0_loc.dot(Field::dstMicromap));
        }
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateCopyMicromapEXT(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                     const VkCopyMicromapInfoEXT* pInfo, const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkCopyMicromapEXT-device-parameter"
    skip |= ValidateObject(deferredOperation, kVulkanObjectTypeDeferredOperationKHR, true,
                           "VUID-vkCopyMicromapEXT-deferredOperation-parameter", "VUID-vkCopyMicromapEXT-deferredOperation-parent",
                           error_obj.location.dot(Field::deferredOperation));
    if (pInfo) {
        [[maybe_unused]] const Location pInfo_loc = error_obj.location.dot(Field::pInfo);
        skip |= ValidateObject(pInfo->src, kVulkanObjectTypeMicromapEXT, false, "VUID-VkCopyMicromapInfoEXT-src-parameter",
                               "VUID-VkCopyMicromapInfoEXT-commonparent", pInfo_loc.dot(Field::src));
        skip |= ValidateObject(pInfo->dst, kVulkanObjectTypeMicromapEXT, false, "VUID-VkCopyMicromapInfoEXT-dst-parameter",
                               "VUID-VkCopyMicromapInfoEXT-commonparent", pInfo_loc.dot(Field::dst));
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateCopyMicromapToMemoryEXT(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                             const VkCopyMicromapToMemoryInfoEXT* pInfo,
                                                             const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkCopyMicromapToMemoryEXT-device-parameter"
    skip |=
        ValidateObject(deferredOperation, kVulkanObjectTypeDeferredOperationKHR, true,
                       "VUID-vkCopyMicromapToMemoryEXT-deferredOperation-parameter",
                       "VUID-vkCopyMicromapToMemoryEXT-deferredOperation-parent", error_obj.location.dot(Field::deferredOperation));
    if (pInfo) {
        [[maybe_unused]] const Location pInfo_loc = error_obj.location.dot(Field::pInfo);
        skip |= ValidateObject(pInfo->src, kVulkanObjectTypeMicromapEXT, false, "VUID-VkCopyMicromapToMemoryInfoEXT-src-parameter",
                               kVUIDUndefined, pInfo_loc.dot(Field::src));
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateCopyMemoryToMicromapEXT(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                             const VkCopyMemoryToMicromapInfoEXT* pInfo,
                                                             const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkCopyMemoryToMicromapEXT-device-parameter"
    skip |=
        ValidateObject(deferredOperation, kVulkanObjectTypeDeferredOperationKHR, true,
                       "VUID-vkCopyMemoryToMicromapEXT-deferredOperation-parameter",
                       "VUID-vkCopyMemoryToMicromapEXT-deferredOperation-parent", error_obj.location.dot(Field::deferredOperation));
    if (pInfo) {
        [[maybe_unused]] const Location pInfo_loc = error_obj.location.dot(Field::pInfo);
        skip |= ValidateObject(pInfo->dst, kVulkanObjectTypeMicromapEXT, false, "VUID-VkCopyMemoryToMicromapInfoEXT-dst-parameter",
                               kVUIDUndefined, pInfo_loc.dot(Field::dst));
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateWriteMicromapsPropertiesEXT(VkDevice device, uint32_t micromapCount,
                                                                 const VkMicromapEXT* pMicromaps, VkQueryType queryType,
                                                                 size_t dataSize, void* pData, size_t stride,
                                                                 const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkWriteMicromapsPropertiesEXT-device-parameter"

    if ((micromapCount > 0) && (pMicromaps)) {
        for (uint32_t index0 = 0; index0 < micromapCount; ++index0) {
            skip |= ValidateObject(
                pMicromaps[index0], kVulkanObjectTypeMicromapEXT, false, "VUID-vkWriteMicromapsPropertiesEXT-pMicromaps-parameter",
                "VUID-vkWriteMicromapsPropertiesEXT-pMicromaps-parent", error_obj.location.dot(Field::pMicromaps, index0));
        }
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdCopyMicromapEXT(VkCommandBuffer commandBuffer, const VkCopyMicromapInfoEXT* pInfo,
                                                        const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: commandBuffer: "VUID-vkCmdCopyMicromapEXT-commandBuffer-parameter"
    if (pInfo) {
        [[maybe_unused]] const Location pInfo_loc = error_obj.location.dot(Field::pInfo);
        skip |= ValidateObject(pInfo->src, kVulkanObjectTypeMicromapEXT, false, "VUID-VkCopyMicromapInfoEXT-src-parameter",
                               "VUID-VkCopyMicromapInfoEXT-commonparent", pInfo_loc.dot(Field::src));
        skip |= ValidateObject(pInfo->dst, kVulkanObjectTypeMicromapEXT, false, "VUID-VkCopyMicromapInfoEXT-dst-parameter",
                               "VUID-VkCopyMicromapInfoEXT-commonparent", pInfo_loc.dot(Field::dst));
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdCopyMicromapToMemoryEXT(VkCommandBuffer commandBuffer,
                                                                const VkCopyMicromapToMemoryInfoEXT* pInfo,
                                                                const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: commandBuffer: "VUID-vkCmdCopyMicromapToMemoryEXT-commandBuffer-parameter"
    if (pInfo) {
        [[maybe_unused]] const Location pInfo_loc = error_obj.location.dot(Field::pInfo);
        skip |= ValidateObject(pInfo->src, kVulkanObjectTypeMicromapEXT, false, "VUID-VkCopyMicromapToMemoryInfoEXT-src-parameter",
                               kVUIDUndefined, pInfo_loc.dot(Field::src));
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdCopyMemoryToMicromapEXT(VkCommandBuffer commandBuffer,
                                                                const VkCopyMemoryToMicromapInfoEXT* pInfo,
                                                                const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: commandBuffer: "VUID-vkCmdCopyMemoryToMicromapEXT-commandBuffer-parameter"
    if (pInfo) {
        [[maybe_unused]] const Location pInfo_loc = error_obj.location.dot(Field::pInfo);
        skip |= ValidateObject(pInfo->dst, kVulkanObjectTypeMicromapEXT, false, "VUID-VkCopyMemoryToMicromapInfoEXT-dst-parameter",
                               kVUIDUndefined, pInfo_loc.dot(Field::dst));
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdWriteMicromapsPropertiesEXT(VkCommandBuffer commandBuffer, uint32_t micromapCount,
                                                                    const VkMicromapEXT* pMicromaps, VkQueryType queryType,
                                                                    VkQueryPool queryPool, uint32_t firstQuery,
                                                                    const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: commandBuffer: "VUID-vkCmdWriteMicromapsPropertiesEXT-commandBuffer-parameter"
    // Checked by chassis: commandBuffer: "VUID-vkCmdWriteMicromapsPropertiesEXT-commonparent"

    if ((micromapCount > 0) && (pMicromaps)) {
        for (uint32_t index0 = 0; index0 < micromapCount; ++index0) {
            skip |= ValidateObject(pMicromaps[index0], kVulkanObjectTypeMicromapEXT, false,
                                   "VUID-vkCmdWriteMicromapsPropertiesEXT-pMicromaps-parameter",
                                   "VUID-vkCmdWriteMicromapsPropertiesEXT-commonparent",
                                   error_obj.location.dot(Field::pMicromaps, index0));
        }
    }
    skip |=
        ValidateObject(queryPool, kVulkanObjectTypeQueryPool, false, "VUID-vkCmdWriteMicromapsPropertiesEXT-queryPool-parameter",
                       "VUID-vkCmdWriteMicromapsPropertiesEXT-commonparent", error_obj.location.dot(Field::queryPool));

    return skip;
}

// vkGetDeviceMicromapCompatibilityEXT:
// Checked by chassis: device: "VUID-vkGetDeviceMicromapCompatibilityEXT-device-parameter"

bool ObjectLifetimes::PreCallValidateGetMicromapBuildSizesEXT(VkDevice device, VkAccelerationStructureBuildTypeKHR buildType,
                                                              const VkMicromapBuildInfoEXT* pBuildInfo,
                                                              VkMicromapBuildSizesInfoEXT* pSizeInfo,
                                                              const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkGetMicromapBuildSizesEXT-device-parameter"
    if (pBuildInfo) {
        [[maybe_unused]] const Location pBuildInfo_loc = error_obj.location.dot(Field::pBuildInfo);
        skip |= ValidateObject(pBuildInfo->dstMicromap, kVulkanObjectTypeMicromapEXT, true, kVUIDUndefined, kVUIDUndefined,
                               pBuildInfo_loc.dot(Field::dstMicromap));
    }

    return skip;
}

// vkCmdDrawClusterHUAWEI:
// Checked by chassis: commandBuffer: "VUID-vkCmdDrawClusterHUAWEI-commandBuffer-parameter"

bool ObjectLifetimes::PreCallValidateCmdDrawClusterIndirectHUAWEI(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                  VkDeviceSize offset, const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: commandBuffer: "VUID-vkCmdDrawClusterIndirectHUAWEI-commandBuffer-parameter"
    // Checked by chassis: commandBuffer: "VUID-vkCmdDrawClusterIndirectHUAWEI-commonparent"
    skip |= ValidateObject(buffer, kVulkanObjectTypeBuffer, false, "VUID-vkCmdDrawClusterIndirectHUAWEI-buffer-parameter",
                           "VUID-vkCmdDrawClusterIndirectHUAWEI-commonparent", error_obj.location.dot(Field::buffer));

    return skip;
}

bool ObjectLifetimes::PreCallValidateSetDeviceMemoryPriorityEXT(VkDevice device, VkDeviceMemory memory, float priority,
                                                                const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkSetDeviceMemoryPriorityEXT-device-parameter"
    skip |= ValidateObject(memory, kVulkanObjectTypeDeviceMemory, false, "VUID-vkSetDeviceMemoryPriorityEXT-memory-parameter",
                           "VUID-vkSetDeviceMemoryPriorityEXT-memory-parent", error_obj.location.dot(Field::memory));

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetDescriptorSetLayoutHostMappingInfoVALVE(
    VkDevice device, const VkDescriptorSetBindingReferenceVALVE* pBindingReference,
    VkDescriptorSetLayoutHostMappingInfoVALVE* pHostMapping, const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkGetDescriptorSetLayoutHostMappingInfoVALVE-device-parameter"
    if (pBindingReference) {
        [[maybe_unused]] const Location pBindingReference_loc = error_obj.location.dot(Field::pBindingReference);
        skip |= ValidateObject(pBindingReference->descriptorSetLayout, kVulkanObjectTypeDescriptorSetLayout, false,
                               "VUID-VkDescriptorSetBindingReferenceVALVE-descriptorSetLayout-parameter", kVUIDUndefined,
                               pBindingReference_loc.dot(Field::descriptorSetLayout));
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetDescriptorSetHostMappingVALVE(VkDevice device, VkDescriptorSet descriptorSet, void** ppData,
                                                                      const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkGetDescriptorSetHostMappingVALVE-device-parameter"
    skip |= ValidateObject(
        descriptorSet, kVulkanObjectTypeDescriptorSet, false, "VUID-vkGetDescriptorSetHostMappingVALVE-descriptorSet-parameter",
        "VUID-vkGetDescriptorSetHostMappingVALVE-descriptorSet-parent", error_obj.location.dot(Field::descriptorSet));

    return skip;
}

// vkCmdCopyMemoryIndirectNV:
// Checked by chassis: commandBuffer: "VUID-vkCmdCopyMemoryIndirectNV-commandBuffer-parameter"

bool ObjectLifetimes::PreCallValidateCmdCopyMemoryToImageIndirectNV(VkCommandBuffer commandBuffer,
                                                                    VkDeviceAddress copyBufferAddress, uint32_t copyCount,
                                                                    uint32_t stride, VkImage dstImage, VkImageLayout dstImageLayout,
                                                                    const VkImageSubresourceLayers* pImageSubresources,
                                                                    const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: commandBuffer: "VUID-vkCmdCopyMemoryToImageIndirectNV-commandBuffer-parameter"
    // Checked by chassis: commandBuffer: "VUID-vkCmdCopyMemoryToImageIndirectNV-commonparent"
    skip |= ValidateObject(dstImage, kVulkanObjectTypeImage, false, "VUID-vkCmdCopyMemoryToImageIndirectNV-dstImage-parameter",
                           "VUID-vkCmdCopyMemoryToImageIndirectNV-commonparent", error_obj.location.dot(Field::dstImage));

    return skip;
}

// vkCmdDecompressMemoryNV:
// Checked by chassis: commandBuffer: "VUID-vkCmdDecompressMemoryNV-commandBuffer-parameter"

// vkCmdDecompressMemoryIndirectCountNV:
// Checked by chassis: commandBuffer: "VUID-vkCmdDecompressMemoryIndirectCountNV-commandBuffer-parameter"

bool ObjectLifetimes::PreCallValidateGetPipelineIndirectMemoryRequirementsNV(VkDevice device,
                                                                             const VkComputePipelineCreateInfo* pCreateInfo,
                                                                             VkMemoryRequirements2* pMemoryRequirements,
                                                                             const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkGetPipelineIndirectMemoryRequirementsNV-device-parameter"
    if (pCreateInfo) {
        [[maybe_unused]] const Location pCreateInfo_loc = error_obj.location.dot(Field::pCreateInfo);
        [[maybe_unused]] const Location stage_loc = pCreateInfo_loc.dot(Field::stage);
        skip |=
            ValidateObject(pCreateInfo->stage.module, kVulkanObjectTypeShaderModule, true,
                           "VUID-VkPipelineShaderStageCreateInfo-module-parameter", kVUIDUndefined, stage_loc.dot(Field::module));
        if (auto pNext = vku::FindStructInPNextChain<VkShaderModuleValidationCacheCreateInfoEXT>(pCreateInfo->stage.pNext)) {
            const Location pNext_loc = stage_loc.pNext(Struct::VkShaderModuleValidationCacheCreateInfoEXT);
            skip |= ValidateObject(pNext->validationCache, kVulkanObjectTypeValidationCacheEXT, false,
                                   "VUID-VkShaderModuleValidationCacheCreateInfoEXT-validationCache-parameter", kVUIDUndefined,
                                   pNext_loc.dot(Field::validationCache));
        }
        skip |= ValidateObject(pCreateInfo->layout, kVulkanObjectTypePipelineLayout, false,
                               "VUID-VkComputePipelineCreateInfo-layout-parameter", "VUID-VkComputePipelineCreateInfo-commonparent",
                               pCreateInfo_loc.dot(Field::layout));
        if ((pCreateInfo->flags & VK_PIPELINE_CREATE_DERIVATIVE_BIT) && (pCreateInfo->basePipelineIndex == -1))
            skip |= ValidateObject(pCreateInfo->basePipelineHandle, kVulkanObjectTypePipeline, false,
                                   "VUID-VkComputePipelineCreateInfo-flags-07984", "VUID-VkComputePipelineCreateInfo-commonparent",
                                   error_obj.location);
        if (auto pNext = vku::FindStructInPNextChain<VkSubpassShadingPipelineCreateInfoHUAWEI>(pCreateInfo->pNext)) {
            const Location pNext_loc = pCreateInfo_loc.pNext(Struct::VkSubpassShadingPipelineCreateInfoHUAWEI);
            skip |= ValidateObject(pNext->renderPass, kVulkanObjectTypeRenderPass, false, kVUIDUndefined, kVUIDUndefined,
                                   pNext_loc.dot(Field::renderPass));
        }
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdUpdatePipelineIndirectBufferNV(VkCommandBuffer commandBuffer,
                                                                       VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline,
                                                                       const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: commandBuffer: "VUID-vkCmdUpdatePipelineIndirectBufferNV-commandBuffer-parameter"
    // Checked by chassis: commandBuffer: "VUID-vkCmdUpdatePipelineIndirectBufferNV-commonparent"
    skip |=
        ValidateObject(pipeline, kVulkanObjectTypePipeline, false, "VUID-vkCmdUpdatePipelineIndirectBufferNV-pipeline-parameter",
                       "VUID-vkCmdUpdatePipelineIndirectBufferNV-commonparent", error_obj.location.dot(Field::pipeline));

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetPipelineIndirectDeviceAddressNV(VkDevice device,
                                                                        const VkPipelineIndirectDeviceAddressInfoNV* pInfo,
                                                                        const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkGetPipelineIndirectDeviceAddressNV-device-parameter"
    if (pInfo) {
        [[maybe_unused]] const Location pInfo_loc = error_obj.location.dot(Field::pInfo);
        skip |= ValidateObject(pInfo->pipeline, kVulkanObjectTypePipeline, false,
                               "VUID-VkPipelineIndirectDeviceAddressInfoNV-pipeline-parameter", kVUIDUndefined,
                               pInfo_loc.dot(Field::pipeline));
    }

    return skip;
}

// vkCmdSetTessellationDomainOriginEXT:
// Checked by chassis: commandBuffer: "VUID-vkCmdSetTessellationDomainOriginEXT-commandBuffer-parameter"

// vkCmdSetDepthClampEnableEXT:
// Checked by chassis: commandBuffer: "VUID-vkCmdSetDepthClampEnableEXT-commandBuffer-parameter"

// vkCmdSetPolygonModeEXT:
// Checked by chassis: commandBuffer: "VUID-vkCmdSetPolygonModeEXT-commandBuffer-parameter"

// vkCmdSetRasterizationSamplesEXT:
// Checked by chassis: commandBuffer: "VUID-vkCmdSetRasterizationSamplesEXT-commandBuffer-parameter"

// vkCmdSetSampleMaskEXT:
// Checked by chassis: commandBuffer: "VUID-vkCmdSetSampleMaskEXT-commandBuffer-parameter"

// vkCmdSetAlphaToCoverageEnableEXT:
// Checked by chassis: commandBuffer: "VUID-vkCmdSetAlphaToCoverageEnableEXT-commandBuffer-parameter"

// vkCmdSetAlphaToOneEnableEXT:
// Checked by chassis: commandBuffer: "VUID-vkCmdSetAlphaToOneEnableEXT-commandBuffer-parameter"

// vkCmdSetLogicOpEnableEXT:
// Checked by chassis: commandBuffer: "VUID-vkCmdSetLogicOpEnableEXT-commandBuffer-parameter"

// vkCmdSetColorBlendEnableEXT:
// Checked by chassis: commandBuffer: "VUID-vkCmdSetColorBlendEnableEXT-commandBuffer-parameter"

// vkCmdSetColorBlendEquationEXT:
// Checked by chassis: commandBuffer: "VUID-vkCmdSetColorBlendEquationEXT-commandBuffer-parameter"

// vkCmdSetColorWriteMaskEXT:
// Checked by chassis: commandBuffer: "VUID-vkCmdSetColorWriteMaskEXT-commandBuffer-parameter"

// vkCmdSetRasterizationStreamEXT:
// Checked by chassis: commandBuffer: "VUID-vkCmdSetRasterizationStreamEXT-commandBuffer-parameter"

// vkCmdSetConservativeRasterizationModeEXT:
// Checked by chassis: commandBuffer: "VUID-vkCmdSetConservativeRasterizationModeEXT-commandBuffer-parameter"

// vkCmdSetExtraPrimitiveOverestimationSizeEXT:
// Checked by chassis: commandBuffer: "VUID-vkCmdSetExtraPrimitiveOverestimationSizeEXT-commandBuffer-parameter"

// vkCmdSetDepthClipEnableEXT:
// Checked by chassis: commandBuffer: "VUID-vkCmdSetDepthClipEnableEXT-commandBuffer-parameter"

// vkCmdSetSampleLocationsEnableEXT:
// Checked by chassis: commandBuffer: "VUID-vkCmdSetSampleLocationsEnableEXT-commandBuffer-parameter"

// vkCmdSetColorBlendAdvancedEXT:
// Checked by chassis: commandBuffer: "VUID-vkCmdSetColorBlendAdvancedEXT-commandBuffer-parameter"

// vkCmdSetProvokingVertexModeEXT:
// Checked by chassis: commandBuffer: "VUID-vkCmdSetProvokingVertexModeEXT-commandBuffer-parameter"

// vkCmdSetLineRasterizationModeEXT:
// Checked by chassis: commandBuffer: "VUID-vkCmdSetLineRasterizationModeEXT-commandBuffer-parameter"

// vkCmdSetLineStippleEnableEXT:
// Checked by chassis: commandBuffer: "VUID-vkCmdSetLineStippleEnableEXT-commandBuffer-parameter"

// vkCmdSetDepthClipNegativeOneToOneEXT:
// Checked by chassis: commandBuffer: "VUID-vkCmdSetDepthClipNegativeOneToOneEXT-commandBuffer-parameter"

// vkCmdSetViewportWScalingEnableNV:
// Checked by chassis: commandBuffer: "VUID-vkCmdSetViewportWScalingEnableNV-commandBuffer-parameter"

// vkCmdSetViewportSwizzleNV:
// Checked by chassis: commandBuffer: "VUID-vkCmdSetViewportSwizzleNV-commandBuffer-parameter"

// vkCmdSetCoverageToColorEnableNV:
// Checked by chassis: commandBuffer: "VUID-vkCmdSetCoverageToColorEnableNV-commandBuffer-parameter"

// vkCmdSetCoverageToColorLocationNV:
// Checked by chassis: commandBuffer: "VUID-vkCmdSetCoverageToColorLocationNV-commandBuffer-parameter"

// vkCmdSetCoverageModulationModeNV:
// Checked by chassis: commandBuffer: "VUID-vkCmdSetCoverageModulationModeNV-commandBuffer-parameter"

// vkCmdSetCoverageModulationTableEnableNV:
// Checked by chassis: commandBuffer: "VUID-vkCmdSetCoverageModulationTableEnableNV-commandBuffer-parameter"

// vkCmdSetCoverageModulationTableNV:
// Checked by chassis: commandBuffer: "VUID-vkCmdSetCoverageModulationTableNV-commandBuffer-parameter"

// vkCmdSetShadingRateImageEnableNV:
// Checked by chassis: commandBuffer: "VUID-vkCmdSetShadingRateImageEnableNV-commandBuffer-parameter"

// vkCmdSetRepresentativeFragmentTestEnableNV:
// Checked by chassis: commandBuffer: "VUID-vkCmdSetRepresentativeFragmentTestEnableNV-commandBuffer-parameter"

// vkCmdSetCoverageReductionModeNV:
// Checked by chassis: commandBuffer: "VUID-vkCmdSetCoverageReductionModeNV-commandBuffer-parameter"

bool ObjectLifetimes::PreCallValidateGetShaderModuleIdentifierEXT(VkDevice device, VkShaderModule shaderModule,
                                                                  VkShaderModuleIdentifierEXT* pIdentifier,
                                                                  const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkGetShaderModuleIdentifierEXT-device-parameter"
    skip |= ValidateObject(shaderModule, kVulkanObjectTypeShaderModule, false,
                           "VUID-vkGetShaderModuleIdentifierEXT-shaderModule-parameter",
                           "VUID-vkGetShaderModuleIdentifierEXT-shaderModule-parent", error_obj.location.dot(Field::shaderModule));

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetShaderModuleCreateInfoIdentifierEXT(VkDevice device,
                                                                            const VkShaderModuleCreateInfo* pCreateInfo,
                                                                            VkShaderModuleIdentifierEXT* pIdentifier,
                                                                            const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkGetShaderModuleCreateInfoIdentifierEXT-device-parameter"
    if (pCreateInfo) {
        [[maybe_unused]] const Location pCreateInfo_loc = error_obj.location.dot(Field::pCreateInfo);
        if (auto pNext = vku::FindStructInPNextChain<VkShaderModuleValidationCacheCreateInfoEXT>(pCreateInfo->pNext)) {
            const Location pNext_loc = pCreateInfo_loc.pNext(Struct::VkShaderModuleValidationCacheCreateInfoEXT);
            skip |= ValidateObject(pNext->validationCache, kVulkanObjectTypeValidationCacheEXT, false,
                                   "VUID-VkShaderModuleValidationCacheCreateInfoEXT-validationCache-parameter", kVUIDUndefined,
                                   pNext_loc.dot(Field::validationCache));
        }
    }

    return skip;
}

// vkGetPhysicalDeviceOpticalFlowImageFormatsNV:
// Checked by chassis: physicalDevice: "VUID-vkGetPhysicalDeviceOpticalFlowImageFormatsNV-physicalDevice-parameter"

// vkCreateOpticalFlowSessionNV:
// Checked by chassis: device: "VUID-vkCreateOpticalFlowSessionNV-device-parameter"

void ObjectLifetimes::PostCallRecordCreateOpticalFlowSessionNV(VkDevice device, const VkOpticalFlowSessionCreateInfoNV* pCreateInfo,
                                                               const VkAllocationCallbacks* pAllocator,
                                                               VkOpticalFlowSessionNV* pSession, const RecordObject& record_obj) {
    if (record_obj.result != VK_SUCCESS) return;
    CreateObject(*pSession, kVulkanObjectTypeOpticalFlowSessionNV, pAllocator);
}

bool ObjectLifetimes::PreCallValidateDestroyOpticalFlowSessionNV(VkDevice device, VkOpticalFlowSessionNV session,
                                                                 const VkAllocationCallbacks* pAllocator,
                                                                 const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkDestroyOpticalFlowSessionNV-device-parameter"
    skip |= ValidateObject(session, kVulkanObjectTypeOpticalFlowSessionNV, false,
                           "VUID-vkDestroyOpticalFlowSessionNV-session-parameter",
                           "VUID-vkDestroyOpticalFlowSessionNV-session-parent", error_obj.location.dot(Field::session));
    skip |= ValidateDestroyObject(session, kVulkanObjectTypeOpticalFlowSessionNV, pAllocator, kVUIDUndefined, kVUIDUndefined,
                                  error_obj.location);

    return skip;
}

void ObjectLifetimes::PreCallRecordDestroyOpticalFlowSessionNV(VkDevice device, VkOpticalFlowSessionNV session,
                                                               const VkAllocationCallbacks* pAllocator) {
    RecordDestroyObject(session, kVulkanObjectTypeOpticalFlowSessionNV);
}

bool ObjectLifetimes::PreCallValidateBindOpticalFlowSessionImageNV(VkDevice device, VkOpticalFlowSessionNV session,
                                                                   VkOpticalFlowSessionBindingPointNV bindingPoint,
                                                                   VkImageView view, VkImageLayout layout,
                                                                   const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkBindOpticalFlowSessionImageNV-device-parameter"
    skip |= ValidateObject(session, kVulkanObjectTypeOpticalFlowSessionNV, false,
                           "VUID-vkBindOpticalFlowSessionImageNV-session-parameter",
                           "VUID-vkBindOpticalFlowSessionImageNV-session-parent", error_obj.location.dot(Field::session));
    skip |= ValidateObject(view, kVulkanObjectTypeImageView, true, "VUID-vkBindOpticalFlowSessionImageNV-view-parameter",
                           "VUID-vkBindOpticalFlowSessionImageNV-view-parent", error_obj.location.dot(Field::view));

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdOpticalFlowExecuteNV(VkCommandBuffer commandBuffer, VkOpticalFlowSessionNV session,
                                                             const VkOpticalFlowExecuteInfoNV* pExecuteInfo,
                                                             const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: commandBuffer: "VUID-vkCmdOpticalFlowExecuteNV-commandBuffer-parameter"
    // Checked by chassis: commandBuffer: "VUID-vkCmdOpticalFlowExecuteNV-commonparent"
    skip |=
        ValidateObject(session, kVulkanObjectTypeOpticalFlowSessionNV, false, "VUID-vkCmdOpticalFlowExecuteNV-session-parameter",
                       "VUID-vkCmdOpticalFlowExecuteNV-commonparent", error_obj.location.dot(Field::session));

    return skip;
}

bool ObjectLifetimes::PreCallValidateCreateShadersEXT(VkDevice device, uint32_t createInfoCount,
                                                      const VkShaderCreateInfoEXT* pCreateInfos,
                                                      const VkAllocationCallbacks* pAllocator, VkShaderEXT* pShaders,
                                                      const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkCreateShadersEXT-device-parameter"
    if (pCreateInfos) {
        for (uint32_t index0 = 0; index0 < createInfoCount; ++index0) {
            [[maybe_unused]] const Location index0_loc = error_obj.location.dot(Field::pCreateInfos, index0);

            if ((pCreateInfos[index0].setLayoutCount > 0) && (pCreateInfos[index0].pSetLayouts)) {
                for (uint32_t index1 = 0; index1 < pCreateInfos[index0].setLayoutCount; ++index1) {
                    skip |= ValidateObject(pCreateInfos[index0].pSetLayouts[index1], kVulkanObjectTypeDescriptorSetLayout, false,
                                           "VUID-VkShaderCreateInfoEXT-pSetLayouts-parameter", kVUIDUndefined,
                                           index0_loc.dot(Field::pSetLayouts, index1));
                }
            }
        }
    }

    return skip;
}

void ObjectLifetimes::PostCallRecordCreateShadersEXT(VkDevice device, uint32_t createInfoCount,
                                                     const VkShaderCreateInfoEXT* pCreateInfos,
                                                     const VkAllocationCallbacks* pAllocator, VkShaderEXT* pShaders,
                                                     const RecordObject& record_obj) {
    if (record_obj.result != VK_SUCCESS && record_obj.result != VK_ERROR_INCOMPATIBLE_SHADER_BINARY_EXT) return;
    if (pShaders) {
        for (uint32_t index = 0; index < createInfoCount; index++) {
            if (!pShaders[index]) break;
            CreateObject(pShaders[index], kVulkanObjectTypeShaderEXT, pAllocator);
        }
    }
}

bool ObjectLifetimes::PreCallValidateDestroyShaderEXT(VkDevice device, VkShaderEXT shader, const VkAllocationCallbacks* pAllocator,
                                                      const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkDestroyShaderEXT-device-parameter"
    skip |= ValidateObject(shader, kVulkanObjectTypeShaderEXT, false, "VUID-vkDestroyShaderEXT-shader-parameter",
                           "VUID-vkDestroyShaderEXT-shader-parent", error_obj.location.dot(Field::shader));
    skip |= ValidateDestroyObject(shader, kVulkanObjectTypeShaderEXT, pAllocator, "VUID-vkDestroyShaderEXT-pAllocator-08483",
                                  "VUID-vkDestroyShaderEXT-pAllocator-08484", error_obj.location);

    return skip;
}

void ObjectLifetimes::PreCallRecordDestroyShaderEXT(VkDevice device, VkShaderEXT shader, const VkAllocationCallbacks* pAllocator) {
    RecordDestroyObject(shader, kVulkanObjectTypeShaderEXT);
}

bool ObjectLifetimes::PreCallValidateGetShaderBinaryDataEXT(VkDevice device, VkShaderEXT shader, size_t* pDataSize, void* pData,
                                                            const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkGetShaderBinaryDataEXT-device-parameter"
    skip |= ValidateObject(shader, kVulkanObjectTypeShaderEXT, false, "VUID-vkGetShaderBinaryDataEXT-shader-parameter",
                           "VUID-vkGetShaderBinaryDataEXT-shader-parent", error_obj.location.dot(Field::shader));

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdBindShadersEXT(VkCommandBuffer commandBuffer, uint32_t stageCount,
                                                       const VkShaderStageFlagBits* pStages, const VkShaderEXT* pShaders,
                                                       const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: commandBuffer: "VUID-vkCmdBindShadersEXT-commandBuffer-parameter"
    // Checked by chassis: commandBuffer: "VUID-vkCmdBindShadersEXT-commonparent"

    if ((stageCount > 0) && (pShaders)) {
        for (uint32_t index0 = 0; index0 < stageCount; ++index0) {
            skip |=
                ValidateObject(pShaders[index0], kVulkanObjectTypeShaderEXT, true, "VUID-vkCmdBindShadersEXT-pShaders-parameter",
                               "VUID-vkCmdBindShadersEXT-commonparent", error_obj.location.dot(Field::pShaders, index0));
        }
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetFramebufferTilePropertiesQCOM(VkDevice device, VkFramebuffer framebuffer,
                                                                      uint32_t* pPropertiesCount, VkTilePropertiesQCOM* pProperties,
                                                                      const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkGetFramebufferTilePropertiesQCOM-device-parameter"
    skip |= ValidateObject(
        framebuffer, kVulkanObjectTypeFramebuffer, false, "VUID-vkGetFramebufferTilePropertiesQCOM-framebuffer-parameter",
        "VUID-vkGetFramebufferTilePropertiesQCOM-framebuffer-parent", error_obj.location.dot(Field::framebuffer));

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetDynamicRenderingTilePropertiesQCOM(VkDevice device, const VkRenderingInfo* pRenderingInfo,
                                                                           VkTilePropertiesQCOM* pProperties,
                                                                           const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkGetDynamicRenderingTilePropertiesQCOM-device-parameter"
    if (pRenderingInfo) {
        [[maybe_unused]] const Location pRenderingInfo_loc = error_obj.location.dot(Field::pRenderingInfo);
        if (pRenderingInfo->pColorAttachments) {
            for (uint32_t index1 = 0; index1 < pRenderingInfo->colorAttachmentCount; ++index1) {
                [[maybe_unused]] const Location index1_loc = pRenderingInfo_loc.dot(Field::pColorAttachments, index1);
                skip |= ValidateObject(pRenderingInfo->pColorAttachments[index1].imageView, kVulkanObjectTypeImageView, true,
                                       "VUID-VkRenderingAttachmentInfo-imageView-parameter",
                                       "VUID-VkRenderingAttachmentInfo-commonparent", index1_loc.dot(Field::imageView));
                skip |= ValidateObject(pRenderingInfo->pColorAttachments[index1].resolveImageView, kVulkanObjectTypeImageView, true,
                                       "VUID-VkRenderingAttachmentInfo-resolveImageView-parameter",
                                       "VUID-VkRenderingAttachmentInfo-commonparent", index1_loc.dot(Field::resolveImageView));
            }
        }
        if (pRenderingInfo->pDepthAttachment) {
            [[maybe_unused]] const Location pDepthAttachment_loc = pRenderingInfo_loc.dot(Field::pDepthAttachment);
            skip |= ValidateObject(pRenderingInfo->pDepthAttachment->imageView, kVulkanObjectTypeImageView, true,
                                   "VUID-VkRenderingAttachmentInfo-imageView-parameter",
                                   "VUID-VkRenderingAttachmentInfo-commonparent", pDepthAttachment_loc.dot(Field::imageView));
            skip |=
                ValidateObject(pRenderingInfo->pDepthAttachment->resolveImageView, kVulkanObjectTypeImageView, true,
                               "VUID-VkRenderingAttachmentInfo-resolveImageView-parameter",
                               "VUID-VkRenderingAttachmentInfo-commonparent", pDepthAttachment_loc.dot(Field::resolveImageView));
        }
        if (pRenderingInfo->pStencilAttachment) {
            [[maybe_unused]] const Location pStencilAttachment_loc = pRenderingInfo_loc.dot(Field::pStencilAttachment);
            skip |= ValidateObject(pRenderingInfo->pStencilAttachment->imageView, kVulkanObjectTypeImageView, true,
                                   "VUID-VkRenderingAttachmentInfo-imageView-parameter",
                                   "VUID-VkRenderingAttachmentInfo-commonparent", pStencilAttachment_loc.dot(Field::imageView));
            skip |=
                ValidateObject(pRenderingInfo->pStencilAttachment->resolveImageView, kVulkanObjectTypeImageView, true,
                               "VUID-VkRenderingAttachmentInfo-resolveImageView-parameter",
                               "VUID-VkRenderingAttachmentInfo-commonparent", pStencilAttachment_loc.dot(Field::resolveImageView));
        }
        if (auto pNext = vku::FindStructInPNextChain<VkRenderingFragmentDensityMapAttachmentInfoEXT>(pRenderingInfo->pNext)) {
            const Location pNext_loc = pRenderingInfo_loc.pNext(Struct::VkRenderingFragmentDensityMapAttachmentInfoEXT);
            skip |= ValidateObject(pNext->imageView, kVulkanObjectTypeImageView, false,
                                   "VUID-VkRenderingFragmentDensityMapAttachmentInfoEXT-imageView-parameter", kVUIDUndefined,
                                   pNext_loc.dot(Field::imageView));
        }
        if (auto pNext = vku::FindStructInPNextChain<VkRenderingFragmentShadingRateAttachmentInfoKHR>(pRenderingInfo->pNext)) {
            const Location pNext_loc = pRenderingInfo_loc.pNext(Struct::VkRenderingFragmentShadingRateAttachmentInfoKHR);
            skip |= ValidateObject(pNext->imageView, kVulkanObjectTypeImageView, true,
                                   "VUID-VkRenderingFragmentShadingRateAttachmentInfoKHR-imageView-parameter", kVUIDUndefined,
                                   pNext_loc.dot(Field::imageView));
        }
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateSetLatencySleepModeNV(VkDevice device, VkSwapchainKHR swapchain,
                                                           const VkLatencySleepModeInfoNV* pSleepModeInfo,
                                                           const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkSetLatencySleepModeNV-device-parameter"
    skip |= ValidateObject(swapchain, kVulkanObjectTypeSwapchainKHR, false, "VUID-vkSetLatencySleepModeNV-swapchain-parameter",
                           "VUID-vkSetLatencySleepModeNV-swapchain-parent", error_obj.location.dot(Field::swapchain));

    return skip;
}

bool ObjectLifetimes::PreCallValidateLatencySleepNV(VkDevice device, VkSwapchainKHR swapchain,
                                                    const VkLatencySleepInfoNV* pSleepInfo, const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkLatencySleepNV-device-parameter"
    skip |= ValidateObject(swapchain, kVulkanObjectTypeSwapchainKHR, false, "VUID-vkLatencySleepNV-swapchain-parameter",
                           "VUID-vkLatencySleepNV-swapchain-parent", error_obj.location.dot(Field::swapchain));
    if (pSleepInfo) {
        [[maybe_unused]] const Location pSleepInfo_loc = error_obj.location.dot(Field::pSleepInfo);
        skip |= ValidateObject(
            pSleepInfo->signalSemaphore, kVulkanObjectTypeSemaphore, false, "VUID-VkLatencySleepInfoNV-signalSemaphore-parameter",
            "UNASSIGNED-VkLatencySleepInfoNV-signalSemaphore-parent", pSleepInfo_loc.dot(Field::signalSemaphore));
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateSetLatencyMarkerNV(VkDevice device, VkSwapchainKHR swapchain,
                                                        const VkSetLatencyMarkerInfoNV* pLatencyMarkerInfo,
                                                        const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkSetLatencyMarkerNV-device-parameter"
    skip |= ValidateObject(swapchain, kVulkanObjectTypeSwapchainKHR, false, "VUID-vkSetLatencyMarkerNV-swapchain-parameter",
                           "VUID-vkSetLatencyMarkerNV-swapchain-parent", error_obj.location.dot(Field::swapchain));

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetLatencyTimingsNV(VkDevice device, VkSwapchainKHR swapchain, uint32_t* pTimingCount,
                                                         VkGetLatencyMarkerInfoNV* pLatencyMarkerInfo,
                                                         const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkGetLatencyTimingsNV-device-parameter"
    skip |= ValidateObject(swapchain, kVulkanObjectTypeSwapchainKHR, false, "VUID-vkGetLatencyTimingsNV-swapchain-parameter",
                           "VUID-vkGetLatencyTimingsNV-swapchain-parent", error_obj.location.dot(Field::swapchain));

    return skip;
}

// vkQueueNotifyOutOfBandNV:
// Checked by chassis: queue: "VUID-vkQueueNotifyOutOfBandNV-queue-parameter"

// vkCmdSetAttachmentFeedbackLoopEnableEXT:
// Checked by chassis: commandBuffer: "VUID-vkCmdSetAttachmentFeedbackLoopEnableEXT-commandBuffer-parameter"

#ifdef VK_USE_PLATFORM_SCREEN_QNX

// vkGetScreenBufferPropertiesQNX:
// Checked by chassis: device: "VUID-vkGetScreenBufferPropertiesQNX-device-parameter"

#endif  // VK_USE_PLATFORM_SCREEN_QNX

bool ObjectLifetimes::PreCallValidateCreateAccelerationStructureKHR(VkDevice device,
                                                                    const VkAccelerationStructureCreateInfoKHR* pCreateInfo,
                                                                    const VkAllocationCallbacks* pAllocator,
                                                                    VkAccelerationStructureKHR* pAccelerationStructure,
                                                                    const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkCreateAccelerationStructureKHR-device-parameter"
    if (pCreateInfo) {
        [[maybe_unused]] const Location pCreateInfo_loc = error_obj.location.dot(Field::pCreateInfo);
        skip |= ValidateObject(pCreateInfo->buffer, kVulkanObjectTypeBuffer, false,
                               "VUID-VkAccelerationStructureCreateInfoKHR-buffer-parameter", kVUIDUndefined,
                               pCreateInfo_loc.dot(Field::buffer));
    }

    return skip;
}

void ObjectLifetimes::PostCallRecordCreateAccelerationStructureKHR(VkDevice device,
                                                                   const VkAccelerationStructureCreateInfoKHR* pCreateInfo,
                                                                   const VkAllocationCallbacks* pAllocator,
                                                                   VkAccelerationStructureKHR* pAccelerationStructure,
                                                                   const RecordObject& record_obj) {
    if (record_obj.result != VK_SUCCESS) return;
    CreateObject(*pAccelerationStructure, kVulkanObjectTypeAccelerationStructureKHR, pAllocator);
}

bool ObjectLifetimes::PreCallValidateDestroyAccelerationStructureKHR(VkDevice device,
                                                                     VkAccelerationStructureKHR accelerationStructure,
                                                                     const VkAllocationCallbacks* pAllocator,
                                                                     const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkDestroyAccelerationStructureKHR-device-parameter"
    skip |= ValidateObject(accelerationStructure, kVulkanObjectTypeAccelerationStructureKHR, true,
                           "VUID-vkDestroyAccelerationStructureKHR-accelerationStructure-parameter",
                           "VUID-vkDestroyAccelerationStructureKHR-accelerationStructure-parent",
                           error_obj.location.dot(Field::accelerationStructure));
    skip |= ValidateDestroyObject(accelerationStructure, kVulkanObjectTypeAccelerationStructureKHR, pAllocator,
                                  "VUID-vkDestroyAccelerationStructureKHR-accelerationStructure-02443",
                                  "VUID-vkDestroyAccelerationStructureKHR-accelerationStructure-02444", error_obj.location);

    return skip;
}

void ObjectLifetimes::PreCallRecordDestroyAccelerationStructureKHR(VkDevice device,
                                                                   VkAccelerationStructureKHR accelerationStructure,
                                                                   const VkAllocationCallbacks* pAllocator) {
    RecordDestroyObject(accelerationStructure, kVulkanObjectTypeAccelerationStructureKHR);
}

bool ObjectLifetimes::PreCallValidateCopyAccelerationStructureKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                                  const VkCopyAccelerationStructureInfoKHR* pInfo,
                                                                  const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkCopyAccelerationStructureKHR-device-parameter"
    skip |= ValidateObject(deferredOperation, kVulkanObjectTypeDeferredOperationKHR, true,
                           "VUID-vkCopyAccelerationStructureKHR-deferredOperation-parameter",
                           "VUID-vkCopyAccelerationStructureKHR-deferredOperation-parent",
                           error_obj.location.dot(Field::deferredOperation));
    if (pInfo) {
        [[maybe_unused]] const Location pInfo_loc = error_obj.location.dot(Field::pInfo);
        skip |= ValidateObject(pInfo->src, kVulkanObjectTypeAccelerationStructureKHR, false,
                               "VUID-VkCopyAccelerationStructureInfoKHR-src-parameter",
                               "VUID-VkCopyAccelerationStructureInfoKHR-commonparent", pInfo_loc.dot(Field::src));
        skip |= ValidateObject(pInfo->dst, kVulkanObjectTypeAccelerationStructureKHR, false,
                               "VUID-VkCopyAccelerationStructureInfoKHR-dst-parameter",
                               "VUID-VkCopyAccelerationStructureInfoKHR-commonparent", pInfo_loc.dot(Field::dst));
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateCopyAccelerationStructureToMemoryKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                                          const VkCopyAccelerationStructureToMemoryInfoKHR* pInfo,
                                                                          const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkCopyAccelerationStructureToMemoryKHR-device-parameter"
    skip |= ValidateObject(deferredOperation, kVulkanObjectTypeDeferredOperationKHR, true,
                           "VUID-vkCopyAccelerationStructureToMemoryKHR-deferredOperation-parameter",
                           "VUID-vkCopyAccelerationStructureToMemoryKHR-deferredOperation-parent",
                           error_obj.location.dot(Field::deferredOperation));
    if (pInfo) {
        [[maybe_unused]] const Location pInfo_loc = error_obj.location.dot(Field::pInfo);
        skip |= ValidateObject(pInfo->src, kVulkanObjectTypeAccelerationStructureKHR, false,
                               "VUID-VkCopyAccelerationStructureToMemoryInfoKHR-src-parameter", kVUIDUndefined,
                               pInfo_loc.dot(Field::src));
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateCopyMemoryToAccelerationStructureKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                                          const VkCopyMemoryToAccelerationStructureInfoKHR* pInfo,
                                                                          const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkCopyMemoryToAccelerationStructureKHR-device-parameter"
    skip |= ValidateObject(deferredOperation, kVulkanObjectTypeDeferredOperationKHR, true,
                           "VUID-vkCopyMemoryToAccelerationStructureKHR-deferredOperation-parameter",
                           "VUID-vkCopyMemoryToAccelerationStructureKHR-deferredOperation-parent",
                           error_obj.location.dot(Field::deferredOperation));
    if (pInfo) {
        [[maybe_unused]] const Location pInfo_loc = error_obj.location.dot(Field::pInfo);
        skip |= ValidateObject(pInfo->dst, kVulkanObjectTypeAccelerationStructureKHR, false,
                               "VUID-VkCopyMemoryToAccelerationStructureInfoKHR-dst-parameter", kVUIDUndefined,
                               pInfo_loc.dot(Field::dst));
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateWriteAccelerationStructuresPropertiesKHR(
    VkDevice device, uint32_t accelerationStructureCount, const VkAccelerationStructureKHR* pAccelerationStructures,
    VkQueryType queryType, size_t dataSize, void* pData, size_t stride, const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkWriteAccelerationStructuresPropertiesKHR-device-parameter"

    if ((accelerationStructureCount > 0) && (pAccelerationStructures)) {
        for (uint32_t index0 = 0; index0 < accelerationStructureCount; ++index0) {
            skip |= ValidateObject(pAccelerationStructures[index0], kVulkanObjectTypeAccelerationStructureKHR, false,
                                   "VUID-vkWriteAccelerationStructuresPropertiesKHR-pAccelerationStructures-parameter",
                                   "VUID-vkWriteAccelerationStructuresPropertiesKHR-pAccelerationStructures-parent",
                                   error_obj.location.dot(Field::pAccelerationStructures, index0));
        }
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdCopyAccelerationStructureKHR(VkCommandBuffer commandBuffer,
                                                                     const VkCopyAccelerationStructureInfoKHR* pInfo,
                                                                     const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: commandBuffer: "VUID-vkCmdCopyAccelerationStructureKHR-commandBuffer-parameter"
    if (pInfo) {
        [[maybe_unused]] const Location pInfo_loc = error_obj.location.dot(Field::pInfo);
        skip |= ValidateObject(pInfo->src, kVulkanObjectTypeAccelerationStructureKHR, false,
                               "VUID-VkCopyAccelerationStructureInfoKHR-src-parameter",
                               "VUID-VkCopyAccelerationStructureInfoKHR-commonparent", pInfo_loc.dot(Field::src));
        skip |= ValidateObject(pInfo->dst, kVulkanObjectTypeAccelerationStructureKHR, false,
                               "VUID-VkCopyAccelerationStructureInfoKHR-dst-parameter",
                               "VUID-VkCopyAccelerationStructureInfoKHR-commonparent", pInfo_loc.dot(Field::dst));
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdCopyAccelerationStructureToMemoryKHR(
    VkCommandBuffer commandBuffer, const VkCopyAccelerationStructureToMemoryInfoKHR* pInfo, const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: commandBuffer: "VUID-vkCmdCopyAccelerationStructureToMemoryKHR-commandBuffer-parameter"
    if (pInfo) {
        [[maybe_unused]] const Location pInfo_loc = error_obj.location.dot(Field::pInfo);
        skip |= ValidateObject(pInfo->src, kVulkanObjectTypeAccelerationStructureKHR, false,
                               "VUID-VkCopyAccelerationStructureToMemoryInfoKHR-src-parameter", kVUIDUndefined,
                               pInfo_loc.dot(Field::src));
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdCopyMemoryToAccelerationStructureKHR(
    VkCommandBuffer commandBuffer, const VkCopyMemoryToAccelerationStructureInfoKHR* pInfo, const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: commandBuffer: "VUID-vkCmdCopyMemoryToAccelerationStructureKHR-commandBuffer-parameter"
    if (pInfo) {
        [[maybe_unused]] const Location pInfo_loc = error_obj.location.dot(Field::pInfo);
        skip |= ValidateObject(pInfo->dst, kVulkanObjectTypeAccelerationStructureKHR, false,
                               "VUID-VkCopyMemoryToAccelerationStructureInfoKHR-dst-parameter", kVUIDUndefined,
                               pInfo_loc.dot(Field::dst));
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateGetAccelerationStructureDeviceAddressKHR(
    VkDevice device, const VkAccelerationStructureDeviceAddressInfoKHR* pInfo, const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkGetAccelerationStructureDeviceAddressKHR-device-parameter"
    if (pInfo) {
        [[maybe_unused]] const Location pInfo_loc = error_obj.location.dot(Field::pInfo);
        skip |= ValidateObject(pInfo->accelerationStructure, kVulkanObjectTypeAccelerationStructureKHR, false,
                               "VUID-VkAccelerationStructureDeviceAddressInfoKHR-accelerationStructure-parameter", kVUIDUndefined,
                               pInfo_loc.dot(Field::accelerationStructure));
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdWriteAccelerationStructuresPropertiesKHR(
    VkCommandBuffer commandBuffer, uint32_t accelerationStructureCount, const VkAccelerationStructureKHR* pAccelerationStructures,
    VkQueryType queryType, VkQueryPool queryPool, uint32_t firstQuery, const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: commandBuffer: "VUID-vkCmdWriteAccelerationStructuresPropertiesKHR-commandBuffer-parameter"
    // Checked by chassis: commandBuffer: "VUID-vkCmdWriteAccelerationStructuresPropertiesKHR-commonparent"

    if ((accelerationStructureCount > 0) && (pAccelerationStructures)) {
        for (uint32_t index0 = 0; index0 < accelerationStructureCount; ++index0) {
            skip |= ValidateObject(pAccelerationStructures[index0], kVulkanObjectTypeAccelerationStructureKHR, false,
                                   "VUID-vkCmdWriteAccelerationStructuresPropertiesKHR-pAccelerationStructures-parameter",
                                   "VUID-vkCmdWriteAccelerationStructuresPropertiesKHR-commonparent",
                                   error_obj.location.dot(Field::pAccelerationStructures, index0));
        }
    }
    skip |= ValidateObject(
        queryPool, kVulkanObjectTypeQueryPool, false, "VUID-vkCmdWriteAccelerationStructuresPropertiesKHR-queryPool-parameter",
        "VUID-vkCmdWriteAccelerationStructuresPropertiesKHR-commonparent", error_obj.location.dot(Field::queryPool));

    return skip;
}

// vkGetDeviceAccelerationStructureCompatibilityKHR:
// Checked by chassis: device: "VUID-vkGetDeviceAccelerationStructureCompatibilityKHR-device-parameter"

bool ObjectLifetimes::PreCallValidateGetAccelerationStructureBuildSizesKHR(
    VkDevice device, VkAccelerationStructureBuildTypeKHR buildType, const VkAccelerationStructureBuildGeometryInfoKHR* pBuildInfo,
    const uint32_t* pMaxPrimitiveCounts, VkAccelerationStructureBuildSizesInfoKHR* pSizeInfo, const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkGetAccelerationStructureBuildSizesKHR-device-parameter"
    if (pBuildInfo) {
        [[maybe_unused]] const Location pBuildInfo_loc = error_obj.location.dot(Field::pBuildInfo);
        skip |= ValidateObject(pBuildInfo->srcAccelerationStructure, kVulkanObjectTypeAccelerationStructureKHR, true,
                               kVUIDUndefined, "VUID-VkAccelerationStructureBuildGeometryInfoKHR-commonparent",
                               pBuildInfo_loc.dot(Field::srcAccelerationStructure));
        skip |= ValidateObject(pBuildInfo->dstAccelerationStructure, kVulkanObjectTypeAccelerationStructureKHR, true,
                               kVUIDUndefined, "VUID-VkAccelerationStructureBuildGeometryInfoKHR-commonparent",
                               pBuildInfo_loc.dot(Field::dstAccelerationStructure));
    }

    return skip;
}

// vkCmdTraceRaysKHR:
// Checked by chassis: commandBuffer: "VUID-vkCmdTraceRaysKHR-commandBuffer-parameter"

bool ObjectLifetimes::PreCallValidateGetRayTracingCaptureReplayShaderGroupHandlesKHR(VkDevice device, VkPipeline pipeline,
                                                                                     uint32_t firstGroup, uint32_t groupCount,
                                                                                     size_t dataSize, void* pData,
                                                                                     const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkGetRayTracingCaptureReplayShaderGroupHandlesKHR-device-parameter"
    skip |= ValidateObject(
        pipeline, kVulkanObjectTypePipeline, false, "VUID-vkGetRayTracingCaptureReplayShaderGroupHandlesKHR-pipeline-parameter",
        "VUID-vkGetRayTracingCaptureReplayShaderGroupHandlesKHR-pipeline-parent", error_obj.location.dot(Field::pipeline));

    return skip;
}

// vkCmdTraceRaysIndirectKHR:
// Checked by chassis: commandBuffer: "VUID-vkCmdTraceRaysIndirectKHR-commandBuffer-parameter"

bool ObjectLifetimes::PreCallValidateGetRayTracingShaderGroupStackSizeKHR(VkDevice device, VkPipeline pipeline, uint32_t group,
                                                                          VkShaderGroupShaderKHR groupShader,
                                                                          const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkGetRayTracingShaderGroupStackSizeKHR-device-parameter"
    skip |=
        ValidateObject(pipeline, kVulkanObjectTypePipeline, false, "VUID-vkGetRayTracingShaderGroupStackSizeKHR-pipeline-parameter",
                       "VUID-vkGetRayTracingShaderGroupStackSizeKHR-pipeline-parent", error_obj.location.dot(Field::pipeline));

    return skip;
}

// vkCmdSetRayTracingPipelineStackSizeKHR:
// Checked by chassis: commandBuffer: "VUID-vkCmdSetRayTracingPipelineStackSizeKHR-commandBuffer-parameter"

// vkCmdDrawMeshTasksEXT:
// Checked by chassis: commandBuffer: "VUID-vkCmdDrawMeshTasksEXT-commandBuffer-parameter"

bool ObjectLifetimes::PreCallValidateCmdDrawMeshTasksIndirectEXT(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                 VkDeviceSize offset, uint32_t drawCount, uint32_t stride,
                                                                 const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: commandBuffer: "VUID-vkCmdDrawMeshTasksIndirectEXT-commandBuffer-parameter"
    // Checked by chassis: commandBuffer: "VUID-vkCmdDrawMeshTasksIndirectEXT-commonparent"
    skip |= ValidateObject(buffer, kVulkanObjectTypeBuffer, false, "VUID-vkCmdDrawMeshTasksIndirectEXT-buffer-parameter",
                           "VUID-vkCmdDrawMeshTasksIndirectEXT-commonparent", error_obj.location.dot(Field::buffer));

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdDrawMeshTasksIndirectCountEXT(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                      VkDeviceSize offset, VkBuffer countBuffer,
                                                                      VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                                      uint32_t stride, const ErrorObject& error_obj) const {
    bool skip = false;
    // Checked by chassis: commandBuffer: "VUID-vkCmdDrawMeshTasksIndirectCountEXT-commandBuffer-parameter"
    // Checked by chassis: commandBuffer: "VUID-vkCmdDrawMeshTasksIndirectCountEXT-commonparent"
    skip |= ValidateObject(buffer, kVulkanObjectTypeBuffer, false, "VUID-vkCmdDrawMeshTasksIndirectCountEXT-buffer-parameter",
                           "VUID-vkCmdDrawMeshTasksIndirectCountEXT-commonparent", error_obj.location.dot(Field::buffer));
    skip |=
        ValidateObject(countBuffer, kVulkanObjectTypeBuffer, false, "VUID-vkCmdDrawMeshTasksIndirectCountEXT-countBuffer-parameter",
                       "VUID-vkCmdDrawMeshTasksIndirectCountEXT-commonparent", error_obj.location.dot(Field::countBuffer));

    return skip;
}

// NOLINTEND
