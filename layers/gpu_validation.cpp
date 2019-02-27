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

// Allow use of STL min and max functions in Windows
#define NOMINMAX

#include "core_validation.h"
#include "shader_validation.h"
#include "spirv-tools/libspirv.h"
#include "spirv-tools/optimizer.hpp"
#include "spirv-tools/instrument.hpp"
#include <SPIRV/spirv.hpp>
#include <algorithm>
#include <regex>

// This is the number of bindings in the debug descriptor set.
static const uint32_t kNumBindingsInSet = 1;

// Implementation for Device Memory Manager class
VkResult GpuDeviceMemoryManager::GetBlock(GpuDeviceMemoryBlock *block) {
    assert(block->buffer == VK_NULL_HANDLE);  // avoid possible overwrite/leak of an allocated block
    VkResult result = VK_SUCCESS;
    MemoryChunk *pChunk = nullptr;
    // Look for a chunk with available offsets.
    for (auto &chunk : chunk_list_) {
        if (!chunk.available_offsets.empty()) {
            pChunk = &chunk;
            break;
        }
    }
    // If no chunks with available offsets, allocate device memory and set up offsets.
    if (pChunk == nullptr) {
        MemoryChunk new_chunk;
        result = AllocMemoryChunk(new_chunk);
        if (result == VK_SUCCESS) {
            new_chunk.available_offsets.resize(blocks_per_chunk_);
            for (uint32_t offset = 0, i = 0; i < blocks_per_chunk_; offset += block_size_, ++i) {
                new_chunk.available_offsets[i] = offset;
            }
            chunk_list_.push_front(std::move(new_chunk));
            pChunk = &chunk_list_.front();
        } else {
            // Indicate failure
            block->buffer = VK_NULL_HANDLE;
            block->memory = VK_NULL_HANDLE;
            return result;
        }
    }
    // Give the requester an available offset
    block->buffer = pChunk->buffer;
    block->memory = pChunk->memory;
    block->offset = pChunk->available_offsets.back();
    pChunk->available_offsets.pop_back();
    return result;
}

void GpuDeviceMemoryManager::PutBackBlock(VkBuffer buffer, VkDeviceMemory memory, uint32_t offset) {
    GpuDeviceMemoryBlock block = {buffer, memory, offset};
    PutBackBlock(block);
}

void GpuDeviceMemoryManager::PutBackBlock(GpuDeviceMemoryBlock &block) {
    // Find the chunk belonging to the allocated offset and make the offset available again
    auto chunk = std::find_if(std::begin(chunk_list_), std::end(chunk_list_),
                              [&block](const MemoryChunk &c) { return c.buffer == block.buffer; });
    if (chunk_list_.end() == chunk) {
        assert(false);
    } else {
        chunk->available_offsets.push_back(block.offset);
        if (chunk->available_offsets.size() == blocks_per_chunk_) {
            // All offsets have been returned
            FreeMemoryChunk(*chunk);
            chunk_list_.erase(chunk);
        }
    }
}

void ResetBlock(GpuDeviceMemoryBlock &block) {
    block.buffer = VK_NULL_HANDLE;
    block.memory = VK_NULL_HANDLE;
    block.offset = 0;
}

bool BlockUsed(GpuDeviceMemoryBlock &block) { return (block.buffer != VK_NULL_HANDLE) && (block.memory != VK_NULL_HANDLE); }

bool GpuDeviceMemoryManager::MemoryTypeFromProperties(uint32_t typeBits, VkFlags requirements_mask, uint32_t *typeIndex) {
    // Search memtypes to find first index with those properties
    const VkPhysicalDeviceMemoryProperties *props = GetPhysicalDeviceMemoryProperties(dev_data_);
    for (uint32_t i = 0; i < VK_MAX_MEMORY_TYPES; i++) {
        if ((typeBits & 1) == 1) {
            // Type is available, does it match user properties?
            if ((props->memoryTypes[i].propertyFlags & requirements_mask) == requirements_mask) {
                *typeIndex = i;
                return true;
            }
        }
        typeBits >>= 1;
    }
    // No memory types matched, return failure
    return false;
}

VkResult GpuDeviceMemoryManager::AllocMemoryChunk(MemoryChunk &chunk) {
    VkBuffer buffer;
    VkDeviceMemory memory;
    VkBufferCreateInfo buffer_create_info = {};
    VkMemoryRequirements mem_reqs = {};
    VkMemoryAllocateInfo mem_alloc = {};
    VkResult result = VK_SUCCESS;
    bool pass;
    void *pData;
    const auto *dispatch_table = GetDispatchTable(dev_data_);

    buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_create_info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    buffer_create_info.size = chunk_size_;
    result = dispatch_table->CreateBuffer(GetDevice(dev_data_), &buffer_create_info, NULL, &buffer);
    if (result != VK_SUCCESS) {
        return result;
    }

    dispatch_table->GetBufferMemoryRequirements(GetDevice(dev_data_), buffer, &mem_reqs);

    mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    mem_alloc.pNext = NULL;
    mem_alloc.allocationSize = mem_reqs.size;
    pass = MemoryTypeFromProperties(mem_reqs.memoryTypeBits,
                                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                    &mem_alloc.memoryTypeIndex);
    if (!pass) {
        dispatch_table->DestroyBuffer(GetDevice(dev_data_), buffer, NULL);
        return result;
    }
    result = dispatch_table->AllocateMemory(GetDevice(dev_data_), &mem_alloc, NULL, &memory);
    if (result != VK_SUCCESS) {
        dispatch_table->DestroyBuffer(GetDevice(dev_data_), buffer, NULL);
        return result;
    }

    result = dispatch_table->BindBufferMemory(GetDevice(dev_data_), buffer, memory, 0);
    if (result != VK_SUCCESS) {
        dispatch_table->DestroyBuffer(GetDevice(dev_data_), buffer, NULL);
        dispatch_table->FreeMemory(GetDevice(dev_data_), memory, NULL);
        return result;
    }

    result = dispatch_table->MapMemory(GetDevice(dev_data_), memory, 0, mem_alloc.allocationSize, 0, &pData);
    if (result == VK_SUCCESS) {
        memset(pData, 0, chunk_size_);
        dispatch_table->UnmapMemory(GetDevice(dev_data_), memory);
    } else {
        dispatch_table->DestroyBuffer(GetDevice(dev_data_), buffer, NULL);
        dispatch_table->FreeMemory(GetDevice(dev_data_), memory, NULL);
        return result;
    }
    chunk.buffer = buffer;
    chunk.memory = memory;
    return result;
}

void GpuDeviceMemoryManager::FreeMemoryChunk(MemoryChunk &chunk) {
    GetDispatchTable(dev_data_)->DestroyBuffer(GetDevice(dev_data_), chunk.buffer, NULL);
    GetDispatchTable(dev_data_)->FreeMemory(GetDevice(dev_data_), chunk.memory, NULL);
}

void GpuDeviceMemoryManager::FreeAllBlocks() {
    for (auto &chunk : chunk_list_) {
        FreeMemoryChunk(chunk);
    }
    chunk_list_.clear();
}

