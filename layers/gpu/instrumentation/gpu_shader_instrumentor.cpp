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

#include "gpu/instrumentation/gpu_shader_instrumentor.h"

#include "gpu/core/gpu_state_tracker.h"
#include "chassis/chassis_modification_state.h"

#include <regex>

namespace gpu {
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

static VkResult UtilInitializeVma(VkInstance instance, VkPhysicalDevice physical_device, VkDevice device,
                                  bool use_buffer_device_address, VmaAllocator *pAllocator) {
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

void SpirvCache::Add(uint32_t hash, std::vector<uint32_t> spirv) { spirv_shaders_.emplace(hash, std::move(spirv)); }

std::vector<uint32_t> *SpirvCache::Get(uint32_t spirv_hash) {
    auto it = spirv_shaders_.find(spirv_hash);
    if (it != spirv_shaders_.end()) {
        return &it->second;
    }
    return nullptr;
}

bool SpirvCache::IsSpirvCached(uint32_t spirv_hash, chassis::CreateShaderModule &chassis_state) const {
    auto it = spirv_shaders_.find(spirv_hash);
    if (it != spirv_shaders_.end()) {
        const std::vector<uint32_t> &spirv = it->second;
        chassis_state.instrumented_create_info.codeSize = spirv.size() * sizeof(uint32_t);
        chassis_state.instrumented_create_info.pCode = spirv.data();
        chassis_state.instrumented_spirv = spirv;
        chassis_state.unique_shader_id = spirv_hash;
        return true;
    }
    return false;
}

bool SpirvCache::IsSpirvCached(uint32_t index, uint32_t spirv_hash, chassis::ShaderObject &chassis_state) const {
    auto it = spirv_shaders_.find(spirv_hash);
    if (it != spirv_shaders_.end()) {
        const std::vector<uint32_t> &spirv = it->second;
        chassis_state.instrumented_create_info[index].codeSize = spirv.size() * sizeof(uint32_t);
        chassis_state.instrumented_create_info[index].pCode = spirv.data();
        return true;
    }
    return false;
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

std::shared_ptr<vvl::Queue> GpuShaderInstrumentor::CreateQueue(VkQueue q, uint32_t family_index, uint32_t queue_index,
                                                               VkDeviceQueueCreateFlags flags,
                                                               const VkQueueFamilyProperties &queueFamilyProperties) {
    return std::static_pointer_cast<vvl::Queue>(
        std::make_shared<gpu_tracker::Queue>(*this, q, family_index, queue_index, flags, queueFamilyProperties));
}

// These are the common things required for anything that deals with shader instrumentation
void GpuShaderInstrumentor::PreCallRecordCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo *pCreateInfo,
                                                      const VkAllocationCallbacks *pAllocator, VkDevice *pDevice,
                                                      const RecordObject &record_obj,
                                                      vku::safe_VkDeviceCreateInfo *modified_create_info) {
    // Force enable required features
    // If the features are not supported, can't internal error until post device creation
    VkPhysicalDeviceFeatures supported_features{};
    DispatchGetPhysicalDeviceFeatures(physicalDevice, &supported_features);

    if (auto enabled_features = const_cast<VkPhysicalDeviceFeatures *>(modified_create_info->pEnabledFeatures)) {
        if (supported_features.fragmentStoresAndAtomics && !enabled_features->fragmentStoresAndAtomics) {
            InternalWarning(device, record_obj.location, "Forcing VkPhysicalDeviceFeatures::fragmentStoresAndAtomics to VK_TRUE");
            enabled_features->fragmentStoresAndAtomics = VK_TRUE;
        }
        if (supported_features.vertexPipelineStoresAndAtomics && !enabled_features->vertexPipelineStoresAndAtomics) {
            InternalWarning(device, record_obj.location,
                            "Forcing VkPhysicalDeviceFeatures::vertexPipelineStoresAndAtomics to VK_TRUE");
            enabled_features->vertexPipelineStoresAndAtomics = VK_TRUE;
        }
    }

    auto add_missing_features = [this, &record_obj, modified_create_info]() {
        // Add timeline semaphore feature - This is required as we use it to manage when command buffers are submitted at queue
        // submit time
        if (auto *ts_features = const_cast<VkPhysicalDeviceTimelineSemaphoreFeatures *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceTimelineSemaphoreFeatures>(modified_create_info))) {
            InternalWarning(device, record_obj.location,
                            "Forcing VkPhysicalDeviceTimelineSemaphoreFeatures::timelineSemaphore to VK_TRUE");
            ts_features->timelineSemaphore = VK_TRUE;
        } else {
            InternalWarning(device, record_obj.location,
                            "Adding a VkPhysicalDeviceTimelineSemaphoreFeatures to pNext with timelineSemaphore set to VK_TRUE");
            VkPhysicalDeviceTimelineSemaphoreFeatures new_ts_features = vku::InitStructHelper();
            new_ts_features.timelineSemaphore = VK_TRUE;
            vku::AddToPnext(*modified_create_info, new_ts_features);
        }
    };

    if (api_version > VK_API_VERSION_1_1) {
        if (auto *features12 = const_cast<VkPhysicalDeviceVulkan12Features *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceVulkan12Features>(modified_create_info->pNext))) {
            InternalWarning(device, record_obj.location, "Forcing VkPhysicalDeviceVulkan12Features::timelineSemaphore to VK_TRUE");
            features12->timelineSemaphore = VK_TRUE;
        } else {
            add_missing_features();
        }
    } else if (api_version == VK_API_VERSION_1_1) {
        // Add our new extensions (will only add if found)
        const std::string_view ts_ext{"VK_KHR_timeline_semaphore"};
        vku::AddExtension(*modified_create_info, ts_ext.data());
        add_missing_features();
    }
}

