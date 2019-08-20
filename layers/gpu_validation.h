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

#include "vk_mem_alloc.h"

#ifndef VULKAN_GPU_VALIDATION_H
#define VULKAN_GPU_VALIDATION_H

struct GpuDeviceMemoryBlock {
    VkBuffer buffer;
    VmaAllocation allocation;
    std::unordered_map<uint32_t, const cvdescriptorset::Descriptor *> update_at_submit;
};

struct GpuBufferInfo {
    GpuDeviceMemoryBlock output_mem_block;
    GpuDeviceMemoryBlock input_mem_block;
    VkDescriptorSet desc_set;
    VkDescriptorPool desc_pool;
    VkPipelineBindPoint pipeline_bind_point;
    GpuBufferInfo(GpuDeviceMemoryBlock output_mem_block, GpuDeviceMemoryBlock input_mem_block, VkDescriptorSet desc_set,
                  VkDescriptorPool desc_pool, VkPipelineBindPoint pipeline_bind_point)
        : output_mem_block(output_mem_block),
          input_mem_block(input_mem_block),
          desc_set(desc_set),
          desc_pool(desc_pool),
          pipeline_bind_point(pipeline_bind_point){};
};

struct GpuQueueBarrierCommandInfo {
    VkCommandPool barrier_command_pool = VK_NULL_HANDLE;
    VkCommandBuffer barrier_command_buffer = VK_NULL_HANDLE;
};

// Class to encapsulate Descriptor Set allocation.  This manager creates and destroys Descriptor Pools
// as needed to satisfy requests for descriptor sets.
class GpuDescriptorSetManager {
   public:
    GpuDescriptorSetManager(CoreChecks *dev_data);
    ~GpuDescriptorSetManager();

    VkResult GetDescriptorSets(uint32_t count, VkDescriptorPool *pool, std::vector<VkDescriptorSet> *desc_sets);
    void PutBackDescriptorSet(VkDescriptorPool desc_pool, VkDescriptorSet desc_set);

   private:
    static const uint32_t kItemsPerChunk = 512;
    struct PoolTracker {
        uint32_t size;
        uint32_t used;
    };

    CoreChecks *dev_data_;
    std::unordered_map<VkDescriptorPool, struct PoolTracker> desc_pool_map_;
};

struct GpuValidationState {
    bool aborted;
    bool reserve_binding_slot;
    VkDescriptorSetLayout debug_desc_layout;
    VkDescriptorSetLayout dummy_desc_layout;
    uint32_t adjusted_max_desc_sets;
    uint32_t desc_set_bind_index;
    uint32_t unique_shader_module_id;
    std::unordered_map<uint32_t, ShaderTracker> shader_map;
    std::unique_ptr<GpuDescriptorSetManager> desc_set_manager;
    std::map<VkQueue, GpuQueueBarrierCommandInfo> queue_barrier_command_infos;
    std::unordered_map<VkCommandBuffer, std::vector<GpuBufferInfo>> command_buffer_map;  // gpu_buffer_list;
    uint32_t output_buffer_size;
    VmaAllocator vmaAllocator;
    PFN_vkSetDeviceLoaderData vkSetDeviceLoaderData;
    GpuValidationState(bool aborted = false, bool reserve_binding_slot = false, uint32_t unique_shader_module_id = 0,
                       VmaAllocator vmaAllocator = {})
        : aborted(aborted),
          reserve_binding_slot(reserve_binding_slot),
          unique_shader_module_id(unique_shader_module_id),
          vmaAllocator(vmaAllocator){};

    std::vector<GpuBufferInfo> &GetGpuBufferInfo(const VkCommandBuffer command_buffer) {
        auto buffer_list = command_buffer_map.find(command_buffer);
        if (buffer_list == command_buffer_map.end()) {
            std::vector<GpuBufferInfo> new_list{};
            command_buffer_map[command_buffer] = new_list;
            return command_buffer_map[command_buffer];
        }
        return buffer_list->second;
    }
};

using mutex_t = std::mutex;
using lock_guard_t = std::lock_guard<mutex_t>;
using unique_lock_t = std::unique_lock<mutex_t>;

#endif  // VULKAN_GPU_VALIDATION_H
