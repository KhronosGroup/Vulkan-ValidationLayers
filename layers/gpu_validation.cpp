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

#include "chassis.h"
#include "core_validation.h"
// This define indicates to build the VMA routines themselves
#define VMA_IMPLEMENTATION
// This define indicates that we will supply Vulkan function pointers at initialization
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#include "gpu_validation.h"
#include "shader_validation.h"
#include "spirv-tools/libspirv.h"
#include "spirv-tools/optimizer.hpp"
#include "spirv-tools/instrument.hpp"
#include <SPIRV/spirv.hpp>
#include <algorithm>
#include <regex>

// This is the number of bindings in the debug descriptor set.
static const uint32_t kNumBindingsInSet = 2;

static const VkShaderStageFlags kShaderStageAllRayTracing =
    VK_SHADER_STAGE_ANY_HIT_BIT_NV | VK_SHADER_STAGE_CALLABLE_BIT_NV | VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV |
    VK_SHADER_STAGE_INTERSECTION_BIT_NV | VK_SHADER_STAGE_MISS_BIT_NV | VK_SHADER_STAGE_RAYGEN_BIT_NV;

// Implementation for Descriptor Set Manager class
GpuDescriptorSetManager::GpuDescriptorSetManager(CoreChecks *dev_data) { dev_data_ = dev_data; }

GpuDescriptorSetManager::~GpuDescriptorSetManager() {
    for (auto &pool : desc_pool_map_) {
        DispatchDestroyDescriptorPool(dev_data_->device, pool.first, NULL);
    }
    desc_pool_map_.clear();
}

VkResult GpuDescriptorSetManager::GetDescriptorSets(uint32_t count, VkDescriptorPool *pool,
                                                    std::vector<VkDescriptorSet> *desc_sets) {
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
        result = DispatchCreateDescriptorPool(dev_data_->device, &desc_pool_info, NULL, &pool_to_use);
        assert(result == VK_SUCCESS);
        if (result != VK_SUCCESS) {
            return result;
        }
        desc_pool_map_[pool_to_use].size = desc_pool_info.maxSets;
        desc_pool_map_[pool_to_use].used = 0;
    }
    std::vector<VkDescriptorSetLayout> desc_layouts(count, dev_data_->gpu_validation_state->debug_desc_layout);

    VkDescriptorSetAllocateInfo alloc_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO, NULL, pool_to_use, count,
                                              desc_layouts.data()};

    result = DispatchAllocateDescriptorSets(dev_data_->device, &alloc_info, desc_sets->data());
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
        VkResult result = DispatchFreeDescriptorSets(dev_data_->device, desc_pool, 1, &desc_set);
        assert(result == VK_SUCCESS);
        if (result != VK_SUCCESS) {
            return;
        }
        desc_pool_map_[desc_pool].used--;
        if (0 == desc_pool_map_[desc_pool].used) {
            DispatchDestroyDescriptorPool(dev_data_->device, desc_pool, NULL);
            desc_pool_map_.erase(desc_pool);
        }
    }
    return;
}

// Trampolines to make VMA call Dispatch for Vulkan calls
static VKAPI_ATTR void VKAPI_CALL gpuVkGetPhysicalDeviceProperties(VkPhysicalDevice physicalDevice,
                                                                   VkPhysicalDeviceProperties *pProperties) {
    DispatchGetPhysicalDeviceProperties(physicalDevice, pProperties);
}
static VKAPI_ATTR void VKAPI_CALL gpuVkGetPhysicalDeviceMemoryProperties(VkPhysicalDevice physicalDevice,
                                                                         VkPhysicalDeviceMemoryProperties *pMemoryProperties) {
    DispatchGetPhysicalDeviceMemoryProperties(physicalDevice, pMemoryProperties);
}
static VKAPI_ATTR VkResult VKAPI_CALL gpuVkAllocateMemory(VkDevice device, const VkMemoryAllocateInfo *pAllocateInfo,
                                                          const VkAllocationCallbacks *pAllocator, VkDeviceMemory *pMemory) {
    return DispatchAllocateMemory(device, pAllocateInfo, pAllocator, pMemory);
}
static VKAPI_ATTR void VKAPI_CALL gpuVkFreeMemory(VkDevice device, VkDeviceMemory memory, const VkAllocationCallbacks *pAllocator) {
    DispatchFreeMemory(device, memory, pAllocator);
}
static VKAPI_ATTR VkResult VKAPI_CALL gpuVkMapMemory(VkDevice device, VkDeviceMemory memory, VkDeviceSize offset, VkDeviceSize size,
                                                     VkMemoryMapFlags flags, void **ppData) {
    return DispatchMapMemory(device, memory, offset, size, flags, ppData);
}
static VKAPI_ATTR void VKAPI_CALL gpuVkUnmapMemory(VkDevice device, VkDeviceMemory memory) { DispatchUnmapMemory(device, memory); }
static VKAPI_ATTR VkResult VKAPI_CALL gpuVkFlushMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount,
                                                                   const VkMappedMemoryRange *pMemoryRanges) {
    return DispatchFlushMappedMemoryRanges(device, memoryRangeCount, pMemoryRanges);
}
static VKAPI_ATTR VkResult VKAPI_CALL gpuVkInvalidateMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount,
                                                                        const VkMappedMemoryRange *pMemoryRanges) {
    return DispatchInvalidateMappedMemoryRanges(device, memoryRangeCount, pMemoryRanges);
}
static VKAPI_ATTR VkResult VKAPI_CALL gpuVkBindBufferMemory(VkDevice device, VkBuffer buffer, VkDeviceMemory memory,
                                                            VkDeviceSize memoryOffset) {
    return DispatchBindBufferMemory(device, buffer, memory, memoryOffset);
}
static VKAPI_ATTR VkResult VKAPI_CALL gpuVkBindImageMemory(VkDevice device, VkImage image, VkDeviceMemory memory,
                                                           VkDeviceSize memoryOffset) {
    return DispatchBindImageMemory(device, image, memory, memoryOffset);
}
static VKAPI_ATTR void VKAPI_CALL gpuVkGetBufferMemoryRequirements(VkDevice device, VkBuffer buffer,
                                                                   VkMemoryRequirements *pMemoryRequirements) {
    DispatchGetBufferMemoryRequirements(device, buffer, pMemoryRequirements);
}
static VKAPI_ATTR void VKAPI_CALL gpuVkGetImageMemoryRequirements(VkDevice device, VkImage image,
                                                                  VkMemoryRequirements *pMemoryRequirements) {
    DispatchGetImageMemoryRequirements(device, image, pMemoryRequirements);
}
static VKAPI_ATTR VkResult VKAPI_CALL gpuVkCreateBuffer(VkDevice device, const VkBufferCreateInfo *pCreateInfo,
                                                        const VkAllocationCallbacks *pAllocator, VkBuffer *pBuffer) {
    return DispatchCreateBuffer(device, pCreateInfo, pAllocator, pBuffer);
}
static VKAPI_ATTR void VKAPI_CALL gpuVkDestroyBuffer(VkDevice device, VkBuffer buffer, const VkAllocationCallbacks *pAllocator) {
    return DispatchDestroyBuffer(device, buffer, pAllocator);
}
static VKAPI_ATTR VkResult VKAPI_CALL gpuVkCreateImage(VkDevice device, const VkImageCreateInfo *pCreateInfo,
                                                       const VkAllocationCallbacks *pAllocator, VkImage *pImage) {
    return DispatchCreateImage(device, pCreateInfo, pAllocator, pImage);
}
static VKAPI_ATTR void VKAPI_CALL gpuVkDestroyImage(VkDevice device, VkImage image, const VkAllocationCallbacks *pAllocator) {
    DispatchDestroyImage(device, image, pAllocator);
}
static VKAPI_ATTR void VKAPI_CALL gpuVkCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer,
                                                     uint32_t regionCount, const VkBufferCopy *pRegions) {
    DispatchCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, regionCount, pRegions);
}