// In charge of getting things for shader instrumentation that both GPU-AV and DebugPrintF will need
void GpuShaderInstrumentor::PostCreateDevice(const VkDeviceCreateInfo *pCreateInfo, const Location &loc) {
    BaseClass::PostCreateDevice(pCreateInfo, loc);
    // If api version 1.1 or later, SetDeviceLoaderData will be in the loader
    auto chain_info = GetChainInfo(pCreateInfo, VK_LOADER_DATA_CALLBACK);
    assert(chain_info->u.pfnSetDeviceLoaderData);
    vk_set_device_loader_data_ = chain_info->u.pfnSetDeviceLoaderData;

    // maxBoundDescriptorSets limit, but possibly adjusted
    const uint32_t adjusted_max_desc_sets_limit =
        std::min(gpu::kMaxAdjustedBoundDescriptorSet, phys_dev_props.limits.maxBoundDescriptorSets);
    // If gpu_validation_reserve_binding_slot: the max slot is where we reserved
    // else: always use the last possible set as least likely to be used
    desc_set_bind_index_ = adjusted_max_desc_sets_limit - 1;

    // We can't do anything if there is only one.
    // Device probably not a legit Vulkan device, since there should be at least 4. Protect ourselves.
    if (adjusted_max_desc_sets_limit == 1) {
        InternalError(device, loc, "Device can bind only a single descriptor set.");
        return;
    }

    VkResult result = UtilInitializeVma(instance, physical_device, device, force_buffer_device_address_, &vma_allocator_);
    if (result != VK_SUCCESS) {
        InternalError(device, loc, "Could not initialize VMA", true);
        return;
    }

    desc_set_manager_ =
        std::make_unique<gpu::DescriptorSetManager>(device, static_cast<uint32_t>(instrumentation_bindings_.size()));

    const VkDescriptorSetLayoutCreateInfo debug_desc_layout_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO, nullptr, 0,
                                                                    static_cast<uint32_t>(instrumentation_bindings_.size()),
                                                                    instrumentation_bindings_.data()};

    result = DispatchCreateDescriptorSetLayout(device, &debug_desc_layout_info, nullptr, &debug_desc_layout_);
    if (result != VK_SUCCESS) {
        InternalError(device, loc, "vkCreateDescriptorSetLayout failed for internal descriptor set");
        Cleanup();
        return;
    }

    const VkDescriptorSetLayoutCreateInfo dummy_desc_layout_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO, nullptr, 0,
                                                                    0, nullptr};
    result = DispatchCreateDescriptorSetLayout(device, &dummy_desc_layout_info, nullptr, &dummy_desc_layout_);
    if (result != VK_SUCCESS) {
        InternalError(device, loc, "vkCreateDescriptorSetLayout failed for internal dummy descriptor set");
        Cleanup();
        return;
    }

    std::vector<VkDescriptorSetLayout> debug_layouts;
    for (uint32_t j = 0; j < desc_set_bind_index_; ++j) {
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
        InternalError(device, loc, "vkCreateDescriptorSetLayout failed for internal pipeline layout");
        Cleanup();
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
    indices_buffer_.Destroy(vma_allocator_);

    Cleanup();

    BaseClass::PreCallRecordDestroyDevice(device, pAllocator, record_obj);
    // State Tracker can end up making vma calls through callbacks - don't destroy allocator until ST is done
    if (output_buffer_pool_) {
        vmaDestroyPool(vma_allocator_, output_buffer_pool_);
    }
    if (vma_allocator_) {
        vmaDestroyAllocator(vma_allocator_);
    }
    desc_set_manager_.reset();
}