// Implementation for Descriptor Set Manager class
VkResult GpuDescriptorSetManager::GetDescriptorSets(uint32_t count, VkDescriptorPool *pool,
                                                    std::vector<VkDescriptorSet> *desc_sets) {
    auto gpu_state = GetGpuValidationState(dev_data_);
    const uint32_t default_pool_size = kItemsPerChunk;
    VkResult result = VK_SUCCESS;
    VkDescriptorPool pool_to_use = VK_NULL_HANDLE;

    if (0 == count) {
        return result;
    }
    desc_sets->clear();
    desc_sets->resize(count);

    for (auto &pool : desc_pool_map_) {
        if (pool.second.used + count < pool.second.size) {
            pool_to_use = pool.first;
            break;
        }
    }
    if (VK_NULL_HANDLE == pool_to_use) {
        uint32_t pool_count = default_pool_size;
        if (count > default_pool_size) {
            pool_count = count;
        }
        const VkDescriptorPoolSize size_counts = {
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            pool_count * kNumBindingsInSet,
        };
        VkDescriptorPoolCreateInfo desc_pool_info = {};
        desc_pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        desc_pool_info.pNext = NULL;
        desc_pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        desc_pool_info.maxSets = pool_count;
        desc_pool_info.poolSizeCount = 1;
        desc_pool_info.pPoolSizes = &size_counts;
        result = GetDispatchTable(dev_data_)->CreateDescriptorPool(GetDevice(dev_data_), &desc_pool_info, NULL, &pool_to_use);
        assert(result == VK_SUCCESS);
        if (result != VK_SUCCESS) {
            return result;
        }
        desc_pool_map_[pool_to_use].size = desc_pool_info.maxSets;
        desc_pool_map_[pool_to_use].used = 0;
    }
    std::vector<VkDescriptorSetLayout> desc_layouts(count, gpu_state->debug_desc_layout);

    VkDescriptorSetAllocateInfo alloc_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO, NULL, pool_to_use, count,
                                              desc_layouts.data()};

    result = GetDispatchTable(dev_data_)->AllocateDescriptorSets(GetDevice(dev_data_), &alloc_info, desc_sets->data());
    assert(result == VK_SUCCESS);
    if (result != VK_SUCCESS) {
        return result;
    }
    *pool = pool_to_use;
    desc_pool_map_[pool_to_use].used += count;
    return result;
}

void GpuDescriptorSetManager::PutBackDescriptorSet(VkDescriptorPool desc_pool, VkDescriptorSet desc_set) {
    auto iter = desc_pool_map_.find(desc_pool);
    if (iter != desc_pool_map_.end()) {
        VkResult result = GetDispatchTable(dev_data_)->FreeDescriptorSets(GetDevice(dev_data_), desc_pool, 1, &desc_set);
        assert(result == VK_SUCCESS);
        if (result != VK_SUCCESS) {
            return;
        }
        desc_pool_map_[desc_pool].used--;
        if (0 == desc_pool_map_[desc_pool].used) {
            GetDispatchTable(dev_data_)->DestroyDescriptorPool(GetDevice(dev_data_), desc_pool, NULL);
            desc_pool_map_.erase(desc_pool);
        }
    }
    return;
}

void GpuDescriptorSetManager::DestroyDescriptorPools() {
    for (auto &pool : desc_pool_map_) {
        GetDispatchTable(dev_data_)->DestroyDescriptorPool(GetDevice(dev_data_), pool.first, NULL);
    }
    desc_pool_map_.clear();
}

// Convenience function for reporting problems with setting up GPU Validation.
static void ReportSetupProblem(const layer_data *dev_data, VkDebugReportObjectTypeEXT object_type, uint64_t object_handle,
                               const char *const specific_message) {
    log_msg(GetReportData(dev_data), VK_DEBUG_REPORT_ERROR_BIT_EXT, object_type, object_handle,
            "UNASSIGNED-GPU-Assisted Validation Error. ", "Detail: (%s)", specific_message);
}

// Turn on necessary device features.
std::unique_ptr<safe_VkDeviceCreateInfo> GpuPreCallRecordCreateDevice(VkPhysicalDevice gpu, const VkDeviceCreateInfo *create_info,
                                                                      VkPhysicalDeviceFeatures *supported_features) {
    std::unique_ptr<safe_VkDeviceCreateInfo> new_info(new safe_VkDeviceCreateInfo(create_info));
    if (supported_features->fragmentStoresAndAtomics || supported_features->vertexPipelineStoresAndAtomics) {
        VkPhysicalDeviceFeatures new_features = {};
        if (new_info->pEnabledFeatures) {
            new_features = *new_info->pEnabledFeatures;
        }
        new_features.fragmentStoresAndAtomics = supported_features->fragmentStoresAndAtomics;
        new_features.vertexPipelineStoresAndAtomics = supported_features->vertexPipelineStoresAndAtomics;
        delete new_info->pEnabledFeatures;
        new_info->pEnabledFeatures = new VkPhysicalDeviceFeatures(new_features);
    }
    return new_info;
}

