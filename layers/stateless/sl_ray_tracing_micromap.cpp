/* Copyright (c) 2015-2026 The Khronos Group Inc.
 * Copyright (c) 2015-2026 Valve Corporation
 * Copyright (c) 2015-2026 LunarG, Inc.
 * Copyright (C) 2015-2025 Google Inc.
 * Modifications Copyright (C) 2025-2026 Advanced Micro Devices, Inc. All rights reserved.
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

#include <vulkan/vulkan_core.h>
#include <vulkan/utility/vk_format_utils.h>
#include "containers/span.h"
#include "error_message/error_location.h"
#include "stateless/stateless_validation.h"

#include "utils/math_utils.h"

namespace stateless {

bool Device::ValidateMicromapBuildInfo(const Context& context, const VkMicromapBuildInfoEXT& build_info,
                                       const Location& build_info_loc) const {
    bool skip = false;

    if (build_info.pUsageCounts && build_info.ppUsageCounts) {
        skip |= LogError("VUID-VkMicromapBuildInfoEXT-pUsageCounts-07516", device, build_info_loc,
                         "both pUsageCounts and ppUsageCounts are not NULL.");
    }

    if (build_info.type == VK_MICROMAP_TYPE_OPACITY_MICROMAP_EXT && (build_info.pUsageCounts || build_info.ppUsageCounts)) {
        const Field usage_field = build_info.pUsageCounts ? Field::pUsageCounts : Field::ppUsageCounts;
        for (uint32_t usage_i = 0; usage_i < build_info.usageCountsCount; ++usage_i) {
            const VkMicromapUsageEXT& usage =
                build_info.pUsageCounts ? build_info.pUsageCounts[usage_i] : *build_info.ppUsageCounts[usage_i];
            skip |= context.ValidateRangedEnum(
                build_info_loc.dot(usage_field, usage_i).dot(Field::format), vvl::Enum::VkOpacityMicromapFormatKHR,
                static_cast<VkOpacityMicromapFormatKHR>(usage.format), "VUID-VkMicromapBuildInfoEXT-format-11652");

            if (usage.format == VK_OPACITY_MICROMAP_FORMAT_2_STATE_EXT &&
                usage.subdivisionLevel > phys_dev_ext_props.micromap_ext_props.maxOpacity2StateSubdivisionLevel) {
                skip |= LogError("VUID-VkMicromapBuildInfoEXT-subdivisionLevel-11654", device,
                                 build_info_loc.dot(usage_field, usage_i).dot(Field::subdivisionLevel),
                                 "(%" PRIu32 ") is greater than maxOpacity2StateSubdivisionLevel (%" PRIu32
                                 ") for VK_OPACITY_MICROMAP_FORMAT_2_STATE_EXT.",
                                 usage.subdivisionLevel, phys_dev_ext_props.micromap_ext_props.maxOpacity2StateSubdivisionLevel);
            } else if (usage.format == VK_OPACITY_MICROMAP_FORMAT_4_STATE_EXT &&
                       usage.subdivisionLevel > phys_dev_ext_props.micromap_ext_props.maxOpacity4StateSubdivisionLevel) {
                skip |= LogError("VUID-VkMicromapBuildInfoEXT-subdivisionLevel-11655", device,
                                 build_info_loc.dot(usage_field, usage_i).dot(Field::subdivisionLevel),
                                 "(%" PRIu32 ") is greater than maxOpacity4StateSubdivisionLevel (%" PRIu32
                                 ") for VK_OPACITY_MICROMAP_FORMAT_4_STATE_EXT.",
                                 usage.subdivisionLevel, phys_dev_ext_props.micromap_ext_props.maxOpacity4StateSubdivisionLevel);
            }
        }
    }

    return skip;
}

bool Device::manual_PreCallValidateCreateMicromapEXT(VkDevice device, const VkMicromapCreateInfoEXT* pCreateInfo,
                                                     const VkAllocationCallbacks* pAllocator, VkMicromapEXT* pMicromap,
                                                     const Context& context) const {
    bool skip = false;
    const auto& error_obj = context.error_obj;
    if (!enabled_features.micromapEXT) {
        skip |= LogError("VUID-vkCreateMicromapEXT-micromap-11615", device, error_obj.location,
                         "The VkPhysicalDeviceOpacityMicromapFeaturesEXT::micromap feature was not enabled.");
    }

    if ((pCreateInfo->createFlags & VK_MICROMAP_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT_EXT) &&
        !enabled_features.micromapCaptureReplayEXT) {
        skip |= LogError("VUID-VkMicromapCreateInfoEXT-micromapCaptureReplay-11616", device,
                         error_obj.location.dot(Field::pCreateInfo).dot(Field::createFlags),
                         "includes VK_MICROMAP_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT_EXT, but the micromapCaptureReplay "
                         "feature was not enabled.");
    }

    if (pCreateInfo->deviceAddress != 0 &&
        (pCreateInfo->createFlags & VK_MICROMAP_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT_EXT) == 0) {
        skip |= LogError("VUID-VkMicromapCreateInfoEXT-deviceAddress-07433", device,
                         error_obj.location.dot(Field::pCreateInfo).dot(Field::deviceAddress),
                         "is 0x%" PRIx64
                         ", but createFlags (%s) does not include VK_MICROMAP_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT_EXT.",
                         pCreateInfo->deviceAddress, string_VkMicromapCreateFlagsEXT(pCreateInfo->createFlags).c_str());
    }

    if (!IsIntegerMultipleOf(pCreateInfo->offset, 256)) {
        skip |= LogError("VUID-VkMicromapCreateInfoEXT-offset-07438", device,
                         error_obj.location.dot(Field::pCreateInfo).dot(Field::offset), "(%" PRIu64 ") must be a multiple of 256.",
                         pCreateInfo->offset);
    }

    return skip;
}

bool Device::manual_PreCallValidateDestroyMicromapEXT(VkDevice device, VkMicromapEXT micromap,
                                                      const VkAllocationCallbacks* pAllocator, const Context& context) const {
    bool skip = false;
    const auto& error_obj = context.error_obj;
    if (!enabled_features.micromapEXT) {
        skip |= LogError("VUID-vkDestroyMicromapEXT-micromap-11619", device, error_obj.location,
                         "The VkPhysicalDeviceOpacityMicromapFeaturesEXT::micromap feature was not enabled.");
    }

    return skip;
}

bool Device::manual_PreCallValidateCmdBuildMicromapsEXT(VkCommandBuffer commandBuffer, uint32_t infoCount,
                                                        const VkMicromapBuildInfoEXT* pInfos, const Context& context) const {
    bool skip = false;
    const auto& error_obj = context.error_obj;

    if (!enabled_features.micromapEXT) {
        skip |= LogError("VUID-vkCmdBuildMicromapsEXT-micromap-11648", commandBuffer, error_obj.location,
                         "The VkPhysicalDeviceOpacityMicromapFeaturesEXT::micromap feature was not enabled.");
    }

    for (const auto [info_i, info] : vvl::enumerate(pInfos, infoCount)) {
        const Location info_loc = error_obj.location.dot(Field::pInfos, info_i);

        if (!IsPointerAligned(info.scratchData.deviceAddress,
                              phys_dev_ext_props.acc_structure_props.minAccelerationStructureScratchOffsetAlignment)) {
            skip |= LogError("VUID-vkCmdBuildMicromapsEXT-pInfos-07514", commandBuffer,
                             info_loc.dot(Field::scratchData).dot(Field::deviceAddress),
                             "(0x%" PRIx64 ") must be aligned to minAccelerationStructureScratchOffsetAlignment (%" PRIu32 ").",
                             info.scratchData.deviceAddress,
                             phys_dev_ext_props.acc_structure_props.minAccelerationStructureScratchOffsetAlignment);
        }

        if (info.triangleArray.deviceAddress == 0) {
            // TODO - This needs to be in CoreChecks and use cc_buffer_address.cpp
            skip |= LogError("VUID-vkCmdBuildMicromapsEXT-pInfos-10897", commandBuffer,
                             info_loc.dot(Field::triangleArray).dot(Field::deviceAddress), "is zero.");
        } else if (!IsPointerAligned(info.triangleArray.deviceAddress, 256)) {
            skip |= LogError("VUID-vkCmdBuildMicromapsEXT-pInfos-07515", commandBuffer,
                             info_loc.dot(Field::triangleArray).dot(Field::deviceAddress),
                             "(0x%" PRIx64 ") must be aligned to 256.", info.triangleArray.deviceAddress);
        }

        if (info.data.deviceAddress == 0) {
            // TODO - This needs to be in CoreChecks and use cc_buffer_address.cpp
            skip |= LogError("VUID-vkCmdBuildMicromapsEXT-pInfos-10896", commandBuffer,
                             info_loc.dot(Field::data).dot(Field::deviceAddress), "is zero.");
        } else if (!IsPointerAligned(info.data.deviceAddress, 256)) {
            skip |= LogError("VUID-vkCmdBuildMicromapsEXT-pInfos-07515", commandBuffer,
                             info_loc.dot(Field::data).dot(Field::deviceAddress), "(0x%" PRIx64 ") must be aligned to 256.",
                             info.data.deviceAddress);
        }

        skip |= ValidateMicromapBuildInfo(context, info, info_loc);
    }

    return skip;
}

bool Device::manual_PreCallValidateBuildMicromapsEXT(VkDevice device, VkDeferredOperationKHR deferredOperation, uint32_t infoCount,
                                                     const VkMicromapBuildInfoEXT* pInfos, const Context& context) const {
    bool skip = false;
    const auto& error_obj = context.error_obj;

    if (!enabled_features.micromapHostCommandsEXT) {
        skip |= LogError("VUID-vkBuildMicromapsEXT-micromapHostCommands-07555", device, error_obj.location,
                         "micromapHostCommands feature was not enabled.");
    }

    for (const auto [info_i, info] : vvl::enumerate(pInfos, infoCount)) {
        const Location info_loc = error_obj.location.dot(Field::pInfos, info_i);
        if (!info.data.hostAddress) {
            skip |= LogError("VUID-vkBuildMicromapsEXT-pInfos-07553", device, info_loc.dot(Field::data).dot(Field::hostAddress),
                             "is NULL.");
        }
        if (!info.triangleArray.hostAddress) {
            skip |= LogError("VUID-vkBuildMicromapsEXT-pInfos-07554", device,
                             info_loc.dot(Field::triangleArray).dot(Field::hostAddress), "is NULL.");
        }

        skip |= ValidateMicromapBuildInfo(context, info, info_loc);
    }

    return skip;
}

bool Device::manual_PreCallValidateCopyMicromapEXT(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                   const VkCopyMicromapInfoEXT* pInfo, const Context& context) const {
    bool skip = false;
    const auto& error_obj = context.error_obj;

    if (!enabled_features.micromapHostCommandsEXT) {
        skip |= LogError("VUID-vkCopyMicromapEXT-micromapHostCommands-07560", device, error_obj.location,
                         "micromapHostCommands feature was not enabled.");
    }

    const Location info_loc = error_obj.location.dot(Field::pInfo);
    if (pInfo->mode != VK_COPY_MICROMAP_MODE_COMPACT_EXT && pInfo->mode != VK_COPY_MICROMAP_MODE_CLONE_EXT) {
        skip |= LogError("VUID-VkCopyMicromapInfoEXT-mode-07531", device, info_loc.dot(Field::mode), "is %s.",
                         string_VkCopyMicromapModeEXT(pInfo->mode));
    }

    return skip;
}

bool Device::manual_PreCallValidateCopyMicromapToMemoryEXT(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                           const VkCopyMicromapToMemoryInfoEXT* pInfo,
                                                           const Context& context) const {
    bool skip = false;
    const auto& error_obj = context.error_obj;

    if (!enabled_features.micromapHostCommandsEXT) {
        skip |= LogError("VUID-vkCopyMicromapToMemoryEXT-micromapHostCommands-07571", device, error_obj.location,
                         "micromapHostCommands feature was not enabled.");
    }

    const Location info_loc = error_obj.location.dot(Field::pInfo);
    if (pInfo->mode != VK_COPY_MICROMAP_MODE_SERIALIZE_EXT) {
        skip |= LogError("VUID-VkCopyMicromapToMemoryInfoEXT-mode-07542", device, info_loc.dot(Field::mode), "is %s.",
                         string_VkCopyMicromapModeEXT(pInfo->mode));
    }

    if (!pInfo->dst.hostAddress) {
        skip |= LogError("VUID-vkCopyMicromapToMemoryEXT-pInfo-07569", device, info_loc.dot(Field::dst).dot(Field::hostAddress),
                         "is NULL.");
    } else if (!IsPointerAligned(pInfo->dst.hostAddress, 16)) {
        skip |= LogError("VUID-vkCopyMicromapToMemoryEXT-pInfo-07570", device, info_loc.dot(Field::dst).dot(Field::hostAddress),
                         "(%p) must be aligned to 16 bytes.", pInfo->dst.hostAddress);
    }

    return skip;
}

bool Device::manual_PreCallValidateCopyMemoryToMicromapEXT(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                           const VkCopyMemoryToMicromapInfoEXT* pInfo,
                                                           const Context& context) const {
    bool skip = false;
    const auto& error_obj = context.error_obj;

    if (!enabled_features.micromapHostCommandsEXT) {
        skip |= LogError("VUID-vkCopyMemoryToMicromapEXT-micromapHostCommands-07566", device, error_obj.location,
                         "micromapHostCommands feature was not enabled.");
    }

    const Location info_loc = error_obj.location.dot(Field::pInfo);
    if (pInfo->mode != VK_COPY_MICROMAP_MODE_DESERIALIZE_EXT) {
        skip |= LogError("VUID-VkCopyMemoryToMicromapInfoEXT-mode-07548", device, info_loc.dot(Field::mode), "is %s.",
                         string_VkCopyMicromapModeEXT(pInfo->mode));
    }

    if (!pInfo->src.hostAddress) {
        skip |= LogError("VUID-vkCopyMemoryToMicromapEXT-pInfo-07563", device, info_loc.dot(Field::src).dot(Field::hostAddress),
                         "is NULL.");
    } else if (!IsPointerAligned(pInfo->src.hostAddress, 16)) {
        skip |= LogError("VUID-vkCopyMemoryToMicromapEXT-pInfo-07564", device, info_loc.dot(Field::src).dot(Field::hostAddress),
                         "(%p) must be aligned to 16 bytes.", pInfo->src.hostAddress);
    }

    return skip;
}

bool Device::manual_PreCallValidateWriteMicromapsPropertiesEXT(VkDevice device, uint32_t micromapCount,
                                                               const VkMicromapEXT* pMicromaps, VkQueryType queryType,
                                                               size_t dataSize, void* pData, size_t stride,
                                                               const Context& context) const {
    bool skip = false;
    const auto& error_obj = context.error_obj;

    if (!enabled_features.micromapHostCommandsEXT) {
        skip |= LogError("VUID-vkWriteMicromapsPropertiesEXT-micromapHostCommands-07578", device, error_obj.location,
                         "micromapHostCommands feature was not enabled.");
    }

    if (dataSize < micromapCount * stride) {
        skip |= LogError("VUID-vkWriteMicromapsPropertiesEXT-dataSize-07576", device, error_obj.location.dot(Field::dataSize),
                         "(%zu) is less than micromapCount (%" PRIu32 ") x stride (%zu).", dataSize, micromapCount, stride);
    }

    if (queryType != VK_QUERY_TYPE_MICROMAP_COMPACTED_SIZE_EXT && queryType != VK_QUERY_TYPE_MICROMAP_SERIALIZATION_SIZE_EXT) {
        skip |= LogError("VUID-vkWriteMicromapsPropertiesEXT-queryType-07503", device, error_obj.location.dot(Field::queryType),
                         "is %s.", string_VkQueryType(queryType));
    } else {
        if (!IsIntegerMultipleOf(stride, sizeof(VkDeviceSize))) {
            skip |= LogError("VUID-vkWriteMicromapsPropertiesEXT-queryType-10071", device, error_obj.location.dot(Field::queryType),
                             "is %s, but stride (%zu) is not a multiple of the size of VkDeviceSize (%zu).",
                             string_VkQueryType(queryType), stride, sizeof(VkDeviceSize));
        }
        if (dataSize < sizeof(VkDeviceSize)) {
            skip |= LogError("VUID-vkWriteMicromapsPropertiesEXT-queryType-10072", device, error_obj.location.dot(Field::queryType),
                             "is %s, but dataSize (%zu) is less than the size of VkDeviceSize (%zu).",
                             string_VkQueryType(queryType), dataSize, sizeof(VkDeviceSize));
        }
    }

    return skip;
}

bool Device::manual_PreCallValidateCmdCopyMicromapEXT(VkCommandBuffer commandBuffer, const VkCopyMicromapInfoEXT* pInfo,
                                                      const Context& context) const {
    bool skip = false;
    const auto& error_obj = context.error_obj;

    const Location info_loc = error_obj.location.dot(Field::pInfo);
    if (pInfo->mode != VK_COPY_MICROMAP_MODE_COMPACT_EXT && pInfo->mode != VK_COPY_MICROMAP_MODE_CLONE_EXT) {
        skip |= LogError("VUID-VkCopyMicromapInfoEXT-mode-07531", commandBuffer, info_loc.dot(Field::mode), "is %s.",
                         string_VkCopyMicromapModeEXT(pInfo->mode));
    }

    return skip;
}

bool Device::manual_PreCallValidateCmdCopyMicromapToMemoryEXT(VkCommandBuffer commandBuffer,
                                                              const VkCopyMicromapToMemoryInfoEXT* pInfo,
                                                              const Context& context) const {
    bool skip = false;
    const auto& error_obj = context.error_obj;

    const Location info_loc = error_obj.location.dot(Field::pInfo);
    if (pInfo->mode != VK_COPY_MICROMAP_MODE_SERIALIZE_EXT) {
        skip |= LogError("VUID-VkCopyMicromapToMemoryInfoEXT-mode-07542", commandBuffer, info_loc.dot(Field::mode), "is %s.",
                         string_VkCopyMicromapModeEXT(pInfo->mode));
    }

    if (pInfo->dst.deviceAddress == 0) {
        skip |= LogError("VUID-vkCmdCopyMicromapToMemoryEXT-pInfo-07536", commandBuffer,
                         info_loc.dot(Field::dst).dot(Field::deviceAddress), "is zero.");
    } else if (!IsPointerAligned(pInfo->dst.deviceAddress, 256)) {
        skip |= LogError("VUID-vkCmdCopyMicromapToMemoryEXT-pInfo-07537", commandBuffer,
                         info_loc.dot(Field::dst).dot(Field::deviceAddress), "(0x%" PRIx64 ") must be aligned to 256 bytes.",
                         pInfo->dst.deviceAddress);
    }

    return skip;
}

bool Device::manual_PreCallValidateCmdCopyMemoryToMicromapEXT(VkCommandBuffer commandBuffer,
                                                              const VkCopyMemoryToMicromapInfoEXT* pInfo,
                                                              const Context& context) const {
    bool skip = false;
    const auto& error_obj = context.error_obj;

    const Location info_loc = error_obj.location.dot(Field::pInfo);
    if (pInfo->mode != VK_COPY_MICROMAP_MODE_DESERIALIZE_EXT) {
        skip |= LogError("VUID-VkCopyMemoryToMicromapInfoEXT-mode-07548", commandBuffer, info_loc.dot(Field::mode), "is %s.",
                         string_VkCopyMicromapModeEXT(pInfo->mode));
    }

    if (pInfo->src.deviceAddress == 0) {
        skip |= LogError("VUID-vkCmdCopyMemoryToMicromapEXT-pInfo-07543", commandBuffer,
                         info_loc.dot(Field::src).dot(Field::deviceAddress), "is zero.");
    } else if (!IsPointerAligned(pInfo->src.deviceAddress, 256)) {
        skip |= LogError("VUID-vkCmdCopyMemoryToMicromapEXT-pInfo-07544", commandBuffer,
                         info_loc.dot(Field::src).dot(Field::deviceAddress), "(0x%" PRIx64 ") must be aligned to 256 bytes.",
                         pInfo->src.deviceAddress);
    }

    return skip;
}

bool Device::manual_PreCallValidateCmdWriteMicromapsPropertiesEXT(VkCommandBuffer commandBuffer, uint32_t micromapCount,
                                                                  const VkMicromapEXT* pMicromaps, VkQueryType queryType,
                                                                  VkQueryPool queryPool, uint32_t firstQuery,
                                                                  const Context& context) const {
    bool skip = false;
    const auto& error_obj = context.error_obj;

    if (queryType != VK_QUERY_TYPE_MICROMAP_COMPACTED_SIZE_EXT && queryType != VK_QUERY_TYPE_MICROMAP_SERIALIZATION_SIZE_EXT) {
        skip |= LogError("VUID-vkCmdWriteMicromapsPropertiesEXT-queryType-07503", commandBuffer,
                         error_obj.location.dot(Field::queryType), "is %s.", string_VkQueryType(queryType));
    }

    return skip;
}

bool Device::manual_PreCallValidateGetDeviceMicromapCompatibilityEXT(VkDevice device, const VkMicromapVersionInfoEXT* pVersionInfo,
                                                                     VkAccelerationStructureCompatibilityKHR* pCompatibility,
                                                                     const Context& context) const {
    bool skip = false;
    const auto& error_obj = context.error_obj;

    if (!enabled_features.micromapEXT) {
        skip |= LogError("VUID-vkGetDeviceMicromapCompatibilityEXT-micromap-11666", device, error_obj.location,
                         "The VkPhysicalDeviceOpacityMicromapFeaturesEXT::micromap feature was not enabled.");
    }

    return skip;
}

bool Device::manual_PreCallValidateGetMicromapBuildSizesEXT(VkDevice device, VkAccelerationStructureBuildTypeKHR buildType,
                                                            const VkMicromapBuildInfoEXT* pBuildInfo,
                                                            VkMicromapBuildSizesInfoEXT* pSizeInfo, const Context& context) const {
    bool skip = false;
    const auto& error_obj = context.error_obj;

    if (!enabled_features.micromapEXT) {
        skip |= LogError("VUID-vkGetMicromapBuildSizesEXT-micromap-11618", device, error_obj.location,
                         "The VkPhysicalDeviceOpacityMicromapFeaturesEXT::micromap feature was not enabled.");
    }

    skip |= ValidateMicromapBuildInfo(context, *pBuildInfo, error_obj.location.dot(Field::pBuildInfo));

    return skip;
}

}  // namespace stateless
