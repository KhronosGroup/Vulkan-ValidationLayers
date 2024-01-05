/* Copyright (c) 2015-2024 The Khronos Group Inc.
 * Copyright (c) 2015-2024 Valve Corporation
 * Copyright (c) 2015-2024 LunarG, Inc.
 * Copyright (C) 2015-2024 Google Inc.
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

#include <vulkan/vk_enum_string_helper.h>
#include "generated/chassis.h"
#include "core_validation.h"

static bool CanSemaphoreExportFromImported(VkPhysicalDevice physical_device, VkExternalSemaphoreHandleTypeFlagBits export_type,
                                           VkExternalSemaphoreHandleTypeFlagBits imported_type) {
    VkPhysicalDeviceExternalSemaphoreInfo semaphore_info = vku::InitStructHelper();
    semaphore_info.handleType = export_type;
    VkExternalSemaphoreProperties semaphore_properties = vku::InitStructHelper();
    DispatchGetPhysicalDeviceExternalSemaphoreProperties(physical_device, &semaphore_info, &semaphore_properties);
    return (imported_type & semaphore_properties.exportFromImportedHandleTypes) != 0;
}

static bool CanFenceExportFromImported(VkPhysicalDevice physical_device, VkExternalFenceHandleTypeFlagBits export_type,
                                       VkExternalFenceHandleTypeFlagBits imported_type) {
    VkPhysicalDeviceExternalFenceInfo fence_info = vku::InitStructHelper();
    fence_info.handleType = export_type;
    VkExternalFenceProperties fence_properties = vku::InitStructHelper();
    DispatchGetPhysicalDeviceExternalFenceProperties(physical_device, &fence_info, &fence_properties);
    return (imported_type & fence_properties.exportFromImportedHandleTypes) != 0;
}

bool CoreChecks::PreCallValidateGetMemoryFdKHR(VkDevice device, const VkMemoryGetFdInfoKHR *pGetFdInfo, int *pFd,
                                               const ErrorObject &error_obj) const {
    bool skip = false;
    if (const auto memory_state = Get<vvl::DeviceMemory>(pGetFdInfo->memory)) {
        const auto export_info = vku::FindStructInPNextChain<VkExportMemoryAllocateInfo>(memory_state->alloc_info.pNext);
        if (!export_info) {
            skip |= LogError("VUID-VkMemoryGetFdInfoKHR-handleType-00671", pGetFdInfo->memory,
                             error_obj.location.dot(Field::pGetFdInfo).dot(Field::memory),
                             "pNext chain does not include a VkExportMemoryAllocateInfo structure.");
        } else if ((export_info->handleTypes & pGetFdInfo->handleType) == 0) {
            skip |= LogError("VUID-VkMemoryGetFdInfoKHR-handleType-00671", pGetFdInfo->memory,
                             error_obj.location.dot(Field::pGetFdInfo).dot(Field::memory),
                             "the requested handle type (%s) is not included in the memory's "
                             "VkExportMemoryAllocateInfo::handleTypes (%s).",
                             string_VkExternalMemoryHandleTypeFlagBits(pGetFdInfo->handleType),
                             string_VkExternalMemoryHandleTypeFlags(export_info->handleTypes).c_str());
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateImportSemaphoreFdKHR(VkDevice device, const VkImportSemaphoreFdInfoKHR *pImportSemaphoreFdInfo,
                                                     const ErrorObject &error_obj) const {
    bool skip = false;
    auto sem_state = Get<vvl::Semaphore>(pImportSemaphoreFdInfo->semaphore);
    if (sem_state) {
        const Location info_loc = error_obj.location.dot(Field::pImportSemaphoreFdInfo);
        skip |=
            ValidateObjectNotInUse(sem_state.get(), info_loc.dot(Field::semaphore), "VUID-vkImportSemaphoreFdKHR-semaphore-01142");

        if ((pImportSemaphoreFdInfo->flags & VK_SEMAPHORE_IMPORT_TEMPORARY_BIT) != 0) {
            if (sem_state->type == VK_SEMAPHORE_TYPE_TIMELINE) {
                skip |= LogError("VUID-VkImportSemaphoreFdInfoKHR-flags-03323", sem_state->Handle(), info_loc.dot(Field::flags),
                                 "includes VK_SEMAPHORE_IMPORT_TEMPORARY_BIT and semaphore is VK_SEMAPHORE_TYPE_TIMELINE.");
            }
        } else {
            // only valid type with copy payload transference semantics currently
            if (pImportSemaphoreFdInfo->handleType == VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_SYNC_FD_BIT) {
                skip |=
                    LogError("VUID-VkImportSemaphoreFdInfoKHR-handleType-07307", sem_state->Handle(), info_loc.dot(Field::flags),
                             "is %s and handleType is VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_SYNC_FD_BIT.",
                             string_VkSemaphoreImportFlags(pImportSemaphoreFdInfo->flags).c_str());
            }
        }

        if (pImportSemaphoreFdInfo->handleType == VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_FD_BIT) {
            if (const auto payload_info = GetOpaqueInfoFromFdHandle(pImportSemaphoreFdInfo->fd)) {
                if (sem_state->flags != payload_info->semaphore_flags) {
                    // would use string_VkSemaphoreCreateFlags but no valid flags yet
                    skip |= LogError("VUID-VkImportSemaphoreFdInfoKHR-handleType-03263", device, info_loc.dot(Field::semaphore),
                                     "was created with flags 0x%" PRIx32 " but fd (%d) was exported with 0x%" PRIx32 ".",
                                     sem_state->flags, pImportSemaphoreFdInfo->fd, payload_info->semaphore_flags);
                }
                if (sem_state->type != payload_info->semaphore_type) {
                    skip |= LogError("VUID-VkImportSemaphoreFdInfoKHR-handleType-03264", device, info_loc.dot(Field::semaphore),
                                     "was created with %s but fd (%d) was exported as %s.", string_VkSemaphoreType(sem_state->type),
                                     pImportSemaphoreFdInfo->fd, string_VkSemaphoreType(payload_info->semaphore_type));
                }
            }
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateGetSemaphoreFdKHR(VkDevice device, const VkSemaphoreGetFdInfoKHR *pGetFdInfo, int *pFd,
                                                  const ErrorObject &error_obj) const {
    bool skip = false;
    auto sem_state = Get<vvl::Semaphore>(pGetFdInfo->semaphore);
    if (sem_state) {
        const Location info_loc = error_obj.location.dot(Field::pGetFdInfo);
        if ((pGetFdInfo->handleType & sem_state->exportHandleTypes) == 0) {
            skip |= LogError("VUID-VkSemaphoreGetFdInfoKHR-handleType-01132", sem_state->Handle(), info_loc.dot(Field::handleType),
                             "(%s) is different from VkExportSemaphoreCreateInfo::handleTypes (%s).",
                             string_VkExternalSemaphoreHandleTypeFlagBits(pGetFdInfo->handleType),
                             string_VkExternalSemaphoreHandleTypeFlags(sem_state->exportHandleTypes).c_str());
        }

        if (sem_state->Scope() != vvl::Semaphore::kInternal &&
            !CanSemaphoreExportFromImported(physical_device, pGetFdInfo->handleType, sem_state->ImportedHandleType())) {
            skip |= LogError("VUID-VkSemaphoreGetFdInfoKHR-semaphore-01133", sem_state->Handle(), info_loc.dot(Field::handleType),
                             "(%s) cannot be exported from semaphore with imported payload with handle type %s",
                             string_VkExternalSemaphoreHandleTypeFlagBits(pGetFdInfo->handleType),
                             string_VkExternalSemaphoreHandleTypeFlagBits(sem_state->ImportedHandleType()));
        }

        if (pGetFdInfo->handleType == VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_SYNC_FD_BIT) {
            if (sem_state->type != VK_SEMAPHORE_TYPE_BINARY) {
                skip |=
                    LogError("VUID-VkSemaphoreGetFdInfoKHR-handleType-03253", sem_state->Handle(), info_loc.dot(Field::handleType),
                             "is VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_SYNC_FD_BIT, but semaphore type is %s.",
                             string_VkSemaphoreType(sem_state->type));
            } else if (!sem_state->CanBinaryBeWaited()) {
                skip |= LogError("VUID-VkSemaphoreGetFdInfoKHR-handleType-03254", sem_state->Handle(),
                                 info_loc.dot(Field::semaphore), "must be signaled or have a pending signal operation.");
            }
        }
    }
    return skip;
}

bool CoreChecks::ValidateImportFence(VkFence fence, const char *vuid, const Location &loc) const {
    auto fence_node = Get<vvl::Fence>(fence);
    bool skip = false;
    if (fence_node && fence_node->Scope() == vvl::Fence::kInternal && fence_node->State() == vvl::Fence::kInflight) {
        skip |= LogError(vuid, fence, loc.dot(Field::fence), "(%s) is currently in use.", FormatHandle(fence).c_str());
    }
    return skip;
}

bool CoreChecks::PreCallValidateImportFenceFdKHR(VkDevice device, const VkImportFenceFdInfoKHR *pImportFenceFdInfo,
                                                 const ErrorObject &error_obj) const {
    return ValidateImportFence(pImportFenceFdInfo->fence, "VUID-vkImportFenceFdKHR-fence-01463",
                               error_obj.location.dot(Field::pImportFenceFdInfo));
}

bool CoreChecks::PreCallValidateGetFenceFdKHR(VkDevice device, const VkFenceGetFdInfoKHR *pGetFdInfo, int *pFd,
                                              const ErrorObject &error_obj) const {
    bool skip = false;
    auto fence_state = Get<vvl::Fence>(pGetFdInfo->fence);
    if (fence_state) {
        const Location info_loc = error_obj.location.dot(Field::pGetFdInfo);
        if ((pGetFdInfo->handleType & fence_state->exportHandleTypes) == 0) {
            skip |= LogError("VUID-VkFenceGetFdInfoKHR-handleType-01453", fence_state->Handle(), info_loc.dot(Field::handleType),
                             "(%s) is different from VkExportFenceCreateInfo::handleTypes (%s). ",
                             string_VkExternalFenceHandleTypeFlagBits(pGetFdInfo->handleType),
                             string_VkExternalFenceHandleTypeFlags(fence_state->exportHandleTypes).c_str());
        }

        if (fence_state->Scope() != vvl::Fence::kInternal &&
            !CanFenceExportFromImported(physical_device, pGetFdInfo->handleType, fence_state->ImportedHandleType())) {
            skip |= LogError("VUID-VkFenceGetFdInfoKHR-fence-01455", fence_state->Handle(), info_loc.dot(Field::handleType),
                             "(%s) cannot be exported from fence with imported payload with handle type %s",
                             string_VkExternalFenceHandleTypeFlagBits(pGetFdInfo->handleType),
                             string_VkExternalFenceHandleTypeFlagBits(fence_state->ImportedHandleType()));
        }

        if (pGetFdInfo->handleType == VK_EXTERNAL_FENCE_HANDLE_TYPE_SYNC_FD_BIT && fence_state->State() == vvl::Fence::kUnsignaled) {
            skip |= LogError("VUID-VkFenceGetFdInfoKHR-handleType-01454", fence_state->Handle(), info_loc.dot(Field::handleType),
                             "is VK_EXTERNAL_FENCE_HANDLE_TYPE_SYNC_FD_BIT which cannot be exported unless the fence has a pending "
                             "signal operation or is already signaled.");
        }
    }
    return skip;
}

#ifdef VK_USE_PLATFORM_WIN32_KHR
bool CoreChecks::PreCallValidateGetMemoryWin32HandleKHR(VkDevice device, const VkMemoryGetWin32HandleInfoKHR *pGetWin32HandleInfo,
                                                        HANDLE *pHandle, const ErrorObject &error_obj) const {
    bool skip = false;
    if (const auto memory_state = Get<vvl::DeviceMemory>(pGetWin32HandleInfo->memory)) {
        const auto export_info = vku::FindStructInPNextChain<VkExportMemoryAllocateInfo>(memory_state->alloc_info.pNext);
        if (!export_info) {
            skip |= LogError("VUID-VkMemoryGetWin32HandleInfoKHR-handleType-00662", pGetWin32HandleInfo->memory,
                             error_obj.location.dot(Field::pGetWin32HandleInfo).dot(Field::memory),
                             "pNext chain does not include a VkExportMemoryAllocateInfo structure.");
        } else if ((export_info->handleTypes & pGetWin32HandleInfo->handleType) == 0) {
            skip |= LogError("VUID-VkMemoryGetWin32HandleInfoKHR-handleType-00662", pGetWin32HandleInfo->memory,
                             error_obj.location.dot(Field::pGetWin32HandleInfo).dot(Field::memory),
                             "the requested handle type (%s) is not included in the memory's "
                             "VkExportMemoryAllocateInfo::handleTypes (%s).",
                             string_VkExternalMemoryHandleTypeFlagBits(pGetWin32HandleInfo->handleType),
                             string_VkExternalMemoryHandleTypeFlags(export_info->handleTypes).c_str());
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateImportSemaphoreWin32HandleKHR(
    VkDevice device, const VkImportSemaphoreWin32HandleInfoKHR *pImportSemaphoreWin32HandleInfo,
    const ErrorObject &error_obj) const {
    bool skip = false;
    auto sem_state = Get<vvl::Semaphore>(pImportSemaphoreWin32HandleInfo->semaphore);
    if (sem_state) {
        // Waiting for: https://gitlab.khronos.org/vulkan/vulkan/-/issues/3507
        skip |= ValidateObjectNotInUse(sem_state.get(), error_obj.location, kVUIDUndefined);

        if ((pImportSemaphoreWin32HandleInfo->flags & VK_SEMAPHORE_IMPORT_TEMPORARY_BIT) != 0 &&
            sem_state->type == VK_SEMAPHORE_TYPE_TIMELINE) {
            skip |= LogError("VUID-VkImportSemaphoreWin32HandleInfoKHR-flags-03322", sem_state->Handle(),
                             error_obj.location.dot(Field::pImportSemaphoreWin32HandleInfo).dot(Field::semaphore),
                             "includes VK_SEMAPHORE_IMPORT_TEMPORARY_BIT and semaphore is VK_SEMAPHORE_TYPE_TIMELINE.");
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateGetSemaphoreWin32HandleKHR(VkDevice device,
                                                           const VkSemaphoreGetWin32HandleInfoKHR *pGetWin32HandleInfo,
                                                           HANDLE *pHandle, const ErrorObject &error_obj) const {
    bool skip = false;
    auto sem_state = Get<vvl::Semaphore>(pGetWin32HandleInfo->semaphore);
    if (sem_state) {
        if ((pGetWin32HandleInfo->handleType & sem_state->exportHandleTypes) == 0) {
            skip |= LogError("VUID-VkSemaphoreGetWin32HandleInfoKHR-handleType-01126", sem_state->Handle(),
                             error_obj.location.dot(Field::pGetWin32HandleInfo).dot(Field::handleType),
                             "(%s) is different from VkExportSemaphoreCreateInfo::handleTypes (%s)",
                             string_VkExternalSemaphoreHandleTypeFlagBits(pGetWin32HandleInfo->handleType),
                             string_VkExternalSemaphoreHandleTypeFlags(sem_state->exportHandleTypes).c_str());
        }

        if (sem_state->Scope() != vvl::Semaphore::kInternal &&
            !CanSemaphoreExportFromImported(physical_device, pGetWin32HandleInfo->handleType, sem_state->ImportedHandleType())) {
            skip |= LogError("VUID-VkSemaphoreGetWin32HandleInfoKHR-semaphore-01128", sem_state->Handle(),
                             error_obj.location.dot(Field::pGetWin32HandleInfo).dot(Field::handleType),
                             "(%s) cannot be exported from semaphore with imported payload with handle type %s",
                             string_VkExternalSemaphoreHandleTypeFlagBits(pGetWin32HandleInfo->handleType),
                             string_VkExternalSemaphoreHandleTypeFlagBits(sem_state->ImportedHandleType()));
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateImportFenceWin32HandleKHR(VkDevice device,
                                                          const VkImportFenceWin32HandleInfoKHR *pImportFenceWin32HandleInfo,
                                                          const ErrorObject &error_obj) const {
    return ValidateImportFence(pImportFenceWin32HandleInfo->fence, "VUID-vkImportFenceWin32HandleKHR-fence-04448",
                               error_obj.location.dot(Field::pImportFenceWin32HandleInfo));
}

bool CoreChecks::PreCallValidateGetFenceWin32HandleKHR(VkDevice device, const VkFenceGetWin32HandleInfoKHR *pGetWin32HandleInfo,
                                                       HANDLE *pHandle, const ErrorObject &error_obj) const {
    bool skip = false;
    auto fence_state = Get<vvl::Fence>(pGetWin32HandleInfo->fence);
    if (fence_state) {
        if ((pGetWin32HandleInfo->handleType & fence_state->exportHandleTypes) == 0) {
            skip |= LogError("VUID-VkFenceGetWin32HandleInfoKHR-handleType-01448", fence_state->Handle(),
                             error_obj.location.dot(Field::pGetWin32HandleInfo).dot(Field::handleType),
                             "(%s) is different from VkExportFenceCreateInfo::handleTypes (%s)",
                             string_VkExternalFenceHandleTypeFlagBits(pGetWin32HandleInfo->handleType),
                             string_VkExternalFenceHandleTypeFlags(fence_state->exportHandleTypes).c_str());
        }

        if (fence_state->Scope() != vvl::Fence::kInternal &&
            !CanFenceExportFromImported(physical_device, pGetWin32HandleInfo->handleType, fence_state->ImportedHandleType())) {
            skip |= LogError("VUID-VkFenceGetWin32HandleInfoKHR-fence-01450", fence_state->Handle(),
                             error_obj.location.dot(Field::pGetWin32HandleInfo).dot(Field::handleType),
                             "(%s) cannot be exported from fence with imported payload with handle type %s",
                             string_VkExternalFenceHandleTypeFlagBits(pGetWin32HandleInfo->handleType),
                             string_VkExternalFenceHandleTypeFlagBits(fence_state->ImportedHandleType()));
        }
    }
    return skip;
}
#endif  // VK_USE_PLATFORM_WIN32_KHR

#ifdef VK_USE_PLATFORM_FUCHSIA
bool CoreChecks::PreCallValidateImportSemaphoreZirconHandleFUCHSIA(
    VkDevice device, const VkImportSemaphoreZirconHandleInfoFUCHSIA *pImportSemaphoreZirconHandleInfo,
    const ErrorObject &error_obj) const {
    bool skip = false;
    auto sem_state = Get<vvl::Semaphore>(pImportSemaphoreZirconHandleInfo->semaphore);
    if (sem_state) {
        skip |= ValidateObjectNotInUse(sem_state.get(), error_obj.location,
                                       "VUID-vkImportSemaphoreZirconHandleFUCHSIA-semaphore-04764");

        if (sem_state->type == VK_SEMAPHORE_TYPE_TIMELINE) {
            skip |= LogError("VUID-VkImportSemaphoreZirconHandleInfoFUCHSIA-semaphoreType-04768", sem_state->Handle(),
                             error_obj.location.dot(Field::pImportSemaphoreZirconHandleInfo).dot(Field::semaphore),
                             "was created with VK_SEMAPHORE_TYPE_TIMELINE.");
        }
    }
    return skip;
}

void CoreChecks::PostCallRecordImportSemaphoreZirconHandleFUCHSIA(
    VkDevice device, const VkImportSemaphoreZirconHandleInfoFUCHSIA *pImportSemaphoreZirconHandleInfo,
    const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;
    RecordImportSemaphoreState(pImportSemaphoreZirconHandleInfo->semaphore, pImportSemaphoreZirconHandleInfo->handleType,
                               pImportSemaphoreZirconHandleInfo->flags);
}

void CoreChecks::PostCallRecordGetSemaphoreZirconHandleFUCHSIA(VkDevice device,
                                                               const VkSemaphoreGetZirconHandleInfoFUCHSIA *pGetZirconHandleInfo,
                                                               zx_handle_t *pZirconHandle, const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;
    RecordGetExternalSemaphoreState(pGetZirconHandleInfo->semaphore, pGetZirconHandleInfo->handleType);
}
#endif

#ifdef VK_USE_PLATFORM_METAL_EXT
bool CoreChecks::PreCallValidateExportMetalObjectsEXT(VkDevice device, VkExportMetalObjectsInfoEXT *pMetalObjectsInfo,
                                                      const ErrorObject &error_obj) const {
    bool skip = false;
    const VkBaseOutStructure *metal_objects_info_ptr = reinterpret_cast<const VkBaseOutStructure *>(pMetalObjectsInfo->pNext);
    while (metal_objects_info_ptr) {
        switch (metal_objects_info_ptr->sType) {
            case VK_STRUCTURE_TYPE_EXPORT_METAL_DEVICE_INFO_EXT:
                if (std::find(instance_state->export_metal_flags.begin(), instance_state->export_metal_flags.end(),
                              VK_EXPORT_METAL_OBJECT_TYPE_METAL_DEVICE_BIT_EXT) == instance_state->export_metal_flags.end()) {
                    skip |= LogError(
                        "VUID-VkExportMetalObjectsInfoEXT-pNext-06791", device, error_obj.location,
                        "pNext chain contains a VkExportMetalDeviceInfoEXT structure "
                        "but instance %s did not have a "
                        "VkExportMetalObjectCreateInfoEXT struct with exportObjectType of "
                        "VK_EXPORT_METAL_OBJECT_TYPE_METAL_DEVICE_BIT_EXT in the pNext chain of its VkInstanceCreateInfo structure",
                        FormatHandle(instance_state->instance).c_str());
                }
                break;

            case VK_STRUCTURE_TYPE_EXPORT_METAL_COMMAND_QUEUE_INFO_EXT:
                if (std::find(instance_state->export_metal_flags.begin(), instance_state->export_metal_flags.end(),
                              VK_EXPORT_METAL_OBJECT_TYPE_METAL_COMMAND_QUEUE_BIT_EXT) ==
                    instance_state->export_metal_flags.end()) {
                    skip |= LogError("VUID-VkExportMetalObjectsInfoEXT-pNext-06792", device, error_obj.location,
                                     "pNext chain contains a VkExportMetalCommandQueueInfoEXT structure "
                                     "but instance %s did not have a "
                                     "VkExportMetalObjectCreateInfoEXT struct with exportObjectType of "
                                     "VK_EXPORT_METAL_OBJECT_TYPE_METAL_COMMAND_QUEUE_BIT_EXT in the pNext chain of its "
                                     "VkInstanceCreateInfo structure",
                                     FormatHandle(instance_state->instance).c_str());
                }
                break;

            case VK_STRUCTURE_TYPE_EXPORT_METAL_BUFFER_INFO_EXT: {
                auto metal_buffer_ptr = reinterpret_cast<const VkExportMetalBufferInfoEXT *>(metal_objects_info_ptr);
                auto mem_info = Get<vvl::DeviceMemory>(metal_buffer_ptr->memory);
                if (mem_info) {
                    if (!mem_info->metal_buffer_export) {
                        skip |= LogError(
                            "VUID-VkExportMetalObjectsInfoEXT-pNext-06793", device, error_obj.location,
                            "pNext chain contains a VkExportMetalBufferInfoEXT structure with memory = "
                            "%s, but that memory was not allocated with a VkExportMetalObjectCreateInfoEXT whose exportObjectType "
                            "member was set to VK_EXPORT_METAL_OBJECT_TYPE_METAL_BUFFER_BIT_EXT in the pNext chain of the "
                            "VkMemoryAllocateInfo structure",
                            FormatHandle(metal_buffer_ptr->memory).c_str());
                    }
                }
            } break;

            case VK_STRUCTURE_TYPE_EXPORT_METAL_TEXTURE_INFO_EXT: {
                auto metal_texture_ptr = reinterpret_cast<const VkExportMetalTextureInfoEXT *>(metal_objects_info_ptr);
                if ((metal_texture_ptr->image == VK_NULL_HANDLE && metal_texture_ptr->imageView == VK_NULL_HANDLE &&
                     metal_texture_ptr->bufferView == VK_NULL_HANDLE) ||
                    (metal_texture_ptr->image &&
                     ((metal_texture_ptr->imageView != VK_NULL_HANDLE) || (metal_texture_ptr->bufferView != VK_NULL_HANDLE))) ||
                    (metal_texture_ptr->imageView &&
                     ((metal_texture_ptr->image != VK_NULL_HANDLE) || (metal_texture_ptr->bufferView != VK_NULL_HANDLE))) ||
                    (metal_texture_ptr->bufferView &&
                     ((metal_texture_ptr->image != VK_NULL_HANDLE) || (metal_texture_ptr->imageView != VK_NULL_HANDLE)))) {
                    skip |=
                        LogError("VUID-VkExportMetalObjectsInfoEXT-pNext-06794", device, error_obj.location,
                                 "pNext chain contains a VkExportMetalTextureInfoEXT structure with image = "
                                 "%s, imageView = %s and bufferView = %s, but exactly one of those 3 must not be VK_NULL_HANDLE",
                                 FormatHandle(metal_texture_ptr->image).c_str(), FormatHandle(metal_texture_ptr->imageView).c_str(),
                                 FormatHandle(metal_texture_ptr->bufferView).c_str());
                }
                if (metal_texture_ptr->image) {
                    auto image_info = Get<vvl::Image>(metal_texture_ptr->image);
                    if (image_info) {
                        if (!image_info->metal_image_export) {
                            skip |= LogError(
                                "VUID-VkExportMetalObjectsInfoEXT-pNext-06795", device, error_obj.location,
                                "pNext chain contains a VkExportMetalTextureInfoEXT structure with image = "
                                "%s, but that image was not created with a VkExportMetalObjectCreateInfoEXT whose exportObjectType "
                                "member was set to VK_EXPORT_METAL_OBJECT_TYPE_METAL_TEXTURE_BIT_EXT in the pNext chain of the "
                                "VkImageCreateInfo structure",
                                FormatHandle(metal_texture_ptr->image).c_str());
                        }
                        auto format_plane_count = vkuFormatPlaneCount(image_info->createInfo.format);
                        auto image_plane = metal_texture_ptr->plane;
                        if (!(format_plane_count > 1) && (image_plane != VK_IMAGE_ASPECT_PLANE_0_BIT)) {
                            skip |= LogError(
                                "VUID-VkExportMetalObjectsInfoEXT-pNext-06799", device, error_obj.location,
                                "pNext chain contains a VkExportMetalTextureInfoEXT structure with image = "
                                "%s, and plane = %s, but image was created with format %s, which is not multiplaner and plane is "
                                "required to be VK_IMAGE_ASPECT_PLANE_0_BIT",
                                FormatHandle(metal_texture_ptr->image).c_str(), string_VkImageAspectFlags(image_plane).c_str(),
                                string_VkFormat(image_info->createInfo.format));
                        }
                        if ((format_plane_count == 2) && (image_plane == VK_IMAGE_ASPECT_PLANE_2_BIT)) {
                            skip |= LogError(
                                "VUID-VkExportMetalObjectsInfoEXT-pNext-06800", device, error_obj.location,
                                "pNext chain contains a VkExportMetalTextureInfoEXT structure with image = "
                                "%s, and plane = %s, but image was created with format %s, which has exactly 2 planes and plane "
                                "cannot"
                                "be VK_IMAGE_ASPECT_PLANE_2_BIT",
                                FormatHandle(metal_texture_ptr->image).c_str(), string_VkImageAspectFlags(image_plane).c_str(),
                                string_VkFormat(image_info->createInfo.format));
                        }
                    }
                }
                if (metal_texture_ptr->imageView) {
                    auto image_view_info = Get<vvl::ImageView>(metal_texture_ptr->imageView);
                    if (image_view_info) {
                        if (!image_view_info->metal_imageview_export) {
                            skip |= LogError(
                                "VUID-VkExportMetalObjectsInfoEXT-pNext-06796", device, error_obj.location,
                                "pNext chain contains a VkExportMetalTextureInfoEXT structure with "
                                "imageView = "
                                "%s, but that image view was not created with a VkExportMetalObjectCreateInfoEXT whose "
                                "exportObjectType "
                                "member was set to VK_EXPORT_METAL_OBJECT_TYPE_METAL_TEXTURE_BIT_EXT in the pNext chain of the "
                                "VkImageViewCreateInfo structure",
                                FormatHandle(metal_texture_ptr->imageView).c_str());
                        }
                        auto format_plane_count = vkuFormatPlaneCount(image_view_info->create_info.format);
                        auto image_plane = metal_texture_ptr->plane;
                        if (!(format_plane_count > 1) && (image_plane != VK_IMAGE_ASPECT_PLANE_0_BIT)) {
                            skip |= LogError(
                                "VUID-VkExportMetalObjectsInfoEXT-pNext-06801", device, error_obj.location,
                                "pNext chain contains a VkExportMetalTextureInfoEXT structure with "
                                "imageView = "
                                "%s, and plane = %s, but imageView was created with format %s, which is not multiplaner and "
                                "plane is "
                                "required to be VK_IMAGE_ASPECT_PLANE_0_BIT",
                                FormatHandle(metal_texture_ptr->imageView).c_str(), string_VkImageAspectFlags(image_plane).c_str(),
                                string_VkFormat(image_view_info->create_info.format));
                        }
                        if ((format_plane_count == 2) && (image_plane == VK_IMAGE_ASPECT_PLANE_2_BIT)) {
                            skip |= LogError("VUID-VkExportMetalObjectsInfoEXT-pNext-06802", device, error_obj.location,
                                             "pNext chain contains a VkExportMetalTextureInfoEXT structure "
                                             "with imageView = "
                                             "%s, and plane = %s, but imageView was created with format %s, which has exactly 2 "
                                             "planes and plane "
                                             "cannot"
                                             "be VK_IMAGE_ASPECT_PLANE_2_BIT",
                                             FormatHandle(metal_texture_ptr->imageView).c_str(),
                                             string_VkImageAspectFlags(image_plane).c_str(),
                                             string_VkFormat(image_view_info->create_info.format));
                        }
                    }
                }
                if (metal_texture_ptr->bufferView) {
                    auto buffer_view_info = Get<vvl::BufferView>(metal_texture_ptr->bufferView);
                    if (buffer_view_info) {
                        if (!buffer_view_info->metal_bufferview_export) {
                            skip |= LogError(
                                "VUID-VkExportMetalObjectsInfoEXT-pNext-06797", device, error_obj.location,
                                "pNext chain contains a VkExportMetalTextureInfoEXT structure with "
                                "bufferView = "
                                "%s, but that buffer view was not created with a VkExportMetalObjectCreateInfoEXT whose "
                                "exportObjectType "
                                "member was set to VK_EXPORT_METAL_OBJECT_TYPE_METAL_TEXTURE_BIT_EXT in the pNext chain of the "
                                "VkBufferViewCreateInfo structure",
                                FormatHandle(metal_texture_ptr->bufferView).c_str());
                        }
                    }
                }
                if (metal_texture_ptr->image || metal_texture_ptr->imageView) {
                    if ((metal_texture_ptr->plane != VK_IMAGE_ASPECT_PLANE_0_BIT) &&
                        (metal_texture_ptr->plane != VK_IMAGE_ASPECT_PLANE_1_BIT) &&
                        (metal_texture_ptr->plane != VK_IMAGE_ASPECT_PLANE_2_BIT)) {
                        skip |= LogError(
                            "VUID-VkExportMetalObjectsInfoEXT-pNext-06798", device, error_obj.location,
                            "pNext chain contains a VkExportMetalTextureInfoEXT structure with "
                            "image = %s and imageView = "
                            "%s, but plane = %s which is not one of  VK_IMAGE_ASPECT_PLANE_0_BIT,  VK_IMAGE_ASPECT_PLANE_1_BIT, "
                            "or  VK_IMAGE_ASPECT_PLANE_2_BIT",
                            FormatHandle(metal_texture_ptr->image).c_str(), FormatHandle(metal_texture_ptr->imageView).c_str(),
                            string_VkImageAspectFlags(metal_texture_ptr->plane).c_str());
                    }
                }
            } break;

            case VK_STRUCTURE_TYPE_EXPORT_METAL_IO_SURFACE_INFO_EXT: {
                auto metal_io_surface_ptr = reinterpret_cast<const VkExportMetalIOSurfaceInfoEXT *>(metal_objects_info_ptr);
                auto image_info = Get<vvl::Image>(metal_io_surface_ptr->image);
                if (image_info) {
                    if (!image_info->metal_io_surface_export) {
                        skip |= LogError(
                            "VUID-VkExportMetalObjectsInfoEXT-pNext-06803", device, error_obj.location,
                            "pNext chain contains a VkExportMetalIOSurfaceInfoEXT structure with image = "
                            "%s, but that image was not created with a VkExportMetalObjectCreateInfoEXT whose exportObjectType "
                            "member was set to VK_EXPORT_METAL_OBJECT_TYPE_METAL_IOSURFACE_BIT_EXT in the pNext chain of the "
                            "VkImageCreateInfo structure",
                            FormatHandle(metal_io_surface_ptr->image).c_str());
                    }
                }
            } break;

            case VK_STRUCTURE_TYPE_EXPORT_METAL_SHARED_EVENT_INFO_EXT: {
                auto metal_shared_event_ptr = reinterpret_cast<const VkExportMetalSharedEventInfoEXT *>(metal_objects_info_ptr);
                if ((metal_shared_event_ptr->event == VK_NULL_HANDLE && metal_shared_event_ptr->semaphore == VK_NULL_HANDLE) ||
                    (metal_shared_event_ptr->event != VK_NULL_HANDLE && metal_shared_event_ptr->semaphore != VK_NULL_HANDLE)) {
                    skip |= LogError("VUID-VkExportMetalObjectsInfoEXT-pNext-06804", device, error_obj.location,
                                     "pNext chain contains a VkExportMetalSharedEventInfoEXT structure with semaphore = "
                                     "%s, and event = %s, but exactly one of them must not be VK_NULL_HANDLE",
                                     FormatHandle(metal_shared_event_ptr->semaphore).c_str(),
                                     FormatHandle(metal_shared_event_ptr->event).c_str());
                }

                if (metal_shared_event_ptr->semaphore) {
                    auto semaphore_info = Get<vvl::Semaphore>(metal_shared_event_ptr->semaphore);
                    if (semaphore_info && !(semaphore_info->metal_semaphore_export)) {
                        skip |= LogError(
                            "VUID-VkExportMetalObjectsInfoEXT-pNext-06805", device, error_obj.location,
                            "pNext chain contains a VkExportMetalSharedEventInfoEXT structure "
                            "with semaphore = "
                            "%s, but that semaphore was not created with a VkExportMetalObjectCreateInfoEXT whose exportObjectType "
                            "member was set to VK_EXPORT_METAL_OBJECT_TYPE_METAL_SHARED_EVENT_BIT_EXT in the pNext chain of the "
                            "VkSemaphoreCreateInfo structure",
                            FormatHandle(metal_shared_event_ptr->semaphore).c_str());
                    }
                }
                if (metal_shared_event_ptr->event) {
                    auto event_info = Get<vvl::Event>(metal_shared_event_ptr->event);
                    if (event_info && !(event_info->metal_event_export)) {
                        skip |= LogError(
                            "VUID-VkExportMetalObjectsInfoEXT-pNext-06806", device, error_obj.location,
                            "pNext chain contains a VkExportMetalSharedEventInfoEXT structure "
                            "with event = "
                            "%s, but that event was not created with a VkExportMetalObjectCreateInfoEXT whose exportObjectType "
                            "member was set to VK_EXPORT_METAL_OBJECT_TYPE_METAL_SHARED_EVENT_BIT_EXT in the pNext chain of the "
                            "VkEventCreateInfo structure",
                            FormatHandle(metal_shared_event_ptr->event).c_str());
                    }
                }
            } break;
            default:
                break;
        }
        metal_objects_info_ptr = metal_objects_info_ptr->pNext;
    }
    return skip;
}
#endif  // VK_USE_PLATFORM_METAL_EXT
