// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See thread_safety_generator.py for modifications

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

#include "chassis.h"
#include "thread_tracker/thread_safety_validation.h"
void ThreadSafety::PreCallRecordCreateInstance(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                               VkInstance* pInstance) {}

void ThreadSafety::PostCallRecordCreateInstance(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                                VkInstance* pInstance, const RecordObject& record_obj) {
    if (record_obj.result == VK_SUCCESS) {
        CreateObjectParentInstance(*pInstance);
    }
}

void ThreadSafety::PreCallRecordDestroyInstance(VkInstance instance, const VkAllocationCallbacks* pAllocator) {
    StartWriteObjectParentInstance(instance, vvl::Func::vkDestroyInstance);
    // Host access to instance must be externally synchronized
    // all sname:VkPhysicalDevice objects enumerated from pname:instance must be externally synchronized between host accesses
}

void ThreadSafety::PostCallRecordDestroyInstance(VkInstance instance, const VkAllocationCallbacks* pAllocator,
                                                 const RecordObject& record_obj) {
    FinishWriteObjectParentInstance(instance, vvl::Func::vkDestroyInstance);
    DestroyObjectParentInstance(instance);
    // Host access to instance must be externally synchronized
    // all sname:VkPhysicalDevice objects enumerated from pname:instance must be externally synchronized between host accesses
}

void ThreadSafety::PreCallRecordEnumeratePhysicalDevices(VkInstance instance, uint32_t* pPhysicalDeviceCount,
                                                         VkPhysicalDevice* pPhysicalDevices) {
    StartReadObjectParentInstance(instance, vvl::Func::vkEnumeratePhysicalDevices);
}

void ThreadSafety::PostCallRecordEnumeratePhysicalDevices(VkInstance instance, uint32_t* pPhysicalDeviceCount,
                                                          VkPhysicalDevice* pPhysicalDevices, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(instance, vvl::Func::vkEnumeratePhysicalDevices);
}

void ThreadSafety::PreCallRecordGetInstanceProcAddr(VkInstance instance, const char* pName) {
    StartReadObjectParentInstance(instance, vvl::Func::vkGetInstanceProcAddr);
}

void ThreadSafety::PostCallRecordGetInstanceProcAddr(VkInstance instance, const char* pName, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(instance, vvl::Func::vkGetInstanceProcAddr);
}

void ThreadSafety::PreCallRecordGetDeviceProcAddr(VkDevice device, const char* pName) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetDeviceProcAddr);
}

void ThreadSafety::PostCallRecordGetDeviceProcAddr(VkDevice device, const char* pName, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetDeviceProcAddr);
}

void ThreadSafety::PreCallRecordCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo,
                                             const VkAllocationCallbacks* pAllocator, VkDevice* pDevice) {}

void ThreadSafety::PostCallRecordCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo,
                                              const VkAllocationCallbacks* pAllocator, VkDevice* pDevice,
                                              const RecordObject& record_obj) {
    if (record_obj.result == VK_SUCCESS) {
        CreateObjectParentInstance(*pDevice);
    }
}

void ThreadSafety::PreCallRecordQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence) {
    StartWriteObject(queue, vvl::Func::vkQueueSubmit);
    StartWriteObject(fence, vvl::Func::vkQueueSubmit);
    // Host access to queue must be externally synchronized
    // Host access to fence must be externally synchronized
}

void ThreadSafety::PostCallRecordQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence,
                                             const RecordObject& record_obj) {
    FinishWriteObject(queue, vvl::Func::vkQueueSubmit);
    FinishWriteObject(fence, vvl::Func::vkQueueSubmit);
    // Host access to queue must be externally synchronized
    // Host access to fence must be externally synchronized
}

void ThreadSafety::PreCallRecordQueueWaitIdle(VkQueue queue) {
    StartWriteObject(queue, vvl::Func::vkQueueWaitIdle);
    // Host access to queue must be externally synchronized
}

void ThreadSafety::PostCallRecordQueueWaitIdle(VkQueue queue, const RecordObject& record_obj) {
    FinishWriteObject(queue, vvl::Func::vkQueueWaitIdle);
    // Host access to queue must be externally synchronized
}

void ThreadSafety::PreCallRecordAllocateMemory(VkDevice device, const VkMemoryAllocateInfo* pAllocateInfo,
                                               const VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory) {
    StartReadObjectParentInstance(device, vvl::Func::vkAllocateMemory);
}

void ThreadSafety::PostCallRecordAllocateMemory(VkDevice device, const VkMemoryAllocateInfo* pAllocateInfo,
                                                const VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory,
                                                const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkAllocateMemory);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pMemory);
    }
}

void ThreadSafety::PreCallRecordFreeMemory(VkDevice device, VkDeviceMemory memory, const VkAllocationCallbacks* pAllocator) {
    StartReadObjectParentInstance(device, vvl::Func::vkFreeMemory);
    StartWriteObject(memory, vvl::Func::vkFreeMemory);
    // Host access to memory must be externally synchronized
}

void ThreadSafety::PostCallRecordFreeMemory(VkDevice device, VkDeviceMemory memory, const VkAllocationCallbacks* pAllocator,
                                            const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkFreeMemory);
    FinishWriteObject(memory, vvl::Func::vkFreeMemory);
    DestroyObject(memory);
    // Host access to memory must be externally synchronized
}

void ThreadSafety::PreCallRecordMapMemory(VkDevice device, VkDeviceMemory memory, VkDeviceSize offset, VkDeviceSize size,
                                          VkMemoryMapFlags flags, void** ppData) {
    StartReadObjectParentInstance(device, vvl::Func::vkMapMemory);
    StartWriteObject(memory, vvl::Func::vkMapMemory);
    // Host access to memory must be externally synchronized
}

void ThreadSafety::PostCallRecordMapMemory(VkDevice device, VkDeviceMemory memory, VkDeviceSize offset, VkDeviceSize size,
                                           VkMemoryMapFlags flags, void** ppData, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkMapMemory);
    FinishWriteObject(memory, vvl::Func::vkMapMemory);
    // Host access to memory must be externally synchronized
}

void ThreadSafety::PreCallRecordUnmapMemory(VkDevice device, VkDeviceMemory memory) {
    StartReadObjectParentInstance(device, vvl::Func::vkUnmapMemory);
    StartWriteObject(memory, vvl::Func::vkUnmapMemory);
    // Host access to memory must be externally synchronized
}

void ThreadSafety::PostCallRecordUnmapMemory(VkDevice device, VkDeviceMemory memory, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkUnmapMemory);
    FinishWriteObject(memory, vvl::Func::vkUnmapMemory);
    // Host access to memory must be externally synchronized
}

void ThreadSafety::PreCallRecordFlushMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount,
                                                        const VkMappedMemoryRange* pMemoryRanges) {
    StartReadObjectParentInstance(device, vvl::Func::vkFlushMappedMemoryRanges);
}

void ThreadSafety::PostCallRecordFlushMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount,
                                                         const VkMappedMemoryRange* pMemoryRanges, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkFlushMappedMemoryRanges);
}

void ThreadSafety::PreCallRecordInvalidateMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount,
                                                             const VkMappedMemoryRange* pMemoryRanges) {
    StartReadObjectParentInstance(device, vvl::Func::vkInvalidateMappedMemoryRanges);
}

void ThreadSafety::PostCallRecordInvalidateMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount,
                                                              const VkMappedMemoryRange* pMemoryRanges,
                                                              const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkInvalidateMappedMemoryRanges);
}

void ThreadSafety::PreCallRecordGetDeviceMemoryCommitment(VkDevice device, VkDeviceMemory memory,
                                                          VkDeviceSize* pCommittedMemoryInBytes) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetDeviceMemoryCommitment);
    StartReadObject(memory, vvl::Func::vkGetDeviceMemoryCommitment);
}

void ThreadSafety::PostCallRecordGetDeviceMemoryCommitment(VkDevice device, VkDeviceMemory memory,
                                                           VkDeviceSize* pCommittedMemoryInBytes, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetDeviceMemoryCommitment);
    FinishReadObject(memory, vvl::Func::vkGetDeviceMemoryCommitment);
}

void ThreadSafety::PreCallRecordBindBufferMemory(VkDevice device, VkBuffer buffer, VkDeviceMemory memory,
                                                 VkDeviceSize memoryOffset) {
    StartReadObjectParentInstance(device, vvl::Func::vkBindBufferMemory);
    StartWriteObject(buffer, vvl::Func::vkBindBufferMemory);
    StartReadObject(memory, vvl::Func::vkBindBufferMemory);
    // Host access to buffer must be externally synchronized
}

void ThreadSafety::PostCallRecordBindBufferMemory(VkDevice device, VkBuffer buffer, VkDeviceMemory memory,
                                                  VkDeviceSize memoryOffset, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkBindBufferMemory);
    FinishWriteObject(buffer, vvl::Func::vkBindBufferMemory);
    FinishReadObject(memory, vvl::Func::vkBindBufferMemory);
    // Host access to buffer must be externally synchronized
}

void ThreadSafety::PreCallRecordBindImageMemory(VkDevice device, VkImage image, VkDeviceMemory memory, VkDeviceSize memoryOffset) {
    StartReadObjectParentInstance(device, vvl::Func::vkBindImageMemory);
    StartWriteObject(image, vvl::Func::vkBindImageMemory);
    StartReadObject(memory, vvl::Func::vkBindImageMemory);
    // Host access to image must be externally synchronized
}

void ThreadSafety::PostCallRecordBindImageMemory(VkDevice device, VkImage image, VkDeviceMemory memory, VkDeviceSize memoryOffset,
                                                 const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkBindImageMemory);
    FinishWriteObject(image, vvl::Func::vkBindImageMemory);
    FinishReadObject(memory, vvl::Func::vkBindImageMemory);
    // Host access to image must be externally synchronized
}

void ThreadSafety::PreCallRecordGetBufferMemoryRequirements(VkDevice device, VkBuffer buffer,
                                                            VkMemoryRequirements* pMemoryRequirements) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetBufferMemoryRequirements);
    StartReadObject(buffer, vvl::Func::vkGetBufferMemoryRequirements);
}

void ThreadSafety::PostCallRecordGetBufferMemoryRequirements(VkDevice device, VkBuffer buffer,
                                                             VkMemoryRequirements* pMemoryRequirements,
                                                             const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetBufferMemoryRequirements);
    FinishReadObject(buffer, vvl::Func::vkGetBufferMemoryRequirements);
}

void ThreadSafety::PreCallRecordGetImageMemoryRequirements(VkDevice device, VkImage image,
                                                           VkMemoryRequirements* pMemoryRequirements) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetImageMemoryRequirements);
    StartReadObject(image, vvl::Func::vkGetImageMemoryRequirements);
}

void ThreadSafety::PostCallRecordGetImageMemoryRequirements(VkDevice device, VkImage image,
                                                            VkMemoryRequirements* pMemoryRequirements,
                                                            const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetImageMemoryRequirements);
    FinishReadObject(image, vvl::Func::vkGetImageMemoryRequirements);
}

void ThreadSafety::PreCallRecordGetImageSparseMemoryRequirements(VkDevice device, VkImage image,
                                                                 uint32_t* pSparseMemoryRequirementCount,
                                                                 VkSparseImageMemoryRequirements* pSparseMemoryRequirements) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetImageSparseMemoryRequirements);
    StartReadObject(image, vvl::Func::vkGetImageSparseMemoryRequirements);
}

void ThreadSafety::PostCallRecordGetImageSparseMemoryRequirements(VkDevice device, VkImage image,
                                                                  uint32_t* pSparseMemoryRequirementCount,
                                                                  VkSparseImageMemoryRequirements* pSparseMemoryRequirements,
                                                                  const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetImageSparseMemoryRequirements);
    FinishReadObject(image, vvl::Func::vkGetImageSparseMemoryRequirements);
}

void ThreadSafety::PreCallRecordQueueBindSparse(VkQueue queue, uint32_t bindInfoCount, const VkBindSparseInfo* pBindInfo,
                                                VkFence fence) {
    StartWriteObject(queue, vvl::Func::vkQueueBindSparse);
    StartWriteObject(fence, vvl::Func::vkQueueBindSparse);
    // Host access to queue must be externally synchronized
    // Host access to fence must be externally synchronized
}

void ThreadSafety::PostCallRecordQueueBindSparse(VkQueue queue, uint32_t bindInfoCount, const VkBindSparseInfo* pBindInfo,
                                                 VkFence fence, const RecordObject& record_obj) {
    FinishWriteObject(queue, vvl::Func::vkQueueBindSparse);
    FinishWriteObject(fence, vvl::Func::vkQueueBindSparse);
    // Host access to queue must be externally synchronized
    // Host access to fence must be externally synchronized
}

void ThreadSafety::PreCallRecordCreateFence(VkDevice device, const VkFenceCreateInfo* pCreateInfo,
                                            const VkAllocationCallbacks* pAllocator, VkFence* pFence) {
    StartReadObjectParentInstance(device, vvl::Func::vkCreateFence);
}

void ThreadSafety::PostCallRecordCreateFence(VkDevice device, const VkFenceCreateInfo* pCreateInfo,
                                             const VkAllocationCallbacks* pAllocator, VkFence* pFence,
                                             const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkCreateFence);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pFence);
    }
}

void ThreadSafety::PreCallRecordDestroyFence(VkDevice device, VkFence fence, const VkAllocationCallbacks* pAllocator) {
    StartReadObjectParentInstance(device, vvl::Func::vkDestroyFence);
    StartWriteObject(fence, vvl::Func::vkDestroyFence);
    // Host access to fence must be externally synchronized
}

void ThreadSafety::PostCallRecordDestroyFence(VkDevice device, VkFence fence, const VkAllocationCallbacks* pAllocator,
                                              const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkDestroyFence);
    FinishWriteObject(fence, vvl::Func::vkDestroyFence);
    DestroyObject(fence);
    // Host access to fence must be externally synchronized
}

void ThreadSafety::PreCallRecordResetFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences) {
    StartReadObjectParentInstance(device, vvl::Func::vkResetFences);

    if (pFences) {
        for (uint32_t index = 0; index < fenceCount; index++) {
            StartWriteObject(pFences[index], vvl::Func::vkResetFences);
        }
    }
    // Host access to each member of pFences must be externally synchronized
}

void ThreadSafety::PostCallRecordResetFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences,
                                             const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkResetFences);

    if (pFences) {
        for (uint32_t index = 0; index < fenceCount; index++) {
            FinishWriteObject(pFences[index], vvl::Func::vkResetFences);
        }
    }
    // Host access to each member of pFences must be externally synchronized
}

void ThreadSafety::PreCallRecordGetFenceStatus(VkDevice device, VkFence fence) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetFenceStatus);
    StartReadObject(fence, vvl::Func::vkGetFenceStatus);
}

void ThreadSafety::PostCallRecordGetFenceStatus(VkDevice device, VkFence fence, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetFenceStatus);
    FinishReadObject(fence, vvl::Func::vkGetFenceStatus);
}

void ThreadSafety::PreCallRecordWaitForFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences, VkBool32 waitAll,
                                              uint64_t timeout) {
    StartReadObjectParentInstance(device, vvl::Func::vkWaitForFences);

    if (pFences) {
        for (uint32_t index = 0; index < fenceCount; index++) {
            StartReadObject(pFences[index], vvl::Func::vkWaitForFences);
        }
    }
}

void ThreadSafety::PostCallRecordWaitForFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences, VkBool32 waitAll,
                                               uint64_t timeout, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkWaitForFences);

    if (pFences) {
        for (uint32_t index = 0; index < fenceCount; index++) {
            FinishReadObject(pFences[index], vvl::Func::vkWaitForFences);
        }
    }
}

void ThreadSafety::PreCallRecordCreateSemaphore(VkDevice device, const VkSemaphoreCreateInfo* pCreateInfo,
                                                const VkAllocationCallbacks* pAllocator, VkSemaphore* pSemaphore) {
    StartReadObjectParentInstance(device, vvl::Func::vkCreateSemaphore);
}

void ThreadSafety::PostCallRecordCreateSemaphore(VkDevice device, const VkSemaphoreCreateInfo* pCreateInfo,
                                                 const VkAllocationCallbacks* pAllocator, VkSemaphore* pSemaphore,
                                                 const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkCreateSemaphore);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pSemaphore);
    }
}

void ThreadSafety::PreCallRecordDestroySemaphore(VkDevice device, VkSemaphore semaphore, const VkAllocationCallbacks* pAllocator) {
    StartReadObjectParentInstance(device, vvl::Func::vkDestroySemaphore);
    StartWriteObject(semaphore, vvl::Func::vkDestroySemaphore);
    // Host access to semaphore must be externally synchronized
}

void ThreadSafety::PostCallRecordDestroySemaphore(VkDevice device, VkSemaphore semaphore, const VkAllocationCallbacks* pAllocator,
                                                  const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkDestroySemaphore);
    FinishWriteObject(semaphore, vvl::Func::vkDestroySemaphore);
    DestroyObject(semaphore);
    // Host access to semaphore must be externally synchronized
}

void ThreadSafety::PreCallRecordCreateEvent(VkDevice device, const VkEventCreateInfo* pCreateInfo,
                                            const VkAllocationCallbacks* pAllocator, VkEvent* pEvent) {
    StartReadObjectParentInstance(device, vvl::Func::vkCreateEvent);
}

void ThreadSafety::PostCallRecordCreateEvent(VkDevice device, const VkEventCreateInfo* pCreateInfo,
                                             const VkAllocationCallbacks* pAllocator, VkEvent* pEvent,
                                             const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkCreateEvent);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pEvent);
    }
}

void ThreadSafety::PreCallRecordDestroyEvent(VkDevice device, VkEvent event, const VkAllocationCallbacks* pAllocator) {
    StartReadObjectParentInstance(device, vvl::Func::vkDestroyEvent);
    StartWriteObject(event, vvl::Func::vkDestroyEvent);
    // Host access to event must be externally synchronized
}

void ThreadSafety::PostCallRecordDestroyEvent(VkDevice device, VkEvent event, const VkAllocationCallbacks* pAllocator,
                                              const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkDestroyEvent);
    FinishWriteObject(event, vvl::Func::vkDestroyEvent);
    DestroyObject(event);
    // Host access to event must be externally synchronized
}

void ThreadSafety::PreCallRecordGetEventStatus(VkDevice device, VkEvent event) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetEventStatus);
    StartReadObject(event, vvl::Func::vkGetEventStatus);
}

void ThreadSafety::PostCallRecordGetEventStatus(VkDevice device, VkEvent event, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetEventStatus);
    FinishReadObject(event, vvl::Func::vkGetEventStatus);
}

void ThreadSafety::PreCallRecordSetEvent(VkDevice device, VkEvent event) {
    StartReadObjectParentInstance(device, vvl::Func::vkSetEvent);
    StartWriteObject(event, vvl::Func::vkSetEvent);
    // Host access to event must be externally synchronized
}

void ThreadSafety::PostCallRecordSetEvent(VkDevice device, VkEvent event, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkSetEvent);
    FinishWriteObject(event, vvl::Func::vkSetEvent);
    // Host access to event must be externally synchronized
}

void ThreadSafety::PreCallRecordResetEvent(VkDevice device, VkEvent event) {
    StartReadObjectParentInstance(device, vvl::Func::vkResetEvent);
    StartWriteObject(event, vvl::Func::vkResetEvent);
    // Host access to event must be externally synchronized
}

void ThreadSafety::PostCallRecordResetEvent(VkDevice device, VkEvent event, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkResetEvent);
    FinishWriteObject(event, vvl::Func::vkResetEvent);
    // Host access to event must be externally synchronized
}

void ThreadSafety::PreCallRecordCreateQueryPool(VkDevice device, const VkQueryPoolCreateInfo* pCreateInfo,
                                                const VkAllocationCallbacks* pAllocator, VkQueryPool* pQueryPool) {
    StartReadObjectParentInstance(device, vvl::Func::vkCreateQueryPool);
}

void ThreadSafety::PostCallRecordCreateQueryPool(VkDevice device, const VkQueryPoolCreateInfo* pCreateInfo,
                                                 const VkAllocationCallbacks* pAllocator, VkQueryPool* pQueryPool,
                                                 const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkCreateQueryPool);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pQueryPool);
    }
}

void ThreadSafety::PreCallRecordDestroyQueryPool(VkDevice device, VkQueryPool queryPool, const VkAllocationCallbacks* pAllocator) {
    StartReadObjectParentInstance(device, vvl::Func::vkDestroyQueryPool);
    StartWriteObject(queryPool, vvl::Func::vkDestroyQueryPool);
    // Host access to queryPool must be externally synchronized
}

void ThreadSafety::PostCallRecordDestroyQueryPool(VkDevice device, VkQueryPool queryPool, const VkAllocationCallbacks* pAllocator,
                                                  const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkDestroyQueryPool);
    FinishWriteObject(queryPool, vvl::Func::vkDestroyQueryPool);
    DestroyObject(queryPool);
    // Host access to queryPool must be externally synchronized
}

void ThreadSafety::PreCallRecordGetQueryPoolResults(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery,
                                                    uint32_t queryCount, size_t dataSize, void* pData, VkDeviceSize stride,
                                                    VkQueryResultFlags flags) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetQueryPoolResults);
    StartReadObject(queryPool, vvl::Func::vkGetQueryPoolResults);
}

void ThreadSafety::PostCallRecordGetQueryPoolResults(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery,
                                                     uint32_t queryCount, size_t dataSize, void* pData, VkDeviceSize stride,
                                                     VkQueryResultFlags flags, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetQueryPoolResults);
    FinishReadObject(queryPool, vvl::Func::vkGetQueryPoolResults);
}

void ThreadSafety::PreCallRecordCreateBuffer(VkDevice device, const VkBufferCreateInfo* pCreateInfo,
                                             const VkAllocationCallbacks* pAllocator, VkBuffer* pBuffer) {
    StartReadObjectParentInstance(device, vvl::Func::vkCreateBuffer);
}

void ThreadSafety::PostCallRecordCreateBuffer(VkDevice device, const VkBufferCreateInfo* pCreateInfo,
                                              const VkAllocationCallbacks* pAllocator, VkBuffer* pBuffer,
                                              const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkCreateBuffer);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pBuffer);
    }
}

void ThreadSafety::PreCallRecordDestroyBuffer(VkDevice device, VkBuffer buffer, const VkAllocationCallbacks* pAllocator) {
    StartReadObjectParentInstance(device, vvl::Func::vkDestroyBuffer);
    StartWriteObject(buffer, vvl::Func::vkDestroyBuffer);
    // Host access to buffer must be externally synchronized
}

void ThreadSafety::PostCallRecordDestroyBuffer(VkDevice device, VkBuffer buffer, const VkAllocationCallbacks* pAllocator,
                                               const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkDestroyBuffer);
    FinishWriteObject(buffer, vvl::Func::vkDestroyBuffer);
    DestroyObject(buffer);
    // Host access to buffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCreateBufferView(VkDevice device, const VkBufferViewCreateInfo* pCreateInfo,
                                                 const VkAllocationCallbacks* pAllocator, VkBufferView* pView) {
    StartReadObjectParentInstance(device, vvl::Func::vkCreateBufferView);
}

void ThreadSafety::PostCallRecordCreateBufferView(VkDevice device, const VkBufferViewCreateInfo* pCreateInfo,
                                                  const VkAllocationCallbacks* pAllocator, VkBufferView* pView,
                                                  const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkCreateBufferView);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pView);
    }
}

void ThreadSafety::PreCallRecordDestroyBufferView(VkDevice device, VkBufferView bufferView,
                                                  const VkAllocationCallbacks* pAllocator) {
    StartReadObjectParentInstance(device, vvl::Func::vkDestroyBufferView);
    StartWriteObject(bufferView, vvl::Func::vkDestroyBufferView);
    // Host access to bufferView must be externally synchronized
}

void ThreadSafety::PostCallRecordDestroyBufferView(VkDevice device, VkBufferView bufferView,
                                                   const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkDestroyBufferView);
    FinishWriteObject(bufferView, vvl::Func::vkDestroyBufferView);
    DestroyObject(bufferView);
    // Host access to bufferView must be externally synchronized
}

void ThreadSafety::PreCallRecordCreateImage(VkDevice device, const VkImageCreateInfo* pCreateInfo,
                                            const VkAllocationCallbacks* pAllocator, VkImage* pImage) {
    StartReadObjectParentInstance(device, vvl::Func::vkCreateImage);
}

void ThreadSafety::PostCallRecordCreateImage(VkDevice device, const VkImageCreateInfo* pCreateInfo,
                                             const VkAllocationCallbacks* pAllocator, VkImage* pImage,
                                             const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkCreateImage);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pImage);
    }
}

void ThreadSafety::PreCallRecordDestroyImage(VkDevice device, VkImage image, const VkAllocationCallbacks* pAllocator) {
    StartReadObjectParentInstance(device, vvl::Func::vkDestroyImage);
    StartWriteObject(image, vvl::Func::vkDestroyImage);
    // Host access to image must be externally synchronized
}

void ThreadSafety::PostCallRecordDestroyImage(VkDevice device, VkImage image, const VkAllocationCallbacks* pAllocator,
                                              const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkDestroyImage);
    FinishWriteObject(image, vvl::Func::vkDestroyImage);
    DestroyObject(image);
    // Host access to image must be externally synchronized
}

void ThreadSafety::PreCallRecordGetImageSubresourceLayout(VkDevice device, VkImage image, const VkImageSubresource* pSubresource,
                                                          VkSubresourceLayout* pLayout) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetImageSubresourceLayout);
    StartReadObject(image, vvl::Func::vkGetImageSubresourceLayout);
}

void ThreadSafety::PostCallRecordGetImageSubresourceLayout(VkDevice device, VkImage image, const VkImageSubresource* pSubresource,
                                                           VkSubresourceLayout* pLayout, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetImageSubresourceLayout);
    FinishReadObject(image, vvl::Func::vkGetImageSubresourceLayout);
}

void ThreadSafety::PreCallRecordCreateImageView(VkDevice device, const VkImageViewCreateInfo* pCreateInfo,
                                                const VkAllocationCallbacks* pAllocator, VkImageView* pView) {
    StartReadObjectParentInstance(device, vvl::Func::vkCreateImageView);
}

void ThreadSafety::PostCallRecordCreateImageView(VkDevice device, const VkImageViewCreateInfo* pCreateInfo,
                                                 const VkAllocationCallbacks* pAllocator, VkImageView* pView,
                                                 const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkCreateImageView);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pView);
    }
}

void ThreadSafety::PreCallRecordDestroyImageView(VkDevice device, VkImageView imageView, const VkAllocationCallbacks* pAllocator) {
    StartReadObjectParentInstance(device, vvl::Func::vkDestroyImageView);
    StartWriteObject(imageView, vvl::Func::vkDestroyImageView);
    // Host access to imageView must be externally synchronized
}

void ThreadSafety::PostCallRecordDestroyImageView(VkDevice device, VkImageView imageView, const VkAllocationCallbacks* pAllocator,
                                                  const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkDestroyImageView);
    FinishWriteObject(imageView, vvl::Func::vkDestroyImageView);
    DestroyObject(imageView);
    // Host access to imageView must be externally synchronized
}

void ThreadSafety::PreCallRecordCreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo,
                                                   const VkAllocationCallbacks* pAllocator, VkShaderModule* pShaderModule) {
    StartReadObjectParentInstance(device, vvl::Func::vkCreateShaderModule);
}

void ThreadSafety::PostCallRecordCreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo,
                                                    const VkAllocationCallbacks* pAllocator, VkShaderModule* pShaderModule,
                                                    const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkCreateShaderModule);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pShaderModule);
    }
}

void ThreadSafety::PreCallRecordDestroyShaderModule(VkDevice device, VkShaderModule shaderModule,
                                                    const VkAllocationCallbacks* pAllocator) {
    StartReadObjectParentInstance(device, vvl::Func::vkDestroyShaderModule);
    StartWriteObject(shaderModule, vvl::Func::vkDestroyShaderModule);
    // Host access to shaderModule must be externally synchronized
}

void ThreadSafety::PostCallRecordDestroyShaderModule(VkDevice device, VkShaderModule shaderModule,
                                                     const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkDestroyShaderModule);
    FinishWriteObject(shaderModule, vvl::Func::vkDestroyShaderModule);
    DestroyObject(shaderModule);
    // Host access to shaderModule must be externally synchronized
}

void ThreadSafety::PreCallRecordCreatePipelineCache(VkDevice device, const VkPipelineCacheCreateInfo* pCreateInfo,
                                                    const VkAllocationCallbacks* pAllocator, VkPipelineCache* pPipelineCache) {
    StartReadObjectParentInstance(device, vvl::Func::vkCreatePipelineCache);
}

void ThreadSafety::PostCallRecordCreatePipelineCache(VkDevice device, const VkPipelineCacheCreateInfo* pCreateInfo,
                                                     const VkAllocationCallbacks* pAllocator, VkPipelineCache* pPipelineCache,
                                                     const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkCreatePipelineCache);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pPipelineCache);
    }
}

void ThreadSafety::PreCallRecordDestroyPipelineCache(VkDevice device, VkPipelineCache pipelineCache,
                                                     const VkAllocationCallbacks* pAllocator) {
    StartReadObjectParentInstance(device, vvl::Func::vkDestroyPipelineCache);
    StartWriteObject(pipelineCache, vvl::Func::vkDestroyPipelineCache);
    // Host access to pipelineCache must be externally synchronized
}

void ThreadSafety::PostCallRecordDestroyPipelineCache(VkDevice device, VkPipelineCache pipelineCache,
                                                      const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkDestroyPipelineCache);
    FinishWriteObject(pipelineCache, vvl::Func::vkDestroyPipelineCache);
    DestroyObject(pipelineCache);
    // Host access to pipelineCache must be externally synchronized
}

void ThreadSafety::PreCallRecordGetPipelineCacheData(VkDevice device, VkPipelineCache pipelineCache, size_t* pDataSize,
                                                     void* pData) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetPipelineCacheData);
    StartReadObject(pipelineCache, vvl::Func::vkGetPipelineCacheData);
}

void ThreadSafety::PostCallRecordGetPipelineCacheData(VkDevice device, VkPipelineCache pipelineCache, size_t* pDataSize,
                                                      void* pData, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetPipelineCacheData);
    FinishReadObject(pipelineCache, vvl::Func::vkGetPipelineCacheData);
}

void ThreadSafety::PreCallRecordMergePipelineCaches(VkDevice device, VkPipelineCache dstCache, uint32_t srcCacheCount,
                                                    const VkPipelineCache* pSrcCaches) {
    StartReadObjectParentInstance(device, vvl::Func::vkMergePipelineCaches);
    StartWriteObject(dstCache, vvl::Func::vkMergePipelineCaches);

    if (pSrcCaches) {
        for (uint32_t index = 0; index < srcCacheCount; index++) {
            StartReadObject(pSrcCaches[index], vvl::Func::vkMergePipelineCaches);
        }
    }
    // Host access to dstCache must be externally synchronized
}

void ThreadSafety::PostCallRecordMergePipelineCaches(VkDevice device, VkPipelineCache dstCache, uint32_t srcCacheCount,
                                                     const VkPipelineCache* pSrcCaches, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkMergePipelineCaches);
    FinishWriteObject(dstCache, vvl::Func::vkMergePipelineCaches);

    if (pSrcCaches) {
        for (uint32_t index = 0; index < srcCacheCount; index++) {
            FinishReadObject(pSrcCaches[index], vvl::Func::vkMergePipelineCaches);
        }
    }
    // Host access to dstCache must be externally synchronized
}

void ThreadSafety::PreCallRecordCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                        const VkGraphicsPipelineCreateInfo* pCreateInfos,
                                                        const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines) {
    StartReadObjectParentInstance(device, vvl::Func::vkCreateGraphicsPipelines);
    StartReadObject(pipelineCache, vvl::Func::vkCreateGraphicsPipelines);
}

void ThreadSafety::PostCallRecordCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                         const VkGraphicsPipelineCreateInfo* pCreateInfos,
                                                         const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                         const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkCreateGraphicsPipelines);
    FinishReadObject(pipelineCache, vvl::Func::vkCreateGraphicsPipelines);
    if (pPipelines) {
        for (uint32_t index = 0; index < createInfoCount; index++) {
            if (!pPipelines[index]) continue;
            CreateObject(pPipelines[index]);
        }
    }
}

void ThreadSafety::PreCallRecordCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                       const VkComputePipelineCreateInfo* pCreateInfos,
                                                       const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines) {
    StartReadObjectParentInstance(device, vvl::Func::vkCreateComputePipelines);
    StartReadObject(pipelineCache, vvl::Func::vkCreateComputePipelines);
}

void ThreadSafety::PostCallRecordCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                        const VkComputePipelineCreateInfo* pCreateInfos,
                                                        const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                        const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkCreateComputePipelines);
    FinishReadObject(pipelineCache, vvl::Func::vkCreateComputePipelines);
    if (pPipelines) {
        for (uint32_t index = 0; index < createInfoCount; index++) {
            if (!pPipelines[index]) continue;
            CreateObject(pPipelines[index]);
        }
    }
}

void ThreadSafety::PreCallRecordDestroyPipeline(VkDevice device, VkPipeline pipeline, const VkAllocationCallbacks* pAllocator) {
    StartReadObjectParentInstance(device, vvl::Func::vkDestroyPipeline);
    StartWriteObject(pipeline, vvl::Func::vkDestroyPipeline);
    // Host access to pipeline must be externally synchronized
}

void ThreadSafety::PostCallRecordDestroyPipeline(VkDevice device, VkPipeline pipeline, const VkAllocationCallbacks* pAllocator,
                                                 const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkDestroyPipeline);
    FinishWriteObject(pipeline, vvl::Func::vkDestroyPipeline);
    DestroyObject(pipeline);
    // Host access to pipeline must be externally synchronized
}

void ThreadSafety::PreCallRecordCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo* pCreateInfo,
                                                     const VkAllocationCallbacks* pAllocator, VkPipelineLayout* pPipelineLayout) {
    StartReadObjectParentInstance(device, vvl::Func::vkCreatePipelineLayout);
}

void ThreadSafety::PostCallRecordCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo* pCreateInfo,
                                                      const VkAllocationCallbacks* pAllocator, VkPipelineLayout* pPipelineLayout,
                                                      const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkCreatePipelineLayout);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pPipelineLayout);
    }
}

void ThreadSafety::PreCallRecordDestroyPipelineLayout(VkDevice device, VkPipelineLayout pipelineLayout,
                                                      const VkAllocationCallbacks* pAllocator) {
    StartReadObjectParentInstance(device, vvl::Func::vkDestroyPipelineLayout);
    StartWriteObject(pipelineLayout, vvl::Func::vkDestroyPipelineLayout);
    // Host access to pipelineLayout must be externally synchronized
}

void ThreadSafety::PostCallRecordDestroyPipelineLayout(VkDevice device, VkPipelineLayout pipelineLayout,
                                                       const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkDestroyPipelineLayout);
    FinishWriteObject(pipelineLayout, vvl::Func::vkDestroyPipelineLayout);
    DestroyObject(pipelineLayout);
    // Host access to pipelineLayout must be externally synchronized
}

void ThreadSafety::PreCallRecordCreateSampler(VkDevice device, const VkSamplerCreateInfo* pCreateInfo,
                                              const VkAllocationCallbacks* pAllocator, VkSampler* pSampler) {
    StartReadObjectParentInstance(device, vvl::Func::vkCreateSampler);
}

void ThreadSafety::PostCallRecordCreateSampler(VkDevice device, const VkSamplerCreateInfo* pCreateInfo,
                                               const VkAllocationCallbacks* pAllocator, VkSampler* pSampler,
                                               const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkCreateSampler);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pSampler);
    }
}

void ThreadSafety::PreCallRecordDestroySampler(VkDevice device, VkSampler sampler, const VkAllocationCallbacks* pAllocator) {
    StartReadObjectParentInstance(device, vvl::Func::vkDestroySampler);
    StartWriteObject(sampler, vvl::Func::vkDestroySampler);
    // Host access to sampler must be externally synchronized
}

void ThreadSafety::PostCallRecordDestroySampler(VkDevice device, VkSampler sampler, const VkAllocationCallbacks* pAllocator,
                                                const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkDestroySampler);
    FinishWriteObject(sampler, vvl::Func::vkDestroySampler);
    DestroyObject(sampler);
    // Host access to sampler must be externally synchronized
}

void ThreadSafety::PreCallRecordDestroyDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout descriptorSetLayout,
                                                           const VkAllocationCallbacks* pAllocator) {
    StartReadObjectParentInstance(device, vvl::Func::vkDestroyDescriptorSetLayout);
    StartWriteObject(descriptorSetLayout, vvl::Func::vkDestroyDescriptorSetLayout);
    // Host access to descriptorSetLayout must be externally synchronized
}

void ThreadSafety::PostCallRecordDestroyDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout descriptorSetLayout,
                                                            const VkAllocationCallbacks* pAllocator,
                                                            const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkDestroyDescriptorSetLayout);
    FinishWriteObject(descriptorSetLayout, vvl::Func::vkDestroyDescriptorSetLayout);
    DestroyObject(descriptorSetLayout);
    // Host access to descriptorSetLayout must be externally synchronized
}

void ThreadSafety::PreCallRecordCreateDescriptorPool(VkDevice device, const VkDescriptorPoolCreateInfo* pCreateInfo,
                                                     const VkAllocationCallbacks* pAllocator, VkDescriptorPool* pDescriptorPool) {
    StartReadObjectParentInstance(device, vvl::Func::vkCreateDescriptorPool);
}

void ThreadSafety::PostCallRecordCreateDescriptorPool(VkDevice device, const VkDescriptorPoolCreateInfo* pCreateInfo,
                                                      const VkAllocationCallbacks* pAllocator, VkDescriptorPool* pDescriptorPool,
                                                      const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkCreateDescriptorPool);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pDescriptorPool);
    }
}

void ThreadSafety::PreCallRecordCreateFramebuffer(VkDevice device, const VkFramebufferCreateInfo* pCreateInfo,
                                                  const VkAllocationCallbacks* pAllocator, VkFramebuffer* pFramebuffer) {
    StartReadObjectParentInstance(device, vvl::Func::vkCreateFramebuffer);
}

void ThreadSafety::PostCallRecordCreateFramebuffer(VkDevice device, const VkFramebufferCreateInfo* pCreateInfo,
                                                   const VkAllocationCallbacks* pAllocator, VkFramebuffer* pFramebuffer,
                                                   const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkCreateFramebuffer);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pFramebuffer);
    }
}

void ThreadSafety::PreCallRecordDestroyFramebuffer(VkDevice device, VkFramebuffer framebuffer,
                                                   const VkAllocationCallbacks* pAllocator) {
    StartReadObjectParentInstance(device, vvl::Func::vkDestroyFramebuffer);
    StartWriteObject(framebuffer, vvl::Func::vkDestroyFramebuffer);
    // Host access to framebuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordDestroyFramebuffer(VkDevice device, VkFramebuffer framebuffer,
                                                    const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkDestroyFramebuffer);
    FinishWriteObject(framebuffer, vvl::Func::vkDestroyFramebuffer);
    DestroyObject(framebuffer);
    // Host access to framebuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCreateRenderPass(VkDevice device, const VkRenderPassCreateInfo* pCreateInfo,
                                                 const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass) {
    StartReadObjectParentInstance(device, vvl::Func::vkCreateRenderPass);
}

void ThreadSafety::PostCallRecordCreateRenderPass(VkDevice device, const VkRenderPassCreateInfo* pCreateInfo,
                                                  const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass,
                                                  const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkCreateRenderPass);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pRenderPass);
    }
}

void ThreadSafety::PreCallRecordDestroyRenderPass(VkDevice device, VkRenderPass renderPass,
                                                  const VkAllocationCallbacks* pAllocator) {
    StartReadObjectParentInstance(device, vvl::Func::vkDestroyRenderPass);
    StartWriteObject(renderPass, vvl::Func::vkDestroyRenderPass);
    // Host access to renderPass must be externally synchronized
}

void ThreadSafety::PostCallRecordDestroyRenderPass(VkDevice device, VkRenderPass renderPass,
                                                   const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkDestroyRenderPass);
    FinishWriteObject(renderPass, vvl::Func::vkDestroyRenderPass);
    DestroyObject(renderPass);
    // Host access to renderPass must be externally synchronized
}

void ThreadSafety::PreCallRecordGetRenderAreaGranularity(VkDevice device, VkRenderPass renderPass, VkExtent2D* pGranularity) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetRenderAreaGranularity);
    StartReadObject(renderPass, vvl::Func::vkGetRenderAreaGranularity);
}

void ThreadSafety::PostCallRecordGetRenderAreaGranularity(VkDevice device, VkRenderPass renderPass, VkExtent2D* pGranularity,
                                                          const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetRenderAreaGranularity);
    FinishReadObject(renderPass, vvl::Func::vkGetRenderAreaGranularity);
}

void ThreadSafety::PreCallRecordBeginCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo* pBeginInfo) {
    StartWriteObject(commandBuffer, vvl::Func::vkBeginCommandBuffer);
    // Host access to commandBuffer must be externally synchronized
    // the sname:VkCommandPool that pname:commandBuffer was allocated from must be externally synchronized between host accesses
}

void ThreadSafety::PostCallRecordBeginCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo* pBeginInfo,
                                                    const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkBeginCommandBuffer);
    // Host access to commandBuffer must be externally synchronized
    // the sname:VkCommandPool that pname:commandBuffer was allocated from must be externally synchronized between host accesses
}

void ThreadSafety::PreCallRecordEndCommandBuffer(VkCommandBuffer commandBuffer) {
    StartWriteObject(commandBuffer, vvl::Func::vkEndCommandBuffer);
    // Host access to commandBuffer must be externally synchronized
    // the sname:VkCommandPool that pname:commandBuffer was allocated from must be externally synchronized between host accesses
}

void ThreadSafety::PostCallRecordEndCommandBuffer(VkCommandBuffer commandBuffer, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkEndCommandBuffer);
    // Host access to commandBuffer must be externally synchronized
    // the sname:VkCommandPool that pname:commandBuffer was allocated from must be externally synchronized between host accesses
}

void ThreadSafety::PreCallRecordResetCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferResetFlags flags) {
    StartWriteObject(commandBuffer, vvl::Func::vkResetCommandBuffer);
    // Host access to commandBuffer must be externally synchronized
    // the sname:VkCommandPool that pname:commandBuffer was allocated from must be externally synchronized between host accesses
}

void ThreadSafety::PostCallRecordResetCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferResetFlags flags,
                                                    const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkResetCommandBuffer);
    // Host access to commandBuffer must be externally synchronized
    // the sname:VkCommandPool that pname:commandBuffer was allocated from must be externally synchronized between host accesses
}

void ThreadSafety::PreCallRecordCmdBindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                VkPipeline pipeline) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdBindPipeline);
    StartReadObject(pipeline, vvl::Func::vkCmdBindPipeline);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdBindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                 VkPipeline pipeline, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdBindPipeline);
    FinishReadObject(pipeline, vvl::Func::vkCmdBindPipeline);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetViewport(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount,
                                               const VkViewport* pViewports) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdSetViewport);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetViewport(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount,
                                                const VkViewport* pViewports, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdSetViewport);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetScissor(VkCommandBuffer commandBuffer, uint32_t firstScissor, uint32_t scissorCount,
                                              const VkRect2D* pScissors) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdSetScissor);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetScissor(VkCommandBuffer commandBuffer, uint32_t firstScissor, uint32_t scissorCount,
                                               const VkRect2D* pScissors, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdSetScissor);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetLineWidth(VkCommandBuffer commandBuffer, float lineWidth) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdSetLineWidth);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetLineWidth(VkCommandBuffer commandBuffer, float lineWidth, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdSetLineWidth);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetDepthBias(VkCommandBuffer commandBuffer, float depthBiasConstantFactor, float depthBiasClamp,
                                                float depthBiasSlopeFactor) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdSetDepthBias);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetDepthBias(VkCommandBuffer commandBuffer, float depthBiasConstantFactor, float depthBiasClamp,
                                                 float depthBiasSlopeFactor, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdSetDepthBias);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetBlendConstants(VkCommandBuffer commandBuffer, const float blendConstants[4]) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdSetBlendConstants);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetBlendConstants(VkCommandBuffer commandBuffer, const float blendConstants[4],
                                                      const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdSetBlendConstants);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetDepthBounds(VkCommandBuffer commandBuffer, float minDepthBounds, float maxDepthBounds) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdSetDepthBounds);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetDepthBounds(VkCommandBuffer commandBuffer, float minDepthBounds, float maxDepthBounds,
                                                   const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdSetDepthBounds);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetStencilCompareMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask,
                                                         uint32_t compareMask) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdSetStencilCompareMask);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetStencilCompareMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask,
                                                          uint32_t compareMask, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdSetStencilCompareMask);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetStencilWriteMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask,
                                                       uint32_t writeMask) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdSetStencilWriteMask);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetStencilWriteMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask,
                                                        uint32_t writeMask, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdSetStencilWriteMask);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetStencilReference(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask,
                                                       uint32_t reference) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdSetStencilReference);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetStencilReference(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask,
                                                        uint32_t reference, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdSetStencilReference);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdBindDescriptorSets(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                      VkPipelineLayout layout, uint32_t firstSet, uint32_t descriptorSetCount,
                                                      const VkDescriptorSet* pDescriptorSets, uint32_t dynamicOffsetCount,
                                                      const uint32_t* pDynamicOffsets) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdBindDescriptorSets);
    StartReadObject(layout, vvl::Func::vkCmdBindDescriptorSets);

    if (pDescriptorSets) {
        for (uint32_t index = 0; index < descriptorSetCount; index++) {
            StartReadObject(pDescriptorSets[index], vvl::Func::vkCmdBindDescriptorSets);
        }
    }
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdBindDescriptorSets(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                       VkPipelineLayout layout, uint32_t firstSet, uint32_t descriptorSetCount,
                                                       const VkDescriptorSet* pDescriptorSets, uint32_t dynamicOffsetCount,
                                                       const uint32_t* pDynamicOffsets, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdBindDescriptorSets);
    FinishReadObject(layout, vvl::Func::vkCmdBindDescriptorSets);

    if (pDescriptorSets) {
        for (uint32_t index = 0; index < descriptorSetCount; index++) {
            FinishReadObject(pDescriptorSets[index], vvl::Func::vkCmdBindDescriptorSets);
        }
    }
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdBindIndexBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                   VkIndexType indexType) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdBindIndexBuffer);
    StartReadObject(buffer, vvl::Func::vkCmdBindIndexBuffer);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdBindIndexBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                    VkIndexType indexType, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdBindIndexBuffer);
    FinishReadObject(buffer, vvl::Func::vkCmdBindIndexBuffer);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdBindVertexBuffers(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount,
                                                     const VkBuffer* pBuffers, const VkDeviceSize* pOffsets) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdBindVertexBuffers);

    if (pBuffers) {
        for (uint32_t index = 0; index < bindingCount; index++) {
            StartReadObject(pBuffers[index], vvl::Func::vkCmdBindVertexBuffers);
        }
    }
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdBindVertexBuffers(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount,
                                                      const VkBuffer* pBuffers, const VkDeviceSize* pOffsets,
                                                      const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdBindVertexBuffers);

    if (pBuffers) {
        for (uint32_t index = 0; index < bindingCount; index++) {
            FinishReadObject(pBuffers[index], vvl::Func::vkCmdBindVertexBuffers);
        }
    }
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount,
                                        uint32_t firstVertex, uint32_t firstInstance) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdDraw);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount,
                                         uint32_t firstVertex, uint32_t firstInstance, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdDraw);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount,
                                               uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdDrawIndexed);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount,
                                                uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance,
                                                const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdDrawIndexed);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                uint32_t drawCount, uint32_t stride) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdDrawIndirect);
    StartReadObject(buffer, vvl::Func::vkCmdDrawIndirect);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                 uint32_t drawCount, uint32_t stride, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdDrawIndirect);
    FinishReadObject(buffer, vvl::Func::vkCmdDrawIndirect);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                       uint32_t drawCount, uint32_t stride) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdDrawIndexedIndirect);
    StartReadObject(buffer, vvl::Func::vkCmdDrawIndexedIndirect);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                        uint32_t drawCount, uint32_t stride, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdDrawIndexedIndirect);
    FinishReadObject(buffer, vvl::Func::vkCmdDrawIndexedIndirect);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdDispatch(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY,
                                            uint32_t groupCountZ) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdDispatch);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdDispatch(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY,
                                             uint32_t groupCountZ, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdDispatch);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdDispatchIndirect);
    StartReadObject(buffer, vvl::Func::vkCmdDispatchIndirect);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                     const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdDispatchIndirect);
    FinishReadObject(buffer, vvl::Func::vkCmdDispatchIndirect);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer,
                                              uint32_t regionCount, const VkBufferCopy* pRegions) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdCopyBuffer);
    StartReadObject(srcBuffer, vvl::Func::vkCmdCopyBuffer);
    StartReadObject(dstBuffer, vvl::Func::vkCmdCopyBuffer);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer,
                                               uint32_t regionCount, const VkBufferCopy* pRegions, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdCopyBuffer);
    FinishReadObject(srcBuffer, vvl::Func::vkCmdCopyBuffer);
    FinishReadObject(dstBuffer, vvl::Func::vkCmdCopyBuffer);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                             VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                             const VkImageCopy* pRegions) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdCopyImage);
    StartReadObject(srcImage, vvl::Func::vkCmdCopyImage);
    StartReadObject(dstImage, vvl::Func::vkCmdCopyImage);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                              VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                              const VkImageCopy* pRegions, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdCopyImage);
    FinishReadObject(srcImage, vvl::Func::vkCmdCopyImage);
    FinishReadObject(dstImage, vvl::Func::vkCmdCopyImage);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                             VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                             const VkImageBlit* pRegions, VkFilter filter) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdBlitImage);
    StartReadObject(srcImage, vvl::Func::vkCmdBlitImage);
    StartReadObject(dstImage, vvl::Func::vkCmdBlitImage);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                              VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                              const VkImageBlit* pRegions, VkFilter filter, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdBlitImage);
    FinishReadObject(srcImage, vvl::Func::vkCmdBlitImage);
    FinishReadObject(dstImage, vvl::Func::vkCmdBlitImage);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage,
                                                     VkImageLayout dstImageLayout, uint32_t regionCount,
                                                     const VkBufferImageCopy* pRegions) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdCopyBufferToImage);
    StartReadObject(srcBuffer, vvl::Func::vkCmdCopyBufferToImage);
    StartReadObject(dstImage, vvl::Func::vkCmdCopyBufferToImage);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage,
                                                      VkImageLayout dstImageLayout, uint32_t regionCount,
                                                      const VkBufferImageCopy* pRegions, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdCopyBufferToImage);
    FinishReadObject(srcBuffer, vvl::Func::vkCmdCopyBufferToImage);
    FinishReadObject(dstImage, vvl::Func::vkCmdCopyBufferToImage);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                                     VkBuffer dstBuffer, uint32_t regionCount, const VkBufferImageCopy* pRegions) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdCopyImageToBuffer);
    StartReadObject(srcImage, vvl::Func::vkCmdCopyImageToBuffer);
    StartReadObject(dstBuffer, vvl::Func::vkCmdCopyImageToBuffer);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                                      VkBuffer dstBuffer, uint32_t regionCount, const VkBufferImageCopy* pRegions,
                                                      const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdCopyImageToBuffer);
    FinishReadObject(srcImage, vvl::Func::vkCmdCopyImageToBuffer);
    FinishReadObject(dstBuffer, vvl::Func::vkCmdCopyImageToBuffer);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                                VkDeviceSize dataSize, const void* pData) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdUpdateBuffer);
    StartReadObject(dstBuffer, vvl::Func::vkCmdUpdateBuffer);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                                 VkDeviceSize dataSize, const void* pData, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdUpdateBuffer);
    FinishReadObject(dstBuffer, vvl::Func::vkCmdUpdateBuffer);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                              VkDeviceSize size, uint32_t data) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdFillBuffer);
    StartReadObject(dstBuffer, vvl::Func::vkCmdFillBuffer);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                               VkDeviceSize size, uint32_t data, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdFillBuffer);
    FinishReadObject(dstBuffer, vvl::Func::vkCmdFillBuffer);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                                   const VkClearColorValue* pColor, uint32_t rangeCount,
                                                   const VkImageSubresourceRange* pRanges) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdClearColorImage);
    StartReadObject(image, vvl::Func::vkCmdClearColorImage);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                                    const VkClearColorValue* pColor, uint32_t rangeCount,
                                                    const VkImageSubresourceRange* pRanges, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdClearColorImage);
    FinishReadObject(image, vvl::Func::vkCmdClearColorImage);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                                          const VkClearDepthStencilValue* pDepthStencil, uint32_t rangeCount,
                                                          const VkImageSubresourceRange* pRanges) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdClearDepthStencilImage);
    StartReadObject(image, vvl::Func::vkCmdClearDepthStencilImage);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                                           const VkClearDepthStencilValue* pDepthStencil, uint32_t rangeCount,
                                                           const VkImageSubresourceRange* pRanges, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdClearDepthStencilImage);
    FinishReadObject(image, vvl::Func::vkCmdClearDepthStencilImage);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
                                                    const VkClearAttachment* pAttachments, uint32_t rectCount,
                                                    const VkClearRect* pRects) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdClearAttachments);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
                                                     const VkClearAttachment* pAttachments, uint32_t rectCount,
                                                     const VkClearRect* pRects, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdClearAttachments);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                                VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                                const VkImageResolve* pRegions) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdResolveImage);
    StartReadObject(srcImage, vvl::Func::vkCmdResolveImage);
    StartReadObject(dstImage, vvl::Func::vkCmdResolveImage);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                                 VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                                 const VkImageResolve* pRegions, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdResolveImage);
    FinishReadObject(srcImage, vvl::Func::vkCmdResolveImage);
    FinishReadObject(dstImage, vvl::Func::vkCmdResolveImage);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdSetEvent);
    StartReadObject(event, vvl::Func::vkCmdSetEvent);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask,
                                             const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdSetEvent);
    FinishReadObject(event, vvl::Func::vkCmdSetEvent);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdResetEvent);
    StartReadObject(event, vvl::Func::vkCmdResetEvent);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask,
                                               const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdResetEvent);
    FinishReadObject(event, vvl::Func::vkCmdResetEvent);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                              VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask,
                                              uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers,
                                              uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers,
                                              uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdWaitEvents);

    if (pEvents) {
        for (uint32_t index = 0; index < eventCount; index++) {
            StartReadObject(pEvents[index], vvl::Func::vkCmdWaitEvents);
        }
    }
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                               VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask,
                                               uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers,
                                               uint32_t bufferMemoryBarrierCount,
                                               const VkBufferMemoryBarrier* pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount,
                                               const VkImageMemoryBarrier* pImageMemoryBarriers, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdWaitEvents);

    if (pEvents) {
        for (uint32_t index = 0; index < eventCount; index++) {
            FinishReadObject(pEvents[index], vvl::Func::vkCmdWaitEvents);
        }
    }
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask,
                                                   VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags,
                                                   uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers,
                                                   uint32_t bufferMemoryBarrierCount,
                                                   const VkBufferMemoryBarrier* pBufferMemoryBarriers,
                                                   uint32_t imageMemoryBarrierCount,
                                                   const VkImageMemoryBarrier* pImageMemoryBarriers) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdPipelineBarrier);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdPipelineBarrier(
    VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask,
    VkDependencyFlags dependencyFlags, uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers,
    uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount,
    const VkImageMemoryBarrier* pImageMemoryBarriers, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdPipelineBarrier);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdBeginQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query,
                                              VkQueryControlFlags flags) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdBeginQuery);
    StartReadObject(queryPool, vvl::Func::vkCmdBeginQuery);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdBeginQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query,
                                               VkQueryControlFlags flags, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdBeginQuery);
    FinishReadObject(queryPool, vvl::Func::vkCmdBeginQuery);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdEndQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdEndQuery);
    StartReadObject(queryPool, vvl::Func::vkCmdEndQuery);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdEndQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query,
                                             const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdEndQuery);
    FinishReadObject(queryPool, vvl::Func::vkCmdEndQuery);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdResetQueryPool(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery,
                                                  uint32_t queryCount) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdResetQueryPool);
    StartReadObject(queryPool, vvl::Func::vkCmdResetQueryPool);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdResetQueryPool(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery,
                                                   uint32_t queryCount, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdResetQueryPool);
    FinishReadObject(queryPool, vvl::Func::vkCmdResetQueryPool);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdWriteTimestamp(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage,
                                                  VkQueryPool queryPool, uint32_t query) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdWriteTimestamp);
    StartReadObject(queryPool, vvl::Func::vkCmdWriteTimestamp);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdWriteTimestamp(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage,
                                                   VkQueryPool queryPool, uint32_t query, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdWriteTimestamp);
    FinishReadObject(queryPool, vvl::Func::vkCmdWriteTimestamp);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdCopyQueryPoolResults(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery,
                                                        uint32_t queryCount, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                                        VkDeviceSize stride, VkQueryResultFlags flags) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdCopyQueryPoolResults);
    StartReadObject(queryPool, vvl::Func::vkCmdCopyQueryPoolResults);
    StartReadObject(dstBuffer, vvl::Func::vkCmdCopyQueryPoolResults);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdCopyQueryPoolResults(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery,
                                                         uint32_t queryCount, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                                         VkDeviceSize stride, VkQueryResultFlags flags,
                                                         const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdCopyQueryPoolResults);
    FinishReadObject(queryPool, vvl::Func::vkCmdCopyQueryPoolResults);
    FinishReadObject(dstBuffer, vvl::Func::vkCmdCopyQueryPoolResults);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdPushConstants(VkCommandBuffer commandBuffer, VkPipelineLayout layout,
                                                 VkShaderStageFlags stageFlags, uint32_t offset, uint32_t size,
                                                 const void* pValues) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdPushConstants);
    StartReadObject(layout, vvl::Func::vkCmdPushConstants);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdPushConstants(VkCommandBuffer commandBuffer, VkPipelineLayout layout,
                                                  VkShaderStageFlags stageFlags, uint32_t offset, uint32_t size,
                                                  const void* pValues, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdPushConstants);
    FinishReadObject(layout, vvl::Func::vkCmdPushConstants);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                                   VkSubpassContents contents) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdBeginRenderPass);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                                    VkSubpassContents contents, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdBeginRenderPass);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdNextSubpass);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents,
                                                const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdNextSubpass);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdEndRenderPass(VkCommandBuffer commandBuffer) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdEndRenderPass);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdEndRenderPass(VkCommandBuffer commandBuffer, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdEndRenderPass);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBufferCount,
                                                   const VkCommandBuffer* pCommandBuffers) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdExecuteCommands);

    if (pCommandBuffers) {
        for (uint32_t index = 0; index < commandBufferCount; index++) {
            StartReadObject(pCommandBuffers[index], vvl::Func::vkCmdExecuteCommands);
        }
    }
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBufferCount,
                                                    const VkCommandBuffer* pCommandBuffers, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdExecuteCommands);

    if (pCommandBuffers) {
        for (uint32_t index = 0; index < commandBufferCount; index++) {
            FinishReadObject(pCommandBuffers[index], vvl::Func::vkCmdExecuteCommands);
        }
    }
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordBindBufferMemory2(VkDevice device, uint32_t bindInfoCount,
                                                  const VkBindBufferMemoryInfo* pBindInfos) {
    StartReadObjectParentInstance(device, vvl::Func::vkBindBufferMemory2);
}

void ThreadSafety::PostCallRecordBindBufferMemory2(VkDevice device, uint32_t bindInfoCount,
                                                   const VkBindBufferMemoryInfo* pBindInfos, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkBindBufferMemory2);
}

void ThreadSafety::PreCallRecordBindImageMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo* pBindInfos) {
    StartReadObjectParentInstance(device, vvl::Func::vkBindImageMemory2);
}

void ThreadSafety::PostCallRecordBindImageMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo* pBindInfos,
                                                  const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkBindImageMemory2);
}

void ThreadSafety::PreCallRecordGetDeviceGroupPeerMemoryFeatures(VkDevice device, uint32_t heapIndex, uint32_t localDeviceIndex,
                                                                 uint32_t remoteDeviceIndex,
                                                                 VkPeerMemoryFeatureFlags* pPeerMemoryFeatures) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetDeviceGroupPeerMemoryFeatures);
}

void ThreadSafety::PostCallRecordGetDeviceGroupPeerMemoryFeatures(VkDevice device, uint32_t heapIndex, uint32_t localDeviceIndex,
                                                                  uint32_t remoteDeviceIndex,
                                                                  VkPeerMemoryFeatureFlags* pPeerMemoryFeatures,
                                                                  const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetDeviceGroupPeerMemoryFeatures);
}

void ThreadSafety::PreCallRecordCmdSetDeviceMask(VkCommandBuffer commandBuffer, uint32_t deviceMask) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdSetDeviceMask);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetDeviceMask(VkCommandBuffer commandBuffer, uint32_t deviceMask,
                                                  const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdSetDeviceMask);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdDispatchBase(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY,
                                                uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY,
                                                uint32_t groupCountZ) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdDispatchBase);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdDispatchBase(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY,
                                                 uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY,
                                                 uint32_t groupCountZ, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdDispatchBase);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordEnumeratePhysicalDeviceGroups(VkInstance instance, uint32_t* pPhysicalDeviceGroupCount,
                                                              VkPhysicalDeviceGroupProperties* pPhysicalDeviceGroupProperties) {
    StartReadObjectParentInstance(instance, vvl::Func::vkEnumeratePhysicalDeviceGroups);
}

void ThreadSafety::PostCallRecordEnumeratePhysicalDeviceGroups(VkInstance instance, uint32_t* pPhysicalDeviceGroupCount,
                                                               VkPhysicalDeviceGroupProperties* pPhysicalDeviceGroupProperties,
                                                               const RecordObject& record_obj) {
    FinishReadObjectParentInstance(instance, vvl::Func::vkEnumeratePhysicalDeviceGroups);
}

void ThreadSafety::PreCallRecordGetImageMemoryRequirements2(VkDevice device, const VkImageMemoryRequirementsInfo2* pInfo,
                                                            VkMemoryRequirements2* pMemoryRequirements) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetImageMemoryRequirements2);
}

void ThreadSafety::PostCallRecordGetImageMemoryRequirements2(VkDevice device, const VkImageMemoryRequirementsInfo2* pInfo,
                                                             VkMemoryRequirements2* pMemoryRequirements,
                                                             const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetImageMemoryRequirements2);
}

void ThreadSafety::PreCallRecordGetBufferMemoryRequirements2(VkDevice device, const VkBufferMemoryRequirementsInfo2* pInfo,
                                                             VkMemoryRequirements2* pMemoryRequirements) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetBufferMemoryRequirements2);
}

void ThreadSafety::PostCallRecordGetBufferMemoryRequirements2(VkDevice device, const VkBufferMemoryRequirementsInfo2* pInfo,
                                                              VkMemoryRequirements2* pMemoryRequirements,
                                                              const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetBufferMemoryRequirements2);
}

void ThreadSafety::PreCallRecordGetImageSparseMemoryRequirements2(VkDevice device,
                                                                  const VkImageSparseMemoryRequirementsInfo2* pInfo,
                                                                  uint32_t* pSparseMemoryRequirementCount,
                                                                  VkSparseImageMemoryRequirements2* pSparseMemoryRequirements) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetImageSparseMemoryRequirements2);
}

void ThreadSafety::PostCallRecordGetImageSparseMemoryRequirements2(VkDevice device,
                                                                   const VkImageSparseMemoryRequirementsInfo2* pInfo,
                                                                   uint32_t* pSparseMemoryRequirementCount,
                                                                   VkSparseImageMemoryRequirements2* pSparseMemoryRequirements,
                                                                   const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetImageSparseMemoryRequirements2);
}

void ThreadSafety::PreCallRecordTrimCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolTrimFlags flags) {
    StartReadObjectParentInstance(device, vvl::Func::vkTrimCommandPool);
    StartWriteObject(commandPool, vvl::Func::vkTrimCommandPool);
    // Host access to commandPool must be externally synchronized
}

void ThreadSafety::PostCallRecordTrimCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolTrimFlags flags,
                                                 const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkTrimCommandPool);
    FinishWriteObject(commandPool, vvl::Func::vkTrimCommandPool);
    // Host access to commandPool must be externally synchronized
}

void ThreadSafety::PreCallRecordCreateSamplerYcbcrConversion(VkDevice device, const VkSamplerYcbcrConversionCreateInfo* pCreateInfo,
                                                             const VkAllocationCallbacks* pAllocator,
                                                             VkSamplerYcbcrConversion* pYcbcrConversion) {
    StartReadObjectParentInstance(device, vvl::Func::vkCreateSamplerYcbcrConversion);
}

void ThreadSafety::PostCallRecordCreateSamplerYcbcrConversion(VkDevice device,
                                                              const VkSamplerYcbcrConversionCreateInfo* pCreateInfo,
                                                              const VkAllocationCallbacks* pAllocator,
                                                              VkSamplerYcbcrConversion* pYcbcrConversion,
                                                              const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkCreateSamplerYcbcrConversion);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pYcbcrConversion);
    }
}

void ThreadSafety::PreCallRecordDestroySamplerYcbcrConversion(VkDevice device, VkSamplerYcbcrConversion ycbcrConversion,
                                                              const VkAllocationCallbacks* pAllocator) {
    StartReadObjectParentInstance(device, vvl::Func::vkDestroySamplerYcbcrConversion);
    StartWriteObject(ycbcrConversion, vvl::Func::vkDestroySamplerYcbcrConversion);
    // Host access to ycbcrConversion must be externally synchronized
}

void ThreadSafety::PostCallRecordDestroySamplerYcbcrConversion(VkDevice device, VkSamplerYcbcrConversion ycbcrConversion,
                                                               const VkAllocationCallbacks* pAllocator,
                                                               const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkDestroySamplerYcbcrConversion);
    FinishWriteObject(ycbcrConversion, vvl::Func::vkDestroySamplerYcbcrConversion);
    DestroyObject(ycbcrConversion);
    // Host access to ycbcrConversion must be externally synchronized
}

void ThreadSafety::PreCallRecordCreateDescriptorUpdateTemplate(VkDevice device,
                                                               const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo,
                                                               const VkAllocationCallbacks* pAllocator,
                                                               VkDescriptorUpdateTemplate* pDescriptorUpdateTemplate) {
    StartReadObjectParentInstance(device, vvl::Func::vkCreateDescriptorUpdateTemplate);
}

void ThreadSafety::PostCallRecordCreateDescriptorUpdateTemplate(VkDevice device,
                                                                const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo,
                                                                const VkAllocationCallbacks* pAllocator,
                                                                VkDescriptorUpdateTemplate* pDescriptorUpdateTemplate,
                                                                const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkCreateDescriptorUpdateTemplate);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pDescriptorUpdateTemplate);
    }
}

void ThreadSafety::PreCallRecordDestroyDescriptorUpdateTemplate(VkDevice device,
                                                                VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                                const VkAllocationCallbacks* pAllocator) {
    StartReadObjectParentInstance(device, vvl::Func::vkDestroyDescriptorUpdateTemplate);
    StartWriteObject(descriptorUpdateTemplate, vvl::Func::vkDestroyDescriptorUpdateTemplate);
    // Host access to descriptorUpdateTemplate must be externally synchronized
}

void ThreadSafety::PostCallRecordDestroyDescriptorUpdateTemplate(VkDevice device,
                                                                 VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                                 const VkAllocationCallbacks* pAllocator,
                                                                 const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkDestroyDescriptorUpdateTemplate);
    FinishWriteObject(descriptorUpdateTemplate, vvl::Func::vkDestroyDescriptorUpdateTemplate);
    DestroyObject(descriptorUpdateTemplate);
    // Host access to descriptorUpdateTemplate must be externally synchronized
}

void ThreadSafety::PreCallRecordGetDescriptorSetLayoutSupport(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo,
                                                              VkDescriptorSetLayoutSupport* pSupport) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetDescriptorSetLayoutSupport);
}

void ThreadSafety::PostCallRecordGetDescriptorSetLayoutSupport(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo,
                                                               VkDescriptorSetLayoutSupport* pSupport,
                                                               const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetDescriptorSetLayoutSupport);
}

void ThreadSafety::PreCallRecordCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                     VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                     uint32_t stride) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdDrawIndirectCount);
    StartReadObject(buffer, vvl::Func::vkCmdDrawIndirectCount);
    StartReadObject(countBuffer, vvl::Func::vkCmdDrawIndirectCount);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                      VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                      uint32_t stride, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdDrawIndirectCount);
    FinishReadObject(buffer, vvl::Func::vkCmdDrawIndirectCount);
    FinishReadObject(countBuffer, vvl::Func::vkCmdDrawIndirectCount);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                            VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                            uint32_t maxDrawCount, uint32_t stride) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdDrawIndexedIndirectCount);
    StartReadObject(buffer, vvl::Func::vkCmdDrawIndexedIndirectCount);
    StartReadObject(countBuffer, vvl::Func::vkCmdDrawIndexedIndirectCount);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                             VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                             uint32_t maxDrawCount, uint32_t stride,
                                                             const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdDrawIndexedIndirectCount);
    FinishReadObject(buffer, vvl::Func::vkCmdDrawIndexedIndirectCount);
    FinishReadObject(countBuffer, vvl::Func::vkCmdDrawIndexedIndirectCount);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCreateRenderPass2(VkDevice device, const VkRenderPassCreateInfo2* pCreateInfo,
                                                  const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass) {
    StartReadObjectParentInstance(device, vvl::Func::vkCreateRenderPass2);
}

void ThreadSafety::PostCallRecordCreateRenderPass2(VkDevice device, const VkRenderPassCreateInfo2* pCreateInfo,
                                                   const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass,
                                                   const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkCreateRenderPass2);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pRenderPass);
    }
}

void ThreadSafety::PreCallRecordCmdBeginRenderPass2(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                                    const VkSubpassBeginInfo* pSubpassBeginInfo) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdBeginRenderPass2);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdBeginRenderPass2(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                                     const VkSubpassBeginInfo* pSubpassBeginInfo, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdBeginRenderPass2);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdNextSubpass2(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo* pSubpassBeginInfo,
                                                const VkSubpassEndInfo* pSubpassEndInfo) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdNextSubpass2);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdNextSubpass2(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo* pSubpassBeginInfo,
                                                 const VkSubpassEndInfo* pSubpassEndInfo, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdNextSubpass2);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdEndRenderPass2(VkCommandBuffer commandBuffer, const VkSubpassEndInfo* pSubpassEndInfo) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdEndRenderPass2);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdEndRenderPass2(VkCommandBuffer commandBuffer, const VkSubpassEndInfo* pSubpassEndInfo,
                                                   const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdEndRenderPass2);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordResetQueryPool(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount) {
    StartReadObjectParentInstance(device, vvl::Func::vkResetQueryPool);
    StartReadObject(queryPool, vvl::Func::vkResetQueryPool);
}

void ThreadSafety::PostCallRecordResetQueryPool(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount,
                                                const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkResetQueryPool);
    FinishReadObject(queryPool, vvl::Func::vkResetQueryPool);
}

void ThreadSafety::PreCallRecordGetSemaphoreCounterValue(VkDevice device, VkSemaphore semaphore, uint64_t* pValue) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetSemaphoreCounterValue);
    StartReadObject(semaphore, vvl::Func::vkGetSemaphoreCounterValue);
}

void ThreadSafety::PostCallRecordGetSemaphoreCounterValue(VkDevice device, VkSemaphore semaphore, uint64_t* pValue,
                                                          const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetSemaphoreCounterValue);
    FinishReadObject(semaphore, vvl::Func::vkGetSemaphoreCounterValue);
}

void ThreadSafety::PreCallRecordWaitSemaphores(VkDevice device, const VkSemaphoreWaitInfo* pWaitInfo, uint64_t timeout) {
    StartReadObjectParentInstance(device, vvl::Func::vkWaitSemaphores);
}

void ThreadSafety::PostCallRecordWaitSemaphores(VkDevice device, const VkSemaphoreWaitInfo* pWaitInfo, uint64_t timeout,
                                                const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkWaitSemaphores);
}

void ThreadSafety::PreCallRecordSignalSemaphore(VkDevice device, const VkSemaphoreSignalInfo* pSignalInfo) {
    StartReadObjectParentInstance(device, vvl::Func::vkSignalSemaphore);
}

void ThreadSafety::PostCallRecordSignalSemaphore(VkDevice device, const VkSemaphoreSignalInfo* pSignalInfo,
                                                 const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkSignalSemaphore);
}

void ThreadSafety::PreCallRecordGetBufferDeviceAddress(VkDevice device, const VkBufferDeviceAddressInfo* pInfo) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetBufferDeviceAddress);
}

void ThreadSafety::PostCallRecordGetBufferDeviceAddress(VkDevice device, const VkBufferDeviceAddressInfo* pInfo,
                                                        const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetBufferDeviceAddress);
}

void ThreadSafety::PreCallRecordGetBufferOpaqueCaptureAddress(VkDevice device, const VkBufferDeviceAddressInfo* pInfo) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetBufferOpaqueCaptureAddress);
}

void ThreadSafety::PostCallRecordGetBufferOpaqueCaptureAddress(VkDevice device, const VkBufferDeviceAddressInfo* pInfo,
                                                               const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetBufferOpaqueCaptureAddress);
}

void ThreadSafety::PreCallRecordGetDeviceMemoryOpaqueCaptureAddress(VkDevice device,
                                                                    const VkDeviceMemoryOpaqueCaptureAddressInfo* pInfo) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetDeviceMemoryOpaqueCaptureAddress);
}

void ThreadSafety::PostCallRecordGetDeviceMemoryOpaqueCaptureAddress(VkDevice device,
                                                                     const VkDeviceMemoryOpaqueCaptureAddressInfo* pInfo,
                                                                     const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetDeviceMemoryOpaqueCaptureAddress);
}

void ThreadSafety::PreCallRecordCreatePrivateDataSlot(VkDevice device, const VkPrivateDataSlotCreateInfo* pCreateInfo,
                                                      const VkAllocationCallbacks* pAllocator,
                                                      VkPrivateDataSlot* pPrivateDataSlot) {
    StartReadObjectParentInstance(device, vvl::Func::vkCreatePrivateDataSlot);
}

void ThreadSafety::PostCallRecordCreatePrivateDataSlot(VkDevice device, const VkPrivateDataSlotCreateInfo* pCreateInfo,
                                                       const VkAllocationCallbacks* pAllocator, VkPrivateDataSlot* pPrivateDataSlot,
                                                       const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkCreatePrivateDataSlot);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pPrivateDataSlot);
    }
}

void ThreadSafety::PreCallRecordDestroyPrivateDataSlot(VkDevice device, VkPrivateDataSlot privateDataSlot,
                                                       const VkAllocationCallbacks* pAllocator) {
    StartReadObjectParentInstance(device, vvl::Func::vkDestroyPrivateDataSlot);
    StartWriteObject(privateDataSlot, vvl::Func::vkDestroyPrivateDataSlot);
    // Host access to privateDataSlot must be externally synchronized
}

void ThreadSafety::PostCallRecordDestroyPrivateDataSlot(VkDevice device, VkPrivateDataSlot privateDataSlot,
                                                        const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkDestroyPrivateDataSlot);
    FinishWriteObject(privateDataSlot, vvl::Func::vkDestroyPrivateDataSlot);
    DestroyObject(privateDataSlot);
    // Host access to privateDataSlot must be externally synchronized
}

void ThreadSafety::PreCallRecordSetPrivateData(VkDevice device, VkObjectType objectType, uint64_t objectHandle,
                                               VkPrivateDataSlot privateDataSlot, uint64_t data) {
    StartReadObjectParentInstance(device, vvl::Func::vkSetPrivateData);
    StartReadObject(privateDataSlot, vvl::Func::vkSetPrivateData);
}

void ThreadSafety::PostCallRecordSetPrivateData(VkDevice device, VkObjectType objectType, uint64_t objectHandle,
                                                VkPrivateDataSlot privateDataSlot, uint64_t data, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkSetPrivateData);
    FinishReadObject(privateDataSlot, vvl::Func::vkSetPrivateData);
}

void ThreadSafety::PreCallRecordGetPrivateData(VkDevice device, VkObjectType objectType, uint64_t objectHandle,
                                               VkPrivateDataSlot privateDataSlot, uint64_t* pData) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetPrivateData);
    StartReadObject(privateDataSlot, vvl::Func::vkGetPrivateData);
}

void ThreadSafety::PostCallRecordGetPrivateData(VkDevice device, VkObjectType objectType, uint64_t objectHandle,
                                                VkPrivateDataSlot privateDataSlot, uint64_t* pData,
                                                const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetPrivateData);
    FinishReadObject(privateDataSlot, vvl::Func::vkGetPrivateData);
}

void ThreadSafety::PreCallRecordCmdSetEvent2(VkCommandBuffer commandBuffer, VkEvent event,
                                             const VkDependencyInfo* pDependencyInfo) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdSetEvent2);
    StartReadObject(event, vvl::Func::vkCmdSetEvent2);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetEvent2(VkCommandBuffer commandBuffer, VkEvent event, const VkDependencyInfo* pDependencyInfo,
                                              const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdSetEvent2);
    FinishReadObject(event, vvl::Func::vkCmdSetEvent2);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdResetEvent2(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags2 stageMask) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdResetEvent2);
    StartReadObject(event, vvl::Func::vkCmdResetEvent2);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdResetEvent2(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags2 stageMask,
                                                const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdResetEvent2);
    FinishReadObject(event, vvl::Func::vkCmdResetEvent2);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdWaitEvents2(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                               const VkDependencyInfo* pDependencyInfos) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdWaitEvents2);

    if (pEvents) {
        for (uint32_t index = 0; index < eventCount; index++) {
            StartReadObject(pEvents[index], vvl::Func::vkCmdWaitEvents2);
        }
    }
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdWaitEvents2(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                                const VkDependencyInfo* pDependencyInfos, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdWaitEvents2);

    if (pEvents) {
        for (uint32_t index = 0; index < eventCount; index++) {
            FinishReadObject(pEvents[index], vvl::Func::vkCmdWaitEvents2);
        }
    }
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdPipelineBarrier2(VkCommandBuffer commandBuffer, const VkDependencyInfo* pDependencyInfo) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdPipelineBarrier2);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdPipelineBarrier2(VkCommandBuffer commandBuffer, const VkDependencyInfo* pDependencyInfo,
                                                     const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdPipelineBarrier2);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdWriteTimestamp2(VkCommandBuffer commandBuffer, VkPipelineStageFlags2 stage,
                                                   VkQueryPool queryPool, uint32_t query) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdWriteTimestamp2);
    StartReadObject(queryPool, vvl::Func::vkCmdWriteTimestamp2);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdWriteTimestamp2(VkCommandBuffer commandBuffer, VkPipelineStageFlags2 stage,
                                                    VkQueryPool queryPool, uint32_t query, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdWriteTimestamp2);
    FinishReadObject(queryPool, vvl::Func::vkCmdWriteTimestamp2);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordQueueSubmit2(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2* pSubmits, VkFence fence) {
    StartWriteObject(queue, vvl::Func::vkQueueSubmit2);
    StartWriteObject(fence, vvl::Func::vkQueueSubmit2);
    // Host access to queue must be externally synchronized
    // Host access to fence must be externally synchronized
}

void ThreadSafety::PostCallRecordQueueSubmit2(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2* pSubmits, VkFence fence,
                                              const RecordObject& record_obj) {
    FinishWriteObject(queue, vvl::Func::vkQueueSubmit2);
    FinishWriteObject(fence, vvl::Func::vkQueueSubmit2);
    // Host access to queue must be externally synchronized
    // Host access to fence must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdCopyBuffer2(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2* pCopyBufferInfo) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdCopyBuffer2);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdCopyBuffer2(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2* pCopyBufferInfo,
                                                const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdCopyBuffer2);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdCopyImage2(VkCommandBuffer commandBuffer, const VkCopyImageInfo2* pCopyImageInfo) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdCopyImage2);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdCopyImage2(VkCommandBuffer commandBuffer, const VkCopyImageInfo2* pCopyImageInfo,
                                               const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdCopyImage2);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdCopyBufferToImage2(VkCommandBuffer commandBuffer,
                                                      const VkCopyBufferToImageInfo2* pCopyBufferToImageInfo) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdCopyBufferToImage2);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdCopyBufferToImage2(VkCommandBuffer commandBuffer,
                                                       const VkCopyBufferToImageInfo2* pCopyBufferToImageInfo,
                                                       const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdCopyBufferToImage2);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdCopyImageToBuffer2(VkCommandBuffer commandBuffer,
                                                      const VkCopyImageToBufferInfo2* pCopyImageToBufferInfo) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdCopyImageToBuffer2);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdCopyImageToBuffer2(VkCommandBuffer commandBuffer,
                                                       const VkCopyImageToBufferInfo2* pCopyImageToBufferInfo,
                                                       const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdCopyImageToBuffer2);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdBlitImage2(VkCommandBuffer commandBuffer, const VkBlitImageInfo2* pBlitImageInfo) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdBlitImage2);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdBlitImage2(VkCommandBuffer commandBuffer, const VkBlitImageInfo2* pBlitImageInfo,
                                               const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdBlitImage2);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdResolveImage2(VkCommandBuffer commandBuffer, const VkResolveImageInfo2* pResolveImageInfo) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdResolveImage2);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdResolveImage2(VkCommandBuffer commandBuffer, const VkResolveImageInfo2* pResolveImageInfo,
                                                  const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdResolveImage2);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdBeginRendering(VkCommandBuffer commandBuffer, const VkRenderingInfo* pRenderingInfo) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdBeginRendering);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdBeginRendering(VkCommandBuffer commandBuffer, const VkRenderingInfo* pRenderingInfo,
                                                   const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdBeginRendering);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdEndRendering(VkCommandBuffer commandBuffer) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdEndRendering);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdEndRendering(VkCommandBuffer commandBuffer, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdEndRendering);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetCullMode(VkCommandBuffer commandBuffer, VkCullModeFlags cullMode) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdSetCullMode);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetCullMode(VkCommandBuffer commandBuffer, VkCullModeFlags cullMode,
                                                const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdSetCullMode);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetFrontFace(VkCommandBuffer commandBuffer, VkFrontFace frontFace) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdSetFrontFace);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetFrontFace(VkCommandBuffer commandBuffer, VkFrontFace frontFace,
                                                 const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdSetFrontFace);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetPrimitiveTopology(VkCommandBuffer commandBuffer, VkPrimitiveTopology primitiveTopology) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdSetPrimitiveTopology);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetPrimitiveTopology(VkCommandBuffer commandBuffer, VkPrimitiveTopology primitiveTopology,
                                                         const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdSetPrimitiveTopology);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetViewportWithCount(VkCommandBuffer commandBuffer, uint32_t viewportCount,
                                                        const VkViewport* pViewports) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdSetViewportWithCount);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetViewportWithCount(VkCommandBuffer commandBuffer, uint32_t viewportCount,
                                                         const VkViewport* pViewports, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdSetViewportWithCount);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetScissorWithCount(VkCommandBuffer commandBuffer, uint32_t scissorCount,
                                                       const VkRect2D* pScissors) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdSetScissorWithCount);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetScissorWithCount(VkCommandBuffer commandBuffer, uint32_t scissorCount,
                                                        const VkRect2D* pScissors, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdSetScissorWithCount);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdBindVertexBuffers2(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount,
                                                      const VkBuffer* pBuffers, const VkDeviceSize* pOffsets,
                                                      const VkDeviceSize* pSizes, const VkDeviceSize* pStrides) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdBindVertexBuffers2);

    if (pBuffers) {
        for (uint32_t index = 0; index < bindingCount; index++) {
            StartReadObject(pBuffers[index], vvl::Func::vkCmdBindVertexBuffers2);
        }
    }
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdBindVertexBuffers2(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount,
                                                       const VkBuffer* pBuffers, const VkDeviceSize* pOffsets,
                                                       const VkDeviceSize* pSizes, const VkDeviceSize* pStrides,
                                                       const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdBindVertexBuffers2);

    if (pBuffers) {
        for (uint32_t index = 0; index < bindingCount; index++) {
            FinishReadObject(pBuffers[index], vvl::Func::vkCmdBindVertexBuffers2);
        }
    }
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetDepthTestEnable(VkCommandBuffer commandBuffer, VkBool32 depthTestEnable) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdSetDepthTestEnable);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetDepthTestEnable(VkCommandBuffer commandBuffer, VkBool32 depthTestEnable,
                                                       const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdSetDepthTestEnable);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetDepthWriteEnable(VkCommandBuffer commandBuffer, VkBool32 depthWriteEnable) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdSetDepthWriteEnable);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetDepthWriteEnable(VkCommandBuffer commandBuffer, VkBool32 depthWriteEnable,
                                                        const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdSetDepthWriteEnable);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetDepthCompareOp(VkCommandBuffer commandBuffer, VkCompareOp depthCompareOp) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdSetDepthCompareOp);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetDepthCompareOp(VkCommandBuffer commandBuffer, VkCompareOp depthCompareOp,
                                                      const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdSetDepthCompareOp);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetDepthBoundsTestEnable(VkCommandBuffer commandBuffer, VkBool32 depthBoundsTestEnable) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdSetDepthBoundsTestEnable);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetDepthBoundsTestEnable(VkCommandBuffer commandBuffer, VkBool32 depthBoundsTestEnable,
                                                             const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdSetDepthBoundsTestEnable);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetStencilTestEnable(VkCommandBuffer commandBuffer, VkBool32 stencilTestEnable) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdSetStencilTestEnable);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetStencilTestEnable(VkCommandBuffer commandBuffer, VkBool32 stencilTestEnable,
                                                         const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdSetStencilTestEnable);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetStencilOp(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, VkStencilOp failOp,
                                                VkStencilOp passOp, VkStencilOp depthFailOp, VkCompareOp compareOp) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdSetStencilOp);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetStencilOp(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, VkStencilOp failOp,
                                                 VkStencilOp passOp, VkStencilOp depthFailOp, VkCompareOp compareOp,
                                                 const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdSetStencilOp);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetRasterizerDiscardEnable(VkCommandBuffer commandBuffer, VkBool32 rasterizerDiscardEnable) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdSetRasterizerDiscardEnable);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetRasterizerDiscardEnable(VkCommandBuffer commandBuffer, VkBool32 rasterizerDiscardEnable,
                                                               const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdSetRasterizerDiscardEnable);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetDepthBiasEnable(VkCommandBuffer commandBuffer, VkBool32 depthBiasEnable) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdSetDepthBiasEnable);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetDepthBiasEnable(VkCommandBuffer commandBuffer, VkBool32 depthBiasEnable,
                                                       const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdSetDepthBiasEnable);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetPrimitiveRestartEnable(VkCommandBuffer commandBuffer, VkBool32 primitiveRestartEnable) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdSetPrimitiveRestartEnable);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetPrimitiveRestartEnable(VkCommandBuffer commandBuffer, VkBool32 primitiveRestartEnable,
                                                              const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdSetPrimitiveRestartEnable);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordGetDeviceBufferMemoryRequirements(VkDevice device, const VkDeviceBufferMemoryRequirements* pInfo,
                                                                  VkMemoryRequirements2* pMemoryRequirements) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetDeviceBufferMemoryRequirements);
}

void ThreadSafety::PostCallRecordGetDeviceBufferMemoryRequirements(VkDevice device, const VkDeviceBufferMemoryRequirements* pInfo,
                                                                   VkMemoryRequirements2* pMemoryRequirements,
                                                                   const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetDeviceBufferMemoryRequirements);
}

void ThreadSafety::PreCallRecordGetDeviceImageMemoryRequirements(VkDevice device, const VkDeviceImageMemoryRequirements* pInfo,
                                                                 VkMemoryRequirements2* pMemoryRequirements) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetDeviceImageMemoryRequirements);
}

void ThreadSafety::PostCallRecordGetDeviceImageMemoryRequirements(VkDevice device, const VkDeviceImageMemoryRequirements* pInfo,
                                                                  VkMemoryRequirements2* pMemoryRequirements,
                                                                  const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetDeviceImageMemoryRequirements);
}

void ThreadSafety::PreCallRecordGetDeviceImageSparseMemoryRequirements(
    VkDevice device, const VkDeviceImageMemoryRequirements* pInfo, uint32_t* pSparseMemoryRequirementCount,
    VkSparseImageMemoryRequirements2* pSparseMemoryRequirements) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetDeviceImageSparseMemoryRequirements);
}

void ThreadSafety::PostCallRecordGetDeviceImageSparseMemoryRequirements(VkDevice device,
                                                                        const VkDeviceImageMemoryRequirements* pInfo,
                                                                        uint32_t* pSparseMemoryRequirementCount,
                                                                        VkSparseImageMemoryRequirements2* pSparseMemoryRequirements,
                                                                        const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetDeviceImageSparseMemoryRequirements);
}

void ThreadSafety::PreCallRecordDestroySurfaceKHR(VkInstance instance, VkSurfaceKHR surface,
                                                  const VkAllocationCallbacks* pAllocator) {
    StartReadObjectParentInstance(instance, vvl::Func::vkDestroySurfaceKHR);
    StartWriteObjectParentInstance(surface, vvl::Func::vkDestroySurfaceKHR);
    // Host access to surface must be externally synchronized
}

void ThreadSafety::PostCallRecordDestroySurfaceKHR(VkInstance instance, VkSurfaceKHR surface,
                                                   const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(instance, vvl::Func::vkDestroySurfaceKHR);
    FinishWriteObjectParentInstance(surface, vvl::Func::vkDestroySurfaceKHR);
    DestroyObjectParentInstance(surface);
    // Host access to surface must be externally synchronized
}

void ThreadSafety::PreCallRecordGetPhysicalDeviceSurfaceSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
                                                                   VkSurfaceKHR surface, VkBool32* pSupported) {
    StartReadObjectParentInstance(surface, vvl::Func::vkGetPhysicalDeviceSurfaceSupportKHR);
}

void ThreadSafety::PostCallRecordGetPhysicalDeviceSurfaceSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
                                                                    VkSurfaceKHR surface, VkBool32* pSupported,
                                                                    const RecordObject& record_obj) {
    FinishReadObjectParentInstance(surface, vvl::Func::vkGetPhysicalDeviceSurfaceSupportKHR);
}

void ThreadSafety::PreCallRecordGetPhysicalDeviceSurfaceCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                        VkSurfaceCapabilitiesKHR* pSurfaceCapabilities) {
    StartReadObjectParentInstance(surface, vvl::Func::vkGetPhysicalDeviceSurfaceCapabilitiesKHR);
}

void ThreadSafety::PostCallRecordGetPhysicalDeviceSurfaceCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                         VkSurfaceCapabilitiesKHR* pSurfaceCapabilities,
                                                                         const RecordObject& record_obj) {
    FinishReadObjectParentInstance(surface, vvl::Func::vkGetPhysicalDeviceSurfaceCapabilitiesKHR);
}

void ThreadSafety::PreCallRecordGetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                   uint32_t* pSurfaceFormatCount,
                                                                   VkSurfaceFormatKHR* pSurfaceFormats) {
    StartReadObjectParentInstance(surface, vvl::Func::vkGetPhysicalDeviceSurfaceFormatsKHR);
}

void ThreadSafety::PostCallRecordGetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                    uint32_t* pSurfaceFormatCount,
                                                                    VkSurfaceFormatKHR* pSurfaceFormats,
                                                                    const RecordObject& record_obj) {
    FinishReadObjectParentInstance(surface, vvl::Func::vkGetPhysicalDeviceSurfaceFormatsKHR);
}

void ThreadSafety::PreCallRecordGetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                        uint32_t* pPresentModeCount,
                                                                        VkPresentModeKHR* pPresentModes) {
    StartReadObjectParentInstance(surface, vvl::Func::vkGetPhysicalDeviceSurfacePresentModesKHR);
}

void ThreadSafety::PostCallRecordGetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                         uint32_t* pPresentModeCount,
                                                                         VkPresentModeKHR* pPresentModes,
                                                                         const RecordObject& record_obj) {
    FinishReadObjectParentInstance(surface, vvl::Func::vkGetPhysicalDeviceSurfacePresentModesKHR);
}

void ThreadSafety::PreCallRecordCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR* pCreateInfo,
                                                   const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain) {
    StartReadObjectParentInstance(device, vvl::Func::vkCreateSwapchainKHR);
    StartWriteObjectParentInstance(pCreateInfo->surface, vvl::Func::vkCreateSwapchainKHR);
    StartWriteObject(pCreateInfo->oldSwapchain, vvl::Func::vkCreateSwapchainKHR);
    // Host access to pCreateInfo->surface,pCreateInfo->oldSwapchain must be externally synchronized
}

void ThreadSafety::PostCallRecordCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR* pCreateInfo,
                                                    const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain,
                                                    const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkCreateSwapchainKHR);
    FinishWriteObjectParentInstance(pCreateInfo->surface, vvl::Func::vkCreateSwapchainKHR);
    FinishWriteObject(pCreateInfo->oldSwapchain, vvl::Func::vkCreateSwapchainKHR);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pSwapchain);
    }
    // Host access to pCreateInfo->surface,pCreateInfo->oldSwapchain must be externally synchronized
}

void ThreadSafety::PreCallRecordAcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout,
                                                    VkSemaphore semaphore, VkFence fence, uint32_t* pImageIndex) {
    StartReadObjectParentInstance(device, vvl::Func::vkAcquireNextImageKHR);
    StartWriteObject(swapchain, vvl::Func::vkAcquireNextImageKHR);
    StartWriteObject(semaphore, vvl::Func::vkAcquireNextImageKHR);
    StartWriteObject(fence, vvl::Func::vkAcquireNextImageKHR);
    // Host access to swapchain must be externally synchronized
    // Host access to semaphore must be externally synchronized
    // Host access to fence must be externally synchronized
}

void ThreadSafety::PostCallRecordAcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout,
                                                     VkSemaphore semaphore, VkFence fence, uint32_t* pImageIndex,
                                                     const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkAcquireNextImageKHR);
    FinishWriteObject(swapchain, vvl::Func::vkAcquireNextImageKHR);
    FinishWriteObject(semaphore, vvl::Func::vkAcquireNextImageKHR);
    FinishWriteObject(fence, vvl::Func::vkAcquireNextImageKHR);
    // Host access to swapchain must be externally synchronized
    // Host access to semaphore must be externally synchronized
    // Host access to fence must be externally synchronized
}

void ThreadSafety::PreCallRecordGetDeviceGroupPresentCapabilitiesKHR(
    VkDevice device, VkDeviceGroupPresentCapabilitiesKHR* pDeviceGroupPresentCapabilities) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetDeviceGroupPresentCapabilitiesKHR);
}

void ThreadSafety::PostCallRecordGetDeviceGroupPresentCapabilitiesKHR(
    VkDevice device, VkDeviceGroupPresentCapabilitiesKHR* pDeviceGroupPresentCapabilities, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetDeviceGroupPresentCapabilitiesKHR);
}

void ThreadSafety::PreCallRecordGetDeviceGroupSurfacePresentModesKHR(VkDevice device, VkSurfaceKHR surface,
                                                                     VkDeviceGroupPresentModeFlagsKHR* pModes) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetDeviceGroupSurfacePresentModesKHR);
    StartWriteObjectParentInstance(surface, vvl::Func::vkGetDeviceGroupSurfacePresentModesKHR);
    // Host access to surface must be externally synchronized
}

void ThreadSafety::PostCallRecordGetDeviceGroupSurfacePresentModesKHR(VkDevice device, VkSurfaceKHR surface,
                                                                      VkDeviceGroupPresentModeFlagsKHR* pModes,
                                                                      const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetDeviceGroupSurfacePresentModesKHR);
    FinishWriteObjectParentInstance(surface, vvl::Func::vkGetDeviceGroupSurfacePresentModesKHR);
    // Host access to surface must be externally synchronized
}

void ThreadSafety::PreCallRecordGetPhysicalDevicePresentRectanglesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                      uint32_t* pRectCount, VkRect2D* pRects) {
    StartWriteObjectParentInstance(surface, vvl::Func::vkGetPhysicalDevicePresentRectanglesKHR);
    // Host access to surface must be externally synchronized
}

void ThreadSafety::PostCallRecordGetPhysicalDevicePresentRectanglesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                       uint32_t* pRectCount, VkRect2D* pRects,
                                                                       const RecordObject& record_obj) {
    FinishWriteObjectParentInstance(surface, vvl::Func::vkGetPhysicalDevicePresentRectanglesKHR);
    // Host access to surface must be externally synchronized
}

void ThreadSafety::PreCallRecordAcquireNextImage2KHR(VkDevice device, const VkAcquireNextImageInfoKHR* pAcquireInfo,
                                                     uint32_t* pImageIndex) {
    StartReadObjectParentInstance(device, vvl::Func::vkAcquireNextImage2KHR);
}

void ThreadSafety::PostCallRecordAcquireNextImage2KHR(VkDevice device, const VkAcquireNextImageInfoKHR* pAcquireInfo,
                                                      uint32_t* pImageIndex, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkAcquireNextImage2KHR);
}

void ThreadSafety::PreCallRecordCreateDisplayModeKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display,
                                                     const VkDisplayModeCreateInfoKHR* pCreateInfo,
                                                     const VkAllocationCallbacks* pAllocator, VkDisplayModeKHR* pMode) {
    StartWriteObjectParentInstance(display, vvl::Func::vkCreateDisplayModeKHR);
    // Host access to display must be externally synchronized
}

void ThreadSafety::PostCallRecordCreateDisplayModeKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display,
                                                      const VkDisplayModeCreateInfoKHR* pCreateInfo,
                                                      const VkAllocationCallbacks* pAllocator, VkDisplayModeKHR* pMode,
                                                      const RecordObject& record_obj) {
    FinishWriteObjectParentInstance(display, vvl::Func::vkCreateDisplayModeKHR);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pMode);
    }
    // Host access to display must be externally synchronized
}

void ThreadSafety::PreCallRecordGetDisplayPlaneCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkDisplayModeKHR mode,
                                                               uint32_t planeIndex, VkDisplayPlaneCapabilitiesKHR* pCapabilities) {
    StartWriteObject(mode, vvl::Func::vkGetDisplayPlaneCapabilitiesKHR);
    // Host access to mode must be externally synchronized
}

void ThreadSafety::PostCallRecordGetDisplayPlaneCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkDisplayModeKHR mode,
                                                                uint32_t planeIndex, VkDisplayPlaneCapabilitiesKHR* pCapabilities,
                                                                const RecordObject& record_obj) {
    FinishWriteObject(mode, vvl::Func::vkGetDisplayPlaneCapabilitiesKHR);
    // Host access to mode must be externally synchronized
}

void ThreadSafety::PreCallRecordCreateDisplayPlaneSurfaceKHR(VkInstance instance, const VkDisplaySurfaceCreateInfoKHR* pCreateInfo,
                                                             const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    StartReadObjectParentInstance(instance, vvl::Func::vkCreateDisplayPlaneSurfaceKHR);
}

void ThreadSafety::PostCallRecordCreateDisplayPlaneSurfaceKHR(VkInstance instance, const VkDisplaySurfaceCreateInfoKHR* pCreateInfo,
                                                              const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                              const RecordObject& record_obj) {
    FinishReadObjectParentInstance(instance, vvl::Func::vkCreateDisplayPlaneSurfaceKHR);
    if (record_obj.result == VK_SUCCESS) {
        CreateObjectParentInstance(*pSurface);
    }
}

void ThreadSafety::PreCallRecordCreateSharedSwapchainsKHR(VkDevice device, uint32_t swapchainCount,
                                                          const VkSwapchainCreateInfoKHR* pCreateInfos,
                                                          const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchains) {
    StartReadObjectParentInstance(device, vvl::Func::vkCreateSharedSwapchainsKHR);
    if (pCreateInfos) {
        for (uint32_t index = 0; index < swapchainCount; index++) {
            StartWriteObjectParentInstance(pCreateInfos[index].surface, vvl::Func::vkCreateSharedSwapchainsKHR);
            StartWriteObject(pCreateInfos[index].oldSwapchain, vvl::Func::vkCreateSharedSwapchainsKHR);
        }
    }

    if (pSwapchains) {
        for (uint32_t index = 0; index < swapchainCount; index++) {
            StartReadObject(pSwapchains[index], vvl::Func::vkCreateSharedSwapchainsKHR);
        }
    }
    // Host access to pCreateInfos[].surface,pCreateInfos[].oldSwapchain must be externally synchronized
}

void ThreadSafety::PostCallRecordCreateSharedSwapchainsKHR(VkDevice device, uint32_t swapchainCount,
                                                           const VkSwapchainCreateInfoKHR* pCreateInfos,
                                                           const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchains,
                                                           const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkCreateSharedSwapchainsKHR);
    if (pCreateInfos) {
        for (uint32_t index = 0; index < swapchainCount; index++) {
            FinishWriteObjectParentInstance(pCreateInfos[index].surface, vvl::Func::vkCreateSharedSwapchainsKHR);
            FinishWriteObject(pCreateInfos[index].oldSwapchain, vvl::Func::vkCreateSharedSwapchainsKHR);
        }
    }
    if (record_obj.result == VK_SUCCESS) {
        if (pSwapchains) {
            for (uint32_t index = 0; index < swapchainCount; index++) {
                CreateObject(pSwapchains[index]);
            }
        }
    }
    // Host access to pCreateInfos[].surface,pCreateInfos[].oldSwapchain must be externally synchronized
}

#ifdef VK_USE_PLATFORM_XLIB_KHR
void ThreadSafety::PreCallRecordCreateXlibSurfaceKHR(VkInstance instance, const VkXlibSurfaceCreateInfoKHR* pCreateInfo,
                                                     const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    StartReadObjectParentInstance(instance, vvl::Func::vkCreateXlibSurfaceKHR);
}

void ThreadSafety::PostCallRecordCreateXlibSurfaceKHR(VkInstance instance, const VkXlibSurfaceCreateInfoKHR* pCreateInfo,
                                                      const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                      const RecordObject& record_obj) {
    FinishReadObjectParentInstance(instance, vvl::Func::vkCreateXlibSurfaceKHR);
    if (record_obj.result == VK_SUCCESS) {
        CreateObjectParentInstance(*pSurface);
    }
}

#endif  // VK_USE_PLATFORM_XLIB_KHR
#ifdef VK_USE_PLATFORM_XCB_KHR
void ThreadSafety::PreCallRecordCreateXcbSurfaceKHR(VkInstance instance, const VkXcbSurfaceCreateInfoKHR* pCreateInfo,
                                                    const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    StartReadObjectParentInstance(instance, vvl::Func::vkCreateXcbSurfaceKHR);
}

void ThreadSafety::PostCallRecordCreateXcbSurfaceKHR(VkInstance instance, const VkXcbSurfaceCreateInfoKHR* pCreateInfo,
                                                     const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                     const RecordObject& record_obj) {
    FinishReadObjectParentInstance(instance, vvl::Func::vkCreateXcbSurfaceKHR);
    if (record_obj.result == VK_SUCCESS) {
        CreateObjectParentInstance(*pSurface);
    }
}

#endif  // VK_USE_PLATFORM_XCB_KHR
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
void ThreadSafety::PreCallRecordCreateWaylandSurfaceKHR(VkInstance instance, const VkWaylandSurfaceCreateInfoKHR* pCreateInfo,
                                                        const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    StartReadObjectParentInstance(instance, vvl::Func::vkCreateWaylandSurfaceKHR);
}

void ThreadSafety::PostCallRecordCreateWaylandSurfaceKHR(VkInstance instance, const VkWaylandSurfaceCreateInfoKHR* pCreateInfo,
                                                         const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                         const RecordObject& record_obj) {
    FinishReadObjectParentInstance(instance, vvl::Func::vkCreateWaylandSurfaceKHR);
    if (record_obj.result == VK_SUCCESS) {
        CreateObjectParentInstance(*pSurface);
    }
}

#endif  // VK_USE_PLATFORM_WAYLAND_KHR
#ifdef VK_USE_PLATFORM_ANDROID_KHR
void ThreadSafety::PreCallRecordCreateAndroidSurfaceKHR(VkInstance instance, const VkAndroidSurfaceCreateInfoKHR* pCreateInfo,
                                                        const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    StartReadObjectParentInstance(instance, vvl::Func::vkCreateAndroidSurfaceKHR);
}

void ThreadSafety::PostCallRecordCreateAndroidSurfaceKHR(VkInstance instance, const VkAndroidSurfaceCreateInfoKHR* pCreateInfo,
                                                         const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                         const RecordObject& record_obj) {
    FinishReadObjectParentInstance(instance, vvl::Func::vkCreateAndroidSurfaceKHR);
    if (record_obj.result == VK_SUCCESS) {
        CreateObjectParentInstance(*pSurface);
    }
}

#endif  // VK_USE_PLATFORM_ANDROID_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
void ThreadSafety::PreCallRecordCreateWin32SurfaceKHR(VkInstance instance, const VkWin32SurfaceCreateInfoKHR* pCreateInfo,
                                                      const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    StartReadObjectParentInstance(instance, vvl::Func::vkCreateWin32SurfaceKHR);
}

void ThreadSafety::PostCallRecordCreateWin32SurfaceKHR(VkInstance instance, const VkWin32SurfaceCreateInfoKHR* pCreateInfo,
                                                       const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                       const RecordObject& record_obj) {
    FinishReadObjectParentInstance(instance, vvl::Func::vkCreateWin32SurfaceKHR);
    if (record_obj.result == VK_SUCCESS) {
        CreateObjectParentInstance(*pSurface);
    }
}

#endif  // VK_USE_PLATFORM_WIN32_KHR
void ThreadSafety::PreCallRecordCreateVideoSessionKHR(VkDevice device, const VkVideoSessionCreateInfoKHR* pCreateInfo,
                                                      const VkAllocationCallbacks* pAllocator, VkVideoSessionKHR* pVideoSession) {
    StartReadObjectParentInstance(device, vvl::Func::vkCreateVideoSessionKHR);
}

void ThreadSafety::PostCallRecordCreateVideoSessionKHR(VkDevice device, const VkVideoSessionCreateInfoKHR* pCreateInfo,
                                                       const VkAllocationCallbacks* pAllocator, VkVideoSessionKHR* pVideoSession,
                                                       const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkCreateVideoSessionKHR);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pVideoSession);
    }
}

void ThreadSafety::PreCallRecordDestroyVideoSessionKHR(VkDevice device, VkVideoSessionKHR videoSession,
                                                       const VkAllocationCallbacks* pAllocator) {
    StartReadObjectParentInstance(device, vvl::Func::vkDestroyVideoSessionKHR);
    StartWriteObject(videoSession, vvl::Func::vkDestroyVideoSessionKHR);
    // Host access to videoSession must be externally synchronized
}

void ThreadSafety::PostCallRecordDestroyVideoSessionKHR(VkDevice device, VkVideoSessionKHR videoSession,
                                                        const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkDestroyVideoSessionKHR);
    FinishWriteObject(videoSession, vvl::Func::vkDestroyVideoSessionKHR);
    DestroyObject(videoSession);
    // Host access to videoSession must be externally synchronized
}

void ThreadSafety::PreCallRecordGetVideoSessionMemoryRequirementsKHR(VkDevice device, VkVideoSessionKHR videoSession,
                                                                     uint32_t* pMemoryRequirementsCount,
                                                                     VkVideoSessionMemoryRequirementsKHR* pMemoryRequirements) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetVideoSessionMemoryRequirementsKHR);
    StartReadObject(videoSession, vvl::Func::vkGetVideoSessionMemoryRequirementsKHR);
}

void ThreadSafety::PostCallRecordGetVideoSessionMemoryRequirementsKHR(VkDevice device, VkVideoSessionKHR videoSession,
                                                                      uint32_t* pMemoryRequirementsCount,
                                                                      VkVideoSessionMemoryRequirementsKHR* pMemoryRequirements,
                                                                      const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetVideoSessionMemoryRequirementsKHR);
    FinishReadObject(videoSession, vvl::Func::vkGetVideoSessionMemoryRequirementsKHR);
}

void ThreadSafety::PreCallRecordBindVideoSessionMemoryKHR(VkDevice device, VkVideoSessionKHR videoSession,
                                                          uint32_t bindSessionMemoryInfoCount,
                                                          const VkBindVideoSessionMemoryInfoKHR* pBindSessionMemoryInfos) {
    StartReadObjectParentInstance(device, vvl::Func::vkBindVideoSessionMemoryKHR);
    StartWriteObject(videoSession, vvl::Func::vkBindVideoSessionMemoryKHR);
    // Host access to videoSession must be externally synchronized
}

void ThreadSafety::PostCallRecordBindVideoSessionMemoryKHR(VkDevice device, VkVideoSessionKHR videoSession,
                                                           uint32_t bindSessionMemoryInfoCount,
                                                           const VkBindVideoSessionMemoryInfoKHR* pBindSessionMemoryInfos,
                                                           const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkBindVideoSessionMemoryKHR);
    FinishWriteObject(videoSession, vvl::Func::vkBindVideoSessionMemoryKHR);
    // Host access to videoSession must be externally synchronized
}

void ThreadSafety::PreCallRecordCreateVideoSessionParametersKHR(VkDevice device,
                                                                const VkVideoSessionParametersCreateInfoKHR* pCreateInfo,
                                                                const VkAllocationCallbacks* pAllocator,
                                                                VkVideoSessionParametersKHR* pVideoSessionParameters) {
    StartReadObjectParentInstance(device, vvl::Func::vkCreateVideoSessionParametersKHR);
}

void ThreadSafety::PostCallRecordCreateVideoSessionParametersKHR(VkDevice device,
                                                                 const VkVideoSessionParametersCreateInfoKHR* pCreateInfo,
                                                                 const VkAllocationCallbacks* pAllocator,
                                                                 VkVideoSessionParametersKHR* pVideoSessionParameters,
                                                                 const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkCreateVideoSessionParametersKHR);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pVideoSessionParameters);
    }
}

void ThreadSafety::PreCallRecordUpdateVideoSessionParametersKHR(VkDevice device, VkVideoSessionParametersKHR videoSessionParameters,
                                                                const VkVideoSessionParametersUpdateInfoKHR* pUpdateInfo) {
    StartReadObjectParentInstance(device, vvl::Func::vkUpdateVideoSessionParametersKHR);
    StartReadObject(videoSessionParameters, vvl::Func::vkUpdateVideoSessionParametersKHR);
}

void ThreadSafety::PostCallRecordUpdateVideoSessionParametersKHR(VkDevice device,
                                                                 VkVideoSessionParametersKHR videoSessionParameters,
                                                                 const VkVideoSessionParametersUpdateInfoKHR* pUpdateInfo,
                                                                 const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkUpdateVideoSessionParametersKHR);
    FinishReadObject(videoSessionParameters, vvl::Func::vkUpdateVideoSessionParametersKHR);
}

void ThreadSafety::PreCallRecordDestroyVideoSessionParametersKHR(VkDevice device,
                                                                 VkVideoSessionParametersKHR videoSessionParameters,
                                                                 const VkAllocationCallbacks* pAllocator) {
    StartReadObjectParentInstance(device, vvl::Func::vkDestroyVideoSessionParametersKHR);
    StartWriteObject(videoSessionParameters, vvl::Func::vkDestroyVideoSessionParametersKHR);
    // Host access to videoSessionParameters must be externally synchronized
}

void ThreadSafety::PostCallRecordDestroyVideoSessionParametersKHR(VkDevice device,
                                                                  VkVideoSessionParametersKHR videoSessionParameters,
                                                                  const VkAllocationCallbacks* pAllocator,
                                                                  const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkDestroyVideoSessionParametersKHR);
    FinishWriteObject(videoSessionParameters, vvl::Func::vkDestroyVideoSessionParametersKHR);
    DestroyObject(videoSessionParameters);
    // Host access to videoSessionParameters must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdBeginVideoCodingKHR(VkCommandBuffer commandBuffer, const VkVideoBeginCodingInfoKHR* pBeginInfo) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdBeginVideoCodingKHR);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdBeginVideoCodingKHR(VkCommandBuffer commandBuffer, const VkVideoBeginCodingInfoKHR* pBeginInfo,
                                                        const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdBeginVideoCodingKHR);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdEndVideoCodingKHR(VkCommandBuffer commandBuffer, const VkVideoEndCodingInfoKHR* pEndCodingInfo) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdEndVideoCodingKHR);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdEndVideoCodingKHR(VkCommandBuffer commandBuffer, const VkVideoEndCodingInfoKHR* pEndCodingInfo,
                                                      const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdEndVideoCodingKHR);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdControlVideoCodingKHR(VkCommandBuffer commandBuffer,
                                                         const VkVideoCodingControlInfoKHR* pCodingControlInfo) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdControlVideoCodingKHR);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdControlVideoCodingKHR(VkCommandBuffer commandBuffer,
                                                          const VkVideoCodingControlInfoKHR* pCodingControlInfo,
                                                          const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdControlVideoCodingKHR);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdDecodeVideoKHR(VkCommandBuffer commandBuffer, const VkVideoDecodeInfoKHR* pDecodeInfo) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdDecodeVideoKHR);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdDecodeVideoKHR(VkCommandBuffer commandBuffer, const VkVideoDecodeInfoKHR* pDecodeInfo,
                                                   const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdDecodeVideoKHR);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdBeginRenderingKHR(VkCommandBuffer commandBuffer, const VkRenderingInfo* pRenderingInfo) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdBeginRenderingKHR);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdBeginRenderingKHR(VkCommandBuffer commandBuffer, const VkRenderingInfo* pRenderingInfo,
                                                      const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdBeginRenderingKHR);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdEndRenderingKHR(VkCommandBuffer commandBuffer) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdEndRenderingKHR);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdEndRenderingKHR(VkCommandBuffer commandBuffer, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdEndRenderingKHR);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordGetDeviceGroupPeerMemoryFeaturesKHR(VkDevice device, uint32_t heapIndex, uint32_t localDeviceIndex,
                                                                    uint32_t remoteDeviceIndex,
                                                                    VkPeerMemoryFeatureFlags* pPeerMemoryFeatures) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetDeviceGroupPeerMemoryFeaturesKHR);
}

void ThreadSafety::PostCallRecordGetDeviceGroupPeerMemoryFeaturesKHR(VkDevice device, uint32_t heapIndex, uint32_t localDeviceIndex,
                                                                     uint32_t remoteDeviceIndex,
                                                                     VkPeerMemoryFeatureFlags* pPeerMemoryFeatures,
                                                                     const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetDeviceGroupPeerMemoryFeaturesKHR);
}

void ThreadSafety::PreCallRecordCmdSetDeviceMaskKHR(VkCommandBuffer commandBuffer, uint32_t deviceMask) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdSetDeviceMaskKHR);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetDeviceMaskKHR(VkCommandBuffer commandBuffer, uint32_t deviceMask,
                                                     const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdSetDeviceMaskKHR);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdDispatchBaseKHR(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY,
                                                   uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY,
                                                   uint32_t groupCountZ) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdDispatchBaseKHR);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdDispatchBaseKHR(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY,
                                                    uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY,
                                                    uint32_t groupCountZ, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdDispatchBaseKHR);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordTrimCommandPoolKHR(VkDevice device, VkCommandPool commandPool, VkCommandPoolTrimFlags flags) {
    StartReadObjectParentInstance(device, vvl::Func::vkTrimCommandPoolKHR);
    StartWriteObject(commandPool, vvl::Func::vkTrimCommandPoolKHR);
    // Host access to commandPool must be externally synchronized
}

void ThreadSafety::PostCallRecordTrimCommandPoolKHR(VkDevice device, VkCommandPool commandPool, VkCommandPoolTrimFlags flags,
                                                    const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkTrimCommandPoolKHR);
    FinishWriteObject(commandPool, vvl::Func::vkTrimCommandPoolKHR);
    // Host access to commandPool must be externally synchronized
}

void ThreadSafety::PreCallRecordEnumeratePhysicalDeviceGroupsKHR(VkInstance instance, uint32_t* pPhysicalDeviceGroupCount,
                                                                 VkPhysicalDeviceGroupProperties* pPhysicalDeviceGroupProperties) {
    StartReadObjectParentInstance(instance, vvl::Func::vkEnumeratePhysicalDeviceGroupsKHR);
}

void ThreadSafety::PostCallRecordEnumeratePhysicalDeviceGroupsKHR(VkInstance instance, uint32_t* pPhysicalDeviceGroupCount,
                                                                  VkPhysicalDeviceGroupProperties* pPhysicalDeviceGroupProperties,
                                                                  const RecordObject& record_obj) {
    FinishReadObjectParentInstance(instance, vvl::Func::vkEnumeratePhysicalDeviceGroupsKHR);
}

#ifdef VK_USE_PLATFORM_WIN32_KHR
void ThreadSafety::PreCallRecordGetMemoryWin32HandleKHR(VkDevice device, const VkMemoryGetWin32HandleInfoKHR* pGetWin32HandleInfo,
                                                        HANDLE* pHandle) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetMemoryWin32HandleKHR);
}

void ThreadSafety::PostCallRecordGetMemoryWin32HandleKHR(VkDevice device, const VkMemoryGetWin32HandleInfoKHR* pGetWin32HandleInfo,
                                                         HANDLE* pHandle, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetMemoryWin32HandleKHR);
}

#endif  // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
void ThreadSafety::PreCallRecordGetMemoryWin32HandlePropertiesKHR(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType,
                                                                  HANDLE handle,
                                                                  VkMemoryWin32HandlePropertiesKHR* pMemoryWin32HandleProperties) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetMemoryWin32HandlePropertiesKHR);
}

void ThreadSafety::PostCallRecordGetMemoryWin32HandlePropertiesKHR(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType,
                                                                   HANDLE handle,
                                                                   VkMemoryWin32HandlePropertiesKHR* pMemoryWin32HandleProperties,
                                                                   const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetMemoryWin32HandlePropertiesKHR);
}

#endif  // VK_USE_PLATFORM_WIN32_KHR
void ThreadSafety::PreCallRecordGetMemoryFdKHR(VkDevice device, const VkMemoryGetFdInfoKHR* pGetFdInfo, int* pFd) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetMemoryFdKHR);
}

void ThreadSafety::PostCallRecordGetMemoryFdKHR(VkDevice device, const VkMemoryGetFdInfoKHR* pGetFdInfo, int* pFd,
                                                const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetMemoryFdKHR);
}

void ThreadSafety::PreCallRecordGetMemoryFdPropertiesKHR(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType, int fd,
                                                         VkMemoryFdPropertiesKHR* pMemoryFdProperties) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetMemoryFdPropertiesKHR);
}

void ThreadSafety::PostCallRecordGetMemoryFdPropertiesKHR(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType, int fd,
                                                          VkMemoryFdPropertiesKHR* pMemoryFdProperties,
                                                          const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetMemoryFdPropertiesKHR);
}

#ifdef VK_USE_PLATFORM_WIN32_KHR
void ThreadSafety::PreCallRecordImportSemaphoreWin32HandleKHR(
    VkDevice device, const VkImportSemaphoreWin32HandleInfoKHR* pImportSemaphoreWin32HandleInfo) {
    StartReadObjectParentInstance(device, vvl::Func::vkImportSemaphoreWin32HandleKHR);
}

void ThreadSafety::PostCallRecordImportSemaphoreWin32HandleKHR(
    VkDevice device, const VkImportSemaphoreWin32HandleInfoKHR* pImportSemaphoreWin32HandleInfo, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkImportSemaphoreWin32HandleKHR);
}

#endif  // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
void ThreadSafety::PreCallRecordGetSemaphoreWin32HandleKHR(VkDevice device,
                                                           const VkSemaphoreGetWin32HandleInfoKHR* pGetWin32HandleInfo,
                                                           HANDLE* pHandle) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetSemaphoreWin32HandleKHR);
}

void ThreadSafety::PostCallRecordGetSemaphoreWin32HandleKHR(VkDevice device,
                                                            const VkSemaphoreGetWin32HandleInfoKHR* pGetWin32HandleInfo,
                                                            HANDLE* pHandle, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetSemaphoreWin32HandleKHR);
}

#endif  // VK_USE_PLATFORM_WIN32_KHR
void ThreadSafety::PreCallRecordImportSemaphoreFdKHR(VkDevice device, const VkImportSemaphoreFdInfoKHR* pImportSemaphoreFdInfo) {
    StartReadObjectParentInstance(device, vvl::Func::vkImportSemaphoreFdKHR);
}

void ThreadSafety::PostCallRecordImportSemaphoreFdKHR(VkDevice device, const VkImportSemaphoreFdInfoKHR* pImportSemaphoreFdInfo,
                                                      const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkImportSemaphoreFdKHR);
}

void ThreadSafety::PreCallRecordGetSemaphoreFdKHR(VkDevice device, const VkSemaphoreGetFdInfoKHR* pGetFdInfo, int* pFd) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetSemaphoreFdKHR);
}

void ThreadSafety::PostCallRecordGetSemaphoreFdKHR(VkDevice device, const VkSemaphoreGetFdInfoKHR* pGetFdInfo, int* pFd,
                                                   const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetSemaphoreFdKHR);
}

void ThreadSafety::PreCallRecordCmdPushDescriptorSetKHR(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                        VkPipelineLayout layout, uint32_t set, uint32_t descriptorWriteCount,
                                                        const VkWriteDescriptorSet* pDescriptorWrites) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdPushDescriptorSetKHR);
    StartReadObject(layout, vvl::Func::vkCmdPushDescriptorSetKHR);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdPushDescriptorSetKHR(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                         VkPipelineLayout layout, uint32_t set, uint32_t descriptorWriteCount,
                                                         const VkWriteDescriptorSet* pDescriptorWrites,
                                                         const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdPushDescriptorSetKHR);
    FinishReadObject(layout, vvl::Func::vkCmdPushDescriptorSetKHR);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdPushDescriptorSetWithTemplateKHR(VkCommandBuffer commandBuffer,
                                                                    VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                                    VkPipelineLayout layout, uint32_t set, const void* pData) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdPushDescriptorSetWithTemplateKHR);
    StartReadObject(descriptorUpdateTemplate, vvl::Func::vkCmdPushDescriptorSetWithTemplateKHR);
    StartReadObject(layout, vvl::Func::vkCmdPushDescriptorSetWithTemplateKHR);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdPushDescriptorSetWithTemplateKHR(VkCommandBuffer commandBuffer,
                                                                     VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                                     VkPipelineLayout layout, uint32_t set, const void* pData,
                                                                     const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdPushDescriptorSetWithTemplateKHR);
    FinishReadObject(descriptorUpdateTemplate, vvl::Func::vkCmdPushDescriptorSetWithTemplateKHR);
    FinishReadObject(layout, vvl::Func::vkCmdPushDescriptorSetWithTemplateKHR);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCreateDescriptorUpdateTemplateKHR(VkDevice device,
                                                                  const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo,
                                                                  const VkAllocationCallbacks* pAllocator,
                                                                  VkDescriptorUpdateTemplate* pDescriptorUpdateTemplate) {
    StartReadObjectParentInstance(device, vvl::Func::vkCreateDescriptorUpdateTemplateKHR);
}

void ThreadSafety::PostCallRecordCreateDescriptorUpdateTemplateKHR(VkDevice device,
                                                                   const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo,
                                                                   const VkAllocationCallbacks* pAllocator,
                                                                   VkDescriptorUpdateTemplate* pDescriptorUpdateTemplate,
                                                                   const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkCreateDescriptorUpdateTemplateKHR);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pDescriptorUpdateTemplate);
    }
}

void ThreadSafety::PreCallRecordDestroyDescriptorUpdateTemplateKHR(VkDevice device,
                                                                   VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                                   const VkAllocationCallbacks* pAllocator) {
    StartReadObjectParentInstance(device, vvl::Func::vkDestroyDescriptorUpdateTemplateKHR);
    StartWriteObject(descriptorUpdateTemplate, vvl::Func::vkDestroyDescriptorUpdateTemplateKHR);
    // Host access to descriptorUpdateTemplate must be externally synchronized
}

void ThreadSafety::PostCallRecordDestroyDescriptorUpdateTemplateKHR(VkDevice device,
                                                                    VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                                    const VkAllocationCallbacks* pAllocator,
                                                                    const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkDestroyDescriptorUpdateTemplateKHR);
    FinishWriteObject(descriptorUpdateTemplate, vvl::Func::vkDestroyDescriptorUpdateTemplateKHR);
    DestroyObject(descriptorUpdateTemplate);
    // Host access to descriptorUpdateTemplate must be externally synchronized
}

void ThreadSafety::PreCallRecordCreateRenderPass2KHR(VkDevice device, const VkRenderPassCreateInfo2* pCreateInfo,
                                                     const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass) {
    StartReadObjectParentInstance(device, vvl::Func::vkCreateRenderPass2KHR);
}

void ThreadSafety::PostCallRecordCreateRenderPass2KHR(VkDevice device, const VkRenderPassCreateInfo2* pCreateInfo,
                                                      const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass,
                                                      const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkCreateRenderPass2KHR);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pRenderPass);
    }
}

void ThreadSafety::PreCallRecordCmdBeginRenderPass2KHR(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                                       const VkSubpassBeginInfo* pSubpassBeginInfo) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdBeginRenderPass2KHR);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdBeginRenderPass2KHR(VkCommandBuffer commandBuffer,
                                                        const VkRenderPassBeginInfo* pRenderPassBegin,
                                                        const VkSubpassBeginInfo* pSubpassBeginInfo,
                                                        const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdBeginRenderPass2KHR);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdNextSubpass2KHR(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo* pSubpassBeginInfo,
                                                   const VkSubpassEndInfo* pSubpassEndInfo) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdNextSubpass2KHR);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdNextSubpass2KHR(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo* pSubpassBeginInfo,
                                                    const VkSubpassEndInfo* pSubpassEndInfo, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdNextSubpass2KHR);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdEndRenderPass2KHR(VkCommandBuffer commandBuffer, const VkSubpassEndInfo* pSubpassEndInfo) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdEndRenderPass2KHR);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdEndRenderPass2KHR(VkCommandBuffer commandBuffer, const VkSubpassEndInfo* pSubpassEndInfo,
                                                      const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdEndRenderPass2KHR);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordGetSwapchainStatusKHR(VkDevice device, VkSwapchainKHR swapchain) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetSwapchainStatusKHR);
    StartWriteObject(swapchain, vvl::Func::vkGetSwapchainStatusKHR);
    // Host access to swapchain must be externally synchronized
}

void ThreadSafety::PostCallRecordGetSwapchainStatusKHR(VkDevice device, VkSwapchainKHR swapchain, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetSwapchainStatusKHR);
    FinishWriteObject(swapchain, vvl::Func::vkGetSwapchainStatusKHR);
    // Host access to swapchain must be externally synchronized
}

#ifdef VK_USE_PLATFORM_WIN32_KHR
void ThreadSafety::PreCallRecordImportFenceWin32HandleKHR(VkDevice device,
                                                          const VkImportFenceWin32HandleInfoKHR* pImportFenceWin32HandleInfo) {
    StartReadObjectParentInstance(device, vvl::Func::vkImportFenceWin32HandleKHR);
}

void ThreadSafety::PostCallRecordImportFenceWin32HandleKHR(VkDevice device,
                                                           const VkImportFenceWin32HandleInfoKHR* pImportFenceWin32HandleInfo,
                                                           const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkImportFenceWin32HandleKHR);
}

#endif  // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
void ThreadSafety::PreCallRecordGetFenceWin32HandleKHR(VkDevice device, const VkFenceGetWin32HandleInfoKHR* pGetWin32HandleInfo,
                                                       HANDLE* pHandle) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetFenceWin32HandleKHR);
}

void ThreadSafety::PostCallRecordGetFenceWin32HandleKHR(VkDevice device, const VkFenceGetWin32HandleInfoKHR* pGetWin32HandleInfo,
                                                        HANDLE* pHandle, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetFenceWin32HandleKHR);
}

#endif  // VK_USE_PLATFORM_WIN32_KHR
void ThreadSafety::PreCallRecordImportFenceFdKHR(VkDevice device, const VkImportFenceFdInfoKHR* pImportFenceFdInfo) {
    StartReadObjectParentInstance(device, vvl::Func::vkImportFenceFdKHR);
}

void ThreadSafety::PostCallRecordImportFenceFdKHR(VkDevice device, const VkImportFenceFdInfoKHR* pImportFenceFdInfo,
                                                  const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkImportFenceFdKHR);
}

void ThreadSafety::PreCallRecordGetFenceFdKHR(VkDevice device, const VkFenceGetFdInfoKHR* pGetFdInfo, int* pFd) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetFenceFdKHR);
}

void ThreadSafety::PostCallRecordGetFenceFdKHR(VkDevice device, const VkFenceGetFdInfoKHR* pGetFdInfo, int* pFd,
                                               const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetFenceFdKHR);
}

void ThreadSafety::PreCallRecordAcquireProfilingLockKHR(VkDevice device, const VkAcquireProfilingLockInfoKHR* pInfo) {
    StartReadObjectParentInstance(device, vvl::Func::vkAcquireProfilingLockKHR);
}

void ThreadSafety::PostCallRecordAcquireProfilingLockKHR(VkDevice device, const VkAcquireProfilingLockInfoKHR* pInfo,
                                                         const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkAcquireProfilingLockKHR);
}

void ThreadSafety::PreCallRecordReleaseProfilingLockKHR(VkDevice device) {
    StartReadObjectParentInstance(device, vvl::Func::vkReleaseProfilingLockKHR);
}

void ThreadSafety::PostCallRecordReleaseProfilingLockKHR(VkDevice device, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkReleaseProfilingLockKHR);
}

void ThreadSafety::PreCallRecordGetImageMemoryRequirements2KHR(VkDevice device, const VkImageMemoryRequirementsInfo2* pInfo,
                                                               VkMemoryRequirements2* pMemoryRequirements) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetImageMemoryRequirements2KHR);
}

void ThreadSafety::PostCallRecordGetImageMemoryRequirements2KHR(VkDevice device, const VkImageMemoryRequirementsInfo2* pInfo,
                                                                VkMemoryRequirements2* pMemoryRequirements,
                                                                const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetImageMemoryRequirements2KHR);
}

void ThreadSafety::PreCallRecordGetBufferMemoryRequirements2KHR(VkDevice device, const VkBufferMemoryRequirementsInfo2* pInfo,
                                                                VkMemoryRequirements2* pMemoryRequirements) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetBufferMemoryRequirements2KHR);
}

void ThreadSafety::PostCallRecordGetBufferMemoryRequirements2KHR(VkDevice device, const VkBufferMemoryRequirementsInfo2* pInfo,
                                                                 VkMemoryRequirements2* pMemoryRequirements,
                                                                 const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetBufferMemoryRequirements2KHR);
}

void ThreadSafety::PreCallRecordGetImageSparseMemoryRequirements2KHR(VkDevice device,
                                                                     const VkImageSparseMemoryRequirementsInfo2* pInfo,
                                                                     uint32_t* pSparseMemoryRequirementCount,
                                                                     VkSparseImageMemoryRequirements2* pSparseMemoryRequirements) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetImageSparseMemoryRequirements2KHR);
}

void ThreadSafety::PostCallRecordGetImageSparseMemoryRequirements2KHR(VkDevice device,
                                                                      const VkImageSparseMemoryRequirementsInfo2* pInfo,
                                                                      uint32_t* pSparseMemoryRequirementCount,
                                                                      VkSparseImageMemoryRequirements2* pSparseMemoryRequirements,
                                                                      const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetImageSparseMemoryRequirements2KHR);
}

void ThreadSafety::PreCallRecordCreateSamplerYcbcrConversionKHR(VkDevice device,
                                                                const VkSamplerYcbcrConversionCreateInfo* pCreateInfo,
                                                                const VkAllocationCallbacks* pAllocator,
                                                                VkSamplerYcbcrConversion* pYcbcrConversion) {
    StartReadObjectParentInstance(device, vvl::Func::vkCreateSamplerYcbcrConversionKHR);
}

void ThreadSafety::PostCallRecordCreateSamplerYcbcrConversionKHR(VkDevice device,
                                                                 const VkSamplerYcbcrConversionCreateInfo* pCreateInfo,
                                                                 const VkAllocationCallbacks* pAllocator,
                                                                 VkSamplerYcbcrConversion* pYcbcrConversion,
                                                                 const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkCreateSamplerYcbcrConversionKHR);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pYcbcrConversion);
    }
}

void ThreadSafety::PreCallRecordDestroySamplerYcbcrConversionKHR(VkDevice device, VkSamplerYcbcrConversion ycbcrConversion,
                                                                 const VkAllocationCallbacks* pAllocator) {
    StartReadObjectParentInstance(device, vvl::Func::vkDestroySamplerYcbcrConversionKHR);
    StartWriteObject(ycbcrConversion, vvl::Func::vkDestroySamplerYcbcrConversionKHR);
    // Host access to ycbcrConversion must be externally synchronized
}

void ThreadSafety::PostCallRecordDestroySamplerYcbcrConversionKHR(VkDevice device, VkSamplerYcbcrConversion ycbcrConversion,
                                                                  const VkAllocationCallbacks* pAllocator,
                                                                  const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkDestroySamplerYcbcrConversionKHR);
    FinishWriteObject(ycbcrConversion, vvl::Func::vkDestroySamplerYcbcrConversionKHR);
    DestroyObject(ycbcrConversion);
    // Host access to ycbcrConversion must be externally synchronized
}

void ThreadSafety::PreCallRecordBindBufferMemory2KHR(VkDevice device, uint32_t bindInfoCount,
                                                     const VkBindBufferMemoryInfo* pBindInfos) {
    StartReadObjectParentInstance(device, vvl::Func::vkBindBufferMemory2KHR);
}

void ThreadSafety::PostCallRecordBindBufferMemory2KHR(VkDevice device, uint32_t bindInfoCount,
                                                      const VkBindBufferMemoryInfo* pBindInfos, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkBindBufferMemory2KHR);
}

void ThreadSafety::PreCallRecordBindImageMemory2KHR(VkDevice device, uint32_t bindInfoCount,
                                                    const VkBindImageMemoryInfo* pBindInfos) {
    StartReadObjectParentInstance(device, vvl::Func::vkBindImageMemory2KHR);
}

void ThreadSafety::PostCallRecordBindImageMemory2KHR(VkDevice device, uint32_t bindInfoCount,
                                                     const VkBindImageMemoryInfo* pBindInfos, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkBindImageMemory2KHR);
}

void ThreadSafety::PreCallRecordGetDescriptorSetLayoutSupportKHR(VkDevice device,
                                                                 const VkDescriptorSetLayoutCreateInfo* pCreateInfo,
                                                                 VkDescriptorSetLayoutSupport* pSupport) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetDescriptorSetLayoutSupportKHR);
}

void ThreadSafety::PostCallRecordGetDescriptorSetLayoutSupportKHR(VkDevice device,
                                                                  const VkDescriptorSetLayoutCreateInfo* pCreateInfo,
                                                                  VkDescriptorSetLayoutSupport* pSupport,
                                                                  const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetDescriptorSetLayoutSupportKHR);
}

void ThreadSafety::PreCallRecordCmdDrawIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                        VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                        uint32_t stride) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdDrawIndirectCountKHR);
    StartReadObject(buffer, vvl::Func::vkCmdDrawIndirectCountKHR);
    StartReadObject(countBuffer, vvl::Func::vkCmdDrawIndirectCountKHR);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdDrawIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                         VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                         uint32_t maxDrawCount, uint32_t stride, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdDrawIndirectCountKHR);
    FinishReadObject(buffer, vvl::Func::vkCmdDrawIndirectCountKHR);
    FinishReadObject(countBuffer, vvl::Func::vkCmdDrawIndirectCountKHR);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                               VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                               uint32_t maxDrawCount, uint32_t stride) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdDrawIndexedIndirectCountKHR);
    StartReadObject(buffer, vvl::Func::vkCmdDrawIndexedIndirectCountKHR);
    StartReadObject(countBuffer, vvl::Func::vkCmdDrawIndexedIndirectCountKHR);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                                VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                                uint32_t maxDrawCount, uint32_t stride,
                                                                const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdDrawIndexedIndirectCountKHR);
    FinishReadObject(buffer, vvl::Func::vkCmdDrawIndexedIndirectCountKHR);
    FinishReadObject(countBuffer, vvl::Func::vkCmdDrawIndexedIndirectCountKHR);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordGetSemaphoreCounterValueKHR(VkDevice device, VkSemaphore semaphore, uint64_t* pValue) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetSemaphoreCounterValueKHR);
    StartReadObject(semaphore, vvl::Func::vkGetSemaphoreCounterValueKHR);
}

void ThreadSafety::PostCallRecordGetSemaphoreCounterValueKHR(VkDevice device, VkSemaphore semaphore, uint64_t* pValue,
                                                             const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetSemaphoreCounterValueKHR);
    FinishReadObject(semaphore, vvl::Func::vkGetSemaphoreCounterValueKHR);
}

void ThreadSafety::PreCallRecordWaitSemaphoresKHR(VkDevice device, const VkSemaphoreWaitInfo* pWaitInfo, uint64_t timeout) {
    StartReadObjectParentInstance(device, vvl::Func::vkWaitSemaphoresKHR);
}

void ThreadSafety::PostCallRecordWaitSemaphoresKHR(VkDevice device, const VkSemaphoreWaitInfo* pWaitInfo, uint64_t timeout,
                                                   const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkWaitSemaphoresKHR);
}

void ThreadSafety::PreCallRecordSignalSemaphoreKHR(VkDevice device, const VkSemaphoreSignalInfo* pSignalInfo) {
    StartReadObjectParentInstance(device, vvl::Func::vkSignalSemaphoreKHR);
}

void ThreadSafety::PostCallRecordSignalSemaphoreKHR(VkDevice device, const VkSemaphoreSignalInfo* pSignalInfo,
                                                    const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkSignalSemaphoreKHR);
}

void ThreadSafety::PreCallRecordCmdSetFragmentShadingRateKHR(VkCommandBuffer commandBuffer, const VkExtent2D* pFragmentSize,
                                                             const VkFragmentShadingRateCombinerOpKHR combinerOps[2]) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdSetFragmentShadingRateKHR);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetFragmentShadingRateKHR(VkCommandBuffer commandBuffer, const VkExtent2D* pFragmentSize,
                                                              const VkFragmentShadingRateCombinerOpKHR combinerOps[2],
                                                              const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdSetFragmentShadingRateKHR);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordWaitForPresentKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t presentId, uint64_t timeout) {
    StartReadObjectParentInstance(device, vvl::Func::vkWaitForPresentKHR);
    StartWriteObject(swapchain, vvl::Func::vkWaitForPresentKHR);
    // Host access to swapchain must be externally synchronized
}

void ThreadSafety::PostCallRecordWaitForPresentKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t presentId, uint64_t timeout,
                                                   const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkWaitForPresentKHR);
    FinishWriteObject(swapchain, vvl::Func::vkWaitForPresentKHR);
    // Host access to swapchain must be externally synchronized
}

void ThreadSafety::PreCallRecordGetBufferDeviceAddressKHR(VkDevice device, const VkBufferDeviceAddressInfo* pInfo) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetBufferDeviceAddressKHR);
}

void ThreadSafety::PostCallRecordGetBufferDeviceAddressKHR(VkDevice device, const VkBufferDeviceAddressInfo* pInfo,
                                                           const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetBufferDeviceAddressKHR);
}

void ThreadSafety::PreCallRecordGetBufferOpaqueCaptureAddressKHR(VkDevice device, const VkBufferDeviceAddressInfo* pInfo) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetBufferOpaqueCaptureAddressKHR);
}

void ThreadSafety::PostCallRecordGetBufferOpaqueCaptureAddressKHR(VkDevice device, const VkBufferDeviceAddressInfo* pInfo,
                                                                  const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetBufferOpaqueCaptureAddressKHR);
}

void ThreadSafety::PreCallRecordGetDeviceMemoryOpaqueCaptureAddressKHR(VkDevice device,
                                                                       const VkDeviceMemoryOpaqueCaptureAddressInfo* pInfo) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetDeviceMemoryOpaqueCaptureAddressKHR);
}

void ThreadSafety::PostCallRecordGetDeviceMemoryOpaqueCaptureAddressKHR(VkDevice device,
                                                                        const VkDeviceMemoryOpaqueCaptureAddressInfo* pInfo,
                                                                        const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetDeviceMemoryOpaqueCaptureAddressKHR);
}

void ThreadSafety::PreCallRecordCreateDeferredOperationKHR(VkDevice device, const VkAllocationCallbacks* pAllocator,
                                                           VkDeferredOperationKHR* pDeferredOperation) {
    StartReadObjectParentInstance(device, vvl::Func::vkCreateDeferredOperationKHR);
}

void ThreadSafety::PostCallRecordCreateDeferredOperationKHR(VkDevice device, const VkAllocationCallbacks* pAllocator,
                                                            VkDeferredOperationKHR* pDeferredOperation,
                                                            const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkCreateDeferredOperationKHR);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pDeferredOperation);
    }
}

void ThreadSafety::PreCallRecordDestroyDeferredOperationKHR(VkDevice device, VkDeferredOperationKHR operation,
                                                            const VkAllocationCallbacks* pAllocator) {
    StartReadObjectParentInstance(device, vvl::Func::vkDestroyDeferredOperationKHR);
    StartWriteObject(operation, vvl::Func::vkDestroyDeferredOperationKHR);
    // Host access to operation must be externally synchronized
}

void ThreadSafety::PostCallRecordDestroyDeferredOperationKHR(VkDevice device, VkDeferredOperationKHR operation,
                                                             const VkAllocationCallbacks* pAllocator,
                                                             const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkDestroyDeferredOperationKHR);
    FinishWriteObject(operation, vvl::Func::vkDestroyDeferredOperationKHR);
    DestroyObject(operation);
    // Host access to operation must be externally synchronized
}

void ThreadSafety::PreCallRecordGetDeferredOperationMaxConcurrencyKHR(VkDevice device, VkDeferredOperationKHR operation) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetDeferredOperationMaxConcurrencyKHR);
    StartReadObject(operation, vvl::Func::vkGetDeferredOperationMaxConcurrencyKHR);
}

void ThreadSafety::PostCallRecordGetDeferredOperationMaxConcurrencyKHR(VkDevice device, VkDeferredOperationKHR operation,
                                                                       const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetDeferredOperationMaxConcurrencyKHR);
    FinishReadObject(operation, vvl::Func::vkGetDeferredOperationMaxConcurrencyKHR);
}

void ThreadSafety::PreCallRecordGetDeferredOperationResultKHR(VkDevice device, VkDeferredOperationKHR operation) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetDeferredOperationResultKHR);
    StartReadObject(operation, vvl::Func::vkGetDeferredOperationResultKHR);
}

void ThreadSafety::PostCallRecordGetDeferredOperationResultKHR(VkDevice device, VkDeferredOperationKHR operation,
                                                               const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetDeferredOperationResultKHR);
    FinishReadObject(operation, vvl::Func::vkGetDeferredOperationResultKHR);
}

void ThreadSafety::PreCallRecordDeferredOperationJoinKHR(VkDevice device, VkDeferredOperationKHR operation) {
    StartReadObjectParentInstance(device, vvl::Func::vkDeferredOperationJoinKHR);
    StartReadObject(operation, vvl::Func::vkDeferredOperationJoinKHR);
}

void ThreadSafety::PostCallRecordDeferredOperationJoinKHR(VkDevice device, VkDeferredOperationKHR operation,
                                                          const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkDeferredOperationJoinKHR);
    FinishReadObject(operation, vvl::Func::vkDeferredOperationJoinKHR);
}

void ThreadSafety::PreCallRecordGetPipelineExecutablePropertiesKHR(VkDevice device, const VkPipelineInfoKHR* pPipelineInfo,
                                                                   uint32_t* pExecutableCount,
                                                                   VkPipelineExecutablePropertiesKHR* pProperties) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetPipelineExecutablePropertiesKHR);
}

void ThreadSafety::PostCallRecordGetPipelineExecutablePropertiesKHR(VkDevice device, const VkPipelineInfoKHR* pPipelineInfo,
                                                                    uint32_t* pExecutableCount,
                                                                    VkPipelineExecutablePropertiesKHR* pProperties,
                                                                    const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetPipelineExecutablePropertiesKHR);
}

void ThreadSafety::PreCallRecordGetPipelineExecutableStatisticsKHR(VkDevice device,
                                                                   const VkPipelineExecutableInfoKHR* pExecutableInfo,
                                                                   uint32_t* pStatisticCount,
                                                                   VkPipelineExecutableStatisticKHR* pStatistics) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetPipelineExecutableStatisticsKHR);
}

void ThreadSafety::PostCallRecordGetPipelineExecutableStatisticsKHR(VkDevice device,
                                                                    const VkPipelineExecutableInfoKHR* pExecutableInfo,
                                                                    uint32_t* pStatisticCount,
                                                                    VkPipelineExecutableStatisticKHR* pStatistics,
                                                                    const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetPipelineExecutableStatisticsKHR);
}

void ThreadSafety::PreCallRecordGetPipelineExecutableInternalRepresentationsKHR(
    VkDevice device, const VkPipelineExecutableInfoKHR* pExecutableInfo, uint32_t* pInternalRepresentationCount,
    VkPipelineExecutableInternalRepresentationKHR* pInternalRepresentations) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetPipelineExecutableInternalRepresentationsKHR);
}

void ThreadSafety::PostCallRecordGetPipelineExecutableInternalRepresentationsKHR(
    VkDevice device, const VkPipelineExecutableInfoKHR* pExecutableInfo, uint32_t* pInternalRepresentationCount,
    VkPipelineExecutableInternalRepresentationKHR* pInternalRepresentations, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetPipelineExecutableInternalRepresentationsKHR);
}

void ThreadSafety::PreCallRecordMapMemory2KHR(VkDevice device, const VkMemoryMapInfoKHR* pMemoryMapInfo, void** ppData) {
    StartReadObjectParentInstance(device, vvl::Func::vkMapMemory2KHR);
}

void ThreadSafety::PostCallRecordMapMemory2KHR(VkDevice device, const VkMemoryMapInfoKHR* pMemoryMapInfo, void** ppData,
                                               const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkMapMemory2KHR);
}

void ThreadSafety::PreCallRecordUnmapMemory2KHR(VkDevice device, const VkMemoryUnmapInfoKHR* pMemoryUnmapInfo) {
    StartReadObjectParentInstance(device, vvl::Func::vkUnmapMemory2KHR);
}

void ThreadSafety::PostCallRecordUnmapMemory2KHR(VkDevice device, const VkMemoryUnmapInfoKHR* pMemoryUnmapInfo,
                                                 const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkUnmapMemory2KHR);
}

#ifdef VK_ENABLE_BETA_EXTENSIONS
void ThreadSafety::PreCallRecordGetEncodedVideoSessionParametersKHR(
    VkDevice device, const VkVideoEncodeSessionParametersGetInfoKHR* pVideoSessionParametersInfo,
    VkVideoEncodeSessionParametersFeedbackInfoKHR* pFeedbackInfo, size_t* pDataSize, void* pData) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetEncodedVideoSessionParametersKHR);
}

void ThreadSafety::PostCallRecordGetEncodedVideoSessionParametersKHR(
    VkDevice device, const VkVideoEncodeSessionParametersGetInfoKHR* pVideoSessionParametersInfo,
    VkVideoEncodeSessionParametersFeedbackInfoKHR* pFeedbackInfo, size_t* pDataSize, void* pData, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetEncodedVideoSessionParametersKHR);
}

#endif  // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
void ThreadSafety::PreCallRecordCmdEncodeVideoKHR(VkCommandBuffer commandBuffer, const VkVideoEncodeInfoKHR* pEncodeInfo) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdEncodeVideoKHR);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdEncodeVideoKHR(VkCommandBuffer commandBuffer, const VkVideoEncodeInfoKHR* pEncodeInfo,
                                                   const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdEncodeVideoKHR);
    // Host access to commandBuffer must be externally synchronized
}

#endif  // VK_ENABLE_BETA_EXTENSIONS
void ThreadSafety::PreCallRecordCmdSetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event,
                                                const VkDependencyInfo* pDependencyInfo) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdSetEvent2KHR);
    StartReadObject(event, vvl::Func::vkCmdSetEvent2KHR);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event,
                                                 const VkDependencyInfo* pDependencyInfo, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdSetEvent2KHR);
    FinishReadObject(event, vvl::Func::vkCmdSetEvent2KHR);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdResetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags2 stageMask) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdResetEvent2KHR);
    StartReadObject(event, vvl::Func::vkCmdResetEvent2KHR);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdResetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags2 stageMask,
                                                   const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdResetEvent2KHR);
    FinishReadObject(event, vvl::Func::vkCmdResetEvent2KHR);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdWaitEvents2KHR(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                                  const VkDependencyInfo* pDependencyInfos) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdWaitEvents2KHR);

    if (pEvents) {
        for (uint32_t index = 0; index < eventCount; index++) {
            StartReadObject(pEvents[index], vvl::Func::vkCmdWaitEvents2KHR);
        }
    }
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdWaitEvents2KHR(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                                   const VkDependencyInfo* pDependencyInfos, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdWaitEvents2KHR);

    if (pEvents) {
        for (uint32_t index = 0; index < eventCount; index++) {
            FinishReadObject(pEvents[index], vvl::Func::vkCmdWaitEvents2KHR);
        }
    }
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdPipelineBarrier2KHR(VkCommandBuffer commandBuffer, const VkDependencyInfo* pDependencyInfo) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdPipelineBarrier2KHR);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdPipelineBarrier2KHR(VkCommandBuffer commandBuffer, const VkDependencyInfo* pDependencyInfo,
                                                        const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdPipelineBarrier2KHR);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdWriteTimestamp2KHR(VkCommandBuffer commandBuffer, VkPipelineStageFlags2 stage,
                                                      VkQueryPool queryPool, uint32_t query) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdWriteTimestamp2KHR);
    StartReadObject(queryPool, vvl::Func::vkCmdWriteTimestamp2KHR);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdWriteTimestamp2KHR(VkCommandBuffer commandBuffer, VkPipelineStageFlags2 stage,
                                                       VkQueryPool queryPool, uint32_t query, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdWriteTimestamp2KHR);
    FinishReadObject(queryPool, vvl::Func::vkCmdWriteTimestamp2KHR);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordQueueSubmit2KHR(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2* pSubmits, VkFence fence) {
    StartWriteObject(queue, vvl::Func::vkQueueSubmit2KHR);
    StartWriteObject(fence, vvl::Func::vkQueueSubmit2KHR);
    // Host access to queue must be externally synchronized
    // Host access to fence must be externally synchronized
}

void ThreadSafety::PostCallRecordQueueSubmit2KHR(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2* pSubmits, VkFence fence,
                                                 const RecordObject& record_obj) {
    FinishWriteObject(queue, vvl::Func::vkQueueSubmit2KHR);
    FinishWriteObject(fence, vvl::Func::vkQueueSubmit2KHR);
    // Host access to queue must be externally synchronized
    // Host access to fence must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdWriteBufferMarker2AMD(VkCommandBuffer commandBuffer, VkPipelineStageFlags2 stage,
                                                         VkBuffer dstBuffer, VkDeviceSize dstOffset, uint32_t marker) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdWriteBufferMarker2AMD);
    StartReadObject(dstBuffer, vvl::Func::vkCmdWriteBufferMarker2AMD);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdWriteBufferMarker2AMD(VkCommandBuffer commandBuffer, VkPipelineStageFlags2 stage,
                                                          VkBuffer dstBuffer, VkDeviceSize dstOffset, uint32_t marker,
                                                          const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdWriteBufferMarker2AMD);
    FinishReadObject(dstBuffer, vvl::Func::vkCmdWriteBufferMarker2AMD);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordGetQueueCheckpointData2NV(VkQueue queue, uint32_t* pCheckpointDataCount,
                                                          VkCheckpointData2NV* pCheckpointData) {
    StartReadObject(queue, vvl::Func::vkGetQueueCheckpointData2NV);
}

void ThreadSafety::PostCallRecordGetQueueCheckpointData2NV(VkQueue queue, uint32_t* pCheckpointDataCount,
                                                           VkCheckpointData2NV* pCheckpointData, const RecordObject& record_obj) {
    FinishReadObject(queue, vvl::Func::vkGetQueueCheckpointData2NV);
}

void ThreadSafety::PreCallRecordCmdCopyBuffer2KHR(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2* pCopyBufferInfo) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdCopyBuffer2KHR);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdCopyBuffer2KHR(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2* pCopyBufferInfo,
                                                   const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdCopyBuffer2KHR);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdCopyImage2KHR(VkCommandBuffer commandBuffer, const VkCopyImageInfo2* pCopyImageInfo) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdCopyImage2KHR);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdCopyImage2KHR(VkCommandBuffer commandBuffer, const VkCopyImageInfo2* pCopyImageInfo,
                                                  const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdCopyImage2KHR);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdCopyBufferToImage2KHR(VkCommandBuffer commandBuffer,
                                                         const VkCopyBufferToImageInfo2* pCopyBufferToImageInfo) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdCopyBufferToImage2KHR);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdCopyBufferToImage2KHR(VkCommandBuffer commandBuffer,
                                                          const VkCopyBufferToImageInfo2* pCopyBufferToImageInfo,
                                                          const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdCopyBufferToImage2KHR);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdCopyImageToBuffer2KHR(VkCommandBuffer commandBuffer,
                                                         const VkCopyImageToBufferInfo2* pCopyImageToBufferInfo) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdCopyImageToBuffer2KHR);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdCopyImageToBuffer2KHR(VkCommandBuffer commandBuffer,
                                                          const VkCopyImageToBufferInfo2* pCopyImageToBufferInfo,
                                                          const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdCopyImageToBuffer2KHR);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdBlitImage2KHR(VkCommandBuffer commandBuffer, const VkBlitImageInfo2* pBlitImageInfo) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdBlitImage2KHR);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdBlitImage2KHR(VkCommandBuffer commandBuffer, const VkBlitImageInfo2* pBlitImageInfo,
                                                  const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdBlitImage2KHR);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdResolveImage2KHR(VkCommandBuffer commandBuffer, const VkResolveImageInfo2* pResolveImageInfo) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdResolveImage2KHR);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdResolveImage2KHR(VkCommandBuffer commandBuffer, const VkResolveImageInfo2* pResolveImageInfo,
                                                     const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdResolveImage2KHR);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdTraceRaysIndirect2KHR(VkCommandBuffer commandBuffer, VkDeviceAddress indirectDeviceAddress) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdTraceRaysIndirect2KHR);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdTraceRaysIndirect2KHR(VkCommandBuffer commandBuffer, VkDeviceAddress indirectDeviceAddress,
                                                          const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdTraceRaysIndirect2KHR);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordGetDeviceBufferMemoryRequirementsKHR(VkDevice device, const VkDeviceBufferMemoryRequirements* pInfo,
                                                                     VkMemoryRequirements2* pMemoryRequirements) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetDeviceBufferMemoryRequirementsKHR);
}

void ThreadSafety::PostCallRecordGetDeviceBufferMemoryRequirementsKHR(VkDevice device,
                                                                      const VkDeviceBufferMemoryRequirements* pInfo,
                                                                      VkMemoryRequirements2* pMemoryRequirements,
                                                                      const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetDeviceBufferMemoryRequirementsKHR);
}

void ThreadSafety::PreCallRecordGetDeviceImageMemoryRequirementsKHR(VkDevice device, const VkDeviceImageMemoryRequirements* pInfo,
                                                                    VkMemoryRequirements2* pMemoryRequirements) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetDeviceImageMemoryRequirementsKHR);
}

void ThreadSafety::PostCallRecordGetDeviceImageMemoryRequirementsKHR(VkDevice device, const VkDeviceImageMemoryRequirements* pInfo,
                                                                     VkMemoryRequirements2* pMemoryRequirements,
                                                                     const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetDeviceImageMemoryRequirementsKHR);
}

void ThreadSafety::PreCallRecordGetDeviceImageSparseMemoryRequirementsKHR(
    VkDevice device, const VkDeviceImageMemoryRequirements* pInfo, uint32_t* pSparseMemoryRequirementCount,
    VkSparseImageMemoryRequirements2* pSparseMemoryRequirements) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetDeviceImageSparseMemoryRequirementsKHR);
}

void ThreadSafety::PostCallRecordGetDeviceImageSparseMemoryRequirementsKHR(
    VkDevice device, const VkDeviceImageMemoryRequirements* pInfo, uint32_t* pSparseMemoryRequirementCount,
    VkSparseImageMemoryRequirements2* pSparseMemoryRequirements, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetDeviceImageSparseMemoryRequirementsKHR);
}

void ThreadSafety::PreCallRecordCmdBindIndexBuffer2KHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                       VkDeviceSize size, VkIndexType indexType) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdBindIndexBuffer2KHR);
    StartReadObject(buffer, vvl::Func::vkCmdBindIndexBuffer2KHR);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdBindIndexBuffer2KHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                        VkDeviceSize size, VkIndexType indexType, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdBindIndexBuffer2KHR);
    FinishReadObject(buffer, vvl::Func::vkCmdBindIndexBuffer2KHR);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordGetRenderingAreaGranularityKHR(VkDevice device, const VkRenderingAreaInfoKHR* pRenderingAreaInfo,
                                                               VkExtent2D* pGranularity) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetRenderingAreaGranularityKHR);
}

void ThreadSafety::PostCallRecordGetRenderingAreaGranularityKHR(VkDevice device, const VkRenderingAreaInfoKHR* pRenderingAreaInfo,
                                                                VkExtent2D* pGranularity, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetRenderingAreaGranularityKHR);
}

void ThreadSafety::PreCallRecordGetDeviceImageSubresourceLayoutKHR(VkDevice device, const VkDeviceImageSubresourceInfoKHR* pInfo,
                                                                   VkSubresourceLayout2KHR* pLayout) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetDeviceImageSubresourceLayoutKHR);
}

void ThreadSafety::PostCallRecordGetDeviceImageSubresourceLayoutKHR(VkDevice device, const VkDeviceImageSubresourceInfoKHR* pInfo,
                                                                    VkSubresourceLayout2KHR* pLayout,
                                                                    const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetDeviceImageSubresourceLayoutKHR);
}

void ThreadSafety::PreCallRecordGetImageSubresourceLayout2KHR(VkDevice device, VkImage image,
                                                              const VkImageSubresource2KHR* pSubresource,
                                                              VkSubresourceLayout2KHR* pLayout) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetImageSubresourceLayout2KHR);
    StartReadObject(image, vvl::Func::vkGetImageSubresourceLayout2KHR);
}

void ThreadSafety::PostCallRecordGetImageSubresourceLayout2KHR(VkDevice device, VkImage image,
                                                               const VkImageSubresource2KHR* pSubresource,
                                                               VkSubresourceLayout2KHR* pLayout, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetImageSubresourceLayout2KHR);
    FinishReadObject(image, vvl::Func::vkGetImageSubresourceLayout2KHR);
}

void ThreadSafety::PreCallRecordCreateDebugReportCallbackEXT(VkInstance instance,
                                                             const VkDebugReportCallbackCreateInfoEXT* pCreateInfo,
                                                             const VkAllocationCallbacks* pAllocator,
                                                             VkDebugReportCallbackEXT* pCallback) {
    StartReadObjectParentInstance(instance, vvl::Func::vkCreateDebugReportCallbackEXT);
}

void ThreadSafety::PostCallRecordCreateDebugReportCallbackEXT(VkInstance instance,
                                                              const VkDebugReportCallbackCreateInfoEXT* pCreateInfo,
                                                              const VkAllocationCallbacks* pAllocator,
                                                              VkDebugReportCallbackEXT* pCallback, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(instance, vvl::Func::vkCreateDebugReportCallbackEXT);
    if (record_obj.result == VK_SUCCESS) {
        CreateObjectParentInstance(*pCallback);
    }
}

void ThreadSafety::PreCallRecordDestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback,
                                                              const VkAllocationCallbacks* pAllocator) {
    StartReadObjectParentInstance(instance, vvl::Func::vkDestroyDebugReportCallbackEXT);
    StartWriteObjectParentInstance(callback, vvl::Func::vkDestroyDebugReportCallbackEXT);
    // Host access to callback must be externally synchronized
}

void ThreadSafety::PostCallRecordDestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback,
                                                               const VkAllocationCallbacks* pAllocator,
                                                               const RecordObject& record_obj) {
    FinishReadObjectParentInstance(instance, vvl::Func::vkDestroyDebugReportCallbackEXT);
    FinishWriteObjectParentInstance(callback, vvl::Func::vkDestroyDebugReportCallbackEXT);
    DestroyObjectParentInstance(callback);
    // Host access to callback must be externally synchronized
}

void ThreadSafety::PreCallRecordDebugReportMessageEXT(VkInstance instance, VkDebugReportFlagsEXT flags,
                                                      VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location,
                                                      int32_t messageCode, const char* pLayerPrefix, const char* pMessage) {
    StartReadObjectParentInstance(instance, vvl::Func::vkDebugReportMessageEXT);
}

void ThreadSafety::PostCallRecordDebugReportMessageEXT(VkInstance instance, VkDebugReportFlagsEXT flags,
                                                       VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location,
                                                       int32_t messageCode, const char* pLayerPrefix, const char* pMessage,
                                                       const RecordObject& record_obj) {
    FinishReadObjectParentInstance(instance, vvl::Func::vkDebugReportMessageEXT);
}

void ThreadSafety::PreCallRecordCmdBindTransformFeedbackBuffersEXT(VkCommandBuffer commandBuffer, uint32_t firstBinding,
                                                                   uint32_t bindingCount, const VkBuffer* pBuffers,
                                                                   const VkDeviceSize* pOffsets, const VkDeviceSize* pSizes) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdBindTransformFeedbackBuffersEXT);

    if (pBuffers) {
        for (uint32_t index = 0; index < bindingCount; index++) {
            StartReadObject(pBuffers[index], vvl::Func::vkCmdBindTransformFeedbackBuffersEXT);
        }
    }
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdBindTransformFeedbackBuffersEXT(VkCommandBuffer commandBuffer, uint32_t firstBinding,
                                                                    uint32_t bindingCount, const VkBuffer* pBuffers,
                                                                    const VkDeviceSize* pOffsets, const VkDeviceSize* pSizes,
                                                                    const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdBindTransformFeedbackBuffersEXT);

    if (pBuffers) {
        for (uint32_t index = 0; index < bindingCount; index++) {
            FinishReadObject(pBuffers[index], vvl::Func::vkCmdBindTransformFeedbackBuffersEXT);
        }
    }
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdBeginTransformFeedbackEXT(VkCommandBuffer commandBuffer, uint32_t firstCounterBuffer,
                                                             uint32_t counterBufferCount, const VkBuffer* pCounterBuffers,
                                                             const VkDeviceSize* pCounterBufferOffsets) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdBeginTransformFeedbackEXT);

    if (pCounterBuffers) {
        for (uint32_t index = 0; index < counterBufferCount; index++) {
            StartReadObject(pCounterBuffers[index], vvl::Func::vkCmdBeginTransformFeedbackEXT);
        }
    }
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdBeginTransformFeedbackEXT(VkCommandBuffer commandBuffer, uint32_t firstCounterBuffer,
                                                              uint32_t counterBufferCount, const VkBuffer* pCounterBuffers,
                                                              const VkDeviceSize* pCounterBufferOffsets,
                                                              const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdBeginTransformFeedbackEXT);

    if (pCounterBuffers) {
        for (uint32_t index = 0; index < counterBufferCount; index++) {
            FinishReadObject(pCounterBuffers[index], vvl::Func::vkCmdBeginTransformFeedbackEXT);
        }
    }
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdEndTransformFeedbackEXT(VkCommandBuffer commandBuffer, uint32_t firstCounterBuffer,
                                                           uint32_t counterBufferCount, const VkBuffer* pCounterBuffers,
                                                           const VkDeviceSize* pCounterBufferOffsets) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdEndTransformFeedbackEXT);

    if (pCounterBuffers) {
        for (uint32_t index = 0; index < counterBufferCount; index++) {
            StartReadObject(pCounterBuffers[index], vvl::Func::vkCmdEndTransformFeedbackEXT);
        }
    }
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdEndTransformFeedbackEXT(VkCommandBuffer commandBuffer, uint32_t firstCounterBuffer,
                                                            uint32_t counterBufferCount, const VkBuffer* pCounterBuffers,
                                                            const VkDeviceSize* pCounterBufferOffsets,
                                                            const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdEndTransformFeedbackEXT);

    if (pCounterBuffers) {
        for (uint32_t index = 0; index < counterBufferCount; index++) {
            FinishReadObject(pCounterBuffers[index], vvl::Func::vkCmdEndTransformFeedbackEXT);
        }
    }
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdBeginQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query,
                                                        VkQueryControlFlags flags, uint32_t index) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdBeginQueryIndexedEXT);
    StartReadObject(queryPool, vvl::Func::vkCmdBeginQueryIndexedEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdBeginQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query,
                                                         VkQueryControlFlags flags, uint32_t index,
                                                         const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdBeginQueryIndexedEXT);
    FinishReadObject(queryPool, vvl::Func::vkCmdBeginQueryIndexedEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdEndQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query,
                                                      uint32_t index) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdEndQueryIndexedEXT);
    StartReadObject(queryPool, vvl::Func::vkCmdEndQueryIndexedEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdEndQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query,
                                                       uint32_t index, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdEndQueryIndexedEXT);
    FinishReadObject(queryPool, vvl::Func::vkCmdEndQueryIndexedEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdDrawIndirectByteCountEXT(VkCommandBuffer commandBuffer, uint32_t instanceCount,
                                                            uint32_t firstInstance, VkBuffer counterBuffer,
                                                            VkDeviceSize counterBufferOffset, uint32_t counterOffset,
                                                            uint32_t vertexStride) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdDrawIndirectByteCountEXT);
    StartReadObject(counterBuffer, vvl::Func::vkCmdDrawIndirectByteCountEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdDrawIndirectByteCountEXT(VkCommandBuffer commandBuffer, uint32_t instanceCount,
                                                             uint32_t firstInstance, VkBuffer counterBuffer,
                                                             VkDeviceSize counterBufferOffset, uint32_t counterOffset,
                                                             uint32_t vertexStride, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdDrawIndirectByteCountEXT);
    FinishReadObject(counterBuffer, vvl::Func::vkCmdDrawIndirectByteCountEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCreateCuModuleNVX(VkDevice device, const VkCuModuleCreateInfoNVX* pCreateInfo,
                                                  const VkAllocationCallbacks* pAllocator, VkCuModuleNVX* pModule) {
    StartReadObjectParentInstance(device, vvl::Func::vkCreateCuModuleNVX);
}

void ThreadSafety::PostCallRecordCreateCuModuleNVX(VkDevice device, const VkCuModuleCreateInfoNVX* pCreateInfo,
                                                   const VkAllocationCallbacks* pAllocator, VkCuModuleNVX* pModule,
                                                   const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkCreateCuModuleNVX);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pModule);
    }
}

void ThreadSafety::PreCallRecordCreateCuFunctionNVX(VkDevice device, const VkCuFunctionCreateInfoNVX* pCreateInfo,
                                                    const VkAllocationCallbacks* pAllocator, VkCuFunctionNVX* pFunction) {
    StartReadObjectParentInstance(device, vvl::Func::vkCreateCuFunctionNVX);
}

void ThreadSafety::PostCallRecordCreateCuFunctionNVX(VkDevice device, const VkCuFunctionCreateInfoNVX* pCreateInfo,
                                                     const VkAllocationCallbacks* pAllocator, VkCuFunctionNVX* pFunction,
                                                     const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkCreateCuFunctionNVX);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pFunction);
    }
}

void ThreadSafety::PreCallRecordDestroyCuModuleNVX(VkDevice device, VkCuModuleNVX module, const VkAllocationCallbacks* pAllocator) {
    StartReadObjectParentInstance(device, vvl::Func::vkDestroyCuModuleNVX);
    StartReadObject(module, vvl::Func::vkDestroyCuModuleNVX);
}

void ThreadSafety::PostCallRecordDestroyCuModuleNVX(VkDevice device, VkCuModuleNVX module, const VkAllocationCallbacks* pAllocator,
                                                    const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkDestroyCuModuleNVX);
    FinishReadObject(module, vvl::Func::vkDestroyCuModuleNVX);
}

void ThreadSafety::PreCallRecordDestroyCuFunctionNVX(VkDevice device, VkCuFunctionNVX function,
                                                     const VkAllocationCallbacks* pAllocator) {
    StartReadObjectParentInstance(device, vvl::Func::vkDestroyCuFunctionNVX);
    StartReadObject(function, vvl::Func::vkDestroyCuFunctionNVX);
}

void ThreadSafety::PostCallRecordDestroyCuFunctionNVX(VkDevice device, VkCuFunctionNVX function,
                                                      const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkDestroyCuFunctionNVX);
    FinishReadObject(function, vvl::Func::vkDestroyCuFunctionNVX);
}

void ThreadSafety::PreCallRecordCmdCuLaunchKernelNVX(VkCommandBuffer commandBuffer, const VkCuLaunchInfoNVX* pLaunchInfo) {
    StartReadObject(commandBuffer, vvl::Func::vkCmdCuLaunchKernelNVX);
}

void ThreadSafety::PostCallRecordCmdCuLaunchKernelNVX(VkCommandBuffer commandBuffer, const VkCuLaunchInfoNVX* pLaunchInfo,
                                                      const RecordObject& record_obj) {
    FinishReadObject(commandBuffer, vvl::Func::vkCmdCuLaunchKernelNVX);
}

void ThreadSafety::PreCallRecordGetImageViewHandleNVX(VkDevice device, const VkImageViewHandleInfoNVX* pInfo) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetImageViewHandleNVX);
}

void ThreadSafety::PostCallRecordGetImageViewHandleNVX(VkDevice device, const VkImageViewHandleInfoNVX* pInfo,
                                                       const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetImageViewHandleNVX);
}

void ThreadSafety::PreCallRecordGetImageViewAddressNVX(VkDevice device, VkImageView imageView,
                                                       VkImageViewAddressPropertiesNVX* pProperties) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetImageViewAddressNVX);
    StartReadObject(imageView, vvl::Func::vkGetImageViewAddressNVX);
}

void ThreadSafety::PostCallRecordGetImageViewAddressNVX(VkDevice device, VkImageView imageView,
                                                        VkImageViewAddressPropertiesNVX* pProperties,
                                                        const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetImageViewAddressNVX);
    FinishReadObject(imageView, vvl::Func::vkGetImageViewAddressNVX);
}

void ThreadSafety::PreCallRecordCmdDrawIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                        VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                        uint32_t stride) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdDrawIndirectCountAMD);
    StartReadObject(buffer, vvl::Func::vkCmdDrawIndirectCountAMD);
    StartReadObject(countBuffer, vvl::Func::vkCmdDrawIndirectCountAMD);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdDrawIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                         VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                         uint32_t maxDrawCount, uint32_t stride, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdDrawIndirectCountAMD);
    FinishReadObject(buffer, vvl::Func::vkCmdDrawIndirectCountAMD);
    FinishReadObject(countBuffer, vvl::Func::vkCmdDrawIndirectCountAMD);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdDrawIndexedIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                               VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                               uint32_t maxDrawCount, uint32_t stride) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdDrawIndexedIndirectCountAMD);
    StartReadObject(buffer, vvl::Func::vkCmdDrawIndexedIndirectCountAMD);
    StartReadObject(countBuffer, vvl::Func::vkCmdDrawIndexedIndirectCountAMD);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdDrawIndexedIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                                VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                                uint32_t maxDrawCount, uint32_t stride,
                                                                const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdDrawIndexedIndirectCountAMD);
    FinishReadObject(buffer, vvl::Func::vkCmdDrawIndexedIndirectCountAMD);
    FinishReadObject(countBuffer, vvl::Func::vkCmdDrawIndexedIndirectCountAMD);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordGetShaderInfoAMD(VkDevice device, VkPipeline pipeline, VkShaderStageFlagBits shaderStage,
                                                 VkShaderInfoTypeAMD infoType, size_t* pInfoSize, void* pInfo) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetShaderInfoAMD);
    StartReadObject(pipeline, vvl::Func::vkGetShaderInfoAMD);
}

void ThreadSafety::PostCallRecordGetShaderInfoAMD(VkDevice device, VkPipeline pipeline, VkShaderStageFlagBits shaderStage,
                                                  VkShaderInfoTypeAMD infoType, size_t* pInfoSize, void* pInfo,
                                                  const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetShaderInfoAMD);
    FinishReadObject(pipeline, vvl::Func::vkGetShaderInfoAMD);
}

#ifdef VK_USE_PLATFORM_GGP
void ThreadSafety::PreCallRecordCreateStreamDescriptorSurfaceGGP(VkInstance instance,
                                                                 const VkStreamDescriptorSurfaceCreateInfoGGP* pCreateInfo,
                                                                 const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    StartReadObjectParentInstance(instance, vvl::Func::vkCreateStreamDescriptorSurfaceGGP);
}

void ThreadSafety::PostCallRecordCreateStreamDescriptorSurfaceGGP(VkInstance instance,
                                                                  const VkStreamDescriptorSurfaceCreateInfoGGP* pCreateInfo,
                                                                  const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                                  const RecordObject& record_obj) {
    FinishReadObjectParentInstance(instance, vvl::Func::vkCreateStreamDescriptorSurfaceGGP);
    if (record_obj.result == VK_SUCCESS) {
        CreateObjectParentInstance(*pSurface);
    }
}

#endif  // VK_USE_PLATFORM_GGP
#ifdef VK_USE_PLATFORM_WIN32_KHR
void ThreadSafety::PreCallRecordGetMemoryWin32HandleNV(VkDevice device, VkDeviceMemory memory,
                                                       VkExternalMemoryHandleTypeFlagsNV handleType, HANDLE* pHandle) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetMemoryWin32HandleNV);
    StartReadObject(memory, vvl::Func::vkGetMemoryWin32HandleNV);
}

void ThreadSafety::PostCallRecordGetMemoryWin32HandleNV(VkDevice device, VkDeviceMemory memory,
                                                        VkExternalMemoryHandleTypeFlagsNV handleType, HANDLE* pHandle,
                                                        const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetMemoryWin32HandleNV);
    FinishReadObject(memory, vvl::Func::vkGetMemoryWin32HandleNV);
}

#endif  // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_VI_NN
void ThreadSafety::PreCallRecordCreateViSurfaceNN(VkInstance instance, const VkViSurfaceCreateInfoNN* pCreateInfo,
                                                  const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    StartReadObjectParentInstance(instance, vvl::Func::vkCreateViSurfaceNN);
}

void ThreadSafety::PostCallRecordCreateViSurfaceNN(VkInstance instance, const VkViSurfaceCreateInfoNN* pCreateInfo,
                                                   const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                   const RecordObject& record_obj) {
    FinishReadObjectParentInstance(instance, vvl::Func::vkCreateViSurfaceNN);
    if (record_obj.result == VK_SUCCESS) {
        CreateObjectParentInstance(*pSurface);
    }
}

#endif  // VK_USE_PLATFORM_VI_NN
void ThreadSafety::PreCallRecordCmdBeginConditionalRenderingEXT(
    VkCommandBuffer commandBuffer, const VkConditionalRenderingBeginInfoEXT* pConditionalRenderingBegin) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdBeginConditionalRenderingEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdBeginConditionalRenderingEXT(
    VkCommandBuffer commandBuffer, const VkConditionalRenderingBeginInfoEXT* pConditionalRenderingBegin,
    const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdBeginConditionalRenderingEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdEndConditionalRenderingEXT(VkCommandBuffer commandBuffer) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdEndConditionalRenderingEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdEndConditionalRenderingEXT(VkCommandBuffer commandBuffer, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdEndConditionalRenderingEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetViewportWScalingNV(VkCommandBuffer commandBuffer, uint32_t firstViewport,
                                                         uint32_t viewportCount, const VkViewportWScalingNV* pViewportWScalings) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdSetViewportWScalingNV);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetViewportWScalingNV(VkCommandBuffer commandBuffer, uint32_t firstViewport,
                                                          uint32_t viewportCount, const VkViewportWScalingNV* pViewportWScalings,
                                                          const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdSetViewportWScalingNV);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordReleaseDisplayEXT(VkPhysicalDevice physicalDevice, VkDisplayKHR display) {
    StartReadObjectParentInstance(display, vvl::Func::vkReleaseDisplayEXT);
}

void ThreadSafety::PostCallRecordReleaseDisplayEXT(VkPhysicalDevice physicalDevice, VkDisplayKHR display,
                                                   const RecordObject& record_obj) {
    FinishReadObjectParentInstance(display, vvl::Func::vkReleaseDisplayEXT);
}

#ifdef VK_USE_PLATFORM_XLIB_XRANDR_EXT
void ThreadSafety::PreCallRecordAcquireXlibDisplayEXT(VkPhysicalDevice physicalDevice, Display* dpy, VkDisplayKHR display) {
    StartReadObjectParentInstance(display, vvl::Func::vkAcquireXlibDisplayEXT);
}

void ThreadSafety::PostCallRecordAcquireXlibDisplayEXT(VkPhysicalDevice physicalDevice, Display* dpy, VkDisplayKHR display,
                                                       const RecordObject& record_obj) {
    FinishReadObjectParentInstance(display, vvl::Func::vkAcquireXlibDisplayEXT);
}

#endif  // VK_USE_PLATFORM_XLIB_XRANDR_EXT
void ThreadSafety::PreCallRecordGetPhysicalDeviceSurfaceCapabilities2EXT(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                         VkSurfaceCapabilities2EXT* pSurfaceCapabilities) {
    StartReadObjectParentInstance(surface, vvl::Func::vkGetPhysicalDeviceSurfaceCapabilities2EXT);
}

void ThreadSafety::PostCallRecordGetPhysicalDeviceSurfaceCapabilities2EXT(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                          VkSurfaceCapabilities2EXT* pSurfaceCapabilities,
                                                                          const RecordObject& record_obj) {
    FinishReadObjectParentInstance(surface, vvl::Func::vkGetPhysicalDeviceSurfaceCapabilities2EXT);
}

void ThreadSafety::PreCallRecordDisplayPowerControlEXT(VkDevice device, VkDisplayKHR display,
                                                       const VkDisplayPowerInfoEXT* pDisplayPowerInfo) {
    StartReadObjectParentInstance(device, vvl::Func::vkDisplayPowerControlEXT);
    StartReadObjectParentInstance(display, vvl::Func::vkDisplayPowerControlEXT);
}

void ThreadSafety::PostCallRecordDisplayPowerControlEXT(VkDevice device, VkDisplayKHR display,
                                                        const VkDisplayPowerInfoEXT* pDisplayPowerInfo,
                                                        const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkDisplayPowerControlEXT);
    FinishReadObjectParentInstance(display, vvl::Func::vkDisplayPowerControlEXT);
}

void ThreadSafety::PreCallRecordRegisterDeviceEventEXT(VkDevice device, const VkDeviceEventInfoEXT* pDeviceEventInfo,
                                                       const VkAllocationCallbacks* pAllocator, VkFence* pFence) {
    StartReadObjectParentInstance(device, vvl::Func::vkRegisterDeviceEventEXT);
}

void ThreadSafety::PostCallRecordRegisterDeviceEventEXT(VkDevice device, const VkDeviceEventInfoEXT* pDeviceEventInfo,
                                                        const VkAllocationCallbacks* pAllocator, VkFence* pFence,
                                                        const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkRegisterDeviceEventEXT);
}

void ThreadSafety::PreCallRecordGetSwapchainCounterEXT(VkDevice device, VkSwapchainKHR swapchain,
                                                       VkSurfaceCounterFlagBitsEXT counter, uint64_t* pCounterValue) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetSwapchainCounterEXT);
    StartReadObject(swapchain, vvl::Func::vkGetSwapchainCounterEXT);
}

void ThreadSafety::PostCallRecordGetSwapchainCounterEXT(VkDevice device, VkSwapchainKHR swapchain,
                                                        VkSurfaceCounterFlagBitsEXT counter, uint64_t* pCounterValue,
                                                        const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetSwapchainCounterEXT);
    FinishReadObject(swapchain, vvl::Func::vkGetSwapchainCounterEXT);
}

void ThreadSafety::PreCallRecordGetRefreshCycleDurationGOOGLE(VkDevice device, VkSwapchainKHR swapchain,
                                                              VkRefreshCycleDurationGOOGLE* pDisplayTimingProperties) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetRefreshCycleDurationGOOGLE);
    StartWriteObject(swapchain, vvl::Func::vkGetRefreshCycleDurationGOOGLE);
    // Host access to swapchain must be externally synchronized
}

void ThreadSafety::PostCallRecordGetRefreshCycleDurationGOOGLE(VkDevice device, VkSwapchainKHR swapchain,
                                                               VkRefreshCycleDurationGOOGLE* pDisplayTimingProperties,
                                                               const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetRefreshCycleDurationGOOGLE);
    FinishWriteObject(swapchain, vvl::Func::vkGetRefreshCycleDurationGOOGLE);
    // Host access to swapchain must be externally synchronized
}

void ThreadSafety::PreCallRecordGetPastPresentationTimingGOOGLE(VkDevice device, VkSwapchainKHR swapchain,
                                                                uint32_t* pPresentationTimingCount,
                                                                VkPastPresentationTimingGOOGLE* pPresentationTimings) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetPastPresentationTimingGOOGLE);
    StartWriteObject(swapchain, vvl::Func::vkGetPastPresentationTimingGOOGLE);
    // Host access to swapchain must be externally synchronized
}

void ThreadSafety::PostCallRecordGetPastPresentationTimingGOOGLE(VkDevice device, VkSwapchainKHR swapchain,
                                                                 uint32_t* pPresentationTimingCount,
                                                                 VkPastPresentationTimingGOOGLE* pPresentationTimings,
                                                                 const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetPastPresentationTimingGOOGLE);
    FinishWriteObject(swapchain, vvl::Func::vkGetPastPresentationTimingGOOGLE);
    // Host access to swapchain must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetDiscardRectangleEXT(VkCommandBuffer commandBuffer, uint32_t firstDiscardRectangle,
                                                          uint32_t discardRectangleCount, const VkRect2D* pDiscardRectangles) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdSetDiscardRectangleEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetDiscardRectangleEXT(VkCommandBuffer commandBuffer, uint32_t firstDiscardRectangle,
                                                           uint32_t discardRectangleCount, const VkRect2D* pDiscardRectangles,
                                                           const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdSetDiscardRectangleEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetDiscardRectangleEnableEXT(VkCommandBuffer commandBuffer, VkBool32 discardRectangleEnable) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdSetDiscardRectangleEnableEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetDiscardRectangleEnableEXT(VkCommandBuffer commandBuffer, VkBool32 discardRectangleEnable,
                                                                 const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdSetDiscardRectangleEnableEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetDiscardRectangleModeEXT(VkCommandBuffer commandBuffer,
                                                              VkDiscardRectangleModeEXT discardRectangleMode) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdSetDiscardRectangleModeEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetDiscardRectangleModeEXT(VkCommandBuffer commandBuffer,
                                                               VkDiscardRectangleModeEXT discardRectangleMode,
                                                               const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdSetDiscardRectangleModeEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordSetHdrMetadataEXT(VkDevice device, uint32_t swapchainCount, const VkSwapchainKHR* pSwapchains,
                                                  const VkHdrMetadataEXT* pMetadata) {
    StartReadObjectParentInstance(device, vvl::Func::vkSetHdrMetadataEXT);

    if (pSwapchains) {
        for (uint32_t index = 0; index < swapchainCount; index++) {
            StartReadObject(pSwapchains[index], vvl::Func::vkSetHdrMetadataEXT);
        }
    }
}

void ThreadSafety::PostCallRecordSetHdrMetadataEXT(VkDevice device, uint32_t swapchainCount, const VkSwapchainKHR* pSwapchains,
                                                   const VkHdrMetadataEXT* pMetadata, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkSetHdrMetadataEXT);

    if (pSwapchains) {
        for (uint32_t index = 0; index < swapchainCount; index++) {
            FinishReadObject(pSwapchains[index], vvl::Func::vkSetHdrMetadataEXT);
        }
    }
}

#ifdef VK_USE_PLATFORM_IOS_MVK
void ThreadSafety::PreCallRecordCreateIOSSurfaceMVK(VkInstance instance, const VkIOSSurfaceCreateInfoMVK* pCreateInfo,
                                                    const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    StartReadObjectParentInstance(instance, vvl::Func::vkCreateIOSSurfaceMVK);
}

void ThreadSafety::PostCallRecordCreateIOSSurfaceMVK(VkInstance instance, const VkIOSSurfaceCreateInfoMVK* pCreateInfo,
                                                     const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                     const RecordObject& record_obj) {
    FinishReadObjectParentInstance(instance, vvl::Func::vkCreateIOSSurfaceMVK);
    if (record_obj.result == VK_SUCCESS) {
        CreateObjectParentInstance(*pSurface);
    }
}

#endif  // VK_USE_PLATFORM_IOS_MVK
#ifdef VK_USE_PLATFORM_MACOS_MVK
void ThreadSafety::PreCallRecordCreateMacOSSurfaceMVK(VkInstance instance, const VkMacOSSurfaceCreateInfoMVK* pCreateInfo,
                                                      const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    StartReadObjectParentInstance(instance, vvl::Func::vkCreateMacOSSurfaceMVK);
}

void ThreadSafety::PostCallRecordCreateMacOSSurfaceMVK(VkInstance instance, const VkMacOSSurfaceCreateInfoMVK* pCreateInfo,
                                                       const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                       const RecordObject& record_obj) {
    FinishReadObjectParentInstance(instance, vvl::Func::vkCreateMacOSSurfaceMVK);
    if (record_obj.result == VK_SUCCESS) {
        CreateObjectParentInstance(*pSurface);
    }
}

#endif  // VK_USE_PLATFORM_MACOS_MVK
void ThreadSafety::PreCallRecordQueueBeginDebugUtilsLabelEXT(VkQueue queue, const VkDebugUtilsLabelEXT* pLabelInfo) {
    StartReadObject(queue, vvl::Func::vkQueueBeginDebugUtilsLabelEXT);
}

void ThreadSafety::PostCallRecordQueueBeginDebugUtilsLabelEXT(VkQueue queue, const VkDebugUtilsLabelEXT* pLabelInfo,
                                                              const RecordObject& record_obj) {
    FinishReadObject(queue, vvl::Func::vkQueueBeginDebugUtilsLabelEXT);
}

void ThreadSafety::PreCallRecordQueueEndDebugUtilsLabelEXT(VkQueue queue) {
    StartReadObject(queue, vvl::Func::vkQueueEndDebugUtilsLabelEXT);
}

void ThreadSafety::PostCallRecordQueueEndDebugUtilsLabelEXT(VkQueue queue, const RecordObject& record_obj) {
    FinishReadObject(queue, vvl::Func::vkQueueEndDebugUtilsLabelEXT);
}

void ThreadSafety::PreCallRecordQueueInsertDebugUtilsLabelEXT(VkQueue queue, const VkDebugUtilsLabelEXT* pLabelInfo) {
    StartReadObject(queue, vvl::Func::vkQueueInsertDebugUtilsLabelEXT);
}

void ThreadSafety::PostCallRecordQueueInsertDebugUtilsLabelEXT(VkQueue queue, const VkDebugUtilsLabelEXT* pLabelInfo,
                                                               const RecordObject& record_obj) {
    FinishReadObject(queue, vvl::Func::vkQueueInsertDebugUtilsLabelEXT);
}

void ThreadSafety::PreCallRecordCmdBeginDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT* pLabelInfo) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdBeginDebugUtilsLabelEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdBeginDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT* pLabelInfo,
                                                            const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdBeginDebugUtilsLabelEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdEndDebugUtilsLabelEXT(VkCommandBuffer commandBuffer) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdEndDebugUtilsLabelEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdEndDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdEndDebugUtilsLabelEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdInsertDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT* pLabelInfo) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdInsertDebugUtilsLabelEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdInsertDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT* pLabelInfo,
                                                             const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdInsertDebugUtilsLabelEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCreateDebugUtilsMessengerEXT(VkInstance instance,
                                                             const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                                             const VkAllocationCallbacks* pAllocator,
                                                             VkDebugUtilsMessengerEXT* pMessenger) {
    StartReadObjectParentInstance(instance, vvl::Func::vkCreateDebugUtilsMessengerEXT);
}

void ThreadSafety::PostCallRecordCreateDebugUtilsMessengerEXT(VkInstance instance,
                                                              const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                                              const VkAllocationCallbacks* pAllocator,
                                                              VkDebugUtilsMessengerEXT* pMessenger,
                                                              const RecordObject& record_obj) {
    FinishReadObjectParentInstance(instance, vvl::Func::vkCreateDebugUtilsMessengerEXT);
    if (record_obj.result == VK_SUCCESS) {
        CreateObjectParentInstance(*pMessenger);
    }
}

void ThreadSafety::PreCallRecordDestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT messenger,
                                                              const VkAllocationCallbacks* pAllocator) {
    StartReadObjectParentInstance(instance, vvl::Func::vkDestroyDebugUtilsMessengerEXT);
    StartWriteObjectParentInstance(messenger, vvl::Func::vkDestroyDebugUtilsMessengerEXT);
    // Host access to messenger must be externally synchronized
}

void ThreadSafety::PostCallRecordDestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT messenger,
                                                               const VkAllocationCallbacks* pAllocator,
                                                               const RecordObject& record_obj) {
    FinishReadObjectParentInstance(instance, vvl::Func::vkDestroyDebugUtilsMessengerEXT);
    FinishWriteObjectParentInstance(messenger, vvl::Func::vkDestroyDebugUtilsMessengerEXT);
    DestroyObjectParentInstance(messenger);
    // Host access to messenger must be externally synchronized
}

void ThreadSafety::PreCallRecordSubmitDebugUtilsMessageEXT(VkInstance instance,
                                                           VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                           VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                                                           const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData) {
    StartReadObjectParentInstance(instance, vvl::Func::vkSubmitDebugUtilsMessageEXT);
}

void ThreadSafety::PostCallRecordSubmitDebugUtilsMessageEXT(VkInstance instance,
                                                            VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                            VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                                                            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                            const RecordObject& record_obj) {
    FinishReadObjectParentInstance(instance, vvl::Func::vkSubmitDebugUtilsMessageEXT);
}

#ifdef VK_USE_PLATFORM_ANDROID_KHR
void ThreadSafety::PreCallRecordGetAndroidHardwareBufferPropertiesANDROID(VkDevice device, const struct AHardwareBuffer* buffer,
                                                                          VkAndroidHardwareBufferPropertiesANDROID* pProperties) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetAndroidHardwareBufferPropertiesANDROID);
}

void ThreadSafety::PostCallRecordGetAndroidHardwareBufferPropertiesANDROID(VkDevice device, const struct AHardwareBuffer* buffer,
                                                                           VkAndroidHardwareBufferPropertiesANDROID* pProperties,
                                                                           const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetAndroidHardwareBufferPropertiesANDROID);
}

#endif  // VK_USE_PLATFORM_ANDROID_KHR
#ifdef VK_USE_PLATFORM_ANDROID_KHR
void ThreadSafety::PreCallRecordGetMemoryAndroidHardwareBufferANDROID(VkDevice device,
                                                                      const VkMemoryGetAndroidHardwareBufferInfoANDROID* pInfo,
                                                                      struct AHardwareBuffer** pBuffer) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetMemoryAndroidHardwareBufferANDROID);
}

void ThreadSafety::PostCallRecordGetMemoryAndroidHardwareBufferANDROID(VkDevice device,
                                                                       const VkMemoryGetAndroidHardwareBufferInfoANDROID* pInfo,
                                                                       struct AHardwareBuffer** pBuffer,
                                                                       const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetMemoryAndroidHardwareBufferANDROID);
}

#endif  // VK_USE_PLATFORM_ANDROID_KHR
#ifdef VK_ENABLE_BETA_EXTENSIONS
void ThreadSafety::PreCallRecordCreateExecutionGraphPipelinesAMDX(VkDevice device, VkPipelineCache pipelineCache,
                                                                  uint32_t createInfoCount,
                                                                  const VkExecutionGraphPipelineCreateInfoAMDX* pCreateInfos,
                                                                  const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines) {
    StartReadObjectParentInstance(device, vvl::Func::vkCreateExecutionGraphPipelinesAMDX);
    StartReadObject(pipelineCache, vvl::Func::vkCreateExecutionGraphPipelinesAMDX);
}

void ThreadSafety::PostCallRecordCreateExecutionGraphPipelinesAMDX(VkDevice device, VkPipelineCache pipelineCache,
                                                                   uint32_t createInfoCount,
                                                                   const VkExecutionGraphPipelineCreateInfoAMDX* pCreateInfos,
                                                                   const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                                   const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkCreateExecutionGraphPipelinesAMDX);
    FinishReadObject(pipelineCache, vvl::Func::vkCreateExecutionGraphPipelinesAMDX);
    if (pPipelines) {
        for (uint32_t index = 0; index < createInfoCount; index++) {
            if (!pPipelines[index]) continue;
            CreateObject(pPipelines[index]);
        }
    }
}

#endif  // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
void ThreadSafety::PreCallRecordGetExecutionGraphPipelineScratchSizeAMDX(VkDevice device, VkPipeline executionGraph,
                                                                         VkExecutionGraphPipelineScratchSizeAMDX* pSizeInfo) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetExecutionGraphPipelineScratchSizeAMDX);
    StartReadObject(executionGraph, vvl::Func::vkGetExecutionGraphPipelineScratchSizeAMDX);
}

void ThreadSafety::PostCallRecordGetExecutionGraphPipelineScratchSizeAMDX(VkDevice device, VkPipeline executionGraph,
                                                                          VkExecutionGraphPipelineScratchSizeAMDX* pSizeInfo,
                                                                          const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetExecutionGraphPipelineScratchSizeAMDX);
    FinishReadObject(executionGraph, vvl::Func::vkGetExecutionGraphPipelineScratchSizeAMDX);
}

#endif  // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
void ThreadSafety::PreCallRecordGetExecutionGraphPipelineNodeIndexAMDX(VkDevice device, VkPipeline executionGraph,
                                                                       const VkPipelineShaderStageNodeCreateInfoAMDX* pNodeInfo,
                                                                       uint32_t* pNodeIndex) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetExecutionGraphPipelineNodeIndexAMDX);
    StartReadObject(executionGraph, vvl::Func::vkGetExecutionGraphPipelineNodeIndexAMDX);
}

void ThreadSafety::PostCallRecordGetExecutionGraphPipelineNodeIndexAMDX(VkDevice device, VkPipeline executionGraph,
                                                                        const VkPipelineShaderStageNodeCreateInfoAMDX* pNodeInfo,
                                                                        uint32_t* pNodeIndex, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetExecutionGraphPipelineNodeIndexAMDX);
    FinishReadObject(executionGraph, vvl::Func::vkGetExecutionGraphPipelineNodeIndexAMDX);
}

#endif  // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
void ThreadSafety::PreCallRecordCmdInitializeGraphScratchMemoryAMDX(VkCommandBuffer commandBuffer, VkDeviceAddress scratch) {
    StartReadObject(commandBuffer, vvl::Func::vkCmdInitializeGraphScratchMemoryAMDX);
}

void ThreadSafety::PostCallRecordCmdInitializeGraphScratchMemoryAMDX(VkCommandBuffer commandBuffer, VkDeviceAddress scratch,
                                                                     const RecordObject& record_obj) {
    FinishReadObject(commandBuffer, vvl::Func::vkCmdInitializeGraphScratchMemoryAMDX);
}

#endif  // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
void ThreadSafety::PreCallRecordCmdDispatchGraphAMDX(VkCommandBuffer commandBuffer, VkDeviceAddress scratch,
                                                     const VkDispatchGraphCountInfoAMDX* pCountInfo) {
    StartReadObject(commandBuffer, vvl::Func::vkCmdDispatchGraphAMDX);
}

void ThreadSafety::PostCallRecordCmdDispatchGraphAMDX(VkCommandBuffer commandBuffer, VkDeviceAddress scratch,
                                                      const VkDispatchGraphCountInfoAMDX* pCountInfo,
                                                      const RecordObject& record_obj) {
    FinishReadObject(commandBuffer, vvl::Func::vkCmdDispatchGraphAMDX);
}

#endif  // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
void ThreadSafety::PreCallRecordCmdDispatchGraphIndirectAMDX(VkCommandBuffer commandBuffer, VkDeviceAddress scratch,
                                                             const VkDispatchGraphCountInfoAMDX* pCountInfo) {
    StartReadObject(commandBuffer, vvl::Func::vkCmdDispatchGraphIndirectAMDX);
}

void ThreadSafety::PostCallRecordCmdDispatchGraphIndirectAMDX(VkCommandBuffer commandBuffer, VkDeviceAddress scratch,
                                                              const VkDispatchGraphCountInfoAMDX* pCountInfo,
                                                              const RecordObject& record_obj) {
    FinishReadObject(commandBuffer, vvl::Func::vkCmdDispatchGraphIndirectAMDX);
}

#endif  // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
void ThreadSafety::PreCallRecordCmdDispatchGraphIndirectCountAMDX(VkCommandBuffer commandBuffer, VkDeviceAddress scratch,
                                                                  VkDeviceAddress countInfo) {
    StartReadObject(commandBuffer, vvl::Func::vkCmdDispatchGraphIndirectCountAMDX);
}

void ThreadSafety::PostCallRecordCmdDispatchGraphIndirectCountAMDX(VkCommandBuffer commandBuffer, VkDeviceAddress scratch,
                                                                   VkDeviceAddress countInfo, const RecordObject& record_obj) {
    FinishReadObject(commandBuffer, vvl::Func::vkCmdDispatchGraphIndirectCountAMDX);
}

#endif  // VK_ENABLE_BETA_EXTENSIONS
void ThreadSafety::PreCallRecordCmdSetSampleLocationsEXT(VkCommandBuffer commandBuffer,
                                                         const VkSampleLocationsInfoEXT* pSampleLocationsInfo) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdSetSampleLocationsEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetSampleLocationsEXT(VkCommandBuffer commandBuffer,
                                                          const VkSampleLocationsInfoEXT* pSampleLocationsInfo,
                                                          const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdSetSampleLocationsEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordGetImageDrmFormatModifierPropertiesEXT(VkDevice device, VkImage image,
                                                                       VkImageDrmFormatModifierPropertiesEXT* pProperties) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetImageDrmFormatModifierPropertiesEXT);
    StartReadObject(image, vvl::Func::vkGetImageDrmFormatModifierPropertiesEXT);
}

void ThreadSafety::PostCallRecordGetImageDrmFormatModifierPropertiesEXT(VkDevice device, VkImage image,
                                                                        VkImageDrmFormatModifierPropertiesEXT* pProperties,
                                                                        const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetImageDrmFormatModifierPropertiesEXT);
    FinishReadObject(image, vvl::Func::vkGetImageDrmFormatModifierPropertiesEXT);
}

void ThreadSafety::PreCallRecordCreateValidationCacheEXT(VkDevice device, const VkValidationCacheCreateInfoEXT* pCreateInfo,
                                                         const VkAllocationCallbacks* pAllocator,
                                                         VkValidationCacheEXT* pValidationCache) {
    StartReadObjectParentInstance(device, vvl::Func::vkCreateValidationCacheEXT);
}

void ThreadSafety::PostCallRecordCreateValidationCacheEXT(VkDevice device, const VkValidationCacheCreateInfoEXT* pCreateInfo,
                                                          const VkAllocationCallbacks* pAllocator,
                                                          VkValidationCacheEXT* pValidationCache, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkCreateValidationCacheEXT);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pValidationCache);
    }
}

void ThreadSafety::PreCallRecordDestroyValidationCacheEXT(VkDevice device, VkValidationCacheEXT validationCache,
                                                          const VkAllocationCallbacks* pAllocator) {
    StartReadObjectParentInstance(device, vvl::Func::vkDestroyValidationCacheEXT);
    StartWriteObject(validationCache, vvl::Func::vkDestroyValidationCacheEXT);
    // Host access to validationCache must be externally synchronized
}

void ThreadSafety::PostCallRecordDestroyValidationCacheEXT(VkDevice device, VkValidationCacheEXT validationCache,
                                                           const VkAllocationCallbacks* pAllocator,
                                                           const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkDestroyValidationCacheEXT);
    FinishWriteObject(validationCache, vvl::Func::vkDestroyValidationCacheEXT);
    DestroyObject(validationCache);
    // Host access to validationCache must be externally synchronized
}

void ThreadSafety::PreCallRecordMergeValidationCachesEXT(VkDevice device, VkValidationCacheEXT dstCache, uint32_t srcCacheCount,
                                                         const VkValidationCacheEXT* pSrcCaches) {
    StartReadObjectParentInstance(device, vvl::Func::vkMergeValidationCachesEXT);
    StartWriteObject(dstCache, vvl::Func::vkMergeValidationCachesEXT);

    if (pSrcCaches) {
        for (uint32_t index = 0; index < srcCacheCount; index++) {
            StartReadObject(pSrcCaches[index], vvl::Func::vkMergeValidationCachesEXT);
        }
    }
    // Host access to dstCache must be externally synchronized
}

void ThreadSafety::PostCallRecordMergeValidationCachesEXT(VkDevice device, VkValidationCacheEXT dstCache, uint32_t srcCacheCount,
                                                          const VkValidationCacheEXT* pSrcCaches, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkMergeValidationCachesEXT);
    FinishWriteObject(dstCache, vvl::Func::vkMergeValidationCachesEXT);

    if (pSrcCaches) {
        for (uint32_t index = 0; index < srcCacheCount; index++) {
            FinishReadObject(pSrcCaches[index], vvl::Func::vkMergeValidationCachesEXT);
        }
    }
    // Host access to dstCache must be externally synchronized
}

void ThreadSafety::PreCallRecordGetValidationCacheDataEXT(VkDevice device, VkValidationCacheEXT validationCache, size_t* pDataSize,
                                                          void* pData) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetValidationCacheDataEXT);
    StartReadObject(validationCache, vvl::Func::vkGetValidationCacheDataEXT);
}

void ThreadSafety::PostCallRecordGetValidationCacheDataEXT(VkDevice device, VkValidationCacheEXT validationCache, size_t* pDataSize,
                                                           void* pData, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetValidationCacheDataEXT);
    FinishReadObject(validationCache, vvl::Func::vkGetValidationCacheDataEXT);
}

void ThreadSafety::PreCallRecordCmdBindShadingRateImageNV(VkCommandBuffer commandBuffer, VkImageView imageView,
                                                          VkImageLayout imageLayout) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdBindShadingRateImageNV);
    StartReadObject(imageView, vvl::Func::vkCmdBindShadingRateImageNV);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdBindShadingRateImageNV(VkCommandBuffer commandBuffer, VkImageView imageView,
                                                           VkImageLayout imageLayout, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdBindShadingRateImageNV);
    FinishReadObject(imageView, vvl::Func::vkCmdBindShadingRateImageNV);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetViewportShadingRatePaletteNV(VkCommandBuffer commandBuffer, uint32_t firstViewport,
                                                                   uint32_t viewportCount,
                                                                   const VkShadingRatePaletteNV* pShadingRatePalettes) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdSetViewportShadingRatePaletteNV);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetViewportShadingRatePaletteNV(VkCommandBuffer commandBuffer, uint32_t firstViewport,
                                                                    uint32_t viewportCount,
                                                                    const VkShadingRatePaletteNV* pShadingRatePalettes,
                                                                    const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdSetViewportShadingRatePaletteNV);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetCoarseSampleOrderNV(VkCommandBuffer commandBuffer, VkCoarseSampleOrderTypeNV sampleOrderType,
                                                          uint32_t customSampleOrderCount,
                                                          const VkCoarseSampleOrderCustomNV* pCustomSampleOrders) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdSetCoarseSampleOrderNV);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetCoarseSampleOrderNV(VkCommandBuffer commandBuffer, VkCoarseSampleOrderTypeNV sampleOrderType,
                                                           uint32_t customSampleOrderCount,
                                                           const VkCoarseSampleOrderCustomNV* pCustomSampleOrders,
                                                           const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdSetCoarseSampleOrderNV);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCreateAccelerationStructureNV(VkDevice device,
                                                              const VkAccelerationStructureCreateInfoNV* pCreateInfo,
                                                              const VkAllocationCallbacks* pAllocator,
                                                              VkAccelerationStructureNV* pAccelerationStructure) {
    StartReadObjectParentInstance(device, vvl::Func::vkCreateAccelerationStructureNV);
}

void ThreadSafety::PostCallRecordCreateAccelerationStructureNV(VkDevice device,
                                                               const VkAccelerationStructureCreateInfoNV* pCreateInfo,
                                                               const VkAllocationCallbacks* pAllocator,
                                                               VkAccelerationStructureNV* pAccelerationStructure,
                                                               const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkCreateAccelerationStructureNV);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pAccelerationStructure);
    }
}

void ThreadSafety::PreCallRecordDestroyAccelerationStructureNV(VkDevice device, VkAccelerationStructureNV accelerationStructure,
                                                               const VkAllocationCallbacks* pAllocator) {
    StartReadObjectParentInstance(device, vvl::Func::vkDestroyAccelerationStructureNV);
    StartWriteObject(accelerationStructure, vvl::Func::vkDestroyAccelerationStructureNV);
    // Host access to accelerationStructure must be externally synchronized
}

void ThreadSafety::PostCallRecordDestroyAccelerationStructureNV(VkDevice device, VkAccelerationStructureNV accelerationStructure,
                                                                const VkAllocationCallbacks* pAllocator,
                                                                const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkDestroyAccelerationStructureNV);
    FinishWriteObject(accelerationStructure, vvl::Func::vkDestroyAccelerationStructureNV);
    DestroyObject(accelerationStructure);
    // Host access to accelerationStructure must be externally synchronized
}

void ThreadSafety::PreCallRecordGetAccelerationStructureMemoryRequirementsNV(
    VkDevice device, const VkAccelerationStructureMemoryRequirementsInfoNV* pInfo, VkMemoryRequirements2KHR* pMemoryRequirements) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetAccelerationStructureMemoryRequirementsNV);
}

void ThreadSafety::PostCallRecordGetAccelerationStructureMemoryRequirementsNV(
    VkDevice device, const VkAccelerationStructureMemoryRequirementsInfoNV* pInfo, VkMemoryRequirements2KHR* pMemoryRequirements,
    const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetAccelerationStructureMemoryRequirementsNV);
}

void ThreadSafety::PreCallRecordBindAccelerationStructureMemoryNV(VkDevice device, uint32_t bindInfoCount,
                                                                  const VkBindAccelerationStructureMemoryInfoNV* pBindInfos) {
    StartReadObjectParentInstance(device, vvl::Func::vkBindAccelerationStructureMemoryNV);
}

void ThreadSafety::PostCallRecordBindAccelerationStructureMemoryNV(VkDevice device, uint32_t bindInfoCount,
                                                                   const VkBindAccelerationStructureMemoryInfoNV* pBindInfos,
                                                                   const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkBindAccelerationStructureMemoryNV);
}

void ThreadSafety::PreCallRecordCmdBuildAccelerationStructureNV(VkCommandBuffer commandBuffer,
                                                                const VkAccelerationStructureInfoNV* pInfo, VkBuffer instanceData,
                                                                VkDeviceSize instanceOffset, VkBool32 update,
                                                                VkAccelerationStructureNV dst, VkAccelerationStructureNV src,
                                                                VkBuffer scratch, VkDeviceSize scratchOffset) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdBuildAccelerationStructureNV);
    StartReadObject(instanceData, vvl::Func::vkCmdBuildAccelerationStructureNV);
    StartReadObject(dst, vvl::Func::vkCmdBuildAccelerationStructureNV);
    StartReadObject(src, vvl::Func::vkCmdBuildAccelerationStructureNV);
    StartReadObject(scratch, vvl::Func::vkCmdBuildAccelerationStructureNV);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdBuildAccelerationStructureNV(VkCommandBuffer commandBuffer,
                                                                 const VkAccelerationStructureInfoNV* pInfo, VkBuffer instanceData,
                                                                 VkDeviceSize instanceOffset, VkBool32 update,
                                                                 VkAccelerationStructureNV dst, VkAccelerationStructureNV src,
                                                                 VkBuffer scratch, VkDeviceSize scratchOffset,
                                                                 const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdBuildAccelerationStructureNV);
    FinishReadObject(instanceData, vvl::Func::vkCmdBuildAccelerationStructureNV);
    FinishReadObject(dst, vvl::Func::vkCmdBuildAccelerationStructureNV);
    FinishReadObject(src, vvl::Func::vkCmdBuildAccelerationStructureNV);
    FinishReadObject(scratch, vvl::Func::vkCmdBuildAccelerationStructureNV);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdCopyAccelerationStructureNV(VkCommandBuffer commandBuffer, VkAccelerationStructureNV dst,
                                                               VkAccelerationStructureNV src,
                                                               VkCopyAccelerationStructureModeKHR mode) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdCopyAccelerationStructureNV);
    StartReadObject(dst, vvl::Func::vkCmdCopyAccelerationStructureNV);
    StartReadObject(src, vvl::Func::vkCmdCopyAccelerationStructureNV);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdCopyAccelerationStructureNV(VkCommandBuffer commandBuffer, VkAccelerationStructureNV dst,
                                                                VkAccelerationStructureNV src,
                                                                VkCopyAccelerationStructureModeKHR mode,
                                                                const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdCopyAccelerationStructureNV);
    FinishReadObject(dst, vvl::Func::vkCmdCopyAccelerationStructureNV);
    FinishReadObject(src, vvl::Func::vkCmdCopyAccelerationStructureNV);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdTraceRaysNV(VkCommandBuffer commandBuffer, VkBuffer raygenShaderBindingTableBuffer,
                                               VkDeviceSize raygenShaderBindingOffset, VkBuffer missShaderBindingTableBuffer,
                                               VkDeviceSize missShaderBindingOffset, VkDeviceSize missShaderBindingStride,
                                               VkBuffer hitShaderBindingTableBuffer, VkDeviceSize hitShaderBindingOffset,
                                               VkDeviceSize hitShaderBindingStride, VkBuffer callableShaderBindingTableBuffer,
                                               VkDeviceSize callableShaderBindingOffset, VkDeviceSize callableShaderBindingStride,
                                               uint32_t width, uint32_t height, uint32_t depth) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdTraceRaysNV);
    StartReadObject(raygenShaderBindingTableBuffer, vvl::Func::vkCmdTraceRaysNV);
    StartReadObject(missShaderBindingTableBuffer, vvl::Func::vkCmdTraceRaysNV);
    StartReadObject(hitShaderBindingTableBuffer, vvl::Func::vkCmdTraceRaysNV);
    StartReadObject(callableShaderBindingTableBuffer, vvl::Func::vkCmdTraceRaysNV);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdTraceRaysNV(VkCommandBuffer commandBuffer, VkBuffer raygenShaderBindingTableBuffer,
                                                VkDeviceSize raygenShaderBindingOffset, VkBuffer missShaderBindingTableBuffer,
                                                VkDeviceSize missShaderBindingOffset, VkDeviceSize missShaderBindingStride,
                                                VkBuffer hitShaderBindingTableBuffer, VkDeviceSize hitShaderBindingOffset,
                                                VkDeviceSize hitShaderBindingStride, VkBuffer callableShaderBindingTableBuffer,
                                                VkDeviceSize callableShaderBindingOffset, VkDeviceSize callableShaderBindingStride,
                                                uint32_t width, uint32_t height, uint32_t depth, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdTraceRaysNV);
    FinishReadObject(raygenShaderBindingTableBuffer, vvl::Func::vkCmdTraceRaysNV);
    FinishReadObject(missShaderBindingTableBuffer, vvl::Func::vkCmdTraceRaysNV);
    FinishReadObject(hitShaderBindingTableBuffer, vvl::Func::vkCmdTraceRaysNV);
    FinishReadObject(callableShaderBindingTableBuffer, vvl::Func::vkCmdTraceRaysNV);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCreateRayTracingPipelinesNV(VkDevice device, VkPipelineCache pipelineCache,
                                                            uint32_t createInfoCount,
                                                            const VkRayTracingPipelineCreateInfoNV* pCreateInfos,
                                                            const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines) {
    StartReadObjectParentInstance(device, vvl::Func::vkCreateRayTracingPipelinesNV);
    StartReadObject(pipelineCache, vvl::Func::vkCreateRayTracingPipelinesNV);
}

void ThreadSafety::PostCallRecordCreateRayTracingPipelinesNV(VkDevice device, VkPipelineCache pipelineCache,
                                                             uint32_t createInfoCount,
                                                             const VkRayTracingPipelineCreateInfoNV* pCreateInfos,
                                                             const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                             const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkCreateRayTracingPipelinesNV);
    FinishReadObject(pipelineCache, vvl::Func::vkCreateRayTracingPipelinesNV);
    if (pPipelines) {
        for (uint32_t index = 0; index < createInfoCount; index++) {
            if (!pPipelines[index]) continue;
            CreateObject(pPipelines[index]);
        }
    }
}

void ThreadSafety::PreCallRecordGetRayTracingShaderGroupHandlesKHR(VkDevice device, VkPipeline pipeline, uint32_t firstGroup,
                                                                   uint32_t groupCount, size_t dataSize, void* pData) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetRayTracingShaderGroupHandlesKHR);
    StartReadObject(pipeline, vvl::Func::vkGetRayTracingShaderGroupHandlesKHR);
}

void ThreadSafety::PostCallRecordGetRayTracingShaderGroupHandlesKHR(VkDevice device, VkPipeline pipeline, uint32_t firstGroup,
                                                                    uint32_t groupCount, size_t dataSize, void* pData,
                                                                    const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetRayTracingShaderGroupHandlesKHR);
    FinishReadObject(pipeline, vvl::Func::vkGetRayTracingShaderGroupHandlesKHR);
}

void ThreadSafety::PreCallRecordGetRayTracingShaderGroupHandlesNV(VkDevice device, VkPipeline pipeline, uint32_t firstGroup,
                                                                  uint32_t groupCount, size_t dataSize, void* pData) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetRayTracingShaderGroupHandlesNV);
    StartReadObject(pipeline, vvl::Func::vkGetRayTracingShaderGroupHandlesNV);
}

void ThreadSafety::PostCallRecordGetRayTracingShaderGroupHandlesNV(VkDevice device, VkPipeline pipeline, uint32_t firstGroup,
                                                                   uint32_t groupCount, size_t dataSize, void* pData,
                                                                   const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetRayTracingShaderGroupHandlesNV);
    FinishReadObject(pipeline, vvl::Func::vkGetRayTracingShaderGroupHandlesNV);
}

void ThreadSafety::PreCallRecordGetAccelerationStructureHandleNV(VkDevice device, VkAccelerationStructureNV accelerationStructure,
                                                                 size_t dataSize, void* pData) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetAccelerationStructureHandleNV);
    StartReadObject(accelerationStructure, vvl::Func::vkGetAccelerationStructureHandleNV);
}

void ThreadSafety::PostCallRecordGetAccelerationStructureHandleNV(VkDevice device, VkAccelerationStructureNV accelerationStructure,
                                                                  size_t dataSize, void* pData, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetAccelerationStructureHandleNV);
    FinishReadObject(accelerationStructure, vvl::Func::vkGetAccelerationStructureHandleNV);
}

void ThreadSafety::PreCallRecordCmdWriteAccelerationStructuresPropertiesNV(VkCommandBuffer commandBuffer,
                                                                           uint32_t accelerationStructureCount,
                                                                           const VkAccelerationStructureNV* pAccelerationStructures,
                                                                           VkQueryType queryType, VkQueryPool queryPool,
                                                                           uint32_t firstQuery) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdWriteAccelerationStructuresPropertiesNV);

    if (pAccelerationStructures) {
        for (uint32_t index = 0; index < accelerationStructureCount; index++) {
            StartReadObject(pAccelerationStructures[index], vvl::Func::vkCmdWriteAccelerationStructuresPropertiesNV);
        }
    }
    StartReadObject(queryPool, vvl::Func::vkCmdWriteAccelerationStructuresPropertiesNV);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdWriteAccelerationStructuresPropertiesNV(
    VkCommandBuffer commandBuffer, uint32_t accelerationStructureCount, const VkAccelerationStructureNV* pAccelerationStructures,
    VkQueryType queryType, VkQueryPool queryPool, uint32_t firstQuery, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdWriteAccelerationStructuresPropertiesNV);

    if (pAccelerationStructures) {
        for (uint32_t index = 0; index < accelerationStructureCount; index++) {
            FinishReadObject(pAccelerationStructures[index], vvl::Func::vkCmdWriteAccelerationStructuresPropertiesNV);
        }
    }
    FinishReadObject(queryPool, vvl::Func::vkCmdWriteAccelerationStructuresPropertiesNV);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCompileDeferredNV(VkDevice device, VkPipeline pipeline, uint32_t shader) {
    StartReadObjectParentInstance(device, vvl::Func::vkCompileDeferredNV);
    StartReadObject(pipeline, vvl::Func::vkCompileDeferredNV);
}

void ThreadSafety::PostCallRecordCompileDeferredNV(VkDevice device, VkPipeline pipeline, uint32_t shader,
                                                   const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkCompileDeferredNV);
    FinishReadObject(pipeline, vvl::Func::vkCompileDeferredNV);
}

void ThreadSafety::PreCallRecordGetMemoryHostPointerPropertiesEXT(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType,
                                                                  const void* pHostPointer,
                                                                  VkMemoryHostPointerPropertiesEXT* pMemoryHostPointerProperties) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetMemoryHostPointerPropertiesEXT);
}

void ThreadSafety::PostCallRecordGetMemoryHostPointerPropertiesEXT(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType,
                                                                   const void* pHostPointer,
                                                                   VkMemoryHostPointerPropertiesEXT* pMemoryHostPointerProperties,
                                                                   const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetMemoryHostPointerPropertiesEXT);
}

void ThreadSafety::PreCallRecordCmdWriteBufferMarkerAMD(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage,
                                                        VkBuffer dstBuffer, VkDeviceSize dstOffset, uint32_t marker) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdWriteBufferMarkerAMD);
    StartReadObject(dstBuffer, vvl::Func::vkCmdWriteBufferMarkerAMD);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdWriteBufferMarkerAMD(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage,
                                                         VkBuffer dstBuffer, VkDeviceSize dstOffset, uint32_t marker,
                                                         const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdWriteBufferMarkerAMD);
    FinishReadObject(dstBuffer, vvl::Func::vkCmdWriteBufferMarkerAMD);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordGetCalibratedTimestampsEXT(VkDevice device, uint32_t timestampCount,
                                                           const VkCalibratedTimestampInfoEXT* pTimestampInfos,
                                                           uint64_t* pTimestamps, uint64_t* pMaxDeviation) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetCalibratedTimestampsEXT);
}

void ThreadSafety::PostCallRecordGetCalibratedTimestampsEXT(VkDevice device, uint32_t timestampCount,
                                                            const VkCalibratedTimestampInfoEXT* pTimestampInfos,
                                                            uint64_t* pTimestamps, uint64_t* pMaxDeviation,
                                                            const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetCalibratedTimestampsEXT);
}

void ThreadSafety::PreCallRecordCmdDrawMeshTasksNV(VkCommandBuffer commandBuffer, uint32_t taskCount, uint32_t firstTask) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdDrawMeshTasksNV);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdDrawMeshTasksNV(VkCommandBuffer commandBuffer, uint32_t taskCount, uint32_t firstTask,
                                                    const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdDrawMeshTasksNV);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdDrawMeshTasksIndirectNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                           uint32_t drawCount, uint32_t stride) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdDrawMeshTasksIndirectNV);
    StartReadObject(buffer, vvl::Func::vkCmdDrawMeshTasksIndirectNV);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdDrawMeshTasksIndirectNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                            uint32_t drawCount, uint32_t stride, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdDrawMeshTasksIndirectNV);
    FinishReadObject(buffer, vvl::Func::vkCmdDrawMeshTasksIndirectNV);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdDrawMeshTasksIndirectCountNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                                VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                                uint32_t maxDrawCount, uint32_t stride) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdDrawMeshTasksIndirectCountNV);
    StartReadObject(buffer, vvl::Func::vkCmdDrawMeshTasksIndirectCountNV);
    StartReadObject(countBuffer, vvl::Func::vkCmdDrawMeshTasksIndirectCountNV);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdDrawMeshTasksIndirectCountNV(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                 VkDeviceSize offset, VkBuffer countBuffer,
                                                                 VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                                 uint32_t stride, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdDrawMeshTasksIndirectCountNV);
    FinishReadObject(buffer, vvl::Func::vkCmdDrawMeshTasksIndirectCountNV);
    FinishReadObject(countBuffer, vvl::Func::vkCmdDrawMeshTasksIndirectCountNV);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetExclusiveScissorEnableNV(VkCommandBuffer commandBuffer, uint32_t firstExclusiveScissor,
                                                               uint32_t exclusiveScissorCount,
                                                               const VkBool32* pExclusiveScissorEnables) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdSetExclusiveScissorEnableNV);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetExclusiveScissorEnableNV(VkCommandBuffer commandBuffer, uint32_t firstExclusiveScissor,
                                                                uint32_t exclusiveScissorCount,
                                                                const VkBool32* pExclusiveScissorEnables,
                                                                const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdSetExclusiveScissorEnableNV);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetExclusiveScissorNV(VkCommandBuffer commandBuffer, uint32_t firstExclusiveScissor,
                                                         uint32_t exclusiveScissorCount, const VkRect2D* pExclusiveScissors) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdSetExclusiveScissorNV);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetExclusiveScissorNV(VkCommandBuffer commandBuffer, uint32_t firstExclusiveScissor,
                                                          uint32_t exclusiveScissorCount, const VkRect2D* pExclusiveScissors,
                                                          const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdSetExclusiveScissorNV);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetCheckpointNV(VkCommandBuffer commandBuffer, const void* pCheckpointMarker) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdSetCheckpointNV);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetCheckpointNV(VkCommandBuffer commandBuffer, const void* pCheckpointMarker,
                                                    const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdSetCheckpointNV);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordGetQueueCheckpointDataNV(VkQueue queue, uint32_t* pCheckpointDataCount,
                                                         VkCheckpointDataNV* pCheckpointData) {
    StartReadObject(queue, vvl::Func::vkGetQueueCheckpointDataNV);
}

void ThreadSafety::PostCallRecordGetQueueCheckpointDataNV(VkQueue queue, uint32_t* pCheckpointDataCount,
                                                          VkCheckpointDataNV* pCheckpointData, const RecordObject& record_obj) {
    FinishReadObject(queue, vvl::Func::vkGetQueueCheckpointDataNV);
}

void ThreadSafety::PreCallRecordInitializePerformanceApiINTEL(VkDevice device,
                                                              const VkInitializePerformanceApiInfoINTEL* pInitializeInfo) {
    StartReadObjectParentInstance(device, vvl::Func::vkInitializePerformanceApiINTEL);
}

void ThreadSafety::PostCallRecordInitializePerformanceApiINTEL(VkDevice device,
                                                               const VkInitializePerformanceApiInfoINTEL* pInitializeInfo,
                                                               const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkInitializePerformanceApiINTEL);
}

void ThreadSafety::PreCallRecordUninitializePerformanceApiINTEL(VkDevice device) {
    StartReadObjectParentInstance(device, vvl::Func::vkUninitializePerformanceApiINTEL);
}

void ThreadSafety::PostCallRecordUninitializePerformanceApiINTEL(VkDevice device, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkUninitializePerformanceApiINTEL);
}

void ThreadSafety::PreCallRecordCmdSetPerformanceMarkerINTEL(VkCommandBuffer commandBuffer,
                                                             const VkPerformanceMarkerInfoINTEL* pMarkerInfo) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdSetPerformanceMarkerINTEL);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetPerformanceMarkerINTEL(VkCommandBuffer commandBuffer,
                                                              const VkPerformanceMarkerInfoINTEL* pMarkerInfo,
                                                              const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdSetPerformanceMarkerINTEL);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetPerformanceStreamMarkerINTEL(VkCommandBuffer commandBuffer,
                                                                   const VkPerformanceStreamMarkerInfoINTEL* pMarkerInfo) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdSetPerformanceStreamMarkerINTEL);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetPerformanceStreamMarkerINTEL(VkCommandBuffer commandBuffer,
                                                                    const VkPerformanceStreamMarkerInfoINTEL* pMarkerInfo,
                                                                    const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdSetPerformanceStreamMarkerINTEL);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetPerformanceOverrideINTEL(VkCommandBuffer commandBuffer,
                                                               const VkPerformanceOverrideInfoINTEL* pOverrideInfo) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdSetPerformanceOverrideINTEL);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetPerformanceOverrideINTEL(VkCommandBuffer commandBuffer,
                                                                const VkPerformanceOverrideInfoINTEL* pOverrideInfo,
                                                                const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdSetPerformanceOverrideINTEL);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordAcquirePerformanceConfigurationINTEL(VkDevice device,
                                                                     const VkPerformanceConfigurationAcquireInfoINTEL* pAcquireInfo,
                                                                     VkPerformanceConfigurationINTEL* pConfiguration) {
    StartReadObjectParentInstance(device, vvl::Func::vkAcquirePerformanceConfigurationINTEL);
}

void ThreadSafety::PostCallRecordAcquirePerformanceConfigurationINTEL(
    VkDevice device, const VkPerformanceConfigurationAcquireInfoINTEL* pAcquireInfo,
    VkPerformanceConfigurationINTEL* pConfiguration, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkAcquirePerformanceConfigurationINTEL);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pConfiguration);
    }
}

void ThreadSafety::PreCallRecordReleasePerformanceConfigurationINTEL(VkDevice device,
                                                                     VkPerformanceConfigurationINTEL configuration) {
    StartReadObjectParentInstance(device, vvl::Func::vkReleasePerformanceConfigurationINTEL);
    StartWriteObject(configuration, vvl::Func::vkReleasePerformanceConfigurationINTEL);
    // Host access to configuration must be externally synchronized
}

void ThreadSafety::PostCallRecordReleasePerformanceConfigurationINTEL(VkDevice device,
                                                                      VkPerformanceConfigurationINTEL configuration,
                                                                      const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkReleasePerformanceConfigurationINTEL);
    FinishWriteObject(configuration, vvl::Func::vkReleasePerformanceConfigurationINTEL);
    DestroyObject(configuration);
    // Host access to configuration must be externally synchronized
}

void ThreadSafety::PreCallRecordQueueSetPerformanceConfigurationINTEL(VkQueue queue,
                                                                      VkPerformanceConfigurationINTEL configuration) {
    StartReadObject(queue, vvl::Func::vkQueueSetPerformanceConfigurationINTEL);
    StartReadObject(configuration, vvl::Func::vkQueueSetPerformanceConfigurationINTEL);
}

void ThreadSafety::PostCallRecordQueueSetPerformanceConfigurationINTEL(VkQueue queue, VkPerformanceConfigurationINTEL configuration,
                                                                       const RecordObject& record_obj) {
    FinishReadObject(queue, vvl::Func::vkQueueSetPerformanceConfigurationINTEL);
    FinishReadObject(configuration, vvl::Func::vkQueueSetPerformanceConfigurationINTEL);
}

void ThreadSafety::PreCallRecordGetPerformanceParameterINTEL(VkDevice device, VkPerformanceParameterTypeINTEL parameter,
                                                             VkPerformanceValueINTEL* pValue) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetPerformanceParameterINTEL);
}

void ThreadSafety::PostCallRecordGetPerformanceParameterINTEL(VkDevice device, VkPerformanceParameterTypeINTEL parameter,
                                                              VkPerformanceValueINTEL* pValue, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetPerformanceParameterINTEL);
}

void ThreadSafety::PreCallRecordSetLocalDimmingAMD(VkDevice device, VkSwapchainKHR swapChain, VkBool32 localDimmingEnable) {
    StartReadObjectParentInstance(device, vvl::Func::vkSetLocalDimmingAMD);
    StartReadObject(swapChain, vvl::Func::vkSetLocalDimmingAMD);
}

void ThreadSafety::PostCallRecordSetLocalDimmingAMD(VkDevice device, VkSwapchainKHR swapChain, VkBool32 localDimmingEnable,
                                                    const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkSetLocalDimmingAMD);
    FinishReadObject(swapChain, vvl::Func::vkSetLocalDimmingAMD);
}

#ifdef VK_USE_PLATFORM_FUCHSIA
void ThreadSafety::PreCallRecordCreateImagePipeSurfaceFUCHSIA(VkInstance instance,
                                                              const VkImagePipeSurfaceCreateInfoFUCHSIA* pCreateInfo,
                                                              const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    StartReadObjectParentInstance(instance, vvl::Func::vkCreateImagePipeSurfaceFUCHSIA);
}

void ThreadSafety::PostCallRecordCreateImagePipeSurfaceFUCHSIA(VkInstance instance,
                                                               const VkImagePipeSurfaceCreateInfoFUCHSIA* pCreateInfo,
                                                               const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                               const RecordObject& record_obj) {
    FinishReadObjectParentInstance(instance, vvl::Func::vkCreateImagePipeSurfaceFUCHSIA);
    if (record_obj.result == VK_SUCCESS) {
        CreateObjectParentInstance(*pSurface);
    }
}

#endif  // VK_USE_PLATFORM_FUCHSIA
#ifdef VK_USE_PLATFORM_METAL_EXT
void ThreadSafety::PreCallRecordCreateMetalSurfaceEXT(VkInstance instance, const VkMetalSurfaceCreateInfoEXT* pCreateInfo,
                                                      const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    StartReadObjectParentInstance(instance, vvl::Func::vkCreateMetalSurfaceEXT);
}

void ThreadSafety::PostCallRecordCreateMetalSurfaceEXT(VkInstance instance, const VkMetalSurfaceCreateInfoEXT* pCreateInfo,
                                                       const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                       const RecordObject& record_obj) {
    FinishReadObjectParentInstance(instance, vvl::Func::vkCreateMetalSurfaceEXT);
    if (record_obj.result == VK_SUCCESS) {
        CreateObjectParentInstance(*pSurface);
    }
}

#endif  // VK_USE_PLATFORM_METAL_EXT
void ThreadSafety::PreCallRecordGetBufferDeviceAddressEXT(VkDevice device, const VkBufferDeviceAddressInfo* pInfo) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetBufferDeviceAddressEXT);
}

void ThreadSafety::PostCallRecordGetBufferDeviceAddressEXT(VkDevice device, const VkBufferDeviceAddressInfo* pInfo,
                                                           const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetBufferDeviceAddressEXT);
}

#ifdef VK_USE_PLATFORM_WIN32_KHR
void ThreadSafety::PreCallRecordAcquireFullScreenExclusiveModeEXT(VkDevice device, VkSwapchainKHR swapchain) {
    StartReadObjectParentInstance(device, vvl::Func::vkAcquireFullScreenExclusiveModeEXT);
    StartReadObject(swapchain, vvl::Func::vkAcquireFullScreenExclusiveModeEXT);
}

void ThreadSafety::PostCallRecordAcquireFullScreenExclusiveModeEXT(VkDevice device, VkSwapchainKHR swapchain,
                                                                   const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkAcquireFullScreenExclusiveModeEXT);
    FinishReadObject(swapchain, vvl::Func::vkAcquireFullScreenExclusiveModeEXT);
}

#endif  // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
void ThreadSafety::PreCallRecordReleaseFullScreenExclusiveModeEXT(VkDevice device, VkSwapchainKHR swapchain) {
    StartReadObjectParentInstance(device, vvl::Func::vkReleaseFullScreenExclusiveModeEXT);
    StartReadObject(swapchain, vvl::Func::vkReleaseFullScreenExclusiveModeEXT);
}

void ThreadSafety::PostCallRecordReleaseFullScreenExclusiveModeEXT(VkDevice device, VkSwapchainKHR swapchain,
                                                                   const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkReleaseFullScreenExclusiveModeEXT);
    FinishReadObject(swapchain, vvl::Func::vkReleaseFullScreenExclusiveModeEXT);
}

#endif  // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
void ThreadSafety::PreCallRecordGetDeviceGroupSurfacePresentModes2EXT(VkDevice device,
                                                                      const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo,
                                                                      VkDeviceGroupPresentModeFlagsKHR* pModes) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetDeviceGroupSurfacePresentModes2EXT);
}

void ThreadSafety::PostCallRecordGetDeviceGroupSurfacePresentModes2EXT(VkDevice device,
                                                                       const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo,
                                                                       VkDeviceGroupPresentModeFlagsKHR* pModes,
                                                                       const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetDeviceGroupSurfacePresentModes2EXT);
}

#endif  // VK_USE_PLATFORM_WIN32_KHR
void ThreadSafety::PreCallRecordCreateHeadlessSurfaceEXT(VkInstance instance, const VkHeadlessSurfaceCreateInfoEXT* pCreateInfo,
                                                         const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    StartReadObjectParentInstance(instance, vvl::Func::vkCreateHeadlessSurfaceEXT);
}

void ThreadSafety::PostCallRecordCreateHeadlessSurfaceEXT(VkInstance instance, const VkHeadlessSurfaceCreateInfoEXT* pCreateInfo,
                                                          const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                          const RecordObject& record_obj) {
    FinishReadObjectParentInstance(instance, vvl::Func::vkCreateHeadlessSurfaceEXT);
    if (record_obj.result == VK_SUCCESS) {
        CreateObjectParentInstance(*pSurface);
    }
}

void ThreadSafety::PreCallRecordCmdSetLineStippleEXT(VkCommandBuffer commandBuffer, uint32_t lineStippleFactor,
                                                     uint16_t lineStipplePattern) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdSetLineStippleEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetLineStippleEXT(VkCommandBuffer commandBuffer, uint32_t lineStippleFactor,
                                                      uint16_t lineStipplePattern, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdSetLineStippleEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordResetQueryPoolEXT(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery,
                                                  uint32_t queryCount) {
    StartReadObjectParentInstance(device, vvl::Func::vkResetQueryPoolEXT);
    StartReadObject(queryPool, vvl::Func::vkResetQueryPoolEXT);
}

void ThreadSafety::PostCallRecordResetQueryPoolEXT(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount,
                                                   const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkResetQueryPoolEXT);
    FinishReadObject(queryPool, vvl::Func::vkResetQueryPoolEXT);
}

void ThreadSafety::PreCallRecordCmdSetCullModeEXT(VkCommandBuffer commandBuffer, VkCullModeFlags cullMode) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdSetCullModeEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetCullModeEXT(VkCommandBuffer commandBuffer, VkCullModeFlags cullMode,
                                                   const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdSetCullModeEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetFrontFaceEXT(VkCommandBuffer commandBuffer, VkFrontFace frontFace) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdSetFrontFaceEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetFrontFaceEXT(VkCommandBuffer commandBuffer, VkFrontFace frontFace,
                                                    const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdSetFrontFaceEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetPrimitiveTopologyEXT(VkCommandBuffer commandBuffer, VkPrimitiveTopology primitiveTopology) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdSetPrimitiveTopologyEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetPrimitiveTopologyEXT(VkCommandBuffer commandBuffer, VkPrimitiveTopology primitiveTopology,
                                                            const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdSetPrimitiveTopologyEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetViewportWithCountEXT(VkCommandBuffer commandBuffer, uint32_t viewportCount,
                                                           const VkViewport* pViewports) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdSetViewportWithCountEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetViewportWithCountEXT(VkCommandBuffer commandBuffer, uint32_t viewportCount,
                                                            const VkViewport* pViewports, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdSetViewportWithCountEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetScissorWithCountEXT(VkCommandBuffer commandBuffer, uint32_t scissorCount,
                                                          const VkRect2D* pScissors) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdSetScissorWithCountEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetScissorWithCountEXT(VkCommandBuffer commandBuffer, uint32_t scissorCount,
                                                           const VkRect2D* pScissors, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdSetScissorWithCountEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdBindVertexBuffers2EXT(VkCommandBuffer commandBuffer, uint32_t firstBinding,
                                                         uint32_t bindingCount, const VkBuffer* pBuffers,
                                                         const VkDeviceSize* pOffsets, const VkDeviceSize* pSizes,
                                                         const VkDeviceSize* pStrides) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdBindVertexBuffers2EXT);

    if (pBuffers) {
        for (uint32_t index = 0; index < bindingCount; index++) {
            StartReadObject(pBuffers[index], vvl::Func::vkCmdBindVertexBuffers2EXT);
        }
    }
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdBindVertexBuffers2EXT(VkCommandBuffer commandBuffer, uint32_t firstBinding,
                                                          uint32_t bindingCount, const VkBuffer* pBuffers,
                                                          const VkDeviceSize* pOffsets, const VkDeviceSize* pSizes,
                                                          const VkDeviceSize* pStrides, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdBindVertexBuffers2EXT);

    if (pBuffers) {
        for (uint32_t index = 0; index < bindingCount; index++) {
            FinishReadObject(pBuffers[index], vvl::Func::vkCmdBindVertexBuffers2EXT);
        }
    }
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetDepthTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthTestEnable) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdSetDepthTestEnableEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetDepthTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthTestEnable,
                                                          const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdSetDepthTestEnableEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetDepthWriteEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthWriteEnable) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdSetDepthWriteEnableEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetDepthWriteEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthWriteEnable,
                                                           const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdSetDepthWriteEnableEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetDepthCompareOpEXT(VkCommandBuffer commandBuffer, VkCompareOp depthCompareOp) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdSetDepthCompareOpEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetDepthCompareOpEXT(VkCommandBuffer commandBuffer, VkCompareOp depthCompareOp,
                                                         const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdSetDepthCompareOpEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetDepthBoundsTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthBoundsTestEnable) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdSetDepthBoundsTestEnableEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetDepthBoundsTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthBoundsTestEnable,
                                                                const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdSetDepthBoundsTestEnableEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetStencilTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 stencilTestEnable) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdSetStencilTestEnableEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetStencilTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 stencilTestEnable,
                                                            const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdSetStencilTestEnableEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetStencilOpEXT(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, VkStencilOp failOp,
                                                   VkStencilOp passOp, VkStencilOp depthFailOp, VkCompareOp compareOp) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdSetStencilOpEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetStencilOpEXT(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, VkStencilOp failOp,
                                                    VkStencilOp passOp, VkStencilOp depthFailOp, VkCompareOp compareOp,
                                                    const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdSetStencilOpEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCopyMemoryToImageEXT(VkDevice device, const VkCopyMemoryToImageInfoEXT* pCopyMemoryToImageInfo) {
    StartReadObjectParentInstance(device, vvl::Func::vkCopyMemoryToImageEXT);
}

void ThreadSafety::PostCallRecordCopyMemoryToImageEXT(VkDevice device, const VkCopyMemoryToImageInfoEXT* pCopyMemoryToImageInfo,
                                                      const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkCopyMemoryToImageEXT);
}

void ThreadSafety::PreCallRecordCopyImageToMemoryEXT(VkDevice device, const VkCopyImageToMemoryInfoEXT* pCopyImageToMemoryInfo) {
    StartReadObjectParentInstance(device, vvl::Func::vkCopyImageToMemoryEXT);
}

void ThreadSafety::PostCallRecordCopyImageToMemoryEXT(VkDevice device, const VkCopyImageToMemoryInfoEXT* pCopyImageToMemoryInfo,
                                                      const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkCopyImageToMemoryEXT);
}

void ThreadSafety::PreCallRecordCopyImageToImageEXT(VkDevice device, const VkCopyImageToImageInfoEXT* pCopyImageToImageInfo) {
    StartReadObjectParentInstance(device, vvl::Func::vkCopyImageToImageEXT);
}

void ThreadSafety::PostCallRecordCopyImageToImageEXT(VkDevice device, const VkCopyImageToImageInfoEXT* pCopyImageToImageInfo,
                                                     const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkCopyImageToImageEXT);
}

void ThreadSafety::PreCallRecordTransitionImageLayoutEXT(VkDevice device, uint32_t transitionCount,
                                                         const VkHostImageLayoutTransitionInfoEXT* pTransitions) {
    StartReadObjectParentInstance(device, vvl::Func::vkTransitionImageLayoutEXT);
}

void ThreadSafety::PostCallRecordTransitionImageLayoutEXT(VkDevice device, uint32_t transitionCount,
                                                          const VkHostImageLayoutTransitionInfoEXT* pTransitions,
                                                          const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkTransitionImageLayoutEXT);
}

void ThreadSafety::PreCallRecordGetImageSubresourceLayout2EXT(VkDevice device, VkImage image,
                                                              const VkImageSubresource2KHR* pSubresource,
                                                              VkSubresourceLayout2KHR* pLayout) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetImageSubresourceLayout2EXT);
    StartReadObject(image, vvl::Func::vkGetImageSubresourceLayout2EXT);
}

void ThreadSafety::PostCallRecordGetImageSubresourceLayout2EXT(VkDevice device, VkImage image,
                                                               const VkImageSubresource2KHR* pSubresource,
                                                               VkSubresourceLayout2KHR* pLayout, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetImageSubresourceLayout2EXT);
    FinishReadObject(image, vvl::Func::vkGetImageSubresourceLayout2EXT);
}

void ThreadSafety::PreCallRecordReleaseSwapchainImagesEXT(VkDevice device, const VkReleaseSwapchainImagesInfoEXT* pReleaseInfo) {
    StartReadObjectParentInstance(device, vvl::Func::vkReleaseSwapchainImagesEXT);
}

void ThreadSafety::PostCallRecordReleaseSwapchainImagesEXT(VkDevice device, const VkReleaseSwapchainImagesInfoEXT* pReleaseInfo,
                                                           const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkReleaseSwapchainImagesEXT);
}

void ThreadSafety::PreCallRecordGetGeneratedCommandsMemoryRequirementsNV(VkDevice device,
                                                                         const VkGeneratedCommandsMemoryRequirementsInfoNV* pInfo,
                                                                         VkMemoryRequirements2* pMemoryRequirements) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetGeneratedCommandsMemoryRequirementsNV);
}

void ThreadSafety::PostCallRecordGetGeneratedCommandsMemoryRequirementsNV(VkDevice device,
                                                                          const VkGeneratedCommandsMemoryRequirementsInfoNV* pInfo,
                                                                          VkMemoryRequirements2* pMemoryRequirements,
                                                                          const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetGeneratedCommandsMemoryRequirementsNV);
}

void ThreadSafety::PreCallRecordCmdPreprocessGeneratedCommandsNV(VkCommandBuffer commandBuffer,
                                                                 const VkGeneratedCommandsInfoNV* pGeneratedCommandsInfo) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdPreprocessGeneratedCommandsNV);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdPreprocessGeneratedCommandsNV(VkCommandBuffer commandBuffer,
                                                                  const VkGeneratedCommandsInfoNV* pGeneratedCommandsInfo,
                                                                  const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdPreprocessGeneratedCommandsNV);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdExecuteGeneratedCommandsNV(VkCommandBuffer commandBuffer, VkBool32 isPreprocessed,
                                                              const VkGeneratedCommandsInfoNV* pGeneratedCommandsInfo) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdExecuteGeneratedCommandsNV);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdExecuteGeneratedCommandsNV(VkCommandBuffer commandBuffer, VkBool32 isPreprocessed,
                                                               const VkGeneratedCommandsInfoNV* pGeneratedCommandsInfo,
                                                               const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdExecuteGeneratedCommandsNV);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdBindPipelineShaderGroupNV(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                             VkPipeline pipeline, uint32_t groupIndex) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdBindPipelineShaderGroupNV);
    StartReadObject(pipeline, vvl::Func::vkCmdBindPipelineShaderGroupNV);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdBindPipelineShaderGroupNV(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                              VkPipeline pipeline, uint32_t groupIndex,
                                                              const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdBindPipelineShaderGroupNV);
    FinishReadObject(pipeline, vvl::Func::vkCmdBindPipelineShaderGroupNV);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCreateIndirectCommandsLayoutNV(VkDevice device,
                                                               const VkIndirectCommandsLayoutCreateInfoNV* pCreateInfo,
                                                               const VkAllocationCallbacks* pAllocator,
                                                               VkIndirectCommandsLayoutNV* pIndirectCommandsLayout) {
    StartReadObjectParentInstance(device, vvl::Func::vkCreateIndirectCommandsLayoutNV);
}

void ThreadSafety::PostCallRecordCreateIndirectCommandsLayoutNV(VkDevice device,
                                                                const VkIndirectCommandsLayoutCreateInfoNV* pCreateInfo,
                                                                const VkAllocationCallbacks* pAllocator,
                                                                VkIndirectCommandsLayoutNV* pIndirectCommandsLayout,
                                                                const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkCreateIndirectCommandsLayoutNV);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pIndirectCommandsLayout);
    }
}

void ThreadSafety::PreCallRecordDestroyIndirectCommandsLayoutNV(VkDevice device, VkIndirectCommandsLayoutNV indirectCommandsLayout,
                                                                const VkAllocationCallbacks* pAllocator) {
    StartReadObjectParentInstance(device, vvl::Func::vkDestroyIndirectCommandsLayoutNV);
    StartWriteObject(indirectCommandsLayout, vvl::Func::vkDestroyIndirectCommandsLayoutNV);
    // Host access to indirectCommandsLayout must be externally synchronized
}

void ThreadSafety::PostCallRecordDestroyIndirectCommandsLayoutNV(VkDevice device, VkIndirectCommandsLayoutNV indirectCommandsLayout,
                                                                 const VkAllocationCallbacks* pAllocator,
                                                                 const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkDestroyIndirectCommandsLayoutNV);
    FinishWriteObject(indirectCommandsLayout, vvl::Func::vkDestroyIndirectCommandsLayoutNV);
    DestroyObject(indirectCommandsLayout);
    // Host access to indirectCommandsLayout must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetDepthBias2EXT(VkCommandBuffer commandBuffer, const VkDepthBiasInfoEXT* pDepthBiasInfo) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdSetDepthBias2EXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetDepthBias2EXT(VkCommandBuffer commandBuffer, const VkDepthBiasInfoEXT* pDepthBiasInfo,
                                                     const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdSetDepthBias2EXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordAcquireDrmDisplayEXT(VkPhysicalDevice physicalDevice, int32_t drmFd, VkDisplayKHR display) {
    StartReadObjectParentInstance(display, vvl::Func::vkAcquireDrmDisplayEXT);
}

void ThreadSafety::PostCallRecordAcquireDrmDisplayEXT(VkPhysicalDevice physicalDevice, int32_t drmFd, VkDisplayKHR display,
                                                      const RecordObject& record_obj) {
    FinishReadObjectParentInstance(display, vvl::Func::vkAcquireDrmDisplayEXT);
}

void ThreadSafety::PreCallRecordCreatePrivateDataSlotEXT(VkDevice device, const VkPrivateDataSlotCreateInfo* pCreateInfo,
                                                         const VkAllocationCallbacks* pAllocator,
                                                         VkPrivateDataSlot* pPrivateDataSlot) {
    StartReadObjectParentInstance(device, vvl::Func::vkCreatePrivateDataSlotEXT);
}

void ThreadSafety::PostCallRecordCreatePrivateDataSlotEXT(VkDevice device, const VkPrivateDataSlotCreateInfo* pCreateInfo,
                                                          const VkAllocationCallbacks* pAllocator,
                                                          VkPrivateDataSlot* pPrivateDataSlot, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkCreatePrivateDataSlotEXT);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pPrivateDataSlot);
    }
}

void ThreadSafety::PreCallRecordDestroyPrivateDataSlotEXT(VkDevice device, VkPrivateDataSlot privateDataSlot,
                                                          const VkAllocationCallbacks* pAllocator) {
    StartReadObjectParentInstance(device, vvl::Func::vkDestroyPrivateDataSlotEXT);
    StartWriteObject(privateDataSlot, vvl::Func::vkDestroyPrivateDataSlotEXT);
    // Host access to privateDataSlot must be externally synchronized
}

void ThreadSafety::PostCallRecordDestroyPrivateDataSlotEXT(VkDevice device, VkPrivateDataSlot privateDataSlot,
                                                           const VkAllocationCallbacks* pAllocator,
                                                           const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkDestroyPrivateDataSlotEXT);
    FinishWriteObject(privateDataSlot, vvl::Func::vkDestroyPrivateDataSlotEXT);
    DestroyObject(privateDataSlot);
    // Host access to privateDataSlot must be externally synchronized
}

void ThreadSafety::PreCallRecordSetPrivateDataEXT(VkDevice device, VkObjectType objectType, uint64_t objectHandle,
                                                  VkPrivateDataSlot privateDataSlot, uint64_t data) {
    StartReadObjectParentInstance(device, vvl::Func::vkSetPrivateDataEXT);
    StartReadObject(privateDataSlot, vvl::Func::vkSetPrivateDataEXT);
}

void ThreadSafety::PostCallRecordSetPrivateDataEXT(VkDevice device, VkObjectType objectType, uint64_t objectHandle,
                                                   VkPrivateDataSlot privateDataSlot, uint64_t data,
                                                   const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkSetPrivateDataEXT);
    FinishReadObject(privateDataSlot, vvl::Func::vkSetPrivateDataEXT);
}

void ThreadSafety::PreCallRecordGetPrivateDataEXT(VkDevice device, VkObjectType objectType, uint64_t objectHandle,
                                                  VkPrivateDataSlot privateDataSlot, uint64_t* pData) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetPrivateDataEXT);
    StartReadObject(privateDataSlot, vvl::Func::vkGetPrivateDataEXT);
}

void ThreadSafety::PostCallRecordGetPrivateDataEXT(VkDevice device, VkObjectType objectType, uint64_t objectHandle,
                                                   VkPrivateDataSlot privateDataSlot, uint64_t* pData,
                                                   const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetPrivateDataEXT);
    FinishReadObject(privateDataSlot, vvl::Func::vkGetPrivateDataEXT);
}

#ifdef VK_USE_PLATFORM_METAL_EXT
void ThreadSafety::PreCallRecordExportMetalObjectsEXT(VkDevice device, VkExportMetalObjectsInfoEXT* pMetalObjectsInfo) {
    StartReadObjectParentInstance(device, vvl::Func::vkExportMetalObjectsEXT);
}

void ThreadSafety::PostCallRecordExportMetalObjectsEXT(VkDevice device, VkExportMetalObjectsInfoEXT* pMetalObjectsInfo,
                                                       const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkExportMetalObjectsEXT);
}

#endif  // VK_USE_PLATFORM_METAL_EXT
void ThreadSafety::PreCallRecordGetDescriptorSetLayoutSizeEXT(VkDevice device, VkDescriptorSetLayout layout,
                                                              VkDeviceSize* pLayoutSizeInBytes) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetDescriptorSetLayoutSizeEXT);
    StartReadObject(layout, vvl::Func::vkGetDescriptorSetLayoutSizeEXT);
}

void ThreadSafety::PostCallRecordGetDescriptorSetLayoutSizeEXT(VkDevice device, VkDescriptorSetLayout layout,
                                                               VkDeviceSize* pLayoutSizeInBytes, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetDescriptorSetLayoutSizeEXT);
    FinishReadObject(layout, vvl::Func::vkGetDescriptorSetLayoutSizeEXT);
}

void ThreadSafety::PreCallRecordGetDescriptorSetLayoutBindingOffsetEXT(VkDevice device, VkDescriptorSetLayout layout,
                                                                       uint32_t binding, VkDeviceSize* pOffset) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetDescriptorSetLayoutBindingOffsetEXT);
    StartReadObject(layout, vvl::Func::vkGetDescriptorSetLayoutBindingOffsetEXT);
}

void ThreadSafety::PostCallRecordGetDescriptorSetLayoutBindingOffsetEXT(VkDevice device, VkDescriptorSetLayout layout,
                                                                        uint32_t binding, VkDeviceSize* pOffset,
                                                                        const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetDescriptorSetLayoutBindingOffsetEXT);
    FinishReadObject(layout, vvl::Func::vkGetDescriptorSetLayoutBindingOffsetEXT);
}

void ThreadSafety::PreCallRecordGetDescriptorEXT(VkDevice device, const VkDescriptorGetInfoEXT* pDescriptorInfo, size_t dataSize,
                                                 void* pDescriptor) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetDescriptorEXT);
}

void ThreadSafety::PostCallRecordGetDescriptorEXT(VkDevice device, const VkDescriptorGetInfoEXT* pDescriptorInfo, size_t dataSize,
                                                  void* pDescriptor, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetDescriptorEXT);
}

void ThreadSafety::PreCallRecordCmdBindDescriptorBuffersEXT(VkCommandBuffer commandBuffer, uint32_t bufferCount,
                                                            const VkDescriptorBufferBindingInfoEXT* pBindingInfos) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdBindDescriptorBuffersEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdBindDescriptorBuffersEXT(VkCommandBuffer commandBuffer, uint32_t bufferCount,
                                                             const VkDescriptorBufferBindingInfoEXT* pBindingInfos,
                                                             const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdBindDescriptorBuffersEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetDescriptorBufferOffsetsEXT(VkCommandBuffer commandBuffer,
                                                                 VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout,
                                                                 uint32_t firstSet, uint32_t setCount,
                                                                 const uint32_t* pBufferIndices, const VkDeviceSize* pOffsets) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdSetDescriptorBufferOffsetsEXT);
    StartReadObject(layout, vvl::Func::vkCmdSetDescriptorBufferOffsetsEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetDescriptorBufferOffsetsEXT(VkCommandBuffer commandBuffer,
                                                                  VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout,
                                                                  uint32_t firstSet, uint32_t setCount,
                                                                  const uint32_t* pBufferIndices, const VkDeviceSize* pOffsets,
                                                                  const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdSetDescriptorBufferOffsetsEXT);
    FinishReadObject(layout, vvl::Func::vkCmdSetDescriptorBufferOffsetsEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdBindDescriptorBufferEmbeddedSamplersEXT(VkCommandBuffer commandBuffer,
                                                                           VkPipelineBindPoint pipelineBindPoint,
                                                                           VkPipelineLayout layout, uint32_t set) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdBindDescriptorBufferEmbeddedSamplersEXT);
    StartReadObject(layout, vvl::Func::vkCmdBindDescriptorBufferEmbeddedSamplersEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdBindDescriptorBufferEmbeddedSamplersEXT(VkCommandBuffer commandBuffer,
                                                                            VkPipelineBindPoint pipelineBindPoint,
                                                                            VkPipelineLayout layout, uint32_t set,
                                                                            const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdBindDescriptorBufferEmbeddedSamplersEXT);
    FinishReadObject(layout, vvl::Func::vkCmdBindDescriptorBufferEmbeddedSamplersEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordGetBufferOpaqueCaptureDescriptorDataEXT(VkDevice device,
                                                                        const VkBufferCaptureDescriptorDataInfoEXT* pInfo,
                                                                        void* pData) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetBufferOpaqueCaptureDescriptorDataEXT);
}

void ThreadSafety::PostCallRecordGetBufferOpaqueCaptureDescriptorDataEXT(VkDevice device,
                                                                         const VkBufferCaptureDescriptorDataInfoEXT* pInfo,
                                                                         void* pData, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetBufferOpaqueCaptureDescriptorDataEXT);
}

void ThreadSafety::PreCallRecordGetImageOpaqueCaptureDescriptorDataEXT(VkDevice device,
                                                                       const VkImageCaptureDescriptorDataInfoEXT* pInfo,
                                                                       void* pData) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetImageOpaqueCaptureDescriptorDataEXT);
}

void ThreadSafety::PostCallRecordGetImageOpaqueCaptureDescriptorDataEXT(VkDevice device,
                                                                        const VkImageCaptureDescriptorDataInfoEXT* pInfo,
                                                                        void* pData, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetImageOpaqueCaptureDescriptorDataEXT);
}

void ThreadSafety::PreCallRecordGetImageViewOpaqueCaptureDescriptorDataEXT(VkDevice device,
                                                                           const VkImageViewCaptureDescriptorDataInfoEXT* pInfo,
                                                                           void* pData) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetImageViewOpaqueCaptureDescriptorDataEXT);
}

void ThreadSafety::PostCallRecordGetImageViewOpaqueCaptureDescriptorDataEXT(VkDevice device,
                                                                            const VkImageViewCaptureDescriptorDataInfoEXT* pInfo,
                                                                            void* pData, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetImageViewOpaqueCaptureDescriptorDataEXT);
}

void ThreadSafety::PreCallRecordGetSamplerOpaqueCaptureDescriptorDataEXT(VkDevice device,
                                                                         const VkSamplerCaptureDescriptorDataInfoEXT* pInfo,
                                                                         void* pData) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetSamplerOpaqueCaptureDescriptorDataEXT);
}

void ThreadSafety::PostCallRecordGetSamplerOpaqueCaptureDescriptorDataEXT(VkDevice device,
                                                                          const VkSamplerCaptureDescriptorDataInfoEXT* pInfo,
                                                                          void* pData, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetSamplerOpaqueCaptureDescriptorDataEXT);
}

void ThreadSafety::PreCallRecordGetAccelerationStructureOpaqueCaptureDescriptorDataEXT(
    VkDevice device, const VkAccelerationStructureCaptureDescriptorDataInfoEXT* pInfo, void* pData) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetAccelerationStructureOpaqueCaptureDescriptorDataEXT);
}

void ThreadSafety::PostCallRecordGetAccelerationStructureOpaqueCaptureDescriptorDataEXT(
    VkDevice device, const VkAccelerationStructureCaptureDescriptorDataInfoEXT* pInfo, void* pData,
    const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetAccelerationStructureOpaqueCaptureDescriptorDataEXT);
}

void ThreadSafety::PreCallRecordCmdSetFragmentShadingRateEnumNV(VkCommandBuffer commandBuffer, VkFragmentShadingRateNV shadingRate,
                                                                const VkFragmentShadingRateCombinerOpKHR combinerOps[2]) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdSetFragmentShadingRateEnumNV);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetFragmentShadingRateEnumNV(VkCommandBuffer commandBuffer, VkFragmentShadingRateNV shadingRate,
                                                                 const VkFragmentShadingRateCombinerOpKHR combinerOps[2],
                                                                 const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdSetFragmentShadingRateEnumNV);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordGetDeviceFaultInfoEXT(VkDevice device, VkDeviceFaultCountsEXT* pFaultCounts,
                                                      VkDeviceFaultInfoEXT* pFaultInfo) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetDeviceFaultInfoEXT);
}

void ThreadSafety::PostCallRecordGetDeviceFaultInfoEXT(VkDevice device, VkDeviceFaultCountsEXT* pFaultCounts,
                                                       VkDeviceFaultInfoEXT* pFaultInfo, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetDeviceFaultInfoEXT);
}

#ifdef VK_USE_PLATFORM_WIN32_KHR
void ThreadSafety::PreCallRecordAcquireWinrtDisplayNV(VkPhysicalDevice physicalDevice, VkDisplayKHR display) {
    StartReadObjectParentInstance(display, vvl::Func::vkAcquireWinrtDisplayNV);
}

void ThreadSafety::PostCallRecordAcquireWinrtDisplayNV(VkPhysicalDevice physicalDevice, VkDisplayKHR display,
                                                       const RecordObject& record_obj) {
    FinishReadObjectParentInstance(display, vvl::Func::vkAcquireWinrtDisplayNV);
}

#endif  // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_DIRECTFB_EXT
void ThreadSafety::PreCallRecordCreateDirectFBSurfaceEXT(VkInstance instance, const VkDirectFBSurfaceCreateInfoEXT* pCreateInfo,
                                                         const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    StartReadObjectParentInstance(instance, vvl::Func::vkCreateDirectFBSurfaceEXT);
}

void ThreadSafety::PostCallRecordCreateDirectFBSurfaceEXT(VkInstance instance, const VkDirectFBSurfaceCreateInfoEXT* pCreateInfo,
                                                          const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                          const RecordObject& record_obj) {
    FinishReadObjectParentInstance(instance, vvl::Func::vkCreateDirectFBSurfaceEXT);
    if (record_obj.result == VK_SUCCESS) {
        CreateObjectParentInstance(*pSurface);
    }
}

#endif  // VK_USE_PLATFORM_DIRECTFB_EXT
void ThreadSafety::PreCallRecordCmdSetVertexInputEXT(VkCommandBuffer commandBuffer, uint32_t vertexBindingDescriptionCount,
                                                     const VkVertexInputBindingDescription2EXT* pVertexBindingDescriptions,
                                                     uint32_t vertexAttributeDescriptionCount,
                                                     const VkVertexInputAttributeDescription2EXT* pVertexAttributeDescriptions) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdSetVertexInputEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetVertexInputEXT(VkCommandBuffer commandBuffer, uint32_t vertexBindingDescriptionCount,
                                                      const VkVertexInputBindingDescription2EXT* pVertexBindingDescriptions,
                                                      uint32_t vertexAttributeDescriptionCount,
                                                      const VkVertexInputAttributeDescription2EXT* pVertexAttributeDescriptions,
                                                      const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdSetVertexInputEXT);
    // Host access to commandBuffer must be externally synchronized
}

#ifdef VK_USE_PLATFORM_FUCHSIA
void ThreadSafety::PreCallRecordGetMemoryZirconHandleFUCHSIA(VkDevice device,
                                                             const VkMemoryGetZirconHandleInfoFUCHSIA* pGetZirconHandleInfo,
                                                             zx_handle_t* pZirconHandle) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetMemoryZirconHandleFUCHSIA);
}

void ThreadSafety::PostCallRecordGetMemoryZirconHandleFUCHSIA(VkDevice device,
                                                              const VkMemoryGetZirconHandleInfoFUCHSIA* pGetZirconHandleInfo,
                                                              zx_handle_t* pZirconHandle, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetMemoryZirconHandleFUCHSIA);
}

#endif  // VK_USE_PLATFORM_FUCHSIA
#ifdef VK_USE_PLATFORM_FUCHSIA
void ThreadSafety::PreCallRecordGetMemoryZirconHandlePropertiesFUCHSIA(
    VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType, zx_handle_t zirconHandle,
    VkMemoryZirconHandlePropertiesFUCHSIA* pMemoryZirconHandleProperties) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetMemoryZirconHandlePropertiesFUCHSIA);
}

void ThreadSafety::PostCallRecordGetMemoryZirconHandlePropertiesFUCHSIA(
    VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType, zx_handle_t zirconHandle,
    VkMemoryZirconHandlePropertiesFUCHSIA* pMemoryZirconHandleProperties, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetMemoryZirconHandlePropertiesFUCHSIA);
}

#endif  // VK_USE_PLATFORM_FUCHSIA
#ifdef VK_USE_PLATFORM_FUCHSIA
void ThreadSafety::PreCallRecordImportSemaphoreZirconHandleFUCHSIA(
    VkDevice device, const VkImportSemaphoreZirconHandleInfoFUCHSIA* pImportSemaphoreZirconHandleInfo) {
    StartReadObjectParentInstance(device, vvl::Func::vkImportSemaphoreZirconHandleFUCHSIA);
}

void ThreadSafety::PostCallRecordImportSemaphoreZirconHandleFUCHSIA(
    VkDevice device, const VkImportSemaphoreZirconHandleInfoFUCHSIA* pImportSemaphoreZirconHandleInfo,
    const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkImportSemaphoreZirconHandleFUCHSIA);
}

#endif  // VK_USE_PLATFORM_FUCHSIA
#ifdef VK_USE_PLATFORM_FUCHSIA
void ThreadSafety::PreCallRecordGetSemaphoreZirconHandleFUCHSIA(VkDevice device,
                                                                const VkSemaphoreGetZirconHandleInfoFUCHSIA* pGetZirconHandleInfo,
                                                                zx_handle_t* pZirconHandle) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetSemaphoreZirconHandleFUCHSIA);
}

void ThreadSafety::PostCallRecordGetSemaphoreZirconHandleFUCHSIA(VkDevice device,
                                                                 const VkSemaphoreGetZirconHandleInfoFUCHSIA* pGetZirconHandleInfo,
                                                                 zx_handle_t* pZirconHandle, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetSemaphoreZirconHandleFUCHSIA);
}

#endif  // VK_USE_PLATFORM_FUCHSIA
#ifdef VK_USE_PLATFORM_FUCHSIA
void ThreadSafety::PreCallRecordCreateBufferCollectionFUCHSIA(VkDevice device,
                                                              const VkBufferCollectionCreateInfoFUCHSIA* pCreateInfo,
                                                              const VkAllocationCallbacks* pAllocator,
                                                              VkBufferCollectionFUCHSIA* pCollection) {
    StartReadObjectParentInstance(device, vvl::Func::vkCreateBufferCollectionFUCHSIA);
}

void ThreadSafety::PostCallRecordCreateBufferCollectionFUCHSIA(VkDevice device,
                                                               const VkBufferCollectionCreateInfoFUCHSIA* pCreateInfo,
                                                               const VkAllocationCallbacks* pAllocator,
                                                               VkBufferCollectionFUCHSIA* pCollection,
                                                               const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkCreateBufferCollectionFUCHSIA);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pCollection);
    }
}

#endif  // VK_USE_PLATFORM_FUCHSIA
#ifdef VK_USE_PLATFORM_FUCHSIA
void ThreadSafety::PreCallRecordSetBufferCollectionImageConstraintsFUCHSIA(
    VkDevice device, VkBufferCollectionFUCHSIA collection, const VkImageConstraintsInfoFUCHSIA* pImageConstraintsInfo) {
    StartReadObjectParentInstance(device, vvl::Func::vkSetBufferCollectionImageConstraintsFUCHSIA);
    StartReadObject(collection, vvl::Func::vkSetBufferCollectionImageConstraintsFUCHSIA);
}

void ThreadSafety::PostCallRecordSetBufferCollectionImageConstraintsFUCHSIA(
    VkDevice device, VkBufferCollectionFUCHSIA collection, const VkImageConstraintsInfoFUCHSIA* pImageConstraintsInfo,
    const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkSetBufferCollectionImageConstraintsFUCHSIA);
    FinishReadObject(collection, vvl::Func::vkSetBufferCollectionImageConstraintsFUCHSIA);
}

#endif  // VK_USE_PLATFORM_FUCHSIA
#ifdef VK_USE_PLATFORM_FUCHSIA
void ThreadSafety::PreCallRecordSetBufferCollectionBufferConstraintsFUCHSIA(
    VkDevice device, VkBufferCollectionFUCHSIA collection, const VkBufferConstraintsInfoFUCHSIA* pBufferConstraintsInfo) {
    StartReadObjectParentInstance(device, vvl::Func::vkSetBufferCollectionBufferConstraintsFUCHSIA);
    StartReadObject(collection, vvl::Func::vkSetBufferCollectionBufferConstraintsFUCHSIA);
}

void ThreadSafety::PostCallRecordSetBufferCollectionBufferConstraintsFUCHSIA(
    VkDevice device, VkBufferCollectionFUCHSIA collection, const VkBufferConstraintsInfoFUCHSIA* pBufferConstraintsInfo,
    const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkSetBufferCollectionBufferConstraintsFUCHSIA);
    FinishReadObject(collection, vvl::Func::vkSetBufferCollectionBufferConstraintsFUCHSIA);
}

#endif  // VK_USE_PLATFORM_FUCHSIA
#ifdef VK_USE_PLATFORM_FUCHSIA
void ThreadSafety::PreCallRecordDestroyBufferCollectionFUCHSIA(VkDevice device, VkBufferCollectionFUCHSIA collection,
                                                               const VkAllocationCallbacks* pAllocator) {
    StartReadObjectParentInstance(device, vvl::Func::vkDestroyBufferCollectionFUCHSIA);
    StartReadObject(collection, vvl::Func::vkDestroyBufferCollectionFUCHSIA);
}

void ThreadSafety::PostCallRecordDestroyBufferCollectionFUCHSIA(VkDevice device, VkBufferCollectionFUCHSIA collection,
                                                                const VkAllocationCallbacks* pAllocator,
                                                                const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkDestroyBufferCollectionFUCHSIA);
    FinishReadObject(collection, vvl::Func::vkDestroyBufferCollectionFUCHSIA);
}

#endif  // VK_USE_PLATFORM_FUCHSIA
#ifdef VK_USE_PLATFORM_FUCHSIA
void ThreadSafety::PreCallRecordGetBufferCollectionPropertiesFUCHSIA(VkDevice device, VkBufferCollectionFUCHSIA collection,
                                                                     VkBufferCollectionPropertiesFUCHSIA* pProperties) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetBufferCollectionPropertiesFUCHSIA);
    StartReadObject(collection, vvl::Func::vkGetBufferCollectionPropertiesFUCHSIA);
}

void ThreadSafety::PostCallRecordGetBufferCollectionPropertiesFUCHSIA(VkDevice device, VkBufferCollectionFUCHSIA collection,
                                                                      VkBufferCollectionPropertiesFUCHSIA* pProperties,
                                                                      const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetBufferCollectionPropertiesFUCHSIA);
    FinishReadObject(collection, vvl::Func::vkGetBufferCollectionPropertiesFUCHSIA);
}

#endif  // VK_USE_PLATFORM_FUCHSIA
void ThreadSafety::PreCallRecordGetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI(VkDevice device, VkRenderPass renderpass,
                                                                              VkExtent2D* pMaxWorkgroupSize) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI);
    StartReadObject(renderpass, vvl::Func::vkGetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI);
}

void ThreadSafety::PostCallRecordGetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI(VkDevice device, VkRenderPass renderpass,
                                                                               VkExtent2D* pMaxWorkgroupSize,
                                                                               const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI);
    FinishReadObject(renderpass, vvl::Func::vkGetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI);
}

void ThreadSafety::PreCallRecordCmdSubpassShadingHUAWEI(VkCommandBuffer commandBuffer) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdSubpassShadingHUAWEI);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSubpassShadingHUAWEI(VkCommandBuffer commandBuffer, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdSubpassShadingHUAWEI);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdBindInvocationMaskHUAWEI(VkCommandBuffer commandBuffer, VkImageView imageView,
                                                            VkImageLayout imageLayout) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdBindInvocationMaskHUAWEI);
    StartReadObject(imageView, vvl::Func::vkCmdBindInvocationMaskHUAWEI);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdBindInvocationMaskHUAWEI(VkCommandBuffer commandBuffer, VkImageView imageView,
                                                             VkImageLayout imageLayout, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdBindInvocationMaskHUAWEI);
    FinishReadObject(imageView, vvl::Func::vkCmdBindInvocationMaskHUAWEI);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordGetMemoryRemoteAddressNV(VkDevice device,
                                                         const VkMemoryGetRemoteAddressInfoNV* pMemoryGetRemoteAddressInfo,
                                                         VkRemoteAddressNV* pAddress) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetMemoryRemoteAddressNV);
}

void ThreadSafety::PostCallRecordGetMemoryRemoteAddressNV(VkDevice device,
                                                          const VkMemoryGetRemoteAddressInfoNV* pMemoryGetRemoteAddressInfo,
                                                          VkRemoteAddressNV* pAddress, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetMemoryRemoteAddressNV);
}

void ThreadSafety::PreCallRecordGetPipelinePropertiesEXT(VkDevice device, const VkPipelineInfoEXT* pPipelineInfo,
                                                         VkBaseOutStructure* pPipelineProperties) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetPipelinePropertiesEXT);
}

void ThreadSafety::PostCallRecordGetPipelinePropertiesEXT(VkDevice device, const VkPipelineInfoEXT* pPipelineInfo,
                                                          VkBaseOutStructure* pPipelineProperties, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetPipelinePropertiesEXT);
}

void ThreadSafety::PreCallRecordCmdSetPatchControlPointsEXT(VkCommandBuffer commandBuffer, uint32_t patchControlPoints) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdSetPatchControlPointsEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetPatchControlPointsEXT(VkCommandBuffer commandBuffer, uint32_t patchControlPoints,
                                                             const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdSetPatchControlPointsEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetRasterizerDiscardEnableEXT(VkCommandBuffer commandBuffer, VkBool32 rasterizerDiscardEnable) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdSetRasterizerDiscardEnableEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetRasterizerDiscardEnableEXT(VkCommandBuffer commandBuffer, VkBool32 rasterizerDiscardEnable,
                                                                  const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdSetRasterizerDiscardEnableEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetDepthBiasEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthBiasEnable) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdSetDepthBiasEnableEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetDepthBiasEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthBiasEnable,
                                                          const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdSetDepthBiasEnableEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetLogicOpEXT(VkCommandBuffer commandBuffer, VkLogicOp logicOp) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdSetLogicOpEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetLogicOpEXT(VkCommandBuffer commandBuffer, VkLogicOp logicOp,
                                                  const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdSetLogicOpEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetPrimitiveRestartEnableEXT(VkCommandBuffer commandBuffer, VkBool32 primitiveRestartEnable) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdSetPrimitiveRestartEnableEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetPrimitiveRestartEnableEXT(VkCommandBuffer commandBuffer, VkBool32 primitiveRestartEnable,
                                                                 const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdSetPrimitiveRestartEnableEXT);
    // Host access to commandBuffer must be externally synchronized
}

#ifdef VK_USE_PLATFORM_SCREEN_QNX
void ThreadSafety::PreCallRecordCreateScreenSurfaceQNX(VkInstance instance, const VkScreenSurfaceCreateInfoQNX* pCreateInfo,
                                                       const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    StartReadObjectParentInstance(instance, vvl::Func::vkCreateScreenSurfaceQNX);
}

void ThreadSafety::PostCallRecordCreateScreenSurfaceQNX(VkInstance instance, const VkScreenSurfaceCreateInfoQNX* pCreateInfo,
                                                        const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                        const RecordObject& record_obj) {
    FinishReadObjectParentInstance(instance, vvl::Func::vkCreateScreenSurfaceQNX);
    if (record_obj.result == VK_SUCCESS) {
        CreateObjectParentInstance(*pSurface);
    }
}

#endif  // VK_USE_PLATFORM_SCREEN_QNX
void ThreadSafety::PreCallRecordCmdSetColorWriteEnableEXT(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
                                                          const VkBool32* pColorWriteEnables) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdSetColorWriteEnableEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetColorWriteEnableEXT(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
                                                           const VkBool32* pColorWriteEnables, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdSetColorWriteEnableEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdDrawMultiEXT(VkCommandBuffer commandBuffer, uint32_t drawCount,
                                                const VkMultiDrawInfoEXT* pVertexInfo, uint32_t instanceCount,
                                                uint32_t firstInstance, uint32_t stride) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdDrawMultiEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdDrawMultiEXT(VkCommandBuffer commandBuffer, uint32_t drawCount,
                                                 const VkMultiDrawInfoEXT* pVertexInfo, uint32_t instanceCount,
                                                 uint32_t firstInstance, uint32_t stride, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdDrawMultiEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdDrawMultiIndexedEXT(VkCommandBuffer commandBuffer, uint32_t drawCount,
                                                       const VkMultiDrawIndexedInfoEXT* pIndexInfo, uint32_t instanceCount,
                                                       uint32_t firstInstance, uint32_t stride, const int32_t* pVertexOffset) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdDrawMultiIndexedEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdDrawMultiIndexedEXT(VkCommandBuffer commandBuffer, uint32_t drawCount,
                                                        const VkMultiDrawIndexedInfoEXT* pIndexInfo, uint32_t instanceCount,
                                                        uint32_t firstInstance, uint32_t stride, const int32_t* pVertexOffset,
                                                        const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdDrawMultiIndexedEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCreateMicromapEXT(VkDevice device, const VkMicromapCreateInfoEXT* pCreateInfo,
                                                  const VkAllocationCallbacks* pAllocator, VkMicromapEXT* pMicromap) {
    StartReadObjectParentInstance(device, vvl::Func::vkCreateMicromapEXT);
}

void ThreadSafety::PostCallRecordCreateMicromapEXT(VkDevice device, const VkMicromapCreateInfoEXT* pCreateInfo,
                                                   const VkAllocationCallbacks* pAllocator, VkMicromapEXT* pMicromap,
                                                   const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkCreateMicromapEXT);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pMicromap);
    }
}

void ThreadSafety::PreCallRecordDestroyMicromapEXT(VkDevice device, VkMicromapEXT micromap,
                                                   const VkAllocationCallbacks* pAllocator) {
    StartReadObjectParentInstance(device, vvl::Func::vkDestroyMicromapEXT);
    StartWriteObject(micromap, vvl::Func::vkDestroyMicromapEXT);
    // Host access to micromap must be externally synchronized
}

void ThreadSafety::PostCallRecordDestroyMicromapEXT(VkDevice device, VkMicromapEXT micromap,
                                                    const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkDestroyMicromapEXT);
    FinishWriteObject(micromap, vvl::Func::vkDestroyMicromapEXT);
    DestroyObject(micromap);
    // Host access to micromap must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdBuildMicromapsEXT(VkCommandBuffer commandBuffer, uint32_t infoCount,
                                                     const VkMicromapBuildInfoEXT* pInfos) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdBuildMicromapsEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdBuildMicromapsEXT(VkCommandBuffer commandBuffer, uint32_t infoCount,
                                                      const VkMicromapBuildInfoEXT* pInfos, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdBuildMicromapsEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordBuildMicromapsEXT(VkDevice device, VkDeferredOperationKHR deferredOperation, uint32_t infoCount,
                                                  const VkMicromapBuildInfoEXT* pInfos) {
    StartReadObjectParentInstance(device, vvl::Func::vkBuildMicromapsEXT);
    StartReadObject(deferredOperation, vvl::Func::vkBuildMicromapsEXT);
}

void ThreadSafety::PostCallRecordBuildMicromapsEXT(VkDevice device, VkDeferredOperationKHR deferredOperation, uint32_t infoCount,
                                                   const VkMicromapBuildInfoEXT* pInfos, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkBuildMicromapsEXT);
    FinishReadObject(deferredOperation, vvl::Func::vkBuildMicromapsEXT);
}

void ThreadSafety::PreCallRecordCopyMicromapEXT(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                const VkCopyMicromapInfoEXT* pInfo) {
    StartReadObjectParentInstance(device, vvl::Func::vkCopyMicromapEXT);
    StartReadObject(deferredOperation, vvl::Func::vkCopyMicromapEXT);
}

void ThreadSafety::PostCallRecordCopyMicromapEXT(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                 const VkCopyMicromapInfoEXT* pInfo, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkCopyMicromapEXT);
    FinishReadObject(deferredOperation, vvl::Func::vkCopyMicromapEXT);
}

void ThreadSafety::PreCallRecordCopyMicromapToMemoryEXT(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                        const VkCopyMicromapToMemoryInfoEXT* pInfo) {
    StartReadObjectParentInstance(device, vvl::Func::vkCopyMicromapToMemoryEXT);
    StartReadObject(deferredOperation, vvl::Func::vkCopyMicromapToMemoryEXT);
}

void ThreadSafety::PostCallRecordCopyMicromapToMemoryEXT(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                         const VkCopyMicromapToMemoryInfoEXT* pInfo,
                                                         const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkCopyMicromapToMemoryEXT);
    FinishReadObject(deferredOperation, vvl::Func::vkCopyMicromapToMemoryEXT);
}

void ThreadSafety::PreCallRecordCopyMemoryToMicromapEXT(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                        const VkCopyMemoryToMicromapInfoEXT* pInfo) {
    StartReadObjectParentInstance(device, vvl::Func::vkCopyMemoryToMicromapEXT);
    StartReadObject(deferredOperation, vvl::Func::vkCopyMemoryToMicromapEXT);
}

void ThreadSafety::PostCallRecordCopyMemoryToMicromapEXT(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                         const VkCopyMemoryToMicromapInfoEXT* pInfo,
                                                         const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkCopyMemoryToMicromapEXT);
    FinishReadObject(deferredOperation, vvl::Func::vkCopyMemoryToMicromapEXT);
}

void ThreadSafety::PreCallRecordWriteMicromapsPropertiesEXT(VkDevice device, uint32_t micromapCount,
                                                            const VkMicromapEXT* pMicromaps, VkQueryType queryType, size_t dataSize,
                                                            void* pData, size_t stride) {
    StartReadObjectParentInstance(device, vvl::Func::vkWriteMicromapsPropertiesEXT);

    if (pMicromaps) {
        for (uint32_t index = 0; index < micromapCount; index++) {
            StartReadObject(pMicromaps[index], vvl::Func::vkWriteMicromapsPropertiesEXT);
        }
    }
}

void ThreadSafety::PostCallRecordWriteMicromapsPropertiesEXT(VkDevice device, uint32_t micromapCount,
                                                             const VkMicromapEXT* pMicromaps, VkQueryType queryType,
                                                             size_t dataSize, void* pData, size_t stride,
                                                             const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkWriteMicromapsPropertiesEXT);

    if (pMicromaps) {
        for (uint32_t index = 0; index < micromapCount; index++) {
            FinishReadObject(pMicromaps[index], vvl::Func::vkWriteMicromapsPropertiesEXT);
        }
    }
}

void ThreadSafety::PreCallRecordCmdCopyMicromapEXT(VkCommandBuffer commandBuffer, const VkCopyMicromapInfoEXT* pInfo) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdCopyMicromapEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdCopyMicromapEXT(VkCommandBuffer commandBuffer, const VkCopyMicromapInfoEXT* pInfo,
                                                    const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdCopyMicromapEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdCopyMicromapToMemoryEXT(VkCommandBuffer commandBuffer,
                                                           const VkCopyMicromapToMemoryInfoEXT* pInfo) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdCopyMicromapToMemoryEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdCopyMicromapToMemoryEXT(VkCommandBuffer commandBuffer,
                                                            const VkCopyMicromapToMemoryInfoEXT* pInfo,
                                                            const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdCopyMicromapToMemoryEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdCopyMemoryToMicromapEXT(VkCommandBuffer commandBuffer,
                                                           const VkCopyMemoryToMicromapInfoEXT* pInfo) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdCopyMemoryToMicromapEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdCopyMemoryToMicromapEXT(VkCommandBuffer commandBuffer,
                                                            const VkCopyMemoryToMicromapInfoEXT* pInfo,
                                                            const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdCopyMemoryToMicromapEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdWriteMicromapsPropertiesEXT(VkCommandBuffer commandBuffer, uint32_t micromapCount,
                                                               const VkMicromapEXT* pMicromaps, VkQueryType queryType,
                                                               VkQueryPool queryPool, uint32_t firstQuery) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdWriteMicromapsPropertiesEXT);

    if (pMicromaps) {
        for (uint32_t index = 0; index < micromapCount; index++) {
            StartReadObject(pMicromaps[index], vvl::Func::vkCmdWriteMicromapsPropertiesEXT);
        }
    }
    StartReadObject(queryPool, vvl::Func::vkCmdWriteMicromapsPropertiesEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdWriteMicromapsPropertiesEXT(VkCommandBuffer commandBuffer, uint32_t micromapCount,
                                                                const VkMicromapEXT* pMicromaps, VkQueryType queryType,
                                                                VkQueryPool queryPool, uint32_t firstQuery,
                                                                const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdWriteMicromapsPropertiesEXT);

    if (pMicromaps) {
        for (uint32_t index = 0; index < micromapCount; index++) {
            FinishReadObject(pMicromaps[index], vvl::Func::vkCmdWriteMicromapsPropertiesEXT);
        }
    }
    FinishReadObject(queryPool, vvl::Func::vkCmdWriteMicromapsPropertiesEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordGetDeviceMicromapCompatibilityEXT(VkDevice device, const VkMicromapVersionInfoEXT* pVersionInfo,
                                                                  VkAccelerationStructureCompatibilityKHR* pCompatibility) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetDeviceMicromapCompatibilityEXT);
}

void ThreadSafety::PostCallRecordGetDeviceMicromapCompatibilityEXT(VkDevice device, const VkMicromapVersionInfoEXT* pVersionInfo,
                                                                   VkAccelerationStructureCompatibilityKHR* pCompatibility,
                                                                   const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetDeviceMicromapCompatibilityEXT);
}

void ThreadSafety::PreCallRecordGetMicromapBuildSizesEXT(VkDevice device, VkAccelerationStructureBuildTypeKHR buildType,
                                                         const VkMicromapBuildInfoEXT* pBuildInfo,
                                                         VkMicromapBuildSizesInfoEXT* pSizeInfo) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetMicromapBuildSizesEXT);
}

void ThreadSafety::PostCallRecordGetMicromapBuildSizesEXT(VkDevice device, VkAccelerationStructureBuildTypeKHR buildType,
                                                          const VkMicromapBuildInfoEXT* pBuildInfo,
                                                          VkMicromapBuildSizesInfoEXT* pSizeInfo, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetMicromapBuildSizesEXT);
}

void ThreadSafety::PreCallRecordCmdDrawClusterHUAWEI(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY,
                                                     uint32_t groupCountZ) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdDrawClusterHUAWEI);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdDrawClusterHUAWEI(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY,
                                                      uint32_t groupCountZ, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdDrawClusterHUAWEI);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdDrawClusterIndirectHUAWEI(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdDrawClusterIndirectHUAWEI);
    StartReadObject(buffer, vvl::Func::vkCmdDrawClusterIndirectHUAWEI);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdDrawClusterIndirectHUAWEI(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                              const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdDrawClusterIndirectHUAWEI);
    FinishReadObject(buffer, vvl::Func::vkCmdDrawClusterIndirectHUAWEI);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordSetDeviceMemoryPriorityEXT(VkDevice device, VkDeviceMemory memory, float priority) {
    StartReadObjectParentInstance(device, vvl::Func::vkSetDeviceMemoryPriorityEXT);
    StartReadObject(memory, vvl::Func::vkSetDeviceMemoryPriorityEXT);
}

void ThreadSafety::PostCallRecordSetDeviceMemoryPriorityEXT(VkDevice device, VkDeviceMemory memory, float priority,
                                                            const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkSetDeviceMemoryPriorityEXT);
    FinishReadObject(memory, vvl::Func::vkSetDeviceMemoryPriorityEXT);
}

void ThreadSafety::PreCallRecordGetDescriptorSetLayoutHostMappingInfoVALVE(
    VkDevice device, const VkDescriptorSetBindingReferenceVALVE* pBindingReference,
    VkDescriptorSetLayoutHostMappingInfoVALVE* pHostMapping) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetDescriptorSetLayoutHostMappingInfoVALVE);
}

void ThreadSafety::PostCallRecordGetDescriptorSetLayoutHostMappingInfoVALVE(
    VkDevice device, const VkDescriptorSetBindingReferenceVALVE* pBindingReference,
    VkDescriptorSetLayoutHostMappingInfoVALVE* pHostMapping, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetDescriptorSetLayoutHostMappingInfoVALVE);
}

void ThreadSafety::PreCallRecordGetDescriptorSetHostMappingVALVE(VkDevice device, VkDescriptorSet descriptorSet, void** ppData) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetDescriptorSetHostMappingVALVE);
    StartReadObject(descriptorSet, vvl::Func::vkGetDescriptorSetHostMappingVALVE);
}

void ThreadSafety::PostCallRecordGetDescriptorSetHostMappingVALVE(VkDevice device, VkDescriptorSet descriptorSet, void** ppData,
                                                                  const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetDescriptorSetHostMappingVALVE);
    FinishReadObject(descriptorSet, vvl::Func::vkGetDescriptorSetHostMappingVALVE);
}

void ThreadSafety::PreCallRecordCmdCopyMemoryIndirectNV(VkCommandBuffer commandBuffer, VkDeviceAddress copyBufferAddress,
                                                        uint32_t copyCount, uint32_t stride) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdCopyMemoryIndirectNV);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdCopyMemoryIndirectNV(VkCommandBuffer commandBuffer, VkDeviceAddress copyBufferAddress,
                                                         uint32_t copyCount, uint32_t stride, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdCopyMemoryIndirectNV);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdCopyMemoryToImageIndirectNV(VkCommandBuffer commandBuffer, VkDeviceAddress copyBufferAddress,
                                                               uint32_t copyCount, uint32_t stride, VkImage dstImage,
                                                               VkImageLayout dstImageLayout,
                                                               const VkImageSubresourceLayers* pImageSubresources) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdCopyMemoryToImageIndirectNV);
    StartReadObject(dstImage, vvl::Func::vkCmdCopyMemoryToImageIndirectNV);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdCopyMemoryToImageIndirectNV(VkCommandBuffer commandBuffer, VkDeviceAddress copyBufferAddress,
                                                                uint32_t copyCount, uint32_t stride, VkImage dstImage,
                                                                VkImageLayout dstImageLayout,
                                                                const VkImageSubresourceLayers* pImageSubresources,
                                                                const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdCopyMemoryToImageIndirectNV);
    FinishReadObject(dstImage, vvl::Func::vkCmdCopyMemoryToImageIndirectNV);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdDecompressMemoryNV(VkCommandBuffer commandBuffer, uint32_t decompressRegionCount,
                                                      const VkDecompressMemoryRegionNV* pDecompressMemoryRegions) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdDecompressMemoryNV);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdDecompressMemoryNV(VkCommandBuffer commandBuffer, uint32_t decompressRegionCount,
                                                       const VkDecompressMemoryRegionNV* pDecompressMemoryRegions,
                                                       const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdDecompressMemoryNV);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdDecompressMemoryIndirectCountNV(VkCommandBuffer commandBuffer,
                                                                   VkDeviceAddress indirectCommandsAddress,
                                                                   VkDeviceAddress indirectCommandsCountAddress, uint32_t stride) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdDecompressMemoryIndirectCountNV);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdDecompressMemoryIndirectCountNV(VkCommandBuffer commandBuffer,
                                                                    VkDeviceAddress indirectCommandsAddress,
                                                                    VkDeviceAddress indirectCommandsCountAddress, uint32_t stride,
                                                                    const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdDecompressMemoryIndirectCountNV);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordGetPipelineIndirectMemoryRequirementsNV(VkDevice device,
                                                                        const VkComputePipelineCreateInfo* pCreateInfo,
                                                                        VkMemoryRequirements2* pMemoryRequirements) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetPipelineIndirectMemoryRequirementsNV);
}

void ThreadSafety::PostCallRecordGetPipelineIndirectMemoryRequirementsNV(VkDevice device,
                                                                         const VkComputePipelineCreateInfo* pCreateInfo,
                                                                         VkMemoryRequirements2* pMemoryRequirements,
                                                                         const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetPipelineIndirectMemoryRequirementsNV);
}

void ThreadSafety::PreCallRecordCmdUpdatePipelineIndirectBufferNV(VkCommandBuffer commandBuffer,
                                                                  VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdUpdatePipelineIndirectBufferNV);
    StartReadObject(pipeline, vvl::Func::vkCmdUpdatePipelineIndirectBufferNV);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdUpdatePipelineIndirectBufferNV(VkCommandBuffer commandBuffer,
                                                                   VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline,
                                                                   const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdUpdatePipelineIndirectBufferNV);
    FinishReadObject(pipeline, vvl::Func::vkCmdUpdatePipelineIndirectBufferNV);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordGetPipelineIndirectDeviceAddressNV(VkDevice device,
                                                                   const VkPipelineIndirectDeviceAddressInfoNV* pInfo) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetPipelineIndirectDeviceAddressNV);
}

void ThreadSafety::PostCallRecordGetPipelineIndirectDeviceAddressNV(VkDevice device,
                                                                    const VkPipelineIndirectDeviceAddressInfoNV* pInfo,
                                                                    const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetPipelineIndirectDeviceAddressNV);
}

void ThreadSafety::PreCallRecordCmdSetTessellationDomainOriginEXT(VkCommandBuffer commandBuffer,
                                                                  VkTessellationDomainOrigin domainOrigin) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdSetTessellationDomainOriginEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetTessellationDomainOriginEXT(VkCommandBuffer commandBuffer,
                                                                   VkTessellationDomainOrigin domainOrigin,
                                                                   const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdSetTessellationDomainOriginEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetDepthClampEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthClampEnable) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdSetDepthClampEnableEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetDepthClampEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthClampEnable,
                                                           const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdSetDepthClampEnableEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetPolygonModeEXT(VkCommandBuffer commandBuffer, VkPolygonMode polygonMode) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdSetPolygonModeEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetPolygonModeEXT(VkCommandBuffer commandBuffer, VkPolygonMode polygonMode,
                                                      const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdSetPolygonModeEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetRasterizationSamplesEXT(VkCommandBuffer commandBuffer,
                                                              VkSampleCountFlagBits rasterizationSamples) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdSetRasterizationSamplesEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetRasterizationSamplesEXT(VkCommandBuffer commandBuffer,
                                                               VkSampleCountFlagBits rasterizationSamples,
                                                               const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdSetRasterizationSamplesEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetSampleMaskEXT(VkCommandBuffer commandBuffer, VkSampleCountFlagBits samples,
                                                    const VkSampleMask* pSampleMask) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdSetSampleMaskEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetSampleMaskEXT(VkCommandBuffer commandBuffer, VkSampleCountFlagBits samples,
                                                     const VkSampleMask* pSampleMask, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdSetSampleMaskEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetAlphaToCoverageEnableEXT(VkCommandBuffer commandBuffer, VkBool32 alphaToCoverageEnable) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdSetAlphaToCoverageEnableEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetAlphaToCoverageEnableEXT(VkCommandBuffer commandBuffer, VkBool32 alphaToCoverageEnable,
                                                                const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdSetAlphaToCoverageEnableEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetAlphaToOneEnableEXT(VkCommandBuffer commandBuffer, VkBool32 alphaToOneEnable) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdSetAlphaToOneEnableEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetAlphaToOneEnableEXT(VkCommandBuffer commandBuffer, VkBool32 alphaToOneEnable,
                                                           const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdSetAlphaToOneEnableEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetLogicOpEnableEXT(VkCommandBuffer commandBuffer, VkBool32 logicOpEnable) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdSetLogicOpEnableEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetLogicOpEnableEXT(VkCommandBuffer commandBuffer, VkBool32 logicOpEnable,
                                                        const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdSetLogicOpEnableEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetColorBlendEnableEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment,
                                                          uint32_t attachmentCount, const VkBool32* pColorBlendEnables) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdSetColorBlendEnableEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetColorBlendEnableEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment,
                                                           uint32_t attachmentCount, const VkBool32* pColorBlendEnables,
                                                           const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdSetColorBlendEnableEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetColorBlendEquationEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment,
                                                            uint32_t attachmentCount,
                                                            const VkColorBlendEquationEXT* pColorBlendEquations) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdSetColorBlendEquationEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetColorBlendEquationEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment,
                                                             uint32_t attachmentCount,
                                                             const VkColorBlendEquationEXT* pColorBlendEquations,
                                                             const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdSetColorBlendEquationEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetColorWriteMaskEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment,
                                                        uint32_t attachmentCount, const VkColorComponentFlags* pColorWriteMasks) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdSetColorWriteMaskEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetColorWriteMaskEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment,
                                                         uint32_t attachmentCount, const VkColorComponentFlags* pColorWriteMasks,
                                                         const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdSetColorWriteMaskEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetRasterizationStreamEXT(VkCommandBuffer commandBuffer, uint32_t rasterizationStream) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdSetRasterizationStreamEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetRasterizationStreamEXT(VkCommandBuffer commandBuffer, uint32_t rasterizationStream,
                                                              const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdSetRasterizationStreamEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetConservativeRasterizationModeEXT(
    VkCommandBuffer commandBuffer, VkConservativeRasterizationModeEXT conservativeRasterizationMode) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdSetConservativeRasterizationModeEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetConservativeRasterizationModeEXT(
    VkCommandBuffer commandBuffer, VkConservativeRasterizationModeEXT conservativeRasterizationMode,
    const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdSetConservativeRasterizationModeEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetExtraPrimitiveOverestimationSizeEXT(VkCommandBuffer commandBuffer,
                                                                          float extraPrimitiveOverestimationSize) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdSetExtraPrimitiveOverestimationSizeEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetExtraPrimitiveOverestimationSizeEXT(VkCommandBuffer commandBuffer,
                                                                           float extraPrimitiveOverestimationSize,
                                                                           const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdSetExtraPrimitiveOverestimationSizeEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetDepthClipEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthClipEnable) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdSetDepthClipEnableEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetDepthClipEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthClipEnable,
                                                          const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdSetDepthClipEnableEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetSampleLocationsEnableEXT(VkCommandBuffer commandBuffer, VkBool32 sampleLocationsEnable) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdSetSampleLocationsEnableEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetSampleLocationsEnableEXT(VkCommandBuffer commandBuffer, VkBool32 sampleLocationsEnable,
                                                                const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdSetSampleLocationsEnableEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetColorBlendAdvancedEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment,
                                                            uint32_t attachmentCount,
                                                            const VkColorBlendAdvancedEXT* pColorBlendAdvanced) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdSetColorBlendAdvancedEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetColorBlendAdvancedEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment,
                                                             uint32_t attachmentCount,
                                                             const VkColorBlendAdvancedEXT* pColorBlendAdvanced,
                                                             const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdSetColorBlendAdvancedEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetProvokingVertexModeEXT(VkCommandBuffer commandBuffer,
                                                             VkProvokingVertexModeEXT provokingVertexMode) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdSetProvokingVertexModeEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetProvokingVertexModeEXT(VkCommandBuffer commandBuffer,
                                                              VkProvokingVertexModeEXT provokingVertexMode,
                                                              const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdSetProvokingVertexModeEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetLineRasterizationModeEXT(VkCommandBuffer commandBuffer,
                                                               VkLineRasterizationModeEXT lineRasterizationMode) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdSetLineRasterizationModeEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetLineRasterizationModeEXT(VkCommandBuffer commandBuffer,
                                                                VkLineRasterizationModeEXT lineRasterizationMode,
                                                                const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdSetLineRasterizationModeEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetLineStippleEnableEXT(VkCommandBuffer commandBuffer, VkBool32 stippledLineEnable) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdSetLineStippleEnableEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetLineStippleEnableEXT(VkCommandBuffer commandBuffer, VkBool32 stippledLineEnable,
                                                            const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdSetLineStippleEnableEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetDepthClipNegativeOneToOneEXT(VkCommandBuffer commandBuffer, VkBool32 negativeOneToOne) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdSetDepthClipNegativeOneToOneEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetDepthClipNegativeOneToOneEXT(VkCommandBuffer commandBuffer, VkBool32 negativeOneToOne,
                                                                    const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdSetDepthClipNegativeOneToOneEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetViewportWScalingEnableNV(VkCommandBuffer commandBuffer, VkBool32 viewportWScalingEnable) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdSetViewportWScalingEnableNV);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetViewportWScalingEnableNV(VkCommandBuffer commandBuffer, VkBool32 viewportWScalingEnable,
                                                                const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdSetViewportWScalingEnableNV);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetViewportSwizzleNV(VkCommandBuffer commandBuffer, uint32_t firstViewport,
                                                        uint32_t viewportCount, const VkViewportSwizzleNV* pViewportSwizzles) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdSetViewportSwizzleNV);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetViewportSwizzleNV(VkCommandBuffer commandBuffer, uint32_t firstViewport,
                                                         uint32_t viewportCount, const VkViewportSwizzleNV* pViewportSwizzles,
                                                         const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdSetViewportSwizzleNV);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetCoverageToColorEnableNV(VkCommandBuffer commandBuffer, VkBool32 coverageToColorEnable) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdSetCoverageToColorEnableNV);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetCoverageToColorEnableNV(VkCommandBuffer commandBuffer, VkBool32 coverageToColorEnable,
                                                               const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdSetCoverageToColorEnableNV);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetCoverageToColorLocationNV(VkCommandBuffer commandBuffer, uint32_t coverageToColorLocation) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdSetCoverageToColorLocationNV);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetCoverageToColorLocationNV(VkCommandBuffer commandBuffer, uint32_t coverageToColorLocation,
                                                                 const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdSetCoverageToColorLocationNV);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetCoverageModulationModeNV(VkCommandBuffer commandBuffer,
                                                               VkCoverageModulationModeNV coverageModulationMode) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdSetCoverageModulationModeNV);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetCoverageModulationModeNV(VkCommandBuffer commandBuffer,
                                                                VkCoverageModulationModeNV coverageModulationMode,
                                                                const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdSetCoverageModulationModeNV);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetCoverageModulationTableEnableNV(VkCommandBuffer commandBuffer,
                                                                      VkBool32 coverageModulationTableEnable) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdSetCoverageModulationTableEnableNV);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetCoverageModulationTableEnableNV(VkCommandBuffer commandBuffer,
                                                                       VkBool32 coverageModulationTableEnable,
                                                                       const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdSetCoverageModulationTableEnableNV);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetCoverageModulationTableNV(VkCommandBuffer commandBuffer,
                                                                uint32_t coverageModulationTableCount,
                                                                const float* pCoverageModulationTable) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdSetCoverageModulationTableNV);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetCoverageModulationTableNV(VkCommandBuffer commandBuffer,
                                                                 uint32_t coverageModulationTableCount,
                                                                 const float* pCoverageModulationTable,
                                                                 const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdSetCoverageModulationTableNV);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetShadingRateImageEnableNV(VkCommandBuffer commandBuffer, VkBool32 shadingRateImageEnable) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdSetShadingRateImageEnableNV);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetShadingRateImageEnableNV(VkCommandBuffer commandBuffer, VkBool32 shadingRateImageEnable,
                                                                const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdSetShadingRateImageEnableNV);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetRepresentativeFragmentTestEnableNV(VkCommandBuffer commandBuffer,
                                                                         VkBool32 representativeFragmentTestEnable) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdSetRepresentativeFragmentTestEnableNV);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetRepresentativeFragmentTestEnableNV(VkCommandBuffer commandBuffer,
                                                                          VkBool32 representativeFragmentTestEnable,
                                                                          const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdSetRepresentativeFragmentTestEnableNV);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetCoverageReductionModeNV(VkCommandBuffer commandBuffer,
                                                              VkCoverageReductionModeNV coverageReductionMode) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdSetCoverageReductionModeNV);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetCoverageReductionModeNV(VkCommandBuffer commandBuffer,
                                                               VkCoverageReductionModeNV coverageReductionMode,
                                                               const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdSetCoverageReductionModeNV);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordGetShaderModuleIdentifierEXT(VkDevice device, VkShaderModule shaderModule,
                                                             VkShaderModuleIdentifierEXT* pIdentifier) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetShaderModuleIdentifierEXT);
    StartReadObject(shaderModule, vvl::Func::vkGetShaderModuleIdentifierEXT);
}

void ThreadSafety::PostCallRecordGetShaderModuleIdentifierEXT(VkDevice device, VkShaderModule shaderModule,
                                                              VkShaderModuleIdentifierEXT* pIdentifier,
                                                              const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetShaderModuleIdentifierEXT);
    FinishReadObject(shaderModule, vvl::Func::vkGetShaderModuleIdentifierEXT);
}

void ThreadSafety::PreCallRecordGetShaderModuleCreateInfoIdentifierEXT(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo,
                                                                       VkShaderModuleIdentifierEXT* pIdentifier) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetShaderModuleCreateInfoIdentifierEXT);
}

void ThreadSafety::PostCallRecordGetShaderModuleCreateInfoIdentifierEXT(VkDevice device,
                                                                        const VkShaderModuleCreateInfo* pCreateInfo,
                                                                        VkShaderModuleIdentifierEXT* pIdentifier,
                                                                        const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetShaderModuleCreateInfoIdentifierEXT);
}

void ThreadSafety::PreCallRecordCreateOpticalFlowSessionNV(VkDevice device, const VkOpticalFlowSessionCreateInfoNV* pCreateInfo,
                                                           const VkAllocationCallbacks* pAllocator,
                                                           VkOpticalFlowSessionNV* pSession) {
    StartReadObjectParentInstance(device, vvl::Func::vkCreateOpticalFlowSessionNV);
}

void ThreadSafety::PostCallRecordCreateOpticalFlowSessionNV(VkDevice device, const VkOpticalFlowSessionCreateInfoNV* pCreateInfo,
                                                            const VkAllocationCallbacks* pAllocator,
                                                            VkOpticalFlowSessionNV* pSession, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkCreateOpticalFlowSessionNV);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pSession);
    }
}

void ThreadSafety::PreCallRecordDestroyOpticalFlowSessionNV(VkDevice device, VkOpticalFlowSessionNV session,
                                                            const VkAllocationCallbacks* pAllocator) {
    StartReadObjectParentInstance(device, vvl::Func::vkDestroyOpticalFlowSessionNV);
    StartReadObject(session, vvl::Func::vkDestroyOpticalFlowSessionNV);
}

void ThreadSafety::PostCallRecordDestroyOpticalFlowSessionNV(VkDevice device, VkOpticalFlowSessionNV session,
                                                             const VkAllocationCallbacks* pAllocator,
                                                             const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkDestroyOpticalFlowSessionNV);
    FinishReadObject(session, vvl::Func::vkDestroyOpticalFlowSessionNV);
}

void ThreadSafety::PreCallRecordBindOpticalFlowSessionImageNV(VkDevice device, VkOpticalFlowSessionNV session,
                                                              VkOpticalFlowSessionBindingPointNV bindingPoint, VkImageView view,
                                                              VkImageLayout layout) {
    StartReadObjectParentInstance(device, vvl::Func::vkBindOpticalFlowSessionImageNV);
    StartReadObject(session, vvl::Func::vkBindOpticalFlowSessionImageNV);
    StartReadObject(view, vvl::Func::vkBindOpticalFlowSessionImageNV);
}

void ThreadSafety::PostCallRecordBindOpticalFlowSessionImageNV(VkDevice device, VkOpticalFlowSessionNV session,
                                                               VkOpticalFlowSessionBindingPointNV bindingPoint, VkImageView view,
                                                               VkImageLayout layout, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkBindOpticalFlowSessionImageNV);
    FinishReadObject(session, vvl::Func::vkBindOpticalFlowSessionImageNV);
    FinishReadObject(view, vvl::Func::vkBindOpticalFlowSessionImageNV);
}

void ThreadSafety::PreCallRecordCmdOpticalFlowExecuteNV(VkCommandBuffer commandBuffer, VkOpticalFlowSessionNV session,
                                                        const VkOpticalFlowExecuteInfoNV* pExecuteInfo) {
    StartReadObject(commandBuffer, vvl::Func::vkCmdOpticalFlowExecuteNV);
    StartReadObject(session, vvl::Func::vkCmdOpticalFlowExecuteNV);
}

void ThreadSafety::PostCallRecordCmdOpticalFlowExecuteNV(VkCommandBuffer commandBuffer, VkOpticalFlowSessionNV session,
                                                         const VkOpticalFlowExecuteInfoNV* pExecuteInfo,
                                                         const RecordObject& record_obj) {
    FinishReadObject(commandBuffer, vvl::Func::vkCmdOpticalFlowExecuteNV);
    FinishReadObject(session, vvl::Func::vkCmdOpticalFlowExecuteNV);
}

void ThreadSafety::PreCallRecordCreateShadersEXT(VkDevice device, uint32_t createInfoCount,
                                                 const VkShaderCreateInfoEXT* pCreateInfos, const VkAllocationCallbacks* pAllocator,
                                                 VkShaderEXT* pShaders) {
    StartReadObjectParentInstance(device, vvl::Func::vkCreateShadersEXT);
}

void ThreadSafety::PostCallRecordCreateShadersEXT(VkDevice device, uint32_t createInfoCount,
                                                  const VkShaderCreateInfoEXT* pCreateInfos,
                                                  const VkAllocationCallbacks* pAllocator, VkShaderEXT* pShaders,
                                                  const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkCreateShadersEXT);
    if (pShaders) {
        for (uint32_t index = 0; index < createInfoCount; index++) {
            if (!pShaders[index]) continue;
            CreateObject(pShaders[index]);
        }
    }
}

void ThreadSafety::PreCallRecordDestroyShaderEXT(VkDevice device, VkShaderEXT shader, const VkAllocationCallbacks* pAllocator) {
    StartReadObjectParentInstance(device, vvl::Func::vkDestroyShaderEXT);
    StartWriteObject(shader, vvl::Func::vkDestroyShaderEXT);
    // Host access to shader must be externally synchronized
}

void ThreadSafety::PostCallRecordDestroyShaderEXT(VkDevice device, VkShaderEXT shader, const VkAllocationCallbacks* pAllocator,
                                                  const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkDestroyShaderEXT);
    FinishWriteObject(shader, vvl::Func::vkDestroyShaderEXT);
    DestroyObject(shader);
    // Host access to shader must be externally synchronized
}

void ThreadSafety::PreCallRecordGetShaderBinaryDataEXT(VkDevice device, VkShaderEXT shader, size_t* pDataSize, void* pData) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetShaderBinaryDataEXT);
    StartReadObject(shader, vvl::Func::vkGetShaderBinaryDataEXT);
}

void ThreadSafety::PostCallRecordGetShaderBinaryDataEXT(VkDevice device, VkShaderEXT shader, size_t* pDataSize, void* pData,
                                                        const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetShaderBinaryDataEXT);
    FinishReadObject(shader, vvl::Func::vkGetShaderBinaryDataEXT);
}

void ThreadSafety::PreCallRecordCmdBindShadersEXT(VkCommandBuffer commandBuffer, uint32_t stageCount,
                                                  const VkShaderStageFlagBits* pStages, const VkShaderEXT* pShaders) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdBindShadersEXT);

    if (pShaders) {
        for (uint32_t index = 0; index < stageCount; index++) {
            StartReadObject(pShaders[index], vvl::Func::vkCmdBindShadersEXT);
        }
    }
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdBindShadersEXT(VkCommandBuffer commandBuffer, uint32_t stageCount,
                                                   const VkShaderStageFlagBits* pStages, const VkShaderEXT* pShaders,
                                                   const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdBindShadersEXT);

    if (pShaders) {
        for (uint32_t index = 0; index < stageCount; index++) {
            FinishReadObject(pShaders[index], vvl::Func::vkCmdBindShadersEXT);
        }
    }
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordGetFramebufferTilePropertiesQCOM(VkDevice device, VkFramebuffer framebuffer,
                                                                 uint32_t* pPropertiesCount, VkTilePropertiesQCOM* pProperties) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetFramebufferTilePropertiesQCOM);
    StartReadObject(framebuffer, vvl::Func::vkGetFramebufferTilePropertiesQCOM);
}

void ThreadSafety::PostCallRecordGetFramebufferTilePropertiesQCOM(VkDevice device, VkFramebuffer framebuffer,
                                                                  uint32_t* pPropertiesCount, VkTilePropertiesQCOM* pProperties,
                                                                  const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetFramebufferTilePropertiesQCOM);
    FinishReadObject(framebuffer, vvl::Func::vkGetFramebufferTilePropertiesQCOM);
}

void ThreadSafety::PreCallRecordGetDynamicRenderingTilePropertiesQCOM(VkDevice device, const VkRenderingInfo* pRenderingInfo,
                                                                      VkTilePropertiesQCOM* pProperties) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetDynamicRenderingTilePropertiesQCOM);
}

void ThreadSafety::PostCallRecordGetDynamicRenderingTilePropertiesQCOM(VkDevice device, const VkRenderingInfo* pRenderingInfo,
                                                                       VkTilePropertiesQCOM* pProperties,
                                                                       const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetDynamicRenderingTilePropertiesQCOM);
}

void ThreadSafety::PreCallRecordSetLatencySleepModeNV(VkDevice device, VkSwapchainKHR swapchain,
                                                      const VkLatencySleepModeInfoNV* pSleepModeInfo) {
    StartReadObjectParentInstance(device, vvl::Func::vkSetLatencySleepModeNV);
    StartReadObject(swapchain, vvl::Func::vkSetLatencySleepModeNV);
}

void ThreadSafety::PostCallRecordSetLatencySleepModeNV(VkDevice device, VkSwapchainKHR swapchain,
                                                       const VkLatencySleepModeInfoNV* pSleepModeInfo,
                                                       const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkSetLatencySleepModeNV);
    FinishReadObject(swapchain, vvl::Func::vkSetLatencySleepModeNV);
}

void ThreadSafety::PreCallRecordLatencySleepNV(VkDevice device, VkSwapchainKHR swapchain, const VkLatencySleepInfoNV* pSleepInfo) {
    StartReadObjectParentInstance(device, vvl::Func::vkLatencySleepNV);
    StartReadObject(swapchain, vvl::Func::vkLatencySleepNV);
}

void ThreadSafety::PostCallRecordLatencySleepNV(VkDevice device, VkSwapchainKHR swapchain, const VkLatencySleepInfoNV* pSleepInfo,
                                                const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkLatencySleepNV);
    FinishReadObject(swapchain, vvl::Func::vkLatencySleepNV);
}

void ThreadSafety::PreCallRecordSetLatencyMarkerNV(VkDevice device, VkSwapchainKHR swapchain,
                                                   const VkSetLatencyMarkerInfoNV* pLatencyMarkerInfo) {
    StartReadObjectParentInstance(device, vvl::Func::vkSetLatencyMarkerNV);
    StartReadObject(swapchain, vvl::Func::vkSetLatencyMarkerNV);
}

void ThreadSafety::PostCallRecordSetLatencyMarkerNV(VkDevice device, VkSwapchainKHR swapchain,
                                                    const VkSetLatencyMarkerInfoNV* pLatencyMarkerInfo,
                                                    const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkSetLatencyMarkerNV);
    FinishReadObject(swapchain, vvl::Func::vkSetLatencyMarkerNV);
}

void ThreadSafety::PreCallRecordGetLatencyTimingsNV(VkDevice device, VkSwapchainKHR swapchain, uint32_t* pTimingCount,
                                                    VkGetLatencyMarkerInfoNV* pLatencyMarkerInfo) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetLatencyTimingsNV);
    StartReadObject(swapchain, vvl::Func::vkGetLatencyTimingsNV);
}

void ThreadSafety::PostCallRecordGetLatencyTimingsNV(VkDevice device, VkSwapchainKHR swapchain, uint32_t* pTimingCount,
                                                     VkGetLatencyMarkerInfoNV* pLatencyMarkerInfo, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetLatencyTimingsNV);
    FinishReadObject(swapchain, vvl::Func::vkGetLatencyTimingsNV);
}

void ThreadSafety::PreCallRecordQueueNotifyOutOfBandNV(VkQueue queue, const VkOutOfBandQueueTypeInfoNV* pQueueTypeInfo) {
    StartReadObject(queue, vvl::Func::vkQueueNotifyOutOfBandNV);
}

void ThreadSafety::PostCallRecordQueueNotifyOutOfBandNV(VkQueue queue, const VkOutOfBandQueueTypeInfoNV* pQueueTypeInfo,
                                                        const RecordObject& record_obj) {
    FinishReadObject(queue, vvl::Func::vkQueueNotifyOutOfBandNV);
}

void ThreadSafety::PreCallRecordCmdSetAttachmentFeedbackLoopEnableEXT(VkCommandBuffer commandBuffer,
                                                                      VkImageAspectFlags aspectMask) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdSetAttachmentFeedbackLoopEnableEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetAttachmentFeedbackLoopEnableEXT(VkCommandBuffer commandBuffer, VkImageAspectFlags aspectMask,
                                                                       const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdSetAttachmentFeedbackLoopEnableEXT);
    // Host access to commandBuffer must be externally synchronized
}

#ifdef VK_USE_PLATFORM_SCREEN_QNX
void ThreadSafety::PreCallRecordGetScreenBufferPropertiesQNX(VkDevice device, const struct _screen_buffer* buffer,
                                                             VkScreenBufferPropertiesQNX* pProperties) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetScreenBufferPropertiesQNX);
}

void ThreadSafety::PostCallRecordGetScreenBufferPropertiesQNX(VkDevice device, const struct _screen_buffer* buffer,
                                                              VkScreenBufferPropertiesQNX* pProperties,
                                                              const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetScreenBufferPropertiesQNX);
}

#endif  // VK_USE_PLATFORM_SCREEN_QNX
void ThreadSafety::PreCallRecordCreateAccelerationStructureKHR(VkDevice device,
                                                               const VkAccelerationStructureCreateInfoKHR* pCreateInfo,
                                                               const VkAllocationCallbacks* pAllocator,
                                                               VkAccelerationStructureKHR* pAccelerationStructure) {
    StartReadObjectParentInstance(device, vvl::Func::vkCreateAccelerationStructureKHR);
}

void ThreadSafety::PostCallRecordCreateAccelerationStructureKHR(VkDevice device,
                                                                const VkAccelerationStructureCreateInfoKHR* pCreateInfo,
                                                                const VkAllocationCallbacks* pAllocator,
                                                                VkAccelerationStructureKHR* pAccelerationStructure,
                                                                const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkCreateAccelerationStructureKHR);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pAccelerationStructure);
    }
}

void ThreadSafety::PreCallRecordDestroyAccelerationStructureKHR(VkDevice device, VkAccelerationStructureKHR accelerationStructure,
                                                                const VkAllocationCallbacks* pAllocator) {
    StartReadObjectParentInstance(device, vvl::Func::vkDestroyAccelerationStructureKHR);
    StartWriteObject(accelerationStructure, vvl::Func::vkDestroyAccelerationStructureKHR);
    // Host access to accelerationStructure must be externally synchronized
}

void ThreadSafety::PostCallRecordDestroyAccelerationStructureKHR(VkDevice device, VkAccelerationStructureKHR accelerationStructure,
                                                                 const VkAllocationCallbacks* pAllocator,
                                                                 const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkDestroyAccelerationStructureKHR);
    FinishWriteObject(accelerationStructure, vvl::Func::vkDestroyAccelerationStructureKHR);
    DestroyObject(accelerationStructure);
    // Host access to accelerationStructure must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdBuildAccelerationStructuresKHR(
    VkCommandBuffer commandBuffer, uint32_t infoCount, const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
    const VkAccelerationStructureBuildRangeInfoKHR* const* ppBuildRangeInfos) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdBuildAccelerationStructuresKHR);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdBuildAccelerationStructuresKHR(
    VkCommandBuffer commandBuffer, uint32_t infoCount, const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
    const VkAccelerationStructureBuildRangeInfoKHR* const* ppBuildRangeInfos, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdBuildAccelerationStructuresKHR);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdBuildAccelerationStructuresIndirectKHR(VkCommandBuffer commandBuffer, uint32_t infoCount,
                                                                          const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
                                                                          const VkDeviceAddress* pIndirectDeviceAddresses,
                                                                          const uint32_t* pIndirectStrides,
                                                                          const uint32_t* const* ppMaxPrimitiveCounts) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdBuildAccelerationStructuresIndirectKHR);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdBuildAccelerationStructuresIndirectKHR(
    VkCommandBuffer commandBuffer, uint32_t infoCount, const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
    const VkDeviceAddress* pIndirectDeviceAddresses, const uint32_t* pIndirectStrides, const uint32_t* const* ppMaxPrimitiveCounts,
    const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdBuildAccelerationStructuresIndirectKHR);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordBuildAccelerationStructuresKHR(
    VkDevice device, VkDeferredOperationKHR deferredOperation, uint32_t infoCount,
    const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
    const VkAccelerationStructureBuildRangeInfoKHR* const* ppBuildRangeInfos) {
    StartReadObjectParentInstance(device, vvl::Func::vkBuildAccelerationStructuresKHR);
    StartReadObject(deferredOperation, vvl::Func::vkBuildAccelerationStructuresKHR);
}

void ThreadSafety::PostCallRecordBuildAccelerationStructuresKHR(
    VkDevice device, VkDeferredOperationKHR deferredOperation, uint32_t infoCount,
    const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
    const VkAccelerationStructureBuildRangeInfoKHR* const* ppBuildRangeInfos, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkBuildAccelerationStructuresKHR);
    FinishReadObject(deferredOperation, vvl::Func::vkBuildAccelerationStructuresKHR);
}

void ThreadSafety::PreCallRecordCopyAccelerationStructureKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                             const VkCopyAccelerationStructureInfoKHR* pInfo) {
    StartReadObjectParentInstance(device, vvl::Func::vkCopyAccelerationStructureKHR);
    StartReadObject(deferredOperation, vvl::Func::vkCopyAccelerationStructureKHR);
}

void ThreadSafety::PostCallRecordCopyAccelerationStructureKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                              const VkCopyAccelerationStructureInfoKHR* pInfo,
                                                              const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkCopyAccelerationStructureKHR);
    FinishReadObject(deferredOperation, vvl::Func::vkCopyAccelerationStructureKHR);
}

void ThreadSafety::PreCallRecordCopyAccelerationStructureToMemoryKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                                     const VkCopyAccelerationStructureToMemoryInfoKHR* pInfo) {
    StartReadObjectParentInstance(device, vvl::Func::vkCopyAccelerationStructureToMemoryKHR);
    StartReadObject(deferredOperation, vvl::Func::vkCopyAccelerationStructureToMemoryKHR);
}

void ThreadSafety::PostCallRecordCopyAccelerationStructureToMemoryKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                                      const VkCopyAccelerationStructureToMemoryInfoKHR* pInfo,
                                                                      const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkCopyAccelerationStructureToMemoryKHR);
    FinishReadObject(deferredOperation, vvl::Func::vkCopyAccelerationStructureToMemoryKHR);
}

void ThreadSafety::PreCallRecordCopyMemoryToAccelerationStructureKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                                     const VkCopyMemoryToAccelerationStructureInfoKHR* pInfo) {
    StartReadObjectParentInstance(device, vvl::Func::vkCopyMemoryToAccelerationStructureKHR);
    StartReadObject(deferredOperation, vvl::Func::vkCopyMemoryToAccelerationStructureKHR);
}

void ThreadSafety::PostCallRecordCopyMemoryToAccelerationStructureKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                                      const VkCopyMemoryToAccelerationStructureInfoKHR* pInfo,
                                                                      const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkCopyMemoryToAccelerationStructureKHR);
    FinishReadObject(deferredOperation, vvl::Func::vkCopyMemoryToAccelerationStructureKHR);
}

void ThreadSafety::PreCallRecordWriteAccelerationStructuresPropertiesKHR(VkDevice device, uint32_t accelerationStructureCount,
                                                                         const VkAccelerationStructureKHR* pAccelerationStructures,
                                                                         VkQueryType queryType, size_t dataSize, void* pData,
                                                                         size_t stride) {
    StartReadObjectParentInstance(device, vvl::Func::vkWriteAccelerationStructuresPropertiesKHR);

    if (pAccelerationStructures) {
        for (uint32_t index = 0; index < accelerationStructureCount; index++) {
            StartReadObject(pAccelerationStructures[index], vvl::Func::vkWriteAccelerationStructuresPropertiesKHR);
        }
    }
}

void ThreadSafety::PostCallRecordWriteAccelerationStructuresPropertiesKHR(VkDevice device, uint32_t accelerationStructureCount,
                                                                          const VkAccelerationStructureKHR* pAccelerationStructures,
                                                                          VkQueryType queryType, size_t dataSize, void* pData,
                                                                          size_t stride, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkWriteAccelerationStructuresPropertiesKHR);

    if (pAccelerationStructures) {
        for (uint32_t index = 0; index < accelerationStructureCount; index++) {
            FinishReadObject(pAccelerationStructures[index], vvl::Func::vkWriteAccelerationStructuresPropertiesKHR);
        }
    }
}

void ThreadSafety::PreCallRecordCmdCopyAccelerationStructureKHR(VkCommandBuffer commandBuffer,
                                                                const VkCopyAccelerationStructureInfoKHR* pInfo) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdCopyAccelerationStructureKHR);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdCopyAccelerationStructureKHR(VkCommandBuffer commandBuffer,
                                                                 const VkCopyAccelerationStructureInfoKHR* pInfo,
                                                                 const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdCopyAccelerationStructureKHR);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdCopyAccelerationStructureToMemoryKHR(VkCommandBuffer commandBuffer,
                                                                        const VkCopyAccelerationStructureToMemoryInfoKHR* pInfo) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdCopyAccelerationStructureToMemoryKHR);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdCopyAccelerationStructureToMemoryKHR(VkCommandBuffer commandBuffer,
                                                                         const VkCopyAccelerationStructureToMemoryInfoKHR* pInfo,
                                                                         const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdCopyAccelerationStructureToMemoryKHR);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdCopyMemoryToAccelerationStructureKHR(VkCommandBuffer commandBuffer,
                                                                        const VkCopyMemoryToAccelerationStructureInfoKHR* pInfo) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdCopyMemoryToAccelerationStructureKHR);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdCopyMemoryToAccelerationStructureKHR(VkCommandBuffer commandBuffer,
                                                                         const VkCopyMemoryToAccelerationStructureInfoKHR* pInfo,
                                                                         const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdCopyMemoryToAccelerationStructureKHR);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordGetAccelerationStructureDeviceAddressKHR(VkDevice device,
                                                                         const VkAccelerationStructureDeviceAddressInfoKHR* pInfo) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetAccelerationStructureDeviceAddressKHR);
}

void ThreadSafety::PostCallRecordGetAccelerationStructureDeviceAddressKHR(VkDevice device,
                                                                          const VkAccelerationStructureDeviceAddressInfoKHR* pInfo,
                                                                          const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetAccelerationStructureDeviceAddressKHR);
}

void ThreadSafety::PreCallRecordCmdWriteAccelerationStructuresPropertiesKHR(
    VkCommandBuffer commandBuffer, uint32_t accelerationStructureCount, const VkAccelerationStructureKHR* pAccelerationStructures,
    VkQueryType queryType, VkQueryPool queryPool, uint32_t firstQuery) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdWriteAccelerationStructuresPropertiesKHR);

    if (pAccelerationStructures) {
        for (uint32_t index = 0; index < accelerationStructureCount; index++) {
            StartReadObject(pAccelerationStructures[index], vvl::Func::vkCmdWriteAccelerationStructuresPropertiesKHR);
        }
    }
    StartReadObject(queryPool, vvl::Func::vkCmdWriteAccelerationStructuresPropertiesKHR);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdWriteAccelerationStructuresPropertiesKHR(
    VkCommandBuffer commandBuffer, uint32_t accelerationStructureCount, const VkAccelerationStructureKHR* pAccelerationStructures,
    VkQueryType queryType, VkQueryPool queryPool, uint32_t firstQuery, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdWriteAccelerationStructuresPropertiesKHR);

    if (pAccelerationStructures) {
        for (uint32_t index = 0; index < accelerationStructureCount; index++) {
            FinishReadObject(pAccelerationStructures[index], vvl::Func::vkCmdWriteAccelerationStructuresPropertiesKHR);
        }
    }
    FinishReadObject(queryPool, vvl::Func::vkCmdWriteAccelerationStructuresPropertiesKHR);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordGetDeviceAccelerationStructureCompatibilityKHR(
    VkDevice device, const VkAccelerationStructureVersionInfoKHR* pVersionInfo,
    VkAccelerationStructureCompatibilityKHR* pCompatibility) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetDeviceAccelerationStructureCompatibilityKHR);
}

void ThreadSafety::PostCallRecordGetDeviceAccelerationStructureCompatibilityKHR(
    VkDevice device, const VkAccelerationStructureVersionInfoKHR* pVersionInfo,
    VkAccelerationStructureCompatibilityKHR* pCompatibility, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetDeviceAccelerationStructureCompatibilityKHR);
}

void ThreadSafety::PreCallRecordGetAccelerationStructureBuildSizesKHR(VkDevice device,
                                                                      VkAccelerationStructureBuildTypeKHR buildType,
                                                                      const VkAccelerationStructureBuildGeometryInfoKHR* pBuildInfo,
                                                                      const uint32_t* pMaxPrimitiveCounts,
                                                                      VkAccelerationStructureBuildSizesInfoKHR* pSizeInfo) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetAccelerationStructureBuildSizesKHR);
}

void ThreadSafety::PostCallRecordGetAccelerationStructureBuildSizesKHR(
    VkDevice device, VkAccelerationStructureBuildTypeKHR buildType, const VkAccelerationStructureBuildGeometryInfoKHR* pBuildInfo,
    const uint32_t* pMaxPrimitiveCounts, VkAccelerationStructureBuildSizesInfoKHR* pSizeInfo, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetAccelerationStructureBuildSizesKHR);
}

void ThreadSafety::PreCallRecordCmdTraceRaysKHR(VkCommandBuffer commandBuffer,
                                                const VkStridedDeviceAddressRegionKHR* pRaygenShaderBindingTable,
                                                const VkStridedDeviceAddressRegionKHR* pMissShaderBindingTable,
                                                const VkStridedDeviceAddressRegionKHR* pHitShaderBindingTable,
                                                const VkStridedDeviceAddressRegionKHR* pCallableShaderBindingTable, uint32_t width,
                                                uint32_t height, uint32_t depth) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdTraceRaysKHR);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdTraceRaysKHR(VkCommandBuffer commandBuffer,
                                                 const VkStridedDeviceAddressRegionKHR* pRaygenShaderBindingTable,
                                                 const VkStridedDeviceAddressRegionKHR* pMissShaderBindingTable,
                                                 const VkStridedDeviceAddressRegionKHR* pHitShaderBindingTable,
                                                 const VkStridedDeviceAddressRegionKHR* pCallableShaderBindingTable, uint32_t width,
                                                 uint32_t height, uint32_t depth, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdTraceRaysKHR);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordGetRayTracingCaptureReplayShaderGroupHandlesKHR(VkDevice device, VkPipeline pipeline,
                                                                                uint32_t firstGroup, uint32_t groupCount,
                                                                                size_t dataSize, void* pData) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetRayTracingCaptureReplayShaderGroupHandlesKHR);
    StartReadObject(pipeline, vvl::Func::vkGetRayTracingCaptureReplayShaderGroupHandlesKHR);
}

void ThreadSafety::PostCallRecordGetRayTracingCaptureReplayShaderGroupHandlesKHR(VkDevice device, VkPipeline pipeline,
                                                                                 uint32_t firstGroup, uint32_t groupCount,
                                                                                 size_t dataSize, void* pData,
                                                                                 const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetRayTracingCaptureReplayShaderGroupHandlesKHR);
    FinishReadObject(pipeline, vvl::Func::vkGetRayTracingCaptureReplayShaderGroupHandlesKHR);
}

void ThreadSafety::PreCallRecordCmdTraceRaysIndirectKHR(VkCommandBuffer commandBuffer,
                                                        const VkStridedDeviceAddressRegionKHR* pRaygenShaderBindingTable,
                                                        const VkStridedDeviceAddressRegionKHR* pMissShaderBindingTable,
                                                        const VkStridedDeviceAddressRegionKHR* pHitShaderBindingTable,
                                                        const VkStridedDeviceAddressRegionKHR* pCallableShaderBindingTable,
                                                        VkDeviceAddress indirectDeviceAddress) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdTraceRaysIndirectKHR);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdTraceRaysIndirectKHR(VkCommandBuffer commandBuffer,
                                                         const VkStridedDeviceAddressRegionKHR* pRaygenShaderBindingTable,
                                                         const VkStridedDeviceAddressRegionKHR* pMissShaderBindingTable,
                                                         const VkStridedDeviceAddressRegionKHR* pHitShaderBindingTable,
                                                         const VkStridedDeviceAddressRegionKHR* pCallableShaderBindingTable,
                                                         VkDeviceAddress indirectDeviceAddress, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdTraceRaysIndirectKHR);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordGetRayTracingShaderGroupStackSizeKHR(VkDevice device, VkPipeline pipeline, uint32_t group,
                                                                     VkShaderGroupShaderKHR groupShader) {
    StartReadObjectParentInstance(device, vvl::Func::vkGetRayTracingShaderGroupStackSizeKHR);
    StartReadObject(pipeline, vvl::Func::vkGetRayTracingShaderGroupStackSizeKHR);
}

void ThreadSafety::PostCallRecordGetRayTracingShaderGroupStackSizeKHR(VkDevice device, VkPipeline pipeline, uint32_t group,
                                                                      VkShaderGroupShaderKHR groupShader,
                                                                      const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, vvl::Func::vkGetRayTracingShaderGroupStackSizeKHR);
    FinishReadObject(pipeline, vvl::Func::vkGetRayTracingShaderGroupStackSizeKHR);
}

void ThreadSafety::PreCallRecordCmdSetRayTracingPipelineStackSizeKHR(VkCommandBuffer commandBuffer, uint32_t pipelineStackSize) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdSetRayTracingPipelineStackSizeKHR);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetRayTracingPipelineStackSizeKHR(VkCommandBuffer commandBuffer, uint32_t pipelineStackSize,
                                                                      const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdSetRayTracingPipelineStackSizeKHR);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdDrawMeshTasksEXT(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY,
                                                    uint32_t groupCountZ) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdDrawMeshTasksEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdDrawMeshTasksEXT(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY,
                                                     uint32_t groupCountZ, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdDrawMeshTasksEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdDrawMeshTasksIndirectEXT(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                            uint32_t drawCount, uint32_t stride) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdDrawMeshTasksIndirectEXT);
    StartReadObject(buffer, vvl::Func::vkCmdDrawMeshTasksIndirectEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdDrawMeshTasksIndirectEXT(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                             uint32_t drawCount, uint32_t stride, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdDrawMeshTasksIndirectEXT);
    FinishReadObject(buffer, vvl::Func::vkCmdDrawMeshTasksIndirectEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdDrawMeshTasksIndirectCountEXT(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                 VkDeviceSize offset, VkBuffer countBuffer,
                                                                 VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                                 uint32_t stride) {
    StartWriteObject(commandBuffer, vvl::Func::vkCmdDrawMeshTasksIndirectCountEXT);
    StartReadObject(buffer, vvl::Func::vkCmdDrawMeshTasksIndirectCountEXT);
    StartReadObject(countBuffer, vvl::Func::vkCmdDrawMeshTasksIndirectCountEXT);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdDrawMeshTasksIndirectCountEXT(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                  VkDeviceSize offset, VkBuffer countBuffer,
                                                                  VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                                  uint32_t stride, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, vvl::Func::vkCmdDrawMeshTasksIndirectCountEXT);
    FinishReadObject(buffer, vvl::Func::vkCmdDrawMeshTasksIndirectCountEXT);
    FinishReadObject(countBuffer, vvl::Func::vkCmdDrawMeshTasksIndirectCountEXT);
    // Host access to commandBuffer must be externally synchronized
}

// NOLINTEND
