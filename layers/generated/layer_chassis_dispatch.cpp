
// This file is ***GENERATED***.  Do Not Edit.
// See layer_chassis_dispatch_generator.py for modifications.

/* Copyright (c) 2015-2019 The Khronos Group Inc.
 * Copyright (c) 2015-2019 Valve Corporation
 * Copyright (c) 2015-2019 LunarG, Inc.
 * Copyright (c) 2015-2019 Google Inc.
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
 * Author: Mark Lobodzinski <mark@lunarg.com>
 */

#include <mutex>
#include "chassis.h"
#include "layer_chassis_dispatch.h"
#include "vk_layer_utils.h"

// This intentionally includes a cpp file
#include "vk_safe_struct.cpp"

// shared_mutex support added in MSVC 2015 update 2
#if defined(_MSC_FULL_VER) && _MSC_FULL_VER >= 190023918 && NTDDI_VERSION > NTDDI_WIN10_RS2
    #include <shared_mutex>
    typedef std::shared_mutex dispatch_lock_t;
    typedef std::shared_lock<dispatch_lock_t> read_dispatch_lock_guard_t;
    typedef std::unique_lock<dispatch_lock_t> write_dispatch_lock_guard_t;
#else
    typedef std::mutex dispatch_lock_t;
    typedef std::unique_lock<dispatch_lock_t> read_dispatch_lock_guard_t;
    typedef std::unique_lock<dispatch_lock_t> write_dispatch_lock_guard_t;
#endif
dispatch_lock_t dispatch_lock;

// Unique Objects pNext extension handling function
void WrapPnextChainHandles(ValidationObject *layer_data, const void *pNext) {
    void *cur_pnext = const_cast<void *>(pNext);
    while (cur_pnext != NULL) {
        VkBaseOutStructure *header = reinterpret_cast<VkBaseOutStructure *>(cur_pnext);

        switch (header->sType) {
#ifdef VK_USE_PLATFORM_WIN32_KHR 
            case VK_STRUCTURE_TYPE_WIN32_KEYED_MUTEX_ACQUIRE_RELEASE_INFO_KHR: {
                    safe_VkWin32KeyedMutexAcquireReleaseInfoKHR *safe_struct = reinterpret_cast<safe_VkWin32KeyedMutexAcquireReleaseInfoKHR *>(cur_pnext);
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
#endif // VK_USE_PLATFORM_WIN32_KHR 

#ifdef VK_USE_PLATFORM_WIN32_KHR 
            case VK_STRUCTURE_TYPE_WIN32_KEYED_MUTEX_ACQUIRE_RELEASE_INFO_NV: {
                    safe_VkWin32KeyedMutexAcquireReleaseInfoNV *safe_struct = reinterpret_cast<safe_VkWin32KeyedMutexAcquireReleaseInfoNV *>(cur_pnext);
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
#endif // VK_USE_PLATFORM_WIN32_KHR 

            case VK_STRUCTURE_TYPE_DEDICATED_ALLOCATION_MEMORY_ALLOCATE_INFO_NV: {
                    safe_VkDedicatedAllocationMemoryAllocateInfoNV *safe_struct = reinterpret_cast<safe_VkDedicatedAllocationMemoryAllocateInfoNV *>(cur_pnext);
                    if (safe_struct->image) {
                        safe_struct->image = layer_data->Unwrap(safe_struct->image);
                    }
                    if (safe_struct->buffer) {
                        safe_struct->buffer = layer_data->Unwrap(safe_struct->buffer);
                    }
                } break;

            case VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO: {
                    safe_VkMemoryDedicatedAllocateInfo *safe_struct = reinterpret_cast<safe_VkMemoryDedicatedAllocateInfo *>(cur_pnext);
                    if (safe_struct->image) {
                        safe_struct->image = layer_data->Unwrap(safe_struct->image);
                    }
                    if (safe_struct->buffer) {
                        safe_struct->buffer = layer_data->Unwrap(safe_struct->buffer);
                    }
                } break;

            case VK_STRUCTURE_TYPE_IMAGE_SWAPCHAIN_CREATE_INFO_KHR: {
                    safe_VkImageSwapchainCreateInfoKHR *safe_struct = reinterpret_cast<safe_VkImageSwapchainCreateInfoKHR *>(cur_pnext);
                    if (safe_struct->swapchain) {
                        safe_struct->swapchain = layer_data->Unwrap(safe_struct->swapchain);
                    }
                } break;

            case VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_INFO: {
                    safe_VkSamplerYcbcrConversionInfo *safe_struct = reinterpret_cast<safe_VkSamplerYcbcrConversionInfo *>(cur_pnext);
                    if (safe_struct->conversion) {
                        safe_struct->conversion = layer_data->Unwrap(safe_struct->conversion);
                    }
                } break;

            case VK_STRUCTURE_TYPE_SHADER_MODULE_VALIDATION_CACHE_CREATE_INFO_EXT: {
                    safe_VkShaderModuleValidationCacheCreateInfoEXT *safe_struct = reinterpret_cast<safe_VkShaderModuleValidationCacheCreateInfoEXT *>(cur_pnext);
                    if (safe_struct->validationCache) {
                        safe_struct->validationCache = layer_data->Unwrap(safe_struct->validationCache);
                    }
                } break;

            case VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_NV: {
                    safe_VkWriteDescriptorSetAccelerationStructureNV *safe_struct = reinterpret_cast<safe_VkWriteDescriptorSetAccelerationStructureNV *>(cur_pnext);
                    if (safe_struct->pAccelerationStructures) {
                        for (uint32_t index0 = 0; index0 < safe_struct->accelerationStructureCount; ++index0) {
                            safe_struct->pAccelerationStructures[index0] = layer_data->Unwrap(safe_struct->pAccelerationStructures[index0]);
                        }
                    }
                } break;

            case VK_STRUCTURE_TYPE_RENDER_PASS_ATTACHMENT_BEGIN_INFO_KHR: {
                    safe_VkRenderPassAttachmentBeginInfoKHR *safe_struct = reinterpret_cast<safe_VkRenderPassAttachmentBeginInfoKHR *>(cur_pnext);
                    if (safe_struct->pAttachments) {
                        for (uint32_t index0 = 0; index0 < safe_struct->attachmentCount; ++index0) {
                            safe_struct->pAttachments[index0] = layer_data->Unwrap(safe_struct->pAttachments[index0]);
                        }
                    }
                } break;

            case VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_SWAPCHAIN_INFO_KHR: {
                    safe_VkBindImageMemorySwapchainInfoKHR *safe_struct = reinterpret_cast<safe_VkBindImageMemorySwapchainInfoKHR *>(cur_pnext);
                    if (safe_struct->swapchain) {
                        safe_struct->swapchain = layer_data->Unwrap(safe_struct->swapchain);
                    }
                } break;

            default:
                break;
        }

        // Process the next structure in the chain
        cur_pnext = header->pNext;
    }
}


// Manually written Dispatch routines

VkResult DispatchCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                        const VkComputePipelineCreateInfo *pCreateInfos,
                                        const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CreateComputePipelines(device, pipelineCache, createInfoCount,
                                                                                          pCreateInfos, pAllocator, pPipelines);
    safe_VkComputePipelineCreateInfo *local_pCreateInfos = NULL;
    if (pCreateInfos) {
        local_pCreateInfos = new safe_VkComputePipelineCreateInfo[createInfoCount];
        for (uint32_t idx0 = 0; idx0 < createInfoCount; ++idx0) {
            local_pCreateInfos[idx0].initialize(&pCreateInfos[idx0]);
            if (pCreateInfos[idx0].basePipelineHandle) {
                local_pCreateInfos[idx0].basePipelineHandle = layer_data->Unwrap(pCreateInfos[idx0].basePipelineHandle);
            }
            if (pCreateInfos[idx0].layout) {
                local_pCreateInfos[idx0].layout = layer_data->Unwrap(pCreateInfos[idx0].layout);
            }
            if (pCreateInfos[idx0].stage.module) {
                local_pCreateInfos[idx0].stage.module = layer_data->Unwrap(pCreateInfos[idx0].stage.module);
            }
        }
    }
    if (pipelineCache) {
        pipelineCache = layer_data->Unwrap(pipelineCache);
    }

    VkResult result = layer_data->device_dispatch_table.CreateComputePipelines(device, pipelineCache, createInfoCount,
                                                                               local_pCreateInfos->ptr(), pAllocator, pPipelines);
    delete[] local_pCreateInfos;
    {
        for (uint32_t i = 0; i < createInfoCount; ++i) {
            if (pPipelines[i] != VK_NULL_HANDLE) {
                pPipelines[i] = layer_data->WrapNew(pPipelines[i]);
            }
        }
    }
    return result;
}

VkResult DispatchCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                         const VkGraphicsPipelineCreateInfo *pCreateInfos,
                                         const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CreateGraphicsPipelines(device, pipelineCache, createInfoCount,
                                                                                           pCreateInfos, pAllocator, pPipelines);
    safe_VkGraphicsPipelineCreateInfo *local_pCreateInfos = nullptr;
    if (pCreateInfos) {
        local_pCreateInfos = new safe_VkGraphicsPipelineCreateInfo[createInfoCount];
        read_dispatch_lock_guard_t lock(dispatch_lock);
        for (uint32_t idx0 = 0; idx0 < createInfoCount; ++idx0) {
            bool uses_color_attachment = false;
            bool uses_depthstencil_attachment = false;
            {
                const auto subpasses_uses_it = layer_data->renderpasses_states.find(layer_data->Unwrap(pCreateInfos[idx0].renderPass));
                if (subpasses_uses_it != layer_data->renderpasses_states.end()) {
                    const auto &subpasses_uses = subpasses_uses_it->second;
                    if (subpasses_uses.subpasses_using_color_attachment.count(pCreateInfos[idx0].subpass))
                        uses_color_attachment = true;
                    if (subpasses_uses.subpasses_using_depthstencil_attachment.count(pCreateInfos[idx0].subpass))
                        uses_depthstencil_attachment = true;
                }
            }

            local_pCreateInfos[idx0].initialize(&pCreateInfos[idx0], uses_color_attachment, uses_depthstencil_attachment);

            if (pCreateInfos[idx0].basePipelineHandle) {
                local_pCreateInfos[idx0].basePipelineHandle = layer_data->Unwrap(pCreateInfos[idx0].basePipelineHandle);
            }
            if (pCreateInfos[idx0].layout) {
                local_pCreateInfos[idx0].layout = layer_data->Unwrap(pCreateInfos[idx0].layout);
            }
            if (pCreateInfos[idx0].pStages) {
                for (uint32_t idx1 = 0; idx1 < pCreateInfos[idx0].stageCount; ++idx1) {
                    if (pCreateInfos[idx0].pStages[idx1].module) {
                        local_pCreateInfos[idx0].pStages[idx1].module = layer_data->Unwrap(pCreateInfos[idx0].pStages[idx1].module);
                    }
                }
            }
            if (pCreateInfos[idx0].renderPass) {
                local_pCreateInfos[idx0].renderPass = layer_data->Unwrap(pCreateInfos[idx0].renderPass);
            }
        }
    }
    if (pipelineCache) {
        pipelineCache = layer_data->Unwrap(pipelineCache);
    }

    VkResult result = layer_data->device_dispatch_table.CreateGraphicsPipelines(device, pipelineCache, createInfoCount,
                                                                                local_pCreateInfos->ptr(), pAllocator, pPipelines);
    delete[] local_pCreateInfos;
    {
        for (uint32_t i = 0; i < createInfoCount; ++i) {
            if (pPipelines[i] != VK_NULL_HANDLE) {
                pPipelines[i] = layer_data->WrapNew(pPipelines[i]);
            }
        }
    }
    return result;
}

template <typename T>
static void UpdateCreateRenderPassState(ValidationObject *layer_data, const T *pCreateInfo, VkRenderPass renderPass) {
    auto &renderpass_state = layer_data->renderpasses_states[renderPass];

    for (uint32_t subpass = 0; subpass < pCreateInfo->subpassCount; ++subpass) {
        bool uses_color = false;
        for (uint32_t i = 0; i < pCreateInfo->pSubpasses[subpass].colorAttachmentCount && !uses_color; ++i)
            if (pCreateInfo->pSubpasses[subpass].pColorAttachments[i].attachment != VK_ATTACHMENT_UNUSED) uses_color = true;

        bool uses_depthstencil = false;
        if (pCreateInfo->pSubpasses[subpass].pDepthStencilAttachment)
            if (pCreateInfo->pSubpasses[subpass].pDepthStencilAttachment->attachment != VK_ATTACHMENT_UNUSED)
                uses_depthstencil = true;

        if (uses_color) renderpass_state.subpasses_using_color_attachment.insert(subpass);
        if (uses_depthstencil) renderpass_state.subpasses_using_depthstencil_attachment.insert(subpass);
    }
}

VkResult DispatchCreateRenderPass(VkDevice device, const VkRenderPassCreateInfo *pCreateInfo,
                                  const VkAllocationCallbacks *pAllocator, VkRenderPass *pRenderPass) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    VkResult result = layer_data->device_dispatch_table.CreateRenderPass(device, pCreateInfo, pAllocator, pRenderPass);
    if (!wrap_handles) return result;
    if (VK_SUCCESS == result) {
        write_dispatch_lock_guard_t lock(dispatch_lock);
        UpdateCreateRenderPassState(layer_data, pCreateInfo, *pRenderPass);
        *pRenderPass = layer_data->WrapNew(*pRenderPass);
    }
    return result;
}

VkResult DispatchCreateRenderPass2KHR(VkDevice device, const VkRenderPassCreateInfo2KHR *pCreateInfo,
                                      const VkAllocationCallbacks *pAllocator, VkRenderPass *pRenderPass) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    VkResult result = layer_data->device_dispatch_table.CreateRenderPass2KHR(device, pCreateInfo, pAllocator, pRenderPass);
    if (!wrap_handles) return result;
    if (VK_SUCCESS == result) {
        write_dispatch_lock_guard_t lock(dispatch_lock);
        UpdateCreateRenderPassState(layer_data, pCreateInfo, *pRenderPass);
        *pRenderPass = layer_data->WrapNew(*pRenderPass);
    }
    return result;
}

void DispatchDestroyRenderPass(VkDevice device, VkRenderPass renderPass, const VkAllocationCallbacks *pAllocator) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.DestroyRenderPass(device, renderPass, pAllocator);
    uint64_t renderPass_id = reinterpret_cast<uint64_t &>(renderPass);

    auto iter = unique_id_mapping.pop(renderPass_id);
    if (iter != unique_id_mapping.end()) {
        renderPass = (VkRenderPass)iter->second;
    } else {
        renderPass = (VkRenderPass)0;
    }

    layer_data->device_dispatch_table.DestroyRenderPass(device, renderPass, pAllocator);

    write_dispatch_lock_guard_t lock(dispatch_lock);
    layer_data->renderpasses_states.erase(renderPass);
}

VkResult DispatchCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR *pCreateInfo,
                                    const VkAllocationCallbacks *pAllocator, VkSwapchainKHR *pSwapchain) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CreateSwapchainKHR(device, pCreateInfo, pAllocator, pSwapchain);
    safe_VkSwapchainCreateInfoKHR *local_pCreateInfo = NULL;
    if (pCreateInfo) {
        local_pCreateInfo = new safe_VkSwapchainCreateInfoKHR(pCreateInfo);
        local_pCreateInfo->oldSwapchain = layer_data->Unwrap(pCreateInfo->oldSwapchain);
        // Surface is instance-level object
        local_pCreateInfo->surface = layer_data->Unwrap(pCreateInfo->surface);
    }

    VkResult result = layer_data->device_dispatch_table.CreateSwapchainKHR(device, local_pCreateInfo->ptr(), pAllocator, pSwapchain);
    delete local_pCreateInfo;

    if (VK_SUCCESS == result) {
        *pSwapchain = layer_data->WrapNew(*pSwapchain);
    }
    return result;
}

VkResult DispatchCreateSharedSwapchainsKHR(VkDevice device, uint32_t swapchainCount, const VkSwapchainCreateInfoKHR *pCreateInfos,
                                           const VkAllocationCallbacks *pAllocator, VkSwapchainKHR *pSwapchains) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.CreateSharedSwapchainsKHR(device, swapchainCount, pCreateInfos, pAllocator,
                                                                           pSwapchains);
    safe_VkSwapchainCreateInfoKHR *local_pCreateInfos = NULL;
    {
        if (pCreateInfos) {
            local_pCreateInfos = new safe_VkSwapchainCreateInfoKHR[swapchainCount];
            for (uint32_t i = 0; i < swapchainCount; ++i) {
                local_pCreateInfos[i].initialize(&pCreateInfos[i]);
                if (pCreateInfos[i].surface) {
                    // Surface is instance-level object
                    local_pCreateInfos[i].surface = layer_data->Unwrap(pCreateInfos[i].surface);
                }
                if (pCreateInfos[i].oldSwapchain) {
                    local_pCreateInfos[i].oldSwapchain = layer_data->Unwrap(pCreateInfos[i].oldSwapchain);
                }
            }
        }
    }
    VkResult result = layer_data->device_dispatch_table.CreateSharedSwapchainsKHR(device, swapchainCount, local_pCreateInfos->ptr(),
                                                                                  pAllocator, pSwapchains);
    delete[] local_pCreateInfos;
    if (VK_SUCCESS == result) {
        for (uint32_t i = 0; i < swapchainCount; i++) {
            pSwapchains[i] = layer_data->WrapNew(pSwapchains[i]);
        }
    }
    return result;
}

VkResult DispatchGetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapchain, uint32_t *pSwapchainImageCount,
                                       VkImage *pSwapchainImages) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.GetSwapchainImagesKHR(device, swapchain, pSwapchainImageCount, pSwapchainImages);
    VkSwapchainKHR wrapped_swapchain_handle = swapchain;
    if (VK_NULL_HANDLE != swapchain) {
        swapchain = layer_data->Unwrap(swapchain);
    }
    VkResult result =
        layer_data->device_dispatch_table.GetSwapchainImagesKHR(device, swapchain, pSwapchainImageCount, pSwapchainImages);
    if ((VK_SUCCESS == result) || (VK_INCOMPLETE == result)) {
        if ((*pSwapchainImageCount > 0) && pSwapchainImages) {
            write_dispatch_lock_guard_t lock(dispatch_lock);
            auto &wrapped_swapchain_image_handles = layer_data->swapchain_wrapped_image_handle_map[wrapped_swapchain_handle];
            for (uint32_t i = static_cast<uint32_t>(wrapped_swapchain_image_handles.size()); i < *pSwapchainImageCount; i++) {
                wrapped_swapchain_image_handles.emplace_back(layer_data->WrapNew(pSwapchainImages[i]));
            }
            for (uint32_t i = 0; i < *pSwapchainImageCount; i++) {
                pSwapchainImages[i] = wrapped_swapchain_image_handles[i];
            }
        }
    }
    return result;
}

void DispatchDestroySwapchainKHR(VkDevice device, VkSwapchainKHR swapchain, const VkAllocationCallbacks *pAllocator) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.DestroySwapchainKHR(device, swapchain, pAllocator);
    write_dispatch_lock_guard_t lock(dispatch_lock);

    auto &image_array = layer_data->swapchain_wrapped_image_handle_map[swapchain];
    for (auto &image_handle : image_array) {
        unique_id_mapping.erase(HandleToUint64(image_handle));
    }
    layer_data->swapchain_wrapped_image_handle_map.erase(swapchain);
    lock.unlock();

    uint64_t swapchain_id = HandleToUint64(swapchain);

    auto iter = unique_id_mapping.pop(swapchain_id);
    if (iter != unique_id_mapping.end()) {
        swapchain = (VkSwapchainKHR)iter->second;
    } else {
        swapchain = (VkSwapchainKHR)0;
    }

    layer_data->device_dispatch_table.DestroySwapchainKHR(device, swapchain, pAllocator);
}

VkResult DispatchQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR *pPresentInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(queue), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.QueuePresentKHR(queue, pPresentInfo);
    safe_VkPresentInfoKHR *local_pPresentInfo = NULL;
    {
        if (pPresentInfo) {
            local_pPresentInfo = new safe_VkPresentInfoKHR(pPresentInfo);
            if (local_pPresentInfo->pWaitSemaphores) {
                for (uint32_t index1 = 0; index1 < local_pPresentInfo->waitSemaphoreCount; ++index1) {
                    local_pPresentInfo->pWaitSemaphores[index1] = layer_data->Unwrap(pPresentInfo->pWaitSemaphores[index1]);
                }
            }
            if (local_pPresentInfo->pSwapchains) {
                for (uint32_t index1 = 0; index1 < local_pPresentInfo->swapchainCount; ++index1) {
                    local_pPresentInfo->pSwapchains[index1] = layer_data->Unwrap(pPresentInfo->pSwapchains[index1]);
                }
            }
        }
    }
    VkResult result = layer_data->device_dispatch_table.QueuePresentKHR(queue, local_pPresentInfo->ptr());

    // pResults is an output array embedded in a structure. The code generator neglects to copy back from the safe_* version,
    // so handle it as a special case here:
    if (pPresentInfo && pPresentInfo->pResults) {
        for (uint32_t i = 0; i < pPresentInfo->swapchainCount; i++) {
            pPresentInfo->pResults[i] = local_pPresentInfo->pResults[i];
        }
    }
    delete local_pPresentInfo;
    return result;
}

void DispatchDestroyDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool, const VkAllocationCallbacks *pAllocator) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.DestroyDescriptorPool(device, descriptorPool, pAllocator);
    write_dispatch_lock_guard_t lock(dispatch_lock);

    // remove references to implicitly freed descriptor sets
    for(auto descriptor_set : layer_data->pool_descriptor_sets_map[descriptorPool]) {
        unique_id_mapping.erase(reinterpret_cast<uint64_t &>(descriptor_set));
    }
    layer_data->pool_descriptor_sets_map.erase(descriptorPool);
    lock.unlock();

    uint64_t descriptorPool_id = reinterpret_cast<uint64_t &>(descriptorPool);

    auto iter = unique_id_mapping.pop(descriptorPool_id);
    if (iter != unique_id_mapping.end()) {
        descriptorPool = (VkDescriptorPool)iter->second;
    } else {
        descriptorPool = (VkDescriptorPool)0;
    }

    layer_data->device_dispatch_table.DestroyDescriptorPool(device, descriptorPool, pAllocator);
}

VkResult DispatchResetDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool, VkDescriptorPoolResetFlags flags) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.ResetDescriptorPool(device, descriptorPool, flags);
    VkDescriptorPool local_descriptor_pool = VK_NULL_HANDLE;
    {
        local_descriptor_pool = layer_data->Unwrap(descriptorPool);
    }
    VkResult result = layer_data->device_dispatch_table.ResetDescriptorPool(device, local_descriptor_pool, flags);
    if (VK_SUCCESS == result) {
        write_dispatch_lock_guard_t lock(dispatch_lock);
        // remove references to implicitly freed descriptor sets
        for(auto descriptor_set : layer_data->pool_descriptor_sets_map[descriptorPool]) {
            unique_id_mapping.erase(reinterpret_cast<uint64_t &>(descriptor_set));
        }
        layer_data->pool_descriptor_sets_map[descriptorPool].clear();
    }

    return result;
}

VkResult DispatchAllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo *pAllocateInfo,
                                        VkDescriptorSet *pDescriptorSets) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.AllocateDescriptorSets(device, pAllocateInfo, pDescriptorSets);
    safe_VkDescriptorSetAllocateInfo *local_pAllocateInfo = NULL;
    {
        if (pAllocateInfo) {
            local_pAllocateInfo = new safe_VkDescriptorSetAllocateInfo(pAllocateInfo);
            if (pAllocateInfo->descriptorPool) {
                local_pAllocateInfo->descriptorPool = layer_data->Unwrap(pAllocateInfo->descriptorPool);
            }
            if (local_pAllocateInfo->pSetLayouts) {
                for (uint32_t index1 = 0; index1 < local_pAllocateInfo->descriptorSetCount; ++index1) {
                    local_pAllocateInfo->pSetLayouts[index1] = layer_data->Unwrap(local_pAllocateInfo->pSetLayouts[index1]);
                }
            }
        }
    }
    VkResult result = layer_data->device_dispatch_table.AllocateDescriptorSets(
        device, (const VkDescriptorSetAllocateInfo *)local_pAllocateInfo, pDescriptorSets);
    if (local_pAllocateInfo) {
        delete local_pAllocateInfo;
    }
    if (VK_SUCCESS == result) {
        write_dispatch_lock_guard_t lock(dispatch_lock);
        auto &pool_descriptor_sets = layer_data->pool_descriptor_sets_map[pAllocateInfo->descriptorPool];
        for (uint32_t index0 = 0; index0 < pAllocateInfo->descriptorSetCount; index0++) {
            pDescriptorSets[index0] = layer_data->WrapNew(pDescriptorSets[index0]);
            pool_descriptor_sets.insert(pDescriptorSets[index0]);
        }
    }
    return result;
}

VkResult DispatchFreeDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool, uint32_t descriptorSetCount,
                                    const VkDescriptorSet *pDescriptorSets) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.FreeDescriptorSets(device, descriptorPool, descriptorSetCount, pDescriptorSets);
    VkDescriptorSet *local_pDescriptorSets = NULL;
    VkDescriptorPool local_descriptor_pool = VK_NULL_HANDLE;
    {
        local_descriptor_pool = layer_data->Unwrap(descriptorPool);
        if (pDescriptorSets) {
            local_pDescriptorSets = new VkDescriptorSet[descriptorSetCount];
            for (uint32_t index0 = 0; index0 < descriptorSetCount; ++index0) {
                local_pDescriptorSets[index0] = layer_data->Unwrap(pDescriptorSets[index0]);
            }
        }
    }
    VkResult result = layer_data->device_dispatch_table.FreeDescriptorSets(device, local_descriptor_pool, descriptorSetCount,
                                                                           (const VkDescriptorSet *)local_pDescriptorSets);
    if (local_pDescriptorSets) delete[] local_pDescriptorSets;
    if ((VK_SUCCESS == result) && (pDescriptorSets)) {
        write_dispatch_lock_guard_t lock(dispatch_lock);
        auto &pool_descriptor_sets = layer_data->pool_descriptor_sets_map[descriptorPool];
        for (uint32_t index0 = 0; index0 < descriptorSetCount; index0++) {
            VkDescriptorSet handle = pDescriptorSets[index0];
            pool_descriptor_sets.erase(handle);
            uint64_t unique_id = reinterpret_cast<uint64_t &>(handle);
            unique_id_mapping.erase(unique_id);
        }
    }
    return result;
}

// This is the core version of this routine.  The extension version is below.
VkResult DispatchCreateDescriptorUpdateTemplate(VkDevice device, const VkDescriptorUpdateTemplateCreateInfoKHR *pCreateInfo,
                                                const VkAllocationCallbacks *pAllocator,
                                                VkDescriptorUpdateTemplateKHR *pDescriptorUpdateTemplate) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.CreateDescriptorUpdateTemplate(device, pCreateInfo, pAllocator,
                                                                                pDescriptorUpdateTemplate);
    safe_VkDescriptorUpdateTemplateCreateInfo *local_create_info = NULL;
    {
        if (pCreateInfo) {
            local_create_info = new safe_VkDescriptorUpdateTemplateCreateInfo(pCreateInfo);
            if (pCreateInfo->descriptorSetLayout) {
                local_create_info->descriptorSetLayout = layer_data->Unwrap(pCreateInfo->descriptorSetLayout);
            }
            if (pCreateInfo->pipelineLayout) {
                local_create_info->pipelineLayout = layer_data->Unwrap(pCreateInfo->pipelineLayout);
            }
        }
    }
    VkResult result = layer_data->device_dispatch_table.CreateDescriptorUpdateTemplate(device, local_create_info->ptr(), pAllocator,
                                                                                       pDescriptorUpdateTemplate);
    if (VK_SUCCESS == result) {
        write_dispatch_lock_guard_t lock(dispatch_lock);
        *pDescriptorUpdateTemplate = layer_data->WrapNew(*pDescriptorUpdateTemplate);

        // Shadow template createInfo for later updates
        std::unique_ptr<TEMPLATE_STATE> template_state(new TEMPLATE_STATE(*pDescriptorUpdateTemplate, local_create_info));
        layer_data->desc_template_createinfo_map[(uint64_t)*pDescriptorUpdateTemplate] = std::move(template_state);
    }
    return result;
}

// This is the extension version of this routine.  The core version is above.
VkResult DispatchCreateDescriptorUpdateTemplateKHR(VkDevice device, const VkDescriptorUpdateTemplateCreateInfoKHR *pCreateInfo,
                                                   const VkAllocationCallbacks *pAllocator,
                                                   VkDescriptorUpdateTemplateKHR *pDescriptorUpdateTemplate) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.CreateDescriptorUpdateTemplateKHR(device, pCreateInfo, pAllocator,
                                                                                   pDescriptorUpdateTemplate);
    safe_VkDescriptorUpdateTemplateCreateInfo *local_create_info = NULL;
    {
        if (pCreateInfo) {
            local_create_info = new safe_VkDescriptorUpdateTemplateCreateInfo(pCreateInfo);
            if (pCreateInfo->descriptorSetLayout) {
                local_create_info->descriptorSetLayout = layer_data->Unwrap(pCreateInfo->descriptorSetLayout);
            }
            if (pCreateInfo->pipelineLayout) {
                local_create_info->pipelineLayout = layer_data->Unwrap(pCreateInfo->pipelineLayout);
            }
        }
    }
    VkResult result = layer_data->device_dispatch_table.CreateDescriptorUpdateTemplateKHR(device, local_create_info->ptr(), pAllocator,
                                                                                          pDescriptorUpdateTemplate);
    if (VK_SUCCESS == result) {
        write_dispatch_lock_guard_t lock(dispatch_lock);
        *pDescriptorUpdateTemplate = layer_data->WrapNew(*pDescriptorUpdateTemplate);

        // Shadow template createInfo for later updates
        std::unique_ptr<TEMPLATE_STATE> template_state(new TEMPLATE_STATE(*pDescriptorUpdateTemplate, local_create_info));
        layer_data->desc_template_createinfo_map[(uint64_t)*pDescriptorUpdateTemplate] = std::move(template_state);
    }
    return result;
}

// This is the core version of this routine.  The extension version is below.
void DispatchDestroyDescriptorUpdateTemplate(VkDevice device, VkDescriptorUpdateTemplateKHR descriptorUpdateTemplate,
                                             const VkAllocationCallbacks *pAllocator) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.DestroyDescriptorUpdateTemplate(device, descriptorUpdateTemplate, pAllocator);
    write_dispatch_lock_guard_t lock(dispatch_lock);
    uint64_t descriptor_update_template_id = reinterpret_cast<uint64_t &>(descriptorUpdateTemplate);
    layer_data->desc_template_createinfo_map.erase(descriptor_update_template_id);
    lock.unlock();

    auto iter = unique_id_mapping.pop(descriptor_update_template_id);
    if (iter != unique_id_mapping.end()) {
        descriptorUpdateTemplate = (VkDescriptorUpdateTemplate)iter->second;
    } else {
        descriptorUpdateTemplate = (VkDescriptorUpdateTemplate)0;
    }

    layer_data->device_dispatch_table.DestroyDescriptorUpdateTemplate(device, descriptorUpdateTemplate, pAllocator);
}

// This is the extension version of this routine.  The core version is above.
void DispatchDestroyDescriptorUpdateTemplateKHR(VkDevice device, VkDescriptorUpdateTemplateKHR descriptorUpdateTemplate,
                                                const VkAllocationCallbacks *pAllocator) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.DestroyDescriptorUpdateTemplateKHR(device, descriptorUpdateTemplate, pAllocator);
    write_dispatch_lock_guard_t lock(dispatch_lock);
    uint64_t descriptor_update_template_id = reinterpret_cast<uint64_t &>(descriptorUpdateTemplate);
    layer_data->desc_template_createinfo_map.erase(descriptor_update_template_id);
    lock.unlock();

    auto iter = unique_id_mapping.pop(descriptor_update_template_id);
    if (iter != unique_id_mapping.end()) {
        descriptorUpdateTemplate = (VkDescriptorUpdateTemplate)iter->second;
    } else {
        descriptorUpdateTemplate = (VkDescriptorUpdateTemplate)0;
    }

    layer_data->device_dispatch_table.DestroyDescriptorUpdateTemplateKHR(device, descriptorUpdateTemplate, pAllocator);
}

