/* Copyright (c) 2015-2021 The Khronos Group Inc.
 * Copyright (c) 2015-2021 Valve Corporation
 * Copyright (c) 2015-2021 LunarG, Inc.
 * Copyright (C) 2015-2021 Google Inc.
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
 *
 * Author: Courtney Goeltzenleuchter <courtneygo@google.com>
 * Author: Tobin Ehlis <tobine@google.com>
 * Author: Chris Forbes <chrisf@ijw.co.nz>
 * Author: Mark Lobodzinski <mark@lunarg.com>
 * Author: Dave Houlton <daveh@lunarg.com>
 * Author: John Zulauf <jzulauf@lunarg.com>
 * Author: Tobias Hector <tobias.hector@amd.com>
 * Author: Jeremy Gebben <jeremyg@lunarg.com>
 */
#include "device_memory_state.h"
#include "image_state.h"

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

static bool IsMultiInstance(const VkMemoryAllocateInfo *p_alloc_info, const VkMemoryHeap &memory_heap) {
    auto alloc_flags = LvlFindInChain<VkMemoryAllocateFlagsInfo>(p_alloc_info->pNext);
    if (alloc_flags) {
        auto dev_mask = alloc_flags->deviceMask;
        return ((dev_mask != 0) && (dev_mask & (dev_mask - 1))) != 0;
    } else {
        return (memory_heap.flags & VK_MEMORY_HEAP_MULTI_INSTANCE_BIT) != 0;
    }
}

DEVICE_MEMORY_STATE::DEVICE_MEMORY_STATE(VkDeviceMemory mem, const VkMemoryAllocateInfo *p_alloc_info, uint64_t fake_address,
                                         const VkMemoryType &memory_type, const VkMemoryHeap &memory_heap,
                                         layer_data::optional<DedicatedBinding> &&dedicated_binding)
    : BASE_NODE(mem, kVulkanObjectTypeDeviceMemory),
      alloc_info(p_alloc_info),
      export_handle_type_flags(GetExportHandleType(p_alloc_info)),
      import_handle_type_flags(GetImportHandleType(p_alloc_info)),
      unprotected((memory_type.propertyFlags & VK_MEMORY_PROPERTY_PROTECTED_BIT) == 0),
      multi_instance(IsMultiInstance(p_alloc_info, memory_heap)),
      dedicated(std::move(dedicated_binding)),
      mapped_range{},
      p_driver_data(nullptr),
      fake_base_address(fake_address) {}

void BINDABLE::Destroy() {
    for (auto &item: bound_memory_) {
        if (item.second.mem_state) {
            item.second.mem_state->RemoveParent(this);
        }
    }
    bound_memory_.clear();
    BASE_NODE::Destroy();
}

// SetMemBinding is used to establish immutable, non-sparse binding between a single image/buffer object and memory object.
// Corresponding valid usage checks are in ValidateSetMemBinding().
void BINDABLE::SetMemBinding(std::shared_ptr<DEVICE_MEMORY_STATE> &mem, VkDeviceSize memory_offset) {
    if (!mem) {
        return;
    }
    assert(!sparse);
    if (bound_memory_.size() > 0) {
        bound_memory_.clear();
    }

    MEM_BINDING binding = {
        mem,
        memory_offset,
    };
    binding.mem_state->AddParent(this);
    bound_memory_.insert({mem->mem(), binding});
}

// For NULL mem case, clear any previous binding Else...
// Make sure given object is in its object map
//  IF a previous binding existed, update binding
//  Add reference from objectInfo to memoryInfo
//  Add reference off of object's binding info
// Return VK_TRUE if addition is successful, VK_FALSE otherwise
void BINDABLE::SetSparseMemBinding(std::shared_ptr<DEVICE_MEMORY_STATE> &mem, const VkDeviceSize mem_offset,
                                   const VkDeviceSize mem_size) {
    if (!mem) {
        return;
    }
    assert(sparse);
    MEM_BINDING sparse_binding = {mem, mem_offset, mem_size};
    sparse_binding.mem_state->AddParent(this);
    // Need to set mem binding for this object
    bound_memory_.insert({mem->mem(), sparse_binding});
}

VkDeviceSize BINDABLE::GetFakeBaseAddress() const {
    assert(!sparse);  // not implemented yet
    const auto *binding = Binding();
    return binding ? binding->offset + binding->mem_state->fake_base_address : 0;
}