// Perform initializations that can be done at Create Device time.
void GpuPostCallRecordCreateDevice(layer_data *dev_data) {
    auto gpu_state = GetGpuValidationState(dev_data);
    const auto *dispatch_table = GetDispatchTable(dev_data);

    gpu_state->aborted = false;
    gpu_state->reserve_binding_slot = false;
    gpu_state->barrier_command_pool = VK_NULL_HANDLE;
    gpu_state->barrier_command_buffer = VK_NULL_HANDLE;

    if (GetPDProperties(dev_data)->apiVersion < VK_API_VERSION_1_1) {
        ReportSetupProblem(dev_data, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(GetDevice(dev_data)),
                           "GPU-Assisted validation requires Vulkan 1.1 or later.  GPU-Assisted Validation disabled.");
        gpu_state->aborted = true;
        return;
    }
    // Some devices have extremely high limits here, so set a reasonable max because we have to pad
    // the pipeline layout with dummy descriptor set layouts.
    gpu_state->adjusted_max_desc_sets = GetPDProperties(dev_data)->limits.maxBoundDescriptorSets;
    gpu_state->adjusted_max_desc_sets = std::min(33U, gpu_state->adjusted_max_desc_sets);

    // We can't do anything if there is only one.
    // Device probably not a legit Vulkan device, since there should be at least 4. Protect ourselves.
    if (gpu_state->adjusted_max_desc_sets == 1) {
        ReportSetupProblem(dev_data, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(GetDevice(dev_data)),
                           "Device can bind only a single descriptor set.  GPU-Assisted Validation disabled.");
        gpu_state->aborted = true;
        return;
    }
    gpu_state->desc_set_bind_index = gpu_state->adjusted_max_desc_sets - 1;
    log_msg(GetReportData(dev_data), VK_DEBUG_REPORT_INFORMATION_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT,
            HandleToUint64(GetDevice(dev_data)), "UNASSIGNED-GPU-Assisted Validation. ",
            "Shaders using descriptor set at index %d. ", gpu_state->desc_set_bind_index);

    std::unique_ptr<GpuDeviceMemoryManager> memory_manager(
        new GpuDeviceMemoryManager(dev_data, sizeof(uint32_t) * (spvtools::kInstMaxOutCnt + 1)));
    std::unique_ptr<GpuDescriptorSetManager> desc_set_manager(new GpuDescriptorSetManager(dev_data));

    // The descriptor indexing checks require only the first "output" binding.
    const VkDescriptorSetLayoutBinding debug_desc_layout_bindings[kNumBindingsInSet] = {
        {
            0,  // output
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            1,
            VK_SHADER_STAGE_ALL_GRAPHICS,
            NULL,
        },
    };

    const VkDescriptorSetLayoutCreateInfo debug_desc_layout_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO, NULL, 0,
                                                                    kNumBindingsInSet, debug_desc_layout_bindings};

    const VkDescriptorSetLayoutCreateInfo dummy_desc_layout_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO, NULL, 0, 0,
                                                                    NULL};

    VkResult result = dispatch_table->CreateDescriptorSetLayout(GetDevice(dev_data), &debug_desc_layout_info, NULL,
                                                                &gpu_state->debug_desc_layout);

    // This is a layout used to "pad" a pipeline layout to fill in any gaps to the selected bind index.
    VkResult result2 = dispatch_table->CreateDescriptorSetLayout(GetDevice(dev_data), &dummy_desc_layout_info, NULL,
                                                                 &gpu_state->dummy_desc_layout);
    assert((result == VK_SUCCESS) && (result2 == VK_SUCCESS));
    if ((result != VK_SUCCESS) || (result2 != VK_SUCCESS)) {
        ReportSetupProblem(dev_data, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(GetDevice(dev_data)),
                           "Unable to create descriptor set layout.  GPU-Assisted Validation disabled.");
        if (result == VK_SUCCESS) {
            dispatch_table->DestroyDescriptorSetLayout(GetDevice(dev_data), gpu_state->debug_desc_layout, NULL);
        }
        if (result2 == VK_SUCCESS) {
            dispatch_table->DestroyDescriptorSetLayout(GetDevice(dev_data), gpu_state->dummy_desc_layout, NULL);
        }
        gpu_state->debug_desc_layout = VK_NULL_HANDLE;
        gpu_state->dummy_desc_layout = VK_NULL_HANDLE;
        gpu_state->aborted = true;
        return;
    }
    gpu_state->memory_manager = std::move(memory_manager);
    gpu_state->desc_set_manager = std::move(desc_set_manager);
}

// Clean up device-related resources
void GpuPreCallRecordDestroyDevice(layer_data *dev_data) {
    auto gpu_state = GetGpuValidationState(dev_data);

    if (gpu_state->barrier_command_buffer) {
        GetDispatchTable(dev_data)->FreeCommandBuffers(GetDevice(dev_data), gpu_state->barrier_command_pool, 1,
                                                       &gpu_state->barrier_command_buffer);
        gpu_state->barrier_command_buffer = VK_NULL_HANDLE;
    }
    if (gpu_state->barrier_command_pool) {
        GetDispatchTable(dev_data)->DestroyCommandPool(GetDevice(dev_data), gpu_state->barrier_command_pool, NULL);
        gpu_state->barrier_command_pool = VK_NULL_HANDLE;
    }
    if (gpu_state->debug_desc_layout) {
        GetDispatchTable(dev_data)->DestroyDescriptorSetLayout(GetDevice(dev_data), gpu_state->debug_desc_layout, NULL);
        gpu_state->debug_desc_layout = VK_NULL_HANDLE;
    }
    if (gpu_state->dummy_desc_layout) {
        GetDispatchTable(dev_data)->DestroyDescriptorSetLayout(GetDevice(dev_data), gpu_state->dummy_desc_layout, NULL);
        gpu_state->dummy_desc_layout = VK_NULL_HANDLE;
    }
    gpu_state->memory_manager->FreeAllBlocks();
    gpu_state->desc_set_manager->DestroyDescriptorPools();
}

// Modify the pipeline layout to include our debug descriptor set and any needed padding with the dummy descriptor set.
bool GpuPreCallCreatePipelineLayout(layer_data *device_data, const VkPipelineLayoutCreateInfo *pCreateInfo,
                                    const VkAllocationCallbacks *pAllocator, VkPipelineLayout *pPipelineLayout,
                                    std::vector<VkDescriptorSetLayout> *new_layouts,
                                    VkPipelineLayoutCreateInfo *modified_create_info) {
    auto gpu_state = GetGpuValidationState(device_data);
    if (gpu_state->aborted) {
        return false;
    }

    if (modified_create_info->setLayoutCount >= gpu_state->adjusted_max_desc_sets) {
        std::ostringstream strm;
        strm << "Pipeline Layout conflict with validation's descriptor set at slot " << gpu_state->desc_set_bind_index << ". "
             << "Application has too many descriptor sets in the pipeline layout to continue with gpu validation. "
             << "Validation is not modifying the pipeline layout. "
             << "Instrumented shaders are replaced with non-instrumented shaders.";
        ReportSetupProblem(device_data, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(GetDevice(device_data)),
                           strm.str().c_str());
    } else {
        // Modify the pipeline layout by:
        // 1. Copying the caller's descriptor set desc_layouts
        // 2. Fill in dummy descriptor layouts up to the max binding
        // 3. Fill in with the debug descriptor layout at the max binding slot
        new_layouts->reserve(gpu_state->adjusted_max_desc_sets);
        new_layouts->insert(new_layouts->end(), &pCreateInfo->pSetLayouts[0],
                            &pCreateInfo->pSetLayouts[pCreateInfo->setLayoutCount]);
        for (uint32_t i = pCreateInfo->setLayoutCount; i < gpu_state->adjusted_max_desc_sets - 1; ++i) {
            new_layouts->push_back(gpu_state->dummy_desc_layout);
        }
        new_layouts->push_back(gpu_state->debug_desc_layout);
        modified_create_info->pSetLayouts = new_layouts->data();
        modified_create_info->setLayoutCount = gpu_state->adjusted_max_desc_sets;
    }
    return true;
}

// Clean up GPU validation after the CreatePipelineLayout call is made
void GpuPostCallCreatePipelineLayout(layer_data *device_data, VkResult result) {
    auto gpu_state = GetGpuValidationState(device_data);
    // Clean up GPU validation
    if (result != VK_SUCCESS) {
        ReportSetupProblem(device_data, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(GetDevice(device_data)),
                           "Unable to create pipeline layout.  Device could become unstable.");
        gpu_state->aborted = true;
    }
}

// Free the device memory and descriptor set associated with a command buffer.
void GpuPreCallRecordFreeCommandBuffers(layer_data *dev_data, uint32_t commandBufferCount, const VkCommandBuffer *pCommandBuffers) {
    auto gpu_state = GetGpuValidationState(dev_data);
    if (gpu_state->aborted) {
        return;
    }
    for (uint32_t i = 0; i < commandBufferCount; ++i) {
        auto cb_node = GetCBNode(dev_data, pCommandBuffers[i]);
        if (cb_node) {
            for (auto &buffer_info : cb_node->gpu_buffer_list) {
                if (BlockUsed(buffer_info.mem_block)) {
                    gpu_state->memory_manager->PutBackBlock(buffer_info.mem_block);
                    ResetBlock(buffer_info.mem_block);
                }
                if (buffer_info.desc_set != VK_NULL_HANDLE) {
                    gpu_state->desc_set_manager->PutBackDescriptorSet(buffer_info.desc_pool, buffer_info.desc_set);
                }
            }
            cb_node->gpu_buffer_list.clear();
        }
    }
}

