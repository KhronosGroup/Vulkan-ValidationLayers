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

class GpuDeviceMemoryManager {
   public:
    GpuDeviceMemoryManager(layer_data *dev_data, uint32_t data_size) {
        uint32_t align = static_cast<uint32_t>(GetPDProperties(dev_data)->limits.minStorageBufferOffsetAlignment);
        if (0 == align) {
            align = 1;
        }
        record_size_ = data_size;
        // Round the requested size up to the next multiple of the storage buffer offset alignment
        // so that we can address each block in the storage buffer using the offset.
        block_size_ = ((record_size_ + align - 1) / align) * align;
        blocks_per_chunk_ = kItemsPerChunk;
        chunk_size_ = blocks_per_chunk_ * block_size_;
        dev_data_ = dev_data;
    }

    ~GpuDeviceMemoryManager() {
        for (auto &chunk : chunk_list_) {
            FreeMemoryChunk(chunk);
        }
        chunk_list_.clear();
    }

    uint32_t GetBlockSize() { return block_size_; }

    VkResult GetBlock(GpuDeviceMemoryBlock *block);
    void PutBackBlock(VkBuffer buffer, VkDeviceMemory memory, uint32_t offset);
    void PutBackBlock(GpuDeviceMemoryBlock &block);

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
    GpuDescriptorSetManager(layer_data *dev_data) { dev_data_ = dev_data; }

    ~GpuDescriptorSetManager() {
        for (auto &pool : desc_pool_map_) {
            GetDispatchTable(dev_data_)->DestroyDescriptorPool(GetDevice(dev_data_), pool.first, NULL);
        }
        desc_pool_map_.clear();
    }

    VkResult GetDescriptorSets(uint32_t count, VkDescriptorPool *pool, std::vector<VkDescriptorSet> *desc_sets);
    void PutBackDescriptorSet(VkDescriptorPool desc_pool, VkDescriptorSet desc_set);

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

std::unique_ptr<safe_VkDeviceCreateInfo> GpuPreCallRecordCreateDevice(VkPhysicalDevice gpu, const VkDeviceCreateInfo *create_info,
                                                                      VkPhysicalDeviceFeatures *supported_features);
void GpuPostCallRecordCreateDevice(layer_data *dev_data);
void GpuPreCallRecordDestroyDevice(layer_data *dev_data);
void GpuPreCallRecordFreeCommandBuffers(layer_data *dev_data, uint32_t commandBufferCount, const VkCommandBuffer *pCommandBuffers);
VkResult GpuOverrideDispatchCreateShaderModule(layer_data *dev_data, const VkShaderModuleCreateInfo *pCreateInfo,
                                               const VkAllocationCallbacks *pAllocator, VkShaderModule *pShaderModule,
                                               uint32_t *unique_shader_id);
VkResult GpuOverrideDispatchCreatePipelineLayout(layer_data *dev_data, const VkPipelineLayoutCreateInfo *pCreateInfo,
                                                 const VkAllocationCallbacks *pAllocator, VkPipelineLayout *pPipelineLayout);
void GpuPostCallQueueSubmit(layer_data *dev_data, VkQueue queue, uint32_t submitCount, const VkSubmitInfo *pSubmits, VkFence fence);
void GpuPreCallValidateCmdWaitEvents(layer_data *dev_data, VkPipelineStageFlags sourceStageMask);
std::vector<safe_VkGraphicsPipelineCreateInfo> GpuPreCallRecordCreateGraphicsPipelines(
    layer_data *dev_data, VkPipelineCache pipelineCache, uint32_t count, const VkGraphicsPipelineCreateInfo *pCreateInfos,
    const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines, std::vector<std::unique_ptr<PIPELINE_STATE>> &pipe_state);
void GpuPostCallRecordCreateGraphicsPipelines(layer_data *dev_data, const uint32_t count,
                                              const VkGraphicsPipelineCreateInfo *pCreateInfos,
                                              const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines);
void GpuPreCallRecordDestroyPipeline(layer_data *dev_data, const VkPipeline pipeline);
void GpuAllocateValidationResources(layer_data *dev_data, const VkCommandBuffer cmd_buffer, VkPipelineBindPoint bind_point);

#endif  // VULKAN_GPU_VALIDATION_H