void *BuildUnwrappedUpdateTemplateBuffer(ValidationObject *layer_data, uint64_t descriptorUpdateTemplate, const void *pData) {
    auto const template_map_entry = layer_data->desc_template_createinfo_map.find(descriptorUpdateTemplate);
    if (template_map_entry == layer_data->desc_template_createinfo_map.end()) {
        assert(0);
    }
    auto const &create_info = template_map_entry->second->create_info;
    size_t allocation_size = 0;
    std::vector<std::tuple<size_t, VulkanObjectType, uint64_t, size_t>> template_entries;

    for (uint32_t i = 0; i < create_info.descriptorUpdateEntryCount; i++) {
        for (uint32_t j = 0; j < create_info.pDescriptorUpdateEntries[i].descriptorCount; j++) {
            size_t offset = create_info.pDescriptorUpdateEntries[i].offset + j * create_info.pDescriptorUpdateEntries[i].stride;
            char *update_entry = (char *)(pData) + offset;

            switch (create_info.pDescriptorUpdateEntries[i].descriptorType) {
                case VK_DESCRIPTOR_TYPE_SAMPLER:
                case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
                case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
                case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
                case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT: {
                    auto image_entry = reinterpret_cast<VkDescriptorImageInfo *>(update_entry);
                    allocation_size = std::max(allocation_size, offset + sizeof(VkDescriptorImageInfo));

                    VkDescriptorImageInfo *wrapped_entry = new VkDescriptorImageInfo(*image_entry);
                    wrapped_entry->sampler = layer_data->Unwrap(image_entry->sampler);
                    wrapped_entry->imageView = layer_data->Unwrap(image_entry->imageView);
                    template_entries.emplace_back(offset, kVulkanObjectTypeImage, CastToUint64(wrapped_entry), 0);
                } break;

                case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
                case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
                case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
                case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC: {
                    auto buffer_entry = reinterpret_cast<VkDescriptorBufferInfo *>(update_entry);
                    allocation_size = std::max(allocation_size, offset + sizeof(VkDescriptorBufferInfo));

                    VkDescriptorBufferInfo *wrapped_entry = new VkDescriptorBufferInfo(*buffer_entry);
                    wrapped_entry->buffer = layer_data->Unwrap(buffer_entry->buffer);
                    template_entries.emplace_back(offset, kVulkanObjectTypeBuffer, CastToUint64(wrapped_entry), 0);
                } break;

                case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
                case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER: {
                    auto buffer_view_handle = reinterpret_cast<VkBufferView *>(update_entry);
                    allocation_size = std::max(allocation_size, offset + sizeof(VkBufferView));

                    VkBufferView wrapped_entry = layer_data->Unwrap(*buffer_view_handle);
                    template_entries.emplace_back(offset, kVulkanObjectTypeBufferView, CastToUint64(wrapped_entry), 0);
                } break;
                case VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT: {
                    size_t numBytes = create_info.pDescriptorUpdateEntries[i].descriptorCount;
                    allocation_size = std::max(allocation_size, offset + numBytes);
                    // nothing to unwrap, just plain data
                    template_entries.emplace_back(offset, kVulkanObjectTypeUnknown, CastToUint64(update_entry),
                                                  numBytes);
                    // to break out of the loop
                    j = create_info.pDescriptorUpdateEntries[i].descriptorCount;
                } break;
                default:
                    assert(0);
                    break;
            }
        }
    }
    // Allocate required buffer size and populate with source/unwrapped data
    void *unwrapped_data = malloc(allocation_size);
    for (auto &this_entry : template_entries) {
        VulkanObjectType type = std::get<1>(this_entry);
        void *destination = (char *)unwrapped_data + std::get<0>(this_entry);
        uint64_t source = std::get<2>(this_entry);
        size_t size = std::get<3>(this_entry);

        if (size != 0) {
            assert(type == kVulkanObjectTypeUnknown);
            memcpy(destination, CastFromUint64<void *>(source), size);
        } else {
            switch (type) {
                case kVulkanObjectTypeImage:
                    *(reinterpret_cast<VkDescriptorImageInfo *>(destination)) =
                        *(reinterpret_cast<VkDescriptorImageInfo *>(source));
                    delete CastFromUint64<VkDescriptorImageInfo *>(source);
                    break;
                case kVulkanObjectTypeBuffer:
                    *(reinterpret_cast<VkDescriptorBufferInfo *>(destination)) =
                        *(CastFromUint64<VkDescriptorBufferInfo *>(source));
                    delete CastFromUint64<VkDescriptorBufferInfo *>(source);
                    break;
                case kVulkanObjectTypeBufferView:
                    *(reinterpret_cast<VkBufferView *>(destination)) = CastFromUint64<VkBufferView>(source);
                    break;
                default:
                    assert(0);
                    break;
            }
        }
    }
    return (void *)unwrapped_data;
}

void DispatchUpdateDescriptorSetWithTemplate(VkDevice device, VkDescriptorSet descriptorSet,
                                             VkDescriptorUpdateTemplateKHR descriptorUpdateTemplate, const void *pData) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.UpdateDescriptorSetWithTemplate(device, descriptorSet, descriptorUpdateTemplate,
                                                                                 pData);
    uint64_t template_handle = reinterpret_cast<uint64_t &>(descriptorUpdateTemplate);
    void *unwrapped_buffer = nullptr;
    {
        read_dispatch_lock_guard_t lock(dispatch_lock);
        descriptorSet = layer_data->Unwrap(descriptorSet);
        descriptorUpdateTemplate = (VkDescriptorUpdateTemplate)layer_data->Unwrap(descriptorUpdateTemplate);
        unwrapped_buffer = BuildUnwrappedUpdateTemplateBuffer(layer_data, template_handle, pData);
    }
    layer_data->device_dispatch_table.UpdateDescriptorSetWithTemplate(device, descriptorSet, descriptorUpdateTemplate, unwrapped_buffer);
    free(unwrapped_buffer);
}

void DispatchUpdateDescriptorSetWithTemplateKHR(VkDevice device, VkDescriptorSet descriptorSet,
                                                VkDescriptorUpdateTemplateKHR descriptorUpdateTemplate, const void *pData) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.UpdateDescriptorSetWithTemplateKHR(device, descriptorSet, descriptorUpdateTemplate,
                                                                                    pData);
    uint64_t template_handle = reinterpret_cast<uint64_t &>(descriptorUpdateTemplate);
    void *unwrapped_buffer = nullptr;
    {
        read_dispatch_lock_guard_t lock(dispatch_lock);
        descriptorSet = layer_data->Unwrap(descriptorSet);
        descriptorUpdateTemplate = layer_data->Unwrap(descriptorUpdateTemplate);
        unwrapped_buffer = BuildUnwrappedUpdateTemplateBuffer(layer_data, template_handle, pData);
    }
    layer_data->device_dispatch_table.UpdateDescriptorSetWithTemplateKHR(device, descriptorSet, descriptorUpdateTemplate, unwrapped_buffer);
    free(unwrapped_buffer);
}

void DispatchCmdPushDescriptorSetWithTemplateKHR(VkCommandBuffer commandBuffer,
                                                 VkDescriptorUpdateTemplateKHR descriptorUpdateTemplate, VkPipelineLayout layout,
                                                 uint32_t set, const void *pData) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles)
        return layer_data->device_dispatch_table.CmdPushDescriptorSetWithTemplateKHR(commandBuffer, descriptorUpdateTemplate,
                                                                                     layout, set, pData);
    uint64_t template_handle = reinterpret_cast<uint64_t &>(descriptorUpdateTemplate);
    void *unwrapped_buffer = nullptr;
    {
        read_dispatch_lock_guard_t lock(dispatch_lock);
        descriptorUpdateTemplate = layer_data->Unwrap(descriptorUpdateTemplate);
        layout = layer_data->Unwrap(layout);
        unwrapped_buffer = BuildUnwrappedUpdateTemplateBuffer(layer_data, template_handle, pData);
    }
    layer_data->device_dispatch_table.CmdPushDescriptorSetWithTemplateKHR(commandBuffer, descriptorUpdateTemplate, layout, set,
                                                                 unwrapped_buffer);
    free(unwrapped_buffer);
}

VkResult DispatchGetPhysicalDeviceDisplayPropertiesKHR(VkPhysicalDevice physicalDevice, uint32_t *pPropertyCount,
                                                       VkDisplayPropertiesKHR *pProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    VkResult result =
        layer_data->instance_dispatch_table.GetPhysicalDeviceDisplayPropertiesKHR(physicalDevice, pPropertyCount, pProperties);
    if (!wrap_handles) return result;
    if ((result == VK_SUCCESS || result == VK_INCOMPLETE) && pProperties) {
        for (uint32_t idx0 = 0; idx0 < *pPropertyCount; ++idx0) {
            pProperties[idx0].display = layer_data->MaybeWrapDisplay(pProperties[idx0].display, layer_data);
        }
    }
    return result;
}

VkResult DispatchGetPhysicalDeviceDisplayProperties2KHR(VkPhysicalDevice physicalDevice, uint32_t *pPropertyCount,
                                                        VkDisplayProperties2KHR *pProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    VkResult result =
        layer_data->instance_dispatch_table.GetPhysicalDeviceDisplayProperties2KHR(physicalDevice, pPropertyCount, pProperties);
    if (!wrap_handles) return result;
    if ((result == VK_SUCCESS || result == VK_INCOMPLETE) && pProperties) {
        for (uint32_t idx0 = 0; idx0 < *pPropertyCount; ++idx0) {
            pProperties[idx0].displayProperties.display =
                layer_data->MaybeWrapDisplay(pProperties[idx0].displayProperties.display, layer_data);
        }
    }
    return result;
}

VkResult DispatchGetPhysicalDeviceDisplayPlanePropertiesKHR(VkPhysicalDevice physicalDevice, uint32_t *pPropertyCount,
                                                            VkDisplayPlanePropertiesKHR *pProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    VkResult result =
        layer_data->instance_dispatch_table.GetPhysicalDeviceDisplayPlanePropertiesKHR(physicalDevice, pPropertyCount, pProperties);
    if (!wrap_handles) return result;
    if ((result == VK_SUCCESS || result == VK_INCOMPLETE) && pProperties) {
        for (uint32_t idx0 = 0; idx0 < *pPropertyCount; ++idx0) {
            VkDisplayKHR &opt_display = pProperties[idx0].currentDisplay;
            if (opt_display) opt_display = layer_data->MaybeWrapDisplay(opt_display, layer_data);
        }
    }
    return result;
}

VkResult DispatchGetPhysicalDeviceDisplayPlaneProperties2KHR(VkPhysicalDevice physicalDevice, uint32_t *pPropertyCount,
                                                             VkDisplayPlaneProperties2KHR *pProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    VkResult result = layer_data->instance_dispatch_table.GetPhysicalDeviceDisplayPlaneProperties2KHR(physicalDevice,
                                                                                                      pPropertyCount, pProperties);
    if (!wrap_handles) return result;
    if ((result == VK_SUCCESS || result == VK_INCOMPLETE) && pProperties) {
        for (uint32_t idx0 = 0; idx0 < *pPropertyCount; ++idx0) {
            VkDisplayKHR &opt_display = pProperties[idx0].displayPlaneProperties.currentDisplay;
            if (opt_display) opt_display = layer_data->MaybeWrapDisplay(opt_display, layer_data);
        }
    }
    return result;
}

VkResult DispatchGetDisplayPlaneSupportedDisplaysKHR(VkPhysicalDevice physicalDevice, uint32_t planeIndex, uint32_t *pDisplayCount,
                                                     VkDisplayKHR *pDisplays) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    VkResult result = layer_data->instance_dispatch_table.GetDisplayPlaneSupportedDisplaysKHR(physicalDevice, planeIndex,
                                                                                              pDisplayCount, pDisplays);
    if ((result == VK_SUCCESS || result == VK_INCOMPLETE) && pDisplays) {
    if (!wrap_handles) return result;
        for (uint32_t i = 0; i < *pDisplayCount; ++i) {
            if (pDisplays[i]) pDisplays[i] = layer_data->MaybeWrapDisplay(pDisplays[i], layer_data);
        }
    }
    return result;
}

VkResult DispatchGetDisplayModePropertiesKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display, uint32_t *pPropertyCount,
                                             VkDisplayModePropertiesKHR *pProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    if (!wrap_handles)
        return layer_data->instance_dispatch_table.GetDisplayModePropertiesKHR(physicalDevice, display, pPropertyCount,
                                                                               pProperties);
    {
        display = layer_data->Unwrap(display);
    }

    VkResult result = layer_data->instance_dispatch_table.GetDisplayModePropertiesKHR(physicalDevice, display, pPropertyCount, pProperties);
    if ((result == VK_SUCCESS || result == VK_INCOMPLETE) && pProperties) {
        for (uint32_t idx0 = 0; idx0 < *pPropertyCount; ++idx0) {
            pProperties[idx0].displayMode = layer_data->WrapNew(pProperties[idx0].displayMode);
        }
    }
    return result;
}

VkResult DispatchGetDisplayModeProperties2KHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display, uint32_t *pPropertyCount,
                                              VkDisplayModeProperties2KHR *pProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    if (!wrap_handles)
        return layer_data->instance_dispatch_table.GetDisplayModeProperties2KHR(physicalDevice, display, pPropertyCount,
                                                                                pProperties);
    {
        display = layer_data->Unwrap(display);
    }

    VkResult result =
        layer_data->instance_dispatch_table.GetDisplayModeProperties2KHR(physicalDevice, display, pPropertyCount, pProperties);
    if ((result == VK_SUCCESS || result == VK_INCOMPLETE) && pProperties) {
        for (uint32_t idx0 = 0; idx0 < *pPropertyCount; ++idx0) {
            pProperties[idx0].displayModeProperties.displayMode = layer_data->WrapNew(pProperties[idx0].displayModeProperties.displayMode);
        }
    }
    return result;
}

VkResult DispatchDebugMarkerSetObjectTagEXT(VkDevice device, const VkDebugMarkerObjectTagInfoEXT *pTagInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.DebugMarkerSetObjectTagEXT(device, pTagInfo);
    safe_VkDebugMarkerObjectTagInfoEXT local_tag_info(pTagInfo);
    {
        auto it = unique_id_mapping.find(reinterpret_cast<uint64_t &>(local_tag_info.object));
        if (it != unique_id_mapping.end()) {
            local_tag_info.object = it->second;
        }
    }
    VkResult result = layer_data->device_dispatch_table.DebugMarkerSetObjectTagEXT(device, 
                                                                                   reinterpret_cast<VkDebugMarkerObjectTagInfoEXT *>(&local_tag_info));
    return result;
}

VkResult DispatchDebugMarkerSetObjectNameEXT(VkDevice device, const VkDebugMarkerObjectNameInfoEXT *pNameInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.DebugMarkerSetObjectNameEXT(device, pNameInfo);
    safe_VkDebugMarkerObjectNameInfoEXT local_name_info(pNameInfo);
    {
        auto it = unique_id_mapping.find(reinterpret_cast<uint64_t &>(local_name_info.object));
        if (it != unique_id_mapping.end()) {
            local_name_info.object = it->second;
        }
    }
    VkResult result = layer_data->device_dispatch_table.DebugMarkerSetObjectNameEXT(
        device, reinterpret_cast<VkDebugMarkerObjectNameInfoEXT *>(&local_name_info));
    return result;
}

// VK_EXT_debug_utils
VkResult DispatchSetDebugUtilsObjectTagEXT(VkDevice device, const VkDebugUtilsObjectTagInfoEXT *pTagInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.SetDebugUtilsObjectTagEXT(device, pTagInfo);
    safe_VkDebugUtilsObjectTagInfoEXT local_tag_info(pTagInfo);
    {
        auto it = unique_id_mapping.find(reinterpret_cast<uint64_t &>(local_tag_info.objectHandle));
        if (it != unique_id_mapping.end()) {
            local_tag_info.objectHandle = it->second;
        }
    }
    VkResult result = layer_data->device_dispatch_table.SetDebugUtilsObjectTagEXT(
        device, reinterpret_cast<const VkDebugUtilsObjectTagInfoEXT *>(&local_tag_info));
    return result;
}

VkResult DispatchSetDebugUtilsObjectNameEXT(VkDevice device, const VkDebugUtilsObjectNameInfoEXT *pNameInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.SetDebugUtilsObjectNameEXT(device, pNameInfo);
    safe_VkDebugUtilsObjectNameInfoEXT local_name_info(pNameInfo);
    {
        auto it = unique_id_mapping.find(reinterpret_cast<uint64_t &>(local_name_info.objectHandle));
        if (it != unique_id_mapping.end()) {
            local_name_info.objectHandle = it->second;
        }
    }
    VkResult result = layer_data->device_dispatch_table.SetDebugUtilsObjectNameEXT(
        device, reinterpret_cast<const VkDebugUtilsObjectNameInfoEXT *>(&local_name_info));
    return result;
}




// Skip vkCreateInstance dispatch, manually generated

// Skip vkDestroyInstance dispatch, manually generated

VkResult DispatchEnumeratePhysicalDevices(
    VkInstance                                  instance,
    uint32_t*                                   pPhysicalDeviceCount,
    VkPhysicalDevice*                           pPhysicalDevices)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(instance), layer_data_map);
    VkResult result = layer_data->instance_dispatch_table.EnumeratePhysicalDevices(instance, pPhysicalDeviceCount, pPhysicalDevices);

    return result;
}

void DispatchGetPhysicalDeviceFeatures(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceFeatures*                   pFeatures)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    layer_data->instance_dispatch_table.GetPhysicalDeviceFeatures(physicalDevice, pFeatures);

}

void DispatchGetPhysicalDeviceFormatProperties(
    VkPhysicalDevice                            physicalDevice,
    VkFormat                                    format,
    VkFormatProperties*                         pFormatProperties)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    layer_data->instance_dispatch_table.GetPhysicalDeviceFormatProperties(physicalDevice, format, pFormatProperties);

}

VkResult DispatchGetPhysicalDeviceImageFormatProperties(
    VkPhysicalDevice                            physicalDevice,
    VkFormat                                    format,
    VkImageType                                 type,
    VkImageTiling                               tiling,
    VkImageUsageFlags                           usage,
    VkImageCreateFlags                          flags,
    VkImageFormatProperties*                    pImageFormatProperties)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    VkResult result = layer_data->instance_dispatch_table.GetPhysicalDeviceImageFormatProperties(physicalDevice, format, type, tiling, usage, flags, pImageFormatProperties);

    return result;
}

void DispatchGetPhysicalDeviceProperties(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceProperties*                 pProperties)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    layer_data->instance_dispatch_table.GetPhysicalDeviceProperties(physicalDevice, pProperties);

}

void DispatchGetPhysicalDeviceQueueFamilyProperties(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pQueueFamilyPropertyCount,
    VkQueueFamilyProperties*                    pQueueFamilyProperties)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    layer_data->instance_dispatch_table.GetPhysicalDeviceQueueFamilyProperties(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);

}

void DispatchGetPhysicalDeviceMemoryProperties(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceMemoryProperties*           pMemoryProperties)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    layer_data->instance_dispatch_table.GetPhysicalDeviceMemoryProperties(physicalDevice, pMemoryProperties);

}

PFN_vkVoidFunction DispatchGetInstanceProcAddr(
    VkInstance                                  instance,
    const char*                                 pName)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(instance), layer_data_map);
    PFN_vkVoidFunction result = layer_data->instance_dispatch_table.GetInstanceProcAddr(instance, pName);

    return result;
}

PFN_vkVoidFunction DispatchGetDeviceProcAddr(
    VkDevice                                    device,
    const char*                                 pName)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    PFN_vkVoidFunction result = layer_data->device_dispatch_table.GetDeviceProcAddr(device, pName);

    return result;
}

// Skip vkCreateDevice dispatch, manually generated

// Skip vkDestroyDevice dispatch, manually generated

// Skip vkEnumerateInstanceExtensionProperties dispatch, manually generated

// Skip vkEnumerateDeviceExtensionProperties dispatch, manually generated

// Skip vkEnumerateInstanceLayerProperties dispatch, manually generated

// Skip vkEnumerateDeviceLayerProperties dispatch, manually generated

void DispatchGetDeviceQueue(
    VkDevice                                    device,
    uint32_t                                    queueFamilyIndex,
    uint32_t                                    queueIndex,
    VkQueue*                                    pQueue)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    layer_data->device_dispatch_table.GetDeviceQueue(device, queueFamilyIndex, queueIndex, pQueue);

}

VkResult DispatchQueueSubmit(
    VkQueue                                     queue,
    uint32_t                                    submitCount,
    const VkSubmitInfo*                         pSubmits,
    VkFence                                     fence)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(queue), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.QueueSubmit(queue, submitCount, pSubmits, fence);
    safe_VkSubmitInfo *local_pSubmits = NULL;
    {
        if (pSubmits) {
            local_pSubmits = new safe_VkSubmitInfo[submitCount];
            for (uint32_t index0 = 0; index0 < submitCount; ++index0) {
                local_pSubmits[index0].initialize(&pSubmits[index0]);
                WrapPnextChainHandles(layer_data, local_pSubmits[index0].pNext);
                if (local_pSubmits[index0].pWaitSemaphores) {
                    for (uint32_t index1 = 0; index1 < local_pSubmits[index0].waitSemaphoreCount; ++index1) {
                        local_pSubmits[index0].pWaitSemaphores[index1] = layer_data->Unwrap(local_pSubmits[index0].pWaitSemaphores[index1]);
                    }
                }
                if (local_pSubmits[index0].pSignalSemaphores) {
                    for (uint32_t index1 = 0; index1 < local_pSubmits[index0].signalSemaphoreCount; ++index1) {
                        local_pSubmits[index0].pSignalSemaphores[index1] = layer_data->Unwrap(local_pSubmits[index0].pSignalSemaphores[index1]);
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

VkResult DispatchQueueWaitIdle(
    VkQueue                                     queue)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(queue), layer_data_map);
    VkResult result = layer_data->device_dispatch_table.QueueWaitIdle(queue);

    return result;
}

VkResult DispatchDeviceWaitIdle(
    VkDevice                                    device)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    VkResult result = layer_data->device_dispatch_table.DeviceWaitIdle(device);

    return result;
}

VkResult DispatchAllocateMemory(
    VkDevice                                    device,
    const VkMemoryAllocateInfo*                 pAllocateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDeviceMemory*                             pMemory)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.AllocateMemory(device, pAllocateInfo, pAllocator, pMemory);
    safe_VkMemoryAllocateInfo *local_pAllocateInfo = NULL;
    {
        if (pAllocateInfo) {
            local_pAllocateInfo = new safe_VkMemoryAllocateInfo(pAllocateInfo);
            WrapPnextChainHandles(layer_data, local_pAllocateInfo->pNext);
        }
    }
    VkResult result = layer_data->device_dispatch_table.AllocateMemory(device, (const VkMemoryAllocateInfo*)local_pAllocateInfo, pAllocator, pMemory);
    if (local_pAllocateInfo) {
        delete local_pAllocateInfo;
    }
    if (VK_SUCCESS == result) {
        *pMemory = layer_data->WrapNew(*pMemory);
    }
    return result;
}

void DispatchFreeMemory(
    VkDevice                                    device,
    VkDeviceMemory                              memory,
    const VkAllocationCallbacks*                pAllocator)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.FreeMemory(device, memory, pAllocator);
    uint64_t memory_id = reinterpret_cast<uint64_t &>(memory);
    auto iter = unique_id_mapping.pop(memory_id);
    if (iter != unique_id_mapping.end()) {
        memory = (VkDeviceMemory)iter->second;
    } else {
        memory = (VkDeviceMemory)0;
    }
    layer_data->device_dispatch_table.FreeMemory(device, memory, pAllocator);

}

VkResult DispatchMapMemory(
    VkDevice                                    device,
    VkDeviceMemory                              memory,
    VkDeviceSize                                offset,
    VkDeviceSize                                size,
    VkMemoryMapFlags                            flags,
    void**                                      ppData)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.MapMemory(device, memory, offset, size, flags, ppData);
    {
        memory = layer_data->Unwrap(memory);
    }
    VkResult result = layer_data->device_dispatch_table.MapMemory(device, memory, offset, size, flags, ppData);

    return result;
}

void DispatchUnmapMemory(
    VkDevice                                    device,
    VkDeviceMemory                              memory)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.UnmapMemory(device, memory);
    {
        memory = layer_data->Unwrap(memory);
    }
    layer_data->device_dispatch_table.UnmapMemory(device, memory);

}

VkResult DispatchFlushMappedMemoryRanges(
    VkDevice                                    device,
    uint32_t                                    memoryRangeCount,
    const VkMappedMemoryRange*                  pMemoryRanges)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.FlushMappedMemoryRanges(device, memoryRangeCount, pMemoryRanges);
    safe_VkMappedMemoryRange *local_pMemoryRanges = NULL;
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
    VkResult result = layer_data->device_dispatch_table.FlushMappedMemoryRanges(device, memoryRangeCount, (const VkMappedMemoryRange*)local_pMemoryRanges);
    if (local_pMemoryRanges) {
        delete[] local_pMemoryRanges;
    }
    return result;
}

VkResult DispatchInvalidateMappedMemoryRanges(
    VkDevice                                    device,
    uint32_t                                    memoryRangeCount,
    const VkMappedMemoryRange*                  pMemoryRanges)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.InvalidateMappedMemoryRanges(device, memoryRangeCount, pMemoryRanges);
    safe_VkMappedMemoryRange *local_pMemoryRanges = NULL;
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
    VkResult result = layer_data->device_dispatch_table.InvalidateMappedMemoryRanges(device, memoryRangeCount, (const VkMappedMemoryRange*)local_pMemoryRanges);
    if (local_pMemoryRanges) {
        delete[] local_pMemoryRanges;
    }
    return result;
}

void DispatchGetDeviceMemoryCommitment(
    VkDevice                                    device,
    VkDeviceMemory                              memory,
    VkDeviceSize*                               pCommittedMemoryInBytes)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.GetDeviceMemoryCommitment(device, memory, pCommittedMemoryInBytes);
    {
        memory = layer_data->Unwrap(memory);
    }
    layer_data->device_dispatch_table.GetDeviceMemoryCommitment(device, memory, pCommittedMemoryInBytes);

}

VkResult DispatchBindBufferMemory(
    VkDevice                                    device,
    VkBuffer                                    buffer,
    VkDeviceMemory                              memory,
    VkDeviceSize                                memoryOffset)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.BindBufferMemory(device, buffer, memory, memoryOffset);
    {
        buffer = layer_data->Unwrap(buffer);
        memory = layer_data->Unwrap(memory);
    }
    VkResult result = layer_data->device_dispatch_table.BindBufferMemory(device, buffer, memory, memoryOffset);

    return result;
}

VkResult DispatchBindImageMemory(
    VkDevice                                    device,
    VkImage                                     image,
    VkDeviceMemory                              memory,
    VkDeviceSize                                memoryOffset)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.BindImageMemory(device, image, memory, memoryOffset);
    {
        image = layer_data->Unwrap(image);
        memory = layer_data->Unwrap(memory);
    }
    VkResult result = layer_data->device_dispatch_table.BindImageMemory(device, image, memory, memoryOffset);

    return result;
}

void DispatchGetBufferMemoryRequirements(
    VkDevice                                    device,
    VkBuffer                                    buffer,
    VkMemoryRequirements*                       pMemoryRequirements)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.GetBufferMemoryRequirements(device, buffer, pMemoryRequirements);
    {
        buffer = layer_data->Unwrap(buffer);
    }
    layer_data->device_dispatch_table.GetBufferMemoryRequirements(device, buffer, pMemoryRequirements);

}

void DispatchGetImageMemoryRequirements(
    VkDevice                                    device,
    VkImage                                     image,
    VkMemoryRequirements*                       pMemoryRequirements)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.GetImageMemoryRequirements(device, image, pMemoryRequirements);
    {
        image = layer_data->Unwrap(image);
    }
    layer_data->device_dispatch_table.GetImageMemoryRequirements(device, image, pMemoryRequirements);

}

void DispatchGetImageSparseMemoryRequirements(
    VkDevice                                    device,
    VkImage                                     image,
    uint32_t*                                   pSparseMemoryRequirementCount,
    VkSparseImageMemoryRequirements*            pSparseMemoryRequirements)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.GetImageSparseMemoryRequirements(device, image, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
    {
        image = layer_data->Unwrap(image);
    }
    layer_data->device_dispatch_table.GetImageSparseMemoryRequirements(device, image, pSparseMemoryRequirementCount, pSparseMemoryRequirements);

}

void DispatchGetPhysicalDeviceSparseImageFormatProperties(
    VkPhysicalDevice                            physicalDevice,
    VkFormat                                    format,
    VkImageType                                 type,
    VkSampleCountFlagBits                       samples,
    VkImageUsageFlags                           usage,
    VkImageTiling                               tiling,
    uint32_t*                                   pPropertyCount,
    VkSparseImageFormatProperties*              pProperties)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    layer_data->instance_dispatch_table.GetPhysicalDeviceSparseImageFormatProperties(physicalDevice, format, type, samples, usage, tiling, pPropertyCount, pProperties);

}

VkResult DispatchQueueBindSparse(
    VkQueue                                     queue,
    uint32_t                                    bindInfoCount,
    const VkBindSparseInfo*                     pBindInfo,
    VkFence                                     fence)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(queue), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.QueueBindSparse(queue, bindInfoCount, pBindInfo, fence);
    safe_VkBindSparseInfo *local_pBindInfo = NULL;
    {
        if (pBindInfo) {
            local_pBindInfo = new safe_VkBindSparseInfo[bindInfoCount];
            for (uint32_t index0 = 0; index0 < bindInfoCount; ++index0) {
                local_pBindInfo[index0].initialize(&pBindInfo[index0]);
                if (local_pBindInfo[index0].pWaitSemaphores) {
                    for (uint32_t index1 = 0; index1 < local_pBindInfo[index0].waitSemaphoreCount; ++index1) {
                        local_pBindInfo[index0].pWaitSemaphores[index1] = layer_data->Unwrap(local_pBindInfo[index0].pWaitSemaphores[index1]);
                    }
                }
                if (local_pBindInfo[index0].pBufferBinds) {
                    for (uint32_t index1 = 0; index1 < local_pBindInfo[index0].bufferBindCount; ++index1) {
                        if (pBindInfo[index0].pBufferBinds[index1].buffer) {
                            local_pBindInfo[index0].pBufferBinds[index1].buffer = layer_data->Unwrap(pBindInfo[index0].pBufferBinds[index1].buffer);
                        }
                        if (local_pBindInfo[index0].pBufferBinds[index1].pBinds) {
                            for (uint32_t index2 = 0; index2 < local_pBindInfo[index0].pBufferBinds[index1].bindCount; ++index2) {
                                if (pBindInfo[index0].pBufferBinds[index1].pBinds[index2].memory) {
                                    local_pBindInfo[index0].pBufferBinds[index1].pBinds[index2].memory = layer_data->Unwrap(pBindInfo[index0].pBufferBinds[index1].pBinds[index2].memory);
                                }
                            }
                        }
                    }
                }
                if (local_pBindInfo[index0].pImageOpaqueBinds) {
                    for (uint32_t index1 = 0; index1 < local_pBindInfo[index0].imageOpaqueBindCount; ++index1) {
                        if (pBindInfo[index0].pImageOpaqueBinds[index1].image) {
                            local_pBindInfo[index0].pImageOpaqueBinds[index1].image = layer_data->Unwrap(pBindInfo[index0].pImageOpaqueBinds[index1].image);
                        }
                        if (local_pBindInfo[index0].pImageOpaqueBinds[index1].pBinds) {
                            for (uint32_t index2 = 0; index2 < local_pBindInfo[index0].pImageOpaqueBinds[index1].bindCount; ++index2) {
                                if (pBindInfo[index0].pImageOpaqueBinds[index1].pBinds[index2].memory) {
                                    local_pBindInfo[index0].pImageOpaqueBinds[index1].pBinds[index2].memory = layer_data->Unwrap(pBindInfo[index0].pImageOpaqueBinds[index1].pBinds[index2].memory);
                                }
                            }
                        }
                    }
                }
                if (local_pBindInfo[index0].pImageBinds) {
                    for (uint32_t index1 = 0; index1 < local_pBindInfo[index0].imageBindCount; ++index1) {
                        if (pBindInfo[index0].pImageBinds[index1].image) {
                            local_pBindInfo[index0].pImageBinds[index1].image = layer_data->Unwrap(pBindInfo[index0].pImageBinds[index1].image);
                        }
                        if (local_pBindInfo[index0].pImageBinds[index1].pBinds) {
                            for (uint32_t index2 = 0; index2 < local_pBindInfo[index0].pImageBinds[index1].bindCount; ++index2) {
                                if (pBindInfo[index0].pImageBinds[index1].pBinds[index2].memory) {
                                    local_pBindInfo[index0].pImageBinds[index1].pBinds[index2].memory = layer_data->Unwrap(pBindInfo[index0].pImageBinds[index1].pBinds[index2].memory);
                                }
                            }
                        }
                    }
                }
                if (local_pBindInfo[index0].pSignalSemaphores) {
                    for (uint32_t index1 = 0; index1 < local_pBindInfo[index0].signalSemaphoreCount; ++index1) {
                        local_pBindInfo[index0].pSignalSemaphores[index1] = layer_data->Unwrap(local_pBindInfo[index0].pSignalSemaphores[index1]);
                    }
                }
            }
        }
        fence = layer_data->Unwrap(fence);
    }
    VkResult result = layer_data->device_dispatch_table.QueueBindSparse(queue, bindInfoCount, (const VkBindSparseInfo*)local_pBindInfo, fence);
    if (local_pBindInfo) {
        delete[] local_pBindInfo;
    }
    return result;
}

