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

using MemoryRange = BindableMemoryTracker::MemoryRange;
using BoundMemoryRange = BindableMemoryTracker::BoundMemoryRange;
using DeviceMemoryState = BindableMemoryTracker::DeviceMemoryState;

// It is allowed to export memory into the handles of different types,
// that's why we use set of flags (VkExternalMemoryHandleTypeFlags)
static VkExternalMemoryHandleTypeFlags GetExportHandleTypes(const VkMemoryAllocateInfo *p_alloc_info) {
    auto export_info = vku::FindStructInPNextChain<VkExportMemoryAllocateInfo>(p_alloc_info->pNext);
    return export_info ? export_info->handleTypes : 0;
}

// Import works with a single handle type, that's why VkExternalMemoryHandleTypeFlagBits type is used.
// Since FlagBits-type cannot have a value of 0, we use std::optional to indicate the presense of an import operation.
static std::optional<VkExternalMemoryHandleTypeFlagBits> GetImportHandleType(const VkMemoryAllocateInfo *p_alloc_info) {
#ifdef VK_USE_PLATFORM_WIN32_KHR
    auto win32_import = vku::FindStructInPNextChain<VkImportMemoryWin32HandleInfoKHR>(p_alloc_info->pNext);
    if (win32_import) {
        return win32_import->handleType;
    }
#endif
    auto fd_import = vku::FindStructInPNextChain<VkImportMemoryFdInfoKHR>(p_alloc_info->pNext);
    if (fd_import) {
        return fd_import->handleType;
    }
    auto host_pointer_import = vku::FindStructInPNextChain<VkImportMemoryHostPointerInfoEXT>(p_alloc_info->pNext);
    if (host_pointer_import) {
        return host_pointer_import->handleType;
    }
#ifdef VK_USE_PLATFORM_ANDROID_KHR
    // AHB Import doesn't have handle in the pNext struct
    // It should be assumed that all imported AHB can only have the same, single handleType
    auto ahb_import = vku::FindStructInPNextChain<VkImportAndroidHardwareBufferInfoANDROID>(p_alloc_info->pNext);
    if ((ahb_import) && (ahb_import->buffer != nullptr)) {
        return VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID;
    }
#endif  // VK_USE_PLATFORM_ANDROID_KHR
    return std::nullopt;
}

static bool IsMultiInstance(const VkMemoryAllocateInfo *p_alloc_info, const VkMemoryHeap &memory_heap,
                            uint32_t physical_device_count) {
    auto alloc_flags = vku::FindStructInPNextChain<VkMemoryAllocateFlagsInfo>(p_alloc_info->pNext);
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
    auto export_metal_object_info = vku::FindStructInPNextChain<VkExportMetalObjectCreateInfoEXT>(info->pNext);
    while (export_metal_object_info) {
        if (export_metal_object_info->exportObjectType == VK_EXPORT_METAL_OBJECT_TYPE_METAL_BUFFER_BIT_EXT) {
            retval = true;
            break;
        }
        export_metal_object_info = vku::FindStructInPNextChain<VkExportMetalObjectCreateInfoEXT>(export_metal_object_info->pNext);
    }
    return retval;
}
#endif  // VK_USE_PLATFORM_METAL_EXT

DEVICE_MEMORY_STATE::DEVICE_MEMORY_STATE(VkDeviceMemory memory, const VkMemoryAllocateInfo *p_alloc_info, uint64_t fake_address,
                                         const VkMemoryType &memory_type, const VkMemoryHeap &memory_heap,
                                         std::optional<DedicatedBinding> &&dedicated_binding, uint32_t physical_device_count)
    : BASE_NODE(memory, kVulkanObjectTypeDeviceMemory),
      alloc_info(p_alloc_info),
      export_handle_types(GetExportHandleTypes(p_alloc_info)),
      import_handle_type(GetImportHandleType(p_alloc_info)),
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

void BindableLinearMemoryTracker::BindMemory(BASE_NODE *parent, std::shared_ptr<DEVICE_MEMORY_STATE> &mem_state,
                                             VkDeviceSize memory_offset, VkDeviceSize resource_offset, VkDeviceSize size) {
    if (!mem_state) return;

    mem_state->AddParent(parent);
    binding_ = {mem_state, memory_offset, 0u};
}

