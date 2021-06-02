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
#pragma once
#include "base_node.h"

class IMAGE_STATE;

struct MemRange {
    VkDeviceSize offset = 0;
    VkDeviceSize size = 0;
};

// Data struct for tracking memory object
struct DEVICE_MEMORY_STATE : public BASE_NODE {
    void *object;  // Dispatchable object used to create this memory (device of swapchain)
    safe_VkMemoryAllocateInfo alloc_info;
    VulkanTypedHandle dedicated_handle;
    union {
        VkBufferCreateInfo buffer;
        VkImageCreateInfo image;
    } dedicated_create_info;
    bool is_export;
    bool is_import;
    bool is_import_ahb;   // The VUID check depends on if the imported memory is for AHB
    bool unprotected;     // can't be used for protected memory
    bool multi_instance;  // Allocated from MULTI_INSTANCE heap or having more than one deviceMask bit set
    VkExternalMemoryHandleTypeFlags export_handle_type_flags;
    VkExternalMemoryHandleTypeFlags import_handle_type_flags;
    // Images for alias search
    layer_data::unordered_set<IMAGE_STATE *> bound_images;

    MemRange mapped_range;
    void *shadow_copy_base;          // Base of layer's allocation for guard band, data, and alignment space
    void *shadow_copy;               // Pointer to start of guard-band data before mapped region
    uint64_t shadow_pad_size;        // Size of the guard-band data before and after actual data. It MUST be a
                                     // multiple of limits.minMemoryMapAlignment
    void *p_driver_data;             // Pointer to application's actual memory
    VkDeviceSize fake_base_address;  // To allow a unified view of allocations, useful to Synchronization Validation

    DEVICE_MEMORY_STATE(void *disp_object, const VkDeviceMemory in_mem, const VkMemoryAllocateInfo *p_alloc_info,
                        uint64_t fake_address)
        : BASE_NODE(in_mem, kVulkanObjectTypeDeviceMemory),
          object(disp_object),
          alloc_info(p_alloc_info),
          dedicated_handle(),
          dedicated_create_info{},
          is_export(false),
          is_import(false),
          is_import_ahb(false),
          unprotected(true),
          multi_instance(false),
          export_handle_type_flags(0),
          import_handle_type_flags(0),
          mapped_range{},
          shadow_copy_base(0),
          shadow_copy(0),
          shadow_pad_size(0),
          p_driver_data(0),
          fake_base_address(fake_address){};

    bool IsDedicatedBuffer() const { return dedicated_handle.type == kVulkanObjectTypeBuffer && dedicated_handle.handle != 0; }

    bool IsDedicatedImage() const { return dedicated_handle.type == kVulkanObjectTypeImage && dedicated_handle.handle != 0; }

    VkDeviceMemory mem() const { return handle_.Cast<VkDeviceMemory>(); }

    virtual ~DEVICE_MEMORY_STATE() { Destroy(); }

    void RemoveParent(BASE_NODE *parent_node) override;

    void Destroy() override;

    const BindingsType &ObjectBindings() const { return parent_nodes_; }
};

// Generic memory binding struct to track objects bound to objects
struct MEM_BINDING {
    std::shared_ptr<DEVICE_MEMORY_STATE> mem_state;
    VkDeviceSize offset;
    VkDeviceSize size;
};

inline bool operator==(MEM_BINDING a, MEM_BINDING b) NOEXCEPT {
    return a.mem_state == b.mem_state && a.offset == b.offset && a.size == b.size;
}

namespace std {
template <>
struct hash<MEM_BINDING> {
    size_t operator()(MEM_BINDING mb) const NOEXCEPT {
        auto intermediate = hash<uint64_t>()(reinterpret_cast<uint64_t &>(mb.mem_state)) ^ hash<uint64_t>()(mb.offset);
        return intermediate ^ hash<uint64_t>()(mb.size);
    }
};
}  // namespace std

// Superclass for bindable object state (currently images, buffers and acceleration structures)
class BINDABLE : public BASE_NODE {
  public:
    using BoundMemorySet = small_unordered_set<DEVICE_MEMORY_STATE *, 1>;

    bool sparse;  // Is this object being bound with sparse memory or not?
    // Non-sparse binding data
    MEM_BINDING binding;
    // Memory requirements for this BINDABLE
    VkMemoryRequirements requirements;
    // bool to track if memory requirements were checked
    bool memory_requirements_checked;
    // Tracks external memory types creating resource
    VkExternalMemoryHandleTypeFlags external_memory_handle;
    // Sparse binding data, initially just tracking MEM_BINDING per mem object
    //  There's more data for sparse bindings so need better long-term solution
    // TODO : Need to update solution to track all sparse binding data
    layer_data::unordered_set<MEM_BINDING> sparse_bindings;
    // True if memory will be imported/exported from/to an Android Hardware Buffer
    bool external_ahb;
    bool unprotected;  // can't be used for protected memory

    BoundMemorySet bound_memory_set_;

    template <typename Handle>
    BINDABLE(Handle h, VulkanObjectType t)
        : BASE_NODE(h, t),
          sparse(false),
          binding{},
          requirements{},
          memory_requirements_checked(false),
          external_memory_handle(0),
          sparse_bindings{},
          external_ahb(false),
          unprotected(true),
          bound_memory_set_{} {};

    virtual ~BINDABLE() {
        if (!Destroyed()) {
            Destroy();
        }
    }

    void Destroy() override;

    // Update the cached set of memory bindings.
    // Code that changes binding.mem or sparse_bindings must call UpdateBoundMemorySet()
    void UpdateBoundMemorySet() {
        bound_memory_set_.clear();
        if (!sparse) {
            if (binding.mem_state) bound_memory_set_.insert(binding.mem_state.get());
        } else {
            for (const auto &sb : sparse_bindings) {
                bound_memory_set_.insert(sb.mem_state.get());
            }
        }
    }

    // Return unordered set of memory objects that are bound
    // Instead of creating a set from scratch each query, return the cached one
    const BoundMemorySet &GetBoundMemory() const { return bound_memory_set_; }

    void SetMemBinding(std::shared_ptr<DEVICE_MEMORY_STATE> &mem, VkDeviceSize memory_offset);
    void SetSparseMemBinding(std::shared_ptr<DEVICE_MEMORY_STATE> &mem, const VkDeviceSize mem_offset, const VkDeviceSize mem_size);
};
