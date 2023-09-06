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
#include "containers/range_vector.h"
#include "generated/vk_safe_struct.h"

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
    const VkExternalMemoryHandleTypeFlags export_handle_types;  // from VkExportMemoryAllocateInfo::handleTypes
    const std::optional<VkExternalMemoryHandleTypeFlagBits> import_handle_type;
    const bool unprotected;     // can't be used for protected memory
    const bool multi_instance;  // Allocated from MULTI_INSTANCE heap or having more than one deviceMask bit set
    const std::optional<DedicatedBinding> dedicated;

    MemRange mapped_range;
#ifdef VK_USE_PLATFORM_METAL_EXT
    const bool metal_buffer_export;        // Can be used in a VkExportMetalBufferInfoEXT struct in a VkExportMetalObjectsEXT call
#endif                                     // VK_USE_PLATFORM_METAL_EXT
    void *p_driver_data;                   // Pointer to application's actual memory
    const VkDeviceSize fake_base_address;  // To allow a unified view of allocations, useful to Synchronization Validation

    DEVICE_MEMORY_STATE(VkDeviceMemory memory, const VkMemoryAllocateInfo *p_alloc_info, uint64_t fake_address,
                        const VkMemoryType &memory_type, const VkMemoryHeap &memory_heap,
                        std::optional<DedicatedBinding> &&dedicated_binding, uint32_t physical_device_count);

    bool IsImport() const { return import_handle_type.has_value(); }
    bool IsImportAHB() const {
        return IsImport() && import_handle_type == VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID;
    }
    bool IsExport() const { return export_handle_types != 0; }

    bool IsDedicatedBuffer() const { return dedicated && dedicated->handle.type == kVulkanObjectTypeBuffer; }

    bool IsDedicatedImage() const { return dedicated && dedicated->handle.type == kVulkanObjectTypeImage; }

    VkDeviceMemory deviceMemory() const { return handle_.Cast<VkDeviceMemory>(); }
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
    using MemoryRange = sparse_container::range<VkDeviceSize>;
    using BoundMemoryRange = std::map<VkDeviceMemory, std::vector<MemoryRange>>;
    using DeviceMemoryState = vvl::unordered_set<std::shared_ptr<DEVICE_MEMORY_STATE>>;

    virtual ~BindableMemoryTracker() {}
    // kept for backwards compatibility, only useful with the Linear tracker
    virtual const MEM_BINDING *Binding() const = 0;
    virtual unsigned CountDeviceMemory(VkDeviceMemory memory) const = 0;
    virtual bool HasFullRangeBound() const = 0;

    virtual void BindMemory(BASE_NODE *, std::shared_ptr<DEVICE_MEMORY_STATE> &, VkDeviceSize, VkDeviceSize, VkDeviceSize) = 0;

    virtual BoundMemoryRange GetBoundMemoryRange(const MemoryRange &) const = 0;
    virtual DeviceMemoryState GetBoundMemoryStates() const = 0;
};

// Dummy memory tracker for swapchains
class BindableNoMemoryTracker : public BindableMemoryTracker {
  public:
    BindableNoMemoryTracker(const VkMemoryRequirements *) {}

    const MEM_BINDING *Binding() const override { return nullptr; }

    unsigned CountDeviceMemory(VkDeviceMemory memory) const override { return 0; }

    bool HasFullRangeBound() const override { return true; }

    void BindMemory(BASE_NODE *, std::shared_ptr<DEVICE_MEMORY_STATE> &, VkDeviceSize, VkDeviceSize, VkDeviceSize) override {}

    BoundMemoryRange GetBoundMemoryRange(const MemoryRange &) const override { return BoundMemoryRange{}; }
    DeviceMemoryState GetBoundMemoryStates() const override { return DeviceMemoryState{}; }
};

// Non sparse bindable memory tracker
class BindableLinearMemoryTracker : public BindableMemoryTracker {
  public:
    BindableLinearMemoryTracker(const VkMemoryRequirements *) {}

    const MEM_BINDING *Binding() const override { return binding_.memory_state ? &binding_ : nullptr; }
    unsigned CountDeviceMemory(VkDeviceMemory memory) const override {
        return binding_.memory_state && binding_.memory_state->deviceMemory() == memory ? 1 : 0;
    }

    bool HasFullRangeBound() const override { return binding_.memory_state != nullptr; }

    void BindMemory(BASE_NODE *parent, std::shared_ptr<DEVICE_MEMORY_STATE> &mem_state, VkDeviceSize memory_offset,
                    VkDeviceSize resource_offset, VkDeviceSize size) override;

    BoundMemoryRange GetBoundMemoryRange(const MemoryRange &range) const override;
    DeviceMemoryState GetBoundMemoryStates() const override;

  private:
    MEM_BINDING binding_;
};

// Sparse bindable memory tracker
// Does not contemplate the idea of multiplanar sparse images
class BindableSparseMemoryTracker : public BindableMemoryTracker {
  public:
    BindableSparseMemoryTracker(const VkMemoryRequirements *requirements, bool is_resident)
        : resource_size_(requirements->size), is_resident_(is_resident) {}

    const MEM_BINDING *Binding() const override { return nullptr; }

    unsigned CountDeviceMemory(VkDeviceMemory memory) const override;

