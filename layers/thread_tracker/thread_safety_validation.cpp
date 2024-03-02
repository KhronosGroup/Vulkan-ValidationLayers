/* Copyright (c) 2015-2024 The Khronos Group Inc.
 * Copyright (c) 2015-2024 Valve Corporation
 * Copyright (c) 2015-2024 LunarG, Inc.
 * Copyright (c) 2015-2024 Google Inc.
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
 */
#include "generated/chassis.h"
#include "generated/layer_chassis_dispatch.h"  // wrap_handles declaration
#include "thread_tracker/thread_safety_validation.h"

ReadLockGuard ThreadSafety::ReadLock() const { return ReadLockGuard(validation_object_mutex, std::defer_lock); }

WriteLockGuard ThreadSafety::WriteLock() { return WriteLockGuard(validation_object_mutex, std::defer_lock); }

void ThreadSafety::PreCallRecordAllocateCommandBuffers(VkDevice device, const VkCommandBufferAllocateInfo* pAllocateInfo,
                                                       VkCommandBuffer* pCommandBuffers, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartWriteObject(pAllocateInfo->commandPool, record_obj.location);
}

void ThreadSafety::PostCallRecordAllocateCommandBuffers(VkDevice device, const VkCommandBufferAllocateInfo* pAllocateInfo,
                                                        VkCommandBuffer* pCommandBuffers, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishWriteObject(pAllocateInfo->commandPool, record_obj.location);

    // Record mapping from command buffer to command pool
    if (pCommandBuffers) {
        auto lock = WriteLockGuard(thread_safety_lock);
        auto& pool_command_buffers = pool_command_buffers_map[pAllocateInfo->commandPool];
        for (uint32_t index = 0; index < pAllocateInfo->commandBufferCount; index++) {
            command_pool_map.insert_or_assign(pCommandBuffers[index], pAllocateInfo->commandPool);
            CreateObject(pCommandBuffers[index]);
            pool_command_buffers.insert(pCommandBuffers[index]);
        }
    }
}

void ThreadSafety::PreCallRecordCreateDescriptorSetLayout(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo,
                                                          const VkAllocationCallbacks* pAllocator,
                                                          VkDescriptorSetLayout* pSetLayout, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordCreateDescriptorSetLayout(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo,
                                                           const VkAllocationCallbacks* pAllocator,
                                                           VkDescriptorSetLayout* pSetLayout, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pSetLayout);

        // Check whether any binding uses read_only
        bool read_only = (pCreateInfo->flags & VK_DESCRIPTOR_SET_LAYOUT_CREATE_HOST_ONLY_POOL_BIT_EXT) != 0;
        if (!read_only) {
            const auto* flags_create_info = vku::FindStructInPNextChain<VkDescriptorSetLayoutBindingFlagsCreateInfo>(pCreateInfo->pNext);
            if (flags_create_info) {
                for (uint32_t i = 0; i < flags_create_info->bindingCount; ++i) {
                    if (flags_create_info->pBindingFlags[i] & VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT) {
                        read_only = true;
                        break;
                    }
                }
            }
        }
        dsl_read_only_map.insert_or_assign(*pSetLayout, read_only);
    }
}

void ThreadSafety::PreCallRecordAllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo* pAllocateInfo,
                                                       VkDescriptorSet* pDescriptorSets, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartWriteObject(pAllocateInfo->descriptorPool, record_obj.location);
    // Host access to pAllocateInfo::descriptorPool must be externally synchronized
}

void ThreadSafety::PostCallRecordAllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo* pAllocateInfo,
                                                        VkDescriptorSet* pDescriptorSets, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishWriteObject(pAllocateInfo->descriptorPool, record_obj.location);
    // Host access to pAllocateInfo::descriptorPool must be externally synchronized
    if (VK_SUCCESS == record_obj.result) {
        auto lock = WriteLockGuard(thread_safety_lock);
        auto& pool_descriptor_sets = pool_descriptor_sets_map[pAllocateInfo->descriptorPool];
        for (uint32_t index0 = 0; index0 < pAllocateInfo->descriptorSetCount; index0++) {
            CreateObject(pDescriptorSets[index0]);
            pool_descriptor_sets.insert(pDescriptorSets[index0]);

            auto iter = dsl_read_only_map.find(pAllocateInfo->pSetLayouts[index0]);
            if (iter != dsl_read_only_map.end()) {
                ds_read_only_map.insert_or_assign(pDescriptorSets[index0], iter->second);
            } else {
                assert(0 && "descriptor set layout not found");
            }
        }
    }
}

