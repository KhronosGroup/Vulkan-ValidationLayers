#pragma once

#include "vk_typemap_helper.h"

struct ImportOperationsInfo {
    const VkImportMemoryHostPointerInfoEXT *host_pointer_info_ext;
    uint32_t total_import_ops;
};

inline ImportOperationsInfo GetNumberOfImportInfo(const VkMemoryAllocateInfo *pAllocateInfo) {
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
    auto import_zircon_fushcia = LvlFindInChain<VkImportMemoryZirconHandleInfoFUCHSIA>(pAllocateInfo->pNext);
    count += (import_zircon_fushcia && import_zircon_fushcia->handleType);

    // VkImportMemoryBufferCollectionFUCHSIA
    auto import_buffer_collection_fushcia = LvlFindInChain<VkImportMemoryBufferCollectionFUCHSIA>(pAllocateInfo->pNext);
    count += (import_buffer_collection_fushcia && import_buffer_collection_fushcia->);
#endif

    ImportOperationsInfo info = {};
    info.total_import_ops = count;
    info.host_pointer_info_ext = host_pointer_info_ext;

    return info;
}