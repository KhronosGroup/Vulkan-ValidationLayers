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
#include "generated/enum_flag_bits.h"

bool StatelessValidation::manual_PreCallValidateGetMemoryFdKHR(VkDevice device, const VkMemoryGetFdInfoKHR *pGetFdInfo, int *pFd,
                                                               const ErrorObject &error_obj) const {
    constexpr auto allowed_types = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT | VK_EXTERNAL_MEMORY_HANDLE_TYPE_DMA_BUF_BIT_EXT;
    bool skip = false;
    if (0 == (pGetFdInfo->handleType & allowed_types)) {
        skip |= LogError("VUID-VkMemoryGetFdInfoKHR-handleType-00672", pGetFdInfo->memory, error_obj.location,
                         "handle type %s is not one of the supported handle types.",
                         string_VkExternalMemoryHandleTypeFlagBits(pGetFdInfo->handleType));
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateGetMemoryFdPropertiesKHR(VkDevice device,
                                                                         VkExternalMemoryHandleTypeFlagBits handleType, int fd,
                                                                         VkMemoryFdPropertiesKHR *pMemoryFdProperties,
                                                                         const ErrorObject &error_obj) const {
    bool skip = false;
    if (fd < 0) {
        skip |= LogError("VUID-vkGetMemoryFdPropertiesKHR-fd-00673", device, error_obj.location,
                         "fd handle (%d) is not a valid POSIX file descriptor.", fd);
    }
    if (handleType == VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT) {
        skip |= LogError("VUID-vkGetMemoryFdPropertiesKHR-handleType-00674", device, error_obj.location,
                         "opaque handle type %s is not allowed.", string_VkExternalMemoryHandleTypeFlagBits(handleType));
    }
    return skip;
}

bool StatelessValidation::ValidateExternalSemaphoreHandleType(VkSemaphore semaphore, const char *vuid,
                                                              const Location &handle_type_loc,
                                                              VkExternalSemaphoreHandleTypeFlagBits handle_type,
                                                              VkExternalSemaphoreHandleTypeFlags allowed_types) const {
    bool skip = false;
    if (0 == (handle_type & allowed_types)) {
        skip |= LogError(vuid, semaphore, handle_type_loc, "%s is not one of the supported handleTypes (%s).",
                         string_VkExternalSemaphoreHandleTypeFlagBits(handle_type),
                         string_VkExternalSemaphoreHandleTypeFlags(allowed_types).c_str());
    }
    return skip;
}

bool StatelessValidation::ValidateExternalFenceHandleType(VkFence fence, const char *vuid, const Location &handle_type_loc,
                                                          VkExternalFenceHandleTypeFlagBits handle_type,
                                                          VkExternalFenceHandleTypeFlags allowed_types) const {
    bool skip = false;
    if (0 == (handle_type & allowed_types)) {
        skip |= LogError(vuid, fence, handle_type_loc, "%s is not one of the supported handleTypes (%s).",
                         string_VkExternalFenceHandleTypeFlagBits(handle_type),
                         string_VkExternalFenceHandleTypeFlags(allowed_types).c_str());
    }
    return skip;
}

static constexpr VkExternalSemaphoreHandleTypeFlags kSemFdHandleTypes =
    VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_FD_BIT | VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_SYNC_FD_BIT;

bool StatelessValidation::manual_PreCallValidateGetSemaphoreFdKHR(VkDevice device, const VkSemaphoreGetFdInfoKHR *info, int *pFd,
                                                                  const ErrorObject &error_obj) const {
    return ValidateExternalSemaphoreHandleType(info->semaphore, "VUID-VkSemaphoreGetFdInfoKHR-handleType-01136",
                                               error_obj.location.dot(Field::info).dot(Field::handleType), info->handleType,
                                               kSemFdHandleTypes);
}

bool StatelessValidation::manual_PreCallValidateImportSemaphoreFdKHR(VkDevice device, const VkImportSemaphoreFdInfoKHR *info,
                                                                     const ErrorObject &error_obj) const {
    bool skip = false;
    skip |= ValidateExternalSemaphoreHandleType(info->semaphore, "VUID-VkImportSemaphoreFdInfoKHR-handleType-01143",
                                                error_obj.location.dot(Field::info).dot(Field::handleType), info->handleType,
                                                kSemFdHandleTypes);

    if (info->handleType == VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_SYNC_FD_BIT &&
        (info->flags & VK_SEMAPHORE_IMPORT_TEMPORARY_BIT) == 0) {
        skip |= LogError("VUID-VkImportSemaphoreFdInfoKHR-handleType-07307", info->semaphore, error_obj.location.dot(Field::info),
                         "handleType is VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_SYNC_FD_BIT so"
                         " VK_SEMAPHORE_IMPORT_TEMPORARY_BIT must be set, but flags is 0x%x",
                         info->flags);
    }
    return skip;
}

static constexpr VkExternalFenceHandleTypeFlags kFenceFdHandleTypes =
    VK_EXTERNAL_FENCE_HANDLE_TYPE_OPAQUE_FD_BIT | VK_EXTERNAL_FENCE_HANDLE_TYPE_SYNC_FD_BIT;

bool StatelessValidation::manual_PreCallValidateGetFenceFdKHR(VkDevice device, const VkFenceGetFdInfoKHR *info, int *pFd,
                                                              const ErrorObject &error_obj) const {
    return ValidateExternalFenceHandleType(info->fence, "VUID-VkFenceGetFdInfoKHR-handleType-01456",
                                           error_obj.location.dot(Field::info).dot(Field::handleType), info->handleType,
                                           kFenceFdHandleTypes);
}

bool StatelessValidation::manual_PreCallValidateImportFenceFdKHR(VkDevice device, const VkImportFenceFdInfoKHR *info,
                                                                 const ErrorObject &error_obj) const {
    bool skip = false;
    skip |= ValidateExternalFenceHandleType(info->fence, "VUID-VkImportFenceFdInfoKHR-handleType-01464",
                                            error_obj.location.dot(Field::info).dot(Field::handleType), info->handleType,
                                            kFenceFdHandleTypes);

    if (info->handleType == VK_EXTERNAL_FENCE_HANDLE_TYPE_SYNC_FD_BIT && (info->flags & VK_FENCE_IMPORT_TEMPORARY_BIT) == 0) {
        skip |= LogError("VUID-VkImportFenceFdInfoKHR-handleType-07306", info->fence, error_obj.location.dot(Field::info),
                         "handleType is VK_EXTERNAL_FENCE_HANDLE_TYPE_SYNC_FD_BIT so"
                         " VK_FENCE_IMPORT_TEMPORARY_BIT must be set, but flags is 0x%x",
                         info->flags);
    }
    return skip;
}

#ifdef VK_USE_PLATFORM_WIN32_KHR
bool StatelessValidation::manual_PreCallValidateGetMemoryWin32HandlePropertiesKHR(
    VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType, HANDLE handle,
    VkMemoryWin32HandlePropertiesKHR *pMemoryWin32HandleProperties, const ErrorObject &error_obj) const {
    bool skip = false;
    if (handle == NULL || handle == INVALID_HANDLE_VALUE) {
        static_assert(sizeof(HANDLE) == sizeof(uintptr_t));  // to use PRIxPTR for HANDLE formatting
        skip |= LogError("VUID-vkGetMemoryWin32HandlePropertiesKHR-handle-00665", device, error_obj.location.dot(Field::handle),
                         "(0x%" PRIxPTR ") is not a valid Windows handle.", reinterpret_cast<std::uintptr_t>(handle));
    }
    if (handleType == VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT ||
        handleType == VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_KMT_BIT) {
        skip |=
            LogError("VUID-vkGetMemoryWin32HandlePropertiesKHR-handleType-00666", device, error_obj.location.dot(Field::handleType),
                     "%s is not allowed.", string_VkExternalMemoryHandleTypeFlagBits(handleType));
    }
    return skip;
}

static constexpr VkExternalSemaphoreHandleTypeFlags kSemWin32HandleTypes = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_WIN32_BIT |
                                                                           VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_WIN32_KMT_BIT |
                                                                           VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_D3D12_FENCE_BIT;

bool StatelessValidation::manual_PreCallValidateImportSemaphoreWin32HandleKHR(VkDevice device,
                                                                              const VkImportSemaphoreWin32HandleInfoKHR *info,
                                                                              const ErrorObject &error_obj) const {
    bool skip = false;

    skip |= ValidateExternalSemaphoreHandleType(info->semaphore, "VUID-VkImportSemaphoreWin32HandleInfoKHR-handleType-01140",
                                                error_obj.location.dot(Field::info).dot(Field::handleType), info->handleType,
                                                kSemWin32HandleTypes);

    static constexpr auto kNameAllowedTypes =
        VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_WIN32_BIT | VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_D3D12_FENCE_BIT;
    if ((info->handleType & kNameAllowedTypes) == 0 && info->name) {
        skip |=
            LogError("VUID-VkImportSemaphoreWin32HandleInfoKHR-handleType-01466", info->semaphore,
                     error_obj.location.dot(Field::info).dot(Field::name), "(%p) must be NULL if handleType is %s",
                     reinterpret_cast<const void *>(info->name), string_VkExternalSemaphoreHandleTypeFlagBits(info->handleType));
    }
    if (info->handle && info->name) {
        skip |=
            LogError("VUID-VkImportSemaphoreWin32HandleInfoKHR-handle-01469", info->semaphore, error_obj.location.dot(Field::info),
                     "both handle (%p) and name (%p) are non-NULL", info->handle, reinterpret_cast<const void *>(info->name));
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateGetSemaphoreWin32HandleKHR(VkDevice device,
                                                                           const VkSemaphoreGetWin32HandleInfoKHR *info,
                                                                           HANDLE *pHandle, const ErrorObject &error_obj) const {
    return ValidateExternalSemaphoreHandleType(info->semaphore, "VUID-VkSemaphoreGetWin32HandleInfoKHR-handleType-01131",
                                               error_obj.location.dot(Field::info).dot(Field::handleType), info->handleType,
                                               kSemWin32HandleTypes);
}

static constexpr VkExternalFenceHandleTypeFlags kFenceWin32HandleTypes =
    VK_EXTERNAL_FENCE_HANDLE_TYPE_OPAQUE_WIN32_BIT | VK_EXTERNAL_FENCE_HANDLE_TYPE_OPAQUE_WIN32_KMT_BIT;

bool StatelessValidation::manual_PreCallValidateImportFenceWin32HandleKHR(VkDevice device,
                                                                          const VkImportFenceWin32HandleInfoKHR *info,
                                                                          const ErrorObject &error_obj) const {
    bool skip = false;
    skip |= ValidateExternalFenceHandleType(info->fence, "VUID-VkImportFenceWin32HandleInfoKHR-handleType-01457",
                                            error_obj.location.dot(Field::info).dot(Field::handleType), info->handleType,
                                            kFenceWin32HandleTypes);

    static constexpr auto kNameAllowedTypes = VK_EXTERNAL_FENCE_HANDLE_TYPE_OPAQUE_WIN32_BIT;
    if ((info->handleType & kNameAllowedTypes) == 0 && info->name) {
        skip |= LogError("VUID-VkImportFenceWin32HandleInfoKHR-handleType-01459", info->fence,
                         error_obj.location.dot(Field::info).dot(Field::name), "(%p) must be NULL if handleType is %s",
                         reinterpret_cast<const void *>(info->name), string_VkExternalFenceHandleTypeFlagBits(info->handleType));
    }
    if (info->handle && info->name) {
        skip |= LogError("VUID-VkImportFenceWin32HandleInfoKHR-handle-01462", info->fence, error_obj.location.dot(Field::info),
                         "both handle (%p) and name (%p) are non-NULL", info->handle, reinterpret_cast<const void *>(info->name));
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateGetFenceWin32HandleKHR(VkDevice device, const VkFenceGetWin32HandleInfoKHR *info,
                                                                       HANDLE *pHandle, const ErrorObject &error_obj) const {
    return ValidateExternalFenceHandleType(info->fence, "VUID-VkFenceGetWin32HandleInfoKHR-handleType-01452",
                                           error_obj.location.dot(Field::info).dot(Field::handleType), info->handleType,
                                           kFenceWin32HandleTypes);
}
#endif

#ifdef VK_USE_PLATFORM_METAL_EXT
bool StatelessValidation::ExportMetalObjectsPNextUtil(VkExportMetalObjectTypeFlagBitsEXT bit, const char *vuid, const Location &loc,
                                                      const char *sType, const void *pNext) const {
    bool skip = false;
    auto export_metal_object_info = vku::FindStructInPNextChain<VkExportMetalObjectCreateInfoEXT>(pNext);
    while (export_metal_object_info) {
        if (export_metal_object_info->exportObjectType != bit) {
            skip |= LogError(vuid, device, loc,
                             "The pNext chain contains a VkExportMetalObjectCreateInfoEXT whose exportObjectType = %s, but only "
                             "VkExportMetalObjectCreateInfoEXT structs with exportObjectType of %s are allowed.",
                             string_VkExportMetalObjectTypeFlagBitsEXT(export_metal_object_info->exportObjectType), sType);
        }
        export_metal_object_info = vku::FindStructInPNextChain<VkExportMetalObjectCreateInfoEXT>(export_metal_object_info->pNext);
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateExportMetalObjectsEXT(VkDevice device,
                                                                      VkExportMetalObjectsInfoEXT *pMetalObjectsInfo,
                                                                      const ErrorObject &error_obj) const {
    bool skip = false;

    static_assert(AllVkExportMetalObjectTypeFlagBitsEXT == 0x3F, "Add new ExportMetalObjects support to VVL!");

    constexpr std::array allowed_structs = {
        VK_STRUCTURE_TYPE_EXPORT_METAL_BUFFER_INFO_EXT,       VK_STRUCTURE_TYPE_EXPORT_METAL_COMMAND_QUEUE_INFO_EXT,
        VK_STRUCTURE_TYPE_EXPORT_METAL_DEVICE_INFO_EXT,       VK_STRUCTURE_TYPE_EXPORT_METAL_IO_SURFACE_INFO_EXT,
        VK_STRUCTURE_TYPE_EXPORT_METAL_SHARED_EVENT_INFO_EXT, VK_STRUCTURE_TYPE_EXPORT_METAL_TEXTURE_INFO_EXT,
    };
    skip |=
        ValidateStructPnext(error_obj.location.dot(Field::pMetalObjectsInfo), pMetalObjectsInfo->pNext, allowed_structs.size(),
                            allowed_structs.data(), GeneratedVulkanHeaderVersion, "VUID-VkExportMetalObjectsInfoEXT-pNext-pNext",
                            "VUID-VkExportMetalObjectsInfoEXT-sType-unique", false, true);
    return skip;
}
#endif  // VK_USE_PLATFORM_METAL_EXT
