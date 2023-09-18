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

#include <vulkan/vk_enum_string_helper.h>
#include "generated/chassis.h"
#include "core_validation.h"

bool CoreChecks::PreCallValidateGetMemoryFdKHR(VkDevice device, const VkMemoryGetFdInfoKHR *pGetFdInfo, int *pFd,
                                               const ErrorObject &error_obj) const {
    bool skip = false;
    if (const auto memory_state = Get<DEVICE_MEMORY_STATE>(pGetFdInfo->memory)) {
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

bool CoreChecks::PreCallValidateImportSemaphoreFdKHR(VkDevice device, const VkImportSemaphoreFdInfoKHR *info,
                                                     const ErrorObject &error_obj) const {
    bool skip = false;
    auto sem_state = Get<SEMAPHORE_STATE>(info->semaphore);
    if (sem_state) {
        skip |= ValidateObjectNotInUse(sem_state.get(), error_obj.location, "VUID-vkImportSemaphoreFdKHR-semaphore-01142");

        if ((info->flags & VK_SEMAPHORE_IMPORT_TEMPORARY_BIT) != 0 && sem_state->type == VK_SEMAPHORE_TYPE_TIMELINE) {
            skip |= LogError("VUID-VkImportSemaphoreFdInfoKHR-flags-03323", sem_state->Handle(),
                             error_obj.location.dot(Field::info).dot(Field::flags),
                             "includes VK_SEMAPHORE_IMPORT_TEMPORARY_BIT and semaphore is VK_SEMAPHORE_TYPE_TIMELINE.");
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateGetSemaphoreFdKHR(VkDevice device, const VkSemaphoreGetFdInfoKHR *info, int *pFd,
                                                  const ErrorObject &error_obj) const {
    bool skip = false;
    auto sem_state = Get<SEMAPHORE_STATE>(info->semaphore);
    if (sem_state) {
        if ((info->handleType & sem_state->exportHandleTypes) == 0) {
            skip |= LogError("VUID-VkSemaphoreGetFdInfoKHR-handleType-01132", sem_state->Handle(),
                             error_obj.location.dot(Field::info).dot(Field::handleType),
                             "(%s) is different from VkExportSemaphoreCreateInfo::handleTypes (%s).",
                             string_VkExternalSemaphoreHandleTypeFlagBits(info->handleType),
                             string_VkExternalSemaphoreHandleTypeFlags(sem_state->exportHandleTypes).c_str());
        }

        if (info->handleType == VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_SYNC_FD_BIT) {
            if (sem_state->type != VK_SEMAPHORE_TYPE_BINARY) {
                skip |= LogError("VUID-VkSemaphoreGetFdInfoKHR-handleType-03253", sem_state->Handle(),
                                 error_obj.location.dot(Field::info).dot(Field::handleType),
                                 "is VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_SYNC_FD_BIT, but semaphore type is %s.",
                                 string_VkSemaphoreType(sem_state->type));
            }
            if (!sem_state->CanBeWaited()) {
                skip |= LogError("VUID-VkSemaphoreGetFdInfoKHR-handleType-03254", sem_state->Handle(),
                                 error_obj.location.dot(Field::info).dot(Field::semaphore),
                                 "must be signaled or have a pending signal operation.");
            }
        }
    }
    return skip;
}

bool CoreChecks::ValidateImportFence(VkFence fence, const char *vuid, const Location &loc) const {
    auto fence_node = Get<FENCE_STATE>(fence);
    bool skip = false;
    if (fence_node && fence_node->Scope() == kSyncScopeInternal && fence_node->State() == FENCE_INFLIGHT) {
        skip |= LogError(vuid, fence, loc.dot(Field::fence), "(%s) is currently in use.", FormatHandle(fence).c_str());
    }
    return skip;
}

bool CoreChecks::PreCallValidateImportFenceFdKHR(VkDevice device, const VkImportFenceFdInfoKHR *pImportFenceFdInfo,
                                                 const ErrorObject &error_obj) const {
    return ValidateImportFence(pImportFenceFdInfo->fence, "VUID-vkImportFenceFdKHR-fence-01463",
                               error_obj.location.dot(Field::pImportFenceFdInfo));
}

bool CoreChecks::PreCallValidateGetFenceFdKHR(VkDevice device, const VkFenceGetFdInfoKHR *info, int *pFd,
                                              const ErrorObject &error_obj) const {
    bool skip = false;
    auto fence_state = Get<FENCE_STATE>(info->fence);
    if (fence_state) {
        if ((info->handleType & fence_state->exportHandleTypes) == 0) {
            skip |= LogError("VUID-VkFenceGetFdInfoKHR-handleType-01453", fence_state->Handle(),
                             error_obj.location.dot(Field::info).dot(Field::handleType),
                             "(%s) is different from VkExportFenceCreateInfo::handleTypes (%s). ",
                             string_VkExternalFenceHandleTypeFlagBits(info->handleType),
                             string_VkExternalFenceHandleTypeFlags(fence_state->exportHandleTypes).c_str());
        }
        if (info->handleType == VK_EXTERNAL_FENCE_HANDLE_TYPE_SYNC_FD_BIT && fence_state->State() == FENCE_UNSIGNALED) {
            skip |= LogError("VUID-VkFenceGetFdInfoKHR-handleType-01454", fence_state->Handle(),
                             error_obj.location.dot(Field::info).dot(Field::handleType),
                             "is VK_EXTERNAL_FENCE_HANDLE_TYPE_SYNC_FD_BIT which cannot be exported unless the fence has a pending "
                             "signal operation or is already signaled.");
        }
    }
    return skip;
}

#ifdef VK_USE_PLATFORM_WIN32_KHR
bool CoreChecks::PreCallValidateImportSemaphoreWin32HandleKHR(VkDevice device, const VkImportSemaphoreWin32HandleInfoKHR *info,
                                                              const ErrorObject &error_obj) const {
    bool skip = false;
    auto sem_state = Get<SEMAPHORE_STATE>(info->semaphore);
    if (sem_state) {
        // Waiting for: https://gitlab.khronos.org/vulkan/vulkan/-/issues/3507
        skip |= ValidateObjectNotInUse(sem_state.get(), error_obj.location, kVUIDUndefined);

        if ((info->flags & VK_SEMAPHORE_IMPORT_TEMPORARY_BIT) != 0 && sem_state->type == VK_SEMAPHORE_TYPE_TIMELINE) {
            skip |= LogError("VUID-VkImportSemaphoreWin32HandleInfoKHR-flags-03322", sem_state->Handle(),
                             error_obj.location.dot(Field::info).dot(Field::semaphore),
                             "includes VK_SEMAPHORE_IMPORT_TEMPORARY_BIT and semaphore is VK_SEMAPHORE_TYPE_TIMELINE.");
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateGetSemaphoreWin32HandleKHR(VkDevice device, const VkSemaphoreGetWin32HandleInfoKHR *info,
                                                           HANDLE *pHandle, const ErrorObject &error_obj) const {
    bool skip = false;
    auto sem_state = Get<SEMAPHORE_STATE>(info->semaphore);
    if (sem_state) {
        if ((info->handleType & sem_state->exportHandleTypes) == 0) {
            skip |= LogError("VUID-VkSemaphoreGetWin32HandleInfoKHR-handleType-01126", sem_state->Handle(),
                             error_obj.location.dot(Field::info).dot(Field::handleType),
                             "(%s) is different from VkExportSemaphoreCreateInfo::handleTypes (%s)",
                             string_VkExternalSemaphoreHandleTypeFlagBits(info->handleType),
                             string_VkExternalSemaphoreHandleTypeFlags(sem_state->exportHandleTypes).c_str());
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

bool CoreChecks::PreCallValidateGetFenceWin32HandleKHR(VkDevice device, const VkFenceGetWin32HandleInfoKHR *info, HANDLE *pHandle,
                                                       const ErrorObject &error_obj) const {
    bool skip = false;
    auto fence_state = Get<FENCE_STATE>(info->fence);
    if (fence_state) {
        if ((info->handleType & fence_state->exportHandleTypes) == 0) {
            skip |= LogError("VUID-VkFenceGetWin32HandleInfoKHR-handleType-01448", fence_state->Handle(),
                             error_obj.location.dot(Field::info).dot(Field::handleType),
                             "(%s) is different from VkExportFenceCreateInfo::handleTypes (%s)",
                             string_VkExternalFenceHandleTypeFlagBits(info->handleType),
                             string_VkExternalFenceHandleTypeFlags(fence_state->exportHandleTypes).c_str());
        }
    }
    return skip;
}
#endif  // VK_USE_PLATFORM_WIN32_KHR

#ifdef VK_USE_PLATFORM_FUCHSIA
bool CoreChecks::PreCallValidateImportSemaphoreZirconHandleFUCHSIA(VkDevice device,
                                                                   const VkImportSemaphoreZirconHandleInfoFUCHSIA *info,
                                                                   const ErrorObject &error_obj) const {
    bool skip = false;
    auto sem_state = Get<SEMAPHORE_STATE>(info->semaphore);
    if (sem_state) {
        skip |= ValidateObjectNotInUse(sem_state.get(), error_obj.location,
                                       "VUID-vkImportSemaphoreZirconHandleFUCHSIA-semaphore-04764");

        if (sem_state->type == VK_SEMAPHORE_TYPE_TIMELINE) {
            skip |=
                LogError("VUID-VkImportSemaphoreZirconHandleInfoFUCHSIA-semaphoreType-04768", sem_state->Handle(),
                         error_obj.location.dot(Field::info).dot(Field::semaphore), "was created with VK_SEMAPHORE_TYPE_TIMELINE.");
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
                auto mem_info = Get<DEVICE_MEMORY_STATE>(metal_buffer_ptr->memory);
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
                    auto image_info = Get<IMAGE_STATE>(metal_texture_ptr->image);
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
                    auto image_view_info = Get<IMAGE_VIEW_STATE>(metal_texture_ptr->imageView);
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
                    auto buffer_view_info = Get<BUFFER_VIEW_STATE>(metal_texture_ptr->bufferView);
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
                auto image_info = Get<IMAGE_STATE>(metal_io_surface_ptr->image);
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
                    auto semaphore_info = Get<SEMAPHORE_STATE>(metal_shared_event_ptr->semaphore);
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
                    auto event_info = Get<EVENT_STATE>(metal_shared_event_ptr->event);
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