VkResult DispatchCreateFence(
    VkDevice                                    device,
    const VkFenceCreateInfo*                    pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkFence*                                    pFence)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CreateFence(device, pCreateInfo, pAllocator, pFence);
    VkResult result = layer_data->device_dispatch_table.CreateFence(device, pCreateInfo, pAllocator, pFence);
    if (VK_SUCCESS == result) {
        *pFence = layer_data->WrapNew(*pFence);
    }
    return result;
}

void DispatchDestroyFence(
    VkDevice                                    device,
    VkFence                                     fence,
    const VkAllocationCallbacks*                pAllocator)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.DestroyFence(device, fence, pAllocator);
    uint64_t fence_id = reinterpret_cast<uint64_t &>(fence);
    auto iter = unique_id_mapping.pop(fence_id);
    if (iter != unique_id_mapping.end()) {
        fence = (VkFence)iter->second;
    } else {
        fence = (VkFence)0;
    }
    layer_data->device_dispatch_table.DestroyFence(device, fence, pAllocator);

}

VkResult DispatchResetFences(
    VkDevice                                    device,
    uint32_t                                    fenceCount,
    const VkFence*                              pFences)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.ResetFences(device, fenceCount, pFences);
    VkFence *local_pFences = NULL;
    {
        if (pFences) {
            local_pFences = new VkFence[fenceCount];
            for (uint32_t index0 = 0; index0 < fenceCount; ++index0) {
                local_pFences[index0] = layer_data->Unwrap(pFences[index0]);
            }
        }
    }
    VkResult result = layer_data->device_dispatch_table.ResetFences(device, fenceCount, (const VkFence*)local_pFences);
    if (local_pFences)
        delete[] local_pFences;
    return result;
}

VkResult DispatchGetFenceStatus(
    VkDevice                                    device,
    VkFence                                     fence)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.GetFenceStatus(device, fence);
    {
        fence = layer_data->Unwrap(fence);
    }
    VkResult result = layer_data->device_dispatch_table.GetFenceStatus(device, fence);

    return result;
}

VkResult DispatchWaitForFences(
    VkDevice                                    device,
    uint32_t                                    fenceCount,
    const VkFence*                              pFences,
    VkBool32                                    waitAll,
    uint64_t                                    timeout)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.WaitForFences(device, fenceCount, pFences, waitAll, timeout);
    VkFence *local_pFences = NULL;
    {
        if (pFences) {
            local_pFences = new VkFence[fenceCount];
            for (uint32_t index0 = 0; index0 < fenceCount; ++index0) {
                local_pFences[index0] = layer_data->Unwrap(pFences[index0]);
            }
        }
    }
    VkResult result = layer_data->device_dispatch_table.WaitForFences(device, fenceCount, (const VkFence*)local_pFences, waitAll, timeout);
    if (local_pFences)
        delete[] local_pFences;
    return result;
}

VkResult DispatchCreateSemaphore(
    VkDevice                                    device,
    const VkSemaphoreCreateInfo*                pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSemaphore*                                pSemaphore)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CreateSemaphore(device, pCreateInfo, pAllocator, pSemaphore);
    VkResult result = layer_data->device_dispatch_table.CreateSemaphore(device, pCreateInfo, pAllocator, pSemaphore);
    if (VK_SUCCESS == result) {
        *pSemaphore = layer_data->WrapNew(*pSemaphore);
    }
    return result;
}

void DispatchDestroySemaphore(
    VkDevice                                    device,
    VkSemaphore                                 semaphore,
    const VkAllocationCallbacks*                pAllocator)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.DestroySemaphore(device, semaphore, pAllocator);
    uint64_t semaphore_id = reinterpret_cast<uint64_t &>(semaphore);
    auto iter = unique_id_mapping.pop(semaphore_id);
    if (iter != unique_id_mapping.end()) {
        semaphore = (VkSemaphore)iter->second;
    } else {
        semaphore = (VkSemaphore)0;
    }
    layer_data->device_dispatch_table.DestroySemaphore(device, semaphore, pAllocator);

}

VkResult DispatchCreateEvent(
    VkDevice                                    device,
    const VkEventCreateInfo*                    pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkEvent*                                    pEvent)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CreateEvent(device, pCreateInfo, pAllocator, pEvent);
    VkResult result = layer_data->device_dispatch_table.CreateEvent(device, pCreateInfo, pAllocator, pEvent);
    if (VK_SUCCESS == result) {
        *pEvent = layer_data->WrapNew(*pEvent);
    }
    return result;
}

void DispatchDestroyEvent(
    VkDevice                                    device,
    VkEvent                                     event,
    const VkAllocationCallbacks*                pAllocator)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.DestroyEvent(device, event, pAllocator);
    uint64_t event_id = reinterpret_cast<uint64_t &>(event);
    auto iter = unique_id_mapping.pop(event_id);
    if (iter != unique_id_mapping.end()) {
        event = (VkEvent)iter->second;
    } else {
        event = (VkEvent)0;
    }
    layer_data->device_dispatch_table.DestroyEvent(device, event, pAllocator);

}

VkResult DispatchGetEventStatus(
    VkDevice                                    device,
    VkEvent                                     event)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.GetEventStatus(device, event);
    {
        event = layer_data->Unwrap(event);
    }
    VkResult result = layer_data->device_dispatch_table.GetEventStatus(device, event);

    return result;
}

VkResult DispatchSetEvent(
    VkDevice                                    device,
    VkEvent                                     event)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.SetEvent(device, event);
    {
        event = layer_data->Unwrap(event);
    }
    VkResult result = layer_data->device_dispatch_table.SetEvent(device, event);

    return result;
}

VkResult DispatchResetEvent(
    VkDevice                                    device,
    VkEvent                                     event)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.ResetEvent(device, event);
    {
        event = layer_data->Unwrap(event);
    }
    VkResult result = layer_data->device_dispatch_table.ResetEvent(device, event);

    return result;
}

VkResult DispatchCreateQueryPool(
    VkDevice                                    device,
    const VkQueryPoolCreateInfo*                pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkQueryPool*                                pQueryPool)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CreateQueryPool(device, pCreateInfo, pAllocator, pQueryPool);
    VkResult result = layer_data->device_dispatch_table.CreateQueryPool(device, pCreateInfo, pAllocator, pQueryPool);
    if (VK_SUCCESS == result) {
        *pQueryPool = layer_data->WrapNew(*pQueryPool);
    }
    return result;
}

void DispatchDestroyQueryPool(
    VkDevice                                    device,
    VkQueryPool                                 queryPool,
    const VkAllocationCallbacks*                pAllocator)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.DestroyQueryPool(device, queryPool, pAllocator);
    uint64_t queryPool_id = reinterpret_cast<uint64_t &>(queryPool);
    auto iter = unique_id_mapping.pop(queryPool_id);
    if (iter != unique_id_mapping.end()) {
        queryPool = (VkQueryPool)iter->second;
    } else {
        queryPool = (VkQueryPool)0;
    }
    layer_data->device_dispatch_table.DestroyQueryPool(device, queryPool, pAllocator);

}

VkResult DispatchGetQueryPoolResults(
    VkDevice                                    device,
    VkQueryPool                                 queryPool,
    uint32_t                                    firstQuery,
    uint32_t                                    queryCount,
    size_t                                      dataSize,
    void*                                       pData,
    VkDeviceSize                                stride,
    VkQueryResultFlags                          flags)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.GetQueryPoolResults(device, queryPool, firstQuery, queryCount, dataSize, pData, stride, flags);
    {
        queryPool = layer_data->Unwrap(queryPool);
    }
    VkResult result = layer_data->device_dispatch_table.GetQueryPoolResults(device, queryPool, firstQuery, queryCount, dataSize, pData, stride, flags);

    return result;
}

VkResult DispatchCreateBuffer(
    VkDevice                                    device,
    const VkBufferCreateInfo*                   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkBuffer*                                   pBuffer)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CreateBuffer(device, pCreateInfo, pAllocator, pBuffer);
    VkResult result = layer_data->device_dispatch_table.CreateBuffer(device, pCreateInfo, pAllocator, pBuffer);
    if (VK_SUCCESS == result) {
        *pBuffer = layer_data->WrapNew(*pBuffer);
    }
    return result;
}

void DispatchDestroyBuffer(
    VkDevice                                    device,
    VkBuffer                                    buffer,
    const VkAllocationCallbacks*                pAllocator)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.DestroyBuffer(device, buffer, pAllocator);
    uint64_t buffer_id = reinterpret_cast<uint64_t &>(buffer);
    auto iter = unique_id_mapping.pop(buffer_id);
    if (iter != unique_id_mapping.end()) {
        buffer = (VkBuffer)iter->second;
    } else {
        buffer = (VkBuffer)0;
    }
    layer_data->device_dispatch_table.DestroyBuffer(device, buffer, pAllocator);

}

VkResult DispatchCreateBufferView(
    VkDevice                                    device,
    const VkBufferViewCreateInfo*               pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkBufferView*                               pView)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CreateBufferView(device, pCreateInfo, pAllocator, pView);
    safe_VkBufferViewCreateInfo *local_pCreateInfo = NULL;
    {
        if (pCreateInfo) {
            local_pCreateInfo = new safe_VkBufferViewCreateInfo(pCreateInfo);
            if (pCreateInfo->buffer) {
                local_pCreateInfo->buffer = layer_data->Unwrap(pCreateInfo->buffer);
            }
        }
    }
    VkResult result = layer_data->device_dispatch_table.CreateBufferView(device, (const VkBufferViewCreateInfo*)local_pCreateInfo, pAllocator, pView);
    if (local_pCreateInfo) {
        delete local_pCreateInfo;
    }
    if (VK_SUCCESS == result) {
        *pView = layer_data->WrapNew(*pView);
    }
    return result;
}

void DispatchDestroyBufferView(
    VkDevice                                    device,
    VkBufferView                                bufferView,
    const VkAllocationCallbacks*                pAllocator)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.DestroyBufferView(device, bufferView, pAllocator);
    uint64_t bufferView_id = reinterpret_cast<uint64_t &>(bufferView);
    auto iter = unique_id_mapping.pop(bufferView_id);
    if (iter != unique_id_mapping.end()) {
        bufferView = (VkBufferView)iter->second;
    } else {
        bufferView = (VkBufferView)0;
    }
    layer_data->device_dispatch_table.DestroyBufferView(device, bufferView, pAllocator);

}

VkResult DispatchCreateImage(
    VkDevice                                    device,
    const VkImageCreateInfo*                    pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkImage*                                    pImage)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CreateImage(device, pCreateInfo, pAllocator, pImage);
    safe_VkImageCreateInfo *local_pCreateInfo = NULL;
    {
        if (pCreateInfo) {
            local_pCreateInfo = new safe_VkImageCreateInfo(pCreateInfo);
            WrapPnextChainHandles(layer_data, local_pCreateInfo->pNext);
        }
    }
    VkResult result = layer_data->device_dispatch_table.CreateImage(device, (const VkImageCreateInfo*)local_pCreateInfo, pAllocator, pImage);
    if (local_pCreateInfo) {
        delete local_pCreateInfo;
    }
    if (VK_SUCCESS == result) {
        *pImage = layer_data->WrapNew(*pImage);
    }
    return result;
}

void DispatchDestroyImage(
    VkDevice                                    device,
    VkImage                                     image,
    const VkAllocationCallbacks*                pAllocator)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.DestroyImage(device, image, pAllocator);
    uint64_t image_id = reinterpret_cast<uint64_t &>(image);
    auto iter = unique_id_mapping.pop(image_id);
    if (iter != unique_id_mapping.end()) {
        image = (VkImage)iter->second;
    } else {
        image = (VkImage)0;
    }
    layer_data->device_dispatch_table.DestroyImage(device, image, pAllocator);

}

void DispatchGetImageSubresourceLayout(
    VkDevice                                    device,
    VkImage                                     image,
    const VkImageSubresource*                   pSubresource,
    VkSubresourceLayout*                        pLayout)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.GetImageSubresourceLayout(device, image, pSubresource, pLayout);
    {
        image = layer_data->Unwrap(image);
    }
    layer_data->device_dispatch_table.GetImageSubresourceLayout(device, image, pSubresource, pLayout);

}

VkResult DispatchCreateImageView(
    VkDevice                                    device,
    const VkImageViewCreateInfo*                pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkImageView*                                pView)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CreateImageView(device, pCreateInfo, pAllocator, pView);
    safe_VkImageViewCreateInfo *local_pCreateInfo = NULL;
    {
        if (pCreateInfo) {
            local_pCreateInfo = new safe_VkImageViewCreateInfo(pCreateInfo);
            if (pCreateInfo->image) {
                local_pCreateInfo->image = layer_data->Unwrap(pCreateInfo->image);
            }
            WrapPnextChainHandles(layer_data, local_pCreateInfo->pNext);
        }
    }
    VkResult result = layer_data->device_dispatch_table.CreateImageView(device, (const VkImageViewCreateInfo*)local_pCreateInfo, pAllocator, pView);
    if (local_pCreateInfo) {
        delete local_pCreateInfo;
    }
    if (VK_SUCCESS == result) {
        *pView = layer_data->WrapNew(*pView);
    }
    return result;
}

void DispatchDestroyImageView(
    VkDevice                                    device,
    VkImageView                                 imageView,
    const VkAllocationCallbacks*                pAllocator)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.DestroyImageView(device, imageView, pAllocator);
    uint64_t imageView_id = reinterpret_cast<uint64_t &>(imageView);
    auto iter = unique_id_mapping.pop(imageView_id);
    if (iter != unique_id_mapping.end()) {
        imageView = (VkImageView)iter->second;
    } else {
        imageView = (VkImageView)0;
    }
    layer_data->device_dispatch_table.DestroyImageView(device, imageView, pAllocator);

}

VkResult DispatchCreateShaderModule(
    VkDevice                                    device,
    const VkShaderModuleCreateInfo*             pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkShaderModule*                             pShaderModule)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CreateShaderModule(device, pCreateInfo, pAllocator, pShaderModule);
    safe_VkShaderModuleCreateInfo *local_pCreateInfo = NULL;
    {
        if (pCreateInfo) {
            local_pCreateInfo = new safe_VkShaderModuleCreateInfo(pCreateInfo);
            WrapPnextChainHandles(layer_data, local_pCreateInfo->pNext);
        }
    }
    VkResult result = layer_data->device_dispatch_table.CreateShaderModule(device, (const VkShaderModuleCreateInfo*)local_pCreateInfo, pAllocator, pShaderModule);
    if (local_pCreateInfo) {
        delete local_pCreateInfo;
    }
    if (VK_SUCCESS == result) {
        *pShaderModule = layer_data->WrapNew(*pShaderModule);
    }
    return result;
}

void DispatchDestroyShaderModule(
    VkDevice                                    device,
    VkShaderModule                              shaderModule,
    const VkAllocationCallbacks*                pAllocator)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.DestroyShaderModule(device, shaderModule, pAllocator);
    uint64_t shaderModule_id = reinterpret_cast<uint64_t &>(shaderModule);
    auto iter = unique_id_mapping.pop(shaderModule_id);
    if (iter != unique_id_mapping.end()) {
        shaderModule = (VkShaderModule)iter->second;
    } else {
        shaderModule = (VkShaderModule)0;
    }
    layer_data->device_dispatch_table.DestroyShaderModule(device, shaderModule, pAllocator);

}

VkResult DispatchCreatePipelineCache(
    VkDevice                                    device,
    const VkPipelineCacheCreateInfo*            pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkPipelineCache*                            pPipelineCache)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CreatePipelineCache(device, pCreateInfo, pAllocator, pPipelineCache);
    VkResult result = layer_data->device_dispatch_table.CreatePipelineCache(device, pCreateInfo, pAllocator, pPipelineCache);
    if (VK_SUCCESS == result) {
        *pPipelineCache = layer_data->WrapNew(*pPipelineCache);
    }
    return result;
}

void DispatchDestroyPipelineCache(
    VkDevice                                    device,
    VkPipelineCache                             pipelineCache,
    const VkAllocationCallbacks*                pAllocator)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.DestroyPipelineCache(device, pipelineCache, pAllocator);
    uint64_t pipelineCache_id = reinterpret_cast<uint64_t &>(pipelineCache);
    auto iter = unique_id_mapping.pop(pipelineCache_id);
    if (iter != unique_id_mapping.end()) {
        pipelineCache = (VkPipelineCache)iter->second;
    } else {
        pipelineCache = (VkPipelineCache)0;
    }
    layer_data->device_dispatch_table.DestroyPipelineCache(device, pipelineCache, pAllocator);

}

VkResult DispatchGetPipelineCacheData(
    VkDevice                                    device,
    VkPipelineCache                             pipelineCache,
    size_t*                                     pDataSize,
    void*                                       pData)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.GetPipelineCacheData(device, pipelineCache, pDataSize, pData);
    {
        pipelineCache = layer_data->Unwrap(pipelineCache);
    }
    VkResult result = layer_data->device_dispatch_table.GetPipelineCacheData(device, pipelineCache, pDataSize, pData);

    return result;
}

VkResult DispatchMergePipelineCaches(
    VkDevice                                    device,
    VkPipelineCache                             dstCache,
    uint32_t                                    srcCacheCount,
    const VkPipelineCache*                      pSrcCaches)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.MergePipelineCaches(device, dstCache, srcCacheCount, pSrcCaches);
    VkPipelineCache *local_pSrcCaches = NULL;
    {
        dstCache = layer_data->Unwrap(dstCache);
        if (pSrcCaches) {
            local_pSrcCaches = new VkPipelineCache[srcCacheCount];
            for (uint32_t index0 = 0; index0 < srcCacheCount; ++index0) {
                local_pSrcCaches[index0] = layer_data->Unwrap(pSrcCaches[index0]);
            }
        }
    }
    VkResult result = layer_data->device_dispatch_table.MergePipelineCaches(device, dstCache, srcCacheCount, (const VkPipelineCache*)local_pSrcCaches);
    if (local_pSrcCaches)
        delete[] local_pSrcCaches;
    return result;
}

// Skip vkCreateGraphicsPipelines dispatch, manually generated

// Skip vkCreateComputePipelines dispatch, manually generated

void DispatchDestroyPipeline(
    VkDevice                                    device,
    VkPipeline                                  pipeline,
    const VkAllocationCallbacks*                pAllocator)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.DestroyPipeline(device, pipeline, pAllocator);
    uint64_t pipeline_id = reinterpret_cast<uint64_t &>(pipeline);
    auto iter = unique_id_mapping.pop(pipeline_id);
    if (iter != unique_id_mapping.end()) {
        pipeline = (VkPipeline)iter->second;
    } else {
        pipeline = (VkPipeline)0;
    }
    layer_data->device_dispatch_table.DestroyPipeline(device, pipeline, pAllocator);

}

VkResult DispatchCreatePipelineLayout(
    VkDevice                                    device,
    const VkPipelineLayoutCreateInfo*           pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkPipelineLayout*                           pPipelineLayout)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CreatePipelineLayout(device, pCreateInfo, pAllocator, pPipelineLayout);
    safe_VkPipelineLayoutCreateInfo *local_pCreateInfo = NULL;
    {
        if (pCreateInfo) {
            local_pCreateInfo = new safe_VkPipelineLayoutCreateInfo(pCreateInfo);
            if (local_pCreateInfo->pSetLayouts) {
                for (uint32_t index1 = 0; index1 < local_pCreateInfo->setLayoutCount; ++index1) {
                    local_pCreateInfo->pSetLayouts[index1] = layer_data->Unwrap(local_pCreateInfo->pSetLayouts[index1]);
                }
            }
        }
    }
    VkResult result = layer_data->device_dispatch_table.CreatePipelineLayout(device, (const VkPipelineLayoutCreateInfo*)local_pCreateInfo, pAllocator, pPipelineLayout);
    if (local_pCreateInfo) {
        delete local_pCreateInfo;
    }
    if (VK_SUCCESS == result) {
        *pPipelineLayout = layer_data->WrapNew(*pPipelineLayout);
    }
    return result;
}

void DispatchDestroyPipelineLayout(
    VkDevice                                    device,
    VkPipelineLayout                            pipelineLayout,
    const VkAllocationCallbacks*                pAllocator)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.DestroyPipelineLayout(device, pipelineLayout, pAllocator);
    uint64_t pipelineLayout_id = reinterpret_cast<uint64_t &>(pipelineLayout);
    auto iter = unique_id_mapping.pop(pipelineLayout_id);
    if (iter != unique_id_mapping.end()) {
        pipelineLayout = (VkPipelineLayout)iter->second;
    } else {
        pipelineLayout = (VkPipelineLayout)0;
    }
    layer_data->device_dispatch_table.DestroyPipelineLayout(device, pipelineLayout, pAllocator);

}

VkResult DispatchCreateSampler(
    VkDevice                                    device,
    const VkSamplerCreateInfo*                  pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSampler*                                  pSampler)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CreateSampler(device, pCreateInfo, pAllocator, pSampler);
    safe_VkSamplerCreateInfo *local_pCreateInfo = NULL;
    {
        if (pCreateInfo) {
            local_pCreateInfo = new safe_VkSamplerCreateInfo(pCreateInfo);
            WrapPnextChainHandles(layer_data, local_pCreateInfo->pNext);
        }
    }
    VkResult result = layer_data->device_dispatch_table.CreateSampler(device, (const VkSamplerCreateInfo*)local_pCreateInfo, pAllocator, pSampler);
    if (local_pCreateInfo) {
        delete local_pCreateInfo;
    }
    if (VK_SUCCESS == result) {
        *pSampler = layer_data->WrapNew(*pSampler);
    }
    return result;
}

void DispatchDestroySampler(
    VkDevice                                    device,
    VkSampler                                   sampler,
    const VkAllocationCallbacks*                pAllocator)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.DestroySampler(device, sampler, pAllocator);
    uint64_t sampler_id = reinterpret_cast<uint64_t &>(sampler);
    auto iter = unique_id_mapping.pop(sampler_id);
    if (iter != unique_id_mapping.end()) {
        sampler = (VkSampler)iter->second;
    } else {
        sampler = (VkSampler)0;
    }
    layer_data->device_dispatch_table.DestroySampler(device, sampler, pAllocator);

}

VkResult DispatchCreateDescriptorSetLayout(
    VkDevice                                    device,
    const VkDescriptorSetLayoutCreateInfo*      pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDescriptorSetLayout*                      pSetLayout)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CreateDescriptorSetLayout(device, pCreateInfo, pAllocator, pSetLayout);
    safe_VkDescriptorSetLayoutCreateInfo *local_pCreateInfo = NULL;
    {
        if (pCreateInfo) {
            local_pCreateInfo = new safe_VkDescriptorSetLayoutCreateInfo(pCreateInfo);
            if (local_pCreateInfo->pBindings) {
                for (uint32_t index1 = 0; index1 < local_pCreateInfo->bindingCount; ++index1) {
                    if (local_pCreateInfo->pBindings[index1].pImmutableSamplers) {
                        for (uint32_t index2 = 0; index2 < local_pCreateInfo->pBindings[index1].descriptorCount; ++index2) {
                            local_pCreateInfo->pBindings[index1].pImmutableSamplers[index2] = layer_data->Unwrap(local_pCreateInfo->pBindings[index1].pImmutableSamplers[index2]);
                        }
                    }
                }
            }
        }
    }
    VkResult result = layer_data->device_dispatch_table.CreateDescriptorSetLayout(device, (const VkDescriptorSetLayoutCreateInfo*)local_pCreateInfo, pAllocator, pSetLayout);
    if (local_pCreateInfo) {
        delete local_pCreateInfo;
    }
    if (VK_SUCCESS == result) {
        *pSetLayout = layer_data->WrapNew(*pSetLayout);
    }
    return result;
}

void DispatchDestroyDescriptorSetLayout(
    VkDevice                                    device,
    VkDescriptorSetLayout                       descriptorSetLayout,
    const VkAllocationCallbacks*                pAllocator)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.DestroyDescriptorSetLayout(device, descriptorSetLayout, pAllocator);
    uint64_t descriptorSetLayout_id = reinterpret_cast<uint64_t &>(descriptorSetLayout);
    auto iter = unique_id_mapping.pop(descriptorSetLayout_id);
    if (iter != unique_id_mapping.end()) {
        descriptorSetLayout = (VkDescriptorSetLayout)iter->second;
    } else {
        descriptorSetLayout = (VkDescriptorSetLayout)0;
    }
    layer_data->device_dispatch_table.DestroyDescriptorSetLayout(device, descriptorSetLayout, pAllocator);

}

VkResult DispatchCreateDescriptorPool(
    VkDevice                                    device,
    const VkDescriptorPoolCreateInfo*           pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDescriptorPool*                           pDescriptorPool)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CreateDescriptorPool(device, pCreateInfo, pAllocator, pDescriptorPool);
    VkResult result = layer_data->device_dispatch_table.CreateDescriptorPool(device, pCreateInfo, pAllocator, pDescriptorPool);
    if (VK_SUCCESS == result) {
        *pDescriptorPool = layer_data->WrapNew(*pDescriptorPool);
    }
    return result;
}

// Skip vkDestroyDescriptorPool dispatch, manually generated

// Skip vkResetDescriptorPool dispatch, manually generated

// Skip vkAllocateDescriptorSets dispatch, manually generated

// Skip vkFreeDescriptorSets dispatch, manually generated

void DispatchUpdateDescriptorSets(
    VkDevice                                    device,
    uint32_t                                    descriptorWriteCount,
    const VkWriteDescriptorSet*                 pDescriptorWrites,
    uint32_t                                    descriptorCopyCount,
    const VkCopyDescriptorSet*                  pDescriptorCopies)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.UpdateDescriptorSets(device, descriptorWriteCount, pDescriptorWrites, descriptorCopyCount, pDescriptorCopies);
    safe_VkWriteDescriptorSet *local_pDescriptorWrites = NULL;
    safe_VkCopyDescriptorSet *local_pDescriptorCopies = NULL;
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
                            local_pDescriptorWrites[index0].pImageInfo[index1].sampler = layer_data->Unwrap(pDescriptorWrites[index0].pImageInfo[index1].sampler);
                        }
                        if (pDescriptorWrites[index0].pImageInfo[index1].imageView) {
                            local_pDescriptorWrites[index0].pImageInfo[index1].imageView = layer_data->Unwrap(pDescriptorWrites[index0].pImageInfo[index1].imageView);
                        }
                    }
                }
                if (local_pDescriptorWrites[index0].pBufferInfo) {
                    for (uint32_t index1 = 0; index1 < local_pDescriptorWrites[index0].descriptorCount; ++index1) {
                        if (pDescriptorWrites[index0].pBufferInfo[index1].buffer) {
                            local_pDescriptorWrites[index0].pBufferInfo[index1].buffer = layer_data->Unwrap(pDescriptorWrites[index0].pBufferInfo[index1].buffer);
                        }
                    }
                }
                if (local_pDescriptorWrites[index0].pTexelBufferView) {
                    for (uint32_t index1 = 0; index1 < local_pDescriptorWrites[index0].descriptorCount; ++index1) {
                        local_pDescriptorWrites[index0].pTexelBufferView[index1] = layer_data->Unwrap(local_pDescriptorWrites[index0].pTexelBufferView[index1]);
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
    layer_data->device_dispatch_table.UpdateDescriptorSets(device, descriptorWriteCount, (const VkWriteDescriptorSet*)local_pDescriptorWrites, descriptorCopyCount, (const VkCopyDescriptorSet*)local_pDescriptorCopies);
    if (local_pDescriptorWrites) {
        delete[] local_pDescriptorWrites;
    }
    if (local_pDescriptorCopies) {
        delete[] local_pDescriptorCopies;
    }
}

VkResult DispatchCreateFramebuffer(
    VkDevice                                    device,
    const VkFramebufferCreateInfo*              pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkFramebuffer*                              pFramebuffer)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CreateFramebuffer(device, pCreateInfo, pAllocator, pFramebuffer);
    safe_VkFramebufferCreateInfo *local_pCreateInfo = NULL;
    {
        if (pCreateInfo) {
            local_pCreateInfo = new safe_VkFramebufferCreateInfo(pCreateInfo);
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
    VkResult result = layer_data->device_dispatch_table.CreateFramebuffer(device, (const VkFramebufferCreateInfo*)local_pCreateInfo, pAllocator, pFramebuffer);
    if (local_pCreateInfo) {
        delete local_pCreateInfo;
    }
    if (VK_SUCCESS == result) {
        *pFramebuffer = layer_data->WrapNew(*pFramebuffer);
    }
    return result;
}

void DispatchDestroyFramebuffer(
    VkDevice                                    device,
    VkFramebuffer                               framebuffer,
    const VkAllocationCallbacks*                pAllocator)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.DestroyFramebuffer(device, framebuffer, pAllocator);
    uint64_t framebuffer_id = reinterpret_cast<uint64_t &>(framebuffer);
    auto iter = unique_id_mapping.pop(framebuffer_id);
    if (iter != unique_id_mapping.end()) {
        framebuffer = (VkFramebuffer)iter->second;
    } else {
        framebuffer = (VkFramebuffer)0;
    }
    layer_data->device_dispatch_table.DestroyFramebuffer(device, framebuffer, pAllocator);

}

// Skip vkCreateRenderPass dispatch, manually generated

// Skip vkDestroyRenderPass dispatch, manually generated

void DispatchGetRenderAreaGranularity(
    VkDevice                                    device,
    VkRenderPass                                renderPass,
    VkExtent2D*                                 pGranularity)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.GetRenderAreaGranularity(device, renderPass, pGranularity);
    {
        renderPass = layer_data->Unwrap(renderPass);
    }
    layer_data->device_dispatch_table.GetRenderAreaGranularity(device, renderPass, pGranularity);

}

VkResult DispatchCreateCommandPool(
    VkDevice                                    device,
    const VkCommandPoolCreateInfo*              pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkCommandPool*                              pCommandPool)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CreateCommandPool(device, pCreateInfo, pAllocator, pCommandPool);
    VkResult result = layer_data->device_dispatch_table.CreateCommandPool(device, pCreateInfo, pAllocator, pCommandPool);
    if (VK_SUCCESS == result) {
        *pCommandPool = layer_data->WrapNew(*pCommandPool);
    }
    return result;
}

void DispatchDestroyCommandPool(
    VkDevice                                    device,
    VkCommandPool                               commandPool,
    const VkAllocationCallbacks*                pAllocator)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.DestroyCommandPool(device, commandPool, pAllocator);
    uint64_t commandPool_id = reinterpret_cast<uint64_t &>(commandPool);
    auto iter = unique_id_mapping.pop(commandPool_id);
    if (iter != unique_id_mapping.end()) {
        commandPool = (VkCommandPool)iter->second;
    } else {
        commandPool = (VkCommandPool)0;
    }
    layer_data->device_dispatch_table.DestroyCommandPool(device, commandPool, pAllocator);

}

VkResult DispatchResetCommandPool(
    VkDevice                                    device,
    VkCommandPool                               commandPool,
    VkCommandPoolResetFlags                     flags)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.ResetCommandPool(device, commandPool, flags);
    {
        commandPool = layer_data->Unwrap(commandPool);
    }
    VkResult result = layer_data->device_dispatch_table.ResetCommandPool(device, commandPool, flags);

    return result;
}

VkResult DispatchAllocateCommandBuffers(
    VkDevice                                    device,
    const VkCommandBufferAllocateInfo*          pAllocateInfo,
    VkCommandBuffer*                            pCommandBuffers)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.AllocateCommandBuffers(device, pAllocateInfo, pCommandBuffers);
    safe_VkCommandBufferAllocateInfo *local_pAllocateInfo = NULL;
    {
        if (pAllocateInfo) {
            local_pAllocateInfo = new safe_VkCommandBufferAllocateInfo(pAllocateInfo);
            if (pAllocateInfo->commandPool) {
                local_pAllocateInfo->commandPool = layer_data->Unwrap(pAllocateInfo->commandPool);
            }
        }
    }
    VkResult result = layer_data->device_dispatch_table.AllocateCommandBuffers(device, (const VkCommandBufferAllocateInfo*)local_pAllocateInfo, pCommandBuffers);
    if (local_pAllocateInfo) {
        delete local_pAllocateInfo;
    }
    return result;
}