void ThreadSafety::PreCallRecordFreeDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool, uint32_t descriptorSetCount,
                                                   const VkDescriptorSet* pDescriptorSets, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartWriteObject(descriptorPool, record_obj.location);
    if (pDescriptorSets) {
        for (uint32_t index = 0; index < descriptorSetCount; index++) {
            StartWriteObject(pDescriptorSets[index], record_obj.location);
        }
    }
    // Host access to descriptorPool must be externally synchronized
    // Host access to each member of pDescriptorSets must be externally synchronized
}

void ThreadSafety::PostCallRecordFreeDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool, uint32_t descriptorSetCount,
                                                    const VkDescriptorSet* pDescriptorSets, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishWriteObject(descriptorPool, record_obj.location);
    if (pDescriptorSets) {
        for (uint32_t index = 0; index < descriptorSetCount; index++) {
            FinishWriteObject(pDescriptorSets[index], record_obj.location);
        }
    }
    // Host access to descriptorPool must be externally synchronized
    // Host access to each member of pDescriptorSets must be externally synchronized
    // Host access to pAllocateInfo::descriptorPool must be externally synchronized
    if (VK_SUCCESS == record_obj.result) {
        auto lock = WriteLockGuard(thread_safety_lock);
        auto& pool_descriptor_sets = pool_descriptor_sets_map[descriptorPool];
        for (uint32_t index0 = 0; index0 < descriptorSetCount; index0++) {
            auto descriptor_set = pDescriptorSets[index0];
            DestroyObject(descriptor_set);
            pool_descriptor_sets.erase(descriptor_set);
            ds_read_only_map.erase(descriptor_set);
        }
    }
}

void ThreadSafety::PreCallRecordDestroyDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool,
                                                      const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartWriteObject(descriptorPool, record_obj.location);
    // Host access to descriptorPool must be externally synchronized
    auto lock = ReadLockGuard(thread_safety_lock);
    auto iterator = pool_descriptor_sets_map.find(descriptorPool);
    // Possible to have no descriptor sets allocated from pool
    if (iterator != pool_descriptor_sets_map.end()) {
        for (auto descriptor_set : pool_descriptor_sets_map[descriptorPool]) {
            StartWriteObject(descriptor_set, record_obj.location);
        }
    }
}

void ThreadSafety::PostCallRecordDestroyDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool,
                                                       const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishWriteObject(descriptorPool, record_obj.location);
    DestroyObject(descriptorPool);
    // Host access to descriptorPool must be externally synchronized
    {
        auto lock = WriteLockGuard(thread_safety_lock);
        // remove references to implicitly freed descriptor sets
        for (auto descriptor_set : pool_descriptor_sets_map[descriptorPool]) {
            FinishWriteObject(descriptor_set, record_obj.location);
            DestroyObject(descriptor_set);
            ds_read_only_map.erase(descriptor_set);
        }
        pool_descriptor_sets_map[descriptorPool].clear();
        pool_descriptor_sets_map.erase(descriptorPool);
    }
}

void ThreadSafety::PreCallRecordResetDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool,
                                                    VkDescriptorPoolResetFlags flags, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartWriteObject(descriptorPool, record_obj.location);
    // Host access to descriptorPool must be externally synchronized
    // any sname:VkDescriptorSet objects allocated from pname:descriptorPool must be externally synchronized between host accesses
    auto lock = ReadLockGuard(thread_safety_lock);
    auto iterator = pool_descriptor_sets_map.find(descriptorPool);
    // Possible to have no descriptor sets allocated from pool
    if (iterator != pool_descriptor_sets_map.end()) {
        for (auto descriptor_set : pool_descriptor_sets_map[descriptorPool]) {
            StartWriteObject(descriptor_set, record_obj.location);
        }
    }
}

void ThreadSafety::PostCallRecordResetDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool,
                                                     VkDescriptorPoolResetFlags flags, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishWriteObject(descriptorPool, record_obj.location);
    // Host access to descriptorPool must be externally synchronized
    // any sname:VkDescriptorSet objects allocated from pname:descriptorPool must be externally synchronized between host accesses
    if (VK_SUCCESS == record_obj.result) {
        // remove references to implicitly freed descriptor sets
        auto lock = WriteLockGuard(thread_safety_lock);
        for (auto descriptor_set : pool_descriptor_sets_map[descriptorPool]) {
            FinishWriteObject(descriptor_set, record_obj.location);
            DestroyObject(descriptor_set);
            ds_read_only_map.erase(descriptor_set);
        }
        pool_descriptor_sets_map[descriptorPool].clear();
    }
}

