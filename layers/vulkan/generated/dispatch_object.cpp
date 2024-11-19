// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See dispatch_object_generator.py for modifications

/***************************************************************************
 *
 * Copyright (c) 2015-2024 The Khronos Group Inc.
 * Copyright (c) 2015-2024 Valve Corporation
 * Copyright (c) 2015-2024 LunarG, Inc.
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

#include "chassis/dispatch_object.h"
#include "utils/cast_utils.h"
#include <vulkan/utility/vk_safe_struct.hpp>
#include "state_tracker/pipeline_state.h"
#include "containers/custom_containers.h"
#include "thread_tracker/thread_safety_validation.h"
#include "stateless/stateless_validation.h"
#include "object_tracker/object_lifetime_validation.h"
#include "core_checks/core_validation.h"
#include "best_practices/best_practices_validation.h"
#include "gpu/core/gpuav.h"
#include "sync/sync_validation.h"

#define DISPATCH_MAX_STACK_ALLOCATIONS 32

std::atomic<uint64_t> DispatchObject::global_unique_id{1};
vvl::concurrent_unordered_map<uint64_t, uint64_t, 4, HashedUint64> DispatchObject::unique_id_mapping;
bool DispatchObject::wrap_handles{true};

void DispatchObject::InitInstanceValidationObjects() {
    // Note that this DEFINES THE ORDER IN WHICH THE LAYER VALIDATION OBJECTS ARE CALLED

    if (!disabled[thread_safety]) {
        object_dispatch.emplace_back(new ThreadSafety(nullptr));
    }
    if (!disabled[stateless_checks]) {
        object_dispatch.emplace_back(new StatelessValidation);
    }
    if (!disabled[object_tracking]) {
        object_dispatch.emplace_back(new ObjectLifetimes);
    }
    if (!disabled[core_checks]) {
        object_dispatch.emplace_back(new CoreChecks);
    }
    if (enabled[best_practices]) {
        object_dispatch.emplace_back(new BestPractices);
    }
    if (enabled[gpu_validation] || enabled[debug_printf_validation]) {
        object_dispatch.emplace_back(new gpuav::Validator);
    }
    if (enabled[sync_validation]) {
        object_dispatch.emplace_back(new SyncValidator);
    }
}

void DispatchObject::InitDeviceValidationObjects(DispatchObject* instance_dispatch) {
    // Note that this DEFINES THE ORDER IN WHICH THE LAYER VALIDATION OBJECTS ARE CALLED

    if (!disabled[thread_safety]) {
        object_dispatch.emplace_back(
            new ThreadSafety(static_cast<ThreadSafety*>(instance_dispatch->GetValidationObject(LayerObjectTypeThreading))));
    }
    if (!disabled[stateless_checks]) {
        object_dispatch.emplace_back(new StatelessValidation);
    }
    if (!disabled[object_tracking]) {
        object_dispatch.emplace_back(new ObjectLifetimes);
    }
    if (!disabled[core_checks]) {
        object_dispatch.emplace_back(new CoreChecks);
    }
    if (enabled[best_practices]) {
        object_dispatch.emplace_back(new BestPractices);
    }
    if (enabled[gpu_validation] || enabled[debug_printf_validation]) {
        object_dispatch.emplace_back(new gpuav::Validator);
    }
    if (enabled[sync_validation]) {
        object_dispatch.emplace_back(new SyncValidator);
    }
}

// Unique Objects pNext extension handling function
void DispatchObject::UnwrapPnextChainHandles(const void* pNext) {
    void* cur_pnext = const_cast<void*>(pNext);
    while (cur_pnext != nullptr) {
        VkBaseOutStructure* header = reinterpret_cast<VkBaseOutStructure*>(cur_pnext);

        switch (header->sType) {
            case VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO: {
                auto* safe_struct = reinterpret_cast<vku::safe_VkComputePipelineCreateInfo*>(cur_pnext);

                if (safe_struct->stage.module) {
                    safe_struct->stage.module = Unwrap(safe_struct->stage.module);
                }
                UnwrapPnextChainHandles(safe_struct->stage.pNext);

                if (safe_struct->layout) {
                    safe_struct->layout = Unwrap(safe_struct->layout);
                }
                if (safe_struct->basePipelineHandle) {
                    safe_struct->basePipelineHandle = Unwrap(safe_struct->basePipelineHandle);
                }
            } break;
            case VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO: {
                auto* safe_struct = reinterpret_cast<vku::safe_VkGraphicsPipelineCreateInfo*>(cur_pnext);
                if (safe_struct->pStages) {
                    for (uint32_t index0 = 0; index0 < safe_struct->stageCount; ++index0) {
                        UnwrapPnextChainHandles(safe_struct->pStages[index0].pNext);

                        if (safe_struct->pStages[index0].module) {
                            safe_struct->pStages[index0].module = Unwrap(safe_struct->pStages[index0].module);
                        }
                    }
                }

                if (safe_struct->layout) {
                    safe_struct->layout = Unwrap(safe_struct->layout);
                }
                if (safe_struct->renderPass) {
                    safe_struct->renderPass = Unwrap(safe_struct->renderPass);
                }
                if (safe_struct->basePipelineHandle) {
                    safe_struct->basePipelineHandle = Unwrap(safe_struct->basePipelineHandle);
                }
            } break;
            case VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR: {
                auto* safe_struct = reinterpret_cast<vku::safe_VkRayTracingPipelineCreateInfoKHR*>(cur_pnext);
                if (safe_struct->pStages) {
                    for (uint32_t index0 = 0; index0 < safe_struct->stageCount; ++index0) {
                        UnwrapPnextChainHandles(safe_struct->pStages[index0].pNext);

                        if (safe_struct->pStages[index0].module) {
                            safe_struct->pStages[index0].module = Unwrap(safe_struct->pStages[index0].module);
                        }
                    }
                }
                if (safe_struct->pLibraryInfo) {
                    if (safe_struct->pLibraryInfo->pLibraries) {
                        for (uint32_t index1 = 0; index1 < safe_struct->pLibraryInfo->libraryCount; ++index1) {
                            safe_struct->pLibraryInfo->pLibraries[index1] = Unwrap(safe_struct->pLibraryInfo->pLibraries[index1]);
                        }
                    }
                }

                if (safe_struct->layout) {
                    safe_struct->layout = Unwrap(safe_struct->layout);
                }
                if (safe_struct->basePipelineHandle) {
                    safe_struct->basePipelineHandle = Unwrap(safe_struct->basePipelineHandle);
                }
            } break;
#ifdef VK_ENABLE_BETA_EXTENSIONS
            case VK_STRUCTURE_TYPE_EXECUTION_GRAPH_PIPELINE_CREATE_INFO_AMDX: {
                auto* safe_struct = reinterpret_cast<vku::safe_VkExecutionGraphPipelineCreateInfoAMDX*>(cur_pnext);
                if (safe_struct->pStages) {
                    for (uint32_t index0 = 0; index0 < safe_struct->stageCount; ++index0) {
                        UnwrapPnextChainHandles(safe_struct->pStages[index0].pNext);

                        if (safe_struct->pStages[index0].module) {
                            safe_struct->pStages[index0].module = Unwrap(safe_struct->pStages[index0].module);
                        }
                    }
                }
                if (safe_struct->pLibraryInfo) {
                    if (safe_struct->pLibraryInfo->pLibraries) {
                        for (uint32_t index1 = 0; index1 < safe_struct->pLibraryInfo->libraryCount; ++index1) {
                            safe_struct->pLibraryInfo->pLibraries[index1] = Unwrap(safe_struct->pLibraryInfo->pLibraries[index1]);
                        }
                    }
                }

                if (safe_struct->layout) {
                    safe_struct->layout = Unwrap(safe_struct->layout);
                }
                if (safe_struct->basePipelineHandle) {
                    safe_struct->basePipelineHandle = Unwrap(safe_struct->basePipelineHandle);
                }
            } break;
#endif  // VK_ENABLE_BETA_EXTENSIONS
            case VK_STRUCTURE_TYPE_FRAME_BOUNDARY_EXT: {
                auto* safe_struct = reinterpret_cast<vku::safe_VkFrameBoundaryEXT*>(cur_pnext);
                if (safe_struct->pImages) {
                    for (uint32_t index0 = 0; index0 < safe_struct->imageCount; ++index0) {
                        safe_struct->pImages[index0] = Unwrap(safe_struct->pImages[index0]);
                    }
                }
                if (safe_struct->pBuffers) {
                    for (uint32_t index0 = 0; index0 < safe_struct->bufferCount; ++index0) {
                        safe_struct->pBuffers[index0] = Unwrap(safe_struct->pBuffers[index0]);
                    }
                }
            } break;
#ifdef VK_USE_PLATFORM_WIN32_KHR
            case VK_STRUCTURE_TYPE_WIN32_KEYED_MUTEX_ACQUIRE_RELEASE_INFO_KHR: {
                auto* safe_struct = reinterpret_cast<vku::safe_VkWin32KeyedMutexAcquireReleaseInfoKHR*>(cur_pnext);
                if (safe_struct->pAcquireSyncs) {
                    for (uint32_t index0 = 0; index0 < safe_struct->acquireCount; ++index0) {
                        safe_struct->pAcquireSyncs[index0] = Unwrap(safe_struct->pAcquireSyncs[index0]);
                    }
                }
                if (safe_struct->pReleaseSyncs) {
                    for (uint32_t index0 = 0; index0 < safe_struct->releaseCount; ++index0) {
                        safe_struct->pReleaseSyncs[index0] = Unwrap(safe_struct->pReleaseSyncs[index0]);
                    }
                }
            } break;
            case VK_STRUCTURE_TYPE_WIN32_KEYED_MUTEX_ACQUIRE_RELEASE_INFO_NV: {
                auto* safe_struct = reinterpret_cast<vku::safe_VkWin32KeyedMutexAcquireReleaseInfoNV*>(cur_pnext);
                if (safe_struct->pAcquireSyncs) {
                    for (uint32_t index0 = 0; index0 < safe_struct->acquireCount; ++index0) {
                        safe_struct->pAcquireSyncs[index0] = Unwrap(safe_struct->pAcquireSyncs[index0]);
                    }
                }
                if (safe_struct->pReleaseSyncs) {
                    for (uint32_t index0 = 0; index0 < safe_struct->releaseCount; ++index0) {
                        safe_struct->pReleaseSyncs[index0] = Unwrap(safe_struct->pReleaseSyncs[index0]);
                    }
                }
            } break;
#endif  // VK_USE_PLATFORM_WIN32_KHR
            case VK_STRUCTURE_TYPE_DEDICATED_ALLOCATION_MEMORY_ALLOCATE_INFO_NV: {
                auto* safe_struct = reinterpret_cast<vku::safe_VkDedicatedAllocationMemoryAllocateInfoNV*>(cur_pnext);

                if (safe_struct->image) {
                    safe_struct->image = Unwrap(safe_struct->image);
                }
                if (safe_struct->buffer) {
                    safe_struct->buffer = Unwrap(safe_struct->buffer);
                }
            } break;
#ifdef VK_USE_PLATFORM_FUCHSIA
            case VK_STRUCTURE_TYPE_IMPORT_MEMORY_BUFFER_COLLECTION_FUCHSIA: {
                auto* safe_struct = reinterpret_cast<vku::safe_VkImportMemoryBufferCollectionFUCHSIA*>(cur_pnext);

                if (safe_struct->collection) {
                    safe_struct->collection = Unwrap(safe_struct->collection);
                }
            } break;
#endif  // VK_USE_PLATFORM_FUCHSIA
            case VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO: {
                auto* safe_struct = reinterpret_cast<vku::safe_VkMemoryDedicatedAllocateInfo*>(cur_pnext);

                if (safe_struct->image) {
                    safe_struct->image = Unwrap(safe_struct->image);
                }
                if (safe_struct->buffer) {
                    safe_struct->buffer = Unwrap(safe_struct->buffer);
                }
            } break;
#ifdef VK_USE_PLATFORM_FUCHSIA
            case VK_STRUCTURE_TYPE_BUFFER_COLLECTION_BUFFER_CREATE_INFO_FUCHSIA: {
                auto* safe_struct = reinterpret_cast<vku::safe_VkBufferCollectionBufferCreateInfoFUCHSIA*>(cur_pnext);

                if (safe_struct->collection) {
                    safe_struct->collection = Unwrap(safe_struct->collection);
                }
            } break;
            case VK_STRUCTURE_TYPE_BUFFER_COLLECTION_IMAGE_CREATE_INFO_FUCHSIA: {
                auto* safe_struct = reinterpret_cast<vku::safe_VkBufferCollectionImageCreateInfoFUCHSIA*>(cur_pnext);

                if (safe_struct->collection) {
                    safe_struct->collection = Unwrap(safe_struct->collection);
                }
            } break;
#endif  // VK_USE_PLATFORM_FUCHSIA
            case VK_STRUCTURE_TYPE_IMAGE_SWAPCHAIN_CREATE_INFO_KHR: {
                auto* safe_struct = reinterpret_cast<vku::safe_VkImageSwapchainCreateInfoKHR*>(cur_pnext);

                if (safe_struct->swapchain) {
                    safe_struct->swapchain = Unwrap(safe_struct->swapchain);
                }
            } break;
            case VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_INFO: {
                auto* safe_struct = reinterpret_cast<vku::safe_VkSamplerYcbcrConversionInfo*>(cur_pnext);

                if (safe_struct->conversion) {
                    safe_struct->conversion = Unwrap(safe_struct->conversion);
                }
            } break;
            case VK_STRUCTURE_TYPE_SHADER_MODULE_VALIDATION_CACHE_CREATE_INFO_EXT: {
                auto* safe_struct = reinterpret_cast<vku::safe_VkShaderModuleValidationCacheCreateInfoEXT*>(cur_pnext);

                if (safe_struct->validationCache) {
                    safe_struct->validationCache = Unwrap(safe_struct->validationCache);
                }
            } break;
            case VK_STRUCTURE_TYPE_PIPELINE_BINARY_INFO_KHR: {
                auto* safe_struct = reinterpret_cast<vku::safe_VkPipelineBinaryInfoKHR*>(cur_pnext);
                if (safe_struct->pPipelineBinaries) {
                    for (uint32_t index0 = 0; index0 < safe_struct->binaryCount; ++index0) {
                        safe_struct->pPipelineBinaries[index0] = Unwrap(safe_struct->pPipelineBinaries[index0]);
                    }
                }
            } break;
            case VK_STRUCTURE_TYPE_SUBPASS_SHADING_PIPELINE_CREATE_INFO_HUAWEI: {
                auto* safe_struct = reinterpret_cast<vku::safe_VkSubpassShadingPipelineCreateInfoHUAWEI*>(cur_pnext);

                if (safe_struct->renderPass) {
                    safe_struct->renderPass = Unwrap(safe_struct->renderPass);
                }
            } break;
            case VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_SHADER_GROUPS_CREATE_INFO_NV: {
                auto* safe_struct = reinterpret_cast<vku::safe_VkGraphicsPipelineShaderGroupsCreateInfoNV*>(cur_pnext);
                if (safe_struct->pGroups) {
                    for (uint32_t index0 = 0; index0 < safe_struct->groupCount; ++index0) {
                        if (safe_struct->pGroups[index0].pStages) {
                            for (uint32_t index1 = 0; index1 < safe_struct->pGroups[index0].stageCount; ++index1) {
                                UnwrapPnextChainHandles(safe_struct->pGroups[index0].pStages[index1].pNext);

                                if (safe_struct->pGroups[index0].pStages[index1].module) {
                                    safe_struct->pGroups[index0].pStages[index1].module =
                                        Unwrap(safe_struct->pGroups[index0].pStages[index1].module);
                                }
                            }
                        }
                    }
                }
                if (safe_struct->pPipelines) {
                    for (uint32_t index0 = 0; index0 < safe_struct->pipelineCount; ++index0) {
                        safe_struct->pPipelines[index0] = Unwrap(safe_struct->pPipelines[index0]);
                    }
                }
            } break;
            case VK_STRUCTURE_TYPE_PIPELINE_LIBRARY_CREATE_INFO_KHR: {
                auto* safe_struct = reinterpret_cast<vku::safe_VkPipelineLibraryCreateInfoKHR*>(cur_pnext);
                if (safe_struct->pLibraries) {
                    for (uint32_t index0 = 0; index0 < safe_struct->libraryCount; ++index0) {
                        safe_struct->pLibraries[index0] = Unwrap(safe_struct->pLibraries[index0]);
                    }
                }
            } break;
            case VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR: {
                auto* safe_struct = reinterpret_cast<vku::safe_VkWriteDescriptorSetAccelerationStructureKHR*>(cur_pnext);
                if (safe_struct->pAccelerationStructures) {
                    for (uint32_t index0 = 0; index0 < safe_struct->accelerationStructureCount; ++index0) {
                        safe_struct->pAccelerationStructures[index0] = Unwrap(safe_struct->pAccelerationStructures[index0]);
                    }
                }
            } break;
            case VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_NV: {
                auto* safe_struct = reinterpret_cast<vku::safe_VkWriteDescriptorSetAccelerationStructureNV*>(cur_pnext);
                if (safe_struct->pAccelerationStructures) {
                    for (uint32_t index0 = 0; index0 < safe_struct->accelerationStructureCount; ++index0) {
                        safe_struct->pAccelerationStructures[index0] = Unwrap(safe_struct->pAccelerationStructures[index0]);
                    }
                }
            } break;
            case VK_STRUCTURE_TYPE_RENDER_PASS_ATTACHMENT_BEGIN_INFO: {
                auto* safe_struct = reinterpret_cast<vku::safe_VkRenderPassAttachmentBeginInfo*>(cur_pnext);
                if (safe_struct->pAttachments) {
                    for (uint32_t index0 = 0; index0 < safe_struct->attachmentCount; ++index0) {
                        safe_struct->pAttachments[index0] = Unwrap(safe_struct->pAttachments[index0]);
                    }
                }
            } break;
            case VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_SWAPCHAIN_INFO_KHR: {
                auto* safe_struct = reinterpret_cast<vku::safe_VkBindImageMemorySwapchainInfoKHR*>(cur_pnext);

                if (safe_struct->swapchain) {
                    safe_struct->swapchain = Unwrap(safe_struct->swapchain);
                }
            } break;
            case VK_STRUCTURE_TYPE_RENDER_PASS_STRIPE_SUBMIT_INFO_ARM: {
                auto* safe_struct = reinterpret_cast<vku::safe_VkRenderPassStripeSubmitInfoARM*>(cur_pnext);
                if (safe_struct->pStripeSemaphoreInfos) {
                    for (uint32_t index0 = 0; index0 < safe_struct->stripeSemaphoreInfoCount; ++index0) {
                        if (safe_struct->pStripeSemaphoreInfos[index0].semaphore) {
                            safe_struct->pStripeSemaphoreInfos[index0].semaphore =
                                Unwrap(safe_struct->pStripeSemaphoreInfos[index0].semaphore);
                        }
                    }
                }
            } break;
            case VK_STRUCTURE_TYPE_RENDERING_FRAGMENT_DENSITY_MAP_ATTACHMENT_INFO_EXT: {
                auto* safe_struct = reinterpret_cast<vku::safe_VkRenderingFragmentDensityMapAttachmentInfoEXT*>(cur_pnext);

                if (safe_struct->imageView) {
                    safe_struct->imageView = Unwrap(safe_struct->imageView);
                }
            } break;
            case VK_STRUCTURE_TYPE_RENDERING_FRAGMENT_SHADING_RATE_ATTACHMENT_INFO_KHR: {
                auto* safe_struct = reinterpret_cast<vku::safe_VkRenderingFragmentShadingRateAttachmentInfoKHR*>(cur_pnext);

                if (safe_struct->imageView) {
                    safe_struct->imageView = Unwrap(safe_struct->imageView);
                }
            } break;
            case VK_STRUCTURE_TYPE_SWAPCHAIN_PRESENT_FENCE_INFO_EXT: {
                auto* safe_struct = reinterpret_cast<vku::safe_VkSwapchainPresentFenceInfoEXT*>(cur_pnext);
                if (safe_struct->pFences) {
                    for (uint32_t index0 = 0; index0 < safe_struct->swapchainCount; ++index0) {
                        safe_struct->pFences[index0] = Unwrap(safe_struct->pFences[index0]);
                    }
                }
            } break;
            case VK_STRUCTURE_TYPE_VIDEO_INLINE_QUERY_INFO_KHR: {
                auto* safe_struct = reinterpret_cast<vku::safe_VkVideoInlineQueryInfoKHR*>(cur_pnext);

                if (safe_struct->queryPool) {
                    safe_struct->queryPool = Unwrap(safe_struct->queryPool);
                }
            } break;
            case VK_STRUCTURE_TYPE_VIDEO_ENCODE_QUANTIZATION_MAP_INFO_KHR: {
                auto* safe_struct = reinterpret_cast<vku::safe_VkVideoEncodeQuantizationMapInfoKHR*>(cur_pnext);

                if (safe_struct->quantizationMap) {
                    safe_struct->quantizationMap = Unwrap(safe_struct->quantizationMap);
                }
            } break;
            case VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO: {
                auto* safe_struct = reinterpret_cast<vku::safe_VkPipelineLayoutCreateInfo*>(cur_pnext);
                if (safe_struct->pSetLayouts) {
                    for (uint32_t index0 = 0; index0 < safe_struct->setLayoutCount; ++index0) {
                        safe_struct->pSetLayouts[index0] = Unwrap(safe_struct->pSetLayouts[index0]);
                    }
                }
            } break;
#ifdef VK_USE_PLATFORM_METAL_EXT
            case VK_STRUCTURE_TYPE_EXPORT_METAL_BUFFER_INFO_EXT: {
                auto* safe_struct = reinterpret_cast<vku::safe_VkExportMetalBufferInfoEXT*>(cur_pnext);

                if (safe_struct->memory) {
                    safe_struct->memory = Unwrap(safe_struct->memory);
                }
            } break;
            case VK_STRUCTURE_TYPE_EXPORT_METAL_IO_SURFACE_INFO_EXT: {
                auto* safe_struct = reinterpret_cast<vku::safe_VkExportMetalIOSurfaceInfoEXT*>(cur_pnext);

                if (safe_struct->image) {
                    safe_struct->image = Unwrap(safe_struct->image);
                }
            } break;
            case VK_STRUCTURE_TYPE_EXPORT_METAL_SHARED_EVENT_INFO_EXT: {
                auto* safe_struct = reinterpret_cast<vku::safe_VkExportMetalSharedEventInfoEXT*>(cur_pnext);

                if (safe_struct->semaphore) {
                    safe_struct->semaphore = Unwrap(safe_struct->semaphore);
                }
                if (safe_struct->event) {
                    safe_struct->event = Unwrap(safe_struct->event);
                }
            } break;
            case VK_STRUCTURE_TYPE_EXPORT_METAL_TEXTURE_INFO_EXT: {
                auto* safe_struct = reinterpret_cast<vku::safe_VkExportMetalTextureInfoEXT*>(cur_pnext);

                if (safe_struct->image) {
                    safe_struct->image = Unwrap(safe_struct->image);
                }
                if (safe_struct->imageView) {
                    safe_struct->imageView = Unwrap(safe_struct->imageView);
                }
                if (safe_struct->bufferView) {
                    safe_struct->bufferView = Unwrap(safe_struct->bufferView);
                }
            } break;
#endif  // VK_USE_PLATFORM_METAL_EXT
            case VK_STRUCTURE_TYPE_DESCRIPTOR_BUFFER_BINDING_PUSH_DESCRIPTOR_BUFFER_HANDLE_EXT: {
                auto* safe_struct = reinterpret_cast<vku::safe_VkDescriptorBufferBindingPushDescriptorBufferHandleEXT*>(cur_pnext);

                if (safe_struct->buffer) {
                    safe_struct->buffer = Unwrap(safe_struct->buffer);
                }
            } break;
            case VK_STRUCTURE_TYPE_GENERATED_COMMANDS_PIPELINE_INFO_EXT: {
                auto* safe_struct = reinterpret_cast<vku::safe_VkGeneratedCommandsPipelineInfoEXT*>(cur_pnext);

                if (safe_struct->pipeline) {
                    safe_struct->pipeline = Unwrap(safe_struct->pipeline);
                }
            } break;
            case VK_STRUCTURE_TYPE_GENERATED_COMMANDS_SHADER_INFO_EXT: {
                auto* safe_struct = reinterpret_cast<vku::safe_VkGeneratedCommandsShaderInfoEXT*>(cur_pnext);
                if (safe_struct->pShaders) {
                    for (uint32_t index0 = 0; index0 < safe_struct->shaderCount; ++index0) {
                        safe_struct->pShaders[index0] = Unwrap(safe_struct->pShaders[index0]);
                    }
                }
            } break;
#ifdef VK_ENABLE_BETA_EXTENSIONS
            case VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_TRIANGLES_DISPLACEMENT_MICROMAP_NV: {
                auto* safe_struct = reinterpret_cast<vku::safe_VkAccelerationStructureTrianglesDisplacementMicromapNV*>(cur_pnext);

                if (safe_struct->micromap) {
                    safe_struct->micromap = Unwrap(safe_struct->micromap);
                }
            } break;
#endif  // VK_ENABLE_BETA_EXTENSIONS
            case VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_TRIANGLES_OPACITY_MICROMAP_EXT: {
                auto* safe_struct = reinterpret_cast<vku::safe_VkAccelerationStructureTrianglesOpacityMicromapEXT*>(cur_pnext);

                if (safe_struct->micromap) {
                    safe_struct->micromap = Unwrap(safe_struct->micromap);
                }
            } break;

            default:
                break;
        }

        // Process the next structure in the chain
        cur_pnext = header->pNext;
    }
}

[[maybe_unused]] static bool NotDispatchableHandle(VkObjectType object_type) {
    switch (object_type) {
        case VK_OBJECT_TYPE_INSTANCE:
        case VK_OBJECT_TYPE_PHYSICAL_DEVICE:
        case VK_OBJECT_TYPE_DEVICE:
        case VK_OBJECT_TYPE_QUEUE:
        case VK_OBJECT_TYPE_COMMAND_BUFFER:
            return false;
        default:
            return true;
    }
}

VkResult DispatchObject::EnumeratePhysicalDevices(VkInstance instance, uint32_t* pPhysicalDeviceCount,
                                                  VkPhysicalDevice* pPhysicalDevices) {
    VkResult result = instance_dispatch_table.EnumeratePhysicalDevices(instance, pPhysicalDeviceCount, pPhysicalDevices);

    return result;
}

void DispatchObject::GetPhysicalDeviceFeatures(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures* pFeatures) {
    instance_dispatch_table.GetPhysicalDeviceFeatures(physicalDevice, pFeatures);
}

void DispatchObject::GetPhysicalDeviceFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format,
                                                       VkFormatProperties* pFormatProperties) {
    instance_dispatch_table.GetPhysicalDeviceFormatProperties(physicalDevice, format, pFormatProperties);
}

VkResult DispatchObject::GetPhysicalDeviceImageFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type,
                                                                VkImageTiling tiling, VkImageUsageFlags usage,
                                                                VkImageCreateFlags flags,
                                                                VkImageFormatProperties* pImageFormatProperties) {
    VkResult result = instance_dispatch_table.GetPhysicalDeviceImageFormatProperties(physicalDevice, format, type, tiling, usage,
                                                                                     flags, pImageFormatProperties);

    return result;
}

void DispatchObject::GetPhysicalDeviceProperties(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties* pProperties) {
    instance_dispatch_table.GetPhysicalDeviceProperties(physicalDevice, pProperties);
}

void DispatchObject::GetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount,
                                                            VkQueueFamilyProperties* pQueueFamilyProperties) {
    instance_dispatch_table.GetPhysicalDeviceQueueFamilyProperties(physicalDevice, pQueueFamilyPropertyCount,
                                                                   pQueueFamilyProperties);
}

void DispatchObject::GetPhysicalDeviceMemoryProperties(VkPhysicalDevice physicalDevice,
                                                       VkPhysicalDeviceMemoryProperties* pMemoryProperties) {
    instance_dispatch_table.GetPhysicalDeviceMemoryProperties(physicalDevice, pMemoryProperties);
}

PFN_vkVoidFunction DispatchObject::GetInstanceProcAddr(VkInstance instance, const char* pName) {
    PFN_vkVoidFunction result = instance_dispatch_table.GetInstanceProcAddr(instance, pName);

    return result;
}

PFN_vkVoidFunction DispatchObject::GetDeviceProcAddr(VkDevice device, const char* pName) {
    PFN_vkVoidFunction result = device_dispatch_table.GetDeviceProcAddr(device, pName);

    return result;
}

VkResult DispatchObject::EnumerateDeviceExtensionProperties(VkPhysicalDevice physicalDevice, const char* pLayerName,
                                                            uint32_t* pPropertyCount, VkExtensionProperties* pProperties) {
    VkResult result =
        instance_dispatch_table.EnumerateDeviceExtensionProperties(physicalDevice, pLayerName, pPropertyCount, pProperties);

    return result;
}

void DispatchObject::GetDeviceQueue(VkDevice device, uint32_t queueFamilyIndex, uint32_t queueIndex, VkQueue* pQueue) {
    device_dispatch_table.GetDeviceQueue(device, queueFamilyIndex, queueIndex, pQueue);
}

VkResult DispatchObject::QueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence) {
    if (!wrap_handles) return device_dispatch_table.QueueSubmit(queue, submitCount, pSubmits, fence);
    small_vector<vku::safe_VkSubmitInfo, DISPATCH_MAX_STACK_ALLOCATIONS> var_local_pSubmits;
    vku::safe_VkSubmitInfo* local_pSubmits = nullptr;
    {
        if (pSubmits) {
            var_local_pSubmits.resize(submitCount);
            local_pSubmits = var_local_pSubmits.data();
            for (uint32_t index0 = 0; index0 < submitCount; ++index0) {
                local_pSubmits[index0].initialize(&pSubmits[index0]);
                UnwrapPnextChainHandles(local_pSubmits[index0].pNext);
                if (local_pSubmits[index0].pWaitSemaphores) {
                    for (uint32_t index1 = 0; index1 < local_pSubmits[index0].waitSemaphoreCount; ++index1) {
                        local_pSubmits[index0].pWaitSemaphores[index1] = Unwrap(local_pSubmits[index0].pWaitSemaphores[index1]);
                    }
                }
                if (local_pSubmits[index0].pSignalSemaphores) {
                    for (uint32_t index1 = 0; index1 < local_pSubmits[index0].signalSemaphoreCount; ++index1) {
                        local_pSubmits[index0].pSignalSemaphores[index1] = Unwrap(local_pSubmits[index0].pSignalSemaphores[index1]);
                    }
                }
            }
        }
        fence = Unwrap(fence);
    }
    VkResult result = device_dispatch_table.QueueSubmit(queue, submitCount, (const VkSubmitInfo*)local_pSubmits, fence);

    return result;
}

VkResult DispatchObject::QueueWaitIdle(VkQueue queue) {
    VkResult result = device_dispatch_table.QueueWaitIdle(queue);

    return result;
}

VkResult DispatchObject::DeviceWaitIdle(VkDevice device) {
    VkResult result = device_dispatch_table.DeviceWaitIdle(device);

    return result;
}

VkResult DispatchObject::AllocateMemory(VkDevice device, const VkMemoryAllocateInfo* pAllocateInfo,
                                        const VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory) {
    if (!wrap_handles) return device_dispatch_table.AllocateMemory(device, pAllocateInfo, pAllocator, pMemory);
    vku::safe_VkMemoryAllocateInfo var_local_pAllocateInfo;
    vku::safe_VkMemoryAllocateInfo* local_pAllocateInfo = nullptr;
    {
        if (pAllocateInfo) {
            local_pAllocateInfo = &var_local_pAllocateInfo;
            local_pAllocateInfo->initialize(pAllocateInfo);
            UnwrapPnextChainHandles(local_pAllocateInfo->pNext);
        }
    }
    VkResult result =
        device_dispatch_table.AllocateMemory(device, (const VkMemoryAllocateInfo*)local_pAllocateInfo, pAllocator, pMemory);
    if (VK_SUCCESS == result) {
        *pMemory = WrapNew(*pMemory);
    }
    return result;
}

void DispatchObject::FreeMemory(VkDevice device, VkDeviceMemory memory, const VkAllocationCallbacks* pAllocator) {
    if (!wrap_handles) return device_dispatch_table.FreeMemory(device, memory, pAllocator);

    uint64_t memory_id = CastToUint64(memory);
    auto iter = unique_id_mapping.pop(memory_id);
    if (iter != unique_id_mapping.end()) {
        memory = (VkDeviceMemory)iter->second;
    } else {
        memory = (VkDeviceMemory)0;
    }
    device_dispatch_table.FreeMemory(device, memory, pAllocator);
}

VkResult DispatchObject::MapMemory(VkDevice device, VkDeviceMemory memory, VkDeviceSize offset, VkDeviceSize size,
                                   VkMemoryMapFlags flags, void** ppData) {
    if (!wrap_handles) return device_dispatch_table.MapMemory(device, memory, offset, size, flags, ppData);
    { memory = Unwrap(memory); }
    VkResult result = device_dispatch_table.MapMemory(device, memory, offset, size, flags, ppData);

    return result;
}

void DispatchObject::UnmapMemory(VkDevice device, VkDeviceMemory memory) {
    if (!wrap_handles) return device_dispatch_table.UnmapMemory(device, memory);
    { memory = Unwrap(memory); }
    device_dispatch_table.UnmapMemory(device, memory);
}

VkResult DispatchObject::FlushMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount,
                                                 const VkMappedMemoryRange* pMemoryRanges) {
    if (!wrap_handles) return device_dispatch_table.FlushMappedMemoryRanges(device, memoryRangeCount, pMemoryRanges);
    small_vector<vku::safe_VkMappedMemoryRange, DISPATCH_MAX_STACK_ALLOCATIONS> var_local_pMemoryRanges;
    vku::safe_VkMappedMemoryRange* local_pMemoryRanges = nullptr;
    {
        if (pMemoryRanges) {
            var_local_pMemoryRanges.resize(memoryRangeCount);
            local_pMemoryRanges = var_local_pMemoryRanges.data();
            for (uint32_t index0 = 0; index0 < memoryRangeCount; ++index0) {
                local_pMemoryRanges[index0].initialize(&pMemoryRanges[index0]);

                if (pMemoryRanges[index0].memory) {
                    local_pMemoryRanges[index0].memory = Unwrap(pMemoryRanges[index0].memory);
                }
            }
        }
    }
    VkResult result =
        device_dispatch_table.FlushMappedMemoryRanges(device, memoryRangeCount, (const VkMappedMemoryRange*)local_pMemoryRanges);

    return result;
}

VkResult DispatchObject::InvalidateMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount,
                                                      const VkMappedMemoryRange* pMemoryRanges) {
    if (!wrap_handles) return device_dispatch_table.InvalidateMappedMemoryRanges(device, memoryRangeCount, pMemoryRanges);
    small_vector<vku::safe_VkMappedMemoryRange, DISPATCH_MAX_STACK_ALLOCATIONS> var_local_pMemoryRanges;
    vku::safe_VkMappedMemoryRange* local_pMemoryRanges = nullptr;
    {
        if (pMemoryRanges) {
            var_local_pMemoryRanges.resize(memoryRangeCount);
            local_pMemoryRanges = var_local_pMemoryRanges.data();
            for (uint32_t index0 = 0; index0 < memoryRangeCount; ++index0) {
                local_pMemoryRanges[index0].initialize(&pMemoryRanges[index0]);

                if (pMemoryRanges[index0].memory) {
                    local_pMemoryRanges[index0].memory = Unwrap(pMemoryRanges[index0].memory);
                }
            }
        }
    }
    VkResult result = device_dispatch_table.InvalidateMappedMemoryRanges(device, memoryRangeCount,
                                                                         (const VkMappedMemoryRange*)local_pMemoryRanges);

    return result;
}

void DispatchObject::GetDeviceMemoryCommitment(VkDevice device, VkDeviceMemory memory, VkDeviceSize* pCommittedMemoryInBytes) {
    if (!wrap_handles) return device_dispatch_table.GetDeviceMemoryCommitment(device, memory, pCommittedMemoryInBytes);
    { memory = Unwrap(memory); }
    device_dispatch_table.GetDeviceMemoryCommitment(device, memory, pCommittedMemoryInBytes);
}

VkResult DispatchObject::BindBufferMemory(VkDevice device, VkBuffer buffer, VkDeviceMemory memory, VkDeviceSize memoryOffset) {
    if (!wrap_handles) return device_dispatch_table.BindBufferMemory(device, buffer, memory, memoryOffset);
    {
        buffer = Unwrap(buffer);
        memory = Unwrap(memory);
    }
    VkResult result = device_dispatch_table.BindBufferMemory(device, buffer, memory, memoryOffset);

    return result;
}

VkResult DispatchObject::BindImageMemory(VkDevice device, VkImage image, VkDeviceMemory memory, VkDeviceSize memoryOffset) {
    if (!wrap_handles) return device_dispatch_table.BindImageMemory(device, image, memory, memoryOffset);
    {
        image = Unwrap(image);
        memory = Unwrap(memory);
    }
    VkResult result = device_dispatch_table.BindImageMemory(device, image, memory, memoryOffset);

    return result;
}

void DispatchObject::GetBufferMemoryRequirements(VkDevice device, VkBuffer buffer, VkMemoryRequirements* pMemoryRequirements) {
    if (!wrap_handles) return device_dispatch_table.GetBufferMemoryRequirements(device, buffer, pMemoryRequirements);
    { buffer = Unwrap(buffer); }
    device_dispatch_table.GetBufferMemoryRequirements(device, buffer, pMemoryRequirements);
}

void DispatchObject::GetImageMemoryRequirements(VkDevice device, VkImage image, VkMemoryRequirements* pMemoryRequirements) {
    if (!wrap_handles) return device_dispatch_table.GetImageMemoryRequirements(device, image, pMemoryRequirements);
    { image = Unwrap(image); }
    device_dispatch_table.GetImageMemoryRequirements(device, image, pMemoryRequirements);
}

void DispatchObject::GetImageSparseMemoryRequirements(VkDevice device, VkImage image, uint32_t* pSparseMemoryRequirementCount,
                                                      VkSparseImageMemoryRequirements* pSparseMemoryRequirements) {
    if (!wrap_handles)
        return device_dispatch_table.GetImageSparseMemoryRequirements(device, image, pSparseMemoryRequirementCount,
                                                                      pSparseMemoryRequirements);
    { image = Unwrap(image); }
    device_dispatch_table.GetImageSparseMemoryRequirements(device, image, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
}

void DispatchObject::GetPhysicalDeviceSparseImageFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format,
                                                                  VkImageType type, VkSampleCountFlagBits samples,
                                                                  VkImageUsageFlags usage, VkImageTiling tiling,
                                                                  uint32_t* pPropertyCount,
                                                                  VkSparseImageFormatProperties* pProperties) {
    instance_dispatch_table.GetPhysicalDeviceSparseImageFormatProperties(physicalDevice, format, type, samples, usage, tiling,
                                                                         pPropertyCount, pProperties);
}

VkResult DispatchObject::QueueBindSparse(VkQueue queue, uint32_t bindInfoCount, const VkBindSparseInfo* pBindInfo, VkFence fence) {
    if (!wrap_handles) return device_dispatch_table.QueueBindSparse(queue, bindInfoCount, pBindInfo, fence);
    small_vector<vku::safe_VkBindSparseInfo, DISPATCH_MAX_STACK_ALLOCATIONS> var_local_pBindInfo;
    vku::safe_VkBindSparseInfo* local_pBindInfo = nullptr;
    {
        if (pBindInfo) {
            var_local_pBindInfo.resize(bindInfoCount);
            local_pBindInfo = var_local_pBindInfo.data();
            for (uint32_t index0 = 0; index0 < bindInfoCount; ++index0) {
                local_pBindInfo[index0].initialize(&pBindInfo[index0]);
                UnwrapPnextChainHandles(local_pBindInfo[index0].pNext);
                if (local_pBindInfo[index0].pWaitSemaphores) {
                    for (uint32_t index1 = 0; index1 < local_pBindInfo[index0].waitSemaphoreCount; ++index1) {
                        local_pBindInfo[index0].pWaitSemaphores[index1] = Unwrap(local_pBindInfo[index0].pWaitSemaphores[index1]);
                    }
                }
                if (local_pBindInfo[index0].pBufferBinds) {
                    for (uint32_t index1 = 0; index1 < local_pBindInfo[index0].bufferBindCount; ++index1) {
                        if (pBindInfo[index0].pBufferBinds[index1].buffer) {
                            local_pBindInfo[index0].pBufferBinds[index1].buffer =
                                Unwrap(pBindInfo[index0].pBufferBinds[index1].buffer);
                        }
                        if (local_pBindInfo[index0].pBufferBinds[index1].pBinds) {
                            for (uint32_t index2 = 0; index2 < local_pBindInfo[index0].pBufferBinds[index1].bindCount; ++index2) {
                                if (pBindInfo[index0].pBufferBinds[index1].pBinds[index2].memory) {
                                    local_pBindInfo[index0].pBufferBinds[index1].pBinds[index2].memory =
                                        Unwrap(pBindInfo[index0].pBufferBinds[index1].pBinds[index2].memory);
                                }
                            }
                        }
                    }
                }
                if (local_pBindInfo[index0].pImageOpaqueBinds) {
                    for (uint32_t index1 = 0; index1 < local_pBindInfo[index0].imageOpaqueBindCount; ++index1) {
                        if (pBindInfo[index0].pImageOpaqueBinds[index1].image) {
                            local_pBindInfo[index0].pImageOpaqueBinds[index1].image =
                                Unwrap(pBindInfo[index0].pImageOpaqueBinds[index1].image);
                        }
                        if (local_pBindInfo[index0].pImageOpaqueBinds[index1].pBinds) {
                            for (uint32_t index2 = 0; index2 < local_pBindInfo[index0].pImageOpaqueBinds[index1].bindCount;
                                 ++index2) {
                                if (pBindInfo[index0].pImageOpaqueBinds[index1].pBinds[index2].memory) {
                                    local_pBindInfo[index0].pImageOpaqueBinds[index1].pBinds[index2].memory =
                                        Unwrap(pBindInfo[index0].pImageOpaqueBinds[index1].pBinds[index2].memory);
                                }
                            }
                        }
                    }
                }
                if (local_pBindInfo[index0].pImageBinds) {
                    for (uint32_t index1 = 0; index1 < local_pBindInfo[index0].imageBindCount; ++index1) {
                        if (pBindInfo[index0].pImageBinds[index1].image) {
                            local_pBindInfo[index0].pImageBinds[index1].image = Unwrap(pBindInfo[index0].pImageBinds[index1].image);
                        }
                        if (local_pBindInfo[index0].pImageBinds[index1].pBinds) {
                            for (uint32_t index2 = 0; index2 < local_pBindInfo[index0].pImageBinds[index1].bindCount; ++index2) {
                                if (pBindInfo[index0].pImageBinds[index1].pBinds[index2].memory) {
                                    local_pBindInfo[index0].pImageBinds[index1].pBinds[index2].memory =
                                        Unwrap(pBindInfo[index0].pImageBinds[index1].pBinds[index2].memory);
                                }
                            }
                        }
                    }
                }
                if (local_pBindInfo[index0].pSignalSemaphores) {
                    for (uint32_t index1 = 0; index1 < local_pBindInfo[index0].signalSemaphoreCount; ++index1) {
                        local_pBindInfo[index0].pSignalSemaphores[index1] =
                            Unwrap(local_pBindInfo[index0].pSignalSemaphores[index1]);
                    }
                }
            }
        }
        fence = Unwrap(fence);
    }
    VkResult result = device_dispatch_table.QueueBindSparse(queue, bindInfoCount, (const VkBindSparseInfo*)local_pBindInfo, fence);

    return result;
}

VkResult DispatchObject::CreateFence(VkDevice device, const VkFenceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                     VkFence* pFence) {
    if (!wrap_handles) return device_dispatch_table.CreateFence(device, pCreateInfo, pAllocator, pFence);

    VkResult result = device_dispatch_table.CreateFence(device, pCreateInfo, pAllocator, pFence);
    if (VK_SUCCESS == result) {
        *pFence = WrapNew(*pFence);
    }
    return result;
}

void DispatchObject::DestroyFence(VkDevice device, VkFence fence, const VkAllocationCallbacks* pAllocator) {
    if (!wrap_handles) return device_dispatch_table.DestroyFence(device, fence, pAllocator);

    uint64_t fence_id = CastToUint64(fence);
    auto iter = unique_id_mapping.pop(fence_id);
    if (iter != unique_id_mapping.end()) {
        fence = (VkFence)iter->second;
    } else {
        fence = (VkFence)0;
    }
    device_dispatch_table.DestroyFence(device, fence, pAllocator);
}

VkResult DispatchObject::ResetFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences) {
    if (!wrap_handles) return device_dispatch_table.ResetFences(device, fenceCount, pFences);
    small_vector<VkFence, DISPATCH_MAX_STACK_ALLOCATIONS> var_local_pFences;
    VkFence* local_pFences = nullptr;
    {
        if (pFences) {
            var_local_pFences.resize(fenceCount);
            local_pFences = var_local_pFences.data();
            for (uint32_t index0 = 0; index0 < fenceCount; ++index0) {
                local_pFences[index0] = Unwrap(pFences[index0]);
            }
        }
    }
    VkResult result = device_dispatch_table.ResetFences(device, fenceCount, (const VkFence*)local_pFences);

    return result;
}

VkResult DispatchObject::GetFenceStatus(VkDevice device, VkFence fence) {
    if (!wrap_handles) return device_dispatch_table.GetFenceStatus(device, fence);
    { fence = Unwrap(fence); }
    VkResult result = device_dispatch_table.GetFenceStatus(device, fence);

    return result;
}

VkResult DispatchObject::WaitForFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences, VkBool32 waitAll,
                                       uint64_t timeout) {
    if (!wrap_handles) return device_dispatch_table.WaitForFences(device, fenceCount, pFences, waitAll, timeout);
    small_vector<VkFence, DISPATCH_MAX_STACK_ALLOCATIONS> var_local_pFences;
    VkFence* local_pFences = nullptr;
    {
        if (pFences) {
            var_local_pFences.resize(fenceCount);
            local_pFences = var_local_pFences.data();
            for (uint32_t index0 = 0; index0 < fenceCount; ++index0) {
                local_pFences[index0] = Unwrap(pFences[index0]);
            }
        }
    }
    VkResult result = device_dispatch_table.WaitForFences(device, fenceCount, (const VkFence*)local_pFences, waitAll, timeout);

    return result;
}

VkResult DispatchObject::CreateSemaphore(VkDevice device, const VkSemaphoreCreateInfo* pCreateInfo,
                                         const VkAllocationCallbacks* pAllocator, VkSemaphore* pSemaphore) {
    if (!wrap_handles) return device_dispatch_table.CreateSemaphore(device, pCreateInfo, pAllocator, pSemaphore);

    VkResult result = device_dispatch_table.CreateSemaphore(device, pCreateInfo, pAllocator, pSemaphore);
    if (VK_SUCCESS == result) {
        *pSemaphore = WrapNew(*pSemaphore);
    }
    return result;
}

void DispatchObject::DestroySemaphore(VkDevice device, VkSemaphore semaphore, const VkAllocationCallbacks* pAllocator) {
    if (!wrap_handles) return device_dispatch_table.DestroySemaphore(device, semaphore, pAllocator);

    uint64_t semaphore_id = CastToUint64(semaphore);
    auto iter = unique_id_mapping.pop(semaphore_id);
    if (iter != unique_id_mapping.end()) {
        semaphore = (VkSemaphore)iter->second;
    } else {
        semaphore = (VkSemaphore)0;
    }
    device_dispatch_table.DestroySemaphore(device, semaphore, pAllocator);
}

VkResult DispatchObject::CreateEvent(VkDevice device, const VkEventCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                     VkEvent* pEvent) {
    if (!wrap_handles) return device_dispatch_table.CreateEvent(device, pCreateInfo, pAllocator, pEvent);

    VkResult result = device_dispatch_table.CreateEvent(device, pCreateInfo, pAllocator, pEvent);
    if (VK_SUCCESS == result) {
        *pEvent = WrapNew(*pEvent);
    }
    return result;
}

void DispatchObject::DestroyEvent(VkDevice device, VkEvent event, const VkAllocationCallbacks* pAllocator) {
    if (!wrap_handles) return device_dispatch_table.DestroyEvent(device, event, pAllocator);

    uint64_t event_id = CastToUint64(event);
    auto iter = unique_id_mapping.pop(event_id);
    if (iter != unique_id_mapping.end()) {
        event = (VkEvent)iter->second;
    } else {
        event = (VkEvent)0;
    }
    device_dispatch_table.DestroyEvent(device, event, pAllocator);
}

VkResult DispatchObject::GetEventStatus(VkDevice device, VkEvent event) {
    if (!wrap_handles) return device_dispatch_table.GetEventStatus(device, event);
    { event = Unwrap(event); }
    VkResult result = device_dispatch_table.GetEventStatus(device, event);

    return result;
}

VkResult DispatchObject::SetEvent(VkDevice device, VkEvent event) {
    if (!wrap_handles) return device_dispatch_table.SetEvent(device, event);
    { event = Unwrap(event); }
    VkResult result = device_dispatch_table.SetEvent(device, event);

    return result;
}

VkResult DispatchObject::ResetEvent(VkDevice device, VkEvent event) {
    if (!wrap_handles) return device_dispatch_table.ResetEvent(device, event);
    { event = Unwrap(event); }
    VkResult result = device_dispatch_table.ResetEvent(device, event);

    return result;
}

VkResult DispatchObject::CreateQueryPool(VkDevice device, const VkQueryPoolCreateInfo* pCreateInfo,
                                         const VkAllocationCallbacks* pAllocator, VkQueryPool* pQueryPool) {
    if (!wrap_handles) return device_dispatch_table.CreateQueryPool(device, pCreateInfo, pAllocator, pQueryPool);

    VkResult result = device_dispatch_table.CreateQueryPool(device, pCreateInfo, pAllocator, pQueryPool);
    if (VK_SUCCESS == result) {
        *pQueryPool = WrapNew(*pQueryPool);
    }
    return result;
}

void DispatchObject::DestroyQueryPool(VkDevice device, VkQueryPool queryPool, const VkAllocationCallbacks* pAllocator) {
    if (!wrap_handles) return device_dispatch_table.DestroyQueryPool(device, queryPool, pAllocator);

    uint64_t queryPool_id = CastToUint64(queryPool);
    auto iter = unique_id_mapping.pop(queryPool_id);
    if (iter != unique_id_mapping.end()) {
        queryPool = (VkQueryPool)iter->second;
    } else {
        queryPool = (VkQueryPool)0;
    }
    device_dispatch_table.DestroyQueryPool(device, queryPool, pAllocator);
}

VkResult DispatchObject::GetQueryPoolResults(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount,
                                             size_t dataSize, void* pData, VkDeviceSize stride, VkQueryResultFlags flags) {
    if (!wrap_handles)
        return device_dispatch_table.GetQueryPoolResults(device, queryPool, firstQuery, queryCount, dataSize, pData, stride, flags);
    { queryPool = Unwrap(queryPool); }
    VkResult result =
        device_dispatch_table.GetQueryPoolResults(device, queryPool, firstQuery, queryCount, dataSize, pData, stride, flags);

    return result;
}

VkResult DispatchObject::CreateBuffer(VkDevice device, const VkBufferCreateInfo* pCreateInfo,
                                      const VkAllocationCallbacks* pAllocator, VkBuffer* pBuffer) {
    if (!wrap_handles) return device_dispatch_table.CreateBuffer(device, pCreateInfo, pAllocator, pBuffer);
    vku::safe_VkBufferCreateInfo var_local_pCreateInfo;
    vku::safe_VkBufferCreateInfo* local_pCreateInfo = nullptr;
    {
        if (pCreateInfo) {
            local_pCreateInfo = &var_local_pCreateInfo;
            local_pCreateInfo->initialize(pCreateInfo);
            UnwrapPnextChainHandles(local_pCreateInfo->pNext);
        }
    }
    VkResult result = device_dispatch_table.CreateBuffer(device, (const VkBufferCreateInfo*)local_pCreateInfo, pAllocator, pBuffer);
    if (VK_SUCCESS == result) {
        *pBuffer = WrapNew(*pBuffer);
    }
    return result;
}

void DispatchObject::DestroyBuffer(VkDevice device, VkBuffer buffer, const VkAllocationCallbacks* pAllocator) {
    if (!wrap_handles) return device_dispatch_table.DestroyBuffer(device, buffer, pAllocator);

    uint64_t buffer_id = CastToUint64(buffer);
    auto iter = unique_id_mapping.pop(buffer_id);
    if (iter != unique_id_mapping.end()) {
        buffer = (VkBuffer)iter->second;
    } else {
        buffer = (VkBuffer)0;
    }
    device_dispatch_table.DestroyBuffer(device, buffer, pAllocator);
}

VkResult DispatchObject::CreateBufferView(VkDevice device, const VkBufferViewCreateInfo* pCreateInfo,
                                          const VkAllocationCallbacks* pAllocator, VkBufferView* pView) {
    if (!wrap_handles) return device_dispatch_table.CreateBufferView(device, pCreateInfo, pAllocator, pView);
    vku::safe_VkBufferViewCreateInfo var_local_pCreateInfo;
    vku::safe_VkBufferViewCreateInfo* local_pCreateInfo = nullptr;
    {
        if (pCreateInfo) {
            local_pCreateInfo = &var_local_pCreateInfo;
            local_pCreateInfo->initialize(pCreateInfo);

            if (pCreateInfo->buffer) {
                local_pCreateInfo->buffer = Unwrap(pCreateInfo->buffer);
            }
        }
    }
    VkResult result =
        device_dispatch_table.CreateBufferView(device, (const VkBufferViewCreateInfo*)local_pCreateInfo, pAllocator, pView);
    if (VK_SUCCESS == result) {
        *pView = WrapNew(*pView);
    }
    return result;
}

void DispatchObject::DestroyBufferView(VkDevice device, VkBufferView bufferView, const VkAllocationCallbacks* pAllocator) {
    if (!wrap_handles) return device_dispatch_table.DestroyBufferView(device, bufferView, pAllocator);

    uint64_t bufferView_id = CastToUint64(bufferView);
    auto iter = unique_id_mapping.pop(bufferView_id);
    if (iter != unique_id_mapping.end()) {
        bufferView = (VkBufferView)iter->second;
    } else {
        bufferView = (VkBufferView)0;
    }
    device_dispatch_table.DestroyBufferView(device, bufferView, pAllocator);
}

VkResult DispatchObject::CreateImage(VkDevice device, const VkImageCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                     VkImage* pImage) {
    if (!wrap_handles) return device_dispatch_table.CreateImage(device, pCreateInfo, pAllocator, pImage);
    vku::safe_VkImageCreateInfo var_local_pCreateInfo;
    vku::safe_VkImageCreateInfo* local_pCreateInfo = nullptr;
    {
        if (pCreateInfo) {
            local_pCreateInfo = &var_local_pCreateInfo;
            local_pCreateInfo->initialize(pCreateInfo);
            UnwrapPnextChainHandles(local_pCreateInfo->pNext);
        }
    }
    VkResult result = device_dispatch_table.CreateImage(device, (const VkImageCreateInfo*)local_pCreateInfo, pAllocator, pImage);
    if (VK_SUCCESS == result) {
        *pImage = WrapNew(*pImage);
    }
    return result;
}

void DispatchObject::DestroyImage(VkDevice device, VkImage image, const VkAllocationCallbacks* pAllocator) {
    if (!wrap_handles) return device_dispatch_table.DestroyImage(device, image, pAllocator);

    uint64_t image_id = CastToUint64(image);
    auto iter = unique_id_mapping.pop(image_id);
    if (iter != unique_id_mapping.end()) {
        image = (VkImage)iter->second;
    } else {
        image = (VkImage)0;
    }
    device_dispatch_table.DestroyImage(device, image, pAllocator);
}

void DispatchObject::GetImageSubresourceLayout(VkDevice device, VkImage image, const VkImageSubresource* pSubresource,
                                               VkSubresourceLayout* pLayout) {
    if (!wrap_handles) return device_dispatch_table.GetImageSubresourceLayout(device, image, pSubresource, pLayout);
    { image = Unwrap(image); }
    device_dispatch_table.GetImageSubresourceLayout(device, image, pSubresource, pLayout);
}

VkResult DispatchObject::CreateImageView(VkDevice device, const VkImageViewCreateInfo* pCreateInfo,
                                         const VkAllocationCallbacks* pAllocator, VkImageView* pView) {
    if (!wrap_handles) return device_dispatch_table.CreateImageView(device, pCreateInfo, pAllocator, pView);
    vku::safe_VkImageViewCreateInfo var_local_pCreateInfo;
    vku::safe_VkImageViewCreateInfo* local_pCreateInfo = nullptr;
    {
        if (pCreateInfo) {
            local_pCreateInfo = &var_local_pCreateInfo;
            local_pCreateInfo->initialize(pCreateInfo);

            if (pCreateInfo->image) {
                local_pCreateInfo->image = Unwrap(pCreateInfo->image);
            }
            UnwrapPnextChainHandles(local_pCreateInfo->pNext);
        }
    }
    VkResult result =
        device_dispatch_table.CreateImageView(device, (const VkImageViewCreateInfo*)local_pCreateInfo, pAllocator, pView);
    if (VK_SUCCESS == result) {
        *pView = WrapNew(*pView);
    }
    return result;
}

void DispatchObject::DestroyImageView(VkDevice device, VkImageView imageView, const VkAllocationCallbacks* pAllocator) {
    if (!wrap_handles) return device_dispatch_table.DestroyImageView(device, imageView, pAllocator);

    uint64_t imageView_id = CastToUint64(imageView);
    auto iter = unique_id_mapping.pop(imageView_id);
    if (iter != unique_id_mapping.end()) {
        imageView = (VkImageView)iter->second;
    } else {
        imageView = (VkImageView)0;
    }
    device_dispatch_table.DestroyImageView(device, imageView, pAllocator);
}

VkResult DispatchObject::CreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo,
                                            const VkAllocationCallbacks* pAllocator, VkShaderModule* pShaderModule) {
    if (!wrap_handles) return device_dispatch_table.CreateShaderModule(device, pCreateInfo, pAllocator, pShaderModule);
    vku::safe_VkShaderModuleCreateInfo var_local_pCreateInfo;
    vku::safe_VkShaderModuleCreateInfo* local_pCreateInfo = nullptr;
    {
        if (pCreateInfo) {
            local_pCreateInfo = &var_local_pCreateInfo;
            local_pCreateInfo->initialize(pCreateInfo);
            UnwrapPnextChainHandles(local_pCreateInfo->pNext);
        }
    }
    VkResult result = device_dispatch_table.CreateShaderModule(device, (const VkShaderModuleCreateInfo*)local_pCreateInfo,
                                                               pAllocator, pShaderModule);
    if (VK_SUCCESS == result) {
        *pShaderModule = WrapNew(*pShaderModule);
    }
    return result;
}

void DispatchObject::DestroyShaderModule(VkDevice device, VkShaderModule shaderModule, const VkAllocationCallbacks* pAllocator) {
    if (!wrap_handles) return device_dispatch_table.DestroyShaderModule(device, shaderModule, pAllocator);

    uint64_t shaderModule_id = CastToUint64(shaderModule);
    auto iter = unique_id_mapping.pop(shaderModule_id);
    if (iter != unique_id_mapping.end()) {
        shaderModule = (VkShaderModule)iter->second;
    } else {
        shaderModule = (VkShaderModule)0;
    }
    device_dispatch_table.DestroyShaderModule(device, shaderModule, pAllocator);
}

VkResult DispatchObject::CreatePipelineCache(VkDevice device, const VkPipelineCacheCreateInfo* pCreateInfo,
                                             const VkAllocationCallbacks* pAllocator, VkPipelineCache* pPipelineCache) {
    if (!wrap_handles) return device_dispatch_table.CreatePipelineCache(device, pCreateInfo, pAllocator, pPipelineCache);

    VkResult result = device_dispatch_table.CreatePipelineCache(device, pCreateInfo, pAllocator, pPipelineCache);
    if (VK_SUCCESS == result) {
        *pPipelineCache = WrapNew(*pPipelineCache);
    }
    return result;
}

void DispatchObject::DestroyPipelineCache(VkDevice device, VkPipelineCache pipelineCache, const VkAllocationCallbacks* pAllocator) {
    if (!wrap_handles) return device_dispatch_table.DestroyPipelineCache(device, pipelineCache, pAllocator);

    uint64_t pipelineCache_id = CastToUint64(pipelineCache);
    auto iter = unique_id_mapping.pop(pipelineCache_id);
    if (iter != unique_id_mapping.end()) {
        pipelineCache = (VkPipelineCache)iter->second;
    } else {
        pipelineCache = (VkPipelineCache)0;
    }
    device_dispatch_table.DestroyPipelineCache(device, pipelineCache, pAllocator);
}

VkResult DispatchObject::GetPipelineCacheData(VkDevice device, VkPipelineCache pipelineCache, size_t* pDataSize, void* pData) {
    if (!wrap_handles) return device_dispatch_table.GetPipelineCacheData(device, pipelineCache, pDataSize, pData);
    { pipelineCache = Unwrap(pipelineCache); }
    VkResult result = device_dispatch_table.GetPipelineCacheData(device, pipelineCache, pDataSize, pData);

    return result;
}

VkResult DispatchObject::MergePipelineCaches(VkDevice device, VkPipelineCache dstCache, uint32_t srcCacheCount,
                                             const VkPipelineCache* pSrcCaches) {
    if (!wrap_handles) return device_dispatch_table.MergePipelineCaches(device, dstCache, srcCacheCount, pSrcCaches);
    small_vector<VkPipelineCache, DISPATCH_MAX_STACK_ALLOCATIONS> var_local_pSrcCaches;
    VkPipelineCache* local_pSrcCaches = nullptr;
    {
        dstCache = Unwrap(dstCache);
        if (pSrcCaches) {
            var_local_pSrcCaches.resize(srcCacheCount);
            local_pSrcCaches = var_local_pSrcCaches.data();
            for (uint32_t index0 = 0; index0 < srcCacheCount; ++index0) {
                local_pSrcCaches[index0] = Unwrap(pSrcCaches[index0]);
            }
        }
    }
    VkResult result =
        device_dispatch_table.MergePipelineCaches(device, dstCache, srcCacheCount, (const VkPipelineCache*)local_pSrcCaches);

    return result;
}

void DispatchObject::DestroyPipeline(VkDevice device, VkPipeline pipeline, const VkAllocationCallbacks* pAllocator) {
    if (!wrap_handles) return device_dispatch_table.DestroyPipeline(device, pipeline, pAllocator);

    uint64_t pipeline_id = CastToUint64(pipeline);
    auto iter = unique_id_mapping.pop(pipeline_id);
    if (iter != unique_id_mapping.end()) {
        pipeline = (VkPipeline)iter->second;
    } else {
        pipeline = (VkPipeline)0;
    }
    device_dispatch_table.DestroyPipeline(device, pipeline, pAllocator);
}

VkResult DispatchObject::CreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo* pCreateInfo,
                                              const VkAllocationCallbacks* pAllocator, VkPipelineLayout* pPipelineLayout) {
    if (!wrap_handles) return device_dispatch_table.CreatePipelineLayout(device, pCreateInfo, pAllocator, pPipelineLayout);
    vku::safe_VkPipelineLayoutCreateInfo var_local_pCreateInfo;
    vku::safe_VkPipelineLayoutCreateInfo* local_pCreateInfo = nullptr;
    {
        if (pCreateInfo) {
            local_pCreateInfo = &var_local_pCreateInfo;
            local_pCreateInfo->initialize(pCreateInfo);
            if (local_pCreateInfo->pSetLayouts) {
                for (uint32_t index1 = 0; index1 < local_pCreateInfo->setLayoutCount; ++index1) {
                    local_pCreateInfo->pSetLayouts[index1] = Unwrap(local_pCreateInfo->pSetLayouts[index1]);
                }
            }
        }
    }
    VkResult result = device_dispatch_table.CreatePipelineLayout(device, (const VkPipelineLayoutCreateInfo*)local_pCreateInfo,
                                                                 pAllocator, pPipelineLayout);
    if (VK_SUCCESS == result) {
        *pPipelineLayout = WrapNew(*pPipelineLayout);
    }
    return result;
}

void DispatchObject::DestroyPipelineLayout(VkDevice device, VkPipelineLayout pipelineLayout,
                                           const VkAllocationCallbacks* pAllocator) {
    if (!wrap_handles) return device_dispatch_table.DestroyPipelineLayout(device, pipelineLayout, pAllocator);

    uint64_t pipelineLayout_id = CastToUint64(pipelineLayout);
    auto iter = unique_id_mapping.pop(pipelineLayout_id);
    if (iter != unique_id_mapping.end()) {
        pipelineLayout = (VkPipelineLayout)iter->second;
    } else {
        pipelineLayout = (VkPipelineLayout)0;
    }
    device_dispatch_table.DestroyPipelineLayout(device, pipelineLayout, pAllocator);
}

VkResult DispatchObject::CreateSampler(VkDevice device, const VkSamplerCreateInfo* pCreateInfo,
                                       const VkAllocationCallbacks* pAllocator, VkSampler* pSampler) {
    if (!wrap_handles) return device_dispatch_table.CreateSampler(device, pCreateInfo, pAllocator, pSampler);
    vku::safe_VkSamplerCreateInfo var_local_pCreateInfo;
    vku::safe_VkSamplerCreateInfo* local_pCreateInfo = nullptr;
    {
        if (pCreateInfo) {
            local_pCreateInfo = &var_local_pCreateInfo;
            local_pCreateInfo->initialize(pCreateInfo);
            UnwrapPnextChainHandles(local_pCreateInfo->pNext);
        }
    }
    VkResult result =
        device_dispatch_table.CreateSampler(device, (const VkSamplerCreateInfo*)local_pCreateInfo, pAllocator, pSampler);
    if (VK_SUCCESS == result) {
        *pSampler = WrapNew(*pSampler);
    }
    return result;
}

void DispatchObject::DestroySampler(VkDevice device, VkSampler sampler, const VkAllocationCallbacks* pAllocator) {
    if (!wrap_handles) return device_dispatch_table.DestroySampler(device, sampler, pAllocator);

    uint64_t sampler_id = CastToUint64(sampler);
    auto iter = unique_id_mapping.pop(sampler_id);
    if (iter != unique_id_mapping.end()) {
        sampler = (VkSampler)iter->second;
    } else {
        sampler = (VkSampler)0;
    }
    device_dispatch_table.DestroySampler(device, sampler, pAllocator);
}

VkResult DispatchObject::CreateDescriptorSetLayout(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo,
                                                   const VkAllocationCallbacks* pAllocator, VkDescriptorSetLayout* pSetLayout) {
    if (!wrap_handles) return device_dispatch_table.CreateDescriptorSetLayout(device, pCreateInfo, pAllocator, pSetLayout);
    vku::safe_VkDescriptorSetLayoutCreateInfo var_local_pCreateInfo;
    vku::safe_VkDescriptorSetLayoutCreateInfo* local_pCreateInfo = nullptr;
    {
        if (pCreateInfo) {
            local_pCreateInfo = &var_local_pCreateInfo;
            local_pCreateInfo->initialize(pCreateInfo);
            if (local_pCreateInfo->pBindings) {
                for (uint32_t index1 = 0; index1 < local_pCreateInfo->bindingCount; ++index1) {
                    if (local_pCreateInfo->pBindings[index1].pImmutableSamplers) {
                        for (uint32_t index2 = 0; index2 < local_pCreateInfo->pBindings[index1].descriptorCount; ++index2) {
                            local_pCreateInfo->pBindings[index1].pImmutableSamplers[index2] =
                                Unwrap(local_pCreateInfo->pBindings[index1].pImmutableSamplers[index2]);
                        }
                    }
                }
            }
        }
    }
    VkResult result = device_dispatch_table.CreateDescriptorSetLayout(
        device, (const VkDescriptorSetLayoutCreateInfo*)local_pCreateInfo, pAllocator, pSetLayout);
    if (VK_SUCCESS == result) {
        *pSetLayout = WrapNew(*pSetLayout);
    }
    return result;
}

void DispatchObject::DestroyDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout descriptorSetLayout,
                                                const VkAllocationCallbacks* pAllocator) {
    if (!wrap_handles) return device_dispatch_table.DestroyDescriptorSetLayout(device, descriptorSetLayout, pAllocator);

    uint64_t descriptorSetLayout_id = CastToUint64(descriptorSetLayout);
    auto iter = unique_id_mapping.pop(descriptorSetLayout_id);
    if (iter != unique_id_mapping.end()) {
        descriptorSetLayout = (VkDescriptorSetLayout)iter->second;
    } else {
        descriptorSetLayout = (VkDescriptorSetLayout)0;
    }
    device_dispatch_table.DestroyDescriptorSetLayout(device, descriptorSetLayout, pAllocator);
}

VkResult DispatchObject::CreateDescriptorPool(VkDevice device, const VkDescriptorPoolCreateInfo* pCreateInfo,
                                              const VkAllocationCallbacks* pAllocator, VkDescriptorPool* pDescriptorPool) {
    if (!wrap_handles) return device_dispatch_table.CreateDescriptorPool(device, pCreateInfo, pAllocator, pDescriptorPool);

    VkResult result = device_dispatch_table.CreateDescriptorPool(device, pCreateInfo, pAllocator, pDescriptorPool);
    if (VK_SUCCESS == result) {
        *pDescriptorPool = WrapNew(*pDescriptorPool);
    }
    return result;
}

void DispatchObject::UpdateDescriptorSets(VkDevice device, uint32_t descriptorWriteCount,
                                          const VkWriteDescriptorSet* pDescriptorWrites, uint32_t descriptorCopyCount,
                                          const VkCopyDescriptorSet* pDescriptorCopies) {
    if (!wrap_handles)
        return device_dispatch_table.UpdateDescriptorSets(device, descriptorWriteCount, pDescriptorWrites, descriptorCopyCount,
                                                          pDescriptorCopies);
    small_vector<vku::safe_VkWriteDescriptorSet, DISPATCH_MAX_STACK_ALLOCATIONS> var_local_pDescriptorWrites;
    vku::safe_VkWriteDescriptorSet* local_pDescriptorWrites = nullptr;
    small_vector<vku::safe_VkCopyDescriptorSet, DISPATCH_MAX_STACK_ALLOCATIONS> var_local_pDescriptorCopies;
    vku::safe_VkCopyDescriptorSet* local_pDescriptorCopies = nullptr;
    {
        if (pDescriptorWrites) {
            var_local_pDescriptorWrites.resize(descriptorWriteCount);
            local_pDescriptorWrites = var_local_pDescriptorWrites.data();
            for (uint32_t index0 = 0; index0 < descriptorWriteCount; ++index0) {
                local_pDescriptorWrites[index0].initialize(&pDescriptorWrites[index0]);
                UnwrapPnextChainHandles(local_pDescriptorWrites[index0].pNext);

                if (pDescriptorWrites[index0].dstSet) {
                    local_pDescriptorWrites[index0].dstSet = Unwrap(pDescriptorWrites[index0].dstSet);
                }
                if (local_pDescriptorWrites[index0].pImageInfo) {
                    for (uint32_t index1 = 0; index1 < local_pDescriptorWrites[index0].descriptorCount; ++index1) {
                        if (pDescriptorWrites[index0].pImageInfo[index1].sampler) {
                            local_pDescriptorWrites[index0].pImageInfo[index1].sampler =
                                Unwrap(pDescriptorWrites[index0].pImageInfo[index1].sampler);
                        }
                        if (pDescriptorWrites[index0].pImageInfo[index1].imageView) {
                            local_pDescriptorWrites[index0].pImageInfo[index1].imageView =
                                Unwrap(pDescriptorWrites[index0].pImageInfo[index1].imageView);
                        }
                    }
                }
                if (local_pDescriptorWrites[index0].pBufferInfo) {
                    for (uint32_t index1 = 0; index1 < local_pDescriptorWrites[index0].descriptorCount; ++index1) {
                        if (pDescriptorWrites[index0].pBufferInfo[index1].buffer) {
                            local_pDescriptorWrites[index0].pBufferInfo[index1].buffer =
                                Unwrap(pDescriptorWrites[index0].pBufferInfo[index1].buffer);
                        }
                    }
                }
                if (local_pDescriptorWrites[index0].pTexelBufferView) {
                    for (uint32_t index1 = 0; index1 < local_pDescriptorWrites[index0].descriptorCount; ++index1) {
                        local_pDescriptorWrites[index0].pTexelBufferView[index1] =
                            Unwrap(local_pDescriptorWrites[index0].pTexelBufferView[index1]);
                    }
                }
            }
        }
        if (pDescriptorCopies) {
            var_local_pDescriptorCopies.resize(descriptorCopyCount);
            local_pDescriptorCopies = var_local_pDescriptorCopies.data();
            for (uint32_t index0 = 0; index0 < descriptorCopyCount; ++index0) {
                local_pDescriptorCopies[index0].initialize(&pDescriptorCopies[index0]);

                if (pDescriptorCopies[index0].srcSet) {
                    local_pDescriptorCopies[index0].srcSet = Unwrap(pDescriptorCopies[index0].srcSet);
                }
                if (pDescriptorCopies[index0].dstSet) {
                    local_pDescriptorCopies[index0].dstSet = Unwrap(pDescriptorCopies[index0].dstSet);
                }
            }
        }
    }
    device_dispatch_table.UpdateDescriptorSets(device, descriptorWriteCount, (const VkWriteDescriptorSet*)local_pDescriptorWrites,
                                               descriptorCopyCount, (const VkCopyDescriptorSet*)local_pDescriptorCopies);
}

VkResult DispatchObject::CreateFramebuffer(VkDevice device, const VkFramebufferCreateInfo* pCreateInfo,
                                           const VkAllocationCallbacks* pAllocator, VkFramebuffer* pFramebuffer) {
    if (!wrap_handles) return device_dispatch_table.CreateFramebuffer(device, pCreateInfo, pAllocator, pFramebuffer);
    vku::safe_VkFramebufferCreateInfo var_local_pCreateInfo;
    vku::safe_VkFramebufferCreateInfo* local_pCreateInfo = nullptr;
    {
        if (pCreateInfo) {
            local_pCreateInfo = &var_local_pCreateInfo;
            local_pCreateInfo->initialize(pCreateInfo);

            if (pCreateInfo->renderPass) {
                local_pCreateInfo->renderPass = Unwrap(pCreateInfo->renderPass);
            }
            if (local_pCreateInfo->pAttachments) {
                for (uint32_t index1 = 0; index1 < local_pCreateInfo->attachmentCount; ++index1) {
                    local_pCreateInfo->pAttachments[index1] = Unwrap(local_pCreateInfo->pAttachments[index1]);
                }
            }
        }
    }
    VkResult result = device_dispatch_table.CreateFramebuffer(device, (const VkFramebufferCreateInfo*)local_pCreateInfo, pAllocator,
                                                              pFramebuffer);
    if (VK_SUCCESS == result) {
        *pFramebuffer = WrapNew(*pFramebuffer);
    }
    return result;
}

void DispatchObject::DestroyFramebuffer(VkDevice device, VkFramebuffer framebuffer, const VkAllocationCallbacks* pAllocator) {
    if (!wrap_handles) return device_dispatch_table.DestroyFramebuffer(device, framebuffer, pAllocator);

    uint64_t framebuffer_id = CastToUint64(framebuffer);
    auto iter = unique_id_mapping.pop(framebuffer_id);
    if (iter != unique_id_mapping.end()) {
        framebuffer = (VkFramebuffer)iter->second;
    } else {
        framebuffer = (VkFramebuffer)0;
    }
    device_dispatch_table.DestroyFramebuffer(device, framebuffer, pAllocator);
}

void DispatchObject::GetRenderAreaGranularity(VkDevice device, VkRenderPass renderPass, VkExtent2D* pGranularity) {
    if (!wrap_handles) return device_dispatch_table.GetRenderAreaGranularity(device, renderPass, pGranularity);
    { renderPass = Unwrap(renderPass); }
    device_dispatch_table.GetRenderAreaGranularity(device, renderPass, pGranularity);
}

VkResult DispatchObject::CreateCommandPool(VkDevice device, const VkCommandPoolCreateInfo* pCreateInfo,
                                           const VkAllocationCallbacks* pAllocator, VkCommandPool* pCommandPool) {
    if (!wrap_handles) return device_dispatch_table.CreateCommandPool(device, pCreateInfo, pAllocator, pCommandPool);

    VkResult result = device_dispatch_table.CreateCommandPool(device, pCreateInfo, pAllocator, pCommandPool);
    if (VK_SUCCESS == result) {
        *pCommandPool = WrapNew(*pCommandPool);
    }
    return result;
}

VkResult DispatchObject::ResetCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolResetFlags flags) {
    if (!wrap_handles) return device_dispatch_table.ResetCommandPool(device, commandPool, flags);
    { commandPool = Unwrap(commandPool); }
    VkResult result = device_dispatch_table.ResetCommandPool(device, commandPool, flags);

    return result;
}

VkResult DispatchObject::EndCommandBuffer(VkCommandBuffer commandBuffer) {
    VkResult result = device_dispatch_table.EndCommandBuffer(commandBuffer);

    return result;
}

VkResult DispatchObject::ResetCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferResetFlags flags) {
    VkResult result = device_dispatch_table.ResetCommandBuffer(commandBuffer, flags);

    return result;
}

void DispatchObject::CmdBindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline) {
    if (!wrap_handles) return device_dispatch_table.CmdBindPipeline(commandBuffer, pipelineBindPoint, pipeline);
    { pipeline = Unwrap(pipeline); }
    device_dispatch_table.CmdBindPipeline(commandBuffer, pipelineBindPoint, pipeline);
}

void DispatchObject::CmdSetViewport(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount,
                                    const VkViewport* pViewports) {
    device_dispatch_table.CmdSetViewport(commandBuffer, firstViewport, viewportCount, pViewports);
}

void DispatchObject::CmdSetScissor(VkCommandBuffer commandBuffer, uint32_t firstScissor, uint32_t scissorCount,
                                   const VkRect2D* pScissors) {
    device_dispatch_table.CmdSetScissor(commandBuffer, firstScissor, scissorCount, pScissors);
}

void DispatchObject::CmdSetLineWidth(VkCommandBuffer commandBuffer, float lineWidth) {
    device_dispatch_table.CmdSetLineWidth(commandBuffer, lineWidth);
}

void DispatchObject::CmdSetDepthBias(VkCommandBuffer commandBuffer, float depthBiasConstantFactor, float depthBiasClamp,
                                     float depthBiasSlopeFactor) {
    device_dispatch_table.CmdSetDepthBias(commandBuffer, depthBiasConstantFactor, depthBiasClamp, depthBiasSlopeFactor);
}

void DispatchObject::CmdSetBlendConstants(VkCommandBuffer commandBuffer, const float blendConstants[4]) {
    device_dispatch_table.CmdSetBlendConstants(commandBuffer, blendConstants);
}

void DispatchObject::CmdSetDepthBounds(VkCommandBuffer commandBuffer, float minDepthBounds, float maxDepthBounds) {
    device_dispatch_table.CmdSetDepthBounds(commandBuffer, minDepthBounds, maxDepthBounds);
}

void DispatchObject::CmdSetStencilCompareMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t compareMask) {
    device_dispatch_table.CmdSetStencilCompareMask(commandBuffer, faceMask, compareMask);
}

void DispatchObject::CmdSetStencilWriteMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t writeMask) {
    device_dispatch_table.CmdSetStencilWriteMask(commandBuffer, faceMask, writeMask);
}

void DispatchObject::CmdSetStencilReference(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t reference) {
    device_dispatch_table.CmdSetStencilReference(commandBuffer, faceMask, reference);
}

void DispatchObject::CmdBindDescriptorSets(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                           VkPipelineLayout layout, uint32_t firstSet, uint32_t descriptorSetCount,
                                           const VkDescriptorSet* pDescriptorSets, uint32_t dynamicOffsetCount,
                                           const uint32_t* pDynamicOffsets) {
    if (!wrap_handles)
        return device_dispatch_table.CmdBindDescriptorSets(commandBuffer, pipelineBindPoint, layout, firstSet, descriptorSetCount,
                                                           pDescriptorSets, dynamicOffsetCount, pDynamicOffsets);
    small_vector<VkDescriptorSet, DISPATCH_MAX_STACK_ALLOCATIONS> var_local_pDescriptorSets;
    VkDescriptorSet* local_pDescriptorSets = nullptr;
    {
        layout = Unwrap(layout);
        if (pDescriptorSets) {
            var_local_pDescriptorSets.resize(descriptorSetCount);
            local_pDescriptorSets = var_local_pDescriptorSets.data();
            for (uint32_t index0 = 0; index0 < descriptorSetCount; ++index0) {
                local_pDescriptorSets[index0] = Unwrap(pDescriptorSets[index0]);
            }
        }
    }
    device_dispatch_table.CmdBindDescriptorSets(commandBuffer, pipelineBindPoint, layout, firstSet, descriptorSetCount,
                                                (const VkDescriptorSet*)local_pDescriptorSets, dynamicOffsetCount, pDynamicOffsets);
}

void DispatchObject::CmdBindIndexBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                        VkIndexType indexType) {
    if (!wrap_handles) return device_dispatch_table.CmdBindIndexBuffer(commandBuffer, buffer, offset, indexType);
    { buffer = Unwrap(buffer); }
    device_dispatch_table.CmdBindIndexBuffer(commandBuffer, buffer, offset, indexType);
}

void DispatchObject::CmdBindVertexBuffers(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount,
                                          const VkBuffer* pBuffers, const VkDeviceSize* pOffsets) {
    if (!wrap_handles)
        return device_dispatch_table.CmdBindVertexBuffers(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets);
    small_vector<VkBuffer, DISPATCH_MAX_STACK_ALLOCATIONS> var_local_pBuffers;
    VkBuffer* local_pBuffers = nullptr;
    {
        if (pBuffers) {
            var_local_pBuffers.resize(bindingCount);
            local_pBuffers = var_local_pBuffers.data();
            for (uint32_t index0 = 0; index0 < bindingCount; ++index0) {
                local_pBuffers[index0] = Unwrap(pBuffers[index0]);
            }
        }
    }
    device_dispatch_table.CmdBindVertexBuffers(commandBuffer, firstBinding, bindingCount, (const VkBuffer*)local_pBuffers,
                                               pOffsets);
}

void DispatchObject::CmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex,
                             uint32_t firstInstance) {
    device_dispatch_table.CmdDraw(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
}

void DispatchObject::CmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex,
                                    int32_t vertexOffset, uint32_t firstInstance) {
    device_dispatch_table.CmdDrawIndexed(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

void DispatchObject::CmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount,
                                     uint32_t stride) {
    if (!wrap_handles) return device_dispatch_table.CmdDrawIndirect(commandBuffer, buffer, offset, drawCount, stride);
    { buffer = Unwrap(buffer); }
    device_dispatch_table.CmdDrawIndirect(commandBuffer, buffer, offset, drawCount, stride);
}

void DispatchObject::CmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount,
                                            uint32_t stride) {
    if (!wrap_handles) return device_dispatch_table.CmdDrawIndexedIndirect(commandBuffer, buffer, offset, drawCount, stride);
    { buffer = Unwrap(buffer); }
    device_dispatch_table.CmdDrawIndexedIndirect(commandBuffer, buffer, offset, drawCount, stride);
}

void DispatchObject::CmdDispatch(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) {
    device_dispatch_table.CmdDispatch(commandBuffer, groupCountX, groupCountY, groupCountZ);
}

void DispatchObject::CmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset) {
    if (!wrap_handles) return device_dispatch_table.CmdDispatchIndirect(commandBuffer, buffer, offset);
    { buffer = Unwrap(buffer); }
    device_dispatch_table.CmdDispatchIndirect(commandBuffer, buffer, offset);
}

void DispatchObject::CmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t regionCount,
                                   const VkBufferCopy* pRegions) {
    if (!wrap_handles) return device_dispatch_table.CmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, regionCount, pRegions);
    {
        srcBuffer = Unwrap(srcBuffer);
        dstBuffer = Unwrap(dstBuffer);
    }
    device_dispatch_table.CmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, regionCount, pRegions);
}

void DispatchObject::CmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage,
                                  VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageCopy* pRegions) {
    if (!wrap_handles)
        return device_dispatch_table.CmdCopyImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount,
                                                  pRegions);
    {
        srcImage = Unwrap(srcImage);
        dstImage = Unwrap(dstImage);
    }
    device_dispatch_table.CmdCopyImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
}

void DispatchObject::CmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage,
                                  VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageBlit* pRegions,
                                  VkFilter filter) {
    if (!wrap_handles)
        return device_dispatch_table.CmdBlitImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount,
                                                  pRegions, filter);
    {
        srcImage = Unwrap(srcImage);
        dstImage = Unwrap(dstImage);
    }
    device_dispatch_table.CmdBlitImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions,
                                       filter);
}

void DispatchObject::CmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage,
                                          VkImageLayout dstImageLayout, uint32_t regionCount, const VkBufferImageCopy* pRegions) {
    if (!wrap_handles)
        return device_dispatch_table.CmdCopyBufferToImage(commandBuffer, srcBuffer, dstImage, dstImageLayout, regionCount,
                                                          pRegions);
    {
        srcBuffer = Unwrap(srcBuffer);
        dstImage = Unwrap(dstImage);
    }
    device_dispatch_table.CmdCopyBufferToImage(commandBuffer, srcBuffer, dstImage, dstImageLayout, regionCount, pRegions);
}

void DispatchObject::CmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                          VkBuffer dstBuffer, uint32_t regionCount, const VkBufferImageCopy* pRegions) {
    if (!wrap_handles)
        return device_dispatch_table.CmdCopyImageToBuffer(commandBuffer, srcImage, srcImageLayout, dstBuffer, regionCount,
                                                          pRegions);
    {
        srcImage = Unwrap(srcImage);
        dstBuffer = Unwrap(dstBuffer);
    }
    device_dispatch_table.CmdCopyImageToBuffer(commandBuffer, srcImage, srcImageLayout, dstBuffer, regionCount, pRegions);
}

void DispatchObject::CmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                     VkDeviceSize dataSize, const void* pData) {
    if (!wrap_handles) return device_dispatch_table.CmdUpdateBuffer(commandBuffer, dstBuffer, dstOffset, dataSize, pData);
    { dstBuffer = Unwrap(dstBuffer); }
    device_dispatch_table.CmdUpdateBuffer(commandBuffer, dstBuffer, dstOffset, dataSize, pData);
}

void DispatchObject::CmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize size,
                                   uint32_t data) {
    if (!wrap_handles) return device_dispatch_table.CmdFillBuffer(commandBuffer, dstBuffer, dstOffset, size, data);
    { dstBuffer = Unwrap(dstBuffer); }
    device_dispatch_table.CmdFillBuffer(commandBuffer, dstBuffer, dstOffset, size, data);
}

void DispatchObject::CmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                        const VkClearColorValue* pColor, uint32_t rangeCount,
                                        const VkImageSubresourceRange* pRanges) {
    if (!wrap_handles)
        return device_dispatch_table.CmdClearColorImage(commandBuffer, image, imageLayout, pColor, rangeCount, pRanges);
    { image = Unwrap(image); }
    device_dispatch_table.CmdClearColorImage(commandBuffer, image, imageLayout, pColor, rangeCount, pRanges);
}

void DispatchObject::CmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                               const VkClearDepthStencilValue* pDepthStencil, uint32_t rangeCount,
                                               const VkImageSubresourceRange* pRanges) {
    if (!wrap_handles)
        return device_dispatch_table.CmdClearDepthStencilImage(commandBuffer, image, imageLayout, pDepthStencil, rangeCount,
                                                               pRanges);
    { image = Unwrap(image); }
    device_dispatch_table.CmdClearDepthStencilImage(commandBuffer, image, imageLayout, pDepthStencil, rangeCount, pRanges);
}

void DispatchObject::CmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
                                         const VkClearAttachment* pAttachments, uint32_t rectCount, const VkClearRect* pRects) {
    device_dispatch_table.CmdClearAttachments(commandBuffer, attachmentCount, pAttachments, rectCount, pRects);
}

void DispatchObject::CmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                     VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                     const VkImageResolve* pRegions) {
    if (!wrap_handles)
        return device_dispatch_table.CmdResolveImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount,
                                                     pRegions);
    {
        srcImage = Unwrap(srcImage);
        dstImage = Unwrap(dstImage);
    }
    device_dispatch_table.CmdResolveImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
}

void DispatchObject::CmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask) {
    if (!wrap_handles) return device_dispatch_table.CmdSetEvent(commandBuffer, event, stageMask);
    { event = Unwrap(event); }
    device_dispatch_table.CmdSetEvent(commandBuffer, event, stageMask);
}

void DispatchObject::CmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask) {
    if (!wrap_handles) return device_dispatch_table.CmdResetEvent(commandBuffer, event, stageMask);
    { event = Unwrap(event); }
    device_dispatch_table.CmdResetEvent(commandBuffer, event, stageMask);
}

void DispatchObject::CmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                   VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask,
                                   uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers,
                                   uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers,
                                   uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers) {
    if (!wrap_handles)
        return device_dispatch_table.CmdWaitEvents(commandBuffer, eventCount, pEvents, srcStageMask, dstStageMask,
                                                   memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount,
                                                   pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
    small_vector<VkEvent, DISPATCH_MAX_STACK_ALLOCATIONS> var_local_pEvents;
    VkEvent* local_pEvents = nullptr;
    small_vector<vku::safe_VkBufferMemoryBarrier, DISPATCH_MAX_STACK_ALLOCATIONS> var_local_pBufferMemoryBarriers;
    vku::safe_VkBufferMemoryBarrier* local_pBufferMemoryBarriers = nullptr;
    small_vector<vku::safe_VkImageMemoryBarrier, DISPATCH_MAX_STACK_ALLOCATIONS> var_local_pImageMemoryBarriers;
    vku::safe_VkImageMemoryBarrier* local_pImageMemoryBarriers = nullptr;
    {
        if (pEvents) {
            var_local_pEvents.resize(eventCount);
            local_pEvents = var_local_pEvents.data();
            for (uint32_t index0 = 0; index0 < eventCount; ++index0) {
                local_pEvents[index0] = Unwrap(pEvents[index0]);
            }
        }
        if (pBufferMemoryBarriers) {
            var_local_pBufferMemoryBarriers.resize(bufferMemoryBarrierCount);
            local_pBufferMemoryBarriers = var_local_pBufferMemoryBarriers.data();
            for (uint32_t index0 = 0; index0 < bufferMemoryBarrierCount; ++index0) {
                local_pBufferMemoryBarriers[index0].initialize(&pBufferMemoryBarriers[index0]);

                if (pBufferMemoryBarriers[index0].buffer) {
                    local_pBufferMemoryBarriers[index0].buffer = Unwrap(pBufferMemoryBarriers[index0].buffer);
                }
            }
        }
        if (pImageMemoryBarriers) {
            var_local_pImageMemoryBarriers.resize(imageMemoryBarrierCount);
            local_pImageMemoryBarriers = var_local_pImageMemoryBarriers.data();
            for (uint32_t index0 = 0; index0 < imageMemoryBarrierCount; ++index0) {
                local_pImageMemoryBarriers[index0].initialize(&pImageMemoryBarriers[index0]);

                if (pImageMemoryBarriers[index0].image) {
                    local_pImageMemoryBarriers[index0].image = Unwrap(pImageMemoryBarriers[index0].image);
                }
            }
        }
    }
    device_dispatch_table.CmdWaitEvents(commandBuffer, eventCount, (const VkEvent*)local_pEvents, srcStageMask, dstStageMask,
                                        memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount,
                                        (const VkBufferMemoryBarrier*)local_pBufferMemoryBarriers, imageMemoryBarrierCount,
                                        (const VkImageMemoryBarrier*)local_pImageMemoryBarriers);
}

void DispatchObject::CmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask,
                                        VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags,
                                        uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers,
                                        uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers,
                                        uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers) {
    if (!wrap_handles)
        return device_dispatch_table.CmdPipelineBarrier(commandBuffer, srcStageMask, dstStageMask, dependencyFlags,
                                                        memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount,
                                                        pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
    small_vector<vku::safe_VkBufferMemoryBarrier, DISPATCH_MAX_STACK_ALLOCATIONS> var_local_pBufferMemoryBarriers;
    vku::safe_VkBufferMemoryBarrier* local_pBufferMemoryBarriers = nullptr;
    small_vector<vku::safe_VkImageMemoryBarrier, DISPATCH_MAX_STACK_ALLOCATIONS> var_local_pImageMemoryBarriers;
    vku::safe_VkImageMemoryBarrier* local_pImageMemoryBarriers = nullptr;
    {
        if (pBufferMemoryBarriers) {
            var_local_pBufferMemoryBarriers.resize(bufferMemoryBarrierCount);
            local_pBufferMemoryBarriers = var_local_pBufferMemoryBarriers.data();
            for (uint32_t index0 = 0; index0 < bufferMemoryBarrierCount; ++index0) {
                local_pBufferMemoryBarriers[index0].initialize(&pBufferMemoryBarriers[index0]);

                if (pBufferMemoryBarriers[index0].buffer) {
                    local_pBufferMemoryBarriers[index0].buffer = Unwrap(pBufferMemoryBarriers[index0].buffer);
                }
            }
        }
        if (pImageMemoryBarriers) {
            var_local_pImageMemoryBarriers.resize(imageMemoryBarrierCount);
            local_pImageMemoryBarriers = var_local_pImageMemoryBarriers.data();
            for (uint32_t index0 = 0; index0 < imageMemoryBarrierCount; ++index0) {
                local_pImageMemoryBarriers[index0].initialize(&pImageMemoryBarriers[index0]);

                if (pImageMemoryBarriers[index0].image) {
                    local_pImageMemoryBarriers[index0].image = Unwrap(pImageMemoryBarriers[index0].image);
                }
            }
        }
    }
    device_dispatch_table.CmdPipelineBarrier(commandBuffer, srcStageMask, dstStageMask, dependencyFlags, memoryBarrierCount,
                                             pMemoryBarriers, bufferMemoryBarrierCount,
                                             (const VkBufferMemoryBarrier*)local_pBufferMemoryBarriers, imageMemoryBarrierCount,
                                             (const VkImageMemoryBarrier*)local_pImageMemoryBarriers);
}

void DispatchObject::CmdBeginQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query,
                                   VkQueryControlFlags flags) {
    if (!wrap_handles) return device_dispatch_table.CmdBeginQuery(commandBuffer, queryPool, query, flags);
    { queryPool = Unwrap(queryPool); }
    device_dispatch_table.CmdBeginQuery(commandBuffer, queryPool, query, flags);
}

void DispatchObject::CmdEndQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query) {
    if (!wrap_handles) return device_dispatch_table.CmdEndQuery(commandBuffer, queryPool, query);
    { queryPool = Unwrap(queryPool); }
    device_dispatch_table.CmdEndQuery(commandBuffer, queryPool, query);
}

void DispatchObject::CmdResetQueryPool(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery,
                                       uint32_t queryCount) {
    if (!wrap_handles) return device_dispatch_table.CmdResetQueryPool(commandBuffer, queryPool, firstQuery, queryCount);
    { queryPool = Unwrap(queryPool); }
    device_dispatch_table.CmdResetQueryPool(commandBuffer, queryPool, firstQuery, queryCount);
}

void DispatchObject::CmdWriteTimestamp(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage, VkQueryPool queryPool,
                                       uint32_t query) {
    if (!wrap_handles) return device_dispatch_table.CmdWriteTimestamp(commandBuffer, pipelineStage, queryPool, query);
    { queryPool = Unwrap(queryPool); }
    device_dispatch_table.CmdWriteTimestamp(commandBuffer, pipelineStage, queryPool, query);
}

void DispatchObject::CmdCopyQueryPoolResults(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery,
                                             uint32_t queryCount, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize stride,
                                             VkQueryResultFlags flags) {
    if (!wrap_handles)
        return device_dispatch_table.CmdCopyQueryPoolResults(commandBuffer, queryPool, firstQuery, queryCount, dstBuffer, dstOffset,
                                                             stride, flags);
    {
        queryPool = Unwrap(queryPool);
        dstBuffer = Unwrap(dstBuffer);
    }
    device_dispatch_table.CmdCopyQueryPoolResults(commandBuffer, queryPool, firstQuery, queryCount, dstBuffer, dstOffset, stride,
                                                  flags);
}

void DispatchObject::CmdPushConstants(VkCommandBuffer commandBuffer, VkPipelineLayout layout, VkShaderStageFlags stageFlags,
                                      uint32_t offset, uint32_t size, const void* pValues) {
    if (!wrap_handles) return device_dispatch_table.CmdPushConstants(commandBuffer, layout, stageFlags, offset, size, pValues);
    { layout = Unwrap(layout); }
    device_dispatch_table.CmdPushConstants(commandBuffer, layout, stageFlags, offset, size, pValues);
}

void DispatchObject::CmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                        VkSubpassContents contents) {
    if (!wrap_handles) return device_dispatch_table.CmdBeginRenderPass(commandBuffer, pRenderPassBegin, contents);
    vku::safe_VkRenderPassBeginInfo var_local_pRenderPassBegin;
    vku::safe_VkRenderPassBeginInfo* local_pRenderPassBegin = nullptr;
    {
        if (pRenderPassBegin) {
            local_pRenderPassBegin = &var_local_pRenderPassBegin;
            local_pRenderPassBegin->initialize(pRenderPassBegin);

            if (pRenderPassBegin->renderPass) {
                local_pRenderPassBegin->renderPass = Unwrap(pRenderPassBegin->renderPass);
            }
            if (pRenderPassBegin->framebuffer) {
                local_pRenderPassBegin->framebuffer = Unwrap(pRenderPassBegin->framebuffer);
            }
            UnwrapPnextChainHandles(local_pRenderPassBegin->pNext);
        }
    }
    device_dispatch_table.CmdBeginRenderPass(commandBuffer, (const VkRenderPassBeginInfo*)local_pRenderPassBegin, contents);
}

void DispatchObject::CmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents) {
    device_dispatch_table.CmdNextSubpass(commandBuffer, contents);
}

void DispatchObject::CmdEndRenderPass(VkCommandBuffer commandBuffer) { device_dispatch_table.CmdEndRenderPass(commandBuffer); }

void DispatchObject::CmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBufferCount,
                                        const VkCommandBuffer* pCommandBuffers) {
    device_dispatch_table.CmdExecuteCommands(commandBuffer, commandBufferCount, pCommandBuffers);
}

VkResult DispatchObject::BindBufferMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindBufferMemoryInfo* pBindInfos) {
    if (!wrap_handles) return device_dispatch_table.BindBufferMemory2(device, bindInfoCount, pBindInfos);
    small_vector<vku::safe_VkBindBufferMemoryInfo, DISPATCH_MAX_STACK_ALLOCATIONS> var_local_pBindInfos;
    vku::safe_VkBindBufferMemoryInfo* local_pBindInfos = nullptr;
    {
        if (pBindInfos) {
            var_local_pBindInfos.resize(bindInfoCount);
            local_pBindInfos = var_local_pBindInfos.data();
            for (uint32_t index0 = 0; index0 < bindInfoCount; ++index0) {
                local_pBindInfos[index0].initialize(&pBindInfos[index0]);

                if (pBindInfos[index0].buffer) {
                    local_pBindInfos[index0].buffer = Unwrap(pBindInfos[index0].buffer);
                }
                if (pBindInfos[index0].memory) {
                    local_pBindInfos[index0].memory = Unwrap(pBindInfos[index0].memory);
                }
            }
        }
    }
    VkResult result =
        device_dispatch_table.BindBufferMemory2(device, bindInfoCount, (const VkBindBufferMemoryInfo*)local_pBindInfos);

    return result;
}

VkResult DispatchObject::BindImageMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo* pBindInfos) {
    if (!wrap_handles) return device_dispatch_table.BindImageMemory2(device, bindInfoCount, pBindInfos);
    small_vector<vku::safe_VkBindImageMemoryInfo, DISPATCH_MAX_STACK_ALLOCATIONS> var_local_pBindInfos;
    vku::safe_VkBindImageMemoryInfo* local_pBindInfos = nullptr;
    {
        if (pBindInfos) {
            var_local_pBindInfos.resize(bindInfoCount);
            local_pBindInfos = var_local_pBindInfos.data();
            for (uint32_t index0 = 0; index0 < bindInfoCount; ++index0) {
                local_pBindInfos[index0].initialize(&pBindInfos[index0]);
                UnwrapPnextChainHandles(local_pBindInfos[index0].pNext);

                if (pBindInfos[index0].image) {
                    local_pBindInfos[index0].image = Unwrap(pBindInfos[index0].image);
                }
                if (pBindInfos[index0].memory) {
                    local_pBindInfos[index0].memory = Unwrap(pBindInfos[index0].memory);
                }
            }
        }
    }
    VkResult result = device_dispatch_table.BindImageMemory2(device, bindInfoCount, (const VkBindImageMemoryInfo*)local_pBindInfos);

    return result;
}

void DispatchObject::GetDeviceGroupPeerMemoryFeatures(VkDevice device, uint32_t heapIndex, uint32_t localDeviceIndex,
                                                      uint32_t remoteDeviceIndex, VkPeerMemoryFeatureFlags* pPeerMemoryFeatures) {
    device_dispatch_table.GetDeviceGroupPeerMemoryFeatures(device, heapIndex, localDeviceIndex, remoteDeviceIndex,
                                                           pPeerMemoryFeatures);
}

void DispatchObject::CmdSetDeviceMask(VkCommandBuffer commandBuffer, uint32_t deviceMask) {
    device_dispatch_table.CmdSetDeviceMask(commandBuffer, deviceMask);
}

void DispatchObject::CmdDispatchBase(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY, uint32_t baseGroupZ,
                                     uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) {
    device_dispatch_table.CmdDispatchBase(commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ);
}

VkResult DispatchObject::EnumeratePhysicalDeviceGroups(VkInstance instance, uint32_t* pPhysicalDeviceGroupCount,
                                                       VkPhysicalDeviceGroupProperties* pPhysicalDeviceGroupProperties) {
    VkResult result =
        instance_dispatch_table.EnumeratePhysicalDeviceGroups(instance, pPhysicalDeviceGroupCount, pPhysicalDeviceGroupProperties);

    return result;
}

void DispatchObject::GetImageMemoryRequirements2(VkDevice device, const VkImageMemoryRequirementsInfo2* pInfo,
                                                 VkMemoryRequirements2* pMemoryRequirements) {
    if (!wrap_handles) return device_dispatch_table.GetImageMemoryRequirements2(device, pInfo, pMemoryRequirements);
    vku::safe_VkImageMemoryRequirementsInfo2 var_local_pInfo;
    vku::safe_VkImageMemoryRequirementsInfo2* local_pInfo = nullptr;
    {
        if (pInfo) {
            local_pInfo = &var_local_pInfo;
            local_pInfo->initialize(pInfo);

            if (pInfo->image) {
                local_pInfo->image = Unwrap(pInfo->image);
            }
        }
    }
    device_dispatch_table.GetImageMemoryRequirements2(device, (const VkImageMemoryRequirementsInfo2*)local_pInfo,
                                                      pMemoryRequirements);
}

void DispatchObject::GetBufferMemoryRequirements2(VkDevice device, const VkBufferMemoryRequirementsInfo2* pInfo,
                                                  VkMemoryRequirements2* pMemoryRequirements) {
    if (!wrap_handles) return device_dispatch_table.GetBufferMemoryRequirements2(device, pInfo, pMemoryRequirements);
    vku::safe_VkBufferMemoryRequirementsInfo2 var_local_pInfo;
    vku::safe_VkBufferMemoryRequirementsInfo2* local_pInfo = nullptr;
    {
        if (pInfo) {
            local_pInfo = &var_local_pInfo;
            local_pInfo->initialize(pInfo);

            if (pInfo->buffer) {
                local_pInfo->buffer = Unwrap(pInfo->buffer);
            }
        }
    }
    device_dispatch_table.GetBufferMemoryRequirements2(device, (const VkBufferMemoryRequirementsInfo2*)local_pInfo,
                                                       pMemoryRequirements);
}

void DispatchObject::GetImageSparseMemoryRequirements2(VkDevice device, const VkImageSparseMemoryRequirementsInfo2* pInfo,
                                                       uint32_t* pSparseMemoryRequirementCount,
                                                       VkSparseImageMemoryRequirements2* pSparseMemoryRequirements) {
    if (!wrap_handles)
        return device_dispatch_table.GetImageSparseMemoryRequirements2(device, pInfo, pSparseMemoryRequirementCount,
                                                                       pSparseMemoryRequirements);
    vku::safe_VkImageSparseMemoryRequirementsInfo2 var_local_pInfo;
    vku::safe_VkImageSparseMemoryRequirementsInfo2* local_pInfo = nullptr;
    {
        if (pInfo) {
            local_pInfo = &var_local_pInfo;
            local_pInfo->initialize(pInfo);

            if (pInfo->image) {
                local_pInfo->image = Unwrap(pInfo->image);
            }
        }
    }
    device_dispatch_table.GetImageSparseMemoryRequirements2(device, (const VkImageSparseMemoryRequirementsInfo2*)local_pInfo,
                                                            pSparseMemoryRequirementCount, pSparseMemoryRequirements);
}

void DispatchObject::GetPhysicalDeviceFeatures2(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures2* pFeatures) {
    instance_dispatch_table.GetPhysicalDeviceFeatures2(physicalDevice, pFeatures);
}

void DispatchObject::GetPhysicalDeviceProperties2(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties2* pProperties) {
    instance_dispatch_table.GetPhysicalDeviceProperties2(physicalDevice, pProperties);
}

void DispatchObject::GetPhysicalDeviceFormatProperties2(VkPhysicalDevice physicalDevice, VkFormat format,
                                                        VkFormatProperties2* pFormatProperties) {
    instance_dispatch_table.GetPhysicalDeviceFormatProperties2(physicalDevice, format, pFormatProperties);
}

VkResult DispatchObject::GetPhysicalDeviceImageFormatProperties2(VkPhysicalDevice physicalDevice,
                                                                 const VkPhysicalDeviceImageFormatInfo2* pImageFormatInfo,
                                                                 VkImageFormatProperties2* pImageFormatProperties) {
    VkResult result =
        instance_dispatch_table.GetPhysicalDeviceImageFormatProperties2(physicalDevice, pImageFormatInfo, pImageFormatProperties);

    return result;
}

void DispatchObject::GetPhysicalDeviceQueueFamilyProperties2(VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount,
                                                             VkQueueFamilyProperties2* pQueueFamilyProperties) {
    instance_dispatch_table.GetPhysicalDeviceQueueFamilyProperties2(physicalDevice, pQueueFamilyPropertyCount,
                                                                    pQueueFamilyProperties);
}

void DispatchObject::GetPhysicalDeviceMemoryProperties2(VkPhysicalDevice physicalDevice,
                                                        VkPhysicalDeviceMemoryProperties2* pMemoryProperties) {
    instance_dispatch_table.GetPhysicalDeviceMemoryProperties2(physicalDevice, pMemoryProperties);
}

void DispatchObject::GetPhysicalDeviceSparseImageFormatProperties2(VkPhysicalDevice physicalDevice,
                                                                   const VkPhysicalDeviceSparseImageFormatInfo2* pFormatInfo,
                                                                   uint32_t* pPropertyCount,
                                                                   VkSparseImageFormatProperties2* pProperties) {
    instance_dispatch_table.GetPhysicalDeviceSparseImageFormatProperties2(physicalDevice, pFormatInfo, pPropertyCount, pProperties);
}

void DispatchObject::TrimCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolTrimFlags flags) {
    if (!wrap_handles) return device_dispatch_table.TrimCommandPool(device, commandPool, flags);
    { commandPool = Unwrap(commandPool); }
    device_dispatch_table.TrimCommandPool(device, commandPool, flags);
}

void DispatchObject::GetDeviceQueue2(VkDevice device, const VkDeviceQueueInfo2* pQueueInfo, VkQueue* pQueue) {
    device_dispatch_table.GetDeviceQueue2(device, pQueueInfo, pQueue);
}

VkResult DispatchObject::CreateSamplerYcbcrConversion(VkDevice device, const VkSamplerYcbcrConversionCreateInfo* pCreateInfo,
                                                      const VkAllocationCallbacks* pAllocator,
                                                      VkSamplerYcbcrConversion* pYcbcrConversion) {
    if (!wrap_handles) return device_dispatch_table.CreateSamplerYcbcrConversion(device, pCreateInfo, pAllocator, pYcbcrConversion);

    VkResult result = device_dispatch_table.CreateSamplerYcbcrConversion(device, pCreateInfo, pAllocator, pYcbcrConversion);
    if (VK_SUCCESS == result) {
        *pYcbcrConversion = WrapNew(*pYcbcrConversion);
    }
    return result;
}

void DispatchObject::DestroySamplerYcbcrConversion(VkDevice device, VkSamplerYcbcrConversion ycbcrConversion,
                                                   const VkAllocationCallbacks* pAllocator) {
    if (!wrap_handles) return device_dispatch_table.DestroySamplerYcbcrConversion(device, ycbcrConversion, pAllocator);

    uint64_t ycbcrConversion_id = CastToUint64(ycbcrConversion);
    auto iter = unique_id_mapping.pop(ycbcrConversion_id);
    if (iter != unique_id_mapping.end()) {
        ycbcrConversion = (VkSamplerYcbcrConversion)iter->second;
    } else {
        ycbcrConversion = (VkSamplerYcbcrConversion)0;
    }
    device_dispatch_table.DestroySamplerYcbcrConversion(device, ycbcrConversion, pAllocator);
}

void DispatchObject::GetPhysicalDeviceExternalBufferProperties(VkPhysicalDevice physicalDevice,
                                                               const VkPhysicalDeviceExternalBufferInfo* pExternalBufferInfo,
                                                               VkExternalBufferProperties* pExternalBufferProperties) {
    instance_dispatch_table.GetPhysicalDeviceExternalBufferProperties(physicalDevice, pExternalBufferInfo,
                                                                      pExternalBufferProperties);
}

void DispatchObject::GetPhysicalDeviceExternalFenceProperties(VkPhysicalDevice physicalDevice,
                                                              const VkPhysicalDeviceExternalFenceInfo* pExternalFenceInfo,
                                                              VkExternalFenceProperties* pExternalFenceProperties) {
    instance_dispatch_table.GetPhysicalDeviceExternalFenceProperties(physicalDevice, pExternalFenceInfo, pExternalFenceProperties);
}

void DispatchObject::GetPhysicalDeviceExternalSemaphoreProperties(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalSemaphoreInfo* pExternalSemaphoreInfo,
    VkExternalSemaphoreProperties* pExternalSemaphoreProperties) {
    instance_dispatch_table.GetPhysicalDeviceExternalSemaphoreProperties(physicalDevice, pExternalSemaphoreInfo,
                                                                         pExternalSemaphoreProperties);
}

void DispatchObject::GetDescriptorSetLayoutSupport(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo,
                                                   VkDescriptorSetLayoutSupport* pSupport) {
    if (!wrap_handles) return device_dispatch_table.GetDescriptorSetLayoutSupport(device, pCreateInfo, pSupport);
    vku::safe_VkDescriptorSetLayoutCreateInfo var_local_pCreateInfo;
    vku::safe_VkDescriptorSetLayoutCreateInfo* local_pCreateInfo = nullptr;
    {
        if (pCreateInfo) {
            local_pCreateInfo = &var_local_pCreateInfo;
            local_pCreateInfo->initialize(pCreateInfo);
            if (local_pCreateInfo->pBindings) {
                for (uint32_t index1 = 0; index1 < local_pCreateInfo->bindingCount; ++index1) {
                    if (local_pCreateInfo->pBindings[index1].pImmutableSamplers) {
                        for (uint32_t index2 = 0; index2 < local_pCreateInfo->pBindings[index1].descriptorCount; ++index2) {
                            local_pCreateInfo->pBindings[index1].pImmutableSamplers[index2] =
                                Unwrap(local_pCreateInfo->pBindings[index1].pImmutableSamplers[index2]);
                        }
                    }
                }
            }
        }
    }
    device_dispatch_table.GetDescriptorSetLayoutSupport(device, (const VkDescriptorSetLayoutCreateInfo*)local_pCreateInfo,
                                                        pSupport);
}

void DispatchObject::CmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer,
                                          VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride) {
    if (!wrap_handles)
        return device_dispatch_table.CmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset,
                                                          maxDrawCount, stride);
    {
        buffer = Unwrap(buffer);
        countBuffer = Unwrap(countBuffer);
    }
    device_dispatch_table.CmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
}

void DispatchObject::CmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                 VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                 uint32_t stride) {
    if (!wrap_handles)
        return device_dispatch_table.CmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset,
                                                                 maxDrawCount, stride);
    {
        buffer = Unwrap(buffer);
        countBuffer = Unwrap(countBuffer);
    }
    device_dispatch_table.CmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount,
                                                      stride);
}

void DispatchObject::CmdBeginRenderPass2(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                         const VkSubpassBeginInfo* pSubpassBeginInfo) {
    if (!wrap_handles) return device_dispatch_table.CmdBeginRenderPass2(commandBuffer, pRenderPassBegin, pSubpassBeginInfo);
    vku::safe_VkRenderPassBeginInfo var_local_pRenderPassBegin;
    vku::safe_VkRenderPassBeginInfo* local_pRenderPassBegin = nullptr;
    {
        if (pRenderPassBegin) {
            local_pRenderPassBegin = &var_local_pRenderPassBegin;
            local_pRenderPassBegin->initialize(pRenderPassBegin);

            if (pRenderPassBegin->renderPass) {
                local_pRenderPassBegin->renderPass = Unwrap(pRenderPassBegin->renderPass);
            }
            if (pRenderPassBegin->framebuffer) {
                local_pRenderPassBegin->framebuffer = Unwrap(pRenderPassBegin->framebuffer);
            }
            UnwrapPnextChainHandles(local_pRenderPassBegin->pNext);
        }
    }
    device_dispatch_table.CmdBeginRenderPass2(commandBuffer, (const VkRenderPassBeginInfo*)local_pRenderPassBegin,
                                              pSubpassBeginInfo);
}

void DispatchObject::CmdNextSubpass2(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo* pSubpassBeginInfo,
                                     const VkSubpassEndInfo* pSubpassEndInfo) {
    device_dispatch_table.CmdNextSubpass2(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo);
}

void DispatchObject::CmdEndRenderPass2(VkCommandBuffer commandBuffer, const VkSubpassEndInfo* pSubpassEndInfo) {
    device_dispatch_table.CmdEndRenderPass2(commandBuffer, pSubpassEndInfo);
}

void DispatchObject::ResetQueryPool(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount) {
    if (!wrap_handles) return device_dispatch_table.ResetQueryPool(device, queryPool, firstQuery, queryCount);
    { queryPool = Unwrap(queryPool); }
    device_dispatch_table.ResetQueryPool(device, queryPool, firstQuery, queryCount);
}

VkResult DispatchObject::GetSemaphoreCounterValue(VkDevice device, VkSemaphore semaphore, uint64_t* pValue) {
    if (!wrap_handles) return device_dispatch_table.GetSemaphoreCounterValue(device, semaphore, pValue);
    { semaphore = Unwrap(semaphore); }
    VkResult result = device_dispatch_table.GetSemaphoreCounterValue(device, semaphore, pValue);

    return result;
}

VkResult DispatchObject::WaitSemaphores(VkDevice device, const VkSemaphoreWaitInfo* pWaitInfo, uint64_t timeout) {
    if (!wrap_handles) return device_dispatch_table.WaitSemaphores(device, pWaitInfo, timeout);
    vku::safe_VkSemaphoreWaitInfo var_local_pWaitInfo;
    vku::safe_VkSemaphoreWaitInfo* local_pWaitInfo = nullptr;
    {
        if (pWaitInfo) {
            local_pWaitInfo = &var_local_pWaitInfo;
            local_pWaitInfo->initialize(pWaitInfo);
            if (local_pWaitInfo->pSemaphores) {
                for (uint32_t index1 = 0; index1 < local_pWaitInfo->semaphoreCount; ++index1) {
                    local_pWaitInfo->pSemaphores[index1] = Unwrap(local_pWaitInfo->pSemaphores[index1]);
                }
            }
        }
    }
    VkResult result = device_dispatch_table.WaitSemaphores(device, (const VkSemaphoreWaitInfo*)local_pWaitInfo, timeout);

    return result;
}

VkResult DispatchObject::SignalSemaphore(VkDevice device, const VkSemaphoreSignalInfo* pSignalInfo) {
    if (!wrap_handles) return device_dispatch_table.SignalSemaphore(device, pSignalInfo);
    vku::safe_VkSemaphoreSignalInfo var_local_pSignalInfo;
    vku::safe_VkSemaphoreSignalInfo* local_pSignalInfo = nullptr;
    {
        if (pSignalInfo) {
            local_pSignalInfo = &var_local_pSignalInfo;
            local_pSignalInfo->initialize(pSignalInfo);

            if (pSignalInfo->semaphore) {
                local_pSignalInfo->semaphore = Unwrap(pSignalInfo->semaphore);
            }
        }
    }
    VkResult result = device_dispatch_table.SignalSemaphore(device, (const VkSemaphoreSignalInfo*)local_pSignalInfo);

    return result;
}

VkDeviceAddress DispatchObject::GetBufferDeviceAddress(VkDevice device, const VkBufferDeviceAddressInfo* pInfo) {
    if (!wrap_handles) return device_dispatch_table.GetBufferDeviceAddress(device, pInfo);
    vku::safe_VkBufferDeviceAddressInfo var_local_pInfo;
    vku::safe_VkBufferDeviceAddressInfo* local_pInfo = nullptr;
    {
        if (pInfo) {
            local_pInfo = &var_local_pInfo;
            local_pInfo->initialize(pInfo);

            if (pInfo->buffer) {
                local_pInfo->buffer = Unwrap(pInfo->buffer);
            }
        }
    }
    VkDeviceAddress result = device_dispatch_table.GetBufferDeviceAddress(device, (const VkBufferDeviceAddressInfo*)local_pInfo);

    return result;
}

uint64_t DispatchObject::GetBufferOpaqueCaptureAddress(VkDevice device, const VkBufferDeviceAddressInfo* pInfo) {
    if (!wrap_handles) return device_dispatch_table.GetBufferOpaqueCaptureAddress(device, pInfo);
    vku::safe_VkBufferDeviceAddressInfo var_local_pInfo;
    vku::safe_VkBufferDeviceAddressInfo* local_pInfo = nullptr;
    {
        if (pInfo) {
            local_pInfo = &var_local_pInfo;
            local_pInfo->initialize(pInfo);

            if (pInfo->buffer) {
                local_pInfo->buffer = Unwrap(pInfo->buffer);
            }
        }
    }
    uint64_t result = device_dispatch_table.GetBufferOpaqueCaptureAddress(device, (const VkBufferDeviceAddressInfo*)local_pInfo);

    return result;
}

uint64_t DispatchObject::GetDeviceMemoryOpaqueCaptureAddress(VkDevice device, const VkDeviceMemoryOpaqueCaptureAddressInfo* pInfo) {
    if (!wrap_handles) return device_dispatch_table.GetDeviceMemoryOpaqueCaptureAddress(device, pInfo);
    vku::safe_VkDeviceMemoryOpaqueCaptureAddressInfo var_local_pInfo;
    vku::safe_VkDeviceMemoryOpaqueCaptureAddressInfo* local_pInfo = nullptr;
    {
        if (pInfo) {
            local_pInfo = &var_local_pInfo;
            local_pInfo->initialize(pInfo);

            if (pInfo->memory) {
                local_pInfo->memory = Unwrap(pInfo->memory);
            }
        }
    }
    uint64_t result = device_dispatch_table.GetDeviceMemoryOpaqueCaptureAddress(
        device, (const VkDeviceMemoryOpaqueCaptureAddressInfo*)local_pInfo);

    return result;
}

VkResult DispatchObject::CreatePrivateDataSlot(VkDevice device, const VkPrivateDataSlotCreateInfo* pCreateInfo,
                                               const VkAllocationCallbacks* pAllocator, VkPrivateDataSlot* pPrivateDataSlot) {
    if (!wrap_handles) return device_dispatch_table.CreatePrivateDataSlot(device, pCreateInfo, pAllocator, pPrivateDataSlot);

    VkResult result = device_dispatch_table.CreatePrivateDataSlot(device, pCreateInfo, pAllocator, pPrivateDataSlot);
    if (VK_SUCCESS == result) {
        *pPrivateDataSlot = WrapNew(*pPrivateDataSlot);
    }
    return result;
}

void DispatchObject::DestroyPrivateDataSlot(VkDevice device, VkPrivateDataSlot privateDataSlot,
                                            const VkAllocationCallbacks* pAllocator) {
    if (!wrap_handles) return device_dispatch_table.DestroyPrivateDataSlot(device, privateDataSlot, pAllocator);

    uint64_t privateDataSlot_id = CastToUint64(privateDataSlot);
    auto iter = unique_id_mapping.pop(privateDataSlot_id);
    if (iter != unique_id_mapping.end()) {
        privateDataSlot = (VkPrivateDataSlot)iter->second;
    } else {
        privateDataSlot = (VkPrivateDataSlot)0;
    }
    device_dispatch_table.DestroyPrivateDataSlot(device, privateDataSlot, pAllocator);
}

VkResult DispatchObject::SetPrivateData(VkDevice device, VkObjectType objectType, uint64_t objectHandle,
                                        VkPrivateDataSlot privateDataSlot, uint64_t data) {
    if (!wrap_handles) return device_dispatch_table.SetPrivateData(device, objectType, objectHandle, privateDataSlot, data);
    {
        if (NotDispatchableHandle(objectType)) {
            objectHandle = Unwrap(objectHandle);
        }
        privateDataSlot = Unwrap(privateDataSlot);
    }
    VkResult result = device_dispatch_table.SetPrivateData(device, objectType, objectHandle, privateDataSlot, data);

    return result;
}

void DispatchObject::GetPrivateData(VkDevice device, VkObjectType objectType, uint64_t objectHandle,
                                    VkPrivateDataSlot privateDataSlot, uint64_t* pData) {
    if (!wrap_handles) return device_dispatch_table.GetPrivateData(device, objectType, objectHandle, privateDataSlot, pData);
    {
        if (NotDispatchableHandle(objectType)) {
            objectHandle = Unwrap(objectHandle);
        }
        privateDataSlot = Unwrap(privateDataSlot);
    }
    device_dispatch_table.GetPrivateData(device, objectType, objectHandle, privateDataSlot, pData);
}

void DispatchObject::CmdSetEvent2(VkCommandBuffer commandBuffer, VkEvent event, const VkDependencyInfo* pDependencyInfo) {
    if (!wrap_handles) return device_dispatch_table.CmdSetEvent2(commandBuffer, event, pDependencyInfo);
    vku::safe_VkDependencyInfo var_local_pDependencyInfo;
    vku::safe_VkDependencyInfo* local_pDependencyInfo = nullptr;
    {
        event = Unwrap(event);
        if (pDependencyInfo) {
            local_pDependencyInfo = &var_local_pDependencyInfo;
            local_pDependencyInfo->initialize(pDependencyInfo);
            if (local_pDependencyInfo->pBufferMemoryBarriers) {
                for (uint32_t index1 = 0; index1 < local_pDependencyInfo->bufferMemoryBarrierCount; ++index1) {
                    if (pDependencyInfo->pBufferMemoryBarriers[index1].buffer) {
                        local_pDependencyInfo->pBufferMemoryBarriers[index1].buffer =
                            Unwrap(pDependencyInfo->pBufferMemoryBarriers[index1].buffer);
                    }
                }
            }
            if (local_pDependencyInfo->pImageMemoryBarriers) {
                for (uint32_t index1 = 0; index1 < local_pDependencyInfo->imageMemoryBarrierCount; ++index1) {
                    if (pDependencyInfo->pImageMemoryBarriers[index1].image) {
                        local_pDependencyInfo->pImageMemoryBarriers[index1].image =
                            Unwrap(pDependencyInfo->pImageMemoryBarriers[index1].image);
                    }
                }
            }
        }
    }
    device_dispatch_table.CmdSetEvent2(commandBuffer, event, (const VkDependencyInfo*)local_pDependencyInfo);
}

void DispatchObject::CmdResetEvent2(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags2 stageMask) {
    if (!wrap_handles) return device_dispatch_table.CmdResetEvent2(commandBuffer, event, stageMask);
    { event = Unwrap(event); }
    device_dispatch_table.CmdResetEvent2(commandBuffer, event, stageMask);
}

void DispatchObject::CmdWaitEvents2(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                    const VkDependencyInfo* pDependencyInfos) {
    if (!wrap_handles) return device_dispatch_table.CmdWaitEvents2(commandBuffer, eventCount, pEvents, pDependencyInfos);
    small_vector<VkEvent, DISPATCH_MAX_STACK_ALLOCATIONS> var_local_pEvents;
    VkEvent* local_pEvents = nullptr;
    small_vector<vku::safe_VkDependencyInfo, DISPATCH_MAX_STACK_ALLOCATIONS> var_local_pDependencyInfos;
    vku::safe_VkDependencyInfo* local_pDependencyInfos = nullptr;
    {
        if (pEvents) {
            var_local_pEvents.resize(eventCount);
            local_pEvents = var_local_pEvents.data();
            for (uint32_t index0 = 0; index0 < eventCount; ++index0) {
                local_pEvents[index0] = Unwrap(pEvents[index0]);
            }
        }
        if (pDependencyInfos) {
            var_local_pDependencyInfos.resize(eventCount);
            local_pDependencyInfos = var_local_pDependencyInfos.data();
            for (uint32_t index0 = 0; index0 < eventCount; ++index0) {
                local_pDependencyInfos[index0].initialize(&pDependencyInfos[index0]);
                if (local_pDependencyInfos[index0].pBufferMemoryBarriers) {
                    for (uint32_t index1 = 0; index1 < local_pDependencyInfos[index0].bufferMemoryBarrierCount; ++index1) {
                        if (pDependencyInfos[index0].pBufferMemoryBarriers[index1].buffer) {
                            local_pDependencyInfos[index0].pBufferMemoryBarriers[index1].buffer =
                                Unwrap(pDependencyInfos[index0].pBufferMemoryBarriers[index1].buffer);
                        }
                    }
                }
                if (local_pDependencyInfos[index0].pImageMemoryBarriers) {
                    for (uint32_t index1 = 0; index1 < local_pDependencyInfos[index0].imageMemoryBarrierCount; ++index1) {
                        if (pDependencyInfos[index0].pImageMemoryBarriers[index1].image) {
                            local_pDependencyInfos[index0].pImageMemoryBarriers[index1].image =
                                Unwrap(pDependencyInfos[index0].pImageMemoryBarriers[index1].image);
                        }
                    }
                }
            }
        }
    }
    device_dispatch_table.CmdWaitEvents2(commandBuffer, eventCount, (const VkEvent*)local_pEvents,
                                         (const VkDependencyInfo*)local_pDependencyInfos);
}

void DispatchObject::CmdPipelineBarrier2(VkCommandBuffer commandBuffer, const VkDependencyInfo* pDependencyInfo) {
    if (!wrap_handles) return device_dispatch_table.CmdPipelineBarrier2(commandBuffer, pDependencyInfo);
    vku::safe_VkDependencyInfo var_local_pDependencyInfo;
    vku::safe_VkDependencyInfo* local_pDependencyInfo = nullptr;
    {
        if (pDependencyInfo) {
            local_pDependencyInfo = &var_local_pDependencyInfo;
            local_pDependencyInfo->initialize(pDependencyInfo);
            if (local_pDependencyInfo->pBufferMemoryBarriers) {
                for (uint32_t index1 = 0; index1 < local_pDependencyInfo->bufferMemoryBarrierCount; ++index1) {
                    if (pDependencyInfo->pBufferMemoryBarriers[index1].buffer) {
                        local_pDependencyInfo->pBufferMemoryBarriers[index1].buffer =
                            Unwrap(pDependencyInfo->pBufferMemoryBarriers[index1].buffer);
                    }
                }
            }
            if (local_pDependencyInfo->pImageMemoryBarriers) {
                for (uint32_t index1 = 0; index1 < local_pDependencyInfo->imageMemoryBarrierCount; ++index1) {
                    if (pDependencyInfo->pImageMemoryBarriers[index1].image) {
                        local_pDependencyInfo->pImageMemoryBarriers[index1].image =
                            Unwrap(pDependencyInfo->pImageMemoryBarriers[index1].image);
                    }
                }
            }
        }
    }
    device_dispatch_table.CmdPipelineBarrier2(commandBuffer, (const VkDependencyInfo*)local_pDependencyInfo);
}

void DispatchObject::CmdWriteTimestamp2(VkCommandBuffer commandBuffer, VkPipelineStageFlags2 stage, VkQueryPool queryPool,
                                        uint32_t query) {
    if (!wrap_handles) return device_dispatch_table.CmdWriteTimestamp2(commandBuffer, stage, queryPool, query);
    { queryPool = Unwrap(queryPool); }
    device_dispatch_table.CmdWriteTimestamp2(commandBuffer, stage, queryPool, query);
}

VkResult DispatchObject::QueueSubmit2(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2* pSubmits, VkFence fence) {
    if (!wrap_handles) return device_dispatch_table.QueueSubmit2(queue, submitCount, pSubmits, fence);
    small_vector<vku::safe_VkSubmitInfo2, DISPATCH_MAX_STACK_ALLOCATIONS> var_local_pSubmits;
    vku::safe_VkSubmitInfo2* local_pSubmits = nullptr;
    {
        if (pSubmits) {
            var_local_pSubmits.resize(submitCount);
            local_pSubmits = var_local_pSubmits.data();
            for (uint32_t index0 = 0; index0 < submitCount; ++index0) {
                local_pSubmits[index0].initialize(&pSubmits[index0]);
                UnwrapPnextChainHandles(local_pSubmits[index0].pNext);
                if (local_pSubmits[index0].pWaitSemaphoreInfos) {
                    for (uint32_t index1 = 0; index1 < local_pSubmits[index0].waitSemaphoreInfoCount; ++index1) {
                        if (pSubmits[index0].pWaitSemaphoreInfos[index1].semaphore) {
                            local_pSubmits[index0].pWaitSemaphoreInfos[index1].semaphore =
                                Unwrap(pSubmits[index0].pWaitSemaphoreInfos[index1].semaphore);
                        }
                    }
                }
                if (local_pSubmits[index0].pCommandBufferInfos) {
                    for (uint32_t index1 = 0; index1 < local_pSubmits[index0].commandBufferInfoCount; ++index1) {
                        UnwrapPnextChainHandles(local_pSubmits[index0].pCommandBufferInfos[index1].pNext);
                    }
                }
                if (local_pSubmits[index0].pSignalSemaphoreInfos) {
                    for (uint32_t index1 = 0; index1 < local_pSubmits[index0].signalSemaphoreInfoCount; ++index1) {
                        if (pSubmits[index0].pSignalSemaphoreInfos[index1].semaphore) {
                            local_pSubmits[index0].pSignalSemaphoreInfos[index1].semaphore =
                                Unwrap(pSubmits[index0].pSignalSemaphoreInfos[index1].semaphore);
                        }
                    }
                }
            }
        }
        fence = Unwrap(fence);
    }
    VkResult result = device_dispatch_table.QueueSubmit2(queue, submitCount, (const VkSubmitInfo2*)local_pSubmits, fence);

    return result;
}

void DispatchObject::CmdCopyBuffer2(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2* pCopyBufferInfo) {
    if (!wrap_handles) return device_dispatch_table.CmdCopyBuffer2(commandBuffer, pCopyBufferInfo);
    vku::safe_VkCopyBufferInfo2 var_local_pCopyBufferInfo;
    vku::safe_VkCopyBufferInfo2* local_pCopyBufferInfo = nullptr;
    {
        if (pCopyBufferInfo) {
            local_pCopyBufferInfo = &var_local_pCopyBufferInfo;
            local_pCopyBufferInfo->initialize(pCopyBufferInfo);

            if (pCopyBufferInfo->srcBuffer) {
                local_pCopyBufferInfo->srcBuffer = Unwrap(pCopyBufferInfo->srcBuffer);
            }
            if (pCopyBufferInfo->dstBuffer) {
                local_pCopyBufferInfo->dstBuffer = Unwrap(pCopyBufferInfo->dstBuffer);
            }
        }
    }
    device_dispatch_table.CmdCopyBuffer2(commandBuffer, (const VkCopyBufferInfo2*)local_pCopyBufferInfo);
}

void DispatchObject::CmdCopyImage2(VkCommandBuffer commandBuffer, const VkCopyImageInfo2* pCopyImageInfo) {
    if (!wrap_handles) return device_dispatch_table.CmdCopyImage2(commandBuffer, pCopyImageInfo);
    vku::safe_VkCopyImageInfo2 var_local_pCopyImageInfo;
    vku::safe_VkCopyImageInfo2* local_pCopyImageInfo = nullptr;
    {
        if (pCopyImageInfo) {
            local_pCopyImageInfo = &var_local_pCopyImageInfo;
            local_pCopyImageInfo->initialize(pCopyImageInfo);

            if (pCopyImageInfo->srcImage) {
                local_pCopyImageInfo->srcImage = Unwrap(pCopyImageInfo->srcImage);
            }
            if (pCopyImageInfo->dstImage) {
                local_pCopyImageInfo->dstImage = Unwrap(pCopyImageInfo->dstImage);
            }
        }
    }
    device_dispatch_table.CmdCopyImage2(commandBuffer, (const VkCopyImageInfo2*)local_pCopyImageInfo);
}

void DispatchObject::CmdCopyBufferToImage2(VkCommandBuffer commandBuffer, const VkCopyBufferToImageInfo2* pCopyBufferToImageInfo) {
    if (!wrap_handles) return device_dispatch_table.CmdCopyBufferToImage2(commandBuffer, pCopyBufferToImageInfo);
    vku::safe_VkCopyBufferToImageInfo2 var_local_pCopyBufferToImageInfo;
    vku::safe_VkCopyBufferToImageInfo2* local_pCopyBufferToImageInfo = nullptr;
    {
        if (pCopyBufferToImageInfo) {
            local_pCopyBufferToImageInfo = &var_local_pCopyBufferToImageInfo;
            local_pCopyBufferToImageInfo->initialize(pCopyBufferToImageInfo);

            if (pCopyBufferToImageInfo->srcBuffer) {
                local_pCopyBufferToImageInfo->srcBuffer = Unwrap(pCopyBufferToImageInfo->srcBuffer);
            }
            if (pCopyBufferToImageInfo->dstImage) {
                local_pCopyBufferToImageInfo->dstImage = Unwrap(pCopyBufferToImageInfo->dstImage);
            }
        }
    }
    device_dispatch_table.CmdCopyBufferToImage2(commandBuffer, (const VkCopyBufferToImageInfo2*)local_pCopyBufferToImageInfo);
}

void DispatchObject::CmdCopyImageToBuffer2(VkCommandBuffer commandBuffer, const VkCopyImageToBufferInfo2* pCopyImageToBufferInfo) {
    if (!wrap_handles) return device_dispatch_table.CmdCopyImageToBuffer2(commandBuffer, pCopyImageToBufferInfo);
    vku::safe_VkCopyImageToBufferInfo2 var_local_pCopyImageToBufferInfo;
    vku::safe_VkCopyImageToBufferInfo2* local_pCopyImageToBufferInfo = nullptr;
    {
        if (pCopyImageToBufferInfo) {
            local_pCopyImageToBufferInfo = &var_local_pCopyImageToBufferInfo;
            local_pCopyImageToBufferInfo->initialize(pCopyImageToBufferInfo);

            if (pCopyImageToBufferInfo->srcImage) {
                local_pCopyImageToBufferInfo->srcImage = Unwrap(pCopyImageToBufferInfo->srcImage);
            }
            if (pCopyImageToBufferInfo->dstBuffer) {
                local_pCopyImageToBufferInfo->dstBuffer = Unwrap(pCopyImageToBufferInfo->dstBuffer);
            }
        }
    }
    device_dispatch_table.CmdCopyImageToBuffer2(commandBuffer, (const VkCopyImageToBufferInfo2*)local_pCopyImageToBufferInfo);
}

void DispatchObject::CmdBlitImage2(VkCommandBuffer commandBuffer, const VkBlitImageInfo2* pBlitImageInfo) {
    if (!wrap_handles) return device_dispatch_table.CmdBlitImage2(commandBuffer, pBlitImageInfo);
    vku::safe_VkBlitImageInfo2 var_local_pBlitImageInfo;
    vku::safe_VkBlitImageInfo2* local_pBlitImageInfo = nullptr;
    {
        if (pBlitImageInfo) {
            local_pBlitImageInfo = &var_local_pBlitImageInfo;
            local_pBlitImageInfo->initialize(pBlitImageInfo);

            if (pBlitImageInfo->srcImage) {
                local_pBlitImageInfo->srcImage = Unwrap(pBlitImageInfo->srcImage);
            }
            if (pBlitImageInfo->dstImage) {
                local_pBlitImageInfo->dstImage = Unwrap(pBlitImageInfo->dstImage);
            }
        }
    }
    device_dispatch_table.CmdBlitImage2(commandBuffer, (const VkBlitImageInfo2*)local_pBlitImageInfo);
}

void DispatchObject::CmdResolveImage2(VkCommandBuffer commandBuffer, const VkResolveImageInfo2* pResolveImageInfo) {
    if (!wrap_handles) return device_dispatch_table.CmdResolveImage2(commandBuffer, pResolveImageInfo);
    vku::safe_VkResolveImageInfo2 var_local_pResolveImageInfo;
    vku::safe_VkResolveImageInfo2* local_pResolveImageInfo = nullptr;
    {
        if (pResolveImageInfo) {
            local_pResolveImageInfo = &var_local_pResolveImageInfo;
            local_pResolveImageInfo->initialize(pResolveImageInfo);

            if (pResolveImageInfo->srcImage) {
                local_pResolveImageInfo->srcImage = Unwrap(pResolveImageInfo->srcImage);
            }
            if (pResolveImageInfo->dstImage) {
                local_pResolveImageInfo->dstImage = Unwrap(pResolveImageInfo->dstImage);
            }
        }
    }
    device_dispatch_table.CmdResolveImage2(commandBuffer, (const VkResolveImageInfo2*)local_pResolveImageInfo);
}

void DispatchObject::CmdBeginRendering(VkCommandBuffer commandBuffer, const VkRenderingInfo* pRenderingInfo) {
    if (!wrap_handles) return device_dispatch_table.CmdBeginRendering(commandBuffer, pRenderingInfo);
    vku::safe_VkRenderingInfo var_local_pRenderingInfo;
    vku::safe_VkRenderingInfo* local_pRenderingInfo = nullptr;
    {
        if (pRenderingInfo) {
            local_pRenderingInfo = &var_local_pRenderingInfo;
            local_pRenderingInfo->initialize(pRenderingInfo);
            if (local_pRenderingInfo->pColorAttachments) {
                for (uint32_t index1 = 0; index1 < local_pRenderingInfo->colorAttachmentCount; ++index1) {
                    if (pRenderingInfo->pColorAttachments[index1].imageView) {
                        local_pRenderingInfo->pColorAttachments[index1].imageView =
                            Unwrap(pRenderingInfo->pColorAttachments[index1].imageView);
                    }
                    if (pRenderingInfo->pColorAttachments[index1].resolveImageView) {
                        local_pRenderingInfo->pColorAttachments[index1].resolveImageView =
                            Unwrap(pRenderingInfo->pColorAttachments[index1].resolveImageView);
                    }
                }
            }
            if (local_pRenderingInfo->pDepthAttachment) {
                if (pRenderingInfo->pDepthAttachment->imageView) {
                    local_pRenderingInfo->pDepthAttachment->imageView = Unwrap(pRenderingInfo->pDepthAttachment->imageView);
                }
                if (pRenderingInfo->pDepthAttachment->resolveImageView) {
                    local_pRenderingInfo->pDepthAttachment->resolveImageView =
                        Unwrap(pRenderingInfo->pDepthAttachment->resolveImageView);
                }
            }
            if (local_pRenderingInfo->pStencilAttachment) {
                if (pRenderingInfo->pStencilAttachment->imageView) {
                    local_pRenderingInfo->pStencilAttachment->imageView = Unwrap(pRenderingInfo->pStencilAttachment->imageView);
                }
                if (pRenderingInfo->pStencilAttachment->resolveImageView) {
                    local_pRenderingInfo->pStencilAttachment->resolveImageView =
                        Unwrap(pRenderingInfo->pStencilAttachment->resolveImageView);
                }
            }
            UnwrapPnextChainHandles(local_pRenderingInfo->pNext);
        }
    }
    device_dispatch_table.CmdBeginRendering(commandBuffer, (const VkRenderingInfo*)local_pRenderingInfo);
}

void DispatchObject::CmdEndRendering(VkCommandBuffer commandBuffer) { device_dispatch_table.CmdEndRendering(commandBuffer); }

void DispatchObject::CmdSetCullMode(VkCommandBuffer commandBuffer, VkCullModeFlags cullMode) {
    device_dispatch_table.CmdSetCullMode(commandBuffer, cullMode);
}

void DispatchObject::CmdSetFrontFace(VkCommandBuffer commandBuffer, VkFrontFace frontFace) {
    device_dispatch_table.CmdSetFrontFace(commandBuffer, frontFace);
}

void DispatchObject::CmdSetPrimitiveTopology(VkCommandBuffer commandBuffer, VkPrimitiveTopology primitiveTopology) {
    device_dispatch_table.CmdSetPrimitiveTopology(commandBuffer, primitiveTopology);
}

void DispatchObject::CmdSetViewportWithCount(VkCommandBuffer commandBuffer, uint32_t viewportCount, const VkViewport* pViewports) {
    device_dispatch_table.CmdSetViewportWithCount(commandBuffer, viewportCount, pViewports);
}

void DispatchObject::CmdSetScissorWithCount(VkCommandBuffer commandBuffer, uint32_t scissorCount, const VkRect2D* pScissors) {
    device_dispatch_table.CmdSetScissorWithCount(commandBuffer, scissorCount, pScissors);
}

void DispatchObject::CmdBindVertexBuffers2(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount,
                                           const VkBuffer* pBuffers, const VkDeviceSize* pOffsets, const VkDeviceSize* pSizes,
                                           const VkDeviceSize* pStrides) {
    if (!wrap_handles)
        return device_dispatch_table.CmdBindVertexBuffers2(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets, pSizes,
                                                           pStrides);
    small_vector<VkBuffer, DISPATCH_MAX_STACK_ALLOCATIONS> var_local_pBuffers;
    VkBuffer* local_pBuffers = nullptr;
    {
        if (pBuffers) {
            var_local_pBuffers.resize(bindingCount);
            local_pBuffers = var_local_pBuffers.data();
            for (uint32_t index0 = 0; index0 < bindingCount; ++index0) {
                local_pBuffers[index0] = Unwrap(pBuffers[index0]);
            }
        }
    }
    device_dispatch_table.CmdBindVertexBuffers2(commandBuffer, firstBinding, bindingCount, (const VkBuffer*)local_pBuffers,
                                                pOffsets, pSizes, pStrides);
}

void DispatchObject::CmdSetDepthTestEnable(VkCommandBuffer commandBuffer, VkBool32 depthTestEnable) {
    device_dispatch_table.CmdSetDepthTestEnable(commandBuffer, depthTestEnable);
}

void DispatchObject::CmdSetDepthWriteEnable(VkCommandBuffer commandBuffer, VkBool32 depthWriteEnable) {
    device_dispatch_table.CmdSetDepthWriteEnable(commandBuffer, depthWriteEnable);
}

void DispatchObject::CmdSetDepthCompareOp(VkCommandBuffer commandBuffer, VkCompareOp depthCompareOp) {
    device_dispatch_table.CmdSetDepthCompareOp(commandBuffer, depthCompareOp);
}

void DispatchObject::CmdSetDepthBoundsTestEnable(VkCommandBuffer commandBuffer, VkBool32 depthBoundsTestEnable) {
    device_dispatch_table.CmdSetDepthBoundsTestEnable(commandBuffer, depthBoundsTestEnable);
}

void DispatchObject::CmdSetStencilTestEnable(VkCommandBuffer commandBuffer, VkBool32 stencilTestEnable) {
    device_dispatch_table.CmdSetStencilTestEnable(commandBuffer, stencilTestEnable);
}

void DispatchObject::CmdSetStencilOp(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, VkStencilOp failOp,
                                     VkStencilOp passOp, VkStencilOp depthFailOp, VkCompareOp compareOp) {
    device_dispatch_table.CmdSetStencilOp(commandBuffer, faceMask, failOp, passOp, depthFailOp, compareOp);
}

void DispatchObject::CmdSetRasterizerDiscardEnable(VkCommandBuffer commandBuffer, VkBool32 rasterizerDiscardEnable) {
    device_dispatch_table.CmdSetRasterizerDiscardEnable(commandBuffer, rasterizerDiscardEnable);
}

void DispatchObject::CmdSetDepthBiasEnable(VkCommandBuffer commandBuffer, VkBool32 depthBiasEnable) {
    device_dispatch_table.CmdSetDepthBiasEnable(commandBuffer, depthBiasEnable);
}

void DispatchObject::CmdSetPrimitiveRestartEnable(VkCommandBuffer commandBuffer, VkBool32 primitiveRestartEnable) {
    device_dispatch_table.CmdSetPrimitiveRestartEnable(commandBuffer, primitiveRestartEnable);
}

void DispatchObject::GetDeviceBufferMemoryRequirements(VkDevice device, const VkDeviceBufferMemoryRequirements* pInfo,
                                                       VkMemoryRequirements2* pMemoryRequirements) {
    device_dispatch_table.GetDeviceBufferMemoryRequirements(device, pInfo, pMemoryRequirements);
}

void DispatchObject::GetDeviceImageMemoryRequirements(VkDevice device, const VkDeviceImageMemoryRequirements* pInfo,
                                                      VkMemoryRequirements2* pMemoryRequirements) {
    device_dispatch_table.GetDeviceImageMemoryRequirements(device, pInfo, pMemoryRequirements);
}

void DispatchObject::GetDeviceImageSparseMemoryRequirements(VkDevice device, const VkDeviceImageMemoryRequirements* pInfo,
                                                            uint32_t* pSparseMemoryRequirementCount,
                                                            VkSparseImageMemoryRequirements2* pSparseMemoryRequirements) {
    device_dispatch_table.GetDeviceImageSparseMemoryRequirements(device, pInfo, pSparseMemoryRequirementCount,
                                                                 pSparseMemoryRequirements);
}

void DispatchObject::DestroySurfaceKHR(VkInstance instance, VkSurfaceKHR surface, const VkAllocationCallbacks* pAllocator) {
    if (!wrap_handles) return instance_dispatch_table.DestroySurfaceKHR(instance, surface, pAllocator);

    uint64_t surface_id = CastToUint64(surface);
    auto iter = unique_id_mapping.pop(surface_id);
    if (iter != unique_id_mapping.end()) {
        surface = (VkSurfaceKHR)iter->second;
    } else {
        surface = (VkSurfaceKHR)0;
    }
    instance_dispatch_table.DestroySurfaceKHR(instance, surface, pAllocator);
}

VkResult DispatchObject::GetPhysicalDeviceSurfaceSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
                                                            VkSurfaceKHR surface, VkBool32* pSupported) {
    if (!wrap_handles)
        return instance_dispatch_table.GetPhysicalDeviceSurfaceSupportKHR(physicalDevice, queueFamilyIndex, surface, pSupported);
    { surface = Unwrap(surface); }
    VkResult result =
        instance_dispatch_table.GetPhysicalDeviceSurfaceSupportKHR(physicalDevice, queueFamilyIndex, surface, pSupported);

    return result;
}

VkResult DispatchObject::GetPhysicalDeviceSurfaceCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                 VkSurfaceCapabilitiesKHR* pSurfaceCapabilities) {
    if (!wrap_handles)
        return instance_dispatch_table.GetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, pSurfaceCapabilities);
    { surface = Unwrap(surface); }
    VkResult result =
        instance_dispatch_table.GetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, pSurfaceCapabilities);

    return result;
}

VkResult DispatchObject::GetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                            uint32_t* pSurfaceFormatCount, VkSurfaceFormatKHR* pSurfaceFormats) {
    if (!wrap_handles)
        return instance_dispatch_table.GetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, pSurfaceFormatCount,
                                                                          pSurfaceFormats);
    { surface = Unwrap(surface); }
    VkResult result =
        instance_dispatch_table.GetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, pSurfaceFormatCount, pSurfaceFormats);

    return result;
}

VkResult DispatchObject::GetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                 uint32_t* pPresentModeCount, VkPresentModeKHR* pPresentModes) {
    if (!wrap_handles)
        return instance_dispatch_table.GetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, pPresentModeCount,
                                                                               pPresentModes);
    { surface = Unwrap(surface); }
    VkResult result =
        instance_dispatch_table.GetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, pPresentModeCount, pPresentModes);

    return result;
}

VkResult DispatchObject::CreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR* pCreateInfo,
                                            const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain) {
    if (!wrap_handles) return device_dispatch_table.CreateSwapchainKHR(device, pCreateInfo, pAllocator, pSwapchain);
    vku::safe_VkSwapchainCreateInfoKHR var_local_pCreateInfo;
    vku::safe_VkSwapchainCreateInfoKHR* local_pCreateInfo = nullptr;
    {
        if (pCreateInfo) {
            local_pCreateInfo = &var_local_pCreateInfo;
            local_pCreateInfo->initialize(pCreateInfo);

            if (pCreateInfo->surface) {
                local_pCreateInfo->surface = Unwrap(pCreateInfo->surface);
            }
            if (pCreateInfo->oldSwapchain) {
                local_pCreateInfo->oldSwapchain = Unwrap(pCreateInfo->oldSwapchain);
            }
        }
    }
    VkResult result = device_dispatch_table.CreateSwapchainKHR(device, (const VkSwapchainCreateInfoKHR*)local_pCreateInfo,
                                                               pAllocator, pSwapchain);
    if (VK_SUCCESS == result) {
        *pSwapchain = WrapNew(*pSwapchain);
    }
    return result;
}

VkResult DispatchObject::AcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore,
                                             VkFence fence, uint32_t* pImageIndex) {
    if (!wrap_handles) return device_dispatch_table.AcquireNextImageKHR(device, swapchain, timeout, semaphore, fence, pImageIndex);
    {
        swapchain = Unwrap(swapchain);
        semaphore = Unwrap(semaphore);
        fence = Unwrap(fence);
    }
    VkResult result = device_dispatch_table.AcquireNextImageKHR(device, swapchain, timeout, semaphore, fence, pImageIndex);

    return result;
}

VkResult DispatchObject::GetDeviceGroupPresentCapabilitiesKHR(
    VkDevice device, VkDeviceGroupPresentCapabilitiesKHR* pDeviceGroupPresentCapabilities) {
    VkResult result = device_dispatch_table.GetDeviceGroupPresentCapabilitiesKHR(device, pDeviceGroupPresentCapabilities);

    return result;
}

VkResult DispatchObject::GetDeviceGroupSurfacePresentModesKHR(VkDevice device, VkSurfaceKHR surface,
                                                              VkDeviceGroupPresentModeFlagsKHR* pModes) {
    if (!wrap_handles) return device_dispatch_table.GetDeviceGroupSurfacePresentModesKHR(device, surface, pModes);
    { surface = Unwrap(surface); }
    VkResult result = device_dispatch_table.GetDeviceGroupSurfacePresentModesKHR(device, surface, pModes);

    return result;
}

VkResult DispatchObject::GetPhysicalDevicePresentRectanglesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                               uint32_t* pRectCount, VkRect2D* pRects) {
    if (!wrap_handles)
        return instance_dispatch_table.GetPhysicalDevicePresentRectanglesKHR(physicalDevice, surface, pRectCount, pRects);
    { surface = Unwrap(surface); }
    VkResult result = instance_dispatch_table.GetPhysicalDevicePresentRectanglesKHR(physicalDevice, surface, pRectCount, pRects);

    return result;
}

VkResult DispatchObject::AcquireNextImage2KHR(VkDevice device, const VkAcquireNextImageInfoKHR* pAcquireInfo,
                                              uint32_t* pImageIndex) {
    if (!wrap_handles) return device_dispatch_table.AcquireNextImage2KHR(device, pAcquireInfo, pImageIndex);
    vku::safe_VkAcquireNextImageInfoKHR var_local_pAcquireInfo;
    vku::safe_VkAcquireNextImageInfoKHR* local_pAcquireInfo = nullptr;
    {
        if (pAcquireInfo) {
            local_pAcquireInfo = &var_local_pAcquireInfo;
            local_pAcquireInfo->initialize(pAcquireInfo);

            if (pAcquireInfo->swapchain) {
                local_pAcquireInfo->swapchain = Unwrap(pAcquireInfo->swapchain);
            }
            if (pAcquireInfo->semaphore) {
                local_pAcquireInfo->semaphore = Unwrap(pAcquireInfo->semaphore);
            }
            if (pAcquireInfo->fence) {
                local_pAcquireInfo->fence = Unwrap(pAcquireInfo->fence);
            }
        }
    }
    VkResult result =
        device_dispatch_table.AcquireNextImage2KHR(device, (const VkAcquireNextImageInfoKHR*)local_pAcquireInfo, pImageIndex);

    return result;
}

VkResult DispatchObject::CreateDisplayModeKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display,
                                              const VkDisplayModeCreateInfoKHR* pCreateInfo,
                                              const VkAllocationCallbacks* pAllocator, VkDisplayModeKHR* pMode) {
    if (!wrap_handles) return instance_dispatch_table.CreateDisplayModeKHR(physicalDevice, display, pCreateInfo, pAllocator, pMode);
    { display = Unwrap(display); }
    VkResult result = instance_dispatch_table.CreateDisplayModeKHR(physicalDevice, display, pCreateInfo, pAllocator, pMode);
    if (VK_SUCCESS == result) {
        *pMode = WrapNew(*pMode);
    }
    return result;
}

VkResult DispatchObject::GetDisplayPlaneCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkDisplayModeKHR mode, uint32_t planeIndex,
                                                        VkDisplayPlaneCapabilitiesKHR* pCapabilities) {
    if (!wrap_handles)
        return instance_dispatch_table.GetDisplayPlaneCapabilitiesKHR(physicalDevice, mode, planeIndex, pCapabilities);
    { mode = Unwrap(mode); }
    VkResult result = instance_dispatch_table.GetDisplayPlaneCapabilitiesKHR(physicalDevice, mode, planeIndex, pCapabilities);

    return result;
}

VkResult DispatchObject::CreateDisplayPlaneSurfaceKHR(VkInstance instance, const VkDisplaySurfaceCreateInfoKHR* pCreateInfo,
                                                      const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    if (!wrap_handles) return instance_dispatch_table.CreateDisplayPlaneSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
    vku::safe_VkDisplaySurfaceCreateInfoKHR var_local_pCreateInfo;
    vku::safe_VkDisplaySurfaceCreateInfoKHR* local_pCreateInfo = nullptr;
    {
        if (pCreateInfo) {
            local_pCreateInfo = &var_local_pCreateInfo;
            local_pCreateInfo->initialize(pCreateInfo);

            if (pCreateInfo->displayMode) {
                local_pCreateInfo->displayMode = Unwrap(pCreateInfo->displayMode);
            }
        }
    }
    VkResult result = instance_dispatch_table.CreateDisplayPlaneSurfaceKHR(
        instance, (const VkDisplaySurfaceCreateInfoKHR*)local_pCreateInfo, pAllocator, pSurface);
    if (VK_SUCCESS == result) {
        *pSurface = WrapNew(*pSurface);
    }
    return result;
}

VkResult DispatchObject::CreateSharedSwapchainsKHR(VkDevice device, uint32_t swapchainCount,
                                                   const VkSwapchainCreateInfoKHR* pCreateInfos,
                                                   const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchains) {
    if (!wrap_handles)
        return device_dispatch_table.CreateSharedSwapchainsKHR(device, swapchainCount, pCreateInfos, pAllocator, pSwapchains);
    small_vector<vku::safe_VkSwapchainCreateInfoKHR, DISPATCH_MAX_STACK_ALLOCATIONS> var_local_pCreateInfos;
    vku::safe_VkSwapchainCreateInfoKHR* local_pCreateInfos = nullptr;
    {
        if (pCreateInfos) {
            var_local_pCreateInfos.resize(swapchainCount);
            local_pCreateInfos = var_local_pCreateInfos.data();
            for (uint32_t index0 = 0; index0 < swapchainCount; ++index0) {
                local_pCreateInfos[index0].initialize(&pCreateInfos[index0]);

                if (pCreateInfos[index0].surface) {
                    local_pCreateInfos[index0].surface = Unwrap(pCreateInfos[index0].surface);
                }
                if (pCreateInfos[index0].oldSwapchain) {
                    local_pCreateInfos[index0].oldSwapchain = Unwrap(pCreateInfos[index0].oldSwapchain);
                }
            }
        }
    }
    VkResult result = device_dispatch_table.CreateSharedSwapchainsKHR(
        device, swapchainCount, (const VkSwapchainCreateInfoKHR*)local_pCreateInfos, pAllocator, pSwapchains);
    if (VK_SUCCESS == result) {
        for (uint32_t index0 = 0; index0 < swapchainCount; index0++) {
            pSwapchains[index0] = WrapNew(pSwapchains[index0]);
        }
    }
    return result;
}
#ifdef VK_USE_PLATFORM_XLIB_KHR

VkResult DispatchObject::CreateXlibSurfaceKHR(VkInstance instance, const VkXlibSurfaceCreateInfoKHR* pCreateInfo,
                                              const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    if (!wrap_handles) return instance_dispatch_table.CreateXlibSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);

    VkResult result = instance_dispatch_table.CreateXlibSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
    if (VK_SUCCESS == result) {
        *pSurface = WrapNew(*pSurface);
    }
    return result;
}

VkBool32 DispatchObject::GetPhysicalDeviceXlibPresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
                                                                     Display* dpy, VisualID visualID) {
    VkBool32 result =
        instance_dispatch_table.GetPhysicalDeviceXlibPresentationSupportKHR(physicalDevice, queueFamilyIndex, dpy, visualID);

    return result;
}
#endif  // VK_USE_PLATFORM_XLIB_KHR
#ifdef VK_USE_PLATFORM_XCB_KHR

VkResult DispatchObject::CreateXcbSurfaceKHR(VkInstance instance, const VkXcbSurfaceCreateInfoKHR* pCreateInfo,
                                             const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    if (!wrap_handles) return instance_dispatch_table.CreateXcbSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);

    VkResult result = instance_dispatch_table.CreateXcbSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
    if (VK_SUCCESS == result) {
        *pSurface = WrapNew(*pSurface);
    }
    return result;
}

VkBool32 DispatchObject::GetPhysicalDeviceXcbPresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
                                                                    xcb_connection_t* connection, xcb_visualid_t visual_id) {
    VkBool32 result =
        instance_dispatch_table.GetPhysicalDeviceXcbPresentationSupportKHR(physicalDevice, queueFamilyIndex, connection, visual_id);

    return result;
}
#endif  // VK_USE_PLATFORM_XCB_KHR
#ifdef VK_USE_PLATFORM_WAYLAND_KHR

VkResult DispatchObject::CreateWaylandSurfaceKHR(VkInstance instance, const VkWaylandSurfaceCreateInfoKHR* pCreateInfo,
                                                 const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    if (!wrap_handles) return instance_dispatch_table.CreateWaylandSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);

    VkResult result = instance_dispatch_table.CreateWaylandSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
    if (VK_SUCCESS == result) {
        *pSurface = WrapNew(*pSurface);
    }
    return result;
}

VkBool32 DispatchObject::GetPhysicalDeviceWaylandPresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
                                                                        struct wl_display* display) {
    VkBool32 result =
        instance_dispatch_table.GetPhysicalDeviceWaylandPresentationSupportKHR(physicalDevice, queueFamilyIndex, display);

    return result;
}
#endif  // VK_USE_PLATFORM_WAYLAND_KHR
#ifdef VK_USE_PLATFORM_ANDROID_KHR

VkResult DispatchObject::CreateAndroidSurfaceKHR(VkInstance instance, const VkAndroidSurfaceCreateInfoKHR* pCreateInfo,
                                                 const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    if (!wrap_handles) return instance_dispatch_table.CreateAndroidSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);

    VkResult result = instance_dispatch_table.CreateAndroidSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
    if (VK_SUCCESS == result) {
        *pSurface = WrapNew(*pSurface);
    }
    return result;
}
#endif  // VK_USE_PLATFORM_ANDROID_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR

VkResult DispatchObject::CreateWin32SurfaceKHR(VkInstance instance, const VkWin32SurfaceCreateInfoKHR* pCreateInfo,
                                               const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    if (!wrap_handles) return instance_dispatch_table.CreateWin32SurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);

    VkResult result = instance_dispatch_table.CreateWin32SurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
    if (VK_SUCCESS == result) {
        *pSurface = WrapNew(*pSurface);
    }
    return result;
}

VkBool32 DispatchObject::GetPhysicalDeviceWin32PresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex) {
    VkBool32 result = instance_dispatch_table.GetPhysicalDeviceWin32PresentationSupportKHR(physicalDevice, queueFamilyIndex);

    return result;
}
#endif  // VK_USE_PLATFORM_WIN32_KHR

VkResult DispatchObject::GetPhysicalDeviceVideoCapabilitiesKHR(VkPhysicalDevice physicalDevice,
                                                               const VkVideoProfileInfoKHR* pVideoProfile,
                                                               VkVideoCapabilitiesKHR* pCapabilities) {
    VkResult result = instance_dispatch_table.GetPhysicalDeviceVideoCapabilitiesKHR(physicalDevice, pVideoProfile, pCapabilities);

    return result;
}

VkResult DispatchObject::GetPhysicalDeviceVideoFormatPropertiesKHR(VkPhysicalDevice physicalDevice,
                                                                   const VkPhysicalDeviceVideoFormatInfoKHR* pVideoFormatInfo,
                                                                   uint32_t* pVideoFormatPropertyCount,
                                                                   VkVideoFormatPropertiesKHR* pVideoFormatProperties) {
    VkResult result = instance_dispatch_table.GetPhysicalDeviceVideoFormatPropertiesKHR(
        physicalDevice, pVideoFormatInfo, pVideoFormatPropertyCount, pVideoFormatProperties);

    return result;
}

VkResult DispatchObject::CreateVideoSessionKHR(VkDevice device, const VkVideoSessionCreateInfoKHR* pCreateInfo,
                                               const VkAllocationCallbacks* pAllocator, VkVideoSessionKHR* pVideoSession) {
    if (!wrap_handles) return device_dispatch_table.CreateVideoSessionKHR(device, pCreateInfo, pAllocator, pVideoSession);

    VkResult result = device_dispatch_table.CreateVideoSessionKHR(device, pCreateInfo, pAllocator, pVideoSession);
    if (VK_SUCCESS == result) {
        *pVideoSession = WrapNew(*pVideoSession);
    }
    return result;
}

void DispatchObject::DestroyVideoSessionKHR(VkDevice device, VkVideoSessionKHR videoSession,
                                            const VkAllocationCallbacks* pAllocator) {
    if (!wrap_handles) return device_dispatch_table.DestroyVideoSessionKHR(device, videoSession, pAllocator);

    uint64_t videoSession_id = CastToUint64(videoSession);
    auto iter = unique_id_mapping.pop(videoSession_id);
    if (iter != unique_id_mapping.end()) {
        videoSession = (VkVideoSessionKHR)iter->second;
    } else {
        videoSession = (VkVideoSessionKHR)0;
    }
    device_dispatch_table.DestroyVideoSessionKHR(device, videoSession, pAllocator);
}

VkResult DispatchObject::GetVideoSessionMemoryRequirementsKHR(VkDevice device, VkVideoSessionKHR videoSession,
                                                              uint32_t* pMemoryRequirementsCount,
                                                              VkVideoSessionMemoryRequirementsKHR* pMemoryRequirements) {
    if (!wrap_handles)
        return device_dispatch_table.GetVideoSessionMemoryRequirementsKHR(device, videoSession, pMemoryRequirementsCount,
                                                                          pMemoryRequirements);
    { videoSession = Unwrap(videoSession); }
    VkResult result = device_dispatch_table.GetVideoSessionMemoryRequirementsKHR(device, videoSession, pMemoryRequirementsCount,
                                                                                 pMemoryRequirements);

    return result;
}

VkResult DispatchObject::BindVideoSessionMemoryKHR(VkDevice device, VkVideoSessionKHR videoSession,
                                                   uint32_t bindSessionMemoryInfoCount,
                                                   const VkBindVideoSessionMemoryInfoKHR* pBindSessionMemoryInfos) {
    if (!wrap_handles)
        return device_dispatch_table.BindVideoSessionMemoryKHR(device, videoSession, bindSessionMemoryInfoCount,
                                                               pBindSessionMemoryInfos);
    small_vector<vku::safe_VkBindVideoSessionMemoryInfoKHR, DISPATCH_MAX_STACK_ALLOCATIONS> var_local_pBindSessionMemoryInfos;
    vku::safe_VkBindVideoSessionMemoryInfoKHR* local_pBindSessionMemoryInfos = nullptr;
    {
        videoSession = Unwrap(videoSession);
        if (pBindSessionMemoryInfos) {
            var_local_pBindSessionMemoryInfos.resize(bindSessionMemoryInfoCount);
            local_pBindSessionMemoryInfos = var_local_pBindSessionMemoryInfos.data();
            for (uint32_t index0 = 0; index0 < bindSessionMemoryInfoCount; ++index0) {
                local_pBindSessionMemoryInfos[index0].initialize(&pBindSessionMemoryInfos[index0]);

                if (pBindSessionMemoryInfos[index0].memory) {
                    local_pBindSessionMemoryInfos[index0].memory = Unwrap(pBindSessionMemoryInfos[index0].memory);
                }
            }
        }
    }
    VkResult result = device_dispatch_table.BindVideoSessionMemoryKHR(
        device, videoSession, bindSessionMemoryInfoCount, (const VkBindVideoSessionMemoryInfoKHR*)local_pBindSessionMemoryInfos);

    return result;
}

VkResult DispatchObject::CreateVideoSessionParametersKHR(VkDevice device, const VkVideoSessionParametersCreateInfoKHR* pCreateInfo,
                                                         const VkAllocationCallbacks* pAllocator,
                                                         VkVideoSessionParametersKHR* pVideoSessionParameters) {
    if (!wrap_handles)
        return device_dispatch_table.CreateVideoSessionParametersKHR(device, pCreateInfo, pAllocator, pVideoSessionParameters);
    vku::safe_VkVideoSessionParametersCreateInfoKHR var_local_pCreateInfo;
    vku::safe_VkVideoSessionParametersCreateInfoKHR* local_pCreateInfo = nullptr;
    {
        if (pCreateInfo) {
            local_pCreateInfo = &var_local_pCreateInfo;
            local_pCreateInfo->initialize(pCreateInfo);

            if (pCreateInfo->videoSessionParametersTemplate) {
                local_pCreateInfo->videoSessionParametersTemplate = Unwrap(pCreateInfo->videoSessionParametersTemplate);
            }
            if (pCreateInfo->videoSession) {
                local_pCreateInfo->videoSession = Unwrap(pCreateInfo->videoSession);
            }
        }
    }
    VkResult result = device_dispatch_table.CreateVideoSessionParametersKHR(
        device, (const VkVideoSessionParametersCreateInfoKHR*)local_pCreateInfo, pAllocator, pVideoSessionParameters);
    if (VK_SUCCESS == result) {
        *pVideoSessionParameters = WrapNew(*pVideoSessionParameters);
    }
    return result;
}

VkResult DispatchObject::UpdateVideoSessionParametersKHR(VkDevice device, VkVideoSessionParametersKHR videoSessionParameters,
                                                         const VkVideoSessionParametersUpdateInfoKHR* pUpdateInfo) {
    if (!wrap_handles) return device_dispatch_table.UpdateVideoSessionParametersKHR(device, videoSessionParameters, pUpdateInfo);
    { videoSessionParameters = Unwrap(videoSessionParameters); }
    VkResult result = device_dispatch_table.UpdateVideoSessionParametersKHR(device, videoSessionParameters, pUpdateInfo);

    return result;
}

void DispatchObject::DestroyVideoSessionParametersKHR(VkDevice device, VkVideoSessionParametersKHR videoSessionParameters,
                                                      const VkAllocationCallbacks* pAllocator) {
    if (!wrap_handles) return device_dispatch_table.DestroyVideoSessionParametersKHR(device, videoSessionParameters, pAllocator);

    uint64_t videoSessionParameters_id = CastToUint64(videoSessionParameters);
    auto iter = unique_id_mapping.pop(videoSessionParameters_id);
    if (iter != unique_id_mapping.end()) {
        videoSessionParameters = (VkVideoSessionParametersKHR)iter->second;
    } else {
        videoSessionParameters = (VkVideoSessionParametersKHR)0;
    }
    device_dispatch_table.DestroyVideoSessionParametersKHR(device, videoSessionParameters, pAllocator);
}

void DispatchObject::CmdBeginVideoCodingKHR(VkCommandBuffer commandBuffer, const VkVideoBeginCodingInfoKHR* pBeginInfo) {
    if (!wrap_handles) return device_dispatch_table.CmdBeginVideoCodingKHR(commandBuffer, pBeginInfo);
    vku::safe_VkVideoBeginCodingInfoKHR var_local_pBeginInfo;
    vku::safe_VkVideoBeginCodingInfoKHR* local_pBeginInfo = nullptr;
    {
        if (pBeginInfo) {
            local_pBeginInfo = &var_local_pBeginInfo;
            local_pBeginInfo->initialize(pBeginInfo);

            if (pBeginInfo->videoSession) {
                local_pBeginInfo->videoSession = Unwrap(pBeginInfo->videoSession);
            }
            if (pBeginInfo->videoSessionParameters) {
                local_pBeginInfo->videoSessionParameters = Unwrap(pBeginInfo->videoSessionParameters);
            }
            if (local_pBeginInfo->pReferenceSlots) {
                for (uint32_t index1 = 0; index1 < local_pBeginInfo->referenceSlotCount; ++index1) {
                    if (local_pBeginInfo->pReferenceSlots[index1].pPictureResource) {
                        if (pBeginInfo->pReferenceSlots[index1].pPictureResource->imageViewBinding) {
                            local_pBeginInfo->pReferenceSlots[index1].pPictureResource->imageViewBinding =
                                Unwrap(pBeginInfo->pReferenceSlots[index1].pPictureResource->imageViewBinding);
                        }
                    }
                }
            }
        }
    }
    device_dispatch_table.CmdBeginVideoCodingKHR(commandBuffer, (const VkVideoBeginCodingInfoKHR*)local_pBeginInfo);
}

void DispatchObject::CmdEndVideoCodingKHR(VkCommandBuffer commandBuffer, const VkVideoEndCodingInfoKHR* pEndCodingInfo) {
    device_dispatch_table.CmdEndVideoCodingKHR(commandBuffer, pEndCodingInfo);
}

void DispatchObject::CmdControlVideoCodingKHR(VkCommandBuffer commandBuffer,
                                              const VkVideoCodingControlInfoKHR* pCodingControlInfo) {
    device_dispatch_table.CmdControlVideoCodingKHR(commandBuffer, pCodingControlInfo);
}

void DispatchObject::CmdDecodeVideoKHR(VkCommandBuffer commandBuffer, const VkVideoDecodeInfoKHR* pDecodeInfo) {
    if (!wrap_handles) return device_dispatch_table.CmdDecodeVideoKHR(commandBuffer, pDecodeInfo);
    vku::safe_VkVideoDecodeInfoKHR var_local_pDecodeInfo;
    vku::safe_VkVideoDecodeInfoKHR* local_pDecodeInfo = nullptr;
    {
        if (pDecodeInfo) {
            local_pDecodeInfo = &var_local_pDecodeInfo;
            local_pDecodeInfo->initialize(pDecodeInfo);

            if (pDecodeInfo->srcBuffer) {
                local_pDecodeInfo->srcBuffer = Unwrap(pDecodeInfo->srcBuffer);
            }
            if (pDecodeInfo->dstPictureResource.imageViewBinding) {
                local_pDecodeInfo->dstPictureResource.imageViewBinding = Unwrap(pDecodeInfo->dstPictureResource.imageViewBinding);
            }
            if (local_pDecodeInfo->pSetupReferenceSlot) {
                if (local_pDecodeInfo->pSetupReferenceSlot->pPictureResource) {
                    if (pDecodeInfo->pSetupReferenceSlot->pPictureResource->imageViewBinding) {
                        local_pDecodeInfo->pSetupReferenceSlot->pPictureResource->imageViewBinding =
                            Unwrap(pDecodeInfo->pSetupReferenceSlot->pPictureResource->imageViewBinding);
                    }
                }
            }
            if (local_pDecodeInfo->pReferenceSlots) {
                for (uint32_t index1 = 0; index1 < local_pDecodeInfo->referenceSlotCount; ++index1) {
                    if (local_pDecodeInfo->pReferenceSlots[index1].pPictureResource) {
                        if (pDecodeInfo->pReferenceSlots[index1].pPictureResource->imageViewBinding) {
                            local_pDecodeInfo->pReferenceSlots[index1].pPictureResource->imageViewBinding =
                                Unwrap(pDecodeInfo->pReferenceSlots[index1].pPictureResource->imageViewBinding);
                        }
                    }
                }
            }
            UnwrapPnextChainHandles(local_pDecodeInfo->pNext);
        }
    }
    device_dispatch_table.CmdDecodeVideoKHR(commandBuffer, (const VkVideoDecodeInfoKHR*)local_pDecodeInfo);
}

void DispatchObject::CmdBeginRenderingKHR(VkCommandBuffer commandBuffer, const VkRenderingInfo* pRenderingInfo) {
    if (!wrap_handles) return device_dispatch_table.CmdBeginRenderingKHR(commandBuffer, pRenderingInfo);
    vku::safe_VkRenderingInfo var_local_pRenderingInfo;
    vku::safe_VkRenderingInfo* local_pRenderingInfo = nullptr;
    {
        if (pRenderingInfo) {
            local_pRenderingInfo = &var_local_pRenderingInfo;
            local_pRenderingInfo->initialize(pRenderingInfo);
            if (local_pRenderingInfo->pColorAttachments) {
                for (uint32_t index1 = 0; index1 < local_pRenderingInfo->colorAttachmentCount; ++index1) {
                    if (pRenderingInfo->pColorAttachments[index1].imageView) {
                        local_pRenderingInfo->pColorAttachments[index1].imageView =
                            Unwrap(pRenderingInfo->pColorAttachments[index1].imageView);
                    }
                    if (pRenderingInfo->pColorAttachments[index1].resolveImageView) {
                        local_pRenderingInfo->pColorAttachments[index1].resolveImageView =
                            Unwrap(pRenderingInfo->pColorAttachments[index1].resolveImageView);
                    }
                }
            }
            if (local_pRenderingInfo->pDepthAttachment) {
                if (pRenderingInfo->pDepthAttachment->imageView) {
                    local_pRenderingInfo->pDepthAttachment->imageView = Unwrap(pRenderingInfo->pDepthAttachment->imageView);
                }
                if (pRenderingInfo->pDepthAttachment->resolveImageView) {
                    local_pRenderingInfo->pDepthAttachment->resolveImageView =
                        Unwrap(pRenderingInfo->pDepthAttachment->resolveImageView);
                }
            }
            if (local_pRenderingInfo->pStencilAttachment) {
                if (pRenderingInfo->pStencilAttachment->imageView) {
                    local_pRenderingInfo->pStencilAttachment->imageView = Unwrap(pRenderingInfo->pStencilAttachment->imageView);
                }
                if (pRenderingInfo->pStencilAttachment->resolveImageView) {
                    local_pRenderingInfo->pStencilAttachment->resolveImageView =
                        Unwrap(pRenderingInfo->pStencilAttachment->resolveImageView);
                }
            }
            UnwrapPnextChainHandles(local_pRenderingInfo->pNext);
        }
    }
    device_dispatch_table.CmdBeginRenderingKHR(commandBuffer, (const VkRenderingInfo*)local_pRenderingInfo);
}

void DispatchObject::CmdEndRenderingKHR(VkCommandBuffer commandBuffer) { device_dispatch_table.CmdEndRenderingKHR(commandBuffer); }

void DispatchObject::GetPhysicalDeviceFeatures2KHR(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures2* pFeatures) {
    instance_dispatch_table.GetPhysicalDeviceFeatures2KHR(physicalDevice, pFeatures);
}

void DispatchObject::GetPhysicalDeviceProperties2KHR(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties2* pProperties) {
    instance_dispatch_table.GetPhysicalDeviceProperties2KHR(physicalDevice, pProperties);
}

void DispatchObject::GetPhysicalDeviceFormatProperties2KHR(VkPhysicalDevice physicalDevice, VkFormat format,
                                                           VkFormatProperties2* pFormatProperties) {
    instance_dispatch_table.GetPhysicalDeviceFormatProperties2KHR(physicalDevice, format, pFormatProperties);
}

VkResult DispatchObject::GetPhysicalDeviceImageFormatProperties2KHR(VkPhysicalDevice physicalDevice,
                                                                    const VkPhysicalDeviceImageFormatInfo2* pImageFormatInfo,
                                                                    VkImageFormatProperties2* pImageFormatProperties) {
    VkResult result = instance_dispatch_table.GetPhysicalDeviceImageFormatProperties2KHR(physicalDevice, pImageFormatInfo,
                                                                                         pImageFormatProperties);

    return result;
}

void DispatchObject::GetPhysicalDeviceQueueFamilyProperties2KHR(VkPhysicalDevice physicalDevice,
                                                                uint32_t* pQueueFamilyPropertyCount,
                                                                VkQueueFamilyProperties2* pQueueFamilyProperties) {
    instance_dispatch_table.GetPhysicalDeviceQueueFamilyProperties2KHR(physicalDevice, pQueueFamilyPropertyCount,
                                                                       pQueueFamilyProperties);
}

void DispatchObject::GetPhysicalDeviceMemoryProperties2KHR(VkPhysicalDevice physicalDevice,
                                                           VkPhysicalDeviceMemoryProperties2* pMemoryProperties) {
    instance_dispatch_table.GetPhysicalDeviceMemoryProperties2KHR(physicalDevice, pMemoryProperties);
}

void DispatchObject::GetPhysicalDeviceSparseImageFormatProperties2KHR(VkPhysicalDevice physicalDevice,
                                                                      const VkPhysicalDeviceSparseImageFormatInfo2* pFormatInfo,
                                                                      uint32_t* pPropertyCount,
                                                                      VkSparseImageFormatProperties2* pProperties) {
    instance_dispatch_table.GetPhysicalDeviceSparseImageFormatProperties2KHR(physicalDevice, pFormatInfo, pPropertyCount,
                                                                             pProperties);
}

void DispatchObject::GetDeviceGroupPeerMemoryFeaturesKHR(VkDevice device, uint32_t heapIndex, uint32_t localDeviceIndex,
                                                         uint32_t remoteDeviceIndex,
                                                         VkPeerMemoryFeatureFlags* pPeerMemoryFeatures) {
    device_dispatch_table.GetDeviceGroupPeerMemoryFeaturesKHR(device, heapIndex, localDeviceIndex, remoteDeviceIndex,
                                                              pPeerMemoryFeatures);
}

void DispatchObject::CmdSetDeviceMaskKHR(VkCommandBuffer commandBuffer, uint32_t deviceMask) {
    device_dispatch_table.CmdSetDeviceMaskKHR(commandBuffer, deviceMask);
}

void DispatchObject::CmdDispatchBaseKHR(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY,
                                        uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) {
    device_dispatch_table.CmdDispatchBaseKHR(commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY,
                                             groupCountZ);
}

void DispatchObject::TrimCommandPoolKHR(VkDevice device, VkCommandPool commandPool, VkCommandPoolTrimFlags flags) {
    if (!wrap_handles) return device_dispatch_table.TrimCommandPoolKHR(device, commandPool, flags);
    { commandPool = Unwrap(commandPool); }
    device_dispatch_table.TrimCommandPoolKHR(device, commandPool, flags);
}

VkResult DispatchObject::EnumeratePhysicalDeviceGroupsKHR(VkInstance instance, uint32_t* pPhysicalDeviceGroupCount,
                                                          VkPhysicalDeviceGroupProperties* pPhysicalDeviceGroupProperties) {
    VkResult result = instance_dispatch_table.EnumeratePhysicalDeviceGroupsKHR(instance, pPhysicalDeviceGroupCount,
                                                                               pPhysicalDeviceGroupProperties);

    return result;
}

void DispatchObject::GetPhysicalDeviceExternalBufferPropertiesKHR(VkPhysicalDevice physicalDevice,
                                                                  const VkPhysicalDeviceExternalBufferInfo* pExternalBufferInfo,
                                                                  VkExternalBufferProperties* pExternalBufferProperties) {
    instance_dispatch_table.GetPhysicalDeviceExternalBufferPropertiesKHR(physicalDevice, pExternalBufferInfo,
                                                                         pExternalBufferProperties);
}
#ifdef VK_USE_PLATFORM_WIN32_KHR

VkResult DispatchObject::GetMemoryWin32HandleKHR(VkDevice device, const VkMemoryGetWin32HandleInfoKHR* pGetWin32HandleInfo,
                                                 HANDLE* pHandle) {
    if (!wrap_handles) return device_dispatch_table.GetMemoryWin32HandleKHR(device, pGetWin32HandleInfo, pHandle);
    vku::safe_VkMemoryGetWin32HandleInfoKHR var_local_pGetWin32HandleInfo;
    vku::safe_VkMemoryGetWin32HandleInfoKHR* local_pGetWin32HandleInfo = nullptr;
    {
        if (pGetWin32HandleInfo) {
            local_pGetWin32HandleInfo = &var_local_pGetWin32HandleInfo;
            local_pGetWin32HandleInfo->initialize(pGetWin32HandleInfo);

            if (pGetWin32HandleInfo->memory) {
                local_pGetWin32HandleInfo->memory = Unwrap(pGetWin32HandleInfo->memory);
            }
        }
    }
    VkResult result = device_dispatch_table.GetMemoryWin32HandleKHR(
        device, (const VkMemoryGetWin32HandleInfoKHR*)local_pGetWin32HandleInfo, pHandle);

    return result;
}

VkResult DispatchObject::GetMemoryWin32HandlePropertiesKHR(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType,
                                                           HANDLE handle,
                                                           VkMemoryWin32HandlePropertiesKHR* pMemoryWin32HandleProperties) {
    VkResult result =
        device_dispatch_table.GetMemoryWin32HandlePropertiesKHR(device, handleType, handle, pMemoryWin32HandleProperties);

    return result;
}
#endif  // VK_USE_PLATFORM_WIN32_KHR

VkResult DispatchObject::GetMemoryFdKHR(VkDevice device, const VkMemoryGetFdInfoKHR* pGetFdInfo, int* pFd) {
    if (!wrap_handles) return device_dispatch_table.GetMemoryFdKHR(device, pGetFdInfo, pFd);
    vku::safe_VkMemoryGetFdInfoKHR var_local_pGetFdInfo;
    vku::safe_VkMemoryGetFdInfoKHR* local_pGetFdInfo = nullptr;
    {
        if (pGetFdInfo) {
            local_pGetFdInfo = &var_local_pGetFdInfo;
            local_pGetFdInfo->initialize(pGetFdInfo);

            if (pGetFdInfo->memory) {
                local_pGetFdInfo->memory = Unwrap(pGetFdInfo->memory);
            }
        }
    }
    VkResult result = device_dispatch_table.GetMemoryFdKHR(device, (const VkMemoryGetFdInfoKHR*)local_pGetFdInfo, pFd);

    return result;
}

VkResult DispatchObject::GetMemoryFdPropertiesKHR(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType, int fd,
                                                  VkMemoryFdPropertiesKHR* pMemoryFdProperties) {
    VkResult result = device_dispatch_table.GetMemoryFdPropertiesKHR(device, handleType, fd, pMemoryFdProperties);

    return result;
}

void DispatchObject::GetPhysicalDeviceExternalSemaphorePropertiesKHR(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalSemaphoreInfo* pExternalSemaphoreInfo,
    VkExternalSemaphoreProperties* pExternalSemaphoreProperties) {
    instance_dispatch_table.GetPhysicalDeviceExternalSemaphorePropertiesKHR(physicalDevice, pExternalSemaphoreInfo,
                                                                            pExternalSemaphoreProperties);
}
#ifdef VK_USE_PLATFORM_WIN32_KHR

VkResult DispatchObject::ImportSemaphoreWin32HandleKHR(VkDevice device,
                                                       const VkImportSemaphoreWin32HandleInfoKHR* pImportSemaphoreWin32HandleInfo) {
    if (!wrap_handles) return device_dispatch_table.ImportSemaphoreWin32HandleKHR(device, pImportSemaphoreWin32HandleInfo);
    vku::safe_VkImportSemaphoreWin32HandleInfoKHR var_local_pImportSemaphoreWin32HandleInfo;
    vku::safe_VkImportSemaphoreWin32HandleInfoKHR* local_pImportSemaphoreWin32HandleInfo = nullptr;
    {
        if (pImportSemaphoreWin32HandleInfo) {
            local_pImportSemaphoreWin32HandleInfo = &var_local_pImportSemaphoreWin32HandleInfo;
            local_pImportSemaphoreWin32HandleInfo->initialize(pImportSemaphoreWin32HandleInfo);

            if (pImportSemaphoreWin32HandleInfo->semaphore) {
                local_pImportSemaphoreWin32HandleInfo->semaphore = Unwrap(pImportSemaphoreWin32HandleInfo->semaphore);
            }
        }
    }
    VkResult result = device_dispatch_table.ImportSemaphoreWin32HandleKHR(
        device, (const VkImportSemaphoreWin32HandleInfoKHR*)local_pImportSemaphoreWin32HandleInfo);

    return result;
}

VkResult DispatchObject::GetSemaphoreWin32HandleKHR(VkDevice device, const VkSemaphoreGetWin32HandleInfoKHR* pGetWin32HandleInfo,
                                                    HANDLE* pHandle) {
    if (!wrap_handles) return device_dispatch_table.GetSemaphoreWin32HandleKHR(device, pGetWin32HandleInfo, pHandle);
    vku::safe_VkSemaphoreGetWin32HandleInfoKHR var_local_pGetWin32HandleInfo;
    vku::safe_VkSemaphoreGetWin32HandleInfoKHR* local_pGetWin32HandleInfo = nullptr;
    {
        if (pGetWin32HandleInfo) {
            local_pGetWin32HandleInfo = &var_local_pGetWin32HandleInfo;
            local_pGetWin32HandleInfo->initialize(pGetWin32HandleInfo);

            if (pGetWin32HandleInfo->semaphore) {
                local_pGetWin32HandleInfo->semaphore = Unwrap(pGetWin32HandleInfo->semaphore);
            }
        }
    }
    VkResult result = device_dispatch_table.GetSemaphoreWin32HandleKHR(
        device, (const VkSemaphoreGetWin32HandleInfoKHR*)local_pGetWin32HandleInfo, pHandle);

    return result;
}
#endif  // VK_USE_PLATFORM_WIN32_KHR

VkResult DispatchObject::ImportSemaphoreFdKHR(VkDevice device, const VkImportSemaphoreFdInfoKHR* pImportSemaphoreFdInfo) {
    if (!wrap_handles) return device_dispatch_table.ImportSemaphoreFdKHR(device, pImportSemaphoreFdInfo);
    vku::safe_VkImportSemaphoreFdInfoKHR var_local_pImportSemaphoreFdInfo;
    vku::safe_VkImportSemaphoreFdInfoKHR* local_pImportSemaphoreFdInfo = nullptr;
    {
        if (pImportSemaphoreFdInfo) {
            local_pImportSemaphoreFdInfo = &var_local_pImportSemaphoreFdInfo;
            local_pImportSemaphoreFdInfo->initialize(pImportSemaphoreFdInfo);

            if (pImportSemaphoreFdInfo->semaphore) {
                local_pImportSemaphoreFdInfo->semaphore = Unwrap(pImportSemaphoreFdInfo->semaphore);
            }
        }
    }
    VkResult result =
        device_dispatch_table.ImportSemaphoreFdKHR(device, (const VkImportSemaphoreFdInfoKHR*)local_pImportSemaphoreFdInfo);

    return result;
}

VkResult DispatchObject::GetSemaphoreFdKHR(VkDevice device, const VkSemaphoreGetFdInfoKHR* pGetFdInfo, int* pFd) {
    if (!wrap_handles) return device_dispatch_table.GetSemaphoreFdKHR(device, pGetFdInfo, pFd);
    vku::safe_VkSemaphoreGetFdInfoKHR var_local_pGetFdInfo;
    vku::safe_VkSemaphoreGetFdInfoKHR* local_pGetFdInfo = nullptr;
    {
        if (pGetFdInfo) {
            local_pGetFdInfo = &var_local_pGetFdInfo;
            local_pGetFdInfo->initialize(pGetFdInfo);

            if (pGetFdInfo->semaphore) {
                local_pGetFdInfo->semaphore = Unwrap(pGetFdInfo->semaphore);
            }
        }
    }
    VkResult result = device_dispatch_table.GetSemaphoreFdKHR(device, (const VkSemaphoreGetFdInfoKHR*)local_pGetFdInfo, pFd);

    return result;
}

void DispatchObject::CmdPushDescriptorSetKHR(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                             VkPipelineLayout layout, uint32_t set, uint32_t descriptorWriteCount,
                                             const VkWriteDescriptorSet* pDescriptorWrites) {
    if (!wrap_handles)
        return device_dispatch_table.CmdPushDescriptorSetKHR(commandBuffer, pipelineBindPoint, layout, set, descriptorWriteCount,
                                                             pDescriptorWrites);
    small_vector<vku::safe_VkWriteDescriptorSet, DISPATCH_MAX_STACK_ALLOCATIONS> var_local_pDescriptorWrites;
    vku::safe_VkWriteDescriptorSet* local_pDescriptorWrites = nullptr;
    {
        layout = Unwrap(layout);
        if (pDescriptorWrites) {
            var_local_pDescriptorWrites.resize(descriptorWriteCount);
            local_pDescriptorWrites = var_local_pDescriptorWrites.data();
            for (uint32_t index0 = 0; index0 < descriptorWriteCount; ++index0) {
                local_pDescriptorWrites[index0].initialize(&pDescriptorWrites[index0]);
                UnwrapPnextChainHandles(local_pDescriptorWrites[index0].pNext);

                if (pDescriptorWrites[index0].dstSet) {
                    local_pDescriptorWrites[index0].dstSet = Unwrap(pDescriptorWrites[index0].dstSet);
                }
                if (local_pDescriptorWrites[index0].pImageInfo) {
                    for (uint32_t index1 = 0; index1 < local_pDescriptorWrites[index0].descriptorCount; ++index1) {
                        if (pDescriptorWrites[index0].pImageInfo[index1].sampler) {
                            local_pDescriptorWrites[index0].pImageInfo[index1].sampler =
                                Unwrap(pDescriptorWrites[index0].pImageInfo[index1].sampler);
                        }
                        if (pDescriptorWrites[index0].pImageInfo[index1].imageView) {
                            local_pDescriptorWrites[index0].pImageInfo[index1].imageView =
                                Unwrap(pDescriptorWrites[index0].pImageInfo[index1].imageView);
                        }
                    }
                }
                if (local_pDescriptorWrites[index0].pBufferInfo) {
                    for (uint32_t index1 = 0; index1 < local_pDescriptorWrites[index0].descriptorCount; ++index1) {
                        if (pDescriptorWrites[index0].pBufferInfo[index1].buffer) {
                            local_pDescriptorWrites[index0].pBufferInfo[index1].buffer =
                                Unwrap(pDescriptorWrites[index0].pBufferInfo[index1].buffer);
                        }
                    }
                }
                if (local_pDescriptorWrites[index0].pTexelBufferView) {
                    for (uint32_t index1 = 0; index1 < local_pDescriptorWrites[index0].descriptorCount; ++index1) {
                        local_pDescriptorWrites[index0].pTexelBufferView[index1] =
                            Unwrap(local_pDescriptorWrites[index0].pTexelBufferView[index1]);
                    }
                }
            }
        }
    }
    device_dispatch_table.CmdPushDescriptorSetKHR(commandBuffer, pipelineBindPoint, layout, set, descriptorWriteCount,
                                                  (const VkWriteDescriptorSet*)local_pDescriptorWrites);
}

void DispatchObject::CmdBeginRenderPass2KHR(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                            const VkSubpassBeginInfo* pSubpassBeginInfo) {
    if (!wrap_handles) return device_dispatch_table.CmdBeginRenderPass2KHR(commandBuffer, pRenderPassBegin, pSubpassBeginInfo);
    vku::safe_VkRenderPassBeginInfo var_local_pRenderPassBegin;
    vku::safe_VkRenderPassBeginInfo* local_pRenderPassBegin = nullptr;
    {
        if (pRenderPassBegin) {
            local_pRenderPassBegin = &var_local_pRenderPassBegin;
            local_pRenderPassBegin->initialize(pRenderPassBegin);

            if (pRenderPassBegin->renderPass) {
                local_pRenderPassBegin->renderPass = Unwrap(pRenderPassBegin->renderPass);
            }
            if (pRenderPassBegin->framebuffer) {
                local_pRenderPassBegin->framebuffer = Unwrap(pRenderPassBegin->framebuffer);
            }
            UnwrapPnextChainHandles(local_pRenderPassBegin->pNext);
        }
    }
    device_dispatch_table.CmdBeginRenderPass2KHR(commandBuffer, (const VkRenderPassBeginInfo*)local_pRenderPassBegin,
                                                 pSubpassBeginInfo);
}

void DispatchObject::CmdNextSubpass2KHR(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo* pSubpassBeginInfo,
                                        const VkSubpassEndInfo* pSubpassEndInfo) {
    device_dispatch_table.CmdNextSubpass2KHR(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo);
}

void DispatchObject::CmdEndRenderPass2KHR(VkCommandBuffer commandBuffer, const VkSubpassEndInfo* pSubpassEndInfo) {
    device_dispatch_table.CmdEndRenderPass2KHR(commandBuffer, pSubpassEndInfo);
}

VkResult DispatchObject::GetSwapchainStatusKHR(VkDevice device, VkSwapchainKHR swapchain) {
    if (!wrap_handles) return device_dispatch_table.GetSwapchainStatusKHR(device, swapchain);
    { swapchain = Unwrap(swapchain); }
    VkResult result = device_dispatch_table.GetSwapchainStatusKHR(device, swapchain);

    return result;
}

void DispatchObject::GetPhysicalDeviceExternalFencePropertiesKHR(VkPhysicalDevice physicalDevice,
                                                                 const VkPhysicalDeviceExternalFenceInfo* pExternalFenceInfo,
                                                                 VkExternalFenceProperties* pExternalFenceProperties) {
    instance_dispatch_table.GetPhysicalDeviceExternalFencePropertiesKHR(physicalDevice, pExternalFenceInfo,
                                                                        pExternalFenceProperties);
}
#ifdef VK_USE_PLATFORM_WIN32_KHR

VkResult DispatchObject::ImportFenceWin32HandleKHR(VkDevice device,
                                                   const VkImportFenceWin32HandleInfoKHR* pImportFenceWin32HandleInfo) {
    if (!wrap_handles) return device_dispatch_table.ImportFenceWin32HandleKHR(device, pImportFenceWin32HandleInfo);
    vku::safe_VkImportFenceWin32HandleInfoKHR var_local_pImportFenceWin32HandleInfo;
    vku::safe_VkImportFenceWin32HandleInfoKHR* local_pImportFenceWin32HandleInfo = nullptr;
    {
        if (pImportFenceWin32HandleInfo) {
            local_pImportFenceWin32HandleInfo = &var_local_pImportFenceWin32HandleInfo;
            local_pImportFenceWin32HandleInfo->initialize(pImportFenceWin32HandleInfo);

            if (pImportFenceWin32HandleInfo->fence) {
                local_pImportFenceWin32HandleInfo->fence = Unwrap(pImportFenceWin32HandleInfo->fence);
            }
        }
    }
    VkResult result = device_dispatch_table.ImportFenceWin32HandleKHR(
        device, (const VkImportFenceWin32HandleInfoKHR*)local_pImportFenceWin32HandleInfo);

    return result;
}

VkResult DispatchObject::GetFenceWin32HandleKHR(VkDevice device, const VkFenceGetWin32HandleInfoKHR* pGetWin32HandleInfo,
                                                HANDLE* pHandle) {
    if (!wrap_handles) return device_dispatch_table.GetFenceWin32HandleKHR(device, pGetWin32HandleInfo, pHandle);
    vku::safe_VkFenceGetWin32HandleInfoKHR var_local_pGetWin32HandleInfo;
    vku::safe_VkFenceGetWin32HandleInfoKHR* local_pGetWin32HandleInfo = nullptr;
    {
        if (pGetWin32HandleInfo) {
            local_pGetWin32HandleInfo = &var_local_pGetWin32HandleInfo;
            local_pGetWin32HandleInfo->initialize(pGetWin32HandleInfo);

            if (pGetWin32HandleInfo->fence) {
                local_pGetWin32HandleInfo->fence = Unwrap(pGetWin32HandleInfo->fence);
            }
        }
    }
    VkResult result = device_dispatch_table.GetFenceWin32HandleKHR(
        device, (const VkFenceGetWin32HandleInfoKHR*)local_pGetWin32HandleInfo, pHandle);

    return result;
}
#endif  // VK_USE_PLATFORM_WIN32_KHR

VkResult DispatchObject::ImportFenceFdKHR(VkDevice device, const VkImportFenceFdInfoKHR* pImportFenceFdInfo) {
    if (!wrap_handles) return device_dispatch_table.ImportFenceFdKHR(device, pImportFenceFdInfo);
    vku::safe_VkImportFenceFdInfoKHR var_local_pImportFenceFdInfo;
    vku::safe_VkImportFenceFdInfoKHR* local_pImportFenceFdInfo = nullptr;
    {
        if (pImportFenceFdInfo) {
            local_pImportFenceFdInfo = &var_local_pImportFenceFdInfo;
            local_pImportFenceFdInfo->initialize(pImportFenceFdInfo);

            if (pImportFenceFdInfo->fence) {
                local_pImportFenceFdInfo->fence = Unwrap(pImportFenceFdInfo->fence);
            }
        }
    }
    VkResult result = device_dispatch_table.ImportFenceFdKHR(device, (const VkImportFenceFdInfoKHR*)local_pImportFenceFdInfo);

    return result;
}

VkResult DispatchObject::GetFenceFdKHR(VkDevice device, const VkFenceGetFdInfoKHR* pGetFdInfo, int* pFd) {
    if (!wrap_handles) return device_dispatch_table.GetFenceFdKHR(device, pGetFdInfo, pFd);
    vku::safe_VkFenceGetFdInfoKHR var_local_pGetFdInfo;
    vku::safe_VkFenceGetFdInfoKHR* local_pGetFdInfo = nullptr;
    {
        if (pGetFdInfo) {
            local_pGetFdInfo = &var_local_pGetFdInfo;
            local_pGetFdInfo->initialize(pGetFdInfo);

            if (pGetFdInfo->fence) {
                local_pGetFdInfo->fence = Unwrap(pGetFdInfo->fence);
            }
        }
    }
    VkResult result = device_dispatch_table.GetFenceFdKHR(device, (const VkFenceGetFdInfoKHR*)local_pGetFdInfo, pFd);

    return result;
}

VkResult DispatchObject::EnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(
    VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, uint32_t* pCounterCount, VkPerformanceCounterKHR* pCounters,
    VkPerformanceCounterDescriptionKHR* pCounterDescriptions) {
    VkResult result = instance_dispatch_table.EnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(
        physicalDevice, queueFamilyIndex, pCounterCount, pCounters, pCounterDescriptions);

    return result;
}

void DispatchObject::GetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR(
    VkPhysicalDevice physicalDevice, const VkQueryPoolPerformanceCreateInfoKHR* pPerformanceQueryCreateInfo, uint32_t* pNumPasses) {
    instance_dispatch_table.GetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR(physicalDevice, pPerformanceQueryCreateInfo,
                                                                                  pNumPasses);
}

VkResult DispatchObject::AcquireProfilingLockKHR(VkDevice device, const VkAcquireProfilingLockInfoKHR* pInfo) {
    VkResult result = device_dispatch_table.AcquireProfilingLockKHR(device, pInfo);

    return result;
}

void DispatchObject::ReleaseProfilingLockKHR(VkDevice device) { device_dispatch_table.ReleaseProfilingLockKHR(device); }

VkResult DispatchObject::GetPhysicalDeviceSurfaceCapabilities2KHR(VkPhysicalDevice physicalDevice,
                                                                  const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo,
                                                                  VkSurfaceCapabilities2KHR* pSurfaceCapabilities) {
    if (!wrap_handles)
        return instance_dispatch_table.GetPhysicalDeviceSurfaceCapabilities2KHR(physicalDevice, pSurfaceInfo, pSurfaceCapabilities);
    vku::safe_VkPhysicalDeviceSurfaceInfo2KHR var_local_pSurfaceInfo;
    vku::safe_VkPhysicalDeviceSurfaceInfo2KHR* local_pSurfaceInfo = nullptr;
    {
        if (pSurfaceInfo) {
            local_pSurfaceInfo = &var_local_pSurfaceInfo;
            local_pSurfaceInfo->initialize(pSurfaceInfo);

            if (pSurfaceInfo->surface) {
                local_pSurfaceInfo->surface = Unwrap(pSurfaceInfo->surface);
            }
        }
    }
    VkResult result = instance_dispatch_table.GetPhysicalDeviceSurfaceCapabilities2KHR(
        physicalDevice, (const VkPhysicalDeviceSurfaceInfo2KHR*)local_pSurfaceInfo, pSurfaceCapabilities);

    return result;
}

VkResult DispatchObject::GetPhysicalDeviceSurfaceFormats2KHR(VkPhysicalDevice physicalDevice,
                                                             const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo,
                                                             uint32_t* pSurfaceFormatCount, VkSurfaceFormat2KHR* pSurfaceFormats) {
    if (!wrap_handles)
        return instance_dispatch_table.GetPhysicalDeviceSurfaceFormats2KHR(physicalDevice, pSurfaceInfo, pSurfaceFormatCount,
                                                                           pSurfaceFormats);
    vku::safe_VkPhysicalDeviceSurfaceInfo2KHR var_local_pSurfaceInfo;
    vku::safe_VkPhysicalDeviceSurfaceInfo2KHR* local_pSurfaceInfo = nullptr;
    {
        if (pSurfaceInfo) {
            local_pSurfaceInfo = &var_local_pSurfaceInfo;
            local_pSurfaceInfo->initialize(pSurfaceInfo);

            if (pSurfaceInfo->surface) {
                local_pSurfaceInfo->surface = Unwrap(pSurfaceInfo->surface);
            }
        }
    }
    VkResult result = instance_dispatch_table.GetPhysicalDeviceSurfaceFormats2KHR(
        physicalDevice, (const VkPhysicalDeviceSurfaceInfo2KHR*)local_pSurfaceInfo, pSurfaceFormatCount, pSurfaceFormats);

    return result;
}

VkResult DispatchObject::GetDisplayPlaneCapabilities2KHR(VkPhysicalDevice physicalDevice,
                                                         const VkDisplayPlaneInfo2KHR* pDisplayPlaneInfo,
                                                         VkDisplayPlaneCapabilities2KHR* pCapabilities) {
    if (!wrap_handles)
        return instance_dispatch_table.GetDisplayPlaneCapabilities2KHR(physicalDevice, pDisplayPlaneInfo, pCapabilities);
    vku::safe_VkDisplayPlaneInfo2KHR var_local_pDisplayPlaneInfo;
    vku::safe_VkDisplayPlaneInfo2KHR* local_pDisplayPlaneInfo = nullptr;
    {
        if (pDisplayPlaneInfo) {
            local_pDisplayPlaneInfo = &var_local_pDisplayPlaneInfo;
            local_pDisplayPlaneInfo->initialize(pDisplayPlaneInfo);

            if (pDisplayPlaneInfo->mode) {
                local_pDisplayPlaneInfo->mode = Unwrap(pDisplayPlaneInfo->mode);
            }
        }
    }
    VkResult result = instance_dispatch_table.GetDisplayPlaneCapabilities2KHR(
        physicalDevice, (const VkDisplayPlaneInfo2KHR*)local_pDisplayPlaneInfo, pCapabilities);

    return result;
}

void DispatchObject::GetImageMemoryRequirements2KHR(VkDevice device, const VkImageMemoryRequirementsInfo2* pInfo,
                                                    VkMemoryRequirements2* pMemoryRequirements) {
    if (!wrap_handles) return device_dispatch_table.GetImageMemoryRequirements2KHR(device, pInfo, pMemoryRequirements);
    vku::safe_VkImageMemoryRequirementsInfo2 var_local_pInfo;
    vku::safe_VkImageMemoryRequirementsInfo2* local_pInfo = nullptr;
    {
        if (pInfo) {
            local_pInfo = &var_local_pInfo;
            local_pInfo->initialize(pInfo);

            if (pInfo->image) {
                local_pInfo->image = Unwrap(pInfo->image);
            }
        }
    }
    device_dispatch_table.GetImageMemoryRequirements2KHR(device, (const VkImageMemoryRequirementsInfo2*)local_pInfo,
                                                         pMemoryRequirements);
}

void DispatchObject::GetBufferMemoryRequirements2KHR(VkDevice device, const VkBufferMemoryRequirementsInfo2* pInfo,
                                                     VkMemoryRequirements2* pMemoryRequirements) {
    if (!wrap_handles) return device_dispatch_table.GetBufferMemoryRequirements2KHR(device, pInfo, pMemoryRequirements);
    vku::safe_VkBufferMemoryRequirementsInfo2 var_local_pInfo;
    vku::safe_VkBufferMemoryRequirementsInfo2* local_pInfo = nullptr;
    {
        if (pInfo) {
            local_pInfo = &var_local_pInfo;
            local_pInfo->initialize(pInfo);

            if (pInfo->buffer) {
                local_pInfo->buffer = Unwrap(pInfo->buffer);
            }
        }
    }
    device_dispatch_table.GetBufferMemoryRequirements2KHR(device, (const VkBufferMemoryRequirementsInfo2*)local_pInfo,
                                                          pMemoryRequirements);
}

void DispatchObject::GetImageSparseMemoryRequirements2KHR(VkDevice device, const VkImageSparseMemoryRequirementsInfo2* pInfo,
                                                          uint32_t* pSparseMemoryRequirementCount,
                                                          VkSparseImageMemoryRequirements2* pSparseMemoryRequirements) {
    if (!wrap_handles)
        return device_dispatch_table.GetImageSparseMemoryRequirements2KHR(device, pInfo, pSparseMemoryRequirementCount,
                                                                          pSparseMemoryRequirements);
    vku::safe_VkImageSparseMemoryRequirementsInfo2 var_local_pInfo;
    vku::safe_VkImageSparseMemoryRequirementsInfo2* local_pInfo = nullptr;
    {
        if (pInfo) {
            local_pInfo = &var_local_pInfo;
            local_pInfo->initialize(pInfo);

            if (pInfo->image) {
                local_pInfo->image = Unwrap(pInfo->image);
            }
        }
    }
    device_dispatch_table.GetImageSparseMemoryRequirements2KHR(device, (const VkImageSparseMemoryRequirementsInfo2*)local_pInfo,
                                                               pSparseMemoryRequirementCount, pSparseMemoryRequirements);
}

VkResult DispatchObject::CreateSamplerYcbcrConversionKHR(VkDevice device, const VkSamplerYcbcrConversionCreateInfo* pCreateInfo,
                                                         const VkAllocationCallbacks* pAllocator,
                                                         VkSamplerYcbcrConversion* pYcbcrConversion) {
    if (!wrap_handles)
        return device_dispatch_table.CreateSamplerYcbcrConversionKHR(device, pCreateInfo, pAllocator, pYcbcrConversion);

    VkResult result = device_dispatch_table.CreateSamplerYcbcrConversionKHR(device, pCreateInfo, pAllocator, pYcbcrConversion);
    if (VK_SUCCESS == result) {
        *pYcbcrConversion = WrapNew(*pYcbcrConversion);
    }
    return result;
}

void DispatchObject::DestroySamplerYcbcrConversionKHR(VkDevice device, VkSamplerYcbcrConversion ycbcrConversion,
                                                      const VkAllocationCallbacks* pAllocator) {
    if (!wrap_handles) return device_dispatch_table.DestroySamplerYcbcrConversionKHR(device, ycbcrConversion, pAllocator);

    uint64_t ycbcrConversion_id = CastToUint64(ycbcrConversion);
    auto iter = unique_id_mapping.pop(ycbcrConversion_id);
    if (iter != unique_id_mapping.end()) {
        ycbcrConversion = (VkSamplerYcbcrConversion)iter->second;
    } else {
        ycbcrConversion = (VkSamplerYcbcrConversion)0;
    }
    device_dispatch_table.DestroySamplerYcbcrConversionKHR(device, ycbcrConversion, pAllocator);
}

VkResult DispatchObject::BindBufferMemory2KHR(VkDevice device, uint32_t bindInfoCount, const VkBindBufferMemoryInfo* pBindInfos) {
    if (!wrap_handles) return device_dispatch_table.BindBufferMemory2KHR(device, bindInfoCount, pBindInfos);
    small_vector<vku::safe_VkBindBufferMemoryInfo, DISPATCH_MAX_STACK_ALLOCATIONS> var_local_pBindInfos;
    vku::safe_VkBindBufferMemoryInfo* local_pBindInfos = nullptr;
    {
        if (pBindInfos) {
            var_local_pBindInfos.resize(bindInfoCount);
            local_pBindInfos = var_local_pBindInfos.data();
            for (uint32_t index0 = 0; index0 < bindInfoCount; ++index0) {
                local_pBindInfos[index0].initialize(&pBindInfos[index0]);

                if (pBindInfos[index0].buffer) {
                    local_pBindInfos[index0].buffer = Unwrap(pBindInfos[index0].buffer);
                }
                if (pBindInfos[index0].memory) {
                    local_pBindInfos[index0].memory = Unwrap(pBindInfos[index0].memory);
                }
            }
        }
    }
    VkResult result =
        device_dispatch_table.BindBufferMemory2KHR(device, bindInfoCount, (const VkBindBufferMemoryInfo*)local_pBindInfos);

    return result;
}

VkResult DispatchObject::BindImageMemory2KHR(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo* pBindInfos) {
    if (!wrap_handles) return device_dispatch_table.BindImageMemory2KHR(device, bindInfoCount, pBindInfos);
    small_vector<vku::safe_VkBindImageMemoryInfo, DISPATCH_MAX_STACK_ALLOCATIONS> var_local_pBindInfos;
    vku::safe_VkBindImageMemoryInfo* local_pBindInfos = nullptr;
    {
        if (pBindInfos) {
            var_local_pBindInfos.resize(bindInfoCount);
            local_pBindInfos = var_local_pBindInfos.data();
            for (uint32_t index0 = 0; index0 < bindInfoCount; ++index0) {
                local_pBindInfos[index0].initialize(&pBindInfos[index0]);
                UnwrapPnextChainHandles(local_pBindInfos[index0].pNext);

                if (pBindInfos[index0].image) {
                    local_pBindInfos[index0].image = Unwrap(pBindInfos[index0].image);
                }
                if (pBindInfos[index0].memory) {
                    local_pBindInfos[index0].memory = Unwrap(pBindInfos[index0].memory);
                }
            }
        }
    }
    VkResult result =
        device_dispatch_table.BindImageMemory2KHR(device, bindInfoCount, (const VkBindImageMemoryInfo*)local_pBindInfos);

    return result;
}

void DispatchObject::GetDescriptorSetLayoutSupportKHR(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo,
                                                      VkDescriptorSetLayoutSupport* pSupport) {
    if (!wrap_handles) return device_dispatch_table.GetDescriptorSetLayoutSupportKHR(device, pCreateInfo, pSupport);
    vku::safe_VkDescriptorSetLayoutCreateInfo var_local_pCreateInfo;
    vku::safe_VkDescriptorSetLayoutCreateInfo* local_pCreateInfo = nullptr;
    {
        if (pCreateInfo) {
            local_pCreateInfo = &var_local_pCreateInfo;
            local_pCreateInfo->initialize(pCreateInfo);
            if (local_pCreateInfo->pBindings) {
                for (uint32_t index1 = 0; index1 < local_pCreateInfo->bindingCount; ++index1) {
                    if (local_pCreateInfo->pBindings[index1].pImmutableSamplers) {
                        for (uint32_t index2 = 0; index2 < local_pCreateInfo->pBindings[index1].descriptorCount; ++index2) {
                            local_pCreateInfo->pBindings[index1].pImmutableSamplers[index2] =
                                Unwrap(local_pCreateInfo->pBindings[index1].pImmutableSamplers[index2]);
                        }
                    }
                }
            }
        }
    }
    device_dispatch_table.GetDescriptorSetLayoutSupportKHR(device, (const VkDescriptorSetLayoutCreateInfo*)local_pCreateInfo,
                                                           pSupport);
}

void DispatchObject::CmdDrawIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                             VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                             uint32_t stride) {
    if (!wrap_handles)
        return device_dispatch_table.CmdDrawIndirectCountKHR(commandBuffer, buffer, offset, countBuffer, countBufferOffset,
                                                             maxDrawCount, stride);
    {
        buffer = Unwrap(buffer);
        countBuffer = Unwrap(countBuffer);
    }
    device_dispatch_table.CmdDrawIndirectCountKHR(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount,
                                                  stride);
}

void DispatchObject::CmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                    VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                    uint32_t stride) {
    if (!wrap_handles)
        return device_dispatch_table.CmdDrawIndexedIndirectCountKHR(commandBuffer, buffer, offset, countBuffer, countBufferOffset,
                                                                    maxDrawCount, stride);
    {
        buffer = Unwrap(buffer);
        countBuffer = Unwrap(countBuffer);
    }
    device_dispatch_table.CmdDrawIndexedIndirectCountKHR(commandBuffer, buffer, offset, countBuffer, countBufferOffset,
                                                         maxDrawCount, stride);
}

VkResult DispatchObject::GetSemaphoreCounterValueKHR(VkDevice device, VkSemaphore semaphore, uint64_t* pValue) {
    if (!wrap_handles) return device_dispatch_table.GetSemaphoreCounterValueKHR(device, semaphore, pValue);
    { semaphore = Unwrap(semaphore); }
    VkResult result = device_dispatch_table.GetSemaphoreCounterValueKHR(device, semaphore, pValue);

    return result;
}

VkResult DispatchObject::WaitSemaphoresKHR(VkDevice device, const VkSemaphoreWaitInfo* pWaitInfo, uint64_t timeout) {
    if (!wrap_handles) return device_dispatch_table.WaitSemaphoresKHR(device, pWaitInfo, timeout);
    vku::safe_VkSemaphoreWaitInfo var_local_pWaitInfo;
    vku::safe_VkSemaphoreWaitInfo* local_pWaitInfo = nullptr;
    {
        if (pWaitInfo) {
            local_pWaitInfo = &var_local_pWaitInfo;
            local_pWaitInfo->initialize(pWaitInfo);
            if (local_pWaitInfo->pSemaphores) {
                for (uint32_t index1 = 0; index1 < local_pWaitInfo->semaphoreCount; ++index1) {
                    local_pWaitInfo->pSemaphores[index1] = Unwrap(local_pWaitInfo->pSemaphores[index1]);
                }
            }
        }
    }
    VkResult result = device_dispatch_table.WaitSemaphoresKHR(device, (const VkSemaphoreWaitInfo*)local_pWaitInfo, timeout);

    return result;
}

VkResult DispatchObject::SignalSemaphoreKHR(VkDevice device, const VkSemaphoreSignalInfo* pSignalInfo) {
    if (!wrap_handles) return device_dispatch_table.SignalSemaphoreKHR(device, pSignalInfo);
    vku::safe_VkSemaphoreSignalInfo var_local_pSignalInfo;
    vku::safe_VkSemaphoreSignalInfo* local_pSignalInfo = nullptr;
    {
        if (pSignalInfo) {
            local_pSignalInfo = &var_local_pSignalInfo;
            local_pSignalInfo->initialize(pSignalInfo);

            if (pSignalInfo->semaphore) {
                local_pSignalInfo->semaphore = Unwrap(pSignalInfo->semaphore);
            }
        }
    }
    VkResult result = device_dispatch_table.SignalSemaphoreKHR(device, (const VkSemaphoreSignalInfo*)local_pSignalInfo);

    return result;
}

VkResult DispatchObject::GetPhysicalDeviceFragmentShadingRatesKHR(VkPhysicalDevice physicalDevice,
                                                                  uint32_t* pFragmentShadingRateCount,
                                                                  VkPhysicalDeviceFragmentShadingRateKHR* pFragmentShadingRates) {
    VkResult result = instance_dispatch_table.GetPhysicalDeviceFragmentShadingRatesKHR(physicalDevice, pFragmentShadingRateCount,
                                                                                       pFragmentShadingRates);

    return result;
}

void DispatchObject::CmdSetFragmentShadingRateKHR(VkCommandBuffer commandBuffer, const VkExtent2D* pFragmentSize,
                                                  const VkFragmentShadingRateCombinerOpKHR combinerOps[2]) {
    device_dispatch_table.CmdSetFragmentShadingRateKHR(commandBuffer, pFragmentSize, combinerOps);
}

void DispatchObject::CmdSetRenderingAttachmentLocationsKHR(VkCommandBuffer commandBuffer,
                                                           const VkRenderingAttachmentLocationInfoKHR* pLocationInfo) {
    device_dispatch_table.CmdSetRenderingAttachmentLocationsKHR(commandBuffer, pLocationInfo);
}

void DispatchObject::CmdSetRenderingInputAttachmentIndicesKHR(
    VkCommandBuffer commandBuffer, const VkRenderingInputAttachmentIndexInfoKHR* pInputAttachmentIndexInfo) {
    device_dispatch_table.CmdSetRenderingInputAttachmentIndicesKHR(commandBuffer, pInputAttachmentIndexInfo);
}

VkResult DispatchObject::WaitForPresentKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t presentId, uint64_t timeout) {
    if (!wrap_handles) return device_dispatch_table.WaitForPresentKHR(device, swapchain, presentId, timeout);
    { swapchain = Unwrap(swapchain); }
    VkResult result = device_dispatch_table.WaitForPresentKHR(device, swapchain, presentId, timeout);

    return result;
}

VkDeviceAddress DispatchObject::GetBufferDeviceAddressKHR(VkDevice device, const VkBufferDeviceAddressInfo* pInfo) {
    if (!wrap_handles) return device_dispatch_table.GetBufferDeviceAddressKHR(device, pInfo);
    vku::safe_VkBufferDeviceAddressInfo var_local_pInfo;
    vku::safe_VkBufferDeviceAddressInfo* local_pInfo = nullptr;
    {
        if (pInfo) {
            local_pInfo = &var_local_pInfo;
            local_pInfo->initialize(pInfo);

            if (pInfo->buffer) {
                local_pInfo->buffer = Unwrap(pInfo->buffer);
            }
        }
    }
    VkDeviceAddress result = device_dispatch_table.GetBufferDeviceAddressKHR(device, (const VkBufferDeviceAddressInfo*)local_pInfo);

    return result;
}

uint64_t DispatchObject::GetBufferOpaqueCaptureAddressKHR(VkDevice device, const VkBufferDeviceAddressInfo* pInfo) {
    if (!wrap_handles) return device_dispatch_table.GetBufferOpaqueCaptureAddressKHR(device, pInfo);
    vku::safe_VkBufferDeviceAddressInfo var_local_pInfo;
    vku::safe_VkBufferDeviceAddressInfo* local_pInfo = nullptr;
    {
        if (pInfo) {
            local_pInfo = &var_local_pInfo;
            local_pInfo->initialize(pInfo);

            if (pInfo->buffer) {
                local_pInfo->buffer = Unwrap(pInfo->buffer);
            }
        }
    }
    uint64_t result = device_dispatch_table.GetBufferOpaqueCaptureAddressKHR(device, (const VkBufferDeviceAddressInfo*)local_pInfo);

    return result;
}

uint64_t DispatchObject::GetDeviceMemoryOpaqueCaptureAddressKHR(VkDevice device,
                                                                const VkDeviceMemoryOpaqueCaptureAddressInfo* pInfo) {
    if (!wrap_handles) return device_dispatch_table.GetDeviceMemoryOpaqueCaptureAddressKHR(device, pInfo);
    vku::safe_VkDeviceMemoryOpaqueCaptureAddressInfo var_local_pInfo;
    vku::safe_VkDeviceMemoryOpaqueCaptureAddressInfo* local_pInfo = nullptr;
    {
        if (pInfo) {
            local_pInfo = &var_local_pInfo;
            local_pInfo->initialize(pInfo);

            if (pInfo->memory) {
                local_pInfo->memory = Unwrap(pInfo->memory);
            }
        }
    }
    uint64_t result = device_dispatch_table.GetDeviceMemoryOpaqueCaptureAddressKHR(
        device, (const VkDeviceMemoryOpaqueCaptureAddressInfo*)local_pInfo);

    return result;
}

VkResult DispatchObject::CreateDeferredOperationKHR(VkDevice device, const VkAllocationCallbacks* pAllocator,
                                                    VkDeferredOperationKHR* pDeferredOperation) {
    if (!wrap_handles) return device_dispatch_table.CreateDeferredOperationKHR(device, pAllocator, pDeferredOperation);

    VkResult result = device_dispatch_table.CreateDeferredOperationKHR(device, pAllocator, pDeferredOperation);
    if (VK_SUCCESS == result) {
        *pDeferredOperation = WrapNew(*pDeferredOperation);
    }
    return result;
}

void DispatchObject::DestroyDeferredOperationKHR(VkDevice device, VkDeferredOperationKHR operation,
                                                 const VkAllocationCallbacks* pAllocator) {
    if (!wrap_handles) return device_dispatch_table.DestroyDeferredOperationKHR(device, operation, pAllocator);

    uint64_t operation_id = CastToUint64(operation);
    auto iter = unique_id_mapping.pop(operation_id);
    if (iter != unique_id_mapping.end()) {
        operation = (VkDeferredOperationKHR)iter->second;
    } else {
        operation = (VkDeferredOperationKHR)0;
    }
    device_dispatch_table.DestroyDeferredOperationKHR(device, operation, pAllocator);
}

uint32_t DispatchObject::GetDeferredOperationMaxConcurrencyKHR(VkDevice device, VkDeferredOperationKHR operation) {
    if (!wrap_handles) return device_dispatch_table.GetDeferredOperationMaxConcurrencyKHR(device, operation);
    { operation = Unwrap(operation); }
    uint32_t result = device_dispatch_table.GetDeferredOperationMaxConcurrencyKHR(device, operation);

    return result;
}

VkResult DispatchObject::GetPipelineExecutablePropertiesKHR(VkDevice device, const VkPipelineInfoKHR* pPipelineInfo,
                                                            uint32_t* pExecutableCount,
                                                            VkPipelineExecutablePropertiesKHR* pProperties) {
    if (!wrap_handles)
        return device_dispatch_table.GetPipelineExecutablePropertiesKHR(device, pPipelineInfo, pExecutableCount, pProperties);
    vku::safe_VkPipelineInfoKHR var_local_pPipelineInfo;
    vku::safe_VkPipelineInfoKHR* local_pPipelineInfo = nullptr;
    {
        if (pPipelineInfo) {
            local_pPipelineInfo = &var_local_pPipelineInfo;
            local_pPipelineInfo->initialize(pPipelineInfo);

            if (pPipelineInfo->pipeline) {
                local_pPipelineInfo->pipeline = Unwrap(pPipelineInfo->pipeline);
            }
        }
    }
    VkResult result = device_dispatch_table.GetPipelineExecutablePropertiesKHR(
        device, (const VkPipelineInfoKHR*)local_pPipelineInfo, pExecutableCount, pProperties);

    return result;
}

VkResult DispatchObject::GetPipelineExecutableStatisticsKHR(VkDevice device, const VkPipelineExecutableInfoKHR* pExecutableInfo,
                                                            uint32_t* pStatisticCount,
                                                            VkPipelineExecutableStatisticKHR* pStatistics) {
    if (!wrap_handles)
        return device_dispatch_table.GetPipelineExecutableStatisticsKHR(device, pExecutableInfo, pStatisticCount, pStatistics);
    vku::safe_VkPipelineExecutableInfoKHR var_local_pExecutableInfo;
    vku::safe_VkPipelineExecutableInfoKHR* local_pExecutableInfo = nullptr;
    {
        if (pExecutableInfo) {
            local_pExecutableInfo = &var_local_pExecutableInfo;
            local_pExecutableInfo->initialize(pExecutableInfo);

            if (pExecutableInfo->pipeline) {
                local_pExecutableInfo->pipeline = Unwrap(pExecutableInfo->pipeline);
            }
        }
    }
    VkResult result = device_dispatch_table.GetPipelineExecutableStatisticsKHR(
        device, (const VkPipelineExecutableInfoKHR*)local_pExecutableInfo, pStatisticCount, pStatistics);

    return result;
}

VkResult DispatchObject::GetPipelineExecutableInternalRepresentationsKHR(
    VkDevice device, const VkPipelineExecutableInfoKHR* pExecutableInfo, uint32_t* pInternalRepresentationCount,
    VkPipelineExecutableInternalRepresentationKHR* pInternalRepresentations) {
    if (!wrap_handles)
        return device_dispatch_table.GetPipelineExecutableInternalRepresentationsKHR(
            device, pExecutableInfo, pInternalRepresentationCount, pInternalRepresentations);
    vku::safe_VkPipelineExecutableInfoKHR var_local_pExecutableInfo;
    vku::safe_VkPipelineExecutableInfoKHR* local_pExecutableInfo = nullptr;
    {
        if (pExecutableInfo) {
            local_pExecutableInfo = &var_local_pExecutableInfo;
            local_pExecutableInfo->initialize(pExecutableInfo);

            if (pExecutableInfo->pipeline) {
                local_pExecutableInfo->pipeline = Unwrap(pExecutableInfo->pipeline);
            }
        }
    }
    VkResult result = device_dispatch_table.GetPipelineExecutableInternalRepresentationsKHR(
        device, (const VkPipelineExecutableInfoKHR*)local_pExecutableInfo, pInternalRepresentationCount, pInternalRepresentations);

    return result;
}

VkResult DispatchObject::MapMemory2KHR(VkDevice device, const VkMemoryMapInfoKHR* pMemoryMapInfo, void** ppData) {
    if (!wrap_handles) return device_dispatch_table.MapMemory2KHR(device, pMemoryMapInfo, ppData);
    vku::safe_VkMemoryMapInfoKHR var_local_pMemoryMapInfo;
    vku::safe_VkMemoryMapInfoKHR* local_pMemoryMapInfo = nullptr;
    {
        if (pMemoryMapInfo) {
            local_pMemoryMapInfo = &var_local_pMemoryMapInfo;
            local_pMemoryMapInfo->initialize(pMemoryMapInfo);

            if (pMemoryMapInfo->memory) {
                local_pMemoryMapInfo->memory = Unwrap(pMemoryMapInfo->memory);
            }
        }
    }
    VkResult result = device_dispatch_table.MapMemory2KHR(device, (const VkMemoryMapInfoKHR*)local_pMemoryMapInfo, ppData);

    return result;
}

VkResult DispatchObject::UnmapMemory2KHR(VkDevice device, const VkMemoryUnmapInfoKHR* pMemoryUnmapInfo) {
    if (!wrap_handles) return device_dispatch_table.UnmapMemory2KHR(device, pMemoryUnmapInfo);
    vku::safe_VkMemoryUnmapInfoKHR var_local_pMemoryUnmapInfo;
    vku::safe_VkMemoryUnmapInfoKHR* local_pMemoryUnmapInfo = nullptr;
    {
        if (pMemoryUnmapInfo) {
            local_pMemoryUnmapInfo = &var_local_pMemoryUnmapInfo;
            local_pMemoryUnmapInfo->initialize(pMemoryUnmapInfo);

            if (pMemoryUnmapInfo->memory) {
                local_pMemoryUnmapInfo->memory = Unwrap(pMemoryUnmapInfo->memory);
            }
        }
    }
    VkResult result = device_dispatch_table.UnmapMemory2KHR(device, (const VkMemoryUnmapInfoKHR*)local_pMemoryUnmapInfo);

    return result;
}

VkResult DispatchObject::GetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceVideoEncodeQualityLevelInfoKHR* pQualityLevelInfo,
    VkVideoEncodeQualityLevelPropertiesKHR* pQualityLevelProperties) {
    VkResult result = instance_dispatch_table.GetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR(
        physicalDevice, pQualityLevelInfo, pQualityLevelProperties);

    return result;
}

VkResult DispatchObject::GetEncodedVideoSessionParametersKHR(
    VkDevice device, const VkVideoEncodeSessionParametersGetInfoKHR* pVideoSessionParametersInfo,
    VkVideoEncodeSessionParametersFeedbackInfoKHR* pFeedbackInfo, size_t* pDataSize, void* pData) {
    if (!wrap_handles)
        return device_dispatch_table.GetEncodedVideoSessionParametersKHR(device, pVideoSessionParametersInfo, pFeedbackInfo,
                                                                         pDataSize, pData);
    vku::safe_VkVideoEncodeSessionParametersGetInfoKHR var_local_pVideoSessionParametersInfo;
    vku::safe_VkVideoEncodeSessionParametersGetInfoKHR* local_pVideoSessionParametersInfo = nullptr;
    {
        if (pVideoSessionParametersInfo) {
            local_pVideoSessionParametersInfo = &var_local_pVideoSessionParametersInfo;
            local_pVideoSessionParametersInfo->initialize(pVideoSessionParametersInfo);

            if (pVideoSessionParametersInfo->videoSessionParameters) {
                local_pVideoSessionParametersInfo->videoSessionParameters =
                    Unwrap(pVideoSessionParametersInfo->videoSessionParameters);
            }
        }
    }
    VkResult result = device_dispatch_table.GetEncodedVideoSessionParametersKHR(
        device, (const VkVideoEncodeSessionParametersGetInfoKHR*)local_pVideoSessionParametersInfo, pFeedbackInfo, pDataSize,
        pData);

    return result;
}

void DispatchObject::CmdEncodeVideoKHR(VkCommandBuffer commandBuffer, const VkVideoEncodeInfoKHR* pEncodeInfo) {
    if (!wrap_handles) return device_dispatch_table.CmdEncodeVideoKHR(commandBuffer, pEncodeInfo);
    vku::safe_VkVideoEncodeInfoKHR var_local_pEncodeInfo;
    vku::safe_VkVideoEncodeInfoKHR* local_pEncodeInfo = nullptr;
    {
        if (pEncodeInfo) {
            local_pEncodeInfo = &var_local_pEncodeInfo;
            local_pEncodeInfo->initialize(pEncodeInfo);

            if (pEncodeInfo->dstBuffer) {
                local_pEncodeInfo->dstBuffer = Unwrap(pEncodeInfo->dstBuffer);
            }
            if (pEncodeInfo->srcPictureResource.imageViewBinding) {
                local_pEncodeInfo->srcPictureResource.imageViewBinding = Unwrap(pEncodeInfo->srcPictureResource.imageViewBinding);
            }
            if (local_pEncodeInfo->pSetupReferenceSlot) {
                if (local_pEncodeInfo->pSetupReferenceSlot->pPictureResource) {
                    if (pEncodeInfo->pSetupReferenceSlot->pPictureResource->imageViewBinding) {
                        local_pEncodeInfo->pSetupReferenceSlot->pPictureResource->imageViewBinding =
                            Unwrap(pEncodeInfo->pSetupReferenceSlot->pPictureResource->imageViewBinding);
                    }
                }
            }
            if (local_pEncodeInfo->pReferenceSlots) {
                for (uint32_t index1 = 0; index1 < local_pEncodeInfo->referenceSlotCount; ++index1) {
                    if (local_pEncodeInfo->pReferenceSlots[index1].pPictureResource) {
                        if (pEncodeInfo->pReferenceSlots[index1].pPictureResource->imageViewBinding) {
                            local_pEncodeInfo->pReferenceSlots[index1].pPictureResource->imageViewBinding =
                                Unwrap(pEncodeInfo->pReferenceSlots[index1].pPictureResource->imageViewBinding);
                        }
                    }
                }
            }
            UnwrapPnextChainHandles(local_pEncodeInfo->pNext);
        }
    }
    device_dispatch_table.CmdEncodeVideoKHR(commandBuffer, (const VkVideoEncodeInfoKHR*)local_pEncodeInfo);
}

void DispatchObject::CmdSetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event, const VkDependencyInfo* pDependencyInfo) {
    if (!wrap_handles) return device_dispatch_table.CmdSetEvent2KHR(commandBuffer, event, pDependencyInfo);
    vku::safe_VkDependencyInfo var_local_pDependencyInfo;
    vku::safe_VkDependencyInfo* local_pDependencyInfo = nullptr;
    {
        event = Unwrap(event);
        if (pDependencyInfo) {
            local_pDependencyInfo = &var_local_pDependencyInfo;
            local_pDependencyInfo->initialize(pDependencyInfo);
            if (local_pDependencyInfo->pBufferMemoryBarriers) {
                for (uint32_t index1 = 0; index1 < local_pDependencyInfo->bufferMemoryBarrierCount; ++index1) {
                    if (pDependencyInfo->pBufferMemoryBarriers[index1].buffer) {
                        local_pDependencyInfo->pBufferMemoryBarriers[index1].buffer =
                            Unwrap(pDependencyInfo->pBufferMemoryBarriers[index1].buffer);
                    }
                }
            }
            if (local_pDependencyInfo->pImageMemoryBarriers) {
                for (uint32_t index1 = 0; index1 < local_pDependencyInfo->imageMemoryBarrierCount; ++index1) {
                    if (pDependencyInfo->pImageMemoryBarriers[index1].image) {
                        local_pDependencyInfo->pImageMemoryBarriers[index1].image =
                            Unwrap(pDependencyInfo->pImageMemoryBarriers[index1].image);
                    }
                }
            }
        }
    }
    device_dispatch_table.CmdSetEvent2KHR(commandBuffer, event, (const VkDependencyInfo*)local_pDependencyInfo);
}

void DispatchObject::CmdResetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags2 stageMask) {
    if (!wrap_handles) return device_dispatch_table.CmdResetEvent2KHR(commandBuffer, event, stageMask);
    { event = Unwrap(event); }
    device_dispatch_table.CmdResetEvent2KHR(commandBuffer, event, stageMask);
}

void DispatchObject::CmdWaitEvents2KHR(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                       const VkDependencyInfo* pDependencyInfos) {
    if (!wrap_handles) return device_dispatch_table.CmdWaitEvents2KHR(commandBuffer, eventCount, pEvents, pDependencyInfos);
    small_vector<VkEvent, DISPATCH_MAX_STACK_ALLOCATIONS> var_local_pEvents;
    VkEvent* local_pEvents = nullptr;
    small_vector<vku::safe_VkDependencyInfo, DISPATCH_MAX_STACK_ALLOCATIONS> var_local_pDependencyInfos;
    vku::safe_VkDependencyInfo* local_pDependencyInfos = nullptr;
    {
        if (pEvents) {
            var_local_pEvents.resize(eventCount);
            local_pEvents = var_local_pEvents.data();
            for (uint32_t index0 = 0; index0 < eventCount; ++index0) {
                local_pEvents[index0] = Unwrap(pEvents[index0]);
            }
        }
        if (pDependencyInfos) {
            var_local_pDependencyInfos.resize(eventCount);
            local_pDependencyInfos = var_local_pDependencyInfos.data();
            for (uint32_t index0 = 0; index0 < eventCount; ++index0) {
                local_pDependencyInfos[index0].initialize(&pDependencyInfos[index0]);
                if (local_pDependencyInfos[index0].pBufferMemoryBarriers) {
                    for (uint32_t index1 = 0; index1 < local_pDependencyInfos[index0].bufferMemoryBarrierCount; ++index1) {
                        if (pDependencyInfos[index0].pBufferMemoryBarriers[index1].buffer) {
                            local_pDependencyInfos[index0].pBufferMemoryBarriers[index1].buffer =
                                Unwrap(pDependencyInfos[index0].pBufferMemoryBarriers[index1].buffer);
                        }
                    }
                }
                if (local_pDependencyInfos[index0].pImageMemoryBarriers) {
                    for (uint32_t index1 = 0; index1 < local_pDependencyInfos[index0].imageMemoryBarrierCount; ++index1) {
                        if (pDependencyInfos[index0].pImageMemoryBarriers[index1].image) {
                            local_pDependencyInfos[index0].pImageMemoryBarriers[index1].image =
                                Unwrap(pDependencyInfos[index0].pImageMemoryBarriers[index1].image);
                        }
                    }
                }
            }
        }
    }
    device_dispatch_table.CmdWaitEvents2KHR(commandBuffer, eventCount, (const VkEvent*)local_pEvents,
                                            (const VkDependencyInfo*)local_pDependencyInfos);
}

void DispatchObject::CmdPipelineBarrier2KHR(VkCommandBuffer commandBuffer, const VkDependencyInfo* pDependencyInfo) {
    if (!wrap_handles) return device_dispatch_table.CmdPipelineBarrier2KHR(commandBuffer, pDependencyInfo);
    vku::safe_VkDependencyInfo var_local_pDependencyInfo;
    vku::safe_VkDependencyInfo* local_pDependencyInfo = nullptr;
    {
        if (pDependencyInfo) {
            local_pDependencyInfo = &var_local_pDependencyInfo;
            local_pDependencyInfo->initialize(pDependencyInfo);
            if (local_pDependencyInfo->pBufferMemoryBarriers) {
                for (uint32_t index1 = 0; index1 < local_pDependencyInfo->bufferMemoryBarrierCount; ++index1) {
                    if (pDependencyInfo->pBufferMemoryBarriers[index1].buffer) {
                        local_pDependencyInfo->pBufferMemoryBarriers[index1].buffer =
                            Unwrap(pDependencyInfo->pBufferMemoryBarriers[index1].buffer);
                    }
                }
            }
            if (local_pDependencyInfo->pImageMemoryBarriers) {
                for (uint32_t index1 = 0; index1 < local_pDependencyInfo->imageMemoryBarrierCount; ++index1) {
                    if (pDependencyInfo->pImageMemoryBarriers[index1].image) {
                        local_pDependencyInfo->pImageMemoryBarriers[index1].image =
                            Unwrap(pDependencyInfo->pImageMemoryBarriers[index1].image);
                    }
                }
            }
        }
    }
    device_dispatch_table.CmdPipelineBarrier2KHR(commandBuffer, (const VkDependencyInfo*)local_pDependencyInfo);
}

void DispatchObject::CmdWriteTimestamp2KHR(VkCommandBuffer commandBuffer, VkPipelineStageFlags2 stage, VkQueryPool queryPool,
                                           uint32_t query) {
    if (!wrap_handles) return device_dispatch_table.CmdWriteTimestamp2KHR(commandBuffer, stage, queryPool, query);
    { queryPool = Unwrap(queryPool); }
    device_dispatch_table.CmdWriteTimestamp2KHR(commandBuffer, stage, queryPool, query);
}

VkResult DispatchObject::QueueSubmit2KHR(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2* pSubmits, VkFence fence) {
    if (!wrap_handles) return device_dispatch_table.QueueSubmit2KHR(queue, submitCount, pSubmits, fence);
    small_vector<vku::safe_VkSubmitInfo2, DISPATCH_MAX_STACK_ALLOCATIONS> var_local_pSubmits;
    vku::safe_VkSubmitInfo2* local_pSubmits = nullptr;
    {
        if (pSubmits) {
            var_local_pSubmits.resize(submitCount);
            local_pSubmits = var_local_pSubmits.data();
            for (uint32_t index0 = 0; index0 < submitCount; ++index0) {
                local_pSubmits[index0].initialize(&pSubmits[index0]);
                UnwrapPnextChainHandles(local_pSubmits[index0].pNext);
                if (local_pSubmits[index0].pWaitSemaphoreInfos) {
                    for (uint32_t index1 = 0; index1 < local_pSubmits[index0].waitSemaphoreInfoCount; ++index1) {
                        if (pSubmits[index0].pWaitSemaphoreInfos[index1].semaphore) {
                            local_pSubmits[index0].pWaitSemaphoreInfos[index1].semaphore =
                                Unwrap(pSubmits[index0].pWaitSemaphoreInfos[index1].semaphore);
                        }
                    }
                }
                if (local_pSubmits[index0].pCommandBufferInfos) {
                    for (uint32_t index1 = 0; index1 < local_pSubmits[index0].commandBufferInfoCount; ++index1) {
                        UnwrapPnextChainHandles(local_pSubmits[index0].pCommandBufferInfos[index1].pNext);
                    }
                }
                if (local_pSubmits[index0].pSignalSemaphoreInfos) {
                    for (uint32_t index1 = 0; index1 < local_pSubmits[index0].signalSemaphoreInfoCount; ++index1) {
                        if (pSubmits[index0].pSignalSemaphoreInfos[index1].semaphore) {
                            local_pSubmits[index0].pSignalSemaphoreInfos[index1].semaphore =
                                Unwrap(pSubmits[index0].pSignalSemaphoreInfos[index1].semaphore);
                        }
                    }
                }
            }
        }
        fence = Unwrap(fence);
    }
    VkResult result = device_dispatch_table.QueueSubmit2KHR(queue, submitCount, (const VkSubmitInfo2*)local_pSubmits, fence);

    return result;
}

void DispatchObject::CmdCopyBuffer2KHR(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2* pCopyBufferInfo) {
    if (!wrap_handles) return device_dispatch_table.CmdCopyBuffer2KHR(commandBuffer, pCopyBufferInfo);
    vku::safe_VkCopyBufferInfo2 var_local_pCopyBufferInfo;
    vku::safe_VkCopyBufferInfo2* local_pCopyBufferInfo = nullptr;
    {
        if (pCopyBufferInfo) {
            local_pCopyBufferInfo = &var_local_pCopyBufferInfo;
            local_pCopyBufferInfo->initialize(pCopyBufferInfo);

            if (pCopyBufferInfo->srcBuffer) {
                local_pCopyBufferInfo->srcBuffer = Unwrap(pCopyBufferInfo->srcBuffer);
            }
            if (pCopyBufferInfo->dstBuffer) {
                local_pCopyBufferInfo->dstBuffer = Unwrap(pCopyBufferInfo->dstBuffer);
            }
        }
    }
    device_dispatch_table.CmdCopyBuffer2KHR(commandBuffer, (const VkCopyBufferInfo2*)local_pCopyBufferInfo);
}

void DispatchObject::CmdCopyImage2KHR(VkCommandBuffer commandBuffer, const VkCopyImageInfo2* pCopyImageInfo) {
    if (!wrap_handles) return device_dispatch_table.CmdCopyImage2KHR(commandBuffer, pCopyImageInfo);
    vku::safe_VkCopyImageInfo2 var_local_pCopyImageInfo;
    vku::safe_VkCopyImageInfo2* local_pCopyImageInfo = nullptr;
    {
        if (pCopyImageInfo) {
            local_pCopyImageInfo = &var_local_pCopyImageInfo;
            local_pCopyImageInfo->initialize(pCopyImageInfo);

            if (pCopyImageInfo->srcImage) {
                local_pCopyImageInfo->srcImage = Unwrap(pCopyImageInfo->srcImage);
            }
            if (pCopyImageInfo->dstImage) {
                local_pCopyImageInfo->dstImage = Unwrap(pCopyImageInfo->dstImage);
            }
        }
    }
    device_dispatch_table.CmdCopyImage2KHR(commandBuffer, (const VkCopyImageInfo2*)local_pCopyImageInfo);
}

void DispatchObject::CmdCopyBufferToImage2KHR(VkCommandBuffer commandBuffer,
                                              const VkCopyBufferToImageInfo2* pCopyBufferToImageInfo) {
    if (!wrap_handles) return device_dispatch_table.CmdCopyBufferToImage2KHR(commandBuffer, pCopyBufferToImageInfo);
    vku::safe_VkCopyBufferToImageInfo2 var_local_pCopyBufferToImageInfo;
    vku::safe_VkCopyBufferToImageInfo2* local_pCopyBufferToImageInfo = nullptr;
    {
        if (pCopyBufferToImageInfo) {
            local_pCopyBufferToImageInfo = &var_local_pCopyBufferToImageInfo;
            local_pCopyBufferToImageInfo->initialize(pCopyBufferToImageInfo);

            if (pCopyBufferToImageInfo->srcBuffer) {
                local_pCopyBufferToImageInfo->srcBuffer = Unwrap(pCopyBufferToImageInfo->srcBuffer);
            }
            if (pCopyBufferToImageInfo->dstImage) {
                local_pCopyBufferToImageInfo->dstImage = Unwrap(pCopyBufferToImageInfo->dstImage);
            }
        }
    }
    device_dispatch_table.CmdCopyBufferToImage2KHR(commandBuffer, (const VkCopyBufferToImageInfo2*)local_pCopyBufferToImageInfo);
}

void DispatchObject::CmdCopyImageToBuffer2KHR(VkCommandBuffer commandBuffer,
                                              const VkCopyImageToBufferInfo2* pCopyImageToBufferInfo) {
    if (!wrap_handles) return device_dispatch_table.CmdCopyImageToBuffer2KHR(commandBuffer, pCopyImageToBufferInfo);
    vku::safe_VkCopyImageToBufferInfo2 var_local_pCopyImageToBufferInfo;
    vku::safe_VkCopyImageToBufferInfo2* local_pCopyImageToBufferInfo = nullptr;
    {
        if (pCopyImageToBufferInfo) {
            local_pCopyImageToBufferInfo = &var_local_pCopyImageToBufferInfo;
            local_pCopyImageToBufferInfo->initialize(pCopyImageToBufferInfo);

            if (pCopyImageToBufferInfo->srcImage) {
                local_pCopyImageToBufferInfo->srcImage = Unwrap(pCopyImageToBufferInfo->srcImage);
            }
            if (pCopyImageToBufferInfo->dstBuffer) {
                local_pCopyImageToBufferInfo->dstBuffer = Unwrap(pCopyImageToBufferInfo->dstBuffer);
            }
        }
    }
    device_dispatch_table.CmdCopyImageToBuffer2KHR(commandBuffer, (const VkCopyImageToBufferInfo2*)local_pCopyImageToBufferInfo);
}

void DispatchObject::CmdBlitImage2KHR(VkCommandBuffer commandBuffer, const VkBlitImageInfo2* pBlitImageInfo) {
    if (!wrap_handles) return device_dispatch_table.CmdBlitImage2KHR(commandBuffer, pBlitImageInfo);
    vku::safe_VkBlitImageInfo2 var_local_pBlitImageInfo;
    vku::safe_VkBlitImageInfo2* local_pBlitImageInfo = nullptr;
    {
        if (pBlitImageInfo) {
            local_pBlitImageInfo = &var_local_pBlitImageInfo;
            local_pBlitImageInfo->initialize(pBlitImageInfo);

            if (pBlitImageInfo->srcImage) {
                local_pBlitImageInfo->srcImage = Unwrap(pBlitImageInfo->srcImage);
            }
            if (pBlitImageInfo->dstImage) {
                local_pBlitImageInfo->dstImage = Unwrap(pBlitImageInfo->dstImage);
            }
        }
    }
    device_dispatch_table.CmdBlitImage2KHR(commandBuffer, (const VkBlitImageInfo2*)local_pBlitImageInfo);
}

void DispatchObject::CmdResolveImage2KHR(VkCommandBuffer commandBuffer, const VkResolveImageInfo2* pResolveImageInfo) {
    if (!wrap_handles) return device_dispatch_table.CmdResolveImage2KHR(commandBuffer, pResolveImageInfo);
    vku::safe_VkResolveImageInfo2 var_local_pResolveImageInfo;
    vku::safe_VkResolveImageInfo2* local_pResolveImageInfo = nullptr;
    {
        if (pResolveImageInfo) {
            local_pResolveImageInfo = &var_local_pResolveImageInfo;
            local_pResolveImageInfo->initialize(pResolveImageInfo);

            if (pResolveImageInfo->srcImage) {
                local_pResolveImageInfo->srcImage = Unwrap(pResolveImageInfo->srcImage);
            }
            if (pResolveImageInfo->dstImage) {
                local_pResolveImageInfo->dstImage = Unwrap(pResolveImageInfo->dstImage);
            }
        }
    }
    device_dispatch_table.CmdResolveImage2KHR(commandBuffer, (const VkResolveImageInfo2*)local_pResolveImageInfo);
}

void DispatchObject::CmdTraceRaysIndirect2KHR(VkCommandBuffer commandBuffer, VkDeviceAddress indirectDeviceAddress) {
    device_dispatch_table.CmdTraceRaysIndirect2KHR(commandBuffer, indirectDeviceAddress);
}

void DispatchObject::GetDeviceBufferMemoryRequirementsKHR(VkDevice device, const VkDeviceBufferMemoryRequirements* pInfo,
                                                          VkMemoryRequirements2* pMemoryRequirements) {
    device_dispatch_table.GetDeviceBufferMemoryRequirementsKHR(device, pInfo, pMemoryRequirements);
}

void DispatchObject::GetDeviceImageMemoryRequirementsKHR(VkDevice device, const VkDeviceImageMemoryRequirements* pInfo,
                                                         VkMemoryRequirements2* pMemoryRequirements) {
    device_dispatch_table.GetDeviceImageMemoryRequirementsKHR(device, pInfo, pMemoryRequirements);
}

void DispatchObject::GetDeviceImageSparseMemoryRequirementsKHR(VkDevice device, const VkDeviceImageMemoryRequirements* pInfo,
                                                               uint32_t* pSparseMemoryRequirementCount,
                                                               VkSparseImageMemoryRequirements2* pSparseMemoryRequirements) {
    device_dispatch_table.GetDeviceImageSparseMemoryRequirementsKHR(device, pInfo, pSparseMemoryRequirementCount,
                                                                    pSparseMemoryRequirements);
}

void DispatchObject::CmdBindIndexBuffer2KHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkDeviceSize size,
                                            VkIndexType indexType) {
    if (!wrap_handles) return device_dispatch_table.CmdBindIndexBuffer2KHR(commandBuffer, buffer, offset, size, indexType);
    { buffer = Unwrap(buffer); }
    device_dispatch_table.CmdBindIndexBuffer2KHR(commandBuffer, buffer, offset, size, indexType);
}

void DispatchObject::GetRenderingAreaGranularityKHR(VkDevice device, const VkRenderingAreaInfoKHR* pRenderingAreaInfo,
                                                    VkExtent2D* pGranularity) {
    device_dispatch_table.GetRenderingAreaGranularityKHR(device, pRenderingAreaInfo, pGranularity);
}

void DispatchObject::GetDeviceImageSubresourceLayoutKHR(VkDevice device, const VkDeviceImageSubresourceInfoKHR* pInfo,
                                                        VkSubresourceLayout2KHR* pLayout) {
    device_dispatch_table.GetDeviceImageSubresourceLayoutKHR(device, pInfo, pLayout);
}

void DispatchObject::GetImageSubresourceLayout2KHR(VkDevice device, VkImage image, const VkImageSubresource2KHR* pSubresource,
                                                   VkSubresourceLayout2KHR* pLayout) {
    if (!wrap_handles) return device_dispatch_table.GetImageSubresourceLayout2KHR(device, image, pSubresource, pLayout);
    { image = Unwrap(image); }
    device_dispatch_table.GetImageSubresourceLayout2KHR(device, image, pSubresource, pLayout);
}

void DispatchObject::DestroyPipelineBinaryKHR(VkDevice device, VkPipelineBinaryKHR pipelineBinary,
                                              const VkAllocationCallbacks* pAllocator) {
    if (!wrap_handles) return device_dispatch_table.DestroyPipelineBinaryKHR(device, pipelineBinary, pAllocator);

    uint64_t pipelineBinary_id = CastToUint64(pipelineBinary);
    auto iter = unique_id_mapping.pop(pipelineBinary_id);
    if (iter != unique_id_mapping.end()) {
        pipelineBinary = (VkPipelineBinaryKHR)iter->second;
    } else {
        pipelineBinary = (VkPipelineBinaryKHR)0;
    }
    device_dispatch_table.DestroyPipelineBinaryKHR(device, pipelineBinary, pAllocator);
}

VkResult DispatchObject::GetPipelineBinaryDataKHR(VkDevice device, const VkPipelineBinaryDataInfoKHR* pInfo,
                                                  VkPipelineBinaryKeyKHR* pPipelineBinaryKey, size_t* pPipelineBinaryDataSize,
                                                  void* pPipelineBinaryData) {
    if (!wrap_handles)
        return device_dispatch_table.GetPipelineBinaryDataKHR(device, pInfo, pPipelineBinaryKey, pPipelineBinaryDataSize,
                                                              pPipelineBinaryData);
    vku::safe_VkPipelineBinaryDataInfoKHR var_local_pInfo;
    vku::safe_VkPipelineBinaryDataInfoKHR* local_pInfo = nullptr;
    {
        if (pInfo) {
            local_pInfo = &var_local_pInfo;
            local_pInfo->initialize(pInfo);

            if (pInfo->pipelineBinary) {
                local_pInfo->pipelineBinary = Unwrap(pInfo->pipelineBinary);
            }
        }
    }
    VkResult result = device_dispatch_table.GetPipelineBinaryDataKHR(
        device, (const VkPipelineBinaryDataInfoKHR*)local_pInfo, pPipelineBinaryKey, pPipelineBinaryDataSize, pPipelineBinaryData);

    return result;
}

VkResult DispatchObject::ReleaseCapturedPipelineDataKHR(VkDevice device, const VkReleaseCapturedPipelineDataInfoKHR* pInfo,
                                                        const VkAllocationCallbacks* pAllocator) {
    if (!wrap_handles) return device_dispatch_table.ReleaseCapturedPipelineDataKHR(device, pInfo, pAllocator);
    vku::safe_VkReleaseCapturedPipelineDataInfoKHR var_local_pInfo;
    vku::safe_VkReleaseCapturedPipelineDataInfoKHR* local_pInfo = nullptr;
    {
        if (pInfo) {
            local_pInfo = &var_local_pInfo;
            local_pInfo->initialize(pInfo);

            if (pInfo->pipeline) {
                local_pInfo->pipeline = Unwrap(pInfo->pipeline);
            }
        }
    }
    VkResult result = device_dispatch_table.ReleaseCapturedPipelineDataKHR(
        device, (const VkReleaseCapturedPipelineDataInfoKHR*)local_pInfo, pAllocator);

    return result;
}

VkResult DispatchObject::GetPhysicalDeviceCooperativeMatrixPropertiesKHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount,
                                                                         VkCooperativeMatrixPropertiesKHR* pProperties) {
    VkResult result =
        instance_dispatch_table.GetPhysicalDeviceCooperativeMatrixPropertiesKHR(physicalDevice, pPropertyCount, pProperties);

    return result;
}

void DispatchObject::CmdSetLineStippleKHR(VkCommandBuffer commandBuffer, uint32_t lineStippleFactor, uint16_t lineStipplePattern) {
    device_dispatch_table.CmdSetLineStippleKHR(commandBuffer, lineStippleFactor, lineStipplePattern);
}

VkResult DispatchObject::GetPhysicalDeviceCalibrateableTimeDomainsKHR(VkPhysicalDevice physicalDevice, uint32_t* pTimeDomainCount,
                                                                      VkTimeDomainKHR* pTimeDomains) {
    VkResult result =
        instance_dispatch_table.GetPhysicalDeviceCalibrateableTimeDomainsKHR(physicalDevice, pTimeDomainCount, pTimeDomains);

    return result;
}

VkResult DispatchObject::GetCalibratedTimestampsKHR(VkDevice device, uint32_t timestampCount,
                                                    const VkCalibratedTimestampInfoKHR* pTimestampInfos, uint64_t* pTimestamps,
                                                    uint64_t* pMaxDeviation) {
    VkResult result =
        device_dispatch_table.GetCalibratedTimestampsKHR(device, timestampCount, pTimestampInfos, pTimestamps, pMaxDeviation);

    return result;
}

void DispatchObject::CmdBindDescriptorSets2KHR(VkCommandBuffer commandBuffer,
                                               const VkBindDescriptorSetsInfoKHR* pBindDescriptorSetsInfo) {
    if (!wrap_handles) return device_dispatch_table.CmdBindDescriptorSets2KHR(commandBuffer, pBindDescriptorSetsInfo);
    vku::safe_VkBindDescriptorSetsInfoKHR var_local_pBindDescriptorSetsInfo;
    vku::safe_VkBindDescriptorSetsInfoKHR* local_pBindDescriptorSetsInfo = nullptr;
    {
        if (pBindDescriptorSetsInfo) {
            local_pBindDescriptorSetsInfo = &var_local_pBindDescriptorSetsInfo;
            local_pBindDescriptorSetsInfo->initialize(pBindDescriptorSetsInfo);

            if (pBindDescriptorSetsInfo->layout) {
                local_pBindDescriptorSetsInfo->layout = Unwrap(pBindDescriptorSetsInfo->layout);
            }
            if (local_pBindDescriptorSetsInfo->pDescriptorSets) {
                for (uint32_t index1 = 0; index1 < local_pBindDescriptorSetsInfo->descriptorSetCount; ++index1) {
                    local_pBindDescriptorSetsInfo->pDescriptorSets[index1] =
                        Unwrap(local_pBindDescriptorSetsInfo->pDescriptorSets[index1]);
                }
            }
            UnwrapPnextChainHandles(local_pBindDescriptorSetsInfo->pNext);
        }
    }
    device_dispatch_table.CmdBindDescriptorSets2KHR(commandBuffer,
                                                    (const VkBindDescriptorSetsInfoKHR*)local_pBindDescriptorSetsInfo);
}

void DispatchObject::CmdPushConstants2KHR(VkCommandBuffer commandBuffer, const VkPushConstantsInfoKHR* pPushConstantsInfo) {
    if (!wrap_handles) return device_dispatch_table.CmdPushConstants2KHR(commandBuffer, pPushConstantsInfo);
    vku::safe_VkPushConstantsInfoKHR var_local_pPushConstantsInfo;
    vku::safe_VkPushConstantsInfoKHR* local_pPushConstantsInfo = nullptr;
    {
        if (pPushConstantsInfo) {
            local_pPushConstantsInfo = &var_local_pPushConstantsInfo;
            local_pPushConstantsInfo->initialize(pPushConstantsInfo);

            if (pPushConstantsInfo->layout) {
                local_pPushConstantsInfo->layout = Unwrap(pPushConstantsInfo->layout);
            }
            UnwrapPnextChainHandles(local_pPushConstantsInfo->pNext);
        }
    }
    device_dispatch_table.CmdPushConstants2KHR(commandBuffer, (const VkPushConstantsInfoKHR*)local_pPushConstantsInfo);
}

void DispatchObject::CmdPushDescriptorSet2KHR(VkCommandBuffer commandBuffer,
                                              const VkPushDescriptorSetInfoKHR* pPushDescriptorSetInfo) {
    if (!wrap_handles) return device_dispatch_table.CmdPushDescriptorSet2KHR(commandBuffer, pPushDescriptorSetInfo);
    vku::safe_VkPushDescriptorSetInfoKHR var_local_pPushDescriptorSetInfo;
    vku::safe_VkPushDescriptorSetInfoKHR* local_pPushDescriptorSetInfo = nullptr;
    {
        if (pPushDescriptorSetInfo) {
            local_pPushDescriptorSetInfo = &var_local_pPushDescriptorSetInfo;
            local_pPushDescriptorSetInfo->initialize(pPushDescriptorSetInfo);

            if (pPushDescriptorSetInfo->layout) {
                local_pPushDescriptorSetInfo->layout = Unwrap(pPushDescriptorSetInfo->layout);
            }
            if (local_pPushDescriptorSetInfo->pDescriptorWrites) {
                for (uint32_t index1 = 0; index1 < local_pPushDescriptorSetInfo->descriptorWriteCount; ++index1) {
                    UnwrapPnextChainHandles(local_pPushDescriptorSetInfo->pDescriptorWrites[index1].pNext);

                    if (pPushDescriptorSetInfo->pDescriptorWrites[index1].dstSet) {
                        local_pPushDescriptorSetInfo->pDescriptorWrites[index1].dstSet =
                            Unwrap(pPushDescriptorSetInfo->pDescriptorWrites[index1].dstSet);
                    }
                    if (local_pPushDescriptorSetInfo->pDescriptorWrites[index1].pImageInfo) {
                        for (uint32_t index2 = 0; index2 < local_pPushDescriptorSetInfo->pDescriptorWrites[index1].descriptorCount;
                             ++index2) {
                            if (pPushDescriptorSetInfo->pDescriptorWrites[index1].pImageInfo[index2].sampler) {
                                local_pPushDescriptorSetInfo->pDescriptorWrites[index1].pImageInfo[index2].sampler =
                                    Unwrap(pPushDescriptorSetInfo->pDescriptorWrites[index1].pImageInfo[index2].sampler);
                            }
                            if (pPushDescriptorSetInfo->pDescriptorWrites[index1].pImageInfo[index2].imageView) {
                                local_pPushDescriptorSetInfo->pDescriptorWrites[index1].pImageInfo[index2].imageView =
                                    Unwrap(pPushDescriptorSetInfo->pDescriptorWrites[index1].pImageInfo[index2].imageView);
                            }
                        }
                    }
                    if (local_pPushDescriptorSetInfo->pDescriptorWrites[index1].pBufferInfo) {
                        for (uint32_t index2 = 0; index2 < local_pPushDescriptorSetInfo->pDescriptorWrites[index1].descriptorCount;
                             ++index2) {
                            if (pPushDescriptorSetInfo->pDescriptorWrites[index1].pBufferInfo[index2].buffer) {
                                local_pPushDescriptorSetInfo->pDescriptorWrites[index1].pBufferInfo[index2].buffer =
                                    Unwrap(pPushDescriptorSetInfo->pDescriptorWrites[index1].pBufferInfo[index2].buffer);
                            }
                        }
                    }
                    if (local_pPushDescriptorSetInfo->pDescriptorWrites[index1].pTexelBufferView) {
                        for (uint32_t index2 = 0; index2 < local_pPushDescriptorSetInfo->pDescriptorWrites[index1].descriptorCount;
                             ++index2) {
                            local_pPushDescriptorSetInfo->pDescriptorWrites[index1].pTexelBufferView[index2] =
                                Unwrap(local_pPushDescriptorSetInfo->pDescriptorWrites[index1].pTexelBufferView[index2]);
                        }
                    }
                }
            }
            UnwrapPnextChainHandles(local_pPushDescriptorSetInfo->pNext);
        }
    }
    device_dispatch_table.CmdPushDescriptorSet2KHR(commandBuffer, (const VkPushDescriptorSetInfoKHR*)local_pPushDescriptorSetInfo);
}

void DispatchObject::CmdSetDescriptorBufferOffsets2EXT(VkCommandBuffer commandBuffer,
                                                       const VkSetDescriptorBufferOffsetsInfoEXT* pSetDescriptorBufferOffsetsInfo) {
    if (!wrap_handles)
        return device_dispatch_table.CmdSetDescriptorBufferOffsets2EXT(commandBuffer, pSetDescriptorBufferOffsetsInfo);
    vku::safe_VkSetDescriptorBufferOffsetsInfoEXT var_local_pSetDescriptorBufferOffsetsInfo;
    vku::safe_VkSetDescriptorBufferOffsetsInfoEXT* local_pSetDescriptorBufferOffsetsInfo = nullptr;
    {
        if (pSetDescriptorBufferOffsetsInfo) {
            local_pSetDescriptorBufferOffsetsInfo = &var_local_pSetDescriptorBufferOffsetsInfo;
            local_pSetDescriptorBufferOffsetsInfo->initialize(pSetDescriptorBufferOffsetsInfo);

            if (pSetDescriptorBufferOffsetsInfo->layout) {
                local_pSetDescriptorBufferOffsetsInfo->layout = Unwrap(pSetDescriptorBufferOffsetsInfo->layout);
            }
            UnwrapPnextChainHandles(local_pSetDescriptorBufferOffsetsInfo->pNext);
        }
    }
    device_dispatch_table.CmdSetDescriptorBufferOffsets2EXT(
        commandBuffer, (const VkSetDescriptorBufferOffsetsInfoEXT*)local_pSetDescriptorBufferOffsetsInfo);
}

void DispatchObject::CmdBindDescriptorBufferEmbeddedSamplers2EXT(
    VkCommandBuffer commandBuffer, const VkBindDescriptorBufferEmbeddedSamplersInfoEXT* pBindDescriptorBufferEmbeddedSamplersInfo) {
    if (!wrap_handles)
        return device_dispatch_table.CmdBindDescriptorBufferEmbeddedSamplers2EXT(commandBuffer,
                                                                                 pBindDescriptorBufferEmbeddedSamplersInfo);
    vku::safe_VkBindDescriptorBufferEmbeddedSamplersInfoEXT var_local_pBindDescriptorBufferEmbeddedSamplersInfo;
    vku::safe_VkBindDescriptorBufferEmbeddedSamplersInfoEXT* local_pBindDescriptorBufferEmbeddedSamplersInfo = nullptr;
    {
        if (pBindDescriptorBufferEmbeddedSamplersInfo) {
            local_pBindDescriptorBufferEmbeddedSamplersInfo = &var_local_pBindDescriptorBufferEmbeddedSamplersInfo;
            local_pBindDescriptorBufferEmbeddedSamplersInfo->initialize(pBindDescriptorBufferEmbeddedSamplersInfo);

            if (pBindDescriptorBufferEmbeddedSamplersInfo->layout) {
                local_pBindDescriptorBufferEmbeddedSamplersInfo->layout = Unwrap(pBindDescriptorBufferEmbeddedSamplersInfo->layout);
            }
            UnwrapPnextChainHandles(local_pBindDescriptorBufferEmbeddedSamplersInfo->pNext);
        }
    }
    device_dispatch_table.CmdBindDescriptorBufferEmbeddedSamplers2EXT(
        commandBuffer, (const VkBindDescriptorBufferEmbeddedSamplersInfoEXT*)local_pBindDescriptorBufferEmbeddedSamplersInfo);
}

VkResult DispatchObject::CreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo,
                                                      const VkAllocationCallbacks* pAllocator,
                                                      VkDebugReportCallbackEXT* pCallback) {
    if (!wrap_handles) return instance_dispatch_table.CreateDebugReportCallbackEXT(instance, pCreateInfo, pAllocator, pCallback);

    VkResult result = instance_dispatch_table.CreateDebugReportCallbackEXT(instance, pCreateInfo, pAllocator, pCallback);
    if (VK_SUCCESS == result) {
        *pCallback = WrapNew(*pCallback);
    }
    return result;
}

void DispatchObject::DestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback,
                                                   const VkAllocationCallbacks* pAllocator) {
    if (!wrap_handles) return instance_dispatch_table.DestroyDebugReportCallbackEXT(instance, callback, pAllocator);

    uint64_t callback_id = CastToUint64(callback);
    auto iter = unique_id_mapping.pop(callback_id);
    if (iter != unique_id_mapping.end()) {
        callback = (VkDebugReportCallbackEXT)iter->second;
    } else {
        callback = (VkDebugReportCallbackEXT)0;
    }
    instance_dispatch_table.DestroyDebugReportCallbackEXT(instance, callback, pAllocator);
}

void DispatchObject::DebugReportMessageEXT(VkInstance instance, VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType,
                                           uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix,
                                           const char* pMessage) {
    instance_dispatch_table.DebugReportMessageEXT(instance, flags, objectType, object, location, messageCode, pLayerPrefix,
                                                  pMessage);
}

void DispatchObject::CmdDebugMarkerBeginEXT(VkCommandBuffer commandBuffer, const VkDebugMarkerMarkerInfoEXT* pMarkerInfo) {
    device_dispatch_table.CmdDebugMarkerBeginEXT(commandBuffer, pMarkerInfo);
}

void DispatchObject::CmdDebugMarkerEndEXT(VkCommandBuffer commandBuffer) {
    device_dispatch_table.CmdDebugMarkerEndEXT(commandBuffer);
}

void DispatchObject::CmdDebugMarkerInsertEXT(VkCommandBuffer commandBuffer, const VkDebugMarkerMarkerInfoEXT* pMarkerInfo) {
    device_dispatch_table.CmdDebugMarkerInsertEXT(commandBuffer, pMarkerInfo);
}

void DispatchObject::CmdBindTransformFeedbackBuffersEXT(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount,
                                                        const VkBuffer* pBuffers, const VkDeviceSize* pOffsets,
                                                        const VkDeviceSize* pSizes) {
    if (!wrap_handles)
        return device_dispatch_table.CmdBindTransformFeedbackBuffersEXT(commandBuffer, firstBinding, bindingCount, pBuffers,
                                                                        pOffsets, pSizes);
    small_vector<VkBuffer, DISPATCH_MAX_STACK_ALLOCATIONS> var_local_pBuffers;
    VkBuffer* local_pBuffers = nullptr;
    {
        if (pBuffers) {
            var_local_pBuffers.resize(bindingCount);
            local_pBuffers = var_local_pBuffers.data();
            for (uint32_t index0 = 0; index0 < bindingCount; ++index0) {
                local_pBuffers[index0] = Unwrap(pBuffers[index0]);
            }
        }
    }
    device_dispatch_table.CmdBindTransformFeedbackBuffersEXT(commandBuffer, firstBinding, bindingCount,
                                                             (const VkBuffer*)local_pBuffers, pOffsets, pSizes);
}

void DispatchObject::CmdBeginTransformFeedbackEXT(VkCommandBuffer commandBuffer, uint32_t firstCounterBuffer,
                                                  uint32_t counterBufferCount, const VkBuffer* pCounterBuffers,
                                                  const VkDeviceSize* pCounterBufferOffsets) {
    if (!wrap_handles)
        return device_dispatch_table.CmdBeginTransformFeedbackEXT(commandBuffer, firstCounterBuffer, counterBufferCount,
                                                                  pCounterBuffers, pCounterBufferOffsets);
    small_vector<VkBuffer, DISPATCH_MAX_STACK_ALLOCATIONS> var_local_pCounterBuffers;
    VkBuffer* local_pCounterBuffers = nullptr;
    {
        if (pCounterBuffers) {
            var_local_pCounterBuffers.resize(counterBufferCount);
            local_pCounterBuffers = var_local_pCounterBuffers.data();
            for (uint32_t index0 = 0; index0 < counterBufferCount; ++index0) {
                local_pCounterBuffers[index0] = Unwrap(pCounterBuffers[index0]);
            }
        }
    }
    device_dispatch_table.CmdBeginTransformFeedbackEXT(commandBuffer, firstCounterBuffer, counterBufferCount,
                                                       (const VkBuffer*)local_pCounterBuffers, pCounterBufferOffsets);
}

void DispatchObject::CmdEndTransformFeedbackEXT(VkCommandBuffer commandBuffer, uint32_t firstCounterBuffer,
                                                uint32_t counterBufferCount, const VkBuffer* pCounterBuffers,
                                                const VkDeviceSize* pCounterBufferOffsets) {
    if (!wrap_handles)
        return device_dispatch_table.CmdEndTransformFeedbackEXT(commandBuffer, firstCounterBuffer, counterBufferCount,
                                                                pCounterBuffers, pCounterBufferOffsets);
    small_vector<VkBuffer, DISPATCH_MAX_STACK_ALLOCATIONS> var_local_pCounterBuffers;
    VkBuffer* local_pCounterBuffers = nullptr;
    {
        if (pCounterBuffers) {
            var_local_pCounterBuffers.resize(counterBufferCount);
            local_pCounterBuffers = var_local_pCounterBuffers.data();
            for (uint32_t index0 = 0; index0 < counterBufferCount; ++index0) {
                local_pCounterBuffers[index0] = Unwrap(pCounterBuffers[index0]);
            }
        }
    }
    device_dispatch_table.CmdEndTransformFeedbackEXT(commandBuffer, firstCounterBuffer, counterBufferCount,
                                                     (const VkBuffer*)local_pCounterBuffers, pCounterBufferOffsets);
}

void DispatchObject::CmdBeginQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query,
                                             VkQueryControlFlags flags, uint32_t index) {
    if (!wrap_handles) return device_dispatch_table.CmdBeginQueryIndexedEXT(commandBuffer, queryPool, query, flags, index);
    { queryPool = Unwrap(queryPool); }
    device_dispatch_table.CmdBeginQueryIndexedEXT(commandBuffer, queryPool, query, flags, index);
}

void DispatchObject::CmdEndQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query, uint32_t index) {
    if (!wrap_handles) return device_dispatch_table.CmdEndQueryIndexedEXT(commandBuffer, queryPool, query, index);
    { queryPool = Unwrap(queryPool); }
    device_dispatch_table.CmdEndQueryIndexedEXT(commandBuffer, queryPool, query, index);
}

void DispatchObject::CmdDrawIndirectByteCountEXT(VkCommandBuffer commandBuffer, uint32_t instanceCount, uint32_t firstInstance,
                                                 VkBuffer counterBuffer, VkDeviceSize counterBufferOffset, uint32_t counterOffset,
                                                 uint32_t vertexStride) {
    if (!wrap_handles)
        return device_dispatch_table.CmdDrawIndirectByteCountEXT(commandBuffer, instanceCount, firstInstance, counterBuffer,
                                                                 counterBufferOffset, counterOffset, vertexStride);
    { counterBuffer = Unwrap(counterBuffer); }
    device_dispatch_table.CmdDrawIndirectByteCountEXT(commandBuffer, instanceCount, firstInstance, counterBuffer,
                                                      counterBufferOffset, counterOffset, vertexStride);
}

VkResult DispatchObject::CreateCuModuleNVX(VkDevice device, const VkCuModuleCreateInfoNVX* pCreateInfo,
                                           const VkAllocationCallbacks* pAllocator, VkCuModuleNVX* pModule) {
    if (!wrap_handles) return device_dispatch_table.CreateCuModuleNVX(device, pCreateInfo, pAllocator, pModule);

    VkResult result = device_dispatch_table.CreateCuModuleNVX(device, pCreateInfo, pAllocator, pModule);
    if (VK_SUCCESS == result) {
        *pModule = WrapNew(*pModule);
    }
    return result;
}

VkResult DispatchObject::CreateCuFunctionNVX(VkDevice device, const VkCuFunctionCreateInfoNVX* pCreateInfo,
                                             const VkAllocationCallbacks* pAllocator, VkCuFunctionNVX* pFunction) {
    if (!wrap_handles) return device_dispatch_table.CreateCuFunctionNVX(device, pCreateInfo, pAllocator, pFunction);
    vku::safe_VkCuFunctionCreateInfoNVX var_local_pCreateInfo;
    vku::safe_VkCuFunctionCreateInfoNVX* local_pCreateInfo = nullptr;
    {
        if (pCreateInfo) {
            local_pCreateInfo = &var_local_pCreateInfo;
            local_pCreateInfo->initialize(pCreateInfo);

            if (pCreateInfo->module) {
                local_pCreateInfo->module = Unwrap(pCreateInfo->module);
            }
        }
    }
    VkResult result = device_dispatch_table.CreateCuFunctionNVX(device, (const VkCuFunctionCreateInfoNVX*)local_pCreateInfo,
                                                                pAllocator, pFunction);
    if (VK_SUCCESS == result) {
        *pFunction = WrapNew(*pFunction);
    }
    return result;
}

void DispatchObject::DestroyCuModuleNVX(VkDevice device, VkCuModuleNVX module, const VkAllocationCallbacks* pAllocator) {
    if (!wrap_handles) return device_dispatch_table.DestroyCuModuleNVX(device, module, pAllocator);

    uint64_t module_id = CastToUint64(module);
    auto iter = unique_id_mapping.pop(module_id);
    if (iter != unique_id_mapping.end()) {
        module = (VkCuModuleNVX)iter->second;
    } else {
        module = (VkCuModuleNVX)0;
    }
    device_dispatch_table.DestroyCuModuleNVX(device, module, pAllocator);
}

void DispatchObject::DestroyCuFunctionNVX(VkDevice device, VkCuFunctionNVX function, const VkAllocationCallbacks* pAllocator) {
    if (!wrap_handles) return device_dispatch_table.DestroyCuFunctionNVX(device, function, pAllocator);

    uint64_t function_id = CastToUint64(function);
    auto iter = unique_id_mapping.pop(function_id);
    if (iter != unique_id_mapping.end()) {
        function = (VkCuFunctionNVX)iter->second;
    } else {
        function = (VkCuFunctionNVX)0;
    }
    device_dispatch_table.DestroyCuFunctionNVX(device, function, pAllocator);
}

void DispatchObject::CmdCuLaunchKernelNVX(VkCommandBuffer commandBuffer, const VkCuLaunchInfoNVX* pLaunchInfo) {
    if (!wrap_handles) return device_dispatch_table.CmdCuLaunchKernelNVX(commandBuffer, pLaunchInfo);
    vku::safe_VkCuLaunchInfoNVX var_local_pLaunchInfo;
    vku::safe_VkCuLaunchInfoNVX* local_pLaunchInfo = nullptr;
    {
        if (pLaunchInfo) {
            local_pLaunchInfo = &var_local_pLaunchInfo;
            local_pLaunchInfo->initialize(pLaunchInfo);

            if (pLaunchInfo->function) {
                local_pLaunchInfo->function = Unwrap(pLaunchInfo->function);
            }
        }
    }
    device_dispatch_table.CmdCuLaunchKernelNVX(commandBuffer, (const VkCuLaunchInfoNVX*)local_pLaunchInfo);
}

uint32_t DispatchObject::GetImageViewHandleNVX(VkDevice device, const VkImageViewHandleInfoNVX* pInfo) {
    if (!wrap_handles) return device_dispatch_table.GetImageViewHandleNVX(device, pInfo);
    vku::safe_VkImageViewHandleInfoNVX var_local_pInfo;
    vku::safe_VkImageViewHandleInfoNVX* local_pInfo = nullptr;
    {
        if (pInfo) {
            local_pInfo = &var_local_pInfo;
            local_pInfo->initialize(pInfo);

            if (pInfo->imageView) {
                local_pInfo->imageView = Unwrap(pInfo->imageView);
            }
            if (pInfo->sampler) {
                local_pInfo->sampler = Unwrap(pInfo->sampler);
            }
        }
    }
    uint32_t result = device_dispatch_table.GetImageViewHandleNVX(device, (const VkImageViewHandleInfoNVX*)local_pInfo);

    return result;
}

uint64_t DispatchObject::GetImageViewHandle64NVX(VkDevice device, const VkImageViewHandleInfoNVX* pInfo) {
    if (!wrap_handles) return device_dispatch_table.GetImageViewHandle64NVX(device, pInfo);
    vku::safe_VkImageViewHandleInfoNVX var_local_pInfo;
    vku::safe_VkImageViewHandleInfoNVX* local_pInfo = nullptr;
    {
        if (pInfo) {
            local_pInfo = &var_local_pInfo;
            local_pInfo->initialize(pInfo);

            if (pInfo->imageView) {
                local_pInfo->imageView = Unwrap(pInfo->imageView);
            }
            if (pInfo->sampler) {
                local_pInfo->sampler = Unwrap(pInfo->sampler);
            }
        }
    }
    uint64_t result = device_dispatch_table.GetImageViewHandle64NVX(device, (const VkImageViewHandleInfoNVX*)local_pInfo);

    return result;
}

VkResult DispatchObject::GetImageViewAddressNVX(VkDevice device, VkImageView imageView,
                                                VkImageViewAddressPropertiesNVX* pProperties) {
    if (!wrap_handles) return device_dispatch_table.GetImageViewAddressNVX(device, imageView, pProperties);
    { imageView = Unwrap(imageView); }
    VkResult result = device_dispatch_table.GetImageViewAddressNVX(device, imageView, pProperties);

    return result;
}

void DispatchObject::CmdDrawIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                             VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                             uint32_t stride) {
    if (!wrap_handles)
        return device_dispatch_table.CmdDrawIndirectCountAMD(commandBuffer, buffer, offset, countBuffer, countBufferOffset,
                                                             maxDrawCount, stride);
    {
        buffer = Unwrap(buffer);
        countBuffer = Unwrap(countBuffer);
    }
    device_dispatch_table.CmdDrawIndirectCountAMD(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount,
                                                  stride);
}

void DispatchObject::CmdDrawIndexedIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                    VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                    uint32_t stride) {
    if (!wrap_handles)
        return device_dispatch_table.CmdDrawIndexedIndirectCountAMD(commandBuffer, buffer, offset, countBuffer, countBufferOffset,
                                                                    maxDrawCount, stride);
    {
        buffer = Unwrap(buffer);
        countBuffer = Unwrap(countBuffer);
    }
    device_dispatch_table.CmdDrawIndexedIndirectCountAMD(commandBuffer, buffer, offset, countBuffer, countBufferOffset,
                                                         maxDrawCount, stride);
}

VkResult DispatchObject::GetShaderInfoAMD(VkDevice device, VkPipeline pipeline, VkShaderStageFlagBits shaderStage,
                                          VkShaderInfoTypeAMD infoType, size_t* pInfoSize, void* pInfo) {
    if (!wrap_handles) return device_dispatch_table.GetShaderInfoAMD(device, pipeline, shaderStage, infoType, pInfoSize, pInfo);
    { pipeline = Unwrap(pipeline); }
    VkResult result = device_dispatch_table.GetShaderInfoAMD(device, pipeline, shaderStage, infoType, pInfoSize, pInfo);

    return result;
}
#ifdef VK_USE_PLATFORM_GGP

VkResult DispatchObject::CreateStreamDescriptorSurfaceGGP(VkInstance instance,
                                                          const VkStreamDescriptorSurfaceCreateInfoGGP* pCreateInfo,
                                                          const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    if (!wrap_handles) return instance_dispatch_table.CreateStreamDescriptorSurfaceGGP(instance, pCreateInfo, pAllocator, pSurface);

    VkResult result = instance_dispatch_table.CreateStreamDescriptorSurfaceGGP(instance, pCreateInfo, pAllocator, pSurface);
    if (VK_SUCCESS == result) {
        *pSurface = WrapNew(*pSurface);
    }
    return result;
}
#endif  // VK_USE_PLATFORM_GGP

VkResult DispatchObject::GetPhysicalDeviceExternalImageFormatPropertiesNV(
    VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type, VkImageTiling tiling, VkImageUsageFlags usage,
    VkImageCreateFlags flags, VkExternalMemoryHandleTypeFlagsNV externalHandleType,
    VkExternalImageFormatPropertiesNV* pExternalImageFormatProperties) {
    VkResult result = instance_dispatch_table.GetPhysicalDeviceExternalImageFormatPropertiesNV(
        physicalDevice, format, type, tiling, usage, flags, externalHandleType, pExternalImageFormatProperties);

    return result;
}
#ifdef VK_USE_PLATFORM_WIN32_KHR

VkResult DispatchObject::GetMemoryWin32HandleNV(VkDevice device, VkDeviceMemory memory,
                                                VkExternalMemoryHandleTypeFlagsNV handleType, HANDLE* pHandle) {
    if (!wrap_handles) return device_dispatch_table.GetMemoryWin32HandleNV(device, memory, handleType, pHandle);
    { memory = Unwrap(memory); }
    VkResult result = device_dispatch_table.GetMemoryWin32HandleNV(device, memory, handleType, pHandle);

    return result;
}
#endif  // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_VI_NN

VkResult DispatchObject::CreateViSurfaceNN(VkInstance instance, const VkViSurfaceCreateInfoNN* pCreateInfo,
                                           const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    if (!wrap_handles) return instance_dispatch_table.CreateViSurfaceNN(instance, pCreateInfo, pAllocator, pSurface);

    VkResult result = instance_dispatch_table.CreateViSurfaceNN(instance, pCreateInfo, pAllocator, pSurface);
    if (VK_SUCCESS == result) {
        *pSurface = WrapNew(*pSurface);
    }
    return result;
}
#endif  // VK_USE_PLATFORM_VI_NN

void DispatchObject::CmdBeginConditionalRenderingEXT(VkCommandBuffer commandBuffer,
                                                     const VkConditionalRenderingBeginInfoEXT* pConditionalRenderingBegin) {
    if (!wrap_handles) return device_dispatch_table.CmdBeginConditionalRenderingEXT(commandBuffer, pConditionalRenderingBegin);
    vku::safe_VkConditionalRenderingBeginInfoEXT var_local_pConditionalRenderingBegin;
    vku::safe_VkConditionalRenderingBeginInfoEXT* local_pConditionalRenderingBegin = nullptr;
    {
        if (pConditionalRenderingBegin) {
            local_pConditionalRenderingBegin = &var_local_pConditionalRenderingBegin;
            local_pConditionalRenderingBegin->initialize(pConditionalRenderingBegin);

            if (pConditionalRenderingBegin->buffer) {
                local_pConditionalRenderingBegin->buffer = Unwrap(pConditionalRenderingBegin->buffer);
            }
        }
    }
    device_dispatch_table.CmdBeginConditionalRenderingEXT(
        commandBuffer, (const VkConditionalRenderingBeginInfoEXT*)local_pConditionalRenderingBegin);
}

void DispatchObject::CmdEndConditionalRenderingEXT(VkCommandBuffer commandBuffer) {
    device_dispatch_table.CmdEndConditionalRenderingEXT(commandBuffer);
}

void DispatchObject::CmdSetViewportWScalingNV(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount,
                                              const VkViewportWScalingNV* pViewportWScalings) {
    device_dispatch_table.CmdSetViewportWScalingNV(commandBuffer, firstViewport, viewportCount, pViewportWScalings);
}

VkResult DispatchObject::ReleaseDisplayEXT(VkPhysicalDevice physicalDevice, VkDisplayKHR display) {
    if (!wrap_handles) return instance_dispatch_table.ReleaseDisplayEXT(physicalDevice, display);
    { display = Unwrap(display); }
    VkResult result = instance_dispatch_table.ReleaseDisplayEXT(physicalDevice, display);

    return result;
}
#ifdef VK_USE_PLATFORM_XLIB_XRANDR_EXT

VkResult DispatchObject::AcquireXlibDisplayEXT(VkPhysicalDevice physicalDevice, Display* dpy, VkDisplayKHR display) {
    if (!wrap_handles) return instance_dispatch_table.AcquireXlibDisplayEXT(physicalDevice, dpy, display);
    { display = Unwrap(display); }
    VkResult result = instance_dispatch_table.AcquireXlibDisplayEXT(physicalDevice, dpy, display);

    return result;
}

VkResult DispatchObject::GetRandROutputDisplayEXT(VkPhysicalDevice physicalDevice, Display* dpy, RROutput rrOutput,
                                                  VkDisplayKHR* pDisplay) {
    if (!wrap_handles) return instance_dispatch_table.GetRandROutputDisplayEXT(physicalDevice, dpy, rrOutput, pDisplay);

    VkResult result = instance_dispatch_table.GetRandROutputDisplayEXT(physicalDevice, dpy, rrOutput, pDisplay);
    if (VK_SUCCESS == result) {
        *pDisplay = MaybeWrapDisplay(*pDisplay);
    }
    return result;
}
#endif  // VK_USE_PLATFORM_XLIB_XRANDR_EXT

VkResult DispatchObject::GetPhysicalDeviceSurfaceCapabilities2EXT(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                  VkSurfaceCapabilities2EXT* pSurfaceCapabilities) {
    if (!wrap_handles)
        return instance_dispatch_table.GetPhysicalDeviceSurfaceCapabilities2EXT(physicalDevice, surface, pSurfaceCapabilities);
    { surface = Unwrap(surface); }
    VkResult result =
        instance_dispatch_table.GetPhysicalDeviceSurfaceCapabilities2EXT(physicalDevice, surface, pSurfaceCapabilities);

    return result;
}

VkResult DispatchObject::DisplayPowerControlEXT(VkDevice device, VkDisplayKHR display,
                                                const VkDisplayPowerInfoEXT* pDisplayPowerInfo) {
    if (!wrap_handles) return device_dispatch_table.DisplayPowerControlEXT(device, display, pDisplayPowerInfo);
    { display = Unwrap(display); }
    VkResult result = device_dispatch_table.DisplayPowerControlEXT(device, display, pDisplayPowerInfo);

    return result;
}

VkResult DispatchObject::RegisterDeviceEventEXT(VkDevice device, const VkDeviceEventInfoEXT* pDeviceEventInfo,
                                                const VkAllocationCallbacks* pAllocator, VkFence* pFence) {
    if (!wrap_handles) return device_dispatch_table.RegisterDeviceEventEXT(device, pDeviceEventInfo, pAllocator, pFence);

    VkResult result = device_dispatch_table.RegisterDeviceEventEXT(device, pDeviceEventInfo, pAllocator, pFence);
    if (VK_SUCCESS == result) {
        *pFence = WrapNew(*pFence);
    }
    return result;
}

VkResult DispatchObject::RegisterDisplayEventEXT(VkDevice device, VkDisplayKHR display,
                                                 const VkDisplayEventInfoEXT* pDisplayEventInfo,
                                                 const VkAllocationCallbacks* pAllocator, VkFence* pFence) {
    if (!wrap_handles) return device_dispatch_table.RegisterDisplayEventEXT(device, display, pDisplayEventInfo, pAllocator, pFence);
    { display = Unwrap(display); }
    VkResult result = device_dispatch_table.RegisterDisplayEventEXT(device, display, pDisplayEventInfo, pAllocator, pFence);
    if (VK_SUCCESS == result) {
        *pFence = WrapNew(*pFence);
    }
    return result;
}

VkResult DispatchObject::GetSwapchainCounterEXT(VkDevice device, VkSwapchainKHR swapchain, VkSurfaceCounterFlagBitsEXT counter,
                                                uint64_t* pCounterValue) {
    if (!wrap_handles) return device_dispatch_table.GetSwapchainCounterEXT(device, swapchain, counter, pCounterValue);
    { swapchain = Unwrap(swapchain); }
    VkResult result = device_dispatch_table.GetSwapchainCounterEXT(device, swapchain, counter, pCounterValue);

    return result;
}

VkResult DispatchObject::GetRefreshCycleDurationGOOGLE(VkDevice device, VkSwapchainKHR swapchain,
                                                       VkRefreshCycleDurationGOOGLE* pDisplayTimingProperties) {
    if (!wrap_handles) return device_dispatch_table.GetRefreshCycleDurationGOOGLE(device, swapchain, pDisplayTimingProperties);
    { swapchain = Unwrap(swapchain); }
    VkResult result = device_dispatch_table.GetRefreshCycleDurationGOOGLE(device, swapchain, pDisplayTimingProperties);

    return result;
}

VkResult DispatchObject::GetPastPresentationTimingGOOGLE(VkDevice device, VkSwapchainKHR swapchain,
                                                         uint32_t* pPresentationTimingCount,
                                                         VkPastPresentationTimingGOOGLE* pPresentationTimings) {
    if (!wrap_handles)
        return device_dispatch_table.GetPastPresentationTimingGOOGLE(device, swapchain, pPresentationTimingCount,
                                                                     pPresentationTimings);
    { swapchain = Unwrap(swapchain); }
    VkResult result =
        device_dispatch_table.GetPastPresentationTimingGOOGLE(device, swapchain, pPresentationTimingCount, pPresentationTimings);

    return result;
}

void DispatchObject::CmdSetDiscardRectangleEXT(VkCommandBuffer commandBuffer, uint32_t firstDiscardRectangle,
                                               uint32_t discardRectangleCount, const VkRect2D* pDiscardRectangles) {
    device_dispatch_table.CmdSetDiscardRectangleEXT(commandBuffer, firstDiscardRectangle, discardRectangleCount,
                                                    pDiscardRectangles);
}

void DispatchObject::CmdSetDiscardRectangleEnableEXT(VkCommandBuffer commandBuffer, VkBool32 discardRectangleEnable) {
    device_dispatch_table.CmdSetDiscardRectangleEnableEXT(commandBuffer, discardRectangleEnable);
}

void DispatchObject::CmdSetDiscardRectangleModeEXT(VkCommandBuffer commandBuffer, VkDiscardRectangleModeEXT discardRectangleMode) {
    device_dispatch_table.CmdSetDiscardRectangleModeEXT(commandBuffer, discardRectangleMode);
}

void DispatchObject::SetHdrMetadataEXT(VkDevice device, uint32_t swapchainCount, const VkSwapchainKHR* pSwapchains,
                                       const VkHdrMetadataEXT* pMetadata) {
    if (!wrap_handles) return device_dispatch_table.SetHdrMetadataEXT(device, swapchainCount, pSwapchains, pMetadata);
    small_vector<VkSwapchainKHR, DISPATCH_MAX_STACK_ALLOCATIONS> var_local_pSwapchains;
    VkSwapchainKHR* local_pSwapchains = nullptr;
    {
        if (pSwapchains) {
            var_local_pSwapchains.resize(swapchainCount);
            local_pSwapchains = var_local_pSwapchains.data();
            for (uint32_t index0 = 0; index0 < swapchainCount; ++index0) {
                local_pSwapchains[index0] = Unwrap(pSwapchains[index0]);
            }
        }
    }
    device_dispatch_table.SetHdrMetadataEXT(device, swapchainCount, (const VkSwapchainKHR*)local_pSwapchains, pMetadata);
}
#ifdef VK_USE_PLATFORM_IOS_MVK

VkResult DispatchObject::CreateIOSSurfaceMVK(VkInstance instance, const VkIOSSurfaceCreateInfoMVK* pCreateInfo,
                                             const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    if (!wrap_handles) return instance_dispatch_table.CreateIOSSurfaceMVK(instance, pCreateInfo, pAllocator, pSurface);

    VkResult result = instance_dispatch_table.CreateIOSSurfaceMVK(instance, pCreateInfo, pAllocator, pSurface);
    if (VK_SUCCESS == result) {
        *pSurface = WrapNew(*pSurface);
    }
    return result;
}
#endif  // VK_USE_PLATFORM_IOS_MVK
#ifdef VK_USE_PLATFORM_MACOS_MVK

VkResult DispatchObject::CreateMacOSSurfaceMVK(VkInstance instance, const VkMacOSSurfaceCreateInfoMVK* pCreateInfo,
                                               const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    if (!wrap_handles) return instance_dispatch_table.CreateMacOSSurfaceMVK(instance, pCreateInfo, pAllocator, pSurface);

    VkResult result = instance_dispatch_table.CreateMacOSSurfaceMVK(instance, pCreateInfo, pAllocator, pSurface);
    if (VK_SUCCESS == result) {
        *pSurface = WrapNew(*pSurface);
    }
    return result;
}
#endif  // VK_USE_PLATFORM_MACOS_MVK

void DispatchObject::QueueBeginDebugUtilsLabelEXT(VkQueue queue, const VkDebugUtilsLabelEXT* pLabelInfo) {
    device_dispatch_table.QueueBeginDebugUtilsLabelEXT(queue, pLabelInfo);
}

void DispatchObject::QueueEndDebugUtilsLabelEXT(VkQueue queue) { device_dispatch_table.QueueEndDebugUtilsLabelEXT(queue); }

void DispatchObject::QueueInsertDebugUtilsLabelEXT(VkQueue queue, const VkDebugUtilsLabelEXT* pLabelInfo) {
    device_dispatch_table.QueueInsertDebugUtilsLabelEXT(queue, pLabelInfo);
}

void DispatchObject::CmdBeginDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT* pLabelInfo) {
    device_dispatch_table.CmdBeginDebugUtilsLabelEXT(commandBuffer, pLabelInfo);
}

void DispatchObject::CmdEndDebugUtilsLabelEXT(VkCommandBuffer commandBuffer) {
    device_dispatch_table.CmdEndDebugUtilsLabelEXT(commandBuffer);
}

void DispatchObject::CmdInsertDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT* pLabelInfo) {
    device_dispatch_table.CmdInsertDebugUtilsLabelEXT(commandBuffer, pLabelInfo);
}

VkResult DispatchObject::CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                                      const VkAllocationCallbacks* pAllocator,
                                                      VkDebugUtilsMessengerEXT* pMessenger) {
    if (!wrap_handles) return instance_dispatch_table.CreateDebugUtilsMessengerEXT(instance, pCreateInfo, pAllocator, pMessenger);

    VkResult result = instance_dispatch_table.CreateDebugUtilsMessengerEXT(instance, pCreateInfo, pAllocator, pMessenger);
    if (VK_SUCCESS == result) {
        *pMessenger = WrapNew(*pMessenger);
    }
    return result;
}

void DispatchObject::DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT messenger,
                                                   const VkAllocationCallbacks* pAllocator) {
    if (!wrap_handles) return instance_dispatch_table.DestroyDebugUtilsMessengerEXT(instance, messenger, pAllocator);

    uint64_t messenger_id = CastToUint64(messenger);
    auto iter = unique_id_mapping.pop(messenger_id);
    if (iter != unique_id_mapping.end()) {
        messenger = (VkDebugUtilsMessengerEXT)iter->second;
    } else {
        messenger = (VkDebugUtilsMessengerEXT)0;
    }
    instance_dispatch_table.DestroyDebugUtilsMessengerEXT(instance, messenger, pAllocator);
}

void DispatchObject::SubmitDebugUtilsMessageEXT(VkInstance instance, VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                                                const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData) {
    instance_dispatch_table.SubmitDebugUtilsMessageEXT(instance, messageSeverity, messageTypes, pCallbackData);
}
#ifdef VK_USE_PLATFORM_ANDROID_KHR

VkResult DispatchObject::GetAndroidHardwareBufferPropertiesANDROID(VkDevice device, const struct AHardwareBuffer* buffer,
                                                                   VkAndroidHardwareBufferPropertiesANDROID* pProperties) {
    VkResult result = device_dispatch_table.GetAndroidHardwareBufferPropertiesANDROID(device, buffer, pProperties);

    return result;
}

VkResult DispatchObject::GetMemoryAndroidHardwareBufferANDROID(VkDevice device,
                                                               const VkMemoryGetAndroidHardwareBufferInfoANDROID* pInfo,
                                                               struct AHardwareBuffer** pBuffer) {
    if (!wrap_handles) return device_dispatch_table.GetMemoryAndroidHardwareBufferANDROID(device, pInfo, pBuffer);
    vku::safe_VkMemoryGetAndroidHardwareBufferInfoANDROID var_local_pInfo;
    vku::safe_VkMemoryGetAndroidHardwareBufferInfoANDROID* local_pInfo = nullptr;
    {
        if (pInfo) {
            local_pInfo = &var_local_pInfo;
            local_pInfo->initialize(pInfo);

            if (pInfo->memory) {
                local_pInfo->memory = Unwrap(pInfo->memory);
            }
        }
    }
    VkResult result = device_dispatch_table.GetMemoryAndroidHardwareBufferANDROID(
        device, (const VkMemoryGetAndroidHardwareBufferInfoANDROID*)local_pInfo, pBuffer);

    return result;
}
#endif  // VK_USE_PLATFORM_ANDROID_KHR
#ifdef VK_ENABLE_BETA_EXTENSIONS

VkResult DispatchObject::CreateExecutionGraphPipelinesAMDX(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                           const VkExecutionGraphPipelineCreateInfoAMDX* pCreateInfos,
                                                           const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines) {
    if (!wrap_handles)
        return device_dispatch_table.CreateExecutionGraphPipelinesAMDX(device, pipelineCache, createInfoCount, pCreateInfos,
                                                                       pAllocator, pPipelines);
    small_vector<vku::safe_VkExecutionGraphPipelineCreateInfoAMDX, DISPATCH_MAX_STACK_ALLOCATIONS> var_local_pCreateInfos;
    vku::safe_VkExecutionGraphPipelineCreateInfoAMDX* local_pCreateInfos = nullptr;
    {
        pipelineCache = Unwrap(pipelineCache);
        if (pCreateInfos) {
            var_local_pCreateInfos.resize(createInfoCount);
            local_pCreateInfos = var_local_pCreateInfos.data();
            for (uint32_t index0 = 0; index0 < createInfoCount; ++index0) {
                local_pCreateInfos[index0].initialize(&pCreateInfos[index0]);
                if (local_pCreateInfos[index0].pStages) {
                    for (uint32_t index1 = 0; index1 < local_pCreateInfos[index0].stageCount; ++index1) {
                        UnwrapPnextChainHandles(local_pCreateInfos[index0].pStages[index1].pNext);

                        if (pCreateInfos[index0].pStages[index1].module) {
                            local_pCreateInfos[index0].pStages[index1].module = Unwrap(pCreateInfos[index0].pStages[index1].module);
                        }
                    }
                }
                if (local_pCreateInfos[index0].pLibraryInfo) {
                    if (local_pCreateInfos[index0].pLibraryInfo->pLibraries) {
                        for (uint32_t index2 = 0; index2 < local_pCreateInfos[index0].pLibraryInfo->libraryCount; ++index2) {
                            local_pCreateInfos[index0].pLibraryInfo->pLibraries[index2] =
                                Unwrap(local_pCreateInfos[index0].pLibraryInfo->pLibraries[index2]);
                        }
                    }
                }

                if (pCreateInfos[index0].layout) {
                    local_pCreateInfos[index0].layout = Unwrap(pCreateInfos[index0].layout);
                }
                if (pCreateInfos[index0].basePipelineHandle) {
                    local_pCreateInfos[index0].basePipelineHandle = Unwrap(pCreateInfos[index0].basePipelineHandle);
                }
            }
        }
    }
    VkResult result = device_dispatch_table.CreateExecutionGraphPipelinesAMDX(
        device, pipelineCache, createInfoCount, (const VkExecutionGraphPipelineCreateInfoAMDX*)local_pCreateInfos, pAllocator,
        pPipelines);
    if (VK_SUCCESS == result) {
        for (uint32_t index0 = 0; index0 < createInfoCount; index0++) {
            pPipelines[index0] = WrapNew(pPipelines[index0]);
        }
    }
    return result;
}

VkResult DispatchObject::GetExecutionGraphPipelineScratchSizeAMDX(VkDevice device, VkPipeline executionGraph,
                                                                  VkExecutionGraphPipelineScratchSizeAMDX* pSizeInfo) {
    if (!wrap_handles) return device_dispatch_table.GetExecutionGraphPipelineScratchSizeAMDX(device, executionGraph, pSizeInfo);
    { executionGraph = Unwrap(executionGraph); }
    VkResult result = device_dispatch_table.GetExecutionGraphPipelineScratchSizeAMDX(device, executionGraph, pSizeInfo);

    return result;
}

VkResult DispatchObject::GetExecutionGraphPipelineNodeIndexAMDX(VkDevice device, VkPipeline executionGraph,
                                                                const VkPipelineShaderStageNodeCreateInfoAMDX* pNodeInfo,
                                                                uint32_t* pNodeIndex) {
    if (!wrap_handles)
        return device_dispatch_table.GetExecutionGraphPipelineNodeIndexAMDX(device, executionGraph, pNodeInfo, pNodeIndex);
    { executionGraph = Unwrap(executionGraph); }
    VkResult result = device_dispatch_table.GetExecutionGraphPipelineNodeIndexAMDX(device, executionGraph, pNodeInfo, pNodeIndex);

    return result;
}

void DispatchObject::CmdInitializeGraphScratchMemoryAMDX(VkCommandBuffer commandBuffer, VkPipeline executionGraph,
                                                         VkDeviceAddress scratch, VkDeviceSize scratchSize) {
    if (!wrap_handles)
        return device_dispatch_table.CmdInitializeGraphScratchMemoryAMDX(commandBuffer, executionGraph, scratch, scratchSize);
    { executionGraph = Unwrap(executionGraph); }
    device_dispatch_table.CmdInitializeGraphScratchMemoryAMDX(commandBuffer, executionGraph, scratch, scratchSize);
}

void DispatchObject::CmdDispatchGraphAMDX(VkCommandBuffer commandBuffer, VkDeviceAddress scratch, VkDeviceSize scratchSize,
                                          const VkDispatchGraphCountInfoAMDX* pCountInfo) {
    device_dispatch_table.CmdDispatchGraphAMDX(commandBuffer, scratch, scratchSize, pCountInfo);
}

void DispatchObject::CmdDispatchGraphIndirectAMDX(VkCommandBuffer commandBuffer, VkDeviceAddress scratch, VkDeviceSize scratchSize,
                                                  const VkDispatchGraphCountInfoAMDX* pCountInfo) {
    device_dispatch_table.CmdDispatchGraphIndirectAMDX(commandBuffer, scratch, scratchSize, pCountInfo);
}

void DispatchObject::CmdDispatchGraphIndirectCountAMDX(VkCommandBuffer commandBuffer, VkDeviceAddress scratch,
                                                       VkDeviceSize scratchSize, VkDeviceAddress countInfo) {
    device_dispatch_table.CmdDispatchGraphIndirectCountAMDX(commandBuffer, scratch, scratchSize, countInfo);
}
#endif  // VK_ENABLE_BETA_EXTENSIONS

void DispatchObject::CmdSetSampleLocationsEXT(VkCommandBuffer commandBuffer, const VkSampleLocationsInfoEXT* pSampleLocationsInfo) {
    device_dispatch_table.CmdSetSampleLocationsEXT(commandBuffer, pSampleLocationsInfo);
}

void DispatchObject::GetPhysicalDeviceMultisamplePropertiesEXT(VkPhysicalDevice physicalDevice, VkSampleCountFlagBits samples,
                                                               VkMultisamplePropertiesEXT* pMultisampleProperties) {
    instance_dispatch_table.GetPhysicalDeviceMultisamplePropertiesEXT(physicalDevice, samples, pMultisampleProperties);
}

VkResult DispatchObject::GetImageDrmFormatModifierPropertiesEXT(VkDevice device, VkImage image,
                                                                VkImageDrmFormatModifierPropertiesEXT* pProperties) {
    if (!wrap_handles) return device_dispatch_table.GetImageDrmFormatModifierPropertiesEXT(device, image, pProperties);
    { image = Unwrap(image); }
    VkResult result = device_dispatch_table.GetImageDrmFormatModifierPropertiesEXT(device, image, pProperties);

    return result;
}

VkResult DispatchObject::CreateValidationCacheEXT(VkDevice device, const VkValidationCacheCreateInfoEXT* pCreateInfo,
                                                  const VkAllocationCallbacks* pAllocator, VkValidationCacheEXT* pValidationCache) {
    if (!wrap_handles) return device_dispatch_table.CreateValidationCacheEXT(device, pCreateInfo, pAllocator, pValidationCache);

    VkResult result = device_dispatch_table.CreateValidationCacheEXT(device, pCreateInfo, pAllocator, pValidationCache);
    if (VK_SUCCESS == result) {
        *pValidationCache = WrapNew(*pValidationCache);
    }
    return result;
}

void DispatchObject::DestroyValidationCacheEXT(VkDevice device, VkValidationCacheEXT validationCache,
                                               const VkAllocationCallbacks* pAllocator) {
    if (!wrap_handles) return device_dispatch_table.DestroyValidationCacheEXT(device, validationCache, pAllocator);

    uint64_t validationCache_id = CastToUint64(validationCache);
    auto iter = unique_id_mapping.pop(validationCache_id);
    if (iter != unique_id_mapping.end()) {
        validationCache = (VkValidationCacheEXT)iter->second;
    } else {
        validationCache = (VkValidationCacheEXT)0;
    }
    device_dispatch_table.DestroyValidationCacheEXT(device, validationCache, pAllocator);
}

VkResult DispatchObject::MergeValidationCachesEXT(VkDevice device, VkValidationCacheEXT dstCache, uint32_t srcCacheCount,
                                                  const VkValidationCacheEXT* pSrcCaches) {
    if (!wrap_handles) return device_dispatch_table.MergeValidationCachesEXT(device, dstCache, srcCacheCount, pSrcCaches);
    small_vector<VkValidationCacheEXT, DISPATCH_MAX_STACK_ALLOCATIONS> var_local_pSrcCaches;
    VkValidationCacheEXT* local_pSrcCaches = nullptr;
    {
        dstCache = Unwrap(dstCache);
        if (pSrcCaches) {
            var_local_pSrcCaches.resize(srcCacheCount);
            local_pSrcCaches = var_local_pSrcCaches.data();
            for (uint32_t index0 = 0; index0 < srcCacheCount; ++index0) {
                local_pSrcCaches[index0] = Unwrap(pSrcCaches[index0]);
            }
        }
    }
    VkResult result = device_dispatch_table.MergeValidationCachesEXT(device, dstCache, srcCacheCount,
                                                                     (const VkValidationCacheEXT*)local_pSrcCaches);

    return result;
}

VkResult DispatchObject::GetValidationCacheDataEXT(VkDevice device, VkValidationCacheEXT validationCache, size_t* pDataSize,
                                                   void* pData) {
    if (!wrap_handles) return device_dispatch_table.GetValidationCacheDataEXT(device, validationCache, pDataSize, pData);
    { validationCache = Unwrap(validationCache); }
    VkResult result = device_dispatch_table.GetValidationCacheDataEXT(device, validationCache, pDataSize, pData);

    return result;
}

void DispatchObject::CmdBindShadingRateImageNV(VkCommandBuffer commandBuffer, VkImageView imageView, VkImageLayout imageLayout) {
    if (!wrap_handles) return device_dispatch_table.CmdBindShadingRateImageNV(commandBuffer, imageView, imageLayout);
    { imageView = Unwrap(imageView); }
    device_dispatch_table.CmdBindShadingRateImageNV(commandBuffer, imageView, imageLayout);
}

void DispatchObject::CmdSetViewportShadingRatePaletteNV(VkCommandBuffer commandBuffer, uint32_t firstViewport,
                                                        uint32_t viewportCount,
                                                        const VkShadingRatePaletteNV* pShadingRatePalettes) {
    device_dispatch_table.CmdSetViewportShadingRatePaletteNV(commandBuffer, firstViewport, viewportCount, pShadingRatePalettes);
}

void DispatchObject::CmdSetCoarseSampleOrderNV(VkCommandBuffer commandBuffer, VkCoarseSampleOrderTypeNV sampleOrderType,
                                               uint32_t customSampleOrderCount,
                                               const VkCoarseSampleOrderCustomNV* pCustomSampleOrders) {
    device_dispatch_table.CmdSetCoarseSampleOrderNV(commandBuffer, sampleOrderType, customSampleOrderCount, pCustomSampleOrders);
}

VkResult DispatchObject::CreateAccelerationStructureNV(VkDevice device, const VkAccelerationStructureCreateInfoNV* pCreateInfo,
                                                       const VkAllocationCallbacks* pAllocator,
                                                       VkAccelerationStructureNV* pAccelerationStructure) {
    if (!wrap_handles)
        return device_dispatch_table.CreateAccelerationStructureNV(device, pCreateInfo, pAllocator, pAccelerationStructure);
    vku::safe_VkAccelerationStructureCreateInfoNV var_local_pCreateInfo;
    vku::safe_VkAccelerationStructureCreateInfoNV* local_pCreateInfo = nullptr;
    {
        if (pCreateInfo) {
            local_pCreateInfo = &var_local_pCreateInfo;
            local_pCreateInfo->initialize(pCreateInfo);
            if (local_pCreateInfo->info.pGeometries) {
                for (uint32_t index2 = 0; index2 < local_pCreateInfo->info.geometryCount; ++index2) {
                    if (pCreateInfo->info.pGeometries[index2].geometry.triangles.vertexData) {
                        local_pCreateInfo->info.pGeometries[index2].geometry.triangles.vertexData =
                            Unwrap(pCreateInfo->info.pGeometries[index2].geometry.triangles.vertexData);
                    }
                    if (pCreateInfo->info.pGeometries[index2].geometry.triangles.indexData) {
                        local_pCreateInfo->info.pGeometries[index2].geometry.triangles.indexData =
                            Unwrap(pCreateInfo->info.pGeometries[index2].geometry.triangles.indexData);
                    }
                    if (pCreateInfo->info.pGeometries[index2].geometry.triangles.transformData) {
                        local_pCreateInfo->info.pGeometries[index2].geometry.triangles.transformData =
                            Unwrap(pCreateInfo->info.pGeometries[index2].geometry.triangles.transformData);
                    }
                    if (pCreateInfo->info.pGeometries[index2].geometry.aabbs.aabbData) {
                        local_pCreateInfo->info.pGeometries[index2].geometry.aabbs.aabbData =
                            Unwrap(pCreateInfo->info.pGeometries[index2].geometry.aabbs.aabbData);
                    }
                }
            }
        }
    }
    VkResult result = device_dispatch_table.CreateAccelerationStructureNV(
        device, (const VkAccelerationStructureCreateInfoNV*)local_pCreateInfo, pAllocator, pAccelerationStructure);
    if (VK_SUCCESS == result) {
        *pAccelerationStructure = WrapNew(*pAccelerationStructure);
    }
    return result;
}

void DispatchObject::DestroyAccelerationStructureNV(VkDevice device, VkAccelerationStructureNV accelerationStructure,
                                                    const VkAllocationCallbacks* pAllocator) {
    if (!wrap_handles) return device_dispatch_table.DestroyAccelerationStructureNV(device, accelerationStructure, pAllocator);

    uint64_t accelerationStructure_id = CastToUint64(accelerationStructure);
    auto iter = unique_id_mapping.pop(accelerationStructure_id);
    if (iter != unique_id_mapping.end()) {
        accelerationStructure = (VkAccelerationStructureNV)iter->second;
    } else {
        accelerationStructure = (VkAccelerationStructureNV)0;
    }
    device_dispatch_table.DestroyAccelerationStructureNV(device, accelerationStructure, pAllocator);
}

void DispatchObject::GetAccelerationStructureMemoryRequirementsNV(VkDevice device,
                                                                  const VkAccelerationStructureMemoryRequirementsInfoNV* pInfo,
                                                                  VkMemoryRequirements2KHR* pMemoryRequirements) {
    if (!wrap_handles)
        return device_dispatch_table.GetAccelerationStructureMemoryRequirementsNV(device, pInfo, pMemoryRequirements);
    vku::safe_VkAccelerationStructureMemoryRequirementsInfoNV var_local_pInfo;
    vku::safe_VkAccelerationStructureMemoryRequirementsInfoNV* local_pInfo = nullptr;
    {
        if (pInfo) {
            local_pInfo = &var_local_pInfo;
            local_pInfo->initialize(pInfo);

            if (pInfo->accelerationStructure) {
                local_pInfo->accelerationStructure = Unwrap(pInfo->accelerationStructure);
            }
        }
    }
    device_dispatch_table.GetAccelerationStructureMemoryRequirementsNV(
        device, (const VkAccelerationStructureMemoryRequirementsInfoNV*)local_pInfo, pMemoryRequirements);
}

VkResult DispatchObject::BindAccelerationStructureMemoryNV(VkDevice device, uint32_t bindInfoCount,
                                                           const VkBindAccelerationStructureMemoryInfoNV* pBindInfos) {
    if (!wrap_handles) return device_dispatch_table.BindAccelerationStructureMemoryNV(device, bindInfoCount, pBindInfos);
    small_vector<vku::safe_VkBindAccelerationStructureMemoryInfoNV, DISPATCH_MAX_STACK_ALLOCATIONS> var_local_pBindInfos;
    vku::safe_VkBindAccelerationStructureMemoryInfoNV* local_pBindInfos = nullptr;
    {
        if (pBindInfos) {
            var_local_pBindInfos.resize(bindInfoCount);
            local_pBindInfos = var_local_pBindInfos.data();
            for (uint32_t index0 = 0; index0 < bindInfoCount; ++index0) {
                local_pBindInfos[index0].initialize(&pBindInfos[index0]);

                if (pBindInfos[index0].accelerationStructure) {
                    local_pBindInfos[index0].accelerationStructure = Unwrap(pBindInfos[index0].accelerationStructure);
                }
                if (pBindInfos[index0].memory) {
                    local_pBindInfos[index0].memory = Unwrap(pBindInfos[index0].memory);
                }
            }
        }
    }
    VkResult result = device_dispatch_table.BindAccelerationStructureMemoryNV(
        device, bindInfoCount, (const VkBindAccelerationStructureMemoryInfoNV*)local_pBindInfos);

    return result;
}

void DispatchObject::CmdBuildAccelerationStructureNV(VkCommandBuffer commandBuffer, const VkAccelerationStructureInfoNV* pInfo,
                                                     VkBuffer instanceData, VkDeviceSize instanceOffset, VkBool32 update,
                                                     VkAccelerationStructureNV dst, VkAccelerationStructureNV src, VkBuffer scratch,
                                                     VkDeviceSize scratchOffset) {
    if (!wrap_handles)
        return device_dispatch_table.CmdBuildAccelerationStructureNV(commandBuffer, pInfo, instanceData, instanceOffset, update,
                                                                     dst, src, scratch, scratchOffset);
    vku::safe_VkAccelerationStructureInfoNV var_local_pInfo;
    vku::safe_VkAccelerationStructureInfoNV* local_pInfo = nullptr;
    {
        if (pInfo) {
            local_pInfo = &var_local_pInfo;
            local_pInfo->initialize(pInfo);
            if (local_pInfo->pGeometries) {
                for (uint32_t index1 = 0; index1 < local_pInfo->geometryCount; ++index1) {
                    if (pInfo->pGeometries[index1].geometry.triangles.vertexData) {
                        local_pInfo->pGeometries[index1].geometry.triangles.vertexData =
                            Unwrap(pInfo->pGeometries[index1].geometry.triangles.vertexData);
                    }
                    if (pInfo->pGeometries[index1].geometry.triangles.indexData) {
                        local_pInfo->pGeometries[index1].geometry.triangles.indexData =
                            Unwrap(pInfo->pGeometries[index1].geometry.triangles.indexData);
                    }
                    if (pInfo->pGeometries[index1].geometry.triangles.transformData) {
                        local_pInfo->pGeometries[index1].geometry.triangles.transformData =
                            Unwrap(pInfo->pGeometries[index1].geometry.triangles.transformData);
                    }
                    if (pInfo->pGeometries[index1].geometry.aabbs.aabbData) {
                        local_pInfo->pGeometries[index1].geometry.aabbs.aabbData =
                            Unwrap(pInfo->pGeometries[index1].geometry.aabbs.aabbData);
                    }
                }
            }
        }
        instanceData = Unwrap(instanceData);
        dst = Unwrap(dst);
        src = Unwrap(src);
        scratch = Unwrap(scratch);
    }
    device_dispatch_table.CmdBuildAccelerationStructureNV(commandBuffer, (const VkAccelerationStructureInfoNV*)local_pInfo,
                                                          instanceData, instanceOffset, update, dst, src, scratch, scratchOffset);
}

void DispatchObject::CmdCopyAccelerationStructureNV(VkCommandBuffer commandBuffer, VkAccelerationStructureNV dst,
                                                    VkAccelerationStructureNV src, VkCopyAccelerationStructureModeKHR mode) {
    if (!wrap_handles) return device_dispatch_table.CmdCopyAccelerationStructureNV(commandBuffer, dst, src, mode);
    {
        dst = Unwrap(dst);
        src = Unwrap(src);
    }
    device_dispatch_table.CmdCopyAccelerationStructureNV(commandBuffer, dst, src, mode);
}

void DispatchObject::CmdTraceRaysNV(VkCommandBuffer commandBuffer, VkBuffer raygenShaderBindingTableBuffer,
                                    VkDeviceSize raygenShaderBindingOffset, VkBuffer missShaderBindingTableBuffer,
                                    VkDeviceSize missShaderBindingOffset, VkDeviceSize missShaderBindingStride,
                                    VkBuffer hitShaderBindingTableBuffer, VkDeviceSize hitShaderBindingOffset,
                                    VkDeviceSize hitShaderBindingStride, VkBuffer callableShaderBindingTableBuffer,
                                    VkDeviceSize callableShaderBindingOffset, VkDeviceSize callableShaderBindingStride,
                                    uint32_t width, uint32_t height, uint32_t depth) {
    if (!wrap_handles)
        return device_dispatch_table.CmdTraceRaysNV(commandBuffer, raygenShaderBindingTableBuffer, raygenShaderBindingOffset,
                                                    missShaderBindingTableBuffer, missShaderBindingOffset, missShaderBindingStride,
                                                    hitShaderBindingTableBuffer, hitShaderBindingOffset, hitShaderBindingStride,
                                                    callableShaderBindingTableBuffer, callableShaderBindingOffset,
                                                    callableShaderBindingStride, width, height, depth);
    {
        raygenShaderBindingTableBuffer = Unwrap(raygenShaderBindingTableBuffer);
        missShaderBindingTableBuffer = Unwrap(missShaderBindingTableBuffer);
        hitShaderBindingTableBuffer = Unwrap(hitShaderBindingTableBuffer);
        callableShaderBindingTableBuffer = Unwrap(callableShaderBindingTableBuffer);
    }
    device_dispatch_table.CmdTraceRaysNV(commandBuffer, raygenShaderBindingTableBuffer, raygenShaderBindingOffset,
                                         missShaderBindingTableBuffer, missShaderBindingOffset, missShaderBindingStride,
                                         hitShaderBindingTableBuffer, hitShaderBindingOffset, hitShaderBindingStride,
                                         callableShaderBindingTableBuffer, callableShaderBindingOffset, callableShaderBindingStride,
                                         width, height, depth);
}

VkResult DispatchObject::GetRayTracingShaderGroupHandlesKHR(VkDevice device, VkPipeline pipeline, uint32_t firstGroup,
                                                            uint32_t groupCount, size_t dataSize, void* pData) {
    if (!wrap_handles)
        return device_dispatch_table.GetRayTracingShaderGroupHandlesKHR(device, pipeline, firstGroup, groupCount, dataSize, pData);
    { pipeline = Unwrap(pipeline); }
    VkResult result =
        device_dispatch_table.GetRayTracingShaderGroupHandlesKHR(device, pipeline, firstGroup, groupCount, dataSize, pData);

    return result;
}

VkResult DispatchObject::GetRayTracingShaderGroupHandlesNV(VkDevice device, VkPipeline pipeline, uint32_t firstGroup,
                                                           uint32_t groupCount, size_t dataSize, void* pData) {
    if (!wrap_handles)
        return device_dispatch_table.GetRayTracingShaderGroupHandlesNV(device, pipeline, firstGroup, groupCount, dataSize, pData);
    { pipeline = Unwrap(pipeline); }
    VkResult result =
        device_dispatch_table.GetRayTracingShaderGroupHandlesNV(device, pipeline, firstGroup, groupCount, dataSize, pData);

    return result;
}

VkResult DispatchObject::GetAccelerationStructureHandleNV(VkDevice device, VkAccelerationStructureNV accelerationStructure,
                                                          size_t dataSize, void* pData) {
    if (!wrap_handles)
        return device_dispatch_table.GetAccelerationStructureHandleNV(device, accelerationStructure, dataSize, pData);
    { accelerationStructure = Unwrap(accelerationStructure); }
    VkResult result = device_dispatch_table.GetAccelerationStructureHandleNV(device, accelerationStructure, dataSize, pData);

    return result;
}

void DispatchObject::CmdWriteAccelerationStructuresPropertiesNV(VkCommandBuffer commandBuffer, uint32_t accelerationStructureCount,
                                                                const VkAccelerationStructureNV* pAccelerationStructures,
                                                                VkQueryType queryType, VkQueryPool queryPool, uint32_t firstQuery) {
    if (!wrap_handles)
        return device_dispatch_table.CmdWriteAccelerationStructuresPropertiesNV(
            commandBuffer, accelerationStructureCount, pAccelerationStructures, queryType, queryPool, firstQuery);
    small_vector<VkAccelerationStructureNV, DISPATCH_MAX_STACK_ALLOCATIONS> var_local_pAccelerationStructures;
    VkAccelerationStructureNV* local_pAccelerationStructures = nullptr;
    {
        if (pAccelerationStructures) {
            var_local_pAccelerationStructures.resize(accelerationStructureCount);
            local_pAccelerationStructures = var_local_pAccelerationStructures.data();
            for (uint32_t index0 = 0; index0 < accelerationStructureCount; ++index0) {
                local_pAccelerationStructures[index0] = Unwrap(pAccelerationStructures[index0]);
            }
        }
        queryPool = Unwrap(queryPool);
    }
    device_dispatch_table.CmdWriteAccelerationStructuresPropertiesNV(
        commandBuffer, accelerationStructureCount, (const VkAccelerationStructureNV*)local_pAccelerationStructures, queryType,
        queryPool, firstQuery);
}

VkResult DispatchObject::CompileDeferredNV(VkDevice device, VkPipeline pipeline, uint32_t shader) {
    if (!wrap_handles) return device_dispatch_table.CompileDeferredNV(device, pipeline, shader);
    { pipeline = Unwrap(pipeline); }
    VkResult result = device_dispatch_table.CompileDeferredNV(device, pipeline, shader);

    return result;
}

VkResult DispatchObject::GetMemoryHostPointerPropertiesEXT(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType,
                                                           const void* pHostPointer,
                                                           VkMemoryHostPointerPropertiesEXT* pMemoryHostPointerProperties) {
    VkResult result =
        device_dispatch_table.GetMemoryHostPointerPropertiesEXT(device, handleType, pHostPointer, pMemoryHostPointerProperties);

    return result;
}

void DispatchObject::CmdWriteBufferMarkerAMD(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage,
                                             VkBuffer dstBuffer, VkDeviceSize dstOffset, uint32_t marker) {
    if (!wrap_handles)
        return device_dispatch_table.CmdWriteBufferMarkerAMD(commandBuffer, pipelineStage, dstBuffer, dstOffset, marker);
    { dstBuffer = Unwrap(dstBuffer); }
    device_dispatch_table.CmdWriteBufferMarkerAMD(commandBuffer, pipelineStage, dstBuffer, dstOffset, marker);
}

void DispatchObject::CmdWriteBufferMarker2AMD(VkCommandBuffer commandBuffer, VkPipelineStageFlags2 stage, VkBuffer dstBuffer,
                                              VkDeviceSize dstOffset, uint32_t marker) {
    if (!wrap_handles) return device_dispatch_table.CmdWriteBufferMarker2AMD(commandBuffer, stage, dstBuffer, dstOffset, marker);
    { dstBuffer = Unwrap(dstBuffer); }
    device_dispatch_table.CmdWriteBufferMarker2AMD(commandBuffer, stage, dstBuffer, dstOffset, marker);
}

VkResult DispatchObject::GetPhysicalDeviceCalibrateableTimeDomainsEXT(VkPhysicalDevice physicalDevice, uint32_t* pTimeDomainCount,
                                                                      VkTimeDomainKHR* pTimeDomains) {
    VkResult result =
        instance_dispatch_table.GetPhysicalDeviceCalibrateableTimeDomainsEXT(physicalDevice, pTimeDomainCount, pTimeDomains);

    return result;
}

VkResult DispatchObject::GetCalibratedTimestampsEXT(VkDevice device, uint32_t timestampCount,
                                                    const VkCalibratedTimestampInfoKHR* pTimestampInfos, uint64_t* pTimestamps,
                                                    uint64_t* pMaxDeviation) {
    VkResult result =
        device_dispatch_table.GetCalibratedTimestampsEXT(device, timestampCount, pTimestampInfos, pTimestamps, pMaxDeviation);

    return result;
}

void DispatchObject::CmdDrawMeshTasksNV(VkCommandBuffer commandBuffer, uint32_t taskCount, uint32_t firstTask) {
    device_dispatch_table.CmdDrawMeshTasksNV(commandBuffer, taskCount, firstTask);
}

void DispatchObject::CmdDrawMeshTasksIndirectNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                uint32_t drawCount, uint32_t stride) {
    if (!wrap_handles) return device_dispatch_table.CmdDrawMeshTasksIndirectNV(commandBuffer, buffer, offset, drawCount, stride);
    { buffer = Unwrap(buffer); }
    device_dispatch_table.CmdDrawMeshTasksIndirectNV(commandBuffer, buffer, offset, drawCount, stride);
}

void DispatchObject::CmdDrawMeshTasksIndirectCountNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                     VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                     uint32_t stride) {
    if (!wrap_handles)
        return device_dispatch_table.CmdDrawMeshTasksIndirectCountNV(commandBuffer, buffer, offset, countBuffer, countBufferOffset,
                                                                     maxDrawCount, stride);
    {
        buffer = Unwrap(buffer);
        countBuffer = Unwrap(countBuffer);
    }
    device_dispatch_table.CmdDrawMeshTasksIndirectCountNV(commandBuffer, buffer, offset, countBuffer, countBufferOffset,
                                                          maxDrawCount, stride);
}

void DispatchObject::CmdSetExclusiveScissorEnableNV(VkCommandBuffer commandBuffer, uint32_t firstExclusiveScissor,
                                                    uint32_t exclusiveScissorCount, const VkBool32* pExclusiveScissorEnables) {
    device_dispatch_table.CmdSetExclusiveScissorEnableNV(commandBuffer, firstExclusiveScissor, exclusiveScissorCount,
                                                         pExclusiveScissorEnables);
}

void DispatchObject::CmdSetExclusiveScissorNV(VkCommandBuffer commandBuffer, uint32_t firstExclusiveScissor,
                                              uint32_t exclusiveScissorCount, const VkRect2D* pExclusiveScissors) {
    device_dispatch_table.CmdSetExclusiveScissorNV(commandBuffer, firstExclusiveScissor, exclusiveScissorCount, pExclusiveScissors);
}

void DispatchObject::CmdSetCheckpointNV(VkCommandBuffer commandBuffer, const void* pCheckpointMarker) {
    device_dispatch_table.CmdSetCheckpointNV(commandBuffer, pCheckpointMarker);
}

void DispatchObject::GetQueueCheckpointDataNV(VkQueue queue, uint32_t* pCheckpointDataCount, VkCheckpointDataNV* pCheckpointData) {
    device_dispatch_table.GetQueueCheckpointDataNV(queue, pCheckpointDataCount, pCheckpointData);
}

void DispatchObject::GetQueueCheckpointData2NV(VkQueue queue, uint32_t* pCheckpointDataCount,
                                               VkCheckpointData2NV* pCheckpointData) {
    device_dispatch_table.GetQueueCheckpointData2NV(queue, pCheckpointDataCount, pCheckpointData);
}

VkResult DispatchObject::InitializePerformanceApiINTEL(VkDevice device,
                                                       const VkInitializePerformanceApiInfoINTEL* pInitializeInfo) {
    VkResult result = device_dispatch_table.InitializePerformanceApiINTEL(device, pInitializeInfo);

    return result;
}

void DispatchObject::UninitializePerformanceApiINTEL(VkDevice device) {
    device_dispatch_table.UninitializePerformanceApiINTEL(device);
}

VkResult DispatchObject::CmdSetPerformanceMarkerINTEL(VkCommandBuffer commandBuffer,
                                                      const VkPerformanceMarkerInfoINTEL* pMarkerInfo) {
    VkResult result = device_dispatch_table.CmdSetPerformanceMarkerINTEL(commandBuffer, pMarkerInfo);

    return result;
}

VkResult DispatchObject::CmdSetPerformanceStreamMarkerINTEL(VkCommandBuffer commandBuffer,
                                                            const VkPerformanceStreamMarkerInfoINTEL* pMarkerInfo) {
    VkResult result = device_dispatch_table.CmdSetPerformanceStreamMarkerINTEL(commandBuffer, pMarkerInfo);

    return result;
}

VkResult DispatchObject::CmdSetPerformanceOverrideINTEL(VkCommandBuffer commandBuffer,
                                                        const VkPerformanceOverrideInfoINTEL* pOverrideInfo) {
    VkResult result = device_dispatch_table.CmdSetPerformanceOverrideINTEL(commandBuffer, pOverrideInfo);

    return result;
}

VkResult DispatchObject::AcquirePerformanceConfigurationINTEL(VkDevice device,
                                                              const VkPerformanceConfigurationAcquireInfoINTEL* pAcquireInfo,
                                                              VkPerformanceConfigurationINTEL* pConfiguration) {
    if (!wrap_handles) return device_dispatch_table.AcquirePerformanceConfigurationINTEL(device, pAcquireInfo, pConfiguration);

    VkResult result = device_dispatch_table.AcquirePerformanceConfigurationINTEL(device, pAcquireInfo, pConfiguration);
    if (VK_SUCCESS == result) {
        *pConfiguration = WrapNew(*pConfiguration);
    }
    return result;
}

VkResult DispatchObject::QueueSetPerformanceConfigurationINTEL(VkQueue queue, VkPerformanceConfigurationINTEL configuration) {
    if (!wrap_handles) return device_dispatch_table.QueueSetPerformanceConfigurationINTEL(queue, configuration);
    { configuration = Unwrap(configuration); }
    VkResult result = device_dispatch_table.QueueSetPerformanceConfigurationINTEL(queue, configuration);

    return result;
}

VkResult DispatchObject::GetPerformanceParameterINTEL(VkDevice device, VkPerformanceParameterTypeINTEL parameter,
                                                      VkPerformanceValueINTEL* pValue) {
    VkResult result = device_dispatch_table.GetPerformanceParameterINTEL(device, parameter, pValue);

    return result;
}

void DispatchObject::SetLocalDimmingAMD(VkDevice device, VkSwapchainKHR swapChain, VkBool32 localDimmingEnable) {
    if (!wrap_handles) return device_dispatch_table.SetLocalDimmingAMD(device, swapChain, localDimmingEnable);
    { swapChain = Unwrap(swapChain); }
    device_dispatch_table.SetLocalDimmingAMD(device, swapChain, localDimmingEnable);
}
#ifdef VK_USE_PLATFORM_FUCHSIA

VkResult DispatchObject::CreateImagePipeSurfaceFUCHSIA(VkInstance instance, const VkImagePipeSurfaceCreateInfoFUCHSIA* pCreateInfo,
                                                       const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    if (!wrap_handles) return instance_dispatch_table.CreateImagePipeSurfaceFUCHSIA(instance, pCreateInfo, pAllocator, pSurface);

    VkResult result = instance_dispatch_table.CreateImagePipeSurfaceFUCHSIA(instance, pCreateInfo, pAllocator, pSurface);
    if (VK_SUCCESS == result) {
        *pSurface = WrapNew(*pSurface);
    }
    return result;
}
#endif  // VK_USE_PLATFORM_FUCHSIA
#ifdef VK_USE_PLATFORM_METAL_EXT

VkResult DispatchObject::CreateMetalSurfaceEXT(VkInstance instance, const VkMetalSurfaceCreateInfoEXT* pCreateInfo,
                                               const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    if (!wrap_handles) return instance_dispatch_table.CreateMetalSurfaceEXT(instance, pCreateInfo, pAllocator, pSurface);

    VkResult result = instance_dispatch_table.CreateMetalSurfaceEXT(instance, pCreateInfo, pAllocator, pSurface);
    if (VK_SUCCESS == result) {
        *pSurface = WrapNew(*pSurface);
    }
    return result;
}
#endif  // VK_USE_PLATFORM_METAL_EXT

VkDeviceAddress DispatchObject::GetBufferDeviceAddressEXT(VkDevice device, const VkBufferDeviceAddressInfo* pInfo) {
    if (!wrap_handles) return device_dispatch_table.GetBufferDeviceAddressEXT(device, pInfo);
    vku::safe_VkBufferDeviceAddressInfo var_local_pInfo;
    vku::safe_VkBufferDeviceAddressInfo* local_pInfo = nullptr;
    {
        if (pInfo) {
            local_pInfo = &var_local_pInfo;
            local_pInfo->initialize(pInfo);

            if (pInfo->buffer) {
                local_pInfo->buffer = Unwrap(pInfo->buffer);
            }
        }
    }
    VkDeviceAddress result = device_dispatch_table.GetBufferDeviceAddressEXT(device, (const VkBufferDeviceAddressInfo*)local_pInfo);

    return result;
}

VkResult DispatchObject::GetPhysicalDeviceCooperativeMatrixPropertiesNV(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount,
                                                                        VkCooperativeMatrixPropertiesNV* pProperties) {
    VkResult result =
        instance_dispatch_table.GetPhysicalDeviceCooperativeMatrixPropertiesNV(physicalDevice, pPropertyCount, pProperties);

    return result;
}

VkResult DispatchObject::GetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV(
    VkPhysicalDevice physicalDevice, uint32_t* pCombinationCount, VkFramebufferMixedSamplesCombinationNV* pCombinations) {
    VkResult result = instance_dispatch_table.GetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV(
        physicalDevice, pCombinationCount, pCombinations);

    return result;
}
#ifdef VK_USE_PLATFORM_WIN32_KHR

VkResult DispatchObject::GetPhysicalDeviceSurfacePresentModes2EXT(VkPhysicalDevice physicalDevice,
                                                                  const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo,
                                                                  uint32_t* pPresentModeCount, VkPresentModeKHR* pPresentModes) {
    if (!wrap_handles)
        return instance_dispatch_table.GetPhysicalDeviceSurfacePresentModes2EXT(physicalDevice, pSurfaceInfo, pPresentModeCount,
                                                                                pPresentModes);
    vku::safe_VkPhysicalDeviceSurfaceInfo2KHR var_local_pSurfaceInfo;
    vku::safe_VkPhysicalDeviceSurfaceInfo2KHR* local_pSurfaceInfo = nullptr;
    {
        if (pSurfaceInfo) {
            local_pSurfaceInfo = &var_local_pSurfaceInfo;
            local_pSurfaceInfo->initialize(pSurfaceInfo);

            if (pSurfaceInfo->surface) {
                local_pSurfaceInfo->surface = Unwrap(pSurfaceInfo->surface);
            }
        }
    }
    VkResult result = instance_dispatch_table.GetPhysicalDeviceSurfacePresentModes2EXT(
        physicalDevice, (const VkPhysicalDeviceSurfaceInfo2KHR*)local_pSurfaceInfo, pPresentModeCount, pPresentModes);

    return result;
}

VkResult DispatchObject::AcquireFullScreenExclusiveModeEXT(VkDevice device, VkSwapchainKHR swapchain) {
    if (!wrap_handles) return device_dispatch_table.AcquireFullScreenExclusiveModeEXT(device, swapchain);
    { swapchain = Unwrap(swapchain); }
    VkResult result = device_dispatch_table.AcquireFullScreenExclusiveModeEXT(device, swapchain);

    return result;
}

VkResult DispatchObject::ReleaseFullScreenExclusiveModeEXT(VkDevice device, VkSwapchainKHR swapchain) {
    if (!wrap_handles) return device_dispatch_table.ReleaseFullScreenExclusiveModeEXT(device, swapchain);
    { swapchain = Unwrap(swapchain); }
    VkResult result = device_dispatch_table.ReleaseFullScreenExclusiveModeEXT(device, swapchain);

    return result;
}

VkResult DispatchObject::GetDeviceGroupSurfacePresentModes2EXT(VkDevice device, const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo,
                                                               VkDeviceGroupPresentModeFlagsKHR* pModes) {
    if (!wrap_handles) return device_dispatch_table.GetDeviceGroupSurfacePresentModes2EXT(device, pSurfaceInfo, pModes);
    vku::safe_VkPhysicalDeviceSurfaceInfo2KHR var_local_pSurfaceInfo;
    vku::safe_VkPhysicalDeviceSurfaceInfo2KHR* local_pSurfaceInfo = nullptr;
    {
        if (pSurfaceInfo) {
            local_pSurfaceInfo = &var_local_pSurfaceInfo;
            local_pSurfaceInfo->initialize(pSurfaceInfo);

            if (pSurfaceInfo->surface) {
                local_pSurfaceInfo->surface = Unwrap(pSurfaceInfo->surface);
            }
        }
    }
    VkResult result = device_dispatch_table.GetDeviceGroupSurfacePresentModes2EXT(
        device, (const VkPhysicalDeviceSurfaceInfo2KHR*)local_pSurfaceInfo, pModes);

    return result;
}
#endif  // VK_USE_PLATFORM_WIN32_KHR

VkResult DispatchObject::CreateHeadlessSurfaceEXT(VkInstance instance, const VkHeadlessSurfaceCreateInfoEXT* pCreateInfo,
                                                  const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    if (!wrap_handles) return instance_dispatch_table.CreateHeadlessSurfaceEXT(instance, pCreateInfo, pAllocator, pSurface);

    VkResult result = instance_dispatch_table.CreateHeadlessSurfaceEXT(instance, pCreateInfo, pAllocator, pSurface);
    if (VK_SUCCESS == result) {
        *pSurface = WrapNew(*pSurface);
    }
    return result;
}

void DispatchObject::CmdSetLineStippleEXT(VkCommandBuffer commandBuffer, uint32_t lineStippleFactor, uint16_t lineStipplePattern) {
    device_dispatch_table.CmdSetLineStippleEXT(commandBuffer, lineStippleFactor, lineStipplePattern);
}

void DispatchObject::ResetQueryPoolEXT(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount) {
    if (!wrap_handles) return device_dispatch_table.ResetQueryPoolEXT(device, queryPool, firstQuery, queryCount);
    { queryPool = Unwrap(queryPool); }
    device_dispatch_table.ResetQueryPoolEXT(device, queryPool, firstQuery, queryCount);
}

void DispatchObject::CmdSetCullModeEXT(VkCommandBuffer commandBuffer, VkCullModeFlags cullMode) {
    device_dispatch_table.CmdSetCullModeEXT(commandBuffer, cullMode);
}

void DispatchObject::CmdSetFrontFaceEXT(VkCommandBuffer commandBuffer, VkFrontFace frontFace) {
    device_dispatch_table.CmdSetFrontFaceEXT(commandBuffer, frontFace);
}

void DispatchObject::CmdSetPrimitiveTopologyEXT(VkCommandBuffer commandBuffer, VkPrimitiveTopology primitiveTopology) {
    device_dispatch_table.CmdSetPrimitiveTopologyEXT(commandBuffer, primitiveTopology);
}

void DispatchObject::CmdSetViewportWithCountEXT(VkCommandBuffer commandBuffer, uint32_t viewportCount,
                                                const VkViewport* pViewports) {
    device_dispatch_table.CmdSetViewportWithCountEXT(commandBuffer, viewportCount, pViewports);
}

void DispatchObject::CmdSetScissorWithCountEXT(VkCommandBuffer commandBuffer, uint32_t scissorCount, const VkRect2D* pScissors) {
    device_dispatch_table.CmdSetScissorWithCountEXT(commandBuffer, scissorCount, pScissors);
}

void DispatchObject::CmdBindVertexBuffers2EXT(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount,
                                              const VkBuffer* pBuffers, const VkDeviceSize* pOffsets, const VkDeviceSize* pSizes,
                                              const VkDeviceSize* pStrides) {
    if (!wrap_handles)
        return device_dispatch_table.CmdBindVertexBuffers2EXT(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets, pSizes,
                                                              pStrides);
    small_vector<VkBuffer, DISPATCH_MAX_STACK_ALLOCATIONS> var_local_pBuffers;
    VkBuffer* local_pBuffers = nullptr;
    {
        if (pBuffers) {
            var_local_pBuffers.resize(bindingCount);
            local_pBuffers = var_local_pBuffers.data();
            for (uint32_t index0 = 0; index0 < bindingCount; ++index0) {
                local_pBuffers[index0] = Unwrap(pBuffers[index0]);
            }
        }
    }
    device_dispatch_table.CmdBindVertexBuffers2EXT(commandBuffer, firstBinding, bindingCount, (const VkBuffer*)local_pBuffers,
                                                   pOffsets, pSizes, pStrides);
}

void DispatchObject::CmdSetDepthTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthTestEnable) {
    device_dispatch_table.CmdSetDepthTestEnableEXT(commandBuffer, depthTestEnable);
}

void DispatchObject::CmdSetDepthWriteEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthWriteEnable) {
    device_dispatch_table.CmdSetDepthWriteEnableEXT(commandBuffer, depthWriteEnable);
}

void DispatchObject::CmdSetDepthCompareOpEXT(VkCommandBuffer commandBuffer, VkCompareOp depthCompareOp) {
    device_dispatch_table.CmdSetDepthCompareOpEXT(commandBuffer, depthCompareOp);
}

void DispatchObject::CmdSetDepthBoundsTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthBoundsTestEnable) {
    device_dispatch_table.CmdSetDepthBoundsTestEnableEXT(commandBuffer, depthBoundsTestEnable);
}

void DispatchObject::CmdSetStencilTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 stencilTestEnable) {
    device_dispatch_table.CmdSetStencilTestEnableEXT(commandBuffer, stencilTestEnable);
}

void DispatchObject::CmdSetStencilOpEXT(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, VkStencilOp failOp,
                                        VkStencilOp passOp, VkStencilOp depthFailOp, VkCompareOp compareOp) {
    device_dispatch_table.CmdSetStencilOpEXT(commandBuffer, faceMask, failOp, passOp, depthFailOp, compareOp);
}

VkResult DispatchObject::CopyMemoryToImageEXT(VkDevice device, const VkCopyMemoryToImageInfoEXT* pCopyMemoryToImageInfo) {
    if (!wrap_handles) return device_dispatch_table.CopyMemoryToImageEXT(device, pCopyMemoryToImageInfo);
    vku::safe_VkCopyMemoryToImageInfoEXT var_local_pCopyMemoryToImageInfo;
    vku::safe_VkCopyMemoryToImageInfoEXT* local_pCopyMemoryToImageInfo = nullptr;
    {
        if (pCopyMemoryToImageInfo) {
            local_pCopyMemoryToImageInfo = &var_local_pCopyMemoryToImageInfo;
            local_pCopyMemoryToImageInfo->initialize(pCopyMemoryToImageInfo);

            if (pCopyMemoryToImageInfo->dstImage) {
                local_pCopyMemoryToImageInfo->dstImage = Unwrap(pCopyMemoryToImageInfo->dstImage);
            }
        }
    }
    VkResult result =
        device_dispatch_table.CopyMemoryToImageEXT(device, (const VkCopyMemoryToImageInfoEXT*)local_pCopyMemoryToImageInfo);

    return result;
}

VkResult DispatchObject::CopyImageToMemoryEXT(VkDevice device, const VkCopyImageToMemoryInfoEXT* pCopyImageToMemoryInfo) {
    if (!wrap_handles) return device_dispatch_table.CopyImageToMemoryEXT(device, pCopyImageToMemoryInfo);
    vku::safe_VkCopyImageToMemoryInfoEXT var_local_pCopyImageToMemoryInfo;
    vku::safe_VkCopyImageToMemoryInfoEXT* local_pCopyImageToMemoryInfo = nullptr;
    {
        if (pCopyImageToMemoryInfo) {
            local_pCopyImageToMemoryInfo = &var_local_pCopyImageToMemoryInfo;
            local_pCopyImageToMemoryInfo->initialize(pCopyImageToMemoryInfo);

            if (pCopyImageToMemoryInfo->srcImage) {
                local_pCopyImageToMemoryInfo->srcImage = Unwrap(pCopyImageToMemoryInfo->srcImage);
            }
        }
    }
    VkResult result =
        device_dispatch_table.CopyImageToMemoryEXT(device, (const VkCopyImageToMemoryInfoEXT*)local_pCopyImageToMemoryInfo);

    return result;
}

VkResult DispatchObject::CopyImageToImageEXT(VkDevice device, const VkCopyImageToImageInfoEXT* pCopyImageToImageInfo) {
    if (!wrap_handles) return device_dispatch_table.CopyImageToImageEXT(device, pCopyImageToImageInfo);
    vku::safe_VkCopyImageToImageInfoEXT var_local_pCopyImageToImageInfo;
    vku::safe_VkCopyImageToImageInfoEXT* local_pCopyImageToImageInfo = nullptr;
    {
        if (pCopyImageToImageInfo) {
            local_pCopyImageToImageInfo = &var_local_pCopyImageToImageInfo;
            local_pCopyImageToImageInfo->initialize(pCopyImageToImageInfo);

            if (pCopyImageToImageInfo->srcImage) {
                local_pCopyImageToImageInfo->srcImage = Unwrap(pCopyImageToImageInfo->srcImage);
            }
            if (pCopyImageToImageInfo->dstImage) {
                local_pCopyImageToImageInfo->dstImage = Unwrap(pCopyImageToImageInfo->dstImage);
            }
        }
    }
    VkResult result =
        device_dispatch_table.CopyImageToImageEXT(device, (const VkCopyImageToImageInfoEXT*)local_pCopyImageToImageInfo);

    return result;
}

VkResult DispatchObject::TransitionImageLayoutEXT(VkDevice device, uint32_t transitionCount,
                                                  const VkHostImageLayoutTransitionInfoEXT* pTransitions) {
    if (!wrap_handles) return device_dispatch_table.TransitionImageLayoutEXT(device, transitionCount, pTransitions);
    small_vector<vku::safe_VkHostImageLayoutTransitionInfoEXT, DISPATCH_MAX_STACK_ALLOCATIONS> var_local_pTransitions;
    vku::safe_VkHostImageLayoutTransitionInfoEXT* local_pTransitions = nullptr;
    {
        if (pTransitions) {
            var_local_pTransitions.resize(transitionCount);
            local_pTransitions = var_local_pTransitions.data();
            for (uint32_t index0 = 0; index0 < transitionCount; ++index0) {
                local_pTransitions[index0].initialize(&pTransitions[index0]);

                if (pTransitions[index0].image) {
                    local_pTransitions[index0].image = Unwrap(pTransitions[index0].image);
                }
            }
        }
    }
    VkResult result = device_dispatch_table.TransitionImageLayoutEXT(device, transitionCount,
                                                                     (const VkHostImageLayoutTransitionInfoEXT*)local_pTransitions);

    return result;
}

void DispatchObject::GetImageSubresourceLayout2EXT(VkDevice device, VkImage image, const VkImageSubresource2KHR* pSubresource,
                                                   VkSubresourceLayout2KHR* pLayout) {
    if (!wrap_handles) return device_dispatch_table.GetImageSubresourceLayout2EXT(device, image, pSubresource, pLayout);
    { image = Unwrap(image); }
    device_dispatch_table.GetImageSubresourceLayout2EXT(device, image, pSubresource, pLayout);
}

VkResult DispatchObject::ReleaseSwapchainImagesEXT(VkDevice device, const VkReleaseSwapchainImagesInfoEXT* pReleaseInfo) {
    if (!wrap_handles) return device_dispatch_table.ReleaseSwapchainImagesEXT(device, pReleaseInfo);
    vku::safe_VkReleaseSwapchainImagesInfoEXT var_local_pReleaseInfo;
    vku::safe_VkReleaseSwapchainImagesInfoEXT* local_pReleaseInfo = nullptr;
    {
        if (pReleaseInfo) {
            local_pReleaseInfo = &var_local_pReleaseInfo;
            local_pReleaseInfo->initialize(pReleaseInfo);

            if (pReleaseInfo->swapchain) {
                local_pReleaseInfo->swapchain = Unwrap(pReleaseInfo->swapchain);
            }
        }
    }
    VkResult result =
        device_dispatch_table.ReleaseSwapchainImagesEXT(device, (const VkReleaseSwapchainImagesInfoEXT*)local_pReleaseInfo);

    return result;
}

void DispatchObject::GetGeneratedCommandsMemoryRequirementsNV(VkDevice device,
                                                              const VkGeneratedCommandsMemoryRequirementsInfoNV* pInfo,
                                                              VkMemoryRequirements2* pMemoryRequirements) {
    if (!wrap_handles) return device_dispatch_table.GetGeneratedCommandsMemoryRequirementsNV(device, pInfo, pMemoryRequirements);
    vku::safe_VkGeneratedCommandsMemoryRequirementsInfoNV var_local_pInfo;
    vku::safe_VkGeneratedCommandsMemoryRequirementsInfoNV* local_pInfo = nullptr;
    {
        if (pInfo) {
            local_pInfo = &var_local_pInfo;
            local_pInfo->initialize(pInfo);

            if (pInfo->pipeline) {
                local_pInfo->pipeline = Unwrap(pInfo->pipeline);
            }
            if (pInfo->indirectCommandsLayout) {
                local_pInfo->indirectCommandsLayout = Unwrap(pInfo->indirectCommandsLayout);
            }
        }
    }
    device_dispatch_table.GetGeneratedCommandsMemoryRequirementsNV(
        device, (const VkGeneratedCommandsMemoryRequirementsInfoNV*)local_pInfo, pMemoryRequirements);
}

void DispatchObject::CmdPreprocessGeneratedCommandsNV(VkCommandBuffer commandBuffer,
                                                      const VkGeneratedCommandsInfoNV* pGeneratedCommandsInfo) {
    if (!wrap_handles) return device_dispatch_table.CmdPreprocessGeneratedCommandsNV(commandBuffer, pGeneratedCommandsInfo);
    vku::safe_VkGeneratedCommandsInfoNV var_local_pGeneratedCommandsInfo;
    vku::safe_VkGeneratedCommandsInfoNV* local_pGeneratedCommandsInfo = nullptr;
    {
        if (pGeneratedCommandsInfo) {
            local_pGeneratedCommandsInfo = &var_local_pGeneratedCommandsInfo;
            local_pGeneratedCommandsInfo->initialize(pGeneratedCommandsInfo);

            if (pGeneratedCommandsInfo->pipeline) {
                local_pGeneratedCommandsInfo->pipeline = Unwrap(pGeneratedCommandsInfo->pipeline);
            }
            if (pGeneratedCommandsInfo->indirectCommandsLayout) {
                local_pGeneratedCommandsInfo->indirectCommandsLayout = Unwrap(pGeneratedCommandsInfo->indirectCommandsLayout);
            }
            if (local_pGeneratedCommandsInfo->pStreams) {
                for (uint32_t index1 = 0; index1 < local_pGeneratedCommandsInfo->streamCount; ++index1) {
                    if (pGeneratedCommandsInfo->pStreams[index1].buffer) {
                        local_pGeneratedCommandsInfo->pStreams[index1].buffer =
                            Unwrap(pGeneratedCommandsInfo->pStreams[index1].buffer);
                    }
                }
            }

            if (pGeneratedCommandsInfo->preprocessBuffer) {
                local_pGeneratedCommandsInfo->preprocessBuffer = Unwrap(pGeneratedCommandsInfo->preprocessBuffer);
            }
            if (pGeneratedCommandsInfo->sequencesCountBuffer) {
                local_pGeneratedCommandsInfo->sequencesCountBuffer = Unwrap(pGeneratedCommandsInfo->sequencesCountBuffer);
            }
            if (pGeneratedCommandsInfo->sequencesIndexBuffer) {
                local_pGeneratedCommandsInfo->sequencesIndexBuffer = Unwrap(pGeneratedCommandsInfo->sequencesIndexBuffer);
            }
        }
    }
    device_dispatch_table.CmdPreprocessGeneratedCommandsNV(commandBuffer,
                                                           (const VkGeneratedCommandsInfoNV*)local_pGeneratedCommandsInfo);
}

void DispatchObject::CmdExecuteGeneratedCommandsNV(VkCommandBuffer commandBuffer, VkBool32 isPreprocessed,
                                                   const VkGeneratedCommandsInfoNV* pGeneratedCommandsInfo) {
    if (!wrap_handles)
        return device_dispatch_table.CmdExecuteGeneratedCommandsNV(commandBuffer, isPreprocessed, pGeneratedCommandsInfo);
    vku::safe_VkGeneratedCommandsInfoNV var_local_pGeneratedCommandsInfo;
    vku::safe_VkGeneratedCommandsInfoNV* local_pGeneratedCommandsInfo = nullptr;
    {
        if (pGeneratedCommandsInfo) {
            local_pGeneratedCommandsInfo = &var_local_pGeneratedCommandsInfo;
            local_pGeneratedCommandsInfo->initialize(pGeneratedCommandsInfo);

            if (pGeneratedCommandsInfo->pipeline) {
                local_pGeneratedCommandsInfo->pipeline = Unwrap(pGeneratedCommandsInfo->pipeline);
            }
            if (pGeneratedCommandsInfo->indirectCommandsLayout) {
                local_pGeneratedCommandsInfo->indirectCommandsLayout = Unwrap(pGeneratedCommandsInfo->indirectCommandsLayout);
            }
            if (local_pGeneratedCommandsInfo->pStreams) {
                for (uint32_t index1 = 0; index1 < local_pGeneratedCommandsInfo->streamCount; ++index1) {
                    if (pGeneratedCommandsInfo->pStreams[index1].buffer) {
                        local_pGeneratedCommandsInfo->pStreams[index1].buffer =
                            Unwrap(pGeneratedCommandsInfo->pStreams[index1].buffer);
                    }
                }
            }

            if (pGeneratedCommandsInfo->preprocessBuffer) {
                local_pGeneratedCommandsInfo->preprocessBuffer = Unwrap(pGeneratedCommandsInfo->preprocessBuffer);
            }
            if (pGeneratedCommandsInfo->sequencesCountBuffer) {
                local_pGeneratedCommandsInfo->sequencesCountBuffer = Unwrap(pGeneratedCommandsInfo->sequencesCountBuffer);
            }
            if (pGeneratedCommandsInfo->sequencesIndexBuffer) {
                local_pGeneratedCommandsInfo->sequencesIndexBuffer = Unwrap(pGeneratedCommandsInfo->sequencesIndexBuffer);
            }
        }
    }
    device_dispatch_table.CmdExecuteGeneratedCommandsNV(commandBuffer, isPreprocessed,
                                                        (const VkGeneratedCommandsInfoNV*)local_pGeneratedCommandsInfo);
}

void DispatchObject::CmdBindPipelineShaderGroupNV(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                  VkPipeline pipeline, uint32_t groupIndex) {
    if (!wrap_handles)
        return device_dispatch_table.CmdBindPipelineShaderGroupNV(commandBuffer, pipelineBindPoint, pipeline, groupIndex);
    { pipeline = Unwrap(pipeline); }
    device_dispatch_table.CmdBindPipelineShaderGroupNV(commandBuffer, pipelineBindPoint, pipeline, groupIndex);
}

VkResult DispatchObject::CreateIndirectCommandsLayoutNV(VkDevice device, const VkIndirectCommandsLayoutCreateInfoNV* pCreateInfo,
                                                        const VkAllocationCallbacks* pAllocator,
                                                        VkIndirectCommandsLayoutNV* pIndirectCommandsLayout) {
    if (!wrap_handles)
        return device_dispatch_table.CreateIndirectCommandsLayoutNV(device, pCreateInfo, pAllocator, pIndirectCommandsLayout);
    vku::safe_VkIndirectCommandsLayoutCreateInfoNV var_local_pCreateInfo;
    vku::safe_VkIndirectCommandsLayoutCreateInfoNV* local_pCreateInfo = nullptr;
    {
        if (pCreateInfo) {
            local_pCreateInfo = &var_local_pCreateInfo;
            local_pCreateInfo->initialize(pCreateInfo);
            if (local_pCreateInfo->pTokens) {
                for (uint32_t index1 = 0; index1 < local_pCreateInfo->tokenCount; ++index1) {
                    if (pCreateInfo->pTokens[index1].pushconstantPipelineLayout) {
                        local_pCreateInfo->pTokens[index1].pushconstantPipelineLayout =
                            Unwrap(pCreateInfo->pTokens[index1].pushconstantPipelineLayout);
                    }
                }
            }
        }
    }
    VkResult result = device_dispatch_table.CreateIndirectCommandsLayoutNV(
        device, (const VkIndirectCommandsLayoutCreateInfoNV*)local_pCreateInfo, pAllocator, pIndirectCommandsLayout);
    if (VK_SUCCESS == result) {
        *pIndirectCommandsLayout = WrapNew(*pIndirectCommandsLayout);
    }
    return result;
}

void DispatchObject::DestroyIndirectCommandsLayoutNV(VkDevice device, VkIndirectCommandsLayoutNV indirectCommandsLayout,
                                                     const VkAllocationCallbacks* pAllocator) {
    if (!wrap_handles) return device_dispatch_table.DestroyIndirectCommandsLayoutNV(device, indirectCommandsLayout, pAllocator);

    uint64_t indirectCommandsLayout_id = CastToUint64(indirectCommandsLayout);
    auto iter = unique_id_mapping.pop(indirectCommandsLayout_id);
    if (iter != unique_id_mapping.end()) {
        indirectCommandsLayout = (VkIndirectCommandsLayoutNV)iter->second;
    } else {
        indirectCommandsLayout = (VkIndirectCommandsLayoutNV)0;
    }
    device_dispatch_table.DestroyIndirectCommandsLayoutNV(device, indirectCommandsLayout, pAllocator);
}

void DispatchObject::CmdSetDepthBias2EXT(VkCommandBuffer commandBuffer, const VkDepthBiasInfoEXT* pDepthBiasInfo) {
    device_dispatch_table.CmdSetDepthBias2EXT(commandBuffer, pDepthBiasInfo);
}

VkResult DispatchObject::AcquireDrmDisplayEXT(VkPhysicalDevice physicalDevice, int32_t drmFd, VkDisplayKHR display) {
    if (!wrap_handles) return instance_dispatch_table.AcquireDrmDisplayEXT(physicalDevice, drmFd, display);
    { display = Unwrap(display); }
    VkResult result = instance_dispatch_table.AcquireDrmDisplayEXT(physicalDevice, drmFd, display);

    return result;
}

VkResult DispatchObject::GetDrmDisplayEXT(VkPhysicalDevice physicalDevice, int32_t drmFd, uint32_t connectorId,
                                          VkDisplayKHR* display) {
    if (!wrap_handles) return instance_dispatch_table.GetDrmDisplayEXT(physicalDevice, drmFd, connectorId, display);

    VkResult result = instance_dispatch_table.GetDrmDisplayEXT(physicalDevice, drmFd, connectorId, display);
    if (VK_SUCCESS == result) {
        *display = MaybeWrapDisplay(*display);
    }
    return result;
}

VkResult DispatchObject::CreatePrivateDataSlotEXT(VkDevice device, const VkPrivateDataSlotCreateInfo* pCreateInfo,
                                                  const VkAllocationCallbacks* pAllocator, VkPrivateDataSlot* pPrivateDataSlot) {
    if (!wrap_handles) return device_dispatch_table.CreatePrivateDataSlotEXT(device, pCreateInfo, pAllocator, pPrivateDataSlot);

    VkResult result = device_dispatch_table.CreatePrivateDataSlotEXT(device, pCreateInfo, pAllocator, pPrivateDataSlot);
    if (VK_SUCCESS == result) {
        *pPrivateDataSlot = WrapNew(*pPrivateDataSlot);
    }
    return result;
}

void DispatchObject::DestroyPrivateDataSlotEXT(VkDevice device, VkPrivateDataSlot privateDataSlot,
                                               const VkAllocationCallbacks* pAllocator) {
    if (!wrap_handles) return device_dispatch_table.DestroyPrivateDataSlotEXT(device, privateDataSlot, pAllocator);

    uint64_t privateDataSlot_id = CastToUint64(privateDataSlot);
    auto iter = unique_id_mapping.pop(privateDataSlot_id);
    if (iter != unique_id_mapping.end()) {
        privateDataSlot = (VkPrivateDataSlot)iter->second;
    } else {
        privateDataSlot = (VkPrivateDataSlot)0;
    }
    device_dispatch_table.DestroyPrivateDataSlotEXT(device, privateDataSlot, pAllocator);
}

VkResult DispatchObject::SetPrivateDataEXT(VkDevice device, VkObjectType objectType, uint64_t objectHandle,
                                           VkPrivateDataSlot privateDataSlot, uint64_t data) {
    if (!wrap_handles) return device_dispatch_table.SetPrivateDataEXT(device, objectType, objectHandle, privateDataSlot, data);
    {
        if (NotDispatchableHandle(objectType)) {
            objectHandle = Unwrap(objectHandle);
        }
        privateDataSlot = Unwrap(privateDataSlot);
    }
    VkResult result = device_dispatch_table.SetPrivateDataEXT(device, objectType, objectHandle, privateDataSlot, data);

    return result;
}

void DispatchObject::GetPrivateDataEXT(VkDevice device, VkObjectType objectType, uint64_t objectHandle,
                                       VkPrivateDataSlot privateDataSlot, uint64_t* pData) {
    if (!wrap_handles) return device_dispatch_table.GetPrivateDataEXT(device, objectType, objectHandle, privateDataSlot, pData);
    {
        if (NotDispatchableHandle(objectType)) {
            objectHandle = Unwrap(objectHandle);
        }
        privateDataSlot = Unwrap(privateDataSlot);
    }
    device_dispatch_table.GetPrivateDataEXT(device, objectType, objectHandle, privateDataSlot, pData);
}

VkResult DispatchObject::CreateCudaModuleNV(VkDevice device, const VkCudaModuleCreateInfoNV* pCreateInfo,
                                            const VkAllocationCallbacks* pAllocator, VkCudaModuleNV* pModule) {
    if (!wrap_handles) return device_dispatch_table.CreateCudaModuleNV(device, pCreateInfo, pAllocator, pModule);

    VkResult result = device_dispatch_table.CreateCudaModuleNV(device, pCreateInfo, pAllocator, pModule);
    if (VK_SUCCESS == result) {
        *pModule = WrapNew(*pModule);
    }
    return result;
}

VkResult DispatchObject::GetCudaModuleCacheNV(VkDevice device, VkCudaModuleNV module, size_t* pCacheSize, void* pCacheData) {
    if (!wrap_handles) return device_dispatch_table.GetCudaModuleCacheNV(device, module, pCacheSize, pCacheData);
    { module = Unwrap(module); }
    VkResult result = device_dispatch_table.GetCudaModuleCacheNV(device, module, pCacheSize, pCacheData);

    return result;
}

VkResult DispatchObject::CreateCudaFunctionNV(VkDevice device, const VkCudaFunctionCreateInfoNV* pCreateInfo,
                                              const VkAllocationCallbacks* pAllocator, VkCudaFunctionNV* pFunction) {
    if (!wrap_handles) return device_dispatch_table.CreateCudaFunctionNV(device, pCreateInfo, pAllocator, pFunction);
    vku::safe_VkCudaFunctionCreateInfoNV var_local_pCreateInfo;
    vku::safe_VkCudaFunctionCreateInfoNV* local_pCreateInfo = nullptr;
    {
        if (pCreateInfo) {
            local_pCreateInfo = &var_local_pCreateInfo;
            local_pCreateInfo->initialize(pCreateInfo);

            if (pCreateInfo->module) {
                local_pCreateInfo->module = Unwrap(pCreateInfo->module);
            }
        }
    }
    VkResult result = device_dispatch_table.CreateCudaFunctionNV(device, (const VkCudaFunctionCreateInfoNV*)local_pCreateInfo,
                                                                 pAllocator, pFunction);
    if (VK_SUCCESS == result) {
        *pFunction = WrapNew(*pFunction);
    }
    return result;
}

void DispatchObject::DestroyCudaModuleNV(VkDevice device, VkCudaModuleNV module, const VkAllocationCallbacks* pAllocator) {
    if (!wrap_handles) return device_dispatch_table.DestroyCudaModuleNV(device, module, pAllocator);

    uint64_t module_id = CastToUint64(module);
    auto iter = unique_id_mapping.pop(module_id);
    if (iter != unique_id_mapping.end()) {
        module = (VkCudaModuleNV)iter->second;
    } else {
        module = (VkCudaModuleNV)0;
    }
    device_dispatch_table.DestroyCudaModuleNV(device, module, pAllocator);
}

void DispatchObject::DestroyCudaFunctionNV(VkDevice device, VkCudaFunctionNV function, const VkAllocationCallbacks* pAllocator) {
    if (!wrap_handles) return device_dispatch_table.DestroyCudaFunctionNV(device, function, pAllocator);

    uint64_t function_id = CastToUint64(function);
    auto iter = unique_id_mapping.pop(function_id);
    if (iter != unique_id_mapping.end()) {
        function = (VkCudaFunctionNV)iter->second;
    } else {
        function = (VkCudaFunctionNV)0;
    }
    device_dispatch_table.DestroyCudaFunctionNV(device, function, pAllocator);
}

void DispatchObject::CmdCudaLaunchKernelNV(VkCommandBuffer commandBuffer, const VkCudaLaunchInfoNV* pLaunchInfo) {
    if (!wrap_handles) return device_dispatch_table.CmdCudaLaunchKernelNV(commandBuffer, pLaunchInfo);
    vku::safe_VkCudaLaunchInfoNV var_local_pLaunchInfo;
    vku::safe_VkCudaLaunchInfoNV* local_pLaunchInfo = nullptr;
    {
        if (pLaunchInfo) {
            local_pLaunchInfo = &var_local_pLaunchInfo;
            local_pLaunchInfo->initialize(pLaunchInfo);

            if (pLaunchInfo->function) {
                local_pLaunchInfo->function = Unwrap(pLaunchInfo->function);
            }
        }
    }
    device_dispatch_table.CmdCudaLaunchKernelNV(commandBuffer, (const VkCudaLaunchInfoNV*)local_pLaunchInfo);
}

void DispatchObject::GetDescriptorSetLayoutSizeEXT(VkDevice device, VkDescriptorSetLayout layout,
                                                   VkDeviceSize* pLayoutSizeInBytes) {
    if (!wrap_handles) return device_dispatch_table.GetDescriptorSetLayoutSizeEXT(device, layout, pLayoutSizeInBytes);
    { layout = Unwrap(layout); }
    device_dispatch_table.GetDescriptorSetLayoutSizeEXT(device, layout, pLayoutSizeInBytes);
}

void DispatchObject::GetDescriptorSetLayoutBindingOffsetEXT(VkDevice device, VkDescriptorSetLayout layout, uint32_t binding,
                                                            VkDeviceSize* pOffset) {
    if (!wrap_handles) return device_dispatch_table.GetDescriptorSetLayoutBindingOffsetEXT(device, layout, binding, pOffset);
    { layout = Unwrap(layout); }
    device_dispatch_table.GetDescriptorSetLayoutBindingOffsetEXT(device, layout, binding, pOffset);
}

void DispatchObject::CmdBindDescriptorBuffersEXT(VkCommandBuffer commandBuffer, uint32_t bufferCount,
                                                 const VkDescriptorBufferBindingInfoEXT* pBindingInfos) {
    if (!wrap_handles) return device_dispatch_table.CmdBindDescriptorBuffersEXT(commandBuffer, bufferCount, pBindingInfos);
    small_vector<vku::safe_VkDescriptorBufferBindingInfoEXT, DISPATCH_MAX_STACK_ALLOCATIONS> var_local_pBindingInfos;
    vku::safe_VkDescriptorBufferBindingInfoEXT* local_pBindingInfos = nullptr;
    {
        if (pBindingInfos) {
            var_local_pBindingInfos.resize(bufferCount);
            local_pBindingInfos = var_local_pBindingInfos.data();
            for (uint32_t index0 = 0; index0 < bufferCount; ++index0) {
                local_pBindingInfos[index0].initialize(&pBindingInfos[index0]);
                UnwrapPnextChainHandles(local_pBindingInfos[index0].pNext);
            }
        }
    }
    device_dispatch_table.CmdBindDescriptorBuffersEXT(commandBuffer, bufferCount,
                                                      (const VkDescriptorBufferBindingInfoEXT*)local_pBindingInfos);
}

void DispatchObject::CmdSetDescriptorBufferOffsetsEXT(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                      VkPipelineLayout layout, uint32_t firstSet, uint32_t setCount,
                                                      const uint32_t* pBufferIndices, const VkDeviceSize* pOffsets) {
    if (!wrap_handles)
        return device_dispatch_table.CmdSetDescriptorBufferOffsetsEXT(commandBuffer, pipelineBindPoint, layout, firstSet, setCount,
                                                                      pBufferIndices, pOffsets);
    { layout = Unwrap(layout); }
    device_dispatch_table.CmdSetDescriptorBufferOffsetsEXT(commandBuffer, pipelineBindPoint, layout, firstSet, setCount,
                                                           pBufferIndices, pOffsets);
}

void DispatchObject::CmdBindDescriptorBufferEmbeddedSamplersEXT(VkCommandBuffer commandBuffer,
                                                                VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout,
                                                                uint32_t set) {
    if (!wrap_handles)
        return device_dispatch_table.CmdBindDescriptorBufferEmbeddedSamplersEXT(commandBuffer, pipelineBindPoint, layout, set);
    { layout = Unwrap(layout); }
    device_dispatch_table.CmdBindDescriptorBufferEmbeddedSamplersEXT(commandBuffer, pipelineBindPoint, layout, set);
}

VkResult DispatchObject::GetBufferOpaqueCaptureDescriptorDataEXT(VkDevice device, const VkBufferCaptureDescriptorDataInfoEXT* pInfo,
                                                                 void* pData) {
    if (!wrap_handles) return device_dispatch_table.GetBufferOpaqueCaptureDescriptorDataEXT(device, pInfo, pData);
    vku::safe_VkBufferCaptureDescriptorDataInfoEXT var_local_pInfo;
    vku::safe_VkBufferCaptureDescriptorDataInfoEXT* local_pInfo = nullptr;
    {
        if (pInfo) {
            local_pInfo = &var_local_pInfo;
            local_pInfo->initialize(pInfo);

            if (pInfo->buffer) {
                local_pInfo->buffer = Unwrap(pInfo->buffer);
            }
        }
    }
    VkResult result = device_dispatch_table.GetBufferOpaqueCaptureDescriptorDataEXT(
        device, (const VkBufferCaptureDescriptorDataInfoEXT*)local_pInfo, pData);

    return result;
}

VkResult DispatchObject::GetImageOpaqueCaptureDescriptorDataEXT(VkDevice device, const VkImageCaptureDescriptorDataInfoEXT* pInfo,
                                                                void* pData) {
    if (!wrap_handles) return device_dispatch_table.GetImageOpaqueCaptureDescriptorDataEXT(device, pInfo, pData);
    vku::safe_VkImageCaptureDescriptorDataInfoEXT var_local_pInfo;
    vku::safe_VkImageCaptureDescriptorDataInfoEXT* local_pInfo = nullptr;
    {
        if (pInfo) {
            local_pInfo = &var_local_pInfo;
            local_pInfo->initialize(pInfo);

            if (pInfo->image) {
                local_pInfo->image = Unwrap(pInfo->image);
            }
        }
    }
    VkResult result = device_dispatch_table.GetImageOpaqueCaptureDescriptorDataEXT(
        device, (const VkImageCaptureDescriptorDataInfoEXT*)local_pInfo, pData);

    return result;
}

VkResult DispatchObject::GetImageViewOpaqueCaptureDescriptorDataEXT(VkDevice device,
                                                                    const VkImageViewCaptureDescriptorDataInfoEXT* pInfo,
                                                                    void* pData) {
    if (!wrap_handles) return device_dispatch_table.GetImageViewOpaqueCaptureDescriptorDataEXT(device, pInfo, pData);
    vku::safe_VkImageViewCaptureDescriptorDataInfoEXT var_local_pInfo;
    vku::safe_VkImageViewCaptureDescriptorDataInfoEXT* local_pInfo = nullptr;
    {
        if (pInfo) {
            local_pInfo = &var_local_pInfo;
            local_pInfo->initialize(pInfo);

            if (pInfo->imageView) {
                local_pInfo->imageView = Unwrap(pInfo->imageView);
            }
        }
    }
    VkResult result = device_dispatch_table.GetImageViewOpaqueCaptureDescriptorDataEXT(
        device, (const VkImageViewCaptureDescriptorDataInfoEXT*)local_pInfo, pData);

    return result;
}

VkResult DispatchObject::GetSamplerOpaqueCaptureDescriptorDataEXT(VkDevice device,
                                                                  const VkSamplerCaptureDescriptorDataInfoEXT* pInfo, void* pData) {
    if (!wrap_handles) return device_dispatch_table.GetSamplerOpaqueCaptureDescriptorDataEXT(device, pInfo, pData);
    vku::safe_VkSamplerCaptureDescriptorDataInfoEXT var_local_pInfo;
    vku::safe_VkSamplerCaptureDescriptorDataInfoEXT* local_pInfo = nullptr;
    {
        if (pInfo) {
            local_pInfo = &var_local_pInfo;
            local_pInfo->initialize(pInfo);

            if (pInfo->sampler) {
                local_pInfo->sampler = Unwrap(pInfo->sampler);
            }
        }
    }
    VkResult result = device_dispatch_table.GetSamplerOpaqueCaptureDescriptorDataEXT(
        device, (const VkSamplerCaptureDescriptorDataInfoEXT*)local_pInfo, pData);

    return result;
}

VkResult DispatchObject::GetAccelerationStructureOpaqueCaptureDescriptorDataEXT(
    VkDevice device, const VkAccelerationStructureCaptureDescriptorDataInfoEXT* pInfo, void* pData) {
    if (!wrap_handles) return device_dispatch_table.GetAccelerationStructureOpaqueCaptureDescriptorDataEXT(device, pInfo, pData);
    vku::safe_VkAccelerationStructureCaptureDescriptorDataInfoEXT var_local_pInfo;
    vku::safe_VkAccelerationStructureCaptureDescriptorDataInfoEXT* local_pInfo = nullptr;
    {
        if (pInfo) {
            local_pInfo = &var_local_pInfo;
            local_pInfo->initialize(pInfo);

            if (pInfo->accelerationStructure) {
                local_pInfo->accelerationStructure = Unwrap(pInfo->accelerationStructure);
            }
            if (pInfo->accelerationStructureNV) {
                local_pInfo->accelerationStructureNV = Unwrap(pInfo->accelerationStructureNV);
            }
        }
    }
    VkResult result = device_dispatch_table.GetAccelerationStructureOpaqueCaptureDescriptorDataEXT(
        device, (const VkAccelerationStructureCaptureDescriptorDataInfoEXT*)local_pInfo, pData);

    return result;
}

void DispatchObject::CmdSetFragmentShadingRateEnumNV(VkCommandBuffer commandBuffer, VkFragmentShadingRateNV shadingRate,
                                                     const VkFragmentShadingRateCombinerOpKHR combinerOps[2]) {
    device_dispatch_table.CmdSetFragmentShadingRateEnumNV(commandBuffer, shadingRate, combinerOps);
}

VkResult DispatchObject::GetDeviceFaultInfoEXT(VkDevice device, VkDeviceFaultCountsEXT* pFaultCounts,
                                               VkDeviceFaultInfoEXT* pFaultInfo) {
    VkResult result = device_dispatch_table.GetDeviceFaultInfoEXT(device, pFaultCounts, pFaultInfo);

    return result;
}
#ifdef VK_USE_PLATFORM_WIN32_KHR

VkResult DispatchObject::AcquireWinrtDisplayNV(VkPhysicalDevice physicalDevice, VkDisplayKHR display) {
    if (!wrap_handles) return instance_dispatch_table.AcquireWinrtDisplayNV(physicalDevice, display);
    { display = Unwrap(display); }
    VkResult result = instance_dispatch_table.AcquireWinrtDisplayNV(physicalDevice, display);

    return result;
}

VkResult DispatchObject::GetWinrtDisplayNV(VkPhysicalDevice physicalDevice, uint32_t deviceRelativeId, VkDisplayKHR* pDisplay) {
    if (!wrap_handles) return instance_dispatch_table.GetWinrtDisplayNV(physicalDevice, deviceRelativeId, pDisplay);

    VkResult result = instance_dispatch_table.GetWinrtDisplayNV(physicalDevice, deviceRelativeId, pDisplay);
    if (VK_SUCCESS == result) {
        *pDisplay = MaybeWrapDisplay(*pDisplay);
    }
    return result;
}
#endif  // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_DIRECTFB_EXT

VkResult DispatchObject::CreateDirectFBSurfaceEXT(VkInstance instance, const VkDirectFBSurfaceCreateInfoEXT* pCreateInfo,
                                                  const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    if (!wrap_handles) return instance_dispatch_table.CreateDirectFBSurfaceEXT(instance, pCreateInfo, pAllocator, pSurface);

    VkResult result = instance_dispatch_table.CreateDirectFBSurfaceEXT(instance, pCreateInfo, pAllocator, pSurface);
    if (VK_SUCCESS == result) {
        *pSurface = WrapNew(*pSurface);
    }
    return result;
}

VkBool32 DispatchObject::GetPhysicalDeviceDirectFBPresentationSupportEXT(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
                                                                         IDirectFB* dfb) {
    VkBool32 result =
        instance_dispatch_table.GetPhysicalDeviceDirectFBPresentationSupportEXT(physicalDevice, queueFamilyIndex, dfb);

    return result;
}
#endif  // VK_USE_PLATFORM_DIRECTFB_EXT

void DispatchObject::CmdSetVertexInputEXT(VkCommandBuffer commandBuffer, uint32_t vertexBindingDescriptionCount,
                                          const VkVertexInputBindingDescription2EXT* pVertexBindingDescriptions,
                                          uint32_t vertexAttributeDescriptionCount,
                                          const VkVertexInputAttributeDescription2EXT* pVertexAttributeDescriptions) {
    device_dispatch_table.CmdSetVertexInputEXT(commandBuffer, vertexBindingDescriptionCount, pVertexBindingDescriptions,
                                               vertexAttributeDescriptionCount, pVertexAttributeDescriptions);
}
#ifdef VK_USE_PLATFORM_FUCHSIA

VkResult DispatchObject::GetMemoryZirconHandleFUCHSIA(VkDevice device,
                                                      const VkMemoryGetZirconHandleInfoFUCHSIA* pGetZirconHandleInfo,
                                                      zx_handle_t* pZirconHandle) {
    if (!wrap_handles) return device_dispatch_table.GetMemoryZirconHandleFUCHSIA(device, pGetZirconHandleInfo, pZirconHandle);
    vku::safe_VkMemoryGetZirconHandleInfoFUCHSIA var_local_pGetZirconHandleInfo;
    vku::safe_VkMemoryGetZirconHandleInfoFUCHSIA* local_pGetZirconHandleInfo = nullptr;
    {
        if (pGetZirconHandleInfo) {
            local_pGetZirconHandleInfo = &var_local_pGetZirconHandleInfo;
            local_pGetZirconHandleInfo->initialize(pGetZirconHandleInfo);

            if (pGetZirconHandleInfo->memory) {
                local_pGetZirconHandleInfo->memory = Unwrap(pGetZirconHandleInfo->memory);
            }
        }
    }
    VkResult result = device_dispatch_table.GetMemoryZirconHandleFUCHSIA(
        device, (const VkMemoryGetZirconHandleInfoFUCHSIA*)local_pGetZirconHandleInfo, pZirconHandle);

    return result;
}

VkResult DispatchObject::GetMemoryZirconHandlePropertiesFUCHSIA(
    VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType, zx_handle_t zirconHandle,
    VkMemoryZirconHandlePropertiesFUCHSIA* pMemoryZirconHandleProperties) {
    VkResult result = device_dispatch_table.GetMemoryZirconHandlePropertiesFUCHSIA(device, handleType, zirconHandle,
                                                                                   pMemoryZirconHandleProperties);

    return result;
}

VkResult DispatchObject::ImportSemaphoreZirconHandleFUCHSIA(
    VkDevice device, const VkImportSemaphoreZirconHandleInfoFUCHSIA* pImportSemaphoreZirconHandleInfo) {
    if (!wrap_handles) return device_dispatch_table.ImportSemaphoreZirconHandleFUCHSIA(device, pImportSemaphoreZirconHandleInfo);
    vku::safe_VkImportSemaphoreZirconHandleInfoFUCHSIA var_local_pImportSemaphoreZirconHandleInfo;
    vku::safe_VkImportSemaphoreZirconHandleInfoFUCHSIA* local_pImportSemaphoreZirconHandleInfo = nullptr;
    {
        if (pImportSemaphoreZirconHandleInfo) {
            local_pImportSemaphoreZirconHandleInfo = &var_local_pImportSemaphoreZirconHandleInfo;
            local_pImportSemaphoreZirconHandleInfo->initialize(pImportSemaphoreZirconHandleInfo);

            if (pImportSemaphoreZirconHandleInfo->semaphore) {
                local_pImportSemaphoreZirconHandleInfo->semaphore = Unwrap(pImportSemaphoreZirconHandleInfo->semaphore);
            }
        }
    }
    VkResult result = device_dispatch_table.ImportSemaphoreZirconHandleFUCHSIA(
        device, (const VkImportSemaphoreZirconHandleInfoFUCHSIA*)local_pImportSemaphoreZirconHandleInfo);

    return result;
}

VkResult DispatchObject::GetSemaphoreZirconHandleFUCHSIA(VkDevice device,
                                                         const VkSemaphoreGetZirconHandleInfoFUCHSIA* pGetZirconHandleInfo,
                                                         zx_handle_t* pZirconHandle) {
    if (!wrap_handles) return device_dispatch_table.GetSemaphoreZirconHandleFUCHSIA(device, pGetZirconHandleInfo, pZirconHandle);
    vku::safe_VkSemaphoreGetZirconHandleInfoFUCHSIA var_local_pGetZirconHandleInfo;
    vku::safe_VkSemaphoreGetZirconHandleInfoFUCHSIA* local_pGetZirconHandleInfo = nullptr;
    {
        if (pGetZirconHandleInfo) {
            local_pGetZirconHandleInfo = &var_local_pGetZirconHandleInfo;
            local_pGetZirconHandleInfo->initialize(pGetZirconHandleInfo);

            if (pGetZirconHandleInfo->semaphore) {
                local_pGetZirconHandleInfo->semaphore = Unwrap(pGetZirconHandleInfo->semaphore);
            }
        }
    }
    VkResult result = device_dispatch_table.GetSemaphoreZirconHandleFUCHSIA(
        device, (const VkSemaphoreGetZirconHandleInfoFUCHSIA*)local_pGetZirconHandleInfo, pZirconHandle);

    return result;
}

VkResult DispatchObject::CreateBufferCollectionFUCHSIA(VkDevice device, const VkBufferCollectionCreateInfoFUCHSIA* pCreateInfo,
                                                       const VkAllocationCallbacks* pAllocator,
                                                       VkBufferCollectionFUCHSIA* pCollection) {
    if (!wrap_handles) return device_dispatch_table.CreateBufferCollectionFUCHSIA(device, pCreateInfo, pAllocator, pCollection);

    VkResult result = device_dispatch_table.CreateBufferCollectionFUCHSIA(device, pCreateInfo, pAllocator, pCollection);
    if (VK_SUCCESS == result) {
        *pCollection = WrapNew(*pCollection);
    }
    return result;
}

VkResult DispatchObject::SetBufferCollectionImageConstraintsFUCHSIA(VkDevice device, VkBufferCollectionFUCHSIA collection,
                                                                    const VkImageConstraintsInfoFUCHSIA* pImageConstraintsInfo) {
    if (!wrap_handles)
        return device_dispatch_table.SetBufferCollectionImageConstraintsFUCHSIA(device, collection, pImageConstraintsInfo);
    { collection = Unwrap(collection); }
    VkResult result = device_dispatch_table.SetBufferCollectionImageConstraintsFUCHSIA(device, collection, pImageConstraintsInfo);

    return result;
}

VkResult DispatchObject::SetBufferCollectionBufferConstraintsFUCHSIA(VkDevice device, VkBufferCollectionFUCHSIA collection,
                                                                     const VkBufferConstraintsInfoFUCHSIA* pBufferConstraintsInfo) {
    if (!wrap_handles)
        return device_dispatch_table.SetBufferCollectionBufferConstraintsFUCHSIA(device, collection, pBufferConstraintsInfo);
    { collection = Unwrap(collection); }
    VkResult result = device_dispatch_table.SetBufferCollectionBufferConstraintsFUCHSIA(device, collection, pBufferConstraintsInfo);

    return result;
}

void DispatchObject::DestroyBufferCollectionFUCHSIA(VkDevice device, VkBufferCollectionFUCHSIA collection,
                                                    const VkAllocationCallbacks* pAllocator) {
    if (!wrap_handles) return device_dispatch_table.DestroyBufferCollectionFUCHSIA(device, collection, pAllocator);

    uint64_t collection_id = CastToUint64(collection);
    auto iter = unique_id_mapping.pop(collection_id);
    if (iter != unique_id_mapping.end()) {
        collection = (VkBufferCollectionFUCHSIA)iter->second;
    } else {
        collection = (VkBufferCollectionFUCHSIA)0;
    }
    device_dispatch_table.DestroyBufferCollectionFUCHSIA(device, collection, pAllocator);
}

VkResult DispatchObject::GetBufferCollectionPropertiesFUCHSIA(VkDevice device, VkBufferCollectionFUCHSIA collection,
                                                              VkBufferCollectionPropertiesFUCHSIA* pProperties) {
    if (!wrap_handles) return device_dispatch_table.GetBufferCollectionPropertiesFUCHSIA(device, collection, pProperties);
    { collection = Unwrap(collection); }
    VkResult result = device_dispatch_table.GetBufferCollectionPropertiesFUCHSIA(device, collection, pProperties);

    return result;
}
#endif  // VK_USE_PLATFORM_FUCHSIA

VkResult DispatchObject::GetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI(VkDevice device, VkRenderPass renderpass,
                                                                       VkExtent2D* pMaxWorkgroupSize) {
    if (!wrap_handles)
        return device_dispatch_table.GetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI(device, renderpass, pMaxWorkgroupSize);
    { renderpass = Unwrap(renderpass); }
    VkResult result = device_dispatch_table.GetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI(device, renderpass, pMaxWorkgroupSize);

    return result;
}

void DispatchObject::CmdSubpassShadingHUAWEI(VkCommandBuffer commandBuffer) {
    device_dispatch_table.CmdSubpassShadingHUAWEI(commandBuffer);
}

void DispatchObject::CmdBindInvocationMaskHUAWEI(VkCommandBuffer commandBuffer, VkImageView imageView, VkImageLayout imageLayout) {
    if (!wrap_handles) return device_dispatch_table.CmdBindInvocationMaskHUAWEI(commandBuffer, imageView, imageLayout);
    { imageView = Unwrap(imageView); }
    device_dispatch_table.CmdBindInvocationMaskHUAWEI(commandBuffer, imageView, imageLayout);
}

VkResult DispatchObject::GetMemoryRemoteAddressNV(VkDevice device,
                                                  const VkMemoryGetRemoteAddressInfoNV* pMemoryGetRemoteAddressInfo,
                                                  VkRemoteAddressNV* pAddress) {
    if (!wrap_handles) return device_dispatch_table.GetMemoryRemoteAddressNV(device, pMemoryGetRemoteAddressInfo, pAddress);
    vku::safe_VkMemoryGetRemoteAddressInfoNV var_local_pMemoryGetRemoteAddressInfo;
    vku::safe_VkMemoryGetRemoteAddressInfoNV* local_pMemoryGetRemoteAddressInfo = nullptr;
    {
        if (pMemoryGetRemoteAddressInfo) {
            local_pMemoryGetRemoteAddressInfo = &var_local_pMemoryGetRemoteAddressInfo;
            local_pMemoryGetRemoteAddressInfo->initialize(pMemoryGetRemoteAddressInfo);

            if (pMemoryGetRemoteAddressInfo->memory) {
                local_pMemoryGetRemoteAddressInfo->memory = Unwrap(pMemoryGetRemoteAddressInfo->memory);
            }
        }
    }
    VkResult result = device_dispatch_table.GetMemoryRemoteAddressNV(
        device, (const VkMemoryGetRemoteAddressInfoNV*)local_pMemoryGetRemoteAddressInfo, pAddress);

    return result;
}

VkResult DispatchObject::GetPipelinePropertiesEXT(VkDevice device, const VkPipelineInfoEXT* pPipelineInfo,
                                                  VkBaseOutStructure* pPipelineProperties) {
    VkResult result = device_dispatch_table.GetPipelinePropertiesEXT(device, pPipelineInfo, pPipelineProperties);

    return result;
}

void DispatchObject::CmdSetPatchControlPointsEXT(VkCommandBuffer commandBuffer, uint32_t patchControlPoints) {
    device_dispatch_table.CmdSetPatchControlPointsEXT(commandBuffer, patchControlPoints);
}

void DispatchObject::CmdSetRasterizerDiscardEnableEXT(VkCommandBuffer commandBuffer, VkBool32 rasterizerDiscardEnable) {
    device_dispatch_table.CmdSetRasterizerDiscardEnableEXT(commandBuffer, rasterizerDiscardEnable);
}

void DispatchObject::CmdSetDepthBiasEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthBiasEnable) {
    device_dispatch_table.CmdSetDepthBiasEnableEXT(commandBuffer, depthBiasEnable);
}

void DispatchObject::CmdSetLogicOpEXT(VkCommandBuffer commandBuffer, VkLogicOp logicOp) {
    device_dispatch_table.CmdSetLogicOpEXT(commandBuffer, logicOp);
}

void DispatchObject::CmdSetPrimitiveRestartEnableEXT(VkCommandBuffer commandBuffer, VkBool32 primitiveRestartEnable) {
    device_dispatch_table.CmdSetPrimitiveRestartEnableEXT(commandBuffer, primitiveRestartEnable);
}
#ifdef VK_USE_PLATFORM_SCREEN_QNX

VkResult DispatchObject::CreateScreenSurfaceQNX(VkInstance instance, const VkScreenSurfaceCreateInfoQNX* pCreateInfo,
                                                const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    if (!wrap_handles) return instance_dispatch_table.CreateScreenSurfaceQNX(instance, pCreateInfo, pAllocator, pSurface);

    VkResult result = instance_dispatch_table.CreateScreenSurfaceQNX(instance, pCreateInfo, pAllocator, pSurface);
    if (VK_SUCCESS == result) {
        *pSurface = WrapNew(*pSurface);
    }
    return result;
}

VkBool32 DispatchObject::GetPhysicalDeviceScreenPresentationSupportQNX(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
                                                                       struct _screen_window* window) {
    VkBool32 result =
        instance_dispatch_table.GetPhysicalDeviceScreenPresentationSupportQNX(physicalDevice, queueFamilyIndex, window);

    return result;
}
#endif  // VK_USE_PLATFORM_SCREEN_QNX

void DispatchObject::CmdSetColorWriteEnableEXT(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
                                               const VkBool32* pColorWriteEnables) {
    device_dispatch_table.CmdSetColorWriteEnableEXT(commandBuffer, attachmentCount, pColorWriteEnables);
}

void DispatchObject::CmdDrawMultiEXT(VkCommandBuffer commandBuffer, uint32_t drawCount, const VkMultiDrawInfoEXT* pVertexInfo,
                                     uint32_t instanceCount, uint32_t firstInstance, uint32_t stride) {
    device_dispatch_table.CmdDrawMultiEXT(commandBuffer, drawCount, pVertexInfo, instanceCount, firstInstance, stride);
}

void DispatchObject::CmdDrawMultiIndexedEXT(VkCommandBuffer commandBuffer, uint32_t drawCount,
                                            const VkMultiDrawIndexedInfoEXT* pIndexInfo, uint32_t instanceCount,
                                            uint32_t firstInstance, uint32_t stride, const int32_t* pVertexOffset) {
    device_dispatch_table.CmdDrawMultiIndexedEXT(commandBuffer, drawCount, pIndexInfo, instanceCount, firstInstance, stride,
                                                 pVertexOffset);
}

VkResult DispatchObject::CreateMicromapEXT(VkDevice device, const VkMicromapCreateInfoEXT* pCreateInfo,
                                           const VkAllocationCallbacks* pAllocator, VkMicromapEXT* pMicromap) {
    if (!wrap_handles) return device_dispatch_table.CreateMicromapEXT(device, pCreateInfo, pAllocator, pMicromap);
    vku::safe_VkMicromapCreateInfoEXT var_local_pCreateInfo;
    vku::safe_VkMicromapCreateInfoEXT* local_pCreateInfo = nullptr;
    {
        if (pCreateInfo) {
            local_pCreateInfo = &var_local_pCreateInfo;
            local_pCreateInfo->initialize(pCreateInfo);

            if (pCreateInfo->buffer) {
                local_pCreateInfo->buffer = Unwrap(pCreateInfo->buffer);
            }
        }
    }
    VkResult result =
        device_dispatch_table.CreateMicromapEXT(device, (const VkMicromapCreateInfoEXT*)local_pCreateInfo, pAllocator, pMicromap);
    if (VK_SUCCESS == result) {
        *pMicromap = WrapNew(*pMicromap);
    }
    return result;
}

void DispatchObject::DestroyMicromapEXT(VkDevice device, VkMicromapEXT micromap, const VkAllocationCallbacks* pAllocator) {
    if (!wrap_handles) return device_dispatch_table.DestroyMicromapEXT(device, micromap, pAllocator);

    uint64_t micromap_id = CastToUint64(micromap);
    auto iter = unique_id_mapping.pop(micromap_id);
    if (iter != unique_id_mapping.end()) {
        micromap = (VkMicromapEXT)iter->second;
    } else {
        micromap = (VkMicromapEXT)0;
    }
    device_dispatch_table.DestroyMicromapEXT(device, micromap, pAllocator);
}

void DispatchObject::CmdBuildMicromapsEXT(VkCommandBuffer commandBuffer, uint32_t infoCount, const VkMicromapBuildInfoEXT* pInfos) {
    if (!wrap_handles) return device_dispatch_table.CmdBuildMicromapsEXT(commandBuffer, infoCount, pInfos);
    small_vector<vku::safe_VkMicromapBuildInfoEXT, DISPATCH_MAX_STACK_ALLOCATIONS> var_local_pInfos;
    vku::safe_VkMicromapBuildInfoEXT* local_pInfos = nullptr;
    {
        if (pInfos) {
            var_local_pInfos.resize(infoCount);
            local_pInfos = var_local_pInfos.data();
            for (uint32_t index0 = 0; index0 < infoCount; ++index0) {
                local_pInfos[index0].initialize(&pInfos[index0]);

                if (pInfos[index0].dstMicromap) {
                    local_pInfos[index0].dstMicromap = Unwrap(pInfos[index0].dstMicromap);
                }
            }
        }
    }
    device_dispatch_table.CmdBuildMicromapsEXT(commandBuffer, infoCount, (const VkMicromapBuildInfoEXT*)local_pInfos);
}

VkResult DispatchObject::BuildMicromapsEXT(VkDevice device, VkDeferredOperationKHR deferredOperation, uint32_t infoCount,
                                           const VkMicromapBuildInfoEXT* pInfos) {
    if (!wrap_handles) return device_dispatch_table.BuildMicromapsEXT(device, deferredOperation, infoCount, pInfos);
    vku::safe_VkMicromapBuildInfoEXT* local_pInfos = nullptr;
    {
        deferredOperation = Unwrap(deferredOperation);
        if (pInfos) {
            local_pInfos = new vku::safe_VkMicromapBuildInfoEXT[infoCount];
            for (uint32_t index0 = 0; index0 < infoCount; ++index0) {
                local_pInfos[index0].initialize(&pInfos[index0]);

                if (pInfos[index0].dstMicromap) {
                    local_pInfos[index0].dstMicromap = Unwrap(pInfos[index0].dstMicromap);
                }
            }
        }
    }
    VkResult result =
        device_dispatch_table.BuildMicromapsEXT(device, deferredOperation, infoCount, (const VkMicromapBuildInfoEXT*)local_pInfos);
    if (local_pInfos) {
        // Fix check for deferred ray tracing pipeline creation
        // https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/5817
        const bool is_operation_deferred = (deferredOperation != VK_NULL_HANDLE) && (result == VK_OPERATION_DEFERRED_KHR);
        if (is_operation_deferred) {
            std::vector<std::function<void()>> cleanup{[local_pInfos]() { delete[] local_pInfos; }};
            deferred_operation_post_completion.insert(deferredOperation, cleanup);
        } else {
            delete[] local_pInfos;
        }
    }
    return result;
}

VkResult DispatchObject::CopyMicromapEXT(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                         const VkCopyMicromapInfoEXT* pInfo) {
    if (!wrap_handles) return device_dispatch_table.CopyMicromapEXT(device, deferredOperation, pInfo);
    vku::safe_VkCopyMicromapInfoEXT* local_pInfo = nullptr;
    {
        deferredOperation = Unwrap(deferredOperation);
        if (pInfo) {
            local_pInfo = new vku::safe_VkCopyMicromapInfoEXT;
            local_pInfo->initialize(pInfo);

            if (pInfo->src) {
                local_pInfo->src = Unwrap(pInfo->src);
            }
            if (pInfo->dst) {
                local_pInfo->dst = Unwrap(pInfo->dst);
            }
        }
    }
    VkResult result = device_dispatch_table.CopyMicromapEXT(device, deferredOperation, (const VkCopyMicromapInfoEXT*)local_pInfo);
    if (local_pInfo) {
        // Fix check for deferred ray tracing pipeline creation
        // https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/5817
        const bool is_operation_deferred = (deferredOperation != VK_NULL_HANDLE) && (result == VK_OPERATION_DEFERRED_KHR);
        if (is_operation_deferred) {
            std::vector<std::function<void()>> cleanup{[local_pInfo]() { delete local_pInfo; }};
            deferred_operation_post_completion.insert(deferredOperation, cleanup);
        } else {
            delete local_pInfo;
        }
    }
    return result;
}

VkResult DispatchObject::CopyMicromapToMemoryEXT(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                 const VkCopyMicromapToMemoryInfoEXT* pInfo) {
    if (!wrap_handles) return device_dispatch_table.CopyMicromapToMemoryEXT(device, deferredOperation, pInfo);
    vku::safe_VkCopyMicromapToMemoryInfoEXT* local_pInfo = nullptr;
    {
        deferredOperation = Unwrap(deferredOperation);
        if (pInfo) {
            local_pInfo = new vku::safe_VkCopyMicromapToMemoryInfoEXT;
            local_pInfo->initialize(pInfo);

            if (pInfo->src) {
                local_pInfo->src = Unwrap(pInfo->src);
            }
        }
    }
    VkResult result =
        device_dispatch_table.CopyMicromapToMemoryEXT(device, deferredOperation, (const VkCopyMicromapToMemoryInfoEXT*)local_pInfo);
    if (local_pInfo) {
        // Fix check for deferred ray tracing pipeline creation
        // https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/5817
        const bool is_operation_deferred = (deferredOperation != VK_NULL_HANDLE) && (result == VK_OPERATION_DEFERRED_KHR);
        if (is_operation_deferred) {
            std::vector<std::function<void()>> cleanup{[local_pInfo]() { delete local_pInfo; }};
            deferred_operation_post_completion.insert(deferredOperation, cleanup);
        } else {
            delete local_pInfo;
        }
    }
    return result;
}

VkResult DispatchObject::CopyMemoryToMicromapEXT(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                 const VkCopyMemoryToMicromapInfoEXT* pInfo) {
    if (!wrap_handles) return device_dispatch_table.CopyMemoryToMicromapEXT(device, deferredOperation, pInfo);
    vku::safe_VkCopyMemoryToMicromapInfoEXT* local_pInfo = nullptr;
    {
        deferredOperation = Unwrap(deferredOperation);
        if (pInfo) {
            local_pInfo = new vku::safe_VkCopyMemoryToMicromapInfoEXT;
            local_pInfo->initialize(pInfo);

            if (pInfo->dst) {
                local_pInfo->dst = Unwrap(pInfo->dst);
            }
        }
    }
    VkResult result =
        device_dispatch_table.CopyMemoryToMicromapEXT(device, deferredOperation, (const VkCopyMemoryToMicromapInfoEXT*)local_pInfo);
    if (local_pInfo) {
        // Fix check for deferred ray tracing pipeline creation
        // https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/5817
        const bool is_operation_deferred = (deferredOperation != VK_NULL_HANDLE) && (result == VK_OPERATION_DEFERRED_KHR);
        if (is_operation_deferred) {
            std::vector<std::function<void()>> cleanup{[local_pInfo]() { delete local_pInfo; }};
            deferred_operation_post_completion.insert(deferredOperation, cleanup);
        } else {
            delete local_pInfo;
        }
    }
    return result;
}

VkResult DispatchObject::WriteMicromapsPropertiesEXT(VkDevice device, uint32_t micromapCount, const VkMicromapEXT* pMicromaps,
                                                     VkQueryType queryType, size_t dataSize, void* pData, size_t stride) {
    if (!wrap_handles)
        return device_dispatch_table.WriteMicromapsPropertiesEXT(device, micromapCount, pMicromaps, queryType, dataSize, pData,
                                                                 stride);
    small_vector<VkMicromapEXT, DISPATCH_MAX_STACK_ALLOCATIONS> var_local_pMicromaps;
    VkMicromapEXT* local_pMicromaps = nullptr;
    {
        if (pMicromaps) {
            var_local_pMicromaps.resize(micromapCount);
            local_pMicromaps = var_local_pMicromaps.data();
            for (uint32_t index0 = 0; index0 < micromapCount; ++index0) {
                local_pMicromaps[index0] = Unwrap(pMicromaps[index0]);
            }
        }
    }
    VkResult result = device_dispatch_table.WriteMicromapsPropertiesEXT(
        device, micromapCount, (const VkMicromapEXT*)local_pMicromaps, queryType, dataSize, pData, stride);

    return result;
}

void DispatchObject::CmdCopyMicromapEXT(VkCommandBuffer commandBuffer, const VkCopyMicromapInfoEXT* pInfo) {
    if (!wrap_handles) return device_dispatch_table.CmdCopyMicromapEXT(commandBuffer, pInfo);
    vku::safe_VkCopyMicromapInfoEXT var_local_pInfo;
    vku::safe_VkCopyMicromapInfoEXT* local_pInfo = nullptr;
    {
        if (pInfo) {
            local_pInfo = &var_local_pInfo;
            local_pInfo->initialize(pInfo);

            if (pInfo->src) {
                local_pInfo->src = Unwrap(pInfo->src);
            }
            if (pInfo->dst) {
                local_pInfo->dst = Unwrap(pInfo->dst);
            }
        }
    }
    device_dispatch_table.CmdCopyMicromapEXT(commandBuffer, (const VkCopyMicromapInfoEXT*)local_pInfo);
}

void DispatchObject::CmdCopyMicromapToMemoryEXT(VkCommandBuffer commandBuffer, const VkCopyMicromapToMemoryInfoEXT* pInfo) {
    if (!wrap_handles) return device_dispatch_table.CmdCopyMicromapToMemoryEXT(commandBuffer, pInfo);
    vku::safe_VkCopyMicromapToMemoryInfoEXT var_local_pInfo;
    vku::safe_VkCopyMicromapToMemoryInfoEXT* local_pInfo = nullptr;
    {
        if (pInfo) {
            local_pInfo = &var_local_pInfo;
            local_pInfo->initialize(pInfo);

            if (pInfo->src) {
                local_pInfo->src = Unwrap(pInfo->src);
            }
        }
    }
    device_dispatch_table.CmdCopyMicromapToMemoryEXT(commandBuffer, (const VkCopyMicromapToMemoryInfoEXT*)local_pInfo);
}

void DispatchObject::CmdCopyMemoryToMicromapEXT(VkCommandBuffer commandBuffer, const VkCopyMemoryToMicromapInfoEXT* pInfo) {
    if (!wrap_handles) return device_dispatch_table.CmdCopyMemoryToMicromapEXT(commandBuffer, pInfo);
    vku::safe_VkCopyMemoryToMicromapInfoEXT var_local_pInfo;
    vku::safe_VkCopyMemoryToMicromapInfoEXT* local_pInfo = nullptr;
    {
        if (pInfo) {
            local_pInfo = &var_local_pInfo;
            local_pInfo->initialize(pInfo);

            if (pInfo->dst) {
                local_pInfo->dst = Unwrap(pInfo->dst);
            }
        }
    }
    device_dispatch_table.CmdCopyMemoryToMicromapEXT(commandBuffer, (const VkCopyMemoryToMicromapInfoEXT*)local_pInfo);
}

void DispatchObject::CmdWriteMicromapsPropertiesEXT(VkCommandBuffer commandBuffer, uint32_t micromapCount,
                                                    const VkMicromapEXT* pMicromaps, VkQueryType queryType, VkQueryPool queryPool,
                                                    uint32_t firstQuery) {
    if (!wrap_handles)
        return device_dispatch_table.CmdWriteMicromapsPropertiesEXT(commandBuffer, micromapCount, pMicromaps, queryType, queryPool,
                                                                    firstQuery);
    small_vector<VkMicromapEXT, DISPATCH_MAX_STACK_ALLOCATIONS> var_local_pMicromaps;
    VkMicromapEXT* local_pMicromaps = nullptr;
    {
        if (pMicromaps) {
            var_local_pMicromaps.resize(micromapCount);
            local_pMicromaps = var_local_pMicromaps.data();
            for (uint32_t index0 = 0; index0 < micromapCount; ++index0) {
                local_pMicromaps[index0] = Unwrap(pMicromaps[index0]);
            }
        }
        queryPool = Unwrap(queryPool);
    }
    device_dispatch_table.CmdWriteMicromapsPropertiesEXT(commandBuffer, micromapCount, (const VkMicromapEXT*)local_pMicromaps,
                                                         queryType, queryPool, firstQuery);
}

void DispatchObject::GetDeviceMicromapCompatibilityEXT(VkDevice device, const VkMicromapVersionInfoEXT* pVersionInfo,
                                                       VkAccelerationStructureCompatibilityKHR* pCompatibility) {
    device_dispatch_table.GetDeviceMicromapCompatibilityEXT(device, pVersionInfo, pCompatibility);
}

void DispatchObject::GetMicromapBuildSizesEXT(VkDevice device, VkAccelerationStructureBuildTypeKHR buildType,
                                              const VkMicromapBuildInfoEXT* pBuildInfo, VkMicromapBuildSizesInfoEXT* pSizeInfo) {
    if (!wrap_handles) return device_dispatch_table.GetMicromapBuildSizesEXT(device, buildType, pBuildInfo, pSizeInfo);
    vku::safe_VkMicromapBuildInfoEXT var_local_pBuildInfo;
    vku::safe_VkMicromapBuildInfoEXT* local_pBuildInfo = nullptr;
    {
        if (pBuildInfo) {
            local_pBuildInfo = &var_local_pBuildInfo;
            local_pBuildInfo->initialize(pBuildInfo);

            if (pBuildInfo->dstMicromap) {
                local_pBuildInfo->dstMicromap = Unwrap(pBuildInfo->dstMicromap);
            }
        }
    }
    device_dispatch_table.GetMicromapBuildSizesEXT(device, buildType, (const VkMicromapBuildInfoEXT*)local_pBuildInfo, pSizeInfo);
}

void DispatchObject::CmdDrawClusterHUAWEI(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY,
                                          uint32_t groupCountZ) {
    device_dispatch_table.CmdDrawClusterHUAWEI(commandBuffer, groupCountX, groupCountY, groupCountZ);
}

void DispatchObject::CmdDrawClusterIndirectHUAWEI(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset) {
    if (!wrap_handles) return device_dispatch_table.CmdDrawClusterIndirectHUAWEI(commandBuffer, buffer, offset);
    { buffer = Unwrap(buffer); }
    device_dispatch_table.CmdDrawClusterIndirectHUAWEI(commandBuffer, buffer, offset);
}

void DispatchObject::SetDeviceMemoryPriorityEXT(VkDevice device, VkDeviceMemory memory, float priority) {
    if (!wrap_handles) return device_dispatch_table.SetDeviceMemoryPriorityEXT(device, memory, priority);
    { memory = Unwrap(memory); }
    device_dispatch_table.SetDeviceMemoryPriorityEXT(device, memory, priority);
}

void DispatchObject::GetDescriptorSetLayoutHostMappingInfoVALVE(VkDevice device,
                                                                const VkDescriptorSetBindingReferenceVALVE* pBindingReference,
                                                                VkDescriptorSetLayoutHostMappingInfoVALVE* pHostMapping) {
    if (!wrap_handles)
        return device_dispatch_table.GetDescriptorSetLayoutHostMappingInfoVALVE(device, pBindingReference, pHostMapping);
    vku::safe_VkDescriptorSetBindingReferenceVALVE var_local_pBindingReference;
    vku::safe_VkDescriptorSetBindingReferenceVALVE* local_pBindingReference = nullptr;
    {
        if (pBindingReference) {
            local_pBindingReference = &var_local_pBindingReference;
            local_pBindingReference->initialize(pBindingReference);

            if (pBindingReference->descriptorSetLayout) {
                local_pBindingReference->descriptorSetLayout = Unwrap(pBindingReference->descriptorSetLayout);
            }
        }
    }
    device_dispatch_table.GetDescriptorSetLayoutHostMappingInfoVALVE(
        device, (const VkDescriptorSetBindingReferenceVALVE*)local_pBindingReference, pHostMapping);
}

void DispatchObject::GetDescriptorSetHostMappingVALVE(VkDevice device, VkDescriptorSet descriptorSet, void** ppData) {
    if (!wrap_handles) return device_dispatch_table.GetDescriptorSetHostMappingVALVE(device, descriptorSet, ppData);
    { descriptorSet = Unwrap(descriptorSet); }
    device_dispatch_table.GetDescriptorSetHostMappingVALVE(device, descriptorSet, ppData);
}

void DispatchObject::CmdCopyMemoryIndirectNV(VkCommandBuffer commandBuffer, VkDeviceAddress copyBufferAddress, uint32_t copyCount,
                                             uint32_t stride) {
    device_dispatch_table.CmdCopyMemoryIndirectNV(commandBuffer, copyBufferAddress, copyCount, stride);
}

void DispatchObject::CmdCopyMemoryToImageIndirectNV(VkCommandBuffer commandBuffer, VkDeviceAddress copyBufferAddress,
                                                    uint32_t copyCount, uint32_t stride, VkImage dstImage,
                                                    VkImageLayout dstImageLayout,
                                                    const VkImageSubresourceLayers* pImageSubresources) {
    if (!wrap_handles)
        return device_dispatch_table.CmdCopyMemoryToImageIndirectNV(commandBuffer, copyBufferAddress, copyCount, stride, dstImage,
                                                                    dstImageLayout, pImageSubresources);
    { dstImage = Unwrap(dstImage); }
    device_dispatch_table.CmdCopyMemoryToImageIndirectNV(commandBuffer, copyBufferAddress, copyCount, stride, dstImage,
                                                         dstImageLayout, pImageSubresources);
}

void DispatchObject::CmdDecompressMemoryNV(VkCommandBuffer commandBuffer, uint32_t decompressRegionCount,
                                           const VkDecompressMemoryRegionNV* pDecompressMemoryRegions) {
    device_dispatch_table.CmdDecompressMemoryNV(commandBuffer, decompressRegionCount, pDecompressMemoryRegions);
}

void DispatchObject::CmdDecompressMemoryIndirectCountNV(VkCommandBuffer commandBuffer, VkDeviceAddress indirectCommandsAddress,
                                                        VkDeviceAddress indirectCommandsCountAddress, uint32_t stride) {
    device_dispatch_table.CmdDecompressMemoryIndirectCountNV(commandBuffer, indirectCommandsAddress, indirectCommandsCountAddress,
                                                             stride);
}

void DispatchObject::GetPipelineIndirectMemoryRequirementsNV(VkDevice device, const VkComputePipelineCreateInfo* pCreateInfo,
                                                             VkMemoryRequirements2* pMemoryRequirements) {
    if (!wrap_handles)
        return device_dispatch_table.GetPipelineIndirectMemoryRequirementsNV(device, pCreateInfo, pMemoryRequirements);
    vku::safe_VkComputePipelineCreateInfo var_local_pCreateInfo;
    vku::safe_VkComputePipelineCreateInfo* local_pCreateInfo = nullptr;
    {
        if (pCreateInfo) {
            local_pCreateInfo = &var_local_pCreateInfo;
            local_pCreateInfo->initialize(pCreateInfo);

            if (pCreateInfo->stage.module) {
                local_pCreateInfo->stage.module = Unwrap(pCreateInfo->stage.module);
            }
            UnwrapPnextChainHandles(local_pCreateInfo->stage.pNext);

            if (pCreateInfo->layout) {
                local_pCreateInfo->layout = Unwrap(pCreateInfo->layout);
            }
            if (pCreateInfo->basePipelineHandle) {
                local_pCreateInfo->basePipelineHandle = Unwrap(pCreateInfo->basePipelineHandle);
            }
            UnwrapPnextChainHandles(local_pCreateInfo->pNext);
        }
    }
    device_dispatch_table.GetPipelineIndirectMemoryRequirementsNV(device, (const VkComputePipelineCreateInfo*)local_pCreateInfo,
                                                                  pMemoryRequirements);
}

void DispatchObject::CmdUpdatePipelineIndirectBufferNV(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                       VkPipeline pipeline) {
    if (!wrap_handles) return device_dispatch_table.CmdUpdatePipelineIndirectBufferNV(commandBuffer, pipelineBindPoint, pipeline);
    { pipeline = Unwrap(pipeline); }
    device_dispatch_table.CmdUpdatePipelineIndirectBufferNV(commandBuffer, pipelineBindPoint, pipeline);
}

VkDeviceAddress DispatchObject::GetPipelineIndirectDeviceAddressNV(VkDevice device,
                                                                   const VkPipelineIndirectDeviceAddressInfoNV* pInfo) {
    if (!wrap_handles) return device_dispatch_table.GetPipelineIndirectDeviceAddressNV(device, pInfo);
    vku::safe_VkPipelineIndirectDeviceAddressInfoNV var_local_pInfo;
    vku::safe_VkPipelineIndirectDeviceAddressInfoNV* local_pInfo = nullptr;
    {
        if (pInfo) {
            local_pInfo = &var_local_pInfo;
            local_pInfo->initialize(pInfo);

            if (pInfo->pipeline) {
                local_pInfo->pipeline = Unwrap(pInfo->pipeline);
            }
        }
    }
    VkDeviceAddress result =
        device_dispatch_table.GetPipelineIndirectDeviceAddressNV(device, (const VkPipelineIndirectDeviceAddressInfoNV*)local_pInfo);

    return result;
}

void DispatchObject::CmdSetDepthClampEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthClampEnable) {
    device_dispatch_table.CmdSetDepthClampEnableEXT(commandBuffer, depthClampEnable);
}

void DispatchObject::CmdSetPolygonModeEXT(VkCommandBuffer commandBuffer, VkPolygonMode polygonMode) {
    device_dispatch_table.CmdSetPolygonModeEXT(commandBuffer, polygonMode);
}

void DispatchObject::CmdSetRasterizationSamplesEXT(VkCommandBuffer commandBuffer, VkSampleCountFlagBits rasterizationSamples) {
    device_dispatch_table.CmdSetRasterizationSamplesEXT(commandBuffer, rasterizationSamples);
}

void DispatchObject::CmdSetSampleMaskEXT(VkCommandBuffer commandBuffer, VkSampleCountFlagBits samples,
                                         const VkSampleMask* pSampleMask) {
    device_dispatch_table.CmdSetSampleMaskEXT(commandBuffer, samples, pSampleMask);
}

void DispatchObject::CmdSetAlphaToCoverageEnableEXT(VkCommandBuffer commandBuffer, VkBool32 alphaToCoverageEnable) {
    device_dispatch_table.CmdSetAlphaToCoverageEnableEXT(commandBuffer, alphaToCoverageEnable);
}

void DispatchObject::CmdSetAlphaToOneEnableEXT(VkCommandBuffer commandBuffer, VkBool32 alphaToOneEnable) {
    device_dispatch_table.CmdSetAlphaToOneEnableEXT(commandBuffer, alphaToOneEnable);
}

void DispatchObject::CmdSetLogicOpEnableEXT(VkCommandBuffer commandBuffer, VkBool32 logicOpEnable) {
    device_dispatch_table.CmdSetLogicOpEnableEXT(commandBuffer, logicOpEnable);
}

void DispatchObject::CmdSetColorBlendEnableEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment, uint32_t attachmentCount,
                                               const VkBool32* pColorBlendEnables) {
    device_dispatch_table.CmdSetColorBlendEnableEXT(commandBuffer, firstAttachment, attachmentCount, pColorBlendEnables);
}

void DispatchObject::CmdSetColorBlendEquationEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment, uint32_t attachmentCount,
                                                 const VkColorBlendEquationEXT* pColorBlendEquations) {
    device_dispatch_table.CmdSetColorBlendEquationEXT(commandBuffer, firstAttachment, attachmentCount, pColorBlendEquations);
}

void DispatchObject::CmdSetColorWriteMaskEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment, uint32_t attachmentCount,
                                             const VkColorComponentFlags* pColorWriteMasks) {
    device_dispatch_table.CmdSetColorWriteMaskEXT(commandBuffer, firstAttachment, attachmentCount, pColorWriteMasks);
}

void DispatchObject::CmdSetTessellationDomainOriginEXT(VkCommandBuffer commandBuffer, VkTessellationDomainOrigin domainOrigin) {
    device_dispatch_table.CmdSetTessellationDomainOriginEXT(commandBuffer, domainOrigin);
}

void DispatchObject::CmdSetRasterizationStreamEXT(VkCommandBuffer commandBuffer, uint32_t rasterizationStream) {
    device_dispatch_table.CmdSetRasterizationStreamEXT(commandBuffer, rasterizationStream);
}

void DispatchObject::CmdSetConservativeRasterizationModeEXT(VkCommandBuffer commandBuffer,
                                                            VkConservativeRasterizationModeEXT conservativeRasterizationMode) {
    device_dispatch_table.CmdSetConservativeRasterizationModeEXT(commandBuffer, conservativeRasterizationMode);
}

void DispatchObject::CmdSetExtraPrimitiveOverestimationSizeEXT(VkCommandBuffer commandBuffer,
                                                               float extraPrimitiveOverestimationSize) {
    device_dispatch_table.CmdSetExtraPrimitiveOverestimationSizeEXT(commandBuffer, extraPrimitiveOverestimationSize);
}

void DispatchObject::CmdSetDepthClipEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthClipEnable) {
    device_dispatch_table.CmdSetDepthClipEnableEXT(commandBuffer, depthClipEnable);
}

void DispatchObject::CmdSetSampleLocationsEnableEXT(VkCommandBuffer commandBuffer, VkBool32 sampleLocationsEnable) {
    device_dispatch_table.CmdSetSampleLocationsEnableEXT(commandBuffer, sampleLocationsEnable);
}

void DispatchObject::CmdSetColorBlendAdvancedEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment, uint32_t attachmentCount,
                                                 const VkColorBlendAdvancedEXT* pColorBlendAdvanced) {
    device_dispatch_table.CmdSetColorBlendAdvancedEXT(commandBuffer, firstAttachment, attachmentCount, pColorBlendAdvanced);
}

void DispatchObject::CmdSetProvokingVertexModeEXT(VkCommandBuffer commandBuffer, VkProvokingVertexModeEXT provokingVertexMode) {
    device_dispatch_table.CmdSetProvokingVertexModeEXT(commandBuffer, provokingVertexMode);
}

void DispatchObject::CmdSetLineRasterizationModeEXT(VkCommandBuffer commandBuffer,
                                                    VkLineRasterizationModeEXT lineRasterizationMode) {
    device_dispatch_table.CmdSetLineRasterizationModeEXT(commandBuffer, lineRasterizationMode);
}

void DispatchObject::CmdSetLineStippleEnableEXT(VkCommandBuffer commandBuffer, VkBool32 stippledLineEnable) {
    device_dispatch_table.CmdSetLineStippleEnableEXT(commandBuffer, stippledLineEnable);
}

void DispatchObject::CmdSetDepthClipNegativeOneToOneEXT(VkCommandBuffer commandBuffer, VkBool32 negativeOneToOne) {
    device_dispatch_table.CmdSetDepthClipNegativeOneToOneEXT(commandBuffer, negativeOneToOne);
}

void DispatchObject::CmdSetViewportWScalingEnableNV(VkCommandBuffer commandBuffer, VkBool32 viewportWScalingEnable) {
    device_dispatch_table.CmdSetViewportWScalingEnableNV(commandBuffer, viewportWScalingEnable);
}

void DispatchObject::CmdSetViewportSwizzleNV(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount,
                                             const VkViewportSwizzleNV* pViewportSwizzles) {
    device_dispatch_table.CmdSetViewportSwizzleNV(commandBuffer, firstViewport, viewportCount, pViewportSwizzles);
}

void DispatchObject::CmdSetCoverageToColorEnableNV(VkCommandBuffer commandBuffer, VkBool32 coverageToColorEnable) {
    device_dispatch_table.CmdSetCoverageToColorEnableNV(commandBuffer, coverageToColorEnable);
}

void DispatchObject::CmdSetCoverageToColorLocationNV(VkCommandBuffer commandBuffer, uint32_t coverageToColorLocation) {
    device_dispatch_table.CmdSetCoverageToColorLocationNV(commandBuffer, coverageToColorLocation);
}

void DispatchObject::CmdSetCoverageModulationModeNV(VkCommandBuffer commandBuffer,
                                                    VkCoverageModulationModeNV coverageModulationMode) {
    device_dispatch_table.CmdSetCoverageModulationModeNV(commandBuffer, coverageModulationMode);
}

void DispatchObject::CmdSetCoverageModulationTableEnableNV(VkCommandBuffer commandBuffer, VkBool32 coverageModulationTableEnable) {
    device_dispatch_table.CmdSetCoverageModulationTableEnableNV(commandBuffer, coverageModulationTableEnable);
}

void DispatchObject::CmdSetCoverageModulationTableNV(VkCommandBuffer commandBuffer, uint32_t coverageModulationTableCount,
                                                     const float* pCoverageModulationTable) {
    device_dispatch_table.CmdSetCoverageModulationTableNV(commandBuffer, coverageModulationTableCount, pCoverageModulationTable);
}

void DispatchObject::CmdSetShadingRateImageEnableNV(VkCommandBuffer commandBuffer, VkBool32 shadingRateImageEnable) {
    device_dispatch_table.CmdSetShadingRateImageEnableNV(commandBuffer, shadingRateImageEnable);
}

void DispatchObject::CmdSetRepresentativeFragmentTestEnableNV(VkCommandBuffer commandBuffer,
                                                              VkBool32 representativeFragmentTestEnable) {
    device_dispatch_table.CmdSetRepresentativeFragmentTestEnableNV(commandBuffer, representativeFragmentTestEnable);
}

void DispatchObject::CmdSetCoverageReductionModeNV(VkCommandBuffer commandBuffer, VkCoverageReductionModeNV coverageReductionMode) {
    device_dispatch_table.CmdSetCoverageReductionModeNV(commandBuffer, coverageReductionMode);
}

void DispatchObject::GetShaderModuleIdentifierEXT(VkDevice device, VkShaderModule shaderModule,
                                                  VkShaderModuleIdentifierEXT* pIdentifier) {
    if (!wrap_handles) return device_dispatch_table.GetShaderModuleIdentifierEXT(device, shaderModule, pIdentifier);
    { shaderModule = Unwrap(shaderModule); }
    device_dispatch_table.GetShaderModuleIdentifierEXT(device, shaderModule, pIdentifier);
}

void DispatchObject::GetShaderModuleCreateInfoIdentifierEXT(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo,
                                                            VkShaderModuleIdentifierEXT* pIdentifier) {
    if (!wrap_handles) return device_dispatch_table.GetShaderModuleCreateInfoIdentifierEXT(device, pCreateInfo, pIdentifier);
    vku::safe_VkShaderModuleCreateInfo var_local_pCreateInfo;
    vku::safe_VkShaderModuleCreateInfo* local_pCreateInfo = nullptr;
    {
        if (pCreateInfo) {
            local_pCreateInfo = &var_local_pCreateInfo;
            local_pCreateInfo->initialize(pCreateInfo);
            UnwrapPnextChainHandles(local_pCreateInfo->pNext);
        }
    }
    device_dispatch_table.GetShaderModuleCreateInfoIdentifierEXT(device, (const VkShaderModuleCreateInfo*)local_pCreateInfo,
                                                                 pIdentifier);
}

VkResult DispatchObject::GetPhysicalDeviceOpticalFlowImageFormatsNV(
    VkPhysicalDevice physicalDevice, const VkOpticalFlowImageFormatInfoNV* pOpticalFlowImageFormatInfo, uint32_t* pFormatCount,
    VkOpticalFlowImageFormatPropertiesNV* pImageFormatProperties) {
    VkResult result = instance_dispatch_table.GetPhysicalDeviceOpticalFlowImageFormatsNV(
        physicalDevice, pOpticalFlowImageFormatInfo, pFormatCount, pImageFormatProperties);

    return result;
}

VkResult DispatchObject::CreateOpticalFlowSessionNV(VkDevice device, const VkOpticalFlowSessionCreateInfoNV* pCreateInfo,
                                                    const VkAllocationCallbacks* pAllocator, VkOpticalFlowSessionNV* pSession) {
    if (!wrap_handles) return device_dispatch_table.CreateOpticalFlowSessionNV(device, pCreateInfo, pAllocator, pSession);

    VkResult result = device_dispatch_table.CreateOpticalFlowSessionNV(device, pCreateInfo, pAllocator, pSession);
    if (VK_SUCCESS == result) {
        *pSession = WrapNew(*pSession);
    }
    return result;
}

void DispatchObject::DestroyOpticalFlowSessionNV(VkDevice device, VkOpticalFlowSessionNV session,
                                                 const VkAllocationCallbacks* pAllocator) {
    if (!wrap_handles) return device_dispatch_table.DestroyOpticalFlowSessionNV(device, session, pAllocator);

    uint64_t session_id = CastToUint64(session);
    auto iter = unique_id_mapping.pop(session_id);
    if (iter != unique_id_mapping.end()) {
        session = (VkOpticalFlowSessionNV)iter->second;
    } else {
        session = (VkOpticalFlowSessionNV)0;
    }
    device_dispatch_table.DestroyOpticalFlowSessionNV(device, session, pAllocator);
}

VkResult DispatchObject::BindOpticalFlowSessionImageNV(VkDevice device, VkOpticalFlowSessionNV session,
                                                       VkOpticalFlowSessionBindingPointNV bindingPoint, VkImageView view,
                                                       VkImageLayout layout) {
    if (!wrap_handles) return device_dispatch_table.BindOpticalFlowSessionImageNV(device, session, bindingPoint, view, layout);
    {
        session = Unwrap(session);
        view = Unwrap(view);
    }
    VkResult result = device_dispatch_table.BindOpticalFlowSessionImageNV(device, session, bindingPoint, view, layout);

    return result;
}

void DispatchObject::CmdOpticalFlowExecuteNV(VkCommandBuffer commandBuffer, VkOpticalFlowSessionNV session,
                                             const VkOpticalFlowExecuteInfoNV* pExecuteInfo) {
    if (!wrap_handles) return device_dispatch_table.CmdOpticalFlowExecuteNV(commandBuffer, session, pExecuteInfo);
    { session = Unwrap(session); }
    device_dispatch_table.CmdOpticalFlowExecuteNV(commandBuffer, session, pExecuteInfo);
}

void DispatchObject::AntiLagUpdateAMD(VkDevice device, const VkAntiLagDataAMD* pData) {
    device_dispatch_table.AntiLagUpdateAMD(device, pData);
}

VkResult DispatchObject::CreateShadersEXT(VkDevice device, uint32_t createInfoCount, const VkShaderCreateInfoEXT* pCreateInfos,
                                          const VkAllocationCallbacks* pAllocator, VkShaderEXT* pShaders) {
    if (!wrap_handles) return device_dispatch_table.CreateShadersEXT(device, createInfoCount, pCreateInfos, pAllocator, pShaders);
    small_vector<vku::safe_VkShaderCreateInfoEXT, DISPATCH_MAX_STACK_ALLOCATIONS> var_local_pCreateInfos;
    vku::safe_VkShaderCreateInfoEXT* local_pCreateInfos = nullptr;
    {
        if (pCreateInfos) {
            var_local_pCreateInfos.resize(createInfoCount);
            local_pCreateInfos = var_local_pCreateInfos.data();
            for (uint32_t index0 = 0; index0 < createInfoCount; ++index0) {
                local_pCreateInfos[index0].initialize(&pCreateInfos[index0]);
                if (local_pCreateInfos[index0].pSetLayouts) {
                    for (uint32_t index1 = 0; index1 < local_pCreateInfos[index0].setLayoutCount; ++index1) {
                        local_pCreateInfos[index0].pSetLayouts[index1] = Unwrap(local_pCreateInfos[index0].pSetLayouts[index1]);
                    }
                }
            }
        }
    }
    VkResult result = device_dispatch_table.CreateShadersEXT(
        device, createInfoCount, (const VkShaderCreateInfoEXT*)local_pCreateInfos, pAllocator, pShaders);
    if (VK_SUCCESS == result) {
        for (uint32_t index0 = 0; index0 < createInfoCount; index0++) {
            pShaders[index0] = WrapNew(pShaders[index0]);
        }
    }
    return result;
}

void DispatchObject::DestroyShaderEXT(VkDevice device, VkShaderEXT shader, const VkAllocationCallbacks* pAllocator) {
    if (!wrap_handles) return device_dispatch_table.DestroyShaderEXT(device, shader, pAllocator);

    uint64_t shader_id = CastToUint64(shader);
    auto iter = unique_id_mapping.pop(shader_id);
    if (iter != unique_id_mapping.end()) {
        shader = (VkShaderEXT)iter->second;
    } else {
        shader = (VkShaderEXT)0;
    }
    device_dispatch_table.DestroyShaderEXT(device, shader, pAllocator);
}

VkResult DispatchObject::GetShaderBinaryDataEXT(VkDevice device, VkShaderEXT shader, size_t* pDataSize, void* pData) {
    if (!wrap_handles) return device_dispatch_table.GetShaderBinaryDataEXT(device, shader, pDataSize, pData);
    { shader = Unwrap(shader); }
    VkResult result = device_dispatch_table.GetShaderBinaryDataEXT(device, shader, pDataSize, pData);

    return result;
}

void DispatchObject::CmdBindShadersEXT(VkCommandBuffer commandBuffer, uint32_t stageCount, const VkShaderStageFlagBits* pStages,
                                       const VkShaderEXT* pShaders) {
    if (!wrap_handles) return device_dispatch_table.CmdBindShadersEXT(commandBuffer, stageCount, pStages, pShaders);
    small_vector<VkShaderEXT, DISPATCH_MAX_STACK_ALLOCATIONS> var_local_pShaders;
    VkShaderEXT* local_pShaders = nullptr;
    {
        if (pShaders) {
            var_local_pShaders.resize(stageCount);
            local_pShaders = var_local_pShaders.data();
            for (uint32_t index0 = 0; index0 < stageCount; ++index0) {
                local_pShaders[index0] = Unwrap(pShaders[index0]);
            }
        }
    }
    device_dispatch_table.CmdBindShadersEXT(commandBuffer, stageCount, pStages, (const VkShaderEXT*)local_pShaders);
}

void DispatchObject::CmdSetDepthClampRangeEXT(VkCommandBuffer commandBuffer, VkDepthClampModeEXT depthClampMode,
                                              const VkDepthClampRangeEXT* pDepthClampRange) {
    device_dispatch_table.CmdSetDepthClampRangeEXT(commandBuffer, depthClampMode, pDepthClampRange);
}

VkResult DispatchObject::GetFramebufferTilePropertiesQCOM(VkDevice device, VkFramebuffer framebuffer, uint32_t* pPropertiesCount,
                                                          VkTilePropertiesQCOM* pProperties) {
    if (!wrap_handles)
        return device_dispatch_table.GetFramebufferTilePropertiesQCOM(device, framebuffer, pPropertiesCount, pProperties);
    { framebuffer = Unwrap(framebuffer); }
    VkResult result = device_dispatch_table.GetFramebufferTilePropertiesQCOM(device, framebuffer, pPropertiesCount, pProperties);

    return result;
}

VkResult DispatchObject::GetDynamicRenderingTilePropertiesQCOM(VkDevice device, const VkRenderingInfo* pRenderingInfo,
                                                               VkTilePropertiesQCOM* pProperties) {
    if (!wrap_handles) return device_dispatch_table.GetDynamicRenderingTilePropertiesQCOM(device, pRenderingInfo, pProperties);
    vku::safe_VkRenderingInfo var_local_pRenderingInfo;
    vku::safe_VkRenderingInfo* local_pRenderingInfo = nullptr;
    {
        if (pRenderingInfo) {
            local_pRenderingInfo = &var_local_pRenderingInfo;
            local_pRenderingInfo->initialize(pRenderingInfo);
            if (local_pRenderingInfo->pColorAttachments) {
                for (uint32_t index1 = 0; index1 < local_pRenderingInfo->colorAttachmentCount; ++index1) {
                    if (pRenderingInfo->pColorAttachments[index1].imageView) {
                        local_pRenderingInfo->pColorAttachments[index1].imageView =
                            Unwrap(pRenderingInfo->pColorAttachments[index1].imageView);
                    }
                    if (pRenderingInfo->pColorAttachments[index1].resolveImageView) {
                        local_pRenderingInfo->pColorAttachments[index1].resolveImageView =
                            Unwrap(pRenderingInfo->pColorAttachments[index1].resolveImageView);
                    }
                }
            }
            if (local_pRenderingInfo->pDepthAttachment) {
                if (pRenderingInfo->pDepthAttachment->imageView) {
                    local_pRenderingInfo->pDepthAttachment->imageView = Unwrap(pRenderingInfo->pDepthAttachment->imageView);
                }
                if (pRenderingInfo->pDepthAttachment->resolveImageView) {
                    local_pRenderingInfo->pDepthAttachment->resolveImageView =
                        Unwrap(pRenderingInfo->pDepthAttachment->resolveImageView);
                }
            }
            if (local_pRenderingInfo->pStencilAttachment) {
                if (pRenderingInfo->pStencilAttachment->imageView) {
                    local_pRenderingInfo->pStencilAttachment->imageView = Unwrap(pRenderingInfo->pStencilAttachment->imageView);
                }
                if (pRenderingInfo->pStencilAttachment->resolveImageView) {
                    local_pRenderingInfo->pStencilAttachment->resolveImageView =
                        Unwrap(pRenderingInfo->pStencilAttachment->resolveImageView);
                }
            }
            UnwrapPnextChainHandles(local_pRenderingInfo->pNext);
        }
    }
    VkResult result = device_dispatch_table.GetDynamicRenderingTilePropertiesQCOM(
        device, (const VkRenderingInfo*)local_pRenderingInfo, pProperties);

    return result;
}

VkResult DispatchObject::SetLatencySleepModeNV(VkDevice device, VkSwapchainKHR swapchain,
                                               const VkLatencySleepModeInfoNV* pSleepModeInfo) {
    if (!wrap_handles) return device_dispatch_table.SetLatencySleepModeNV(device, swapchain, pSleepModeInfo);
    { swapchain = Unwrap(swapchain); }
    VkResult result = device_dispatch_table.SetLatencySleepModeNV(device, swapchain, pSleepModeInfo);

    return result;
}

VkResult DispatchObject::LatencySleepNV(VkDevice device, VkSwapchainKHR swapchain, const VkLatencySleepInfoNV* pSleepInfo) {
    if (!wrap_handles) return device_dispatch_table.LatencySleepNV(device, swapchain, pSleepInfo);
    vku::safe_VkLatencySleepInfoNV var_local_pSleepInfo;
    vku::safe_VkLatencySleepInfoNV* local_pSleepInfo = nullptr;
    {
        swapchain = Unwrap(swapchain);
        if (pSleepInfo) {
            local_pSleepInfo = &var_local_pSleepInfo;
            local_pSleepInfo->initialize(pSleepInfo);

            if (pSleepInfo->signalSemaphore) {
                local_pSleepInfo->signalSemaphore = Unwrap(pSleepInfo->signalSemaphore);
            }
        }
    }
    VkResult result = device_dispatch_table.LatencySleepNV(device, swapchain, (const VkLatencySleepInfoNV*)local_pSleepInfo);

    return result;
}

void DispatchObject::SetLatencyMarkerNV(VkDevice device, VkSwapchainKHR swapchain,
                                        const VkSetLatencyMarkerInfoNV* pLatencyMarkerInfo) {
    if (!wrap_handles) return device_dispatch_table.SetLatencyMarkerNV(device, swapchain, pLatencyMarkerInfo);
    { swapchain = Unwrap(swapchain); }
    device_dispatch_table.SetLatencyMarkerNV(device, swapchain, pLatencyMarkerInfo);
}

void DispatchObject::GetLatencyTimingsNV(VkDevice device, VkSwapchainKHR swapchain, VkGetLatencyMarkerInfoNV* pLatencyMarkerInfo) {
    if (!wrap_handles) return device_dispatch_table.GetLatencyTimingsNV(device, swapchain, pLatencyMarkerInfo);
    { swapchain = Unwrap(swapchain); }
    device_dispatch_table.GetLatencyTimingsNV(device, swapchain, pLatencyMarkerInfo);
}

void DispatchObject::QueueNotifyOutOfBandNV(VkQueue queue, const VkOutOfBandQueueTypeInfoNV* pQueueTypeInfo) {
    device_dispatch_table.QueueNotifyOutOfBandNV(queue, pQueueTypeInfo);
}

void DispatchObject::CmdSetAttachmentFeedbackLoopEnableEXT(VkCommandBuffer commandBuffer, VkImageAspectFlags aspectMask) {
    device_dispatch_table.CmdSetAttachmentFeedbackLoopEnableEXT(commandBuffer, aspectMask);
}
#ifdef VK_USE_PLATFORM_SCREEN_QNX

VkResult DispatchObject::GetScreenBufferPropertiesQNX(VkDevice device, const struct _screen_buffer* buffer,
                                                      VkScreenBufferPropertiesQNX* pProperties) {
    VkResult result = device_dispatch_table.GetScreenBufferPropertiesQNX(device, buffer, pProperties);

    return result;
}
#endif  // VK_USE_PLATFORM_SCREEN_QNX

void DispatchObject::GetGeneratedCommandsMemoryRequirementsEXT(VkDevice device,
                                                               const VkGeneratedCommandsMemoryRequirementsInfoEXT* pInfo,
                                                               VkMemoryRequirements2* pMemoryRequirements) {
    if (!wrap_handles) return device_dispatch_table.GetGeneratedCommandsMemoryRequirementsEXT(device, pInfo, pMemoryRequirements);
    vku::safe_VkGeneratedCommandsMemoryRequirementsInfoEXT var_local_pInfo;
    vku::safe_VkGeneratedCommandsMemoryRequirementsInfoEXT* local_pInfo = nullptr;
    {
        if (pInfo) {
            local_pInfo = &var_local_pInfo;
            local_pInfo->initialize(pInfo);

            if (pInfo->indirectExecutionSet) {
                local_pInfo->indirectExecutionSet = Unwrap(pInfo->indirectExecutionSet);
            }
            if (pInfo->indirectCommandsLayout) {
                local_pInfo->indirectCommandsLayout = Unwrap(pInfo->indirectCommandsLayout);
            }
            UnwrapPnextChainHandles(local_pInfo->pNext);
        }
    }
    device_dispatch_table.GetGeneratedCommandsMemoryRequirementsEXT(
        device, (const VkGeneratedCommandsMemoryRequirementsInfoEXT*)local_pInfo, pMemoryRequirements);
}

void DispatchObject::CmdPreprocessGeneratedCommandsEXT(VkCommandBuffer commandBuffer,
                                                       const VkGeneratedCommandsInfoEXT* pGeneratedCommandsInfo,
                                                       VkCommandBuffer stateCommandBuffer) {
    if (!wrap_handles)
        return device_dispatch_table.CmdPreprocessGeneratedCommandsEXT(commandBuffer, pGeneratedCommandsInfo, stateCommandBuffer);
    vku::safe_VkGeneratedCommandsInfoEXT var_local_pGeneratedCommandsInfo;
    vku::safe_VkGeneratedCommandsInfoEXT* local_pGeneratedCommandsInfo = nullptr;
    {
        if (pGeneratedCommandsInfo) {
            local_pGeneratedCommandsInfo = &var_local_pGeneratedCommandsInfo;
            local_pGeneratedCommandsInfo->initialize(pGeneratedCommandsInfo);

            if (pGeneratedCommandsInfo->indirectExecutionSet) {
                local_pGeneratedCommandsInfo->indirectExecutionSet = Unwrap(pGeneratedCommandsInfo->indirectExecutionSet);
            }
            if (pGeneratedCommandsInfo->indirectCommandsLayout) {
                local_pGeneratedCommandsInfo->indirectCommandsLayout = Unwrap(pGeneratedCommandsInfo->indirectCommandsLayout);
            }
            UnwrapPnextChainHandles(local_pGeneratedCommandsInfo->pNext);
        }
    }
    device_dispatch_table.CmdPreprocessGeneratedCommandsEXT(
        commandBuffer, (const VkGeneratedCommandsInfoEXT*)local_pGeneratedCommandsInfo, stateCommandBuffer);
}

void DispatchObject::CmdExecuteGeneratedCommandsEXT(VkCommandBuffer commandBuffer, VkBool32 isPreprocessed,
                                                    const VkGeneratedCommandsInfoEXT* pGeneratedCommandsInfo) {
    if (!wrap_handles)
        return device_dispatch_table.CmdExecuteGeneratedCommandsEXT(commandBuffer, isPreprocessed, pGeneratedCommandsInfo);
    vku::safe_VkGeneratedCommandsInfoEXT var_local_pGeneratedCommandsInfo;
    vku::safe_VkGeneratedCommandsInfoEXT* local_pGeneratedCommandsInfo = nullptr;
    {
        if (pGeneratedCommandsInfo) {
            local_pGeneratedCommandsInfo = &var_local_pGeneratedCommandsInfo;
            local_pGeneratedCommandsInfo->initialize(pGeneratedCommandsInfo);

            if (pGeneratedCommandsInfo->indirectExecutionSet) {
                local_pGeneratedCommandsInfo->indirectExecutionSet = Unwrap(pGeneratedCommandsInfo->indirectExecutionSet);
            }
            if (pGeneratedCommandsInfo->indirectCommandsLayout) {
                local_pGeneratedCommandsInfo->indirectCommandsLayout = Unwrap(pGeneratedCommandsInfo->indirectCommandsLayout);
            }
            UnwrapPnextChainHandles(local_pGeneratedCommandsInfo->pNext);
        }
    }
    device_dispatch_table.CmdExecuteGeneratedCommandsEXT(commandBuffer, isPreprocessed,
                                                         (const VkGeneratedCommandsInfoEXT*)local_pGeneratedCommandsInfo);
}

VkResult DispatchObject::CreateIndirectCommandsLayoutEXT(VkDevice device, const VkIndirectCommandsLayoutCreateInfoEXT* pCreateInfo,
                                                         const VkAllocationCallbacks* pAllocator,
                                                         VkIndirectCommandsLayoutEXT* pIndirectCommandsLayout) {
    if (!wrap_handles)
        return device_dispatch_table.CreateIndirectCommandsLayoutEXT(device, pCreateInfo, pAllocator, pIndirectCommandsLayout);
    vku::safe_VkIndirectCommandsLayoutCreateInfoEXT var_local_pCreateInfo;
    vku::safe_VkIndirectCommandsLayoutCreateInfoEXT* local_pCreateInfo = nullptr;
    {
        if (pCreateInfo) {
            local_pCreateInfo = &var_local_pCreateInfo;
            local_pCreateInfo->initialize(pCreateInfo);

            if (pCreateInfo->pipelineLayout) {
                local_pCreateInfo->pipelineLayout = Unwrap(pCreateInfo->pipelineLayout);
            }
            UnwrapPnextChainHandles(local_pCreateInfo->pNext);
        }
    }
    VkResult result = device_dispatch_table.CreateIndirectCommandsLayoutEXT(
        device, (const VkIndirectCommandsLayoutCreateInfoEXT*)local_pCreateInfo, pAllocator, pIndirectCommandsLayout);
    if (VK_SUCCESS == result) {
        *pIndirectCommandsLayout = WrapNew(*pIndirectCommandsLayout);
    }
    return result;
}

void DispatchObject::DestroyIndirectCommandsLayoutEXT(VkDevice device, VkIndirectCommandsLayoutEXT indirectCommandsLayout,
                                                      const VkAllocationCallbacks* pAllocator) {
    if (!wrap_handles) return device_dispatch_table.DestroyIndirectCommandsLayoutEXT(device, indirectCommandsLayout, pAllocator);

    uint64_t indirectCommandsLayout_id = CastToUint64(indirectCommandsLayout);
    auto iter = unique_id_mapping.pop(indirectCommandsLayout_id);
    if (iter != unique_id_mapping.end()) {
        indirectCommandsLayout = (VkIndirectCommandsLayoutEXT)iter->second;
    } else {
        indirectCommandsLayout = (VkIndirectCommandsLayoutEXT)0;
    }
    device_dispatch_table.DestroyIndirectCommandsLayoutEXT(device, indirectCommandsLayout, pAllocator);
}

void DispatchObject::DestroyIndirectExecutionSetEXT(VkDevice device, VkIndirectExecutionSetEXT indirectExecutionSet,
                                                    const VkAllocationCallbacks* pAllocator) {
    if (!wrap_handles) return device_dispatch_table.DestroyIndirectExecutionSetEXT(device, indirectExecutionSet, pAllocator);

    uint64_t indirectExecutionSet_id = CastToUint64(indirectExecutionSet);
    auto iter = unique_id_mapping.pop(indirectExecutionSet_id);
    if (iter != unique_id_mapping.end()) {
        indirectExecutionSet = (VkIndirectExecutionSetEXT)iter->second;
    } else {
        indirectExecutionSet = (VkIndirectExecutionSetEXT)0;
    }
    device_dispatch_table.DestroyIndirectExecutionSetEXT(device, indirectExecutionSet, pAllocator);
}

void DispatchObject::UpdateIndirectExecutionSetPipelineEXT(VkDevice device, VkIndirectExecutionSetEXT indirectExecutionSet,
                                                           uint32_t executionSetWriteCount,
                                                           const VkWriteIndirectExecutionSetPipelineEXT* pExecutionSetWrites) {
    if (!wrap_handles)
        return device_dispatch_table.UpdateIndirectExecutionSetPipelineEXT(device, indirectExecutionSet, executionSetWriteCount,
                                                                           pExecutionSetWrites);
    small_vector<vku::safe_VkWriteIndirectExecutionSetPipelineEXT, DISPATCH_MAX_STACK_ALLOCATIONS> var_local_pExecutionSetWrites;
    vku::safe_VkWriteIndirectExecutionSetPipelineEXT* local_pExecutionSetWrites = nullptr;
    {
        indirectExecutionSet = Unwrap(indirectExecutionSet);
        if (pExecutionSetWrites) {
            var_local_pExecutionSetWrites.resize(executionSetWriteCount);
            local_pExecutionSetWrites = var_local_pExecutionSetWrites.data();
            for (uint32_t index0 = 0; index0 < executionSetWriteCount; ++index0) {
                local_pExecutionSetWrites[index0].initialize(&pExecutionSetWrites[index0]);

                if (pExecutionSetWrites[index0].pipeline) {
                    local_pExecutionSetWrites[index0].pipeline = Unwrap(pExecutionSetWrites[index0].pipeline);
                }
            }
        }
    }
    device_dispatch_table.UpdateIndirectExecutionSetPipelineEXT(
        device, indirectExecutionSet, executionSetWriteCount,
        (const VkWriteIndirectExecutionSetPipelineEXT*)local_pExecutionSetWrites);
}

void DispatchObject::UpdateIndirectExecutionSetShaderEXT(VkDevice device, VkIndirectExecutionSetEXT indirectExecutionSet,
                                                         uint32_t executionSetWriteCount,
                                                         const VkWriteIndirectExecutionSetShaderEXT* pExecutionSetWrites) {
    if (!wrap_handles)
        return device_dispatch_table.UpdateIndirectExecutionSetShaderEXT(device, indirectExecutionSet, executionSetWriteCount,
                                                                         pExecutionSetWrites);
    small_vector<vku::safe_VkWriteIndirectExecutionSetShaderEXT, DISPATCH_MAX_STACK_ALLOCATIONS> var_local_pExecutionSetWrites;
    vku::safe_VkWriteIndirectExecutionSetShaderEXT* local_pExecutionSetWrites = nullptr;
    {
        indirectExecutionSet = Unwrap(indirectExecutionSet);
        if (pExecutionSetWrites) {
            var_local_pExecutionSetWrites.resize(executionSetWriteCount);
            local_pExecutionSetWrites = var_local_pExecutionSetWrites.data();
            for (uint32_t index0 = 0; index0 < executionSetWriteCount; ++index0) {
                local_pExecutionSetWrites[index0].initialize(&pExecutionSetWrites[index0]);

                if (pExecutionSetWrites[index0].shader) {
                    local_pExecutionSetWrites[index0].shader = Unwrap(pExecutionSetWrites[index0].shader);
                }
            }
        }
    }
    device_dispatch_table.UpdateIndirectExecutionSetShaderEXT(
        device, indirectExecutionSet, executionSetWriteCount,
        (const VkWriteIndirectExecutionSetShaderEXT*)local_pExecutionSetWrites);
}

VkResult DispatchObject::GetPhysicalDeviceCooperativeMatrixFlexibleDimensionsPropertiesNV(
    VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount, VkCooperativeMatrixFlexibleDimensionsPropertiesNV* pProperties) {
    VkResult result = instance_dispatch_table.GetPhysicalDeviceCooperativeMatrixFlexibleDimensionsPropertiesNV(
        physicalDevice, pPropertyCount, pProperties);

    return result;
}

VkResult DispatchObject::CreateAccelerationStructureKHR(VkDevice device, const VkAccelerationStructureCreateInfoKHR* pCreateInfo,
                                                        const VkAllocationCallbacks* pAllocator,
                                                        VkAccelerationStructureKHR* pAccelerationStructure) {
    if (!wrap_handles)
        return device_dispatch_table.CreateAccelerationStructureKHR(device, pCreateInfo, pAllocator, pAccelerationStructure);
    vku::safe_VkAccelerationStructureCreateInfoKHR var_local_pCreateInfo;
    vku::safe_VkAccelerationStructureCreateInfoKHR* local_pCreateInfo = nullptr;
    {
        if (pCreateInfo) {
            local_pCreateInfo = &var_local_pCreateInfo;
            local_pCreateInfo->initialize(pCreateInfo);

            if (pCreateInfo->buffer) {
                local_pCreateInfo->buffer = Unwrap(pCreateInfo->buffer);
            }
        }
    }
    VkResult result = device_dispatch_table.CreateAccelerationStructureKHR(
        device, (const VkAccelerationStructureCreateInfoKHR*)local_pCreateInfo, pAllocator, pAccelerationStructure);
    if (VK_SUCCESS == result) {
        *pAccelerationStructure = WrapNew(*pAccelerationStructure);
    }
    return result;
}

void DispatchObject::DestroyAccelerationStructureKHR(VkDevice device, VkAccelerationStructureKHR accelerationStructure,
                                                     const VkAllocationCallbacks* pAllocator) {
    if (!wrap_handles) return device_dispatch_table.DestroyAccelerationStructureKHR(device, accelerationStructure, pAllocator);

    uint64_t accelerationStructure_id = CastToUint64(accelerationStructure);
    auto iter = unique_id_mapping.pop(accelerationStructure_id);
    if (iter != unique_id_mapping.end()) {
        accelerationStructure = (VkAccelerationStructureKHR)iter->second;
    } else {
        accelerationStructure = (VkAccelerationStructureKHR)0;
    }
    device_dispatch_table.DestroyAccelerationStructureKHR(device, accelerationStructure, pAllocator);
}

void DispatchObject::CmdBuildAccelerationStructuresIndirectKHR(VkCommandBuffer commandBuffer, uint32_t infoCount,
                                                               const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
                                                               const VkDeviceAddress* pIndirectDeviceAddresses,
                                                               const uint32_t* pIndirectStrides,
                                                               const uint32_t* const* ppMaxPrimitiveCounts) {
    if (!wrap_handles)
        return device_dispatch_table.CmdBuildAccelerationStructuresIndirectKHR(
            commandBuffer, infoCount, pInfos, pIndirectDeviceAddresses, pIndirectStrides, ppMaxPrimitiveCounts);
    small_vector<vku::safe_VkAccelerationStructureBuildGeometryInfoKHR, DISPATCH_MAX_STACK_ALLOCATIONS> var_local_pInfos;
    vku::safe_VkAccelerationStructureBuildGeometryInfoKHR* local_pInfos = nullptr;
    {
        if (pInfos) {
            var_local_pInfos.resize(infoCount);
            local_pInfos = var_local_pInfos.data();
            for (uint32_t index0 = 0; index0 < infoCount; ++index0) {
                local_pInfos[index0].initialize(&pInfos[index0], false, nullptr);

                if (pInfos[index0].srcAccelerationStructure) {
                    local_pInfos[index0].srcAccelerationStructure = Unwrap(pInfos[index0].srcAccelerationStructure);
                }
                if (pInfos[index0].dstAccelerationStructure) {
                    local_pInfos[index0].dstAccelerationStructure = Unwrap(pInfos[index0].dstAccelerationStructure);
                }
            }
        }
    }
    device_dispatch_table.CmdBuildAccelerationStructuresIndirectKHR(
        commandBuffer, infoCount, (const VkAccelerationStructureBuildGeometryInfoKHR*)local_pInfos, pIndirectDeviceAddresses,
        pIndirectStrides, ppMaxPrimitiveCounts);
}

VkResult DispatchObject::CopyAccelerationStructureKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                      const VkCopyAccelerationStructureInfoKHR* pInfo) {
    if (!wrap_handles) return device_dispatch_table.CopyAccelerationStructureKHR(device, deferredOperation, pInfo);
    vku::safe_VkCopyAccelerationStructureInfoKHR* local_pInfo = nullptr;
    {
        deferredOperation = Unwrap(deferredOperation);
        if (pInfo) {
            local_pInfo = new vku::safe_VkCopyAccelerationStructureInfoKHR;
            local_pInfo->initialize(pInfo);

            if (pInfo->src) {
                local_pInfo->src = Unwrap(pInfo->src);
            }
            if (pInfo->dst) {
                local_pInfo->dst = Unwrap(pInfo->dst);
            }
        }
    }
    VkResult result = device_dispatch_table.CopyAccelerationStructureKHR(device, deferredOperation,
                                                                         (const VkCopyAccelerationStructureInfoKHR*)local_pInfo);
    if (local_pInfo) {
        // Fix check for deferred ray tracing pipeline creation
        // https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/5817
        const bool is_operation_deferred = (deferredOperation != VK_NULL_HANDLE) && (result == VK_OPERATION_DEFERRED_KHR);
        if (is_operation_deferred) {
            std::vector<std::function<void()>> cleanup{[local_pInfo]() { delete local_pInfo; }};
            deferred_operation_post_completion.insert(deferredOperation, cleanup);
        } else {
            delete local_pInfo;
        }
    }
    return result;
}

VkResult DispatchObject::CopyAccelerationStructureToMemoryKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                              const VkCopyAccelerationStructureToMemoryInfoKHR* pInfo) {
    if (!wrap_handles) return device_dispatch_table.CopyAccelerationStructureToMemoryKHR(device, deferredOperation, pInfo);
    vku::safe_VkCopyAccelerationStructureToMemoryInfoKHR* local_pInfo = nullptr;
    {
        deferredOperation = Unwrap(deferredOperation);
        if (pInfo) {
            local_pInfo = new vku::safe_VkCopyAccelerationStructureToMemoryInfoKHR;
            local_pInfo->initialize(pInfo);

            if (pInfo->src) {
                local_pInfo->src = Unwrap(pInfo->src);
            }
        }
    }
    VkResult result = device_dispatch_table.CopyAccelerationStructureToMemoryKHR(
        device, deferredOperation, (const VkCopyAccelerationStructureToMemoryInfoKHR*)local_pInfo);
    if (local_pInfo) {
        // Fix check for deferred ray tracing pipeline creation
        // https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/5817
        const bool is_operation_deferred = (deferredOperation != VK_NULL_HANDLE) && (result == VK_OPERATION_DEFERRED_KHR);
        if (is_operation_deferred) {
            std::vector<std::function<void()>> cleanup{[local_pInfo]() { delete local_pInfo; }};
            deferred_operation_post_completion.insert(deferredOperation, cleanup);
        } else {
            delete local_pInfo;
        }
    }
    return result;
}

VkResult DispatchObject::CopyMemoryToAccelerationStructureKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                              const VkCopyMemoryToAccelerationStructureInfoKHR* pInfo) {
    if (!wrap_handles) return device_dispatch_table.CopyMemoryToAccelerationStructureKHR(device, deferredOperation, pInfo);
    vku::safe_VkCopyMemoryToAccelerationStructureInfoKHR* local_pInfo = nullptr;
    {
        deferredOperation = Unwrap(deferredOperation);
        if (pInfo) {
            local_pInfo = new vku::safe_VkCopyMemoryToAccelerationStructureInfoKHR;
            local_pInfo->initialize(pInfo);

            if (pInfo->dst) {
                local_pInfo->dst = Unwrap(pInfo->dst);
            }
        }
    }
    VkResult result = device_dispatch_table.CopyMemoryToAccelerationStructureKHR(
        device, deferredOperation, (const VkCopyMemoryToAccelerationStructureInfoKHR*)local_pInfo);
    if (local_pInfo) {
        // Fix check for deferred ray tracing pipeline creation
        // https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/5817
        const bool is_operation_deferred = (deferredOperation != VK_NULL_HANDLE) && (result == VK_OPERATION_DEFERRED_KHR);
        if (is_operation_deferred) {
            std::vector<std::function<void()>> cleanup{[local_pInfo]() { delete local_pInfo; }};
            deferred_operation_post_completion.insert(deferredOperation, cleanup);
        } else {
            delete local_pInfo;
        }
    }
    return result;
}

VkResult DispatchObject::WriteAccelerationStructuresPropertiesKHR(VkDevice device, uint32_t accelerationStructureCount,
                                                                  const VkAccelerationStructureKHR* pAccelerationStructures,
                                                                  VkQueryType queryType, size_t dataSize, void* pData,
                                                                  size_t stride) {
    if (!wrap_handles)
        return device_dispatch_table.WriteAccelerationStructuresPropertiesKHR(
            device, accelerationStructureCount, pAccelerationStructures, queryType, dataSize, pData, stride);
    small_vector<VkAccelerationStructureKHR, DISPATCH_MAX_STACK_ALLOCATIONS> var_local_pAccelerationStructures;
    VkAccelerationStructureKHR* local_pAccelerationStructures = nullptr;
    {
        if (pAccelerationStructures) {
            var_local_pAccelerationStructures.resize(accelerationStructureCount);
            local_pAccelerationStructures = var_local_pAccelerationStructures.data();
            for (uint32_t index0 = 0; index0 < accelerationStructureCount; ++index0) {
                local_pAccelerationStructures[index0] = Unwrap(pAccelerationStructures[index0]);
            }
        }
    }
    VkResult result = device_dispatch_table.WriteAccelerationStructuresPropertiesKHR(
        device, accelerationStructureCount, (const VkAccelerationStructureKHR*)local_pAccelerationStructures, queryType, dataSize,
        pData, stride);

    return result;
}

void DispatchObject::CmdCopyAccelerationStructureKHR(VkCommandBuffer commandBuffer,
                                                     const VkCopyAccelerationStructureInfoKHR* pInfo) {
    if (!wrap_handles) return device_dispatch_table.CmdCopyAccelerationStructureKHR(commandBuffer, pInfo);
    vku::safe_VkCopyAccelerationStructureInfoKHR var_local_pInfo;
    vku::safe_VkCopyAccelerationStructureInfoKHR* local_pInfo = nullptr;
    {
        if (pInfo) {
            local_pInfo = &var_local_pInfo;
            local_pInfo->initialize(pInfo);

            if (pInfo->src) {
                local_pInfo->src = Unwrap(pInfo->src);
            }
            if (pInfo->dst) {
                local_pInfo->dst = Unwrap(pInfo->dst);
            }
        }
    }
    device_dispatch_table.CmdCopyAccelerationStructureKHR(commandBuffer, (const VkCopyAccelerationStructureInfoKHR*)local_pInfo);
}

void DispatchObject::CmdCopyAccelerationStructureToMemoryKHR(VkCommandBuffer commandBuffer,
                                                             const VkCopyAccelerationStructureToMemoryInfoKHR* pInfo) {
    if (!wrap_handles) return device_dispatch_table.CmdCopyAccelerationStructureToMemoryKHR(commandBuffer, pInfo);
    vku::safe_VkCopyAccelerationStructureToMemoryInfoKHR var_local_pInfo;
    vku::safe_VkCopyAccelerationStructureToMemoryInfoKHR* local_pInfo = nullptr;
    {
        if (pInfo) {
            local_pInfo = &var_local_pInfo;
            local_pInfo->initialize(pInfo);

            if (pInfo->src) {
                local_pInfo->src = Unwrap(pInfo->src);
            }
        }
    }
    device_dispatch_table.CmdCopyAccelerationStructureToMemoryKHR(commandBuffer,
                                                                  (const VkCopyAccelerationStructureToMemoryInfoKHR*)local_pInfo);
}

void DispatchObject::CmdCopyMemoryToAccelerationStructureKHR(VkCommandBuffer commandBuffer,
                                                             const VkCopyMemoryToAccelerationStructureInfoKHR* pInfo) {
    if (!wrap_handles) return device_dispatch_table.CmdCopyMemoryToAccelerationStructureKHR(commandBuffer, pInfo);
    vku::safe_VkCopyMemoryToAccelerationStructureInfoKHR var_local_pInfo;
    vku::safe_VkCopyMemoryToAccelerationStructureInfoKHR* local_pInfo = nullptr;
    {
        if (pInfo) {
            local_pInfo = &var_local_pInfo;
            local_pInfo->initialize(pInfo);

            if (pInfo->dst) {
                local_pInfo->dst = Unwrap(pInfo->dst);
            }
        }
    }
    device_dispatch_table.CmdCopyMemoryToAccelerationStructureKHR(commandBuffer,
                                                                  (const VkCopyMemoryToAccelerationStructureInfoKHR*)local_pInfo);
}

VkDeviceAddress DispatchObject::GetAccelerationStructureDeviceAddressKHR(VkDevice device,
                                                                         const VkAccelerationStructureDeviceAddressInfoKHR* pInfo) {
    if (!wrap_handles) return device_dispatch_table.GetAccelerationStructureDeviceAddressKHR(device, pInfo);
    vku::safe_VkAccelerationStructureDeviceAddressInfoKHR var_local_pInfo;
    vku::safe_VkAccelerationStructureDeviceAddressInfoKHR* local_pInfo = nullptr;
    {
        if (pInfo) {
            local_pInfo = &var_local_pInfo;
            local_pInfo->initialize(pInfo);

            if (pInfo->accelerationStructure) {
                local_pInfo->accelerationStructure = Unwrap(pInfo->accelerationStructure);
            }
        }
    }
    VkDeviceAddress result = device_dispatch_table.GetAccelerationStructureDeviceAddressKHR(
        device, (const VkAccelerationStructureDeviceAddressInfoKHR*)local_pInfo);

    return result;
}

void DispatchObject::CmdWriteAccelerationStructuresPropertiesKHR(VkCommandBuffer commandBuffer, uint32_t accelerationStructureCount,
                                                                 const VkAccelerationStructureKHR* pAccelerationStructures,
                                                                 VkQueryType queryType, VkQueryPool queryPool,
                                                                 uint32_t firstQuery) {
    if (!wrap_handles)
        return device_dispatch_table.CmdWriteAccelerationStructuresPropertiesKHR(
            commandBuffer, accelerationStructureCount, pAccelerationStructures, queryType, queryPool, firstQuery);
    small_vector<VkAccelerationStructureKHR, DISPATCH_MAX_STACK_ALLOCATIONS> var_local_pAccelerationStructures;
    VkAccelerationStructureKHR* local_pAccelerationStructures = nullptr;
    {
        if (pAccelerationStructures) {
            var_local_pAccelerationStructures.resize(accelerationStructureCount);
            local_pAccelerationStructures = var_local_pAccelerationStructures.data();
            for (uint32_t index0 = 0; index0 < accelerationStructureCount; ++index0) {
                local_pAccelerationStructures[index0] = Unwrap(pAccelerationStructures[index0]);
            }
        }
        queryPool = Unwrap(queryPool);
    }
    device_dispatch_table.CmdWriteAccelerationStructuresPropertiesKHR(
        commandBuffer, accelerationStructureCount, (const VkAccelerationStructureKHR*)local_pAccelerationStructures, queryType,
        queryPool, firstQuery);
}

void DispatchObject::GetDeviceAccelerationStructureCompatibilityKHR(VkDevice device,
                                                                    const VkAccelerationStructureVersionInfoKHR* pVersionInfo,
                                                                    VkAccelerationStructureCompatibilityKHR* pCompatibility) {
    device_dispatch_table.GetDeviceAccelerationStructureCompatibilityKHR(device, pVersionInfo, pCompatibility);
}

void DispatchObject::CmdTraceRaysKHR(VkCommandBuffer commandBuffer,
                                     const VkStridedDeviceAddressRegionKHR* pRaygenShaderBindingTable,
                                     const VkStridedDeviceAddressRegionKHR* pMissShaderBindingTable,
                                     const VkStridedDeviceAddressRegionKHR* pHitShaderBindingTable,
                                     const VkStridedDeviceAddressRegionKHR* pCallableShaderBindingTable, uint32_t width,
                                     uint32_t height, uint32_t depth) {
    device_dispatch_table.CmdTraceRaysKHR(commandBuffer, pRaygenShaderBindingTable, pMissShaderBindingTable, pHitShaderBindingTable,
                                          pCallableShaderBindingTable, width, height, depth);
}

VkResult DispatchObject::GetRayTracingCaptureReplayShaderGroupHandlesKHR(VkDevice device, VkPipeline pipeline, uint32_t firstGroup,
                                                                         uint32_t groupCount, size_t dataSize, void* pData) {
    if (!wrap_handles)
        return device_dispatch_table.GetRayTracingCaptureReplayShaderGroupHandlesKHR(device, pipeline, firstGroup, groupCount,
                                                                                     dataSize, pData);
    { pipeline = Unwrap(pipeline); }
    VkResult result = device_dispatch_table.GetRayTracingCaptureReplayShaderGroupHandlesKHR(device, pipeline, firstGroup,
                                                                                            groupCount, dataSize, pData);

    return result;
}

void DispatchObject::CmdTraceRaysIndirectKHR(VkCommandBuffer commandBuffer,
                                             const VkStridedDeviceAddressRegionKHR* pRaygenShaderBindingTable,
                                             const VkStridedDeviceAddressRegionKHR* pMissShaderBindingTable,
                                             const VkStridedDeviceAddressRegionKHR* pHitShaderBindingTable,
                                             const VkStridedDeviceAddressRegionKHR* pCallableShaderBindingTable,
                                             VkDeviceAddress indirectDeviceAddress) {
    device_dispatch_table.CmdTraceRaysIndirectKHR(commandBuffer, pRaygenShaderBindingTable, pMissShaderBindingTable,
                                                  pHitShaderBindingTable, pCallableShaderBindingTable, indirectDeviceAddress);
}

VkDeviceSize DispatchObject::GetRayTracingShaderGroupStackSizeKHR(VkDevice device, VkPipeline pipeline, uint32_t group,
                                                                  VkShaderGroupShaderKHR groupShader) {
    if (!wrap_handles) return device_dispatch_table.GetRayTracingShaderGroupStackSizeKHR(device, pipeline, group, groupShader);
    { pipeline = Unwrap(pipeline); }
    VkDeviceSize result = device_dispatch_table.GetRayTracingShaderGroupStackSizeKHR(device, pipeline, group, groupShader);

    return result;
}

void DispatchObject::CmdSetRayTracingPipelineStackSizeKHR(VkCommandBuffer commandBuffer, uint32_t pipelineStackSize) {
    device_dispatch_table.CmdSetRayTracingPipelineStackSizeKHR(commandBuffer, pipelineStackSize);
}

void DispatchObject::CmdDrawMeshTasksEXT(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY,
                                         uint32_t groupCountZ) {
    device_dispatch_table.CmdDrawMeshTasksEXT(commandBuffer, groupCountX, groupCountY, groupCountZ);
}

void DispatchObject::CmdDrawMeshTasksIndirectEXT(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                 uint32_t drawCount, uint32_t stride) {
    if (!wrap_handles) return device_dispatch_table.CmdDrawMeshTasksIndirectEXT(commandBuffer, buffer, offset, drawCount, stride);
    { buffer = Unwrap(buffer); }
    device_dispatch_table.CmdDrawMeshTasksIndirectEXT(commandBuffer, buffer, offset, drawCount, stride);
}

void DispatchObject::CmdDrawMeshTasksIndirectCountEXT(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                      VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                      uint32_t stride) {
    if (!wrap_handles)
        return device_dispatch_table.CmdDrawMeshTasksIndirectCountEXT(commandBuffer, buffer, offset, countBuffer, countBufferOffset,
                                                                      maxDrawCount, stride);
    {
        buffer = Unwrap(buffer);
        countBuffer = Unwrap(countBuffer);
    }
    device_dispatch_table.CmdDrawMeshTasksIndirectCountEXT(commandBuffer, buffer, offset, countBuffer, countBufferOffset,
                                                           maxDrawCount, stride);
}

// NOLINTEND
