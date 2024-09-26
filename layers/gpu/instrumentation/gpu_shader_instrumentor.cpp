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
#include "gpu/spirv/module.h"
#include "chassis/chassis_modification_state.h"
#include "gpu/shaders/gpu_error_codes.h"
#include "spirv-tools/optimizer.hpp"
#include "utils/vk_layer_utils.h"
#include "state_tracker/descriptor_sets.h"
#include "state_tracker/shader_object_state.h"

#include <cassert>
#include <regex>
#include <fstream>

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

ReadLockGuard GpuShaderInstrumentor::ReadLock() const {
    if (global_settings.fine_grained_locking) {
        return ReadLockGuard(validation_object_mutex, std::defer_lock);
    } else {
        return ReadLockGuard(validation_object_mutex);
    }
}

WriteLockGuard GpuShaderInstrumentor::WriteLock() {
    if (global_settings.fine_grained_locking) {
        return WriteLockGuard(validation_object_mutex, std::defer_lock);
    } else {
        return WriteLockGuard(validation_object_mutex);
    }
}

std::shared_ptr<vvl::Queue> GpuShaderInstrumentor::CreateQueue(VkQueue handle, uint32_t family_index, uint32_t queue_index,
                                                               VkDeviceQueueCreateFlags flags,
                                                               const VkQueueFamilyProperties &queueFamilyProperties) {
    return std::static_pointer_cast<vvl::Queue>(std::make_shared<gpu_tracker::Queue>(*this, handle, family_index, queue_index,
                                                                                     flags, queueFamilyProperties, timeline_khr_));
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
        if (supported_features.shaderInt64 && !enabled_features->shaderInt64) {
            InternalWarning(device, record_obj.location, "Forcing VkPhysicalDeviceFeatures::shaderInt64 to VK_TRUE");
            enabled_features->shaderInt64 = VK_TRUE;
        }
    }

    auto add_missing_features = [this, &record_obj, modified_create_info]() {
        // Add timeline semaphore feature - This is required as we use it to manage when command buffers are submitted at queue
        // submit time
        if (auto *ts_features = const_cast<VkPhysicalDeviceTimelineSemaphoreFeatures *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceTimelineSemaphoreFeatures>(modified_create_info))) {
            if (ts_features->timelineSemaphore == VK_FALSE) {
                InternalWarning(device, record_obj.location,
                                "Forcing VkPhysicalDeviceTimelineSemaphoreFeatures::timelineSemaphore to VK_TRUE");
                ts_features->timelineSemaphore = VK_TRUE;
            }
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
            if (features12->timelineSemaphore == VK_FALSE) {
                InternalWarning(device, record_obj.location,
                                "Forcing VkPhysicalDeviceVulkan12Features::timelineSemaphore to VK_TRUE");
                features12->timelineSemaphore = VK_TRUE;
            }
        } else {
            add_missing_features();
        }
    } else if (api_version == VK_API_VERSION_1_1) {
        // Add our new extensions (will only add if found)
        const std::string_view ts_ext{"VK_KHR_timeline_semaphore"};
        vku::AddExtension(*modified_create_info, ts_ext.data());
        add_missing_features();
        timeline_khr_ = true;
    }
}

// In charge of getting things for shader instrumentation that both GPU-AV and DebugPrintF will need
void GpuShaderInstrumentor::PostCreateDevice(const VkDeviceCreateInfo *pCreateInfo, const Location &loc) {
    if (api_version < VK_API_VERSION_1_1) {
        InternalError(device, loc, "GPU Shader Instrumentation requires Vulkan 1.1 or later.");
        return;
    }

    // If using 1.0 and doesn't supports VK_KHR_timeline_semaphore, this will error inside, simple fix is to do after the 1.1 check
    BaseClass::PostCreateDevice(pCreateInfo, loc);

    VkPhysicalDeviceFeatures supported_features{};
    DispatchGetPhysicalDeviceFeatures(physical_device, &supported_features);
    if (!supported_features.fragmentStoresAndAtomics) {
        InternalError(
            device, loc,
            "GPU Shader Instrumentation requires fragmentStoresAndAtomics to allow writting out data inside the fragment shader.");
        return;
    }
    if (!supported_features.vertexPipelineStoresAndAtomics) {
        InternalError(device, loc,
                      "GPU Shader Instrumentation requires vertexPipelineStoresAndAtomics to allow writting out data inside the "
                      "vertex shader.");
        return;
    }

    // If api version 1.1 or later, SetDeviceLoaderData will be in the loader
    auto chain_info = GetChainInfo(pCreateInfo, VK_LOADER_DATA_CALLBACK);
    assert(chain_info->u.pfnSetDeviceLoaderData);
    vk_set_device_loader_data_ = chain_info->u.pfnSetDeviceLoaderData;

    // maxBoundDescriptorSets limit, but possibly adjusted
    const uint32_t adjusted_max_desc_sets_limit =
        std::min(gpu::kMaxAdjustedBoundDescriptorSet, phys_dev_props.limits.maxBoundDescriptorSets);
    // If gpu_validation_reserve_binding_slot: the max slot is where we reserved
    // else: always use the last possible set as least likely to be used
    instrumentation_desc_set_bind_index_ = adjusted_max_desc_sets_limit - 1;

    // We can't do anything if there is only one.
    // Device probably not a legit Vulkan device, since there should be at least 4. Protect ourselves.
    if (adjusted_max_desc_sets_limit == 1) {
        InternalError(device, loc, "Device can bind only a single descriptor set.");
        return;
    }

    VkResult result = UtilInitializeVma(instance, physical_device, device, enabled_features.bufferDeviceAddress, &vma_allocator_);
    if (result != VK_SUCCESS) {
        InternalError(device, loc, "Could not initialize VMA", true);
        return;
    }

    desc_set_manager_ =
        std::make_unique<gpu::DescriptorSetManager>(device, static_cast<uint32_t>(instrumentation_bindings_.size()));

    const VkDescriptorSetLayoutCreateInfo debug_desc_layout_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO, nullptr, 0,
                                                                    static_cast<uint32_t>(instrumentation_bindings_.size()),
                                                                    instrumentation_bindings_.data()};

    result = DispatchCreateDescriptorSetLayout(device, &debug_desc_layout_info, nullptr, &instrumentation_desc_layout_);
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
    for (uint32_t j = 0; j < instrumentation_desc_set_bind_index_; ++j) {
        debug_layouts.push_back(dummy_desc_layout_);
    }
    debug_layouts.push_back(instrumentation_desc_layout_);

    const VkPipelineLayoutCreateInfo debug_pipeline_layout_info = {VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
                                                                   nullptr,
                                                                   0u,
                                                                   static_cast<uint32_t>(debug_layouts.size()),
                                                                   debug_layouts.data(),
                                                                   0u,
                                                                   nullptr};
    result = DispatchCreatePipelineLayout(device, &debug_pipeline_layout_info, nullptr, &instrumentation_pipeline_layout_);
    if (result != VK_SUCCESS) {
        InternalError(device, loc, "vkCreateDescriptorSetLayout failed for internal pipeline layout");
        Cleanup();
        return;
    }
}

void GpuShaderInstrumentor::Cleanup() {
    if (instrumentation_desc_layout_) {
        DispatchDestroyDescriptorSetLayout(device, instrumentation_desc_layout_, nullptr);
        instrumentation_desc_layout_ = VK_NULL_HANDLE;
    }
    if (dummy_desc_layout_) {
        DispatchDestroyDescriptorSetLayout(device, dummy_desc_layout_, nullptr);
        dummy_desc_layout_ = VK_NULL_HANDLE;
    }

    if (instrumentation_pipeline_layout_) {
        DispatchDestroyPipelineLayout(device, instrumentation_pipeline_layout_, nullptr);
        instrumentation_pipeline_layout_ = VK_NULL_HANDLE;
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
    if (gpuav_settings.shader_instrumentation_enabled) {
        if (chassis_state.modified_create_info.setLayoutCount > instrumentation_desc_set_bind_index_) {
            std::ostringstream strm;
            strm << "pCreateInfo::setLayoutCount (" << chassis_state.modified_create_info.setLayoutCount
                 << ") will conflicts with validation's descriptor set at slot " << instrumentation_desc_set_bind_index_ << ". "
                 << "This Pipeline Layout has too many descriptor sets that will not allow GPU shader instrumentation to be setup "
                    "for "
                    "pipelines created with it, therefor no validation error will be repored for them by GPU-AV at "
                    "runtime.";
            InternalWarning(device, record_obj.location, strm.str().c_str());
        } else {
            // Modify the pipeline layout by:
            // 1. Copying the caller's descriptor set desc_layouts
            // 2. Fill in dummy descriptor layouts up to the max binding
            // 3. Fill in with the debug descriptor layout at the max binding slot
            chassis_state.new_layouts.reserve(instrumentation_desc_set_bind_index_ + 1);
            chassis_state.new_layouts.insert(chassis_state.new_layouts.end(), &pCreateInfo->pSetLayouts[0],
                                             &pCreateInfo->pSetLayouts[pCreateInfo->setLayoutCount]);
            for (uint32_t i = pCreateInfo->setLayoutCount; i < instrumentation_desc_set_bind_index_; ++i) {
                chassis_state.new_layouts.push_back(dummy_desc_layout_);
            }
            chassis_state.new_layouts.push_back(instrumentation_desc_layout_);
            chassis_state.modified_create_info.pSetLayouts = chassis_state.new_layouts.data();
            chassis_state.modified_create_info.setLayoutCount = instrumentation_desc_set_bind_index_ + 1;
        }
    }
    BaseClass::PreCallRecordCreatePipelineLayout(device, pCreateInfo, pAllocator, pPipelineLayout, record_obj, chassis_state);
}

void GpuShaderInstrumentor::PostCallRecordCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo *pCreateInfo,
                                                               const VkAllocationCallbacks *pAllocator,
                                                               VkPipelineLayout *pPipelineLayout, const RecordObject &record_obj) {
    if (record_obj.result != VK_SUCCESS) {
        InternalError(device, record_obj.location, "Unable to create pipeline layout.");
        return;
    }
    BaseClass::PostCallRecordCreatePipelineLayout(device, pCreateInfo, pAllocator, pPipelineLayout, record_obj);
}

