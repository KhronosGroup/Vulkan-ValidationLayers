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
    auto import_memory_win32_handle = vku::FindStructInPNextChain<VkImportMemoryWin32HandleInfoKHR>(pAllocateInfo->pNext);
    count += (import_memory_win32_handle && import_memory_win32_handle->handleType);
#endif

    // VkImportMemoryFdInfoKHR with a non-zero handleType value
    auto fd_info_khr = vku::FindStructInPNextChain<VkImportMemoryFdInfoKHR>(pAllocateInfo->pNext);
    count += (fd_info_khr && fd_info_khr->handleType);

    // VkImportMemoryHostPointerInfoEXT with a non-zero handleType value
    auto host_pointer_info_ext = vku::FindStructInPNextChain<VkImportMemoryHostPointerInfoEXT>(pAllocateInfo->pNext);
    count += (host_pointer_info_ext && host_pointer_info_ext->handleType);

#ifdef VK_USE_PLATFORM_ANDROID_KHR
    // VkImportAndroidHardwareBufferInfoANDROID with a non-NULL buffer value
    auto import_memory_ahb = vku::FindStructInPNextChain<VkImportAndroidHardwareBufferInfoANDROID>(pAllocateInfo->pNext);
    count += (import_memory_ahb && import_memory_ahb->buffer);
#endif