void DispatchFreeCommandBuffers(
    VkDevice                                    device,
    VkCommandPool                               commandPool,
    uint32_t                                    commandBufferCount,
    const VkCommandBuffer*                      pCommandBuffers)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.FreeCommandBuffers(device, commandPool, commandBufferCount, pCommandBuffers);
    {
        commandPool = layer_data->Unwrap(commandPool);
    }
    layer_data->device_dispatch_table.FreeCommandBuffers(device, commandPool, commandBufferCount, pCommandBuffers);

}

VkResult DispatchBeginCommandBuffer(
    VkCommandBuffer                             commandBuffer,
    const VkCommandBufferBeginInfo*             pBeginInfo)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.BeginCommandBuffer(commandBuffer, pBeginInfo);
    safe_VkCommandBufferBeginInfo *local_pBeginInfo = NULL;
    {
        if (pBeginInfo) {
            local_pBeginInfo = new safe_VkCommandBufferBeginInfo(pBeginInfo);
            if (local_pBeginInfo->pInheritanceInfo) {
                if (pBeginInfo->pInheritanceInfo->renderPass) {
                    local_pBeginInfo->pInheritanceInfo->renderPass = layer_data->Unwrap(pBeginInfo->pInheritanceInfo->renderPass);
                }
                if (pBeginInfo->pInheritanceInfo->framebuffer) {
                    local_pBeginInfo->pInheritanceInfo->framebuffer = layer_data->Unwrap(pBeginInfo->pInheritanceInfo->framebuffer);
                }
            }
        }
    }
    VkResult result = layer_data->device_dispatch_table.BeginCommandBuffer(commandBuffer, (const VkCommandBufferBeginInfo*)local_pBeginInfo);
    if (local_pBeginInfo) {
        delete local_pBeginInfo;
    }
    return result;
}

VkResult DispatchEndCommandBuffer(
    VkCommandBuffer                             commandBuffer)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    VkResult result = layer_data->device_dispatch_table.EndCommandBuffer(commandBuffer);

    return result;
}

VkResult DispatchResetCommandBuffer(
    VkCommandBuffer                             commandBuffer,
    VkCommandBufferResetFlags                   flags)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    VkResult result = layer_data->device_dispatch_table.ResetCommandBuffer(commandBuffer, flags);

    return result;
}

void DispatchCmdBindPipeline(
    VkCommandBuffer                             commandBuffer,
    VkPipelineBindPoint                         pipelineBindPoint,
    VkPipeline                                  pipeline)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CmdBindPipeline(commandBuffer, pipelineBindPoint, pipeline);
    {
        pipeline = layer_data->Unwrap(pipeline);
    }
    layer_data->device_dispatch_table.CmdBindPipeline(commandBuffer, pipelineBindPoint, pipeline);

}

void DispatchCmdSetViewport(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstViewport,
    uint32_t                                    viewportCount,
    const VkViewport*                           pViewports)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    layer_data->device_dispatch_table.CmdSetViewport(commandBuffer, firstViewport, viewportCount, pViewports);

}

void DispatchCmdSetScissor(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstScissor,
    uint32_t                                    scissorCount,
    const VkRect2D*                             pScissors)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    layer_data->device_dispatch_table.CmdSetScissor(commandBuffer, firstScissor, scissorCount, pScissors);

}

void DispatchCmdSetLineWidth(
    VkCommandBuffer                             commandBuffer,
    float                                       lineWidth)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    layer_data->device_dispatch_table.CmdSetLineWidth(commandBuffer, lineWidth);

}

void DispatchCmdSetDepthBias(
    VkCommandBuffer                             commandBuffer,
    float                                       depthBiasConstantFactor,
    float                                       depthBiasClamp,
    float                                       depthBiasSlopeFactor)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    layer_data->device_dispatch_table.CmdSetDepthBias(commandBuffer, depthBiasConstantFactor, depthBiasClamp, depthBiasSlopeFactor);

}

void DispatchCmdSetBlendConstants(
    VkCommandBuffer                             commandBuffer,
    const float                                 blendConstants[4])
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    layer_data->device_dispatch_table.CmdSetBlendConstants(commandBuffer, blendConstants);

}

void DispatchCmdSetDepthBounds(
    VkCommandBuffer                             commandBuffer,
    float                                       minDepthBounds,
    float                                       maxDepthBounds)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    layer_data->device_dispatch_table.CmdSetDepthBounds(commandBuffer, minDepthBounds, maxDepthBounds);

}

void DispatchCmdSetStencilCompareMask(
    VkCommandBuffer                             commandBuffer,
    VkStencilFaceFlags                          faceMask,
    uint32_t                                    compareMask)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    layer_data->device_dispatch_table.CmdSetStencilCompareMask(commandBuffer, faceMask, compareMask);

}

void DispatchCmdSetStencilWriteMask(
    VkCommandBuffer                             commandBuffer,
    VkStencilFaceFlags                          faceMask,
    uint32_t                                    writeMask)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    layer_data->device_dispatch_table.CmdSetStencilWriteMask(commandBuffer, faceMask, writeMask);

}

void DispatchCmdSetStencilReference(
    VkCommandBuffer                             commandBuffer,
    VkStencilFaceFlags                          faceMask,
    uint32_t                                    reference)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    layer_data->device_dispatch_table.CmdSetStencilReference(commandBuffer, faceMask, reference);

}

void DispatchCmdBindDescriptorSets(
    VkCommandBuffer                             commandBuffer,
    VkPipelineBindPoint                         pipelineBindPoint,
    VkPipelineLayout                            layout,
    uint32_t                                    firstSet,
    uint32_t                                    descriptorSetCount,
    const VkDescriptorSet*                      pDescriptorSets,
    uint32_t                                    dynamicOffsetCount,
    const uint32_t*                             pDynamicOffsets)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CmdBindDescriptorSets(commandBuffer, pipelineBindPoint, layout, firstSet, descriptorSetCount, pDescriptorSets, dynamicOffsetCount, pDynamicOffsets);
    VkDescriptorSet *local_pDescriptorSets = NULL;
    {
        layout = layer_data->Unwrap(layout);
        if (pDescriptorSets) {
            local_pDescriptorSets = new VkDescriptorSet[descriptorSetCount];
            for (uint32_t index0 = 0; index0 < descriptorSetCount; ++index0) {
                local_pDescriptorSets[index0] = layer_data->Unwrap(pDescriptorSets[index0]);
            }
        }
    }
    layer_data->device_dispatch_table.CmdBindDescriptorSets(commandBuffer, pipelineBindPoint, layout, firstSet, descriptorSetCount, (const VkDescriptorSet*)local_pDescriptorSets, dynamicOffsetCount, pDynamicOffsets);
    if (local_pDescriptorSets)
        delete[] local_pDescriptorSets;
}

void DispatchCmdBindIndexBuffer(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkIndexType                                 indexType)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CmdBindIndexBuffer(commandBuffer, buffer, offset, indexType);
    {
        buffer = layer_data->Unwrap(buffer);
    }
    layer_data->device_dispatch_table.CmdBindIndexBuffer(commandBuffer, buffer, offset, indexType);

}

void DispatchCmdBindVertexBuffers(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstBinding,
    uint32_t                                    bindingCount,
    const VkBuffer*                             pBuffers,
    const VkDeviceSize*                         pOffsets)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CmdBindVertexBuffers(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets);
    VkBuffer *local_pBuffers = NULL;
    {
        if (pBuffers) {
            local_pBuffers = new VkBuffer[bindingCount];
            for (uint32_t index0 = 0; index0 < bindingCount; ++index0) {
                local_pBuffers[index0] = layer_data->Unwrap(pBuffers[index0]);
            }
        }
    }
    layer_data->device_dispatch_table.CmdBindVertexBuffers(commandBuffer, firstBinding, bindingCount, (const VkBuffer*)local_pBuffers, pOffsets);
    if (local_pBuffers)
        delete[] local_pBuffers;
}

void DispatchCmdDraw(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    vertexCount,
    uint32_t                                    instanceCount,
    uint32_t                                    firstVertex,
    uint32_t                                    firstInstance)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    layer_data->device_dispatch_table.CmdDraw(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);

}

void DispatchCmdDrawIndexed(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    indexCount,
    uint32_t                                    instanceCount,
    uint32_t                                    firstIndex,
    int32_t                                     vertexOffset,
    uint32_t                                    firstInstance)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    layer_data->device_dispatch_table.CmdDrawIndexed(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);

}

void DispatchCmdDrawIndirect(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    uint32_t                                    drawCount,
    uint32_t                                    stride)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CmdDrawIndirect(commandBuffer, buffer, offset, drawCount, stride);
    {
        buffer = layer_data->Unwrap(buffer);
    }
    layer_data->device_dispatch_table.CmdDrawIndirect(commandBuffer, buffer, offset, drawCount, stride);

}

void DispatchCmdDrawIndexedIndirect(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    uint32_t                                    drawCount,
    uint32_t                                    stride)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CmdDrawIndexedIndirect(commandBuffer, buffer, offset, drawCount, stride);
    {
        buffer = layer_data->Unwrap(buffer);
    }
    layer_data->device_dispatch_table.CmdDrawIndexedIndirect(commandBuffer, buffer, offset, drawCount, stride);

}

void DispatchCmdDispatch(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    groupCountX,
    uint32_t                                    groupCountY,
    uint32_t                                    groupCountZ)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    layer_data->device_dispatch_table.CmdDispatch(commandBuffer, groupCountX, groupCountY, groupCountZ);

}

void DispatchCmdDispatchIndirect(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CmdDispatchIndirect(commandBuffer, buffer, offset);
    {
        buffer = layer_data->Unwrap(buffer);
    }
    layer_data->device_dispatch_table.CmdDispatchIndirect(commandBuffer, buffer, offset);

}

void DispatchCmdCopyBuffer(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    srcBuffer,
    VkBuffer                                    dstBuffer,
    uint32_t                                    regionCount,
    const VkBufferCopy*                         pRegions)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, regionCount, pRegions);
    {
        srcBuffer = layer_data->Unwrap(srcBuffer);
        dstBuffer = layer_data->Unwrap(dstBuffer);
    }
    layer_data->device_dispatch_table.CmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, regionCount, pRegions);

}

void DispatchCmdCopyImage(
    VkCommandBuffer                             commandBuffer,
    VkImage                                     srcImage,
    VkImageLayout                               srcImageLayout,
    VkImage                                     dstImage,
    VkImageLayout                               dstImageLayout,
    uint32_t                                    regionCount,
    const VkImageCopy*                          pRegions)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CmdCopyImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
    {
        srcImage = layer_data->Unwrap(srcImage);
        dstImage = layer_data->Unwrap(dstImage);
    }
    layer_data->device_dispatch_table.CmdCopyImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);

}

void DispatchCmdBlitImage(
    VkCommandBuffer                             commandBuffer,
    VkImage                                     srcImage,
    VkImageLayout                               srcImageLayout,
    VkImage                                     dstImage,
    VkImageLayout                               dstImageLayout,
    uint32_t                                    regionCount,
    const VkImageBlit*                          pRegions,
    VkFilter                                    filter)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CmdBlitImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions, filter);
    {
        srcImage = layer_data->Unwrap(srcImage);
        dstImage = layer_data->Unwrap(dstImage);
    }
    layer_data->device_dispatch_table.CmdBlitImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions, filter);

}

void DispatchCmdCopyBufferToImage(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    srcBuffer,
    VkImage                                     dstImage,
    VkImageLayout                               dstImageLayout,
    uint32_t                                    regionCount,
    const VkBufferImageCopy*                    pRegions)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CmdCopyBufferToImage(commandBuffer, srcBuffer, dstImage, dstImageLayout, regionCount, pRegions);
    {
        srcBuffer = layer_data->Unwrap(srcBuffer);
        dstImage = layer_data->Unwrap(dstImage);
    }
    layer_data->device_dispatch_table.CmdCopyBufferToImage(commandBuffer, srcBuffer, dstImage, dstImageLayout, regionCount, pRegions);

}

void DispatchCmdCopyImageToBuffer(
    VkCommandBuffer                             commandBuffer,
    VkImage                                     srcImage,
    VkImageLayout                               srcImageLayout,
    VkBuffer                                    dstBuffer,
    uint32_t                                    regionCount,
    const VkBufferImageCopy*                    pRegions)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CmdCopyImageToBuffer(commandBuffer, srcImage, srcImageLayout, dstBuffer, regionCount, pRegions);
    {
        srcImage = layer_data->Unwrap(srcImage);
        dstBuffer = layer_data->Unwrap(dstBuffer);
    }
    layer_data->device_dispatch_table.CmdCopyImageToBuffer(commandBuffer, srcImage, srcImageLayout, dstBuffer, regionCount, pRegions);

}

void DispatchCmdUpdateBuffer(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    dstBuffer,
    VkDeviceSize                                dstOffset,
    VkDeviceSize                                dataSize,
    const void*                                 pData)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CmdUpdateBuffer(commandBuffer, dstBuffer, dstOffset, dataSize, pData);
    {
        dstBuffer = layer_data->Unwrap(dstBuffer);
    }
    layer_data->device_dispatch_table.CmdUpdateBuffer(commandBuffer, dstBuffer, dstOffset, dataSize, pData);

}

void DispatchCmdFillBuffer(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    dstBuffer,
    VkDeviceSize                                dstOffset,
    VkDeviceSize                                size,
    uint32_t                                    data)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CmdFillBuffer(commandBuffer, dstBuffer, dstOffset, size, data);
    {
        dstBuffer = layer_data->Unwrap(dstBuffer);
    }
    layer_data->device_dispatch_table.CmdFillBuffer(commandBuffer, dstBuffer, dstOffset, size, data);

}

void DispatchCmdClearColorImage(
    VkCommandBuffer                             commandBuffer,
    VkImage                                     image,
    VkImageLayout                               imageLayout,
    const VkClearColorValue*                    pColor,
    uint32_t                                    rangeCount,
    const VkImageSubresourceRange*              pRanges)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CmdClearColorImage(commandBuffer, image, imageLayout, pColor, rangeCount, pRanges);
    {
        image = layer_data->Unwrap(image);
    }
    layer_data->device_dispatch_table.CmdClearColorImage(commandBuffer, image, imageLayout, pColor, rangeCount, pRanges);

}

void DispatchCmdClearDepthStencilImage(
    VkCommandBuffer                             commandBuffer,
    VkImage                                     image,
    VkImageLayout                               imageLayout,
    const VkClearDepthStencilValue*             pDepthStencil,
    uint32_t                                    rangeCount,
    const VkImageSubresourceRange*              pRanges)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CmdClearDepthStencilImage(commandBuffer, image, imageLayout, pDepthStencil, rangeCount, pRanges);
    {
        image = layer_data->Unwrap(image);
    }
    layer_data->device_dispatch_table.CmdClearDepthStencilImage(commandBuffer, image, imageLayout, pDepthStencil, rangeCount, pRanges);

}

void DispatchCmdClearAttachments(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    attachmentCount,
    const VkClearAttachment*                    pAttachments,
    uint32_t                                    rectCount,
    const VkClearRect*                          pRects)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    layer_data->device_dispatch_table.CmdClearAttachments(commandBuffer, attachmentCount, pAttachments, rectCount, pRects);

}

void DispatchCmdResolveImage(
    VkCommandBuffer                             commandBuffer,
    VkImage                                     srcImage,
    VkImageLayout                               srcImageLayout,
    VkImage                                     dstImage,
    VkImageLayout                               dstImageLayout,
    uint32_t                                    regionCount,
    const VkImageResolve*                       pRegions)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CmdResolveImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
    {
        srcImage = layer_data->Unwrap(srcImage);
        dstImage = layer_data->Unwrap(dstImage);
    }
    layer_data->device_dispatch_table.CmdResolveImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);

}

void DispatchCmdSetEvent(
    VkCommandBuffer                             commandBuffer,
    VkEvent                                     event,
    VkPipelineStageFlags                        stageMask)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CmdSetEvent(commandBuffer, event, stageMask);
    {
        event = layer_data->Unwrap(event);
    }
    layer_data->device_dispatch_table.CmdSetEvent(commandBuffer, event, stageMask);

}

void DispatchCmdResetEvent(
    VkCommandBuffer                             commandBuffer,
    VkEvent                                     event,
    VkPipelineStageFlags                        stageMask)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CmdResetEvent(commandBuffer, event, stageMask);
    {
        event = layer_data->Unwrap(event);
    }
    layer_data->device_dispatch_table.CmdResetEvent(commandBuffer, event, stageMask);

}

void DispatchCmdWaitEvents(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    eventCount,
    const VkEvent*                              pEvents,
    VkPipelineStageFlags                        srcStageMask,
    VkPipelineStageFlags                        dstStageMask,
    uint32_t                                    memoryBarrierCount,
    const VkMemoryBarrier*                      pMemoryBarriers,
    uint32_t                                    bufferMemoryBarrierCount,
    const VkBufferMemoryBarrier*                pBufferMemoryBarriers,
    uint32_t                                    imageMemoryBarrierCount,
    const VkImageMemoryBarrier*                 pImageMemoryBarriers)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CmdWaitEvents(commandBuffer, eventCount, pEvents, srcStageMask, dstStageMask, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
    VkEvent *local_pEvents = NULL;
    safe_VkBufferMemoryBarrier *local_pBufferMemoryBarriers = NULL;
    safe_VkImageMemoryBarrier *local_pImageMemoryBarriers = NULL;
    {
        if (pEvents) {
            local_pEvents = new VkEvent[eventCount];
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
    layer_data->device_dispatch_table.CmdWaitEvents(commandBuffer, eventCount, (const VkEvent*)local_pEvents, srcStageMask, dstStageMask, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, (const VkBufferMemoryBarrier*)local_pBufferMemoryBarriers, imageMemoryBarrierCount, (const VkImageMemoryBarrier*)local_pImageMemoryBarriers);
    if (local_pEvents)
        delete[] local_pEvents;
    if (local_pBufferMemoryBarriers) {
        delete[] local_pBufferMemoryBarriers;
    }
    if (local_pImageMemoryBarriers) {
        delete[] local_pImageMemoryBarriers;
    }
}

void DispatchCmdPipelineBarrier(
    VkCommandBuffer                             commandBuffer,
    VkPipelineStageFlags                        srcStageMask,
    VkPipelineStageFlags                        dstStageMask,
    VkDependencyFlags                           dependencyFlags,
    uint32_t                                    memoryBarrierCount,
    const VkMemoryBarrier*                      pMemoryBarriers,
    uint32_t                                    bufferMemoryBarrierCount,
    const VkBufferMemoryBarrier*                pBufferMemoryBarriers,
    uint32_t                                    imageMemoryBarrierCount,
    const VkImageMemoryBarrier*                 pImageMemoryBarriers)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CmdPipelineBarrier(commandBuffer, srcStageMask, dstStageMask, dependencyFlags, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
    safe_VkBufferMemoryBarrier *local_pBufferMemoryBarriers = NULL;
    safe_VkImageMemoryBarrier *local_pImageMemoryBarriers = NULL;
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
    layer_data->device_dispatch_table.CmdPipelineBarrier(commandBuffer, srcStageMask, dstStageMask, dependencyFlags, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, (const VkBufferMemoryBarrier*)local_pBufferMemoryBarriers, imageMemoryBarrierCount, (const VkImageMemoryBarrier*)local_pImageMemoryBarriers);
    if (local_pBufferMemoryBarriers) {
        delete[] local_pBufferMemoryBarriers;
    }
    if (local_pImageMemoryBarriers) {
        delete[] local_pImageMemoryBarriers;
    }
}

void DispatchCmdBeginQuery(
    VkCommandBuffer                             commandBuffer,
    VkQueryPool                                 queryPool,
    uint32_t                                    query,
    VkQueryControlFlags                         flags)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CmdBeginQuery(commandBuffer, queryPool, query, flags);
    {
        queryPool = layer_data->Unwrap(queryPool);
    }
    layer_data->device_dispatch_table.CmdBeginQuery(commandBuffer, queryPool, query, flags);

}

void DispatchCmdEndQuery(
    VkCommandBuffer                             commandBuffer,
    VkQueryPool                                 queryPool,
    uint32_t                                    query)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CmdEndQuery(commandBuffer, queryPool, query);
    {
        queryPool = layer_data->Unwrap(queryPool);
    }
    layer_data->device_dispatch_table.CmdEndQuery(commandBuffer, queryPool, query);

}

void DispatchCmdResetQueryPool(
    VkCommandBuffer                             commandBuffer,
    VkQueryPool                                 queryPool,
    uint32_t                                    firstQuery,
    uint32_t                                    queryCount)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CmdResetQueryPool(commandBuffer, queryPool, firstQuery, queryCount);
    {
        queryPool = layer_data->Unwrap(queryPool);
    }
    layer_data->device_dispatch_table.CmdResetQueryPool(commandBuffer, queryPool, firstQuery, queryCount);

}

void DispatchCmdWriteTimestamp(
    VkCommandBuffer                             commandBuffer,
    VkPipelineStageFlagBits                     pipelineStage,
    VkQueryPool                                 queryPool,
    uint32_t                                    query)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CmdWriteTimestamp(commandBuffer, pipelineStage, queryPool, query);
    {
        queryPool = layer_data->Unwrap(queryPool);
    }
    layer_data->device_dispatch_table.CmdWriteTimestamp(commandBuffer, pipelineStage, queryPool, query);

}

void DispatchCmdCopyQueryPoolResults(
    VkCommandBuffer                             commandBuffer,
    VkQueryPool                                 queryPool,
    uint32_t                                    firstQuery,
    uint32_t                                    queryCount,
    VkBuffer                                    dstBuffer,
    VkDeviceSize                                dstOffset,
    VkDeviceSize                                stride,
    VkQueryResultFlags                          flags)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CmdCopyQueryPoolResults(commandBuffer, queryPool, firstQuery, queryCount, dstBuffer, dstOffset, stride, flags);
    {
        queryPool = layer_data->Unwrap(queryPool);
        dstBuffer = layer_data->Unwrap(dstBuffer);
    }
    layer_data->device_dispatch_table.CmdCopyQueryPoolResults(commandBuffer, queryPool, firstQuery, queryCount, dstBuffer, dstOffset, stride, flags);

}

void DispatchCmdPushConstants(
    VkCommandBuffer                             commandBuffer,
    VkPipelineLayout                            layout,
    VkShaderStageFlags                          stageFlags,
    uint32_t                                    offset,
    uint32_t                                    size,
    const void*                                 pValues)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CmdPushConstants(commandBuffer, layout, stageFlags, offset, size, pValues);
    {
        layout = layer_data->Unwrap(layout);
    }
    layer_data->device_dispatch_table.CmdPushConstants(commandBuffer, layout, stageFlags, offset, size, pValues);

}

void DispatchCmdBeginRenderPass(
    VkCommandBuffer                             commandBuffer,
    const VkRenderPassBeginInfo*                pRenderPassBegin,
    VkSubpassContents                           contents)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CmdBeginRenderPass(commandBuffer, pRenderPassBegin, contents);
    safe_VkRenderPassBeginInfo *local_pRenderPassBegin = NULL;
    {
        if (pRenderPassBegin) {
            local_pRenderPassBegin = new safe_VkRenderPassBeginInfo(pRenderPassBegin);
            if (pRenderPassBegin->renderPass) {
                local_pRenderPassBegin->renderPass = layer_data->Unwrap(pRenderPassBegin->renderPass);
            }
            if (pRenderPassBegin->framebuffer) {
                local_pRenderPassBegin->framebuffer = layer_data->Unwrap(pRenderPassBegin->framebuffer);
            }
            WrapPnextChainHandles(layer_data, local_pRenderPassBegin->pNext);
        }
    }
    layer_data->device_dispatch_table.CmdBeginRenderPass(commandBuffer, (const VkRenderPassBeginInfo*)local_pRenderPassBegin, contents);
    if (local_pRenderPassBegin) {
        delete local_pRenderPassBegin;
    }
}

void DispatchCmdNextSubpass(
    VkCommandBuffer                             commandBuffer,
    VkSubpassContents                           contents)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    layer_data->device_dispatch_table.CmdNextSubpass(commandBuffer, contents);

}

void DispatchCmdEndRenderPass(
    VkCommandBuffer                             commandBuffer)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    layer_data->device_dispatch_table.CmdEndRenderPass(commandBuffer);

}

void DispatchCmdExecuteCommands(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    commandBufferCount,
    const VkCommandBuffer*                      pCommandBuffers)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    layer_data->device_dispatch_table.CmdExecuteCommands(commandBuffer, commandBufferCount, pCommandBuffers);

}

// Skip vkEnumerateInstanceVersion dispatch, manually generated

VkResult DispatchBindBufferMemory2(
    VkDevice                                    device,
    uint32_t                                    bindInfoCount,
    const VkBindBufferMemoryInfo*               pBindInfos)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.BindBufferMemory2(device, bindInfoCount, pBindInfos);
    safe_VkBindBufferMemoryInfo *local_pBindInfos = NULL;
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
    VkResult result = layer_data->device_dispatch_table.BindBufferMemory2(device, bindInfoCount, (const VkBindBufferMemoryInfo*)local_pBindInfos);
    if (local_pBindInfos) {
        delete[] local_pBindInfos;
    }
    return result;
}

VkResult DispatchBindImageMemory2(
    VkDevice                                    device,
    uint32_t                                    bindInfoCount,
    const VkBindImageMemoryInfo*                pBindInfos)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.BindImageMemory2(device, bindInfoCount, pBindInfos);
    safe_VkBindImageMemoryInfo *local_pBindInfos = NULL;
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
    VkResult result = layer_data->device_dispatch_table.BindImageMemory2(device, bindInfoCount, (const VkBindImageMemoryInfo*)local_pBindInfos);
    if (local_pBindInfos) {
        delete[] local_pBindInfos;
    }
    return result;
}

void DispatchGetDeviceGroupPeerMemoryFeatures(
    VkDevice                                    device,
    uint32_t                                    heapIndex,
    uint32_t                                    localDeviceIndex,
    uint32_t                                    remoteDeviceIndex,
    VkPeerMemoryFeatureFlags*                   pPeerMemoryFeatures)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    layer_data->device_dispatch_table.GetDeviceGroupPeerMemoryFeatures(device, heapIndex, localDeviceIndex, remoteDeviceIndex, pPeerMemoryFeatures);

}

void DispatchCmdSetDeviceMask(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    deviceMask)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    layer_data->device_dispatch_table.CmdSetDeviceMask(commandBuffer, deviceMask);

}

void DispatchCmdDispatchBase(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    baseGroupX,
    uint32_t                                    baseGroupY,
    uint32_t                                    baseGroupZ,
    uint32_t                                    groupCountX,
    uint32_t                                    groupCountY,
    uint32_t                                    groupCountZ)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    layer_data->device_dispatch_table.CmdDispatchBase(commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ);

}

VkResult DispatchEnumeratePhysicalDeviceGroups(
    VkInstance                                  instance,
    uint32_t*                                   pPhysicalDeviceGroupCount,
    VkPhysicalDeviceGroupProperties*            pPhysicalDeviceGroupProperties)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(instance), layer_data_map);
    VkResult result = layer_data->instance_dispatch_table.EnumeratePhysicalDeviceGroups(instance, pPhysicalDeviceGroupCount, pPhysicalDeviceGroupProperties);

    return result;
}

void DispatchGetImageMemoryRequirements2(
    VkDevice                                    device,
    const VkImageMemoryRequirementsInfo2*       pInfo,
    VkMemoryRequirements2*                      pMemoryRequirements)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.GetImageMemoryRequirements2(device, pInfo, pMemoryRequirements);
    safe_VkImageMemoryRequirementsInfo2 *local_pInfo = NULL;
    {
        if (pInfo) {
            local_pInfo = new safe_VkImageMemoryRequirementsInfo2(pInfo);
            if (pInfo->image) {
                local_pInfo->image = layer_data->Unwrap(pInfo->image);
            }
        }
    }
    layer_data->device_dispatch_table.GetImageMemoryRequirements2(device, (const VkImageMemoryRequirementsInfo2*)local_pInfo, pMemoryRequirements);
    if (local_pInfo) {
        delete local_pInfo;
    }
}

void DispatchGetBufferMemoryRequirements2(
    VkDevice                                    device,
    const VkBufferMemoryRequirementsInfo2*      pInfo,
    VkMemoryRequirements2*                      pMemoryRequirements)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.GetBufferMemoryRequirements2(device, pInfo, pMemoryRequirements);
    safe_VkBufferMemoryRequirementsInfo2 *local_pInfo = NULL;
    {
        if (pInfo) {
            local_pInfo = new safe_VkBufferMemoryRequirementsInfo2(pInfo);
            if (pInfo->buffer) {
                local_pInfo->buffer = layer_data->Unwrap(pInfo->buffer);
            }
        }
    }
    layer_data->device_dispatch_table.GetBufferMemoryRequirements2(device, (const VkBufferMemoryRequirementsInfo2*)local_pInfo, pMemoryRequirements);
    if (local_pInfo) {
        delete local_pInfo;
    }
}

void DispatchGetImageSparseMemoryRequirements2(
    VkDevice                                    device,
    const VkImageSparseMemoryRequirementsInfo2* pInfo,
    uint32_t*                                   pSparseMemoryRequirementCount,
    VkSparseImageMemoryRequirements2*           pSparseMemoryRequirements)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.GetImageSparseMemoryRequirements2(device, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
    safe_VkImageSparseMemoryRequirementsInfo2 *local_pInfo = NULL;
    {
        if (pInfo) {
            local_pInfo = new safe_VkImageSparseMemoryRequirementsInfo2(pInfo);
            if (pInfo->image) {
                local_pInfo->image = layer_data->Unwrap(pInfo->image);
            }
        }
    }
    layer_data->device_dispatch_table.GetImageSparseMemoryRequirements2(device, (const VkImageSparseMemoryRequirementsInfo2*)local_pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
    if (local_pInfo) {
        delete local_pInfo;
    }
}

void DispatchGetPhysicalDeviceFeatures2(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceFeatures2*                  pFeatures)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    layer_data->instance_dispatch_table.GetPhysicalDeviceFeatures2(physicalDevice, pFeatures);

}

void DispatchGetPhysicalDeviceProperties2(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceProperties2*                pProperties)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    layer_data->instance_dispatch_table.GetPhysicalDeviceProperties2(physicalDevice, pProperties);

}

void DispatchGetPhysicalDeviceFormatProperties2(
    VkPhysicalDevice                            physicalDevice,
    VkFormat                                    format,
    VkFormatProperties2*                        pFormatProperties)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    layer_data->instance_dispatch_table.GetPhysicalDeviceFormatProperties2(physicalDevice, format, pFormatProperties);

}

VkResult DispatchGetPhysicalDeviceImageFormatProperties2(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceImageFormatInfo2*     pImageFormatInfo,
    VkImageFormatProperties2*                   pImageFormatProperties)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    if (!wrap_handles) return layer_data->instance_dispatch_table.GetPhysicalDeviceImageFormatProperties2(physicalDevice, pImageFormatInfo, pImageFormatProperties);
    safe_VkPhysicalDeviceImageFormatInfo2 *local_pImageFormatInfo = NULL;
    {
        if (pImageFormatInfo) {
            local_pImageFormatInfo = new safe_VkPhysicalDeviceImageFormatInfo2(pImageFormatInfo);
            WrapPnextChainHandles(layer_data, local_pImageFormatInfo->pNext);
        }
    }
    VkResult result = layer_data->instance_dispatch_table.GetPhysicalDeviceImageFormatProperties2(physicalDevice, (const VkPhysicalDeviceImageFormatInfo2*)local_pImageFormatInfo, pImageFormatProperties);
    if (local_pImageFormatInfo) {
        delete local_pImageFormatInfo;
    }
    return result;
}

void DispatchGetPhysicalDeviceQueueFamilyProperties2(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pQueueFamilyPropertyCount,
    VkQueueFamilyProperties2*                   pQueueFamilyProperties)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    layer_data->instance_dispatch_table.GetPhysicalDeviceQueueFamilyProperties2(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);

}

void DispatchGetPhysicalDeviceMemoryProperties2(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceMemoryProperties2*          pMemoryProperties)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    layer_data->instance_dispatch_table.GetPhysicalDeviceMemoryProperties2(physicalDevice, pMemoryProperties);

}

