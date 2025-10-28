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

#include <vulkan/utility/vk_format_utils.h>
#include "stateless/stateless_validation.h"
#include <cstdint>

namespace stateless {

bool Device::ValidateTensorDescriptionARM(const VkTensorDescriptionARM &description, const Location &description_loc) const {
    bool skip = false;

    if (description.format == VK_FORMAT_UNDEFINED) {
        skip |= LogError("VUID-VkTensorDescriptionARM-format-09735", device, description_loc.dot(Field::format),
                         "is VK_FORMAT_UNDEFINED");
        return skip;
    } else {
        const uint32_t component_count = vkuFormatComponentCount(description.format);
        if (component_count != 1) {
            skip |= LogError("VUID-VkTensorDescriptionARM-format-09735", device, description_loc.dot(Field::format),
                             "(%s) has (%" PRIu32 ") components.", string_VkFormat(description.format), component_count);
        }
    }

    // pStrides can be null
    if (const auto *strides = description.pStrides) {
        if (description.tiling == VK_TENSOR_TILING_OPTIMAL_ARM) {
            skip |= LogError("VUID-VkTensorCreateInfoARM-pDescription-09720", device, description_loc.dot(Field::tiling),
                             "is VK_TENSOR_TILING_OPTIMAL_ARM, but pDescription::pStrides (%p) is not null", description.pStrides);
        }

        const uint32_t texel_block_size = vkuFormatTexelBlockSize(description.format);
        if (strides[description.dimensionCount - 1] != texel_block_size) {
            skip |= LogError("VUID-VkTensorDescriptionARM-pStrides-09736", device,
                             description_loc.dot(Field::pStrides, description.dimensionCount - 1),
                             "(%" PRIi64 ") must equal the size in bytes of a tensor element (%" PRIu32 ")",
                             strides[description.dimensionCount - 1], texel_block_size);
        }
        if (static_cast<uint64_t>(strides[0]) * static_cast<uint64_t>(description.pDimensions[0]) >
            phys_dev_ext_props.tensor_properties.maxTensorSize) {
            skip |= LogError("VUID-VkTensorDescriptionARM-pStrides-09884", device, description_loc.dot(Field::pStrides, 0),
                             "(%" PRId64 ") x pDimensions[0] (%" PRId64
                             ") is greater than VkPhysicalDeviceTensorPropertiesARM::maxTensorSize (%" PRIu64 ").",
                             strides[0], description.pDimensions[0], phys_dev_ext_props.tensor_properties.maxTensorSize);
        }
        for (uint32_t i = 0; i < description.dimensionCount; i++) {
            if ((strides[i] % texel_block_size) != 0) {
                skip |=
                    LogError("VUID-VkTensorDescriptionARM-pStrides-09737", device, description_loc.dot(Field::pStrides, i),
                             "(%" PRIi64 ") must be a multiple of the element size (%" PRIu32 ")", strides[i], texel_block_size);
            }

            if (strides[i] <= 0 || strides[i] > phys_dev_ext_props.tensor_properties.maxTensorStride) {
                skip |= LogError("VUID-VkTensorDescriptionARM-pStrides-09738", device, description_loc.dot(Field::pStrides, i),
                                 "(%" PRIi64 ") is <= 0 or > maxTensorStride (%" PRIu64 ")", strides[i],
                                 phys_dev_ext_props.tensor_properties.maxTensorStride);
            }

            if (i > 0) {
                if (strides[i - 1] < static_cast<int64_t>(strides[i] * description.pDimensions[i])) {
                    skip |=
                        LogError("VUID-VkTensorDescriptionARM-pStrides-09739", device, description_loc.dot(Field::pStrides, i - 1),
                                 "(%" PRIi64 ") < pStrides[%" PRIu32 "] (%" PRIi64 ") * pDimensions[%" PRIu32 "] (%" PRIu64 ")",
                                 strides[i - 1], i, strides[i], i, description.pDimensions[i]);
                }
            }
        }

        if (!enabled_features.tensorNonPacked) {
            int64_t i = description.dimensionCount - 1;
            bool is_packed = strides[i] == vkuFormatTexelBlockSize(description.format);
            while (is_packed && i > 0) {
                if (strides[i - 1] != static_cast<int64_t>(strides[i] * description.pDimensions[i])) {
                    is_packed = false;
                    break;
                }
                i--;
            }
            // if other errors above occured, i might be zero, but another error will be reported already
            if (!is_packed && i > 0) {
                skip |= LogError("VUID-VkTensorDescriptionARM-None-09740", device, description_loc,
                                 "does not define a packed tensor: pStrides[%" PRIi64 "] (%" PRIi64 ") != pStrides[%" PRIi64
                                 "] (%" PRIi64 ") * pDimensions[%" PRIi64 "] (%" PRIi64 ")",
                                 i - 1, strides[i - 1], i, strides[i], i, description.pDimensions[i]);
            }
        }
    }

    if (description.dimensionCount > phys_dev_ext_props.tensor_properties.maxTensorDimensionCount) {
        skip |= LogError("VUID-VkTensorDescriptionARM-dimensionCount-09733", device, description_loc.dot(Field::dimensionCount),
                         "(%" PRIu32
                         ") must be less than or equal to "
                         "VkPhysicalDeviceTensorPropertiesARM::maxTensorDimensionCount (%" PRIu32 ")",
                         description.dimensionCount, phys_dev_ext_props.tensor_properties.maxTensorDimensionCount);
    }

    {
        int64_t total_elements = 1;
        auto *dims = description.pDimensions;
        auto would_overflow = false;
        for (uint32_t i = 0; i < description.dimensionCount; i++) {
            if (INT64_MAX / total_elements >= dims[i]) {
                total_elements *= dims[i];
            } else {
                would_overflow = true;
            }
            if (dims[i] <= 0) {
                skip |= LogError("VUID-VkTensorDescriptionARM-pDimensions-09734", device,
                                 description_loc.dot(Field::pDimensions, i), "(%" PRIi64 ") must be greater than 0.", dims[i]);
            } else if (static_cast<uint64_t>(dims[i]) > phys_dev_ext_props.tensor_properties.maxPerDimensionTensorElements) {
                skip |=
                    LogError("VUID-VkTensorDescriptionARM-pDimensions-09883", device, description_loc.dot(Field::pDimensions, i),
                             "(%" PRIi64 ") is greater than maxPerDimensionTensorElements.(%" PRIu64 ")", dims[i],
                             phys_dev_ext_props.tensor_properties.maxPerDimensionTensorElements);
            }
        }
        if (static_cast<uint64_t>(total_elements) > phys_dev_ext_props.tensor_properties.maxTensorElements || would_overflow) {
            skip |= LogError("VUID-VkTensorCreateInfoARM-tensorElements-09721", device, description_loc.dot(Field::pDimensions),
                             "the total number of elements (%" PRIi64 ") is greater than maxTensorElements (%" PRIu64 ")", total_elements,
                             phys_dev_ext_props.tensor_properties.maxTensorElements);
        }
    }

    bool image_aliasing = (description.usage & VK_TENSOR_USAGE_IMAGE_ALIASING_BIT_ARM) != 0;
    if (description.tiling == VK_TENSOR_TILING_OPTIMAL_ARM && image_aliasing) {
        if (description.pDimensions[description.dimensionCount - 1] > 4) {
            skip |= LogError("VUID-VkTensorDescriptionARM-tiling-09741", device, description_loc.dot(Field::tiling),
                             "is VK_TENSOR_TILING_OPTIMAL_ARM and usage (%s) includes VK_TENSOR_USAGE_IMAGE_ALIASING_BIT_ARM "
                             "but pDimensions[%" PRIu32 "] (%" PRIu64 ") > 4",
                             string_VkTensorUsageFlagsARM(description.usage).c_str(), (description.dimensionCount - 1),
                             description.pDimensions[description.dimensionCount - 1]);
        }
    }
    if (description.tiling == VK_TENSOR_TILING_LINEAR_ARM && image_aliasing) {
        skip |= LogError("VUID-VkTensorDescriptionARM-tiling-09742", device, description_loc.dot(Field::tiling),
                         "is VK_TENSOR_TILING_LINEAR_ARM but usage (%s) includes VK_TENSOR_USAGE_IMAGE_ALIASING_BIT_ARM",
                         string_VkTensorUsageFlagsARM(description.usage).c_str());
    }
    return skip;
}

bool Device::manual_PreCallValidateCreateTensorARM(VkDevice device, const VkTensorCreateInfoARM *pCreateInfo,
                                                   const VkAllocationCallbacks *pAllocator, VkTensorARM *pTensor,
                                                   const Context &context) const {
    bool skip = false;

    if (!enabled_features.tensors) {
        skip |=
            LogError("VUID-vkCreateTensorARM-tensors-09832", device, context.error_obj.location, "tensors feature is not enabled");
    }

    const Location create_info_loc = context.error_obj.location.dot(Field::pCreateInfo);
    ASSERT_AND_RETURN_SKIP(pCreateInfo->pDescription);
    auto description = *pCreateInfo->pDescription;

    skip |= ValidateTensorDescriptionARM(description, create_info_loc.dot(Field::pDescription));

    if (pCreateInfo->sharingMode == VK_SHARING_MODE_CONCURRENT) {
        if (pCreateInfo->queueFamilyIndexCount <= 1) {
            skip |= LogError("VUID-VkTensorCreateInfoARM-sharingMode-09723", device, create_info_loc.dot(Field::sharingMode),
                             "is VK_SHARING_MODE_CONCURRENT, but queueFamilyIndexCount is %" PRIu32 ".",
                             pCreateInfo->queueFamilyIndexCount);
        }

        if (!pCreateInfo->pQueueFamilyIndices) {
            skip |= LogError("VUID-VkTensorCreateInfoARM-sharingMode-09722", device, create_info_loc.dot(Field::sharingMode),
                             "is VK_SHARING_MODE_CONCURRENT, but pQueueFamilyIndices is NULL.");
        }
    }

    if (vku::FindStructInPNextChain<VkOpaqueCaptureDescriptorDataCreateInfoEXT>(pCreateInfo->pNext)) {
        if (!(pCreateInfo->flags & VK_TENSOR_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_ARM)) {
            skip |= LogError("VUID-VkTensorCreateInfoARM-pNext-09727", device, create_info_loc.dot(Field::pNext),
                             "includes a VkOpaqueCaptureDescriptorDataCreateInfoEXT structure, but flags (%s) is "
                             "missing VK_TENSOR_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_ARM",
                             string_VkTensorCreateFlagsARM(pCreateInfo->flags).c_str());
        }
    }

    if ((pCreateInfo->flags & VK_TENSOR_CREATE_PROTECTED_BIT_ARM) != 0) {
        if (!enabled_features.protectedMemory) {
            skip |= LogError("VUID-VkTensorCreateInfoARM-protectedMemory-09729", device, create_info_loc.dot(Field::flags),
                             "%s has the (%s) bit set but the protectedMemory device feature is not enabled.",
                             string_VkTensorCreateFlagsARM(pCreateInfo->flags).c_str(),
                             string_VkTensorCreateFlagsARM(VK_TENSOR_CREATE_PROTECTED_BIT_ARM).c_str());
        }
    }

    if ((pCreateInfo->flags & VK_TENSOR_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_ARM) != 0) {
        if (!enabled_features.descriptorBufferCaptureReplay) {
            skip |= LogError("VUID-VkTensorCreateInfoARM-flags-09726", device, create_info_loc.dot(Field::flags),
                             "(%s) includes (%s), but the descriptorBufferCaptureReplay feature is not enabled.",
                             string_VkTensorCreateFlagsARM(pCreateInfo->flags).c_str(),
                             string_VkTensorCreateFlagsARM(VK_TENSOR_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_ARM).c_str());
        }
    }
    if (auto external_memory_info = vku::FindStructInPNextChain<VkExternalMemoryTensorCreateInfoARM>(pCreateInfo->pNext)) {
        VkPhysicalDeviceExternalTensorInfoARM tensor_info = vku::InitStructHelper();
        tensor_info.pDescription = pCreateInfo->pDescription;
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
