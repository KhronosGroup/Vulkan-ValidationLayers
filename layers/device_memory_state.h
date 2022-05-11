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
#pragma once
#include "base_node.h"
#include "range_vector.h"

#include <unordered_set>

class IMAGE_STATE;

struct MemRange {
    VkDeviceSize offset = 0;
    VkDeviceSize size = 0;
};

struct DedicatedBinding {
    VulkanTypedHandle handle;
    union CreateInfo {
        CreateInfo(const VkBufferCreateInfo &b) : buffer(b) {}
        CreateInfo(const VkImageCreateInfo &i) : image(i) {}
        VkBufferCreateInfo buffer;
        VkImageCreateInfo image;
    } create_info;

    DedicatedBinding(VkBuffer buffer, const VkBufferCreateInfo &buffer_create_info)
        : handle(buffer, kVulkanObjectTypeBuffer), create_info(buffer_create_info) {}

    DedicatedBinding(VkImage image, const VkImageCreateInfo &image_create_info)
        : handle(image, kVulkanObjectTypeImage), create_info(image_create_info) {}
};

// Data struct for tracking memory object
class DEVICE_MEMORY_STATE : public BASE_NODE {
  public:
    const safe_VkMemoryAllocateInfo alloc_info;
    const VkExternalMemoryHandleTypeFlags export_handle_type_flags;
    const VkExternalMemoryHandleTypeFlags import_handle_type_flags;
    const bool unprotected;     // can't be used for protected memory
    const bool multi_instance;  // Allocated from MULTI_INSTANCE heap or having more than one deviceMask bit set
    const layer_data::optional<DedicatedBinding> dedicated;

    MemRange mapped_range;
    void *p_driver_data;             // Pointer to application's actual memory
    const VkDeviceSize fake_base_address;  // To allow a unified view of allocations, useful to Synchronization Validation

    DEVICE_MEMORY_STATE(VkDeviceMemory mem, const VkMemoryAllocateInfo *p_alloc_info, uint64_t fake_address,
                        const VkMemoryType &memory_type, const VkMemoryHeap &memory_heap,
                        layer_data::optional<DedicatedBinding> &&dedicated_binding, uint32_t physical_device_count);

    bool IsImport() const { return import_handle_type_flags != 0; }
    bool IsImportAHB() const {
        return (import_handle_type_flags & VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID) != 0;
    }
    bool IsExport() const { return export_handle_type_flags != 0; }

    bool IsDedicatedBuffer() const { return dedicated && dedicated->handle.type == kVulkanObjectTypeBuffer; }

    bool IsDedicatedImage() const { return dedicated && dedicated->handle.type == kVulkanObjectTypeImage; }

    VkDeviceMemory mem() const { return handle_.Cast<VkDeviceMemory>(); }
};

// Generic memory binding struct to track objects bound to objects
struct MEM_BINDING {
    MEM_BINDING() = default;
    MEM_BINDING(VkDeviceMemory mem, std::shared_ptr<DEVICE_MEMORY_STATE> mem_state, VkDeviceSize mem_offset,
                VkDeviceSize res_offset)
        : device_memory(mem), mem_state(mem_state), memory_offset(mem_offset), resource_offset(res_offset) {}

    VkDeviceMemory device_memory = VK_NULL_HANDLE;
    std::shared_ptr<DEVICE_MEMORY_STATE> mem_state = nullptr;
    VkDeviceSize memory_offset = 0u;
    VkDeviceSize resource_offset = 0u;
};

// Superclass for bindable object state (currently images, buffers and acceleration structures)
class BINDABLE : public BASE_NODE {
  protected:
    using BindingMap = sparse_container::range_map<VkDeviceSize, MEM_BINDING>;
    // TODO AITOR: There's a bug form multiplanar images due to not bein handled correctly. Needs fix before merge
    BindingMap binding_map_;

    // Used to know if the resource if fully bound
    VkDeviceSize resource_size_ =
        std::numeric_limits<VkDeviceSize>::max();  // Just to make sure this is set at construction of derived classes
  public:
    // Tracks external memory types creating resource
    const VkExternalMemoryHandleTypeFlags external_memory_handle;
    const bool sparse;       // Is this object being bound with sparse memory or not?
    const bool is_sparse_resident;
    const bool unprotected;  // can't be used for protected memory

    template <typename Handle>
    BINDABLE(Handle h, VulkanObjectType t, bool is_sparse, bool is_sparse_resident, bool is_unprotected,
             VkExternalMemoryHandleTypeFlags handle_type)
        : BASE_NODE(h, t),
          binding_map_{},
          external_memory_handle(handle_type),
          sparse(is_sparse),
          is_sparse_resident(is_sparse_resident),
          unprotected(is_unprotected) {}

    virtual ~BINDABLE() {
        if (!Destroyed()) {
            Destroy();
        }
    }

    void Destroy() override { BASE_NODE::Destroy(); }

