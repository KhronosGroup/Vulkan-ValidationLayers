/* Copyright (c) 2020-2024 The Khronos Group Inc.
 * Copyright (c) 2020-2024 Valve Corporation
 * Copyright (c) 2020-2024 LunarG, Inc.
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

#include "gpu_validation/gpu_shader_instrumentor.h"
#include "gpu_validation/gpu_state_tracker.h"
#include "chassis/chassis_modification_state.h"

// Implementation for Descriptor Set Manager class
DescriptorSetManager::DescriptorSetManager(VkDevice device, uint32_t num_bindings_in_set)
    : device(device), num_bindings_in_set(num_bindings_in_set) {}

DescriptorSetManager::~DescriptorSetManager() {
    for (auto &pool : desc_pool_map_) {
        DispatchDestroyDescriptorPool(device, pool.first, nullptr);
    }
    desc_pool_map_.clear();
}

VkResult DescriptorSetManager::GetDescriptorSet(VkDescriptorPool *out_desc_pool, VkDescriptorSetLayout ds_layout,
                                                VkDescriptorSet *out_desc_sets) {
    std::vector<VkDescriptorSet> desc_sets;
    VkResult result = GetDescriptorSets(1, out_desc_pool, ds_layout, &desc_sets);
    assert(result == VK_SUCCESS);
    if (result == VK_SUCCESS) {
        *out_desc_sets = desc_sets[0];
    }
    return result;
}

VkResult DescriptorSetManager::GetDescriptorSets(uint32_t count, VkDescriptorPool *out_pool, VkDescriptorSetLayout ds_layout,
                                                 std::vector<VkDescriptorSet> *out_desc_sets) {
    auto guard = Lock();
    const uint32_t default_pool_size = kItemsPerChunk;
    VkResult result = VK_SUCCESS;
    VkDescriptorPool pool_to_use = VK_NULL_HANDLE;

    assert(count > 0);
    if (0 == count) {
        return result;
    }
    out_desc_sets->clear();
    out_desc_sets->resize(count);

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

        // TODO: The logic to compute descriptor pool sizes should not be
        // hardcoded like so, should be dynamic depending on the descriptor sets
        // to be created. Not too dramatic as Vulkan will gracefully fail if there is a
        // mismatch between this and created descriptor sets.
        const std::array<VkDescriptorPoolSize, 2> pool_sizes = {{{
                                                                     VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                                                                     pool_count * num_bindings_in_set,
                                                                 },
                                                                 {
                                                                     VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC,
                                                                     pool_count * num_bindings_in_set,
                                                                 }}};

        VkDescriptorPoolCreateInfo desc_pool_info = vku::InitStructHelper();
        desc_pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        desc_pool_info.maxSets = pool_count;
        desc_pool_info.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
        desc_pool_info.pPoolSizes = pool_sizes.data();
        result = DispatchCreateDescriptorPool(device, &desc_pool_info, nullptr, &pool_to_use);
        assert(result == VK_SUCCESS);
        if (result != VK_SUCCESS) {
            return result;
        }
        desc_pool_map_[pool_to_use].size = desc_pool_info.maxSets;
        desc_pool_map_[pool_to_use].used = 0;
    }
    std::vector<VkDescriptorSetLayout> desc_layouts(count, ds_layout);

    VkDescriptorSetAllocateInfo alloc_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO, nullptr, pool_to_use, count,
                                              desc_layouts.data()};

    result = DispatchAllocateDescriptorSets(device, &alloc_info, out_desc_sets->data());
    assert(result == VK_SUCCESS);
    if (result != VK_SUCCESS) {
        return result;
    }
    *out_pool = pool_to_use;
    desc_pool_map_[pool_to_use].used += count;
    return result;
}

void DescriptorSetManager::PutBackDescriptorSet(VkDescriptorPool desc_pool, VkDescriptorSet desc_set) {
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
            DispatchDestroyDescriptorPool(device, desc_pool, nullptr);
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

ReadLockGuard GpuShaderInstrumentor::ReadLock() const {
    if (fine_grained_locking) {
        return ReadLockGuard(validation_object_mutex, std::defer_lock);
    } else {
        return ReadLockGuard(validation_object_mutex);
    }
}

WriteLockGuard GpuShaderInstrumentor::WriteLock() {
    if (fine_grained_locking) {
        return WriteLockGuard(validation_object_mutex, std::defer_lock);
    } else {
        return WriteLockGuard(validation_object_mutex);
    }
}

void GpuShaderInstrumentor::PreCallRecordCreateDevice(VkPhysicalDevice gpu, const VkDeviceCreateInfo *pCreateInfo,
                                                      const VkAllocationCallbacks *pAllocator, VkDevice *pDevice,
                                                      const RecordObject &record_obj,
                                                      vku::safe_VkDeviceCreateInfo *modified_create_info) {
    BaseClass::PreCallRecordCreateDevice(gpu, pCreateInfo, pAllocator, pDevice, record_obj, modified_create_info);
    VkPhysicalDeviceFeatures *features = nullptr;
    // Use a local variable to query features since this method runs in the instance validation object.
    // To avoid confusion and race conditions about which physical device's features are stored in the
    // 'supported_devices' member variable, it will only be set in the device validation objects.
    // See CreateDevice() below.
    VkPhysicalDeviceFeatures gpu_supported_features;
    DispatchGetPhysicalDeviceFeatures(gpu, &gpu_supported_features);

    // See CreateDevice() in chassis.cpp. modified_create_info is a pointer to a safe struct stored on the stack.
    // This code follows the safe struct memory memory management scheme. That is, we must delete any memory
    // remove from the safe struct, and any additions must be allocated in a way that is compatible with
    // the safe struct destructor.
    if (modified_create_info->pEnabledFeatures) {
        // If pEnabledFeatures, VkPhysicalDeviceFeatures2 in pNext chain is not allowed
        features = const_cast<VkPhysicalDeviceFeatures *>(modified_create_info->pEnabledFeatures);
    } else {
        auto *features2 = const_cast<VkPhysicalDeviceFeatures2 *>(
            vku::FindStructInPNextChain<VkPhysicalDeviceFeatures2>(modified_create_info->pNext));
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

    auto add_missing_features = [this, modified_create_info]() {
        if (force_buffer_device_address) {
            // Add buffer device address feature
            auto *bda_features = const_cast<VkPhysicalDeviceBufferDeviceAddressFeatures *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceBufferDeviceAddressFeatures>(modified_create_info));
            if (bda_features) {
                bda_features->bufferDeviceAddress = VK_TRUE;
            } else {
                VkPhysicalDeviceBufferDeviceAddressFeatures new_bda_features = vku::InitStructHelper();
                new_bda_features.bufferDeviceAddress = VK_TRUE;
                vku::AddToPnext(*modified_create_info, new_bda_features);
            }
        }

        // Add timeline semaphore feature
        auto *ts_features = const_cast<VkPhysicalDeviceTimelineSemaphoreFeatures *>(
            vku::FindStructInPNextChain<VkPhysicalDeviceTimelineSemaphoreFeatures>(modified_create_info));
        if (ts_features) {
            ts_features->timelineSemaphore = VK_TRUE;
        } else {
            VkPhysicalDeviceTimelineSemaphoreFeatures new_ts_features = vku::InitStructHelper();
            new_ts_features.timelineSemaphore = VK_TRUE;
            vku::AddToPnext(*modified_create_info, new_ts_features);
        }
    };

    // TODO How to handle multi-device
    if (api_version > VK_API_VERSION_1_1) {
        auto *features12 = const_cast<VkPhysicalDeviceVulkan12Features *>(
            vku::FindStructInPNextChain<VkPhysicalDeviceVulkan12Features>(modified_create_info->pNext));
        if (features12) {
            features12->timelineSemaphore = VK_TRUE;
            if (force_buffer_device_address) {
                features12->bufferDeviceAddress = VK_TRUE;
            }
        } else {
            add_missing_features();
        }
    } else if (api_version == VK_API_VERSION_1_1) {
        const std::string_view bda_ext{"VK_KHR_buffer_device_address"};
        bool found_bda = false;
        for (uint32_t i = 0; i < modified_create_info->enabledExtensionCount; i++) {
            if (bda_ext == modified_create_info->ppEnabledExtensionNames[i]) {
                found_bda = true;
                break;
            }
        }
        const std::string_view ts_ext{"VK_KHR_timeline_semaphore"};
        bool found_ts = false;
        for (uint32_t i = 0; i < modified_create_info->enabledExtensionCount; i++) {
            if (ts_ext == modified_create_info->ppEnabledExtensionNames[i]) {
                found_ts = true;
                break;
            }
        }

        // Add our new extensions
        if (!found_bda) {
            vku::AddExtension(*modified_create_info, bda_ext.data());
        }
        if (!found_ts) {
            vku::AddExtension(*modified_create_info, ts_ext.data());
        }

        add_missing_features();
    } else {
        force_buffer_device_address = false;
    }
}

std::shared_ptr<vvl::Queue> GpuShaderInstrumentor::CreateQueue(VkQueue q, uint32_t index, VkDeviceQueueCreateFlags flags,
                                                               const VkQueueFamilyProperties &queueFamilyProperties) {
    return std::static_pointer_cast<vvl::Queue>(
        std::make_shared<gpu_tracker::Queue>(*this, q, index, flags, queueFamilyProperties));
}

// In charge of getting things for shader instrumentation that both GPU-AV and DebugPrintF will need
void GpuShaderInstrumentor::CreateDevice(const VkDeviceCreateInfo *pCreateInfo, const Location &loc) {
    BaseClass::CreateDevice(pCreateInfo, loc);
    // If api version 1.1 or later, SetDeviceLoaderData will be in the loader
    auto chain_info = GetChainInfo(pCreateInfo, VK_LOADER_DATA_CALLBACK);
    assert(chain_info->u.pfnSetDeviceLoaderData);
    vkSetDeviceLoaderData = chain_info->u.pfnSetDeviceLoaderData;

    // Some devices have extremely high limits here, so set a reasonable max because we have to pad
    // the pipeline layout with dummy descriptor set layouts.
    adjusted_max_desc_sets = phys_dev_props.limits.maxBoundDescriptorSets;
    adjusted_max_desc_sets = std::min(33U, adjusted_max_desc_sets);

    // We can't do anything if there is only one.
    // Device probably not a legit Vulkan device, since there should be at least 4. Protect ourselves.
    if (adjusted_max_desc_sets == 1) {
        ReportSetupProblem(device, loc, "Device can bind only a single descriptor set.");
        aborted = true;
        return;
    }

    desc_set_bind_index = adjusted_max_desc_sets - 1;

    VkResult result = UtilInitializeVma(instance, physical_device, device, force_buffer_device_address, &vmaAllocator);
    if (result != VK_SUCCESS) {
        ReportSetupProblem(device, loc, "Could not initialize VMA", true);
        aborted = true;
        return;
    }

    desc_set_manager = std::make_unique<DescriptorSetManager>(device, static_cast<uint32_t>(validation_bindings_.size()));

    const VkDescriptorSetLayoutCreateInfo debug_desc_layout_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO, nullptr, 0,
                                                                    static_cast<uint32_t>(validation_bindings_.size()),
                                                                    validation_bindings_.data()};

    result = DispatchCreateDescriptorSetLayout(device, &debug_desc_layout_info, nullptr, &debug_desc_layout_);
    if (result != VK_SUCCESS) {
        ReportSetupProblem(device, loc, "vkCreateDescriptorSetLayout failed for internal descriptor set");
        Cleanup();
        aborted = true;
        return;
    }

    const VkDescriptorSetLayoutCreateInfo dummy_desc_layout_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO, nullptr, 0,
                                                                    0, nullptr};
    result = DispatchCreateDescriptorSetLayout(device, &dummy_desc_layout_info, nullptr, &dummy_desc_layout_);
    if (result != VK_SUCCESS) {
        ReportSetupProblem(device, loc, "vkCreateDescriptorSetLayout failed for internal dummy descriptor set");
        Cleanup();
        aborted = true;
        return;
    }

    std::vector<VkDescriptorSetLayout> debug_layouts;
    for (uint32_t j = 0; j < adjusted_max_desc_sets - 1; ++j) {
        debug_layouts.push_back(dummy_desc_layout_);
    }
    debug_layouts.push_back(debug_desc_layout_);
    const VkPipelineLayoutCreateInfo debug_pipeline_layout_info = {VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
                                                                   nullptr,
                                                                   0u,
                                                                   static_cast<uint32_t>(debug_layouts.size()),
                                                                   debug_layouts.data(),
                                                                   0u,
                                                                   nullptr};
    result = DispatchCreatePipelineLayout(device, &debug_pipeline_layout_info, nullptr, &debug_pipeline_layout_);
    if (result != VK_SUCCESS) {
        ReportSetupProblem(device, loc, "vkCreateDescriptorSetLayout failed for internal pipeline layout");
        Cleanup();
        aborted = true;
        return;
    }
}

void GpuShaderInstrumentor::Cleanup() {
    if (debug_desc_layout_) {
        DispatchDestroyDescriptorSetLayout(device, debug_desc_layout_, nullptr);
        debug_desc_layout_ = VK_NULL_HANDLE;
    }
    if (dummy_desc_layout_) {
        DispatchDestroyDescriptorSetLayout(device, dummy_desc_layout_, nullptr);
        dummy_desc_layout_ = VK_NULL_HANDLE;
    }
    if (debug_pipeline_layout_) {
        DispatchDestroyPipelineLayout(device, debug_pipeline_layout_, nullptr);
        debug_pipeline_layout_ = VK_NULL_HANDLE;
    }
}

void GpuShaderInstrumentor::PreCallRecordDestroyDevice(VkDevice device, const VkAllocationCallbacks *pAllocator,
                                                       const RecordObject &record_obj) {
    indices_buffer.Destroy(vmaAllocator);

    Cleanup();

    BaseClass::PreCallRecordDestroyDevice(device, pAllocator, record_obj);
    // State Tracker can end up making vma calls through callbacks - don't destroy allocator until ST is done
    if (output_buffer_pool) {
        vmaDestroyPool(vmaAllocator, output_buffer_pool);
    }
    if (vmaAllocator) {
        vmaDestroyAllocator(vmaAllocator);
    }
    desc_set_manager.reset();
}

// Just gives a warning about a possible deadlock.
bool GpuShaderInstrumentor::ValidateCmdWaitEvents(VkCommandBuffer command_buffer, VkPipelineStageFlags2 src_stage_mask,
                                                  const Location &loc) const {
    if (src_stage_mask & VK_PIPELINE_STAGE_2_HOST_BIT) {
        std::ostringstream error_msg;
        error_msg << loc.Message()
                  << ": recorded with VK_PIPELINE_STAGE_HOST_BIT set. GPU-Assisted validation waits on queue completion. This wait "
                     "could block the host's signaling of this event, resulting in deadlock.";
        ReportSetupProblem(command_buffer, loc, error_msg.str().c_str());
    }
    return false;
}

bool GpuShaderInstrumentor::PreCallValidateCmdWaitEvents(
    VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents, VkPipelineStageFlags srcStageMask,
    VkPipelineStageFlags dstStageMask, uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers,
    uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier *pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount,
    const VkImageMemoryBarrier *pImageMemoryBarriers, const ErrorObject &error_obj) const {
    BaseClass::PreCallValidateCmdWaitEvents(commandBuffer, eventCount, pEvents, srcStageMask, dstStageMask, memoryBarrierCount,
                                            pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers,
                                            imageMemoryBarrierCount, pImageMemoryBarriers, error_obj);
    return ValidateCmdWaitEvents(commandBuffer, static_cast<VkPipelineStageFlags2>(srcStageMask), error_obj.location);
}

bool GpuShaderInstrumentor::PreCallValidateCmdWaitEvents2KHR(VkCommandBuffer commandBuffer, uint32_t eventCount,
                                                             const VkEvent *pEvents, const VkDependencyInfoKHR *pDependencyInfos,
                                                             const ErrorObject &error_obj) const {
    return PreCallValidateCmdWaitEvents2(commandBuffer, eventCount, pEvents, pDependencyInfos, error_obj);
}

bool GpuShaderInstrumentor::PreCallValidateCmdWaitEvents2(VkCommandBuffer commandBuffer, uint32_t eventCount,
                                                          const VkEvent *pEvents, const VkDependencyInfo *pDependencyInfos,
                                                          const ErrorObject &error_obj) const {
    VkPipelineStageFlags2 src_stage_mask = 0;

    for (uint32_t i = 0; i < eventCount; i++) {
        auto stage_masks = sync_utils::GetGlobalStageMasks(pDependencyInfos[i]);
        src_stage_mask |= stage_masks.src;
    }

    BaseClass::PreCallValidateCmdWaitEvents2(commandBuffer, eventCount, pEvents, pDependencyInfos, error_obj);
    return ValidateCmdWaitEvents(commandBuffer, src_stage_mask, error_obj.location);
}

void GpuShaderInstrumentor::PreCallRecordCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo *pCreateInfo,
                                                              const VkAllocationCallbacks *pAllocator,
                                                              VkPipelineLayout *pPipelineLayout, const RecordObject &record_obj,
                                                              chassis::CreatePipelineLayout &chassis_state) {
    if (aborted) {
        return;
    }
    if (chassis_state.modified_create_info.setLayoutCount >= adjusted_max_desc_sets) {
        std::ostringstream strm;
        strm << "Pipeline Layout conflict with validation's descriptor set at slot " << desc_set_bind_index << ". "
             << "Application has too many descriptor sets in the pipeline layout to continue with gpu validation. "
             << "Validation is not modifying the pipeline layout. "
             << "Instrumented shaders are replaced with non-instrumented shaders.";
        ReportSetupProblem(device, record_obj.location, strm.str().c_str());
    } else {
        // Modify the pipeline layout by:
        // 1. Copying the caller's descriptor set desc_layouts
        // 2. Fill in dummy descriptor layouts up to the max binding
        // 3. Fill in with the debug descriptor layout at the max binding slot
        chassis_state.new_layouts.reserve(adjusted_max_desc_sets);
        chassis_state.new_layouts.insert(chassis_state.new_layouts.end(), &pCreateInfo->pSetLayouts[0],
                                         &pCreateInfo->pSetLayouts[pCreateInfo->setLayoutCount]);
        for (uint32_t i = pCreateInfo->setLayoutCount; i < adjusted_max_desc_sets - 1; ++i) {
            chassis_state.new_layouts.push_back(dummy_desc_layout_);
        }
        chassis_state.new_layouts.push_back(debug_desc_layout_);
        chassis_state.modified_create_info.pSetLayouts = chassis_state.new_layouts.data();
        chassis_state.modified_create_info.setLayoutCount = adjusted_max_desc_sets;
    }
    BaseClass::PreCallRecordCreatePipelineLayout(device, pCreateInfo, pAllocator, pPipelineLayout, record_obj, chassis_state);
}

void GpuShaderInstrumentor::PostCallRecordCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo *pCreateInfo,
                                                               const VkAllocationCallbacks *pAllocator,
                                                               VkPipelineLayout *pPipelineLayout, const RecordObject &record_obj) {
    if (record_obj.result != VK_SUCCESS) {
        ReportSetupProblem(device, record_obj.location, "Unable to create pipeline layout.  Device could become unstable.");
        aborted = true;
    }
    BaseClass::PostCallRecordCreatePipelineLayout(device, pCreateInfo, pAllocator, pPipelineLayout, record_obj);
}

void GpuShaderInstrumentor::PreCallRecordCreateShadersEXT(VkDevice device, uint32_t createInfoCount,
                                                          const VkShaderCreateInfoEXT *pCreateInfos,
                                                          const VkAllocationCallbacks *pAllocator, VkShaderEXT *pShaders,
                                                          const RecordObject &record_obj, chassis::ShaderObject &chassis_state) {
    if (aborted) {
        return;
    }
    for (uint32_t i = 0; i < createInfoCount; ++i) {
        if (chassis_state.instrumented_create_info->setLayoutCount >= adjusted_max_desc_sets) {
            std::ostringstream strm;
            strm << "Descriptor Set Layout conflict with validation's descriptor set at slot " << desc_set_bind_index << ". "
                 << "Application has too many descriptor sets in the pipeline layout to continue with gpu validation. "
                 << "Validation is not modifying the pipeline layout. "
                 << "Instrumented shaders are replaced with non-instrumented shaders.";
            ReportSetupProblem(device, record_obj.location, strm.str().c_str());
        } else {
            // Modify the pipeline layout by:
            // 1. Copying the caller's descriptor set desc_layouts
            // 2. Fill in dummy descriptor layouts up to the max binding
            // 3. Fill in with the debug descriptor layout at the max binding slot
            chassis_state.new_layouts.reserve(adjusted_max_desc_sets);
            chassis_state.new_layouts.insert(chassis_state.new_layouts.end(), pCreateInfos[i].pSetLayouts,
                                             &pCreateInfos[i].pSetLayouts[pCreateInfos[i].setLayoutCount]);
            for (uint32_t j = pCreateInfos[i].setLayoutCount; j < adjusted_max_desc_sets - 1; ++j) {
                chassis_state.new_layouts.push_back(dummy_desc_layout_);
            }
            chassis_state.new_layouts.push_back(debug_desc_layout_);
            chassis_state.instrumented_create_info->pSetLayouts = chassis_state.new_layouts.data();
            chassis_state.instrumented_create_info->setLayoutCount = adjusted_max_desc_sets;
        }
    }
}

void GpuShaderInstrumentor::PostCallRecordCreateShadersEXT(VkDevice device, uint32_t createInfoCount,
                                                           const VkShaderCreateInfoEXT *pCreateInfos,
                                                           const VkAllocationCallbacks *pAllocator, VkShaderEXT *pShaders,
                                                           const RecordObject &record_obj, chassis::ShaderObject &chassis_state) {
    BaseClass::PostCallRecordCreateShadersEXT(device, createInfoCount, pCreateInfos, pAllocator, pShaders, record_obj,
                                              chassis_state);
    if (aborted) return;

    for (uint32_t i = 0; i < createInfoCount; ++i) {
        shader_map.insert_or_assign(chassis_state.unique_shader_ids[i], VK_NULL_HANDLE, VK_NULL_HANDLE, pShaders[i],
                                    chassis_state.instrumented_spirv[i]);
    }
}

void GpuShaderInstrumentor::PreCallRecordDestroyShaderEXT(VkDevice device, VkShaderEXT shader,
                                                          const VkAllocationCallbacks *pAllocator, const RecordObject &record_obj) {
    auto to_erase = shader_map.snapshot([shader](const GpuAssistedShaderTracker &entry) { return entry.shader_object == shader; });
    for (const auto &entry : to_erase) {
        shader_map.erase(entry.first);
    }
    BaseClass::PreCallRecordDestroyShaderEXT(device, shader, pAllocator, record_obj);
}

void GpuShaderInstrumentor::PreCallRecordCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                                 const VkGraphicsPipelineCreateInfo *pCreateInfos,
                                                                 const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                                 const RecordObject &record_obj, PipelineStates &pipeline_states,
                                                                 chassis::CreateGraphicsPipelines &chassis_state) {
    if (aborted) return;
    std::vector<vku::safe_VkGraphicsPipelineCreateInfo> new_pipeline_create_infos;
    PreCallRecordPipelineCreations(count, pCreateInfos, pAllocator, pPipelines, pipeline_states, &new_pipeline_create_infos,
                                   record_obj, chassis_state);
    chassis_state.modified_create_infos = new_pipeline_create_infos;
    chassis_state.pCreateInfos = reinterpret_cast<VkGraphicsPipelineCreateInfo *>(chassis_state.modified_create_infos.data());
}

void GpuShaderInstrumentor::PreCallRecordCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                                const VkComputePipelineCreateInfo *pCreateInfos,
                                                                const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                                const RecordObject &record_obj, PipelineStates &pipeline_states,
                                                                chassis::CreateComputePipelines &chassis_state) {
    if (aborted) return;
    std::vector<vku::safe_VkComputePipelineCreateInfo> new_pipeline_create_infos;
    PreCallRecordPipelineCreations(count, pCreateInfos, pAllocator, pPipelines, pipeline_states, &new_pipeline_create_infos,
                                   record_obj, chassis_state);
    chassis_state.modified_create_infos = new_pipeline_create_infos;
    chassis_state.pCreateInfos = reinterpret_cast<VkComputePipelineCreateInfo *>(chassis_state.modified_create_infos.data());
}

void GpuShaderInstrumentor::PreCallRecordCreateRayTracingPipelinesNV(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                                     const VkRayTracingPipelineCreateInfoNV *pCreateInfos,
                                                                     const VkAllocationCallbacks *pAllocator,
                                                                     VkPipeline *pPipelines, const RecordObject &record_obj,
                                                                     PipelineStates &pipeline_states,
                                                                     chassis::CreateRayTracingPipelinesNV &chassis_state) {
    if (aborted) return;
    std::vector<vku::safe_VkRayTracingPipelineCreateInfoCommon> new_pipeline_create_infos;
    PreCallRecordPipelineCreations(count, pCreateInfos, pAllocator, pPipelines, pipeline_states, &new_pipeline_create_infos,
                                   record_obj, chassis_state);
    chassis_state.modified_create_infos = new_pipeline_create_infos;
    chassis_state.pCreateInfos = reinterpret_cast<VkRayTracingPipelineCreateInfoNV *>(chassis_state.modified_create_infos.data());
}

void GpuShaderInstrumentor::PreCallRecordCreateRayTracingPipelinesKHR(
    VkDevice device, VkDeferredOperationKHR deferredOperation, VkPipelineCache pipelineCache, uint32_t count,
    const VkRayTracingPipelineCreateInfoKHR *pCreateInfos, const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
    const RecordObject &record_obj, PipelineStates &pipeline_states, chassis::CreateRayTracingPipelinesKHR &chassis_state) {
    if (aborted) return;
    std::vector<vku::safe_VkRayTracingPipelineCreateInfoCommon> new_pipeline_create_infos;
    PreCallRecordPipelineCreations(count, pCreateInfos, pAllocator, pPipelines, pipeline_states, &new_pipeline_create_infos,
                                   record_obj, chassis_state);
    chassis_state.modified_create_infos = new_pipeline_create_infos;
    chassis_state.pCreateInfos = reinterpret_cast<VkRayTracingPipelineCreateInfoKHR *>(chassis_state.modified_create_infos.data());
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

void GpuShaderInstrumentor::PostCallRecordCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                                  const VkGraphicsPipelineCreateInfo *pCreateInfos,
                                                                  const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                                  const RecordObject &record_obj, PipelineStates &pipeline_states,
                                                                  chassis::CreateGraphicsPipelines &chassis_state) {
    BaseClass::PostCallRecordCreateGraphicsPipelines(device, pipelineCache, count, pCreateInfos, pAllocator, pPipelines, record_obj,
                                                     pipeline_states, chassis_state);
    if (aborted) return;
    UtilCopyCreatePipelineFeedbackData(count, pCreateInfos, chassis_state.modified_create_infos.data());
    PostCallRecordPipelineCreations(count, pCreateInfos, pAllocator, pPipelines, chassis_state.modified_create_infos.data());
}

void GpuShaderInstrumentor::PostCallRecordCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                                 const VkComputePipelineCreateInfo *pCreateInfos,
                                                                 const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                                 const RecordObject &record_obj, PipelineStates &pipeline_states,
                                                                 chassis::CreateComputePipelines &chassis_state) {
    BaseClass::PostCallRecordCreateComputePipelines(device, pipelineCache, count, pCreateInfos, pAllocator, pPipelines, record_obj,
                                                    pipeline_states, chassis_state);
    if (aborted) return;
    UtilCopyCreatePipelineFeedbackData(count, pCreateInfos, chassis_state.modified_create_infos.data());
    PostCallRecordPipelineCreations(count, pCreateInfos, pAllocator, pPipelines, chassis_state.modified_create_infos.data());
}

void GpuShaderInstrumentor::PostCallRecordCreateRayTracingPipelinesNV(
    VkDevice device, VkPipelineCache pipelineCache, uint32_t count, const VkRayTracingPipelineCreateInfoNV *pCreateInfos,
    const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines, const RecordObject &record_obj,
    PipelineStates &pipeline_states, chassis::CreateRayTracingPipelinesNV &chassis_state) {
    BaseClass::PostCallRecordCreateRayTracingPipelinesNV(device, pipelineCache, count, pCreateInfos, pAllocator, pPipelines,
                                                         record_obj, pipeline_states, chassis_state);
    if (aborted) return;
    UtilCopyCreatePipelineFeedbackData(count, pCreateInfos, chassis_state.modified_create_infos.data());
    PostCallRecordPipelineCreations(count, pCreateInfos, pAllocator, pPipelines, chassis_state.modified_create_infos.data());
}

void GpuShaderInstrumentor::PostCallRecordCreateRayTracingPipelinesKHR(
    VkDevice device, VkDeferredOperationKHR deferredOperation, VkPipelineCache pipelineCache, uint32_t count,
    const VkRayTracingPipelineCreateInfoKHR *pCreateInfos, const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
    const RecordObject &record_obj, PipelineStates &pipeline_states, chassis::CreateRayTracingPipelinesKHR &chassis_state) {
    BaseClass::PostCallRecordCreateRayTracingPipelinesKHR(device, deferredOperation, pipelineCache, count, pCreateInfos, pAllocator,
                                                          pPipelines, record_obj, pipeline_states, chassis_state);
    if (aborted) return;
    UtilCopyCreatePipelineFeedbackData(count, pCreateInfos, chassis_state.modified_create_infos.data());
    PostCallRecordPipelineCreations(count, pCreateInfos, pAllocator, pPipelines, chassis_state.modified_create_infos.data());
}

// Remove all the shader trackers associated with this destroyed pipeline.
void GpuShaderInstrumentor::PreCallRecordDestroyPipeline(VkDevice device, VkPipeline pipeline,
                                                         const VkAllocationCallbacks *pAllocator, const RecordObject &record_obj) {
    auto to_erase = shader_map.snapshot([pipeline](const GpuAssistedShaderTracker &entry) { return entry.pipeline == pipeline; });
    for (const auto &entry : to_erase) {
        shader_map.erase(entry.first);
    }
    BaseClass::PreCallRecordDestroyPipeline(device, pipeline, pAllocator, record_obj);
}

template <typename CreateInfo>
VkShaderModule GetShaderModule(const CreateInfo &create_info, VkShaderStageFlagBits stage) {
    for (uint32_t i = 0; i < create_info.stageCount; ++i) {
        if (create_info.pStages[i].stage == stage) {
            return create_info.pStages[i].module;
        }
    }
    return {};
}

template <>
VkShaderModule GetShaderModule(const VkComputePipelineCreateInfo &create_info, VkShaderStageFlagBits) {
    return create_info.stage.module;
}

template <typename SafeType>
void SetShaderModule(SafeType &create_info, const vku::safe_VkPipelineShaderStageCreateInfo &stage_info,
                     VkShaderModule shader_module, uint32_t stage_ci_index) {
    create_info.pStages[stage_ci_index] = stage_info;
    create_info.pStages[stage_ci_index].module = shader_module;
}

template <>
void SetShaderModule(vku::safe_VkComputePipelineCreateInfo &create_info,
                     const vku::safe_VkPipelineShaderStageCreateInfo &stage_info, VkShaderModule shader_module,
                     uint32_t stage_ci_index) {
    assert(stage_ci_index == 0);
    create_info.stage = stage_info;
    create_info.stage.module = shader_module;
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
vku::safe_VkPipelineShaderStageCreateInfo &GetShaderStageCI(vku::safe_VkComputePipelineCreateInfo &ci, VkShaderStageFlagBits) {
    return ci.stage;
}

bool GpuShaderInstrumentor::CheckForGpuAvEnabled(const void *pNext) {
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
template <typename CreateInfo, typename SafeCreateInfo, typename ChassisState>
void GpuShaderInstrumentor::PreCallRecordPipelineCreations(uint32_t count, const CreateInfo *pCreateInfos,
                                                           const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                           PipelineStates &pipeline_states,
                                                           std::vector<SafeCreateInfo> *new_pipeline_create_infos,
                                                           const RecordObject &record_obj, ChassisState &chassis_state) {
    // Walk through all the pipelines, make a copy of each and flag each pipeline that contains a shader that uses the debug
    // descriptor set index.
    for (uint32_t pipeline = 0; pipeline < count; ++pipeline) {
        const auto &pipe = pipeline_states[pipeline];
        // NOTE: since these are "safe" CreateInfos, this will create a deep copy via the safe copy constructor
        auto new_pipeline_ci = std::get<SafeCreateInfo>(pipe->create_info);

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
                    ReportSetupProblem(device, record_obj.location,
                                       "Unable to replace instrumented shader with non-instrumented one.  "
                                       "Device could become unstable.");
                }
            }
        } else {
            // !replace_shaders implies that the instrumented shaders should be used. However, if this is a non-executable pipeline
            // library created with pre-raster or fragment shader state, it contains shaders that have not yet been instrumented
            if (!pipe->HasFullState() && (pipe->pre_raster_state || pipe->fragment_shader_state)) {
                for (const auto &stage_state : pipe->stage_states) {
                    auto module_state = std::const_pointer_cast<vvl::ShaderModule>(stage_state.module_state);
                    if (!module_state->Handle()) {
                        // If the shader module's handle is non-null, then it was defined with CreateShaderModule and covered by the
                        // case above. Otherwise, it is being defined during CGPL time
                        if (chassis_state.shader_unique_id_maps.size() <= pipeline) {
                            chassis_state.shader_unique_id_maps.resize(pipeline + 1);
                        }
                        const VkShaderStageFlagBits stage = stage_state.GetStage();
                        // Now find the corresponding VkShaderModuleCreateInfo
                        auto &stage_ci =
                            GetShaderStageCI<SafeCreateInfo, vku::safe_VkPipelineShaderStageCreateInfo>(new_pipeline_ci, stage);
                        // We're modifying the copied, safe create info, which is ok to be non-const
                        auto sm_ci = const_cast<vku::safe_VkShaderModuleCreateInfo *>(
                            reinterpret_cast<const vku::safe_VkShaderModuleCreateInfo *>(
                                vku::FindStructInPNextChain<VkShaderModuleCreateInfo>(stage_ci.pNext)));
                        if (gpuav_settings.select_instrumented_shaders && sm_ci && !CheckForGpuAvEnabled(sm_ci->pNext)) continue;
                        uint32_t unique_shader_id = 0;
                        bool cached = false;
                        bool pass = false;
                        std::vector<uint32_t> instrumented_spirv;
                        if (gpuav_settings.cache_instrumented_shaders) {
                            unique_shader_id =
                                hash_util::ShaderHash(module_state->spirv->words_.data(), module_state->spirv->words_.size());
                            auto it = instrumented_shaders.find(unique_shader_id);
                            if (it != instrumented_shaders.end()) {
                                instrumented_spirv = it->second.second;
                                cached = true;
                            }
                        } else {
                            unique_shader_id = unique_shader_module_id++;
                        }
                        if (!cached) {
                            pass = InstrumentShader(module_state->spirv->words_, instrumented_spirv, unique_shader_id,
                                                    record_obj.location);
                        }
                        if (cached || pass) {
                            module_state->gpu_validation_shader_id = unique_shader_id;
                            // Now we need to update the shader code in VkShaderModuleCreateInfo
                            // module_state->Handle() == VK_NULL_HANDLE should imply sm_ci != nullptr, but checking here anyway
                            if (sm_ci) {
                                sm_ci->SetCode(instrumented_spirv);
                            }
                            if (gpuav_settings.cache_instrumented_shaders && !cached) {
                                instrumented_shaders.emplace(unique_shader_id,
                                                             std::make_pair(instrumented_spirv.size(), instrumented_spirv));
                            }
                        }

                        chassis_state.shader_unique_id_maps[pipeline][stage] = unique_shader_id;
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
void GpuShaderInstrumentor::PostCallRecordPipelineCreations(const uint32_t count, const CreateInfo *pCreateInfos,
                                                            const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                            const SafeCreateInfo &modified_create_infos) {
    for (uint32_t pipeline = 0; pipeline < count; ++pipeline) {
        auto pipeline_state = Get<vvl::Pipeline>(pPipelines[pipeline]);
        if (!pipeline_state) continue;

        if (!pipeline_state->stage_states.empty() && !(pipeline_state->create_flags & VK_PIPELINE_CREATE_LIBRARY_BIT_KHR)) {
            const auto pipeline_layout = pipeline_state->PipelineLayoutState();
            for (auto &stage_state : pipeline_state->stage_states) {
                auto &module_state = stage_state.module_state;

                if (pipeline_state->active_slots.find(desc_set_bind_index) != pipeline_state->active_slots.end() ||
                    (pipeline_layout->set_layouts.size() >= adjusted_max_desc_sets)) {
                    auto *modified_ci = reinterpret_cast<const CreateInfo *>(modified_create_infos[pipeline].ptr());
                    auto uninstrumented_module = GetShaderModule(*modified_ci, stage_state.GetStage());
                    assert(uninstrumented_module != module_state->VkHandle());
                    DispatchDestroyShaderModule(device, uninstrumented_module, pAllocator);
                }

                std::vector<unsigned int> code;
                // Save the shader binary
                // The core_validation ShaderModule tracker saves the binary too, but discards it when the ShaderModule
                // is destroyed.  Applications may destroy ShaderModules after they are placed in a pipeline and before
                // the pipeline is used, so we have to keep another copy.
                if (module_state && module_state->spirv) code = module_state->spirv->words_;

                shader_map.insert_or_assign(module_state->gpu_validation_shader_id, pipeline_state->VkHandle(),
                                            module_state->VkHandle(), VK_NULL_HANDLE, std::move(code));
            }
        }
    }
}
