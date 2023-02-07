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
#pragma once
#include "state_tracker/base_node.h"
#include "range_vector.h"
#include "vk_safe_struct.h"

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
    const std::optional<DedicatedBinding> dedicated;

    MemRange mapped_range;
#ifdef VK_USE_PLATFORM_METAL_EXT
    const bool metal_buffer_export;        // Can be used in a VkExportMetalBufferInfoEXT struct in a VkExportMetalObjectsEXT call
#endif                                     // VK_USE_PLATFORM_METAL_EXT
    void *p_driver_data;                   // Pointer to application's actual memory
    const VkDeviceSize fake_base_address;  // To allow a unified view of allocations, useful to Synchronization Validation

    DEVICE_MEMORY_STATE(VkDeviceMemory mem, const VkMemoryAllocateInfo *p_alloc_info, uint64_t fake_address,
                        const VkMemoryType &memory_type, const VkMemoryHeap &memory_heap,
                        std::optional<DedicatedBinding> &&dedicated_binding, uint32_t physical_device_count);

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
// No size needed since it will either be the size of the resource if not sparse;
// the size of the plane if multiplanar; and if sparse, the size of the bound range
// will be stored in the range_map
// We need the resource_offset and memory_offset to be able to transform from
// resource space (in which the range is) to memory space
struct MEM_BINDING {
    std::shared_ptr<DEVICE_MEMORY_STATE> memory_state;
    VkDeviceSize memory_offset;
    VkDeviceSize resource_offset;
};

class BindableMemoryTracker {
  public:
    using BoundMemoryRange = std::map<VkDeviceMemory, std::vector<sparse_container::range<VkDeviceSize>>>;
    using DeviceMemoryState = vvl::unordered_set<std::shared_ptr<DEVICE_MEMORY_STATE>>;
};

// Dummy memory tracker for swapchains
class BindableNoMemoryTracker : public BindableMemoryTracker {
  public:
    BindableNoMemoryTracker(const VkMemoryRequirements *) {}

    bool IsSparse() const { return false; }
    bool IsResident() const { return false; }
    unsigned TrackingsCount() const { return 0; }

    //----------------------------------------------------------------------------------------------------
    // Kept for backwards compatibility
    const MEM_BINDING *Binding() const { return nullptr; }
    unsigned CountDeviceMemory(VkDeviceMemory memory) const { return 0; }
    //----------------------------------------------------------------------------------------------------

    bool HasFullRangeBound() const { return true; }

    void BindMemory(BASE_NODE *, std::shared_ptr<DEVICE_MEMORY_STATE> &, VkDeviceSize, VkDeviceSize, VkDeviceSize) {}

    BoundMemoryRange GetBoundMemoryRange(const sparse_container::range<VkDeviceSize> &) const { return BoundMemoryRange{}; }
    DeviceMemoryState GetBoundMemoryStates() const { return DeviceMemoryState{}; }
};

// Non sparse bindable memory tracker
class BindableLinearMemoryTracker : public BindableMemoryTracker {
  public:
    BindableLinearMemoryTracker(const VkMemoryRequirements *) {}

    bool IsSparse() const { return false; }
    bool IsResident() const { return false; }
    unsigned TrackingsCount() const { return 1; }

    //----------------------------------------------------------------------------------------------------
    // Kept for backwards compatibility
    const MEM_BINDING *Binding() const { return binding_.memory_state ? &binding_ : nullptr; }
    unsigned CountDeviceMemory(VkDeviceMemory memory) const {
        return binding_.memory_state && binding_.memory_state->mem() == memory ? 1 : 0;
    }
    //----------------------------------------------------------------------------------------------------

    bool HasFullRangeBound() const { return binding_.memory_state != nullptr; }

    void BindMemory(BASE_NODE *parent, std::shared_ptr<DEVICE_MEMORY_STATE> &mem_state, VkDeviceSize memory_offset,
                    VkDeviceSize resource_offset, VkDeviceSize size);

    BoundMemoryRange GetBoundMemoryRange(const sparse_container::range<VkDeviceSize> &range) const;
    DeviceMemoryState GetBoundMemoryStates() const;

