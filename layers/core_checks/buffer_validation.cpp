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

#include <algorithm>
#include <assert.h>
#include <sstream>
#include <string>
#include <vector>

#include "vk_enum_string_helper.h"
#include "chassis.h"
#include "core_checks/core_validation.h"

// Helper function to validate usage flags for buffers. For given buffer_state send actual vs. desired usage off to helper above
// where an error will be flagged if usage is not correct
template <typename HandleT>
bool CoreChecks::ValidateBufferUsageFlags(HandleT handle, BUFFER_STATE const &buffer_state, VkFlags desired, bool strict,
                                          const char *msgCode, char const *func_name, char const *usage_string) const {
    LogObjectList objlist(handle, buffer_state.Handle());
    return ValidateUsageFlags(buffer_state.createInfo.usage, desired, strict, objlist, buffer_state.Handle(), msgCode, func_name,
                              usage_string);
}
// explictly instantiate templates needed by other .cpp files
template bool CoreChecks::ValidateBufferUsageFlags(VkCommandBuffer, BUFFER_STATE const &, unsigned int, bool, char const *,
                                                   char const *, char const *) const;
template bool CoreChecks::ValidateBufferUsageFlags(VkDevice, BUFFER_STATE const &, unsigned int, bool, char const *, char const *,
                                                   char const *) const;

bool CoreChecks::ValidateBufferViewRange(const BUFFER_STATE *buffer_state, const VkBufferViewCreateInfo *pCreateInfo,
                                         const VkPhysicalDeviceLimits *device_limits) const {
    bool skip = false;

    const VkDeviceSize &range = pCreateInfo->range;
    if (range != VK_WHOLE_SIZE) {
        // Range must be greater than 0
        if (range <= 0) {
            skip |= LogError(buffer_state->buffer(), "VUID-VkBufferViewCreateInfo-range-00928",
                             "vkCreateBufferView(): If VkBufferViewCreateInfo range (%" PRIuLEAST64
                             ") does not equal VK_WHOLE_SIZE, range must be greater than 0.",
                             range);
        }
        // Range must be a multiple of the element size of format
        const uint32_t format_size = FormatElementSize(pCreateInfo->format);
        if (SafeModulo(range, format_size) != 0) {
            skip |= LogError(buffer_state->buffer(), "VUID-VkBufferViewCreateInfo-range-00929",
                             "vkCreateBufferView(): If VkBufferViewCreateInfo range (%" PRIuLEAST64
                             ") does not equal VK_WHOLE_SIZE, range must be a multiple of the element size of the format "
                             "(%" PRIu32 ").",
                             range, format_size);
        }
        // Range divided by the element size of format must be less than or equal to VkPhysicalDeviceLimits::maxTexelBufferElements
        if (SafeDivision(range, format_size) > device_limits->maxTexelBufferElements) {
            skip |= LogError(buffer_state->buffer(), "VUID-VkBufferViewCreateInfo-range-00930",
                             "vkCreateBufferView(): If VkBufferViewCreateInfo range (%" PRIuLEAST64
                             ") does not equal VK_WHOLE_SIZE, range divided by the element size of the format (%" PRIu32
                             ") must be less than or equal to VkPhysicalDeviceLimits::maxTexelBufferElements (%" PRIuLEAST32 ").",
                             range, format_size, device_limits->maxTexelBufferElements);
        }
        // The sum of range and offset must be less than or equal to the size of buffer
        if (range + pCreateInfo->offset > buffer_state->createInfo.size) {
            skip |= LogError(buffer_state->buffer(), "VUID-VkBufferViewCreateInfo-offset-00931",
                             "vkCreateBufferView(): If VkBufferViewCreateInfo range (%" PRIuLEAST64
                             ") does not equal VK_WHOLE_SIZE, the sum of offset (%" PRIuLEAST64
                             ") and range must be less than or equal to the size of the buffer (%" PRIuLEAST64 ").",
                             range, pCreateInfo->offset, buffer_state->createInfo.size);
        }
    } else {
        const uint32_t format_size = FormatElementSize(pCreateInfo->format);

        // Size of buffer - offset, divided by the element size of format must be less than or equal to
        // VkPhysicalDeviceLimits::maxTexelBufferElements
        if (SafeDivision(buffer_state->createInfo.size - pCreateInfo->offset, format_size) >
            device_limits->maxTexelBufferElements) {
            skip |= LogError(buffer_state->buffer(), "VUID-VkBufferViewCreateInfo-range-04059",
                             "vkCreateBufferView(): If VkBufferViewCreateInfo range (%" PRIuLEAST64
                             ") equals VK_WHOLE_SIZE, the buffer's size (%" PRIuLEAST64 ") minus the offset (%" PRIuLEAST64
                             "), divided by the element size of the format (%" PRIu32
                             ") must be less than or equal to VkPhysicalDeviceLimits::maxTexelBufferElements (%" PRIuLEAST32 ").",
                             range, buffer_state->createInfo.size, pCreateInfo->offset, format_size,
                             device_limits->maxTexelBufferElements);
        }
    }
    return skip;
}