// Just gives a warning about a possible deadlock.
void GpuPreCallValidateCmdWaitEvents(layer_data *dev_data, VkPipelineStageFlags sourceStageMask) {
    if (sourceStageMask & VK_PIPELINE_STAGE_HOST_BIT) {
        ReportSetupProblem(dev_data, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(GetDevice(dev_data)),
                           "CmdWaitEvents recorded with VK_PIPELINE_STAGE_HOST_BIT set. "
                           "GPU_Assisted validation waits on queue completion. "
                           "This wait could block the host's signaling of this event, resulting in deadlock.");
    }
}

// Examine the pipelines to see if they use the debug descriptor set binding index.
// If any do, create new non-instrumented shader modules and use them to replace the instrumented
// shaders in the pipeline.  Return the (possibly) modified create infos to the caller.
std::vector<safe_VkGraphicsPipelineCreateInfo> GpuPreCallRecordCreateGraphicsPipelines(
    layer_data *dev_data, VkPipelineCache pipelineCache, uint32_t count, const VkGraphicsPipelineCreateInfo *pCreateInfos,
    const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines, std::vector<std::unique_ptr<PIPELINE_STATE>> &pipe_state) {
    auto gpu_state = GetGpuValidationState(dev_data);

    std::vector<safe_VkGraphicsPipelineCreateInfo> new_pipeline_create_infos;
    std::vector<unsigned int> pipeline_uses_debug_index(count);

    // Walk through all the pipelines, make a copy of each and flag each pipeline that contains a shader that uses the debug
    // descriptor set index.
    for (uint32_t pipeline = 0; pipeline < count; ++pipeline) {
        new_pipeline_create_infos.push_back(pipe_state[pipeline]->graphicsPipelineCI);
        if (pipe_state[pipeline]->active_slots.find(gpu_state->desc_set_bind_index) != pipe_state[pipeline]->active_slots.end()) {
            pipeline_uses_debug_index[pipeline] = 1;
        }
    }

    // See if any pipeline has shaders using the debug descriptor set index
    if (std::all_of(pipeline_uses_debug_index.begin(), pipeline_uses_debug_index.end(), [](unsigned int i) { return i == 0; })) {
        // None of the shaders in all the pipelines use the debug descriptor set index, so use the pipelines
        // as they stand with the instrumented shaders.
        return new_pipeline_create_infos;
    }

    // At least one pipeline has a shader that uses the debug descriptor set index.
    for (uint32_t pipeline = 0; pipeline < count; ++pipeline) {
        if (pipeline_uses_debug_index[pipeline]) {
            for (uint32_t stage = 0; stage < pCreateInfos[pipeline].stageCount; ++stage) {
                const shader_module *shader = GetShaderModuleState(dev_data, pCreateInfos[pipeline].pStages[stage].module);
                VkShaderModuleCreateInfo create_info = {};
                VkShaderModule shader_module;
                create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
                create_info.pCode = shader->words.data();
                create_info.codeSize = shader->words.size() * sizeof(uint32_t);
                VkResult result =
                    GetDispatchTable(dev_data)->CreateShaderModule(GetDevice(dev_data), &create_info, pAllocator, &shader_module);
                if (result == VK_SUCCESS) {
                    new_pipeline_create_infos[pipeline].pStages[stage].module = shader_module;
                } else {
                    ReportSetupProblem(dev_data, VK_DEBUG_REPORT_OBJECT_TYPE_SHADER_MODULE_EXT,
                                       HandleToUint64(pCreateInfos[pipeline].pStages[stage].module),
                                       "Unable to replace instrumented shader with non-instrumented one.  "
                                       "Device could become unstable.");
                }
            }
        }
    }
    return new_pipeline_create_infos;
}

// For every pipeline:
// - For every shader in a pipeline:
//   - If the shader had to be replaced in PreCallRecord (because the pipeline is using the debug desc set index):
//     - Destroy it since it has been bound into the pipeline by now.  This is our only chance to delete it.
//   - Track the shader in the shader_map
//   - Save the shader binary if it contains debug code
void GpuPostCallRecordCreateGraphicsPipelines(layer_data *dev_data, const uint32_t count,
                                              const VkGraphicsPipelineCreateInfo *pCreateInfos,
                                              const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines) {
    auto gpu_state = GetGpuValidationState(dev_data);
    for (uint32_t pipeline = 0; pipeline < count; ++pipeline) {
        auto pipeline_state = GetPipelineState(dev_data, pPipelines[pipeline]);
        if (nullptr == pipeline_state) continue;
        for (uint32_t stage = 0; stage < pipeline_state->graphicsPipelineCI.stageCount; ++stage) {
            if (pipeline_state->active_slots.find(gpu_state->desc_set_bind_index) != pipeline_state->active_slots.end()) {
                GetDispatchTable(dev_data)->DestroyShaderModule(GetDevice(dev_data), pCreateInfos->pStages[stage].module,
                                                                pAllocator);
            }
            auto shader_state = GetShaderModuleState(dev_data, pipeline_state->graphicsPipelineCI.pStages[stage].module);
            std::vector<unsigned int> code;
            // Save the shader binary if debug info is present.
            // The core_validation ShaderModule tracker saves the binary too, but discards it when the ShaderModule
            // is destroyed.  Applications may destroy ShaderModules after they are placed in a pipeline and before
            // the pipeline is used, so we have to keep another copy.
            if (shader_state && shader_state->has_valid_spirv) {  // really checking for presense of SPIR-V code.
                for (auto insn : *shader_state) {
                    if (insn.opcode() == spv::OpLine) {
                        code = shader_state->words;
                        break;
                    }
                }
            }
            gpu_state->shader_map[shader_state->gpu_validation_shader_id].pipeline = pipeline_state->pipeline;
            // Be careful to use the originally bound (instrumented) shader here, even if PreCallRecord had to back it
            // out with a non-instrumented shader.  The non-instrumented shader (found in pCreateInfo) was destroyed above.
            gpu_state->shader_map[shader_state->gpu_validation_shader_id].shader_module =
                pipeline_state->graphicsPipelineCI.pStages[stage].module;
            gpu_state->shader_map[shader_state->gpu_validation_shader_id].pgm = std::move(code);
        }
    }
}

// Remove all the shader trackers associated with this destroyed pipeline.
void GpuPreCallRecordDestroyPipeline(layer_data *dev_data, const VkPipeline pipeline) {
    auto gpu_state = GetGpuValidationState(dev_data);
    for (auto it = gpu_state->shader_map.begin(); it != gpu_state->shader_map.end();) {
        if (it->second.pipeline == pipeline) {
            it = gpu_state->shader_map.erase(it);
        } else {
            ++it;
        }
    }
}