void DispatchGetPhysicalDeviceSparseImageFormatProperties2(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceSparseImageFormatInfo2* pFormatInfo,
    uint32_t*                                   pPropertyCount,
    VkSparseImageFormatProperties2*             pProperties)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    layer_data->instance_dispatch_table.GetPhysicalDeviceSparseImageFormatProperties2(physicalDevice, pFormatInfo, pPropertyCount, pProperties);

}

void DispatchTrimCommandPool(
    VkDevice                                    device,
    VkCommandPool                               commandPool,
    VkCommandPoolTrimFlags                      flags)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.TrimCommandPool(device, commandPool, flags);
    {
        commandPool = layer_data->Unwrap(commandPool);
    }
    layer_data->device_dispatch_table.TrimCommandPool(device, commandPool, flags);

}

void DispatchGetDeviceQueue2(
    VkDevice                                    device,
    const VkDeviceQueueInfo2*                   pQueueInfo,
    VkQueue*                                    pQueue)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    layer_data->device_dispatch_table.GetDeviceQueue2(device, pQueueInfo, pQueue);

}

VkResult DispatchCreateSamplerYcbcrConversion(
    VkDevice                                    device,
    const VkSamplerYcbcrConversionCreateInfo*   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSamplerYcbcrConversion*                   pYcbcrConversion)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CreateSamplerYcbcrConversion(device, pCreateInfo, pAllocator, pYcbcrConversion);
    safe_VkSamplerYcbcrConversionCreateInfo *local_pCreateInfo = NULL;
    {
        if (pCreateInfo) {
            local_pCreateInfo = new safe_VkSamplerYcbcrConversionCreateInfo(pCreateInfo);
            WrapPnextChainHandles(layer_data, local_pCreateInfo->pNext);
        }
    }
    VkResult result = layer_data->device_dispatch_table.CreateSamplerYcbcrConversion(device, (const VkSamplerYcbcrConversionCreateInfo*)local_pCreateInfo, pAllocator, pYcbcrConversion);
    if (local_pCreateInfo) {
        delete local_pCreateInfo;
    }
    if (VK_SUCCESS == result) {
        *pYcbcrConversion = layer_data->WrapNew(*pYcbcrConversion);
    }
    return result;
}

void DispatchDestroySamplerYcbcrConversion(
    VkDevice                                    device,
    VkSamplerYcbcrConversion                    ycbcrConversion,
    const VkAllocationCallbacks*                pAllocator)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.DestroySamplerYcbcrConversion(device, ycbcrConversion, pAllocator);
    uint64_t ycbcrConversion_id = reinterpret_cast<uint64_t &>(ycbcrConversion);
    auto iter = unique_id_mapping.pop(ycbcrConversion_id);
    if (iter != unique_id_mapping.end()) {
        ycbcrConversion = (VkSamplerYcbcrConversion)iter->second;
    } else {
        ycbcrConversion = (VkSamplerYcbcrConversion)0;
    }
    layer_data->device_dispatch_table.DestroySamplerYcbcrConversion(device, ycbcrConversion, pAllocator);

}

// Skip vkCreateDescriptorUpdateTemplate dispatch, manually generated

// Skip vkDestroyDescriptorUpdateTemplate dispatch, manually generated

// Skip vkUpdateDescriptorSetWithTemplate dispatch, manually generated

void DispatchGetPhysicalDeviceExternalBufferProperties(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceExternalBufferInfo*   pExternalBufferInfo,
    VkExternalBufferProperties*                 pExternalBufferProperties)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    layer_data->instance_dispatch_table.GetPhysicalDeviceExternalBufferProperties(physicalDevice, pExternalBufferInfo, pExternalBufferProperties);

}

void DispatchGetPhysicalDeviceExternalFenceProperties(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceExternalFenceInfo*    pExternalFenceInfo,
    VkExternalFenceProperties*                  pExternalFenceProperties)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    layer_data->instance_dispatch_table.GetPhysicalDeviceExternalFenceProperties(physicalDevice, pExternalFenceInfo, pExternalFenceProperties);

}

void DispatchGetPhysicalDeviceExternalSemaphoreProperties(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceExternalSemaphoreInfo* pExternalSemaphoreInfo,
    VkExternalSemaphoreProperties*              pExternalSemaphoreProperties)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    layer_data->instance_dispatch_table.GetPhysicalDeviceExternalSemaphoreProperties(physicalDevice, pExternalSemaphoreInfo, pExternalSemaphoreProperties);

}

void DispatchGetDescriptorSetLayoutSupport(
    VkDevice                                    device,
    const VkDescriptorSetLayoutCreateInfo*      pCreateInfo,
    VkDescriptorSetLayoutSupport*               pSupport)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.GetDescriptorSetLayoutSupport(device, pCreateInfo, pSupport);
    safe_VkDescriptorSetLayoutCreateInfo *local_pCreateInfo = NULL;
    {
        if (pCreateInfo) {
            local_pCreateInfo = new safe_VkDescriptorSetLayoutCreateInfo(pCreateInfo);
            if (local_pCreateInfo->pBindings) {
                for (uint32_t index1 = 0; index1 < local_pCreateInfo->bindingCount; ++index1) {
                    if (local_pCreateInfo->pBindings[index1].pImmutableSamplers) {
                        for (uint32_t index2 = 0; index2 < local_pCreateInfo->pBindings[index1].descriptorCount; ++index2) {
                            local_pCreateInfo->pBindings[index1].pImmutableSamplers[index2] = layer_data->Unwrap(local_pCreateInfo->pBindings[index1].pImmutableSamplers[index2]);
                        }
                    }
                }
            }
        }
    }
    layer_data->device_dispatch_table.GetDescriptorSetLayoutSupport(device, (const VkDescriptorSetLayoutCreateInfo*)local_pCreateInfo, pSupport);
    if (local_pCreateInfo) {
        delete local_pCreateInfo;
    }
}

void DispatchDestroySurfaceKHR(
    VkInstance                                  instance,
    VkSurfaceKHR                                surface,
    const VkAllocationCallbacks*                pAllocator)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(instance), layer_data_map);
    if (!wrap_handles) return layer_data->instance_dispatch_table.DestroySurfaceKHR(instance, surface, pAllocator);
    uint64_t surface_id = reinterpret_cast<uint64_t &>(surface);
    auto iter = unique_id_mapping.pop(surface_id);
    if (iter != unique_id_mapping.end()) {
        surface = (VkSurfaceKHR)iter->second;
    } else {
        surface = (VkSurfaceKHR)0;
    }
    layer_data->instance_dispatch_table.DestroySurfaceKHR(instance, surface, pAllocator);

}

VkResult DispatchGetPhysicalDeviceSurfaceSupportKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    queueFamilyIndex,
    VkSurfaceKHR                                surface,
    VkBool32*                                   pSupported)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    if (!wrap_handles) return layer_data->instance_dispatch_table.GetPhysicalDeviceSurfaceSupportKHR(physicalDevice, queueFamilyIndex, surface, pSupported);
    {
        surface = layer_data->Unwrap(surface);
    }
    VkResult result = layer_data->instance_dispatch_table.GetPhysicalDeviceSurfaceSupportKHR(physicalDevice, queueFamilyIndex, surface, pSupported);

    return result;
}

VkResult DispatchGetPhysicalDeviceSurfaceCapabilitiesKHR(
    VkPhysicalDevice                            physicalDevice,
    VkSurfaceKHR                                surface,
    VkSurfaceCapabilitiesKHR*                   pSurfaceCapabilities)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    if (!wrap_handles) return layer_data->instance_dispatch_table.GetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, pSurfaceCapabilities);
    {
        surface = layer_data->Unwrap(surface);
    }
    VkResult result = layer_data->instance_dispatch_table.GetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, pSurfaceCapabilities);

    return result;
}

VkResult DispatchGetPhysicalDeviceSurfaceFormatsKHR(
    VkPhysicalDevice                            physicalDevice,
    VkSurfaceKHR                                surface,
    uint32_t*                                   pSurfaceFormatCount,
    VkSurfaceFormatKHR*                         pSurfaceFormats)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    if (!wrap_handles) return layer_data->instance_dispatch_table.GetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, pSurfaceFormatCount, pSurfaceFormats);
    {
        surface = layer_data->Unwrap(surface);
    }
    VkResult result = layer_data->instance_dispatch_table.GetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, pSurfaceFormatCount, pSurfaceFormats);

    return result;
}

VkResult DispatchGetPhysicalDeviceSurfacePresentModesKHR(
    VkPhysicalDevice                            physicalDevice,
    VkSurfaceKHR                                surface,
    uint32_t*                                   pPresentModeCount,
    VkPresentModeKHR*                           pPresentModes)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    if (!wrap_handles) return layer_data->instance_dispatch_table.GetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, pPresentModeCount, pPresentModes);
    {
        surface = layer_data->Unwrap(surface);
    }
    VkResult result = layer_data->instance_dispatch_table.GetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, pPresentModeCount, pPresentModes);

    return result;
}

// Skip vkCreateSwapchainKHR dispatch, manually generated

// Skip vkDestroySwapchainKHR dispatch, manually generated

// Skip vkGetSwapchainImagesKHR dispatch, manually generated

VkResult DispatchAcquireNextImageKHR(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    uint64_t                                    timeout,
    VkSemaphore                                 semaphore,
    VkFence                                     fence,
    uint32_t*                                   pImageIndex)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.AcquireNextImageKHR(device, swapchain, timeout, semaphore, fence, pImageIndex);
    {
        swapchain = layer_data->Unwrap(swapchain);
        semaphore = layer_data->Unwrap(semaphore);
        fence = layer_data->Unwrap(fence);
    }
    VkResult result = layer_data->device_dispatch_table.AcquireNextImageKHR(device, swapchain, timeout, semaphore, fence, pImageIndex);

    return result;
}

// Skip vkQueuePresentKHR dispatch, manually generated

VkResult DispatchGetDeviceGroupPresentCapabilitiesKHR(
    VkDevice                                    device,
    VkDeviceGroupPresentCapabilitiesKHR*        pDeviceGroupPresentCapabilities)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    VkResult result = layer_data->device_dispatch_table.GetDeviceGroupPresentCapabilitiesKHR(device, pDeviceGroupPresentCapabilities);

    return result;
}

VkResult DispatchGetDeviceGroupSurfacePresentModesKHR(
    VkDevice                                    device,
    VkSurfaceKHR                                surface,
    VkDeviceGroupPresentModeFlagsKHR*           pModes)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.GetDeviceGroupSurfacePresentModesKHR(device, surface, pModes);
    {
        surface = layer_data->Unwrap(surface);
    }
    VkResult result = layer_data->device_dispatch_table.GetDeviceGroupSurfacePresentModesKHR(device, surface, pModes);

    return result;
}

VkResult DispatchGetPhysicalDevicePresentRectanglesKHR(
    VkPhysicalDevice                            physicalDevice,
    VkSurfaceKHR                                surface,
    uint32_t*                                   pRectCount,
    VkRect2D*                                   pRects)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    if (!wrap_handles) return layer_data->instance_dispatch_table.GetPhysicalDevicePresentRectanglesKHR(physicalDevice, surface, pRectCount, pRects);
    {
        surface = layer_data->Unwrap(surface);
    }
    VkResult result = layer_data->instance_dispatch_table.GetPhysicalDevicePresentRectanglesKHR(physicalDevice, surface, pRectCount, pRects);

    return result;
}

VkResult DispatchAcquireNextImage2KHR(
    VkDevice                                    device,
    const VkAcquireNextImageInfoKHR*            pAcquireInfo,
    uint32_t*                                   pImageIndex)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.AcquireNextImage2KHR(device, pAcquireInfo, pImageIndex);
    safe_VkAcquireNextImageInfoKHR *local_pAcquireInfo = NULL;
    {
        if (pAcquireInfo) {
            local_pAcquireInfo = new safe_VkAcquireNextImageInfoKHR(pAcquireInfo);
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
    VkResult result = layer_data->device_dispatch_table.AcquireNextImage2KHR(device, (const VkAcquireNextImageInfoKHR*)local_pAcquireInfo, pImageIndex);
    if (local_pAcquireInfo) {
        delete local_pAcquireInfo;
    }
    return result;
}

// Skip vkGetPhysicalDeviceDisplayPropertiesKHR dispatch, manually generated

// Skip vkGetPhysicalDeviceDisplayPlanePropertiesKHR dispatch, manually generated

// Skip vkGetDisplayPlaneSupportedDisplaysKHR dispatch, manually generated

// Skip vkGetDisplayModePropertiesKHR dispatch, manually generated

VkResult DispatchCreateDisplayModeKHR(
    VkPhysicalDevice                            physicalDevice,
    VkDisplayKHR                                display,
    const VkDisplayModeCreateInfoKHR*           pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDisplayModeKHR*                           pMode)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    if (!wrap_handles) return layer_data->instance_dispatch_table.CreateDisplayModeKHR(physicalDevice, display, pCreateInfo, pAllocator, pMode);
    {
        display = layer_data->Unwrap(display);
    }
    VkResult result = layer_data->instance_dispatch_table.CreateDisplayModeKHR(physicalDevice, display, pCreateInfo, pAllocator, pMode);
    if (VK_SUCCESS == result) {
        *pMode = layer_data->WrapNew(*pMode);
    }
    return result;
}

VkResult DispatchGetDisplayPlaneCapabilitiesKHR(
    VkPhysicalDevice                            physicalDevice,
    VkDisplayModeKHR                            mode,
    uint32_t                                    planeIndex,
    VkDisplayPlaneCapabilitiesKHR*              pCapabilities)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    if (!wrap_handles) return layer_data->instance_dispatch_table.GetDisplayPlaneCapabilitiesKHR(physicalDevice, mode, planeIndex, pCapabilities);
    {
        mode = layer_data->Unwrap(mode);
    }
    VkResult result = layer_data->instance_dispatch_table.GetDisplayPlaneCapabilitiesKHR(physicalDevice, mode, planeIndex, pCapabilities);

    return result;
}

VkResult DispatchCreateDisplayPlaneSurfaceKHR(
    VkInstance                                  instance,
    const VkDisplaySurfaceCreateInfoKHR*        pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(instance), layer_data_map);
    if (!wrap_handles) return layer_data->instance_dispatch_table.CreateDisplayPlaneSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
    safe_VkDisplaySurfaceCreateInfoKHR *local_pCreateInfo = NULL;
    {
        if (pCreateInfo) {
            local_pCreateInfo = new safe_VkDisplaySurfaceCreateInfoKHR(pCreateInfo);
            if (pCreateInfo->displayMode) {
                local_pCreateInfo->displayMode = layer_data->Unwrap(pCreateInfo->displayMode);
            }
        }
    }
    VkResult result = layer_data->instance_dispatch_table.CreateDisplayPlaneSurfaceKHR(instance, (const VkDisplaySurfaceCreateInfoKHR*)local_pCreateInfo, pAllocator, pSurface);
    if (local_pCreateInfo) {
        delete local_pCreateInfo;
    }
    if (VK_SUCCESS == result) {
        *pSurface = layer_data->WrapNew(*pSurface);
    }
    return result;
}

// Skip vkCreateSharedSwapchainsKHR dispatch, manually generated

#ifdef VK_USE_PLATFORM_XLIB_KHR

VkResult DispatchCreateXlibSurfaceKHR(
    VkInstance                                  instance,
    const VkXlibSurfaceCreateInfoKHR*           pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(instance), layer_data_map);
    if (!wrap_handles) return layer_data->instance_dispatch_table.CreateXlibSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
    VkResult result = layer_data->instance_dispatch_table.CreateXlibSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
    if (VK_SUCCESS == result) {
        *pSurface = layer_data->WrapNew(*pSurface);
    }
    return result;
}
#endif // VK_USE_PLATFORM_XLIB_KHR

#ifdef VK_USE_PLATFORM_XLIB_KHR

VkBool32 DispatchGetPhysicalDeviceXlibPresentationSupportKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    queueFamilyIndex,
    Display*                                    dpy,
    VisualID                                    visualID)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    VkBool32 result = layer_data->instance_dispatch_table.GetPhysicalDeviceXlibPresentationSupportKHR(physicalDevice, queueFamilyIndex, dpy, visualID);

    return result;
}
#endif // VK_USE_PLATFORM_XLIB_KHR

#ifdef VK_USE_PLATFORM_XCB_KHR

VkResult DispatchCreateXcbSurfaceKHR(
    VkInstance                                  instance,
    const VkXcbSurfaceCreateInfoKHR*            pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(instance), layer_data_map);
    if (!wrap_handles) return layer_data->instance_dispatch_table.CreateXcbSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
    VkResult result = layer_data->instance_dispatch_table.CreateXcbSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
    if (VK_SUCCESS == result) {
        *pSurface = layer_data->WrapNew(*pSurface);
    }
    return result;
}
#endif // VK_USE_PLATFORM_XCB_KHR

#ifdef VK_USE_PLATFORM_XCB_KHR

VkBool32 DispatchGetPhysicalDeviceXcbPresentationSupportKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    queueFamilyIndex,
    xcb_connection_t*                           connection,
    xcb_visualid_t                              visual_id)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    VkBool32 result = layer_data->instance_dispatch_table.GetPhysicalDeviceXcbPresentationSupportKHR(physicalDevice, queueFamilyIndex, connection, visual_id);

    return result;
}
#endif // VK_USE_PLATFORM_XCB_KHR

#ifdef VK_USE_PLATFORM_WAYLAND_KHR

VkResult DispatchCreateWaylandSurfaceKHR(
    VkInstance                                  instance,
    const VkWaylandSurfaceCreateInfoKHR*        pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(instance), layer_data_map);
    if (!wrap_handles) return layer_data->instance_dispatch_table.CreateWaylandSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
    VkResult result = layer_data->instance_dispatch_table.CreateWaylandSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
    if (VK_SUCCESS == result) {
        *pSurface = layer_data->WrapNew(*pSurface);
    }
    return result;
}
#endif // VK_USE_PLATFORM_WAYLAND_KHR

#ifdef VK_USE_PLATFORM_WAYLAND_KHR

VkBool32 DispatchGetPhysicalDeviceWaylandPresentationSupportKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    queueFamilyIndex,
    struct wl_display*                          display)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    VkBool32 result = layer_data->instance_dispatch_table.GetPhysicalDeviceWaylandPresentationSupportKHR(physicalDevice, queueFamilyIndex, display);

    return result;
}
#endif // VK_USE_PLATFORM_WAYLAND_KHR

#ifdef VK_USE_PLATFORM_ANDROID_KHR

VkResult DispatchCreateAndroidSurfaceKHR(
    VkInstance                                  instance,
    const VkAndroidSurfaceCreateInfoKHR*        pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(instance), layer_data_map);
    if (!wrap_handles) return layer_data->instance_dispatch_table.CreateAndroidSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
    VkResult result = layer_data->instance_dispatch_table.CreateAndroidSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
    if (VK_SUCCESS == result) {
        *pSurface = layer_data->WrapNew(*pSurface);
    }
    return result;
}
#endif // VK_USE_PLATFORM_ANDROID_KHR

#ifdef VK_USE_PLATFORM_WIN32_KHR

VkResult DispatchCreateWin32SurfaceKHR(
    VkInstance                                  instance,
    const VkWin32SurfaceCreateInfoKHR*          pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(instance), layer_data_map);
    if (!wrap_handles) return layer_data->instance_dispatch_table.CreateWin32SurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
    VkResult result = layer_data->instance_dispatch_table.CreateWin32SurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
    if (VK_SUCCESS == result) {
        *pSurface = layer_data->WrapNew(*pSurface);
    }
    return result;
}
#endif // VK_USE_PLATFORM_WIN32_KHR

#ifdef VK_USE_PLATFORM_WIN32_KHR

VkBool32 DispatchGetPhysicalDeviceWin32PresentationSupportKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    queueFamilyIndex)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    VkBool32 result = layer_data->instance_dispatch_table.GetPhysicalDeviceWin32PresentationSupportKHR(physicalDevice, queueFamilyIndex);

    return result;
}
#endif // VK_USE_PLATFORM_WIN32_KHR

void DispatchGetPhysicalDeviceFeatures2KHR(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceFeatures2*                  pFeatures)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    layer_data->instance_dispatch_table.GetPhysicalDeviceFeatures2KHR(physicalDevice, pFeatures);

}

void DispatchGetPhysicalDeviceProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceProperties2*                pProperties)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    layer_data->instance_dispatch_table.GetPhysicalDeviceProperties2KHR(physicalDevice, pProperties);

}

void DispatchGetPhysicalDeviceFormatProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    VkFormat                                    format,
    VkFormatProperties2*                        pFormatProperties)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    layer_data->instance_dispatch_table.GetPhysicalDeviceFormatProperties2KHR(physicalDevice, format, pFormatProperties);

}

VkResult DispatchGetPhysicalDeviceImageFormatProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceImageFormatInfo2*     pImageFormatInfo,
    VkImageFormatProperties2*                   pImageFormatProperties)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    if (!wrap_handles) return layer_data->instance_dispatch_table.GetPhysicalDeviceImageFormatProperties2KHR(physicalDevice, pImageFormatInfo, pImageFormatProperties);
    safe_VkPhysicalDeviceImageFormatInfo2 *local_pImageFormatInfo = NULL;
    {
        if (pImageFormatInfo) {
            local_pImageFormatInfo = new safe_VkPhysicalDeviceImageFormatInfo2(pImageFormatInfo);
            WrapPnextChainHandles(layer_data, local_pImageFormatInfo->pNext);
        }
    }
    VkResult result = layer_data->instance_dispatch_table.GetPhysicalDeviceImageFormatProperties2KHR(physicalDevice, (const VkPhysicalDeviceImageFormatInfo2*)local_pImageFormatInfo, pImageFormatProperties);
    if (local_pImageFormatInfo) {
        delete local_pImageFormatInfo;
    }
    return result;
}

void DispatchGetPhysicalDeviceQueueFamilyProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pQueueFamilyPropertyCount,
    VkQueueFamilyProperties2*                   pQueueFamilyProperties)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    layer_data->instance_dispatch_table.GetPhysicalDeviceQueueFamilyProperties2KHR(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);

}

void DispatchGetPhysicalDeviceMemoryProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceMemoryProperties2*          pMemoryProperties)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    layer_data->instance_dispatch_table.GetPhysicalDeviceMemoryProperties2KHR(physicalDevice, pMemoryProperties);

}

void DispatchGetPhysicalDeviceSparseImageFormatProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceSparseImageFormatInfo2* pFormatInfo,
    uint32_t*                                   pPropertyCount,
    VkSparseImageFormatProperties2*             pProperties)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    layer_data->instance_dispatch_table.GetPhysicalDeviceSparseImageFormatProperties2KHR(physicalDevice, pFormatInfo, pPropertyCount, pProperties);

}

void DispatchGetDeviceGroupPeerMemoryFeaturesKHR(
    VkDevice                                    device,
    uint32_t                                    heapIndex,
    uint32_t                                    localDeviceIndex,
    uint32_t                                    remoteDeviceIndex,
    VkPeerMemoryFeatureFlags*                   pPeerMemoryFeatures)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    layer_data->device_dispatch_table.GetDeviceGroupPeerMemoryFeaturesKHR(device, heapIndex, localDeviceIndex, remoteDeviceIndex, pPeerMemoryFeatures);

}

void DispatchCmdSetDeviceMaskKHR(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    deviceMask)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    layer_data->device_dispatch_table.CmdSetDeviceMaskKHR(commandBuffer, deviceMask);

}

void DispatchCmdDispatchBaseKHR(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    baseGroupX,
    uint32_t                                    baseGroupY,
    uint32_t                                    baseGroupZ,
    uint32_t                                    groupCountX,
    uint32_t                                    groupCountY,
    uint32_t                                    groupCountZ)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    layer_data->device_dispatch_table.CmdDispatchBaseKHR(commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ);

}

void DispatchTrimCommandPoolKHR(
    VkDevice                                    device,
    VkCommandPool                               commandPool,
    VkCommandPoolTrimFlags                      flags)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.TrimCommandPoolKHR(device, commandPool, flags);
    {
        commandPool = layer_data->Unwrap(commandPool);
    }
    layer_data->device_dispatch_table.TrimCommandPoolKHR(device, commandPool, flags);

}

VkResult DispatchEnumeratePhysicalDeviceGroupsKHR(
    VkInstance                                  instance,
    uint32_t*                                   pPhysicalDeviceGroupCount,
    VkPhysicalDeviceGroupProperties*            pPhysicalDeviceGroupProperties)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(instance), layer_data_map);
    VkResult result = layer_data->instance_dispatch_table.EnumeratePhysicalDeviceGroupsKHR(instance, pPhysicalDeviceGroupCount, pPhysicalDeviceGroupProperties);

    return result;
}

void DispatchGetPhysicalDeviceExternalBufferPropertiesKHR(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceExternalBufferInfo*   pExternalBufferInfo,
    VkExternalBufferProperties*                 pExternalBufferProperties)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    layer_data->instance_dispatch_table.GetPhysicalDeviceExternalBufferPropertiesKHR(physicalDevice, pExternalBufferInfo, pExternalBufferProperties);

}

#ifdef VK_USE_PLATFORM_WIN32_KHR

VkResult DispatchGetMemoryWin32HandleKHR(
    VkDevice                                    device,
    const VkMemoryGetWin32HandleInfoKHR*        pGetWin32HandleInfo,
    HANDLE*                                     pHandle)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.GetMemoryWin32HandleKHR(device, pGetWin32HandleInfo, pHandle);
    safe_VkMemoryGetWin32HandleInfoKHR *local_pGetWin32HandleInfo = NULL;
    {
        if (pGetWin32HandleInfo) {
            local_pGetWin32HandleInfo = new safe_VkMemoryGetWin32HandleInfoKHR(pGetWin32HandleInfo);
            if (pGetWin32HandleInfo->memory) {
                local_pGetWin32HandleInfo->memory = layer_data->Unwrap(pGetWin32HandleInfo->memory);
            }
        }
    }
    VkResult result = layer_data->device_dispatch_table.GetMemoryWin32HandleKHR(device, (const VkMemoryGetWin32HandleInfoKHR*)local_pGetWin32HandleInfo, pHandle);
    if (local_pGetWin32HandleInfo) {
        delete local_pGetWin32HandleInfo;
    }
    return result;
}
#endif // VK_USE_PLATFORM_WIN32_KHR

#ifdef VK_USE_PLATFORM_WIN32_KHR

VkResult DispatchGetMemoryWin32HandlePropertiesKHR(
    VkDevice                                    device,
    VkExternalMemoryHandleTypeFlagBits          handleType,
    HANDLE                                      handle,
    VkMemoryWin32HandlePropertiesKHR*           pMemoryWin32HandleProperties)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    VkResult result = layer_data->device_dispatch_table.GetMemoryWin32HandlePropertiesKHR(device, handleType, handle, pMemoryWin32HandleProperties);

    return result;
}
#endif // VK_USE_PLATFORM_WIN32_KHR

VkResult DispatchGetMemoryFdKHR(
    VkDevice                                    device,
    const VkMemoryGetFdInfoKHR*                 pGetFdInfo,
    int*                                        pFd)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.GetMemoryFdKHR(device, pGetFdInfo, pFd);
    safe_VkMemoryGetFdInfoKHR *local_pGetFdInfo = NULL;
    {
        if (pGetFdInfo) {
            local_pGetFdInfo = new safe_VkMemoryGetFdInfoKHR(pGetFdInfo);
            if (pGetFdInfo->memory) {
                local_pGetFdInfo->memory = layer_data->Unwrap(pGetFdInfo->memory);
            }
        }
    }
    VkResult result = layer_data->device_dispatch_table.GetMemoryFdKHR(device, (const VkMemoryGetFdInfoKHR*)local_pGetFdInfo, pFd);
    if (local_pGetFdInfo) {
        delete local_pGetFdInfo;
    }
    return result;
}

VkResult DispatchGetMemoryFdPropertiesKHR(
    VkDevice                                    device,
    VkExternalMemoryHandleTypeFlagBits          handleType,
    int                                         fd,
    VkMemoryFdPropertiesKHR*                    pMemoryFdProperties)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    VkResult result = layer_data->device_dispatch_table.GetMemoryFdPropertiesKHR(device, handleType, fd, pMemoryFdProperties);

    return result;
}

void DispatchGetPhysicalDeviceExternalSemaphorePropertiesKHR(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceExternalSemaphoreInfo* pExternalSemaphoreInfo,
    VkExternalSemaphoreProperties*              pExternalSemaphoreProperties)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    layer_data->instance_dispatch_table.GetPhysicalDeviceExternalSemaphorePropertiesKHR(physicalDevice, pExternalSemaphoreInfo, pExternalSemaphoreProperties);

}

#ifdef VK_USE_PLATFORM_WIN32_KHR

VkResult DispatchImportSemaphoreWin32HandleKHR(
    VkDevice                                    device,
    const VkImportSemaphoreWin32HandleInfoKHR*  pImportSemaphoreWin32HandleInfo)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.ImportSemaphoreWin32HandleKHR(device, pImportSemaphoreWin32HandleInfo);
    safe_VkImportSemaphoreWin32HandleInfoKHR *local_pImportSemaphoreWin32HandleInfo = NULL;
    {
        if (pImportSemaphoreWin32HandleInfo) {
            local_pImportSemaphoreWin32HandleInfo = new safe_VkImportSemaphoreWin32HandleInfoKHR(pImportSemaphoreWin32HandleInfo);
            if (pImportSemaphoreWin32HandleInfo->semaphore) {
                local_pImportSemaphoreWin32HandleInfo->semaphore = layer_data->Unwrap(pImportSemaphoreWin32HandleInfo->semaphore);
            }
        }
    }
    VkResult result = layer_data->device_dispatch_table.ImportSemaphoreWin32HandleKHR(device, (const VkImportSemaphoreWin32HandleInfoKHR*)local_pImportSemaphoreWin32HandleInfo);
    if (local_pImportSemaphoreWin32HandleInfo) {
        delete local_pImportSemaphoreWin32HandleInfo;
    }
    return result;
}
#endif // VK_USE_PLATFORM_WIN32_KHR

#ifdef VK_USE_PLATFORM_WIN32_KHR

VkResult DispatchGetSemaphoreWin32HandleKHR(
    VkDevice                                    device,
    const VkSemaphoreGetWin32HandleInfoKHR*     pGetWin32HandleInfo,
    HANDLE*                                     pHandle)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.GetSemaphoreWin32HandleKHR(device, pGetWin32HandleInfo, pHandle);
    safe_VkSemaphoreGetWin32HandleInfoKHR *local_pGetWin32HandleInfo = NULL;
    {
        if (pGetWin32HandleInfo) {
            local_pGetWin32HandleInfo = new safe_VkSemaphoreGetWin32HandleInfoKHR(pGetWin32HandleInfo);
            if (pGetWin32HandleInfo->semaphore) {
                local_pGetWin32HandleInfo->semaphore = layer_data->Unwrap(pGetWin32HandleInfo->semaphore);
            }
        }
    }
    VkResult result = layer_data->device_dispatch_table.GetSemaphoreWin32HandleKHR(device, (const VkSemaphoreGetWin32HandleInfoKHR*)local_pGetWin32HandleInfo, pHandle);
    if (local_pGetWin32HandleInfo) {
        delete local_pGetWin32HandleInfo;
    }
    return result;
}
#endif // VK_USE_PLATFORM_WIN32_KHR

VkResult DispatchImportSemaphoreFdKHR(
    VkDevice                                    device,
    const VkImportSemaphoreFdInfoKHR*           pImportSemaphoreFdInfo)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.ImportSemaphoreFdKHR(device, pImportSemaphoreFdInfo);
    safe_VkImportSemaphoreFdInfoKHR *local_pImportSemaphoreFdInfo = NULL;
    {
        if (pImportSemaphoreFdInfo) {
            local_pImportSemaphoreFdInfo = new safe_VkImportSemaphoreFdInfoKHR(pImportSemaphoreFdInfo);
            if (pImportSemaphoreFdInfo->semaphore) {
                local_pImportSemaphoreFdInfo->semaphore = layer_data->Unwrap(pImportSemaphoreFdInfo->semaphore);
            }
        }
    }
    VkResult result = layer_data->device_dispatch_table.ImportSemaphoreFdKHR(device, (const VkImportSemaphoreFdInfoKHR*)local_pImportSemaphoreFdInfo);
    if (local_pImportSemaphoreFdInfo) {
        delete local_pImportSemaphoreFdInfo;
    }
    return result;
}