void GpuShaderInstrumentor::ReserveBindingSlot(VkPhysicalDevice physicalDevice, VkPhysicalDeviceLimits &limits,
                                               const Location &loc) {
    // There is an implicit layer that can cause this call to return 0 for maxBoundDescriptorSets - Ignore such calls
    if (limits.maxBoundDescriptorSets == 0) return;

    if (limits.maxBoundDescriptorSets > gpu::kMaxAdjustedBoundDescriptorSet) {
        std::stringstream ss;
        ss << "A descriptor binding slot is required to store GPU-side information, but the device maxBoundDescriptorSets is "
           << limits.maxBoundDescriptorSets << " which is too large, so we will be trying to use slot "
           << gpu::kMaxAdjustedBoundDescriptorSet;
        InternalWarning(physicalDevice, loc, ss.str().c_str());
    }

    if (enabled[gpu_validation_reserve_binding_slot]) {
        if (limits.maxBoundDescriptorSets > 1) {
            limits.maxBoundDescriptorSets -= 1;
        } else {
            InternalWarning(physicalDevice, loc, "Unable to reserve descriptor binding slot on a device with only one slot.");
        }
    }
}

void GpuShaderInstrumentor::PostCallRecordGetPhysicalDeviceProperties(VkPhysicalDevice physicalDevice,
                                                                      VkPhysicalDeviceProperties *device_props,
                                                                      const RecordObject &record_obj) {
    ReserveBindingSlot(physicalDevice, device_props->limits, record_obj.location);
}

void GpuShaderInstrumentor::PostCallRecordGetPhysicalDeviceProperties2(VkPhysicalDevice physicalDevice,
                                                                       VkPhysicalDeviceProperties2 *device_props2,
                                                                       const RecordObject &record_obj) {
    ReserveBindingSlot(physicalDevice, device_props2->properties.limits, record_obj.location);
}

// Just gives a warning about a possible deadlock.
bool GpuShaderInstrumentor::ValidateCmdWaitEvents(VkCommandBuffer command_buffer, VkPipelineStageFlags2 src_stage_mask,
                                                  const Location &loc) const {
    if (src_stage_mask & VK_PIPELINE_STAGE_2_HOST_BIT) {
        std::ostringstream error_msg;
        error_msg << loc.Message()
                  << ": recorded with VK_PIPELINE_STAGE_HOST_BIT set. GPU-Assisted validation waits on queue completion. This wait "
                     "could block the host's signaling of this event, resulting in deadlock.";
        InternalError(command_buffer, loc, error_msg.str().c_str());
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
    if (chassis_state.modified_create_info.setLayoutCount > desc_set_bind_index_) {
        std::ostringstream strm;
        strm << "pCreateInfo::setLayoutCount (" << chassis_state.modified_create_info.setLayoutCount
             << ") will conflicts with validation's descriptor set at slot " << desc_set_bind_index_ << ". "
             << "This Pipeline Layout has too many descriptor sets that will not allow GPU shader instrumentation to be setup for "
                "pipelines created with it, therefor no validation error will be repored for them by GPU-AV at "
                "runtime.";
        InternalWarning(device, record_obj.location, strm.str().c_str());
    } else {
        // Modify the pipeline layout by:
        // 1. Copying the caller's descriptor set desc_layouts
        // 2. Fill in dummy descriptor layouts up to the max binding
        // 3. Fill in with the debug descriptor layout at the max binding slot
        chassis_state.new_layouts.reserve(desc_set_bind_index_ + 1);
        chassis_state.new_layouts.insert(chassis_state.new_layouts.end(), &pCreateInfo->pSetLayouts[0],
                                         &pCreateInfo->pSetLayouts[pCreateInfo->setLayoutCount]);
        for (uint32_t i = pCreateInfo->setLayoutCount; i < desc_set_bind_index_; ++i) {
            chassis_state.new_layouts.push_back(dummy_desc_layout_);
        }
        chassis_state.new_layouts.push_back(debug_desc_layout_);
        chassis_state.modified_create_info.pSetLayouts = chassis_state.new_layouts.data();
        chassis_state.modified_create_info.setLayoutCount = desc_set_bind_index_ + 1;
    }
    BaseClass::PreCallRecordCreatePipelineLayout(device, pCreateInfo, pAllocator, pPipelineLayout, record_obj, chassis_state);
}

void GpuShaderInstrumentor::PostCallRecordCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo *pCreateInfo,
                                                               const VkAllocationCallbacks *pAllocator,
                                                               VkPipelineLayout *pPipelineLayout, const RecordObject &record_obj) {
    if (record_obj.result != VK_SUCCESS) {
        InternalError(device, record_obj.location, "Unable to create pipeline layout.  Device could become unstable.");
        return;
    }
    BaseClass::PostCallRecordCreatePipelineLayout(device, pCreateInfo, pAllocator, pPipelineLayout, record_obj);
}