VkResult CoreChecks::GpuInitializeVma() {
    VmaVulkanFunctions functions;
    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.device = device;
    ValidationObject *device_object = GetLayerDataPtr(get_dispatch_key(allocatorInfo.device), layer_data_map);
    ValidationObject *validation_data =
        ValidationObject::GetValidationObject(device_object->object_dispatch, LayerObjectTypeCoreValidation);
    CoreChecks *core_checks = static_cast<CoreChecks *>(validation_data);
    allocatorInfo.physicalDevice = core_checks->physical_device;

    functions.vkGetPhysicalDeviceProperties = (PFN_vkGetPhysicalDeviceProperties)gpuVkGetPhysicalDeviceProperties;
    functions.vkGetPhysicalDeviceMemoryProperties = (PFN_vkGetPhysicalDeviceMemoryProperties)gpuVkGetPhysicalDeviceMemoryProperties;
    functions.vkAllocateMemory = (PFN_vkAllocateMemory)gpuVkAllocateMemory;
    functions.vkFreeMemory = (PFN_vkFreeMemory)gpuVkFreeMemory;
    functions.vkMapMemory = (PFN_vkMapMemory)gpuVkMapMemory;
    functions.vkUnmapMemory = (PFN_vkUnmapMemory)gpuVkUnmapMemory;
    functions.vkFlushMappedMemoryRanges = (PFN_vkFlushMappedMemoryRanges)gpuVkFlushMappedMemoryRanges;
    functions.vkInvalidateMappedMemoryRanges = (PFN_vkInvalidateMappedMemoryRanges)gpuVkInvalidateMappedMemoryRanges;
    functions.vkBindBufferMemory = (PFN_vkBindBufferMemory)gpuVkBindBufferMemory;
    functions.vkBindImageMemory = (PFN_vkBindImageMemory)gpuVkBindImageMemory;
    functions.vkGetBufferMemoryRequirements = (PFN_vkGetBufferMemoryRequirements)gpuVkGetBufferMemoryRequirements;
    functions.vkGetImageMemoryRequirements = (PFN_vkGetImageMemoryRequirements)gpuVkGetImageMemoryRequirements;
    functions.vkCreateBuffer = (PFN_vkCreateBuffer)gpuVkCreateBuffer;
    functions.vkDestroyBuffer = (PFN_vkDestroyBuffer)gpuVkDestroyBuffer;
    functions.vkCreateImage = (PFN_vkCreateImage)gpuVkCreateImage;
    functions.vkDestroyImage = (PFN_vkDestroyImage)gpuVkDestroyImage;
    functions.vkCmdCopyBuffer = (PFN_vkCmdCopyBuffer)gpuVkCmdCopyBuffer;
    allocatorInfo.pVulkanFunctions = &functions;

    return vmaCreateAllocator(&allocatorInfo, &gpu_validation_state->vmaAllocator);
}

// Convenience function for reporting problems with setting up GPU Validation.
void CoreChecks::ReportSetupProblem(VkDebugReportObjectTypeEXT object_type, uint64_t object_handle,
                                    const char *const specific_message) {
    log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, object_type, object_handle, "UNASSIGNED-GPU-Assisted Validation Error. ",
            "Detail: (%s)", specific_message);
}

// Turn on necessary device features.
void CoreChecks::GpuPreCallRecordCreateDevice(VkPhysicalDevice gpu, safe_VkDeviceCreateInfo *modified_create_info,
                                              VkPhysicalDeviceFeatures *supported_features) {
    if (supported_features->fragmentStoresAndAtomics || supported_features->vertexPipelineStoresAndAtomics) {
        VkPhysicalDeviceFeatures *features = nullptr;
        if (modified_create_info->pEnabledFeatures) {
            // If pEnabledFeatures, VkPhysicalDeviceFeatures2 in pNext chain is not allowed
            features = const_cast<VkPhysicalDeviceFeatures *>(modified_create_info->pEnabledFeatures);
        } else {
            VkPhysicalDeviceFeatures2 *features2 = nullptr;
            features2 =
                const_cast<VkPhysicalDeviceFeatures2 *>(lvl_find_in_chain<VkPhysicalDeviceFeatures2>(modified_create_info->pNext));
            if (features2) features = &features2->features;
        }
        if (features) {
            features->fragmentStoresAndAtomics = supported_features->fragmentStoresAndAtomics;
            features->vertexPipelineStoresAndAtomics = supported_features->vertexPipelineStoresAndAtomics;
        } else {
            VkPhysicalDeviceFeatures new_features = {};
            new_features.fragmentStoresAndAtomics = supported_features->fragmentStoresAndAtomics;
            new_features.vertexPipelineStoresAndAtomics = supported_features->vertexPipelineStoresAndAtomics;
            delete modified_create_info->pEnabledFeatures;
            modified_create_info->pEnabledFeatures = new VkPhysicalDeviceFeatures(new_features);
        }
    }
}

// Perform initializations that can be done at Create Device time.
void CoreChecks::GpuPostCallRecordCreateDevice(const CHECK_ENABLED *enables, const VkDeviceCreateInfo *pCreateInfo) {
    // Set instance-level enables in device-enable data structure if using legacy settings
    enabled.gpu_validation = enables->gpu_validation;
    enabled.gpu_validation_reserve_binding_slot = enables->gpu_validation_reserve_binding_slot;

    gpu_validation_state = std::unique_ptr<GpuValidationState>(new GpuValidationState);
    gpu_validation_state->reserve_binding_slot = enables->gpu_validation_reserve_binding_slot;

    if (phys_dev_props.apiVersion < VK_API_VERSION_1_1) {
        ReportSetupProblem(VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(device),
                           "GPU-Assisted validation requires Vulkan 1.1 or later.  GPU-Assisted Validation disabled.");
        gpu_validation_state->aborted = true;
        return;
    }

    // If api version 1.1 or later, SetDeviceLoaderData will be in the loader
    auto chain_info = get_chain_info(pCreateInfo, VK_LOADER_DATA_CALLBACK);
    assert(chain_info->u.pfnSetDeviceLoaderData);
    gpu_validation_state->vkSetDeviceLoaderData = chain_info->u.pfnSetDeviceLoaderData;

    // Some devices have extremely high limits here, so set a reasonable max because we have to pad
    // the pipeline layout with dummy descriptor set layouts.
    gpu_validation_state->adjusted_max_desc_sets = phys_dev_props.limits.maxBoundDescriptorSets;
    gpu_validation_state->adjusted_max_desc_sets = std::min(33U, gpu_validation_state->adjusted_max_desc_sets);

    // We can't do anything if there is only one.
    // Device probably not a legit Vulkan device, since there should be at least 4. Protect ourselves.
    if (gpu_validation_state->adjusted_max_desc_sets == 1) {
        ReportSetupProblem(VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(device),
                           "Device can bind only a single descriptor set.  GPU-Assisted Validation disabled.");
        gpu_validation_state->aborted = true;
        return;
    }
    gpu_validation_state->desc_set_bind_index = gpu_validation_state->adjusted_max_desc_sets - 1;
    log_msg(report_data, VK_DEBUG_REPORT_INFORMATION_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(device),
            "UNASSIGNED-GPU-Assisted Validation. ", "Shaders using descriptor set at index %d. ",
            gpu_validation_state->desc_set_bind_index);

    gpu_validation_state->output_buffer_size = sizeof(uint32_t) * (spvtools::kInstMaxOutCnt + 1);
    VkResult result = GpuInitializeVma();
    assert(result == VK_SUCCESS);
    std::unique_ptr<GpuDescriptorSetManager> desc_set_manager(new GpuDescriptorSetManager(this));

    // The descriptor indexing checks require only the first "output" binding.
    const VkDescriptorSetLayoutBinding debug_desc_layout_bindings[kNumBindingsInSet] = {
        {
            0,  // output
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            1,
            VK_SHADER_STAGE_ALL_GRAPHICS | VK_SHADER_STAGE_COMPUTE_BIT | kShaderStageAllRayTracing,
            NULL,
        },
        {
            1,  // input
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            1,
            VK_SHADER_STAGE_ALL_GRAPHICS | VK_SHADER_STAGE_COMPUTE_BIT | kShaderStageAllRayTracing,
            NULL,
        },
    };

    const VkDescriptorSetLayoutCreateInfo debug_desc_layout_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO, NULL, 0,
                                                                    kNumBindingsInSet, debug_desc_layout_bindings};

    const VkDescriptorSetLayoutCreateInfo dummy_desc_layout_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO, NULL, 0, 0,
                                                                    NULL};

    result = DispatchCreateDescriptorSetLayout(device, &debug_desc_layout_info, NULL, &gpu_validation_state->debug_desc_layout);

    // This is a layout used to "pad" a pipeline layout to fill in any gaps to the selected bind index.
    VkResult result2 =
        DispatchCreateDescriptorSetLayout(device, &dummy_desc_layout_info, NULL, &gpu_validation_state->dummy_desc_layout);
    assert((result == VK_SUCCESS) && (result2 == VK_SUCCESS));
    if ((result != VK_SUCCESS) || (result2 != VK_SUCCESS)) {
        ReportSetupProblem(VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(device),
                           "Unable to create descriptor set layout.  GPU-Assisted Validation disabled.");
        if (result == VK_SUCCESS) {
            DispatchDestroyDescriptorSetLayout(device, gpu_validation_state->debug_desc_layout, NULL);
        }
        if (result2 == VK_SUCCESS) {
            DispatchDestroyDescriptorSetLayout(device, gpu_validation_state->dummy_desc_layout, NULL);
        }
        gpu_validation_state->debug_desc_layout = VK_NULL_HANDLE;
        gpu_validation_state->dummy_desc_layout = VK_NULL_HANDLE;
        gpu_validation_state->aborted = true;
        return;
    }
    gpu_validation_state->desc_set_manager = std::move(desc_set_manager);
}