VkResult DispatchGetSemaphoreFdKHR(
    VkDevice                                    device,
    const VkSemaphoreGetFdInfoKHR*              pGetFdInfo,
    int*                                        pFd)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.GetSemaphoreFdKHR(device, pGetFdInfo, pFd);
    safe_VkSemaphoreGetFdInfoKHR *local_pGetFdInfo = NULL;
    {
        if (pGetFdInfo) {
            local_pGetFdInfo = new safe_VkSemaphoreGetFdInfoKHR(pGetFdInfo);
            if (pGetFdInfo->semaphore) {
                local_pGetFdInfo->semaphore = layer_data->Unwrap(pGetFdInfo->semaphore);
            }
        }
    }
    VkResult result = layer_data->device_dispatch_table.GetSemaphoreFdKHR(device, (const VkSemaphoreGetFdInfoKHR*)local_pGetFdInfo, pFd);
    if (local_pGetFdInfo) {
        delete local_pGetFdInfo;
    }
    return result;
}

void DispatchCmdPushDescriptorSetKHR(
    VkCommandBuffer                             commandBuffer,
    VkPipelineBindPoint                         pipelineBindPoint,
    VkPipelineLayout                            layout,
    uint32_t                                    set,
    uint32_t                                    descriptorWriteCount,
    const VkWriteDescriptorSet*                 pDescriptorWrites)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CmdPushDescriptorSetKHR(commandBuffer, pipelineBindPoint, layout, set, descriptorWriteCount, pDescriptorWrites);
    safe_VkWriteDescriptorSet *local_pDescriptorWrites = NULL;
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
                            local_pDescriptorWrites[index0].pImageInfo[index1].sampler = layer_data->Unwrap(pDescriptorWrites[index0].pImageInfo[index1].sampler);
                        }
                        if (pDescriptorWrites[index0].pImageInfo[index1].imageView) {
                            local_pDescriptorWrites[index0].pImageInfo[index1].imageView = layer_data->Unwrap(pDescriptorWrites[index0].pImageInfo[index1].imageView);
                        }
                    }
                }
                if (local_pDescriptorWrites[index0].pBufferInfo) {
                    for (uint32_t index1 = 0; index1 < local_pDescriptorWrites[index0].descriptorCount; ++index1) {
                        if (pDescriptorWrites[index0].pBufferInfo[index1].buffer) {
                            local_pDescriptorWrites[index0].pBufferInfo[index1].buffer = layer_data->Unwrap(pDescriptorWrites[index0].pBufferInfo[index1].buffer);
                        }
                    }
                }
                if (local_pDescriptorWrites[index0].pTexelBufferView) {
                    for (uint32_t index1 = 0; index1 < local_pDescriptorWrites[index0].descriptorCount; ++index1) {
                        local_pDescriptorWrites[index0].pTexelBufferView[index1] = layer_data->Unwrap(local_pDescriptorWrites[index0].pTexelBufferView[index1]);
                    }
                }
            }
        }
    }
    layer_data->device_dispatch_table.CmdPushDescriptorSetKHR(commandBuffer, pipelineBindPoint, layout, set, descriptorWriteCount, (const VkWriteDescriptorSet*)local_pDescriptorWrites);
    if (local_pDescriptorWrites) {
        delete[] local_pDescriptorWrites;
    }
}

// Skip vkCmdPushDescriptorSetWithTemplateKHR dispatch, manually generated

// Skip vkCreateDescriptorUpdateTemplateKHR dispatch, manually generated

// Skip vkDestroyDescriptorUpdateTemplateKHR dispatch, manually generated

// Skip vkUpdateDescriptorSetWithTemplateKHR dispatch, manually generated

// Skip vkCreateRenderPass2KHR dispatch, manually generated

void DispatchCmdBeginRenderPass2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkRenderPassBeginInfo*                pRenderPassBegin,
    const VkSubpassBeginInfoKHR*                pSubpassBeginInfo)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CmdBeginRenderPass2KHR(commandBuffer, pRenderPassBegin, pSubpassBeginInfo);
    safe_VkRenderPassBeginInfo *local_pRenderPassBegin = NULL;
    {
        if (pRenderPassBegin) {
            local_pRenderPassBegin = new safe_VkRenderPassBeginInfo(pRenderPassBegin);
            if (pRenderPassBegin->renderPass) {
                local_pRenderPassBegin->renderPass = layer_data->Unwrap(pRenderPassBegin->renderPass);
            }
            if (pRenderPassBegin->framebuffer) {
                local_pRenderPassBegin->framebuffer = layer_data->Unwrap(pRenderPassBegin->framebuffer);
            }
            WrapPnextChainHandles(layer_data, local_pRenderPassBegin->pNext);
        }
    }
    layer_data->device_dispatch_table.CmdBeginRenderPass2KHR(commandBuffer, (const VkRenderPassBeginInfo*)local_pRenderPassBegin, pSubpassBeginInfo);
    if (local_pRenderPassBegin) {
        delete local_pRenderPassBegin;
    }
}

void DispatchCmdNextSubpass2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkSubpassBeginInfoKHR*                pSubpassBeginInfo,
    const VkSubpassEndInfoKHR*                  pSubpassEndInfo)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    layer_data->device_dispatch_table.CmdNextSubpass2KHR(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo);

}

void DispatchCmdEndRenderPass2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkSubpassEndInfoKHR*                  pSubpassEndInfo)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    layer_data->device_dispatch_table.CmdEndRenderPass2KHR(commandBuffer, pSubpassEndInfo);

}

VkResult DispatchGetSwapchainStatusKHR(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.GetSwapchainStatusKHR(device, swapchain);
    {
        swapchain = layer_data->Unwrap(swapchain);
    }
    VkResult result = layer_data->device_dispatch_table.GetSwapchainStatusKHR(device, swapchain);

    return result;
}

void DispatchGetPhysicalDeviceExternalFencePropertiesKHR(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceExternalFenceInfo*    pExternalFenceInfo,
    VkExternalFenceProperties*                  pExternalFenceProperties)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    layer_data->instance_dispatch_table.GetPhysicalDeviceExternalFencePropertiesKHR(physicalDevice, pExternalFenceInfo, pExternalFenceProperties);

}

#ifdef VK_USE_PLATFORM_WIN32_KHR

VkResult DispatchImportFenceWin32HandleKHR(
    VkDevice                                    device,
    const VkImportFenceWin32HandleInfoKHR*      pImportFenceWin32HandleInfo)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.ImportFenceWin32HandleKHR(device, pImportFenceWin32HandleInfo);
    safe_VkImportFenceWin32HandleInfoKHR *local_pImportFenceWin32HandleInfo = NULL;
    {
        if (pImportFenceWin32HandleInfo) {
            local_pImportFenceWin32HandleInfo = new safe_VkImportFenceWin32HandleInfoKHR(pImportFenceWin32HandleInfo);
            if (pImportFenceWin32HandleInfo->fence) {
                local_pImportFenceWin32HandleInfo->fence = layer_data->Unwrap(pImportFenceWin32HandleInfo->fence);
            }
        }
    }
    VkResult result = layer_data->device_dispatch_table.ImportFenceWin32HandleKHR(device, (const VkImportFenceWin32HandleInfoKHR*)local_pImportFenceWin32HandleInfo);
    if (local_pImportFenceWin32HandleInfo) {
        delete local_pImportFenceWin32HandleInfo;
    }
    return result;
}
#endif // VK_USE_PLATFORM_WIN32_KHR

#ifdef VK_USE_PLATFORM_WIN32_KHR

VkResult DispatchGetFenceWin32HandleKHR(
    VkDevice                                    device,
    const VkFenceGetWin32HandleInfoKHR*         pGetWin32HandleInfo,
    HANDLE*                                     pHandle)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.GetFenceWin32HandleKHR(device, pGetWin32HandleInfo, pHandle);
    safe_VkFenceGetWin32HandleInfoKHR *local_pGetWin32HandleInfo = NULL;
    {
        if (pGetWin32HandleInfo) {
            local_pGetWin32HandleInfo = new safe_VkFenceGetWin32HandleInfoKHR(pGetWin32HandleInfo);
            if (pGetWin32HandleInfo->fence) {
                local_pGetWin32HandleInfo->fence = layer_data->Unwrap(pGetWin32HandleInfo->fence);
            }
        }
    }
    VkResult result = layer_data->device_dispatch_table.GetFenceWin32HandleKHR(device, (const VkFenceGetWin32HandleInfoKHR*)local_pGetWin32HandleInfo, pHandle);
    if (local_pGetWin32HandleInfo) {
        delete local_pGetWin32HandleInfo;
    }
    return result;
}
#endif // VK_USE_PLATFORM_WIN32_KHR

VkResult DispatchImportFenceFdKHR(
    VkDevice                                    device,
    const VkImportFenceFdInfoKHR*               pImportFenceFdInfo)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.ImportFenceFdKHR(device, pImportFenceFdInfo);
    safe_VkImportFenceFdInfoKHR *local_pImportFenceFdInfo = NULL;
    {
        if (pImportFenceFdInfo) {
            local_pImportFenceFdInfo = new safe_VkImportFenceFdInfoKHR(pImportFenceFdInfo);
            if (pImportFenceFdInfo->fence) {
                local_pImportFenceFdInfo->fence = layer_data->Unwrap(pImportFenceFdInfo->fence);
            }
        }
    }
    VkResult result = layer_data->device_dispatch_table.ImportFenceFdKHR(device, (const VkImportFenceFdInfoKHR*)local_pImportFenceFdInfo);
    if (local_pImportFenceFdInfo) {
        delete local_pImportFenceFdInfo;
    }
    return result;
}

VkResult DispatchGetFenceFdKHR(
    VkDevice                                    device,
    const VkFenceGetFdInfoKHR*                  pGetFdInfo,
    int*                                        pFd)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.GetFenceFdKHR(device, pGetFdInfo, pFd);
    safe_VkFenceGetFdInfoKHR *local_pGetFdInfo = NULL;
    {
        if (pGetFdInfo) {
            local_pGetFdInfo = new safe_VkFenceGetFdInfoKHR(pGetFdInfo);
            if (pGetFdInfo->fence) {
                local_pGetFdInfo->fence = layer_data->Unwrap(pGetFdInfo->fence);
            }
        }
    }
    VkResult result = layer_data->device_dispatch_table.GetFenceFdKHR(device, (const VkFenceGetFdInfoKHR*)local_pGetFdInfo, pFd);
    if (local_pGetFdInfo) {
        delete local_pGetFdInfo;
    }
    return result;
}

VkResult DispatchGetPhysicalDeviceSurfaceCapabilities2KHR(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceSurfaceInfo2KHR*      pSurfaceInfo,
    VkSurfaceCapabilities2KHR*                  pSurfaceCapabilities)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    if (!wrap_handles) return layer_data->instance_dispatch_table.GetPhysicalDeviceSurfaceCapabilities2KHR(physicalDevice, pSurfaceInfo, pSurfaceCapabilities);
    safe_VkPhysicalDeviceSurfaceInfo2KHR *local_pSurfaceInfo = NULL;
    {
        if (pSurfaceInfo) {
            local_pSurfaceInfo = new safe_VkPhysicalDeviceSurfaceInfo2KHR(pSurfaceInfo);
            if (pSurfaceInfo->surface) {
                local_pSurfaceInfo->surface = layer_data->Unwrap(pSurfaceInfo->surface);
            }
        }
    }
    VkResult result = layer_data->instance_dispatch_table.GetPhysicalDeviceSurfaceCapabilities2KHR(physicalDevice, (const VkPhysicalDeviceSurfaceInfo2KHR*)local_pSurfaceInfo, pSurfaceCapabilities);
    if (local_pSurfaceInfo) {
        delete local_pSurfaceInfo;
    }
    return result;
}

VkResult DispatchGetPhysicalDeviceSurfaceFormats2KHR(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceSurfaceInfo2KHR*      pSurfaceInfo,
    uint32_t*                                   pSurfaceFormatCount,
    VkSurfaceFormat2KHR*                        pSurfaceFormats)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    if (!wrap_handles) return layer_data->instance_dispatch_table.GetPhysicalDeviceSurfaceFormats2KHR(physicalDevice, pSurfaceInfo, pSurfaceFormatCount, pSurfaceFormats);
    safe_VkPhysicalDeviceSurfaceInfo2KHR *local_pSurfaceInfo = NULL;
    {
        if (pSurfaceInfo) {
            local_pSurfaceInfo = new safe_VkPhysicalDeviceSurfaceInfo2KHR(pSurfaceInfo);
            if (pSurfaceInfo->surface) {
                local_pSurfaceInfo->surface = layer_data->Unwrap(pSurfaceInfo->surface);
            }
        }
    }
    VkResult result = layer_data->instance_dispatch_table.GetPhysicalDeviceSurfaceFormats2KHR(physicalDevice, (const VkPhysicalDeviceSurfaceInfo2KHR*)local_pSurfaceInfo, pSurfaceFormatCount, pSurfaceFormats);
    if (local_pSurfaceInfo) {
        delete local_pSurfaceInfo;
    }
    return result;
}

// Skip vkGetPhysicalDeviceDisplayProperties2KHR dispatch, manually generated

// Skip vkGetPhysicalDeviceDisplayPlaneProperties2KHR dispatch, manually generated

// Skip vkGetDisplayModeProperties2KHR dispatch, manually generated

VkResult DispatchGetDisplayPlaneCapabilities2KHR(
    VkPhysicalDevice                            physicalDevice,
    const VkDisplayPlaneInfo2KHR*               pDisplayPlaneInfo,
    VkDisplayPlaneCapabilities2KHR*             pCapabilities)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    if (!wrap_handles) return layer_data->instance_dispatch_table.GetDisplayPlaneCapabilities2KHR(physicalDevice, pDisplayPlaneInfo, pCapabilities);
    safe_VkDisplayPlaneInfo2KHR *local_pDisplayPlaneInfo = NULL;
    {
        if (pDisplayPlaneInfo) {
            local_pDisplayPlaneInfo = new safe_VkDisplayPlaneInfo2KHR(pDisplayPlaneInfo);
            if (pDisplayPlaneInfo->mode) {
                local_pDisplayPlaneInfo->mode = layer_data->Unwrap(pDisplayPlaneInfo->mode);
            }
        }
    }
    VkResult result = layer_data->instance_dispatch_table.GetDisplayPlaneCapabilities2KHR(physicalDevice, (const VkDisplayPlaneInfo2KHR*)local_pDisplayPlaneInfo, pCapabilities);
    if (local_pDisplayPlaneInfo) {
        delete local_pDisplayPlaneInfo;
    }
    return result;
}

void DispatchGetImageMemoryRequirements2KHR(
    VkDevice                                    device,
    const VkImageMemoryRequirementsInfo2*       pInfo,
    VkMemoryRequirements2*                      pMemoryRequirements)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.GetImageMemoryRequirements2KHR(device, pInfo, pMemoryRequirements);
    safe_VkImageMemoryRequirementsInfo2 *local_pInfo = NULL;
    {
        if (pInfo) {
            local_pInfo = new safe_VkImageMemoryRequirementsInfo2(pInfo);
            if (pInfo->image) {
                local_pInfo->image = layer_data->Unwrap(pInfo->image);
            }
        }
    }
    layer_data->device_dispatch_table.GetImageMemoryRequirements2KHR(device, (const VkImageMemoryRequirementsInfo2*)local_pInfo, pMemoryRequirements);
    if (local_pInfo) {
        delete local_pInfo;
    }
}

void DispatchGetBufferMemoryRequirements2KHR(
    VkDevice                                    device,
    const VkBufferMemoryRequirementsInfo2*      pInfo,
    VkMemoryRequirements2*                      pMemoryRequirements)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.GetBufferMemoryRequirements2KHR(device, pInfo, pMemoryRequirements);
    safe_VkBufferMemoryRequirementsInfo2 *local_pInfo = NULL;
    {
        if (pInfo) {
            local_pInfo = new safe_VkBufferMemoryRequirementsInfo2(pInfo);
            if (pInfo->buffer) {
                local_pInfo->buffer = layer_data->Unwrap(pInfo->buffer);
            }
        }
    }
    layer_data->device_dispatch_table.GetBufferMemoryRequirements2KHR(device, (const VkBufferMemoryRequirementsInfo2*)local_pInfo, pMemoryRequirements);
    if (local_pInfo) {
        delete local_pInfo;
    }
}

void DispatchGetImageSparseMemoryRequirements2KHR(
    VkDevice                                    device,
    const VkImageSparseMemoryRequirementsInfo2* pInfo,
    uint32_t*                                   pSparseMemoryRequirementCount,
    VkSparseImageMemoryRequirements2*           pSparseMemoryRequirements)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.GetImageSparseMemoryRequirements2KHR(device, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
    safe_VkImageSparseMemoryRequirementsInfo2 *local_pInfo = NULL;
    {
        if (pInfo) {
            local_pInfo = new safe_VkImageSparseMemoryRequirementsInfo2(pInfo);
            if (pInfo->image) {
                local_pInfo->image = layer_data->Unwrap(pInfo->image);
            }
        }
    }
    layer_data->device_dispatch_table.GetImageSparseMemoryRequirements2KHR(device, (const VkImageSparseMemoryRequirementsInfo2*)local_pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
    if (local_pInfo) {
        delete local_pInfo;
    }
}

VkResult DispatchCreateSamplerYcbcrConversionKHR(
    VkDevice                                    device,
    const VkSamplerYcbcrConversionCreateInfo*   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSamplerYcbcrConversion*                   pYcbcrConversion)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CreateSamplerYcbcrConversionKHR(device, pCreateInfo, pAllocator, pYcbcrConversion);
    safe_VkSamplerYcbcrConversionCreateInfo *local_pCreateInfo = NULL;
    {
        if (pCreateInfo) {
            local_pCreateInfo = new safe_VkSamplerYcbcrConversionCreateInfo(pCreateInfo);
            WrapPnextChainHandles(layer_data, local_pCreateInfo->pNext);
        }
    }
    VkResult result = layer_data->device_dispatch_table.CreateSamplerYcbcrConversionKHR(device, (const VkSamplerYcbcrConversionCreateInfo*)local_pCreateInfo, pAllocator, pYcbcrConversion);
    if (local_pCreateInfo) {
        delete local_pCreateInfo;
    }
    if (VK_SUCCESS == result) {
        *pYcbcrConversion = layer_data->WrapNew(*pYcbcrConversion);
    }
    return result;
}

void DispatchDestroySamplerYcbcrConversionKHR(
    VkDevice                                    device,
    VkSamplerYcbcrConversion                    ycbcrConversion,
    const VkAllocationCallbacks*                pAllocator)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.DestroySamplerYcbcrConversionKHR(device, ycbcrConversion, pAllocator);
    uint64_t ycbcrConversion_id = reinterpret_cast<uint64_t &>(ycbcrConversion);
    auto iter = unique_id_mapping.pop(ycbcrConversion_id);
    if (iter != unique_id_mapping.end()) {
        ycbcrConversion = (VkSamplerYcbcrConversion)iter->second;
    } else {
        ycbcrConversion = (VkSamplerYcbcrConversion)0;
    }
    layer_data->device_dispatch_table.DestroySamplerYcbcrConversionKHR(device, ycbcrConversion, pAllocator);

}

VkResult DispatchBindBufferMemory2KHR(
    VkDevice                                    device,
    uint32_t                                    bindInfoCount,
    const VkBindBufferMemoryInfo*               pBindInfos)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.BindBufferMemory2KHR(device, bindInfoCount, pBindInfos);
    safe_VkBindBufferMemoryInfo *local_pBindInfos = NULL;
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
    VkResult result = layer_data->device_dispatch_table.BindBufferMemory2KHR(device, bindInfoCount, (const VkBindBufferMemoryInfo*)local_pBindInfos);
    if (local_pBindInfos) {
        delete[] local_pBindInfos;
    }
    return result;
}

VkResult DispatchBindImageMemory2KHR(
    VkDevice                                    device,
    uint32_t                                    bindInfoCount,
    const VkBindImageMemoryInfo*                pBindInfos)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.BindImageMemory2KHR(device, bindInfoCount, pBindInfos);
    safe_VkBindImageMemoryInfo *local_pBindInfos = NULL;
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
    VkResult result = layer_data->device_dispatch_table.BindImageMemory2KHR(device, bindInfoCount, (const VkBindImageMemoryInfo*)local_pBindInfos);
    if (local_pBindInfos) {
        delete[] local_pBindInfos;
    }
    return result;
}

void DispatchGetDescriptorSetLayoutSupportKHR(
    VkDevice                                    device,
    const VkDescriptorSetLayoutCreateInfo*      pCreateInfo,
    VkDescriptorSetLayoutSupport*               pSupport)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.GetDescriptorSetLayoutSupportKHR(device, pCreateInfo, pSupport);
    safe_VkDescriptorSetLayoutCreateInfo *local_pCreateInfo = NULL;
    {
        if (pCreateInfo) {
            local_pCreateInfo = new safe_VkDescriptorSetLayoutCreateInfo(pCreateInfo);
            if (local_pCreateInfo->pBindings) {
                for (uint32_t index1 = 0; index1 < local_pCreateInfo->bindingCount; ++index1) {
                    if (local_pCreateInfo->pBindings[index1].pImmutableSamplers) {
                        for (uint32_t index2 = 0; index2 < local_pCreateInfo->pBindings[index1].descriptorCount; ++index2) {
                            local_pCreateInfo->pBindings[index1].pImmutableSamplers[index2] = layer_data->Unwrap(local_pCreateInfo->pBindings[index1].pImmutableSamplers[index2]);
                        }
                    }
                }
            }
        }
    }
    layer_data->device_dispatch_table.GetDescriptorSetLayoutSupportKHR(device, (const VkDescriptorSetLayoutCreateInfo*)local_pCreateInfo, pSupport);
    if (local_pCreateInfo) {
        delete local_pCreateInfo;
    }
}

void DispatchCmdDrawIndirectCountKHR(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkBuffer                                    countBuffer,
    VkDeviceSize                                countBufferOffset,
    uint32_t                                    maxDrawCount,
    uint32_t                                    stride)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CmdDrawIndirectCountKHR(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    {
        buffer = layer_data->Unwrap(buffer);
        countBuffer = layer_data->Unwrap(countBuffer);
    }
    layer_data->device_dispatch_table.CmdDrawIndirectCountKHR(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);

}

void DispatchCmdDrawIndexedIndirectCountKHR(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkBuffer                                    countBuffer,
    VkDeviceSize                                countBufferOffset,
    uint32_t                                    maxDrawCount,
    uint32_t                                    stride)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CmdDrawIndexedIndirectCountKHR(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    {
        buffer = layer_data->Unwrap(buffer);
        countBuffer = layer_data->Unwrap(countBuffer);
    }
    layer_data->device_dispatch_table.CmdDrawIndexedIndirectCountKHR(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);

}

VkResult DispatchGetPipelineExecutablePropertiesKHR(
    VkDevice                                    device,
    const VkPipelineInfoKHR*                    pPipelineInfo,
    uint32_t*                                   pExecutableCount,
    VkPipelineExecutablePropertiesKHR*          pProperties)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.GetPipelineExecutablePropertiesKHR(device, pPipelineInfo, pExecutableCount, pProperties);
    safe_VkPipelineInfoKHR *local_pPipelineInfo = NULL;
    {
        if (pPipelineInfo) {
            local_pPipelineInfo = new safe_VkPipelineInfoKHR(pPipelineInfo);
            if (pPipelineInfo->pipeline) {
                local_pPipelineInfo->pipeline = layer_data->Unwrap(pPipelineInfo->pipeline);
            }
        }
    }
    VkResult result = layer_data->device_dispatch_table.GetPipelineExecutablePropertiesKHR(device, (const VkPipelineInfoKHR*)local_pPipelineInfo, pExecutableCount, pProperties);
    if (local_pPipelineInfo) {
        delete local_pPipelineInfo;
    }
    return result;
}

VkResult DispatchGetPipelineExecutableStatisticsKHR(
    VkDevice                                    device,
    const VkPipelineExecutableInfoKHR*          pExecutableInfo,
    uint32_t*                                   pStatisticCount,
    VkPipelineExecutableStatisticKHR*           pStatistics)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.GetPipelineExecutableStatisticsKHR(device, pExecutableInfo, pStatisticCount, pStatistics);
    safe_VkPipelineExecutableInfoKHR *local_pExecutableInfo = NULL;
    {
        if (pExecutableInfo) {
            local_pExecutableInfo = new safe_VkPipelineExecutableInfoKHR(pExecutableInfo);
            if (pExecutableInfo->pipeline) {
                local_pExecutableInfo->pipeline = layer_data->Unwrap(pExecutableInfo->pipeline);
            }
        }
    }
    VkResult result = layer_data->device_dispatch_table.GetPipelineExecutableStatisticsKHR(device, (const VkPipelineExecutableInfoKHR*)local_pExecutableInfo, pStatisticCount, pStatistics);
    if (local_pExecutableInfo) {
        delete local_pExecutableInfo;
    }
    return result;
}

VkResult DispatchGetPipelineExecutableInternalRepresentationsKHR(
    VkDevice                                    device,
    const VkPipelineExecutableInfoKHR*          pExecutableInfo,
    uint32_t*                                   pInternalRepresentationCount,
    VkPipelineExecutableInternalRepresentationKHR* pInternalRepresentations)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.GetPipelineExecutableInternalRepresentationsKHR(device, pExecutableInfo, pInternalRepresentationCount, pInternalRepresentations);
    safe_VkPipelineExecutableInfoKHR *local_pExecutableInfo = NULL;
    {
        if (pExecutableInfo) {
            local_pExecutableInfo = new safe_VkPipelineExecutableInfoKHR(pExecutableInfo);
            if (pExecutableInfo->pipeline) {
                local_pExecutableInfo->pipeline = layer_data->Unwrap(pExecutableInfo->pipeline);
            }
        }
    }
    VkResult result = layer_data->device_dispatch_table.GetPipelineExecutableInternalRepresentationsKHR(device, (const VkPipelineExecutableInfoKHR*)local_pExecutableInfo, pInternalRepresentationCount, pInternalRepresentations);
    if (local_pExecutableInfo) {
        delete local_pExecutableInfo;
    }
    return result;
}

VkResult DispatchCreateDebugReportCallbackEXT(
    VkInstance                                  instance,
    const VkDebugReportCallbackCreateInfoEXT*   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDebugReportCallbackEXT*                   pCallback)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(instance), layer_data_map);
    if (!wrap_handles) return layer_data->instance_dispatch_table.CreateDebugReportCallbackEXT(instance, pCreateInfo, pAllocator, pCallback);
    VkResult result = layer_data->instance_dispatch_table.CreateDebugReportCallbackEXT(instance, pCreateInfo, pAllocator, pCallback);
    if (VK_SUCCESS == result) {
        *pCallback = layer_data->WrapNew(*pCallback);
    }
    return result;
}

void DispatchDestroyDebugReportCallbackEXT(
    VkInstance                                  instance,
    VkDebugReportCallbackEXT                    callback,
    const VkAllocationCallbacks*                pAllocator)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(instance), layer_data_map);
    if (!wrap_handles) return layer_data->instance_dispatch_table.DestroyDebugReportCallbackEXT(instance, callback, pAllocator);
    uint64_t callback_id = reinterpret_cast<uint64_t &>(callback);
    auto iter = unique_id_mapping.pop(callback_id);
    if (iter != unique_id_mapping.end()) {
        callback = (VkDebugReportCallbackEXT)iter->second;
    } else {
        callback = (VkDebugReportCallbackEXT)0;
    }
    layer_data->instance_dispatch_table.DestroyDebugReportCallbackEXT(instance, callback, pAllocator);

}

void DispatchDebugReportMessageEXT(
    VkInstance                                  instance,
    VkDebugReportFlagsEXT                       flags,
    VkDebugReportObjectTypeEXT                  objectType,
    uint64_t                                    object,
    size_t                                      location,
    int32_t                                     messageCode,
    const char*                                 pLayerPrefix,
    const char*                                 pMessage)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(instance), layer_data_map);
    layer_data->instance_dispatch_table.DebugReportMessageEXT(instance, flags, objectType, object, location, messageCode, pLayerPrefix, pMessage);

}

// Skip vkDebugMarkerSetObjectTagEXT dispatch, manually generated

// Skip vkDebugMarkerSetObjectNameEXT dispatch, manually generated

void DispatchCmdDebugMarkerBeginEXT(
    VkCommandBuffer                             commandBuffer,
    const VkDebugMarkerMarkerInfoEXT*           pMarkerInfo)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    layer_data->device_dispatch_table.CmdDebugMarkerBeginEXT(commandBuffer, pMarkerInfo);

}

void DispatchCmdDebugMarkerEndEXT(
    VkCommandBuffer                             commandBuffer)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    layer_data->device_dispatch_table.CmdDebugMarkerEndEXT(commandBuffer);

}

void DispatchCmdDebugMarkerInsertEXT(
    VkCommandBuffer                             commandBuffer,
    const VkDebugMarkerMarkerInfoEXT*           pMarkerInfo)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    layer_data->device_dispatch_table.CmdDebugMarkerInsertEXT(commandBuffer, pMarkerInfo);

}

void DispatchCmdBindTransformFeedbackBuffersEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstBinding,
    uint32_t                                    bindingCount,
    const VkBuffer*                             pBuffers,
    const VkDeviceSize*                         pOffsets,
    const VkDeviceSize*                         pSizes)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CmdBindTransformFeedbackBuffersEXT(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets, pSizes);
    VkBuffer *local_pBuffers = NULL;
    {
        if (pBuffers) {
            local_pBuffers = new VkBuffer[bindingCount];
            for (uint32_t index0 = 0; index0 < bindingCount; ++index0) {
                local_pBuffers[index0] = layer_data->Unwrap(pBuffers[index0]);
            }
        }
    }
    layer_data->device_dispatch_table.CmdBindTransformFeedbackBuffersEXT(commandBuffer, firstBinding, bindingCount, (const VkBuffer*)local_pBuffers, pOffsets, pSizes);
    if (local_pBuffers)
        delete[] local_pBuffers;
}

void DispatchCmdBeginTransformFeedbackEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstCounterBuffer,
    uint32_t                                    counterBufferCount,
    const VkBuffer*                             pCounterBuffers,
    const VkDeviceSize*                         pCounterBufferOffsets)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CmdBeginTransformFeedbackEXT(commandBuffer, firstCounterBuffer, counterBufferCount, pCounterBuffers, pCounterBufferOffsets);
    VkBuffer *local_pCounterBuffers = NULL;
    {
        if (pCounterBuffers) {
            local_pCounterBuffers = new VkBuffer[counterBufferCount];
            for (uint32_t index0 = 0; index0 < counterBufferCount; ++index0) {
                local_pCounterBuffers[index0] = layer_data->Unwrap(pCounterBuffers[index0]);
            }
        }
    }
    layer_data->device_dispatch_table.CmdBeginTransformFeedbackEXT(commandBuffer, firstCounterBuffer, counterBufferCount, (const VkBuffer*)local_pCounterBuffers, pCounterBufferOffsets);
    if (local_pCounterBuffers)
        delete[] local_pCounterBuffers;
}

void DispatchCmdEndTransformFeedbackEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstCounterBuffer,
    uint32_t                                    counterBufferCount,
    const VkBuffer*                             pCounterBuffers,
    const VkDeviceSize*                         pCounterBufferOffsets)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CmdEndTransformFeedbackEXT(commandBuffer, firstCounterBuffer, counterBufferCount, pCounterBuffers, pCounterBufferOffsets);
    VkBuffer *local_pCounterBuffers = NULL;
    {
        if (pCounterBuffers) {
            local_pCounterBuffers = new VkBuffer[counterBufferCount];
            for (uint32_t index0 = 0; index0 < counterBufferCount; ++index0) {
                local_pCounterBuffers[index0] = layer_data->Unwrap(pCounterBuffers[index0]);
            }
        }
    }
    layer_data->device_dispatch_table.CmdEndTransformFeedbackEXT(commandBuffer, firstCounterBuffer, counterBufferCount, (const VkBuffer*)local_pCounterBuffers, pCounterBufferOffsets);
    if (local_pCounterBuffers)
        delete[] local_pCounterBuffers;
}

void DispatchCmdBeginQueryIndexedEXT(
    VkCommandBuffer                             commandBuffer,
    VkQueryPool                                 queryPool,
    uint32_t                                    query,
    VkQueryControlFlags                         flags,
    uint32_t                                    index)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CmdBeginQueryIndexedEXT(commandBuffer, queryPool, query, flags, index);
    {
        queryPool = layer_data->Unwrap(queryPool);
    }
    layer_data->device_dispatch_table.CmdBeginQueryIndexedEXT(commandBuffer, queryPool, query, flags, index);

}