// Call the SPIR-V Optimizer to run the instrumentation pass on the shader.
static bool GpuInstrumentShader(layer_data *dev_data, const VkShaderModuleCreateInfo *pCreateInfo,
                                std::vector<unsigned int> &new_pgm, uint32_t *unique_shader_id) {
    auto gpu_state = GetGpuValidationState(dev_data);
    if (gpu_state->aborted) return false;
    if (pCreateInfo->pCode[0] != spv::MagicNumber) return false;

    // Load original shader SPIR-V
    uint32_t num_words = static_cast<uint32_t>(pCreateInfo->codeSize / 4);
    new_pgm.clear();
    new_pgm.reserve(num_words);
    new_pgm.insert(new_pgm.end(), &pCreateInfo->pCode[0], &pCreateInfo->pCode[num_words]);

    // Call the optimizer to instrument the shader.
    // Use the unique_shader_module_id as a shader ID so we can look up its handle later in the shader_map.
    using namespace spvtools;
    spv_target_env target_env = SPV_ENV_VULKAN_1_1;
    Optimizer optimizer(target_env);
    optimizer.RegisterPass(CreateInstBindlessCheckPass(gpu_state->desc_set_bind_index, gpu_state->unique_shader_module_id));
    optimizer.RegisterPass(CreateAggressiveDCEPass());
    bool pass = optimizer.Run(new_pgm.data(), new_pgm.size(), &new_pgm);
    if (!pass) {
        ReportSetupProblem(dev_data, VK_DEBUG_REPORT_OBJECT_TYPE_SHADER_MODULE_EXT, VK_NULL_HANDLE,
                           "Failure to instrument shader.  Proceeding with non-instrumented shader.");
    }
    *unique_shader_id = gpu_state->unique_shader_module_id++;
    return pass;
}

// Create the instrumented shader data to provide to the driver.
bool GpuPreCallCreateShaderModule(layer_data *dev_data, const VkShaderModuleCreateInfo *pCreateInfo,
                                  const VkAllocationCallbacks *pAllocator, VkShaderModule *pShaderModule,
                                  uint32_t *unique_shader_id, VkShaderModuleCreateInfo *instrumented_create_info,
                                  std::vector<unsigned int> *instrumented_pgm) {
    bool pass = GpuInstrumentShader(dev_data, pCreateInfo, *instrumented_pgm, unique_shader_id);
    if (pass) {
        instrumented_create_info->pCode = instrumented_pgm->data();
        instrumented_create_info->codeSize = instrumented_pgm->size() * sizeof(unsigned int);
    }
    return pass;
}

// Generate the stage-specific part of the message.
static void GenerateStageMessage(const uint32_t *debug_record, std::string &msg) {
    using namespace spvtools;
    std::ostringstream strm;
    switch (debug_record[kInstCommonOutStageIdx]) {
        case 0: {
            strm << "Stage = Vertex. Vertex Index = " << debug_record[kInstVertOutVertexIndex]
                 << " Instance Index = " << debug_record[kInstVertOutInstanceIndex] << ". ";
        } break;
        case 1: {
            strm << "Stage = Tessellation Control.  Invocation ID = " << debug_record[kInstTessOutInvocationId] << ". ";
        } break;
        case 2: {
            strm << "Stage = Tessellation Eval.  Invocation ID = " << debug_record[kInstTessOutInvocationId] << ". ";
        } break;
        case 3: {
            strm << "Stage = Geometry.  Primitive ID = " << debug_record[kInstGeomOutPrimitiveId]
                 << " Invocation ID = " << debug_record[kInstGeomOutInvocationId] << ". ";
        } break;
        case 4: {
            strm << "Stage = Fragment.  Fragment coord (x,y) = ("
                 << *reinterpret_cast<const float *>(&debug_record[kInstFragOutFragCoordX]) << ", "
                 << *reinterpret_cast<const float *>(&debug_record[kInstFragOutFragCoordY]) << "). ";
        } break;
        case 5: {
            strm << "Stage = Compute.  Global invocation ID = " << debug_record[kInstCompOutGlobalInvocationId] << ". ";
        } break;
        default: {
            strm << "Internal Error (unexpected stage = " << debug_record[kInstCommonOutStageIdx] << "). ";
            assert(false);
        } break;
    }
    msg = strm.str();
}

// Generate the part of the message describing the violation.
static void GenerateValidationMessage(const uint32_t *debug_record, std::string &msg, std::string &vuid_msg) {
    using namespace spvtools;
    std::ostringstream strm;
    switch (debug_record[kInstValidationOutError]) {
        case 0: {
            strm << "Index of " << debug_record[kInstBindlessOutDescIndex] << " used to index descriptor array of length "
                 << debug_record[kInstBindlessOutDescBound] << ". ";
            vuid_msg = "UNASSIGNED-Descriptor index out of bounds";
        } break;
        case 1: {
            strm << "Descriptor index " << debug_record[kInstBindlessOutDescIndex] << " is uninitialized. ";
            vuid_msg = "UNASSIGNED-Descriptor uninitialized";
        } break;
        default: {
            strm << "Internal Error (unexpected error type = " << debug_record[kInstValidationOutError] << "). ";
            vuid_msg = "UNASSIGNED-Internal Error";
            assert(false);
        } break;
    }
    msg = strm.str();
}

static std::string LookupDebugUtilsName(const layer_data *device_data, const uint64_t object) {
    debug_report_data *report_data = device_data->report_data;
    auto object_label = report_data->DebugReportGetUtilsObjectName(object);
    if (object_label != "") {
        object_label = "(" + object_label + ")";
    }
    return object_label;
}

// Generate message from the common portion of the debug report record.
static void GenerateCommonMessage(const layer_data *dev_data, const GLOBAL_CB_NODE *cb_node, const uint32_t *debug_record,
                                  const VkShaderModule shader_module_handle, const VkPipeline pipeline_handle,
                                  const uint32_t draw_index, std::string &msg) {
    using namespace spvtools;
    std::ostringstream strm;
    if (shader_module_handle == VK_NULL_HANDLE) {
        strm << std::hex << std::showbase << "Internal Error: Unable to locate information for shader used in command buffer "
             << LookupDebugUtilsName(dev_data, HandleToUint64(cb_node->commandBuffer)) << "("
             << HandleToUint64(cb_node->commandBuffer) << "). ";
        assert(true);
    } else {
        strm << std::hex << std::showbase << "Command buffer "
             << LookupDebugUtilsName(dev_data, HandleToUint64(cb_node->commandBuffer)) << "("
             << HandleToUint64(cb_node->commandBuffer) << "). "
             << "Draw Index " << draw_index << ". "
             << "Pipeline " << LookupDebugUtilsName(dev_data, HandleToUint64(pipeline_handle)) << "("
             << HandleToUint64(pipeline_handle) << "). "
             << "Shader Module " << LookupDebugUtilsName(dev_data, HandleToUint64(shader_module_handle)) << "("
             << HandleToUint64(shader_module_handle) << "). ";
    }
    strm << std::dec << std::noshowbase;
    strm << "Shader Instruction Index = " << debug_record[kInstCommonOutInstructionIdx] << ". ";
    msg = strm.str();
}