// Clean up device-related resources
void CoreChecks::GpuPreCallRecordDestroyDevice() {
    for (auto &queue_barrier_command_info_kv : gpu_validation_state->queue_barrier_command_infos) {
        GpuQueueBarrierCommandInfo &queue_barrier_command_info = queue_barrier_command_info_kv.second;

        DispatchFreeCommandBuffers(device, queue_barrier_command_info.barrier_command_pool, 1,
                                   &queue_barrier_command_info.barrier_command_buffer);
        queue_barrier_command_info.barrier_command_buffer = VK_NULL_HANDLE;

        DispatchDestroyCommandPool(device, queue_barrier_command_info.barrier_command_pool, NULL);
        queue_barrier_command_info.barrier_command_pool = VK_NULL_HANDLE;
    }
    gpu_validation_state->queue_barrier_command_infos.clear();
    if (gpu_validation_state->debug_desc_layout) {
        DispatchDestroyDescriptorSetLayout(device, gpu_validation_state->debug_desc_layout, NULL);
        gpu_validation_state->debug_desc_layout = VK_NULL_HANDLE;
    }
    if (gpu_validation_state->dummy_desc_layout) {
        DispatchDestroyDescriptorSetLayout(device, gpu_validation_state->dummy_desc_layout, NULL);
        gpu_validation_state->dummy_desc_layout = VK_NULL_HANDLE;
    }
    gpu_validation_state->desc_set_manager.reset();
    if (gpu_validation_state->vmaAllocator) {
        vmaDestroyAllocator(gpu_validation_state->vmaAllocator);
    }
}

// Modify the pipeline layout to include our debug descriptor set and any needed padding with the dummy descriptor set.
bool CoreChecks::GpuPreCallCreatePipelineLayout(const VkPipelineLayoutCreateInfo *pCreateInfo,
                                                const VkAllocationCallbacks *pAllocator, VkPipelineLayout *pPipelineLayout,
                                                std::vector<VkDescriptorSetLayout> *new_layouts,
                                                VkPipelineLayoutCreateInfo *modified_create_info) {
    if (gpu_validation_state->aborted) {
        return false;
    }

    if (modified_create_info->setLayoutCount >= gpu_validation_state->adjusted_max_desc_sets) {
        std::ostringstream strm;
        strm << "Pipeline Layout conflict with validation's descriptor set at slot " << gpu_validation_state->desc_set_bind_index
             << ". "
             << "Application has too many descriptor sets in the pipeline layout to continue with gpu validation. "
             << "Validation is not modifying the pipeline layout. "
             << "Instrumented shaders are replaced with non-instrumented shaders.";
        ReportSetupProblem(VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(device), strm.str().c_str());
    } else {
        // Modify the pipeline layout by:
        // 1. Copying the caller's descriptor set desc_layouts
        // 2. Fill in dummy descriptor layouts up to the max binding
        // 3. Fill in with the debug descriptor layout at the max binding slot
        new_layouts->reserve(gpu_validation_state->adjusted_max_desc_sets);
        new_layouts->insert(new_layouts->end(), &pCreateInfo->pSetLayouts[0],
                            &pCreateInfo->pSetLayouts[pCreateInfo->setLayoutCount]);
        for (uint32_t i = pCreateInfo->setLayoutCount; i < gpu_validation_state->adjusted_max_desc_sets - 1; ++i) {
            new_layouts->push_back(gpu_validation_state->dummy_desc_layout);
        }
        new_layouts->push_back(gpu_validation_state->debug_desc_layout);
        modified_create_info->pSetLayouts = new_layouts->data();
        modified_create_info->setLayoutCount = gpu_validation_state->adjusted_max_desc_sets;
    }
    return true;
}

// Clean up GPU validation after the CreatePipelineLayout call is made
void CoreChecks::GpuPostCallCreatePipelineLayout(VkResult result) {
    // Clean up GPU validation
    if (result != VK_SUCCESS) {
        ReportSetupProblem(VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(device),
                           "Unable to create pipeline layout.  Device could become unstable.");
        gpu_validation_state->aborted = true;
    }
}

// Free the device memory and descriptor set associated with a command buffer.
void CoreChecks::GpuResetCommandBuffer(const VkCommandBuffer commandBuffer) {
    if (gpu_validation_state->aborted) {
        return;
    }
    auto gpu_buffer_list = gpu_validation_state->GetGpuBufferInfo(commandBuffer);
    for (auto buffer_info : gpu_buffer_list) {
        vmaDestroyBuffer(gpu_validation_state->vmaAllocator, buffer_info.output_mem_block.buffer,
                         buffer_info.output_mem_block.allocation);
        if (buffer_info.input_mem_block.buffer) {
            vmaDestroyBuffer(gpu_validation_state->vmaAllocator, buffer_info.input_mem_block.buffer,
                             buffer_info.input_mem_block.allocation);
        }
        if (buffer_info.desc_set != VK_NULL_HANDLE) {
            gpu_validation_state->desc_set_manager->PutBackDescriptorSet(buffer_info.desc_pool, buffer_info.desc_set);
        }
    }
    gpu_validation_state->command_buffer_map.erase(commandBuffer);
}

// Just gives a warning about a possible deadlock.
void CoreChecks::GpuPreCallValidateCmdWaitEvents(VkPipelineStageFlags sourceStageMask) {
    if (sourceStageMask & VK_PIPELINE_STAGE_HOST_BIT) {
        ReportSetupProblem(VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(device),
                           "CmdWaitEvents recorded with VK_PIPELINE_STAGE_HOST_BIT set. "
                           "GPU_Assisted validation waits on queue completion. "
                           "This wait could block the host's signaling of this event, resulting in deadlock.");
    }
}