  private:
    MEM_BINDING binding_;
};

// Sparse bindable memory tracker
// Does not contemplate the idea of multiplanar sparse images
template <bool IS_RESIDENT>
class BindableSparseMemoryTracker : public BindableMemoryTracker {
  public:
    BindableSparseMemoryTracker(const VkMemoryRequirements *requirements) : resource_size_(requirements->size) {}

    bool IsSparse() const { return true; }
    bool IsResident() const { return IS_RESIDENT; }
    unsigned TrackingsCount() const { return 1; }

    //----------------------------------------------------------------------------------------------------
    // Kept for backwards compatibility
    const MEM_BINDING *Binding() const { return nullptr; }
    unsigned CountDeviceMemory(VkDeviceMemory memory) const {
        unsigned count = 0u;
        {
            auto guard = ReadLockGuard{binding_lock_};
            for (const auto &range_state : binding_map_) {
                count += (range_state.second.memory_state && range_state.second.memory_state->mem() == memory);
            }
        }

        return count;
    }
    //----------------------------------------------------------------------------------------------------

    bool HasFullRangeBound() const {
        if (!IS_RESIDENT) {
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

    void BindMemory(BASE_NODE *parent, std::shared_ptr<DEVICE_MEMORY_STATE> &mem_state, VkDeviceSize memory_offset,
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

    BoundMemoryRange GetBoundMemoryRange(const sparse_container::range<VkDeviceSize> &range) const {
        BoundMemoryRange mem_ranges;
        {
            auto guard = ReadLockGuard{binding_lock_};
            auto range_bounds = binding_map_.bounds(range);

            for (auto it = range_bounds.begin; it != range_bounds.end; ++it) {
                const auto &binding = *it;
                if (binding.second.memory_state && binding.second.memory_state->mem() != VK_NULL_HANDLE) {
                    VkDeviceSize range_start = binding.first.begin - binding.second.resource_offset;
                    VkDeviceSize range_end = binding.first.end - binding.second.resource_offset;
                    range_start += binding.second.memory_offset;
                    range_end += binding.second.memory_offset;
                    mem_ranges[binding.second.memory_state->mem()].emplace_back(range_start, range_end);
                }
            }
        }
        return mem_ranges;
    }

    DeviceMemoryState GetBoundMemoryStates() const {
        DeviceMemoryState dev_mem_states;

        {
            auto guard = ReadLockGuard{binding_lock_};
            for (auto &binding : binding_map_) {
                if (binding.second.memory_state) dev_mem_states.emplace(binding.second.memory_state);
            }
        }

        return dev_mem_states;
    }

  private:
    // This range map uses the range in resource space to know the size of the bound memory
    using BindingMap = sparse_container::range_map<VkDeviceSize, MEM_BINDING>;
    BindingMap binding_map_;
    VkDeviceSize resource_size_;
    mutable std::shared_mutex binding_lock_;
};

// Non sparse multi planar bindable memory tracker
template <unsigned TRACKING_COUNT>
class BindableMultiplanarMemoryTracker : public BindableMemoryTracker {
  public:
    BindableMultiplanarMemoryTracker(const VkMemoryRequirements *requirements) {
        for (unsigned i = 0; i < TRACKING_COUNT; ++i) {
            plane_size_[i] = requirements[i].size;
        }
    }

    bool IsSparse() const { return false; }
    bool IsResident() const { return false; }
    unsigned TrackingsCount() const { return TRACKING_COUNT; }

    //----------------------------------------------------------------------------------------------------
    // Kept for backwards compatibility
    const MEM_BINDING *Binding() const { return nullptr; }
    unsigned CountDeviceMemory(VkDeviceMemory memory) const {
        unsigned count = 0u;
        for (unsigned i = 0u; i < TRACKING_COUNT; ++i) {
            count += (bindings_[i].memory_state && bindings_[i].memory_state->mem() == memory);
        }

        return count;
    }
    //----------------------------------------------------------------------------------------------------

    bool HasFullRangeBound() const {
        bool full_range_bound = true;

        for (unsigned i = 0u; i < TRACKING_COUNT; ++i) {
            full_range_bound &= (bindings_[i].memory_state != nullptr);
        }

        return full_range_bound;
    }

    // resource_offset is the plane index
    void BindMemory(BASE_NODE *parent, std::shared_ptr<DEVICE_MEMORY_STATE> &mem_state, VkDeviceSize memory_offset,
                    VkDeviceSize resource_offset, VkDeviceSize size) {
        if (!mem_state) return;

        mem_state->AddParent(parent);
        bindings_[resource_offset] = {mem_state, memory_offset, 0u};
    }

    // range needs to be between [0, resource_size_[0] + resource_size_[1] + resource_size_[2])
    // To access plane 0 range must be [0, plane_size_[0])
    // To access plane 1 range must be [plane_size_[0], plane_size_[1])
    // To access plane 2 range must be [plane_size_[1], plane_size_[2])
    BoundMemoryRange GetBoundMemoryRange(const sparse_container::range<VkDeviceSize> &range) const {
        BoundMemoryRange mem_ranges;
        VkDeviceSize start_offset = 0u;
        for (unsigned i = 0u; i < TRACKING_COUNT; ++i) {
            sparse_container::range<VkDeviceSize> plane_range{start_offset, start_offset + plane_size_[i]};
            if (bindings_[i].memory_state && range.intersects(plane_range)) {
                VkDeviceSize range_end = range.end > plane_range.end ? plane_range.end : range.end;
                mem_ranges[bindings_[i].memory_state->mem()].emplace_back(sparse_container::range<VkDeviceSize>{
                    bindings_[i].memory_offset + range.begin, bindings_[i].memory_offset + range_end});
            }
            start_offset += plane_size_[i];
        }

        return mem_ranges;
    }

    DeviceMemoryState GetBoundMemoryStates() const {
        DeviceMemoryState dev_mem_states;

        for (unsigned i = 0u; i < TRACKING_COUNT; ++i) {
            if (bindings_[i].memory_state) {
                dev_mem_states.insert(bindings_[i].memory_state);
            }
        }

        return dev_mem_states;
    }

  private:
    MEM_BINDING bindings_[TRACKING_COUNT];
    VkDeviceSize plane_size_[TRACKING_COUNT];
};

// Superclass for bindable object state (currently images, buffers and acceleration structures)
class BINDABLE : public BASE_NODE {
  public:
    template <typename Handle>
    BINDABLE(Handle h, VulkanObjectType t, bool is_sparse, bool is_unprotected, VkExternalMemoryHandleTypeFlags handle_type)
        : BASE_NODE(h, t), external_memory_handle(handle_type), sparse(is_sparse), unprotected(is_unprotected) {}

    virtual ~BINDABLE() {
        if (!Destroyed()) {
            Destroy();
        }
    }

    void Destroy() override { BASE_NODE::Destroy(); }

    bool IsExternalAHB() const {
        return (external_memory_handle & VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID) != 0;
    }

    const DEVICE_MEMORY_STATE *MemState() const {
        const MEM_BINDING *binding = Binding();
        return binding ? binding->memory_state.get() : nullptr;
    }

    virtual VkDeviceSize GetFakeBaseAddress() const;

    bool Invalid() const override {
        if (Destroyed()) {
            return true;
        }

        return !HasFullRangeBound();
    }
    void NotifyInvalidate(const NodeList &invalid_nodes, bool unlink) override {
        need_to_recache_invalid_memory_ = true;
        BASE_NODE::NotifyInvalidate(invalid_nodes, unlink);
    }

    const BindableMemoryTracker::DeviceMemoryState &GetInvalidMemory() const {
        if (need_to_recache_invalid_memory_) {
            CacheInvalidMemory();
        }
        return cached_invalid_memory_;
    }

    // Implemented by template class MemoryTrackedResource that has a BindableMemoryTracker with each needed function
    virtual void BindMemory(BASE_NODE *parent, std::shared_ptr<DEVICE_MEMORY_STATE> &mem, const VkDeviceSize memory_offset,
                            const VkDeviceSize resource_offset, const VkDeviceSize mem_size) = 0;
    virtual bool HasFullRangeBound() const = 0;
    virtual bool DoesResourceMemoryOverlap(const sparse_container::range<VkDeviceSize> &memory_region,
                                           const BINDABLE *other_resource,
                                           const sparse_container::range<VkDeviceSize> &other_memory_region) const = 0;
    virtual BindableMemoryTracker::BoundMemoryRange GetBoundMemoryRange(
        const sparse_container::range<VkDeviceSize> &range) const = 0;
    virtual BindableMemoryTracker::DeviceMemoryState GetBoundMemoryStates() const = 0;
    // Kept for compatibility
    virtual const MEM_BINDING *Binding() const = 0;
    virtual unsigned CountDeviceMemory(VkDeviceMemory memory) const = 0;

  protected:
    virtual void CacheInvalidMemory() const = 0;

    mutable BindableMemoryTracker::DeviceMemoryState cached_invalid_memory_;

  public:
    // Tracks external memory types creating resource
    const VkExternalMemoryHandleTypeFlags external_memory_handle;
    const bool sparse;       // Is this object being bound with sparse memory or not?
    const bool unprotected;  // can't be used for protected memory
  protected:
    mutable bool need_to_recache_invalid_memory_ = false;
};

template <typename BaseClass, typename MemoryTracker>
class MEMORY_TRACKED_RESOURCE_STATE : public BaseClass {
  public:
    template <typename... Args>
    MEMORY_TRACKED_RESOURCE_STATE(Args... args)
        : BaseClass(std::forward<Args>(args)...), memory_tracker_(BaseClass::memory_requirements_pointer) {}

    virtual ~MEMORY_TRACKED_RESOURCE_STATE() {
        if (!BaseClass::Destroyed()) Destroy();
    }

    void Destroy() override {
        for (auto &state : GetBoundMemoryStates()) {
            state->RemoveParent(this);
        }

        BaseClass::Destroy();
    }

    void BindMemory(BASE_NODE *parent, std::shared_ptr<DEVICE_MEMORY_STATE> &memory_state, const VkDeviceSize memory_offset,
                    const VkDeviceSize resource_offset, const VkDeviceSize memory_size) override {
        memory_tracker_.BindMemory(parent, memory_state, memory_offset, resource_offset, memory_size);
    }

    bool HasFullRangeBound() const override { return memory_tracker_.HasFullRangeBound(); }

    bool DoesResourceMemoryOverlap(const sparse_container::range<VkDeviceSize> &memory_region, const BINDABLE *other_resource,
                                   const sparse_container::range<VkDeviceSize> &other_memory_region) const override {
        if (!other_resource) return false;

        auto ranges = GetBoundMemoryRange(memory_region);
        auto other_ranges = other_resource->GetBoundMemoryRange(other_memory_region);

        for (const auto &value_pair : ranges) {
            // Check if we have memory from same VkDeviceMemory bound
            auto it = other_ranges.find(value_pair.first);
            if (it != other_ranges.end()) {
                // Check if any of the bound memory ranges overlap
                for (const auto &memory_range : value_pair.second) {
                    for (const auto &other_memory_range : it->second) {
                        if (other_memory_range.intersects(memory_range)) return true;
                    }
                }
            }
        }

        return false;
    }

    BindableMemoryTracker::BoundMemoryRange GetBoundMemoryRange(const sparse_container::range<VkDeviceSize> &range) const override {
        return memory_tracker_.GetBoundMemoryRange(range);
    }

    BindableMemoryTracker::DeviceMemoryState GetBoundMemoryStates() const override {
        return memory_tracker_.GetBoundMemoryStates();
    }

    const MEM_BINDING *Binding() const override { return memory_tracker_.Binding(); }

    unsigned CountDeviceMemory(VkDeviceMemory memory) const override { return memory_tracker_.CountDeviceMemory(memory); }

  protected:
    void CacheInvalidMemory() const override {
        BaseClass::need_to_recache_invalid_memory_ = false;
        BaseClass::cached_invalid_memory_.clear();
        for (auto const &bindable : GetBoundMemoryStates()) {
            if (bindable->Invalid()) {
                BaseClass::cached_invalid_memory_.insert(bindable);
            }
        }
    }

  private:
    MemoryTracker memory_tracker_;
};