bool ThreadSafety::DsReadOnly(VkDescriptorSet set) const {
    auto iter = ds_read_only_map.find(set);
    if (iter != ds_read_only_map.end()) {
        return iter->second;
    }
    return false;
}

void ThreadSafety::PreCallRecordUpdateDescriptorSets(VkDevice device, uint32_t descriptorWriteCount,
                                                     const VkWriteDescriptorSet* pDescriptorWrites, uint32_t descriptorCopyCount,
                                                     const VkCopyDescriptorSet* pDescriptorCopies, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    if (pDescriptorWrites) {
        for (uint32_t index = 0; index < descriptorWriteCount; index++) {
            auto dstSet = pDescriptorWrites[index].dstSet;
            bool read_only = DsReadOnly(dstSet);
            if (read_only) {
                StartReadObject(dstSet, record_obj.location);
            } else {
                StartWriteObject(dstSet, record_obj.location);
            }
        }
    }
    if (pDescriptorCopies) {
        for (uint32_t index = 0; index < descriptorCopyCount; index++) {
            auto dstSet = pDescriptorCopies[index].dstSet;
            bool read_only = DsReadOnly(dstSet);
            if (read_only) {
                StartReadObject(dstSet, record_obj.location);
            } else {
                StartWriteObject(dstSet, record_obj.location);
            }
            StartReadObject(pDescriptorCopies[index].srcSet, record_obj.location);
        }
    }
    // Host access to pDescriptorWrites[].dstSet must be externally synchronized
    // Host access to pDescriptorCopies[].dstSet must be externally synchronized
}

void ThreadSafety::PostCallRecordUpdateDescriptorSets(VkDevice device, uint32_t descriptorWriteCount,
                                                      const VkWriteDescriptorSet* pDescriptorWrites, uint32_t descriptorCopyCount,
                                                      const VkCopyDescriptorSet* pDescriptorCopies,
                                                      const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    if (pDescriptorWrites) {
        for (uint32_t index = 0; index < descriptorWriteCount; index++) {
            auto dstSet = pDescriptorWrites[index].dstSet;
            bool read_only = DsReadOnly(dstSet);
            if (read_only) {
                FinishReadObject(dstSet, record_obj.location);
            } else {
                FinishWriteObject(dstSet, record_obj.location);
            }
        }
    }
    if (pDescriptorCopies) {
        for (uint32_t index = 0; index < descriptorCopyCount; index++) {
            auto dstSet = pDescriptorCopies[index].dstSet;
            bool read_only = DsReadOnly(dstSet);
            if (read_only) {
                FinishReadObject(dstSet, record_obj.location);
            } else {
                FinishWriteObject(dstSet, record_obj.location);
            }
            FinishReadObject(pDescriptorCopies[index].srcSet, record_obj.location);
        }
    }
    // Host access to pDescriptorWrites[].dstSet must be externally synchronized
    // Host access to pDescriptorCopies[].dstSet must be externally synchronized
}

void ThreadSafety::PreCallRecordUpdateDescriptorSetWithTemplate(VkDevice device, VkDescriptorSet descriptorSet,
                                                                VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                                const void* pData, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(descriptorUpdateTemplate, record_obj.location);

    const bool read_only = DsReadOnly(descriptorSet);
    if (read_only) {
        StartReadObject(descriptorSet, record_obj.location);
    } else {
        StartWriteObject(descriptorSet, record_obj.location);
    }
    // Host access to descriptorSet must be externally synchronized
}

void ThreadSafety::PostCallRecordUpdateDescriptorSetWithTemplate(VkDevice device, VkDescriptorSet descriptorSet,
                                                                 VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                                 const void* pData, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(descriptorUpdateTemplate, record_obj.location);

    const bool read_only = DsReadOnly(descriptorSet);
    if (read_only) {
        FinishReadObject(descriptorSet, record_obj.location);
    } else {
        FinishWriteObject(descriptorSet, record_obj.location);
    }
    // Host access to descriptorSet must be externally synchronized
}

void ThreadSafety::PreCallRecordUpdateDescriptorSetWithTemplateKHR(VkDevice device, VkDescriptorSet descriptorSet,
                                                                   VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                                   const void* pData, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(descriptorUpdateTemplate, record_obj.location);

    const bool read_only = DsReadOnly(descriptorSet);
    if (read_only) {
        StartReadObject(descriptorSet, record_obj.location);
    } else {
        StartWriteObject(descriptorSet, record_obj.location);
    }
    // Host access to descriptorSet must be externally synchronized
}