    // Return ordered set of memory objects that are bound
    // Instead of creating a set from scratch each query, return the cached one
    virtual const BindingMap &GetBoundMemory() const { return binding_map_; }

    bool IsExternalAHB() const {
        return (external_memory_handle & VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID) != 0;
    }

    // Kept for backwards compatibility. We should do a bigger rework on memory with current binding tracking
    virtual const MEM_BINDING *Binding() const = 0;

    // Added for backwards compatibility. We should do a bigger rework on memory with current binding tracking
    virtual const BindingMap &GetBindingMap() const { return GetBoundMemory(); }

    // Kept for backwards compatibility. We should do a bigger rework on memory with current binding tracking
    const DEVICE_MEMORY_STATE *MemState() const {
        const auto *binding = Binding();
        return binding ? binding->mem_state.get() : nullptr;
    }

    // Added for backwards compatibility. We should do a bigger rework on memory with current binding tracking
    virtual unsigned CountDeviceMemory(VkDeviceMemory mem) const = 0;

    // Binds a range of the resource memory [resource_offset, resource_offset + size)
    virtual void BindMemoryRange(std::shared_ptr<DEVICE_MEMORY_STATE> &mem_state, VkDeviceMemory mem, VkDeviceSize mem_offset,
                                 VkDeviceSize resource_offset, VkDeviceSize size) = 0;

    virtual bool FullRangeBound() const = 0;

    // Kept for backwards compatibility. We should do a bigger rework on memory with current binding tracking
    virtual VkDeviceSize GetFakeBaseAddress() const;
};

class BINDABLE_OLD : public BINDABLE {
  public:
    template <typename Handle>
    BINDABLE_OLD(Handle h, VulkanObjectType t, bool is_sparse, bool is_sparse_resident, bool is_unprotected,
                 VkExternalMemoryHandleTypeFlags handle_type)
        : BINDABLE(h, t, is_sparse, is_sparse_resident, is_unprotected, handle_type) {}

    virtual ~BINDABLE_OLD() {
        if (!Destroyed()) {
            Destroy();
        }
    }

    void Destroy() override;

    // Kept for backwards compatibility. We should do a bigger rework on memory with current binding tracking
    const MEM_BINDING *Binding() const override {
        return (!sparse && binding_map_.size() == 1 && binding_map_.begin()->second.mem_state) ? &(binding_map_.begin()->second)
                                                                                               : nullptr;
    }

    // Added for backwards compatibility. We should do a bigger rework on memory with current binding tracking
    unsigned CountDeviceMemory(VkDeviceMemory mem) const override;

    // Binds a range of the resource memory [resource_offset, resource_offset + size)
    void BindMemoryRange(std::shared_ptr<DEVICE_MEMORY_STATE> &mem_state, VkDeviceMemory mem, VkDeviceSize mem_offset,
                         VkDeviceSize resource_offset, VkDeviceSize size) override;

    bool FullRangeBound() const override;

    bool Invalid() const override {
        if (Destroyed()) {
            return true;
        }

        return !FullRangeBound();
    }
};

class BINDABLE_NEW : public BINDABLE {
  public:
    template <typename Handle>
    BINDABLE_NEW(Handle h, VulkanObjectType t, bool is_sparse, bool is_sparse_resident, bool is_unprotected,
                 VkExternalMemoryHandleTypeFlags handle_type)
        : BINDABLE(h, t, is_sparse, is_sparse_resident, is_unprotected, handle_type) {}

    virtual ~BINDABLE_NEW() {
        if (!Destroyed()) {
            Destroy();
        }
    }

    void Destroy() override { BINDABLE::Destroy(); }
};

class BindableMemoryTracker {
  public:
    using BoundMemoryRange = std::map<VkDeviceMemory, std::vector<sparse_container::range<VkDeviceSize>>>;
    using DeviceMemoryState = std::unordered_set<std::shared_ptr<DEVICE_MEMORY_STATE>>;
};

// Non sparse bindable memory tracker
// template <unsigned TRACKINGS_COUNT>
// class BindableMemoryTracker<TRACKINGS_COUNT, false, false> {
//  public:
//    bool is_sparse() const { return false; }
//    bool is_resident() const { return false; }
//    unsigned trackings_count() const { return TRACKINGS_COUNT; }
//
//    //----------------------------------------------------------------------------------------------------
//    // Kept for backwards compatibility
//    const MEM_BINDING *Binding() const { return bindings_[0].mem_state ? &bindings_[0] : nullptr; }
//    const DEVICE_MEMORY_STATE *MemState() const {
//        const auto *binding = Binding();
//        return binding ? binding->mem_state.get() : nullptr;
//    }
//    //----------------------------------------------------------------------------------------------------
//
//    bool FullRangeBound() const;
//
//    void BindMemoryRange(std::shared_ptr<DEVICE_MEMORY_STATE> &mem_state, VkDeviceSize mem_offset, unsigned plane_index);
//
//  private:
//    VkMemoryRequirements requirements_[TRACKINGS_COUNT] = {};
//    MEM_BINDING bindings_[TRACKINGS_COUNT];
//};