#ifdef VK_USE_PLATFORM_FUCHSIA
    // VkImportMemoryZirconHandleInfoFUCHSIA with a non-zero handleType value
    auto import_zircon_fuchsia = vku::FindStructInPNextChain<VkImportMemoryZirconHandleInfoFUCHSIA>(pAllocateInfo->pNext);
    count += (import_zircon_fuchsia && import_zircon_fuchsia->handleType);

    // VkImportMemoryBufferCollectionFUCHSIA
    auto import_buffer_collection_fuchsia = vku::FindStructInPNextChain<VkImportMemoryBufferCollectionFUCHSIA>(pAllocateInfo->pNext);
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
                                                               const VkAllocationCallbacks *pAllocator, VkDeviceMemory *pMemory,
                                                               const ErrorObject &error_obj) const {
    bool skip = false;

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

    VkMemoryAllocateFlags flags = 0;
    auto flags_info = vku::FindStructInPNextChain<VkMemoryAllocateFlagsInfo>(pAllocateInfo->pNext);
    if (flags_info) {
        flags = flags_info->flags;
    }

    const ImportOperationsInfo import_info = GetNumberOfImportInfo(pAllocateInfo);

    auto opaque_alloc_info = vku::FindStructInPNextChain<VkMemoryOpaqueCaptureAddressAllocateInfo>(pAllocateInfo->pNext);
    if (opaque_alloc_info && opaque_alloc_info->opaqueCaptureAddress != 0) {
        const Location address_loc =
            allocate_info_loc.pNext(Struct::VkMemoryOpaqueCaptureAddressAllocateInfo, Field::opaqueCaptureAddress);
        if (!(flags & VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT)) {
            skip |= LogError("VUID-VkMemoryAllocateInfo-opaqueCaptureAddress-03329", device, address_loc,
                             "is non-zero (%" PRIu64
                             ") so VkMemoryAllocateFlagsInfo::flags must include "
                             "VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT.",
                             opaque_alloc_info->opaqueCaptureAddress);
        }

        if (import_info.host_pointer_info_ext) {
            skip |= LogError("VUID-VkMemoryAllocateInfo-pNext-03332", device, address_loc,
                             "is non-zero (%" PRIu64 ") but the pNext chain includes a VkImportMemoryHostPointerInfoEXT structure.",
                             opaque_alloc_info->opaqueCaptureAddress);
        }

        if (import_info.total_import_ops > 0) {
            skip |=
                LogError("VUID-VkMemoryAllocateInfo-opaqueCaptureAddress-03333", device, address_loc,
                         "is non-zero (%" PRIu64 ") but an import operation is defined.", opaque_alloc_info->opaqueCaptureAddress);
        }
    }

    if (import_info.total_import_ops > 1) {
        skip |= LogError("VUID-VkMemoryAllocateInfo-None-06657", device, allocate_info_loc,
                         "%" PRIu32 " import operations are defined", import_info.total_import_ops);
    }

    auto export_memory = vku::FindStructInPNextChain<VkExportMemoryAllocateInfo>(pAllocateInfo->pNext);
    if (export_memory) {
        auto export_memory_nv = vku::FindStructInPNextChain<VkExportMemoryAllocateInfoNV>(pAllocateInfo->pNext);
        if (export_memory_nv) {
            skip |= LogError("VUID-VkMemoryAllocateInfo-pNext-00640", device, allocate_info_loc,
                             "pNext chain includes both VkExportMemoryAllocateInfo and "
                             "VkExportMemoryAllocateInfoNV");
        }
#ifdef VK_USE_PLATFORM_WIN32_KHR
        auto export_memory_win32_nv = vku::FindStructInPNextChain<VkExportMemoryWin32HandleInfoNV>(pAllocateInfo->pNext);
        if (export_memory_win32_nv) {
            skip |= LogError("VUID-VkMemoryAllocateInfo-pNext-00640", device, allocate_info_loc,
                             "pNext chain includes both VkExportMemoryAllocateInfo and "
                             "VkExportMemoryWin32HandleInfoNV");
        }
#endif
    }

#ifdef VK_USE_PLATFORM_WIN32_KHR
    if (vku::FindStructInPNextChain<VkImportMemoryWin32HandleInfoKHR>(pAllocateInfo->pNext) &&
        vku::FindStructInPNextChain<VkImportMemoryWin32HandleInfoNV>(pAllocateInfo->pNext)) {
        skip |= LogError("VUID-VkMemoryAllocateInfo-pNext-00641", device, allocate_info_loc,
                         "pNext chain includes both VkImportMemoryWin32HandleInfoKHR and "
                         "VkImportMemoryWin32HandleInfoNV");
    }
#endif

    if (flags) {
        const Location flags_loc = allocate_info_loc.pNext(Struct::VkMemoryAllocateFlagsInfo, Field::flags);
        VkBool32 capture_replay = false;
        VkBool32 buffer_device_address = false;
        const auto *vulkan_12_features = vku::FindStructInPNextChain<VkPhysicalDeviceVulkan12Features>(device_createinfo_pnext);
        if (vulkan_12_features) {
            capture_replay = vulkan_12_features->bufferDeviceAddressCaptureReplay;
            buffer_device_address = vulkan_12_features->bufferDeviceAddress;
        } else {
            const auto *bda_features = vku::FindStructInPNextChain<VkPhysicalDeviceBufferDeviceAddressFeatures>(device_createinfo_pnext);
            if (bda_features) {
                capture_replay = bda_features->bufferDeviceAddressCaptureReplay;
                buffer_device_address = bda_features->bufferDeviceAddress;
            }
        }
        if ((flags & VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT) && !capture_replay) {
            skip |= LogError("VUID-VkMemoryAllocateInfo-flags-03330", device, flags_loc,
                             "has VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT set, but"
                             "bufferDeviceAddressCaptureReplay feature is not enabled.");
        }
        if ((flags & VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT) && !buffer_device_address) {
            skip |= LogError("VUID-VkMemoryAllocateInfo-flags-03331", device, flags_loc,
                             "has VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT set, but bufferDeviceAddress feature is not enabled.");
        }
    }
#ifdef VK_USE_PLATFORM_METAL_EXT
    skip |=
        ExportMetalObjectsPNextUtil(VK_EXPORT_METAL_OBJECT_TYPE_METAL_BUFFER_BIT_EXT, "VUID-VkMemoryAllocateInfo-pNext-06780",
                                    error_obj.location, "VK_EXPORT_METAL_OBJECT_TYPE_METAL_BUFFER_BIT_EXT", pAllocateInfo->pNext);
#endif  // VK_USE_PLATFORM_METAL_EXT
    return skip;
}