void ThreadSafety::PostCallRecordUpdateDescriptorSetWithTemplateKHR(VkDevice device, VkDescriptorSet descriptorSet,
                                                                    VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                                    const void* pData, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(descriptorUpdateTemplate, record_obj.location);

    const bool read_only = DsReadOnly(descriptorSet);
    if (read_only) {
        FinishReadObject(descriptorSet, record_obj.location);
    } else {
        FinishWriteObject(descriptorSet, record_obj.location);
    }
    // Host access to descriptorSet must be externally synchronized
}

void ThreadSafety::PreCallRecordFreeCommandBuffers(VkDevice device, VkCommandPool commandPool, uint32_t commandBufferCount,
                                                   const VkCommandBuffer* pCommandBuffers, const RecordObject& record_obj) {
    const bool lockCommandPool = false;  // pool is already directly locked
    StartReadObjectParentInstance(device, record_obj.location);
    StartWriteObject(commandPool, record_obj.location);
    if (pCommandBuffers) {
        // Even though we're immediately "finishing" below, we still are testing for concurrency with any call in process
        // so this isn't a no-op
        // The driver may immediately reuse command buffers in another thread.
        // These updates need to be done before calling down to the driver.
        auto lock = WriteLockGuard(thread_safety_lock);
        auto& pool_command_buffers = pool_command_buffers_map[commandPool];
        for (uint32_t index = 0; index < commandBufferCount; index++) {
            StartWriteObject(pCommandBuffers[index], record_obj.location, lockCommandPool);
            FinishWriteObject(pCommandBuffers[index], record_obj.location, lockCommandPool);
            DestroyObject(pCommandBuffers[index]);
            pool_command_buffers.erase(pCommandBuffers[index]);
            command_pool_map.erase(pCommandBuffers[index]);
        }
    }
}

void ThreadSafety::PostCallRecordFreeCommandBuffers(VkDevice device, VkCommandPool commandPool, uint32_t commandBufferCount,
                                                    const VkCommandBuffer* pCommandBuffers, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishWriteObject(commandPool, record_obj.location);
}

void ThreadSafety::PreCallRecordCreateCommandPool(VkDevice device, const VkCommandPoolCreateInfo* pCreateInfo,
                                                  const VkAllocationCallbacks* pAllocator, VkCommandPool* pCommandPool,
                                                  const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordCreateCommandPool(VkDevice device, const VkCommandPoolCreateInfo* pCreateInfo,
                                                   const VkAllocationCallbacks* pAllocator, VkCommandPool* pCommandPool,
                                                   const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pCommandPool);
        c_VkCommandPoolContents.CreateObject(*pCommandPool);
    }
}

void ThreadSafety::PreCallRecordResetCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolResetFlags flags,
                                                 const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartWriteObject(commandPool, record_obj.location);
    // Check for any uses of non-externally sync'd command buffers (for example from vkCmdExecuteCommands)
    c_VkCommandPoolContents.StartWrite(commandPool, record_obj.location);
    // Host access to commandPool must be externally synchronized
}

void ThreadSafety::PostCallRecordResetCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolResetFlags flags,
                                                  const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishWriteObject(commandPool, record_obj.location);
    c_VkCommandPoolContents.FinishWrite(commandPool, record_obj.location);
    // Host access to commandPool must be externally synchronized
}

void ThreadSafety::PreCallRecordDestroyCommandPool(VkDevice device, VkCommandPool commandPool,
                                                   const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartWriteObject(commandPool, record_obj.location);
    // Check for any uses of non-externally sync'd command buffers (for example from vkCmdExecuteCommands)
    c_VkCommandPoolContents.StartWrite(commandPool, record_obj.location);
    // Host access to commandPool must be externally synchronized

    auto lock = WriteLockGuard(thread_safety_lock);
    // The driver may immediately reuse command buffers in another thread.
    // These updates need to be done before calling down to the driver.
    // remove references to implicitly freed command pools
    for (auto command_buffer : pool_command_buffers_map[commandPool]) {
        DestroyObject(command_buffer);
    }
    pool_command_buffers_map[commandPool].clear();
    pool_command_buffers_map.erase(commandPool);
}