// Non sparse bindable memory tracker
class BindableLinearMemoryTracker : public BindableMemoryTracker {
  public:
    BindableLinearMemoryTracker(VkMemoryRequirements) {}

    bool is_sparse() const { return false; }
    bool is_resident() const { return false; }
    unsigned trackings_count() const { return 1; }

    //----------------------------------------------------------------------------------------------------
    // Kept for backwards compatibility
    const MEM_BINDING *Binding() const { return binding_.mem_state ? &binding_ : nullptr; }
    const DEVICE_MEMORY_STATE *MemState() const {
        const auto *binding = Binding();
        return binding ? binding->mem_state.get() : nullptr;
    }
    //----------------------------------------------------------------------------------------------------

    bool FullRangeBound() const { return binding_.device_memory != VK_NULL_HANDLE; }

    void BindMemoryRange(std::shared_ptr<DEVICE_MEMORY_STATE> &mem_state, VkDeviceSize mem_offset, VkDeviceSize resource_offset,
                         VkDeviceSize size);

    BoundMemoryRange GetBoundMemoryRange(const sparse_container::range<VkDeviceSize> &range) const;
    DeviceMemoryState GetDeviceMemoryState();
    VkDeviceMemory GetDeviceMemoryHandle() const { return binding_.device_memory; }

  private:
    // VkMemoryRequirements requirements_ = {};
    MEM_BINDING binding_;
};

// Sparse bindable memory tracker
// Does not contemplate the idea of multiplanar sparse images
template <bool IS_RESIDENT>
class BindableSparseMemoryTracker : public BindableMemoryTracker {
  public:
    BindableSparseMemoryTracker(VkMemoryRequirements req) : requirements_(req) {}

    bool is_sparse() const { return true; }
    bool is_resident() const { return IS_RESIDENT; }
    unsigned trackings_count() const { return 1; }

    //----------------------------------------------------------------------------------------------------
    // Kept for backwards compatibility
    const MEM_BINDING *Binding() const { return nullptr; }
    const DEVICE_MEMORY_STATE *MemState() const { return nullptr; }
    //----------------------------------------------------------------------------------------------------

    bool FullRangeBound() const {
        if (!IS_RESIDENT) {
            const auto &map = binding_map_;
            VkDeviceSize resource_size = requirements_.size;
            auto ranges = map.bounds(sparse_container::range<VkDeviceSize>{0, resource_size});

            VkDeviceSize current_offset = 0u;
            for (auto it = ranges.begin; it != ranges.end; ++it) {
                if (current_offset != it->first.begin || !it->second.mem_state || it->second.mem_state->Invalid()) {
                    return false;
                }
                current_offset = it->first.end;
            }

            if (current_offset != resource_size) return false;
        }

        return true;
    }

    void BindMemoryRange(std::shared_ptr<DEVICE_MEMORY_STATE> &mem_state, VkDeviceSize mem_offset, VkDeviceSize resource_offset,
                         VkDeviceSize size) {
        VkDeviceMemory mem = mem_state ? mem_state->mem() : VK_NULL_HANDLE;
        MEM_BINDING memory_data{mem, mem_state, mem_offset, resource_offset};
        BindingMap::value_type item{{resource_offset, resource_offset + size}, memory_data};
        binding_map_.overwrite_range(item);
    }

    BoundMemoryRange GetBoundMemoryRange(const sparse_container::range<VkDeviceSize> &range) const {
        auto range_bounds = binding_map_.bounds(range);

        BoundMemoryRange mem_ranges;
        for (auto it = range_bounds.begin; it != range_bounds.end; ++it) {
            const auto &binding = *it;
            if (binding.second.device_memory != VK_NULL_HANDLE) {
                VkDeviceSize range_start = binding.first.begin - binding.second.resource_offset;
                VkDeviceSize range_end = binding.first.end - binding.second.resource_offset;
                range_start += binding.second.memory_offset;
                range_end += binding.second.memory_offset;
                mem_ranges[binding.second.device_memory].emplace_back(range_start, range_end);
            }
        }

        return mem_ranges;
    }

    DeviceMemoryState GetDeviceMemoryState() {
        DeviceMemoryState dev_mem_states;

        for (auto &binding : binding_map_) {
            if (binding.second.mem_state) dev_mem_states.emplace(binding.second.mem_state);
        }

        return dev_mem_states;
    }
    VkDeviceMemory GetDeviceMemoryHandle() const { return VK_NULL_HANDLE; }

  private:
    using BindingMap = sparse_container::range_map<VkDeviceSize, MEM_BINDING>;
    VkMemoryRequirements requirements_ = {};
    BindingMap binding_map_;

    template <typename BindableMemoryTrackerType>
    friend class BUFFER_STATE_FINAL;
};
