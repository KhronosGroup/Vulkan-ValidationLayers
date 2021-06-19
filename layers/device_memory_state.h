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
#if defined(VK_ANDROID_external_memory_android_hardware_buffer)
        return (import_handle_type_flags & VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID) != 0;
#else
        return false;
#endif
    }
    bool IsExport() const { return export_handle_type_flags != 0; }

    bool IsDedicatedBuffer() const { return dedicated && dedicated->handle.type == kVulkanObjectTypeBuffer; }

    bool IsDedicatedImage() const { return dedicated && dedicated->handle.type == kVulkanObjectTypeImage; }

    VkDeviceMemory mem() const { return handle_.Cast<VkDeviceMemory>(); }

    const NodeSet &ObjectBindings() const { return parent_nodes_; }
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
  protected:
    using BoundMemoryMap = small_unordered_map<VkDeviceMemory, MEM_BINDING, 1>;
    BoundMemoryMap bound_memory_;

  public:
    // Tracks external memory types creating resource
    const VkExternalMemoryHandleTypeFlags external_memory_handle;
    const bool sparse;       // Is this object being bound with sparse memory or not?
    const bool unprotected;  // can't be used for protected memory

    template <typename Handle>
    BINDABLE(Handle h, VulkanObjectType t, bool is_sparse, bool is_unprotected, VkExternalMemoryHandleTypeFlags handle_type)
        : BASE_NODE(h, t),
          bound_memory_{},
          external_memory_handle(handle_type),
          sparse(is_sparse),
          unprotected(is_unprotected) {}

    virtual ~BINDABLE() {
        if (!Destroyed()) {
            Destroy();
        }
    }

    void Destroy() override;

    // Return unordered set of memory objects that are bound
    // Instead of creating a set from scratch each query, return the cached one
    const BoundMemoryMap &GetBoundMemory() const { return bound_memory_; }

    const MEM_BINDING *Binding() const {
        return (!sparse && bound_memory_.size() == 1) ? &(bound_memory_.begin()->second) : nullptr;
    }

    const DEVICE_MEMORY_STATE *MemState() const { return Binding() ? Binding()->mem_state.get() : nullptr; }

    virtual void SetMemBinding(std::shared_ptr<DEVICE_MEMORY_STATE> &mem, VkDeviceSize memory_offset);
    void SetSparseMemBinding(std::shared_ptr<DEVICE_MEMORY_STATE> &mem, const VkDeviceSize mem_offset, const VkDeviceSize mem_size);

    bool IsExternalAHB() const {
#if defined(VK_ANDROID_external_memory_android_hardware_buffer)
        return (external_memory_handle & VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID) != 0;
#else
        return false;
#endif
    }

    virtual VkDeviceSize GetFakeBaseAddress() const;
};