void ThreadSafety::PostCallRecordDestroyCommandPool(VkDevice device, VkCommandPool commandPool,
                                                    const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishWriteObject(commandPool, record_obj.location);
    DestroyObject(commandPool);
    c_VkCommandPoolContents.FinishWrite(commandPool, record_obj.location);
    c_VkCommandPoolContents.DestroyObject(commandPool);
}

// GetSwapchainImages can return a non-zero count with a NULL pSwapchainImages pointer.  Let's avoid crashes by ignoring
// pSwapchainImages.
void ThreadSafety::PreCallRecordGetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapchain, uint32_t* pSwapchainImageCount,
                                                      VkImage* pSwapchainImages, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(swapchain, record_obj.location);
}

void ThreadSafety::PostCallRecordGetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapchain, uint32_t* pSwapchainImageCount,
                                                       VkImage* pSwapchainImages, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObject(swapchain, record_obj.location);
    if (pSwapchainImages != nullptr) {
        auto lock = WriteLockGuard(thread_safety_lock);
        auto& wrapped_swapchain_image_handles = swapchain_wrapped_image_handle_map[swapchain];
        for (uint32_t i = static_cast<uint32_t>(wrapped_swapchain_image_handles.size()); i < *pSwapchainImageCount; i++) {
            CreateObject(pSwapchainImages[i]);
            wrapped_swapchain_image_handles.emplace_back(pSwapchainImages[i]);
        }
    }
}

void ThreadSafety::PreCallRecordDestroySwapchainKHR(VkDevice device, VkSwapchainKHR swapchain,
                                                    const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartWriteObject(swapchain, record_obj.location);
    // Host access to swapchain must be externally synchronized
    auto lock = ReadLockGuard(thread_safety_lock);
    for (auto& image_handle : swapchain_wrapped_image_handle_map[swapchain]) {
        StartWriteObject(image_handle, record_obj.location);
    }
}

void ThreadSafety::PostCallRecordDestroySwapchainKHR(VkDevice device, VkSwapchainKHR swapchain,
                                                     const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishWriteObject(swapchain, record_obj.location);
    DestroyObject(swapchain);
    // Host access to swapchain must be externally synchronized
    auto lock = WriteLockGuard(thread_safety_lock);
    for (auto& image_handle : swapchain_wrapped_image_handle_map[swapchain]) {
        FinishWriteObject(image_handle, record_obj.location);
        DestroyObject(image_handle);
    }
    swapchain_wrapped_image_handle_map.erase(swapchain);
}

void ThreadSafety::PreCallRecordDestroyDevice(VkDevice device, const VkAllocationCallbacks* pAllocator,
                                              const RecordObject& record_obj) {
    StartWriteObjectParentInstance(device, record_obj.location);
    // Host access to device must be externally synchronized
}

void ThreadSafety::PostCallRecordDestroyDevice(VkDevice device, const VkAllocationCallbacks* pAllocator,
                                               const RecordObject& record_obj) {
    FinishWriteObjectParentInstance(device, record_obj.location);
    DestroyObjectParentInstance(device);
    // Host access to device must be externally synchronized
    auto lock = WriteLockGuard(thread_safety_lock);
    for (auto& queue : device_queues_map[device]) {
        DestroyObject(queue);
    }
    device_queues_map[device].clear();
}

void ThreadSafety::PreCallRecordGetDeviceQueue(VkDevice device, uint32_t queueFamilyIndex, uint32_t queueIndex, VkQueue* pQueue,
                                               const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordGetDeviceQueue(VkDevice device, uint32_t queueFamilyIndex, uint32_t queueIndex, VkQueue* pQueue,
                                                const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    CreateObject(*pQueue);
    auto lock = WriteLockGuard(thread_safety_lock);
    device_queues_map[device].insert(*pQueue);
}

void ThreadSafety::PreCallRecordGetDeviceQueue2(VkDevice device, const VkDeviceQueueInfo2* pQueueInfo, VkQueue* pQueue,
                                                const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
}

void ThreadSafety::PostCallRecordGetDeviceQueue2(VkDevice device, const VkDeviceQueueInfo2* pQueueInfo, VkQueue* pQueue,
                                                 const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    CreateObject(*pQueue);
    auto lock = WriteLockGuard(thread_safety_lock);
    device_queues_map[device].insert(*pQueue);
}

void ThreadSafety::PostCallRecordGetPhysicalDeviceDisplayPropertiesKHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount,
                                                                       VkDisplayPropertiesKHR* pProperties,
                                                                       const RecordObject& record_obj) {
    if ((record_obj.result != VK_SUCCESS) && (record_obj.result != VK_INCOMPLETE)) return;
    if (pProperties) {
        for (uint32_t i = 0; i < *pPropertyCount; ++i) {
            CreateObjectParentInstance(pProperties[i].display);
        }
    }
}

