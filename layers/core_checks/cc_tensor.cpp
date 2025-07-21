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
 *
 */

#include <vulkan/utility/vk_format_utils.h>
#include <vulkan/vulkan_core.h>

#include "core_validation.h"
#include "generated/dispatch_functions.h"
#include "state_tracker/tensor_state.h"
#include "state_tracker/cmd_buffer_state.h"

bool CoreChecks::ValidateTensorFormatUsage(VkFormat format, VkTensorUsageFlagsARM usage, VkTensorTilingARM tiling, const char *vuid,
                                           const Location &loc) const {
    bool skip = false;
    VkTensorFormatPropertiesARM tensor_fmt_props = vku::InitStructHelper();
    VkFormatProperties2 fmt_props_2 = vku::InitStructHelper(&tensor_fmt_props);
    DispatchGetPhysicalDeviceFormatProperties2Helper(api_version, physical_device, format, &fmt_props_2);
    VkFormatFeatureFlags2 tensor_feature_flags{};
    if (VK_TENSOR_TILING_OPTIMAL_ARM == tiling) {
        tensor_feature_flags = tensor_fmt_props.optimalTilingTensorFeatures;
    } else if (VK_TENSOR_TILING_LINEAR_ARM == tiling) {
        tensor_feature_flags = tensor_fmt_props.linearTilingTensorFeatures;
    }

    const std::vector<std::pair<VkTensorUsageFlagBitsARM, VkFormatFeatureFlagBits2>> usage_to_feature_map = {
        { VK_TENSOR_USAGE_TRANSFER_SRC_BIT_ARM, VK_FORMAT_FEATURE_2_TRANSFER_SRC_BIT },
        { VK_TENSOR_USAGE_TRANSFER_DST_BIT_ARM, VK_FORMAT_FEATURE_2_TRANSFER_DST_BIT },
        { VK_TENSOR_USAGE_IMAGE_ALIASING_BIT_ARM, VK_FORMAT_FEATURE_2_TENSOR_IMAGE_ALIASING_BIT_ARM },
        { VK_TENSOR_USAGE_SHADER_BIT_ARM, VK_FORMAT_FEATURE_2_TENSOR_SHADER_BIT_ARM },
        { VK_TENSOR_USAGE_DATA_GRAPH_BIT_ARM, VK_FORMAT_FEATURE_2_TENSOR_DATA_GRAPH_BIT_ARM },
    };

    for (auto element : usage_to_feature_map) {
        auto usage_bit = element.first;
        auto feature_bit = element.second;
        if (usage & usage_bit && !(tensor_feature_flags & feature_bit)) {
            skip |= LogError(vuid, device, loc.dot(Field::usage),
                             "(%s) has bit (%s) set but format features (%s) does not include matching required bit (%s)",
                             string_VkTensorUsageFlagsARM(usage).c_str(), string_VkTensorUsageFlagsARM(usage_bit).c_str(),
                             string_VkFormatFeatureFlags2(tensor_feature_flags).c_str(),
                             string_VkFormatFeatureFlags2(feature_bit).c_str());
        }
    }

    return skip;
}