void GpuShaderInstrumentor::PostCallRecordCreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo *pCreateInfo,
                                                             const VkAllocationCallbacks *pAllocator, VkShaderModule *pShaderModule,
                                                             const RecordObject &record_obj,
                                                             chassis::CreateShaderModule &chassis_state) {
    BaseClass::PostCallRecordCreateShaderModule(device, pCreateInfo, pAllocator, pShaderModule, record_obj, chassis_state);

    // By default, we instrument everything, but if the setting is enabled, we only will instrument the shaders the app picks
    if (gpuav_settings.select_instrumented_shaders && IsSelectiveInstrumentationEnabled(pCreateInfo->pNext)) {
        // If this is being filled up, likely only a few shaders and the app scope is narrowed down, so no need to spend time
        // removing these later
        selected_instrumented_shaders.insert(*pShaderModule);
    };
}

void GpuShaderInstrumentor::PreCallRecordShaderObjectInstrumentation(
    VkShaderCreateInfoEXT &create_info, const Location &create_info_loc,
    chassis::ShaderObjectInstrumentationData &instrumentation_data) {
    if (gpuav_settings.select_instrumented_shaders && !IsSelectiveInstrumentationEnabled(create_info.pNext)) return;
    uint32_t unique_shader_id = 0;
    bool cached = false;
    bool pass = false;
    std::vector<uint32_t> &instrumented_spirv = instrumentation_data.instrumented_spirv;
    if (gpuav_settings.cache_instrumented_shaders) {
        unique_shader_id = hash_util::ShaderHash(create_info.pCode, create_info.codeSize);
        if (const auto spirv = instrumented_shaders_cache_.Get(unique_shader_id)) {
            instrumented_spirv = *spirv;
            cached = true;
        }
    } else {
        unique_shader_id = unique_shader_module_id_++;
    }

    const bool has_bindless_descriptors = HasBindlessDescriptors(create_info);

    if (!cached) {
        pass = InstrumentShader(
            vvl::make_span(static_cast<const uint32_t *>(create_info.pCode), create_info.codeSize / sizeof(uint32_t)),
            unique_shader_id, has_bindless_descriptors, create_info_loc, instrumented_spirv);
    }

    if (cached || pass) {
        instrumentation_data.unique_shader_id = unique_shader_id;
        create_info.pCode = instrumented_spirv.data();
        create_info.codeSize = instrumented_spirv.size() * sizeof(uint32_t);
        if (gpuav_settings.cache_instrumented_shaders && !cached) {
            instrumented_shaders_cache_.Add(unique_shader_id, instrumented_spirv);
        }
    }
}

void GpuShaderInstrumentor::PreCallRecordCreateShadersEXT(VkDevice device, uint32_t createInfoCount,
                                                          const VkShaderCreateInfoEXT *pCreateInfos,
                                                          const VkAllocationCallbacks *pAllocator, VkShaderEXT *pShaders,
                                                          const RecordObject &record_obj, chassis::ShaderObject &chassis_state) {
    BaseClass::PreCallRecordCreateShadersEXT(device, createInfoCount, pCreateInfos, pAllocator, pShaders, record_obj,
                                             chassis_state);
    if (!gpuav_settings.shader_instrumentation_enabled) return;

    chassis_state.modified_create_infos.reserve(createInfoCount);

    // Resize here so if using just CoreCheck we don't waste time allocating this
    chassis_state.instrumentations_data.resize(createInfoCount);

    for (uint32_t i = 0; i < createInfoCount; ++i) {
        VkShaderCreateInfoEXT new_create_info = pCreateInfos[i];
        auto &instrumentation_data = chassis_state.instrumentations_data[i];

        if (new_create_info.setLayoutCount > instrumentation_desc_set_bind_index_) {
            std::ostringstream strm;
            strm << "pCreateInfos[" << i << "]::setLayoutCount (" << new_create_info.setLayoutCount
                 << ") will conflicts with validation's descriptor set at slot " << instrumentation_desc_set_bind_index_ << ". "
                 << "This Shader Object has too many descriptor sets that will not allow GPU shader instrumentation to be setup "
                    "for VkShaderEXT created with it, therefor no validation error will be repored for them by GPU-AV at "
                    "runtime.";
            InternalWarning(device, record_obj.location, strm.str().c_str());
        } else {
            // Modify the pipeline layout by:
            // 1. Copying the caller's descriptor set desc_layouts
            // 2. Fill in dummy descriptor layouts up to the max binding
            // 3. Fill in with the debug descriptor layout at the max binding slot
            instrumentation_data.new_layouts.reserve(instrumentation_desc_set_bind_index_ + 1);
            instrumentation_data.new_layouts.insert(instrumentation_data.new_layouts.end(), pCreateInfos[i].pSetLayouts,
                                                    &pCreateInfos[i].pSetLayouts[pCreateInfos[i].setLayoutCount]);
            for (uint32_t j = pCreateInfos[i].setLayoutCount; j < instrumentation_desc_set_bind_index_; ++j) {
                instrumentation_data.new_layouts.push_back(dummy_desc_layout_);
            }
            instrumentation_data.new_layouts.push_back(instrumentation_desc_layout_);
            new_create_info.pSetLayouts = instrumentation_data.new_layouts.data();
            new_create_info.setLayoutCount = instrumentation_desc_set_bind_index_ + 1;
        }

        PreCallRecordShaderObjectInstrumentation(new_create_info, record_obj.location.dot(vvl::Field::pCreateInfos, i),
                                                 instrumentation_data);

        chassis_state.modified_create_infos.emplace_back(std::move(new_create_info));
    }

    chassis_state.pCreateInfos = reinterpret_cast<VkShaderCreateInfoEXT *>(chassis_state.modified_create_infos.data());
}