void ThreadSafety::PostCallRecordGetPhysicalDeviceDisplayProperties2KHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount,
                                                                        VkDisplayProperties2KHR* pProperties,
                                                                        const RecordObject& record_obj) {
    if ((record_obj.result != VK_SUCCESS) && (record_obj.result != VK_INCOMPLETE)) return;
    if (pProperties) {
        for (uint32_t i = 0; i < *pPropertyCount; ++i) {
            CreateObjectParentInstance(pProperties[i].displayProperties.display);
        }
    }
}

void ThreadSafety::PostCallRecordGetPhysicalDeviceDisplayPlanePropertiesKHR(VkPhysicalDevice physicalDevice,
                                                                            uint32_t* pPropertyCount,
                                                                            VkDisplayPlanePropertiesKHR* pProperties,
                                                                            const RecordObject& record_obj) {
    if ((record_obj.result != VK_SUCCESS) && (record_obj.result != VK_INCOMPLETE)) return;
    if (pProperties) {
        for (uint32_t i = 0; i < *pPropertyCount; ++i) {
            CreateObjectParentInstance(pProperties[i].currentDisplay);
        }
    }
}

void ThreadSafety::PostCallRecordGetPhysicalDeviceDisplayPlaneProperties2KHR(VkPhysicalDevice physicalDevice,
                                                                             uint32_t* pPropertyCount,
                                                                             VkDisplayPlaneProperties2KHR* pProperties,
                                                                             const RecordObject& record_obj) {
    if ((record_obj.result != VK_SUCCESS) && (record_obj.result != VK_INCOMPLETE)) return;
    if (pProperties) {
        for (uint32_t i = 0; i < *pPropertyCount; ++i) {
            CreateObjectParentInstance(pProperties[i].displayPlaneProperties.currentDisplay);
        }
    }
}

void ThreadSafety::PreCallRecordGetDisplayPlaneSupportedDisplaysKHR(VkPhysicalDevice physicalDevice, uint32_t planeIndex,
                                                                    uint32_t* pDisplayCount, VkDisplayKHR* pDisplays,
                                                                    const RecordObject& record_obj) {
    // Nothing to do for this pre-call function
}

void ThreadSafety::PostCallRecordGetDisplayPlaneSupportedDisplaysKHR(VkPhysicalDevice physicalDevice, uint32_t planeIndex,
                                                                     uint32_t* pDisplayCount, VkDisplayKHR* pDisplays,
                                                                     const RecordObject& record_obj) {
    if ((record_obj.result != VK_SUCCESS) && (record_obj.result != VK_INCOMPLETE)) return;
    if (pDisplays) {
        for (uint32_t index = 0; index < *pDisplayCount; index++) {
            CreateObjectParentInstance(pDisplays[index]);
        }
    }
}

void ThreadSafety::PreCallRecordGetDisplayModePropertiesKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display,
                                                            uint32_t* pPropertyCount, VkDisplayModePropertiesKHR* pProperties,
                                                            const RecordObject& record_obj) {
    StartReadObjectParentInstance(display, record_obj.location);
}

void ThreadSafety::PostCallRecordGetDisplayModePropertiesKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display,
                                                             uint32_t* pPropertyCount, VkDisplayModePropertiesKHR* pProperties,
                                                             const RecordObject& record_obj) {
    FinishReadObjectParentInstance(display, record_obj.location);
    if ((record_obj.result != VK_SUCCESS) && (record_obj.result != VK_INCOMPLETE)) return;
    if (pProperties != nullptr) {
        for (uint32_t index = 0; index < *pPropertyCount; index++) {
            CreateObject(pProperties[index].displayMode);
        }
    }
}

void ThreadSafety::PreCallRecordGetDisplayModeProperties2KHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display,
                                                             uint32_t* pPropertyCount, VkDisplayModeProperties2KHR* pProperties,
                                                             const RecordObject& record_obj) {
    StartReadObjectParentInstance(display, record_obj.location);
}