void GpuShaderInstrumentor::PreCallRecordCreateShadersEXT(VkDevice device, uint32_t createInfoCount,
                                                          const VkShaderCreateInfoEXT *pCreateInfos,
                                                          const VkAllocationCallbacks *pAllocator, VkShaderEXT *pShaders,
                                                          const RecordObject &record_obj, chassis::ShaderObject &chassis_state) {
    for (uint32_t i = 0; i < createInfoCount; ++i) {
        // TODO - Should check descriptor set layout per-createInfoCount
        if (chassis_state.instrumented_create_info->setLayoutCount > desc_set_bind_index_) {
            std::ostringstream strm;
            strm << "pCreateInfos[" << i << "]::setLayoutCount (" << chassis_state.instrumented_create_info->setLayoutCount
                 << ") will conflicts with validation's descriptor set at slot " << desc_set_bind_index_ << ". "
                 << "This Shader Object has too many descriptor sets that will not allow GPU shader instrumentation to be setup "
                    "for VkShaderEXT created with it, therefor no validation error will be repored for them by GPU-AV at "
                    "runtime.";
            InternalWarning(device, record_obj.location, strm.str().c_str());
        } else {
            // Modify the pipeline layout by:
            // 1. Copying the caller's descriptor set desc_layouts
            // 2. Fill in dummy descriptor layouts up to the max binding
            // 3. Fill in with the debug descriptor layout at the max binding slot
            chassis_state.new_layouts.reserve(desc_set_bind_index_ + 1);
            chassis_state.new_layouts.insert(chassis_state.new_layouts.end(), pCreateInfos[i].pSetLayouts,
                                             &pCreateInfos[i].pSetLayouts[pCreateInfos[i].setLayoutCount]);
            for (uint32_t j = pCreateInfos[i].setLayoutCount; j < desc_set_bind_index_; ++j) {
                chassis_state.new_layouts.push_back(dummy_desc_layout_);
            }
            chassis_state.new_layouts.push_back(debug_desc_layout_);
            chassis_state.instrumented_create_info->pSetLayouts = chassis_state.new_layouts.data();
            chassis_state.instrumented_create_info->setLayoutCount = desc_set_bind_index_ + 1;
        }
    }
}

void GpuShaderInstrumentor::PostCallRecordCreateShadersEXT(VkDevice device, uint32_t createInfoCount,
                                                           const VkShaderCreateInfoEXT *pCreateInfos,
                                                           const VkAllocationCallbacks *pAllocator, VkShaderEXT *pShaders,
                                                           const RecordObject &record_obj, chassis::ShaderObject &chassis_state) {
    BaseClass::PostCallRecordCreateShadersEXT(device, createInfoCount, pCreateInfos, pAllocator, pShaders, record_obj,
                                              chassis_state);

    for (uint32_t i = 0; i < createInfoCount; ++i) {
        shader_map_.insert_or_assign(chassis_state.unique_shader_ids[i], VK_NULL_HANDLE, VK_NULL_HANDLE, pShaders[i],
                                     chassis_state.instrumented_spirv[i]);
    }
}

void GpuShaderInstrumentor::PreCallRecordDestroyShaderEXT(VkDevice device, VkShaderEXT shader,
                                                          const VkAllocationCallbacks *pAllocator, const RecordObject &record_obj) {
    auto to_erase = shader_map_.snapshot([shader](const GpuAssistedShaderTracker &entry) { return entry.shader_object == shader; });
    for (const auto &entry : to_erase) {
        shader_map_.erase(entry.first);
    }
    BaseClass::PreCallRecordDestroyShaderEXT(device, shader, pAllocator, record_obj);
}

void GpuShaderInstrumentor::PreCallRecordCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                                 const VkGraphicsPipelineCreateInfo *pCreateInfos,
                                                                 const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                                 const RecordObject &record_obj, PipelineStates &pipeline_states,
                                                                 chassis::CreateGraphicsPipelines &chassis_state) {
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
    UtilCopyCreatePipelineFeedbackData(count, pCreateInfos, chassis_state.modified_create_infos.data());
    PostCallRecordPipelineCreations(count, pCreateInfos, pAllocator, pPipelines, chassis_state.modified_create_infos.data(),
                                    chassis_state.passed_in_shader_stage_ci);
}

void GpuShaderInstrumentor::PostCallRecordCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                                 const VkComputePipelineCreateInfo *pCreateInfos,
                                                                 const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                                 const RecordObject &record_obj, PipelineStates &pipeline_states,
                                                                 chassis::CreateComputePipelines &chassis_state) {
    BaseClass::PostCallRecordCreateComputePipelines(device, pipelineCache, count, pCreateInfos, pAllocator, pPipelines, record_obj,
                                                    pipeline_states, chassis_state);
    UtilCopyCreatePipelineFeedbackData(count, pCreateInfos, chassis_state.modified_create_infos.data());
    PostCallRecordPipelineCreations(count, pCreateInfos, pAllocator, pPipelines, chassis_state.modified_create_infos.data(),
                                    chassis_state.passed_in_shader_stage_ci);
}