void DispatchCmdEndQueryIndexedEXT(
    VkCommandBuffer                             commandBuffer,
    VkQueryPool                                 queryPool,
    uint32_t                                    query,
    uint32_t                                    index)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CmdEndQueryIndexedEXT(commandBuffer, queryPool, query, index);
    {
        queryPool = layer_data->Unwrap(queryPool);
    }
    layer_data->device_dispatch_table.CmdEndQueryIndexedEXT(commandBuffer, queryPool, query, index);

}

void DispatchCmdDrawIndirectByteCountEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    instanceCount,
    uint32_t                                    firstInstance,
    VkBuffer                                    counterBuffer,
    VkDeviceSize                                counterBufferOffset,
    uint32_t                                    counterOffset,
    uint32_t                                    vertexStride)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CmdDrawIndirectByteCountEXT(commandBuffer, instanceCount, firstInstance, counterBuffer, counterBufferOffset, counterOffset, vertexStride);
    {
        counterBuffer = layer_data->Unwrap(counterBuffer);
    }
    layer_data->device_dispatch_table.CmdDrawIndirectByteCountEXT(commandBuffer, instanceCount, firstInstance, counterBuffer, counterBufferOffset, counterOffset, vertexStride);

}

uint32_t DispatchGetImageViewHandleNVX(
    VkDevice                                    device,
    const VkImageViewHandleInfoNVX*             pInfo)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.GetImageViewHandleNVX(device, pInfo);
    safe_VkImageViewHandleInfoNVX *local_pInfo = NULL;
    {
        if (pInfo) {
            local_pInfo = new safe_VkImageViewHandleInfoNVX(pInfo);
            if (pInfo->imageView) {
                local_pInfo->imageView = layer_data->Unwrap(pInfo->imageView);
            }
            if (pInfo->sampler) {
                local_pInfo->sampler = layer_data->Unwrap(pInfo->sampler);
            }
        }
    }
    uint32_t result = layer_data->device_dispatch_table.GetImageViewHandleNVX(device, (const VkImageViewHandleInfoNVX*)local_pInfo);
    if (local_pInfo) {
        delete local_pInfo;
    }
    return result;
}

void DispatchCmdDrawIndirectCountAMD(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkBuffer                                    countBuffer,
    VkDeviceSize                                countBufferOffset,
    uint32_t                                    maxDrawCount,
    uint32_t                                    stride)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CmdDrawIndirectCountAMD(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    {
        buffer = layer_data->Unwrap(buffer);
        countBuffer = layer_data->Unwrap(countBuffer);
    }
    layer_data->device_dispatch_table.CmdDrawIndirectCountAMD(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);

}

void DispatchCmdDrawIndexedIndirectCountAMD(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkBuffer                                    countBuffer,
    VkDeviceSize                                countBufferOffset,
    uint32_t                                    maxDrawCount,
    uint32_t                                    stride)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CmdDrawIndexedIndirectCountAMD(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    {
        buffer = layer_data->Unwrap(buffer);
        countBuffer = layer_data->Unwrap(countBuffer);
    }
    layer_data->device_dispatch_table.CmdDrawIndexedIndirectCountAMD(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);

}

VkResult DispatchGetShaderInfoAMD(
    VkDevice                                    device,
    VkPipeline                                  pipeline,
    VkShaderStageFlagBits                       shaderStage,
    VkShaderInfoTypeAMD                         infoType,
    size_t*                                     pInfoSize,
    void*                                       pInfo)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.GetShaderInfoAMD(device, pipeline, shaderStage, infoType, pInfoSize, pInfo);
    {
        pipeline = layer_data->Unwrap(pipeline);
    }
    VkResult result = layer_data->device_dispatch_table.GetShaderInfoAMD(device, pipeline, shaderStage, infoType, pInfoSize, pInfo);

    return result;
}

#ifdef VK_USE_PLATFORM_GGP

VkResult DispatchCreateStreamDescriptorSurfaceGGP(
    VkInstance                                  instance,
    const VkStreamDescriptorSurfaceCreateInfoGGP* pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(instance), layer_data_map);
    if (!wrap_handles) return layer_data->instance_dispatch_table.CreateStreamDescriptorSurfaceGGP(instance, pCreateInfo, pAllocator, pSurface);
    VkResult result = layer_data->instance_dispatch_table.CreateStreamDescriptorSurfaceGGP(instance, pCreateInfo, pAllocator, pSurface);
    if (VK_SUCCESS == result) {
        *pSurface = layer_data->WrapNew(*pSurface);
    }
    return result;
}
#endif // VK_USE_PLATFORM_GGP

VkResult DispatchGetPhysicalDeviceExternalImageFormatPropertiesNV(
    VkPhysicalDevice                            physicalDevice,
    VkFormat                                    format,
    VkImageType                                 type,
    VkImageTiling                               tiling,
    VkImageUsageFlags                           usage,
    VkImageCreateFlags                          flags,
    VkExternalMemoryHandleTypeFlagsNV           externalHandleType,
    VkExternalImageFormatPropertiesNV*          pExternalImageFormatProperties)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    VkResult result = layer_data->instance_dispatch_table.GetPhysicalDeviceExternalImageFormatPropertiesNV(physicalDevice, format, type, tiling, usage, flags, externalHandleType, pExternalImageFormatProperties);

    return result;
}

#ifdef VK_USE_PLATFORM_WIN32_KHR

VkResult DispatchGetMemoryWin32HandleNV(
    VkDevice                                    device,
    VkDeviceMemory                              memory,
    VkExternalMemoryHandleTypeFlagsNV           handleType,
    HANDLE*                                     pHandle)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.GetMemoryWin32HandleNV(device, memory, handleType, pHandle);
    {
        memory = layer_data->Unwrap(memory);
    }
    VkResult result = layer_data->device_dispatch_table.GetMemoryWin32HandleNV(device, memory, handleType, pHandle);

    return result;
}
#endif // VK_USE_PLATFORM_WIN32_KHR

#ifdef VK_USE_PLATFORM_VI_NN

VkResult DispatchCreateViSurfaceNN(
    VkInstance                                  instance,
    const VkViSurfaceCreateInfoNN*              pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(instance), layer_data_map);
    if (!wrap_handles) return layer_data->instance_dispatch_table.CreateViSurfaceNN(instance, pCreateInfo, pAllocator, pSurface);
    VkResult result = layer_data->instance_dispatch_table.CreateViSurfaceNN(instance, pCreateInfo, pAllocator, pSurface);
    if (VK_SUCCESS == result) {
        *pSurface = layer_data->WrapNew(*pSurface);
    }
    return result;
}
#endif // VK_USE_PLATFORM_VI_NN

void DispatchCmdBeginConditionalRenderingEXT(
    VkCommandBuffer                             commandBuffer,
    const VkConditionalRenderingBeginInfoEXT*   pConditionalRenderingBegin)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CmdBeginConditionalRenderingEXT(commandBuffer, pConditionalRenderingBegin);
    safe_VkConditionalRenderingBeginInfoEXT *local_pConditionalRenderingBegin = NULL;
    {
        if (pConditionalRenderingBegin) {
            local_pConditionalRenderingBegin = new safe_VkConditionalRenderingBeginInfoEXT(pConditionalRenderingBegin);
            if (pConditionalRenderingBegin->buffer) {
                local_pConditionalRenderingBegin->buffer = layer_data->Unwrap(pConditionalRenderingBegin->buffer);
            }
        }
    }
    layer_data->device_dispatch_table.CmdBeginConditionalRenderingEXT(commandBuffer, (const VkConditionalRenderingBeginInfoEXT*)local_pConditionalRenderingBegin);
    if (local_pConditionalRenderingBegin) {
        delete local_pConditionalRenderingBegin;
    }
}

void DispatchCmdEndConditionalRenderingEXT(
    VkCommandBuffer                             commandBuffer)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    layer_data->device_dispatch_table.CmdEndConditionalRenderingEXT(commandBuffer);

}

void DispatchCmdProcessCommandsNVX(
    VkCommandBuffer                             commandBuffer,
    const VkCmdProcessCommandsInfoNVX*          pProcessCommandsInfo)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CmdProcessCommandsNVX(commandBuffer, pProcessCommandsInfo);
    safe_VkCmdProcessCommandsInfoNVX *local_pProcessCommandsInfo = NULL;
    {
        if (pProcessCommandsInfo) {
            local_pProcessCommandsInfo = new safe_VkCmdProcessCommandsInfoNVX(pProcessCommandsInfo);
            if (pProcessCommandsInfo->objectTable) {
                local_pProcessCommandsInfo->objectTable = layer_data->Unwrap(pProcessCommandsInfo->objectTable);
            }
            if (pProcessCommandsInfo->indirectCommandsLayout) {
                local_pProcessCommandsInfo->indirectCommandsLayout = layer_data->Unwrap(pProcessCommandsInfo->indirectCommandsLayout);
            }
            if (local_pProcessCommandsInfo->pIndirectCommandsTokens) {
                for (uint32_t index1 = 0; index1 < local_pProcessCommandsInfo->indirectCommandsTokenCount; ++index1) {
                    if (pProcessCommandsInfo->pIndirectCommandsTokens[index1].buffer) {
                        local_pProcessCommandsInfo->pIndirectCommandsTokens[index1].buffer = layer_data->Unwrap(pProcessCommandsInfo->pIndirectCommandsTokens[index1].buffer);
                    }
                }
            }
            if (pProcessCommandsInfo->sequencesCountBuffer) {
                local_pProcessCommandsInfo->sequencesCountBuffer = layer_data->Unwrap(pProcessCommandsInfo->sequencesCountBuffer);
            }
            if (pProcessCommandsInfo->sequencesIndexBuffer) {
                local_pProcessCommandsInfo->sequencesIndexBuffer = layer_data->Unwrap(pProcessCommandsInfo->sequencesIndexBuffer);
            }
        }
    }
    layer_data->device_dispatch_table.CmdProcessCommandsNVX(commandBuffer, (const VkCmdProcessCommandsInfoNVX*)local_pProcessCommandsInfo);
    if (local_pProcessCommandsInfo) {
        delete local_pProcessCommandsInfo;
    }
}

void DispatchCmdReserveSpaceForCommandsNVX(
    VkCommandBuffer                             commandBuffer,
    const VkCmdReserveSpaceForCommandsInfoNVX*  pReserveSpaceInfo)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CmdReserveSpaceForCommandsNVX(commandBuffer, pReserveSpaceInfo);
    safe_VkCmdReserveSpaceForCommandsInfoNVX *local_pReserveSpaceInfo = NULL;
    {
        if (pReserveSpaceInfo) {
            local_pReserveSpaceInfo = new safe_VkCmdReserveSpaceForCommandsInfoNVX(pReserveSpaceInfo);
            if (pReserveSpaceInfo->objectTable) {
                local_pReserveSpaceInfo->objectTable = layer_data->Unwrap(pReserveSpaceInfo->objectTable);
            }
            if (pReserveSpaceInfo->indirectCommandsLayout) {
                local_pReserveSpaceInfo->indirectCommandsLayout = layer_data->Unwrap(pReserveSpaceInfo->indirectCommandsLayout);
            }
        }
    }
    layer_data->device_dispatch_table.CmdReserveSpaceForCommandsNVX(commandBuffer, (const VkCmdReserveSpaceForCommandsInfoNVX*)local_pReserveSpaceInfo);
    if (local_pReserveSpaceInfo) {
        delete local_pReserveSpaceInfo;
    }
}

VkResult DispatchCreateIndirectCommandsLayoutNVX(
    VkDevice                                    device,
    const VkIndirectCommandsLayoutCreateInfoNVX* pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkIndirectCommandsLayoutNVX*                pIndirectCommandsLayout)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CreateIndirectCommandsLayoutNVX(device, pCreateInfo, pAllocator, pIndirectCommandsLayout);
    VkResult result = layer_data->device_dispatch_table.CreateIndirectCommandsLayoutNVX(device, pCreateInfo, pAllocator, pIndirectCommandsLayout);
    if (VK_SUCCESS == result) {
        *pIndirectCommandsLayout = layer_data->WrapNew(*pIndirectCommandsLayout);
    }
    return result;
}

void DispatchDestroyIndirectCommandsLayoutNVX(
    VkDevice                                    device,
    VkIndirectCommandsLayoutNVX                 indirectCommandsLayout,
    const VkAllocationCallbacks*                pAllocator)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.DestroyIndirectCommandsLayoutNVX(device, indirectCommandsLayout, pAllocator);
    uint64_t indirectCommandsLayout_id = reinterpret_cast<uint64_t &>(indirectCommandsLayout);
    auto iter = unique_id_mapping.pop(indirectCommandsLayout_id);
    if (iter != unique_id_mapping.end()) {
        indirectCommandsLayout = (VkIndirectCommandsLayoutNVX)iter->second;
    } else {
        indirectCommandsLayout = (VkIndirectCommandsLayoutNVX)0;
    }
    layer_data->device_dispatch_table.DestroyIndirectCommandsLayoutNVX(device, indirectCommandsLayout, pAllocator);

}

VkResult DispatchCreateObjectTableNVX(
    VkDevice                                    device,
    const VkObjectTableCreateInfoNVX*           pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkObjectTableNVX*                           pObjectTable)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CreateObjectTableNVX(device, pCreateInfo, pAllocator, pObjectTable);
    VkResult result = layer_data->device_dispatch_table.CreateObjectTableNVX(device, pCreateInfo, pAllocator, pObjectTable);
    if (VK_SUCCESS == result) {
        *pObjectTable = layer_data->WrapNew(*pObjectTable);
    }
    return result;
}

void DispatchDestroyObjectTableNVX(
    VkDevice                                    device,
    VkObjectTableNVX                            objectTable,
    const VkAllocationCallbacks*                pAllocator)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.DestroyObjectTableNVX(device, objectTable, pAllocator);
    uint64_t objectTable_id = reinterpret_cast<uint64_t &>(objectTable);
    auto iter = unique_id_mapping.pop(objectTable_id);
    if (iter != unique_id_mapping.end()) {
        objectTable = (VkObjectTableNVX)iter->second;
    } else {
        objectTable = (VkObjectTableNVX)0;
    }
    layer_data->device_dispatch_table.DestroyObjectTableNVX(device, objectTable, pAllocator);

}

VkResult DispatchRegisterObjectsNVX(
    VkDevice                                    device,
    VkObjectTableNVX                            objectTable,
    uint32_t                                    objectCount,
    const VkObjectTableEntryNVX* const*         ppObjectTableEntries,
    const uint32_t*                             pObjectIndices)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.RegisterObjectsNVX(device, objectTable, objectCount, ppObjectTableEntries, pObjectIndices);
    {
        objectTable = layer_data->Unwrap(objectTable);
    }
    VkResult result = layer_data->device_dispatch_table.RegisterObjectsNVX(device, objectTable, objectCount, ppObjectTableEntries, pObjectIndices);

    return result;
}

VkResult DispatchUnregisterObjectsNVX(
    VkDevice                                    device,
    VkObjectTableNVX                            objectTable,
    uint32_t                                    objectCount,
    const VkObjectEntryTypeNVX*                 pObjectEntryTypes,
    const uint32_t*                             pObjectIndices)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.UnregisterObjectsNVX(device, objectTable, objectCount, pObjectEntryTypes, pObjectIndices);
    {
        objectTable = layer_data->Unwrap(objectTable);
    }
    VkResult result = layer_data->device_dispatch_table.UnregisterObjectsNVX(device, objectTable, objectCount, pObjectEntryTypes, pObjectIndices);

    return result;
}

void DispatchGetPhysicalDeviceGeneratedCommandsPropertiesNVX(
    VkPhysicalDevice                            physicalDevice,
    VkDeviceGeneratedCommandsFeaturesNVX*       pFeatures,
    VkDeviceGeneratedCommandsLimitsNVX*         pLimits)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    layer_data->instance_dispatch_table.GetPhysicalDeviceGeneratedCommandsPropertiesNVX(physicalDevice, pFeatures, pLimits);

}

void DispatchCmdSetViewportWScalingNV(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstViewport,
    uint32_t                                    viewportCount,
    const VkViewportWScalingNV*                 pViewportWScalings)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    layer_data->device_dispatch_table.CmdSetViewportWScalingNV(commandBuffer, firstViewport, viewportCount, pViewportWScalings);

}

VkResult DispatchReleaseDisplayEXT(
    VkPhysicalDevice                            physicalDevice,
    VkDisplayKHR                                display)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    if (!wrap_handles) return layer_data->instance_dispatch_table.ReleaseDisplayEXT(physicalDevice, display);
    {
        display = layer_data->Unwrap(display);
    }
    VkResult result = layer_data->instance_dispatch_table.ReleaseDisplayEXT(physicalDevice, display);

    return result;
}

#ifdef VK_USE_PLATFORM_XLIB_XRANDR_EXT

VkResult DispatchAcquireXlibDisplayEXT(
    VkPhysicalDevice                            physicalDevice,
    Display*                                    dpy,
    VkDisplayKHR                                display)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    if (!wrap_handles) return layer_data->instance_dispatch_table.AcquireXlibDisplayEXT(physicalDevice, dpy, display);
    {
        display = layer_data->Unwrap(display);
    }
    VkResult result = layer_data->instance_dispatch_table.AcquireXlibDisplayEXT(physicalDevice, dpy, display);

    return result;
}
#endif // VK_USE_PLATFORM_XLIB_XRANDR_EXT

#ifdef VK_USE_PLATFORM_XLIB_XRANDR_EXT

VkResult DispatchGetRandROutputDisplayEXT(
    VkPhysicalDevice                            physicalDevice,
    Display*                                    dpy,
    RROutput                                    rrOutput,
    VkDisplayKHR*                               pDisplay)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    if (!wrap_handles) return layer_data->instance_dispatch_table.GetRandROutputDisplayEXT(physicalDevice, dpy, rrOutput, pDisplay);
    VkResult result = layer_data->instance_dispatch_table.GetRandROutputDisplayEXT(physicalDevice, dpy, rrOutput, pDisplay);
    if (VK_SUCCESS == result) {
        *pDisplay = layer_data->WrapNew(*pDisplay);
    }
    return result;
}
#endif // VK_USE_PLATFORM_XLIB_XRANDR_EXT

VkResult DispatchGetPhysicalDeviceSurfaceCapabilities2EXT(
    VkPhysicalDevice                            physicalDevice,
    VkSurfaceKHR                                surface,
    VkSurfaceCapabilities2EXT*                  pSurfaceCapabilities)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    if (!wrap_handles) return layer_data->instance_dispatch_table.GetPhysicalDeviceSurfaceCapabilities2EXT(physicalDevice, surface, pSurfaceCapabilities);
    {
        surface = layer_data->Unwrap(surface);
    }
    VkResult result = layer_data->instance_dispatch_table.GetPhysicalDeviceSurfaceCapabilities2EXT(physicalDevice, surface, pSurfaceCapabilities);

    return result;
}

VkResult DispatchDisplayPowerControlEXT(
    VkDevice                                    device,
    VkDisplayKHR                                display,
    const VkDisplayPowerInfoEXT*                pDisplayPowerInfo)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.DisplayPowerControlEXT(device, display, pDisplayPowerInfo);
    {
        display = layer_data->Unwrap(display);
    }
    VkResult result = layer_data->device_dispatch_table.DisplayPowerControlEXT(device, display, pDisplayPowerInfo);

    return result;
}

VkResult DispatchRegisterDeviceEventEXT(
    VkDevice                                    device,
    const VkDeviceEventInfoEXT*                 pDeviceEventInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkFence*                                    pFence)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.RegisterDeviceEventEXT(device, pDeviceEventInfo, pAllocator, pFence);
    VkResult result = layer_data->device_dispatch_table.RegisterDeviceEventEXT(device, pDeviceEventInfo, pAllocator, pFence);
    if (VK_SUCCESS == result) {
        *pFence = layer_data->WrapNew(*pFence);
    }
    return result;
}

VkResult DispatchRegisterDisplayEventEXT(
    VkDevice                                    device,
    VkDisplayKHR                                display,
    const VkDisplayEventInfoEXT*                pDisplayEventInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkFence*                                    pFence)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.RegisterDisplayEventEXT(device, display, pDisplayEventInfo, pAllocator, pFence);
    {
        display = layer_data->Unwrap(display);
    }
    VkResult result = layer_data->device_dispatch_table.RegisterDisplayEventEXT(device, display, pDisplayEventInfo, pAllocator, pFence);
    if (VK_SUCCESS == result) {
        *pFence = layer_data->WrapNew(*pFence);
    }
    return result;
}

VkResult DispatchGetSwapchainCounterEXT(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    VkSurfaceCounterFlagBitsEXT                 counter,
    uint64_t*                                   pCounterValue)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.GetSwapchainCounterEXT(device, swapchain, counter, pCounterValue);
    {
        swapchain = layer_data->Unwrap(swapchain);
    }
    VkResult result = layer_data->device_dispatch_table.GetSwapchainCounterEXT(device, swapchain, counter, pCounterValue);

    return result;
}

VkResult DispatchGetRefreshCycleDurationGOOGLE(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    VkRefreshCycleDurationGOOGLE*               pDisplayTimingProperties)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.GetRefreshCycleDurationGOOGLE(device, swapchain, pDisplayTimingProperties);
    {
        swapchain = layer_data->Unwrap(swapchain);
    }
    VkResult result = layer_data->device_dispatch_table.GetRefreshCycleDurationGOOGLE(device, swapchain, pDisplayTimingProperties);

    return result;
}

VkResult DispatchGetPastPresentationTimingGOOGLE(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    uint32_t*                                   pPresentationTimingCount,
    VkPastPresentationTimingGOOGLE*             pPresentationTimings)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.GetPastPresentationTimingGOOGLE(device, swapchain, pPresentationTimingCount, pPresentationTimings);
    {
        swapchain = layer_data->Unwrap(swapchain);
    }
    VkResult result = layer_data->device_dispatch_table.GetPastPresentationTimingGOOGLE(device, swapchain, pPresentationTimingCount, pPresentationTimings);

    return result;
}

void DispatchCmdSetDiscardRectangleEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstDiscardRectangle,
    uint32_t                                    discardRectangleCount,
    const VkRect2D*                             pDiscardRectangles)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    layer_data->device_dispatch_table.CmdSetDiscardRectangleEXT(commandBuffer, firstDiscardRectangle, discardRectangleCount, pDiscardRectangles);

}

void DispatchSetHdrMetadataEXT(
    VkDevice                                    device,
    uint32_t                                    swapchainCount,
    const VkSwapchainKHR*                       pSwapchains,
    const VkHdrMetadataEXT*                     pMetadata)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.SetHdrMetadataEXT(device, swapchainCount, pSwapchains, pMetadata);
    VkSwapchainKHR *local_pSwapchains = NULL;
    {
        if (pSwapchains) {
            local_pSwapchains = new VkSwapchainKHR[swapchainCount];
            for (uint32_t index0 = 0; index0 < swapchainCount; ++index0) {
                local_pSwapchains[index0] = layer_data->Unwrap(pSwapchains[index0]);
            }
        }
    }
    layer_data->device_dispatch_table.SetHdrMetadataEXT(device, swapchainCount, (const VkSwapchainKHR*)local_pSwapchains, pMetadata);
    if (local_pSwapchains)
        delete[] local_pSwapchains;
}

#ifdef VK_USE_PLATFORM_IOS_MVK

VkResult DispatchCreateIOSSurfaceMVK(
    VkInstance                                  instance,
    const VkIOSSurfaceCreateInfoMVK*            pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(instance), layer_data_map);
    if (!wrap_handles) return layer_data->instance_dispatch_table.CreateIOSSurfaceMVK(instance, pCreateInfo, pAllocator, pSurface);
    VkResult result = layer_data->instance_dispatch_table.CreateIOSSurfaceMVK(instance, pCreateInfo, pAllocator, pSurface);
    if (VK_SUCCESS == result) {
        *pSurface = layer_data->WrapNew(*pSurface);
    }
    return result;
}
#endif // VK_USE_PLATFORM_IOS_MVK

#ifdef VK_USE_PLATFORM_MACOS_MVK

VkResult DispatchCreateMacOSSurfaceMVK(
    VkInstance                                  instance,
    const VkMacOSSurfaceCreateInfoMVK*          pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(instance), layer_data_map);
    if (!wrap_handles) return layer_data->instance_dispatch_table.CreateMacOSSurfaceMVK(instance, pCreateInfo, pAllocator, pSurface);
    VkResult result = layer_data->instance_dispatch_table.CreateMacOSSurfaceMVK(instance, pCreateInfo, pAllocator, pSurface);
    if (VK_SUCCESS == result) {
        *pSurface = layer_data->WrapNew(*pSurface);
    }
    return result;
}
#endif // VK_USE_PLATFORM_MACOS_MVK

// Skip vkSetDebugUtilsObjectNameEXT dispatch, manually generated

// Skip vkSetDebugUtilsObjectTagEXT dispatch, manually generated

void DispatchQueueBeginDebugUtilsLabelEXT(
    VkQueue                                     queue,
    const VkDebugUtilsLabelEXT*                 pLabelInfo)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(queue), layer_data_map);
    layer_data->device_dispatch_table.QueueBeginDebugUtilsLabelEXT(queue, pLabelInfo);

}

void DispatchQueueEndDebugUtilsLabelEXT(
    VkQueue                                     queue)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(queue), layer_data_map);
    layer_data->device_dispatch_table.QueueEndDebugUtilsLabelEXT(queue);

}

void DispatchQueueInsertDebugUtilsLabelEXT(
    VkQueue                                     queue,
    const VkDebugUtilsLabelEXT*                 pLabelInfo)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(queue), layer_data_map);
    layer_data->device_dispatch_table.QueueInsertDebugUtilsLabelEXT(queue, pLabelInfo);

}

void DispatchCmdBeginDebugUtilsLabelEXT(
    VkCommandBuffer                             commandBuffer,
    const VkDebugUtilsLabelEXT*                 pLabelInfo)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    layer_data->device_dispatch_table.CmdBeginDebugUtilsLabelEXT(commandBuffer, pLabelInfo);

}

void DispatchCmdEndDebugUtilsLabelEXT(
    VkCommandBuffer                             commandBuffer)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    layer_data->device_dispatch_table.CmdEndDebugUtilsLabelEXT(commandBuffer);

}

void DispatchCmdInsertDebugUtilsLabelEXT(
    VkCommandBuffer                             commandBuffer,
    const VkDebugUtilsLabelEXT*                 pLabelInfo)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    layer_data->device_dispatch_table.CmdInsertDebugUtilsLabelEXT(commandBuffer, pLabelInfo);

}

VkResult DispatchCreateDebugUtilsMessengerEXT(
    VkInstance                                  instance,
    const VkDebugUtilsMessengerCreateInfoEXT*   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDebugUtilsMessengerEXT*                   pMessenger)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(instance), layer_data_map);
    if (!wrap_handles) return layer_data->instance_dispatch_table.CreateDebugUtilsMessengerEXT(instance, pCreateInfo, pAllocator, pMessenger);
    VkResult result = layer_data->instance_dispatch_table.CreateDebugUtilsMessengerEXT(instance, pCreateInfo, pAllocator, pMessenger);
    if (VK_SUCCESS == result) {
        *pMessenger = layer_data->WrapNew(*pMessenger);
    }
    return result;
}

void DispatchDestroyDebugUtilsMessengerEXT(
    VkInstance                                  instance,
    VkDebugUtilsMessengerEXT                    messenger,
    const VkAllocationCallbacks*                pAllocator)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(instance), layer_data_map);
    if (!wrap_handles) return layer_data->instance_dispatch_table.DestroyDebugUtilsMessengerEXT(instance, messenger, pAllocator);
    uint64_t messenger_id = reinterpret_cast<uint64_t &>(messenger);
    auto iter = unique_id_mapping.pop(messenger_id);
    if (iter != unique_id_mapping.end()) {
        messenger = (VkDebugUtilsMessengerEXT)iter->second;
    } else {
        messenger = (VkDebugUtilsMessengerEXT)0;
    }
    layer_data->instance_dispatch_table.DestroyDebugUtilsMessengerEXT(instance, messenger, pAllocator);

}

void DispatchSubmitDebugUtilsMessageEXT(
    VkInstance                                  instance,
    VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT             messageTypes,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(instance), layer_data_map);
    layer_data->instance_dispatch_table.SubmitDebugUtilsMessageEXT(instance, messageSeverity, messageTypes, pCallbackData);

}

#ifdef VK_USE_PLATFORM_ANDROID_KHR

VkResult DispatchGetAndroidHardwareBufferPropertiesANDROID(
    VkDevice                                    device,
    const struct AHardwareBuffer*               buffer,
    VkAndroidHardwareBufferPropertiesANDROID*   pProperties)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    VkResult result = layer_data->device_dispatch_table.GetAndroidHardwareBufferPropertiesANDROID(device, buffer, pProperties);

    return result;
}
#endif // VK_USE_PLATFORM_ANDROID_KHR

#ifdef VK_USE_PLATFORM_ANDROID_KHR

VkResult DispatchGetMemoryAndroidHardwareBufferANDROID(
    VkDevice                                    device,
    const VkMemoryGetAndroidHardwareBufferInfoANDROID* pInfo,
    struct AHardwareBuffer**                    pBuffer)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.GetMemoryAndroidHardwareBufferANDROID(device, pInfo, pBuffer);
    safe_VkMemoryGetAndroidHardwareBufferInfoANDROID *local_pInfo = NULL;
    {
        if (pInfo) {
            local_pInfo = new safe_VkMemoryGetAndroidHardwareBufferInfoANDROID(pInfo);
            if (pInfo->memory) {
                local_pInfo->memory = layer_data->Unwrap(pInfo->memory);
            }
        }
    }
    VkResult result = layer_data->device_dispatch_table.GetMemoryAndroidHardwareBufferANDROID(device, (const VkMemoryGetAndroidHardwareBufferInfoANDROID*)local_pInfo, pBuffer);
    if (local_pInfo) {
        delete local_pInfo;
    }
    return result;
}
#endif // VK_USE_PLATFORM_ANDROID_KHR

void DispatchCmdSetSampleLocationsEXT(
    VkCommandBuffer                             commandBuffer,
    const VkSampleLocationsInfoEXT*             pSampleLocationsInfo)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    layer_data->device_dispatch_table.CmdSetSampleLocationsEXT(commandBuffer, pSampleLocationsInfo);

}

void DispatchGetPhysicalDeviceMultisamplePropertiesEXT(
    VkPhysicalDevice                            physicalDevice,
    VkSampleCountFlagBits                       samples,
    VkMultisamplePropertiesEXT*                 pMultisampleProperties)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    layer_data->instance_dispatch_table.GetPhysicalDeviceMultisamplePropertiesEXT(physicalDevice, samples, pMultisampleProperties);

}

VkResult DispatchGetImageDrmFormatModifierPropertiesEXT(
    VkDevice                                    device,
    VkImage                                     image,
    VkImageDrmFormatModifierPropertiesEXT*      pProperties)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.GetImageDrmFormatModifierPropertiesEXT(device, image, pProperties);
    {
        image = layer_data->Unwrap(image);
    }
    VkResult result = layer_data->device_dispatch_table.GetImageDrmFormatModifierPropertiesEXT(device, image, pProperties);

    return result;
}

VkResult DispatchCreateValidationCacheEXT(
    VkDevice                                    device,
    const VkValidationCacheCreateInfoEXT*       pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkValidationCacheEXT*                       pValidationCache)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CreateValidationCacheEXT(device, pCreateInfo, pAllocator, pValidationCache);
    VkResult result = layer_data->device_dispatch_table.CreateValidationCacheEXT(device, pCreateInfo, pAllocator, pValidationCache);
    if (VK_SUCCESS == result) {
        *pValidationCache = layer_data->WrapNew(*pValidationCache);
    }
    return result;
}

void DispatchDestroyValidationCacheEXT(
    VkDevice                                    device,
    VkValidationCacheEXT                        validationCache,
    const VkAllocationCallbacks*                pAllocator)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.DestroyValidationCacheEXT(device, validationCache, pAllocator);
    uint64_t validationCache_id = reinterpret_cast<uint64_t &>(validationCache);
    auto iter = unique_id_mapping.pop(validationCache_id);
    if (iter != unique_id_mapping.end()) {
        validationCache = (VkValidationCacheEXT)iter->second;
    } else {
        validationCache = (VkValidationCacheEXT)0;
    }
    layer_data->device_dispatch_table.DestroyValidationCacheEXT(device, validationCache, pAllocator);

}

