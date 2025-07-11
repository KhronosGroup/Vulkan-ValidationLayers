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

// #include <string>
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
    bool supports_transfer_src = (tensor_feature_flags & VK_FORMAT_FEATURE_2_TRANSFER_SRC_BIT) != 0;
    bool supports_transfer_dst = (tensor_feature_flags & VK_FORMAT_FEATURE_2_TRANSFER_DST_BIT) != 0;
    if (usage & VK_TENSOR_USAGE_TRANSFER_SRC_BIT_ARM) {
        if (!supports_transfer_src) {
            skip |= LogError(vuid, device, loc.dot(Field::usage), "The format features (%s) must contain (%s)",
                             string_VkTensorUsageFlagsARM(usage).c_str(),
                             string_VkTensorUsageFlagsARM(VK_TENSOR_USAGE_SHADER_BIT_ARM).c_str());
        }
    }
    if (usage & VK_TENSOR_USAGE_TRANSFER_DST_BIT_ARM) {
        if (!supports_transfer_dst) {
            skip |= LogError(vuid, device, loc.dot(Field::usage), "The format features (%s) must contain (%s)",
                             string_VkTensorUsageFlagsARM(usage).c_str(),
                             string_VkTensorUsageFlagsARM(VK_TENSOR_USAGE_SHADER_BIT_ARM).c_str());
        }
    }
    return skip;
}