bool StatelessValidation::ValidateDeviceImageMemoryRequirements(VkDevice device, const VkDeviceImageMemoryRequirementsKHR *pInfo,
                                                                const Location &loc) const {
    bool skip = false;

    if (!pInfo || !pInfo->pCreateInfo) {
        return skip;
    }
    const auto &create_info = *(pInfo->pCreateInfo);
    if (vku::FindStructInPNextChain<VkImageSwapchainCreateInfoKHR>(create_info.pNext)) {
        skip |= LogError("VUID-VkDeviceImageMemoryRequirements-pCreateInfo-06416", device, loc,
                         "pNext chain contains VkImageSwapchainCreateInfoKHR.");
    }
    if (vku::FindStructInPNextChain<VkImageDrmFormatModifierExplicitCreateInfoEXT>(create_info.pNext)) {
        skip |= LogError("VUID-VkDeviceImageMemoryRequirements-pCreateInfo-06776", device, loc,
                         "pNext chain contains VkImageDrmFormatModifierExplicitCreateInfoEXT.");
    }

    if (vkuFormatIsMultiplane(create_info.format) && (create_info.flags & VK_IMAGE_CREATE_DISJOINT_BIT) != 0) {
        if (pInfo->planeAspect == VK_IMAGE_ASPECT_NONE_KHR) {
            skip |= LogError("VUID-VkDeviceImageMemoryRequirements-pCreateInfo-06417", device, loc.dot(Field::planeAspect),
                             "is VK_IMAGE_ASPECT_NONE_KHR with a multi-planar format and disjoint flag.");
        } else if ((create_info.tiling == VK_IMAGE_TILING_LINEAR || create_info.tiling == VK_IMAGE_TILING_OPTIMAL) &&
                   !IsOnlyOneValidPlaneAspect(create_info.format, pInfo->planeAspect)) {
            skip |= LogError("VUID-VkDeviceImageMemoryRequirements-pCreateInfo-06419", device, loc.dot(Field::planeAspect),
                             "is %s but is invalid for %s.", string_VkImageAspectFlags(pInfo->planeAspect).c_str(),
                             string_VkFormat(create_info.format));
        }
    }
    const uint64_t external_format = GetExternalFormat(pInfo->pCreateInfo->pNext);
    if (external_format != 0) {
        skip |= LogError("VUID-VkDeviceImageMemoryRequirements-pNext-06996", device, loc.dot(Field::pCreateInfo),
                         "pNext chain contains VkExternalFormatANDROID with externalFormat %" PRIu64 ".", external_format);
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateGetDeviceImageMemoryRequirements(VkDevice device,
                                                                                 const VkDeviceImageMemoryRequirements *pInfo,
                                                                                 VkMemoryRequirements2 *pMemoryRequirements,
                                                                                 const ErrorObject &error_obj) const {
    bool skip = false;

    skip |= ValidateDeviceImageMemoryRequirements(device, pInfo, error_obj.location.dot(Field::pInfo));

    return skip;
}

bool StatelessValidation::manual_PreCallValidateGetDeviceImageMemoryRequirementsKHR(VkDevice device,
                                                                                    const VkDeviceImageMemoryRequirements *pInfo,
                                                                                    VkMemoryRequirements2 *pMemoryRequirements,
                                                                                    const ErrorObject &error_obj) const {
    return manual_PreCallValidateGetDeviceImageMemoryRequirements(device, pInfo, pMemoryRequirements, error_obj);
}

bool StatelessValidation::manual_PreCallValidateGetDeviceImageSparseMemoryRequirements(
    VkDevice device, const VkDeviceImageMemoryRequirements *pInfo, uint32_t *pSparseMemoryRequirementCount,
    VkSparseImageMemoryRequirements2 *pSparseMemoryRequirements, const ErrorObject &error_obj) const {
    bool skip = false;

    skip |= ValidateDeviceImageMemoryRequirements(device, pInfo, error_obj.location.dot(Field::pInfo));

    return skip;
}

bool StatelessValidation::manual_PreCallValidateGetDeviceImageSparseMemoryRequirementsKHR(
    VkDevice device, const VkDeviceImageMemoryRequirements *pInfo, uint32_t *pSparseMemoryRequirementCount,
    VkSparseImageMemoryRequirements2 *pSparseMemoryRequirements, const ErrorObject &error_obj) const {
    return manual_PreCallValidateGetDeviceImageSparseMemoryRequirements(device, pInfo, pSparseMemoryRequirementCount,
                                                                        pSparseMemoryRequirements, error_obj);
}

bool StatelessValidation::manual_PreCallValidateQueueBindSparse(VkQueue queue, uint32_t bindInfoCount,
                                                                const VkBindSparseInfo *pBindInfo, VkFence fence,
                                                                const ErrorObject &error_obj) const {
    bool skip = false;

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