VkResult DispatchMergeValidationCachesEXT(
    VkDevice                                    device,
    VkValidationCacheEXT                        dstCache,
    uint32_t                                    srcCacheCount,
    const VkValidationCacheEXT*                 pSrcCaches)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.MergeValidationCachesEXT(device, dstCache, srcCacheCount, pSrcCaches);
    VkValidationCacheEXT *local_pSrcCaches = NULL;
    {
        dstCache = layer_data->Unwrap(dstCache);
        if (pSrcCaches) {
            local_pSrcCaches = new VkValidationCacheEXT[srcCacheCount];
            for (uint32_t index0 = 0; index0 < srcCacheCount; ++index0) {
                local_pSrcCaches[index0] = layer_data->Unwrap(pSrcCaches[index0]);
            }
        }
    }
    VkResult result = layer_data->device_dispatch_table.MergeValidationCachesEXT(device, dstCache, srcCacheCount, (const VkValidationCacheEXT*)local_pSrcCaches);
    if (local_pSrcCaches)
        delete[] local_pSrcCaches;
    return result;
}

VkResult DispatchGetValidationCacheDataEXT(
    VkDevice                                    device,
    VkValidationCacheEXT                        validationCache,
    size_t*                                     pDataSize,
    void*                                       pData)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.GetValidationCacheDataEXT(device, validationCache, pDataSize, pData);
    {
        validationCache = layer_data->Unwrap(validationCache);
    }
    VkResult result = layer_data->device_dispatch_table.GetValidationCacheDataEXT(device, validationCache, pDataSize, pData);

    return result;
}

void DispatchCmdBindShadingRateImageNV(
    VkCommandBuffer                             commandBuffer,
    VkImageView                                 imageView,
    VkImageLayout                               imageLayout)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CmdBindShadingRateImageNV(commandBuffer, imageView, imageLayout);
    {
        imageView = layer_data->Unwrap(imageView);
    }
    layer_data->device_dispatch_table.CmdBindShadingRateImageNV(commandBuffer, imageView, imageLayout);

}

void DispatchCmdSetViewportShadingRatePaletteNV(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstViewport,
    uint32_t                                    viewportCount,
    const VkShadingRatePaletteNV*               pShadingRatePalettes)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    layer_data->device_dispatch_table.CmdSetViewportShadingRatePaletteNV(commandBuffer, firstViewport, viewportCount, pShadingRatePalettes);

}

void DispatchCmdSetCoarseSampleOrderNV(
    VkCommandBuffer                             commandBuffer,
    VkCoarseSampleOrderTypeNV                   sampleOrderType,
    uint32_t                                    customSampleOrderCount,
    const VkCoarseSampleOrderCustomNV*          pCustomSampleOrders)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    layer_data->device_dispatch_table.CmdSetCoarseSampleOrderNV(commandBuffer, sampleOrderType, customSampleOrderCount, pCustomSampleOrders);

}

VkResult DispatchCreateAccelerationStructureNV(
    VkDevice                                    device,
    const VkAccelerationStructureCreateInfoNV*  pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkAccelerationStructureNV*                  pAccelerationStructure)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CreateAccelerationStructureNV(device, pCreateInfo, pAllocator, pAccelerationStructure);
    safe_VkAccelerationStructureCreateInfoNV *local_pCreateInfo = NULL;
    {
        if (pCreateInfo) {
            local_pCreateInfo = new safe_VkAccelerationStructureCreateInfoNV(pCreateInfo);
            if (local_pCreateInfo->info.pGeometries) {
                for (uint32_t index2 = 0; index2 < local_pCreateInfo->info.geometryCount; ++index2) {
                    if (pCreateInfo->info.pGeometries[index2].geometry.triangles.vertexData) {
                        local_pCreateInfo->info.pGeometries[index2].geometry.triangles.vertexData = layer_data->Unwrap(pCreateInfo->info.pGeometries[index2].geometry.triangles.vertexData);
                    }
                    if (pCreateInfo->info.pGeometries[index2].geometry.triangles.indexData) {
                        local_pCreateInfo->info.pGeometries[index2].geometry.triangles.indexData = layer_data->Unwrap(pCreateInfo->info.pGeometries[index2].geometry.triangles.indexData);
                    }
                    if (pCreateInfo->info.pGeometries[index2].geometry.triangles.transformData) {
                        local_pCreateInfo->info.pGeometries[index2].geometry.triangles.transformData = layer_data->Unwrap(pCreateInfo->info.pGeometries[index2].geometry.triangles.transformData);
                    }
                    if (pCreateInfo->info.pGeometries[index2].geometry.aabbs.aabbData) {
                        local_pCreateInfo->info.pGeometries[index2].geometry.aabbs.aabbData = layer_data->Unwrap(pCreateInfo->info.pGeometries[index2].geometry.aabbs.aabbData);
                    }
                }
            }
        }
    }
    VkResult result = layer_data->device_dispatch_table.CreateAccelerationStructureNV(device, (const VkAccelerationStructureCreateInfoNV*)local_pCreateInfo, pAllocator, pAccelerationStructure);
    if (local_pCreateInfo) {
        delete local_pCreateInfo;
    }
    if (VK_SUCCESS == result) {
        *pAccelerationStructure = layer_data->WrapNew(*pAccelerationStructure);
    }
    return result;
}

void DispatchDestroyAccelerationStructureNV(
    VkDevice                                    device,
    VkAccelerationStructureNV                   accelerationStructure,
    const VkAllocationCallbacks*                pAllocator)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.DestroyAccelerationStructureNV(device, accelerationStructure, pAllocator);
    uint64_t accelerationStructure_id = reinterpret_cast<uint64_t &>(accelerationStructure);
    auto iter = unique_id_mapping.pop(accelerationStructure_id);
    if (iter != unique_id_mapping.end()) {
        accelerationStructure = (VkAccelerationStructureNV)iter->second;
    } else {
        accelerationStructure = (VkAccelerationStructureNV)0;
    }
    layer_data->device_dispatch_table.DestroyAccelerationStructureNV(device, accelerationStructure, pAllocator);

}

void DispatchGetAccelerationStructureMemoryRequirementsNV(
    VkDevice                                    device,
    const VkAccelerationStructureMemoryRequirementsInfoNV* pInfo,
    VkMemoryRequirements2KHR*                   pMemoryRequirements)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.GetAccelerationStructureMemoryRequirementsNV(device, pInfo, pMemoryRequirements);
    safe_VkAccelerationStructureMemoryRequirementsInfoNV *local_pInfo = NULL;
    {
        if (pInfo) {
            local_pInfo = new safe_VkAccelerationStructureMemoryRequirementsInfoNV(pInfo);
            if (pInfo->accelerationStructure) {
                local_pInfo->accelerationStructure = layer_data->Unwrap(pInfo->accelerationStructure);
            }
        }
    }
    layer_data->device_dispatch_table.GetAccelerationStructureMemoryRequirementsNV(device, (const VkAccelerationStructureMemoryRequirementsInfoNV*)local_pInfo, pMemoryRequirements);
    if (local_pInfo) {
        delete local_pInfo;
    }
}

VkResult DispatchBindAccelerationStructureMemoryNV(
    VkDevice                                    device,
    uint32_t                                    bindInfoCount,
    const VkBindAccelerationStructureMemoryInfoNV* pBindInfos)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.BindAccelerationStructureMemoryNV(device, bindInfoCount, pBindInfos);
    safe_VkBindAccelerationStructureMemoryInfoNV *local_pBindInfos = NULL;
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
    VkResult result = layer_data->device_dispatch_table.BindAccelerationStructureMemoryNV(device, bindInfoCount, (const VkBindAccelerationStructureMemoryInfoNV*)local_pBindInfos);
    if (local_pBindInfos) {
        delete[] local_pBindInfos;
    }
    return result;
}

void DispatchCmdBuildAccelerationStructureNV(
    VkCommandBuffer                             commandBuffer,
    const VkAccelerationStructureInfoNV*        pInfo,
    VkBuffer                                    instanceData,
    VkDeviceSize                                instanceOffset,
    VkBool32                                    update,
    VkAccelerationStructureNV                   dst,
    VkAccelerationStructureNV                   src,
    VkBuffer                                    scratch,
    VkDeviceSize                                scratchOffset)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CmdBuildAccelerationStructureNV(commandBuffer, pInfo, instanceData, instanceOffset, update, dst, src, scratch, scratchOffset);
    safe_VkAccelerationStructureInfoNV *local_pInfo = NULL;
    {
        if (pInfo) {
            local_pInfo = new safe_VkAccelerationStructureInfoNV(pInfo);
            if (local_pInfo->pGeometries) {
                for (uint32_t index1 = 0; index1 < local_pInfo->geometryCount; ++index1) {
                    if (pInfo->pGeometries[index1].geometry.triangles.vertexData) {
                        local_pInfo->pGeometries[index1].geometry.triangles.vertexData = layer_data->Unwrap(pInfo->pGeometries[index1].geometry.triangles.vertexData);
                    }
                    if (pInfo->pGeometries[index1].geometry.triangles.indexData) {
                        local_pInfo->pGeometries[index1].geometry.triangles.indexData = layer_data->Unwrap(pInfo->pGeometries[index1].geometry.triangles.indexData);
                    }
                    if (pInfo->pGeometries[index1].geometry.triangles.transformData) {
                        local_pInfo->pGeometries[index1].geometry.triangles.transformData = layer_data->Unwrap(pInfo->pGeometries[index1].geometry.triangles.transformData);
                    }
                    if (pInfo->pGeometries[index1].geometry.aabbs.aabbData) {
                        local_pInfo->pGeometries[index1].geometry.aabbs.aabbData = layer_data->Unwrap(pInfo->pGeometries[index1].geometry.aabbs.aabbData);
                    }
                }
            }
        }
        instanceData = layer_data->Unwrap(instanceData);
        dst = layer_data->Unwrap(dst);
        src = layer_data->Unwrap(src);
        scratch = layer_data->Unwrap(scratch);
    }
    layer_data->device_dispatch_table.CmdBuildAccelerationStructureNV(commandBuffer, (const VkAccelerationStructureInfoNV*)local_pInfo, instanceData, instanceOffset, update, dst, src, scratch, scratchOffset);
    if (local_pInfo) {
        delete local_pInfo;
    }
}

void DispatchCmdCopyAccelerationStructureNV(
    VkCommandBuffer                             commandBuffer,
    VkAccelerationStructureNV                   dst,
    VkAccelerationStructureNV                   src,
    VkCopyAccelerationStructureModeNV           mode)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CmdCopyAccelerationStructureNV(commandBuffer, dst, src, mode);
    {
        dst = layer_data->Unwrap(dst);
        src = layer_data->Unwrap(src);
    }
    layer_data->device_dispatch_table.CmdCopyAccelerationStructureNV(commandBuffer, dst, src, mode);

}

void DispatchCmdTraceRaysNV(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    raygenShaderBindingTableBuffer,
    VkDeviceSize                                raygenShaderBindingOffset,
    VkBuffer                                    missShaderBindingTableBuffer,
    VkDeviceSize                                missShaderBindingOffset,
    VkDeviceSize                                missShaderBindingStride,
    VkBuffer                                    hitShaderBindingTableBuffer,
    VkDeviceSize                                hitShaderBindingOffset,
    VkDeviceSize                                hitShaderBindingStride,
    VkBuffer                                    callableShaderBindingTableBuffer,
    VkDeviceSize                                callableShaderBindingOffset,
    VkDeviceSize                                callableShaderBindingStride,
    uint32_t                                    width,
    uint32_t                                    height,
    uint32_t                                    depth)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CmdTraceRaysNV(commandBuffer, raygenShaderBindingTableBuffer, raygenShaderBindingOffset, missShaderBindingTableBuffer, missShaderBindingOffset, missShaderBindingStride, hitShaderBindingTableBuffer, hitShaderBindingOffset, hitShaderBindingStride, callableShaderBindingTableBuffer, callableShaderBindingOffset, callableShaderBindingStride, width, height, depth);
    {
        raygenShaderBindingTableBuffer = layer_data->Unwrap(raygenShaderBindingTableBuffer);
        missShaderBindingTableBuffer = layer_data->Unwrap(missShaderBindingTableBuffer);
        hitShaderBindingTableBuffer = layer_data->Unwrap(hitShaderBindingTableBuffer);
        callableShaderBindingTableBuffer = layer_data->Unwrap(callableShaderBindingTableBuffer);
    }
    layer_data->device_dispatch_table.CmdTraceRaysNV(commandBuffer, raygenShaderBindingTableBuffer, raygenShaderBindingOffset, missShaderBindingTableBuffer, missShaderBindingOffset, missShaderBindingStride, hitShaderBindingTableBuffer, hitShaderBindingOffset, hitShaderBindingStride, callableShaderBindingTableBuffer, callableShaderBindingOffset, callableShaderBindingStride, width, height, depth);

}

VkResult DispatchCreateRayTracingPipelinesNV(
    VkDevice                                    device,
    VkPipelineCache                             pipelineCache,
    uint32_t                                    createInfoCount,
    const VkRayTracingPipelineCreateInfoNV*     pCreateInfos,
    const VkAllocationCallbacks*                pAllocator,
    VkPipeline*                                 pPipelines)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CreateRayTracingPipelinesNV(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);
    safe_VkRayTracingPipelineCreateInfoNV *local_pCreateInfos = NULL;
    {
        pipelineCache = layer_data->Unwrap(pipelineCache);
        if (pCreateInfos) {
            local_pCreateInfos = new safe_VkRayTracingPipelineCreateInfoNV[createInfoCount];
            for (uint32_t index0 = 0; index0 < createInfoCount; ++index0) {
                local_pCreateInfos[index0].initialize(&pCreateInfos[index0]);
                if (local_pCreateInfos[index0].pStages) {
                    for (uint32_t index1 = 0; index1 < local_pCreateInfos[index0].stageCount; ++index1) {
                        if (pCreateInfos[index0].pStages[index1].module) {
                            local_pCreateInfos[index0].pStages[index1].module = layer_data->Unwrap(pCreateInfos[index0].pStages[index1].module);
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
    VkResult result = layer_data->device_dispatch_table.CreateRayTracingPipelinesNV(device, pipelineCache, createInfoCount, (const VkRayTracingPipelineCreateInfoNV*)local_pCreateInfos, pAllocator, pPipelines);
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

VkResult DispatchGetRayTracingShaderGroupHandlesNV(
    VkDevice                                    device,
    VkPipeline                                  pipeline,
    uint32_t                                    firstGroup,
    uint32_t                                    groupCount,
    size_t                                      dataSize,
    void*                                       pData)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.GetRayTracingShaderGroupHandlesNV(device, pipeline, firstGroup, groupCount, dataSize, pData);
    {
        pipeline = layer_data->Unwrap(pipeline);
    }
    VkResult result = layer_data->device_dispatch_table.GetRayTracingShaderGroupHandlesNV(device, pipeline, firstGroup, groupCount, dataSize, pData);

    return result;
}

VkResult DispatchGetAccelerationStructureHandleNV(
    VkDevice                                    device,
    VkAccelerationStructureNV                   accelerationStructure,
    size_t                                      dataSize,
    void*                                       pData)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.GetAccelerationStructureHandleNV(device, accelerationStructure, dataSize, pData);
    {
        accelerationStructure = layer_data->Unwrap(accelerationStructure);
    }
    VkResult result = layer_data->device_dispatch_table.GetAccelerationStructureHandleNV(device, accelerationStructure, dataSize, pData);

    return result;
}

void DispatchCmdWriteAccelerationStructuresPropertiesNV(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    accelerationStructureCount,
    const VkAccelerationStructureNV*            pAccelerationStructures,
    VkQueryType                                 queryType,
    VkQueryPool                                 queryPool,
    uint32_t                                    firstQuery)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CmdWriteAccelerationStructuresPropertiesNV(commandBuffer, accelerationStructureCount, pAccelerationStructures, queryType, queryPool, firstQuery);
    VkAccelerationStructureNV *local_pAccelerationStructures = NULL;
    {
        if (pAccelerationStructures) {
            local_pAccelerationStructures = new VkAccelerationStructureNV[accelerationStructureCount];
            for (uint32_t index0 = 0; index0 < accelerationStructureCount; ++index0) {
                local_pAccelerationStructures[index0] = layer_data->Unwrap(pAccelerationStructures[index0]);
            }
        }
        queryPool = layer_data->Unwrap(queryPool);
    }
    layer_data->device_dispatch_table.CmdWriteAccelerationStructuresPropertiesNV(commandBuffer, accelerationStructureCount, (const VkAccelerationStructureNV*)local_pAccelerationStructures, queryType, queryPool, firstQuery);
    if (local_pAccelerationStructures)
        delete[] local_pAccelerationStructures;
}

VkResult DispatchCompileDeferredNV(
    VkDevice                                    device,
    VkPipeline                                  pipeline,
    uint32_t                                    shader)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CompileDeferredNV(device, pipeline, shader);
    {
        pipeline = layer_data->Unwrap(pipeline);
    }
    VkResult result = layer_data->device_dispatch_table.CompileDeferredNV(device, pipeline, shader);

    return result;
}

VkResult DispatchGetMemoryHostPointerPropertiesEXT(
    VkDevice                                    device,
    VkExternalMemoryHandleTypeFlagBits          handleType,
    const void*                                 pHostPointer,
    VkMemoryHostPointerPropertiesEXT*           pMemoryHostPointerProperties)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    VkResult result = layer_data->device_dispatch_table.GetMemoryHostPointerPropertiesEXT(device, handleType, pHostPointer, pMemoryHostPointerProperties);

    return result;
}

void DispatchCmdWriteBufferMarkerAMD(
    VkCommandBuffer                             commandBuffer,
    VkPipelineStageFlagBits                     pipelineStage,
    VkBuffer                                    dstBuffer,
    VkDeviceSize                                dstOffset,
    uint32_t                                    marker)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CmdWriteBufferMarkerAMD(commandBuffer, pipelineStage, dstBuffer, dstOffset, marker);
    {
        dstBuffer = layer_data->Unwrap(dstBuffer);
    }
    layer_data->device_dispatch_table.CmdWriteBufferMarkerAMD(commandBuffer, pipelineStage, dstBuffer, dstOffset, marker);

}

VkResult DispatchGetPhysicalDeviceCalibrateableTimeDomainsEXT(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pTimeDomainCount,
    VkTimeDomainEXT*                            pTimeDomains)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    VkResult result = layer_data->instance_dispatch_table.GetPhysicalDeviceCalibrateableTimeDomainsEXT(physicalDevice, pTimeDomainCount, pTimeDomains);

    return result;
}

VkResult DispatchGetCalibratedTimestampsEXT(
    VkDevice                                    device,
    uint32_t                                    timestampCount,
    const VkCalibratedTimestampInfoEXT*         pTimestampInfos,
    uint64_t*                                   pTimestamps,
    uint64_t*                                   pMaxDeviation)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    VkResult result = layer_data->device_dispatch_table.GetCalibratedTimestampsEXT(device, timestampCount, pTimestampInfos, pTimestamps, pMaxDeviation);

    return result;
}

void DispatchCmdDrawMeshTasksNV(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    taskCount,
    uint32_t                                    firstTask)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    layer_data->device_dispatch_table.CmdDrawMeshTasksNV(commandBuffer, taskCount, firstTask);

}

void DispatchCmdDrawMeshTasksIndirectNV(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    uint32_t                                    drawCount,
    uint32_t                                    stride)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CmdDrawMeshTasksIndirectNV(commandBuffer, buffer, offset, drawCount, stride);
    {
        buffer = layer_data->Unwrap(buffer);
    }
    layer_data->device_dispatch_table.CmdDrawMeshTasksIndirectNV(commandBuffer, buffer, offset, drawCount, stride);

}

void DispatchCmdDrawMeshTasksIndirectCountNV(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkBuffer                                    countBuffer,
    VkDeviceSize                                countBufferOffset,
    uint32_t                                    maxDrawCount,
    uint32_t                                    stride)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.CmdDrawMeshTasksIndirectCountNV(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    {
        buffer = layer_data->Unwrap(buffer);
        countBuffer = layer_data->Unwrap(countBuffer);
    }
    layer_data->device_dispatch_table.CmdDrawMeshTasksIndirectCountNV(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);

}

void DispatchCmdSetExclusiveScissorNV(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstExclusiveScissor,
    uint32_t                                    exclusiveScissorCount,
    const VkRect2D*                             pExclusiveScissors)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    layer_data->device_dispatch_table.CmdSetExclusiveScissorNV(commandBuffer, firstExclusiveScissor, exclusiveScissorCount, pExclusiveScissors);

}

void DispatchCmdSetCheckpointNV(
    VkCommandBuffer                             commandBuffer,
    const void*                                 pCheckpointMarker)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    layer_data->device_dispatch_table.CmdSetCheckpointNV(commandBuffer, pCheckpointMarker);

}

void DispatchGetQueueCheckpointDataNV(
    VkQueue                                     queue,
    uint32_t*                                   pCheckpointDataCount,
    VkCheckpointDataNV*                         pCheckpointData)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(queue), layer_data_map);
    layer_data->device_dispatch_table.GetQueueCheckpointDataNV(queue, pCheckpointDataCount, pCheckpointData);

}

VkResult DispatchInitializePerformanceApiINTEL(
    VkDevice                                    device,
    const VkInitializePerformanceApiInfoINTEL*  pInitializeInfo)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    VkResult result = layer_data->device_dispatch_table.InitializePerformanceApiINTEL(device, pInitializeInfo);

    return result;
}

void DispatchUninitializePerformanceApiINTEL(
    VkDevice                                    device)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    layer_data->device_dispatch_table.UninitializePerformanceApiINTEL(device);

}

VkResult DispatchCmdSetPerformanceMarkerINTEL(
    VkCommandBuffer                             commandBuffer,
    const VkPerformanceMarkerInfoINTEL*         pMarkerInfo)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    VkResult result = layer_data->device_dispatch_table.CmdSetPerformanceMarkerINTEL(commandBuffer, pMarkerInfo);

    return result;
}

VkResult DispatchCmdSetPerformanceStreamMarkerINTEL(
    VkCommandBuffer                             commandBuffer,
    const VkPerformanceStreamMarkerInfoINTEL*   pMarkerInfo)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    VkResult result = layer_data->device_dispatch_table.CmdSetPerformanceStreamMarkerINTEL(commandBuffer, pMarkerInfo);

    return result;
}

VkResult DispatchCmdSetPerformanceOverrideINTEL(
    VkCommandBuffer                             commandBuffer,
    const VkPerformanceOverrideInfoINTEL*       pOverrideInfo)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    VkResult result = layer_data->device_dispatch_table.CmdSetPerformanceOverrideINTEL(commandBuffer, pOverrideInfo);

    return result;
}

VkResult DispatchAcquirePerformanceConfigurationINTEL(
    VkDevice                                    device,
    const VkPerformanceConfigurationAcquireInfoINTEL* pAcquireInfo,
    VkPerformanceConfigurationINTEL*            pConfiguration)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.AcquirePerformanceConfigurationINTEL(device, pAcquireInfo, pConfiguration);
    {
        pConfiguration = layer_data->Unwrap(pConfiguration);
    }
    VkResult result = layer_data->device_dispatch_table.AcquirePerformanceConfigurationINTEL(device, pAcquireInfo, pConfiguration);

    return result;
}

VkResult DispatchReleasePerformanceConfigurationINTEL(
    VkDevice                                    device,
    VkPerformanceConfigurationINTEL             configuration)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.ReleasePerformanceConfigurationINTEL(device, configuration);
    {
        configuration = layer_data->Unwrap(configuration);
    }
    VkResult result = layer_data->device_dispatch_table.ReleasePerformanceConfigurationINTEL(device, configuration);

    return result;
}

VkResult DispatchQueueSetPerformanceConfigurationINTEL(
    VkQueue                                     queue,
    VkPerformanceConfigurationINTEL             configuration)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(queue), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.QueueSetPerformanceConfigurationINTEL(queue, configuration);
    {
        configuration = layer_data->Unwrap(configuration);
    }
    VkResult result = layer_data->device_dispatch_table.QueueSetPerformanceConfigurationINTEL(queue, configuration);

    return result;
}

VkResult DispatchGetPerformanceParameterINTEL(
    VkDevice                                    device,
    VkPerformanceParameterTypeINTEL             parameter,
    VkPerformanceValueINTEL*                    pValue)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    VkResult result = layer_data->device_dispatch_table.GetPerformanceParameterINTEL(device, parameter, pValue);

    return result;
}

void DispatchSetLocalDimmingAMD(
    VkDevice                                    device,
    VkSwapchainKHR                              swapChain,
    VkBool32                                    localDimmingEnable)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.SetLocalDimmingAMD(device, swapChain, localDimmingEnable);
    {
        swapChain = layer_data->Unwrap(swapChain);
    }
    layer_data->device_dispatch_table.SetLocalDimmingAMD(device, swapChain, localDimmingEnable);

}

#ifdef VK_USE_PLATFORM_FUCHSIA

VkResult DispatchCreateImagePipeSurfaceFUCHSIA(
    VkInstance                                  instance,
    const VkImagePipeSurfaceCreateInfoFUCHSIA*  pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(instance), layer_data_map);
    if (!wrap_handles) return layer_data->instance_dispatch_table.CreateImagePipeSurfaceFUCHSIA(instance, pCreateInfo, pAllocator, pSurface);
    VkResult result = layer_data->instance_dispatch_table.CreateImagePipeSurfaceFUCHSIA(instance, pCreateInfo, pAllocator, pSurface);
    if (VK_SUCCESS == result) {
        *pSurface = layer_data->WrapNew(*pSurface);
    }
    return result;
}
#endif // VK_USE_PLATFORM_FUCHSIA

#ifdef VK_USE_PLATFORM_METAL_EXT

VkResult DispatchCreateMetalSurfaceEXT(
    VkInstance                                  instance,
    const VkMetalSurfaceCreateInfoEXT*          pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(instance), layer_data_map);
    if (!wrap_handles) return layer_data->instance_dispatch_table.CreateMetalSurfaceEXT(instance, pCreateInfo, pAllocator, pSurface);
    VkResult result = layer_data->instance_dispatch_table.CreateMetalSurfaceEXT(instance, pCreateInfo, pAllocator, pSurface);
    if (VK_SUCCESS == result) {
        *pSurface = layer_data->WrapNew(*pSurface);
    }
    return result;
}
#endif // VK_USE_PLATFORM_METAL_EXT

VkDeviceAddress DispatchGetBufferDeviceAddressEXT(
    VkDevice                                    device,
    const VkBufferDeviceAddressInfoEXT*         pInfo)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.GetBufferDeviceAddressEXT(device, pInfo);
    safe_VkBufferDeviceAddressInfoEXT *local_pInfo = NULL;
    {
        if (pInfo) {
            local_pInfo = new safe_VkBufferDeviceAddressInfoEXT(pInfo);
            if (pInfo->buffer) {
                local_pInfo->buffer = layer_data->Unwrap(pInfo->buffer);
            }
        }
    }
    VkDeviceAddress result = layer_data->device_dispatch_table.GetBufferDeviceAddressEXT(device, (const VkBufferDeviceAddressInfoEXT*)local_pInfo);
    if (local_pInfo) {
        delete local_pInfo;
    }
    return result;
}

VkResult DispatchGetPhysicalDeviceCooperativeMatrixPropertiesNV(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pPropertyCount,
    VkCooperativeMatrixPropertiesNV*            pProperties)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    VkResult result = layer_data->instance_dispatch_table.GetPhysicalDeviceCooperativeMatrixPropertiesNV(physicalDevice, pPropertyCount, pProperties);

    return result;
}

VkResult DispatchGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pCombinationCount,
    VkFramebufferMixedSamplesCombinationNV*     pCombinations)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    VkResult result = layer_data->instance_dispatch_table.GetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV(physicalDevice, pCombinationCount, pCombinations);

    return result;
}

#ifdef VK_USE_PLATFORM_WIN32_KHR

VkResult DispatchGetPhysicalDeviceSurfacePresentModes2EXT(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceSurfaceInfo2KHR*      pSurfaceInfo,
    uint32_t*                                   pPresentModeCount,
    VkPresentModeKHR*                           pPresentModes)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    if (!wrap_handles) return layer_data->instance_dispatch_table.GetPhysicalDeviceSurfacePresentModes2EXT(physicalDevice, pSurfaceInfo, pPresentModeCount, pPresentModes);
    safe_VkPhysicalDeviceSurfaceInfo2KHR *local_pSurfaceInfo = NULL;
    {
        if (pSurfaceInfo) {
            local_pSurfaceInfo = new safe_VkPhysicalDeviceSurfaceInfo2KHR(pSurfaceInfo);
            if (pSurfaceInfo->surface) {
                local_pSurfaceInfo->surface = layer_data->Unwrap(pSurfaceInfo->surface);
            }
        }
    }
    VkResult result = layer_data->instance_dispatch_table.GetPhysicalDeviceSurfacePresentModes2EXT(physicalDevice, (const VkPhysicalDeviceSurfaceInfo2KHR*)local_pSurfaceInfo, pPresentModeCount, pPresentModes);
    if (local_pSurfaceInfo) {
        delete local_pSurfaceInfo;
    }
    return result;
}
#endif // VK_USE_PLATFORM_WIN32_KHR

#ifdef VK_USE_PLATFORM_WIN32_KHR

VkResult DispatchAcquireFullScreenExclusiveModeEXT(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.AcquireFullScreenExclusiveModeEXT(device, swapchain);
    {
        swapchain = layer_data->Unwrap(swapchain);
    }
    VkResult result = layer_data->device_dispatch_table.AcquireFullScreenExclusiveModeEXT(device, swapchain);

    return result;
}
#endif // VK_USE_PLATFORM_WIN32_KHR

#ifdef VK_USE_PLATFORM_WIN32_KHR

VkResult DispatchReleaseFullScreenExclusiveModeEXT(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.ReleaseFullScreenExclusiveModeEXT(device, swapchain);
    {
        swapchain = layer_data->Unwrap(swapchain);
    }
    VkResult result = layer_data->device_dispatch_table.ReleaseFullScreenExclusiveModeEXT(device, swapchain);

    return result;
}
#endif // VK_USE_PLATFORM_WIN32_KHR

#ifdef VK_USE_PLATFORM_WIN32_KHR

VkResult DispatchGetDeviceGroupSurfacePresentModes2EXT(
    VkDevice                                    device,
    const VkPhysicalDeviceSurfaceInfo2KHR*      pSurfaceInfo,
    VkDeviceGroupPresentModeFlagsKHR*           pModes)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.GetDeviceGroupSurfacePresentModes2EXT(device, pSurfaceInfo, pModes);
    safe_VkPhysicalDeviceSurfaceInfo2KHR *local_pSurfaceInfo = NULL;
    {
        if (pSurfaceInfo) {
            local_pSurfaceInfo = new safe_VkPhysicalDeviceSurfaceInfo2KHR(pSurfaceInfo);
            if (pSurfaceInfo->surface) {
                local_pSurfaceInfo->surface = layer_data->Unwrap(pSurfaceInfo->surface);
            }
        }
    }
    VkResult result = layer_data->device_dispatch_table.GetDeviceGroupSurfacePresentModes2EXT(device, (const VkPhysicalDeviceSurfaceInfo2KHR*)local_pSurfaceInfo, pModes);
    if (local_pSurfaceInfo) {
        delete local_pSurfaceInfo;
    }
    return result;
}
#endif // VK_USE_PLATFORM_WIN32_KHR

VkResult DispatchCreateHeadlessSurfaceEXT(
    VkInstance                                  instance,
    const VkHeadlessSurfaceCreateInfoEXT*       pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(instance), layer_data_map);
    if (!wrap_handles) return layer_data->instance_dispatch_table.CreateHeadlessSurfaceEXT(instance, pCreateInfo, pAllocator, pSurface);
    VkResult result = layer_data->instance_dispatch_table.CreateHeadlessSurfaceEXT(instance, pCreateInfo, pAllocator, pSurface);
    if (VK_SUCCESS == result) {
        *pSurface = layer_data->WrapNew(*pSurface);
    }
    return result;
}

void DispatchCmdSetLineStippleEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    lineStippleFactor,
    uint16_t                                    lineStipplePattern)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    layer_data->device_dispatch_table.CmdSetLineStippleEXT(commandBuffer, lineStippleFactor, lineStipplePattern);

}

void DispatchResetQueryPoolEXT(
    VkDevice                                    device,
    VkQueryPool                                 queryPool,
    uint32_t                                    firstQuery,
    uint32_t                                    queryCount)
{
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!wrap_handles) return layer_data->device_dispatch_table.ResetQueryPoolEXT(device, queryPool, firstQuery, queryCount);
    {
        queryPool = layer_data->Unwrap(queryPool);
    }
    layer_data->device_dispatch_table.ResetQueryPoolEXT(device, queryPool, firstQuery, queryCount);

}