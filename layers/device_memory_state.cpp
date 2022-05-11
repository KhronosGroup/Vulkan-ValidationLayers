/* Copyright (c) 2015-2022 The Khronos Group Inc.
 * Copyright (c) 2015-2022 Valve Corporation
 * Copyright (c) 2015-2022 LunarG, Inc.
 * Copyright (C) 2015-2022 Google Inc.
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

static bool IsMultiInstance(const VkMemoryAllocateInfo *p_alloc_info, const VkMemoryHeap &memory_heap, uint32_t physical_device_count) {
    auto alloc_flags = LvlFindInChain<VkMemoryAllocateFlagsInfo>(p_alloc_info->pNext);
    if (alloc_flags && (alloc_flags->flags & VK_MEMORY_ALLOCATE_DEVICE_MASK_BIT)) {
        auto dev_mask = alloc_flags->deviceMask;
        return ((dev_mask != 0) && (dev_mask & (dev_mask - 1))) != 0;
    } else if (memory_heap.flags & VK_MEMORY_HEAP_MULTI_INSTANCE_BIT) {
        return physical_device_count > 1;
    }
    return false;
}

DEVICE_MEMORY_STATE::DEVICE_MEMORY_STATE(VkDeviceMemory mem, const VkMemoryAllocateInfo *p_alloc_info,
                                         uint64_t fake_address,
                                         const VkMemoryType &memory_type, const VkMemoryHeap &memory_heap,
                                         layer_data::optional<DedicatedBinding> &&dedicated_binding,
                                         uint32_t physical_device_count)
    : BASE_NODE(mem, kVulkanObjectTypeDeviceMemory),
      alloc_info(p_alloc_info),
      export_handle_type_flags(GetExportHandleType(p_alloc_info)),
      import_handle_type_flags(GetImportHandleType(p_alloc_info)),
      unprotected((memory_type.propertyFlags & VK_MEMORY_PROPERTY_PROTECTED_BIT) == 0),
      multi_instance(IsMultiInstance(p_alloc_info, memory_heap, physical_device_count)),
      dedicated(std::move(dedicated_binding)),
      mapped_range{},
      p_driver_data(nullptr),
      fake_base_address(fake_address) {}

void BINDABLE_OLD::Destroy() {
    for (auto &item : binding_map_) {
        if (item.second.mem_state) {
            item.second.mem_state->RemoveParent(this);
        }
    }
    binding_map_.clear();
    BASE_NODE::Destroy();
}

unsigned BINDABLE_OLD::CountDeviceMemory(VkDeviceMemory mem) const {
    unsigned count = 0u;

    for (const auto &value : binding_map_) {
        count += static_cast<unsigned>(value.second.device_memory == mem);
    }

    return count;
}

void BINDABLE_OLD::BindMemoryRange(std::shared_ptr<DEVICE_MEMORY_STATE> &mem_state, VkDeviceMemory mem, VkDeviceSize mem_offset,
                                   VkDeviceSize resource_offset, VkDeviceSize size) {
    // Since we don't know which ranges will be removed, we need to unbind everything and rebind later
    for (auto &value : binding_map_) {
        if (value.second.mem_state) {
            value.second.mem_state->RemoveParent(this);
        }
    }

    MEM_BINDING memory_data = {mem, mem_state, mem_offset, resource_offset};
    std::pair<sparse_container::range<VkDeviceSize>, MEM_BINDING> item;
    item.first = sparse_container::range<VkDeviceSize>{resource_offset, resource_offset + size};
    item.second = memory_data;
    binding_map_.overwrite_range(item);

    for (auto &value : binding_map_) {
        if (value.second.mem_state) {
            value.second.mem_state->AddParent(this);
        }
    }
}

bool BINDABLE_OLD::FullRangeBound() const {
    // If the resource is sparse, it either needs to be completely bound or it needs to be sparse resident
    if (sparse && is_sparse_resident) {
        for (const auto &binding : binding_map_) {
            if (binding.second.device_memory != VK_NULL_HANDLE && binding.second.mem_state->Invalid()) {
                return false;
            }
        }

        return true;
    }

    auto ranges = binding_map_.bounds(sparse_container::range<VkDeviceSize>{0, resource_size_});

    VkDeviceSize current_offset = 0u;
    for (auto it = ranges.begin; it != ranges.end; ++it) {
        if (current_offset != it->first.begin || !it->second.mem_state || it->second.mem_state->Invalid()) {
            return false;
        }
        current_offset = it->first.end;
    }

    return current_offset == resource_size_;
}

VkDeviceSize BINDABLE::GetFakeBaseAddress() const {
    assert(!sparse);  // not implemented yet (Can now be done)

    // Non sparse implementation
    const auto *binding = Binding();
    return binding ? binding->memory_offset + binding->mem_state->fake_base_address : 0;
}

// template <unsigned TRACKINGS_COUNT>
// bool BindableMemoryTracker<TRACKINGS_COUNT, false, false>::FullRangeBound() const {
//    bool is_full_range_bound = true;
//    switch (TRACKINGS_COUNT) {
//        case 3:
//            is_full_range_bound &= (bindings_[2].device_memory != VK_NULL_HANDLE)
//        case 2:
//            is_full_range_bound &= (bindings_[1].device_memory != VK_NULL_HANDLE)
//        case 1:
//            is_full_range_bound &= (bindings_[0].device_memory != VK_NULL_HANDLE)
//            break;
//        default:
//            assert(true); // We can only have 1-3 planes
//    }
//
//    return is_full_range_bound;
//}

// template <unsigned TRACKINGS_COUNT>
// void BindableMemoryTracker<TRACKINGS_COUNT, false, false>::BindMemoryRange(std::shared_ptr<DEVICE_MEMORY_STATE> &mem_state,
//                                                                           VkDeviceSize mem_offset, unsigned plane_index) {
//    if (!mem_state) return;
//
//    mem_state->AddParent(this);
//    bindings_[plane_index] = {mem_state->mem(), mem_state, mem_offset, 0u};
//}

void BindableLinearMemoryTracker::BindMemoryRange(std::shared_ptr<DEVICE_MEMORY_STATE> &mem_state, VkDeviceSize mem_offset,
                                                  VkDeviceSize resource_offset, VkDeviceSize size) {
    if (!mem_state) return;

    binding_ = {mem_state->mem(), mem_state, mem_offset, 0u};
}

BindableMemoryTracker::DeviceMemoryState BindableLinearMemoryTracker::GetDeviceMemoryState() {
    return binding_.mem_state ? DeviceMemoryState{binding_.mem_state} : DeviceMemoryState{};
}

BindableMemoryTracker::BoundMemoryRange BindableLinearMemoryTracker::GetBoundMemoryRange(
    const sparse_container::range<VkDeviceSize> &range) const {
    return binding_.device_memory != VK_NULL_HANDLE
               ? BoundMemoryRange{BoundMemoryRange::value_type{
                     binding_.device_memory, BoundMemoryRange::value_type::second_type{{binding_.memory_offset + range.begin,
                                                                                        binding_.memory_offset + range.end}}}}
               : BoundMemoryRange{};
}