bool CoreChecks::ValidateBufferViewBuffer(const BUFFER_STATE *buffer_state, const VkBufferViewCreateInfo *pCreateInfo) const {
    bool skip = false;
    const VkFormatProperties3KHR format_properties = GetPDFormatProperties(pCreateInfo->format);
    if ((buffer_state->createInfo.usage & VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT) &&
        !(format_properties.bufferFeatures & VK_FORMAT_FEATURE_2_UNIFORM_TEXEL_BUFFER_BIT_KHR)) {
        skip |= LogError(buffer_state->buffer(), "VUID-VkBufferViewCreateInfo-buffer-00933",
                         "vkCreateBufferView(): If buffer was created with `usage` containing "
                         "VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT, format (%s) must "
                         "be supported for uniform texel buffers",
                         string_VkFormat(pCreateInfo->format));
    }
    if ((buffer_state->createInfo.usage & VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT) &&
        !(format_properties.bufferFeatures & VK_FORMAT_FEATURE_2_STORAGE_TEXEL_BUFFER_BIT_KHR)) {
        skip |= LogError(buffer_state->buffer(), "VUID-VkBufferViewCreateInfo-buffer-00934",
                         "vkCreateBufferView(): If buffer was created with `usage` containing "
                         "VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT, format (%s) must "
                         "be supported for storage texel buffers",
                         string_VkFormat(pCreateInfo->format));
    }
    return skip;
}

