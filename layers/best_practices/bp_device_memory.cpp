/* Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
 * Modifications Copyright (C) 2020 Advanced Micro Devices, Inc. All rights reserved.
 * Modifications Copyright (C) 2022 RasterGrid Kft.
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

#include "best_practices/best_practices_validation.h"
#include "best_practices/best_practices_error_enums.h"

void BestPractices::PreCallRecordAllocateMemory(VkDevice device, const VkMemoryAllocateInfo* pAllocateInfo,
                                                const VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory) {
    if (VendorCheckEnabled(kBPVendorNVIDIA)) {
        WriteLockGuard guard{memory_free_events_lock_};

        // Release old allocations to avoid overpopulating the container
        const auto now = std::chrono::high_resolution_clock::now();
        const auto last_old = std::find_if(
            memory_free_events_.rbegin(), memory_free_events_.rend(),
            [now](const MemoryFreeEvent& event) { return now - event.time > kAllocateMemoryReuseTimeThresholdNVIDIA; });
        memory_free_events_.erase(memory_free_events_.begin(), last_old.base());
    }
}

bool BestPractices::PreCallValidateAllocateMemory(VkDevice device, const VkMemoryAllocateInfo* pAllocateInfo,
                                                  const VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory) const {
    bool skip = false;

    if ((Count<DEVICE_MEMORY_STATE>() + 1) > kMemoryObjectWarningLimit) {
        skip |= LogPerformanceWarning(device, kVUID_BestPractices_AllocateMemory_TooManyObjects,
                                      "Performance Warning: This app has > %" PRIu32 " memory objects.", kMemoryObjectWarningLimit);
    }

    if (pAllocateInfo->allocationSize < kMinDeviceAllocationSize) {
        skip |= LogPerformanceWarning(device, kVUID_BestPractices_AllocateMemory_SmallAllocation,
                                      "vkAllocateMemory(): Allocating a VkDeviceMemory of size %" PRIu64
                                      ". This is a very small allocation (current "
                                      "threshold is %" PRIu64
                                      " bytes). "
                                      "You should make large allocations and sub-allocate from one large VkDeviceMemory.",
                                      pAllocateInfo->allocationSize, kMinDeviceAllocationSize);
    }

    if (VendorCheckEnabled(kBPVendorNVIDIA)) {
        if (!IsExtEnabled(device_extensions.vk_ext_pageable_device_local_memory) &&
            !LvlFindInChain<VkMemoryPriorityAllocateInfoEXT>(pAllocateInfo->pNext)) {
            skip |= LogPerformanceWarning(
                device, kVUID_BestPractices_AllocateMemory_SetPriority,
                "%s Use VkMemoryPriorityAllocateInfoEXT to provide the operating system information on the allocations that "
                "should stay in video memory and which should be demoted first when video memory is limited. "
                "The highest priority should be given to GPU-written resources like color attachments, depth attachments, "
                "storage images, and buffers written from the GPU.",
                VendorSpecificTag(kBPVendorNVIDIA));
        }

        {
            // Size in bytes for an allocation to be considered "compatible"
            static constexpr VkDeviceSize size_threshold = VkDeviceSize{1} << 20;

            ReadLockGuard guard{memory_free_events_lock_};

            const auto now = std::chrono::high_resolution_clock::now();
            const VkDeviceSize alloc_size = pAllocateInfo->allocationSize;
            const uint32_t memory_type_index = pAllocateInfo->memoryTypeIndex;
            const auto latest_event =
                std::find_if(memory_free_events_.rbegin(), memory_free_events_.rend(), [&](const MemoryFreeEvent& event) {
                    return (memory_type_index == event.memory_type_index) && (alloc_size <= event.allocation_size) &&
                           (alloc_size - event.allocation_size <= size_threshold) &&
                           (now - event.time < kAllocateMemoryReuseTimeThresholdNVIDIA);
                });

            if (latest_event != memory_free_events_.rend()) {
                const auto time_delta = std::chrono::duration_cast<std::chrono::milliseconds>(now - latest_event->time);
                if (time_delta < std::chrono::milliseconds{5}) {
                    skip |= LogPerformanceWarning(
                        device, kVUID_BestPractices_AllocateMemory_ReuseAllocations,
                        "%s Reuse memory allocations instead of releasing and reallocating. A memory allocation "
                        "has just been released, and it could have been reused in place of this allocation.",
                        VendorSpecificTag(kBPVendorNVIDIA));
                } else {
                    const uint32_t seconds = static_cast<uint32_t>(time_delta.count() / 1000);
                    const uint32_t milliseconds = static_cast<uint32_t>(time_delta.count() % 1000);

                    skip |= LogPerformanceWarning(
                        device, kVUID_BestPractices_AllocateMemory_ReuseAllocations,
                        "%s Reuse memory allocations instead of releasing and reallocating. A memory allocation has been released "
                        "%" PRIu32 ".%03" PRIu32 " seconds ago, and it could have been reused in place of this allocation.",
                        VendorSpecificTag(kBPVendorNVIDIA), seconds, milliseconds);
                }
            }
        }
    }

    // TODO: Insert get check for GetPhysicalDeviceMemoryProperties once the state is tracked in the StateTracker

    return skip;
}

void BestPractices::PreCallRecordFreeMemory(VkDevice device, VkDeviceMemory memory, const VkAllocationCallbacks* pAllocator) {
    if (memory != VK_NULL_HANDLE && VendorCheckEnabled(kBPVendorNVIDIA)) {
        auto mem_info = Get<DEVICE_MEMORY_STATE>(memory);

        // Exclude memory free events on dedicated allocations, or imported/exported allocations.
        if (!mem_info->IsDedicatedBuffer() && !mem_info->IsDedicatedImage() && !mem_info->IsExport() && !mem_info->IsImport()) {
            MemoryFreeEvent event;
            event.time = std::chrono::high_resolution_clock::now();
            event.memory_type_index = mem_info->alloc_info.memoryTypeIndex;
            event.allocation_size = mem_info->alloc_info.allocationSize;

            WriteLockGuard guard{memory_free_events_lock_};
            memory_free_events_.push_back(event);
        }
    }

    ValidationStateTracker::PreCallRecordFreeMemory(device, memory, pAllocator);
}

bool BestPractices::PreCallValidateFreeMemory(VkDevice device, VkDeviceMemory memory,
                                              const VkAllocationCallbacks* pAllocator) const {
    if (memory == VK_NULL_HANDLE) return false;
    bool skip = false;

    auto mem_info = Get<DEVICE_MEMORY_STATE>(memory);

    for (const auto& item : mem_info->ObjectBindings()) {
        const auto& obj = item.first;
        const LogObjectList objlist(device, obj, mem_info->mem());
        skip |= LogWarning(objlist, layer_name.c_str(), "VK Object %s still has a reference to mem obj %s.",
                           report_data->FormatHandle(obj).c_str(), report_data->FormatHandle(mem_info->mem()).c_str());
    }

    return skip;
}

bool BestPractices::ValidateBindBufferMemory(VkBuffer buffer, VkDeviceMemory memory, const char* api_name) const {
    bool skip = false;
    auto buffer_state = Get<BUFFER_STATE>(buffer);
    auto mem_state = Get<DEVICE_MEMORY_STATE>(memory);

    if (mem_state && mem_state->alloc_info.allocationSize == buffer_state->createInfo.size &&
        mem_state->alloc_info.allocationSize < kMinDedicatedAllocationSize) {
        skip |= LogPerformanceWarning(device, kVUID_BestPractices_SmallDedicatedAllocation,
                                      "%s: Trying to bind %s to a memory block which is fully consumed by the buffer. "
                                      "The required size of the allocation is %" PRIu64
                                      ", but smaller buffers like this should be sub-allocated from "
                                      "larger memory blocks. (Current threshold is %" PRIu64 " bytes.)",
                                      api_name, report_data->FormatHandle(buffer).c_str(), mem_state->alloc_info.allocationSize,
                                      kMinDedicatedAllocationSize);
    }

    skip |= ValidateBindMemory(device, memory);

    return skip;
}

bool BestPractices::PreCallValidateBindBufferMemory(VkDevice device, VkBuffer buffer, VkDeviceMemory memory,
                                                    VkDeviceSize memoryOffset) const {
    bool skip = false;
    const char* api_name = "BindBufferMemory()";

    skip |= ValidateBindBufferMemory(buffer, memory, api_name);

    return skip;
}

bool BestPractices::PreCallValidateBindBufferMemory2(VkDevice device, uint32_t bindInfoCount,
                                                     const VkBindBufferMemoryInfo* pBindInfos) const {
    char api_name[64];
    bool skip = false;

    for (uint32_t i = 0; i < bindInfoCount; i++) {
        snprintf(api_name, sizeof(api_name), "vkBindBufferMemory2() pBindInfos[%u]", i);
        skip |= ValidateBindBufferMemory(pBindInfos[i].buffer, pBindInfos[i].memory, api_name);
    }

    return skip;
}

bool BestPractices::PreCallValidateBindBufferMemory2KHR(VkDevice device, uint32_t bindInfoCount,
                                                        const VkBindBufferMemoryInfo* pBindInfos) const {
    char api_name[64];
    bool skip = false;

    for (uint32_t i = 0; i < bindInfoCount; i++) {
        snprintf(api_name, sizeof(api_name), "vkBindBufferMemory2KHR() pBindInfos[%u]", i);
        skip |= ValidateBindBufferMemory(pBindInfos[i].buffer, pBindInfos[i].memory, api_name);
    }

    return skip;
}

bool BestPractices::ValidateBindImageMemory(VkImage image, VkDeviceMemory memory, const char* api_name) const {
    bool skip = false;
    auto image_state = Get<IMAGE_STATE>(image);
    auto mem_state = Get<DEVICE_MEMORY_STATE>(memory);

    if (mem_state->alloc_info.allocationSize == image_state->requirements[0].size &&
        mem_state->alloc_info.allocationSize < kMinDedicatedAllocationSize) {
        skip |= LogPerformanceWarning(device, kVUID_BestPractices_SmallDedicatedAllocation,
                                      "%s: Trying to bind %s to a memory block which is fully consumed by the image. "
                                      "The required size of the allocation is %" PRIu64
                                      ", but smaller images like this should be sub-allocated from "
                                      "larger memory blocks. (Current threshold is %" PRIu64 " bytes.)",
                                      api_name, report_data->FormatHandle(image).c_str(), mem_state->alloc_info.allocationSize,
                                      kMinDedicatedAllocationSize);
    }

    // If we're binding memory to a image which was created as TRANSIENT and the image supports LAZY allocation,
    // make sure this type is actually used.
    // This warning will only trigger if this layer is run on a platform that supports LAZILY_ALLOCATED_BIT
    // (i.e.most tile - based renderers)
    if (image_state->createInfo.usage & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT) {
        bool supports_lazy = false;
        uint32_t suggested_type = 0;

        for (uint32_t i = 0; i < phys_dev_mem_props.memoryTypeCount; i++) {
            if ((1u << i) & image_state->requirements[0].memoryTypeBits) {
                if (phys_dev_mem_props.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT) {
                    supports_lazy = true;
                    suggested_type = i;
                    break;
                }
            }
        }

        uint32_t allocated_properties = phys_dev_mem_props.memoryTypes[mem_state->alloc_info.memoryTypeIndex].propertyFlags;

        if (supports_lazy && (allocated_properties & VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT) == 0) {
            skip |= LogPerformanceWarning(
                device, kVUID_BestPractices_NonLazyTransientImage,
                "%s: Attempting to bind memory type %u to VkImage which was created with TRANSIENT_ATTACHMENT_BIT,"
                "but this memory type is not LAZILY_ALLOCATED_BIT. You should use memory type %u here instead to save "
                "%" PRIu64 " bytes of physical memory.",
                api_name, mem_state->alloc_info.memoryTypeIndex, suggested_type, image_state->requirements[0].size);
        }
    }

    skip |= ValidateBindMemory(device, memory);

    return skip;
}

bool BestPractices::PreCallValidateBindImageMemory(VkDevice device, VkImage image, VkDeviceMemory memory,
                                                   VkDeviceSize memoryOffset) const {
    bool skip = false;
    const char* api_name = "vkBindImageMemory()";

    skip |= ValidateBindImageMemory(image, memory, api_name);

    return skip;
}

bool BestPractices::PreCallValidateBindImageMemory2(VkDevice device, uint32_t bindInfoCount,
                                                    const VkBindImageMemoryInfo* pBindInfos) const {
    char api_name[64];
    bool skip = false;

    for (uint32_t i = 0; i < bindInfoCount; i++) {
        snprintf(api_name, sizeof(api_name), "vkBindImageMemory2() pBindInfos[%u]", i);
        if (!LvlFindInChain<VkBindImageMemorySwapchainInfoKHR>(pBindInfos[i].pNext)) {
            skip |= ValidateBindImageMemory(pBindInfos[i].image, pBindInfos[i].memory, api_name);
        }
    }

    return skip;
}

bool BestPractices::PreCallValidateBindImageMemory2KHR(VkDevice device, uint32_t bindInfoCount,
                                                       const VkBindImageMemoryInfo* pBindInfos) const {
    char api_name[64];
    bool skip = false;

    for (uint32_t i = 0; i < bindInfoCount; i++) {
        snprintf(api_name, sizeof(api_name), "vkBindImageMemory2KHR() pBindInfos[%u]", i);
        skip |= ValidateBindImageMemory(pBindInfos[i].image, pBindInfos[i].memory, api_name);
    }

    return skip;
}

void BestPractices::PreCallRecordSetDeviceMemoryPriorityEXT(VkDevice device, VkDeviceMemory memory, float priority) {
    auto mem_info = std::static_pointer_cast<bp_state::DeviceMemory>(Get<DEVICE_MEMORY_STATE>(memory));
    mem_info->dynamic_priority.emplace(priority);
}

bool BestPractices::ValidateBindMemory(VkDevice device, VkDeviceMemory memory) const {
    bool skip = false;

    if (VendorCheckEnabled(kBPVendorNVIDIA) && IsExtEnabled(device_extensions.vk_ext_pageable_device_local_memory)) {
        auto mem_info = std::static_pointer_cast<const bp_state::DeviceMemory>(Get<DEVICE_MEMORY_STATE>(memory));
        if (!mem_info->dynamic_priority) {
            skip |=
                LogPerformanceWarning(device, kVUID_BestPractices_BindMemory_NoPriority,
                                      "%s Use vkSetDeviceMemoryPriorityEXT to provide the OS with information on which allocations "
                                      "should stay in memory and which should be demoted first when video memory is limited. The "
                                      "highest priority should be given to GPU-written resources like color attachments, depth "
                                      "attachments, storage images, and buffers written from the GPU.",
                                      VendorSpecificTag(kBPVendorNVIDIA));
        }
    }

    return skip;
}
