/* Copyright (c) 2018-2019 The Khronos Group Inc.
 * Copyright (c) 2018-2019 Valve Corporation
 * Copyright (c) 2018-2019 LunarG, Inc.
 * Copyright (C) 2018-2019 Google Inc.
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
 */

#ifndef VULKAN_GPU_VALIDATION_H
#define VULKAN_GPU_VALIDATION_H

// Class to encapsulate Vulkan Device Memory allocations.
// It allocates device memory in large chunks for efficiency and to avoid
// hitting the device limit of the number of allocations.
// This manager handles only fixed-sized blocks of "data_size" bytes.
// The interface allows the caller to "get" and "put back" blocks.
// The manager allocates and frees chunks as needed.

class CoreChecks;
typedef CoreChecks layer_data;

class GpuDeviceMemoryManager {
   public:
    GpuDeviceMemoryManager(layer_data *dev_data, uint32_t data_size);
    ~GpuDeviceMemoryManager();

    uint32_t GetBlockSize() { return block_size_; }

    VkResult GetBlock(GpuDeviceMemoryBlock *block);
    void PutBackBlock(VkBuffer buffer, VkDeviceMemory memory, uint32_t offset);
    void PutBackBlock(GpuDeviceMemoryBlock &block);
    void FreeAllBlocks();

   private:
    // Define allocation granularity of Vulkan resources.
    // Things like device memory and descriptors are allocated in "chunks".
    // This number should be chosen to try to avoid too many chunk allocations
    // and chunk allocations that are too large.
    static const uint32_t kItemsPerChunk = 512;

    struct MemoryChunk {
        VkBuffer buffer;
        VkDeviceMemory memory;
        std::vector<uint32_t> available_offsets;
    };

    layer_data *dev_data_;
    uint32_t record_size_;
    uint32_t block_size_;
    uint32_t blocks_per_chunk_;
    uint32_t chunk_size_;
    std::list<MemoryChunk> chunk_list_;

    bool MemoryTypeFromProperties(uint32_t typeBits, VkFlags requirements_mask, uint32_t *typeIndex);
    VkResult AllocMemoryChunk(MemoryChunk &chunk);
    void FreeMemoryChunk(MemoryChunk &chunk);
};

// Class to encapsulate Descriptor Set allocation.  This manager creates and destroys Descriptor Pools
// as needed to satisfy requests for descriptor sets.
class GpuDescriptorSetManager {
   public:
    GpuDescriptorSetManager(layer_data *dev_data);
    ~GpuDescriptorSetManager();

    VkResult GetDescriptorSets(uint32_t count, VkDescriptorPool *pool, std::vector<VkDescriptorSet> *desc_sets);
    void PutBackDescriptorSet(VkDescriptorPool desc_pool, VkDescriptorSet desc_set);
    void DestroyDescriptorPools();

   private:
    static const uint32_t kItemsPerChunk = 512;
    struct PoolTracker {
        uint32_t size;
        uint32_t used;
    };

    layer_data *dev_data_;
    std::unordered_map<VkDescriptorPool, struct PoolTracker> desc_pool_map_;
};

using mutex_t = std::mutex;
using lock_guard_t = std::lock_guard<mutex_t>;
using unique_lock_t = std::unique_lock<mutex_t>;

#endif  // VULKAN_GPU_VALIDATION_H