void ThreadSafety::PostCallRecordGetDisplayModeProperties2KHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display,
                                                              uint32_t* pPropertyCount, VkDisplayModeProperties2KHR* pProperties,
                                                              const RecordObject& record_obj) {
    FinishReadObjectParentInstance(display, record_obj.location);
    if ((record_obj.result != VK_SUCCESS) && (record_obj.result != VK_INCOMPLETE)) return;
    if (pProperties != nullptr) {
        for (uint32_t index = 0; index < *pPropertyCount; index++) {
            CreateObject(pProperties[index].displayModeProperties.displayMode);
        }
    }
}

void ThreadSafety::PreCallRecordGetDisplayPlaneCapabilities2KHR(VkPhysicalDevice physicalDevice,
                                                                const VkDisplayPlaneInfo2KHR* pDisplayPlaneInfo,
                                                                VkDisplayPlaneCapabilities2KHR* pCapabilities,
                                                                const RecordObject& record_obj) {
    StartWriteObject(pDisplayPlaneInfo->mode, record_obj.location);
}

void ThreadSafety::PostCallRecordGetDisplayPlaneCapabilities2KHR(VkPhysicalDevice physicalDevice,
                                                                 const VkDisplayPlaneInfo2KHR* pDisplayPlaneInfo,
                                                                 VkDisplayPlaneCapabilities2KHR* pCapabilities,
                                                                 const RecordObject& record_obj) {
    FinishWriteObject(pDisplayPlaneInfo->mode, record_obj.location);
}

#ifdef VK_USE_PLATFORM_XLIB_XRANDR_EXT

void ThreadSafety::PostCallRecordGetRandROutputDisplayEXT(VkPhysicalDevice physicalDevice, Display* dpy, RROutput rrOutput,
                                                          VkDisplayKHR* pDisplay, const RecordObject& record_obj) {
    if ((record_obj.result != VK_SUCCESS) || (pDisplay == nullptr)) return;
    CreateObjectParentInstance(*pDisplay);
}

#endif  // VK_USE_PLATFORM_XLIB_XRANDR_EXT

void ThreadSafety::PostCallRecordGetDrmDisplayEXT(VkPhysicalDevice physicalDevice, int32_t drmFd, uint32_t connectorId,
                                                  VkDisplayKHR* display, const RecordObject& record_obj) {
    if ((record_obj.result != VK_SUCCESS) || (display == nullptr)) return;
    CreateObjectParentInstance(*display);
}

void ThreadSafety::PreCallRecordRegisterDisplayEventEXT(VkDevice device, VkDisplayKHR display,
                                                        const VkDisplayEventInfoEXT* pDisplayEventInfo,
                                                        const VkAllocationCallbacks* pAllocator, VkFence* pFence,
                                                        const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObjectParentInstance(display, record_obj.location);
}

void ThreadSafety::PostCallRecordRegisterDisplayEventEXT(VkDevice device, VkDisplayKHR display,
                                                         const VkDisplayEventInfoEXT* pDisplayEventInfo,
                                                         const VkAllocationCallbacks* pAllocator, VkFence* pFence,
                                                         const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    FinishReadObjectParentInstance(display, record_obj.location);
    if (record_obj.result == VK_SUCCESS) {
        CreateObject(*pFence);
    }
}

void ThreadSafety::PreCallRecordDeviceWaitIdle(VkDevice device, const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    auto lock = ReadLockGuard(thread_safety_lock);
    const auto& queue_set = device_queues_map[device];
    for (const auto& queue : queue_set) {
        StartWriteObject(queue, record_obj.location);
    }
}

void ThreadSafety::PostCallRecordDeviceWaitIdle(VkDevice device, const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
    auto lock = ReadLockGuard(thread_safety_lock);
    const auto& queue_set = device_queues_map[device];
    for (const auto& queue : queue_set) {
        FinishWriteObject(queue, record_obj.location);
    }
}

void ThreadSafety::PreCallRecordCreateRayTracingPipelinesKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                             VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                             const VkRayTracingPipelineCreateInfoKHR* pCreateInfos,
                                                             const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                             const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);
    StartReadObject(deferredOperation, record_obj.location);
    StartReadObject(pipelineCache, record_obj.location);
}