void GpuShaderInstrumentor::PostCallRecordCreateShadersEXT(VkDevice device, uint32_t createInfoCount,
                                                           const VkShaderCreateInfoEXT *pCreateInfos,
                                                           const VkAllocationCallbacks *pAllocator, VkShaderEXT *pShaders,
                                                           const RecordObject &record_obj, chassis::ShaderObject &chassis_state) {
    BaseClass::PostCallRecordCreateShadersEXT(device, createInfoCount, pCreateInfos, pAllocator, pShaders, record_obj,
                                              chassis_state);
    if (!gpuav_settings.shader_instrumentation_enabled) return;

    for (uint32_t i = 0; i < createInfoCount; ++i) {
        auto &instrumentation_data = chassis_state.instrumentations_data[i];

        // if the shader for some reason was not instrumented, there is nothing to save
        if (!instrumentation_data.IsInstrumented()) {
            continue;
        }
        if (const auto &shader_object_state = Get<vvl::ShaderObject>(pShaders[i])) {
            shader_object_state->instrumentation_data.was_instrumented = true;
        }

        shader_map_.insert_or_assign(instrumentation_data.unique_shader_id, VK_NULL_HANDLE, VK_NULL_HANDLE, pShaders[i],
                                     instrumentation_data.instrumented_spirv);
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
    BaseClass::PreCallRecordCreateGraphicsPipelines(device, pipelineCache, count, pCreateInfos, pAllocator, pPipelines, record_obj,
                                                    pipeline_states, chassis_state);
    if (!gpuav_settings.shader_instrumentation_enabled) return;

    chassis_state.shader_instrumentations_metadata.resize(count);
    chassis_state.modified_create_infos.resize(count);

    for (uint32_t i = 0; i < count; ++i) {
        const auto &pipeline_state = pipeline_states[i];

        // Need to make a deep copy so if SPIR-V is inlined, user doesn't see it after the call
        auto &new_pipeline_ci = chassis_state.modified_create_infos[i];
        new_pipeline_ci.initialize(&pipeline_state->GraphicsCreateInfo());

        if (!NeedPipelineCreationShaderInstrumentation(*pipeline_state)) {
            continue;
        }

        const Location create_info_loc = record_obj.location.dot(vvl::Field::pCreateInfos, i);
        auto &shader_instrumentation_metadata = chassis_state.shader_instrumentations_metadata[i];

        if (pipeline_state->linking_shaders != 0) {
            PreCallRecordPipelineCreationShaderInstrumentationGPL(pAllocator, *pipeline_state, new_pipeline_ci, create_info_loc,
                                                                  shader_instrumentation_metadata);
        } else {
            PreCallRecordPipelineCreationShaderInstrumentation(pAllocator, *pipeline_state, new_pipeline_ci, create_info_loc,
                                                               shader_instrumentation_metadata);
        }
    }

    chassis_state.pCreateInfos = reinterpret_cast<VkGraphicsPipelineCreateInfo *>(chassis_state.modified_create_infos.data());
}

void GpuShaderInstrumentor::PreCallRecordCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                                const VkComputePipelineCreateInfo *pCreateInfos,
                                                                const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                                const RecordObject &record_obj, PipelineStates &pipeline_states,
                                                                chassis::CreateComputePipelines &chassis_state) {
    BaseClass::PreCallRecordCreateComputePipelines(device, pipelineCache, count, pCreateInfos, pAllocator, pPipelines, record_obj,
                                                   pipeline_states, chassis_state);
    if (!gpuav_settings.shader_instrumentation_enabled) return;

    chassis_state.shader_instrumentations_metadata.resize(count);
    chassis_state.modified_create_infos.resize(count);

    for (uint32_t i = 0; i < count; ++i) {
        const auto &pipeline_state = pipeline_states[i];

        // Need to make a deep copy so if SPIR-V is inlined, user doesn't see it after the call
        auto &new_pipeline_ci = chassis_state.modified_create_infos[i];
        new_pipeline_ci.initialize(&pipeline_state->ComputeCreateInfo());

        if (!NeedPipelineCreationShaderInstrumentation(*pipeline_state)) {
            continue;
        }

        const Location create_info_loc = record_obj.location.dot(vvl::Field::pCreateInfos, i);
        auto &shader_instrumentation_metadata = chassis_state.shader_instrumentations_metadata[i];

        PreCallRecordPipelineCreationShaderInstrumentation(pAllocator, *pipeline_state, new_pipeline_ci, create_info_loc,
                                                           shader_instrumentation_metadata);
    }

    chassis_state.pCreateInfos = reinterpret_cast<VkComputePipelineCreateInfo *>(chassis_state.modified_create_infos.data());
}

void GpuShaderInstrumentor::PreCallRecordCreateRayTracingPipelinesNV(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                                     const VkRayTracingPipelineCreateInfoNV *pCreateInfos,
                                                                     const VkAllocationCallbacks *pAllocator,
                                                                     VkPipeline *pPipelines, const RecordObject &record_obj,
                                                                     PipelineStates &pipeline_states,
                                                                     chassis::CreateRayTracingPipelinesNV &chassis_state) {
    BaseClass::PreCallRecordCreateRayTracingPipelinesNV(device, pipelineCache, count, pCreateInfos, pAllocator, pPipelines,
                                                        record_obj, pipeline_states, chassis_state);
    if (!gpuav_settings.shader_instrumentation_enabled) return;

    chassis_state.shader_instrumentations_metadata.resize(count);
    chassis_state.modified_create_infos.resize(count);

    for (uint32_t i = 0; i < count; ++i) {
        const auto &pipeline_state = pipeline_states[i];

        // Need to make a deep copy so if SPIR-V is inlined, user doesn't see it after the call
        auto &new_pipeline_ci = chassis_state.modified_create_infos[i];
        new_pipeline_ci = pipeline_state->RayTracingCreateInfo();  // use copy operation to fight the Common vs NV

        if (!NeedPipelineCreationShaderInstrumentation(*pipeline_state)) {
            continue;
        }

        const Location create_info_loc = record_obj.location.dot(vvl::Field::pCreateInfos, i);
        auto &shader_instrumentation_metadata = chassis_state.shader_instrumentations_metadata[i];

        PreCallRecordPipelineCreationShaderInstrumentation(pAllocator, *pipeline_state, new_pipeline_ci, create_info_loc,
                                                           shader_instrumentation_metadata);
    }

    chassis_state.pCreateInfos = reinterpret_cast<VkRayTracingPipelineCreateInfoNV *>(chassis_state.modified_create_infos.data());
}

void GpuShaderInstrumentor::PreCallRecordCreateRayTracingPipelinesKHR(
    VkDevice device, VkDeferredOperationKHR deferredOperation, VkPipelineCache pipelineCache, uint32_t count,
    const VkRayTracingPipelineCreateInfoKHR *pCreateInfos, const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
    const RecordObject &record_obj, PipelineStates &pipeline_states, chassis::CreateRayTracingPipelinesKHR &chassis_state) {
    BaseClass::PreCallRecordCreateRayTracingPipelinesKHR(device, deferredOperation, pipelineCache, count, pCreateInfos, pAllocator,
                                                         pPipelines, record_obj, pipeline_states, chassis_state);
    if (!gpuav_settings.shader_instrumentation_enabled) return;

    chassis_state.shader_instrumentations_metadata.resize(count);
    chassis_state.modified_create_infos.resize(count);

    for (uint32_t i = 0; i < count; ++i) {
        const auto &pipeline_state = pipeline_states[i];

        // Need to make a deep copy so if SPIR-V is inlined, user doesn't see it after the call
        auto &new_pipeline_ci = chassis_state.modified_create_infos[i];
        new_pipeline_ci.initialize(&pipeline_state->RayTracingCreateInfo());

        if (!NeedPipelineCreationShaderInstrumentation(*pipeline_state)) {
            continue;
        }

        const Location create_info_loc = record_obj.location.dot(vvl::Field::pCreateInfos, i);
        auto &shader_instrumentation_metadata = chassis_state.shader_instrumentations_metadata[i];

        PreCallRecordPipelineCreationShaderInstrumentation(pAllocator, *pipeline_state, new_pipeline_ci, create_info_loc,
                                                           shader_instrumentation_metadata);
    }

    chassis_state.pCreateInfos = reinterpret_cast<VkRayTracingPipelineCreateInfoKHR *>(chassis_state.modified_create_infos.data());
}

template <typename CreateInfos, typename SafeCreateInfos>
static void UtilCopyCreatePipelineFeedbackData(CreateInfos &create_info, SafeCreateInfos &safe_create_info) {
    auto src_feedback_struct = vku::FindStructInPNextChain<VkPipelineCreationFeedbackCreateInfoEXT>(safe_create_info.pNext);
    if (!src_feedback_struct) return;
    auto dst_feedback_struct = const_cast<VkPipelineCreationFeedbackCreateInfoEXT *>(
        vku::FindStructInPNextChain<VkPipelineCreationFeedbackCreateInfoEXT>(create_info.pNext));
    *dst_feedback_struct->pPipelineCreationFeedback = *src_feedback_struct->pPipelineCreationFeedback;
    for (uint32_t j = 0; j < src_feedback_struct->pipelineStageCreationFeedbackCount; j++) {
        dst_feedback_struct->pPipelineStageCreationFeedbacks[j] = src_feedback_struct->pPipelineStageCreationFeedbacks[j];
    }
}