bool CoreChecks::PreCallValidateCreateBuffer(VkDevice device, const VkBufferCreateInfo *pCreateInfo,
                                             const VkAllocationCallbacks *pAllocator, VkBuffer *pBuffer) const {
    bool skip = false;

    auto chained_devaddr_struct = LvlFindInChain<VkBufferDeviceAddressCreateInfoEXT>(pCreateInfo->pNext);
    if (chained_devaddr_struct) {
        if (!(pCreateInfo->flags & VK_BUFFER_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT) &&
            chained_devaddr_struct->deviceAddress != 0) {
            skip |= LogError(device, "VUID-VkBufferCreateInfo-deviceAddress-02604",
                             "vkCreateBuffer(): Non-zero VkBufferDeviceAddressCreateInfoEXT::deviceAddress "
                             "requires VK_BUFFER_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT.");
        }
    }

    auto chained_opaqueaddr_struct = LvlFindInChain<VkBufferOpaqueCaptureAddressCreateInfo>(pCreateInfo->pNext);
    if (chained_opaqueaddr_struct) {
        if (!(pCreateInfo->flags & VK_BUFFER_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT) &&
            chained_opaqueaddr_struct->opaqueCaptureAddress != 0) {
            skip |= LogError(device, "VUID-VkBufferCreateInfo-opaqueCaptureAddress-03337",
                             "vkCreateBuffer(): Non-zero VkBufferOpaqueCaptureAddressCreateInfo::opaqueCaptureAddress"
                             "requires VK_BUFFER_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT.");
        }
    }

    auto dedicated_allocation_buffer = LvlFindInChain<VkDedicatedAllocationBufferCreateInfoNV>(pCreateInfo->pNext);
    if (dedicated_allocation_buffer && dedicated_allocation_buffer->dedicatedAllocation == VK_TRUE) {
        if (pCreateInfo->flags &
            (VK_BUFFER_CREATE_SPARSE_BINDING_BIT | VK_BUFFER_CREATE_SPARSE_RESIDENCY_BIT | VK_BUFFER_CREATE_SPARSE_ALIASED_BIT)) {
            skip |= LogError(device, "VUID-VkBufferCreateInfo-pNext-01571",
                             "vkCreateBuffer(): pCreateInfos->flags must not include VK_BUFFER_CREATE_SPARSE_BINDING_BIT, "
                             "VK_BUFFER_CREATE_SPARSE_RESIDENCY_BIT, or VK_BUFFER_CREATE_SPARSE_ALIASED_BIT when "
                             "VkDedicatedAllocationBufferCreateInfoNV is in pNext chain with dedicatedAllocation VK_TRUE.");
        }
    }

    if ((pCreateInfo->flags & VK_BUFFER_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT) &&
        !enabled_features.core12.bufferDeviceAddressCaptureReplay &&
        !enabled_features.buffer_device_address_ext_features.bufferDeviceAddressCaptureReplay) {
        skip |= LogError(
            device, "VUID-VkBufferCreateInfo-flags-03338",
            "vkCreateBuffer(): the bufferDeviceAddressCaptureReplay device feature is disabled: Buffers cannot be created with "
            "the VK_BUFFER_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT set.");
    }

    if (pCreateInfo->sharingMode == VK_SHARING_MODE_CONCURRENT && pCreateInfo->pQueueFamilyIndices) {
        const char *vuid = IsExtEnabled(device_extensions.vk_khr_get_physical_device_properties2)
                               ? "VUID-VkBufferCreateInfo-sharingMode-01419"
                               : "VUID-VkBufferCreateInfo-sharingMode-01391";
        skip |= ValidatePhysicalDeviceQueueFamilies(pCreateInfo->queueFamilyIndexCount, pCreateInfo->pQueueFamilyIndices,
                                                    "vkCreateBuffer", "pCreateInfo->pQueueFamilyIndices", vuid);
    }

    if ((pCreateInfo->flags & VK_BUFFER_CREATE_PROTECTED_BIT) != 0) {
        if (enabled_features.core11.protectedMemory == VK_FALSE) {
            skip |= LogError(device, "VUID-VkBufferCreateInfo-flags-01887",
                             "vkCreateBuffer(): the protectedMemory device feature is disabled: Buffers cannot be created with the "
                             "VK_BUFFER_CREATE_PROTECTED_BIT set.");
        }
        const VkBufferCreateFlags invalid_flags =
            VK_BUFFER_CREATE_SPARSE_BINDING_BIT | VK_BUFFER_CREATE_SPARSE_RESIDENCY_BIT | VK_BUFFER_CREATE_SPARSE_ALIASED_BIT;
        if ((pCreateInfo->flags & invalid_flags) != 0) {
            skip |= LogError(device, "VUID-VkBufferCreateInfo-None-01888",
                             "vkCreateBuffer(): VK_BUFFER_CREATE_PROTECTED_BIT is set so no sparse create flags can be used at "
                             "same time (VK_BUFFER_CREATE_SPARSE_BINDING_BIT | VK_BUFFER_CREATE_SPARSE_RESIDENCY_BIT | "
                             "VK_BUFFER_CREATE_SPARSE_ALIASED_BIT).");
        }
    }

    bool has_decode_usage =
        pCreateInfo->usage & (VK_BUFFER_USAGE_VIDEO_DECODE_SRC_BIT_KHR | VK_BUFFER_USAGE_VIDEO_DECODE_DST_BIT_KHR);
    bool has_encode_usage =
        pCreateInfo->usage & (VK_BUFFER_USAGE_VIDEO_ENCODE_SRC_BIT_KHR | VK_BUFFER_USAGE_VIDEO_ENCODE_DST_BIT_KHR);
    if (has_decode_usage || has_encode_usage) {
        const auto *video_profiles = LvlFindInChain<VkVideoProfileListInfoKHR>(pCreateInfo->pNext);
        skip |= ValidateVideoProfileListInfo(video_profiles, device, "vkCreateBuffer", has_decode_usage,
                                             "VUID-VkBufferCreateInfo-usage-04813", has_encode_usage,
                                             "VUID-VkBufferCreateInfo-usage-04814");
    }

    if (pCreateInfo->usage & VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT) {
        if (pCreateInfo->size + samplerDescriptorBufferAddressSpaceSize >
            phys_dev_ext_props.descriptor_buffer_props.samplerDescriptorBufferAddressSpaceSize) {
            skip |= LogError(
                device, "VUID-VkBufferCreateInfo-usage-08097",
                "vkCreateBuffer(): Requested buffer size (%" PRIuLEAST64 ") plus current total (%" PRIuLEAST64
                ") is greater than specified in properties field samplerDescriptorBufferAddressSpaceSize (%" PRIuLEAST64 ").",
                pCreateInfo->size, samplerDescriptorBufferAddressSpaceSize.load(),
                phys_dev_ext_props.descriptor_buffer_props.samplerDescriptorBufferAddressSpaceSize);
        }

        if (pCreateInfo->size + descriptorBufferAddressSpaceSize >
            phys_dev_ext_props.descriptor_buffer_props.descriptorBufferAddressSpaceSize) {
            skip |= LogError(device, "VUID-VkBufferCreateInfo-usage-08097",
                             "vkCreateBuffer(): Requested buffer size (%" PRIuLEAST64 ") plus current total (%" PRIuLEAST64
                             ") is greater than specified in properties field descriptorBufferAddressSpaceSize (%" PRIuLEAST64 ")",
                             pCreateInfo->size, descriptorBufferAddressSpaceSize.load(),
                             phys_dev_ext_props.descriptor_buffer_props.descriptorBufferAddressSpaceSize);
        }
    }

    if (pCreateInfo->usage & VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT) {
        if (pCreateInfo->size + resourceDescriptorBufferAddressSpaceSize >
            phys_dev_ext_props.descriptor_buffer_props.resourceDescriptorBufferAddressSpaceSize) {
            skip |= LogError(
                device, "VUID-VkBufferCreateInfo-usage-08098",
                "vkCreateBuffer(): Requested buffer size (%" PRIuLEAST64 ") plus current total (%" PRIuLEAST64
                ") is greater than specified in properties field resourceDescriptorBufferAddressSpaceSize (%" PRIuLEAST64 ").",
                pCreateInfo->size, resourceDescriptorBufferAddressSpaceSize.load(),
                phys_dev_ext_props.descriptor_buffer_props.resourceDescriptorBufferAddressSpaceSize);
        }

        if (pCreateInfo->size + descriptorBufferAddressSpaceSize >
            phys_dev_ext_props.descriptor_buffer_props.descriptorBufferAddressSpaceSize) {
            skip |= LogError(device, "VUID-VkBufferCreateInfo-usage-08098",
                             "vkCreateBuffer(): Requested buffer size (%" PRIuLEAST64 ") plus current total (%" PRIuLEAST64
                             ") is greater than specified in properties field descriptorBufferAddressSpaceSize (%" PRIuLEAST64 ").",
                             pCreateInfo->size, descriptorBufferAddressSpaceSize.load(),
                             phys_dev_ext_props.descriptor_buffer_props.descriptorBufferAddressSpaceSize);
        }
    }

    if ((pCreateInfo->flags & VK_BUFFER_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT) &&
        !enabled_features.descriptor_buffer_features.descriptorBufferCaptureReplay) {
        skip |= LogError(
            device, "VUID-VkBufferCreateInfo-flags-08099",
            "vkCreateBuffer(): the descriptorBufferCaptureReplay device feature is disabled: Buffers cannot be created with "
            "the VK_BUFFER_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT.");
    }

    auto opaque_capture_descriptor_buffer = LvlFindInChain<VkOpaqueCaptureDescriptorDataCreateInfoEXT>(pCreateInfo->pNext);
    if (opaque_capture_descriptor_buffer && !(pCreateInfo->flags & VK_BUFFER_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT)) {
        skip |= LogError(device, "VUID-VkBufferCreateInfo-pNext-08100",
                         "vkCreateBuffer(): VkOpaqueCaptureDescriptorDataCreateInfoEXT is in pNext chain, but "
                         "VK_BUFFER_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT is not set.");
    }

    if (pCreateInfo->usage & VK_BUFFER_USAGE_PUSH_DESCRIPTORS_DESCRIPTOR_BUFFER_BIT_EXT) {
        if (!enabled_features.descriptor_buffer_features.descriptorBufferPushDescriptors) {
            skip |= LogError(
                device, "VUID-VkBufferCreateInfo-usage-08101",
                "vkCreateBuffer(): the descriptorBufferPushDescriptors device feature is disabled: Buffers cannot be created with "
                "VK_BUFFER_USAGE_PUSH_DESCRIPTORS_DESCRIPTOR_BUFFER_BIT_EXT set.");
        }

        if (phys_dev_ext_props.descriptor_buffer_props.bufferlessPushDescriptors) {
            skip |= LogError(
                device, "VUID-VkBufferCreateInfo-usage-08102",
                "vkCreateBuffer(): the bufferlessPushDescriptors device feature is VK_TRUE: Buffers cannot be created with "
                "VK_BUFFER_USAGE_PUSH_DESCRIPTORS_DESCRIPTOR_BUFFER_BIT_EXT set.");
        }

        if (!(pCreateInfo->usage &
              (VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT | VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT))) {
            skip |= LogError(
                device, "VUID-VkBufferCreateInfo-usage-08103",
                "vkCreateBuffer(): If VK_BUFFER_USAGE_PUSH_DESCRIPTORS_DESCRIPTOR_BUFFER_BIT_EXT is set, usage must also contain "
                "one of VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT or VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT.");
        }
    }

    auto external_memory_info = LvlFindInChain<VkExternalMemoryBufferCreateInfo>(pCreateInfo->pNext);
    if (external_memory_info && external_memory_info->handleTypes) {
        const uint32_t any_type = 1u << MostSignificantBit(external_memory_info->handleTypes);
        auto external_buffer_info = LvlInitStruct<VkPhysicalDeviceExternalBufferInfo>();
        external_buffer_info.flags = pCreateInfo->flags;
        external_buffer_info.usage = pCreateInfo->usage;
        external_buffer_info.handleType = static_cast<VkExternalMemoryHandleTypeFlagBits>(any_type);
        auto external_buffer_properties = LvlInitStruct<VkExternalBufferProperties>();
        DispatchGetPhysicalDeviceExternalBufferProperties(physical_device, &external_buffer_info, &external_buffer_properties);
        const auto compatible_types = external_buffer_properties.externalMemoryProperties.compatibleHandleTypes;

        if ((external_memory_info->handleTypes & compatible_types) != external_memory_info->handleTypes) {
            skip |= LogError(
                device, "VUID-VkBufferCreateInfo-pNext-00920",
                "vkCreateBuffer(): VkBufferCreateInfo pNext chain contains VkExternalMemoryBufferCreateInfo with handleTypes flags "
                "(%s) that are not reported as compatible by vkGetPhysicalDeviceExternalBufferProperties.",
                string_VkExternalMemoryHandleTypeFlags(external_memory_info->handleTypes).c_str());
        }
    }

    return skip;
}