bool CoreChecks::ValidateTensorCreateInfo(const VkTensorCreateInfoARM &create_info, const Location &create_info_loc) const {
    bool skip = false;
    ASSERT_AND_RETURN_SKIP(create_info.pDescription);
    auto description = *create_info.pDescription;
    if (create_info.sharingMode == VK_SHARING_MODE_CONCURRENT && create_info.pQueueFamilyIndices) {
        skip |= ValidatePhysicalDeviceQueueFamilies(create_info.queueFamilyIndexCount, create_info.pQueueFamilyIndices,
                                                    create_info_loc, "VUID-VkTensorCreateInfoARM-sharingMode-09725");
        if (!skip) {
            uint32_t queue_family_property_count = 0;
            DispatchGetPhysicalDeviceQueueFamilyProperties2Helper(api_version, physical_device, &queue_family_property_count,
                                                                  nullptr);
            vvl::unordered_set<uint32_t> queue_family_indices_set;
            for (uint32_t i = 0; i < create_info.queueFamilyIndexCount; i++) {
                const uint32_t queue_index = create_info.pQueueFamilyIndices[i];
                if (queue_family_indices_set.find(queue_index) != queue_family_indices_set.end()) {
                    skip |=
                        LogError("VUID-VkTensorCreateInfoARM-sharingMode-09725", device, create_info_loc.dot(Field::sharingMode),
                                 "is VK_SHARING_MODE_CONCURRENT but pQueueFamilyIndices[%" PRIu32 "] (%" PRIu32 ") is not unique",
                                 i, queue_index);
                    break;
                } else if (queue_index >= queue_family_property_count) {
                    skip |=
                        LogError("VUID-VkTensorCreateInfoARM-sharingMode-09725", device, create_info_loc.dot(Field::sharingMode),
                                 "is VK_SHARING_MODE_CONCURRENT but pQueueFamilyIndices[%" PRIu32 "] (%" PRIu32
                                 ") is >= pQueueFamilyPropertyCount (%" PRIu32
                                 ") returned by vkGetPhysicalDeviceQueueFamilyProperties2 for the "
                                 "physicalDevice that was used to create device",
                                 i, queue_index, queue_family_property_count);
                    break;
                }
                queue_family_indices_set.emplace(queue_index);
            }
        }
    }
    if ((create_info.flags & VK_TENSOR_CREATE_PROTECTED_BIT_ARM) != 0) {
        if (VK_FALSE == enabled_features.protectedMemory) {
            skip |= LogError("VUID-VkTensorCreateInfoARM-protectedMemory-09729", device, create_info_loc.dot(Field::flags),
                             "%s has the (%s) bit set but the protectedMemory device feature is not enabled.",
                             string_VkTensorCreateFlagsARM(create_info.flags).c_str(),
                             string_VkTensorCreateFlagsARM(VK_TENSOR_CREATE_PROTECTED_BIT_ARM).c_str());
        }
    }

    if ((create_info.flags & VK_TENSOR_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_ARM) != 0) {
        if (VK_FALSE == enabled_features.descriptorBufferCaptureReplay) {
            skip |= LogError("VUID-VkTensorCreateInfoARM-flags-09726", device, create_info_loc.dot(Field::flags),
                             "(%s) includes (%s), but the descriptorBufferCaptureReplay feature is not enabled.",
                             string_VkTensorCreateFlagsARM(create_info.flags).c_str(),
                             string_VkTensorCreateFlagsARM(VK_TENSOR_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_ARM).c_str());
        }
    }

    // Check usage vs. features
    const auto required_bits = VK_TENSOR_USAGE_SHADER_BIT_ARM | VK_TENSOR_USAGE_DATA_GRAPH_BIT_ARM;
    if ((description.usage & required_bits) == 0) {
        skip |= ValidateTensorFormatUsage(description.format, description.usage, description.tiling,
                                            "VUID-VkTensorCreateInfoARM-pDescription-09728", create_info_loc);
    }

    // format must not be VK_FORMAT_UNDEFINED and must be a one-component VkFormat.
    if (VK_FORMAT_UNDEFINED == description.format) {
        skip |= LogError("VUID-VkTensorDescriptionARM-format-09735", device, create_info_loc.dot(Field::format),
                         "is VK_FORMAT_UNDEFINED");
    } else {
        const uint32_t component_count = vkuFormatComponentCount(description.format);
        if (component_count != 1) {
            skip |= LogError("VUID-VkTensorDescriptionARM-format-09735", device, create_info_loc.dot(Field::format),
                             "(%s) has (%" PRIu32 ") components.", string_VkFormat(description.format), component_count);
        }

        // pStrides can be null
        if (const auto *strides = description.pStrides) {
            const uint32_t texel_block_size = vkuFormatTexelBlockSize(description.format);
            if (strides[description.dimensionCount - 1] != texel_block_size) {
                skip |= LogError("VUID-VkTensorDescriptionARM-pStrides-09736", device,
                                 create_info_loc.dot(Field::pDescription).dot(Field::pStrides, description.dimensionCount - 1),
                                 "(%" PRIi64 ") must equal the size in bytes of a tensor element (%" PRIu32 ")",
                                 strides[description.dimensionCount - 1], texel_block_size);
            }

            for (uint32_t i = 0; i < description.dimensionCount; i++) {
                if ((strides[i] % texel_block_size) != 0) {
                    skip |= LogError("VUID-VkTensorDescriptionARM-pStrides-09737", device, create_info_loc.dot(Field::pStrides, i),
                                     "(%" PRIi64 ") must be a multiple of the element size (%" PRIu32 ")", strides[i],
                                     texel_block_size);
                }
            }
        }
    }
    // no point continuing if the checks above failed
    if (skip) {
        return skip;
    }

    if (!enabled_features.tensorNonPacked) {
        if (const auto *strides = description.pStrides) {
            ASSERT_AND_RETURN_SKIP(description.pDimensions);
            int64_t i = description.dimensionCount - 1;
            bool is_packed = strides[i] == vkuFormatTexelBlockSize(description.format);
            while (is_packed && i > 0) {
                if (strides[i - 1] != static_cast<int64_t>(strides[i] * description.pDimensions[i])) {
                    is_packed = false;
                    break;
                }
                i--;
            }
            ASSERT_AND_RETURN_SKIP(i >= 0);  // sanity check for is_packed computation
            if (!is_packed) {
                skip |= LogError("VUID-VkTensorDescriptionARM-None-09740", device, create_info_loc.dot(Field::description),
                                 "does not define a packed tensor: pStrides[%" PRIi64 "] (%" PRIi64 ") != pStrides[%" PRIi64
                                 "] (%" PRIi64 ") * pDimensions[%" PRIi64 "] (%" PRIi64 ")",
                                 i - 1, description.pStrides[i - 1], i, description.pStrides[i], i, description.pDimensions[i]);
            }
        }
    }

    return skip;
}