void ThreadSafety::PostCallRecordCreateRayTracingPipelinesKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                              VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                              const VkRayTracingPipelineCreateInfoKHR* pCreateInfos,
                                                              const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                              const RecordObject& record_obj) {
    auto unlock_objects = [this, device, deferredOperation, pipelineCache, record_obj]() {
        this->FinishReadObjectParentInstance(device, record_obj.location);
        this->FinishReadObject(deferredOperation, record_obj.location);
        this->FinishReadObject(pipelineCache, record_obj.location);
    };

    auto register_objects = [this](const std::vector<VkPipeline>& pipelines) {
        for (auto pipe : pipelines) {
            if (!pipe) continue;
            CreateObject(pipe);
        }
    };

    // Fix check for deferred ray tracing pipeline creation
    // https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/5817
    const bool is_operation_deferred = (deferredOperation != VK_NULL_HANDLE && record_obj.result == VK_OPERATION_DEFERRED_KHR);
    if (is_operation_deferred) {
        auto layer_data = GetLayerDataPtr(GetDispatchKey(device), layer_data_map);
        if (wrap_handles) {
            deferredOperation = layer_data->Unwrap(deferredOperation);
        }

        // Unlock objects once the deferred operation is complete
        std::vector<std::function<void()>> post_completion_fns;
        auto completion_find = layer_data->deferred_operation_post_completion.pop(deferredOperation);
        if (completion_find->first) {
            post_completion_fns = std::move(completion_find->second);
        }
        post_completion_fns.emplace_back(unlock_objects);
        layer_data->deferred_operation_post_completion.insert(deferredOperation, std::move(post_completion_fns));

        // We will only register the object once we know it was created successfully
        std::vector<std::function<void(const std::vector<VkPipeline>&)>> post_check_fns;
        auto check_find = layer_data->deferred_operation_post_check.pop(deferredOperation);
        if (check_find->first) {
            post_check_fns = std::move(check_find->second);
        }
        post_check_fns.emplace_back(register_objects);
        layer_data->deferred_operation_post_check.insert(deferredOperation, std::move(post_check_fns));
    } else {
        unlock_objects();
        if (pPipelines) {
            for (uint32_t index = 0; index < createInfoCount; index++) {
                if (!pPipelines[index]) continue;
                CreateObject(pPipelines[index]);
            }
        }
    }
}

void ThreadSafety::PreCallRecordQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* pPresentInfo,
                                                const RecordObject& record_obj) {
    StartWriteObject(queue, record_obj.location);
    uint32_t waitSemaphoreCount = pPresentInfo->waitSemaphoreCount;
    if (pPresentInfo->pWaitSemaphores != nullptr) {
        for (uint32_t index = 0; index < waitSemaphoreCount; index++) {
            StartReadObject(pPresentInfo->pWaitSemaphores[index], record_obj.location);
        }
    }
    if (pPresentInfo->pSwapchains != nullptr) {
        for (uint32_t index = 0; index < pPresentInfo->swapchainCount; ++index) {
            StartWriteObject(pPresentInfo->pSwapchains[index], record_obj.location);
        }
    }
}

void ThreadSafety::PostCallRecordQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* pPresentInfo,
                                                 const RecordObject& record_obj) {
    FinishWriteObject(queue, record_obj.location);
    uint32_t waitSemaphoreCount = pPresentInfo->waitSemaphoreCount;
    if (pPresentInfo->pWaitSemaphores != nullptr) {
        for (uint32_t index = 0; index < waitSemaphoreCount; index++) {
            FinishReadObject(pPresentInfo->pWaitSemaphores[index], record_obj.location);
        }
    }
    if (pPresentInfo->pSwapchains != nullptr) {
        for (uint32_t index = 0; index < pPresentInfo->swapchainCount; ++index) {
            FinishWriteObject(pPresentInfo->pSwapchains[index], record_obj.location);
        }
    }
}

void ThreadSafety::PreCallRecordWaitForPresentKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t presentId, uint64_t timeout,
                                                  const RecordObject& record_obj) {
    StartReadObjectParentInstance(device, record_obj.location);

    // Do not track swapchain parameter for vkWaitForPresentKHR.
    // vkWaitForPresentKHR has exception to external synchronization rules that swapchain
    // passed to it can be used by the functions (except vkDestroySwapchainKHR) in other threads.

    // NOTE: when we add support for tracking opposite side of the thread conflict (planned functionality),
    // then it would be possible to account for vkWaitForPresentKHR + vkDestroySwapchainKHR combination.
    // In that case we still need to consider if that's something that's worth to do.
    // Code for checking that combination will interact with main thread safety detection code,
    // so it should be simple and robust addition in order not to break main thread safety detection code
    // in a subtle way (threading!). Ratio risk/value looks too high now.
}

void ThreadSafety::PostCallRecordWaitForPresentKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t presentId, uint64_t timeout,
                                                   const RecordObject& record_obj) {
    FinishReadObjectParentInstance(device, record_obj.location);
}