std::vector<safe_VkGraphicsPipelineCreateInfo> CoreChecks::GpuPreCallRecordCreateGraphicsPipelines(
    VkPipelineCache pipelineCache, uint32_t count, const VkGraphicsPipelineCreateInfo *pCreateInfos,
    const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines, std::vector<std::unique_ptr<PIPELINE_STATE>> &pipe_state) {
    std::vector<safe_VkGraphicsPipelineCreateInfo> new_pipeline_create_infos;
    GpuPreCallRecordPipelineCreations(count, pCreateInfos, pAllocator, pPipelines, pipe_state, &new_pipeline_create_infos,
                                      VK_PIPELINE_BIND_POINT_GRAPHICS);
    return new_pipeline_create_infos;
}
std::vector<safe_VkComputePipelineCreateInfo> CoreChecks::GpuPreCallRecordCreateComputePipelines(
    VkPipelineCache pipelineCache, uint32_t count, const VkComputePipelineCreateInfo *pCreateInfos,
    const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines, std::vector<std::unique_ptr<PIPELINE_STATE>> &pipe_state) {
    std::vector<safe_VkComputePipelineCreateInfo> new_pipeline_create_infos;
    GpuPreCallRecordPipelineCreations(count, pCreateInfos, pAllocator, pPipelines, pipe_state, &new_pipeline_create_infos,
                                      VK_PIPELINE_BIND_POINT_COMPUTE);
    return new_pipeline_create_infos;
}
std::vector<safe_VkRayTracingPipelineCreateInfoNV> CoreChecks::GpuPreCallRecordCreateRayTracingPipelinesNV(
    VkPipelineCache pipelineCache, uint32_t count, const VkRayTracingPipelineCreateInfoNV *pCreateInfos,
    const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines, std::vector<std::unique_ptr<PIPELINE_STATE>> &pipe_state) {
    std::vector<safe_VkRayTracingPipelineCreateInfoNV> new_pipeline_create_infos;
    GpuPreCallRecordPipelineCreations(count, pCreateInfos, pAllocator, pPipelines, pipe_state, &new_pipeline_create_infos,
                                      VK_PIPELINE_BIND_POINT_RAY_TRACING_NV);
    return new_pipeline_create_infos;
}
template <typename CreateInfo>
struct CreatePipelineTraits {};
template <>
struct CreatePipelineTraits<VkGraphicsPipelineCreateInfo> {
    using SafeType = safe_VkGraphicsPipelineCreateInfo;
    static const SafeType &GetPipelineCI(const PIPELINE_STATE *pipeline_state) { return pipeline_state->graphicsPipelineCI; }
    static uint32_t GetStageCount(const VkGraphicsPipelineCreateInfo &createInfo) { return createInfo.stageCount; }
    static VkShaderModule GetShaderModule(const VkGraphicsPipelineCreateInfo &createInfo, uint32_t stage) {
        return createInfo.pStages[stage].module;
    }
    static void SetShaderModule(SafeType *createInfo, VkShaderModule shader_module, uint32_t stage) {
        createInfo->pStages[stage].module = shader_module;
    }
};

template <>
struct CreatePipelineTraits<VkComputePipelineCreateInfo> {
    using SafeType = safe_VkComputePipelineCreateInfo;
    static const SafeType &GetPipelineCI(const PIPELINE_STATE *pipeline_state) { return pipeline_state->computePipelineCI; }
    static uint32_t GetStageCount(const VkComputePipelineCreateInfo &createInfo) { return 1; }
    static VkShaderModule GetShaderModule(const VkComputePipelineCreateInfo &createInfo, uint32_t stage) {
        return createInfo.stage.module;
    }
    static void SetShaderModule(SafeType *createInfo, VkShaderModule shader_module, uint32_t stage) {
        assert(stage == 0);
        createInfo->stage.module = shader_module;
    }
};
template <>
struct CreatePipelineTraits<VkRayTracingPipelineCreateInfoNV> {
    using SafeType = safe_VkRayTracingPipelineCreateInfoNV;
    static const SafeType &GetPipelineCI(const PIPELINE_STATE *pipeline_state) { return pipeline_state->raytracingPipelineCI; }
    static uint32_t GetStageCount(const VkRayTracingPipelineCreateInfoNV &createInfo) { return createInfo.stageCount; }
    static VkShaderModule GetShaderModule(const VkRayTracingPipelineCreateInfoNV &createInfo, uint32_t stage) {
        return createInfo.pStages[stage].module;
    }
    static void SetShaderModule(SafeType *createInfo, VkShaderModule shader_module, uint32_t stage) {
        createInfo->pStages[stage].module = shader_module;
    }
};

// Examine the pipelines to see if they use the debug descriptor set binding index.
// If any do, create new non-instrumented shader modules and use them to replace the instrumented
// shaders in the pipeline.  Return the (possibly) modified create infos to the caller.
template <typename CreateInfo, typename SafeCreateInfo>
void CoreChecks::GpuPreCallRecordPipelineCreations(uint32_t count, const CreateInfo *pCreateInfos,
                                                   const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                   std::vector<std::unique_ptr<PIPELINE_STATE>> &pipe_state,
                                                   std::vector<SafeCreateInfo> *new_pipeline_create_infos,
                                                   const VkPipelineBindPoint bind_point) {
    using Accessor = CreatePipelineTraits<CreateInfo>;
    if (bind_point != VK_PIPELINE_BIND_POINT_GRAPHICS && bind_point != VK_PIPELINE_BIND_POINT_COMPUTE &&
        bind_point != VK_PIPELINE_BIND_POINT_RAY_TRACING_NV) {
        return;
    }

    // Walk through all the pipelines, make a copy of each and flag each pipeline that contains a shader that uses the debug
    // descriptor set index.
    for (uint32_t pipeline = 0; pipeline < count; ++pipeline) {
        uint32_t stageCount = Accessor::GetStageCount(pCreateInfos[pipeline]);
        new_pipeline_create_infos->push_back(Accessor::GetPipelineCI(pipe_state[pipeline].get()));

        bool replace_shaders = false;
        if (pipe_state[pipeline]->active_slots.find(gpu_validation_state->desc_set_bind_index) !=
            pipe_state[pipeline]->active_slots.end()) {
            replace_shaders = true;
        }
        // If the app requests all available sets, the pipeline layout was not modified at pipeline layout creation and the already
        // instrumented shaders need to be replaced with uninstrumented shaders
        if (pipe_state[pipeline]->pipeline_layout.set_layouts.size() >= gpu_validation_state->adjusted_max_desc_sets) {
            replace_shaders = true;
        }

        if (replace_shaders) {
            for (uint32_t stage = 0; stage < stageCount; ++stage) {
                const SHADER_MODULE_STATE *shader = GetShaderModuleState(Accessor::GetShaderModule(pCreateInfos[pipeline], stage));

                VkShaderModuleCreateInfo create_info = {};
                VkShaderModule shader_module;
                create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
                create_info.pCode = shader->words.data();
                create_info.codeSize = shader->words.size() * sizeof(uint32_t);
                VkResult result = DispatchCreateShaderModule(device, &create_info, pAllocator, &shader_module);
                if (result == VK_SUCCESS) {
                    Accessor::SetShaderModule(new_pipeline_create_infos[pipeline].data(), shader_module, stage);
                } else {
                    uint64_t moduleHandle = HandleToUint64(Accessor::GetShaderModule(pCreateInfos[pipeline], stage));
                    ReportSetupProblem(VK_DEBUG_REPORT_OBJECT_TYPE_SHADER_MODULE_EXT, moduleHandle,
                                       "Unable to replace instrumented shader with non-instrumented one.  "
                                       "Device could become unstable.");
                }
            }
        }
    }
}

void CoreChecks::GpuPostCallRecordCreateGraphicsPipelines(const uint32_t count, const VkGraphicsPipelineCreateInfo *pCreateInfos,
                                                          const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines) {
    GpuPostCallRecordPipelineCreations(count, pCreateInfos, pAllocator, pPipelines, VK_PIPELINE_BIND_POINT_GRAPHICS);
}
void CoreChecks::GpuPostCallRecordCreateComputePipelines(const uint32_t count, const VkComputePipelineCreateInfo *pCreateInfos,
                                                         const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines) {
    GpuPostCallRecordPipelineCreations(count, pCreateInfos, pAllocator, pPipelines, VK_PIPELINE_BIND_POINT_COMPUTE);
}
void CoreChecks::GpuPostCallRecordCreateRayTracingPipelinesNV(const uint32_t count,
                                                              const VkRayTracingPipelineCreateInfoNV *pCreateInfos,
                                                              const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines) {
    GpuPostCallRecordPipelineCreations(count, pCreateInfos, pAllocator, pPipelines, VK_PIPELINE_BIND_POINT_RAY_TRACING_NV);
}