void GpuShaderInstrumentor::PostCallRecordCreateRayTracingPipelinesNV(
    VkDevice device, VkPipelineCache pipelineCache, uint32_t count, const VkRayTracingPipelineCreateInfoNV *pCreateInfos,
    const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines, const RecordObject &record_obj,
    PipelineStates &pipeline_states, chassis::CreateRayTracingPipelinesNV &chassis_state) {
    BaseClass::PostCallRecordCreateRayTracingPipelinesNV(device, pipelineCache, count, pCreateInfos, pAllocator, pPipelines,
                                                         record_obj, pipeline_states, chassis_state);
    UtilCopyCreatePipelineFeedbackData(count, pCreateInfos, chassis_state.modified_create_infos.data());
    PostCallRecordPipelineCreations(count, pCreateInfos, pAllocator, pPipelines, chassis_state.modified_create_infos.data(),
                                    chassis_state.passed_in_shader_stage_ci);
}

void GpuShaderInstrumentor::PostCallRecordCreateRayTracingPipelinesKHR(
    VkDevice device, VkDeferredOperationKHR deferredOperation, VkPipelineCache pipelineCache, uint32_t count,
    const VkRayTracingPipelineCreateInfoKHR *pCreateInfos, const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
    const RecordObject &record_obj, PipelineStates &pipeline_states, chassis::CreateRayTracingPipelinesKHR &chassis_state) {
    BaseClass::PostCallRecordCreateRayTracingPipelinesKHR(device, deferredOperation, pipelineCache, count, pCreateInfos, pAllocator,
                                                          pPipelines, record_obj, pipeline_states, chassis_state);
    UtilCopyCreatePipelineFeedbackData(count, pCreateInfos, chassis_state.modified_create_infos.data());
    PostCallRecordPipelineCreations(count, pCreateInfos, pAllocator, pPipelines, chassis_state.modified_create_infos.data(),
                                    chassis_state.passed_in_shader_stage_ci);
}

