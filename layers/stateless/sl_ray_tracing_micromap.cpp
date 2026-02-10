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

bool Device::manual_PreCallValidateCreateMicromapEXT(VkDevice device, const VkMicromapCreateInfoEXT* pCreateInfo,
                                                     const VkAllocationCallbacks* pAllocator, VkMicromapEXT* pMicromap,
                                                     const Context& context) const {
    bool skip = false;
    const auto& error_obj = context.error_obj;

    if (!enabled_features.micromap) {
        skip |=
            LogError("VUID-vkCreateMicromapEXT-micromap-07430", device, error_obj.location, "micromap feature was not enabled.");
    }

    if ((pCreateInfo->deviceAddress != 0ULL) && !enabled_features.micromapCaptureReplay) {
        skip |= LogError("VUID-vkCreateMicromapEXT-deviceAddress-07431", device, error_obj.location,
                         "micromapCaptureReplay feature was not enabled.");
    }

    return skip;
}

bool Device::manual_PreCallValidateDestroyMicromapEXT(VkDevice device, VkMicromapEXT micromap,
                                                      const VkAllocationCallbacks* pAllocator, const Context& context) const {
    bool skip = false;
    const auto& error_obj = context.error_obj;

    if (!enabled_features.micromap) {
        skip |=
            LogError("VUID-vkDestroyMicromapEXT-micromap-10382", device, error_obj.location, "micromap feature was not enabled.");
    }

    return skip;
}

bool Device::manual_PreCallValidateCmdBuildMicromapsEXT(VkCommandBuffer commandBuffer, uint32_t infoCount,
                                                        const VkMicromapBuildInfoEXT* pInfos, const Context& context) const {
    bool skip = false;
    const auto& error_obj = context.error_obj;

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
            skip |= LogError("VUID-vkCmdBuildMicromapsEXT-pInfos-10897", device,
                             info_loc.dot(Field::triangleArray).dot(Field::hostAddress), "is NULL.");
        } else if (!IsPointerAligned(info.triangleArray.deviceAddress, 256)) {
            skip |= LogError("VUID-vkCmdBuildMicromapsEXT-pInfos-07515", commandBuffer,
                             info_loc.dot(Field::triangleArray).dot(Field::deviceAddress),
                             "(0x%" PRIx64 ") must be aligned to 256.", info.triangleArray.deviceAddress);
        }

        if (info.data.deviceAddress == 0) {
            skip |= LogError("VUID-vkCmdBuildMicromapsEXT-pInfos-10896", device, info_loc.dot(Field::data).dot(Field::hostAddress),
                             "is NULL.");
        } else if (!IsPointerAligned(info.data.deviceAddress, 256)) {
            skip |= LogError("VUID-vkCmdBuildMicromapsEXT-pInfos-07515", commandBuffer,
                             info_loc.dot(Field::data).dot(Field::deviceAddress), "(0x%" PRIx64 ") must be aligned to 256.",
                             info.data.deviceAddress);
        }
        if (info.pUsageCounts && info.ppUsageCounts) {
            skip |= LogError("VUID-VkMicromapBuildInfoEXT-pUsageCounts-07516", commandBuffer, info_loc,
                             "both pUsageCounts and ppUsageCounts are not NULL.");
        }
    }

    return skip;
}

bool Device::manual_PreCallValidateBuildMicromapsEXT(VkDevice device, VkDeferredOperationKHR deferredOperation, uint32_t infoCount,
                                                     const VkMicromapBuildInfoEXT* pInfos, const Context& context) const {
    bool skip = false;
    const auto& error_obj = context.error_obj;

    if (!enabled_features.micromapHostCommands) {
        skip |= LogError("VUID-vkBuildMicromapsEXT-micromapHostCommands-07555", device, error_obj.location,
                         "micromapHostCommands feature was not enabled.");
    }

    for (const auto [info_i, info] : vvl::enumerate(pInfos, infoCount)) {
        const Location info_loc = error_obj.location.dot(Field::pInfos, info_i);
        if (!info.data.hostAddress) {
            skip |= LogError("VUID-vkBuildMicromapsEXT-pInfos-07553", device, info_loc.dot(Field::data).dot(Field::hostAddress),
                             "is zero.");
        }
        if (!info.triangleArray.hostAddress) {
            skip |= LogError("VUID-vkBuildMicromapsEXT-pInfos-07554", device,
                             info_loc.dot(Field::triangleArray).dot(Field::hostAddress), "is zero.");
        }
    }

    return skip;
}