// Read the contents of the SPIR-V OpSource instruction and any following continuation instructions.
// Split the single string into a vector of strings, one for each line, for easier processing.
static void ReadOpSource(const shader_module &shader, const uint32_t reported_file_id, std::vector<std::string> &opsource_lines) {
    for (auto insn : shader) {
        if ((insn.opcode() == spv::OpSource) && (insn.len() >= 5) && (insn.word(3) == reported_file_id)) {
            std::istringstream in_stream;
            std::string cur_line;
            in_stream.str((char *)&insn.word(4));
            while (std::getline(in_stream, cur_line)) {
                opsource_lines.push_back(cur_line);
            }
            while ((++insn).opcode() == spv::OpSourceContinued) {
                in_stream.str((char *)&insn.word(1));
                while (std::getline(in_stream, cur_line)) {
                    opsource_lines.push_back(cur_line);
                }
            }
            break;
        }
    }
}

// The task here is to search the OpSource content to find the #line directive with the
// line number that is closest to, but still prior to the reported error line number and
// still within the reported filename.
// From this known position in the OpSource content we can add the difference between
// the #line line number and the reported error line number to determine the location
// in the OpSource content of the reported error line.
//
// Considerations:
// - Look only at #line directives that specify the reported_filename since
//   the reported error line number refers to its location in the reported filename.
// - If a #line directive does not have a filename, the file is the reported filename, or
//   the filename found in a prior #line directive.  (This is C-preprocessor behavior)
// - It is possible (e.g., inlining) for blocks of code to get shuffled out of their
//   original order and the #line directives are used to keep the numbering correct.  This
//   is why we need to examine the entire contents of the source, instead of leaving early
//   when finding a #line line number larger than the reported error line number.
//

// GCC 4.8 has a problem with std::regex that is fixed in GCC 4.9.  Provide fallback code for 4.8
#define GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)

#if defined(__GNUC__) && GCC_VERSION < 40900
static bool GetLineAndFilename(const std::string string, uint32_t *linenumber, std::string &filename) {
    // # line <linenumber> "<filename>" or
    // #line <linenumber> "<filename>"
    std::vector<std::string> tokens;
    std::stringstream stream(string);
    std::string temp;
    uint32_t line_index = 0;

    while (stream >> temp) tokens.push_back(temp);
    auto size = tokens.size();
    if (size > 1) {
        if (tokens[0] == "#" && tokens[1] == "line") {
            line_index = 2;
        } else if (tokens[0] == "#line") {
            line_index = 1;
        }
    }
    if (0 == line_index) return false;
    *linenumber = std::stoul(tokens[line_index]);
    uint32_t filename_index = line_index + 1;
    // Remove enclosing double quotes around filename
    if (size > filename_index) filename = tokens[filename_index].substr(1, tokens[filename_index].size() - 2);
    return true;
}
#else
static bool GetLineAndFilename(const std::string string, uint32_t *linenumber, std::string &filename) {
    static const std::regex line_regex(  // matches #line directives
        "^"                              // beginning of line
        "\\s*"                           // optional whitespace
        "#"                              // required text
        "\\s*"                           // optional whitespace
        "line"                           // required text
        "\\s+"                           // required whitespace
        "([0-9]+)"                       // required first capture - line number
        "(\\s+)?"                        // optional second capture - whitespace
        "(\".+\")?"                      // optional third capture - quoted filename with at least one char inside
        ".*");                           // rest of line (needed when using std::regex_match since the entire line is tested)

    std::smatch captures;

    bool found_line = std::regex_match(string, captures, line_regex);
    if (!found_line) return false;

    // filename is optional and considered found only if the whitespace and the filename are captured
    if (captures[2].matched && captures[3].matched) {
        // Remove enclosing double quotes.  The regex guarantees the quotes and at least one char.
        filename = captures[3].str().substr(1, captures[3].str().size() - 2);
    }
    *linenumber = std::stoul(captures[1]);
    return true;
}
#endif  // GCC_VERSION

// Extract the filename, line number, and column number from the correct OpLine and build a message string from it.
// Scan the source (from OpSource) to find the line of source at the reported line number and place it in another message string.
static void GenerateSourceMessages(const std::vector<unsigned int> &pgm, const uint32_t *debug_record, std::string &filename_msg,
                                   std::string &source_msg) {
    using namespace spvtools;
    std::ostringstream filename_stream;
    std::ostringstream source_stream;
    shader_module shader;
    shader.words = pgm;
    // Find the OpLine just before the failing instruction indicated by the debug info.
    // SPIR-V can only be iterated in the forward direction due to its opcode/length encoding.
    uint32_t instruction_index = 0;
    uint32_t reported_file_id = 0;
    uint32_t reported_line_number = 0;
    uint32_t reported_column_number = 0;
    if (shader.words.size() > 0) {
        for (auto insn : shader) {
            if (insn.opcode() == spv::OpLine) {
                reported_file_id = insn.word(1);
                reported_line_number = insn.word(2);
                reported_column_number = insn.word(3);
            }
            if (instruction_index == debug_record[kInstCommonOutInstructionIdx]) {
                break;
            }
            instruction_index++;
        }
    }
    // Create message with file information obtained from the OpString pointed to by the discovered OpLine.
    std::string reported_filename;
    if (reported_file_id == 0) {
        filename_stream
            << "Unable to find SPIR-V OpLine for source information.  Build shader with debug info to get source information.";
    } else {
        bool found_opstring = false;
        for (auto insn : shader) {
            if ((insn.opcode() == spv::OpString) && (insn.len() >= 3) && (insn.word(1) == reported_file_id)) {
                found_opstring = true;
                reported_filename = (char *)&insn.word(2);
                if (reported_filename.empty()) {
                    filename_stream << "Shader validation error occurred at line " << reported_line_number;
                } else {
                    filename_stream << "Shader validation error occurred in file: " << reported_filename << " at line "
                                    << reported_line_number;
                }
                if (reported_column_number > 0) {
                    filename_stream << ", column " << reported_column_number;
                }
                filename_stream << ".";
                break;
            }
        }
        if (!found_opstring) {
            filename_stream << "Unable to find SPIR-V OpString for file id " << reported_file_id << " from OpLine instruction.";
        }
    }
    filename_msg = filename_stream.str();

    // Create message to display source code line containing error.
    if ((reported_file_id != 0)) {
        // Read the source code and split it up into separate lines.
        std::vector<std::string> opsource_lines;
        ReadOpSource(shader, reported_file_id, opsource_lines);
        // Find the line in the OpSource content that corresponds to the reported error file and line.
        if (!opsource_lines.empty()) {
            uint32_t saved_line_number = 0;
            std::string current_filename = reported_filename;  // current "preprocessor" filename state.
            std::vector<std::string>::size_type saved_opsource_offset = 0;
            bool found_best_line = false;
            for (auto it = opsource_lines.begin(); it != opsource_lines.end(); ++it) {
                uint32_t parsed_line_number;
                std::string parsed_filename;
                bool found_line = GetLineAndFilename(*it, &parsed_line_number, parsed_filename);
                if (!found_line) continue;

                bool found_filename = parsed_filename.size() > 0;
                if (found_filename) {
                    current_filename = parsed_filename;
                }
                if ((!found_filename) || (current_filename == reported_filename)) {
                    // Update the candidate best line directive, if the current one is prior and closer to the reported line
                    if (reported_line_number >= parsed_line_number) {
                        if (!found_best_line ||
                            (reported_line_number - parsed_line_number <= reported_line_number - saved_line_number)) {
                            saved_line_number = parsed_line_number;
                            saved_opsource_offset = std::distance(opsource_lines.begin(), it);
                            found_best_line = true;
                        }
                    }
                }
            }
            if (found_best_line) {
                assert(reported_line_number >= saved_line_number);
                std::vector<std::string>::size_type opsource_index =
                    (reported_line_number - saved_line_number) + 1 + saved_opsource_offset;
                if (opsource_index < opsource_lines.size()) {
                    source_stream << "\n" << reported_line_number << ": " << opsource_lines[opsource_index].c_str();
                } else {
                    source_stream << "Internal error: calculated source line of " << opsource_index << " for source size of "
                                  << opsource_lines.size() << " lines.";
                }
            } else {
                source_stream << "Unable to find suitable #line directive in SPIR-V OpSource.";
            }
        } else {
            source_stream << "Unable to find SPIR-V OpSource.";
        }
    }
    source_msg = source_stream.str();
}