bool CoreChecks::PreCallValidateCreateBufferView(VkDevice device, const VkBufferViewCreateInfo *pCreateInfo,
                                                 const VkAllocationCallbacks *pAllocator, VkBufferView *pView) const {
    bool skip = false;
    auto buffer_state = Get<BUFFER_STATE>(pCreateInfo->buffer);

    if (FormatIsDepthOrStencil(pCreateInfo->format)) {
        // Should never hopefully get here, but there are known driver advertising the wrong feature flags
        // see https://gitlab.khronos.org/vulkan/vulkan/-/merge_requests/4849
        skip |= LogError(device, kVUID_Core_invalidDepthStencilFormat,
                         "vkCreateBufferView(): format is a depth/stencil format (%s) but depth/stencil formats do not have a "
                         "defined sizes for alignment, replace with a color format.",
                         string_VkFormat(pCreateInfo->format));
    }

    // If this isn't a sparse buffer, it needs to have memory backing it at CreateBufferView time
    if (buffer_state) {
        skip |= ValidateMemoryIsBoundToBuffer(device, *buffer_state, "vkCreateBufferView()",
                                              "VUID-VkBufferViewCreateInfo-buffer-00935");
        // In order to create a valid buffer view, the buffer must have been created with at least one of the following flags:
        // UNIFORM_TEXEL_BUFFER_BIT or STORAGE_TEXEL_BUFFER_BIT
        skip |= ValidateBufferUsageFlags(device, *buffer_state,
                                         VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT, false,
                                         "VUID-VkBufferViewCreateInfo-buffer-00932", "vkCreateBufferView()",
                                         "VK_BUFFER_USAGE_[STORAGE|UNIFORM]_TEXEL_BUFFER_BIT");

        // Buffer view offset must be less than the size of buffer
        if (pCreateInfo->offset >= buffer_state->createInfo.size) {
            skip |= LogError(buffer_state->buffer(), "VUID-VkBufferViewCreateInfo-offset-00925",
                             "vkCreateBufferView(): VkBufferViewCreateInfo offset (%" PRIuLEAST64
                             ") must be less than the size of the buffer (%" PRIuLEAST64 ").",
                             pCreateInfo->offset, buffer_state->createInfo.size);
        }

        const VkPhysicalDeviceLimits *device_limits = &phys_dev_props.limits;
        // Buffer view offset must be a multiple of VkPhysicalDeviceLimits::minTexelBufferOffsetAlignment
        if ((pCreateInfo->offset % device_limits->minTexelBufferOffsetAlignment) != 0 &&
            !enabled_features.texel_buffer_alignment_features.texelBufferAlignment) {
            const char *vuid = IsExtEnabled(device_extensions.vk_ext_texel_buffer_alignment)
                                   ? "VUID-VkBufferViewCreateInfo-offset-02749"
                                   : "VUID-VkBufferViewCreateInfo-offset-00926";
            skip |= LogError(buffer_state->buffer(), vuid,
                             "vkCreateBufferView(): VkBufferViewCreateInfo offset (%" PRIuLEAST64
                             ") must be a multiple of VkPhysicalDeviceLimits::minTexelBufferOffsetAlignment (%" PRIuLEAST64 ").",
                             pCreateInfo->offset, device_limits->minTexelBufferOffsetAlignment);
        }

        if (enabled_features.texel_buffer_alignment_features.texelBufferAlignment) {
            VkDeviceSize element_size = FormatElementSize(pCreateInfo->format);
            if ((element_size % 3) == 0) {
                element_size /= 3;
            }
            if (buffer_state->createInfo.usage & VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT) {
                VkDeviceSize alignment_requirement =
                    phys_dev_ext_props.texel_buffer_alignment_props.storageTexelBufferOffsetAlignmentBytes;
                if (phys_dev_ext_props.texel_buffer_alignment_props.storageTexelBufferOffsetSingleTexelAlignment) {
                    alignment_requirement = std::min(alignment_requirement, element_size);
                }
                if (SafeModulo(pCreateInfo->offset, alignment_requirement) != 0) {
                    skip |= LogError(
                        buffer_state->buffer(), "VUID-VkBufferViewCreateInfo-buffer-02750",
                        "vkCreateBufferView(): If buffer was created with usage containing "
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
            if (buffer_state->createInfo.usage & VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT) {
                VkDeviceSize alignment_requirement =
                    phys_dev_ext_props.texel_buffer_alignment_props.uniformTexelBufferOffsetAlignmentBytes;
                if (phys_dev_ext_props.texel_buffer_alignment_props.uniformTexelBufferOffsetSingleTexelAlignment) {
                    alignment_requirement = std::min(alignment_requirement, element_size);
                }
                if (SafeModulo(pCreateInfo->offset, alignment_requirement) != 0) {
                    skip |= LogError(
                        buffer_state->buffer(), "VUID-VkBufferViewCreateInfo-buffer-02751",
                        "vkCreateBufferView(): If buffer was created with usage containing "
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

        skip |= ValidateBufferViewRange(buffer_state.get(), pCreateInfo, device_limits);

        skip |= ValidateBufferViewBuffer(buffer_state.get(), pCreateInfo);
    }
    return skip;
}

bool CoreChecks::PreCallValidateDestroyBuffer(VkDevice device, VkBuffer buffer, const VkAllocationCallbacks *pAllocator) const {
    auto buffer_state = Get<BUFFER_STATE>(buffer);

    bool skip = false;
    if (buffer_state) {
        skip |= ValidateObjectNotInUse(buffer_state.get(), "vkDestroyBuffer", "VUID-vkDestroyBuffer-buffer-00922");
    }
    return skip;
}

bool CoreChecks::PreCallValidateDestroyBufferView(VkDevice device, VkBufferView bufferView,
                                                  const VkAllocationCallbacks *pAllocator) const {
    auto buffer_view_state = Get<BUFFER_VIEW_STATE>(bufferView);
    bool skip = false;
    if (buffer_view_state) {
        skip |= ValidateObjectNotInUse(buffer_view_state.get(), "vkDestroyBufferView", "VUID-vkDestroyBufferView-bufferView-00936");
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                              VkDeviceSize size, uint32_t data) const {
    bool skip = false;
    auto cb_state_ptr = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    auto buffer_state = Get<BUFFER_STATE>(dstBuffer);
    if (!cb_state_ptr || !buffer_state) {
        return skip;
    }
    const CMD_BUFFER_STATE &cb_state = *cb_state_ptr;
    skip |=
        ValidateMemoryIsBoundToBuffer(commandBuffer, *buffer_state, "vkCmdFillBuffer()", "VUID-vkCmdFillBuffer-dstBuffer-00031");
    skip |= ValidateCmd(cb_state, CMD_FILLBUFFER);
    // Validate that DST buffer has correct usage flags set
    skip |=
        ValidateBufferUsageFlags(commandBuffer, *buffer_state, VK_BUFFER_USAGE_TRANSFER_DST_BIT, true,
                                 "VUID-vkCmdFillBuffer-dstBuffer-00029", "vkCmdFillBuffer()", "VK_BUFFER_USAGE_TRANSFER_DST_BIT");

    skip |= ValidateProtectedBuffer(cb_state, *buffer_state, "vkCmdFillBuffer()", "VUID-vkCmdFillBuffer-commandBuffer-01811");
    skip |= ValidateUnprotectedBuffer(cb_state, *buffer_state, "vkCmdFillBuffer()", "VUID-vkCmdFillBuffer-commandBuffer-01812");

    if (dstOffset >= buffer_state->createInfo.size) {
        skip |= LogError(dstBuffer, "VUID-vkCmdFillBuffer-dstOffset-00024",
                         "vkCmdFillBuffer(): dstOffset (0x%" PRIxLEAST64
                         ") is not less than destination buffer (%s) size (0x%" PRIxLEAST64 ").",
                         dstOffset, report_data->FormatHandle(dstBuffer).c_str(), buffer_state->createInfo.size);
    }

    if ((size != VK_WHOLE_SIZE) && (size > (buffer_state->createInfo.size - dstOffset))) {
        skip |= LogError(dstBuffer, "VUID-vkCmdFillBuffer-size-00027",
                         "vkCmdFillBuffer(): size (0x%" PRIxLEAST64 ") is greater than dstBuffer (%s) size (0x%" PRIxLEAST64
                         ") minus dstOffset (0x%" PRIxLEAST64 ").",
                         size, report_data->FormatHandle(dstBuffer).c_str(), buffer_state->createInfo.size, dstOffset);
    }

    if (!IsExtEnabled(device_extensions.vk_khr_maintenance1)) {
        skip |= ValidateCmdQueueFlags(cb_state, "vkCmdFillBuffer()", VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT,
                                      "VUID-vkCmdFillBuffer-commandBuffer-00030");
    }

    return skip;
}

// Validates the buffer is allowed to be protected
bool CoreChecks::ValidateProtectedBuffer(const CMD_BUFFER_STATE &cb_state, const BUFFER_STATE &buffer_state, const char *cmd_name,
                                         const char *vuid, const char *more_message) const {
    bool skip = false;

    // if driver supports protectedNoFault the operation is valid, just has undefined values
    if ((!phys_dev_props_core11.protectedNoFault) && (cb_state.unprotected == true) && (buffer_state.unprotected == false)) {
        const LogObjectList objlist(cb_state.Handle(), buffer_state.Handle());
        skip |= LogError(objlist, vuid, "%s: command buffer %s is unprotected while buffer %s is a protected buffer.%s", cmd_name,
                         report_data->FormatHandle(cb_state.Handle()).c_str(),
                         report_data->FormatHandle(buffer_state.Handle()).c_str(), more_message);
    }
    return skip;
}

// Validates the buffer is allowed to be unprotected
bool CoreChecks::ValidateUnprotectedBuffer(const CMD_BUFFER_STATE &cb_state, const BUFFER_STATE &buffer_state, const char *cmd_name,
                                           const char *vuid, const char *more_message) const {
    bool skip = false;

    // if driver supports protectedNoFault the operation is valid, just has undefined values
    if ((!phys_dev_props_core11.protectedNoFault) && (cb_state.unprotected == false) && (buffer_state.unprotected == true)) {
        const LogObjectList objlist(cb_state.Handle(), buffer_state.Handle());
        skip |= LogError(objlist, vuid, "%s: command buffer %s is protected while buffer %s is an unprotected buffer.%s", cmd_name,
                         report_data->FormatHandle(cb_state.Handle()).c_str(),
                         report_data->FormatHandle(buffer_state.Handle()).c_str(), more_message);
    }
    return skip;
}
