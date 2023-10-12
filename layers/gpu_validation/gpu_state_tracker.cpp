/* Copyright (c) 2020-2023 The Khronos Group Inc.
 * Copyright (c) 2020-2023 Valve Corporation
 * Copyright (c) 2020-2023 LunarG, Inc.
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

#include "gpu_validation/gpu_state_tracker.h"
#include "sync/sync_utils.h"
#include "vma/vma.h"

// Implementation for Descriptor Set Manager class
UtilDescriptorSetManager::UtilDescriptorSetManager(VkDevice device, uint32_t num_bindings_in_set)
    : device(device), num_bindings_in_set(num_bindings_in_set) {}

UtilDescriptorSetManager::~UtilDescriptorSetManager() {
    for (auto &pool : desc_pool_map_) {
        DispatchDestroyDescriptorPool(device, pool.first, NULL);
    }
    desc_pool_map_.clear();
}

VkResult UtilDescriptorSetManager::GetDescriptorSet(VkDescriptorPool *desc_pool, VkDescriptorSetLayout ds_layout,
                                                    VkDescriptorSet *desc_set) {
    std::vector<VkDescriptorSet> desc_sets;
    VkResult result = GetDescriptorSets(1, desc_pool, ds_layout, &desc_sets);
    assert(result == VK_SUCCESS);
    if (result == VK_SUCCESS) {
        *desc_set = desc_sets[0];
    }
    return result;
}

VkResult UtilDescriptorSetManager::GetDescriptorSets(uint32_t count, VkDescriptorPool *pool, VkDescriptorSetLayout ds_layout,
                                                     std::vector<VkDescriptorSet> *desc_sets) {
    auto guard = Lock();
    const uint32_t default_pool_size = kItemsPerChunk;
    VkResult result = VK_SUCCESS;
    VkDescriptorPool pool_to_use = VK_NULL_HANDLE;

    assert(count > 0);
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
            pool_count * num_bindings_in_set,
        };
        VkDescriptorPoolCreateInfo desc_pool_info = vku::InitStructHelper();
        desc_pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        desc_pool_info.maxSets = pool_count;
        desc_pool_info.poolSizeCount = 1;
        desc_pool_info.pPoolSizes = &size_counts;
        result = DispatchCreateDescriptorPool(device, &desc_pool_info, NULL, &pool_to_use);
        assert(result == VK_SUCCESS);
        if (result != VK_SUCCESS) {
            return result;
        }
        desc_pool_map_[pool_to_use].size = desc_pool_info.maxSets;
        desc_pool_map_[pool_to_use].used = 0;
    }
    std::vector<VkDescriptorSetLayout> desc_layouts(count, ds_layout);

    VkDescriptorSetAllocateInfo alloc_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO, NULL, pool_to_use, count,
                                              desc_layouts.data()};

    result = DispatchAllocateDescriptorSets(device, &alloc_info, desc_sets->data());
    assert(result == VK_SUCCESS);
    if (result != VK_SUCCESS) {
        return result;
    }
    *pool = pool_to_use;
    desc_pool_map_[pool_to_use].used += count;
    return result;
}

void UtilDescriptorSetManager::PutBackDescriptorSet(VkDescriptorPool desc_pool, VkDescriptorSet desc_set) {
    auto guard = Lock();
    auto iter = desc_pool_map_.find(desc_pool);
    if (iter != desc_pool_map_.end()) {
        VkResult result = DispatchFreeDescriptorSets(device, desc_pool, 1, &desc_set);
        assert(result == VK_SUCCESS);
        if (result != VK_SUCCESS) {
            return;
        }
        desc_pool_map_[desc_pool].used--;
        if (0 == desc_pool_map_[desc_pool].used) {
            DispatchDestroyDescriptorPool(device, desc_pool, NULL);
            desc_pool_map_.erase(desc_pool);
        }
    }
    return;
}

// Trampolines to make VMA call Dispatch for Vulkan calls
static VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL gpuVkGetInstanceProcAddr(VkInstance inst, const char *name) {
    return DispatchGetInstanceProcAddr(inst, name);
}
static VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL gpuVkGetDeviceProcAddr(VkDevice dev, const char *name) {
    return DispatchGetDeviceProcAddr(dev, name);
}
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

VkResult UtilInitializeVma(VkInstance instance, VkPhysicalDevice physical_device, VkDevice device, bool use_buffer_device_address,
                           VmaAllocator *pAllocator) {
    VmaVulkanFunctions functions;
    VmaAllocatorCreateInfo allocator_info = {};
    allocator_info.instance = instance;
    allocator_info.device = device;
    allocator_info.physicalDevice = physical_device;

    if (use_buffer_device_address) {
        allocator_info.flags |= VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
    }

    functions.vkGetInstanceProcAddr = static_cast<PFN_vkGetInstanceProcAddr>(gpuVkGetInstanceProcAddr);
    functions.vkGetDeviceProcAddr = static_cast<PFN_vkGetDeviceProcAddr>(gpuVkGetDeviceProcAddr);
    functions.vkGetPhysicalDeviceProperties = static_cast<PFN_vkGetPhysicalDeviceProperties>(gpuVkGetPhysicalDeviceProperties);
    functions.vkGetPhysicalDeviceMemoryProperties =
        static_cast<PFN_vkGetPhysicalDeviceMemoryProperties>(gpuVkGetPhysicalDeviceMemoryProperties);
    functions.vkAllocateMemory = static_cast<PFN_vkAllocateMemory>(gpuVkAllocateMemory);
    functions.vkFreeMemory = static_cast<PFN_vkFreeMemory>(gpuVkFreeMemory);
    functions.vkMapMemory = static_cast<PFN_vkMapMemory>(gpuVkMapMemory);
    functions.vkUnmapMemory = static_cast<PFN_vkUnmapMemory>(gpuVkUnmapMemory);
    functions.vkFlushMappedMemoryRanges = static_cast<PFN_vkFlushMappedMemoryRanges>(gpuVkFlushMappedMemoryRanges);
    functions.vkInvalidateMappedMemoryRanges = static_cast<PFN_vkInvalidateMappedMemoryRanges>(gpuVkInvalidateMappedMemoryRanges);
    functions.vkBindBufferMemory = static_cast<PFN_vkBindBufferMemory>(gpuVkBindBufferMemory);
    functions.vkBindImageMemory = static_cast<PFN_vkBindImageMemory>(gpuVkBindImageMemory);
    functions.vkGetBufferMemoryRequirements = static_cast<PFN_vkGetBufferMemoryRequirements>(gpuVkGetBufferMemoryRequirements);
    functions.vkGetImageMemoryRequirements = static_cast<PFN_vkGetImageMemoryRequirements>(gpuVkGetImageMemoryRequirements);
    functions.vkCreateBuffer = static_cast<PFN_vkCreateBuffer>(gpuVkCreateBuffer);
    functions.vkDestroyBuffer = static_cast<PFN_vkDestroyBuffer>(gpuVkDestroyBuffer);
    functions.vkCreateImage = static_cast<PFN_vkCreateImage>(gpuVkCreateImage);
    functions.vkDestroyImage = static_cast<PFN_vkDestroyImage>(gpuVkDestroyImage);
    functions.vkCmdCopyBuffer = static_cast<PFN_vkCmdCopyBuffer>(gpuVkCmdCopyBuffer);
    allocator_info.pVulkanFunctions = &functions;

    return vmaCreateAllocator(&allocator_info, pAllocator);
}

gpu_utils_state::CommandBuffer::CommandBuffer(GpuAssistedBase *ga, VkCommandBuffer cb,
                                              const VkCommandBufferAllocateInfo *pCreateInfo, const COMMAND_POOL_STATE *pool)
    : CMD_BUFFER_STATE(ga, cb, pCreateInfo, pool) {}

ReadLockGuard GpuAssistedBase::ReadLock() const {
    if (fine_grained_locking) {
        return ReadLockGuard(validation_object_mutex, std::defer_lock);
    } else {
        return ReadLockGuard(validation_object_mutex);
    }
}

WriteLockGuard GpuAssistedBase::WriteLock() {
    if (fine_grained_locking) {
        return WriteLockGuard(validation_object_mutex, std::defer_lock);
    } else {
        return WriteLockGuard(validation_object_mutex);
    }
}

void GpuAssistedBase::PreCallRecordCreateDevice(VkPhysicalDevice gpu, const VkDeviceCreateInfo *pCreateInfo,
                                                const VkAllocationCallbacks *pAllocator, VkDevice *pDevice, void *modified_ci) {
    ValidationStateTracker::PreCallRecordCreateDevice(gpu, pCreateInfo, pAllocator, pDevice, modified_ci);
    VkPhysicalDeviceFeatures *features = nullptr;
    // Use a local variable to query features since this method runs in the instance validation object.
    // To avoid confusion and race conditions about which physical device's features are stored in the
    // 'supported_devices' member variable, it will only be set in the device validation objects.
    // See CreateDevice() below.
    VkPhysicalDeviceFeatures gpu_supported_features;
    DispatchGetPhysicalDeviceFeatures(gpu, &gpu_supported_features);

    // See CreateDevice() in chassis.cpp. modified_ci is a pointer to a safe struct stored on the stack.
    // This code follows the safe struct memory memory management scheme. That is, we must delete any memory
    // remove from the safe struct, and any additions must be allocated in a way that is compatible with
    // the safe struct destructor.
    auto *modified_create_info = static_cast<safe_VkDeviceCreateInfo *>(modified_ci);
    if (modified_create_info->pEnabledFeatures) {
        // If pEnabledFeatures, VkPhysicalDeviceFeatures2 in pNext chain is not allowed
        features = const_cast<VkPhysicalDeviceFeatures *>(modified_create_info->pEnabledFeatures);
    } else {
        auto *features2 =
            const_cast<VkPhysicalDeviceFeatures2 *>(vku::FindStructInPNextChain<VkPhysicalDeviceFeatures2>(modified_create_info->pNext));
        if (features2) features = &features2->features;
    }
    VkPhysicalDeviceFeatures new_features = {};
    VkBool32 *desired = reinterpret_cast<VkBool32 *>(&desired_features);
    VkBool32 *feature_ptr;
    if (features) {
        feature_ptr = reinterpret_cast<VkBool32 *>(features);
    } else {
        feature_ptr = reinterpret_cast<VkBool32 *>(&new_features);
    }
    VkBool32 *supported = reinterpret_cast<VkBool32 *>(&supported_features);
    for (size_t i = 0; i < sizeof(VkPhysicalDeviceFeatures); i += (sizeof(VkBool32))) {
        if (*supported && *desired) {
            *feature_ptr = true;
        }
        supported++;
        desired++;
        feature_ptr++;
    }
    if (!features) {
        delete modified_create_info->pEnabledFeatures;
        modified_create_info->pEnabledFeatures = new VkPhysicalDeviceFeatures(new_features);
    }
    if (force_buffer_device_address) {
        // TODO How to handle multi-device
        if (api_version > VK_API_VERSION_1_1) {
            auto *features12 = const_cast<VkPhysicalDeviceVulkan12Features *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceVulkan12Features>(modified_create_info->pNext));
            if (features12) {
                features12->bufferDeviceAddress = VK_TRUE;
            } else {
                auto *bda_features = const_cast<VkPhysicalDeviceBufferDeviceAddressFeatures *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceBufferDeviceAddressFeatures>(modified_create_info->pNext));
                if (bda_features) {
                    bda_features->bufferDeviceAddress = VK_TRUE;
                } else {
                    VkPhysicalDeviceBufferDeviceAddressFeatures new_bda_features = vku::InitStructHelper();
                    new_bda_features.bufferDeviceAddress = VK_TRUE;
                    new_bda_features.pNext = const_cast<void *>(modified_create_info->pNext);
                    modified_create_info->pNext = new VkPhysicalDeviceBufferDeviceAddressFeatures(new_bda_features);
                }
            }
        } else if (api_version == VK_API_VERSION_1_1) {
            static const std::string bda_ext{"VK_KHR_buffer_device_address"};
            bool found_ext = false;
            for (uint32_t i = 0; i < modified_create_info->enabledExtensionCount; i++) {
                if (bda_ext == modified_create_info->ppEnabledExtensionNames[i]) {
                    found_ext = true;
                    break;
                }
            }
            if (!found_ext) {
                const char **ext_names = new const char *[modified_create_info->enabledExtensionCount + 1];
                // Copy the existing pointer table
                std::copy(modified_create_info->ppEnabledExtensionNames,
                          modified_create_info->ppEnabledExtensionNames + modified_create_info->enabledExtensionCount, ext_names);
                // Add our new extension
                char *bda_ext_copy = new char[bda_ext.size() + 1]{};
                bda_ext.copy(bda_ext_copy, bda_ext.size());
                bda_ext_copy[bda_ext.size()] = '\0';
                ext_names[modified_create_info->enabledExtensionCount] = bda_ext_copy;
                // Patch up the safe struct
                delete[] modified_create_info->ppEnabledExtensionNames;
                modified_create_info->ppEnabledExtensionNames = ext_names;
                modified_create_info->enabledExtensionCount++;
            }
            auto *bda_features = const_cast<VkPhysicalDeviceBufferDeviceAddressFeatures *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceBufferDeviceAddressFeatures>(modified_create_info));
            if (bda_features) {
                bda_features->bufferDeviceAddress = VK_TRUE;
            } else {
                VkPhysicalDeviceBufferDeviceAddressFeatures new_bda_features = vku::InitStructHelper();
                new_bda_features.bufferDeviceAddress = VK_TRUE;
                new_bda_features.pNext = const_cast<void *>(modified_create_info->pNext);
                modified_create_info->pNext = new VkPhysicalDeviceBufferDeviceAddressFeatures(new_bda_features);
            }
        } else {
            force_buffer_device_address = false;
        }
    }
}

void GpuAssistedBase::CreateDevice(const VkDeviceCreateInfo *pCreateInfo) {
    ValidationStateTracker::CreateDevice(pCreateInfo);
    // If api version 1.1 or later, SetDeviceLoaderData will be in the loader
    auto chain_info = get_chain_info(pCreateInfo, VK_LOADER_DATA_CALLBACK);
    assert(chain_info->u.pfnSetDeviceLoaderData);
    vkSetDeviceLoaderData = chain_info->u.pfnSetDeviceLoaderData;

    // Some devices have extremely high limits here, so set a reasonable max because we have to pad
    // the pipeline layout with dummy descriptor set layouts.
    adjusted_max_desc_sets = phys_dev_props.limits.maxBoundDescriptorSets;
    adjusted_max_desc_sets = std::min(33U, adjusted_max_desc_sets);

    // We can't do anything if there is only one.
    // Device probably not a legit Vulkan device, since there should be at least 4. Protect ourselves.
    if (adjusted_max_desc_sets == 1) {
        ReportSetupProblem(device, "Device can bind only a single descriptor set.");
        aborted = true;
        return;
    }
    desc_set_bind_index = adjusted_max_desc_sets - 1;

    VkResult result1 = UtilInitializeVma(instance, physical_device, device, force_buffer_device_address, &vmaAllocator);
    assert(result1 == VK_SUCCESS);
    desc_set_manager = std::make_unique<UtilDescriptorSetManager>(device, static_cast<uint32_t>(bindings_.size()));

    const VkDescriptorSetLayoutCreateInfo debug_desc_layout_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO, NULL, 0,
                                                                    static_cast<uint32_t>(bindings_.size()), bindings_.data()};

    const VkDescriptorSetLayoutCreateInfo dummy_desc_layout_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO, NULL, 0, 0,
                                                                    NULL};

    result1 = DispatchCreateDescriptorSetLayout(device, &debug_desc_layout_info, NULL, &debug_desc_layout);

    // This is a layout used to "pad" a pipeline layout to fill in any gaps to the selected bind index.
    VkResult result2 = DispatchCreateDescriptorSetLayout(device, &dummy_desc_layout_info, NULL, &dummy_desc_layout);

    std::vector<VkDescriptorSetLayout> debug_layouts;
    for (uint32_t j = 0; j < adjusted_max_desc_sets - 1; ++j) {
        debug_layouts.push_back(dummy_desc_layout);
    }
    debug_layouts.push_back(debug_desc_layout);
    const VkPipelineLayoutCreateInfo debug_pipeline_layout_info = {
        VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO, NULL, 0u, static_cast<uint32_t>(debug_layouts.size()), debug_layouts.data(), 0u, NULL};
    VkResult result3 = DispatchCreatePipelineLayout(device, &debug_pipeline_layout_info, NULL, &debug_pipeline_layout);

    assert((result1 == VK_SUCCESS) && (result2 == VK_SUCCESS) && (result3 == VK_SUCCESS));
    if ((result1 != VK_SUCCESS) || (result2 != VK_SUCCESS) || (result3 != VK_SUCCESS)) {
        ReportSetupProblem(device, "Unable to create descriptor set layout.");
        if (result1 == VK_SUCCESS) {
            DispatchDestroyDescriptorSetLayout(device, debug_desc_layout, NULL);
        }
        if (result2 == VK_SUCCESS) {
            DispatchDestroyDescriptorSetLayout(device, dummy_desc_layout, NULL);
        }
        if (result3 == VK_SUCCESS) {
            DispatchDestroyPipelineLayout(device, debug_pipeline_layout, NULL);
        }
        debug_desc_layout = VK_NULL_HANDLE;
        dummy_desc_layout = VK_NULL_HANDLE;
        debug_pipeline_layout = VK_NULL_HANDLE;
        aborted = true;
        return;
    }
}

void GpuAssistedBase::PreCallRecordDestroyDevice(VkDevice device, const VkAllocationCallbacks *pAllocator) {
    if (debug_desc_layout) {
        DispatchDestroyDescriptorSetLayout(device, debug_desc_layout, NULL);
        debug_desc_layout = VK_NULL_HANDLE;
    }
    if (dummy_desc_layout) {
        DispatchDestroyDescriptorSetLayout(device, dummy_desc_layout, NULL);
        dummy_desc_layout = VK_NULL_HANDLE;
    }
    if (debug_pipeline_layout) {
        DispatchDestroyPipelineLayout(device, debug_pipeline_layout, NULL);
    }
    ValidationStateTracker::PreCallRecordDestroyDevice(device, pAllocator);
    // State Tracker can end up making vma calls through callbacks - don't destroy allocator until ST is done
    if (output_buffer_pool) {
        vmaDestroyPool(vmaAllocator, output_buffer_pool);
    }
    if (vmaAllocator) {
        vmaDestroyAllocator(vmaAllocator);
    }
    desc_set_manager.reset();
}

gpu_utils_state::Queue::Queue(GpuAssistedBase &state, VkQueue q, uint32_t index, VkDeviceQueueCreateFlags flags,
                              const VkQueueFamilyProperties &queueFamilyProperties)
    : QUEUE_STATE(state, q, index, flags, queueFamilyProperties), state_(state) {}

gpu_utils_state::Queue::~Queue() {
    if (barrier_command_buffer_) {
        DispatchFreeCommandBuffers(state_.device, barrier_command_pool_, 1, &barrier_command_buffer_);
        barrier_command_buffer_ = VK_NULL_HANDLE;
    }
    if (barrier_command_pool_) {
        DispatchDestroyCommandPool(state_.device, barrier_command_pool_, NULL);
        barrier_command_pool_ = VK_NULL_HANDLE;
    }
}

// Submit a memory barrier on graphics queues.
// Lazy-create and record the needed command buffer.
void gpu_utils_state::Queue::SubmitBarrier() {
    if (barrier_command_pool_ == VK_NULL_HANDLE) {
        VkResult result = VK_SUCCESS;

        VkCommandPoolCreateInfo pool_create_info = vku::InitStructHelper();
        pool_create_info.queueFamilyIndex = queueFamilyIndex;
        result = DispatchCreateCommandPool(state_.device, &pool_create_info, nullptr, &barrier_command_pool_);
        if (result != VK_SUCCESS) {
            state_.ReportSetupProblem(state_.device, "Unable to create command pool for barrier CB.");
            barrier_command_pool_ = VK_NULL_HANDLE;
            return;
        }

        VkCommandBufferAllocateInfo buffer_alloc_info = vku::InitStructHelper();
        buffer_alloc_info.commandPool = barrier_command_pool_;
        buffer_alloc_info.commandBufferCount = 1;
        buffer_alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        result = DispatchAllocateCommandBuffers(state_.device, &buffer_alloc_info, &barrier_command_buffer_);
        if (result != VK_SUCCESS) {
            state_.ReportSetupProblem(state_.device, "Unable to create barrier command buffer.");
            DispatchDestroyCommandPool(state_.device, barrier_command_pool_, nullptr);
            barrier_command_pool_ = VK_NULL_HANDLE;
            barrier_command_buffer_ = VK_NULL_HANDLE;
            return;
        }

        // Hook up command buffer dispatch
        state_.vkSetDeviceLoaderData(state_.device, barrier_command_buffer_);

        // Record a global memory barrier to force availability of device memory operations to the host domain.
        VkCommandBufferBeginInfo command_buffer_begin_info = vku::InitStructHelper();
        result = DispatchBeginCommandBuffer(barrier_command_buffer_, &command_buffer_begin_info);
        if (result == VK_SUCCESS) {
            VkMemoryBarrier memory_barrier = vku::InitStructHelper();
            memory_barrier.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
            memory_barrier.dstAccessMask = VK_ACCESS_HOST_READ_BIT;
            DispatchCmdPipelineBarrier(barrier_command_buffer_, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_HOST_BIT, 0,
                                       1, &memory_barrier, 0, nullptr, 0, nullptr);
            DispatchEndCommandBuffer(barrier_command_buffer_);
        }
    }
    if (barrier_command_buffer_ != VK_NULL_HANDLE) {
        VkSubmitInfo submit_info = vku::InitStructHelper();
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &barrier_command_buffer_;
        DispatchQueueSubmit(QUEUE_STATE::Queue(), 1, &submit_info, VK_NULL_HANDLE);
    }
}

bool GpuAssistedBase::CommandBufferNeedsProcessing(VkCommandBuffer command_buffer) const {
    auto cb_node = GetRead<gpu_utils_state::CommandBuffer>(command_buffer);
    if (cb_node->NeedsProcessing()) {
        return true;
    }
    for (const auto *secondary_cb : cb_node->linkedCommandBuffers) {
        auto secondary_cb_node = static_cast<const gpu_utils_state::CommandBuffer *>(secondary_cb);
        auto guard = secondary_cb_node->ReadLock();
        if (secondary_cb_node->NeedsProcessing()) {
            return true;
        }
    }
    return false;
}

void GpuAssistedBase::ProcessCommandBuffer(VkQueue queue, VkCommandBuffer command_buffer) {
    auto cb_node = GetWrite<gpu_utils_state::CommandBuffer>(command_buffer);

    cb_node->Process(queue);
    for (auto *secondary_cmd_base : cb_node->linkedCommandBuffers) {
        auto *secondary_cb_node = static_cast<gpu_utils_state::CommandBuffer *>(secondary_cmd_base);
        auto guard = secondary_cb_node->WriteLock();
        secondary_cb_node->Process(queue);
    }
}

// Issue a memory barrier to make GPU-written data available to host.
// Wait for the queue to complete execution.
// Check the debug buffers for all the command buffers that were submitted.
void GpuAssistedBase::PostCallRecordQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo *pSubmits, VkFence fence,
                                                const RecordObject &record_obj) {
    ValidationStateTracker::PostCallRecordQueueSubmit(queue, submitCount, pSubmits, fence, record_obj);

    if (aborted || (record_obj.result != VK_SUCCESS)) return;
    bool buffers_present = false;
    // Don't QueueWaitIdle if there's nothing to process
    for (uint32_t submit_idx = 0; submit_idx < submitCount; submit_idx++) {
        const VkSubmitInfo *submit = &pSubmits[submit_idx];
        for (uint32_t i = 0; i < submit->commandBufferCount; i++) {
            buffers_present |= CommandBufferNeedsProcessing(submit->pCommandBuffers[i]);
        }
    }
    if (!buffers_present) return;

    SubmitBarrier(queue);

    DispatchQueueWaitIdle(queue);

    for (uint32_t submit_idx = 0; submit_idx < submitCount; submit_idx++) {
        const VkSubmitInfo *submit = &pSubmits[submit_idx];
        for (uint32_t i = 0; i < submit->commandBufferCount; i++) {
            ProcessCommandBuffer(queue, submit->pCommandBuffers[i]);
        }
    }
}

void GpuAssistedBase::RecordQueueSubmit2(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2 *pSubmits, VkFence fence,
                                         const RecordObject &record_obj) {
    if (aborted || (record_obj.result != VK_SUCCESS)) return;
    bool buffers_present = false;
    // Don't QueueWaitIdle if there's nothing to process
    for (uint32_t submit_idx = 0; submit_idx < submitCount; submit_idx++) {
        const VkSubmitInfo2 *submit = &pSubmits[submit_idx];
        for (uint32_t i = 0; i < submit->commandBufferInfoCount; i++) {
            buffers_present |= CommandBufferNeedsProcessing(submit->pCommandBufferInfos[i].commandBuffer);
        }
    }
    if (!buffers_present) return;

    SubmitBarrier(queue);

    DispatchQueueWaitIdle(queue);

    for (uint32_t submit_idx = 0; submit_idx < submitCount; submit_idx++) {
        const VkSubmitInfo2 *submit = &pSubmits[submit_idx];
        for (uint32_t i = 0; i < submit->commandBufferInfoCount; i++) {
            ProcessCommandBuffer(queue, submit->pCommandBufferInfos[i].commandBuffer);
        }
    }
}

void GpuAssistedBase::PostCallRecordQueueSubmit2KHR(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2KHR *pSubmits,
                                                    VkFence fence, const RecordObject &record_obj) {
    ValidationStateTracker::PostCallRecordQueueSubmit2KHR(queue, submitCount, pSubmits, fence, record_obj);
    RecordQueueSubmit2(queue, submitCount, pSubmits, fence, record_obj);
}

void GpuAssistedBase::PostCallRecordQueueSubmit2(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2 *pSubmits, VkFence fence,
                                                 const RecordObject &record_obj) {
    ValidationStateTracker::PostCallRecordQueueSubmit2(queue, submitCount, pSubmits, fence, record_obj);
    RecordQueueSubmit2(queue, submitCount, pSubmits, fence, record_obj);
}

// Just gives a warning about a possible deadlock.
bool GpuAssistedBase::ValidateCmdWaitEvents(VkCommandBuffer command_buffer, VkPipelineStageFlags2 src_stage_mask,
                                            const Location &loc) const {
    if (src_stage_mask & VK_PIPELINE_STAGE_2_HOST_BIT) {
        std::ostringstream error_msg;
        error_msg << loc.Message()
                  << ": recorded with VK_PIPELINE_STAGE_HOST_BIT set. GPU-Assisted validation waits on queue completion. This wait "
                     "could block the host's signaling of this event, resulting in deadlock.";
        ReportSetupProblem(command_buffer, error_msg.str().c_str());
    }
    return false;
}

bool GpuAssistedBase::PreCallValidateCmdWaitEvents(
    VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents, VkPipelineStageFlags srcStageMask,
    VkPipelineStageFlags dstStageMask, uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers,
    uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier *pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount,
    const VkImageMemoryBarrier *pImageMemoryBarriers, const ErrorObject &error_obj) const {
    ValidationStateTracker::PreCallValidateCmdWaitEvents(
        commandBuffer, eventCount, pEvents, srcStageMask, dstStageMask, memoryBarrierCount, pMemoryBarriers,
        bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers, error_obj);
    return ValidateCmdWaitEvents(commandBuffer, static_cast<VkPipelineStageFlags2>(srcStageMask), error_obj.location);
}

bool GpuAssistedBase::PreCallValidateCmdWaitEvents2KHR(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                                       const VkDependencyInfoKHR *pDependencyInfos,
                                                       const ErrorObject &error_obj) const {
    return PreCallValidateCmdWaitEvents2(commandBuffer, eventCount, pEvents, pDependencyInfos, error_obj);
}

bool GpuAssistedBase::PreCallValidateCmdWaitEvents2(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                                    const VkDependencyInfo *pDependencyInfos, const ErrorObject &error_obj) const {
    VkPipelineStageFlags2 src_stage_mask = 0;

    for (uint32_t i = 0; i < eventCount; i++) {
        auto stage_masks = sync_utils::GetGlobalStageMasks(pDependencyInfos[i]);
        src_stage_mask |= stage_masks.src;
    }

    ValidationStateTracker::PreCallValidateCmdWaitEvents2(commandBuffer, eventCount, pEvents, pDependencyInfos, error_obj);
    return ValidateCmdWaitEvents(commandBuffer, src_stage_mask, error_obj.location);
}

void GpuAssistedBase::PreCallRecordCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo *pCreateInfo,
                                                        const VkAllocationCallbacks *pAllocator, VkPipelineLayout *pPipelineLayout,
                                                        void *cpl_state_data) {
    if (aborted) {
        return;
    }
    auto cpl_state = static_cast<create_pipeline_layout_api_state *>(cpl_state_data);
    if (cpl_state->modified_create_info.setLayoutCount >= adjusted_max_desc_sets) {
        std::ostringstream strm;
        strm << "Pipeline Layout conflict with validation's descriptor set at slot " << desc_set_bind_index << ". "
             << "Application has too many descriptor sets in the pipeline layout to continue with gpu validation. "
             << "Validation is not modifying the pipeline layout. "
             << "Instrumented shaders are replaced with non-instrumented shaders.";
        ReportSetupProblem(device, strm.str().c_str());
    } else {
        // Modify the pipeline layout by:
        // 1. Copying the caller's descriptor set desc_layouts
        // 2. Fill in dummy descriptor layouts up to the max binding
        // 3. Fill in with the debug descriptor layout at the max binding slot
        cpl_state->new_layouts.reserve(adjusted_max_desc_sets);
        cpl_state->new_layouts.insert(cpl_state->new_layouts.end(), &pCreateInfo->pSetLayouts[0],
                                      &pCreateInfo->pSetLayouts[pCreateInfo->setLayoutCount]);
        for (uint32_t i = pCreateInfo->setLayoutCount; i < adjusted_max_desc_sets - 1; ++i) {
            cpl_state->new_layouts.push_back(dummy_desc_layout);
        }
        cpl_state->new_layouts.push_back(debug_desc_layout);
        cpl_state->modified_create_info.pSetLayouts = cpl_state->new_layouts.data();
        cpl_state->modified_create_info.setLayoutCount = adjusted_max_desc_sets;
    }
    ValidationStateTracker::PreCallRecordCreatePipelineLayout(device, pCreateInfo, pAllocator, pPipelineLayout, cpl_state_data);
}

void GpuAssistedBase::PostCallRecordCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo *pCreateInfo,
                                                         const VkAllocationCallbacks *pAllocator, VkPipelineLayout *pPipelineLayout,
                                                         const RecordObject &record_obj) {
    if (record_obj.result != VK_SUCCESS) {
        ReportSetupProblem(device, "Unable to create pipeline layout.  Device could become unstable.");
        aborted = true;
    }
    ValidationStateTracker::PostCallRecordCreatePipelineLayout(device, pCreateInfo, pAllocator, pPipelineLayout, record_obj);
}

void GpuAssistedBase::PreCallRecordCreateShadersEXT(VkDevice device, uint32_t createInfoCount,
                                                    const VkShaderCreateInfoEXT *pCreateInfos,
                                                    const VkAllocationCallbacks *pAllocator, VkShaderEXT *pShaders,
                                                    void *csm_state_data) {
    if (aborted) {
        return;
    }
    auto cso_state = static_cast<create_shader_object_api_state *>(csm_state_data);
    for (uint32_t i = 0; i < createInfoCount; ++i) {
        if (cso_state->instrumented_create_info->setLayoutCount >= adjusted_max_desc_sets) {
            std::ostringstream strm;
            strm << "Descriptor Set Layout conflict with validation's descriptor set at slot " << desc_set_bind_index << ". "
                 << "Application has too many descriptor sets in the pipeline layout to continue with gpu validation. "
                 << "Validation is not modifying the pipeline layout. "
                 << "Instrumented shaders are replaced with non-instrumented shaders.";
            ReportSetupProblem(device, strm.str().c_str());
        } else {
            // Modify the pipeline layout by:
            // 1. Copying the caller's descriptor set desc_layouts
            // 2. Fill in dummy descriptor layouts up to the max binding
            // 3. Fill in with the debug descriptor layout at the max binding slot
            cso_state->new_layouts.reserve(adjusted_max_desc_sets);
            cso_state->new_layouts.insert(cso_state->new_layouts.end(), pCreateInfos[i].pSetLayouts,
                                          &pCreateInfos[i].pSetLayouts[pCreateInfos[i].setLayoutCount]);
            for (uint32_t j = pCreateInfos[i].setLayoutCount; j < adjusted_max_desc_sets - 1; ++j) {
                cso_state->new_layouts.push_back(dummy_desc_layout);
            }
            cso_state->new_layouts.push_back(debug_desc_layout);
            cso_state->instrumented_create_info->pSetLayouts = cso_state->new_layouts.data();
            cso_state->instrumented_create_info->setLayoutCount = adjusted_max_desc_sets;
        }
    }
}

void GpuAssistedBase::PostCallRecordCreateShadersEXT(VkDevice device, uint32_t createInfoCount,
                                                     const VkShaderCreateInfoEXT *pCreateInfos,
                                                     const VkAllocationCallbacks *pAllocator, VkShaderEXT *pShaders,
                                                     const RecordObject &record_obj, void *csm_state_data) {
    ValidationStateTracker::PostCallRecordCreateShadersEXT(device, createInfoCount, pCreateInfos, pAllocator, pShaders, record_obj,
                                                           csm_state_data);
    if (aborted) return;

    auto cso_state = static_cast<create_shader_object_api_state *>(csm_state_data);
    for (uint32_t i = 0; i < createInfoCount; ++i) {
        shader_map.insert_or_assign(cso_state->unique_shader_ids[i], VK_NULL_HANDLE, VK_NULL_HANDLE, pShaders[i],
                                    cso_state->instrumented_spirv[i]);
    }
}

void GpuAssistedBase::PreCallRecordDestroyShaderEXT(VkDevice device, VkShaderEXT shader, const VkAllocationCallbacks *pAllocator) {
    auto to_erase = shader_map.snapshot([shader](const GpuAssistedShaderTracker &entry) { return entry.shader_object == shader; });
    for (const auto &entry : to_erase) {
        shader_map.erase(entry.first);
    }
    ValidationStateTracker::PreCallRecordDestroyShaderEXT(device, shader, pAllocator);
}

void GpuAssistedBase::PreCallRecordCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                           const VkGraphicsPipelineCreateInfo *pCreateInfos,
                                                           const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                           void *cgpl_state_data) {
    if (aborted) return;
    std::vector<safe_VkGraphicsPipelineCreateInfo> new_pipeline_create_infos;
    create_graphics_pipeline_api_state *cgpl_state = reinterpret_cast<create_graphics_pipeline_api_state *>(cgpl_state_data);
    PreCallRecordPipelineCreations(count, pCreateInfos, pAllocator, pPipelines, cgpl_state->pipe_state, &new_pipeline_create_infos,
                                   VK_PIPELINE_BIND_POINT_GRAPHICS, *cgpl_state);
    cgpl_state->modified_create_infos = new_pipeline_create_infos;
    cgpl_state->pCreateInfos = reinterpret_cast<VkGraphicsPipelineCreateInfo *>(cgpl_state->modified_create_infos.data());
}

void GpuAssistedBase::PreCallRecordCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                          const VkComputePipelineCreateInfo *pCreateInfos,
                                                          const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                          void *ccpl_state_data) {
    if (aborted) return;
    std::vector<safe_VkComputePipelineCreateInfo> new_pipeline_create_infos;
    auto *ccpl_state = reinterpret_cast<create_compute_pipeline_api_state *>(ccpl_state_data);
    PreCallRecordPipelineCreations(count, pCreateInfos, pAllocator, pPipelines, ccpl_state->pipe_state, &new_pipeline_create_infos,
                                   VK_PIPELINE_BIND_POINT_COMPUTE, *ccpl_state);
    ccpl_state->modified_create_infos = new_pipeline_create_infos;
    ccpl_state->pCreateInfos = reinterpret_cast<VkComputePipelineCreateInfo *>(ccpl_state->modified_create_infos.data());
}

void GpuAssistedBase::PreCallRecordCreateRayTracingPipelinesNV(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                               const VkRayTracingPipelineCreateInfoNV *pCreateInfos,
                                                               const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                               void *crtpl_state_data) {
    if (aborted) return;
    std::vector<safe_VkRayTracingPipelineCreateInfoCommon> new_pipeline_create_infos;
    auto *crtpl_state = reinterpret_cast<create_ray_tracing_pipeline_api_state *>(crtpl_state_data);
    PreCallRecordPipelineCreations(count, pCreateInfos, pAllocator, pPipelines, crtpl_state->pipe_state, &new_pipeline_create_infos,
                                   VK_PIPELINE_BIND_POINT_RAY_TRACING_NV, *crtpl_state);
    crtpl_state->modified_create_infos = new_pipeline_create_infos;
    crtpl_state->pCreateInfos = reinterpret_cast<VkRayTracingPipelineCreateInfoNV *>(crtpl_state->modified_create_infos.data());
}

void GpuAssistedBase::PreCallRecordCreateRayTracingPipelinesKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                                VkPipelineCache pipelineCache, uint32_t count,
                                                                const VkRayTracingPipelineCreateInfoKHR *pCreateInfos,
                                                                const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                                void *crtpl_state_data) {
    if (aborted) return;
    std::vector<safe_VkRayTracingPipelineCreateInfoCommon> new_pipeline_create_infos;
    auto *crtpl_state = reinterpret_cast<create_ray_tracing_pipeline_khr_api_state *>(crtpl_state_data);
    PreCallRecordPipelineCreations(count, pCreateInfos, pAllocator, pPipelines, crtpl_state->pipe_state, &new_pipeline_create_infos,
                                   VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, *crtpl_state);
    crtpl_state->modified_create_infos = new_pipeline_create_infos;
    crtpl_state->pCreateInfos = reinterpret_cast<VkRayTracingPipelineCreateInfoKHR *>(crtpl_state->modified_create_infos.data());
}

template <typename CreateInfos, typename SafeCreateInfos>
static void UtilCopyCreatePipelineFeedbackData(const uint32_t count, CreateInfos *pCreateInfos, SafeCreateInfos *pSafeCreateInfos) {
    for (uint32_t i = 0; i < count; i++) {
        auto src_feedback_struct = vku::FindStructInPNextChain<VkPipelineCreationFeedbackCreateInfoEXT>(pSafeCreateInfos[i].pNext);
        if (!src_feedback_struct) return;
        auto dst_feedback_struct = const_cast<VkPipelineCreationFeedbackCreateInfoEXT *>(
            vku::FindStructInPNextChain<VkPipelineCreationFeedbackCreateInfoEXT>(pCreateInfos[i].pNext));
        *dst_feedback_struct->pPipelineCreationFeedback = *src_feedback_struct->pPipelineCreationFeedback;
        for (uint32_t j = 0; j < src_feedback_struct->pipelineStageCreationFeedbackCount; j++) {
            dst_feedback_struct->pPipelineStageCreationFeedbacks[j] = src_feedback_struct->pPipelineStageCreationFeedbacks[j];
        }
    }
}

void GpuAssistedBase::PostCallRecordCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                            const VkGraphicsPipelineCreateInfo *pCreateInfos,
                                                            const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                            const RecordObject &record_obj, void *cgpl_state_data) {
    ValidationStateTracker::PostCallRecordCreateGraphicsPipelines(device, pipelineCache, count, pCreateInfos, pAllocator,
                                                                  pPipelines, record_obj, cgpl_state_data);
    if (aborted) return;
    create_graphics_pipeline_api_state *cgpl_state = reinterpret_cast<create_graphics_pipeline_api_state *>(cgpl_state_data);
    UtilCopyCreatePipelineFeedbackData(count, pCreateInfos, cgpl_state->modified_create_infos.data());
    PostCallRecordPipelineCreations(count, pCreateInfos, pAllocator, pPipelines, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                    cgpl_state->modified_create_infos.data());
}

void GpuAssistedBase::PostCallRecordCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                           const VkComputePipelineCreateInfo *pCreateInfos,
                                                           const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                           const RecordObject &record_obj, void *ccpl_state_data) {
    ValidationStateTracker::PostCallRecordCreateComputePipelines(device, pipelineCache, count, pCreateInfos, pAllocator, pPipelines,
                                                                 record_obj, ccpl_state_data);
    if (aborted) return;
    create_compute_pipeline_api_state *ccpl_state = reinterpret_cast<create_compute_pipeline_api_state *>(ccpl_state_data);
    UtilCopyCreatePipelineFeedbackData(count, pCreateInfos, ccpl_state->modified_create_infos.data());
    PostCallRecordPipelineCreations(count, pCreateInfos, pAllocator, pPipelines, VK_PIPELINE_BIND_POINT_COMPUTE,
                                    ccpl_state->modified_create_infos.data());
}

void GpuAssistedBase::PostCallRecordCreateRayTracingPipelinesNV(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                                const VkRayTracingPipelineCreateInfoNV *pCreateInfos,
                                                                const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                                const RecordObject &record_obj, void *crtpl_state_data) {
    auto *crtpl_state = reinterpret_cast<create_ray_tracing_pipeline_khr_api_state *>(crtpl_state_data);
    ValidationStateTracker::PostCallRecordCreateRayTracingPipelinesNV(device, pipelineCache, count, pCreateInfos, pAllocator,
                                                                      pPipelines, record_obj, crtpl_state_data);
    if (aborted) return;
    UtilCopyCreatePipelineFeedbackData(count, pCreateInfos, crtpl_state->modified_create_infos.data());
    PostCallRecordPipelineCreations(count, pCreateInfos, pAllocator, pPipelines, VK_PIPELINE_BIND_POINT_RAY_TRACING_NV,
                                    crtpl_state->modified_create_infos.data());
}

void GpuAssistedBase::PostCallRecordCreateRayTracingPipelinesKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                                 VkPipelineCache pipelineCache, uint32_t count,
                                                                 const VkRayTracingPipelineCreateInfoKHR *pCreateInfos,
                                                                 const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                                 const RecordObject &record_obj, void *crtpl_state_data) {
    auto *crtpl_state = reinterpret_cast<create_ray_tracing_pipeline_khr_api_state *>(crtpl_state_data);
    ValidationStateTracker::PostCallRecordCreateRayTracingPipelinesKHR(
        device, deferredOperation, pipelineCache, count, pCreateInfos, pAllocator, pPipelines, record_obj, crtpl_state_data);
    if (aborted) return;
    UtilCopyCreatePipelineFeedbackData(count, pCreateInfos, crtpl_state->modified_create_infos.data());
    PostCallRecordPipelineCreations(count, pCreateInfos, pAllocator, pPipelines, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR,
                                    crtpl_state->modified_create_infos.data());
}

// Remove all the shader trackers associated with this destroyed pipeline.
void GpuAssistedBase::PreCallRecordDestroyPipeline(VkDevice device, VkPipeline pipeline, const VkAllocationCallbacks *pAllocator) {
    auto to_erase = shader_map.snapshot([pipeline](const GpuAssistedShaderTracker &entry) { return entry.pipeline == pipeline; });
    for (const auto &entry : to_erase) {
        shader_map.erase(entry.first);
    }
    ValidationStateTracker::PreCallRecordDestroyPipeline(device, pipeline, pAllocator);
}

template <typename CreateInfo>
VkShaderModule GetShaderModule(const CreateInfo &createInfo, VkShaderStageFlagBits stage) {
    for (uint32_t i = 0; i < createInfo.stageCount; ++i) {
        if (createInfo.pStages[i].stage == stage) {
            return createInfo.pStages[i].module;
        }
    }
    return {};
}

template <>
VkShaderModule GetShaderModule(const VkComputePipelineCreateInfo &createInfo, VkShaderStageFlagBits) {
    return createInfo.stage.module;
}

template <typename SafeType>
void SetShaderModule(SafeType &createInfo, const safe_VkPipelineShaderStageCreateInfo &stage_info, VkShaderModule shader_module,
                     uint32_t stage_ci_index) {
    createInfo.pStages[stage_ci_index] = stage_info;
    createInfo.pStages[stage_ci_index].module = shader_module;
}

template <>
void SetShaderModule(safe_VkComputePipelineCreateInfo &createInfo, const safe_VkPipelineShaderStageCreateInfo &stage_info,
                     VkShaderModule shader_module, uint32_t stage_ci_index) {
    assert(stage_ci_index == 0);
    createInfo.stage = stage_info;
    createInfo.stage.module = shader_module;
}

template <typename CreateInfo, typename StageInfo>
StageInfo &GetShaderStageCI(CreateInfo &ci, VkShaderStageFlagBits stage) {
    static StageInfo null_stage{};
    for (uint32_t i = 0; i < ci.stageCount; ++i) {
        if (ci.pStages[i].stage == stage) {
            return ci.pStages[i];
        }
    }
    return null_stage;
}

template <>
safe_VkPipelineShaderStageCreateInfo &GetShaderStageCI(safe_VkComputePipelineCreateInfo &ci, VkShaderStageFlagBits) {
    return ci.stage;
}

bool GpuAssistedBase::CheckForGpuAvEnabled(const void *pNext) {
    auto features = vku::FindStructInPNextChain<VkValidationFeaturesEXT>(pNext);
    if (features) {
        for (uint32_t i = 0; i < features->enabledValidationFeatureCount; i++) {
            if (features->pEnabledValidationFeatures[i] == VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT) {
                return true;
            }
        }
    }
    return false;
}

// Examine the pipelines to see if they use the debug descriptor set binding index.
// If any do, create new non-instrumented shader modules and use them to replace the instrumented
// shaders in the pipeline.  Return the (possibly) modified create infos to the caller.
template <typename CreateInfo, typename SafeCreateInfo, typename GPUAVState>
void GpuAssistedBase::PreCallRecordPipelineCreations(uint32_t count, const CreateInfo *pCreateInfos,
                                                     const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                     std::vector<std::shared_ptr<PIPELINE_STATE>> &pipe_state,
                                                     std::vector<SafeCreateInfo> *new_pipeline_create_infos,
                                                     const VkPipelineBindPoint bind_point, GPUAVState &cgpl_state) {
    if (bind_point != VK_PIPELINE_BIND_POINT_GRAPHICS && bind_point != VK_PIPELINE_BIND_POINT_COMPUTE &&
        bind_point != VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR) {
        return;
    }

    // Walk through all the pipelines, make a copy of each and flag each pipeline that contains a shader that uses the debug
    // descriptor set index.
    for (uint32_t pipeline = 0; pipeline < count; ++pipeline) {
        const auto &pipe = pipe_state[pipeline];
        // NOTE: since these are "safe" CreateInfos, this will create a deep copy via the safe copy constructor
        auto new_pipeline_ci = pipe->GetCreateInfo<CreateInfo>();

        bool replace_shaders = false;
        if (pipe->active_slots.find(desc_set_bind_index) != pipe->active_slots.end()) {
            replace_shaders = true;
        }
        // If the app requests all available sets, the pipeline layout was not modified at pipeline layout creation and the
        // already instrumented shaders need to be replaced with uninstrumented shaders
        const auto pipeline_layout = pipe->PipelineLayoutState();
        if (pipeline_layout && pipeline_layout->set_layouts.size() >= adjusted_max_desc_sets) {
            replace_shaders = true;
        }

        if (replace_shaders) {
            for (uint32_t i = 0; i < static_cast<uint32_t>(pipe->stage_states.size()); ++i) {
                const auto &stage = pipe->stage_states[i];
                const auto &spirv_state = stage.spirv_state;

                VkShaderModule shader_module;
                VkShaderModuleCreateInfo create_info = vku::InitStructHelper();
                create_info.pCode = spirv_state->words_.data();
                create_info.codeSize = spirv_state->words_.size() * sizeof(uint32_t);
                VkResult result = DispatchCreateShaderModule(device, &create_info, pAllocator, &shader_module);
                if (result == VK_SUCCESS) {
                    SetShaderModule(new_pipeline_ci, *stage.pipeline_create_info, shader_module, i);
                } else {
                    ReportSetupProblem(device,
                                       "Unable to replace instrumented shader with non-instrumented one.  "
                                       "Device could become unstable.");
                }
            }
        } else {
            // !replace_shaders implies that the instrumented shaders should be used. However, if this is a non-executable pipeline
            // library created with pre-raster or fragment shader state, it contains shaders that have not yet been instrumented
            if (!pipe->HasFullState() && (pipe->pre_raster_state || pipe->fragment_shader_state)) {
                for (const auto &stage_state : pipe->stage_states) {
                    auto module_state = std::const_pointer_cast<SHADER_MODULE_STATE>(stage_state.module_state);
                    if (!module_state->Handle()) {
                        // If the shader module's handle is non-null, then it was defined with CreateShaderModule and covered by the
                        // case above. Otherwise, it is being defined during CGPL time
                        if (cgpl_state.shader_states.size() <= pipeline) {
                            cgpl_state.shader_states.resize(pipeline + 1);
                        }
                        const VkShaderStageFlagBits stage = stage_state.GetStage();
                        // Now find the corresponding VkShaderModuleCreateInfo
                        auto &stage_ci =
                            GetShaderStageCI<SafeCreateInfo, safe_VkPipelineShaderStageCreateInfo>(new_pipeline_ci, stage);
                        // We're modifying the copied, safe create info, which is ok to be non-const
                        auto sm_ci =
                            const_cast<safe_VkShaderModuleCreateInfo *>(reinterpret_cast<const safe_VkShaderModuleCreateInfo *>(
                                vku::FindStructInPNextChain<VkShaderModuleCreateInfo>(stage_ci.pNext)));
                        if (gpuav_settings.select_instrumented_shaders && sm_ci && !CheckForGpuAvEnabled(sm_ci->pNext)) continue;
                        auto &csm_state = cgpl_state.shader_states[pipeline][stage];
                        bool cached = false;
                        bool pass = false;
                        if (gpuav_settings.cache_instrumented_shaders) {
                            csm_state.unique_shader_id = ValidationCache::MakeShaderHash(module_state->spirv->words_.data(),
                                                                                         module_state->spirv->words_.size());
                            auto it = instrumented_shaders.find(csm_state.unique_shader_id);
                            if (it != instrumented_shaders.end()) {
                                csm_state.instrumented_spirv = it->second.second;
                                cached = true;
                            }
                        } else {
                            csm_state.unique_shader_id = unique_shader_module_id++;
                        }
                        if (!cached) {
                            pass = InstrumentShader(module_state->spirv->words_, csm_state.instrumented_spirv,
                                                    csm_state.unique_shader_id);
                        }
                        if (cached || pass) {
                            module_state->gpu_validation_shader_id = csm_state.unique_shader_id;
                            // Now we need to update the shader code in VkShaderModuleCreateInfo
                            // module_state->Handle() == VK_NULL_HANDLE should imply sm_ci != nullptr, but checking here anyway
                            if (sm_ci) {
                                sm_ci->SetCode(csm_state.instrumented_spirv);
                            }
                            if (gpuav_settings.cache_instrumented_shaders && !cached) {
                                instrumented_shaders.emplace(
                                    csm_state.unique_shader_id,
                                    std::make_pair(csm_state.instrumented_spirv.size(), csm_state.instrumented_spirv));
                            }
                        }
                    }
                }
            }
        }
        new_pipeline_create_infos->push_back(std::move(new_pipeline_ci));
    }
}
// For every pipeline:
// - For every shader in a pipeline:
//   - If the shader had to be replaced in PreCallRecord (because the pipeline is using the debug desc set index):
//     - Destroy it since it has been bound into the pipeline by now.  This is our only chance to delete it.
//   - Track the shader in the shader_map
//   - Save the shader binary if it contains debug code
template <typename CreateInfo, typename SafeCreateInfo>
void GpuAssistedBase::PostCallRecordPipelineCreations(const uint32_t count, const CreateInfo *pCreateInfos,
                                                      const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                      const VkPipelineBindPoint bind_point,
                                                      const SafeCreateInfo &modified_create_infos) {
    if (bind_point != VK_PIPELINE_BIND_POINT_GRAPHICS && bind_point != VK_PIPELINE_BIND_POINT_COMPUTE &&
        bind_point != VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR) {
        return;
    }
    for (uint32_t pipeline = 0; pipeline < count; ++pipeline) {
        auto pipeline_state = Get<PIPELINE_STATE>(pPipelines[pipeline]);
        if (!pipeline_state) continue;

        if (!pipeline_state->stage_states.empty() && !(pipeline_state->create_flags & VK_PIPELINE_CREATE_LIBRARY_BIT_KHR)) {
            const auto pipeline_layout = pipeline_state->PipelineLayoutState();
            for (auto &stage_state : pipeline_state->stage_states) {
                auto &module_state = stage_state.module_state;
                const auto shader_module = module_state->Handle();

                if (pipeline_state->active_slots.find(desc_set_bind_index) != pipeline_state->active_slots.end() ||
                    (pipeline_layout->set_layouts.size() >= adjusted_max_desc_sets)) {
                    auto *modified_ci = reinterpret_cast<const CreateInfo *>(modified_create_infos[pipeline].ptr());
                    auto uninstrumented_module = GetShaderModule(*modified_ci, stage_state.GetStage());
                    assert(uninstrumented_module != shader_module.Cast<VkShaderModule>());
                    DispatchDestroyShaderModule(device, uninstrumented_module, pAllocator);
                }

                std::vector<unsigned int> code;
                // Save the shader binary
                // The core_validation ShaderModule tracker saves the binary too, but discards it when the ShaderModule
                // is destroyed.  Applications may destroy ShaderModules after they are placed in a pipeline and before
                // the pipeline is used, so we have to keep another copy.
                if (module_state && module_state->spirv) code = module_state->spirv->words_;

                shader_map.insert_or_assign(module_state->gpu_validation_shader_id, pipeline_state->pipeline(),
                                            shader_module.Cast<VkShaderModule>(), VK_NULL_HANDLE, std::move(code));
            }
        }
    }
}