// Remove all the shader trackers associated with this destroyed pipeline.
void GpuShaderInstrumentor::PreCallRecordDestroyPipeline(VkDevice device, VkPipeline pipeline,
                                                         const VkAllocationCallbacks *pAllocator, const RecordObject &record_obj) {
    auto to_erase = shader_map_.snapshot([pipeline](const GpuAssistedShaderTracker &entry) { return entry.pipeline == pipeline; });
    for (const auto &entry : to_erase) {
        shader_map_.erase(entry.first);
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
    chassis_state.passed_in_shader_stage_ci = false;
    // Walk through all the pipelines, make a copy of each and flag each pipeline that contains a shader that uses the debug
    // descriptor set index.
    for (uint32_t pipeline = 0; pipeline < count; ++pipeline) {
        const auto &pipe = pipeline_states[pipeline];
        // NOTE: since these are "safe" CreateInfos, this will create a deep copy via the safe copy constructor
        auto new_pipeline_ci = std::get<SafeCreateInfo>(pipe->create_info);

        bool replace_shaders = false;
        if (pipe->active_slots.find(desc_set_bind_index_) != pipe->active_slots.end()) {
            replace_shaders = true;
        }
        // If the app requests all available sets, the pipeline layout was not modified at pipeline layout creation and the
        // already instrumented shaders need to be replaced with uninstrumented shaders
        const auto pipeline_layout = pipe->PipelineLayoutState();
        if (pipeline_layout && pipeline_layout->set_layouts.size() > desc_set_bind_index_) {
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
                    InternalError(device, record_obj.location, "Unable to replace instrumented shader with non-instrumented one.");
                }
            }
        } else {
            // !replace_shaders implies that the instrumented shaders should be used. However, if this is a non-executable pipeline
            // library created with pre-raster or fragment shader state, it contains shaders that have not yet been instrumented
            //
            // This also could be because safe_VkShaderModuleCreateInfo is passed in the pNext of VkPipelineShaderStageCreateInfo
            for (const auto &stage_state : pipe->stage_states) {
                auto module_state = std::const_pointer_cast<vvl::ShaderModule>(stage_state.module_state);
                ASSERT_AND_CONTINUE(module_state);
                if (module_state->Handle()) continue;

                const VkShaderStageFlagBits stage = stage_state.GetStage();
                // Now find the corresponding VkShaderModuleCreateInfo
                auto &stage_ci =
                    GetShaderStageCI<SafeCreateInfo, vku::safe_VkPipelineShaderStageCreateInfo>(new_pipeline_ci, stage);
                // We're modifying the copied, safe create info, which is ok to be non-const
                auto sm_ci =
                    const_cast<vku::safe_VkShaderModuleCreateInfo *>(reinterpret_cast<const vku::safe_VkShaderModuleCreateInfo *>(
                        vku::FindStructInPNextChain<VkShaderModuleCreateInfo>(stage_ci.pNext)));

                if (sm_ci) {
                    // Skip if we are in selective mode
                    if (gpuav_settings.select_instrumented_shaders && !CheckForGpuAvEnabled(sm_ci->pNext)) continue;
                } else {
                    // We only get here if using GPL, but if here with a linking pipeline, everything is already handled
                    if (pipe->HasFullState() || (!pipe->pre_raster_state && !pipe->fragment_shader_state)) break;
                }

                // If the shader module's handle is non-null, then it was defined with CreateShaderModule and covered by the
                // case above. Otherwise, it is being defined during CGPL time
                if (chassis_state.shader_unique_id_maps.size() <= pipeline) {
                    chassis_state.shader_unique_id_maps.resize(pipeline + 1);
                }

                uint32_t unique_shader_id = 0;
                bool cached = false;
                bool pass = false;
                std::vector<uint32_t> instrumented_spirv;
                if (gpuav_settings.cache_instrumented_shaders) {
                    unique_shader_id =
                        hash_util::ShaderHash(module_state->spirv->words_.data(), module_state->spirv->words_.size());
                    if (const auto spirv = instrumented_shaders_cache_.Get(unique_shader_id)) {
                        instrumented_spirv = *spirv;
                        cached = true;
                    }
                } else {
                    unique_shader_id = unique_shader_module_id_++;
                }
                if (!cached) {
                    pass = InstrumentShader(module_state->spirv->words_, unique_shader_id, record_obj.location, instrumented_spirv);
                }
                if (cached || pass) {
                    module_state->gpu_validation_shader_id = unique_shader_id;
                    if (sm_ci) {
                        chassis_state.passed_in_shader_stage_ci = true;
                        sm_ci->SetCode(instrumented_spirv);
                    } else {
                        // TODO - GPL will hit here for executable pipelines, but we should be able to update the shader module
                        // handle since it can be found
                    }
                    if (gpuav_settings.cache_instrumented_shaders && !cached) {
                        instrumented_shaders_cache_.Add(unique_shader_id, instrumented_spirv);
                    }
                }

                chassis_state.shader_unique_id_maps[pipeline][stage] = unique_shader_id;
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
                                                            const SafeCreateInfo &modified_create_infos,
                                                            bool passed_in_shader_stage_ci) {
    for (uint32_t pipeline = 0; pipeline < count; ++pipeline) {
        auto pipeline_state = Get<vvl::Pipeline>(pPipelines[pipeline]);
        if (!pipeline_state) continue;

        if (!pipeline_state->stage_states.empty() && !(pipeline_state->create_flags & VK_PIPELINE_CREATE_LIBRARY_BIT_KHR)) {
            const auto pipeline_layout = pipeline_state->PipelineLayoutState();
            for (auto &stage_state : pipeline_state->stage_states) {
                auto &module_state = stage_state.module_state;

                if (pipeline_state->active_slots.find(desc_set_bind_index_) != pipeline_state->active_slots.end() ||
                    (pipeline_layout->set_layouts.size() > desc_set_bind_index_)) {
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

                VkShaderModule shader_module_handle = module_state->VkHandle();
                if (shader_module_handle == VK_NULL_HANDLE && passed_in_shader_stage_ci) {
                    shader_module_handle = kPipelineStageInfoHandle;
                }
                shader_map_.insert_or_assign(module_state->gpu_validation_shader_id, pipeline_state->VkHandle(),
                                             shader_module_handle, VK_NULL_HANDLE, std::move(code));
            }
        }
    }
}

void GpuShaderInstrumentor::InternalError(LogObjectList objlist, const Location &loc, const char *const specific_message,
                                          bool vma_fail) const {
    aborted_ = true;
    std::string error_message = specific_message;
    if (vma_fail) {
        char *stats_string;
        vmaBuildStatsString(vma_allocator_, &stats_string, false);
        error_message += " VMA statistics = ";
        error_message += stats_string;
        vmaFreeStatsString(vma_allocator_, stats_string);
    }

    char const *layer_name = container_type == LayerObjectTypeDebugPrintf ? "Debug PrintF" : "GPU-AV";
    char const *vuid =
        container_type == LayerObjectTypeDebugPrintf ? "UNASSIGNED-DEBUG-PRINTF" : "UNASSIGNED-GPU-Assisted-Validation";

    LogError(vuid, objlist, loc, "Internal Error, %s is being disabled. Details:\n%s", layer_name, error_message.c_str());

    // Once we encounter an internal issue disconnect everything.
    // This prevents need to check "if (aborted)" (which is awful when we easily forget to check somewhere and the user gets spammed
    // with errors making it hard to see the first error with the real source of the problem).
    ReleaseDeviceDispatchObject(this->container_type);
}

void GpuShaderInstrumentor::InternalWarning(LogObjectList objlist, const Location &loc, const char *const specific_message) const {
    char const *vuid = container_type == LayerObjectTypeDebugPrintf ? "WARNING-DEBUG-PRINTF" : "WARNING-GPU-Assisted-Validation";
    LogWarning(vuid, objlist, loc, "Internal Warning: %s", specific_message);
}

// The lock (debug_output_mutex) is held by the caller,
// because the latter has code paths that make multiple calls of this function,
// and all such calls have to access the same debug reporting state to ensure consistency of output information.
static std::string LookupDebugUtilsNameNoLock(const DebugReport *debug_report, const uint64_t object) {
    auto object_label = debug_report->GetUtilsObjectNameNoLock(object);
    if (object_label != "") {
        object_label = "(" + object_label + ")";
    }
    return object_label;
}

// Read the contents of the SPIR-V OpSource instruction and any following continuation instructions.
// Split the single string into a vector of strings, one for each line, for easier processing.
static void ReadOpSource(const std::vector<spirv::Instruction> &instructions, const uint32_t reported_file_id,
                         std::vector<std::string> &opsource_lines) {
    for (size_t i = 0; i < instructions.size(); i++) {
        const spirv::Instruction &insn = instructions[i];
        if ((insn.Opcode() == spv::OpSource) && (insn.Length() >= 5) && (insn.Word(3) == reported_file_id)) {
            std::istringstream in_stream;
            std::string cur_line;
            in_stream.str(insn.GetAsString(4));
            while (std::getline(in_stream, cur_line)) {
                opsource_lines.push_back(cur_line);
            }

            for (size_t k = i + 1; k < instructions.size(); k++) {
                const spirv::Instruction &continue_insn = instructions[k];
                if (continue_insn.Opcode() != spv::OpSourceContinued) {
                    break;
                }
                in_stream.str(continue_insn.GetAsString(1));
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
static bool GetLineAndFilename(const std::string &string, uint32_t *linenumber, std::string &filename) {
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

    const bool found_line = std::regex_match(string, captures, line_regex);
    if (!found_line) return false;

    // filename is optional and considered found only if the whitespace and the filename are captured
    if (captures[2].matched && captures[3].matched) {
        // Remove enclosing double quotes.  The regex guarantees the quotes and at least one char.
        filename = captures[3].str().substr(1, captures[3].str().size() - 2);
    }
    *linenumber = (uint32_t)std::stoul(captures[1]);
    return true;
}

// Where we build up the error message with all the useful debug information about where the error occured
std::string GpuShaderInstrumentor::GenerateDebugInfoMessage(
    VkCommandBuffer commandBuffer, const std::vector<spirv::Instruction> &instructions, uint32_t instruction_position,
    const gpu::GpuAssistedShaderTracker *tracker_info, VkPipelineBindPoint pipeline_bind_point, uint32_t operation_index) const {
    std::ostringstream ss;
    if (instructions.empty() || !tracker_info) {
        ss << "[Internal Error] - Can't get instructions from shader_map\n";
        return ss.str();
    }

    ss << std::hex << std::showbase;
    if (tracker_info->shader_module == VK_NULL_HANDLE && tracker_info->shader_object == VK_NULL_HANDLE) {
        std::unique_lock<std::mutex> lock(debug_report->debug_output_mutex);
        ss << "[Internal Error] - Unable to locate shader/pipeline handles used in command buffer "
           << LookupDebugUtilsNameNoLock(debug_report, HandleToUint64(commandBuffer)) << "(" << HandleToUint64(commandBuffer)
           << ")\n";
        assert(true);
    } else {
        std::unique_lock<std::mutex> lock(debug_report->debug_output_mutex);
        ss << "Command buffer " << LookupDebugUtilsNameNoLock(debug_report, HandleToUint64(commandBuffer)) << "("
           << HandleToUint64(commandBuffer) << ")\n";

        ss << std::dec << std::noshowbase;
        ss << "\t";  // helps to show that the index is expressed with respect to the command buffer
        if (pipeline_bind_point == VK_PIPELINE_BIND_POINT_GRAPHICS) {
            ss << "Draw ";
        } else if (pipeline_bind_point == VK_PIPELINE_BIND_POINT_COMPUTE) {
            ss << "Compute Dispatch ";
        } else if (pipeline_bind_point == VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR) {
            ss << "Ray Trace ";
        } else {
            assert(false);
            ss << "Unknown Pipeline Operation ";
        }
        ss << "Index " << operation_index << "\n";
        ss << std::hex << std::noshowbase;

        if (tracker_info->shader_module == VK_NULL_HANDLE) {
            ss << "Shader Object " << LookupDebugUtilsNameNoLock(debug_report, HandleToUint64(tracker_info->shader_object)) << "("
               << HandleToUint64(tracker_info->shader_object) << ")\n";
        } else {
            ss << "Pipeline " << LookupDebugUtilsNameNoLock(debug_report, HandleToUint64(tracker_info->pipeline)) << "("
               << HandleToUint64(tracker_info->pipeline) << ")\n";
            if (tracker_info->shader_module == gpu::kPipelineStageInfoHandle) {
                ss << "Shader Module was passed in via VkPipelineShaderStageCreateInfo::pNext\n";
            } else {
                ss << "Shader Module " << LookupDebugUtilsNameNoLock(debug_report, HandleToUint64(tracker_info->shader_module))
                   << "(" << HandleToUint64(tracker_info->shader_module) << ")\n";
            }
        }
    }

    ss << std::dec << std::noshowbase;
    ss << "SPIR-V Instruction Index = " << instruction_position << "\n";

    // Find the OpLine just before the failing instruction indicated by the debug info.
    // SPIR-V can only be iterated in the forward direction due to its opcode/length encoding.
    uint32_t index = 0;
    uint32_t reported_file_id = 0;
    uint32_t reported_line_number = 0;
    uint32_t reported_column_number = 0;
    for (const spirv::Instruction &insn : instructions) {
        if (insn.Opcode() == spv::OpLine) {
            reported_file_id = insn.Word(1);
            reported_line_number = insn.Word(2);
            reported_column_number = insn.Word(3);
        }
        if (index == instruction_position) {
            break;
        }
        index++;
    }

    if (reported_file_id == 0) {
        ss << "Unable to find SPIR-V OpLine for source information.  Build shader with debug info to get source information.\n";
        return ss.str();
    }

    std::string prefix;
    if (container_type == LayerObjectTypeDebugPrintf) {
        prefix = "Debug shader printf message generated ";
    } else {
        prefix = "Shader validation error occurred ";
    }

    // Create message with file information obtained from the OpString pointed to by the discovered OpLine.
    bool found_opstring = false;
    std::string reported_filename;
    for (const spirv::Instruction &insn : instructions) {
        if (insn.Opcode() == spv::OpString && insn.Length() >= 3 && insn.Word(1) == reported_file_id) {
            found_opstring = true;
            reported_filename = insn.GetAsString(2);
            if (reported_filename.empty()) {
                ss << prefix << "at line " << reported_line_number;
            } else {
                ss << prefix << "in file " << reported_filename << " at line " << reported_line_number;
            }
            if (reported_column_number > 0) {
                ss << ", column " << reported_column_number;
            }
            ss << "\n";
            break;
        }
        // OpString can only be in the debug section, so can break early if not found
        if (insn.Opcode() == spv::OpFunction) break;
    }

    if (!found_opstring) {
        ss << "Unable to find SPIR-V OpString from OpLine instruction.\n";
        ss << "File ID = " << reported_file_id << ", Line Number = " << reported_line_number
           << ", Column = " << reported_column_number << "\n";
    }

    // Create message to display source code line containing error.
    // Read the source code and split it up into separate lines.
    std::vector<std::string> opsource_lines;
    ReadOpSource(instructions, reported_file_id, opsource_lines);
    // Find the line in the OpSource content that corresponds to the reported error file and line.
    if (!opsource_lines.empty()) {
        uint32_t saved_line_number = 0;
        std::string current_filename = reported_filename;  // current "preprocessor" filename state.
        std::vector<std::string>::size_type saved_opsource_offset = 0;

        // This was designed to fine the best line if using #line in GLSL
        bool found_best_line = false;
        for (auto it = opsource_lines.begin(); it != opsource_lines.end(); ++it) {
            uint32_t parsed_line_number;
            std::string parsed_filename;
            const bool found_line = GetLineAndFilename(*it, &parsed_line_number, parsed_filename);
            if (!found_line) continue;

            const bool found_filename = parsed_filename.size() > 0;
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
            const size_t opsource_index = (reported_line_number - saved_line_number) + 1 + saved_opsource_offset;
            if (opsource_index < opsource_lines.size()) {
                ss << "\n" << reported_line_number << ": " << opsource_lines[opsource_index] << "\n";
            } else {
                ss << "Internal error: calculated source line of " << opsource_index << " for source size of "
                   << opsource_lines.size() << " lines\n";
            }
        } else if (reported_line_number < opsource_lines.size() && reported_line_number != 0) {
            // file lines normally start at 1 index
            ss << "\n" << opsource_lines[reported_line_number - 1] << "\n";
            if (reported_column_number > 0) {
                std::string spaces(reported_column_number - 1, ' ');
                ss << spaces << "^";
            }
        } else {
            ss << "Unable to find neither a suitable line in SPIR-V OpSource\n";
        }
    } else {
        ss << "Unable to find SPIR-V OpSource\n";
    }
    return ss.str();
}

}  // namespace gpu