bool CoreChecks::PreCallValidateCreateTensorARM(VkDevice device, const VkTensorCreateInfoARM *pCreateInfo,
                                                const VkAllocationCallbacks *pAllocator, VkTensorARM *pTensor,
                                                const ErrorObject &error_obj) const {
    bool skip = false;
    const Location create_info_loc = error_obj.location.dot(Field::pCreateInfo);
    if (!enabled_features.tensors) {
        skip |= LogError("VUID-vkCreateTensorARM-tensors-09832", device, create_info_loc.dot(Field::tensors),
                         "tensors feature is not enabled");
    }
    skip |= ValidateTensorCreateInfo(*pCreateInfo, create_info_loc);
    return skip;
}

bool CoreChecks::PreCallValidateCreateTensorViewARM(VkDevice device, const VkTensorViewCreateInfoARM *pCreateInfo,
                                                    const VkAllocationCallbacks *pAllocator, VkTensorViewARM *pView,
                                                    const ErrorObject &error_obj) const {
    bool skip = false;
    auto tensor_state_ptr = Get<vvl::Tensor>(pCreateInfo->tensor);
    ASSERT_AND_RETURN_SKIP(tensor_state_ptr);
    const Location create_info_loc = error_obj.location.dot(Field::pCreateInfo);
    const auto &tensor_state = *tensor_state_ptr;
    auto valid_usage_flags = VK_TENSOR_USAGE_SHADER_BIT_ARM | VK_TENSOR_USAGE_DATA_GRAPH_BIT_ARM;
    const VkFormat tensor_format = tensor_state.description.format;
    const VkFormat view_format = pCreateInfo->format;
    skip |= ValidateTensorUsageFlags(VK_NULL_HANDLE, tensor_state, valid_usage_flags, "VUID-VkTensorViewCreateInfoARM-usage-09747",
                                     create_info_loc.dot(Field::tensor));
    skip |= ValidateMemoryIsBoundToTensor(LogObjectList(device, pCreateInfo->tensor), tensor_state,
                                          create_info_loc.dot(Field::tensor), "VUID-VkTensorViewCreateInfoARM-tensor-09749");
    if ((tensor_state.create_info.flags & VK_TENSOR_CREATE_MUTABLE_FORMAT_BIT_ARM) != 0) {
        auto view_format_compatibility = vkuFormatCompatibilityClass(view_format);
        if (vkuFormatCompatibilityClass(tensor_format) != view_format_compatibility) {
            skip |= LogError("VUID-VkTensorViewCreateInfoARM-tensor-09744", pCreateInfo->tensor, create_info_loc.dot(Field::format),
                             "(%s) is not in the same format compatibility class as the tensor format (%s).",
                             string_VkFormat(view_format), string_VkFormat(tensor_format));
        }
    } else if (tensor_format != view_format) {
        skip |= LogError("VUID-VkTensorViewCreateInfoARM-tensor-09743", pCreateInfo->tensor, create_info_loc.dot(Field::format),
                         "(%s) is different from the tensor format (%s).", string_VkFormat(view_format),
                         string_VkFormat(tensor_format));
    }
    if ((pCreateInfo->flags & VK_TENSOR_VIEW_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_ARM) != 0) {
        if (VK_FALSE == enabled_features.descriptorBufferCaptureReplay) {
            skip |= LogError("VUID-VkTensorViewCreateInfoARM-flags-09745", device, create_info_loc.dot(Field::flags),
                             "(%s) contains VK_TENSOR_VIEW_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY BIT but the "
                             "descriptorBufferCaptureReplay feature is not enabled",
                             string_VkTensorViewCreateFlagsARM(pCreateInfo->flags).c_str());
        }
    }
    if (const auto opaque_capture_descriptor_buffer =
            vku::FindStructInPNextChain<VkOpaqueCaptureDescriptorDataCreateInfoEXT>(pCreateInfo->pNext);
        opaque_capture_descriptor_buffer &&
        !(pCreateInfo->flags & VK_TENSOR_VIEW_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_ARM)) {
        skip |= LogError("VUID-VkTensorViewCreateInfoARM-pNext-09746", device, create_info_loc.dot(Field::flags),
                         "(%s) is missing VK_IMAGE_VIEW_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT but "
                         "VkOpaqueCaptureDescriptorDataCreateInfoEXT is in the pNext chain.",
                         string_VkTensorViewCreateFlagsARM(pCreateInfo->flags).c_str());
    }

    skip |= ValidateTensorFormatUsage(view_format, tensor_state.create_info.pDescription->usage,
                                      tensor_state.create_info.pDescription->tiling, "VUID-VkTensorViewCreateInfoARM-usage-09748",
                                      create_info_loc);

    return skip;
}