// For every pipeline:
// - For every shader in a pipeline:
//   - If the shader had to be replaced in PreCallRecord (because the pipeline is using the debug desc set index):
//     - Destroy it since it has been bound into the pipeline by now.  This is our only chance to delete it.
//   - Track the shader in the shader_map
//   - Save the shader binary if it contains debug code
template <typename CreateInfo>
void CoreChecks::GpuPostCallRecordPipelineCreations(const uint32_t count, const CreateInfo *pCreateInfos,
                                                    const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                    const VkPipelineBindPoint bind_point) {
    using Accessor = CreatePipelineTraits<CreateInfo>;
    if (bind_point != VK_PIPELINE_BIND_POINT_GRAPHICS && bind_point != VK_PIPELINE_BIND_POINT_COMPUTE &&
        bind_point != VK_PIPELINE_BIND_POINT_RAY_TRACING_NV) {
        return;
    }
    for (uint32_t pipeline = 0; pipeline < count; ++pipeline) {
        auto pipeline_state = ValidationStateTracker::GetPipelineState(pPipelines[pipeline]);
        if (nullptr == pipeline_state) continue;

        uint32_t stageCount = 0;
        if (bind_point == VK_PIPELINE_BIND_POINT_GRAPHICS) {
            stageCount = pipeline_state->graphicsPipelineCI.stageCount;
        } else if (bind_point == VK_PIPELINE_BIND_POINT_COMPUTE) {
            stageCount = 1;
        } else if (bind_point == VK_PIPELINE_BIND_POINT_RAY_TRACING_NV) {
            stageCount = pipeline_state->raytracingPipelineCI.stageCount;
        } else {
            assert(false);
        }

        for (uint32_t stage = 0; stage < stageCount; ++stage) {
            if (pipeline_state->active_slots.find(gpu_validation_state->desc_set_bind_index) !=
                pipeline_state->active_slots.end()) {
                DispatchDestroyShaderModule(device, Accessor::GetShaderModule(pCreateInfos[pipeline], stage), pAllocator);
            }

            const SHADER_MODULE_STATE *shader_state = nullptr;
            if (bind_point == VK_PIPELINE_BIND_POINT_GRAPHICS) {
                shader_state = GetShaderModuleState(pipeline_state->graphicsPipelineCI.pStages[stage].module);
            } else if (bind_point == VK_PIPELINE_BIND_POINT_COMPUTE) {
                assert(stage == 0);
                shader_state = GetShaderModuleState(pipeline_state->computePipelineCI.stage.module);
            } else if (bind_point == VK_PIPELINE_BIND_POINT_RAY_TRACING_NV) {
                shader_state = GetShaderModuleState(pipeline_state->raytracingPipelineCI.pStages[stage].module);
            } else {
                assert(false);
            }

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
            gpu_validation_state->shader_map[shader_state->gpu_validation_shader_id].pipeline = pipeline_state->pipeline;
            // Be careful to use the originally bound (instrumented) shader here, even if PreCallRecord had to back it
            // out with a non-instrumented shader.  The non-instrumented shader (found in pCreateInfo) was destroyed above.
            VkShaderModule shader_module = VK_NULL_HANDLE;
            if (bind_point == VK_PIPELINE_BIND_POINT_GRAPHICS) {
                shader_module = pipeline_state->graphicsPipelineCI.pStages[stage].module;
            } else if (bind_point == VK_PIPELINE_BIND_POINT_COMPUTE) {
                assert(stage == 0);
                shader_module = pipeline_state->computePipelineCI.stage.module;
            } else if (bind_point == VK_PIPELINE_BIND_POINT_RAY_TRACING_NV) {
                shader_module = pipeline_state->raytracingPipelineCI.pStages[stage].module;
            } else {
                assert(false);
            }
            gpu_validation_state->shader_map[shader_state->gpu_validation_shader_id].shader_module = shader_module;
            gpu_validation_state->shader_map[shader_state->gpu_validation_shader_id].pgm = std::move(code);
        }
    }
}

// Remove all the shader trackers associated with this destroyed pipeline.
void CoreChecks::GpuPreCallRecordDestroyPipeline(const VkPipeline pipeline) {
    for (auto it = gpu_validation_state->shader_map.begin(); it != gpu_validation_state->shader_map.end();) {
        if (it->second.pipeline == pipeline) {
            it = gpu_validation_state->shader_map.erase(it);
        } else {
            ++it;
        }
    }
}

// Call the SPIR-V Optimizer to run the instrumentation pass on the shader.
bool CoreChecks::GpuInstrumentShader(const VkShaderModuleCreateInfo *pCreateInfo, std::vector<unsigned int> &new_pgm,
                                     uint32_t *unique_shader_id) {
    if (gpu_validation_state->aborted) return false;
    if (pCreateInfo->pCode[0] != spv::MagicNumber) return false;

    // Load original shader SPIR-V
    uint32_t num_words = static_cast<uint32_t>(pCreateInfo->codeSize / 4);
    new_pgm.clear();
    new_pgm.reserve(num_words);
    new_pgm.insert(new_pgm.end(), &pCreateInfo->pCode[0], &pCreateInfo->pCode[num_words]);

    // Call the optimizer to instrument the shader.
    // Use the unique_shader_module_id as a shader ID so we can look up its handle later in the shader_map.
    // If descriptor indexing is enabled, enable length checks and updated descriptor checks
    const bool descriptor_indexing = device_extensions.vk_ext_descriptor_indexing;
    using namespace spvtools;
    spv_target_env target_env = SPV_ENV_VULKAN_1_1;
    Optimizer optimizer(target_env);
    optimizer.RegisterPass(CreateInstBindlessCheckPass(gpu_validation_state->desc_set_bind_index,
                                                       gpu_validation_state->unique_shader_module_id, descriptor_indexing,
                                                       descriptor_indexing));
    optimizer.RegisterPass(CreateAggressiveDCEPass());
    bool pass = optimizer.Run(new_pgm.data(), new_pgm.size(), &new_pgm);
    if (!pass) {
        ReportSetupProblem(VK_DEBUG_REPORT_OBJECT_TYPE_SHADER_MODULE_EXT, VK_NULL_HANDLE,
                           "Failure to instrument shader.  Proceeding with non-instrumented shader.");
    }
    *unique_shader_id = gpu_validation_state->unique_shader_module_id++;
    return pass;
}

