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
                                               VkInstance* pInstance, const RecordObject& record_obj) {}

void ThreadSafety::PostCallRecordCreateInstance(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                                VkInstance* pInstance, const RecordObject& record_obj) {
    if (record_obj.result == VK_SUCCESS) {
        CreateObjectParentInstance(*pInstance);
    }
}

void ThreadSafety::PreCallRecordDestroyInstance(VkInstance instance, const VkAllocationCallbacks* pAllocator,
                                                const RecordObject& record_obj) {
    StartWriteObjectParentInstance(instance, record_obj.location);
    // Host access to instance must be externally synchronized
    // all sname:VkPhysicalDevice objects enumerated from pname:instance must be externally synchronized between host accesses
}

void ThreadSafety::PostCallRecordDestroyInstance(VkInstance instance, const VkAllocationCallbacks* pAllocator,
                                                 const RecordObject& record_obj) {
    FinishWriteObjectParentInstance(instance, record_obj.location);
    DestroyObjectParentInstance(instance);
    // Host access to instance must be externally synchronized
    // all sname:VkPhysicalDevice objects enumerated from pname:instance must be externally synchronized between host accesses
}

void ThreadSafety::PreCallRecordEnumeratePhysicalDevices(VkInstance instance, uint32_t* pPhysicalDeviceCount,
                                                         VkPhysicalDevice* pPhysicalDevices, const RecordObject& record_obj) {
    StartReadObjectParentInstance(instance, record_obj.location);
}

void ThreadSafety::PostCallRecordEnumeratePhysicalDevices(VkInstance instance, uint32_t* pPhysicalDeviceCount,
                                                          VkPhysicalDevice* pPhysicalDevices, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(instance, record_obj.location);
}

void ThreadSafety::PreCallRecordGetInstanceProcAddr(VkInstance instance, const char* pName, const RecordObject& record_obj) {
    StartReadObjectParentInstance(instance, record_obj.location);
}

void ThreadSafety::PostCallRecordGetInstanceProcAddr(VkInstance instance, const char* pName, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(instance, record_obj.location);
}

void ThreadSafety::PreCallRecordGetDeviceProcAddr(VkDevice device, const char* pName, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordGetDeviceProcAddr(VkDevice device, const char* pName, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PreCallRecordCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo,
                                             const VkAllocationCallbacks* pAllocator, VkDevice* pDevice,
                                             const RecordObject& record_obj) {}

void ThreadSafety::PostCallRecordCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo,
                                              const VkAllocationCallbacks* pAllocator, VkDevice* pDevice,
                                              const RecordObject& record_obj) {
    if (record_obj.result == VK_SUCCESS) {
        CreateObjectParentInstance(*pDevice);
    }
}

void ThreadSafety::PreCallRecordQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence,
                                            const RecordObject& record_obj) {
    StartWriteObject(queue, record_obj.location);
    StartWriteObject(fence, record_obj.location);
    // Host access to queue must be externally synchronized
    // Host access to fence must be externally synchronized
}

void ThreadSafety::PostCallRecordQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence,
                                             const RecordObject& record_obj) {
    FinishWriteObject(queue, record_obj.location);
    FinishWriteObject(fence, record_obj.location);
    // Host access to queue must be externally synchronized
    // Host access to fence must be externally synchronized
}

void ThreadSafety::PreCallRecordQueueWaitIdle(VkQueue queue, const RecordObject& record_obj) {
    StartWriteObject(queue, record_obj.location);
    // Host access to queue must be externally synchronized
}

void ThreadSafety::PostCallRecordQueueWaitIdle(VkQueue queue, const RecordObject& record_obj) {
    FinishWriteObject(queue, record_obj.location);
    // Host access to queue must be externally synchronized
}

void ThreadSafety::PreCallRecordAllocateMemory(VkDevice device, const VkMemoryAllocateInfo* pAllocateInfo,
                                               const VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory,
                                               const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordAllocateMemory(VkDevice device, const VkMemoryAllocateInfo* pAllocateInfo,
                                                const VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory,
                                                const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pMemory);
    }
}

void ThreadSafety::PreCallRecordFreeMemory(VkDevice device, VkDeviceMemory memory, const VkAllocationCallbacks* pAllocator,
                                           const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartWriteObject(memory, record_obj.location);
    // Host access to memory must be externally synchronized
}

void ThreadSafety::PostCallRecordFreeMemory(VkDevice device, VkDeviceMemory memory, const VkAllocationCallbacks* pAllocator,
                                            const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishWriteObject(memory, record_obj.location);
    DestroyObject(memory);
    // Host access to memory must be externally synchronized
}

void ThreadSafety::PreCallRecordMapMemory(VkDevice device, VkDeviceMemory memory, VkDeviceSize offset, VkDeviceSize size,
                                          VkMemoryMapFlags flags, void** ppData, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartWriteObject(memory, record_obj.location);
    // Host access to memory must be externally synchronized
}

void ThreadSafety::PostCallRecordMapMemory(VkDevice device, VkDeviceMemory memory, VkDeviceSize offset, VkDeviceSize size,
                                           VkMemoryMapFlags flags, void** ppData, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishWriteObject(memory, record_obj.location);
    // Host access to memory must be externally synchronized
}

void ThreadSafety::PreCallRecordUnmapMemory(VkDevice device, VkDeviceMemory memory, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartWriteObject(memory, record_obj.location);
    // Host access to memory must be externally synchronized
}

void ThreadSafety::PostCallRecordUnmapMemory(VkDevice device, VkDeviceMemory memory, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishWriteObject(memory, record_obj.location);
    // Host access to memory must be externally synchronized
}

void ThreadSafety::PreCallRecordFlushMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount,
                                                        const VkMappedMemoryRange* pMemoryRanges, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordFlushMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount,
                                                         const VkMappedMemoryRange* pMemoryRanges, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PreCallRecordInvalidateMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount,
                                                             const VkMappedMemoryRange* pMemoryRanges,
                                                             const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordInvalidateMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount,
                                                              const VkMappedMemoryRange* pMemoryRanges,
                                                              const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PreCallRecordGetDeviceMemoryCommitment(VkDevice device, VkDeviceMemory memory,
                                                          VkDeviceSize* pCommittedMemoryInBytes, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(memory, record_obj.location);
}

void ThreadSafety::PostCallRecordGetDeviceMemoryCommitment(VkDevice device, VkDeviceMemory memory,
                                                           VkDeviceSize* pCommittedMemoryInBytes, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(memory, record_obj.location);
}

void ThreadSafety::PreCallRecordBindBufferMemory(VkDevice device, VkBuffer buffer, VkDeviceMemory memory, VkDeviceSize memoryOffset,
                                                 const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartWriteObject(buffer, record_obj.location);
    StartReadObject(memory, record_obj.location);
    // Host access to buffer must be externally synchronized
}

void ThreadSafety::PostCallRecordBindBufferMemory(VkDevice device, VkBuffer buffer, VkDeviceMemory memory,
                                                  VkDeviceSize memoryOffset, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishWriteObject(buffer, record_obj.location);
    FinishReadObject(memory, record_obj.location);
    // Host access to buffer must be externally synchronized
}

void ThreadSafety::PreCallRecordBindImageMemory(VkDevice device, VkImage image, VkDeviceMemory memory, VkDeviceSize memoryOffset,
                                                const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartWriteObject(image, record_obj.location);
    StartReadObject(memory, record_obj.location);
    // Host access to image must be externally synchronized
}

void ThreadSafety::PostCallRecordBindImageMemory(VkDevice device, VkImage image, VkDeviceMemory memory, VkDeviceSize memoryOffset,
                                                 const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishWriteObject(image, record_obj.location);
    FinishReadObject(memory, record_obj.location);
    // Host access to image must be externally synchronized
}

void ThreadSafety::PreCallRecordGetBufferMemoryRequirements(VkDevice device, VkBuffer buffer,
                                                            VkMemoryRequirements* pMemoryRequirements,
                                                            const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(buffer, record_obj.location);
}

void ThreadSafety::PostCallRecordGetBufferMemoryRequirements(VkDevice device, VkBuffer buffer,
                                                             VkMemoryRequirements* pMemoryRequirements,
                                                             const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(buffer, record_obj.location);
}

void ThreadSafety::PreCallRecordGetImageMemoryRequirements(VkDevice device, VkImage image,
                                                           VkMemoryRequirements* pMemoryRequirements,
                                                           const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(image, record_obj.location);
}

void ThreadSafety::PostCallRecordGetImageMemoryRequirements(VkDevice device, VkImage image,
                                                            VkMemoryRequirements* pMemoryRequirements,
                                                            const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(image, record_obj.location);
}

void ThreadSafety::PreCallRecordGetImageSparseMemoryRequirements(VkDevice device, VkImage image,
                                                                 uint32_t* pSparseMemoryRequirementCount,
                                                                 VkSparseImageMemoryRequirements* pSparseMemoryRequirements,
                                                                 const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(image, record_obj.location);
}

void ThreadSafety::PostCallRecordGetImageSparseMemoryRequirements(VkDevice device, VkImage image,
                                                                  uint32_t* pSparseMemoryRequirementCount,
                                                                  VkSparseImageMemoryRequirements* pSparseMemoryRequirements,
                                                                  const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(image, record_obj.location);
}

void ThreadSafety::PreCallRecordQueueBindSparse(VkQueue queue, uint32_t bindInfoCount, const VkBindSparseInfo* pBindInfo,
                                                VkFence fence, const RecordObject& record_obj) {
    StartWriteObject(queue, record_obj.location);
    StartWriteObject(fence, record_obj.location);
    // Host access to queue must be externally synchronized
    // Host access to fence must be externally synchronized
}

void ThreadSafety::PostCallRecordQueueBindSparse(VkQueue queue, uint32_t bindInfoCount, const VkBindSparseInfo* pBindInfo,
                                                 VkFence fence, const RecordObject& record_obj) {
    FinishWriteObject(queue, record_obj.location);
    FinishWriteObject(fence, record_obj.location);
    // Host access to queue must be externally synchronized
    // Host access to fence must be externally synchronized
}

void ThreadSafety::PreCallRecordCreateFence(VkDevice device, const VkFenceCreateInfo* pCreateInfo,
                                            const VkAllocationCallbacks* pAllocator, VkFence* pFence,
                                            const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordCreateFence(VkDevice device, const VkFenceCreateInfo* pCreateInfo,
                                             const VkAllocationCallbacks* pAllocator, VkFence* pFence,
                                             const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pFence);
    }
}

void ThreadSafety::PreCallRecordDestroyFence(VkDevice device, VkFence fence, const VkAllocationCallbacks* pAllocator,
                                             const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartWriteObject(fence, record_obj.location);
    // Host access to fence must be externally synchronized
}

void ThreadSafety::PostCallRecordDestroyFence(VkDevice device, VkFence fence, const VkAllocationCallbacks* pAllocator,
                                              const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishWriteObject(fence, record_obj.location);
    DestroyObject(fence);
    // Host access to fence must be externally synchronized
}

void ThreadSafety::PreCallRecordResetFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences,
                                            const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);

    if (pFences) {
        for (uint32_t index = 0; index < fenceCount; index++) {
            StartWriteObject(pFences[index], record_obj.location);
        }
    }
    // Host access to each member of pFences must be externally synchronized
}

void ThreadSafety::PostCallRecordResetFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences,
                                             const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);

    if (pFences) {
        for (uint32_t index = 0; index < fenceCount; index++) {
            FinishWriteObject(pFences[index], record_obj.location);
        }
    }
    // Host access to each member of pFences must be externally synchronized
}

void ThreadSafety::PreCallRecordGetFenceStatus(VkDevice device, VkFence fence, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(fence, record_obj.location);
}

void ThreadSafety::PostCallRecordGetFenceStatus(VkDevice device, VkFence fence, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(fence, record_obj.location);
}

void ThreadSafety::PreCallRecordWaitForFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences, VkBool32 waitAll,
                                              uint64_t timeout, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);

    if (pFences) {
        for (uint32_t index = 0; index < fenceCount; index++) {
            StartReadObject(pFences[index], record_obj.location);
        }
    }
}

void ThreadSafety::PostCallRecordWaitForFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences, VkBool32 waitAll,
                                               uint64_t timeout, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);

    if (pFences) {
        for (uint32_t index = 0; index < fenceCount; index++) {
            FinishReadObject(pFences[index], record_obj.location);
        }
    }
}

void ThreadSafety::PreCallRecordCreateSemaphore(VkDevice device, const VkSemaphoreCreateInfo* pCreateInfo,
                                                const VkAllocationCallbacks* pAllocator, VkSemaphore* pSemaphore,
                                                const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordCreateSemaphore(VkDevice device, const VkSemaphoreCreateInfo* pCreateInfo,
                                                 const VkAllocationCallbacks* pAllocator, VkSemaphore* pSemaphore,
                                                 const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pSemaphore);
    }
}

void ThreadSafety::PreCallRecordDestroySemaphore(VkDevice device, VkSemaphore semaphore, const VkAllocationCallbacks* pAllocator,
                                                 const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartWriteObject(semaphore, record_obj.location);
    // Host access to semaphore must be externally synchronized
}

void ThreadSafety::PostCallRecordDestroySemaphore(VkDevice device, VkSemaphore semaphore, const VkAllocationCallbacks* pAllocator,
                                                  const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishWriteObject(semaphore, record_obj.location);
    DestroyObject(semaphore);
    // Host access to semaphore must be externally synchronized
}

void ThreadSafety::PreCallRecordCreateEvent(VkDevice device, const VkEventCreateInfo* pCreateInfo,
                                            const VkAllocationCallbacks* pAllocator, VkEvent* pEvent,
                                            const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordCreateEvent(VkDevice device, const VkEventCreateInfo* pCreateInfo,
                                             const VkAllocationCallbacks* pAllocator, VkEvent* pEvent,
                                             const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pEvent);
    }
}

void ThreadSafety::PreCallRecordDestroyEvent(VkDevice device, VkEvent event, const VkAllocationCallbacks* pAllocator,
                                             const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartWriteObject(event, record_obj.location);
    // Host access to event must be externally synchronized
}

void ThreadSafety::PostCallRecordDestroyEvent(VkDevice device, VkEvent event, const VkAllocationCallbacks* pAllocator,
                                              const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishWriteObject(event, record_obj.location);
    DestroyObject(event);
    // Host access to event must be externally synchronized
}

void ThreadSafety::PreCallRecordGetEventStatus(VkDevice device, VkEvent event, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(event, record_obj.location);
}

void ThreadSafety::PostCallRecordGetEventStatus(VkDevice device, VkEvent event, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(event, record_obj.location);
}

void ThreadSafety::PreCallRecordSetEvent(VkDevice device, VkEvent event, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartWriteObject(event, record_obj.location);
    // Host access to event must be externally synchronized
}

void ThreadSafety::PostCallRecordSetEvent(VkDevice device, VkEvent event, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishWriteObject(event, record_obj.location);
    // Host access to event must be externally synchronized
}

void ThreadSafety::PreCallRecordResetEvent(VkDevice device, VkEvent event, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartWriteObject(event, record_obj.location);
    // Host access to event must be externally synchronized
}

void ThreadSafety::PostCallRecordResetEvent(VkDevice device, VkEvent event, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishWriteObject(event, record_obj.location);
    // Host access to event must be externally synchronized
}

void ThreadSafety::PreCallRecordCreateQueryPool(VkDevice device, const VkQueryPoolCreateInfo* pCreateInfo,
                                                const VkAllocationCallbacks* pAllocator, VkQueryPool* pQueryPool,
                                                const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordCreateQueryPool(VkDevice device, const VkQueryPoolCreateInfo* pCreateInfo,
                                                 const VkAllocationCallbacks* pAllocator, VkQueryPool* pQueryPool,
                                                 const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pQueryPool);
    }
}

void ThreadSafety::PreCallRecordDestroyQueryPool(VkDevice device, VkQueryPool queryPool, const VkAllocationCallbacks* pAllocator,
                                                 const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartWriteObject(queryPool, record_obj.location);
    // Host access to queryPool must be externally synchronized
}

void ThreadSafety::PostCallRecordDestroyQueryPool(VkDevice device, VkQueryPool queryPool, const VkAllocationCallbacks* pAllocator,
                                                  const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishWriteObject(queryPool, record_obj.location);
    DestroyObject(queryPool);
    // Host access to queryPool must be externally synchronized
}

void ThreadSafety::PreCallRecordGetQueryPoolResults(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery,
                                                    uint32_t queryCount, size_t dataSize, void* pData, VkDeviceSize stride,
                                                    VkQueryResultFlags flags, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(queryPool, record_obj.location);
}

void ThreadSafety::PostCallRecordGetQueryPoolResults(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery,
                                                     uint32_t queryCount, size_t dataSize, void* pData, VkDeviceSize stride,
                                                     VkQueryResultFlags flags, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(queryPool, record_obj.location);
}

void ThreadSafety::PreCallRecordCreateBuffer(VkDevice device, const VkBufferCreateInfo* pCreateInfo,
                                             const VkAllocationCallbacks* pAllocator, VkBuffer* pBuffer,
                                             const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordCreateBuffer(VkDevice device, const VkBufferCreateInfo* pCreateInfo,
                                              const VkAllocationCallbacks* pAllocator, VkBuffer* pBuffer,
                                              const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pBuffer);
    }
}

void ThreadSafety::PreCallRecordDestroyBuffer(VkDevice device, VkBuffer buffer, const VkAllocationCallbacks* pAllocator,
                                              const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartWriteObject(buffer, record_obj.location);
    // Host access to buffer must be externally synchronized
}

void ThreadSafety::PostCallRecordDestroyBuffer(VkDevice device, VkBuffer buffer, const VkAllocationCallbacks* pAllocator,
                                               const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishWriteObject(buffer, record_obj.location);
    DestroyObject(buffer);
    // Host access to buffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCreateBufferView(VkDevice device, const VkBufferViewCreateInfo* pCreateInfo,
                                                 const VkAllocationCallbacks* pAllocator, VkBufferView* pView,
                                                 const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordCreateBufferView(VkDevice device, const VkBufferViewCreateInfo* pCreateInfo,
                                                  const VkAllocationCallbacks* pAllocator, VkBufferView* pView,
                                                  const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pView);
    }
}

void ThreadSafety::PreCallRecordDestroyBufferView(VkDevice device, VkBufferView bufferView, const VkAllocationCallbacks* pAllocator,
                                                  const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartWriteObject(bufferView, record_obj.location);
    // Host access to bufferView must be externally synchronized
}

void ThreadSafety::PostCallRecordDestroyBufferView(VkDevice device, VkBufferView bufferView,
                                                   const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishWriteObject(bufferView, record_obj.location);
    DestroyObject(bufferView);
    // Host access to bufferView must be externally synchronized
}

void ThreadSafety::PreCallRecordCreateImage(VkDevice device, const VkImageCreateInfo* pCreateInfo,
                                            const VkAllocationCallbacks* pAllocator, VkImage* pImage,
                                            const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordCreateImage(VkDevice device, const VkImageCreateInfo* pCreateInfo,
                                             const VkAllocationCallbacks* pAllocator, VkImage* pImage,
                                             const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pImage);
    }
}

void ThreadSafety::PreCallRecordDestroyImage(VkDevice device, VkImage image, const VkAllocationCallbacks* pAllocator,
                                             const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartWriteObject(image, record_obj.location);
    // Host access to image must be externally synchronized
}

void ThreadSafety::PostCallRecordDestroyImage(VkDevice device, VkImage image, const VkAllocationCallbacks* pAllocator,
                                              const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishWriteObject(image, record_obj.location);
    DestroyObject(image);
    // Host access to image must be externally synchronized
}

void ThreadSafety::PreCallRecordGetImageSubresourceLayout(VkDevice device, VkImage image, const VkImageSubresource* pSubresource,
                                                          VkSubresourceLayout* pLayout, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(image, record_obj.location);
}

void ThreadSafety::PostCallRecordGetImageSubresourceLayout(VkDevice device, VkImage image, const VkImageSubresource* pSubresource,
                                                           VkSubresourceLayout* pLayout, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(image, record_obj.location);
}

void ThreadSafety::PreCallRecordCreateImageView(VkDevice device, const VkImageViewCreateInfo* pCreateInfo,
                                                const VkAllocationCallbacks* pAllocator, VkImageView* pView,
                                                const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordCreateImageView(VkDevice device, const VkImageViewCreateInfo* pCreateInfo,
                                                 const VkAllocationCallbacks* pAllocator, VkImageView* pView,
                                                 const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pView);
    }
}

void ThreadSafety::PreCallRecordDestroyImageView(VkDevice device, VkImageView imageView, const VkAllocationCallbacks* pAllocator,
                                                 const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartWriteObject(imageView, record_obj.location);
    // Host access to imageView must be externally synchronized
}

void ThreadSafety::PostCallRecordDestroyImageView(VkDevice device, VkImageView imageView, const VkAllocationCallbacks* pAllocator,
                                                  const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishWriteObject(imageView, record_obj.location);
    DestroyObject(imageView);
    // Host access to imageView must be externally synchronized
}

void ThreadSafety::PreCallRecordCreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo,
                                                   const VkAllocationCallbacks* pAllocator, VkShaderModule* pShaderModule,
                                                   const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordCreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo,
                                                    const VkAllocationCallbacks* pAllocator, VkShaderModule* pShaderModule,
                                                    const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pShaderModule);
    }
}

void ThreadSafety::PreCallRecordDestroyShaderModule(VkDevice device, VkShaderModule shaderModule,
                                                    const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartWriteObject(shaderModule, record_obj.location);
    // Host access to shaderModule must be externally synchronized
}

void ThreadSafety::PostCallRecordDestroyShaderModule(VkDevice device, VkShaderModule shaderModule,
                                                     const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishWriteObject(shaderModule, record_obj.location);
    DestroyObject(shaderModule);
    // Host access to shaderModule must be externally synchronized
}

void ThreadSafety::PreCallRecordCreatePipelineCache(VkDevice device, const VkPipelineCacheCreateInfo* pCreateInfo,
                                                    const VkAllocationCallbacks* pAllocator, VkPipelineCache* pPipelineCache,
                                                    const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordCreatePipelineCache(VkDevice device, const VkPipelineCacheCreateInfo* pCreateInfo,
                                                     const VkAllocationCallbacks* pAllocator, VkPipelineCache* pPipelineCache,
                                                     const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pPipelineCache);
    }
}

void ThreadSafety::PreCallRecordDestroyPipelineCache(VkDevice device, VkPipelineCache pipelineCache,
                                                     const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartWriteObject(pipelineCache, record_obj.location);
    // Host access to pipelineCache must be externally synchronized
}

void ThreadSafety::PostCallRecordDestroyPipelineCache(VkDevice device, VkPipelineCache pipelineCache,
                                                      const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishWriteObject(pipelineCache, record_obj.location);
    DestroyObject(pipelineCache);
    // Host access to pipelineCache must be externally synchronized
}

void ThreadSafety::PreCallRecordGetPipelineCacheData(VkDevice device, VkPipelineCache pipelineCache, size_t* pDataSize, void* pData,
                                                     const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(pipelineCache, record_obj.location);
}

void ThreadSafety::PostCallRecordGetPipelineCacheData(VkDevice device, VkPipelineCache pipelineCache, size_t* pDataSize,
                                                      void* pData, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(pipelineCache, record_obj.location);
}

void ThreadSafety::PreCallRecordMergePipelineCaches(VkDevice device, VkPipelineCache dstCache, uint32_t srcCacheCount,
                                                    const VkPipelineCache* pSrcCaches, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartWriteObject(dstCache, record_obj.location);

    if (pSrcCaches) {
        for (uint32_t index = 0; index < srcCacheCount; index++) {
            StartReadObject(pSrcCaches[index], record_obj.location);
        }
    }
    // Host access to dstCache must be externally synchronized
}

void ThreadSafety::PostCallRecordMergePipelineCaches(VkDevice device, VkPipelineCache dstCache, uint32_t srcCacheCount,
                                                     const VkPipelineCache* pSrcCaches, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishWriteObject(dstCache, record_obj.location);

    if (pSrcCaches) {
        for (uint32_t index = 0; index < srcCacheCount; index++) {
            FinishReadObject(pSrcCaches[index], record_obj.location);
        }
    }
    // Host access to dstCache must be externally synchronized
}

void ThreadSafety::PreCallRecordCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                        const VkGraphicsPipelineCreateInfo* pCreateInfos,
                                                        const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                        const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(pipelineCache, record_obj.location);
}

void ThreadSafety::PostCallRecordCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                         const VkGraphicsPipelineCreateInfo* pCreateInfos,
                                                         const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                         const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(pipelineCache, record_obj.location);
    if (pPipelines) {
        for (uint32_t index = 0; index < createInfoCount; index++) {
            if (!pPipelines[index]) continue;
            CreateObject(pPipelines[index]);
        }
    }
}

void ThreadSafety::PreCallRecordCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                       const VkComputePipelineCreateInfo* pCreateInfos,
                                                       const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                       const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(pipelineCache, record_obj.location);
}

void ThreadSafety::PostCallRecordCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                        const VkComputePipelineCreateInfo* pCreateInfos,
                                                        const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                        const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(pipelineCache, record_obj.location);
    if (pPipelines) {
        for (uint32_t index = 0; index < createInfoCount; index++) {
            if (!pPipelines[index]) continue;
            CreateObject(pPipelines[index]);
        }
    }
}

void ThreadSafety::PreCallRecordDestroyPipeline(VkDevice device, VkPipeline pipeline, const VkAllocationCallbacks* pAllocator,
                                                const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartWriteObject(pipeline, record_obj.location);
    // Host access to pipeline must be externally synchronized
}

void ThreadSafety::PostCallRecordDestroyPipeline(VkDevice device, VkPipeline pipeline, const VkAllocationCallbacks* pAllocator,
                                                 const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishWriteObject(pipeline, record_obj.location);
    DestroyObject(pipeline);
    // Host access to pipeline must be externally synchronized
}

void ThreadSafety::PreCallRecordCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo* pCreateInfo,
                                                     const VkAllocationCallbacks* pAllocator, VkPipelineLayout* pPipelineLayout,
                                                     const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo* pCreateInfo,
                                                      const VkAllocationCallbacks* pAllocator, VkPipelineLayout* pPipelineLayout,
                                                      const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pPipelineLayout);
    }
}

void ThreadSafety::PreCallRecordDestroyPipelineLayout(VkDevice device, VkPipelineLayout pipelineLayout,
                                                      const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartWriteObject(pipelineLayout, record_obj.location);
    // Host access to pipelineLayout must be externally synchronized
}

void ThreadSafety::PostCallRecordDestroyPipelineLayout(VkDevice device, VkPipelineLayout pipelineLayout,
                                                       const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishWriteObject(pipelineLayout, record_obj.location);
    DestroyObject(pipelineLayout);
    // Host access to pipelineLayout must be externally synchronized
}

void ThreadSafety::PreCallRecordCreateSampler(VkDevice device, const VkSamplerCreateInfo* pCreateInfo,
                                              const VkAllocationCallbacks* pAllocator, VkSampler* pSampler,
                                              const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordCreateSampler(VkDevice device, const VkSamplerCreateInfo* pCreateInfo,
                                               const VkAllocationCallbacks* pAllocator, VkSampler* pSampler,
                                               const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pSampler);
    }
}

void ThreadSafety::PreCallRecordDestroySampler(VkDevice device, VkSampler sampler, const VkAllocationCallbacks* pAllocator,
                                               const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartWriteObject(sampler, record_obj.location);
    // Host access to sampler must be externally synchronized
}

void ThreadSafety::PostCallRecordDestroySampler(VkDevice device, VkSampler sampler, const VkAllocationCallbacks* pAllocator,
                                                const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishWriteObject(sampler, record_obj.location);
    DestroyObject(sampler);
    // Host access to sampler must be externally synchronized
}

void ThreadSafety::PreCallRecordDestroyDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout descriptorSetLayout,
                                                           const VkAllocationCallbacks* pAllocator,
                                                           const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartWriteObject(descriptorSetLayout, record_obj.location);
    // Host access to descriptorSetLayout must be externally synchronized
}

void ThreadSafety::PostCallRecordDestroyDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout descriptorSetLayout,
                                                            const VkAllocationCallbacks* pAllocator,
                                                            const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishWriteObject(descriptorSetLayout, record_obj.location);
    DestroyObject(descriptorSetLayout);
    // Host access to descriptorSetLayout must be externally synchronized
}

void ThreadSafety::PreCallRecordCreateDescriptorPool(VkDevice device, const VkDescriptorPoolCreateInfo* pCreateInfo,
                                                     const VkAllocationCallbacks* pAllocator, VkDescriptorPool* pDescriptorPool,
                                                     const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordCreateDescriptorPool(VkDevice device, const VkDescriptorPoolCreateInfo* pCreateInfo,
                                                      const VkAllocationCallbacks* pAllocator, VkDescriptorPool* pDescriptorPool,
                                                      const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pDescriptorPool);
    }
}

void ThreadSafety::PreCallRecordCreateFramebuffer(VkDevice device, const VkFramebufferCreateInfo* pCreateInfo,
                                                  const VkAllocationCallbacks* pAllocator, VkFramebuffer* pFramebuffer,
                                                  const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordCreateFramebuffer(VkDevice device, const VkFramebufferCreateInfo* pCreateInfo,
                                                   const VkAllocationCallbacks* pAllocator, VkFramebuffer* pFramebuffer,
                                                   const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pFramebuffer);
    }
}

void ThreadSafety::PreCallRecordDestroyFramebuffer(VkDevice device, VkFramebuffer framebuffer,
                                                   const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartWriteObject(framebuffer, record_obj.location);
    // Host access to framebuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordDestroyFramebuffer(VkDevice device, VkFramebuffer framebuffer,
                                                    const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishWriteObject(framebuffer, record_obj.location);
    DestroyObject(framebuffer);
    // Host access to framebuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCreateRenderPass(VkDevice device, const VkRenderPassCreateInfo* pCreateInfo,
                                                 const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass,
                                                 const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordCreateRenderPass(VkDevice device, const VkRenderPassCreateInfo* pCreateInfo,
                                                  const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass,
                                                  const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pRenderPass);
    }
}

void ThreadSafety::PreCallRecordDestroyRenderPass(VkDevice device, VkRenderPass renderPass, const VkAllocationCallbacks* pAllocator,
                                                  const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartWriteObject(renderPass, record_obj.location);
    // Host access to renderPass must be externally synchronized
}

void ThreadSafety::PostCallRecordDestroyRenderPass(VkDevice device, VkRenderPass renderPass,
                                                   const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishWriteObject(renderPass, record_obj.location);
    DestroyObject(renderPass);
    // Host access to renderPass must be externally synchronized
}

void ThreadSafety::PreCallRecordGetRenderAreaGranularity(VkDevice device, VkRenderPass renderPass, VkExtent2D* pGranularity,
                                                         const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(renderPass, record_obj.location);
}

void ThreadSafety::PostCallRecordGetRenderAreaGranularity(VkDevice device, VkRenderPass renderPass, VkExtent2D* pGranularity,
                                                          const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(renderPass, record_obj.location);
}

void ThreadSafety::PreCallRecordBeginCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo* pBeginInfo,
                                                   const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
    // the sname:VkCommandPool that pname:commandBuffer was allocated from must be externally synchronized between host accesses
}

void ThreadSafety::PostCallRecordBeginCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo* pBeginInfo,
                                                    const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
    // the sname:VkCommandPool that pname:commandBuffer was allocated from must be externally synchronized between host accesses
}

void ThreadSafety::PreCallRecordEndCommandBuffer(VkCommandBuffer commandBuffer, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
    // the sname:VkCommandPool that pname:commandBuffer was allocated from must be externally synchronized between host accesses
}

void ThreadSafety::PostCallRecordEndCommandBuffer(VkCommandBuffer commandBuffer, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
    // the sname:VkCommandPool that pname:commandBuffer was allocated from must be externally synchronized between host accesses
}

void ThreadSafety::PreCallRecordResetCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferResetFlags flags,
                                                   const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
    // the sname:VkCommandPool that pname:commandBuffer was allocated from must be externally synchronized between host accesses
}

void ThreadSafety::PostCallRecordResetCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferResetFlags flags,
                                                    const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
    // the sname:VkCommandPool that pname:commandBuffer was allocated from must be externally synchronized between host accesses
}

