/* Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (C) 2015-2024 Google Inc.
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
#include <vulkan/utility/vk_format_utils.h>
#include "utils/image_utils.h"
#include "utils/math_utils.h"
#include "containers/range.h"

namespace stateless {

bool Device::manual_PreCallValidateAllocateMemory(VkDevice device, const VkMemoryAllocateInfo *pAllocateInfo,
                                                  const VkAllocationCallbacks *pAllocator, VkDeviceMemory *pMemory,
                                                  const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;

    if (!pAllocateInfo) {
        return skip;
    }
    const Location allocate_info_loc = error_obj.location.dot(Field::pAllocateInfo);
    auto chained_prio_struct = vku::FindStructInPNextChain<VkMemoryPriorityAllocateInfoEXT>(pAllocateInfo->pNext);
    if (chained_prio_struct && (chained_prio_struct->priority < 0.0f || chained_prio_struct->priority > 1.0f)) {
        skip |= LogError("VUID-VkMemoryPriorityAllocateInfoEXT-priority-02602", device,
                         allocate_info_loc.pNext(Struct::VkMemoryPriorityAllocateInfoEXT, Field::priority), "is %f",
                         chained_prio_struct->priority);
    }

    auto flags_info = vku::FindStructInPNextChain<VkMemoryAllocateFlagsInfo>(pAllocateInfo->pNext);
    const VkMemoryAllocateFlags flags = flags_info ? flags_info->flags : 0;

    skip |= ValidateAllocateMemoryExternal(device, *pAllocateInfo, flags, allocate_info_loc);

    if (flags) {
        const Location flags_loc = allocate_info_loc.pNext(Struct::VkMemoryAllocateFlagsInfo, Field::flags);
        if ((flags & VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT) && !enabled_features.bufferDeviceAddressCaptureReplay) {
            skip |= LogError("VUID-VkMemoryAllocateInfo-flags-03330", device, flags_loc,
                             "has VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT set, but "
                             "bufferDeviceAddressCaptureReplay feature is not enabled.");
        }
        if ((flags & VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT) && !enabled_features.bufferDeviceAddress) {
            skip |= LogError("VUID-VkMemoryAllocateInfo-flags-03331", device, flags_loc,
                             "has VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT set, but bufferDeviceAddress feature is not enabled.");
        }
    }
    return skip;
}

bool Device::ValidateDeviceImageMemoryRequirements(VkDevice device, const VkDeviceImageMemoryRequirements &memory_requirements,
                                                   const Location &loc) const {
    bool skip = false;

    const auto &create_info = *(memory_requirements.pCreateInfo);
    if (vku::FindStructInPNextChain<VkImageSwapchainCreateInfoKHR>(create_info.pNext)) {
        skip |= LogError("VUID-VkDeviceImageMemoryRequirements-pCreateInfo-06416", device,
                         loc.dot(Field::pCreateInfo).dot(Field::pNext), "chain contains VkImageSwapchainCreateInfoKHR.\n%s",
                         PrintPNextChain(Struct::VkImageCreateInfo, create_info.pNext).c_str());
    }
    if (vku::FindStructInPNextChain<VkImageDrmFormatModifierExplicitCreateInfoEXT>(create_info.pNext)) {
        skip |= LogError("VUID-VkDeviceImageMemoryRequirements-pCreateInfo-06776", device,
                         loc.dot(Field::pCreateInfo).dot(Field::pNext),
                         "chain contains VkImageDrmFormatModifierExplicitCreateInfoEXT.\n%s",
                         PrintPNextChain(Struct::VkImageCreateInfo, create_info.pNext).c_str());
    }

    if (vkuFormatIsMultiplane(create_info.format) && (create_info.flags & VK_IMAGE_CREATE_DISJOINT_BIT) != 0) {
        if (memory_requirements.planeAspect == VK_IMAGE_ASPECT_NONE) {
            skip |= LogError("VUID-VkDeviceImageMemoryRequirements-pCreateInfo-06417", device, loc.dot(Field::planeAspect),
                             "is VK_IMAGE_ASPECT_NONE with a multi-planar format and disjoint flag.");
        } else if ((create_info.tiling == VK_IMAGE_TILING_LINEAR || create_info.tiling == VK_IMAGE_TILING_OPTIMAL) &&
                   !IsOnlyOneValidPlaneAspect(create_info.format, memory_requirements.planeAspect)) {
            skip |= LogError("VUID-VkDeviceImageMemoryRequirements-pCreateInfo-06419", device, loc.dot(Field::planeAspect),
                             "is %s but is invalid for %s.", string_VkImageAspectFlags(memory_requirements.planeAspect).c_str(),
                             string_VkFormat(create_info.format));
        }
    }
    const uint64_t external_format = GetExternalFormat(memory_requirements.pCreateInfo->pNext);
    if (external_format != 0) {
        skip |= LogError("VUID-VkDeviceImageMemoryRequirements-pNext-06996", device, loc.dot(Field::pCreateInfo),
                         "pNext chain contains VkExternalFormatANDROID with externalFormat %" PRIu64 ".", external_format);
    }

    return skip;
}

bool Device::manual_PreCallValidateGetDeviceImageMemoryRequirements(VkDevice device, const VkDeviceImageMemoryRequirements *pInfo,
                                                                    VkMemoryRequirements2 *pMemoryRequirements,
                                                                    const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;

    skip |= ValidateDeviceImageMemoryRequirements(device, *pInfo, error_obj.location.dot(Field::pInfo));

    return skip;
}

bool Device::manual_PreCallValidateGetDeviceImageSparseMemoryRequirements(
    VkDevice device, const VkDeviceImageMemoryRequirements *pInfo, uint32_t *pSparseMemoryRequirementCount,
    VkSparseImageMemoryRequirements2 *pSparseMemoryRequirements, const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;

    skip |= ValidateDeviceImageMemoryRequirements(device, *pInfo, error_obj.location.dot(Field::pInfo));

    return skip;
}
bool Device::manual_PreCallValidateCmdDecompressMemoryEXT(VkCommandBuffer commandBuffer,
                                                          const VkDecompressMemoryInfoEXT* pDecompressMemoryInfoEXT,
                                                          const Context& context) const {
    bool skip = false;
    const auto& error_obj = context.error_obj;

    if (!enabled_features.memoryDecompression) {
        skip |= LogError("VUID-vkCmdDecompressMemoryEXT-memoryDecompression-11761", commandBuffer, error_obj.location,
                         "The memoryDecompression feature must be enabled.");
    }

    const auto& props = phys_dev_ext_props.memory_decompression_props;
    if ((pDecompressMemoryInfoEXT->decompressionMethod & props.decompressionMethods) == 0) {
        skip |= LogError("VUID-VkDecompressMemoryInfoEXT-decompressionMethod-11763", commandBuffer,
                         error_obj.location.dot(Field::decompressionMethod),
                         "(0x%" PRIx64 ") is not a supported decompression method bit (supported mask: 0x%" PRIx64 ").",
                         pDecompressMemoryInfoEXT->decompressionMethod, props.decompressionMethods);
    }

    if (pDecompressMemoryInfoEXT->decompressionMethod & VK_MEMORY_DECOMPRESSION_METHOD_GDEFLATE_1_0_BIT_EXT) {
        const Location memory_info_loc = error_obj.location.dot(Field::pDecompressMemoryInfoEXT);
        for (uint32_t i = 0; i < pDecompressMemoryInfoEXT->regionCount; ++i) {
            const Location region_loc = memory_info_loc.dot(Field::pRegions, i);
            const VkDecompressMemoryRegionEXT mem_region = pDecompressMemoryInfoEXT->pRegions[i];
            if (mem_region.decompressedSize > 65536) {
                skip |= LogError("VUID-VkDecompressMemoryInfoEXT-decompressionMethod-11762", commandBuffer,
                                 region_loc.dot(Field::decompressedSize),
                                 "(%" PRIu64 ") must be less than or equal to 65536 bytes.", mem_region.decompressedSize);
            }

            if (mem_region.compressedSize == 0) {
                skip |= LogError("VUID-VkDecompressMemoryRegionEXT-compressedSize-11795", commandBuffer,
                                 region_loc.dot(Field::compressedSize), "must not be zero.");
            }

            if (mem_region.decompressedSize == 0) {
                skip |= LogError("VUID-VkDecompressMemoryRegionEXT-decompressedSize-11796", commandBuffer,
                                 region_loc.dot(Field::decompressedSize), "must not be zero.");
            }

            if (mem_region.srcAddress & 0x3) {
                skip |=
                    LogError("VUID-VkDecompressMemoryRegionEXT-srcAddress-07685", commandBuffer, region_loc.dot(Field::srcAddress),
                             "(0x%" PRIx64 ") is not 4-byte aligned.", mem_region.srcAddress);
            }

            if (mem_region.dstAddress & 0x3) {
                skip |=
                    LogError("VUID-VkDecompressMemoryRegionEXT-dstAddress-07687", commandBuffer, region_loc.dot(Field::dstAddress),
                             "(0x%" PRIx64 ") is not 4-byte aligned.", mem_region.dstAddress);
            }

            const vvl::range<VkDeviceAddress> src_range{mem_region.srcAddress, mem_region.srcAddress + mem_region.compressedSize};
            const vvl::range<VkDeviceAddress> dst_range{mem_region.dstAddress, mem_region.dstAddress + mem_region.decompressedSize};
            if (src_range.intersects(dst_range)) {
                skip |= LogError("VUID-VkDecompressMemoryRegionEXT-srcAddress-07691", commandBuffer,
                                 region_loc.dot(Field::srcAddress), "range %s overlaps with dstAddress range %s.",
                                 string_range_hex(src_range).c_str(), string_range_hex(dst_range).c_str());
            }
        }
    }

    if (!IsPowerOfTwo(pDecompressMemoryInfoEXT->decompressionMethod)) {
        skip |= LogError("VUID-VkDecompressMemoryInfoEXT-decompressionMethod-07690", commandBuffer,
                         error_obj.location.dot(Field::pDecompressMemoryInfoEXT).dot(Field::decompressionMethod),
                         "(0x%" PRIx64 ") must have a single bit set.", pDecompressMemoryInfoEXT->decompressionMethod);
    }

    return skip;
}

bool Device::manual_PreCallValidateCmdDecompressMemoryIndirectCountEXT(
    VkCommandBuffer commandBuffer, VkMemoryDecompressionMethodFlagsEXT decompressionMethod, VkDeviceAddress indirectCommandsAddress,
    VkDeviceAddress indirectCommandsCountAddress, uint32_t maxDecompressionCount, uint32_t stride, const Context& context) const {
    bool skip = false;
    const auto& error_obj = context.error_obj;

    if (!enabled_features.memoryDecompression) {
        skip |= LogError("VUID-vkCmdDecompressMemoryIndirectCountEXT-None-07692", commandBuffer, error_obj.location,
                         "The memoryDecompression feature must be enabled.");
    }

    const auto& props = phys_dev_ext_props.memory_decompression_props;
    if ((decompressionMethod & props.decompressionMethods) == 0) {
        skip |= LogError("VUID-vkCmdDecompressMemoryIndirectCountEXT-decompressionMethod-11810", commandBuffer,
                         error_obj.location.dot(Field::decompressionMethod),
                         "(0x%" PRIx64 ") is not a supported decompression method bit (supported mask: 0x%" PRIx64 ").",
                         decompressionMethod, props.decompressionMethods);
    }

    if (!IsPowerOfTwo(decompressionMethod)) {
        skip |= LogError("VUID-vkCmdDecompressMemoryIndirectCountEXT-decompressionMethod-07690", commandBuffer,
                         error_obj.location.dot(Field::decompressionMethod), "(0x%" PRIx64 ") must have a single bit set.",
                         decompressionMethod);
    }

    if (!IsPointerAligned(indirectCommandsAddress, 4)) {
        skip |= LogError("VUID-vkCmdDecompressMemoryIndirectCountEXT-indirectCommandsAddress-07695", commandBuffer,
                         error_obj.location.dot(Field::indirectCommandsAddress), "(0x%" PRIx64 ") is not aligned to 4 bytes.",
                         indirectCommandsAddress);
    }

    if (!IsPointerAligned(indirectCommandsCountAddress, 4)) {
        skip |= LogError("VUID-vkCmdDecompressMemoryIndirectCountEXT-indirectCommandsCountAddress-07698", commandBuffer,
                         error_obj.location.dot(Field::indirectCommandsCountAddress), "(0x%" PRIx64 ") is not aligned to 4 bytes.",
                         indirectCommandsCountAddress);
    }

    if (!IsIntegerMultipleOf(stride, 4) || stride < sizeof(VkDecompressMemoryRegionEXT)) {
        skip |= LogError(
            "VUID-vkCmdDecompressMemoryIndirectCountEXT-stride-11767", commandBuffer, error_obj.location.dot(Field::stride),
            "(%" PRIu32
            ") must be a multiple of 4 and must be greater than or equal to size of VkDecompressMemoryRegionEXT (%" PRIu64 ").",
            stride, static_cast<uint64_t>(sizeof(VkDecompressMemoryRegionEXT)));
    }

    if (maxDecompressionCount > props.maxDecompressionIndirectCount) {
        skip |= LogError("VUID-vkCmdDecompressMemoryIndirectCountEXT-maxDecompressionCount-11768", commandBuffer,
                         error_obj.location.dot(Field::maxDecompressionCount),
                         "(%" PRIu32
                         ") must be less than or equal to "
                         "VkPhysicalDeviceMemoryDecompressionPropertiesEXT::maxDecompressionIndirectCount (%" PRIu64 ").",
                         maxDecompressionCount, props.maxDecompressionIndirectCount);
    }

    return skip;
}

bool Device::manual_PreCallValidateQueueBindSparse(VkQueue queue, uint32_t bindInfoCount, const VkBindSparseInfo *pBindInfo,
                                                   VkFence fence, const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;

    for (uint32_t bind_info_i = 0; bind_info_i < bindInfoCount; ++bind_info_i) {
        const VkBindSparseInfo &bind_info = pBindInfo[bind_info_i];
        for (uint32_t image_bind_i = 0; image_bind_i < bind_info.imageBindCount; ++image_bind_i) {
            const VkSparseImageMemoryBindInfo &image_bind = bind_info.pImageBinds[image_bind_i];
            for (uint32_t bind_i = 0; bind_i < image_bind.bindCount; ++bind_i) {
                const VkSparseImageMemoryBind &bind = image_bind.pBinds[bind_i];
                if (bind.extent.width == 0) {
                    const LogObjectList objlist(queue, image_bind.image);
                    skip |= LogError("VUID-VkSparseImageMemoryBind-extent-09388", objlist,
                                     error_obj.location.dot(Field::pBindInfo, bind_info_i)
                                         .dot(Field::pImageBinds, image_bind_i)
                                         .dot(Field::pBinds, bind_i)
                                         .dot(Field::extent)
                                         .dot(Field::width),
                                     "is zero.");
                }

                if (bind.extent.height == 0) {
                    const LogObjectList objlist(queue, image_bind.image);
                    skip |= LogError("VUID-VkSparseImageMemoryBind-extent-09389", objlist,
                                     error_obj.location.dot(Field::pBindInfo, bind_info_i)
                                         .dot(Field::pImageBinds, image_bind_i)
                                         .dot(Field::pBinds, bind_i)
                                         .dot(Field::extent)
                                         .dot(Field::height),
                                     "is zero.");
                }

                if (bind.extent.depth == 0) {
                    const LogObjectList objlist(queue, image_bind.image);
                    skip |= LogError("VUID-VkSparseImageMemoryBind-extent-09390", objlist,
                                     error_obj.location.dot(Field::pBindInfo, bind_info_i)
                                         .dot(Field::pImageBinds, image_bind_i)
                                         .dot(Field::pBinds, bind_i)
                                         .dot(Field::extent)
                                         .dot(Field::depth),
                                     "is zero.");
                }
            }
        }
    }

    return skip;
}

bool Device::manual_PreCallValidateSetDeviceMemoryPriorityEXT(VkDevice device, VkDeviceMemory memory, float priority,
                                                              const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;
    if (!IsBetweenInclusive(priority, 0.0F, 1.0F)) {
        skip |= LogError("VUID-vkSetDeviceMemoryPriorityEXT-priority-06258", device, error_obj.location.dot(Field::priority),
                         "is %f.", priority);
    }
    return skip;
}
}  // namespace stateless