bool CoreChecks::ValidateTensorCreateInfo(const VkTensorCreateInfoARM *pCreateInfo, const ErrorObject &error_obj) const {
    bool skip = false;
    auto description = *pCreateInfo->pDescription;
    using iter_t = decltype(description.dimensionCount);
    const Location create_info_loc = error_obj.location.dot(Field::pCreateInfo);
    if (pCreateInfo->sharingMode == VK_SHARING_MODE_CONCURRENT && pCreateInfo->pQueueFamilyIndices) {
        skip |= ValidatePhysicalDeviceQueueFamilies(pCreateInfo->queueFamilyIndexCount, pCreateInfo->pQueueFamilyIndices,
                                                    create_info_loc, "VUID-VkTensorCreateInfoARM-sharingMode-09725");
        if (!skip) {
            uint32_t queue_family_property_count = 0;
            DispatchGetPhysicalDeviceQueueFamilyProperties2Helper(api_version, physical_device, &queue_family_property_count,
                                                                  nullptr);
            vvl::unordered_set<uint32_t> queue_family_indices_set;
            for (uint32_t i = 0; i < pCreateInfo->queueFamilyIndexCount; i++) {
                const uint32_t queue_index = pCreateInfo->pQueueFamilyIndices[i];
                if (queue_family_indices_set.find(queue_index) != queue_family_indices_set.end()) {
                    skip |= LogError("VUID-VkTensorCreateInfoARM-sharingMode-09725", device, create_info_loc.dot(Field::flags),
                                     "sharingMode is VK_SHARING_MODE_CONCURRENT but pQueueFamilyIndices[%" PRIu32 "] (%" PRIu32
                                     ") is not unique",
                                     i, queue_index);
                    break;
                } else if (queue_index >= queue_family_property_count) {
                    skip |= LogError("VUID-VkTensorCreateInfoARM-sharingMode-09725", device, create_info_loc.dot(Field::flags),
                                     "sharingMode is VK_SHARING_MODE_CONCURRENT but pQueueFamilyIndices[%" PRIu32 "] (%" PRIu32
                                     ") is greater than pQueueFamilyPropertyCount (%" PRIu32
                                     ") returned by vkGetPhysicalDeviceQueueFamilyProperties2 for the "
                                     "physicalDevice that was used to create device",
                                     i, queue_index, queue_family_property_count);
                    break;
                }
                queue_family_indices_set.emplace(queue_index);
            }
        }
    }
    if ((pCreateInfo->flags & VK_TENSOR_CREATE_PROTECTED_BIT_ARM) != 0) {
        if (VK_FALSE == enabled_features.protectedMemory) {
            skip |= LogError("VUID-VkTensorCreateInfoARM-protectedMemory-09729", device, create_info_loc.dot(Field::flags),
                             "%s has the (%s) bit set but the protectedMemory device feature is not enabled.",
                             string_VkTensorCreateFlagsARM(pCreateInfo->flags).c_str(),
                             string_VkTensorCreateFlagsARM(VK_TENSOR_CREATE_PROTECTED_BIT_ARM).c_str());
        }
    }

    if ((pCreateInfo->flags & VK_TENSOR_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_ARM) != 0) {
        if (VK_FALSE == enabled_features.descriptorBufferCaptureReplay) {
            skip |= LogError("VUID-VkTensorCreateInfoARM-flags-09726", device, create_info_loc.dot(Field::flags),
                             "Flags (%s) includes (%s), but the "
                             "descriptorBufferCaptureReplay feature is not enabled.",
                             string_VkTensorCreateFlagsARM(pCreateInfo->flags).c_str(),
                             string_VkTensorCreateFlagsARM(VK_TENSOR_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_ARM).c_str());
        }
    }

    // If pDescription::usage does not have any of the following bits set (i.e. if it is not possible to create a tensor view for
    // this tensor), then the format features must contain the format feature flags required by the usage flags pDescription::format
    // as indicated in the Format Feature Dependent Usage Flags section.
    {
        VkTensorFormatPropertiesARM tensor_fmt_props = vku::InitStructHelper();
        VkFormatProperties2 fmt_props_2 = vku::InitStructHelper(&tensor_fmt_props);
        DispatchGetPhysicalDeviceFormatProperties2Helper(api_version, physical_device, description.format, &fmt_props_2);
        auto usage = description.usage;
        auto required_bits = VK_TENSOR_USAGE_SHADER_BIT_ARM;
        if ((usage & required_bits) == 0) {
            if (VK_TENSOR_TILING_OPTIMAL_ARM == description.tiling) {
                if (usage & VK_TENSOR_USAGE_TRANSFER_SRC_BIT_ARM) {
                    if ((tensor_fmt_props.optimalTilingTensorFeatures & VK_FORMAT_FEATURE_2_TRANSFER_SRC_BIT) == 0) {
                        skip |=
                            LogError("VUID-VkTensorCreateInfoARM-pDescription-09728", device, create_info_loc.dot(Field::flags),
                                     "pDescription::usage (%s) does not have any of the following bits set: (%s)"
                                     "(i.e. if it is not "
                                     "possible to create a tensor view for this tensor), then the format features must contain "
                                     "the format feature flags required by the usage flags pDescription::format (%s) as indicated "
                                     "in the Format Feature Dependent Usage Flags section (%s)",
                                     string_VkTensorUsageFlagBitsARM(usage), string_VkTensorUsageFlagBitsARM(required_bits),
                                     string_VkFormat(description.format),
                                     string_VkFormatFeatureFlagBits2(VK_FORMAT_FEATURE_2_TRANSFER_SRC_BIT));
                    }
                }
                if (usage & VK_TENSOR_USAGE_TRANSFER_DST_BIT_ARM) {
                    if ((tensor_fmt_props.optimalTilingTensorFeatures & VK_FORMAT_FEATURE_2_TRANSFER_DST_BIT) == 0) {
                        skip |=
                            LogError("VUID-VkTensorCreateInfoARM-pDescription-09728", device, create_info_loc.dot(Field::flags),
                                     "pDescription::usage (%s) does not have any of the following bits set: (%s)"
                                     "(i.e. if it is not "
                                     "possible to create a tensor view for this tensor), then the format features must contain "
                                     "the format feature flags required by the usage flags pDescription::format (%s) as indicated "
                                     "in the Format Feature Dependent Usage Flags section (%s)",
                                     string_VkTensorUsageFlagBitsARM(usage), string_VkTensorUsageFlagBitsARM(required_bits),
                                     string_VkFormat(description.format),
                                     string_VkFormatFeatureFlagBits2(VK_FORMAT_FEATURE_2_TRANSFER_DST_BIT));
                    }
                }
            } else if (VK_TENSOR_TILING_LINEAR_ARM == description.tiling) {
                if (usage & VK_TENSOR_USAGE_TRANSFER_SRC_BIT_ARM) {
                    if ((tensor_fmt_props.linearTilingTensorFeatures & VK_FORMAT_FEATURE_2_TRANSFER_SRC_BIT) == 0) {
                        skip |=
                            LogError("VUID-VkTensorCreateInfoARM-pDescription-09728", device, create_info_loc.dot(Field::flags),
                                     "pDescription::usage (%s) does not have any of the following bits set: (%s)"
                                     "(i.e. if it is not "
                                     "possible to create a tensor view for this tensor), then the format features must contain "
                                     "the format feature flags required by the usage flags pDescription::format (%s) as indicated "
                                     "in the Format Feature Dependent Usage Flags section (%s)",
                                     string_VkTensorUsageFlagBitsARM(usage), string_VkTensorUsageFlagBitsARM(required_bits),
                                     string_VkFormat(description.format),
                                     string_VkFormatFeatureFlagBits2(VK_FORMAT_FEATURE_2_TRANSFER_SRC_BIT));
                    }
                }
                if (usage & VK_TENSOR_USAGE_TRANSFER_DST_BIT_ARM) {
                    if ((tensor_fmt_props.linearTilingTensorFeatures & VK_FORMAT_FEATURE_2_TRANSFER_DST_BIT) == 0) {
                        skip |=
                            LogError("VUID-VkTensorCreateInfoARM-pDescription-09728", device, create_info_loc.dot(Field::flags),
                                     "pDescription::usage (%s) does not have any of the following bits set: (%s)"
                                     "(i.e. if it is not "
                                     "possible to create a tensor view for this tensor), then the format features must contain "
                                     "the format feature flags required by the usage flags pDescription::format (%s) as indicated "
                                     "in the Format Feature Dependent Usage Flags section (%s)",
                                     string_VkTensorUsageFlagBitsARM(usage), string_VkTensorUsageFlagBitsARM(required_bits),
                                     string_VkFormat(description.format),
                                     string_VkFormatFeatureFlagBits2(VK_FORMAT_FEATURE_2_TRANSFER_DST_BIT));
                    }
                }
            }
            if ((usage & required_bits) == 0) {
                skip |= ValidateTensorFormatUsage(description.format, usage, description.tiling,
                                                  "VUID-VkTensorCreateInfoARM-pDescription-09728", create_info_loc);
            }
        }
    }

    {
        auto format = description.format;
        const VKU_FORMAT_INFO format_info = vkuGetFormatInfo(format);
        const auto *description = pCreateInfo->pDescription;
        // format must not be VK_FORMAT_UNDEFINED and must be a one-component VkFormat.
        if (VK_FORMAT_UNDEFINED == format) {
            skip |= LogError("VUID-VkTensorDescriptionARM-format-09735", device, create_info_loc.dot(Field::flags),
                             "format must not be VK_FORMAT_UNDEFINED");
        } else {
            if (format_info.component_count != 1) {
                skip |= LogError("VUID-VkTensorDescriptionARM-format-09735", device, create_info_loc.dot(Field::flags),
                                 "format (%s) has component count (%" PRIu32 ") but must be a one-component VkFormat",
                                 string_VkFormat(format), format_info.component_count);
            }
            const auto *strides = description->pStrides;
            // pStrides[dimensionCount-1] must equal the size in bytes of a tensor element.
            if (strides && strides[description->dimensionCount - 1] != format_info.texel_block_size) {
                skip |= LogError(
                    "VUID-VkTensorDescriptionARM-pStrides-09736", device, create_info_loc.dot(Field::flags),
                    "pStrides[%" PRIu32 "](%" PRIi64 ") must equal the size in bytes of a tensor element (%" PRIu32 ")",
                    description->dimensionCount - 1, strides[description->dimensionCount - 1], format_info.texel_block_size);
            }

            //  For each i, pStrides[i] must be a multiple of the element size.
            if (strides) {
                for (iter_t i = 0; i < description->dimensionCount; i++) {
                    if ((strides[i] % format_info.texel_block_size) != 0) {
                        skip |= LogError("VUID-VkTensorDescriptionARM-pStrides-09737", device, create_info_loc.dot(Field::flags),
                                         "pStrides[%" PRIu32 "] (%" PRIi64 ") must be a multiple of the element size (%" PRIu32 ")",
                                         i, strides[i], format_info.texel_block_size);
                    }
                }
            }
        }
    }
    {
        //  If tensorNonPacked is not supported, then the members of VkTensorDescriptionARM must describe a packed tensor.
        VkPhysicalDeviceTensorFeaturesARM tensor_features = vku::InitStructHelper();
        VkPhysicalDeviceFeatures2 features = vku::InitStructHelper(&tensor_features);
        DispatchGetPhysicalDeviceFeatures2Helper(api_version, physical_device, &features);
        auto format = description.format;
        const VKU_FORMAT_INFO format_info = vkuGetFormatInfo(format);
        if (!tensor_features.tensorNonPacked) {
            const auto *description = pCreateInfo->pDescription;
            const auto *strides = description->pStrides;
            if (strides) {
                for (iter_t i = 1; i < description->dimensionCount - 1; i++) {
                    if (strides[description->dimensionCount - 1] == format_info.texel_block_size) {
                        if (strides[i - 1] != static_cast<int64_t>(strides[i] * description->pDimensions[i])) {
                            skip |= LogError("VUID-VkTensorDescriptionARM-None-09740", device, create_info_loc.dot(Field::flags),
                                             "If tensorNonPacked is not supported, then the members of VkTensorDescriptionARM must "
                                             "describe a packed tensor.");
                        }
                    }
                }
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
    skip |= ValidateTensorCreateInfo(pCreateInfo, error_obj);
    return skip;
}

bool CoreChecks::PreCallValidateCreateTensorViewARM(VkDevice device, const VkTensorViewCreateInfoARM *pCreateInfo,
                                                    const VkAllocationCallbacks *pAllocator, VkTensorViewARM *pView,
                                                    const ErrorObject &error_obj) const {
    bool skip = false;
    auto tensor_state_ptr = Get<vvl::Tensor>(pCreateInfo->tensor);
    if (!tensor_state_ptr) {
        return skip;
    }
    const Location create_info_loc = error_obj.location.dot(Field::pCreateInfo);
    const auto &tensor_state = *tensor_state_ptr;
    auto valid_usage_flags = VK_TENSOR_USAGE_SHADER_BIT_ARM;
    const VkFormat tensor_format = tensor_state.Format();
    const VkFormat view_format = pCreateInfo->format;
    const VkTensorCreateFlagsARM tensor_flags = tensor_state.create_info.flags;
    skip |= ValidateTensorUsageFlags(VK_NULL_HANDLE, tensor_state, valid_usage_flags, "VUID-VkTensorViewCreateInfoARM-usage-09747",
                                     create_info_loc.dot(Field::tensor));
    skip |= ValidateMemoryIsBoundToTensor(LogObjectList(device, pCreateInfo->tensor), tensor_state,
                                          create_info_loc.dot(Field::tensor), "VUID-VkTensorViewCreateInfoARM-tensor-09749");
    if ((tensor_flags & VK_TENSOR_CREATE_MUTABLE_FORMAT_BIT_ARM) != 0) {
        const VKU_FORMAT_INFO tensor_format_info = vkuGetFormatInfo(tensor_format);
        const VKU_FORMAT_INFO view_format_info = vkuGetFormatInfo(view_format);
        if (tensor_format_info.compatibility != view_format_info.compatibility ||
            VKU_FORMAT_COMPATIBILITY_CLASS_NONE == view_format_info.compatibility) {
            skip |= LogError(
                "VUID-VkTensorViewCreateInfoARM-tensor-09744", pCreateInfo->tensor, create_info_loc.dot(Field::format),
                "%s is not in the same format compatibility class as %s format %s. Tensors created with the "
                "VK_TENSOR_CREATE_MUTABLE_FORMAT_BIT_ARM BIT can support TensorViews with differing formats but they must be in "
                "the same compatibility class.",
                string_VkFormat(view_format), FormatHandle(pCreateInfo->tensor).c_str(), string_VkFormat(tensor_format));
        }
    } else if (tensor_format != view_format) {
        skip |=
            LogError("VUID-VkTensorViewCreateInfoARM-tensor-09743", pCreateInfo->tensor, create_info_loc.dot(Field::format),
                     "%s is different from %s format (%s). Formats MUST be IDENTICAL unless VK_TENSOR_CREATE_MUTABLE_FORMAT_BIT_ARM"
                     " was set on tensor creation.",
                     string_VkFormat(view_format), FormatHandle(pCreateInfo->tensor).c_str(), string_VkFormat(tensor_format));
    }
    if ((pCreateInfo->flags & VK_TENSOR_VIEW_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_ARM) != 0) {
        if (VK_FALSE == enabled_features.descriptorBufferCaptureReplay) {
            skip |= LogError("VUID-VkTensorViewCreateInfoARM-flags-09745", device, create_info_loc.dot(Field::flags),
                             "Flags contains VK_TENSOR_VIEW_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY BIT but the "
                             "descriptorBufferCaptureReplay feature is not enabled");
        }
    }
    if (const auto opaque_capture_descriptor_buffer =
            vku::FindStructInPNextChain<VkOpaqueCaptureDescriptorDataCreateInfoEXT>(pCreateInfo->pNext);
        opaque_capture_descriptor_buffer &&
        !(pCreateInfo->flags & VK_TENSOR_VIEW_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_ARM)) {
        skip |= LogError("VUID-VkTensorViewCreateInfoARM-pNext-09746", device, create_info_loc.dot(Field::flags),
                         "(%s) is missing VK_IMAGE_VIEW_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT but "
                         "VkOpaqueCaptureDescriptorDataCreateInfoEXT is in pNext chain.",
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
    bool correct_usage = ((tensor_state.create_info.pDescription->usage & desired) != 0);

    if (!correct_usage) {
        skip |= LogError(vuid, objlist, tensor_loc, "(%s) was created with %s but requires %s.",
                         FormatHandle(tensor_state.Handle()).c_str(),
                         string_VkTensorUsageFlagsARM(tensor_state.create_info.pDescription->usage).c_str(),
                         string_VkTensorUsageFlagsARM(desired).c_str());
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdCopyTensorARM(VkCommandBuffer cb, const VkCopyTensorInfoARM *pCopyTensorInfo,
                                                 const ErrorObject &error_obj) const {
    bool skip = false;
    auto cb_state_ptr = GetRead<vvl::CommandBuffer>(cb);
    auto src_tensor_state_ptr = Get<vvl::Tensor>(pCopyTensorInfo->srcTensor);
    if (!cb_state_ptr) {
        return skip;
    }
    if (!src_tensor_state_ptr) {
        return skip;
    }

    auto dst_tensor_state_ptr = Get<vvl::Tensor>(pCopyTensorInfo->dstTensor);
    if (!dst_tensor_state_ptr) {
        return skip;
    }

    const auto &src_tensor_state = *src_tensor_state_ptr;
    const auto &dst_tensor_state = *dst_tensor_state_ptr;
    const auto &cb_state = *cb_state_ptr;
    const Location copy_info_loc = error_obj.location.dot(Field::pCopyTensorInfo);
    LogObjectList src_objlist(cb, src_tensor_state.Handle());
    LogObjectList dst_objlist(cb, dst_tensor_state.Handle());
    auto *regions = pCopyTensorInfo->pRegions;
    skip |= ValidateCmd(cb_state, error_obj.location);
    if (src_tensor_state.DimensionCount() != dst_tensor_state.DimensionCount()) {
        skip |= LogError("VUID-VkCopyTensorInfoARM-srcTensor-09684", src_objlist, copy_info_loc.dot(Field::srcTensor),
                         "srcTensor and dstTensor were created with different values for VkTensorDescriptionARM::dimensionCount. "
                         "srcTensor::dimensionCount (%d) : dstTensor::dimensionCount (%d)",
                         src_tensor_state.DimensionCount(), dst_tensor_state.DimensionCount());
    } else {
        for (uint32_t i = 0; i < src_tensor_state.DimensionCount(); i++) {
            if (src_tensor_state.Dimensions()[i] != dst_tensor_state.Dimensions()[i]) {
                skip |= LogError("VUID-VkCopyTensorInfoARM-pDimensions-09685", src_objlist, copy_info_loc.dot(Field::pDimensions),
                                 "srcTensor and dstTensor have different sizes in pDimensions. srcTensor::pDimensions[%d] (%d) : "
                                 "dstTensor::pDimensions[%d] (%d)",
                                 i, src_tensor_state.DimensionCount(), i, dst_tensor_state.DimensionCount());
            } else {
                if (regions->pExtent) {
                    if (static_cast<int64_t>(regions->pExtent[i]) != src_tensor_state.Dimensions()[i]) {
                        skip |=
                            LogError("VUID-VkCopyTensorInfoARM-pExtent-09689", src_objlist, copy_info_loc.dot(Field::pExtent),
                                     "pExtent is not NULL but pExtent[%d] (%lu) does not equal srcTensor::pDimensions[%d] (%ld)", i,
                                     regions->pExtent[i], i, src_tensor_state.Dimensions()[i]);
                    }
                }
            }
        }
    }
    if (1 != pCopyTensorInfo->regionCount) {
        skip |= LogError("VUID-VkCopyTensorInfoARM-regionCount-09686", src_objlist, copy_info_loc.dot(Field::regionCount),
                         "regionCount (%d) does not equal 1", pCopyTensorInfo->regionCount);
    }
    if (regions->pSrcOffset) {
        for (uint32_t i = 0; i < regions->dimensionCount; i++) {
            if (0 != regions->pSrcOffset[i]) {
                skip |= LogError("VUID-VkCopyTensorInfoARM-pSrcOffset-09687", src_objlist, copy_info_loc.dot(Field::pSrcOffset),
                                 "SrcOffset is not NULL but SrcOffset[%d] (%ld) does not equal 0", i, regions->pSrcOffset[i]);
                break;
            }
        }
    }

    if (regions->pDstOffset) {
        for (uint32_t i = 0; i < regions->dimensionCount; i++) {
            if (0 != regions->pDstOffset[i]) {
                skip |= LogError("VUID-VkCopyTensorInfoARM-pDstOffset-09688", dst_objlist, copy_info_loc.dot(Field::pDstOffset),
                                 "DstOffset is not NULL but DstOffset[%d] (%ld) does not equal 0", i, regions->pDstOffset[i]);
                break;
            }
        }
    }
    skip |= ValidateTensorFormatUsage(src_tensor_state.Format(), VK_TENSOR_USAGE_TRANSFER_SRC_BIT_ARM, src_tensor_state.Tiling(),
                                      "VUID-VkCopyTensorInfoARM-srcTensor-09690", copy_info_loc.dot(Field::srcTensor));
    skip |= ValidateTensorUsageFlags(cb, src_tensor_state, VK_TENSOR_USAGE_TRANSFER_SRC_BIT_ARM,
                                     "VUID-VkCopyTensorInfoARM-srcTensor-09691", copy_info_loc.dot(Field::srcTensor));

    skip |= ValidateTensorFormatUsage(dst_tensor_state.Format(), VK_TENSOR_USAGE_TRANSFER_DST_BIT_ARM, dst_tensor_state.Tiling(),
                                      "VUID-VkCopyTensorInfoARM-dstTensor-09692", copy_info_loc.dot(Field::dstTensor));
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
    auto tensor_state = Get<vvl::Tensor>(tensor);
    bool skip = false;
    if (tensor_state) {
        skip |= ValidateObjectNotInUse(tensor_state.get(), error_obj.location, "VUID-vkDestroyTensorARM-tensor-09730");
    }
    return skip;
}

bool CoreChecks::PreCallValidateDestroyTensorViewARM(VkDevice device, VkTensorViewARM tensorView,
                                                     const VkAllocationCallbacks *pAllocator, const ErrorObject &error_obj) const {
    auto tensor_view_state = Get<vvl::TensorView>(tensorView);
    bool skip = false;
    if (tensor_view_state) {
        skip |= ValidateObjectNotInUse(tensor_view_state.get(), error_obj.location, "VUID-vkDestroyTensorViewARM-tensorView-09750");
    }
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
        if (auto tensor_state = Get<vvl::Tensor>(pInfo->tensor)) {
            if (!(tensor_state->create_info.flags & VK_TENSOR_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_ARM)) {
                skip |= LogError("VUID-VkTensorCaptureDescriptorDataInfoARM-tensor-09705", pInfo->tensor,
                                 error_obj.location.dot(Field::pInfo).dot(Field::tensor), "was created with %s.",
                                 string_VkTensorCreateFlagsARM(tensor_state->create_info.flags).c_str());
            }
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
        if (auto tensor_view_state = Get<vvl::TensorView>(pInfo->tensorView)) {
            if (!(tensor_view_state->create_info.flags & VK_TENSOR_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_ARM)) {
                skip |= LogError("VUID-VkTensorViewCaptureDescriptorDataInfoARM-tensorView-09709", pInfo->tensorView,
                                 error_obj.location.dot(Field::pInfo).dot(Field::tensor), "was created with %s.",
                                 string_VkTensorViewCreateFlagsARM(tensor_view_state->create_info.flags).c_str());
            }
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

    // Implicit Rules guarantee that pInfo and pInfo->pCreateInfo are not nullptr
    skip |= ValidateTensorCreateInfo(pInfo->pCreateInfo, error_obj);
    return skip;
}
