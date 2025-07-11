/* Copyright (c) 2015-2024 The Khronos Group Inc.
 * Copyright (C) 2025 Arm Limited.
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
#include <cstdint>
#include <unordered_set>
#include <iostream>

namespace stateless {

bool Device::manual_PreCallValidateCreateTensorARM(VkDevice device, const VkTensorCreateInfoARM *pCreateInfo,
                                                   const VkAllocationCallbacks *pAllocator, VkTensorARM *pTensor,
                                                   const Context &context) const {
    bool skip = false;

    if (!pCreateInfo) {
        return skip;
    }
    const Location create_info_loc = context.error_obj.location.dot(Field::pCreateInfo);
    if (pCreateInfo->sharingMode == VK_SHARING_MODE_CONCURRENT) {
        if (pCreateInfo->queueFamilyIndexCount <= 1) {
            skip |= LogError("VUID-VkTensorCreateInfoARM-sharingMode-09723", device, create_info_loc.dot(Field::sharingMode),
                             "is VK_SHARING_MODE_CONCURRENT, but queueFamilyIndexCount is %" PRIu32 ".",
                             pCreateInfo->queueFamilyIndexCount);
        }

        if (pCreateInfo->pQueueFamilyIndices == nullptr) {
            skip |= LogError("VUID-VkTensorCreateInfoARM-sharingMode-09722", device, create_info_loc.dot(Field::sharingMode),
                             "is VK_SHARING_MODE_CONCURRENT, but pQueueFamilyIndices is NULL.");
        }
    }
    const auto *opaque_capture = vku::FindStructInPNextChain<VkOpaqueCaptureDescriptorDataCreateInfoEXT>(pCreateInfo->pNext);
    if (opaque_capture) {
        if (!(pCreateInfo->flags & VK_TENSOR_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_ARM)) {
            skip |= LogError("VUID-VkTensorCreateInfoARM-pNext-09727", device, create_info_loc.dot(Field::pNext),
                             "includes a VkOpaqueCaptureDescriptorDataCreateInfoEXT structure, but flags (%s) is "
                             "missing VK_TENSOR_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_ARM",
                             string_VkTensorCreateFlagsARM(pCreateInfo->flags).c_str());
        }
    }

    auto *description = pCreateInfo->pDescription;
    auto desc_loc = create_info_loc.dot(Field::pDescription);

    if ((VK_TENSOR_TILING_OPTIMAL_ARM == description->tiling) && (nullptr != description->pStrides)) {
        skip |= LogError("VUID-VkTensorCreateInfoARM-pDescription-09720", device, desc_loc.dot(Field::tiling),
                         "is VK_TENSOR_TILING_OPTIMAL_ARM, but pDescription::pStrides (%p) is not null", description->pStrides);
    }
    {
        if (description->dimensionCount > phys_dev_ext_props.tensor_properties.maxTensorDimensionCount) {
            skip |= LogError("VUID-VkTensorDescriptionARM-dimensionCount-09733", device, desc_loc.dot(Field::dimensionCount),
                             "(%" PRIu32 ") must be less than or equal to "
                             "VkPhysicalDeviceTensorPropertiesARM::maxTensorDimensionCount (%" PRIu32 ")",
                             description->dimensionCount, phys_dev_ext_props.tensor_properties.maxTensorDimensionCount);
        }
    }
    {
        int64_t total_elements = 1;
        auto *dims = description->pDimensions;
        auto would_overflow = false;
        for (uint32_t i = 0; i < description->dimensionCount; i++) {
            if (INT64_MAX / total_elements >= dims[i]) {
                total_elements *= dims[i];
            } else {
                would_overflow = true;
            }
            if (dims[i] <= 0) {
                skip |= LogError("VUID-VkTensorDescriptionARM-pDimensions-09734", device, desc_loc.dot(Field::pDimensions, i),
                                 "(%" PRIi64 ") must be greater than 0.", dims[i]);
            }
        }
        if (static_cast<uint64_t>(total_elements) > phys_dev_ext_props.tensor_properties.maxTensorElements || would_overflow) {
            skip |= LogError("VUID-VkTensorCreateInfoARM-tensorElements-09721", device, desc_loc.dot(Field::pDimensions),
                             "tensorElements (%" PRIi64 ") > maxTensorElements (%" PRIu64 ")",
                             total_elements, phys_dev_ext_props.tensor_properties.maxTensorElements);
        }
    }
    {
        if (nullptr != description->pStrides) {
            for (uint32_t i = 0; i < description->dimensionCount; i++) {
                if (description->pStrides[i] <= 0 ||
                    description->pStrides[i] > phys_dev_ext_props.tensor_properties.maxTensorStride) {
                    skip |= LogError("VUID-VkTensorDescriptionARM-pStrides-09738", device, desc_loc.dot(Field::pStrides, i),
                                     "(%" PRIi64 ") > maxTensorStride (%" PRIu64 ")",
                                     description->pStrides[i], phys_dev_ext_props.tensor_properties.maxTensorStride);
                }
                if (i > 0) {
                    if (description->pStrides[i - 1] <
                        static_cast<int64_t>(description->pStrides[i] * description->pDimensions[i])) {
                        skip |= LogError(
                            "VUID-VkTensorDescriptionARM-pStrides-09739", device, desc_loc.dot(Field::pStrides, i-1),
                            "(%" PRIi64 ") < pStrides[%" PRIu32 "] (%" PRIi64 ") * pDimensions[%" PRIu32 "] (%" PRIu64 ")",
                            description->pStrides[i - 1], i, description->pStrides[i], i, description->pDimensions[i]);
                    }
                }
            }
        }
    }
    {
        if ((VK_TENSOR_TILING_OPTIMAL_ARM == description->tiling) &&
            ((VK_TENSOR_USAGE_IMAGE_ALIASING_BIT_ARM & description->usage) != 0)) {
            if (description->pDimensions[description->dimensionCount - 1] > 4) {
                skip |=
                    LogError("VUID-VkTensorDescriptionARM-tiling-09741", device, desc_loc.dot(Field::tiling),
                             "is VK_TENSOR_TILING_OPTIMAL_ARM and usage (%s) includes VK_TENSOR_USAGE_IMAGE_ALIASING_BIT_ARM "
                             "but pDimensions[%" PRIu32 "] (%" PRIu64 ") > 4", string_VkTensorUsageFlagsARM(description->usage).c_str(),
                             (description->dimensionCount - 1), description->pDimensions[description->dimensionCount - 1]);
            }
        }
    }
    {
        if ((VK_TENSOR_TILING_LINEAR_ARM == description->tiling) &&
            ((VK_TENSOR_USAGE_IMAGE_ALIASING_BIT_ARM & description->usage) != 0)) {
            skip |= LogError(
                "VUID-VkTensorDescriptionARM-tiling-09742", device, desc_loc.dot(Field::tiling),
                "is VK_TENSOR_TILING_LINEAR_ARM but usage (%s) includes VK_TENSOR_USAGE_IMAGE_ALIASING_BIT_ARM",
                string_VkTensorUsageFlagsARM(description->usage).c_str());
        }
    }

    if (auto external_memory_info = vku::FindStructInPNextChain<VkExternalMemoryTensorCreateInfoARM>(pCreateInfo->pNext)) {
        VkPhysicalDeviceExternalTensorInfoARM tensor_info = vku::InitStructHelper();
        tensor_info.pDescription = description;
        tensor_info.handleType = static_cast<VkExternalMemoryHandleTypeFlagBits>(external_memory_info->handleTypes);
        VkExternalTensorPropertiesARM properties = vku::InitStructHelper();
        dispatch_instance_->GetPhysicalDeviceExternalTensorPropertiesARM(physical_device, &tensor_info, &properties);
        const auto compatible_types = properties.externalMemoryProperties.compatibleHandleTypes;
        if ((external_memory_info->handleTypes & compatible_types) != external_memory_info->handleTypes) {
            skip |= LogError(
                "VUID-VkTensorCreateInfoARM-pNext-09864", device,
                create_info_loc.pNext(Struct::VkExternalMemoryTensorCreateInfoARM, Field::handleTypes),
                "(%s) is not reported as compatible by vkGetPhysicalDeviceExternalTensorPropertiesARM. Compatible types are: %s",
                string_VkExternalMemoryHandleTypeFlags(external_memory_info->handleTypes).c_str(),
                string_VkExternalMemoryHandleTypeFlags(compatible_types).c_str());
        }
    }

    return skip;
}

}  // namespace stateless
