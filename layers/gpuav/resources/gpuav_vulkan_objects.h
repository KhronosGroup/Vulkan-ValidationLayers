/* Copyright (c) 2018-2025 The Khronos Group Inc.
 * Copyright (c) 2018-2025 Valve Corporation
 * Copyright (c) 2018-2025 LunarG, Inc.
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

#include "external/vma/vma.h"

#include <typeinfo>
#include <vector>
#include "containers/custom_containers.h"
#include "containers/range.h"

struct Location;
namespace gpuav {
class Validator;

namespace vko {

class DescriptorSetManager {
  public:
    DescriptorSetManager(VkDevice device, uint32_t num_bindings_in_set);
    ~DescriptorSetManager();

    VkResult GetDescriptorSet(VkDescriptorPool *out_desc_pool, VkDescriptorSetLayout ds_layout, VkDescriptorSet *out_desc_sets);
    VkResult GetDescriptorSets(uint32_t count, VkDescriptorPool *out_pool, VkDescriptorSetLayout ds_layout,
                               std::vector<VkDescriptorSet> *out_desc_sets);
    void PutBackDescriptorSet(VkDescriptorPool desc_pool, VkDescriptorSet desc_set);

  private:
    struct PoolTracker {
        uint32_t size;
        uint32_t used;
    };
    VkDevice device;
    uint32_t num_bindings_in_set;
    vvl::unordered_map<VkDescriptorPool, PoolTracker> desc_pool_map_;
    mutable std::mutex lock_;
};

class Buffer {
  public:
    explicit Buffer(Validator &gpuav) : gpuav(gpuav) {}

    // Warps VMA calls to simplify error reporting.
    // No error propagation, but if hitting a VMA error, GPU-AV is likely not going to recover anyway.

    [[nodiscard]] void *GetMappedPtr() const;
    void FlushAllocation(VkDeviceSize offset = 0, VkDeviceSize size = VK_WHOLE_SIZE) const;
    void InvalidateAllocation(VkDeviceSize offset = 0, VkDeviceSize size = VK_WHOLE_SIZE) const;

    [[nodiscard]] bool Create(const VkBufferCreateInfo *buffer_create_info, const VmaAllocationCreateInfo *allocation_create_info);
    void Destroy();

    bool IsDestroyed() const { return buffer == VK_NULL_HANDLE; }
    const VkBuffer &VkHandle() const { return buffer; }
    const VmaAllocation &Allocation() const { return allocation; }
    VkDeviceAddress Address() const { return device_address; }
    VkDeviceSize Size() const { return size; }
    void Clear() const;

  private:
    const Validator &gpuav;
    VkBuffer buffer = VK_NULL_HANDLE;
    VmaAllocation allocation = VK_NULL_HANDLE;
    // If buffer was not created with VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT then this will not be zero
    VkDeviceAddress device_address = 0;
    VkDeviceSize size = 0;
    void *mapped_ptr = nullptr;
};

struct BufferRange {
    VkBuffer buffer = VK_NULL_HANDLE;
    VkDeviceSize offset = 0;
    VkDeviceSize size = 0;
    void *offset_mapped_ptr = nullptr;
    VkDeviceAddress offset_address = 0;
    VmaAllocation vma_alloc = VK_NULL_HANDLE;  // Todo: get rid of this once host cached allocation are removed

    VkDescriptorBufferInfo GetDescriptorBufferInfo() const { return {buffer, offset, size}; }
    void Clear() const;
};

// Register/Create and register GPU resources, all to be destroyed upon a call to DestroyResources
class GpuResourcesManager {
  public:
    explicit GpuResourcesManager(Validator &gpuav);

    VkDescriptorSet GetManagedDescriptorSet(VkDescriptorSetLayout desc_set_layout);

    vko::BufferRange GetHostCoherentBufferRange(VkDeviceSize size);
    vko::BufferRange GetHostCachedBufferRange(VkDeviceSize size);
    void FlushAllocation(const vko::BufferRange &buffer_range);
    void InvalidateAllocation(const vko::BufferRange &buffer_range);
    vko::BufferRange GetDeviceLocalBufferRange(VkDeviceSize size);
    vko::BufferRange GetDeviceLocalIndirectBufferRange(VkDeviceSize size);
    vko::BufferRange GetStagingBufferRange(VkDeviceSize size);

    void ReturnResources();
    void DestroyResources();

    Validator &gpuav_;

  private:
    struct CachedDescriptor {
        VkDescriptorPool desc_pool = VK_NULL_HANDLE;
        VkDescriptorSet desc_set = VK_NULL_HANDLE;
    };
    struct LayoutToSets {
        VkDescriptorSetLayout desc_set_layout = VK_NULL_HANDLE;
        std::vector<CachedDescriptor> cached_descriptors;
        size_t first_available_desc_set = 0;
    };
    std::vector<LayoutToSets> cache_layouts_to_sets_;

    class BufferCache {
      public:
        BufferCache() = default;
        void Create(VkBufferUsageFlags buffer_usage_flags, const VmaAllocationCreateInfo allocation_ci);
        vko::BufferRange GetBufferRange(Validator &gpuav, VkDeviceSize byte_size, VkDeviceSize alignment,
                                        VkDeviceSize min_buffer_block_byte_size = 0);
        ~BufferCache();
        void ReturnBufferRange(const vko::BufferRange &buffer_range);
        void ReturnBuffers();
        void DestroyBuffers();

      private:
        VkBufferUsageFlags buffer_usage_flags_{};
        VmaAllocationCreateInfo allocation_ci_{};

        struct CachedBufferBlock {
            vko::Buffer buffer;
            vvl::range<VkDeviceAddress> total_range;
            vvl::range<VkDeviceAddress> used_range;
        };

        std::vector<CachedBufferBlock> cached_buffers_blocks_{};
        VkDeviceSize total_available_byte_size_ = 0;
        size_t next_avail_buffer_pos_hint_ = 0;
    };

    // One cache per buffer type: having them mixed in just one would make cache lookups worse
    struct BufferCaches {
        // Will have HOST_VISIBLE and HOST_COHERENT (may be HOST_CACHED)
        BufferCache host_coherent;
        // Will have HOST_VISIBLE and HOST_CACHED (may be HOST_COHERENT)
        BufferCache host_cached;
        // Will have DEVICE_LOCAL
        BufferCache device_local;
        // Will have DEVICE_LOCAL with VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT
        BufferCache device_local_indirect;
        // Used to transfer from HOST_VISIBLE to DEVICE_LOCAL buffers
        BufferCache staging;

        void ReturnBuffers() {
            host_coherent.ReturnBuffers();
            host_cached.ReturnBuffers();
            device_local.ReturnBuffers();
            device_local_indirect.ReturnBuffers();
            staging.ReturnBuffers();
        }

        void DestroyBuffers() {
            host_coherent.DestroyBuffers();
            host_cached.DestroyBuffers();
            device_local.DestroyBuffers();
            device_local_indirect.DestroyBuffers();
            staging.DestroyBuffers();
        }
    } buffer_caches_;
};

class StagingBuffer {
  public:
    static bool CanDeviceEverStage(Validator &gpuav);
    StagingBuffer(GpuResourcesManager &gpu_resources_manager, VkDeviceSize size, VkCommandBuffer cb);
    void CmdCopyDeviceToHost(VkCommandBuffer cb) const;
    void CmdCopyHostToDevice(VkCommandBuffer cb) const;
    const BufferRange &GetBufferRange() const { return device_buffer_range; }
    void *GetHostBufferPtr() {
        gpu_resources_manager.InvalidateAllocation(host_buffer_range);
        return host_buffer_range.offset_mapped_ptr;
    }

  private:
    BufferRange host_buffer_range = {};
    BufferRange device_buffer_range = {};
    GpuResourcesManager &gpu_resources_manager;
    VkMemoryPropertyFlags device_buffer_mem_prop_flags = {};
};

// Used to allocate and submit GPU-AV's own command buffers
class CommandPool {
  public:
    CommandPool(Validator &gpuav, uint32_t queue_family_i, const Location &loc);
    ~CommandPool();
    // Returned command buffer is ready to be used,
    // corresponding fence has been waited upon.
    std::pair<VkCommandBuffer, VkFence> GetCommandBuffer();

  private:
    Validator &gpuav_;
    VkCommandPool cmd_pool_ = VK_NULL_HANDLE;
    std::vector<VkCommandBuffer> cmd_buffers_{};
    std::vector<VkFence> fences_{};
    uint32_t cmd_buffer_ring_head_ = 0;
};

// Cache a single object of type T. Key is *only* based on typeid(T)
// Set "thread_safe" to true if the caches needs to be.
template <bool thread_safe>
class SharedResourcesCache {
  public:
    // Try get an object, returns null if not found
    template <typename T>
    T *TryGet() {
        std::unique_lock<std::mutex> lock(mtx, std::defer_lock);
        if (thread_safe) {
            lock.lock();
        }

        auto entry = shared_validation_resources_map_.find(typeid(T));
        if (entry == shared_validation_resources_map_.cend()) {
            return nullptr;
        }
        T *t = reinterpret_cast<T *>(entry->second.first);
        return t;
    }
    template <typename T>
    const T *TryGet() const {
        std::unique_lock<std::mutex> lock(mtx, std::defer_lock);
        if (thread_safe) {
            lock.lock();
        }

        auto entry = shared_validation_resources_map_.find(typeid(T));
        if (entry == shared_validation_resources_map_.cend()) {
            return nullptr;
        }
        const T *t = reinterpret_cast<const T *>(entry->second.first);
        return t;
    }

    // Get an object, assuming it has been created
    template <typename T>
    T &Get() {
        T *t = TryGet<T>();
        assert(t);
        return *t;
    }
    template <typename T>
    const T &Get() const {
        const T *t = TryGet<T>();
        assert(t);
        return *t;
    }

    // First call to GetOrCreate<T> will create the object, subsequent calls will retrieve the cached entry.
    // /!\ The cache key is only based on the type T, not on the passed parameters
    // => Successive calls to GetOrCreate<T> with different parameters will NOT give different objects,
    // only the entry cached upon the first call to Get<T> will be retrieved
    template <typename T, class... ConstructorTypes>
    T &GetOrCreate(ConstructorTypes &&...args) {
        T *t = TryGet<T>();
        if (t) {
            return *t;
        }
        std::unique_lock<std::mutex> lock(mtx, std::defer_lock);
        if (thread_safe) {
            lock.lock();
        }
        auto entry =
            shared_validation_resources_map_.insert({typeid(T), {new T(std::forward<ConstructorTypes>(args)...), [](void *ptr) {
                                                                     auto obj = static_cast<T *>(ptr);
                                                                     delete obj;
                                                                 }}});
        return *static_cast<T *>(entry.first->second.first);
    }

    void Clear() {
        for (auto &[key, value] : shared_validation_resources_map_) {
            auto &[object, destructor] = value;
            destructor(object);
        }
        shared_validation_resources_map_.clear();
    }

  private:
    using TypeInfoRef = std::reference_wrapper<const std::type_info>;
    struct Hasher {
        std::size_t operator()(TypeInfoRef code) const { return code.get().hash_code(); }
    };
    struct EqualTo {
        bool operator()(TypeInfoRef lhs, TypeInfoRef rhs) const { return lhs.get() == rhs.get(); }
    };
    mutable std::mutex mtx;
    vvl::unordered_map<TypeInfoRef, std::pair<void * /*object*/, void (*)(void *) /*object destructor*/>, Hasher, EqualTo>
        shared_validation_resources_map_;
};

}  // namespace vko

}  // namespace gpuav
