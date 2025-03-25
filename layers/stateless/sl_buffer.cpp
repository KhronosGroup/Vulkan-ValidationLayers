/* Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (C) 2015-2025 Google Inc.
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

namespace stateless {

bool Device::manual_PreCallValidateCreateBuffer(VkDevice device, const VkBufferCreateInfo *pCreateInfo,
                                                const VkAllocationCallbacks *pAllocator, VkBuffer *pBuffer,
                                                const stateless::Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;

    const Location create_info_loc = error_obj.location.dot(Field::pCreateInfo);
    skip |= context.ValidateNotZero(pCreateInfo->size == 0, "VUID-VkBufferCreateInfo-size-00912", create_info_loc.dot(Field::size));

    // Validation for parameters excluded from the generated validation code due to a 'noautovalidity' tag in vk.xml
    if (pCreateInfo->sharingMode == VK_SHARING_MODE_CONCURRENT) {
        // If sharingMode is VK_SHARING_MODE_CONCURRENT, queueFamilyIndexCount must be greater than 1
        if (pCreateInfo->queueFamilyIndexCount <= 1) {
            skip |= LogError("VUID-VkBufferCreateInfo-sharingMode-00914", device, create_info_loc.dot(Field::sharingMode),
                             "is VK_SHARING_MODE_CONCURRENT, but queueFamilyIndexCount is %" PRIu32 ".",
                             pCreateInfo->queueFamilyIndexCount);
        }

        // If sharingMode is VK_SHARING_MODE_CONCURRENT, pQueueFamilyIndices must be a pointer to an array of
        // queueFamilyIndexCount uint32_t values
        if (pCreateInfo->pQueueFamilyIndices == nullptr) {
            skip |= LogError("VUID-VkBufferCreateInfo-sharingMode-00913", device, create_info_loc.dot(Field::sharingMode),
                             "is VK_SHARING_MODE_CONCURRENT, but pQueueFamilyIndices is NULL.");
        }
    }

    const auto *usage_flags2 = vku::FindStructInPNextChain<VkBufferUsageFlags2CreateInfo>(pCreateInfo->pNext);
    if (!usage_flags2) {
        skip |= context.ValidateFlags(create_info_loc.dot(Field::usage), vvl::FlagBitmask::VkBufferUsageFlagBits,
                                      AllVkBufferUsageFlagBits, pCreateInfo->usage, kRequiredFlags,
                                      "VUID-VkBufferCreateInfo-None-09499", "VUID-VkBufferCreateInfo-None-09500");
    }
    const VkBufferUsageFlags2KHR usage = usage_flags2 ? usage_flags2->usage : pCreateInfo->usage;

    if (pCreateInfo->flags & VK_BUFFER_CREATE_PROTECTED_BIT) {
        const VkBufferUsageFlags invalid =
            VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_BUFFER_BIT_EXT | VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_COUNTER_BUFFER_BIT_EXT |
            VK_BUFFER_USAGE_CONDITIONAL_RENDERING_BIT_EXT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
            VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR |
            VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT | VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT |
            VK_BUFFER_USAGE_PUSH_DESCRIPTORS_DESCRIPTOR_BUFFER_BIT_EXT | VK_BUFFER_USAGE_MICROMAP_BUILD_INPUT_READ_ONLY_BIT_EXT |
            VK_BUFFER_USAGE_MICROMAP_STORAGE_BIT_EXT;
        if (usage & invalid) {
            skip |= LogError("VUID-VkBufferCreateInfo-flags-09641", device, create_info_loc.dot(Field::flags),
                             "includes VK_BUFFER_CREATE_PROTECTED_BIT, but the usage is %s.",
                             string_VkBufferUsageFlags2(usage).c_str());
        }
    };

    auto dedicated_allocation_buffer = vku::FindStructInPNextChain<VkDedicatedAllocationBufferCreateInfoNV>(pCreateInfo->pNext);
    if (dedicated_allocation_buffer && dedicated_allocation_buffer->dedicatedAllocation == VK_TRUE) {
        if (pCreateInfo->flags &
            (VK_BUFFER_CREATE_SPARSE_BINDING_BIT | VK_BUFFER_CREATE_SPARSE_RESIDENCY_BIT | VK_BUFFER_CREATE_SPARSE_ALIASED_BIT)) {
            skip |= LogError("VUID-VkBufferCreateInfo-pNext-01571", device, create_info_loc.dot(Field::flags),
                             "%s when VkDedicatedAllocationBufferCreateInfoNV::dedicatedAllocation is VK_TRUE.",
                             string_VkBufferCreateFlags(pCreateInfo->flags).c_str());
        }
    }

    if (enabled_features.maintenance4 && pCreateInfo->size > phys_dev_props_core13.maxBufferSize) {
        skip |= LogError("VUID-VkBufferCreateInfo-size-06409", device, create_info_loc.dot(Field::size),
                         "(%" PRIu64
                         ") is larger than the maximum allowed buffer size "
                         "VkPhysicalDeviceMaintenance4Properties.maxBufferSize (%" PRIu64 ").",
                         pCreateInfo->size, phys_dev_props_core13.maxBufferSize);
    }

    skip |= ValidateCreateBufferFlags(pCreateInfo->flags, create_info_loc.dot(Field::flags));
    skip |= ValidateCreateBufferBufferDeviceAddress(*pCreateInfo, create_info_loc);

    return skip;
}

bool Device::manual_PreCallValidateCreateBufferView(VkDevice device, const VkBufferViewCreateInfo *pCreateInfo,
                                                    const VkAllocationCallbacks *pAllocator, VkBufferView *pBufferView,
                                                    const Context &context) const {
    bool skip = false;
#ifdef VK_USE_PLATFORM_METAL_EXT
    skip |= ExportMetalObjectsPNextUtil(VK_EXPORT_METAL_OBJECT_TYPE_METAL_TEXTURE_BIT_EXT,
                                        "VUID-VkBufferViewCreateInfo-pNext-06782", context.error_obj.location,
                                        "VK_EXPORT_METAL_OBJECT_TYPE_METAL_TEXTURE_BIT_EXT", pCreateInfo->pNext);
#endif  // VK_USE_PLATFORM_METAL_EXT

    const Location create_info_loc = context.error_obj.location.dot(Field::pCreateInfo);

    const VkDeviceSize &range = pCreateInfo->range;
    const VkFormat format = pCreateInfo->format;

    if (vkuFormatIsDepthOrStencil(format)) {
        // Should never hopefully get here, but there are known driver advertising the wrong feature flags
        // see https://gitlab.khronos.org/vulkan/vulkan/-/merge_requests/4849
        skip |= LogError("UNASSIGNED-VkBufferViewCreateInfo-depthStencil-format", pCreateInfo->buffer,
                         create_info_loc.dot(Field::format),
                         "is a depth/stencil format (%s) but depth/stencil formats do not have a "
                         "defined sizes for alignment, replace with a color format.",
                         string_VkFormat(format));
    }

    // Buffer view offset must be a multiple of VkPhysicalDeviceLimits::minTexelBufferOffsetAlignment
    if ((pCreateInfo->offset % phys_dev_props.limits.minTexelBufferOffsetAlignment) != 0 &&
        !enabled_features.texelBufferAlignment) {
        skip |= LogError("VUID-VkBufferViewCreateInfo-offset-02749", pCreateInfo->buffer, create_info_loc.dot(Field::offset),
                         "(%" PRIuLEAST64
                         ") must be a multiple of VkPhysicalDeviceLimits::minTexelBufferOffsetAlignment (%" PRIuLEAST64 ").",
                         pCreateInfo->offset, phys_dev_props.limits.minTexelBufferOffsetAlignment);
    }

    if (range != VK_WHOLE_SIZE) {
        // will be 1 because  block-compressed format are not supported for Texe l Buffer
        const VkDeviceSize texels_per_block = static_cast<VkDeviceSize>(vkuFormatTexelsPerBlock(format));
        const VkDeviceSize texel_block_size = static_cast<VkDeviceSize>(GetTexelBufferFormatSize(format));

        // Range must be greater than 0
        if (range <= 0) {
            skip |= LogError("VUID-VkBufferViewCreateInfo-range-00928", pCreateInfo->buffer, create_info_loc.dot(Field::range),
                             "(%" PRIuLEAST64 ") does not equal VK_WHOLE_SIZE, range must be greater than 0.", range);
        }
        // Range must be a multiple of the texel block size of format
        if (SafeModulo(range, texel_block_size) != 0) {
            skip |= LogError("VUID-VkBufferViewCreateInfo-range-00929", pCreateInfo->buffer, create_info_loc.dot(Field::range),
                             "(%" PRIuLEAST64
                             ") does not equal VK_WHOLE_SIZE, so it must be a multiple of the texel block size (%" PRIuLEAST64
                             ") of %s.",
                             range, texel_block_size, string_VkFormat(format));
        }
        const VkDeviceSize texels = SafeDivision(range, texel_block_size) * texels_per_block;
        if (texels > static_cast<VkDeviceSize>(phys_dev_props.limits.maxTexelBufferElements)) {
            skip |= LogError("VUID-VkBufferViewCreateInfo-range-00930", pCreateInfo->buffer, create_info_loc.dot(Field::range),
                             "(%" PRIuLEAST64 "), %s texel block size (%" PRIuLEAST64 "), and texels per block (%" PRIuLEAST64
                             ") is a total of (%" PRIuLEAST64
                             ") texels which is more than VkPhysicalDeviceLimits::maxTexelBufferElements (%" PRIuLEAST32 ").",
                             range, string_VkFormat(format), texel_block_size, texels_per_block, texels,
                             phys_dev_props.limits.maxTexelBufferElements);
        }
    }

    return skip;
}

bool Device::ValidateCreateBufferFlags(const VkBufferCreateFlags flags, const Location &flag_loc) const {
    bool skip = false;

    if ((flags & VK_BUFFER_CREATE_SPARSE_BINDING_BIT) && (!enabled_features.sparseBinding)) {
        skip |= LogError("VUID-VkBufferCreateInfo-flags-00915", device, flag_loc,
                         "includes VK_BUFFER_CREATE_SPARSE_BINDING_BIT, but the sparseBinding feature is not enabled.");
    }

    if ((flags & VK_BUFFER_CREATE_SPARSE_RESIDENCY_BIT) && (!enabled_features.sparseResidencyBuffer)) {
        skip |= LogError("VUID-VkBufferCreateInfo-flags-00916", device, flag_loc,
                         "includes VK_BUFFER_CREATE_SPARSE_RESIDENCY_BIT, but the sparseResidencyBuffer feature is not enabled.");
    }

    if ((flags & VK_BUFFER_CREATE_SPARSE_ALIASED_BIT) && (!enabled_features.sparseResidencyAliased)) {
        skip |= LogError("VUID-VkBufferCreateInfo-flags-00917", device, flag_loc,
                         "includes VK_BUFFER_CREATE_SPARSE_ALIASED_BIT, but the sparseResidencyAliased feature is not enabled.");
    };

    // If flags contains VK_BUFFER_CREATE_SPARSE_RESIDENCY_BIT or VK_BUFFER_CREATE_SPARSE_ALIASED_BIT, it must also contain
    // VK_BUFFER_CREATE_SPARSE_BINDING_BIT
    if (((flags & (VK_BUFFER_CREATE_SPARSE_RESIDENCY_BIT | VK_BUFFER_CREATE_SPARSE_ALIASED_BIT)) != 0) &&
        ((flags & VK_BUFFER_CREATE_SPARSE_BINDING_BIT) != VK_BUFFER_CREATE_SPARSE_BINDING_BIT)) {
        skip |=
            LogError("VUID-VkBufferCreateInfo-flags-00918", device, flag_loc, "is %s.", string_VkBufferCreateFlags(flags).c_str());
    }

    if ((flags & VK_BUFFER_CREATE_PROTECTED_BIT) != 0) {
        if (enabled_features.protectedMemory == VK_FALSE) {
            skip |= LogError("VUID-VkBufferCreateInfo-flags-01887", device, flag_loc,
                             "has VK_BUFFER_CREATE_PROTECTED_BIT set but the protectedMemory device feature is not enabled.");
        }
        const VkBufferCreateFlags invalid_flags =
            VK_BUFFER_CREATE_SPARSE_BINDING_BIT | VK_BUFFER_CREATE_SPARSE_RESIDENCY_BIT | VK_BUFFER_CREATE_SPARSE_ALIASED_BIT;
        if ((flags & invalid_flags) != 0) {
            skip |= LogError("VUID-VkBufferCreateInfo-None-01888", device, flag_loc,
                             "is %s but can't mix protected with sparse flags.", string_VkBufferCreateFlags(flags).c_str());
        }
    }

    return skip;
}

bool Device::ValidateCreateBufferBufferDeviceAddress(const VkBufferCreateInfo &create_info, const Location &create_info_loc) const {
    bool skip = false;

    if (auto chained_devaddr_struct = vku::FindStructInPNextChain<VkBufferDeviceAddressCreateInfoEXT>(create_info.pNext)) {
        if (!(create_info.flags & VK_BUFFER_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT) &&
            chained_devaddr_struct->deviceAddress != 0) {
            skip |= LogError("VUID-VkBufferCreateInfo-deviceAddress-02604", device,
                             create_info_loc.pNext(Struct::VkBufferDeviceAddressCreateInfoEXT, Field::deviceAddress),
                             "(%" PRIu64 ") is non-zero but requires VK_BUFFER_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT.",
                             chained_devaddr_struct->deviceAddress);
        }
    }

    if (auto chained_opaqueaddr_struct = vku::FindStructInPNextChain<VkBufferOpaqueCaptureAddressCreateInfo>(create_info.pNext)) {
        if (!(create_info.flags & VK_BUFFER_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT) &&
            chained_opaqueaddr_struct->opaqueCaptureAddress != 0) {
            skip |= LogError("VUID-VkBufferCreateInfo-opaqueCaptureAddress-03337", device,
                             create_info_loc.pNext(Struct::VkBufferOpaqueCaptureAddressCreateInfo, Field::opaqueCaptureAddress),
                             "(%" PRIu64 ") is non-zero but requires VK_BUFFER_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT.",
                             chained_opaqueaddr_struct->opaqueCaptureAddress);
        }
    }

    if ((create_info.flags & VK_BUFFER_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT) &&
        !enabled_features.bufferDeviceAddressCaptureReplay && !enabled_features.bufferDeviceAddressCaptureReplayEXT) {
        skip |= LogError("VUID-VkBufferCreateInfo-flags-03338", device, create_info_loc.dot(Field::flags),
                         "has VK_BUFFER_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT set but the bufferDeviceAddressCaptureReplay "
                         "device feature is not enabled.");
    }

    return skip;
}

}  // namespace stateless
