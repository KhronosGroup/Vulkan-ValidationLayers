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
#include <unordered_map>
#include <vector>
#include "containers/custom_containers.h"

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
    std::unique_lock<std::mutex> Lock() const { return std::unique_lock<std::mutex>(lock_); }

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
    void FlushAllocation(const Location &loc, VkDeviceSize offset = 0, VkDeviceSize size = VK_WHOLE_SIZE) const;
    void InvalidateAllocation(const Location &loc, VkDeviceSize offset = 0, VkDeviceSize size = VK_WHOLE_SIZE) const;

    [[nodiscard]] bool Create(const Location &loc, const VkBufferCreateInfo *buffer_create_info,
                              const VmaAllocationCreateInfo *allocation_create_info);
    void Destroy();

    bool IsDestroyed() const { return buffer == VK_NULL_HANDLE; }
    const VkBuffer &VkHandle() const { return buffer; }
    const VmaAllocation &Allocation() const { return allocation; }
    VkDeviceAddress Address() const { return device_address; };
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

// Register/Create and register GPU resources, all to be destroyed upon a call to DestroyResources
class GpuResourcesManager {
  public:
    explicit GpuResourcesManager(DescriptorSetManager &descriptor_set_manager) : descriptor_set_manager_(descriptor_set_manager) {}

    VkDescriptorSet GetManagedDescriptorSet(VkDescriptorSetLayout desc_set_layout);
    vko::Buffer GetManagedBuffer(Validator &gpuav, const Location &loc, const VkBufferCreateInfo &ci,
                                 const VmaAllocationCreateInfo &vma_ci);

    void ReturnResources();
    void DestroyResources();

  private:
    DescriptorSetManager &descriptor_set_manager_;
    enum class CachedStatus { Undefined, InUse, Available };
    struct CachedDescriptor {
        // VkDescriptorSetLayout desc_set_layout = VK_NULL_HANDLE;
        VkDescriptorPool desc_pool = VK_NULL_HANDLE;
        VkDescriptorSet desc_set = VK_NULL_HANDLE;
        // CachedStatus status = CachedStatus::Undefined;
    };
    struct LayoutToSets {
        VkDescriptorSetLayout desc_set_layout = VK_NULL_HANDLE;
        std::vector<CachedDescriptor> cached_descriptors;
        size_t first_available_desc_set = 0;
    };
    std::vector<LayoutToSets> cache_layouts_to_sets_;
    // std::vector<CachedDescriptor> cached_descriptors_;

    struct CachedBuffer {
        VkBufferCreateInfo buffer_ci{};
        VmaAllocationCreateInfo allocation_ci{};
        vko::Buffer buffer;

        CachedStatus status = CachedStatus::Undefined;
    };
    std::vector<CachedBuffer> cached_buffers_;
};

// Cache a single object of type T. Key is *only* based on typeid(T)
class SharedResourcesCache {
  public:
    template <typename T>
    T *TryGet() {
        auto entry = shared_validation_resources_map_.find(typeid(T));
        if (entry == shared_validation_resources_map_.cend()) {
            return nullptr;
        }
        T *t = reinterpret_cast<T *>(entry->second.first);
        return t;
    }

    // First call to Get<T> will create the object, subsequent calls will retrieve the cached entry.
    // /!\ The cache key is only based on the type T, not on the passed parameters
    // => Successive calls to Get<T> with different parameters will NOT give different objects,
    // only the entry cached upon the first call to Get<T> will be retrieved
    template <typename T, class... ConstructorTypes>
    T &Get(ConstructorTypes &&...args) {
        T *t = TryGet<T>();
        if (t) return *t;

        auto entry =
            shared_validation_resources_map_.insert({typeid(T), {new T(std::forward<ConstructorTypes>(args)...), [](void *ptr) {
                                                                     auto obj = static_cast<T *>(ptr);
                                                                     delete obj;
                                                                 }}});
        return *static_cast<T *>(entry.first->second.first);
    }

    void Clear();

  private:
    using TypeInfoRef = std::reference_wrapper<const std::type_info>;
    struct Hasher {
        std::size_t operator()(TypeInfoRef code) const { return code.get().hash_code(); }
    };
    struct EqualTo {
        bool operator()(TypeInfoRef lhs, TypeInfoRef rhs) const { return lhs.get() == rhs.get(); }
    };

    vvl::unordered_map<TypeInfoRef, std::pair<void * /*object*/, void (*)(void *) /*object destructor*/>, Hasher, EqualTo>
        shared_validation_resources_map_;
};

}  // namespace vko

}  // namespace gpuav
