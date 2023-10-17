/* Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
 * Copyright (C) 2015-2023 Google Inc.
 * Modifications Copyright (C) 2020-2022 Advanced Micro Devices, Inc. All rights reserved.
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

#include <string>
#include <vector>

#include <vulkan/vk_enum_string_helper.h>
#include "generated/chassis.h"
#include "core_validation.h"

// Helper function to validate usage flags for buffers. For given buffer_state send actual vs. desired usage off to helper above
// where an error will be flagged if usage is not correct
bool CoreChecks::ValidateBufferUsageFlags(const LogObjectList &objlist, BUFFER_STATE const &buffer_state,
                                          VkBufferUsageFlags desired, bool strict, const char *vuid,
                                          const Location &buffer_loc) const {
    bool skip = false;
    bool correct_usage = false;
    if (strict) {
        correct_usage = ((buffer_state.usage & desired) == desired);
    } else {
        correct_usage = ((buffer_state.usage & desired) != 0);
    }

    if (!correct_usage) {
        skip = LogError(vuid, objlist, buffer_loc, "(%s) was created with %s but requires %s.",
                        FormatHandle(buffer_state.Handle()).c_str(), string_VkBufferUsageFlags2KHR(buffer_state.usage).c_str(),
                        string_VkBufferUsageFlags(desired).c_str());
    }
    return skip;
}

bool CoreChecks::ValidateBufferViewRange(const BUFFER_STATE &buffer_state, const VkBufferViewCreateInfo *pCreateInfo,
                                         const VkPhysicalDeviceLimits *device_limits, const Location &loc) const {
    bool skip = false;

    const VkDeviceSize &range = pCreateInfo->range;
    const VkFormat format = pCreateInfo->format;
    const uint32_t format_size = vkuFormatElementSize(format);
    if (range != VK_WHOLE_SIZE) {
        // Range must be greater than 0
        if (range <= 0) {
            skip |= LogError("VUID-VkBufferViewCreateInfo-range-00928", buffer_state.buffer(), loc.dot(Field::range),
                             "(%" PRIuLEAST64 ") does not equal VK_WHOLE_SIZE, range must be greater than 0.", range);
        }
        // Range must be a multiple of the element size of format
        if (SafeModulo(range, format_size) != 0) {
            skip |=
                LogError("VUID-VkBufferViewCreateInfo-range-00929", buffer_state.buffer(), loc.dot(Field::range),
                         "(%" PRIuLEAST64 ") does not equal VK_WHOLE_SIZE, range must be a multiple of the element size (%" PRIu32
                         ") of the format %s.",
                         range, format_size, string_VkFormat(format));
        }
        // Range divided by the element size of format must be less than or equal to VkPhysicalDeviceLimits::maxTexelBufferElements
        if (SafeDivision(range, format_size) > device_limits->maxTexelBufferElements) {
            skip |= LogError("VUID-VkBufferViewCreateInfo-range-00930", buffer_state.buffer(), loc.dot(Field::range),
                             "(%" PRIuLEAST64
                             ") does not equal VK_WHOLE_SIZE, range divided by the element size of the format (%" PRIu32
                             ") must be less than or equal to VkPhysicalDeviceLimits::maxTexelBufferElements (%" PRIuLEAST32 ").",
                             range, format_size, device_limits->maxTexelBufferElements);
        }
        // The sum of range and offset must be less than or equal to the size of buffer
        if (range + pCreateInfo->offset > buffer_state.createInfo.size) {
            skip |= LogError("VUID-VkBufferViewCreateInfo-offset-00931", buffer_state.buffer(), loc.dot(Field::range),
                             "(%" PRIuLEAST64 ") does not equal VK_WHOLE_SIZE, the sum of offset (%" PRIuLEAST64
                             ") and range must be less than or equal to the size of the buffer (%" PRIuLEAST64 ").",
                             range, pCreateInfo->offset, buffer_state.createInfo.size);
        }
    } else {
        // Size of buffer - offset, divided by the element size of format must be less than or equal to
        // VkPhysicalDeviceLimits::maxTexelBufferElements
        if (SafeDivision(buffer_state.createInfo.size - pCreateInfo->offset, format_size) > device_limits->maxTexelBufferElements) {
            skip |= LogError(
                "VUID-VkBufferViewCreateInfo-range-04059", buffer_state.buffer(), loc.dot(Field::range),
                "(%" PRIuLEAST64 ") equals VK_WHOLE_SIZE, the buffer's size (%" PRIuLEAST64 ") minus the offset (%" PRIuLEAST64
                "), divided by the element size (%" PRIu32
                ") of the format %s must be less than or equal to VkPhysicalDeviceLimits::maxTexelBufferElements (%" PRIuLEAST32
                ").",
                range, buffer_state.createInfo.size, pCreateInfo->offset, format_size, string_VkFormat(format),
                device_limits->maxTexelBufferElements);
        }
    }
    return skip;
}

bool CoreChecks::ValidateBufferViewBuffer(const BUFFER_STATE &buffer_state, const VkBufferViewCreateInfo *pCreateInfo,
                                          const Location &loc) const {
    bool skip = false;
    const VkFormat format = pCreateInfo->format;
    const VkFormatProperties3KHR format_properties = GetPDFormatProperties(format);
    const VkBufferUsageFlags2KHR usage = buffer_state.usage;
    if ((usage & VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT) &&
        !(format_properties.bufferFeatures & VK_FORMAT_FEATURE_2_UNIFORM_TEXEL_BUFFER_BIT_KHR)) {
        skip |= LogError("VUID-VkBufferViewCreateInfo-format-08778", buffer_state.buffer(), loc.dot(Field::buffer),
                         "was created with usage (%s) containing VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT, format "
                         "(%s) must be supported for uniform texel buffers. (supported bufferFeatures: %s)",
                         string_VkBufferUsageFlags2KHR(usage).c_str(), string_VkFormat(format),
                         string_VkFormatFeatureFlags2(format_properties.bufferFeatures).c_str());
    }
    if ((usage & VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT) &&
        !(format_properties.bufferFeatures & VK_FORMAT_FEATURE_2_STORAGE_TEXEL_BUFFER_BIT_KHR)) {
        skip |= LogError("VUID-VkBufferViewCreateInfo-format-08779", buffer_state.buffer(), loc.dot(Field::buffer),
                         "was created with usage (%s) containing VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT, format "
                         "(%s) must be supported for storage texel buffers. (supported bufferFeatures: %s)",
                         string_VkBufferUsageFlags2KHR(usage).c_str(), string_VkFormat(format),
                         string_VkFormatFeatureFlags2(format_properties.bufferFeatures).c_str());
    }
    return skip;
}

bool CoreChecks::PreCallValidateCreateBuffer(VkDevice device, const VkBufferCreateInfo *pCreateInfo,
                                             const VkAllocationCallbacks *pAllocator, VkBuffer *pBuffer,
                                             const ErrorObject &error_obj) const {
    bool skip = false;
    const Location create_info_loc = error_obj.location.dot(Field::pCreateInfo);
    auto chained_devaddr_struct = vku::FindStructInPNextChain<VkBufferDeviceAddressCreateInfoEXT>(pCreateInfo->pNext);
    if (chained_devaddr_struct) {
        if (!(pCreateInfo->flags & VK_BUFFER_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT) &&
            chained_devaddr_struct->deviceAddress != 0) {
            skip |= LogError("VUID-VkBufferCreateInfo-deviceAddress-02604", device,
                             create_info_loc.pNext(Struct::VkBufferDeviceAddressCreateInfoEXT, Field::deviceAddress),
                             "(%" PRIu64 ") is non-zero but requires VK_BUFFER_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT.",
                             chained_devaddr_struct->deviceAddress);
        }
    }

    auto chained_opaqueaddr_struct = vku::FindStructInPNextChain<VkBufferOpaqueCaptureAddressCreateInfo>(pCreateInfo->pNext);
    if (chained_opaqueaddr_struct) {
        if (!(pCreateInfo->flags & VK_BUFFER_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT) &&
            chained_opaqueaddr_struct->opaqueCaptureAddress != 0) {
            skip |= LogError("VUID-VkBufferCreateInfo-opaqueCaptureAddress-03337", device,
                             create_info_loc.pNext(Struct::VkBufferOpaqueCaptureAddressCreateInfo, Field::opaqueCaptureAddress),
                             "(%" PRIu64 ") is non-zero but requires VK_BUFFER_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT.",
                             chained_opaqueaddr_struct->opaqueCaptureAddress);
        }
    }

    auto dedicated_allocation_buffer = vku::FindStructInPNextChain<VkDedicatedAllocationBufferCreateInfoNV>(pCreateInfo->pNext);
    if (dedicated_allocation_buffer && dedicated_allocation_buffer->dedicatedAllocation == VK_TRUE) {
        if (pCreateInfo->flags &
            (VK_BUFFER_CREATE_SPARSE_BINDING_BIT | VK_BUFFER_CREATE_SPARSE_RESIDENCY_BIT | VK_BUFFER_CREATE_SPARSE_ALIASED_BIT)) {
            skip |= LogError("VUID-VkBufferCreateInfo-pNext-01571", device, create_info_loc.dot(Field::flags),
                             "%s when VkDedicatedAllocationBufferCreateInfoNV::dedicatedAllocation is VK_TRUE.",
                             string_VkBufferCreateFlags(pCreateInfo->flags).c_str());
        }
    }

    if ((pCreateInfo->flags & VK_BUFFER_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT) &&
        !enabled_features.bufferDeviceAddressCaptureReplay && !enabled_features.bufferDeviceAddressCaptureReplayEXT) {
        skip |= LogError("VUID-VkBufferCreateInfo-flags-03338", device, create_info_loc.dot(Field::flags),
                         "has VK_BUFFER_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT set but the bufferDeviceAddressCaptureReplay "
                         "device feature is not enabled.");
    }

    if (pCreateInfo->sharingMode == VK_SHARING_MODE_CONCURRENT && pCreateInfo->pQueueFamilyIndices) {
        skip |= ValidatePhysicalDeviceQueueFamilies(pCreateInfo->queueFamilyIndexCount, pCreateInfo->pQueueFamilyIndices,
                                                    create_info_loc, "VUID-VkBufferCreateInfo-sharingMode-01419");
    }

    if ((pCreateInfo->flags & VK_BUFFER_CREATE_PROTECTED_BIT) != 0) {
        if (enabled_features.protectedMemory == VK_FALSE) {
            skip |= LogError("VUID-VkBufferCreateInfo-flags-01887", device, create_info_loc.dot(Field::flags),
                             "has VK_BUFFER_CREATE_PROTECTED_BIT set but the protectedMemory device feature is not enabled.");
        }
        const VkBufferCreateFlags invalid_flags =
            VK_BUFFER_CREATE_SPARSE_BINDING_BIT | VK_BUFFER_CREATE_SPARSE_RESIDENCY_BIT | VK_BUFFER_CREATE_SPARSE_ALIASED_BIT;
        if ((pCreateInfo->flags & invalid_flags) != 0) {
            skip |= LogError("VUID-VkBufferCreateInfo-None-01888", device, create_info_loc.dot(Field::flags),
                             "is %s but can't mix protected with sparse flags.",
                             string_VkBufferCreateFlags(pCreateInfo->flags).c_str());
        }
    }

    const auto *usage_flags2 = vku::FindStructInPNextChain<VkBufferUsageFlags2CreateInfoKHR>(pCreateInfo->pNext);
    const VkBufferUsageFlags2KHR usage = usage_flags2 ? usage_flags2->usage : pCreateInfo->usage;

    bool has_decode_usage = usage & (VK_BUFFER_USAGE_VIDEO_DECODE_SRC_BIT_KHR | VK_BUFFER_USAGE_VIDEO_DECODE_DST_BIT_KHR);
    bool has_encode_usage = usage & (VK_BUFFER_USAGE_VIDEO_ENCODE_SRC_BIT_KHR | VK_BUFFER_USAGE_VIDEO_ENCODE_DST_BIT_KHR);
    if (has_decode_usage || has_encode_usage) {
        const auto *video_profiles = vku::FindStructInPNextChain<VkVideoProfileListInfoKHR>(pCreateInfo->pNext);
        skip |= ValidateVideoProfileListInfo(video_profiles, device, "vkCreateBuffer", has_decode_usage,
                                             "VUID-VkBufferCreateInfo-usage-04813", has_encode_usage,
                                             "VUID-VkBufferCreateInfo-usage-04814");
    }

    if (usage & VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT) {
        if (pCreateInfo->size + samplerDescriptorBufferAddressSpaceSize >
            phys_dev_ext_props.descriptor_buffer_props.samplerDescriptorBufferAddressSpaceSize) {
            skip |= LogError(
                "VUID-VkBufferCreateInfo-usage-08097", device, create_info_loc.dot(Field::size),
                "(%" PRIuLEAST64 ") plus current total (%" PRIuLEAST64
                ") is greater than specified in properties field samplerDescriptorBufferAddressSpaceSize (%" PRIuLEAST64 ").",
                pCreateInfo->size, samplerDescriptorBufferAddressSpaceSize.load(),
                phys_dev_ext_props.descriptor_buffer_props.samplerDescriptorBufferAddressSpaceSize);
        } else if (pCreateInfo->size + descriptorBufferAddressSpaceSize >
                   phys_dev_ext_props.descriptor_buffer_props.descriptorBufferAddressSpaceSize) {
            skip |= LogError("VUID-VkBufferCreateInfo-usage-08097", device, create_info_loc.dot(Field::size),
                             "(%" PRIuLEAST64 ") plus current total (%" PRIuLEAST64
                             ") is greater than specified in properties field descriptorBufferAddressSpaceSize (%" PRIuLEAST64 ")",
                             pCreateInfo->size, descriptorBufferAddressSpaceSize.load(),
                             phys_dev_ext_props.descriptor_buffer_props.descriptorBufferAddressSpaceSize);
        }
    }

    if (usage & VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT) {
        if (pCreateInfo->size + resourceDescriptorBufferAddressSpaceSize >
            phys_dev_ext_props.descriptor_buffer_props.resourceDescriptorBufferAddressSpaceSize) {
            skip |= LogError(
                "VUID-VkBufferCreateInfo-usage-08098", device, create_info_loc.dot(Field::size),
                "(%" PRIuLEAST64 ") plus current total (%" PRIuLEAST64
                ") is greater than specified in properties field resourceDescriptorBufferAddressSpaceSize (%" PRIuLEAST64 ").",
                pCreateInfo->size, resourceDescriptorBufferAddressSpaceSize.load(),
                phys_dev_ext_props.descriptor_buffer_props.resourceDescriptorBufferAddressSpaceSize);
        } else if (pCreateInfo->size + descriptorBufferAddressSpaceSize >
                   phys_dev_ext_props.descriptor_buffer_props.descriptorBufferAddressSpaceSize) {
            skip |= LogError("VUID-VkBufferCreateInfo-usage-08098", device, create_info_loc.dot(Field::size),
                             "(%" PRIuLEAST64 ") plus current total (%" PRIuLEAST64
                             ") is greater than specified in properties field descriptorBufferAddressSpaceSize (%" PRIuLEAST64 ").",
                             pCreateInfo->size, descriptorBufferAddressSpaceSize.load(),
                             phys_dev_ext_props.descriptor_buffer_props.descriptorBufferAddressSpaceSize);
        }
    }

    if ((pCreateInfo->flags & VK_BUFFER_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT) &&
        !enabled_features.descriptorBufferCaptureReplay) {
        skip |= LogError("VUID-VkBufferCreateInfo-flags-08099", device, create_info_loc.dot(Field::flags),
                         "has VK_BUFFER_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT set but the descriptorBufferCaptureReplay "
                         "device feature is not enabled.");
    }

    auto opaque_capture_descriptor_buffer = vku::FindStructInPNextChain<VkOpaqueCaptureDescriptorDataCreateInfoEXT>(pCreateInfo->pNext);
    if (opaque_capture_descriptor_buffer && !(pCreateInfo->flags & VK_BUFFER_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT)) {
        skip |= LogError("VUID-VkBufferCreateInfo-pNext-08100", device, create_info_loc.dot(Field::flags),
                         "(%s) is missing VK_BUFFER_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT but "
                         "VkOpaqueCaptureDescriptorDataCreateInfoEXT is in pNext chain.",
                         string_VkBufferCreateFlags(pCreateInfo->flags).c_str());
    }

    if (usage & VK_BUFFER_USAGE_PUSH_DESCRIPTORS_DESCRIPTOR_BUFFER_BIT_EXT) {
        if (!enabled_features.descriptorBufferPushDescriptors) {
            skip |= LogError("VUID-VkBufferCreateInfo-usage-08101", device, create_info_loc.dot(Field::usage),
                             "has VK_BUFFER_USAGE_PUSH_DESCRIPTORS_DESCRIPTOR_BUFFER_BIT_EXT set but the "
                             "descriptorBufferPushDescriptors device feature is not enabled.");
        }

        if (phys_dev_ext_props.descriptor_buffer_props.bufferlessPushDescriptors) {
            skip |= LogError("VUID-VkBufferCreateInfo-usage-08102", device, create_info_loc.dot(Field::usage),
                             "has VK_BUFFER_USAGE_PUSH_DESCRIPTORS_DESCRIPTOR_BUFFER_BIT_EXT set but the bufferlessPushDescriptors "
                             "device feature is enabled.");
        }

        if (!(usage & (VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT | VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT))) {
            skip |= LogError("VUID-VkBufferCreateInfo-usage-08103", device, create_info_loc.dot(Field::usage), "is (%s).",
                             string_VkBufferUsageFlags2KHR(usage).c_str());
        }
    }

    auto external_memory_info = vku::FindStructInPNextChain<VkExternalMemoryBufferCreateInfo>(pCreateInfo->pNext);
    if (external_memory_info && external_memory_info->handleTypes) {
        const uint32_t any_type = 1u << MostSignificantBit(external_memory_info->handleTypes);
        VkPhysicalDeviceExternalBufferInfo external_buffer_info = vku::InitStructHelper();
        external_buffer_info.flags = pCreateInfo->flags;
        // for now no VkBufferUsageFlags2KHR flag can be used, so safe to pass in as 32-bit version
        external_buffer_info.usage = VkBufferUsageFlags(pCreateInfo->usage);
        external_buffer_info.handleType = static_cast<VkExternalMemoryHandleTypeFlagBits>(any_type);
        VkExternalBufferProperties external_buffer_properties = vku::InitStructHelper();
        DispatchGetPhysicalDeviceExternalBufferProperties(physical_device, &external_buffer_info, &external_buffer_properties);
        const auto compatible_types = external_buffer_properties.externalMemoryProperties.compatibleHandleTypes;

        if ((external_memory_info->handleTypes & compatible_types) != external_memory_info->handleTypes) {
            skip |= LogError("VUID-VkBufferCreateInfo-pNext-00920", device,
                             create_info_loc.pNext(Struct::VkExternalMemoryBufferCreateInfo, Field::handleTypes),
                             "(%s) is not reported as compatible by vkGetPhysicalDeviceExternalBufferProperties.",
                             string_VkExternalMemoryHandleTypeFlags(external_memory_info->handleTypes).c_str());
        }
    }

    return skip;
}

bool CoreChecks::PreCallValidateCreateBufferView(VkDevice device, const VkBufferViewCreateInfo *pCreateInfo,
                                                 const VkAllocationCallbacks *pAllocator, VkBufferView *pView,
                                                 const ErrorObject &error_obj) const {
    bool skip = false;
    auto buffer_state_ptr = Get<BUFFER_STATE>(pCreateInfo->buffer);
    const Location create_info_loc = error_obj.location.dot(Field::pCreateInfo);
    // If this isn't a sparse buffer, it needs to have memory backing it at CreateBufferView time
    if (!buffer_state_ptr) {
        return skip;
    }
    const auto &buffer_state = *buffer_state_ptr;
    const LogObjectList objlist(device, pCreateInfo->buffer);

    if (vkuFormatIsDepthOrStencil(pCreateInfo->format)) {
        // Should never hopefully get here, but there are known driver advertising the wrong feature flags
        // see https://gitlab.khronos.org/vulkan/vulkan/-/merge_requests/4849
        skip |= LogError(kVUID_Core_invalidDepthStencilFormat, device, create_info_loc.dot(Field::format),
                         "is a depth/stencil format (%s) but depth/stencil formats do not have a "
                         "defined sizes for alignment, replace with a color format.",
                         string_VkFormat(pCreateInfo->format));
    }

    skip |= ValidateMemoryIsBoundToBuffer(device, buffer_state, create_info_loc.dot(Field::buffer),
                                          "VUID-VkBufferViewCreateInfo-buffer-00935");
    // In order to create a valid buffer view, the buffer must have been created with at least one of the following flags:
    // UNIFORM_TEXEL_BUFFER_BIT or STORAGE_TEXEL_BUFFER_BIT
    skip |= ValidateBufferUsageFlags(objlist, buffer_state,
                                     VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT, false,
                                     "VUID-VkBufferViewCreateInfo-buffer-00932", create_info_loc.dot(Field::buffer));

    // Buffer view offset must be less than the size of buffer
    if (pCreateInfo->offset >= buffer_state.createInfo.size) {
        skip |= LogError("VUID-VkBufferViewCreateInfo-offset-00925", buffer_state.buffer(), create_info_loc.dot(Field::offset),
                         "(%" PRIuLEAST64 ") must be less than the size of the buffer (%" PRIuLEAST64 ").", pCreateInfo->offset,
                         buffer_state.createInfo.size);
    }

    const VkPhysicalDeviceLimits *device_limits = &phys_dev_props.limits;
    // Buffer view offset must be a multiple of VkPhysicalDeviceLimits::minTexelBufferOffsetAlignment
    if ((pCreateInfo->offset % device_limits->minTexelBufferOffsetAlignment) != 0 && !enabled_features.texelBufferAlignment) {
        skip |= LogError("VUID-VkBufferViewCreateInfo-offset-02749", objlist, create_info_loc.dot(Field::offset),
                         "(%" PRIuLEAST64
                         ") must be a multiple of VkPhysicalDeviceLimits::minTexelBufferOffsetAlignment (%" PRIuLEAST64 ").",
                         pCreateInfo->offset, device_limits->minTexelBufferOffsetAlignment);
    }

    if (enabled_features.texelBufferAlignment) {
        VkDeviceSize element_size = vkuFormatElementSize(pCreateInfo->format);
        if ((element_size % 3) == 0) {
            element_size /= 3;
        }
        if (buffer_state.usage & VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT) {
            VkDeviceSize alignment_requirement =
                phys_dev_ext_props.texel_buffer_alignment_props.storageTexelBufferOffsetAlignmentBytes;
            if (phys_dev_ext_props.texel_buffer_alignment_props.storageTexelBufferOffsetSingleTexelAlignment) {
                alignment_requirement = std::min(alignment_requirement, element_size);
            }
            if (SafeModulo(pCreateInfo->offset, alignment_requirement) != 0) {
                skip |= LogError(
                    "VUID-VkBufferViewCreateInfo-buffer-02750", objlist, create_info_loc,
                    "If buffer was created with usage containing "
                    "VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT, "
                    "VkBufferViewCreateInfo offset (%" PRIuLEAST64
                    ") must be a multiple of the lesser of "
                    "VkPhysicalDeviceTexelBufferAlignmentPropertiesEXT::storageTexelBufferOffsetAlignmentBytes (%" PRIuLEAST64
                    ") or, if VkPhysicalDeviceTexelBufferAlignmentPropertiesEXT::storageTexelBufferOffsetSingleTexelAlignment "
                    "(%" PRId32
                    ") is VK_TRUE, the size of a texel of the requested format. "
                    "If the size of a texel is a multiple of three bytes, then the size of a "
                    "single component of format is used instead",
                    pCreateInfo->offset, phys_dev_ext_props.texel_buffer_alignment_props.storageTexelBufferOffsetAlignmentBytes,
                    phys_dev_ext_props.texel_buffer_alignment_props.storageTexelBufferOffsetSingleTexelAlignment);
            }
        }
        if (buffer_state.usage & VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT) {
            VkDeviceSize alignment_requirement =
                phys_dev_ext_props.texel_buffer_alignment_props.uniformTexelBufferOffsetAlignmentBytes;
            if (phys_dev_ext_props.texel_buffer_alignment_props.uniformTexelBufferOffsetSingleTexelAlignment) {
                alignment_requirement = std::min(alignment_requirement, element_size);
            }
            if (SafeModulo(pCreateInfo->offset, alignment_requirement) != 0) {
                skip |= LogError(
                    "VUID-VkBufferViewCreateInfo-buffer-02751", objlist, create_info_loc,
                    "If buffer was created with usage containing "
                    "VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT, "
                    "VkBufferViewCreateInfo offset (%" PRIuLEAST64
                    ") must be a multiple of the lesser of "
                    "VkPhysicalDeviceTexelBufferAlignmentPropertiesEXT::uniformTexelBufferOffsetAlignmentBytes (%" PRIuLEAST64
                    ") or, if VkPhysicalDeviceTexelBufferAlignmentPropertiesEXT::uniformTexelBufferOffsetSingleTexelAlignment "
                    "(%" PRId32
                    ") is VK_TRUE, the size of a texel of the requested format. "
                    "If the size of a texel is a multiple of three bytes, then the size of a "
                    "single component of format is used instead",
                    pCreateInfo->offset, phys_dev_ext_props.texel_buffer_alignment_props.uniformTexelBufferOffsetAlignmentBytes,
                    phys_dev_ext_props.texel_buffer_alignment_props.uniformTexelBufferOffsetSingleTexelAlignment);
            }
        }
    }

    if (auto buffer_usage_flags2 = vku::FindStructInPNextChain<VkBufferUsageFlags2CreateInfoKHR>(pCreateInfo->pNext)) {
        const VkBufferUsageFlags2KHR usage = buffer_usage_flags2->usage;
        if ((usage & ~(VK_BUFFER_USAGE_2_UNIFORM_TEXEL_BUFFER_BIT_KHR | VK_BUFFER_USAGE_2_STORAGE_TEXEL_BUFFER_BIT_KHR)) != 0) {
            skip |= LogError("VUID-VkBufferViewCreateInfo-pNext-08780", objlist,
                             create_info_loc.pNext(Struct::VkBufferUsageFlags2CreateInfoKHR, Field::usage), "is %s.",
                             string_VkBufferUsageFlags2KHR(usage).c_str());
        } else if ((usage & buffer_state.usage) != buffer_state.usage) {
            skip |= LogError("VUID-VkBufferViewCreateInfo-pNext-08781", objlist,
                             create_info_loc.pNext(Struct::VkBufferUsageFlags2CreateInfoKHR, Field::usage),
                             "(%s) is not a subset of the buffer's usage (%s).", string_VkBufferUsageFlags2KHR(usage).c_str(),
                             string_VkBufferUsageFlags2KHR(buffer_state.usage).c_str());
        }
    }

    skip |= ValidateBufferViewRange(buffer_state, pCreateInfo, device_limits, create_info_loc);

    skip |= ValidateBufferViewBuffer(buffer_state, pCreateInfo, create_info_loc);
    return skip;
}

bool CoreChecks::PreCallValidateDestroyBuffer(VkDevice device, VkBuffer buffer, const VkAllocationCallbacks *pAllocator,
                                              const ErrorObject &error_obj) const {
    auto buffer_state = Get<BUFFER_STATE>(buffer);

    bool skip = false;
    if (buffer_state) {
        skip |= ValidateObjectNotInUse(buffer_state.get(), error_obj.location, "VUID-vkDestroyBuffer-buffer-00922");
    }
    return skip;
}

bool CoreChecks::PreCallValidateDestroyBufferView(VkDevice device, VkBufferView bufferView, const VkAllocationCallbacks *pAllocator,
                                                  const ErrorObject &error_obj) const {
    auto buffer_view_state = Get<BUFFER_VIEW_STATE>(bufferView);
    bool skip = false;
    if (buffer_view_state) {
        skip |= ValidateObjectNotInUse(buffer_view_state.get(), error_obj.location, "VUID-vkDestroyBufferView-bufferView-00936");
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                              VkDeviceSize size, uint32_t data, const ErrorObject &error_obj) const {
    bool skip = false;
    auto cb_state_ptr = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    auto buffer_state = Get<BUFFER_STATE>(dstBuffer);
    if (!cb_state_ptr || !buffer_state) {
        return skip;
    }
    const LogObjectList objlist(commandBuffer, dstBuffer);
    const CMD_BUFFER_STATE &cb_state = *cb_state_ptr;
    const Location buffer_loc = error_obj.location.dot(Field::dstBuffer);
    skip |= ValidateMemoryIsBoundToBuffer(commandBuffer, *buffer_state, buffer_loc, "VUID-vkCmdFillBuffer-dstBuffer-00031");
    skip |= ValidateCmd(cb_state, error_obj.location);
    // Validate that DST buffer has correct usage flags set
    skip |= ValidateBufferUsageFlags(objlist, *buffer_state, VK_BUFFER_USAGE_TRANSFER_DST_BIT, true,
                                     "VUID-vkCmdFillBuffer-dstBuffer-00029", buffer_loc);

    skip |= ValidateProtectedBuffer(cb_state, *buffer_state, buffer_loc, "VUID-vkCmdFillBuffer-commandBuffer-01811");
    skip |= ValidateUnprotectedBuffer(cb_state, *buffer_state, buffer_loc, "VUID-vkCmdFillBuffer-commandBuffer-01812");

    if (dstOffset >= buffer_state->createInfo.size) {
        skip |= LogError("VUID-vkCmdFillBuffer-dstOffset-00024", objlist, error_obj.location.dot(Field::dstOffset),
                         "(%" PRIu64 ") is not less than destination buffer (%s) size (%" PRIu64 ").", dstOffset,
                         FormatHandle(dstBuffer).c_str(), buffer_state->createInfo.size);
    }

    if ((size != VK_WHOLE_SIZE) && (size > (buffer_state->createInfo.size - dstOffset))) {
        skip |= LogError("VUID-vkCmdFillBuffer-size-00027", objlist, error_obj.location.dot(Field::size),
                         "(%" PRIu64 ") is greater than dstBuffer (%s) size (%" PRIu64 ") minus dstOffset (%" PRIu64 ").", size,
                         FormatHandle(dstBuffer).c_str(), buffer_state->createInfo.size, dstOffset);
    }

    if (!IsExtEnabled(device_extensions.vk_khr_maintenance1)) {
        skip |= ValidateCmdQueueFlags(cb_state, error_obj.location, VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT,
                                      "VUID-vkCmdFillBuffer-apiVersion-07894");
    }

    return skip;
}

// Validates the buffer is allowed to be protected
bool CoreChecks::ValidateProtectedBuffer(const CMD_BUFFER_STATE &cb_state, const BUFFER_STATE &buffer_state,
                                         const Location &buffer_loc, const char *vuid, const char *more_message) const {
    bool skip = false;

    // if driver supports protectedNoFault the operation is valid, just has undefined values
    if ((!phys_dev_props_core11.protectedNoFault) && (cb_state.unprotected == true) && (buffer_state.unprotected == false)) {
        const LogObjectList objlist(cb_state.Handle(), buffer_state.Handle());
        skip |= LogError(vuid, objlist, buffer_loc, "(%s) is a protected buffer, but command buffer (%s) is unprotected.%s",
                         FormatHandle(buffer_state).c_str(), FormatHandle(cb_state).c_str(), more_message);
    }
    return skip;
}

// Validates the buffer is allowed to be unprotected
bool CoreChecks::ValidateUnprotectedBuffer(const CMD_BUFFER_STATE &cb_state, const BUFFER_STATE &buffer_state,
                                           const Location &buffer_loc, const char *vuid, const char *more_message) const {
    bool skip = false;

    // if driver supports protectedNoFault the operation is valid, just has undefined values
    if ((!phys_dev_props_core11.protectedNoFault) && (cb_state.unprotected == false) && (buffer_state.unprotected == true)) {
        const LogObjectList objlist(cb_state.Handle(), buffer_state.Handle());
        skip |= LogError(vuid, objlist, buffer_loc, "(%s) is an unprotected buffer, but command buffer (%s) is protected.%s",
                         FormatHandle(buffer_state).c_str(), FormatHandle(cb_state).c_str(), more_message);
    }
    return skip;
}
