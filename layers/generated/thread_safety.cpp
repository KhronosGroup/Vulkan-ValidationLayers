
// This file is ***GENERATED***.  Do Not Edit.
// See thread_safety_generator.py for modifications.

/* Copyright (c) 2015-2023 The Khronos Group Inc.
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
 */
#include "chassis.h"
#include "layer_chassis_dispatch.h"
#include "thread_safety.h"


ReadLockGuard ThreadSafety::ReadLock() const {
    return ReadLockGuard(validation_object_mutex, std::defer_lock);
}

WriteLockGuard ThreadSafety::WriteLock() {
    return WriteLockGuard(validation_object_mutex, std::defer_lock);
}

void ThreadSafety::PreCallRecordAllocateCommandBuffers(VkDevice device, const VkCommandBufferAllocateInfo *pAllocateInfo,
                                                       VkCommandBuffer *pCommandBuffers) {
    StartReadObjectParentInstance(device, "vkAllocateCommandBuffers");
    StartWriteObject(pAllocateInfo->commandPool, "vkAllocateCommandBuffers");
}

void ThreadSafety::PostCallRecordAllocateCommandBuffers(VkDevice device, const VkCommandBufferAllocateInfo *pAllocateInfo,
                                                        VkCommandBuffer *pCommandBuffers, VkResult result) {
    FinishReadObjectParentInstance(device, "vkAllocateCommandBuffers");
    FinishWriteObject(pAllocateInfo->commandPool, "vkAllocateCommandBuffers");

    // Record mapping from command buffer to command pool
    if(pCommandBuffers) {
        auto lock = WriteLockGuard(thread_safety_lock);
        auto &pool_command_buffers = pool_command_buffers_map[pAllocateInfo->commandPool];
        for (uint32_t index = 0; index < pAllocateInfo->commandBufferCount; index++) {
            command_pool_map.insert_or_assign(pCommandBuffers[index], pAllocateInfo->commandPool);
            CreateObject(pCommandBuffers[index]);
            pool_command_buffers.insert(pCommandBuffers[index]);
        }
    }
}


void ThreadSafety::PreCallRecordCreateDescriptorSetLayout(
    VkDevice                                    device,
    const VkDescriptorSetLayoutCreateInfo*      pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDescriptorSetLayout*                      pSetLayout) {
    StartReadObjectParentInstance(device, "vkCreateDescriptorSetLayout");
}

void ThreadSafety::PostCallRecordCreateDescriptorSetLayout(
    VkDevice                                    device,
    const VkDescriptorSetLayoutCreateInfo*      pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDescriptorSetLayout*                      pSetLayout,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkCreateDescriptorSetLayout");
    if (result == VK_SUCCESS) {
        CreateObject(*pSetLayout);

        // Check whether any binding uses read_only
        bool read_only = (pCreateInfo->flags & VK_DESCRIPTOR_SET_LAYOUT_CREATE_HOST_ONLY_POOL_BIT_EXT) != 0;
        if (!read_only) {
            const auto *flags_create_info = LvlFindInChain<VkDescriptorSetLayoutBindingFlagsCreateInfo>(pCreateInfo->pNext);
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

void ThreadSafety::PreCallRecordAllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo *pAllocateInfo,
                                                       VkDescriptorSet *pDescriptorSets) {
    StartReadObjectParentInstance(device, "vkAllocateDescriptorSets");
    StartWriteObject(pAllocateInfo->descriptorPool, "vkAllocateDescriptorSets");
    // Host access to pAllocateInfo::descriptorPool must be externally synchronized
}

void ThreadSafety::PostCallRecordAllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo *pAllocateInfo,
                                                        VkDescriptorSet *pDescriptorSets, VkResult result) {
    FinishReadObjectParentInstance(device, "vkAllocateDescriptorSets");
    FinishWriteObject(pAllocateInfo->descriptorPool, "vkAllocateDescriptorSets");
    // Host access to pAllocateInfo::descriptorPool must be externally synchronized
    if (VK_SUCCESS == result) {
        auto lock = WriteLockGuard(thread_safety_lock);
        auto &pool_descriptor_sets = pool_descriptor_sets_map[pAllocateInfo->descriptorPool];
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

void ThreadSafety::PreCallRecordFreeDescriptorSets(
    VkDevice                                    device,
    VkDescriptorPool                            descriptorPool,
    uint32_t                                    descriptorSetCount,
    const VkDescriptorSet*                      pDescriptorSets) {
    StartReadObjectParentInstance(device, "vkFreeDescriptorSets");
    StartWriteObject(descriptorPool, "vkFreeDescriptorSets");
    if (pDescriptorSets) {
        for (uint32_t index=0; index < descriptorSetCount; index++) {
            StartWriteObject(pDescriptorSets[index], "vkFreeDescriptorSets");
        }
    }
    // Host access to descriptorPool must be externally synchronized
    // Host access to each member of pDescriptorSets must be externally synchronized
}

void ThreadSafety::PostCallRecordFreeDescriptorSets(
    VkDevice                                    device,
    VkDescriptorPool                            descriptorPool,
    uint32_t                                    descriptorSetCount,
    const VkDescriptorSet*                      pDescriptorSets,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkFreeDescriptorSets");
    FinishWriteObject(descriptorPool, "vkFreeDescriptorSets");
    if (pDescriptorSets) {
        for (uint32_t index=0; index < descriptorSetCount; index++) {
            FinishWriteObject(pDescriptorSets[index], "vkFreeDescriptorSets");
        }
    }
    // Host access to descriptorPool must be externally synchronized
    // Host access to each member of pDescriptorSets must be externally synchronized
    // Host access to pAllocateInfo::descriptorPool must be externally synchronized
    if (VK_SUCCESS == result) {
        auto lock = WriteLockGuard(thread_safety_lock);
        auto &pool_descriptor_sets = pool_descriptor_sets_map[descriptorPool];
        for (uint32_t index0 = 0; index0 < descriptorSetCount; index0++) {
            auto descriptor_set = pDescriptorSets[index0];
            DestroyObject(descriptor_set);
            pool_descriptor_sets.erase(descriptor_set);
            ds_read_only_map.erase(descriptor_set);
        }
    }
}

void ThreadSafety::PreCallRecordDestroyDescriptorPool(
    VkDevice                                    device,
    VkDescriptorPool                            descriptorPool,
    const VkAllocationCallbacks*                pAllocator) {
    StartReadObjectParentInstance(device, "vkDestroyDescriptorPool");
    StartWriteObject(descriptorPool, "vkDestroyDescriptorPool");
    // Host access to descriptorPool must be externally synchronized
    auto lock = ReadLockGuard(thread_safety_lock);
    auto iterator = pool_descriptor_sets_map.find(descriptorPool);
    // Possible to have no descriptor sets allocated from pool
    if (iterator != pool_descriptor_sets_map.end()) {
        for(auto descriptor_set : pool_descriptor_sets_map[descriptorPool]) {
            StartWriteObject(descriptor_set, "vkDestroyDescriptorPool");
        }
    }
}

void ThreadSafety::PostCallRecordDestroyDescriptorPool(
    VkDevice                                    device,
    VkDescriptorPool                            descriptorPool,
    const VkAllocationCallbacks*                pAllocator) {
    FinishReadObjectParentInstance(device, "vkDestroyDescriptorPool");
    FinishWriteObject(descriptorPool, "vkDestroyDescriptorPool");
    DestroyObject(descriptorPool);
    // Host access to descriptorPool must be externally synchronized
    {
        auto lock = WriteLockGuard(thread_safety_lock);
        // remove references to implicitly freed descriptor sets
        for(auto descriptor_set : pool_descriptor_sets_map[descriptorPool]) {
            FinishWriteObject(descriptor_set, "vkDestroyDescriptorPool");
            DestroyObject(descriptor_set);
            ds_read_only_map.erase(descriptor_set);
        }
        pool_descriptor_sets_map[descriptorPool].clear();
        pool_descriptor_sets_map.erase(descriptorPool);
    }
}

void ThreadSafety::PreCallRecordResetDescriptorPool(
    VkDevice                                    device,
    VkDescriptorPool                            descriptorPool,
    VkDescriptorPoolResetFlags                  flags) {
    StartReadObjectParentInstance(device, "vkResetDescriptorPool");
    StartWriteObject(descriptorPool, "vkResetDescriptorPool");
    // Host access to descriptorPool must be externally synchronized
    // any sname:VkDescriptorSet objects allocated from pname:descriptorPool must be externally synchronized between host accesses
    auto lock = ReadLockGuard(thread_safety_lock);
    auto iterator = pool_descriptor_sets_map.find(descriptorPool);
    // Possible to have no descriptor sets allocated from pool
    if (iterator != pool_descriptor_sets_map.end()) {
        for(auto descriptor_set : pool_descriptor_sets_map[descriptorPool]) {
            StartWriteObject(descriptor_set, "vkResetDescriptorPool");
        }
    }
}

void ThreadSafety::PostCallRecordResetDescriptorPool(
    VkDevice                                    device,
    VkDescriptorPool                            descriptorPool,
    VkDescriptorPoolResetFlags                  flags,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkResetDescriptorPool");
    FinishWriteObject(descriptorPool, "vkResetDescriptorPool");
    // Host access to descriptorPool must be externally synchronized
    // any sname:VkDescriptorSet objects allocated from pname:descriptorPool must be externally synchronized between host accesses
    if (VK_SUCCESS == result) {
        // remove references to implicitly freed descriptor sets
        auto lock = WriteLockGuard(thread_safety_lock);
        for(auto descriptor_set : pool_descriptor_sets_map[descriptorPool]) {
            FinishWriteObject(descriptor_set, "vkResetDescriptorPool");
            DestroyObject(descriptor_set);
            ds_read_only_map.erase(descriptor_set);
        }
        pool_descriptor_sets_map[descriptorPool].clear();
    }
}

bool ThreadSafety::DsReadOnly(VkDescriptorSet set) const
{
    auto iter = ds_read_only_map.find(set);
    if (iter != ds_read_only_map.end()) {
        return iter->second;
    }
    return false;
}

void ThreadSafety::PreCallRecordUpdateDescriptorSets(
    VkDevice                                    device,
    uint32_t                                    descriptorWriteCount,
    const VkWriteDescriptorSet*                 pDescriptorWrites,
    uint32_t                                    descriptorCopyCount,
    const VkCopyDescriptorSet*                  pDescriptorCopies) {
    StartReadObjectParentInstance(device, "vkUpdateDescriptorSets");
    if (pDescriptorWrites) {
        for (uint32_t index=0; index < descriptorWriteCount; index++) {
            auto dstSet = pDescriptorWrites[index].dstSet;
            bool read_only = DsReadOnly(dstSet);
            if (read_only) {
                StartReadObject(dstSet, "vkUpdateDescriptorSets");
            } else {
                StartWriteObject(dstSet, "vkUpdateDescriptorSets");
            }
        }
    }
    if (pDescriptorCopies) {
        for (uint32_t index=0; index < descriptorCopyCount; index++) {
            auto dstSet = pDescriptorCopies[index].dstSet;
            bool read_only = DsReadOnly(dstSet);
            if (read_only) {
                StartReadObject(dstSet, "vkUpdateDescriptorSets");
            } else {
                StartWriteObject(dstSet, "vkUpdateDescriptorSets");
            }
            StartReadObject(pDescriptorCopies[index].srcSet, "vkUpdateDescriptorSets");
        }
    }
    // Host access to pDescriptorWrites[].dstSet must be externally synchronized
    // Host access to pDescriptorCopies[].dstSet must be externally synchronized
}

void ThreadSafety::PostCallRecordUpdateDescriptorSets(
    VkDevice                                    device,
    uint32_t                                    descriptorWriteCount,
    const VkWriteDescriptorSet*                 pDescriptorWrites,
    uint32_t                                    descriptorCopyCount,
    const VkCopyDescriptorSet*                  pDescriptorCopies) {
    FinishReadObjectParentInstance(device, "vkUpdateDescriptorSets");
    if (pDescriptorWrites) {
        for (uint32_t index=0; index < descriptorWriteCount; index++) {
            auto dstSet = pDescriptorWrites[index].dstSet;
            bool read_only = DsReadOnly(dstSet);
            if (read_only) {
                FinishReadObject(dstSet, "vkUpdateDescriptorSets");
            } else {
                FinishWriteObject(dstSet, "vkUpdateDescriptorSets");
            }
        }
    }
    if (pDescriptorCopies) {
        for (uint32_t index=0; index < descriptorCopyCount; index++) {
            auto dstSet = pDescriptorCopies[index].dstSet;
            bool read_only = DsReadOnly(dstSet);
            if (read_only) {
                FinishReadObject(dstSet, "vkUpdateDescriptorSets");
            } else {
                FinishWriteObject(dstSet, "vkUpdateDescriptorSets");
            }
            FinishReadObject(pDescriptorCopies[index].srcSet, "vkUpdateDescriptorSets");
        }
    }
    // Host access to pDescriptorWrites[].dstSet must be externally synchronized
    // Host access to pDescriptorCopies[].dstSet must be externally synchronized
}

void ThreadSafety::PreCallRecordUpdateDescriptorSetWithTemplate(
    VkDevice                                    device,
    VkDescriptorSet                             descriptorSet,
    VkDescriptorUpdateTemplate                  descriptorUpdateTemplate,
    const void*                                 pData) {
    StartReadObjectParentInstance(device, "vkUpdateDescriptorSetWithTemplate");
    StartReadObject(descriptorUpdateTemplate, "vkUpdateDescriptorSetWithTemplate");

    const bool read_only = DsReadOnly(descriptorSet);
    if (read_only) {
        StartReadObject(descriptorSet, "vkUpdateDescriptorSetWithTemplate");
    } else {
        StartWriteObject(descriptorSet, "vkUpdateDescriptorSetWithTemplate");
    }
    // Host access to descriptorSet must be externally synchronized
}

void ThreadSafety::PostCallRecordUpdateDescriptorSetWithTemplate(
    VkDevice                                    device,
    VkDescriptorSet                             descriptorSet,
    VkDescriptorUpdateTemplate                  descriptorUpdateTemplate,
    const void*                                 pData) {
    FinishReadObjectParentInstance(device, "vkUpdateDescriptorSetWithTemplate");
    FinishReadObject(descriptorUpdateTemplate, "vkUpdateDescriptorSetWithTemplate");

    const bool read_only = DsReadOnly(descriptorSet);
    if (read_only) {
        FinishReadObject(descriptorSet, "vkUpdateDescriptorSetWithTemplate");
    } else {
        FinishWriteObject(descriptorSet, "vkUpdateDescriptorSetWithTemplate");
    }
    // Host access to descriptorSet must be externally synchronized
}

void ThreadSafety::PreCallRecordUpdateDescriptorSetWithTemplateKHR(
    VkDevice                                    device,
    VkDescriptorSet                             descriptorSet,
    VkDescriptorUpdateTemplate                  descriptorUpdateTemplate,
    const void*                                 pData) {
    StartReadObjectParentInstance(device, "vkUpdateDescriptorSetWithTemplateKHR");
    StartReadObject(descriptorUpdateTemplate, "vkUpdateDescriptorSetWithTemplateKHR");

    const bool read_only = DsReadOnly(descriptorSet);
    if (read_only) {
        StartReadObject(descriptorSet, "vkUpdateDescriptorSetWithTemplateKHR");
    } else {
        StartWriteObject(descriptorSet, "vkUpdateDescriptorSetWithTemplateKHR");
    }
    // Host access to descriptorSet must be externally synchronized
}

void ThreadSafety::PostCallRecordUpdateDescriptorSetWithTemplateKHR(
    VkDevice                                    device,
    VkDescriptorSet                             descriptorSet,
    VkDescriptorUpdateTemplate                  descriptorUpdateTemplate,
    const void*                                 pData) {
    FinishReadObjectParentInstance(device, "vkUpdateDescriptorSetWithTemplateKHR");
    FinishReadObject(descriptorUpdateTemplate, "vkUpdateDescriptorSetWithTemplateKHR");

    const bool read_only = DsReadOnly(descriptorSet);
    if (read_only) {
        FinishReadObject(descriptorSet, "vkUpdateDescriptorSetWithTemplateKHR");
    } else {
        FinishWriteObject(descriptorSet, "vkUpdateDescriptorSetWithTemplateKHR");
    }
    // Host access to descriptorSet must be externally synchronized
}

void ThreadSafety::PreCallRecordFreeCommandBuffers(VkDevice device, VkCommandPool commandPool, uint32_t commandBufferCount,
                                                   const VkCommandBuffer *pCommandBuffers) {
    const bool lockCommandPool = false;  // pool is already directly locked
    StartReadObjectParentInstance(device, "vkFreeCommandBuffers");
    StartWriteObject(commandPool, "vkFreeCommandBuffers");
    if(pCommandBuffers) {
        // Even though we're immediately "finishing" below, we still are testing for concurrency with any call in process
        // so this isn't a no-op
        // The driver may immediately reuse command buffers in another thread.
        // These updates need to be done before calling down to the driver.
        auto lock = WriteLockGuard(thread_safety_lock);
        auto &pool_command_buffers = pool_command_buffers_map[commandPool];
        for (uint32_t index = 0; index < commandBufferCount; index++) {
            StartWriteObject(pCommandBuffers[index], "vkFreeCommandBuffers", lockCommandPool);
            FinishWriteObject(pCommandBuffers[index], "vkFreeCommandBuffers", lockCommandPool);
            DestroyObject(pCommandBuffers[index]);
            pool_command_buffers.erase(pCommandBuffers[index]);
            command_pool_map.erase(pCommandBuffers[index]);
        }
    }
}

void ThreadSafety::PostCallRecordFreeCommandBuffers(VkDevice device, VkCommandPool commandPool, uint32_t commandBufferCount,
                                                    const VkCommandBuffer *pCommandBuffers) {
    FinishReadObjectParentInstance(device, "vkFreeCommandBuffers");
    FinishWriteObject(commandPool, "vkFreeCommandBuffers");
}

void ThreadSafety::PreCallRecordCreateCommandPool(
    VkDevice                                    device,
    const VkCommandPoolCreateInfo*              pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkCommandPool*                              pCommandPool) {
    StartReadObjectParentInstance(device, "vkCreateCommandPool");
}

void ThreadSafety::PostCallRecordCreateCommandPool(
    VkDevice                                    device,
    const VkCommandPoolCreateInfo*              pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkCommandPool*                              pCommandPool,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkCreateCommandPool");
    if (result == VK_SUCCESS) {
        CreateObject(*pCommandPool);
        c_VkCommandPoolContents.CreateObject(*pCommandPool);
    }
}

void ThreadSafety::PreCallRecordResetCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolResetFlags flags) {
    StartReadObjectParentInstance(device, "vkResetCommandPool");
    StartWriteObject(commandPool, "vkResetCommandPool");
    // Check for any uses of non-externally sync'd command buffers (for example from vkCmdExecuteCommands)
    c_VkCommandPoolContents.StartWrite(commandPool, "vkResetCommandPool");
    // Host access to commandPool must be externally synchronized
}

void ThreadSafety::PostCallRecordResetCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolResetFlags flags, VkResult result) {
    FinishReadObjectParentInstance(device, "vkResetCommandPool");
    FinishWriteObject(commandPool, "vkResetCommandPool");
    c_VkCommandPoolContents.FinishWrite(commandPool, "vkResetCommandPool");
    // Host access to commandPool must be externally synchronized
}

void ThreadSafety::PreCallRecordDestroyCommandPool(VkDevice device, VkCommandPool commandPool, const VkAllocationCallbacks *pAllocator) {
    StartReadObjectParentInstance(device, "vkDestroyCommandPool");
    StartWriteObject(commandPool, "vkDestroyCommandPool");
    // Check for any uses of non-externally sync'd command buffers (for example from vkCmdExecuteCommands)
    c_VkCommandPoolContents.StartWrite(commandPool, "vkDestroyCommandPool");
    // Host access to commandPool must be externally synchronized

    auto lock = WriteLockGuard(thread_safety_lock);
    // The driver may immediately reuse command buffers in another thread.
    // These updates need to be done before calling down to the driver.
    // remove references to implicitly freed command pools
    for(auto command_buffer : pool_command_buffers_map[commandPool]) {
        DestroyObject(command_buffer);
    }
    pool_command_buffers_map[commandPool].clear();
    pool_command_buffers_map.erase(commandPool);
}

void ThreadSafety::PostCallRecordDestroyCommandPool(VkDevice device, VkCommandPool commandPool, const VkAllocationCallbacks *pAllocator) {
    FinishReadObjectParentInstance(device, "vkDestroyCommandPool");
    FinishWriteObject(commandPool, "vkDestroyCommandPool");
    DestroyObject(commandPool);
    c_VkCommandPoolContents.FinishWrite(commandPool, "vkDestroyCommandPool");
    c_VkCommandPoolContents.DestroyObject(commandPool);
}

// GetSwapchainImages can return a non-zero count with a NULL pSwapchainImages pointer.  Let's avoid crashes by ignoring
// pSwapchainImages.
void ThreadSafety::PreCallRecordGetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapchain, uint32_t *pSwapchainImageCount,
                                                      VkImage *pSwapchainImages) {
    StartReadObjectParentInstance(device, "vkGetSwapchainImagesKHR");
    StartReadObjectParentInstance(swapchain, "vkGetSwapchainImagesKHR");
}

void ThreadSafety::PostCallRecordGetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapchain, uint32_t *pSwapchainImageCount,
                                                       VkImage *pSwapchainImages, VkResult result) {
    FinishReadObjectParentInstance(device, "vkGetSwapchainImagesKHR");
    FinishReadObjectParentInstance(swapchain, "vkGetSwapchainImagesKHR");
    if (pSwapchainImages != nullptr) {
        auto lock = WriteLockGuard(thread_safety_lock);
        auto &wrapped_swapchain_image_handles = swapchain_wrapped_image_handle_map[swapchain];
        for (uint32_t i = static_cast<uint32_t>(wrapped_swapchain_image_handles.size()); i < *pSwapchainImageCount; i++) {
            CreateObject(pSwapchainImages[i]);
            wrapped_swapchain_image_handles.emplace_back(pSwapchainImages[i]);
        }
    }
}

void ThreadSafety::PreCallRecordDestroySwapchainKHR(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    const VkAllocationCallbacks*                pAllocator) {
    StartReadObjectParentInstance(device, "vkDestroySwapchainKHR");
    StartWriteObjectParentInstance(swapchain, "vkDestroySwapchainKHR");
    // Host access to swapchain must be externally synchronized
    auto lock = ReadLockGuard(thread_safety_lock);
    for (auto &image_handle : swapchain_wrapped_image_handle_map[swapchain]) {
        StartWriteObject(image_handle, "vkDestroySwapchainKHR");
    }
}

void ThreadSafety::PostCallRecordDestroySwapchainKHR(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    const VkAllocationCallbacks*                pAllocator) {
    FinishReadObjectParentInstance(device, "vkDestroySwapchainKHR");
    FinishWriteObjectParentInstance(swapchain, "vkDestroySwapchainKHR");
    DestroyObjectParentInstance(swapchain);
    // Host access to swapchain must be externally synchronized
    auto lock = WriteLockGuard(thread_safety_lock);
    for (auto &image_handle : swapchain_wrapped_image_handle_map[swapchain]) {
        FinishWriteObject(image_handle, "vkDestroySwapchainKHR");
        DestroyObject(image_handle);
    }
    swapchain_wrapped_image_handle_map.erase(swapchain);
}

void ThreadSafety::PreCallRecordDestroyDevice(
    VkDevice                                    device,
    const VkAllocationCallbacks*                pAllocator) {
    StartWriteObjectParentInstance(device, "vkDestroyDevice");
    // Host access to device must be externally synchronized
}

void ThreadSafety::PostCallRecordDestroyDevice(
    VkDevice                                    device,
    const VkAllocationCallbacks*                pAllocator) {
    FinishWriteObjectParentInstance(device, "vkDestroyDevice");
    DestroyObjectParentInstance(device);
    // Host access to device must be externally synchronized
    auto lock = WriteLockGuard(thread_safety_lock);
    for (auto &queue : device_queues_map[device]) {
        DestroyObject(queue);
    }
    device_queues_map[device].clear();
}

void ThreadSafety::PreCallRecordGetDeviceQueue(
    VkDevice                                    device,
    uint32_t                                    queueFamilyIndex,
    uint32_t                                    queueIndex,
    VkQueue*                                    pQueue) {
    StartReadObjectParentInstance(device, "vkGetDeviceQueue");
}

void ThreadSafety::PostCallRecordGetDeviceQueue(
    VkDevice                                    device,
    uint32_t                                    queueFamilyIndex,
    uint32_t                                    queueIndex,
    VkQueue*                                    pQueue) {
    FinishReadObjectParentInstance(device, "vkGetDeviceQueue");
    CreateObject(*pQueue);
    auto lock = WriteLockGuard(thread_safety_lock);
    device_queues_map[device].insert(*pQueue);
}

void ThreadSafety::PreCallRecordGetDeviceQueue2(
    VkDevice                                    device,
    const VkDeviceQueueInfo2*                   pQueueInfo,
    VkQueue*                                    pQueue) {
    StartReadObjectParentInstance(device, "vkGetDeviceQueue2");
}

void ThreadSafety::PostCallRecordGetDeviceQueue2(
    VkDevice                                    device,
    const VkDeviceQueueInfo2*                   pQueueInfo,
    VkQueue*                                    pQueue) {
    FinishReadObjectParentInstance(device, "vkGetDeviceQueue2");
    CreateObject(*pQueue);
    auto lock = WriteLockGuard(thread_safety_lock);
    device_queues_map[device].insert(*pQueue);
}

void ThreadSafety::PostCallRecordGetPhysicalDeviceDisplayPropertiesKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pPropertyCount,
    VkDisplayPropertiesKHR*                     pProperties,
    VkResult                                    result) {
    if ((result != VK_SUCCESS) && (result != VK_INCOMPLETE)) return;
    if (pProperties) {
        for (uint32_t i = 0; i < *pPropertyCount; ++i) {
            CreateObjectParentInstance(pProperties[i].display);
        }
    }
}

void ThreadSafety::PostCallRecordGetPhysicalDeviceDisplayProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pPropertyCount,
    VkDisplayProperties2KHR*                    pProperties,
    VkResult                                    result) {
    if ((result != VK_SUCCESS) && (result != VK_INCOMPLETE)) return;
    if (pProperties) {
        for (uint32_t i = 0; i < *pPropertyCount; ++i) {
            CreateObjectParentInstance(pProperties[i].displayProperties.display);
        }
    }
}

void ThreadSafety::PostCallRecordGetPhysicalDeviceDisplayPlanePropertiesKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pPropertyCount,
    VkDisplayPlanePropertiesKHR*                pProperties,
    VkResult                                    result) {
    if ((result != VK_SUCCESS) && (result != VK_INCOMPLETE)) return;
    if (pProperties) {
        for (uint32_t i = 0; i < *pPropertyCount; ++i) {
            CreateObjectParentInstance(pProperties[i].currentDisplay);
        }
    }
}

void ThreadSafety::PostCallRecordGetPhysicalDeviceDisplayPlaneProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pPropertyCount,
    VkDisplayPlaneProperties2KHR*               pProperties,
    VkResult                                    result) {
    if ((result != VK_SUCCESS) && (result != VK_INCOMPLETE)) return;
    if (pProperties) {
        for (uint32_t i = 0; i < *pPropertyCount; ++i) {
            CreateObjectParentInstance(pProperties[i].displayPlaneProperties.currentDisplay);
        }
    }
}

void ThreadSafety::PreCallRecordGetDisplayPlaneSupportedDisplaysKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    planeIndex,
    uint32_t*                                   pDisplayCount,
    VkDisplayKHR*                               pDisplays) {
    // Nothing to do for this pre-call function
}

void ThreadSafety::PostCallRecordGetDisplayPlaneSupportedDisplaysKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    planeIndex,
    uint32_t*                                   pDisplayCount,
    VkDisplayKHR*                               pDisplays,
    VkResult                                    result) {
    if ((result != VK_SUCCESS) && (result != VK_INCOMPLETE)) return;
    if (pDisplays) {
        for (uint32_t index = 0; index < *pDisplayCount; index++) {
            CreateObjectParentInstance(pDisplays[index]);
        }
    }
}

void ThreadSafety::PreCallRecordGetDisplayModePropertiesKHR(
    VkPhysicalDevice                            physicalDevice,
    VkDisplayKHR                                display,
    uint32_t*                                   pPropertyCount,
    VkDisplayModePropertiesKHR*                 pProperties) {
    StartReadObjectParentInstance(display, "vkGetDisplayModePropertiesKHR");
}

void ThreadSafety::PostCallRecordGetDisplayModePropertiesKHR(
    VkPhysicalDevice                            physicalDevice,
    VkDisplayKHR                                display,
    uint32_t*                                   pPropertyCount,
    VkDisplayModePropertiesKHR*                 pProperties,
    VkResult                                    result) {
    FinishReadObjectParentInstance(display, "vkGetDisplayModePropertiesKHR");
    if ((result != VK_SUCCESS) && (result != VK_INCOMPLETE)) return;
    if (pProperties != nullptr) {
        for (uint32_t index = 0; index < *pPropertyCount; index++) {
            CreateObject(pProperties[index].displayMode);
        }
    }
}

void ThreadSafety::PreCallRecordGetDisplayModeProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    VkDisplayKHR                                display,
    uint32_t*                                   pPropertyCount,
    VkDisplayModeProperties2KHR*                pProperties) {
    StartReadObjectParentInstance(display, "vkGetDisplayModeProperties2KHR");
}

void ThreadSafety::PostCallRecordGetDisplayModeProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    VkDisplayKHR                                display,
    uint32_t*                                   pPropertyCount,
    VkDisplayModeProperties2KHR*                pProperties,
    VkResult                                    result) {
    FinishReadObjectParentInstance(display, "vkGetDisplayModeProperties2KHR");
    if ((result != VK_SUCCESS) && (result != VK_INCOMPLETE)) return;
    if (pProperties != nullptr) {
        for (uint32_t index = 0; index < *pPropertyCount; index++) {
            CreateObject(pProperties[index].displayModeProperties.displayMode);
        }
    }
}

void ThreadSafety::PreCallRecordGetDisplayPlaneCapabilities2KHR(
    VkPhysicalDevice                            physicalDevice,
    const VkDisplayPlaneInfo2KHR*               pDisplayPlaneInfo,
    VkDisplayPlaneCapabilities2KHR*             pCapabilities) {
    StartWriteObject(pDisplayPlaneInfo->mode, "vkGetDisplayPlaneCapabilities2KHR");
}

void ThreadSafety::PostCallRecordGetDisplayPlaneCapabilities2KHR(
    VkPhysicalDevice                            physicalDevice,
    const VkDisplayPlaneInfo2KHR*               pDisplayPlaneInfo,
    VkDisplayPlaneCapabilities2KHR*             pCapabilities,
    VkResult                                    result) {
    FinishWriteObject(pDisplayPlaneInfo->mode, "vkGetDisplayPlaneCapabilities2KHR");
}

#ifdef VK_USE_PLATFORM_XLIB_XRANDR_EXT

void ThreadSafety::PostCallRecordGetRandROutputDisplayEXT(
    VkPhysicalDevice                            physicalDevice,
    Display*                                    dpy,
    RROutput                                    rrOutput,
    VkDisplayKHR*                               pDisplay,
    VkResult                                    result) {
    if ((result != VK_SUCCESS) || (pDisplay == nullptr)) return;
    CreateObjectParentInstance(*pDisplay);
}

#endif // VK_USE_PLATFORM_XLIB_XRANDR_EXT

void ThreadSafety::PostCallRecordGetDrmDisplayEXT(
    VkPhysicalDevice                            physicalDevice,
    int32_t                                     drmFd,
    uint32_t                                    connectorId,
    VkDisplayKHR*                               display,
    VkResult                                    result) {
    if ((result != VK_SUCCESS) || (display == nullptr)) return;
    CreateObjectParentInstance(*display);
}

void ThreadSafety::PreCallRecordRegisterDisplayEventEXT(
    VkDevice                                    device,
    VkDisplayKHR                                display,
    const VkDisplayEventInfoEXT*                pDisplayEventInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkFence*                                    pFence) {
    StartReadObjectParentInstance(device, "vkRegisterDisplayEventEXT");
    StartReadObjectParentInstance(display, "vkRegisterDisplayEventEXT");
}

void ThreadSafety::PostCallRecordRegisterDisplayEventEXT(
    VkDevice                                    device,
    VkDisplayKHR                                display,
    const VkDisplayEventInfoEXT*                pDisplayEventInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkFence*                                    pFence,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkRegisterDisplayEventEXT");
    FinishReadObjectParentInstance(display, "vkRegisterDisplayEventEXT");
    if (result == VK_SUCCESS) {
        CreateObject(*pFence);
    }
}

void ThreadSafety::PreCallRecordDeviceWaitIdle(
    VkDevice                                    device) {
    StartReadObjectParentInstance(device, "vkDeviceWaitIdle");
    auto lock = ReadLockGuard(thread_safety_lock);
    const auto &queue_set = device_queues_map[device];
    for (const auto &queue : queue_set) {
        StartWriteObject(queue, "vkDeviceWaitIdle");
    }
}

void ThreadSafety::PostCallRecordDeviceWaitIdle(
    VkDevice                                    device,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkDeviceWaitIdle");
    auto lock = ReadLockGuard(thread_safety_lock);
    const auto &queue_set = device_queues_map[device];
    for (const auto &queue : queue_set) {
        FinishWriteObject(queue, "vkDeviceWaitIdle");
    }
}

void ThreadSafety::PreCallRecordCreateRayTracingPipelinesKHR(
    VkDevice                                    device,
    VkDeferredOperationKHR                      deferredOperation,
    VkPipelineCache                             pipelineCache,
    uint32_t                                    createInfoCount,
    const VkRayTracingPipelineCreateInfoKHR*    pCreateInfos,
    const VkAllocationCallbacks*                pAllocator,
    VkPipeline*                                 pPipelines) {
    StartReadObjectParentInstance(device, "vkCreateRayTracingPipelinesKHR");
    StartReadObject(deferredOperation, "vkCreateRayTracingPipelinesKHR");
    StartReadObject(pipelineCache, "vkCreateRayTracingPipelinesKHR");
}

void ThreadSafety::PostCallRecordCreateRayTracingPipelinesKHR(
    VkDevice                                    device,
    VkDeferredOperationKHR                      deferredOperation,
    VkPipelineCache                             pipelineCache,
    uint32_t                                    createInfoCount,
    const VkRayTracingPipelineCreateInfoKHR*    pCreateInfos,
    const VkAllocationCallbacks*                pAllocator,
    VkPipeline*                                 pPipelines,
    VkResult                                    result) {
    auto unlock_objects = [this, device, deferredOperation, pipelineCache]() {
        this->FinishReadObjectParentInstance(device, "vkCreateRayTracingPipelinesKHR");
        this->FinishReadObject(deferredOperation, "vkCreateRayTracingPipelinesKHR");
        this->FinishReadObject(pipelineCache, "vkCreateRayTracingPipelinesKHR");
    };

    auto register_objects = [this](const std::vector<VkPipeline>& pipelines) {
        for(auto pipe : pipelines) {
            if (!pipe) continue;
            CreateObject(pipe);
        }
    };

    const bool is_operation_deferred = (deferredOperation != VK_NULL_HANDLE && result == VK_OPERATION_DEFERRED_KHR);
    if (is_operation_deferred) {
        auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
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
        std::vector<std::function<void(const std::vector<VkPipeline> &)>> post_check_fns;
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


void ThreadSafety::PreCallRecordCreateInstance(
    const VkInstanceCreateInfo*                 pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkInstance*                                 pInstance) {
    
}

void ThreadSafety::PostCallRecordCreateInstance(
    const VkInstanceCreateInfo*                 pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkInstance*                                 pInstance,
    VkResult                                    result) {
    if (result == VK_SUCCESS) {
        CreateObjectParentInstance(*pInstance);
    }
}

void ThreadSafety::PreCallRecordDestroyInstance(
    VkInstance                                  instance,
    const VkAllocationCallbacks*                pAllocator) {
    StartWriteObjectParentInstance(instance, "vkDestroyInstance");
    // Host access to instance must be externally synchronized
    // all sname:VkPhysicalDevice objects enumerated from pname:instance must be externally synchronized between host accesses
}

void ThreadSafety::PostCallRecordDestroyInstance(
    VkInstance                                  instance,
    const VkAllocationCallbacks*                pAllocator) {
    FinishWriteObjectParentInstance(instance, "vkDestroyInstance");
    DestroyObjectParentInstance(instance);
    // Host access to instance must be externally synchronized
    // all sname:VkPhysicalDevice objects enumerated from pname:instance must be externally synchronized between host accesses
}

void ThreadSafety::PreCallRecordEnumeratePhysicalDevices(
    VkInstance                                  instance,
    uint32_t*                                   pPhysicalDeviceCount,
    VkPhysicalDevice*                           pPhysicalDevices) {
    StartReadObjectParentInstance(instance, "vkEnumeratePhysicalDevices");
}

void ThreadSafety::PostCallRecordEnumeratePhysicalDevices(
    VkInstance                                  instance,
    uint32_t*                                   pPhysicalDeviceCount,
    VkPhysicalDevice*                           pPhysicalDevices,
    VkResult                                    result) {
    FinishReadObjectParentInstance(instance, "vkEnumeratePhysicalDevices");
}

void ThreadSafety::PreCallRecordGetInstanceProcAddr(
    VkInstance                                  instance,
    const char*                                 pName) {
    StartReadObjectParentInstance(instance, "vkGetInstanceProcAddr");
}

void ThreadSafety::PostCallRecordGetInstanceProcAddr(
    VkInstance                                  instance,
    const char*                                 pName) {
    FinishReadObjectParentInstance(instance, "vkGetInstanceProcAddr");
}

void ThreadSafety::PreCallRecordGetDeviceProcAddr(
    VkDevice                                    device,
    const char*                                 pName) {
    StartReadObjectParentInstance(device, "vkGetDeviceProcAddr");
}

void ThreadSafety::PostCallRecordGetDeviceProcAddr(
    VkDevice                                    device,
    const char*                                 pName) {
    FinishReadObjectParentInstance(device, "vkGetDeviceProcAddr");
}

void ThreadSafety::PreCallRecordCreateDevice(
    VkPhysicalDevice                            physicalDevice,
    const VkDeviceCreateInfo*                   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDevice*                                   pDevice) {
    
}

void ThreadSafety::PostCallRecordCreateDevice(
    VkPhysicalDevice                            physicalDevice,
    const VkDeviceCreateInfo*                   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDevice*                                   pDevice,
    VkResult                                    result) {
    if (result == VK_SUCCESS) {
        CreateObjectParentInstance(*pDevice);
    }
}

void ThreadSafety::PreCallRecordQueueSubmit(
    VkQueue                                     queue,
    uint32_t                                    submitCount,
    const VkSubmitInfo*                         pSubmits,
    VkFence                                     fence) {
    StartWriteObject(queue, "vkQueueSubmit");
    StartWriteObject(fence, "vkQueueSubmit");
    // Host access to queue must be externally synchronized
    // Host access to fence must be externally synchronized
}

void ThreadSafety::PostCallRecordQueueSubmit(
    VkQueue                                     queue,
    uint32_t                                    submitCount,
    const VkSubmitInfo*                         pSubmits,
    VkFence                                     fence,
    VkResult                                    result) {
    FinishWriteObject(queue, "vkQueueSubmit");
    FinishWriteObject(fence, "vkQueueSubmit");
    // Host access to queue must be externally synchronized
    // Host access to fence must be externally synchronized
}

void ThreadSafety::PreCallRecordQueueWaitIdle(
    VkQueue                                     queue) {
    StartWriteObject(queue, "vkQueueWaitIdle");
    // Host access to queue must be externally synchronized
}

void ThreadSafety::PostCallRecordQueueWaitIdle(
    VkQueue                                     queue,
    VkResult                                    result) {
    FinishWriteObject(queue, "vkQueueWaitIdle");
    // Host access to queue must be externally synchronized
}

void ThreadSafety::PreCallRecordAllocateMemory(
    VkDevice                                    device,
    const VkMemoryAllocateInfo*                 pAllocateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDeviceMemory*                             pMemory) {
    StartReadObjectParentInstance(device, "vkAllocateMemory");
}

void ThreadSafety::PostCallRecordAllocateMemory(
    VkDevice                                    device,
    const VkMemoryAllocateInfo*                 pAllocateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDeviceMemory*                             pMemory,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkAllocateMemory");
    if (result == VK_SUCCESS) {
        CreateObject(*pMemory);
    }
}

void ThreadSafety::PreCallRecordFreeMemory(
    VkDevice                                    device,
    VkDeviceMemory                              memory,
    const VkAllocationCallbacks*                pAllocator) {
    StartReadObjectParentInstance(device, "vkFreeMemory");
    StartWriteObject(memory, "vkFreeMemory");
    // Host access to memory must be externally synchronized
}

void ThreadSafety::PostCallRecordFreeMemory(
    VkDevice                                    device,
    VkDeviceMemory                              memory,
    const VkAllocationCallbacks*                pAllocator) {
    FinishReadObjectParentInstance(device, "vkFreeMemory");
    FinishWriteObject(memory, "vkFreeMemory");
    DestroyObject(memory);
    // Host access to memory must be externally synchronized
}

void ThreadSafety::PreCallRecordMapMemory(
    VkDevice                                    device,
    VkDeviceMemory                              memory,
    VkDeviceSize                                offset,
    VkDeviceSize                                size,
    VkMemoryMapFlags                            flags,
    void**                                      ppData) {
    StartReadObjectParentInstance(device, "vkMapMemory");
    StartWriteObject(memory, "vkMapMemory");
    // Host access to memory must be externally synchronized
}

void ThreadSafety::PostCallRecordMapMemory(
    VkDevice                                    device,
    VkDeviceMemory                              memory,
    VkDeviceSize                                offset,
    VkDeviceSize                                size,
    VkMemoryMapFlags                            flags,
    void**                                      ppData,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkMapMemory");
    FinishWriteObject(memory, "vkMapMemory");
    // Host access to memory must be externally synchronized
}

void ThreadSafety::PreCallRecordUnmapMemory(
    VkDevice                                    device,
    VkDeviceMemory                              memory) {
    StartReadObjectParentInstance(device, "vkUnmapMemory");
    StartWriteObject(memory, "vkUnmapMemory");
    // Host access to memory must be externally synchronized
}

void ThreadSafety::PostCallRecordUnmapMemory(
    VkDevice                                    device,
    VkDeviceMemory                              memory) {
    FinishReadObjectParentInstance(device, "vkUnmapMemory");
    FinishWriteObject(memory, "vkUnmapMemory");
    // Host access to memory must be externally synchronized
}

void ThreadSafety::PreCallRecordFlushMappedMemoryRanges(
    VkDevice                                    device,
    uint32_t                                    memoryRangeCount,
    const VkMappedMemoryRange*                  pMemoryRanges) {
    StartReadObjectParentInstance(device, "vkFlushMappedMemoryRanges");
}

void ThreadSafety::PostCallRecordFlushMappedMemoryRanges(
    VkDevice                                    device,
    uint32_t                                    memoryRangeCount,
    const VkMappedMemoryRange*                  pMemoryRanges,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkFlushMappedMemoryRanges");
}

void ThreadSafety::PreCallRecordInvalidateMappedMemoryRanges(
    VkDevice                                    device,
    uint32_t                                    memoryRangeCount,
    const VkMappedMemoryRange*                  pMemoryRanges) {
    StartReadObjectParentInstance(device, "vkInvalidateMappedMemoryRanges");
}

void ThreadSafety::PostCallRecordInvalidateMappedMemoryRanges(
    VkDevice                                    device,
    uint32_t                                    memoryRangeCount,
    const VkMappedMemoryRange*                  pMemoryRanges,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkInvalidateMappedMemoryRanges");
}

void ThreadSafety::PreCallRecordGetDeviceMemoryCommitment(
    VkDevice                                    device,
    VkDeviceMemory                              memory,
    VkDeviceSize*                               pCommittedMemoryInBytes) {
    StartReadObjectParentInstance(device, "vkGetDeviceMemoryCommitment");
    StartReadObject(memory, "vkGetDeviceMemoryCommitment");
}

void ThreadSafety::PostCallRecordGetDeviceMemoryCommitment(
    VkDevice                                    device,
    VkDeviceMemory                              memory,
    VkDeviceSize*                               pCommittedMemoryInBytes) {
    FinishReadObjectParentInstance(device, "vkGetDeviceMemoryCommitment");
    FinishReadObject(memory, "vkGetDeviceMemoryCommitment");
}

void ThreadSafety::PreCallRecordBindBufferMemory(
    VkDevice                                    device,
    VkBuffer                                    buffer,
    VkDeviceMemory                              memory,
    VkDeviceSize                                memoryOffset) {
    StartReadObjectParentInstance(device, "vkBindBufferMemory");
    StartWriteObject(buffer, "vkBindBufferMemory");
    StartReadObject(memory, "vkBindBufferMemory");
    // Host access to buffer must be externally synchronized
}

void ThreadSafety::PostCallRecordBindBufferMemory(
    VkDevice                                    device,
    VkBuffer                                    buffer,
    VkDeviceMemory                              memory,
    VkDeviceSize                                memoryOffset,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkBindBufferMemory");
    FinishWriteObject(buffer, "vkBindBufferMemory");
    FinishReadObject(memory, "vkBindBufferMemory");
    // Host access to buffer must be externally synchronized
}

void ThreadSafety::PreCallRecordBindImageMemory(
    VkDevice                                    device,
    VkImage                                     image,
    VkDeviceMemory                              memory,
    VkDeviceSize                                memoryOffset) {
    StartReadObjectParentInstance(device, "vkBindImageMemory");
    StartWriteObject(image, "vkBindImageMemory");
    StartReadObject(memory, "vkBindImageMemory");
    // Host access to image must be externally synchronized
}

void ThreadSafety::PostCallRecordBindImageMemory(
    VkDevice                                    device,
    VkImage                                     image,
    VkDeviceMemory                              memory,
    VkDeviceSize                                memoryOffset,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkBindImageMemory");
    FinishWriteObject(image, "vkBindImageMemory");
    FinishReadObject(memory, "vkBindImageMemory");
    // Host access to image must be externally synchronized
}

void ThreadSafety::PreCallRecordGetBufferMemoryRequirements(
    VkDevice                                    device,
    VkBuffer                                    buffer,
    VkMemoryRequirements*                       pMemoryRequirements) {
    StartReadObjectParentInstance(device, "vkGetBufferMemoryRequirements");
    StartReadObject(buffer, "vkGetBufferMemoryRequirements");
}

void ThreadSafety::PostCallRecordGetBufferMemoryRequirements(
    VkDevice                                    device,
    VkBuffer                                    buffer,
    VkMemoryRequirements*                       pMemoryRequirements) {
    FinishReadObjectParentInstance(device, "vkGetBufferMemoryRequirements");
    FinishReadObject(buffer, "vkGetBufferMemoryRequirements");
}

void ThreadSafety::PreCallRecordGetImageMemoryRequirements(
    VkDevice                                    device,
    VkImage                                     image,
    VkMemoryRequirements*                       pMemoryRequirements) {
    StartReadObjectParentInstance(device, "vkGetImageMemoryRequirements");
    StartReadObject(image, "vkGetImageMemoryRequirements");
}

void ThreadSafety::PostCallRecordGetImageMemoryRequirements(
    VkDevice                                    device,
    VkImage                                     image,
    VkMemoryRequirements*                       pMemoryRequirements) {
    FinishReadObjectParentInstance(device, "vkGetImageMemoryRequirements");
    FinishReadObject(image, "vkGetImageMemoryRequirements");
}

void ThreadSafety::PreCallRecordGetImageSparseMemoryRequirements(
    VkDevice                                    device,
    VkImage                                     image,
    uint32_t*                                   pSparseMemoryRequirementCount,
    VkSparseImageMemoryRequirements*            pSparseMemoryRequirements) {
    StartReadObjectParentInstance(device, "vkGetImageSparseMemoryRequirements");
    StartReadObject(image, "vkGetImageSparseMemoryRequirements");
}

void ThreadSafety::PostCallRecordGetImageSparseMemoryRequirements(
    VkDevice                                    device,
    VkImage                                     image,
    uint32_t*                                   pSparseMemoryRequirementCount,
    VkSparseImageMemoryRequirements*            pSparseMemoryRequirements) {
    FinishReadObjectParentInstance(device, "vkGetImageSparseMemoryRequirements");
    FinishReadObject(image, "vkGetImageSparseMemoryRequirements");
}

void ThreadSafety::PreCallRecordQueueBindSparse(
    VkQueue                                     queue,
    uint32_t                                    bindInfoCount,
    const VkBindSparseInfo*                     pBindInfo,
    VkFence                                     fence) {
    StartWriteObject(queue, "vkQueueBindSparse");
    StartWriteObject(fence, "vkQueueBindSparse");
    // Host access to queue must be externally synchronized
    // Host access to fence must be externally synchronized
}

void ThreadSafety::PostCallRecordQueueBindSparse(
    VkQueue                                     queue,
    uint32_t                                    bindInfoCount,
    const VkBindSparseInfo*                     pBindInfo,
    VkFence                                     fence,
    VkResult                                    result) {
    FinishWriteObject(queue, "vkQueueBindSparse");
    FinishWriteObject(fence, "vkQueueBindSparse");
    // Host access to queue must be externally synchronized
    // Host access to fence must be externally synchronized
}

void ThreadSafety::PreCallRecordCreateFence(
    VkDevice                                    device,
    const VkFenceCreateInfo*                    pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkFence*                                    pFence) {
    StartReadObjectParentInstance(device, "vkCreateFence");
}

void ThreadSafety::PostCallRecordCreateFence(
    VkDevice                                    device,
    const VkFenceCreateInfo*                    pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkFence*                                    pFence,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkCreateFence");
    if (result == VK_SUCCESS) {
        CreateObject(*pFence);
    }
}

void ThreadSafety::PreCallRecordDestroyFence(
    VkDevice                                    device,
    VkFence                                     fence,
    const VkAllocationCallbacks*                pAllocator) {
    StartReadObjectParentInstance(device, "vkDestroyFence");
    StartWriteObject(fence, "vkDestroyFence");
    // Host access to fence must be externally synchronized
}

void ThreadSafety::PostCallRecordDestroyFence(
    VkDevice                                    device,
    VkFence                                     fence,
    const VkAllocationCallbacks*                pAllocator) {
    FinishReadObjectParentInstance(device, "vkDestroyFence");
    FinishWriteObject(fence, "vkDestroyFence");
    DestroyObject(fence);
    // Host access to fence must be externally synchronized
}

void ThreadSafety::PreCallRecordResetFences(
    VkDevice                                    device,
    uint32_t                                    fenceCount,
    const VkFence*                              pFences) {
    StartReadObjectParentInstance(device, "vkResetFences");
    if (pFences) {
        for (uint32_t index=0; index < fenceCount; index++) {
            StartWriteObject(pFences[index], "vkResetFences");
        }
    }
    // Host access to each member of pFences must be externally synchronized
}

void ThreadSafety::PostCallRecordResetFences(
    VkDevice                                    device,
    uint32_t                                    fenceCount,
    const VkFence*                              pFences,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkResetFences");
    if (pFences) {
        for (uint32_t index=0; index < fenceCount; index++) {
            FinishWriteObject(pFences[index], "vkResetFences");
        }
    }
    // Host access to each member of pFences must be externally synchronized
}

void ThreadSafety::PreCallRecordGetFenceStatus(
    VkDevice                                    device,
    VkFence                                     fence) {
    StartReadObjectParentInstance(device, "vkGetFenceStatus");
    StartReadObject(fence, "vkGetFenceStatus");
}

void ThreadSafety::PostCallRecordGetFenceStatus(
    VkDevice                                    device,
    VkFence                                     fence,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkGetFenceStatus");
    FinishReadObject(fence, "vkGetFenceStatus");
}

void ThreadSafety::PreCallRecordWaitForFences(
    VkDevice                                    device,
    uint32_t                                    fenceCount,
    const VkFence*                              pFences,
    VkBool32                                    waitAll,
    uint64_t                                    timeout) {
    StartReadObjectParentInstance(device, "vkWaitForFences");
    if (pFences) {
        for (uint32_t index = 0; index < fenceCount; index++) {
            StartReadObject(pFences[index], "vkWaitForFences");
        }
    }
}

void ThreadSafety::PostCallRecordWaitForFences(
    VkDevice                                    device,
    uint32_t                                    fenceCount,
    const VkFence*                              pFences,
    VkBool32                                    waitAll,
    uint64_t                                    timeout,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkWaitForFences");
    if (pFences) {
        for (uint32_t index = 0; index < fenceCount; index++) {
            FinishReadObject(pFences[index], "vkWaitForFences");
        }
    }
}

void ThreadSafety::PreCallRecordCreateSemaphore(
    VkDevice                                    device,
    const VkSemaphoreCreateInfo*                pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSemaphore*                                pSemaphore) {
    StartReadObjectParentInstance(device, "vkCreateSemaphore");
}

void ThreadSafety::PostCallRecordCreateSemaphore(
    VkDevice                                    device,
    const VkSemaphoreCreateInfo*                pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSemaphore*                                pSemaphore,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkCreateSemaphore");
    if (result == VK_SUCCESS) {
        CreateObject(*pSemaphore);
    }
}

void ThreadSafety::PreCallRecordDestroySemaphore(
    VkDevice                                    device,
    VkSemaphore                                 semaphore,
    const VkAllocationCallbacks*                pAllocator) {
    StartReadObjectParentInstance(device, "vkDestroySemaphore");
    StartWriteObject(semaphore, "vkDestroySemaphore");
    // Host access to semaphore must be externally synchronized
}

void ThreadSafety::PostCallRecordDestroySemaphore(
    VkDevice                                    device,
    VkSemaphore                                 semaphore,
    const VkAllocationCallbacks*                pAllocator) {
    FinishReadObjectParentInstance(device, "vkDestroySemaphore");
    FinishWriteObject(semaphore, "vkDestroySemaphore");
    DestroyObject(semaphore);
    // Host access to semaphore must be externally synchronized
}

void ThreadSafety::PreCallRecordCreateEvent(
    VkDevice                                    device,
    const VkEventCreateInfo*                    pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkEvent*                                    pEvent) {
    StartReadObjectParentInstance(device, "vkCreateEvent");
}

void ThreadSafety::PostCallRecordCreateEvent(
    VkDevice                                    device,
    const VkEventCreateInfo*                    pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkEvent*                                    pEvent,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkCreateEvent");
    if (result == VK_SUCCESS) {
        CreateObject(*pEvent);
    }
}

void ThreadSafety::PreCallRecordDestroyEvent(
    VkDevice                                    device,
    VkEvent                                     event,
    const VkAllocationCallbacks*                pAllocator) {
    StartReadObjectParentInstance(device, "vkDestroyEvent");
    StartWriteObject(event, "vkDestroyEvent");
    // Host access to event must be externally synchronized
}

void ThreadSafety::PostCallRecordDestroyEvent(
    VkDevice                                    device,
    VkEvent                                     event,
    const VkAllocationCallbacks*                pAllocator) {
    FinishReadObjectParentInstance(device, "vkDestroyEvent");
    FinishWriteObject(event, "vkDestroyEvent");
    DestroyObject(event);
    // Host access to event must be externally synchronized
}

void ThreadSafety::PreCallRecordGetEventStatus(
    VkDevice                                    device,
    VkEvent                                     event) {
    StartReadObjectParentInstance(device, "vkGetEventStatus");
    StartReadObject(event, "vkGetEventStatus");
}

void ThreadSafety::PostCallRecordGetEventStatus(
    VkDevice                                    device,
    VkEvent                                     event,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkGetEventStatus");
    FinishReadObject(event, "vkGetEventStatus");
}

void ThreadSafety::PreCallRecordSetEvent(
    VkDevice                                    device,
    VkEvent                                     event) {
    StartReadObjectParentInstance(device, "vkSetEvent");
    StartWriteObject(event, "vkSetEvent");
    // Host access to event must be externally synchronized
}

void ThreadSafety::PostCallRecordSetEvent(
    VkDevice                                    device,
    VkEvent                                     event,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkSetEvent");
    FinishWriteObject(event, "vkSetEvent");
    // Host access to event must be externally synchronized
}

void ThreadSafety::PreCallRecordResetEvent(
    VkDevice                                    device,
    VkEvent                                     event) {
    StartReadObjectParentInstance(device, "vkResetEvent");
    StartWriteObject(event, "vkResetEvent");
    // Host access to event must be externally synchronized
}

void ThreadSafety::PostCallRecordResetEvent(
    VkDevice                                    device,
    VkEvent                                     event,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkResetEvent");
    FinishWriteObject(event, "vkResetEvent");
    // Host access to event must be externally synchronized
}

void ThreadSafety::PreCallRecordCreateQueryPool(
    VkDevice                                    device,
    const VkQueryPoolCreateInfo*                pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkQueryPool*                                pQueryPool) {
    StartReadObjectParentInstance(device, "vkCreateQueryPool");
}

void ThreadSafety::PostCallRecordCreateQueryPool(
    VkDevice                                    device,
    const VkQueryPoolCreateInfo*                pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkQueryPool*                                pQueryPool,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkCreateQueryPool");
    if (result == VK_SUCCESS) {
        CreateObject(*pQueryPool);
    }
}

void ThreadSafety::PreCallRecordDestroyQueryPool(
    VkDevice                                    device,
    VkQueryPool                                 queryPool,
    const VkAllocationCallbacks*                pAllocator) {
    StartReadObjectParentInstance(device, "vkDestroyQueryPool");
    StartWriteObject(queryPool, "vkDestroyQueryPool");
    // Host access to queryPool must be externally synchronized
}

void ThreadSafety::PostCallRecordDestroyQueryPool(
    VkDevice                                    device,
    VkQueryPool                                 queryPool,
    const VkAllocationCallbacks*                pAllocator) {
    FinishReadObjectParentInstance(device, "vkDestroyQueryPool");
    FinishWriteObject(queryPool, "vkDestroyQueryPool");
    DestroyObject(queryPool);
    // Host access to queryPool must be externally synchronized
}

void ThreadSafety::PreCallRecordGetQueryPoolResults(
    VkDevice                                    device,
    VkQueryPool                                 queryPool,
    uint32_t                                    firstQuery,
    uint32_t                                    queryCount,
    size_t                                      dataSize,
    void*                                       pData,
    VkDeviceSize                                stride,
    VkQueryResultFlags                          flags) {
    StartReadObjectParentInstance(device, "vkGetQueryPoolResults");
    StartReadObject(queryPool, "vkGetQueryPoolResults");
}

void ThreadSafety::PostCallRecordGetQueryPoolResults(
    VkDevice                                    device,
    VkQueryPool                                 queryPool,
    uint32_t                                    firstQuery,
    uint32_t                                    queryCount,
    size_t                                      dataSize,
    void*                                       pData,
    VkDeviceSize                                stride,
    VkQueryResultFlags                          flags,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkGetQueryPoolResults");
    FinishReadObject(queryPool, "vkGetQueryPoolResults");
}

void ThreadSafety::PreCallRecordCreateBuffer(
    VkDevice                                    device,
    const VkBufferCreateInfo*                   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkBuffer*                                   pBuffer) {
    StartReadObjectParentInstance(device, "vkCreateBuffer");
}

void ThreadSafety::PostCallRecordCreateBuffer(
    VkDevice                                    device,
    const VkBufferCreateInfo*                   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkBuffer*                                   pBuffer,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkCreateBuffer");
    if (result == VK_SUCCESS) {
        CreateObject(*pBuffer);
    }
}

void ThreadSafety::PreCallRecordDestroyBuffer(
    VkDevice                                    device,
    VkBuffer                                    buffer,
    const VkAllocationCallbacks*                pAllocator) {
    StartReadObjectParentInstance(device, "vkDestroyBuffer");
    StartWriteObject(buffer, "vkDestroyBuffer");
    // Host access to buffer must be externally synchronized
}

void ThreadSafety::PostCallRecordDestroyBuffer(
    VkDevice                                    device,
    VkBuffer                                    buffer,
    const VkAllocationCallbacks*                pAllocator) {
    FinishReadObjectParentInstance(device, "vkDestroyBuffer");
    FinishWriteObject(buffer, "vkDestroyBuffer");
    DestroyObject(buffer);
    // Host access to buffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCreateBufferView(
    VkDevice                                    device,
    const VkBufferViewCreateInfo*               pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkBufferView*                               pView) {
    StartReadObjectParentInstance(device, "vkCreateBufferView");
}

void ThreadSafety::PostCallRecordCreateBufferView(
    VkDevice                                    device,
    const VkBufferViewCreateInfo*               pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkBufferView*                               pView,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkCreateBufferView");
    if (result == VK_SUCCESS) {
        CreateObject(*pView);
    }
}

void ThreadSafety::PreCallRecordDestroyBufferView(
    VkDevice                                    device,
    VkBufferView                                bufferView,
    const VkAllocationCallbacks*                pAllocator) {
    StartReadObjectParentInstance(device, "vkDestroyBufferView");
    StartWriteObject(bufferView, "vkDestroyBufferView");
    // Host access to bufferView must be externally synchronized
}

void ThreadSafety::PostCallRecordDestroyBufferView(
    VkDevice                                    device,
    VkBufferView                                bufferView,
    const VkAllocationCallbacks*                pAllocator) {
    FinishReadObjectParentInstance(device, "vkDestroyBufferView");
    FinishWriteObject(bufferView, "vkDestroyBufferView");
    DestroyObject(bufferView);
    // Host access to bufferView must be externally synchronized
}

void ThreadSafety::PreCallRecordCreateImage(
    VkDevice                                    device,
    const VkImageCreateInfo*                    pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkImage*                                    pImage) {
    StartReadObjectParentInstance(device, "vkCreateImage");
}

void ThreadSafety::PostCallRecordCreateImage(
    VkDevice                                    device,
    const VkImageCreateInfo*                    pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkImage*                                    pImage,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkCreateImage");
    if (result == VK_SUCCESS) {
        CreateObject(*pImage);
    }
}

void ThreadSafety::PreCallRecordDestroyImage(
    VkDevice                                    device,
    VkImage                                     image,
    const VkAllocationCallbacks*                pAllocator) {
    StartReadObjectParentInstance(device, "vkDestroyImage");
    StartWriteObject(image, "vkDestroyImage");
    // Host access to image must be externally synchronized
}

void ThreadSafety::PostCallRecordDestroyImage(
    VkDevice                                    device,
    VkImage                                     image,
    const VkAllocationCallbacks*                pAllocator) {
    FinishReadObjectParentInstance(device, "vkDestroyImage");
    FinishWriteObject(image, "vkDestroyImage");
    DestroyObject(image);
    // Host access to image must be externally synchronized
}

void ThreadSafety::PreCallRecordGetImageSubresourceLayout(
    VkDevice                                    device,
    VkImage                                     image,
    const VkImageSubresource*                   pSubresource,
    VkSubresourceLayout*                        pLayout) {
    StartReadObjectParentInstance(device, "vkGetImageSubresourceLayout");
    StartReadObject(image, "vkGetImageSubresourceLayout");
}

void ThreadSafety::PostCallRecordGetImageSubresourceLayout(
    VkDevice                                    device,
    VkImage                                     image,
    const VkImageSubresource*                   pSubresource,
    VkSubresourceLayout*                        pLayout) {
    FinishReadObjectParentInstance(device, "vkGetImageSubresourceLayout");
    FinishReadObject(image, "vkGetImageSubresourceLayout");
}

void ThreadSafety::PreCallRecordCreateImageView(
    VkDevice                                    device,
    const VkImageViewCreateInfo*                pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkImageView*                                pView) {
    StartReadObjectParentInstance(device, "vkCreateImageView");
}

void ThreadSafety::PostCallRecordCreateImageView(
    VkDevice                                    device,
    const VkImageViewCreateInfo*                pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkImageView*                                pView,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkCreateImageView");
    if (result == VK_SUCCESS) {
        CreateObject(*pView);
    }
}

void ThreadSafety::PreCallRecordDestroyImageView(
    VkDevice                                    device,
    VkImageView                                 imageView,
    const VkAllocationCallbacks*                pAllocator) {
    StartReadObjectParentInstance(device, "vkDestroyImageView");
    StartWriteObject(imageView, "vkDestroyImageView");
    // Host access to imageView must be externally synchronized
}

void ThreadSafety::PostCallRecordDestroyImageView(
    VkDevice                                    device,
    VkImageView                                 imageView,
    const VkAllocationCallbacks*                pAllocator) {
    FinishReadObjectParentInstance(device, "vkDestroyImageView");
    FinishWriteObject(imageView, "vkDestroyImageView");
    DestroyObject(imageView);
    // Host access to imageView must be externally synchronized
}

void ThreadSafety::PreCallRecordCreateShaderModule(
    VkDevice                                    device,
    const VkShaderModuleCreateInfo*             pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkShaderModule*                             pShaderModule) {
    StartReadObjectParentInstance(device, "vkCreateShaderModule");
}

void ThreadSafety::PostCallRecordCreateShaderModule(
    VkDevice                                    device,
    const VkShaderModuleCreateInfo*             pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkShaderModule*                             pShaderModule,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkCreateShaderModule");
    if (result == VK_SUCCESS) {
        CreateObject(*pShaderModule);
    }
}

void ThreadSafety::PreCallRecordDestroyShaderModule(
    VkDevice                                    device,
    VkShaderModule                              shaderModule,
    const VkAllocationCallbacks*                pAllocator) {
    StartReadObjectParentInstance(device, "vkDestroyShaderModule");
    StartWriteObject(shaderModule, "vkDestroyShaderModule");
    // Host access to shaderModule must be externally synchronized
}

void ThreadSafety::PostCallRecordDestroyShaderModule(
    VkDevice                                    device,
    VkShaderModule                              shaderModule,
    const VkAllocationCallbacks*                pAllocator) {
    FinishReadObjectParentInstance(device, "vkDestroyShaderModule");
    FinishWriteObject(shaderModule, "vkDestroyShaderModule");
    DestroyObject(shaderModule);
    // Host access to shaderModule must be externally synchronized
}

void ThreadSafety::PreCallRecordCreatePipelineCache(
    VkDevice                                    device,
    const VkPipelineCacheCreateInfo*            pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkPipelineCache*                            pPipelineCache) {
    StartReadObjectParentInstance(device, "vkCreatePipelineCache");
}

void ThreadSafety::PostCallRecordCreatePipelineCache(
    VkDevice                                    device,
    const VkPipelineCacheCreateInfo*            pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkPipelineCache*                            pPipelineCache,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkCreatePipelineCache");
    if (result == VK_SUCCESS) {
        CreateObject(*pPipelineCache);
    }
}

void ThreadSafety::PreCallRecordDestroyPipelineCache(
    VkDevice                                    device,
    VkPipelineCache                             pipelineCache,
    const VkAllocationCallbacks*                pAllocator) {
    StartReadObjectParentInstance(device, "vkDestroyPipelineCache");
    StartWriteObject(pipelineCache, "vkDestroyPipelineCache");
    // Host access to pipelineCache must be externally synchronized
}

void ThreadSafety::PostCallRecordDestroyPipelineCache(
    VkDevice                                    device,
    VkPipelineCache                             pipelineCache,
    const VkAllocationCallbacks*                pAllocator) {
    FinishReadObjectParentInstance(device, "vkDestroyPipelineCache");
    FinishWriteObject(pipelineCache, "vkDestroyPipelineCache");
    DestroyObject(pipelineCache);
    // Host access to pipelineCache must be externally synchronized
}

void ThreadSafety::PreCallRecordGetPipelineCacheData(
    VkDevice                                    device,
    VkPipelineCache                             pipelineCache,
    size_t*                                     pDataSize,
    void*                                       pData) {
    StartReadObjectParentInstance(device, "vkGetPipelineCacheData");
    StartReadObject(pipelineCache, "vkGetPipelineCacheData");
}

void ThreadSafety::PostCallRecordGetPipelineCacheData(
    VkDevice                                    device,
    VkPipelineCache                             pipelineCache,
    size_t*                                     pDataSize,
    void*                                       pData,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkGetPipelineCacheData");
    FinishReadObject(pipelineCache, "vkGetPipelineCacheData");
}

void ThreadSafety::PreCallRecordMergePipelineCaches(
    VkDevice                                    device,
    VkPipelineCache                             dstCache,
    uint32_t                                    srcCacheCount,
    const VkPipelineCache*                      pSrcCaches) {
    StartReadObjectParentInstance(device, "vkMergePipelineCaches");
    StartWriteObject(dstCache, "vkMergePipelineCaches");
    if (pSrcCaches) {
        for (uint32_t index = 0; index < srcCacheCount; index++) {
            StartReadObject(pSrcCaches[index], "vkMergePipelineCaches");
        }
    }
    // Host access to dstCache must be externally synchronized
}

void ThreadSafety::PostCallRecordMergePipelineCaches(
    VkDevice                                    device,
    VkPipelineCache                             dstCache,
    uint32_t                                    srcCacheCount,
    const VkPipelineCache*                      pSrcCaches,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkMergePipelineCaches");
    FinishWriteObject(dstCache, "vkMergePipelineCaches");
    if (pSrcCaches) {
        for (uint32_t index = 0; index < srcCacheCount; index++) {
            FinishReadObject(pSrcCaches[index], "vkMergePipelineCaches");
        }
    }
    // Host access to dstCache must be externally synchronized
}

void ThreadSafety::PreCallRecordCreateGraphicsPipelines(
    VkDevice                                    device,
    VkPipelineCache                             pipelineCache,
    uint32_t                                    createInfoCount,
    const VkGraphicsPipelineCreateInfo*         pCreateInfos,
    const VkAllocationCallbacks*                pAllocator,
    VkPipeline*                                 pPipelines) {
    StartReadObjectParentInstance(device, "vkCreateGraphicsPipelines");
    StartReadObject(pipelineCache, "vkCreateGraphicsPipelines");
}

void ThreadSafety::PostCallRecordCreateGraphicsPipelines(
    VkDevice                                    device,
    VkPipelineCache                             pipelineCache,
    uint32_t                                    createInfoCount,
    const VkGraphicsPipelineCreateInfo*         pCreateInfos,
    const VkAllocationCallbacks*                pAllocator,
    VkPipeline*                                 pPipelines,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkCreateGraphicsPipelines");
    FinishReadObject(pipelineCache, "vkCreateGraphicsPipelines");
    if (pPipelines) {
        for (uint32_t index = 0; index < createInfoCount; index++) {
            if (!pPipelines[index]) continue;
            CreateObject(pPipelines[index]);
        }
    }
}

void ThreadSafety::PreCallRecordCreateComputePipelines(
    VkDevice                                    device,
    VkPipelineCache                             pipelineCache,
    uint32_t                                    createInfoCount,
    const VkComputePipelineCreateInfo*          pCreateInfos,
    const VkAllocationCallbacks*                pAllocator,
    VkPipeline*                                 pPipelines) {
    StartReadObjectParentInstance(device, "vkCreateComputePipelines");
    StartReadObject(pipelineCache, "vkCreateComputePipelines");
}

void ThreadSafety::PostCallRecordCreateComputePipelines(
    VkDevice                                    device,
    VkPipelineCache                             pipelineCache,
    uint32_t                                    createInfoCount,
    const VkComputePipelineCreateInfo*          pCreateInfos,
    const VkAllocationCallbacks*                pAllocator,
    VkPipeline*                                 pPipelines,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkCreateComputePipelines");
    FinishReadObject(pipelineCache, "vkCreateComputePipelines");
    if (pPipelines) {
        for (uint32_t index = 0; index < createInfoCount; index++) {
            if (!pPipelines[index]) continue;
            CreateObject(pPipelines[index]);
        }
    }
}

void ThreadSafety::PreCallRecordDestroyPipeline(
    VkDevice                                    device,
    VkPipeline                                  pipeline,
    const VkAllocationCallbacks*                pAllocator) {
    StartReadObjectParentInstance(device, "vkDestroyPipeline");
    StartWriteObject(pipeline, "vkDestroyPipeline");
    // Host access to pipeline must be externally synchronized
}

void ThreadSafety::PostCallRecordDestroyPipeline(
    VkDevice                                    device,
    VkPipeline                                  pipeline,
    const VkAllocationCallbacks*                pAllocator) {
    FinishReadObjectParentInstance(device, "vkDestroyPipeline");
    FinishWriteObject(pipeline, "vkDestroyPipeline");
    DestroyObject(pipeline);
    // Host access to pipeline must be externally synchronized
}

void ThreadSafety::PreCallRecordCreatePipelineLayout(
    VkDevice                                    device,
    const VkPipelineLayoutCreateInfo*           pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkPipelineLayout*                           pPipelineLayout) {
    StartReadObjectParentInstance(device, "vkCreatePipelineLayout");
}

void ThreadSafety::PostCallRecordCreatePipelineLayout(
    VkDevice                                    device,
    const VkPipelineLayoutCreateInfo*           pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkPipelineLayout*                           pPipelineLayout,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkCreatePipelineLayout");
    if (result == VK_SUCCESS) {
        CreateObject(*pPipelineLayout);
    }
}

void ThreadSafety::PreCallRecordDestroyPipelineLayout(
    VkDevice                                    device,
    VkPipelineLayout                            pipelineLayout,
    const VkAllocationCallbacks*                pAllocator) {
    StartReadObjectParentInstance(device, "vkDestroyPipelineLayout");
    StartWriteObject(pipelineLayout, "vkDestroyPipelineLayout");
    // Host access to pipelineLayout must be externally synchronized
}

void ThreadSafety::PostCallRecordDestroyPipelineLayout(
    VkDevice                                    device,
    VkPipelineLayout                            pipelineLayout,
    const VkAllocationCallbacks*                pAllocator) {
    FinishReadObjectParentInstance(device, "vkDestroyPipelineLayout");
    FinishWriteObject(pipelineLayout, "vkDestroyPipelineLayout");
    DestroyObject(pipelineLayout);
    // Host access to pipelineLayout must be externally synchronized
}

void ThreadSafety::PreCallRecordCreateSampler(
    VkDevice                                    device,
    const VkSamplerCreateInfo*                  pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSampler*                                  pSampler) {
    StartReadObjectParentInstance(device, "vkCreateSampler");
}

void ThreadSafety::PostCallRecordCreateSampler(
    VkDevice                                    device,
    const VkSamplerCreateInfo*                  pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSampler*                                  pSampler,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkCreateSampler");
    if (result == VK_SUCCESS) {
        CreateObject(*pSampler);
    }
}

void ThreadSafety::PreCallRecordDestroySampler(
    VkDevice                                    device,
    VkSampler                                   sampler,
    const VkAllocationCallbacks*                pAllocator) {
    StartReadObjectParentInstance(device, "vkDestroySampler");
    StartWriteObject(sampler, "vkDestroySampler");
    // Host access to sampler must be externally synchronized
}

void ThreadSafety::PostCallRecordDestroySampler(
    VkDevice                                    device,
    VkSampler                                   sampler,
    const VkAllocationCallbacks*                pAllocator) {
    FinishReadObjectParentInstance(device, "vkDestroySampler");
    FinishWriteObject(sampler, "vkDestroySampler");
    DestroyObject(sampler);
    // Host access to sampler must be externally synchronized
}

void ThreadSafety::PreCallRecordDestroyDescriptorSetLayout(
    VkDevice                                    device,
    VkDescriptorSetLayout                       descriptorSetLayout,
    const VkAllocationCallbacks*                pAllocator) {
    StartReadObjectParentInstance(device, "vkDestroyDescriptorSetLayout");
    StartWriteObject(descriptorSetLayout, "vkDestroyDescriptorSetLayout");
    // Host access to descriptorSetLayout must be externally synchronized
}

void ThreadSafety::PostCallRecordDestroyDescriptorSetLayout(
    VkDevice                                    device,
    VkDescriptorSetLayout                       descriptorSetLayout,
    const VkAllocationCallbacks*                pAllocator) {
    FinishReadObjectParentInstance(device, "vkDestroyDescriptorSetLayout");
    FinishWriteObject(descriptorSetLayout, "vkDestroyDescriptorSetLayout");
    DestroyObject(descriptorSetLayout);
    // Host access to descriptorSetLayout must be externally synchronized
}

void ThreadSafety::PreCallRecordCreateDescriptorPool(
    VkDevice                                    device,
    const VkDescriptorPoolCreateInfo*           pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDescriptorPool*                           pDescriptorPool) {
    StartReadObjectParentInstance(device, "vkCreateDescriptorPool");
}

void ThreadSafety::PostCallRecordCreateDescriptorPool(
    VkDevice                                    device,
    const VkDescriptorPoolCreateInfo*           pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDescriptorPool*                           pDescriptorPool,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkCreateDescriptorPool");
    if (result == VK_SUCCESS) {
        CreateObject(*pDescriptorPool);
    }
}

void ThreadSafety::PreCallRecordCreateFramebuffer(
    VkDevice                                    device,
    const VkFramebufferCreateInfo*              pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkFramebuffer*                              pFramebuffer) {
    StartReadObjectParentInstance(device, "vkCreateFramebuffer");
}

void ThreadSafety::PostCallRecordCreateFramebuffer(
    VkDevice                                    device,
    const VkFramebufferCreateInfo*              pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkFramebuffer*                              pFramebuffer,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkCreateFramebuffer");
    if (result == VK_SUCCESS) {
        CreateObject(*pFramebuffer);
    }
}

void ThreadSafety::PreCallRecordDestroyFramebuffer(
    VkDevice                                    device,
    VkFramebuffer                               framebuffer,
    const VkAllocationCallbacks*                pAllocator) {
    StartReadObjectParentInstance(device, "vkDestroyFramebuffer");
    StartWriteObject(framebuffer, "vkDestroyFramebuffer");
    // Host access to framebuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordDestroyFramebuffer(
    VkDevice                                    device,
    VkFramebuffer                               framebuffer,
    const VkAllocationCallbacks*                pAllocator) {
    FinishReadObjectParentInstance(device, "vkDestroyFramebuffer");
    FinishWriteObject(framebuffer, "vkDestroyFramebuffer");
    DestroyObject(framebuffer);
    // Host access to framebuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCreateRenderPass(
    VkDevice                                    device,
    const VkRenderPassCreateInfo*               pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkRenderPass*                               pRenderPass) {
    StartReadObjectParentInstance(device, "vkCreateRenderPass");
}

void ThreadSafety::PostCallRecordCreateRenderPass(
    VkDevice                                    device,
    const VkRenderPassCreateInfo*               pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkRenderPass*                               pRenderPass,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkCreateRenderPass");
    if (result == VK_SUCCESS) {
        CreateObject(*pRenderPass);
    }
}

void ThreadSafety::PreCallRecordDestroyRenderPass(
    VkDevice                                    device,
    VkRenderPass                                renderPass,
    const VkAllocationCallbacks*                pAllocator) {
    StartReadObjectParentInstance(device, "vkDestroyRenderPass");
    StartWriteObject(renderPass, "vkDestroyRenderPass");
    // Host access to renderPass must be externally synchronized
}

void ThreadSafety::PostCallRecordDestroyRenderPass(
    VkDevice                                    device,
    VkRenderPass                                renderPass,
    const VkAllocationCallbacks*                pAllocator) {
    FinishReadObjectParentInstance(device, "vkDestroyRenderPass");
    FinishWriteObject(renderPass, "vkDestroyRenderPass");
    DestroyObject(renderPass);
    // Host access to renderPass must be externally synchronized
}

void ThreadSafety::PreCallRecordGetRenderAreaGranularity(
    VkDevice                                    device,
    VkRenderPass                                renderPass,
    VkExtent2D*                                 pGranularity) {
    StartReadObjectParentInstance(device, "vkGetRenderAreaGranularity");
    StartReadObject(renderPass, "vkGetRenderAreaGranularity");
}

void ThreadSafety::PostCallRecordGetRenderAreaGranularity(
    VkDevice                                    device,
    VkRenderPass                                renderPass,
    VkExtent2D*                                 pGranularity) {
    FinishReadObjectParentInstance(device, "vkGetRenderAreaGranularity");
    FinishReadObject(renderPass, "vkGetRenderAreaGranularity");
}

void ThreadSafety::PreCallRecordBeginCommandBuffer(
    VkCommandBuffer                             commandBuffer,
    const VkCommandBufferBeginInfo*             pBeginInfo) {
    StartWriteObject(commandBuffer, "vkBeginCommandBuffer");
    // Host access to commandBuffer must be externally synchronized
    // the sname:VkCommandPool that pname:commandBuffer was allocated from must be externally synchronized between host accesses
}

void ThreadSafety::PostCallRecordBeginCommandBuffer(
    VkCommandBuffer                             commandBuffer,
    const VkCommandBufferBeginInfo*             pBeginInfo,
    VkResult                                    result) {
    FinishWriteObject(commandBuffer, "vkBeginCommandBuffer");
    // Host access to commandBuffer must be externally synchronized
    // the sname:VkCommandPool that pname:commandBuffer was allocated from must be externally synchronized between host accesses
}

void ThreadSafety::PreCallRecordEndCommandBuffer(
    VkCommandBuffer                             commandBuffer) {
    StartWriteObject(commandBuffer, "vkEndCommandBuffer");
    // Host access to commandBuffer must be externally synchronized
    // the sname:VkCommandPool that pname:commandBuffer was allocated from must be externally synchronized between host accesses
}

void ThreadSafety::PostCallRecordEndCommandBuffer(
    VkCommandBuffer                             commandBuffer,
    VkResult                                    result) {
    FinishWriteObject(commandBuffer, "vkEndCommandBuffer");
    // Host access to commandBuffer must be externally synchronized
    // the sname:VkCommandPool that pname:commandBuffer was allocated from must be externally synchronized between host accesses
}

void ThreadSafety::PreCallRecordResetCommandBuffer(
    VkCommandBuffer                             commandBuffer,
    VkCommandBufferResetFlags                   flags) {
    StartWriteObject(commandBuffer, "vkResetCommandBuffer");
    // Host access to commandBuffer must be externally synchronized
    // the sname:VkCommandPool that pname:commandBuffer was allocated from must be externally synchronized between host accesses
}

void ThreadSafety::PostCallRecordResetCommandBuffer(
    VkCommandBuffer                             commandBuffer,
    VkCommandBufferResetFlags                   flags,
    VkResult                                    result) {
    FinishWriteObject(commandBuffer, "vkResetCommandBuffer");
    // Host access to commandBuffer must be externally synchronized
    // the sname:VkCommandPool that pname:commandBuffer was allocated from must be externally synchronized between host accesses
}

void ThreadSafety::PreCallRecordCmdBindPipeline(
    VkCommandBuffer                             commandBuffer,
    VkPipelineBindPoint                         pipelineBindPoint,
    VkPipeline                                  pipeline) {
    StartWriteObject(commandBuffer, "vkCmdBindPipeline");
    StartReadObject(pipeline, "vkCmdBindPipeline");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdBindPipeline(
    VkCommandBuffer                             commandBuffer,
    VkPipelineBindPoint                         pipelineBindPoint,
    VkPipeline                                  pipeline) {
    FinishWriteObject(commandBuffer, "vkCmdBindPipeline");
    FinishReadObject(pipeline, "vkCmdBindPipeline");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetViewport(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstViewport,
    uint32_t                                    viewportCount,
    const VkViewport*                           pViewports) {
    StartWriteObject(commandBuffer, "vkCmdSetViewport");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetViewport(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstViewport,
    uint32_t                                    viewportCount,
    const VkViewport*                           pViewports) {
    FinishWriteObject(commandBuffer, "vkCmdSetViewport");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetScissor(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstScissor,
    uint32_t                                    scissorCount,
    const VkRect2D*                             pScissors) {
    StartWriteObject(commandBuffer, "vkCmdSetScissor");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetScissor(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstScissor,
    uint32_t                                    scissorCount,
    const VkRect2D*                             pScissors) {
    FinishWriteObject(commandBuffer, "vkCmdSetScissor");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetLineWidth(
    VkCommandBuffer                             commandBuffer,
    float                                       lineWidth) {
    StartWriteObject(commandBuffer, "vkCmdSetLineWidth");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetLineWidth(
    VkCommandBuffer                             commandBuffer,
    float                                       lineWidth) {
    FinishWriteObject(commandBuffer, "vkCmdSetLineWidth");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetDepthBias(
    VkCommandBuffer                             commandBuffer,
    float                                       depthBiasConstantFactor,
    float                                       depthBiasClamp,
    float                                       depthBiasSlopeFactor) {
    StartWriteObject(commandBuffer, "vkCmdSetDepthBias");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetDepthBias(
    VkCommandBuffer                             commandBuffer,
    float                                       depthBiasConstantFactor,
    float                                       depthBiasClamp,
    float                                       depthBiasSlopeFactor) {
    FinishWriteObject(commandBuffer, "vkCmdSetDepthBias");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetBlendConstants(
    VkCommandBuffer                             commandBuffer,
    const float                                 blendConstants[4]) {
    StartWriteObject(commandBuffer, "vkCmdSetBlendConstants");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetBlendConstants(
    VkCommandBuffer                             commandBuffer,
    const float                                 blendConstants[4]) {
    FinishWriteObject(commandBuffer, "vkCmdSetBlendConstants");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetDepthBounds(
    VkCommandBuffer                             commandBuffer,
    float                                       minDepthBounds,
    float                                       maxDepthBounds) {
    StartWriteObject(commandBuffer, "vkCmdSetDepthBounds");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetDepthBounds(
    VkCommandBuffer                             commandBuffer,
    float                                       minDepthBounds,
    float                                       maxDepthBounds) {
    FinishWriteObject(commandBuffer, "vkCmdSetDepthBounds");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetStencilCompareMask(
    VkCommandBuffer                             commandBuffer,
    VkStencilFaceFlags                          faceMask,
    uint32_t                                    compareMask) {
    StartWriteObject(commandBuffer, "vkCmdSetStencilCompareMask");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetStencilCompareMask(
    VkCommandBuffer                             commandBuffer,
    VkStencilFaceFlags                          faceMask,
    uint32_t                                    compareMask) {
    FinishWriteObject(commandBuffer, "vkCmdSetStencilCompareMask");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetStencilWriteMask(
    VkCommandBuffer                             commandBuffer,
    VkStencilFaceFlags                          faceMask,
    uint32_t                                    writeMask) {
    StartWriteObject(commandBuffer, "vkCmdSetStencilWriteMask");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetStencilWriteMask(
    VkCommandBuffer                             commandBuffer,
    VkStencilFaceFlags                          faceMask,
    uint32_t                                    writeMask) {
    FinishWriteObject(commandBuffer, "vkCmdSetStencilWriteMask");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetStencilReference(
    VkCommandBuffer                             commandBuffer,
    VkStencilFaceFlags                          faceMask,
    uint32_t                                    reference) {
    StartWriteObject(commandBuffer, "vkCmdSetStencilReference");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetStencilReference(
    VkCommandBuffer                             commandBuffer,
    VkStencilFaceFlags                          faceMask,
    uint32_t                                    reference) {
    FinishWriteObject(commandBuffer, "vkCmdSetStencilReference");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdBindDescriptorSets(
    VkCommandBuffer                             commandBuffer,
    VkPipelineBindPoint                         pipelineBindPoint,
    VkPipelineLayout                            layout,
    uint32_t                                    firstSet,
    uint32_t                                    descriptorSetCount,
    const VkDescriptorSet*                      pDescriptorSets,
    uint32_t                                    dynamicOffsetCount,
    const uint32_t*                             pDynamicOffsets) {
    StartWriteObject(commandBuffer, "vkCmdBindDescriptorSets");
    StartReadObject(layout, "vkCmdBindDescriptorSets");
    if (pDescriptorSets) {
        for (uint32_t index = 0; index < descriptorSetCount; index++) {
            StartReadObject(pDescriptorSets[index], "vkCmdBindDescriptorSets");
        }
    }
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdBindDescriptorSets(
    VkCommandBuffer                             commandBuffer,
    VkPipelineBindPoint                         pipelineBindPoint,
    VkPipelineLayout                            layout,
    uint32_t                                    firstSet,
    uint32_t                                    descriptorSetCount,
    const VkDescriptorSet*                      pDescriptorSets,
    uint32_t                                    dynamicOffsetCount,
    const uint32_t*                             pDynamicOffsets) {
    FinishWriteObject(commandBuffer, "vkCmdBindDescriptorSets");
    FinishReadObject(layout, "vkCmdBindDescriptorSets");
    if (pDescriptorSets) {
        for (uint32_t index = 0; index < descriptorSetCount; index++) {
            FinishReadObject(pDescriptorSets[index], "vkCmdBindDescriptorSets");
        }
    }
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdBindIndexBuffer(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkIndexType                                 indexType) {
    StartWriteObject(commandBuffer, "vkCmdBindIndexBuffer");
    StartReadObject(buffer, "vkCmdBindIndexBuffer");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdBindIndexBuffer(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkIndexType                                 indexType) {
    FinishWriteObject(commandBuffer, "vkCmdBindIndexBuffer");
    FinishReadObject(buffer, "vkCmdBindIndexBuffer");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdBindVertexBuffers(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstBinding,
    uint32_t                                    bindingCount,
    const VkBuffer*                             pBuffers,
    const VkDeviceSize*                         pOffsets) {
    StartWriteObject(commandBuffer, "vkCmdBindVertexBuffers");
    if (pBuffers) {
        for (uint32_t index = 0; index < bindingCount; index++) {
            StartReadObject(pBuffers[index], "vkCmdBindVertexBuffers");
        }
    }
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdBindVertexBuffers(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstBinding,
    uint32_t                                    bindingCount,
    const VkBuffer*                             pBuffers,
    const VkDeviceSize*                         pOffsets) {
    FinishWriteObject(commandBuffer, "vkCmdBindVertexBuffers");
    if (pBuffers) {
        for (uint32_t index = 0; index < bindingCount; index++) {
            FinishReadObject(pBuffers[index], "vkCmdBindVertexBuffers");
        }
    }
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdDraw(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    vertexCount,
    uint32_t                                    instanceCount,
    uint32_t                                    firstVertex,
    uint32_t                                    firstInstance) {
    StartWriteObject(commandBuffer, "vkCmdDraw");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdDraw(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    vertexCount,
    uint32_t                                    instanceCount,
    uint32_t                                    firstVertex,
    uint32_t                                    firstInstance) {
    FinishWriteObject(commandBuffer, "vkCmdDraw");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdDrawIndexed(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    indexCount,
    uint32_t                                    instanceCount,
    uint32_t                                    firstIndex,
    int32_t                                     vertexOffset,
    uint32_t                                    firstInstance) {
    StartWriteObject(commandBuffer, "vkCmdDrawIndexed");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdDrawIndexed(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    indexCount,
    uint32_t                                    instanceCount,
    uint32_t                                    firstIndex,
    int32_t                                     vertexOffset,
    uint32_t                                    firstInstance) {
    FinishWriteObject(commandBuffer, "vkCmdDrawIndexed");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdDrawIndirect(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    uint32_t                                    drawCount,
    uint32_t                                    stride) {
    StartWriteObject(commandBuffer, "vkCmdDrawIndirect");
    StartReadObject(buffer, "vkCmdDrawIndirect");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdDrawIndirect(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    uint32_t                                    drawCount,
    uint32_t                                    stride) {
    FinishWriteObject(commandBuffer, "vkCmdDrawIndirect");
    FinishReadObject(buffer, "vkCmdDrawIndirect");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdDrawIndexedIndirect(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    uint32_t                                    drawCount,
    uint32_t                                    stride) {
    StartWriteObject(commandBuffer, "vkCmdDrawIndexedIndirect");
    StartReadObject(buffer, "vkCmdDrawIndexedIndirect");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdDrawIndexedIndirect(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    uint32_t                                    drawCount,
    uint32_t                                    stride) {
    FinishWriteObject(commandBuffer, "vkCmdDrawIndexedIndirect");
    FinishReadObject(buffer, "vkCmdDrawIndexedIndirect");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdDispatch(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    groupCountX,
    uint32_t                                    groupCountY,
    uint32_t                                    groupCountZ) {
    StartWriteObject(commandBuffer, "vkCmdDispatch");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdDispatch(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    groupCountX,
    uint32_t                                    groupCountY,
    uint32_t                                    groupCountZ) {
    FinishWriteObject(commandBuffer, "vkCmdDispatch");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdDispatchIndirect(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset) {
    StartWriteObject(commandBuffer, "vkCmdDispatchIndirect");
    StartReadObject(buffer, "vkCmdDispatchIndirect");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdDispatchIndirect(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset) {
    FinishWriteObject(commandBuffer, "vkCmdDispatchIndirect");
    FinishReadObject(buffer, "vkCmdDispatchIndirect");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdCopyBuffer(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    srcBuffer,
    VkBuffer                                    dstBuffer,
    uint32_t                                    regionCount,
    const VkBufferCopy*                         pRegions) {
    StartWriteObject(commandBuffer, "vkCmdCopyBuffer");
    StartReadObject(srcBuffer, "vkCmdCopyBuffer");
    StartReadObject(dstBuffer, "vkCmdCopyBuffer");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdCopyBuffer(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    srcBuffer,
    VkBuffer                                    dstBuffer,
    uint32_t                                    regionCount,
    const VkBufferCopy*                         pRegions) {
    FinishWriteObject(commandBuffer, "vkCmdCopyBuffer");
    FinishReadObject(srcBuffer, "vkCmdCopyBuffer");
    FinishReadObject(dstBuffer, "vkCmdCopyBuffer");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdCopyImage(
    VkCommandBuffer                             commandBuffer,
    VkImage                                     srcImage,
    VkImageLayout                               srcImageLayout,
    VkImage                                     dstImage,
    VkImageLayout                               dstImageLayout,
    uint32_t                                    regionCount,
    const VkImageCopy*                          pRegions) {
    StartWriteObject(commandBuffer, "vkCmdCopyImage");
    StartReadObject(srcImage, "vkCmdCopyImage");
    StartReadObject(dstImage, "vkCmdCopyImage");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdCopyImage(
    VkCommandBuffer                             commandBuffer,
    VkImage                                     srcImage,
    VkImageLayout                               srcImageLayout,
    VkImage                                     dstImage,
    VkImageLayout                               dstImageLayout,
    uint32_t                                    regionCount,
    const VkImageCopy*                          pRegions) {
    FinishWriteObject(commandBuffer, "vkCmdCopyImage");
    FinishReadObject(srcImage, "vkCmdCopyImage");
    FinishReadObject(dstImage, "vkCmdCopyImage");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdBlitImage(
    VkCommandBuffer                             commandBuffer,
    VkImage                                     srcImage,
    VkImageLayout                               srcImageLayout,
    VkImage                                     dstImage,
    VkImageLayout                               dstImageLayout,
    uint32_t                                    regionCount,
    const VkImageBlit*                          pRegions,
    VkFilter                                    filter) {
    StartWriteObject(commandBuffer, "vkCmdBlitImage");
    StartReadObject(srcImage, "vkCmdBlitImage");
    StartReadObject(dstImage, "vkCmdBlitImage");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdBlitImage(
    VkCommandBuffer                             commandBuffer,
    VkImage                                     srcImage,
    VkImageLayout                               srcImageLayout,
    VkImage                                     dstImage,
    VkImageLayout                               dstImageLayout,
    uint32_t                                    regionCount,
    const VkImageBlit*                          pRegions,
    VkFilter                                    filter) {
    FinishWriteObject(commandBuffer, "vkCmdBlitImage");
    FinishReadObject(srcImage, "vkCmdBlitImage");
    FinishReadObject(dstImage, "vkCmdBlitImage");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdCopyBufferToImage(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    srcBuffer,
    VkImage                                     dstImage,
    VkImageLayout                               dstImageLayout,
    uint32_t                                    regionCount,
    const VkBufferImageCopy*                    pRegions) {
    StartWriteObject(commandBuffer, "vkCmdCopyBufferToImage");
    StartReadObject(srcBuffer, "vkCmdCopyBufferToImage");
    StartReadObject(dstImage, "vkCmdCopyBufferToImage");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdCopyBufferToImage(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    srcBuffer,
    VkImage                                     dstImage,
    VkImageLayout                               dstImageLayout,
    uint32_t                                    regionCount,
    const VkBufferImageCopy*                    pRegions) {
    FinishWriteObject(commandBuffer, "vkCmdCopyBufferToImage");
    FinishReadObject(srcBuffer, "vkCmdCopyBufferToImage");
    FinishReadObject(dstImage, "vkCmdCopyBufferToImage");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdCopyImageToBuffer(
    VkCommandBuffer                             commandBuffer,
    VkImage                                     srcImage,
    VkImageLayout                               srcImageLayout,
    VkBuffer                                    dstBuffer,
    uint32_t                                    regionCount,
    const VkBufferImageCopy*                    pRegions) {
    StartWriteObject(commandBuffer, "vkCmdCopyImageToBuffer");
    StartReadObject(srcImage, "vkCmdCopyImageToBuffer");
    StartReadObject(dstBuffer, "vkCmdCopyImageToBuffer");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdCopyImageToBuffer(
    VkCommandBuffer                             commandBuffer,
    VkImage                                     srcImage,
    VkImageLayout                               srcImageLayout,
    VkBuffer                                    dstBuffer,
    uint32_t                                    regionCount,
    const VkBufferImageCopy*                    pRegions) {
    FinishWriteObject(commandBuffer, "vkCmdCopyImageToBuffer");
    FinishReadObject(srcImage, "vkCmdCopyImageToBuffer");
    FinishReadObject(dstBuffer, "vkCmdCopyImageToBuffer");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdUpdateBuffer(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    dstBuffer,
    VkDeviceSize                                dstOffset,
    VkDeviceSize                                dataSize,
    const void*                                 pData) {
    StartWriteObject(commandBuffer, "vkCmdUpdateBuffer");
    StartReadObject(dstBuffer, "vkCmdUpdateBuffer");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdUpdateBuffer(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    dstBuffer,
    VkDeviceSize                                dstOffset,
    VkDeviceSize                                dataSize,
    const void*                                 pData) {
    FinishWriteObject(commandBuffer, "vkCmdUpdateBuffer");
    FinishReadObject(dstBuffer, "vkCmdUpdateBuffer");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdFillBuffer(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    dstBuffer,
    VkDeviceSize                                dstOffset,
    VkDeviceSize                                size,
    uint32_t                                    data) {
    StartWriteObject(commandBuffer, "vkCmdFillBuffer");
    StartReadObject(dstBuffer, "vkCmdFillBuffer");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdFillBuffer(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    dstBuffer,
    VkDeviceSize                                dstOffset,
    VkDeviceSize                                size,
    uint32_t                                    data) {
    FinishWriteObject(commandBuffer, "vkCmdFillBuffer");
    FinishReadObject(dstBuffer, "vkCmdFillBuffer");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdClearColorImage(
    VkCommandBuffer                             commandBuffer,
    VkImage                                     image,
    VkImageLayout                               imageLayout,
    const VkClearColorValue*                    pColor,
    uint32_t                                    rangeCount,
    const VkImageSubresourceRange*              pRanges) {
    StartWriteObject(commandBuffer, "vkCmdClearColorImage");
    StartReadObject(image, "vkCmdClearColorImage");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdClearColorImage(
    VkCommandBuffer                             commandBuffer,
    VkImage                                     image,
    VkImageLayout                               imageLayout,
    const VkClearColorValue*                    pColor,
    uint32_t                                    rangeCount,
    const VkImageSubresourceRange*              pRanges) {
    FinishWriteObject(commandBuffer, "vkCmdClearColorImage");
    FinishReadObject(image, "vkCmdClearColorImage");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdClearDepthStencilImage(
    VkCommandBuffer                             commandBuffer,
    VkImage                                     image,
    VkImageLayout                               imageLayout,
    const VkClearDepthStencilValue*             pDepthStencil,
    uint32_t                                    rangeCount,
    const VkImageSubresourceRange*              pRanges) {
    StartWriteObject(commandBuffer, "vkCmdClearDepthStencilImage");
    StartReadObject(image, "vkCmdClearDepthStencilImage");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdClearDepthStencilImage(
    VkCommandBuffer                             commandBuffer,
    VkImage                                     image,
    VkImageLayout                               imageLayout,
    const VkClearDepthStencilValue*             pDepthStencil,
    uint32_t                                    rangeCount,
    const VkImageSubresourceRange*              pRanges) {
    FinishWriteObject(commandBuffer, "vkCmdClearDepthStencilImage");
    FinishReadObject(image, "vkCmdClearDepthStencilImage");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdClearAttachments(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    attachmentCount,
    const VkClearAttachment*                    pAttachments,
    uint32_t                                    rectCount,
    const VkClearRect*                          pRects) {
    StartWriteObject(commandBuffer, "vkCmdClearAttachments");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdClearAttachments(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    attachmentCount,
    const VkClearAttachment*                    pAttachments,
    uint32_t                                    rectCount,
    const VkClearRect*                          pRects) {
    FinishWriteObject(commandBuffer, "vkCmdClearAttachments");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdResolveImage(
    VkCommandBuffer                             commandBuffer,
    VkImage                                     srcImage,
    VkImageLayout                               srcImageLayout,
    VkImage                                     dstImage,
    VkImageLayout                               dstImageLayout,
    uint32_t                                    regionCount,
    const VkImageResolve*                       pRegions) {
    StartWriteObject(commandBuffer, "vkCmdResolveImage");
    StartReadObject(srcImage, "vkCmdResolveImage");
    StartReadObject(dstImage, "vkCmdResolveImage");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdResolveImage(
    VkCommandBuffer                             commandBuffer,
    VkImage                                     srcImage,
    VkImageLayout                               srcImageLayout,
    VkImage                                     dstImage,
    VkImageLayout                               dstImageLayout,
    uint32_t                                    regionCount,
    const VkImageResolve*                       pRegions) {
    FinishWriteObject(commandBuffer, "vkCmdResolveImage");
    FinishReadObject(srcImage, "vkCmdResolveImage");
    FinishReadObject(dstImage, "vkCmdResolveImage");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetEvent(
    VkCommandBuffer                             commandBuffer,
    VkEvent                                     event,
    VkPipelineStageFlags                        stageMask) {
    StartWriteObject(commandBuffer, "vkCmdSetEvent");
    StartReadObject(event, "vkCmdSetEvent");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetEvent(
    VkCommandBuffer                             commandBuffer,
    VkEvent                                     event,
    VkPipelineStageFlags                        stageMask) {
    FinishWriteObject(commandBuffer, "vkCmdSetEvent");
    FinishReadObject(event, "vkCmdSetEvent");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdResetEvent(
    VkCommandBuffer                             commandBuffer,
    VkEvent                                     event,
    VkPipelineStageFlags                        stageMask) {
    StartWriteObject(commandBuffer, "vkCmdResetEvent");
    StartReadObject(event, "vkCmdResetEvent");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdResetEvent(
    VkCommandBuffer                             commandBuffer,
    VkEvent                                     event,
    VkPipelineStageFlags                        stageMask) {
    FinishWriteObject(commandBuffer, "vkCmdResetEvent");
    FinishReadObject(event, "vkCmdResetEvent");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdWaitEvents(
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
    StartWriteObject(commandBuffer, "vkCmdWaitEvents");
    if (pEvents) {
        for (uint32_t index = 0; index < eventCount; index++) {
            StartReadObject(pEvents[index], "vkCmdWaitEvents");
        }
    }
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdWaitEvents(
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
    FinishWriteObject(commandBuffer, "vkCmdWaitEvents");
    if (pEvents) {
        for (uint32_t index = 0; index < eventCount; index++) {
            FinishReadObject(pEvents[index], "vkCmdWaitEvents");
        }
    }
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdPipelineBarrier(
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
    StartWriteObject(commandBuffer, "vkCmdPipelineBarrier");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdPipelineBarrier(
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
    FinishWriteObject(commandBuffer, "vkCmdPipelineBarrier");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdBeginQuery(
    VkCommandBuffer                             commandBuffer,
    VkQueryPool                                 queryPool,
    uint32_t                                    query,
    VkQueryControlFlags                         flags) {
    StartWriteObject(commandBuffer, "vkCmdBeginQuery");
    StartReadObject(queryPool, "vkCmdBeginQuery");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdBeginQuery(
    VkCommandBuffer                             commandBuffer,
    VkQueryPool                                 queryPool,
    uint32_t                                    query,
    VkQueryControlFlags                         flags) {
    FinishWriteObject(commandBuffer, "vkCmdBeginQuery");
    FinishReadObject(queryPool, "vkCmdBeginQuery");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdEndQuery(
    VkCommandBuffer                             commandBuffer,
    VkQueryPool                                 queryPool,
    uint32_t                                    query) {
    StartWriteObject(commandBuffer, "vkCmdEndQuery");
    StartReadObject(queryPool, "vkCmdEndQuery");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdEndQuery(
    VkCommandBuffer                             commandBuffer,
    VkQueryPool                                 queryPool,
    uint32_t                                    query) {
    FinishWriteObject(commandBuffer, "vkCmdEndQuery");
    FinishReadObject(queryPool, "vkCmdEndQuery");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdResetQueryPool(
    VkCommandBuffer                             commandBuffer,
    VkQueryPool                                 queryPool,
    uint32_t                                    firstQuery,
    uint32_t                                    queryCount) {
    StartWriteObject(commandBuffer, "vkCmdResetQueryPool");
    StartReadObject(queryPool, "vkCmdResetQueryPool");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdResetQueryPool(
    VkCommandBuffer                             commandBuffer,
    VkQueryPool                                 queryPool,
    uint32_t                                    firstQuery,
    uint32_t                                    queryCount) {
    FinishWriteObject(commandBuffer, "vkCmdResetQueryPool");
    FinishReadObject(queryPool, "vkCmdResetQueryPool");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdWriteTimestamp(
    VkCommandBuffer                             commandBuffer,
    VkPipelineStageFlagBits                     pipelineStage,
    VkQueryPool                                 queryPool,
    uint32_t                                    query) {
    StartWriteObject(commandBuffer, "vkCmdWriteTimestamp");
    StartReadObject(queryPool, "vkCmdWriteTimestamp");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdWriteTimestamp(
    VkCommandBuffer                             commandBuffer,
    VkPipelineStageFlagBits                     pipelineStage,
    VkQueryPool                                 queryPool,
    uint32_t                                    query) {
    FinishWriteObject(commandBuffer, "vkCmdWriteTimestamp");
    FinishReadObject(queryPool, "vkCmdWriteTimestamp");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdCopyQueryPoolResults(
    VkCommandBuffer                             commandBuffer,
    VkQueryPool                                 queryPool,
    uint32_t                                    firstQuery,
    uint32_t                                    queryCount,
    VkBuffer                                    dstBuffer,
    VkDeviceSize                                dstOffset,
    VkDeviceSize                                stride,
    VkQueryResultFlags                          flags) {
    StartWriteObject(commandBuffer, "vkCmdCopyQueryPoolResults");
    StartReadObject(queryPool, "vkCmdCopyQueryPoolResults");
    StartReadObject(dstBuffer, "vkCmdCopyQueryPoolResults");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdCopyQueryPoolResults(
    VkCommandBuffer                             commandBuffer,
    VkQueryPool                                 queryPool,
    uint32_t                                    firstQuery,
    uint32_t                                    queryCount,
    VkBuffer                                    dstBuffer,
    VkDeviceSize                                dstOffset,
    VkDeviceSize                                stride,
    VkQueryResultFlags                          flags) {
    FinishWriteObject(commandBuffer, "vkCmdCopyQueryPoolResults");
    FinishReadObject(queryPool, "vkCmdCopyQueryPoolResults");
    FinishReadObject(dstBuffer, "vkCmdCopyQueryPoolResults");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdPushConstants(
    VkCommandBuffer                             commandBuffer,
    VkPipelineLayout                            layout,
    VkShaderStageFlags                          stageFlags,
    uint32_t                                    offset,
    uint32_t                                    size,
    const void*                                 pValues) {
    StartWriteObject(commandBuffer, "vkCmdPushConstants");
    StartReadObject(layout, "vkCmdPushConstants");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdPushConstants(
    VkCommandBuffer                             commandBuffer,
    VkPipelineLayout                            layout,
    VkShaderStageFlags                          stageFlags,
    uint32_t                                    offset,
    uint32_t                                    size,
    const void*                                 pValues) {
    FinishWriteObject(commandBuffer, "vkCmdPushConstants");
    FinishReadObject(layout, "vkCmdPushConstants");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdBeginRenderPass(
    VkCommandBuffer                             commandBuffer,
    const VkRenderPassBeginInfo*                pRenderPassBegin,
    VkSubpassContents                           contents) {
    StartWriteObject(commandBuffer, "vkCmdBeginRenderPass");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdBeginRenderPass(
    VkCommandBuffer                             commandBuffer,
    const VkRenderPassBeginInfo*                pRenderPassBegin,
    VkSubpassContents                           contents) {
    FinishWriteObject(commandBuffer, "vkCmdBeginRenderPass");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdNextSubpass(
    VkCommandBuffer                             commandBuffer,
    VkSubpassContents                           contents) {
    StartWriteObject(commandBuffer, "vkCmdNextSubpass");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdNextSubpass(
    VkCommandBuffer                             commandBuffer,
    VkSubpassContents                           contents) {
    FinishWriteObject(commandBuffer, "vkCmdNextSubpass");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdEndRenderPass(
    VkCommandBuffer                             commandBuffer) {
    StartWriteObject(commandBuffer, "vkCmdEndRenderPass");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdEndRenderPass(
    VkCommandBuffer                             commandBuffer) {
    FinishWriteObject(commandBuffer, "vkCmdEndRenderPass");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdExecuteCommands(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    commandBufferCount,
    const VkCommandBuffer*                      pCommandBuffers) {
    StartWriteObject(commandBuffer, "vkCmdExecuteCommands");
    if (pCommandBuffers) {
        for (uint32_t index = 0; index < commandBufferCount; index++) {
            StartReadObject(pCommandBuffers[index], "vkCmdExecuteCommands");
        }
    }
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdExecuteCommands(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    commandBufferCount,
    const VkCommandBuffer*                      pCommandBuffers) {
    FinishWriteObject(commandBuffer, "vkCmdExecuteCommands");
    if (pCommandBuffers) {
        for (uint32_t index = 0; index < commandBufferCount; index++) {
            FinishReadObject(pCommandBuffers[index], "vkCmdExecuteCommands");
        }
    }
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordBindBufferMemory2(
    VkDevice                                    device,
    uint32_t                                    bindInfoCount,
    const VkBindBufferMemoryInfo*               pBindInfos) {
    StartReadObjectParentInstance(device, "vkBindBufferMemory2");
}

void ThreadSafety::PostCallRecordBindBufferMemory2(
    VkDevice                                    device,
    uint32_t                                    bindInfoCount,
    const VkBindBufferMemoryInfo*               pBindInfos,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkBindBufferMemory2");
}

void ThreadSafety::PreCallRecordBindImageMemory2(
    VkDevice                                    device,
    uint32_t                                    bindInfoCount,
    const VkBindImageMemoryInfo*                pBindInfos) {
    StartReadObjectParentInstance(device, "vkBindImageMemory2");
}

void ThreadSafety::PostCallRecordBindImageMemory2(
    VkDevice                                    device,
    uint32_t                                    bindInfoCount,
    const VkBindImageMemoryInfo*                pBindInfos,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkBindImageMemory2");
}

void ThreadSafety::PreCallRecordGetDeviceGroupPeerMemoryFeatures(
    VkDevice                                    device,
    uint32_t                                    heapIndex,
    uint32_t                                    localDeviceIndex,
    uint32_t                                    remoteDeviceIndex,
    VkPeerMemoryFeatureFlags*                   pPeerMemoryFeatures) {
    StartReadObjectParentInstance(device, "vkGetDeviceGroupPeerMemoryFeatures");
}

void ThreadSafety::PostCallRecordGetDeviceGroupPeerMemoryFeatures(
    VkDevice                                    device,
    uint32_t                                    heapIndex,
    uint32_t                                    localDeviceIndex,
    uint32_t                                    remoteDeviceIndex,
    VkPeerMemoryFeatureFlags*                   pPeerMemoryFeatures) {
    FinishReadObjectParentInstance(device, "vkGetDeviceGroupPeerMemoryFeatures");
}

void ThreadSafety::PreCallRecordCmdSetDeviceMask(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    deviceMask) {
    StartWriteObject(commandBuffer, "vkCmdSetDeviceMask");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetDeviceMask(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    deviceMask) {
    FinishWriteObject(commandBuffer, "vkCmdSetDeviceMask");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdDispatchBase(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    baseGroupX,
    uint32_t                                    baseGroupY,
    uint32_t                                    baseGroupZ,
    uint32_t                                    groupCountX,
    uint32_t                                    groupCountY,
    uint32_t                                    groupCountZ) {
    StartWriteObject(commandBuffer, "vkCmdDispatchBase");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdDispatchBase(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    baseGroupX,
    uint32_t                                    baseGroupY,
    uint32_t                                    baseGroupZ,
    uint32_t                                    groupCountX,
    uint32_t                                    groupCountY,
    uint32_t                                    groupCountZ) {
    FinishWriteObject(commandBuffer, "vkCmdDispatchBase");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordEnumeratePhysicalDeviceGroups(
    VkInstance                                  instance,
    uint32_t*                                   pPhysicalDeviceGroupCount,
    VkPhysicalDeviceGroupProperties*            pPhysicalDeviceGroupProperties) {
    StartReadObjectParentInstance(instance, "vkEnumeratePhysicalDeviceGroups");
}

void ThreadSafety::PostCallRecordEnumeratePhysicalDeviceGroups(
    VkInstance                                  instance,
    uint32_t*                                   pPhysicalDeviceGroupCount,
    VkPhysicalDeviceGroupProperties*            pPhysicalDeviceGroupProperties,
    VkResult                                    result) {
    FinishReadObjectParentInstance(instance, "vkEnumeratePhysicalDeviceGroups");
}

void ThreadSafety::PreCallRecordGetImageMemoryRequirements2(
    VkDevice                                    device,
    const VkImageMemoryRequirementsInfo2*       pInfo,
    VkMemoryRequirements2*                      pMemoryRequirements) {
    StartReadObjectParentInstance(device, "vkGetImageMemoryRequirements2");
}

void ThreadSafety::PostCallRecordGetImageMemoryRequirements2(
    VkDevice                                    device,
    const VkImageMemoryRequirementsInfo2*       pInfo,
    VkMemoryRequirements2*                      pMemoryRequirements) {
    FinishReadObjectParentInstance(device, "vkGetImageMemoryRequirements2");
}

void ThreadSafety::PreCallRecordGetBufferMemoryRequirements2(
    VkDevice                                    device,
    const VkBufferMemoryRequirementsInfo2*      pInfo,
    VkMemoryRequirements2*                      pMemoryRequirements) {
    StartReadObjectParentInstance(device, "vkGetBufferMemoryRequirements2");
}

void ThreadSafety::PostCallRecordGetBufferMemoryRequirements2(
    VkDevice                                    device,
    const VkBufferMemoryRequirementsInfo2*      pInfo,
    VkMemoryRequirements2*                      pMemoryRequirements) {
    FinishReadObjectParentInstance(device, "vkGetBufferMemoryRequirements2");
}

void ThreadSafety::PreCallRecordGetImageSparseMemoryRequirements2(
    VkDevice                                    device,
    const VkImageSparseMemoryRequirementsInfo2* pInfo,
    uint32_t*                                   pSparseMemoryRequirementCount,
    VkSparseImageMemoryRequirements2*           pSparseMemoryRequirements) {
    StartReadObjectParentInstance(device, "vkGetImageSparseMemoryRequirements2");
}

void ThreadSafety::PostCallRecordGetImageSparseMemoryRequirements2(
    VkDevice                                    device,
    const VkImageSparseMemoryRequirementsInfo2* pInfo,
    uint32_t*                                   pSparseMemoryRequirementCount,
    VkSparseImageMemoryRequirements2*           pSparseMemoryRequirements) {
    FinishReadObjectParentInstance(device, "vkGetImageSparseMemoryRequirements2");
}

void ThreadSafety::PreCallRecordTrimCommandPool(
    VkDevice                                    device,
    VkCommandPool                               commandPool,
    VkCommandPoolTrimFlags                      flags) {
    StartReadObjectParentInstance(device, "vkTrimCommandPool");
    StartWriteObject(commandPool, "vkTrimCommandPool");
    // Host access to commandPool must be externally synchronized
}

void ThreadSafety::PostCallRecordTrimCommandPool(
    VkDevice                                    device,
    VkCommandPool                               commandPool,
    VkCommandPoolTrimFlags                      flags) {
    FinishReadObjectParentInstance(device, "vkTrimCommandPool");
    FinishWriteObject(commandPool, "vkTrimCommandPool");
    // Host access to commandPool must be externally synchronized
}

void ThreadSafety::PreCallRecordCreateSamplerYcbcrConversion(
    VkDevice                                    device,
    const VkSamplerYcbcrConversionCreateInfo*   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSamplerYcbcrConversion*                   pYcbcrConversion) {
    StartReadObjectParentInstance(device, "vkCreateSamplerYcbcrConversion");
}

void ThreadSafety::PostCallRecordCreateSamplerYcbcrConversion(
    VkDevice                                    device,
    const VkSamplerYcbcrConversionCreateInfo*   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSamplerYcbcrConversion*                   pYcbcrConversion,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkCreateSamplerYcbcrConversion");
    if (result == VK_SUCCESS) {
        CreateObject(*pYcbcrConversion);
    }
}

void ThreadSafety::PreCallRecordDestroySamplerYcbcrConversion(
    VkDevice                                    device,
    VkSamplerYcbcrConversion                    ycbcrConversion,
    const VkAllocationCallbacks*                pAllocator) {
    StartReadObjectParentInstance(device, "vkDestroySamplerYcbcrConversion");
    StartWriteObject(ycbcrConversion, "vkDestroySamplerYcbcrConversion");
    // Host access to ycbcrConversion must be externally synchronized
}

void ThreadSafety::PostCallRecordDestroySamplerYcbcrConversion(
    VkDevice                                    device,
    VkSamplerYcbcrConversion                    ycbcrConversion,
    const VkAllocationCallbacks*                pAllocator) {
    FinishReadObjectParentInstance(device, "vkDestroySamplerYcbcrConversion");
    FinishWriteObject(ycbcrConversion, "vkDestroySamplerYcbcrConversion");
    DestroyObject(ycbcrConversion);
    // Host access to ycbcrConversion must be externally synchronized
}

void ThreadSafety::PreCallRecordCreateDescriptorUpdateTemplate(
    VkDevice                                    device,
    const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDescriptorUpdateTemplate*                 pDescriptorUpdateTemplate) {
    StartReadObjectParentInstance(device, "vkCreateDescriptorUpdateTemplate");
}

void ThreadSafety::PostCallRecordCreateDescriptorUpdateTemplate(
    VkDevice                                    device,
    const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDescriptorUpdateTemplate*                 pDescriptorUpdateTemplate,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkCreateDescriptorUpdateTemplate");
    if (result == VK_SUCCESS) {
        CreateObject(*pDescriptorUpdateTemplate);
    }
}

void ThreadSafety::PreCallRecordDestroyDescriptorUpdateTemplate(
    VkDevice                                    device,
    VkDescriptorUpdateTemplate                  descriptorUpdateTemplate,
    const VkAllocationCallbacks*                pAllocator) {
    StartReadObjectParentInstance(device, "vkDestroyDescriptorUpdateTemplate");
    StartWriteObject(descriptorUpdateTemplate, "vkDestroyDescriptorUpdateTemplate");
    // Host access to descriptorUpdateTemplate must be externally synchronized
}

void ThreadSafety::PostCallRecordDestroyDescriptorUpdateTemplate(
    VkDevice                                    device,
    VkDescriptorUpdateTemplate                  descriptorUpdateTemplate,
    const VkAllocationCallbacks*                pAllocator) {
    FinishReadObjectParentInstance(device, "vkDestroyDescriptorUpdateTemplate");
    FinishWriteObject(descriptorUpdateTemplate, "vkDestroyDescriptorUpdateTemplate");
    DestroyObject(descriptorUpdateTemplate);
    // Host access to descriptorUpdateTemplate must be externally synchronized
}

void ThreadSafety::PreCallRecordGetDescriptorSetLayoutSupport(
    VkDevice                                    device,
    const VkDescriptorSetLayoutCreateInfo*      pCreateInfo,
    VkDescriptorSetLayoutSupport*               pSupport) {
    StartReadObjectParentInstance(device, "vkGetDescriptorSetLayoutSupport");
}

void ThreadSafety::PostCallRecordGetDescriptorSetLayoutSupport(
    VkDevice                                    device,
    const VkDescriptorSetLayoutCreateInfo*      pCreateInfo,
    VkDescriptorSetLayoutSupport*               pSupport) {
    FinishReadObjectParentInstance(device, "vkGetDescriptorSetLayoutSupport");
}

void ThreadSafety::PreCallRecordCmdDrawIndirectCount(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkBuffer                                    countBuffer,
    VkDeviceSize                                countBufferOffset,
    uint32_t                                    maxDrawCount,
    uint32_t                                    stride) {
    StartWriteObject(commandBuffer, "vkCmdDrawIndirectCount");
    StartReadObject(buffer, "vkCmdDrawIndirectCount");
    StartReadObject(countBuffer, "vkCmdDrawIndirectCount");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdDrawIndirectCount(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkBuffer                                    countBuffer,
    VkDeviceSize                                countBufferOffset,
    uint32_t                                    maxDrawCount,
    uint32_t                                    stride) {
    FinishWriteObject(commandBuffer, "vkCmdDrawIndirectCount");
    FinishReadObject(buffer, "vkCmdDrawIndirectCount");
    FinishReadObject(countBuffer, "vkCmdDrawIndirectCount");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdDrawIndexedIndirectCount(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkBuffer                                    countBuffer,
    VkDeviceSize                                countBufferOffset,
    uint32_t                                    maxDrawCount,
    uint32_t                                    stride) {
    StartWriteObject(commandBuffer, "vkCmdDrawIndexedIndirectCount");
    StartReadObject(buffer, "vkCmdDrawIndexedIndirectCount");
    StartReadObject(countBuffer, "vkCmdDrawIndexedIndirectCount");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdDrawIndexedIndirectCount(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkBuffer                                    countBuffer,
    VkDeviceSize                                countBufferOffset,
    uint32_t                                    maxDrawCount,
    uint32_t                                    stride) {
    FinishWriteObject(commandBuffer, "vkCmdDrawIndexedIndirectCount");
    FinishReadObject(buffer, "vkCmdDrawIndexedIndirectCount");
    FinishReadObject(countBuffer, "vkCmdDrawIndexedIndirectCount");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCreateRenderPass2(
    VkDevice                                    device,
    const VkRenderPassCreateInfo2*              pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkRenderPass*                               pRenderPass) {
    StartReadObjectParentInstance(device, "vkCreateRenderPass2");
}

void ThreadSafety::PostCallRecordCreateRenderPass2(
    VkDevice                                    device,
    const VkRenderPassCreateInfo2*              pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkRenderPass*                               pRenderPass,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkCreateRenderPass2");
    if (result == VK_SUCCESS) {
        CreateObject(*pRenderPass);
    }
}

void ThreadSafety::PreCallRecordCmdBeginRenderPass2(
    VkCommandBuffer                             commandBuffer,
    const VkRenderPassBeginInfo*                pRenderPassBegin,
    const VkSubpassBeginInfo*                   pSubpassBeginInfo) {
    StartWriteObject(commandBuffer, "vkCmdBeginRenderPass2");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdBeginRenderPass2(
    VkCommandBuffer                             commandBuffer,
    const VkRenderPassBeginInfo*                pRenderPassBegin,
    const VkSubpassBeginInfo*                   pSubpassBeginInfo) {
    FinishWriteObject(commandBuffer, "vkCmdBeginRenderPass2");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdNextSubpass2(
    VkCommandBuffer                             commandBuffer,
    const VkSubpassBeginInfo*                   pSubpassBeginInfo,
    const VkSubpassEndInfo*                     pSubpassEndInfo) {
    StartWriteObject(commandBuffer, "vkCmdNextSubpass2");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdNextSubpass2(
    VkCommandBuffer                             commandBuffer,
    const VkSubpassBeginInfo*                   pSubpassBeginInfo,
    const VkSubpassEndInfo*                     pSubpassEndInfo) {
    FinishWriteObject(commandBuffer, "vkCmdNextSubpass2");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdEndRenderPass2(
    VkCommandBuffer                             commandBuffer,
    const VkSubpassEndInfo*                     pSubpassEndInfo) {
    StartWriteObject(commandBuffer, "vkCmdEndRenderPass2");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdEndRenderPass2(
    VkCommandBuffer                             commandBuffer,
    const VkSubpassEndInfo*                     pSubpassEndInfo) {
    FinishWriteObject(commandBuffer, "vkCmdEndRenderPass2");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordResetQueryPool(
    VkDevice                                    device,
    VkQueryPool                                 queryPool,
    uint32_t                                    firstQuery,
    uint32_t                                    queryCount) {
    StartReadObjectParentInstance(device, "vkResetQueryPool");
    StartReadObject(queryPool, "vkResetQueryPool");
}

void ThreadSafety::PostCallRecordResetQueryPool(
    VkDevice                                    device,
    VkQueryPool                                 queryPool,
    uint32_t                                    firstQuery,
    uint32_t                                    queryCount) {
    FinishReadObjectParentInstance(device, "vkResetQueryPool");
    FinishReadObject(queryPool, "vkResetQueryPool");
}

void ThreadSafety::PreCallRecordGetSemaphoreCounterValue(
    VkDevice                                    device,
    VkSemaphore                                 semaphore,
    uint64_t*                                   pValue) {
    StartReadObjectParentInstance(device, "vkGetSemaphoreCounterValue");
    StartReadObject(semaphore, "vkGetSemaphoreCounterValue");
}

void ThreadSafety::PostCallRecordGetSemaphoreCounterValue(
    VkDevice                                    device,
    VkSemaphore                                 semaphore,
    uint64_t*                                   pValue,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkGetSemaphoreCounterValue");
    FinishReadObject(semaphore, "vkGetSemaphoreCounterValue");
}

void ThreadSafety::PreCallRecordWaitSemaphores(
    VkDevice                                    device,
    const VkSemaphoreWaitInfo*                  pWaitInfo,
    uint64_t                                    timeout) {
    StartReadObjectParentInstance(device, "vkWaitSemaphores");
}

void ThreadSafety::PostCallRecordWaitSemaphores(
    VkDevice                                    device,
    const VkSemaphoreWaitInfo*                  pWaitInfo,
    uint64_t                                    timeout,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkWaitSemaphores");
}

void ThreadSafety::PreCallRecordSignalSemaphore(
    VkDevice                                    device,
    const VkSemaphoreSignalInfo*                pSignalInfo) {
    StartReadObjectParentInstance(device, "vkSignalSemaphore");
}

void ThreadSafety::PostCallRecordSignalSemaphore(
    VkDevice                                    device,
    const VkSemaphoreSignalInfo*                pSignalInfo,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkSignalSemaphore");
}

void ThreadSafety::PreCallRecordGetBufferDeviceAddress(
    VkDevice                                    device,
    const VkBufferDeviceAddressInfo*            pInfo) {
    StartReadObjectParentInstance(device, "vkGetBufferDeviceAddress");
}

void ThreadSafety::PostCallRecordGetBufferDeviceAddress(
    VkDevice                                    device,
    const VkBufferDeviceAddressInfo*            pInfo,
    VkDeviceAddress                             result) {
    FinishReadObjectParentInstance(device, "vkGetBufferDeviceAddress");
}

void ThreadSafety::PreCallRecordGetBufferOpaqueCaptureAddress(
    VkDevice                                    device,
    const VkBufferDeviceAddressInfo*            pInfo) {
    StartReadObjectParentInstance(device, "vkGetBufferOpaqueCaptureAddress");
}

void ThreadSafety::PostCallRecordGetBufferOpaqueCaptureAddress(
    VkDevice                                    device,
    const VkBufferDeviceAddressInfo*            pInfo) {
    FinishReadObjectParentInstance(device, "vkGetBufferOpaqueCaptureAddress");
}

void ThreadSafety::PreCallRecordGetDeviceMemoryOpaqueCaptureAddress(
    VkDevice                                    device,
    const VkDeviceMemoryOpaqueCaptureAddressInfo* pInfo) {
    StartReadObjectParentInstance(device, "vkGetDeviceMemoryOpaqueCaptureAddress");
}

void ThreadSafety::PostCallRecordGetDeviceMemoryOpaqueCaptureAddress(
    VkDevice                                    device,
    const VkDeviceMemoryOpaqueCaptureAddressInfo* pInfo) {
    FinishReadObjectParentInstance(device, "vkGetDeviceMemoryOpaqueCaptureAddress");
}

void ThreadSafety::PreCallRecordCreatePrivateDataSlot(
    VkDevice                                    device,
    const VkPrivateDataSlotCreateInfo*          pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkPrivateDataSlot*                          pPrivateDataSlot) {
    StartReadObjectParentInstance(device, "vkCreatePrivateDataSlot");
}

void ThreadSafety::PostCallRecordCreatePrivateDataSlot(
    VkDevice                                    device,
    const VkPrivateDataSlotCreateInfo*          pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkPrivateDataSlot*                          pPrivateDataSlot,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkCreatePrivateDataSlot");
    if (result == VK_SUCCESS) {
        CreateObject(*pPrivateDataSlot);
    }
}

void ThreadSafety::PreCallRecordDestroyPrivateDataSlot(
    VkDevice                                    device,
    VkPrivateDataSlot                           privateDataSlot,
    const VkAllocationCallbacks*                pAllocator) {
    StartReadObjectParentInstance(device, "vkDestroyPrivateDataSlot");
    StartWriteObject(privateDataSlot, "vkDestroyPrivateDataSlot");
    // Host access to privateDataSlot must be externally synchronized
}

void ThreadSafety::PostCallRecordDestroyPrivateDataSlot(
    VkDevice                                    device,
    VkPrivateDataSlot                           privateDataSlot,
    const VkAllocationCallbacks*                pAllocator) {
    FinishReadObjectParentInstance(device, "vkDestroyPrivateDataSlot");
    FinishWriteObject(privateDataSlot, "vkDestroyPrivateDataSlot");
    DestroyObject(privateDataSlot);
    // Host access to privateDataSlot must be externally synchronized
}

void ThreadSafety::PreCallRecordSetPrivateData(
    VkDevice                                    device,
    VkObjectType                                objectType,
    uint64_t                                    objectHandle,
    VkPrivateDataSlot                           privateDataSlot,
    uint64_t                                    data) {
    StartReadObjectParentInstance(device, "vkSetPrivateData");
    StartReadObject(privateDataSlot, "vkSetPrivateData");
}

void ThreadSafety::PostCallRecordSetPrivateData(
    VkDevice                                    device,
    VkObjectType                                objectType,
    uint64_t                                    objectHandle,
    VkPrivateDataSlot                           privateDataSlot,
    uint64_t                                    data,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkSetPrivateData");
    FinishReadObject(privateDataSlot, "vkSetPrivateData");
}

void ThreadSafety::PreCallRecordGetPrivateData(
    VkDevice                                    device,
    VkObjectType                                objectType,
    uint64_t                                    objectHandle,
    VkPrivateDataSlot                           privateDataSlot,
    uint64_t*                                   pData) {
    StartReadObjectParentInstance(device, "vkGetPrivateData");
    StartReadObject(privateDataSlot, "vkGetPrivateData");
}

void ThreadSafety::PostCallRecordGetPrivateData(
    VkDevice                                    device,
    VkObjectType                                objectType,
    uint64_t                                    objectHandle,
    VkPrivateDataSlot                           privateDataSlot,
    uint64_t*                                   pData) {
    FinishReadObjectParentInstance(device, "vkGetPrivateData");
    FinishReadObject(privateDataSlot, "vkGetPrivateData");
}

void ThreadSafety::PreCallRecordCmdSetEvent2(
    VkCommandBuffer                             commandBuffer,
    VkEvent                                     event,
    const VkDependencyInfo*                     pDependencyInfo) {
    StartWriteObject(commandBuffer, "vkCmdSetEvent2");
    StartReadObject(event, "vkCmdSetEvent2");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetEvent2(
    VkCommandBuffer                             commandBuffer,
    VkEvent                                     event,
    const VkDependencyInfo*                     pDependencyInfo) {
    FinishWriteObject(commandBuffer, "vkCmdSetEvent2");
    FinishReadObject(event, "vkCmdSetEvent2");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdResetEvent2(
    VkCommandBuffer                             commandBuffer,
    VkEvent                                     event,
    VkPipelineStageFlags2                       stageMask) {
    StartWriteObject(commandBuffer, "vkCmdResetEvent2");
    StartReadObject(event, "vkCmdResetEvent2");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdResetEvent2(
    VkCommandBuffer                             commandBuffer,
    VkEvent                                     event,
    VkPipelineStageFlags2                       stageMask) {
    FinishWriteObject(commandBuffer, "vkCmdResetEvent2");
    FinishReadObject(event, "vkCmdResetEvent2");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdWaitEvents2(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    eventCount,
    const VkEvent*                              pEvents,
    const VkDependencyInfo*                     pDependencyInfos) {
    StartWriteObject(commandBuffer, "vkCmdWaitEvents2");
    if (pEvents) {
        for (uint32_t index = 0; index < eventCount; index++) {
            StartReadObject(pEvents[index], "vkCmdWaitEvents2");
        }
    }
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdWaitEvents2(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    eventCount,
    const VkEvent*                              pEvents,
    const VkDependencyInfo*                     pDependencyInfos) {
    FinishWriteObject(commandBuffer, "vkCmdWaitEvents2");
    if (pEvents) {
        for (uint32_t index = 0; index < eventCount; index++) {
            FinishReadObject(pEvents[index], "vkCmdWaitEvents2");
        }
    }
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdPipelineBarrier2(
    VkCommandBuffer                             commandBuffer,
    const VkDependencyInfo*                     pDependencyInfo) {
    StartWriteObject(commandBuffer, "vkCmdPipelineBarrier2");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdPipelineBarrier2(
    VkCommandBuffer                             commandBuffer,
    const VkDependencyInfo*                     pDependencyInfo) {
    FinishWriteObject(commandBuffer, "vkCmdPipelineBarrier2");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdWriteTimestamp2(
    VkCommandBuffer                             commandBuffer,
    VkPipelineStageFlags2                       stage,
    VkQueryPool                                 queryPool,
    uint32_t                                    query) {
    StartWriteObject(commandBuffer, "vkCmdWriteTimestamp2");
    StartReadObject(queryPool, "vkCmdWriteTimestamp2");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdWriteTimestamp2(
    VkCommandBuffer                             commandBuffer,
    VkPipelineStageFlags2                       stage,
    VkQueryPool                                 queryPool,
    uint32_t                                    query) {
    FinishWriteObject(commandBuffer, "vkCmdWriteTimestamp2");
    FinishReadObject(queryPool, "vkCmdWriteTimestamp2");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordQueueSubmit2(
    VkQueue                                     queue,
    uint32_t                                    submitCount,
    const VkSubmitInfo2*                        pSubmits,
    VkFence                                     fence) {
    StartWriteObject(queue, "vkQueueSubmit2");
    StartWriteObject(fence, "vkQueueSubmit2");
    // Host access to queue must be externally synchronized
    // Host access to fence must be externally synchronized
}

void ThreadSafety::PostCallRecordQueueSubmit2(
    VkQueue                                     queue,
    uint32_t                                    submitCount,
    const VkSubmitInfo2*                        pSubmits,
    VkFence                                     fence,
    VkResult                                    result) {
    FinishWriteObject(queue, "vkQueueSubmit2");
    FinishWriteObject(fence, "vkQueueSubmit2");
    // Host access to queue must be externally synchronized
    // Host access to fence must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdCopyBuffer2(
    VkCommandBuffer                             commandBuffer,
    const VkCopyBufferInfo2*                    pCopyBufferInfo) {
    StartWriteObject(commandBuffer, "vkCmdCopyBuffer2");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdCopyBuffer2(
    VkCommandBuffer                             commandBuffer,
    const VkCopyBufferInfo2*                    pCopyBufferInfo) {
    FinishWriteObject(commandBuffer, "vkCmdCopyBuffer2");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdCopyImage2(
    VkCommandBuffer                             commandBuffer,
    const VkCopyImageInfo2*                     pCopyImageInfo) {
    StartWriteObject(commandBuffer, "vkCmdCopyImage2");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdCopyImage2(
    VkCommandBuffer                             commandBuffer,
    const VkCopyImageInfo2*                     pCopyImageInfo) {
    FinishWriteObject(commandBuffer, "vkCmdCopyImage2");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdCopyBufferToImage2(
    VkCommandBuffer                             commandBuffer,
    const VkCopyBufferToImageInfo2*             pCopyBufferToImageInfo) {
    StartWriteObject(commandBuffer, "vkCmdCopyBufferToImage2");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdCopyBufferToImage2(
    VkCommandBuffer                             commandBuffer,
    const VkCopyBufferToImageInfo2*             pCopyBufferToImageInfo) {
    FinishWriteObject(commandBuffer, "vkCmdCopyBufferToImage2");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdCopyImageToBuffer2(
    VkCommandBuffer                             commandBuffer,
    const VkCopyImageToBufferInfo2*             pCopyImageToBufferInfo) {
    StartWriteObject(commandBuffer, "vkCmdCopyImageToBuffer2");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdCopyImageToBuffer2(
    VkCommandBuffer                             commandBuffer,
    const VkCopyImageToBufferInfo2*             pCopyImageToBufferInfo) {
    FinishWriteObject(commandBuffer, "vkCmdCopyImageToBuffer2");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdBlitImage2(
    VkCommandBuffer                             commandBuffer,
    const VkBlitImageInfo2*                     pBlitImageInfo) {
    StartWriteObject(commandBuffer, "vkCmdBlitImage2");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdBlitImage2(
    VkCommandBuffer                             commandBuffer,
    const VkBlitImageInfo2*                     pBlitImageInfo) {
    FinishWriteObject(commandBuffer, "vkCmdBlitImage2");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdResolveImage2(
    VkCommandBuffer                             commandBuffer,
    const VkResolveImageInfo2*                  pResolveImageInfo) {
    StartWriteObject(commandBuffer, "vkCmdResolveImage2");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdResolveImage2(
    VkCommandBuffer                             commandBuffer,
    const VkResolveImageInfo2*                  pResolveImageInfo) {
    FinishWriteObject(commandBuffer, "vkCmdResolveImage2");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdBeginRendering(
    VkCommandBuffer                             commandBuffer,
    const VkRenderingInfo*                      pRenderingInfo) {
    StartWriteObject(commandBuffer, "vkCmdBeginRendering");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdBeginRendering(
    VkCommandBuffer                             commandBuffer,
    const VkRenderingInfo*                      pRenderingInfo) {
    FinishWriteObject(commandBuffer, "vkCmdBeginRendering");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdEndRendering(
    VkCommandBuffer                             commandBuffer) {
    StartWriteObject(commandBuffer, "vkCmdEndRendering");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdEndRendering(
    VkCommandBuffer                             commandBuffer) {
    FinishWriteObject(commandBuffer, "vkCmdEndRendering");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetCullMode(
    VkCommandBuffer                             commandBuffer,
    VkCullModeFlags                             cullMode) {
    StartWriteObject(commandBuffer, "vkCmdSetCullMode");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetCullMode(
    VkCommandBuffer                             commandBuffer,
    VkCullModeFlags                             cullMode) {
    FinishWriteObject(commandBuffer, "vkCmdSetCullMode");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetFrontFace(
    VkCommandBuffer                             commandBuffer,
    VkFrontFace                                 frontFace) {
    StartWriteObject(commandBuffer, "vkCmdSetFrontFace");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetFrontFace(
    VkCommandBuffer                             commandBuffer,
    VkFrontFace                                 frontFace) {
    FinishWriteObject(commandBuffer, "vkCmdSetFrontFace");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetPrimitiveTopology(
    VkCommandBuffer                             commandBuffer,
    VkPrimitiveTopology                         primitiveTopology) {
    StartWriteObject(commandBuffer, "vkCmdSetPrimitiveTopology");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetPrimitiveTopology(
    VkCommandBuffer                             commandBuffer,
    VkPrimitiveTopology                         primitiveTopology) {
    FinishWriteObject(commandBuffer, "vkCmdSetPrimitiveTopology");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetViewportWithCount(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    viewportCount,
    const VkViewport*                           pViewports) {
    StartWriteObject(commandBuffer, "vkCmdSetViewportWithCount");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetViewportWithCount(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    viewportCount,
    const VkViewport*                           pViewports) {
    FinishWriteObject(commandBuffer, "vkCmdSetViewportWithCount");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetScissorWithCount(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    scissorCount,
    const VkRect2D*                             pScissors) {
    StartWriteObject(commandBuffer, "vkCmdSetScissorWithCount");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetScissorWithCount(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    scissorCount,
    const VkRect2D*                             pScissors) {
    FinishWriteObject(commandBuffer, "vkCmdSetScissorWithCount");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdBindVertexBuffers2(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstBinding,
    uint32_t                                    bindingCount,
    const VkBuffer*                             pBuffers,
    const VkDeviceSize*                         pOffsets,
    const VkDeviceSize*                         pSizes,
    const VkDeviceSize*                         pStrides) {
    StartWriteObject(commandBuffer, "vkCmdBindVertexBuffers2");
    if (pBuffers) {
        for (uint32_t index = 0; index < bindingCount; index++) {
            StartReadObject(pBuffers[index], "vkCmdBindVertexBuffers2");
        }
    }
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdBindVertexBuffers2(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstBinding,
    uint32_t                                    bindingCount,
    const VkBuffer*                             pBuffers,
    const VkDeviceSize*                         pOffsets,
    const VkDeviceSize*                         pSizes,
    const VkDeviceSize*                         pStrides) {
    FinishWriteObject(commandBuffer, "vkCmdBindVertexBuffers2");
    if (pBuffers) {
        for (uint32_t index = 0; index < bindingCount; index++) {
            FinishReadObject(pBuffers[index], "vkCmdBindVertexBuffers2");
        }
    }
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetDepthTestEnable(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    depthTestEnable) {
    StartWriteObject(commandBuffer, "vkCmdSetDepthTestEnable");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetDepthTestEnable(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    depthTestEnable) {
    FinishWriteObject(commandBuffer, "vkCmdSetDepthTestEnable");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetDepthWriteEnable(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    depthWriteEnable) {
    StartWriteObject(commandBuffer, "vkCmdSetDepthWriteEnable");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetDepthWriteEnable(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    depthWriteEnable) {
    FinishWriteObject(commandBuffer, "vkCmdSetDepthWriteEnable");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetDepthCompareOp(
    VkCommandBuffer                             commandBuffer,
    VkCompareOp                                 depthCompareOp) {
    StartWriteObject(commandBuffer, "vkCmdSetDepthCompareOp");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetDepthCompareOp(
    VkCommandBuffer                             commandBuffer,
    VkCompareOp                                 depthCompareOp) {
    FinishWriteObject(commandBuffer, "vkCmdSetDepthCompareOp");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetDepthBoundsTestEnable(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    depthBoundsTestEnable) {
    StartWriteObject(commandBuffer, "vkCmdSetDepthBoundsTestEnable");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetDepthBoundsTestEnable(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    depthBoundsTestEnable) {
    FinishWriteObject(commandBuffer, "vkCmdSetDepthBoundsTestEnable");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetStencilTestEnable(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    stencilTestEnable) {
    StartWriteObject(commandBuffer, "vkCmdSetStencilTestEnable");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetStencilTestEnable(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    stencilTestEnable) {
    FinishWriteObject(commandBuffer, "vkCmdSetStencilTestEnable");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetStencilOp(
    VkCommandBuffer                             commandBuffer,
    VkStencilFaceFlags                          faceMask,
    VkStencilOp                                 failOp,
    VkStencilOp                                 passOp,
    VkStencilOp                                 depthFailOp,
    VkCompareOp                                 compareOp) {
    StartWriteObject(commandBuffer, "vkCmdSetStencilOp");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetStencilOp(
    VkCommandBuffer                             commandBuffer,
    VkStencilFaceFlags                          faceMask,
    VkStencilOp                                 failOp,
    VkStencilOp                                 passOp,
    VkStencilOp                                 depthFailOp,
    VkCompareOp                                 compareOp) {
    FinishWriteObject(commandBuffer, "vkCmdSetStencilOp");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetRasterizerDiscardEnable(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    rasterizerDiscardEnable) {
    StartWriteObject(commandBuffer, "vkCmdSetRasterizerDiscardEnable");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetRasterizerDiscardEnable(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    rasterizerDiscardEnable) {
    FinishWriteObject(commandBuffer, "vkCmdSetRasterizerDiscardEnable");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetDepthBiasEnable(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    depthBiasEnable) {
    StartWriteObject(commandBuffer, "vkCmdSetDepthBiasEnable");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetDepthBiasEnable(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    depthBiasEnable) {
    FinishWriteObject(commandBuffer, "vkCmdSetDepthBiasEnable");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetPrimitiveRestartEnable(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    primitiveRestartEnable) {
    StartWriteObject(commandBuffer, "vkCmdSetPrimitiveRestartEnable");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetPrimitiveRestartEnable(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    primitiveRestartEnable) {
    FinishWriteObject(commandBuffer, "vkCmdSetPrimitiveRestartEnable");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordGetDeviceBufferMemoryRequirements(
    VkDevice                                    device,
    const VkDeviceBufferMemoryRequirements*     pInfo,
    VkMemoryRequirements2*                      pMemoryRequirements) {
    StartReadObjectParentInstance(device, "vkGetDeviceBufferMemoryRequirements");
}

void ThreadSafety::PostCallRecordGetDeviceBufferMemoryRequirements(
    VkDevice                                    device,
    const VkDeviceBufferMemoryRequirements*     pInfo,
    VkMemoryRequirements2*                      pMemoryRequirements) {
    FinishReadObjectParentInstance(device, "vkGetDeviceBufferMemoryRequirements");
}

void ThreadSafety::PreCallRecordGetDeviceImageMemoryRequirements(
    VkDevice                                    device,
    const VkDeviceImageMemoryRequirements*      pInfo,
    VkMemoryRequirements2*                      pMemoryRequirements) {
    StartReadObjectParentInstance(device, "vkGetDeviceImageMemoryRequirements");
}

void ThreadSafety::PostCallRecordGetDeviceImageMemoryRequirements(
    VkDevice                                    device,
    const VkDeviceImageMemoryRequirements*      pInfo,
    VkMemoryRequirements2*                      pMemoryRequirements) {
    FinishReadObjectParentInstance(device, "vkGetDeviceImageMemoryRequirements");
}

void ThreadSafety::PreCallRecordGetDeviceImageSparseMemoryRequirements(
    VkDevice                                    device,
    const VkDeviceImageMemoryRequirements*      pInfo,
    uint32_t*                                   pSparseMemoryRequirementCount,
    VkSparseImageMemoryRequirements2*           pSparseMemoryRequirements) {
    StartReadObjectParentInstance(device, "vkGetDeviceImageSparseMemoryRequirements");
}

void ThreadSafety::PostCallRecordGetDeviceImageSparseMemoryRequirements(
    VkDevice                                    device,
    const VkDeviceImageMemoryRequirements*      pInfo,
    uint32_t*                                   pSparseMemoryRequirementCount,
    VkSparseImageMemoryRequirements2*           pSparseMemoryRequirements) {
    FinishReadObjectParentInstance(device, "vkGetDeviceImageSparseMemoryRequirements");
}

void ThreadSafety::PreCallRecordDestroySurfaceKHR(
    VkInstance                                  instance,
    VkSurfaceKHR                                surface,
    const VkAllocationCallbacks*                pAllocator) {
    StartReadObjectParentInstance(instance, "vkDestroySurfaceKHR");
    StartWriteObjectParentInstance(surface, "vkDestroySurfaceKHR");
    // Host access to surface must be externally synchronized
}

void ThreadSafety::PostCallRecordDestroySurfaceKHR(
    VkInstance                                  instance,
    VkSurfaceKHR                                surface,
    const VkAllocationCallbacks*                pAllocator) {
    FinishReadObjectParentInstance(instance, "vkDestroySurfaceKHR");
    FinishWriteObjectParentInstance(surface, "vkDestroySurfaceKHR");
    DestroyObjectParentInstance(surface);
    // Host access to surface must be externally synchronized
}

void ThreadSafety::PreCallRecordGetPhysicalDeviceSurfaceSupportKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    queueFamilyIndex,
    VkSurfaceKHR                                surface,
    VkBool32*                                   pSupported) {
    StartReadObjectParentInstance(surface, "vkGetPhysicalDeviceSurfaceSupportKHR");
}

void ThreadSafety::PostCallRecordGetPhysicalDeviceSurfaceSupportKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    queueFamilyIndex,
    VkSurfaceKHR                                surface,
    VkBool32*                                   pSupported,
    VkResult                                    result) {
    FinishReadObjectParentInstance(surface, "vkGetPhysicalDeviceSurfaceSupportKHR");
}

void ThreadSafety::PreCallRecordGetPhysicalDeviceSurfaceCapabilitiesKHR(
    VkPhysicalDevice                            physicalDevice,
    VkSurfaceKHR                                surface,
    VkSurfaceCapabilitiesKHR*                   pSurfaceCapabilities) {
    StartReadObjectParentInstance(surface, "vkGetPhysicalDeviceSurfaceCapabilitiesKHR");
}

void ThreadSafety::PostCallRecordGetPhysicalDeviceSurfaceCapabilitiesKHR(
    VkPhysicalDevice                            physicalDevice,
    VkSurfaceKHR                                surface,
    VkSurfaceCapabilitiesKHR*                   pSurfaceCapabilities,
    VkResult                                    result) {
    FinishReadObjectParentInstance(surface, "vkGetPhysicalDeviceSurfaceCapabilitiesKHR");
}

void ThreadSafety::PreCallRecordGetPhysicalDeviceSurfaceFormatsKHR(
    VkPhysicalDevice                            physicalDevice,
    VkSurfaceKHR                                surface,
    uint32_t*                                   pSurfaceFormatCount,
    VkSurfaceFormatKHR*                         pSurfaceFormats) {
    StartReadObjectParentInstance(surface, "vkGetPhysicalDeviceSurfaceFormatsKHR");
}

void ThreadSafety::PostCallRecordGetPhysicalDeviceSurfaceFormatsKHR(
    VkPhysicalDevice                            physicalDevice,
    VkSurfaceKHR                                surface,
    uint32_t*                                   pSurfaceFormatCount,
    VkSurfaceFormatKHR*                         pSurfaceFormats,
    VkResult                                    result) {
    FinishReadObjectParentInstance(surface, "vkGetPhysicalDeviceSurfaceFormatsKHR");
}

void ThreadSafety::PreCallRecordGetPhysicalDeviceSurfacePresentModesKHR(
    VkPhysicalDevice                            physicalDevice,
    VkSurfaceKHR                                surface,
    uint32_t*                                   pPresentModeCount,
    VkPresentModeKHR*                           pPresentModes) {
    StartReadObjectParentInstance(surface, "vkGetPhysicalDeviceSurfacePresentModesKHR");
}

void ThreadSafety::PostCallRecordGetPhysicalDeviceSurfacePresentModesKHR(
    VkPhysicalDevice                            physicalDevice,
    VkSurfaceKHR                                surface,
    uint32_t*                                   pPresentModeCount,
    VkPresentModeKHR*                           pPresentModes,
    VkResult                                    result) {
    FinishReadObjectParentInstance(surface, "vkGetPhysicalDeviceSurfacePresentModesKHR");
}

void ThreadSafety::PreCallRecordCreateSwapchainKHR(
    VkDevice                                    device,
    const VkSwapchainCreateInfoKHR*             pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSwapchainKHR*                             pSwapchain) {
    StartReadObjectParentInstance(device, "vkCreateSwapchainKHR");
        StartWriteObjectParentInstance(pCreateInfo->surface, "vkCreateSwapchainKHR");
        StartWriteObjectParentInstance(pCreateInfo->oldSwapchain, "vkCreateSwapchainKHR");
    // Host access to pCreateInfo->surface,pCreateInfo->oldSwapchain must be externally synchronized
}

void ThreadSafety::PostCallRecordCreateSwapchainKHR(
    VkDevice                                    device,
    const VkSwapchainCreateInfoKHR*             pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSwapchainKHR*                             pSwapchain,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkCreateSwapchainKHR");
        FinishWriteObjectParentInstance(pCreateInfo->surface, "vkCreateSwapchainKHR");
        FinishWriteObjectParentInstance(pCreateInfo->oldSwapchain, "vkCreateSwapchainKHR");
    if (result == VK_SUCCESS) {
        CreateObjectParentInstance(*pSwapchain);
    }
    // Host access to pCreateInfo->surface,pCreateInfo->oldSwapchain must be externally synchronized
}

void ThreadSafety::PreCallRecordAcquireNextImageKHR(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    uint64_t                                    timeout,
    VkSemaphore                                 semaphore,
    VkFence                                     fence,
    uint32_t*                                   pImageIndex) {
    StartReadObjectParentInstance(device, "vkAcquireNextImageKHR");
    StartWriteObjectParentInstance(swapchain, "vkAcquireNextImageKHR");
    StartWriteObject(semaphore, "vkAcquireNextImageKHR");
    StartWriteObject(fence, "vkAcquireNextImageKHR");
    // Host access to swapchain must be externally synchronized
    // Host access to semaphore must be externally synchronized
    // Host access to fence must be externally synchronized
}

void ThreadSafety::PostCallRecordAcquireNextImageKHR(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    uint64_t                                    timeout,
    VkSemaphore                                 semaphore,
    VkFence                                     fence,
    uint32_t*                                   pImageIndex,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkAcquireNextImageKHR");
    FinishWriteObjectParentInstance(swapchain, "vkAcquireNextImageKHR");
    FinishWriteObject(semaphore, "vkAcquireNextImageKHR");
    FinishWriteObject(fence, "vkAcquireNextImageKHR");
    // Host access to swapchain must be externally synchronized
    // Host access to semaphore must be externally synchronized
    // Host access to fence must be externally synchronized
}

void ThreadSafety::PreCallRecordGetDeviceGroupPresentCapabilitiesKHR(
    VkDevice                                    device,
    VkDeviceGroupPresentCapabilitiesKHR*        pDeviceGroupPresentCapabilities) {
    StartReadObjectParentInstance(device, "vkGetDeviceGroupPresentCapabilitiesKHR");
}

void ThreadSafety::PostCallRecordGetDeviceGroupPresentCapabilitiesKHR(
    VkDevice                                    device,
    VkDeviceGroupPresentCapabilitiesKHR*        pDeviceGroupPresentCapabilities,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkGetDeviceGroupPresentCapabilitiesKHR");
}

void ThreadSafety::PreCallRecordGetDeviceGroupSurfacePresentModesKHR(
    VkDevice                                    device,
    VkSurfaceKHR                                surface,
    VkDeviceGroupPresentModeFlagsKHR*           pModes) {
    StartReadObjectParentInstance(device, "vkGetDeviceGroupSurfacePresentModesKHR");
    StartWriteObjectParentInstance(surface, "vkGetDeviceGroupSurfacePresentModesKHR");
    // Host access to surface must be externally synchronized
}

void ThreadSafety::PostCallRecordGetDeviceGroupSurfacePresentModesKHR(
    VkDevice                                    device,
    VkSurfaceKHR                                surface,
    VkDeviceGroupPresentModeFlagsKHR*           pModes,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkGetDeviceGroupSurfacePresentModesKHR");
    FinishWriteObjectParentInstance(surface, "vkGetDeviceGroupSurfacePresentModesKHR");
    // Host access to surface must be externally synchronized
}

void ThreadSafety::PreCallRecordGetPhysicalDevicePresentRectanglesKHR(
    VkPhysicalDevice                            physicalDevice,
    VkSurfaceKHR                                surface,
    uint32_t*                                   pRectCount,
    VkRect2D*                                   pRects) {
    StartWriteObjectParentInstance(surface, "vkGetPhysicalDevicePresentRectanglesKHR");
    // Host access to surface must be externally synchronized
}

void ThreadSafety::PostCallRecordGetPhysicalDevicePresentRectanglesKHR(
    VkPhysicalDevice                            physicalDevice,
    VkSurfaceKHR                                surface,
    uint32_t*                                   pRectCount,
    VkRect2D*                                   pRects,
    VkResult                                    result) {
    FinishWriteObjectParentInstance(surface, "vkGetPhysicalDevicePresentRectanglesKHR");
    // Host access to surface must be externally synchronized
}

void ThreadSafety::PreCallRecordAcquireNextImage2KHR(
    VkDevice                                    device,
    const VkAcquireNextImageInfoKHR*            pAcquireInfo,
    uint32_t*                                   pImageIndex) {
    StartReadObjectParentInstance(device, "vkAcquireNextImage2KHR");
}

void ThreadSafety::PostCallRecordAcquireNextImage2KHR(
    VkDevice                                    device,
    const VkAcquireNextImageInfoKHR*            pAcquireInfo,
    uint32_t*                                   pImageIndex,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkAcquireNextImage2KHR");
}

void ThreadSafety::PreCallRecordCreateDisplayModeKHR(
    VkPhysicalDevice                            physicalDevice,
    VkDisplayKHR                                display,
    const VkDisplayModeCreateInfoKHR*           pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDisplayModeKHR*                           pMode) {
    StartWriteObjectParentInstance(display, "vkCreateDisplayModeKHR");
    // Host access to display must be externally synchronized
}

void ThreadSafety::PostCallRecordCreateDisplayModeKHR(
    VkPhysicalDevice                            physicalDevice,
    VkDisplayKHR                                display,
    const VkDisplayModeCreateInfoKHR*           pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDisplayModeKHR*                           pMode,
    VkResult                                    result) {
    FinishWriteObjectParentInstance(display, "vkCreateDisplayModeKHR");
    if (result == VK_SUCCESS) {
        CreateObject(*pMode);
    }
    // Host access to display must be externally synchronized
}

void ThreadSafety::PreCallRecordGetDisplayPlaneCapabilitiesKHR(
    VkPhysicalDevice                            physicalDevice,
    VkDisplayModeKHR                            mode,
    uint32_t                                    planeIndex,
    VkDisplayPlaneCapabilitiesKHR*              pCapabilities) {
    StartWriteObject(mode, "vkGetDisplayPlaneCapabilitiesKHR");
    // Host access to mode must be externally synchronized
}

void ThreadSafety::PostCallRecordGetDisplayPlaneCapabilitiesKHR(
    VkPhysicalDevice                            physicalDevice,
    VkDisplayModeKHR                            mode,
    uint32_t                                    planeIndex,
    VkDisplayPlaneCapabilitiesKHR*              pCapabilities,
    VkResult                                    result) {
    FinishWriteObject(mode, "vkGetDisplayPlaneCapabilitiesKHR");
    // Host access to mode must be externally synchronized
}

void ThreadSafety::PreCallRecordCreateDisplayPlaneSurfaceKHR(
    VkInstance                                  instance,
    const VkDisplaySurfaceCreateInfoKHR*        pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) {
    StartReadObjectParentInstance(instance, "vkCreateDisplayPlaneSurfaceKHR");
}

void ThreadSafety::PostCallRecordCreateDisplayPlaneSurfaceKHR(
    VkInstance                                  instance,
    const VkDisplaySurfaceCreateInfoKHR*        pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface,
    VkResult                                    result) {
    FinishReadObjectParentInstance(instance, "vkCreateDisplayPlaneSurfaceKHR");
    if (result == VK_SUCCESS) {
        CreateObjectParentInstance(*pSurface);
    }
}

void ThreadSafety::PreCallRecordCreateSharedSwapchainsKHR(
    VkDevice                                    device,
    uint32_t                                    swapchainCount,
    const VkSwapchainCreateInfoKHR*             pCreateInfos,
    const VkAllocationCallbacks*                pAllocator,
    VkSwapchainKHR*                             pSwapchains) {
    StartReadObjectParentInstance(device, "vkCreateSharedSwapchainsKHR");
    if (pCreateInfos) {
        for (uint32_t index=0; index < swapchainCount; index++) {
            StartWriteObjectParentInstance(pCreateInfos[index].surface, "vkCreateSharedSwapchainsKHR");
            StartWriteObjectParentInstance(pCreateInfos[index].oldSwapchain, "vkCreateSharedSwapchainsKHR");
        }
    }
    if (pSwapchains) {
        for (uint32_t index = 0; index < swapchainCount; index++) {
            StartReadObjectParentInstance(pSwapchains[index], "vkCreateSharedSwapchainsKHR");
        }
    }
    // Host access to pCreateInfos[].surface,pCreateInfos[].oldSwapchain must be externally synchronized
}

void ThreadSafety::PostCallRecordCreateSharedSwapchainsKHR(
    VkDevice                                    device,
    uint32_t                                    swapchainCount,
    const VkSwapchainCreateInfoKHR*             pCreateInfos,
    const VkAllocationCallbacks*                pAllocator,
    VkSwapchainKHR*                             pSwapchains,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkCreateSharedSwapchainsKHR");
    if (pCreateInfos) {
        for (uint32_t index=0; index < swapchainCount; index++) {
            FinishWriteObjectParentInstance(pCreateInfos[index].surface, "vkCreateSharedSwapchainsKHR");
            FinishWriteObjectParentInstance(pCreateInfos[index].oldSwapchain, "vkCreateSharedSwapchainsKHR");
        }
    }
    if (result == VK_SUCCESS) {
        if (pSwapchains) {
            for (uint32_t index = 0; index < swapchainCount; index++) {
                CreateObjectParentInstance(pSwapchains[index]);
            }
        }
    }
    // Host access to pCreateInfos[].surface,pCreateInfos[].oldSwapchain must be externally synchronized
}

#ifdef VK_USE_PLATFORM_XLIB_KHR

void ThreadSafety::PreCallRecordCreateXlibSurfaceKHR(
    VkInstance                                  instance,
    const VkXlibSurfaceCreateInfoKHR*           pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) {
    StartReadObjectParentInstance(instance, "vkCreateXlibSurfaceKHR");
}

void ThreadSafety::PostCallRecordCreateXlibSurfaceKHR(
    VkInstance                                  instance,
    const VkXlibSurfaceCreateInfoKHR*           pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface,
    VkResult                                    result) {
    FinishReadObjectParentInstance(instance, "vkCreateXlibSurfaceKHR");
    if (result == VK_SUCCESS) {
        CreateObjectParentInstance(*pSurface);
    }
}
#endif // VK_USE_PLATFORM_XLIB_KHR

#ifdef VK_USE_PLATFORM_XCB_KHR

void ThreadSafety::PreCallRecordCreateXcbSurfaceKHR(
    VkInstance                                  instance,
    const VkXcbSurfaceCreateInfoKHR*            pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) {
    StartReadObjectParentInstance(instance, "vkCreateXcbSurfaceKHR");
}

void ThreadSafety::PostCallRecordCreateXcbSurfaceKHR(
    VkInstance                                  instance,
    const VkXcbSurfaceCreateInfoKHR*            pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface,
    VkResult                                    result) {
    FinishReadObjectParentInstance(instance, "vkCreateXcbSurfaceKHR");
    if (result == VK_SUCCESS) {
        CreateObjectParentInstance(*pSurface);
    }
}
#endif // VK_USE_PLATFORM_XCB_KHR

#ifdef VK_USE_PLATFORM_WAYLAND_KHR

void ThreadSafety::PreCallRecordCreateWaylandSurfaceKHR(
    VkInstance                                  instance,
    const VkWaylandSurfaceCreateInfoKHR*        pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) {
    StartReadObjectParentInstance(instance, "vkCreateWaylandSurfaceKHR");
}

void ThreadSafety::PostCallRecordCreateWaylandSurfaceKHR(
    VkInstance                                  instance,
    const VkWaylandSurfaceCreateInfoKHR*        pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface,
    VkResult                                    result) {
    FinishReadObjectParentInstance(instance, "vkCreateWaylandSurfaceKHR");
    if (result == VK_SUCCESS) {
        CreateObjectParentInstance(*pSurface);
    }
}
#endif // VK_USE_PLATFORM_WAYLAND_KHR

#ifdef VK_USE_PLATFORM_ANDROID_KHR

void ThreadSafety::PreCallRecordCreateAndroidSurfaceKHR(
    VkInstance                                  instance,
    const VkAndroidSurfaceCreateInfoKHR*        pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) {
    StartReadObjectParentInstance(instance, "vkCreateAndroidSurfaceKHR");
}

void ThreadSafety::PostCallRecordCreateAndroidSurfaceKHR(
    VkInstance                                  instance,
    const VkAndroidSurfaceCreateInfoKHR*        pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface,
    VkResult                                    result) {
    FinishReadObjectParentInstance(instance, "vkCreateAndroidSurfaceKHR");
    if (result == VK_SUCCESS) {
        CreateObjectParentInstance(*pSurface);
    }
}
#endif // VK_USE_PLATFORM_ANDROID_KHR

#ifdef VK_USE_PLATFORM_WIN32_KHR

void ThreadSafety::PreCallRecordCreateWin32SurfaceKHR(
    VkInstance                                  instance,
    const VkWin32SurfaceCreateInfoKHR*          pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) {
    StartReadObjectParentInstance(instance, "vkCreateWin32SurfaceKHR");
}

void ThreadSafety::PostCallRecordCreateWin32SurfaceKHR(
    VkInstance                                  instance,
    const VkWin32SurfaceCreateInfoKHR*          pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface,
    VkResult                                    result) {
    FinishReadObjectParentInstance(instance, "vkCreateWin32SurfaceKHR");
    if (result == VK_SUCCESS) {
        CreateObjectParentInstance(*pSurface);
    }
}
#endif // VK_USE_PLATFORM_WIN32_KHR

void ThreadSafety::PreCallRecordCreateVideoSessionKHR(
    VkDevice                                    device,
    const VkVideoSessionCreateInfoKHR*          pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkVideoSessionKHR*                          pVideoSession) {
    StartReadObjectParentInstance(device, "vkCreateVideoSessionKHR");
}

void ThreadSafety::PostCallRecordCreateVideoSessionKHR(
    VkDevice                                    device,
    const VkVideoSessionCreateInfoKHR*          pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkVideoSessionKHR*                          pVideoSession,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkCreateVideoSessionKHR");
    if (result == VK_SUCCESS) {
        CreateObject(*pVideoSession);
    }
}

void ThreadSafety::PreCallRecordDestroyVideoSessionKHR(
    VkDevice                                    device,
    VkVideoSessionKHR                           videoSession,
    const VkAllocationCallbacks*                pAllocator) {
    StartReadObjectParentInstance(device, "vkDestroyVideoSessionKHR");
    StartWriteObject(videoSession, "vkDestroyVideoSessionKHR");
    // Host access to videoSession must be externally synchronized
}

void ThreadSafety::PostCallRecordDestroyVideoSessionKHR(
    VkDevice                                    device,
    VkVideoSessionKHR                           videoSession,
    const VkAllocationCallbacks*                pAllocator) {
    FinishReadObjectParentInstance(device, "vkDestroyVideoSessionKHR");
    FinishWriteObject(videoSession, "vkDestroyVideoSessionKHR");
    DestroyObject(videoSession);
    // Host access to videoSession must be externally synchronized
}

void ThreadSafety::PreCallRecordGetVideoSessionMemoryRequirementsKHR(
    VkDevice                                    device,
    VkVideoSessionKHR                           videoSession,
    uint32_t*                                   pMemoryRequirementsCount,
    VkVideoSessionMemoryRequirementsKHR*        pMemoryRequirements) {
    StartReadObjectParentInstance(device, "vkGetVideoSessionMemoryRequirementsKHR");
    StartReadObject(videoSession, "vkGetVideoSessionMemoryRequirementsKHR");
}

void ThreadSafety::PostCallRecordGetVideoSessionMemoryRequirementsKHR(
    VkDevice                                    device,
    VkVideoSessionKHR                           videoSession,
    uint32_t*                                   pMemoryRequirementsCount,
    VkVideoSessionMemoryRequirementsKHR*        pMemoryRequirements,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkGetVideoSessionMemoryRequirementsKHR");
    FinishReadObject(videoSession, "vkGetVideoSessionMemoryRequirementsKHR");
}

void ThreadSafety::PreCallRecordBindVideoSessionMemoryKHR(
    VkDevice                                    device,
    VkVideoSessionKHR                           videoSession,
    uint32_t                                    bindSessionMemoryInfoCount,
    const VkBindVideoSessionMemoryInfoKHR*      pBindSessionMemoryInfos) {
    StartReadObjectParentInstance(device, "vkBindVideoSessionMemoryKHR");
    StartWriteObject(videoSession, "vkBindVideoSessionMemoryKHR");
    // Host access to videoSession must be externally synchronized
}

void ThreadSafety::PostCallRecordBindVideoSessionMemoryKHR(
    VkDevice                                    device,
    VkVideoSessionKHR                           videoSession,
    uint32_t                                    bindSessionMemoryInfoCount,
    const VkBindVideoSessionMemoryInfoKHR*      pBindSessionMemoryInfos,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkBindVideoSessionMemoryKHR");
    FinishWriteObject(videoSession, "vkBindVideoSessionMemoryKHR");
    // Host access to videoSession must be externally synchronized
}

void ThreadSafety::PreCallRecordCreateVideoSessionParametersKHR(
    VkDevice                                    device,
    const VkVideoSessionParametersCreateInfoKHR* pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkVideoSessionParametersKHR*                pVideoSessionParameters) {
    StartReadObjectParentInstance(device, "vkCreateVideoSessionParametersKHR");
}

void ThreadSafety::PostCallRecordCreateVideoSessionParametersKHR(
    VkDevice                                    device,
    const VkVideoSessionParametersCreateInfoKHR* pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkVideoSessionParametersKHR*                pVideoSessionParameters,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkCreateVideoSessionParametersKHR");
    if (result == VK_SUCCESS) {
        CreateObject(*pVideoSessionParameters);
    }
}

void ThreadSafety::PreCallRecordUpdateVideoSessionParametersKHR(
    VkDevice                                    device,
    VkVideoSessionParametersKHR                 videoSessionParameters,
    const VkVideoSessionParametersUpdateInfoKHR* pUpdateInfo) {
    StartReadObjectParentInstance(device, "vkUpdateVideoSessionParametersKHR");
    StartReadObject(videoSessionParameters, "vkUpdateVideoSessionParametersKHR");
}

void ThreadSafety::PostCallRecordUpdateVideoSessionParametersKHR(
    VkDevice                                    device,
    VkVideoSessionParametersKHR                 videoSessionParameters,
    const VkVideoSessionParametersUpdateInfoKHR* pUpdateInfo,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkUpdateVideoSessionParametersKHR");
    FinishReadObject(videoSessionParameters, "vkUpdateVideoSessionParametersKHR");
}

void ThreadSafety::PreCallRecordDestroyVideoSessionParametersKHR(
    VkDevice                                    device,
    VkVideoSessionParametersKHR                 videoSessionParameters,
    const VkAllocationCallbacks*                pAllocator) {
    StartReadObjectParentInstance(device, "vkDestroyVideoSessionParametersKHR");
    StartWriteObject(videoSessionParameters, "vkDestroyVideoSessionParametersKHR");
    // Host access to videoSessionParameters must be externally synchronized
}

void ThreadSafety::PostCallRecordDestroyVideoSessionParametersKHR(
    VkDevice                                    device,
    VkVideoSessionParametersKHR                 videoSessionParameters,
    const VkAllocationCallbacks*                pAllocator) {
    FinishReadObjectParentInstance(device, "vkDestroyVideoSessionParametersKHR");
    FinishWriteObject(videoSessionParameters, "vkDestroyVideoSessionParametersKHR");
    DestroyObject(videoSessionParameters);
    // Host access to videoSessionParameters must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdBeginVideoCodingKHR(
    VkCommandBuffer                             commandBuffer,
    const VkVideoBeginCodingInfoKHR*            pBeginInfo) {
    StartWriteObject(commandBuffer, "vkCmdBeginVideoCodingKHR");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdBeginVideoCodingKHR(
    VkCommandBuffer                             commandBuffer,
    const VkVideoBeginCodingInfoKHR*            pBeginInfo) {
    FinishWriteObject(commandBuffer, "vkCmdBeginVideoCodingKHR");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdEndVideoCodingKHR(
    VkCommandBuffer                             commandBuffer,
    const VkVideoEndCodingInfoKHR*              pEndCodingInfo) {
    StartWriteObject(commandBuffer, "vkCmdEndVideoCodingKHR");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdEndVideoCodingKHR(
    VkCommandBuffer                             commandBuffer,
    const VkVideoEndCodingInfoKHR*              pEndCodingInfo) {
    FinishWriteObject(commandBuffer, "vkCmdEndVideoCodingKHR");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdControlVideoCodingKHR(
    VkCommandBuffer                             commandBuffer,
    const VkVideoCodingControlInfoKHR*          pCodingControlInfo) {
    StartWriteObject(commandBuffer, "vkCmdControlVideoCodingKHR");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdControlVideoCodingKHR(
    VkCommandBuffer                             commandBuffer,
    const VkVideoCodingControlInfoKHR*          pCodingControlInfo) {
    FinishWriteObject(commandBuffer, "vkCmdControlVideoCodingKHR");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdDecodeVideoKHR(
    VkCommandBuffer                             commandBuffer,
    const VkVideoDecodeInfoKHR*                 pDecodeInfo) {
    StartWriteObject(commandBuffer, "vkCmdDecodeVideoKHR");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdDecodeVideoKHR(
    VkCommandBuffer                             commandBuffer,
    const VkVideoDecodeInfoKHR*                 pDecodeInfo) {
    FinishWriteObject(commandBuffer, "vkCmdDecodeVideoKHR");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdBeginRenderingKHR(
    VkCommandBuffer                             commandBuffer,
    const VkRenderingInfo*                      pRenderingInfo) {
    StartWriteObject(commandBuffer, "vkCmdBeginRenderingKHR");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdBeginRenderingKHR(
    VkCommandBuffer                             commandBuffer,
    const VkRenderingInfo*                      pRenderingInfo) {
    FinishWriteObject(commandBuffer, "vkCmdBeginRenderingKHR");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdEndRenderingKHR(
    VkCommandBuffer                             commandBuffer) {
    StartWriteObject(commandBuffer, "vkCmdEndRenderingKHR");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdEndRenderingKHR(
    VkCommandBuffer                             commandBuffer) {
    FinishWriteObject(commandBuffer, "vkCmdEndRenderingKHR");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordGetDeviceGroupPeerMemoryFeaturesKHR(
    VkDevice                                    device,
    uint32_t                                    heapIndex,
    uint32_t                                    localDeviceIndex,
    uint32_t                                    remoteDeviceIndex,
    VkPeerMemoryFeatureFlags*                   pPeerMemoryFeatures) {
    StartReadObjectParentInstance(device, "vkGetDeviceGroupPeerMemoryFeaturesKHR");
}

void ThreadSafety::PostCallRecordGetDeviceGroupPeerMemoryFeaturesKHR(
    VkDevice                                    device,
    uint32_t                                    heapIndex,
    uint32_t                                    localDeviceIndex,
    uint32_t                                    remoteDeviceIndex,
    VkPeerMemoryFeatureFlags*                   pPeerMemoryFeatures) {
    FinishReadObjectParentInstance(device, "vkGetDeviceGroupPeerMemoryFeaturesKHR");
}

void ThreadSafety::PreCallRecordCmdSetDeviceMaskKHR(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    deviceMask) {
    StartWriteObject(commandBuffer, "vkCmdSetDeviceMaskKHR");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetDeviceMaskKHR(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    deviceMask) {
    FinishWriteObject(commandBuffer, "vkCmdSetDeviceMaskKHR");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdDispatchBaseKHR(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    baseGroupX,
    uint32_t                                    baseGroupY,
    uint32_t                                    baseGroupZ,
    uint32_t                                    groupCountX,
    uint32_t                                    groupCountY,
    uint32_t                                    groupCountZ) {
    StartWriteObject(commandBuffer, "vkCmdDispatchBaseKHR");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdDispatchBaseKHR(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    baseGroupX,
    uint32_t                                    baseGroupY,
    uint32_t                                    baseGroupZ,
    uint32_t                                    groupCountX,
    uint32_t                                    groupCountY,
    uint32_t                                    groupCountZ) {
    FinishWriteObject(commandBuffer, "vkCmdDispatchBaseKHR");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordTrimCommandPoolKHR(
    VkDevice                                    device,
    VkCommandPool                               commandPool,
    VkCommandPoolTrimFlags                      flags) {
    StartReadObjectParentInstance(device, "vkTrimCommandPoolKHR");
    StartWriteObject(commandPool, "vkTrimCommandPoolKHR");
    // Host access to commandPool must be externally synchronized
}

void ThreadSafety::PostCallRecordTrimCommandPoolKHR(
    VkDevice                                    device,
    VkCommandPool                               commandPool,
    VkCommandPoolTrimFlags                      flags) {
    FinishReadObjectParentInstance(device, "vkTrimCommandPoolKHR");
    FinishWriteObject(commandPool, "vkTrimCommandPoolKHR");
    // Host access to commandPool must be externally synchronized
}

void ThreadSafety::PreCallRecordEnumeratePhysicalDeviceGroupsKHR(
    VkInstance                                  instance,
    uint32_t*                                   pPhysicalDeviceGroupCount,
    VkPhysicalDeviceGroupProperties*            pPhysicalDeviceGroupProperties) {
    StartReadObjectParentInstance(instance, "vkEnumeratePhysicalDeviceGroupsKHR");
}

void ThreadSafety::PostCallRecordEnumeratePhysicalDeviceGroupsKHR(
    VkInstance                                  instance,
    uint32_t*                                   pPhysicalDeviceGroupCount,
    VkPhysicalDeviceGroupProperties*            pPhysicalDeviceGroupProperties,
    VkResult                                    result) {
    FinishReadObjectParentInstance(instance, "vkEnumeratePhysicalDeviceGroupsKHR");
}

#ifdef VK_USE_PLATFORM_WIN32_KHR

void ThreadSafety::PreCallRecordGetMemoryWin32HandleKHR(
    VkDevice                                    device,
    const VkMemoryGetWin32HandleInfoKHR*        pGetWin32HandleInfo,
    HANDLE*                                     pHandle) {
    StartReadObjectParentInstance(device, "vkGetMemoryWin32HandleKHR");
}

void ThreadSafety::PostCallRecordGetMemoryWin32HandleKHR(
    VkDevice                                    device,
    const VkMemoryGetWin32HandleInfoKHR*        pGetWin32HandleInfo,
    HANDLE*                                     pHandle,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkGetMemoryWin32HandleKHR");
}

void ThreadSafety::PreCallRecordGetMemoryWin32HandlePropertiesKHR(
    VkDevice                                    device,
    VkExternalMemoryHandleTypeFlagBits          handleType,
    HANDLE                                      handle,
    VkMemoryWin32HandlePropertiesKHR*           pMemoryWin32HandleProperties) {
    StartReadObjectParentInstance(device, "vkGetMemoryWin32HandlePropertiesKHR");
}

void ThreadSafety::PostCallRecordGetMemoryWin32HandlePropertiesKHR(
    VkDevice                                    device,
    VkExternalMemoryHandleTypeFlagBits          handleType,
    HANDLE                                      handle,
    VkMemoryWin32HandlePropertiesKHR*           pMemoryWin32HandleProperties,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkGetMemoryWin32HandlePropertiesKHR");
}
#endif // VK_USE_PLATFORM_WIN32_KHR

void ThreadSafety::PreCallRecordGetMemoryFdKHR(
    VkDevice                                    device,
    const VkMemoryGetFdInfoKHR*                 pGetFdInfo,
    int*                                        pFd) {
    StartReadObjectParentInstance(device, "vkGetMemoryFdKHR");
}

void ThreadSafety::PostCallRecordGetMemoryFdKHR(
    VkDevice                                    device,
    const VkMemoryGetFdInfoKHR*                 pGetFdInfo,
    int*                                        pFd,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkGetMemoryFdKHR");
}

void ThreadSafety::PreCallRecordGetMemoryFdPropertiesKHR(
    VkDevice                                    device,
    VkExternalMemoryHandleTypeFlagBits          handleType,
    int                                         fd,
    VkMemoryFdPropertiesKHR*                    pMemoryFdProperties) {
    StartReadObjectParentInstance(device, "vkGetMemoryFdPropertiesKHR");
}

void ThreadSafety::PostCallRecordGetMemoryFdPropertiesKHR(
    VkDevice                                    device,
    VkExternalMemoryHandleTypeFlagBits          handleType,
    int                                         fd,
    VkMemoryFdPropertiesKHR*                    pMemoryFdProperties,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkGetMemoryFdPropertiesKHR");
}

#ifdef VK_USE_PLATFORM_WIN32_KHR
#endif // VK_USE_PLATFORM_WIN32_KHR

#ifdef VK_USE_PLATFORM_WIN32_KHR

void ThreadSafety::PreCallRecordImportSemaphoreWin32HandleKHR(
    VkDevice                                    device,
    const VkImportSemaphoreWin32HandleInfoKHR*  pImportSemaphoreWin32HandleInfo) {
    StartReadObjectParentInstance(device, "vkImportSemaphoreWin32HandleKHR");
}

void ThreadSafety::PostCallRecordImportSemaphoreWin32HandleKHR(
    VkDevice                                    device,
    const VkImportSemaphoreWin32HandleInfoKHR*  pImportSemaphoreWin32HandleInfo,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkImportSemaphoreWin32HandleKHR");
}

void ThreadSafety::PreCallRecordGetSemaphoreWin32HandleKHR(
    VkDevice                                    device,
    const VkSemaphoreGetWin32HandleInfoKHR*     pGetWin32HandleInfo,
    HANDLE*                                     pHandle) {
    StartReadObjectParentInstance(device, "vkGetSemaphoreWin32HandleKHR");
}

void ThreadSafety::PostCallRecordGetSemaphoreWin32HandleKHR(
    VkDevice                                    device,
    const VkSemaphoreGetWin32HandleInfoKHR*     pGetWin32HandleInfo,
    HANDLE*                                     pHandle,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkGetSemaphoreWin32HandleKHR");
}
#endif // VK_USE_PLATFORM_WIN32_KHR

void ThreadSafety::PreCallRecordImportSemaphoreFdKHR(
    VkDevice                                    device,
    const VkImportSemaphoreFdInfoKHR*           pImportSemaphoreFdInfo) {
    StartReadObjectParentInstance(device, "vkImportSemaphoreFdKHR");
}

void ThreadSafety::PostCallRecordImportSemaphoreFdKHR(
    VkDevice                                    device,
    const VkImportSemaphoreFdInfoKHR*           pImportSemaphoreFdInfo,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkImportSemaphoreFdKHR");
}

void ThreadSafety::PreCallRecordGetSemaphoreFdKHR(
    VkDevice                                    device,
    const VkSemaphoreGetFdInfoKHR*              pGetFdInfo,
    int*                                        pFd) {
    StartReadObjectParentInstance(device, "vkGetSemaphoreFdKHR");
}

void ThreadSafety::PostCallRecordGetSemaphoreFdKHR(
    VkDevice                                    device,
    const VkSemaphoreGetFdInfoKHR*              pGetFdInfo,
    int*                                        pFd,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkGetSemaphoreFdKHR");
}

void ThreadSafety::PreCallRecordCmdPushDescriptorSetKHR(
    VkCommandBuffer                             commandBuffer,
    VkPipelineBindPoint                         pipelineBindPoint,
    VkPipelineLayout                            layout,
    uint32_t                                    set,
    uint32_t                                    descriptorWriteCount,
    const VkWriteDescriptorSet*                 pDescriptorWrites) {
    StartWriteObject(commandBuffer, "vkCmdPushDescriptorSetKHR");
    StartReadObject(layout, "vkCmdPushDescriptorSetKHR");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdPushDescriptorSetKHR(
    VkCommandBuffer                             commandBuffer,
    VkPipelineBindPoint                         pipelineBindPoint,
    VkPipelineLayout                            layout,
    uint32_t                                    set,
    uint32_t                                    descriptorWriteCount,
    const VkWriteDescriptorSet*                 pDescriptorWrites) {
    FinishWriteObject(commandBuffer, "vkCmdPushDescriptorSetKHR");
    FinishReadObject(layout, "vkCmdPushDescriptorSetKHR");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdPushDescriptorSetWithTemplateKHR(
    VkCommandBuffer                             commandBuffer,
    VkDescriptorUpdateTemplate                  descriptorUpdateTemplate,
    VkPipelineLayout                            layout,
    uint32_t                                    set,
    const void*                                 pData) {
    StartWriteObject(commandBuffer, "vkCmdPushDescriptorSetWithTemplateKHR");
    StartReadObject(descriptorUpdateTemplate, "vkCmdPushDescriptorSetWithTemplateKHR");
    StartReadObject(layout, "vkCmdPushDescriptorSetWithTemplateKHR");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdPushDescriptorSetWithTemplateKHR(
    VkCommandBuffer                             commandBuffer,
    VkDescriptorUpdateTemplate                  descriptorUpdateTemplate,
    VkPipelineLayout                            layout,
    uint32_t                                    set,
    const void*                                 pData) {
    FinishWriteObject(commandBuffer, "vkCmdPushDescriptorSetWithTemplateKHR");
    FinishReadObject(descriptorUpdateTemplate, "vkCmdPushDescriptorSetWithTemplateKHR");
    FinishReadObject(layout, "vkCmdPushDescriptorSetWithTemplateKHR");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCreateDescriptorUpdateTemplateKHR(
    VkDevice                                    device,
    const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDescriptorUpdateTemplate*                 pDescriptorUpdateTemplate) {
    StartReadObjectParentInstance(device, "vkCreateDescriptorUpdateTemplateKHR");
}

void ThreadSafety::PostCallRecordCreateDescriptorUpdateTemplateKHR(
    VkDevice                                    device,
    const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDescriptorUpdateTemplate*                 pDescriptorUpdateTemplate,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkCreateDescriptorUpdateTemplateKHR");
    if (result == VK_SUCCESS) {
        CreateObject(*pDescriptorUpdateTemplate);
    }
}

void ThreadSafety::PreCallRecordDestroyDescriptorUpdateTemplateKHR(
    VkDevice                                    device,
    VkDescriptorUpdateTemplate                  descriptorUpdateTemplate,
    const VkAllocationCallbacks*                pAllocator) {
    StartReadObjectParentInstance(device, "vkDestroyDescriptorUpdateTemplateKHR");
    StartWriteObject(descriptorUpdateTemplate, "vkDestroyDescriptorUpdateTemplateKHR");
    // Host access to descriptorUpdateTemplate must be externally synchronized
}

void ThreadSafety::PostCallRecordDestroyDescriptorUpdateTemplateKHR(
    VkDevice                                    device,
    VkDescriptorUpdateTemplate                  descriptorUpdateTemplate,
    const VkAllocationCallbacks*                pAllocator) {
    FinishReadObjectParentInstance(device, "vkDestroyDescriptorUpdateTemplateKHR");
    FinishWriteObject(descriptorUpdateTemplate, "vkDestroyDescriptorUpdateTemplateKHR");
    DestroyObject(descriptorUpdateTemplate);
    // Host access to descriptorUpdateTemplate must be externally synchronized
}

void ThreadSafety::PreCallRecordCreateRenderPass2KHR(
    VkDevice                                    device,
    const VkRenderPassCreateInfo2*              pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkRenderPass*                               pRenderPass) {
    StartReadObjectParentInstance(device, "vkCreateRenderPass2KHR");
}

void ThreadSafety::PostCallRecordCreateRenderPass2KHR(
    VkDevice                                    device,
    const VkRenderPassCreateInfo2*              pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkRenderPass*                               pRenderPass,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkCreateRenderPass2KHR");
    if (result == VK_SUCCESS) {
        CreateObject(*pRenderPass);
    }
}

void ThreadSafety::PreCallRecordCmdBeginRenderPass2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkRenderPassBeginInfo*                pRenderPassBegin,
    const VkSubpassBeginInfo*                   pSubpassBeginInfo) {
    StartWriteObject(commandBuffer, "vkCmdBeginRenderPass2KHR");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdBeginRenderPass2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkRenderPassBeginInfo*                pRenderPassBegin,
    const VkSubpassBeginInfo*                   pSubpassBeginInfo) {
    FinishWriteObject(commandBuffer, "vkCmdBeginRenderPass2KHR");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdNextSubpass2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkSubpassBeginInfo*                   pSubpassBeginInfo,
    const VkSubpassEndInfo*                     pSubpassEndInfo) {
    StartWriteObject(commandBuffer, "vkCmdNextSubpass2KHR");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdNextSubpass2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkSubpassBeginInfo*                   pSubpassBeginInfo,
    const VkSubpassEndInfo*                     pSubpassEndInfo) {
    FinishWriteObject(commandBuffer, "vkCmdNextSubpass2KHR");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdEndRenderPass2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkSubpassEndInfo*                     pSubpassEndInfo) {
    StartWriteObject(commandBuffer, "vkCmdEndRenderPass2KHR");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdEndRenderPass2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkSubpassEndInfo*                     pSubpassEndInfo) {
    FinishWriteObject(commandBuffer, "vkCmdEndRenderPass2KHR");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordGetSwapchainStatusKHR(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain) {
    StartReadObjectParentInstance(device, "vkGetSwapchainStatusKHR");
    StartWriteObjectParentInstance(swapchain, "vkGetSwapchainStatusKHR");
    // Host access to swapchain must be externally synchronized
}

void ThreadSafety::PostCallRecordGetSwapchainStatusKHR(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkGetSwapchainStatusKHR");
    FinishWriteObjectParentInstance(swapchain, "vkGetSwapchainStatusKHR");
    // Host access to swapchain must be externally synchronized
}

#ifdef VK_USE_PLATFORM_WIN32_KHR

void ThreadSafety::PreCallRecordImportFenceWin32HandleKHR(
    VkDevice                                    device,
    const VkImportFenceWin32HandleInfoKHR*      pImportFenceWin32HandleInfo) {
    StartReadObjectParentInstance(device, "vkImportFenceWin32HandleKHR");
}

void ThreadSafety::PostCallRecordImportFenceWin32HandleKHR(
    VkDevice                                    device,
    const VkImportFenceWin32HandleInfoKHR*      pImportFenceWin32HandleInfo,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkImportFenceWin32HandleKHR");
}

void ThreadSafety::PreCallRecordGetFenceWin32HandleKHR(
    VkDevice                                    device,
    const VkFenceGetWin32HandleInfoKHR*         pGetWin32HandleInfo,
    HANDLE*                                     pHandle) {
    StartReadObjectParentInstance(device, "vkGetFenceWin32HandleKHR");
}

void ThreadSafety::PostCallRecordGetFenceWin32HandleKHR(
    VkDevice                                    device,
    const VkFenceGetWin32HandleInfoKHR*         pGetWin32HandleInfo,
    HANDLE*                                     pHandle,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkGetFenceWin32HandleKHR");
}
#endif // VK_USE_PLATFORM_WIN32_KHR

void ThreadSafety::PreCallRecordImportFenceFdKHR(
    VkDevice                                    device,
    const VkImportFenceFdInfoKHR*               pImportFenceFdInfo) {
    StartReadObjectParentInstance(device, "vkImportFenceFdKHR");
}

void ThreadSafety::PostCallRecordImportFenceFdKHR(
    VkDevice                                    device,
    const VkImportFenceFdInfoKHR*               pImportFenceFdInfo,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkImportFenceFdKHR");
}

void ThreadSafety::PreCallRecordGetFenceFdKHR(
    VkDevice                                    device,
    const VkFenceGetFdInfoKHR*                  pGetFdInfo,
    int*                                        pFd) {
    StartReadObjectParentInstance(device, "vkGetFenceFdKHR");
}

void ThreadSafety::PostCallRecordGetFenceFdKHR(
    VkDevice                                    device,
    const VkFenceGetFdInfoKHR*                  pGetFdInfo,
    int*                                        pFd,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkGetFenceFdKHR");
}

void ThreadSafety::PreCallRecordAcquireProfilingLockKHR(
    VkDevice                                    device,
    const VkAcquireProfilingLockInfoKHR*        pInfo) {
    StartReadObjectParentInstance(device, "vkAcquireProfilingLockKHR");
}

void ThreadSafety::PostCallRecordAcquireProfilingLockKHR(
    VkDevice                                    device,
    const VkAcquireProfilingLockInfoKHR*        pInfo,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkAcquireProfilingLockKHR");
}

void ThreadSafety::PreCallRecordReleaseProfilingLockKHR(
    VkDevice                                    device) {
    StartReadObjectParentInstance(device, "vkReleaseProfilingLockKHR");
}

void ThreadSafety::PostCallRecordReleaseProfilingLockKHR(
    VkDevice                                    device) {
    FinishReadObjectParentInstance(device, "vkReleaseProfilingLockKHR");
}

void ThreadSafety::PreCallRecordGetImageMemoryRequirements2KHR(
    VkDevice                                    device,
    const VkImageMemoryRequirementsInfo2*       pInfo,
    VkMemoryRequirements2*                      pMemoryRequirements) {
    StartReadObjectParentInstance(device, "vkGetImageMemoryRequirements2KHR");
}

void ThreadSafety::PostCallRecordGetImageMemoryRequirements2KHR(
    VkDevice                                    device,
    const VkImageMemoryRequirementsInfo2*       pInfo,
    VkMemoryRequirements2*                      pMemoryRequirements) {
    FinishReadObjectParentInstance(device, "vkGetImageMemoryRequirements2KHR");
}

void ThreadSafety::PreCallRecordGetBufferMemoryRequirements2KHR(
    VkDevice                                    device,
    const VkBufferMemoryRequirementsInfo2*      pInfo,
    VkMemoryRequirements2*                      pMemoryRequirements) {
    StartReadObjectParentInstance(device, "vkGetBufferMemoryRequirements2KHR");
}

void ThreadSafety::PostCallRecordGetBufferMemoryRequirements2KHR(
    VkDevice                                    device,
    const VkBufferMemoryRequirementsInfo2*      pInfo,
    VkMemoryRequirements2*                      pMemoryRequirements) {
    FinishReadObjectParentInstance(device, "vkGetBufferMemoryRequirements2KHR");
}

void ThreadSafety::PreCallRecordGetImageSparseMemoryRequirements2KHR(
    VkDevice                                    device,
    const VkImageSparseMemoryRequirementsInfo2* pInfo,
    uint32_t*                                   pSparseMemoryRequirementCount,
    VkSparseImageMemoryRequirements2*           pSparseMemoryRequirements) {
    StartReadObjectParentInstance(device, "vkGetImageSparseMemoryRequirements2KHR");
}

void ThreadSafety::PostCallRecordGetImageSparseMemoryRequirements2KHR(
    VkDevice                                    device,
    const VkImageSparseMemoryRequirementsInfo2* pInfo,
    uint32_t*                                   pSparseMemoryRequirementCount,
    VkSparseImageMemoryRequirements2*           pSparseMemoryRequirements) {
    FinishReadObjectParentInstance(device, "vkGetImageSparseMemoryRequirements2KHR");
}

void ThreadSafety::PreCallRecordCreateSamplerYcbcrConversionKHR(
    VkDevice                                    device,
    const VkSamplerYcbcrConversionCreateInfo*   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSamplerYcbcrConversion*                   pYcbcrConversion) {
    StartReadObjectParentInstance(device, "vkCreateSamplerYcbcrConversionKHR");
}

void ThreadSafety::PostCallRecordCreateSamplerYcbcrConversionKHR(
    VkDevice                                    device,
    const VkSamplerYcbcrConversionCreateInfo*   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSamplerYcbcrConversion*                   pYcbcrConversion,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkCreateSamplerYcbcrConversionKHR");
    if (result == VK_SUCCESS) {
        CreateObject(*pYcbcrConversion);
    }
}

void ThreadSafety::PreCallRecordDestroySamplerYcbcrConversionKHR(
    VkDevice                                    device,
    VkSamplerYcbcrConversion                    ycbcrConversion,
    const VkAllocationCallbacks*                pAllocator) {
    StartReadObjectParentInstance(device, "vkDestroySamplerYcbcrConversionKHR");
    StartWriteObject(ycbcrConversion, "vkDestroySamplerYcbcrConversionKHR");
    // Host access to ycbcrConversion must be externally synchronized
}

void ThreadSafety::PostCallRecordDestroySamplerYcbcrConversionKHR(
    VkDevice                                    device,
    VkSamplerYcbcrConversion                    ycbcrConversion,
    const VkAllocationCallbacks*                pAllocator) {
    FinishReadObjectParentInstance(device, "vkDestroySamplerYcbcrConversionKHR");
    FinishWriteObject(ycbcrConversion, "vkDestroySamplerYcbcrConversionKHR");
    DestroyObject(ycbcrConversion);
    // Host access to ycbcrConversion must be externally synchronized
}

void ThreadSafety::PreCallRecordBindBufferMemory2KHR(
    VkDevice                                    device,
    uint32_t                                    bindInfoCount,
    const VkBindBufferMemoryInfo*               pBindInfos) {
    StartReadObjectParentInstance(device, "vkBindBufferMemory2KHR");
}

void ThreadSafety::PostCallRecordBindBufferMemory2KHR(
    VkDevice                                    device,
    uint32_t                                    bindInfoCount,
    const VkBindBufferMemoryInfo*               pBindInfos,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkBindBufferMemory2KHR");
}

void ThreadSafety::PreCallRecordBindImageMemory2KHR(
    VkDevice                                    device,
    uint32_t                                    bindInfoCount,
    const VkBindImageMemoryInfo*                pBindInfos) {
    StartReadObjectParentInstance(device, "vkBindImageMemory2KHR");
}

void ThreadSafety::PostCallRecordBindImageMemory2KHR(
    VkDevice                                    device,
    uint32_t                                    bindInfoCount,
    const VkBindImageMemoryInfo*                pBindInfos,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkBindImageMemory2KHR");
}

#ifdef VK_ENABLE_BETA_EXTENSIONS
#endif // VK_ENABLE_BETA_EXTENSIONS

void ThreadSafety::PreCallRecordGetDescriptorSetLayoutSupportKHR(
    VkDevice                                    device,
    const VkDescriptorSetLayoutCreateInfo*      pCreateInfo,
    VkDescriptorSetLayoutSupport*               pSupport) {
    StartReadObjectParentInstance(device, "vkGetDescriptorSetLayoutSupportKHR");
}

void ThreadSafety::PostCallRecordGetDescriptorSetLayoutSupportKHR(
    VkDevice                                    device,
    const VkDescriptorSetLayoutCreateInfo*      pCreateInfo,
    VkDescriptorSetLayoutSupport*               pSupport) {
    FinishReadObjectParentInstance(device, "vkGetDescriptorSetLayoutSupportKHR");
}

void ThreadSafety::PreCallRecordCmdDrawIndirectCountKHR(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkBuffer                                    countBuffer,
    VkDeviceSize                                countBufferOffset,
    uint32_t                                    maxDrawCount,
    uint32_t                                    stride) {
    StartWriteObject(commandBuffer, "vkCmdDrawIndirectCountKHR");
    StartReadObject(buffer, "vkCmdDrawIndirectCountKHR");
    StartReadObject(countBuffer, "vkCmdDrawIndirectCountKHR");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdDrawIndirectCountKHR(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkBuffer                                    countBuffer,
    VkDeviceSize                                countBufferOffset,
    uint32_t                                    maxDrawCount,
    uint32_t                                    stride) {
    FinishWriteObject(commandBuffer, "vkCmdDrawIndirectCountKHR");
    FinishReadObject(buffer, "vkCmdDrawIndirectCountKHR");
    FinishReadObject(countBuffer, "vkCmdDrawIndirectCountKHR");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdDrawIndexedIndirectCountKHR(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkBuffer                                    countBuffer,
    VkDeviceSize                                countBufferOffset,
    uint32_t                                    maxDrawCount,
    uint32_t                                    stride) {
    StartWriteObject(commandBuffer, "vkCmdDrawIndexedIndirectCountKHR");
    StartReadObject(buffer, "vkCmdDrawIndexedIndirectCountKHR");
    StartReadObject(countBuffer, "vkCmdDrawIndexedIndirectCountKHR");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdDrawIndexedIndirectCountKHR(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkBuffer                                    countBuffer,
    VkDeviceSize                                countBufferOffset,
    uint32_t                                    maxDrawCount,
    uint32_t                                    stride) {
    FinishWriteObject(commandBuffer, "vkCmdDrawIndexedIndirectCountKHR");
    FinishReadObject(buffer, "vkCmdDrawIndexedIndirectCountKHR");
    FinishReadObject(countBuffer, "vkCmdDrawIndexedIndirectCountKHR");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordGetSemaphoreCounterValueKHR(
    VkDevice                                    device,
    VkSemaphore                                 semaphore,
    uint64_t*                                   pValue) {
    StartReadObjectParentInstance(device, "vkGetSemaphoreCounterValueKHR");
    StartReadObject(semaphore, "vkGetSemaphoreCounterValueKHR");
}

void ThreadSafety::PostCallRecordGetSemaphoreCounterValueKHR(
    VkDevice                                    device,
    VkSemaphore                                 semaphore,
    uint64_t*                                   pValue,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkGetSemaphoreCounterValueKHR");
    FinishReadObject(semaphore, "vkGetSemaphoreCounterValueKHR");
}

void ThreadSafety::PreCallRecordWaitSemaphoresKHR(
    VkDevice                                    device,
    const VkSemaphoreWaitInfo*                  pWaitInfo,
    uint64_t                                    timeout) {
    StartReadObjectParentInstance(device, "vkWaitSemaphoresKHR");
}

void ThreadSafety::PostCallRecordWaitSemaphoresKHR(
    VkDevice                                    device,
    const VkSemaphoreWaitInfo*                  pWaitInfo,
    uint64_t                                    timeout,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkWaitSemaphoresKHR");
}

void ThreadSafety::PreCallRecordSignalSemaphoreKHR(
    VkDevice                                    device,
    const VkSemaphoreSignalInfo*                pSignalInfo) {
    StartReadObjectParentInstance(device, "vkSignalSemaphoreKHR");
}

void ThreadSafety::PostCallRecordSignalSemaphoreKHR(
    VkDevice                                    device,
    const VkSemaphoreSignalInfo*                pSignalInfo,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkSignalSemaphoreKHR");
}

void ThreadSafety::PreCallRecordCmdSetFragmentShadingRateKHR(
    VkCommandBuffer                             commandBuffer,
    const VkExtent2D*                           pFragmentSize,
    const VkFragmentShadingRateCombinerOpKHR    combinerOps[2]) {
    StartWriteObject(commandBuffer, "vkCmdSetFragmentShadingRateKHR");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetFragmentShadingRateKHR(
    VkCommandBuffer                             commandBuffer,
    const VkExtent2D*                           pFragmentSize,
    const VkFragmentShadingRateCombinerOpKHR    combinerOps[2]) {
    FinishWriteObject(commandBuffer, "vkCmdSetFragmentShadingRateKHR");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordWaitForPresentKHR(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    uint64_t                                    presentId,
    uint64_t                                    timeout) {
    StartReadObjectParentInstance(device, "vkWaitForPresentKHR");
    StartWriteObjectParentInstance(swapchain, "vkWaitForPresentKHR");
    // Host access to swapchain must be externally synchronized
}

void ThreadSafety::PostCallRecordWaitForPresentKHR(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    uint64_t                                    presentId,
    uint64_t                                    timeout,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkWaitForPresentKHR");
    FinishWriteObjectParentInstance(swapchain, "vkWaitForPresentKHR");
    // Host access to swapchain must be externally synchronized
}

void ThreadSafety::PreCallRecordGetBufferDeviceAddressKHR(
    VkDevice                                    device,
    const VkBufferDeviceAddressInfo*            pInfo) {
    StartReadObjectParentInstance(device, "vkGetBufferDeviceAddressKHR");
}

void ThreadSafety::PostCallRecordGetBufferDeviceAddressKHR(
    VkDevice                                    device,
    const VkBufferDeviceAddressInfo*            pInfo,
    VkDeviceAddress                             result) {
    FinishReadObjectParentInstance(device, "vkGetBufferDeviceAddressKHR");
}

void ThreadSafety::PreCallRecordGetBufferOpaqueCaptureAddressKHR(
    VkDevice                                    device,
    const VkBufferDeviceAddressInfo*            pInfo) {
    StartReadObjectParentInstance(device, "vkGetBufferOpaqueCaptureAddressKHR");
}

void ThreadSafety::PostCallRecordGetBufferOpaqueCaptureAddressKHR(
    VkDevice                                    device,
    const VkBufferDeviceAddressInfo*            pInfo) {
    FinishReadObjectParentInstance(device, "vkGetBufferOpaqueCaptureAddressKHR");
}

void ThreadSafety::PreCallRecordGetDeviceMemoryOpaqueCaptureAddressKHR(
    VkDevice                                    device,
    const VkDeviceMemoryOpaqueCaptureAddressInfo* pInfo) {
    StartReadObjectParentInstance(device, "vkGetDeviceMemoryOpaqueCaptureAddressKHR");
}

void ThreadSafety::PostCallRecordGetDeviceMemoryOpaqueCaptureAddressKHR(
    VkDevice                                    device,
    const VkDeviceMemoryOpaqueCaptureAddressInfo* pInfo) {
    FinishReadObjectParentInstance(device, "vkGetDeviceMemoryOpaqueCaptureAddressKHR");
}

void ThreadSafety::PreCallRecordCreateDeferredOperationKHR(
    VkDevice                                    device,
    const VkAllocationCallbacks*                pAllocator,
    VkDeferredOperationKHR*                     pDeferredOperation) {
    StartReadObjectParentInstance(device, "vkCreateDeferredOperationKHR");
}

void ThreadSafety::PostCallRecordCreateDeferredOperationKHR(
    VkDevice                                    device,
    const VkAllocationCallbacks*                pAllocator,
    VkDeferredOperationKHR*                     pDeferredOperation,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkCreateDeferredOperationKHR");
    if (result == VK_SUCCESS) {
        CreateObject(*pDeferredOperation);
    }
}

void ThreadSafety::PreCallRecordDestroyDeferredOperationKHR(
    VkDevice                                    device,
    VkDeferredOperationKHR                      operation,
    const VkAllocationCallbacks*                pAllocator) {
    StartReadObjectParentInstance(device, "vkDestroyDeferredOperationKHR");
    StartWriteObject(operation, "vkDestroyDeferredOperationKHR");
    // Host access to operation must be externally synchronized
}

void ThreadSafety::PostCallRecordDestroyDeferredOperationKHR(
    VkDevice                                    device,
    VkDeferredOperationKHR                      operation,
    const VkAllocationCallbacks*                pAllocator) {
    FinishReadObjectParentInstance(device, "vkDestroyDeferredOperationKHR");
    FinishWriteObject(operation, "vkDestroyDeferredOperationKHR");
    DestroyObject(operation);
    // Host access to operation must be externally synchronized
}

void ThreadSafety::PreCallRecordGetDeferredOperationMaxConcurrencyKHR(
    VkDevice                                    device,
    VkDeferredOperationKHR                      operation) {
    StartReadObjectParentInstance(device, "vkGetDeferredOperationMaxConcurrencyKHR");
    StartReadObject(operation, "vkGetDeferredOperationMaxConcurrencyKHR");
}

void ThreadSafety::PostCallRecordGetDeferredOperationMaxConcurrencyKHR(
    VkDevice                                    device,
    VkDeferredOperationKHR                      operation) {
    FinishReadObjectParentInstance(device, "vkGetDeferredOperationMaxConcurrencyKHR");
    FinishReadObject(operation, "vkGetDeferredOperationMaxConcurrencyKHR");
}

void ThreadSafety::PreCallRecordGetDeferredOperationResultKHR(
    VkDevice                                    device,
    VkDeferredOperationKHR                      operation) {
    StartReadObjectParentInstance(device, "vkGetDeferredOperationResultKHR");
    StartReadObject(operation, "vkGetDeferredOperationResultKHR");
}

void ThreadSafety::PostCallRecordGetDeferredOperationResultKHR(
    VkDevice                                    device,
    VkDeferredOperationKHR                      operation,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkGetDeferredOperationResultKHR");
    FinishReadObject(operation, "vkGetDeferredOperationResultKHR");
}

void ThreadSafety::PreCallRecordDeferredOperationJoinKHR(
    VkDevice                                    device,
    VkDeferredOperationKHR                      operation) {
    StartReadObjectParentInstance(device, "vkDeferredOperationJoinKHR");
    StartReadObject(operation, "vkDeferredOperationJoinKHR");
}

void ThreadSafety::PostCallRecordDeferredOperationJoinKHR(
    VkDevice                                    device,
    VkDeferredOperationKHR                      operation,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkDeferredOperationJoinKHR");
    FinishReadObject(operation, "vkDeferredOperationJoinKHR");
}

void ThreadSafety::PreCallRecordGetPipelineExecutablePropertiesKHR(
    VkDevice                                    device,
    const VkPipelineInfoKHR*                    pPipelineInfo,
    uint32_t*                                   pExecutableCount,
    VkPipelineExecutablePropertiesKHR*          pProperties) {
    StartReadObjectParentInstance(device, "vkGetPipelineExecutablePropertiesKHR");
}

void ThreadSafety::PostCallRecordGetPipelineExecutablePropertiesKHR(
    VkDevice                                    device,
    const VkPipelineInfoKHR*                    pPipelineInfo,
    uint32_t*                                   pExecutableCount,
    VkPipelineExecutablePropertiesKHR*          pProperties,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkGetPipelineExecutablePropertiesKHR");
}

void ThreadSafety::PreCallRecordGetPipelineExecutableStatisticsKHR(
    VkDevice                                    device,
    const VkPipelineExecutableInfoKHR*          pExecutableInfo,
    uint32_t*                                   pStatisticCount,
    VkPipelineExecutableStatisticKHR*           pStatistics) {
    StartReadObjectParentInstance(device, "vkGetPipelineExecutableStatisticsKHR");
}

void ThreadSafety::PostCallRecordGetPipelineExecutableStatisticsKHR(
    VkDevice                                    device,
    const VkPipelineExecutableInfoKHR*          pExecutableInfo,
    uint32_t*                                   pStatisticCount,
    VkPipelineExecutableStatisticKHR*           pStatistics,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkGetPipelineExecutableStatisticsKHR");
}

void ThreadSafety::PreCallRecordGetPipelineExecutableInternalRepresentationsKHR(
    VkDevice                                    device,
    const VkPipelineExecutableInfoKHR*          pExecutableInfo,
    uint32_t*                                   pInternalRepresentationCount,
    VkPipelineExecutableInternalRepresentationKHR* pInternalRepresentations) {
    StartReadObjectParentInstance(device, "vkGetPipelineExecutableInternalRepresentationsKHR");
}

void ThreadSafety::PostCallRecordGetPipelineExecutableInternalRepresentationsKHR(
    VkDevice                                    device,
    const VkPipelineExecutableInfoKHR*          pExecutableInfo,
    uint32_t*                                   pInternalRepresentationCount,
    VkPipelineExecutableInternalRepresentationKHR* pInternalRepresentations,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkGetPipelineExecutableInternalRepresentationsKHR");
}

#ifdef VK_ENABLE_BETA_EXTENSIONS

void ThreadSafety::PreCallRecordCmdEncodeVideoKHR(
    VkCommandBuffer                             commandBuffer,
    const VkVideoEncodeInfoKHR*                 pEncodeInfo) {
    StartWriteObject(commandBuffer, "vkCmdEncodeVideoKHR");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdEncodeVideoKHR(
    VkCommandBuffer                             commandBuffer,
    const VkVideoEncodeInfoKHR*                 pEncodeInfo) {
    FinishWriteObject(commandBuffer, "vkCmdEncodeVideoKHR");
    // Host access to commandBuffer must be externally synchronized
}
#endif // VK_ENABLE_BETA_EXTENSIONS

void ThreadSafety::PreCallRecordCmdSetEvent2KHR(
    VkCommandBuffer                             commandBuffer,
    VkEvent                                     event,
    const VkDependencyInfo*                     pDependencyInfo) {
    StartWriteObject(commandBuffer, "vkCmdSetEvent2KHR");
    StartReadObject(event, "vkCmdSetEvent2KHR");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetEvent2KHR(
    VkCommandBuffer                             commandBuffer,
    VkEvent                                     event,
    const VkDependencyInfo*                     pDependencyInfo) {
    FinishWriteObject(commandBuffer, "vkCmdSetEvent2KHR");
    FinishReadObject(event, "vkCmdSetEvent2KHR");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdResetEvent2KHR(
    VkCommandBuffer                             commandBuffer,
    VkEvent                                     event,
    VkPipelineStageFlags2                       stageMask) {
    StartWriteObject(commandBuffer, "vkCmdResetEvent2KHR");
    StartReadObject(event, "vkCmdResetEvent2KHR");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdResetEvent2KHR(
    VkCommandBuffer                             commandBuffer,
    VkEvent                                     event,
    VkPipelineStageFlags2                       stageMask) {
    FinishWriteObject(commandBuffer, "vkCmdResetEvent2KHR");
    FinishReadObject(event, "vkCmdResetEvent2KHR");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdWaitEvents2KHR(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    eventCount,
    const VkEvent*                              pEvents,
    const VkDependencyInfo*                     pDependencyInfos) {
    StartWriteObject(commandBuffer, "vkCmdWaitEvents2KHR");
    if (pEvents) {
        for (uint32_t index = 0; index < eventCount; index++) {
            StartReadObject(pEvents[index], "vkCmdWaitEvents2KHR");
        }
    }
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdWaitEvents2KHR(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    eventCount,
    const VkEvent*                              pEvents,
    const VkDependencyInfo*                     pDependencyInfos) {
    FinishWriteObject(commandBuffer, "vkCmdWaitEvents2KHR");
    if (pEvents) {
        for (uint32_t index = 0; index < eventCount; index++) {
            FinishReadObject(pEvents[index], "vkCmdWaitEvents2KHR");
        }
    }
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdPipelineBarrier2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkDependencyInfo*                     pDependencyInfo) {
    StartWriteObject(commandBuffer, "vkCmdPipelineBarrier2KHR");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdPipelineBarrier2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkDependencyInfo*                     pDependencyInfo) {
    FinishWriteObject(commandBuffer, "vkCmdPipelineBarrier2KHR");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdWriteTimestamp2KHR(
    VkCommandBuffer                             commandBuffer,
    VkPipelineStageFlags2                       stage,
    VkQueryPool                                 queryPool,
    uint32_t                                    query) {
    StartWriteObject(commandBuffer, "vkCmdWriteTimestamp2KHR");
    StartReadObject(queryPool, "vkCmdWriteTimestamp2KHR");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdWriteTimestamp2KHR(
    VkCommandBuffer                             commandBuffer,
    VkPipelineStageFlags2                       stage,
    VkQueryPool                                 queryPool,
    uint32_t                                    query) {
    FinishWriteObject(commandBuffer, "vkCmdWriteTimestamp2KHR");
    FinishReadObject(queryPool, "vkCmdWriteTimestamp2KHR");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordQueueSubmit2KHR(
    VkQueue                                     queue,
    uint32_t                                    submitCount,
    const VkSubmitInfo2*                        pSubmits,
    VkFence                                     fence) {
    StartWriteObject(queue, "vkQueueSubmit2KHR");
    StartWriteObject(fence, "vkQueueSubmit2KHR");
    // Host access to queue must be externally synchronized
    // Host access to fence must be externally synchronized
}

void ThreadSafety::PostCallRecordQueueSubmit2KHR(
    VkQueue                                     queue,
    uint32_t                                    submitCount,
    const VkSubmitInfo2*                        pSubmits,
    VkFence                                     fence,
    VkResult                                    result) {
    FinishWriteObject(queue, "vkQueueSubmit2KHR");
    FinishWriteObject(fence, "vkQueueSubmit2KHR");
    // Host access to queue must be externally synchronized
    // Host access to fence must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdWriteBufferMarker2AMD(
    VkCommandBuffer                             commandBuffer,
    VkPipelineStageFlags2                       stage,
    VkBuffer                                    dstBuffer,
    VkDeviceSize                                dstOffset,
    uint32_t                                    marker) {
    StartWriteObject(commandBuffer, "vkCmdWriteBufferMarker2AMD");
    StartReadObject(dstBuffer, "vkCmdWriteBufferMarker2AMD");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdWriteBufferMarker2AMD(
    VkCommandBuffer                             commandBuffer,
    VkPipelineStageFlags2                       stage,
    VkBuffer                                    dstBuffer,
    VkDeviceSize                                dstOffset,
    uint32_t                                    marker) {
    FinishWriteObject(commandBuffer, "vkCmdWriteBufferMarker2AMD");
    FinishReadObject(dstBuffer, "vkCmdWriteBufferMarker2AMD");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordGetQueueCheckpointData2NV(
    VkQueue                                     queue,
    uint32_t*                                   pCheckpointDataCount,
    VkCheckpointData2NV*                        pCheckpointData) {
    StartReadObject(queue, "vkGetQueueCheckpointData2NV");
}

void ThreadSafety::PostCallRecordGetQueueCheckpointData2NV(
    VkQueue                                     queue,
    uint32_t*                                   pCheckpointDataCount,
    VkCheckpointData2NV*                        pCheckpointData) {
    FinishReadObject(queue, "vkGetQueueCheckpointData2NV");
}

void ThreadSafety::PreCallRecordCmdCopyBuffer2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkCopyBufferInfo2*                    pCopyBufferInfo) {
    StartWriteObject(commandBuffer, "vkCmdCopyBuffer2KHR");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdCopyBuffer2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkCopyBufferInfo2*                    pCopyBufferInfo) {
    FinishWriteObject(commandBuffer, "vkCmdCopyBuffer2KHR");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdCopyImage2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkCopyImageInfo2*                     pCopyImageInfo) {
    StartWriteObject(commandBuffer, "vkCmdCopyImage2KHR");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdCopyImage2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkCopyImageInfo2*                     pCopyImageInfo) {
    FinishWriteObject(commandBuffer, "vkCmdCopyImage2KHR");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdCopyBufferToImage2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkCopyBufferToImageInfo2*             pCopyBufferToImageInfo) {
    StartWriteObject(commandBuffer, "vkCmdCopyBufferToImage2KHR");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdCopyBufferToImage2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkCopyBufferToImageInfo2*             pCopyBufferToImageInfo) {
    FinishWriteObject(commandBuffer, "vkCmdCopyBufferToImage2KHR");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdCopyImageToBuffer2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkCopyImageToBufferInfo2*             pCopyImageToBufferInfo) {
    StartWriteObject(commandBuffer, "vkCmdCopyImageToBuffer2KHR");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdCopyImageToBuffer2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkCopyImageToBufferInfo2*             pCopyImageToBufferInfo) {
    FinishWriteObject(commandBuffer, "vkCmdCopyImageToBuffer2KHR");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdBlitImage2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkBlitImageInfo2*                     pBlitImageInfo) {
    StartWriteObject(commandBuffer, "vkCmdBlitImage2KHR");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdBlitImage2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkBlitImageInfo2*                     pBlitImageInfo) {
    FinishWriteObject(commandBuffer, "vkCmdBlitImage2KHR");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdResolveImage2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkResolveImageInfo2*                  pResolveImageInfo) {
    StartWriteObject(commandBuffer, "vkCmdResolveImage2KHR");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdResolveImage2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkResolveImageInfo2*                  pResolveImageInfo) {
    FinishWriteObject(commandBuffer, "vkCmdResolveImage2KHR");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdTraceRaysIndirect2KHR(
    VkCommandBuffer                             commandBuffer,
    VkDeviceAddress                             indirectDeviceAddress) {
    StartWriteObject(commandBuffer, "vkCmdTraceRaysIndirect2KHR");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdTraceRaysIndirect2KHR(
    VkCommandBuffer                             commandBuffer,
    VkDeviceAddress                             indirectDeviceAddress) {
    FinishWriteObject(commandBuffer, "vkCmdTraceRaysIndirect2KHR");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordGetDeviceBufferMemoryRequirementsKHR(
    VkDevice                                    device,
    const VkDeviceBufferMemoryRequirements*     pInfo,
    VkMemoryRequirements2*                      pMemoryRequirements) {
    StartReadObjectParentInstance(device, "vkGetDeviceBufferMemoryRequirementsKHR");
}

void ThreadSafety::PostCallRecordGetDeviceBufferMemoryRequirementsKHR(
    VkDevice                                    device,
    const VkDeviceBufferMemoryRequirements*     pInfo,
    VkMemoryRequirements2*                      pMemoryRequirements) {
    FinishReadObjectParentInstance(device, "vkGetDeviceBufferMemoryRequirementsKHR");
}

void ThreadSafety::PreCallRecordGetDeviceImageMemoryRequirementsKHR(
    VkDevice                                    device,
    const VkDeviceImageMemoryRequirements*      pInfo,
    VkMemoryRequirements2*                      pMemoryRequirements) {
    StartReadObjectParentInstance(device, "vkGetDeviceImageMemoryRequirementsKHR");
}

void ThreadSafety::PostCallRecordGetDeviceImageMemoryRequirementsKHR(
    VkDevice                                    device,
    const VkDeviceImageMemoryRequirements*      pInfo,
    VkMemoryRequirements2*                      pMemoryRequirements) {
    FinishReadObjectParentInstance(device, "vkGetDeviceImageMemoryRequirementsKHR");
}

void ThreadSafety::PreCallRecordGetDeviceImageSparseMemoryRequirementsKHR(
    VkDevice                                    device,
    const VkDeviceImageMemoryRequirements*      pInfo,
    uint32_t*                                   pSparseMemoryRequirementCount,
    VkSparseImageMemoryRequirements2*           pSparseMemoryRequirements) {
    StartReadObjectParentInstance(device, "vkGetDeviceImageSparseMemoryRequirementsKHR");
}

void ThreadSafety::PostCallRecordGetDeviceImageSparseMemoryRequirementsKHR(
    VkDevice                                    device,
    const VkDeviceImageMemoryRequirements*      pInfo,
    uint32_t*                                   pSparseMemoryRequirementCount,
    VkSparseImageMemoryRequirements2*           pSparseMemoryRequirements) {
    FinishReadObjectParentInstance(device, "vkGetDeviceImageSparseMemoryRequirementsKHR");
}

void ThreadSafety::PreCallRecordCreateDebugReportCallbackEXT(
    VkInstance                                  instance,
    const VkDebugReportCallbackCreateInfoEXT*   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDebugReportCallbackEXT*                   pCallback) {
    StartReadObjectParentInstance(instance, "vkCreateDebugReportCallbackEXT");
}

void ThreadSafety::PostCallRecordCreateDebugReportCallbackEXT(
    VkInstance                                  instance,
    const VkDebugReportCallbackCreateInfoEXT*   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDebugReportCallbackEXT*                   pCallback,
    VkResult                                    result) {
    FinishReadObjectParentInstance(instance, "vkCreateDebugReportCallbackEXT");
    if (result == VK_SUCCESS) {
        CreateObjectParentInstance(*pCallback);
    }
}

void ThreadSafety::PreCallRecordDestroyDebugReportCallbackEXT(
    VkInstance                                  instance,
    VkDebugReportCallbackEXT                    callback,
    const VkAllocationCallbacks*                pAllocator) {
    StartReadObjectParentInstance(instance, "vkDestroyDebugReportCallbackEXT");
    StartWriteObjectParentInstance(callback, "vkDestroyDebugReportCallbackEXT");
    // Host access to callback must be externally synchronized
}

void ThreadSafety::PostCallRecordDestroyDebugReportCallbackEXT(
    VkInstance                                  instance,
    VkDebugReportCallbackEXT                    callback,
    const VkAllocationCallbacks*                pAllocator) {
    FinishReadObjectParentInstance(instance, "vkDestroyDebugReportCallbackEXT");
    FinishWriteObjectParentInstance(callback, "vkDestroyDebugReportCallbackEXT");
    DestroyObjectParentInstance(callback);
    // Host access to callback must be externally synchronized
}

void ThreadSafety::PreCallRecordDebugReportMessageEXT(
    VkInstance                                  instance,
    VkDebugReportFlagsEXT                       flags,
    VkDebugReportObjectTypeEXT                  objectType,
    uint64_t                                    object,
    size_t                                      location,
    int32_t                                     messageCode,
    const char*                                 pLayerPrefix,
    const char*                                 pMessage) {
    StartReadObjectParentInstance(instance, "vkDebugReportMessageEXT");
}

void ThreadSafety::PostCallRecordDebugReportMessageEXT(
    VkInstance                                  instance,
    VkDebugReportFlagsEXT                       flags,
    VkDebugReportObjectTypeEXT                  objectType,
    uint64_t                                    object,
    size_t                                      location,
    int32_t                                     messageCode,
    const char*                                 pLayerPrefix,
    const char*                                 pMessage) {
    FinishReadObjectParentInstance(instance, "vkDebugReportMessageEXT");
}
// TODO - not wrapping EXT function vkDebugMarkerSetObjectTagEXT
// TODO - not wrapping EXT function vkDebugMarkerSetObjectNameEXT
// TODO - not wrapping EXT function vkCmdDebugMarkerBeginEXT
// TODO - not wrapping EXT function vkCmdDebugMarkerEndEXT
// TODO - not wrapping EXT function vkCmdDebugMarkerInsertEXT

void ThreadSafety::PreCallRecordCmdBindTransformFeedbackBuffersEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstBinding,
    uint32_t                                    bindingCount,
    const VkBuffer*                             pBuffers,
    const VkDeviceSize*                         pOffsets,
    const VkDeviceSize*                         pSizes) {
    StartWriteObject(commandBuffer, "vkCmdBindTransformFeedbackBuffersEXT");
    if (pBuffers) {
        for (uint32_t index = 0; index < bindingCount; index++) {
            StartReadObject(pBuffers[index], "vkCmdBindTransformFeedbackBuffersEXT");
        }
    }
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdBindTransformFeedbackBuffersEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstBinding,
    uint32_t                                    bindingCount,
    const VkBuffer*                             pBuffers,
    const VkDeviceSize*                         pOffsets,
    const VkDeviceSize*                         pSizes) {
    FinishWriteObject(commandBuffer, "vkCmdBindTransformFeedbackBuffersEXT");
    if (pBuffers) {
        for (uint32_t index = 0; index < bindingCount; index++) {
            FinishReadObject(pBuffers[index], "vkCmdBindTransformFeedbackBuffersEXT");
        }
    }
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdBeginTransformFeedbackEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstCounterBuffer,
    uint32_t                                    counterBufferCount,
    const VkBuffer*                             pCounterBuffers,
    const VkDeviceSize*                         pCounterBufferOffsets) {
    StartWriteObject(commandBuffer, "vkCmdBeginTransformFeedbackEXT");
    if (pCounterBuffers) {
        for (uint32_t index = 0; index < counterBufferCount; index++) {
            StartReadObject(pCounterBuffers[index], "vkCmdBeginTransformFeedbackEXT");
        }
    }
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdBeginTransformFeedbackEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstCounterBuffer,
    uint32_t                                    counterBufferCount,
    const VkBuffer*                             pCounterBuffers,
    const VkDeviceSize*                         pCounterBufferOffsets) {
    FinishWriteObject(commandBuffer, "vkCmdBeginTransformFeedbackEXT");
    if (pCounterBuffers) {
        for (uint32_t index = 0; index < counterBufferCount; index++) {
            FinishReadObject(pCounterBuffers[index], "vkCmdBeginTransformFeedbackEXT");
        }
    }
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdEndTransformFeedbackEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstCounterBuffer,
    uint32_t                                    counterBufferCount,
    const VkBuffer*                             pCounterBuffers,
    const VkDeviceSize*                         pCounterBufferOffsets) {
    StartWriteObject(commandBuffer, "vkCmdEndTransformFeedbackEXT");
    if (pCounterBuffers) {
        for (uint32_t index = 0; index < counterBufferCount; index++) {
            StartReadObject(pCounterBuffers[index], "vkCmdEndTransformFeedbackEXT");
        }
    }
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdEndTransformFeedbackEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstCounterBuffer,
    uint32_t                                    counterBufferCount,
    const VkBuffer*                             pCounterBuffers,
    const VkDeviceSize*                         pCounterBufferOffsets) {
    FinishWriteObject(commandBuffer, "vkCmdEndTransformFeedbackEXT");
    if (pCounterBuffers) {
        for (uint32_t index = 0; index < counterBufferCount; index++) {
            FinishReadObject(pCounterBuffers[index], "vkCmdEndTransformFeedbackEXT");
        }
    }
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdBeginQueryIndexedEXT(
    VkCommandBuffer                             commandBuffer,
    VkQueryPool                                 queryPool,
    uint32_t                                    query,
    VkQueryControlFlags                         flags,
    uint32_t                                    index) {
    StartWriteObject(commandBuffer, "vkCmdBeginQueryIndexedEXT");
    StartReadObject(queryPool, "vkCmdBeginQueryIndexedEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdBeginQueryIndexedEXT(
    VkCommandBuffer                             commandBuffer,
    VkQueryPool                                 queryPool,
    uint32_t                                    query,
    VkQueryControlFlags                         flags,
    uint32_t                                    index) {
    FinishWriteObject(commandBuffer, "vkCmdBeginQueryIndexedEXT");
    FinishReadObject(queryPool, "vkCmdBeginQueryIndexedEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdEndQueryIndexedEXT(
    VkCommandBuffer                             commandBuffer,
    VkQueryPool                                 queryPool,
    uint32_t                                    query,
    uint32_t                                    index) {
    StartWriteObject(commandBuffer, "vkCmdEndQueryIndexedEXT");
    StartReadObject(queryPool, "vkCmdEndQueryIndexedEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdEndQueryIndexedEXT(
    VkCommandBuffer                             commandBuffer,
    VkQueryPool                                 queryPool,
    uint32_t                                    query,
    uint32_t                                    index) {
    FinishWriteObject(commandBuffer, "vkCmdEndQueryIndexedEXT");
    FinishReadObject(queryPool, "vkCmdEndQueryIndexedEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdDrawIndirectByteCountEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    instanceCount,
    uint32_t                                    firstInstance,
    VkBuffer                                    counterBuffer,
    VkDeviceSize                                counterBufferOffset,
    uint32_t                                    counterOffset,
    uint32_t                                    vertexStride) {
    StartWriteObject(commandBuffer, "vkCmdDrawIndirectByteCountEXT");
    StartReadObject(counterBuffer, "vkCmdDrawIndirectByteCountEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdDrawIndirectByteCountEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    instanceCount,
    uint32_t                                    firstInstance,
    VkBuffer                                    counterBuffer,
    VkDeviceSize                                counterBufferOffset,
    uint32_t                                    counterOffset,
    uint32_t                                    vertexStride) {
    FinishWriteObject(commandBuffer, "vkCmdDrawIndirectByteCountEXT");
    FinishReadObject(counterBuffer, "vkCmdDrawIndirectByteCountEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCreateCuModuleNVX(
    VkDevice                                    device,
    const VkCuModuleCreateInfoNVX*              pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkCuModuleNVX*                              pModule) {
    StartReadObjectParentInstance(device, "vkCreateCuModuleNVX");
}

void ThreadSafety::PostCallRecordCreateCuModuleNVX(
    VkDevice                                    device,
    const VkCuModuleCreateInfoNVX*              pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkCuModuleNVX*                              pModule,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkCreateCuModuleNVX");
    if (result == VK_SUCCESS) {
        CreateObject(*pModule);
    }
}

void ThreadSafety::PreCallRecordCreateCuFunctionNVX(
    VkDevice                                    device,
    const VkCuFunctionCreateInfoNVX*            pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkCuFunctionNVX*                            pFunction) {
    StartReadObjectParentInstance(device, "vkCreateCuFunctionNVX");
}

void ThreadSafety::PostCallRecordCreateCuFunctionNVX(
    VkDevice                                    device,
    const VkCuFunctionCreateInfoNVX*            pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkCuFunctionNVX*                            pFunction,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkCreateCuFunctionNVX");
    if (result == VK_SUCCESS) {
        CreateObject(*pFunction);
    }
}

void ThreadSafety::PreCallRecordDestroyCuModuleNVX(
    VkDevice                                    device,
    VkCuModuleNVX                               module,
    const VkAllocationCallbacks*                pAllocator) {
    StartReadObjectParentInstance(device, "vkDestroyCuModuleNVX");
    StartReadObject(module, "vkDestroyCuModuleNVX");
}

void ThreadSafety::PostCallRecordDestroyCuModuleNVX(
    VkDevice                                    device,
    VkCuModuleNVX                               module,
    const VkAllocationCallbacks*                pAllocator) {
    FinishReadObjectParentInstance(device, "vkDestroyCuModuleNVX");
    FinishReadObject(module, "vkDestroyCuModuleNVX");
}

void ThreadSafety::PreCallRecordDestroyCuFunctionNVX(
    VkDevice                                    device,
    VkCuFunctionNVX                             function,
    const VkAllocationCallbacks*                pAllocator) {
    StartReadObjectParentInstance(device, "vkDestroyCuFunctionNVX");
    StartReadObject(function, "vkDestroyCuFunctionNVX");
}

void ThreadSafety::PostCallRecordDestroyCuFunctionNVX(
    VkDevice                                    device,
    VkCuFunctionNVX                             function,
    const VkAllocationCallbacks*                pAllocator) {
    FinishReadObjectParentInstance(device, "vkDestroyCuFunctionNVX");
    FinishReadObject(function, "vkDestroyCuFunctionNVX");
}

void ThreadSafety::PreCallRecordCmdCuLaunchKernelNVX(
    VkCommandBuffer                             commandBuffer,
    const VkCuLaunchInfoNVX*                    pLaunchInfo) {
    StartReadObject(commandBuffer, "vkCmdCuLaunchKernelNVX");
}

void ThreadSafety::PostCallRecordCmdCuLaunchKernelNVX(
    VkCommandBuffer                             commandBuffer,
    const VkCuLaunchInfoNVX*                    pLaunchInfo) {
    FinishReadObject(commandBuffer, "vkCmdCuLaunchKernelNVX");
}

void ThreadSafety::PreCallRecordGetImageViewHandleNVX(
    VkDevice                                    device,
    const VkImageViewHandleInfoNVX*             pInfo) {
    StartReadObjectParentInstance(device, "vkGetImageViewHandleNVX");
}

void ThreadSafety::PostCallRecordGetImageViewHandleNVX(
    VkDevice                                    device,
    const VkImageViewHandleInfoNVX*             pInfo) {
    FinishReadObjectParentInstance(device, "vkGetImageViewHandleNVX");
}

void ThreadSafety::PreCallRecordGetImageViewAddressNVX(
    VkDevice                                    device,
    VkImageView                                 imageView,
    VkImageViewAddressPropertiesNVX*            pProperties) {
    StartReadObjectParentInstance(device, "vkGetImageViewAddressNVX");
    StartReadObject(imageView, "vkGetImageViewAddressNVX");
}

void ThreadSafety::PostCallRecordGetImageViewAddressNVX(
    VkDevice                                    device,
    VkImageView                                 imageView,
    VkImageViewAddressPropertiesNVX*            pProperties,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkGetImageViewAddressNVX");
    FinishReadObject(imageView, "vkGetImageViewAddressNVX");
}

void ThreadSafety::PreCallRecordCmdDrawIndirectCountAMD(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkBuffer                                    countBuffer,
    VkDeviceSize                                countBufferOffset,
    uint32_t                                    maxDrawCount,
    uint32_t                                    stride) {
    StartWriteObject(commandBuffer, "vkCmdDrawIndirectCountAMD");
    StartReadObject(buffer, "vkCmdDrawIndirectCountAMD");
    StartReadObject(countBuffer, "vkCmdDrawIndirectCountAMD");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdDrawIndirectCountAMD(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkBuffer                                    countBuffer,
    VkDeviceSize                                countBufferOffset,
    uint32_t                                    maxDrawCount,
    uint32_t                                    stride) {
    FinishWriteObject(commandBuffer, "vkCmdDrawIndirectCountAMD");
    FinishReadObject(buffer, "vkCmdDrawIndirectCountAMD");
    FinishReadObject(countBuffer, "vkCmdDrawIndirectCountAMD");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdDrawIndexedIndirectCountAMD(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkBuffer                                    countBuffer,
    VkDeviceSize                                countBufferOffset,
    uint32_t                                    maxDrawCount,
    uint32_t                                    stride) {
    StartWriteObject(commandBuffer, "vkCmdDrawIndexedIndirectCountAMD");
    StartReadObject(buffer, "vkCmdDrawIndexedIndirectCountAMD");
    StartReadObject(countBuffer, "vkCmdDrawIndexedIndirectCountAMD");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdDrawIndexedIndirectCountAMD(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkBuffer                                    countBuffer,
    VkDeviceSize                                countBufferOffset,
    uint32_t                                    maxDrawCount,
    uint32_t                                    stride) {
    FinishWriteObject(commandBuffer, "vkCmdDrawIndexedIndirectCountAMD");
    FinishReadObject(buffer, "vkCmdDrawIndexedIndirectCountAMD");
    FinishReadObject(countBuffer, "vkCmdDrawIndexedIndirectCountAMD");
    // Host access to commandBuffer must be externally synchronized
}

#ifdef VK_ENABLE_BETA_EXTENSIONS
#endif // VK_ENABLE_BETA_EXTENSIONS

#ifdef VK_ENABLE_BETA_EXTENSIONS
#endif // VK_ENABLE_BETA_EXTENSIONS

void ThreadSafety::PreCallRecordGetShaderInfoAMD(
    VkDevice                                    device,
    VkPipeline                                  pipeline,
    VkShaderStageFlagBits                       shaderStage,
    VkShaderInfoTypeAMD                         infoType,
    size_t*                                     pInfoSize,
    void*                                       pInfo) {
    StartReadObjectParentInstance(device, "vkGetShaderInfoAMD");
    StartReadObject(pipeline, "vkGetShaderInfoAMD");
}

void ThreadSafety::PostCallRecordGetShaderInfoAMD(
    VkDevice                                    device,
    VkPipeline                                  pipeline,
    VkShaderStageFlagBits                       shaderStage,
    VkShaderInfoTypeAMD                         infoType,
    size_t*                                     pInfoSize,
    void*                                       pInfo,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkGetShaderInfoAMD");
    FinishReadObject(pipeline, "vkGetShaderInfoAMD");
}

#ifdef VK_USE_PLATFORM_GGP

void ThreadSafety::PreCallRecordCreateStreamDescriptorSurfaceGGP(
    VkInstance                                  instance,
    const VkStreamDescriptorSurfaceCreateInfoGGP* pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) {
    StartReadObjectParentInstance(instance, "vkCreateStreamDescriptorSurfaceGGP");
}

void ThreadSafety::PostCallRecordCreateStreamDescriptorSurfaceGGP(
    VkInstance                                  instance,
    const VkStreamDescriptorSurfaceCreateInfoGGP* pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface,
    VkResult                                    result) {
    FinishReadObjectParentInstance(instance, "vkCreateStreamDescriptorSurfaceGGP");
    if (result == VK_SUCCESS) {
        CreateObjectParentInstance(*pSurface);
    }
}
#endif // VK_USE_PLATFORM_GGP

#ifdef VK_USE_PLATFORM_WIN32_KHR

void ThreadSafety::PreCallRecordGetMemoryWin32HandleNV(
    VkDevice                                    device,
    VkDeviceMemory                              memory,
    VkExternalMemoryHandleTypeFlagsNV           handleType,
    HANDLE*                                     pHandle) {
    StartReadObjectParentInstance(device, "vkGetMemoryWin32HandleNV");
    StartReadObject(memory, "vkGetMemoryWin32HandleNV");
}

void ThreadSafety::PostCallRecordGetMemoryWin32HandleNV(
    VkDevice                                    device,
    VkDeviceMemory                              memory,
    VkExternalMemoryHandleTypeFlagsNV           handleType,
    HANDLE*                                     pHandle,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkGetMemoryWin32HandleNV");
    FinishReadObject(memory, "vkGetMemoryWin32HandleNV");
}
#endif // VK_USE_PLATFORM_WIN32_KHR

#ifdef VK_USE_PLATFORM_WIN32_KHR
#endif // VK_USE_PLATFORM_WIN32_KHR

#ifdef VK_USE_PLATFORM_VI_NN

void ThreadSafety::PreCallRecordCreateViSurfaceNN(
    VkInstance                                  instance,
    const VkViSurfaceCreateInfoNN*              pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) {
    StartReadObjectParentInstance(instance, "vkCreateViSurfaceNN");
}

void ThreadSafety::PostCallRecordCreateViSurfaceNN(
    VkInstance                                  instance,
    const VkViSurfaceCreateInfoNN*              pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface,
    VkResult                                    result) {
    FinishReadObjectParentInstance(instance, "vkCreateViSurfaceNN");
    if (result == VK_SUCCESS) {
        CreateObjectParentInstance(*pSurface);
    }
}
#endif // VK_USE_PLATFORM_VI_NN

void ThreadSafety::PreCallRecordCmdBeginConditionalRenderingEXT(
    VkCommandBuffer                             commandBuffer,
    const VkConditionalRenderingBeginInfoEXT*   pConditionalRenderingBegin) {
    StartWriteObject(commandBuffer, "vkCmdBeginConditionalRenderingEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdBeginConditionalRenderingEXT(
    VkCommandBuffer                             commandBuffer,
    const VkConditionalRenderingBeginInfoEXT*   pConditionalRenderingBegin) {
    FinishWriteObject(commandBuffer, "vkCmdBeginConditionalRenderingEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdEndConditionalRenderingEXT(
    VkCommandBuffer                             commandBuffer) {
    StartWriteObject(commandBuffer, "vkCmdEndConditionalRenderingEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdEndConditionalRenderingEXT(
    VkCommandBuffer                             commandBuffer) {
    FinishWriteObject(commandBuffer, "vkCmdEndConditionalRenderingEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetViewportWScalingNV(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstViewport,
    uint32_t                                    viewportCount,
    const VkViewportWScalingNV*                 pViewportWScalings) {
    StartWriteObject(commandBuffer, "vkCmdSetViewportWScalingNV");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetViewportWScalingNV(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstViewport,
    uint32_t                                    viewportCount,
    const VkViewportWScalingNV*                 pViewportWScalings) {
    FinishWriteObject(commandBuffer, "vkCmdSetViewportWScalingNV");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordReleaseDisplayEXT(
    VkPhysicalDevice                            physicalDevice,
    VkDisplayKHR                                display) {
    StartReadObjectParentInstance(display, "vkReleaseDisplayEXT");
}

void ThreadSafety::PostCallRecordReleaseDisplayEXT(
    VkPhysicalDevice                            physicalDevice,
    VkDisplayKHR                                display,
    VkResult                                    result) {
    FinishReadObjectParentInstance(display, "vkReleaseDisplayEXT");
}

#ifdef VK_USE_PLATFORM_XLIB_XRANDR_EXT

void ThreadSafety::PreCallRecordAcquireXlibDisplayEXT(
    VkPhysicalDevice                            physicalDevice,
    Display*                                    dpy,
    VkDisplayKHR                                display) {
    StartReadObjectParentInstance(display, "vkAcquireXlibDisplayEXT");
}

void ThreadSafety::PostCallRecordAcquireXlibDisplayEXT(
    VkPhysicalDevice                            physicalDevice,
    Display*                                    dpy,
    VkDisplayKHR                                display,
    VkResult                                    result) {
    FinishReadObjectParentInstance(display, "vkAcquireXlibDisplayEXT");
}
#endif // VK_USE_PLATFORM_XLIB_XRANDR_EXT

void ThreadSafety::PreCallRecordGetPhysicalDeviceSurfaceCapabilities2EXT(
    VkPhysicalDevice                            physicalDevice,
    VkSurfaceKHR                                surface,
    VkSurfaceCapabilities2EXT*                  pSurfaceCapabilities) {
    StartReadObjectParentInstance(surface, "vkGetPhysicalDeviceSurfaceCapabilities2EXT");
}

void ThreadSafety::PostCallRecordGetPhysicalDeviceSurfaceCapabilities2EXT(
    VkPhysicalDevice                            physicalDevice,
    VkSurfaceKHR                                surface,
    VkSurfaceCapabilities2EXT*                  pSurfaceCapabilities,
    VkResult                                    result) {
    FinishReadObjectParentInstance(surface, "vkGetPhysicalDeviceSurfaceCapabilities2EXT");
}

void ThreadSafety::PreCallRecordDisplayPowerControlEXT(
    VkDevice                                    device,
    VkDisplayKHR                                display,
    const VkDisplayPowerInfoEXT*                pDisplayPowerInfo) {
    StartReadObjectParentInstance(device, "vkDisplayPowerControlEXT");
    StartReadObjectParentInstance(display, "vkDisplayPowerControlEXT");
}

void ThreadSafety::PostCallRecordDisplayPowerControlEXT(
    VkDevice                                    device,
    VkDisplayKHR                                display,
    const VkDisplayPowerInfoEXT*                pDisplayPowerInfo,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkDisplayPowerControlEXT");
    FinishReadObjectParentInstance(display, "vkDisplayPowerControlEXT");
}

void ThreadSafety::PreCallRecordRegisterDeviceEventEXT(
    VkDevice                                    device,
    const VkDeviceEventInfoEXT*                 pDeviceEventInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkFence*                                    pFence) {
    StartReadObjectParentInstance(device, "vkRegisterDeviceEventEXT");
}

void ThreadSafety::PostCallRecordRegisterDeviceEventEXT(
    VkDevice                                    device,
    const VkDeviceEventInfoEXT*                 pDeviceEventInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkFence*                                    pFence,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkRegisterDeviceEventEXT");
}

void ThreadSafety::PreCallRecordGetSwapchainCounterEXT(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    VkSurfaceCounterFlagBitsEXT                 counter,
    uint64_t*                                   pCounterValue) {
    StartReadObjectParentInstance(device, "vkGetSwapchainCounterEXT");
    StartReadObjectParentInstance(swapchain, "vkGetSwapchainCounterEXT");
}

void ThreadSafety::PostCallRecordGetSwapchainCounterEXT(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    VkSurfaceCounterFlagBitsEXT                 counter,
    uint64_t*                                   pCounterValue,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkGetSwapchainCounterEXT");
    FinishReadObjectParentInstance(swapchain, "vkGetSwapchainCounterEXT");
}

void ThreadSafety::PreCallRecordGetRefreshCycleDurationGOOGLE(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    VkRefreshCycleDurationGOOGLE*               pDisplayTimingProperties) {
    StartReadObjectParentInstance(device, "vkGetRefreshCycleDurationGOOGLE");
    StartWriteObjectParentInstance(swapchain, "vkGetRefreshCycleDurationGOOGLE");
    // Host access to swapchain must be externally synchronized
}

void ThreadSafety::PostCallRecordGetRefreshCycleDurationGOOGLE(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    VkRefreshCycleDurationGOOGLE*               pDisplayTimingProperties,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkGetRefreshCycleDurationGOOGLE");
    FinishWriteObjectParentInstance(swapchain, "vkGetRefreshCycleDurationGOOGLE");
    // Host access to swapchain must be externally synchronized
}

void ThreadSafety::PreCallRecordGetPastPresentationTimingGOOGLE(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    uint32_t*                                   pPresentationTimingCount,
    VkPastPresentationTimingGOOGLE*             pPresentationTimings) {
    StartReadObjectParentInstance(device, "vkGetPastPresentationTimingGOOGLE");
    StartWriteObjectParentInstance(swapchain, "vkGetPastPresentationTimingGOOGLE");
    // Host access to swapchain must be externally synchronized
}

void ThreadSafety::PostCallRecordGetPastPresentationTimingGOOGLE(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    uint32_t*                                   pPresentationTimingCount,
    VkPastPresentationTimingGOOGLE*             pPresentationTimings,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkGetPastPresentationTimingGOOGLE");
    FinishWriteObjectParentInstance(swapchain, "vkGetPastPresentationTimingGOOGLE");
    // Host access to swapchain must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetDiscardRectangleEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstDiscardRectangle,
    uint32_t                                    discardRectangleCount,
    const VkRect2D*                             pDiscardRectangles) {
    StartWriteObject(commandBuffer, "vkCmdSetDiscardRectangleEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetDiscardRectangleEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstDiscardRectangle,
    uint32_t                                    discardRectangleCount,
    const VkRect2D*                             pDiscardRectangles) {
    FinishWriteObject(commandBuffer, "vkCmdSetDiscardRectangleEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordSetHdrMetadataEXT(
    VkDevice                                    device,
    uint32_t                                    swapchainCount,
    const VkSwapchainKHR*                       pSwapchains,
    const VkHdrMetadataEXT*                     pMetadata) {
    StartReadObjectParentInstance(device, "vkSetHdrMetadataEXT");
    if (pSwapchains) {
        for (uint32_t index = 0; index < swapchainCount; index++) {
            StartReadObjectParentInstance(pSwapchains[index], "vkSetHdrMetadataEXT");
        }
    }
}

void ThreadSafety::PostCallRecordSetHdrMetadataEXT(
    VkDevice                                    device,
    uint32_t                                    swapchainCount,
    const VkSwapchainKHR*                       pSwapchains,
    const VkHdrMetadataEXT*                     pMetadata) {
    FinishReadObjectParentInstance(device, "vkSetHdrMetadataEXT");
    if (pSwapchains) {
        for (uint32_t index = 0; index < swapchainCount; index++) {
            FinishReadObjectParentInstance(pSwapchains[index], "vkSetHdrMetadataEXT");
        }
    }
}

#ifdef VK_USE_PLATFORM_IOS_MVK

void ThreadSafety::PreCallRecordCreateIOSSurfaceMVK(
    VkInstance                                  instance,
    const VkIOSSurfaceCreateInfoMVK*            pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) {
    StartReadObjectParentInstance(instance, "vkCreateIOSSurfaceMVK");
}

void ThreadSafety::PostCallRecordCreateIOSSurfaceMVK(
    VkInstance                                  instance,
    const VkIOSSurfaceCreateInfoMVK*            pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface,
    VkResult                                    result) {
    FinishReadObjectParentInstance(instance, "vkCreateIOSSurfaceMVK");
    if (result == VK_SUCCESS) {
        CreateObjectParentInstance(*pSurface);
    }
}
#endif // VK_USE_PLATFORM_IOS_MVK

#ifdef VK_USE_PLATFORM_MACOS_MVK

void ThreadSafety::PreCallRecordCreateMacOSSurfaceMVK(
    VkInstance                                  instance,
    const VkMacOSSurfaceCreateInfoMVK*          pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) {
    StartReadObjectParentInstance(instance, "vkCreateMacOSSurfaceMVK");
}

void ThreadSafety::PostCallRecordCreateMacOSSurfaceMVK(
    VkInstance                                  instance,
    const VkMacOSSurfaceCreateInfoMVK*          pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface,
    VkResult                                    result) {
    FinishReadObjectParentInstance(instance, "vkCreateMacOSSurfaceMVK");
    if (result == VK_SUCCESS) {
        CreateObjectParentInstance(*pSurface);
    }
}
#endif // VK_USE_PLATFORM_MACOS_MVK
// TODO - not wrapping EXT function vkSetDebugUtilsObjectNameEXT
// TODO - not wrapping EXT function vkSetDebugUtilsObjectTagEXT

void ThreadSafety::PreCallRecordQueueBeginDebugUtilsLabelEXT(
    VkQueue                                     queue,
    const VkDebugUtilsLabelEXT*                 pLabelInfo) {
    StartReadObject(queue, "vkQueueBeginDebugUtilsLabelEXT");
}

void ThreadSafety::PostCallRecordQueueBeginDebugUtilsLabelEXT(
    VkQueue                                     queue,
    const VkDebugUtilsLabelEXT*                 pLabelInfo) {
    FinishReadObject(queue, "vkQueueBeginDebugUtilsLabelEXT");
}

void ThreadSafety::PreCallRecordQueueEndDebugUtilsLabelEXT(
    VkQueue                                     queue) {
    StartReadObject(queue, "vkQueueEndDebugUtilsLabelEXT");
}

void ThreadSafety::PostCallRecordQueueEndDebugUtilsLabelEXT(
    VkQueue                                     queue) {
    FinishReadObject(queue, "vkQueueEndDebugUtilsLabelEXT");
}

void ThreadSafety::PreCallRecordQueueInsertDebugUtilsLabelEXT(
    VkQueue                                     queue,
    const VkDebugUtilsLabelEXT*                 pLabelInfo) {
    StartReadObject(queue, "vkQueueInsertDebugUtilsLabelEXT");
}

void ThreadSafety::PostCallRecordQueueInsertDebugUtilsLabelEXT(
    VkQueue                                     queue,
    const VkDebugUtilsLabelEXT*                 pLabelInfo) {
    FinishReadObject(queue, "vkQueueInsertDebugUtilsLabelEXT");
}

void ThreadSafety::PreCallRecordCmdBeginDebugUtilsLabelEXT(
    VkCommandBuffer                             commandBuffer,
    const VkDebugUtilsLabelEXT*                 pLabelInfo) {
    StartWriteObject(commandBuffer, "vkCmdBeginDebugUtilsLabelEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdBeginDebugUtilsLabelEXT(
    VkCommandBuffer                             commandBuffer,
    const VkDebugUtilsLabelEXT*                 pLabelInfo) {
    FinishWriteObject(commandBuffer, "vkCmdBeginDebugUtilsLabelEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdEndDebugUtilsLabelEXT(
    VkCommandBuffer                             commandBuffer) {
    StartWriteObject(commandBuffer, "vkCmdEndDebugUtilsLabelEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdEndDebugUtilsLabelEXT(
    VkCommandBuffer                             commandBuffer) {
    FinishWriteObject(commandBuffer, "vkCmdEndDebugUtilsLabelEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdInsertDebugUtilsLabelEXT(
    VkCommandBuffer                             commandBuffer,
    const VkDebugUtilsLabelEXT*                 pLabelInfo) {
    StartWriteObject(commandBuffer, "vkCmdInsertDebugUtilsLabelEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdInsertDebugUtilsLabelEXT(
    VkCommandBuffer                             commandBuffer,
    const VkDebugUtilsLabelEXT*                 pLabelInfo) {
    FinishWriteObject(commandBuffer, "vkCmdInsertDebugUtilsLabelEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCreateDebugUtilsMessengerEXT(
    VkInstance                                  instance,
    const VkDebugUtilsMessengerCreateInfoEXT*   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDebugUtilsMessengerEXT*                   pMessenger) {
    StartReadObjectParentInstance(instance, "vkCreateDebugUtilsMessengerEXT");
}

void ThreadSafety::PostCallRecordCreateDebugUtilsMessengerEXT(
    VkInstance                                  instance,
    const VkDebugUtilsMessengerCreateInfoEXT*   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDebugUtilsMessengerEXT*                   pMessenger,
    VkResult                                    result) {
    FinishReadObjectParentInstance(instance, "vkCreateDebugUtilsMessengerEXT");
    if (result == VK_SUCCESS) {
        CreateObjectParentInstance(*pMessenger);
    }
}

void ThreadSafety::PreCallRecordDestroyDebugUtilsMessengerEXT(
    VkInstance                                  instance,
    VkDebugUtilsMessengerEXT                    messenger,
    const VkAllocationCallbacks*                pAllocator) {
    StartReadObjectParentInstance(instance, "vkDestroyDebugUtilsMessengerEXT");
    StartWriteObjectParentInstance(messenger, "vkDestroyDebugUtilsMessengerEXT");
    // Host access to messenger must be externally synchronized
}

void ThreadSafety::PostCallRecordDestroyDebugUtilsMessengerEXT(
    VkInstance                                  instance,
    VkDebugUtilsMessengerEXT                    messenger,
    const VkAllocationCallbacks*                pAllocator) {
    FinishReadObjectParentInstance(instance, "vkDestroyDebugUtilsMessengerEXT");
    FinishWriteObjectParentInstance(messenger, "vkDestroyDebugUtilsMessengerEXT");
    DestroyObjectParentInstance(messenger);
    // Host access to messenger must be externally synchronized
}

void ThreadSafety::PreCallRecordSubmitDebugUtilsMessageEXT(
    VkInstance                                  instance,
    VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT             messageTypes,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData) {
    StartReadObjectParentInstance(instance, "vkSubmitDebugUtilsMessageEXT");
}

void ThreadSafety::PostCallRecordSubmitDebugUtilsMessageEXT(
    VkInstance                                  instance,
    VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT             messageTypes,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData) {
    FinishReadObjectParentInstance(instance, "vkSubmitDebugUtilsMessageEXT");
}

#ifdef VK_USE_PLATFORM_ANDROID_KHR

void ThreadSafety::PreCallRecordGetAndroidHardwareBufferPropertiesANDROID(
    VkDevice                                    device,
    const struct AHardwareBuffer*               buffer,
    VkAndroidHardwareBufferPropertiesANDROID*   pProperties) {
    StartReadObjectParentInstance(device, "vkGetAndroidHardwareBufferPropertiesANDROID");
}

void ThreadSafety::PostCallRecordGetAndroidHardwareBufferPropertiesANDROID(
    VkDevice                                    device,
    const struct AHardwareBuffer*               buffer,
    VkAndroidHardwareBufferPropertiesANDROID*   pProperties,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkGetAndroidHardwareBufferPropertiesANDROID");
}

void ThreadSafety::PreCallRecordGetMemoryAndroidHardwareBufferANDROID(
    VkDevice                                    device,
    const VkMemoryGetAndroidHardwareBufferInfoANDROID* pInfo,
    struct AHardwareBuffer**                    pBuffer) {
    StartReadObjectParentInstance(device, "vkGetMemoryAndroidHardwareBufferANDROID");
}

void ThreadSafety::PostCallRecordGetMemoryAndroidHardwareBufferANDROID(
    VkDevice                                    device,
    const VkMemoryGetAndroidHardwareBufferInfoANDROID* pInfo,
    struct AHardwareBuffer**                    pBuffer,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkGetMemoryAndroidHardwareBufferANDROID");
}
#endif // VK_USE_PLATFORM_ANDROID_KHR

void ThreadSafety::PreCallRecordCmdSetSampleLocationsEXT(
    VkCommandBuffer                             commandBuffer,
    const VkSampleLocationsInfoEXT*             pSampleLocationsInfo) {
    StartWriteObject(commandBuffer, "vkCmdSetSampleLocationsEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetSampleLocationsEXT(
    VkCommandBuffer                             commandBuffer,
    const VkSampleLocationsInfoEXT*             pSampleLocationsInfo) {
    FinishWriteObject(commandBuffer, "vkCmdSetSampleLocationsEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordGetImageDrmFormatModifierPropertiesEXT(
    VkDevice                                    device,
    VkImage                                     image,
    VkImageDrmFormatModifierPropertiesEXT*      pProperties) {
    StartReadObjectParentInstance(device, "vkGetImageDrmFormatModifierPropertiesEXT");
    StartReadObject(image, "vkGetImageDrmFormatModifierPropertiesEXT");
}

void ThreadSafety::PostCallRecordGetImageDrmFormatModifierPropertiesEXT(
    VkDevice                                    device,
    VkImage                                     image,
    VkImageDrmFormatModifierPropertiesEXT*      pProperties,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkGetImageDrmFormatModifierPropertiesEXT");
    FinishReadObject(image, "vkGetImageDrmFormatModifierPropertiesEXT");
}

void ThreadSafety::PreCallRecordCreateValidationCacheEXT(
    VkDevice                                    device,
    const VkValidationCacheCreateInfoEXT*       pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkValidationCacheEXT*                       pValidationCache) {
    StartReadObjectParentInstance(device, "vkCreateValidationCacheEXT");
}

void ThreadSafety::PostCallRecordCreateValidationCacheEXT(
    VkDevice                                    device,
    const VkValidationCacheCreateInfoEXT*       pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkValidationCacheEXT*                       pValidationCache,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkCreateValidationCacheEXT");
    if (result == VK_SUCCESS) {
        CreateObject(*pValidationCache);
    }
}

void ThreadSafety::PreCallRecordDestroyValidationCacheEXT(
    VkDevice                                    device,
    VkValidationCacheEXT                        validationCache,
    const VkAllocationCallbacks*                pAllocator) {
    StartReadObjectParentInstance(device, "vkDestroyValidationCacheEXT");
    StartWriteObject(validationCache, "vkDestroyValidationCacheEXT");
    // Host access to validationCache must be externally synchronized
}

void ThreadSafety::PostCallRecordDestroyValidationCacheEXT(
    VkDevice                                    device,
    VkValidationCacheEXT                        validationCache,
    const VkAllocationCallbacks*                pAllocator) {
    FinishReadObjectParentInstance(device, "vkDestroyValidationCacheEXT");
    FinishWriteObject(validationCache, "vkDestroyValidationCacheEXT");
    DestroyObject(validationCache);
    // Host access to validationCache must be externally synchronized
}

void ThreadSafety::PreCallRecordMergeValidationCachesEXT(
    VkDevice                                    device,
    VkValidationCacheEXT                        dstCache,
    uint32_t                                    srcCacheCount,
    const VkValidationCacheEXT*                 pSrcCaches) {
    StartReadObjectParentInstance(device, "vkMergeValidationCachesEXT");
    StartWriteObject(dstCache, "vkMergeValidationCachesEXT");
    if (pSrcCaches) {
        for (uint32_t index = 0; index < srcCacheCount; index++) {
            StartReadObject(pSrcCaches[index], "vkMergeValidationCachesEXT");
        }
    }
    // Host access to dstCache must be externally synchronized
}

void ThreadSafety::PostCallRecordMergeValidationCachesEXT(
    VkDevice                                    device,
    VkValidationCacheEXT                        dstCache,
    uint32_t                                    srcCacheCount,
    const VkValidationCacheEXT*                 pSrcCaches,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkMergeValidationCachesEXT");
    FinishWriteObject(dstCache, "vkMergeValidationCachesEXT");
    if (pSrcCaches) {
        for (uint32_t index = 0; index < srcCacheCount; index++) {
            FinishReadObject(pSrcCaches[index], "vkMergeValidationCachesEXT");
        }
    }
    // Host access to dstCache must be externally synchronized
}

void ThreadSafety::PreCallRecordGetValidationCacheDataEXT(
    VkDevice                                    device,
    VkValidationCacheEXT                        validationCache,
    size_t*                                     pDataSize,
    void*                                       pData) {
    StartReadObjectParentInstance(device, "vkGetValidationCacheDataEXT");
    StartReadObject(validationCache, "vkGetValidationCacheDataEXT");
}

void ThreadSafety::PostCallRecordGetValidationCacheDataEXT(
    VkDevice                                    device,
    VkValidationCacheEXT                        validationCache,
    size_t*                                     pDataSize,
    void*                                       pData,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkGetValidationCacheDataEXT");
    FinishReadObject(validationCache, "vkGetValidationCacheDataEXT");
}

void ThreadSafety::PreCallRecordCmdBindShadingRateImageNV(
    VkCommandBuffer                             commandBuffer,
    VkImageView                                 imageView,
    VkImageLayout                               imageLayout) {
    StartWriteObject(commandBuffer, "vkCmdBindShadingRateImageNV");
    StartReadObject(imageView, "vkCmdBindShadingRateImageNV");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdBindShadingRateImageNV(
    VkCommandBuffer                             commandBuffer,
    VkImageView                                 imageView,
    VkImageLayout                               imageLayout) {
    FinishWriteObject(commandBuffer, "vkCmdBindShadingRateImageNV");
    FinishReadObject(imageView, "vkCmdBindShadingRateImageNV");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetViewportShadingRatePaletteNV(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstViewport,
    uint32_t                                    viewportCount,
    const VkShadingRatePaletteNV*               pShadingRatePalettes) {
    StartWriteObject(commandBuffer, "vkCmdSetViewportShadingRatePaletteNV");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetViewportShadingRatePaletteNV(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstViewport,
    uint32_t                                    viewportCount,
    const VkShadingRatePaletteNV*               pShadingRatePalettes) {
    FinishWriteObject(commandBuffer, "vkCmdSetViewportShadingRatePaletteNV");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetCoarseSampleOrderNV(
    VkCommandBuffer                             commandBuffer,
    VkCoarseSampleOrderTypeNV                   sampleOrderType,
    uint32_t                                    customSampleOrderCount,
    const VkCoarseSampleOrderCustomNV*          pCustomSampleOrders) {
    StartWriteObject(commandBuffer, "vkCmdSetCoarseSampleOrderNV");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetCoarseSampleOrderNV(
    VkCommandBuffer                             commandBuffer,
    VkCoarseSampleOrderTypeNV                   sampleOrderType,
    uint32_t                                    customSampleOrderCount,
    const VkCoarseSampleOrderCustomNV*          pCustomSampleOrders) {
    FinishWriteObject(commandBuffer, "vkCmdSetCoarseSampleOrderNV");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCreateAccelerationStructureNV(
    VkDevice                                    device,
    const VkAccelerationStructureCreateInfoNV*  pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkAccelerationStructureNV*                  pAccelerationStructure) {
    StartReadObjectParentInstance(device, "vkCreateAccelerationStructureNV");
}

void ThreadSafety::PostCallRecordCreateAccelerationStructureNV(
    VkDevice                                    device,
    const VkAccelerationStructureCreateInfoNV*  pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkAccelerationStructureNV*                  pAccelerationStructure,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkCreateAccelerationStructureNV");
    if (result == VK_SUCCESS) {
        CreateObject(*pAccelerationStructure);
    }
}

void ThreadSafety::PreCallRecordDestroyAccelerationStructureNV(
    VkDevice                                    device,
    VkAccelerationStructureNV                   accelerationStructure,
    const VkAllocationCallbacks*                pAllocator) {
    StartReadObjectParentInstance(device, "vkDestroyAccelerationStructureNV");
    StartWriteObject(accelerationStructure, "vkDestroyAccelerationStructureNV");
    // Host access to accelerationStructure must be externally synchronized
}

void ThreadSafety::PostCallRecordDestroyAccelerationStructureNV(
    VkDevice                                    device,
    VkAccelerationStructureNV                   accelerationStructure,
    const VkAllocationCallbacks*                pAllocator) {
    FinishReadObjectParentInstance(device, "vkDestroyAccelerationStructureNV");
    FinishWriteObject(accelerationStructure, "vkDestroyAccelerationStructureNV");
    DestroyObject(accelerationStructure);
    // Host access to accelerationStructure must be externally synchronized
}

void ThreadSafety::PreCallRecordGetAccelerationStructureMemoryRequirementsNV(
    VkDevice                                    device,
    const VkAccelerationStructureMemoryRequirementsInfoNV* pInfo,
    VkMemoryRequirements2KHR*                   pMemoryRequirements) {
    StartReadObjectParentInstance(device, "vkGetAccelerationStructureMemoryRequirementsNV");
}

void ThreadSafety::PostCallRecordGetAccelerationStructureMemoryRequirementsNV(
    VkDevice                                    device,
    const VkAccelerationStructureMemoryRequirementsInfoNV* pInfo,
    VkMemoryRequirements2KHR*                   pMemoryRequirements) {
    FinishReadObjectParentInstance(device, "vkGetAccelerationStructureMemoryRequirementsNV");
}

void ThreadSafety::PreCallRecordBindAccelerationStructureMemoryNV(
    VkDevice                                    device,
    uint32_t                                    bindInfoCount,
    const VkBindAccelerationStructureMemoryInfoNV* pBindInfos) {
    StartReadObjectParentInstance(device, "vkBindAccelerationStructureMemoryNV");
}

void ThreadSafety::PostCallRecordBindAccelerationStructureMemoryNV(
    VkDevice                                    device,
    uint32_t                                    bindInfoCount,
    const VkBindAccelerationStructureMemoryInfoNV* pBindInfos,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkBindAccelerationStructureMemoryNV");
}

void ThreadSafety::PreCallRecordCmdBuildAccelerationStructureNV(
    VkCommandBuffer                             commandBuffer,
    const VkAccelerationStructureInfoNV*        pInfo,
    VkBuffer                                    instanceData,
    VkDeviceSize                                instanceOffset,
    VkBool32                                    update,
    VkAccelerationStructureNV                   dst,
    VkAccelerationStructureNV                   src,
    VkBuffer                                    scratch,
    VkDeviceSize                                scratchOffset) {
    StartWriteObject(commandBuffer, "vkCmdBuildAccelerationStructureNV");
    StartReadObject(instanceData, "vkCmdBuildAccelerationStructureNV");
    StartReadObject(dst, "vkCmdBuildAccelerationStructureNV");
    StartReadObject(src, "vkCmdBuildAccelerationStructureNV");
    StartReadObject(scratch, "vkCmdBuildAccelerationStructureNV");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdBuildAccelerationStructureNV(
    VkCommandBuffer                             commandBuffer,
    const VkAccelerationStructureInfoNV*        pInfo,
    VkBuffer                                    instanceData,
    VkDeviceSize                                instanceOffset,
    VkBool32                                    update,
    VkAccelerationStructureNV                   dst,
    VkAccelerationStructureNV                   src,
    VkBuffer                                    scratch,
    VkDeviceSize                                scratchOffset) {
    FinishWriteObject(commandBuffer, "vkCmdBuildAccelerationStructureNV");
    FinishReadObject(instanceData, "vkCmdBuildAccelerationStructureNV");
    FinishReadObject(dst, "vkCmdBuildAccelerationStructureNV");
    FinishReadObject(src, "vkCmdBuildAccelerationStructureNV");
    FinishReadObject(scratch, "vkCmdBuildAccelerationStructureNV");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdCopyAccelerationStructureNV(
    VkCommandBuffer                             commandBuffer,
    VkAccelerationStructureNV                   dst,
    VkAccelerationStructureNV                   src,
    VkCopyAccelerationStructureModeKHR          mode) {
    StartWriteObject(commandBuffer, "vkCmdCopyAccelerationStructureNV");
    StartReadObject(dst, "vkCmdCopyAccelerationStructureNV");
    StartReadObject(src, "vkCmdCopyAccelerationStructureNV");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdCopyAccelerationStructureNV(
    VkCommandBuffer                             commandBuffer,
    VkAccelerationStructureNV                   dst,
    VkAccelerationStructureNV                   src,
    VkCopyAccelerationStructureModeKHR          mode) {
    FinishWriteObject(commandBuffer, "vkCmdCopyAccelerationStructureNV");
    FinishReadObject(dst, "vkCmdCopyAccelerationStructureNV");
    FinishReadObject(src, "vkCmdCopyAccelerationStructureNV");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdTraceRaysNV(
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
    uint32_t                                    depth) {
    StartWriteObject(commandBuffer, "vkCmdTraceRaysNV");
    StartReadObject(raygenShaderBindingTableBuffer, "vkCmdTraceRaysNV");
    StartReadObject(missShaderBindingTableBuffer, "vkCmdTraceRaysNV");
    StartReadObject(hitShaderBindingTableBuffer, "vkCmdTraceRaysNV");
    StartReadObject(callableShaderBindingTableBuffer, "vkCmdTraceRaysNV");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdTraceRaysNV(
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
    uint32_t                                    depth) {
    FinishWriteObject(commandBuffer, "vkCmdTraceRaysNV");
    FinishReadObject(raygenShaderBindingTableBuffer, "vkCmdTraceRaysNV");
    FinishReadObject(missShaderBindingTableBuffer, "vkCmdTraceRaysNV");
    FinishReadObject(hitShaderBindingTableBuffer, "vkCmdTraceRaysNV");
    FinishReadObject(callableShaderBindingTableBuffer, "vkCmdTraceRaysNV");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCreateRayTracingPipelinesNV(
    VkDevice                                    device,
    VkPipelineCache                             pipelineCache,
    uint32_t                                    createInfoCount,
    const VkRayTracingPipelineCreateInfoNV*     pCreateInfos,
    const VkAllocationCallbacks*                pAllocator,
    VkPipeline*                                 pPipelines) {
    StartReadObjectParentInstance(device, "vkCreateRayTracingPipelinesNV");
    StartReadObject(pipelineCache, "vkCreateRayTracingPipelinesNV");
}

void ThreadSafety::PostCallRecordCreateRayTracingPipelinesNV(
    VkDevice                                    device,
    VkPipelineCache                             pipelineCache,
    uint32_t                                    createInfoCount,
    const VkRayTracingPipelineCreateInfoNV*     pCreateInfos,
    const VkAllocationCallbacks*                pAllocator,
    VkPipeline*                                 pPipelines,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkCreateRayTracingPipelinesNV");
    FinishReadObject(pipelineCache, "vkCreateRayTracingPipelinesNV");
    if (pPipelines) {
        for (uint32_t index = 0; index < createInfoCount; index++) {
            if (!pPipelines[index]) continue;
            CreateObject(pPipelines[index]);
        }
    }
}

void ThreadSafety::PreCallRecordGetRayTracingShaderGroupHandlesKHR(
    VkDevice                                    device,
    VkPipeline                                  pipeline,
    uint32_t                                    firstGroup,
    uint32_t                                    groupCount,
    size_t                                      dataSize,
    void*                                       pData) {
    StartReadObjectParentInstance(device, "vkGetRayTracingShaderGroupHandlesKHR");
    StartReadObject(pipeline, "vkGetRayTracingShaderGroupHandlesKHR");
}

void ThreadSafety::PostCallRecordGetRayTracingShaderGroupHandlesKHR(
    VkDevice                                    device,
    VkPipeline                                  pipeline,
    uint32_t                                    firstGroup,
    uint32_t                                    groupCount,
    size_t                                      dataSize,
    void*                                       pData,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkGetRayTracingShaderGroupHandlesKHR");
    FinishReadObject(pipeline, "vkGetRayTracingShaderGroupHandlesKHR");
}

void ThreadSafety::PreCallRecordGetRayTracingShaderGroupHandlesNV(
    VkDevice                                    device,
    VkPipeline                                  pipeline,
    uint32_t                                    firstGroup,
    uint32_t                                    groupCount,
    size_t                                      dataSize,
    void*                                       pData) {
    StartReadObjectParentInstance(device, "vkGetRayTracingShaderGroupHandlesNV");
    StartReadObject(pipeline, "vkGetRayTracingShaderGroupHandlesNV");
}

void ThreadSafety::PostCallRecordGetRayTracingShaderGroupHandlesNV(
    VkDevice                                    device,
    VkPipeline                                  pipeline,
    uint32_t                                    firstGroup,
    uint32_t                                    groupCount,
    size_t                                      dataSize,
    void*                                       pData,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkGetRayTracingShaderGroupHandlesNV");
    FinishReadObject(pipeline, "vkGetRayTracingShaderGroupHandlesNV");
}

void ThreadSafety::PreCallRecordGetAccelerationStructureHandleNV(
    VkDevice                                    device,
    VkAccelerationStructureNV                   accelerationStructure,
    size_t                                      dataSize,
    void*                                       pData) {
    StartReadObjectParentInstance(device, "vkGetAccelerationStructureHandleNV");
    StartReadObject(accelerationStructure, "vkGetAccelerationStructureHandleNV");
}

void ThreadSafety::PostCallRecordGetAccelerationStructureHandleNV(
    VkDevice                                    device,
    VkAccelerationStructureNV                   accelerationStructure,
    size_t                                      dataSize,
    void*                                       pData,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkGetAccelerationStructureHandleNV");
    FinishReadObject(accelerationStructure, "vkGetAccelerationStructureHandleNV");
}

void ThreadSafety::PreCallRecordCmdWriteAccelerationStructuresPropertiesNV(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    accelerationStructureCount,
    const VkAccelerationStructureNV*            pAccelerationStructures,
    VkQueryType                                 queryType,
    VkQueryPool                                 queryPool,
    uint32_t                                    firstQuery) {
    StartWriteObject(commandBuffer, "vkCmdWriteAccelerationStructuresPropertiesNV");
    if (pAccelerationStructures) {
        for (uint32_t index = 0; index < accelerationStructureCount; index++) {
            StartReadObject(pAccelerationStructures[index], "vkCmdWriteAccelerationStructuresPropertiesNV");
        }
    }
    StartReadObject(queryPool, "vkCmdWriteAccelerationStructuresPropertiesNV");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdWriteAccelerationStructuresPropertiesNV(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    accelerationStructureCount,
    const VkAccelerationStructureNV*            pAccelerationStructures,
    VkQueryType                                 queryType,
    VkQueryPool                                 queryPool,
    uint32_t                                    firstQuery) {
    FinishWriteObject(commandBuffer, "vkCmdWriteAccelerationStructuresPropertiesNV");
    if (pAccelerationStructures) {
        for (uint32_t index = 0; index < accelerationStructureCount; index++) {
            FinishReadObject(pAccelerationStructures[index], "vkCmdWriteAccelerationStructuresPropertiesNV");
        }
    }
    FinishReadObject(queryPool, "vkCmdWriteAccelerationStructuresPropertiesNV");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCompileDeferredNV(
    VkDevice                                    device,
    VkPipeline                                  pipeline,
    uint32_t                                    shader) {
    StartReadObjectParentInstance(device, "vkCompileDeferredNV");
    StartReadObject(pipeline, "vkCompileDeferredNV");
}

void ThreadSafety::PostCallRecordCompileDeferredNV(
    VkDevice                                    device,
    VkPipeline                                  pipeline,
    uint32_t                                    shader,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkCompileDeferredNV");
    FinishReadObject(pipeline, "vkCompileDeferredNV");
}

void ThreadSafety::PreCallRecordGetMemoryHostPointerPropertiesEXT(
    VkDevice                                    device,
    VkExternalMemoryHandleTypeFlagBits          handleType,
    const void*                                 pHostPointer,
    VkMemoryHostPointerPropertiesEXT*           pMemoryHostPointerProperties) {
    StartReadObjectParentInstance(device, "vkGetMemoryHostPointerPropertiesEXT");
}

void ThreadSafety::PostCallRecordGetMemoryHostPointerPropertiesEXT(
    VkDevice                                    device,
    VkExternalMemoryHandleTypeFlagBits          handleType,
    const void*                                 pHostPointer,
    VkMemoryHostPointerPropertiesEXT*           pMemoryHostPointerProperties,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkGetMemoryHostPointerPropertiesEXT");
}

void ThreadSafety::PreCallRecordCmdWriteBufferMarkerAMD(
    VkCommandBuffer                             commandBuffer,
    VkPipelineStageFlagBits                     pipelineStage,
    VkBuffer                                    dstBuffer,
    VkDeviceSize                                dstOffset,
    uint32_t                                    marker) {
    StartWriteObject(commandBuffer, "vkCmdWriteBufferMarkerAMD");
    StartReadObject(dstBuffer, "vkCmdWriteBufferMarkerAMD");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdWriteBufferMarkerAMD(
    VkCommandBuffer                             commandBuffer,
    VkPipelineStageFlagBits                     pipelineStage,
    VkBuffer                                    dstBuffer,
    VkDeviceSize                                dstOffset,
    uint32_t                                    marker) {
    FinishWriteObject(commandBuffer, "vkCmdWriteBufferMarkerAMD");
    FinishReadObject(dstBuffer, "vkCmdWriteBufferMarkerAMD");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordGetCalibratedTimestampsEXT(
    VkDevice                                    device,
    uint32_t                                    timestampCount,
    const VkCalibratedTimestampInfoEXT*         pTimestampInfos,
    uint64_t*                                   pTimestamps,
    uint64_t*                                   pMaxDeviation) {
    StartReadObjectParentInstance(device, "vkGetCalibratedTimestampsEXT");
}

void ThreadSafety::PostCallRecordGetCalibratedTimestampsEXT(
    VkDevice                                    device,
    uint32_t                                    timestampCount,
    const VkCalibratedTimestampInfoEXT*         pTimestampInfos,
    uint64_t*                                   pTimestamps,
    uint64_t*                                   pMaxDeviation,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkGetCalibratedTimestampsEXT");
}

#ifdef VK_USE_PLATFORM_GGP
#endif // VK_USE_PLATFORM_GGP

void ThreadSafety::PreCallRecordCmdDrawMeshTasksNV(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    taskCount,
    uint32_t                                    firstTask) {
    StartWriteObject(commandBuffer, "vkCmdDrawMeshTasksNV");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdDrawMeshTasksNV(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    taskCount,
    uint32_t                                    firstTask) {
    FinishWriteObject(commandBuffer, "vkCmdDrawMeshTasksNV");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdDrawMeshTasksIndirectNV(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    uint32_t                                    drawCount,
    uint32_t                                    stride) {
    StartWriteObject(commandBuffer, "vkCmdDrawMeshTasksIndirectNV");
    StartReadObject(buffer, "vkCmdDrawMeshTasksIndirectNV");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdDrawMeshTasksIndirectNV(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    uint32_t                                    drawCount,
    uint32_t                                    stride) {
    FinishWriteObject(commandBuffer, "vkCmdDrawMeshTasksIndirectNV");
    FinishReadObject(buffer, "vkCmdDrawMeshTasksIndirectNV");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdDrawMeshTasksIndirectCountNV(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkBuffer                                    countBuffer,
    VkDeviceSize                                countBufferOffset,
    uint32_t                                    maxDrawCount,
    uint32_t                                    stride) {
    StartWriteObject(commandBuffer, "vkCmdDrawMeshTasksIndirectCountNV");
    StartReadObject(buffer, "vkCmdDrawMeshTasksIndirectCountNV");
    StartReadObject(countBuffer, "vkCmdDrawMeshTasksIndirectCountNV");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdDrawMeshTasksIndirectCountNV(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkBuffer                                    countBuffer,
    VkDeviceSize                                countBufferOffset,
    uint32_t                                    maxDrawCount,
    uint32_t                                    stride) {
    FinishWriteObject(commandBuffer, "vkCmdDrawMeshTasksIndirectCountNV");
    FinishReadObject(buffer, "vkCmdDrawMeshTasksIndirectCountNV");
    FinishReadObject(countBuffer, "vkCmdDrawMeshTasksIndirectCountNV");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetExclusiveScissorNV(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstExclusiveScissor,
    uint32_t                                    exclusiveScissorCount,
    const VkRect2D*                             pExclusiveScissors) {
    StartWriteObject(commandBuffer, "vkCmdSetExclusiveScissorNV");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetExclusiveScissorNV(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstExclusiveScissor,
    uint32_t                                    exclusiveScissorCount,
    const VkRect2D*                             pExclusiveScissors) {
    FinishWriteObject(commandBuffer, "vkCmdSetExclusiveScissorNV");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetCheckpointNV(
    VkCommandBuffer                             commandBuffer,
    const void*                                 pCheckpointMarker) {
    StartWriteObject(commandBuffer, "vkCmdSetCheckpointNV");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetCheckpointNV(
    VkCommandBuffer                             commandBuffer,
    const void*                                 pCheckpointMarker) {
    FinishWriteObject(commandBuffer, "vkCmdSetCheckpointNV");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordGetQueueCheckpointDataNV(
    VkQueue                                     queue,
    uint32_t*                                   pCheckpointDataCount,
    VkCheckpointDataNV*                         pCheckpointData) {
    StartReadObject(queue, "vkGetQueueCheckpointDataNV");
}

void ThreadSafety::PostCallRecordGetQueueCheckpointDataNV(
    VkQueue                                     queue,
    uint32_t*                                   pCheckpointDataCount,
    VkCheckpointDataNV*                         pCheckpointData) {
    FinishReadObject(queue, "vkGetQueueCheckpointDataNV");
}

void ThreadSafety::PreCallRecordInitializePerformanceApiINTEL(
    VkDevice                                    device,
    const VkInitializePerformanceApiInfoINTEL*  pInitializeInfo) {
    StartReadObjectParentInstance(device, "vkInitializePerformanceApiINTEL");
}

void ThreadSafety::PostCallRecordInitializePerformanceApiINTEL(
    VkDevice                                    device,
    const VkInitializePerformanceApiInfoINTEL*  pInitializeInfo,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkInitializePerformanceApiINTEL");
}

void ThreadSafety::PreCallRecordUninitializePerformanceApiINTEL(
    VkDevice                                    device) {
    StartReadObjectParentInstance(device, "vkUninitializePerformanceApiINTEL");
}

void ThreadSafety::PostCallRecordUninitializePerformanceApiINTEL(
    VkDevice                                    device) {
    FinishReadObjectParentInstance(device, "vkUninitializePerformanceApiINTEL");
}

void ThreadSafety::PreCallRecordCmdSetPerformanceMarkerINTEL(
    VkCommandBuffer                             commandBuffer,
    const VkPerformanceMarkerInfoINTEL*         pMarkerInfo) {
    StartWriteObject(commandBuffer, "vkCmdSetPerformanceMarkerINTEL");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetPerformanceMarkerINTEL(
    VkCommandBuffer                             commandBuffer,
    const VkPerformanceMarkerInfoINTEL*         pMarkerInfo,
    VkResult                                    result) {
    FinishWriteObject(commandBuffer, "vkCmdSetPerformanceMarkerINTEL");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetPerformanceStreamMarkerINTEL(
    VkCommandBuffer                             commandBuffer,
    const VkPerformanceStreamMarkerInfoINTEL*   pMarkerInfo) {
    StartWriteObject(commandBuffer, "vkCmdSetPerformanceStreamMarkerINTEL");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetPerformanceStreamMarkerINTEL(
    VkCommandBuffer                             commandBuffer,
    const VkPerformanceStreamMarkerInfoINTEL*   pMarkerInfo,
    VkResult                                    result) {
    FinishWriteObject(commandBuffer, "vkCmdSetPerformanceStreamMarkerINTEL");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetPerformanceOverrideINTEL(
    VkCommandBuffer                             commandBuffer,
    const VkPerformanceOverrideInfoINTEL*       pOverrideInfo) {
    StartWriteObject(commandBuffer, "vkCmdSetPerformanceOverrideINTEL");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetPerformanceOverrideINTEL(
    VkCommandBuffer                             commandBuffer,
    const VkPerformanceOverrideInfoINTEL*       pOverrideInfo,
    VkResult                                    result) {
    FinishWriteObject(commandBuffer, "vkCmdSetPerformanceOverrideINTEL");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordAcquirePerformanceConfigurationINTEL(
    VkDevice                                    device,
    const VkPerformanceConfigurationAcquireInfoINTEL* pAcquireInfo,
    VkPerformanceConfigurationINTEL*            pConfiguration) {
    StartReadObjectParentInstance(device, "vkAcquirePerformanceConfigurationINTEL");
}

void ThreadSafety::PostCallRecordAcquirePerformanceConfigurationINTEL(
    VkDevice                                    device,
    const VkPerformanceConfigurationAcquireInfoINTEL* pAcquireInfo,
    VkPerformanceConfigurationINTEL*            pConfiguration,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkAcquirePerformanceConfigurationINTEL");
    if (result == VK_SUCCESS) {
        CreateObject(*pConfiguration);
    }
}

void ThreadSafety::PreCallRecordReleasePerformanceConfigurationINTEL(
    VkDevice                                    device,
    VkPerformanceConfigurationINTEL             configuration) {
    StartReadObjectParentInstance(device, "vkReleasePerformanceConfigurationINTEL");
    StartWriteObject(configuration, "vkReleasePerformanceConfigurationINTEL");
    // Host access to configuration must be externally synchronized
}

void ThreadSafety::PostCallRecordReleasePerformanceConfigurationINTEL(
    VkDevice                                    device,
    VkPerformanceConfigurationINTEL             configuration,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkReleasePerformanceConfigurationINTEL");
    FinishWriteObject(configuration, "vkReleasePerformanceConfigurationINTEL");
    DestroyObject(configuration);
    // Host access to configuration must be externally synchronized
}

void ThreadSafety::PreCallRecordQueueSetPerformanceConfigurationINTEL(
    VkQueue                                     queue,
    VkPerformanceConfigurationINTEL             configuration) {
    StartReadObject(queue, "vkQueueSetPerformanceConfigurationINTEL");
    StartReadObject(configuration, "vkQueueSetPerformanceConfigurationINTEL");
}

void ThreadSafety::PostCallRecordQueueSetPerformanceConfigurationINTEL(
    VkQueue                                     queue,
    VkPerformanceConfigurationINTEL             configuration,
    VkResult                                    result) {
    FinishReadObject(queue, "vkQueueSetPerformanceConfigurationINTEL");
    FinishReadObject(configuration, "vkQueueSetPerformanceConfigurationINTEL");
}

void ThreadSafety::PreCallRecordGetPerformanceParameterINTEL(
    VkDevice                                    device,
    VkPerformanceParameterTypeINTEL             parameter,
    VkPerformanceValueINTEL*                    pValue) {
    StartReadObjectParentInstance(device, "vkGetPerformanceParameterINTEL");
}

void ThreadSafety::PostCallRecordGetPerformanceParameterINTEL(
    VkDevice                                    device,
    VkPerformanceParameterTypeINTEL             parameter,
    VkPerformanceValueINTEL*                    pValue,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkGetPerformanceParameterINTEL");
}

void ThreadSafety::PreCallRecordSetLocalDimmingAMD(
    VkDevice                                    device,
    VkSwapchainKHR                              swapChain,
    VkBool32                                    localDimmingEnable) {
    StartReadObjectParentInstance(device, "vkSetLocalDimmingAMD");
    StartReadObjectParentInstance(swapChain, "vkSetLocalDimmingAMD");
}

void ThreadSafety::PostCallRecordSetLocalDimmingAMD(
    VkDevice                                    device,
    VkSwapchainKHR                              swapChain,
    VkBool32                                    localDimmingEnable) {
    FinishReadObjectParentInstance(device, "vkSetLocalDimmingAMD");
    FinishReadObjectParentInstance(swapChain, "vkSetLocalDimmingAMD");
}

#ifdef VK_USE_PLATFORM_FUCHSIA

void ThreadSafety::PreCallRecordCreateImagePipeSurfaceFUCHSIA(
    VkInstance                                  instance,
    const VkImagePipeSurfaceCreateInfoFUCHSIA*  pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) {
    StartReadObjectParentInstance(instance, "vkCreateImagePipeSurfaceFUCHSIA");
}

void ThreadSafety::PostCallRecordCreateImagePipeSurfaceFUCHSIA(
    VkInstance                                  instance,
    const VkImagePipeSurfaceCreateInfoFUCHSIA*  pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface,
    VkResult                                    result) {
    FinishReadObjectParentInstance(instance, "vkCreateImagePipeSurfaceFUCHSIA");
    if (result == VK_SUCCESS) {
        CreateObjectParentInstance(*pSurface);
    }
}
#endif // VK_USE_PLATFORM_FUCHSIA

#ifdef VK_USE_PLATFORM_METAL_EXT

void ThreadSafety::PreCallRecordCreateMetalSurfaceEXT(
    VkInstance                                  instance,
    const VkMetalSurfaceCreateInfoEXT*          pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) {
    StartReadObjectParentInstance(instance, "vkCreateMetalSurfaceEXT");
}

void ThreadSafety::PostCallRecordCreateMetalSurfaceEXT(
    VkInstance                                  instance,
    const VkMetalSurfaceCreateInfoEXT*          pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface,
    VkResult                                    result) {
    FinishReadObjectParentInstance(instance, "vkCreateMetalSurfaceEXT");
    if (result == VK_SUCCESS) {
        CreateObjectParentInstance(*pSurface);
    }
}
#endif // VK_USE_PLATFORM_METAL_EXT

void ThreadSafety::PreCallRecordGetBufferDeviceAddressEXT(
    VkDevice                                    device,
    const VkBufferDeviceAddressInfo*            pInfo) {
    StartReadObjectParentInstance(device, "vkGetBufferDeviceAddressEXT");
}

void ThreadSafety::PostCallRecordGetBufferDeviceAddressEXT(
    VkDevice                                    device,
    const VkBufferDeviceAddressInfo*            pInfo,
    VkDeviceAddress                             result) {
    FinishReadObjectParentInstance(device, "vkGetBufferDeviceAddressEXT");
}

#ifdef VK_USE_PLATFORM_WIN32_KHR

void ThreadSafety::PreCallRecordAcquireFullScreenExclusiveModeEXT(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain) {
    StartReadObjectParentInstance(device, "vkAcquireFullScreenExclusiveModeEXT");
    StartReadObjectParentInstance(swapchain, "vkAcquireFullScreenExclusiveModeEXT");
}

void ThreadSafety::PostCallRecordAcquireFullScreenExclusiveModeEXT(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkAcquireFullScreenExclusiveModeEXT");
    FinishReadObjectParentInstance(swapchain, "vkAcquireFullScreenExclusiveModeEXT");
}

void ThreadSafety::PreCallRecordReleaseFullScreenExclusiveModeEXT(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain) {
    StartReadObjectParentInstance(device, "vkReleaseFullScreenExclusiveModeEXT");
    StartReadObjectParentInstance(swapchain, "vkReleaseFullScreenExclusiveModeEXT");
}

void ThreadSafety::PostCallRecordReleaseFullScreenExclusiveModeEXT(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkReleaseFullScreenExclusiveModeEXT");
    FinishReadObjectParentInstance(swapchain, "vkReleaseFullScreenExclusiveModeEXT");
}

void ThreadSafety::PreCallRecordGetDeviceGroupSurfacePresentModes2EXT(
    VkDevice                                    device,
    const VkPhysicalDeviceSurfaceInfo2KHR*      pSurfaceInfo,
    VkDeviceGroupPresentModeFlagsKHR*           pModes) {
    StartReadObjectParentInstance(device, "vkGetDeviceGroupSurfacePresentModes2EXT");
}

void ThreadSafety::PostCallRecordGetDeviceGroupSurfacePresentModes2EXT(
    VkDevice                                    device,
    const VkPhysicalDeviceSurfaceInfo2KHR*      pSurfaceInfo,
    VkDeviceGroupPresentModeFlagsKHR*           pModes,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkGetDeviceGroupSurfacePresentModes2EXT");
}
#endif // VK_USE_PLATFORM_WIN32_KHR

void ThreadSafety::PreCallRecordCreateHeadlessSurfaceEXT(
    VkInstance                                  instance,
    const VkHeadlessSurfaceCreateInfoEXT*       pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) {
    StartReadObjectParentInstance(instance, "vkCreateHeadlessSurfaceEXT");
}

void ThreadSafety::PostCallRecordCreateHeadlessSurfaceEXT(
    VkInstance                                  instance,
    const VkHeadlessSurfaceCreateInfoEXT*       pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface,
    VkResult                                    result) {
    FinishReadObjectParentInstance(instance, "vkCreateHeadlessSurfaceEXT");
    if (result == VK_SUCCESS) {
        CreateObjectParentInstance(*pSurface);
    }
}

void ThreadSafety::PreCallRecordCmdSetLineStippleEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    lineStippleFactor,
    uint16_t                                    lineStipplePattern) {
    StartWriteObject(commandBuffer, "vkCmdSetLineStippleEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetLineStippleEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    lineStippleFactor,
    uint16_t                                    lineStipplePattern) {
    FinishWriteObject(commandBuffer, "vkCmdSetLineStippleEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordResetQueryPoolEXT(
    VkDevice                                    device,
    VkQueryPool                                 queryPool,
    uint32_t                                    firstQuery,
    uint32_t                                    queryCount) {
    StartReadObjectParentInstance(device, "vkResetQueryPoolEXT");
    StartReadObject(queryPool, "vkResetQueryPoolEXT");
}

void ThreadSafety::PostCallRecordResetQueryPoolEXT(
    VkDevice                                    device,
    VkQueryPool                                 queryPool,
    uint32_t                                    firstQuery,
    uint32_t                                    queryCount) {
    FinishReadObjectParentInstance(device, "vkResetQueryPoolEXT");
    FinishReadObject(queryPool, "vkResetQueryPoolEXT");
}

void ThreadSafety::PreCallRecordCmdSetCullModeEXT(
    VkCommandBuffer                             commandBuffer,
    VkCullModeFlags                             cullMode) {
    StartWriteObject(commandBuffer, "vkCmdSetCullModeEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetCullModeEXT(
    VkCommandBuffer                             commandBuffer,
    VkCullModeFlags                             cullMode) {
    FinishWriteObject(commandBuffer, "vkCmdSetCullModeEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetFrontFaceEXT(
    VkCommandBuffer                             commandBuffer,
    VkFrontFace                                 frontFace) {
    StartWriteObject(commandBuffer, "vkCmdSetFrontFaceEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetFrontFaceEXT(
    VkCommandBuffer                             commandBuffer,
    VkFrontFace                                 frontFace) {
    FinishWriteObject(commandBuffer, "vkCmdSetFrontFaceEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetPrimitiveTopologyEXT(
    VkCommandBuffer                             commandBuffer,
    VkPrimitiveTopology                         primitiveTopology) {
    StartWriteObject(commandBuffer, "vkCmdSetPrimitiveTopologyEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetPrimitiveTopologyEXT(
    VkCommandBuffer                             commandBuffer,
    VkPrimitiveTopology                         primitiveTopology) {
    FinishWriteObject(commandBuffer, "vkCmdSetPrimitiveTopologyEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetViewportWithCountEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    viewportCount,
    const VkViewport*                           pViewports) {
    StartWriteObject(commandBuffer, "vkCmdSetViewportWithCountEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetViewportWithCountEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    viewportCount,
    const VkViewport*                           pViewports) {
    FinishWriteObject(commandBuffer, "vkCmdSetViewportWithCountEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetScissorWithCountEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    scissorCount,
    const VkRect2D*                             pScissors) {
    StartWriteObject(commandBuffer, "vkCmdSetScissorWithCountEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetScissorWithCountEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    scissorCount,
    const VkRect2D*                             pScissors) {
    FinishWriteObject(commandBuffer, "vkCmdSetScissorWithCountEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdBindVertexBuffers2EXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstBinding,
    uint32_t                                    bindingCount,
    const VkBuffer*                             pBuffers,
    const VkDeviceSize*                         pOffsets,
    const VkDeviceSize*                         pSizes,
    const VkDeviceSize*                         pStrides) {
    StartWriteObject(commandBuffer, "vkCmdBindVertexBuffers2EXT");
    if (pBuffers) {
        for (uint32_t index = 0; index < bindingCount; index++) {
            StartReadObject(pBuffers[index], "vkCmdBindVertexBuffers2EXT");
        }
    }
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdBindVertexBuffers2EXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstBinding,
    uint32_t                                    bindingCount,
    const VkBuffer*                             pBuffers,
    const VkDeviceSize*                         pOffsets,
    const VkDeviceSize*                         pSizes,
    const VkDeviceSize*                         pStrides) {
    FinishWriteObject(commandBuffer, "vkCmdBindVertexBuffers2EXT");
    if (pBuffers) {
        for (uint32_t index = 0; index < bindingCount; index++) {
            FinishReadObject(pBuffers[index], "vkCmdBindVertexBuffers2EXT");
        }
    }
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetDepthTestEnableEXT(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    depthTestEnable) {
    StartWriteObject(commandBuffer, "vkCmdSetDepthTestEnableEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetDepthTestEnableEXT(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    depthTestEnable) {
    FinishWriteObject(commandBuffer, "vkCmdSetDepthTestEnableEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetDepthWriteEnableEXT(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    depthWriteEnable) {
    StartWriteObject(commandBuffer, "vkCmdSetDepthWriteEnableEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetDepthWriteEnableEXT(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    depthWriteEnable) {
    FinishWriteObject(commandBuffer, "vkCmdSetDepthWriteEnableEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetDepthCompareOpEXT(
    VkCommandBuffer                             commandBuffer,
    VkCompareOp                                 depthCompareOp) {
    StartWriteObject(commandBuffer, "vkCmdSetDepthCompareOpEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetDepthCompareOpEXT(
    VkCommandBuffer                             commandBuffer,
    VkCompareOp                                 depthCompareOp) {
    FinishWriteObject(commandBuffer, "vkCmdSetDepthCompareOpEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetDepthBoundsTestEnableEXT(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    depthBoundsTestEnable) {
    StartWriteObject(commandBuffer, "vkCmdSetDepthBoundsTestEnableEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetDepthBoundsTestEnableEXT(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    depthBoundsTestEnable) {
    FinishWriteObject(commandBuffer, "vkCmdSetDepthBoundsTestEnableEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetStencilTestEnableEXT(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    stencilTestEnable) {
    StartWriteObject(commandBuffer, "vkCmdSetStencilTestEnableEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetStencilTestEnableEXT(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    stencilTestEnable) {
    FinishWriteObject(commandBuffer, "vkCmdSetStencilTestEnableEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetStencilOpEXT(
    VkCommandBuffer                             commandBuffer,
    VkStencilFaceFlags                          faceMask,
    VkStencilOp                                 failOp,
    VkStencilOp                                 passOp,
    VkStencilOp                                 depthFailOp,
    VkCompareOp                                 compareOp) {
    StartWriteObject(commandBuffer, "vkCmdSetStencilOpEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetStencilOpEXT(
    VkCommandBuffer                             commandBuffer,
    VkStencilFaceFlags                          faceMask,
    VkStencilOp                                 failOp,
    VkStencilOp                                 passOp,
    VkStencilOp                                 depthFailOp,
    VkCompareOp                                 compareOp) {
    FinishWriteObject(commandBuffer, "vkCmdSetStencilOpEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordReleaseSwapchainImagesEXT(
    VkDevice                                    device,
    const VkReleaseSwapchainImagesInfoEXT*      pReleaseInfo) {
    StartReadObjectParentInstance(device, "vkReleaseSwapchainImagesEXT");
}

void ThreadSafety::PostCallRecordReleaseSwapchainImagesEXT(
    VkDevice                                    device,
    const VkReleaseSwapchainImagesInfoEXT*      pReleaseInfo,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkReleaseSwapchainImagesEXT");
}

void ThreadSafety::PreCallRecordGetGeneratedCommandsMemoryRequirementsNV(
    VkDevice                                    device,
    const VkGeneratedCommandsMemoryRequirementsInfoNV* pInfo,
    VkMemoryRequirements2*                      pMemoryRequirements) {
    StartReadObjectParentInstance(device, "vkGetGeneratedCommandsMemoryRequirementsNV");
}

void ThreadSafety::PostCallRecordGetGeneratedCommandsMemoryRequirementsNV(
    VkDevice                                    device,
    const VkGeneratedCommandsMemoryRequirementsInfoNV* pInfo,
    VkMemoryRequirements2*                      pMemoryRequirements) {
    FinishReadObjectParentInstance(device, "vkGetGeneratedCommandsMemoryRequirementsNV");
}

void ThreadSafety::PreCallRecordCmdPreprocessGeneratedCommandsNV(
    VkCommandBuffer                             commandBuffer,
    const VkGeneratedCommandsInfoNV*            pGeneratedCommandsInfo) {
    StartWriteObject(commandBuffer, "vkCmdPreprocessGeneratedCommandsNV");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdPreprocessGeneratedCommandsNV(
    VkCommandBuffer                             commandBuffer,
    const VkGeneratedCommandsInfoNV*            pGeneratedCommandsInfo) {
    FinishWriteObject(commandBuffer, "vkCmdPreprocessGeneratedCommandsNV");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdExecuteGeneratedCommandsNV(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    isPreprocessed,
    const VkGeneratedCommandsInfoNV*            pGeneratedCommandsInfo) {
    StartWriteObject(commandBuffer, "vkCmdExecuteGeneratedCommandsNV");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdExecuteGeneratedCommandsNV(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    isPreprocessed,
    const VkGeneratedCommandsInfoNV*            pGeneratedCommandsInfo) {
    FinishWriteObject(commandBuffer, "vkCmdExecuteGeneratedCommandsNV");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdBindPipelineShaderGroupNV(
    VkCommandBuffer                             commandBuffer,
    VkPipelineBindPoint                         pipelineBindPoint,
    VkPipeline                                  pipeline,
    uint32_t                                    groupIndex) {
    StartWriteObject(commandBuffer, "vkCmdBindPipelineShaderGroupNV");
    StartReadObject(pipeline, "vkCmdBindPipelineShaderGroupNV");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdBindPipelineShaderGroupNV(
    VkCommandBuffer                             commandBuffer,
    VkPipelineBindPoint                         pipelineBindPoint,
    VkPipeline                                  pipeline,
    uint32_t                                    groupIndex) {
    FinishWriteObject(commandBuffer, "vkCmdBindPipelineShaderGroupNV");
    FinishReadObject(pipeline, "vkCmdBindPipelineShaderGroupNV");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCreateIndirectCommandsLayoutNV(
    VkDevice                                    device,
    const VkIndirectCommandsLayoutCreateInfoNV* pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkIndirectCommandsLayoutNV*                 pIndirectCommandsLayout) {
    StartReadObjectParentInstance(device, "vkCreateIndirectCommandsLayoutNV");
}

void ThreadSafety::PostCallRecordCreateIndirectCommandsLayoutNV(
    VkDevice                                    device,
    const VkIndirectCommandsLayoutCreateInfoNV* pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkIndirectCommandsLayoutNV*                 pIndirectCommandsLayout,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkCreateIndirectCommandsLayoutNV");
    if (result == VK_SUCCESS) {
        CreateObject(*pIndirectCommandsLayout);
    }
}

void ThreadSafety::PreCallRecordDestroyIndirectCommandsLayoutNV(
    VkDevice                                    device,
    VkIndirectCommandsLayoutNV                  indirectCommandsLayout,
    const VkAllocationCallbacks*                pAllocator) {
    StartReadObjectParentInstance(device, "vkDestroyIndirectCommandsLayoutNV");
    StartWriteObject(indirectCommandsLayout, "vkDestroyIndirectCommandsLayoutNV");
    // Host access to indirectCommandsLayout must be externally synchronized
}

void ThreadSafety::PostCallRecordDestroyIndirectCommandsLayoutNV(
    VkDevice                                    device,
    VkIndirectCommandsLayoutNV                  indirectCommandsLayout,
    const VkAllocationCallbacks*                pAllocator) {
    FinishReadObjectParentInstance(device, "vkDestroyIndirectCommandsLayoutNV");
    FinishWriteObject(indirectCommandsLayout, "vkDestroyIndirectCommandsLayoutNV");
    DestroyObject(indirectCommandsLayout);
    // Host access to indirectCommandsLayout must be externally synchronized
}

void ThreadSafety::PreCallRecordAcquireDrmDisplayEXT(
    VkPhysicalDevice                            physicalDevice,
    int32_t                                     drmFd,
    VkDisplayKHR                                display) {
    StartReadObjectParentInstance(display, "vkAcquireDrmDisplayEXT");
}

void ThreadSafety::PostCallRecordAcquireDrmDisplayEXT(
    VkPhysicalDevice                            physicalDevice,
    int32_t                                     drmFd,
    VkDisplayKHR                                display,
    VkResult                                    result) {
    FinishReadObjectParentInstance(display, "vkAcquireDrmDisplayEXT");
}

void ThreadSafety::PreCallRecordCreatePrivateDataSlotEXT(
    VkDevice                                    device,
    const VkPrivateDataSlotCreateInfo*          pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkPrivateDataSlot*                          pPrivateDataSlot) {
    StartReadObjectParentInstance(device, "vkCreatePrivateDataSlotEXT");
}

void ThreadSafety::PostCallRecordCreatePrivateDataSlotEXT(
    VkDevice                                    device,
    const VkPrivateDataSlotCreateInfo*          pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkPrivateDataSlot*                          pPrivateDataSlot,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkCreatePrivateDataSlotEXT");
    if (result == VK_SUCCESS) {
        CreateObject(*pPrivateDataSlot);
    }
}

void ThreadSafety::PreCallRecordDestroyPrivateDataSlotEXT(
    VkDevice                                    device,
    VkPrivateDataSlot                           privateDataSlot,
    const VkAllocationCallbacks*                pAllocator) {
    StartReadObjectParentInstance(device, "vkDestroyPrivateDataSlotEXT");
    StartWriteObject(privateDataSlot, "vkDestroyPrivateDataSlotEXT");
    // Host access to privateDataSlot must be externally synchronized
}

void ThreadSafety::PostCallRecordDestroyPrivateDataSlotEXT(
    VkDevice                                    device,
    VkPrivateDataSlot                           privateDataSlot,
    const VkAllocationCallbacks*                pAllocator) {
    FinishReadObjectParentInstance(device, "vkDestroyPrivateDataSlotEXT");
    FinishWriteObject(privateDataSlot, "vkDestroyPrivateDataSlotEXT");
    DestroyObject(privateDataSlot);
    // Host access to privateDataSlot must be externally synchronized
}

void ThreadSafety::PreCallRecordSetPrivateDataEXT(
    VkDevice                                    device,
    VkObjectType                                objectType,
    uint64_t                                    objectHandle,
    VkPrivateDataSlot                           privateDataSlot,
    uint64_t                                    data) {
    StartReadObjectParentInstance(device, "vkSetPrivateDataEXT");
    StartReadObject(privateDataSlot, "vkSetPrivateDataEXT");
}

void ThreadSafety::PostCallRecordSetPrivateDataEXT(
    VkDevice                                    device,
    VkObjectType                                objectType,
    uint64_t                                    objectHandle,
    VkPrivateDataSlot                           privateDataSlot,
    uint64_t                                    data,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkSetPrivateDataEXT");
    FinishReadObject(privateDataSlot, "vkSetPrivateDataEXT");
}

void ThreadSafety::PreCallRecordGetPrivateDataEXT(
    VkDevice                                    device,
    VkObjectType                                objectType,
    uint64_t                                    objectHandle,
    VkPrivateDataSlot                           privateDataSlot,
    uint64_t*                                   pData) {
    StartReadObjectParentInstance(device, "vkGetPrivateDataEXT");
    StartReadObject(privateDataSlot, "vkGetPrivateDataEXT");
}

void ThreadSafety::PostCallRecordGetPrivateDataEXT(
    VkDevice                                    device,
    VkObjectType                                objectType,
    uint64_t                                    objectHandle,
    VkPrivateDataSlot                           privateDataSlot,
    uint64_t*                                   pData) {
    FinishReadObjectParentInstance(device, "vkGetPrivateDataEXT");
    FinishReadObject(privateDataSlot, "vkGetPrivateDataEXT");
}

#ifdef VK_USE_PLATFORM_METAL_EXT

void ThreadSafety::PreCallRecordExportMetalObjectsEXT(
    VkDevice                                    device,
    VkExportMetalObjectsInfoEXT*                pMetalObjectsInfo) {
    StartReadObjectParentInstance(device, "vkExportMetalObjectsEXT");
}

void ThreadSafety::PostCallRecordExportMetalObjectsEXT(
    VkDevice                                    device,
    VkExportMetalObjectsInfoEXT*                pMetalObjectsInfo) {
    FinishReadObjectParentInstance(device, "vkExportMetalObjectsEXT");
}
#endif // VK_USE_PLATFORM_METAL_EXT

void ThreadSafety::PreCallRecordGetDescriptorSetLayoutSizeEXT(
    VkDevice                                    device,
    VkDescriptorSetLayout                       layout,
    VkDeviceSize*                               pLayoutSizeInBytes) {
    StartReadObjectParentInstance(device, "vkGetDescriptorSetLayoutSizeEXT");
    StartReadObject(layout, "vkGetDescriptorSetLayoutSizeEXT");
}

void ThreadSafety::PostCallRecordGetDescriptorSetLayoutSizeEXT(
    VkDevice                                    device,
    VkDescriptorSetLayout                       layout,
    VkDeviceSize*                               pLayoutSizeInBytes) {
    FinishReadObjectParentInstance(device, "vkGetDescriptorSetLayoutSizeEXT");
    FinishReadObject(layout, "vkGetDescriptorSetLayoutSizeEXT");
}

void ThreadSafety::PreCallRecordGetDescriptorSetLayoutBindingOffsetEXT(
    VkDevice                                    device,
    VkDescriptorSetLayout                       layout,
    uint32_t                                    binding,
    VkDeviceSize*                               pOffset) {
    StartReadObjectParentInstance(device, "vkGetDescriptorSetLayoutBindingOffsetEXT");
    StartReadObject(layout, "vkGetDescriptorSetLayoutBindingOffsetEXT");
}

void ThreadSafety::PostCallRecordGetDescriptorSetLayoutBindingOffsetEXT(
    VkDevice                                    device,
    VkDescriptorSetLayout                       layout,
    uint32_t                                    binding,
    VkDeviceSize*                               pOffset) {
    FinishReadObjectParentInstance(device, "vkGetDescriptorSetLayoutBindingOffsetEXT");
    FinishReadObject(layout, "vkGetDescriptorSetLayoutBindingOffsetEXT");
}

void ThreadSafety::PreCallRecordGetDescriptorEXT(
    VkDevice                                    device,
    const VkDescriptorGetInfoEXT*               pDescriptorInfo,
    size_t                                      dataSize,
    void*                                       pDescriptor) {
    StartReadObjectParentInstance(device, "vkGetDescriptorEXT");
}

void ThreadSafety::PostCallRecordGetDescriptorEXT(
    VkDevice                                    device,
    const VkDescriptorGetInfoEXT*               pDescriptorInfo,
    size_t                                      dataSize,
    void*                                       pDescriptor) {
    FinishReadObjectParentInstance(device, "vkGetDescriptorEXT");
}

void ThreadSafety::PreCallRecordCmdBindDescriptorBuffersEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    bufferCount,
    const VkDescriptorBufferBindingInfoEXT*     pBindingInfos) {
    StartWriteObject(commandBuffer, "vkCmdBindDescriptorBuffersEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdBindDescriptorBuffersEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    bufferCount,
    const VkDescriptorBufferBindingInfoEXT*     pBindingInfos) {
    FinishWriteObject(commandBuffer, "vkCmdBindDescriptorBuffersEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetDescriptorBufferOffsetsEXT(
    VkCommandBuffer                             commandBuffer,
    VkPipelineBindPoint                         pipelineBindPoint,
    VkPipelineLayout                            layout,
    uint32_t                                    firstSet,
    uint32_t                                    setCount,
    const uint32_t*                             pBufferIndices,
    const VkDeviceSize*                         pOffsets) {
    StartWriteObject(commandBuffer, "vkCmdSetDescriptorBufferOffsetsEXT");
    StartReadObject(layout, "vkCmdSetDescriptorBufferOffsetsEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetDescriptorBufferOffsetsEXT(
    VkCommandBuffer                             commandBuffer,
    VkPipelineBindPoint                         pipelineBindPoint,
    VkPipelineLayout                            layout,
    uint32_t                                    firstSet,
    uint32_t                                    setCount,
    const uint32_t*                             pBufferIndices,
    const VkDeviceSize*                         pOffsets) {
    FinishWriteObject(commandBuffer, "vkCmdSetDescriptorBufferOffsetsEXT");
    FinishReadObject(layout, "vkCmdSetDescriptorBufferOffsetsEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdBindDescriptorBufferEmbeddedSamplersEXT(
    VkCommandBuffer                             commandBuffer,
    VkPipelineBindPoint                         pipelineBindPoint,
    VkPipelineLayout                            layout,
    uint32_t                                    set) {
    StartWriteObject(commandBuffer, "vkCmdBindDescriptorBufferEmbeddedSamplersEXT");
    StartReadObject(layout, "vkCmdBindDescriptorBufferEmbeddedSamplersEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdBindDescriptorBufferEmbeddedSamplersEXT(
    VkCommandBuffer                             commandBuffer,
    VkPipelineBindPoint                         pipelineBindPoint,
    VkPipelineLayout                            layout,
    uint32_t                                    set) {
    FinishWriteObject(commandBuffer, "vkCmdBindDescriptorBufferEmbeddedSamplersEXT");
    FinishReadObject(layout, "vkCmdBindDescriptorBufferEmbeddedSamplersEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordGetBufferOpaqueCaptureDescriptorDataEXT(
    VkDevice                                    device,
    const VkBufferCaptureDescriptorDataInfoEXT* pInfo,
    void*                                       pData) {
    StartReadObjectParentInstance(device, "vkGetBufferOpaqueCaptureDescriptorDataEXT");
}

void ThreadSafety::PostCallRecordGetBufferOpaqueCaptureDescriptorDataEXT(
    VkDevice                                    device,
    const VkBufferCaptureDescriptorDataInfoEXT* pInfo,
    void*                                       pData,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkGetBufferOpaqueCaptureDescriptorDataEXT");
}

void ThreadSafety::PreCallRecordGetImageOpaqueCaptureDescriptorDataEXT(
    VkDevice                                    device,
    const VkImageCaptureDescriptorDataInfoEXT*  pInfo,
    void*                                       pData) {
    StartReadObjectParentInstance(device, "vkGetImageOpaqueCaptureDescriptorDataEXT");
}

void ThreadSafety::PostCallRecordGetImageOpaqueCaptureDescriptorDataEXT(
    VkDevice                                    device,
    const VkImageCaptureDescriptorDataInfoEXT*  pInfo,
    void*                                       pData,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkGetImageOpaqueCaptureDescriptorDataEXT");
}

void ThreadSafety::PreCallRecordGetImageViewOpaqueCaptureDescriptorDataEXT(
    VkDevice                                    device,
    const VkImageViewCaptureDescriptorDataInfoEXT* pInfo,
    void*                                       pData) {
    StartReadObjectParentInstance(device, "vkGetImageViewOpaqueCaptureDescriptorDataEXT");
}

void ThreadSafety::PostCallRecordGetImageViewOpaqueCaptureDescriptorDataEXT(
    VkDevice                                    device,
    const VkImageViewCaptureDescriptorDataInfoEXT* pInfo,
    void*                                       pData,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkGetImageViewOpaqueCaptureDescriptorDataEXT");
}

void ThreadSafety::PreCallRecordGetSamplerOpaqueCaptureDescriptorDataEXT(
    VkDevice                                    device,
    const VkSamplerCaptureDescriptorDataInfoEXT* pInfo,
    void*                                       pData) {
    StartReadObjectParentInstance(device, "vkGetSamplerOpaqueCaptureDescriptorDataEXT");
}

void ThreadSafety::PostCallRecordGetSamplerOpaqueCaptureDescriptorDataEXT(
    VkDevice                                    device,
    const VkSamplerCaptureDescriptorDataInfoEXT* pInfo,
    void*                                       pData,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkGetSamplerOpaqueCaptureDescriptorDataEXT");
}

void ThreadSafety::PreCallRecordGetAccelerationStructureOpaqueCaptureDescriptorDataEXT(
    VkDevice                                    device,
    const VkAccelerationStructureCaptureDescriptorDataInfoEXT* pInfo,
    void*                                       pData) {
    StartReadObjectParentInstance(device, "vkGetAccelerationStructureOpaqueCaptureDescriptorDataEXT");
}

void ThreadSafety::PostCallRecordGetAccelerationStructureOpaqueCaptureDescriptorDataEXT(
    VkDevice                                    device,
    const VkAccelerationStructureCaptureDescriptorDataInfoEXT* pInfo,
    void*                                       pData,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkGetAccelerationStructureOpaqueCaptureDescriptorDataEXT");
}

void ThreadSafety::PreCallRecordCmdSetFragmentShadingRateEnumNV(
    VkCommandBuffer                             commandBuffer,
    VkFragmentShadingRateNV                     shadingRate,
    const VkFragmentShadingRateCombinerOpKHR    combinerOps[2]) {
    StartWriteObject(commandBuffer, "vkCmdSetFragmentShadingRateEnumNV");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetFragmentShadingRateEnumNV(
    VkCommandBuffer                             commandBuffer,
    VkFragmentShadingRateNV                     shadingRate,
    const VkFragmentShadingRateCombinerOpKHR    combinerOps[2]) {
    FinishWriteObject(commandBuffer, "vkCmdSetFragmentShadingRateEnumNV");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordGetImageSubresourceLayout2EXT(
    VkDevice                                    device,
    VkImage                                     image,
    const VkImageSubresource2EXT*               pSubresource,
    VkSubresourceLayout2EXT*                    pLayout) {
    StartReadObjectParentInstance(device, "vkGetImageSubresourceLayout2EXT");
    StartReadObject(image, "vkGetImageSubresourceLayout2EXT");
}

void ThreadSafety::PostCallRecordGetImageSubresourceLayout2EXT(
    VkDevice                                    device,
    VkImage                                     image,
    const VkImageSubresource2EXT*               pSubresource,
    VkSubresourceLayout2EXT*                    pLayout) {
    FinishReadObjectParentInstance(device, "vkGetImageSubresourceLayout2EXT");
    FinishReadObject(image, "vkGetImageSubresourceLayout2EXT");
}

void ThreadSafety::PreCallRecordGetDeviceFaultInfoEXT(
    VkDevice                                    device,
    VkDeviceFaultCountsEXT*                     pFaultCounts,
    VkDeviceFaultInfoEXT*                       pFaultInfo) {
    StartReadObjectParentInstance(device, "vkGetDeviceFaultInfoEXT");
}

void ThreadSafety::PostCallRecordGetDeviceFaultInfoEXT(
    VkDevice                                    device,
    VkDeviceFaultCountsEXT*                     pFaultCounts,
    VkDeviceFaultInfoEXT*                       pFaultInfo,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkGetDeviceFaultInfoEXT");
}

#ifdef VK_USE_PLATFORM_WIN32_KHR

void ThreadSafety::PreCallRecordAcquireWinrtDisplayNV(
    VkPhysicalDevice                            physicalDevice,
    VkDisplayKHR                                display) {
    StartReadObjectParentInstance(display, "vkAcquireWinrtDisplayNV");
}

void ThreadSafety::PostCallRecordAcquireWinrtDisplayNV(
    VkPhysicalDevice                            physicalDevice,
    VkDisplayKHR                                display,
    VkResult                                    result) {
    FinishReadObjectParentInstance(display, "vkAcquireWinrtDisplayNV");
}
#endif // VK_USE_PLATFORM_WIN32_KHR

#ifdef VK_USE_PLATFORM_DIRECTFB_EXT

void ThreadSafety::PreCallRecordCreateDirectFBSurfaceEXT(
    VkInstance                                  instance,
    const VkDirectFBSurfaceCreateInfoEXT*       pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) {
    StartReadObjectParentInstance(instance, "vkCreateDirectFBSurfaceEXT");
}

void ThreadSafety::PostCallRecordCreateDirectFBSurfaceEXT(
    VkInstance                                  instance,
    const VkDirectFBSurfaceCreateInfoEXT*       pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface,
    VkResult                                    result) {
    FinishReadObjectParentInstance(instance, "vkCreateDirectFBSurfaceEXT");
    if (result == VK_SUCCESS) {
        CreateObjectParentInstance(*pSurface);
    }
}
#endif // VK_USE_PLATFORM_DIRECTFB_EXT

void ThreadSafety::PreCallRecordCmdSetVertexInputEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    vertexBindingDescriptionCount,
    const VkVertexInputBindingDescription2EXT*  pVertexBindingDescriptions,
    uint32_t                                    vertexAttributeDescriptionCount,
    const VkVertexInputAttributeDescription2EXT* pVertexAttributeDescriptions) {
    StartWriteObject(commandBuffer, "vkCmdSetVertexInputEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetVertexInputEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    vertexBindingDescriptionCount,
    const VkVertexInputBindingDescription2EXT*  pVertexBindingDescriptions,
    uint32_t                                    vertexAttributeDescriptionCount,
    const VkVertexInputAttributeDescription2EXT* pVertexAttributeDescriptions) {
    FinishWriteObject(commandBuffer, "vkCmdSetVertexInputEXT");
    // Host access to commandBuffer must be externally synchronized
}

#ifdef VK_USE_PLATFORM_FUCHSIA

void ThreadSafety::PreCallRecordGetMemoryZirconHandleFUCHSIA(
    VkDevice                                    device,
    const VkMemoryGetZirconHandleInfoFUCHSIA*   pGetZirconHandleInfo,
    zx_handle_t*                                pZirconHandle) {
    StartReadObjectParentInstance(device, "vkGetMemoryZirconHandleFUCHSIA");
}

void ThreadSafety::PostCallRecordGetMemoryZirconHandleFUCHSIA(
    VkDevice                                    device,
    const VkMemoryGetZirconHandleInfoFUCHSIA*   pGetZirconHandleInfo,
    zx_handle_t*                                pZirconHandle,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkGetMemoryZirconHandleFUCHSIA");
}

void ThreadSafety::PreCallRecordGetMemoryZirconHandlePropertiesFUCHSIA(
    VkDevice                                    device,
    VkExternalMemoryHandleTypeFlagBits          handleType,
    zx_handle_t                                 zirconHandle,
    VkMemoryZirconHandlePropertiesFUCHSIA*      pMemoryZirconHandleProperties) {
    StartReadObjectParentInstance(device, "vkGetMemoryZirconHandlePropertiesFUCHSIA");
}

void ThreadSafety::PostCallRecordGetMemoryZirconHandlePropertiesFUCHSIA(
    VkDevice                                    device,
    VkExternalMemoryHandleTypeFlagBits          handleType,
    zx_handle_t                                 zirconHandle,
    VkMemoryZirconHandlePropertiesFUCHSIA*      pMemoryZirconHandleProperties,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkGetMemoryZirconHandlePropertiesFUCHSIA");
}
#endif // VK_USE_PLATFORM_FUCHSIA

#ifdef VK_USE_PLATFORM_FUCHSIA

void ThreadSafety::PreCallRecordImportSemaphoreZirconHandleFUCHSIA(
    VkDevice                                    device,
    const VkImportSemaphoreZirconHandleInfoFUCHSIA* pImportSemaphoreZirconHandleInfo) {
    StartReadObjectParentInstance(device, "vkImportSemaphoreZirconHandleFUCHSIA");
}

void ThreadSafety::PostCallRecordImportSemaphoreZirconHandleFUCHSIA(
    VkDevice                                    device,
    const VkImportSemaphoreZirconHandleInfoFUCHSIA* pImportSemaphoreZirconHandleInfo,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkImportSemaphoreZirconHandleFUCHSIA");
}

void ThreadSafety::PreCallRecordGetSemaphoreZirconHandleFUCHSIA(
    VkDevice                                    device,
    const VkSemaphoreGetZirconHandleInfoFUCHSIA* pGetZirconHandleInfo,
    zx_handle_t*                                pZirconHandle) {
    StartReadObjectParentInstance(device, "vkGetSemaphoreZirconHandleFUCHSIA");
}

void ThreadSafety::PostCallRecordGetSemaphoreZirconHandleFUCHSIA(
    VkDevice                                    device,
    const VkSemaphoreGetZirconHandleInfoFUCHSIA* pGetZirconHandleInfo,
    zx_handle_t*                                pZirconHandle,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkGetSemaphoreZirconHandleFUCHSIA");
}
#endif // VK_USE_PLATFORM_FUCHSIA

#ifdef VK_USE_PLATFORM_FUCHSIA

void ThreadSafety::PreCallRecordCreateBufferCollectionFUCHSIA(
    VkDevice                                    device,
    const VkBufferCollectionCreateInfoFUCHSIA*  pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkBufferCollectionFUCHSIA*                  pCollection) {
    StartReadObjectParentInstance(device, "vkCreateBufferCollectionFUCHSIA");
}

void ThreadSafety::PostCallRecordCreateBufferCollectionFUCHSIA(
    VkDevice                                    device,
    const VkBufferCollectionCreateInfoFUCHSIA*  pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkBufferCollectionFUCHSIA*                  pCollection,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkCreateBufferCollectionFUCHSIA");
    if (result == VK_SUCCESS) {
        CreateObject(*pCollection);
    }
}

void ThreadSafety::PreCallRecordSetBufferCollectionImageConstraintsFUCHSIA(
    VkDevice                                    device,
    VkBufferCollectionFUCHSIA                   collection,
    const VkImageConstraintsInfoFUCHSIA*        pImageConstraintsInfo) {
    StartReadObjectParentInstance(device, "vkSetBufferCollectionImageConstraintsFUCHSIA");
    StartReadObject(collection, "vkSetBufferCollectionImageConstraintsFUCHSIA");
}

void ThreadSafety::PostCallRecordSetBufferCollectionImageConstraintsFUCHSIA(
    VkDevice                                    device,
    VkBufferCollectionFUCHSIA                   collection,
    const VkImageConstraintsInfoFUCHSIA*        pImageConstraintsInfo,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkSetBufferCollectionImageConstraintsFUCHSIA");
    FinishReadObject(collection, "vkSetBufferCollectionImageConstraintsFUCHSIA");
}

void ThreadSafety::PreCallRecordSetBufferCollectionBufferConstraintsFUCHSIA(
    VkDevice                                    device,
    VkBufferCollectionFUCHSIA                   collection,
    const VkBufferConstraintsInfoFUCHSIA*       pBufferConstraintsInfo) {
    StartReadObjectParentInstance(device, "vkSetBufferCollectionBufferConstraintsFUCHSIA");
    StartReadObject(collection, "vkSetBufferCollectionBufferConstraintsFUCHSIA");
}

void ThreadSafety::PostCallRecordSetBufferCollectionBufferConstraintsFUCHSIA(
    VkDevice                                    device,
    VkBufferCollectionFUCHSIA                   collection,
    const VkBufferConstraintsInfoFUCHSIA*       pBufferConstraintsInfo,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkSetBufferCollectionBufferConstraintsFUCHSIA");
    FinishReadObject(collection, "vkSetBufferCollectionBufferConstraintsFUCHSIA");
}

void ThreadSafety::PreCallRecordDestroyBufferCollectionFUCHSIA(
    VkDevice                                    device,
    VkBufferCollectionFUCHSIA                   collection,
    const VkAllocationCallbacks*                pAllocator) {
    StartReadObjectParentInstance(device, "vkDestroyBufferCollectionFUCHSIA");
    StartReadObject(collection, "vkDestroyBufferCollectionFUCHSIA");
}

void ThreadSafety::PostCallRecordDestroyBufferCollectionFUCHSIA(
    VkDevice                                    device,
    VkBufferCollectionFUCHSIA                   collection,
    const VkAllocationCallbacks*                pAllocator) {
    FinishReadObjectParentInstance(device, "vkDestroyBufferCollectionFUCHSIA");
    FinishReadObject(collection, "vkDestroyBufferCollectionFUCHSIA");
}

void ThreadSafety::PreCallRecordGetBufferCollectionPropertiesFUCHSIA(
    VkDevice                                    device,
    VkBufferCollectionFUCHSIA                   collection,
    VkBufferCollectionPropertiesFUCHSIA*        pProperties) {
    StartReadObjectParentInstance(device, "vkGetBufferCollectionPropertiesFUCHSIA");
    StartReadObject(collection, "vkGetBufferCollectionPropertiesFUCHSIA");
}

void ThreadSafety::PostCallRecordGetBufferCollectionPropertiesFUCHSIA(
    VkDevice                                    device,
    VkBufferCollectionFUCHSIA                   collection,
    VkBufferCollectionPropertiesFUCHSIA*        pProperties,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkGetBufferCollectionPropertiesFUCHSIA");
    FinishReadObject(collection, "vkGetBufferCollectionPropertiesFUCHSIA");
}
#endif // VK_USE_PLATFORM_FUCHSIA

void ThreadSafety::PreCallRecordGetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI(
    VkDevice                                    device,
    VkRenderPass                                renderpass,
    VkExtent2D*                                 pMaxWorkgroupSize) {
    StartReadObjectParentInstance(device, "vkGetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI");
    StartReadObject(renderpass, "vkGetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI");
}

void ThreadSafety::PostCallRecordGetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI(
    VkDevice                                    device,
    VkRenderPass                                renderpass,
    VkExtent2D*                                 pMaxWorkgroupSize,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkGetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI");
    FinishReadObject(renderpass, "vkGetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI");
}

void ThreadSafety::PreCallRecordCmdSubpassShadingHUAWEI(
    VkCommandBuffer                             commandBuffer) {
    StartWriteObject(commandBuffer, "vkCmdSubpassShadingHUAWEI");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSubpassShadingHUAWEI(
    VkCommandBuffer                             commandBuffer) {
    FinishWriteObject(commandBuffer, "vkCmdSubpassShadingHUAWEI");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdBindInvocationMaskHUAWEI(
    VkCommandBuffer                             commandBuffer,
    VkImageView                                 imageView,
    VkImageLayout                               imageLayout) {
    StartWriteObject(commandBuffer, "vkCmdBindInvocationMaskHUAWEI");
    StartReadObject(imageView, "vkCmdBindInvocationMaskHUAWEI");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdBindInvocationMaskHUAWEI(
    VkCommandBuffer                             commandBuffer,
    VkImageView                                 imageView,
    VkImageLayout                               imageLayout) {
    FinishWriteObject(commandBuffer, "vkCmdBindInvocationMaskHUAWEI");
    FinishReadObject(imageView, "vkCmdBindInvocationMaskHUAWEI");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordGetMemoryRemoteAddressNV(
    VkDevice                                    device,
    const VkMemoryGetRemoteAddressInfoNV*       pMemoryGetRemoteAddressInfo,
    VkRemoteAddressNV*                          pAddress) {
    StartReadObjectParentInstance(device, "vkGetMemoryRemoteAddressNV");
}

void ThreadSafety::PostCallRecordGetMemoryRemoteAddressNV(
    VkDevice                                    device,
    const VkMemoryGetRemoteAddressInfoNV*       pMemoryGetRemoteAddressInfo,
    VkRemoteAddressNV*                          pAddress,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkGetMemoryRemoteAddressNV");
}

void ThreadSafety::PreCallRecordGetPipelinePropertiesEXT(
    VkDevice                                    device,
    const VkPipelineInfoEXT*                    pPipelineInfo,
    VkBaseOutStructure*                         pPipelineProperties) {
    StartReadObjectParentInstance(device, "vkGetPipelinePropertiesEXT");
}

void ThreadSafety::PostCallRecordGetPipelinePropertiesEXT(
    VkDevice                                    device,
    const VkPipelineInfoEXT*                    pPipelineInfo,
    VkBaseOutStructure*                         pPipelineProperties,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkGetPipelinePropertiesEXT");
}

void ThreadSafety::PreCallRecordCmdSetPatchControlPointsEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    patchControlPoints) {
    StartWriteObject(commandBuffer, "vkCmdSetPatchControlPointsEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetPatchControlPointsEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    patchControlPoints) {
    FinishWriteObject(commandBuffer, "vkCmdSetPatchControlPointsEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetRasterizerDiscardEnableEXT(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    rasterizerDiscardEnable) {
    StartWriteObject(commandBuffer, "vkCmdSetRasterizerDiscardEnableEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetRasterizerDiscardEnableEXT(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    rasterizerDiscardEnable) {
    FinishWriteObject(commandBuffer, "vkCmdSetRasterizerDiscardEnableEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetDepthBiasEnableEXT(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    depthBiasEnable) {
    StartWriteObject(commandBuffer, "vkCmdSetDepthBiasEnableEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetDepthBiasEnableEXT(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    depthBiasEnable) {
    FinishWriteObject(commandBuffer, "vkCmdSetDepthBiasEnableEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetLogicOpEXT(
    VkCommandBuffer                             commandBuffer,
    VkLogicOp                                   logicOp) {
    StartWriteObject(commandBuffer, "vkCmdSetLogicOpEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetLogicOpEXT(
    VkCommandBuffer                             commandBuffer,
    VkLogicOp                                   logicOp) {
    FinishWriteObject(commandBuffer, "vkCmdSetLogicOpEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetPrimitiveRestartEnableEXT(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    primitiveRestartEnable) {
    StartWriteObject(commandBuffer, "vkCmdSetPrimitiveRestartEnableEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetPrimitiveRestartEnableEXT(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    primitiveRestartEnable) {
    FinishWriteObject(commandBuffer, "vkCmdSetPrimitiveRestartEnableEXT");
    // Host access to commandBuffer must be externally synchronized
}

#ifdef VK_USE_PLATFORM_SCREEN_QNX

void ThreadSafety::PreCallRecordCreateScreenSurfaceQNX(
    VkInstance                                  instance,
    const VkScreenSurfaceCreateInfoQNX*         pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) {
    StartReadObjectParentInstance(instance, "vkCreateScreenSurfaceQNX");
}

void ThreadSafety::PostCallRecordCreateScreenSurfaceQNX(
    VkInstance                                  instance,
    const VkScreenSurfaceCreateInfoQNX*         pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface,
    VkResult                                    result) {
    FinishReadObjectParentInstance(instance, "vkCreateScreenSurfaceQNX");
    if (result == VK_SUCCESS) {
        CreateObjectParentInstance(*pSurface);
    }
}
#endif // VK_USE_PLATFORM_SCREEN_QNX

void ThreadSafety::PreCallRecordCmdSetColorWriteEnableEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    attachmentCount,
    const VkBool32*                             pColorWriteEnables) {
    StartWriteObject(commandBuffer, "vkCmdSetColorWriteEnableEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetColorWriteEnableEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    attachmentCount,
    const VkBool32*                             pColorWriteEnables) {
    FinishWriteObject(commandBuffer, "vkCmdSetColorWriteEnableEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdDrawMultiEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    drawCount,
    const VkMultiDrawInfoEXT*                   pVertexInfo,
    uint32_t                                    instanceCount,
    uint32_t                                    firstInstance,
    uint32_t                                    stride) {
    StartWriteObject(commandBuffer, "vkCmdDrawMultiEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdDrawMultiEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    drawCount,
    const VkMultiDrawInfoEXT*                   pVertexInfo,
    uint32_t                                    instanceCount,
    uint32_t                                    firstInstance,
    uint32_t                                    stride) {
    FinishWriteObject(commandBuffer, "vkCmdDrawMultiEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdDrawMultiIndexedEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    drawCount,
    const VkMultiDrawIndexedInfoEXT*            pIndexInfo,
    uint32_t                                    instanceCount,
    uint32_t                                    firstInstance,
    uint32_t                                    stride,
    const int32_t*                              pVertexOffset) {
    StartWriteObject(commandBuffer, "vkCmdDrawMultiIndexedEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdDrawMultiIndexedEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    drawCount,
    const VkMultiDrawIndexedInfoEXT*            pIndexInfo,
    uint32_t                                    instanceCount,
    uint32_t                                    firstInstance,
    uint32_t                                    stride,
    const int32_t*                              pVertexOffset) {
    FinishWriteObject(commandBuffer, "vkCmdDrawMultiIndexedEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCreateMicromapEXT(
    VkDevice                                    device,
    const VkMicromapCreateInfoEXT*              pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkMicromapEXT*                              pMicromap) {
    StartReadObjectParentInstance(device, "vkCreateMicromapEXT");
}

void ThreadSafety::PostCallRecordCreateMicromapEXT(
    VkDevice                                    device,
    const VkMicromapCreateInfoEXT*              pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkMicromapEXT*                              pMicromap,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkCreateMicromapEXT");
    if (result == VK_SUCCESS) {
        CreateObject(*pMicromap);
    }
}

void ThreadSafety::PreCallRecordDestroyMicromapEXT(
    VkDevice                                    device,
    VkMicromapEXT                               micromap,
    const VkAllocationCallbacks*                pAllocator) {
    StartReadObjectParentInstance(device, "vkDestroyMicromapEXT");
    StartWriteObject(micromap, "vkDestroyMicromapEXT");
    // Host access to micromap must be externally synchronized
}

void ThreadSafety::PostCallRecordDestroyMicromapEXT(
    VkDevice                                    device,
    VkMicromapEXT                               micromap,
    const VkAllocationCallbacks*                pAllocator) {
    FinishReadObjectParentInstance(device, "vkDestroyMicromapEXT");
    FinishWriteObject(micromap, "vkDestroyMicromapEXT");
    DestroyObject(micromap);
    // Host access to micromap must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdBuildMicromapsEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    infoCount,
    const VkMicromapBuildInfoEXT*               pInfos) {
    StartWriteObject(commandBuffer, "vkCmdBuildMicromapsEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdBuildMicromapsEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    infoCount,
    const VkMicromapBuildInfoEXT*               pInfos) {
    FinishWriteObject(commandBuffer, "vkCmdBuildMicromapsEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordBuildMicromapsEXT(
    VkDevice                                    device,
    VkDeferredOperationKHR                      deferredOperation,
    uint32_t                                    infoCount,
    const VkMicromapBuildInfoEXT*               pInfos) {
    StartReadObjectParentInstance(device, "vkBuildMicromapsEXT");
    StartReadObject(deferredOperation, "vkBuildMicromapsEXT");
}

void ThreadSafety::PostCallRecordBuildMicromapsEXT(
    VkDevice                                    device,
    VkDeferredOperationKHR                      deferredOperation,
    uint32_t                                    infoCount,
    const VkMicromapBuildInfoEXT*               pInfos,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkBuildMicromapsEXT");
    FinishReadObject(deferredOperation, "vkBuildMicromapsEXT");
}

void ThreadSafety::PreCallRecordCopyMicromapEXT(
    VkDevice                                    device,
    VkDeferredOperationKHR                      deferredOperation,
    const VkCopyMicromapInfoEXT*                pInfo) {
    StartReadObjectParentInstance(device, "vkCopyMicromapEXT");
    StartReadObject(deferredOperation, "vkCopyMicromapEXT");
}

void ThreadSafety::PostCallRecordCopyMicromapEXT(
    VkDevice                                    device,
    VkDeferredOperationKHR                      deferredOperation,
    const VkCopyMicromapInfoEXT*                pInfo,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkCopyMicromapEXT");
    FinishReadObject(deferredOperation, "vkCopyMicromapEXT");
}

void ThreadSafety::PreCallRecordCopyMicromapToMemoryEXT(
    VkDevice                                    device,
    VkDeferredOperationKHR                      deferredOperation,
    const VkCopyMicromapToMemoryInfoEXT*        pInfo) {
    StartReadObjectParentInstance(device, "vkCopyMicromapToMemoryEXT");
    StartReadObject(deferredOperation, "vkCopyMicromapToMemoryEXT");
}

void ThreadSafety::PostCallRecordCopyMicromapToMemoryEXT(
    VkDevice                                    device,
    VkDeferredOperationKHR                      deferredOperation,
    const VkCopyMicromapToMemoryInfoEXT*        pInfo,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkCopyMicromapToMemoryEXT");
    FinishReadObject(deferredOperation, "vkCopyMicromapToMemoryEXT");
}

void ThreadSafety::PreCallRecordCopyMemoryToMicromapEXT(
    VkDevice                                    device,
    VkDeferredOperationKHR                      deferredOperation,
    const VkCopyMemoryToMicromapInfoEXT*        pInfo) {
    StartReadObjectParentInstance(device, "vkCopyMemoryToMicromapEXT");
    StartReadObject(deferredOperation, "vkCopyMemoryToMicromapEXT");
}

void ThreadSafety::PostCallRecordCopyMemoryToMicromapEXT(
    VkDevice                                    device,
    VkDeferredOperationKHR                      deferredOperation,
    const VkCopyMemoryToMicromapInfoEXT*        pInfo,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkCopyMemoryToMicromapEXT");
    FinishReadObject(deferredOperation, "vkCopyMemoryToMicromapEXT");
}

void ThreadSafety::PreCallRecordWriteMicromapsPropertiesEXT(
    VkDevice                                    device,
    uint32_t                                    micromapCount,
    const VkMicromapEXT*                        pMicromaps,
    VkQueryType                                 queryType,
    size_t                                      dataSize,
    void*                                       pData,
    size_t                                      stride) {
    StartReadObjectParentInstance(device, "vkWriteMicromapsPropertiesEXT");
    if (pMicromaps) {
        for (uint32_t index = 0; index < micromapCount; index++) {
            StartReadObject(pMicromaps[index], "vkWriteMicromapsPropertiesEXT");
        }
    }
}

void ThreadSafety::PostCallRecordWriteMicromapsPropertiesEXT(
    VkDevice                                    device,
    uint32_t                                    micromapCount,
    const VkMicromapEXT*                        pMicromaps,
    VkQueryType                                 queryType,
    size_t                                      dataSize,
    void*                                       pData,
    size_t                                      stride,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkWriteMicromapsPropertiesEXT");
    if (pMicromaps) {
        for (uint32_t index = 0; index < micromapCount; index++) {
            FinishReadObject(pMicromaps[index], "vkWriteMicromapsPropertiesEXT");
        }
    }
}

void ThreadSafety::PreCallRecordCmdCopyMicromapEXT(
    VkCommandBuffer                             commandBuffer,
    const VkCopyMicromapInfoEXT*                pInfo) {
    StartWriteObject(commandBuffer, "vkCmdCopyMicromapEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdCopyMicromapEXT(
    VkCommandBuffer                             commandBuffer,
    const VkCopyMicromapInfoEXT*                pInfo) {
    FinishWriteObject(commandBuffer, "vkCmdCopyMicromapEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdCopyMicromapToMemoryEXT(
    VkCommandBuffer                             commandBuffer,
    const VkCopyMicromapToMemoryInfoEXT*        pInfo) {
    StartWriteObject(commandBuffer, "vkCmdCopyMicromapToMemoryEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdCopyMicromapToMemoryEXT(
    VkCommandBuffer                             commandBuffer,
    const VkCopyMicromapToMemoryInfoEXT*        pInfo) {
    FinishWriteObject(commandBuffer, "vkCmdCopyMicromapToMemoryEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdCopyMemoryToMicromapEXT(
    VkCommandBuffer                             commandBuffer,
    const VkCopyMemoryToMicromapInfoEXT*        pInfo) {
    StartWriteObject(commandBuffer, "vkCmdCopyMemoryToMicromapEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdCopyMemoryToMicromapEXT(
    VkCommandBuffer                             commandBuffer,
    const VkCopyMemoryToMicromapInfoEXT*        pInfo) {
    FinishWriteObject(commandBuffer, "vkCmdCopyMemoryToMicromapEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdWriteMicromapsPropertiesEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    micromapCount,
    const VkMicromapEXT*                        pMicromaps,
    VkQueryType                                 queryType,
    VkQueryPool                                 queryPool,
    uint32_t                                    firstQuery) {
    StartWriteObject(commandBuffer, "vkCmdWriteMicromapsPropertiesEXT");
    if (pMicromaps) {
        for (uint32_t index = 0; index < micromapCount; index++) {
            StartReadObject(pMicromaps[index], "vkCmdWriteMicromapsPropertiesEXT");
        }
    }
    StartReadObject(queryPool, "vkCmdWriteMicromapsPropertiesEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdWriteMicromapsPropertiesEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    micromapCount,
    const VkMicromapEXT*                        pMicromaps,
    VkQueryType                                 queryType,
    VkQueryPool                                 queryPool,
    uint32_t                                    firstQuery) {
    FinishWriteObject(commandBuffer, "vkCmdWriteMicromapsPropertiesEXT");
    if (pMicromaps) {
        for (uint32_t index = 0; index < micromapCount; index++) {
            FinishReadObject(pMicromaps[index], "vkCmdWriteMicromapsPropertiesEXT");
        }
    }
    FinishReadObject(queryPool, "vkCmdWriteMicromapsPropertiesEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordGetDeviceMicromapCompatibilityEXT(
    VkDevice                                    device,
    const VkMicromapVersionInfoEXT*             pVersionInfo,
    VkAccelerationStructureCompatibilityKHR*    pCompatibility) {
    StartReadObjectParentInstance(device, "vkGetDeviceMicromapCompatibilityEXT");
}

void ThreadSafety::PostCallRecordGetDeviceMicromapCompatibilityEXT(
    VkDevice                                    device,
    const VkMicromapVersionInfoEXT*             pVersionInfo,
    VkAccelerationStructureCompatibilityKHR*    pCompatibility) {
    FinishReadObjectParentInstance(device, "vkGetDeviceMicromapCompatibilityEXT");
}

void ThreadSafety::PreCallRecordGetMicromapBuildSizesEXT(
    VkDevice                                    device,
    VkAccelerationStructureBuildTypeKHR         buildType,
    const VkMicromapBuildInfoEXT*               pBuildInfo,
    VkMicromapBuildSizesInfoEXT*                pSizeInfo) {
    StartReadObjectParentInstance(device, "vkGetMicromapBuildSizesEXT");
}

void ThreadSafety::PostCallRecordGetMicromapBuildSizesEXT(
    VkDevice                                    device,
    VkAccelerationStructureBuildTypeKHR         buildType,
    const VkMicromapBuildInfoEXT*               pBuildInfo,
    VkMicromapBuildSizesInfoEXT*                pSizeInfo) {
    FinishReadObjectParentInstance(device, "vkGetMicromapBuildSizesEXT");
}

void ThreadSafety::PreCallRecordCmdDrawClusterHUAWEI(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    groupCountX,
    uint32_t                                    groupCountY,
    uint32_t                                    groupCountZ) {
    StartWriteObject(commandBuffer, "vkCmdDrawClusterHUAWEI");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdDrawClusterHUAWEI(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    groupCountX,
    uint32_t                                    groupCountY,
    uint32_t                                    groupCountZ) {
    FinishWriteObject(commandBuffer, "vkCmdDrawClusterHUAWEI");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdDrawClusterIndirectHUAWEI(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset) {
    StartWriteObject(commandBuffer, "vkCmdDrawClusterIndirectHUAWEI");
    StartReadObject(buffer, "vkCmdDrawClusterIndirectHUAWEI");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdDrawClusterIndirectHUAWEI(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset) {
    FinishWriteObject(commandBuffer, "vkCmdDrawClusterIndirectHUAWEI");
    FinishReadObject(buffer, "vkCmdDrawClusterIndirectHUAWEI");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordSetDeviceMemoryPriorityEXT(
    VkDevice                                    device,
    VkDeviceMemory                              memory,
    float                                       priority) {
    StartReadObjectParentInstance(device, "vkSetDeviceMemoryPriorityEXT");
    StartReadObject(memory, "vkSetDeviceMemoryPriorityEXT");
}

void ThreadSafety::PostCallRecordSetDeviceMemoryPriorityEXT(
    VkDevice                                    device,
    VkDeviceMemory                              memory,
    float                                       priority) {
    FinishReadObjectParentInstance(device, "vkSetDeviceMemoryPriorityEXT");
    FinishReadObject(memory, "vkSetDeviceMemoryPriorityEXT");
}

void ThreadSafety::PreCallRecordGetDescriptorSetLayoutHostMappingInfoVALVE(
    VkDevice                                    device,
    const VkDescriptorSetBindingReferenceVALVE* pBindingReference,
    VkDescriptorSetLayoutHostMappingInfoVALVE*  pHostMapping) {
    StartReadObjectParentInstance(device, "vkGetDescriptorSetLayoutHostMappingInfoVALVE");
}

void ThreadSafety::PostCallRecordGetDescriptorSetLayoutHostMappingInfoVALVE(
    VkDevice                                    device,
    const VkDescriptorSetBindingReferenceVALVE* pBindingReference,
    VkDescriptorSetLayoutHostMappingInfoVALVE*  pHostMapping) {
    FinishReadObjectParentInstance(device, "vkGetDescriptorSetLayoutHostMappingInfoVALVE");
}

void ThreadSafety::PreCallRecordGetDescriptorSetHostMappingVALVE(
    VkDevice                                    device,
    VkDescriptorSet                             descriptorSet,
    void**                                      ppData) {
    StartReadObjectParentInstance(device, "vkGetDescriptorSetHostMappingVALVE");
    StartReadObject(descriptorSet, "vkGetDescriptorSetHostMappingVALVE");
}

void ThreadSafety::PostCallRecordGetDescriptorSetHostMappingVALVE(
    VkDevice                                    device,
    VkDescriptorSet                             descriptorSet,
    void**                                      ppData) {
    FinishReadObjectParentInstance(device, "vkGetDescriptorSetHostMappingVALVE");
    FinishReadObject(descriptorSet, "vkGetDescriptorSetHostMappingVALVE");
}

void ThreadSafety::PreCallRecordCmdCopyMemoryIndirectNV(
    VkCommandBuffer                             commandBuffer,
    VkDeviceAddress                             copyBufferAddress,
    uint32_t                                    copyCount,
    uint32_t                                    stride) {
    StartWriteObject(commandBuffer, "vkCmdCopyMemoryIndirectNV");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdCopyMemoryIndirectNV(
    VkCommandBuffer                             commandBuffer,
    VkDeviceAddress                             copyBufferAddress,
    uint32_t                                    copyCount,
    uint32_t                                    stride) {
    FinishWriteObject(commandBuffer, "vkCmdCopyMemoryIndirectNV");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdCopyMemoryToImageIndirectNV(
    VkCommandBuffer                             commandBuffer,
    VkDeviceAddress                             copyBufferAddress,
    uint32_t                                    copyCount,
    uint32_t                                    stride,
    VkImage                                     dstImage,
    VkImageLayout                               dstImageLayout,
    const VkImageSubresourceLayers*             pImageSubresources) {
    StartWriteObject(commandBuffer, "vkCmdCopyMemoryToImageIndirectNV");
    StartReadObject(dstImage, "vkCmdCopyMemoryToImageIndirectNV");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdCopyMemoryToImageIndirectNV(
    VkCommandBuffer                             commandBuffer,
    VkDeviceAddress                             copyBufferAddress,
    uint32_t                                    copyCount,
    uint32_t                                    stride,
    VkImage                                     dstImage,
    VkImageLayout                               dstImageLayout,
    const VkImageSubresourceLayers*             pImageSubresources) {
    FinishWriteObject(commandBuffer, "vkCmdCopyMemoryToImageIndirectNV");
    FinishReadObject(dstImage, "vkCmdCopyMemoryToImageIndirectNV");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdDecompressMemoryNV(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    decompressRegionCount,
    const VkDecompressMemoryRegionNV*           pDecompressMemoryRegions) {
    StartWriteObject(commandBuffer, "vkCmdDecompressMemoryNV");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdDecompressMemoryNV(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    decompressRegionCount,
    const VkDecompressMemoryRegionNV*           pDecompressMemoryRegions) {
    FinishWriteObject(commandBuffer, "vkCmdDecompressMemoryNV");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdDecompressMemoryIndirectCountNV(
    VkCommandBuffer                             commandBuffer,
    VkDeviceAddress                             indirectCommandsAddress,
    VkDeviceAddress                             indirectCommandsCountAddress,
    uint32_t                                    stride) {
    StartWriteObject(commandBuffer, "vkCmdDecompressMemoryIndirectCountNV");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdDecompressMemoryIndirectCountNV(
    VkCommandBuffer                             commandBuffer,
    VkDeviceAddress                             indirectCommandsAddress,
    VkDeviceAddress                             indirectCommandsCountAddress,
    uint32_t                                    stride) {
    FinishWriteObject(commandBuffer, "vkCmdDecompressMemoryIndirectCountNV");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetTessellationDomainOriginEXT(
    VkCommandBuffer                             commandBuffer,
    VkTessellationDomainOrigin                  domainOrigin) {
    StartWriteObject(commandBuffer, "vkCmdSetTessellationDomainOriginEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetTessellationDomainOriginEXT(
    VkCommandBuffer                             commandBuffer,
    VkTessellationDomainOrigin                  domainOrigin) {
    FinishWriteObject(commandBuffer, "vkCmdSetTessellationDomainOriginEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetDepthClampEnableEXT(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    depthClampEnable) {
    StartWriteObject(commandBuffer, "vkCmdSetDepthClampEnableEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetDepthClampEnableEXT(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    depthClampEnable) {
    FinishWriteObject(commandBuffer, "vkCmdSetDepthClampEnableEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetPolygonModeEXT(
    VkCommandBuffer                             commandBuffer,
    VkPolygonMode                               polygonMode) {
    StartWriteObject(commandBuffer, "vkCmdSetPolygonModeEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetPolygonModeEXT(
    VkCommandBuffer                             commandBuffer,
    VkPolygonMode                               polygonMode) {
    FinishWriteObject(commandBuffer, "vkCmdSetPolygonModeEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetRasterizationSamplesEXT(
    VkCommandBuffer                             commandBuffer,
    VkSampleCountFlagBits                       rasterizationSamples) {
    StartWriteObject(commandBuffer, "vkCmdSetRasterizationSamplesEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetRasterizationSamplesEXT(
    VkCommandBuffer                             commandBuffer,
    VkSampleCountFlagBits                       rasterizationSamples) {
    FinishWriteObject(commandBuffer, "vkCmdSetRasterizationSamplesEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetSampleMaskEXT(
    VkCommandBuffer                             commandBuffer,
    VkSampleCountFlagBits                       samples,
    const VkSampleMask*                         pSampleMask) {
    StartWriteObject(commandBuffer, "vkCmdSetSampleMaskEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetSampleMaskEXT(
    VkCommandBuffer                             commandBuffer,
    VkSampleCountFlagBits                       samples,
    const VkSampleMask*                         pSampleMask) {
    FinishWriteObject(commandBuffer, "vkCmdSetSampleMaskEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetAlphaToCoverageEnableEXT(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    alphaToCoverageEnable) {
    StartWriteObject(commandBuffer, "vkCmdSetAlphaToCoverageEnableEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetAlphaToCoverageEnableEXT(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    alphaToCoverageEnable) {
    FinishWriteObject(commandBuffer, "vkCmdSetAlphaToCoverageEnableEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetAlphaToOneEnableEXT(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    alphaToOneEnable) {
    StartWriteObject(commandBuffer, "vkCmdSetAlphaToOneEnableEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetAlphaToOneEnableEXT(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    alphaToOneEnable) {
    FinishWriteObject(commandBuffer, "vkCmdSetAlphaToOneEnableEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetLogicOpEnableEXT(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    logicOpEnable) {
    StartWriteObject(commandBuffer, "vkCmdSetLogicOpEnableEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetLogicOpEnableEXT(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    logicOpEnable) {
    FinishWriteObject(commandBuffer, "vkCmdSetLogicOpEnableEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetColorBlendEnableEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstAttachment,
    uint32_t                                    attachmentCount,
    const VkBool32*                             pColorBlendEnables) {
    StartWriteObject(commandBuffer, "vkCmdSetColorBlendEnableEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetColorBlendEnableEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstAttachment,
    uint32_t                                    attachmentCount,
    const VkBool32*                             pColorBlendEnables) {
    FinishWriteObject(commandBuffer, "vkCmdSetColorBlendEnableEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetColorBlendEquationEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstAttachment,
    uint32_t                                    attachmentCount,
    const VkColorBlendEquationEXT*              pColorBlendEquations) {
    StartWriteObject(commandBuffer, "vkCmdSetColorBlendEquationEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetColorBlendEquationEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstAttachment,
    uint32_t                                    attachmentCount,
    const VkColorBlendEquationEXT*              pColorBlendEquations) {
    FinishWriteObject(commandBuffer, "vkCmdSetColorBlendEquationEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetColorWriteMaskEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstAttachment,
    uint32_t                                    attachmentCount,
    const VkColorComponentFlags*                pColorWriteMasks) {
    StartWriteObject(commandBuffer, "vkCmdSetColorWriteMaskEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetColorWriteMaskEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstAttachment,
    uint32_t                                    attachmentCount,
    const VkColorComponentFlags*                pColorWriteMasks) {
    FinishWriteObject(commandBuffer, "vkCmdSetColorWriteMaskEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetRasterizationStreamEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    rasterizationStream) {
    StartWriteObject(commandBuffer, "vkCmdSetRasterizationStreamEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetRasterizationStreamEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    rasterizationStream) {
    FinishWriteObject(commandBuffer, "vkCmdSetRasterizationStreamEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetConservativeRasterizationModeEXT(
    VkCommandBuffer                             commandBuffer,
    VkConservativeRasterizationModeEXT          conservativeRasterizationMode) {
    StartWriteObject(commandBuffer, "vkCmdSetConservativeRasterizationModeEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetConservativeRasterizationModeEXT(
    VkCommandBuffer                             commandBuffer,
    VkConservativeRasterizationModeEXT          conservativeRasterizationMode) {
    FinishWriteObject(commandBuffer, "vkCmdSetConservativeRasterizationModeEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetExtraPrimitiveOverestimationSizeEXT(
    VkCommandBuffer                             commandBuffer,
    float                                       extraPrimitiveOverestimationSize) {
    StartWriteObject(commandBuffer, "vkCmdSetExtraPrimitiveOverestimationSizeEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetExtraPrimitiveOverestimationSizeEXT(
    VkCommandBuffer                             commandBuffer,
    float                                       extraPrimitiveOverestimationSize) {
    FinishWriteObject(commandBuffer, "vkCmdSetExtraPrimitiveOverestimationSizeEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetDepthClipEnableEXT(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    depthClipEnable) {
    StartWriteObject(commandBuffer, "vkCmdSetDepthClipEnableEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetDepthClipEnableEXT(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    depthClipEnable) {
    FinishWriteObject(commandBuffer, "vkCmdSetDepthClipEnableEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetSampleLocationsEnableEXT(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    sampleLocationsEnable) {
    StartWriteObject(commandBuffer, "vkCmdSetSampleLocationsEnableEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetSampleLocationsEnableEXT(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    sampleLocationsEnable) {
    FinishWriteObject(commandBuffer, "vkCmdSetSampleLocationsEnableEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetColorBlendAdvancedEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstAttachment,
    uint32_t                                    attachmentCount,
    const VkColorBlendAdvancedEXT*              pColorBlendAdvanced) {
    StartWriteObject(commandBuffer, "vkCmdSetColorBlendAdvancedEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetColorBlendAdvancedEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstAttachment,
    uint32_t                                    attachmentCount,
    const VkColorBlendAdvancedEXT*              pColorBlendAdvanced) {
    FinishWriteObject(commandBuffer, "vkCmdSetColorBlendAdvancedEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetProvokingVertexModeEXT(
    VkCommandBuffer                             commandBuffer,
    VkProvokingVertexModeEXT                    provokingVertexMode) {
    StartWriteObject(commandBuffer, "vkCmdSetProvokingVertexModeEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetProvokingVertexModeEXT(
    VkCommandBuffer                             commandBuffer,
    VkProvokingVertexModeEXT                    provokingVertexMode) {
    FinishWriteObject(commandBuffer, "vkCmdSetProvokingVertexModeEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetLineRasterizationModeEXT(
    VkCommandBuffer                             commandBuffer,
    VkLineRasterizationModeEXT                  lineRasterizationMode) {
    StartWriteObject(commandBuffer, "vkCmdSetLineRasterizationModeEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetLineRasterizationModeEXT(
    VkCommandBuffer                             commandBuffer,
    VkLineRasterizationModeEXT                  lineRasterizationMode) {
    FinishWriteObject(commandBuffer, "vkCmdSetLineRasterizationModeEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetLineStippleEnableEXT(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    stippledLineEnable) {
    StartWriteObject(commandBuffer, "vkCmdSetLineStippleEnableEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetLineStippleEnableEXT(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    stippledLineEnable) {
    FinishWriteObject(commandBuffer, "vkCmdSetLineStippleEnableEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetDepthClipNegativeOneToOneEXT(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    negativeOneToOne) {
    StartWriteObject(commandBuffer, "vkCmdSetDepthClipNegativeOneToOneEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetDepthClipNegativeOneToOneEXT(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    negativeOneToOne) {
    FinishWriteObject(commandBuffer, "vkCmdSetDepthClipNegativeOneToOneEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetViewportWScalingEnableNV(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    viewportWScalingEnable) {
    StartWriteObject(commandBuffer, "vkCmdSetViewportWScalingEnableNV");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetViewportWScalingEnableNV(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    viewportWScalingEnable) {
    FinishWriteObject(commandBuffer, "vkCmdSetViewportWScalingEnableNV");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetViewportSwizzleNV(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstViewport,
    uint32_t                                    viewportCount,
    const VkViewportSwizzleNV*                  pViewportSwizzles) {
    StartWriteObject(commandBuffer, "vkCmdSetViewportSwizzleNV");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetViewportSwizzleNV(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstViewport,
    uint32_t                                    viewportCount,
    const VkViewportSwizzleNV*                  pViewportSwizzles) {
    FinishWriteObject(commandBuffer, "vkCmdSetViewportSwizzleNV");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetCoverageToColorEnableNV(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    coverageToColorEnable) {
    StartWriteObject(commandBuffer, "vkCmdSetCoverageToColorEnableNV");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetCoverageToColorEnableNV(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    coverageToColorEnable) {
    FinishWriteObject(commandBuffer, "vkCmdSetCoverageToColorEnableNV");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetCoverageToColorLocationNV(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    coverageToColorLocation) {
    StartWriteObject(commandBuffer, "vkCmdSetCoverageToColorLocationNV");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetCoverageToColorLocationNV(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    coverageToColorLocation) {
    FinishWriteObject(commandBuffer, "vkCmdSetCoverageToColorLocationNV");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetCoverageModulationModeNV(
    VkCommandBuffer                             commandBuffer,
    VkCoverageModulationModeNV                  coverageModulationMode) {
    StartWriteObject(commandBuffer, "vkCmdSetCoverageModulationModeNV");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetCoverageModulationModeNV(
    VkCommandBuffer                             commandBuffer,
    VkCoverageModulationModeNV                  coverageModulationMode) {
    FinishWriteObject(commandBuffer, "vkCmdSetCoverageModulationModeNV");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetCoverageModulationTableEnableNV(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    coverageModulationTableEnable) {
    StartWriteObject(commandBuffer, "vkCmdSetCoverageModulationTableEnableNV");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetCoverageModulationTableEnableNV(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    coverageModulationTableEnable) {
    FinishWriteObject(commandBuffer, "vkCmdSetCoverageModulationTableEnableNV");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetCoverageModulationTableNV(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    coverageModulationTableCount,
    const float*                                pCoverageModulationTable) {
    StartWriteObject(commandBuffer, "vkCmdSetCoverageModulationTableNV");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetCoverageModulationTableNV(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    coverageModulationTableCount,
    const float*                                pCoverageModulationTable) {
    FinishWriteObject(commandBuffer, "vkCmdSetCoverageModulationTableNV");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetShadingRateImageEnableNV(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    shadingRateImageEnable) {
    StartWriteObject(commandBuffer, "vkCmdSetShadingRateImageEnableNV");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetShadingRateImageEnableNV(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    shadingRateImageEnable) {
    FinishWriteObject(commandBuffer, "vkCmdSetShadingRateImageEnableNV");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetRepresentativeFragmentTestEnableNV(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    representativeFragmentTestEnable) {
    StartWriteObject(commandBuffer, "vkCmdSetRepresentativeFragmentTestEnableNV");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetRepresentativeFragmentTestEnableNV(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    representativeFragmentTestEnable) {
    FinishWriteObject(commandBuffer, "vkCmdSetRepresentativeFragmentTestEnableNV");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdSetCoverageReductionModeNV(
    VkCommandBuffer                             commandBuffer,
    VkCoverageReductionModeNV                   coverageReductionMode) {
    StartWriteObject(commandBuffer, "vkCmdSetCoverageReductionModeNV");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetCoverageReductionModeNV(
    VkCommandBuffer                             commandBuffer,
    VkCoverageReductionModeNV                   coverageReductionMode) {
    FinishWriteObject(commandBuffer, "vkCmdSetCoverageReductionModeNV");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordGetShaderModuleIdentifierEXT(
    VkDevice                                    device,
    VkShaderModule                              shaderModule,
    VkShaderModuleIdentifierEXT*                pIdentifier) {
    StartReadObjectParentInstance(device, "vkGetShaderModuleIdentifierEXT");
    StartReadObject(shaderModule, "vkGetShaderModuleIdentifierEXT");
}

void ThreadSafety::PostCallRecordGetShaderModuleIdentifierEXT(
    VkDevice                                    device,
    VkShaderModule                              shaderModule,
    VkShaderModuleIdentifierEXT*                pIdentifier) {
    FinishReadObjectParentInstance(device, "vkGetShaderModuleIdentifierEXT");
    FinishReadObject(shaderModule, "vkGetShaderModuleIdentifierEXT");
}

void ThreadSafety::PreCallRecordGetShaderModuleCreateInfoIdentifierEXT(
    VkDevice                                    device,
    const VkShaderModuleCreateInfo*             pCreateInfo,
    VkShaderModuleIdentifierEXT*                pIdentifier) {
    StartReadObjectParentInstance(device, "vkGetShaderModuleCreateInfoIdentifierEXT");
}

void ThreadSafety::PostCallRecordGetShaderModuleCreateInfoIdentifierEXT(
    VkDevice                                    device,
    const VkShaderModuleCreateInfo*             pCreateInfo,
    VkShaderModuleIdentifierEXT*                pIdentifier) {
    FinishReadObjectParentInstance(device, "vkGetShaderModuleCreateInfoIdentifierEXT");
}

void ThreadSafety::PreCallRecordCreateOpticalFlowSessionNV(
    VkDevice                                    device,
    const VkOpticalFlowSessionCreateInfoNV*     pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkOpticalFlowSessionNV*                     pSession) {
    StartReadObjectParentInstance(device, "vkCreateOpticalFlowSessionNV");
}

void ThreadSafety::PostCallRecordCreateOpticalFlowSessionNV(
    VkDevice                                    device,
    const VkOpticalFlowSessionCreateInfoNV*     pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkOpticalFlowSessionNV*                     pSession,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkCreateOpticalFlowSessionNV");
    if (result == VK_SUCCESS) {
        CreateObject(*pSession);
    }
}

void ThreadSafety::PreCallRecordDestroyOpticalFlowSessionNV(
    VkDevice                                    device,
    VkOpticalFlowSessionNV                      session,
    const VkAllocationCallbacks*                pAllocator) {
    StartReadObjectParentInstance(device, "vkDestroyOpticalFlowSessionNV");
    StartReadObject(session, "vkDestroyOpticalFlowSessionNV");
}

void ThreadSafety::PostCallRecordDestroyOpticalFlowSessionNV(
    VkDevice                                    device,
    VkOpticalFlowSessionNV                      session,
    const VkAllocationCallbacks*                pAllocator) {
    FinishReadObjectParentInstance(device, "vkDestroyOpticalFlowSessionNV");
    FinishReadObject(session, "vkDestroyOpticalFlowSessionNV");
}

void ThreadSafety::PreCallRecordBindOpticalFlowSessionImageNV(
    VkDevice                                    device,
    VkOpticalFlowSessionNV                      session,
    VkOpticalFlowSessionBindingPointNV          bindingPoint,
    VkImageView                                 view,
    VkImageLayout                               layout) {
    StartReadObjectParentInstance(device, "vkBindOpticalFlowSessionImageNV");
    StartReadObject(session, "vkBindOpticalFlowSessionImageNV");
    StartReadObject(view, "vkBindOpticalFlowSessionImageNV");
}

void ThreadSafety::PostCallRecordBindOpticalFlowSessionImageNV(
    VkDevice                                    device,
    VkOpticalFlowSessionNV                      session,
    VkOpticalFlowSessionBindingPointNV          bindingPoint,
    VkImageView                                 view,
    VkImageLayout                               layout,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkBindOpticalFlowSessionImageNV");
    FinishReadObject(session, "vkBindOpticalFlowSessionImageNV");
    FinishReadObject(view, "vkBindOpticalFlowSessionImageNV");
}

void ThreadSafety::PreCallRecordCmdOpticalFlowExecuteNV(
    VkCommandBuffer                             commandBuffer,
    VkOpticalFlowSessionNV                      session,
    const VkOpticalFlowExecuteInfoNV*           pExecuteInfo) {
    StartReadObject(commandBuffer, "vkCmdOpticalFlowExecuteNV");
    StartReadObject(session, "vkCmdOpticalFlowExecuteNV");
}

void ThreadSafety::PostCallRecordCmdOpticalFlowExecuteNV(
    VkCommandBuffer                             commandBuffer,
    VkOpticalFlowSessionNV                      session,
    const VkOpticalFlowExecuteInfoNV*           pExecuteInfo) {
    FinishReadObject(commandBuffer, "vkCmdOpticalFlowExecuteNV");
    FinishReadObject(session, "vkCmdOpticalFlowExecuteNV");
}

void ThreadSafety::PreCallRecordGetFramebufferTilePropertiesQCOM(
    VkDevice                                    device,
    VkFramebuffer                               framebuffer,
    uint32_t*                                   pPropertiesCount,
    VkTilePropertiesQCOM*                       pProperties) {
    StartReadObjectParentInstance(device, "vkGetFramebufferTilePropertiesQCOM");
    StartReadObject(framebuffer, "vkGetFramebufferTilePropertiesQCOM");
}

void ThreadSafety::PostCallRecordGetFramebufferTilePropertiesQCOM(
    VkDevice                                    device,
    VkFramebuffer                               framebuffer,
    uint32_t*                                   pPropertiesCount,
    VkTilePropertiesQCOM*                       pProperties,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkGetFramebufferTilePropertiesQCOM");
    FinishReadObject(framebuffer, "vkGetFramebufferTilePropertiesQCOM");
}

void ThreadSafety::PreCallRecordGetDynamicRenderingTilePropertiesQCOM(
    VkDevice                                    device,
    const VkRenderingInfo*                      pRenderingInfo,
    VkTilePropertiesQCOM*                       pProperties) {
    StartReadObjectParentInstance(device, "vkGetDynamicRenderingTilePropertiesQCOM");
}

void ThreadSafety::PostCallRecordGetDynamicRenderingTilePropertiesQCOM(
    VkDevice                                    device,
    const VkRenderingInfo*                      pRenderingInfo,
    VkTilePropertiesQCOM*                       pProperties,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkGetDynamicRenderingTilePropertiesQCOM");
}

void ThreadSafety::PreCallRecordCreateAccelerationStructureKHR(
    VkDevice                                    device,
    const VkAccelerationStructureCreateInfoKHR* pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkAccelerationStructureKHR*                 pAccelerationStructure) {
    StartReadObjectParentInstance(device, "vkCreateAccelerationStructureKHR");
}

void ThreadSafety::PostCallRecordCreateAccelerationStructureKHR(
    VkDevice                                    device,
    const VkAccelerationStructureCreateInfoKHR* pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkAccelerationStructureKHR*                 pAccelerationStructure,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkCreateAccelerationStructureKHR");
    if (result == VK_SUCCESS) {
        CreateObject(*pAccelerationStructure);
    }
}

void ThreadSafety::PreCallRecordDestroyAccelerationStructureKHR(
    VkDevice                                    device,
    VkAccelerationStructureKHR                  accelerationStructure,
    const VkAllocationCallbacks*                pAllocator) {
    StartReadObjectParentInstance(device, "vkDestroyAccelerationStructureKHR");
    StartWriteObject(accelerationStructure, "vkDestroyAccelerationStructureKHR");
    // Host access to accelerationStructure must be externally synchronized
}

void ThreadSafety::PostCallRecordDestroyAccelerationStructureKHR(
    VkDevice                                    device,
    VkAccelerationStructureKHR                  accelerationStructure,
    const VkAllocationCallbacks*                pAllocator) {
    FinishReadObjectParentInstance(device, "vkDestroyAccelerationStructureKHR");
    FinishWriteObject(accelerationStructure, "vkDestroyAccelerationStructureKHR");
    DestroyObject(accelerationStructure);
    // Host access to accelerationStructure must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdBuildAccelerationStructuresKHR(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    infoCount,
    const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
    const VkAccelerationStructureBuildRangeInfoKHR* const* ppBuildRangeInfos) {
    StartWriteObject(commandBuffer, "vkCmdBuildAccelerationStructuresKHR");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdBuildAccelerationStructuresKHR(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    infoCount,
    const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
    const VkAccelerationStructureBuildRangeInfoKHR* const* ppBuildRangeInfos) {
    FinishWriteObject(commandBuffer, "vkCmdBuildAccelerationStructuresKHR");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdBuildAccelerationStructuresIndirectKHR(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    infoCount,
    const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
    const VkDeviceAddress*                      pIndirectDeviceAddresses,
    const uint32_t*                             pIndirectStrides,
    const uint32_t* const*                      ppMaxPrimitiveCounts) {
    StartWriteObject(commandBuffer, "vkCmdBuildAccelerationStructuresIndirectKHR");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdBuildAccelerationStructuresIndirectKHR(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    infoCount,
    const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
    const VkDeviceAddress*                      pIndirectDeviceAddresses,
    const uint32_t*                             pIndirectStrides,
    const uint32_t* const*                      ppMaxPrimitiveCounts) {
    FinishWriteObject(commandBuffer, "vkCmdBuildAccelerationStructuresIndirectKHR");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordBuildAccelerationStructuresKHR(
    VkDevice                                    device,
    VkDeferredOperationKHR                      deferredOperation,
    uint32_t                                    infoCount,
    const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
    const VkAccelerationStructureBuildRangeInfoKHR* const* ppBuildRangeInfos) {
    StartReadObjectParentInstance(device, "vkBuildAccelerationStructuresKHR");
    StartReadObject(deferredOperation, "vkBuildAccelerationStructuresKHR");
}

void ThreadSafety::PostCallRecordBuildAccelerationStructuresKHR(
    VkDevice                                    device,
    VkDeferredOperationKHR                      deferredOperation,
    uint32_t                                    infoCount,
    const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
    const VkAccelerationStructureBuildRangeInfoKHR* const* ppBuildRangeInfos,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkBuildAccelerationStructuresKHR");
    FinishReadObject(deferredOperation, "vkBuildAccelerationStructuresKHR");
}

void ThreadSafety::PreCallRecordCopyAccelerationStructureKHR(
    VkDevice                                    device,
    VkDeferredOperationKHR                      deferredOperation,
    const VkCopyAccelerationStructureInfoKHR*   pInfo) {
    StartReadObjectParentInstance(device, "vkCopyAccelerationStructureKHR");
    StartReadObject(deferredOperation, "vkCopyAccelerationStructureKHR");
}

void ThreadSafety::PostCallRecordCopyAccelerationStructureKHR(
    VkDevice                                    device,
    VkDeferredOperationKHR                      deferredOperation,
    const VkCopyAccelerationStructureInfoKHR*   pInfo,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkCopyAccelerationStructureKHR");
    FinishReadObject(deferredOperation, "vkCopyAccelerationStructureKHR");
}

void ThreadSafety::PreCallRecordCopyAccelerationStructureToMemoryKHR(
    VkDevice                                    device,
    VkDeferredOperationKHR                      deferredOperation,
    const VkCopyAccelerationStructureToMemoryInfoKHR* pInfo) {
    StartReadObjectParentInstance(device, "vkCopyAccelerationStructureToMemoryKHR");
    StartReadObject(deferredOperation, "vkCopyAccelerationStructureToMemoryKHR");
}

void ThreadSafety::PostCallRecordCopyAccelerationStructureToMemoryKHR(
    VkDevice                                    device,
    VkDeferredOperationKHR                      deferredOperation,
    const VkCopyAccelerationStructureToMemoryInfoKHR* pInfo,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkCopyAccelerationStructureToMemoryKHR");
    FinishReadObject(deferredOperation, "vkCopyAccelerationStructureToMemoryKHR");
}

void ThreadSafety::PreCallRecordCopyMemoryToAccelerationStructureKHR(
    VkDevice                                    device,
    VkDeferredOperationKHR                      deferredOperation,
    const VkCopyMemoryToAccelerationStructureInfoKHR* pInfo) {
    StartReadObjectParentInstance(device, "vkCopyMemoryToAccelerationStructureKHR");
    StartReadObject(deferredOperation, "vkCopyMemoryToAccelerationStructureKHR");
}

void ThreadSafety::PostCallRecordCopyMemoryToAccelerationStructureKHR(
    VkDevice                                    device,
    VkDeferredOperationKHR                      deferredOperation,
    const VkCopyMemoryToAccelerationStructureInfoKHR* pInfo,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkCopyMemoryToAccelerationStructureKHR");
    FinishReadObject(deferredOperation, "vkCopyMemoryToAccelerationStructureKHR");
}

void ThreadSafety::PreCallRecordWriteAccelerationStructuresPropertiesKHR(
    VkDevice                                    device,
    uint32_t                                    accelerationStructureCount,
    const VkAccelerationStructureKHR*           pAccelerationStructures,
    VkQueryType                                 queryType,
    size_t                                      dataSize,
    void*                                       pData,
    size_t                                      stride) {
    StartReadObjectParentInstance(device, "vkWriteAccelerationStructuresPropertiesKHR");
    if (pAccelerationStructures) {
        for (uint32_t index = 0; index < accelerationStructureCount; index++) {
            StartReadObject(pAccelerationStructures[index], "vkWriteAccelerationStructuresPropertiesKHR");
        }
    }
}

void ThreadSafety::PostCallRecordWriteAccelerationStructuresPropertiesKHR(
    VkDevice                                    device,
    uint32_t                                    accelerationStructureCount,
    const VkAccelerationStructureKHR*           pAccelerationStructures,
    VkQueryType                                 queryType,
    size_t                                      dataSize,
    void*                                       pData,
    size_t                                      stride,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkWriteAccelerationStructuresPropertiesKHR");
    if (pAccelerationStructures) {
        for (uint32_t index = 0; index < accelerationStructureCount; index++) {
            FinishReadObject(pAccelerationStructures[index], "vkWriteAccelerationStructuresPropertiesKHR");
        }
    }
}

void ThreadSafety::PreCallRecordCmdCopyAccelerationStructureKHR(
    VkCommandBuffer                             commandBuffer,
    const VkCopyAccelerationStructureInfoKHR*   pInfo) {
    StartWriteObject(commandBuffer, "vkCmdCopyAccelerationStructureKHR");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdCopyAccelerationStructureKHR(
    VkCommandBuffer                             commandBuffer,
    const VkCopyAccelerationStructureInfoKHR*   pInfo) {
    FinishWriteObject(commandBuffer, "vkCmdCopyAccelerationStructureKHR");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdCopyAccelerationStructureToMemoryKHR(
    VkCommandBuffer                             commandBuffer,
    const VkCopyAccelerationStructureToMemoryInfoKHR* pInfo) {
    StartWriteObject(commandBuffer, "vkCmdCopyAccelerationStructureToMemoryKHR");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdCopyAccelerationStructureToMemoryKHR(
    VkCommandBuffer                             commandBuffer,
    const VkCopyAccelerationStructureToMemoryInfoKHR* pInfo) {
    FinishWriteObject(commandBuffer, "vkCmdCopyAccelerationStructureToMemoryKHR");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdCopyMemoryToAccelerationStructureKHR(
    VkCommandBuffer                             commandBuffer,
    const VkCopyMemoryToAccelerationStructureInfoKHR* pInfo) {
    StartWriteObject(commandBuffer, "vkCmdCopyMemoryToAccelerationStructureKHR");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdCopyMemoryToAccelerationStructureKHR(
    VkCommandBuffer                             commandBuffer,
    const VkCopyMemoryToAccelerationStructureInfoKHR* pInfo) {
    FinishWriteObject(commandBuffer, "vkCmdCopyMemoryToAccelerationStructureKHR");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordGetAccelerationStructureDeviceAddressKHR(
    VkDevice                                    device,
    const VkAccelerationStructureDeviceAddressInfoKHR* pInfo) {
    StartReadObjectParentInstance(device, "vkGetAccelerationStructureDeviceAddressKHR");
}

void ThreadSafety::PostCallRecordGetAccelerationStructureDeviceAddressKHR(
    VkDevice                                    device,
    const VkAccelerationStructureDeviceAddressInfoKHR* pInfo,
    VkDeviceAddress                             result) {
    FinishReadObjectParentInstance(device, "vkGetAccelerationStructureDeviceAddressKHR");
}

void ThreadSafety::PreCallRecordCmdWriteAccelerationStructuresPropertiesKHR(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    accelerationStructureCount,
    const VkAccelerationStructureKHR*           pAccelerationStructures,
    VkQueryType                                 queryType,
    VkQueryPool                                 queryPool,
    uint32_t                                    firstQuery) {
    StartWriteObject(commandBuffer, "vkCmdWriteAccelerationStructuresPropertiesKHR");
    if (pAccelerationStructures) {
        for (uint32_t index = 0; index < accelerationStructureCount; index++) {
            StartReadObject(pAccelerationStructures[index], "vkCmdWriteAccelerationStructuresPropertiesKHR");
        }
    }
    StartReadObject(queryPool, "vkCmdWriteAccelerationStructuresPropertiesKHR");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdWriteAccelerationStructuresPropertiesKHR(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    accelerationStructureCount,
    const VkAccelerationStructureKHR*           pAccelerationStructures,
    VkQueryType                                 queryType,
    VkQueryPool                                 queryPool,
    uint32_t                                    firstQuery) {
    FinishWriteObject(commandBuffer, "vkCmdWriteAccelerationStructuresPropertiesKHR");
    if (pAccelerationStructures) {
        for (uint32_t index = 0; index < accelerationStructureCount; index++) {
            FinishReadObject(pAccelerationStructures[index], "vkCmdWriteAccelerationStructuresPropertiesKHR");
        }
    }
    FinishReadObject(queryPool, "vkCmdWriteAccelerationStructuresPropertiesKHR");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordGetDeviceAccelerationStructureCompatibilityKHR(
    VkDevice                                    device,
    const VkAccelerationStructureVersionInfoKHR* pVersionInfo,
    VkAccelerationStructureCompatibilityKHR*    pCompatibility) {
    StartReadObjectParentInstance(device, "vkGetDeviceAccelerationStructureCompatibilityKHR");
}

void ThreadSafety::PostCallRecordGetDeviceAccelerationStructureCompatibilityKHR(
    VkDevice                                    device,
    const VkAccelerationStructureVersionInfoKHR* pVersionInfo,
    VkAccelerationStructureCompatibilityKHR*    pCompatibility) {
    FinishReadObjectParentInstance(device, "vkGetDeviceAccelerationStructureCompatibilityKHR");
}

void ThreadSafety::PreCallRecordGetAccelerationStructureBuildSizesKHR(
    VkDevice                                    device,
    VkAccelerationStructureBuildTypeKHR         buildType,
    const VkAccelerationStructureBuildGeometryInfoKHR* pBuildInfo,
    const uint32_t*                             pMaxPrimitiveCounts,
    VkAccelerationStructureBuildSizesInfoKHR*   pSizeInfo) {
    StartReadObjectParentInstance(device, "vkGetAccelerationStructureBuildSizesKHR");
}

void ThreadSafety::PostCallRecordGetAccelerationStructureBuildSizesKHR(
    VkDevice                                    device,
    VkAccelerationStructureBuildTypeKHR         buildType,
    const VkAccelerationStructureBuildGeometryInfoKHR* pBuildInfo,
    const uint32_t*                             pMaxPrimitiveCounts,
    VkAccelerationStructureBuildSizesInfoKHR*   pSizeInfo) {
    FinishReadObjectParentInstance(device, "vkGetAccelerationStructureBuildSizesKHR");
}

void ThreadSafety::PreCallRecordCmdTraceRaysKHR(
    VkCommandBuffer                             commandBuffer,
    const VkStridedDeviceAddressRegionKHR*      pRaygenShaderBindingTable,
    const VkStridedDeviceAddressRegionKHR*      pMissShaderBindingTable,
    const VkStridedDeviceAddressRegionKHR*      pHitShaderBindingTable,
    const VkStridedDeviceAddressRegionKHR*      pCallableShaderBindingTable,
    uint32_t                                    width,
    uint32_t                                    height,
    uint32_t                                    depth) {
    StartWriteObject(commandBuffer, "vkCmdTraceRaysKHR");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdTraceRaysKHR(
    VkCommandBuffer                             commandBuffer,
    const VkStridedDeviceAddressRegionKHR*      pRaygenShaderBindingTable,
    const VkStridedDeviceAddressRegionKHR*      pMissShaderBindingTable,
    const VkStridedDeviceAddressRegionKHR*      pHitShaderBindingTable,
    const VkStridedDeviceAddressRegionKHR*      pCallableShaderBindingTable,
    uint32_t                                    width,
    uint32_t                                    height,
    uint32_t                                    depth) {
    FinishWriteObject(commandBuffer, "vkCmdTraceRaysKHR");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordGetRayTracingCaptureReplayShaderGroupHandlesKHR(
    VkDevice                                    device,
    VkPipeline                                  pipeline,
    uint32_t                                    firstGroup,
    uint32_t                                    groupCount,
    size_t                                      dataSize,
    void*                                       pData) {
    StartReadObjectParentInstance(device, "vkGetRayTracingCaptureReplayShaderGroupHandlesKHR");
    StartReadObject(pipeline, "vkGetRayTracingCaptureReplayShaderGroupHandlesKHR");
}

void ThreadSafety::PostCallRecordGetRayTracingCaptureReplayShaderGroupHandlesKHR(
    VkDevice                                    device,
    VkPipeline                                  pipeline,
    uint32_t                                    firstGroup,
    uint32_t                                    groupCount,
    size_t                                      dataSize,
    void*                                       pData,
    VkResult                                    result) {
    FinishReadObjectParentInstance(device, "vkGetRayTracingCaptureReplayShaderGroupHandlesKHR");
    FinishReadObject(pipeline, "vkGetRayTracingCaptureReplayShaderGroupHandlesKHR");
}

void ThreadSafety::PreCallRecordCmdTraceRaysIndirectKHR(
    VkCommandBuffer                             commandBuffer,
    const VkStridedDeviceAddressRegionKHR*      pRaygenShaderBindingTable,
    const VkStridedDeviceAddressRegionKHR*      pMissShaderBindingTable,
    const VkStridedDeviceAddressRegionKHR*      pHitShaderBindingTable,
    const VkStridedDeviceAddressRegionKHR*      pCallableShaderBindingTable,
    VkDeviceAddress                             indirectDeviceAddress) {
    StartWriteObject(commandBuffer, "vkCmdTraceRaysIndirectKHR");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdTraceRaysIndirectKHR(
    VkCommandBuffer                             commandBuffer,
    const VkStridedDeviceAddressRegionKHR*      pRaygenShaderBindingTable,
    const VkStridedDeviceAddressRegionKHR*      pMissShaderBindingTable,
    const VkStridedDeviceAddressRegionKHR*      pHitShaderBindingTable,
    const VkStridedDeviceAddressRegionKHR*      pCallableShaderBindingTable,
    VkDeviceAddress                             indirectDeviceAddress) {
    FinishWriteObject(commandBuffer, "vkCmdTraceRaysIndirectKHR");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordGetRayTracingShaderGroupStackSizeKHR(
    VkDevice                                    device,
    VkPipeline                                  pipeline,
    uint32_t                                    group,
    VkShaderGroupShaderKHR                      groupShader) {
    StartReadObjectParentInstance(device, "vkGetRayTracingShaderGroupStackSizeKHR");
    StartReadObject(pipeline, "vkGetRayTracingShaderGroupStackSizeKHR");
}

void ThreadSafety::PostCallRecordGetRayTracingShaderGroupStackSizeKHR(
    VkDevice                                    device,
    VkPipeline                                  pipeline,
    uint32_t                                    group,
    VkShaderGroupShaderKHR                      groupShader) {
    FinishReadObjectParentInstance(device, "vkGetRayTracingShaderGroupStackSizeKHR");
    FinishReadObject(pipeline, "vkGetRayTracingShaderGroupStackSizeKHR");
}

void ThreadSafety::PreCallRecordCmdSetRayTracingPipelineStackSizeKHR(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    pipelineStackSize) {
    StartWriteObject(commandBuffer, "vkCmdSetRayTracingPipelineStackSizeKHR");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdSetRayTracingPipelineStackSizeKHR(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    pipelineStackSize) {
    FinishWriteObject(commandBuffer, "vkCmdSetRayTracingPipelineStackSizeKHR");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdDrawMeshTasksEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    groupCountX,
    uint32_t                                    groupCountY,
    uint32_t                                    groupCountZ) {
    StartWriteObject(commandBuffer, "vkCmdDrawMeshTasksEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdDrawMeshTasksEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    groupCountX,
    uint32_t                                    groupCountY,
    uint32_t                                    groupCountZ) {
    FinishWriteObject(commandBuffer, "vkCmdDrawMeshTasksEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdDrawMeshTasksIndirectEXT(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    uint32_t                                    drawCount,
    uint32_t                                    stride) {
    StartWriteObject(commandBuffer, "vkCmdDrawMeshTasksIndirectEXT");
    StartReadObject(buffer, "vkCmdDrawMeshTasksIndirectEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdDrawMeshTasksIndirectEXT(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    uint32_t                                    drawCount,
    uint32_t                                    stride) {
    FinishWriteObject(commandBuffer, "vkCmdDrawMeshTasksIndirectEXT");
    FinishReadObject(buffer, "vkCmdDrawMeshTasksIndirectEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PreCallRecordCmdDrawMeshTasksIndirectCountEXT(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkBuffer                                    countBuffer,
    VkDeviceSize                                countBufferOffset,
    uint32_t                                    maxDrawCount,
    uint32_t                                    stride) {
    StartWriteObject(commandBuffer, "vkCmdDrawMeshTasksIndirectCountEXT");
    StartReadObject(buffer, "vkCmdDrawMeshTasksIndirectCountEXT");
    StartReadObject(countBuffer, "vkCmdDrawMeshTasksIndirectCountEXT");
    // Host access to commandBuffer must be externally synchronized
}

void ThreadSafety::PostCallRecordCmdDrawMeshTasksIndirectCountEXT(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkBuffer                                    countBuffer,
    VkDeviceSize                                countBufferOffset,
    uint32_t                                    maxDrawCount,
    uint32_t                                    stride) {
    FinishWriteObject(commandBuffer, "vkCmdDrawMeshTasksIndirectCountEXT");
    FinishReadObject(buffer, "vkCmdDrawMeshTasksIndirectCountEXT");
    FinishReadObject(countBuffer, "vkCmdDrawMeshTasksIndirectCountEXT");
    // Host access to commandBuffer must be externally synchronized
}
