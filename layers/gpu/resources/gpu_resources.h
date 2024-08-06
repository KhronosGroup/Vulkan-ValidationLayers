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

namespace gpu {

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

struct DeviceMemoryBlock {
    VkBuffer buffer = VK_NULL_HANDLE;
    VmaAllocation allocation = VK_NULL_HANDLE;
    void Destroy(VmaAllocator allocator);
    bool IsNull() { return buffer == VK_NULL_HANDLE; }
};

class GpuResourcesManager {
  public:
    GpuResourcesManager(VmaAllocator vma_allocator, DescriptorSetManager &descriptor_set_manager)
        : vma_allocator_(vma_allocator), descriptor_set_manager_(descriptor_set_manager) {}

    VkDescriptorSet GetManagedDescriptorSet(VkDescriptorSetLayout desc_set_layout);
    void ManageDeviceMemoryBlock(gpu::DeviceMemoryBlock mem_block);

    void DestroyResources();

  private:
    VmaAllocator vma_allocator_;
    DescriptorSetManager &descriptor_set_manager_;
    std::vector<std::pair<VkDescriptorPool, VkDescriptorSet>> descriptors_;
    std::vector<gpu::DeviceMemoryBlock> mem_blocks_;
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

}  // namespace gpu
