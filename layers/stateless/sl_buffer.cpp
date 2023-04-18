/* Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
 * Copyright (C) 2015-2023 Google Inc.
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

#include "stateless/stateless_validation.h"

bool StatelessValidation::manual_PreCallValidateCreateBuffer(VkDevice device, const VkBufferCreateInfo *pCreateInfo,
                                                             const VkAllocationCallbacks *pAllocator, VkBuffer *pBuffer) const {
    bool skip = false;

    if (pCreateInfo != nullptr) {
        skip |=
            ValidateGreaterThanZero(pCreateInfo->size, "pCreateInfo->size", "VUID-VkBufferCreateInfo-size-00912", "vkCreateBuffer");

        // Validation for parameters excluded from the generated validation code due to a 'noautovalidity' tag in vk.xml
        if (pCreateInfo->sharingMode == VK_SHARING_MODE_CONCURRENT) {
            // If sharingMode is VK_SHARING_MODE_CONCURRENT, queueFamilyIndexCount must be greater than 1
            if (pCreateInfo->queueFamilyIndexCount <= 1) {
                skip |= LogError(device, "VUID-VkBufferCreateInfo-sharingMode-00914",
                                 "vkCreateBuffer: if pCreateInfo->sharingMode is VK_SHARING_MODE_CONCURRENT, "
                                 "pCreateInfo->queueFamilyIndexCount must be greater than 1.");
            }

            // If sharingMode is VK_SHARING_MODE_CONCURRENT, pQueueFamilyIndices must be a pointer to an array of
            // queueFamilyIndexCount uint32_t values
            if (pCreateInfo->pQueueFamilyIndices == nullptr) {
                skip |= LogError(device, "VUID-VkBufferCreateInfo-sharingMode-00913",
                                 "vkCreateBuffer: if pCreateInfo->sharingMode is VK_SHARING_MODE_CONCURRENT, "
                                 "pCreateInfo->pQueueFamilyIndices must be a pointer to an array of "
                                 "pCreateInfo->queueFamilyIndexCount uint32_t values.");
            }
        }

        if ((pCreateInfo->flags & VK_BUFFER_CREATE_SPARSE_BINDING_BIT) && (!physical_device_features.sparseBinding)) {
            skip |= LogError(device, "VUID-VkBufferCreateInfo-flags-00915",
                             "vkCreateBuffer(): the sparseBinding device feature is disabled: Buffers cannot be created with the "
                             "VK_BUFFER_CREATE_SPARSE_BINDING_BIT set.");
        }

        if ((pCreateInfo->flags & VK_BUFFER_CREATE_SPARSE_RESIDENCY_BIT) && (!physical_device_features.sparseResidencyBuffer)) {
            skip |=
                LogError(device, "VUID-VkBufferCreateInfo-flags-00916",
                         "vkCreateBuffer(): the sparseResidencyBuffer device feature is disabled: Buffers cannot be created with "
                         "the VK_BUFFER_CREATE_SPARSE_RESIDENCY_BIT set.");
        }

        if ((pCreateInfo->flags & VK_BUFFER_CREATE_SPARSE_ALIASED_BIT) && (!physical_device_features.sparseResidencyAliased)) {
            skip |=
                LogError(device, "VUID-VkBufferCreateInfo-flags-00917",
                         "vkCreateBuffer(): the sparseResidencyAliased device feature is disabled: Buffers cannot be created with "
                         "the VK_BUFFER_CREATE_SPARSE_ALIASED_BIT set.");
        }

        // If flags contains VK_BUFFER_CREATE_SPARSE_RESIDENCY_BIT or VK_BUFFER_CREATE_SPARSE_ALIASED_BIT, it must also contain
        // VK_BUFFER_CREATE_SPARSE_BINDING_BIT
        if (((pCreateInfo->flags & (VK_BUFFER_CREATE_SPARSE_RESIDENCY_BIT | VK_BUFFER_CREATE_SPARSE_ALIASED_BIT)) != 0) &&
            ((pCreateInfo->flags & VK_BUFFER_CREATE_SPARSE_BINDING_BIT) != VK_BUFFER_CREATE_SPARSE_BINDING_BIT)) {
            skip |= LogError(device, "VUID-VkBufferCreateInfo-flags-00918",
                             "vkCreateBuffer: if pCreateInfo->flags contains VK_BUFFER_CREATE_SPARSE_RESIDENCY_BIT or "
                             "VK_BUFFER_CREATE_SPARSE_ALIASED_BIT, it must also contain VK_BUFFER_CREATE_SPARSE_BINDING_BIT.");
        }

        const auto *maintenance4_features = LvlFindInChain<VkPhysicalDeviceMaintenance4FeaturesKHR>(device_createinfo_pnext);
        if (maintenance4_features && maintenance4_features->maintenance4) {
            if (pCreateInfo->size > phys_dev_ext_props.maintenance4_props.maxBufferSize) {
                skip |= LogError(device, "VUID-VkBufferCreateInfo-size-06409",
                                 "vkCreateBuffer: pCreateInfo->size is larger than the maximum allowed buffer size "
                                 "VkPhysicalDeviceMaintenance4Properties.maxBufferSize");
            }
        }
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCreateBufferView(VkDevice device, const VkBufferViewCreateInfo *pCreateInfo,
                                                                 const VkAllocationCallbacks *pAllocator,
                                                                 VkBufferView *pBufferView) const {
    bool skip = false;
#ifdef VK_USE_PLATFORM_METAL_EXT
    skip |= ExportMetalObjectsPNextUtil(
        VK_EXPORT_METAL_OBJECT_TYPE_METAL_TEXTURE_BIT_EXT, "VUID-VkBufferViewCreateInfo-pNext-06782",
        "vkCreateBufferView():", "VK_EXPORT_METAL_OBJECT_TYPE_METAL_TEXTURE_BIT_EXT", pCreateInfo->pNext);
#endif  // VK_USE_PLATFORM_METAL_EXT
    return skip;
}