// Create the instrumented shader data to provide to the driver.
bool CoreChecks::GpuPreCallCreateShaderModule(const VkShaderModuleCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator,
                                              VkShaderModule *pShaderModule, uint32_t *unique_shader_id,
                                              VkShaderModuleCreateInfo *instrumented_create_info,
                                              std::vector<unsigned int> *instrumented_pgm) {
    bool pass = GpuInstrumentShader(pCreateInfo, *instrumented_pgm, unique_shader_id);
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
        case spv::ExecutionModelVertex: {
            strm << "Stage = Vertex. Vertex Index = " << debug_record[kInstVertOutVertexIndex]
                 << " Instance Index = " << debug_record[kInstVertOutInstanceIndex] << ". ";
        } break;
        case spv::ExecutionModelTessellationControl: {
            strm << "Stage = Tessellation Control.  Invocation ID = " << debug_record[kInstTessOutInvocationId] << ". ";
        } break;
        case spv::ExecutionModelTessellationEvaluation: {
            strm << "Stage = Tessellation Eval.  Invocation ID = " << debug_record[kInstTessOutInvocationId] << ". ";
        } break;
        case spv::ExecutionModelGeometry: {
            strm << "Stage = Geometry.  Primitive ID = " << debug_record[kInstGeomOutPrimitiveId]
                 << " Invocation ID = " << debug_record[kInstGeomOutInvocationId] << ". ";
        } break;
        case spv::ExecutionModelFragment: {
            strm << "Stage = Fragment.  Fragment coord (x,y) = ("
                 << *reinterpret_cast<const float *>(&debug_record[kInstFragOutFragCoordX]) << ", "
                 << *reinterpret_cast<const float *>(&debug_record[kInstFragOutFragCoordY]) << "). ";
        } break;
        case spv::ExecutionModelGLCompute: {
            strm << "Stage = Compute.  Global invocation ID = " << debug_record[kInstCompOutGlobalInvocationId] << ". ";
        } break;
        case spv::ExecutionModelRayGenerationNV: {
            strm << "Stage = Ray Generation.  Global Launch ID (x,y,z) = (" << debug_record[kInstRayTracingOutLaunchIdX] << ", "
                 << debug_record[kInstRayTracingOutLaunchIdY] << ", " << debug_record[kInstRayTracingOutLaunchIdZ] << "). ";
        } break;
        case spv::ExecutionModelIntersectionNV: {
            strm << "Stage = Intersection.  Global Launch ID (x,y,z) = (" << debug_record[kInstRayTracingOutLaunchIdX] << ", "
                 << debug_record[kInstRayTracingOutLaunchIdY] << ", " << debug_record[kInstRayTracingOutLaunchIdZ] << "). ";
        } break;
        case spv::ExecutionModelAnyHitNV: {
            strm << "Stage = Any Hit.  Global Launch ID (x,y,z) = (" << debug_record[kInstRayTracingOutLaunchIdX] << ", "
                 << debug_record[kInstRayTracingOutLaunchIdY] << ", " << debug_record[kInstRayTracingOutLaunchIdZ] << "). ";
        } break;
        case spv::ExecutionModelClosestHitNV: {
            strm << "Stage = Closest Hit.  Global Launch ID (x,y,z) = (" << debug_record[kInstRayTracingOutLaunchIdX] << ", "
                 << debug_record[kInstRayTracingOutLaunchIdY] << ", " << debug_record[kInstRayTracingOutLaunchIdZ] << "). ";
        } break;
        case spv::ExecutionModelMissNV: {
            strm << "Stage = Miss.  Global Launch ID (x,y,z) = (" << debug_record[kInstRayTracingOutLaunchIdX] << ", "
                 << debug_record[kInstRayTracingOutLaunchIdY] << ", " << debug_record[kInstRayTracingOutLaunchIdZ] << "). ";
        } break;
        case spv::ExecutionModelCallableNV: {
            strm << "Stage = Callable.  Global Launch ID (x,y,z) = (" << debug_record[kInstRayTracingOutLaunchIdX] << ", "
                 << debug_record[kInstRayTracingOutLaunchIdY] << ", " << debug_record[kInstRayTracingOutLaunchIdZ] << "). ";
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

static std::string LookupDebugUtilsName(const debug_report_data *report_data, const uint64_t object) {
    auto object_label = report_data->DebugReportGetUtilsObjectName(object);
    if (object_label != "") {
        object_label = "(" + object_label + ")";
    }
    return object_label;
}

// Generate message from the common portion of the debug report record.
static void GenerateCommonMessage(const debug_report_data *report_data, const CMD_BUFFER_STATE *cb_node,
                                  const uint32_t *debug_record, const VkShaderModule shader_module_handle,
                                  const VkPipeline pipeline_handle, const VkPipelineBindPoint pipeline_bind_point,
                                  const uint32_t operation_index, std::string &msg) {
    using namespace spvtools;
    std::ostringstream strm;
    if (shader_module_handle == VK_NULL_HANDLE) {
        strm << std::hex << std::showbase << "Internal Error: Unable to locate information for shader used in command buffer "
             << LookupDebugUtilsName(report_data, HandleToUint64(cb_node->commandBuffer)) << "("
             << HandleToUint64(cb_node->commandBuffer) << "). ";
        assert(true);
    } else {
        strm << std::hex << std::showbase << "Command buffer "
             << LookupDebugUtilsName(report_data, HandleToUint64(cb_node->commandBuffer)) << "("
             << HandleToUint64(cb_node->commandBuffer) << "). ";
        if (pipeline_bind_point == VK_PIPELINE_BIND_POINT_GRAPHICS) {
            strm << "Draw ";
        } else if (pipeline_bind_point == VK_PIPELINE_BIND_POINT_COMPUTE) {
            strm << "Compute ";
        } else if (pipeline_bind_point == VK_PIPELINE_BIND_POINT_RAY_TRACING_NV) {
            strm << "Ray Trace ";
        } else {
            assert(false);
            strm << "Unknown Pipeline Operation ";
        }
        strm << "Index " << operation_index << ". "
             << "Pipeline " << LookupDebugUtilsName(report_data, HandleToUint64(pipeline_handle)) << "("
             << HandleToUint64(pipeline_handle) << "). "
             << "Shader Module " << LookupDebugUtilsName(report_data, HandleToUint64(shader_module_handle)) << "("
             << HandleToUint64(shader_module_handle) << "). ";
    }
    strm << std::dec << std::noshowbase;
    strm << "Shader Instruction Index = " << debug_record[kInstCommonOutInstructionIdx] << ". ";
    msg = strm.str();
}

// Read the contents of the SPIR-V OpSource instruction and any following continuation instructions.
// Split the single string into a vector of strings, one for each line, for easier processing.
static void ReadOpSource(const SHADER_MODULE_STATE &shader, const uint32_t reported_file_id,
                         std::vector<std::string> &opsource_lines) {
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
    SHADER_MODULE_STATE shader;
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
void CoreChecks::AnalyzeAndReportError(CMD_BUFFER_STATE *cb_node, VkQueue queue, VkPipelineBindPoint pipeline_bind_point,
                                       uint32_t operation_index, uint32_t *const debug_output_buffer) {
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
    auto it = gpu_validation_state->shader_map.find(debug_record[kInstCommonOutShaderId]);
    if (it != gpu_validation_state->shader_map.end()) {
        shader_module_handle = it->second.shader_module;
        pipeline_handle = it->second.pipeline;
        pgm = it->second.pgm;
    }
    GenerateValidationMessage(debug_record, validation_message, vuid_msg);
    GenerateStageMessage(debug_record, stage_message);
    GenerateCommonMessage(report_data, cb_node, debug_record, shader_module_handle, pipeline_handle, pipeline_bind_point,
                          operation_index, common_message);
    GenerateSourceMessages(pgm, debug_record, filename_message, source_message);
    log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_QUEUE_EXT, HandleToUint64(queue),
            vuid_msg.c_str(), "%s %s %s %s%s", validation_message.c_str(), common_message.c_str(), stage_message.c_str(),
            filename_message.c_str(), source_message.c_str());
    // The debug record at word kInstCommonOutSize is the number of words in the record
    // written by the shader.  Clear the entire record plus the total_words word at the start.
    const uint32_t words_to_clear = 1 + std::min(debug_record[kInstCommonOutSize], (uint32_t)kInstMaxOutCnt);
    memset(debug_output_buffer, 0, sizeof(uint32_t) * words_to_clear);
}

// For the given command buffer, map its debug data buffers and read their contents for analysis.
void CoreChecks::ProcessInstrumentationBuffer(VkQueue queue, CMD_BUFFER_STATE *cb_node) {
    auto gpu_buffer_list = gpu_validation_state->GetGpuBufferInfo(cb_node->commandBuffer);
    if (cb_node && (cb_node->hasDrawCmd || cb_node->hasTraceRaysCmd || cb_node->hasDispatchCmd) && gpu_buffer_list.size() > 0) {
        VkResult result;
        char *pData;
        uint32_t draw_index = 0;
        uint32_t compute_index = 0;
        uint32_t ray_trace_index = 0;

        for (auto &buffer_info : gpu_buffer_list) {
            result = vmaMapMemory(gpu_validation_state->vmaAllocator, buffer_info.output_mem_block.allocation, (void **)&pData);
            // Analyze debug output buffer
            if (result == VK_SUCCESS) {
                uint32_t operation_index = 0;
                if (buffer_info.pipeline_bind_point == VK_PIPELINE_BIND_POINT_GRAPHICS) {
                    operation_index = draw_index;
                } else if (buffer_info.pipeline_bind_point == VK_PIPELINE_BIND_POINT_COMPUTE) {
                    operation_index = compute_index;
                } else if (buffer_info.pipeline_bind_point == VK_PIPELINE_BIND_POINT_RAY_TRACING_NV) {
                    operation_index = ray_trace_index;
                } else {
                    assert(false);
                }

                AnalyzeAndReportError(cb_node, queue, buffer_info.pipeline_bind_point, operation_index, (uint32_t *)pData);
                vmaUnmapMemory(gpu_validation_state->vmaAllocator, buffer_info.output_mem_block.allocation);
            }

            if (buffer_info.pipeline_bind_point == VK_PIPELINE_BIND_POINT_GRAPHICS) {
                draw_index++;
            } else if (buffer_info.pipeline_bind_point == VK_PIPELINE_BIND_POINT_COMPUTE) {
                compute_index++;
            } else if (buffer_info.pipeline_bind_point == VK_PIPELINE_BIND_POINT_RAY_TRACING_NV) {
                ray_trace_index++;
            } else {
                assert(false);
            }
        }
    }
}

// For the given command buffer, map its debug data buffers and update the status of any update after bind descriptors
void CoreChecks::UpdateInstrumentationBuffer(CMD_BUFFER_STATE *cb_node) {
    auto gpu_buffer_list = gpu_validation_state->GetGpuBufferInfo(cb_node->commandBuffer);
    uint32_t *pData;
    for (auto &buffer_info : gpu_buffer_list) {
        if (buffer_info.input_mem_block.update_at_submit.size() > 0) {
            VkResult result =
                vmaMapMemory(gpu_validation_state->vmaAllocator, buffer_info.input_mem_block.allocation, (void **)&pData);
            if (result == VK_SUCCESS) {
                for (auto update : buffer_info.input_mem_block.update_at_submit) {
                    if (update.second->updated) pData[update.first] = 1;
                }
                vmaUnmapMemory(gpu_validation_state->vmaAllocator, buffer_info.input_mem_block.allocation);
            }
        }
    }
}

// Submit a memory barrier on graphics queues.
// Lazy-create and record the needed command buffer.
void CoreChecks::SubmitBarrier(VkQueue queue) {
    auto queue_barrier_command_info_it =
        gpu_validation_state->queue_barrier_command_infos.emplace(queue, GpuQueueBarrierCommandInfo{});
    if (queue_barrier_command_info_it.second) {
        GpuQueueBarrierCommandInfo &quere_barrier_command_info = queue_barrier_command_info_it.first->second;

        uint32_t queue_family_index = 0;

        auto queue_state_it = queueMap.find(queue);
        if (queue_state_it != queueMap.end()) {
            queue_family_index = queue_state_it->second.queueFamilyIndex;
        }

        VkResult result = VK_SUCCESS;

        VkCommandPoolCreateInfo pool_create_info = {};
        pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        pool_create_info.queueFamilyIndex = queue_family_index;
        result = DispatchCreateCommandPool(device, &pool_create_info, nullptr, &quere_barrier_command_info.barrier_command_pool);
        if (result != VK_SUCCESS) {
            ReportSetupProblem(VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(device),
                               "Unable to create command pool for barrier CB.");
            quere_barrier_command_info.barrier_command_pool = VK_NULL_HANDLE;
            return;
        }

        VkCommandBufferAllocateInfo buffer_alloc_info = {};
        buffer_alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        buffer_alloc_info.commandPool = quere_barrier_command_info.barrier_command_pool;
        buffer_alloc_info.commandBufferCount = 1;
        buffer_alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        result = DispatchAllocateCommandBuffers(device, &buffer_alloc_info, &quere_barrier_command_info.barrier_command_buffer);
        if (result != VK_SUCCESS) {
            ReportSetupProblem(VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(device),
                               "Unable to create barrier command buffer.");
            DispatchDestroyCommandPool(device, quere_barrier_command_info.barrier_command_pool, nullptr);
            quere_barrier_command_info.barrier_command_pool = VK_NULL_HANDLE;
            quere_barrier_command_info.barrier_command_buffer = VK_NULL_HANDLE;
            return;
        }

        // Hook up command buffer dispatch
        gpu_validation_state->vkSetDeviceLoaderData(device, quere_barrier_command_info.barrier_command_buffer);

        // Record a global memory barrier to force availability of device memory operations to the host domain.
        VkCommandBufferBeginInfo command_buffer_begin_info = {};
        command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        result = DispatchBeginCommandBuffer(quere_barrier_command_info.barrier_command_buffer, &command_buffer_begin_info);
        if (result == VK_SUCCESS) {
            VkMemoryBarrier memory_barrier = {};
            memory_barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
            memory_barrier.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
            memory_barrier.dstAccessMask = VK_ACCESS_HOST_READ_BIT;

            DispatchCmdPipelineBarrier(quere_barrier_command_info.barrier_command_buffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                                       VK_PIPELINE_STAGE_HOST_BIT, 0, 1, &memory_barrier, 0, nullptr, 0, nullptr);
            DispatchEndCommandBuffer(quere_barrier_command_info.barrier_command_buffer);
        }
    }

    GpuQueueBarrierCommandInfo &quere_barrier_command_info = queue_barrier_command_info_it.first->second;
    if (quere_barrier_command_info.barrier_command_buffer != VK_NULL_HANDLE) {
        VkSubmitInfo submit_info = {};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &quere_barrier_command_info.barrier_command_buffer;
        DispatchQueueSubmit(queue, 1, &submit_info, VK_NULL_HANDLE);
    }
}

void CoreChecks::GpuPreCallRecordQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo *pSubmits, VkFence fence) {
    for (uint32_t submit_idx = 0; submit_idx < submitCount; submit_idx++) {
        const VkSubmitInfo *submit = &pSubmits[submit_idx];
        for (uint32_t i = 0; i < submit->commandBufferCount; i++) {
            auto cb_node = GetCBState(submit->pCommandBuffers[i]);
            UpdateInstrumentationBuffer(cb_node);
            for (auto secondaryCmdBuffer : cb_node->linkedCommandBuffers) {
                UpdateInstrumentationBuffer(secondaryCmdBuffer);
            }
        }
    }
}

// Issue a memory barrier to make GPU-written data available to host.
// Wait for the queue to complete execution.
// Check the debug buffers for all the command buffers that were submitted.
void CoreChecks::GpuPostCallQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo *pSubmits, VkFence fence) {
    if (gpu_validation_state->aborted) return;

    SubmitBarrier(queue);

    DispatchQueueWaitIdle(queue);

    for (uint32_t submit_idx = 0; submit_idx < submitCount; submit_idx++) {
        const VkSubmitInfo *submit = &pSubmits[submit_idx];
        for (uint32_t i = 0; i < submit->commandBufferCount; i++) {
            auto cb_node = GetCBState(submit->pCommandBuffers[i]);
            ProcessInstrumentationBuffer(queue, cb_node);
            for (auto secondaryCmdBuffer : cb_node->linkedCommandBuffers) {
                ProcessInstrumentationBuffer(queue, secondaryCmdBuffer);
            }
        }
    }
}

