/* Copyright (c) 2018-2024 The Khronos Group Inc.
 * Copyright (c) 2018-2024 Valve Corporation
 * Copyright (c) 2018-2024 LunarG, Inc.
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

#include "containers/custom_containers.h"
#include "vma/vma.h"

#include <unordered_map>
#include <vector>

struct Location;
namespace gpuav {
class Validator;

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

// Simplest wrapper around device memory and the allocation
struct DeviceMemoryBlock {
    VkBuffer buffer = VK_NULL_HANDLE;
    VmaAllocation allocation = VK_NULL_HANDLE;
    bool IsNull() { return buffer == VK_NULL_HANDLE; }

    void DestroyBuffer(VmaAllocator allocator);
};

// Similar to DeviceMemoryBlock, but used for things that will require Buffer Device Address
struct AddressMemoryBlock {
    const Validator &gpuav;
    VkBuffer buffer = VK_NULL_HANDLE;
    VmaAllocation allocation = VK_NULL_HANDLE;
    VkDeviceAddress device_addr{0};

    AddressMemoryBlock(Validator &gpuav) : gpuav(gpuav) {}

    // Warps VMA calls so we can report (unlikely) errors if found while making the usages of these clean
    // (while these don't return up the chain, if we are hitting a VMA error likely not going to recover anyway)
    void MapMemory(const Location &loc, void **data) const;
    void UnmapMemory() const;
    void FlushAllocation(const Location &loc, VkDeviceSize offset = 0, VkDeviceSize size = VK_WHOLE_SIZE) const;
    void InvalidateAllocation(const Location &loc, VkDeviceSize offset = 0, VkDeviceSize size = VK_WHOLE_SIZE) const;

    void CreateBuffer(const Location &loc, const VkBufferCreateInfo *buffer_create_info,
                      const VmaAllocationCreateInfo *allocation_create_info);
    void DestroyBuffer();
};

class GpuResourcesManager {
  public:
    GpuResourcesManager(VmaAllocator vma_allocator, DescriptorSetManager &descriptor_set_manager)
        : vma_allocator_(vma_allocator), descriptor_set_manager_(descriptor_set_manager) {}

    VkDescriptorSet GetManagedDescriptorSet(VkDescriptorSetLayout desc_set_layout);
    void ManageDeviceMemoryBlock(DeviceMemoryBlock mem_block);

    void DestroyResources();

  private:
    VmaAllocator vma_allocator_;
    DescriptorSetManager &descriptor_set_manager_;
    std::vector<std::pair<VkDescriptorPool, VkDescriptorSet>> descriptors_;
    std::vector<DeviceMemoryBlock> mem_blocks_;
};

class SharedResourcesManager {
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

    // Tried to use vvl::unordered_map, but fails to compile on Windows currently
    std::unordered_map<TypeInfoRef, std::pair<void * /*object*/, void (*)(void *) /*object destructor*/>, Hasher, EqualTo>
        shared_validation_resources_map_;
};

}  // namespace gpuav