DeviceMemoryState BindableLinearMemoryTracker::GetBoundMemoryStates() const {
    return binding_.memory_state ? DeviceMemoryState{binding_.memory_state} : DeviceMemoryState{};
}

BoundMemoryRange BindableLinearMemoryTracker::GetBoundMemoryRange(const MemoryRange &range) const {
    return binding_.memory_state ? BoundMemoryRange{BoundMemoryRange::value_type{
                                       binding_.memory_state->deviceMemory(),
                                       BoundMemoryRange::value_type::second_type{
                                           {binding_.memory_offset + range.begin, binding_.memory_offset + range.end}}}}
                                 : BoundMemoryRange{};
}

unsigned BindableSparseMemoryTracker::CountDeviceMemory(VkDeviceMemory memory) const {
    unsigned count = 0u;
    auto guard = ReadLockGuard{binding_lock_};
    for (const auto &range_state : binding_map_) {
        count += (range_state.second.memory_state && range_state.second.memory_state->deviceMemory() == memory);
    }
    return count;
}

bool BindableSparseMemoryTracker::HasFullRangeBound() const {
    if (!is_resident_) {
        VkDeviceSize current_offset = 0u;
        {
            auto guard = ReadLockGuard{binding_lock_};
            for (const auto &range : binding_map_) {
                if (current_offset != range.first.begin || !range.second.memory_state || range.second.memory_state->Invalid()) {
                    return false;
                }
                current_offset = range.first.end;
            }
        }

        if (current_offset != resource_size_) return false;
    }

    return true;
}

void BindableSparseMemoryTracker::BindMemory(BASE_NODE *parent, std::shared_ptr<DEVICE_MEMORY_STATE> &mem_state, VkDeviceSize memory_offset,
                                             VkDeviceSize resource_offset, VkDeviceSize size) {
    MEM_BINDING memory_data{mem_state, memory_offset, resource_offset};
    BindingMap::value_type item{{resource_offset, resource_offset + size}, memory_data};

    auto guard = WriteLockGuard{binding_lock_};

    // Since we don't know which ranges will be removed, we need to unbind everything and rebind later
    for (auto &value_pair : binding_map_) {
        if (value_pair.second.memory_state) value_pair.second.memory_state->RemoveParent(parent);
    }
    binding_map_.overwrite_range(item);

    for (auto &value_pair : binding_map_) {
        if (value_pair.second.memory_state) value_pair.second.memory_state->AddParent(parent);
    }
}

BoundMemoryRange BindableSparseMemoryTracker::GetBoundMemoryRange(const MemoryRange &range) const {
    BoundMemoryRange mem_ranges;
    auto guard = ReadLockGuard{binding_lock_};
    auto range_bounds = binding_map_.bounds(range);

    for (auto it = range_bounds.begin; it != range_bounds.end; ++it) {
        const auto &[resource_range, memory_data] = *it;
        if (memory_data.memory_state && memory_data.memory_state->deviceMemory() != VK_NULL_HANDLE) {
            const VkDeviceSize memory_range_start = std::max(range.begin, memory_data.resource_offset) -
                memory_data.resource_offset + memory_data.memory_offset;
            const VkDeviceSize memory_range_end =
                std::min(range.end, memory_data.resource_offset + resource_range.distance()) - memory_data.resource_offset +
                memory_data.memory_offset;

            mem_ranges[memory_data.memory_state->deviceMemory()].emplace_back(memory_range_start, memory_range_end);
        }
    }
    return mem_ranges;
}

DeviceMemoryState BindableSparseMemoryTracker::GetBoundMemoryStates() const {
    DeviceMemoryState dev_mem_states;

    {
        auto guard = ReadLockGuard{binding_lock_};
        for (auto &binding : binding_map_) {
            if (binding.second.memory_state) dev_mem_states.emplace(binding.second.memory_state);
        }
    }

    return dev_mem_states;
}


BindableMultiplanarMemoryTracker::BindableMultiplanarMemoryTracker(const VkMemoryRequirements *requirements, uint32_t num_planes)
    : planes_(num_planes) {
    for (unsigned i = 0; i < num_planes; ++i) {
        planes_[i].size = requirements[i].size;
    }
}
unsigned BindableMultiplanarMemoryTracker::CountDeviceMemory(VkDeviceMemory memory) const {
    unsigned count = 0u;
    for (size_t i = 0u; i < planes_.size(); i++) {
        const auto &plane = planes_[i];
        count += (plane.binding.memory_state && plane.binding.memory_state->deviceMemory() == memory);
    }

    return count;
}