void CoreChecks::GpuAllocateValidationResources(const VkCommandBuffer cmd_buffer, const VkPipelineBindPoint bind_point) {
    if (bind_point != VK_PIPELINE_BIND_POINT_GRAPHICS && bind_point != VK_PIPELINE_BIND_POINT_COMPUTE &&
        bind_point != VK_PIPELINE_BIND_POINT_RAY_TRACING_NV) {
        return;
    }
    VkResult result;

    if (!(enabled.gpu_validation)) return;

    if (gpu_validation_state->aborted) return;

    std::vector<VkDescriptorSet> desc_sets;
    VkDescriptorPool desc_pool = VK_NULL_HANDLE;
    result = gpu_validation_state->desc_set_manager->GetDescriptorSets(1, &desc_pool, &desc_sets);
    assert(result == VK_SUCCESS);
    if (result != VK_SUCCESS) {
        ReportSetupProblem(VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(device),
                           "Unable to allocate descriptor sets.  Device could become unstable.");
        gpu_validation_state->aborted = true;
        return;
    }

    VkDescriptorBufferInfo output_desc_buffer_info = {};
    output_desc_buffer_info.range = gpu_validation_state->output_buffer_size;

    auto cb_node = GetCBState(cmd_buffer);
    if (!cb_node) {
        ReportSetupProblem(VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(device), "Unrecognized command buffer");
        gpu_validation_state->aborted = true;
        return;
    }

    // Allocate memory for the output block that the gpu will use to return any error information
    GpuDeviceMemoryBlock output_block = {};
    VkBufferCreateInfo bufferInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    bufferInfo.size = gpu_validation_state->output_buffer_size;
    bufferInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = VMA_MEMORY_USAGE_GPU_TO_CPU;
    result = vmaCreateBuffer(gpu_validation_state->vmaAllocator, &bufferInfo, &allocInfo, &output_block.buffer,
                             &output_block.allocation, nullptr);
    if (result != VK_SUCCESS) {
        ReportSetupProblem(VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(device),
                           "Unable to allocate device memory.  Device could become unstable.");
        gpu_validation_state->aborted = true;
        return;
    }

    // Clear the output block to zeros so that only error information from the gpu will be present
    uint32_t *pData;
    result = vmaMapMemory(gpu_validation_state->vmaAllocator, output_block.allocation, (void **)&pData);
    if (result == VK_SUCCESS) {
        memset(pData, 0, gpu_validation_state->output_buffer_size);
        vmaUnmapMemory(gpu_validation_state->vmaAllocator, output_block.allocation);
    }

    GpuDeviceMemoryBlock input_block = {};
    VkWriteDescriptorSet desc_writes[2] = {};
    uint32_t desc_count = 1;
    auto const &state = cb_node->lastBound[bind_point];
    uint32_t number_of_sets = (uint32_t)state.per_set.size();

    // Figure out how much memory we need for the input block based on how many sets and bindings there are
    // and how big each of the bindings is
    if (number_of_sets > 0 && device_extensions.vk_ext_descriptor_indexing) {
        uint32_t descriptor_count = 0;  // Number of descriptors, including all array elements
        uint32_t binding_count = 0;     // Number of bindings based on the max binding number used
        for (auto s : state.per_set) {
            auto desc = s.bound_descriptor_set;
            auto bindings = desc->GetLayout()->GetSortedBindingSet();
            if (bindings.size() > 0) {
                binding_count += desc->GetLayout()->GetMaxBinding() + 1;
                for (auto binding : bindings) {
                    // Shader instrumentation is tracking inline uniform blocks as scalers. Don't try to validate inline uniform
                    // blocks
                    if (VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT == desc->GetLayout()->GetTypeFromBinding(binding)) {
                        descriptor_count++;
                        log_msg(report_data, VK_DEBUG_REPORT_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_EXT,
                                VK_NULL_HANDLE, "UNASSIGNED-GPU-Assisted Validation Warning",
                                "VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT descriptors will not be validated by GPU assisted "
                                "validation");
                    } else if (binding == desc->GetLayout()->GetMaxBinding() && desc->IsVariableDescriptorCount(binding)) {
                        descriptor_count += desc->GetVariableDescriptorCount();
                    } else {
                        descriptor_count += desc->GetDescriptorCountFromBinding(binding);
                    }
                }
            }
        }

        // Note that the size of the input buffer is dependent on the maximum binding number, which
        // can be very large.  This is because for (set = s, binding = b, index = i), the validation
        // code is going to dereference Input[ i + Input[ b + Input[ s + Input[ Input[0] ] ] ] ] to
        // see if descriptors have been written. In gpu_validation.md, we note this and advise
        // using densely packed bindings as a best practice when using gpu-av with descriptor indexing
        uint32_t words_needed = 1 + (number_of_sets * 2) + (binding_count * 2) + descriptor_count;
        allocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
        bufferInfo.size = words_needed * 4;
        result = vmaCreateBuffer(gpu_validation_state->vmaAllocator, &bufferInfo, &allocInfo, &input_block.buffer,
                                 &input_block.allocation, nullptr);
        if (result != VK_SUCCESS) {
            ReportSetupProblem(VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(device),
                               "Unable to allocate device memory.  Device could become unstable.");
            gpu_validation_state->aborted = true;
            return;
        }

        // Populate input buffer first with the sizes of every descriptor in every set, then with whether
        // each element of each descriptor has been written or not.  See gpu_validation.md for a more thourough
        // outline of the input buffer format
        result = vmaMapMemory(gpu_validation_state->vmaAllocator, input_block.allocation, (void **)&pData);
        memset(pData, 0, static_cast<size_t>(bufferInfo.size));
        // Pointer to a sets array that points into the sizes array
        uint32_t *sets_to_sizes = pData + 1;
        // Pointer to the sizes array that contains the array size of the descriptor at each binding
        uint32_t *sizes = sets_to_sizes + number_of_sets;
        // Pointer to another sets array that points into the bindings array that points into the written array
        uint32_t *sets_to_bindings = sizes + binding_count;
        // Pointer to the bindings array that points at the start of the writes in the writes array for each binding
        uint32_t *bindings_to_written = sets_to_bindings + number_of_sets;
        // Index of the next entry in the written array to be updated
        uint32_t written_index = 1 + (number_of_sets * 2) + (binding_count * 2);
        uint32_t bindCounter = number_of_sets + 1;
        // Index of the start of the sets_to_bindings array
        pData[0] = number_of_sets + binding_count + 1;

        for (auto s : state.per_set) {
            auto desc = s.bound_descriptor_set;
            auto layout = desc->GetLayout();
            auto bindings = layout->GetSortedBindingSet();
            if (bindings.size() > 0) {
                // For each set, fill in index of its bindings sizes in the sizes array
                *sets_to_sizes++ = bindCounter;
                // For each set, fill in the index of its bindings in the bindings_to_written array
                *sets_to_bindings++ = bindCounter + number_of_sets + binding_count;
                for (auto binding : bindings) {
                    // For each binding, fill in its size in the sizes array
                    // Shader instrumentation is tracking inline uniform blocks as scalers. Don't try to validate inline uniform
                    // blocks
                    if (VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT == desc->GetLayout()->GetTypeFromBinding(binding)) {
                        sizes[binding] = 1;
                    } else if (binding == layout->GetMaxBinding() && desc->IsVariableDescriptorCount(binding)) {
                        sizes[binding] = desc->GetVariableDescriptorCount();
                    } else {
                        sizes[binding] = desc->GetDescriptorCountFromBinding(binding);
                    }
                    // Fill in the starting index for this binding in the written array in the bindings_to_written array
                    bindings_to_written[binding] = written_index;

                    // Shader instrumentation is tracking inline uniform blocks as scalers. Don't try to validate inline uniform
                    // blocks
                    if (VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT == desc->GetLayout()->GetTypeFromBinding(binding)) {
                        pData[written_index++] = 1;
                        continue;
                    }

                    auto index_range = desc->GetGlobalIndexRangeFromBinding(binding, true);
                    // For each array element in the binding, update the written array with whether it has been written
                    for (uint32_t i = index_range.start; i < index_range.end; ++i) {
                        auto *descriptor = desc->GetDescriptorFromGlobalIndex(i);
                        if (descriptor->updated) {
                            pData[written_index] = 1;
                        } else if (desc->IsUpdateAfterBind(binding)) {
                            // If it hasn't been written now and it's update after bind, put it in a list to check at QueueSubmit
                            input_block.update_at_submit[written_index] = descriptor;
                        }
                        written_index++;
                    }
                }
                auto last = desc->GetLayout()->GetMaxBinding();
                bindings_to_written += last + 1;
                bindCounter += last + 1;
                sizes += last + 1;
            } else {
                *sets_to_sizes++ = 0;
                *sets_to_bindings++ = 0;
            }
        }
        vmaUnmapMemory(gpu_validation_state->vmaAllocator, input_block.allocation);

        VkDescriptorBufferInfo input_desc_buffer_info = {};
        input_desc_buffer_info.range = (words_needed * 4);
        input_desc_buffer_info.buffer = input_block.buffer;
        input_desc_buffer_info.offset = 0;

        desc_writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        desc_writes[1].dstBinding = 1;
        desc_writes[1].descriptorCount = 1;
        desc_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        desc_writes[1].pBufferInfo = &input_desc_buffer_info;
        desc_writes[1].dstSet = desc_sets[0];

        desc_count = 2;
    }

    // Write the descriptor
    output_desc_buffer_info.buffer = output_block.buffer;
    output_desc_buffer_info.offset = 0;

    desc_writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    desc_writes[0].descriptorCount = 1;
    desc_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    desc_writes[0].pBufferInfo = &output_desc_buffer_info;
    desc_writes[0].dstSet = desc_sets[0];
    DispatchUpdateDescriptorSets(device, desc_count, desc_writes, 0, NULL);

    auto iter = cb_node->lastBound.find(bind_point);  // find() allows read-only access to cb_state
    if (iter != cb_node->lastBound.end()) {
        auto pipeline_state = iter->second.pipeline_state;
        if (pipeline_state && (pipeline_state->pipeline_layout.set_layouts.size() <= gpu_validation_state->desc_set_bind_index)) {
            DispatchCmdBindDescriptorSets(cmd_buffer, bind_point, pipeline_state->pipeline_layout.layout,
                                          gpu_validation_state->desc_set_bind_index, 1, desc_sets.data(), 0, nullptr);
        }
        // Record buffer and memory info in CB state tracking
        gpu_validation_state->GetGpuBufferInfo(cmd_buffer)
            .emplace_back(output_block, input_block, desc_sets[0], desc_pool, bind_point);
    } else {
        ReportSetupProblem(VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(device), "Unable to find pipeline state");
        vmaDestroyBuffer(gpu_validation_state->vmaAllocator, input_block.buffer, input_block.allocation);
        vmaDestroyBuffer(gpu_validation_state->vmaAllocator, output_block.buffer, output_block.allocation);
        gpu_validation_state->aborted = true;
        return;
    }
}
