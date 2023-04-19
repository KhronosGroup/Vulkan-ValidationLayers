/* Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
 * Copyright (C) 2015-2023 Google Inc.
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

namespace {
struct ImportOperationsInfo {
    const VkImportMemoryHostPointerInfoEXT *host_pointer_info_ext;
    uint32_t total_import_ops;
};

ImportOperationsInfo GetNumberOfImportInfo(const VkMemoryAllocateInfo *pAllocateInfo) {
    uint32_t count = 0;

#ifdef VK_USE_PLATFORM_WIN32_KHR
    // VkImportMemoryWin32HandleInfoKHR with a non-zero handleType value
    auto import_memory_win32_handle = LvlFindInChain<VkImportMemoryWin32HandleInfoKHR>(pAllocateInfo->pNext);
    count += (import_memory_win32_handle && import_memory_win32_handle->handleType);
#endif

    // VkImportMemoryFdInfoKHR with a non-zero handleType value
    auto fd_info_khr = LvlFindInChain<VkImportMemoryFdInfoKHR>(pAllocateInfo->pNext);
    count += (fd_info_khr && fd_info_khr->handleType);

    // VkImportMemoryHostPointerInfoEXT with a non-zero handleType value
    auto host_pointer_info_ext = LvlFindInChain<VkImportMemoryHostPointerInfoEXT>(pAllocateInfo->pNext);
    count += (host_pointer_info_ext && host_pointer_info_ext->handleType);

#ifdef VK_USE_PLATFORM_ANDROID_KHR
    // VkImportAndroidHardwareBufferInfoANDROID with a non-NULL buffer value
    auto import_memory_ahb = LvlFindInChain<VkImportAndroidHardwareBufferInfoANDROID>(pAllocateInfo->pNext);
    count += (import_memory_ahb && import_memory_ahb->buffer);
#endif

#ifdef VK_USE_PLATFORM_FUCHSIA
    // VkImportMemoryZirconHandleInfoFUCHSIA with a non-zero handleType value
    auto import_zircon_fuchsia = LvlFindInChain<VkImportMemoryZirconHandleInfoFUCHSIA>(pAllocateInfo->pNext);
    count += (import_zircon_fuchsia && import_zircon_fuchsia->handleType);

    // VkImportMemoryBufferCollectionFUCHSIA
    auto import_buffer_collection_fuchsia = LvlFindInChain<VkImportMemoryBufferCollectionFUCHSIA>(pAllocateInfo->pNext);
    count += static_cast<bool>(
        import_buffer_collection_fuchsia);  // NOTE: There's no handleType on VkImportMemoryBufferCollectionFUCHSIA, so we
                                            // can't check that, and from the "Valid Usage (Implicit)" collection has to
                                            // always be valid.
#endif

    ImportOperationsInfo info = {};
    info.total_import_ops = count;
    info.host_pointer_info_ext = host_pointer_info_ext;

    return info;
}
}  // namespace

bool StatelessValidation::manual_PreCallValidateAllocateMemory(VkDevice device, const VkMemoryAllocateInfo *pAllocateInfo,
                                                               const VkAllocationCallbacks *pAllocator,
                                                               VkDeviceMemory *pMemory) const {
    bool skip = false;

    if (pAllocateInfo) {
        auto chained_prio_struct = LvlFindInChain<VkMemoryPriorityAllocateInfoEXT>(pAllocateInfo->pNext);
        if (chained_prio_struct && (chained_prio_struct->priority < 0.0f || chained_prio_struct->priority > 1.0f)) {
            skip |= LogError(device, "VUID-VkMemoryPriorityAllocateInfoEXT-priority-02602",
                             "priority (=%f) must be between `0` and `1`, inclusive.", chained_prio_struct->priority);
        }

        VkMemoryAllocateFlags flags = 0;
        auto flags_info = LvlFindInChain<VkMemoryAllocateFlagsInfo>(pAllocateInfo->pNext);
        if (flags_info) {
            flags = flags_info->flags;
        }

        const ImportOperationsInfo import_info = GetNumberOfImportInfo(pAllocateInfo);

        auto opaque_alloc_info = LvlFindInChain<VkMemoryOpaqueCaptureAddressAllocateInfo>(pAllocateInfo->pNext);
        if (opaque_alloc_info && opaque_alloc_info->opaqueCaptureAddress != 0) {
            if (!(flags & VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT)) {
                skip |= LogError(device, "VUID-VkMemoryAllocateInfo-opaqueCaptureAddress-03329",
                                 "If opaqueCaptureAddress is non-zero, VkMemoryAllocateFlagsInfo::flags must include "
                                 "VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT.");
            }

            if (import_info.host_pointer_info_ext) {
                skip |= LogError(
                    device, "VUID-VkMemoryAllocateInfo-pNext-03332",
                    "If the pNext chain includes a VkImportMemoryHostPointerInfoEXT structure, opaqueCaptureAddress must be zero.");
            }

            if (import_info.total_import_ops > 0) {
                skip |= LogError(device, "VUID-VkMemoryAllocateInfo-opaqueCaptureAddress-03333",
                                 "If the parameters define an import operation, opaqueCaptureAddress must be zero.");
            }
        }

        if (import_info.total_import_ops > 1) {
            skip |=
                LogError(device, "VUID-VkMemoryAllocateInfo-None-06657",
                         "The parameters must not define more than 1 import operation. User defined %" PRIu32 " import operations",
                         import_info.total_import_ops);
        }

        auto export_memory = LvlFindInChain<VkExportMemoryAllocateInfo>(pAllocateInfo->pNext);
        if (export_memory) {
            auto export_memory_nv = LvlFindInChain<VkExportMemoryAllocateInfoNV>(pAllocateInfo->pNext);
            if (export_memory_nv) {
                skip |= LogError(device, "VUID-VkMemoryAllocateInfo-pNext-00640",
                                 "pNext chain of VkMemoryAllocateInfo includes both VkExportMemoryAllocateInfo and "
                                 "VkExportMemoryAllocateInfoNV");
            }
#ifdef VK_USE_PLATFORM_WIN32_KHR
            auto export_memory_win32_nv = LvlFindInChain<VkExportMemoryWin32HandleInfoNV>(pAllocateInfo->pNext);
            if (export_memory_win32_nv) {
                skip |= LogError(device, "VUID-VkMemoryAllocateInfo-pNext-00640",
                                 "pNext chain of VkMemoryAllocateInfo includes both VkExportMemoryAllocateInfo and "
                                 "VkExportMemoryWin32HandleInfoNV");
            }
#endif
        }

#ifdef VK_USE_PLATFORM_WIN32_KHR
        if (LvlFindInChain<VkImportMemoryWin32HandleInfoKHR>(pAllocateInfo->pNext) &&
            LvlFindInChain<VkImportMemoryWin32HandleInfoNV>(pAllocateInfo->pNext)) {
            skip |= LogError(device, "VUID-VkMemoryAllocateInfo-pNext-00641",
                             "pNext chain of VkMemoryAllocateInfo includes both VkImportMemoryWin32HandleInfoKHR and "
                             "VkImportMemoryWin32HandleInfoNV");
        }
#endif

        if (flags) {
            VkBool32 capture_replay = false;
            VkBool32 buffer_device_address = false;
            const auto *vulkan_12_features = LvlFindInChain<VkPhysicalDeviceVulkan12Features>(device_createinfo_pnext);
            if (vulkan_12_features) {
                capture_replay = vulkan_12_features->bufferDeviceAddressCaptureReplay;
                buffer_device_address = vulkan_12_features->bufferDeviceAddress;
            } else {
                const auto *bda_features = LvlFindInChain<VkPhysicalDeviceBufferDeviceAddressFeatures>(device_createinfo_pnext);
                if (bda_features) {
                    capture_replay = bda_features->bufferDeviceAddressCaptureReplay;
                    buffer_device_address = bda_features->bufferDeviceAddress;
                }
            }
            if ((flags & VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT) && !capture_replay) {
                skip |= LogError(device, "VUID-VkMemoryAllocateInfo-flags-03330",
                                 "If VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT is set, "
                                 "bufferDeviceAddressCaptureReplay must be enabled.");
            }
            if ((flags & VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT) && !buffer_device_address) {
                skip |= LogError(device, "VUID-VkMemoryAllocateInfo-flags-03331",
                                 "If VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT is set, bufferDeviceAddress must be enabled.");
            }
        }
#ifdef VK_USE_PLATFORM_METAL_EXT
        skip |= ExportMetalObjectsPNextUtil(
            VK_EXPORT_METAL_OBJECT_TYPE_METAL_BUFFER_BIT_EXT, "VUID-VkMemoryAllocateInfo-pNext-06780",
            "vkAllocateMemory():", "VK_EXPORT_METAL_OBJECT_TYPE_METAL_BUFFER_BIT_EXT", pAllocateInfo->pNext);
#endif  // VK_USE_PLATFORM_METAL_EXT
    }
    return skip;
}

bool StatelessValidation::ValidateDeviceImageMemoryRequirements(VkDevice device, const VkDeviceImageMemoryRequirementsKHR *pInfo,
                                                                const char *func_name) const {
    bool skip = false;

    if (pInfo && pInfo->pCreateInfo) {
        const auto &create_info = *(pInfo->pCreateInfo);
        if (LvlFindInChain<VkImageSwapchainCreateInfoKHR>(create_info.pNext)) {
            skip |= LogError(device, "VUID-VkDeviceImageMemoryRequirementsKHR-pCreateInfo-06416",
                             "%s(): pCreateInfo->pNext chain contains VkImageSwapchainCreateInfoKHR.", func_name);
        }
        if (LvlFindInChain<VkImageDrmFormatModifierExplicitCreateInfoEXT>(create_info.pNext)) {
            skip |= LogError(device, "VUID-VkDeviceImageMemoryRequirements-pCreateInfo-06776",
                             "%s(): pCreateInfo->pNext chain contains VkImageDrmFormatModifierExplicitCreateInfoEXT.", func_name);
        }

        if (FormatIsMultiplane(create_info.format) && (create_info.flags & VK_IMAGE_CREATE_DISJOINT_BIT) != 0) {
            if (pInfo->planeAspect == VK_IMAGE_ASPECT_NONE_KHR) {
                skip |= LogError(device, "VUID-VkDeviceImageMemoryRequirementsKHR-pCreateInfo-06417",
                                 "%s(): Must not specify VK_IMAGE_ASPECT_NONE_KHR with a multi-planar format and disjoint flag.",
                                 func_name);
            } else if ((create_info.tiling == VK_IMAGE_TILING_LINEAR || create_info.tiling == VK_IMAGE_TILING_OPTIMAL) &&
                       !IsOnlyOneValidPlaneAspect(create_info.format, pInfo->planeAspect)) {
                skip |= LogError(device, "VUID-VkDeviceImageMemoryRequirementsKHR-pCreateInfo-06419",
                                 "%s(): planeAspect is %s but is invalid for %s.", func_name,
                                 string_VkImageAspectFlags(pInfo->planeAspect).c_str(), string_VkFormat(create_info.format));
            }
        }
#ifdef VK_USE_PLATFORM_ANDROID_KHR
        const auto *external_format = LvlFindInChain<VkExternalFormatANDROID>(pInfo->pCreateInfo);
        if (external_format && external_format->externalFormat) {
            skip |=
                LogError(device, "VUID-VkDeviceImageMemoryRequirements-pNext-06996",
                         "%s(): pInfo->pCreateInfo->pNext chain contains VkExternalFormatANDROID with externalFormat %" PRIu64 ".",
                         func_name, external_format->externalFormat);
        }
#endif  // VK_USE_PLATFORM_ANDROID_KHR
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateGetDeviceImageMemoryRequirementsKHR(
    VkDevice device, const VkDeviceImageMemoryRequirements *pInfo, VkMemoryRequirements2 *pMemoryRequirements) const {
    bool skip = false;

    skip |= ValidateDeviceImageMemoryRequirements(device, pInfo, "vkGetDeviceImageMemoryRequirementsKHR");

    return skip;
}

bool StatelessValidation::manual_PreCallValidateGetDeviceImageSparseMemoryRequirementsKHR(
    VkDevice device, const VkDeviceImageMemoryRequirements *pInfo, uint32_t *pSparseMemoryRequirementCount,
    VkSparseImageMemoryRequirements2 *pSparseMemoryRequirements) const {
    bool skip = false;

    skip |= ValidateDeviceImageMemoryRequirements(device, pInfo, "vkGetDeviceImageSparseMemoryRequirementsKHR");

    return skip;
}
