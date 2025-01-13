// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See layer_chassis_generator.py for modifications

/***************************************************************************
 *
 * Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (c) 2015-2024 Google Inc.
 * Copyright (c) 2023-2024 RasterGrid Kft.
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

#include <array>
#include <cstring>
#include <mutex>

#include "chassis/validation_object.h"

thread_local WriteLockGuard* ValidationObject::record_guard{};

void ValidationObject::DispatchGetPhysicalDeviceFeatures2Helper(VkPhysicalDevice physicalDevice,
                                                                VkPhysicalDeviceFeatures2* pFeatures) const {
    if (api_version >= VK_API_VERSION_1_1) {
        return dispatch_instance_->GetPhysicalDeviceFeatures2(physicalDevice, pFeatures);
    } else {
        return dispatch_instance_->GetPhysicalDeviceFeatures2KHR(physicalDevice, pFeatures);
    }
}

void ValidationObject::DispatchGetPhysicalDeviceProperties2Helper(VkPhysicalDevice physicalDevice,
                                                                  VkPhysicalDeviceProperties2* pProperties) const {
    if (api_version >= VK_API_VERSION_1_1) {
        return dispatch_instance_->GetPhysicalDeviceProperties2(physicalDevice, pProperties);
    } else {
        return dispatch_instance_->GetPhysicalDeviceProperties2KHR(physicalDevice, pProperties);
    }
}

void ValidationObject::DispatchGetPhysicalDeviceFormatProperties2Helper(VkPhysicalDevice physicalDevice, VkFormat format,
                                                                        VkFormatProperties2* pFormatProperties) const {
    if (api_version >= VK_API_VERSION_1_1) {
        return dispatch_instance_->GetPhysicalDeviceFormatProperties2(physicalDevice, format, pFormatProperties);
    } else {
        return dispatch_instance_->GetPhysicalDeviceFormatProperties2KHR(physicalDevice, format, pFormatProperties);
    }
}

VkResult ValidationObject::DispatchGetPhysicalDeviceImageFormatProperties2Helper(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceImageFormatInfo2* pImageFormatInfo,
    VkImageFormatProperties2* pImageFormatProperties) const {
    if (api_version >= VK_API_VERSION_1_1) {
        return dispatch_instance_->GetPhysicalDeviceImageFormatProperties2(physicalDevice, pImageFormatInfo,
                                                                           pImageFormatProperties);
    } else {
        return dispatch_instance_->GetPhysicalDeviceImageFormatProperties2KHR(physicalDevice, pImageFormatInfo,
                                                                              pImageFormatProperties);
    }
}

void ValidationObject::DispatchGetPhysicalDeviceQueueFamilyProperties2Helper(
    VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount, VkQueueFamilyProperties2* pQueueFamilyProperties) const {
    if (api_version >= VK_API_VERSION_1_1) {
        return dispatch_instance_->GetPhysicalDeviceQueueFamilyProperties2(physicalDevice, pQueueFamilyPropertyCount,
                                                                           pQueueFamilyProperties);
    } else {
        return dispatch_instance_->GetPhysicalDeviceQueueFamilyProperties2KHR(physicalDevice, pQueueFamilyPropertyCount,
                                                                              pQueueFamilyProperties);
    }
}

void ValidationObject::DispatchGetPhysicalDeviceMemoryProperties2Helper(
    VkPhysicalDevice physicalDevice, VkPhysicalDeviceMemoryProperties2* pMemoryProperties) const {
    if (api_version >= VK_API_VERSION_1_1) {
        return dispatch_instance_->GetPhysicalDeviceMemoryProperties2(physicalDevice, pMemoryProperties);
    } else {
        return dispatch_instance_->GetPhysicalDeviceMemoryProperties2KHR(physicalDevice, pMemoryProperties);
    }
}

void ValidationObject::DispatchGetPhysicalDeviceSparseImageFormatProperties2Helper(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSparseImageFormatInfo2* pFormatInfo, uint32_t* pPropertyCount,
    VkSparseImageFormatProperties2* pProperties) const {
    if (api_version >= VK_API_VERSION_1_1) {
        return dispatch_instance_->GetPhysicalDeviceSparseImageFormatProperties2(physicalDevice, pFormatInfo, pPropertyCount,
                                                                                 pProperties);
    } else {
        return dispatch_instance_->GetPhysicalDeviceSparseImageFormatProperties2KHR(physicalDevice, pFormatInfo, pPropertyCount,
                                                                                    pProperties);
    }
}

void ValidationObject::DispatchGetPhysicalDeviceExternalSemaphorePropertiesHelper(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalSemaphoreInfo* pExternalSemaphoreInfo,
    VkExternalSemaphoreProperties* pExternalSemaphoreProperties) const {
    if (api_version >= VK_API_VERSION_1_1) {
        return dispatch_instance_->GetPhysicalDeviceExternalSemaphoreProperties(physicalDevice, pExternalSemaphoreInfo,
                                                                                pExternalSemaphoreProperties);
    } else {
        return dispatch_instance_->GetPhysicalDeviceExternalSemaphorePropertiesKHR(physicalDevice, pExternalSemaphoreInfo,
                                                                                   pExternalSemaphoreProperties);
    }
}

void ValidationObject::DispatchGetPhysicalDeviceExternalFencePropertiesHelper(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalFenceInfo* pExternalFenceInfo,
    VkExternalFenceProperties* pExternalFenceProperties) const {
    if (api_version >= VK_API_VERSION_1_1) {
        return dispatch_instance_->GetPhysicalDeviceExternalFenceProperties(physicalDevice, pExternalFenceInfo,
                                                                            pExternalFenceProperties);
    } else {
        return dispatch_instance_->GetPhysicalDeviceExternalFencePropertiesKHR(physicalDevice, pExternalFenceInfo,
                                                                               pExternalFenceProperties);
    }
}

void ValidationObject::DispatchGetPhysicalDeviceExternalBufferPropertiesHelper(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalBufferInfo* pExternalBufferInfo,
    VkExternalBufferProperties* pExternalBufferProperties) const {
    if (api_version >= VK_API_VERSION_1_1) {
        return dispatch_instance_->GetPhysicalDeviceExternalBufferProperties(physicalDevice, pExternalBufferInfo,
                                                                             pExternalBufferProperties);
    } else {
        return dispatch_instance_->GetPhysicalDeviceExternalBufferPropertiesKHR(physicalDevice, pExternalBufferInfo,
                                                                                pExternalBufferProperties);
    }
}

// NOLINTEND
