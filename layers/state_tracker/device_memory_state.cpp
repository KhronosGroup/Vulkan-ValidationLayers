/* Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
 * Copyright (C) 2015-2023 Google Inc.
 * Modifications Copyright (C) 2020 Advanced Micro Devices, Inc. All rights reserved.
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
#include "state_tracker/device_memory_state.h"
#include "state_tracker/image_state.h"

static VkExternalMemoryHandleTypeFlags GetExportHandleType(const VkMemoryAllocateInfo *p_alloc_info) {
    auto export_info = LvlFindInChain<VkExportMemoryAllocateInfo>(p_alloc_info->pNext);
    return export_info ? export_info->handleTypes : 0;
}

static VkExternalMemoryHandleTypeFlags GetImportHandleType(const VkMemoryAllocateInfo *p_alloc_info) {
#ifdef VK_USE_PLATFORM_WIN32_KHR
    auto win32_import = LvlFindInChain<VkImportMemoryWin32HandleInfoKHR>(p_alloc_info->pNext);
    if (win32_import) {
        return win32_import->handleType;
    }
#endif
    auto fd_import = LvlFindInChain<VkImportMemoryFdInfoKHR>(p_alloc_info->pNext);
    if (fd_import) {
        return fd_import->handleType;
    }
    auto host_pointer_import = LvlFindInChain<VkImportMemoryHostPointerInfoEXT>(p_alloc_info->pNext);
    if (host_pointer_import) {
        return host_pointer_import->handleType;
    }
#ifdef VK_USE_PLATFORM_ANDROID_KHR
    // AHB Import doesn't have handle in the pNext struct
    // It should be assumed that all imported AHB can only have the same, single handleType
    auto ahb_import = LvlFindInChain<VkImportAndroidHardwareBufferInfoANDROID>(p_alloc_info->pNext);
    if ((ahb_import) && (ahb_import->buffer != nullptr)) {
        return VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID;
    }
#endif  // VK_USE_PLATFORM_ANDROID_KHR
    return 0;
}

static bool IsMultiInstance(const VkMemoryAllocateInfo *p_alloc_info, const VkMemoryHeap &memory_heap,
                            uint32_t physical_device_count) {
    auto alloc_flags = LvlFindInChain<VkMemoryAllocateFlagsInfo>(p_alloc_info->pNext);
    if (alloc_flags && (alloc_flags->flags & VK_MEMORY_ALLOCATE_DEVICE_MASK_BIT)) {
        auto dev_mask = alloc_flags->deviceMask;
        return ((dev_mask != 0) && (dev_mask & (dev_mask - 1))) != 0;
    } else if (memory_heap.flags & VK_MEMORY_HEAP_MULTI_INSTANCE_BIT) {
        return physical_device_count > 1;
    }
    return false;
}

#ifdef VK_USE_PLATFORM_METAL_EXT
static bool GetMetalExport(const VkMemoryAllocateInfo *info) {
    bool retval = false;
    auto export_metal_object_info = LvlFindInChain<VkExportMetalObjectCreateInfoEXT>(info->pNext);
    while (export_metal_object_info) {
        if (export_metal_object_info->exportObjectType == VK_EXPORT_METAL_OBJECT_TYPE_METAL_BUFFER_BIT_EXT) {
            retval = true;
            break;
        }
        export_metal_object_info = LvlFindInChain<VkExportMetalObjectCreateInfoEXT>(export_metal_object_info->pNext);
    }
    return retval;
}
#endif  // VK_USE_PLATFORM_METAL_EXT

DEVICE_MEMORY_STATE::DEVICE_MEMORY_STATE(VkDeviceMemory mem, const VkMemoryAllocateInfo *p_alloc_info, uint64_t fake_address,
                                         const VkMemoryType &memory_type, const VkMemoryHeap &memory_heap,
                                         std::optional<DedicatedBinding> &&dedicated_binding, uint32_t physical_device_count)
    : BASE_NODE(mem, kVulkanObjectTypeDeviceMemory),
      alloc_info(p_alloc_info),
      export_handle_type_flags(GetExportHandleType(p_alloc_info)),
      import_handle_type_flags(GetImportHandleType(p_alloc_info)),
      unprotected((memory_type.propertyFlags & VK_MEMORY_PROPERTY_PROTECTED_BIT) == 0),
      multi_instance(IsMultiInstance(p_alloc_info, memory_heap, physical_device_count)),
      dedicated(std::move(dedicated_binding)),
      mapped_range{},
#ifdef VK_USE_PLATFORM_METAL_EXT
      metal_buffer_export(GetMetalExport(p_alloc_info)),
#endif  // VK_USE_PLATFORM_METAL_EXT
      p_driver_data(nullptr),
      fake_base_address(fake_address) {
}

VkDeviceSize BINDABLE::GetFakeBaseAddress() const {
    assert(!sparse);  // not implemented yet
    const auto *binding = Binding();
    return binding ? binding->memory_offset + binding->memory_state->fake_base_address : 0;
}

void BindableLinearMemoryTracker::BindMemory(BASE_NODE *parent, std::shared_ptr<DEVICE_MEMORY_STATE> &mem_state,
                                             VkDeviceSize memory_offset, VkDeviceSize resource_offset, VkDeviceSize size) {
    if (!mem_state) return;

    mem_state->AddParent(parent);
    binding_ = {mem_state, memory_offset, 0u};
}

BindableMemoryTracker::DeviceMemoryState BindableLinearMemoryTracker::GetBoundMemoryStates() const {
    return binding_.memory_state ? DeviceMemoryState{binding_.memory_state} : DeviceMemoryState{};
}

BindableMemoryTracker::BoundMemoryRange BindableLinearMemoryTracker::GetBoundMemoryRange(
    const sparse_container::range<VkDeviceSize> &range) const {
    return binding_.memory_state
               ? BoundMemoryRange{BoundMemoryRange::value_type{
                     binding_.memory_state->mem(), BoundMemoryRange::value_type::second_type{{binding_.memory_offset + range.begin,
                                                                                              binding_.memory_offset + range.end}}}}
               : BoundMemoryRange{};
}