bool CoreChecks::ValidateTensorUsageFlags(VkCommandBuffer cb, vvl::Tensor const &tensor_state, VkTensorUsageFlagsARM desired,
                                          const char *vuid, const Location &tensor_loc) const {
    bool skip = false;
    LogObjectList objlist(cb, tensor_state.Handle());
    if ((tensor_state.create_info.pDescription->usage & desired) == 0) {
        skip |= LogError(vuid, objlist, tensor_loc.dot(Field::usage), "(%s) for tensor (%s) doesn't match requirements.",
                         string_VkTensorUsageFlagsARM(tensor_state.create_info.pDescription->usage).c_str(),
                         FormatHandle(tensor_state.Handle()).c_str());
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdCopyTensorARM(VkCommandBuffer cb, const VkCopyTensorInfoARM *pCopyTensorInfo,
                                                 const ErrorObject &error_obj) const {
    bool skip = false;
    auto cb_state_ptr = GetRead<vvl::CommandBuffer>(cb);
    ASSERT_AND_RETURN_SKIP(cb_state_ptr);
    auto src_tensor_state_ptr = Get<vvl::Tensor>(pCopyTensorInfo->srcTensor);
    ASSERT_AND_RETURN_SKIP(src_tensor_state_ptr);
    auto dst_tensor_state_ptr = Get<vvl::Tensor>(pCopyTensorInfo->dstTensor);
    ASSERT_AND_RETURN_SKIP(dst_tensor_state_ptr);

    const auto &src_tensor_state = *src_tensor_state_ptr;
    const auto &dst_tensor_state = *dst_tensor_state_ptr;
    const auto &cb_state = *cb_state_ptr;
    const Location copy_info_loc = error_obj.location.dot(Field::pCopyTensorInfo);
    LogObjectList src_objlist(cb, src_tensor_state.Handle());
    LogObjectList dst_objlist(cb, dst_tensor_state.Handle());
    LogObjectList tensors_objlist(src_tensor_state.Handle(), dst_tensor_state.Handle());
    auto *regions = pCopyTensorInfo->pRegions;
    skip |= ValidateCmd(cb_state, error_obj.location);
    if (src_tensor_state.description.dimensionCount != dst_tensor_state.description.dimensionCount) {
        skip |= LogError("VUID-VkCopyTensorInfoARM-srcTensor-09684", tensors_objlist, copy_info_loc,
                         "dimensionCount for srcTensor (%d) and dstTensor (%d) are different",
                         src_tensor_state.description.dimensionCount, dst_tensor_state.description.dimensionCount);
    } else {
        for (uint32_t i = 0; i < src_tensor_state.description.dimensionCount; i++) {
            if (src_tensor_state.description.pDimensions[i] != dst_tensor_state.description.pDimensions[i]) {
                skip |= LogError("VUID-VkCopyTensorInfoARM-pDimensions-09685", tensors_objlist, copy_info_loc,
                                 "pDimensions[%u] for srcTensor (%" PRIi64 ") and dstTensor (%" PRIi64 ") are different.", i,
                                 src_tensor_state.description.pDimensions[i], dst_tensor_state.description.pDimensions[i]);
            } else {
                if (regions->pExtent) {
                    if (static_cast<int64_t>(regions->pExtent[i]) != src_tensor_state.description.pDimensions[i]) {
                        skip |= LogError("VUID-VkCopyTensorInfoARM-pExtent-09689", src_objlist,
                                         copy_info_loc.dot(Field::pRegions).dot(Field::pExtent, i),
                                         "(%" PRIu64 ") is not equal to srcTensor::pDimensions[%d] (%" PRIi64 ")",
                                         regions->pExtent[i], i, src_tensor_state.description.pDimensions[i]);
                    }
                }
            }
        }
    }
    if (1 != pCopyTensorInfo->regionCount) {
        skip |= LogError("VUID-VkCopyTensorInfoARM-regionCount-09686", tensors_objlist, copy_info_loc.dot(Field::regionCount),
                         "(%u) is not 1", pCopyTensorInfo->regionCount);
    }
    if (regions->pSrcOffset) {
        for (uint32_t i = 0; i < regions->dimensionCount; i++) {
            if (0 != regions->pSrcOffset[i]) {
                skip |= LogError("VUID-VkCopyTensorInfoARM-pSrcOffset-09687", src_objlist, copy_info_loc.dot(Field::pSrcOffset, i),
                                 "(%" PRIu64 ") is not zero", regions->pSrcOffset[i]);
                break;
            }
        }
    }
    if (regions->pDstOffset) {
        for (uint32_t i = 0; i < regions->dimensionCount; i++) {
            if (0 != regions->pDstOffset[i]) {
                skip |= LogError("VUID-VkCopyTensorInfoARM-pDstOffset-09688", dst_objlist, copy_info_loc.dot(Field::pDstOffset, i),
                                 "(%" PRIu64 ") is not zero", regions->pDstOffset[i]);
                break;
            }
        }
    }
    skip |= ValidateTensorFormatUsage(src_tensor_state.description.format, VK_TENSOR_USAGE_TRANSFER_SRC_BIT_ARM,
                                      src_tensor_state.description.tiling, "VUID-VkCopyTensorInfoARM-srcTensor-09690",
                                      copy_info_loc.dot(Field::srcTensor));
    skip |= ValidateTensorUsageFlags(cb, src_tensor_state, VK_TENSOR_USAGE_TRANSFER_SRC_BIT_ARM,
                                     "VUID-VkCopyTensorInfoARM-srcTensor-09691", copy_info_loc.dot(Field::srcTensor));

    skip |= ValidateTensorFormatUsage(dst_tensor_state.description.format, VK_TENSOR_USAGE_TRANSFER_DST_BIT_ARM,
                                      dst_tensor_state.description.tiling, "VUID-VkCopyTensorInfoARM-dstTensor-09692",
                                      copy_info_loc.dot(Field::dstTensor));
    skip |= ValidateTensorUsageFlags(cb, dst_tensor_state, VK_TENSOR_USAGE_TRANSFER_DST_BIT_ARM,
                                     "VUID-VkCopyTensorInfoARM-dstTensor-09693", copy_info_loc.dot(Field::dstTensor));

    skip |= ValidateMemoryIsBoundToTensor(src_objlist, src_tensor_state, copy_info_loc.dot(Field::srcTensor),
                                          "VUID-VkCopyTensorInfoARM-srcTensor-09694");
    skip |= ValidateMemoryIsBoundToTensor(dst_objlist, dst_tensor_state, copy_info_loc.dot(Field::dstTensor),
                                          "VUID-VkCopyTensorInfoARM-dstTensor-09695");

    return skip;
}

bool CoreChecks::PreCallValidateDestroyTensorARM(VkDevice device, VkTensorARM tensor, const VkAllocationCallbacks *pAllocator,
                                                 const ErrorObject &error_obj) const {
    bool skip = false;
    auto tensor_state = Get<vvl::Tensor>(tensor);
    ASSERT_AND_RETURN_SKIP(tensor_state);
    skip |= ValidateObjectNotInUse(tensor_state.get(), error_obj.location, "VUID-vkDestroyTensorARM-tensor-09730");
    return skip;
}

bool CoreChecks::PreCallValidateDestroyTensorViewARM(VkDevice device, VkTensorViewARM tensorView,
                                                     const VkAllocationCallbacks *pAllocator, const ErrorObject &error_obj) const {
    bool skip = false;
    auto tensor_view_state = Get<vvl::TensorView>(tensorView);
    ASSERT_AND_RETURN_SKIP(tensor_view_state);
    skip |= ValidateObjectNotInUse(tensor_view_state.get(), error_obj.location, "VUID-vkDestroyTensorViewARM-tensorView-09750");
    return skip;
}

bool CoreChecks::PreCallValidateGetTensorOpaqueCaptureDescriptorDataARM(VkDevice device,
                                                                        const VkTensorCaptureDescriptorDataInfoARM *pInfo,
                                                                        void *pData, const ErrorObject &error_obj) const {
    bool skip = false;
    if (VK_FALSE == enabled_features.descriptorBufferCaptureReplay) {
        skip |= LogError("VUID-vkGetTensorOpaqueCaptureDescriptorDataARM-descriptorBufferCaptureReplay-09702", pInfo->tensor,
                         error_obj.location, "descriptorBufferCaptureReplay feature was not enabled.");
    } else if (VK_FALSE == enabled_features.descriptorBufferTensorDescriptors) {
        skip |= LogError("VUID-vkGetTensorOpaqueCaptureDescriptorDataARM-descriptorBufferCaptureReplay-09702", pInfo->tensor,
                         error_obj.location, "descriptorBufferTensorDescriptors feature was not enabled.");

    } else {
        auto tensor_state = Get<vvl::Tensor>(pInfo->tensor);
        ASSERT_AND_RETURN_SKIP(tensor_state);
        if (!(tensor_state->create_info.flags & VK_TENSOR_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_ARM)) {
            skip |= LogError("VUID-VkTensorCaptureDescriptorDataInfoARM-tensor-09705", pInfo->tensor,
                                error_obj.location.dot(Field::pInfo).dot(Field::tensor), "was created with %s.",
                                string_VkTensorCreateFlagsARM(tensor_state->create_info.flags).c_str());
        }
    }
    if (device_state->physical_device_count > 1 && !enabled_features.bufferDeviceAddressMultiDevice &&
        !enabled_features.bufferDeviceAddressMultiDeviceEXT) {
        skip |= LogError("VUID-vkGetTensorOpaqueCaptureDescriptorDataARM-device-09704", pInfo->tensor, error_obj.location,
                         "device was created with multiple physical devices (%" PRIu32
                         "), but the "
                         "bufferDeviceAddressMultiDevice feature was not enabled.",
                         device_state->physical_device_count);
    }
    return skip;
}

bool CoreChecks::PreCallValidateGetTensorViewOpaqueCaptureDescriptorDataARM(VkDevice device,
                                                                            const VkTensorViewCaptureDescriptorDataInfoARM *pInfo,
                                                                            void *pData, const ErrorObject &error_obj) const {
    bool skip = false;
    if (VK_FALSE == enabled_features.descriptorBufferCaptureReplay) {
        skip |= LogError("VUID-vkGetTensorViewOpaqueCaptureDescriptorDataARM-descriptorBufferCaptureReplay-09706",
                         pInfo->tensorView, error_obj.location, "descriptorBufferCaptureReplay feature was not enabled.");
    } else if (VK_FALSE == enabled_features.descriptorBufferTensorDescriptors) {
        skip |= LogError("VUID-vkGetTensorViewOpaqueCaptureDescriptorDataARM-descriptorBufferCaptureReplay-09706",
                         pInfo->tensorView, error_obj.location, "descriptorBufferTensorDescriptors feature was not enabled.");
    } else {
        auto tensor_view_state = Get<vvl::TensorView>(pInfo->tensorView);
        ASSERT_AND_RETURN_SKIP(tensor_view_state);
        if (!(tensor_view_state->create_info.flags & VK_TENSOR_VIEW_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_ARM)) {
            skip |= LogError("VUID-VkTensorViewCaptureDescriptorDataInfoARM-tensorView-09709", pInfo->tensorView,
                                error_obj.location.dot(Field::pInfo).dot(Field::tensor), "was created with %s.",
                                string_VkTensorViewCreateFlagsARM(tensor_view_state->create_info.flags).c_str());
        }
    }
    if (device_state->physical_device_count > 1 && !enabled_features.bufferDeviceAddressMultiDevice &&
        !enabled_features.bufferDeviceAddressMultiDeviceEXT) {
        skip |= LogError("VUID-vkGetTensorViewOpaqueCaptureDescriptorDataARM-device-09708", pInfo->tensorView, error_obj.location,
                         "device was created with multiple physical devices (%" PRIu32
                         "), but the "
                         "bufferDeviceAddressMultiDevice feature was not enabled.",
                         device_state->physical_device_count);
    }
    return skip;
}

// Validates the buffer is allowed to be protected
bool CoreChecks::ValidateProtectedTensor(const vvl::CommandBuffer &cb_state, const vvl::Tensor &tensor_state,
                                         const Location &tensor_loc, const char *vuid, const char *more_message) const {
    /* don't use on an unprotected tensor */
    assert(tensor_state.unprotected == false);

    bool skip = false;

    // if driver supports protectedNoFault the operation is valid, just has undefined values
    if ((!phys_dev_props_core11.protectedNoFault) && (cb_state.unprotected == true)) {
        const LogObjectList objlist(cb_state.Handle(), tensor_state.Handle());
        skip |= LogError(vuid, objlist, tensor_loc, "(%s) is a protected tensor, but command buffer (%s) is unprotected.%s",
                         FormatHandle(tensor_state).c_str(), FormatHandle(cb_state).c_str(), more_message);
    }
    return skip;
}

// Validates the buffer is allowed to be unprotected
bool CoreChecks::ValidateUnprotectedTensor(const vvl::CommandBuffer &cb_state, const vvl::Tensor &tensor_state,
                                           const Location &tensor_loc, const char *vuid, const char *more_message) const {
    /* don't use on a protected tensor */
    assert(tensor_state.unprotected == true);

    bool skip = false;

    // if driver supports protectedNoFault the operation is valid, just has undefined values
    if ((!phys_dev_props_core11.protectedNoFault) && (cb_state.unprotected == false)) {
        const LogObjectList objlist(cb_state.Handle(), tensor_state.Handle());
        skip |= LogError(vuid, objlist, tensor_loc, "(%s) is an unprotected tensor, but command buffer (%s) is protected.%s",
                         FormatHandle(tensor_state).c_str(), FormatHandle(cb_state).c_str(), more_message);
    }
    return skip;
}

bool CoreChecks::PreCallValidateGetDeviceTensorMemoryRequirementsARM(VkDevice device,
                                                                     const VkDeviceTensorMemoryRequirementsARM *pInfo,
                                                                     VkMemoryRequirements2 *pMemoryRequirements,
                                                                     const ErrorObject &error_obj) const {
    bool skip = false;
    const Location loc = error_obj.location.dot(Field::tensors);
    if (!enabled_features.tensors) {
        skip |=
            LogError("VUID-vkGetDeviceTensorMemoryRequirementsARM-tensors-09831", device, loc, "tensors feature is not enabled");
    }
    return skip;
}