void ThreadSafety::PreCallRecordCmdBindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                VkPipeline pipeline, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    StartReadObject(pipeline, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdBindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                 VkPipeline pipeline, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    FinishReadObject(pipeline, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetViewport(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount,
                                               const VkViewport* pViewports, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetViewport(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount,
                                                const VkViewport* pViewports, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetScissor(VkCommandBuffer commandBuffer, uint32_t firstScissor, uint32_t scissorCount,
                                              const VkRect2D* pScissors, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetScissor(VkCommandBuffer commandBuffer, uint32_t firstScissor, uint32_t scissorCount,
                                               const VkRect2D* pScissors, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetLineWidth(VkCommandBuffer commandBuffer, float lineWidth, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetLineWidth(VkCommandBuffer commandBuffer, float lineWidth, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetDepthBias(VkCommandBuffer commandBuffer, float depthBiasConstantFactor, float depthBiasClamp,
                                                float depthBiasSlopeFactor, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetDepthBias(VkCommandBuffer commandBuffer, float depthBiasConstantFactor, float depthBiasClamp,
                                                 float depthBiasSlopeFactor, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetBlendConstants(VkCommandBuffer commandBuffer, const float blendConstants[4],
                                                     const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetBlendConstants(VkCommandBuffer commandBuffer, const float blendConstants[4],
                                                      const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetDepthBounds(VkCommandBuffer commandBuffer, float minDepthBounds, float maxDepthBounds,
                                                  const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetDepthBounds(VkCommandBuffer commandBuffer, float minDepthBounds, float maxDepthBounds,
                                                   const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetStencilCompareMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask,
                                                         uint32_t compareMask, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetStencilCompareMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask,
                                                          uint32_t compareMask, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetStencilWriteMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask,
                                                       uint32_t writeMask, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetStencilWriteMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask,
                                                        uint32_t writeMask, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetStencilReference(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask,
                                                       uint32_t reference, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetStencilReference(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask,
                                                        uint32_t reference, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdBindDescriptorSets(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                      VkPipelineLayout layout, uint32_t firstSet, uint32_t descriptorSetCount,
                                                      const VkDescriptorSet* pDescriptorSets, uint32_t dynamicOffsetCount,
                                                      const uint32_t* pDynamicOffsets, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    StartReadObject(layout, record_obj.location);

    if (pDescriptorSets) {
        for (uint32_t index = 0; index < descriptorSetCount; index++) {
            StartReadObject(pDescriptorSets[index], record_obj.location);
        }
    }
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdBindDescriptorSets(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                       VkPipelineLayout layout, uint32_t firstSet, uint32_t descriptorSetCount,
                                                       const VkDescriptorSet* pDescriptorSets, uint32_t dynamicOffsetCount,
                                                       const uint32_t* pDynamicOffsets, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    FinishReadObject(layout, record_obj.location);

    if (pDescriptorSets) {
        for (uint32_t index = 0; index < descriptorSetCount; index++) {
            FinishReadObject(pDescriptorSets[index], record_obj.location);
        }
    }
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdBindIndexBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                   VkIndexType indexType, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    StartReadObject(buffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdBindIndexBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                    VkIndexType indexType, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    FinishReadObject(buffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdBindVertexBuffers(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount,
                                                     const VkBuffer* pBuffers, const VkDeviceSize* pOffsets,
                                                     const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);

    if (pBuffers) {
        for (uint32_t index = 0; index < bindingCount; index++) {
            StartReadObject(pBuffers[index], record_obj.location);
        }
    }
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdBindVertexBuffers(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount,
                                                      const VkBuffer* pBuffers, const VkDeviceSize* pOffsets,
                                                      const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);

    if (pBuffers) {
        for (uint32_t index = 0; index < bindingCount; index++) {
            FinishReadObject(pBuffers[index], record_obj.location);
        }
    }
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount,
                                        uint32_t firstVertex, uint32_t firstInstance, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount,
                                         uint32_t firstVertex, uint32_t firstInstance, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount,
                                               uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance,
                                               const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount,
                                                uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance,
                                                const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                uint32_t drawCount, uint32_t stride, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    StartReadObject(buffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                 uint32_t drawCount, uint32_t stride, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    FinishReadObject(buffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                       uint32_t drawCount, uint32_t stride, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    StartReadObject(buffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                        uint32_t drawCount, uint32_t stride, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    FinishReadObject(buffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdDispatch(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY,
                                            uint32_t groupCountZ, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdDispatch(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY,
                                             uint32_t groupCountZ, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                    const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    StartReadObject(buffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                     const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    FinishReadObject(buffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer,
                                              uint32_t regionCount, const VkBufferCopy* pRegions, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    StartReadObject(srcBuffer, record_obj.location);
    StartReadObject(dstBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer,
                                               uint32_t regionCount, const VkBufferCopy* pRegions, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    FinishReadObject(srcBuffer, record_obj.location);
    FinishReadObject(dstBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                             VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                             const VkImageCopy* pRegions, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    StartReadObject(srcImage, record_obj.location);
    StartReadObject(dstImage, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                              VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                              const VkImageCopy* pRegions, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    FinishReadObject(srcImage, record_obj.location);
    FinishReadObject(dstImage, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                             VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                             const VkImageBlit* pRegions, VkFilter filter, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    StartReadObject(srcImage, record_obj.location);
    StartReadObject(dstImage, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                              VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                              const VkImageBlit* pRegions, VkFilter filter, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    FinishReadObject(srcImage, record_obj.location);
    FinishReadObject(dstImage, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage,
                                                     VkImageLayout dstImageLayout, uint32_t regionCount,
                                                     const VkBufferImageCopy* pRegions, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    StartReadObject(srcBuffer, record_obj.location);
    StartReadObject(dstImage, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage,
                                                      VkImageLayout dstImageLayout, uint32_t regionCount,
                                                      const VkBufferImageCopy* pRegions, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    FinishReadObject(srcBuffer, record_obj.location);
    FinishReadObject(dstImage, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                                     VkBuffer dstBuffer, uint32_t regionCount, const VkBufferImageCopy* pRegions,
                                                     const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    StartReadObject(srcImage, record_obj.location);
    StartReadObject(dstBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                                      VkBuffer dstBuffer, uint32_t regionCount, const VkBufferImageCopy* pRegions,
                                                      const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    FinishReadObject(srcImage, record_obj.location);
    FinishReadObject(dstBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                                VkDeviceSize dataSize, const void* pData, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    StartReadObject(dstBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                                 VkDeviceSize dataSize, const void* pData, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    FinishReadObject(dstBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                              VkDeviceSize size, uint32_t data, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    StartReadObject(dstBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                               VkDeviceSize size, uint32_t data, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    FinishReadObject(dstBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                                   const VkClearColorValue* pColor, uint32_t rangeCount,
                                                   const VkImageSubresourceRange* pRanges, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    StartReadObject(image, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                                    const VkClearColorValue* pColor, uint32_t rangeCount,
                                                    const VkImageSubresourceRange* pRanges, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    FinishReadObject(image, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                                          const VkClearDepthStencilValue* pDepthStencil, uint32_t rangeCount,
                                                          const VkImageSubresourceRange* pRanges, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    StartReadObject(image, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                                           const VkClearDepthStencilValue* pDepthStencil, uint32_t rangeCount,
                                                           const VkImageSubresourceRange* pRanges, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    FinishReadObject(image, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
                                                    const VkClearAttachment* pAttachments, uint32_t rectCount,
                                                    const VkClearRect* pRects, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
                                                     const VkClearAttachment* pAttachments, uint32_t rectCount,
                                                     const VkClearRect* pRects, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                                VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                                const VkImageResolve* pRegions, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    StartReadObject(srcImage, record_obj.location);
    StartReadObject(dstImage, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                                 VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                                 const VkImageResolve* pRegions, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    FinishReadObject(srcImage, record_obj.location);
    FinishReadObject(dstImage, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask,
                                            const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    StartReadObject(event, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask,
                                             const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    FinishReadObject(event, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask,
                                              const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    StartReadObject(event, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask,
                                               const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    FinishReadObject(event, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                              VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask,
                                              uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers,
                                              uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers,
                                              uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers,
                                              const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);

    if (pEvents) {
        for (uint32_t index = 0; index < eventCount; index++) {
            StartReadObject(pEvents[index], record_obj.location);
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
    FinishWriteObject(commandBuffer, record_obj.location);

    if (pEvents) {
        for (uint32_t index = 0; index < eventCount; index++) {
            FinishReadObject(pEvents[index], record_obj.location);
        }
    }
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdPipelineBarrier(
    VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask,
    VkDependencyFlags dependencyFlags, uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers,
    uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount,
    const VkImageMemoryBarrier* pImageMemoryBarriers, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdPipelineBarrier(
    VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask,
    VkDependencyFlags dependencyFlags, uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers,
    uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount,
    const VkImageMemoryBarrier* pImageMemoryBarriers, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdBeginQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query,
                                              VkQueryControlFlags flags, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    StartReadObject(queryPool, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdBeginQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query,
                                               VkQueryControlFlags flags, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    FinishReadObject(queryPool, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdEndQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query,
                                            const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    StartReadObject(queryPool, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdEndQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query,
                                             const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    FinishReadObject(queryPool, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdResetQueryPool(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery,
                                                  uint32_t queryCount, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    StartReadObject(queryPool, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdResetQueryPool(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery,
                                                   uint32_t queryCount, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    FinishReadObject(queryPool, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdWriteTimestamp(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage,
                                                  VkQueryPool queryPool, uint32_t query, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    StartReadObject(queryPool, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdWriteTimestamp(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage,
                                                   VkQueryPool queryPool, uint32_t query, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    FinishReadObject(queryPool, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdCopyQueryPoolResults(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery,
                                                        uint32_t queryCount, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                                        VkDeviceSize stride, VkQueryResultFlags flags,
                                                        const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    StartReadObject(queryPool, record_obj.location);
    StartReadObject(dstBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdCopyQueryPoolResults(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery,
                                                         uint32_t queryCount, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                                         VkDeviceSize stride, VkQueryResultFlags flags,
                                                         const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    FinishReadObject(queryPool, record_obj.location);
    FinishReadObject(dstBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdPushConstants(VkCommandBuffer commandBuffer, VkPipelineLayout layout,
                                                 VkShaderStageFlags stageFlags, uint32_t offset, uint32_t size, const void* pValues,
                                                 const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    StartReadObject(layout, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdPushConstants(VkCommandBuffer commandBuffer, VkPipelineLayout layout,
                                                  VkShaderStageFlags stageFlags, uint32_t offset, uint32_t size,
                                                  const void* pValues, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    FinishReadObject(layout, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                                   VkSubpassContents contents, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                                    VkSubpassContents contents, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents,
                                               const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents,
                                                const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdEndRenderPass(VkCommandBuffer commandBuffer, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdEndRenderPass(VkCommandBuffer commandBuffer, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBufferCount,
                                                   const VkCommandBuffer* pCommandBuffers, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);

    if (pCommandBuffers) {
        for (uint32_t index = 0; index < commandBufferCount; index++) {
            StartReadObject(pCommandBuffers[index], record_obj.location);
        }
    }
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBufferCount,
                                                    const VkCommandBuffer* pCommandBuffers, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);

    if (pCommandBuffers) {
        for (uint32_t index = 0; index < commandBufferCount; index++) {
            FinishReadObject(pCommandBuffers[index], record_obj.location);
        }
    }
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordBindBufferMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindBufferMemoryInfo* pBindInfos,
                                                  const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordBindBufferMemory2(VkDevice device, uint32_t bindInfoCount,
                                                   const VkBindBufferMemoryInfo* pBindInfos, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PreCallRecordBindImageMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo* pBindInfos,
                                                 const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordBindImageMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo* pBindInfos,
                                                  const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PreCallRecordGetDeviceGroupPeerMemoryFeatures(VkDevice device, uint32_t heapIndex, uint32_t localDeviceIndex,
                                                                 uint32_t remoteDeviceIndex,
                                                                 VkPeerMemoryFeatureFlags* pPeerMemoryFeatures,
                                                                 const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordGetDeviceGroupPeerMemoryFeatures(VkDevice device, uint32_t heapIndex, uint32_t localDeviceIndex,
                                                                  uint32_t remoteDeviceIndex,
                                                                  VkPeerMemoryFeatureFlags* pPeerMemoryFeatures,
                                                                  const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PreCallRecordCmdSetDeviceMask(VkCommandBuffer commandBuffer, uint32_t deviceMask,
                                                 const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetDeviceMask(VkCommandBuffer commandBuffer, uint32_t deviceMask,
                                                  const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdDispatchBase(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY,
                                                uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY,
                                                uint32_t groupCountZ, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdDispatchBase(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY,
                                                 uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY,
                                                 uint32_t groupCountZ, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordEnumeratePhysicalDeviceGroups(VkInstance instance, uint32_t* pPhysicalDeviceGroupCount,
                                                              VkPhysicalDeviceGroupProperties* pPhysicalDeviceGroupProperties,
                                                              const RecordObject& record_obj) {
    StartReadObjectParentInstance(instance, record_obj.location);
}

void ThreadSafety::PostCallRecordEnumeratePhysicalDeviceGroups(VkInstance instance, uint32_t* pPhysicalDeviceGroupCount,
                                                               VkPhysicalDeviceGroupProperties* pPhysicalDeviceGroupProperties,
                                                               const RecordObject& record_obj) {
    FinishReadObjectParentInstance(instance, record_obj.location);
}

void ThreadSafety::PreCallRecordGetImageMemoryRequirements2(VkDevice device, const VkImageMemoryRequirementsInfo2* pInfo,
                                                            VkMemoryRequirements2* pMemoryRequirements,
                                                            const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordGetImageMemoryRequirements2(VkDevice device, const VkImageMemoryRequirementsInfo2* pInfo,
                                                             VkMemoryRequirements2* pMemoryRequirements,
                                                             const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PreCallRecordGetBufferMemoryRequirements2(VkDevice device, const VkBufferMemoryRequirementsInfo2* pInfo,
                                                             VkMemoryRequirements2* pMemoryRequirements,
                                                             const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordGetBufferMemoryRequirements2(VkDevice device, const VkBufferMemoryRequirementsInfo2* pInfo,
                                                              VkMemoryRequirements2* pMemoryRequirements,
                                                              const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PreCallRecordGetImageSparseMemoryRequirements2(VkDevice device,
                                                                  const VkImageSparseMemoryRequirementsInfo2* pInfo,
                                                                  uint32_t* pSparseMemoryRequirementCount,
                                                                  VkSparseImageMemoryRequirements2* pSparseMemoryRequirements,
                                                                  const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordGetImageSparseMemoryRequirements2(VkDevice device,
                                                                   const VkImageSparseMemoryRequirementsInfo2* pInfo,
                                                                   uint32_t* pSparseMemoryRequirementCount,
                                                                   VkSparseImageMemoryRequirements2* pSparseMemoryRequirements,
                                                                   const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PreCallRecordTrimCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolTrimFlags flags,
                                                const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartWriteObject(commandPool, record_obj.location);
    // Host access to commandPool must be externally synchronized
}

void ThreadSafety::PostCallRecordTrimCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolTrimFlags flags,
                                                 const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishWriteObject(commandPool, record_obj.location);
    // Host access to commandPool must be externally synchronized
}

void ThreadSafety::PreCallRecordCreateSamplerYcbcrConversion(VkDevice device, const VkSamplerYcbcrConversionCreateInfo* pCreateInfo,
                                                             const VkAllocationCallbacks* pAllocator,
                                                             VkSamplerYcbcrConversion* pYcbcrConversion,
                                                             const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordCreateSamplerYcbcrConversion(VkDevice device,
                                                              const VkSamplerYcbcrConversionCreateInfo* pCreateInfo,
                                                              const VkAllocationCallbacks* pAllocator,
                                                              VkSamplerYcbcrConversion* pYcbcrConversion,
                                                              const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pYcbcrConversion);
    }
}

void ThreadSafety::PreCallRecordDestroySamplerYcbcrConversion(VkDevice device, VkSamplerYcbcrConversion ycbcrConversion,
                                                              const VkAllocationCallbacks* pAllocator,
                                                              const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartWriteObject(ycbcrConversion, record_obj.location);
    // Host access to ycbcrConversion must be externally synchronized
}

void ThreadSafety::PostCallRecordDestroySamplerYcbcrConversion(VkDevice device, VkSamplerYcbcrConversion ycbcrConversion,
                                                               const VkAllocationCallbacks* pAllocator,
                                                               const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishWriteObject(ycbcrConversion, record_obj.location);
    DestroyObject(ycbcrConversion);
    // Host access to ycbcrConversion must be externally synchronized
}

void ThreadSafety::PreCallRecordCreateDescriptorUpdateTemplate(VkDevice device,
                                                               const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo,
                                                               const VkAllocationCallbacks* pAllocator,
                                                               VkDescriptorUpdateTemplate* pDescriptorUpdateTemplate,
                                                               const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordCreateDescriptorUpdateTemplate(VkDevice device,
                                                                const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo,
                                                                const VkAllocationCallbacks* pAllocator,
                                                                VkDescriptorUpdateTemplate* pDescriptorUpdateTemplate,
                                                                const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pDescriptorUpdateTemplate);
    }
}

void ThreadSafety::PreCallRecordDestroyDescriptorUpdateTemplate(VkDevice device,
                                                                VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                                const VkAllocationCallbacks* pAllocator,
                                                                const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartWriteObject(descriptorUpdateTemplate, record_obj.location);
    // Host access to descriptorUpdateTemplate must be externally synchronized
}

void ThreadSafety::PostCallRecordDestroyDescriptorUpdateTemplate(VkDevice device,
                                                                 VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                                 const VkAllocationCallbacks* pAllocator,
                                                                 const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishWriteObject(descriptorUpdateTemplate, record_obj.location);
    DestroyObject(descriptorUpdateTemplate);
    // Host access to descriptorUpdateTemplate must be externally synchronized
}

void ThreadSafety::PreCallRecordGetDescriptorSetLayoutSupport(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo,
                                                              VkDescriptorSetLayoutSupport* pSupport,
                                                              const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordGetDescriptorSetLayoutSupport(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo,
                                                               VkDescriptorSetLayoutSupport* pSupport,
                                                               const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PreCallRecordCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                     VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                     uint32_t stride, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    StartReadObject(buffer, record_obj.location);
    StartReadObject(countBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                      VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                      uint32_t stride, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    FinishReadObject(buffer, record_obj.location);
    FinishReadObject(countBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                            VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                            uint32_t maxDrawCount, uint32_t stride,
                                                            const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    StartReadObject(buffer, record_obj.location);
    StartReadObject(countBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                             VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                             uint32_t maxDrawCount, uint32_t stride,
                                                             const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    FinishReadObject(buffer, record_obj.location);
    FinishReadObject(countBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCreateRenderPass2(VkDevice device, const VkRenderPassCreateInfo2* pCreateInfo,
                                                  const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass,
                                                  const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordCreateRenderPass2(VkDevice device, const VkRenderPassCreateInfo2* pCreateInfo,
                                                   const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass,
                                                   const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pRenderPass);
    }
}

void ThreadSafety::PreCallRecordCmdBeginRenderPass2(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                                    const VkSubpassBeginInfo* pSubpassBeginInfo, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdBeginRenderPass2(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                                     const VkSubpassBeginInfo* pSubpassBeginInfo, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdNextSubpass2(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo* pSubpassBeginInfo,
                                                const VkSubpassEndInfo* pSubpassEndInfo, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdNextSubpass2(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo* pSubpassBeginInfo,
                                                 const VkSubpassEndInfo* pSubpassEndInfo, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdEndRenderPass2(VkCommandBuffer commandBuffer, const VkSubpassEndInfo* pSubpassEndInfo,
                                                  const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdEndRenderPass2(VkCommandBuffer commandBuffer, const VkSubpassEndInfo* pSubpassEndInfo,
                                                   const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordResetQueryPool(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount,
                                               const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(queryPool, record_obj.location);
}

void ThreadSafety::PostCallRecordResetQueryPool(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount,
                                                const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(queryPool, record_obj.location);
}

void ThreadSafety::PreCallRecordGetSemaphoreCounterValue(VkDevice device, VkSemaphore semaphore, uint64_t* pValue,
                                                         const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(semaphore, record_obj.location);
}

void ThreadSafety::PostCallRecordGetSemaphoreCounterValue(VkDevice device, VkSemaphore semaphore, uint64_t* pValue,
                                                          const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(semaphore, record_obj.location);
}

void ThreadSafety::PreCallRecordWaitSemaphores(VkDevice device, const VkSemaphoreWaitInfo* pWaitInfo, uint64_t timeout,
                                               const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordWaitSemaphores(VkDevice device, const VkSemaphoreWaitInfo* pWaitInfo, uint64_t timeout,
                                                const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PreCallRecordSignalSemaphore(VkDevice device, const VkSemaphoreSignalInfo* pSignalInfo,
                                                const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordSignalSemaphore(VkDevice device, const VkSemaphoreSignalInfo* pSignalInfo,
                                                 const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PreCallRecordGetBufferDeviceAddress(VkDevice device, const VkBufferDeviceAddressInfo* pInfo,
                                                       const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordGetBufferDeviceAddress(VkDevice device, const VkBufferDeviceAddressInfo* pInfo,
                                                        const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PreCallRecordGetBufferOpaqueCaptureAddress(VkDevice device, const VkBufferDeviceAddressInfo* pInfo,
                                                              const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordGetBufferOpaqueCaptureAddress(VkDevice device, const VkBufferDeviceAddressInfo* pInfo,
                                                               const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PreCallRecordGetDeviceMemoryOpaqueCaptureAddress(VkDevice device,
                                                                    const VkDeviceMemoryOpaqueCaptureAddressInfo* pInfo,
                                                                    const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordGetDeviceMemoryOpaqueCaptureAddress(VkDevice device,
                                                                     const VkDeviceMemoryOpaqueCaptureAddressInfo* pInfo,
                                                                     const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PreCallRecordCreatePrivateDataSlot(VkDevice device, const VkPrivateDataSlotCreateInfo* pCreateInfo,
                                                      const VkAllocationCallbacks* pAllocator, VkPrivateDataSlot* pPrivateDataSlot,
                                                      const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordCreatePrivateDataSlot(VkDevice device, const VkPrivateDataSlotCreateInfo* pCreateInfo,
                                                       const VkAllocationCallbacks* pAllocator, VkPrivateDataSlot* pPrivateDataSlot,
                                                       const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pPrivateDataSlot);
    }
}

void ThreadSafety::PreCallRecordDestroyPrivateDataSlot(VkDevice device, VkPrivateDataSlot privateDataSlot,
                                                       const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartWriteObject(privateDataSlot, record_obj.location);
    // Host access to privateDataSlot must be externally synchronized
}

void ThreadSafety::PostCallRecordDestroyPrivateDataSlot(VkDevice device, VkPrivateDataSlot privateDataSlot,
                                                        const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishWriteObject(privateDataSlot, record_obj.location);
    DestroyObject(privateDataSlot);
    // Host access to privateDataSlot must be externally synchronized
}

void ThreadSafety::PreCallRecordSetPrivateData(VkDevice device, VkObjectType objectType, uint64_t objectHandle,
                                               VkPrivateDataSlot privateDataSlot, uint64_t data, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(privateDataSlot, record_obj.location);
}

void ThreadSafety::PostCallRecordSetPrivateData(VkDevice device, VkObjectType objectType, uint64_t objectHandle,
                                                VkPrivateDataSlot privateDataSlot, uint64_t data, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(privateDataSlot, record_obj.location);
}

void ThreadSafety::PreCallRecordGetPrivateData(VkDevice device, VkObjectType objectType, uint64_t objectHandle,
                                               VkPrivateDataSlot privateDataSlot, uint64_t* pData, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(privateDataSlot, record_obj.location);
}

void ThreadSafety::PostCallRecordGetPrivateData(VkDevice device, VkObjectType objectType, uint64_t objectHandle,
                                                VkPrivateDataSlot privateDataSlot, uint64_t* pData,
                                                const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(privateDataSlot, record_obj.location);
}

void ThreadSafety::PreCallRecordCmdSetEvent2(VkCommandBuffer commandBuffer, VkEvent event, const VkDependencyInfo* pDependencyInfo,
                                             const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    StartReadObject(event, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetEvent2(VkCommandBuffer commandBuffer, VkEvent event, const VkDependencyInfo* pDependencyInfo,
                                              const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    FinishReadObject(event, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdResetEvent2(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags2 stageMask,
                                               const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    StartReadObject(event, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdResetEvent2(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags2 stageMask,
                                                const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    FinishReadObject(event, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdWaitEvents2(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                               const VkDependencyInfo* pDependencyInfos, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);

    if (pEvents) {
        for (uint32_t index = 0; index < eventCount; index++) {
            StartReadObject(pEvents[index], record_obj.location);
        }
    }
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdWaitEvents2(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                                const VkDependencyInfo* pDependencyInfos, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);

    if (pEvents) {
        for (uint32_t index = 0; index < eventCount; index++) {
            FinishReadObject(pEvents[index], record_obj.location);
        }
    }
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdPipelineBarrier2(VkCommandBuffer commandBuffer, const VkDependencyInfo* pDependencyInfo,
                                                    const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdPipelineBarrier2(VkCommandBuffer commandBuffer, const VkDependencyInfo* pDependencyInfo,
                                                     const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdWriteTimestamp2(VkCommandBuffer commandBuffer, VkPipelineStageFlags2 stage,
                                                   VkQueryPool queryPool, uint32_t query, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    StartReadObject(queryPool, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdWriteTimestamp2(VkCommandBuffer commandBuffer, VkPipelineStageFlags2 stage,
                                                    VkQueryPool queryPool, uint32_t query, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    FinishReadObject(queryPool, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordQueueSubmit2(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2* pSubmits, VkFence fence,
                                             const RecordObject& record_obj) {
    StartWriteObject(queue, record_obj.location);
    StartWriteObject(fence, record_obj.location);
    // Host access to queue must be externally synchronized
    // Host access to fence must be externally synchronized
}

void ThreadSafety::PostCallRecordQueueSubmit2(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2* pSubmits, VkFence fence,
                                              const RecordObject& record_obj) {
    FinishWriteObject(queue, record_obj.location);
    FinishWriteObject(fence, record_obj.location);
    // Host access to queue must be externally synchronized
    // Host access to fence must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdCopyBuffer2(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2* pCopyBufferInfo,
                                               const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdCopyBuffer2(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2* pCopyBufferInfo,
                                                const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdCopyImage2(VkCommandBuffer commandBuffer, const VkCopyImageInfo2* pCopyImageInfo,
                                              const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdCopyImage2(VkCommandBuffer commandBuffer, const VkCopyImageInfo2* pCopyImageInfo,
                                               const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdCopyBufferToImage2(VkCommandBuffer commandBuffer,
                                                      const VkCopyBufferToImageInfo2* pCopyBufferToImageInfo,
                                                      const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdCopyBufferToImage2(VkCommandBuffer commandBuffer,
                                                       const VkCopyBufferToImageInfo2* pCopyBufferToImageInfo,
                                                       const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdCopyImageToBuffer2(VkCommandBuffer commandBuffer,
                                                      const VkCopyImageToBufferInfo2* pCopyImageToBufferInfo,
                                                      const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdCopyImageToBuffer2(VkCommandBuffer commandBuffer,
                                                       const VkCopyImageToBufferInfo2* pCopyImageToBufferInfo,
                                                       const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdBlitImage2(VkCommandBuffer commandBuffer, const VkBlitImageInfo2* pBlitImageInfo,
                                              const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdBlitImage2(VkCommandBuffer commandBuffer, const VkBlitImageInfo2* pBlitImageInfo,
                                               const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdResolveImage2(VkCommandBuffer commandBuffer, const VkResolveImageInfo2* pResolveImageInfo,
                                                 const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdResolveImage2(VkCommandBuffer commandBuffer, const VkResolveImageInfo2* pResolveImageInfo,
                                                  const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdBeginRendering(VkCommandBuffer commandBuffer, const VkRenderingInfo* pRenderingInfo,
                                                  const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdBeginRendering(VkCommandBuffer commandBuffer, const VkRenderingInfo* pRenderingInfo,
                                                   const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdEndRendering(VkCommandBuffer commandBuffer, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdEndRendering(VkCommandBuffer commandBuffer, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetCullMode(VkCommandBuffer commandBuffer, VkCullModeFlags cullMode,
                                               const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetCullMode(VkCommandBuffer commandBuffer, VkCullModeFlags cullMode,
                                                const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetFrontFace(VkCommandBuffer commandBuffer, VkFrontFace frontFace,
                                                const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetFrontFace(VkCommandBuffer commandBuffer, VkFrontFace frontFace,
                                                 const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetPrimitiveTopology(VkCommandBuffer commandBuffer, VkPrimitiveTopology primitiveTopology,
                                                        const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetPrimitiveTopology(VkCommandBuffer commandBuffer, VkPrimitiveTopology primitiveTopology,
                                                         const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetViewportWithCount(VkCommandBuffer commandBuffer, uint32_t viewportCount,
                                                        const VkViewport* pViewports, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetViewportWithCount(VkCommandBuffer commandBuffer, uint32_t viewportCount,
                                                         const VkViewport* pViewports, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetScissorWithCount(VkCommandBuffer commandBuffer, uint32_t scissorCount,
                                                       const VkRect2D* pScissors, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetScissorWithCount(VkCommandBuffer commandBuffer, uint32_t scissorCount,
                                                        const VkRect2D* pScissors, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdBindVertexBuffers2(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount,
                                                      const VkBuffer* pBuffers, const VkDeviceSize* pOffsets,
                                                      const VkDeviceSize* pSizes, const VkDeviceSize* pStrides,
                                                      const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);

    if (pBuffers) {
        for (uint32_t index = 0; index < bindingCount; index++) {
            StartReadObject(pBuffers[index], record_obj.location);
        }
    }
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdBindVertexBuffers2(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount,
                                                       const VkBuffer* pBuffers, const VkDeviceSize* pOffsets,
                                                       const VkDeviceSize* pSizes, const VkDeviceSize* pStrides,
                                                       const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);

    if (pBuffers) {
        for (uint32_t index = 0; index < bindingCount; index++) {
            FinishReadObject(pBuffers[index], record_obj.location);
        }
    }
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetDepthTestEnable(VkCommandBuffer commandBuffer, VkBool32 depthTestEnable,
                                                      const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetDepthTestEnable(VkCommandBuffer commandBuffer, VkBool32 depthTestEnable,
                                                       const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetDepthWriteEnable(VkCommandBuffer commandBuffer, VkBool32 depthWriteEnable,
                                                       const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetDepthWriteEnable(VkCommandBuffer commandBuffer, VkBool32 depthWriteEnable,
                                                        const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetDepthCompareOp(VkCommandBuffer commandBuffer, VkCompareOp depthCompareOp,
                                                     const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetDepthCompareOp(VkCommandBuffer commandBuffer, VkCompareOp depthCompareOp,
                                                      const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetDepthBoundsTestEnable(VkCommandBuffer commandBuffer, VkBool32 depthBoundsTestEnable,
                                                            const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetDepthBoundsTestEnable(VkCommandBuffer commandBuffer, VkBool32 depthBoundsTestEnable,
                                                             const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetStencilTestEnable(VkCommandBuffer commandBuffer, VkBool32 stencilTestEnable,
                                                        const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetStencilTestEnable(VkCommandBuffer commandBuffer, VkBool32 stencilTestEnable,
                                                         const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetStencilOp(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, VkStencilOp failOp,
                                                VkStencilOp passOp, VkStencilOp depthFailOp, VkCompareOp compareOp,
                                                const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetStencilOp(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, VkStencilOp failOp,
                                                 VkStencilOp passOp, VkStencilOp depthFailOp, VkCompareOp compareOp,
                                                 const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetRasterizerDiscardEnable(VkCommandBuffer commandBuffer, VkBool32 rasterizerDiscardEnable,
                                                              const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetRasterizerDiscardEnable(VkCommandBuffer commandBuffer, VkBool32 rasterizerDiscardEnable,
                                                               const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetDepthBiasEnable(VkCommandBuffer commandBuffer, VkBool32 depthBiasEnable,
                                                      const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetDepthBiasEnable(VkCommandBuffer commandBuffer, VkBool32 depthBiasEnable,
                                                       const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetPrimitiveRestartEnable(VkCommandBuffer commandBuffer, VkBool32 primitiveRestartEnable,
                                                             const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetPrimitiveRestartEnable(VkCommandBuffer commandBuffer, VkBool32 primitiveRestartEnable,
                                                              const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordGetDeviceBufferMemoryRequirements(VkDevice device, const VkDeviceBufferMemoryRequirements* pInfo,
                                                                  VkMemoryRequirements2* pMemoryRequirements,
                                                                  const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordGetDeviceBufferMemoryRequirements(VkDevice device, const VkDeviceBufferMemoryRequirements* pInfo,
                                                                   VkMemoryRequirements2* pMemoryRequirements,
                                                                   const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PreCallRecordGetDeviceImageMemoryRequirements(VkDevice device, const VkDeviceImageMemoryRequirements* pInfo,
                                                                 VkMemoryRequirements2* pMemoryRequirements,
                                                                 const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordGetDeviceImageMemoryRequirements(VkDevice device, const VkDeviceImageMemoryRequirements* pInfo,
                                                                  VkMemoryRequirements2* pMemoryRequirements,
                                                                  const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PreCallRecordGetDeviceImageSparseMemoryRequirements(VkDevice device,
                                                                       const VkDeviceImageMemoryRequirements* pInfo,
                                                                       uint32_t* pSparseMemoryRequirementCount,
                                                                       VkSparseImageMemoryRequirements2* pSparseMemoryRequirements,
                                                                       const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordGetDeviceImageSparseMemoryRequirements(VkDevice device,
                                                                        const VkDeviceImageMemoryRequirements* pInfo,
                                                                        uint32_t* pSparseMemoryRequirementCount,
                                                                        VkSparseImageMemoryRequirements2* pSparseMemoryRequirements,
                                                                        const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PreCallRecordDestroySurfaceKHR(VkInstance instance, VkSurfaceKHR surface,
                                                  const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {
    StartReadObjectParentInstance(instance, record_obj.location);
    StartWriteObjectParentInstance(surface, record_obj.location);
    // Host access to surface must be externally synchronized
}

void ThreadSafety::PostCallRecordDestroySurfaceKHR(VkInstance instance, VkSurfaceKHR surface,
                                                   const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(instance, record_obj.location);
    FinishWriteObjectParentInstance(surface, record_obj.location);
    DestroyObjectParentInstance(surface);
    // Host access to surface must be externally synchronized
}

void ThreadSafety::PreCallRecordGetPhysicalDeviceSurfaceSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
                                                                   VkSurfaceKHR surface, VkBool32* pSupported,
                                                                   const RecordObject& record_obj) {
    StartReadObjectParentInstance(surface, record_obj.location);
}

void ThreadSafety::PostCallRecordGetPhysicalDeviceSurfaceSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
                                                                    VkSurfaceKHR surface, VkBool32* pSupported,
                                                                    const RecordObject& record_obj) {
    FinishReadObjectParentInstance(surface, record_obj.location);
}

void ThreadSafety::PreCallRecordGetPhysicalDeviceSurfaceCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                        VkSurfaceCapabilitiesKHR* pSurfaceCapabilities,
                                                                        const RecordObject& record_obj) {
    StartReadObjectParentInstance(surface, record_obj.location);
}

void ThreadSafety::PostCallRecordGetPhysicalDeviceSurfaceCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                         VkSurfaceCapabilitiesKHR* pSurfaceCapabilities,
                                                                         const RecordObject& record_obj) {
    FinishReadObjectParentInstance(surface, record_obj.location);
}

void ThreadSafety::PreCallRecordGetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                   uint32_t* pSurfaceFormatCount,
                                                                   VkSurfaceFormatKHR* pSurfaceFormats,
                                                                   const RecordObject& record_obj) {
    StartReadObjectParentInstance(surface, record_obj.location);
}

void ThreadSafety::PostCallRecordGetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                    uint32_t* pSurfaceFormatCount,
                                                                    VkSurfaceFormatKHR* pSurfaceFormats,
                                                                    const RecordObject& record_obj) {
    FinishReadObjectParentInstance(surface, record_obj.location);
}

void ThreadSafety::PreCallRecordGetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                        uint32_t* pPresentModeCount,
                                                                        VkPresentModeKHR* pPresentModes,
                                                                        const RecordObject& record_obj) {
    StartReadObjectParentInstance(surface, record_obj.location);
}

void ThreadSafety::PostCallRecordGetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                         uint32_t* pPresentModeCount,
                                                                         VkPresentModeKHR* pPresentModes,
                                                                         const RecordObject& record_obj) {
    FinishReadObjectParentInstance(surface, record_obj.location);
}

void ThreadSafety::PreCallRecordCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR* pCreateInfo,
                                                   const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain,
                                                   const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartWriteObjectParentInstance(pCreateInfo->surface, record_obj.location);
    StartWriteObject(pCreateInfo->oldSwapchain, record_obj.location);
    // Host access to pCreateInfo->surface,pCreateInfo->oldSwapchain must be externally synchronized
}

void ThreadSafety::PostCallRecordCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR* pCreateInfo,
                                                    const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain,
                                                    const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishWriteObjectParentInstance(pCreateInfo->surface, record_obj.location);
    FinishWriteObject(pCreateInfo->oldSwapchain, record_obj.location);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pSwapchain);
    }
    // Host access to pCreateInfo->surface,pCreateInfo->oldSwapchain must be externally synchronized
}

void ThreadSafety::PreCallRecordAcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout,
                                                    VkSemaphore semaphore, VkFence fence, uint32_t* pImageIndex,
                                                    const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartWriteObject(swapchain, record_obj.location);
    StartWriteObject(semaphore, record_obj.location);
    StartWriteObject(fence, record_obj.location);
    // Host access to swapchain must be externally synchronized
    // Host access to semaphore must be externally synchronized
    // Host access to fence must be externally synchronized
}

void ThreadSafety::PostCallRecordAcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout,
                                                     VkSemaphore semaphore, VkFence fence, uint32_t* pImageIndex,
                                                     const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishWriteObject(swapchain, record_obj.location);
    FinishWriteObject(semaphore, record_obj.location);
    FinishWriteObject(fence, record_obj.location);
    // Host access to swapchain must be externally synchronized
    // Host access to semaphore must be externally synchronized
    // Host access to fence must be externally synchronized
}

void ThreadSafety::PreCallRecordGetDeviceGroupPresentCapabilitiesKHR(
    VkDevice device, VkDeviceGroupPresentCapabilitiesKHR* pDeviceGroupPresentCapabilities, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordGetDeviceGroupPresentCapabilitiesKHR(
    VkDevice device, VkDeviceGroupPresentCapabilitiesKHR* pDeviceGroupPresentCapabilities, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PreCallRecordGetDeviceGroupSurfacePresentModesKHR(VkDevice device, VkSurfaceKHR surface,
                                                                     VkDeviceGroupPresentModeFlagsKHR* pModes,
                                                                     const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartWriteObjectParentInstance(surface, record_obj.location);
    // Host access to surface must be externally synchronized
}

void ThreadSafety::PostCallRecordGetDeviceGroupSurfacePresentModesKHR(VkDevice device, VkSurfaceKHR surface,
                                                                      VkDeviceGroupPresentModeFlagsKHR* pModes,
                                                                      const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishWriteObjectParentInstance(surface, record_obj.location);
    // Host access to surface must be externally synchronized
}

void ThreadSafety::PreCallRecordGetPhysicalDevicePresentRectanglesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                      uint32_t* pRectCount, VkRect2D* pRects,
                                                                      const RecordObject& record_obj) {
    StartWriteObjectParentInstance(surface, record_obj.location);
    // Host access to surface must be externally synchronized
}

void ThreadSafety::PostCallRecordGetPhysicalDevicePresentRectanglesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                       uint32_t* pRectCount, VkRect2D* pRects,
                                                                       const RecordObject& record_obj) {
    FinishWriteObjectParentInstance(surface, record_obj.location);
    // Host access to surface must be externally synchronized
}

void ThreadSafety::PreCallRecordAcquireNextImage2KHR(VkDevice device, const VkAcquireNextImageInfoKHR* pAcquireInfo,
                                                     uint32_t* pImageIndex, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordAcquireNextImage2KHR(VkDevice device, const VkAcquireNextImageInfoKHR* pAcquireInfo,
                                                      uint32_t* pImageIndex, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PreCallRecordCreateDisplayModeKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display,
                                                     const VkDisplayModeCreateInfoKHR* pCreateInfo,
                                                     const VkAllocationCallbacks* pAllocator, VkDisplayModeKHR* pMode,
                                                     const RecordObject& record_obj) {
    StartWriteObjectParentInstance(display, record_obj.location);
    // Host access to display must be externally synchronized
}

void ThreadSafety::PostCallRecordCreateDisplayModeKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display,
                                                      const VkDisplayModeCreateInfoKHR* pCreateInfo,
                                                      const VkAllocationCallbacks* pAllocator, VkDisplayModeKHR* pMode,
                                                      const RecordObject& record_obj) {
    FinishWriteObjectParentInstance(display, record_obj.location);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pMode);
    }
    // Host access to display must be externally synchronized
}

void ThreadSafety::PreCallRecordGetDisplayPlaneCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkDisplayModeKHR mode,
                                                               uint32_t planeIndex, VkDisplayPlaneCapabilitiesKHR* pCapabilities,
                                                               const RecordObject& record_obj) {
    StartWriteObject(mode, record_obj.location);
    // Host access to mode must be externally synchronized
}

void ThreadSafety::PostCallRecordGetDisplayPlaneCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkDisplayModeKHR mode,
                                                                uint32_t planeIndex, VkDisplayPlaneCapabilitiesKHR* pCapabilities,
                                                                const RecordObject& record_obj) {
    FinishWriteObject(mode, record_obj.location);
    // Host access to mode must be externally synchronized
}

void ThreadSafety::PreCallRecordCreateDisplayPlaneSurfaceKHR(VkInstance instance, const VkDisplaySurfaceCreateInfoKHR* pCreateInfo,
                                                             const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                             const RecordObject& record_obj) {
    StartReadObjectParentInstance(instance, record_obj.location);
}

void ThreadSafety::PostCallRecordCreateDisplayPlaneSurfaceKHR(VkInstance instance, const VkDisplaySurfaceCreateInfoKHR* pCreateInfo,
                                                              const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                              const RecordObject& record_obj) {
    FinishReadObjectParentInstance(instance, record_obj.location);
    if (record_obj.result == VK_SUCCESS) {
        CreateObjectParentInstance(*pSurface);
    }
}

void ThreadSafety::PreCallRecordCreateSharedSwapchainsKHR(VkDevice device, uint32_t swapchainCount,
                                                          const VkSwapchainCreateInfoKHR* pCreateInfos,
                                                          const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchains,
                                                          const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    if (pCreateInfos) {
        for (uint32_t index = 0; index < swapchainCount; index++) {
            StartWriteObjectParentInstance(pCreateInfos[index].surface, record_obj.location);
            StartWriteObject(pCreateInfos[index].oldSwapchain, record_obj.location);
        }
    }

    if (pSwapchains) {
        for (uint32_t index = 0; index < swapchainCount; index++) {
            StartReadObject(pSwapchains[index], record_obj.location);
        }
    }
    // Host access to pCreateInfos[].surface,pCreateInfos[].oldSwapchain must be externally synchronized
}

void ThreadSafety::PostCallRecordCreateSharedSwapchainsKHR(VkDevice device, uint32_t swapchainCount,
                                                           const VkSwapchainCreateInfoKHR* pCreateInfos,
                                                           const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchains,
                                                           const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    if (pCreateInfos) {
        for (uint32_t index = 0; index < swapchainCount; index++) {
            FinishWriteObjectParentInstance(pCreateInfos[index].surface, record_obj.location);
            FinishWriteObject(pCreateInfos[index].oldSwapchain, record_obj.location);
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
                                                     const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                     const RecordObject& record_obj) {
    StartReadObjectParentInstance(instance, record_obj.location);
}

void ThreadSafety::PostCallRecordCreateXlibSurfaceKHR(VkInstance instance, const VkXlibSurfaceCreateInfoKHR* pCreateInfo,
                                                      const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                      const RecordObject& record_obj) {
    FinishReadObjectParentInstance(instance, record_obj.location);
    if (record_obj.result == VK_SUCCESS) {
        CreateObjectParentInstance(*pSurface);
    }
}

#endif  // VK_USE_PLATFORM_XLIB_KHR
#ifdef VK_USE_PLATFORM_XCB_KHR
void ThreadSafety::PreCallRecordCreateXcbSurfaceKHR(VkInstance instance, const VkXcbSurfaceCreateInfoKHR* pCreateInfo,
                                                    const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                    const RecordObject& record_obj) {
    StartReadObjectParentInstance(instance, record_obj.location);
}

void ThreadSafety::PostCallRecordCreateXcbSurfaceKHR(VkInstance instance, const VkXcbSurfaceCreateInfoKHR* pCreateInfo,
                                                     const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                     const RecordObject& record_obj) {
    FinishReadObjectParentInstance(instance, record_obj.location);
    if (record_obj.result == VK_SUCCESS) {
        CreateObjectParentInstance(*pSurface);
    }
}

#endif  // VK_USE_PLATFORM_XCB_KHR
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
void ThreadSafety::PreCallRecordCreateWaylandSurfaceKHR(VkInstance instance, const VkWaylandSurfaceCreateInfoKHR* pCreateInfo,
                                                        const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                        const RecordObject& record_obj) {
    StartReadObjectParentInstance(instance, record_obj.location);
}

void ThreadSafety::PostCallRecordCreateWaylandSurfaceKHR(VkInstance instance, const VkWaylandSurfaceCreateInfoKHR* pCreateInfo,
                                                         const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                         const RecordObject& record_obj) {
    FinishReadObjectParentInstance(instance, record_obj.location);
    if (record_obj.result == VK_SUCCESS) {
        CreateObjectParentInstance(*pSurface);
    }
}

#endif  // VK_USE_PLATFORM_WAYLAND_KHR
#ifdef VK_USE_PLATFORM_ANDROID_KHR
void ThreadSafety::PreCallRecordCreateAndroidSurfaceKHR(VkInstance instance, const VkAndroidSurfaceCreateInfoKHR* pCreateInfo,
                                                        const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                        const RecordObject& record_obj) {
    StartReadObjectParentInstance(instance, record_obj.location);
}

void ThreadSafety::PostCallRecordCreateAndroidSurfaceKHR(VkInstance instance, const VkAndroidSurfaceCreateInfoKHR* pCreateInfo,
                                                         const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                         const RecordObject& record_obj) {
    FinishReadObjectParentInstance(instance, record_obj.location);
    if (record_obj.result == VK_SUCCESS) {
        CreateObjectParentInstance(*pSurface);
    }
}

#endif  // VK_USE_PLATFORM_ANDROID_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
void ThreadSafety::PreCallRecordCreateWin32SurfaceKHR(VkInstance instance, const VkWin32SurfaceCreateInfoKHR* pCreateInfo,
                                                      const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                      const RecordObject& record_obj) {
    StartReadObjectParentInstance(instance, record_obj.location);
}

void ThreadSafety::PostCallRecordCreateWin32SurfaceKHR(VkInstance instance, const VkWin32SurfaceCreateInfoKHR* pCreateInfo,
                                                       const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                       const RecordObject& record_obj) {
    FinishReadObjectParentInstance(instance, record_obj.location);
    if (record_obj.result == VK_SUCCESS) {
        CreateObjectParentInstance(*pSurface);
    }
}

#endif  // VK_USE_PLATFORM_WIN32_KHR
void ThreadSafety::PreCallRecordCreateVideoSessionKHR(VkDevice device, const VkVideoSessionCreateInfoKHR* pCreateInfo,
                                                      const VkAllocationCallbacks* pAllocator, VkVideoSessionKHR* pVideoSession,
                                                      const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordCreateVideoSessionKHR(VkDevice device, const VkVideoSessionCreateInfoKHR* pCreateInfo,
                                                       const VkAllocationCallbacks* pAllocator, VkVideoSessionKHR* pVideoSession,
                                                       const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pVideoSession);
    }
}

void ThreadSafety::PreCallRecordDestroyVideoSessionKHR(VkDevice device, VkVideoSessionKHR videoSession,
                                                       const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartWriteObject(videoSession, record_obj.location);
    // Host access to videoSession must be externally synchronized
}

void ThreadSafety::PostCallRecordDestroyVideoSessionKHR(VkDevice device, VkVideoSessionKHR videoSession,
                                                        const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishWriteObject(videoSession, record_obj.location);
    DestroyObject(videoSession);
    // Host access to videoSession must be externally synchronized
}

void ThreadSafety::PreCallRecordGetVideoSessionMemoryRequirementsKHR(VkDevice device, VkVideoSessionKHR videoSession,
                                                                     uint32_t* pMemoryRequirementsCount,
                                                                     VkVideoSessionMemoryRequirementsKHR* pMemoryRequirements,
                                                                     const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(videoSession, record_obj.location);
}

void ThreadSafety::PostCallRecordGetVideoSessionMemoryRequirementsKHR(VkDevice device, VkVideoSessionKHR videoSession,
                                                                      uint32_t* pMemoryRequirementsCount,
                                                                      VkVideoSessionMemoryRequirementsKHR* pMemoryRequirements,
                                                                      const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(videoSession, record_obj.location);
}

void ThreadSafety::PreCallRecordBindVideoSessionMemoryKHR(VkDevice device, VkVideoSessionKHR videoSession,
                                                          uint32_t bindSessionMemoryInfoCount,
                                                          const VkBindVideoSessionMemoryInfoKHR* pBindSessionMemoryInfos,
                                                          const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartWriteObject(videoSession, record_obj.location);
    // Host access to videoSession must be externally synchronized
}

void ThreadSafety::PostCallRecordBindVideoSessionMemoryKHR(VkDevice device, VkVideoSessionKHR videoSession,
                                                           uint32_t bindSessionMemoryInfoCount,
                                                           const VkBindVideoSessionMemoryInfoKHR* pBindSessionMemoryInfos,
                                                           const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishWriteObject(videoSession, record_obj.location);
    // Host access to videoSession must be externally synchronized
}

void ThreadSafety::PreCallRecordCreateVideoSessionParametersKHR(VkDevice device,
                                                                const VkVideoSessionParametersCreateInfoKHR* pCreateInfo,
                                                                const VkAllocationCallbacks* pAllocator,
                                                                VkVideoSessionParametersKHR* pVideoSessionParameters,
                                                                const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordCreateVideoSessionParametersKHR(VkDevice device,
                                                                 const VkVideoSessionParametersCreateInfoKHR* pCreateInfo,
                                                                 const VkAllocationCallbacks* pAllocator,
                                                                 VkVideoSessionParametersKHR* pVideoSessionParameters,
                                                                 const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pVideoSessionParameters);
    }
}

void ThreadSafety::PreCallRecordUpdateVideoSessionParametersKHR(VkDevice device, VkVideoSessionParametersKHR videoSessionParameters,
                                                                const VkVideoSessionParametersUpdateInfoKHR* pUpdateInfo,
                                                                const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(videoSessionParameters, record_obj.location);
}

void ThreadSafety::PostCallRecordUpdateVideoSessionParametersKHR(VkDevice device,
                                                                 VkVideoSessionParametersKHR videoSessionParameters,
                                                                 const VkVideoSessionParametersUpdateInfoKHR* pUpdateInfo,
                                                                 const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(videoSessionParameters, record_obj.location);
}

void ThreadSafety::PreCallRecordDestroyVideoSessionParametersKHR(VkDevice device,
                                                                 VkVideoSessionParametersKHR videoSessionParameters,
                                                                 const VkAllocationCallbacks* pAllocator,
                                                                 const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartWriteObject(videoSessionParameters, record_obj.location);
    // Host access to videoSessionParameters must be externally synchronized
}

void ThreadSafety::PostCallRecordDestroyVideoSessionParametersKHR(VkDevice device,
                                                                  VkVideoSessionParametersKHR videoSessionParameters,
                                                                  const VkAllocationCallbacks* pAllocator,
                                                                  const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishWriteObject(videoSessionParameters, record_obj.location);
    DestroyObject(videoSessionParameters);
    // Host access to videoSessionParameters must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdBeginVideoCodingKHR(VkCommandBuffer commandBuffer, const VkVideoBeginCodingInfoKHR* pBeginInfo,
                                                       const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdBeginVideoCodingKHR(VkCommandBuffer commandBuffer, const VkVideoBeginCodingInfoKHR* pBeginInfo,
                                                        const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdEndVideoCodingKHR(VkCommandBuffer commandBuffer, const VkVideoEndCodingInfoKHR* pEndCodingInfo,
                                                     const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdEndVideoCodingKHR(VkCommandBuffer commandBuffer, const VkVideoEndCodingInfoKHR* pEndCodingInfo,
                                                      const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdControlVideoCodingKHR(VkCommandBuffer commandBuffer,
                                                         const VkVideoCodingControlInfoKHR* pCodingControlInfo,
                                                         const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdControlVideoCodingKHR(VkCommandBuffer commandBuffer,
                                                          const VkVideoCodingControlInfoKHR* pCodingControlInfo,
                                                          const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdDecodeVideoKHR(VkCommandBuffer commandBuffer, const VkVideoDecodeInfoKHR* pDecodeInfo,
                                                  const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdDecodeVideoKHR(VkCommandBuffer commandBuffer, const VkVideoDecodeInfoKHR* pDecodeInfo,
                                                   const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdBeginRenderingKHR(VkCommandBuffer commandBuffer, const VkRenderingInfo* pRenderingInfo,
                                                     const RecordObject& record_obj) {
    PreCallRecordCmdBeginRendering(commandBuffer, pRenderingInfo, record_obj);
}

void ThreadSafety::PostCallRecordCmdBeginRenderingKHR(VkCommandBuffer commandBuffer, const VkRenderingInfo* pRenderingInfo,
                                                      const RecordObject& record_obj) {
    PostCallRecordCmdBeginRendering(commandBuffer, pRenderingInfo, record_obj);
}

void ThreadSafety::PreCallRecordCmdEndRenderingKHR(VkCommandBuffer commandBuffer, const RecordObject& record_obj) {
    PreCallRecordCmdEndRendering(commandBuffer, record_obj);
}

void ThreadSafety::PostCallRecordCmdEndRenderingKHR(VkCommandBuffer commandBuffer, const RecordObject& record_obj) {
    PostCallRecordCmdEndRendering(commandBuffer, record_obj);
}

void ThreadSafety::PreCallRecordGetDeviceGroupPeerMemoryFeaturesKHR(VkDevice device, uint32_t heapIndex, uint32_t localDeviceIndex,
                                                                    uint32_t remoteDeviceIndex,
                                                                    VkPeerMemoryFeatureFlags* pPeerMemoryFeatures,
                                                                    const RecordObject& record_obj) {
    PreCallRecordGetDeviceGroupPeerMemoryFeatures(device, heapIndex, localDeviceIndex, remoteDeviceIndex, pPeerMemoryFeatures,
                                                  record_obj);
}

void ThreadSafety::PostCallRecordGetDeviceGroupPeerMemoryFeaturesKHR(VkDevice device, uint32_t heapIndex, uint32_t localDeviceIndex,
                                                                     uint32_t remoteDeviceIndex,
                                                                     VkPeerMemoryFeatureFlags* pPeerMemoryFeatures,
                                                                     const RecordObject& record_obj) {
    PostCallRecordGetDeviceGroupPeerMemoryFeatures(device, heapIndex, localDeviceIndex, remoteDeviceIndex, pPeerMemoryFeatures,
                                                   record_obj);
}

void ThreadSafety::PreCallRecordCmdSetDeviceMaskKHR(VkCommandBuffer commandBuffer, uint32_t deviceMask,
                                                    const RecordObject& record_obj) {
    PreCallRecordCmdSetDeviceMask(commandBuffer, deviceMask, record_obj);
}

void ThreadSafety::PostCallRecordCmdSetDeviceMaskKHR(VkCommandBuffer commandBuffer, uint32_t deviceMask,
                                                     const RecordObject& record_obj) {
    PostCallRecordCmdSetDeviceMask(commandBuffer, deviceMask, record_obj);
}

void ThreadSafety::PreCallRecordCmdDispatchBaseKHR(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY,
                                                   uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY,
                                                   uint32_t groupCountZ, const RecordObject& record_obj) {
    PreCallRecordCmdDispatchBase(commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ,
                                 record_obj);
}

void ThreadSafety::PostCallRecordCmdDispatchBaseKHR(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY,
                                                    uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY,
                                                    uint32_t groupCountZ, const RecordObject& record_obj) {
    PostCallRecordCmdDispatchBase(commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ,
                                  record_obj);
}

void ThreadSafety::PreCallRecordTrimCommandPoolKHR(VkDevice device, VkCommandPool commandPool, VkCommandPoolTrimFlags flags,
                                                   const RecordObject& record_obj) {
    PreCallRecordTrimCommandPool(device, commandPool, flags, record_obj);
}

void ThreadSafety::PostCallRecordTrimCommandPoolKHR(VkDevice device, VkCommandPool commandPool, VkCommandPoolTrimFlags flags,
                                                    const RecordObject& record_obj) {
    PostCallRecordTrimCommandPool(device, commandPool, flags, record_obj);
}

void ThreadSafety::PreCallRecordEnumeratePhysicalDeviceGroupsKHR(VkInstance instance, uint32_t* pPhysicalDeviceGroupCount,
                                                                 VkPhysicalDeviceGroupProperties* pPhysicalDeviceGroupProperties,
                                                                 const RecordObject& record_obj) {
    PreCallRecordEnumeratePhysicalDeviceGroups(instance, pPhysicalDeviceGroupCount, pPhysicalDeviceGroupProperties, record_obj);
}

void ThreadSafety::PostCallRecordEnumeratePhysicalDeviceGroupsKHR(VkInstance instance, uint32_t* pPhysicalDeviceGroupCount,
                                                                  VkPhysicalDeviceGroupProperties* pPhysicalDeviceGroupProperties,
                                                                  const RecordObject& record_obj) {
    PostCallRecordEnumeratePhysicalDeviceGroups(instance, pPhysicalDeviceGroupCount, pPhysicalDeviceGroupProperties, record_obj);
}

#ifdef VK_USE_PLATFORM_WIN32_KHR
void ThreadSafety::PreCallRecordGetMemoryWin32HandleKHR(VkDevice device, const VkMemoryGetWin32HandleInfoKHR* pGetWin32HandleInfo,
                                                        HANDLE* pHandle, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordGetMemoryWin32HandleKHR(VkDevice device, const VkMemoryGetWin32HandleInfoKHR* pGetWin32HandleInfo,
                                                         HANDLE* pHandle, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PreCallRecordGetMemoryWin32HandlePropertiesKHR(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType,
                                                                  HANDLE handle,
                                                                  VkMemoryWin32HandlePropertiesKHR* pMemoryWin32HandleProperties,
                                                                  const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordGetMemoryWin32HandlePropertiesKHR(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType,
                                                                   HANDLE handle,
                                                                   VkMemoryWin32HandlePropertiesKHR* pMemoryWin32HandleProperties,
                                                                   const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

#endif  // VK_USE_PLATFORM_WIN32_KHR
void ThreadSafety::PreCallRecordGetMemoryFdKHR(VkDevice device, const VkMemoryGetFdInfoKHR* pGetFdInfo, int* pFd,
                                               const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordGetMemoryFdKHR(VkDevice device, const VkMemoryGetFdInfoKHR* pGetFdInfo, int* pFd,
                                                const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PreCallRecordGetMemoryFdPropertiesKHR(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType, int fd,
                                                         VkMemoryFdPropertiesKHR* pMemoryFdProperties,
                                                         const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordGetMemoryFdPropertiesKHR(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType, int fd,
                                                          VkMemoryFdPropertiesKHR* pMemoryFdProperties,
                                                          const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

#ifdef VK_USE_PLATFORM_WIN32_KHR
void ThreadSafety::PreCallRecordImportSemaphoreWin32HandleKHR(
    VkDevice device, const VkImportSemaphoreWin32HandleInfoKHR* pImportSemaphoreWin32HandleInfo, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordImportSemaphoreWin32HandleKHR(
    VkDevice device, const VkImportSemaphoreWin32HandleInfoKHR* pImportSemaphoreWin32HandleInfo, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PreCallRecordGetSemaphoreWin32HandleKHR(VkDevice device,
                                                           const VkSemaphoreGetWin32HandleInfoKHR* pGetWin32HandleInfo,
                                                           HANDLE* pHandle, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordGetSemaphoreWin32HandleKHR(VkDevice device,
                                                            const VkSemaphoreGetWin32HandleInfoKHR* pGetWin32HandleInfo,
                                                            HANDLE* pHandle, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

#endif  // VK_USE_PLATFORM_WIN32_KHR
void ThreadSafety::PreCallRecordImportSemaphoreFdKHR(VkDevice device, const VkImportSemaphoreFdInfoKHR* pImportSemaphoreFdInfo,
                                                     const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordImportSemaphoreFdKHR(VkDevice device, const VkImportSemaphoreFdInfoKHR* pImportSemaphoreFdInfo,
                                                      const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PreCallRecordGetSemaphoreFdKHR(VkDevice device, const VkSemaphoreGetFdInfoKHR* pGetFdInfo, int* pFd,
                                                  const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordGetSemaphoreFdKHR(VkDevice device, const VkSemaphoreGetFdInfoKHR* pGetFdInfo, int* pFd,
                                                   const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PreCallRecordCmdPushDescriptorSetKHR(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                        VkPipelineLayout layout, uint32_t set, uint32_t descriptorWriteCount,
                                                        const VkWriteDescriptorSet* pDescriptorWrites,
                                                        const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    StartReadObject(layout, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdPushDescriptorSetKHR(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                         VkPipelineLayout layout, uint32_t set, uint32_t descriptorWriteCount,
                                                         const VkWriteDescriptorSet* pDescriptorWrites,
                                                         const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    FinishReadObject(layout, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdPushDescriptorSetWithTemplateKHR(VkCommandBuffer commandBuffer,
                                                                    VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                                    VkPipelineLayout layout, uint32_t set, const void* pData,
                                                                    const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    StartReadObject(descriptorUpdateTemplate, record_obj.location);
    StartReadObject(layout, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdPushDescriptorSetWithTemplateKHR(VkCommandBuffer commandBuffer,
                                                                     VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                                     VkPipelineLayout layout, uint32_t set, const void* pData,
                                                                     const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    FinishReadObject(descriptorUpdateTemplate, record_obj.location);
    FinishReadObject(layout, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCreateDescriptorUpdateTemplateKHR(VkDevice device,
                                                                  const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo,
                                                                  const VkAllocationCallbacks* pAllocator,
                                                                  VkDescriptorUpdateTemplate* pDescriptorUpdateTemplate,
                                                                  const RecordObject& record_obj) {
    PreCallRecordCreateDescriptorUpdateTemplate(device, pCreateInfo, pAllocator, pDescriptorUpdateTemplate, record_obj);
}

void ThreadSafety::PostCallRecordCreateDescriptorUpdateTemplateKHR(VkDevice device,
                                                                   const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo,
                                                                   const VkAllocationCallbacks* pAllocator,
                                                                   VkDescriptorUpdateTemplate* pDescriptorUpdateTemplate,
                                                                   const RecordObject& record_obj) {
    PostCallRecordCreateDescriptorUpdateTemplate(device, pCreateInfo, pAllocator, pDescriptorUpdateTemplate, record_obj);
}

void ThreadSafety::PreCallRecordDestroyDescriptorUpdateTemplateKHR(VkDevice device,
                                                                   VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                                   const VkAllocationCallbacks* pAllocator,
                                                                   const RecordObject& record_obj) {
    PreCallRecordDestroyDescriptorUpdateTemplate(device, descriptorUpdateTemplate, pAllocator, record_obj);
}

void ThreadSafety::PostCallRecordDestroyDescriptorUpdateTemplateKHR(VkDevice device,
                                                                    VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                                    const VkAllocationCallbacks* pAllocator,
                                                                    const RecordObject& record_obj) {
    PostCallRecordDestroyDescriptorUpdateTemplate(device, descriptorUpdateTemplate, pAllocator, record_obj);
}

void ThreadSafety::PreCallRecordCreateRenderPass2KHR(VkDevice device, const VkRenderPassCreateInfo2* pCreateInfo,
                                                     const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass,
                                                     const RecordObject& record_obj) {
    PreCallRecordCreateRenderPass2(device, pCreateInfo, pAllocator, pRenderPass, record_obj);
}

void ThreadSafety::PostCallRecordCreateRenderPass2KHR(VkDevice device, const VkRenderPassCreateInfo2* pCreateInfo,
                                                      const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass,
                                                      const RecordObject& record_obj) {
    PostCallRecordCreateRenderPass2(device, pCreateInfo, pAllocator, pRenderPass, record_obj);
}

void ThreadSafety::PreCallRecordCmdBeginRenderPass2KHR(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                                       const VkSubpassBeginInfo* pSubpassBeginInfo,
                                                       const RecordObject& record_obj) {
    PreCallRecordCmdBeginRenderPass2(commandBuffer, pRenderPassBegin, pSubpassBeginInfo, record_obj);
}

void ThreadSafety::PostCallRecordCmdBeginRenderPass2KHR(VkCommandBuffer commandBuffer,
                                                        const VkRenderPassBeginInfo* pRenderPassBegin,
                                                        const VkSubpassBeginInfo* pSubpassBeginInfo,
                                                        const RecordObject& record_obj) {
    PostCallRecordCmdBeginRenderPass2(commandBuffer, pRenderPassBegin, pSubpassBeginInfo, record_obj);
}

void ThreadSafety::PreCallRecordCmdNextSubpass2KHR(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo* pSubpassBeginInfo,
                                                   const VkSubpassEndInfo* pSubpassEndInfo, const RecordObject& record_obj) {
    PreCallRecordCmdNextSubpass2(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo, record_obj);
}

void ThreadSafety::PostCallRecordCmdNextSubpass2KHR(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo* pSubpassBeginInfo,
                                                    const VkSubpassEndInfo* pSubpassEndInfo, const RecordObject& record_obj) {
    PostCallRecordCmdNextSubpass2(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo, record_obj);
}

void ThreadSafety::PreCallRecordCmdEndRenderPass2KHR(VkCommandBuffer commandBuffer, const VkSubpassEndInfo* pSubpassEndInfo,
                                                     const RecordObject& record_obj) {
    PreCallRecordCmdEndRenderPass2(commandBuffer, pSubpassEndInfo, record_obj);
}

void ThreadSafety::PostCallRecordCmdEndRenderPass2KHR(VkCommandBuffer commandBuffer, const VkSubpassEndInfo* pSubpassEndInfo,
                                                      const RecordObject& record_obj) {
    PostCallRecordCmdEndRenderPass2(commandBuffer, pSubpassEndInfo, record_obj);
}

void ThreadSafety::PreCallRecordGetSwapchainStatusKHR(VkDevice device, VkSwapchainKHR swapchain, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartWriteObject(swapchain, record_obj.location);
    // Host access to swapchain must be externally synchronized
}

void ThreadSafety::PostCallRecordGetSwapchainStatusKHR(VkDevice device, VkSwapchainKHR swapchain, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishWriteObject(swapchain, record_obj.location);
    // Host access to swapchain must be externally synchronized
}

#ifdef VK_USE_PLATFORM_WIN32_KHR
void ThreadSafety::PreCallRecordImportFenceWin32HandleKHR(VkDevice device,
                                                          const VkImportFenceWin32HandleInfoKHR* pImportFenceWin32HandleInfo,
                                                          const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordImportFenceWin32HandleKHR(VkDevice device,
                                                           const VkImportFenceWin32HandleInfoKHR* pImportFenceWin32HandleInfo,
                                                           const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PreCallRecordGetFenceWin32HandleKHR(VkDevice device, const VkFenceGetWin32HandleInfoKHR* pGetWin32HandleInfo,
                                                       HANDLE* pHandle, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordGetFenceWin32HandleKHR(VkDevice device, const VkFenceGetWin32HandleInfoKHR* pGetWin32HandleInfo,
                                                        HANDLE* pHandle, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

#endif  // VK_USE_PLATFORM_WIN32_KHR
void ThreadSafety::PreCallRecordImportFenceFdKHR(VkDevice device, const VkImportFenceFdInfoKHR* pImportFenceFdInfo,
                                                 const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordImportFenceFdKHR(VkDevice device, const VkImportFenceFdInfoKHR* pImportFenceFdInfo,
                                                  const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PreCallRecordGetFenceFdKHR(VkDevice device, const VkFenceGetFdInfoKHR* pGetFdInfo, int* pFd,
                                              const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordGetFenceFdKHR(VkDevice device, const VkFenceGetFdInfoKHR* pGetFdInfo, int* pFd,
                                               const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PreCallRecordAcquireProfilingLockKHR(VkDevice device, const VkAcquireProfilingLockInfoKHR* pInfo,
                                                        const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordAcquireProfilingLockKHR(VkDevice device, const VkAcquireProfilingLockInfoKHR* pInfo,
                                                         const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PreCallRecordReleaseProfilingLockKHR(VkDevice device, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordReleaseProfilingLockKHR(VkDevice device, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PreCallRecordGetImageMemoryRequirements2KHR(VkDevice device, const VkImageMemoryRequirementsInfo2* pInfo,
                                                               VkMemoryRequirements2* pMemoryRequirements,
                                                               const RecordObject& record_obj) {
    PreCallRecordGetImageMemoryRequirements2(device, pInfo, pMemoryRequirements, record_obj);
}

void ThreadSafety::PostCallRecordGetImageMemoryRequirements2KHR(VkDevice device, const VkImageMemoryRequirementsInfo2* pInfo,
                                                                VkMemoryRequirements2* pMemoryRequirements,
                                                                const RecordObject& record_obj) {
    PostCallRecordGetImageMemoryRequirements2(device, pInfo, pMemoryRequirements, record_obj);
}

void ThreadSafety::PreCallRecordGetBufferMemoryRequirements2KHR(VkDevice device, const VkBufferMemoryRequirementsInfo2* pInfo,
                                                                VkMemoryRequirements2* pMemoryRequirements,
                                                                const RecordObject& record_obj) {
    PreCallRecordGetBufferMemoryRequirements2(device, pInfo, pMemoryRequirements, record_obj);
}

void ThreadSafety::PostCallRecordGetBufferMemoryRequirements2KHR(VkDevice device, const VkBufferMemoryRequirementsInfo2* pInfo,
                                                                 VkMemoryRequirements2* pMemoryRequirements,
                                                                 const RecordObject& record_obj) {
    PostCallRecordGetBufferMemoryRequirements2(device, pInfo, pMemoryRequirements, record_obj);
}

void ThreadSafety::PreCallRecordGetImageSparseMemoryRequirements2KHR(VkDevice device,
                                                                     const VkImageSparseMemoryRequirementsInfo2* pInfo,
                                                                     uint32_t* pSparseMemoryRequirementCount,
                                                                     VkSparseImageMemoryRequirements2* pSparseMemoryRequirements,
                                                                     const RecordObject& record_obj) {
    PreCallRecordGetImageSparseMemoryRequirements2(device, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements,
                                                   record_obj);
}

void ThreadSafety::PostCallRecordGetImageSparseMemoryRequirements2KHR(VkDevice device,
                                                                      const VkImageSparseMemoryRequirementsInfo2* pInfo,
                                                                      uint32_t* pSparseMemoryRequirementCount,
                                                                      VkSparseImageMemoryRequirements2* pSparseMemoryRequirements,
                                                                      const RecordObject& record_obj) {
    PostCallRecordGetImageSparseMemoryRequirements2(device, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements,
                                                    record_obj);
}

void ThreadSafety::PreCallRecordCreateSamplerYcbcrConversionKHR(VkDevice device,
                                                                const VkSamplerYcbcrConversionCreateInfo* pCreateInfo,
                                                                const VkAllocationCallbacks* pAllocator,
                                                                VkSamplerYcbcrConversion* pYcbcrConversion,
                                                                const RecordObject& record_obj) {
    PreCallRecordCreateSamplerYcbcrConversion(device, pCreateInfo, pAllocator, pYcbcrConversion, record_obj);
}

void ThreadSafety::PostCallRecordCreateSamplerYcbcrConversionKHR(VkDevice device,
                                                                 const VkSamplerYcbcrConversionCreateInfo* pCreateInfo,
                                                                 const VkAllocationCallbacks* pAllocator,
                                                                 VkSamplerYcbcrConversion* pYcbcrConversion,
                                                                 const RecordObject& record_obj) {
    PostCallRecordCreateSamplerYcbcrConversion(device, pCreateInfo, pAllocator, pYcbcrConversion, record_obj);
}

void ThreadSafety::PreCallRecordDestroySamplerYcbcrConversionKHR(VkDevice device, VkSamplerYcbcrConversion ycbcrConversion,
                                                                 const VkAllocationCallbacks* pAllocator,
                                                                 const RecordObject& record_obj) {
    PreCallRecordDestroySamplerYcbcrConversion(device, ycbcrConversion, pAllocator, record_obj);
}

void ThreadSafety::PostCallRecordDestroySamplerYcbcrConversionKHR(VkDevice device, VkSamplerYcbcrConversion ycbcrConversion,
                                                                  const VkAllocationCallbacks* pAllocator,
                                                                  const RecordObject& record_obj) {
    PostCallRecordDestroySamplerYcbcrConversion(device, ycbcrConversion, pAllocator, record_obj);
}

void ThreadSafety::PreCallRecordBindBufferMemory2KHR(VkDevice device, uint32_t bindInfoCount,
                                                     const VkBindBufferMemoryInfo* pBindInfos, const RecordObject& record_obj) {
    PreCallRecordBindBufferMemory2(device, bindInfoCount, pBindInfos, record_obj);
}

void ThreadSafety::PostCallRecordBindBufferMemory2KHR(VkDevice device, uint32_t bindInfoCount,
                                                      const VkBindBufferMemoryInfo* pBindInfos, const RecordObject& record_obj) {
    PostCallRecordBindBufferMemory2(device, bindInfoCount, pBindInfos, record_obj);
}

void ThreadSafety::PreCallRecordBindImageMemory2KHR(VkDevice device, uint32_t bindInfoCount,
                                                    const VkBindImageMemoryInfo* pBindInfos, const RecordObject& record_obj) {
    PreCallRecordBindImageMemory2(device, bindInfoCount, pBindInfos, record_obj);
}

void ThreadSafety::PostCallRecordBindImageMemory2KHR(VkDevice device, uint32_t bindInfoCount,
                                                     const VkBindImageMemoryInfo* pBindInfos, const RecordObject& record_obj) {
    PostCallRecordBindImageMemory2(device, bindInfoCount, pBindInfos, record_obj);
}

void ThreadSafety::PreCallRecordGetDescriptorSetLayoutSupportKHR(VkDevice device,
                                                                 const VkDescriptorSetLayoutCreateInfo* pCreateInfo,
                                                                 VkDescriptorSetLayoutSupport* pSupport,
                                                                 const RecordObject& record_obj) {
    PreCallRecordGetDescriptorSetLayoutSupport(device, pCreateInfo, pSupport, record_obj);
}

void ThreadSafety::PostCallRecordGetDescriptorSetLayoutSupportKHR(VkDevice device,
                                                                  const VkDescriptorSetLayoutCreateInfo* pCreateInfo,
                                                                  VkDescriptorSetLayoutSupport* pSupport,
                                                                  const RecordObject& record_obj) {
    PostCallRecordGetDescriptorSetLayoutSupport(device, pCreateInfo, pSupport, record_obj);
}

void ThreadSafety::PreCallRecordCmdDrawIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                        VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                        uint32_t stride, const RecordObject& record_obj) {
    PreCallRecordCmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride,
                                      record_obj);
}

void ThreadSafety::PostCallRecordCmdDrawIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                         VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                         uint32_t maxDrawCount, uint32_t stride, const RecordObject& record_obj) {
    PostCallRecordCmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride,
                                       record_obj);
}

void ThreadSafety::PreCallRecordCmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                               VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                               uint32_t maxDrawCount, uint32_t stride,
                                                               const RecordObject& record_obj) {
    PreCallRecordCmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride,
                                             record_obj);
}

void ThreadSafety::PostCallRecordCmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                                VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                                uint32_t maxDrawCount, uint32_t stride,
                                                                const RecordObject& record_obj) {
    PostCallRecordCmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride,
                                              record_obj);
}

void ThreadSafety::PreCallRecordGetSemaphoreCounterValueKHR(VkDevice device, VkSemaphore semaphore, uint64_t* pValue,
                                                            const RecordObject& record_obj) {
    PreCallRecordGetSemaphoreCounterValue(device, semaphore, pValue, record_obj);
}

void ThreadSafety::PostCallRecordGetSemaphoreCounterValueKHR(VkDevice device, VkSemaphore semaphore, uint64_t* pValue,
                                                             const RecordObject& record_obj) {
    PostCallRecordGetSemaphoreCounterValue(device, semaphore, pValue, record_obj);
}

void ThreadSafety::PreCallRecordWaitSemaphoresKHR(VkDevice device, const VkSemaphoreWaitInfo* pWaitInfo, uint64_t timeout,
                                                  const RecordObject& record_obj) {
    PreCallRecordWaitSemaphores(device, pWaitInfo, timeout, record_obj);
}

void ThreadSafety::PostCallRecordWaitSemaphoresKHR(VkDevice device, const VkSemaphoreWaitInfo* pWaitInfo, uint64_t timeout,
                                                   const RecordObject& record_obj) {
    PostCallRecordWaitSemaphores(device, pWaitInfo, timeout, record_obj);
}

void ThreadSafety::PreCallRecordSignalSemaphoreKHR(VkDevice device, const VkSemaphoreSignalInfo* pSignalInfo,
                                                   const RecordObject& record_obj) {
    PreCallRecordSignalSemaphore(device, pSignalInfo, record_obj);
}

void ThreadSafety::PostCallRecordSignalSemaphoreKHR(VkDevice device, const VkSemaphoreSignalInfo* pSignalInfo,
                                                    const RecordObject& record_obj) {
    PostCallRecordSignalSemaphore(device, pSignalInfo, record_obj);
}

void ThreadSafety::PreCallRecordCmdSetFragmentShadingRateKHR(VkCommandBuffer commandBuffer, const VkExtent2D* pFragmentSize,
                                                             const VkFragmentShadingRateCombinerOpKHR combinerOps[2],
                                                             const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetFragmentShadingRateKHR(VkCommandBuffer commandBuffer, const VkExtent2D* pFragmentSize,
                                                              const VkFragmentShadingRateCombinerOpKHR combinerOps[2],
                                                              const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordGetBufferDeviceAddressKHR(VkDevice device, const VkBufferDeviceAddressInfo* pInfo,
                                                          const RecordObject& record_obj) {
    PreCallRecordGetBufferDeviceAddress(device, pInfo, record_obj);
}

void ThreadSafety::PostCallRecordGetBufferDeviceAddressKHR(VkDevice device, const VkBufferDeviceAddressInfo* pInfo,
                                                           const RecordObject& record_obj) {
    PostCallRecordGetBufferDeviceAddress(device, pInfo, record_obj);
}

void ThreadSafety::PreCallRecordGetBufferOpaqueCaptureAddressKHR(VkDevice device, const VkBufferDeviceAddressInfo* pInfo,
                                                                 const RecordObject& record_obj) {
    PreCallRecordGetBufferOpaqueCaptureAddress(device, pInfo, record_obj);
}

void ThreadSafety::PostCallRecordGetBufferOpaqueCaptureAddressKHR(VkDevice device, const VkBufferDeviceAddressInfo* pInfo,
                                                                  const RecordObject& record_obj) {
    PostCallRecordGetBufferOpaqueCaptureAddress(device, pInfo, record_obj);
}

void ThreadSafety::PreCallRecordGetDeviceMemoryOpaqueCaptureAddressKHR(VkDevice device,
                                                                       const VkDeviceMemoryOpaqueCaptureAddressInfo* pInfo,
                                                                       const RecordObject& record_obj) {
    PreCallRecordGetDeviceMemoryOpaqueCaptureAddress(device, pInfo, record_obj);
}

void ThreadSafety::PostCallRecordGetDeviceMemoryOpaqueCaptureAddressKHR(VkDevice device,
                                                                        const VkDeviceMemoryOpaqueCaptureAddressInfo* pInfo,
                                                                        const RecordObject& record_obj) {
    PostCallRecordGetDeviceMemoryOpaqueCaptureAddress(device, pInfo, record_obj);
}

void ThreadSafety::PreCallRecordCreateDeferredOperationKHR(VkDevice device, const VkAllocationCallbacks* pAllocator,
                                                           VkDeferredOperationKHR* pDeferredOperation,
                                                           const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordCreateDeferredOperationKHR(VkDevice device, const VkAllocationCallbacks* pAllocator,
                                                            VkDeferredOperationKHR* pDeferredOperation,
                                                            const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pDeferredOperation);
    }
}

void ThreadSafety::PreCallRecordDestroyDeferredOperationKHR(VkDevice device, VkDeferredOperationKHR operation,
                                                            const VkAllocationCallbacks* pAllocator,
                                                            const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartWriteObject(operation, record_obj.location);
    // Host access to operation must be externally synchronized
}

void ThreadSafety::PostCallRecordDestroyDeferredOperationKHR(VkDevice device, VkDeferredOperationKHR operation,
                                                             const VkAllocationCallbacks* pAllocator,
                                                             const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishWriteObject(operation, record_obj.location);
    DestroyObject(operation);
    // Host access to operation must be externally synchronized
}

void ThreadSafety::PreCallRecordGetDeferredOperationMaxConcurrencyKHR(VkDevice device, VkDeferredOperationKHR operation,
                                                                      const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(operation, record_obj.location);
}

void ThreadSafety::PostCallRecordGetDeferredOperationMaxConcurrencyKHR(VkDevice device, VkDeferredOperationKHR operation,
                                                                       const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(operation, record_obj.location);
}

void ThreadSafety::PreCallRecordGetDeferredOperationResultKHR(VkDevice device, VkDeferredOperationKHR operation,
                                                              const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(operation, record_obj.location);
}

void ThreadSafety::PostCallRecordGetDeferredOperationResultKHR(VkDevice device, VkDeferredOperationKHR operation,
                                                               const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(operation, record_obj.location);
}

void ThreadSafety::PreCallRecordDeferredOperationJoinKHR(VkDevice device, VkDeferredOperationKHR operation,
                                                         const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(operation, record_obj.location);
}

void ThreadSafety::PostCallRecordDeferredOperationJoinKHR(VkDevice device, VkDeferredOperationKHR operation,
                                                          const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(operation, record_obj.location);
}

void ThreadSafety::PreCallRecordGetPipelineExecutablePropertiesKHR(VkDevice device, const VkPipelineInfoKHR* pPipelineInfo,
                                                                   uint32_t* pExecutableCount,
                                                                   VkPipelineExecutablePropertiesKHR* pProperties,
                                                                   const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordGetPipelineExecutablePropertiesKHR(VkDevice device, const VkPipelineInfoKHR* pPipelineInfo,
                                                                    uint32_t* pExecutableCount,
                                                                    VkPipelineExecutablePropertiesKHR* pProperties,
                                                                    const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PreCallRecordGetPipelineExecutableStatisticsKHR(VkDevice device,
                                                                   const VkPipelineExecutableInfoKHR* pExecutableInfo,
                                                                   uint32_t* pStatisticCount,
                                                                   VkPipelineExecutableStatisticKHR* pStatistics,
                                                                   const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordGetPipelineExecutableStatisticsKHR(VkDevice device,
                                                                    const VkPipelineExecutableInfoKHR* pExecutableInfo,
                                                                    uint32_t* pStatisticCount,
                                                                    VkPipelineExecutableStatisticKHR* pStatistics,
                                                                    const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PreCallRecordGetPipelineExecutableInternalRepresentationsKHR(
    VkDevice device, const VkPipelineExecutableInfoKHR* pExecutableInfo, uint32_t* pInternalRepresentationCount,
    VkPipelineExecutableInternalRepresentationKHR* pInternalRepresentations, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordGetPipelineExecutableInternalRepresentationsKHR(
    VkDevice device, const VkPipelineExecutableInfoKHR* pExecutableInfo, uint32_t* pInternalRepresentationCount,
    VkPipelineExecutableInternalRepresentationKHR* pInternalRepresentations, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PreCallRecordMapMemory2KHR(VkDevice device, const VkMemoryMapInfoKHR* pMemoryMapInfo, void** ppData,
                                              const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordMapMemory2KHR(VkDevice device, const VkMemoryMapInfoKHR* pMemoryMapInfo, void** ppData,
                                               const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PreCallRecordUnmapMemory2KHR(VkDevice device, const VkMemoryUnmapInfoKHR* pMemoryUnmapInfo,
                                                const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordUnmapMemory2KHR(VkDevice device, const VkMemoryUnmapInfoKHR* pMemoryUnmapInfo,
                                                 const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PreCallRecordGetEncodedVideoSessionParametersKHR(
    VkDevice device, const VkVideoEncodeSessionParametersGetInfoKHR* pVideoSessionParametersInfo,
    VkVideoEncodeSessionParametersFeedbackInfoKHR* pFeedbackInfo, size_t* pDataSize, void* pData, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordGetEncodedVideoSessionParametersKHR(
    VkDevice device, const VkVideoEncodeSessionParametersGetInfoKHR* pVideoSessionParametersInfo,
    VkVideoEncodeSessionParametersFeedbackInfoKHR* pFeedbackInfo, size_t* pDataSize, void* pData, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PreCallRecordCmdEncodeVideoKHR(VkCommandBuffer commandBuffer, const VkVideoEncodeInfoKHR* pEncodeInfo,
                                                  const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdEncodeVideoKHR(VkCommandBuffer commandBuffer, const VkVideoEncodeInfoKHR* pEncodeInfo,
                                                   const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event,
                                                const VkDependencyInfo* pDependencyInfo, const RecordObject& record_obj) {
    PreCallRecordCmdSetEvent2(commandBuffer, event, pDependencyInfo, record_obj);
}

void ThreadSafety::PostCallRecordCmdSetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event,
                                                 const VkDependencyInfo* pDependencyInfo, const RecordObject& record_obj) {
    PostCallRecordCmdSetEvent2(commandBuffer, event, pDependencyInfo, record_obj);
}

void ThreadSafety::PreCallRecordCmdResetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags2 stageMask,
                                                  const RecordObject& record_obj) {
    PreCallRecordCmdResetEvent2(commandBuffer, event, stageMask, record_obj);
}

void ThreadSafety::PostCallRecordCmdResetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags2 stageMask,
                                                   const RecordObject& record_obj) {
    PostCallRecordCmdResetEvent2(commandBuffer, event, stageMask, record_obj);
}

void ThreadSafety::PreCallRecordCmdWaitEvents2KHR(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                                  const VkDependencyInfo* pDependencyInfos, const RecordObject& record_obj) {
    PreCallRecordCmdWaitEvents2(commandBuffer, eventCount, pEvents, pDependencyInfos, record_obj);
}

void ThreadSafety::PostCallRecordCmdWaitEvents2KHR(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                                   const VkDependencyInfo* pDependencyInfos, const RecordObject& record_obj) {
    PostCallRecordCmdWaitEvents2(commandBuffer, eventCount, pEvents, pDependencyInfos, record_obj);
}

void ThreadSafety::PreCallRecordCmdPipelineBarrier2KHR(VkCommandBuffer commandBuffer, const VkDependencyInfo* pDependencyInfo,
                                                       const RecordObject& record_obj) {
    PreCallRecordCmdPipelineBarrier2(commandBuffer, pDependencyInfo, record_obj);
}

void ThreadSafety::PostCallRecordCmdPipelineBarrier2KHR(VkCommandBuffer commandBuffer, const VkDependencyInfo* pDependencyInfo,
                                                        const RecordObject& record_obj) {
    PostCallRecordCmdPipelineBarrier2(commandBuffer, pDependencyInfo, record_obj);
}

void ThreadSafety::PreCallRecordCmdWriteTimestamp2KHR(VkCommandBuffer commandBuffer, VkPipelineStageFlags2 stage,
                                                      VkQueryPool queryPool, uint32_t query, const RecordObject& record_obj) {
    PreCallRecordCmdWriteTimestamp2(commandBuffer, stage, queryPool, query, record_obj);
}

void ThreadSafety::PostCallRecordCmdWriteTimestamp2KHR(VkCommandBuffer commandBuffer, VkPipelineStageFlags2 stage,
                                                       VkQueryPool queryPool, uint32_t query, const RecordObject& record_obj) {
    PostCallRecordCmdWriteTimestamp2(commandBuffer, stage, queryPool, query, record_obj);
}

void ThreadSafety::PreCallRecordQueueSubmit2KHR(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2* pSubmits, VkFence fence,
                                                const RecordObject& record_obj) {
    PreCallRecordQueueSubmit2(queue, submitCount, pSubmits, fence, record_obj);
}

void ThreadSafety::PostCallRecordQueueSubmit2KHR(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2* pSubmits, VkFence fence,
                                                 const RecordObject& record_obj) {
    PostCallRecordQueueSubmit2(queue, submitCount, pSubmits, fence, record_obj);
}

void ThreadSafety::PreCallRecordCmdWriteBufferMarker2AMD(VkCommandBuffer commandBuffer, VkPipelineStageFlags2 stage,
                                                         VkBuffer dstBuffer, VkDeviceSize dstOffset, uint32_t marker,
                                                         const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    StartReadObject(dstBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdWriteBufferMarker2AMD(VkCommandBuffer commandBuffer, VkPipelineStageFlags2 stage,
                                                          VkBuffer dstBuffer, VkDeviceSize dstOffset, uint32_t marker,
                                                          const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    FinishReadObject(dstBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordGetQueueCheckpointData2NV(VkQueue queue, uint32_t* pCheckpointDataCount,
                                                          VkCheckpointData2NV* pCheckpointData, const RecordObject& record_obj) {
    StartReadObject(queue, record_obj.location);
}

void ThreadSafety::PostCallRecordGetQueueCheckpointData2NV(VkQueue queue, uint32_t* pCheckpointDataCount,
                                                           VkCheckpointData2NV* pCheckpointData, const RecordObject& record_obj) {
    FinishReadObject(queue, record_obj.location);
}

void ThreadSafety::PreCallRecordCmdCopyBuffer2KHR(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2* pCopyBufferInfo,
                                                  const RecordObject& record_obj) {
    PreCallRecordCmdCopyBuffer2(commandBuffer, pCopyBufferInfo, record_obj);
}

void ThreadSafety::PostCallRecordCmdCopyBuffer2KHR(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2* pCopyBufferInfo,
                                                   const RecordObject& record_obj) {
    PostCallRecordCmdCopyBuffer2(commandBuffer, pCopyBufferInfo, record_obj);
}

void ThreadSafety::PreCallRecordCmdCopyImage2KHR(VkCommandBuffer commandBuffer, const VkCopyImageInfo2* pCopyImageInfo,
                                                 const RecordObject& record_obj) {
    PreCallRecordCmdCopyImage2(commandBuffer, pCopyImageInfo, record_obj);
}

void ThreadSafety::PostCallRecordCmdCopyImage2KHR(VkCommandBuffer commandBuffer, const VkCopyImageInfo2* pCopyImageInfo,
                                                  const RecordObject& record_obj) {
    PostCallRecordCmdCopyImage2(commandBuffer, pCopyImageInfo, record_obj);
}

void ThreadSafety::PreCallRecordCmdCopyBufferToImage2KHR(VkCommandBuffer commandBuffer,
                                                         const VkCopyBufferToImageInfo2* pCopyBufferToImageInfo,
                                                         const RecordObject& record_obj) {
    PreCallRecordCmdCopyBufferToImage2(commandBuffer, pCopyBufferToImageInfo, record_obj);
}

void ThreadSafety::PostCallRecordCmdCopyBufferToImage2KHR(VkCommandBuffer commandBuffer,
                                                          const VkCopyBufferToImageInfo2* pCopyBufferToImageInfo,
                                                          const RecordObject& record_obj) {
    PostCallRecordCmdCopyBufferToImage2(commandBuffer, pCopyBufferToImageInfo, record_obj);
}

void ThreadSafety::PreCallRecordCmdCopyImageToBuffer2KHR(VkCommandBuffer commandBuffer,
                                                         const VkCopyImageToBufferInfo2* pCopyImageToBufferInfo,
                                                         const RecordObject& record_obj) {
    PreCallRecordCmdCopyImageToBuffer2(commandBuffer, pCopyImageToBufferInfo, record_obj);
}

void ThreadSafety::PostCallRecordCmdCopyImageToBuffer2KHR(VkCommandBuffer commandBuffer,
                                                          const VkCopyImageToBufferInfo2* pCopyImageToBufferInfo,
                                                          const RecordObject& record_obj) {
    PostCallRecordCmdCopyImageToBuffer2(commandBuffer, pCopyImageToBufferInfo, record_obj);
}

void ThreadSafety::PreCallRecordCmdBlitImage2KHR(VkCommandBuffer commandBuffer, const VkBlitImageInfo2* pBlitImageInfo,
                                                 const RecordObject& record_obj) {
    PreCallRecordCmdBlitImage2(commandBuffer, pBlitImageInfo, record_obj);
}

void ThreadSafety::PostCallRecordCmdBlitImage2KHR(VkCommandBuffer commandBuffer, const VkBlitImageInfo2* pBlitImageInfo,
                                                  const RecordObject& record_obj) {
    PostCallRecordCmdBlitImage2(commandBuffer, pBlitImageInfo, record_obj);
}

void ThreadSafety::PreCallRecordCmdResolveImage2KHR(VkCommandBuffer commandBuffer, const VkResolveImageInfo2* pResolveImageInfo,
                                                    const RecordObject& record_obj) {
    PreCallRecordCmdResolveImage2(commandBuffer, pResolveImageInfo, record_obj);
}

void ThreadSafety::PostCallRecordCmdResolveImage2KHR(VkCommandBuffer commandBuffer, const VkResolveImageInfo2* pResolveImageInfo,
                                                     const RecordObject& record_obj) {
    PostCallRecordCmdResolveImage2(commandBuffer, pResolveImageInfo, record_obj);
}

void ThreadSafety::PreCallRecordCmdTraceRaysIndirect2KHR(VkCommandBuffer commandBuffer, VkDeviceAddress indirectDeviceAddress,
                                                         const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdTraceRaysIndirect2KHR(VkCommandBuffer commandBuffer, VkDeviceAddress indirectDeviceAddress,
                                                          const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordGetDeviceBufferMemoryRequirementsKHR(VkDevice device, const VkDeviceBufferMemoryRequirements* pInfo,
                                                                     VkMemoryRequirements2* pMemoryRequirements,
                                                                     const RecordObject& record_obj) {
    PreCallRecordGetDeviceBufferMemoryRequirements(device, pInfo, pMemoryRequirements, record_obj);
}

void ThreadSafety::PostCallRecordGetDeviceBufferMemoryRequirementsKHR(VkDevice device,
                                                                      const VkDeviceBufferMemoryRequirements* pInfo,
                                                                      VkMemoryRequirements2* pMemoryRequirements,
                                                                      const RecordObject& record_obj) {
    PostCallRecordGetDeviceBufferMemoryRequirements(device, pInfo, pMemoryRequirements, record_obj);
}

void ThreadSafety::PreCallRecordGetDeviceImageMemoryRequirementsKHR(VkDevice device, const VkDeviceImageMemoryRequirements* pInfo,
                                                                    VkMemoryRequirements2* pMemoryRequirements,
                                                                    const RecordObject& record_obj) {
    PreCallRecordGetDeviceImageMemoryRequirements(device, pInfo, pMemoryRequirements, record_obj);
}

void ThreadSafety::PostCallRecordGetDeviceImageMemoryRequirementsKHR(VkDevice device, const VkDeviceImageMemoryRequirements* pInfo,
                                                                     VkMemoryRequirements2* pMemoryRequirements,
                                                                     const RecordObject& record_obj) {
    PostCallRecordGetDeviceImageMemoryRequirements(device, pInfo, pMemoryRequirements, record_obj);
}

void ThreadSafety::PreCallRecordGetDeviceImageSparseMemoryRequirementsKHR(
    VkDevice device, const VkDeviceImageMemoryRequirements* pInfo, uint32_t* pSparseMemoryRequirementCount,
    VkSparseImageMemoryRequirements2* pSparseMemoryRequirements, const RecordObject& record_obj) {
    PreCallRecordGetDeviceImageSparseMemoryRequirements(device, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements,
                                                        record_obj);
}

void ThreadSafety::PostCallRecordGetDeviceImageSparseMemoryRequirementsKHR(
    VkDevice device, const VkDeviceImageMemoryRequirements* pInfo, uint32_t* pSparseMemoryRequirementCount,
    VkSparseImageMemoryRequirements2* pSparseMemoryRequirements, const RecordObject& record_obj) {
    PostCallRecordGetDeviceImageSparseMemoryRequirements(device, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements,
                                                         record_obj);
}

void ThreadSafety::PreCallRecordCmdBindIndexBuffer2KHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                       VkDeviceSize size, VkIndexType indexType, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    StartReadObject(buffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdBindIndexBuffer2KHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                        VkDeviceSize size, VkIndexType indexType, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    FinishReadObject(buffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordGetRenderingAreaGranularityKHR(VkDevice device, const VkRenderingAreaInfoKHR* pRenderingAreaInfo,
                                                               VkExtent2D* pGranularity, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordGetRenderingAreaGranularityKHR(VkDevice device, const VkRenderingAreaInfoKHR* pRenderingAreaInfo,
                                                                VkExtent2D* pGranularity, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PreCallRecordGetDeviceImageSubresourceLayoutKHR(VkDevice device, const VkDeviceImageSubresourceInfoKHR* pInfo,
                                                                   VkSubresourceLayout2KHR* pLayout,
                                                                   const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordGetDeviceImageSubresourceLayoutKHR(VkDevice device, const VkDeviceImageSubresourceInfoKHR* pInfo,
                                                                    VkSubresourceLayout2KHR* pLayout,
                                                                    const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PreCallRecordGetImageSubresourceLayout2KHR(VkDevice device, VkImage image,
                                                              const VkImageSubresource2KHR* pSubresource,
                                                              VkSubresourceLayout2KHR* pLayout, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(image, record_obj.location);
}

void ThreadSafety::PostCallRecordGetImageSubresourceLayout2KHR(VkDevice device, VkImage image,
                                                               const VkImageSubresource2KHR* pSubresource,
                                                               VkSubresourceLayout2KHR* pLayout, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(image, record_obj.location);
}

void ThreadSafety::PreCallRecordGetCalibratedTimestampsKHR(VkDevice device, uint32_t timestampCount,
                                                           const VkCalibratedTimestampInfoKHR* pTimestampInfos,
                                                           uint64_t* pTimestamps, uint64_t* pMaxDeviation,
                                                           const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordGetCalibratedTimestampsKHR(VkDevice device, uint32_t timestampCount,
                                                            const VkCalibratedTimestampInfoKHR* pTimestampInfos,
                                                            uint64_t* pTimestamps, uint64_t* pMaxDeviation,
                                                            const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PreCallRecordCmdBindDescriptorSets2KHR(VkCommandBuffer commandBuffer,
                                                          const VkBindDescriptorSetsInfoKHR* pBindDescriptorSetsInfo,
                                                          const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdBindDescriptorSets2KHR(VkCommandBuffer commandBuffer,
                                                           const VkBindDescriptorSetsInfoKHR* pBindDescriptorSetsInfo,
                                                           const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdPushConstants2KHR(VkCommandBuffer commandBuffer,
                                                     const VkPushConstantsInfoKHR* pPushConstantsInfo,
                                                     const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdPushConstants2KHR(VkCommandBuffer commandBuffer,
                                                      const VkPushConstantsInfoKHR* pPushConstantsInfo,
                                                      const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdPushDescriptorSet2KHR(VkCommandBuffer commandBuffer,
                                                         const VkPushDescriptorSetInfoKHR* pPushDescriptorSetInfo,
                                                         const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdPushDescriptorSet2KHR(VkCommandBuffer commandBuffer,
                                                          const VkPushDescriptorSetInfoKHR* pPushDescriptorSetInfo,
                                                          const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdPushDescriptorSetWithTemplate2KHR(
    VkCommandBuffer commandBuffer, const VkPushDescriptorSetWithTemplateInfoKHR* pPushDescriptorSetWithTemplateInfo,
    const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdPushDescriptorSetWithTemplate2KHR(
    VkCommandBuffer commandBuffer, const VkPushDescriptorSetWithTemplateInfoKHR* pPushDescriptorSetWithTemplateInfo,
    const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetDescriptorBufferOffsets2EXT(
    VkCommandBuffer commandBuffer, const VkSetDescriptorBufferOffsetsInfoEXT* pSetDescriptorBufferOffsetsInfo,
    const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetDescriptorBufferOffsets2EXT(
    VkCommandBuffer commandBuffer, const VkSetDescriptorBufferOffsetsInfoEXT* pSetDescriptorBufferOffsetsInfo,
    const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdBindDescriptorBufferEmbeddedSamplers2EXT(
    VkCommandBuffer commandBuffer, const VkBindDescriptorBufferEmbeddedSamplersInfoEXT* pBindDescriptorBufferEmbeddedSamplersInfo,
    const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdBindDescriptorBufferEmbeddedSamplers2EXT(
    VkCommandBuffer commandBuffer, const VkBindDescriptorBufferEmbeddedSamplersInfoEXT* pBindDescriptorBufferEmbeddedSamplersInfo,
    const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCreateDebugReportCallbackEXT(VkInstance instance,
                                                             const VkDebugReportCallbackCreateInfoEXT* pCreateInfo,
                                                             const VkAllocationCallbacks* pAllocator,
                                                             VkDebugReportCallbackEXT* pCallback, const RecordObject& record_obj) {
    StartReadObjectParentInstance(instance, record_obj.location);
}

void ThreadSafety::PostCallRecordCreateDebugReportCallbackEXT(VkInstance instance,
                                                              const VkDebugReportCallbackCreateInfoEXT* pCreateInfo,
                                                              const VkAllocationCallbacks* pAllocator,
                                                              VkDebugReportCallbackEXT* pCallback, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(instance, record_obj.location);
    if (record_obj.result == VK_SUCCESS) {
        CreateObjectParentInstance(*pCallback);
    }
}

void ThreadSafety::PreCallRecordDestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback,
                                                              const VkAllocationCallbacks* pAllocator,
                                                              const RecordObject& record_obj) {
    StartReadObjectParentInstance(instance, record_obj.location);
    StartWriteObjectParentInstance(callback, record_obj.location);
    // Host access to callback must be externally synchronized
}

void ThreadSafety::PostCallRecordDestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback,
                                                               const VkAllocationCallbacks* pAllocator,
                                                               const RecordObject& record_obj) {
    FinishReadObjectParentInstance(instance, record_obj.location);
    FinishWriteObjectParentInstance(callback, record_obj.location);
    DestroyObjectParentInstance(callback);
    // Host access to callback must be externally synchronized
}

void ThreadSafety::PreCallRecordDebugReportMessageEXT(VkInstance instance, VkDebugReportFlagsEXT flags,
                                                      VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location,
                                                      int32_t messageCode, const char* pLayerPrefix, const char* pMessage,
                                                      const RecordObject& record_obj) {
    StartReadObjectParentInstance(instance, record_obj.location);
}

void ThreadSafety::PostCallRecordDebugReportMessageEXT(VkInstance instance, VkDebugReportFlagsEXT flags,
                                                       VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location,
                                                       int32_t messageCode, const char* pLayerPrefix, const char* pMessage,
                                                       const RecordObject& record_obj) {
    FinishReadObjectParentInstance(instance, record_obj.location);
}

void ThreadSafety::PreCallRecordCmdBindTransformFeedbackBuffersEXT(VkCommandBuffer commandBuffer, uint32_t firstBinding,
                                                                   uint32_t bindingCount, const VkBuffer* pBuffers,
                                                                   const VkDeviceSize* pOffsets, const VkDeviceSize* pSizes,
                                                                   const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);

    if (pBuffers) {
        for (uint32_t index = 0; index < bindingCount; index++) {
            StartReadObject(pBuffers[index], record_obj.location);
        }
    }
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdBindTransformFeedbackBuffersEXT(VkCommandBuffer commandBuffer, uint32_t firstBinding,
                                                                    uint32_t bindingCount, const VkBuffer* pBuffers,
                                                                    const VkDeviceSize* pOffsets, const VkDeviceSize* pSizes,
                                                                    const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);

    if (pBuffers) {
        for (uint32_t index = 0; index < bindingCount; index++) {
            FinishReadObject(pBuffers[index], record_obj.location);
        }
    }
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdBeginTransformFeedbackEXT(VkCommandBuffer commandBuffer, uint32_t firstCounterBuffer,
                                                             uint32_t counterBufferCount, const VkBuffer* pCounterBuffers,
                                                             const VkDeviceSize* pCounterBufferOffsets,
                                                             const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);

    if (pCounterBuffers) {
        for (uint32_t index = 0; index < counterBufferCount; index++) {
            StartReadObject(pCounterBuffers[index], record_obj.location);
        }
    }
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdBeginTransformFeedbackEXT(VkCommandBuffer commandBuffer, uint32_t firstCounterBuffer,
                                                              uint32_t counterBufferCount, const VkBuffer* pCounterBuffers,
                                                              const VkDeviceSize* pCounterBufferOffsets,
                                                              const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);

    if (pCounterBuffers) {
        for (uint32_t index = 0; index < counterBufferCount; index++) {
            FinishReadObject(pCounterBuffers[index], record_obj.location);
        }
    }
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdEndTransformFeedbackEXT(VkCommandBuffer commandBuffer, uint32_t firstCounterBuffer,
                                                           uint32_t counterBufferCount, const VkBuffer* pCounterBuffers,
                                                           const VkDeviceSize* pCounterBufferOffsets,
                                                           const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);

    if (pCounterBuffers) {
        for (uint32_t index = 0; index < counterBufferCount; index++) {
            StartReadObject(pCounterBuffers[index], record_obj.location);
        }
    }
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdEndTransformFeedbackEXT(VkCommandBuffer commandBuffer, uint32_t firstCounterBuffer,
                                                            uint32_t counterBufferCount, const VkBuffer* pCounterBuffers,
                                                            const VkDeviceSize* pCounterBufferOffsets,
                                                            const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);

    if (pCounterBuffers) {
        for (uint32_t index = 0; index < counterBufferCount; index++) {
            FinishReadObject(pCounterBuffers[index], record_obj.location);
        }
    }
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdBeginQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query,
                                                        VkQueryControlFlags flags, uint32_t index, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    StartReadObject(queryPool, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdBeginQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query,
                                                         VkQueryControlFlags flags, uint32_t index,
                                                         const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    FinishReadObject(queryPool, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdEndQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query,
                                                      uint32_t index, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    StartReadObject(queryPool, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdEndQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query,
                                                       uint32_t index, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    FinishReadObject(queryPool, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdDrawIndirectByteCountEXT(VkCommandBuffer commandBuffer, uint32_t instanceCount,
                                                            uint32_t firstInstance, VkBuffer counterBuffer,
                                                            VkDeviceSize counterBufferOffset, uint32_t counterOffset,
                                                            uint32_t vertexStride, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    StartReadObject(counterBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdDrawIndirectByteCountEXT(VkCommandBuffer commandBuffer, uint32_t instanceCount,
                                                             uint32_t firstInstance, VkBuffer counterBuffer,
                                                             VkDeviceSize counterBufferOffset, uint32_t counterOffset,
                                                             uint32_t vertexStride, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    FinishReadObject(counterBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCreateCuModuleNVX(VkDevice device, const VkCuModuleCreateInfoNVX* pCreateInfo,
                                                  const VkAllocationCallbacks* pAllocator, VkCuModuleNVX* pModule,
                                                  const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordCreateCuModuleNVX(VkDevice device, const VkCuModuleCreateInfoNVX* pCreateInfo,
                                                   const VkAllocationCallbacks* pAllocator, VkCuModuleNVX* pModule,
                                                   const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pModule);
    }
}

void ThreadSafety::PreCallRecordCreateCuFunctionNVX(VkDevice device, const VkCuFunctionCreateInfoNVX* pCreateInfo,
                                                    const VkAllocationCallbacks* pAllocator, VkCuFunctionNVX* pFunction,
                                                    const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordCreateCuFunctionNVX(VkDevice device, const VkCuFunctionCreateInfoNVX* pCreateInfo,
                                                     const VkAllocationCallbacks* pAllocator, VkCuFunctionNVX* pFunction,
                                                     const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pFunction);
    }
}

void ThreadSafety::PreCallRecordDestroyCuModuleNVX(VkDevice device, VkCuModuleNVX module, const VkAllocationCallbacks* pAllocator,
                                                   const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(module, record_obj.location);
}

void ThreadSafety::PostCallRecordDestroyCuModuleNVX(VkDevice device, VkCuModuleNVX module, const VkAllocationCallbacks* pAllocator,
                                                    const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(module, record_obj.location);
}

void ThreadSafety::PreCallRecordDestroyCuFunctionNVX(VkDevice device, VkCuFunctionNVX function,
                                                     const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(function, record_obj.location);
}

void ThreadSafety::PostCallRecordDestroyCuFunctionNVX(VkDevice device, VkCuFunctionNVX function,
                                                      const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(function, record_obj.location);
}

void ThreadSafety::PreCallRecordCmdCuLaunchKernelNVX(VkCommandBuffer commandBuffer, const VkCuLaunchInfoNVX* pLaunchInfo,
                                                     const RecordObject& record_obj) {
    StartReadObject(commandBuffer, record_obj.location);
}

void ThreadSafety::PostCallRecordCmdCuLaunchKernelNVX(VkCommandBuffer commandBuffer, const VkCuLaunchInfoNVX* pLaunchInfo,
                                                      const RecordObject& record_obj) {
    FinishReadObject(commandBuffer, record_obj.location);
}

void ThreadSafety::PreCallRecordGetImageViewHandleNVX(VkDevice device, const VkImageViewHandleInfoNVX* pInfo,
                                                      const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordGetImageViewHandleNVX(VkDevice device, const VkImageViewHandleInfoNVX* pInfo,
                                                       const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PreCallRecordGetImageViewAddressNVX(VkDevice device, VkImageView imageView,
                                                       VkImageViewAddressPropertiesNVX* pProperties,
                                                       const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(imageView, record_obj.location);
}

void ThreadSafety::PostCallRecordGetImageViewAddressNVX(VkDevice device, VkImageView imageView,
                                                        VkImageViewAddressPropertiesNVX* pProperties,
                                                        const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(imageView, record_obj.location);
}

void ThreadSafety::PreCallRecordCmdDrawIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                        VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                        uint32_t stride, const RecordObject& record_obj) {
    PreCallRecordCmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride,
                                      record_obj);
}

void ThreadSafety::PostCallRecordCmdDrawIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                         VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                         uint32_t maxDrawCount, uint32_t stride, const RecordObject& record_obj) {
    PostCallRecordCmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride,
                                       record_obj);
}

void ThreadSafety::PreCallRecordCmdDrawIndexedIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                               VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                               uint32_t maxDrawCount, uint32_t stride,
                                                               const RecordObject& record_obj) {
    PreCallRecordCmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride,
                                             record_obj);
}

void ThreadSafety::PostCallRecordCmdDrawIndexedIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                                VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                                uint32_t maxDrawCount, uint32_t stride,
                                                                const RecordObject& record_obj) {
    PostCallRecordCmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride,
                                              record_obj);
}

void ThreadSafety::PreCallRecordGetShaderInfoAMD(VkDevice device, VkPipeline pipeline, VkShaderStageFlagBits shaderStage,
                                                 VkShaderInfoTypeAMD infoType, size_t* pInfoSize, void* pInfo,
                                                 const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(pipeline, record_obj.location);
}

void ThreadSafety::PostCallRecordGetShaderInfoAMD(VkDevice device, VkPipeline pipeline, VkShaderStageFlagBits shaderStage,
                                                  VkShaderInfoTypeAMD infoType, size_t* pInfoSize, void* pInfo,
                                                  const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(pipeline, record_obj.location);
}

#ifdef VK_USE_PLATFORM_GGP
void ThreadSafety::PreCallRecordCreateStreamDescriptorSurfaceGGP(VkInstance instance,
                                                                 const VkStreamDescriptorSurfaceCreateInfoGGP* pCreateInfo,
                                                                 const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                                 const RecordObject& record_obj) {
    StartReadObjectParentInstance(instance, record_obj.location);
}

void ThreadSafety::PostCallRecordCreateStreamDescriptorSurfaceGGP(VkInstance instance,
                                                                  const VkStreamDescriptorSurfaceCreateInfoGGP* pCreateInfo,
                                                                  const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                                  const RecordObject& record_obj) {
    FinishReadObjectParentInstance(instance, record_obj.location);
    if (record_obj.result == VK_SUCCESS) {
        CreateObjectParentInstance(*pSurface);
    }
}

#endif  // VK_USE_PLATFORM_GGP
#ifdef VK_USE_PLATFORM_WIN32_KHR
void ThreadSafety::PreCallRecordGetMemoryWin32HandleNV(VkDevice device, VkDeviceMemory memory,
                                                       VkExternalMemoryHandleTypeFlagsNV handleType, HANDLE* pHandle,
                                                       const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(memory, record_obj.location);
}

void ThreadSafety::PostCallRecordGetMemoryWin32HandleNV(VkDevice device, VkDeviceMemory memory,
                                                        VkExternalMemoryHandleTypeFlagsNV handleType, HANDLE* pHandle,
                                                        const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(memory, record_obj.location);
}

#endif  // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_VI_NN
void ThreadSafety::PreCallRecordCreateViSurfaceNN(VkInstance instance, const VkViSurfaceCreateInfoNN* pCreateInfo,
                                                  const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                  const RecordObject& record_obj) {
    StartReadObjectParentInstance(instance, record_obj.location);
}

void ThreadSafety::PostCallRecordCreateViSurfaceNN(VkInstance instance, const VkViSurfaceCreateInfoNN* pCreateInfo,
                                                   const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                   const RecordObject& record_obj) {
    FinishReadObjectParentInstance(instance, record_obj.location);
    if (record_obj.result == VK_SUCCESS) {
        CreateObjectParentInstance(*pSurface);
    }
}

#endif  // VK_USE_PLATFORM_VI_NN
void ThreadSafety::PreCallRecordCmdBeginConditionalRenderingEXT(
    VkCommandBuffer commandBuffer, const VkConditionalRenderingBeginInfoEXT* pConditionalRenderingBegin,
    const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdBeginConditionalRenderingEXT(
    VkCommandBuffer commandBuffer, const VkConditionalRenderingBeginInfoEXT* pConditionalRenderingBegin,
    const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdEndConditionalRenderingEXT(VkCommandBuffer commandBuffer, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdEndConditionalRenderingEXT(VkCommandBuffer commandBuffer, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetViewportWScalingNV(VkCommandBuffer commandBuffer, uint32_t firstViewport,
                                                         uint32_t viewportCount, const VkViewportWScalingNV* pViewportWScalings,
                                                         const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetViewportWScalingNV(VkCommandBuffer commandBuffer, uint32_t firstViewport,
                                                          uint32_t viewportCount, const VkViewportWScalingNV* pViewportWScalings,
                                                          const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordReleaseDisplayEXT(VkPhysicalDevice physicalDevice, VkDisplayKHR display,
                                                  const RecordObject& record_obj) {
    StartReadObjectParentInstance(display, record_obj.location);
}

void ThreadSafety::PostCallRecordReleaseDisplayEXT(VkPhysicalDevice physicalDevice, VkDisplayKHR display,
                                                   const RecordObject& record_obj) {
    FinishReadObjectParentInstance(display, record_obj.location);
}

#ifdef VK_USE_PLATFORM_XLIB_XRANDR_EXT
void ThreadSafety::PreCallRecordAcquireXlibDisplayEXT(VkPhysicalDevice physicalDevice, Display* dpy, VkDisplayKHR display,
                                                      const RecordObject& record_obj) {
    StartReadObjectParentInstance(display, record_obj.location);
}

void ThreadSafety::PostCallRecordAcquireXlibDisplayEXT(VkPhysicalDevice physicalDevice, Display* dpy, VkDisplayKHR display,
                                                       const RecordObject& record_obj) {
    FinishReadObjectParentInstance(display, record_obj.location);
}

#endif  // VK_USE_PLATFORM_XLIB_XRANDR_EXT
void ThreadSafety::PreCallRecordGetPhysicalDeviceSurfaceCapabilities2EXT(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                         VkSurfaceCapabilities2EXT* pSurfaceCapabilities,
                                                                         const RecordObject& record_obj) {
    StartReadObjectParentInstance(surface, record_obj.location);
}

void ThreadSafety::PostCallRecordGetPhysicalDeviceSurfaceCapabilities2EXT(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                          VkSurfaceCapabilities2EXT* pSurfaceCapabilities,
                                                                          const RecordObject& record_obj) {
    FinishReadObjectParentInstance(surface, record_obj.location);
}

void ThreadSafety::PreCallRecordDisplayPowerControlEXT(VkDevice device, VkDisplayKHR display,
                                                       const VkDisplayPowerInfoEXT* pDisplayPowerInfo,
                                                       const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObjectParentInstance(display, record_obj.location);
}

void ThreadSafety::PostCallRecordDisplayPowerControlEXT(VkDevice device, VkDisplayKHR display,
                                                        const VkDisplayPowerInfoEXT* pDisplayPowerInfo,
                                                        const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObjectParentInstance(display, record_obj.location);
}

void ThreadSafety::PreCallRecordRegisterDeviceEventEXT(VkDevice device, const VkDeviceEventInfoEXT* pDeviceEventInfo,
                                                       const VkAllocationCallbacks* pAllocator, VkFence* pFence,
                                                       const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordRegisterDeviceEventEXT(VkDevice device, const VkDeviceEventInfoEXT* pDeviceEventInfo,
                                                        const VkAllocationCallbacks* pAllocator, VkFence* pFence,
                                                        const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PreCallRecordGetSwapchainCounterEXT(VkDevice device, VkSwapchainKHR swapchain,
                                                       VkSurfaceCounterFlagBitsEXT counter, uint64_t* pCounterValue,
                                                       const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(swapchain, record_obj.location);
}

void ThreadSafety::PostCallRecordGetSwapchainCounterEXT(VkDevice device, VkSwapchainKHR swapchain,
                                                        VkSurfaceCounterFlagBitsEXT counter, uint64_t* pCounterValue,
                                                        const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(swapchain, record_obj.location);
}

void ThreadSafety::PreCallRecordGetRefreshCycleDurationGOOGLE(VkDevice device, VkSwapchainKHR swapchain,
                                                              VkRefreshCycleDurationGOOGLE* pDisplayTimingProperties,
                                                              const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartWriteObject(swapchain, record_obj.location);
    // Host access to swapchain must be externally synchronized
}

void ThreadSafety::PostCallRecordGetRefreshCycleDurationGOOGLE(VkDevice device, VkSwapchainKHR swapchain,
                                                               VkRefreshCycleDurationGOOGLE* pDisplayTimingProperties,
                                                               const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishWriteObject(swapchain, record_obj.location);
    // Host access to swapchain must be externally synchronized
}

void ThreadSafety::PreCallRecordGetPastPresentationTimingGOOGLE(VkDevice device, VkSwapchainKHR swapchain,
                                                                uint32_t* pPresentationTimingCount,
                                                                VkPastPresentationTimingGOOGLE* pPresentationTimings,
                                                                const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartWriteObject(swapchain, record_obj.location);
    // Host access to swapchain must be externally synchronized
}

void ThreadSafety::PostCallRecordGetPastPresentationTimingGOOGLE(VkDevice device, VkSwapchainKHR swapchain,
                                                                 uint32_t* pPresentationTimingCount,
                                                                 VkPastPresentationTimingGOOGLE* pPresentationTimings,
                                                                 const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishWriteObject(swapchain, record_obj.location);
    // Host access to swapchain must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetDiscardRectangleEXT(VkCommandBuffer commandBuffer, uint32_t firstDiscardRectangle,
                                                          uint32_t discardRectangleCount, const VkRect2D* pDiscardRectangles,
                                                          const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetDiscardRectangleEXT(VkCommandBuffer commandBuffer, uint32_t firstDiscardRectangle,
                                                           uint32_t discardRectangleCount, const VkRect2D* pDiscardRectangles,
                                                           const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetDiscardRectangleEnableEXT(VkCommandBuffer commandBuffer, VkBool32 discardRectangleEnable,
                                                                const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetDiscardRectangleEnableEXT(VkCommandBuffer commandBuffer, VkBool32 discardRectangleEnable,
                                                                 const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetDiscardRectangleModeEXT(VkCommandBuffer commandBuffer,
                                                              VkDiscardRectangleModeEXT discardRectangleMode,
                                                              const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetDiscardRectangleModeEXT(VkCommandBuffer commandBuffer,
                                                               VkDiscardRectangleModeEXT discardRectangleMode,
                                                               const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordSetHdrMetadataEXT(VkDevice device, uint32_t swapchainCount, const VkSwapchainKHR* pSwapchains,
                                                  const VkHdrMetadataEXT* pMetadata, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);

    if (pSwapchains) {
        for (uint32_t index = 0; index < swapchainCount; index++) {
            StartReadObject(pSwapchains[index], record_obj.location);
        }
    }
}

void ThreadSafety::PostCallRecordSetHdrMetadataEXT(VkDevice device, uint32_t swapchainCount, const VkSwapchainKHR* pSwapchains,
                                                   const VkHdrMetadataEXT* pMetadata, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);

    if (pSwapchains) {
        for (uint32_t index = 0; index < swapchainCount; index++) {
            FinishReadObject(pSwapchains[index], record_obj.location);
        }
    }
}

#ifdef VK_USE_PLATFORM_IOS_MVK
void ThreadSafety::PreCallRecordCreateIOSSurfaceMVK(VkInstance instance, const VkIOSSurfaceCreateInfoMVK* pCreateInfo,
                                                    const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                    const RecordObject& record_obj) {
    StartReadObjectParentInstance(instance, record_obj.location);
}

void ThreadSafety::PostCallRecordCreateIOSSurfaceMVK(VkInstance instance, const VkIOSSurfaceCreateInfoMVK* pCreateInfo,
                                                     const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                     const RecordObject& record_obj) {
    FinishReadObjectParentInstance(instance, record_obj.location);
    if (record_obj.result == VK_SUCCESS) {
        CreateObjectParentInstance(*pSurface);
    }
}

#endif  // VK_USE_PLATFORM_IOS_MVK
#ifdef VK_USE_PLATFORM_MACOS_MVK
void ThreadSafety::PreCallRecordCreateMacOSSurfaceMVK(VkInstance instance, const VkMacOSSurfaceCreateInfoMVK* pCreateInfo,
                                                      const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                      const RecordObject& record_obj) {
    StartReadObjectParentInstance(instance, record_obj.location);
}

void ThreadSafety::PostCallRecordCreateMacOSSurfaceMVK(VkInstance instance, const VkMacOSSurfaceCreateInfoMVK* pCreateInfo,
                                                       const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                       const RecordObject& record_obj) {
    FinishReadObjectParentInstance(instance, record_obj.location);
    if (record_obj.result == VK_SUCCESS) {
        CreateObjectParentInstance(*pSurface);
    }
}

#endif  // VK_USE_PLATFORM_MACOS_MVK
void ThreadSafety::PreCallRecordQueueBeginDebugUtilsLabelEXT(VkQueue queue, const VkDebugUtilsLabelEXT* pLabelInfo,
                                                             const RecordObject& record_obj) {
    StartReadObject(queue, record_obj.location);
}

void ThreadSafety::PostCallRecordQueueBeginDebugUtilsLabelEXT(VkQueue queue, const VkDebugUtilsLabelEXT* pLabelInfo,
                                                              const RecordObject& record_obj) {
    FinishReadObject(queue, record_obj.location);
}

void ThreadSafety::PreCallRecordQueueEndDebugUtilsLabelEXT(VkQueue queue, const RecordObject& record_obj) {
    StartReadObject(queue, record_obj.location);
}

void ThreadSafety::PostCallRecordQueueEndDebugUtilsLabelEXT(VkQueue queue, const RecordObject& record_obj) {
    FinishReadObject(queue, record_obj.location);
}

void ThreadSafety::PreCallRecordQueueInsertDebugUtilsLabelEXT(VkQueue queue, const VkDebugUtilsLabelEXT* pLabelInfo,
                                                              const RecordObject& record_obj) {
    StartReadObject(queue, record_obj.location);
}

void ThreadSafety::PostCallRecordQueueInsertDebugUtilsLabelEXT(VkQueue queue, const VkDebugUtilsLabelEXT* pLabelInfo,
                                                               const RecordObject& record_obj) {
    FinishReadObject(queue, record_obj.location);
}

void ThreadSafety::PreCallRecordCmdBeginDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT* pLabelInfo,
                                                           const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdBeginDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT* pLabelInfo,
                                                            const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdEndDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdEndDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdInsertDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT* pLabelInfo,
                                                            const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdInsertDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT* pLabelInfo,
                                                             const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCreateDebugUtilsMessengerEXT(VkInstance instance,
                                                             const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                                             const VkAllocationCallbacks* pAllocator,
                                                             VkDebugUtilsMessengerEXT* pMessenger, const RecordObject& record_obj) {
    StartReadObjectParentInstance(instance, record_obj.location);
}

void ThreadSafety::PostCallRecordCreateDebugUtilsMessengerEXT(VkInstance instance,
                                                              const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                                              const VkAllocationCallbacks* pAllocator,
                                                              VkDebugUtilsMessengerEXT* pMessenger,
                                                              const RecordObject& record_obj) {
    FinishReadObjectParentInstance(instance, record_obj.location);
    if (record_obj.result == VK_SUCCESS) {
        CreateObjectParentInstance(*pMessenger);
    }
}

void ThreadSafety::PreCallRecordDestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT messenger,
                                                              const VkAllocationCallbacks* pAllocator,
                                                              const RecordObject& record_obj) {
    StartReadObjectParentInstance(instance, record_obj.location);
    StartWriteObjectParentInstance(messenger, record_obj.location);
    // Host access to messenger must be externally synchronized
}

void ThreadSafety::PostCallRecordDestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT messenger,
                                                               const VkAllocationCallbacks* pAllocator,
                                                               const RecordObject& record_obj) {
    FinishReadObjectParentInstance(instance, record_obj.location);
    FinishWriteObjectParentInstance(messenger, record_obj.location);
    DestroyObjectParentInstance(messenger);
    // Host access to messenger must be externally synchronized
}

void ThreadSafety::PreCallRecordSubmitDebugUtilsMessageEXT(VkInstance instance,
                                                           VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                           VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                                                           const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                           const RecordObject& record_obj) {
    StartReadObjectParentInstance(instance, record_obj.location);
}

void ThreadSafety::PostCallRecordSubmitDebugUtilsMessageEXT(VkInstance instance,
                                                            VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                            VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                                                            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                            const RecordObject& record_obj) {
    FinishReadObjectParentInstance(instance, record_obj.location);
}

#ifdef VK_USE_PLATFORM_ANDROID_KHR
void ThreadSafety::PreCallRecordGetAndroidHardwareBufferPropertiesANDROID(VkDevice device, const struct AHardwareBuffer* buffer,
                                                                          VkAndroidHardwareBufferPropertiesANDROID* pProperties,
                                                                          const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordGetAndroidHardwareBufferPropertiesANDROID(VkDevice device, const struct AHardwareBuffer* buffer,
                                                                           VkAndroidHardwareBufferPropertiesANDROID* pProperties,
                                                                           const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PreCallRecordGetMemoryAndroidHardwareBufferANDROID(VkDevice device,
                                                                      const VkMemoryGetAndroidHardwareBufferInfoANDROID* pInfo,
                                                                      struct AHardwareBuffer** pBuffer,
                                                                      const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordGetMemoryAndroidHardwareBufferANDROID(VkDevice device,
                                                                       const VkMemoryGetAndroidHardwareBufferInfoANDROID* pInfo,
                                                                       struct AHardwareBuffer** pBuffer,
                                                                       const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

#endif  // VK_USE_PLATFORM_ANDROID_KHR
#ifdef VK_ENABLE_BETA_EXTENSIONS
void ThreadSafety::PreCallRecordCreateExecutionGraphPipelinesAMDX(VkDevice device, VkPipelineCache pipelineCache,
                                                                  uint32_t createInfoCount,
                                                                  const VkExecutionGraphPipelineCreateInfoAMDX* pCreateInfos,
                                                                  const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                                  const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(pipelineCache, record_obj.location);
}

void ThreadSafety::PostCallRecordCreateExecutionGraphPipelinesAMDX(VkDevice device, VkPipelineCache pipelineCache,
                                                                   uint32_t createInfoCount,
                                                                   const VkExecutionGraphPipelineCreateInfoAMDX* pCreateInfos,
                                                                   const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                                   const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(pipelineCache, record_obj.location);
    if (pPipelines) {
        for (uint32_t index = 0; index < createInfoCount; index++) {
            if (!pPipelines[index]) continue;
            CreateObject(pPipelines[index]);
        }
    }
}

void ThreadSafety::PreCallRecordGetExecutionGraphPipelineScratchSizeAMDX(VkDevice device, VkPipeline executionGraph,
                                                                         VkExecutionGraphPipelineScratchSizeAMDX* pSizeInfo,
                                                                         const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(executionGraph, record_obj.location);
}

void ThreadSafety::PostCallRecordGetExecutionGraphPipelineScratchSizeAMDX(VkDevice device, VkPipeline executionGraph,
                                                                          VkExecutionGraphPipelineScratchSizeAMDX* pSizeInfo,
                                                                          const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(executionGraph, record_obj.location);
}

void ThreadSafety::PreCallRecordGetExecutionGraphPipelineNodeIndexAMDX(VkDevice device, VkPipeline executionGraph,
                                                                       const VkPipelineShaderStageNodeCreateInfoAMDX* pNodeInfo,
                                                                       uint32_t* pNodeIndex, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(executionGraph, record_obj.location);
}

void ThreadSafety::PostCallRecordGetExecutionGraphPipelineNodeIndexAMDX(VkDevice device, VkPipeline executionGraph,
                                                                        const VkPipelineShaderStageNodeCreateInfoAMDX* pNodeInfo,
                                                                        uint32_t* pNodeIndex, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(executionGraph, record_obj.location);
}

void ThreadSafety::PreCallRecordCmdInitializeGraphScratchMemoryAMDX(VkCommandBuffer commandBuffer, VkDeviceAddress scratch,
                                                                    const RecordObject& record_obj) {
    StartReadObject(commandBuffer, record_obj.location);
}

void ThreadSafety::PostCallRecordCmdInitializeGraphScratchMemoryAMDX(VkCommandBuffer commandBuffer, VkDeviceAddress scratch,
                                                                     const RecordObject& record_obj) {
    FinishReadObject(commandBuffer, record_obj.location);
}

void ThreadSafety::PreCallRecordCmdDispatchGraphAMDX(VkCommandBuffer commandBuffer, VkDeviceAddress scratch,
                                                     const VkDispatchGraphCountInfoAMDX* pCountInfo,
                                                     const RecordObject& record_obj) {
    StartReadObject(commandBuffer, record_obj.location);
}

void ThreadSafety::PostCallRecordCmdDispatchGraphAMDX(VkCommandBuffer commandBuffer, VkDeviceAddress scratch,
                                                      const VkDispatchGraphCountInfoAMDX* pCountInfo,
                                                      const RecordObject& record_obj) {
    FinishReadObject(commandBuffer, record_obj.location);
}

void ThreadSafety::PreCallRecordCmdDispatchGraphIndirectAMDX(VkCommandBuffer commandBuffer, VkDeviceAddress scratch,
                                                             const VkDispatchGraphCountInfoAMDX* pCountInfo,
                                                             const RecordObject& record_obj) {
    StartReadObject(commandBuffer, record_obj.location);
}

void ThreadSafety::PostCallRecordCmdDispatchGraphIndirectAMDX(VkCommandBuffer commandBuffer, VkDeviceAddress scratch,
                                                              const VkDispatchGraphCountInfoAMDX* pCountInfo,
                                                              const RecordObject& record_obj) {
    FinishReadObject(commandBuffer, record_obj.location);
}

void ThreadSafety::PreCallRecordCmdDispatchGraphIndirectCountAMDX(VkCommandBuffer commandBuffer, VkDeviceAddress scratch,
                                                                  VkDeviceAddress countInfo, const RecordObject& record_obj) {
    StartReadObject(commandBuffer, record_obj.location);
}

void ThreadSafety::PostCallRecordCmdDispatchGraphIndirectCountAMDX(VkCommandBuffer commandBuffer, VkDeviceAddress scratch,
                                                                   VkDeviceAddress countInfo, const RecordObject& record_obj) {
    FinishReadObject(commandBuffer, record_obj.location);
}

#endif  // VK_ENABLE_BETA_EXTENSIONS
void ThreadSafety::PreCallRecordCmdSetSampleLocationsEXT(VkCommandBuffer commandBuffer,
                                                         const VkSampleLocationsInfoEXT* pSampleLocationsInfo,
                                                         const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetSampleLocationsEXT(VkCommandBuffer commandBuffer,
                                                          const VkSampleLocationsInfoEXT* pSampleLocationsInfo,
                                                          const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordGetImageDrmFormatModifierPropertiesEXT(VkDevice device, VkImage image,
                                                                       VkImageDrmFormatModifierPropertiesEXT* pProperties,
                                                                       const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(image, record_obj.location);
}

void ThreadSafety::PostCallRecordGetImageDrmFormatModifierPropertiesEXT(VkDevice device, VkImage image,
                                                                        VkImageDrmFormatModifierPropertiesEXT* pProperties,
                                                                        const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(image, record_obj.location);
}

void ThreadSafety::PreCallRecordCreateValidationCacheEXT(VkDevice device, const VkValidationCacheCreateInfoEXT* pCreateInfo,
                                                         const VkAllocationCallbacks* pAllocator,
                                                         VkValidationCacheEXT* pValidationCache, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordCreateValidationCacheEXT(VkDevice device, const VkValidationCacheCreateInfoEXT* pCreateInfo,
                                                          const VkAllocationCallbacks* pAllocator,
                                                          VkValidationCacheEXT* pValidationCache, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pValidationCache);
    }
}

void ThreadSafety::PreCallRecordDestroyValidationCacheEXT(VkDevice device, VkValidationCacheEXT validationCache,
                                                          const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartWriteObject(validationCache, record_obj.location);
    // Host access to validationCache must be externally synchronized
}

void ThreadSafety::PostCallRecordDestroyValidationCacheEXT(VkDevice device, VkValidationCacheEXT validationCache,
                                                           const VkAllocationCallbacks* pAllocator,
                                                           const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishWriteObject(validationCache, record_obj.location);
    DestroyObject(validationCache);
    // Host access to validationCache must be externally synchronized
}

void ThreadSafety::PreCallRecordMergeValidationCachesEXT(VkDevice device, VkValidationCacheEXT dstCache, uint32_t srcCacheCount,
                                                         const VkValidationCacheEXT* pSrcCaches, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartWriteObject(dstCache, record_obj.location);

    if (pSrcCaches) {
        for (uint32_t index = 0; index < srcCacheCount; index++) {
            StartReadObject(pSrcCaches[index], record_obj.location);
        }
    }
    // Host access to dstCache must be externally synchronized
}

void ThreadSafety::PostCallRecordMergeValidationCachesEXT(VkDevice device, VkValidationCacheEXT dstCache, uint32_t srcCacheCount,
                                                          const VkValidationCacheEXT* pSrcCaches, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishWriteObject(dstCache, record_obj.location);

    if (pSrcCaches) {
        for (uint32_t index = 0; index < srcCacheCount; index++) {
            FinishReadObject(pSrcCaches[index], record_obj.location);
        }
    }
    // Host access to dstCache must be externally synchronized
}

void ThreadSafety::PreCallRecordGetValidationCacheDataEXT(VkDevice device, VkValidationCacheEXT validationCache, size_t* pDataSize,
                                                          void* pData, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(validationCache, record_obj.location);
}

void ThreadSafety::PostCallRecordGetValidationCacheDataEXT(VkDevice device, VkValidationCacheEXT validationCache, size_t* pDataSize,
                                                           void* pData, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(validationCache, record_obj.location);
}

void ThreadSafety::PreCallRecordCmdBindShadingRateImageNV(VkCommandBuffer commandBuffer, VkImageView imageView,
                                                          VkImageLayout imageLayout, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    StartReadObject(imageView, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdBindShadingRateImageNV(VkCommandBuffer commandBuffer, VkImageView imageView,
                                                           VkImageLayout imageLayout, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    FinishReadObject(imageView, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetViewportShadingRatePaletteNV(VkCommandBuffer commandBuffer, uint32_t firstViewport,
                                                                   uint32_t viewportCount,
                                                                   const VkShadingRatePaletteNV* pShadingRatePalettes,
                                                                   const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetViewportShadingRatePaletteNV(VkCommandBuffer commandBuffer, uint32_t firstViewport,
                                                                    uint32_t viewportCount,
                                                                    const VkShadingRatePaletteNV* pShadingRatePalettes,
                                                                    const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetCoarseSampleOrderNV(VkCommandBuffer commandBuffer, VkCoarseSampleOrderTypeNV sampleOrderType,
                                                          uint32_t customSampleOrderCount,
                                                          const VkCoarseSampleOrderCustomNV* pCustomSampleOrders,
                                                          const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetCoarseSampleOrderNV(VkCommandBuffer commandBuffer, VkCoarseSampleOrderTypeNV sampleOrderType,
                                                           uint32_t customSampleOrderCount,
                                                           const VkCoarseSampleOrderCustomNV* pCustomSampleOrders,
                                                           const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCreateAccelerationStructureNV(VkDevice device,
                                                              const VkAccelerationStructureCreateInfoNV* pCreateInfo,
                                                              const VkAllocationCallbacks* pAllocator,
                                                              VkAccelerationStructureNV* pAccelerationStructure,
                                                              const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordCreateAccelerationStructureNV(VkDevice device,
                                                               const VkAccelerationStructureCreateInfoNV* pCreateInfo,
                                                               const VkAllocationCallbacks* pAllocator,
                                                               VkAccelerationStructureNV* pAccelerationStructure,
                                                               const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pAccelerationStructure);
    }
}

void ThreadSafety::PreCallRecordDestroyAccelerationStructureNV(VkDevice device, VkAccelerationStructureNV accelerationStructure,
                                                               const VkAllocationCallbacks* pAllocator,
                                                               const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartWriteObject(accelerationStructure, record_obj.location);
    // Host access to accelerationStructure must be externally synchronized
}

void ThreadSafety::PostCallRecordDestroyAccelerationStructureNV(VkDevice device, VkAccelerationStructureNV accelerationStructure,
                                                                const VkAllocationCallbacks* pAllocator,
                                                                const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishWriteObject(accelerationStructure, record_obj.location);
    DestroyObject(accelerationStructure);
    // Host access to accelerationStructure must be externally synchronized
}

void ThreadSafety::PreCallRecordGetAccelerationStructureMemoryRequirementsNV(
    VkDevice device, const VkAccelerationStructureMemoryRequirementsInfoNV* pInfo, VkMemoryRequirements2KHR* pMemoryRequirements,
    const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordGetAccelerationStructureMemoryRequirementsNV(
    VkDevice device, const VkAccelerationStructureMemoryRequirementsInfoNV* pInfo, VkMemoryRequirements2KHR* pMemoryRequirements,
    const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PreCallRecordBindAccelerationStructureMemoryNV(VkDevice device, uint32_t bindInfoCount,
                                                                  const VkBindAccelerationStructureMemoryInfoNV* pBindInfos,
                                                                  const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordBindAccelerationStructureMemoryNV(VkDevice device, uint32_t bindInfoCount,
                                                                   const VkBindAccelerationStructureMemoryInfoNV* pBindInfos,
                                                                   const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PreCallRecordCmdBuildAccelerationStructureNV(VkCommandBuffer commandBuffer,
                                                                const VkAccelerationStructureInfoNV* pInfo, VkBuffer instanceData,
                                                                VkDeviceSize instanceOffset, VkBool32 update,
                                                                VkAccelerationStructureNV dst, VkAccelerationStructureNV src,
                                                                VkBuffer scratch, VkDeviceSize scratchOffset,
                                                                const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    StartReadObject(instanceData, record_obj.location);
    StartReadObject(dst, record_obj.location);
    StartReadObject(src, record_obj.location);
    StartReadObject(scratch, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdBuildAccelerationStructureNV(VkCommandBuffer commandBuffer,
                                                                 const VkAccelerationStructureInfoNV* pInfo, VkBuffer instanceData,
                                                                 VkDeviceSize instanceOffset, VkBool32 update,
                                                                 VkAccelerationStructureNV dst, VkAccelerationStructureNV src,
                                                                 VkBuffer scratch, VkDeviceSize scratchOffset,
                                                                 const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    FinishReadObject(instanceData, record_obj.location);
    FinishReadObject(dst, record_obj.location);
    FinishReadObject(src, record_obj.location);
    FinishReadObject(scratch, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdCopyAccelerationStructureNV(VkCommandBuffer commandBuffer, VkAccelerationStructureNV dst,
                                                               VkAccelerationStructureNV src,
                                                               VkCopyAccelerationStructureModeKHR mode,
                                                               const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    StartReadObject(dst, record_obj.location);
    StartReadObject(src, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdCopyAccelerationStructureNV(VkCommandBuffer commandBuffer, VkAccelerationStructureNV dst,
                                                                VkAccelerationStructureNV src,
                                                                VkCopyAccelerationStructureModeKHR mode,
                                                                const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    FinishReadObject(dst, record_obj.location);
    FinishReadObject(src, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdTraceRaysNV(VkCommandBuffer commandBuffer, VkBuffer raygenShaderBindingTableBuffer,
                                               VkDeviceSize raygenShaderBindingOffset, VkBuffer missShaderBindingTableBuffer,
                                               VkDeviceSize missShaderBindingOffset, VkDeviceSize missShaderBindingStride,
                                               VkBuffer hitShaderBindingTableBuffer, VkDeviceSize hitShaderBindingOffset,
                                               VkDeviceSize hitShaderBindingStride, VkBuffer callableShaderBindingTableBuffer,
                                               VkDeviceSize callableShaderBindingOffset, VkDeviceSize callableShaderBindingStride,
                                               uint32_t width, uint32_t height, uint32_t depth, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    StartReadObject(raygenShaderBindingTableBuffer, record_obj.location);
    StartReadObject(missShaderBindingTableBuffer, record_obj.location);
    StartReadObject(hitShaderBindingTableBuffer, record_obj.location);
    StartReadObject(callableShaderBindingTableBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdTraceRaysNV(VkCommandBuffer commandBuffer, VkBuffer raygenShaderBindingTableBuffer,
                                                VkDeviceSize raygenShaderBindingOffset, VkBuffer missShaderBindingTableBuffer,
                                                VkDeviceSize missShaderBindingOffset, VkDeviceSize missShaderBindingStride,
                                                VkBuffer hitShaderBindingTableBuffer, VkDeviceSize hitShaderBindingOffset,
                                                VkDeviceSize hitShaderBindingStride, VkBuffer callableShaderBindingTableBuffer,
                                                VkDeviceSize callableShaderBindingOffset, VkDeviceSize callableShaderBindingStride,
                                                uint32_t width, uint32_t height, uint32_t depth, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    FinishReadObject(raygenShaderBindingTableBuffer, record_obj.location);
    FinishReadObject(missShaderBindingTableBuffer, record_obj.location);
    FinishReadObject(hitShaderBindingTableBuffer, record_obj.location);
    FinishReadObject(callableShaderBindingTableBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCreateRayTracingPipelinesNV(VkDevice device, VkPipelineCache pipelineCache,
                                                            uint32_t createInfoCount,
                                                            const VkRayTracingPipelineCreateInfoNV* pCreateInfos,
                                                            const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                            const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(pipelineCache, record_obj.location);
}

void ThreadSafety::PostCallRecordCreateRayTracingPipelinesNV(VkDevice device, VkPipelineCache pipelineCache,
                                                             uint32_t createInfoCount,
                                                             const VkRayTracingPipelineCreateInfoNV* pCreateInfos,
                                                             const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                             const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(pipelineCache, record_obj.location);
    if (pPipelines) {
        for (uint32_t index = 0; index < createInfoCount; index++) {
            if (!pPipelines[index]) continue;
            CreateObject(pPipelines[index]);
        }
    }
}

void ThreadSafety::PreCallRecordGetRayTracingShaderGroupHandlesKHR(VkDevice device, VkPipeline pipeline, uint32_t firstGroup,
                                                                   uint32_t groupCount, size_t dataSize, void* pData,
                                                                   const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(pipeline, record_obj.location);
}

void ThreadSafety::PostCallRecordGetRayTracingShaderGroupHandlesKHR(VkDevice device, VkPipeline pipeline, uint32_t firstGroup,
                                                                    uint32_t groupCount, size_t dataSize, void* pData,
                                                                    const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(pipeline, record_obj.location);
}

void ThreadSafety::PreCallRecordGetRayTracingShaderGroupHandlesNV(VkDevice device, VkPipeline pipeline, uint32_t firstGroup,
                                                                  uint32_t groupCount, size_t dataSize, void* pData,
                                                                  const RecordObject& record_obj) {
    PreCallRecordGetRayTracingShaderGroupHandlesKHR(device, pipeline, firstGroup, groupCount, dataSize, pData, record_obj);
}

void ThreadSafety::PostCallRecordGetRayTracingShaderGroupHandlesNV(VkDevice device, VkPipeline pipeline, uint32_t firstGroup,
                                                                   uint32_t groupCount, size_t dataSize, void* pData,
                                                                   const RecordObject& record_obj) {
    PostCallRecordGetRayTracingShaderGroupHandlesKHR(device, pipeline, firstGroup, groupCount, dataSize, pData, record_obj);
}

void ThreadSafety::PreCallRecordGetAccelerationStructureHandleNV(VkDevice device, VkAccelerationStructureNV accelerationStructure,
                                                                 size_t dataSize, void* pData, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(accelerationStructure, record_obj.location);
}

void ThreadSafety::PostCallRecordGetAccelerationStructureHandleNV(VkDevice device, VkAccelerationStructureNV accelerationStructure,
                                                                  size_t dataSize, void* pData, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(accelerationStructure, record_obj.location);
}

void ThreadSafety::PreCallRecordCmdWriteAccelerationStructuresPropertiesNV(VkCommandBuffer commandBuffer,
                                                                           uint32_t accelerationStructureCount,
                                                                           const VkAccelerationStructureNV* pAccelerationStructures,
                                                                           VkQueryType queryType, VkQueryPool queryPool,
                                                                           uint32_t firstQuery, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);

    if (pAccelerationStructures) {
        for (uint32_t index = 0; index < accelerationStructureCount; index++) {
            StartReadObject(pAccelerationStructures[index], record_obj.location);
        }
    }
    StartReadObject(queryPool, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdWriteAccelerationStructuresPropertiesNV(
    VkCommandBuffer commandBuffer, uint32_t accelerationStructureCount, const VkAccelerationStructureNV* pAccelerationStructures,
    VkQueryType queryType, VkQueryPool queryPool, uint32_t firstQuery, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);

    if (pAccelerationStructures) {
        for (uint32_t index = 0; index < accelerationStructureCount; index++) {
            FinishReadObject(pAccelerationStructures[index], record_obj.location);
        }
    }
    FinishReadObject(queryPool, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCompileDeferredNV(VkDevice device, VkPipeline pipeline, uint32_t shader,
                                                  const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(pipeline, record_obj.location);
}

void ThreadSafety::PostCallRecordCompileDeferredNV(VkDevice device, VkPipeline pipeline, uint32_t shader,
                                                   const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(pipeline, record_obj.location);
}

void ThreadSafety::PreCallRecordGetMemoryHostPointerPropertiesEXT(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType,
                                                                  const void* pHostPointer,
                                                                  VkMemoryHostPointerPropertiesEXT* pMemoryHostPointerProperties,
                                                                  const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordGetMemoryHostPointerPropertiesEXT(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType,
                                                                   const void* pHostPointer,
                                                                   VkMemoryHostPointerPropertiesEXT* pMemoryHostPointerProperties,
                                                                   const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PreCallRecordCmdWriteBufferMarkerAMD(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage,
                                                        VkBuffer dstBuffer, VkDeviceSize dstOffset, uint32_t marker,
                                                        const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    StartReadObject(dstBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdWriteBufferMarkerAMD(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage,
                                                         VkBuffer dstBuffer, VkDeviceSize dstOffset, uint32_t marker,
                                                         const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    FinishReadObject(dstBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordGetCalibratedTimestampsEXT(VkDevice device, uint32_t timestampCount,
                                                           const VkCalibratedTimestampInfoKHR* pTimestampInfos,
                                                           uint64_t* pTimestamps, uint64_t* pMaxDeviation,
                                                           const RecordObject& record_obj) {
    PreCallRecordGetCalibratedTimestampsKHR(device, timestampCount, pTimestampInfos, pTimestamps, pMaxDeviation, record_obj);
}

void ThreadSafety::PostCallRecordGetCalibratedTimestampsEXT(VkDevice device, uint32_t timestampCount,
                                                            const VkCalibratedTimestampInfoKHR* pTimestampInfos,
                                                            uint64_t* pTimestamps, uint64_t* pMaxDeviation,
                                                            const RecordObject& record_obj) {
    PostCallRecordGetCalibratedTimestampsKHR(device, timestampCount, pTimestampInfos, pTimestamps, pMaxDeviation, record_obj);
}

void ThreadSafety::PreCallRecordCmdDrawMeshTasksNV(VkCommandBuffer commandBuffer, uint32_t taskCount, uint32_t firstTask,
                                                   const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdDrawMeshTasksNV(VkCommandBuffer commandBuffer, uint32_t taskCount, uint32_t firstTask,
                                                    const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdDrawMeshTasksIndirectNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                           uint32_t drawCount, uint32_t stride, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    StartReadObject(buffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdDrawMeshTasksIndirectNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                            uint32_t drawCount, uint32_t stride, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    FinishReadObject(buffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdDrawMeshTasksIndirectCountNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                                VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                                uint32_t maxDrawCount, uint32_t stride,
                                                                const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    StartReadObject(buffer, record_obj.location);
    StartReadObject(countBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdDrawMeshTasksIndirectCountNV(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                 VkDeviceSize offset, VkBuffer countBuffer,
                                                                 VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                                 uint32_t stride, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    FinishReadObject(buffer, record_obj.location);
    FinishReadObject(countBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetExclusiveScissorEnableNV(VkCommandBuffer commandBuffer, uint32_t firstExclusiveScissor,
                                                               uint32_t exclusiveScissorCount,
                                                               const VkBool32* pExclusiveScissorEnables,
                                                               const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetExclusiveScissorEnableNV(VkCommandBuffer commandBuffer, uint32_t firstExclusiveScissor,
                                                                uint32_t exclusiveScissorCount,
                                                                const VkBool32* pExclusiveScissorEnables,
                                                                const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetExclusiveScissorNV(VkCommandBuffer commandBuffer, uint32_t firstExclusiveScissor,
                                                         uint32_t exclusiveScissorCount, const VkRect2D* pExclusiveScissors,
                                                         const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetExclusiveScissorNV(VkCommandBuffer commandBuffer, uint32_t firstExclusiveScissor,
                                                          uint32_t exclusiveScissorCount, const VkRect2D* pExclusiveScissors,
                                                          const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetCheckpointNV(VkCommandBuffer commandBuffer, const void* pCheckpointMarker,
                                                   const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetCheckpointNV(VkCommandBuffer commandBuffer, const void* pCheckpointMarker,
                                                    const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordGetQueueCheckpointDataNV(VkQueue queue, uint32_t* pCheckpointDataCount,
                                                         VkCheckpointDataNV* pCheckpointData, const RecordObject& record_obj) {
    StartReadObject(queue, record_obj.location);
}

void ThreadSafety::PostCallRecordGetQueueCheckpointDataNV(VkQueue queue, uint32_t* pCheckpointDataCount,
                                                          VkCheckpointDataNV* pCheckpointData, const RecordObject& record_obj) {
    FinishReadObject(queue, record_obj.location);
}

void ThreadSafety::PreCallRecordInitializePerformanceApiINTEL(VkDevice device,
                                                              const VkInitializePerformanceApiInfoINTEL* pInitializeInfo,
                                                              const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordInitializePerformanceApiINTEL(VkDevice device,
                                                               const VkInitializePerformanceApiInfoINTEL* pInitializeInfo,
                                                               const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PreCallRecordUninitializePerformanceApiINTEL(VkDevice device, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordUninitializePerformanceApiINTEL(VkDevice device, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PreCallRecordCmdSetPerformanceMarkerINTEL(VkCommandBuffer commandBuffer,
                                                             const VkPerformanceMarkerInfoINTEL* pMarkerInfo,
                                                             const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetPerformanceMarkerINTEL(VkCommandBuffer commandBuffer,
                                                              const VkPerformanceMarkerInfoINTEL* pMarkerInfo,
                                                              const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetPerformanceStreamMarkerINTEL(VkCommandBuffer commandBuffer,
                                                                   const VkPerformanceStreamMarkerInfoINTEL* pMarkerInfo,
                                                                   const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetPerformanceStreamMarkerINTEL(VkCommandBuffer commandBuffer,
                                                                    const VkPerformanceStreamMarkerInfoINTEL* pMarkerInfo,
                                                                    const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetPerformanceOverrideINTEL(VkCommandBuffer commandBuffer,
                                                               const VkPerformanceOverrideInfoINTEL* pOverrideInfo,
                                                               const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetPerformanceOverrideINTEL(VkCommandBuffer commandBuffer,
                                                                const VkPerformanceOverrideInfoINTEL* pOverrideInfo,
                                                                const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordAcquirePerformanceConfigurationINTEL(VkDevice device,
                                                                     const VkPerformanceConfigurationAcquireInfoINTEL* pAcquireInfo,
                                                                     VkPerformanceConfigurationINTEL* pConfiguration,
                                                                     const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordAcquirePerformanceConfigurationINTEL(
    VkDevice device, const VkPerformanceConfigurationAcquireInfoINTEL* pAcquireInfo,
    VkPerformanceConfigurationINTEL* pConfiguration, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pConfiguration);
    }
}

void ThreadSafety::PreCallRecordReleasePerformanceConfigurationINTEL(VkDevice device, VkPerformanceConfigurationINTEL configuration,
                                                                     const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartWriteObject(configuration, record_obj.location);
    // Host access to configuration must be externally synchronized
}

void ThreadSafety::PostCallRecordReleasePerformanceConfigurationINTEL(VkDevice device,
                                                                      VkPerformanceConfigurationINTEL configuration,
                                                                      const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishWriteObject(configuration, record_obj.location);
    DestroyObject(configuration);
    // Host access to configuration must be externally synchronized
}

void ThreadSafety::PreCallRecordQueueSetPerformanceConfigurationINTEL(VkQueue queue, VkPerformanceConfigurationINTEL configuration,
                                                                      const RecordObject& record_obj) {
    StartReadObject(queue, record_obj.location);
    StartReadObject(configuration, record_obj.location);
}

void ThreadSafety::PostCallRecordQueueSetPerformanceConfigurationINTEL(VkQueue queue, VkPerformanceConfigurationINTEL configuration,
                                                                       const RecordObject& record_obj) {
    FinishReadObject(queue, record_obj.location);
    FinishReadObject(configuration, record_obj.location);
}

void ThreadSafety::PreCallRecordGetPerformanceParameterINTEL(VkDevice device, VkPerformanceParameterTypeINTEL parameter,
                                                             VkPerformanceValueINTEL* pValue, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordGetPerformanceParameterINTEL(VkDevice device, VkPerformanceParameterTypeINTEL parameter,
                                                              VkPerformanceValueINTEL* pValue, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PreCallRecordSetLocalDimmingAMD(VkDevice device, VkSwapchainKHR swapChain, VkBool32 localDimmingEnable,
                                                   const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(swapChain, record_obj.location);
}

void ThreadSafety::PostCallRecordSetLocalDimmingAMD(VkDevice device, VkSwapchainKHR swapChain, VkBool32 localDimmingEnable,
                                                    const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(swapChain, record_obj.location);
}

#ifdef VK_USE_PLATFORM_FUCHSIA
void ThreadSafety::PreCallRecordCreateImagePipeSurfaceFUCHSIA(VkInstance instance,
                                                              const VkImagePipeSurfaceCreateInfoFUCHSIA* pCreateInfo,
                                                              const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                              const RecordObject& record_obj) {
    StartReadObjectParentInstance(instance, record_obj.location);
}

void ThreadSafety::PostCallRecordCreateImagePipeSurfaceFUCHSIA(VkInstance instance,
                                                               const VkImagePipeSurfaceCreateInfoFUCHSIA* pCreateInfo,
                                                               const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                               const RecordObject& record_obj) {
    FinishReadObjectParentInstance(instance, record_obj.location);
    if (record_obj.result == VK_SUCCESS) {
        CreateObjectParentInstance(*pSurface);
    }
}

#endif  // VK_USE_PLATFORM_FUCHSIA
#ifdef VK_USE_PLATFORM_METAL_EXT
void ThreadSafety::PreCallRecordCreateMetalSurfaceEXT(VkInstance instance, const VkMetalSurfaceCreateInfoEXT* pCreateInfo,
                                                      const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                      const RecordObject& record_obj) {
    StartReadObjectParentInstance(instance, record_obj.location);
}

void ThreadSafety::PostCallRecordCreateMetalSurfaceEXT(VkInstance instance, const VkMetalSurfaceCreateInfoEXT* pCreateInfo,
                                                       const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                       const RecordObject& record_obj) {
    FinishReadObjectParentInstance(instance, record_obj.location);
    if (record_obj.result == VK_SUCCESS) {
        CreateObjectParentInstance(*pSurface);
    }
}

#endif  // VK_USE_PLATFORM_METAL_EXT
void ThreadSafety::PreCallRecordGetBufferDeviceAddressEXT(VkDevice device, const VkBufferDeviceAddressInfo* pInfo,
                                                          const RecordObject& record_obj) {
    PreCallRecordGetBufferDeviceAddress(device, pInfo, record_obj);
}

void ThreadSafety::PostCallRecordGetBufferDeviceAddressEXT(VkDevice device, const VkBufferDeviceAddressInfo* pInfo,
                                                           const RecordObject& record_obj) {
    PostCallRecordGetBufferDeviceAddress(device, pInfo, record_obj);
}

#ifdef VK_USE_PLATFORM_WIN32_KHR
void ThreadSafety::PreCallRecordAcquireFullScreenExclusiveModeEXT(VkDevice device, VkSwapchainKHR swapchain,
                                                                  const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(swapchain, record_obj.location);
}

void ThreadSafety::PostCallRecordAcquireFullScreenExclusiveModeEXT(VkDevice device, VkSwapchainKHR swapchain,
                                                                   const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(swapchain, record_obj.location);
}

void ThreadSafety::PreCallRecordReleaseFullScreenExclusiveModeEXT(VkDevice device, VkSwapchainKHR swapchain,
                                                                  const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(swapchain, record_obj.location);
}

void ThreadSafety::PostCallRecordReleaseFullScreenExclusiveModeEXT(VkDevice device, VkSwapchainKHR swapchain,
                                                                   const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(swapchain, record_obj.location);
}

void ThreadSafety::PreCallRecordGetDeviceGroupSurfacePresentModes2EXT(VkDevice device,
                                                                      const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo,
                                                                      VkDeviceGroupPresentModeFlagsKHR* pModes,
                                                                      const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordGetDeviceGroupSurfacePresentModes2EXT(VkDevice device,
                                                                       const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo,
                                                                       VkDeviceGroupPresentModeFlagsKHR* pModes,
                                                                       const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

#endif  // VK_USE_PLATFORM_WIN32_KHR
void ThreadSafety::PreCallRecordCreateHeadlessSurfaceEXT(VkInstance instance, const VkHeadlessSurfaceCreateInfoEXT* pCreateInfo,
                                                         const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                         const RecordObject& record_obj) {
    StartReadObjectParentInstance(instance, record_obj.location);
}

void ThreadSafety::PostCallRecordCreateHeadlessSurfaceEXT(VkInstance instance, const VkHeadlessSurfaceCreateInfoEXT* pCreateInfo,
                                                          const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                          const RecordObject& record_obj) {
    FinishReadObjectParentInstance(instance, record_obj.location);
    if (record_obj.result == VK_SUCCESS) {
        CreateObjectParentInstance(*pSurface);
    }
}

void ThreadSafety::PreCallRecordCmdSetLineStippleEXT(VkCommandBuffer commandBuffer, uint32_t lineStippleFactor,
                                                     uint16_t lineStipplePattern, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetLineStippleEXT(VkCommandBuffer commandBuffer, uint32_t lineStippleFactor,
                                                      uint16_t lineStipplePattern, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordResetQueryPoolEXT(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount,
                                                  const RecordObject& record_obj) {
    PreCallRecordResetQueryPool(device, queryPool, firstQuery, queryCount, record_obj);
}

void ThreadSafety::PostCallRecordResetQueryPoolEXT(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount,
                                                   const RecordObject& record_obj) {
    PostCallRecordResetQueryPool(device, queryPool, firstQuery, queryCount, record_obj);
}

void ThreadSafety::PreCallRecordCmdSetCullModeEXT(VkCommandBuffer commandBuffer, VkCullModeFlags cullMode,
                                                  const RecordObject& record_obj) {
    PreCallRecordCmdSetCullMode(commandBuffer, cullMode, record_obj);
}

void ThreadSafety::PostCallRecordCmdSetCullModeEXT(VkCommandBuffer commandBuffer, VkCullModeFlags cullMode,
                                                   const RecordObject& record_obj) {
    PostCallRecordCmdSetCullMode(commandBuffer, cullMode, record_obj);
}

void ThreadSafety::PreCallRecordCmdSetFrontFaceEXT(VkCommandBuffer commandBuffer, VkFrontFace frontFace,
                                                   const RecordObject& record_obj) {
    PreCallRecordCmdSetFrontFace(commandBuffer, frontFace, record_obj);
}

void ThreadSafety::PostCallRecordCmdSetFrontFaceEXT(VkCommandBuffer commandBuffer, VkFrontFace frontFace,
                                                    const RecordObject& record_obj) {
    PostCallRecordCmdSetFrontFace(commandBuffer, frontFace, record_obj);
}

void ThreadSafety::PreCallRecordCmdSetPrimitiveTopologyEXT(VkCommandBuffer commandBuffer, VkPrimitiveTopology primitiveTopology,
                                                           const RecordObject& record_obj) {
    PreCallRecordCmdSetPrimitiveTopology(commandBuffer, primitiveTopology, record_obj);
}

void ThreadSafety::PostCallRecordCmdSetPrimitiveTopologyEXT(VkCommandBuffer commandBuffer, VkPrimitiveTopology primitiveTopology,
                                                            const RecordObject& record_obj) {
    PostCallRecordCmdSetPrimitiveTopology(commandBuffer, primitiveTopology, record_obj);
}

void ThreadSafety::PreCallRecordCmdSetViewportWithCountEXT(VkCommandBuffer commandBuffer, uint32_t viewportCount,
                                                           const VkViewport* pViewports, const RecordObject& record_obj) {
    PreCallRecordCmdSetViewportWithCount(commandBuffer, viewportCount, pViewports, record_obj);
}

void ThreadSafety::PostCallRecordCmdSetViewportWithCountEXT(VkCommandBuffer commandBuffer, uint32_t viewportCount,
                                                            const VkViewport* pViewports, const RecordObject& record_obj) {
    PostCallRecordCmdSetViewportWithCount(commandBuffer, viewportCount, pViewports, record_obj);
}

void ThreadSafety::PreCallRecordCmdSetScissorWithCountEXT(VkCommandBuffer commandBuffer, uint32_t scissorCount,
                                                          const VkRect2D* pScissors, const RecordObject& record_obj) {
    PreCallRecordCmdSetScissorWithCount(commandBuffer, scissorCount, pScissors, record_obj);
}

void ThreadSafety::PostCallRecordCmdSetScissorWithCountEXT(VkCommandBuffer commandBuffer, uint32_t scissorCount,
                                                           const VkRect2D* pScissors, const RecordObject& record_obj) {
    PostCallRecordCmdSetScissorWithCount(commandBuffer, scissorCount, pScissors, record_obj);
}

void ThreadSafety::PreCallRecordCmdBindVertexBuffers2EXT(VkCommandBuffer commandBuffer, uint32_t firstBinding,
                                                         uint32_t bindingCount, const VkBuffer* pBuffers,
                                                         const VkDeviceSize* pOffsets, const VkDeviceSize* pSizes,
                                                         const VkDeviceSize* pStrides, const RecordObject& record_obj) {
    PreCallRecordCmdBindVertexBuffers2(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets, pSizes, pStrides, record_obj);
}

void ThreadSafety::PostCallRecordCmdBindVertexBuffers2EXT(VkCommandBuffer commandBuffer, uint32_t firstBinding,
                                                          uint32_t bindingCount, const VkBuffer* pBuffers,
                                                          const VkDeviceSize* pOffsets, const VkDeviceSize* pSizes,
                                                          const VkDeviceSize* pStrides, const RecordObject& record_obj) {
    PostCallRecordCmdBindVertexBuffers2(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets, pSizes, pStrides,
                                        record_obj);
}

void ThreadSafety::PreCallRecordCmdSetDepthTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthTestEnable,
                                                         const RecordObject& record_obj) {
    PreCallRecordCmdSetDepthTestEnable(commandBuffer, depthTestEnable, record_obj);
}

void ThreadSafety::PostCallRecordCmdSetDepthTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthTestEnable,
                                                          const RecordObject& record_obj) {
    PostCallRecordCmdSetDepthTestEnable(commandBuffer, depthTestEnable, record_obj);
}

void ThreadSafety::PreCallRecordCmdSetDepthWriteEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthWriteEnable,
                                                          const RecordObject& record_obj) {
    PreCallRecordCmdSetDepthWriteEnable(commandBuffer, depthWriteEnable, record_obj);
}

void ThreadSafety::PostCallRecordCmdSetDepthWriteEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthWriteEnable,
                                                           const RecordObject& record_obj) {
    PostCallRecordCmdSetDepthWriteEnable(commandBuffer, depthWriteEnable, record_obj);
}

void ThreadSafety::PreCallRecordCmdSetDepthCompareOpEXT(VkCommandBuffer commandBuffer, VkCompareOp depthCompareOp,
                                                        const RecordObject& record_obj) {
    PreCallRecordCmdSetDepthCompareOp(commandBuffer, depthCompareOp, record_obj);
}

void ThreadSafety::PostCallRecordCmdSetDepthCompareOpEXT(VkCommandBuffer commandBuffer, VkCompareOp depthCompareOp,
                                                         const RecordObject& record_obj) {
    PostCallRecordCmdSetDepthCompareOp(commandBuffer, depthCompareOp, record_obj);
}

void ThreadSafety::PreCallRecordCmdSetDepthBoundsTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthBoundsTestEnable,
                                                               const RecordObject& record_obj) {
    PreCallRecordCmdSetDepthBoundsTestEnable(commandBuffer, depthBoundsTestEnable, record_obj);
}

void ThreadSafety::PostCallRecordCmdSetDepthBoundsTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthBoundsTestEnable,
                                                                const RecordObject& record_obj) {
    PostCallRecordCmdSetDepthBoundsTestEnable(commandBuffer, depthBoundsTestEnable, record_obj);
}

void ThreadSafety::PreCallRecordCmdSetStencilTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 stencilTestEnable,
                                                           const RecordObject& record_obj) {
    PreCallRecordCmdSetStencilTestEnable(commandBuffer, stencilTestEnable, record_obj);
}

void ThreadSafety::PostCallRecordCmdSetStencilTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 stencilTestEnable,
                                                            const RecordObject& record_obj) {
    PostCallRecordCmdSetStencilTestEnable(commandBuffer, stencilTestEnable, record_obj);
}

void ThreadSafety::PreCallRecordCmdSetStencilOpEXT(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, VkStencilOp failOp,
                                                   VkStencilOp passOp, VkStencilOp depthFailOp, VkCompareOp compareOp,
                                                   const RecordObject& record_obj) {
    PreCallRecordCmdSetStencilOp(commandBuffer, faceMask, failOp, passOp, depthFailOp, compareOp, record_obj);
}

void ThreadSafety::PostCallRecordCmdSetStencilOpEXT(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, VkStencilOp failOp,
                                                    VkStencilOp passOp, VkStencilOp depthFailOp, VkCompareOp compareOp,
                                                    const RecordObject& record_obj) {
    PostCallRecordCmdSetStencilOp(commandBuffer, faceMask, failOp, passOp, depthFailOp, compareOp, record_obj);
}

void ThreadSafety::PreCallRecordCopyMemoryToImageEXT(VkDevice device, const VkCopyMemoryToImageInfoEXT* pCopyMemoryToImageInfo,
                                                     const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordCopyMemoryToImageEXT(VkDevice device, const VkCopyMemoryToImageInfoEXT* pCopyMemoryToImageInfo,
                                                      const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PreCallRecordCopyImageToMemoryEXT(VkDevice device, const VkCopyImageToMemoryInfoEXT* pCopyImageToMemoryInfo,
                                                     const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordCopyImageToMemoryEXT(VkDevice device, const VkCopyImageToMemoryInfoEXT* pCopyImageToMemoryInfo,
                                                      const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PreCallRecordCopyImageToImageEXT(VkDevice device, const VkCopyImageToImageInfoEXT* pCopyImageToImageInfo,
                                                    const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordCopyImageToImageEXT(VkDevice device, const VkCopyImageToImageInfoEXT* pCopyImageToImageInfo,
                                                     const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PreCallRecordTransitionImageLayoutEXT(VkDevice device, uint32_t transitionCount,
                                                         const VkHostImageLayoutTransitionInfoEXT* pTransitions,
                                                         const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordTransitionImageLayoutEXT(VkDevice device, uint32_t transitionCount,
                                                          const VkHostImageLayoutTransitionInfoEXT* pTransitions,
                                                          const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PreCallRecordGetImageSubresourceLayout2EXT(VkDevice device, VkImage image,
                                                              const VkImageSubresource2KHR* pSubresource,
                                                              VkSubresourceLayout2KHR* pLayout, const RecordObject& record_obj) {
    PreCallRecordGetImageSubresourceLayout2KHR(device, image, pSubresource, pLayout, record_obj);
}

void ThreadSafety::PostCallRecordGetImageSubresourceLayout2EXT(VkDevice device, VkImage image,
                                                               const VkImageSubresource2KHR* pSubresource,
                                                               VkSubresourceLayout2KHR* pLayout, const RecordObject& record_obj) {
    PostCallRecordGetImageSubresourceLayout2KHR(device, image, pSubresource, pLayout, record_obj);
}

void ThreadSafety::PreCallRecordReleaseSwapchainImagesEXT(VkDevice device, const VkReleaseSwapchainImagesInfoEXT* pReleaseInfo,
                                                          const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordReleaseSwapchainImagesEXT(VkDevice device, const VkReleaseSwapchainImagesInfoEXT* pReleaseInfo,
                                                           const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PreCallRecordGetGeneratedCommandsMemoryRequirementsNV(VkDevice device,
                                                                         const VkGeneratedCommandsMemoryRequirementsInfoNV* pInfo,
                                                                         VkMemoryRequirements2* pMemoryRequirements,
                                                                         const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordGetGeneratedCommandsMemoryRequirementsNV(VkDevice device,
                                                                          const VkGeneratedCommandsMemoryRequirementsInfoNV* pInfo,
                                                                          VkMemoryRequirements2* pMemoryRequirements,
                                                                          const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PreCallRecordCmdPreprocessGeneratedCommandsNV(VkCommandBuffer commandBuffer,
                                                                 const VkGeneratedCommandsInfoNV* pGeneratedCommandsInfo,
                                                                 const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdPreprocessGeneratedCommandsNV(VkCommandBuffer commandBuffer,
                                                                  const VkGeneratedCommandsInfoNV* pGeneratedCommandsInfo,
                                                                  const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdExecuteGeneratedCommandsNV(VkCommandBuffer commandBuffer, VkBool32 isPreprocessed,
                                                              const VkGeneratedCommandsInfoNV* pGeneratedCommandsInfo,
                                                              const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdExecuteGeneratedCommandsNV(VkCommandBuffer commandBuffer, VkBool32 isPreprocessed,
                                                               const VkGeneratedCommandsInfoNV* pGeneratedCommandsInfo,
                                                               const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdBindPipelineShaderGroupNV(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                             VkPipeline pipeline, uint32_t groupIndex,
                                                             const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    StartReadObject(pipeline, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdBindPipelineShaderGroupNV(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                              VkPipeline pipeline, uint32_t groupIndex,
                                                              const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    FinishReadObject(pipeline, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCreateIndirectCommandsLayoutNV(VkDevice device,
                                                               const VkIndirectCommandsLayoutCreateInfoNV* pCreateInfo,
                                                               const VkAllocationCallbacks* pAllocator,
                                                               VkIndirectCommandsLayoutNV* pIndirectCommandsLayout,
                                                               const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordCreateIndirectCommandsLayoutNV(VkDevice device,
                                                                const VkIndirectCommandsLayoutCreateInfoNV* pCreateInfo,
                                                                const VkAllocationCallbacks* pAllocator,
                                                                VkIndirectCommandsLayoutNV* pIndirectCommandsLayout,
                                                                const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pIndirectCommandsLayout);
    }
}

void ThreadSafety::PreCallRecordDestroyIndirectCommandsLayoutNV(VkDevice device, VkIndirectCommandsLayoutNV indirectCommandsLayout,
                                                                const VkAllocationCallbacks* pAllocator,
                                                                const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartWriteObject(indirectCommandsLayout, record_obj.location);
    // Host access to indirectCommandsLayout must be externally synchronized
}

void ThreadSafety::PostCallRecordDestroyIndirectCommandsLayoutNV(VkDevice device, VkIndirectCommandsLayoutNV indirectCommandsLayout,
                                                                 const VkAllocationCallbacks* pAllocator,
                                                                 const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishWriteObject(indirectCommandsLayout, record_obj.location);
    DestroyObject(indirectCommandsLayout);
    // Host access to indirectCommandsLayout must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetDepthBias2EXT(VkCommandBuffer commandBuffer, const VkDepthBiasInfoEXT* pDepthBiasInfo,
                                                    const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetDepthBias2EXT(VkCommandBuffer commandBuffer, const VkDepthBiasInfoEXT* pDepthBiasInfo,
                                                     const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordAcquireDrmDisplayEXT(VkPhysicalDevice physicalDevice, int32_t drmFd, VkDisplayKHR display,
                                                     const RecordObject& record_obj) {
    StartReadObjectParentInstance(display, record_obj.location);
}

void ThreadSafety::PostCallRecordAcquireDrmDisplayEXT(VkPhysicalDevice physicalDevice, int32_t drmFd, VkDisplayKHR display,
                                                      const RecordObject& record_obj) {
    FinishReadObjectParentInstance(display, record_obj.location);
}

void ThreadSafety::PreCallRecordCreatePrivateDataSlotEXT(VkDevice device, const VkPrivateDataSlotCreateInfo* pCreateInfo,
                                                         const VkAllocationCallbacks* pAllocator,
                                                         VkPrivateDataSlot* pPrivateDataSlot, const RecordObject& record_obj) {
    PreCallRecordCreatePrivateDataSlot(device, pCreateInfo, pAllocator, pPrivateDataSlot, record_obj);
}

void ThreadSafety::PostCallRecordCreatePrivateDataSlotEXT(VkDevice device, const VkPrivateDataSlotCreateInfo* pCreateInfo,
                                                          const VkAllocationCallbacks* pAllocator,
                                                          VkPrivateDataSlot* pPrivateDataSlot, const RecordObject& record_obj) {
    PostCallRecordCreatePrivateDataSlot(device, pCreateInfo, pAllocator, pPrivateDataSlot, record_obj);
}

void ThreadSafety::PreCallRecordDestroyPrivateDataSlotEXT(VkDevice device, VkPrivateDataSlot privateDataSlot,
                                                          const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {
    PreCallRecordDestroyPrivateDataSlot(device, privateDataSlot, pAllocator, record_obj);
}

void ThreadSafety::PostCallRecordDestroyPrivateDataSlotEXT(VkDevice device, VkPrivateDataSlot privateDataSlot,
                                                           const VkAllocationCallbacks* pAllocator,
                                                           const RecordObject& record_obj) {
    PostCallRecordDestroyPrivateDataSlot(device, privateDataSlot, pAllocator, record_obj);
}

void ThreadSafety::PreCallRecordSetPrivateDataEXT(VkDevice device, VkObjectType objectType, uint64_t objectHandle,
                                                  VkPrivateDataSlot privateDataSlot, uint64_t data,
                                                  const RecordObject& record_obj) {
    PreCallRecordSetPrivateData(device, objectType, objectHandle, privateDataSlot, data, record_obj);
}

void ThreadSafety::PostCallRecordSetPrivateDataEXT(VkDevice device, VkObjectType objectType, uint64_t objectHandle,
                                                   VkPrivateDataSlot privateDataSlot, uint64_t data,
                                                   const RecordObject& record_obj) {
    PostCallRecordSetPrivateData(device, objectType, objectHandle, privateDataSlot, data, record_obj);
}

void ThreadSafety::PreCallRecordGetPrivateDataEXT(VkDevice device, VkObjectType objectType, uint64_t objectHandle,
                                                  VkPrivateDataSlot privateDataSlot, uint64_t* pData,
                                                  const RecordObject& record_obj) {
    PreCallRecordGetPrivateData(device, objectType, objectHandle, privateDataSlot, pData, record_obj);
}

void ThreadSafety::PostCallRecordGetPrivateDataEXT(VkDevice device, VkObjectType objectType, uint64_t objectHandle,
                                                   VkPrivateDataSlot privateDataSlot, uint64_t* pData,
                                                   const RecordObject& record_obj) {
    PostCallRecordGetPrivateData(device, objectType, objectHandle, privateDataSlot, pData, record_obj);
}

void ThreadSafety::PreCallRecordCreateCudaModuleNV(VkDevice device, const VkCudaModuleCreateInfoNV* pCreateInfo,
                                                   const VkAllocationCallbacks* pAllocator, VkCudaModuleNV* pModule,
                                                   const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordCreateCudaModuleNV(VkDevice device, const VkCudaModuleCreateInfoNV* pCreateInfo,
                                                    const VkAllocationCallbacks* pAllocator, VkCudaModuleNV* pModule,
                                                    const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pModule);
    }
}

void ThreadSafety::PreCallRecordGetCudaModuleCacheNV(VkDevice device, VkCudaModuleNV module, size_t* pCacheSize, void* pCacheData,
                                                     const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(module, record_obj.location);
}

void ThreadSafety::PostCallRecordGetCudaModuleCacheNV(VkDevice device, VkCudaModuleNV module, size_t* pCacheSize, void* pCacheData,
                                                      const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(module, record_obj.location);
}

void ThreadSafety::PreCallRecordCreateCudaFunctionNV(VkDevice device, const VkCudaFunctionCreateInfoNV* pCreateInfo,
                                                     const VkAllocationCallbacks* pAllocator, VkCudaFunctionNV* pFunction,
                                                     const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordCreateCudaFunctionNV(VkDevice device, const VkCudaFunctionCreateInfoNV* pCreateInfo,
                                                      const VkAllocationCallbacks* pAllocator, VkCudaFunctionNV* pFunction,
                                                      const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pFunction);
    }
}

void ThreadSafety::PreCallRecordDestroyCudaModuleNV(VkDevice device, VkCudaModuleNV module, const VkAllocationCallbacks* pAllocator,
                                                    const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(module, record_obj.location);
}

void ThreadSafety::PostCallRecordDestroyCudaModuleNV(VkDevice device, VkCudaModuleNV module,
                                                     const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(module, record_obj.location);
}

void ThreadSafety::PreCallRecordDestroyCudaFunctionNV(VkDevice device, VkCudaFunctionNV function,
                                                      const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(function, record_obj.location);
}

void ThreadSafety::PostCallRecordDestroyCudaFunctionNV(VkDevice device, VkCudaFunctionNV function,
                                                       const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(function, record_obj.location);
}

void ThreadSafety::PreCallRecordCmdCudaLaunchKernelNV(VkCommandBuffer commandBuffer, const VkCudaLaunchInfoNV* pLaunchInfo,
                                                      const RecordObject& record_obj) {
    StartReadObject(commandBuffer, record_obj.location);
}

void ThreadSafety::PostCallRecordCmdCudaLaunchKernelNV(VkCommandBuffer commandBuffer, const VkCudaLaunchInfoNV* pLaunchInfo,
                                                       const RecordObject& record_obj) {
    FinishReadObject(commandBuffer, record_obj.location);
}

#ifdef VK_USE_PLATFORM_METAL_EXT
void ThreadSafety::PreCallRecordExportMetalObjectsEXT(VkDevice device, VkExportMetalObjectsInfoEXT* pMetalObjectsInfo,
                                                      const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordExportMetalObjectsEXT(VkDevice device, VkExportMetalObjectsInfoEXT* pMetalObjectsInfo,
                                                       const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

#endif  // VK_USE_PLATFORM_METAL_EXT
void ThreadSafety::PreCallRecordGetDescriptorSetLayoutSizeEXT(VkDevice device, VkDescriptorSetLayout layout,
                                                              VkDeviceSize* pLayoutSizeInBytes, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(layout, record_obj.location);
}

void ThreadSafety::PostCallRecordGetDescriptorSetLayoutSizeEXT(VkDevice device, VkDescriptorSetLayout layout,
                                                               VkDeviceSize* pLayoutSizeInBytes, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(layout, record_obj.location);
}

void ThreadSafety::PreCallRecordGetDescriptorSetLayoutBindingOffsetEXT(VkDevice device, VkDescriptorSetLayout layout,
                                                                       uint32_t binding, VkDeviceSize* pOffset,
                                                                       const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(layout, record_obj.location);
}

void ThreadSafety::PostCallRecordGetDescriptorSetLayoutBindingOffsetEXT(VkDevice device, VkDescriptorSetLayout layout,
                                                                        uint32_t binding, VkDeviceSize* pOffset,
                                                                        const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(layout, record_obj.location);
}

void ThreadSafety::PreCallRecordGetDescriptorEXT(VkDevice device, const VkDescriptorGetInfoEXT* pDescriptorInfo, size_t dataSize,
                                                 void* pDescriptor, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordGetDescriptorEXT(VkDevice device, const VkDescriptorGetInfoEXT* pDescriptorInfo, size_t dataSize,
                                                  void* pDescriptor, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PreCallRecordCmdBindDescriptorBuffersEXT(VkCommandBuffer commandBuffer, uint32_t bufferCount,
                                                            const VkDescriptorBufferBindingInfoEXT* pBindingInfos,
                                                            const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdBindDescriptorBuffersEXT(VkCommandBuffer commandBuffer, uint32_t bufferCount,
                                                             const VkDescriptorBufferBindingInfoEXT* pBindingInfos,
                                                             const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetDescriptorBufferOffsetsEXT(VkCommandBuffer commandBuffer,
                                                                 VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout,
                                                                 uint32_t firstSet, uint32_t setCount,
                                                                 const uint32_t* pBufferIndices, const VkDeviceSize* pOffsets,
                                                                 const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    StartReadObject(layout, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetDescriptorBufferOffsetsEXT(VkCommandBuffer commandBuffer,
                                                                  VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout,
                                                                  uint32_t firstSet, uint32_t setCount,
                                                                  const uint32_t* pBufferIndices, const VkDeviceSize* pOffsets,
                                                                  const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    FinishReadObject(layout, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdBindDescriptorBufferEmbeddedSamplersEXT(VkCommandBuffer commandBuffer,
                                                                           VkPipelineBindPoint pipelineBindPoint,
                                                                           VkPipelineLayout layout, uint32_t set,
                                                                           const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    StartReadObject(layout, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdBindDescriptorBufferEmbeddedSamplersEXT(VkCommandBuffer commandBuffer,
                                                                            VkPipelineBindPoint pipelineBindPoint,
                                                                            VkPipelineLayout layout, uint32_t set,
                                                                            const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    FinishReadObject(layout, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordGetBufferOpaqueCaptureDescriptorDataEXT(VkDevice device,
                                                                        const VkBufferCaptureDescriptorDataInfoEXT* pInfo,
                                                                        void* pData, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordGetBufferOpaqueCaptureDescriptorDataEXT(VkDevice device,
                                                                         const VkBufferCaptureDescriptorDataInfoEXT* pInfo,
                                                                         void* pData, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PreCallRecordGetImageOpaqueCaptureDescriptorDataEXT(VkDevice device,
                                                                       const VkImageCaptureDescriptorDataInfoEXT* pInfo,
                                                                       void* pData, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordGetImageOpaqueCaptureDescriptorDataEXT(VkDevice device,
                                                                        const VkImageCaptureDescriptorDataInfoEXT* pInfo,
                                                                        void* pData, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PreCallRecordGetImageViewOpaqueCaptureDescriptorDataEXT(VkDevice device,
                                                                           const VkImageViewCaptureDescriptorDataInfoEXT* pInfo,
                                                                           void* pData, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordGetImageViewOpaqueCaptureDescriptorDataEXT(VkDevice device,
                                                                            const VkImageViewCaptureDescriptorDataInfoEXT* pInfo,
                                                                            void* pData, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PreCallRecordGetSamplerOpaqueCaptureDescriptorDataEXT(VkDevice device,
                                                                         const VkSamplerCaptureDescriptorDataInfoEXT* pInfo,
                                                                         void* pData, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordGetSamplerOpaqueCaptureDescriptorDataEXT(VkDevice device,
                                                                          const VkSamplerCaptureDescriptorDataInfoEXT* pInfo,
                                                                          void* pData, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PreCallRecordGetAccelerationStructureOpaqueCaptureDescriptorDataEXT(
    VkDevice device, const VkAccelerationStructureCaptureDescriptorDataInfoEXT* pInfo, void* pData,
    const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordGetAccelerationStructureOpaqueCaptureDescriptorDataEXT(
    VkDevice device, const VkAccelerationStructureCaptureDescriptorDataInfoEXT* pInfo, void* pData,
    const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PreCallRecordCmdSetFragmentShadingRateEnumNV(VkCommandBuffer commandBuffer, VkFragmentShadingRateNV shadingRate,
                                                                const VkFragmentShadingRateCombinerOpKHR combinerOps[2],
                                                                const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetFragmentShadingRateEnumNV(VkCommandBuffer commandBuffer, VkFragmentShadingRateNV shadingRate,
                                                                 const VkFragmentShadingRateCombinerOpKHR combinerOps[2],
                                                                 const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordGetDeviceFaultInfoEXT(VkDevice device, VkDeviceFaultCountsEXT* pFaultCounts,
                                                      VkDeviceFaultInfoEXT* pFaultInfo, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordGetDeviceFaultInfoEXT(VkDevice device, VkDeviceFaultCountsEXT* pFaultCounts,
                                                       VkDeviceFaultInfoEXT* pFaultInfo, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

#ifdef VK_USE_PLATFORM_WIN32_KHR
void ThreadSafety::PreCallRecordAcquireWinrtDisplayNV(VkPhysicalDevice physicalDevice, VkDisplayKHR display,
                                                      const RecordObject& record_obj) {
    StartReadObjectParentInstance(display, record_obj.location);
}

void ThreadSafety::PostCallRecordAcquireWinrtDisplayNV(VkPhysicalDevice physicalDevice, VkDisplayKHR display,
                                                       const RecordObject& record_obj) {
    FinishReadObjectParentInstance(display, record_obj.location);
}

#endif  // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_DIRECTFB_EXT
void ThreadSafety::PreCallRecordCreateDirectFBSurfaceEXT(VkInstance instance, const VkDirectFBSurfaceCreateInfoEXT* pCreateInfo,
                                                         const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                         const RecordObject& record_obj) {
    StartReadObjectParentInstance(instance, record_obj.location);
}

void ThreadSafety::PostCallRecordCreateDirectFBSurfaceEXT(VkInstance instance, const VkDirectFBSurfaceCreateInfoEXT* pCreateInfo,
                                                          const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                          const RecordObject& record_obj) {
    FinishReadObjectParentInstance(instance, record_obj.location);
    if (record_obj.result == VK_SUCCESS) {
        CreateObjectParentInstance(*pSurface);
    }
}

#endif  // VK_USE_PLATFORM_DIRECTFB_EXT
void ThreadSafety::PreCallRecordCmdSetVertexInputEXT(VkCommandBuffer commandBuffer, uint32_t vertexBindingDescriptionCount,
                                                     const VkVertexInputBindingDescription2EXT* pVertexBindingDescriptions,
                                                     uint32_t vertexAttributeDescriptionCount,
                                                     const VkVertexInputAttributeDescription2EXT* pVertexAttributeDescriptions,
                                                     const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetVertexInputEXT(VkCommandBuffer commandBuffer, uint32_t vertexBindingDescriptionCount,
                                                      const VkVertexInputBindingDescription2EXT* pVertexBindingDescriptions,
                                                      uint32_t vertexAttributeDescriptionCount,
                                                      const VkVertexInputAttributeDescription2EXT* pVertexAttributeDescriptions,
                                                      const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

#ifdef VK_USE_PLATFORM_FUCHSIA
void ThreadSafety::PreCallRecordGetMemoryZirconHandleFUCHSIA(VkDevice device,
                                                             const VkMemoryGetZirconHandleInfoFUCHSIA* pGetZirconHandleInfo,
                                                             zx_handle_t* pZirconHandle, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordGetMemoryZirconHandleFUCHSIA(VkDevice device,
                                                              const VkMemoryGetZirconHandleInfoFUCHSIA* pGetZirconHandleInfo,
                                                              zx_handle_t* pZirconHandle, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PreCallRecordGetMemoryZirconHandlePropertiesFUCHSIA(
    VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType, zx_handle_t zirconHandle,
    VkMemoryZirconHandlePropertiesFUCHSIA* pMemoryZirconHandleProperties, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordGetMemoryZirconHandlePropertiesFUCHSIA(
    VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType, zx_handle_t zirconHandle,
    VkMemoryZirconHandlePropertiesFUCHSIA* pMemoryZirconHandleProperties, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PreCallRecordImportSemaphoreZirconHandleFUCHSIA(
    VkDevice device, const VkImportSemaphoreZirconHandleInfoFUCHSIA* pImportSemaphoreZirconHandleInfo,
    const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordImportSemaphoreZirconHandleFUCHSIA(
    VkDevice device, const VkImportSemaphoreZirconHandleInfoFUCHSIA* pImportSemaphoreZirconHandleInfo,
    const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PreCallRecordGetSemaphoreZirconHandleFUCHSIA(VkDevice device,
                                                                const VkSemaphoreGetZirconHandleInfoFUCHSIA* pGetZirconHandleInfo,
                                                                zx_handle_t* pZirconHandle, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordGetSemaphoreZirconHandleFUCHSIA(VkDevice device,
                                                                 const VkSemaphoreGetZirconHandleInfoFUCHSIA* pGetZirconHandleInfo,
                                                                 zx_handle_t* pZirconHandle, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PreCallRecordCreateBufferCollectionFUCHSIA(VkDevice device,
                                                              const VkBufferCollectionCreateInfoFUCHSIA* pCreateInfo,
                                                              const VkAllocationCallbacks* pAllocator,
                                                              VkBufferCollectionFUCHSIA* pCollection,
                                                              const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordCreateBufferCollectionFUCHSIA(VkDevice device,
                                                               const VkBufferCollectionCreateInfoFUCHSIA* pCreateInfo,
                                                               const VkAllocationCallbacks* pAllocator,
                                                               VkBufferCollectionFUCHSIA* pCollection,
                                                               const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pCollection);
    }
}

void ThreadSafety::PreCallRecordSetBufferCollectionImageConstraintsFUCHSIA(
    VkDevice device, VkBufferCollectionFUCHSIA collection, const VkImageConstraintsInfoFUCHSIA* pImageConstraintsInfo,
    const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(collection, record_obj.location);
}

void ThreadSafety::PostCallRecordSetBufferCollectionImageConstraintsFUCHSIA(
    VkDevice device, VkBufferCollectionFUCHSIA collection, const VkImageConstraintsInfoFUCHSIA* pImageConstraintsInfo,
    const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(collection, record_obj.location);
}

void ThreadSafety::PreCallRecordSetBufferCollectionBufferConstraintsFUCHSIA(
    VkDevice device, VkBufferCollectionFUCHSIA collection, const VkBufferConstraintsInfoFUCHSIA* pBufferConstraintsInfo,
    const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(collection, record_obj.location);
}

void ThreadSafety::PostCallRecordSetBufferCollectionBufferConstraintsFUCHSIA(
    VkDevice device, VkBufferCollectionFUCHSIA collection, const VkBufferConstraintsInfoFUCHSIA* pBufferConstraintsInfo,
    const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(collection, record_obj.location);
}

void ThreadSafety::PreCallRecordDestroyBufferCollectionFUCHSIA(VkDevice device, VkBufferCollectionFUCHSIA collection,
                                                               const VkAllocationCallbacks* pAllocator,
                                                               const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(collection, record_obj.location);
}

void ThreadSafety::PostCallRecordDestroyBufferCollectionFUCHSIA(VkDevice device, VkBufferCollectionFUCHSIA collection,
                                                                const VkAllocationCallbacks* pAllocator,
                                                                const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(collection, record_obj.location);
}

void ThreadSafety::PreCallRecordGetBufferCollectionPropertiesFUCHSIA(VkDevice device, VkBufferCollectionFUCHSIA collection,
                                                                     VkBufferCollectionPropertiesFUCHSIA* pProperties,
                                                                     const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(collection, record_obj.location);
}

void ThreadSafety::PostCallRecordGetBufferCollectionPropertiesFUCHSIA(VkDevice device, VkBufferCollectionFUCHSIA collection,
                                                                      VkBufferCollectionPropertiesFUCHSIA* pProperties,
                                                                      const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(collection, record_obj.location);
}

#endif  // VK_USE_PLATFORM_FUCHSIA
void ThreadSafety::PreCallRecordGetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI(VkDevice device, VkRenderPass renderpass,
                                                                              VkExtent2D* pMaxWorkgroupSize,
                                                                              const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(renderpass, record_obj.location);
}

void ThreadSafety::PostCallRecordGetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI(VkDevice device, VkRenderPass renderpass,
                                                                               VkExtent2D* pMaxWorkgroupSize,
                                                                               const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(renderpass, record_obj.location);
}

void ThreadSafety::PreCallRecordCmdSubpassShadingHUAWEI(VkCommandBuffer commandBuffer, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSubpassShadingHUAWEI(VkCommandBuffer commandBuffer, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdBindInvocationMaskHUAWEI(VkCommandBuffer commandBuffer, VkImageView imageView,
                                                            VkImageLayout imageLayout, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    StartReadObject(imageView, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdBindInvocationMaskHUAWEI(VkCommandBuffer commandBuffer, VkImageView imageView,
                                                             VkImageLayout imageLayout, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    FinishReadObject(imageView, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordGetMemoryRemoteAddressNV(VkDevice device,
                                                         const VkMemoryGetRemoteAddressInfoNV* pMemoryGetRemoteAddressInfo,
                                                         VkRemoteAddressNV* pAddress, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordGetMemoryRemoteAddressNV(VkDevice device,
                                                          const VkMemoryGetRemoteAddressInfoNV* pMemoryGetRemoteAddressInfo,
                                                          VkRemoteAddressNV* pAddress, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PreCallRecordGetPipelinePropertiesEXT(VkDevice device, const VkPipelineInfoEXT* pPipelineInfo,
                                                         VkBaseOutStructure* pPipelineProperties, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordGetPipelinePropertiesEXT(VkDevice device, const VkPipelineInfoEXT* pPipelineInfo,
                                                          VkBaseOutStructure* pPipelineProperties, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PreCallRecordCmdSetPatchControlPointsEXT(VkCommandBuffer commandBuffer, uint32_t patchControlPoints,
                                                            const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetPatchControlPointsEXT(VkCommandBuffer commandBuffer, uint32_t patchControlPoints,
                                                             const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetRasterizerDiscardEnableEXT(VkCommandBuffer commandBuffer, VkBool32 rasterizerDiscardEnable,
                                                                 const RecordObject& record_obj) {
    PreCallRecordCmdSetRasterizerDiscardEnable(commandBuffer, rasterizerDiscardEnable, record_obj);
}

void ThreadSafety::PostCallRecordCmdSetRasterizerDiscardEnableEXT(VkCommandBuffer commandBuffer, VkBool32 rasterizerDiscardEnable,
                                                                  const RecordObject& record_obj) {
    PostCallRecordCmdSetRasterizerDiscardEnable(commandBuffer, rasterizerDiscardEnable, record_obj);
}

void ThreadSafety::PreCallRecordCmdSetDepthBiasEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthBiasEnable,
                                                         const RecordObject& record_obj) {
    PreCallRecordCmdSetDepthBiasEnable(commandBuffer, depthBiasEnable, record_obj);
}

void ThreadSafety::PostCallRecordCmdSetDepthBiasEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthBiasEnable,
                                                          const RecordObject& record_obj) {
    PostCallRecordCmdSetDepthBiasEnable(commandBuffer, depthBiasEnable, record_obj);
}

void ThreadSafety::PreCallRecordCmdSetLogicOpEXT(VkCommandBuffer commandBuffer, VkLogicOp logicOp, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetLogicOpEXT(VkCommandBuffer commandBuffer, VkLogicOp logicOp,
                                                  const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetPrimitiveRestartEnableEXT(VkCommandBuffer commandBuffer, VkBool32 primitiveRestartEnable,
                                                                const RecordObject& record_obj) {
    PreCallRecordCmdSetPrimitiveRestartEnable(commandBuffer, primitiveRestartEnable, record_obj);
}

void ThreadSafety::PostCallRecordCmdSetPrimitiveRestartEnableEXT(VkCommandBuffer commandBuffer, VkBool32 primitiveRestartEnable,
                                                                 const RecordObject& record_obj) {
    PostCallRecordCmdSetPrimitiveRestartEnable(commandBuffer, primitiveRestartEnable, record_obj);
}

#ifdef VK_USE_PLATFORM_SCREEN_QNX
void ThreadSafety::PreCallRecordCreateScreenSurfaceQNX(VkInstance instance, const VkScreenSurfaceCreateInfoQNX* pCreateInfo,
                                                       const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                       const RecordObject& record_obj) {
    StartReadObjectParentInstance(instance, record_obj.location);
}

void ThreadSafety::PostCallRecordCreateScreenSurfaceQNX(VkInstance instance, const VkScreenSurfaceCreateInfoQNX* pCreateInfo,
                                                        const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                        const RecordObject& record_obj) {
    FinishReadObjectParentInstance(instance, record_obj.location);
    if (record_obj.result == VK_SUCCESS) {
        CreateObjectParentInstance(*pSurface);
    }
}

#endif  // VK_USE_PLATFORM_SCREEN_QNX
void ThreadSafety::PreCallRecordCmdSetColorWriteEnableEXT(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
                                                          const VkBool32* pColorWriteEnables, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetColorWriteEnableEXT(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
                                                           const VkBool32* pColorWriteEnables, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdDrawMultiEXT(VkCommandBuffer commandBuffer, uint32_t drawCount,
                                                const VkMultiDrawInfoEXT* pVertexInfo, uint32_t instanceCount,
                                                uint32_t firstInstance, uint32_t stride, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdDrawMultiEXT(VkCommandBuffer commandBuffer, uint32_t drawCount,
                                                 const VkMultiDrawInfoEXT* pVertexInfo, uint32_t instanceCount,
                                                 uint32_t firstInstance, uint32_t stride, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdDrawMultiIndexedEXT(VkCommandBuffer commandBuffer, uint32_t drawCount,
                                                       const VkMultiDrawIndexedInfoEXT* pIndexInfo, uint32_t instanceCount,
                                                       uint32_t firstInstance, uint32_t stride, const int32_t* pVertexOffset,
                                                       const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdDrawMultiIndexedEXT(VkCommandBuffer commandBuffer, uint32_t drawCount,
                                                        const VkMultiDrawIndexedInfoEXT* pIndexInfo, uint32_t instanceCount,
                                                        uint32_t firstInstance, uint32_t stride, const int32_t* pVertexOffset,
                                                        const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCreateMicromapEXT(VkDevice device, const VkMicromapCreateInfoEXT* pCreateInfo,
                                                  const VkAllocationCallbacks* pAllocator, VkMicromapEXT* pMicromap,
                                                  const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordCreateMicromapEXT(VkDevice device, const VkMicromapCreateInfoEXT* pCreateInfo,
                                                   const VkAllocationCallbacks* pAllocator, VkMicromapEXT* pMicromap,
                                                   const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pMicromap);
    }
}

void ThreadSafety::PreCallRecordDestroyMicromapEXT(VkDevice device, VkMicromapEXT micromap, const VkAllocationCallbacks* pAllocator,
                                                   const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartWriteObject(micromap, record_obj.location);
    // Host access to micromap must be externally synchronized
}

void ThreadSafety::PostCallRecordDestroyMicromapEXT(VkDevice device, VkMicromapEXT micromap,
                                                    const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishWriteObject(micromap, record_obj.location);
    DestroyObject(micromap);
    // Host access to micromap must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdBuildMicromapsEXT(VkCommandBuffer commandBuffer, uint32_t infoCount,
                                                     const VkMicromapBuildInfoEXT* pInfos, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdBuildMicromapsEXT(VkCommandBuffer commandBuffer, uint32_t infoCount,
                                                      const VkMicromapBuildInfoEXT* pInfos, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordBuildMicromapsEXT(VkDevice device, VkDeferredOperationKHR deferredOperation, uint32_t infoCount,
                                                  const VkMicromapBuildInfoEXT* pInfos, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(deferredOperation, record_obj.location);
}

void ThreadSafety::PostCallRecordBuildMicromapsEXT(VkDevice device, VkDeferredOperationKHR deferredOperation, uint32_t infoCount,
                                                   const VkMicromapBuildInfoEXT* pInfos, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(deferredOperation, record_obj.location);
}

void ThreadSafety::PreCallRecordCopyMicromapEXT(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                const VkCopyMicromapInfoEXT* pInfo, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(deferredOperation, record_obj.location);
}

void ThreadSafety::PostCallRecordCopyMicromapEXT(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                 const VkCopyMicromapInfoEXT* pInfo, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(deferredOperation, record_obj.location);
}

void ThreadSafety::PreCallRecordCopyMicromapToMemoryEXT(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                        const VkCopyMicromapToMemoryInfoEXT* pInfo,
                                                        const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(deferredOperation, record_obj.location);
}

void ThreadSafety::PostCallRecordCopyMicromapToMemoryEXT(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                         const VkCopyMicromapToMemoryInfoEXT* pInfo,
                                                         const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(deferredOperation, record_obj.location);
}

void ThreadSafety::PreCallRecordCopyMemoryToMicromapEXT(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                        const VkCopyMemoryToMicromapInfoEXT* pInfo,
                                                        const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(deferredOperation, record_obj.location);
}

void ThreadSafety::PostCallRecordCopyMemoryToMicromapEXT(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                         const VkCopyMemoryToMicromapInfoEXT* pInfo,
                                                         const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(deferredOperation, record_obj.location);
}

void ThreadSafety::PreCallRecordWriteMicromapsPropertiesEXT(VkDevice device, uint32_t micromapCount,
                                                            const VkMicromapEXT* pMicromaps, VkQueryType queryType, size_t dataSize,
                                                            void* pData, size_t stride, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);

    if (pMicromaps) {
        for (uint32_t index = 0; index < micromapCount; index++) {
            StartReadObject(pMicromaps[index], record_obj.location);
        }
    }
}

void ThreadSafety::PostCallRecordWriteMicromapsPropertiesEXT(VkDevice device, uint32_t micromapCount,
                                                             const VkMicromapEXT* pMicromaps, VkQueryType queryType,
                                                             size_t dataSize, void* pData, size_t stride,
                                                             const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);

    if (pMicromaps) {
        for (uint32_t index = 0; index < micromapCount; index++) {
            FinishReadObject(pMicromaps[index], record_obj.location);
        }
    }
}

void ThreadSafety::PreCallRecordCmdCopyMicromapEXT(VkCommandBuffer commandBuffer, const VkCopyMicromapInfoEXT* pInfo,
                                                   const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdCopyMicromapEXT(VkCommandBuffer commandBuffer, const VkCopyMicromapInfoEXT* pInfo,
                                                    const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdCopyMicromapToMemoryEXT(VkCommandBuffer commandBuffer,
                                                           const VkCopyMicromapToMemoryInfoEXT* pInfo,
                                                           const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdCopyMicromapToMemoryEXT(VkCommandBuffer commandBuffer,
                                                            const VkCopyMicromapToMemoryInfoEXT* pInfo,
                                                            const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdCopyMemoryToMicromapEXT(VkCommandBuffer commandBuffer,
                                                           const VkCopyMemoryToMicromapInfoEXT* pInfo,
                                                           const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdCopyMemoryToMicromapEXT(VkCommandBuffer commandBuffer,
                                                            const VkCopyMemoryToMicromapInfoEXT* pInfo,
                                                            const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdWriteMicromapsPropertiesEXT(VkCommandBuffer commandBuffer, uint32_t micromapCount,
                                                               const VkMicromapEXT* pMicromaps, VkQueryType queryType,
                                                               VkQueryPool queryPool, uint32_t firstQuery,
                                                               const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);

    if (pMicromaps) {
        for (uint32_t index = 0; index < micromapCount; index++) {
            StartReadObject(pMicromaps[index], record_obj.location);
        }
    }
    StartReadObject(queryPool, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdWriteMicromapsPropertiesEXT(VkCommandBuffer commandBuffer, uint32_t micromapCount,
                                                                const VkMicromapEXT* pMicromaps, VkQueryType queryType,
                                                                VkQueryPool queryPool, uint32_t firstQuery,
                                                                const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);

    if (pMicromaps) {
        for (uint32_t index = 0; index < micromapCount; index++) {
            FinishReadObject(pMicromaps[index], record_obj.location);
        }
    }
    FinishReadObject(queryPool, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordGetDeviceMicromapCompatibilityEXT(VkDevice device, const VkMicromapVersionInfoEXT* pVersionInfo,
                                                                  VkAccelerationStructureCompatibilityKHR* pCompatibility,
                                                                  const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordGetDeviceMicromapCompatibilityEXT(VkDevice device, const VkMicromapVersionInfoEXT* pVersionInfo,
                                                                   VkAccelerationStructureCompatibilityKHR* pCompatibility,
                                                                   const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PreCallRecordGetMicromapBuildSizesEXT(VkDevice device, VkAccelerationStructureBuildTypeKHR buildType,
                                                         const VkMicromapBuildInfoEXT* pBuildInfo,
                                                         VkMicromapBuildSizesInfoEXT* pSizeInfo, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordGetMicromapBuildSizesEXT(VkDevice device, VkAccelerationStructureBuildTypeKHR buildType,
                                                          const VkMicromapBuildInfoEXT* pBuildInfo,
                                                          VkMicromapBuildSizesInfoEXT* pSizeInfo, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PreCallRecordCmdDrawClusterHUAWEI(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY,
                                                     uint32_t groupCountZ, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdDrawClusterHUAWEI(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY,
                                                      uint32_t groupCountZ, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdDrawClusterIndirectHUAWEI(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                             const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    StartReadObject(buffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdDrawClusterIndirectHUAWEI(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                              const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    FinishReadObject(buffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordSetDeviceMemoryPriorityEXT(VkDevice device, VkDeviceMemory memory, float priority,
                                                           const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(memory, record_obj.location);
}

void ThreadSafety::PostCallRecordSetDeviceMemoryPriorityEXT(VkDevice device, VkDeviceMemory memory, float priority,
                                                            const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(memory, record_obj.location);
}

void ThreadSafety::PreCallRecordGetDescriptorSetLayoutHostMappingInfoVALVE(
    VkDevice device, const VkDescriptorSetBindingReferenceVALVE* pBindingReference,
    VkDescriptorSetLayoutHostMappingInfoVALVE* pHostMapping, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordGetDescriptorSetLayoutHostMappingInfoVALVE(
    VkDevice device, const VkDescriptorSetBindingReferenceVALVE* pBindingReference,
    VkDescriptorSetLayoutHostMappingInfoVALVE* pHostMapping, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PreCallRecordGetDescriptorSetHostMappingVALVE(VkDevice device, VkDescriptorSet descriptorSet, void** ppData,
                                                                 const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(descriptorSet, record_obj.location);
}

void ThreadSafety::PostCallRecordGetDescriptorSetHostMappingVALVE(VkDevice device, VkDescriptorSet descriptorSet, void** ppData,
                                                                  const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(descriptorSet, record_obj.location);
}

void ThreadSafety::PreCallRecordCmdCopyMemoryIndirectNV(VkCommandBuffer commandBuffer, VkDeviceAddress copyBufferAddress,
                                                        uint32_t copyCount, uint32_t stride, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdCopyMemoryIndirectNV(VkCommandBuffer commandBuffer, VkDeviceAddress copyBufferAddress,
                                                         uint32_t copyCount, uint32_t stride, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdCopyMemoryToImageIndirectNV(VkCommandBuffer commandBuffer, VkDeviceAddress copyBufferAddress,
                                                               uint32_t copyCount, uint32_t stride, VkImage dstImage,
                                                               VkImageLayout dstImageLayout,
                                                               const VkImageSubresourceLayers* pImageSubresources,
                                                               const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    StartReadObject(dstImage, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdCopyMemoryToImageIndirectNV(VkCommandBuffer commandBuffer, VkDeviceAddress copyBufferAddress,
                                                                uint32_t copyCount, uint32_t stride, VkImage dstImage,
                                                                VkImageLayout dstImageLayout,
                                                                const VkImageSubresourceLayers* pImageSubresources,
                                                                const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    FinishReadObject(dstImage, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdDecompressMemoryNV(VkCommandBuffer commandBuffer, uint32_t decompressRegionCount,
                                                      const VkDecompressMemoryRegionNV* pDecompressMemoryRegions,
                                                      const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdDecompressMemoryNV(VkCommandBuffer commandBuffer, uint32_t decompressRegionCount,
                                                       const VkDecompressMemoryRegionNV* pDecompressMemoryRegions,
                                                       const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdDecompressMemoryIndirectCountNV(VkCommandBuffer commandBuffer,
                                                                   VkDeviceAddress indirectCommandsAddress,
                                                                   VkDeviceAddress indirectCommandsCountAddress, uint32_t stride,
                                                                   const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdDecompressMemoryIndirectCountNV(VkCommandBuffer commandBuffer,
                                                                    VkDeviceAddress indirectCommandsAddress,
                                                                    VkDeviceAddress indirectCommandsCountAddress, uint32_t stride,
                                                                    const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordGetPipelineIndirectMemoryRequirementsNV(VkDevice device,
                                                                        const VkComputePipelineCreateInfo* pCreateInfo,
                                                                        VkMemoryRequirements2* pMemoryRequirements,
                                                                        const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordGetPipelineIndirectMemoryRequirementsNV(VkDevice device,
                                                                         const VkComputePipelineCreateInfo* pCreateInfo,
                                                                         VkMemoryRequirements2* pMemoryRequirements,
                                                                         const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PreCallRecordCmdUpdatePipelineIndirectBufferNV(VkCommandBuffer commandBuffer,
                                                                  VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline,
                                                                  const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    StartReadObject(pipeline, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdUpdatePipelineIndirectBufferNV(VkCommandBuffer commandBuffer,
                                                                   VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline,
                                                                   const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    FinishReadObject(pipeline, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordGetPipelineIndirectDeviceAddressNV(VkDevice device,
                                                                   const VkPipelineIndirectDeviceAddressInfoNV* pInfo,
                                                                   const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordGetPipelineIndirectDeviceAddressNV(VkDevice device,
                                                                    const VkPipelineIndirectDeviceAddressInfoNV* pInfo,
                                                                    const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PreCallRecordCmdSetTessellationDomainOriginEXT(VkCommandBuffer commandBuffer,
                                                                  VkTessellationDomainOrigin domainOrigin,
                                                                  const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetTessellationDomainOriginEXT(VkCommandBuffer commandBuffer,
                                                                   VkTessellationDomainOrigin domainOrigin,
                                                                   const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetDepthClampEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthClampEnable,
                                                          const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetDepthClampEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthClampEnable,
                                                           const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetPolygonModeEXT(VkCommandBuffer commandBuffer, VkPolygonMode polygonMode,
                                                     const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetPolygonModeEXT(VkCommandBuffer commandBuffer, VkPolygonMode polygonMode,
                                                      const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetRasterizationSamplesEXT(VkCommandBuffer commandBuffer,
                                                              VkSampleCountFlagBits rasterizationSamples,
                                                              const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetRasterizationSamplesEXT(VkCommandBuffer commandBuffer,
                                                               VkSampleCountFlagBits rasterizationSamples,
                                                               const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetSampleMaskEXT(VkCommandBuffer commandBuffer, VkSampleCountFlagBits samples,
                                                    const VkSampleMask* pSampleMask, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetSampleMaskEXT(VkCommandBuffer commandBuffer, VkSampleCountFlagBits samples,
                                                     const VkSampleMask* pSampleMask, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetAlphaToCoverageEnableEXT(VkCommandBuffer commandBuffer, VkBool32 alphaToCoverageEnable,
                                                               const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetAlphaToCoverageEnableEXT(VkCommandBuffer commandBuffer, VkBool32 alphaToCoverageEnable,
                                                                const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetAlphaToOneEnableEXT(VkCommandBuffer commandBuffer, VkBool32 alphaToOneEnable,
                                                          const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetAlphaToOneEnableEXT(VkCommandBuffer commandBuffer, VkBool32 alphaToOneEnable,
                                                           const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetLogicOpEnableEXT(VkCommandBuffer commandBuffer, VkBool32 logicOpEnable,
                                                       const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetLogicOpEnableEXT(VkCommandBuffer commandBuffer, VkBool32 logicOpEnable,
                                                        const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetColorBlendEnableEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment,
                                                          uint32_t attachmentCount, const VkBool32* pColorBlendEnables,
                                                          const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetColorBlendEnableEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment,
                                                           uint32_t attachmentCount, const VkBool32* pColorBlendEnables,
                                                           const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetColorBlendEquationEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment,
                                                            uint32_t attachmentCount,
                                                            const VkColorBlendEquationEXT* pColorBlendEquations,
                                                            const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetColorBlendEquationEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment,
                                                             uint32_t attachmentCount,
                                                             const VkColorBlendEquationEXT* pColorBlendEquations,
                                                             const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetColorWriteMaskEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment,
                                                        uint32_t attachmentCount, const VkColorComponentFlags* pColorWriteMasks,
                                                        const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetColorWriteMaskEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment,
                                                         uint32_t attachmentCount, const VkColorComponentFlags* pColorWriteMasks,
                                                         const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetRasterizationStreamEXT(VkCommandBuffer commandBuffer, uint32_t rasterizationStream,
                                                             const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetRasterizationStreamEXT(VkCommandBuffer commandBuffer, uint32_t rasterizationStream,
                                                              const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetConservativeRasterizationModeEXT(
    VkCommandBuffer commandBuffer, VkConservativeRasterizationModeEXT conservativeRasterizationMode,
    const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetConservativeRasterizationModeEXT(
    VkCommandBuffer commandBuffer, VkConservativeRasterizationModeEXT conservativeRasterizationMode,
    const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetExtraPrimitiveOverestimationSizeEXT(VkCommandBuffer commandBuffer,
                                                                          float extraPrimitiveOverestimationSize,
                                                                          const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetExtraPrimitiveOverestimationSizeEXT(VkCommandBuffer commandBuffer,
                                                                           float extraPrimitiveOverestimationSize,
                                                                           const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetDepthClipEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthClipEnable,
                                                         const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetDepthClipEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthClipEnable,
                                                          const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetSampleLocationsEnableEXT(VkCommandBuffer commandBuffer, VkBool32 sampleLocationsEnable,
                                                               const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetSampleLocationsEnableEXT(VkCommandBuffer commandBuffer, VkBool32 sampleLocationsEnable,
                                                                const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetColorBlendAdvancedEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment,
                                                            uint32_t attachmentCount,
                                                            const VkColorBlendAdvancedEXT* pColorBlendAdvanced,
                                                            const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetColorBlendAdvancedEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment,
                                                             uint32_t attachmentCount,
                                                             const VkColorBlendAdvancedEXT* pColorBlendAdvanced,
                                                             const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetProvokingVertexModeEXT(VkCommandBuffer commandBuffer,
                                                             VkProvokingVertexModeEXT provokingVertexMode,
                                                             const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetProvokingVertexModeEXT(VkCommandBuffer commandBuffer,
                                                              VkProvokingVertexModeEXT provokingVertexMode,
                                                              const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetLineRasterizationModeEXT(VkCommandBuffer commandBuffer,
                                                               VkLineRasterizationModeEXT lineRasterizationMode,
                                                               const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetLineRasterizationModeEXT(VkCommandBuffer commandBuffer,
                                                                VkLineRasterizationModeEXT lineRasterizationMode,
                                                                const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetLineStippleEnableEXT(VkCommandBuffer commandBuffer, VkBool32 stippledLineEnable,
                                                           const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetLineStippleEnableEXT(VkCommandBuffer commandBuffer, VkBool32 stippledLineEnable,
                                                            const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetDepthClipNegativeOneToOneEXT(VkCommandBuffer commandBuffer, VkBool32 negativeOneToOne,
                                                                   const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetDepthClipNegativeOneToOneEXT(VkCommandBuffer commandBuffer, VkBool32 negativeOneToOne,
                                                                    const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetViewportWScalingEnableNV(VkCommandBuffer commandBuffer, VkBool32 viewportWScalingEnable,
                                                               const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetViewportWScalingEnableNV(VkCommandBuffer commandBuffer, VkBool32 viewportWScalingEnable,
                                                                const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetViewportSwizzleNV(VkCommandBuffer commandBuffer, uint32_t firstViewport,
                                                        uint32_t viewportCount, const VkViewportSwizzleNV* pViewportSwizzles,
                                                        const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetViewportSwizzleNV(VkCommandBuffer commandBuffer, uint32_t firstViewport,
                                                         uint32_t viewportCount, const VkViewportSwizzleNV* pViewportSwizzles,
                                                         const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetCoverageToColorEnableNV(VkCommandBuffer commandBuffer, VkBool32 coverageToColorEnable,
                                                              const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetCoverageToColorEnableNV(VkCommandBuffer commandBuffer, VkBool32 coverageToColorEnable,
                                                               const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetCoverageToColorLocationNV(VkCommandBuffer commandBuffer, uint32_t coverageToColorLocation,
                                                                const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetCoverageToColorLocationNV(VkCommandBuffer commandBuffer, uint32_t coverageToColorLocation,
                                                                 const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetCoverageModulationModeNV(VkCommandBuffer commandBuffer,
                                                               VkCoverageModulationModeNV coverageModulationMode,
                                                               const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetCoverageModulationModeNV(VkCommandBuffer commandBuffer,
                                                                VkCoverageModulationModeNV coverageModulationMode,
                                                                const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetCoverageModulationTableEnableNV(VkCommandBuffer commandBuffer,
                                                                      VkBool32 coverageModulationTableEnable,
                                                                      const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetCoverageModulationTableEnableNV(VkCommandBuffer commandBuffer,
                                                                       VkBool32 coverageModulationTableEnable,
                                                                       const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetCoverageModulationTableNV(VkCommandBuffer commandBuffer,
                                                                uint32_t coverageModulationTableCount,
                                                                const float* pCoverageModulationTable,
                                                                const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetCoverageModulationTableNV(VkCommandBuffer commandBuffer,
                                                                 uint32_t coverageModulationTableCount,
                                                                 const float* pCoverageModulationTable,
                                                                 const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetShadingRateImageEnableNV(VkCommandBuffer commandBuffer, VkBool32 shadingRateImageEnable,
                                                               const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetShadingRateImageEnableNV(VkCommandBuffer commandBuffer, VkBool32 shadingRateImageEnable,
                                                                const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetRepresentativeFragmentTestEnableNV(VkCommandBuffer commandBuffer,
                                                                         VkBool32 representativeFragmentTestEnable,
                                                                         const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetRepresentativeFragmentTestEnableNV(VkCommandBuffer commandBuffer,
                                                                          VkBool32 representativeFragmentTestEnable,
                                                                          const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetCoverageReductionModeNV(VkCommandBuffer commandBuffer,
                                                              VkCoverageReductionModeNV coverageReductionMode,
                                                              const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetCoverageReductionModeNV(VkCommandBuffer commandBuffer,
                                                               VkCoverageReductionModeNV coverageReductionMode,
                                                               const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordGetShaderModuleIdentifierEXT(VkDevice device, VkShaderModule shaderModule,
                                                             VkShaderModuleIdentifierEXT* pIdentifier,
                                                             const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(shaderModule, record_obj.location);
}

void ThreadSafety::PostCallRecordGetShaderModuleIdentifierEXT(VkDevice device, VkShaderModule shaderModule,
                                                              VkShaderModuleIdentifierEXT* pIdentifier,
                                                              const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(shaderModule, record_obj.location);
}

void ThreadSafety::PreCallRecordGetShaderModuleCreateInfoIdentifierEXT(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo,
                                                                       VkShaderModuleIdentifierEXT* pIdentifier,
                                                                       const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordGetShaderModuleCreateInfoIdentifierEXT(VkDevice device,
                                                                        const VkShaderModuleCreateInfo* pCreateInfo,
                                                                        VkShaderModuleIdentifierEXT* pIdentifier,
                                                                        const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PreCallRecordCreateOpticalFlowSessionNV(VkDevice device, const VkOpticalFlowSessionCreateInfoNV* pCreateInfo,
                                                           const VkAllocationCallbacks* pAllocator,
                                                           VkOpticalFlowSessionNV* pSession, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordCreateOpticalFlowSessionNV(VkDevice device, const VkOpticalFlowSessionCreateInfoNV* pCreateInfo,
                                                            const VkAllocationCallbacks* pAllocator,
                                                            VkOpticalFlowSessionNV* pSession, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pSession);
    }
}

void ThreadSafety::PreCallRecordDestroyOpticalFlowSessionNV(VkDevice device, VkOpticalFlowSessionNV session,
                                                            const VkAllocationCallbacks* pAllocator,
                                                            const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(session, record_obj.location);
}

void ThreadSafety::PostCallRecordDestroyOpticalFlowSessionNV(VkDevice device, VkOpticalFlowSessionNV session,
                                                             const VkAllocationCallbacks* pAllocator,
                                                             const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(session, record_obj.location);
}

void ThreadSafety::PreCallRecordBindOpticalFlowSessionImageNV(VkDevice device, VkOpticalFlowSessionNV session,
                                                              VkOpticalFlowSessionBindingPointNV bindingPoint, VkImageView view,
                                                              VkImageLayout layout, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(session, record_obj.location);
    StartReadObject(view, record_obj.location);
}

void ThreadSafety::PostCallRecordBindOpticalFlowSessionImageNV(VkDevice device, VkOpticalFlowSessionNV session,
                                                               VkOpticalFlowSessionBindingPointNV bindingPoint, VkImageView view,
                                                               VkImageLayout layout, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(session, record_obj.location);
    FinishReadObject(view, record_obj.location);
}

void ThreadSafety::PreCallRecordCmdOpticalFlowExecuteNV(VkCommandBuffer commandBuffer, VkOpticalFlowSessionNV session,
                                                        const VkOpticalFlowExecuteInfoNV* pExecuteInfo,
                                                        const RecordObject& record_obj) {
    StartReadObject(commandBuffer, record_obj.location);
    StartReadObject(session, record_obj.location);
}

void ThreadSafety::PostCallRecordCmdOpticalFlowExecuteNV(VkCommandBuffer commandBuffer, VkOpticalFlowSessionNV session,
                                                         const VkOpticalFlowExecuteInfoNV* pExecuteInfo,
                                                         const RecordObject& record_obj) {
    FinishReadObject(commandBuffer, record_obj.location);
    FinishReadObject(session, record_obj.location);
}

void ThreadSafety::PreCallRecordCreateShadersEXT(VkDevice device, uint32_t createInfoCount,
                                                 const VkShaderCreateInfoEXT* pCreateInfos, const VkAllocationCallbacks* pAllocator,
                                                 VkShaderEXT* pShaders, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordCreateShadersEXT(VkDevice device, uint32_t createInfoCount,
                                                  const VkShaderCreateInfoEXT* pCreateInfos,
                                                  const VkAllocationCallbacks* pAllocator, VkShaderEXT* pShaders,
                                                  const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    if (pShaders) {
        for (uint32_t index = 0; index < createInfoCount; index++) {
            if (!pShaders[index]) continue;
            CreateObject(pShaders[index]);
        }
    }
}

void ThreadSafety::PreCallRecordDestroyShaderEXT(VkDevice device, VkShaderEXT shader, const VkAllocationCallbacks* pAllocator,
                                                 const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartWriteObject(shader, record_obj.location);
    // Host access to shader must be externally synchronized
}

void ThreadSafety::PostCallRecordDestroyShaderEXT(VkDevice device, VkShaderEXT shader, const VkAllocationCallbacks* pAllocator,
                                                  const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishWriteObject(shader, record_obj.location);
    DestroyObject(shader);
    // Host access to shader must be externally synchronized
}

void ThreadSafety::PreCallRecordGetShaderBinaryDataEXT(VkDevice device, VkShaderEXT shader, size_t* pDataSize, void* pData,
                                                       const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(shader, record_obj.location);
}

void ThreadSafety::PostCallRecordGetShaderBinaryDataEXT(VkDevice device, VkShaderEXT shader, size_t* pDataSize, void* pData,
                                                        const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(shader, record_obj.location);
}

void ThreadSafety::PreCallRecordCmdBindShadersEXT(VkCommandBuffer commandBuffer, uint32_t stageCount,
                                                  const VkShaderStageFlagBits* pStages, const VkShaderEXT* pShaders,
                                                  const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);

    if (pShaders) {
        for (uint32_t index = 0; index < stageCount; index++) {
            StartReadObject(pShaders[index], record_obj.location);
        }
    }
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdBindShadersEXT(VkCommandBuffer commandBuffer, uint32_t stageCount,
                                                   const VkShaderStageFlagBits* pStages, const VkShaderEXT* pShaders,
                                                   const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);

    if (pShaders) {
        for (uint32_t index = 0; index < stageCount; index++) {
            FinishReadObject(pShaders[index], record_obj.location);
        }
    }
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordGetFramebufferTilePropertiesQCOM(VkDevice device, VkFramebuffer framebuffer,
                                                                 uint32_t* pPropertiesCount, VkTilePropertiesQCOM* pProperties,
                                                                 const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(framebuffer, record_obj.location);
}

void ThreadSafety::PostCallRecordGetFramebufferTilePropertiesQCOM(VkDevice device, VkFramebuffer framebuffer,
                                                                  uint32_t* pPropertiesCount, VkTilePropertiesQCOM* pProperties,
                                                                  const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(framebuffer, record_obj.location);
}

void ThreadSafety::PreCallRecordGetDynamicRenderingTilePropertiesQCOM(VkDevice device, const VkRenderingInfo* pRenderingInfo,
                                                                      VkTilePropertiesQCOM* pProperties,
                                                                      const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordGetDynamicRenderingTilePropertiesQCOM(VkDevice device, const VkRenderingInfo* pRenderingInfo,
                                                                       VkTilePropertiesQCOM* pProperties,
                                                                       const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PreCallRecordSetLatencySleepModeNV(VkDevice device, VkSwapchainKHR swapchain,
                                                      const VkLatencySleepModeInfoNV* pSleepModeInfo,
                                                      const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(swapchain, record_obj.location);
}

void ThreadSafety::PostCallRecordSetLatencySleepModeNV(VkDevice device, VkSwapchainKHR swapchain,
                                                       const VkLatencySleepModeInfoNV* pSleepModeInfo,
                                                       const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(swapchain, record_obj.location);
}

void ThreadSafety::PreCallRecordLatencySleepNV(VkDevice device, VkSwapchainKHR swapchain, const VkLatencySleepInfoNV* pSleepInfo,
                                               const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(swapchain, record_obj.location);
}

void ThreadSafety::PostCallRecordLatencySleepNV(VkDevice device, VkSwapchainKHR swapchain, const VkLatencySleepInfoNV* pSleepInfo,
                                                const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(swapchain, record_obj.location);
}

void ThreadSafety::PreCallRecordSetLatencyMarkerNV(VkDevice device, VkSwapchainKHR swapchain,
                                                   const VkSetLatencyMarkerInfoNV* pLatencyMarkerInfo,
                                                   const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(swapchain, record_obj.location);
}

void ThreadSafety::PostCallRecordSetLatencyMarkerNV(VkDevice device, VkSwapchainKHR swapchain,
                                                    const VkSetLatencyMarkerInfoNV* pLatencyMarkerInfo,
                                                    const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(swapchain, record_obj.location);
}

void ThreadSafety::PreCallRecordGetLatencyTimingsNV(VkDevice device, VkSwapchainKHR swapchain,
                                                    VkGetLatencyMarkerInfoNV* pLatencyMarkerInfo, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(swapchain, record_obj.location);
}

void ThreadSafety::PostCallRecordGetLatencyTimingsNV(VkDevice device, VkSwapchainKHR swapchain,
                                                     VkGetLatencyMarkerInfoNV* pLatencyMarkerInfo, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(swapchain, record_obj.location);
}

void ThreadSafety::PreCallRecordQueueNotifyOutOfBandNV(VkQueue queue, const VkOutOfBandQueueTypeInfoNV* pQueueTypeInfo,
                                                       const RecordObject& record_obj) {
    StartReadObject(queue, record_obj.location);
}

void ThreadSafety::PostCallRecordQueueNotifyOutOfBandNV(VkQueue queue, const VkOutOfBandQueueTypeInfoNV* pQueueTypeInfo,
                                                        const RecordObject& record_obj) {
    FinishReadObject(queue, record_obj.location);
}

void ThreadSafety::PreCallRecordCmdSetAttachmentFeedbackLoopEnableEXT(VkCommandBuffer commandBuffer, VkImageAspectFlags aspectMask,
                                                                      const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetAttachmentFeedbackLoopEnableEXT(VkCommandBuffer commandBuffer, VkImageAspectFlags aspectMask,
                                                                       const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

#ifdef VK_USE_PLATFORM_SCREEN_QNX
void ThreadSafety::PreCallRecordGetScreenBufferPropertiesQNX(VkDevice device, const struct _screen_buffer* buffer,
                                                             VkScreenBufferPropertiesQNX* pProperties,
                                                             const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordGetScreenBufferPropertiesQNX(VkDevice device, const struct _screen_buffer* buffer,
                                                              VkScreenBufferPropertiesQNX* pProperties,
                                                              const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

#endif  // VK_USE_PLATFORM_SCREEN_QNX
void ThreadSafety::PreCallRecordCreateAccelerationStructureKHR(VkDevice device,
                                                               const VkAccelerationStructureCreateInfoKHR* pCreateInfo,
                                                               const VkAllocationCallbacks* pAllocator,
                                                               VkAccelerationStructureKHR* pAccelerationStructure,
                                                               const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordCreateAccelerationStructureKHR(VkDevice device,
                                                                const VkAccelerationStructureCreateInfoKHR* pCreateInfo,
                                                                const VkAllocationCallbacks* pAllocator,
                                                                VkAccelerationStructureKHR* pAccelerationStructure,
                                                                const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pAccelerationStructure);
    }
}

void ThreadSafety::PreCallRecordDestroyAccelerationStructureKHR(VkDevice device, VkAccelerationStructureKHR accelerationStructure,
                                                                const VkAllocationCallbacks* pAllocator,
                                                                const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartWriteObject(accelerationStructure, record_obj.location);
    // Host access to accelerationStructure must be externally synchronized
}

void ThreadSafety::PostCallRecordDestroyAccelerationStructureKHR(VkDevice device, VkAccelerationStructureKHR accelerationStructure,
                                                                 const VkAllocationCallbacks* pAllocator,
                                                                 const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishWriteObject(accelerationStructure, record_obj.location);
    DestroyObject(accelerationStructure);
    // Host access to accelerationStructure must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdBuildAccelerationStructuresKHR(
    VkCommandBuffer commandBuffer, uint32_t infoCount, const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
    const VkAccelerationStructureBuildRangeInfoKHR* const* ppBuildRangeInfos, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdBuildAccelerationStructuresKHR(
    VkCommandBuffer commandBuffer, uint32_t infoCount, const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
    const VkAccelerationStructureBuildRangeInfoKHR* const* ppBuildRangeInfos, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdBuildAccelerationStructuresIndirectKHR(VkCommandBuffer commandBuffer, uint32_t infoCount,
                                                                          const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
                                                                          const VkDeviceAddress* pIndirectDeviceAddresses,
                                                                          const uint32_t* pIndirectStrides,
                                                                          const uint32_t* const* ppMaxPrimitiveCounts,
                                                                          const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdBuildAccelerationStructuresIndirectKHR(
    VkCommandBuffer commandBuffer, uint32_t infoCount, const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
    const VkDeviceAddress* pIndirectDeviceAddresses, const uint32_t* pIndirectStrides, const uint32_t* const* ppMaxPrimitiveCounts,
    const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordBuildAccelerationStructuresKHR(
    VkDevice device, VkDeferredOperationKHR deferredOperation, uint32_t infoCount,
    const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
    const VkAccelerationStructureBuildRangeInfoKHR* const* ppBuildRangeInfos, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(deferredOperation, record_obj.location);
}

void ThreadSafety::PostCallRecordBuildAccelerationStructuresKHR(
    VkDevice device, VkDeferredOperationKHR deferredOperation, uint32_t infoCount,
    const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
    const VkAccelerationStructureBuildRangeInfoKHR* const* ppBuildRangeInfos, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(deferredOperation, record_obj.location);
}

void ThreadSafety::PreCallRecordCopyAccelerationStructureKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                             const VkCopyAccelerationStructureInfoKHR* pInfo,
                                                             const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(deferredOperation, record_obj.location);
}

void ThreadSafety::PostCallRecordCopyAccelerationStructureKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                              const VkCopyAccelerationStructureInfoKHR* pInfo,
                                                              const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(deferredOperation, record_obj.location);
}

void ThreadSafety::PreCallRecordCopyAccelerationStructureToMemoryKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                                     const VkCopyAccelerationStructureToMemoryInfoKHR* pInfo,
                                                                     const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(deferredOperation, record_obj.location);
}

void ThreadSafety::PostCallRecordCopyAccelerationStructureToMemoryKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                                      const VkCopyAccelerationStructureToMemoryInfoKHR* pInfo,
                                                                      const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(deferredOperation, record_obj.location);
}

void ThreadSafety::PreCallRecordCopyMemoryToAccelerationStructureKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                                     const VkCopyMemoryToAccelerationStructureInfoKHR* pInfo,
                                                                     const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(deferredOperation, record_obj.location);
}

void ThreadSafety::PostCallRecordCopyMemoryToAccelerationStructureKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                                      const VkCopyMemoryToAccelerationStructureInfoKHR* pInfo,
                                                                      const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(deferredOperation, record_obj.location);
}

void ThreadSafety::PreCallRecordWriteAccelerationStructuresPropertiesKHR(VkDevice device, uint32_t accelerationStructureCount,
                                                                         const VkAccelerationStructureKHR* pAccelerationStructures,
                                                                         VkQueryType queryType, size_t dataSize, void* pData,
                                                                         size_t stride, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);

    if (pAccelerationStructures) {
        for (uint32_t index = 0; index < accelerationStructureCount; index++) {
            StartReadObject(pAccelerationStructures[index], record_obj.location);
        }
    }
}

void ThreadSafety::PostCallRecordWriteAccelerationStructuresPropertiesKHR(VkDevice device, uint32_t accelerationStructureCount,
                                                                          const VkAccelerationStructureKHR* pAccelerationStructures,
                                                                          VkQueryType queryType, size_t dataSize, void* pData,
                                                                          size_t stride, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);

    if (pAccelerationStructures) {
        for (uint32_t index = 0; index < accelerationStructureCount; index++) {
            FinishReadObject(pAccelerationStructures[index], record_obj.location);
        }
    }
}

void ThreadSafety::PreCallRecordCmdCopyAccelerationStructureKHR(VkCommandBuffer commandBuffer,
                                                                const VkCopyAccelerationStructureInfoKHR* pInfo,
                                                                const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdCopyAccelerationStructureKHR(VkCommandBuffer commandBuffer,
                                                                 const VkCopyAccelerationStructureInfoKHR* pInfo,
                                                                 const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdCopyAccelerationStructureToMemoryKHR(VkCommandBuffer commandBuffer,
                                                                        const VkCopyAccelerationStructureToMemoryInfoKHR* pInfo,
                                                                        const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdCopyAccelerationStructureToMemoryKHR(VkCommandBuffer commandBuffer,
                                                                         const VkCopyAccelerationStructureToMemoryInfoKHR* pInfo,
                                                                         const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdCopyMemoryToAccelerationStructureKHR(VkCommandBuffer commandBuffer,
                                                                        const VkCopyMemoryToAccelerationStructureInfoKHR* pInfo,
                                                                        const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdCopyMemoryToAccelerationStructureKHR(VkCommandBuffer commandBuffer,
                                                                         const VkCopyMemoryToAccelerationStructureInfoKHR* pInfo,
                                                                         const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordGetAccelerationStructureDeviceAddressKHR(VkDevice device,
                                                                         const VkAccelerationStructureDeviceAddressInfoKHR* pInfo,
                                                                         const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordGetAccelerationStructureDeviceAddressKHR(VkDevice device,
                                                                          const VkAccelerationStructureDeviceAddressInfoKHR* pInfo,
                                                                          const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PreCallRecordCmdWriteAccelerationStructuresPropertiesKHR(
    VkCommandBuffer commandBuffer, uint32_t accelerationStructureCount, const VkAccelerationStructureKHR* pAccelerationStructures,
    VkQueryType queryType, VkQueryPool queryPool, uint32_t firstQuery, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);

    if (pAccelerationStructures) {
        for (uint32_t index = 0; index < accelerationStructureCount; index++) {
            StartReadObject(pAccelerationStructures[index], record_obj.location);
        }
    }
    StartReadObject(queryPool, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdWriteAccelerationStructuresPropertiesKHR(
    VkCommandBuffer commandBuffer, uint32_t accelerationStructureCount, const VkAccelerationStructureKHR* pAccelerationStructures,
    VkQueryType queryType, VkQueryPool queryPool, uint32_t firstQuery, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);

    if (pAccelerationStructures) {
        for (uint32_t index = 0; index < accelerationStructureCount; index++) {
            FinishReadObject(pAccelerationStructures[index], record_obj.location);
        }
    }
    FinishReadObject(queryPool, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordGetDeviceAccelerationStructureCompatibilityKHR(
    VkDevice device, const VkAccelerationStructureVersionInfoKHR* pVersionInfo,
    VkAccelerationStructureCompatibilityKHR* pCompatibility, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordGetDeviceAccelerationStructureCompatibilityKHR(
    VkDevice device, const VkAccelerationStructureVersionInfoKHR* pVersionInfo,
    VkAccelerationStructureCompatibilityKHR* pCompatibility, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PreCallRecordGetAccelerationStructureBuildSizesKHR(
    VkDevice device, VkAccelerationStructureBuildTypeKHR buildType, const VkAccelerationStructureBuildGeometryInfoKHR* pBuildInfo,
    const uint32_t* pMaxPrimitiveCounts, VkAccelerationStructureBuildSizesInfoKHR* pSizeInfo, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordGetAccelerationStructureBuildSizesKHR(
    VkDevice device, VkAccelerationStructureBuildTypeKHR buildType, const VkAccelerationStructureBuildGeometryInfoKHR* pBuildInfo,
    const uint32_t* pMaxPrimitiveCounts, VkAccelerationStructureBuildSizesInfoKHR* pSizeInfo, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PreCallRecordCmdTraceRaysKHR(VkCommandBuffer commandBuffer,
                                                const VkStridedDeviceAddressRegionKHR* pRaygenShaderBindingTable,
                                                const VkStridedDeviceAddressRegionKHR* pMissShaderBindingTable,
                                                const VkStridedDeviceAddressRegionKHR* pHitShaderBindingTable,
                                                const VkStridedDeviceAddressRegionKHR* pCallableShaderBindingTable, uint32_t width,
                                                uint32_t height, uint32_t depth, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdTraceRaysKHR(VkCommandBuffer commandBuffer,
                                                 const VkStridedDeviceAddressRegionKHR* pRaygenShaderBindingTable,
                                                 const VkStridedDeviceAddressRegionKHR* pMissShaderBindingTable,
                                                 const VkStridedDeviceAddressRegionKHR* pHitShaderBindingTable,
                                                 const VkStridedDeviceAddressRegionKHR* pCallableShaderBindingTable, uint32_t width,
                                                 uint32_t height, uint32_t depth, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordGetRayTracingCaptureReplayShaderGroupHandlesKHR(VkDevice device, VkPipeline pipeline,
                                                                                uint32_t firstGroup, uint32_t groupCount,
                                                                                size_t dataSize, void* pData,
                                                                                const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(pipeline, record_obj.location);
}

void ThreadSafety::PostCallRecordGetRayTracingCaptureReplayShaderGroupHandlesKHR(VkDevice device, VkPipeline pipeline,
                                                                                 uint32_t firstGroup, uint32_t groupCount,
                                                                                 size_t dataSize, void* pData,
                                                                                 const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(pipeline, record_obj.location);
}

void ThreadSafety::PreCallRecordCmdTraceRaysIndirectKHR(VkCommandBuffer commandBuffer,
                                                        const VkStridedDeviceAddressRegionKHR* pRaygenShaderBindingTable,
                                                        const VkStridedDeviceAddressRegionKHR* pMissShaderBindingTable,
                                                        const VkStridedDeviceAddressRegionKHR* pHitShaderBindingTable,
                                                        const VkStridedDeviceAddressRegionKHR* pCallableShaderBindingTable,
                                                        VkDeviceAddress indirectDeviceAddress, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdTraceRaysIndirectKHR(VkCommandBuffer commandBuffer,
                                                         const VkStridedDeviceAddressRegionKHR* pRaygenShaderBindingTable,
                                                         const VkStridedDeviceAddressRegionKHR* pMissShaderBindingTable,
                                                         const VkStridedDeviceAddressRegionKHR* pHitShaderBindingTable,
                                                         const VkStridedDeviceAddressRegionKHR* pCallableShaderBindingTable,
                                                         VkDeviceAddress indirectDeviceAddress, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordGetRayTracingShaderGroupStackSizeKHR(VkDevice device, VkPipeline pipeline, uint32_t group,
                                                                     VkShaderGroupShaderKHR groupShader,
                                                                     const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(pipeline, record_obj.location);
}

void ThreadSafety::PostCallRecordGetRayTracingShaderGroupStackSizeKHR(VkDevice device, VkPipeline pipeline, uint32_t group,
                                                                      VkShaderGroupShaderKHR groupShader,
                                                                      const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(pipeline, record_obj.location);
}

void ThreadSafety::PreCallRecordCmdSetRayTracingPipelineStackSizeKHR(VkCommandBuffer commandBuffer, uint32_t pipelineStackSize,
                                                                     const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetRayTracingPipelineStackSizeKHR(VkCommandBuffer commandBuffer, uint32_t pipelineStackSize,
                                                                      const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdDrawMeshTasksEXT(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY,
                                                    uint32_t groupCountZ, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdDrawMeshTasksEXT(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY,
                                                     uint32_t groupCountZ, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdDrawMeshTasksIndirectEXT(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                            uint32_t drawCount, uint32_t stride, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    StartReadObject(buffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdDrawMeshTasksIndirectEXT(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                             uint32_t drawCount, uint32_t stride, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    FinishReadObject(buffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdDrawMeshTasksIndirectCountEXT(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                 VkDeviceSize offset, VkBuffer countBuffer,
                                                                 VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                                 uint32_t stride, const RecordObject& record_obj) {
    StartWriteObject(commandBuffer, record_obj.location);
    StartReadObject(buffer, record_obj.location);
    StartReadObject(countBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdDrawMeshTasksIndirectCountEXT(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                  VkDeviceSize offset, VkBuffer countBuffer,
                                                                  VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                                  uint32_t stride, const RecordObject& record_obj) {
    FinishWriteObject(commandBuffer, record_obj.location);
    FinishReadObject(buffer, record_obj.location);
    FinishReadObject(countBuffer, record_obj.location);
    // Host access to commandBuffer must be externally synchronized
}

// NOLINTEND