// Pull together all the information from the debug record to build the error message strings,
// and then assemble them into a single message string.
// Retrieve the shader program referenced by the unique shader ID provided in the debug record.
// We had to keep a copy of the shader program with the same lifecycle as the pipeline to make
// sure it is available when the pipeline is submitted.  (The ShaderModule tracking object also
// keeps a copy, but it can be destroyed after the pipeline is created and before it is submitted.)
//
static void AnalyzeAndReportError(const layer_data *dev_data, GLOBAL_CB_NODE *cb_node, VkQueue queue, uint32_t draw_index,
                                  uint32_t *const debug_output_buffer) {
    using namespace spvtools;
    const uint32_t total_words = debug_output_buffer[0];
    // A zero here means that the shader instrumentation didn't write anything.
    // If you have nothing to say, don't say it here.
    if (0 == total_words) {
        return;
    }
    // The first word in the debug output buffer is the number of words that would have
    // been written by the shader instrumentation, if there was enough room in the buffer we provided.
    // The number of words actually written by the shaders is determined by the size of the buffer
    // we provide via the descriptor.  So, we process only the number of words that can fit in the
    // buffer.
    // Each "report" written by the shader instrumentation is considered a "record".  This function
    // is hard-coded to process only one record because it expects the buffer to be large enough to
    // hold only one record.  If there is a desire to process more than one record, this function needs
    // to be modified to loop over records and the buffer size increased.
    auto gpu_state = GetGpuValidationState(dev_data);
    std::string validation_message;
    std::string stage_message;
    std::string common_message;
    std::string filename_message;
    std::string source_message;
    std::string vuid_msg;
    VkShaderModule shader_module_handle = VK_NULL_HANDLE;
    VkPipeline pipeline_handle = VK_NULL_HANDLE;
    std::vector<unsigned int> pgm;
    // The first record starts at this offset after the total_words.
    const uint32_t *debug_record = &debug_output_buffer[kDebugOutputDataOffset];
    // Lookup the VkShaderModule handle and SPIR-V code used to create the shader, using the unique shader ID value returned
    // by the instrumented shader.
    auto it = gpu_state->shader_map.find(debug_record[kInstCommonOutShaderId]);
    if (it != gpu_state->shader_map.end()) {
        shader_module_handle = it->second.shader_module;
        pipeline_handle = it->second.pipeline;
        pgm = it->second.pgm;
    }
    GenerateValidationMessage(debug_record, validation_message, vuid_msg);
    GenerateStageMessage(debug_record, stage_message);
    GenerateCommonMessage(dev_data, cb_node, debug_record, shader_module_handle, pipeline_handle, draw_index, common_message);
    GenerateSourceMessages(pgm, debug_record, filename_message, source_message);
    log_msg(GetReportData(dev_data), VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_QUEUE_EXT, HandleToUint64(queue),
            vuid_msg.c_str(), "%s %s %s %s%s", validation_message.c_str(), common_message.c_str(), stage_message.c_str(),
            filename_message.c_str(), source_message.c_str());
    // The debug record at word kInstCommonOutSize is the number of words in the record
    // written by the shader.  Clear the entire record plus the total_words word at the start.
    const uint32_t words_to_clear = 1 + std::min(debug_record[kInstCommonOutSize], (uint32_t)kInstMaxOutCnt);
    memset(debug_output_buffer, 0, sizeof(uint32_t) * words_to_clear);
}

// For the given command buffer, map its debug data buffer and read its contents for analysis.
static void ProcessInstrumentationBuffer(const layer_data *dev_data, VkQueue queue, GLOBAL_CB_NODE *cb_node) {
    auto gpu_state = GetGpuValidationState(dev_data);
    if (cb_node && cb_node->hasDrawCmd && cb_node->gpu_buffer_list.size() > 0) {
        VkResult result;
        char *pData;
        for (auto &buffer_info : cb_node->gpu_buffer_list) {
            uint32_t block_offset = buffer_info.mem_block.offset;
            uint32_t block_size = gpu_state->memory_manager->GetBlockSize();
            uint32_t offset_to_data = 0;
            uint32_t draw_index = 0;
            const uint32_t map_align = std::max(1U, static_cast<uint32_t>(GetPDProperties(dev_data)->limits.minMemoryMapAlignment));

            // Adjust the offset to the alignment required for mapping.
            block_offset = (block_offset / map_align) * map_align;
            offset_to_data = buffer_info.mem_block.offset - block_offset;
            block_size += offset_to_data;
            result = GetDispatchTable(dev_data)->MapMemory(cb_node->device, buffer_info.mem_block.memory, block_offset, block_size,
                                                           0, (void **)&pData);
            // Analyze debug output buffer
            if (result == VK_SUCCESS) {
                AnalyzeAndReportError(dev_data, cb_node, queue, draw_index, (uint32_t *)(pData + offset_to_data));
                GetDispatchTable(dev_data)->UnmapMemory(cb_node->device, buffer_info.mem_block.memory);
            }
            draw_index++;
        }
    }
}