    bool HasFullRangeBound() const override;

    void BindMemory(BASE_NODE *parent, std::shared_ptr<DEVICE_MEMORY_STATE> &mem_state, VkDeviceSize memory_offset,
                    VkDeviceSize resource_offset, VkDeviceSize size) override;

    BoundMemoryRange GetBoundMemoryRange(const MemoryRange &range) const override;

    DeviceMemoryState GetBoundMemoryStates() const override;

  private:
    // This range map uses the range in resource space to know the size of the bound memory
    using BindingMap = sparse_container::range_map<VkDeviceSize, MEM_BINDING>;
    BindingMap binding_map_;
    mutable std::shared_mutex binding_lock_;
    VkDeviceSize resource_size_;
    bool is_resident_;
};

// Non sparse multi planar bindable memory tracker
class BindableMultiplanarMemoryTracker : public BindableMemoryTracker {
  public:
    BindableMultiplanarMemoryTracker(const VkMemoryRequirements *requirements, uint32_t num_planes);

    const MEM_BINDING *Binding() const override { return nullptr; }

    unsigned CountDeviceMemory(VkDeviceMemory memory) const override;

    bool HasFullRangeBound() const override;

    void BindMemory(BASE_NODE *parent, std::shared_ptr<DEVICE_MEMORY_STATE> &mem_state, VkDeviceSize memory_offset,
                    VkDeviceSize resource_offset, VkDeviceSize size) override;

    BoundMemoryRange GetBoundMemoryRange(const MemoryRange &range) const override;

    DeviceMemoryState GetBoundMemoryStates() const override;

  private:
    struct Plane {
        MEM_BINDING binding;
        VkDeviceSize size;
    };
    std::vector<Plane> planes_;
};

// Superclass for bindable object state (currently images, buffers and acceleration structures)
class BINDABLE : public BASE_NODE {
  public:
    template <typename Handle>
    BINDABLE(Handle h, VulkanObjectType t, bool is_sparse, bool is_unprotected, VkExternalMemoryHandleTypeFlags handle_types)
        : BASE_NODE(h, t), external_memory_handle_types(handle_types), sparse(is_sparse), unprotected(is_unprotected),
          memory_tracker_(nullptr) {}

    virtual ~BINDABLE() {
        if (!Destroyed()) {
            Destroy();
        }
    }

    void Destroy() override {
        for (auto &state : memory_tracker_->GetBoundMemoryStates()) {
            state->RemoveParent(this);
        }

        BASE_NODE::Destroy();
    }

    bool IsExternalBuffer() const {
        return ((external_memory_handle_types & VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID) != 0) ||
               ((external_memory_handle_types & VK_EXTERNAL_MEMORY_HANDLE_TYPE_SCREEN_BUFFER_BIT_QNX) != 0);
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

    bool IsMemoryBound() const {
        const auto mem_state = MemState();
        return mem_state && !mem_state->Destroyed();
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
    void BindMemory(BASE_NODE *parent, std::shared_ptr<DEVICE_MEMORY_STATE> &mem, const VkDeviceSize memory_offset,
                            const VkDeviceSize resource_offset, const VkDeviceSize mem_size) {
        memory_tracker_->BindMemory(parent, mem, memory_offset, resource_offset, mem_size);
    }

    bool HasFullRangeBound() const { return memory_tracker_->HasFullRangeBound(); }

    std::pair<VkDeviceMemory, BindableMemoryTracker::MemoryRange> GetResourceMemoryOverlap(
        const BindableMemoryTracker::MemoryRange &memory_region, const BINDABLE *other_resource,
        const BindableMemoryTracker::MemoryRange &other_memory_region) const;

    bool DoesResourceMemoryOverlap(const BindableMemoryTracker::MemoryRange &memory_region, const BINDABLE *other_resource,
                                   const BindableMemoryTracker::MemoryRange &other_memory_region) const {
        return GetResourceMemoryOverlap(memory_region, other_resource, other_memory_region).first != VK_NULL_HANDLE;
    }

    BindableMemoryTracker::BoundMemoryRange GetBoundMemoryRange(const BindableMemoryTracker::MemoryRange &range) const {
        return memory_tracker_->GetBoundMemoryRange(range);
    }

    BindableMemoryTracker::DeviceMemoryState GetBoundMemoryStates() const {
        return memory_tracker_->GetBoundMemoryStates();
    }

    // Kept for compatibility
    const MEM_BINDING *Binding() const { return memory_tracker_->Binding(); }

    unsigned CountDeviceMemory(VkDeviceMemory memory) const { return memory_tracker_->CountDeviceMemory(memory); }

  protected:
    void CacheInvalidMemory() const;

    mutable BindableMemoryTracker::DeviceMemoryState cached_invalid_memory_;

    void SetMemoryTracker(BindableMemoryTracker *tracker) { memory_tracker_ = tracker; }
  public:
    // Tracks external memory types creating resource
    const VkExternalMemoryHandleTypeFlags external_memory_handle_types;
    const bool sparse;       // Is this object being bound with sparse memory or not?
    const bool unprotected;  // can't be used for protected memory
  private:
    mutable bool need_to_recache_invalid_memory_ = false;
    BindableMemoryTracker *memory_tracker_;
};

