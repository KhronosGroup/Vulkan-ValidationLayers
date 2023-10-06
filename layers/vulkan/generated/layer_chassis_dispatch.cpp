// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See layer_chassis_dispatch_generator.py for modifications

/***************************************************************************
 *
 * Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
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
 ****************************************************************************/

// NOLINTBEGIN

#include "utils/cast_utils.h"
#include "chassis.h"
#include "layer_chassis_dispatch.h"
#include "vk_safe_struct.h"
#include "state_tracker/pipeline_state.h"

#define DISPATCH_MAX_STACK_ALLOCATIONS 32

// Unique Objects pNext extension handling function
void WrapPnextChainHandles(ValidationObject* layer_data, const void* pNext) {
    void* cur_pnext = const_cast<void*>(pNext);
    while (cur_pnext != nullptr) {
        VkBaseOutStructure* header = reinterpret_cast<VkBaseOutStructure*>(cur_pnext);

        switch (header->sType) {
            case VK_STRUCTURE_TYPE_FRAME_BOUNDARY_EXT: {
                safe_VkFrameBoundaryEXT* safe_struct = reinterpret_cast<safe_VkFrameBoundaryEXT*>(cur_pnext);
                if (safe_struct->pImages) {
                    for (uint32_t index0 = 0; index0 < safe_struct->imageCount; ++index0) {
                        safe_struct->pImages[index0] = layer_data->Unwrap(safe_struct->pImages[index0]);
                    }
                }
                if (safe_struct->pBuffers) {
                    for (uint32_t index0 = 0; index0 < safe_struct->bufferCount; ++index0) {
                        safe_struct->pBuffers[index0] = layer_data->Unwrap(safe_struct->pBuffers[index0]);
                    }
                }
            } break;

#ifdef VK_USE_PLATFORM_WIN32_KHR
            case VK_STRUCTURE_TYPE_WIN32_KEYED_MUTEX_ACQUIRE_RELEASE_INFO_KHR: {
                safe_VkWin32KeyedMutexAcquireReleaseInfoKHR* safe_struct =
                    reinterpret_cast<safe_VkWin32KeyedMutexAcquireReleaseInfoKHR*>(cur_pnext);
                if (safe_struct->pAcquireSyncs) {
                    for (uint32_t index0 = 0; index0 < safe_struct->acquireCount; ++index0) {
                        safe_struct->pAcquireSyncs[index0] = layer_data->Unwrap(safe_struct->pAcquireSyncs[index0]);
                    }
                }
                if (safe_struct->pReleaseSyncs) {
                    for (uint32_t index0 = 0; index0 < safe_struct->releaseCount; ++index0) {
                        safe_struct->pReleaseSyncs[index0] = layer_data->Unwrap(safe_struct->pReleaseSyncs[index0]);
                    }
                }
            } break;
#endif  // VK_USE_PLATFORM_WIN32_KHR

#ifdef VK_USE_PLATFORM_WIN32_KHR
            case VK_STRUCTURE_TYPE_WIN32_KEYED_MUTEX_ACQUIRE_RELEASE_INFO_NV: {
                safe_VkWin32KeyedMutexAcquireReleaseInfoNV* safe_struct =
                    reinterpret_cast<safe_VkWin32KeyedMutexAcquireReleaseInfoNV*>(cur_pnext);
                if (safe_struct->pAcquireSyncs) {
                    for (uint32_t index0 = 0; index0 < safe_struct->acquireCount; ++index0) {
                        safe_struct->pAcquireSyncs[index0] = layer_data->Unwrap(safe_struct->pAcquireSyncs[index0]);
                    }
                }
                if (safe_struct->pReleaseSyncs) {
                    for (uint32_t index0 = 0; index0 < safe_struct->releaseCount; ++index0) {
                        safe_struct->pReleaseSyncs[index0] = layer_data->Unwrap(safe_struct->pReleaseSyncs[index0]);
                    }
                }
            } break;
#endif  // VK_USE_PLATFORM_WIN32_KHR

            case VK_STRUCTURE_TYPE_DEDICATED_ALLOCATION_MEMORY_ALLOCATE_INFO_NV: {
                safe_VkDedicatedAllocationMemoryAllocateInfoNV* safe_struct =
                    reinterpret_cast<safe_VkDedicatedAllocationMemoryAllocateInfoNV*>(cur_pnext);

                if (safe_struct->image) {
                    safe_struct->image = layer_data->Unwrap(safe_struct->image);
                }
                if (safe_struct->buffer) {
                    safe_struct->buffer = layer_data->Unwrap(safe_struct->buffer);
                }
            } break;

#ifdef VK_USE_PLATFORM_FUCHSIA
            case VK_STRUCTURE_TYPE_IMPORT_MEMORY_BUFFER_COLLECTION_FUCHSIA: {
                safe_VkImportMemoryBufferCollectionFUCHSIA* safe_struct =
                    reinterpret_cast<safe_VkImportMemoryBufferCollectionFUCHSIA*>(cur_pnext);

                if (safe_struct->collection) {
                    safe_struct->collection = layer_data->Unwrap(safe_struct->collection);
                }
            } break;
#endif  // VK_USE_PLATFORM_FUCHSIA

            case VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO: {
                safe_VkMemoryDedicatedAllocateInfo* safe_struct = reinterpret_cast<safe_VkMemoryDedicatedAllocateInfo*>(cur_pnext);

                if (safe_struct->image) {
                    safe_struct->image = layer_data->Unwrap(safe_struct->image);
                }
                if (safe_struct->buffer) {
                    safe_struct->buffer = layer_data->Unwrap(safe_struct->buffer);
                }
            } break;

#ifdef VK_USE_PLATFORM_FUCHSIA
            case VK_STRUCTURE_TYPE_BUFFER_COLLECTION_BUFFER_CREATE_INFO_FUCHSIA: {
                safe_VkBufferCollectionBufferCreateInfoFUCHSIA* safe_struct =
                    reinterpret_cast<safe_VkBufferCollectionBufferCreateInfoFUCHSIA*>(cur_pnext);

                if (safe_struct->collection) {
                    safe_struct->collection = layer_data->Unwrap(safe_struct->collection);
                }
            } break;
#endif  // VK_USE_PLATFORM_FUCHSIA

#ifdef VK_USE_PLATFORM_FUCHSIA
            case VK_STRUCTURE_TYPE_BUFFER_COLLECTION_IMAGE_CREATE_INFO_FUCHSIA: {
                safe_VkBufferCollectionImageCreateInfoFUCHSIA* safe_struct =
                    reinterpret_cast<safe_VkBufferCollectionImageCreateInfoFUCHSIA*>(cur_pnext);

                if (safe_struct->collection) {
                    safe_struct->collection = layer_data->Unwrap(safe_struct->collection);
                }
            } break;
#endif  // VK_USE_PLATFORM_FUCHSIA

            case VK_STRUCTURE_TYPE_IMAGE_SWAPCHAIN_CREATE_INFO_KHR: {
                safe_VkImageSwapchainCreateInfoKHR* safe_struct = reinterpret_cast<safe_VkImageSwapchainCreateInfoKHR*>(cur_pnext);

                if (safe_struct->swapchain) {
                    safe_struct->swapchain = layer_data->Unwrap(safe_struct->swapchain);
                }
            } break;

            case VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_INFO: {
                safe_VkSamplerYcbcrConversionInfo* safe_struct = reinterpret_cast<safe_VkSamplerYcbcrConversionInfo*>(cur_pnext);

                if (safe_struct->conversion) {
                    safe_struct->conversion = layer_data->Unwrap(safe_struct->conversion);
                }
            } break;

            case VK_STRUCTURE_TYPE_SHADER_MODULE_VALIDATION_CACHE_CREATE_INFO_EXT: {
                safe_VkShaderModuleValidationCacheCreateInfoEXT* safe_struct =
                    reinterpret_cast<safe_VkShaderModuleValidationCacheCreateInfoEXT*>(cur_pnext);

                if (safe_struct->validationCache) {
                    safe_struct->validationCache = layer_data->Unwrap(safe_struct->validationCache);
                }
            } break;

            case VK_STRUCTURE_TYPE_SUBPASS_SHADING_PIPELINE_CREATE_INFO_HUAWEI: {
                safe_VkSubpassShadingPipelineCreateInfoHUAWEI* safe_struct =
                    reinterpret_cast<safe_VkSubpassShadingPipelineCreateInfoHUAWEI*>(cur_pnext);

                if (safe_struct->renderPass) {
                    safe_struct->renderPass = layer_data->Unwrap(safe_struct->renderPass);
                }
            } break;

            case VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_SHADER_GROUPS_CREATE_INFO_NV: {
                safe_VkGraphicsPipelineShaderGroupsCreateInfoNV* safe_struct =
                    reinterpret_cast<safe_VkGraphicsPipelineShaderGroupsCreateInfoNV*>(cur_pnext);
                if (safe_struct->pGroups) {
                    for (uint32_t index0 = 0; index0 < safe_struct->groupCount; ++index0) {
                        if (safe_struct->pGroups[index0].pStages) {
                            for (uint32_t index1 = 0; index1 < safe_struct->pGroups[index0].stageCount; ++index1) {
                                if (safe_struct->pGroups[index0].pStages[index1].module) {
                                    safe_struct->pGroups[index0].pStages[index1].module =
                                        layer_data->Unwrap(safe_struct->pGroups[index0].pStages[index1].module);
                                }
                            }
                        }
                    }
                }
                if (safe_struct->pPipelines) {
                    for (uint32_t index0 = 0; index0 < safe_struct->pipelineCount; ++index0) {
                        safe_struct->pPipelines[index0] = layer_data->Unwrap(safe_struct->pPipelines[index0]);
                    }
                }
            } break;

            case VK_STRUCTURE_TYPE_PIPELINE_LIBRARY_CREATE_INFO_KHR: {
                safe_VkPipelineLibraryCreateInfoKHR* safe_struct =
                    reinterpret_cast<safe_VkPipelineLibraryCreateInfoKHR*>(cur_pnext);
                if (safe_struct->pLibraries) {
                    for (uint32_t index0 = 0; index0 < safe_struct->libraryCount; ++index0) {
                        safe_struct->pLibraries[index0] = layer_data->Unwrap(safe_struct->pLibraries[index0]);
                    }
                }
            } break;

            case VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR: {
                safe_VkWriteDescriptorSetAccelerationStructureKHR* safe_struct =
                    reinterpret_cast<safe_VkWriteDescriptorSetAccelerationStructureKHR*>(cur_pnext);
                if (safe_struct->pAccelerationStructures) {
                    for (uint32_t index0 = 0; index0 < safe_struct->accelerationStructureCount; ++index0) {
                        safe_struct->pAccelerationStructures[index0] =
                            layer_data->Unwrap(safe_struct->pAccelerationStructures[index0]);
                    }
                }
            } break;

            case VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_NV: {
                safe_VkWriteDescriptorSetAccelerationStructureNV* safe_struct =
                    reinterpret_cast<safe_VkWriteDescriptorSetAccelerationStructureNV*>(cur_pnext);
                if (safe_struct->pAccelerationStructures) {
                    for (uint32_t index0 = 0; index0 < safe_struct->accelerationStructureCount; ++index0) {
                        safe_struct->pAccelerationStructures[index0] =
                            layer_data->Unwrap(safe_struct->pAccelerationStructures[index0]);
                    }
                }
            } break;

            case VK_STRUCTURE_TYPE_RENDER_PASS_ATTACHMENT_BEGIN_INFO: {
                safe_VkRenderPassAttachmentBeginInfo* safe_struct =
                    reinterpret_cast<safe_VkRenderPassAttachmentBeginInfo*>(cur_pnext);
                if (safe_struct->pAttachments) {
                    for (uint32_t index0 = 0; index0 < safe_struct->attachmentCount; ++index0) {
                        safe_struct->pAttachments[index0] = layer_data->Unwrap(safe_struct->pAttachments[index0]);
                    }
                }
            } break;

            case VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_SWAPCHAIN_INFO_KHR: {
                safe_VkBindImageMemorySwapchainInfoKHR* safe_struct =
                    reinterpret_cast<safe_VkBindImageMemorySwapchainInfoKHR*>(cur_pnext);

                if (safe_struct->swapchain) {
                    safe_struct->swapchain = layer_data->Unwrap(safe_struct->swapchain);
                }
            } break;

            case VK_STRUCTURE_TYPE_RENDERING_FRAGMENT_DENSITY_MAP_ATTACHMENT_INFO_EXT: {
                safe_VkRenderingFragmentDensityMapAttachmentInfoEXT* safe_struct =
                    reinterpret_cast<safe_VkRenderingFragmentDensityMapAttachmentInfoEXT*>(cur_pnext);

                if (safe_struct->imageView) {
                    safe_struct->imageView = layer_data->Unwrap(safe_struct->imageView);
                }
            } break;

            case VK_STRUCTURE_TYPE_RENDERING_FRAGMENT_SHADING_RATE_ATTACHMENT_INFO_KHR: {
                safe_VkRenderingFragmentShadingRateAttachmentInfoKHR* safe_struct =
                    reinterpret_cast<safe_VkRenderingFragmentShadingRateAttachmentInfoKHR*>(cur_pnext);

                if (safe_struct->imageView) {
                    safe_struct->imageView = layer_data->Unwrap(safe_struct->imageView);
                }
            } break;

            case VK_STRUCTURE_TYPE_SWAPCHAIN_PRESENT_FENCE_INFO_EXT: {
                safe_VkSwapchainPresentFenceInfoEXT* safe_struct =
                    reinterpret_cast<safe_VkSwapchainPresentFenceInfoEXT*>(cur_pnext);
                if (safe_struct->pFences) {
                    for (uint32_t index0 = 0; index0 < safe_struct->swapchainCount; ++index0) {
                        safe_struct->pFences[index0] = layer_data->Unwrap(safe_struct->pFences[index0]);
                    }
                }
            } break;

#ifdef VK_USE_PLATFORM_METAL_EXT
            case VK_STRUCTURE_TYPE_EXPORT_METAL_BUFFER_INFO_EXT: {
                safe_VkExportMetalBufferInfoEXT* safe_struct = reinterpret_cast<safe_VkExportMetalBufferInfoEXT*>(cur_pnext);

                if (safe_struct->memory) {
                    safe_struct->memory = layer_data->Unwrap(safe_struct->memory);
                }
            } break;
#endif  // VK_USE_PLATFORM_METAL_EXT

#ifdef VK_USE_PLATFORM_METAL_EXT
            case VK_STRUCTURE_TYPE_EXPORT_METAL_IO_SURFACE_INFO_EXT: {
                safe_VkExportMetalIOSurfaceInfoEXT* safe_struct = reinterpret_cast<safe_VkExportMetalIOSurfaceInfoEXT*>(cur_pnext);

                if (safe_struct->image) {
                    safe_struct->image = layer_data->Unwrap(safe_struct->image);
                }
            } break;
#endif  // VK_USE_PLATFORM_METAL_EXT

#ifdef VK_USE_PLATFORM_METAL_EXT
            case VK_STRUCTURE_TYPE_EXPORT_METAL_SHARED_EVENT_INFO_EXT: {
                safe_VkExportMetalSharedEventInfoEXT* safe_struct =
                    reinterpret_cast<safe_VkExportMetalSharedEventInfoEXT*>(cur_pnext);

                if (safe_struct->semaphore) {
                    safe_struct->semaphore = layer_data->Unwrap(safe_struct->semaphore);
                }
                if (safe_struct->event) {
                    safe_struct->event = layer_data->Unwrap(safe_struct->event);
                }
            } break;
#endif  // VK_USE_PLATFORM_METAL_EXT

#ifdef VK_USE_PLATFORM_METAL_EXT
            case VK_STRUCTURE_TYPE_EXPORT_METAL_TEXTURE_INFO_EXT: {
                safe_VkExportMetalTextureInfoEXT* safe_struct = reinterpret_cast<safe_VkExportMetalTextureInfoEXT*>(cur_pnext);

                if (safe_struct->image) {
                    safe_struct->image = layer_data->Unwrap(safe_struct->image);
                }
                if (safe_struct->imageView) {
                    safe_struct->imageView = layer_data->Unwrap(safe_struct->imageView);
                }
                if (safe_struct->bufferView) {
                    safe_struct->bufferView = layer_data->Unwrap(safe_struct->bufferView);
                }
            } break;
#endif  // VK_USE_PLATFORM_METAL_EXT

            case VK_STRUCTURE_TYPE_DESCRIPTOR_BUFFER_BINDING_PUSH_DESCRIPTOR_BUFFER_HANDLE_EXT: {
                safe_VkDescriptorBufferBindingPushDescriptorBufferHandleEXT* safe_struct =
                    reinterpret_cast<safe_VkDescriptorBufferBindingPushDescriptorBufferHandleEXT*>(cur_pnext);

                if (safe_struct->buffer) {
                    safe_struct->buffer = layer_data->Unwrap(safe_struct->buffer);
                }
            } break;

#ifdef VK_ENABLE_BETA_EXTENSIONS
            case VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_TRIANGLES_DISPLACEMENT_MICROMAP_NV: {
                safe_VkAccelerationStructureTrianglesDisplacementMicromapNV* safe_struct =
                    reinterpret_cast<safe_VkAccelerationStructureTrianglesDisplacementMicromapNV*>(cur_pnext);

                if (safe_struct->micromap) {
                    safe_struct->micromap = layer_data->Unwrap(safe_struct->micromap);
                }
            } break;
#endif  // VK_ENABLE_BETA_EXTENSIONS

            case VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_TRIANGLES_OPACITY_MICROMAP_EXT: {
                safe_VkAccelerationStructureTrianglesOpacityMicromapEXT* safe_struct =
                    reinterpret_cast<safe_VkAccelerationStructureTrianglesOpacityMicromapEXT*>(cur_pnext);

                if (safe_struct->micromap) {
                    safe_struct->micromap = layer_data->Unwrap(safe_struct->micromap);
                }
            } break;

            default:
                break;
        }

        // Process the next structure in the chain
        cur_pnext = header->pNext;
    }
}

VkResult DispatchEnumeratePhysicalDevices(VkInstance instance, uint32_t* pPhysicalDeviceCount, VkPhysicalDevice* pPhysicalDevices) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(instance), layer_data_map);

    VkResult result =
        layer_data->instance_dispatch_table.EnumeratePhysicalDevices(instance, pPhysicalDeviceCount, pPhysicalDevices);

    return result;
}

void DispatchGetPhysicalDeviceFeatures(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures* pFeatures) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);

    layer_data->instance_dispatch_table.GetPhysicalDeviceFeatures(physicalDevice, pFeatures);
}

void DispatchGetPhysicalDeviceFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format,
                                               VkFormatProperties* pFormatProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);

    layer_data->instance_dispatch_table.GetPhysicalDeviceFormatProperties(physicalDevice, format, pFormatProperties);
}

VkResult DispatchGetPhysicalDeviceImageFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type,
                                                        VkImageTiling tiling, VkImageUsageFlags usage, VkImageCreateFlags flags,
                                                        VkImageFormatProperties* pImageFormatProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);

    VkResult result = layer_data->instance_dispatch_table.GetPhysicalDeviceImageFormatProperties(
        physicalDevice, format, type, tiling, usage, flags, pImageFormatProperties);

    return result;
}

void DispatchGetPhysicalDeviceProperties(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties* pProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);

    layer_data->instance_dispatch_table.GetPhysicalDeviceProperties(physicalDevice, pProperties);
}

void DispatchGetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount,
                                                    VkQueueFamilyProperties* pQueueFamilyProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);

    layer_data->instance_dispatch_table.GetPhysicalDeviceQueueFamilyProperties(physicalDevice, pQueueFamilyPropertyCount,
                                                                               pQueueFamilyProperties);
}

void DispatchGetPhysicalDeviceMemoryProperties(VkPhysicalDevice physicalDevice,
                                               VkPhysicalDeviceMemoryProperties* pMemoryProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);

    layer_data->instance_dispatch_table.GetPhysicalDeviceMemoryProperties(physicalDevice, pMemoryProperties);
}

PFN_vkVoidFunction DispatchGetInstanceProcAddr(VkInstance instance, const char* pName) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(instance), layer_data_map);

    PFN_vkVoidFunction result = layer_data->instance_dispatch_table.GetInstanceProcAddr(instance, pName);

    return result;
}

PFN_vkVoidFunction DispatchGetDeviceProcAddr(VkDevice device, const char* pName) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);

    PFN_vkVoidFunction result = layer_data->device_dispatch_table.GetDeviceProcAddr(device, pName);

    return result;
}

void DispatchGetDeviceQueue(VkDevice device, uint32_t queueFamilyIndex, uint32_t queueIndex, VkQueue* pQueue) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);

    layer_data->device_dispatch_table.GetDeviceQueue(device, queueFamilyIndex, queueIndex, pQueue);
}

VkResult DispatchQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(queue), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.QueueSubmit(queue, submitCount, pSubmits, fence);
    safe_VkSubmitInfo* local_pSubmits = nullptr;
    {
        if (pSubmits) {
            local_pSubmits = new safe_VkSubmitInfo[submitCount];
            for (uint32_t index0 = 0; index0 < submitCount; ++index0) {
                local_pSubmits[index0].initialize(&pSubmits[index0]);
                WrapPnextChainHandles(layer_data, local_pSubmits[index0].pNext);
                if (local_pSubmits[index0].pWaitSemaphores) {
                    for (uint32_t index1 = 0; index1 < local_pSubmits[index0].waitSemaphoreCount; ++index1) {
                        local_pSubmits[index0].pWaitSemaphores[index1] =
                            layer_data->Unwrap(local_pSubmits[index0].pWaitSemaphores[index1]);
                    }
                }
                if (local_pSubmits[index0].pSignalSemaphores) {
                    for (uint32_t index1 = 0; index1 < local_pSubmits[index0].signalSemaphoreCount; ++index1) {
                        local_pSubmits[index0].pSignalSemaphores[index1] =
                            layer_data->Unwrap(local_pSubmits[index0].pSignalSemaphores[index1]);
                    }
                }
            }
        }
        fence = layer_data->Unwrap(fence);
    }
    VkResult result = layer_data->device_dispatch_table.QueueSubmit(queue, submitCount, (const VkSubmitInfo*)local_pSubmits, fence);
    if (local_pSubmits) {
        delete[] local_pSubmits;
    }
    return result;
}

VkResult DispatchQueueWaitIdle(VkQueue queue) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(queue), layer_data_map);

    VkResult result = layer_data->device_dispatch_table.QueueWaitIdle(queue);

    return result;
}

VkResult DispatchDeviceWaitIdle(VkDevice device) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);

    VkResult result = layer_data->device_dispatch_table.DeviceWaitIdle(device);

    return result;
}

VkResult DispatchAllocateMemory(VkDevice device, const VkMemoryAllocateInfo* pAllocateInfo, const VkAllocationCallbacks* pAllocator,
                                VkDeviceMemory* pMemory) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.AllocateMemory(device, pAllocateInfo, pAllocator, pMemory);
    safe_VkMemoryAllocateInfo var_local_pAllocateInfo;
    safe_VkMemoryAllocateInfo* local_pAllocateInfo = nullptr;
    {
        if (pAllocateInfo) {
            local_pAllocateInfo = &var_local_pAllocateInfo;
            local_pAllocateInfo->initialize(pAllocateInfo);
            WrapPnextChainHandles(layer_data, local_pAllocateInfo->pNext);
        }
    }
    VkResult result = layer_data->device_dispatch_table.AllocateMemory(device, (const VkMemoryAllocateInfo*)local_pAllocateInfo,
                                                                       pAllocator, pMemory);
    if (VK_SUCCESS == result) {
        *pMemory = layer_data->WrapNew(*pMemory);
    }
    return result;
}

void DispatchFreeMemory(VkDevice device, VkDeviceMemory memory, const VkAllocationCallbacks* pAllocator) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.FreeMemory(device, memory, pAllocator);

    uint64_t memory_id = CastToUint64(memory);
    auto iter = unique_id_mapping.pop(memory_id);
    if (iter != unique_id_mapping.end()) {
        memory = (VkDeviceMemory)iter->second;
    } else {
        memory = (VkDeviceMemory)0;
    }
    layer_data->device_dispatch_table.FreeMemory(device, memory, pAllocator);
}

VkResult DispatchMapMemory(VkDevice device, VkDeviceMemory memory, VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags,
                           void** ppData) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.MapMemory(device, memory, offset, size, flags, ppData);
    { memory = layer_data->Unwrap(memory); }
    VkResult result = layer_data->device_dispatch_table.MapMemory(device, memory, offset, size, flags, ppData);

    return result;
}

void DispatchUnmapMemory(VkDevice device, VkDeviceMemory memory) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.UnmapMemory(device, memory);
    { memory = layer_data->Unwrap(memory); }
    layer_data->device_dispatch_table.UnmapMemory(device, memory);
}

VkResult DispatchFlushMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount, const VkMappedMemoryRange* pMemoryRanges) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.FlushMappedMemoryRanges(device, memoryRangeCount, pMemoryRanges);
    safe_VkMappedMemoryRange* local_pMemoryRanges = nullptr;
    {
        if (pMemoryRanges) {
            local_pMemoryRanges = new safe_VkMappedMemoryRange[memoryRangeCount];
            for (uint32_t index0 = 0; index0 < memoryRangeCount; ++index0) {
                local_pMemoryRanges[index0].initialize(&pMemoryRanges[index0]);

                if (pMemoryRanges[index0].memory) {
                    local_pMemoryRanges[index0].memory = layer_data->Unwrap(pMemoryRanges[index0].memory);
                }
            }
        }
    }
    VkResult result = layer_data->device_dispatch_table.FlushMappedMemoryRanges(device, memoryRangeCount,
                                                                                (const VkMappedMemoryRange*)local_pMemoryRanges);
    if (local_pMemoryRanges) {
        delete[] local_pMemoryRanges;
    }
    return result;
}

VkResult DispatchInvalidateMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount,
                                              const VkMappedMemoryRange* pMemoryRanges) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.InvalidateMappedMemoryRanges(device, memoryRangeCount, pMemoryRanges);
    safe_VkMappedMemoryRange* local_pMemoryRanges = nullptr;
    {
        if (pMemoryRanges) {
            local_pMemoryRanges = new safe_VkMappedMemoryRange[memoryRangeCount];
            for (uint32_t index0 = 0; index0 < memoryRangeCount; ++index0) {
                local_pMemoryRanges[index0].initialize(&pMemoryRanges[index0]);

                if (pMemoryRanges[index0].memory) {
                    local_pMemoryRanges[index0].memory = layer_data->Unwrap(pMemoryRanges[index0].memory);
                }
            }
        }
    }
    VkResult result = layer_data->device_dispatch_table.InvalidateMappedMemoryRanges(
        device, memoryRangeCount, (const VkMappedMemoryRange*)local_pMemoryRanges);
    if (local_pMemoryRanges) {
        delete[] local_pMemoryRanges;
    }
    return result;
}

void DispatchGetDeviceMemoryCommitment(VkDevice device, VkDeviceMemory memory, VkDeviceSize* pCommittedMemoryInBytes) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.GetDeviceMemoryCommitment(device, memory, pCommittedMemoryInBytes);
    { memory = layer_data->Unwrap(memory); }
    layer_data->device_dispatch_table.GetDeviceMemoryCommitment(device, memory, pCommittedMemoryInBytes);
}

VkResult DispatchBindBufferMemory(VkDevice device, VkBuffer buffer, VkDeviceMemory memory, VkDeviceSize memoryOffset) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.BindBufferMemory(device, buffer, memory, memoryOffset);
    {
        buffer = layer_data->Unwrap(buffer);
        memory = layer_data->Unwrap(memory);
    }
    VkResult result = layer_data->device_dispatch_table.BindBufferMemory(device, buffer, memory, memoryOffset);

    return result;
}

VkResult DispatchBindImageMemory(VkDevice device, VkImage image, VkDeviceMemory memory, VkDeviceSize memoryOffset) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.BindImageMemory(device, image, memory, memoryOffset);
    {
        image = layer_data->Unwrap(image);
        memory = layer_data->Unwrap(memory);
    }
    VkResult result = layer_data->device_dispatch_table.BindImageMemory(device, image, memory, memoryOffset);

    return result;
}

void DispatchGetBufferMemoryRequirements(VkDevice device, VkBuffer buffer, VkMemoryRequirements* pMemoryRequirements) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.GetBufferMemoryRequirements(device, buffer, pMemoryRequirements);
    { buffer = layer_data->Unwrap(buffer); }
    layer_data->device_dispatch_table.GetBufferMemoryRequirements(device, buffer, pMemoryRequirements);
}

void DispatchGetImageMemoryRequirements(VkDevice device, VkImage image, VkMemoryRequirements* pMemoryRequirements) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.GetImageMemoryRequirements(device, image, pMemoryRequirements);
    { image = layer_data->Unwrap(image); }
    layer_data->device_dispatch_table.GetImageMemoryRequirements(device, image, pMemoryRequirements);
}

void DispatchGetImageSparseMemoryRequirements(VkDevice device, VkImage image, uint32_t* pSparseMemoryRequirementCount,
                                              VkSparseImageMemoryRequirements* pSparseMemoryRequirements) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.GetImageSparseMemoryRequirements(device, image, pSparseMemoryRequirementCount,
                                                                                  pSparseMemoryRequirements);
    { image = layer_data->Unwrap(image); }
    layer_data->device_dispatch_table.GetImageSparseMemoryRequirements(device, image, pSparseMemoryRequirementCount,
                                                                       pSparseMemoryRequirements);
}

void DispatchGetPhysicalDeviceSparseImageFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type,
                                                          VkSampleCountFlagBits samples, VkImageUsageFlags usage,
                                                          VkImageTiling tiling, uint32_t* pPropertyCount,
                                                          VkSparseImageFormatProperties* pProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);

    layer_data->instance_dispatch_table.GetPhysicalDeviceSparseImageFormatProperties(physicalDevice, format, type, samples, usage,
                                                                                     tiling, pPropertyCount, pProperties);
}

VkResult DispatchQueueBindSparse(VkQueue queue, uint32_t bindInfoCount, const VkBindSparseInfo* pBindInfo, VkFence fence) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(queue), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.QueueBindSparse(queue, bindInfoCount, pBindInfo, fence);
    safe_VkBindSparseInfo* local_pBindInfo = nullptr;
    {
        if (pBindInfo) {
            local_pBindInfo = new safe_VkBindSparseInfo[bindInfoCount];
            for (uint32_t index0 = 0; index0 < bindInfoCount; ++index0) {
                local_pBindInfo[index0].initialize(&pBindInfo[index0]);
                WrapPnextChainHandles(layer_data, local_pBindInfo[index0].pNext);
                if (local_pBindInfo[index0].pWaitSemaphores) {
                    for (uint32_t index1 = 0; index1 < local_pBindInfo[index0].waitSemaphoreCount; ++index1) {
                        local_pBindInfo[index0].pWaitSemaphores[index1] =
                            layer_data->Unwrap(local_pBindInfo[index0].pWaitSemaphores[index1]);
                    }
                }
                if (local_pBindInfo[index0].pBufferBinds) {
                    for (uint32_t index1 = 0; index1 < local_pBindInfo[index0].bufferBindCount; ++index1) {
                        if (pBindInfo[index0].pBufferBinds[index1].buffer) {
                            local_pBindInfo[index0].pBufferBinds[index1].buffer =
                                layer_data->Unwrap(pBindInfo[index0].pBufferBinds[index1].buffer);
                        }
                        if (local_pBindInfo[index0].pBufferBinds[index1].pBinds) {
                            for (uint32_t index2 = 0; index2 < local_pBindInfo[index0].pBufferBinds[index1].bindCount; ++index2) {
                                if (pBindInfo[index0].pBufferBinds[index1].pBinds[index2].memory) {
                                    local_pBindInfo[index0].pBufferBinds[index1].pBinds[index2].memory =
                                        layer_data->Unwrap(pBindInfo[index0].pBufferBinds[index1].pBinds[index2].memory);
                                }
                            }
                        }
                    }
                }
                if (local_pBindInfo[index0].pImageOpaqueBinds) {
                    for (uint32_t index1 = 0; index1 < local_pBindInfo[index0].imageOpaqueBindCount; ++index1) {
                        if (pBindInfo[index0].pImageOpaqueBinds[index1].image) {
                            local_pBindInfo[index0].pImageOpaqueBinds[index1].image =
                                layer_data->Unwrap(pBindInfo[index0].pImageOpaqueBinds[index1].image);
                        }
                        if (local_pBindInfo[index0].pImageOpaqueBinds[index1].pBinds) {
                            for (uint32_t index2 = 0; index2 < local_pBindInfo[index0].pImageOpaqueBinds[index1].bindCount;
                                 ++index2) {
                                if (pBindInfo[index0].pImageOpaqueBinds[index1].pBinds[index2].memory) {
                                    local_pBindInfo[index0].pImageOpaqueBinds[index1].pBinds[index2].memory =
                                        layer_data->Unwrap(pBindInfo[index0].pImageOpaqueBinds[index1].pBinds[index2].memory);
                                }
                            }
                        }
                    }
                }
                if (local_pBindInfo[index0].pImageBinds) {
                    for (uint32_t index1 = 0; index1 < local_pBindInfo[index0].imageBindCount; ++index1) {
                        if (pBindInfo[index0].pImageBinds[index1].image) {
                            local_pBindInfo[index0].pImageBinds[index1].image =
                                layer_data->Unwrap(pBindInfo[index0].pImageBinds[index1].image);
                        }
                        if (local_pBindInfo[index0].pImageBinds[index1].pBinds) {
                            for (uint32_t index2 = 0; index2 < local_pBindInfo[index0].pImageBinds[index1].bindCount; ++index2) {
                                if (pBindInfo[index0].pImageBinds[index1].pBinds[index2].memory) {
                                    local_pBindInfo[index0].pImageBinds[index1].pBinds[index2].memory =
                                        layer_data->Unwrap(pBindInfo[index0].pImageBinds[index1].pBinds[index2].memory);
                                }
                            }
                        }
                    }
                }
                if (local_pBindInfo[index0].pSignalSemaphores) {
                    for (uint32_t index1 = 0; index1 < local_pBindInfo[index0].signalSemaphoreCount; ++index1) {
                        local_pBindInfo[index0].pSignalSemaphores[index1] =
                            layer_data->Unwrap(local_pBindInfo[index0].pSignalSemaphores[index1]);
                    }
                }
            }
        }
        fence = layer_data->Unwrap(fence);
    }
    VkResult result =
        layer_data->device_dispatch_table.QueueBindSparse(queue, bindInfoCount, (const VkBindSparseInfo*)local_pBindInfo, fence);
    if (local_pBindInfo) {
        delete[] local_pBindInfo;
    }
    return result;
}

VkResult DispatchCreateFence(VkDevice device, const VkFenceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                             VkFence* pFence) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CreateFence(device, pCreateInfo, pAllocator, pFence);

    VkResult result = layer_data->device_dispatch_table.CreateFence(device, pCreateInfo, pAllocator, pFence);
    if (VK_SUCCESS == result) {
        *pFence = layer_data->WrapNew(*pFence);
    }
    return result;
}

void DispatchDestroyFence(VkDevice device, VkFence fence, const VkAllocationCallbacks* pAllocator) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.DestroyFence(device, fence, pAllocator);

    uint64_t fence_id = CastToUint64(fence);
    auto iter = unique_id_mapping.pop(fence_id);
    if (iter != unique_id_mapping.end()) {
        fence = (VkFence)iter->second;
    } else {
        fence = (VkFence)0;
    }
    layer_data->device_dispatch_table.DestroyFence(device, fence, pAllocator);
}

VkResult DispatchResetFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.ResetFences(device, fenceCount, pFences);
    VkFence var_local_pFences[DISPATCH_MAX_STACK_ALLOCATIONS];
    VkFence* local_pFences = nullptr;
    {
        if (pFences) {
            local_pFences = fenceCount > DISPATCH_MAX_STACK_ALLOCATIONS ? new VkFence[fenceCount] : var_local_pFences;
            for (uint32_t index0 = 0; index0 < fenceCount; ++index0) {
                local_pFences[index0] = layer_data->Unwrap(pFences[index0]);
            }
        }
    }
    VkResult result = layer_data->device_dispatch_table.ResetFences(device, fenceCount, (const VkFence*)local_pFences);
    if (local_pFences != var_local_pFences) delete[] local_pFences;
    return result;
}

VkResult DispatchGetFenceStatus(VkDevice device, VkFence fence) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.GetFenceStatus(device, fence);
    { fence = layer_data->Unwrap(fence); }
    VkResult result = layer_data->device_dispatch_table.GetFenceStatus(device, fence);

    return result;
}

VkResult DispatchWaitForFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences, VkBool32 waitAll, uint64_t timeout) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.WaitForFences(device, fenceCount, pFences, waitAll, timeout);
    VkFence var_local_pFences[DISPATCH_MAX_STACK_ALLOCATIONS];
    VkFence* local_pFences = nullptr;
    {
        if (pFences) {
            local_pFences = fenceCount > DISPATCH_MAX_STACK_ALLOCATIONS ? new VkFence[fenceCount] : var_local_pFences;
            for (uint32_t index0 = 0; index0 < fenceCount; ++index0) {
                local_pFences[index0] = layer_data->Unwrap(pFences[index0]);
            }
        }
    }
    VkResult result =
        layer_data->device_dispatch_table.WaitForFences(device, fenceCount, (const VkFence*)local_pFences, waitAll, timeout);
    if (local_pFences != var_local_pFences) delete[] local_pFences;
    return result;
}

VkResult DispatchCreateSemaphore(VkDevice device, const VkSemaphoreCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                 VkSemaphore* pSemaphore) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CreateSemaphore(device, pCreateInfo, pAllocator, pSemaphore);

    VkResult result = layer_data->device_dispatch_table.CreateSemaphore(device, pCreateInfo, pAllocator, pSemaphore);
    if (VK_SUCCESS == result) {
        *pSemaphore = layer_data->WrapNew(*pSemaphore);
    }
    return result;
}

void DispatchDestroySemaphore(VkDevice device, VkSemaphore semaphore, const VkAllocationCallbacks* pAllocator) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.DestroySemaphore(device, semaphore, pAllocator);

    uint64_t semaphore_id = CastToUint64(semaphore);
    auto iter = unique_id_mapping.pop(semaphore_id);
    if (iter != unique_id_mapping.end()) {
        semaphore = (VkSemaphore)iter->second;
    } else {
        semaphore = (VkSemaphore)0;
    }
    layer_data->device_dispatch_table.DestroySemaphore(device, semaphore, pAllocator);
}

VkResult DispatchCreateEvent(VkDevice device, const VkEventCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                             VkEvent* pEvent) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CreateEvent(device, pCreateInfo, pAllocator, pEvent);

    VkResult result = layer_data->device_dispatch_table.CreateEvent(device, pCreateInfo, pAllocator, pEvent);
    if (VK_SUCCESS == result) {
        *pEvent = layer_data->WrapNew(*pEvent);
    }
    return result;
}

void DispatchDestroyEvent(VkDevice device, VkEvent event, const VkAllocationCallbacks* pAllocator) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.DestroyEvent(device, event, pAllocator);

    uint64_t event_id = CastToUint64(event);
    auto iter = unique_id_mapping.pop(event_id);
    if (iter != unique_id_mapping.end()) {
        event = (VkEvent)iter->second;
    } else {
        event = (VkEvent)0;
    }
    layer_data->device_dispatch_table.DestroyEvent(device, event, pAllocator);
}

VkResult DispatchGetEventStatus(VkDevice device, VkEvent event) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.GetEventStatus(device, event);
    { event = layer_data->Unwrap(event); }
    VkResult result = layer_data->device_dispatch_table.GetEventStatus(device, event);

    return result;
}

VkResult DispatchSetEvent(VkDevice device, VkEvent event) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.SetEvent(device, event);
    { event = layer_data->Unwrap(event); }
    VkResult result = layer_data->device_dispatch_table.SetEvent(device, event);

    return result;
}

VkResult DispatchResetEvent(VkDevice device, VkEvent event) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.ResetEvent(device, event);
    { event = layer_data->Unwrap(event); }
    VkResult result = layer_data->device_dispatch_table.ResetEvent(device, event);

    return result;
}

VkResult DispatchCreateQueryPool(VkDevice device, const VkQueryPoolCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                 VkQueryPool* pQueryPool) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CreateQueryPool(device, pCreateInfo, pAllocator, pQueryPool);

    VkResult result = layer_data->device_dispatch_table.CreateQueryPool(device, pCreateInfo, pAllocator, pQueryPool);
    if (VK_SUCCESS == result) {
        *pQueryPool = layer_data->WrapNew(*pQueryPool);
    }
    return result;
}

void DispatchDestroyQueryPool(VkDevice device, VkQueryPool queryPool, const VkAllocationCallbacks* pAllocator) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.DestroyQueryPool(device, queryPool, pAllocator);

    uint64_t queryPool_id = CastToUint64(queryPool);
    auto iter = unique_id_mapping.pop(queryPool_id);
    if (iter != unique_id_mapping.end()) {
        queryPool = (VkQueryPool)iter->second;
    } else {
        queryPool = (VkQueryPool)0;
    }
    layer_data->device_dispatch_table.DestroyQueryPool(device, queryPool, pAllocator);
}

VkResult DispatchGetQueryPoolResults(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount,
                                     size_t dataSize, void* pData, VkDeviceSize stride, VkQueryResultFlags flags) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.GetQueryPoolResults(device, queryPool, firstQuery, queryCount, dataSize, pData,
                                                                     stride, flags);
    { queryPool = layer_data->Unwrap(queryPool); }
    VkResult result = layer_data->device_dispatch_table.GetQueryPoolResults(device, queryPool, firstQuery, queryCount, dataSize,
                                                                            pData, stride, flags);

    return result;
}

VkResult DispatchCreateBuffer(VkDevice device, const VkBufferCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                              VkBuffer* pBuffer) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CreateBuffer(device, pCreateInfo, pAllocator, pBuffer);
    safe_VkBufferCreateInfo var_local_pCreateInfo;
    safe_VkBufferCreateInfo* local_pCreateInfo = nullptr;
    {
        if (pCreateInfo) {
            local_pCreateInfo = &var_local_pCreateInfo;
            local_pCreateInfo->initialize(pCreateInfo);
            WrapPnextChainHandles(layer_data, local_pCreateInfo->pNext);
        }
    }
    VkResult result =
        layer_data->device_dispatch_table.CreateBuffer(device, (const VkBufferCreateInfo*)local_pCreateInfo, pAllocator, pBuffer);
    if (VK_SUCCESS == result) {
        *pBuffer = layer_data->WrapNew(*pBuffer);
    }
    return result;
}

void DispatchDestroyBuffer(VkDevice device, VkBuffer buffer, const VkAllocationCallbacks* pAllocator) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.DestroyBuffer(device, buffer, pAllocator);

    uint64_t buffer_id = CastToUint64(buffer);
    auto iter = unique_id_mapping.pop(buffer_id);
    if (iter != unique_id_mapping.end()) {
        buffer = (VkBuffer)iter->second;
    } else {
        buffer = (VkBuffer)0;
    }
    layer_data->device_dispatch_table.DestroyBuffer(device, buffer, pAllocator);
}

VkResult DispatchCreateBufferView(VkDevice device, const VkBufferViewCreateInfo* pCreateInfo,
                                  const VkAllocationCallbacks* pAllocator, VkBufferView* pView) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CreateBufferView(device, pCreateInfo, pAllocator, pView);
    safe_VkBufferViewCreateInfo var_local_pCreateInfo;
    safe_VkBufferViewCreateInfo* local_pCreateInfo = nullptr;
    {
        if (pCreateInfo) {
            local_pCreateInfo = &var_local_pCreateInfo;
            local_pCreateInfo->initialize(pCreateInfo);

            if (pCreateInfo->buffer) {
                local_pCreateInfo->buffer = layer_data->Unwrap(pCreateInfo->buffer);
            }
        }
    }
    VkResult result = layer_data->device_dispatch_table.CreateBufferView(device, (const VkBufferViewCreateInfo*)local_pCreateInfo,
                                                                         pAllocator, pView);
    if (VK_SUCCESS == result) {
        *pView = layer_data->WrapNew(*pView);
    }
    return result;
}

void DispatchDestroyBufferView(VkDevice device, VkBufferView bufferView, const VkAllocationCallbacks* pAllocator) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.DestroyBufferView(device, bufferView, pAllocator);

    uint64_t bufferView_id = CastToUint64(bufferView);
    auto iter = unique_id_mapping.pop(bufferView_id);
    if (iter != unique_id_mapping.end()) {
        bufferView = (VkBufferView)iter->second;
    } else {
        bufferView = (VkBufferView)0;
    }
    layer_data->device_dispatch_table.DestroyBufferView(device, bufferView, pAllocator);
}

VkResult DispatchCreateImage(VkDevice device, const VkImageCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                             VkImage* pImage) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CreateImage(device, pCreateInfo, pAllocator, pImage);
    safe_VkImageCreateInfo var_local_pCreateInfo;
    safe_VkImageCreateInfo* local_pCreateInfo = nullptr;
    {
        if (pCreateInfo) {
            local_pCreateInfo = &var_local_pCreateInfo;
            local_pCreateInfo->initialize(pCreateInfo);
            WrapPnextChainHandles(layer_data, local_pCreateInfo->pNext);
        }
    }
    VkResult result =
        layer_data->device_dispatch_table.CreateImage(device, (const VkImageCreateInfo*)local_pCreateInfo, pAllocator, pImage);
    if (VK_SUCCESS == result) {
        *pImage = layer_data->WrapNew(*pImage);
    }
    return result;
}

void DispatchDestroyImage(VkDevice device, VkImage image, const VkAllocationCallbacks* pAllocator) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.DestroyImage(device, image, pAllocator);

    uint64_t image_id = CastToUint64(image);
    auto iter = unique_id_mapping.pop(image_id);
    if (iter != unique_id_mapping.end()) {
        image = (VkImage)iter->second;
    } else {
        image = (VkImage)0;
    }
    layer_data->device_dispatch_table.DestroyImage(device, image, pAllocator);
}

void DispatchGetImageSubresourceLayout(VkDevice device, VkImage image, const VkImageSubresource* pSubresource,
                                       VkSubresourceLayout* pLayout) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.GetImageSubresourceLayout(device, image, pSubresource, pLayout);
    { image = layer_data->Unwrap(image); }
    layer_data->device_dispatch_table.GetImageSubresourceLayout(device, image, pSubresource, pLayout);
}

VkResult DispatchCreateImageView(VkDevice device, const VkImageViewCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                 VkImageView* pView) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CreateImageView(device, pCreateInfo, pAllocator, pView);
    safe_VkImageViewCreateInfo var_local_pCreateInfo;
    safe_VkImageViewCreateInfo* local_pCreateInfo = nullptr;
    {
        if (pCreateInfo) {
            local_pCreateInfo = &var_local_pCreateInfo;
            local_pCreateInfo->initialize(pCreateInfo);

            if (pCreateInfo->image) {
                local_pCreateInfo->image = layer_data->Unwrap(pCreateInfo->image);
            }
            WrapPnextChainHandles(layer_data, local_pCreateInfo->pNext);
        }
    }
    VkResult result = layer_data->device_dispatch_table.CreateImageView(device, (const VkImageViewCreateInfo*)local_pCreateInfo,
                                                                        pAllocator, pView);
    if (VK_SUCCESS == result) {
        *pView = layer_data->WrapNew(*pView);
    }
    return result;
}

void DispatchDestroyImageView(VkDevice device, VkImageView imageView, const VkAllocationCallbacks* pAllocator) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.DestroyImageView(device, imageView, pAllocator);

    uint64_t imageView_id = CastToUint64(imageView);
    auto iter = unique_id_mapping.pop(imageView_id);
    if (iter != unique_id_mapping.end()) {
        imageView = (VkImageView)iter->second;
    } else {
        imageView = (VkImageView)0;
    }
    layer_data->device_dispatch_table.DestroyImageView(device, imageView, pAllocator);
}

VkResult DispatchCreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo,
                                    const VkAllocationCallbacks* pAllocator, VkShaderModule* pShaderModule) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CreateShaderModule(device, pCreateInfo, pAllocator, pShaderModule);
    safe_VkShaderModuleCreateInfo var_local_pCreateInfo;
    safe_VkShaderModuleCreateInfo* local_pCreateInfo = nullptr;
    {
        if (pCreateInfo) {
            local_pCreateInfo = &var_local_pCreateInfo;
            local_pCreateInfo->initialize(pCreateInfo);
            WrapPnextChainHandles(layer_data, local_pCreateInfo->pNext);
        }
    }
    VkResult result = layer_data->device_dispatch_table.CreateShaderModule(
        device, (const VkShaderModuleCreateInfo*)local_pCreateInfo, pAllocator, pShaderModule);
    if (VK_SUCCESS == result) {
        *pShaderModule = layer_data->WrapNew(*pShaderModule);
    }
    return result;
}

void DispatchDestroyShaderModule(VkDevice device, VkShaderModule shaderModule, const VkAllocationCallbacks* pAllocator) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.DestroyShaderModule(device, shaderModule, pAllocator);

    uint64_t shaderModule_id = CastToUint64(shaderModule);
    auto iter = unique_id_mapping.pop(shaderModule_id);
    if (iter != unique_id_mapping.end()) {
        shaderModule = (VkShaderModule)iter->second;
    } else {
        shaderModule = (VkShaderModule)0;
    }
    layer_data->device_dispatch_table.DestroyShaderModule(device, shaderModule, pAllocator);
}

VkResult DispatchCreatePipelineCache(VkDevice device, const VkPipelineCacheCreateInfo* pCreateInfo,
                                     const VkAllocationCallbacks* pAllocator, VkPipelineCache* pPipelineCache) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.CreatePipelineCache(device, pCreateInfo, pAllocator, pPipelineCache);

    VkResult result = layer_data->device_dispatch_table.CreatePipelineCache(device, pCreateInfo, pAllocator, pPipelineCache);
    if (VK_SUCCESS == result) {
        *pPipelineCache = layer_data->WrapNew(*pPipelineCache);
    }
    return result;
}

void DispatchDestroyPipelineCache(VkDevice device, VkPipelineCache pipelineCache, const VkAllocationCallbacks* pAllocator) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.DestroyPipelineCache(device, pipelineCache, pAllocator);

    uint64_t pipelineCache_id = CastToUint64(pipelineCache);
    auto iter = unique_id_mapping.pop(pipelineCache_id);
    if (iter != unique_id_mapping.end()) {
        pipelineCache = (VkPipelineCache)iter->second;
    } else {
        pipelineCache = (VkPipelineCache)0;
    }
    layer_data->device_dispatch_table.DestroyPipelineCache(device, pipelineCache, pAllocator);
}

VkResult DispatchGetPipelineCacheData(VkDevice device, VkPipelineCache pipelineCache, size_t* pDataSize, void* pData) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.GetPipelineCacheData(device, pipelineCache, pDataSize, pData);
    { pipelineCache = layer_data->Unwrap(pipelineCache); }
    VkResult result = layer_data->device_dispatch_table.GetPipelineCacheData(device, pipelineCache, pDataSize, pData);

    return result;
}

VkResult DispatchMergePipelineCaches(VkDevice device, VkPipelineCache dstCache, uint32_t srcCacheCount,
                                     const VkPipelineCache* pSrcCaches) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.MergePipelineCaches(device, dstCache, srcCacheCount, pSrcCaches);
    VkPipelineCache var_local_pSrcCaches[DISPATCH_MAX_STACK_ALLOCATIONS];
    VkPipelineCache* local_pSrcCaches = nullptr;
    {
        dstCache = layer_data->Unwrap(dstCache);
        if (pSrcCaches) {
            local_pSrcCaches =
                srcCacheCount > DISPATCH_MAX_STACK_ALLOCATIONS ? new VkPipelineCache[srcCacheCount] : var_local_pSrcCaches;
            for (uint32_t index0 = 0; index0 < srcCacheCount; ++index0) {
                local_pSrcCaches[index0] = layer_data->Unwrap(pSrcCaches[index0]);
            }
        }
    }
    VkResult result = layer_data->device_dispatch_table.MergePipelineCaches(device, dstCache, srcCacheCount,
                                                                            (const VkPipelineCache*)local_pSrcCaches);
    if (local_pSrcCaches != var_local_pSrcCaches) delete[] local_pSrcCaches;
    return result;
}

void DispatchDestroyPipeline(VkDevice device, VkPipeline pipeline, const VkAllocationCallbacks* pAllocator) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.DestroyPipeline(device, pipeline, pAllocator);

    uint64_t pipeline_id = CastToUint64(pipeline);
    auto iter = unique_id_mapping.pop(pipeline_id);
    if (iter != unique_id_mapping.end()) {
        pipeline = (VkPipeline)iter->second;
    } else {
        pipeline = (VkPipeline)0;
    }
    layer_data->device_dispatch_table.DestroyPipeline(device, pipeline, pAllocator);
}

VkResult DispatchCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo* pCreateInfo,
                                      const VkAllocationCallbacks* pAllocator, VkPipelineLayout* pPipelineLayout) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.CreatePipelineLayout(device, pCreateInfo, pAllocator, pPipelineLayout);
    safe_VkPipelineLayoutCreateInfo var_local_pCreateInfo;
    safe_VkPipelineLayoutCreateInfo* local_pCreateInfo = nullptr;
    {
        if (pCreateInfo) {
            local_pCreateInfo = &var_local_pCreateInfo;
            local_pCreateInfo->initialize(pCreateInfo);
            if (local_pCreateInfo->pSetLayouts) {
                for (uint32_t index1 = 0; index1 < local_pCreateInfo->setLayoutCount; ++index1) {
                    local_pCreateInfo->pSetLayouts[index1] = layer_data->Unwrap(local_pCreateInfo->pSetLayouts[index1]);
                }
            }
        }
    }
    VkResult result = layer_data->device_dispatch_table.CreatePipelineLayout(
        device, (const VkPipelineLayoutCreateInfo*)local_pCreateInfo, pAllocator, pPipelineLayout);
    if (VK_SUCCESS == result) {
        *pPipelineLayout = layer_data->WrapNew(*pPipelineLayout);
    }
    return result;
}

void DispatchDestroyPipelineLayout(VkDevice device, VkPipelineLayout pipelineLayout, const VkAllocationCallbacks* pAllocator) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.DestroyPipelineLayout(device, pipelineLayout, pAllocator);

    uint64_t pipelineLayout_id = CastToUint64(pipelineLayout);
    auto iter = unique_id_mapping.pop(pipelineLayout_id);
    if (iter != unique_id_mapping.end()) {
        pipelineLayout = (VkPipelineLayout)iter->second;
    } else {
        pipelineLayout = (VkPipelineLayout)0;
    }
    layer_data->device_dispatch_table.DestroyPipelineLayout(device, pipelineLayout, pAllocator);
}

VkResult DispatchCreateSampler(VkDevice device, const VkSamplerCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                               VkSampler* pSampler) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CreateSampler(device, pCreateInfo, pAllocator, pSampler);
    safe_VkSamplerCreateInfo var_local_pCreateInfo;
    safe_VkSamplerCreateInfo* local_pCreateInfo = nullptr;
    {
        if (pCreateInfo) {
            local_pCreateInfo = &var_local_pCreateInfo;
            local_pCreateInfo->initialize(pCreateInfo);
            WrapPnextChainHandles(layer_data, local_pCreateInfo->pNext);
        }
    }
    VkResult result = layer_data->device_dispatch_table.CreateSampler(device, (const VkSamplerCreateInfo*)local_pCreateInfo,
                                                                      pAllocator, pSampler);
    if (VK_SUCCESS == result) {
        *pSampler = layer_data->WrapNew(*pSampler);
    }
    return result;
}

void DispatchDestroySampler(VkDevice device, VkSampler sampler, const VkAllocationCallbacks* pAllocator) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.DestroySampler(device, sampler, pAllocator);

    uint64_t sampler_id = CastToUint64(sampler);
    auto iter = unique_id_mapping.pop(sampler_id);
    if (iter != unique_id_mapping.end()) {
        sampler = (VkSampler)iter->second;
    } else {
        sampler = (VkSampler)0;
    }
    layer_data->device_dispatch_table.DestroySampler(device, sampler, pAllocator);
}

VkResult DispatchCreateDescriptorSetLayout(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo,
                                           const VkAllocationCallbacks* pAllocator, VkDescriptorSetLayout* pSetLayout) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.CreateDescriptorSetLayout(device, pCreateInfo, pAllocator, pSetLayout);
    safe_VkDescriptorSetLayoutCreateInfo var_local_pCreateInfo;
    safe_VkDescriptorSetLayoutCreateInfo* local_pCreateInfo = nullptr;
    {
        if (pCreateInfo) {
            local_pCreateInfo = &var_local_pCreateInfo;
            local_pCreateInfo->initialize(pCreateInfo);
            if (local_pCreateInfo->pBindings) {
                for (uint32_t index1 = 0; index1 < local_pCreateInfo->bindingCount; ++index1) {
                    if (local_pCreateInfo->pBindings[index1].pImmutableSamplers) {
                        for (uint32_t index2 = 0; index2 < local_pCreateInfo->pBindings[index1].descriptorCount; ++index2) {
                            local_pCreateInfo->pBindings[index1].pImmutableSamplers[index2] =
                                layer_data->Unwrap(local_pCreateInfo->pBindings[index1].pImmutableSamplers[index2]);
                        }
                    }
                }
            }
        }
    }
    VkResult result = layer_data->device_dispatch_table.CreateDescriptorSetLayout(
        device, (const VkDescriptorSetLayoutCreateInfo*)local_pCreateInfo, pAllocator, pSetLayout);
    if (VK_SUCCESS == result) {
        *pSetLayout = layer_data->WrapNew(*pSetLayout);
    }
    return result;
}

void DispatchDestroyDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout descriptorSetLayout,
                                        const VkAllocationCallbacks* pAllocator) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.DestroyDescriptorSetLayout(device, descriptorSetLayout, pAllocator);

    uint64_t descriptorSetLayout_id = CastToUint64(descriptorSetLayout);
    auto iter = unique_id_mapping.pop(descriptorSetLayout_id);
    if (iter != unique_id_mapping.end()) {
        descriptorSetLayout = (VkDescriptorSetLayout)iter->second;
    } else {
        descriptorSetLayout = (VkDescriptorSetLayout)0;
    }
    layer_data->device_dispatch_table.DestroyDescriptorSetLayout(device, descriptorSetLayout, pAllocator);
}

VkResult DispatchCreateDescriptorPool(VkDevice device, const VkDescriptorPoolCreateInfo* pCreateInfo,
                                      const VkAllocationCallbacks* pAllocator, VkDescriptorPool* pDescriptorPool) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.CreateDescriptorPool(device, pCreateInfo, pAllocator, pDescriptorPool);

    VkResult result = layer_data->device_dispatch_table.CreateDescriptorPool(device, pCreateInfo, pAllocator, pDescriptorPool);
    if (VK_SUCCESS == result) {
        *pDescriptorPool = layer_data->WrapNew(*pDescriptorPool);
    }
    return result;
}

void DispatchUpdateDescriptorSets(VkDevice device, uint32_t descriptorWriteCount, const VkWriteDescriptorSet* pDescriptorWrites,
                                  uint32_t descriptorCopyCount, const VkCopyDescriptorSet* pDescriptorCopies) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.UpdateDescriptorSets(device, descriptorWriteCount, pDescriptorWrites,
                                                                      descriptorCopyCount, pDescriptorCopies);
    safe_VkWriteDescriptorSet* local_pDescriptorWrites = nullptr;
    safe_VkCopyDescriptorSet* local_pDescriptorCopies = nullptr;
    {
        if (pDescriptorWrites) {
            local_pDescriptorWrites = new safe_VkWriteDescriptorSet[descriptorWriteCount];
            for (uint32_t index0 = 0; index0 < descriptorWriteCount; ++index0) {
                local_pDescriptorWrites[index0].initialize(&pDescriptorWrites[index0]);
                WrapPnextChainHandles(layer_data, local_pDescriptorWrites[index0].pNext);

                if (pDescriptorWrites[index0].dstSet) {
                    local_pDescriptorWrites[index0].dstSet = layer_data->Unwrap(pDescriptorWrites[index0].dstSet);
                }
                if (local_pDescriptorWrites[index0].pImageInfo) {
                    for (uint32_t index1 = 0; index1 < local_pDescriptorWrites[index0].descriptorCount; ++index1) {
                        if (pDescriptorWrites[index0].pImageInfo[index1].sampler) {
                            local_pDescriptorWrites[index0].pImageInfo[index1].sampler =
                                layer_data->Unwrap(pDescriptorWrites[index0].pImageInfo[index1].sampler);
                        }
                        if (pDescriptorWrites[index0].pImageInfo[index1].imageView) {
                            local_pDescriptorWrites[index0].pImageInfo[index1].imageView =
                                layer_data->Unwrap(pDescriptorWrites[index0].pImageInfo[index1].imageView);
                        }
                    }
                }
                if (local_pDescriptorWrites[index0].pBufferInfo) {
                    for (uint32_t index1 = 0; index1 < local_pDescriptorWrites[index0].descriptorCount; ++index1) {
                        if (pDescriptorWrites[index0].pBufferInfo[index1].buffer) {
                            local_pDescriptorWrites[index0].pBufferInfo[index1].buffer =
                                layer_data->Unwrap(pDescriptorWrites[index0].pBufferInfo[index1].buffer);
                        }
                    }
                }
                if (local_pDescriptorWrites[index0].pTexelBufferView) {
                    for (uint32_t index1 = 0; index1 < local_pDescriptorWrites[index0].descriptorCount; ++index1) {
                        local_pDescriptorWrites[index0].pTexelBufferView[index1] =
                            layer_data->Unwrap(local_pDescriptorWrites[index0].pTexelBufferView[index1]);
                    }
                }
            }
        }
        if (pDescriptorCopies) {
            local_pDescriptorCopies = new safe_VkCopyDescriptorSet[descriptorCopyCount];
            for (uint32_t index0 = 0; index0 < descriptorCopyCount; ++index0) {
                local_pDescriptorCopies[index0].initialize(&pDescriptorCopies[index0]);

                if (pDescriptorCopies[index0].srcSet) {
                    local_pDescriptorCopies[index0].srcSet = layer_data->Unwrap(pDescriptorCopies[index0].srcSet);
                }
                if (pDescriptorCopies[index0].dstSet) {
                    local_pDescriptorCopies[index0].dstSet = layer_data->Unwrap(pDescriptorCopies[index0].dstSet);
                }
            }
        }
    }
    layer_data->device_dispatch_table.UpdateDescriptorSets(
        device, descriptorWriteCount, (const VkWriteDescriptorSet*)local_pDescriptorWrites, descriptorCopyCount,
        (const VkCopyDescriptorSet*)local_pDescriptorCopies);
    if (local_pDescriptorWrites) {
        delete[] local_pDescriptorWrites;
    }
    if (local_pDescriptorCopies) {
        delete[] local_pDescriptorCopies;
    }
}

VkResult DispatchCreateFramebuffer(VkDevice device, const VkFramebufferCreateInfo* pCreateInfo,
                                   const VkAllocationCallbacks* pAllocator, VkFramebuffer* pFramebuffer) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CreateFramebuffer(device, pCreateInfo, pAllocator, pFramebuffer);
    safe_VkFramebufferCreateInfo var_local_pCreateInfo;
    safe_VkFramebufferCreateInfo* local_pCreateInfo = nullptr;
    {
        if (pCreateInfo) {
            local_pCreateInfo = &var_local_pCreateInfo;
            local_pCreateInfo->initialize(pCreateInfo);

            if (pCreateInfo->renderPass) {
                local_pCreateInfo->renderPass = layer_data->Unwrap(pCreateInfo->renderPass);
            }
            if (local_pCreateInfo->pAttachments) {
                for (uint32_t index1 = 0; index1 < local_pCreateInfo->attachmentCount; ++index1) {
                    local_pCreateInfo->pAttachments[index1] = layer_data->Unwrap(local_pCreateInfo->pAttachments[index1]);
                }
            }
        }
    }
    VkResult result = layer_data->device_dispatch_table.CreateFramebuffer(device, (const VkFramebufferCreateInfo*)local_pCreateInfo,
                                                                          pAllocator, pFramebuffer);
    if (VK_SUCCESS == result) {
        *pFramebuffer = layer_data->WrapNew(*pFramebuffer);
    }
    return result;
}

void DispatchDestroyFramebuffer(VkDevice device, VkFramebuffer framebuffer, const VkAllocationCallbacks* pAllocator) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.DestroyFramebuffer(device, framebuffer, pAllocator);

    uint64_t framebuffer_id = CastToUint64(framebuffer);
    auto iter = unique_id_mapping.pop(framebuffer_id);
    if (iter != unique_id_mapping.end()) {
        framebuffer = (VkFramebuffer)iter->second;
    } else {
        framebuffer = (VkFramebuffer)0;
    }
    layer_data->device_dispatch_table.DestroyFramebuffer(device, framebuffer, pAllocator);
}

void DispatchGetRenderAreaGranularity(VkDevice device, VkRenderPass renderPass, VkExtent2D* pGranularity) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.GetRenderAreaGranularity(device, renderPass, pGranularity);
    { renderPass = layer_data->Unwrap(renderPass); }
    layer_data->device_dispatch_table.GetRenderAreaGranularity(device, renderPass, pGranularity);
}

VkResult DispatchCreateCommandPool(VkDevice device, const VkCommandPoolCreateInfo* pCreateInfo,
                                   const VkAllocationCallbacks* pAllocator, VkCommandPool* pCommandPool) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CreateCommandPool(device, pCreateInfo, pAllocator, pCommandPool);

    VkResult result = layer_data->device_dispatch_table.CreateCommandPool(device, pCreateInfo, pAllocator, pCommandPool);
    if (VK_SUCCESS == result) {
        *pCommandPool = layer_data->WrapNew(*pCommandPool);
    }
    return result;
}

VkResult DispatchResetCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolResetFlags flags) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.ResetCommandPool(device, commandPool, flags);
    { commandPool = layer_data->Unwrap(commandPool); }
    VkResult result = layer_data->device_dispatch_table.ResetCommandPool(device, commandPool, flags);

    return result;
}

VkResult DispatchEndCommandBuffer(VkCommandBuffer commandBuffer) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    VkResult result = layer_data->device_dispatch_table.EndCommandBuffer(commandBuffer);

    return result;
}

VkResult DispatchResetCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferResetFlags flags) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    VkResult result = layer_data->device_dispatch_table.ResetCommandBuffer(commandBuffer, flags);

    return result;
}

void DispatchCmdBindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CmdBindPipeline(commandBuffer, pipelineBindPoint, pipeline);
    { pipeline = layer_data->Unwrap(pipeline); }
    layer_data->device_dispatch_table.CmdBindPipeline(commandBuffer, pipelineBindPoint, pipeline);
}

void DispatchCmdSetViewport(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount,
                            const VkViewport* pViewports) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdSetViewport(commandBuffer, firstViewport, viewportCount, pViewports);
}

void DispatchCmdSetScissor(VkCommandBuffer commandBuffer, uint32_t firstScissor, uint32_t scissorCount, const VkRect2D* pScissors) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdSetScissor(commandBuffer, firstScissor, scissorCount, pScissors);
}

void DispatchCmdSetLineWidth(VkCommandBuffer commandBuffer, float lineWidth) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdSetLineWidth(commandBuffer, lineWidth);
}

void DispatchCmdSetDepthBias(VkCommandBuffer commandBuffer, float depthBiasConstantFactor, float depthBiasClamp,
                             float depthBiasSlopeFactor) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdSetDepthBias(commandBuffer, depthBiasConstantFactor, depthBiasClamp, depthBiasSlopeFactor);
}

void DispatchCmdSetBlendConstants(VkCommandBuffer commandBuffer, const float blendConstants[4]) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdSetBlendConstants(commandBuffer, blendConstants);
}

void DispatchCmdSetDepthBounds(VkCommandBuffer commandBuffer, float minDepthBounds, float maxDepthBounds) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdSetDepthBounds(commandBuffer, minDepthBounds, maxDepthBounds);
}

void DispatchCmdSetStencilCompareMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t compareMask) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdSetStencilCompareMask(commandBuffer, faceMask, compareMask);
}

void DispatchCmdSetStencilWriteMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t writeMask) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdSetStencilWriteMask(commandBuffer, faceMask, writeMask);
}

void DispatchCmdSetStencilReference(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t reference) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdSetStencilReference(commandBuffer, faceMask, reference);
}

void DispatchCmdBindDescriptorSets(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout,
                                   uint32_t firstSet, uint32_t descriptorSetCount, const VkDescriptorSet* pDescriptorSets,
                                   uint32_t dynamicOffsetCount, const uint32_t* pDynamicOffsets) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.CmdBindDescriptorSets(commandBuffer, pipelineBindPoint, layout, firstSet,
                                                                       descriptorSetCount, pDescriptorSets, dynamicOffsetCount,
                                                                       pDynamicOffsets);
    VkDescriptorSet var_local_pDescriptorSets[DISPATCH_MAX_STACK_ALLOCATIONS];
    VkDescriptorSet* local_pDescriptorSets = nullptr;
    {
        layout = layer_data->Unwrap(layout);
        if (pDescriptorSets) {
            local_pDescriptorSets = descriptorSetCount > DISPATCH_MAX_STACK_ALLOCATIONS ? new VkDescriptorSet[descriptorSetCount]
                                                                                        : var_local_pDescriptorSets;
            for (uint32_t index0 = 0; index0 < descriptorSetCount; ++index0) {
                local_pDescriptorSets[index0] = layer_data->Unwrap(pDescriptorSets[index0]);
            }
        }
    }
    layer_data->device_dispatch_table.CmdBindDescriptorSets(commandBuffer, pipelineBindPoint, layout, firstSet, descriptorSetCount,
                                                            (const VkDescriptorSet*)local_pDescriptorSets, dynamicOffsetCount,
                                                            pDynamicOffsets);
    if (local_pDescriptorSets != var_local_pDescriptorSets) delete[] local_pDescriptorSets;
}

void DispatchCmdBindIndexBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkIndexType indexType) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CmdBindIndexBuffer(commandBuffer, buffer, offset, indexType);
    { buffer = layer_data->Unwrap(buffer); }
    layer_data->device_dispatch_table.CmdBindIndexBuffer(commandBuffer, buffer, offset, indexType);
}

void DispatchCmdBindVertexBuffers(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount,
                                  const VkBuffer* pBuffers, const VkDeviceSize* pOffsets) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.CmdBindVertexBuffers(commandBuffer, firstBinding, bindingCount, pBuffers,
                                                                      pOffsets);
    VkBuffer var_local_pBuffers[DISPATCH_MAX_STACK_ALLOCATIONS];
    VkBuffer* local_pBuffers = nullptr;
    {
        if (pBuffers) {
            local_pBuffers = bindingCount > DISPATCH_MAX_STACK_ALLOCATIONS ? new VkBuffer[bindingCount] : var_local_pBuffers;
            for (uint32_t index0 = 0; index0 < bindingCount; ++index0) {
                local_pBuffers[index0] = layer_data->Unwrap(pBuffers[index0]);
            }
        }
    }
    layer_data->device_dispatch_table.CmdBindVertexBuffers(commandBuffer, firstBinding, bindingCount,
                                                           (const VkBuffer*)local_pBuffers, pOffsets);
    if (local_pBuffers != var_local_pBuffers) delete[] local_pBuffers;
}

void DispatchCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex,
                     uint32_t firstInstance) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdDraw(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
}

void DispatchCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex,
                            int32_t vertexOffset, uint32_t firstInstance) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdDrawIndexed(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset,
                                                     firstInstance);
}

void DispatchCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount,
                             uint32_t stride) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CmdDrawIndirect(commandBuffer, buffer, offset, drawCount, stride);
    { buffer = layer_data->Unwrap(buffer); }
    layer_data->device_dispatch_table.CmdDrawIndirect(commandBuffer, buffer, offset, drawCount, stride);
}

void DispatchCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount,
                                    uint32_t stride) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.CmdDrawIndexedIndirect(commandBuffer, buffer, offset, drawCount, stride);
    { buffer = layer_data->Unwrap(buffer); }
    layer_data->device_dispatch_table.CmdDrawIndexedIndirect(commandBuffer, buffer, offset, drawCount, stride);
}

void DispatchCmdDispatch(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdDispatch(commandBuffer, groupCountX, groupCountY, groupCountZ);
}

void DispatchCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CmdDispatchIndirect(commandBuffer, buffer, offset);
    { buffer = layer_data->Unwrap(buffer); }
    layer_data->device_dispatch_table.CmdDispatchIndirect(commandBuffer, buffer, offset);
}

void DispatchCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t regionCount,
                           const VkBufferCopy* pRegions) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.CmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, regionCount, pRegions);
    {
        srcBuffer = layer_data->Unwrap(srcBuffer);
        dstBuffer = layer_data->Unwrap(dstBuffer);
    }
    layer_data->device_dispatch_table.CmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, regionCount, pRegions);
}

void DispatchCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage,
                          VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageCopy* pRegions) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.CmdCopyImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout,
                                                              regionCount, pRegions);
    {
        srcImage = layer_data->Unwrap(srcImage);
        dstImage = layer_data->Unwrap(dstImage);
    }
    layer_data->device_dispatch_table.CmdCopyImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount,
                                                   pRegions);
}

void DispatchCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage,
                          VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageBlit* pRegions, VkFilter filter) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.CmdBlitImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout,
                                                              regionCount, pRegions, filter);
    {
        srcImage = layer_data->Unwrap(srcImage);
        dstImage = layer_data->Unwrap(dstImage);
    }
    layer_data->device_dispatch_table.CmdBlitImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount,
                                                   pRegions, filter);
}

void DispatchCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage, VkImageLayout dstImageLayout,
                                  uint32_t regionCount, const VkBufferImageCopy* pRegions) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.CmdCopyBufferToImage(commandBuffer, srcBuffer, dstImage, dstImageLayout,
                                                                      regionCount, pRegions);
    {
        srcBuffer = layer_data->Unwrap(srcBuffer);
        dstImage = layer_data->Unwrap(dstImage);
    }
    layer_data->device_dispatch_table.CmdCopyBufferToImage(commandBuffer, srcBuffer, dstImage, dstImageLayout, regionCount,
                                                           pRegions);
}

void DispatchCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkBuffer dstBuffer,
                                  uint32_t regionCount, const VkBufferImageCopy* pRegions) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.CmdCopyImageToBuffer(commandBuffer, srcImage, srcImageLayout, dstBuffer,
                                                                      regionCount, pRegions);
    {
        srcImage = layer_data->Unwrap(srcImage);
        dstBuffer = layer_data->Unwrap(dstBuffer);
    }
    layer_data->device_dispatch_table.CmdCopyImageToBuffer(commandBuffer, srcImage, srcImageLayout, dstBuffer, regionCount,
                                                           pRegions);
}

void DispatchCmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize dataSize,
                             const void* pData) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.CmdUpdateBuffer(commandBuffer, dstBuffer, dstOffset, dataSize, pData);
    { dstBuffer = layer_data->Unwrap(dstBuffer); }
    layer_data->device_dispatch_table.CmdUpdateBuffer(commandBuffer, dstBuffer, dstOffset, dataSize, pData);
}

void DispatchCmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize size,
                           uint32_t data) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CmdFillBuffer(commandBuffer, dstBuffer, dstOffset, size, data);
    { dstBuffer = layer_data->Unwrap(dstBuffer); }
    layer_data->device_dispatch_table.CmdFillBuffer(commandBuffer, dstBuffer, dstOffset, size, data);
}

void DispatchCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                const VkClearColorValue* pColor, uint32_t rangeCount, const VkImageSubresourceRange* pRanges) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.CmdClearColorImage(commandBuffer, image, imageLayout, pColor, rangeCount, pRanges);
    { image = layer_data->Unwrap(image); }
    layer_data->device_dispatch_table.CmdClearColorImage(commandBuffer, image, imageLayout, pColor, rangeCount, pRanges);
}

void DispatchCmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                       const VkClearDepthStencilValue* pDepthStencil, uint32_t rangeCount,
                                       const VkImageSubresourceRange* pRanges) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.CmdClearDepthStencilImage(commandBuffer, image, imageLayout, pDepthStencil,
                                                                           rangeCount, pRanges);
    { image = layer_data->Unwrap(image); }
    layer_data->device_dispatch_table.CmdClearDepthStencilImage(commandBuffer, image, imageLayout, pDepthStencil, rangeCount,
                                                                pRanges);
}

void DispatchCmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount, const VkClearAttachment* pAttachments,
                                 uint32_t rectCount, const VkClearRect* pRects) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdClearAttachments(commandBuffer, attachmentCount, pAttachments, rectCount, pRects);
}

void DispatchCmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage,
                             VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageResolve* pRegions) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.CmdResolveImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout,
                                                                 regionCount, pRegions);
    {
        srcImage = layer_data->Unwrap(srcImage);
        dstImage = layer_data->Unwrap(dstImage);
    }
    layer_data->device_dispatch_table.CmdResolveImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout,
                                                      regionCount, pRegions);
}

void DispatchCmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CmdSetEvent(commandBuffer, event, stageMask);
    { event = layer_data->Unwrap(event); }
    layer_data->device_dispatch_table.CmdSetEvent(commandBuffer, event, stageMask);
}

void DispatchCmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CmdResetEvent(commandBuffer, event, stageMask);
    { event = layer_data->Unwrap(event); }
    layer_data->device_dispatch_table.CmdResetEvent(commandBuffer, event, stageMask);
}

void DispatchCmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                           VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, uint32_t memoryBarrierCount,
                           const VkMemoryBarrier* pMemoryBarriers, uint32_t bufferMemoryBarrierCount,
                           const VkBufferMemoryBarrier* pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount,
                           const VkImageMemoryBarrier* pImageMemoryBarriers) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.CmdWaitEvents(
            commandBuffer, eventCount, pEvents, srcStageMask, dstStageMask, memoryBarrierCount, pMemoryBarriers,
            bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
    VkEvent var_local_pEvents[DISPATCH_MAX_STACK_ALLOCATIONS];
    VkEvent* local_pEvents = nullptr;
    safe_VkBufferMemoryBarrier* local_pBufferMemoryBarriers = nullptr;
    safe_VkImageMemoryBarrier* local_pImageMemoryBarriers = nullptr;
    {
        if (pEvents) {
            local_pEvents = eventCount > DISPATCH_MAX_STACK_ALLOCATIONS ? new VkEvent[eventCount] : var_local_pEvents;
            for (uint32_t index0 = 0; index0 < eventCount; ++index0) {
                local_pEvents[index0] = layer_data->Unwrap(pEvents[index0]);
            }
        }
        if (pBufferMemoryBarriers) {
            local_pBufferMemoryBarriers = new safe_VkBufferMemoryBarrier[bufferMemoryBarrierCount];
            for (uint32_t index0 = 0; index0 < bufferMemoryBarrierCount; ++index0) {
                local_pBufferMemoryBarriers[index0].initialize(&pBufferMemoryBarriers[index0]);

                if (pBufferMemoryBarriers[index0].buffer) {
                    local_pBufferMemoryBarriers[index0].buffer = layer_data->Unwrap(pBufferMemoryBarriers[index0].buffer);
                }
            }
        }
        if (pImageMemoryBarriers) {
            local_pImageMemoryBarriers = new safe_VkImageMemoryBarrier[imageMemoryBarrierCount];
            for (uint32_t index0 = 0; index0 < imageMemoryBarrierCount; ++index0) {
                local_pImageMemoryBarriers[index0].initialize(&pImageMemoryBarriers[index0]);

                if (pImageMemoryBarriers[index0].image) {
                    local_pImageMemoryBarriers[index0].image = layer_data->Unwrap(pImageMemoryBarriers[index0].image);
                }
            }
        }
    }
    layer_data->device_dispatch_table.CmdWaitEvents(
        commandBuffer, eventCount, (const VkEvent*)local_pEvents, srcStageMask, dstStageMask, memoryBarrierCount, pMemoryBarriers,
        bufferMemoryBarrierCount, (const VkBufferMemoryBarrier*)local_pBufferMemoryBarriers, imageMemoryBarrierCount,
        (const VkImageMemoryBarrier*)local_pImageMemoryBarriers);
    if (local_pEvents != var_local_pEvents) delete[] local_pEvents;
    if (local_pBufferMemoryBarriers) {
        delete[] local_pBufferMemoryBarriers;
    }
    if (local_pImageMemoryBarriers) {
        delete[] local_pImageMemoryBarriers;
    }
}

void DispatchCmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask,
                                VkDependencyFlags dependencyFlags, uint32_t memoryBarrierCount,
                                const VkMemoryBarrier* pMemoryBarriers, uint32_t bufferMemoryBarrierCount,
                                const VkBufferMemoryBarrier* pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount,
                                const VkImageMemoryBarrier* pImageMemoryBarriers) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.CmdPipelineBarrier(
            commandBuffer, srcStageMask, dstStageMask, dependencyFlags, memoryBarrierCount, pMemoryBarriers,
            bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
    safe_VkBufferMemoryBarrier* local_pBufferMemoryBarriers = nullptr;
    safe_VkImageMemoryBarrier* local_pImageMemoryBarriers = nullptr;
    {
        if (pBufferMemoryBarriers) {
            local_pBufferMemoryBarriers = new safe_VkBufferMemoryBarrier[bufferMemoryBarrierCount];
            for (uint32_t index0 = 0; index0 < bufferMemoryBarrierCount; ++index0) {
                local_pBufferMemoryBarriers[index0].initialize(&pBufferMemoryBarriers[index0]);

                if (pBufferMemoryBarriers[index0].buffer) {
                    local_pBufferMemoryBarriers[index0].buffer = layer_data->Unwrap(pBufferMemoryBarriers[index0].buffer);
                }
            }
        }
        if (pImageMemoryBarriers) {
            local_pImageMemoryBarriers = new safe_VkImageMemoryBarrier[imageMemoryBarrierCount];
            for (uint32_t index0 = 0; index0 < imageMemoryBarrierCount; ++index0) {
                local_pImageMemoryBarriers[index0].initialize(&pImageMemoryBarriers[index0]);

                if (pImageMemoryBarriers[index0].image) {
                    local_pImageMemoryBarriers[index0].image = layer_data->Unwrap(pImageMemoryBarriers[index0].image);
                }
            }
        }
    }
    layer_data->device_dispatch_table.CmdPipelineBarrier(
        commandBuffer, srcStageMask, dstStageMask, dependencyFlags, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount,
        (const VkBufferMemoryBarrier*)local_pBufferMemoryBarriers, imageMemoryBarrierCount,
        (const VkImageMemoryBarrier*)local_pImageMemoryBarriers);
    if (local_pBufferMemoryBarriers) {
        delete[] local_pBufferMemoryBarriers;
    }
    if (local_pImageMemoryBarriers) {
        delete[] local_pImageMemoryBarriers;
    }
}

void DispatchCmdBeginQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query, VkQueryControlFlags flags) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CmdBeginQuery(commandBuffer, queryPool, query, flags);
    { queryPool = layer_data->Unwrap(queryPool); }
    layer_data->device_dispatch_table.CmdBeginQuery(commandBuffer, queryPool, query, flags);
}

void DispatchCmdEndQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CmdEndQuery(commandBuffer, queryPool, query);
    { queryPool = layer_data->Unwrap(queryPool); }
    layer_data->device_dispatch_table.CmdEndQuery(commandBuffer, queryPool, query);
}

void DispatchCmdResetQueryPool(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CmdResetQueryPool(commandBuffer, queryPool, firstQuery, queryCount);
    { queryPool = layer_data->Unwrap(queryPool); }
    layer_data->device_dispatch_table.CmdResetQueryPool(commandBuffer, queryPool, firstQuery, queryCount);
}

void DispatchCmdWriteTimestamp(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage, VkQueryPool queryPool,
                               uint32_t query) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CmdWriteTimestamp(commandBuffer, pipelineStage, queryPool, query);
    { queryPool = layer_data->Unwrap(queryPool); }
    layer_data->device_dispatch_table.CmdWriteTimestamp(commandBuffer, pipelineStage, queryPool, query);
}

void DispatchCmdCopyQueryPoolResults(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount,
                                     VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize stride, VkQueryResultFlags flags) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.CmdCopyQueryPoolResults(commandBuffer, queryPool, firstQuery, queryCount,
                                                                         dstBuffer, dstOffset, stride, flags);
    {
        queryPool = layer_data->Unwrap(queryPool);
        dstBuffer = layer_data->Unwrap(dstBuffer);
    }
    layer_data->device_dispatch_table.CmdCopyQueryPoolResults(commandBuffer, queryPool, firstQuery, queryCount, dstBuffer,
                                                              dstOffset, stride, flags);
}

void DispatchCmdPushConstants(VkCommandBuffer commandBuffer, VkPipelineLayout layout, VkShaderStageFlags stageFlags,
                              uint32_t offset, uint32_t size, const void* pValues) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.CmdPushConstants(commandBuffer, layout, stageFlags, offset, size, pValues);
    { layout = layer_data->Unwrap(layout); }
    layer_data->device_dispatch_table.CmdPushConstants(commandBuffer, layout, stageFlags, offset, size, pValues);
}

void DispatchCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                VkSubpassContents contents) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CmdBeginRenderPass(commandBuffer, pRenderPassBegin, contents);
    safe_VkRenderPassBeginInfo var_local_pRenderPassBegin;
    safe_VkRenderPassBeginInfo* local_pRenderPassBegin = nullptr;
    {
        if (pRenderPassBegin) {
            local_pRenderPassBegin = &var_local_pRenderPassBegin;
            local_pRenderPassBegin->initialize(pRenderPassBegin);

            if (pRenderPassBegin->renderPass) {
                local_pRenderPassBegin->renderPass = layer_data->Unwrap(pRenderPassBegin->renderPass);
            }
            if (pRenderPassBegin->framebuffer) {
                local_pRenderPassBegin->framebuffer = layer_data->Unwrap(pRenderPassBegin->framebuffer);
            }
            WrapPnextChainHandles(layer_data, local_pRenderPassBegin->pNext);
        }
    }
    layer_data->device_dispatch_table.CmdBeginRenderPass(commandBuffer, (const VkRenderPassBeginInfo*)local_pRenderPassBegin,
                                                         contents);
}

void DispatchCmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdNextSubpass(commandBuffer, contents);
}

void DispatchCmdEndRenderPass(VkCommandBuffer commandBuffer) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdEndRenderPass(commandBuffer);
}

void DispatchCmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBufferCount,
                                const VkCommandBuffer* pCommandBuffers) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdExecuteCommands(commandBuffer, commandBufferCount, pCommandBuffers);
}

VkResult DispatchBindBufferMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindBufferMemoryInfo* pBindInfos) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.BindBufferMemory2(device, bindInfoCount, pBindInfos);
    safe_VkBindBufferMemoryInfo* local_pBindInfos = nullptr;
    {
        if (pBindInfos) {
            local_pBindInfos = new safe_VkBindBufferMemoryInfo[bindInfoCount];
            for (uint32_t index0 = 0; index0 < bindInfoCount; ++index0) {
                local_pBindInfos[index0].initialize(&pBindInfos[index0]);

                if (pBindInfos[index0].buffer) {
                    local_pBindInfos[index0].buffer = layer_data->Unwrap(pBindInfos[index0].buffer);
                }
                if (pBindInfos[index0].memory) {
                    local_pBindInfos[index0].memory = layer_data->Unwrap(pBindInfos[index0].memory);
                }
            }
        }
    }
    VkResult result =
        layer_data->device_dispatch_table.BindBufferMemory2(device, bindInfoCount, (const VkBindBufferMemoryInfo*)local_pBindInfos);
    if (local_pBindInfos) {
        delete[] local_pBindInfos;
    }
    return result;
}

VkResult DispatchBindImageMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo* pBindInfos) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.BindImageMemory2(device, bindInfoCount, pBindInfos);
    safe_VkBindImageMemoryInfo* local_pBindInfos = nullptr;
    {
        if (pBindInfos) {
            local_pBindInfos = new safe_VkBindImageMemoryInfo[bindInfoCount];
            for (uint32_t index0 = 0; index0 < bindInfoCount; ++index0) {
                local_pBindInfos[index0].initialize(&pBindInfos[index0]);
                WrapPnextChainHandles(layer_data, local_pBindInfos[index0].pNext);

                if (pBindInfos[index0].image) {
                    local_pBindInfos[index0].image = layer_data->Unwrap(pBindInfos[index0].image);
                }
                if (pBindInfos[index0].memory) {
                    local_pBindInfos[index0].memory = layer_data->Unwrap(pBindInfos[index0].memory);
                }
            }
        }
    }
    VkResult result =
        layer_data->device_dispatch_table.BindImageMemory2(device, bindInfoCount, (const VkBindImageMemoryInfo*)local_pBindInfos);
    if (local_pBindInfos) {
        delete[] local_pBindInfos;
    }
    return result;
}

void DispatchGetDeviceGroupPeerMemoryFeatures(VkDevice device, uint32_t heapIndex, uint32_t localDeviceIndex,
                                              uint32_t remoteDeviceIndex, VkPeerMemoryFeatureFlags* pPeerMemoryFeatures) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);

    layer_data->device_dispatch_table.GetDeviceGroupPeerMemoryFeatures(device, heapIndex, localDeviceIndex, remoteDeviceIndex,
                                                                       pPeerMemoryFeatures);
}

void DispatchCmdSetDeviceMask(VkCommandBuffer commandBuffer, uint32_t deviceMask) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdSetDeviceMask(commandBuffer, deviceMask);
}

void DispatchCmdDispatchBase(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY, uint32_t baseGroupZ,
                             uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdDispatchBase(commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY,
                                                      groupCountZ);
}

VkResult DispatchEnumeratePhysicalDeviceGroups(VkInstance instance, uint32_t* pPhysicalDeviceGroupCount,
                                               VkPhysicalDeviceGroupProperties* pPhysicalDeviceGroupProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(instance), layer_data_map);

    VkResult result = layer_data->instance_dispatch_table.EnumeratePhysicalDeviceGroups(instance, pPhysicalDeviceGroupCount,
                                                                                        pPhysicalDeviceGroupProperties);

    return result;
}

void DispatchGetImageMemoryRequirements2(VkDevice device, const VkImageMemoryRequirementsInfo2* pInfo,
                                         VkMemoryRequirements2* pMemoryRequirements) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.GetImageMemoryRequirements2(device, pInfo, pMemoryRequirements);
    safe_VkImageMemoryRequirementsInfo2 var_local_pInfo;
    safe_VkImageMemoryRequirementsInfo2* local_pInfo = nullptr;
    {
        if (pInfo) {
            local_pInfo = &var_local_pInfo;
            local_pInfo->initialize(pInfo);

            if (pInfo->image) {
                local_pInfo->image = layer_data->Unwrap(pInfo->image);
            }
        }
    }
    layer_data->device_dispatch_table.GetImageMemoryRequirements2(device, (const VkImageMemoryRequirementsInfo2*)local_pInfo,
                                                                  pMemoryRequirements);
}

void DispatchGetBufferMemoryRequirements2(VkDevice device, const VkBufferMemoryRequirementsInfo2* pInfo,
                                          VkMemoryRequirements2* pMemoryRequirements) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.GetBufferMemoryRequirements2(device, pInfo, pMemoryRequirements);
    safe_VkBufferMemoryRequirementsInfo2 var_local_pInfo;
    safe_VkBufferMemoryRequirementsInfo2* local_pInfo = nullptr;
    {
        if (pInfo) {
            local_pInfo = &var_local_pInfo;
            local_pInfo->initialize(pInfo);

            if (pInfo->buffer) {
                local_pInfo->buffer = layer_data->Unwrap(pInfo->buffer);
            }
        }
    }
    layer_data->device_dispatch_table.GetBufferMemoryRequirements2(device, (const VkBufferMemoryRequirementsInfo2*)local_pInfo,
                                                                   pMemoryRequirements);
}

void DispatchGetImageSparseMemoryRequirements2(VkDevice device, const VkImageSparseMemoryRequirementsInfo2* pInfo,
                                               uint32_t* pSparseMemoryRequirementCount,
                                               VkSparseImageMemoryRequirements2* pSparseMemoryRequirements) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.GetImageSparseMemoryRequirements2(device, pInfo, pSparseMemoryRequirementCount,
                                                                                   pSparseMemoryRequirements);
    safe_VkImageSparseMemoryRequirementsInfo2 var_local_pInfo;
    safe_VkImageSparseMemoryRequirementsInfo2* local_pInfo = nullptr;
    {
        if (pInfo) {
            local_pInfo = &var_local_pInfo;
            local_pInfo->initialize(pInfo);

            if (pInfo->image) {
                local_pInfo->image = layer_data->Unwrap(pInfo->image);
            }
        }
    }
    layer_data->device_dispatch_table.GetImageSparseMemoryRequirements2(
        device, (const VkImageSparseMemoryRequirementsInfo2*)local_pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
}

void DispatchGetPhysicalDeviceFeatures2(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures2* pFeatures) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);

    layer_data->instance_dispatch_table.GetPhysicalDeviceFeatures2(physicalDevice, pFeatures);
}

void DispatchGetPhysicalDeviceProperties2(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties2* pProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);

    layer_data->instance_dispatch_table.GetPhysicalDeviceProperties2(physicalDevice, pProperties);
}

void DispatchGetPhysicalDeviceFormatProperties2(VkPhysicalDevice physicalDevice, VkFormat format,
                                                VkFormatProperties2* pFormatProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);

    layer_data->instance_dispatch_table.GetPhysicalDeviceFormatProperties2(physicalDevice, format, pFormatProperties);
}

VkResult DispatchGetPhysicalDeviceImageFormatProperties2(VkPhysicalDevice physicalDevice,
                                                         const VkPhysicalDeviceImageFormatInfo2* pImageFormatInfo,
                                                         VkImageFormatProperties2* pImageFormatProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);

    VkResult result = layer_data->instance_dispatch_table.GetPhysicalDeviceImageFormatProperties2(physicalDevice, pImageFormatInfo,
                                                                                                  pImageFormatProperties);

    return result;
}

void DispatchGetPhysicalDeviceQueueFamilyProperties2(VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount,
                                                     VkQueueFamilyProperties2* pQueueFamilyProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);

    layer_data->instance_dispatch_table.GetPhysicalDeviceQueueFamilyProperties2(physicalDevice, pQueueFamilyPropertyCount,
                                                                                pQueueFamilyProperties);
}

void DispatchGetPhysicalDeviceMemoryProperties2(VkPhysicalDevice physicalDevice,
                                                VkPhysicalDeviceMemoryProperties2* pMemoryProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);

    layer_data->instance_dispatch_table.GetPhysicalDeviceMemoryProperties2(physicalDevice, pMemoryProperties);
}

void DispatchGetPhysicalDeviceSparseImageFormatProperties2(VkPhysicalDevice physicalDevice,
                                                           const VkPhysicalDeviceSparseImageFormatInfo2* pFormatInfo,
                                                           uint32_t* pPropertyCount, VkSparseImageFormatProperties2* pProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);

    layer_data->instance_dispatch_table.GetPhysicalDeviceSparseImageFormatProperties2(physicalDevice, pFormatInfo, pPropertyCount,
                                                                                      pProperties);
}

void DispatchTrimCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolTrimFlags flags) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.TrimCommandPool(device, commandPool, flags);
    { commandPool = layer_data->Unwrap(commandPool); }
    layer_data->device_dispatch_table.TrimCommandPool(device, commandPool, flags);
}

void DispatchGetDeviceQueue2(VkDevice device, const VkDeviceQueueInfo2* pQueueInfo, VkQueue* pQueue) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);

    layer_data->device_dispatch_table.GetDeviceQueue2(device, pQueueInfo, pQueue);
}

VkResult DispatchCreateSamplerYcbcrConversion(VkDevice device, const VkSamplerYcbcrConversionCreateInfo* pCreateInfo,
                                              const VkAllocationCallbacks* pAllocator, VkSamplerYcbcrConversion* pYcbcrConversion) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.CreateSamplerYcbcrConversion(device, pCreateInfo, pAllocator, pYcbcrConversion);

    VkResult result =
        layer_data->device_dispatch_table.CreateSamplerYcbcrConversion(device, pCreateInfo, pAllocator, pYcbcrConversion);
    if (VK_SUCCESS == result) {
        *pYcbcrConversion = layer_data->WrapNew(*pYcbcrConversion);
    }
    return result;
}

void DispatchDestroySamplerYcbcrConversion(VkDevice device, VkSamplerYcbcrConversion ycbcrConversion,
                                           const VkAllocationCallbacks* pAllocator) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.DestroySamplerYcbcrConversion(device, ycbcrConversion, pAllocator);

    uint64_t ycbcrConversion_id = CastToUint64(ycbcrConversion);
    auto iter = unique_id_mapping.pop(ycbcrConversion_id);
    if (iter != unique_id_mapping.end()) {
        ycbcrConversion = (VkSamplerYcbcrConversion)iter->second;
    } else {
        ycbcrConversion = (VkSamplerYcbcrConversion)0;
    }
    layer_data->device_dispatch_table.DestroySamplerYcbcrConversion(device, ycbcrConversion, pAllocator);
}

void DispatchGetPhysicalDeviceExternalBufferProperties(VkPhysicalDevice physicalDevice,
                                                       const VkPhysicalDeviceExternalBufferInfo* pExternalBufferInfo,
                                                       VkExternalBufferProperties* pExternalBufferProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);

    layer_data->instance_dispatch_table.GetPhysicalDeviceExternalBufferProperties(physicalDevice, pExternalBufferInfo,
                                                                                  pExternalBufferProperties);
}

void DispatchGetPhysicalDeviceExternalFenceProperties(VkPhysicalDevice physicalDevice,
                                                      const VkPhysicalDeviceExternalFenceInfo* pExternalFenceInfo,
                                                      VkExternalFenceProperties* pExternalFenceProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);

    layer_data->instance_dispatch_table.GetPhysicalDeviceExternalFenceProperties(physicalDevice, pExternalFenceInfo,
                                                                                 pExternalFenceProperties);
}

void DispatchGetPhysicalDeviceExternalSemaphoreProperties(VkPhysicalDevice physicalDevice,
                                                          const VkPhysicalDeviceExternalSemaphoreInfo* pExternalSemaphoreInfo,
                                                          VkExternalSemaphoreProperties* pExternalSemaphoreProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);

    layer_data->instance_dispatch_table.GetPhysicalDeviceExternalSemaphoreProperties(physicalDevice, pExternalSemaphoreInfo,
                                                                                     pExternalSemaphoreProperties);
}

void DispatchGetDescriptorSetLayoutSupport(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo,
                                           VkDescriptorSetLayoutSupport* pSupport) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.GetDescriptorSetLayoutSupport(device, pCreateInfo, pSupport);
    safe_VkDescriptorSetLayoutCreateInfo var_local_pCreateInfo;
    safe_VkDescriptorSetLayoutCreateInfo* local_pCreateInfo = nullptr;
    {
        if (pCreateInfo) {
            local_pCreateInfo = &var_local_pCreateInfo;
            local_pCreateInfo->initialize(pCreateInfo);
            if (local_pCreateInfo->pBindings) {
                for (uint32_t index1 = 0; index1 < local_pCreateInfo->bindingCount; ++index1) {
                    if (local_pCreateInfo->pBindings[index1].pImmutableSamplers) {
                        for (uint32_t index2 = 0; index2 < local_pCreateInfo->pBindings[index1].descriptorCount; ++index2) {
                            local_pCreateInfo->pBindings[index1].pImmutableSamplers[index2] =
                                layer_data->Unwrap(local_pCreateInfo->pBindings[index1].pImmutableSamplers[index2]);
                        }
                    }
                }
            }
        }
    }
    layer_data->device_dispatch_table.GetDescriptorSetLayoutSupport(
        device, (const VkDescriptorSetLayoutCreateInfo*)local_pCreateInfo, pSupport);
}

void DispatchCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer,
                                  VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.CmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset,
                                                                      maxDrawCount, stride);
    {
        buffer = layer_data->Unwrap(buffer);
        countBuffer = layer_data->Unwrap(countBuffer);
    }
    layer_data->device_dispatch_table.CmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset,
                                                           maxDrawCount, stride);
}

void DispatchCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer,
                                         VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.CmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer,
                                                                             countBufferOffset, maxDrawCount, stride);
    {
        buffer = layer_data->Unwrap(buffer);
        countBuffer = layer_data->Unwrap(countBuffer);
    }
    layer_data->device_dispatch_table.CmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset,
                                                                  maxDrawCount, stride);
}

void DispatchCmdBeginRenderPass2(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                 const VkSubpassBeginInfo* pSubpassBeginInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.CmdBeginRenderPass2(commandBuffer, pRenderPassBegin, pSubpassBeginInfo);
    safe_VkRenderPassBeginInfo var_local_pRenderPassBegin;
    safe_VkRenderPassBeginInfo* local_pRenderPassBegin = nullptr;
    {
        if (pRenderPassBegin) {
            local_pRenderPassBegin = &var_local_pRenderPassBegin;
            local_pRenderPassBegin->initialize(pRenderPassBegin);

            if (pRenderPassBegin->renderPass) {
                local_pRenderPassBegin->renderPass = layer_data->Unwrap(pRenderPassBegin->renderPass);
            }
            if (pRenderPassBegin->framebuffer) {
                local_pRenderPassBegin->framebuffer = layer_data->Unwrap(pRenderPassBegin->framebuffer);
            }
            WrapPnextChainHandles(layer_data, local_pRenderPassBegin->pNext);
        }
    }
    layer_data->device_dispatch_table.CmdBeginRenderPass2(commandBuffer, (const VkRenderPassBeginInfo*)local_pRenderPassBegin,
                                                          pSubpassBeginInfo);
}

void DispatchCmdNextSubpass2(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo* pSubpassBeginInfo,
                             const VkSubpassEndInfo* pSubpassEndInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdNextSubpass2(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo);
}

void DispatchCmdEndRenderPass2(VkCommandBuffer commandBuffer, const VkSubpassEndInfo* pSubpassEndInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdEndRenderPass2(commandBuffer, pSubpassEndInfo);
}

void DispatchResetQueryPool(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.ResetQueryPool(device, queryPool, firstQuery, queryCount);
    { queryPool = layer_data->Unwrap(queryPool); }
    layer_data->device_dispatch_table.ResetQueryPool(device, queryPool, firstQuery, queryCount);
}

VkResult DispatchGetSemaphoreCounterValue(VkDevice device, VkSemaphore semaphore, uint64_t* pValue) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.GetSemaphoreCounterValue(device, semaphore, pValue);
    { semaphore = layer_data->Unwrap(semaphore); }
    VkResult result = layer_data->device_dispatch_table.GetSemaphoreCounterValue(device, semaphore, pValue);

    return result;
}

VkResult DispatchWaitSemaphores(VkDevice device, const VkSemaphoreWaitInfo* pWaitInfo, uint64_t timeout) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.WaitSemaphores(device, pWaitInfo, timeout);
    safe_VkSemaphoreWaitInfo var_local_pWaitInfo;
    safe_VkSemaphoreWaitInfo* local_pWaitInfo = nullptr;
    {
        if (pWaitInfo) {
            local_pWaitInfo = &var_local_pWaitInfo;
            local_pWaitInfo->initialize(pWaitInfo);
            if (local_pWaitInfo->pSemaphores) {
                for (uint32_t index1 = 0; index1 < local_pWaitInfo->semaphoreCount; ++index1) {
                    local_pWaitInfo->pSemaphores[index1] = layer_data->Unwrap(local_pWaitInfo->pSemaphores[index1]);
                }
            }
        }
    }
    VkResult result =
        layer_data->device_dispatch_table.WaitSemaphores(device, (const VkSemaphoreWaitInfo*)local_pWaitInfo, timeout);

    return result;
}

VkResult DispatchSignalSemaphore(VkDevice device, const VkSemaphoreSignalInfo* pSignalInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.SignalSemaphore(device, pSignalInfo);
    safe_VkSemaphoreSignalInfo var_local_pSignalInfo;
    safe_VkSemaphoreSignalInfo* local_pSignalInfo = nullptr;
    {
        if (pSignalInfo) {
            local_pSignalInfo = &var_local_pSignalInfo;
            local_pSignalInfo->initialize(pSignalInfo);

            if (pSignalInfo->semaphore) {
                local_pSignalInfo->semaphore = layer_data->Unwrap(pSignalInfo->semaphore);
            }
        }
    }
    VkResult result = layer_data->device_dispatch_table.SignalSemaphore(device, (const VkSemaphoreSignalInfo*)local_pSignalInfo);

    return result;
}

VkDeviceAddress DispatchGetBufferDeviceAddress(VkDevice device, const VkBufferDeviceAddressInfo* pInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.GetBufferDeviceAddress(device, pInfo);
    safe_VkBufferDeviceAddressInfo var_local_pInfo;
    safe_VkBufferDeviceAddressInfo* local_pInfo = nullptr;
    {
        if (pInfo) {
            local_pInfo = &var_local_pInfo;
            local_pInfo->initialize(pInfo);

            if (pInfo->buffer) {
                local_pInfo->buffer = layer_data->Unwrap(pInfo->buffer);
            }
        }
    }
    VkDeviceAddress result =
        layer_data->device_dispatch_table.GetBufferDeviceAddress(device, (const VkBufferDeviceAddressInfo*)local_pInfo);

    return result;
}

uint64_t DispatchGetBufferOpaqueCaptureAddress(VkDevice device, const VkBufferDeviceAddressInfo* pInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.GetBufferOpaqueCaptureAddress(device, pInfo);
    safe_VkBufferDeviceAddressInfo var_local_pInfo;
    safe_VkBufferDeviceAddressInfo* local_pInfo = nullptr;
    {
        if (pInfo) {
            local_pInfo = &var_local_pInfo;
            local_pInfo->initialize(pInfo);

            if (pInfo->buffer) {
                local_pInfo->buffer = layer_data->Unwrap(pInfo->buffer);
            }
        }
    }
    uint64_t result =
        layer_data->device_dispatch_table.GetBufferOpaqueCaptureAddress(device, (const VkBufferDeviceAddressInfo*)local_pInfo);

    return result;
}

uint64_t DispatchGetDeviceMemoryOpaqueCaptureAddress(VkDevice device, const VkDeviceMemoryOpaqueCaptureAddressInfo* pInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.GetDeviceMemoryOpaqueCaptureAddress(device, pInfo);
    safe_VkDeviceMemoryOpaqueCaptureAddressInfo var_local_pInfo;
    safe_VkDeviceMemoryOpaqueCaptureAddressInfo* local_pInfo = nullptr;
    {
        if (pInfo) {
            local_pInfo = &var_local_pInfo;
            local_pInfo->initialize(pInfo);

            if (pInfo->memory) {
                local_pInfo->memory = layer_data->Unwrap(pInfo->memory);
            }
        }
    }
    uint64_t result = layer_data->device_dispatch_table.GetDeviceMemoryOpaqueCaptureAddress(
        device, (const VkDeviceMemoryOpaqueCaptureAddressInfo*)local_pInfo);

    return result;
}

VkResult DispatchGetPhysicalDeviceToolProperties(VkPhysicalDevice physicalDevice, uint32_t* pToolCount,
                                                 VkPhysicalDeviceToolProperties* pToolProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);

    VkResult result =
        layer_data->instance_dispatch_table.GetPhysicalDeviceToolProperties(physicalDevice, pToolCount, pToolProperties);

    return result;
}

VkResult DispatchCreatePrivateDataSlot(VkDevice device, const VkPrivateDataSlotCreateInfo* pCreateInfo,
                                       const VkAllocationCallbacks* pAllocator, VkPrivateDataSlot* pPrivateDataSlot) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.CreatePrivateDataSlot(device, pCreateInfo, pAllocator, pPrivateDataSlot);

    VkResult result = layer_data->device_dispatch_table.CreatePrivateDataSlot(device, pCreateInfo, pAllocator, pPrivateDataSlot);
    if (VK_SUCCESS == result) {
        *pPrivateDataSlot = layer_data->WrapNew(*pPrivateDataSlot);
    }
    return result;
}

void DispatchDestroyPrivateDataSlot(VkDevice device, VkPrivateDataSlot privateDataSlot, const VkAllocationCallbacks* pAllocator) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.DestroyPrivateDataSlot(device, privateDataSlot, pAllocator);

    uint64_t privateDataSlot_id = CastToUint64(privateDataSlot);
    auto iter = unique_id_mapping.pop(privateDataSlot_id);
    if (iter != unique_id_mapping.end()) {
        privateDataSlot = (VkPrivateDataSlot)iter->second;
    } else {
        privateDataSlot = (VkPrivateDataSlot)0;
    }
    layer_data->device_dispatch_table.DestroyPrivateDataSlot(device, privateDataSlot, pAllocator);
}

void DispatchCmdSetEvent2(VkCommandBuffer commandBuffer, VkEvent event, const VkDependencyInfo* pDependencyInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CmdSetEvent2(commandBuffer, event, pDependencyInfo);
    safe_VkDependencyInfo var_local_pDependencyInfo;
    safe_VkDependencyInfo* local_pDependencyInfo = nullptr;
    {
        event = layer_data->Unwrap(event);
        if (pDependencyInfo) {
            local_pDependencyInfo = &var_local_pDependencyInfo;
            local_pDependencyInfo->initialize(pDependencyInfo);
            if (local_pDependencyInfo->pBufferMemoryBarriers) {
                for (uint32_t index1 = 0; index1 < local_pDependencyInfo->bufferMemoryBarrierCount; ++index1) {
                    if (pDependencyInfo->pBufferMemoryBarriers[index1].buffer) {
                        local_pDependencyInfo->pBufferMemoryBarriers[index1].buffer =
                            layer_data->Unwrap(pDependencyInfo->pBufferMemoryBarriers[index1].buffer);
                    }
                }
            }
            if (local_pDependencyInfo->pImageMemoryBarriers) {
                for (uint32_t index1 = 0; index1 < local_pDependencyInfo->imageMemoryBarrierCount; ++index1) {
                    if (pDependencyInfo->pImageMemoryBarriers[index1].image) {
                        local_pDependencyInfo->pImageMemoryBarriers[index1].image =
                            layer_data->Unwrap(pDependencyInfo->pImageMemoryBarriers[index1].image);
                    }
                }
            }
        }
    }
    layer_data->device_dispatch_table.CmdSetEvent2(commandBuffer, event, (const VkDependencyInfo*)local_pDependencyInfo);
}

void DispatchCmdResetEvent2(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags2 stageMask) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CmdResetEvent2(commandBuffer, event, stageMask);
    { event = layer_data->Unwrap(event); }
    layer_data->device_dispatch_table.CmdResetEvent2(commandBuffer, event, stageMask);
}

void DispatchCmdWaitEvents2(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                            const VkDependencyInfo* pDependencyInfos) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.CmdWaitEvents2(commandBuffer, eventCount, pEvents, pDependencyInfos);
    VkEvent var_local_pEvents[DISPATCH_MAX_STACK_ALLOCATIONS];
    VkEvent* local_pEvents = nullptr;
    safe_VkDependencyInfo* local_pDependencyInfos = nullptr;
    {
        if (pEvents) {
            local_pEvents = eventCount > DISPATCH_MAX_STACK_ALLOCATIONS ? new VkEvent[eventCount] : var_local_pEvents;
            for (uint32_t index0 = 0; index0 < eventCount; ++index0) {
                local_pEvents[index0] = layer_data->Unwrap(pEvents[index0]);
            }
        }
        if (pDependencyInfos) {
            local_pDependencyInfos = new safe_VkDependencyInfo[eventCount];
            for (uint32_t index0 = 0; index0 < eventCount; ++index0) {
                local_pDependencyInfos[index0].initialize(&pDependencyInfos[index0]);
                if (local_pDependencyInfos[index0].pBufferMemoryBarriers) {
                    for (uint32_t index1 = 0; index1 < local_pDependencyInfos[index0].bufferMemoryBarrierCount; ++index1) {
                        if (pDependencyInfos[index0].pBufferMemoryBarriers[index1].buffer) {
                            local_pDependencyInfos[index0].pBufferMemoryBarriers[index1].buffer =
                                layer_data->Unwrap(pDependencyInfos[index0].pBufferMemoryBarriers[index1].buffer);
                        }
                    }
                }
                if (local_pDependencyInfos[index0].pImageMemoryBarriers) {
                    for (uint32_t index1 = 0; index1 < local_pDependencyInfos[index0].imageMemoryBarrierCount; ++index1) {
                        if (pDependencyInfos[index0].pImageMemoryBarriers[index1].image) {
                            local_pDependencyInfos[index0].pImageMemoryBarriers[index1].image =
                                layer_data->Unwrap(pDependencyInfos[index0].pImageMemoryBarriers[index1].image);
                        }
                    }
                }
            }
        }
    }
    layer_data->device_dispatch_table.CmdWaitEvents2(commandBuffer, eventCount, (const VkEvent*)local_pEvents,
                                                     (const VkDependencyInfo*)local_pDependencyInfos);
    if (local_pEvents != var_local_pEvents) delete[] local_pEvents;
    if (local_pDependencyInfos) {
        delete[] local_pDependencyInfos;
    }
}

void DispatchCmdPipelineBarrier2(VkCommandBuffer commandBuffer, const VkDependencyInfo* pDependencyInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CmdPipelineBarrier2(commandBuffer, pDependencyInfo);
    safe_VkDependencyInfo var_local_pDependencyInfo;
    safe_VkDependencyInfo* local_pDependencyInfo = nullptr;
    {
        if (pDependencyInfo) {
            local_pDependencyInfo = &var_local_pDependencyInfo;
            local_pDependencyInfo->initialize(pDependencyInfo);
            if (local_pDependencyInfo->pBufferMemoryBarriers) {
                for (uint32_t index1 = 0; index1 < local_pDependencyInfo->bufferMemoryBarrierCount; ++index1) {
                    if (pDependencyInfo->pBufferMemoryBarriers[index1].buffer) {
                        local_pDependencyInfo->pBufferMemoryBarriers[index1].buffer =
                            layer_data->Unwrap(pDependencyInfo->pBufferMemoryBarriers[index1].buffer);
                    }
                }
            }
            if (local_pDependencyInfo->pImageMemoryBarriers) {
                for (uint32_t index1 = 0; index1 < local_pDependencyInfo->imageMemoryBarrierCount; ++index1) {
                    if (pDependencyInfo->pImageMemoryBarriers[index1].image) {
                        local_pDependencyInfo->pImageMemoryBarriers[index1].image =
                            layer_data->Unwrap(pDependencyInfo->pImageMemoryBarriers[index1].image);
                    }
                }
            }
        }
    }
    layer_data->device_dispatch_table.CmdPipelineBarrier2(commandBuffer, (const VkDependencyInfo*)local_pDependencyInfo);
}

void DispatchCmdWriteTimestamp2(VkCommandBuffer commandBuffer, VkPipelineStageFlags2 stage, VkQueryPool queryPool, uint32_t query) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CmdWriteTimestamp2(commandBuffer, stage, queryPool, query);
    { queryPool = layer_data->Unwrap(queryPool); }
    layer_data->device_dispatch_table.CmdWriteTimestamp2(commandBuffer, stage, queryPool, query);
}

VkResult DispatchQueueSubmit2(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2* pSubmits, VkFence fence) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(queue), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.QueueSubmit2(queue, submitCount, pSubmits, fence);
    safe_VkSubmitInfo2* local_pSubmits = nullptr;
    {
        if (pSubmits) {
            local_pSubmits = new safe_VkSubmitInfo2[submitCount];
            for (uint32_t index0 = 0; index0 < submitCount; ++index0) {
                local_pSubmits[index0].initialize(&pSubmits[index0]);
                WrapPnextChainHandles(layer_data, local_pSubmits[index0].pNext);
                if (local_pSubmits[index0].pWaitSemaphoreInfos) {
                    for (uint32_t index1 = 0; index1 < local_pSubmits[index0].waitSemaphoreInfoCount; ++index1) {
                        if (pSubmits[index0].pWaitSemaphoreInfos[index1].semaphore) {
                            local_pSubmits[index0].pWaitSemaphoreInfos[index1].semaphore =
                                layer_data->Unwrap(pSubmits[index0].pWaitSemaphoreInfos[index1].semaphore);
                        }
                    }
                }
                if (local_pSubmits[index0].pSignalSemaphoreInfos) {
                    for (uint32_t index1 = 0; index1 < local_pSubmits[index0].signalSemaphoreInfoCount; ++index1) {
                        if (pSubmits[index0].pSignalSemaphoreInfos[index1].semaphore) {
                            local_pSubmits[index0].pSignalSemaphoreInfos[index1].semaphore =
                                layer_data->Unwrap(pSubmits[index0].pSignalSemaphoreInfos[index1].semaphore);
                        }
                    }
                }
            }
        }
        fence = layer_data->Unwrap(fence);
    }
    VkResult result =
        layer_data->device_dispatch_table.QueueSubmit2(queue, submitCount, (const VkSubmitInfo2*)local_pSubmits, fence);
    if (local_pSubmits) {
        delete[] local_pSubmits;
    }
    return result;
}

void DispatchCmdCopyBuffer2(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2* pCopyBufferInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CmdCopyBuffer2(commandBuffer, pCopyBufferInfo);
    safe_VkCopyBufferInfo2 var_local_pCopyBufferInfo;
    safe_VkCopyBufferInfo2* local_pCopyBufferInfo = nullptr;
    {
        if (pCopyBufferInfo) {
            local_pCopyBufferInfo = &var_local_pCopyBufferInfo;
            local_pCopyBufferInfo->initialize(pCopyBufferInfo);

            if (pCopyBufferInfo->srcBuffer) {
                local_pCopyBufferInfo->srcBuffer = layer_data->Unwrap(pCopyBufferInfo->srcBuffer);
            }
            if (pCopyBufferInfo->dstBuffer) {
                local_pCopyBufferInfo->dstBuffer = layer_data->Unwrap(pCopyBufferInfo->dstBuffer);
            }
        }
    }
    layer_data->device_dispatch_table.CmdCopyBuffer2(commandBuffer, (const VkCopyBufferInfo2*)local_pCopyBufferInfo);
}

void DispatchCmdCopyImage2(VkCommandBuffer commandBuffer, const VkCopyImageInfo2* pCopyImageInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CmdCopyImage2(commandBuffer, pCopyImageInfo);
    safe_VkCopyImageInfo2 var_local_pCopyImageInfo;
    safe_VkCopyImageInfo2* local_pCopyImageInfo = nullptr;
    {
        if (pCopyImageInfo) {
            local_pCopyImageInfo = &var_local_pCopyImageInfo;
            local_pCopyImageInfo->initialize(pCopyImageInfo);

            if (pCopyImageInfo->srcImage) {
                local_pCopyImageInfo->srcImage = layer_data->Unwrap(pCopyImageInfo->srcImage);
            }
            if (pCopyImageInfo->dstImage) {
                local_pCopyImageInfo->dstImage = layer_data->Unwrap(pCopyImageInfo->dstImage);
            }
        }
    }
    layer_data->device_dispatch_table.CmdCopyImage2(commandBuffer, (const VkCopyImageInfo2*)local_pCopyImageInfo);
}

void DispatchCmdCopyBufferToImage2(VkCommandBuffer commandBuffer, const VkCopyBufferToImageInfo2* pCopyBufferToImageInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CmdCopyBufferToImage2(commandBuffer, pCopyBufferToImageInfo);
    safe_VkCopyBufferToImageInfo2 var_local_pCopyBufferToImageInfo;
    safe_VkCopyBufferToImageInfo2* local_pCopyBufferToImageInfo = nullptr;
    {
        if (pCopyBufferToImageInfo) {
            local_pCopyBufferToImageInfo = &var_local_pCopyBufferToImageInfo;
            local_pCopyBufferToImageInfo->initialize(pCopyBufferToImageInfo);

            if (pCopyBufferToImageInfo->srcBuffer) {
                local_pCopyBufferToImageInfo->srcBuffer = layer_data->Unwrap(pCopyBufferToImageInfo->srcBuffer);
            }
            if (pCopyBufferToImageInfo->dstImage) {
                local_pCopyBufferToImageInfo->dstImage = layer_data->Unwrap(pCopyBufferToImageInfo->dstImage);
            }
        }
    }
    layer_data->device_dispatch_table.CmdCopyBufferToImage2(commandBuffer,
                                                            (const VkCopyBufferToImageInfo2*)local_pCopyBufferToImageInfo);
}

void DispatchCmdCopyImageToBuffer2(VkCommandBuffer commandBuffer, const VkCopyImageToBufferInfo2* pCopyImageToBufferInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CmdCopyImageToBuffer2(commandBuffer, pCopyImageToBufferInfo);
    safe_VkCopyImageToBufferInfo2 var_local_pCopyImageToBufferInfo;
    safe_VkCopyImageToBufferInfo2* local_pCopyImageToBufferInfo = nullptr;
    {
        if (pCopyImageToBufferInfo) {
            local_pCopyImageToBufferInfo = &var_local_pCopyImageToBufferInfo;
            local_pCopyImageToBufferInfo->initialize(pCopyImageToBufferInfo);

            if (pCopyImageToBufferInfo->srcImage) {
                local_pCopyImageToBufferInfo->srcImage = layer_data->Unwrap(pCopyImageToBufferInfo->srcImage);
            }
            if (pCopyImageToBufferInfo->dstBuffer) {
                local_pCopyImageToBufferInfo->dstBuffer = layer_data->Unwrap(pCopyImageToBufferInfo->dstBuffer);
            }
        }
    }
    layer_data->device_dispatch_table.CmdCopyImageToBuffer2(commandBuffer,
                                                            (const VkCopyImageToBufferInfo2*)local_pCopyImageToBufferInfo);
}

void DispatchCmdBlitImage2(VkCommandBuffer commandBuffer, const VkBlitImageInfo2* pBlitImageInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CmdBlitImage2(commandBuffer, pBlitImageInfo);
    safe_VkBlitImageInfo2 var_local_pBlitImageInfo;
    safe_VkBlitImageInfo2* local_pBlitImageInfo = nullptr;
    {
        if (pBlitImageInfo) {
            local_pBlitImageInfo = &var_local_pBlitImageInfo;
            local_pBlitImageInfo->initialize(pBlitImageInfo);

            if (pBlitImageInfo->srcImage) {
                local_pBlitImageInfo->srcImage = layer_data->Unwrap(pBlitImageInfo->srcImage);
            }
            if (pBlitImageInfo->dstImage) {
                local_pBlitImageInfo->dstImage = layer_data->Unwrap(pBlitImageInfo->dstImage);
            }
        }
    }
    layer_data->device_dispatch_table.CmdBlitImage2(commandBuffer, (const VkBlitImageInfo2*)local_pBlitImageInfo);
}

void DispatchCmdResolveImage2(VkCommandBuffer commandBuffer, const VkResolveImageInfo2* pResolveImageInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CmdResolveImage2(commandBuffer, pResolveImageInfo);
    safe_VkResolveImageInfo2 var_local_pResolveImageInfo;
    safe_VkResolveImageInfo2* local_pResolveImageInfo = nullptr;
    {
        if (pResolveImageInfo) {
            local_pResolveImageInfo = &var_local_pResolveImageInfo;
            local_pResolveImageInfo->initialize(pResolveImageInfo);

            if (pResolveImageInfo->srcImage) {
                local_pResolveImageInfo->srcImage = layer_data->Unwrap(pResolveImageInfo->srcImage);
            }
            if (pResolveImageInfo->dstImage) {
                local_pResolveImageInfo->dstImage = layer_data->Unwrap(pResolveImageInfo->dstImage);
            }
        }
    }
    layer_data->device_dispatch_table.CmdResolveImage2(commandBuffer, (const VkResolveImageInfo2*)local_pResolveImageInfo);
}

void DispatchCmdBeginRendering(VkCommandBuffer commandBuffer, const VkRenderingInfo* pRenderingInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CmdBeginRendering(commandBuffer, pRenderingInfo);
    safe_VkRenderingInfo var_local_pRenderingInfo;
    safe_VkRenderingInfo* local_pRenderingInfo = nullptr;
    {
        if (pRenderingInfo) {
            local_pRenderingInfo = &var_local_pRenderingInfo;
            local_pRenderingInfo->initialize(pRenderingInfo);
            if (local_pRenderingInfo->pColorAttachments) {
                for (uint32_t index1 = 0; index1 < local_pRenderingInfo->colorAttachmentCount; ++index1) {
                    if (pRenderingInfo->pColorAttachments[index1].imageView) {
                        local_pRenderingInfo->pColorAttachments[index1].imageView =
                            layer_data->Unwrap(pRenderingInfo->pColorAttachments[index1].imageView);
                    }
                    if (pRenderingInfo->pColorAttachments[index1].resolveImageView) {
                        local_pRenderingInfo->pColorAttachments[index1].resolveImageView =
                            layer_data->Unwrap(pRenderingInfo->pColorAttachments[index1].resolveImageView);
                    }
                }
            }
            if (local_pRenderingInfo->pDepthAttachment) {
                if (pRenderingInfo->pDepthAttachment->imageView) {
                    local_pRenderingInfo->pDepthAttachment->imageView =
                        layer_data->Unwrap(pRenderingInfo->pDepthAttachment->imageView);
                }
                if (pRenderingInfo->pDepthAttachment->resolveImageView) {
                    local_pRenderingInfo->pDepthAttachment->resolveImageView =
                        layer_data->Unwrap(pRenderingInfo->pDepthAttachment->resolveImageView);
                }
            }
            if (local_pRenderingInfo->pStencilAttachment) {
                if (pRenderingInfo->pStencilAttachment->imageView) {
                    local_pRenderingInfo->pStencilAttachment->imageView =
                        layer_data->Unwrap(pRenderingInfo->pStencilAttachment->imageView);
                }
                if (pRenderingInfo->pStencilAttachment->resolveImageView) {
                    local_pRenderingInfo->pStencilAttachment->resolveImageView =
                        layer_data->Unwrap(pRenderingInfo->pStencilAttachment->resolveImageView);
                }
            }
            WrapPnextChainHandles(layer_data, local_pRenderingInfo->pNext);
        }
    }
    layer_data->device_dispatch_table.CmdBeginRendering(commandBuffer, (const VkRenderingInfo*)local_pRenderingInfo);
}

void DispatchCmdEndRendering(VkCommandBuffer commandBuffer) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdEndRendering(commandBuffer);
}

void DispatchCmdSetCullMode(VkCommandBuffer commandBuffer, VkCullModeFlags cullMode) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdSetCullMode(commandBuffer, cullMode);
}

void DispatchCmdSetFrontFace(VkCommandBuffer commandBuffer, VkFrontFace frontFace) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdSetFrontFace(commandBuffer, frontFace);
}

void DispatchCmdSetPrimitiveTopology(VkCommandBuffer commandBuffer, VkPrimitiveTopology primitiveTopology) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdSetPrimitiveTopology(commandBuffer, primitiveTopology);
}

void DispatchCmdSetViewportWithCount(VkCommandBuffer commandBuffer, uint32_t viewportCount, const VkViewport* pViewports) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdSetViewportWithCount(commandBuffer, viewportCount, pViewports);
}

void DispatchCmdSetScissorWithCount(VkCommandBuffer commandBuffer, uint32_t scissorCount, const VkRect2D* pScissors) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdSetScissorWithCount(commandBuffer, scissorCount, pScissors);
}

void DispatchCmdBindVertexBuffers2(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount,
                                   const VkBuffer* pBuffers, const VkDeviceSize* pOffsets, const VkDeviceSize* pSizes,
                                   const VkDeviceSize* pStrides) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.CmdBindVertexBuffers2(commandBuffer, firstBinding, bindingCount, pBuffers,
                                                                       pOffsets, pSizes, pStrides);
    VkBuffer var_local_pBuffers[DISPATCH_MAX_STACK_ALLOCATIONS];
    VkBuffer* local_pBuffers = nullptr;
    {
        if (pBuffers) {
            local_pBuffers = bindingCount > DISPATCH_MAX_STACK_ALLOCATIONS ? new VkBuffer[bindingCount] : var_local_pBuffers;
            for (uint32_t index0 = 0; index0 < bindingCount; ++index0) {
                local_pBuffers[index0] = layer_data->Unwrap(pBuffers[index0]);
            }
        }
    }
    layer_data->device_dispatch_table.CmdBindVertexBuffers2(commandBuffer, firstBinding, bindingCount,
                                                            (const VkBuffer*)local_pBuffers, pOffsets, pSizes, pStrides);
    if (local_pBuffers != var_local_pBuffers) delete[] local_pBuffers;
}

void DispatchCmdSetDepthTestEnable(VkCommandBuffer commandBuffer, VkBool32 depthTestEnable) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdSetDepthTestEnable(commandBuffer, depthTestEnable);
}

void DispatchCmdSetDepthWriteEnable(VkCommandBuffer commandBuffer, VkBool32 depthWriteEnable) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdSetDepthWriteEnable(commandBuffer, depthWriteEnable);
}

void DispatchCmdSetDepthCompareOp(VkCommandBuffer commandBuffer, VkCompareOp depthCompareOp) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdSetDepthCompareOp(commandBuffer, depthCompareOp);
}

void DispatchCmdSetDepthBoundsTestEnable(VkCommandBuffer commandBuffer, VkBool32 depthBoundsTestEnable) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdSetDepthBoundsTestEnable(commandBuffer, depthBoundsTestEnable);
}

void DispatchCmdSetStencilTestEnable(VkCommandBuffer commandBuffer, VkBool32 stencilTestEnable) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdSetStencilTestEnable(commandBuffer, stencilTestEnable);
}

void DispatchCmdSetStencilOp(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, VkStencilOp failOp, VkStencilOp passOp,
                             VkStencilOp depthFailOp, VkCompareOp compareOp) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdSetStencilOp(commandBuffer, faceMask, failOp, passOp, depthFailOp, compareOp);
}

void DispatchCmdSetRasterizerDiscardEnable(VkCommandBuffer commandBuffer, VkBool32 rasterizerDiscardEnable) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdSetRasterizerDiscardEnable(commandBuffer, rasterizerDiscardEnable);
}

void DispatchCmdSetDepthBiasEnable(VkCommandBuffer commandBuffer, VkBool32 depthBiasEnable) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdSetDepthBiasEnable(commandBuffer, depthBiasEnable);
}

void DispatchCmdSetPrimitiveRestartEnable(VkCommandBuffer commandBuffer, VkBool32 primitiveRestartEnable) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdSetPrimitiveRestartEnable(commandBuffer, primitiveRestartEnable);
}

void DispatchGetDeviceBufferMemoryRequirements(VkDevice device, const VkDeviceBufferMemoryRequirements* pInfo,
                                               VkMemoryRequirements2* pMemoryRequirements) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);

    layer_data->device_dispatch_table.GetDeviceBufferMemoryRequirements(device, pInfo, pMemoryRequirements);
}

void DispatchGetDeviceImageMemoryRequirements(VkDevice device, const VkDeviceImageMemoryRequirements* pInfo,
                                              VkMemoryRequirements2* pMemoryRequirements) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);

    layer_data->device_dispatch_table.GetDeviceImageMemoryRequirements(device, pInfo, pMemoryRequirements);
}

void DispatchGetDeviceImageSparseMemoryRequirements(VkDevice device, const VkDeviceImageMemoryRequirements* pInfo,
                                                    uint32_t* pSparseMemoryRequirementCount,
                                                    VkSparseImageMemoryRequirements2* pSparseMemoryRequirements) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);

    layer_data->device_dispatch_table.GetDeviceImageSparseMemoryRequirements(device, pInfo, pSparseMemoryRequirementCount,
                                                                             pSparseMemoryRequirements);
}

void DispatchDestroySurfaceKHR(VkInstance instance, VkSurfaceKHR surface, const VkAllocationCallbacks* pAllocator) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(instance), layer_data_map);
    if (!wrap_handles) return layer_data->instance_dispatch_table.DestroySurfaceKHR(instance, surface, pAllocator);

    uint64_t surface_id = CastToUint64(surface);
    auto iter = unique_id_mapping.pop(surface_id);
    if (iter != unique_id_mapping.end()) {
        surface = (VkSurfaceKHR)iter->second;
    } else {
        surface = (VkSurfaceKHR)0;
    }
    layer_data->instance_dispatch_table.DestroySurfaceKHR(instance, surface, pAllocator);
}

VkResult DispatchGetPhysicalDeviceSurfaceSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
                                                    VkSurfaceKHR surface, VkBool32* pSupported) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    if (!wrap_handles)
        return layer_data->instance_dispatch_table.GetPhysicalDeviceSurfaceSupportKHR(physicalDevice, queueFamilyIndex, surface,
                                                                                      pSupported);
    { surface = layer_data->Unwrap(surface); }
    VkResult result = layer_data->instance_dispatch_table.GetPhysicalDeviceSurfaceSupportKHR(physicalDevice, queueFamilyIndex,
                                                                                             surface, pSupported);

    return result;
}

VkResult DispatchGetPhysicalDeviceSurfaceCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                         VkSurfaceCapabilitiesKHR* pSurfaceCapabilities) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    if (!wrap_handles)
        return layer_data->instance_dispatch_table.GetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface,
                                                                                           pSurfaceCapabilities);
    { surface = layer_data->Unwrap(surface); }
    VkResult result =
        layer_data->instance_dispatch_table.GetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, pSurfaceCapabilities);

    return result;
}

VkResult DispatchGetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                    uint32_t* pSurfaceFormatCount, VkSurfaceFormatKHR* pSurfaceFormats) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    if (!wrap_handles)
        return layer_data->instance_dispatch_table.GetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, pSurfaceFormatCount,
                                                                                      pSurfaceFormats);
    { surface = layer_data->Unwrap(surface); }
    VkResult result = layer_data->instance_dispatch_table.GetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface,
                                                                                             pSurfaceFormatCount, pSurfaceFormats);

    return result;
}

VkResult DispatchGetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                         uint32_t* pPresentModeCount, VkPresentModeKHR* pPresentModes) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    if (!wrap_handles)
        return layer_data->instance_dispatch_table.GetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface,
                                                                                           pPresentModeCount, pPresentModes);
    { surface = layer_data->Unwrap(surface); }
    VkResult result = layer_data->instance_dispatch_table.GetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface,
                                                                                                  pPresentModeCount, pPresentModes);

    return result;
}

VkResult DispatchAcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore,
                                     VkFence fence, uint32_t* pImageIndex) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.AcquireNextImageKHR(device, swapchain, timeout, semaphore, fence, pImageIndex);
    {
        swapchain = layer_data->Unwrap(swapchain);
        semaphore = layer_data->Unwrap(semaphore);
        fence = layer_data->Unwrap(fence);
    }
    VkResult result =
        layer_data->device_dispatch_table.AcquireNextImageKHR(device, swapchain, timeout, semaphore, fence, pImageIndex);

    return result;
}

VkResult DispatchGetDeviceGroupPresentCapabilitiesKHR(VkDevice device,
                                                      VkDeviceGroupPresentCapabilitiesKHR* pDeviceGroupPresentCapabilities) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);

    VkResult result =
        layer_data->device_dispatch_table.GetDeviceGroupPresentCapabilitiesKHR(device, pDeviceGroupPresentCapabilities);

    return result;
}

VkResult DispatchGetDeviceGroupSurfacePresentModesKHR(VkDevice device, VkSurfaceKHR surface,
                                                      VkDeviceGroupPresentModeFlagsKHR* pModes) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.GetDeviceGroupSurfacePresentModesKHR(device, surface, pModes);
    { surface = layer_data->Unwrap(surface); }
    VkResult result = layer_data->device_dispatch_table.GetDeviceGroupSurfacePresentModesKHR(device, surface, pModes);

    return result;
}

VkResult DispatchGetPhysicalDevicePresentRectanglesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t* pRectCount,
                                                       VkRect2D* pRects) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    if (!wrap_handles)
        return layer_data->instance_dispatch_table.GetPhysicalDevicePresentRectanglesKHR(physicalDevice, surface, pRectCount,
                                                                                         pRects);
    { surface = layer_data->Unwrap(surface); }
    VkResult result =
        layer_data->instance_dispatch_table.GetPhysicalDevicePresentRectanglesKHR(physicalDevice, surface, pRectCount, pRects);

    return result;
}

VkResult DispatchAcquireNextImage2KHR(VkDevice device, const VkAcquireNextImageInfoKHR* pAcquireInfo, uint32_t* pImageIndex) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.AcquireNextImage2KHR(device, pAcquireInfo, pImageIndex);
    safe_VkAcquireNextImageInfoKHR var_local_pAcquireInfo;
    safe_VkAcquireNextImageInfoKHR* local_pAcquireInfo = nullptr;
    {
        if (pAcquireInfo) {
            local_pAcquireInfo = &var_local_pAcquireInfo;
            local_pAcquireInfo->initialize(pAcquireInfo);

            if (pAcquireInfo->swapchain) {
                local_pAcquireInfo->swapchain = layer_data->Unwrap(pAcquireInfo->swapchain);
            }
            if (pAcquireInfo->semaphore) {
                local_pAcquireInfo->semaphore = layer_data->Unwrap(pAcquireInfo->semaphore);
            }
            if (pAcquireInfo->fence) {
                local_pAcquireInfo->fence = layer_data->Unwrap(pAcquireInfo->fence);
            }
        }
    }
    VkResult result = layer_data->device_dispatch_table.AcquireNextImage2KHR(
        device, (const VkAcquireNextImageInfoKHR*)local_pAcquireInfo, pImageIndex);

    return result;
}

VkResult DispatchCreateDisplayModeKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display,
                                      const VkDisplayModeCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                      VkDisplayModeKHR* pMode) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    if (!wrap_handles)
        return layer_data->instance_dispatch_table.CreateDisplayModeKHR(physicalDevice, display, pCreateInfo, pAllocator, pMode);
    { display = layer_data->Unwrap(display); }
    VkResult result =
        layer_data->instance_dispatch_table.CreateDisplayModeKHR(physicalDevice, display, pCreateInfo, pAllocator, pMode);
    if (VK_SUCCESS == result) {
        *pMode = layer_data->WrapNew(*pMode);
    }
    return result;
}

VkResult DispatchGetDisplayPlaneCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkDisplayModeKHR mode, uint32_t planeIndex,
                                                VkDisplayPlaneCapabilitiesKHR* pCapabilities) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    if (!wrap_handles)
        return layer_data->instance_dispatch_table.GetDisplayPlaneCapabilitiesKHR(physicalDevice, mode, planeIndex, pCapabilities);
    { mode = layer_data->Unwrap(mode); }
    VkResult result =
        layer_data->instance_dispatch_table.GetDisplayPlaneCapabilitiesKHR(physicalDevice, mode, planeIndex, pCapabilities);

    return result;
}

VkResult DispatchCreateDisplayPlaneSurfaceKHR(VkInstance instance, const VkDisplaySurfaceCreateInfoKHR* pCreateInfo,
                                              const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(instance), layer_data_map);
    if (!wrap_handles)
        return layer_data->instance_dispatch_table.CreateDisplayPlaneSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
    safe_VkDisplaySurfaceCreateInfoKHR var_local_pCreateInfo;
    safe_VkDisplaySurfaceCreateInfoKHR* local_pCreateInfo = nullptr;
    {
        if (pCreateInfo) {
            local_pCreateInfo = &var_local_pCreateInfo;
            local_pCreateInfo->initialize(pCreateInfo);

            if (pCreateInfo->displayMode) {
                local_pCreateInfo->displayMode = layer_data->Unwrap(pCreateInfo->displayMode);
            }
        }
    }
    VkResult result = layer_data->instance_dispatch_table.CreateDisplayPlaneSurfaceKHR(
        instance, (const VkDisplaySurfaceCreateInfoKHR*)local_pCreateInfo, pAllocator, pSurface);
    if (VK_SUCCESS == result) {
        *pSurface = layer_data->WrapNew(*pSurface);
    }
    return result;
}
#ifdef VK_USE_PLATFORM_XLIB_KHR

VkResult DispatchCreateXlibSurfaceKHR(VkInstance instance, const VkXlibSurfaceCreateInfoKHR* pCreateInfo,
                                      const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(instance), layer_data_map);
    if (!wrap_handles) return layer_data->instance_dispatch_table.CreateXlibSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);

    VkResult result = layer_data->instance_dispatch_table.CreateXlibSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
    if (VK_SUCCESS == result) {
        *pSurface = layer_data->WrapNew(*pSurface);
    }
    return result;
}
#endif  // VK_USE_PLATFORM_XLIB_KHR
#ifdef VK_USE_PLATFORM_XLIB_KHR

VkBool32 DispatchGetPhysicalDeviceXlibPresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
                                                             Display* dpy, VisualID visualID) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);

    VkBool32 result = layer_data->instance_dispatch_table.GetPhysicalDeviceXlibPresentationSupportKHR(
        physicalDevice, queueFamilyIndex, dpy, visualID);

    return result;
}
#endif  // VK_USE_PLATFORM_XLIB_KHR
#ifdef VK_USE_PLATFORM_XCB_KHR

VkResult DispatchCreateXcbSurfaceKHR(VkInstance instance, const VkXcbSurfaceCreateInfoKHR* pCreateInfo,
                                     const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(instance), layer_data_map);
    if (!wrap_handles) return layer_data->instance_dispatch_table.CreateXcbSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);

    VkResult result = layer_data->instance_dispatch_table.CreateXcbSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
    if (VK_SUCCESS == result) {
        *pSurface = layer_data->WrapNew(*pSurface);
    }
    return result;
}
#endif  // VK_USE_PLATFORM_XCB_KHR
#ifdef VK_USE_PLATFORM_XCB_KHR

VkBool32 DispatchGetPhysicalDeviceXcbPresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
                                                            xcb_connection_t* connection, xcb_visualid_t visual_id) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);

    VkBool32 result = layer_data->instance_dispatch_table.GetPhysicalDeviceXcbPresentationSupportKHR(
        physicalDevice, queueFamilyIndex, connection, visual_id);

    return result;
}
#endif  // VK_USE_PLATFORM_XCB_KHR
#ifdef VK_USE_PLATFORM_WAYLAND_KHR

VkResult DispatchCreateWaylandSurfaceKHR(VkInstance instance, const VkWaylandSurfaceCreateInfoKHR* pCreateInfo,
                                         const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(instance), layer_data_map);
    if (!wrap_handles)
        return layer_data->instance_dispatch_table.CreateWaylandSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);

    VkResult result = layer_data->instance_dispatch_table.CreateWaylandSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
    if (VK_SUCCESS == result) {
        *pSurface = layer_data->WrapNew(*pSurface);
    }
    return result;
}
#endif  // VK_USE_PLATFORM_WAYLAND_KHR
#ifdef VK_USE_PLATFORM_WAYLAND_KHR

VkBool32 DispatchGetPhysicalDeviceWaylandPresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
                                                                struct wl_display* display) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);

    VkBool32 result = layer_data->instance_dispatch_table.GetPhysicalDeviceWaylandPresentationSupportKHR(physicalDevice,
                                                                                                         queueFamilyIndex, display);

    return result;
}
#endif  // VK_USE_PLATFORM_WAYLAND_KHR
#ifdef VK_USE_PLATFORM_ANDROID_KHR

VkResult DispatchCreateAndroidSurfaceKHR(VkInstance instance, const VkAndroidSurfaceCreateInfoKHR* pCreateInfo,
                                         const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(instance), layer_data_map);
    if (!wrap_handles)
        return layer_data->instance_dispatch_table.CreateAndroidSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);

    VkResult result = layer_data->instance_dispatch_table.CreateAndroidSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
    if (VK_SUCCESS == result) {
        *pSurface = layer_data->WrapNew(*pSurface);
    }
    return result;
}
#endif  // VK_USE_PLATFORM_ANDROID_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR

VkResult DispatchCreateWin32SurfaceKHR(VkInstance instance, const VkWin32SurfaceCreateInfoKHR* pCreateInfo,
                                       const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(instance), layer_data_map);
    if (!wrap_handles)
        return layer_data->instance_dispatch_table.CreateWin32SurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);

    VkResult result = layer_data->instance_dispatch_table.CreateWin32SurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
    if (VK_SUCCESS == result) {
        *pSurface = layer_data->WrapNew(*pSurface);
    }
    return result;
}
#endif  // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR

VkBool32 DispatchGetPhysicalDeviceWin32PresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);

    VkBool32 result =
        layer_data->instance_dispatch_table.GetPhysicalDeviceWin32PresentationSupportKHR(physicalDevice, queueFamilyIndex);

    return result;
}
#endif  // VK_USE_PLATFORM_WIN32_KHR

VkResult DispatchGetPhysicalDeviceVideoCapabilitiesKHR(VkPhysicalDevice physicalDevice, const VkVideoProfileInfoKHR* pVideoProfile,
                                                       VkVideoCapabilitiesKHR* pCapabilities) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);

    VkResult result =
        layer_data->instance_dispatch_table.GetPhysicalDeviceVideoCapabilitiesKHR(physicalDevice, pVideoProfile, pCapabilities);

    return result;
}

VkResult DispatchGetPhysicalDeviceVideoFormatPropertiesKHR(VkPhysicalDevice physicalDevice,
                                                           const VkPhysicalDeviceVideoFormatInfoKHR* pVideoFormatInfo,
                                                           uint32_t* pVideoFormatPropertyCount,
                                                           VkVideoFormatPropertiesKHR* pVideoFormatProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);

    VkResult result = layer_data->instance_dispatch_table.GetPhysicalDeviceVideoFormatPropertiesKHR(
        physicalDevice, pVideoFormatInfo, pVideoFormatPropertyCount, pVideoFormatProperties);

    return result;
}

VkResult DispatchCreateVideoSessionKHR(VkDevice device, const VkVideoSessionCreateInfoKHR* pCreateInfo,
                                       const VkAllocationCallbacks* pAllocator, VkVideoSessionKHR* pVideoSession) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.CreateVideoSessionKHR(device, pCreateInfo, pAllocator, pVideoSession);

    VkResult result = layer_data->device_dispatch_table.CreateVideoSessionKHR(device, pCreateInfo, pAllocator, pVideoSession);
    if (VK_SUCCESS == result) {
        *pVideoSession = layer_data->WrapNew(*pVideoSession);
    }
    return result;
}

void DispatchDestroyVideoSessionKHR(VkDevice device, VkVideoSessionKHR videoSession, const VkAllocationCallbacks* pAllocator) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.DestroyVideoSessionKHR(device, videoSession, pAllocator);

    uint64_t videoSession_id = CastToUint64(videoSession);
    auto iter = unique_id_mapping.pop(videoSession_id);
    if (iter != unique_id_mapping.end()) {
        videoSession = (VkVideoSessionKHR)iter->second;
    } else {
        videoSession = (VkVideoSessionKHR)0;
    }
    layer_data->device_dispatch_table.DestroyVideoSessionKHR(device, videoSession, pAllocator);
}

VkResult DispatchGetVideoSessionMemoryRequirementsKHR(VkDevice device, VkVideoSessionKHR videoSession,
                                                      uint32_t* pMemoryRequirementsCount,
                                                      VkVideoSessionMemoryRequirementsKHR* pMemoryRequirements) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.GetVideoSessionMemoryRequirementsKHR(
            device, videoSession, pMemoryRequirementsCount, pMemoryRequirements);
    { videoSession = layer_data->Unwrap(videoSession); }
    VkResult result = layer_data->device_dispatch_table.GetVideoSessionMemoryRequirementsKHR(
        device, videoSession, pMemoryRequirementsCount, pMemoryRequirements);

    return result;
}

VkResult DispatchBindVideoSessionMemoryKHR(VkDevice device, VkVideoSessionKHR videoSession, uint32_t bindSessionMemoryInfoCount,
                                           const VkBindVideoSessionMemoryInfoKHR* pBindSessionMemoryInfos) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.BindVideoSessionMemoryKHR(device, videoSession, bindSessionMemoryInfoCount,
                                                                           pBindSessionMemoryInfos);
    safe_VkBindVideoSessionMemoryInfoKHR* local_pBindSessionMemoryInfos = nullptr;
    {
        videoSession = layer_data->Unwrap(videoSession);
        if (pBindSessionMemoryInfos) {
            local_pBindSessionMemoryInfos = new safe_VkBindVideoSessionMemoryInfoKHR[bindSessionMemoryInfoCount];
            for (uint32_t index0 = 0; index0 < bindSessionMemoryInfoCount; ++index0) {
                local_pBindSessionMemoryInfos[index0].initialize(&pBindSessionMemoryInfos[index0]);

                if (pBindSessionMemoryInfos[index0].memory) {
                    local_pBindSessionMemoryInfos[index0].memory = layer_data->Unwrap(pBindSessionMemoryInfos[index0].memory);
                }
            }
        }
    }
    VkResult result = layer_data->device_dispatch_table.BindVideoSessionMemoryKHR(
        device, videoSession, bindSessionMemoryInfoCount, (const VkBindVideoSessionMemoryInfoKHR*)local_pBindSessionMemoryInfos);
    if (local_pBindSessionMemoryInfos) {
        delete[] local_pBindSessionMemoryInfos;
    }
    return result;
}

VkResult DispatchCreateVideoSessionParametersKHR(VkDevice device, const VkVideoSessionParametersCreateInfoKHR* pCreateInfo,
                                                 const VkAllocationCallbacks* pAllocator,
                                                 VkVideoSessionParametersKHR* pVideoSessionParameters) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.CreateVideoSessionParametersKHR(device, pCreateInfo, pAllocator,
                                                                                 pVideoSessionParameters);
    safe_VkVideoSessionParametersCreateInfoKHR var_local_pCreateInfo;
    safe_VkVideoSessionParametersCreateInfoKHR* local_pCreateInfo = nullptr;
    {
        if (pCreateInfo) {
            local_pCreateInfo = &var_local_pCreateInfo;
            local_pCreateInfo->initialize(pCreateInfo);

            if (pCreateInfo->videoSessionParametersTemplate) {
                local_pCreateInfo->videoSessionParametersTemplate = layer_data->Unwrap(pCreateInfo->videoSessionParametersTemplate);
            }
            if (pCreateInfo->videoSession) {
                local_pCreateInfo->videoSession = layer_data->Unwrap(pCreateInfo->videoSession);
            }
        }
    }
    VkResult result = layer_data->device_dispatch_table.CreateVideoSessionParametersKHR(
        device, (const VkVideoSessionParametersCreateInfoKHR*)local_pCreateInfo, pAllocator, pVideoSessionParameters);
    if (VK_SUCCESS == result) {
        *pVideoSessionParameters = layer_data->WrapNew(*pVideoSessionParameters);
    }
    return result;
}

VkResult DispatchUpdateVideoSessionParametersKHR(VkDevice device, VkVideoSessionParametersKHR videoSessionParameters,
                                                 const VkVideoSessionParametersUpdateInfoKHR* pUpdateInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.UpdateVideoSessionParametersKHR(device, videoSessionParameters, pUpdateInfo);
    { videoSessionParameters = layer_data->Unwrap(videoSessionParameters); }
    VkResult result =
        layer_data->device_dispatch_table.UpdateVideoSessionParametersKHR(device, videoSessionParameters, pUpdateInfo);

    return result;
}

void DispatchDestroyVideoSessionParametersKHR(VkDevice device, VkVideoSessionParametersKHR videoSessionParameters,
                                              const VkAllocationCallbacks* pAllocator) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.DestroyVideoSessionParametersKHR(device, videoSessionParameters, pAllocator);

    uint64_t videoSessionParameters_id = CastToUint64(videoSessionParameters);
    auto iter = unique_id_mapping.pop(videoSessionParameters_id);
    if (iter != unique_id_mapping.end()) {
        videoSessionParameters = (VkVideoSessionParametersKHR)iter->second;
    } else {
        videoSessionParameters = (VkVideoSessionParametersKHR)0;
    }
    layer_data->device_dispatch_table.DestroyVideoSessionParametersKHR(device, videoSessionParameters, pAllocator);
}

void DispatchCmdBeginVideoCodingKHR(VkCommandBuffer commandBuffer, const VkVideoBeginCodingInfoKHR* pBeginInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CmdBeginVideoCodingKHR(commandBuffer, pBeginInfo);
    safe_VkVideoBeginCodingInfoKHR var_local_pBeginInfo;
    safe_VkVideoBeginCodingInfoKHR* local_pBeginInfo = nullptr;
    {
        if (pBeginInfo) {
            local_pBeginInfo = &var_local_pBeginInfo;
            local_pBeginInfo->initialize(pBeginInfo);

            if (pBeginInfo->videoSession) {
                local_pBeginInfo->videoSession = layer_data->Unwrap(pBeginInfo->videoSession);
            }
            if (pBeginInfo->videoSessionParameters) {
                local_pBeginInfo->videoSessionParameters = layer_data->Unwrap(pBeginInfo->videoSessionParameters);
            }
            if (local_pBeginInfo->pReferenceSlots) {
                for (uint32_t index1 = 0; index1 < local_pBeginInfo->referenceSlotCount; ++index1) {
                    if (local_pBeginInfo->pReferenceSlots[index1].pPictureResource) {
                        if (pBeginInfo->pReferenceSlots[index1].pPictureResource->imageViewBinding) {
                            local_pBeginInfo->pReferenceSlots[index1].pPictureResource->imageViewBinding =
                                layer_data->Unwrap(pBeginInfo->pReferenceSlots[index1].pPictureResource->imageViewBinding);
                        }
                    }
                }
            }
        }
    }
    layer_data->device_dispatch_table.CmdBeginVideoCodingKHR(commandBuffer, (const VkVideoBeginCodingInfoKHR*)local_pBeginInfo);
}

void DispatchCmdEndVideoCodingKHR(VkCommandBuffer commandBuffer, const VkVideoEndCodingInfoKHR* pEndCodingInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdEndVideoCodingKHR(commandBuffer, pEndCodingInfo);
}

void DispatchCmdControlVideoCodingKHR(VkCommandBuffer commandBuffer, const VkVideoCodingControlInfoKHR* pCodingControlInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdControlVideoCodingKHR(commandBuffer, pCodingControlInfo);
}

void DispatchCmdDecodeVideoKHR(VkCommandBuffer commandBuffer, const VkVideoDecodeInfoKHR* pDecodeInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CmdDecodeVideoKHR(commandBuffer, pDecodeInfo);
    safe_VkVideoDecodeInfoKHR var_local_pDecodeInfo;
    safe_VkVideoDecodeInfoKHR* local_pDecodeInfo = nullptr;
    {
        if (pDecodeInfo) {
            local_pDecodeInfo = &var_local_pDecodeInfo;
            local_pDecodeInfo->initialize(pDecodeInfo);

            if (pDecodeInfo->srcBuffer) {
                local_pDecodeInfo->srcBuffer = layer_data->Unwrap(pDecodeInfo->srcBuffer);
            }
            if (pDecodeInfo->dstPictureResource.imageViewBinding) {
                local_pDecodeInfo->dstPictureResource.imageViewBinding =
                    layer_data->Unwrap(pDecodeInfo->dstPictureResource.imageViewBinding);
            }
            if (local_pDecodeInfo->pSetupReferenceSlot) {
                if (local_pDecodeInfo->pSetupReferenceSlot->pPictureResource) {
                    if (pDecodeInfo->pSetupReferenceSlot->pPictureResource->imageViewBinding) {
                        local_pDecodeInfo->pSetupReferenceSlot->pPictureResource->imageViewBinding =
                            layer_data->Unwrap(pDecodeInfo->pSetupReferenceSlot->pPictureResource->imageViewBinding);
                    }
                }
            }
            if (local_pDecodeInfo->pReferenceSlots) {
                for (uint32_t index1 = 0; index1 < local_pDecodeInfo->referenceSlotCount; ++index1) {
                    if (local_pDecodeInfo->pReferenceSlots[index1].pPictureResource) {
                        if (pDecodeInfo->pReferenceSlots[index1].pPictureResource->imageViewBinding) {
                            local_pDecodeInfo->pReferenceSlots[index1].pPictureResource->imageViewBinding =
                                layer_data->Unwrap(pDecodeInfo->pReferenceSlots[index1].pPictureResource->imageViewBinding);
                        }
                    }
                }
            }
        }
    }
    layer_data->device_dispatch_table.CmdDecodeVideoKHR(commandBuffer, (const VkVideoDecodeInfoKHR*)local_pDecodeInfo);
}

void DispatchCmdBeginRenderingKHR(VkCommandBuffer commandBuffer, const VkRenderingInfo* pRenderingInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CmdBeginRenderingKHR(commandBuffer, pRenderingInfo);
    safe_VkRenderingInfo var_local_pRenderingInfo;
    safe_VkRenderingInfo* local_pRenderingInfo = nullptr;
    {
        if (pRenderingInfo) {
            local_pRenderingInfo = &var_local_pRenderingInfo;
            local_pRenderingInfo->initialize(pRenderingInfo);
            if (local_pRenderingInfo->pColorAttachments) {
                for (uint32_t index1 = 0; index1 < local_pRenderingInfo->colorAttachmentCount; ++index1) {
                    if (pRenderingInfo->pColorAttachments[index1].imageView) {
                        local_pRenderingInfo->pColorAttachments[index1].imageView =
                            layer_data->Unwrap(pRenderingInfo->pColorAttachments[index1].imageView);
                    }
                    if (pRenderingInfo->pColorAttachments[index1].resolveImageView) {
                        local_pRenderingInfo->pColorAttachments[index1].resolveImageView =
                            layer_data->Unwrap(pRenderingInfo->pColorAttachments[index1].resolveImageView);
                    }
                }
            }
            if (local_pRenderingInfo->pDepthAttachment) {
                if (pRenderingInfo->pDepthAttachment->imageView) {
                    local_pRenderingInfo->pDepthAttachment->imageView =
                        layer_data->Unwrap(pRenderingInfo->pDepthAttachment->imageView);
                }
                if (pRenderingInfo->pDepthAttachment->resolveImageView) {
                    local_pRenderingInfo->pDepthAttachment->resolveImageView =
                        layer_data->Unwrap(pRenderingInfo->pDepthAttachment->resolveImageView);
                }
            }
            if (local_pRenderingInfo->pStencilAttachment) {
                if (pRenderingInfo->pStencilAttachment->imageView) {
                    local_pRenderingInfo->pStencilAttachment->imageView =
                        layer_data->Unwrap(pRenderingInfo->pStencilAttachment->imageView);
                }
                if (pRenderingInfo->pStencilAttachment->resolveImageView) {
                    local_pRenderingInfo->pStencilAttachment->resolveImageView =
                        layer_data->Unwrap(pRenderingInfo->pStencilAttachment->resolveImageView);
                }
            }
            WrapPnextChainHandles(layer_data, local_pRenderingInfo->pNext);
        }
    }
    layer_data->device_dispatch_table.CmdBeginRenderingKHR(commandBuffer, (const VkRenderingInfo*)local_pRenderingInfo);
}

void DispatchCmdEndRenderingKHR(VkCommandBuffer commandBuffer) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdEndRenderingKHR(commandBuffer);
}

void DispatchGetPhysicalDeviceFeatures2KHR(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures2* pFeatures) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);

    layer_data->instance_dispatch_table.GetPhysicalDeviceFeatures2KHR(physicalDevice, pFeatures);
}

void DispatchGetPhysicalDeviceProperties2KHR(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties2* pProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);

    layer_data->instance_dispatch_table.GetPhysicalDeviceProperties2KHR(physicalDevice, pProperties);
}

void DispatchGetPhysicalDeviceFormatProperties2KHR(VkPhysicalDevice physicalDevice, VkFormat format,
                                                   VkFormatProperties2* pFormatProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);

    layer_data->instance_dispatch_table.GetPhysicalDeviceFormatProperties2KHR(physicalDevice, format, pFormatProperties);
}

VkResult DispatchGetPhysicalDeviceImageFormatProperties2KHR(VkPhysicalDevice physicalDevice,
                                                            const VkPhysicalDeviceImageFormatInfo2* pImageFormatInfo,
                                                            VkImageFormatProperties2* pImageFormatProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);

    VkResult result = layer_data->instance_dispatch_table.GetPhysicalDeviceImageFormatProperties2KHR(
        physicalDevice, pImageFormatInfo, pImageFormatProperties);

    return result;
}

void DispatchGetPhysicalDeviceQueueFamilyProperties2KHR(VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount,
                                                        VkQueueFamilyProperties2* pQueueFamilyProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);

    layer_data->instance_dispatch_table.GetPhysicalDeviceQueueFamilyProperties2KHR(physicalDevice, pQueueFamilyPropertyCount,
                                                                                   pQueueFamilyProperties);
}

void DispatchGetPhysicalDeviceMemoryProperties2KHR(VkPhysicalDevice physicalDevice,
                                                   VkPhysicalDeviceMemoryProperties2* pMemoryProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);

    layer_data->instance_dispatch_table.GetPhysicalDeviceMemoryProperties2KHR(physicalDevice, pMemoryProperties);
}

void DispatchGetPhysicalDeviceSparseImageFormatProperties2KHR(VkPhysicalDevice physicalDevice,
                                                              const VkPhysicalDeviceSparseImageFormatInfo2* pFormatInfo,
                                                              uint32_t* pPropertyCount,
                                                              VkSparseImageFormatProperties2* pProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);

    layer_data->instance_dispatch_table.GetPhysicalDeviceSparseImageFormatProperties2KHR(physicalDevice, pFormatInfo,
                                                                                         pPropertyCount, pProperties);
}

void DispatchGetDeviceGroupPeerMemoryFeaturesKHR(VkDevice device, uint32_t heapIndex, uint32_t localDeviceIndex,
                                                 uint32_t remoteDeviceIndex, VkPeerMemoryFeatureFlags* pPeerMemoryFeatures) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);

    layer_data->device_dispatch_table.GetDeviceGroupPeerMemoryFeaturesKHR(device, heapIndex, localDeviceIndex, remoteDeviceIndex,
                                                                          pPeerMemoryFeatures);
}

void DispatchCmdSetDeviceMaskKHR(VkCommandBuffer commandBuffer, uint32_t deviceMask) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdSetDeviceMaskKHR(commandBuffer, deviceMask);
}

void DispatchCmdDispatchBaseKHR(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY, uint32_t baseGroupZ,
                                uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdDispatchBaseKHR(commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX,
                                                         groupCountY, groupCountZ);
}

void DispatchTrimCommandPoolKHR(VkDevice device, VkCommandPool commandPool, VkCommandPoolTrimFlags flags) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.TrimCommandPoolKHR(device, commandPool, flags);
    { commandPool = layer_data->Unwrap(commandPool); }
    layer_data->device_dispatch_table.TrimCommandPoolKHR(device, commandPool, flags);
}

VkResult DispatchEnumeratePhysicalDeviceGroupsKHR(VkInstance instance, uint32_t* pPhysicalDeviceGroupCount,
                                                  VkPhysicalDeviceGroupProperties* pPhysicalDeviceGroupProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(instance), layer_data_map);

    VkResult result = layer_data->instance_dispatch_table.EnumeratePhysicalDeviceGroupsKHR(instance, pPhysicalDeviceGroupCount,
                                                                                           pPhysicalDeviceGroupProperties);

    return result;
}

void DispatchGetPhysicalDeviceExternalBufferPropertiesKHR(VkPhysicalDevice physicalDevice,
                                                          const VkPhysicalDeviceExternalBufferInfo* pExternalBufferInfo,
                                                          VkExternalBufferProperties* pExternalBufferProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);

    layer_data->instance_dispatch_table.GetPhysicalDeviceExternalBufferPropertiesKHR(physicalDevice, pExternalBufferInfo,
                                                                                     pExternalBufferProperties);
}
#ifdef VK_USE_PLATFORM_WIN32_KHR

VkResult DispatchGetMemoryWin32HandleKHR(VkDevice device, const VkMemoryGetWin32HandleInfoKHR* pGetWin32HandleInfo,
                                         HANDLE* pHandle) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.GetMemoryWin32HandleKHR(device, pGetWin32HandleInfo, pHandle);
    safe_VkMemoryGetWin32HandleInfoKHR var_local_pGetWin32HandleInfo;
    safe_VkMemoryGetWin32HandleInfoKHR* local_pGetWin32HandleInfo = nullptr;
    {
        if (pGetWin32HandleInfo) {
            local_pGetWin32HandleInfo = &var_local_pGetWin32HandleInfo;
            local_pGetWin32HandleInfo->initialize(pGetWin32HandleInfo);

            if (pGetWin32HandleInfo->memory) {
                local_pGetWin32HandleInfo->memory = layer_data->Unwrap(pGetWin32HandleInfo->memory);
            }
        }
    }
    VkResult result = layer_data->device_dispatch_table.GetMemoryWin32HandleKHR(
        device, (const VkMemoryGetWin32HandleInfoKHR*)local_pGetWin32HandleInfo, pHandle);

    return result;
}
#endif  // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR

VkResult DispatchGetMemoryWin32HandlePropertiesKHR(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType, HANDLE handle,
                                                   VkMemoryWin32HandlePropertiesKHR* pMemoryWin32HandleProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);

    VkResult result = layer_data->device_dispatch_table.GetMemoryWin32HandlePropertiesKHR(device, handleType, handle,
                                                                                          pMemoryWin32HandleProperties);

    return result;
}
#endif  // VK_USE_PLATFORM_WIN32_KHR

VkResult DispatchGetMemoryFdKHR(VkDevice device, const VkMemoryGetFdInfoKHR* pGetFdInfo, int* pFd) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.GetMemoryFdKHR(device, pGetFdInfo, pFd);
    safe_VkMemoryGetFdInfoKHR var_local_pGetFdInfo;
    safe_VkMemoryGetFdInfoKHR* local_pGetFdInfo = nullptr;
    {
        if (pGetFdInfo) {
            local_pGetFdInfo = &var_local_pGetFdInfo;
            local_pGetFdInfo->initialize(pGetFdInfo);

            if (pGetFdInfo->memory) {
                local_pGetFdInfo->memory = layer_data->Unwrap(pGetFdInfo->memory);
            }
        }
    }
    VkResult result = layer_data->device_dispatch_table.GetMemoryFdKHR(device, (const VkMemoryGetFdInfoKHR*)local_pGetFdInfo, pFd);

    return result;
}

VkResult DispatchGetMemoryFdPropertiesKHR(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType, int fd,
                                          VkMemoryFdPropertiesKHR* pMemoryFdProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);

    VkResult result = layer_data->device_dispatch_table.GetMemoryFdPropertiesKHR(device, handleType, fd, pMemoryFdProperties);

    return result;
}

void DispatchGetPhysicalDeviceExternalSemaphorePropertiesKHR(VkPhysicalDevice physicalDevice,
                                                             const VkPhysicalDeviceExternalSemaphoreInfo* pExternalSemaphoreInfo,
                                                             VkExternalSemaphoreProperties* pExternalSemaphoreProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);

    layer_data->instance_dispatch_table.GetPhysicalDeviceExternalSemaphorePropertiesKHR(physicalDevice, pExternalSemaphoreInfo,
                                                                                        pExternalSemaphoreProperties);
}
#ifdef VK_USE_PLATFORM_WIN32_KHR

VkResult DispatchImportSemaphoreWin32HandleKHR(VkDevice device,
                                               const VkImportSemaphoreWin32HandleInfoKHR* pImportSemaphoreWin32HandleInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.ImportSemaphoreWin32HandleKHR(device, pImportSemaphoreWin32HandleInfo);
    safe_VkImportSemaphoreWin32HandleInfoKHR var_local_pImportSemaphoreWin32HandleInfo;
    safe_VkImportSemaphoreWin32HandleInfoKHR* local_pImportSemaphoreWin32HandleInfo = nullptr;
    {
        if (pImportSemaphoreWin32HandleInfo) {
            local_pImportSemaphoreWin32HandleInfo = &var_local_pImportSemaphoreWin32HandleInfo;
            local_pImportSemaphoreWin32HandleInfo->initialize(pImportSemaphoreWin32HandleInfo);

            if (pImportSemaphoreWin32HandleInfo->semaphore) {
                local_pImportSemaphoreWin32HandleInfo->semaphore = layer_data->Unwrap(pImportSemaphoreWin32HandleInfo->semaphore);
            }
        }
    }
    VkResult result = layer_data->device_dispatch_table.ImportSemaphoreWin32HandleKHR(
        device, (const VkImportSemaphoreWin32HandleInfoKHR*)local_pImportSemaphoreWin32HandleInfo);

    return result;
}
#endif  // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR

VkResult DispatchGetSemaphoreWin32HandleKHR(VkDevice device, const VkSemaphoreGetWin32HandleInfoKHR* pGetWin32HandleInfo,
                                            HANDLE* pHandle) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.GetSemaphoreWin32HandleKHR(device, pGetWin32HandleInfo, pHandle);
    safe_VkSemaphoreGetWin32HandleInfoKHR var_local_pGetWin32HandleInfo;
    safe_VkSemaphoreGetWin32HandleInfoKHR* local_pGetWin32HandleInfo = nullptr;
    {
        if (pGetWin32HandleInfo) {
            local_pGetWin32HandleInfo = &var_local_pGetWin32HandleInfo;
            local_pGetWin32HandleInfo->initialize(pGetWin32HandleInfo);

            if (pGetWin32HandleInfo->semaphore) {
                local_pGetWin32HandleInfo->semaphore = layer_data->Unwrap(pGetWin32HandleInfo->semaphore);
            }
        }
    }
    VkResult result = layer_data->device_dispatch_table.GetSemaphoreWin32HandleKHR(
        device, (const VkSemaphoreGetWin32HandleInfoKHR*)local_pGetWin32HandleInfo, pHandle);

    return result;
}
#endif  // VK_USE_PLATFORM_WIN32_KHR

VkResult DispatchImportSemaphoreFdKHR(VkDevice device, const VkImportSemaphoreFdInfoKHR* pImportSemaphoreFdInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.ImportSemaphoreFdKHR(device, pImportSemaphoreFdInfo);
    safe_VkImportSemaphoreFdInfoKHR var_local_pImportSemaphoreFdInfo;
    safe_VkImportSemaphoreFdInfoKHR* local_pImportSemaphoreFdInfo = nullptr;
    {
        if (pImportSemaphoreFdInfo) {
            local_pImportSemaphoreFdInfo = &var_local_pImportSemaphoreFdInfo;
            local_pImportSemaphoreFdInfo->initialize(pImportSemaphoreFdInfo);

            if (pImportSemaphoreFdInfo->semaphore) {
                local_pImportSemaphoreFdInfo->semaphore = layer_data->Unwrap(pImportSemaphoreFdInfo->semaphore);
            }
        }
    }
    VkResult result = layer_data->device_dispatch_table.ImportSemaphoreFdKHR(
        device, (const VkImportSemaphoreFdInfoKHR*)local_pImportSemaphoreFdInfo);

    return result;
}

VkResult DispatchGetSemaphoreFdKHR(VkDevice device, const VkSemaphoreGetFdInfoKHR* pGetFdInfo, int* pFd) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.GetSemaphoreFdKHR(device, pGetFdInfo, pFd);
    safe_VkSemaphoreGetFdInfoKHR var_local_pGetFdInfo;
    safe_VkSemaphoreGetFdInfoKHR* local_pGetFdInfo = nullptr;
    {
        if (pGetFdInfo) {
            local_pGetFdInfo = &var_local_pGetFdInfo;
            local_pGetFdInfo->initialize(pGetFdInfo);

            if (pGetFdInfo->semaphore) {
                local_pGetFdInfo->semaphore = layer_data->Unwrap(pGetFdInfo->semaphore);
            }
        }
    }
    VkResult result =
        layer_data->device_dispatch_table.GetSemaphoreFdKHR(device, (const VkSemaphoreGetFdInfoKHR*)local_pGetFdInfo, pFd);

    return result;
}

void DispatchCmdPushDescriptorSetKHR(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout,
                                     uint32_t set, uint32_t descriptorWriteCount, const VkWriteDescriptorSet* pDescriptorWrites) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.CmdPushDescriptorSetKHR(commandBuffer, pipelineBindPoint, layout, set,
                                                                         descriptorWriteCount, pDescriptorWrites);
    safe_VkWriteDescriptorSet* local_pDescriptorWrites = nullptr;
    {
        layout = layer_data->Unwrap(layout);
        if (pDescriptorWrites) {
            local_pDescriptorWrites = new safe_VkWriteDescriptorSet[descriptorWriteCount];
            for (uint32_t index0 = 0; index0 < descriptorWriteCount; ++index0) {
                local_pDescriptorWrites[index0].initialize(&pDescriptorWrites[index0]);
                WrapPnextChainHandles(layer_data, local_pDescriptorWrites[index0].pNext);

                if (pDescriptorWrites[index0].dstSet) {
                    local_pDescriptorWrites[index0].dstSet = layer_data->Unwrap(pDescriptorWrites[index0].dstSet);
                }
                if (local_pDescriptorWrites[index0].pImageInfo) {
                    for (uint32_t index1 = 0; index1 < local_pDescriptorWrites[index0].descriptorCount; ++index1) {
                        if (pDescriptorWrites[index0].pImageInfo[index1].sampler) {
                            local_pDescriptorWrites[index0].pImageInfo[index1].sampler =
                                layer_data->Unwrap(pDescriptorWrites[index0].pImageInfo[index1].sampler);
                        }
                        if (pDescriptorWrites[index0].pImageInfo[index1].imageView) {
                            local_pDescriptorWrites[index0].pImageInfo[index1].imageView =
                                layer_data->Unwrap(pDescriptorWrites[index0].pImageInfo[index1].imageView);
                        }
                    }
                }
                if (local_pDescriptorWrites[index0].pBufferInfo) {
                    for (uint32_t index1 = 0; index1 < local_pDescriptorWrites[index0].descriptorCount; ++index1) {
                        if (pDescriptorWrites[index0].pBufferInfo[index1].buffer) {
                            local_pDescriptorWrites[index0].pBufferInfo[index1].buffer =
                                layer_data->Unwrap(pDescriptorWrites[index0].pBufferInfo[index1].buffer);
                        }
                    }
                }
                if (local_pDescriptorWrites[index0].pTexelBufferView) {
                    for (uint32_t index1 = 0; index1 < local_pDescriptorWrites[index0].descriptorCount; ++index1) {
                        local_pDescriptorWrites[index0].pTexelBufferView[index1] =
                            layer_data->Unwrap(local_pDescriptorWrites[index0].pTexelBufferView[index1]);
                    }
                }
            }
        }
    }
    layer_data->device_dispatch_table.CmdPushDescriptorSetKHR(commandBuffer, pipelineBindPoint, layout, set, descriptorWriteCount,
                                                              (const VkWriteDescriptorSet*)local_pDescriptorWrites);
    if (local_pDescriptorWrites) {
        delete[] local_pDescriptorWrites;
    }
}

void DispatchCmdBeginRenderPass2KHR(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                    const VkSubpassBeginInfo* pSubpassBeginInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.CmdBeginRenderPass2KHR(commandBuffer, pRenderPassBegin, pSubpassBeginInfo);
    safe_VkRenderPassBeginInfo var_local_pRenderPassBegin;
    safe_VkRenderPassBeginInfo* local_pRenderPassBegin = nullptr;
    {
        if (pRenderPassBegin) {
            local_pRenderPassBegin = &var_local_pRenderPassBegin;
            local_pRenderPassBegin->initialize(pRenderPassBegin);

            if (pRenderPassBegin->renderPass) {
                local_pRenderPassBegin->renderPass = layer_data->Unwrap(pRenderPassBegin->renderPass);
            }
            if (pRenderPassBegin->framebuffer) {
                local_pRenderPassBegin->framebuffer = layer_data->Unwrap(pRenderPassBegin->framebuffer);
            }
            WrapPnextChainHandles(layer_data, local_pRenderPassBegin->pNext);
        }
    }
    layer_data->device_dispatch_table.CmdBeginRenderPass2KHR(commandBuffer, (const VkRenderPassBeginInfo*)local_pRenderPassBegin,
                                                             pSubpassBeginInfo);
}

void DispatchCmdNextSubpass2KHR(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo* pSubpassBeginInfo,
                                const VkSubpassEndInfo* pSubpassEndInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdNextSubpass2KHR(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo);
}

void DispatchCmdEndRenderPass2KHR(VkCommandBuffer commandBuffer, const VkSubpassEndInfo* pSubpassEndInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdEndRenderPass2KHR(commandBuffer, pSubpassEndInfo);
}

VkResult DispatchGetSwapchainStatusKHR(VkDevice device, VkSwapchainKHR swapchain) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.GetSwapchainStatusKHR(device, swapchain);
    { swapchain = layer_data->Unwrap(swapchain); }
    VkResult result = layer_data->device_dispatch_table.GetSwapchainStatusKHR(device, swapchain);

    return result;
}

void DispatchGetPhysicalDeviceExternalFencePropertiesKHR(VkPhysicalDevice physicalDevice,
                                                         const VkPhysicalDeviceExternalFenceInfo* pExternalFenceInfo,
                                                         VkExternalFenceProperties* pExternalFenceProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);

    layer_data->instance_dispatch_table.GetPhysicalDeviceExternalFencePropertiesKHR(physicalDevice, pExternalFenceInfo,
                                                                                    pExternalFenceProperties);
}
#ifdef VK_USE_PLATFORM_WIN32_KHR

VkResult DispatchImportFenceWin32HandleKHR(VkDevice device, const VkImportFenceWin32HandleInfoKHR* pImportFenceWin32HandleInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.ImportFenceWin32HandleKHR(device, pImportFenceWin32HandleInfo);
    safe_VkImportFenceWin32HandleInfoKHR var_local_pImportFenceWin32HandleInfo;
    safe_VkImportFenceWin32HandleInfoKHR* local_pImportFenceWin32HandleInfo = nullptr;
    {
        if (pImportFenceWin32HandleInfo) {
            local_pImportFenceWin32HandleInfo = &var_local_pImportFenceWin32HandleInfo;
            local_pImportFenceWin32HandleInfo->initialize(pImportFenceWin32HandleInfo);

            if (pImportFenceWin32HandleInfo->fence) {
                local_pImportFenceWin32HandleInfo->fence = layer_data->Unwrap(pImportFenceWin32HandleInfo->fence);
            }
        }
    }
    VkResult result = layer_data->device_dispatch_table.ImportFenceWin32HandleKHR(
        device, (const VkImportFenceWin32HandleInfoKHR*)local_pImportFenceWin32HandleInfo);

    return result;
}
#endif  // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR

VkResult DispatchGetFenceWin32HandleKHR(VkDevice device, const VkFenceGetWin32HandleInfoKHR* pGetWin32HandleInfo, HANDLE* pHandle) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.GetFenceWin32HandleKHR(device, pGetWin32HandleInfo, pHandle);
    safe_VkFenceGetWin32HandleInfoKHR var_local_pGetWin32HandleInfo;
    safe_VkFenceGetWin32HandleInfoKHR* local_pGetWin32HandleInfo = nullptr;
    {
        if (pGetWin32HandleInfo) {
            local_pGetWin32HandleInfo = &var_local_pGetWin32HandleInfo;
            local_pGetWin32HandleInfo->initialize(pGetWin32HandleInfo);

            if (pGetWin32HandleInfo->fence) {
                local_pGetWin32HandleInfo->fence = layer_data->Unwrap(pGetWin32HandleInfo->fence);
            }
        }
    }
    VkResult result = layer_data->device_dispatch_table.GetFenceWin32HandleKHR(
        device, (const VkFenceGetWin32HandleInfoKHR*)local_pGetWin32HandleInfo, pHandle);

    return result;
}
#endif  // VK_USE_PLATFORM_WIN32_KHR

VkResult DispatchImportFenceFdKHR(VkDevice device, const VkImportFenceFdInfoKHR* pImportFenceFdInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.ImportFenceFdKHR(device, pImportFenceFdInfo);
    safe_VkImportFenceFdInfoKHR var_local_pImportFenceFdInfo;
    safe_VkImportFenceFdInfoKHR* local_pImportFenceFdInfo = nullptr;
    {
        if (pImportFenceFdInfo) {
            local_pImportFenceFdInfo = &var_local_pImportFenceFdInfo;
            local_pImportFenceFdInfo->initialize(pImportFenceFdInfo);

            if (pImportFenceFdInfo->fence) {
                local_pImportFenceFdInfo->fence = layer_data->Unwrap(pImportFenceFdInfo->fence);
            }
        }
    }
    VkResult result =
        layer_data->device_dispatch_table.ImportFenceFdKHR(device, (const VkImportFenceFdInfoKHR*)local_pImportFenceFdInfo);

    return result;
}

VkResult DispatchGetFenceFdKHR(VkDevice device, const VkFenceGetFdInfoKHR* pGetFdInfo, int* pFd) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.GetFenceFdKHR(device, pGetFdInfo, pFd);
    safe_VkFenceGetFdInfoKHR var_local_pGetFdInfo;
    safe_VkFenceGetFdInfoKHR* local_pGetFdInfo = nullptr;
    {
        if (pGetFdInfo) {
            local_pGetFdInfo = &var_local_pGetFdInfo;
            local_pGetFdInfo->initialize(pGetFdInfo);

            if (pGetFdInfo->fence) {
                local_pGetFdInfo->fence = layer_data->Unwrap(pGetFdInfo->fence);
            }
        }
    }
    VkResult result = layer_data->device_dispatch_table.GetFenceFdKHR(device, (const VkFenceGetFdInfoKHR*)local_pGetFdInfo, pFd);

    return result;
}

VkResult DispatchEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(
    VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, uint32_t* pCounterCount, VkPerformanceCounterKHR* pCounters,
    VkPerformanceCounterDescriptionKHR* pCounterDescriptions) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);

    VkResult result = layer_data->instance_dispatch_table.EnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(
        physicalDevice, queueFamilyIndex, pCounterCount, pCounters, pCounterDescriptions);

    return result;
}

void DispatchGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR(
    VkPhysicalDevice physicalDevice, const VkQueryPoolPerformanceCreateInfoKHR* pPerformanceQueryCreateInfo, uint32_t* pNumPasses) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);

    layer_data->instance_dispatch_table.GetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR(
        physicalDevice, pPerformanceQueryCreateInfo, pNumPasses);
}

VkResult DispatchAcquireProfilingLockKHR(VkDevice device, const VkAcquireProfilingLockInfoKHR* pInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);

    VkResult result = layer_data->device_dispatch_table.AcquireProfilingLockKHR(device, pInfo);

    return result;
}

void DispatchReleaseProfilingLockKHR(VkDevice device) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);

    layer_data->device_dispatch_table.ReleaseProfilingLockKHR(device);
}

VkResult DispatchGetPhysicalDeviceSurfaceCapabilities2KHR(VkPhysicalDevice physicalDevice,
                                                          const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo,
                                                          VkSurfaceCapabilities2KHR* pSurfaceCapabilities) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    if (!wrap_handles)
        return layer_data->instance_dispatch_table.GetPhysicalDeviceSurfaceCapabilities2KHR(physicalDevice, pSurfaceInfo,
                                                                                            pSurfaceCapabilities);
    safe_VkPhysicalDeviceSurfaceInfo2KHR var_local_pSurfaceInfo;
    safe_VkPhysicalDeviceSurfaceInfo2KHR* local_pSurfaceInfo = nullptr;
    {
        if (pSurfaceInfo) {
            local_pSurfaceInfo = &var_local_pSurfaceInfo;
            local_pSurfaceInfo->initialize(pSurfaceInfo);

            if (pSurfaceInfo->surface) {
                local_pSurfaceInfo->surface = layer_data->Unwrap(pSurfaceInfo->surface);
            }
        }
    }
    VkResult result = layer_data->instance_dispatch_table.GetPhysicalDeviceSurfaceCapabilities2KHR(
        physicalDevice, (const VkPhysicalDeviceSurfaceInfo2KHR*)local_pSurfaceInfo, pSurfaceCapabilities);

    return result;
}

VkResult DispatchGetPhysicalDeviceSurfaceFormats2KHR(VkPhysicalDevice physicalDevice,
                                                     const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo,
                                                     uint32_t* pSurfaceFormatCount, VkSurfaceFormat2KHR* pSurfaceFormats) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    if (!wrap_handles)
        return layer_data->instance_dispatch_table.GetPhysicalDeviceSurfaceFormats2KHR(physicalDevice, pSurfaceInfo,
                                                                                       pSurfaceFormatCount, pSurfaceFormats);
    safe_VkPhysicalDeviceSurfaceInfo2KHR var_local_pSurfaceInfo;
    safe_VkPhysicalDeviceSurfaceInfo2KHR* local_pSurfaceInfo = nullptr;
    {
        if (pSurfaceInfo) {
            local_pSurfaceInfo = &var_local_pSurfaceInfo;
            local_pSurfaceInfo->initialize(pSurfaceInfo);

            if (pSurfaceInfo->surface) {
                local_pSurfaceInfo->surface = layer_data->Unwrap(pSurfaceInfo->surface);
            }
        }
    }
    VkResult result = layer_data->instance_dispatch_table.GetPhysicalDeviceSurfaceFormats2KHR(
        physicalDevice, (const VkPhysicalDeviceSurfaceInfo2KHR*)local_pSurfaceInfo, pSurfaceFormatCount, pSurfaceFormats);

    return result;
}

VkResult DispatchGetDisplayPlaneCapabilities2KHR(VkPhysicalDevice physicalDevice, const VkDisplayPlaneInfo2KHR* pDisplayPlaneInfo,
                                                 VkDisplayPlaneCapabilities2KHR* pCapabilities) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    if (!wrap_handles)
        return layer_data->instance_dispatch_table.GetDisplayPlaneCapabilities2KHR(physicalDevice, pDisplayPlaneInfo,
                                                                                   pCapabilities);
    safe_VkDisplayPlaneInfo2KHR var_local_pDisplayPlaneInfo;
    safe_VkDisplayPlaneInfo2KHR* local_pDisplayPlaneInfo = nullptr;
    {
        if (pDisplayPlaneInfo) {
            local_pDisplayPlaneInfo = &var_local_pDisplayPlaneInfo;
            local_pDisplayPlaneInfo->initialize(pDisplayPlaneInfo);

            if (pDisplayPlaneInfo->mode) {
                local_pDisplayPlaneInfo->mode = layer_data->Unwrap(pDisplayPlaneInfo->mode);
            }
        }
    }
    VkResult result = layer_data->instance_dispatch_table.GetDisplayPlaneCapabilities2KHR(
        physicalDevice, (const VkDisplayPlaneInfo2KHR*)local_pDisplayPlaneInfo, pCapabilities);

    return result;
}

void DispatchGetImageMemoryRequirements2KHR(VkDevice device, const VkImageMemoryRequirementsInfo2* pInfo,
                                            VkMemoryRequirements2* pMemoryRequirements) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.GetImageMemoryRequirements2KHR(device, pInfo, pMemoryRequirements);
    safe_VkImageMemoryRequirementsInfo2 var_local_pInfo;
    safe_VkImageMemoryRequirementsInfo2* local_pInfo = nullptr;
    {
        if (pInfo) {
            local_pInfo = &var_local_pInfo;
            local_pInfo->initialize(pInfo);

            if (pInfo->image) {
                local_pInfo->image = layer_data->Unwrap(pInfo->image);
            }
        }
    }
    layer_data->device_dispatch_table.GetImageMemoryRequirements2KHR(device, (const VkImageMemoryRequirementsInfo2*)local_pInfo,
                                                                     pMemoryRequirements);
}

void DispatchGetBufferMemoryRequirements2KHR(VkDevice device, const VkBufferMemoryRequirementsInfo2* pInfo,
                                             VkMemoryRequirements2* pMemoryRequirements) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.GetBufferMemoryRequirements2KHR(device, pInfo, pMemoryRequirements);
    safe_VkBufferMemoryRequirementsInfo2 var_local_pInfo;
    safe_VkBufferMemoryRequirementsInfo2* local_pInfo = nullptr;
    {
        if (pInfo) {
            local_pInfo = &var_local_pInfo;
            local_pInfo->initialize(pInfo);

            if (pInfo->buffer) {
                local_pInfo->buffer = layer_data->Unwrap(pInfo->buffer);
            }
        }
    }
    layer_data->device_dispatch_table.GetBufferMemoryRequirements2KHR(device, (const VkBufferMemoryRequirementsInfo2*)local_pInfo,
                                                                      pMemoryRequirements);
}

void DispatchGetImageSparseMemoryRequirements2KHR(VkDevice device, const VkImageSparseMemoryRequirementsInfo2* pInfo,
                                                  uint32_t* pSparseMemoryRequirementCount,
                                                  VkSparseImageMemoryRequirements2* pSparseMemoryRequirements) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.GetImageSparseMemoryRequirements2KHR(device, pInfo, pSparseMemoryRequirementCount,
                                                                                      pSparseMemoryRequirements);
    safe_VkImageSparseMemoryRequirementsInfo2 var_local_pInfo;
    safe_VkImageSparseMemoryRequirementsInfo2* local_pInfo = nullptr;
    {
        if (pInfo) {
            local_pInfo = &var_local_pInfo;
            local_pInfo->initialize(pInfo);

            if (pInfo->image) {
                local_pInfo->image = layer_data->Unwrap(pInfo->image);
            }
        }
    }
    layer_data->device_dispatch_table.GetImageSparseMemoryRequirements2KHR(
        device, (const VkImageSparseMemoryRequirementsInfo2*)local_pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
}

VkResult DispatchCreateSamplerYcbcrConversionKHR(VkDevice device, const VkSamplerYcbcrConversionCreateInfo* pCreateInfo,
                                                 const VkAllocationCallbacks* pAllocator,
                                                 VkSamplerYcbcrConversion* pYcbcrConversion) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.CreateSamplerYcbcrConversionKHR(device, pCreateInfo, pAllocator, pYcbcrConversion);

    VkResult result =
        layer_data->device_dispatch_table.CreateSamplerYcbcrConversionKHR(device, pCreateInfo, pAllocator, pYcbcrConversion);
    if (VK_SUCCESS == result) {
        *pYcbcrConversion = layer_data->WrapNew(*pYcbcrConversion);
    }
    return result;
}

void DispatchDestroySamplerYcbcrConversionKHR(VkDevice device, VkSamplerYcbcrConversion ycbcrConversion,
                                              const VkAllocationCallbacks* pAllocator) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.DestroySamplerYcbcrConversionKHR(device, ycbcrConversion, pAllocator);

    uint64_t ycbcrConversion_id = CastToUint64(ycbcrConversion);
    auto iter = unique_id_mapping.pop(ycbcrConversion_id);
    if (iter != unique_id_mapping.end()) {
        ycbcrConversion = (VkSamplerYcbcrConversion)iter->second;
    } else {
        ycbcrConversion = (VkSamplerYcbcrConversion)0;
    }
    layer_data->device_dispatch_table.DestroySamplerYcbcrConversionKHR(device, ycbcrConversion, pAllocator);
}

VkResult DispatchBindBufferMemory2KHR(VkDevice device, uint32_t bindInfoCount, const VkBindBufferMemoryInfo* pBindInfos) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.BindBufferMemory2KHR(device, bindInfoCount, pBindInfos);
    safe_VkBindBufferMemoryInfo* local_pBindInfos = nullptr;
    {
        if (pBindInfos) {
            local_pBindInfos = new safe_VkBindBufferMemoryInfo[bindInfoCount];
            for (uint32_t index0 = 0; index0 < bindInfoCount; ++index0) {
                local_pBindInfos[index0].initialize(&pBindInfos[index0]);

                if (pBindInfos[index0].buffer) {
                    local_pBindInfos[index0].buffer = layer_data->Unwrap(pBindInfos[index0].buffer);
                }
                if (pBindInfos[index0].memory) {
                    local_pBindInfos[index0].memory = layer_data->Unwrap(pBindInfos[index0].memory);
                }
            }
        }
    }
    VkResult result = layer_data->device_dispatch_table.BindBufferMemory2KHR(device, bindInfoCount,
                                                                             (const VkBindBufferMemoryInfo*)local_pBindInfos);
    if (local_pBindInfos) {
        delete[] local_pBindInfos;
    }
    return result;
}

VkResult DispatchBindImageMemory2KHR(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo* pBindInfos) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.BindImageMemory2KHR(device, bindInfoCount, pBindInfos);
    safe_VkBindImageMemoryInfo* local_pBindInfos = nullptr;
    {
        if (pBindInfos) {
            local_pBindInfos = new safe_VkBindImageMemoryInfo[bindInfoCount];
            for (uint32_t index0 = 0; index0 < bindInfoCount; ++index0) {
                local_pBindInfos[index0].initialize(&pBindInfos[index0]);
                WrapPnextChainHandles(layer_data, local_pBindInfos[index0].pNext);

                if (pBindInfos[index0].image) {
                    local_pBindInfos[index0].image = layer_data->Unwrap(pBindInfos[index0].image);
                }
                if (pBindInfos[index0].memory) {
                    local_pBindInfos[index0].memory = layer_data->Unwrap(pBindInfos[index0].memory);
                }
            }
        }
    }
    VkResult result = layer_data->device_dispatch_table.BindImageMemory2KHR(device, bindInfoCount,
                                                                            (const VkBindImageMemoryInfo*)local_pBindInfos);
    if (local_pBindInfos) {
        delete[] local_pBindInfos;
    }
    return result;
}

void DispatchGetDescriptorSetLayoutSupportKHR(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo,
                                              VkDescriptorSetLayoutSupport* pSupport) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.GetDescriptorSetLayoutSupportKHR(device, pCreateInfo, pSupport);
    safe_VkDescriptorSetLayoutCreateInfo var_local_pCreateInfo;
    safe_VkDescriptorSetLayoutCreateInfo* local_pCreateInfo = nullptr;
    {
        if (pCreateInfo) {
            local_pCreateInfo = &var_local_pCreateInfo;
            local_pCreateInfo->initialize(pCreateInfo);
            if (local_pCreateInfo->pBindings) {
                for (uint32_t index1 = 0; index1 < local_pCreateInfo->bindingCount; ++index1) {
                    if (local_pCreateInfo->pBindings[index1].pImmutableSamplers) {
                        for (uint32_t index2 = 0; index2 < local_pCreateInfo->pBindings[index1].descriptorCount; ++index2) {
                            local_pCreateInfo->pBindings[index1].pImmutableSamplers[index2] =
                                layer_data->Unwrap(local_pCreateInfo->pBindings[index1].pImmutableSamplers[index2]);
                        }
                    }
                }
            }
        }
    }
    layer_data->device_dispatch_table.GetDescriptorSetLayoutSupportKHR(
        device, (const VkDescriptorSetLayoutCreateInfo*)local_pCreateInfo, pSupport);
}

void DispatchCmdDrawIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer,
                                     VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.CmdDrawIndirectCountKHR(commandBuffer, buffer, offset, countBuffer,
                                                                         countBufferOffset, maxDrawCount, stride);
    {
        buffer = layer_data->Unwrap(buffer);
        countBuffer = layer_data->Unwrap(countBuffer);
    }
    layer_data->device_dispatch_table.CmdDrawIndirectCountKHR(commandBuffer, buffer, offset, countBuffer, countBufferOffset,
                                                              maxDrawCount, stride);
}

void DispatchCmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                            VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                            uint32_t stride) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.CmdDrawIndexedIndirectCountKHR(commandBuffer, buffer, offset, countBuffer,
                                                                                countBufferOffset, maxDrawCount, stride);
    {
        buffer = layer_data->Unwrap(buffer);
        countBuffer = layer_data->Unwrap(countBuffer);
    }
    layer_data->device_dispatch_table.CmdDrawIndexedIndirectCountKHR(commandBuffer, buffer, offset, countBuffer, countBufferOffset,
                                                                     maxDrawCount, stride);
}

VkResult DispatchGetSemaphoreCounterValueKHR(VkDevice device, VkSemaphore semaphore, uint64_t* pValue) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.GetSemaphoreCounterValueKHR(device, semaphore, pValue);
    { semaphore = layer_data->Unwrap(semaphore); }
    VkResult result = layer_data->device_dispatch_table.GetSemaphoreCounterValueKHR(device, semaphore, pValue);

    return result;
}

VkResult DispatchWaitSemaphoresKHR(VkDevice device, const VkSemaphoreWaitInfo* pWaitInfo, uint64_t timeout) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.WaitSemaphoresKHR(device, pWaitInfo, timeout);
    safe_VkSemaphoreWaitInfo var_local_pWaitInfo;
    safe_VkSemaphoreWaitInfo* local_pWaitInfo = nullptr;
    {
        if (pWaitInfo) {
            local_pWaitInfo = &var_local_pWaitInfo;
            local_pWaitInfo->initialize(pWaitInfo);
            if (local_pWaitInfo->pSemaphores) {
                for (uint32_t index1 = 0; index1 < local_pWaitInfo->semaphoreCount; ++index1) {
                    local_pWaitInfo->pSemaphores[index1] = layer_data->Unwrap(local_pWaitInfo->pSemaphores[index1]);
                }
            }
        }
    }
    VkResult result =
        layer_data->device_dispatch_table.WaitSemaphoresKHR(device, (const VkSemaphoreWaitInfo*)local_pWaitInfo, timeout);

    return result;
}

VkResult DispatchSignalSemaphoreKHR(VkDevice device, const VkSemaphoreSignalInfo* pSignalInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.SignalSemaphoreKHR(device, pSignalInfo);
    safe_VkSemaphoreSignalInfo var_local_pSignalInfo;
    safe_VkSemaphoreSignalInfo* local_pSignalInfo = nullptr;
    {
        if (pSignalInfo) {
            local_pSignalInfo = &var_local_pSignalInfo;
            local_pSignalInfo->initialize(pSignalInfo);

            if (pSignalInfo->semaphore) {
                local_pSignalInfo->semaphore = layer_data->Unwrap(pSignalInfo->semaphore);
            }
        }
    }
    VkResult result = layer_data->device_dispatch_table.SignalSemaphoreKHR(device, (const VkSemaphoreSignalInfo*)local_pSignalInfo);

    return result;
}

VkResult DispatchGetPhysicalDeviceFragmentShadingRatesKHR(VkPhysicalDevice physicalDevice, uint32_t* pFragmentShadingRateCount,
                                                          VkPhysicalDeviceFragmentShadingRateKHR* pFragmentShadingRates) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);

    VkResult result = layer_data->instance_dispatch_table.GetPhysicalDeviceFragmentShadingRatesKHR(
        physicalDevice, pFragmentShadingRateCount, pFragmentShadingRates);

    return result;
}

void DispatchCmdSetFragmentShadingRateKHR(VkCommandBuffer commandBuffer, const VkExtent2D* pFragmentSize,
                                          const VkFragmentShadingRateCombinerOpKHR combinerOps[2]) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdSetFragmentShadingRateKHR(commandBuffer, pFragmentSize, combinerOps);
}

VkResult DispatchWaitForPresentKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t presentId, uint64_t timeout) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.WaitForPresentKHR(device, swapchain, presentId, timeout);
    { swapchain = layer_data->Unwrap(swapchain); }
    VkResult result = layer_data->device_dispatch_table.WaitForPresentKHR(device, swapchain, presentId, timeout);

    return result;
}

VkDeviceAddress DispatchGetBufferDeviceAddressKHR(VkDevice device, const VkBufferDeviceAddressInfo* pInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.GetBufferDeviceAddressKHR(device, pInfo);
    safe_VkBufferDeviceAddressInfo var_local_pInfo;
    safe_VkBufferDeviceAddressInfo* local_pInfo = nullptr;
    {
        if (pInfo) {
            local_pInfo = &var_local_pInfo;
            local_pInfo->initialize(pInfo);

            if (pInfo->buffer) {
                local_pInfo->buffer = layer_data->Unwrap(pInfo->buffer);
            }
        }
    }
    VkDeviceAddress result =
        layer_data->device_dispatch_table.GetBufferDeviceAddressKHR(device, (const VkBufferDeviceAddressInfo*)local_pInfo);

    return result;
}

uint64_t DispatchGetBufferOpaqueCaptureAddressKHR(VkDevice device, const VkBufferDeviceAddressInfo* pInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.GetBufferOpaqueCaptureAddressKHR(device, pInfo);
    safe_VkBufferDeviceAddressInfo var_local_pInfo;
    safe_VkBufferDeviceAddressInfo* local_pInfo = nullptr;
    {
        if (pInfo) {
            local_pInfo = &var_local_pInfo;
            local_pInfo->initialize(pInfo);

            if (pInfo->buffer) {
                local_pInfo->buffer = layer_data->Unwrap(pInfo->buffer);
            }
        }
    }
    uint64_t result =
        layer_data->device_dispatch_table.GetBufferOpaqueCaptureAddressKHR(device, (const VkBufferDeviceAddressInfo*)local_pInfo);

    return result;
}

uint64_t DispatchGetDeviceMemoryOpaqueCaptureAddressKHR(VkDevice device, const VkDeviceMemoryOpaqueCaptureAddressInfo* pInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.GetDeviceMemoryOpaqueCaptureAddressKHR(device, pInfo);
    safe_VkDeviceMemoryOpaqueCaptureAddressInfo var_local_pInfo;
    safe_VkDeviceMemoryOpaqueCaptureAddressInfo* local_pInfo = nullptr;
    {
        if (pInfo) {
            local_pInfo = &var_local_pInfo;
            local_pInfo->initialize(pInfo);

            if (pInfo->memory) {
                local_pInfo->memory = layer_data->Unwrap(pInfo->memory);
            }
        }
    }
    uint64_t result = layer_data->device_dispatch_table.GetDeviceMemoryOpaqueCaptureAddressKHR(
        device, (const VkDeviceMemoryOpaqueCaptureAddressInfo*)local_pInfo);

    return result;
}

VkResult DispatchCreateDeferredOperationKHR(VkDevice device, const VkAllocationCallbacks* pAllocator,
                                            VkDeferredOperationKHR* pDeferredOperation) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CreateDeferredOperationKHR(device, pAllocator, pDeferredOperation);

    VkResult result = layer_data->device_dispatch_table.CreateDeferredOperationKHR(device, pAllocator, pDeferredOperation);
    if (VK_SUCCESS == result) {
        *pDeferredOperation = layer_data->WrapNew(*pDeferredOperation);
    }
    return result;
}

void DispatchDestroyDeferredOperationKHR(VkDevice device, VkDeferredOperationKHR operation,
                                         const VkAllocationCallbacks* pAllocator) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.DestroyDeferredOperationKHR(device, operation, pAllocator);

    uint64_t operation_id = CastToUint64(operation);
    auto iter = unique_id_mapping.pop(operation_id);
    if (iter != unique_id_mapping.end()) {
        operation = (VkDeferredOperationKHR)iter->second;
    } else {
        operation = (VkDeferredOperationKHR)0;
    }
    layer_data->device_dispatch_table.DestroyDeferredOperationKHR(device, operation, pAllocator);
}

uint32_t DispatchGetDeferredOperationMaxConcurrencyKHR(VkDevice device, VkDeferredOperationKHR operation) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.GetDeferredOperationMaxConcurrencyKHR(device, operation);
    { operation = layer_data->Unwrap(operation); }
    uint32_t result = layer_data->device_dispatch_table.GetDeferredOperationMaxConcurrencyKHR(device, operation);

    return result;
}

VkResult DispatchGetPipelineExecutablePropertiesKHR(VkDevice device, const VkPipelineInfoKHR* pPipelineInfo,
                                                    uint32_t* pExecutableCount, VkPipelineExecutablePropertiesKHR* pProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.GetPipelineExecutablePropertiesKHR(device, pPipelineInfo, pExecutableCount,
                                                                                    pProperties);
    safe_VkPipelineInfoKHR var_local_pPipelineInfo;
    safe_VkPipelineInfoKHR* local_pPipelineInfo = nullptr;
    {
        if (pPipelineInfo) {
            local_pPipelineInfo = &var_local_pPipelineInfo;
            local_pPipelineInfo->initialize(pPipelineInfo);

            if (pPipelineInfo->pipeline) {
                local_pPipelineInfo->pipeline = layer_data->Unwrap(pPipelineInfo->pipeline);
            }
        }
    }
    VkResult result = layer_data->device_dispatch_table.GetPipelineExecutablePropertiesKHR(
        device, (const VkPipelineInfoKHR*)local_pPipelineInfo, pExecutableCount, pProperties);

    return result;
}

VkResult DispatchGetPipelineExecutableStatisticsKHR(VkDevice device, const VkPipelineExecutableInfoKHR* pExecutableInfo,
                                                    uint32_t* pStatisticCount, VkPipelineExecutableStatisticKHR* pStatistics) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.GetPipelineExecutableStatisticsKHR(device, pExecutableInfo, pStatisticCount,
                                                                                    pStatistics);
    safe_VkPipelineExecutableInfoKHR var_local_pExecutableInfo;
    safe_VkPipelineExecutableInfoKHR* local_pExecutableInfo = nullptr;
    {
        if (pExecutableInfo) {
            local_pExecutableInfo = &var_local_pExecutableInfo;
            local_pExecutableInfo->initialize(pExecutableInfo);

            if (pExecutableInfo->pipeline) {
                local_pExecutableInfo->pipeline = layer_data->Unwrap(pExecutableInfo->pipeline);
            }
        }
    }
    VkResult result = layer_data->device_dispatch_table.GetPipelineExecutableStatisticsKHR(
        device, (const VkPipelineExecutableInfoKHR*)local_pExecutableInfo, pStatisticCount, pStatistics);

    return result;
}

VkResult DispatchGetPipelineExecutableInternalRepresentationsKHR(
    VkDevice device, const VkPipelineExecutableInfoKHR* pExecutableInfo, uint32_t* pInternalRepresentationCount,
    VkPipelineExecutableInternalRepresentationKHR* pInternalRepresentations) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.GetPipelineExecutableInternalRepresentationsKHR(
            device, pExecutableInfo, pInternalRepresentationCount, pInternalRepresentations);
    safe_VkPipelineExecutableInfoKHR var_local_pExecutableInfo;
    safe_VkPipelineExecutableInfoKHR* local_pExecutableInfo = nullptr;
    {
        if (pExecutableInfo) {
            local_pExecutableInfo = &var_local_pExecutableInfo;
            local_pExecutableInfo->initialize(pExecutableInfo);

            if (pExecutableInfo->pipeline) {
                local_pExecutableInfo->pipeline = layer_data->Unwrap(pExecutableInfo->pipeline);
            }
        }
    }
    VkResult result = layer_data->device_dispatch_table.GetPipelineExecutableInternalRepresentationsKHR(
        device, (const VkPipelineExecutableInfoKHR*)local_pExecutableInfo, pInternalRepresentationCount, pInternalRepresentations);

    return result;
}

VkResult DispatchMapMemory2KHR(VkDevice device, const VkMemoryMapInfoKHR* pMemoryMapInfo, void** ppData) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.MapMemory2KHR(device, pMemoryMapInfo, ppData);
    safe_VkMemoryMapInfoKHR var_local_pMemoryMapInfo;
    safe_VkMemoryMapInfoKHR* local_pMemoryMapInfo = nullptr;
    {
        if (pMemoryMapInfo) {
            local_pMemoryMapInfo = &var_local_pMemoryMapInfo;
            local_pMemoryMapInfo->initialize(pMemoryMapInfo);

            if (pMemoryMapInfo->memory) {
                local_pMemoryMapInfo->memory = layer_data->Unwrap(pMemoryMapInfo->memory);
            }
        }
    }
    VkResult result =
        layer_data->device_dispatch_table.MapMemory2KHR(device, (const VkMemoryMapInfoKHR*)local_pMemoryMapInfo, ppData);

    return result;
}

VkResult DispatchUnmapMemory2KHR(VkDevice device, const VkMemoryUnmapInfoKHR* pMemoryUnmapInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.UnmapMemory2KHR(device, pMemoryUnmapInfo);
    safe_VkMemoryUnmapInfoKHR var_local_pMemoryUnmapInfo;
    safe_VkMemoryUnmapInfoKHR* local_pMemoryUnmapInfo = nullptr;
    {
        if (pMemoryUnmapInfo) {
            local_pMemoryUnmapInfo = &var_local_pMemoryUnmapInfo;
            local_pMemoryUnmapInfo->initialize(pMemoryUnmapInfo);

            if (pMemoryUnmapInfo->memory) {
                local_pMemoryUnmapInfo->memory = layer_data->Unwrap(pMemoryUnmapInfo->memory);
            }
        }
    }
    VkResult result =
        layer_data->device_dispatch_table.UnmapMemory2KHR(device, (const VkMemoryUnmapInfoKHR*)local_pMemoryUnmapInfo);

    return result;
}
#ifdef VK_ENABLE_BETA_EXTENSIONS

VkResult DispatchGetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceVideoEncodeQualityLevelInfoKHR* pQualityLevelInfo,
    VkVideoEncodeQualityLevelPropertiesKHR* pQualityLevelProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);

    VkResult result = layer_data->instance_dispatch_table.GetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR(
        physicalDevice, pQualityLevelInfo, pQualityLevelProperties);

    return result;
}
#endif  // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS

VkResult DispatchGetEncodedVideoSessionParametersKHR(VkDevice device,
                                                     const VkVideoEncodeSessionParametersGetInfoKHR* pVideoSessionParametersInfo,
                                                     VkVideoEncodeSessionParametersFeedbackInfoKHR* pFeedbackInfo,
                                                     size_t* pDataSize, void* pData) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.GetEncodedVideoSessionParametersKHR(device, pVideoSessionParametersInfo,
                                                                                     pFeedbackInfo, pDataSize, pData);
    safe_VkVideoEncodeSessionParametersGetInfoKHR var_local_pVideoSessionParametersInfo;
    safe_VkVideoEncodeSessionParametersGetInfoKHR* local_pVideoSessionParametersInfo = nullptr;
    {
        if (pVideoSessionParametersInfo) {
            local_pVideoSessionParametersInfo = &var_local_pVideoSessionParametersInfo;
            local_pVideoSessionParametersInfo->initialize(pVideoSessionParametersInfo);

            if (pVideoSessionParametersInfo->videoSessionParameters) {
                local_pVideoSessionParametersInfo->videoSessionParameters =
                    layer_data->Unwrap(pVideoSessionParametersInfo->videoSessionParameters);
            }
        }
    }
    VkResult result = layer_data->device_dispatch_table.GetEncodedVideoSessionParametersKHR(
        device, (const VkVideoEncodeSessionParametersGetInfoKHR*)local_pVideoSessionParametersInfo, pFeedbackInfo, pDataSize,
        pData);

    return result;
}
#endif  // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS

void DispatchCmdEncodeVideoKHR(VkCommandBuffer commandBuffer, const VkVideoEncodeInfoKHR* pEncodeInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CmdEncodeVideoKHR(commandBuffer, pEncodeInfo);
    safe_VkVideoEncodeInfoKHR var_local_pEncodeInfo;
    safe_VkVideoEncodeInfoKHR* local_pEncodeInfo = nullptr;
    {
        if (pEncodeInfo) {
            local_pEncodeInfo = &var_local_pEncodeInfo;
            local_pEncodeInfo->initialize(pEncodeInfo);

            if (pEncodeInfo->dstBuffer) {
                local_pEncodeInfo->dstBuffer = layer_data->Unwrap(pEncodeInfo->dstBuffer);
            }
            if (pEncodeInfo->srcPictureResource.imageViewBinding) {
                local_pEncodeInfo->srcPictureResource.imageViewBinding =
                    layer_data->Unwrap(pEncodeInfo->srcPictureResource.imageViewBinding);
            }
            if (local_pEncodeInfo->pSetupReferenceSlot) {
                if (local_pEncodeInfo->pSetupReferenceSlot->pPictureResource) {
                    if (pEncodeInfo->pSetupReferenceSlot->pPictureResource->imageViewBinding) {
                        local_pEncodeInfo->pSetupReferenceSlot->pPictureResource->imageViewBinding =
                            layer_data->Unwrap(pEncodeInfo->pSetupReferenceSlot->pPictureResource->imageViewBinding);
                    }
                }
            }
            if (local_pEncodeInfo->pReferenceSlots) {
                for (uint32_t index1 = 0; index1 < local_pEncodeInfo->referenceSlotCount; ++index1) {
                    if (local_pEncodeInfo->pReferenceSlots[index1].pPictureResource) {
                        if (pEncodeInfo->pReferenceSlots[index1].pPictureResource->imageViewBinding) {
                            local_pEncodeInfo->pReferenceSlots[index1].pPictureResource->imageViewBinding =
                                layer_data->Unwrap(pEncodeInfo->pReferenceSlots[index1].pPictureResource->imageViewBinding);
                        }
                    }
                }
            }
        }
    }
    layer_data->device_dispatch_table.CmdEncodeVideoKHR(commandBuffer, (const VkVideoEncodeInfoKHR*)local_pEncodeInfo);
}
#endif  // VK_ENABLE_BETA_EXTENSIONS

void DispatchCmdSetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event, const VkDependencyInfo* pDependencyInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CmdSetEvent2KHR(commandBuffer, event, pDependencyInfo);
    safe_VkDependencyInfo var_local_pDependencyInfo;
    safe_VkDependencyInfo* local_pDependencyInfo = nullptr;
    {
        event = layer_data->Unwrap(event);
        if (pDependencyInfo) {
            local_pDependencyInfo = &var_local_pDependencyInfo;
            local_pDependencyInfo->initialize(pDependencyInfo);
            if (local_pDependencyInfo->pBufferMemoryBarriers) {
                for (uint32_t index1 = 0; index1 < local_pDependencyInfo->bufferMemoryBarrierCount; ++index1) {
                    if (pDependencyInfo->pBufferMemoryBarriers[index1].buffer) {
                        local_pDependencyInfo->pBufferMemoryBarriers[index1].buffer =
                            layer_data->Unwrap(pDependencyInfo->pBufferMemoryBarriers[index1].buffer);
                    }
                }
            }
            if (local_pDependencyInfo->pImageMemoryBarriers) {
                for (uint32_t index1 = 0; index1 < local_pDependencyInfo->imageMemoryBarrierCount; ++index1) {
                    if (pDependencyInfo->pImageMemoryBarriers[index1].image) {
                        local_pDependencyInfo->pImageMemoryBarriers[index1].image =
                            layer_data->Unwrap(pDependencyInfo->pImageMemoryBarriers[index1].image);
                    }
                }
            }
        }
    }
    layer_data->device_dispatch_table.CmdSetEvent2KHR(commandBuffer, event, (const VkDependencyInfo*)local_pDependencyInfo);
}

void DispatchCmdResetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags2 stageMask) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CmdResetEvent2KHR(commandBuffer, event, stageMask);
    { event = layer_data->Unwrap(event); }
    layer_data->device_dispatch_table.CmdResetEvent2KHR(commandBuffer, event, stageMask);
}

void DispatchCmdWaitEvents2KHR(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                               const VkDependencyInfo* pDependencyInfos) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.CmdWaitEvents2KHR(commandBuffer, eventCount, pEvents, pDependencyInfos);
    VkEvent var_local_pEvents[DISPATCH_MAX_STACK_ALLOCATIONS];
    VkEvent* local_pEvents = nullptr;
    safe_VkDependencyInfo* local_pDependencyInfos = nullptr;
    {
        if (pEvents) {
            local_pEvents = eventCount > DISPATCH_MAX_STACK_ALLOCATIONS ? new VkEvent[eventCount] : var_local_pEvents;
            for (uint32_t index0 = 0; index0 < eventCount; ++index0) {
                local_pEvents[index0] = layer_data->Unwrap(pEvents[index0]);
            }
        }
        if (pDependencyInfos) {
            local_pDependencyInfos = new safe_VkDependencyInfo[eventCount];
            for (uint32_t index0 = 0; index0 < eventCount; ++index0) {
                local_pDependencyInfos[index0].initialize(&pDependencyInfos[index0]);
                if (local_pDependencyInfos[index0].pBufferMemoryBarriers) {
                    for (uint32_t index1 = 0; index1 < local_pDependencyInfos[index0].bufferMemoryBarrierCount; ++index1) {
                        if (pDependencyInfos[index0].pBufferMemoryBarriers[index1].buffer) {
                            local_pDependencyInfos[index0].pBufferMemoryBarriers[index1].buffer =
                                layer_data->Unwrap(pDependencyInfos[index0].pBufferMemoryBarriers[index1].buffer);
                        }
                    }
                }
                if (local_pDependencyInfos[index0].pImageMemoryBarriers) {
                    for (uint32_t index1 = 0; index1 < local_pDependencyInfos[index0].imageMemoryBarrierCount; ++index1) {
                        if (pDependencyInfos[index0].pImageMemoryBarriers[index1].image) {
                            local_pDependencyInfos[index0].pImageMemoryBarriers[index1].image =
                                layer_data->Unwrap(pDependencyInfos[index0].pImageMemoryBarriers[index1].image);
                        }
                    }
                }
            }
        }
    }
    layer_data->device_dispatch_table.CmdWaitEvents2KHR(commandBuffer, eventCount, (const VkEvent*)local_pEvents,
                                                        (const VkDependencyInfo*)local_pDependencyInfos);
    if (local_pEvents != var_local_pEvents) delete[] local_pEvents;
    if (local_pDependencyInfos) {
        delete[] local_pDependencyInfos;
    }
}

void DispatchCmdPipelineBarrier2KHR(VkCommandBuffer commandBuffer, const VkDependencyInfo* pDependencyInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CmdPipelineBarrier2KHR(commandBuffer, pDependencyInfo);
    safe_VkDependencyInfo var_local_pDependencyInfo;
    safe_VkDependencyInfo* local_pDependencyInfo = nullptr;
    {
        if (pDependencyInfo) {
            local_pDependencyInfo = &var_local_pDependencyInfo;
            local_pDependencyInfo->initialize(pDependencyInfo);
            if (local_pDependencyInfo->pBufferMemoryBarriers) {
                for (uint32_t index1 = 0; index1 < local_pDependencyInfo->bufferMemoryBarrierCount; ++index1) {
                    if (pDependencyInfo->pBufferMemoryBarriers[index1].buffer) {
                        local_pDependencyInfo->pBufferMemoryBarriers[index1].buffer =
                            layer_data->Unwrap(pDependencyInfo->pBufferMemoryBarriers[index1].buffer);
                    }
                }
            }
            if (local_pDependencyInfo->pImageMemoryBarriers) {
                for (uint32_t index1 = 0; index1 < local_pDependencyInfo->imageMemoryBarrierCount; ++index1) {
                    if (pDependencyInfo->pImageMemoryBarriers[index1].image) {
                        local_pDependencyInfo->pImageMemoryBarriers[index1].image =
                            layer_data->Unwrap(pDependencyInfo->pImageMemoryBarriers[index1].image);
                    }
                }
            }
        }
    }
    layer_data->device_dispatch_table.CmdPipelineBarrier2KHR(commandBuffer, (const VkDependencyInfo*)local_pDependencyInfo);
}

void DispatchCmdWriteTimestamp2KHR(VkCommandBuffer commandBuffer, VkPipelineStageFlags2 stage, VkQueryPool queryPool,
                                   uint32_t query) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CmdWriteTimestamp2KHR(commandBuffer, stage, queryPool, query);
    { queryPool = layer_data->Unwrap(queryPool); }
    layer_data->device_dispatch_table.CmdWriteTimestamp2KHR(commandBuffer, stage, queryPool, query);
}

VkResult DispatchQueueSubmit2KHR(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2* pSubmits, VkFence fence) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(queue), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.QueueSubmit2KHR(queue, submitCount, pSubmits, fence);
    safe_VkSubmitInfo2* local_pSubmits = nullptr;
    {
        if (pSubmits) {
            local_pSubmits = new safe_VkSubmitInfo2[submitCount];
            for (uint32_t index0 = 0; index0 < submitCount; ++index0) {
                local_pSubmits[index0].initialize(&pSubmits[index0]);
                WrapPnextChainHandles(layer_data, local_pSubmits[index0].pNext);
                if (local_pSubmits[index0].pWaitSemaphoreInfos) {
                    for (uint32_t index1 = 0; index1 < local_pSubmits[index0].waitSemaphoreInfoCount; ++index1) {
                        if (pSubmits[index0].pWaitSemaphoreInfos[index1].semaphore) {
                            local_pSubmits[index0].pWaitSemaphoreInfos[index1].semaphore =
                                layer_data->Unwrap(pSubmits[index0].pWaitSemaphoreInfos[index1].semaphore);
                        }
                    }
                }
                if (local_pSubmits[index0].pSignalSemaphoreInfos) {
                    for (uint32_t index1 = 0; index1 < local_pSubmits[index0].signalSemaphoreInfoCount; ++index1) {
                        if (pSubmits[index0].pSignalSemaphoreInfos[index1].semaphore) {
                            local_pSubmits[index0].pSignalSemaphoreInfos[index1].semaphore =
                                layer_data->Unwrap(pSubmits[index0].pSignalSemaphoreInfos[index1].semaphore);
                        }
                    }
                }
            }
        }
        fence = layer_data->Unwrap(fence);
    }
    VkResult result =
        layer_data->device_dispatch_table.QueueSubmit2KHR(queue, submitCount, (const VkSubmitInfo2*)local_pSubmits, fence);
    if (local_pSubmits) {
        delete[] local_pSubmits;
    }
    return result;
}

void DispatchCmdWriteBufferMarker2AMD(VkCommandBuffer commandBuffer, VkPipelineStageFlags2 stage, VkBuffer dstBuffer,
                                      VkDeviceSize dstOffset, uint32_t marker) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.CmdWriteBufferMarker2AMD(commandBuffer, stage, dstBuffer, dstOffset, marker);
    { dstBuffer = layer_data->Unwrap(dstBuffer); }
    layer_data->device_dispatch_table.CmdWriteBufferMarker2AMD(commandBuffer, stage, dstBuffer, dstOffset, marker);
}

void DispatchGetQueueCheckpointData2NV(VkQueue queue, uint32_t* pCheckpointDataCount, VkCheckpointData2NV* pCheckpointData) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(queue), layer_data_map);

    layer_data->device_dispatch_table.GetQueueCheckpointData2NV(queue, pCheckpointDataCount, pCheckpointData);
}

void DispatchCmdCopyBuffer2KHR(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2* pCopyBufferInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CmdCopyBuffer2KHR(commandBuffer, pCopyBufferInfo);
    safe_VkCopyBufferInfo2 var_local_pCopyBufferInfo;
    safe_VkCopyBufferInfo2* local_pCopyBufferInfo = nullptr;
    {
        if (pCopyBufferInfo) {
            local_pCopyBufferInfo = &var_local_pCopyBufferInfo;
            local_pCopyBufferInfo->initialize(pCopyBufferInfo);

            if (pCopyBufferInfo->srcBuffer) {
                local_pCopyBufferInfo->srcBuffer = layer_data->Unwrap(pCopyBufferInfo->srcBuffer);
            }
            if (pCopyBufferInfo->dstBuffer) {
                local_pCopyBufferInfo->dstBuffer = layer_data->Unwrap(pCopyBufferInfo->dstBuffer);
            }
        }
    }
    layer_data->device_dispatch_table.CmdCopyBuffer2KHR(commandBuffer, (const VkCopyBufferInfo2*)local_pCopyBufferInfo);
}

void DispatchCmdCopyImage2KHR(VkCommandBuffer commandBuffer, const VkCopyImageInfo2* pCopyImageInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CmdCopyImage2KHR(commandBuffer, pCopyImageInfo);
    safe_VkCopyImageInfo2 var_local_pCopyImageInfo;
    safe_VkCopyImageInfo2* local_pCopyImageInfo = nullptr;
    {
        if (pCopyImageInfo) {
            local_pCopyImageInfo = &var_local_pCopyImageInfo;
            local_pCopyImageInfo->initialize(pCopyImageInfo);

            if (pCopyImageInfo->srcImage) {
                local_pCopyImageInfo->srcImage = layer_data->Unwrap(pCopyImageInfo->srcImage);
            }
            if (pCopyImageInfo->dstImage) {
                local_pCopyImageInfo->dstImage = layer_data->Unwrap(pCopyImageInfo->dstImage);
            }
        }
    }
    layer_data->device_dispatch_table.CmdCopyImage2KHR(commandBuffer, (const VkCopyImageInfo2*)local_pCopyImageInfo);
}

void DispatchCmdCopyBufferToImage2KHR(VkCommandBuffer commandBuffer, const VkCopyBufferToImageInfo2* pCopyBufferToImageInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CmdCopyBufferToImage2KHR(commandBuffer, pCopyBufferToImageInfo);
    safe_VkCopyBufferToImageInfo2 var_local_pCopyBufferToImageInfo;
    safe_VkCopyBufferToImageInfo2* local_pCopyBufferToImageInfo = nullptr;
    {
        if (pCopyBufferToImageInfo) {
            local_pCopyBufferToImageInfo = &var_local_pCopyBufferToImageInfo;
            local_pCopyBufferToImageInfo->initialize(pCopyBufferToImageInfo);

            if (pCopyBufferToImageInfo->srcBuffer) {
                local_pCopyBufferToImageInfo->srcBuffer = layer_data->Unwrap(pCopyBufferToImageInfo->srcBuffer);
            }
            if (pCopyBufferToImageInfo->dstImage) {
                local_pCopyBufferToImageInfo->dstImage = layer_data->Unwrap(pCopyBufferToImageInfo->dstImage);
            }
        }
    }
    layer_data->device_dispatch_table.CmdCopyBufferToImage2KHR(commandBuffer,
                                                               (const VkCopyBufferToImageInfo2*)local_pCopyBufferToImageInfo);
}

void DispatchCmdCopyImageToBuffer2KHR(VkCommandBuffer commandBuffer, const VkCopyImageToBufferInfo2* pCopyImageToBufferInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CmdCopyImageToBuffer2KHR(commandBuffer, pCopyImageToBufferInfo);
    safe_VkCopyImageToBufferInfo2 var_local_pCopyImageToBufferInfo;
    safe_VkCopyImageToBufferInfo2* local_pCopyImageToBufferInfo = nullptr;
    {
        if (pCopyImageToBufferInfo) {
            local_pCopyImageToBufferInfo = &var_local_pCopyImageToBufferInfo;
            local_pCopyImageToBufferInfo->initialize(pCopyImageToBufferInfo);

            if (pCopyImageToBufferInfo->srcImage) {
                local_pCopyImageToBufferInfo->srcImage = layer_data->Unwrap(pCopyImageToBufferInfo->srcImage);
            }
            if (pCopyImageToBufferInfo->dstBuffer) {
                local_pCopyImageToBufferInfo->dstBuffer = layer_data->Unwrap(pCopyImageToBufferInfo->dstBuffer);
            }
        }
    }
    layer_data->device_dispatch_table.CmdCopyImageToBuffer2KHR(commandBuffer,
                                                               (const VkCopyImageToBufferInfo2*)local_pCopyImageToBufferInfo);
}

void DispatchCmdBlitImage2KHR(VkCommandBuffer commandBuffer, const VkBlitImageInfo2* pBlitImageInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CmdBlitImage2KHR(commandBuffer, pBlitImageInfo);
    safe_VkBlitImageInfo2 var_local_pBlitImageInfo;
    safe_VkBlitImageInfo2* local_pBlitImageInfo = nullptr;
    {
        if (pBlitImageInfo) {
            local_pBlitImageInfo = &var_local_pBlitImageInfo;
            local_pBlitImageInfo->initialize(pBlitImageInfo);

            if (pBlitImageInfo->srcImage) {
                local_pBlitImageInfo->srcImage = layer_data->Unwrap(pBlitImageInfo->srcImage);
            }
            if (pBlitImageInfo->dstImage) {
                local_pBlitImageInfo->dstImage = layer_data->Unwrap(pBlitImageInfo->dstImage);
            }
        }
    }
    layer_data->device_dispatch_table.CmdBlitImage2KHR(commandBuffer, (const VkBlitImageInfo2*)local_pBlitImageInfo);
}

void DispatchCmdResolveImage2KHR(VkCommandBuffer commandBuffer, const VkResolveImageInfo2* pResolveImageInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CmdResolveImage2KHR(commandBuffer, pResolveImageInfo);
    safe_VkResolveImageInfo2 var_local_pResolveImageInfo;
    safe_VkResolveImageInfo2* local_pResolveImageInfo = nullptr;
    {
        if (pResolveImageInfo) {
            local_pResolveImageInfo = &var_local_pResolveImageInfo;
            local_pResolveImageInfo->initialize(pResolveImageInfo);

            if (pResolveImageInfo->srcImage) {
                local_pResolveImageInfo->srcImage = layer_data->Unwrap(pResolveImageInfo->srcImage);
            }
            if (pResolveImageInfo->dstImage) {
                local_pResolveImageInfo->dstImage = layer_data->Unwrap(pResolveImageInfo->dstImage);
            }
        }
    }
    layer_data->device_dispatch_table.CmdResolveImage2KHR(commandBuffer, (const VkResolveImageInfo2*)local_pResolveImageInfo);
}

void DispatchCmdTraceRaysIndirect2KHR(VkCommandBuffer commandBuffer, VkDeviceAddress indirectDeviceAddress) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdTraceRaysIndirect2KHR(commandBuffer, indirectDeviceAddress);
}

void DispatchGetDeviceBufferMemoryRequirementsKHR(VkDevice device, const VkDeviceBufferMemoryRequirements* pInfo,
                                                  VkMemoryRequirements2* pMemoryRequirements) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);

    layer_data->device_dispatch_table.GetDeviceBufferMemoryRequirementsKHR(device, pInfo, pMemoryRequirements);
}

void DispatchGetDeviceImageMemoryRequirementsKHR(VkDevice device, const VkDeviceImageMemoryRequirements* pInfo,
                                                 VkMemoryRequirements2* pMemoryRequirements) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);

    layer_data->device_dispatch_table.GetDeviceImageMemoryRequirementsKHR(device, pInfo, pMemoryRequirements);
}

void DispatchGetDeviceImageSparseMemoryRequirementsKHR(VkDevice device, const VkDeviceImageMemoryRequirements* pInfo,
                                                       uint32_t* pSparseMemoryRequirementCount,
                                                       VkSparseImageMemoryRequirements2* pSparseMemoryRequirements) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);

    layer_data->device_dispatch_table.GetDeviceImageSparseMemoryRequirementsKHR(device, pInfo, pSparseMemoryRequirementCount,
                                                                                pSparseMemoryRequirements);
}

void DispatchCmdBindIndexBuffer2KHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkDeviceSize size,
                                    VkIndexType indexType) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.CmdBindIndexBuffer2KHR(commandBuffer, buffer, offset, size, indexType);
    { buffer = layer_data->Unwrap(buffer); }
    layer_data->device_dispatch_table.CmdBindIndexBuffer2KHR(commandBuffer, buffer, offset, size, indexType);
}

void DispatchGetRenderingAreaGranularityKHR(VkDevice device, const VkRenderingAreaInfoKHR* pRenderingAreaInfo,
                                            VkExtent2D* pGranularity) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);

    layer_data->device_dispatch_table.GetRenderingAreaGranularityKHR(device, pRenderingAreaInfo, pGranularity);
}

void DispatchGetDeviceImageSubresourceLayoutKHR(VkDevice device, const VkDeviceImageSubresourceInfoKHR* pInfo,
                                                VkSubresourceLayout2KHR* pLayout) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);

    layer_data->device_dispatch_table.GetDeviceImageSubresourceLayoutKHR(device, pInfo, pLayout);
}

void DispatchGetImageSubresourceLayout2KHR(VkDevice device, VkImage image, const VkImageSubresource2KHR* pSubresource,
                                           VkSubresourceLayout2KHR* pLayout) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.GetImageSubresourceLayout2KHR(device, image, pSubresource, pLayout);
    { image = layer_data->Unwrap(image); }
    layer_data->device_dispatch_table.GetImageSubresourceLayout2KHR(device, image, pSubresource, pLayout);
}

VkResult DispatchGetPhysicalDeviceCooperativeMatrixPropertiesKHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount,
                                                                 VkCooperativeMatrixPropertiesKHR* pProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);

    VkResult result = layer_data->instance_dispatch_table.GetPhysicalDeviceCooperativeMatrixPropertiesKHR(
        physicalDevice, pPropertyCount, pProperties);

    return result;
}

VkResult DispatchCreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo,
                                              const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(instance), layer_data_map);
    if (!wrap_handles)
        return layer_data->instance_dispatch_table.CreateDebugReportCallbackEXT(instance, pCreateInfo, pAllocator, pCallback);

    VkResult result =
        layer_data->instance_dispatch_table.CreateDebugReportCallbackEXT(instance, pCreateInfo, pAllocator, pCallback);
    if (VK_SUCCESS == result) {
        *pCallback = layer_data->WrapNew(*pCallback);
    }
    return result;
}

void DispatchDestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback,
                                           const VkAllocationCallbacks* pAllocator) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(instance), layer_data_map);
    if (!wrap_handles) return layer_data->instance_dispatch_table.DestroyDebugReportCallbackEXT(instance, callback, pAllocator);

    uint64_t callback_id = CastToUint64(callback);
    auto iter = unique_id_mapping.pop(callback_id);
    if (iter != unique_id_mapping.end()) {
        callback = (VkDebugReportCallbackEXT)iter->second;
    } else {
        callback = (VkDebugReportCallbackEXT)0;
    }
    layer_data->instance_dispatch_table.DestroyDebugReportCallbackEXT(instance, callback, pAllocator);
}

void DispatchDebugReportMessageEXT(VkInstance instance, VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType,
                                   uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix,
                                   const char* pMessage) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(instance), layer_data_map);

    layer_data->instance_dispatch_table.DebugReportMessageEXT(instance, flags, objectType, object, location, messageCode,
                                                              pLayerPrefix, pMessage);
}

void DispatchCmdDebugMarkerBeginEXT(VkCommandBuffer commandBuffer, const VkDebugMarkerMarkerInfoEXT* pMarkerInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdDebugMarkerBeginEXT(commandBuffer, pMarkerInfo);
}

void DispatchCmdDebugMarkerEndEXT(VkCommandBuffer commandBuffer) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdDebugMarkerEndEXT(commandBuffer);
}

void DispatchCmdDebugMarkerInsertEXT(VkCommandBuffer commandBuffer, const VkDebugMarkerMarkerInfoEXT* pMarkerInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdDebugMarkerInsertEXT(commandBuffer, pMarkerInfo);
}

void DispatchCmdBindTransformFeedbackBuffersEXT(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount,
                                                const VkBuffer* pBuffers, const VkDeviceSize* pOffsets,
                                                const VkDeviceSize* pSizes) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.CmdBindTransformFeedbackBuffersEXT(commandBuffer, firstBinding, bindingCount,
                                                                                    pBuffers, pOffsets, pSizes);
    VkBuffer var_local_pBuffers[DISPATCH_MAX_STACK_ALLOCATIONS];
    VkBuffer* local_pBuffers = nullptr;
    {
        if (pBuffers) {
            local_pBuffers = bindingCount > DISPATCH_MAX_STACK_ALLOCATIONS ? new VkBuffer[bindingCount] : var_local_pBuffers;
            for (uint32_t index0 = 0; index0 < bindingCount; ++index0) {
                local_pBuffers[index0] = layer_data->Unwrap(pBuffers[index0]);
            }
        }
    }
    layer_data->device_dispatch_table.CmdBindTransformFeedbackBuffersEXT(commandBuffer, firstBinding, bindingCount,
                                                                         (const VkBuffer*)local_pBuffers, pOffsets, pSizes);
    if (local_pBuffers != var_local_pBuffers) delete[] local_pBuffers;
}

void DispatchCmdBeginTransformFeedbackEXT(VkCommandBuffer commandBuffer, uint32_t firstCounterBuffer, uint32_t counterBufferCount,
                                          const VkBuffer* pCounterBuffers, const VkDeviceSize* pCounterBufferOffsets) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.CmdBeginTransformFeedbackEXT(commandBuffer, firstCounterBuffer, counterBufferCount,
                                                                              pCounterBuffers, pCounterBufferOffsets);
    VkBuffer var_local_pCounterBuffers[DISPATCH_MAX_STACK_ALLOCATIONS];
    VkBuffer* local_pCounterBuffers = nullptr;
    {
        if (pCounterBuffers) {
            local_pCounterBuffers =
                counterBufferCount > DISPATCH_MAX_STACK_ALLOCATIONS ? new VkBuffer[counterBufferCount] : var_local_pCounterBuffers;
            for (uint32_t index0 = 0; index0 < counterBufferCount; ++index0) {
                local_pCounterBuffers[index0] = layer_data->Unwrap(pCounterBuffers[index0]);
            }
        }
    }
    layer_data->device_dispatch_table.CmdBeginTransformFeedbackEXT(commandBuffer, firstCounterBuffer, counterBufferCount,
                                                                   (const VkBuffer*)local_pCounterBuffers, pCounterBufferOffsets);
    if (local_pCounterBuffers != var_local_pCounterBuffers) delete[] local_pCounterBuffers;
}

void DispatchCmdEndTransformFeedbackEXT(VkCommandBuffer commandBuffer, uint32_t firstCounterBuffer, uint32_t counterBufferCount,
                                        const VkBuffer* pCounterBuffers, const VkDeviceSize* pCounterBufferOffsets) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.CmdEndTransformFeedbackEXT(commandBuffer, firstCounterBuffer, counterBufferCount,
                                                                            pCounterBuffers, pCounterBufferOffsets);
    VkBuffer var_local_pCounterBuffers[DISPATCH_MAX_STACK_ALLOCATIONS];
    VkBuffer* local_pCounterBuffers = nullptr;
    {
        if (pCounterBuffers) {
            local_pCounterBuffers =
                counterBufferCount > DISPATCH_MAX_STACK_ALLOCATIONS ? new VkBuffer[counterBufferCount] : var_local_pCounterBuffers;
            for (uint32_t index0 = 0; index0 < counterBufferCount; ++index0) {
                local_pCounterBuffers[index0] = layer_data->Unwrap(pCounterBuffers[index0]);
            }
        }
    }
    layer_data->device_dispatch_table.CmdEndTransformFeedbackEXT(commandBuffer, firstCounterBuffer, counterBufferCount,
                                                                 (const VkBuffer*)local_pCounterBuffers, pCounterBufferOffsets);
    if (local_pCounterBuffers != var_local_pCounterBuffers) delete[] local_pCounterBuffers;
}

void DispatchCmdBeginQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query,
                                     VkQueryControlFlags flags, uint32_t index) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.CmdBeginQueryIndexedEXT(commandBuffer, queryPool, query, flags, index);
    { queryPool = layer_data->Unwrap(queryPool); }
    layer_data->device_dispatch_table.CmdBeginQueryIndexedEXT(commandBuffer, queryPool, query, flags, index);
}

void DispatchCmdEndQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query, uint32_t index) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CmdEndQueryIndexedEXT(commandBuffer, queryPool, query, index);
    { queryPool = layer_data->Unwrap(queryPool); }
    layer_data->device_dispatch_table.CmdEndQueryIndexedEXT(commandBuffer, queryPool, query, index);
}

void DispatchCmdDrawIndirectByteCountEXT(VkCommandBuffer commandBuffer, uint32_t instanceCount, uint32_t firstInstance,
                                         VkBuffer counterBuffer, VkDeviceSize counterBufferOffset, uint32_t counterOffset,
                                         uint32_t vertexStride) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.CmdDrawIndirectByteCountEXT(
            commandBuffer, instanceCount, firstInstance, counterBuffer, counterBufferOffset, counterOffset, vertexStride);
    { counterBuffer = layer_data->Unwrap(counterBuffer); }
    layer_data->device_dispatch_table.CmdDrawIndirectByteCountEXT(commandBuffer, instanceCount, firstInstance, counterBuffer,
                                                                  counterBufferOffset, counterOffset, vertexStride);
}

VkResult DispatchCreateCuModuleNVX(VkDevice device, const VkCuModuleCreateInfoNVX* pCreateInfo,
                                   const VkAllocationCallbacks* pAllocator, VkCuModuleNVX* pModule) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CreateCuModuleNVX(device, pCreateInfo, pAllocator, pModule);

    VkResult result = layer_data->device_dispatch_table.CreateCuModuleNVX(device, pCreateInfo, pAllocator, pModule);
    if (VK_SUCCESS == result) {
        *pModule = layer_data->WrapNew(*pModule);
    }
    return result;
}

VkResult DispatchCreateCuFunctionNVX(VkDevice device, const VkCuFunctionCreateInfoNVX* pCreateInfo,
                                     const VkAllocationCallbacks* pAllocator, VkCuFunctionNVX* pFunction) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CreateCuFunctionNVX(device, pCreateInfo, pAllocator, pFunction);
    safe_VkCuFunctionCreateInfoNVX var_local_pCreateInfo;
    safe_VkCuFunctionCreateInfoNVX* local_pCreateInfo = nullptr;
    {
        if (pCreateInfo) {
            local_pCreateInfo = &var_local_pCreateInfo;
            local_pCreateInfo->initialize(pCreateInfo);

            if (pCreateInfo->module) {
                local_pCreateInfo->module = layer_data->Unwrap(pCreateInfo->module);
            }
        }
    }
    VkResult result = layer_data->device_dispatch_table.CreateCuFunctionNVX(
        device, (const VkCuFunctionCreateInfoNVX*)local_pCreateInfo, pAllocator, pFunction);
    if (VK_SUCCESS == result) {
        *pFunction = layer_data->WrapNew(*pFunction);
    }
    return result;
}

void DispatchDestroyCuModuleNVX(VkDevice device, VkCuModuleNVX module, const VkAllocationCallbacks* pAllocator) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.DestroyCuModuleNVX(device, module, pAllocator);

    uint64_t module_id = CastToUint64(module);
    auto iter = unique_id_mapping.pop(module_id);
    if (iter != unique_id_mapping.end()) {
        module = (VkCuModuleNVX)iter->second;
    } else {
        module = (VkCuModuleNVX)0;
    }
    layer_data->device_dispatch_table.DestroyCuModuleNVX(device, module, pAllocator);
}

void DispatchDestroyCuFunctionNVX(VkDevice device, VkCuFunctionNVX function, const VkAllocationCallbacks* pAllocator) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.DestroyCuFunctionNVX(device, function, pAllocator);

    uint64_t function_id = CastToUint64(function);
    auto iter = unique_id_mapping.pop(function_id);
    if (iter != unique_id_mapping.end()) {
        function = (VkCuFunctionNVX)iter->second;
    } else {
        function = (VkCuFunctionNVX)0;
    }
    layer_data->device_dispatch_table.DestroyCuFunctionNVX(device, function, pAllocator);
}

void DispatchCmdCuLaunchKernelNVX(VkCommandBuffer commandBuffer, const VkCuLaunchInfoNVX* pLaunchInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CmdCuLaunchKernelNVX(commandBuffer, pLaunchInfo);
    safe_VkCuLaunchInfoNVX var_local_pLaunchInfo;
    safe_VkCuLaunchInfoNVX* local_pLaunchInfo = nullptr;
    {
        if (pLaunchInfo) {
            local_pLaunchInfo = &var_local_pLaunchInfo;
            local_pLaunchInfo->initialize(pLaunchInfo);

            if (pLaunchInfo->function) {
                local_pLaunchInfo->function = layer_data->Unwrap(pLaunchInfo->function);
            }
        }
    }
    layer_data->device_dispatch_table.CmdCuLaunchKernelNVX(commandBuffer, (const VkCuLaunchInfoNVX*)local_pLaunchInfo);
}

uint32_t DispatchGetImageViewHandleNVX(VkDevice device, const VkImageViewHandleInfoNVX* pInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.GetImageViewHandleNVX(device, pInfo);
    safe_VkImageViewHandleInfoNVX var_local_pInfo;
    safe_VkImageViewHandleInfoNVX* local_pInfo = nullptr;
    {
        if (pInfo) {
            local_pInfo = &var_local_pInfo;
            local_pInfo->initialize(pInfo);

            if (pInfo->imageView) {
                local_pInfo->imageView = layer_data->Unwrap(pInfo->imageView);
            }
            if (pInfo->sampler) {
                local_pInfo->sampler = layer_data->Unwrap(pInfo->sampler);
            }
        }
    }
    uint32_t result = layer_data->device_dispatch_table.GetImageViewHandleNVX(device, (const VkImageViewHandleInfoNVX*)local_pInfo);

    return result;
}

VkResult DispatchGetImageViewAddressNVX(VkDevice device, VkImageView imageView, VkImageViewAddressPropertiesNVX* pProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.GetImageViewAddressNVX(device, imageView, pProperties);
    { imageView = layer_data->Unwrap(imageView); }
    VkResult result = layer_data->device_dispatch_table.GetImageViewAddressNVX(device, imageView, pProperties);

    return result;
}

void DispatchCmdDrawIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer,
                                     VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.CmdDrawIndirectCountAMD(commandBuffer, buffer, offset, countBuffer,
                                                                         countBufferOffset, maxDrawCount, stride);
    {
        buffer = layer_data->Unwrap(buffer);
        countBuffer = layer_data->Unwrap(countBuffer);
    }
    layer_data->device_dispatch_table.CmdDrawIndirectCountAMD(commandBuffer, buffer, offset, countBuffer, countBufferOffset,
                                                              maxDrawCount, stride);
}

void DispatchCmdDrawIndexedIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                            VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                            uint32_t stride) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.CmdDrawIndexedIndirectCountAMD(commandBuffer, buffer, offset, countBuffer,
                                                                                countBufferOffset, maxDrawCount, stride);
    {
        buffer = layer_data->Unwrap(buffer);
        countBuffer = layer_data->Unwrap(countBuffer);
    }
    layer_data->device_dispatch_table.CmdDrawIndexedIndirectCountAMD(commandBuffer, buffer, offset, countBuffer, countBufferOffset,
                                                                     maxDrawCount, stride);
}

VkResult DispatchGetShaderInfoAMD(VkDevice device, VkPipeline pipeline, VkShaderStageFlagBits shaderStage,
                                  VkShaderInfoTypeAMD infoType, size_t* pInfoSize, void* pInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.GetShaderInfoAMD(device, pipeline, shaderStage, infoType, pInfoSize, pInfo);
    { pipeline = layer_data->Unwrap(pipeline); }
    VkResult result = layer_data->device_dispatch_table.GetShaderInfoAMD(device, pipeline, shaderStage, infoType, pInfoSize, pInfo);

    return result;
}
#ifdef VK_USE_PLATFORM_GGP

VkResult DispatchCreateStreamDescriptorSurfaceGGP(VkInstance instance, const VkStreamDescriptorSurfaceCreateInfoGGP* pCreateInfo,
                                                  const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(instance), layer_data_map);
    if (!wrap_handles)
        return layer_data->instance_dispatch_table.CreateStreamDescriptorSurfaceGGP(instance, pCreateInfo, pAllocator, pSurface);

    VkResult result =
        layer_data->instance_dispatch_table.CreateStreamDescriptorSurfaceGGP(instance, pCreateInfo, pAllocator, pSurface);
    if (VK_SUCCESS == result) {
        *pSurface = layer_data->WrapNew(*pSurface);
    }
    return result;
}
#endif  // VK_USE_PLATFORM_GGP

VkResult DispatchGetPhysicalDeviceExternalImageFormatPropertiesNV(
    VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type, VkImageTiling tiling, VkImageUsageFlags usage,
    VkImageCreateFlags flags, VkExternalMemoryHandleTypeFlagsNV externalHandleType,
    VkExternalImageFormatPropertiesNV* pExternalImageFormatProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);

    VkResult result = layer_data->instance_dispatch_table.GetPhysicalDeviceExternalImageFormatPropertiesNV(
        physicalDevice, format, type, tiling, usage, flags, externalHandleType, pExternalImageFormatProperties);

    return result;
}
#ifdef VK_USE_PLATFORM_WIN32_KHR

VkResult DispatchGetMemoryWin32HandleNV(VkDevice device, VkDeviceMemory memory, VkExternalMemoryHandleTypeFlagsNV handleType,
                                        HANDLE* pHandle) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.GetMemoryWin32HandleNV(device, memory, handleType, pHandle);
    { memory = layer_data->Unwrap(memory); }
    VkResult result = layer_data->device_dispatch_table.GetMemoryWin32HandleNV(device, memory, handleType, pHandle);

    return result;
}
#endif  // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_VI_NN

VkResult DispatchCreateViSurfaceNN(VkInstance instance, const VkViSurfaceCreateInfoNN* pCreateInfo,
                                   const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(instance), layer_data_map);
    if (!wrap_handles) return layer_data->instance_dispatch_table.CreateViSurfaceNN(instance, pCreateInfo, pAllocator, pSurface);

    VkResult result = layer_data->instance_dispatch_table.CreateViSurfaceNN(instance, pCreateInfo, pAllocator, pSurface);
    if (VK_SUCCESS == result) {
        *pSurface = layer_data->WrapNew(*pSurface);
    }
    return result;
}
#endif  // VK_USE_PLATFORM_VI_NN

void DispatchCmdBeginConditionalRenderingEXT(VkCommandBuffer commandBuffer,
                                             const VkConditionalRenderingBeginInfoEXT* pConditionalRenderingBegin) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.CmdBeginConditionalRenderingEXT(commandBuffer, pConditionalRenderingBegin);
    safe_VkConditionalRenderingBeginInfoEXT var_local_pConditionalRenderingBegin;
    safe_VkConditionalRenderingBeginInfoEXT* local_pConditionalRenderingBegin = nullptr;
    {
        if (pConditionalRenderingBegin) {
            local_pConditionalRenderingBegin = &var_local_pConditionalRenderingBegin;
            local_pConditionalRenderingBegin->initialize(pConditionalRenderingBegin);

            if (pConditionalRenderingBegin->buffer) {
                local_pConditionalRenderingBegin->buffer = layer_data->Unwrap(pConditionalRenderingBegin->buffer);
            }
        }
    }
    layer_data->device_dispatch_table.CmdBeginConditionalRenderingEXT(
        commandBuffer, (const VkConditionalRenderingBeginInfoEXT*)local_pConditionalRenderingBegin);
}

void DispatchCmdEndConditionalRenderingEXT(VkCommandBuffer commandBuffer) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdEndConditionalRenderingEXT(commandBuffer);
}

void DispatchCmdSetViewportWScalingNV(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount,
                                      const VkViewportWScalingNV* pViewportWScalings) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdSetViewportWScalingNV(commandBuffer, firstViewport, viewportCount, pViewportWScalings);
}

VkResult DispatchReleaseDisplayEXT(VkPhysicalDevice physicalDevice, VkDisplayKHR display) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    if (!wrap_handles) return layer_data->instance_dispatch_table.ReleaseDisplayEXT(physicalDevice, display);
    { display = layer_data->Unwrap(display); }
    VkResult result = layer_data->instance_dispatch_table.ReleaseDisplayEXT(physicalDevice, display);

    return result;
}
#ifdef VK_USE_PLATFORM_XLIB_XRANDR_EXT

VkResult DispatchAcquireXlibDisplayEXT(VkPhysicalDevice physicalDevice, Display* dpy, VkDisplayKHR display) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    if (!wrap_handles) return layer_data->instance_dispatch_table.AcquireXlibDisplayEXT(physicalDevice, dpy, display);
    { display = layer_data->Unwrap(display); }
    VkResult result = layer_data->instance_dispatch_table.AcquireXlibDisplayEXT(physicalDevice, dpy, display);

    return result;
}
#endif  // VK_USE_PLATFORM_XLIB_XRANDR_EXT
#ifdef VK_USE_PLATFORM_XLIB_XRANDR_EXT

VkResult DispatchGetRandROutputDisplayEXT(VkPhysicalDevice physicalDevice, Display* dpy, RROutput rrOutput,
                                          VkDisplayKHR* pDisplay) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    if (!wrap_handles) return layer_data->instance_dispatch_table.GetRandROutputDisplayEXT(physicalDevice, dpy, rrOutput, pDisplay);

    VkResult result = layer_data->instance_dispatch_table.GetRandROutputDisplayEXT(physicalDevice, dpy, rrOutput, pDisplay);
    if (VK_SUCCESS == result) {
        *pDisplay = layer_data->WrapNew(*pDisplay);
    }
    return result;
}
#endif  // VK_USE_PLATFORM_XLIB_XRANDR_EXT

VkResult DispatchGetPhysicalDeviceSurfaceCapabilities2EXT(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                          VkSurfaceCapabilities2EXT* pSurfaceCapabilities) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    if (!wrap_handles)
        return layer_data->instance_dispatch_table.GetPhysicalDeviceSurfaceCapabilities2EXT(physicalDevice, surface,
                                                                                            pSurfaceCapabilities);
    { surface = layer_data->Unwrap(surface); }
    VkResult result =
        layer_data->instance_dispatch_table.GetPhysicalDeviceSurfaceCapabilities2EXT(physicalDevice, surface, pSurfaceCapabilities);

    return result;
}

VkResult DispatchDisplayPowerControlEXT(VkDevice device, VkDisplayKHR display, const VkDisplayPowerInfoEXT* pDisplayPowerInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.DisplayPowerControlEXT(device, display, pDisplayPowerInfo);
    { display = layer_data->Unwrap(display); }
    VkResult result = layer_data->device_dispatch_table.DisplayPowerControlEXT(device, display, pDisplayPowerInfo);

    return result;
}

VkResult DispatchRegisterDeviceEventEXT(VkDevice device, const VkDeviceEventInfoEXT* pDeviceEventInfo,
                                        const VkAllocationCallbacks* pAllocator, VkFence* pFence) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.RegisterDeviceEventEXT(device, pDeviceEventInfo, pAllocator, pFence);

    VkResult result = layer_data->device_dispatch_table.RegisterDeviceEventEXT(device, pDeviceEventInfo, pAllocator, pFence);
    if (VK_SUCCESS == result) {
        *pFence = layer_data->WrapNew(*pFence);
    }
    return result;
}

VkResult DispatchRegisterDisplayEventEXT(VkDevice device, VkDisplayKHR display, const VkDisplayEventInfoEXT* pDisplayEventInfo,
                                         const VkAllocationCallbacks* pAllocator, VkFence* pFence) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.RegisterDisplayEventEXT(device, display, pDisplayEventInfo, pAllocator, pFence);
    { display = layer_data->Unwrap(display); }
    VkResult result =
        layer_data->device_dispatch_table.RegisterDisplayEventEXT(device, display, pDisplayEventInfo, pAllocator, pFence);
    if (VK_SUCCESS == result) {
        *pFence = layer_data->WrapNew(*pFence);
    }
    return result;
}

VkResult DispatchGetSwapchainCounterEXT(VkDevice device, VkSwapchainKHR swapchain, VkSurfaceCounterFlagBitsEXT counter,
                                        uint64_t* pCounterValue) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.GetSwapchainCounterEXT(device, swapchain, counter, pCounterValue);
    { swapchain = layer_data->Unwrap(swapchain); }
    VkResult result = layer_data->device_dispatch_table.GetSwapchainCounterEXT(device, swapchain, counter, pCounterValue);

    return result;
}

VkResult DispatchGetRefreshCycleDurationGOOGLE(VkDevice device, VkSwapchainKHR swapchain,
                                               VkRefreshCycleDurationGOOGLE* pDisplayTimingProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.GetRefreshCycleDurationGOOGLE(device, swapchain, pDisplayTimingProperties);
    { swapchain = layer_data->Unwrap(swapchain); }
    VkResult result = layer_data->device_dispatch_table.GetRefreshCycleDurationGOOGLE(device, swapchain, pDisplayTimingProperties);

    return result;
}

VkResult DispatchGetPastPresentationTimingGOOGLE(VkDevice device, VkSwapchainKHR swapchain, uint32_t* pPresentationTimingCount,
                                                 VkPastPresentationTimingGOOGLE* pPresentationTimings) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.GetPastPresentationTimingGOOGLE(device, swapchain, pPresentationTimingCount,
                                                                                 pPresentationTimings);
    { swapchain = layer_data->Unwrap(swapchain); }
    VkResult result = layer_data->device_dispatch_table.GetPastPresentationTimingGOOGLE(device, swapchain, pPresentationTimingCount,
                                                                                        pPresentationTimings);

    return result;
}

void DispatchCmdSetDiscardRectangleEXT(VkCommandBuffer commandBuffer, uint32_t firstDiscardRectangle,
                                       uint32_t discardRectangleCount, const VkRect2D* pDiscardRectangles) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdSetDiscardRectangleEXT(commandBuffer, firstDiscardRectangle, discardRectangleCount,
                                                                pDiscardRectangles);
}

void DispatchCmdSetDiscardRectangleEnableEXT(VkCommandBuffer commandBuffer, VkBool32 discardRectangleEnable) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdSetDiscardRectangleEnableEXT(commandBuffer, discardRectangleEnable);
}

void DispatchCmdSetDiscardRectangleModeEXT(VkCommandBuffer commandBuffer, VkDiscardRectangleModeEXT discardRectangleMode) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdSetDiscardRectangleModeEXT(commandBuffer, discardRectangleMode);
}

void DispatchSetHdrMetadataEXT(VkDevice device, uint32_t swapchainCount, const VkSwapchainKHR* pSwapchains,
                               const VkHdrMetadataEXT* pMetadata) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.SetHdrMetadataEXT(device, swapchainCount, pSwapchains, pMetadata);
    VkSwapchainKHR var_local_pSwapchains[DISPATCH_MAX_STACK_ALLOCATIONS];
    VkSwapchainKHR* local_pSwapchains = nullptr;
    {
        if (pSwapchains) {
            local_pSwapchains =
                swapchainCount > DISPATCH_MAX_STACK_ALLOCATIONS ? new VkSwapchainKHR[swapchainCount] : var_local_pSwapchains;
            for (uint32_t index0 = 0; index0 < swapchainCount; ++index0) {
                local_pSwapchains[index0] = layer_data->Unwrap(pSwapchains[index0]);
            }
        }
    }
    layer_data->device_dispatch_table.SetHdrMetadataEXT(device, swapchainCount, (const VkSwapchainKHR*)local_pSwapchains,
                                                        pMetadata);
    if (local_pSwapchains != var_local_pSwapchains) delete[] local_pSwapchains;
}
#ifdef VK_USE_PLATFORM_IOS_MVK

VkResult DispatchCreateIOSSurfaceMVK(VkInstance instance, const VkIOSSurfaceCreateInfoMVK* pCreateInfo,
                                     const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(instance), layer_data_map);
    if (!wrap_handles) return layer_data->instance_dispatch_table.CreateIOSSurfaceMVK(instance, pCreateInfo, pAllocator, pSurface);

    VkResult result = layer_data->instance_dispatch_table.CreateIOSSurfaceMVK(instance, pCreateInfo, pAllocator, pSurface);
    if (VK_SUCCESS == result) {
        *pSurface = layer_data->WrapNew(*pSurface);
    }
    return result;
}
#endif  // VK_USE_PLATFORM_IOS_MVK
#ifdef VK_USE_PLATFORM_MACOS_MVK

VkResult DispatchCreateMacOSSurfaceMVK(VkInstance instance, const VkMacOSSurfaceCreateInfoMVK* pCreateInfo,
                                       const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(instance), layer_data_map);
    if (!wrap_handles)
        return layer_data->instance_dispatch_table.CreateMacOSSurfaceMVK(instance, pCreateInfo, pAllocator, pSurface);

    VkResult result = layer_data->instance_dispatch_table.CreateMacOSSurfaceMVK(instance, pCreateInfo, pAllocator, pSurface);
    if (VK_SUCCESS == result) {
        *pSurface = layer_data->WrapNew(*pSurface);
    }
    return result;
}
#endif  // VK_USE_PLATFORM_MACOS_MVK

void DispatchQueueBeginDebugUtilsLabelEXT(VkQueue queue, const VkDebugUtilsLabelEXT* pLabelInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(queue), layer_data_map);

    layer_data->device_dispatch_table.QueueBeginDebugUtilsLabelEXT(queue, pLabelInfo);
}

void DispatchQueueEndDebugUtilsLabelEXT(VkQueue queue) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(queue), layer_data_map);

    layer_data->device_dispatch_table.QueueEndDebugUtilsLabelEXT(queue);
}

void DispatchQueueInsertDebugUtilsLabelEXT(VkQueue queue, const VkDebugUtilsLabelEXT* pLabelInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(queue), layer_data_map);

    layer_data->device_dispatch_table.QueueInsertDebugUtilsLabelEXT(queue, pLabelInfo);
}

void DispatchCmdBeginDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT* pLabelInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdBeginDebugUtilsLabelEXT(commandBuffer, pLabelInfo);
}

void DispatchCmdEndDebugUtilsLabelEXT(VkCommandBuffer commandBuffer) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdEndDebugUtilsLabelEXT(commandBuffer);
}

void DispatchCmdInsertDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT* pLabelInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdInsertDebugUtilsLabelEXT(commandBuffer, pLabelInfo);
}

VkResult DispatchCreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                              const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pMessenger) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(instance), layer_data_map);
    if (!wrap_handles)
        return layer_data->instance_dispatch_table.CreateDebugUtilsMessengerEXT(instance, pCreateInfo, pAllocator, pMessenger);

    VkResult result =
        layer_data->instance_dispatch_table.CreateDebugUtilsMessengerEXT(instance, pCreateInfo, pAllocator, pMessenger);
    if (VK_SUCCESS == result) {
        *pMessenger = layer_data->WrapNew(*pMessenger);
    }
    return result;
}

void DispatchDestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT messenger,
                                           const VkAllocationCallbacks* pAllocator) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(instance), layer_data_map);
    if (!wrap_handles) return layer_data->instance_dispatch_table.DestroyDebugUtilsMessengerEXT(instance, messenger, pAllocator);

    uint64_t messenger_id = CastToUint64(messenger);
    auto iter = unique_id_mapping.pop(messenger_id);
    if (iter != unique_id_mapping.end()) {
        messenger = (VkDebugUtilsMessengerEXT)iter->second;
    } else {
        messenger = (VkDebugUtilsMessengerEXT)0;
    }
    layer_data->instance_dispatch_table.DestroyDebugUtilsMessengerEXT(instance, messenger, pAllocator);
}

void DispatchSubmitDebugUtilsMessageEXT(VkInstance instance, VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                        VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                                        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(instance), layer_data_map);

    layer_data->instance_dispatch_table.SubmitDebugUtilsMessageEXT(instance, messageSeverity, messageTypes, pCallbackData);
}
#ifdef VK_USE_PLATFORM_ANDROID_KHR

VkResult DispatchGetAndroidHardwareBufferPropertiesANDROID(VkDevice device, const struct AHardwareBuffer* buffer,
                                                           VkAndroidHardwareBufferPropertiesANDROID* pProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);

    VkResult result = layer_data->device_dispatch_table.GetAndroidHardwareBufferPropertiesANDROID(device, buffer, pProperties);

    return result;
}
#endif  // VK_USE_PLATFORM_ANDROID_KHR
#ifdef VK_USE_PLATFORM_ANDROID_KHR

VkResult DispatchGetMemoryAndroidHardwareBufferANDROID(VkDevice device, const VkMemoryGetAndroidHardwareBufferInfoANDROID* pInfo,
                                                       struct AHardwareBuffer** pBuffer) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.GetMemoryAndroidHardwareBufferANDROID(device, pInfo, pBuffer);
    safe_VkMemoryGetAndroidHardwareBufferInfoANDROID var_local_pInfo;
    safe_VkMemoryGetAndroidHardwareBufferInfoANDROID* local_pInfo = nullptr;
    {
        if (pInfo) {
            local_pInfo = &var_local_pInfo;
            local_pInfo->initialize(pInfo);

            if (pInfo->memory) {
                local_pInfo->memory = layer_data->Unwrap(pInfo->memory);
            }
        }
    }
    VkResult result = layer_data->device_dispatch_table.GetMemoryAndroidHardwareBufferANDROID(
        device, (const VkMemoryGetAndroidHardwareBufferInfoANDROID*)local_pInfo, pBuffer);

    return result;
}
#endif  // VK_USE_PLATFORM_ANDROID_KHR
#ifdef VK_ENABLE_BETA_EXTENSIONS

VkResult DispatchCreateExecutionGraphPipelinesAMDX(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                   const VkExecutionGraphPipelineCreateInfoAMDX* pCreateInfos,
                                                   const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.CreateExecutionGraphPipelinesAMDX(device, pipelineCache, createInfoCount,
                                                                                   pCreateInfos, pAllocator, pPipelines);
    safe_VkExecutionGraphPipelineCreateInfoAMDX* local_pCreateInfos = nullptr;
    {
        pipelineCache = layer_data->Unwrap(pipelineCache);
        if (pCreateInfos) {
            local_pCreateInfos = new safe_VkExecutionGraphPipelineCreateInfoAMDX[createInfoCount];
            for (uint32_t index0 = 0; index0 < createInfoCount; ++index0) {
                local_pCreateInfos[index0].initialize(&pCreateInfos[index0]);
                if (local_pCreateInfos[index0].pStages) {
                    for (uint32_t index1 = 0; index1 < local_pCreateInfos[index0].stageCount; ++index1) {
                        if (pCreateInfos[index0].pStages[index1].module) {
                            local_pCreateInfos[index0].pStages[index1].module =
                                layer_data->Unwrap(pCreateInfos[index0].pStages[index1].module);
                        }
                    }
                }
                if (local_pCreateInfos[index0].pLibraryInfo) {
                    if (local_pCreateInfos[index0].pLibraryInfo->pLibraries) {
                        for (uint32_t index2 = 0; index2 < local_pCreateInfos[index0].pLibraryInfo->libraryCount; ++index2) {
                            local_pCreateInfos[index0].pLibraryInfo->pLibraries[index2] =
                                layer_data->Unwrap(local_pCreateInfos[index0].pLibraryInfo->pLibraries[index2]);
                        }
                    }
                }

                if (pCreateInfos[index0].layout) {
                    local_pCreateInfos[index0].layout = layer_data->Unwrap(pCreateInfos[index0].layout);
                }
                if (pCreateInfos[index0].basePipelineHandle) {
                    local_pCreateInfos[index0].basePipelineHandle = layer_data->Unwrap(pCreateInfos[index0].basePipelineHandle);
                }
            }
        }
    }
    VkResult result = layer_data->device_dispatch_table.CreateExecutionGraphPipelinesAMDX(
        device, pipelineCache, createInfoCount, (const VkExecutionGraphPipelineCreateInfoAMDX*)local_pCreateInfos, pAllocator,
        pPipelines);
    if (local_pCreateInfos) {
        delete[] local_pCreateInfos;
    }
    if (VK_SUCCESS == result) {
        for (uint32_t index0 = 0; index0 < createInfoCount; index0++) {
            pPipelines[index0] = layer_data->WrapNew(pPipelines[index0]);
        }
    }
    return result;
}
#endif  // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS

VkResult DispatchGetExecutionGraphPipelineScratchSizeAMDX(VkDevice device, VkPipeline executionGraph,
                                                          VkExecutionGraphPipelineScratchSizeAMDX* pSizeInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.GetExecutionGraphPipelineScratchSizeAMDX(device, executionGraph, pSizeInfo);
    { executionGraph = layer_data->Unwrap(executionGraph); }
    VkResult result = layer_data->device_dispatch_table.GetExecutionGraphPipelineScratchSizeAMDX(device, executionGraph, pSizeInfo);

    return result;
}
#endif  // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS

VkResult DispatchGetExecutionGraphPipelineNodeIndexAMDX(VkDevice device, VkPipeline executionGraph,
                                                        const VkPipelineShaderStageNodeCreateInfoAMDX* pNodeInfo,
                                                        uint32_t* pNodeIndex) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.GetExecutionGraphPipelineNodeIndexAMDX(device, executionGraph, pNodeInfo,
                                                                                        pNodeIndex);
    { executionGraph = layer_data->Unwrap(executionGraph); }
    VkResult result =
        layer_data->device_dispatch_table.GetExecutionGraphPipelineNodeIndexAMDX(device, executionGraph, pNodeInfo, pNodeIndex);

    return result;
}
#endif  // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS

void DispatchCmdInitializeGraphScratchMemoryAMDX(VkCommandBuffer commandBuffer, VkDeviceAddress scratch) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdInitializeGraphScratchMemoryAMDX(commandBuffer, scratch);
}
#endif  // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS

void DispatchCmdDispatchGraphAMDX(VkCommandBuffer commandBuffer, VkDeviceAddress scratch,
                                  const VkDispatchGraphCountInfoAMDX* pCountInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdDispatchGraphAMDX(commandBuffer, scratch, pCountInfo);
}
#endif  // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS

void DispatchCmdDispatchGraphIndirectAMDX(VkCommandBuffer commandBuffer, VkDeviceAddress scratch,
                                          const VkDispatchGraphCountInfoAMDX* pCountInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdDispatchGraphIndirectAMDX(commandBuffer, scratch, pCountInfo);
}
#endif  // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS

void DispatchCmdDispatchGraphIndirectCountAMDX(VkCommandBuffer commandBuffer, VkDeviceAddress scratch, VkDeviceAddress countInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdDispatchGraphIndirectCountAMDX(commandBuffer, scratch, countInfo);
}
#endif  // VK_ENABLE_BETA_EXTENSIONS

void DispatchCmdSetSampleLocationsEXT(VkCommandBuffer commandBuffer, const VkSampleLocationsInfoEXT* pSampleLocationsInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdSetSampleLocationsEXT(commandBuffer, pSampleLocationsInfo);
}

void DispatchGetPhysicalDeviceMultisamplePropertiesEXT(VkPhysicalDevice physicalDevice, VkSampleCountFlagBits samples,
                                                       VkMultisamplePropertiesEXT* pMultisampleProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);

    layer_data->instance_dispatch_table.GetPhysicalDeviceMultisamplePropertiesEXT(physicalDevice, samples, pMultisampleProperties);
}

VkResult DispatchGetImageDrmFormatModifierPropertiesEXT(VkDevice device, VkImage image,
                                                        VkImageDrmFormatModifierPropertiesEXT* pProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.GetImageDrmFormatModifierPropertiesEXT(device, image, pProperties);
    { image = layer_data->Unwrap(image); }
    VkResult result = layer_data->device_dispatch_table.GetImageDrmFormatModifierPropertiesEXT(device, image, pProperties);

    return result;
}

VkResult DispatchCreateValidationCacheEXT(VkDevice device, const VkValidationCacheCreateInfoEXT* pCreateInfo,
                                          const VkAllocationCallbacks* pAllocator, VkValidationCacheEXT* pValidationCache) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.CreateValidationCacheEXT(device, pCreateInfo, pAllocator, pValidationCache);

    VkResult result = layer_data->device_dispatch_table.CreateValidationCacheEXT(device, pCreateInfo, pAllocator, pValidationCache);
    if (VK_SUCCESS == result) {
        *pValidationCache = layer_data->WrapNew(*pValidationCache);
    }
    return result;
}

void DispatchDestroyValidationCacheEXT(VkDevice device, VkValidationCacheEXT validationCache,
                                       const VkAllocationCallbacks* pAllocator) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.DestroyValidationCacheEXT(device, validationCache, pAllocator);

    uint64_t validationCache_id = CastToUint64(validationCache);
    auto iter = unique_id_mapping.pop(validationCache_id);
    if (iter != unique_id_mapping.end()) {
        validationCache = (VkValidationCacheEXT)iter->second;
    } else {
        validationCache = (VkValidationCacheEXT)0;
    }
    layer_data->device_dispatch_table.DestroyValidationCacheEXT(device, validationCache, pAllocator);
}

VkResult DispatchMergeValidationCachesEXT(VkDevice device, VkValidationCacheEXT dstCache, uint32_t srcCacheCount,
                                          const VkValidationCacheEXT* pSrcCaches) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.MergeValidationCachesEXT(device, dstCache, srcCacheCount, pSrcCaches);
    VkValidationCacheEXT var_local_pSrcCaches[DISPATCH_MAX_STACK_ALLOCATIONS];
    VkValidationCacheEXT* local_pSrcCaches = nullptr;
    {
        dstCache = layer_data->Unwrap(dstCache);
        if (pSrcCaches) {
            local_pSrcCaches =
                srcCacheCount > DISPATCH_MAX_STACK_ALLOCATIONS ? new VkValidationCacheEXT[srcCacheCount] : var_local_pSrcCaches;
            for (uint32_t index0 = 0; index0 < srcCacheCount; ++index0) {
                local_pSrcCaches[index0] = layer_data->Unwrap(pSrcCaches[index0]);
            }
        }
    }
    VkResult result = layer_data->device_dispatch_table.MergeValidationCachesEXT(device, dstCache, srcCacheCount,
                                                                                 (const VkValidationCacheEXT*)local_pSrcCaches);
    if (local_pSrcCaches != var_local_pSrcCaches) delete[] local_pSrcCaches;
    return result;
}

VkResult DispatchGetValidationCacheDataEXT(VkDevice device, VkValidationCacheEXT validationCache, size_t* pDataSize, void* pData) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.GetValidationCacheDataEXT(device, validationCache, pDataSize, pData);
    { validationCache = layer_data->Unwrap(validationCache); }
    VkResult result = layer_data->device_dispatch_table.GetValidationCacheDataEXT(device, validationCache, pDataSize, pData);

    return result;
}

void DispatchCmdBindShadingRateImageNV(VkCommandBuffer commandBuffer, VkImageView imageView, VkImageLayout imageLayout) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CmdBindShadingRateImageNV(commandBuffer, imageView, imageLayout);
    { imageView = layer_data->Unwrap(imageView); }
    layer_data->device_dispatch_table.CmdBindShadingRateImageNV(commandBuffer, imageView, imageLayout);
}

void DispatchCmdSetViewportShadingRatePaletteNV(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount,
                                                const VkShadingRatePaletteNV* pShadingRatePalettes) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdSetViewportShadingRatePaletteNV(commandBuffer, firstViewport, viewportCount,
                                                                         pShadingRatePalettes);
}

void DispatchCmdSetCoarseSampleOrderNV(VkCommandBuffer commandBuffer, VkCoarseSampleOrderTypeNV sampleOrderType,
                                       uint32_t customSampleOrderCount, const VkCoarseSampleOrderCustomNV* pCustomSampleOrders) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdSetCoarseSampleOrderNV(commandBuffer, sampleOrderType, customSampleOrderCount,
                                                                pCustomSampleOrders);
}

VkResult DispatchCreateAccelerationStructureNV(VkDevice device, const VkAccelerationStructureCreateInfoNV* pCreateInfo,
                                               const VkAllocationCallbacks* pAllocator,
                                               VkAccelerationStructureNV* pAccelerationStructure) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.CreateAccelerationStructureNV(device, pCreateInfo, pAllocator,
                                                                               pAccelerationStructure);
    safe_VkAccelerationStructureCreateInfoNV var_local_pCreateInfo;
    safe_VkAccelerationStructureCreateInfoNV* local_pCreateInfo = nullptr;
    {
        if (pCreateInfo) {
            local_pCreateInfo = &var_local_pCreateInfo;
            local_pCreateInfo->initialize(pCreateInfo);
            if (local_pCreateInfo->info.pGeometries) {
                for (uint32_t index2 = 0; index2 < local_pCreateInfo->info.geometryCount; ++index2) {
                    if (pCreateInfo->info.pGeometries[index2].geometry.triangles.vertexData) {
                        local_pCreateInfo->info.pGeometries[index2].geometry.triangles.vertexData =
                            layer_data->Unwrap(pCreateInfo->info.pGeometries[index2].geometry.triangles.vertexData);
                    }
                    if (pCreateInfo->info.pGeometries[index2].geometry.triangles.indexData) {
                        local_pCreateInfo->info.pGeometries[index2].geometry.triangles.indexData =
                            layer_data->Unwrap(pCreateInfo->info.pGeometries[index2].geometry.triangles.indexData);
                    }
                    if (pCreateInfo->info.pGeometries[index2].geometry.triangles.transformData) {
                        local_pCreateInfo->info.pGeometries[index2].geometry.triangles.transformData =
                            layer_data->Unwrap(pCreateInfo->info.pGeometries[index2].geometry.triangles.transformData);
                    }
                    if (pCreateInfo->info.pGeometries[index2].geometry.aabbs.aabbData) {
                        local_pCreateInfo->info.pGeometries[index2].geometry.aabbs.aabbData =
                            layer_data->Unwrap(pCreateInfo->info.pGeometries[index2].geometry.aabbs.aabbData);
                    }
                }
            }
        }
    }
    VkResult result = layer_data->device_dispatch_table.CreateAccelerationStructureNV(
        device, (const VkAccelerationStructureCreateInfoNV*)local_pCreateInfo, pAllocator, pAccelerationStructure);
    if (VK_SUCCESS == result) {
        *pAccelerationStructure = layer_data->WrapNew(*pAccelerationStructure);
    }
    return result;
}

void DispatchDestroyAccelerationStructureNV(VkDevice device, VkAccelerationStructureNV accelerationStructure,
                                            const VkAllocationCallbacks* pAllocator) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.DestroyAccelerationStructureNV(device, accelerationStructure, pAllocator);

    uint64_t accelerationStructure_id = CastToUint64(accelerationStructure);
    auto iter = unique_id_mapping.pop(accelerationStructure_id);
    if (iter != unique_id_mapping.end()) {
        accelerationStructure = (VkAccelerationStructureNV)iter->second;
    } else {
        accelerationStructure = (VkAccelerationStructureNV)0;
    }
    layer_data->device_dispatch_table.DestroyAccelerationStructureNV(device, accelerationStructure, pAllocator);
}

void DispatchGetAccelerationStructureMemoryRequirementsNV(VkDevice device,
                                                          const VkAccelerationStructureMemoryRequirementsInfoNV* pInfo,
                                                          VkMemoryRequirements2KHR* pMemoryRequirements) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.GetAccelerationStructureMemoryRequirementsNV(device, pInfo, pMemoryRequirements);
    safe_VkAccelerationStructureMemoryRequirementsInfoNV var_local_pInfo;
    safe_VkAccelerationStructureMemoryRequirementsInfoNV* local_pInfo = nullptr;
    {
        if (pInfo) {
            local_pInfo = &var_local_pInfo;
            local_pInfo->initialize(pInfo);

            if (pInfo->accelerationStructure) {
                local_pInfo->accelerationStructure = layer_data->Unwrap(pInfo->accelerationStructure);
            }
        }
    }
    layer_data->device_dispatch_table.GetAccelerationStructureMemoryRequirementsNV(
        device, (const VkAccelerationStructureMemoryRequirementsInfoNV*)local_pInfo, pMemoryRequirements);
}

VkResult DispatchBindAccelerationStructureMemoryNV(VkDevice device, uint32_t bindInfoCount,
                                                   const VkBindAccelerationStructureMemoryInfoNV* pBindInfos) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.BindAccelerationStructureMemoryNV(device, bindInfoCount, pBindInfos);
    safe_VkBindAccelerationStructureMemoryInfoNV* local_pBindInfos = nullptr;
    {
        if (pBindInfos) {
            local_pBindInfos = new safe_VkBindAccelerationStructureMemoryInfoNV[bindInfoCount];
            for (uint32_t index0 = 0; index0 < bindInfoCount; ++index0) {
                local_pBindInfos[index0].initialize(&pBindInfos[index0]);

                if (pBindInfos[index0].accelerationStructure) {
                    local_pBindInfos[index0].accelerationStructure = layer_data->Unwrap(pBindInfos[index0].accelerationStructure);
                }
                if (pBindInfos[index0].memory) {
                    local_pBindInfos[index0].memory = layer_data->Unwrap(pBindInfos[index0].memory);
                }
            }
        }
    }
    VkResult result = layer_data->device_dispatch_table.BindAccelerationStructureMemoryNV(
        device, bindInfoCount, (const VkBindAccelerationStructureMemoryInfoNV*)local_pBindInfos);
    if (local_pBindInfos) {
        delete[] local_pBindInfos;
    }
    return result;
}

void DispatchCmdBuildAccelerationStructureNV(VkCommandBuffer commandBuffer, const VkAccelerationStructureInfoNV* pInfo,
                                             VkBuffer instanceData, VkDeviceSize instanceOffset, VkBool32 update,
                                             VkAccelerationStructureNV dst, VkAccelerationStructureNV src, VkBuffer scratch,
                                             VkDeviceSize scratchOffset) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.CmdBuildAccelerationStructureNV(commandBuffer, pInfo, instanceData, instanceOffset,
                                                                                 update, dst, src, scratch, scratchOffset);
    safe_VkAccelerationStructureInfoNV var_local_pInfo;
    safe_VkAccelerationStructureInfoNV* local_pInfo = nullptr;
    {
        if (pInfo) {
            local_pInfo = &var_local_pInfo;
            local_pInfo->initialize(pInfo);
            if (local_pInfo->pGeometries) {
                for (uint32_t index1 = 0; index1 < local_pInfo->geometryCount; ++index1) {
                    if (pInfo->pGeometries[index1].geometry.triangles.vertexData) {
                        local_pInfo->pGeometries[index1].geometry.triangles.vertexData =
                            layer_data->Unwrap(pInfo->pGeometries[index1].geometry.triangles.vertexData);
                    }
                    if (pInfo->pGeometries[index1].geometry.triangles.indexData) {
                        local_pInfo->pGeometries[index1].geometry.triangles.indexData =
                            layer_data->Unwrap(pInfo->pGeometries[index1].geometry.triangles.indexData);
                    }
                    if (pInfo->pGeometries[index1].geometry.triangles.transformData) {
                        local_pInfo->pGeometries[index1].geometry.triangles.transformData =
                            layer_data->Unwrap(pInfo->pGeometries[index1].geometry.triangles.transformData);
                    }
                    if (pInfo->pGeometries[index1].geometry.aabbs.aabbData) {
                        local_pInfo->pGeometries[index1].geometry.aabbs.aabbData =
                            layer_data->Unwrap(pInfo->pGeometries[index1].geometry.aabbs.aabbData);
                    }
                }
            }
        }
        instanceData = layer_data->Unwrap(instanceData);
        dst = layer_data->Unwrap(dst);
        src = layer_data->Unwrap(src);
        scratch = layer_data->Unwrap(scratch);
    }
    layer_data->device_dispatch_table.CmdBuildAccelerationStructureNV(
        commandBuffer, (const VkAccelerationStructureInfoNV*)local_pInfo, instanceData, instanceOffset, update, dst, src, scratch,
        scratchOffset);
}

void DispatchCmdCopyAccelerationStructureNV(VkCommandBuffer commandBuffer, VkAccelerationStructureNV dst,
                                            VkAccelerationStructureNV src, VkCopyAccelerationStructureModeKHR mode) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CmdCopyAccelerationStructureNV(commandBuffer, dst, src, mode);
    {
        dst = layer_data->Unwrap(dst);
        src = layer_data->Unwrap(src);
    }
    layer_data->device_dispatch_table.CmdCopyAccelerationStructureNV(commandBuffer, dst, src, mode);
}

void DispatchCmdTraceRaysNV(VkCommandBuffer commandBuffer, VkBuffer raygenShaderBindingTableBuffer,
                            VkDeviceSize raygenShaderBindingOffset, VkBuffer missShaderBindingTableBuffer,
                            VkDeviceSize missShaderBindingOffset, VkDeviceSize missShaderBindingStride,
                            VkBuffer hitShaderBindingTableBuffer, VkDeviceSize hitShaderBindingOffset,
                            VkDeviceSize hitShaderBindingStride, VkBuffer callableShaderBindingTableBuffer,
                            VkDeviceSize callableShaderBindingOffset, VkDeviceSize callableShaderBindingStride, uint32_t width,
                            uint32_t height, uint32_t depth) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.CmdTraceRaysNV(
            commandBuffer, raygenShaderBindingTableBuffer, raygenShaderBindingOffset, missShaderBindingTableBuffer,
            missShaderBindingOffset, missShaderBindingStride, hitShaderBindingTableBuffer, hitShaderBindingOffset,
            hitShaderBindingStride, callableShaderBindingTableBuffer, callableShaderBindingOffset, callableShaderBindingStride,
            width, height, depth);
    {
        raygenShaderBindingTableBuffer = layer_data->Unwrap(raygenShaderBindingTableBuffer);
        missShaderBindingTableBuffer = layer_data->Unwrap(missShaderBindingTableBuffer);
        hitShaderBindingTableBuffer = layer_data->Unwrap(hitShaderBindingTableBuffer);
        callableShaderBindingTableBuffer = layer_data->Unwrap(callableShaderBindingTableBuffer);
    }
    layer_data->device_dispatch_table.CmdTraceRaysNV(commandBuffer, raygenShaderBindingTableBuffer, raygenShaderBindingOffset,
                                                     missShaderBindingTableBuffer, missShaderBindingOffset, missShaderBindingStride,
                                                     hitShaderBindingTableBuffer, hitShaderBindingOffset, hitShaderBindingStride,
                                                     callableShaderBindingTableBuffer, callableShaderBindingOffset,
                                                     callableShaderBindingStride, width, height, depth);
}

VkResult DispatchGetRayTracingShaderGroupHandlesKHR(VkDevice device, VkPipeline pipeline, uint32_t firstGroup, uint32_t groupCount,
                                                    size_t dataSize, void* pData) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.GetRayTracingShaderGroupHandlesKHR(device, pipeline, firstGroup, groupCount,
                                                                                    dataSize, pData);
    { pipeline = layer_data->Unwrap(pipeline); }
    VkResult result = layer_data->device_dispatch_table.GetRayTracingShaderGroupHandlesKHR(device, pipeline, firstGroup, groupCount,
                                                                                           dataSize, pData);

    return result;
}

VkResult DispatchGetRayTracingShaderGroupHandlesNV(VkDevice device, VkPipeline pipeline, uint32_t firstGroup, uint32_t groupCount,
                                                   size_t dataSize, void* pData) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.GetRayTracingShaderGroupHandlesNV(device, pipeline, firstGroup, groupCount,
                                                                                   dataSize, pData);
    { pipeline = layer_data->Unwrap(pipeline); }
    VkResult result = layer_data->device_dispatch_table.GetRayTracingShaderGroupHandlesNV(device, pipeline, firstGroup, groupCount,
                                                                                          dataSize, pData);

    return result;
}

VkResult DispatchGetAccelerationStructureHandleNV(VkDevice device, VkAccelerationStructureNV accelerationStructure, size_t dataSize,
                                                  void* pData) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.GetAccelerationStructureHandleNV(device, accelerationStructure, dataSize, pData);
    { accelerationStructure = layer_data->Unwrap(accelerationStructure); }
    VkResult result =
        layer_data->device_dispatch_table.GetAccelerationStructureHandleNV(device, accelerationStructure, dataSize, pData);

    return result;
}

void DispatchCmdWriteAccelerationStructuresPropertiesNV(VkCommandBuffer commandBuffer, uint32_t accelerationStructureCount,
                                                        const VkAccelerationStructureNV* pAccelerationStructures,
                                                        VkQueryType queryType, VkQueryPool queryPool, uint32_t firstQuery) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.CmdWriteAccelerationStructuresPropertiesNV(
            commandBuffer, accelerationStructureCount, pAccelerationStructures, queryType, queryPool, firstQuery);
    VkAccelerationStructureNV var_local_pAccelerationStructures[DISPATCH_MAX_STACK_ALLOCATIONS];
    VkAccelerationStructureNV* local_pAccelerationStructures = nullptr;
    {
        if (pAccelerationStructures) {
            local_pAccelerationStructures = accelerationStructureCount > DISPATCH_MAX_STACK_ALLOCATIONS
                                                ? new VkAccelerationStructureNV[accelerationStructureCount]
                                                : var_local_pAccelerationStructures;
            for (uint32_t index0 = 0; index0 < accelerationStructureCount; ++index0) {
                local_pAccelerationStructures[index0] = layer_data->Unwrap(pAccelerationStructures[index0]);
            }
        }
        queryPool = layer_data->Unwrap(queryPool);
    }
    layer_data->device_dispatch_table.CmdWriteAccelerationStructuresPropertiesNV(
        commandBuffer, accelerationStructureCount, (const VkAccelerationStructureNV*)local_pAccelerationStructures, queryType,
        queryPool, firstQuery);
    if (local_pAccelerationStructures != var_local_pAccelerationStructures) delete[] local_pAccelerationStructures;
}

VkResult DispatchCompileDeferredNV(VkDevice device, VkPipeline pipeline, uint32_t shader) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CompileDeferredNV(device, pipeline, shader);
    { pipeline = layer_data->Unwrap(pipeline); }
    VkResult result = layer_data->device_dispatch_table.CompileDeferredNV(device, pipeline, shader);

    return result;
}

VkResult DispatchGetMemoryHostPointerPropertiesEXT(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType,
                                                   const void* pHostPointer,
                                                   VkMemoryHostPointerPropertiesEXT* pMemoryHostPointerProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);

    VkResult result = layer_data->device_dispatch_table.GetMemoryHostPointerPropertiesEXT(device, handleType, pHostPointer,
                                                                                          pMemoryHostPointerProperties);

    return result;
}

void DispatchCmdWriteBufferMarkerAMD(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage, VkBuffer dstBuffer,
                                     VkDeviceSize dstOffset, uint32_t marker) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.CmdWriteBufferMarkerAMD(commandBuffer, pipelineStage, dstBuffer, dstOffset,
                                                                         marker);
    { dstBuffer = layer_data->Unwrap(dstBuffer); }
    layer_data->device_dispatch_table.CmdWriteBufferMarkerAMD(commandBuffer, pipelineStage, dstBuffer, dstOffset, marker);
}

VkResult DispatchGetPhysicalDeviceCalibrateableTimeDomainsEXT(VkPhysicalDevice physicalDevice, uint32_t* pTimeDomainCount,
                                                              VkTimeDomainEXT* pTimeDomains) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);

    VkResult result = layer_data->instance_dispatch_table.GetPhysicalDeviceCalibrateableTimeDomainsEXT(
        physicalDevice, pTimeDomainCount, pTimeDomains);

    return result;
}

VkResult DispatchGetCalibratedTimestampsEXT(VkDevice device, uint32_t timestampCount,
                                            const VkCalibratedTimestampInfoEXT* pTimestampInfos, uint64_t* pTimestamps,
                                            uint64_t* pMaxDeviation) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);

    VkResult result = layer_data->device_dispatch_table.GetCalibratedTimestampsEXT(device, timestampCount, pTimestampInfos,
                                                                                   pTimestamps, pMaxDeviation);

    return result;
}

void DispatchCmdDrawMeshTasksNV(VkCommandBuffer commandBuffer, uint32_t taskCount, uint32_t firstTask) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdDrawMeshTasksNV(commandBuffer, taskCount, firstTask);
}

void DispatchCmdDrawMeshTasksIndirectNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount,
                                        uint32_t stride) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.CmdDrawMeshTasksIndirectNV(commandBuffer, buffer, offset, drawCount, stride);
    { buffer = layer_data->Unwrap(buffer); }
    layer_data->device_dispatch_table.CmdDrawMeshTasksIndirectNV(commandBuffer, buffer, offset, drawCount, stride);
}

void DispatchCmdDrawMeshTasksIndirectCountNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                             VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                             uint32_t stride) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.CmdDrawMeshTasksIndirectCountNV(commandBuffer, buffer, offset, countBuffer,
                                                                                 countBufferOffset, maxDrawCount, stride);
    {
        buffer = layer_data->Unwrap(buffer);
        countBuffer = layer_data->Unwrap(countBuffer);
    }
    layer_data->device_dispatch_table.CmdDrawMeshTasksIndirectCountNV(commandBuffer, buffer, offset, countBuffer, countBufferOffset,
                                                                      maxDrawCount, stride);
}

void DispatchCmdSetExclusiveScissorEnableNV(VkCommandBuffer commandBuffer, uint32_t firstExclusiveScissor,
                                            uint32_t exclusiveScissorCount, const VkBool32* pExclusiveScissorEnables) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdSetExclusiveScissorEnableNV(commandBuffer, firstExclusiveScissor, exclusiveScissorCount,
                                                                     pExclusiveScissorEnables);
}

void DispatchCmdSetExclusiveScissorNV(VkCommandBuffer commandBuffer, uint32_t firstExclusiveScissor, uint32_t exclusiveScissorCount,
                                      const VkRect2D* pExclusiveScissors) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdSetExclusiveScissorNV(commandBuffer, firstExclusiveScissor, exclusiveScissorCount,
                                                               pExclusiveScissors);
}

void DispatchCmdSetCheckpointNV(VkCommandBuffer commandBuffer, const void* pCheckpointMarker) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdSetCheckpointNV(commandBuffer, pCheckpointMarker);
}

void DispatchGetQueueCheckpointDataNV(VkQueue queue, uint32_t* pCheckpointDataCount, VkCheckpointDataNV* pCheckpointData) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(queue), layer_data_map);

    layer_data->device_dispatch_table.GetQueueCheckpointDataNV(queue, pCheckpointDataCount, pCheckpointData);
}

VkResult DispatchInitializePerformanceApiINTEL(VkDevice device, const VkInitializePerformanceApiInfoINTEL* pInitializeInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);

    VkResult result = layer_data->device_dispatch_table.InitializePerformanceApiINTEL(device, pInitializeInfo);

    return result;
}

void DispatchUninitializePerformanceApiINTEL(VkDevice device) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);

    layer_data->device_dispatch_table.UninitializePerformanceApiINTEL(device);
}

VkResult DispatchCmdSetPerformanceMarkerINTEL(VkCommandBuffer commandBuffer, const VkPerformanceMarkerInfoINTEL* pMarkerInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    VkResult result = layer_data->device_dispatch_table.CmdSetPerformanceMarkerINTEL(commandBuffer, pMarkerInfo);

    return result;
}

VkResult DispatchCmdSetPerformanceStreamMarkerINTEL(VkCommandBuffer commandBuffer,
                                                    const VkPerformanceStreamMarkerInfoINTEL* pMarkerInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    VkResult result = layer_data->device_dispatch_table.CmdSetPerformanceStreamMarkerINTEL(commandBuffer, pMarkerInfo);

    return result;
}

VkResult DispatchCmdSetPerformanceOverrideINTEL(VkCommandBuffer commandBuffer,
                                                const VkPerformanceOverrideInfoINTEL* pOverrideInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    VkResult result = layer_data->device_dispatch_table.CmdSetPerformanceOverrideINTEL(commandBuffer, pOverrideInfo);

    return result;
}

VkResult DispatchAcquirePerformanceConfigurationINTEL(VkDevice device,
                                                      const VkPerformanceConfigurationAcquireInfoINTEL* pAcquireInfo,
                                                      VkPerformanceConfigurationINTEL* pConfiguration) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.AcquirePerformanceConfigurationINTEL(device, pAcquireInfo, pConfiguration);

    VkResult result = layer_data->device_dispatch_table.AcquirePerformanceConfigurationINTEL(device, pAcquireInfo, pConfiguration);
    if (VK_SUCCESS == result) {
        *pConfiguration = layer_data->WrapNew(*pConfiguration);
    }
    return result;
}

VkResult DispatchQueueSetPerformanceConfigurationINTEL(VkQueue queue, VkPerformanceConfigurationINTEL configuration) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(queue), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.QueueSetPerformanceConfigurationINTEL(queue, configuration);
    { configuration = layer_data->Unwrap(configuration); }
    VkResult result = layer_data->device_dispatch_table.QueueSetPerformanceConfigurationINTEL(queue, configuration);

    return result;
}

VkResult DispatchGetPerformanceParameterINTEL(VkDevice device, VkPerformanceParameterTypeINTEL parameter,
                                              VkPerformanceValueINTEL* pValue) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);

    VkResult result = layer_data->device_dispatch_table.GetPerformanceParameterINTEL(device, parameter, pValue);

    return result;
}

void DispatchSetLocalDimmingAMD(VkDevice device, VkSwapchainKHR swapChain, VkBool32 localDimmingEnable) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.SetLocalDimmingAMD(device, swapChain, localDimmingEnable);
    { swapChain = layer_data->Unwrap(swapChain); }
    layer_data->device_dispatch_table.SetLocalDimmingAMD(device, swapChain, localDimmingEnable);
}
#ifdef VK_USE_PLATFORM_FUCHSIA

VkResult DispatchCreateImagePipeSurfaceFUCHSIA(VkInstance instance, const VkImagePipeSurfaceCreateInfoFUCHSIA* pCreateInfo,
                                               const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(instance), layer_data_map);
    if (!wrap_handles)
        return layer_data->instance_dispatch_table.CreateImagePipeSurfaceFUCHSIA(instance, pCreateInfo, pAllocator, pSurface);

    VkResult result =
        layer_data->instance_dispatch_table.CreateImagePipeSurfaceFUCHSIA(instance, pCreateInfo, pAllocator, pSurface);
    if (VK_SUCCESS == result) {
        *pSurface = layer_data->WrapNew(*pSurface);
    }
    return result;
}
#endif  // VK_USE_PLATFORM_FUCHSIA
#ifdef VK_USE_PLATFORM_METAL_EXT

VkResult DispatchCreateMetalSurfaceEXT(VkInstance instance, const VkMetalSurfaceCreateInfoEXT* pCreateInfo,
                                       const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(instance), layer_data_map);
    if (!wrap_handles)
        return layer_data->instance_dispatch_table.CreateMetalSurfaceEXT(instance, pCreateInfo, pAllocator, pSurface);

    VkResult result = layer_data->instance_dispatch_table.CreateMetalSurfaceEXT(instance, pCreateInfo, pAllocator, pSurface);
    if (VK_SUCCESS == result) {
        *pSurface = layer_data->WrapNew(*pSurface);
    }
    return result;
}
#endif  // VK_USE_PLATFORM_METAL_EXT

VkDeviceAddress DispatchGetBufferDeviceAddressEXT(VkDevice device, const VkBufferDeviceAddressInfo* pInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.GetBufferDeviceAddressEXT(device, pInfo);
    safe_VkBufferDeviceAddressInfo var_local_pInfo;
    safe_VkBufferDeviceAddressInfo* local_pInfo = nullptr;
    {
        if (pInfo) {
            local_pInfo = &var_local_pInfo;
            local_pInfo->initialize(pInfo);

            if (pInfo->buffer) {
                local_pInfo->buffer = layer_data->Unwrap(pInfo->buffer);
            }
        }
    }
    VkDeviceAddress result =
        layer_data->device_dispatch_table.GetBufferDeviceAddressEXT(device, (const VkBufferDeviceAddressInfo*)local_pInfo);

    return result;
}

VkResult DispatchGetPhysicalDeviceCooperativeMatrixPropertiesNV(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount,
                                                                VkCooperativeMatrixPropertiesNV* pProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);

    VkResult result = layer_data->instance_dispatch_table.GetPhysicalDeviceCooperativeMatrixPropertiesNV(
        physicalDevice, pPropertyCount, pProperties);

    return result;
}

VkResult DispatchGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV(
    VkPhysicalDevice physicalDevice, uint32_t* pCombinationCount, VkFramebufferMixedSamplesCombinationNV* pCombinations) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);

    VkResult result = layer_data->instance_dispatch_table.GetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV(
        physicalDevice, pCombinationCount, pCombinations);

    return result;
}
#ifdef VK_USE_PLATFORM_WIN32_KHR

VkResult DispatchGetPhysicalDeviceSurfacePresentModes2EXT(VkPhysicalDevice physicalDevice,
                                                          const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo,
                                                          uint32_t* pPresentModeCount, VkPresentModeKHR* pPresentModes) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    if (!wrap_handles)
        return layer_data->instance_dispatch_table.GetPhysicalDeviceSurfacePresentModes2EXT(physicalDevice, pSurfaceInfo,
                                                                                            pPresentModeCount, pPresentModes);
    safe_VkPhysicalDeviceSurfaceInfo2KHR var_local_pSurfaceInfo;
    safe_VkPhysicalDeviceSurfaceInfo2KHR* local_pSurfaceInfo = nullptr;
    {
        if (pSurfaceInfo) {
            local_pSurfaceInfo = &var_local_pSurfaceInfo;
            local_pSurfaceInfo->initialize(pSurfaceInfo);

            if (pSurfaceInfo->surface) {
                local_pSurfaceInfo->surface = layer_data->Unwrap(pSurfaceInfo->surface);
            }
        }
    }
    VkResult result = layer_data->instance_dispatch_table.GetPhysicalDeviceSurfacePresentModes2EXT(
        physicalDevice, (const VkPhysicalDeviceSurfaceInfo2KHR*)local_pSurfaceInfo, pPresentModeCount, pPresentModes);

    return result;
}
#endif  // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR

VkResult DispatchAcquireFullScreenExclusiveModeEXT(VkDevice device, VkSwapchainKHR swapchain) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.AcquireFullScreenExclusiveModeEXT(device, swapchain);
    { swapchain = layer_data->Unwrap(swapchain); }
    VkResult result = layer_data->device_dispatch_table.AcquireFullScreenExclusiveModeEXT(device, swapchain);

    return result;
}
#endif  // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR

VkResult DispatchReleaseFullScreenExclusiveModeEXT(VkDevice device, VkSwapchainKHR swapchain) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.ReleaseFullScreenExclusiveModeEXT(device, swapchain);
    { swapchain = layer_data->Unwrap(swapchain); }
    VkResult result = layer_data->device_dispatch_table.ReleaseFullScreenExclusiveModeEXT(device, swapchain);

    return result;
}
#endif  // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR

VkResult DispatchGetDeviceGroupSurfacePresentModes2EXT(VkDevice device, const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo,
                                                       VkDeviceGroupPresentModeFlagsKHR* pModes) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.GetDeviceGroupSurfacePresentModes2EXT(device, pSurfaceInfo, pModes);
    safe_VkPhysicalDeviceSurfaceInfo2KHR var_local_pSurfaceInfo;
    safe_VkPhysicalDeviceSurfaceInfo2KHR* local_pSurfaceInfo = nullptr;
    {
        if (pSurfaceInfo) {
            local_pSurfaceInfo = &var_local_pSurfaceInfo;
            local_pSurfaceInfo->initialize(pSurfaceInfo);

            if (pSurfaceInfo->surface) {
                local_pSurfaceInfo->surface = layer_data->Unwrap(pSurfaceInfo->surface);
            }
        }
    }
    VkResult result = layer_data->device_dispatch_table.GetDeviceGroupSurfacePresentModes2EXT(
        device, (const VkPhysicalDeviceSurfaceInfo2KHR*)local_pSurfaceInfo, pModes);

    return result;
}
#endif  // VK_USE_PLATFORM_WIN32_KHR

VkResult DispatchCreateHeadlessSurfaceEXT(VkInstance instance, const VkHeadlessSurfaceCreateInfoEXT* pCreateInfo,
                                          const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(instance), layer_data_map);
    if (!wrap_handles)
        return layer_data->instance_dispatch_table.CreateHeadlessSurfaceEXT(instance, pCreateInfo, pAllocator, pSurface);

    VkResult result = layer_data->instance_dispatch_table.CreateHeadlessSurfaceEXT(instance, pCreateInfo, pAllocator, pSurface);
    if (VK_SUCCESS == result) {
        *pSurface = layer_data->WrapNew(*pSurface);
    }
    return result;
}

void DispatchCmdSetLineStippleEXT(VkCommandBuffer commandBuffer, uint32_t lineStippleFactor, uint16_t lineStipplePattern) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdSetLineStippleEXT(commandBuffer, lineStippleFactor, lineStipplePattern);
}

void DispatchResetQueryPoolEXT(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.ResetQueryPoolEXT(device, queryPool, firstQuery, queryCount);
    { queryPool = layer_data->Unwrap(queryPool); }
    layer_data->device_dispatch_table.ResetQueryPoolEXT(device, queryPool, firstQuery, queryCount);
}

void DispatchCmdSetCullModeEXT(VkCommandBuffer commandBuffer, VkCullModeFlags cullMode) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdSetCullModeEXT(commandBuffer, cullMode);
}

void DispatchCmdSetFrontFaceEXT(VkCommandBuffer commandBuffer, VkFrontFace frontFace) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdSetFrontFaceEXT(commandBuffer, frontFace);
}

void DispatchCmdSetPrimitiveTopologyEXT(VkCommandBuffer commandBuffer, VkPrimitiveTopology primitiveTopology) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdSetPrimitiveTopologyEXT(commandBuffer, primitiveTopology);
}

void DispatchCmdSetViewportWithCountEXT(VkCommandBuffer commandBuffer, uint32_t viewportCount, const VkViewport* pViewports) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdSetViewportWithCountEXT(commandBuffer, viewportCount, pViewports);
}

void DispatchCmdSetScissorWithCountEXT(VkCommandBuffer commandBuffer, uint32_t scissorCount, const VkRect2D* pScissors) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdSetScissorWithCountEXT(commandBuffer, scissorCount, pScissors);
}

void DispatchCmdBindVertexBuffers2EXT(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount,
                                      const VkBuffer* pBuffers, const VkDeviceSize* pOffsets, const VkDeviceSize* pSizes,
                                      const VkDeviceSize* pStrides) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.CmdBindVertexBuffers2EXT(commandBuffer, firstBinding, bindingCount, pBuffers,
                                                                          pOffsets, pSizes, pStrides);
    VkBuffer var_local_pBuffers[DISPATCH_MAX_STACK_ALLOCATIONS];
    VkBuffer* local_pBuffers = nullptr;
    {
        if (pBuffers) {
            local_pBuffers = bindingCount > DISPATCH_MAX_STACK_ALLOCATIONS ? new VkBuffer[bindingCount] : var_local_pBuffers;
            for (uint32_t index0 = 0; index0 < bindingCount; ++index0) {
                local_pBuffers[index0] = layer_data->Unwrap(pBuffers[index0]);
            }
        }
    }
    layer_data->device_dispatch_table.CmdBindVertexBuffers2EXT(commandBuffer, firstBinding, bindingCount,
                                                               (const VkBuffer*)local_pBuffers, pOffsets, pSizes, pStrides);
    if (local_pBuffers != var_local_pBuffers) delete[] local_pBuffers;
}

void DispatchCmdSetDepthTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthTestEnable) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdSetDepthTestEnableEXT(commandBuffer, depthTestEnable);
}

void DispatchCmdSetDepthWriteEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthWriteEnable) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdSetDepthWriteEnableEXT(commandBuffer, depthWriteEnable);
}

void DispatchCmdSetDepthCompareOpEXT(VkCommandBuffer commandBuffer, VkCompareOp depthCompareOp) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdSetDepthCompareOpEXT(commandBuffer, depthCompareOp);
}

void DispatchCmdSetDepthBoundsTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthBoundsTestEnable) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdSetDepthBoundsTestEnableEXT(commandBuffer, depthBoundsTestEnable);
}

void DispatchCmdSetStencilTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 stencilTestEnable) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdSetStencilTestEnableEXT(commandBuffer, stencilTestEnable);
}

void DispatchCmdSetStencilOpEXT(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, VkStencilOp failOp, VkStencilOp passOp,
                                VkStencilOp depthFailOp, VkCompareOp compareOp) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdSetStencilOpEXT(commandBuffer, faceMask, failOp, passOp, depthFailOp, compareOp);
}

VkResult DispatchCopyMemoryToImageEXT(VkDevice device, const VkCopyMemoryToImageInfoEXT* pCopyMemoryToImageInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CopyMemoryToImageEXT(device, pCopyMemoryToImageInfo);
    safe_VkCopyMemoryToImageInfoEXT var_local_pCopyMemoryToImageInfo;
    safe_VkCopyMemoryToImageInfoEXT* local_pCopyMemoryToImageInfo = nullptr;
    {
        if (pCopyMemoryToImageInfo) {
            local_pCopyMemoryToImageInfo = &var_local_pCopyMemoryToImageInfo;
            local_pCopyMemoryToImageInfo->initialize(pCopyMemoryToImageInfo);

            if (pCopyMemoryToImageInfo->dstImage) {
                local_pCopyMemoryToImageInfo->dstImage = layer_data->Unwrap(pCopyMemoryToImageInfo->dstImage);
            }
        }
    }
    VkResult result = layer_data->device_dispatch_table.CopyMemoryToImageEXT(
        device, (const VkCopyMemoryToImageInfoEXT*)local_pCopyMemoryToImageInfo);

    return result;
}

VkResult DispatchCopyImageToMemoryEXT(VkDevice device, const VkCopyImageToMemoryInfoEXT* pCopyImageToMemoryInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CopyImageToMemoryEXT(device, pCopyImageToMemoryInfo);
    safe_VkCopyImageToMemoryInfoEXT var_local_pCopyImageToMemoryInfo;
    safe_VkCopyImageToMemoryInfoEXT* local_pCopyImageToMemoryInfo = nullptr;
    {
        if (pCopyImageToMemoryInfo) {
            local_pCopyImageToMemoryInfo = &var_local_pCopyImageToMemoryInfo;
            local_pCopyImageToMemoryInfo->initialize(pCopyImageToMemoryInfo);

            if (pCopyImageToMemoryInfo->srcImage) {
                local_pCopyImageToMemoryInfo->srcImage = layer_data->Unwrap(pCopyImageToMemoryInfo->srcImage);
            }
        }
    }
    VkResult result = layer_data->device_dispatch_table.CopyImageToMemoryEXT(
        device, (const VkCopyImageToMemoryInfoEXT*)local_pCopyImageToMemoryInfo);

    return result;
}

VkResult DispatchCopyImageToImageEXT(VkDevice device, const VkCopyImageToImageInfoEXT* pCopyImageToImageInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CopyImageToImageEXT(device, pCopyImageToImageInfo);
    safe_VkCopyImageToImageInfoEXT var_local_pCopyImageToImageInfo;
    safe_VkCopyImageToImageInfoEXT* local_pCopyImageToImageInfo = nullptr;
    {
        if (pCopyImageToImageInfo) {
            local_pCopyImageToImageInfo = &var_local_pCopyImageToImageInfo;
            local_pCopyImageToImageInfo->initialize(pCopyImageToImageInfo);

            if (pCopyImageToImageInfo->srcImage) {
                local_pCopyImageToImageInfo->srcImage = layer_data->Unwrap(pCopyImageToImageInfo->srcImage);
            }
            if (pCopyImageToImageInfo->dstImage) {
                local_pCopyImageToImageInfo->dstImage = layer_data->Unwrap(pCopyImageToImageInfo->dstImage);
            }
        }
    }
    VkResult result = layer_data->device_dispatch_table.CopyImageToImageEXT(
        device, (const VkCopyImageToImageInfoEXT*)local_pCopyImageToImageInfo);

    return result;
}

VkResult DispatchTransitionImageLayoutEXT(VkDevice device, uint32_t transitionCount,
                                          const VkHostImageLayoutTransitionInfoEXT* pTransitions) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.TransitionImageLayoutEXT(device, transitionCount, pTransitions);
    safe_VkHostImageLayoutTransitionInfoEXT* local_pTransitions = nullptr;
    {
        if (pTransitions) {
            local_pTransitions = new safe_VkHostImageLayoutTransitionInfoEXT[transitionCount];
            for (uint32_t index0 = 0; index0 < transitionCount; ++index0) {
                local_pTransitions[index0].initialize(&pTransitions[index0]);

                if (pTransitions[index0].image) {
                    local_pTransitions[index0].image = layer_data->Unwrap(pTransitions[index0].image);
                }
            }
        }
    }
    VkResult result = layer_data->device_dispatch_table.TransitionImageLayoutEXT(
        device, transitionCount, (const VkHostImageLayoutTransitionInfoEXT*)local_pTransitions);
    if (local_pTransitions) {
        delete[] local_pTransitions;
    }
    return result;
}

void DispatchGetImageSubresourceLayout2EXT(VkDevice device, VkImage image, const VkImageSubresource2KHR* pSubresource,
                                           VkSubresourceLayout2KHR* pLayout) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.GetImageSubresourceLayout2EXT(device, image, pSubresource, pLayout);
    { image = layer_data->Unwrap(image); }
    layer_data->device_dispatch_table.GetImageSubresourceLayout2EXT(device, image, pSubresource, pLayout);
}

VkResult DispatchReleaseSwapchainImagesEXT(VkDevice device, const VkReleaseSwapchainImagesInfoEXT* pReleaseInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.ReleaseSwapchainImagesEXT(device, pReleaseInfo);
    safe_VkReleaseSwapchainImagesInfoEXT var_local_pReleaseInfo;
    safe_VkReleaseSwapchainImagesInfoEXT* local_pReleaseInfo = nullptr;
    {
        if (pReleaseInfo) {
            local_pReleaseInfo = &var_local_pReleaseInfo;
            local_pReleaseInfo->initialize(pReleaseInfo);

            if (pReleaseInfo->swapchain) {
                local_pReleaseInfo->swapchain = layer_data->Unwrap(pReleaseInfo->swapchain);
            }
        }
    }
    VkResult result = layer_data->device_dispatch_table.ReleaseSwapchainImagesEXT(
        device, (const VkReleaseSwapchainImagesInfoEXT*)local_pReleaseInfo);

    return result;
}

void DispatchGetGeneratedCommandsMemoryRequirementsNV(VkDevice device, const VkGeneratedCommandsMemoryRequirementsInfoNV* pInfo,
                                                      VkMemoryRequirements2* pMemoryRequirements) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.GetGeneratedCommandsMemoryRequirementsNV(device, pInfo, pMemoryRequirements);
    safe_VkGeneratedCommandsMemoryRequirementsInfoNV var_local_pInfo;
    safe_VkGeneratedCommandsMemoryRequirementsInfoNV* local_pInfo = nullptr;
    {
        if (pInfo) {
            local_pInfo = &var_local_pInfo;
            local_pInfo->initialize(pInfo);

            if (pInfo->pipeline) {
                local_pInfo->pipeline = layer_data->Unwrap(pInfo->pipeline);
            }
            if (pInfo->indirectCommandsLayout) {
                local_pInfo->indirectCommandsLayout = layer_data->Unwrap(pInfo->indirectCommandsLayout);
            }
        }
    }
    layer_data->device_dispatch_table.GetGeneratedCommandsMemoryRequirementsNV(
        device, (const VkGeneratedCommandsMemoryRequirementsInfoNV*)local_pInfo, pMemoryRequirements);
}

void DispatchCmdPreprocessGeneratedCommandsNV(VkCommandBuffer commandBuffer,
                                              const VkGeneratedCommandsInfoNV* pGeneratedCommandsInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.CmdPreprocessGeneratedCommandsNV(commandBuffer, pGeneratedCommandsInfo);
    safe_VkGeneratedCommandsInfoNV var_local_pGeneratedCommandsInfo;
    safe_VkGeneratedCommandsInfoNV* local_pGeneratedCommandsInfo = nullptr;
    {
        if (pGeneratedCommandsInfo) {
            local_pGeneratedCommandsInfo = &var_local_pGeneratedCommandsInfo;
            local_pGeneratedCommandsInfo->initialize(pGeneratedCommandsInfo);

            if (pGeneratedCommandsInfo->pipeline) {
                local_pGeneratedCommandsInfo->pipeline = layer_data->Unwrap(pGeneratedCommandsInfo->pipeline);
            }
            if (pGeneratedCommandsInfo->indirectCommandsLayout) {
                local_pGeneratedCommandsInfo->indirectCommandsLayout =
                    layer_data->Unwrap(pGeneratedCommandsInfo->indirectCommandsLayout);
            }
            if (local_pGeneratedCommandsInfo->pStreams) {
                for (uint32_t index1 = 0; index1 < local_pGeneratedCommandsInfo->streamCount; ++index1) {
                    if (pGeneratedCommandsInfo->pStreams[index1].buffer) {
                        local_pGeneratedCommandsInfo->pStreams[index1].buffer =
                            layer_data->Unwrap(pGeneratedCommandsInfo->pStreams[index1].buffer);
                    }
                }
            }

            if (pGeneratedCommandsInfo->preprocessBuffer) {
                local_pGeneratedCommandsInfo->preprocessBuffer = layer_data->Unwrap(pGeneratedCommandsInfo->preprocessBuffer);
            }
            if (pGeneratedCommandsInfo->sequencesCountBuffer) {
                local_pGeneratedCommandsInfo->sequencesCountBuffer =
                    layer_data->Unwrap(pGeneratedCommandsInfo->sequencesCountBuffer);
            }
            if (pGeneratedCommandsInfo->sequencesIndexBuffer) {
                local_pGeneratedCommandsInfo->sequencesIndexBuffer =
                    layer_data->Unwrap(pGeneratedCommandsInfo->sequencesIndexBuffer);
            }
        }
    }
    layer_data->device_dispatch_table.CmdPreprocessGeneratedCommandsNV(
        commandBuffer, (const VkGeneratedCommandsInfoNV*)local_pGeneratedCommandsInfo);
}

void DispatchCmdExecuteGeneratedCommandsNV(VkCommandBuffer commandBuffer, VkBool32 isPreprocessed,
                                           const VkGeneratedCommandsInfoNV* pGeneratedCommandsInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.CmdExecuteGeneratedCommandsNV(commandBuffer, isPreprocessed,
                                                                               pGeneratedCommandsInfo);
    safe_VkGeneratedCommandsInfoNV var_local_pGeneratedCommandsInfo;
    safe_VkGeneratedCommandsInfoNV* local_pGeneratedCommandsInfo = nullptr;
    {
        if (pGeneratedCommandsInfo) {
            local_pGeneratedCommandsInfo = &var_local_pGeneratedCommandsInfo;
            local_pGeneratedCommandsInfo->initialize(pGeneratedCommandsInfo);

            if (pGeneratedCommandsInfo->pipeline) {
                local_pGeneratedCommandsInfo->pipeline = layer_data->Unwrap(pGeneratedCommandsInfo->pipeline);
            }
            if (pGeneratedCommandsInfo->indirectCommandsLayout) {
                local_pGeneratedCommandsInfo->indirectCommandsLayout =
                    layer_data->Unwrap(pGeneratedCommandsInfo->indirectCommandsLayout);
            }
            if (local_pGeneratedCommandsInfo->pStreams) {
                for (uint32_t index1 = 0; index1 < local_pGeneratedCommandsInfo->streamCount; ++index1) {
                    if (pGeneratedCommandsInfo->pStreams[index1].buffer) {
                        local_pGeneratedCommandsInfo->pStreams[index1].buffer =
                            layer_data->Unwrap(pGeneratedCommandsInfo->pStreams[index1].buffer);
                    }
                }
            }

            if (pGeneratedCommandsInfo->preprocessBuffer) {
                local_pGeneratedCommandsInfo->preprocessBuffer = layer_data->Unwrap(pGeneratedCommandsInfo->preprocessBuffer);
            }
            if (pGeneratedCommandsInfo->sequencesCountBuffer) {
                local_pGeneratedCommandsInfo->sequencesCountBuffer =
                    layer_data->Unwrap(pGeneratedCommandsInfo->sequencesCountBuffer);
            }
            if (pGeneratedCommandsInfo->sequencesIndexBuffer) {
                local_pGeneratedCommandsInfo->sequencesIndexBuffer =
                    layer_data->Unwrap(pGeneratedCommandsInfo->sequencesIndexBuffer);
            }
        }
    }
    layer_data->device_dispatch_table.CmdExecuteGeneratedCommandsNV(commandBuffer, isPreprocessed,
                                                                    (const VkGeneratedCommandsInfoNV*)local_pGeneratedCommandsInfo);
}

void DispatchCmdBindPipelineShaderGroupNV(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline,
                                          uint32_t groupIndex) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.CmdBindPipelineShaderGroupNV(commandBuffer, pipelineBindPoint, pipeline,
                                                                              groupIndex);
    { pipeline = layer_data->Unwrap(pipeline); }
    layer_data->device_dispatch_table.CmdBindPipelineShaderGroupNV(commandBuffer, pipelineBindPoint, pipeline, groupIndex);
}

VkResult DispatchCreateIndirectCommandsLayoutNV(VkDevice device, const VkIndirectCommandsLayoutCreateInfoNV* pCreateInfo,
                                                const VkAllocationCallbacks* pAllocator,
                                                VkIndirectCommandsLayoutNV* pIndirectCommandsLayout) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.CreateIndirectCommandsLayoutNV(device, pCreateInfo, pAllocator,
                                                                                pIndirectCommandsLayout);
    safe_VkIndirectCommandsLayoutCreateInfoNV var_local_pCreateInfo;
    safe_VkIndirectCommandsLayoutCreateInfoNV* local_pCreateInfo = nullptr;
    {
        if (pCreateInfo) {
            local_pCreateInfo = &var_local_pCreateInfo;
            local_pCreateInfo->initialize(pCreateInfo);
            if (local_pCreateInfo->pTokens) {
                for (uint32_t index1 = 0; index1 < local_pCreateInfo->tokenCount; ++index1) {
                    if (pCreateInfo->pTokens[index1].pushconstantPipelineLayout) {
                        local_pCreateInfo->pTokens[index1].pushconstantPipelineLayout =
                            layer_data->Unwrap(pCreateInfo->pTokens[index1].pushconstantPipelineLayout);
                    }
                }
            }
        }
    }
    VkResult result = layer_data->device_dispatch_table.CreateIndirectCommandsLayoutNV(
        device, (const VkIndirectCommandsLayoutCreateInfoNV*)local_pCreateInfo, pAllocator, pIndirectCommandsLayout);
    if (VK_SUCCESS == result) {
        *pIndirectCommandsLayout = layer_data->WrapNew(*pIndirectCommandsLayout);
    }
    return result;
}

void DispatchDestroyIndirectCommandsLayoutNV(VkDevice device, VkIndirectCommandsLayoutNV indirectCommandsLayout,
                                             const VkAllocationCallbacks* pAllocator) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.DestroyIndirectCommandsLayoutNV(device, indirectCommandsLayout, pAllocator);

    uint64_t indirectCommandsLayout_id = CastToUint64(indirectCommandsLayout);
    auto iter = unique_id_mapping.pop(indirectCommandsLayout_id);
    if (iter != unique_id_mapping.end()) {
        indirectCommandsLayout = (VkIndirectCommandsLayoutNV)iter->second;
    } else {
        indirectCommandsLayout = (VkIndirectCommandsLayoutNV)0;
    }
    layer_data->device_dispatch_table.DestroyIndirectCommandsLayoutNV(device, indirectCommandsLayout, pAllocator);
}

void DispatchCmdSetDepthBias2EXT(VkCommandBuffer commandBuffer, const VkDepthBiasInfoEXT* pDepthBiasInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdSetDepthBias2EXT(commandBuffer, pDepthBiasInfo);
}

VkResult DispatchAcquireDrmDisplayEXT(VkPhysicalDevice physicalDevice, int32_t drmFd, VkDisplayKHR display) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    if (!wrap_handles) return layer_data->instance_dispatch_table.AcquireDrmDisplayEXT(physicalDevice, drmFd, display);
    { display = layer_data->Unwrap(display); }
    VkResult result = layer_data->instance_dispatch_table.AcquireDrmDisplayEXT(physicalDevice, drmFd, display);

    return result;
}

VkResult DispatchGetDrmDisplayEXT(VkPhysicalDevice physicalDevice, int32_t drmFd, uint32_t connectorId, VkDisplayKHR* display) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    if (!wrap_handles) return layer_data->instance_dispatch_table.GetDrmDisplayEXT(physicalDevice, drmFd, connectorId, display);

    VkResult result = layer_data->instance_dispatch_table.GetDrmDisplayEXT(physicalDevice, drmFd, connectorId, display);
    if (VK_SUCCESS == result) {
        *display = layer_data->WrapNew(*display);
    }
    return result;
}

VkResult DispatchCreatePrivateDataSlotEXT(VkDevice device, const VkPrivateDataSlotCreateInfo* pCreateInfo,
                                          const VkAllocationCallbacks* pAllocator, VkPrivateDataSlot* pPrivateDataSlot) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.CreatePrivateDataSlotEXT(device, pCreateInfo, pAllocator, pPrivateDataSlot);

    VkResult result = layer_data->device_dispatch_table.CreatePrivateDataSlotEXT(device, pCreateInfo, pAllocator, pPrivateDataSlot);
    if (VK_SUCCESS == result) {
        *pPrivateDataSlot = layer_data->WrapNew(*pPrivateDataSlot);
    }
    return result;
}

void DispatchDestroyPrivateDataSlotEXT(VkDevice device, VkPrivateDataSlot privateDataSlot,
                                       const VkAllocationCallbacks* pAllocator) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.DestroyPrivateDataSlotEXT(device, privateDataSlot, pAllocator);

    uint64_t privateDataSlot_id = CastToUint64(privateDataSlot);
    auto iter = unique_id_mapping.pop(privateDataSlot_id);
    if (iter != unique_id_mapping.end()) {
        privateDataSlot = (VkPrivateDataSlot)iter->second;
    } else {
        privateDataSlot = (VkPrivateDataSlot)0;
    }
    layer_data->device_dispatch_table.DestroyPrivateDataSlotEXT(device, privateDataSlot, pAllocator);
}

void DispatchGetDescriptorSetLayoutSizeEXT(VkDevice device, VkDescriptorSetLayout layout, VkDeviceSize* pLayoutSizeInBytes) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.GetDescriptorSetLayoutSizeEXT(device, layout, pLayoutSizeInBytes);
    { layout = layer_data->Unwrap(layout); }
    layer_data->device_dispatch_table.GetDescriptorSetLayoutSizeEXT(device, layout, pLayoutSizeInBytes);
}

void DispatchGetDescriptorSetLayoutBindingOffsetEXT(VkDevice device, VkDescriptorSetLayout layout, uint32_t binding,
                                                    VkDeviceSize* pOffset) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.GetDescriptorSetLayoutBindingOffsetEXT(device, layout, binding, pOffset);
    { layout = layer_data->Unwrap(layout); }
    layer_data->device_dispatch_table.GetDescriptorSetLayoutBindingOffsetEXT(device, layout, binding, pOffset);
}

void DispatchCmdBindDescriptorBuffersEXT(VkCommandBuffer commandBuffer, uint32_t bufferCount,
                                         const VkDescriptorBufferBindingInfoEXT* pBindingInfos) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.CmdBindDescriptorBuffersEXT(commandBuffer, bufferCount, pBindingInfos);
    safe_VkDescriptorBufferBindingInfoEXT* local_pBindingInfos = nullptr;
    {
        if (pBindingInfos) {
            local_pBindingInfos = new safe_VkDescriptorBufferBindingInfoEXT[bufferCount];
            for (uint32_t index0 = 0; index0 < bufferCount; ++index0) {
                local_pBindingInfos[index0].initialize(&pBindingInfos[index0]);
                WrapPnextChainHandles(layer_data, local_pBindingInfos[index0].pNext);
            }
        }
    }
    layer_data->device_dispatch_table.CmdBindDescriptorBuffersEXT(commandBuffer, bufferCount,
                                                                  (const VkDescriptorBufferBindingInfoEXT*)local_pBindingInfos);
    if (local_pBindingInfos) {
        delete[] local_pBindingInfos;
    }
}

void DispatchCmdSetDescriptorBufferOffsetsEXT(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                              VkPipelineLayout layout, uint32_t firstSet, uint32_t setCount,
                                              const uint32_t* pBufferIndices, const VkDeviceSize* pOffsets) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.CmdSetDescriptorBufferOffsetsEXT(commandBuffer, pipelineBindPoint, layout,
                                                                                  firstSet, setCount, pBufferIndices, pOffsets);
    { layout = layer_data->Unwrap(layout); }
    layer_data->device_dispatch_table.CmdSetDescriptorBufferOffsetsEXT(commandBuffer, pipelineBindPoint, layout, firstSet, setCount,
                                                                       pBufferIndices, pOffsets);
}

void DispatchCmdBindDescriptorBufferEmbeddedSamplersEXT(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                        VkPipelineLayout layout, uint32_t set) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.CmdBindDescriptorBufferEmbeddedSamplersEXT(commandBuffer, pipelineBindPoint,
                                                                                            layout, set);
    { layout = layer_data->Unwrap(layout); }
    layer_data->device_dispatch_table.CmdBindDescriptorBufferEmbeddedSamplersEXT(commandBuffer, pipelineBindPoint, layout, set);
}

VkResult DispatchGetBufferOpaqueCaptureDescriptorDataEXT(VkDevice device, const VkBufferCaptureDescriptorDataInfoEXT* pInfo,
                                                         void* pData) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.GetBufferOpaqueCaptureDescriptorDataEXT(device, pInfo, pData);
    safe_VkBufferCaptureDescriptorDataInfoEXT var_local_pInfo;
    safe_VkBufferCaptureDescriptorDataInfoEXT* local_pInfo = nullptr;
    {
        if (pInfo) {
            local_pInfo = &var_local_pInfo;
            local_pInfo->initialize(pInfo);

            if (pInfo->buffer) {
                local_pInfo->buffer = layer_data->Unwrap(pInfo->buffer);
            }
        }
    }
    VkResult result = layer_data->device_dispatch_table.GetBufferOpaqueCaptureDescriptorDataEXT(
        device, (const VkBufferCaptureDescriptorDataInfoEXT*)local_pInfo, pData);

    return result;
}

VkResult DispatchGetImageOpaqueCaptureDescriptorDataEXT(VkDevice device, const VkImageCaptureDescriptorDataInfoEXT* pInfo,
                                                        void* pData) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.GetImageOpaqueCaptureDescriptorDataEXT(device, pInfo, pData);
    safe_VkImageCaptureDescriptorDataInfoEXT var_local_pInfo;
    safe_VkImageCaptureDescriptorDataInfoEXT* local_pInfo = nullptr;
    {
        if (pInfo) {
            local_pInfo = &var_local_pInfo;
            local_pInfo->initialize(pInfo);

            if (pInfo->image) {
                local_pInfo->image = layer_data->Unwrap(pInfo->image);
            }
        }
    }
    VkResult result = layer_data->device_dispatch_table.GetImageOpaqueCaptureDescriptorDataEXT(
        device, (const VkImageCaptureDescriptorDataInfoEXT*)local_pInfo, pData);

    return result;
}

VkResult DispatchGetImageViewOpaqueCaptureDescriptorDataEXT(VkDevice device, const VkImageViewCaptureDescriptorDataInfoEXT* pInfo,
                                                            void* pData) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.GetImageViewOpaqueCaptureDescriptorDataEXT(device, pInfo, pData);
    safe_VkImageViewCaptureDescriptorDataInfoEXT var_local_pInfo;
    safe_VkImageViewCaptureDescriptorDataInfoEXT* local_pInfo = nullptr;
    {
        if (pInfo) {
            local_pInfo = &var_local_pInfo;
            local_pInfo->initialize(pInfo);

            if (pInfo->imageView) {
                local_pInfo->imageView = layer_data->Unwrap(pInfo->imageView);
            }
        }
    }
    VkResult result = layer_data->device_dispatch_table.GetImageViewOpaqueCaptureDescriptorDataEXT(
        device, (const VkImageViewCaptureDescriptorDataInfoEXT*)local_pInfo, pData);

    return result;
}

VkResult DispatchGetSamplerOpaqueCaptureDescriptorDataEXT(VkDevice device, const VkSamplerCaptureDescriptorDataInfoEXT* pInfo,
                                                          void* pData) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.GetSamplerOpaqueCaptureDescriptorDataEXT(device, pInfo, pData);
    safe_VkSamplerCaptureDescriptorDataInfoEXT var_local_pInfo;
    safe_VkSamplerCaptureDescriptorDataInfoEXT* local_pInfo = nullptr;
    {
        if (pInfo) {
            local_pInfo = &var_local_pInfo;
            local_pInfo->initialize(pInfo);

            if (pInfo->sampler) {
                local_pInfo->sampler = layer_data->Unwrap(pInfo->sampler);
            }
        }
    }
    VkResult result = layer_data->device_dispatch_table.GetSamplerOpaqueCaptureDescriptorDataEXT(
        device, (const VkSamplerCaptureDescriptorDataInfoEXT*)local_pInfo, pData);

    return result;
}

VkResult DispatchGetAccelerationStructureOpaqueCaptureDescriptorDataEXT(
    VkDevice device, const VkAccelerationStructureCaptureDescriptorDataInfoEXT* pInfo, void* pData) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.GetAccelerationStructureOpaqueCaptureDescriptorDataEXT(device, pInfo, pData);
    safe_VkAccelerationStructureCaptureDescriptorDataInfoEXT var_local_pInfo;
    safe_VkAccelerationStructureCaptureDescriptorDataInfoEXT* local_pInfo = nullptr;
    {
        if (pInfo) {
            local_pInfo = &var_local_pInfo;
            local_pInfo->initialize(pInfo);

            if (pInfo->accelerationStructure) {
                local_pInfo->accelerationStructure = layer_data->Unwrap(pInfo->accelerationStructure);
            }
            if (pInfo->accelerationStructureNV) {
                local_pInfo->accelerationStructureNV = layer_data->Unwrap(pInfo->accelerationStructureNV);
            }
        }
    }
    VkResult result = layer_data->device_dispatch_table.GetAccelerationStructureOpaqueCaptureDescriptorDataEXT(
        device, (const VkAccelerationStructureCaptureDescriptorDataInfoEXT*)local_pInfo, pData);

    return result;
}

void DispatchCmdSetFragmentShadingRateEnumNV(VkCommandBuffer commandBuffer, VkFragmentShadingRateNV shadingRate,
                                             const VkFragmentShadingRateCombinerOpKHR combinerOps[2]) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdSetFragmentShadingRateEnumNV(commandBuffer, shadingRate, combinerOps);
}

VkResult DispatchGetDeviceFaultInfoEXT(VkDevice device, VkDeviceFaultCountsEXT* pFaultCounts, VkDeviceFaultInfoEXT* pFaultInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);

    VkResult result = layer_data->device_dispatch_table.GetDeviceFaultInfoEXT(device, pFaultCounts, pFaultInfo);

    return result;
}
#ifdef VK_USE_PLATFORM_WIN32_KHR

VkResult DispatchAcquireWinrtDisplayNV(VkPhysicalDevice physicalDevice, VkDisplayKHR display) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    if (!wrap_handles) return layer_data->instance_dispatch_table.AcquireWinrtDisplayNV(physicalDevice, display);
    { display = layer_data->Unwrap(display); }
    VkResult result = layer_data->instance_dispatch_table.AcquireWinrtDisplayNV(physicalDevice, display);

    return result;
}
#endif  // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR

VkResult DispatchGetWinrtDisplayNV(VkPhysicalDevice physicalDevice, uint32_t deviceRelativeId, VkDisplayKHR* pDisplay) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    if (!wrap_handles) return layer_data->instance_dispatch_table.GetWinrtDisplayNV(physicalDevice, deviceRelativeId, pDisplay);
    { pDisplay = layer_data->Unwrap(pDisplay); }
    VkResult result = layer_data->instance_dispatch_table.GetWinrtDisplayNV(physicalDevice, deviceRelativeId, pDisplay);

    return result;
}
#endif  // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_DIRECTFB_EXT

VkResult DispatchCreateDirectFBSurfaceEXT(VkInstance instance, const VkDirectFBSurfaceCreateInfoEXT* pCreateInfo,
                                          const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(instance), layer_data_map);
    if (!wrap_handles)
        return layer_data->instance_dispatch_table.CreateDirectFBSurfaceEXT(instance, pCreateInfo, pAllocator, pSurface);

    VkResult result = layer_data->instance_dispatch_table.CreateDirectFBSurfaceEXT(instance, pCreateInfo, pAllocator, pSurface);
    if (VK_SUCCESS == result) {
        *pSurface = layer_data->WrapNew(*pSurface);
    }
    return result;
}
#endif  // VK_USE_PLATFORM_DIRECTFB_EXT
#ifdef VK_USE_PLATFORM_DIRECTFB_EXT

VkBool32 DispatchGetPhysicalDeviceDirectFBPresentationSupportEXT(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
                                                                 IDirectFB* dfb) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);

    VkBool32 result =
        layer_data->instance_dispatch_table.GetPhysicalDeviceDirectFBPresentationSupportEXT(physicalDevice, queueFamilyIndex, dfb);

    return result;
}
#endif  // VK_USE_PLATFORM_DIRECTFB_EXT

void DispatchCmdSetVertexInputEXT(VkCommandBuffer commandBuffer, uint32_t vertexBindingDescriptionCount,
                                  const VkVertexInputBindingDescription2EXT* pVertexBindingDescriptions,
                                  uint32_t vertexAttributeDescriptionCount,
                                  const VkVertexInputAttributeDescription2EXT* pVertexAttributeDescriptions) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdSetVertexInputEXT(commandBuffer, vertexBindingDescriptionCount, pVertexBindingDescriptions,
                                                           vertexAttributeDescriptionCount, pVertexAttributeDescriptions);
}
#ifdef VK_USE_PLATFORM_FUCHSIA

VkResult DispatchGetMemoryZirconHandleFUCHSIA(VkDevice device, const VkMemoryGetZirconHandleInfoFUCHSIA* pGetZirconHandleInfo,
                                              zx_handle_t* pZirconHandle) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.GetMemoryZirconHandleFUCHSIA(device, pGetZirconHandleInfo, pZirconHandle);
    safe_VkMemoryGetZirconHandleInfoFUCHSIA var_local_pGetZirconHandleInfo;
    safe_VkMemoryGetZirconHandleInfoFUCHSIA* local_pGetZirconHandleInfo = nullptr;
    {
        if (pGetZirconHandleInfo) {
            local_pGetZirconHandleInfo = &var_local_pGetZirconHandleInfo;
            local_pGetZirconHandleInfo->initialize(pGetZirconHandleInfo);

            if (pGetZirconHandleInfo->memory) {
                local_pGetZirconHandleInfo->memory = layer_data->Unwrap(pGetZirconHandleInfo->memory);
            }
        }
    }
    VkResult result = layer_data->device_dispatch_table.GetMemoryZirconHandleFUCHSIA(
        device, (const VkMemoryGetZirconHandleInfoFUCHSIA*)local_pGetZirconHandleInfo, pZirconHandle);

    return result;
}
#endif  // VK_USE_PLATFORM_FUCHSIA
#ifdef VK_USE_PLATFORM_FUCHSIA

VkResult DispatchGetMemoryZirconHandlePropertiesFUCHSIA(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType,
                                                        zx_handle_t zirconHandle,
                                                        VkMemoryZirconHandlePropertiesFUCHSIA* pMemoryZirconHandleProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);

    VkResult result = layer_data->device_dispatch_table.GetMemoryZirconHandlePropertiesFUCHSIA(device, handleType, zirconHandle,
                                                                                               pMemoryZirconHandleProperties);

    return result;
}
#endif  // VK_USE_PLATFORM_FUCHSIA
#ifdef VK_USE_PLATFORM_FUCHSIA

VkResult DispatchImportSemaphoreZirconHandleFUCHSIA(
    VkDevice device, const VkImportSemaphoreZirconHandleInfoFUCHSIA* pImportSemaphoreZirconHandleInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.ImportSemaphoreZirconHandleFUCHSIA(device, pImportSemaphoreZirconHandleInfo);
    safe_VkImportSemaphoreZirconHandleInfoFUCHSIA var_local_pImportSemaphoreZirconHandleInfo;
    safe_VkImportSemaphoreZirconHandleInfoFUCHSIA* local_pImportSemaphoreZirconHandleInfo = nullptr;
    {
        if (pImportSemaphoreZirconHandleInfo) {
            local_pImportSemaphoreZirconHandleInfo = &var_local_pImportSemaphoreZirconHandleInfo;
            local_pImportSemaphoreZirconHandleInfo->initialize(pImportSemaphoreZirconHandleInfo);

            if (pImportSemaphoreZirconHandleInfo->semaphore) {
                local_pImportSemaphoreZirconHandleInfo->semaphore = layer_data->Unwrap(pImportSemaphoreZirconHandleInfo->semaphore);
            }
        }
    }
    VkResult result = layer_data->device_dispatch_table.ImportSemaphoreZirconHandleFUCHSIA(
        device, (const VkImportSemaphoreZirconHandleInfoFUCHSIA*)local_pImportSemaphoreZirconHandleInfo);

    return result;
}
#endif  // VK_USE_PLATFORM_FUCHSIA
#ifdef VK_USE_PLATFORM_FUCHSIA

VkResult DispatchGetSemaphoreZirconHandleFUCHSIA(VkDevice device, const VkSemaphoreGetZirconHandleInfoFUCHSIA* pGetZirconHandleInfo,
                                                 zx_handle_t* pZirconHandle) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.GetSemaphoreZirconHandleFUCHSIA(device, pGetZirconHandleInfo, pZirconHandle);
    safe_VkSemaphoreGetZirconHandleInfoFUCHSIA var_local_pGetZirconHandleInfo;
    safe_VkSemaphoreGetZirconHandleInfoFUCHSIA* local_pGetZirconHandleInfo = nullptr;
    {
        if (pGetZirconHandleInfo) {
            local_pGetZirconHandleInfo = &var_local_pGetZirconHandleInfo;
            local_pGetZirconHandleInfo->initialize(pGetZirconHandleInfo);

            if (pGetZirconHandleInfo->semaphore) {
                local_pGetZirconHandleInfo->semaphore = layer_data->Unwrap(pGetZirconHandleInfo->semaphore);
            }
        }
    }
    VkResult result = layer_data->device_dispatch_table.GetSemaphoreZirconHandleFUCHSIA(
        device, (const VkSemaphoreGetZirconHandleInfoFUCHSIA*)local_pGetZirconHandleInfo, pZirconHandle);

    return result;
}
#endif  // VK_USE_PLATFORM_FUCHSIA
#ifdef VK_USE_PLATFORM_FUCHSIA

VkResult DispatchCreateBufferCollectionFUCHSIA(VkDevice device, const VkBufferCollectionCreateInfoFUCHSIA* pCreateInfo,
                                               const VkAllocationCallbacks* pAllocator, VkBufferCollectionFUCHSIA* pCollection) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.CreateBufferCollectionFUCHSIA(device, pCreateInfo, pAllocator, pCollection);

    VkResult result = layer_data->device_dispatch_table.CreateBufferCollectionFUCHSIA(device, pCreateInfo, pAllocator, pCollection);
    if (VK_SUCCESS == result) {
        *pCollection = layer_data->WrapNew(*pCollection);
    }
    return result;
}
#endif  // VK_USE_PLATFORM_FUCHSIA
#ifdef VK_USE_PLATFORM_FUCHSIA

VkResult DispatchSetBufferCollectionImageConstraintsFUCHSIA(VkDevice device, VkBufferCollectionFUCHSIA collection,
                                                            const VkImageConstraintsInfoFUCHSIA* pImageConstraintsInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.SetBufferCollectionImageConstraintsFUCHSIA(device, collection,
                                                                                            pImageConstraintsInfo);
    { collection = layer_data->Unwrap(collection); }
    VkResult result =
        layer_data->device_dispatch_table.SetBufferCollectionImageConstraintsFUCHSIA(device, collection, pImageConstraintsInfo);

    return result;
}
#endif  // VK_USE_PLATFORM_FUCHSIA
#ifdef VK_USE_PLATFORM_FUCHSIA

VkResult DispatchSetBufferCollectionBufferConstraintsFUCHSIA(VkDevice device, VkBufferCollectionFUCHSIA collection,
                                                             const VkBufferConstraintsInfoFUCHSIA* pBufferConstraintsInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.SetBufferCollectionBufferConstraintsFUCHSIA(device, collection,
                                                                                             pBufferConstraintsInfo);
    { collection = layer_data->Unwrap(collection); }
    VkResult result =
        layer_data->device_dispatch_table.SetBufferCollectionBufferConstraintsFUCHSIA(device, collection, pBufferConstraintsInfo);

    return result;
}
#endif  // VK_USE_PLATFORM_FUCHSIA
#ifdef VK_USE_PLATFORM_FUCHSIA

void DispatchDestroyBufferCollectionFUCHSIA(VkDevice device, VkBufferCollectionFUCHSIA collection,
                                            const VkAllocationCallbacks* pAllocator) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.DestroyBufferCollectionFUCHSIA(device, collection, pAllocator);

    uint64_t collection_id = CastToUint64(collection);
    auto iter = unique_id_mapping.pop(collection_id);
    if (iter != unique_id_mapping.end()) {
        collection = (VkBufferCollectionFUCHSIA)iter->second;
    } else {
        collection = (VkBufferCollectionFUCHSIA)0;
    }
    layer_data->device_dispatch_table.DestroyBufferCollectionFUCHSIA(device, collection, pAllocator);
}
#endif  // VK_USE_PLATFORM_FUCHSIA
#ifdef VK_USE_PLATFORM_FUCHSIA

VkResult DispatchGetBufferCollectionPropertiesFUCHSIA(VkDevice device, VkBufferCollectionFUCHSIA collection,
                                                      VkBufferCollectionPropertiesFUCHSIA* pProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.GetBufferCollectionPropertiesFUCHSIA(device, collection, pProperties);
    { collection = layer_data->Unwrap(collection); }
    VkResult result = layer_data->device_dispatch_table.GetBufferCollectionPropertiesFUCHSIA(device, collection, pProperties);

    return result;
}
#endif  // VK_USE_PLATFORM_FUCHSIA

VkResult DispatchGetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI(VkDevice device, VkRenderPass renderpass,
                                                               VkExtent2D* pMaxWorkgroupSize) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.GetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI(device, renderpass,
                                                                                               pMaxWorkgroupSize);
    { renderpass = layer_data->Unwrap(renderpass); }
    VkResult result =
        layer_data->device_dispatch_table.GetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI(device, renderpass, pMaxWorkgroupSize);

    return result;
}

void DispatchCmdSubpassShadingHUAWEI(VkCommandBuffer commandBuffer) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdSubpassShadingHUAWEI(commandBuffer);
}

void DispatchCmdBindInvocationMaskHUAWEI(VkCommandBuffer commandBuffer, VkImageView imageView, VkImageLayout imageLayout) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CmdBindInvocationMaskHUAWEI(commandBuffer, imageView, imageLayout);
    { imageView = layer_data->Unwrap(imageView); }
    layer_data->device_dispatch_table.CmdBindInvocationMaskHUAWEI(commandBuffer, imageView, imageLayout);
}

VkResult DispatchGetMemoryRemoteAddressNV(VkDevice device, const VkMemoryGetRemoteAddressInfoNV* pMemoryGetRemoteAddressInfo,
                                          VkRemoteAddressNV* pAddress) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.GetMemoryRemoteAddressNV(device, pMemoryGetRemoteAddressInfo, pAddress);
    safe_VkMemoryGetRemoteAddressInfoNV var_local_pMemoryGetRemoteAddressInfo;
    safe_VkMemoryGetRemoteAddressInfoNV* local_pMemoryGetRemoteAddressInfo = nullptr;
    {
        if (pMemoryGetRemoteAddressInfo) {
            local_pMemoryGetRemoteAddressInfo = &var_local_pMemoryGetRemoteAddressInfo;
            local_pMemoryGetRemoteAddressInfo->initialize(pMemoryGetRemoteAddressInfo);

            if (pMemoryGetRemoteAddressInfo->memory) {
                local_pMemoryGetRemoteAddressInfo->memory = layer_data->Unwrap(pMemoryGetRemoteAddressInfo->memory);
            }
        }
    }
    VkResult result = layer_data->device_dispatch_table.GetMemoryRemoteAddressNV(
        device, (const VkMemoryGetRemoteAddressInfoNV*)local_pMemoryGetRemoteAddressInfo, pAddress);

    return result;
}

VkResult DispatchGetPipelinePropertiesEXT(VkDevice device, const VkPipelineInfoEXT* pPipelineInfo,
                                          VkBaseOutStructure* pPipelineProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);

    VkResult result = layer_data->device_dispatch_table.GetPipelinePropertiesEXT(device, pPipelineInfo, pPipelineProperties);

    return result;
}

void DispatchCmdSetPatchControlPointsEXT(VkCommandBuffer commandBuffer, uint32_t patchControlPoints) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdSetPatchControlPointsEXT(commandBuffer, patchControlPoints);
}

void DispatchCmdSetRasterizerDiscardEnableEXT(VkCommandBuffer commandBuffer, VkBool32 rasterizerDiscardEnable) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdSetRasterizerDiscardEnableEXT(commandBuffer, rasterizerDiscardEnable);
}

void DispatchCmdSetDepthBiasEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthBiasEnable) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdSetDepthBiasEnableEXT(commandBuffer, depthBiasEnable);
}

void DispatchCmdSetLogicOpEXT(VkCommandBuffer commandBuffer, VkLogicOp logicOp) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdSetLogicOpEXT(commandBuffer, logicOp);
}

void DispatchCmdSetPrimitiveRestartEnableEXT(VkCommandBuffer commandBuffer, VkBool32 primitiveRestartEnable) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdSetPrimitiveRestartEnableEXT(commandBuffer, primitiveRestartEnable);
}
#ifdef VK_USE_PLATFORM_SCREEN_QNX

VkResult DispatchCreateScreenSurfaceQNX(VkInstance instance, const VkScreenSurfaceCreateInfoQNX* pCreateInfo,
                                        const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(instance), layer_data_map);
    if (!wrap_handles)
        return layer_data->instance_dispatch_table.CreateScreenSurfaceQNX(instance, pCreateInfo, pAllocator, pSurface);

    VkResult result = layer_data->instance_dispatch_table.CreateScreenSurfaceQNX(instance, pCreateInfo, pAllocator, pSurface);
    if (VK_SUCCESS == result) {
        *pSurface = layer_data->WrapNew(*pSurface);
    }
    return result;
}
#endif  // VK_USE_PLATFORM_SCREEN_QNX
#ifdef VK_USE_PLATFORM_SCREEN_QNX

VkBool32 DispatchGetPhysicalDeviceScreenPresentationSupportQNX(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
                                                               struct _screen_window* window) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);

    VkBool32 result =
        layer_data->instance_dispatch_table.GetPhysicalDeviceScreenPresentationSupportQNX(physicalDevice, queueFamilyIndex, window);

    return result;
}
#endif  // VK_USE_PLATFORM_SCREEN_QNX

void DispatchCmdSetColorWriteEnableEXT(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
                                       const VkBool32* pColorWriteEnables) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdSetColorWriteEnableEXT(commandBuffer, attachmentCount, pColorWriteEnables);
}

void DispatchCmdDrawMultiEXT(VkCommandBuffer commandBuffer, uint32_t drawCount, const VkMultiDrawInfoEXT* pVertexInfo,
                             uint32_t instanceCount, uint32_t firstInstance, uint32_t stride) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdDrawMultiEXT(commandBuffer, drawCount, pVertexInfo, instanceCount, firstInstance, stride);
}

void DispatchCmdDrawMultiIndexedEXT(VkCommandBuffer commandBuffer, uint32_t drawCount, const VkMultiDrawIndexedInfoEXT* pIndexInfo,
                                    uint32_t instanceCount, uint32_t firstInstance, uint32_t stride, const int32_t* pVertexOffset) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdDrawMultiIndexedEXT(commandBuffer, drawCount, pIndexInfo, instanceCount, firstInstance,
                                                             stride, pVertexOffset);
}

VkResult DispatchCreateMicromapEXT(VkDevice device, const VkMicromapCreateInfoEXT* pCreateInfo,
                                   const VkAllocationCallbacks* pAllocator, VkMicromapEXT* pMicromap) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CreateMicromapEXT(device, pCreateInfo, pAllocator, pMicromap);
    safe_VkMicromapCreateInfoEXT var_local_pCreateInfo;
    safe_VkMicromapCreateInfoEXT* local_pCreateInfo = nullptr;
    {
        if (pCreateInfo) {
            local_pCreateInfo = &var_local_pCreateInfo;
            local_pCreateInfo->initialize(pCreateInfo);

            if (pCreateInfo->buffer) {
                local_pCreateInfo->buffer = layer_data->Unwrap(pCreateInfo->buffer);
            }
        }
    }
    VkResult result = layer_data->device_dispatch_table.CreateMicromapEXT(device, (const VkMicromapCreateInfoEXT*)local_pCreateInfo,
                                                                          pAllocator, pMicromap);
    if (VK_SUCCESS == result) {
        *pMicromap = layer_data->WrapNew(*pMicromap);
    }
    return result;
}

void DispatchDestroyMicromapEXT(VkDevice device, VkMicromapEXT micromap, const VkAllocationCallbacks* pAllocator) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.DestroyMicromapEXT(device, micromap, pAllocator);

    uint64_t micromap_id = CastToUint64(micromap);
    auto iter = unique_id_mapping.pop(micromap_id);
    if (iter != unique_id_mapping.end()) {
        micromap = (VkMicromapEXT)iter->second;
    } else {
        micromap = (VkMicromapEXT)0;
    }
    layer_data->device_dispatch_table.DestroyMicromapEXT(device, micromap, pAllocator);
}

void DispatchCmdBuildMicromapsEXT(VkCommandBuffer commandBuffer, uint32_t infoCount, const VkMicromapBuildInfoEXT* pInfos) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CmdBuildMicromapsEXT(commandBuffer, infoCount, pInfos);
    safe_VkMicromapBuildInfoEXT* local_pInfos = nullptr;
    {
        if (pInfos) {
            local_pInfos = new safe_VkMicromapBuildInfoEXT[infoCount];
            for (uint32_t index0 = 0; index0 < infoCount; ++index0) {
                local_pInfos[index0].initialize(&pInfos[index0]);

                if (pInfos[index0].dstMicromap) {
                    local_pInfos[index0].dstMicromap = layer_data->Unwrap(pInfos[index0].dstMicromap);
                }
            }
        }
    }
    layer_data->device_dispatch_table.CmdBuildMicromapsEXT(commandBuffer, infoCount, (const VkMicromapBuildInfoEXT*)local_pInfos);
    if (local_pInfos) {
        delete[] local_pInfos;
    }
}

VkResult DispatchBuildMicromapsEXT(VkDevice device, VkDeferredOperationKHR deferredOperation, uint32_t infoCount,
                                   const VkMicromapBuildInfoEXT* pInfos) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.BuildMicromapsEXT(device, deferredOperation, infoCount, pInfos);
    safe_VkMicromapBuildInfoEXT* local_pInfos = nullptr;
    {
        deferredOperation = layer_data->Unwrap(deferredOperation);
        if (pInfos) {
            local_pInfos = new safe_VkMicromapBuildInfoEXT[infoCount];
            for (uint32_t index0 = 0; index0 < infoCount; ++index0) {
                local_pInfos[index0].initialize(&pInfos[index0]);

                if (pInfos[index0].dstMicromap) {
                    local_pInfos[index0].dstMicromap = layer_data->Unwrap(pInfos[index0].dstMicromap);
                }
            }
        }
    }
    VkResult result = layer_data->device_dispatch_table.BuildMicromapsEXT(device, deferredOperation, infoCount,
                                                                          (const VkMicromapBuildInfoEXT*)local_pInfos);
    if (local_pInfos) {
        // Fix check for deferred ray tracing pipeline creation
        // https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/5817
        const bool is_operation_deferred = (deferredOperation != VK_NULL_HANDLE) && (result == VK_OPERATION_DEFERRED_KHR);
        if (is_operation_deferred) {
            std::vector<std::function<void()>> cleanup{[local_pInfos]() { delete[] local_pInfos; }};
            layer_data->deferred_operation_post_completion.insert(deferredOperation, cleanup);
        } else {
            delete[] local_pInfos;
        }
    }
    return result;
}

VkResult DispatchCopyMicromapEXT(VkDevice device, VkDeferredOperationKHR deferredOperation, const VkCopyMicromapInfoEXT* pInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CopyMicromapEXT(device, deferredOperation, pInfo);
    safe_VkCopyMicromapInfoEXT* local_pInfo = nullptr;
    {
        deferredOperation = layer_data->Unwrap(deferredOperation);
        if (pInfo) {
            local_pInfo = new safe_VkCopyMicromapInfoEXT;
            local_pInfo->initialize(pInfo);

            if (pInfo->src) {
                local_pInfo->src = layer_data->Unwrap(pInfo->src);
            }
            if (pInfo->dst) {
                local_pInfo->dst = layer_data->Unwrap(pInfo->dst);
            }
        }
    }
    VkResult result =
        layer_data->device_dispatch_table.CopyMicromapEXT(device, deferredOperation, (const VkCopyMicromapInfoEXT*)local_pInfo);
    if (local_pInfo) {
        // Fix check for deferred ray tracing pipeline creation
        // https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/5817
        const bool is_operation_deferred = (deferredOperation != VK_NULL_HANDLE) && (result == VK_OPERATION_DEFERRED_KHR);
        if (is_operation_deferred) {
            std::vector<std::function<void()>> cleanup{[local_pInfo]() { delete local_pInfo; }};
            layer_data->deferred_operation_post_completion.insert(deferredOperation, cleanup);
        } else {
            delete local_pInfo;
        }
    }
    return result;
}

VkResult DispatchCopyMicromapToMemoryEXT(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                         const VkCopyMicromapToMemoryInfoEXT* pInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CopyMicromapToMemoryEXT(device, deferredOperation, pInfo);
    safe_VkCopyMicromapToMemoryInfoEXT* local_pInfo = nullptr;
    {
        deferredOperation = layer_data->Unwrap(deferredOperation);
        if (pInfo) {
            local_pInfo = new safe_VkCopyMicromapToMemoryInfoEXT;
            local_pInfo->initialize(pInfo);

            if (pInfo->src) {
                local_pInfo->src = layer_data->Unwrap(pInfo->src);
            }
        }
    }
    VkResult result = layer_data->device_dispatch_table.CopyMicromapToMemoryEXT(device, deferredOperation,
                                                                                (const VkCopyMicromapToMemoryInfoEXT*)local_pInfo);
    if (local_pInfo) {
        // Fix check for deferred ray tracing pipeline creation
        // https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/5817
        const bool is_operation_deferred = (deferredOperation != VK_NULL_HANDLE) && (result == VK_OPERATION_DEFERRED_KHR);
        if (is_operation_deferred) {
            std::vector<std::function<void()>> cleanup{[local_pInfo]() { delete local_pInfo; }};
            layer_data->deferred_operation_post_completion.insert(deferredOperation, cleanup);
        } else {
            delete local_pInfo;
        }
    }
    return result;
}

VkResult DispatchCopyMemoryToMicromapEXT(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                         const VkCopyMemoryToMicromapInfoEXT* pInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CopyMemoryToMicromapEXT(device, deferredOperation, pInfo);
    safe_VkCopyMemoryToMicromapInfoEXT* local_pInfo = nullptr;
    {
        deferredOperation = layer_data->Unwrap(deferredOperation);
        if (pInfo) {
            local_pInfo = new safe_VkCopyMemoryToMicromapInfoEXT;
            local_pInfo->initialize(pInfo);

            if (pInfo->dst) {
                local_pInfo->dst = layer_data->Unwrap(pInfo->dst);
            }
        }
    }
    VkResult result = layer_data->device_dispatch_table.CopyMemoryToMicromapEXT(device, deferredOperation,
                                                                                (const VkCopyMemoryToMicromapInfoEXT*)local_pInfo);
    if (local_pInfo) {
        // Fix check for deferred ray tracing pipeline creation
        // https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/5817
        const bool is_operation_deferred = (deferredOperation != VK_NULL_HANDLE) && (result == VK_OPERATION_DEFERRED_KHR);
        if (is_operation_deferred) {
            std::vector<std::function<void()>> cleanup{[local_pInfo]() { delete local_pInfo; }};
            layer_data->deferred_operation_post_completion.insert(deferredOperation, cleanup);
        } else {
            delete local_pInfo;
        }
    }
    return result;
}

VkResult DispatchWriteMicromapsPropertiesEXT(VkDevice device, uint32_t micromapCount, const VkMicromapEXT* pMicromaps,
                                             VkQueryType queryType, size_t dataSize, void* pData, size_t stride) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.WriteMicromapsPropertiesEXT(device, micromapCount, pMicromaps, queryType, dataSize,
                                                                             pData, stride);
    VkMicromapEXT var_local_pMicromaps[DISPATCH_MAX_STACK_ALLOCATIONS];
    VkMicromapEXT* local_pMicromaps = nullptr;
    {
        if (pMicromaps) {
            local_pMicromaps =
                micromapCount > DISPATCH_MAX_STACK_ALLOCATIONS ? new VkMicromapEXT[micromapCount] : var_local_pMicromaps;
            for (uint32_t index0 = 0; index0 < micromapCount; ++index0) {
                local_pMicromaps[index0] = layer_data->Unwrap(pMicromaps[index0]);
            }
        }
    }
    VkResult result = layer_data->device_dispatch_table.WriteMicromapsPropertiesEXT(
        device, micromapCount, (const VkMicromapEXT*)local_pMicromaps, queryType, dataSize, pData, stride);
    if (local_pMicromaps != var_local_pMicromaps) delete[] local_pMicromaps;
    return result;
}

void DispatchCmdCopyMicromapEXT(VkCommandBuffer commandBuffer, const VkCopyMicromapInfoEXT* pInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CmdCopyMicromapEXT(commandBuffer, pInfo);
    safe_VkCopyMicromapInfoEXT var_local_pInfo;
    safe_VkCopyMicromapInfoEXT* local_pInfo = nullptr;
    {
        if (pInfo) {
            local_pInfo = &var_local_pInfo;
            local_pInfo->initialize(pInfo);

            if (pInfo->src) {
                local_pInfo->src = layer_data->Unwrap(pInfo->src);
            }
            if (pInfo->dst) {
                local_pInfo->dst = layer_data->Unwrap(pInfo->dst);
            }
        }
    }
    layer_data->device_dispatch_table.CmdCopyMicromapEXT(commandBuffer, (const VkCopyMicromapInfoEXT*)local_pInfo);
}

void DispatchCmdCopyMicromapToMemoryEXT(VkCommandBuffer commandBuffer, const VkCopyMicromapToMemoryInfoEXT* pInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CmdCopyMicromapToMemoryEXT(commandBuffer, pInfo);
    safe_VkCopyMicromapToMemoryInfoEXT var_local_pInfo;
    safe_VkCopyMicromapToMemoryInfoEXT* local_pInfo = nullptr;
    {
        if (pInfo) {
            local_pInfo = &var_local_pInfo;
            local_pInfo->initialize(pInfo);

            if (pInfo->src) {
                local_pInfo->src = layer_data->Unwrap(pInfo->src);
            }
        }
    }
    layer_data->device_dispatch_table.CmdCopyMicromapToMemoryEXT(commandBuffer, (const VkCopyMicromapToMemoryInfoEXT*)local_pInfo);
}

void DispatchCmdCopyMemoryToMicromapEXT(VkCommandBuffer commandBuffer, const VkCopyMemoryToMicromapInfoEXT* pInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CmdCopyMemoryToMicromapEXT(commandBuffer, pInfo);
    safe_VkCopyMemoryToMicromapInfoEXT var_local_pInfo;
    safe_VkCopyMemoryToMicromapInfoEXT* local_pInfo = nullptr;
    {
        if (pInfo) {
            local_pInfo = &var_local_pInfo;
            local_pInfo->initialize(pInfo);

            if (pInfo->dst) {
                local_pInfo->dst = layer_data->Unwrap(pInfo->dst);
            }
        }
    }
    layer_data->device_dispatch_table.CmdCopyMemoryToMicromapEXT(commandBuffer, (const VkCopyMemoryToMicromapInfoEXT*)local_pInfo);
}

void DispatchCmdWriteMicromapsPropertiesEXT(VkCommandBuffer commandBuffer, uint32_t micromapCount, const VkMicromapEXT* pMicromaps,
                                            VkQueryType queryType, VkQueryPool queryPool, uint32_t firstQuery) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.CmdWriteMicromapsPropertiesEXT(commandBuffer, micromapCount, pMicromaps, queryType,
                                                                                queryPool, firstQuery);
    VkMicromapEXT var_local_pMicromaps[DISPATCH_MAX_STACK_ALLOCATIONS];
    VkMicromapEXT* local_pMicromaps = nullptr;
    {
        if (pMicromaps) {
            local_pMicromaps =
                micromapCount > DISPATCH_MAX_STACK_ALLOCATIONS ? new VkMicromapEXT[micromapCount] : var_local_pMicromaps;
            for (uint32_t index0 = 0; index0 < micromapCount; ++index0) {
                local_pMicromaps[index0] = layer_data->Unwrap(pMicromaps[index0]);
            }
        }
        queryPool = layer_data->Unwrap(queryPool);
    }
    layer_data->device_dispatch_table.CmdWriteMicromapsPropertiesEXT(
        commandBuffer, micromapCount, (const VkMicromapEXT*)local_pMicromaps, queryType, queryPool, firstQuery);
    if (local_pMicromaps != var_local_pMicromaps) delete[] local_pMicromaps;
}

void DispatchGetDeviceMicromapCompatibilityEXT(VkDevice device, const VkMicromapVersionInfoEXT* pVersionInfo,
                                               VkAccelerationStructureCompatibilityKHR* pCompatibility) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);

    layer_data->device_dispatch_table.GetDeviceMicromapCompatibilityEXT(device, pVersionInfo, pCompatibility);
}

void DispatchGetMicromapBuildSizesEXT(VkDevice device, VkAccelerationStructureBuildTypeKHR buildType,
                                      const VkMicromapBuildInfoEXT* pBuildInfo, VkMicromapBuildSizesInfoEXT* pSizeInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.GetMicromapBuildSizesEXT(device, buildType, pBuildInfo, pSizeInfo);
    safe_VkMicromapBuildInfoEXT var_local_pBuildInfo;
    safe_VkMicromapBuildInfoEXT* local_pBuildInfo = nullptr;
    {
        if (pBuildInfo) {
            local_pBuildInfo = &var_local_pBuildInfo;
            local_pBuildInfo->initialize(pBuildInfo);

            if (pBuildInfo->dstMicromap) {
                local_pBuildInfo->dstMicromap = layer_data->Unwrap(pBuildInfo->dstMicromap);
            }
        }
    }
    layer_data->device_dispatch_table.GetMicromapBuildSizesEXT(device, buildType, (const VkMicromapBuildInfoEXT*)local_pBuildInfo,
                                                               pSizeInfo);
}

void DispatchCmdDrawClusterHUAWEI(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdDrawClusterHUAWEI(commandBuffer, groupCountX, groupCountY, groupCountZ);
}

void DispatchCmdDrawClusterIndirectHUAWEI(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CmdDrawClusterIndirectHUAWEI(commandBuffer, buffer, offset);
    { buffer = layer_data->Unwrap(buffer); }
    layer_data->device_dispatch_table.CmdDrawClusterIndirectHUAWEI(commandBuffer, buffer, offset);
}

void DispatchSetDeviceMemoryPriorityEXT(VkDevice device, VkDeviceMemory memory, float priority) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.SetDeviceMemoryPriorityEXT(device, memory, priority);
    { memory = layer_data->Unwrap(memory); }
    layer_data->device_dispatch_table.SetDeviceMemoryPriorityEXT(device, memory, priority);
}

void DispatchGetDescriptorSetLayoutHostMappingInfoVALVE(VkDevice device,
                                                        const VkDescriptorSetBindingReferenceVALVE* pBindingReference,
                                                        VkDescriptorSetLayoutHostMappingInfoVALVE* pHostMapping) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.GetDescriptorSetLayoutHostMappingInfoVALVE(device, pBindingReference,
                                                                                            pHostMapping);
    safe_VkDescriptorSetBindingReferenceVALVE var_local_pBindingReference;
    safe_VkDescriptorSetBindingReferenceVALVE* local_pBindingReference = nullptr;
    {
        if (pBindingReference) {
            local_pBindingReference = &var_local_pBindingReference;
            local_pBindingReference->initialize(pBindingReference);

            if (pBindingReference->descriptorSetLayout) {
                local_pBindingReference->descriptorSetLayout = layer_data->Unwrap(pBindingReference->descriptorSetLayout);
            }
        }
    }
    layer_data->device_dispatch_table.GetDescriptorSetLayoutHostMappingInfoVALVE(
        device, (const VkDescriptorSetBindingReferenceVALVE*)local_pBindingReference, pHostMapping);
}

void DispatchGetDescriptorSetHostMappingVALVE(VkDevice device, VkDescriptorSet descriptorSet, void** ppData) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.GetDescriptorSetHostMappingVALVE(device, descriptorSet, ppData);
    { descriptorSet = layer_data->Unwrap(descriptorSet); }
    layer_data->device_dispatch_table.GetDescriptorSetHostMappingVALVE(device, descriptorSet, ppData);
}

void DispatchCmdCopyMemoryIndirectNV(VkCommandBuffer commandBuffer, VkDeviceAddress copyBufferAddress, uint32_t copyCount,
                                     uint32_t stride) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdCopyMemoryIndirectNV(commandBuffer, copyBufferAddress, copyCount, stride);
}

void DispatchCmdCopyMemoryToImageIndirectNV(VkCommandBuffer commandBuffer, VkDeviceAddress copyBufferAddress, uint32_t copyCount,
                                            uint32_t stride, VkImage dstImage, VkImageLayout dstImageLayout,
                                            const VkImageSubresourceLayers* pImageSubresources) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.CmdCopyMemoryToImageIndirectNV(commandBuffer, copyBufferAddress, copyCount, stride,
                                                                                dstImage, dstImageLayout, pImageSubresources);
    { dstImage = layer_data->Unwrap(dstImage); }
    layer_data->device_dispatch_table.CmdCopyMemoryToImageIndirectNV(commandBuffer, copyBufferAddress, copyCount, stride, dstImage,
                                                                     dstImageLayout, pImageSubresources);
}

void DispatchCmdDecompressMemoryNV(VkCommandBuffer commandBuffer, uint32_t decompressRegionCount,
                                   const VkDecompressMemoryRegionNV* pDecompressMemoryRegions) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdDecompressMemoryNV(commandBuffer, decompressRegionCount, pDecompressMemoryRegions);
}

void DispatchCmdDecompressMemoryIndirectCountNV(VkCommandBuffer commandBuffer, VkDeviceAddress indirectCommandsAddress,
                                                VkDeviceAddress indirectCommandsCountAddress, uint32_t stride) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdDecompressMemoryIndirectCountNV(commandBuffer, indirectCommandsAddress,
                                                                         indirectCommandsCountAddress, stride);
}

void DispatchGetPipelineIndirectMemoryRequirementsNV(VkDevice device, const VkComputePipelineCreateInfo* pCreateInfo,
                                                     VkMemoryRequirements2* pMemoryRequirements) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.GetPipelineIndirectMemoryRequirementsNV(device, pCreateInfo, pMemoryRequirements);
    safe_VkComputePipelineCreateInfo var_local_pCreateInfo;
    safe_VkComputePipelineCreateInfo* local_pCreateInfo = nullptr;
    {
        if (pCreateInfo) {
            local_pCreateInfo = &var_local_pCreateInfo;
            local_pCreateInfo->initialize(pCreateInfo);

            if (pCreateInfo->stage.module) {
                local_pCreateInfo->stage.module = layer_data->Unwrap(pCreateInfo->stage.module);
            }
            WrapPnextChainHandles(layer_data, local_pCreateInfo->stage.pNext);

            if (pCreateInfo->layout) {
                local_pCreateInfo->layout = layer_data->Unwrap(pCreateInfo->layout);
            }
            if (pCreateInfo->basePipelineHandle) {
                local_pCreateInfo->basePipelineHandle = layer_data->Unwrap(pCreateInfo->basePipelineHandle);
            }
            WrapPnextChainHandles(layer_data, local_pCreateInfo->pNext);
        }
    }
    layer_data->device_dispatch_table.GetPipelineIndirectMemoryRequirementsNV(
        device, (const VkComputePipelineCreateInfo*)local_pCreateInfo, pMemoryRequirements);
}

void DispatchCmdUpdatePipelineIndirectBufferNV(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                               VkPipeline pipeline) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.CmdUpdatePipelineIndirectBufferNV(commandBuffer, pipelineBindPoint, pipeline);
    { pipeline = layer_data->Unwrap(pipeline); }
    layer_data->device_dispatch_table.CmdUpdatePipelineIndirectBufferNV(commandBuffer, pipelineBindPoint, pipeline);
}

VkDeviceAddress DispatchGetPipelineIndirectDeviceAddressNV(VkDevice device, const VkPipelineIndirectDeviceAddressInfoNV* pInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.GetPipelineIndirectDeviceAddressNV(device, pInfo);
    safe_VkPipelineIndirectDeviceAddressInfoNV var_local_pInfo;
    safe_VkPipelineIndirectDeviceAddressInfoNV* local_pInfo = nullptr;
    {
        if (pInfo) {
            local_pInfo = &var_local_pInfo;
            local_pInfo->initialize(pInfo);

            if (pInfo->pipeline) {
                local_pInfo->pipeline = layer_data->Unwrap(pInfo->pipeline);
            }
        }
    }
    VkDeviceAddress result = layer_data->device_dispatch_table.GetPipelineIndirectDeviceAddressNV(
        device, (const VkPipelineIndirectDeviceAddressInfoNV*)local_pInfo);

    return result;
}

void DispatchCmdSetTessellationDomainOriginEXT(VkCommandBuffer commandBuffer, VkTessellationDomainOrigin domainOrigin) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdSetTessellationDomainOriginEXT(commandBuffer, domainOrigin);
}

void DispatchCmdSetDepthClampEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthClampEnable) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdSetDepthClampEnableEXT(commandBuffer, depthClampEnable);
}

void DispatchCmdSetPolygonModeEXT(VkCommandBuffer commandBuffer, VkPolygonMode polygonMode) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdSetPolygonModeEXT(commandBuffer, polygonMode);
}

void DispatchCmdSetRasterizationSamplesEXT(VkCommandBuffer commandBuffer, VkSampleCountFlagBits rasterizationSamples) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdSetRasterizationSamplesEXT(commandBuffer, rasterizationSamples);
}

void DispatchCmdSetSampleMaskEXT(VkCommandBuffer commandBuffer, VkSampleCountFlagBits samples, const VkSampleMask* pSampleMask) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdSetSampleMaskEXT(commandBuffer, samples, pSampleMask);
}

void DispatchCmdSetAlphaToCoverageEnableEXT(VkCommandBuffer commandBuffer, VkBool32 alphaToCoverageEnable) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdSetAlphaToCoverageEnableEXT(commandBuffer, alphaToCoverageEnable);
}

void DispatchCmdSetAlphaToOneEnableEXT(VkCommandBuffer commandBuffer, VkBool32 alphaToOneEnable) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdSetAlphaToOneEnableEXT(commandBuffer, alphaToOneEnable);
}

void DispatchCmdSetLogicOpEnableEXT(VkCommandBuffer commandBuffer, VkBool32 logicOpEnable) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdSetLogicOpEnableEXT(commandBuffer, logicOpEnable);
}

void DispatchCmdSetColorBlendEnableEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment, uint32_t attachmentCount,
                                       const VkBool32* pColorBlendEnables) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdSetColorBlendEnableEXT(commandBuffer, firstAttachment, attachmentCount,
                                                                pColorBlendEnables);
}

void DispatchCmdSetColorBlendEquationEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment, uint32_t attachmentCount,
                                         const VkColorBlendEquationEXT* pColorBlendEquations) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdSetColorBlendEquationEXT(commandBuffer, firstAttachment, attachmentCount,
                                                                  pColorBlendEquations);
}

void DispatchCmdSetColorWriteMaskEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment, uint32_t attachmentCount,
                                     const VkColorComponentFlags* pColorWriteMasks) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdSetColorWriteMaskEXT(commandBuffer, firstAttachment, attachmentCount, pColorWriteMasks);
}

void DispatchCmdSetRasterizationStreamEXT(VkCommandBuffer commandBuffer, uint32_t rasterizationStream) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdSetRasterizationStreamEXT(commandBuffer, rasterizationStream);
}

void DispatchCmdSetConservativeRasterizationModeEXT(VkCommandBuffer commandBuffer,
                                                    VkConservativeRasterizationModeEXT conservativeRasterizationMode) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdSetConservativeRasterizationModeEXT(commandBuffer, conservativeRasterizationMode);
}

void DispatchCmdSetExtraPrimitiveOverestimationSizeEXT(VkCommandBuffer commandBuffer, float extraPrimitiveOverestimationSize) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdSetExtraPrimitiveOverestimationSizeEXT(commandBuffer, extraPrimitiveOverestimationSize);
}

void DispatchCmdSetDepthClipEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthClipEnable) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdSetDepthClipEnableEXT(commandBuffer, depthClipEnable);
}

void DispatchCmdSetSampleLocationsEnableEXT(VkCommandBuffer commandBuffer, VkBool32 sampleLocationsEnable) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdSetSampleLocationsEnableEXT(commandBuffer, sampleLocationsEnable);
}

void DispatchCmdSetColorBlendAdvancedEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment, uint32_t attachmentCount,
                                         const VkColorBlendAdvancedEXT* pColorBlendAdvanced) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdSetColorBlendAdvancedEXT(commandBuffer, firstAttachment, attachmentCount,
                                                                  pColorBlendAdvanced);
}

void DispatchCmdSetProvokingVertexModeEXT(VkCommandBuffer commandBuffer, VkProvokingVertexModeEXT provokingVertexMode) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdSetProvokingVertexModeEXT(commandBuffer, provokingVertexMode);
}

void DispatchCmdSetLineRasterizationModeEXT(VkCommandBuffer commandBuffer, VkLineRasterizationModeEXT lineRasterizationMode) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdSetLineRasterizationModeEXT(commandBuffer, lineRasterizationMode);
}

void DispatchCmdSetLineStippleEnableEXT(VkCommandBuffer commandBuffer, VkBool32 stippledLineEnable) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdSetLineStippleEnableEXT(commandBuffer, stippledLineEnable);
}

void DispatchCmdSetDepthClipNegativeOneToOneEXT(VkCommandBuffer commandBuffer, VkBool32 negativeOneToOne) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdSetDepthClipNegativeOneToOneEXT(commandBuffer, negativeOneToOne);
}

void DispatchCmdSetViewportWScalingEnableNV(VkCommandBuffer commandBuffer, VkBool32 viewportWScalingEnable) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdSetViewportWScalingEnableNV(commandBuffer, viewportWScalingEnable);
}

void DispatchCmdSetViewportSwizzleNV(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount,
                                     const VkViewportSwizzleNV* pViewportSwizzles) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdSetViewportSwizzleNV(commandBuffer, firstViewport, viewportCount, pViewportSwizzles);
}

void DispatchCmdSetCoverageToColorEnableNV(VkCommandBuffer commandBuffer, VkBool32 coverageToColorEnable) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdSetCoverageToColorEnableNV(commandBuffer, coverageToColorEnable);
}

void DispatchCmdSetCoverageToColorLocationNV(VkCommandBuffer commandBuffer, uint32_t coverageToColorLocation) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdSetCoverageToColorLocationNV(commandBuffer, coverageToColorLocation);
}

void DispatchCmdSetCoverageModulationModeNV(VkCommandBuffer commandBuffer, VkCoverageModulationModeNV coverageModulationMode) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdSetCoverageModulationModeNV(commandBuffer, coverageModulationMode);
}

void DispatchCmdSetCoverageModulationTableEnableNV(VkCommandBuffer commandBuffer, VkBool32 coverageModulationTableEnable) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdSetCoverageModulationTableEnableNV(commandBuffer, coverageModulationTableEnable);
}

void DispatchCmdSetCoverageModulationTableNV(VkCommandBuffer commandBuffer, uint32_t coverageModulationTableCount,
                                             const float* pCoverageModulationTable) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdSetCoverageModulationTableNV(commandBuffer, coverageModulationTableCount,
                                                                      pCoverageModulationTable);
}

void DispatchCmdSetShadingRateImageEnableNV(VkCommandBuffer commandBuffer, VkBool32 shadingRateImageEnable) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdSetShadingRateImageEnableNV(commandBuffer, shadingRateImageEnable);
}

void DispatchCmdSetRepresentativeFragmentTestEnableNV(VkCommandBuffer commandBuffer, VkBool32 representativeFragmentTestEnable) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdSetRepresentativeFragmentTestEnableNV(commandBuffer, representativeFragmentTestEnable);
}

void DispatchCmdSetCoverageReductionModeNV(VkCommandBuffer commandBuffer, VkCoverageReductionModeNV coverageReductionMode) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdSetCoverageReductionModeNV(commandBuffer, coverageReductionMode);
}

void DispatchGetShaderModuleIdentifierEXT(VkDevice device, VkShaderModule shaderModule, VkShaderModuleIdentifierEXT* pIdentifier) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.GetShaderModuleIdentifierEXT(device, shaderModule, pIdentifier);
    { shaderModule = layer_data->Unwrap(shaderModule); }
    layer_data->device_dispatch_table.GetShaderModuleIdentifierEXT(device, shaderModule, pIdentifier);
}

void DispatchGetShaderModuleCreateInfoIdentifierEXT(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo,
                                                    VkShaderModuleIdentifierEXT* pIdentifier) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.GetShaderModuleCreateInfoIdentifierEXT(device, pCreateInfo, pIdentifier);
    safe_VkShaderModuleCreateInfo var_local_pCreateInfo;
    safe_VkShaderModuleCreateInfo* local_pCreateInfo = nullptr;
    {
        if (pCreateInfo) {
            local_pCreateInfo = &var_local_pCreateInfo;
            local_pCreateInfo->initialize(pCreateInfo);
            WrapPnextChainHandles(layer_data, local_pCreateInfo->pNext);
        }
    }
    layer_data->device_dispatch_table.GetShaderModuleCreateInfoIdentifierEXT(
        device, (const VkShaderModuleCreateInfo*)local_pCreateInfo, pIdentifier);
}

VkResult DispatchGetPhysicalDeviceOpticalFlowImageFormatsNV(VkPhysicalDevice physicalDevice,
                                                            const VkOpticalFlowImageFormatInfoNV* pOpticalFlowImageFormatInfo,
                                                            uint32_t* pFormatCount,
                                                            VkOpticalFlowImageFormatPropertiesNV* pImageFormatProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);

    VkResult result = layer_data->instance_dispatch_table.GetPhysicalDeviceOpticalFlowImageFormatsNV(
        physicalDevice, pOpticalFlowImageFormatInfo, pFormatCount, pImageFormatProperties);

    return result;
}

VkResult DispatchCreateOpticalFlowSessionNV(VkDevice device, const VkOpticalFlowSessionCreateInfoNV* pCreateInfo,
                                            const VkAllocationCallbacks* pAllocator, VkOpticalFlowSessionNV* pSession) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.CreateOpticalFlowSessionNV(device, pCreateInfo, pAllocator, pSession);

    VkResult result = layer_data->device_dispatch_table.CreateOpticalFlowSessionNV(device, pCreateInfo, pAllocator, pSession);
    if (VK_SUCCESS == result) {
        *pSession = layer_data->WrapNew(*pSession);
    }
    return result;
}

void DispatchDestroyOpticalFlowSessionNV(VkDevice device, VkOpticalFlowSessionNV session, const VkAllocationCallbacks* pAllocator) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.DestroyOpticalFlowSessionNV(device, session, pAllocator);

    uint64_t session_id = CastToUint64(session);
    auto iter = unique_id_mapping.pop(session_id);
    if (iter != unique_id_mapping.end()) {
        session = (VkOpticalFlowSessionNV)iter->second;
    } else {
        session = (VkOpticalFlowSessionNV)0;
    }
    layer_data->device_dispatch_table.DestroyOpticalFlowSessionNV(device, session, pAllocator);
}

VkResult DispatchBindOpticalFlowSessionImageNV(VkDevice device, VkOpticalFlowSessionNV session,
                                               VkOpticalFlowSessionBindingPointNV bindingPoint, VkImageView view,
                                               VkImageLayout layout) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.BindOpticalFlowSessionImageNV(device, session, bindingPoint, view, layout);
    {
        session = layer_data->Unwrap(session);
        view = layer_data->Unwrap(view);
    }
    VkResult result = layer_data->device_dispatch_table.BindOpticalFlowSessionImageNV(device, session, bindingPoint, view, layout);

    return result;
}

void DispatchCmdOpticalFlowExecuteNV(VkCommandBuffer commandBuffer, VkOpticalFlowSessionNV session,
                                     const VkOpticalFlowExecuteInfoNV* pExecuteInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CmdOpticalFlowExecuteNV(commandBuffer, session, pExecuteInfo);
    { session = layer_data->Unwrap(session); }
    layer_data->device_dispatch_table.CmdOpticalFlowExecuteNV(commandBuffer, session, pExecuteInfo);
}

VkResult DispatchCreateShadersEXT(VkDevice device, uint32_t createInfoCount, const VkShaderCreateInfoEXT* pCreateInfos,
                                  const VkAllocationCallbacks* pAllocator, VkShaderEXT* pShaders) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.CreateShadersEXT(device, createInfoCount, pCreateInfos, pAllocator, pShaders);
    safe_VkShaderCreateInfoEXT* local_pCreateInfos = nullptr;
    {
        if (pCreateInfos) {
            local_pCreateInfos = new safe_VkShaderCreateInfoEXT[createInfoCount];
            for (uint32_t index0 = 0; index0 < createInfoCount; ++index0) {
                local_pCreateInfos[index0].initialize(&pCreateInfos[index0]);
                if (local_pCreateInfos[index0].pSetLayouts) {
                    for (uint32_t index1 = 0; index1 < local_pCreateInfos[index0].setLayoutCount; ++index1) {
                        local_pCreateInfos[index0].pSetLayouts[index1] =
                            layer_data->Unwrap(local_pCreateInfos[index0].pSetLayouts[index1]);
                    }
                }
            }
        }
    }
    VkResult result = layer_data->device_dispatch_table.CreateShadersEXT(
        device, createInfoCount, (const VkShaderCreateInfoEXT*)local_pCreateInfos, pAllocator, pShaders);
    if (local_pCreateInfos) {
        delete[] local_pCreateInfos;
    }
    if (VK_SUCCESS == result) {
        for (uint32_t index0 = 0; index0 < createInfoCount; index0++) {
            pShaders[index0] = layer_data->WrapNew(pShaders[index0]);
        }
    }
    return result;
}

void DispatchDestroyShaderEXT(VkDevice device, VkShaderEXT shader, const VkAllocationCallbacks* pAllocator) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.DestroyShaderEXT(device, shader, pAllocator);

    uint64_t shader_id = CastToUint64(shader);
    auto iter = unique_id_mapping.pop(shader_id);
    if (iter != unique_id_mapping.end()) {
        shader = (VkShaderEXT)iter->second;
    } else {
        shader = (VkShaderEXT)0;
    }
    layer_data->device_dispatch_table.DestroyShaderEXT(device, shader, pAllocator);
}

VkResult DispatchGetShaderBinaryDataEXT(VkDevice device, VkShaderEXT shader, size_t* pDataSize, void* pData) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.GetShaderBinaryDataEXT(device, shader, pDataSize, pData);
    { shader = layer_data->Unwrap(shader); }
    VkResult result = layer_data->device_dispatch_table.GetShaderBinaryDataEXT(device, shader, pDataSize, pData);

    return result;
}

void DispatchCmdBindShadersEXT(VkCommandBuffer commandBuffer, uint32_t stageCount, const VkShaderStageFlagBits* pStages,
                               const VkShaderEXT* pShaders) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CmdBindShadersEXT(commandBuffer, stageCount, pStages, pShaders);
    VkShaderEXT var_local_pShaders[DISPATCH_MAX_STACK_ALLOCATIONS];
    VkShaderEXT* local_pShaders = nullptr;
    {
        if (pShaders) {
            local_pShaders = stageCount > DISPATCH_MAX_STACK_ALLOCATIONS ? new VkShaderEXT[stageCount] : var_local_pShaders;
            for (uint32_t index0 = 0; index0 < stageCount; ++index0) {
                local_pShaders[index0] = layer_data->Unwrap(pShaders[index0]);
            }
        }
    }
    layer_data->device_dispatch_table.CmdBindShadersEXT(commandBuffer, stageCount, pStages, (const VkShaderEXT*)local_pShaders);
    if (local_pShaders != var_local_pShaders) delete[] local_pShaders;
}

VkResult DispatchGetFramebufferTilePropertiesQCOM(VkDevice device, VkFramebuffer framebuffer, uint32_t* pPropertiesCount,
                                                  VkTilePropertiesQCOM* pProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.GetFramebufferTilePropertiesQCOM(device, framebuffer, pPropertiesCount,
                                                                                  pProperties);
    { framebuffer = layer_data->Unwrap(framebuffer); }
    VkResult result =
        layer_data->device_dispatch_table.GetFramebufferTilePropertiesQCOM(device, framebuffer, pPropertiesCount, pProperties);

    return result;
}

VkResult DispatchGetDynamicRenderingTilePropertiesQCOM(VkDevice device, const VkRenderingInfo* pRenderingInfo,
                                                       VkTilePropertiesQCOM* pProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.GetDynamicRenderingTilePropertiesQCOM(device, pRenderingInfo, pProperties);
    safe_VkRenderingInfo var_local_pRenderingInfo;
    safe_VkRenderingInfo* local_pRenderingInfo = nullptr;
    {
        if (pRenderingInfo) {
            local_pRenderingInfo = &var_local_pRenderingInfo;
            local_pRenderingInfo->initialize(pRenderingInfo);
            if (local_pRenderingInfo->pColorAttachments) {
                for (uint32_t index1 = 0; index1 < local_pRenderingInfo->colorAttachmentCount; ++index1) {
                    if (pRenderingInfo->pColorAttachments[index1].imageView) {
                        local_pRenderingInfo->pColorAttachments[index1].imageView =
                            layer_data->Unwrap(pRenderingInfo->pColorAttachments[index1].imageView);
                    }
                    if (pRenderingInfo->pColorAttachments[index1].resolveImageView) {
                        local_pRenderingInfo->pColorAttachments[index1].resolveImageView =
                            layer_data->Unwrap(pRenderingInfo->pColorAttachments[index1].resolveImageView);
                    }
                }
            }
            if (local_pRenderingInfo->pDepthAttachment) {
                if (pRenderingInfo->pDepthAttachment->imageView) {
                    local_pRenderingInfo->pDepthAttachment->imageView =
                        layer_data->Unwrap(pRenderingInfo->pDepthAttachment->imageView);
                }
                if (pRenderingInfo->pDepthAttachment->resolveImageView) {
                    local_pRenderingInfo->pDepthAttachment->resolveImageView =
                        layer_data->Unwrap(pRenderingInfo->pDepthAttachment->resolveImageView);
                }
            }
            if (local_pRenderingInfo->pStencilAttachment) {
                if (pRenderingInfo->pStencilAttachment->imageView) {
                    local_pRenderingInfo->pStencilAttachment->imageView =
                        layer_data->Unwrap(pRenderingInfo->pStencilAttachment->imageView);
                }
                if (pRenderingInfo->pStencilAttachment->resolveImageView) {
                    local_pRenderingInfo->pStencilAttachment->resolveImageView =
                        layer_data->Unwrap(pRenderingInfo->pStencilAttachment->resolveImageView);
                }
            }
            WrapPnextChainHandles(layer_data, local_pRenderingInfo->pNext);
        }
    }
    VkResult result = layer_data->device_dispatch_table.GetDynamicRenderingTilePropertiesQCOM(
        device, (const VkRenderingInfo*)local_pRenderingInfo, pProperties);

    return result;
}

VkResult DispatchSetLatencySleepModeNV(VkDevice device, VkSwapchainKHR swapchain, const VkLatencySleepModeInfoNV* pSleepModeInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.SetLatencySleepModeNV(device, swapchain, pSleepModeInfo);
    { swapchain = layer_data->Unwrap(swapchain); }
    VkResult result = layer_data->device_dispatch_table.SetLatencySleepModeNV(device, swapchain, pSleepModeInfo);

    return result;
}

VkResult DispatchLatencySleepNV(VkDevice device, VkSwapchainKHR swapchain, const VkLatencySleepInfoNV* pSleepInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.LatencySleepNV(device, swapchain, pSleepInfo);
    safe_VkLatencySleepInfoNV var_local_pSleepInfo;
    safe_VkLatencySleepInfoNV* local_pSleepInfo = nullptr;
    {
        swapchain = layer_data->Unwrap(swapchain);
        if (pSleepInfo) {
            local_pSleepInfo = &var_local_pSleepInfo;
            local_pSleepInfo->initialize(pSleepInfo);

            if (pSleepInfo->signalSemaphore) {
                local_pSleepInfo->signalSemaphore = layer_data->Unwrap(pSleepInfo->signalSemaphore);
            }
        }
    }
    VkResult result =
        layer_data->device_dispatch_table.LatencySleepNV(device, swapchain, (const VkLatencySleepInfoNV*)local_pSleepInfo);

    return result;
}

void DispatchSetLatencyMarkerNV(VkDevice device, VkSwapchainKHR swapchain, const VkSetLatencyMarkerInfoNV* pLatencyMarkerInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.SetLatencyMarkerNV(device, swapchain, pLatencyMarkerInfo);
    { swapchain = layer_data->Unwrap(swapchain); }
    layer_data->device_dispatch_table.SetLatencyMarkerNV(device, swapchain, pLatencyMarkerInfo);
}

void DispatchGetLatencyTimingsNV(VkDevice device, VkSwapchainKHR swapchain, uint32_t* pTimingCount,
                                 VkGetLatencyMarkerInfoNV* pLatencyMarkerInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.GetLatencyTimingsNV(device, swapchain, pTimingCount, pLatencyMarkerInfo);
    { swapchain = layer_data->Unwrap(swapchain); }
    layer_data->device_dispatch_table.GetLatencyTimingsNV(device, swapchain, pTimingCount, pLatencyMarkerInfo);
}

void DispatchQueueNotifyOutOfBandNV(VkQueue queue, const VkOutOfBandQueueTypeInfoNV* pQueueTypeInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(queue), layer_data_map);

    layer_data->device_dispatch_table.QueueNotifyOutOfBandNV(queue, pQueueTypeInfo);
}

void DispatchCmdSetAttachmentFeedbackLoopEnableEXT(VkCommandBuffer commandBuffer, VkImageAspectFlags aspectMask) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdSetAttachmentFeedbackLoopEnableEXT(commandBuffer, aspectMask);
}
#ifdef VK_USE_PLATFORM_SCREEN_QNX

VkResult DispatchGetScreenBufferPropertiesQNX(VkDevice device, const struct _screen_buffer* buffer,
                                              VkScreenBufferPropertiesQNX* pProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);

    VkResult result = layer_data->device_dispatch_table.GetScreenBufferPropertiesQNX(device, buffer, pProperties);

    return result;
}
#endif  // VK_USE_PLATFORM_SCREEN_QNX

VkResult DispatchCreateAccelerationStructureKHR(VkDevice device, const VkAccelerationStructureCreateInfoKHR* pCreateInfo,
                                                const VkAllocationCallbacks* pAllocator,
                                                VkAccelerationStructureKHR* pAccelerationStructure) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.CreateAccelerationStructureKHR(device, pCreateInfo, pAllocator,
                                                                                pAccelerationStructure);
    safe_VkAccelerationStructureCreateInfoKHR var_local_pCreateInfo;
    safe_VkAccelerationStructureCreateInfoKHR* local_pCreateInfo = nullptr;
    {
        if (pCreateInfo) {
            local_pCreateInfo = &var_local_pCreateInfo;
            local_pCreateInfo->initialize(pCreateInfo);

            if (pCreateInfo->buffer) {
                local_pCreateInfo->buffer = layer_data->Unwrap(pCreateInfo->buffer);
            }
        }
    }
    VkResult result = layer_data->device_dispatch_table.CreateAccelerationStructureKHR(
        device, (const VkAccelerationStructureCreateInfoKHR*)local_pCreateInfo, pAllocator, pAccelerationStructure);
    if (VK_SUCCESS == result) {
        *pAccelerationStructure = layer_data->WrapNew(*pAccelerationStructure);
    }
    return result;
}

void DispatchDestroyAccelerationStructureKHR(VkDevice device, VkAccelerationStructureKHR accelerationStructure,
                                             const VkAllocationCallbacks* pAllocator) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.DestroyAccelerationStructureKHR(device, accelerationStructure, pAllocator);

    uint64_t accelerationStructure_id = CastToUint64(accelerationStructure);
    auto iter = unique_id_mapping.pop(accelerationStructure_id);
    if (iter != unique_id_mapping.end()) {
        accelerationStructure = (VkAccelerationStructureKHR)iter->second;
    } else {
        accelerationStructure = (VkAccelerationStructureKHR)0;
    }
    layer_data->device_dispatch_table.DestroyAccelerationStructureKHR(device, accelerationStructure, pAllocator);
}

void DispatchCmdBuildAccelerationStructuresKHR(VkCommandBuffer commandBuffer, uint32_t infoCount,
                                               const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
                                               const VkAccelerationStructureBuildRangeInfoKHR* const* ppBuildRangeInfos) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.CmdBuildAccelerationStructuresKHR(commandBuffer, infoCount, pInfos,
                                                                                   ppBuildRangeInfos);
    safe_VkAccelerationStructureBuildGeometryInfoKHR* local_pInfos = nullptr;
    {
        if (pInfos) {
            local_pInfos = new safe_VkAccelerationStructureBuildGeometryInfoKHR[infoCount];
            for (uint32_t index0 = 0; index0 < infoCount; ++index0) {
                local_pInfos[index0].initialize(&pInfos[index0], false, nullptr);

                if (pInfos[index0].srcAccelerationStructure) {
                    local_pInfos[index0].srcAccelerationStructure = layer_data->Unwrap(pInfos[index0].srcAccelerationStructure);
                }
                if (pInfos[index0].dstAccelerationStructure) {
                    local_pInfos[index0].dstAccelerationStructure = layer_data->Unwrap(pInfos[index0].dstAccelerationStructure);
                }
            }
        }
    }
    layer_data->device_dispatch_table.CmdBuildAccelerationStructuresKHR(
        commandBuffer, infoCount, (const VkAccelerationStructureBuildGeometryInfoKHR*)local_pInfos, ppBuildRangeInfos);
    if (local_pInfos) {
        delete[] local_pInfos;
    }
}

void DispatchCmdBuildAccelerationStructuresIndirectKHR(VkCommandBuffer commandBuffer, uint32_t infoCount,
                                                       const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
                                                       const VkDeviceAddress* pIndirectDeviceAddresses,
                                                       const uint32_t* pIndirectStrides,
                                                       const uint32_t* const* ppMaxPrimitiveCounts) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.CmdBuildAccelerationStructuresIndirectKHR(
            commandBuffer, infoCount, pInfos, pIndirectDeviceAddresses, pIndirectStrides, ppMaxPrimitiveCounts);
    safe_VkAccelerationStructureBuildGeometryInfoKHR* local_pInfos = nullptr;
    {
        if (pInfos) {
            local_pInfos = new safe_VkAccelerationStructureBuildGeometryInfoKHR[infoCount];
            for (uint32_t index0 = 0; index0 < infoCount; ++index0) {
                local_pInfos[index0].initialize(&pInfos[index0], false, nullptr);

                if (pInfos[index0].srcAccelerationStructure) {
                    local_pInfos[index0].srcAccelerationStructure = layer_data->Unwrap(pInfos[index0].srcAccelerationStructure);
                }
                if (pInfos[index0].dstAccelerationStructure) {
                    local_pInfos[index0].dstAccelerationStructure = layer_data->Unwrap(pInfos[index0].dstAccelerationStructure);
                }
            }
        }
    }
    layer_data->device_dispatch_table.CmdBuildAccelerationStructuresIndirectKHR(
        commandBuffer, infoCount, (const VkAccelerationStructureBuildGeometryInfoKHR*)local_pInfos, pIndirectDeviceAddresses,
        pIndirectStrides, ppMaxPrimitiveCounts);
    if (local_pInfos) {
        delete[] local_pInfos;
    }
}

VkResult DispatchCopyAccelerationStructureKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                              const VkCopyAccelerationStructureInfoKHR* pInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CopyAccelerationStructureKHR(device, deferredOperation, pInfo);
    safe_VkCopyAccelerationStructureInfoKHR* local_pInfo = nullptr;
    {
        deferredOperation = layer_data->Unwrap(deferredOperation);
        if (pInfo) {
            local_pInfo = new safe_VkCopyAccelerationStructureInfoKHR;
            local_pInfo->initialize(pInfo);

            if (pInfo->src) {
                local_pInfo->src = layer_data->Unwrap(pInfo->src);
            }
            if (pInfo->dst) {
                local_pInfo->dst = layer_data->Unwrap(pInfo->dst);
            }
        }
    }
    VkResult result = layer_data->device_dispatch_table.CopyAccelerationStructureKHR(
        device, deferredOperation, (const VkCopyAccelerationStructureInfoKHR*)local_pInfo);
    if (local_pInfo) {
        // Fix check for deferred ray tracing pipeline creation
        // https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/5817
        const bool is_operation_deferred = (deferredOperation != VK_NULL_HANDLE) && (result == VK_OPERATION_DEFERRED_KHR);
        if (is_operation_deferred) {
            std::vector<std::function<void()>> cleanup{[local_pInfo]() { delete local_pInfo; }};
            layer_data->deferred_operation_post_completion.insert(deferredOperation, cleanup);
        } else {
            delete local_pInfo;
        }
    }
    return result;
}

VkResult DispatchCopyAccelerationStructureToMemoryKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                      const VkCopyAccelerationStructureToMemoryInfoKHR* pInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.CopyAccelerationStructureToMemoryKHR(device, deferredOperation, pInfo);
    safe_VkCopyAccelerationStructureToMemoryInfoKHR* local_pInfo = nullptr;
    {
        deferredOperation = layer_data->Unwrap(deferredOperation);
        if (pInfo) {
            local_pInfo = new safe_VkCopyAccelerationStructureToMemoryInfoKHR;
            local_pInfo->initialize(pInfo);

            if (pInfo->src) {
                local_pInfo->src = layer_data->Unwrap(pInfo->src);
            }
        }
    }
    VkResult result = layer_data->device_dispatch_table.CopyAccelerationStructureToMemoryKHR(
        device, deferredOperation, (const VkCopyAccelerationStructureToMemoryInfoKHR*)local_pInfo);
    if (local_pInfo) {
        // Fix check for deferred ray tracing pipeline creation
        // https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/5817
        const bool is_operation_deferred = (deferredOperation != VK_NULL_HANDLE) && (result == VK_OPERATION_DEFERRED_KHR);
        if (is_operation_deferred) {
            std::vector<std::function<void()>> cleanup{[local_pInfo]() { delete local_pInfo; }};
            layer_data->deferred_operation_post_completion.insert(deferredOperation, cleanup);
        } else {
            delete local_pInfo;
        }
    }
    return result;
}

VkResult DispatchCopyMemoryToAccelerationStructureKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                      const VkCopyMemoryToAccelerationStructureInfoKHR* pInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.CopyMemoryToAccelerationStructureKHR(device, deferredOperation, pInfo);
    safe_VkCopyMemoryToAccelerationStructureInfoKHR* local_pInfo = nullptr;
    {
        deferredOperation = layer_data->Unwrap(deferredOperation);
        if (pInfo) {
            local_pInfo = new safe_VkCopyMemoryToAccelerationStructureInfoKHR;
            local_pInfo->initialize(pInfo);

            if (pInfo->dst) {
                local_pInfo->dst = layer_data->Unwrap(pInfo->dst);
            }
        }
    }
    VkResult result = layer_data->device_dispatch_table.CopyMemoryToAccelerationStructureKHR(
        device, deferredOperation, (const VkCopyMemoryToAccelerationStructureInfoKHR*)local_pInfo);
    if (local_pInfo) {
        // Fix check for deferred ray tracing pipeline creation
        // https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/5817
        const bool is_operation_deferred = (deferredOperation != VK_NULL_HANDLE) && (result == VK_OPERATION_DEFERRED_KHR);
        if (is_operation_deferred) {
            std::vector<std::function<void()>> cleanup{[local_pInfo]() { delete local_pInfo; }};
            layer_data->deferred_operation_post_completion.insert(deferredOperation, cleanup);
        } else {
            delete local_pInfo;
        }
    }
    return result;
}

VkResult DispatchWriteAccelerationStructuresPropertiesKHR(VkDevice device, uint32_t accelerationStructureCount,
                                                          const VkAccelerationStructureKHR* pAccelerationStructures,
                                                          VkQueryType queryType, size_t dataSize, void* pData, size_t stride) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.WriteAccelerationStructuresPropertiesKHR(
            device, accelerationStructureCount, pAccelerationStructures, queryType, dataSize, pData, stride);
    VkAccelerationStructureKHR var_local_pAccelerationStructures[DISPATCH_MAX_STACK_ALLOCATIONS];
    VkAccelerationStructureKHR* local_pAccelerationStructures = nullptr;
    {
        if (pAccelerationStructures) {
            local_pAccelerationStructures = accelerationStructureCount > DISPATCH_MAX_STACK_ALLOCATIONS
                                                ? new VkAccelerationStructureKHR[accelerationStructureCount]
                                                : var_local_pAccelerationStructures;
            for (uint32_t index0 = 0; index0 < accelerationStructureCount; ++index0) {
                local_pAccelerationStructures[index0] = layer_data->Unwrap(pAccelerationStructures[index0]);
            }
        }
    }
    VkResult result = layer_data->device_dispatch_table.WriteAccelerationStructuresPropertiesKHR(
        device, accelerationStructureCount, (const VkAccelerationStructureKHR*)local_pAccelerationStructures, queryType, dataSize,
        pData, stride);
    if (local_pAccelerationStructures != var_local_pAccelerationStructures) delete[] local_pAccelerationStructures;
    return result;
}

void DispatchCmdCopyAccelerationStructureKHR(VkCommandBuffer commandBuffer, const VkCopyAccelerationStructureInfoKHR* pInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CmdCopyAccelerationStructureKHR(commandBuffer, pInfo);
    safe_VkCopyAccelerationStructureInfoKHR var_local_pInfo;
    safe_VkCopyAccelerationStructureInfoKHR* local_pInfo = nullptr;
    {
        if (pInfo) {
            local_pInfo = &var_local_pInfo;
            local_pInfo->initialize(pInfo);

            if (pInfo->src) {
                local_pInfo->src = layer_data->Unwrap(pInfo->src);
            }
            if (pInfo->dst) {
                local_pInfo->dst = layer_data->Unwrap(pInfo->dst);
            }
        }
    }
    layer_data->device_dispatch_table.CmdCopyAccelerationStructureKHR(commandBuffer,
                                                                      (const VkCopyAccelerationStructureInfoKHR*)local_pInfo);
}

void DispatchCmdCopyAccelerationStructureToMemoryKHR(VkCommandBuffer commandBuffer,
                                                     const VkCopyAccelerationStructureToMemoryInfoKHR* pInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CmdCopyAccelerationStructureToMemoryKHR(commandBuffer, pInfo);
    safe_VkCopyAccelerationStructureToMemoryInfoKHR var_local_pInfo;
    safe_VkCopyAccelerationStructureToMemoryInfoKHR* local_pInfo = nullptr;
    {
        if (pInfo) {
            local_pInfo = &var_local_pInfo;
            local_pInfo->initialize(pInfo);

            if (pInfo->src) {
                local_pInfo->src = layer_data->Unwrap(pInfo->src);
            }
        }
    }
    layer_data->device_dispatch_table.CmdCopyAccelerationStructureToMemoryKHR(
        commandBuffer, (const VkCopyAccelerationStructureToMemoryInfoKHR*)local_pInfo);
}

void DispatchCmdCopyMemoryToAccelerationStructureKHR(VkCommandBuffer commandBuffer,
                                                     const VkCopyMemoryToAccelerationStructureInfoKHR* pInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CmdCopyMemoryToAccelerationStructureKHR(commandBuffer, pInfo);
    safe_VkCopyMemoryToAccelerationStructureInfoKHR var_local_pInfo;
    safe_VkCopyMemoryToAccelerationStructureInfoKHR* local_pInfo = nullptr;
    {
        if (pInfo) {
            local_pInfo = &var_local_pInfo;
            local_pInfo->initialize(pInfo);

            if (pInfo->dst) {
                local_pInfo->dst = layer_data->Unwrap(pInfo->dst);
            }
        }
    }
    layer_data->device_dispatch_table.CmdCopyMemoryToAccelerationStructureKHR(
        commandBuffer, (const VkCopyMemoryToAccelerationStructureInfoKHR*)local_pInfo);
}

VkDeviceAddress DispatchGetAccelerationStructureDeviceAddressKHR(VkDevice device,
                                                                 const VkAccelerationStructureDeviceAddressInfoKHR* pInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.GetAccelerationStructureDeviceAddressKHR(device, pInfo);
    safe_VkAccelerationStructureDeviceAddressInfoKHR var_local_pInfo;
    safe_VkAccelerationStructureDeviceAddressInfoKHR* local_pInfo = nullptr;
    {
        if (pInfo) {
            local_pInfo = &var_local_pInfo;
            local_pInfo->initialize(pInfo);

            if (pInfo->accelerationStructure) {
                local_pInfo->accelerationStructure = layer_data->Unwrap(pInfo->accelerationStructure);
            }
        }
    }
    VkDeviceAddress result = layer_data->device_dispatch_table.GetAccelerationStructureDeviceAddressKHR(
        device, (const VkAccelerationStructureDeviceAddressInfoKHR*)local_pInfo);

    return result;
}

void DispatchCmdWriteAccelerationStructuresPropertiesKHR(VkCommandBuffer commandBuffer, uint32_t accelerationStructureCount,
                                                         const VkAccelerationStructureKHR* pAccelerationStructures,
                                                         VkQueryType queryType, VkQueryPool queryPool, uint32_t firstQuery) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.CmdWriteAccelerationStructuresPropertiesKHR(
            commandBuffer, accelerationStructureCount, pAccelerationStructures, queryType, queryPool, firstQuery);
    VkAccelerationStructureKHR var_local_pAccelerationStructures[DISPATCH_MAX_STACK_ALLOCATIONS];
    VkAccelerationStructureKHR* local_pAccelerationStructures = nullptr;
    {
        if (pAccelerationStructures) {
            local_pAccelerationStructures = accelerationStructureCount > DISPATCH_MAX_STACK_ALLOCATIONS
                                                ? new VkAccelerationStructureKHR[accelerationStructureCount]
                                                : var_local_pAccelerationStructures;
            for (uint32_t index0 = 0; index0 < accelerationStructureCount; ++index0) {
                local_pAccelerationStructures[index0] = layer_data->Unwrap(pAccelerationStructures[index0]);
            }
        }
        queryPool = layer_data->Unwrap(queryPool);
    }
    layer_data->device_dispatch_table.CmdWriteAccelerationStructuresPropertiesKHR(
        commandBuffer, accelerationStructureCount, (const VkAccelerationStructureKHR*)local_pAccelerationStructures, queryType,
        queryPool, firstQuery);
    if (local_pAccelerationStructures != var_local_pAccelerationStructures) delete[] local_pAccelerationStructures;
}

void DispatchGetDeviceAccelerationStructureCompatibilityKHR(VkDevice device,
                                                            const VkAccelerationStructureVersionInfoKHR* pVersionInfo,
                                                            VkAccelerationStructureCompatibilityKHR* pCompatibility) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);

    layer_data->device_dispatch_table.GetDeviceAccelerationStructureCompatibilityKHR(device, pVersionInfo, pCompatibility);
}

void DispatchCmdTraceRaysKHR(VkCommandBuffer commandBuffer, const VkStridedDeviceAddressRegionKHR* pRaygenShaderBindingTable,
                             const VkStridedDeviceAddressRegionKHR* pMissShaderBindingTable,
                             const VkStridedDeviceAddressRegionKHR* pHitShaderBindingTable,
                             const VkStridedDeviceAddressRegionKHR* pCallableShaderBindingTable, uint32_t width, uint32_t height,
                             uint32_t depth) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdTraceRaysKHR(commandBuffer, pRaygenShaderBindingTable, pMissShaderBindingTable,
                                                      pHitShaderBindingTable, pCallableShaderBindingTable, width, height, depth);
}

VkResult DispatchGetRayTracingCaptureReplayShaderGroupHandlesKHR(VkDevice device, VkPipeline pipeline, uint32_t firstGroup,
                                                                 uint32_t groupCount, size_t dataSize, void* pData) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.GetRayTracingCaptureReplayShaderGroupHandlesKHR(device, pipeline, firstGroup,
                                                                                                 groupCount, dataSize, pData);
    { pipeline = layer_data->Unwrap(pipeline); }
    VkResult result = layer_data->device_dispatch_table.GetRayTracingCaptureReplayShaderGroupHandlesKHR(
        device, pipeline, firstGroup, groupCount, dataSize, pData);

    return result;
}

void DispatchCmdTraceRaysIndirectKHR(VkCommandBuffer commandBuffer,
                                     const VkStridedDeviceAddressRegionKHR* pRaygenShaderBindingTable,
                                     const VkStridedDeviceAddressRegionKHR* pMissShaderBindingTable,
                                     const VkStridedDeviceAddressRegionKHR* pHitShaderBindingTable,
                                     const VkStridedDeviceAddressRegionKHR* pCallableShaderBindingTable,
                                     VkDeviceAddress indirectDeviceAddress) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdTraceRaysIndirectKHR(commandBuffer, pRaygenShaderBindingTable, pMissShaderBindingTable,
                                                              pHitShaderBindingTable, pCallableShaderBindingTable,
                                                              indirectDeviceAddress);
}

VkDeviceSize DispatchGetRayTracingShaderGroupStackSizeKHR(VkDevice device, VkPipeline pipeline, uint32_t group,
                                                          VkShaderGroupShaderKHR groupShader) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.GetRayTracingShaderGroupStackSizeKHR(device, pipeline, group, groupShader);
    { pipeline = layer_data->Unwrap(pipeline); }
    VkDeviceSize result =
        layer_data->device_dispatch_table.GetRayTracingShaderGroupStackSizeKHR(device, pipeline, group, groupShader);

    return result;
}

void DispatchCmdSetRayTracingPipelineStackSizeKHR(VkCommandBuffer commandBuffer, uint32_t pipelineStackSize) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdSetRayTracingPipelineStackSizeKHR(commandBuffer, pipelineStackSize);
}

void DispatchCmdDrawMeshTasksEXT(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    layer_data->device_dispatch_table.CmdDrawMeshTasksEXT(commandBuffer, groupCountX, groupCountY, groupCountZ);
}

void DispatchCmdDrawMeshTasksIndirectEXT(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount,
                                         uint32_t stride) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.CmdDrawMeshTasksIndirectEXT(commandBuffer, buffer, offset, drawCount, stride);
    { buffer = layer_data->Unwrap(buffer); }
    layer_data->device_dispatch_table.CmdDrawMeshTasksIndirectEXT(commandBuffer, buffer, offset, drawCount, stride);
}

void DispatchCmdDrawMeshTasksIndirectCountEXT(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                              VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                              uint32_t stride) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.CmdDrawMeshTasksIndirectCountEXT(commandBuffer, buffer, offset, countBuffer,
                                                                                  countBufferOffset, maxDrawCount, stride);
    {
        buffer = layer_data->Unwrap(buffer);
        countBuffer = layer_data->Unwrap(countBuffer);
    }
    layer_data->device_dispatch_table.CmdDrawMeshTasksIndirectCountEXT(commandBuffer, buffer, offset, countBuffer,
                                                                       countBufferOffset, maxDrawCount, stride);
}

// NOLINTEND