void GpuShaderInstrumentor::PostCallRecordCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                                  const VkGraphicsPipelineCreateInfo *pCreateInfos,
                                                                  const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                                  const RecordObject &record_obj, PipelineStates &pipeline_states,
                                                                  chassis::CreateGraphicsPipelines &chassis_state) {
    BaseClass::PostCallRecordCreateGraphicsPipelines(device, pipelineCache, count, pCreateInfos, pAllocator, pPipelines, record_obj,
                                                     pipeline_states, chassis_state);
    if (!gpuav_settings.shader_instrumentation_enabled) return;
    for (uint32_t i = 0; i < count; ++i) {
        UtilCopyCreatePipelineFeedbackData(pCreateInfos[i], chassis_state.modified_create_infos[i]);

        auto pipeline_state = Get<vvl::Pipeline>(pPipelines[i]);
        ASSERT_AND_CONTINUE(pipeline_state);

        // Move all instrumentation until the final linking time
        if (pipeline_state->create_flags & VK_PIPELINE_CREATE_LIBRARY_BIT_KHR) continue;

        auto &shader_instrumentation_metadata = chassis_state.shader_instrumentations_metadata[i];
        if (pipeline_state->linking_shaders != 0) {
            PostCallRecordPipelineCreationShaderInstrumentationGPL(*pipeline_state, pAllocator, shader_instrumentation_metadata);
        } else {
            PostCallRecordPipelineCreationShaderInstrumentation(*pipeline_state, shader_instrumentation_metadata);
        }
    }
}

void GpuShaderInstrumentor::PostCallRecordCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                                 const VkComputePipelineCreateInfo *pCreateInfos,
                                                                 const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                                 const RecordObject &record_obj, PipelineStates &pipeline_states,
                                                                 chassis::CreateComputePipelines &chassis_state) {
    BaseClass::PostCallRecordCreateComputePipelines(device, pipelineCache, count, pCreateInfos, pAllocator, pPipelines, record_obj,
                                                    pipeline_states, chassis_state);
    if (!gpuav_settings.shader_instrumentation_enabled) return;
    for (uint32_t i = 0; i < count; ++i) {
        UtilCopyCreatePipelineFeedbackData(pCreateInfos[i], chassis_state.modified_create_infos[i]);

        auto pipeline_state = Get<vvl::Pipeline>(pPipelines[i]);
        ASSERT_AND_CONTINUE(pipeline_state);
        auto &shader_instrumentation_metadata = chassis_state.shader_instrumentations_metadata[i];
        PostCallRecordPipelineCreationShaderInstrumentation(*pipeline_state, shader_instrumentation_metadata);
    }
}

void GpuShaderInstrumentor::PostCallRecordCreateRayTracingPipelinesNV(
    VkDevice device, VkPipelineCache pipelineCache, uint32_t count, const VkRayTracingPipelineCreateInfoNV *pCreateInfos,
    const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines, const RecordObject &record_obj,
    PipelineStates &pipeline_states, chassis::CreateRayTracingPipelinesNV &chassis_state) {
    BaseClass::PostCallRecordCreateRayTracingPipelinesNV(device, pipelineCache, count, pCreateInfos, pAllocator, pPipelines,
                                                         record_obj, pipeline_states, chassis_state);
    if (!gpuav_settings.shader_instrumentation_enabled) return;
    for (uint32_t i = 0; i < count; ++i) {
        UtilCopyCreatePipelineFeedbackData(pCreateInfos[i], chassis_state.modified_create_infos[i]);

        auto pipeline_state = Get<vvl::Pipeline>(pPipelines[i]);
        ASSERT_AND_CONTINUE(pipeline_state);
        auto &shader_instrumentation_metadata = chassis_state.shader_instrumentations_metadata[i];
        PostCallRecordPipelineCreationShaderInstrumentation(*pipeline_state, shader_instrumentation_metadata);
    }
}

void GpuShaderInstrumentor::PostCallRecordCreateRayTracingPipelinesKHR(
    VkDevice device, VkDeferredOperationKHR deferredOperation, VkPipelineCache pipelineCache, uint32_t count,
    const VkRayTracingPipelineCreateInfoKHR *pCreateInfos, const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
    const RecordObject &record_obj, PipelineStates &pipeline_states,
    std::shared_ptr<chassis::CreateRayTracingPipelinesKHR> chassis_state) {
    BaseClass::PostCallRecordCreateRayTracingPipelinesKHR(device, deferredOperation, pipelineCache, count, pCreateInfos, pAllocator,
                                                          pPipelines, record_obj, pipeline_states, chassis_state);
    if (!gpuav_settings.shader_instrumentation_enabled) return;

    const bool is_operation_deferred = deferredOperation != VK_NULL_HANDLE && record_obj.result == VK_OPERATION_DEFERRED_KHR;

    auto layer_data = GetLayerDataPtr(GetDispatchKey(device), layer_data_map);
    if (is_operation_deferred) {
        for (uint32_t i = 0; i < count; ++i) {
            UtilCopyCreatePipelineFeedbackData(pCreateInfos[i], chassis_state->modified_create_infos[i]);
        }

        if (wrap_handles) {
            deferredOperation = layer_data->Unwrap(deferredOperation);
        }

        auto found = layer_data->deferred_operation_post_check.pop(deferredOperation);
        std::vector<std::function<void(const std::vector<VkPipeline> &)>> deferred_op_post_checks;
        if (found->first) {
            deferred_op_post_checks = std::move(found->second);
        } else {
            // ValidationStateTracker::PostCallRecordCreateRayTracingPipelinesKHR should have added a lambda in
            // deferred_operation_post_check for the current deferredOperation.
            // This lambda is responsible for initializing the pipeline state we maintain,
            // this state will be accessed in the following lambda.
            // Given how PostCallRecordCreateRayTracingPipelinesKHR is called in
            // GpuShaderInstrumentor::PostCallRecordCreateRayTracingPipelinesKHR
            // conditions holds as of writing. But it is something we need to be aware of.
            assert(false);
            return;
        }

        deferred_op_post_checks.emplace_back([this, held_chassis_state =
                                                        chassis_state](const std::vector<VkPipeline> &vk_pipelines) mutable {
            for (size_t i = 0; i < vk_pipelines.size(); ++i) {
                std::shared_ptr<vvl::Pipeline> pipeline_state =
                    ((GpuShaderInstrumentor *)this)->Get<vvl::Pipeline>(vk_pipelines[i]);
                ASSERT_AND_CONTINUE(pipeline_state);
                auto &shader_instrumentation_metadata = held_chassis_state->shader_instrumentations_metadata[i];
                PostCallRecordPipelineCreationShaderInstrumentation(*pipeline_state, shader_instrumentation_metadata);
            }
        });
        layer_data->deferred_operation_post_check.insert(deferredOperation, std::move(deferred_op_post_checks));
    } else {
        for (uint32_t i = 0; i < count; ++i) {
            UtilCopyCreatePipelineFeedbackData(pCreateInfos[i], chassis_state->modified_create_infos[i]);

            auto pipeline_state = Get<vvl::Pipeline>(pPipelines[i]);

            auto &shader_instrumentation_metadata = chassis_state->shader_instrumentations_metadata[i];
            PostCallRecordPipelineCreationShaderInstrumentation(*pipeline_state, shader_instrumentation_metadata);
        }
    }
}