bool Device::manual_PreCallValidateCopyMicromapEXT(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                   const VkCopyMicromapInfoEXT* pInfo, const Context& context) const {
    bool skip = false;
    const auto& error_obj = context.error_obj;

    if (!enabled_features.micromapHostCommands) {
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

    if (!enabled_features.micromapHostCommands) {
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
                         "is zero.");
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

    if (!enabled_features.micromapHostCommands) {
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
                         "is zero.");
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

    if (queryType != VK_QUERY_TYPE_MICROMAP_COMPACTED_SIZE_EXT && queryType != VK_QUERY_TYPE_MICROMAP_SERIALIZATION_SIZE_EXT) {
        skip |= LogError("VUID-vkWriteMicromapsPropertiesEXT-queryType-07503", device, error_obj.location, "is %s.",
                         string_VkQueryType(queryType));
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
        skip |= LogError("VUID-vkCmdCopyMicromapToMemoryEXT-pInfo-07536", device,
                         info_loc.dot(Field::dst).dot(Field::deviceAddress), "is NULL.");
    } else if (!IsPointerAligned(pInfo->dst.deviceAddress, 256)) {
        skip |=
            LogError("VUID-vkCmdCopyMicromapToMemoryEXT-pInfo-07537", device, info_loc.dot(Field::dst).dot(Field::deviceAddress),
                     "(0x%" PRIx64 ") must be aligned to 256 bytes.", pInfo->dst.deviceAddress);
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
        skip |= LogError("VUID-vkCmdCopyMemoryToMicromapEXT-pInfo-07543", device,
                         info_loc.dot(Field::src).dot(Field::deviceAddress), "is NULL.");
    } else if (!IsPointerAligned(pInfo->src.deviceAddress, 256)) {
        skip |=
            LogError("VUID-vkCmdCopyMemoryToMicromapEXT-pInfo-07544", device, info_loc.dot(Field::src).dot(Field::deviceAddress),
                     "(0x%" PRIx64 ") must be aligned to 256 bytes.", pInfo->src.deviceAddress);
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
        skip |= LogError("VUID-vkCmdWriteMicromapsPropertiesEXT-queryType-07503", commandBuffer, error_obj.location, "is %s.",
                         string_VkQueryType(queryType));
    }

    return skip;
}

bool Device::manual_PreCallValidateGetDeviceMicromapCompatibilityEXT(VkDevice device, const VkMicromapVersionInfoEXT* pVersionInfo,
                                                                     VkAccelerationStructureCompatibilityKHR* pCompatibility,
                                                                     const Context& context) const {
    bool skip = false;
    const auto& error_obj = context.error_obj;

    if (!enabled_features.micromap) {
        skip |= LogError("VUID-vkGetDeviceMicromapCompatibilityEXT-micromap-07551", device, error_obj.location,
                         "micromap feature was not enabled.");
    }

    return skip;
}

bool Device::manual_PreCallValidateGetMicromapBuildSizesEXT(VkDevice device, VkAccelerationStructureBuildTypeKHR buildType,
                                                            const VkMicromapBuildInfoEXT* pBuildInfo,
                                                            VkMicromapBuildSizesInfoEXT* pSizeInfo, const Context& context) const {
    bool skip = false;
    const auto& error_obj = context.error_obj;

    if (!enabled_features.micromap) {
        skip |= LogError("VUID-vkGetMicromapBuildSizesEXT-micromap-07439", device, error_obj.location,
                         "micromap feature was not enabled.");
    }

    if (pBuildInfo->pUsageCounts && pBuildInfo->ppUsageCounts) {
        skip |= LogError("VUID-VkMicromapBuildInfoEXT-pUsageCounts-07516", device, error_obj.location,
                         "both pUsageCounts and ppUsageCounts are not NULL.");
    }

    return skip;
}

}  // namespace stateless