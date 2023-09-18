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
#include "generated/enum_flag_bits.h"

bool StatelessValidation::manual_PreCallValidateCreateBuffer(VkDevice device, const VkBufferCreateInfo *pCreateInfo,
                                                             const VkAllocationCallbacks *pAllocator, VkBuffer *pBuffer,
                                                             const ErrorObject &error_obj) const {
    bool skip = false;

    if (!pCreateInfo) {
        return skip;
    }
    const Location create_info_loc = error_obj.location.dot(Field::pCreateInfo);
    skip |= ValidateNotZero(pCreateInfo->size == 0, "VUID-VkBufferCreateInfo-size-00912", create_info_loc.dot(Field::size));

    // Validation for parameters excluded from the generated validation code due to a 'noautovalidity' tag in vk.xml
    if (pCreateInfo->sharingMode == VK_SHARING_MODE_CONCURRENT) {
        // If sharingMode is VK_SHARING_MODE_CONCURRENT, queueFamilyIndexCount must be greater than 1
        if (pCreateInfo->queueFamilyIndexCount <= 1) {
            skip |= LogError("VUID-VkBufferCreateInfo-sharingMode-00914", device, create_info_loc.dot(Field::sharingMode),
                             "VK_SHARING_MODE_CONCURRENT, but queueFamilyIndexCount is %" PRIu32 ".",
                             pCreateInfo->queueFamilyIndexCount);
        }

        // If sharingMode is VK_SHARING_MODE_CONCURRENT, pQueueFamilyIndices must be a pointer to an array of
        // queueFamilyIndexCount uint32_t values
        if (pCreateInfo->pQueueFamilyIndices == nullptr) {
            skip |= LogError("VUID-VkBufferCreateInfo-sharingMode-00913", device, create_info_loc.dot(Field::sharingMode),
                             "is VK_SHARING_MODE_CONCURRENT, but pQueueFamilyIndices is NULL.");
        }
    }

    if ((pCreateInfo->flags & VK_BUFFER_CREATE_SPARSE_BINDING_BIT) && (!physical_device_features.sparseBinding)) {
        skip |= LogError("VUID-VkBufferCreateInfo-flags-00915", device, create_info_loc.dot(Field::flags),
                         "includes VK_BUFFER_CREATE_SPARSE_BINDING_BIT, but the sparseBinding feature is not enabled.");
    }

    if ((pCreateInfo->flags & VK_BUFFER_CREATE_SPARSE_RESIDENCY_BIT) && (!physical_device_features.sparseResidencyBuffer)) {
        skip |= LogError("VUID-VkBufferCreateInfo-flags-00916", device, create_info_loc.dot(Field::flags),
                         "includes VK_BUFFER_CREATE_SPARSE_RESIDENCY_BIT, but the sparseResidencyBuffer feature is not enabled.");
    }

    if ((pCreateInfo->flags & VK_BUFFER_CREATE_SPARSE_ALIASED_BIT) && (!physical_device_features.sparseResidencyAliased)) {
        skip |= LogError("VUID-VkBufferCreateInfo-flags-00917", device, create_info_loc.dot(Field::flags),
                         "includes VK_BUFFER_CREATE_SPARSE_ALIASED_BIT, but the sparseResidencyAliased feature is not enabled.");
    };

    // If flags contains VK_BUFFER_CREATE_SPARSE_RESIDENCY_BIT or VK_BUFFER_CREATE_SPARSE_ALIASED_BIT, it must also contain
    // VK_BUFFER_CREATE_SPARSE_BINDING_BIT
    if (((pCreateInfo->flags & (VK_BUFFER_CREATE_SPARSE_RESIDENCY_BIT | VK_BUFFER_CREATE_SPARSE_ALIASED_BIT)) != 0) &&
        ((pCreateInfo->flags & VK_BUFFER_CREATE_SPARSE_BINDING_BIT) != VK_BUFFER_CREATE_SPARSE_BINDING_BIT)) {
        skip |= LogError("VUID-VkBufferCreateInfo-flags-00918", device, create_info_loc.dot(Field::flags), "is %s.",
                         string_VkBufferCreateFlags(pCreateInfo->flags).c_str());
    }

    const auto *maintenance4_features = vku::FindStructInPNextChain<VkPhysicalDeviceMaintenance4FeaturesKHR>(device_createinfo_pnext);
    if (maintenance4_features && maintenance4_features->maintenance4) {
        if (pCreateInfo->size > phys_dev_ext_props.maintenance4_props.maxBufferSize) {
            skip |= LogError("VUID-VkBufferCreateInfo-size-06409", device, create_info_loc.dot(Field::size),
                             "(%" PRIu64
                             ") is larger than the maximum allowed buffer size "
                             "VkPhysicalDeviceMaintenance4Properties.maxBufferSize (%" PRIu64 ").",
                             pCreateInfo->size, phys_dev_ext_props.maintenance4_props.maxBufferSize);
        }
    }

    if (!vku::FindStructInPNextChain<VkBufferUsageFlags2CreateInfoKHR>(pCreateInfo->pNext)) {
        skip |=
            ValidateFlags(create_info_loc.dot(Field::usage), "VkBufferUsageFlagBits", AllVkBufferUsageFlagBits, pCreateInfo->usage,
                          kRequiredFlags, "VUID-VkBufferCreateInfo-None-09205", "VUID-VkBufferCreateInfo-None-09206");
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCreateBufferView(VkDevice device, const VkBufferViewCreateInfo *pCreateInfo,
                                                                 const VkAllocationCallbacks *pAllocator, VkBufferView *pBufferView,
                                                                 const ErrorObject &error_obj) const {
    bool skip = false;
#ifdef VK_USE_PLATFORM_METAL_EXT
    skip |=
        ExportMetalObjectsPNextUtil(VK_EXPORT_METAL_OBJECT_TYPE_METAL_TEXTURE_BIT_EXT, "VUID-VkBufferViewCreateInfo-pNext-06782",
                                    error_obj.location, "VK_EXPORT_METAL_OBJECT_TYPE_METAL_TEXTURE_BIT_EXT", pCreateInfo->pNext);
#endif  // VK_USE_PLATFORM_METAL_EXT
    return skip;
}