// Submit a memory barrier on graphics queues.
// Lazy-create and record the needed command buffer.
static void SubmitBarrier(layer_data *dev_data, VkQueue queue) {
    auto gpu_state = GetGpuValidationState(dev_data);
    const auto *dispatch_table = GetDispatchTable(dev_data);
    uint32_t queue_family_index = 0;

    auto it = dev_data->queueMap.find(queue);
    if (it != dev_data->queueMap.end()) {
        queue_family_index = it->second.queueFamilyIndex;
    }

    // Pay attention only to queues that support graphics.
    // This ensures that the command buffer pool is created so that it can be used on a graphics queue.
    VkQueueFlags queue_flags = GetPhysicalDeviceState(dev_data)->queue_family_properties[queue_family_index].queueFlags;
    if (!(queue_flags & VK_QUEUE_GRAPHICS_BIT)) {
        return;
    }

    // Lazy-allocate and record the command buffer.
    if (gpu_state->barrier_command_buffer == VK_NULL_HANDLE) {
        VkResult result;
        VkCommandPoolCreateInfo pool_create_info = {};
        pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        pool_create_info.queueFamilyIndex = queue_family_index;
        result =
            dispatch_table->CreateCommandPool(GetDevice(dev_data), &pool_create_info, nullptr, &gpu_state->barrier_command_pool);
        if (result != VK_SUCCESS) {
            ReportSetupProblem(dev_data, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(GetDevice(dev_data)),
                               "Unable to create command pool for barrier CB.");
            gpu_state->barrier_command_pool = VK_NULL_HANDLE;
            return;
        }

        VkCommandBufferAllocateInfo command_buffer_alloc_info = {};
        command_buffer_alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        command_buffer_alloc_info.commandPool = gpu_state->barrier_command_pool;
        command_buffer_alloc_info.commandBufferCount = 1;
        command_buffer_alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        result = dispatch_table->AllocateCommandBuffers(GetDevice(dev_data), &command_buffer_alloc_info,
                                                        &gpu_state->barrier_command_buffer);
        if (result != VK_SUCCESS) {
            ReportSetupProblem(dev_data, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(GetDevice(dev_data)),
                               "Unable to create barrier command buffer.");
            dispatch_table->DestroyCommandPool(GetDevice(dev_data), gpu_state->barrier_command_pool, nullptr);
            gpu_state->barrier_command_pool = VK_NULL_HANDLE;
            gpu_state->barrier_command_buffer = VK_NULL_HANDLE;
            return;
        }

        // Hook up command buffer dispatch
        *((const void **)gpu_state->barrier_command_buffer) = *(void **)(GetDevice(dev_data));

        // Record a global memory barrier to force availability of device memory operations to the host domain.
        VkCommandBufferBeginInfo command_buffer_begin_info = {};
        command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        result = dispatch_table->BeginCommandBuffer(gpu_state->barrier_command_buffer, &command_buffer_begin_info);

        if (result == VK_SUCCESS) {
            VkMemoryBarrier memory_barrier = {};
            memory_barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
            memory_barrier.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
            memory_barrier.dstAccessMask = VK_ACCESS_HOST_READ_BIT;

            dispatch_table->CmdPipelineBarrier(gpu_state->barrier_command_buffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                                               VK_PIPELINE_STAGE_HOST_BIT, 0, 1, &memory_barrier, 0, nullptr, 0, nullptr);
            dispatch_table->EndCommandBuffer(gpu_state->barrier_command_buffer);
        }
    }

    if (gpu_state->barrier_command_buffer) {
        VkSubmitInfo submit_info = {};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &gpu_state->barrier_command_buffer;
        dispatch_table->QueueSubmit(queue, 1, &submit_info, VK_NULL_HANDLE);
    }
}

// Issue a memory barrier to make GPU-written data available to host.
// Wait for the queue to complete execution.
// Check the debug buffers for all the command buffers that were submitted.
void GpuPostCallQueueSubmit(layer_data *dev_data, VkQueue queue, uint32_t submitCount, const VkSubmitInfo *pSubmits,
                            VkFence fence) {
    auto gpu_state = GetGpuValidationState(dev_data);
    if (gpu_state->aborted) return;

    SubmitBarrier(dev_data, queue);

    dev_data->dispatch_table.QueueWaitIdle(queue);

    for (uint32_t submit_idx = 0; submit_idx < submitCount; submit_idx++) {
        const VkSubmitInfo *submit = &pSubmits[submit_idx];
        for (uint32_t i = 0; i < submit->commandBufferCount; i++) {
            auto cb_node = GetCBNode(dev_data, submit->pCommandBuffers[i]);
            ProcessInstrumentationBuffer(dev_data, queue, cb_node);
            for (auto secondaryCmdBuffer : cb_node->linkedCommandBuffers) {
                ProcessInstrumentationBuffer(dev_data, queue, secondaryCmdBuffer);
            }
        }
    }
}

void GpuAllocateValidationResources(layer_data *dev_data, const VkCommandBuffer cmd_buffer, const VkPipelineBindPoint bind_point) {
    VkResult result;

    if (!(GetEnables(dev_data)->gpu_validation)) return;

    auto gpu_state = GetGpuValidationState(dev_data);
    if (gpu_state->aborted) return;

    std::vector<VkDescriptorSet> desc_sets;
    VkDescriptorPool desc_pool = VK_NULL_HANDLE;
    result = gpu_state->desc_set_manager->GetDescriptorSets(1, &desc_pool, &desc_sets);
    assert(result == VK_SUCCESS);
    if (result != VK_SUCCESS) {
        ReportSetupProblem(dev_data, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(GetDevice(dev_data)),
                           "Unable to allocate descriptor sets.  Device could become unstable.");
        gpu_state->aborted = true;
        return;
    }

    VkDescriptorBufferInfo desc_buffer_info = {};
    desc_buffer_info.range = gpu_state->memory_manager->GetBlockSize();

    auto cb_node = GetCBNode(dev_data, cmd_buffer);
    if (!cb_node) {
        ReportSetupProblem(dev_data, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(GetDevice(dev_data)),
                           "Unrecognized command buffer");
        gpu_state->aborted = true;
        return;
    }

    GpuDeviceMemoryBlock block = {};
    result = gpu_state->memory_manager->GetBlock(&block);
    if (result != VK_SUCCESS) {
        ReportSetupProblem(dev_data, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(GetDevice(dev_data)),
                           "Unable to allocate device memory.  Device could become unstable.");
        gpu_state->aborted = true;
        return;
    }

    // Record buffer and memory info in CB state tracking
    cb_node->gpu_buffer_list.emplace_back(block, desc_sets[0], desc_pool);

    // Write the descriptor
    desc_buffer_info.buffer = block.buffer;
    desc_buffer_info.offset = block.offset;

    VkWriteDescriptorSet desc_write = {};
    desc_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    desc_write.descriptorCount = 1;
    desc_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    desc_write.pBufferInfo = &desc_buffer_info;
    desc_write.dstSet = desc_sets[0];
    GetDispatchTable(dev_data)->UpdateDescriptorSets(GetDevice(dev_data), 1, &desc_write, 0, NULL);

    auto iter = cb_node->lastBound.find(VK_PIPELINE_BIND_POINT_GRAPHICS);  // find() allows read-only access to cb_state
    if (iter != cb_node->lastBound.end()) {
        auto pipeline_state = iter->second.pipeline_state;
        if (pipeline_state && (pipeline_state->pipeline_layout.set_layouts.size() <= gpu_state->desc_set_bind_index)) {
            GetDispatchTable(dev_data)->CmdBindDescriptorSets(
                cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_state->pipeline_layout.layout, gpu_state->desc_set_bind_index,
                1, &cb_node->gpu_buffer_list[0].desc_set, 0, nullptr);
        }
    } else {
        ReportSetupProblem(dev_data, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(GetDevice(dev_data)),
                           "Unable to find pipeline state");
        gpu_state->aborted = true;
        return;
    }
}