bool BindableMultiplanarMemoryTracker::HasFullRangeBound() const {
    bool full_range_bound = true;

    for (unsigned i = 0u; i < planes_.size(); ++i) {
        full_range_bound &= (planes_[i].binding.memory_state != nullptr);
    }

    return full_range_bound;
}

// resource_offset is the plane index
void BindableMultiplanarMemoryTracker::BindMemory(BASE_NODE *parent, std::shared_ptr<DEVICE_MEMORY_STATE> &mem_state,
                                                  VkDeviceSize memory_offset, VkDeviceSize resource_offset, VkDeviceSize size) {
    if (!mem_state) return;

    assert(resource_offset < planes_.size());
    mem_state->AddParent(parent);
    planes_[static_cast<size_t>(resource_offset)].binding = {mem_state, memory_offset, 0u};
}

// range needs to be between [0, planes_[0].size + planes_[1].size + planes_[2].size)
// To access plane 0 range must be [0, planes_[0].size)
// To access plane 1 range must be [planes_[0].size, planes_[1].size)
// To access plane 2 range must be [planes_[1].size, planes_[2].size)
BoundMemoryRange BindableMultiplanarMemoryTracker::GetBoundMemoryRange(const MemoryRange &range) const {
    BoundMemoryRange mem_ranges;
    VkDeviceSize start_offset = 0u;
    for (unsigned i = 0u; i < planes_.size(); ++i) {
        const auto &plane = planes_[i];
        MemoryRange plane_range{start_offset, start_offset + plane.size};
        if (plane.binding.memory_state && range.intersects(plane_range)) {
            VkDeviceSize range_end = range.end > plane_range.end ? plane_range.end : range.end;
            const auto &dev_mem = plane.binding.memory_state->deviceMemory();
            mem_ranges[dev_mem].emplace_back(MemoryRange{plane.binding.memory_offset + range.begin,
                                                                                   plane.binding.memory_offset + range_end});
        }
        start_offset += plane.size;
    }

    return mem_ranges;
}

DeviceMemoryState BindableMultiplanarMemoryTracker::GetBoundMemoryStates() const {
    DeviceMemoryState dev_mem_states;

    for (unsigned i = 0u; i < planes_.size(); ++i) {
        if (planes_[i].binding.memory_state) {
            dev_mem_states.insert(planes_[i].binding.memory_state);
        }
    }

    return dev_mem_states;
}

std::pair<VkDeviceMemory, MemoryRange> BINDABLE::GetResourceMemoryOverlap(
        const MemoryRange &memory_region, const BINDABLE *other_resource,
        const MemoryRange &other_memory_region) const {
    if (!other_resource) return {VK_NULL_HANDLE, {}};

    auto ranges = GetBoundMemoryRange(memory_region);
    auto other_ranges = other_resource->GetBoundMemoryRange(other_memory_region);

    for (const auto &[memory, memory_ranges] : ranges) {
        // Check if we have memory from same VkDeviceMemory bound
        auto it = other_ranges.find(memory);
        if (it != other_ranges.end()) {
            // Check if any of the bound memory ranges overlap
            for (const auto &memory_range : memory_ranges) {
                for (const auto &other_memory_range : it->second) {
                    if (other_memory_range.intersects(memory_range)) {
                        auto memory_space_intersection = other_memory_range & memory_range;
                        return {memory, memory_space_intersection};
                    }
                }
            }
        }
    }
    return {VK_NULL_HANDLE, {}};
}

VkDeviceSize BINDABLE::GetFakeBaseAddress() const {
    assert(!sparse);  // not implemented yet
    const auto *binding = Binding();
    return binding ? binding->memory_offset + binding->memory_state->fake_base_address : 0;
}

void BINDABLE::CacheInvalidMemory() const {
    need_to_recache_invalid_memory_ = false;
    cached_invalid_memory_.clear();
    for (auto const &bindable : GetBoundMemoryStates()) {
        if (bindable->Invalid()) {
            cached_invalid_memory_.insert(bindable);
        }
    }
}