// Remove all the shader trackers associated with this destroyed pipeline.
void GpuShaderInstrumentor::PreCallRecordDestroyPipeline(VkDevice device, VkPipeline pipeline,
                                                         const VkAllocationCallbacks *pAllocator, const RecordObject &record_obj) {
    auto to_erase = shader_map_.snapshot([pipeline](const GpuAssistedShaderTracker &entry) { return entry.pipeline == pipeline; });
    for (const auto &entry : to_erase) {
        shader_map_.erase(entry.first);
    }

    if (auto pipeline_state = Get<vvl::Pipeline>(pipeline)) {
        for (auto shader_module : pipeline_state->instrumentation_data.instrumented_shader_module) {
            DispatchDestroyShaderModule(device, shader_module, pAllocator);
        }
        if (pipeline_state->instrumentation_data.pre_raster_lib != VK_NULL_HANDLE) {
            DispatchDestroyPipeline(device, pipeline_state->instrumentation_data.pre_raster_lib, pAllocator);
        }
        if (pipeline_state->instrumentation_data.frag_out_lib != VK_NULL_HANDLE) {
            DispatchDestroyPipeline(device, pipeline_state->instrumentation_data.frag_out_lib, pAllocator);
        }
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

bool GpuShaderInstrumentor::IsSelectiveInstrumentationEnabled(const void *pNext) {
    if (auto features = vku::FindStructInPNextChain<VkValidationFeaturesEXT>(pNext)) {
        for (uint32_t i = 0; i < features->enabledValidationFeatureCount; i++) {
            if (features->pEnabledValidationFeatures[i] == VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT) {
                return true;
            }
        }
    }
    return false;
}

bool GpuShaderInstrumentor::NeedPipelineCreationShaderInstrumentation(vvl::Pipeline &pipeline_state) {
    // will hit with using GPL without shaders in them (ex. fragment output)
    if (pipeline_state.stage_states.empty()) return false;

    // Move all instrumentation until the final linking time
    // This still needs to create a copy of the create_info (we *could* have a mix of GPL and non-GPL)
    if (pipeline_state.create_flags & VK_PIPELINE_CREATE_LIBRARY_BIT_KHR) return false;

    // If the app requests all available sets, the pipeline layout was not modified at pipeline layout creation and the
    // already instrumented shaders need to be replaced with uninstrumented shaders
    if (pipeline_state.active_slots.find(instrumentation_desc_set_bind_index_) != pipeline_state.active_slots.end()) {
        return false;
    }
    const auto pipeline_layout = pipeline_state.PipelineLayoutState();
    if (pipeline_layout && pipeline_layout->set_layouts.size() > instrumentation_desc_set_bind_index_) {
        return false;
    }

    return true;
}

bool GpuShaderInstrumentor::HasBindlessDescriptors(vvl::Pipeline &pipeline_state) {
    const auto pipeline_layout = pipeline_state.PipelineLayoutState();
    if (!pipeline_layout) return false;

    for (const auto &set_layout : pipeline_layout->set_layouts) {
        if (set_layout) {
            for (uint32_t i = 0; i < set_layout->GetBindingCount(); i++) {
                const VkDescriptorBindingFlags flags = set_layout->GetDescriptorBindingFlagsFromIndex(i);
                if (vvl::IsBindless(flags)) {
                    return true;
                }
            }
        }
    }
    return false;
}

bool GpuShaderInstrumentor::HasBindlessDescriptors(VkShaderCreateInfoEXT &create_info) {
    for (const auto [layout_i, set_layout] : vvl::enumerate(create_info.pSetLayouts, create_info.setLayoutCount)) {
        if (auto set_layout_state = Get<vvl::DescriptorSetLayout>(*set_layout)) {
            for (uint32_t i = 0; i < set_layout_state->GetBindingCount(); i++) {
                const VkDescriptorBindingFlags flags = set_layout_state->GetDescriptorBindingFlagsFromIndex(i);
                if (vvl::IsBindless(flags)) {
                    return true;
                }
            }
        }
    }
    return false;
}

// Instrument all SPIR-V that is sent through pipeline. This can be done in various ways
// 1. VkCreateShaderModule and passed in VkShaderModule.
//    For this we create our own VkShaderModule with instrumented shader and manage it inside the pipeline state
// 2. GPL
//    We defer until linking time, otherwise we will instrument many libraries that might never be used.
//    (this also spreads the compile time cost evenly instead of a huge spike on startup)
// 3. Inlined via VkPipelineShaderStageCreateInfo pNext
//    We just instrument the shader and update the inlined SPIR-V
// 4. VK_EXT_shader_module_identifier
//    We will skip these as we don't know the incoming SPIR-V
// Note: Shader Objects are handled in their own path as they don't use pipelines
template <typename SafeCreateInfo>
void GpuShaderInstrumentor::PreCallRecordPipelineCreationShaderInstrumentation(
    const VkAllocationCallbacks *pAllocator, vvl::Pipeline &pipeline_state, SafeCreateInfo &new_pipeline_ci, const Location &loc,
    std::vector<chassis::ShaderInstrumentationMetadata> &shader_instrumentation_metadata) {
    // Init here instead of in chassis so we don't pay cost when GPU-AV is not used
    const size_t total_stages = pipeline_state.stage_states.size();
    shader_instrumentation_metadata.resize(total_stages);

    // TODO - measure and see if would be better to make a gpuav subclasses of pipeline layout and store this information once there
    // (not sure how much pipeline layout re-usage there is)
    const bool has_bindless_descriptors = HasBindlessDescriptors(pipeline_state);

    for (uint32_t i = 0; i < static_cast<uint32_t>(pipeline_state.stage_states.size()); ++i) {
        const auto &stage_state = pipeline_state.stage_states[i];
        auto module_state = std::const_pointer_cast<vvl::ShaderModule>(stage_state.module_state);
        ASSERT_AND_CONTINUE(module_state);
        auto &instrumentation_metadata = shader_instrumentation_metadata[i];

        const VkShaderStageFlagBits stage = stage_state.GetStage();

        // Check pNext for inlined SPIR-V
        auto &stage_ci = GetShaderStageCI<SafeCreateInfo, vku::safe_VkPipelineShaderStageCreateInfo>(new_pipeline_ci, stage);
        // We're modifying the copied, safe create info, which is ok to be non-const
        auto sm_ci = const_cast<vku::safe_VkShaderModuleCreateInfo *>(reinterpret_cast<const vku::safe_VkShaderModuleCreateInfo *>(
            vku::FindStructInPNextChain<VkShaderModuleCreateInfo>(stage_ci.pNext)));

        if (gpuav_settings.select_instrumented_shaders) {
            if (sm_ci && !IsSelectiveInstrumentationEnabled(sm_ci->pNext)) {
                continue;
            } else if (selected_instrumented_shaders.find(module_state->VkHandle()) == selected_instrumented_shaders.end()) {
                continue;
            }
        }

        uint32_t unique_shader_id = 0;
        bool cached = false;
        bool pass = false;
        std::vector<uint32_t> instrumented_spirv;
        if (gpuav_settings.cache_instrumented_shaders) {
            unique_shader_id =
                hash_util::ShaderHash(module_state->spirv->words_.data(), module_state->spirv->words_.size() * sizeof(uint32_t));
            if (const auto spirv = instrumented_shaders_cache_.Get(unique_shader_id)) {
                instrumented_spirv = *spirv;
                cached = true;
            }
        } else {
            unique_shader_id = unique_shader_module_id_++;
        }
        if (!cached) {
            pass =
                InstrumentShader(module_state->spirv->words_, unique_shader_id, has_bindless_descriptors, loc, instrumented_spirv);
        }
        if (cached || pass) {
            instrumentation_metadata.unique_shader_id = unique_shader_id;
            if (module_state->VkHandle() != VK_NULL_HANDLE) {
                // If the user used vkCreateShaderModule, we create a new VkShaderModule to replace with the instrumented
                // shader
                VkShaderModule instrumented_shader_module;
                VkShaderModuleCreateInfo create_info = vku::InitStructHelper();
                create_info.pCode = instrumented_spirv.data();
                create_info.codeSize = instrumented_spirv.size() * sizeof(uint32_t);
                VkResult result = DispatchCreateShaderModule(device, &create_info, pAllocator, &instrumented_shader_module);
                if (result == VK_SUCCESS) {
                    SetShaderModule(new_pipeline_ci, *stage_state.pipeline_create_info, instrumented_shader_module, i);
                    pipeline_state.instrumentation_data.instrumented_shader_module.emplace_back(instrumented_shader_module);
                } else {
                    InternalError(device, loc, "Unable to replace non-instrumented shader with instrumented one.");
                }
            } else if (sm_ci) {
                // The user is inlining the Shader Module into the pipeline, so just need to update the spirv
                instrumentation_metadata.passed_in_shader_stage_ci = true;
                // TODO - This makes a copy, but could save on Chassis stack instead (then remove function from VUL).
                // The core issue is we always use std::vector<uint32_t> but Safe Struct manages its own version of the pCode
                // memory. It would be much harder to change everything from std::vector and instead to adjust Safe Struct to not
                // double-free the memory on us. If making any changes, we have to consider a case where the user inlines the
                // fragment shader, but use a normal VkShaderModule in the vertex shader.
                sm_ci->SetCode(instrumented_spirv);
            } else {
                assert(false);
            }

            if (gpuav_settings.cache_instrumented_shaders && !cached) {
                instrumented_shaders_cache_.Add(unique_shader_id, instrumented_spirv);
            }
        }
    }
}

// Now that we have created the pipeline (and have its handle) build up the shader map for each shader we instrumented
void GpuShaderInstrumentor::PostCallRecordPipelineCreationShaderInstrumentation(
    vvl::Pipeline &pipeline_state, std::vector<chassis::ShaderInstrumentationMetadata> &shader_instrumentation_metadata) {
    // if we return early from NeedPipelineCreationShaderInstrumentation, will need to skip at this point in PostCall
    if (shader_instrumentation_metadata.empty()) return;

    for (uint32_t i = 0; i < static_cast<uint32_t>(pipeline_state.stage_states.size()); ++i) {
        auto &instrumentation_metadata = shader_instrumentation_metadata[i];

        // if the shader for some reason was not instrumented, there is nothing to save
        if (!instrumentation_metadata.IsInstrumented()) {
            continue;
        }
        pipeline_state.instrumentation_data.was_instrumented = true;

        const auto &stage_state = pipeline_state.stage_states[i];
        auto &module_state = stage_state.module_state;

        // We currently need to store a copy of the original, non-instrumented shader so if there is debug information,
        // we can reference it by the instruction number printed out in the shader. Since the application can destroy the
        // original VkShaderModule, there is a chance this will be gone, we need to copy it now.
        // TODO - in the instrumentation, instead of printing the instruction number only, if we print out debug info, we
        // can remove this copy
        std::vector<uint32_t> code;
        if (module_state && module_state->spirv) code = module_state->spirv->words_;

        VkShaderModule shader_module_handle = module_state->VkHandle();
        if (shader_module_handle == VK_NULL_HANDLE && instrumentation_metadata.passed_in_shader_stage_ci) {
            shader_module_handle = kPipelineStageInfoHandle;
        }

        shader_map_.insert_or_assign(instrumentation_metadata.unique_shader_id, pipeline_state.VkHandle(), shader_module_handle,
                                     VK_NULL_HANDLE, std::move(code));
    }
}

// While have an almost duplicated funciton is not ideal, the core issue is we have a single, templated function designed for
// Graphics, Compute, and Ray Tracing. GPL is only for graphics, so we end up needing this "side code path" for graphics only and it
// doesn't fit in the "all pipeline" templated flow.
void GpuShaderInstrumentor::PreCallRecordPipelineCreationShaderInstrumentationGPL(
    const VkAllocationCallbacks *pAllocator, vvl::Pipeline &pipeline_state, vku::safe_VkGraphicsPipelineCreateInfo &new_pipeline_ci,
    const Location &loc, std::vector<chassis::ShaderInstrumentationMetadata> &shader_instrumentation_metadata) {
    // Init here instead of in chassis so we don't pay cost when GPU-AV is not used
    const size_t total_stages = pipeline_state.stage_states.size();
    shader_instrumentation_metadata.resize(total_stages);

    const bool has_bindless_descriptors = HasBindlessDescriptors(pipeline_state);

    auto library_create_info = const_cast<VkPipelineLibraryCreateInfoKHR *>(
        vku::FindStructInPNextChain<VkPipelineLibraryCreateInfoKHR>(new_pipeline_ci.pNext));

    // the "pStages[]" is spread across libraries, so build it up in the double for loop
    uint32_t shader_index = 0;

    // This outer loop is the main difference between the GPL and non-GPL version and why its hard to merge them
    for (uint32_t library_i = 0; library_i < library_create_info->libraryCount; ++library_i) {
        const auto lib = Get<vvl::Pipeline>(library_create_info->pLibraries[library_i]);
        if (!lib) continue;
        if (lib->stage_states.empty()) continue;

        vku::safe_VkGraphicsPipelineCreateInfo new_lib_pipeline_ci(lib->GraphicsCreateInfo());

        for (uint32_t stage_state_i = 0; stage_state_i < static_cast<uint32_t>(lib->stage_states.size()); ++stage_state_i) {
            const auto &stage_state = lib->stage_states[stage_state_i];
            auto module_state = std::const_pointer_cast<vvl::ShaderModule>(stage_state.module_state);
            ASSERT_AND_CONTINUE(module_state);
            auto &instrumentation_metadata = shader_instrumentation_metadata[shader_index++];

            const VkShaderStageFlagBits stage = stage_state.GetStage();

            vku::safe_VkPipelineShaderStageCreateInfo *stage_ci = nullptr;
            // Check pNext for inlined SPIR-V
            for (uint32_t i = 0; i < new_lib_pipeline_ci.stageCount; ++i) {
                if (new_lib_pipeline_ci.pStages[i].stage == stage) {
                    stage_ci = &new_lib_pipeline_ci.pStages[i];
                }
            }

            // We're modifying the copied, safe create info, which is ok to be non-const
            auto sm_ci =
                const_cast<vku::safe_VkShaderModuleCreateInfo *>(reinterpret_cast<const vku::safe_VkShaderModuleCreateInfo *>(
                    vku::FindStructInPNextChain<VkShaderModuleCreateInfo>(stage_ci->pNext)));

            if (gpuav_settings.select_instrumented_shaders) {
                if (sm_ci && !IsSelectiveInstrumentationEnabled(sm_ci->pNext)) {
                    continue;
                } else if (selected_instrumented_shaders.find(module_state->VkHandle()) == selected_instrumented_shaders.end()) {
                    continue;
                }
            }

            uint32_t unique_shader_id = 0;
            bool cached = false;
            bool pass = false;
            std::vector<uint32_t> instrumented_spirv;
            if (gpuav_settings.cache_instrumented_shaders) {
                unique_shader_id = hash_util::ShaderHash(module_state->spirv->words_.data(),
                                                         module_state->spirv->words_.size() * sizeof(uint32_t));
                if (const auto spirv = instrumented_shaders_cache_.Get(unique_shader_id)) {
                    instrumented_spirv = *spirv;
                    cached = true;
                }
            } else {
                unique_shader_id = unique_shader_module_id_++;
            }
            if (!cached) {
                pass = InstrumentShader(module_state->spirv->words_, unique_shader_id, has_bindless_descriptors, loc,
                                        instrumented_spirv);
            }
            if (cached || pass) {
                instrumentation_metadata.unique_shader_id = unique_shader_id;
                if (module_state->VkHandle() != VK_NULL_HANDLE) {
                    // If the user used vkCreateShaderModule, we create a new VkShaderModule to replace with the instrumented
                    // shader
                    VkShaderModule instrumented_shader_module;
                    VkShaderModuleCreateInfo create_info = vku::InitStructHelper();
                    create_info.pCode = instrumented_spirv.data();
                    create_info.codeSize = instrumented_spirv.size() * sizeof(uint32_t);
                    VkResult result = DispatchCreateShaderModule(device, &create_info, pAllocator, &instrumented_shader_module);
                    if (result == VK_SUCCESS) {
                        SetShaderModule(new_lib_pipeline_ci, *stage_state.pipeline_create_info, instrumented_shader_module,
                                        stage_state_i);
                        lib->instrumentation_data.instrumented_shader_module.emplace_back(instrumented_shader_module);
                    } else {
                        InternalError(device, loc, "Unable to replace non-instrumented shader with instrumented one.");
                    }
                } else if (sm_ci) {
                    // The user is inlining the Shader Module into the pipeline, so just need to update the spirv
                    instrumentation_metadata.passed_in_shader_stage_ci = true;
                    // TODO - This makes a copy, but could save on Chassis stack instead (then remove function from VUL).
                    // The core issue is we always use std::vector<uint32_t> but Safe Struct manages its own version of the pCode
                    // memory. It would be much harder to change everything from std::vector and instead to adjust Safe Struct to
                    // not double-free the memory on us. If making any changes, we have to consider a case where the user inlines
                    // the fragment shader, but use a normal VkShaderModule in the vertex shader.
                    sm_ci->SetCode(instrumented_spirv);
                } else {
                    assert(false);
                }

                if (gpuav_settings.cache_instrumented_shaders && !cached) {
                    instrumented_shaders_cache_.Add(unique_shader_id, instrumented_spirv);
                }
            }
        }

        VkPipeline new_lib_pipeline;
        DispatchCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, new_lib_pipeline_ci.ptr(), pAllocator, &new_lib_pipeline);

        if (lib->active_shaders & VK_SHADER_STAGE_FRAGMENT_BIT) {
            pipeline_state.instrumentation_data.frag_out_lib = new_lib_pipeline;
        } else {
            pipeline_state.instrumentation_data.pre_raster_lib = new_lib_pipeline;
        }

        const_cast<VkPipeline *>(library_create_info->pLibraries)[library_i] = new_lib_pipeline;
    }
}

void GpuShaderInstrumentor::PostCallRecordPipelineCreationShaderInstrumentationGPL(
    vvl::Pipeline &pipeline_state, const VkAllocationCallbacks *pAllocator,
    std::vector<chassis::ShaderInstrumentationMetadata> &shader_instrumentation_metadata) {
    // if we return early from NeedPipelineCreationShaderInstrumentation, will need to skip at this point in PostCall
    if (shader_instrumentation_metadata.empty()) return;

    uint32_t shader_index = 0;
    // This outer loop is the main difference between the GPL and non-GPL version and why its hard to merge them
    for (uint32_t library_i = 0; library_i < pipeline_state.library_create_info->libraryCount; ++library_i) {
        const auto lib = Get<vvl::Pipeline>(pipeline_state.library_create_info->pLibraries[library_i]);
        if (!lib) continue;
        if (lib->stage_states.empty()) continue;

        vku::safe_VkGraphicsPipelineCreateInfo new_lib_pipeline_ci(lib->GraphicsCreateInfo());

        for (uint32_t stage_state_i = 0; stage_state_i < static_cast<uint32_t>(lib->stage_states.size()); ++stage_state_i) {
            auto &instrumentation_metadata = shader_instrumentation_metadata[shader_index++];

            // if the shader for some reason was not instrumented, there is nothing to save
            if (!instrumentation_metadata.IsInstrumented()) continue;

            pipeline_state.instrumentation_data.was_instrumented = true;

            const auto &stage_state = lib->stage_states[stage_state_i];
            auto &module_state = stage_state.module_state;

            // We currently need to store a copy of the original, non-instrumented shader so if there is debug information,
            // we can reference it by the instruction number printed out in the shader. Since the application can destroy the
            // original VkShaderModule, there is a chance this will be gone, we need to copy it now.
            // TODO - in the instrumentation, instead of printing the instruction number only, if we print out debug info, we
            // can remove this copy
            std::vector<uint32_t> code;
            if (module_state && module_state->spirv) code = module_state->spirv->words_;

            VkShaderModule shader_module_handle = module_state->VkHandle();
            if (shader_module_handle == VK_NULL_HANDLE && instrumentation_metadata.passed_in_shader_stage_ci) {
                shader_module_handle = kPipelineStageInfoHandle;
            }

            shader_map_.insert_or_assign(instrumentation_metadata.unique_shader_id, lib->VkHandle(), shader_module_handle,
                                         VK_NULL_HANDLE, std::move(code));
        }
    }
}

static bool GpuValidateShader(const std::vector<uint32_t> &input, bool SetRelaxBlockLayout, bool SetScalarBlockLayout,
                              spv_target_env target_env, std::string &error) {
    // Use SPIRV-Tools validator to try and catch any issues with the module
    spv_context ctx = spvContextCreate(target_env);
    spv_const_binary_t binary{input.data(), input.size()};
    spv_diagnostic diag = nullptr;
    spv_validator_options options = spvValidatorOptionsCreate();
    spvValidatorOptionsSetRelaxBlockLayout(options, SetRelaxBlockLayout);
    spvValidatorOptionsSetScalarBlockLayout(options, SetScalarBlockLayout);
    spv_result_t result = spvValidateWithOptions(ctx, options, &binary, &diag);
    if (result != SPV_SUCCESS && diag) error = diag->error;
    return (result == SPV_SUCCESS);
}

// Call the SPIR-V Optimizer to run the instrumentation pass on the shader.
bool GpuShaderInstrumentor::InstrumentShader(const vvl::span<const uint32_t> &input_spirv, uint32_t unique_shader_id,
                                             bool has_bindless_descriptors, const Location &loc,
                                             std::vector<uint32_t> &out_instrumented_spirv) {
    if (input_spirv[0] != spv::MagicNumber) return false;

    if (gpuav_settings.debug_dump_instrumented_shaders) {
        std::string file_name = "dump_" + std::to_string(unique_shader_id) + "_before.spv";
        std::ofstream debug_file(file_name, std::ios::out | std::ios::binary);
        debug_file.write(reinterpret_cast<const char *>(input_spirv.data()),
                         static_cast<std::streamsize>(input_spirv.size() * sizeof(uint32_t)));
    }

    gpu::spirv::Settings module_settings{};
    // Use the unique_shader_id as a shader ID so we can look up its handle later in the shader_map.
    module_settings.shader_id = unique_shader_id;
    module_settings.output_buffer_descriptor_set = instrumentation_desc_set_bind_index_;
    module_settings.print_debug_info = gpuav_settings.debug_print_instrumentation_info;
    module_settings.max_instrumented_count = gpuav_settings.debug_max_instrumented_count;
    module_settings.support_int64 = enabled_features.shaderInt64;
    module_settings.support_memory_model_device_scope = enabled_features.vulkanMemoryModelDeviceScope;
    module_settings.has_bindless_descriptors = has_bindless_descriptors;

    gpu::spirv::Module module(input_spirv, debug_report, module_settings);

    // For now, we don't yet support (or have tested) combining GPU-AV and DebugPrintf, so have 2 paths here
    const bool is_debug_printf = container_type == LayerObjectTypeDebugPrintf;

    bool modified = false;
    if (is_debug_printf) {
        modified |= module.RunPassDebugPrintf(debug_printf_binding_slot_);
    } else {
        GpuAVSettings::ShaderInstrumentation &shader_instrumentation = gpuav_settings.shader_instrumentation;
        // If descriptor indexing is enabled, enable length checks and updated descriptor checks
        if (shader_instrumentation.bindless_descriptor) {
            modified |= module.RunPassBindlessDescriptor();
            modified |= module.RunPassNonBindlessOOBBuffer();
            modified |= module.RunPassNonBindlessOOBTexelBuffer();
        }

        if (shader_instrumentation.buffer_device_address) {
            modified |= module.RunPassBufferDeviceAddress();
        }

        if (shader_instrumentation.ray_query) {
            modified |= module.RunPassRayQuery();
        }

        // If there were GLSL written function injected, we will grab them and link them in here
        for (const auto &info : module.link_info_) {
            module.LinkFunction(info);
        }
    }

    // If nothing was instrumented, leave early to save time
    if (!modified) {
        return false;
    }

    // some small cleanup to make sure SPIR-V is legal
    module.PostProcess();
    // translate internal representation of SPIR-V into legal SPIR-V binary
    module.ToBinary(out_instrumented_spirv);

    if (gpuav_settings.debug_dump_instrumented_shaders) {
        std::string file_name = "dump_" + std::to_string(unique_shader_id) + "_after.spv";
        std::ofstream debug_file(file_name, std::ios::out | std::ios::binary);
        debug_file.write(reinterpret_cast<char *>(out_instrumented_spirv.data()),
                         static_cast<std::streamsize>(out_instrumented_spirv.size() * sizeof(uint32_t)));
    }

    spv_target_env target_env = PickSpirvEnv(api_version, IsExtEnabled(device_extensions.vk_khr_spirv_1_4));
    // (Maybe) validate the instrumented and linked shader
    if (gpuav_settings.debug_validate_instrumented_shaders) {
        std::string instrumented_error;
        if (!GpuValidateShader(out_instrumented_spirv, device_extensions.vk_khr_relaxed_block_layout,
                               device_extensions.vk_ext_scalar_block_layout, target_env, instrumented_error)) {
            std::ostringstream strm;
            strm << "Instrumented shader (id " << unique_shader_id << ") is invalid, spirv-val error:\n"
                 << instrumented_error << " Proceeding with non instrumented shader.";
            InternalError(device, loc, strm.str().c_str());
            return false;
        }
    }

    // Run Dead Code elimination
    // If DebugPrintf is the only thing, there will be nothing to eliminate so don't waste time on it
    if (!is_debug_printf) {
        using namespace spvtools;
        OptimizerOptions opt_options;
        opt_options.set_run_validator(false);
        Optimizer dce_pass(target_env);

        const MessageConsumer gpu_console_message_consumer =
            [this, loc](spv_message_level_t level, const char *, const spv_position_t &position, const char *message) -> void {
            switch (level) {
                case SPV_MSG_FATAL:
                case SPV_MSG_INTERNAL_ERROR:
                case SPV_MSG_ERROR:
                    this->LogError("UNASSIGNED-GPU-Assisted", this->device, loc,
                                   "Error during shader instrumentation: line %zu: %s", position.index, message);
                    break;
                default:
                    break;
            }
        };

        dce_pass.SetMessageConsumer(gpu_console_message_consumer);
        // Call CreateAggressiveDCEPass with preserve_interface == true
        dce_pass.RegisterPass(CreateAggressiveDCEPass(true));
        if (!dce_pass.Run(out_instrumented_spirv.data(), out_instrumented_spirv.size(), &out_instrumented_spirv, opt_options)) {
            InternalError(device, loc,
                          "Failure to run spirv-opt DCE on instrumented shader. Proceeding with non-instrumented shader.");
            return false;
        }

        if (gpuav_settings.debug_dump_instrumented_shaders) {
            std::string file_name = "dump_" + std::to_string(unique_shader_id) + "_opt.spv";
            std::ofstream debug_file(file_name, std::ios::out | std::ios::binary);
            debug_file.write(reinterpret_cast<char *>(out_instrumented_spirv.data()),
                             static_cast<std::streamsize>(out_instrumented_spirv.size() * sizeof(uint32_t)));
        }
    }

    return true;
}

VkDeviceAddress GpuShaderInstrumentor::GetBufferDeviceAddressHelper(VkBuffer buffer) const {
    VkBufferDeviceAddressInfo address_info = vku::InitStructHelper();
    address_info.buffer = buffer;

    if (api_version >= VK_API_VERSION_1_2) {
        return DispatchGetBufferDeviceAddress(device, &address_info);
    } else {
        return DispatchGetBufferDeviceAddressKHR(device, &address_info);
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

    char const *layer_name = container_type == LayerObjectTypeDebugPrintf ? "DebugPrintf" : "GPU-AV";
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
        const auto &insn = instructions[i];
        if ((insn.Opcode() == spv::OpSource) && (insn.Length() >= 5) && (insn.Word(3) == reported_file_id)) {
            std::istringstream in_stream;
            std::string cur_line;
            in_stream.str(insn.GetAsString(4));
            while (std::getline(in_stream, cur_line)) {
                opsource_lines.emplace_back(cur_line);
            }

            for (size_t k = i + 1; k < instructions.size(); k++) {
                const auto &continue_insn = instructions[k];
                if (continue_insn.Opcode() != spv::OpSourceContinued) {
                    break;
                }
                in_stream.str(continue_insn.GetAsString(1));
                while (std::getline(in_stream, cur_line)) {
                    opsource_lines.emplace_back(cur_line);
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

// Generate the stage-specific part of the message.
static void GenerateStageMessage(std::ostringstream &ss, uint32_t stage_id, uint32_t stage_info_0, uint32_t stage_info_1,
                                 uint32_t stage_info_2) {
    switch (stage_id) {
        case gpuav::glsl::kHeaderStageIdMultiEntryPoint: {
            ss << "Stage has multiple OpEntryPoint and could not detect stage. ";
        } break;
        case spv::ExecutionModelVertex: {
            ss << "Stage = Vertex. Vertex Index = " << stage_info_0 << " Instance Index = " << stage_info_1 << ". ";
        } break;
        case spv::ExecutionModelTessellationControl: {
            ss << "Stage = Tessellation Control.  Invocation ID = " << stage_info_0 << ", Primitive ID = " << stage_info_1;
        } break;
        case spv::ExecutionModelTessellationEvaluation: {
            ss << "Stage = Tessellation Eval.  Primitive ID = " << stage_info_0 << ", TessCoord (u, v) = (" << stage_info_1 << ", "
               << stage_info_2 << "). ";
        } break;
        case spv::ExecutionModelGeometry: {
            ss << "Stage = Geometry.  Primitive ID = " << stage_info_0 << " Invocation ID = " << stage_info_1 << ". ";
        } break;
        case spv::ExecutionModelFragment: {
            // Should use std::bit_cast but requires c++20
            float x_coord;
            float y_coord;
            std::memcpy(&x_coord, &stage_info_0, sizeof(float));
            std::memcpy(&y_coord, &stage_info_1, sizeof(float));
            ss << "Stage = Fragment.  Fragment coord (x,y) = (" << x_coord << ", " << y_coord << "). ";
        } break;
        case spv::ExecutionModelGLCompute: {
            ss << "Stage = Compute.  Global invocation ID (x, y, z) = (" << stage_info_0 << ", " << stage_info_1 << ", "
               << stage_info_2 << ")";
        } break;
        case spv::ExecutionModelRayGenerationKHR: {
            ss << "Stage = Ray Generation.  Global Launch ID (x,y,z) = (" << stage_info_0 << ", " << stage_info_1 << ", "
               << stage_info_2 << "). ";
        } break;
        case spv::ExecutionModelIntersectionKHR: {
            ss << "Stage = Intersection.  Global Launch ID (x,y,z) = (" << stage_info_0 << ", " << stage_info_1 << ", "
               << stage_info_2 << "). ";
        } break;
        case spv::ExecutionModelAnyHitKHR: {
            ss << "Stage = Any Hit.  Global Launch ID (x,y,z) = (" << stage_info_0 << ", " << stage_info_1 << ", " << stage_info_2
               << "). ";
        } break;
        case spv::ExecutionModelClosestHitKHR: {
            ss << "Stage = Closest Hit.  Global Launch ID (x,y,z) = (" << stage_info_0 << ", " << stage_info_1 << ", "
               << stage_info_2 << "). ";
        } break;
        case spv::ExecutionModelMissKHR: {
            ss << "Stage = Miss.  Global Launch ID (x,y,z) = (" << stage_info_0 << ", " << stage_info_1 << ", " << stage_info_2
               << "). ";
        } break;
        case spv::ExecutionModelCallableKHR: {
            ss << "Stage = Callable.  Global Launch ID (x,y,z) = (" << stage_info_0 << ", " << stage_info_1 << ", " << stage_info_2
               << "). ";
        } break;
        case spv::ExecutionModelTaskEXT: {
            ss << "Stage = TaskEXT. Global invocation ID (x, y, z) = (" << stage_info_0 << ", " << stage_info_1 << ", "
               << stage_info_2 << " )";
        } break;
        case spv::ExecutionModelMeshEXT: {
            ss << "Stage = MeshEXT. Global invocation ID (x, y, z) = (" << stage_info_0 << ", " << stage_info_1 << ", "
               << stage_info_2 << " )";
        } break;
        case spv::ExecutionModelTaskNV: {
            ss << "Stage = TaskNV. Global invocation ID (x, y, z) = (" << stage_info_0 << ", " << stage_info_1 << ", "
               << stage_info_2 << " )";
        } break;
        case spv::ExecutionModelMeshNV: {
            ss << "Stage = MeshNV. Global invocation ID (x, y, z) = (" << stage_info_0 << ", " << stage_info_1 << ", "
               << stage_info_2 << " )";
        } break;
        default: {
            ss << "Internal Error (unexpected stage = " << stage_id << "). ";
            assert(false);
        } break;
    }
    ss << '\n';
}

// Where we build up the error message with all the useful debug information about where the error occured
std::string GpuShaderInstrumentor::GenerateDebugInfoMessage(
    VkCommandBuffer commandBuffer, const std::vector<spirv::Instruction> &instructions, uint32_t stage_id, uint32_t stage_info_0,
    uint32_t stage_info_1, uint32_t stage_info_2, uint32_t instruction_position, const gpu::GpuAssistedShaderTracker *tracker_info,
    uint32_t shader_id, VkPipelineBindPoint pipeline_bind_point, uint32_t operation_index) const {
    std::ostringstream ss;
    if (instructions.empty() || !tracker_info) {
        ss << "[Internal Error] - Can't get instructions from shader_map\n";
        return ss.str();
    }

    GenerateStageMessage(ss, stage_id, stage_info_0, stage_info_1, stage_info_2);

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
        ss << '\t';  // helps to show that the index is expressed with respect to the command buffer
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
        ss << "Index " << operation_index << '\n';
        ss << std::hex << std::noshowbase;

        if (tracker_info->shader_module == VK_NULL_HANDLE) {
            ss << "Shader Object " << LookupDebugUtilsNameNoLock(debug_report, HandleToUint64(tracker_info->shader_object)) << "("
               << HandleToUint64(tracker_info->shader_object) << ") (internal ID " << shader_id << ")\n";
        } else {
            ss << "Pipeline " << LookupDebugUtilsNameNoLock(debug_report, HandleToUint64(tracker_info->pipeline)) << "("
               << HandleToUint64(tracker_info->pipeline) << ")";
            if (tracker_info->shader_module == gpu::kPipelineStageInfoHandle) {
                ss << " (internal ID " << shader_id
                   << ")\nShader Module was passed in via VkPipelineShaderStageCreateInfo::pNext\n";
            } else {
                ss << "\nShader Module " << LookupDebugUtilsNameNoLock(debug_report, HandleToUint64(tracker_info->shader_module))
                   << "(" << HandleToUint64(tracker_info->shader_module) << ") (internal ID " << shader_id << ")\n";
            }
        }
    }

    ss << std::dec << std::noshowbase;
    ss << "SPIR-V Instruction Index = " << instruction_position << '\n';

    // Find the OpLine just before the failing instruction indicated by the debug info.
    // SPIR-V can only be iterated in the forward direction due to its opcode/length encoding.
    uint32_t index = 0;
    uint32_t reported_file_id = 0;
    uint32_t reported_line_number = 0;
    uint32_t reported_column_number = 0;
    for (const auto &insn : instructions) {
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
    for (const auto &insn : instructions) {
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
            ss << '\n';
            break;
        }
        // OpString can only be in the debug section, so can break early if not found
        if (insn.Opcode() == spv::OpFunction) break;
    }

    if (!found_opstring) {
        ss << "Unable to find SPIR-V OpString from OpLine instruction.\n";
        ss << "File ID = " << reported_file_id << ", Line Number = " << reported_line_number
           << ", Column = " << reported_column_number << '\n';
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
                ss << '\n' << reported_line_number << ": " << opsource_lines[opsource_index] << '\n';
            } else {
                ss << "Internal error: calculated source line of " << opsource_index << " for source size of "
                   << opsource_lines.size() << " lines\n";
            }
        } else if (reported_line_number < opsource_lines.size() && reported_line_number != 0) {
            // file lines normally start at 1 index
            ss << '\n' << opsource_lines[reported_line_number - 1] << '\n';
            if (reported_column_number > 0) {
                std::string spaces(reported_column_number - 1, ' ');
                ss << spaces << '^';
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
