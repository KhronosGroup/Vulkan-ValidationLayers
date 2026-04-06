// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See dispatch_object_generator.py for modifications

/***************************************************************************
 *
 * Copyright (c) 2015-2026 The Khronos Group Inc.
 * Copyright (c) 2015-2026 Valve Corporation
 * Copyright (c) 2015-2026 LunarG, Inc.
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
#include "generated/legacy.h"
#include "object_tracker/object_lifetime_validation.h"
#include "state_tracker/state_tracker.h"
#include "core_checks/core_validation.h"
#include "best_practices/best_practices_validation.h"
#include "gpuav/core/gpuav.h"
#include "sync/sync_validation.h"
#include "gpu_dump/gpu_dump.h"

#define DISPATCH_MAX_STACK_ALLOCATIONS 32

namespace vvl {

void DispatchInstance::InitValidationObjects() {
    // Note that this DEFINES THE ORDER IN WHICH THE LAYER VALIDATION OBJECTS ARE CALLED

    if (!settings.disabled[thread_safety]) {
        object_dispatch.emplace_back(new threadsafety::Instance(this));
    }
    if (!settings.disabled[stateless_checks]) {
        object_dispatch.emplace_back(new stateless::Instance(this));
    }
    if (settings.enabled[legacy_detection]) {
        object_dispatch.emplace_back(new legacy::Instance(this));
    }
    if (!settings.disabled[object_tracking]) {
        object_dispatch.emplace_back(new object_lifetimes::Instance(this));
    }
    if (!settings.disabled[core_checks] || settings.enabled[best_practices] || settings.enabled[gpu_validation] ||
        settings.enabled[debug_printf_validation] || settings.enabled[sync_validation]) {
        object_dispatch.emplace_back(new vvl::InstanceState(this));
    }
    if (!settings.disabled[core_checks]) {
        object_dispatch.emplace_back(new core::Instance(this));
    }
    if (settings.enabled[best_practices]) {
        object_dispatch.emplace_back(new bp_state::Instance(this));
    }
    if (settings.enabled[gpu_validation] || settings.enabled[debug_printf_validation]) {
        object_dispatch.emplace_back(new gpuav::Instance(this));
    }
    if (settings.enabled[sync_validation]) {
        object_dispatch.emplace_back(new syncval::Instance(this));
    }
    if (settings.enabled[gpu_dump]) {
        object_dispatch.emplace_back(new gpudump::Instance(this));
    }
}

void DispatchDevice::InitValidationObjects() {
    // Note that this DEFINES THE ORDER IN WHICH THE LAYER VALIDATION OBJECTS ARE CALLED

    if (!settings.disabled[thread_safety]) {
        object_dispatch.emplace_back(new threadsafety::Device(
            this, static_cast<threadsafety::Instance*>(dispatch_instance->GetValidationObject(LayerObjectTypeThreading))));
    }
    if (!settings.disabled[stateless_checks]) {
        object_dispatch.emplace_back(new stateless::Device(
            this, static_cast<stateless::Instance*>(dispatch_instance->GetValidationObject(LayerObjectTypeParameterValidation))));
    }
    if (settings.enabled[legacy_detection]) {
        object_dispatch.emplace_back(new legacy::Device(
            this, static_cast<legacy::Instance*>(dispatch_instance->GetValidationObject(LayerObjectTypeLegacy))));
    }
    if (!settings.disabled[object_tracking]) {
        object_dispatch.emplace_back(new object_lifetimes::Device(
            this, static_cast<object_lifetimes::Instance*>(dispatch_instance->GetValidationObject(LayerObjectTypeObjectTracker))));
    }
    if (!settings.disabled[core_checks] || settings.enabled[best_practices] || settings.enabled[gpu_validation] ||
        settings.enabled[debug_printf_validation] || settings.enabled[sync_validation]) {
        object_dispatch.emplace_back(new vvl::DeviceState(
            this, static_cast<vvl::InstanceState*>(dispatch_instance->GetValidationObject(LayerObjectTypeStateTracker))));
    }
    if (!settings.disabled[core_checks]) {
        object_dispatch.emplace_back(new CoreChecks(
            this, static_cast<core::Instance*>(dispatch_instance->GetValidationObject(LayerObjectTypeCoreValidation))));
    }
    if (settings.enabled[best_practices]) {
        object_dispatch.emplace_back(new BestPractices(
            this, static_cast<bp_state::Instance*>(dispatch_instance->GetValidationObject(LayerObjectTypeBestPractices))));
    }
    if (settings.enabled[gpu_validation] || settings.enabled[debug_printf_validation]) {
        object_dispatch.emplace_back(new gpuav::Validator(
            this, static_cast<gpuav::Instance*>(dispatch_instance->GetValidationObject(LayerObjectTypeGpuAssisted))));
    }
    if (settings.enabled[sync_validation]) {
        object_dispatch.emplace_back(new syncval::SyncValidator(
            this, static_cast<syncval::Instance*>(dispatch_instance->GetValidationObject(LayerObjectTypeSyncValidation))));
    }
    if (settings.enabled[gpu_dump]) {
        object_dispatch.emplace_back(new gpudump::GpuDump(
            this, static_cast<gpudump::Instance*>(dispatch_instance->GetValidationObject(LayerObjectTypeGpuDump))));
    }
}

// Unique Objects pNext extension handling function
void HandleWrapper::UnwrapPnextChainHandles(const void* pNext) {
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
            case VK_STRUCTURE_TYPE_SHADER_DESCRIPTOR_SET_AND_BINDING_MAPPING_INFO_EXT: {
                auto* safe_struct = reinterpret_cast<vku::safe_VkShaderDescriptorSetAndBindingMappingInfoEXT*>(cur_pnext);
                if (safe_struct->pMappings) {
                    for (uint32_t index0 = 0; index0 < safe_struct->mappingCount; ++index0) {
                        if (safe_struct->pMappings[index0].source == VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT) {
                            if (safe_struct->pMappings[index0].sourceData.constantOffset.pEmbeddedSampler) {
                                UnwrapPnextChainHandles(
                                    safe_struct->pMappings[index0].sourceData.constantOffset.pEmbeddedSampler->pNext);
                            }
                        }
                        if (safe_struct->pMappings[index0].source == VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_PUSH_INDEX_EXT) {
                            if (safe_struct->pMappings[index0].sourceData.pushIndex.pEmbeddedSampler) {
                                UnwrapPnextChainHandles(
                                    safe_struct->pMappings[index0].sourceData.pushIndex.pEmbeddedSampler->pNext);
                            }
                        }
                        if (safe_struct->pMappings[index0].source == VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_EXT) {
                            if (safe_struct->pMappings[index0].sourceData.indirectIndex.pEmbeddedSampler) {
                                UnwrapPnextChainHandles(
                                    safe_struct->pMappings[index0].sourceData.indirectIndex.pEmbeddedSampler->pNext);
                            }
                        }
                        if (safe_struct->pMappings[index0].source ==
                            VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_ARRAY_EXT) {
                            if (safe_struct->pMappings[index0].sourceData.indirectIndexArray.pEmbeddedSampler) {
                                UnwrapPnextChainHandles(
                                    safe_struct->pMappings[index0].sourceData.indirectIndexArray.pEmbeddedSampler->pNext);
                            }
                        }
                        if (safe_struct->pMappings[index0].source == VK_DESCRIPTOR_MAPPING_SOURCE_RESOURCE_HEAP_DATA_EXT) {
                        }
                        if (safe_struct->pMappings[index0].source == VK_DESCRIPTOR_MAPPING_SOURCE_INDIRECT_ADDRESS_EXT) {
                        }
                        if (safe_struct->pMappings[index0].source ==
                            VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_SHADER_RECORD_INDEX_EXT) {
                            if (safe_struct->pMappings[index0].sourceData.shaderRecordIndex.pEmbeddedSampler) {
                                UnwrapPnextChainHandles(
                                    safe_struct->pMappings[index0].sourceData.shaderRecordIndex.pEmbeddedSampler->pNext);
                            }
                        }
                    }
                }
            } break;
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
            case VK_STRUCTURE_TYPE_FRAME_BOUNDARY_TENSORS_ARM: {
                auto* safe_struct = reinterpret_cast<vku::safe_VkFrameBoundaryTensorsARM*>(cur_pnext);
                if (safe_struct->pTensors) {
                    for (uint32_t index0 = 0; index0 < safe_struct->tensorCount; ++index0) {
                        safe_struct->pTensors[index0] = Unwrap(safe_struct->pTensors[index0]);
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
            case VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO_TENSOR_ARM: {
                auto* safe_struct = reinterpret_cast<vku::safe_VkMemoryDedicatedAllocateInfoTensorARM*>(cur_pnext);

                if (safe_struct->tensor) {
                    safe_struct->tensor = Unwrap(safe_struct->tensor);
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
            case VK_STRUCTURE_TYPE_TILE_MEMORY_BIND_INFO_QCOM: {
                auto* safe_struct = reinterpret_cast<vku::safe_VkTileMemoryBindInfoQCOM*>(cur_pnext);

                if (safe_struct->memory) {
                    safe_struct->memory = Unwrap(safe_struct->memory);
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
            case VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_TENSOR_ARM: {
                auto* safe_struct = reinterpret_cast<vku::safe_VkWriteDescriptorSetTensorARM*>(cur_pnext);
                if (safe_struct->pTensorViews) {
                    for (uint32_t index0 = 0; index0 < safe_struct->tensorViewCount; ++index0) {
                        safe_struct->pTensorViews[index0] = Unwrap(safe_struct->pTensorViews[index0]);
                    }
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
            case VK_STRUCTURE_TYPE_TENSOR_DEPENDENCY_INFO_ARM: {
                auto* safe_struct = reinterpret_cast<vku::safe_VkTensorDependencyInfoARM*>(cur_pnext);
                if (safe_struct->pTensorMemoryBarriers) {
                    for (uint32_t index0 = 0; index0 < safe_struct->tensorMemoryBarrierCount; ++index0) {
                        if (safe_struct->pTensorMemoryBarriers[index0].tensor) {
                            safe_struct->pTensorMemoryBarriers[index0].tensor =
                                Unwrap(safe_struct->pTensorMemoryBarriers[index0].tensor);
                        }
                    }
                }
            } break;
            case VK_STRUCTURE_TYPE_TENSOR_MEMORY_BARRIER_ARM: {
                auto* safe_struct = reinterpret_cast<vku::safe_VkTensorMemoryBarrierARM*>(cur_pnext);

                if (safe_struct->tensor) {
                    safe_struct->tensor = Unwrap(safe_struct->tensor);
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
            case VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO: {
                auto* safe_struct = reinterpret_cast<vku::safe_VkPipelineLayoutCreateInfo*>(cur_pnext);
                if (safe_struct->pSetLayouts) {
                    for (uint32_t index0 = 0; index0 < safe_struct->setLayoutCount; ++index0) {
                        safe_struct->pSetLayouts[index0] = Unwrap(safe_struct->pSetLayouts[index0]);
                    }
                }
            } break;
            case VK_STRUCTURE_TYPE_SWAPCHAIN_PRESENT_FENCE_INFO_KHR: {
                auto* safe_struct = reinterpret_cast<vku::safe_VkSwapchainPresentFenceInfoKHR*>(cur_pnext);
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
            case VK_STRUCTURE_TYPE_SWAPCHAIN_CALIBRATED_TIMESTAMP_INFO_EXT: {
                auto* safe_struct = reinterpret_cast<vku::safe_VkSwapchainCalibratedTimestampInfoEXT*>(cur_pnext);

                if (safe_struct->swapchain) {
                    safe_struct->swapchain = Unwrap(safe_struct->swapchain);
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
            case VK_STRUCTURE_TYPE_DESCRIPTOR_GET_TENSOR_INFO_ARM: {
                auto* safe_struct = reinterpret_cast<vku::safe_VkDescriptorGetTensorInfoARM*>(cur_pnext);

                if (safe_struct->tensorView) {
                    safe_struct->tensorView = Unwrap(safe_struct->tensorView);
                }
            } break;
            case VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_TRIANGLES_OPACITY_MICROMAP_EXT: {
                auto* safe_struct = reinterpret_cast<vku::safe_VkAccelerationStructureTrianglesOpacityMicromapEXT*>(cur_pnext);

                if (safe_struct->micromap) {
                    safe_struct->micromap = Unwrap(safe_struct->micromap);
                }
            } break;
            case VK_STRUCTURE_TYPE_DATA_GRAPH_PIPELINE_SHADER_MODULE_CREATE_INFO_ARM: {
                auto* safe_struct = reinterpret_cast<vku::safe_VkDataGraphPipelineShaderModuleCreateInfoARM*>(cur_pnext);

                if (safe_struct->module) {
                    safe_struct->module = Unwrap(safe_struct->module);
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
        case VK_OBJECT_TYPE_EXTERNAL_COMPUTE_QUEUE_NV:
            return false;
        default:
            return true;
    }
}

VkResult DispatchInstance::EnumeratePhysicalDevices(VkInstance instance, uint32_t* pPhysicalDeviceCount,
                                                    VkPhysicalDevice* pPhysicalDevices) {
    VkResult result = instance_dispatch_table.EnumeratePhysicalDevices(instance, pPhysicalDeviceCount, pPhysicalDevices);

    return result;
}

void DispatchInstance::GetPhysicalDeviceFeatures(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures* pFeatures) {
    instance_dispatch_table.GetPhysicalDeviceFeatures(physicalDevice, pFeatures);
}

void DispatchInstance::GetPhysicalDeviceFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format,
                                                         VkFormatProperties* pFormatProperties) {
    instance_dispatch_table.GetPhysicalDeviceFormatProperties(physicalDevice, format, pFormatProperties);
}

VkResult DispatchInstance::GetPhysicalDeviceImageFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format,
                                                                  VkImageType type, VkImageTiling tiling, VkImageUsageFlags usage,
                                                                  VkImageCreateFlags flags,
                                                                  VkImageFormatProperties* pImageFormatProperties) {
    VkResult result = instance_dispatch_table.GetPhysicalDeviceImageFormatProperties(physicalDevice, format, type, tiling, usage,
                                                                                     flags, pImageFormatProperties);

    return result;
}

void DispatchInstance::GetPhysicalDeviceProperties(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties* pProperties) {
    instance_dispatch_table.GetPhysicalDeviceProperties(physicalDevice, pProperties);
}

void DispatchInstance::GetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount,
                                                              VkQueueFamilyProperties* pQueueFamilyProperties) {
    instance_dispatch_table.GetPhysicalDeviceQueueFamilyProperties(physicalDevice, pQueueFamilyPropertyCount,
                                                                   pQueueFamilyProperties);
}

void DispatchInstance::GetPhysicalDeviceMemoryProperties(VkPhysicalDevice physicalDevice,
                                                         VkPhysicalDeviceMemoryProperties* pMemoryProperties) {
    instance_dispatch_table.GetPhysicalDeviceMemoryProperties(physicalDevice, pMemoryProperties);
}

PFN_vkVoidFunction DispatchInstance::GetInstanceProcAddr(VkInstance instance, const char* pName) {
    PFN_vkVoidFunction result = instance_dispatch_table.GetInstanceProcAddr(instance, pName);

    return result;
}

PFN_vkVoidFunction DispatchDevice::GetDeviceProcAddr(VkDevice device, const char* pName) {
    PFN_vkVoidFunction result = device_dispatch_table.GetDeviceProcAddr(device, pName);

    return result;
}

VkResult DispatchInstance::EnumerateDeviceExtensionProperties(VkPhysicalDevice physicalDevice, const char* pLayerName,
                                                              uint32_t* pPropertyCount, VkExtensionProperties* pProperties) {
    VkResult result =
        instance_dispatch_table.EnumerateDeviceExtensionProperties(physicalDevice, pLayerName, pPropertyCount, pProperties);

    return result;
}

void DispatchDevice::GetDeviceQueue(VkDevice device, uint32_t queueFamilyIndex, uint32_t queueIndex, VkQueue* pQueue) {
    device_dispatch_table.GetDeviceQueue(device, queueFamilyIndex, queueIndex, pQueue);
}

VkResult DispatchDevice::QueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence) {
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

VkResult DispatchDevice::QueueWaitIdle(VkQueue queue) {
    VkResult result = device_dispatch_table.QueueWaitIdle(queue);

    return result;
}

VkResult DispatchDevice::DeviceWaitIdle(VkDevice device) {
    VkResult result = device_dispatch_table.DeviceWaitIdle(device);

    return result;
}

VkResult DispatchDevice::AllocateMemory(VkDevice device, const VkMemoryAllocateInfo* pAllocateInfo,
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
    if (result == VK_SUCCESS) {
        *pMemory = WrapNew(*pMemory);
    }
    return result;
}

void DispatchDevice::FreeMemory(VkDevice device, VkDeviceMemory memory, const VkAllocationCallbacks* pAllocator) {
    if (!wrap_handles) return device_dispatch_table.FreeMemory(device, memory, pAllocator);
    memory = Erase(memory);
    device_dispatch_table.FreeMemory(device, memory, pAllocator);
}

VkResult DispatchDevice::MapMemory(VkDevice device, VkDeviceMemory memory, VkDeviceSize offset, VkDeviceSize size,
                                   VkMemoryMapFlags flags, void** ppData) {
    if (!wrap_handles) return device_dispatch_table.MapMemory(device, memory, offset, size, flags, ppData);
    {
        memory = Unwrap(memory);
    }
    VkResult result = device_dispatch_table.MapMemory(device, memory, offset, size, flags, ppData);

    return result;
}

void DispatchDevice::UnmapMemory(VkDevice device, VkDeviceMemory memory) {
    if (!wrap_handles) return device_dispatch_table.UnmapMemory(device, memory);
    {
        memory = Unwrap(memory);
    }
    device_dispatch_table.UnmapMemory(device, memory);
}

VkResult DispatchDevice::FlushMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount,
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

VkResult DispatchDevice::InvalidateMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount,
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

void DispatchDevice::GetDeviceMemoryCommitment(VkDevice device, VkDeviceMemory memory, VkDeviceSize* pCommittedMemoryInBytes) {
    if (!wrap_handles) return device_dispatch_table.GetDeviceMemoryCommitment(device, memory, pCommittedMemoryInBytes);
    {
        memory = Unwrap(memory);
    }
    device_dispatch_table.GetDeviceMemoryCommitment(device, memory, pCommittedMemoryInBytes);
}

VkResult DispatchDevice::BindBufferMemory(VkDevice device, VkBuffer buffer, VkDeviceMemory memory, VkDeviceSize memoryOffset) {
    if (!wrap_handles) return device_dispatch_table.BindBufferMemory(device, buffer, memory, memoryOffset);
    {
        buffer = Unwrap(buffer);
        memory = Unwrap(memory);
    }
    VkResult result = device_dispatch_table.BindBufferMemory(device, buffer, memory, memoryOffset);

    return result;
}

VkResult DispatchDevice::BindImageMemory(VkDevice device, VkImage image, VkDeviceMemory memory, VkDeviceSize memoryOffset) {
    if (!wrap_handles) return device_dispatch_table.BindImageMemory(device, image, memory, memoryOffset);
    {
        image = Unwrap(image);
        memory = Unwrap(memory);
    }
    VkResult result = device_dispatch_table.BindImageMemory(device, image, memory, memoryOffset);

    return result;
}

void DispatchDevice::GetBufferMemoryRequirements(VkDevice device, VkBuffer buffer, VkMemoryRequirements* pMemoryRequirements) {
    if (!wrap_handles) return device_dispatch_table.GetBufferMemoryRequirements(device, buffer, pMemoryRequirements);
    {
        buffer = Unwrap(buffer);
    }
    device_dispatch_table.GetBufferMemoryRequirements(device, buffer, pMemoryRequirements);
}

void DispatchDevice::GetImageMemoryRequirements(VkDevice device, VkImage image, VkMemoryRequirements* pMemoryRequirements) {
    if (!wrap_handles) return device_dispatch_table.GetImageMemoryRequirements(device, image, pMemoryRequirements);
    {
        image = Unwrap(image);
    }
    device_dispatch_table.GetImageMemoryRequirements(device, image, pMemoryRequirements);
}

void DispatchDevice::GetImageSparseMemoryRequirements(VkDevice device, VkImage image, uint32_t* pSparseMemoryRequirementCount,
                                                      VkSparseImageMemoryRequirements* pSparseMemoryRequirements) {
    if (!wrap_handles)
        return device_dispatch_table.GetImageSparseMemoryRequirements(device, image, pSparseMemoryRequirementCount,
                                                                      pSparseMemoryRequirements);
    {
        image = Unwrap(image);
    }
    device_dispatch_table.GetImageSparseMemoryRequirements(device, image, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
}

void DispatchInstance::GetPhysicalDeviceSparseImageFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format,
                                                                    VkImageType type, VkSampleCountFlagBits samples,
                                                                    VkImageUsageFlags usage, VkImageTiling tiling,
                                                                    uint32_t* pPropertyCount,
                                                                    VkSparseImageFormatProperties* pProperties) {
    instance_dispatch_table.GetPhysicalDeviceSparseImageFormatProperties(physicalDevice, format, type, samples, usage, tiling,
                                                                         pPropertyCount, pProperties);
}

VkResult DispatchDevice::QueueBindSparse(VkQueue queue, uint32_t bindInfoCount, const VkBindSparseInfo* pBindInfo, VkFence fence) {
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

VkResult DispatchDevice::CreateFence(VkDevice device, const VkFenceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                     VkFence* pFence) {
    if (!wrap_handles) return device_dispatch_table.CreateFence(device, pCreateInfo, pAllocator, pFence);

    VkResult result = device_dispatch_table.CreateFence(device, pCreateInfo, pAllocator, pFence);
    if (result == VK_SUCCESS) {
        *pFence = WrapNew(*pFence);
    }
    return result;
}

void DispatchDevice::DestroyFence(VkDevice device, VkFence fence, const VkAllocationCallbacks* pAllocator) {
    if (!wrap_handles) return device_dispatch_table.DestroyFence(device, fence, pAllocator);
    fence = Erase(fence);
    device_dispatch_table.DestroyFence(device, fence, pAllocator);
}

VkResult DispatchDevice::ResetFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences) {
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

VkResult DispatchDevice::GetFenceStatus(VkDevice device, VkFence fence) {
    if (!wrap_handles) return device_dispatch_table.GetFenceStatus(device, fence);
    {
        fence = Unwrap(fence);
    }
    VkResult result = device_dispatch_table.GetFenceStatus(device, fence);

    return result;
}

VkResult DispatchDevice::WaitForFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences, VkBool32 waitAll,
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

VkResult DispatchDevice::CreateSemaphore(VkDevice device, const VkSemaphoreCreateInfo* pCreateInfo,
                                         const VkAllocationCallbacks* pAllocator, VkSemaphore* pSemaphore) {
    if (!wrap_handles) return device_dispatch_table.CreateSemaphore(device, pCreateInfo, pAllocator, pSemaphore);

    VkResult result = device_dispatch_table.CreateSemaphore(device, pCreateInfo, pAllocator, pSemaphore);
    if (result == VK_SUCCESS) {
        *pSemaphore = WrapNew(*pSemaphore);
    }
    return result;
}

void DispatchDevice::DestroySemaphore(VkDevice device, VkSemaphore semaphore, const VkAllocationCallbacks* pAllocator) {
    if (!wrap_handles) return device_dispatch_table.DestroySemaphore(device, semaphore, pAllocator);
    semaphore = Erase(semaphore);
    device_dispatch_table.DestroySemaphore(device, semaphore, pAllocator);
}

VkResult DispatchDevice::CreateQueryPool(VkDevice device, const VkQueryPoolCreateInfo* pCreateInfo,
                                         const VkAllocationCallbacks* pAllocator, VkQueryPool* pQueryPool) {
    if (!wrap_handles) return device_dispatch_table.CreateQueryPool(device, pCreateInfo, pAllocator, pQueryPool);

    VkResult result = device_dispatch_table.CreateQueryPool(device, pCreateInfo, pAllocator, pQueryPool);
    if (result == VK_SUCCESS) {
        *pQueryPool = WrapNew(*pQueryPool);
    }
    return result;
}

void DispatchDevice::DestroyQueryPool(VkDevice device, VkQueryPool queryPool, const VkAllocationCallbacks* pAllocator) {
    if (!wrap_handles) return device_dispatch_table.DestroyQueryPool(device, queryPool, pAllocator);
    queryPool = Erase(queryPool);
    device_dispatch_table.DestroyQueryPool(device, queryPool, pAllocator);
}

VkResult DispatchDevice::GetQueryPoolResults(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount,
                                             size_t dataSize, void* pData, VkDeviceSize stride, VkQueryResultFlags flags) {
    if (!wrap_handles)
        return device_dispatch_table.GetQueryPoolResults(device, queryPool, firstQuery, queryCount, dataSize, pData, stride, flags);
    {
        queryPool = Unwrap(queryPool);
    }
    VkResult result =
        device_dispatch_table.GetQueryPoolResults(device, queryPool, firstQuery, queryCount, dataSize, pData, stride, flags);

    return result;
}

VkResult DispatchDevice::CreateBuffer(VkDevice device, const VkBufferCreateInfo* pCreateInfo,
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
    if (result == VK_SUCCESS) {
        *pBuffer = WrapNew(*pBuffer);
    }
    return result;
}

void DispatchDevice::DestroyBuffer(VkDevice device, VkBuffer buffer, const VkAllocationCallbacks* pAllocator) {
    if (!wrap_handles) return device_dispatch_table.DestroyBuffer(device, buffer, pAllocator);
    buffer = Erase(buffer);
    device_dispatch_table.DestroyBuffer(device, buffer, pAllocator);
}

VkResult DispatchDevice::CreateImage(VkDevice device, const VkImageCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
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
    if (result == VK_SUCCESS) {
        *pImage = WrapNew(*pImage);
    }
    return result;
}

void DispatchDevice::DestroyImage(VkDevice device, VkImage image, const VkAllocationCallbacks* pAllocator) {
    if (!wrap_handles) return device_dispatch_table.DestroyImage(device, image, pAllocator);
    image = Erase(image);
    device_dispatch_table.DestroyImage(device, image, pAllocator);
}

void DispatchDevice::GetImageSubresourceLayout(VkDevice device, VkImage image, const VkImageSubresource* pSubresource,
                                               VkSubresourceLayout* pLayout) {
    if (!wrap_handles) return device_dispatch_table.GetImageSubresourceLayout(device, image, pSubresource, pLayout);
    {
        image = Unwrap(image);
    }
    device_dispatch_table.GetImageSubresourceLayout(device, image, pSubresource, pLayout);
}

VkResult DispatchDevice::CreateImageView(VkDevice device, const VkImageViewCreateInfo* pCreateInfo,
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
    if (result == VK_SUCCESS) {
        *pView = WrapNew(*pView);
    }
    return result;
}

void DispatchDevice::DestroyImageView(VkDevice device, VkImageView imageView, const VkAllocationCallbacks* pAllocator) {
    if (!wrap_handles) return device_dispatch_table.DestroyImageView(device, imageView, pAllocator);
    imageView = Erase(imageView);
    device_dispatch_table.DestroyImageView(device, imageView, pAllocator);
}

VkResult DispatchDevice::CreateCommandPool(VkDevice device, const VkCommandPoolCreateInfo* pCreateInfo,
                                           const VkAllocationCallbacks* pAllocator, VkCommandPool* pCommandPool) {
    if (!wrap_handles) return device_dispatch_table.CreateCommandPool(device, pCreateInfo, pAllocator, pCommandPool);

    VkResult result = device_dispatch_table.CreateCommandPool(device, pCreateInfo, pAllocator, pCommandPool);
    if (result == VK_SUCCESS) {
        *pCommandPool = WrapNew(*pCommandPool);
    }
    return result;
}

VkResult DispatchDevice::ResetCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolResetFlags flags) {
    if (!wrap_handles) return device_dispatch_table.ResetCommandPool(device, commandPool, flags);
    {
        commandPool = Unwrap(commandPool);
    }
    VkResult result = device_dispatch_table.ResetCommandPool(device, commandPool, flags);

    return result;
}

VkResult DispatchDevice::EndCommandBuffer(VkCommandBuffer commandBuffer) {
    VkResult result = device_dispatch_table.EndCommandBuffer(commandBuffer);

    return result;
}

VkResult DispatchDevice::ResetCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferResetFlags flags) {
    VkResult result = device_dispatch_table.ResetCommandBuffer(commandBuffer, flags);

    return result;
}

void DispatchDevice::CmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t regionCount,
                                   const VkBufferCopy* pRegions) {
    if (!wrap_handles) return device_dispatch_table.CmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, regionCount, pRegions);
    {
        srcBuffer = Unwrap(srcBuffer);
        dstBuffer = Unwrap(dstBuffer);
    }
    device_dispatch_table.CmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, regionCount, pRegions);
}

void DispatchDevice::CmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage,
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

void DispatchDevice::CmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage,
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

void DispatchDevice::CmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
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

void DispatchDevice::CmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                     VkDeviceSize dataSize, const void* pData) {
    if (!wrap_handles) return device_dispatch_table.CmdUpdateBuffer(commandBuffer, dstBuffer, dstOffset, dataSize, pData);
    {
        dstBuffer = Unwrap(dstBuffer);
    }
    device_dispatch_table.CmdUpdateBuffer(commandBuffer, dstBuffer, dstOffset, dataSize, pData);
}

void DispatchDevice::CmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize size,
                                   uint32_t data) {
    if (!wrap_handles) return device_dispatch_table.CmdFillBuffer(commandBuffer, dstBuffer, dstOffset, size, data);
    {
        dstBuffer = Unwrap(dstBuffer);
    }
    device_dispatch_table.CmdFillBuffer(commandBuffer, dstBuffer, dstOffset, size, data);
}

void DispatchDevice::CmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask,
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

void DispatchDevice::CmdBeginQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query,
                                   VkQueryControlFlags flags) {
    if (!wrap_handles) return device_dispatch_table.CmdBeginQuery(commandBuffer, queryPool, query, flags);
    {
        queryPool = Unwrap(queryPool);
    }
    device_dispatch_table.CmdBeginQuery(commandBuffer, queryPool, query, flags);
}

void DispatchDevice::CmdEndQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query) {
    if (!wrap_handles) return device_dispatch_table.CmdEndQuery(commandBuffer, queryPool, query);
    {
        queryPool = Unwrap(queryPool);
    }
    device_dispatch_table.CmdEndQuery(commandBuffer, queryPool, query);
}

void DispatchDevice::CmdResetQueryPool(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery,
                                       uint32_t queryCount) {
    if (!wrap_handles) return device_dispatch_table.CmdResetQueryPool(commandBuffer, queryPool, firstQuery, queryCount);
    {
        queryPool = Unwrap(queryPool);
    }
    device_dispatch_table.CmdResetQueryPool(commandBuffer, queryPool, firstQuery, queryCount);
}

void DispatchDevice::CmdWriteTimestamp(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage, VkQueryPool queryPool,
                                       uint32_t query) {
    if (!wrap_handles) return device_dispatch_table.CmdWriteTimestamp(commandBuffer, pipelineStage, queryPool, query);
    {
        queryPool = Unwrap(queryPool);
    }
    device_dispatch_table.CmdWriteTimestamp(commandBuffer, pipelineStage, queryPool, query);
}

void DispatchDevice::CmdCopyQueryPoolResults(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery,
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

void DispatchDevice::CmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBufferCount,
                                        const VkCommandBuffer* pCommandBuffers) {
    device_dispatch_table.CmdExecuteCommands(commandBuffer, commandBufferCount, pCommandBuffers);
}

VkResult DispatchDevice::CreateEvent(VkDevice device, const VkEventCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                     VkEvent* pEvent) {
    if (!wrap_handles) return device_dispatch_table.CreateEvent(device, pCreateInfo, pAllocator, pEvent);

    VkResult result = device_dispatch_table.CreateEvent(device, pCreateInfo, pAllocator, pEvent);
    if (result == VK_SUCCESS) {
        *pEvent = WrapNew(*pEvent);
    }
    return result;
}

void DispatchDevice::DestroyEvent(VkDevice device, VkEvent event, const VkAllocationCallbacks* pAllocator) {
    if (!wrap_handles) return device_dispatch_table.DestroyEvent(device, event, pAllocator);
    event = Erase(event);
    device_dispatch_table.DestroyEvent(device, event, pAllocator);
}

VkResult DispatchDevice::GetEventStatus(VkDevice device, VkEvent event) {
    if (!wrap_handles) return device_dispatch_table.GetEventStatus(device, event);
    {
        event = Unwrap(event);
    }
    VkResult result = device_dispatch_table.GetEventStatus(device, event);

    return result;
}

VkResult DispatchDevice::SetEvent(VkDevice device, VkEvent event) {
    if (!wrap_handles) return device_dispatch_table.SetEvent(device, event);
    {
        event = Unwrap(event);
    }
    VkResult result = device_dispatch_table.SetEvent(device, event);

    return result;
}

VkResult DispatchDevice::ResetEvent(VkDevice device, VkEvent event) {
    if (!wrap_handles) return device_dispatch_table.ResetEvent(device, event);
    {
        event = Unwrap(event);
    }
    VkResult result = device_dispatch_table.ResetEvent(device, event);

    return result;
}

VkResult DispatchDevice::CreateBufferView(VkDevice device, const VkBufferViewCreateInfo* pCreateInfo,
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
    if (result == VK_SUCCESS) {
        *pView = WrapNew(*pView);
    }
    return result;
}

void DispatchDevice::DestroyBufferView(VkDevice device, VkBufferView bufferView, const VkAllocationCallbacks* pAllocator) {
    if (!wrap_handles) return device_dispatch_table.DestroyBufferView(device, bufferView, pAllocator);
    bufferView = Erase(bufferView);
    device_dispatch_table.DestroyBufferView(device, bufferView, pAllocator);
}

VkResult DispatchDevice::CreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo,
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
    if (result == VK_SUCCESS) {
        *pShaderModule = WrapNew(*pShaderModule);
    }
    return result;
}

void DispatchDevice::DestroyShaderModule(VkDevice device, VkShaderModule shaderModule, const VkAllocationCallbacks* pAllocator) {
    if (!wrap_handles) return device_dispatch_table.DestroyShaderModule(device, shaderModule, pAllocator);
    shaderModule = Erase(shaderModule);
    device_dispatch_table.DestroyShaderModule(device, shaderModule, pAllocator);
}

VkResult DispatchDevice::CreatePipelineCache(VkDevice device, const VkPipelineCacheCreateInfo* pCreateInfo,
                                             const VkAllocationCallbacks* pAllocator, VkPipelineCache* pPipelineCache) {
    if (!wrap_handles) return device_dispatch_table.CreatePipelineCache(device, pCreateInfo, pAllocator, pPipelineCache);

    VkResult result = device_dispatch_table.CreatePipelineCache(device, pCreateInfo, pAllocator, pPipelineCache);
    if (result == VK_SUCCESS) {
        *pPipelineCache = WrapNew(*pPipelineCache);
    }
    return result;
}

void DispatchDevice::DestroyPipelineCache(VkDevice device, VkPipelineCache pipelineCache, const VkAllocationCallbacks* pAllocator) {
    if (!wrap_handles) return device_dispatch_table.DestroyPipelineCache(device, pipelineCache, pAllocator);
    pipelineCache = Erase(pipelineCache);
    device_dispatch_table.DestroyPipelineCache(device, pipelineCache, pAllocator);
}

VkResult DispatchDevice::GetPipelineCacheData(VkDevice device, VkPipelineCache pipelineCache, size_t* pDataSize, void* pData) {
    if (!wrap_handles) return device_dispatch_table.GetPipelineCacheData(device, pipelineCache, pDataSize, pData);
    {
        pipelineCache = Unwrap(pipelineCache);
    }
    VkResult result = device_dispatch_table.GetPipelineCacheData(device, pipelineCache, pDataSize, pData);

    return result;
}

VkResult DispatchDevice::MergePipelineCaches(VkDevice device, VkPipelineCache dstCache, uint32_t srcCacheCount,
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

void DispatchDevice::DestroyPipeline(VkDevice device, VkPipeline pipeline, const VkAllocationCallbacks* pAllocator) {
    if (!wrap_handles) return device_dispatch_table.DestroyPipeline(device, pipeline, pAllocator);
    pipeline = Erase(pipeline);
    device_dispatch_table.DestroyPipeline(device, pipeline, pAllocator);
}

VkResult DispatchDevice::CreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo* pCreateInfo,
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
    if (result == VK_SUCCESS) {
        *pPipelineLayout = WrapNew(*pPipelineLayout);
    }
    return result;
}

void DispatchDevice::DestroyPipelineLayout(VkDevice device, VkPipelineLayout pipelineLayout,
                                           const VkAllocationCallbacks* pAllocator) {
    if (!wrap_handles) return device_dispatch_table.DestroyPipelineLayout(device, pipelineLayout, pAllocator);
    pipelineLayout = Erase(pipelineLayout);
    device_dispatch_table.DestroyPipelineLayout(device, pipelineLayout, pAllocator);
}

VkResult DispatchDevice::CreateSampler(VkDevice device, const VkSamplerCreateInfo* pCreateInfo,
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
    if (result == VK_SUCCESS) {
        *pSampler = WrapNew(*pSampler);
    }
    return result;
}

void DispatchDevice::DestroySampler(VkDevice device, VkSampler sampler, const VkAllocationCallbacks* pAllocator) {
    if (!wrap_handles) return device_dispatch_table.DestroySampler(device, sampler, pAllocator);
    sampler = Erase(sampler);
    device_dispatch_table.DestroySampler(device, sampler, pAllocator);
}

VkResult DispatchDevice::CreateDescriptorSetLayout(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo,
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
    if (result == VK_SUCCESS) {
        *pSetLayout = WrapNew(*pSetLayout);
    }
    return result;
}

void DispatchDevice::DestroyDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout descriptorSetLayout,
                                                const VkAllocationCallbacks* pAllocator) {
    if (!wrap_handles) return device_dispatch_table.DestroyDescriptorSetLayout(device, descriptorSetLayout, pAllocator);
    descriptorSetLayout = Erase(descriptorSetLayout);
    device_dispatch_table.DestroyDescriptorSetLayout(device, descriptorSetLayout, pAllocator);
}

VkResult DispatchDevice::CreateDescriptorPool(VkDevice device, const VkDescriptorPoolCreateInfo* pCreateInfo,
                                              const VkAllocationCallbacks* pAllocator, VkDescriptorPool* pDescriptorPool) {
    if (!wrap_handles) return device_dispatch_table.CreateDescriptorPool(device, pCreateInfo, pAllocator, pDescriptorPool);

    VkResult result = device_dispatch_table.CreateDescriptorPool(device, pCreateInfo, pAllocator, pDescriptorPool);
    if (result == VK_SUCCESS) {
        *pDescriptorPool = WrapNew(*pDescriptorPool);
    }
    return result;
}

void DispatchDevice::UpdateDescriptorSets(VkDevice device, uint32_t descriptorWriteCount,
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
                    // need for when updating VkDescriptorImageInfo
                    bool has_sampler =
                        local_pDescriptorWrites[index0].descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER ||
                        local_pDescriptorWrites[index0].descriptorType == VK_DESCRIPTOR_TYPE_SAMPLER;
                    bool has_image_view = local_pDescriptorWrites[index0].descriptorType != VK_DESCRIPTOR_TYPE_SAMPLER;

                    for (uint32_t index1 = 0; index1 < local_pDescriptorWrites[index0].descriptorCount; ++index1) {
                        if (pDescriptorWrites[index0].pImageInfo[index1].sampler && has_sampler) {
                            local_pDescriptorWrites[index0].pImageInfo[index1].sampler =
                                Unwrap(pDescriptorWrites[index0].pImageInfo[index1].sampler);
                        }
                        if (pDescriptorWrites[index0].pImageInfo[index1].imageView && has_image_view) {
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

void DispatchDevice::CmdBindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline) {
    if (!wrap_handles) return device_dispatch_table.CmdBindPipeline(commandBuffer, pipelineBindPoint, pipeline);
    {
        pipeline = Unwrap(pipeline);
    }
    device_dispatch_table.CmdBindPipeline(commandBuffer, pipelineBindPoint, pipeline);
}

void DispatchDevice::CmdBindDescriptorSets(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
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

void DispatchDevice::CmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                        const VkClearColorValue* pColor, uint32_t rangeCount,
                                        const VkImageSubresourceRange* pRanges) {
    if (!wrap_handles)
        return device_dispatch_table.CmdClearColorImage(commandBuffer, image, imageLayout, pColor, rangeCount, pRanges);
    {
        image = Unwrap(image);
    }
    device_dispatch_table.CmdClearColorImage(commandBuffer, image, imageLayout, pColor, rangeCount, pRanges);
}

void DispatchDevice::CmdDispatch(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) {
    device_dispatch_table.CmdDispatch(commandBuffer, groupCountX, groupCountY, groupCountZ);
}

void DispatchDevice::CmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset) {
    if (!wrap_handles) return device_dispatch_table.CmdDispatchIndirect(commandBuffer, buffer, offset);
    {
        buffer = Unwrap(buffer);
    }
    device_dispatch_table.CmdDispatchIndirect(commandBuffer, buffer, offset);
}

void DispatchDevice::CmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask) {
    if (!wrap_handles) return device_dispatch_table.CmdSetEvent(commandBuffer, event, stageMask);
    {
        event = Unwrap(event);
    }
    device_dispatch_table.CmdSetEvent(commandBuffer, event, stageMask);
}

void DispatchDevice::CmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask) {
    if (!wrap_handles) return device_dispatch_table.CmdResetEvent(commandBuffer, event, stageMask);
    {
        event = Unwrap(event);
    }
    device_dispatch_table.CmdResetEvent(commandBuffer, event, stageMask);
}

void DispatchDevice::CmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
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

void DispatchDevice::CmdPushConstants(VkCommandBuffer commandBuffer, VkPipelineLayout layout, VkShaderStageFlags stageFlags,
                                      uint32_t offset, uint32_t size, const void* pValues) {
    if (!wrap_handles) return device_dispatch_table.CmdPushConstants(commandBuffer, layout, stageFlags, offset, size, pValues);
    {
        layout = Unwrap(layout);
    }
    device_dispatch_table.CmdPushConstants(commandBuffer, layout, stageFlags, offset, size, pValues);
}

VkResult DispatchDevice::CreateFramebuffer(VkDevice device, const VkFramebufferCreateInfo* pCreateInfo,
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
    if (result == VK_SUCCESS) {
        *pFramebuffer = WrapNew(*pFramebuffer);
    }
    return result;
}

void DispatchDevice::DestroyFramebuffer(VkDevice device, VkFramebuffer framebuffer, const VkAllocationCallbacks* pAllocator) {
    if (!wrap_handles) return device_dispatch_table.DestroyFramebuffer(device, framebuffer, pAllocator);
    framebuffer = Erase(framebuffer);
    device_dispatch_table.DestroyFramebuffer(device, framebuffer, pAllocator);
}

void DispatchDevice::GetRenderAreaGranularity(VkDevice device, VkRenderPass renderPass, VkExtent2D* pGranularity) {
    if (!wrap_handles) return device_dispatch_table.GetRenderAreaGranularity(device, renderPass, pGranularity);
    {
        renderPass = Unwrap(renderPass);
    }
    device_dispatch_table.GetRenderAreaGranularity(device, renderPass, pGranularity);
}

void DispatchDevice::CmdSetViewport(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount,
                                    const VkViewport* pViewports) {
    device_dispatch_table.CmdSetViewport(commandBuffer, firstViewport, viewportCount, pViewports);
}

void DispatchDevice::CmdSetScissor(VkCommandBuffer commandBuffer, uint32_t firstScissor, uint32_t scissorCount,
                                   const VkRect2D* pScissors) {
    device_dispatch_table.CmdSetScissor(commandBuffer, firstScissor, scissorCount, pScissors);
}

void DispatchDevice::CmdSetLineWidth(VkCommandBuffer commandBuffer, float lineWidth) {
    device_dispatch_table.CmdSetLineWidth(commandBuffer, lineWidth);
}

void DispatchDevice::CmdSetDepthBias(VkCommandBuffer commandBuffer, float depthBiasConstantFactor, float depthBiasClamp,
                                     float depthBiasSlopeFactor) {
    device_dispatch_table.CmdSetDepthBias(commandBuffer, depthBiasConstantFactor, depthBiasClamp, depthBiasSlopeFactor);
}

void DispatchDevice::CmdSetBlendConstants(VkCommandBuffer commandBuffer, const float blendConstants[4]) {
    device_dispatch_table.CmdSetBlendConstants(commandBuffer, blendConstants);
}

void DispatchDevice::CmdSetDepthBounds(VkCommandBuffer commandBuffer, float minDepthBounds, float maxDepthBounds) {
    device_dispatch_table.CmdSetDepthBounds(commandBuffer, minDepthBounds, maxDepthBounds);
}

void DispatchDevice::CmdSetStencilCompareMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t compareMask) {
    device_dispatch_table.CmdSetStencilCompareMask(commandBuffer, faceMask, compareMask);
}

void DispatchDevice::CmdSetStencilWriteMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t writeMask) {
    device_dispatch_table.CmdSetStencilWriteMask(commandBuffer, faceMask, writeMask);
}

void DispatchDevice::CmdSetStencilReference(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t reference) {
    device_dispatch_table.CmdSetStencilReference(commandBuffer, faceMask, reference);
}

void DispatchDevice::CmdBindIndexBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                        VkIndexType indexType) {
    if (!wrap_handles) return device_dispatch_table.CmdBindIndexBuffer(commandBuffer, buffer, offset, indexType);
    {
        buffer = Unwrap(buffer);
    }
    device_dispatch_table.CmdBindIndexBuffer(commandBuffer, buffer, offset, indexType);
}

void DispatchDevice::CmdBindVertexBuffers(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount,
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

void DispatchDevice::CmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex,
                             uint32_t firstInstance) {
    device_dispatch_table.CmdDraw(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
}

void DispatchDevice::CmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex,
                                    int32_t vertexOffset, uint32_t firstInstance) {
    device_dispatch_table.CmdDrawIndexed(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

void DispatchDevice::CmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount,
                                     uint32_t stride) {
    if (!wrap_handles) return device_dispatch_table.CmdDrawIndirect(commandBuffer, buffer, offset, drawCount, stride);
    {
        buffer = Unwrap(buffer);
    }
    device_dispatch_table.CmdDrawIndirect(commandBuffer, buffer, offset, drawCount, stride);
}

void DispatchDevice::CmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount,
                                            uint32_t stride) {
    if (!wrap_handles) return device_dispatch_table.CmdDrawIndexedIndirect(commandBuffer, buffer, offset, drawCount, stride);
    {
        buffer = Unwrap(buffer);
    }
    device_dispatch_table.CmdDrawIndexedIndirect(commandBuffer, buffer, offset, drawCount, stride);
}

void DispatchDevice::CmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage,
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

void DispatchDevice::CmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                               const VkClearDepthStencilValue* pDepthStencil, uint32_t rangeCount,
                                               const VkImageSubresourceRange* pRanges) {
    if (!wrap_handles)
        return device_dispatch_table.CmdClearDepthStencilImage(commandBuffer, image, imageLayout, pDepthStencil, rangeCount,
                                                               pRanges);
    {
        image = Unwrap(image);
    }
    device_dispatch_table.CmdClearDepthStencilImage(commandBuffer, image, imageLayout, pDepthStencil, rangeCount, pRanges);
}

void DispatchDevice::CmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
                                         const VkClearAttachment* pAttachments, uint32_t rectCount, const VkClearRect* pRects) {
    device_dispatch_table.CmdClearAttachments(commandBuffer, attachmentCount, pAttachments, rectCount, pRects);
}

void DispatchDevice::CmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
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

void DispatchDevice::CmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
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

void DispatchDevice::CmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents) {
    device_dispatch_table.CmdNextSubpass(commandBuffer, contents);
}

void DispatchDevice::CmdEndRenderPass(VkCommandBuffer commandBuffer) { device_dispatch_table.CmdEndRenderPass(commandBuffer); }

void DispatchDevice::GetDeviceGroupPeerMemoryFeatures(VkDevice device, uint32_t heapIndex, uint32_t localDeviceIndex,
                                                      uint32_t remoteDeviceIndex, VkPeerMemoryFeatureFlags* pPeerMemoryFeatures) {
    device_dispatch_table.GetDeviceGroupPeerMemoryFeatures(device, heapIndex, localDeviceIndex, remoteDeviceIndex,
                                                           pPeerMemoryFeatures);
}

void DispatchDevice::CmdSetDeviceMask(VkCommandBuffer commandBuffer, uint32_t deviceMask) {
    device_dispatch_table.CmdSetDeviceMask(commandBuffer, deviceMask);
}

VkResult DispatchInstance::EnumeratePhysicalDeviceGroups(VkInstance instance, uint32_t* pPhysicalDeviceGroupCount,
                                                         VkPhysicalDeviceGroupProperties* pPhysicalDeviceGroupProperties) {
    VkResult result =
        instance_dispatch_table.EnumeratePhysicalDeviceGroups(instance, pPhysicalDeviceGroupCount, pPhysicalDeviceGroupProperties);

    return result;
}

void DispatchDevice::GetImageMemoryRequirements2(VkDevice device, const VkImageMemoryRequirementsInfo2* pInfo,
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

void DispatchDevice::GetBufferMemoryRequirements2(VkDevice device, const VkBufferMemoryRequirementsInfo2* pInfo,
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

void DispatchDevice::GetImageSparseMemoryRequirements2(VkDevice device, const VkImageSparseMemoryRequirementsInfo2* pInfo,
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

void DispatchInstance::GetPhysicalDeviceFeatures2(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures2* pFeatures) {
    instance_dispatch_table.GetPhysicalDeviceFeatures2(physicalDevice, pFeatures);
}

void DispatchInstance::GetPhysicalDeviceProperties2(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties2* pProperties) {
    instance_dispatch_table.GetPhysicalDeviceProperties2(physicalDevice, pProperties);
}

void DispatchInstance::GetPhysicalDeviceFormatProperties2(VkPhysicalDevice physicalDevice, VkFormat format,
                                                          VkFormatProperties2* pFormatProperties) {
    instance_dispatch_table.GetPhysicalDeviceFormatProperties2(physicalDevice, format, pFormatProperties);
}

VkResult DispatchInstance::GetPhysicalDeviceImageFormatProperties2(VkPhysicalDevice physicalDevice,
                                                                   const VkPhysicalDeviceImageFormatInfo2* pImageFormatInfo,
                                                                   VkImageFormatProperties2* pImageFormatProperties) {
    VkResult result =
        instance_dispatch_table.GetPhysicalDeviceImageFormatProperties2(physicalDevice, pImageFormatInfo, pImageFormatProperties);

    return result;
}

void DispatchInstance::GetPhysicalDeviceQueueFamilyProperties2(VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount,
                                                               VkQueueFamilyProperties2* pQueueFamilyProperties) {
    instance_dispatch_table.GetPhysicalDeviceQueueFamilyProperties2(physicalDevice, pQueueFamilyPropertyCount,
                                                                    pQueueFamilyProperties);
}

void DispatchInstance::GetPhysicalDeviceMemoryProperties2(VkPhysicalDevice physicalDevice,
                                                          VkPhysicalDeviceMemoryProperties2* pMemoryProperties) {
    instance_dispatch_table.GetPhysicalDeviceMemoryProperties2(physicalDevice, pMemoryProperties);
}

void DispatchInstance::GetPhysicalDeviceSparseImageFormatProperties2(VkPhysicalDevice physicalDevice,
                                                                     const VkPhysicalDeviceSparseImageFormatInfo2* pFormatInfo,
                                                                     uint32_t* pPropertyCount,
                                                                     VkSparseImageFormatProperties2* pProperties) {
    instance_dispatch_table.GetPhysicalDeviceSparseImageFormatProperties2(physicalDevice, pFormatInfo, pPropertyCount, pProperties);
}

void DispatchDevice::TrimCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolTrimFlags flags) {
    if (!wrap_handles) return device_dispatch_table.TrimCommandPool(device, commandPool, flags);
    {
        commandPool = Unwrap(commandPool);
    }
    device_dispatch_table.TrimCommandPool(device, commandPool, flags);
}

void DispatchDevice::GetDeviceQueue2(VkDevice device, const VkDeviceQueueInfo2* pQueueInfo, VkQueue* pQueue) {
    device_dispatch_table.GetDeviceQueue2(device, pQueueInfo, pQueue);
}

void DispatchInstance::GetPhysicalDeviceExternalBufferProperties(VkPhysicalDevice physicalDevice,
                                                                 const VkPhysicalDeviceExternalBufferInfo* pExternalBufferInfo,
                                                                 VkExternalBufferProperties* pExternalBufferProperties) {
    instance_dispatch_table.GetPhysicalDeviceExternalBufferProperties(physicalDevice, pExternalBufferInfo,
                                                                      pExternalBufferProperties);
}

void DispatchInstance::GetPhysicalDeviceExternalFenceProperties(VkPhysicalDevice physicalDevice,
                                                                const VkPhysicalDeviceExternalFenceInfo* pExternalFenceInfo,
                                                                VkExternalFenceProperties* pExternalFenceProperties) {
    instance_dispatch_table.GetPhysicalDeviceExternalFenceProperties(physicalDevice, pExternalFenceInfo, pExternalFenceProperties);
}

void DispatchInstance::GetPhysicalDeviceExternalSemaphoreProperties(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalSemaphoreInfo* pExternalSemaphoreInfo,
    VkExternalSemaphoreProperties* pExternalSemaphoreProperties) {
    instance_dispatch_table.GetPhysicalDeviceExternalSemaphoreProperties(physicalDevice, pExternalSemaphoreInfo,
                                                                         pExternalSemaphoreProperties);
}

void DispatchDevice::CmdDispatchBase(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY, uint32_t baseGroupZ,
                                     uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) {
    device_dispatch_table.CmdDispatchBase(commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ);
}

void DispatchDevice::GetDescriptorSetLayoutSupport(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo,
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

VkResult DispatchDevice::CreateSamplerYcbcrConversion(VkDevice device, const VkSamplerYcbcrConversionCreateInfo* pCreateInfo,
                                                      const VkAllocationCallbacks* pAllocator,
                                                      VkSamplerYcbcrConversion* pYcbcrConversion) {
    if (!wrap_handles) return device_dispatch_table.CreateSamplerYcbcrConversion(device, pCreateInfo, pAllocator, pYcbcrConversion);

    VkResult result = device_dispatch_table.CreateSamplerYcbcrConversion(device, pCreateInfo, pAllocator, pYcbcrConversion);
    if (result == VK_SUCCESS) {
        *pYcbcrConversion = WrapNew(*pYcbcrConversion);
    }
    return result;
}

void DispatchDevice::DestroySamplerYcbcrConversion(VkDevice device, VkSamplerYcbcrConversion ycbcrConversion,
                                                   const VkAllocationCallbacks* pAllocator) {
    if (!wrap_handles) return device_dispatch_table.DestroySamplerYcbcrConversion(device, ycbcrConversion, pAllocator);
    ycbcrConversion = Erase(ycbcrConversion);
    device_dispatch_table.DestroySamplerYcbcrConversion(device, ycbcrConversion, pAllocator);
}

void DispatchDevice::ResetQueryPool(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount) {
    if (!wrap_handles) return device_dispatch_table.ResetQueryPool(device, queryPool, firstQuery, queryCount);
    {
        queryPool = Unwrap(queryPool);
    }
    device_dispatch_table.ResetQueryPool(device, queryPool, firstQuery, queryCount);
}

VkResult DispatchDevice::GetSemaphoreCounterValue(VkDevice device, VkSemaphore semaphore, uint64_t* pValue) {
    if (!wrap_handles) return device_dispatch_table.GetSemaphoreCounterValue(device, semaphore, pValue);
    {
        semaphore = Unwrap(semaphore);
    }
    VkResult result = device_dispatch_table.GetSemaphoreCounterValue(device, semaphore, pValue);

    return result;
}

VkResult DispatchDevice::WaitSemaphores(VkDevice device, const VkSemaphoreWaitInfo* pWaitInfo, uint64_t timeout) {
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

VkResult DispatchDevice::SignalSemaphore(VkDevice device, const VkSemaphoreSignalInfo* pSignalInfo) {
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

VkDeviceAddress DispatchDevice::GetBufferDeviceAddress(VkDevice device, const VkBufferDeviceAddressInfo* pInfo) {
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

uint64_t DispatchDevice::GetBufferOpaqueCaptureAddress(VkDevice device, const VkBufferDeviceAddressInfo* pInfo) {
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

uint64_t DispatchDevice::GetDeviceMemoryOpaqueCaptureAddress(VkDevice device, const VkDeviceMemoryOpaqueCaptureAddressInfo* pInfo) {
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

void DispatchDevice::CmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer,
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

void DispatchDevice::CmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
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

void DispatchDevice::CmdBeginRenderPass2(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
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

void DispatchDevice::CmdNextSubpass2(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo* pSubpassBeginInfo,
                                     const VkSubpassEndInfo* pSubpassEndInfo) {
    device_dispatch_table.CmdNextSubpass2(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo);
}

void DispatchDevice::CmdEndRenderPass2(VkCommandBuffer commandBuffer, const VkSubpassEndInfo* pSubpassEndInfo) {
    device_dispatch_table.CmdEndRenderPass2(commandBuffer, pSubpassEndInfo);
}

VkResult DispatchDevice::CreatePrivateDataSlot(VkDevice device, const VkPrivateDataSlotCreateInfo* pCreateInfo,
                                               const VkAllocationCallbacks* pAllocator, VkPrivateDataSlot* pPrivateDataSlot) {
    if (!wrap_handles) return device_dispatch_table.CreatePrivateDataSlot(device, pCreateInfo, pAllocator, pPrivateDataSlot);

    VkResult result = device_dispatch_table.CreatePrivateDataSlot(device, pCreateInfo, pAllocator, pPrivateDataSlot);
    if (result == VK_SUCCESS) {
        *pPrivateDataSlot = WrapNew(*pPrivateDataSlot);
    }
    return result;
}

void DispatchDevice::DestroyPrivateDataSlot(VkDevice device, VkPrivateDataSlot privateDataSlot,
                                            const VkAllocationCallbacks* pAllocator) {
    if (!wrap_handles) return device_dispatch_table.DestroyPrivateDataSlot(device, privateDataSlot, pAllocator);
    privateDataSlot = Erase(privateDataSlot);
    device_dispatch_table.DestroyPrivateDataSlot(device, privateDataSlot, pAllocator);
}

VkResult DispatchDevice::SetPrivateData(VkDevice device, VkObjectType objectType, uint64_t objectHandle,
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

void DispatchDevice::GetPrivateData(VkDevice device, VkObjectType objectType, uint64_t objectHandle,
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

void DispatchDevice::CmdPipelineBarrier2(VkCommandBuffer commandBuffer, const VkDependencyInfo* pDependencyInfo) {
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
            UnwrapPnextChainHandles(local_pDependencyInfo->pNext);
        }
    }
    device_dispatch_table.CmdPipelineBarrier2(commandBuffer, (const VkDependencyInfo*)local_pDependencyInfo);
}

void DispatchDevice::CmdWriteTimestamp2(VkCommandBuffer commandBuffer, VkPipelineStageFlags2 stage, VkQueryPool queryPool,
                                        uint32_t query) {
    if (!wrap_handles) return device_dispatch_table.CmdWriteTimestamp2(commandBuffer, stage, queryPool, query);
    {
        queryPool = Unwrap(queryPool);
    }
    device_dispatch_table.CmdWriteTimestamp2(commandBuffer, stage, queryPool, query);
}

VkResult DispatchDevice::QueueSubmit2(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2* pSubmits, VkFence fence) {
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

void DispatchDevice::CmdCopyBuffer2(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2* pCopyBufferInfo) {
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

void DispatchDevice::CmdCopyImage2(VkCommandBuffer commandBuffer, const VkCopyImageInfo2* pCopyImageInfo) {
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

void DispatchDevice::CmdCopyBufferToImage2(VkCommandBuffer commandBuffer, const VkCopyBufferToImageInfo2* pCopyBufferToImageInfo) {
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

void DispatchDevice::CmdCopyImageToBuffer2(VkCommandBuffer commandBuffer, const VkCopyImageToBufferInfo2* pCopyImageToBufferInfo) {
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

void DispatchDevice::GetDeviceBufferMemoryRequirements(VkDevice device, const VkDeviceBufferMemoryRequirements* pInfo,
                                                       VkMemoryRequirements2* pMemoryRequirements) {
    device_dispatch_table.GetDeviceBufferMemoryRequirements(device, pInfo, pMemoryRequirements);
}

void DispatchDevice::GetDeviceImageMemoryRequirements(VkDevice device, const VkDeviceImageMemoryRequirements* pInfo,
                                                      VkMemoryRequirements2* pMemoryRequirements) {
    device_dispatch_table.GetDeviceImageMemoryRequirements(device, pInfo, pMemoryRequirements);
}

void DispatchDevice::GetDeviceImageSparseMemoryRequirements(VkDevice device, const VkDeviceImageMemoryRequirements* pInfo,
                                                            uint32_t* pSparseMemoryRequirementCount,
                                                            VkSparseImageMemoryRequirements2* pSparseMemoryRequirements) {
    device_dispatch_table.GetDeviceImageSparseMemoryRequirements(device, pInfo, pSparseMemoryRequirementCount,
                                                                 pSparseMemoryRequirements);
}

void DispatchDevice::CmdSetEvent2(VkCommandBuffer commandBuffer, VkEvent event, const VkDependencyInfo* pDependencyInfo) {
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
            UnwrapPnextChainHandles(local_pDependencyInfo->pNext);
        }
    }
    device_dispatch_table.CmdSetEvent2(commandBuffer, event, (const VkDependencyInfo*)local_pDependencyInfo);
}

void DispatchDevice::CmdResetEvent2(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags2 stageMask) {
    if (!wrap_handles) return device_dispatch_table.CmdResetEvent2(commandBuffer, event, stageMask);
    {
        event = Unwrap(event);
    }
    device_dispatch_table.CmdResetEvent2(commandBuffer, event, stageMask);
}

void DispatchDevice::CmdWaitEvents2(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
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
                UnwrapPnextChainHandles(local_pDependencyInfos[index0].pNext);
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

void DispatchDevice::CmdBlitImage2(VkCommandBuffer commandBuffer, const VkBlitImageInfo2* pBlitImageInfo) {
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

void DispatchDevice::CmdResolveImage2(VkCommandBuffer commandBuffer, const VkResolveImageInfo2* pResolveImageInfo) {
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

void DispatchDevice::CmdBeginRendering(VkCommandBuffer commandBuffer, const VkRenderingInfo* pRenderingInfo) {
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

void DispatchDevice::CmdEndRendering(VkCommandBuffer commandBuffer) { device_dispatch_table.CmdEndRendering(commandBuffer); }

void DispatchDevice::CmdSetCullMode(VkCommandBuffer commandBuffer, VkCullModeFlags cullMode) {
    device_dispatch_table.CmdSetCullMode(commandBuffer, cullMode);
}

void DispatchDevice::CmdSetFrontFace(VkCommandBuffer commandBuffer, VkFrontFace frontFace) {
    device_dispatch_table.CmdSetFrontFace(commandBuffer, frontFace);
}

void DispatchDevice::CmdSetPrimitiveTopology(VkCommandBuffer commandBuffer, VkPrimitiveTopology primitiveTopology) {
    device_dispatch_table.CmdSetPrimitiveTopology(commandBuffer, primitiveTopology);
}

void DispatchDevice::CmdSetViewportWithCount(VkCommandBuffer commandBuffer, uint32_t viewportCount, const VkViewport* pViewports) {
    device_dispatch_table.CmdSetViewportWithCount(commandBuffer, viewportCount, pViewports);
}

void DispatchDevice::CmdSetScissorWithCount(VkCommandBuffer commandBuffer, uint32_t scissorCount, const VkRect2D* pScissors) {
    device_dispatch_table.CmdSetScissorWithCount(commandBuffer, scissorCount, pScissors);
}

void DispatchDevice::CmdBindVertexBuffers2(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount,
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

void DispatchDevice::CmdSetDepthTestEnable(VkCommandBuffer commandBuffer, VkBool32 depthTestEnable) {
    device_dispatch_table.CmdSetDepthTestEnable(commandBuffer, depthTestEnable);
}

void DispatchDevice::CmdSetDepthWriteEnable(VkCommandBuffer commandBuffer, VkBool32 depthWriteEnable) {
    device_dispatch_table.CmdSetDepthWriteEnable(commandBuffer, depthWriteEnable);
}

void DispatchDevice::CmdSetDepthCompareOp(VkCommandBuffer commandBuffer, VkCompareOp depthCompareOp) {
    device_dispatch_table.CmdSetDepthCompareOp(commandBuffer, depthCompareOp);
}

void DispatchDevice::CmdSetDepthBoundsTestEnable(VkCommandBuffer commandBuffer, VkBool32 depthBoundsTestEnable) {
    device_dispatch_table.CmdSetDepthBoundsTestEnable(commandBuffer, depthBoundsTestEnable);
}

void DispatchDevice::CmdSetStencilTestEnable(VkCommandBuffer commandBuffer, VkBool32 stencilTestEnable) {
    device_dispatch_table.CmdSetStencilTestEnable(commandBuffer, stencilTestEnable);
}

void DispatchDevice::CmdSetStencilOp(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, VkStencilOp failOp,
                                     VkStencilOp passOp, VkStencilOp depthFailOp, VkCompareOp compareOp) {
    device_dispatch_table.CmdSetStencilOp(commandBuffer, faceMask, failOp, passOp, depthFailOp, compareOp);
}

void DispatchDevice::CmdSetRasterizerDiscardEnable(VkCommandBuffer commandBuffer, VkBool32 rasterizerDiscardEnable) {
    device_dispatch_table.CmdSetRasterizerDiscardEnable(commandBuffer, rasterizerDiscardEnable);
}

void DispatchDevice::CmdSetDepthBiasEnable(VkCommandBuffer commandBuffer, VkBool32 depthBiasEnable) {
    device_dispatch_table.CmdSetDepthBiasEnable(commandBuffer, depthBiasEnable);
}

void DispatchDevice::CmdSetPrimitiveRestartEnable(VkCommandBuffer commandBuffer, VkBool32 primitiveRestartEnable) {
    device_dispatch_table.CmdSetPrimitiveRestartEnable(commandBuffer, primitiveRestartEnable);
}

VkResult DispatchDevice::MapMemory2(VkDevice device, const VkMemoryMapInfo* pMemoryMapInfo, void** ppData) {
    if (!wrap_handles) return device_dispatch_table.MapMemory2(device, pMemoryMapInfo, ppData);
    vku::safe_VkMemoryMapInfo var_local_pMemoryMapInfo;
    vku::safe_VkMemoryMapInfo* local_pMemoryMapInfo = nullptr;
    {
        if (pMemoryMapInfo) {
            local_pMemoryMapInfo = &var_local_pMemoryMapInfo;
            local_pMemoryMapInfo->initialize(pMemoryMapInfo);

            if (pMemoryMapInfo->memory) {
                local_pMemoryMapInfo->memory = Unwrap(pMemoryMapInfo->memory);
            }
        }
    }
    VkResult result = device_dispatch_table.MapMemory2(device, (const VkMemoryMapInfo*)local_pMemoryMapInfo, ppData);

    return result;
}

VkResult DispatchDevice::UnmapMemory2(VkDevice device, const VkMemoryUnmapInfo* pMemoryUnmapInfo) {
    if (!wrap_handles) return device_dispatch_table.UnmapMemory2(device, pMemoryUnmapInfo);
    vku::safe_VkMemoryUnmapInfo var_local_pMemoryUnmapInfo;
    vku::safe_VkMemoryUnmapInfo* local_pMemoryUnmapInfo = nullptr;
    {
        if (pMemoryUnmapInfo) {
            local_pMemoryUnmapInfo = &var_local_pMemoryUnmapInfo;
            local_pMemoryUnmapInfo->initialize(pMemoryUnmapInfo);

            if (pMemoryUnmapInfo->memory) {
                local_pMemoryUnmapInfo->memory = Unwrap(pMemoryUnmapInfo->memory);
            }
        }
    }
    VkResult result = device_dispatch_table.UnmapMemory2(device, (const VkMemoryUnmapInfo*)local_pMemoryUnmapInfo);

    return result;
}

void DispatchDevice::GetDeviceImageSubresourceLayout(VkDevice device, const VkDeviceImageSubresourceInfo* pInfo,
                                                     VkSubresourceLayout2* pLayout) {
    device_dispatch_table.GetDeviceImageSubresourceLayout(device, pInfo, pLayout);
}

void DispatchDevice::GetImageSubresourceLayout2(VkDevice device, VkImage image, const VkImageSubresource2* pSubresource,
                                                VkSubresourceLayout2* pLayout) {
    if (!wrap_handles) return device_dispatch_table.GetImageSubresourceLayout2(device, image, pSubresource, pLayout);
    {
        image = Unwrap(image);
    }
    device_dispatch_table.GetImageSubresourceLayout2(device, image, pSubresource, pLayout);
}

VkResult DispatchDevice::CopyMemoryToImage(VkDevice device, const VkCopyMemoryToImageInfo* pCopyMemoryToImageInfo) {
    if (!wrap_handles) return device_dispatch_table.CopyMemoryToImage(device, pCopyMemoryToImageInfo);
    vku::safe_VkCopyMemoryToImageInfo var_local_pCopyMemoryToImageInfo;
    vku::safe_VkCopyMemoryToImageInfo* local_pCopyMemoryToImageInfo = nullptr;
    {
        if (pCopyMemoryToImageInfo) {
            local_pCopyMemoryToImageInfo = &var_local_pCopyMemoryToImageInfo;
            local_pCopyMemoryToImageInfo->initialize(pCopyMemoryToImageInfo);

            if (pCopyMemoryToImageInfo->dstImage) {
                local_pCopyMemoryToImageInfo->dstImage = Unwrap(pCopyMemoryToImageInfo->dstImage);
            }
        }
    }
    VkResult result = device_dispatch_table.CopyMemoryToImage(device, (const VkCopyMemoryToImageInfo*)local_pCopyMemoryToImageInfo);

    return result;
}

VkResult DispatchDevice::CopyImageToMemory(VkDevice device, const VkCopyImageToMemoryInfo* pCopyImageToMemoryInfo) {
    if (!wrap_handles) return device_dispatch_table.CopyImageToMemory(device, pCopyImageToMemoryInfo);
    vku::safe_VkCopyImageToMemoryInfo var_local_pCopyImageToMemoryInfo;
    vku::safe_VkCopyImageToMemoryInfo* local_pCopyImageToMemoryInfo = nullptr;
    {
        if (pCopyImageToMemoryInfo) {
            local_pCopyImageToMemoryInfo = &var_local_pCopyImageToMemoryInfo;
            local_pCopyImageToMemoryInfo->initialize(pCopyImageToMemoryInfo);

            if (pCopyImageToMemoryInfo->srcImage) {
                local_pCopyImageToMemoryInfo->srcImage = Unwrap(pCopyImageToMemoryInfo->srcImage);
            }
        }
    }
    VkResult result = device_dispatch_table.CopyImageToMemory(device, (const VkCopyImageToMemoryInfo*)local_pCopyImageToMemoryInfo);

    return result;
}

VkResult DispatchDevice::CopyImageToImage(VkDevice device, const VkCopyImageToImageInfo* pCopyImageToImageInfo) {
    if (!wrap_handles) return device_dispatch_table.CopyImageToImage(device, pCopyImageToImageInfo);
    vku::safe_VkCopyImageToImageInfo var_local_pCopyImageToImageInfo;
    vku::safe_VkCopyImageToImageInfo* local_pCopyImageToImageInfo = nullptr;
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
    VkResult result = device_dispatch_table.CopyImageToImage(device, (const VkCopyImageToImageInfo*)local_pCopyImageToImageInfo);

    return result;
}

VkResult DispatchDevice::TransitionImageLayout(VkDevice device, uint32_t transitionCount,
                                               const VkHostImageLayoutTransitionInfo* pTransitions) {
    if (!wrap_handles) return device_dispatch_table.TransitionImageLayout(device, transitionCount, pTransitions);
    small_vector<vku::safe_VkHostImageLayoutTransitionInfo, DISPATCH_MAX_STACK_ALLOCATIONS> var_local_pTransitions;
    vku::safe_VkHostImageLayoutTransitionInfo* local_pTransitions = nullptr;
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
    VkResult result = device_dispatch_table.TransitionImageLayout(device, transitionCount,
                                                                  (const VkHostImageLayoutTransitionInfo*)local_pTransitions);

    return result;
}

void DispatchDevice::CmdPushDescriptorSet(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                          VkPipelineLayout layout, uint32_t set, uint32_t descriptorWriteCount,
                                          const VkWriteDescriptorSet* pDescriptorWrites) {
    if (!wrap_handles)
        return device_dispatch_table.CmdPushDescriptorSet(commandBuffer, pipelineBindPoint, layout, set, descriptorWriteCount,
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
                    // need for when updating VkDescriptorImageInfo
                    bool has_sampler =
                        local_pDescriptorWrites[index0].descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER ||
                        local_pDescriptorWrites[index0].descriptorType == VK_DESCRIPTOR_TYPE_SAMPLER;
                    bool has_image_view = local_pDescriptorWrites[index0].descriptorType != VK_DESCRIPTOR_TYPE_SAMPLER;

                    for (uint32_t index1 = 0; index1 < local_pDescriptorWrites[index0].descriptorCount; ++index1) {
                        if (pDescriptorWrites[index0].pImageInfo[index1].sampler && has_sampler) {
                            local_pDescriptorWrites[index0].pImageInfo[index1].sampler =
                                Unwrap(pDescriptorWrites[index0].pImageInfo[index1].sampler);
                        }
                        if (pDescriptorWrites[index0].pImageInfo[index1].imageView && has_image_view) {
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
    device_dispatch_table.CmdPushDescriptorSet(commandBuffer, pipelineBindPoint, layout, set, descriptorWriteCount,
                                               (const VkWriteDescriptorSet*)local_pDescriptorWrites);
}

void DispatchDevice::CmdBindDescriptorSets2(VkCommandBuffer commandBuffer,
                                            const VkBindDescriptorSetsInfo* pBindDescriptorSetsInfo) {
    if (!wrap_handles) return device_dispatch_table.CmdBindDescriptorSets2(commandBuffer, pBindDescriptorSetsInfo);
    vku::safe_VkBindDescriptorSetsInfo var_local_pBindDescriptorSetsInfo;
    vku::safe_VkBindDescriptorSetsInfo* local_pBindDescriptorSetsInfo = nullptr;
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
    device_dispatch_table.CmdBindDescriptorSets2(commandBuffer, (const VkBindDescriptorSetsInfo*)local_pBindDescriptorSetsInfo);
}

void DispatchDevice::CmdPushConstants2(VkCommandBuffer commandBuffer, const VkPushConstantsInfo* pPushConstantsInfo) {
    if (!wrap_handles) return device_dispatch_table.CmdPushConstants2(commandBuffer, pPushConstantsInfo);
    vku::safe_VkPushConstantsInfo var_local_pPushConstantsInfo;
    vku::safe_VkPushConstantsInfo* local_pPushConstantsInfo = nullptr;
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
    device_dispatch_table.CmdPushConstants2(commandBuffer, (const VkPushConstantsInfo*)local_pPushConstantsInfo);
}

void DispatchDevice::CmdPushDescriptorSet2(VkCommandBuffer commandBuffer, const VkPushDescriptorSetInfo* pPushDescriptorSetInfo) {
    if (!wrap_handles) return device_dispatch_table.CmdPushDescriptorSet2(commandBuffer, pPushDescriptorSetInfo);
    vku::safe_VkPushDescriptorSetInfo var_local_pPushDescriptorSetInfo;
    vku::safe_VkPushDescriptorSetInfo* local_pPushDescriptorSetInfo = nullptr;
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
                        // need for when updating VkDescriptorImageInfo
                        bool has_sampler =
                            local_pPushDescriptorSetInfo->pDescriptorWrites[index1].descriptorType ==
                                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER ||
                            local_pPushDescriptorSetInfo->pDescriptorWrites[index1].descriptorType == VK_DESCRIPTOR_TYPE_SAMPLER;
                        bool has_image_view =
                            local_pPushDescriptorSetInfo->pDescriptorWrites[index1].descriptorType != VK_DESCRIPTOR_TYPE_SAMPLER;

                        for (uint32_t index2 = 0; index2 < local_pPushDescriptorSetInfo->pDescriptorWrites[index1].descriptorCount;
                             ++index2) {
                            if (pPushDescriptorSetInfo->pDescriptorWrites[index1].pImageInfo[index2].sampler && has_sampler) {
                                local_pPushDescriptorSetInfo->pDescriptorWrites[index1].pImageInfo[index2].sampler =
                                    Unwrap(pPushDescriptorSetInfo->pDescriptorWrites[index1].pImageInfo[index2].sampler);
                            }
                            if (pPushDescriptorSetInfo->pDescriptorWrites[index1].pImageInfo[index2].imageView && has_image_view) {
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
    device_dispatch_table.CmdPushDescriptorSet2(commandBuffer, (const VkPushDescriptorSetInfo*)local_pPushDescriptorSetInfo);
}

void DispatchDevice::CmdSetLineStipple(VkCommandBuffer commandBuffer, uint32_t lineStippleFactor, uint16_t lineStipplePattern) {
    device_dispatch_table.CmdSetLineStipple(commandBuffer, lineStippleFactor, lineStipplePattern);
}

void DispatchDevice::CmdBindIndexBuffer2(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkDeviceSize size,
                                         VkIndexType indexType) {
    if (!wrap_handles) return device_dispatch_table.CmdBindIndexBuffer2(commandBuffer, buffer, offset, size, indexType);
    {
        buffer = Unwrap(buffer);
    }
    device_dispatch_table.CmdBindIndexBuffer2(commandBuffer, buffer, offset, size, indexType);
}

void DispatchDevice::GetRenderingAreaGranularity(VkDevice device, const VkRenderingAreaInfo* pRenderingAreaInfo,
                                                 VkExtent2D* pGranularity) {
    device_dispatch_table.GetRenderingAreaGranularity(device, pRenderingAreaInfo, pGranularity);
}

void DispatchDevice::CmdSetRenderingAttachmentLocations(VkCommandBuffer commandBuffer,
                                                        const VkRenderingAttachmentLocationInfo* pLocationInfo) {
    device_dispatch_table.CmdSetRenderingAttachmentLocations(commandBuffer, pLocationInfo);
}

void DispatchDevice::CmdSetRenderingInputAttachmentIndices(VkCommandBuffer commandBuffer,
                                                           const VkRenderingInputAttachmentIndexInfo* pInputAttachmentIndexInfo) {
    device_dispatch_table.CmdSetRenderingInputAttachmentIndices(commandBuffer, pInputAttachmentIndexInfo);
}

void DispatchInstance::DestroySurfaceKHR(VkInstance instance, VkSurfaceKHR surface, const VkAllocationCallbacks* pAllocator) {
    if (!wrap_handles) return instance_dispatch_table.DestroySurfaceKHR(instance, surface, pAllocator);
    surface = Erase(surface);
    instance_dispatch_table.DestroySurfaceKHR(instance, surface, pAllocator);
}

VkResult DispatchInstance::GetPhysicalDeviceSurfaceSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
                                                              VkSurfaceKHR surface, VkBool32* pSupported) {
    if (!wrap_handles)
        return instance_dispatch_table.GetPhysicalDeviceSurfaceSupportKHR(physicalDevice, queueFamilyIndex, surface, pSupported);
    {
        surface = Unwrap(surface);
    }
    VkResult result =
        instance_dispatch_table.GetPhysicalDeviceSurfaceSupportKHR(physicalDevice, queueFamilyIndex, surface, pSupported);

    return result;
}

VkResult DispatchInstance::GetPhysicalDeviceSurfaceCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                   VkSurfaceCapabilitiesKHR* pSurfaceCapabilities) {
    if (!wrap_handles)
        return instance_dispatch_table.GetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, pSurfaceCapabilities);
    {
        surface = Unwrap(surface);
    }
    VkResult result =
        instance_dispatch_table.GetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, pSurfaceCapabilities);

    return result;
}

VkResult DispatchInstance::GetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                              uint32_t* pSurfaceFormatCount, VkSurfaceFormatKHR* pSurfaceFormats) {
    if (!wrap_handles)
        return instance_dispatch_table.GetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, pSurfaceFormatCount,
                                                                          pSurfaceFormats);
    {
        surface = Unwrap(surface);
    }
    VkResult result =
        instance_dispatch_table.GetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, pSurfaceFormatCount, pSurfaceFormats);

    return result;
}

VkResult DispatchInstance::GetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                   uint32_t* pPresentModeCount, VkPresentModeKHR* pPresentModes) {
    if (!wrap_handles)
        return instance_dispatch_table.GetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, pPresentModeCount,
                                                                               pPresentModes);
    {
        surface = Unwrap(surface);
    }
    VkResult result =
        instance_dispatch_table.GetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, pPresentModeCount, pPresentModes);

    return result;
}

VkResult DispatchDevice::CreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR* pCreateInfo,
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
    if (result == VK_SUCCESS) {
        *pSwapchain = WrapNew(*pSwapchain);
    }
    return result;
}

VkResult DispatchDevice::AcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore,
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

VkResult DispatchDevice::GetDeviceGroupPresentCapabilitiesKHR(
    VkDevice device, VkDeviceGroupPresentCapabilitiesKHR* pDeviceGroupPresentCapabilities) {
    VkResult result = device_dispatch_table.GetDeviceGroupPresentCapabilitiesKHR(device, pDeviceGroupPresentCapabilities);

    return result;
}

VkResult DispatchDevice::GetDeviceGroupSurfacePresentModesKHR(VkDevice device, VkSurfaceKHR surface,
                                                              VkDeviceGroupPresentModeFlagsKHR* pModes) {
    if (!wrap_handles) return device_dispatch_table.GetDeviceGroupSurfacePresentModesKHR(device, surface, pModes);
    {
        surface = Unwrap(surface);
    }
    VkResult result = device_dispatch_table.GetDeviceGroupSurfacePresentModesKHR(device, surface, pModes);

    return result;
}

VkResult DispatchInstance::GetPhysicalDevicePresentRectanglesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                 uint32_t* pRectCount, VkRect2D* pRects) {
    if (!wrap_handles)
        return instance_dispatch_table.GetPhysicalDevicePresentRectanglesKHR(physicalDevice, surface, pRectCount, pRects);
    {
        surface = Unwrap(surface);
    }
    VkResult result = instance_dispatch_table.GetPhysicalDevicePresentRectanglesKHR(physicalDevice, surface, pRectCount, pRects);

    return result;
}

VkResult DispatchDevice::AcquireNextImage2KHR(VkDevice device, const VkAcquireNextImageInfoKHR* pAcquireInfo,
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

VkResult DispatchInstance::CreateDisplayModeKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display,
                                                const VkDisplayModeCreateInfoKHR* pCreateInfo,
                                                const VkAllocationCallbacks* pAllocator, VkDisplayModeKHR* pMode) {
    if (!wrap_handles) return instance_dispatch_table.CreateDisplayModeKHR(physicalDevice, display, pCreateInfo, pAllocator, pMode);
    {
        display = Unwrap(display);
    }
    VkResult result = instance_dispatch_table.CreateDisplayModeKHR(physicalDevice, display, pCreateInfo, pAllocator, pMode);
    if (result == VK_SUCCESS) {
        *pMode = WrapNew(*pMode);
    }
    return result;
}

VkResult DispatchInstance::GetDisplayPlaneCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkDisplayModeKHR mode,
                                                          uint32_t planeIndex, VkDisplayPlaneCapabilitiesKHR* pCapabilities) {
    if (!wrap_handles)
        return instance_dispatch_table.GetDisplayPlaneCapabilitiesKHR(physicalDevice, mode, planeIndex, pCapabilities);
    {
        mode = Unwrap(mode);
    }
    VkResult result = instance_dispatch_table.GetDisplayPlaneCapabilitiesKHR(physicalDevice, mode, planeIndex, pCapabilities);

    return result;
}

VkResult DispatchInstance::CreateDisplayPlaneSurfaceKHR(VkInstance instance, const VkDisplaySurfaceCreateInfoKHR* pCreateInfo,
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
    if (result == VK_SUCCESS) {
        *pSurface = WrapNew(*pSurface);
    }
    return result;
}

VkResult DispatchDevice::CreateSharedSwapchainsKHR(VkDevice device, uint32_t swapchainCount,
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
    if (result == VK_SUCCESS) {
        for (uint32_t index0 = 0; index0 < swapchainCount; index0++) {
            pSwapchains[index0] = WrapNew(pSwapchains[index0]);
        }
    }
    return result;
}
#ifdef VK_USE_PLATFORM_XLIB_KHR

VkResult DispatchInstance::CreateXlibSurfaceKHR(VkInstance instance, const VkXlibSurfaceCreateInfoKHR* pCreateInfo,
                                                const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    if (!wrap_handles) return instance_dispatch_table.CreateXlibSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);

    VkResult result = instance_dispatch_table.CreateXlibSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
    if (result == VK_SUCCESS) {
        *pSurface = WrapNew(*pSurface);
    }
    return result;
}

VkBool32 DispatchInstance::GetPhysicalDeviceXlibPresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
                                                                       Display* dpy, VisualID visualID) {
    VkBool32 result =
        instance_dispatch_table.GetPhysicalDeviceXlibPresentationSupportKHR(physicalDevice, queueFamilyIndex, dpy, visualID);

    return result;
}
#endif  // VK_USE_PLATFORM_XLIB_KHR
#ifdef VK_USE_PLATFORM_XCB_KHR

VkResult DispatchInstance::CreateXcbSurfaceKHR(VkInstance instance, const VkXcbSurfaceCreateInfoKHR* pCreateInfo,
                                               const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    if (!wrap_handles) return instance_dispatch_table.CreateXcbSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);

    VkResult result = instance_dispatch_table.CreateXcbSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
    if (result == VK_SUCCESS) {
        *pSurface = WrapNew(*pSurface);
    }
    return result;
}

VkBool32 DispatchInstance::GetPhysicalDeviceXcbPresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
                                                                      xcb_connection_t* connection, xcb_visualid_t visual_id) {
    VkBool32 result =
        instance_dispatch_table.GetPhysicalDeviceXcbPresentationSupportKHR(physicalDevice, queueFamilyIndex, connection, visual_id);

    return result;
}
#endif  // VK_USE_PLATFORM_XCB_KHR
#ifdef VK_USE_PLATFORM_WAYLAND_KHR

VkResult DispatchInstance::CreateWaylandSurfaceKHR(VkInstance instance, const VkWaylandSurfaceCreateInfoKHR* pCreateInfo,
                                                   const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    if (!wrap_handles) return instance_dispatch_table.CreateWaylandSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);

    VkResult result = instance_dispatch_table.CreateWaylandSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
    if (result == VK_SUCCESS) {
        *pSurface = WrapNew(*pSurface);
    }
    return result;
}

VkBool32 DispatchInstance::GetPhysicalDeviceWaylandPresentationSupportKHR(VkPhysicalDevice physicalDevice,
                                                                          uint32_t queueFamilyIndex, struct wl_display* display) {
    VkBool32 result =
        instance_dispatch_table.GetPhysicalDeviceWaylandPresentationSupportKHR(physicalDevice, queueFamilyIndex, display);

    return result;
}
#endif  // VK_USE_PLATFORM_WAYLAND_KHR
#ifdef VK_USE_PLATFORM_ANDROID_KHR

VkResult DispatchInstance::CreateAndroidSurfaceKHR(VkInstance instance, const VkAndroidSurfaceCreateInfoKHR* pCreateInfo,
                                                   const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    if (!wrap_handles) return instance_dispatch_table.CreateAndroidSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);

    VkResult result = instance_dispatch_table.CreateAndroidSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
    if (result == VK_SUCCESS) {
        *pSurface = WrapNew(*pSurface);
    }
    return result;
}
#endif  // VK_USE_PLATFORM_ANDROID_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR

VkResult DispatchInstance::CreateWin32SurfaceKHR(VkInstance instance, const VkWin32SurfaceCreateInfoKHR* pCreateInfo,
                                                 const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    if (!wrap_handles) return instance_dispatch_table.CreateWin32SurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);

    VkResult result = instance_dispatch_table.CreateWin32SurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
    if (result == VK_SUCCESS) {
        *pSurface = WrapNew(*pSurface);
    }
    return result;
}

VkBool32 DispatchInstance::GetPhysicalDeviceWin32PresentationSupportKHR(VkPhysicalDevice physicalDevice,
                                                                        uint32_t queueFamilyIndex) {
    VkBool32 result = instance_dispatch_table.GetPhysicalDeviceWin32PresentationSupportKHR(physicalDevice, queueFamilyIndex);

    return result;
}
#endif  // VK_USE_PLATFORM_WIN32_KHR

VkResult DispatchInstance::GetPhysicalDeviceVideoCapabilitiesKHR(VkPhysicalDevice physicalDevice,
                                                                 const VkVideoProfileInfoKHR* pVideoProfile,
                                                                 VkVideoCapabilitiesKHR* pCapabilities) {
    VkResult result = instance_dispatch_table.GetPhysicalDeviceVideoCapabilitiesKHR(physicalDevice, pVideoProfile, pCapabilities);

    return result;
}

VkResult DispatchInstance::GetPhysicalDeviceVideoFormatPropertiesKHR(VkPhysicalDevice physicalDevice,
                                                                     const VkPhysicalDeviceVideoFormatInfoKHR* pVideoFormatInfo,
                                                                     uint32_t* pVideoFormatPropertyCount,
                                                                     VkVideoFormatPropertiesKHR* pVideoFormatProperties) {
    VkResult result = instance_dispatch_table.GetPhysicalDeviceVideoFormatPropertiesKHR(
        physicalDevice, pVideoFormatInfo, pVideoFormatPropertyCount, pVideoFormatProperties);

    return result;
}

VkResult DispatchDevice::CreateVideoSessionKHR(VkDevice device, const VkVideoSessionCreateInfoKHR* pCreateInfo,
                                               const VkAllocationCallbacks* pAllocator, VkVideoSessionKHR* pVideoSession) {
    if (!wrap_handles) return device_dispatch_table.CreateVideoSessionKHR(device, pCreateInfo, pAllocator, pVideoSession);

    VkResult result = device_dispatch_table.CreateVideoSessionKHR(device, pCreateInfo, pAllocator, pVideoSession);
    if (result == VK_SUCCESS) {
        *pVideoSession = WrapNew(*pVideoSession);
    }
    return result;
}

void DispatchDevice::DestroyVideoSessionKHR(VkDevice device, VkVideoSessionKHR videoSession,
                                            const VkAllocationCallbacks* pAllocator) {
    if (!wrap_handles) return device_dispatch_table.DestroyVideoSessionKHR(device, videoSession, pAllocator);
    videoSession = Erase(videoSession);
    device_dispatch_table.DestroyVideoSessionKHR(device, videoSession, pAllocator);
}

VkResult DispatchDevice::GetVideoSessionMemoryRequirementsKHR(VkDevice device, VkVideoSessionKHR videoSession,
                                                              uint32_t* pMemoryRequirementsCount,
                                                              VkVideoSessionMemoryRequirementsKHR* pMemoryRequirements) {
    if (!wrap_handles)
        return device_dispatch_table.GetVideoSessionMemoryRequirementsKHR(device, videoSession, pMemoryRequirementsCount,
                                                                          pMemoryRequirements);
    {
        videoSession = Unwrap(videoSession);
    }
    VkResult result = device_dispatch_table.GetVideoSessionMemoryRequirementsKHR(device, videoSession, pMemoryRequirementsCount,
                                                                                 pMemoryRequirements);

    return result;
}

VkResult DispatchDevice::BindVideoSessionMemoryKHR(VkDevice device, VkVideoSessionKHR videoSession,
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

VkResult DispatchDevice::CreateVideoSessionParametersKHR(VkDevice device, const VkVideoSessionParametersCreateInfoKHR* pCreateInfo,
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
    if (result == VK_SUCCESS) {
        *pVideoSessionParameters = WrapNew(*pVideoSessionParameters);
    }
    return result;
}

VkResult DispatchDevice::UpdateVideoSessionParametersKHR(VkDevice device, VkVideoSessionParametersKHR videoSessionParameters,
                                                         const VkVideoSessionParametersUpdateInfoKHR* pUpdateInfo) {
    if (!wrap_handles) return device_dispatch_table.UpdateVideoSessionParametersKHR(device, videoSessionParameters, pUpdateInfo);
    {
        videoSessionParameters = Unwrap(videoSessionParameters);
    }
    VkResult result = device_dispatch_table.UpdateVideoSessionParametersKHR(device, videoSessionParameters, pUpdateInfo);

    return result;
}

void DispatchDevice::DestroyVideoSessionParametersKHR(VkDevice device, VkVideoSessionParametersKHR videoSessionParameters,
                                                      const VkAllocationCallbacks* pAllocator) {
    if (!wrap_handles) return device_dispatch_table.DestroyVideoSessionParametersKHR(device, videoSessionParameters, pAllocator);
    videoSessionParameters = Erase(videoSessionParameters);
    device_dispatch_table.DestroyVideoSessionParametersKHR(device, videoSessionParameters, pAllocator);
}

void DispatchDevice::CmdBeginVideoCodingKHR(VkCommandBuffer commandBuffer, const VkVideoBeginCodingInfoKHR* pBeginInfo) {
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

void DispatchDevice::CmdEndVideoCodingKHR(VkCommandBuffer commandBuffer, const VkVideoEndCodingInfoKHR* pEndCodingInfo) {
    device_dispatch_table.CmdEndVideoCodingKHR(commandBuffer, pEndCodingInfo);
}

void DispatchDevice::CmdControlVideoCodingKHR(VkCommandBuffer commandBuffer,
                                              const VkVideoCodingControlInfoKHR* pCodingControlInfo) {
    device_dispatch_table.CmdControlVideoCodingKHR(commandBuffer, pCodingControlInfo);
}

void DispatchDevice::CmdDecodeVideoKHR(VkCommandBuffer commandBuffer, const VkVideoDecodeInfoKHR* pDecodeInfo) {
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

void DispatchDevice::CmdBeginRenderingKHR(VkCommandBuffer commandBuffer, const VkRenderingInfo* pRenderingInfo) {
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

void DispatchDevice::CmdEndRenderingKHR(VkCommandBuffer commandBuffer) { device_dispatch_table.CmdEndRenderingKHR(commandBuffer); }

void DispatchInstance::GetPhysicalDeviceFeatures2KHR(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures2* pFeatures) {
    instance_dispatch_table.GetPhysicalDeviceFeatures2KHR(physicalDevice, pFeatures);
}

void DispatchInstance::GetPhysicalDeviceProperties2KHR(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties2* pProperties) {
    instance_dispatch_table.GetPhysicalDeviceProperties2KHR(physicalDevice, pProperties);
}

void DispatchInstance::GetPhysicalDeviceFormatProperties2KHR(VkPhysicalDevice physicalDevice, VkFormat format,
                                                             VkFormatProperties2* pFormatProperties) {
    instance_dispatch_table.GetPhysicalDeviceFormatProperties2KHR(physicalDevice, format, pFormatProperties);
}

VkResult DispatchInstance::GetPhysicalDeviceImageFormatProperties2KHR(VkPhysicalDevice physicalDevice,
                                                                      const VkPhysicalDeviceImageFormatInfo2* pImageFormatInfo,
                                                                      VkImageFormatProperties2* pImageFormatProperties) {
    VkResult result = instance_dispatch_table.GetPhysicalDeviceImageFormatProperties2KHR(physicalDevice, pImageFormatInfo,
                                                                                         pImageFormatProperties);

    return result;
}

void DispatchInstance::GetPhysicalDeviceQueueFamilyProperties2KHR(VkPhysicalDevice physicalDevice,
                                                                  uint32_t* pQueueFamilyPropertyCount,
                                                                  VkQueueFamilyProperties2* pQueueFamilyProperties) {
    instance_dispatch_table.GetPhysicalDeviceQueueFamilyProperties2KHR(physicalDevice, pQueueFamilyPropertyCount,
                                                                       pQueueFamilyProperties);
}

void DispatchInstance::GetPhysicalDeviceMemoryProperties2KHR(VkPhysicalDevice physicalDevice,
                                                             VkPhysicalDeviceMemoryProperties2* pMemoryProperties) {
    instance_dispatch_table.GetPhysicalDeviceMemoryProperties2KHR(physicalDevice, pMemoryProperties);
}

void DispatchInstance::GetPhysicalDeviceSparseImageFormatProperties2KHR(VkPhysicalDevice physicalDevice,
                                                                        const VkPhysicalDeviceSparseImageFormatInfo2* pFormatInfo,
                                                                        uint32_t* pPropertyCount,
                                                                        VkSparseImageFormatProperties2* pProperties) {
    instance_dispatch_table.GetPhysicalDeviceSparseImageFormatProperties2KHR(physicalDevice, pFormatInfo, pPropertyCount,
                                                                             pProperties);
}

void DispatchDevice::GetDeviceGroupPeerMemoryFeaturesKHR(VkDevice device, uint32_t heapIndex, uint32_t localDeviceIndex,
                                                         uint32_t remoteDeviceIndex,
                                                         VkPeerMemoryFeatureFlags* pPeerMemoryFeatures) {
    device_dispatch_table.GetDeviceGroupPeerMemoryFeaturesKHR(device, heapIndex, localDeviceIndex, remoteDeviceIndex,
                                                              pPeerMemoryFeatures);
}

void DispatchDevice::CmdSetDeviceMaskKHR(VkCommandBuffer commandBuffer, uint32_t deviceMask) {
    device_dispatch_table.CmdSetDeviceMaskKHR(commandBuffer, deviceMask);
}

void DispatchDevice::CmdDispatchBaseKHR(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY,
                                        uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) {
    device_dispatch_table.CmdDispatchBaseKHR(commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY,
                                             groupCountZ);
}

void DispatchDevice::TrimCommandPoolKHR(VkDevice device, VkCommandPool commandPool, VkCommandPoolTrimFlags flags) {
    if (!wrap_handles) return device_dispatch_table.TrimCommandPoolKHR(device, commandPool, flags);
    {
        commandPool = Unwrap(commandPool);
    }
    device_dispatch_table.TrimCommandPoolKHR(device, commandPool, flags);
}

VkResult DispatchInstance::EnumeratePhysicalDeviceGroupsKHR(VkInstance instance, uint32_t* pPhysicalDeviceGroupCount,
                                                            VkPhysicalDeviceGroupProperties* pPhysicalDeviceGroupProperties) {
    VkResult result = instance_dispatch_table.EnumeratePhysicalDeviceGroupsKHR(instance, pPhysicalDeviceGroupCount,
                                                                               pPhysicalDeviceGroupProperties);

    return result;
}

void DispatchInstance::GetPhysicalDeviceExternalBufferPropertiesKHR(VkPhysicalDevice physicalDevice,
                                                                    const VkPhysicalDeviceExternalBufferInfo* pExternalBufferInfo,
                                                                    VkExternalBufferProperties* pExternalBufferProperties) {
    instance_dispatch_table.GetPhysicalDeviceExternalBufferPropertiesKHR(physicalDevice, pExternalBufferInfo,
                                                                         pExternalBufferProperties);
}
#ifdef VK_USE_PLATFORM_WIN32_KHR

VkResult DispatchDevice::GetMemoryWin32HandleKHR(VkDevice device, const VkMemoryGetWin32HandleInfoKHR* pGetWin32HandleInfo,
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

VkResult DispatchDevice::GetMemoryWin32HandlePropertiesKHR(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType,
                                                           HANDLE handle,
                                                           VkMemoryWin32HandlePropertiesKHR* pMemoryWin32HandleProperties) {
    VkResult result =
        device_dispatch_table.GetMemoryWin32HandlePropertiesKHR(device, handleType, handle, pMemoryWin32HandleProperties);

    return result;
}
#endif  // VK_USE_PLATFORM_WIN32_KHR

VkResult DispatchDevice::GetMemoryFdKHR(VkDevice device, const VkMemoryGetFdInfoKHR* pGetFdInfo, int* pFd) {
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

VkResult DispatchDevice::GetMemoryFdPropertiesKHR(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType, int fd,
                                                  VkMemoryFdPropertiesKHR* pMemoryFdProperties) {
    VkResult result = device_dispatch_table.GetMemoryFdPropertiesKHR(device, handleType, fd, pMemoryFdProperties);

    return result;
}

void DispatchInstance::GetPhysicalDeviceExternalSemaphorePropertiesKHR(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalSemaphoreInfo* pExternalSemaphoreInfo,
    VkExternalSemaphoreProperties* pExternalSemaphoreProperties) {
    instance_dispatch_table.GetPhysicalDeviceExternalSemaphorePropertiesKHR(physicalDevice, pExternalSemaphoreInfo,
                                                                            pExternalSemaphoreProperties);
}
#ifdef VK_USE_PLATFORM_WIN32_KHR

VkResult DispatchDevice::ImportSemaphoreWin32HandleKHR(VkDevice device,
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

VkResult DispatchDevice::GetSemaphoreWin32HandleKHR(VkDevice device, const VkSemaphoreGetWin32HandleInfoKHR* pGetWin32HandleInfo,
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

VkResult DispatchDevice::ImportSemaphoreFdKHR(VkDevice device, const VkImportSemaphoreFdInfoKHR* pImportSemaphoreFdInfo) {
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

VkResult DispatchDevice::GetSemaphoreFdKHR(VkDevice device, const VkSemaphoreGetFdInfoKHR* pGetFdInfo, int* pFd) {
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

void DispatchDevice::CmdPushDescriptorSetKHR(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
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
                    // need for when updating VkDescriptorImageInfo
                    bool has_sampler =
                        local_pDescriptorWrites[index0].descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER ||
                        local_pDescriptorWrites[index0].descriptorType == VK_DESCRIPTOR_TYPE_SAMPLER;
                    bool has_image_view = local_pDescriptorWrites[index0].descriptorType != VK_DESCRIPTOR_TYPE_SAMPLER;

                    for (uint32_t index1 = 0; index1 < local_pDescriptorWrites[index0].descriptorCount; ++index1) {
                        if (pDescriptorWrites[index0].pImageInfo[index1].sampler && has_sampler) {
                            local_pDescriptorWrites[index0].pImageInfo[index1].sampler =
                                Unwrap(pDescriptorWrites[index0].pImageInfo[index1].sampler);
                        }
                        if (pDescriptorWrites[index0].pImageInfo[index1].imageView && has_image_view) {
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

void DispatchDevice::CmdBeginRenderPass2KHR(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
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

void DispatchDevice::CmdNextSubpass2KHR(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo* pSubpassBeginInfo,
                                        const VkSubpassEndInfo* pSubpassEndInfo) {
    device_dispatch_table.CmdNextSubpass2KHR(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo);
}

void DispatchDevice::CmdEndRenderPass2KHR(VkCommandBuffer commandBuffer, const VkSubpassEndInfo* pSubpassEndInfo) {
    device_dispatch_table.CmdEndRenderPass2KHR(commandBuffer, pSubpassEndInfo);
}

VkResult DispatchDevice::GetSwapchainStatusKHR(VkDevice device, VkSwapchainKHR swapchain) {
    if (!wrap_handles) return device_dispatch_table.GetSwapchainStatusKHR(device, swapchain);
    {
        swapchain = Unwrap(swapchain);
    }
    VkResult result = device_dispatch_table.GetSwapchainStatusKHR(device, swapchain);

    return result;
}

void DispatchInstance::GetPhysicalDeviceExternalFencePropertiesKHR(VkPhysicalDevice physicalDevice,
                                                                   const VkPhysicalDeviceExternalFenceInfo* pExternalFenceInfo,
                                                                   VkExternalFenceProperties* pExternalFenceProperties) {
    instance_dispatch_table.GetPhysicalDeviceExternalFencePropertiesKHR(physicalDevice, pExternalFenceInfo,
                                                                        pExternalFenceProperties);
}
#ifdef VK_USE_PLATFORM_WIN32_KHR

VkResult DispatchDevice::ImportFenceWin32HandleKHR(VkDevice device,
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

VkResult DispatchDevice::GetFenceWin32HandleKHR(VkDevice device, const VkFenceGetWin32HandleInfoKHR* pGetWin32HandleInfo,
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

VkResult DispatchDevice::ImportFenceFdKHR(VkDevice device, const VkImportFenceFdInfoKHR* pImportFenceFdInfo) {
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

VkResult DispatchDevice::GetFenceFdKHR(VkDevice device, const VkFenceGetFdInfoKHR* pGetFdInfo, int* pFd) {
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

VkResult DispatchInstance::EnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(
    VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, uint32_t* pCounterCount, VkPerformanceCounterKHR* pCounters,
    VkPerformanceCounterDescriptionKHR* pCounterDescriptions) {
    VkResult result = instance_dispatch_table.EnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(
        physicalDevice, queueFamilyIndex, pCounterCount, pCounters, pCounterDescriptions);

    return result;
}

void DispatchInstance::GetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR(
    VkPhysicalDevice physicalDevice, const VkQueryPoolPerformanceCreateInfoKHR* pPerformanceQueryCreateInfo, uint32_t* pNumPasses) {
    instance_dispatch_table.GetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR(physicalDevice, pPerformanceQueryCreateInfo,
                                                                                  pNumPasses);
}

VkResult DispatchDevice::AcquireProfilingLockKHR(VkDevice device, const VkAcquireProfilingLockInfoKHR* pInfo) {
    VkResult result = device_dispatch_table.AcquireProfilingLockKHR(device, pInfo);

    return result;
}

void DispatchDevice::ReleaseProfilingLockKHR(VkDevice device) { device_dispatch_table.ReleaseProfilingLockKHR(device); }

VkResult DispatchInstance::GetPhysicalDeviceSurfaceCapabilities2KHR(VkPhysicalDevice physicalDevice,
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

VkResult DispatchInstance::GetPhysicalDeviceSurfaceFormats2KHR(VkPhysicalDevice physicalDevice,
                                                               const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo,
                                                               uint32_t* pSurfaceFormatCount,
                                                               VkSurfaceFormat2KHR* pSurfaceFormats) {
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

VkResult DispatchInstance::GetDisplayPlaneCapabilities2KHR(VkPhysicalDevice physicalDevice,
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

void DispatchDevice::GetImageMemoryRequirements2KHR(VkDevice device, const VkImageMemoryRequirementsInfo2* pInfo,
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

void DispatchDevice::GetBufferMemoryRequirements2KHR(VkDevice device, const VkBufferMemoryRequirementsInfo2* pInfo,
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

void DispatchDevice::GetImageSparseMemoryRequirements2KHR(VkDevice device, const VkImageSparseMemoryRequirementsInfo2* pInfo,
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

VkResult DispatchDevice::CreateSamplerYcbcrConversionKHR(VkDevice device, const VkSamplerYcbcrConversionCreateInfo* pCreateInfo,
                                                         const VkAllocationCallbacks* pAllocator,
                                                         VkSamplerYcbcrConversion* pYcbcrConversion) {
    if (!wrap_handles)
        return device_dispatch_table.CreateSamplerYcbcrConversionKHR(device, pCreateInfo, pAllocator, pYcbcrConversion);

    VkResult result = device_dispatch_table.CreateSamplerYcbcrConversionKHR(device, pCreateInfo, pAllocator, pYcbcrConversion);
    if (result == VK_SUCCESS) {
        *pYcbcrConversion = WrapNew(*pYcbcrConversion);
    }
    return result;
}

void DispatchDevice::DestroySamplerYcbcrConversionKHR(VkDevice device, VkSamplerYcbcrConversion ycbcrConversion,
                                                      const VkAllocationCallbacks* pAllocator) {
    if (!wrap_handles) return device_dispatch_table.DestroySamplerYcbcrConversionKHR(device, ycbcrConversion, pAllocator);
    ycbcrConversion = Erase(ycbcrConversion);
    device_dispatch_table.DestroySamplerYcbcrConversionKHR(device, ycbcrConversion, pAllocator);
}

void DispatchDevice::GetDescriptorSetLayoutSupportKHR(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo,
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

void DispatchDevice::CmdDrawIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
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

void DispatchDevice::CmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
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

VkResult DispatchDevice::GetSemaphoreCounterValueKHR(VkDevice device, VkSemaphore semaphore, uint64_t* pValue) {
    if (!wrap_handles) return device_dispatch_table.GetSemaphoreCounterValueKHR(device, semaphore, pValue);
    {
        semaphore = Unwrap(semaphore);
    }
    VkResult result = device_dispatch_table.GetSemaphoreCounterValueKHR(device, semaphore, pValue);

    return result;
}

VkResult DispatchDevice::WaitSemaphoresKHR(VkDevice device, const VkSemaphoreWaitInfo* pWaitInfo, uint64_t timeout) {
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

VkResult DispatchDevice::SignalSemaphoreKHR(VkDevice device, const VkSemaphoreSignalInfo* pSignalInfo) {
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

VkResult DispatchInstance::GetPhysicalDeviceFragmentShadingRatesKHR(VkPhysicalDevice physicalDevice,
                                                                    uint32_t* pFragmentShadingRateCount,
                                                                    VkPhysicalDeviceFragmentShadingRateKHR* pFragmentShadingRates) {
    VkResult result = instance_dispatch_table.GetPhysicalDeviceFragmentShadingRatesKHR(physicalDevice, pFragmentShadingRateCount,
                                                                                       pFragmentShadingRates);

    return result;
}

void DispatchDevice::CmdSetFragmentShadingRateKHR(VkCommandBuffer commandBuffer, const VkExtent2D* pFragmentSize,
                                                  const VkFragmentShadingRateCombinerOpKHR combinerOps[2]) {
    device_dispatch_table.CmdSetFragmentShadingRateKHR(commandBuffer, pFragmentSize, combinerOps);
}

void DispatchDevice::CmdSetRenderingAttachmentLocationsKHR(VkCommandBuffer commandBuffer,
                                                           const VkRenderingAttachmentLocationInfo* pLocationInfo) {
    device_dispatch_table.CmdSetRenderingAttachmentLocationsKHR(commandBuffer, pLocationInfo);
}

void DispatchDevice::CmdSetRenderingInputAttachmentIndicesKHR(
    VkCommandBuffer commandBuffer, const VkRenderingInputAttachmentIndexInfo* pInputAttachmentIndexInfo) {
    device_dispatch_table.CmdSetRenderingInputAttachmentIndicesKHR(commandBuffer, pInputAttachmentIndexInfo);
}

VkResult DispatchDevice::WaitForPresentKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t presentId, uint64_t timeout) {
    if (!wrap_handles) return device_dispatch_table.WaitForPresentKHR(device, swapchain, presentId, timeout);
    {
        swapchain = Unwrap(swapchain);
    }
    VkResult result = device_dispatch_table.WaitForPresentKHR(device, swapchain, presentId, timeout);

    return result;
}

VkDeviceAddress DispatchDevice::GetBufferDeviceAddressKHR(VkDevice device, const VkBufferDeviceAddressInfo* pInfo) {
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

uint64_t DispatchDevice::GetBufferOpaqueCaptureAddressKHR(VkDevice device, const VkBufferDeviceAddressInfo* pInfo) {
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

uint64_t DispatchDevice::GetDeviceMemoryOpaqueCaptureAddressKHR(VkDevice device,
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

VkResult DispatchDevice::CreateDeferredOperationKHR(VkDevice device, const VkAllocationCallbacks* pAllocator,
                                                    VkDeferredOperationKHR* pDeferredOperation) {
    if (!wrap_handles) return device_dispatch_table.CreateDeferredOperationKHR(device, pAllocator, pDeferredOperation);

    VkResult result = device_dispatch_table.CreateDeferredOperationKHR(device, pAllocator, pDeferredOperation);
    if (result == VK_SUCCESS) {
        *pDeferredOperation = WrapNew(*pDeferredOperation);
    }
    return result;
}

void DispatchDevice::DestroyDeferredOperationKHR(VkDevice device, VkDeferredOperationKHR operation,
                                                 const VkAllocationCallbacks* pAllocator) {
    if (!wrap_handles) return device_dispatch_table.DestroyDeferredOperationKHR(device, operation, pAllocator);
    operation = Erase(operation);
    device_dispatch_table.DestroyDeferredOperationKHR(device, operation, pAllocator);
}

uint32_t DispatchDevice::GetDeferredOperationMaxConcurrencyKHR(VkDevice device, VkDeferredOperationKHR operation) {
    if (!wrap_handles) return device_dispatch_table.GetDeferredOperationMaxConcurrencyKHR(device, operation);
    {
        operation = Unwrap(operation);
    }
    uint32_t result = device_dispatch_table.GetDeferredOperationMaxConcurrencyKHR(device, operation);

    return result;
}

VkResult DispatchDevice::GetPipelineExecutablePropertiesKHR(VkDevice device, const VkPipelineInfoKHR* pPipelineInfo,
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

VkResult DispatchDevice::GetPipelineExecutableStatisticsKHR(VkDevice device, const VkPipelineExecutableInfoKHR* pExecutableInfo,
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

VkResult DispatchDevice::GetPipelineExecutableInternalRepresentationsKHR(
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

VkResult DispatchDevice::MapMemory2KHR(VkDevice device, const VkMemoryMapInfo* pMemoryMapInfo, void** ppData) {
    if (!wrap_handles) return device_dispatch_table.MapMemory2KHR(device, pMemoryMapInfo, ppData);
    vku::safe_VkMemoryMapInfo var_local_pMemoryMapInfo;
    vku::safe_VkMemoryMapInfo* local_pMemoryMapInfo = nullptr;
    {
        if (pMemoryMapInfo) {
            local_pMemoryMapInfo = &var_local_pMemoryMapInfo;
            local_pMemoryMapInfo->initialize(pMemoryMapInfo);

            if (pMemoryMapInfo->memory) {
                local_pMemoryMapInfo->memory = Unwrap(pMemoryMapInfo->memory);
            }
        }
    }
    VkResult result = device_dispatch_table.MapMemory2KHR(device, (const VkMemoryMapInfo*)local_pMemoryMapInfo, ppData);

    return result;
}

VkResult DispatchDevice::UnmapMemory2KHR(VkDevice device, const VkMemoryUnmapInfo* pMemoryUnmapInfo) {
    if (!wrap_handles) return device_dispatch_table.UnmapMemory2KHR(device, pMemoryUnmapInfo);
    vku::safe_VkMemoryUnmapInfo var_local_pMemoryUnmapInfo;
    vku::safe_VkMemoryUnmapInfo* local_pMemoryUnmapInfo = nullptr;
    {
        if (pMemoryUnmapInfo) {
            local_pMemoryUnmapInfo = &var_local_pMemoryUnmapInfo;
            local_pMemoryUnmapInfo->initialize(pMemoryUnmapInfo);

            if (pMemoryUnmapInfo->memory) {
                local_pMemoryUnmapInfo->memory = Unwrap(pMemoryUnmapInfo->memory);
            }
        }
    }
    VkResult result = device_dispatch_table.UnmapMemory2KHR(device, (const VkMemoryUnmapInfo*)local_pMemoryUnmapInfo);

    return result;
}

VkResult DispatchInstance::GetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceVideoEncodeQualityLevelInfoKHR* pQualityLevelInfo,
    VkVideoEncodeQualityLevelPropertiesKHR* pQualityLevelProperties) {
    VkResult result = instance_dispatch_table.GetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR(
        physicalDevice, pQualityLevelInfo, pQualityLevelProperties);

    return result;
}

VkResult DispatchDevice::GetEncodedVideoSessionParametersKHR(
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

void DispatchDevice::CmdEncodeVideoKHR(VkCommandBuffer commandBuffer, const VkVideoEncodeInfoKHR* pEncodeInfo) {
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

void DispatchDevice::CmdSetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event, const VkDependencyInfo* pDependencyInfo) {
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
            UnwrapPnextChainHandles(local_pDependencyInfo->pNext);
        }
    }
    device_dispatch_table.CmdSetEvent2KHR(commandBuffer, event, (const VkDependencyInfo*)local_pDependencyInfo);
}

void DispatchDevice::CmdResetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags2 stageMask) {
    if (!wrap_handles) return device_dispatch_table.CmdResetEvent2KHR(commandBuffer, event, stageMask);
    {
        event = Unwrap(event);
    }
    device_dispatch_table.CmdResetEvent2KHR(commandBuffer, event, stageMask);
}

void DispatchDevice::CmdWaitEvents2KHR(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
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
                UnwrapPnextChainHandles(local_pDependencyInfos[index0].pNext);
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

void DispatchDevice::CmdPipelineBarrier2KHR(VkCommandBuffer commandBuffer, const VkDependencyInfo* pDependencyInfo) {
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
            UnwrapPnextChainHandles(local_pDependencyInfo->pNext);
        }
    }
    device_dispatch_table.CmdPipelineBarrier2KHR(commandBuffer, (const VkDependencyInfo*)local_pDependencyInfo);
}

void DispatchDevice::CmdWriteTimestamp2KHR(VkCommandBuffer commandBuffer, VkPipelineStageFlags2 stage, VkQueryPool queryPool,
                                           uint32_t query) {
    if (!wrap_handles) return device_dispatch_table.CmdWriteTimestamp2KHR(commandBuffer, stage, queryPool, query);
    {
        queryPool = Unwrap(queryPool);
    }
    device_dispatch_table.CmdWriteTimestamp2KHR(commandBuffer, stage, queryPool, query);
}

VkResult DispatchDevice::QueueSubmit2KHR(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2* pSubmits, VkFence fence) {
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

void DispatchDevice::CmdBindIndexBuffer3KHR(VkCommandBuffer commandBuffer, const VkBindIndexBuffer3InfoKHR* pInfo) {
    device_dispatch_table.CmdBindIndexBuffer3KHR(commandBuffer, pInfo);
}

void DispatchDevice::CmdBindVertexBuffers3KHR(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount,
                                              const VkBindVertexBuffer3InfoKHR* pBindingInfos) {
    device_dispatch_table.CmdBindVertexBuffers3KHR(commandBuffer, firstBinding, bindingCount, pBindingInfos);
}

void DispatchDevice::CmdDrawIndirect2KHR(VkCommandBuffer commandBuffer, const VkDrawIndirect2InfoKHR* pInfo) {
    device_dispatch_table.CmdDrawIndirect2KHR(commandBuffer, pInfo);
}

void DispatchDevice::CmdDrawIndexedIndirect2KHR(VkCommandBuffer commandBuffer, const VkDrawIndirect2InfoKHR* pInfo) {
    device_dispatch_table.CmdDrawIndexedIndirect2KHR(commandBuffer, pInfo);
}

void DispatchDevice::CmdDispatchIndirect2KHR(VkCommandBuffer commandBuffer, const VkDispatchIndirect2InfoKHR* pInfo) {
    device_dispatch_table.CmdDispatchIndirect2KHR(commandBuffer, pInfo);
}

void DispatchDevice::CmdCopyMemoryKHR(VkCommandBuffer commandBuffer, const VkCopyDeviceMemoryInfoKHR* pCopyMemoryInfo) {
    device_dispatch_table.CmdCopyMemoryKHR(commandBuffer, pCopyMemoryInfo);
}

void DispatchDevice::CmdCopyMemoryToImageKHR(VkCommandBuffer commandBuffer, const VkCopyDeviceMemoryImageInfoKHR* pCopyMemoryInfo) {
    if (!wrap_handles) return device_dispatch_table.CmdCopyMemoryToImageKHR(commandBuffer, pCopyMemoryInfo);
    vku::safe_VkCopyDeviceMemoryImageInfoKHR var_local_pCopyMemoryInfo;
    vku::safe_VkCopyDeviceMemoryImageInfoKHR* local_pCopyMemoryInfo = nullptr;
    {
        if (pCopyMemoryInfo) {
            local_pCopyMemoryInfo = &var_local_pCopyMemoryInfo;
            local_pCopyMemoryInfo->initialize(pCopyMemoryInfo);

            if (pCopyMemoryInfo->image) {
                local_pCopyMemoryInfo->image = Unwrap(pCopyMemoryInfo->image);
            }
        }
    }
    device_dispatch_table.CmdCopyMemoryToImageKHR(commandBuffer, (const VkCopyDeviceMemoryImageInfoKHR*)local_pCopyMemoryInfo);
}

void DispatchDevice::CmdCopyImageToMemoryKHR(VkCommandBuffer commandBuffer, const VkCopyDeviceMemoryImageInfoKHR* pCopyMemoryInfo) {
    if (!wrap_handles) return device_dispatch_table.CmdCopyImageToMemoryKHR(commandBuffer, pCopyMemoryInfo);
    vku::safe_VkCopyDeviceMemoryImageInfoKHR var_local_pCopyMemoryInfo;
    vku::safe_VkCopyDeviceMemoryImageInfoKHR* local_pCopyMemoryInfo = nullptr;
    {
        if (pCopyMemoryInfo) {
            local_pCopyMemoryInfo = &var_local_pCopyMemoryInfo;
            local_pCopyMemoryInfo->initialize(pCopyMemoryInfo);

            if (pCopyMemoryInfo->image) {
                local_pCopyMemoryInfo->image = Unwrap(pCopyMemoryInfo->image);
            }
        }
    }
    device_dispatch_table.CmdCopyImageToMemoryKHR(commandBuffer, (const VkCopyDeviceMemoryImageInfoKHR*)local_pCopyMemoryInfo);
}

void DispatchDevice::CmdUpdateMemoryKHR(VkCommandBuffer commandBuffer, const VkDeviceAddressRangeKHR* pDstRange,
                                        VkAddressCommandFlagsKHR dstFlags, VkDeviceSize dataSize, const void* pData) {
    device_dispatch_table.CmdUpdateMemoryKHR(commandBuffer, pDstRange, dstFlags, dataSize, pData);
}

void DispatchDevice::CmdFillMemoryKHR(VkCommandBuffer commandBuffer, const VkDeviceAddressRangeKHR* pDstRange,
                                      VkAddressCommandFlagsKHR dstFlags, uint32_t data) {
    device_dispatch_table.CmdFillMemoryKHR(commandBuffer, pDstRange, dstFlags, data);
}

void DispatchDevice::CmdCopyQueryPoolResultsToMemoryKHR(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery,
                                                        uint32_t queryCount, const VkStridedDeviceAddressRangeKHR* pDstRange,
                                                        VkAddressCommandFlagsKHR dstFlags, VkQueryResultFlags queryResultFlags) {
    if (!wrap_handles)
        return device_dispatch_table.CmdCopyQueryPoolResultsToMemoryKHR(commandBuffer, queryPool, firstQuery, queryCount, pDstRange,
                                                                        dstFlags, queryResultFlags);
    {
        queryPool = Unwrap(queryPool);
    }
    device_dispatch_table.CmdCopyQueryPoolResultsToMemoryKHR(commandBuffer, queryPool, firstQuery, queryCount, pDstRange, dstFlags,
                                                             queryResultFlags);
}

void DispatchDevice::CmdDrawIndirectCount2KHR(VkCommandBuffer commandBuffer, const VkDrawIndirectCount2InfoKHR* pInfo) {
    device_dispatch_table.CmdDrawIndirectCount2KHR(commandBuffer, pInfo);
}

void DispatchDevice::CmdDrawIndexedIndirectCount2KHR(VkCommandBuffer commandBuffer, const VkDrawIndirectCount2InfoKHR* pInfo) {
    device_dispatch_table.CmdDrawIndexedIndirectCount2KHR(commandBuffer, pInfo);
}

void DispatchDevice::CmdBeginConditionalRendering2EXT(VkCommandBuffer commandBuffer,
                                                      const VkConditionalRenderingBeginInfo2EXT* pConditionalRenderingBegin) {
    device_dispatch_table.CmdBeginConditionalRendering2EXT(commandBuffer, pConditionalRenderingBegin);
}

void DispatchDevice::CmdBindTransformFeedbackBuffers2EXT(VkCommandBuffer commandBuffer, uint32_t firstBinding,
                                                         uint32_t bindingCount,
                                                         const VkBindTransformFeedbackBuffer2InfoEXT* pBindingInfos) {
    device_dispatch_table.CmdBindTransformFeedbackBuffers2EXT(commandBuffer, firstBinding, bindingCount, pBindingInfos);
}

void DispatchDevice::CmdBeginTransformFeedback2EXT(VkCommandBuffer commandBuffer, uint32_t firstCounterRange,
                                                   uint32_t counterRangeCount,
                                                   const VkBindTransformFeedbackBuffer2InfoEXT* pCounterInfos) {
    device_dispatch_table.CmdBeginTransformFeedback2EXT(commandBuffer, firstCounterRange, counterRangeCount, pCounterInfos);
}

void DispatchDevice::CmdEndTransformFeedback2EXT(VkCommandBuffer commandBuffer, uint32_t firstCounterRange,
                                                 uint32_t counterRangeCount,
                                                 const VkBindTransformFeedbackBuffer2InfoEXT* pCounterInfos) {
    device_dispatch_table.CmdEndTransformFeedback2EXT(commandBuffer, firstCounterRange, counterRangeCount, pCounterInfos);
}

void DispatchDevice::CmdDrawIndirectByteCount2EXT(VkCommandBuffer commandBuffer, uint32_t instanceCount, uint32_t firstInstance,
                                                  const VkBindTransformFeedbackBuffer2InfoEXT* pCounterInfo, uint32_t counterOffset,
                                                  uint32_t vertexStride) {
    device_dispatch_table.CmdDrawIndirectByteCount2EXT(commandBuffer, instanceCount, firstInstance, pCounterInfo, counterOffset,
                                                       vertexStride);
}

void DispatchDevice::CmdDrawMeshTasksIndirect2EXT(VkCommandBuffer commandBuffer, const VkDrawIndirect2InfoKHR* pInfo) {
    device_dispatch_table.CmdDrawMeshTasksIndirect2EXT(commandBuffer, pInfo);
}

void DispatchDevice::CmdDrawMeshTasksIndirectCount2EXT(VkCommandBuffer commandBuffer, const VkDrawIndirectCount2InfoKHR* pInfo) {
    device_dispatch_table.CmdDrawMeshTasksIndirectCount2EXT(commandBuffer, pInfo);
}

void DispatchDevice::CmdWriteMarkerToMemoryAMD(VkCommandBuffer commandBuffer, const VkMemoryMarkerInfoAMD* pInfo) {
    device_dispatch_table.CmdWriteMarkerToMemoryAMD(commandBuffer, pInfo);
}

VkResult DispatchDevice::CreateAccelerationStructure2KHR(VkDevice device, const VkAccelerationStructureCreateInfo2KHR* pCreateInfo,
                                                         const VkAllocationCallbacks* pAllocator,
                                                         VkAccelerationStructureKHR* pAccelerationStructure) {
    if (!wrap_handles)
        return device_dispatch_table.CreateAccelerationStructure2KHR(device, pCreateInfo, pAllocator, pAccelerationStructure);

    VkResult result =
        device_dispatch_table.CreateAccelerationStructure2KHR(device, pCreateInfo, pAllocator, pAccelerationStructure);
    if (result == VK_SUCCESS) {
        *pAccelerationStructure = WrapNew(*pAccelerationStructure);
    }
    return result;
}

void DispatchDevice::CmdCopyBuffer2KHR(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2* pCopyBufferInfo) {
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

void DispatchDevice::CmdCopyImage2KHR(VkCommandBuffer commandBuffer, const VkCopyImageInfo2* pCopyImageInfo) {
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

void DispatchDevice::CmdCopyBufferToImage2KHR(VkCommandBuffer commandBuffer,
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

void DispatchDevice::CmdCopyImageToBuffer2KHR(VkCommandBuffer commandBuffer,
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

void DispatchDevice::CmdBlitImage2KHR(VkCommandBuffer commandBuffer, const VkBlitImageInfo2* pBlitImageInfo) {
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

void DispatchDevice::CmdResolveImage2KHR(VkCommandBuffer commandBuffer, const VkResolveImageInfo2* pResolveImageInfo) {
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

void DispatchDevice::CmdTraceRaysIndirect2KHR(VkCommandBuffer commandBuffer, VkDeviceAddress indirectDeviceAddress) {
    device_dispatch_table.CmdTraceRaysIndirect2KHR(commandBuffer, indirectDeviceAddress);
}

void DispatchDevice::GetDeviceBufferMemoryRequirementsKHR(VkDevice device, const VkDeviceBufferMemoryRequirements* pInfo,
                                                          VkMemoryRequirements2* pMemoryRequirements) {
    device_dispatch_table.GetDeviceBufferMemoryRequirementsKHR(device, pInfo, pMemoryRequirements);
}

void DispatchDevice::GetDeviceImageMemoryRequirementsKHR(VkDevice device, const VkDeviceImageMemoryRequirements* pInfo,
                                                         VkMemoryRequirements2* pMemoryRequirements) {
    device_dispatch_table.GetDeviceImageMemoryRequirementsKHR(device, pInfo, pMemoryRequirements);
}

void DispatchDevice::GetDeviceImageSparseMemoryRequirementsKHR(VkDevice device, const VkDeviceImageMemoryRequirements* pInfo,
                                                               uint32_t* pSparseMemoryRequirementCount,
                                                               VkSparseImageMemoryRequirements2* pSparseMemoryRequirements) {
    device_dispatch_table.GetDeviceImageSparseMemoryRequirementsKHR(device, pInfo, pSparseMemoryRequirementCount,
                                                                    pSparseMemoryRequirements);
}

void DispatchDevice::CmdBindIndexBuffer2KHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkDeviceSize size,
                                            VkIndexType indexType) {
    if (!wrap_handles) return device_dispatch_table.CmdBindIndexBuffer2KHR(commandBuffer, buffer, offset, size, indexType);
    {
        buffer = Unwrap(buffer);
    }
    device_dispatch_table.CmdBindIndexBuffer2KHR(commandBuffer, buffer, offset, size, indexType);
}

void DispatchDevice::GetRenderingAreaGranularityKHR(VkDevice device, const VkRenderingAreaInfo* pRenderingAreaInfo,
                                                    VkExtent2D* pGranularity) {
    device_dispatch_table.GetRenderingAreaGranularityKHR(device, pRenderingAreaInfo, pGranularity);
}

void DispatchDevice::GetDeviceImageSubresourceLayoutKHR(VkDevice device, const VkDeviceImageSubresourceInfo* pInfo,
                                                        VkSubresourceLayout2* pLayout) {
    device_dispatch_table.GetDeviceImageSubresourceLayoutKHR(device, pInfo, pLayout);
}

void DispatchDevice::GetImageSubresourceLayout2KHR(VkDevice device, VkImage image, const VkImageSubresource2* pSubresource,
                                                   VkSubresourceLayout2* pLayout) {
    if (!wrap_handles) return device_dispatch_table.GetImageSubresourceLayout2KHR(device, image, pSubresource, pLayout);
    {
        image = Unwrap(image);
    }
    device_dispatch_table.GetImageSubresourceLayout2KHR(device, image, pSubresource, pLayout);
}

VkResult DispatchDevice::WaitForPresent2KHR(VkDevice device, VkSwapchainKHR swapchain,
                                            const VkPresentWait2InfoKHR* pPresentWait2Info) {
    if (!wrap_handles) return device_dispatch_table.WaitForPresent2KHR(device, swapchain, pPresentWait2Info);
    {
        swapchain = Unwrap(swapchain);
    }
    VkResult result = device_dispatch_table.WaitForPresent2KHR(device, swapchain, pPresentWait2Info);

    return result;
}

void DispatchDevice::DestroyPipelineBinaryKHR(VkDevice device, VkPipelineBinaryKHR pipelineBinary,
                                              const VkAllocationCallbacks* pAllocator) {
    if (!wrap_handles) return device_dispatch_table.DestroyPipelineBinaryKHR(device, pipelineBinary, pAllocator);
    pipelineBinary = Erase(pipelineBinary);
    device_dispatch_table.DestroyPipelineBinaryKHR(device, pipelineBinary, pAllocator);
}

VkResult DispatchDevice::GetPipelineBinaryDataKHR(VkDevice device, const VkPipelineBinaryDataInfoKHR* pInfo,
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

VkResult DispatchDevice::ReleaseCapturedPipelineDataKHR(VkDevice device, const VkReleaseCapturedPipelineDataInfoKHR* pInfo,
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

VkResult DispatchDevice::ReleaseSwapchainImagesKHR(VkDevice device, const VkReleaseSwapchainImagesInfoKHR* pReleaseInfo) {
    if (!wrap_handles) return device_dispatch_table.ReleaseSwapchainImagesKHR(device, pReleaseInfo);
    vku::safe_VkReleaseSwapchainImagesInfoKHR var_local_pReleaseInfo;
    vku::safe_VkReleaseSwapchainImagesInfoKHR* local_pReleaseInfo = nullptr;
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
        device_dispatch_table.ReleaseSwapchainImagesKHR(device, (const VkReleaseSwapchainImagesInfoKHR*)local_pReleaseInfo);

    return result;
}

VkResult DispatchInstance::GetPhysicalDeviceCooperativeMatrixPropertiesKHR(VkPhysicalDevice physicalDevice,
                                                                           uint32_t* pPropertyCount,
                                                                           VkCooperativeMatrixPropertiesKHR* pProperties) {
    VkResult result =
        instance_dispatch_table.GetPhysicalDeviceCooperativeMatrixPropertiesKHR(physicalDevice, pPropertyCount, pProperties);

    return result;
}

void DispatchDevice::CmdSetLineStippleKHR(VkCommandBuffer commandBuffer, uint32_t lineStippleFactor, uint16_t lineStipplePattern) {
    device_dispatch_table.CmdSetLineStippleKHR(commandBuffer, lineStippleFactor, lineStipplePattern);
}

VkResult DispatchInstance::GetPhysicalDeviceCalibrateableTimeDomainsKHR(VkPhysicalDevice physicalDevice, uint32_t* pTimeDomainCount,
                                                                        VkTimeDomainKHR* pTimeDomains) {
    VkResult result =
        instance_dispatch_table.GetPhysicalDeviceCalibrateableTimeDomainsKHR(physicalDevice, pTimeDomainCount, pTimeDomains);

    return result;
}

VkResult DispatchDevice::GetCalibratedTimestampsKHR(VkDevice device, uint32_t timestampCount,
                                                    const VkCalibratedTimestampInfoKHR* pTimestampInfos, uint64_t* pTimestamps,
                                                    uint64_t* pMaxDeviation) {
    if (!wrap_handles)
        return device_dispatch_table.GetCalibratedTimestampsKHR(device, timestampCount, pTimestampInfos, pTimestamps,
                                                                pMaxDeviation);
    small_vector<vku::safe_VkCalibratedTimestampInfoKHR, DISPATCH_MAX_STACK_ALLOCATIONS> var_local_pTimestampInfos;
    vku::safe_VkCalibratedTimestampInfoKHR* local_pTimestampInfos = nullptr;
    {
        if (pTimestampInfos) {
            var_local_pTimestampInfos.resize(timestampCount);
            local_pTimestampInfos = var_local_pTimestampInfos.data();
            for (uint32_t index0 = 0; index0 < timestampCount; ++index0) {
                local_pTimestampInfos[index0].initialize(&pTimestampInfos[index0]);
                UnwrapPnextChainHandles(local_pTimestampInfos[index0].pNext);
            }
        }
    }
    VkResult result = device_dispatch_table.GetCalibratedTimestampsKHR(
        device, timestampCount, (const VkCalibratedTimestampInfoKHR*)local_pTimestampInfos, pTimestamps, pMaxDeviation);

    return result;
}

void DispatchDevice::CmdBindDescriptorSets2KHR(VkCommandBuffer commandBuffer,
                                               const VkBindDescriptorSetsInfo* pBindDescriptorSetsInfo) {
    if (!wrap_handles) return device_dispatch_table.CmdBindDescriptorSets2KHR(commandBuffer, pBindDescriptorSetsInfo);
    vku::safe_VkBindDescriptorSetsInfo var_local_pBindDescriptorSetsInfo;
    vku::safe_VkBindDescriptorSetsInfo* local_pBindDescriptorSetsInfo = nullptr;
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
    device_dispatch_table.CmdBindDescriptorSets2KHR(commandBuffer, (const VkBindDescriptorSetsInfo*)local_pBindDescriptorSetsInfo);
}

void DispatchDevice::CmdPushConstants2KHR(VkCommandBuffer commandBuffer, const VkPushConstantsInfo* pPushConstantsInfo) {
    if (!wrap_handles) return device_dispatch_table.CmdPushConstants2KHR(commandBuffer, pPushConstantsInfo);
    vku::safe_VkPushConstantsInfo var_local_pPushConstantsInfo;
    vku::safe_VkPushConstantsInfo* local_pPushConstantsInfo = nullptr;
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
    device_dispatch_table.CmdPushConstants2KHR(commandBuffer, (const VkPushConstantsInfo*)local_pPushConstantsInfo);
}

void DispatchDevice::CmdPushDescriptorSet2KHR(VkCommandBuffer commandBuffer,
                                              const VkPushDescriptorSetInfo* pPushDescriptorSetInfo) {
    if (!wrap_handles) return device_dispatch_table.CmdPushDescriptorSet2KHR(commandBuffer, pPushDescriptorSetInfo);
    vku::safe_VkPushDescriptorSetInfo var_local_pPushDescriptorSetInfo;
    vku::safe_VkPushDescriptorSetInfo* local_pPushDescriptorSetInfo = nullptr;
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
                        // need for when updating VkDescriptorImageInfo
                        bool has_sampler =
                            local_pPushDescriptorSetInfo->pDescriptorWrites[index1].descriptorType ==
                                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER ||
                            local_pPushDescriptorSetInfo->pDescriptorWrites[index1].descriptorType == VK_DESCRIPTOR_TYPE_SAMPLER;
                        bool has_image_view =
                            local_pPushDescriptorSetInfo->pDescriptorWrites[index1].descriptorType != VK_DESCRIPTOR_TYPE_SAMPLER;

                        for (uint32_t index2 = 0; index2 < local_pPushDescriptorSetInfo->pDescriptorWrites[index1].descriptorCount;
                             ++index2) {
                            if (pPushDescriptorSetInfo->pDescriptorWrites[index1].pImageInfo[index2].sampler && has_sampler) {
                                local_pPushDescriptorSetInfo->pDescriptorWrites[index1].pImageInfo[index2].sampler =
                                    Unwrap(pPushDescriptorSetInfo->pDescriptorWrites[index1].pImageInfo[index2].sampler);
                            }
                            if (pPushDescriptorSetInfo->pDescriptorWrites[index1].pImageInfo[index2].imageView && has_image_view) {
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
    device_dispatch_table.CmdPushDescriptorSet2KHR(commandBuffer, (const VkPushDescriptorSetInfo*)local_pPushDescriptorSetInfo);
}

void DispatchDevice::CmdSetDescriptorBufferOffsets2EXT(VkCommandBuffer commandBuffer,
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

void DispatchDevice::CmdBindDescriptorBufferEmbeddedSamplers2EXT(
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

void DispatchDevice::CmdCopyMemoryIndirectKHR(VkCommandBuffer commandBuffer,
                                              const VkCopyMemoryIndirectInfoKHR* pCopyMemoryIndirectInfo) {
    device_dispatch_table.CmdCopyMemoryIndirectKHR(commandBuffer, pCopyMemoryIndirectInfo);
}

void DispatchDevice::CmdCopyMemoryToImageIndirectKHR(VkCommandBuffer commandBuffer,
                                                     const VkCopyMemoryToImageIndirectInfoKHR* pCopyMemoryToImageIndirectInfo) {
    if (!wrap_handles) return device_dispatch_table.CmdCopyMemoryToImageIndirectKHR(commandBuffer, pCopyMemoryToImageIndirectInfo);
    vku::safe_VkCopyMemoryToImageIndirectInfoKHR var_local_pCopyMemoryToImageIndirectInfo;
    vku::safe_VkCopyMemoryToImageIndirectInfoKHR* local_pCopyMemoryToImageIndirectInfo = nullptr;
    {
        if (pCopyMemoryToImageIndirectInfo) {
            local_pCopyMemoryToImageIndirectInfo = &var_local_pCopyMemoryToImageIndirectInfo;
            local_pCopyMemoryToImageIndirectInfo->initialize(pCopyMemoryToImageIndirectInfo);

            if (pCopyMemoryToImageIndirectInfo->dstImage) {
                local_pCopyMemoryToImageIndirectInfo->dstImage = Unwrap(pCopyMemoryToImageIndirectInfo->dstImage);
            }
        }
    }
    device_dispatch_table.CmdCopyMemoryToImageIndirectKHR(
        commandBuffer, (const VkCopyMemoryToImageIndirectInfoKHR*)local_pCopyMemoryToImageIndirectInfo);
}

VkResult DispatchDevice::GetDeviceFaultReportsKHR(VkDevice device, uint64_t timeout, uint32_t* pFaultCounts,
                                                  VkDeviceFaultInfoKHR* pFaultInfo) {
    VkResult result = device_dispatch_table.GetDeviceFaultReportsKHR(device, timeout, pFaultCounts, pFaultInfo);

    return result;
}

VkResult DispatchDevice::GetDeviceFaultDebugInfoKHR(VkDevice device, VkDeviceFaultDebugInfoKHR* pDebugInfo) {
    VkResult result = device_dispatch_table.GetDeviceFaultDebugInfoKHR(device, pDebugInfo);

    return result;
}

void DispatchDevice::CmdEndRendering2KHR(VkCommandBuffer commandBuffer, const VkRenderingEndInfoKHR* pRenderingEndInfo) {
    device_dispatch_table.CmdEndRendering2KHR(commandBuffer, pRenderingEndInfo);
}

VkResult DispatchInstance::CreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo,
                                                        const VkAllocationCallbacks* pAllocator,
                                                        VkDebugReportCallbackEXT* pCallback) {
    if (!wrap_handles) return instance_dispatch_table.CreateDebugReportCallbackEXT(instance, pCreateInfo, pAllocator, pCallback);

    VkResult result = instance_dispatch_table.CreateDebugReportCallbackEXT(instance, pCreateInfo, pAllocator, pCallback);
    if (result == VK_SUCCESS) {
        *pCallback = WrapNew(*pCallback);
    }
    return result;
}

void DispatchInstance::DestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback,
                                                     const VkAllocationCallbacks* pAllocator) {
    if (!wrap_handles) return instance_dispatch_table.DestroyDebugReportCallbackEXT(instance, callback, pAllocator);
    callback = Erase(callback);
    instance_dispatch_table.DestroyDebugReportCallbackEXT(instance, callback, pAllocator);
}

void DispatchInstance::DebugReportMessageEXT(VkInstance instance, VkDebugReportFlagsEXT flags,
                                             VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location,
                                             int32_t messageCode, const char* pLayerPrefix, const char* pMessage) {
    instance_dispatch_table.DebugReportMessageEXT(instance, flags, objectType, object, location, messageCode, pLayerPrefix,
                                                  pMessage);
}

void DispatchDevice::CmdDebugMarkerBeginEXT(VkCommandBuffer commandBuffer, const VkDebugMarkerMarkerInfoEXT* pMarkerInfo) {
    device_dispatch_table.CmdDebugMarkerBeginEXT(commandBuffer, pMarkerInfo);
}

void DispatchDevice::CmdDebugMarkerEndEXT(VkCommandBuffer commandBuffer) {
    device_dispatch_table.CmdDebugMarkerEndEXT(commandBuffer);
}

void DispatchDevice::CmdDebugMarkerInsertEXT(VkCommandBuffer commandBuffer, const VkDebugMarkerMarkerInfoEXT* pMarkerInfo) {
    device_dispatch_table.CmdDebugMarkerInsertEXT(commandBuffer, pMarkerInfo);
}

void DispatchDevice::CmdBindTransformFeedbackBuffersEXT(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount,
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

void DispatchDevice::CmdBeginTransformFeedbackEXT(VkCommandBuffer commandBuffer, uint32_t firstCounterBuffer,
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

void DispatchDevice::CmdEndTransformFeedbackEXT(VkCommandBuffer commandBuffer, uint32_t firstCounterBuffer,
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

void DispatchDevice::CmdBeginQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query,
                                             VkQueryControlFlags flags, uint32_t index) {
    if (!wrap_handles) return device_dispatch_table.CmdBeginQueryIndexedEXT(commandBuffer, queryPool, query, flags, index);
    {
        queryPool = Unwrap(queryPool);
    }
    device_dispatch_table.CmdBeginQueryIndexedEXT(commandBuffer, queryPool, query, flags, index);
}

void DispatchDevice::CmdEndQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query, uint32_t index) {
    if (!wrap_handles) return device_dispatch_table.CmdEndQueryIndexedEXT(commandBuffer, queryPool, query, index);
    {
        queryPool = Unwrap(queryPool);
    }
    device_dispatch_table.CmdEndQueryIndexedEXT(commandBuffer, queryPool, query, index);
}

void DispatchDevice::CmdDrawIndirectByteCountEXT(VkCommandBuffer commandBuffer, uint32_t instanceCount, uint32_t firstInstance,
                                                 VkBuffer counterBuffer, VkDeviceSize counterBufferOffset, uint32_t counterOffset,
                                                 uint32_t vertexStride) {
    if (!wrap_handles)
        return device_dispatch_table.CmdDrawIndirectByteCountEXT(commandBuffer, instanceCount, firstInstance, counterBuffer,
                                                                 counterBufferOffset, counterOffset, vertexStride);
    {
        counterBuffer = Unwrap(counterBuffer);
    }
    device_dispatch_table.CmdDrawIndirectByteCountEXT(commandBuffer, instanceCount, firstInstance, counterBuffer,
                                                      counterBufferOffset, counterOffset, vertexStride);
}

VkResult DispatchDevice::CreateCuModuleNVX(VkDevice device, const VkCuModuleCreateInfoNVX* pCreateInfo,
                                           const VkAllocationCallbacks* pAllocator, VkCuModuleNVX* pModule) {
    if (!wrap_handles) return device_dispatch_table.CreateCuModuleNVX(device, pCreateInfo, pAllocator, pModule);

    VkResult result = device_dispatch_table.CreateCuModuleNVX(device, pCreateInfo, pAllocator, pModule);
    if (result == VK_SUCCESS) {
        *pModule = WrapNew(*pModule);
    }
    return result;
}

VkResult DispatchDevice::CreateCuFunctionNVX(VkDevice device, const VkCuFunctionCreateInfoNVX* pCreateInfo,
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
    if (result == VK_SUCCESS) {
        *pFunction = WrapNew(*pFunction);
    }
    return result;
}

void DispatchDevice::DestroyCuModuleNVX(VkDevice device, VkCuModuleNVX module, const VkAllocationCallbacks* pAllocator) {
    if (!wrap_handles) return device_dispatch_table.DestroyCuModuleNVX(device, module, pAllocator);
    module = Erase(module);
    device_dispatch_table.DestroyCuModuleNVX(device, module, pAllocator);
}

void DispatchDevice::DestroyCuFunctionNVX(VkDevice device, VkCuFunctionNVX function, const VkAllocationCallbacks* pAllocator) {
    if (!wrap_handles) return device_dispatch_table.DestroyCuFunctionNVX(device, function, pAllocator);
    function = Erase(function);
    device_dispatch_table.DestroyCuFunctionNVX(device, function, pAllocator);
}

void DispatchDevice::CmdCuLaunchKernelNVX(VkCommandBuffer commandBuffer, const VkCuLaunchInfoNVX* pLaunchInfo) {
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

uint32_t DispatchDevice::GetImageViewHandleNVX(VkDevice device, const VkImageViewHandleInfoNVX* pInfo) {
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

uint64_t DispatchDevice::GetImageViewHandle64NVX(VkDevice device, const VkImageViewHandleInfoNVX* pInfo) {
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

VkResult DispatchDevice::GetImageViewAddressNVX(VkDevice device, VkImageView imageView,
                                                VkImageViewAddressPropertiesNVX* pProperties) {
    if (!wrap_handles) return device_dispatch_table.GetImageViewAddressNVX(device, imageView, pProperties);
    {
        imageView = Unwrap(imageView);
    }
    VkResult result = device_dispatch_table.GetImageViewAddressNVX(device, imageView, pProperties);

    return result;
}

uint64_t DispatchDevice::GetDeviceCombinedImageSamplerIndexNVX(VkDevice device, uint64_t imageViewIndex, uint64_t samplerIndex) {
    uint64_t result = device_dispatch_table.GetDeviceCombinedImageSamplerIndexNVX(device, imageViewIndex, samplerIndex);

    return result;
}

void DispatchDevice::CmdDrawIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
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

void DispatchDevice::CmdDrawIndexedIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
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

VkResult DispatchDevice::GetShaderInfoAMD(VkDevice device, VkPipeline pipeline, VkShaderStageFlagBits shaderStage,
                                          VkShaderInfoTypeAMD infoType, size_t* pInfoSize, void* pInfo) {
    if (!wrap_handles) return device_dispatch_table.GetShaderInfoAMD(device, pipeline, shaderStage, infoType, pInfoSize, pInfo);
    {
        pipeline = Unwrap(pipeline);
    }
    VkResult result = device_dispatch_table.GetShaderInfoAMD(device, pipeline, shaderStage, infoType, pInfoSize, pInfo);

    return result;
}
#ifdef VK_USE_PLATFORM_GGP

VkResult DispatchInstance::CreateStreamDescriptorSurfaceGGP(VkInstance instance,
                                                            const VkStreamDescriptorSurfaceCreateInfoGGP* pCreateInfo,
                                                            const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    if (!wrap_handles) return instance_dispatch_table.CreateStreamDescriptorSurfaceGGP(instance, pCreateInfo, pAllocator, pSurface);

    VkResult result = instance_dispatch_table.CreateStreamDescriptorSurfaceGGP(instance, pCreateInfo, pAllocator, pSurface);
    if (result == VK_SUCCESS) {
        *pSurface = WrapNew(*pSurface);
    }
    return result;
}
#endif  // VK_USE_PLATFORM_GGP

VkResult DispatchInstance::GetPhysicalDeviceExternalImageFormatPropertiesNV(
    VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type, VkImageTiling tiling, VkImageUsageFlags usage,
    VkImageCreateFlags flags, VkExternalMemoryHandleTypeFlagsNV externalHandleType,
    VkExternalImageFormatPropertiesNV* pExternalImageFormatProperties) {
    VkResult result = instance_dispatch_table.GetPhysicalDeviceExternalImageFormatPropertiesNV(
        physicalDevice, format, type, tiling, usage, flags, externalHandleType, pExternalImageFormatProperties);

    return result;
}
#ifdef VK_USE_PLATFORM_WIN32_KHR

VkResult DispatchDevice::GetMemoryWin32HandleNV(VkDevice device, VkDeviceMemory memory,
                                                VkExternalMemoryHandleTypeFlagsNV handleType, HANDLE* pHandle) {
    if (!wrap_handles) return device_dispatch_table.GetMemoryWin32HandleNV(device, memory, handleType, pHandle);
    {
        memory = Unwrap(memory);
    }
    VkResult result = device_dispatch_table.GetMemoryWin32HandleNV(device, memory, handleType, pHandle);

    return result;
}
#endif  // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_VI_NN

VkResult DispatchInstance::CreateViSurfaceNN(VkInstance instance, const VkViSurfaceCreateInfoNN* pCreateInfo,
                                             const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    if (!wrap_handles) return instance_dispatch_table.CreateViSurfaceNN(instance, pCreateInfo, pAllocator, pSurface);

    VkResult result = instance_dispatch_table.CreateViSurfaceNN(instance, pCreateInfo, pAllocator, pSurface);
    if (result == VK_SUCCESS) {
        *pSurface = WrapNew(*pSurface);
    }
    return result;
}
#endif  // VK_USE_PLATFORM_VI_NN

void DispatchDevice::CmdBeginConditionalRenderingEXT(VkCommandBuffer commandBuffer,
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

void DispatchDevice::CmdEndConditionalRenderingEXT(VkCommandBuffer commandBuffer) {
    device_dispatch_table.CmdEndConditionalRenderingEXT(commandBuffer);
}

void DispatchDevice::CmdSetViewportWScalingNV(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount,
                                              const VkViewportWScalingNV* pViewportWScalings) {
    device_dispatch_table.CmdSetViewportWScalingNV(commandBuffer, firstViewport, viewportCount, pViewportWScalings);
}

VkResult DispatchInstance::ReleaseDisplayEXT(VkPhysicalDevice physicalDevice, VkDisplayKHR display) {
    if (!wrap_handles) return instance_dispatch_table.ReleaseDisplayEXT(physicalDevice, display);
    {
        display = Unwrap(display);
    }
    VkResult result = instance_dispatch_table.ReleaseDisplayEXT(physicalDevice, display);

    return result;
}
#ifdef VK_USE_PLATFORM_XLIB_XRANDR_EXT

VkResult DispatchInstance::AcquireXlibDisplayEXT(VkPhysicalDevice physicalDevice, Display* dpy, VkDisplayKHR display) {
    if (!wrap_handles) return instance_dispatch_table.AcquireXlibDisplayEXT(physicalDevice, dpy, display);
    {
        display = Unwrap(display);
    }
    VkResult result = instance_dispatch_table.AcquireXlibDisplayEXT(physicalDevice, dpy, display);

    return result;
}

VkResult DispatchInstance::GetRandROutputDisplayEXT(VkPhysicalDevice physicalDevice, Display* dpy, RROutput rrOutput,
                                                    VkDisplayKHR* pDisplay) {
    if (!wrap_handles) return instance_dispatch_table.GetRandROutputDisplayEXT(physicalDevice, dpy, rrOutput, pDisplay);

    VkResult result = instance_dispatch_table.GetRandROutputDisplayEXT(physicalDevice, dpy, rrOutput, pDisplay);
    if (result == VK_SUCCESS) {
        *pDisplay = MaybeWrapDisplay(*pDisplay);
    }
    return result;
}
#endif  // VK_USE_PLATFORM_XLIB_XRANDR_EXT

VkResult DispatchInstance::GetPhysicalDeviceSurfaceCapabilities2EXT(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                    VkSurfaceCapabilities2EXT* pSurfaceCapabilities) {
    if (!wrap_handles)
        return instance_dispatch_table.GetPhysicalDeviceSurfaceCapabilities2EXT(physicalDevice, surface, pSurfaceCapabilities);
    {
        surface = Unwrap(surface);
    }
    VkResult result =
        instance_dispatch_table.GetPhysicalDeviceSurfaceCapabilities2EXT(physicalDevice, surface, pSurfaceCapabilities);

    return result;
}

VkResult DispatchDevice::DisplayPowerControlEXT(VkDevice device, VkDisplayKHR display,
                                                const VkDisplayPowerInfoEXT* pDisplayPowerInfo) {
    if (!wrap_handles) return device_dispatch_table.DisplayPowerControlEXT(device, display, pDisplayPowerInfo);
    {
        display = Unwrap(display);
    }
    VkResult result = device_dispatch_table.DisplayPowerControlEXT(device, display, pDisplayPowerInfo);

    return result;
}

VkResult DispatchDevice::RegisterDeviceEventEXT(VkDevice device, const VkDeviceEventInfoEXT* pDeviceEventInfo,
                                                const VkAllocationCallbacks* pAllocator, VkFence* pFence) {
    if (!wrap_handles) return device_dispatch_table.RegisterDeviceEventEXT(device, pDeviceEventInfo, pAllocator, pFence);

    VkResult result = device_dispatch_table.RegisterDeviceEventEXT(device, pDeviceEventInfo, pAllocator, pFence);
    if (result == VK_SUCCESS) {
        *pFence = WrapNew(*pFence);
    }
    return result;
}

VkResult DispatchDevice::RegisterDisplayEventEXT(VkDevice device, VkDisplayKHR display,
                                                 const VkDisplayEventInfoEXT* pDisplayEventInfo,
                                                 const VkAllocationCallbacks* pAllocator, VkFence* pFence) {
    if (!wrap_handles) return device_dispatch_table.RegisterDisplayEventEXT(device, display, pDisplayEventInfo, pAllocator, pFence);
    {
        display = Unwrap(display);
    }
    VkResult result = device_dispatch_table.RegisterDisplayEventEXT(device, display, pDisplayEventInfo, pAllocator, pFence);
    if (result == VK_SUCCESS) {
        *pFence = WrapNew(*pFence);
    }
    return result;
}

VkResult DispatchDevice::GetSwapchainCounterEXT(VkDevice device, VkSwapchainKHR swapchain, VkSurfaceCounterFlagBitsEXT counter,
                                                uint64_t* pCounterValue) {
    if (!wrap_handles) return device_dispatch_table.GetSwapchainCounterEXT(device, swapchain, counter, pCounterValue);
    {
        swapchain = Unwrap(swapchain);
    }
    VkResult result = device_dispatch_table.GetSwapchainCounterEXT(device, swapchain, counter, pCounterValue);

    return result;
}

VkResult DispatchDevice::GetRefreshCycleDurationGOOGLE(VkDevice device, VkSwapchainKHR swapchain,
                                                       VkRefreshCycleDurationGOOGLE* pDisplayTimingProperties) {
    if (!wrap_handles) return device_dispatch_table.GetRefreshCycleDurationGOOGLE(device, swapchain, pDisplayTimingProperties);
    {
        swapchain = Unwrap(swapchain);
    }
    VkResult result = device_dispatch_table.GetRefreshCycleDurationGOOGLE(device, swapchain, pDisplayTimingProperties);

    return result;
}

VkResult DispatchDevice::GetPastPresentationTimingGOOGLE(VkDevice device, VkSwapchainKHR swapchain,
                                                         uint32_t* pPresentationTimingCount,
                                                         VkPastPresentationTimingGOOGLE* pPresentationTimings) {
    if (!wrap_handles)
        return device_dispatch_table.GetPastPresentationTimingGOOGLE(device, swapchain, pPresentationTimingCount,
                                                                     pPresentationTimings);
    {
        swapchain = Unwrap(swapchain);
    }
    VkResult result =
        device_dispatch_table.GetPastPresentationTimingGOOGLE(device, swapchain, pPresentationTimingCount, pPresentationTimings);

    return result;
}

void DispatchDevice::CmdSetDiscardRectangleEXT(VkCommandBuffer commandBuffer, uint32_t firstDiscardRectangle,
                                               uint32_t discardRectangleCount, const VkRect2D* pDiscardRectangles) {
    device_dispatch_table.CmdSetDiscardRectangleEXT(commandBuffer, firstDiscardRectangle, discardRectangleCount,
                                                    pDiscardRectangles);
}

void DispatchDevice::CmdSetDiscardRectangleEnableEXT(VkCommandBuffer commandBuffer, VkBool32 discardRectangleEnable) {
    device_dispatch_table.CmdSetDiscardRectangleEnableEXT(commandBuffer, discardRectangleEnable);
}

void DispatchDevice::CmdSetDiscardRectangleModeEXT(VkCommandBuffer commandBuffer, VkDiscardRectangleModeEXT discardRectangleMode) {
    device_dispatch_table.CmdSetDiscardRectangleModeEXT(commandBuffer, discardRectangleMode);
}

void DispatchDevice::SetHdrMetadataEXT(VkDevice device, uint32_t swapchainCount, const VkSwapchainKHR* pSwapchains,
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

VkResult DispatchInstance::CreateIOSSurfaceMVK(VkInstance instance, const VkIOSSurfaceCreateInfoMVK* pCreateInfo,
                                               const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    if (!wrap_handles) return instance_dispatch_table.CreateIOSSurfaceMVK(instance, pCreateInfo, pAllocator, pSurface);

    VkResult result = instance_dispatch_table.CreateIOSSurfaceMVK(instance, pCreateInfo, pAllocator, pSurface);
    if (result == VK_SUCCESS) {
        *pSurface = WrapNew(*pSurface);
    }
    return result;
}
#endif  // VK_USE_PLATFORM_IOS_MVK
#ifdef VK_USE_PLATFORM_MACOS_MVK

VkResult DispatchInstance::CreateMacOSSurfaceMVK(VkInstance instance, const VkMacOSSurfaceCreateInfoMVK* pCreateInfo,
                                                 const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    if (!wrap_handles) return instance_dispatch_table.CreateMacOSSurfaceMVK(instance, pCreateInfo, pAllocator, pSurface);

    VkResult result = instance_dispatch_table.CreateMacOSSurfaceMVK(instance, pCreateInfo, pAllocator, pSurface);
    if (result == VK_SUCCESS) {
        *pSurface = WrapNew(*pSurface);
    }
    return result;
}
#endif  // VK_USE_PLATFORM_MACOS_MVK

void DispatchDevice::QueueBeginDebugUtilsLabelEXT(VkQueue queue, const VkDebugUtilsLabelEXT* pLabelInfo) {
    device_dispatch_table.QueueBeginDebugUtilsLabelEXT(queue, pLabelInfo);
}

void DispatchDevice::QueueEndDebugUtilsLabelEXT(VkQueue queue) { device_dispatch_table.QueueEndDebugUtilsLabelEXT(queue); }

void DispatchDevice::QueueInsertDebugUtilsLabelEXT(VkQueue queue, const VkDebugUtilsLabelEXT* pLabelInfo) {
    device_dispatch_table.QueueInsertDebugUtilsLabelEXT(queue, pLabelInfo);
}

void DispatchDevice::CmdBeginDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT* pLabelInfo) {
    device_dispatch_table.CmdBeginDebugUtilsLabelEXT(commandBuffer, pLabelInfo);
}

void DispatchDevice::CmdEndDebugUtilsLabelEXT(VkCommandBuffer commandBuffer) {
    device_dispatch_table.CmdEndDebugUtilsLabelEXT(commandBuffer);
}

void DispatchDevice::CmdInsertDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT* pLabelInfo) {
    device_dispatch_table.CmdInsertDebugUtilsLabelEXT(commandBuffer, pLabelInfo);
}

VkResult DispatchInstance::CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                                        const VkAllocationCallbacks* pAllocator,
                                                        VkDebugUtilsMessengerEXT* pMessenger) {
    if (!wrap_handles) return instance_dispatch_table.CreateDebugUtilsMessengerEXT(instance, pCreateInfo, pAllocator, pMessenger);

    VkResult result = instance_dispatch_table.CreateDebugUtilsMessengerEXT(instance, pCreateInfo, pAllocator, pMessenger);
    if (result == VK_SUCCESS) {
        *pMessenger = WrapNew(*pMessenger);
    }
    return result;
}

void DispatchInstance::DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT messenger,
                                                     const VkAllocationCallbacks* pAllocator) {
    if (!wrap_handles) return instance_dispatch_table.DestroyDebugUtilsMessengerEXT(instance, messenger, pAllocator);
    messenger = Erase(messenger);
    instance_dispatch_table.DestroyDebugUtilsMessengerEXT(instance, messenger, pAllocator);
}

void DispatchInstance::SubmitDebugUtilsMessageEXT(VkInstance instance, VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                  VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                                                  const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData) {
    instance_dispatch_table.SubmitDebugUtilsMessageEXT(instance, messageSeverity, messageTypes, pCallbackData);
}
#ifdef VK_USE_PLATFORM_ANDROID_KHR

VkResult DispatchDevice::GetAndroidHardwareBufferPropertiesANDROID(VkDevice device, const struct AHardwareBuffer* buffer,
                                                                   VkAndroidHardwareBufferPropertiesANDROID* pProperties) {
    VkResult result = device_dispatch_table.GetAndroidHardwareBufferPropertiesANDROID(device, buffer, pProperties);

    return result;
}

VkResult DispatchDevice::GetMemoryAndroidHardwareBufferANDROID(VkDevice device,
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

VkResult DispatchDevice::CreateExecutionGraphPipelinesAMDX(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
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
    if (result == VK_SUCCESS) {
        for (uint32_t index0 = 0; index0 < createInfoCount; index0++) {
            pPipelines[index0] = WrapNew(pPipelines[index0]);
        }
    }
    return result;
}

VkResult DispatchDevice::GetExecutionGraphPipelineScratchSizeAMDX(VkDevice device, VkPipeline executionGraph,
                                                                  VkExecutionGraphPipelineScratchSizeAMDX* pSizeInfo) {
    if (!wrap_handles) return device_dispatch_table.GetExecutionGraphPipelineScratchSizeAMDX(device, executionGraph, pSizeInfo);
    {
        executionGraph = Unwrap(executionGraph);
    }
    VkResult result = device_dispatch_table.GetExecutionGraphPipelineScratchSizeAMDX(device, executionGraph, pSizeInfo);

    return result;
}

VkResult DispatchDevice::GetExecutionGraphPipelineNodeIndexAMDX(VkDevice device, VkPipeline executionGraph,
                                                                const VkPipelineShaderStageNodeCreateInfoAMDX* pNodeInfo,
                                                                uint32_t* pNodeIndex) {
    if (!wrap_handles)
        return device_dispatch_table.GetExecutionGraphPipelineNodeIndexAMDX(device, executionGraph, pNodeInfo, pNodeIndex);
    {
        executionGraph = Unwrap(executionGraph);
    }
    VkResult result = device_dispatch_table.GetExecutionGraphPipelineNodeIndexAMDX(device, executionGraph, pNodeInfo, pNodeIndex);

    return result;
}

void DispatchDevice::CmdInitializeGraphScratchMemoryAMDX(VkCommandBuffer commandBuffer, VkPipeline executionGraph,
                                                         VkDeviceAddress scratch, VkDeviceSize scratchSize) {
    if (!wrap_handles)
        return device_dispatch_table.CmdInitializeGraphScratchMemoryAMDX(commandBuffer, executionGraph, scratch, scratchSize);
    {
        executionGraph = Unwrap(executionGraph);
    }
    device_dispatch_table.CmdInitializeGraphScratchMemoryAMDX(commandBuffer, executionGraph, scratch, scratchSize);
}

void DispatchDevice::CmdDispatchGraphAMDX(VkCommandBuffer commandBuffer, VkDeviceAddress scratch, VkDeviceSize scratchSize,
                                          const VkDispatchGraphCountInfoAMDX* pCountInfo) {
    device_dispatch_table.CmdDispatchGraphAMDX(commandBuffer, scratch, scratchSize, pCountInfo);
}

void DispatchDevice::CmdDispatchGraphIndirectAMDX(VkCommandBuffer commandBuffer, VkDeviceAddress scratch, VkDeviceSize scratchSize,
                                                  const VkDispatchGraphCountInfoAMDX* pCountInfo) {
    device_dispatch_table.CmdDispatchGraphIndirectAMDX(commandBuffer, scratch, scratchSize, pCountInfo);
}

void DispatchDevice::CmdDispatchGraphIndirectCountAMDX(VkCommandBuffer commandBuffer, VkDeviceAddress scratch,
                                                       VkDeviceSize scratchSize, VkDeviceAddress countInfo) {
    device_dispatch_table.CmdDispatchGraphIndirectCountAMDX(commandBuffer, scratch, scratchSize, countInfo);
}
#endif  // VK_ENABLE_BETA_EXTENSIONS

VkResult DispatchDevice::WriteSamplerDescriptorsEXT(VkDevice device, uint32_t samplerCount, const VkSamplerCreateInfo* pSamplers,
                                                    const VkHostAddressRangeEXT* pDescriptors) {
    if (!wrap_handles) return device_dispatch_table.WriteSamplerDescriptorsEXT(device, samplerCount, pSamplers, pDescriptors);
    small_vector<vku::safe_VkSamplerCreateInfo, DISPATCH_MAX_STACK_ALLOCATIONS> var_local_pSamplers;
    vku::safe_VkSamplerCreateInfo* local_pSamplers = nullptr;
    {
        if (pSamplers) {
            var_local_pSamplers.resize(samplerCount);
            local_pSamplers = var_local_pSamplers.data();
            for (uint32_t index0 = 0; index0 < samplerCount; ++index0) {
                local_pSamplers[index0].initialize(&pSamplers[index0]);
                UnwrapPnextChainHandles(local_pSamplers[index0].pNext);
            }
        }
    }
    VkResult result = device_dispatch_table.WriteSamplerDescriptorsEXT(device, samplerCount,
                                                                       (const VkSamplerCreateInfo*)local_pSamplers, pDescriptors);

    return result;
}

void DispatchDevice::CmdBindSamplerHeapEXT(VkCommandBuffer commandBuffer, const VkBindHeapInfoEXT* pBindInfo) {
    device_dispatch_table.CmdBindSamplerHeapEXT(commandBuffer, pBindInfo);
}

void DispatchDevice::CmdBindResourceHeapEXT(VkCommandBuffer commandBuffer, const VkBindHeapInfoEXT* pBindInfo) {
    device_dispatch_table.CmdBindResourceHeapEXT(commandBuffer, pBindInfo);
}

void DispatchDevice::CmdPushDataEXT(VkCommandBuffer commandBuffer, const VkPushDataInfoEXT* pPushDataInfo) {
    device_dispatch_table.CmdPushDataEXT(commandBuffer, pPushDataInfo);
}

VkResult DispatchDevice::GetImageOpaqueCaptureDataEXT(VkDevice device, uint32_t imageCount, const VkImage* pImages,
                                                      VkHostAddressRangeEXT* pDatas) {
    if (!wrap_handles) return device_dispatch_table.GetImageOpaqueCaptureDataEXT(device, imageCount, pImages, pDatas);
    small_vector<VkImage, DISPATCH_MAX_STACK_ALLOCATIONS> var_local_pImages;
    VkImage* local_pImages = nullptr;
    {
        if (pImages) {
            var_local_pImages.resize(imageCount);
            local_pImages = var_local_pImages.data();
            for (uint32_t index0 = 0; index0 < imageCount; ++index0) {
                local_pImages[index0] = Unwrap(pImages[index0]);
            }
        }
    }
    VkResult result = device_dispatch_table.GetImageOpaqueCaptureDataEXT(device, imageCount, (const VkImage*)local_pImages, pDatas);

    return result;
}

VkDeviceSize DispatchInstance::GetPhysicalDeviceDescriptorSizeEXT(VkPhysicalDevice physicalDevice,
                                                                  VkDescriptorType descriptorType) {
    VkDeviceSize result = instance_dispatch_table.GetPhysicalDeviceDescriptorSizeEXT(physicalDevice, descriptorType);

    return result;
}

VkResult DispatchDevice::RegisterCustomBorderColorEXT(VkDevice device, const VkSamplerCustomBorderColorCreateInfoEXT* pBorderColor,
                                                      VkBool32 requestIndex, uint32_t* pIndex) {
    VkResult result = device_dispatch_table.RegisterCustomBorderColorEXT(device, pBorderColor, requestIndex, pIndex);

    return result;
}

void DispatchDevice::UnregisterCustomBorderColorEXT(VkDevice device, uint32_t index) {
    device_dispatch_table.UnregisterCustomBorderColorEXT(device, index);
}

VkResult DispatchDevice::GetTensorOpaqueCaptureDataARM(VkDevice device, uint32_t tensorCount, const VkTensorARM* pTensors,
                                                       VkHostAddressRangeEXT* pDatas) {
    if (!wrap_handles) return device_dispatch_table.GetTensorOpaqueCaptureDataARM(device, tensorCount, pTensors, pDatas);
    small_vector<VkTensorARM, DISPATCH_MAX_STACK_ALLOCATIONS> var_local_pTensors;
    VkTensorARM* local_pTensors = nullptr;
    {
        if (pTensors) {
            var_local_pTensors.resize(tensorCount);
            local_pTensors = var_local_pTensors.data();
            for (uint32_t index0 = 0; index0 < tensorCount; ++index0) {
                local_pTensors[index0] = Unwrap(pTensors[index0]);
            }
        }
    }
    VkResult result =
        device_dispatch_table.GetTensorOpaqueCaptureDataARM(device, tensorCount, (const VkTensorARM*)local_pTensors, pDatas);

    return result;
}

void DispatchDevice::CmdSetSampleLocationsEXT(VkCommandBuffer commandBuffer, const VkSampleLocationsInfoEXT* pSampleLocationsInfo) {
    device_dispatch_table.CmdSetSampleLocationsEXT(commandBuffer, pSampleLocationsInfo);
}

void DispatchInstance::GetPhysicalDeviceMultisamplePropertiesEXT(VkPhysicalDevice physicalDevice, VkSampleCountFlagBits samples,
                                                                 VkMultisamplePropertiesEXT* pMultisampleProperties) {
    instance_dispatch_table.GetPhysicalDeviceMultisamplePropertiesEXT(physicalDevice, samples, pMultisampleProperties);
}

VkResult DispatchDevice::GetImageDrmFormatModifierPropertiesEXT(VkDevice device, VkImage image,
                                                                VkImageDrmFormatModifierPropertiesEXT* pProperties) {
    if (!wrap_handles) return device_dispatch_table.GetImageDrmFormatModifierPropertiesEXT(device, image, pProperties);
    {
        image = Unwrap(image);
    }
    VkResult result = device_dispatch_table.GetImageDrmFormatModifierPropertiesEXT(device, image, pProperties);

    return result;
}

VkResult DispatchDevice::CreateValidationCacheEXT(VkDevice device, const VkValidationCacheCreateInfoEXT* pCreateInfo,
                                                  const VkAllocationCallbacks* pAllocator, VkValidationCacheEXT* pValidationCache) {
    if (!wrap_handles) return device_dispatch_table.CreateValidationCacheEXT(device, pCreateInfo, pAllocator, pValidationCache);

    VkResult result = device_dispatch_table.CreateValidationCacheEXT(device, pCreateInfo, pAllocator, pValidationCache);
    if (result == VK_SUCCESS) {
        *pValidationCache = WrapNew(*pValidationCache);
    }
    return result;
}

void DispatchDevice::DestroyValidationCacheEXT(VkDevice device, VkValidationCacheEXT validationCache,
                                               const VkAllocationCallbacks* pAllocator) {
    if (!wrap_handles) return device_dispatch_table.DestroyValidationCacheEXT(device, validationCache, pAllocator);
    validationCache = Erase(validationCache);
    device_dispatch_table.DestroyValidationCacheEXT(device, validationCache, pAllocator);
}

VkResult DispatchDevice::MergeValidationCachesEXT(VkDevice device, VkValidationCacheEXT dstCache, uint32_t srcCacheCount,
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

VkResult DispatchDevice::GetValidationCacheDataEXT(VkDevice device, VkValidationCacheEXT validationCache, size_t* pDataSize,
                                                   void* pData) {
    if (!wrap_handles) return device_dispatch_table.GetValidationCacheDataEXT(device, validationCache, pDataSize, pData);
    {
        validationCache = Unwrap(validationCache);
    }
    VkResult result = device_dispatch_table.GetValidationCacheDataEXT(device, validationCache, pDataSize, pData);

    return result;
}

void DispatchDevice::CmdBindShadingRateImageNV(VkCommandBuffer commandBuffer, VkImageView imageView, VkImageLayout imageLayout) {
    if (!wrap_handles) return device_dispatch_table.CmdBindShadingRateImageNV(commandBuffer, imageView, imageLayout);
    {
        imageView = Unwrap(imageView);
    }
    device_dispatch_table.CmdBindShadingRateImageNV(commandBuffer, imageView, imageLayout);
}

void DispatchDevice::CmdSetViewportShadingRatePaletteNV(VkCommandBuffer commandBuffer, uint32_t firstViewport,
                                                        uint32_t viewportCount,
                                                        const VkShadingRatePaletteNV* pShadingRatePalettes) {
    device_dispatch_table.CmdSetViewportShadingRatePaletteNV(commandBuffer, firstViewport, viewportCount, pShadingRatePalettes);
}

void DispatchDevice::CmdSetCoarseSampleOrderNV(VkCommandBuffer commandBuffer, VkCoarseSampleOrderTypeNV sampleOrderType,
                                               uint32_t customSampleOrderCount,
                                               const VkCoarseSampleOrderCustomNV* pCustomSampleOrders) {
    device_dispatch_table.CmdSetCoarseSampleOrderNV(commandBuffer, sampleOrderType, customSampleOrderCount, pCustomSampleOrders);
}

VkResult DispatchDevice::CreateAccelerationStructureNV(VkDevice device, const VkAccelerationStructureCreateInfoNV* pCreateInfo,
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
    if (result == VK_SUCCESS) {
        *pAccelerationStructure = WrapNew(*pAccelerationStructure);
    }
    return result;
}

void DispatchDevice::DestroyAccelerationStructureNV(VkDevice device, VkAccelerationStructureNV accelerationStructure,
                                                    const VkAllocationCallbacks* pAllocator) {
    if (!wrap_handles) return device_dispatch_table.DestroyAccelerationStructureNV(device, accelerationStructure, pAllocator);
    accelerationStructure = Erase(accelerationStructure);
    device_dispatch_table.DestroyAccelerationStructureNV(device, accelerationStructure, pAllocator);
}

void DispatchDevice::GetAccelerationStructureMemoryRequirementsNV(VkDevice device,
                                                                  const VkAccelerationStructureMemoryRequirementsInfoNV* pInfo,
                                                                  VkMemoryRequirements2* pMemoryRequirements) {
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

VkResult DispatchDevice::BindAccelerationStructureMemoryNV(VkDevice device, uint32_t bindInfoCount,
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

void DispatchDevice::CmdBuildAccelerationStructureNV(VkCommandBuffer commandBuffer, const VkAccelerationStructureInfoNV* pInfo,
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

void DispatchDevice::CmdCopyAccelerationStructureNV(VkCommandBuffer commandBuffer, VkAccelerationStructureNV dst,
                                                    VkAccelerationStructureNV src, VkCopyAccelerationStructureModeKHR mode) {
    if (!wrap_handles) return device_dispatch_table.CmdCopyAccelerationStructureNV(commandBuffer, dst, src, mode);
    {
        dst = Unwrap(dst);
        src = Unwrap(src);
    }
    device_dispatch_table.CmdCopyAccelerationStructureNV(commandBuffer, dst, src, mode);
}

void DispatchDevice::CmdTraceRaysNV(VkCommandBuffer commandBuffer, VkBuffer raygenShaderBindingTableBuffer,
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

VkResult DispatchDevice::GetRayTracingShaderGroupHandlesKHR(VkDevice device, VkPipeline pipeline, uint32_t firstGroup,
                                                            uint32_t groupCount, size_t dataSize, void* pData) {
    if (!wrap_handles)
        return device_dispatch_table.GetRayTracingShaderGroupHandlesKHR(device, pipeline, firstGroup, groupCount, dataSize, pData);
    {
        pipeline = Unwrap(pipeline);
    }
    VkResult result =
        device_dispatch_table.GetRayTracingShaderGroupHandlesKHR(device, pipeline, firstGroup, groupCount, dataSize, pData);

    return result;
}

VkResult DispatchDevice::GetRayTracingShaderGroupHandlesNV(VkDevice device, VkPipeline pipeline, uint32_t firstGroup,
                                                           uint32_t groupCount, size_t dataSize, void* pData) {
    if (!wrap_handles)
        return device_dispatch_table.GetRayTracingShaderGroupHandlesNV(device, pipeline, firstGroup, groupCount, dataSize, pData);
    {
        pipeline = Unwrap(pipeline);
    }
    VkResult result =
        device_dispatch_table.GetRayTracingShaderGroupHandlesNV(device, pipeline, firstGroup, groupCount, dataSize, pData);

    return result;
}

VkResult DispatchDevice::GetAccelerationStructureHandleNV(VkDevice device, VkAccelerationStructureNV accelerationStructure,
                                                          size_t dataSize, void* pData) {
    if (!wrap_handles)
        return device_dispatch_table.GetAccelerationStructureHandleNV(device, accelerationStructure, dataSize, pData);
    {
        accelerationStructure = Unwrap(accelerationStructure);
    }
    VkResult result = device_dispatch_table.GetAccelerationStructureHandleNV(device, accelerationStructure, dataSize, pData);

    return result;
}

void DispatchDevice::CmdWriteAccelerationStructuresPropertiesNV(VkCommandBuffer commandBuffer, uint32_t accelerationStructureCount,
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

VkResult DispatchDevice::CompileDeferredNV(VkDevice device, VkPipeline pipeline, uint32_t shader) {
    if (!wrap_handles) return device_dispatch_table.CompileDeferredNV(device, pipeline, shader);
    {
        pipeline = Unwrap(pipeline);
    }
    VkResult result = device_dispatch_table.CompileDeferredNV(device, pipeline, shader);

    return result;
}

VkResult DispatchDevice::GetMemoryHostPointerPropertiesEXT(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType,
                                                           const void* pHostPointer,
                                                           VkMemoryHostPointerPropertiesEXT* pMemoryHostPointerProperties) {
    VkResult result =
        device_dispatch_table.GetMemoryHostPointerPropertiesEXT(device, handleType, pHostPointer, pMemoryHostPointerProperties);

    return result;
}

void DispatchDevice::CmdWriteBufferMarkerAMD(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage,
                                             VkBuffer dstBuffer, VkDeviceSize dstOffset, uint32_t marker) {
    if (!wrap_handles)
        return device_dispatch_table.CmdWriteBufferMarkerAMD(commandBuffer, pipelineStage, dstBuffer, dstOffset, marker);
    {
        dstBuffer = Unwrap(dstBuffer);
    }
    device_dispatch_table.CmdWriteBufferMarkerAMD(commandBuffer, pipelineStage, dstBuffer, dstOffset, marker);
}

void DispatchDevice::CmdWriteBufferMarker2AMD(VkCommandBuffer commandBuffer, VkPipelineStageFlags2 stage, VkBuffer dstBuffer,
                                              VkDeviceSize dstOffset, uint32_t marker) {
    if (!wrap_handles) return device_dispatch_table.CmdWriteBufferMarker2AMD(commandBuffer, stage, dstBuffer, dstOffset, marker);
    {
        dstBuffer = Unwrap(dstBuffer);
    }
    device_dispatch_table.CmdWriteBufferMarker2AMD(commandBuffer, stage, dstBuffer, dstOffset, marker);
}

VkResult DispatchInstance::GetPhysicalDeviceCalibrateableTimeDomainsEXT(VkPhysicalDevice physicalDevice, uint32_t* pTimeDomainCount,
                                                                        VkTimeDomainKHR* pTimeDomains) {
    VkResult result =
        instance_dispatch_table.GetPhysicalDeviceCalibrateableTimeDomainsEXT(physicalDevice, pTimeDomainCount, pTimeDomains);

    return result;
}

VkResult DispatchDevice::GetCalibratedTimestampsEXT(VkDevice device, uint32_t timestampCount,
                                                    const VkCalibratedTimestampInfoKHR* pTimestampInfos, uint64_t* pTimestamps,
                                                    uint64_t* pMaxDeviation) {
    if (!wrap_handles)
        return device_dispatch_table.GetCalibratedTimestampsEXT(device, timestampCount, pTimestampInfos, pTimestamps,
                                                                pMaxDeviation);
    small_vector<vku::safe_VkCalibratedTimestampInfoKHR, DISPATCH_MAX_STACK_ALLOCATIONS> var_local_pTimestampInfos;
    vku::safe_VkCalibratedTimestampInfoKHR* local_pTimestampInfos = nullptr;
    {
        if (pTimestampInfos) {
            var_local_pTimestampInfos.resize(timestampCount);
            local_pTimestampInfos = var_local_pTimestampInfos.data();
            for (uint32_t index0 = 0; index0 < timestampCount; ++index0) {
                local_pTimestampInfos[index0].initialize(&pTimestampInfos[index0]);
                UnwrapPnextChainHandles(local_pTimestampInfos[index0].pNext);
            }
        }
    }
    VkResult result = device_dispatch_table.GetCalibratedTimestampsEXT(
        device, timestampCount, (const VkCalibratedTimestampInfoKHR*)local_pTimestampInfos, pTimestamps, pMaxDeviation);

    return result;
}

void DispatchDevice::CmdDrawMeshTasksNV(VkCommandBuffer commandBuffer, uint32_t taskCount, uint32_t firstTask) {
    device_dispatch_table.CmdDrawMeshTasksNV(commandBuffer, taskCount, firstTask);
}

void DispatchDevice::CmdDrawMeshTasksIndirectNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                uint32_t drawCount, uint32_t stride) {
    if (!wrap_handles) return device_dispatch_table.CmdDrawMeshTasksIndirectNV(commandBuffer, buffer, offset, drawCount, stride);
    {
        buffer = Unwrap(buffer);
    }
    device_dispatch_table.CmdDrawMeshTasksIndirectNV(commandBuffer, buffer, offset, drawCount, stride);
}

void DispatchDevice::CmdDrawMeshTasksIndirectCountNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
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

void DispatchDevice::CmdSetExclusiveScissorEnableNV(VkCommandBuffer commandBuffer, uint32_t firstExclusiveScissor,
                                                    uint32_t exclusiveScissorCount, const VkBool32* pExclusiveScissorEnables) {
    device_dispatch_table.CmdSetExclusiveScissorEnableNV(commandBuffer, firstExclusiveScissor, exclusiveScissorCount,
                                                         pExclusiveScissorEnables);
}

void DispatchDevice::CmdSetExclusiveScissorNV(VkCommandBuffer commandBuffer, uint32_t firstExclusiveScissor,
                                              uint32_t exclusiveScissorCount, const VkRect2D* pExclusiveScissors) {
    device_dispatch_table.CmdSetExclusiveScissorNV(commandBuffer, firstExclusiveScissor, exclusiveScissorCount, pExclusiveScissors);
}

void DispatchDevice::CmdSetCheckpointNV(VkCommandBuffer commandBuffer, const void* pCheckpointMarker) {
    device_dispatch_table.CmdSetCheckpointNV(commandBuffer, pCheckpointMarker);
}

void DispatchDevice::GetQueueCheckpointDataNV(VkQueue queue, uint32_t* pCheckpointDataCount, VkCheckpointDataNV* pCheckpointData) {
    device_dispatch_table.GetQueueCheckpointDataNV(queue, pCheckpointDataCount, pCheckpointData);
}

void DispatchDevice::GetQueueCheckpointData2NV(VkQueue queue, uint32_t* pCheckpointDataCount,
                                               VkCheckpointData2NV* pCheckpointData) {
    device_dispatch_table.GetQueueCheckpointData2NV(queue, pCheckpointDataCount, pCheckpointData);
}

VkResult DispatchDevice::SetSwapchainPresentTimingQueueSizeEXT(VkDevice device, VkSwapchainKHR swapchain, uint32_t size) {
    if (!wrap_handles) return device_dispatch_table.SetSwapchainPresentTimingQueueSizeEXT(device, swapchain, size);
    {
        swapchain = Unwrap(swapchain);
    }
    VkResult result = device_dispatch_table.SetSwapchainPresentTimingQueueSizeEXT(device, swapchain, size);

    return result;
}

VkResult DispatchDevice::GetSwapchainTimingPropertiesEXT(VkDevice device, VkSwapchainKHR swapchain,
                                                         VkSwapchainTimingPropertiesEXT* pSwapchainTimingProperties,
                                                         uint64_t* pSwapchainTimingPropertiesCounter) {
    if (!wrap_handles)
        return device_dispatch_table.GetSwapchainTimingPropertiesEXT(device, swapchain, pSwapchainTimingProperties,
                                                                     pSwapchainTimingPropertiesCounter);
    {
        swapchain = Unwrap(swapchain);
    }
    VkResult result = device_dispatch_table.GetSwapchainTimingPropertiesEXT(device, swapchain, pSwapchainTimingProperties,
                                                                            pSwapchainTimingPropertiesCounter);

    return result;
}

VkResult DispatchDevice::GetSwapchainTimeDomainPropertiesEXT(VkDevice device, VkSwapchainKHR swapchain,
                                                             VkSwapchainTimeDomainPropertiesEXT* pSwapchainTimeDomainProperties,
                                                             uint64_t* pTimeDomainsCounter) {
    if (!wrap_handles)
        return device_dispatch_table.GetSwapchainTimeDomainPropertiesEXT(device, swapchain, pSwapchainTimeDomainProperties,
                                                                         pTimeDomainsCounter);
    {
        swapchain = Unwrap(swapchain);
    }
    VkResult result = device_dispatch_table.GetSwapchainTimeDomainPropertiesEXT(device, swapchain, pSwapchainTimeDomainProperties,
                                                                                pTimeDomainsCounter);

    return result;
}

VkResult DispatchDevice::GetPastPresentationTimingEXT(VkDevice device,
                                                      const VkPastPresentationTimingInfoEXT* pPastPresentationTimingInfo,
                                                      VkPastPresentationTimingPropertiesEXT* pPastPresentationTimingProperties) {
    if (!wrap_handles)
        return device_dispatch_table.GetPastPresentationTimingEXT(device, pPastPresentationTimingInfo,
                                                                  pPastPresentationTimingProperties);
    vku::safe_VkPastPresentationTimingInfoEXT var_local_pPastPresentationTimingInfo;
    vku::safe_VkPastPresentationTimingInfoEXT* local_pPastPresentationTimingInfo = nullptr;
    {
        if (pPastPresentationTimingInfo) {
            local_pPastPresentationTimingInfo = &var_local_pPastPresentationTimingInfo;
            local_pPastPresentationTimingInfo->initialize(pPastPresentationTimingInfo);

            if (pPastPresentationTimingInfo->swapchain) {
                local_pPastPresentationTimingInfo->swapchain = Unwrap(pPastPresentationTimingInfo->swapchain);
            }
        }
    }
    VkResult result = device_dispatch_table.GetPastPresentationTimingEXT(
        device, (const VkPastPresentationTimingInfoEXT*)local_pPastPresentationTimingInfo, pPastPresentationTimingProperties);

    return result;
}

VkResult DispatchDevice::InitializePerformanceApiINTEL(VkDevice device,
                                                       const VkInitializePerformanceApiInfoINTEL* pInitializeInfo) {
    VkResult result = device_dispatch_table.InitializePerformanceApiINTEL(device, pInitializeInfo);

    return result;
}

void DispatchDevice::UninitializePerformanceApiINTEL(VkDevice device) {
    device_dispatch_table.UninitializePerformanceApiINTEL(device);
}

VkResult DispatchDevice::CmdSetPerformanceMarkerINTEL(VkCommandBuffer commandBuffer,
                                                      const VkPerformanceMarkerInfoINTEL* pMarkerInfo) {
    VkResult result = device_dispatch_table.CmdSetPerformanceMarkerINTEL(commandBuffer, pMarkerInfo);

    return result;
}

VkResult DispatchDevice::CmdSetPerformanceStreamMarkerINTEL(VkCommandBuffer commandBuffer,
                                                            const VkPerformanceStreamMarkerInfoINTEL* pMarkerInfo) {
    VkResult result = device_dispatch_table.CmdSetPerformanceStreamMarkerINTEL(commandBuffer, pMarkerInfo);

    return result;
}

VkResult DispatchDevice::CmdSetPerformanceOverrideINTEL(VkCommandBuffer commandBuffer,
                                                        const VkPerformanceOverrideInfoINTEL* pOverrideInfo) {
    VkResult result = device_dispatch_table.CmdSetPerformanceOverrideINTEL(commandBuffer, pOverrideInfo);

    return result;
}

VkResult DispatchDevice::AcquirePerformanceConfigurationINTEL(VkDevice device,
                                                              const VkPerformanceConfigurationAcquireInfoINTEL* pAcquireInfo,
                                                              VkPerformanceConfigurationINTEL* pConfiguration) {
    if (!wrap_handles) return device_dispatch_table.AcquirePerformanceConfigurationINTEL(device, pAcquireInfo, pConfiguration);

    VkResult result = device_dispatch_table.AcquirePerformanceConfigurationINTEL(device, pAcquireInfo, pConfiguration);
    if (result == VK_SUCCESS) {
        *pConfiguration = WrapNew(*pConfiguration);
    }
    return result;
}

VkResult DispatchDevice::QueueSetPerformanceConfigurationINTEL(VkQueue queue, VkPerformanceConfigurationINTEL configuration) {
    if (!wrap_handles) return device_dispatch_table.QueueSetPerformanceConfigurationINTEL(queue, configuration);
    {
        configuration = Unwrap(configuration);
    }
    VkResult result = device_dispatch_table.QueueSetPerformanceConfigurationINTEL(queue, configuration);

    return result;
}

VkResult DispatchDevice::GetPerformanceParameterINTEL(VkDevice device, VkPerformanceParameterTypeINTEL parameter,
                                                      VkPerformanceValueINTEL* pValue) {
    VkResult result = device_dispatch_table.GetPerformanceParameterINTEL(device, parameter, pValue);

    return result;
}

void DispatchDevice::SetLocalDimmingAMD(VkDevice device, VkSwapchainKHR swapChain, VkBool32 localDimmingEnable) {
    if (!wrap_handles) return device_dispatch_table.SetLocalDimmingAMD(device, swapChain, localDimmingEnable);
    {
        swapChain = Unwrap(swapChain);
    }
    device_dispatch_table.SetLocalDimmingAMD(device, swapChain, localDimmingEnable);
}
#ifdef VK_USE_PLATFORM_FUCHSIA

VkResult DispatchInstance::CreateImagePipeSurfaceFUCHSIA(VkInstance instance,
                                                         const VkImagePipeSurfaceCreateInfoFUCHSIA* pCreateInfo,
                                                         const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    if (!wrap_handles) return instance_dispatch_table.CreateImagePipeSurfaceFUCHSIA(instance, pCreateInfo, pAllocator, pSurface);

    VkResult result = instance_dispatch_table.CreateImagePipeSurfaceFUCHSIA(instance, pCreateInfo, pAllocator, pSurface);
    if (result == VK_SUCCESS) {
        *pSurface = WrapNew(*pSurface);
    }
    return result;
}
#endif  // VK_USE_PLATFORM_FUCHSIA
#ifdef VK_USE_PLATFORM_METAL_EXT

VkResult DispatchInstance::CreateMetalSurfaceEXT(VkInstance instance, const VkMetalSurfaceCreateInfoEXT* pCreateInfo,
                                                 const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    if (!wrap_handles) return instance_dispatch_table.CreateMetalSurfaceEXT(instance, pCreateInfo, pAllocator, pSurface);

    VkResult result = instance_dispatch_table.CreateMetalSurfaceEXT(instance, pCreateInfo, pAllocator, pSurface);
    if (result == VK_SUCCESS) {
        *pSurface = WrapNew(*pSurface);
    }
    return result;
}
#endif  // VK_USE_PLATFORM_METAL_EXT

VkDeviceAddress DispatchDevice::GetBufferDeviceAddressEXT(VkDevice device, const VkBufferDeviceAddressInfo* pInfo) {
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

VkResult DispatchInstance::GetPhysicalDeviceCooperativeMatrixPropertiesNV(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount,
                                                                          VkCooperativeMatrixPropertiesNV* pProperties) {
    VkResult result =
        instance_dispatch_table.GetPhysicalDeviceCooperativeMatrixPropertiesNV(physicalDevice, pPropertyCount, pProperties);

    return result;
}

VkResult DispatchInstance::GetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV(
    VkPhysicalDevice physicalDevice, uint32_t* pCombinationCount, VkFramebufferMixedSamplesCombinationNV* pCombinations) {
    VkResult result = instance_dispatch_table.GetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV(
        physicalDevice, pCombinationCount, pCombinations);

    return result;
}
#ifdef VK_USE_PLATFORM_WIN32_KHR

VkResult DispatchInstance::GetPhysicalDeviceSurfacePresentModes2EXT(VkPhysicalDevice physicalDevice,
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

VkResult DispatchDevice::AcquireFullScreenExclusiveModeEXT(VkDevice device, VkSwapchainKHR swapchain) {
    if (!wrap_handles) return device_dispatch_table.AcquireFullScreenExclusiveModeEXT(device, swapchain);
    {
        swapchain = Unwrap(swapchain);
    }
    VkResult result = device_dispatch_table.AcquireFullScreenExclusiveModeEXT(device, swapchain);

    return result;
}

VkResult DispatchDevice::ReleaseFullScreenExclusiveModeEXT(VkDevice device, VkSwapchainKHR swapchain) {
    if (!wrap_handles) return device_dispatch_table.ReleaseFullScreenExclusiveModeEXT(device, swapchain);
    {
        swapchain = Unwrap(swapchain);
    }
    VkResult result = device_dispatch_table.ReleaseFullScreenExclusiveModeEXT(device, swapchain);

    return result;
}

VkResult DispatchDevice::GetDeviceGroupSurfacePresentModes2EXT(VkDevice device, const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo,
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

VkResult DispatchInstance::CreateHeadlessSurfaceEXT(VkInstance instance, const VkHeadlessSurfaceCreateInfoEXT* pCreateInfo,
                                                    const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    if (!wrap_handles) return instance_dispatch_table.CreateHeadlessSurfaceEXT(instance, pCreateInfo, pAllocator, pSurface);

    VkResult result = instance_dispatch_table.CreateHeadlessSurfaceEXT(instance, pCreateInfo, pAllocator, pSurface);
    if (result == VK_SUCCESS) {
        *pSurface = WrapNew(*pSurface);
    }
    return result;
}

void DispatchDevice::CmdSetLineStippleEXT(VkCommandBuffer commandBuffer, uint32_t lineStippleFactor, uint16_t lineStipplePattern) {
    device_dispatch_table.CmdSetLineStippleEXT(commandBuffer, lineStippleFactor, lineStipplePattern);
}

void DispatchDevice::ResetQueryPoolEXT(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount) {
    if (!wrap_handles) return device_dispatch_table.ResetQueryPoolEXT(device, queryPool, firstQuery, queryCount);
    {
        queryPool = Unwrap(queryPool);
    }
    device_dispatch_table.ResetQueryPoolEXT(device, queryPool, firstQuery, queryCount);
}

void DispatchDevice::CmdSetCullModeEXT(VkCommandBuffer commandBuffer, VkCullModeFlags cullMode) {
    device_dispatch_table.CmdSetCullModeEXT(commandBuffer, cullMode);
}

void DispatchDevice::CmdSetFrontFaceEXT(VkCommandBuffer commandBuffer, VkFrontFace frontFace) {
    device_dispatch_table.CmdSetFrontFaceEXT(commandBuffer, frontFace);
}

void DispatchDevice::CmdSetPrimitiveTopologyEXT(VkCommandBuffer commandBuffer, VkPrimitiveTopology primitiveTopology) {
    device_dispatch_table.CmdSetPrimitiveTopologyEXT(commandBuffer, primitiveTopology);
}

void DispatchDevice::CmdSetViewportWithCountEXT(VkCommandBuffer commandBuffer, uint32_t viewportCount,
                                                const VkViewport* pViewports) {
    device_dispatch_table.CmdSetViewportWithCountEXT(commandBuffer, viewportCount, pViewports);
}

void DispatchDevice::CmdSetScissorWithCountEXT(VkCommandBuffer commandBuffer, uint32_t scissorCount, const VkRect2D* pScissors) {
    device_dispatch_table.CmdSetScissorWithCountEXT(commandBuffer, scissorCount, pScissors);
}

void DispatchDevice::CmdBindVertexBuffers2EXT(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount,
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

void DispatchDevice::CmdSetDepthTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthTestEnable) {
    device_dispatch_table.CmdSetDepthTestEnableEXT(commandBuffer, depthTestEnable);
}

void DispatchDevice::CmdSetDepthWriteEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthWriteEnable) {
    device_dispatch_table.CmdSetDepthWriteEnableEXT(commandBuffer, depthWriteEnable);
}

void DispatchDevice::CmdSetDepthCompareOpEXT(VkCommandBuffer commandBuffer, VkCompareOp depthCompareOp) {
    device_dispatch_table.CmdSetDepthCompareOpEXT(commandBuffer, depthCompareOp);
}

void DispatchDevice::CmdSetDepthBoundsTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthBoundsTestEnable) {
    device_dispatch_table.CmdSetDepthBoundsTestEnableEXT(commandBuffer, depthBoundsTestEnable);
}

void DispatchDevice::CmdSetStencilTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 stencilTestEnable) {
    device_dispatch_table.CmdSetStencilTestEnableEXT(commandBuffer, stencilTestEnable);
}

void DispatchDevice::CmdSetStencilOpEXT(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, VkStencilOp failOp,
                                        VkStencilOp passOp, VkStencilOp depthFailOp, VkCompareOp compareOp) {
    device_dispatch_table.CmdSetStencilOpEXT(commandBuffer, faceMask, failOp, passOp, depthFailOp, compareOp);
}

VkResult DispatchDevice::CopyMemoryToImageEXT(VkDevice device, const VkCopyMemoryToImageInfo* pCopyMemoryToImageInfo) {
    if (!wrap_handles) return device_dispatch_table.CopyMemoryToImageEXT(device, pCopyMemoryToImageInfo);
    vku::safe_VkCopyMemoryToImageInfo var_local_pCopyMemoryToImageInfo;
    vku::safe_VkCopyMemoryToImageInfo* local_pCopyMemoryToImageInfo = nullptr;
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
        device_dispatch_table.CopyMemoryToImageEXT(device, (const VkCopyMemoryToImageInfo*)local_pCopyMemoryToImageInfo);

    return result;
}

VkResult DispatchDevice::CopyImageToMemoryEXT(VkDevice device, const VkCopyImageToMemoryInfo* pCopyImageToMemoryInfo) {
    if (!wrap_handles) return device_dispatch_table.CopyImageToMemoryEXT(device, pCopyImageToMemoryInfo);
    vku::safe_VkCopyImageToMemoryInfo var_local_pCopyImageToMemoryInfo;
    vku::safe_VkCopyImageToMemoryInfo* local_pCopyImageToMemoryInfo = nullptr;
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
        device_dispatch_table.CopyImageToMemoryEXT(device, (const VkCopyImageToMemoryInfo*)local_pCopyImageToMemoryInfo);

    return result;
}

VkResult DispatchDevice::CopyImageToImageEXT(VkDevice device, const VkCopyImageToImageInfo* pCopyImageToImageInfo) {
    if (!wrap_handles) return device_dispatch_table.CopyImageToImageEXT(device, pCopyImageToImageInfo);
    vku::safe_VkCopyImageToImageInfo var_local_pCopyImageToImageInfo;
    vku::safe_VkCopyImageToImageInfo* local_pCopyImageToImageInfo = nullptr;
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
    VkResult result = device_dispatch_table.CopyImageToImageEXT(device, (const VkCopyImageToImageInfo*)local_pCopyImageToImageInfo);

    return result;
}

VkResult DispatchDevice::TransitionImageLayoutEXT(VkDevice device, uint32_t transitionCount,
                                                  const VkHostImageLayoutTransitionInfo* pTransitions) {
    if (!wrap_handles) return device_dispatch_table.TransitionImageLayoutEXT(device, transitionCount, pTransitions);
    small_vector<vku::safe_VkHostImageLayoutTransitionInfo, DISPATCH_MAX_STACK_ALLOCATIONS> var_local_pTransitions;
    vku::safe_VkHostImageLayoutTransitionInfo* local_pTransitions = nullptr;
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
                                                                     (const VkHostImageLayoutTransitionInfo*)local_pTransitions);

    return result;
}

void DispatchDevice::GetImageSubresourceLayout2EXT(VkDevice device, VkImage image, const VkImageSubresource2* pSubresource,
                                                   VkSubresourceLayout2* pLayout) {
    if (!wrap_handles) return device_dispatch_table.GetImageSubresourceLayout2EXT(device, image, pSubresource, pLayout);
    {
        image = Unwrap(image);
    }
    device_dispatch_table.GetImageSubresourceLayout2EXT(device, image, pSubresource, pLayout);
}

VkResult DispatchDevice::ReleaseSwapchainImagesEXT(VkDevice device, const VkReleaseSwapchainImagesInfoKHR* pReleaseInfo) {
    if (!wrap_handles) return device_dispatch_table.ReleaseSwapchainImagesEXT(device, pReleaseInfo);
    vku::safe_VkReleaseSwapchainImagesInfoKHR var_local_pReleaseInfo;
    vku::safe_VkReleaseSwapchainImagesInfoKHR* local_pReleaseInfo = nullptr;
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
        device_dispatch_table.ReleaseSwapchainImagesEXT(device, (const VkReleaseSwapchainImagesInfoKHR*)local_pReleaseInfo);

    return result;
}

void DispatchDevice::GetGeneratedCommandsMemoryRequirementsNV(VkDevice device,
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

void DispatchDevice::CmdPreprocessGeneratedCommandsNV(VkCommandBuffer commandBuffer,
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

void DispatchDevice::CmdExecuteGeneratedCommandsNV(VkCommandBuffer commandBuffer, VkBool32 isPreprocessed,
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

void DispatchDevice::CmdBindPipelineShaderGroupNV(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                  VkPipeline pipeline, uint32_t groupIndex) {
    if (!wrap_handles)
        return device_dispatch_table.CmdBindPipelineShaderGroupNV(commandBuffer, pipelineBindPoint, pipeline, groupIndex);
    {
        pipeline = Unwrap(pipeline);
    }
    device_dispatch_table.CmdBindPipelineShaderGroupNV(commandBuffer, pipelineBindPoint, pipeline, groupIndex);
}

VkResult DispatchDevice::CreateIndirectCommandsLayoutNV(VkDevice device, const VkIndirectCommandsLayoutCreateInfoNV* pCreateInfo,
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
    if (result == VK_SUCCESS) {
        *pIndirectCommandsLayout = WrapNew(*pIndirectCommandsLayout);
    }
    return result;
}

void DispatchDevice::DestroyIndirectCommandsLayoutNV(VkDevice device, VkIndirectCommandsLayoutNV indirectCommandsLayout,
                                                     const VkAllocationCallbacks* pAllocator) {
    if (!wrap_handles) return device_dispatch_table.DestroyIndirectCommandsLayoutNV(device, indirectCommandsLayout, pAllocator);
    indirectCommandsLayout = Erase(indirectCommandsLayout);
    device_dispatch_table.DestroyIndirectCommandsLayoutNV(device, indirectCommandsLayout, pAllocator);
}

void DispatchDevice::CmdSetDepthBias2EXT(VkCommandBuffer commandBuffer, const VkDepthBiasInfoEXT* pDepthBiasInfo) {
    device_dispatch_table.CmdSetDepthBias2EXT(commandBuffer, pDepthBiasInfo);
}

VkResult DispatchInstance::AcquireDrmDisplayEXT(VkPhysicalDevice physicalDevice, int32_t drmFd, VkDisplayKHR display) {
    if (!wrap_handles) return instance_dispatch_table.AcquireDrmDisplayEXT(physicalDevice, drmFd, display);
    {
        display = Unwrap(display);
    }
    VkResult result = instance_dispatch_table.AcquireDrmDisplayEXT(physicalDevice, drmFd, display);

    return result;
}

VkResult DispatchInstance::GetDrmDisplayEXT(VkPhysicalDevice physicalDevice, int32_t drmFd, uint32_t connectorId,
                                            VkDisplayKHR* display) {
    if (!wrap_handles) return instance_dispatch_table.GetDrmDisplayEXT(physicalDevice, drmFd, connectorId, display);

    VkResult result = instance_dispatch_table.GetDrmDisplayEXT(physicalDevice, drmFd, connectorId, display);
    if (result == VK_SUCCESS) {
        *display = MaybeWrapDisplay(*display);
    }
    return result;
}

VkResult DispatchDevice::CreatePrivateDataSlotEXT(VkDevice device, const VkPrivateDataSlotCreateInfo* pCreateInfo,
                                                  const VkAllocationCallbacks* pAllocator, VkPrivateDataSlot* pPrivateDataSlot) {
    if (!wrap_handles) return device_dispatch_table.CreatePrivateDataSlotEXT(device, pCreateInfo, pAllocator, pPrivateDataSlot);

    VkResult result = device_dispatch_table.CreatePrivateDataSlotEXT(device, pCreateInfo, pAllocator, pPrivateDataSlot);
    if (result == VK_SUCCESS) {
        *pPrivateDataSlot = WrapNew(*pPrivateDataSlot);
    }
    return result;
}

void DispatchDevice::DestroyPrivateDataSlotEXT(VkDevice device, VkPrivateDataSlot privateDataSlot,
                                               const VkAllocationCallbacks* pAllocator) {
    if (!wrap_handles) return device_dispatch_table.DestroyPrivateDataSlotEXT(device, privateDataSlot, pAllocator);
    privateDataSlot = Erase(privateDataSlot);
    device_dispatch_table.DestroyPrivateDataSlotEXT(device, privateDataSlot, pAllocator);
}

VkResult DispatchDevice::SetPrivateDataEXT(VkDevice device, VkObjectType objectType, uint64_t objectHandle,
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

void DispatchDevice::GetPrivateDataEXT(VkDevice device, VkObjectType objectType, uint64_t objectHandle,
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

VkResult DispatchDevice::QueueSetPerfHintQCOM(VkQueue queue, const VkPerfHintInfoQCOM* pPerfHintInfo) {
    VkResult result = device_dispatch_table.QueueSetPerfHintQCOM(queue, pPerfHintInfo);

    return result;
}
#ifdef VK_ENABLE_BETA_EXTENSIONS

VkResult DispatchDevice::CreateCudaModuleNV(VkDevice device, const VkCudaModuleCreateInfoNV* pCreateInfo,
                                            const VkAllocationCallbacks* pAllocator, VkCudaModuleNV* pModule) {
    if (!wrap_handles) return device_dispatch_table.CreateCudaModuleNV(device, pCreateInfo, pAllocator, pModule);

    VkResult result = device_dispatch_table.CreateCudaModuleNV(device, pCreateInfo, pAllocator, pModule);
    if (result == VK_SUCCESS) {
        *pModule = WrapNew(*pModule);
    }
    return result;
}

VkResult DispatchDevice::GetCudaModuleCacheNV(VkDevice device, VkCudaModuleNV module, size_t* pCacheSize, void* pCacheData) {
    if (!wrap_handles) return device_dispatch_table.GetCudaModuleCacheNV(device, module, pCacheSize, pCacheData);
    {
        module = Unwrap(module);
    }
    VkResult result = device_dispatch_table.GetCudaModuleCacheNV(device, module, pCacheSize, pCacheData);

    return result;
}

VkResult DispatchDevice::CreateCudaFunctionNV(VkDevice device, const VkCudaFunctionCreateInfoNV* pCreateInfo,
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
    if (result == VK_SUCCESS) {
        *pFunction = WrapNew(*pFunction);
    }
    return result;
}

void DispatchDevice::DestroyCudaModuleNV(VkDevice device, VkCudaModuleNV module, const VkAllocationCallbacks* pAllocator) {
    if (!wrap_handles) return device_dispatch_table.DestroyCudaModuleNV(device, module, pAllocator);
    module = Erase(module);
    device_dispatch_table.DestroyCudaModuleNV(device, module, pAllocator);
}

void DispatchDevice::DestroyCudaFunctionNV(VkDevice device, VkCudaFunctionNV function, const VkAllocationCallbacks* pAllocator) {
    if (!wrap_handles) return device_dispatch_table.DestroyCudaFunctionNV(device, function, pAllocator);
    function = Erase(function);
    device_dispatch_table.DestroyCudaFunctionNV(device, function, pAllocator);
}

void DispatchDevice::CmdCudaLaunchKernelNV(VkCommandBuffer commandBuffer, const VkCudaLaunchInfoNV* pLaunchInfo) {
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
#endif  // VK_ENABLE_BETA_EXTENSIONS

void DispatchDevice::CmdDispatchTileQCOM(VkCommandBuffer commandBuffer, const VkDispatchTileInfoQCOM* pDispatchTileInfo) {
    device_dispatch_table.CmdDispatchTileQCOM(commandBuffer, pDispatchTileInfo);
}

void DispatchDevice::CmdBeginPerTileExecutionQCOM(VkCommandBuffer commandBuffer, const VkPerTileBeginInfoQCOM* pPerTileBeginInfo) {
    device_dispatch_table.CmdBeginPerTileExecutionQCOM(commandBuffer, pPerTileBeginInfo);
}

void DispatchDevice::CmdEndPerTileExecutionQCOM(VkCommandBuffer commandBuffer, const VkPerTileEndInfoQCOM* pPerTileEndInfo) {
    device_dispatch_table.CmdEndPerTileExecutionQCOM(commandBuffer, pPerTileEndInfo);
}

void DispatchDevice::GetDescriptorSetLayoutSizeEXT(VkDevice device, VkDescriptorSetLayout layout,
                                                   VkDeviceSize* pLayoutSizeInBytes) {
    if (!wrap_handles) return device_dispatch_table.GetDescriptorSetLayoutSizeEXT(device, layout, pLayoutSizeInBytes);
    {
        layout = Unwrap(layout);
    }
    device_dispatch_table.GetDescriptorSetLayoutSizeEXT(device, layout, pLayoutSizeInBytes);
}

void DispatchDevice::GetDescriptorSetLayoutBindingOffsetEXT(VkDevice device, VkDescriptorSetLayout layout, uint32_t binding,
                                                            VkDeviceSize* pOffset) {
    if (!wrap_handles) return device_dispatch_table.GetDescriptorSetLayoutBindingOffsetEXT(device, layout, binding, pOffset);
    {
        layout = Unwrap(layout);
    }
    device_dispatch_table.GetDescriptorSetLayoutBindingOffsetEXT(device, layout, binding, pOffset);
}

void DispatchDevice::CmdBindDescriptorBuffersEXT(VkCommandBuffer commandBuffer, uint32_t bufferCount,
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

void DispatchDevice::CmdSetDescriptorBufferOffsetsEXT(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                      VkPipelineLayout layout, uint32_t firstSet, uint32_t setCount,
                                                      const uint32_t* pBufferIndices, const VkDeviceSize* pOffsets) {
    if (!wrap_handles)
        return device_dispatch_table.CmdSetDescriptorBufferOffsetsEXT(commandBuffer, pipelineBindPoint, layout, firstSet, setCount,
                                                                      pBufferIndices, pOffsets);
    {
        layout = Unwrap(layout);
    }
    device_dispatch_table.CmdSetDescriptorBufferOffsetsEXT(commandBuffer, pipelineBindPoint, layout, firstSet, setCount,
                                                           pBufferIndices, pOffsets);
}

void DispatchDevice::CmdBindDescriptorBufferEmbeddedSamplersEXT(VkCommandBuffer commandBuffer,
                                                                VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout,
                                                                uint32_t set) {
    if (!wrap_handles)
        return device_dispatch_table.CmdBindDescriptorBufferEmbeddedSamplersEXT(commandBuffer, pipelineBindPoint, layout, set);
    {
        layout = Unwrap(layout);
    }
    device_dispatch_table.CmdBindDescriptorBufferEmbeddedSamplersEXT(commandBuffer, pipelineBindPoint, layout, set);
}

VkResult DispatchDevice::GetBufferOpaqueCaptureDescriptorDataEXT(VkDevice device, const VkBufferCaptureDescriptorDataInfoEXT* pInfo,
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

VkResult DispatchDevice::GetImageOpaqueCaptureDescriptorDataEXT(VkDevice device, const VkImageCaptureDescriptorDataInfoEXT* pInfo,
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

VkResult DispatchDevice::GetImageViewOpaqueCaptureDescriptorDataEXT(VkDevice device,
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

VkResult DispatchDevice::GetSamplerOpaqueCaptureDescriptorDataEXT(VkDevice device,
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

VkResult DispatchDevice::GetAccelerationStructureOpaqueCaptureDescriptorDataEXT(
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

void DispatchDevice::CmdSetFragmentShadingRateEnumNV(VkCommandBuffer commandBuffer, VkFragmentShadingRateNV shadingRate,
                                                     const VkFragmentShadingRateCombinerOpKHR combinerOps[2]) {
    device_dispatch_table.CmdSetFragmentShadingRateEnumNV(commandBuffer, shadingRate, combinerOps);
}

VkResult DispatchDevice::GetDeviceFaultInfoEXT(VkDevice device, VkDeviceFaultCountsEXT* pFaultCounts,
                                               VkDeviceFaultInfoEXT* pFaultInfo) {
    VkResult result = device_dispatch_table.GetDeviceFaultInfoEXT(device, pFaultCounts, pFaultInfo);

    return result;
}
#ifdef VK_USE_PLATFORM_WIN32_KHR

VkResult DispatchInstance::AcquireWinrtDisplayNV(VkPhysicalDevice physicalDevice, VkDisplayKHR display) {
    if (!wrap_handles) return instance_dispatch_table.AcquireWinrtDisplayNV(physicalDevice, display);
    {
        display = Unwrap(display);
    }
    VkResult result = instance_dispatch_table.AcquireWinrtDisplayNV(physicalDevice, display);

    return result;
}

VkResult DispatchInstance::GetWinrtDisplayNV(VkPhysicalDevice physicalDevice, uint32_t deviceRelativeId, VkDisplayKHR* pDisplay) {
    if (!wrap_handles) return instance_dispatch_table.GetWinrtDisplayNV(physicalDevice, deviceRelativeId, pDisplay);

    VkResult result = instance_dispatch_table.GetWinrtDisplayNV(physicalDevice, deviceRelativeId, pDisplay);
    if (result == VK_SUCCESS) {
        *pDisplay = MaybeWrapDisplay(*pDisplay);
    }
    return result;
}
#endif  // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_DIRECTFB_EXT

VkResult DispatchInstance::CreateDirectFBSurfaceEXT(VkInstance instance, const VkDirectFBSurfaceCreateInfoEXT* pCreateInfo,
                                                    const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    if (!wrap_handles) return instance_dispatch_table.CreateDirectFBSurfaceEXT(instance, pCreateInfo, pAllocator, pSurface);

    VkResult result = instance_dispatch_table.CreateDirectFBSurfaceEXT(instance, pCreateInfo, pAllocator, pSurface);
    if (result == VK_SUCCESS) {
        *pSurface = WrapNew(*pSurface);
    }
    return result;
}

VkBool32 DispatchInstance::GetPhysicalDeviceDirectFBPresentationSupportEXT(VkPhysicalDevice physicalDevice,
                                                                           uint32_t queueFamilyIndex, IDirectFB* dfb) {
    VkBool32 result =
        instance_dispatch_table.GetPhysicalDeviceDirectFBPresentationSupportEXT(physicalDevice, queueFamilyIndex, dfb);

    return result;
}
#endif  // VK_USE_PLATFORM_DIRECTFB_EXT

void DispatchDevice::CmdSetVertexInputEXT(VkCommandBuffer commandBuffer, uint32_t vertexBindingDescriptionCount,
                                          const VkVertexInputBindingDescription2EXT* pVertexBindingDescriptions,
                                          uint32_t vertexAttributeDescriptionCount,
                                          const VkVertexInputAttributeDescription2EXT* pVertexAttributeDescriptions) {
    device_dispatch_table.CmdSetVertexInputEXT(commandBuffer, vertexBindingDescriptionCount, pVertexBindingDescriptions,
                                               vertexAttributeDescriptionCount, pVertexAttributeDescriptions);
}
#ifdef VK_USE_PLATFORM_FUCHSIA

VkResult DispatchDevice::GetMemoryZirconHandleFUCHSIA(VkDevice device,
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

VkResult DispatchDevice::GetMemoryZirconHandlePropertiesFUCHSIA(
    VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType, zx_handle_t zirconHandle,
    VkMemoryZirconHandlePropertiesFUCHSIA* pMemoryZirconHandleProperties) {
    VkResult result = device_dispatch_table.GetMemoryZirconHandlePropertiesFUCHSIA(device, handleType, zirconHandle,
                                                                                   pMemoryZirconHandleProperties);

    return result;
}

VkResult DispatchDevice::ImportSemaphoreZirconHandleFUCHSIA(
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

VkResult DispatchDevice::GetSemaphoreZirconHandleFUCHSIA(VkDevice device,
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

VkResult DispatchDevice::CreateBufferCollectionFUCHSIA(VkDevice device, const VkBufferCollectionCreateInfoFUCHSIA* pCreateInfo,
                                                       const VkAllocationCallbacks* pAllocator,
                                                       VkBufferCollectionFUCHSIA* pCollection) {
    if (!wrap_handles) return device_dispatch_table.CreateBufferCollectionFUCHSIA(device, pCreateInfo, pAllocator, pCollection);

    VkResult result = device_dispatch_table.CreateBufferCollectionFUCHSIA(device, pCreateInfo, pAllocator, pCollection);
    if (result == VK_SUCCESS) {
        *pCollection = WrapNew(*pCollection);
    }
    return result;
}

VkResult DispatchDevice::SetBufferCollectionImageConstraintsFUCHSIA(VkDevice device, VkBufferCollectionFUCHSIA collection,
                                                                    const VkImageConstraintsInfoFUCHSIA* pImageConstraintsInfo) {
    if (!wrap_handles)
        return device_dispatch_table.SetBufferCollectionImageConstraintsFUCHSIA(device, collection, pImageConstraintsInfo);
    {
        collection = Unwrap(collection);
    }
    VkResult result = device_dispatch_table.SetBufferCollectionImageConstraintsFUCHSIA(device, collection, pImageConstraintsInfo);

    return result;
}

VkResult DispatchDevice::SetBufferCollectionBufferConstraintsFUCHSIA(VkDevice device, VkBufferCollectionFUCHSIA collection,
                                                                     const VkBufferConstraintsInfoFUCHSIA* pBufferConstraintsInfo) {
    if (!wrap_handles)
        return device_dispatch_table.SetBufferCollectionBufferConstraintsFUCHSIA(device, collection, pBufferConstraintsInfo);
    {
        collection = Unwrap(collection);
    }
    VkResult result = device_dispatch_table.SetBufferCollectionBufferConstraintsFUCHSIA(device, collection, pBufferConstraintsInfo);

    return result;
}

void DispatchDevice::DestroyBufferCollectionFUCHSIA(VkDevice device, VkBufferCollectionFUCHSIA collection,
                                                    const VkAllocationCallbacks* pAllocator) {
    if (!wrap_handles) return device_dispatch_table.DestroyBufferCollectionFUCHSIA(device, collection, pAllocator);
    collection = Erase(collection);
    device_dispatch_table.DestroyBufferCollectionFUCHSIA(device, collection, pAllocator);
}

VkResult DispatchDevice::GetBufferCollectionPropertiesFUCHSIA(VkDevice device, VkBufferCollectionFUCHSIA collection,
                                                              VkBufferCollectionPropertiesFUCHSIA* pProperties) {
    if (!wrap_handles) return device_dispatch_table.GetBufferCollectionPropertiesFUCHSIA(device, collection, pProperties);
    {
        collection = Unwrap(collection);
    }
    VkResult result = device_dispatch_table.GetBufferCollectionPropertiesFUCHSIA(device, collection, pProperties);

    return result;
}
#endif  // VK_USE_PLATFORM_FUCHSIA

VkResult DispatchDevice::GetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI(VkDevice device, VkRenderPass renderpass,
                                                                       VkExtent2D* pMaxWorkgroupSize) {
    if (!wrap_handles)
        return device_dispatch_table.GetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI(device, renderpass, pMaxWorkgroupSize);
    {
        renderpass = Unwrap(renderpass);
    }
    VkResult result = device_dispatch_table.GetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI(device, renderpass, pMaxWorkgroupSize);

    return result;
}

void DispatchDevice::CmdSubpassShadingHUAWEI(VkCommandBuffer commandBuffer) {
    device_dispatch_table.CmdSubpassShadingHUAWEI(commandBuffer);
}

void DispatchDevice::CmdBindInvocationMaskHUAWEI(VkCommandBuffer commandBuffer, VkImageView imageView, VkImageLayout imageLayout) {
    if (!wrap_handles) return device_dispatch_table.CmdBindInvocationMaskHUAWEI(commandBuffer, imageView, imageLayout);
    {
        imageView = Unwrap(imageView);
    }
    device_dispatch_table.CmdBindInvocationMaskHUAWEI(commandBuffer, imageView, imageLayout);
}

VkResult DispatchDevice::GetMemoryRemoteAddressNV(VkDevice device,
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

VkResult DispatchDevice::GetPipelinePropertiesEXT(VkDevice device, const VkPipelineInfoKHR* pPipelineInfo,
                                                  VkBaseOutStructure* pPipelineProperties) {
    if (!wrap_handles) return device_dispatch_table.GetPipelinePropertiesEXT(device, pPipelineInfo, pPipelineProperties);
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
    VkResult result =
        device_dispatch_table.GetPipelinePropertiesEXT(device, (const VkPipelineInfoKHR*)local_pPipelineInfo, pPipelineProperties);

    return result;
}

void DispatchDevice::CmdSetPatchControlPointsEXT(VkCommandBuffer commandBuffer, uint32_t patchControlPoints) {
    device_dispatch_table.CmdSetPatchControlPointsEXT(commandBuffer, patchControlPoints);
}

void DispatchDevice::CmdSetRasterizerDiscardEnableEXT(VkCommandBuffer commandBuffer, VkBool32 rasterizerDiscardEnable) {
    device_dispatch_table.CmdSetRasterizerDiscardEnableEXT(commandBuffer, rasterizerDiscardEnable);
}

void DispatchDevice::CmdSetDepthBiasEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthBiasEnable) {
    device_dispatch_table.CmdSetDepthBiasEnableEXT(commandBuffer, depthBiasEnable);
}

void DispatchDevice::CmdSetLogicOpEXT(VkCommandBuffer commandBuffer, VkLogicOp logicOp) {
    device_dispatch_table.CmdSetLogicOpEXT(commandBuffer, logicOp);
}

void DispatchDevice::CmdSetPrimitiveRestartEnableEXT(VkCommandBuffer commandBuffer, VkBool32 primitiveRestartEnable) {
    device_dispatch_table.CmdSetPrimitiveRestartEnableEXT(commandBuffer, primitiveRestartEnable);
}
#ifdef VK_USE_PLATFORM_SCREEN_QNX

VkResult DispatchInstance::CreateScreenSurfaceQNX(VkInstance instance, const VkScreenSurfaceCreateInfoQNX* pCreateInfo,
                                                  const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    if (!wrap_handles) return instance_dispatch_table.CreateScreenSurfaceQNX(instance, pCreateInfo, pAllocator, pSurface);

    VkResult result = instance_dispatch_table.CreateScreenSurfaceQNX(instance, pCreateInfo, pAllocator, pSurface);
    if (result == VK_SUCCESS) {
        *pSurface = WrapNew(*pSurface);
    }
    return result;
}

VkBool32 DispatchInstance::GetPhysicalDeviceScreenPresentationSupportQNX(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
                                                                         struct _screen_window* window) {
    VkBool32 result =
        instance_dispatch_table.GetPhysicalDeviceScreenPresentationSupportQNX(physicalDevice, queueFamilyIndex, window);

    return result;
}
#endif  // VK_USE_PLATFORM_SCREEN_QNX

void DispatchDevice::CmdSetColorWriteEnableEXT(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
                                               const VkBool32* pColorWriteEnables) {
    device_dispatch_table.CmdSetColorWriteEnableEXT(commandBuffer, attachmentCount, pColorWriteEnables);
}

void DispatchDevice::CmdDrawMultiEXT(VkCommandBuffer commandBuffer, uint32_t drawCount, const VkMultiDrawInfoEXT* pVertexInfo,
                                     uint32_t instanceCount, uint32_t firstInstance, uint32_t stride) {
    device_dispatch_table.CmdDrawMultiEXT(commandBuffer, drawCount, pVertexInfo, instanceCount, firstInstance, stride);
}

void DispatchDevice::CmdDrawMultiIndexedEXT(VkCommandBuffer commandBuffer, uint32_t drawCount,
                                            const VkMultiDrawIndexedInfoEXT* pIndexInfo, uint32_t instanceCount,
                                            uint32_t firstInstance, uint32_t stride, const int32_t* pVertexOffset) {
    device_dispatch_table.CmdDrawMultiIndexedEXT(commandBuffer, drawCount, pIndexInfo, instanceCount, firstInstance, stride,
                                                 pVertexOffset);
}

VkResult DispatchDevice::CreateMicromapEXT(VkDevice device, const VkMicromapCreateInfoEXT* pCreateInfo,
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
    if (result == VK_SUCCESS) {
        *pMicromap = WrapNew(*pMicromap);
    }
    return result;
}

void DispatchDevice::DestroyMicromapEXT(VkDevice device, VkMicromapEXT micromap, const VkAllocationCallbacks* pAllocator) {
    if (!wrap_handles) return device_dispatch_table.DestroyMicromapEXT(device, micromap, pAllocator);
    micromap = Erase(micromap);
    device_dispatch_table.DestroyMicromapEXT(device, micromap, pAllocator);
}

void DispatchDevice::CmdBuildMicromapsEXT(VkCommandBuffer commandBuffer, uint32_t infoCount, const VkMicromapBuildInfoEXT* pInfos) {
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

VkResult DispatchDevice::BuildMicromapsEXT(VkDevice device, VkDeferredOperationKHR deferredOperation, uint32_t infoCount,
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

VkResult DispatchDevice::CopyMicromapEXT(VkDevice device, VkDeferredOperationKHR deferredOperation,
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

VkResult DispatchDevice::CopyMicromapToMemoryEXT(VkDevice device, VkDeferredOperationKHR deferredOperation,
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

VkResult DispatchDevice::CopyMemoryToMicromapEXT(VkDevice device, VkDeferredOperationKHR deferredOperation,
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

VkResult DispatchDevice::WriteMicromapsPropertiesEXT(VkDevice device, uint32_t micromapCount, const VkMicromapEXT* pMicromaps,
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

void DispatchDevice::CmdCopyMicromapEXT(VkCommandBuffer commandBuffer, const VkCopyMicromapInfoEXT* pInfo) {
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

void DispatchDevice::CmdCopyMicromapToMemoryEXT(VkCommandBuffer commandBuffer, const VkCopyMicromapToMemoryInfoEXT* pInfo) {
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

void DispatchDevice::CmdCopyMemoryToMicromapEXT(VkCommandBuffer commandBuffer, const VkCopyMemoryToMicromapInfoEXT* pInfo) {
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

void DispatchDevice::CmdWriteMicromapsPropertiesEXT(VkCommandBuffer commandBuffer, uint32_t micromapCount,
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

void DispatchDevice::GetDeviceMicromapCompatibilityEXT(VkDevice device, const VkMicromapVersionInfoEXT* pVersionInfo,
                                                       VkAccelerationStructureCompatibilityKHR* pCompatibility) {
    device_dispatch_table.GetDeviceMicromapCompatibilityEXT(device, pVersionInfo, pCompatibility);
}

void DispatchDevice::GetMicromapBuildSizesEXT(VkDevice device, VkAccelerationStructureBuildTypeKHR buildType,
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

void DispatchDevice::CmdDrawClusterHUAWEI(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY,
                                          uint32_t groupCountZ) {
    device_dispatch_table.CmdDrawClusterHUAWEI(commandBuffer, groupCountX, groupCountY, groupCountZ);
}

void DispatchDevice::CmdDrawClusterIndirectHUAWEI(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset) {
    if (!wrap_handles) return device_dispatch_table.CmdDrawClusterIndirectHUAWEI(commandBuffer, buffer, offset);
    {
        buffer = Unwrap(buffer);
    }
    device_dispatch_table.CmdDrawClusterIndirectHUAWEI(commandBuffer, buffer, offset);
}

void DispatchDevice::SetDeviceMemoryPriorityEXT(VkDevice device, VkDeviceMemory memory, float priority) {
    if (!wrap_handles) return device_dispatch_table.SetDeviceMemoryPriorityEXT(device, memory, priority);
    {
        memory = Unwrap(memory);
    }
    device_dispatch_table.SetDeviceMemoryPriorityEXT(device, memory, priority);
}

void DispatchDevice::CmdSetDispatchParametersARM(VkCommandBuffer commandBuffer,
                                                 const VkDispatchParametersARM* pDispatchParameters) {
    device_dispatch_table.CmdSetDispatchParametersARM(commandBuffer, pDispatchParameters);
}

void DispatchDevice::GetDescriptorSetLayoutHostMappingInfoVALVE(VkDevice device,
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

void DispatchDevice::GetDescriptorSetHostMappingVALVE(VkDevice device, VkDescriptorSet descriptorSet, void** ppData) {
    if (!wrap_handles) return device_dispatch_table.GetDescriptorSetHostMappingVALVE(device, descriptorSet, ppData);
    {
        descriptorSet = Unwrap(descriptorSet);
    }
    device_dispatch_table.GetDescriptorSetHostMappingVALVE(device, descriptorSet, ppData);
}

void DispatchDevice::CmdCopyMemoryIndirectNV(VkCommandBuffer commandBuffer, VkDeviceAddress copyBufferAddress, uint32_t copyCount,
                                             uint32_t stride) {
    device_dispatch_table.CmdCopyMemoryIndirectNV(commandBuffer, copyBufferAddress, copyCount, stride);
}

void DispatchDevice::CmdCopyMemoryToImageIndirectNV(VkCommandBuffer commandBuffer, VkDeviceAddress copyBufferAddress,
                                                    uint32_t copyCount, uint32_t stride, VkImage dstImage,
                                                    VkImageLayout dstImageLayout,
                                                    const VkImageSubresourceLayers* pImageSubresources) {
    if (!wrap_handles)
        return device_dispatch_table.CmdCopyMemoryToImageIndirectNV(commandBuffer, copyBufferAddress, copyCount, stride, dstImage,
                                                                    dstImageLayout, pImageSubresources);
    {
        dstImage = Unwrap(dstImage);
    }
    device_dispatch_table.CmdCopyMemoryToImageIndirectNV(commandBuffer, copyBufferAddress, copyCount, stride, dstImage,
                                                         dstImageLayout, pImageSubresources);
}

void DispatchDevice::CmdDecompressMemoryNV(VkCommandBuffer commandBuffer, uint32_t decompressRegionCount,
                                           const VkDecompressMemoryRegionNV* pDecompressMemoryRegions) {
    device_dispatch_table.CmdDecompressMemoryNV(commandBuffer, decompressRegionCount, pDecompressMemoryRegions);
}

void DispatchDevice::CmdDecompressMemoryIndirectCountNV(VkCommandBuffer commandBuffer, VkDeviceAddress indirectCommandsAddress,
                                                        VkDeviceAddress indirectCommandsCountAddress, uint32_t stride) {
    device_dispatch_table.CmdDecompressMemoryIndirectCountNV(commandBuffer, indirectCommandsAddress, indirectCommandsCountAddress,
                                                             stride);
}

void DispatchDevice::GetPipelineIndirectMemoryRequirementsNV(VkDevice device, const VkComputePipelineCreateInfo* pCreateInfo,
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

void DispatchDevice::CmdUpdatePipelineIndirectBufferNV(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                       VkPipeline pipeline) {
    if (!wrap_handles) return device_dispatch_table.CmdUpdatePipelineIndirectBufferNV(commandBuffer, pipelineBindPoint, pipeline);
    {
        pipeline = Unwrap(pipeline);
    }
    device_dispatch_table.CmdUpdatePipelineIndirectBufferNV(commandBuffer, pipelineBindPoint, pipeline);
}

VkDeviceAddress DispatchDevice::GetPipelineIndirectDeviceAddressNV(VkDevice device,
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
#ifdef VK_USE_PLATFORM_OHOS

VkResult DispatchDevice::GetNativeBufferPropertiesOHOS(VkDevice device, const struct OH_NativeBuffer* buffer,
                                                       VkNativeBufferPropertiesOHOS* pProperties) {
    VkResult result = device_dispatch_table.GetNativeBufferPropertiesOHOS(device, buffer, pProperties);

    return result;
}

VkResult DispatchDevice::GetMemoryNativeBufferOHOS(VkDevice device, const VkMemoryGetNativeBufferInfoOHOS* pInfo,
                                                   struct OH_NativeBuffer** pBuffer) {
    if (!wrap_handles) return device_dispatch_table.GetMemoryNativeBufferOHOS(device, pInfo, pBuffer);
    vku::safe_VkMemoryGetNativeBufferInfoOHOS var_local_pInfo;
    vku::safe_VkMemoryGetNativeBufferInfoOHOS* local_pInfo = nullptr;
    {
        if (pInfo) {
            local_pInfo = &var_local_pInfo;
            local_pInfo->initialize(pInfo);

            if (pInfo->memory) {
                local_pInfo->memory = Unwrap(pInfo->memory);
            }
        }
    }
    VkResult result =
        device_dispatch_table.GetMemoryNativeBufferOHOS(device, (const VkMemoryGetNativeBufferInfoOHOS*)local_pInfo, pBuffer);

    return result;
}
#endif  // VK_USE_PLATFORM_OHOS

void DispatchDevice::CmdSetDepthClampEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthClampEnable) {
    device_dispatch_table.CmdSetDepthClampEnableEXT(commandBuffer, depthClampEnable);
}

void DispatchDevice::CmdSetPolygonModeEXT(VkCommandBuffer commandBuffer, VkPolygonMode polygonMode) {
    device_dispatch_table.CmdSetPolygonModeEXT(commandBuffer, polygonMode);
}

void DispatchDevice::CmdSetRasterizationSamplesEXT(VkCommandBuffer commandBuffer, VkSampleCountFlagBits rasterizationSamples) {
    device_dispatch_table.CmdSetRasterizationSamplesEXT(commandBuffer, rasterizationSamples);
}

void DispatchDevice::CmdSetSampleMaskEXT(VkCommandBuffer commandBuffer, VkSampleCountFlagBits samples,
                                         const VkSampleMask* pSampleMask) {
    device_dispatch_table.CmdSetSampleMaskEXT(commandBuffer, samples, pSampleMask);
}

void DispatchDevice::CmdSetAlphaToCoverageEnableEXT(VkCommandBuffer commandBuffer, VkBool32 alphaToCoverageEnable) {
    device_dispatch_table.CmdSetAlphaToCoverageEnableEXT(commandBuffer, alphaToCoverageEnable);
}

void DispatchDevice::CmdSetAlphaToOneEnableEXT(VkCommandBuffer commandBuffer, VkBool32 alphaToOneEnable) {
    device_dispatch_table.CmdSetAlphaToOneEnableEXT(commandBuffer, alphaToOneEnable);
}

void DispatchDevice::CmdSetLogicOpEnableEXT(VkCommandBuffer commandBuffer, VkBool32 logicOpEnable) {
    device_dispatch_table.CmdSetLogicOpEnableEXT(commandBuffer, logicOpEnable);
}

void DispatchDevice::CmdSetColorBlendEnableEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment, uint32_t attachmentCount,
                                               const VkBool32* pColorBlendEnables) {
    device_dispatch_table.CmdSetColorBlendEnableEXT(commandBuffer, firstAttachment, attachmentCount, pColorBlendEnables);
}

void DispatchDevice::CmdSetColorBlendEquationEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment, uint32_t attachmentCount,
                                                 const VkColorBlendEquationEXT* pColorBlendEquations) {
    device_dispatch_table.CmdSetColorBlendEquationEXT(commandBuffer, firstAttachment, attachmentCount, pColorBlendEquations);
}

void DispatchDevice::CmdSetColorWriteMaskEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment, uint32_t attachmentCount,
                                             const VkColorComponentFlags* pColorWriteMasks) {
    device_dispatch_table.CmdSetColorWriteMaskEXT(commandBuffer, firstAttachment, attachmentCount, pColorWriteMasks);
}

void DispatchDevice::CmdSetTessellationDomainOriginEXT(VkCommandBuffer commandBuffer, VkTessellationDomainOrigin domainOrigin) {
    device_dispatch_table.CmdSetTessellationDomainOriginEXT(commandBuffer, domainOrigin);
}

void DispatchDevice::CmdSetRasterizationStreamEXT(VkCommandBuffer commandBuffer, uint32_t rasterizationStream) {
    device_dispatch_table.CmdSetRasterizationStreamEXT(commandBuffer, rasterizationStream);
}

void DispatchDevice::CmdSetConservativeRasterizationModeEXT(VkCommandBuffer commandBuffer,
                                                            VkConservativeRasterizationModeEXT conservativeRasterizationMode) {
    device_dispatch_table.CmdSetConservativeRasterizationModeEXT(commandBuffer, conservativeRasterizationMode);
}

void DispatchDevice::CmdSetExtraPrimitiveOverestimationSizeEXT(VkCommandBuffer commandBuffer,
                                                               float extraPrimitiveOverestimationSize) {
    device_dispatch_table.CmdSetExtraPrimitiveOverestimationSizeEXT(commandBuffer, extraPrimitiveOverestimationSize);
}

void DispatchDevice::CmdSetDepthClipEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthClipEnable) {
    device_dispatch_table.CmdSetDepthClipEnableEXT(commandBuffer, depthClipEnable);
}

void DispatchDevice::CmdSetSampleLocationsEnableEXT(VkCommandBuffer commandBuffer, VkBool32 sampleLocationsEnable) {
    device_dispatch_table.CmdSetSampleLocationsEnableEXT(commandBuffer, sampleLocationsEnable);
}

void DispatchDevice::CmdSetColorBlendAdvancedEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment, uint32_t attachmentCount,
                                                 const VkColorBlendAdvancedEXT* pColorBlendAdvanced) {
    device_dispatch_table.CmdSetColorBlendAdvancedEXT(commandBuffer, firstAttachment, attachmentCount, pColorBlendAdvanced);
}

void DispatchDevice::CmdSetProvokingVertexModeEXT(VkCommandBuffer commandBuffer, VkProvokingVertexModeEXT provokingVertexMode) {
    device_dispatch_table.CmdSetProvokingVertexModeEXT(commandBuffer, provokingVertexMode);
}

void DispatchDevice::CmdSetLineRasterizationModeEXT(VkCommandBuffer commandBuffer,
                                                    VkLineRasterizationModeEXT lineRasterizationMode) {
    device_dispatch_table.CmdSetLineRasterizationModeEXT(commandBuffer, lineRasterizationMode);
}

void DispatchDevice::CmdSetLineStippleEnableEXT(VkCommandBuffer commandBuffer, VkBool32 stippledLineEnable) {
    device_dispatch_table.CmdSetLineStippleEnableEXT(commandBuffer, stippledLineEnable);
}

void DispatchDevice::CmdSetDepthClipNegativeOneToOneEXT(VkCommandBuffer commandBuffer, VkBool32 negativeOneToOne) {
    device_dispatch_table.CmdSetDepthClipNegativeOneToOneEXT(commandBuffer, negativeOneToOne);
}

void DispatchDevice::CmdSetViewportWScalingEnableNV(VkCommandBuffer commandBuffer, VkBool32 viewportWScalingEnable) {
    device_dispatch_table.CmdSetViewportWScalingEnableNV(commandBuffer, viewportWScalingEnable);
}

void DispatchDevice::CmdSetViewportSwizzleNV(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount,
                                             const VkViewportSwizzleNV* pViewportSwizzles) {
    device_dispatch_table.CmdSetViewportSwizzleNV(commandBuffer, firstViewport, viewportCount, pViewportSwizzles);
}

void DispatchDevice::CmdSetCoverageToColorEnableNV(VkCommandBuffer commandBuffer, VkBool32 coverageToColorEnable) {
    device_dispatch_table.CmdSetCoverageToColorEnableNV(commandBuffer, coverageToColorEnable);
}

void DispatchDevice::CmdSetCoverageToColorLocationNV(VkCommandBuffer commandBuffer, uint32_t coverageToColorLocation) {
    device_dispatch_table.CmdSetCoverageToColorLocationNV(commandBuffer, coverageToColorLocation);
}

void DispatchDevice::CmdSetCoverageModulationModeNV(VkCommandBuffer commandBuffer,
                                                    VkCoverageModulationModeNV coverageModulationMode) {
    device_dispatch_table.CmdSetCoverageModulationModeNV(commandBuffer, coverageModulationMode);
}

void DispatchDevice::CmdSetCoverageModulationTableEnableNV(VkCommandBuffer commandBuffer, VkBool32 coverageModulationTableEnable) {
    device_dispatch_table.CmdSetCoverageModulationTableEnableNV(commandBuffer, coverageModulationTableEnable);
}

void DispatchDevice::CmdSetCoverageModulationTableNV(VkCommandBuffer commandBuffer, uint32_t coverageModulationTableCount,
                                                     const float* pCoverageModulationTable) {
    device_dispatch_table.CmdSetCoverageModulationTableNV(commandBuffer, coverageModulationTableCount, pCoverageModulationTable);
}

void DispatchDevice::CmdSetShadingRateImageEnableNV(VkCommandBuffer commandBuffer, VkBool32 shadingRateImageEnable) {
    device_dispatch_table.CmdSetShadingRateImageEnableNV(commandBuffer, shadingRateImageEnable);
}

void DispatchDevice::CmdSetRepresentativeFragmentTestEnableNV(VkCommandBuffer commandBuffer,
                                                              VkBool32 representativeFragmentTestEnable) {
    device_dispatch_table.CmdSetRepresentativeFragmentTestEnableNV(commandBuffer, representativeFragmentTestEnable);
}

void DispatchDevice::CmdSetCoverageReductionModeNV(VkCommandBuffer commandBuffer, VkCoverageReductionModeNV coverageReductionMode) {
    device_dispatch_table.CmdSetCoverageReductionModeNV(commandBuffer, coverageReductionMode);
}

VkResult DispatchDevice::CreateTensorARM(VkDevice device, const VkTensorCreateInfoARM* pCreateInfo,
                                         const VkAllocationCallbacks* pAllocator, VkTensorARM* pTensor) {
    if (!wrap_handles) return device_dispatch_table.CreateTensorARM(device, pCreateInfo, pAllocator, pTensor);

    VkResult result = device_dispatch_table.CreateTensorARM(device, pCreateInfo, pAllocator, pTensor);
    if (result == VK_SUCCESS) {
        *pTensor = WrapNew(*pTensor);
    }
    return result;
}

void DispatchDevice::DestroyTensorARM(VkDevice device, VkTensorARM tensor, const VkAllocationCallbacks* pAllocator) {
    if (!wrap_handles) return device_dispatch_table.DestroyTensorARM(device, tensor, pAllocator);
    tensor = Erase(tensor);
    device_dispatch_table.DestroyTensorARM(device, tensor, pAllocator);
}

VkResult DispatchDevice::CreateTensorViewARM(VkDevice device, const VkTensorViewCreateInfoARM* pCreateInfo,
                                             const VkAllocationCallbacks* pAllocator, VkTensorViewARM* pView) {
    if (!wrap_handles) return device_dispatch_table.CreateTensorViewARM(device, pCreateInfo, pAllocator, pView);
    vku::safe_VkTensorViewCreateInfoARM var_local_pCreateInfo;
    vku::safe_VkTensorViewCreateInfoARM* local_pCreateInfo = nullptr;
    {
        if (pCreateInfo) {
            local_pCreateInfo = &var_local_pCreateInfo;
            local_pCreateInfo->initialize(pCreateInfo);

            if (pCreateInfo->tensor) {
                local_pCreateInfo->tensor = Unwrap(pCreateInfo->tensor);
            }
        }
    }
    VkResult result =
        device_dispatch_table.CreateTensorViewARM(device, (const VkTensorViewCreateInfoARM*)local_pCreateInfo, pAllocator, pView);
    if (result == VK_SUCCESS) {
        *pView = WrapNew(*pView);
    }
    return result;
}

void DispatchDevice::DestroyTensorViewARM(VkDevice device, VkTensorViewARM tensorView, const VkAllocationCallbacks* pAllocator) {
    if (!wrap_handles) return device_dispatch_table.DestroyTensorViewARM(device, tensorView, pAllocator);
    tensorView = Erase(tensorView);
    device_dispatch_table.DestroyTensorViewARM(device, tensorView, pAllocator);
}

void DispatchDevice::GetTensorMemoryRequirementsARM(VkDevice device, const VkTensorMemoryRequirementsInfoARM* pInfo,
                                                    VkMemoryRequirements2* pMemoryRequirements) {
    if (!wrap_handles) return device_dispatch_table.GetTensorMemoryRequirementsARM(device, pInfo, pMemoryRequirements);
    vku::safe_VkTensorMemoryRequirementsInfoARM var_local_pInfo;
    vku::safe_VkTensorMemoryRequirementsInfoARM* local_pInfo = nullptr;
    {
        if (pInfo) {
            local_pInfo = &var_local_pInfo;
            local_pInfo->initialize(pInfo);

            if (pInfo->tensor) {
                local_pInfo->tensor = Unwrap(pInfo->tensor);
            }
        }
    }
    device_dispatch_table.GetTensorMemoryRequirementsARM(device, (const VkTensorMemoryRequirementsInfoARM*)local_pInfo,
                                                         pMemoryRequirements);
}

VkResult DispatchDevice::BindTensorMemoryARM(VkDevice device, uint32_t bindInfoCount, const VkBindTensorMemoryInfoARM* pBindInfos) {
    if (!wrap_handles) return device_dispatch_table.BindTensorMemoryARM(device, bindInfoCount, pBindInfos);
    small_vector<vku::safe_VkBindTensorMemoryInfoARM, DISPATCH_MAX_STACK_ALLOCATIONS> var_local_pBindInfos;
    vku::safe_VkBindTensorMemoryInfoARM* local_pBindInfos = nullptr;
    {
        if (pBindInfos) {
            var_local_pBindInfos.resize(bindInfoCount);
            local_pBindInfos = var_local_pBindInfos.data();
            for (uint32_t index0 = 0; index0 < bindInfoCount; ++index0) {
                local_pBindInfos[index0].initialize(&pBindInfos[index0]);

                if (pBindInfos[index0].tensor) {
                    local_pBindInfos[index0].tensor = Unwrap(pBindInfos[index0].tensor);
                }
                if (pBindInfos[index0].memory) {
                    local_pBindInfos[index0].memory = Unwrap(pBindInfos[index0].memory);
                }
            }
        }
    }
    VkResult result =
        device_dispatch_table.BindTensorMemoryARM(device, bindInfoCount, (const VkBindTensorMemoryInfoARM*)local_pBindInfos);

    return result;
}

void DispatchDevice::GetDeviceTensorMemoryRequirementsARM(VkDevice device, const VkDeviceTensorMemoryRequirementsARM* pInfo,
                                                          VkMemoryRequirements2* pMemoryRequirements) {
    device_dispatch_table.GetDeviceTensorMemoryRequirementsARM(device, pInfo, pMemoryRequirements);
}

void DispatchDevice::CmdCopyTensorARM(VkCommandBuffer commandBuffer, const VkCopyTensorInfoARM* pCopyTensorInfo) {
    if (!wrap_handles) return device_dispatch_table.CmdCopyTensorARM(commandBuffer, pCopyTensorInfo);
    vku::safe_VkCopyTensorInfoARM var_local_pCopyTensorInfo;
    vku::safe_VkCopyTensorInfoARM* local_pCopyTensorInfo = nullptr;
    {
        if (pCopyTensorInfo) {
            local_pCopyTensorInfo = &var_local_pCopyTensorInfo;
            local_pCopyTensorInfo->initialize(pCopyTensorInfo);

            if (pCopyTensorInfo->srcTensor) {
                local_pCopyTensorInfo->srcTensor = Unwrap(pCopyTensorInfo->srcTensor);
            }
            if (pCopyTensorInfo->dstTensor) {
                local_pCopyTensorInfo->dstTensor = Unwrap(pCopyTensorInfo->dstTensor);
            }
        }
    }
    device_dispatch_table.CmdCopyTensorARM(commandBuffer, (const VkCopyTensorInfoARM*)local_pCopyTensorInfo);
}

void DispatchInstance::GetPhysicalDeviceExternalTensorPropertiesARM(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalTensorInfoARM* pExternalTensorInfo,
    VkExternalTensorPropertiesARM* pExternalTensorProperties) {
    instance_dispatch_table.GetPhysicalDeviceExternalTensorPropertiesARM(physicalDevice, pExternalTensorInfo,
                                                                         pExternalTensorProperties);
}

VkResult DispatchDevice::GetTensorOpaqueCaptureDescriptorDataARM(VkDevice device, const VkTensorCaptureDescriptorDataInfoARM* pInfo,
                                                                 void* pData) {
    if (!wrap_handles) return device_dispatch_table.GetTensorOpaqueCaptureDescriptorDataARM(device, pInfo, pData);
    vku::safe_VkTensorCaptureDescriptorDataInfoARM var_local_pInfo;
    vku::safe_VkTensorCaptureDescriptorDataInfoARM* local_pInfo = nullptr;
    {
        if (pInfo) {
            local_pInfo = &var_local_pInfo;
            local_pInfo->initialize(pInfo);

            if (pInfo->tensor) {
                local_pInfo->tensor = Unwrap(pInfo->tensor);
            }
        }
    }
    VkResult result = device_dispatch_table.GetTensorOpaqueCaptureDescriptorDataARM(
        device, (const VkTensorCaptureDescriptorDataInfoARM*)local_pInfo, pData);

    return result;
}

VkResult DispatchDevice::GetTensorViewOpaqueCaptureDescriptorDataARM(VkDevice device,
                                                                     const VkTensorViewCaptureDescriptorDataInfoARM* pInfo,
                                                                     void* pData) {
    if (!wrap_handles) return device_dispatch_table.GetTensorViewOpaqueCaptureDescriptorDataARM(device, pInfo, pData);
    vku::safe_VkTensorViewCaptureDescriptorDataInfoARM var_local_pInfo;
    vku::safe_VkTensorViewCaptureDescriptorDataInfoARM* local_pInfo = nullptr;
    {
        if (pInfo) {
            local_pInfo = &var_local_pInfo;
            local_pInfo->initialize(pInfo);

            if (pInfo->tensorView) {
                local_pInfo->tensorView = Unwrap(pInfo->tensorView);
            }
        }
    }
    VkResult result = device_dispatch_table.GetTensorViewOpaqueCaptureDescriptorDataARM(
        device, (const VkTensorViewCaptureDescriptorDataInfoARM*)local_pInfo, pData);

    return result;
}

void DispatchDevice::GetShaderModuleIdentifierEXT(VkDevice device, VkShaderModule shaderModule,
                                                  VkShaderModuleIdentifierEXT* pIdentifier) {
    if (!wrap_handles) return device_dispatch_table.GetShaderModuleIdentifierEXT(device, shaderModule, pIdentifier);
    {
        shaderModule = Unwrap(shaderModule);
    }
    device_dispatch_table.GetShaderModuleIdentifierEXT(device, shaderModule, pIdentifier);
}

void DispatchDevice::GetShaderModuleCreateInfoIdentifierEXT(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo,
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

VkResult DispatchInstance::GetPhysicalDeviceOpticalFlowImageFormatsNV(
    VkPhysicalDevice physicalDevice, const VkOpticalFlowImageFormatInfoNV* pOpticalFlowImageFormatInfo, uint32_t* pFormatCount,
    VkOpticalFlowImageFormatPropertiesNV* pImageFormatProperties) {
    VkResult result = instance_dispatch_table.GetPhysicalDeviceOpticalFlowImageFormatsNV(
        physicalDevice, pOpticalFlowImageFormatInfo, pFormatCount, pImageFormatProperties);

    return result;
}

VkResult DispatchDevice::CreateOpticalFlowSessionNV(VkDevice device, const VkOpticalFlowSessionCreateInfoNV* pCreateInfo,
                                                    const VkAllocationCallbacks* pAllocator, VkOpticalFlowSessionNV* pSession) {
    if (!wrap_handles) return device_dispatch_table.CreateOpticalFlowSessionNV(device, pCreateInfo, pAllocator, pSession);

    VkResult result = device_dispatch_table.CreateOpticalFlowSessionNV(device, pCreateInfo, pAllocator, pSession);
    if (result == VK_SUCCESS) {
        *pSession = WrapNew(*pSession);
    }
    return result;
}

void DispatchDevice::DestroyOpticalFlowSessionNV(VkDevice device, VkOpticalFlowSessionNV session,
                                                 const VkAllocationCallbacks* pAllocator) {
    if (!wrap_handles) return device_dispatch_table.DestroyOpticalFlowSessionNV(device, session, pAllocator);
    session = Erase(session);
    device_dispatch_table.DestroyOpticalFlowSessionNV(device, session, pAllocator);
}

VkResult DispatchDevice::BindOpticalFlowSessionImageNV(VkDevice device, VkOpticalFlowSessionNV session,
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

void DispatchDevice::CmdOpticalFlowExecuteNV(VkCommandBuffer commandBuffer, VkOpticalFlowSessionNV session,
                                             const VkOpticalFlowExecuteInfoNV* pExecuteInfo) {
    if (!wrap_handles) return device_dispatch_table.CmdOpticalFlowExecuteNV(commandBuffer, session, pExecuteInfo);
    {
        session = Unwrap(session);
    }
    device_dispatch_table.CmdOpticalFlowExecuteNV(commandBuffer, session, pExecuteInfo);
}

void DispatchDevice::AntiLagUpdateAMD(VkDevice device, const VkAntiLagDataAMD* pData) {
    device_dispatch_table.AntiLagUpdateAMD(device, pData);
}

void DispatchDevice::DestroyShaderEXT(VkDevice device, VkShaderEXT shader, const VkAllocationCallbacks* pAllocator) {
    if (!wrap_handles) return device_dispatch_table.DestroyShaderEXT(device, shader, pAllocator);
    shader = Erase(shader);
    device_dispatch_table.DestroyShaderEXT(device, shader, pAllocator);
}

VkResult DispatchDevice::GetShaderBinaryDataEXT(VkDevice device, VkShaderEXT shader, size_t* pDataSize, void* pData) {
    if (!wrap_handles) return device_dispatch_table.GetShaderBinaryDataEXT(device, shader, pDataSize, pData);
    {
        shader = Unwrap(shader);
    }
    VkResult result = device_dispatch_table.GetShaderBinaryDataEXT(device, shader, pDataSize, pData);

    return result;
}

void DispatchDevice::CmdBindShadersEXT(VkCommandBuffer commandBuffer, uint32_t stageCount, const VkShaderStageFlagBits* pStages,
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

void DispatchDevice::CmdSetDepthClampRangeEXT(VkCommandBuffer commandBuffer, VkDepthClampModeEXT depthClampMode,
                                              const VkDepthClampRangeEXT* pDepthClampRange) {
    device_dispatch_table.CmdSetDepthClampRangeEXT(commandBuffer, depthClampMode, pDepthClampRange);
}

VkResult DispatchDevice::GetFramebufferTilePropertiesQCOM(VkDevice device, VkFramebuffer framebuffer, uint32_t* pPropertiesCount,
                                                          VkTilePropertiesQCOM* pProperties) {
    if (!wrap_handles)
        return device_dispatch_table.GetFramebufferTilePropertiesQCOM(device, framebuffer, pPropertiesCount, pProperties);
    {
        framebuffer = Unwrap(framebuffer);
    }
    VkResult result = device_dispatch_table.GetFramebufferTilePropertiesQCOM(device, framebuffer, pPropertiesCount, pProperties);

    return result;
}

VkResult DispatchDevice::GetDynamicRenderingTilePropertiesQCOM(VkDevice device, const VkRenderingInfo* pRenderingInfo,
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

VkResult DispatchInstance::GetPhysicalDeviceCooperativeVectorPropertiesNV(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount,
                                                                          VkCooperativeVectorPropertiesNV* pProperties) {
    VkResult result =
        instance_dispatch_table.GetPhysicalDeviceCooperativeVectorPropertiesNV(physicalDevice, pPropertyCount, pProperties);

    return result;
}

VkResult DispatchDevice::ConvertCooperativeVectorMatrixNV(VkDevice device, const VkConvertCooperativeVectorMatrixInfoNV* pInfo) {
    VkResult result = device_dispatch_table.ConvertCooperativeVectorMatrixNV(device, pInfo);

    return result;
}

void DispatchDevice::CmdConvertCooperativeVectorMatrixNV(VkCommandBuffer commandBuffer, uint32_t infoCount,
                                                         const VkConvertCooperativeVectorMatrixInfoNV* pInfos) {
    device_dispatch_table.CmdConvertCooperativeVectorMatrixNV(commandBuffer, infoCount, pInfos);
}

VkResult DispatchDevice::SetLatencySleepModeNV(VkDevice device, VkSwapchainKHR swapchain,
                                               const VkLatencySleepModeInfoNV* pSleepModeInfo) {
    if (!wrap_handles) return device_dispatch_table.SetLatencySleepModeNV(device, swapchain, pSleepModeInfo);
    {
        swapchain = Unwrap(swapchain);
    }
    VkResult result = device_dispatch_table.SetLatencySleepModeNV(device, swapchain, pSleepModeInfo);

    return result;
}

VkResult DispatchDevice::LatencySleepNV(VkDevice device, VkSwapchainKHR swapchain, const VkLatencySleepInfoNV* pSleepInfo) {
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

void DispatchDevice::SetLatencyMarkerNV(VkDevice device, VkSwapchainKHR swapchain,
                                        const VkSetLatencyMarkerInfoNV* pLatencyMarkerInfo) {
    if (!wrap_handles) return device_dispatch_table.SetLatencyMarkerNV(device, swapchain, pLatencyMarkerInfo);
    {
        swapchain = Unwrap(swapchain);
    }
    device_dispatch_table.SetLatencyMarkerNV(device, swapchain, pLatencyMarkerInfo);
}

void DispatchDevice::GetLatencyTimingsNV(VkDevice device, VkSwapchainKHR swapchain, VkGetLatencyMarkerInfoNV* pLatencyMarkerInfo) {
    if (!wrap_handles) return device_dispatch_table.GetLatencyTimingsNV(device, swapchain, pLatencyMarkerInfo);
    {
        swapchain = Unwrap(swapchain);
    }
    device_dispatch_table.GetLatencyTimingsNV(device, swapchain, pLatencyMarkerInfo);
}

void DispatchDevice::QueueNotifyOutOfBandNV(VkQueue queue, const VkOutOfBandQueueTypeInfoNV* pQueueTypeInfo) {
    device_dispatch_table.QueueNotifyOutOfBandNV(queue, pQueueTypeInfo);
}

VkResult DispatchDevice::CreateDataGraphPipelineSessionARM(VkDevice device,
                                                           const VkDataGraphPipelineSessionCreateInfoARM* pCreateInfo,
                                                           const VkAllocationCallbacks* pAllocator,
                                                           VkDataGraphPipelineSessionARM* pSession) {
    if (!wrap_handles) return device_dispatch_table.CreateDataGraphPipelineSessionARM(device, pCreateInfo, pAllocator, pSession);
    vku::safe_VkDataGraphPipelineSessionCreateInfoARM var_local_pCreateInfo;
    vku::safe_VkDataGraphPipelineSessionCreateInfoARM* local_pCreateInfo = nullptr;
    {
        if (pCreateInfo) {
            local_pCreateInfo = &var_local_pCreateInfo;
            local_pCreateInfo->initialize(pCreateInfo);

            if (pCreateInfo->dataGraphPipeline) {
                local_pCreateInfo->dataGraphPipeline = Unwrap(pCreateInfo->dataGraphPipeline);
            }
        }
    }
    VkResult result = device_dispatch_table.CreateDataGraphPipelineSessionARM(
        device, (const VkDataGraphPipelineSessionCreateInfoARM*)local_pCreateInfo, pAllocator, pSession);
    if (result == VK_SUCCESS) {
        *pSession = WrapNew(*pSession);
    }
    return result;
}

VkResult DispatchDevice::GetDataGraphPipelineSessionBindPointRequirementsARM(
    VkDevice device, const VkDataGraphPipelineSessionBindPointRequirementsInfoARM* pInfo, uint32_t* pBindPointRequirementCount,
    VkDataGraphPipelineSessionBindPointRequirementARM* pBindPointRequirements) {
    if (!wrap_handles)
        return device_dispatch_table.GetDataGraphPipelineSessionBindPointRequirementsARM(device, pInfo, pBindPointRequirementCount,
                                                                                         pBindPointRequirements);
    vku::safe_VkDataGraphPipelineSessionBindPointRequirementsInfoARM var_local_pInfo;
    vku::safe_VkDataGraphPipelineSessionBindPointRequirementsInfoARM* local_pInfo = nullptr;
    {
        if (pInfo) {
            local_pInfo = &var_local_pInfo;
            local_pInfo->initialize(pInfo);

            if (pInfo->session) {
                local_pInfo->session = Unwrap(pInfo->session);
            }
        }
    }
    VkResult result = device_dispatch_table.GetDataGraphPipelineSessionBindPointRequirementsARM(
        device, (const VkDataGraphPipelineSessionBindPointRequirementsInfoARM*)local_pInfo, pBindPointRequirementCount,
        pBindPointRequirements);

    return result;
}

void DispatchDevice::GetDataGraphPipelineSessionMemoryRequirementsARM(
    VkDevice device, const VkDataGraphPipelineSessionMemoryRequirementsInfoARM* pInfo, VkMemoryRequirements2* pMemoryRequirements) {
    if (!wrap_handles)
        return device_dispatch_table.GetDataGraphPipelineSessionMemoryRequirementsARM(device, pInfo, pMemoryRequirements);
    vku::safe_VkDataGraphPipelineSessionMemoryRequirementsInfoARM var_local_pInfo;
    vku::safe_VkDataGraphPipelineSessionMemoryRequirementsInfoARM* local_pInfo = nullptr;
    {
        if (pInfo) {
            local_pInfo = &var_local_pInfo;
            local_pInfo->initialize(pInfo);

            if (pInfo->session) {
                local_pInfo->session = Unwrap(pInfo->session);
            }
        }
    }
    device_dispatch_table.GetDataGraphPipelineSessionMemoryRequirementsARM(
        device, (const VkDataGraphPipelineSessionMemoryRequirementsInfoARM*)local_pInfo, pMemoryRequirements);
}

VkResult DispatchDevice::BindDataGraphPipelineSessionMemoryARM(VkDevice device, uint32_t bindInfoCount,
                                                               const VkBindDataGraphPipelineSessionMemoryInfoARM* pBindInfos) {
    if (!wrap_handles) return device_dispatch_table.BindDataGraphPipelineSessionMemoryARM(device, bindInfoCount, pBindInfos);
    small_vector<vku::safe_VkBindDataGraphPipelineSessionMemoryInfoARM, DISPATCH_MAX_STACK_ALLOCATIONS> var_local_pBindInfos;
    vku::safe_VkBindDataGraphPipelineSessionMemoryInfoARM* local_pBindInfos = nullptr;
    {
        if (pBindInfos) {
            var_local_pBindInfos.resize(bindInfoCount);
            local_pBindInfos = var_local_pBindInfos.data();
            for (uint32_t index0 = 0; index0 < bindInfoCount; ++index0) {
                local_pBindInfos[index0].initialize(&pBindInfos[index0]);

                if (pBindInfos[index0].session) {
                    local_pBindInfos[index0].session = Unwrap(pBindInfos[index0].session);
                }
                if (pBindInfos[index0].memory) {
                    local_pBindInfos[index0].memory = Unwrap(pBindInfos[index0].memory);
                }
            }
        }
    }
    VkResult result = device_dispatch_table.BindDataGraphPipelineSessionMemoryARM(
        device, bindInfoCount, (const VkBindDataGraphPipelineSessionMemoryInfoARM*)local_pBindInfos);

    return result;
}

void DispatchDevice::DestroyDataGraphPipelineSessionARM(VkDevice device, VkDataGraphPipelineSessionARM session,
                                                        const VkAllocationCallbacks* pAllocator) {
    if (!wrap_handles) return device_dispatch_table.DestroyDataGraphPipelineSessionARM(device, session, pAllocator);
    session = Erase(session);
    device_dispatch_table.DestroyDataGraphPipelineSessionARM(device, session, pAllocator);
}

void DispatchDevice::CmdDispatchDataGraphARM(VkCommandBuffer commandBuffer, VkDataGraphPipelineSessionARM session,
                                             const VkDataGraphPipelineDispatchInfoARM* pInfo) {
    if (!wrap_handles) return device_dispatch_table.CmdDispatchDataGraphARM(commandBuffer, session, pInfo);
    {
        session = Unwrap(session);
    }
    device_dispatch_table.CmdDispatchDataGraphARM(commandBuffer, session, pInfo);
}

VkResult DispatchDevice::GetDataGraphPipelineAvailablePropertiesARM(VkDevice device,
                                                                    const VkDataGraphPipelineInfoARM* pPipelineInfo,
                                                                    uint32_t* pPropertiesCount,
                                                                    VkDataGraphPipelinePropertyARM* pProperties) {
    if (!wrap_handles)
        return device_dispatch_table.GetDataGraphPipelineAvailablePropertiesARM(device, pPipelineInfo, pPropertiesCount,
                                                                                pProperties);
    vku::safe_VkDataGraphPipelineInfoARM var_local_pPipelineInfo;
    vku::safe_VkDataGraphPipelineInfoARM* local_pPipelineInfo = nullptr;
    {
        if (pPipelineInfo) {
            local_pPipelineInfo = &var_local_pPipelineInfo;
            local_pPipelineInfo->initialize(pPipelineInfo);

            if (pPipelineInfo->dataGraphPipeline) {
                local_pPipelineInfo->dataGraphPipeline = Unwrap(pPipelineInfo->dataGraphPipeline);
            }
        }
    }
    VkResult result = device_dispatch_table.GetDataGraphPipelineAvailablePropertiesARM(
        device, (const VkDataGraphPipelineInfoARM*)local_pPipelineInfo, pPropertiesCount, pProperties);

    return result;
}

VkResult DispatchDevice::GetDataGraphPipelinePropertiesARM(VkDevice device, const VkDataGraphPipelineInfoARM* pPipelineInfo,
                                                           uint32_t propertiesCount,
                                                           VkDataGraphPipelinePropertyQueryResultARM* pProperties) {
    if (!wrap_handles)
        return device_dispatch_table.GetDataGraphPipelinePropertiesARM(device, pPipelineInfo, propertiesCount, pProperties);
    vku::safe_VkDataGraphPipelineInfoARM var_local_pPipelineInfo;
    vku::safe_VkDataGraphPipelineInfoARM* local_pPipelineInfo = nullptr;
    {
        if (pPipelineInfo) {
            local_pPipelineInfo = &var_local_pPipelineInfo;
            local_pPipelineInfo->initialize(pPipelineInfo);

            if (pPipelineInfo->dataGraphPipeline) {
                local_pPipelineInfo->dataGraphPipeline = Unwrap(pPipelineInfo->dataGraphPipeline);
            }
        }
    }
    VkResult result = device_dispatch_table.GetDataGraphPipelinePropertiesARM(
        device, (const VkDataGraphPipelineInfoARM*)local_pPipelineInfo, propertiesCount, pProperties);

    return result;
}

VkResult DispatchInstance::GetPhysicalDeviceQueueFamilyDataGraphPropertiesARM(
    VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, uint32_t* pQueueFamilyDataGraphPropertyCount,
    VkQueueFamilyDataGraphPropertiesARM* pQueueFamilyDataGraphProperties) {
    VkResult result = instance_dispatch_table.GetPhysicalDeviceQueueFamilyDataGraphPropertiesARM(
        physicalDevice, queueFamilyIndex, pQueueFamilyDataGraphPropertyCount, pQueueFamilyDataGraphProperties);

    return result;
}

void DispatchInstance::GetPhysicalDeviceQueueFamilyDataGraphProcessingEnginePropertiesARM(
    VkPhysicalDevice physicalDevice,
    const VkPhysicalDeviceQueueFamilyDataGraphProcessingEngineInfoARM* pQueueFamilyDataGraphProcessingEngineInfo,
    VkQueueFamilyDataGraphProcessingEnginePropertiesARM* pQueueFamilyDataGraphProcessingEngineProperties) {
    instance_dispatch_table.GetPhysicalDeviceQueueFamilyDataGraphProcessingEnginePropertiesARM(
        physicalDevice, pQueueFamilyDataGraphProcessingEngineInfo, pQueueFamilyDataGraphProcessingEngineProperties);
}

VkResult DispatchInstance::GetPhysicalDeviceQueueFamilyDataGraphEngineOperationPropertiesARM(
    VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
    const VkQueueFamilyDataGraphPropertiesARM* pQueueFamilyDataGraphProperties, VkBaseOutStructure* pProperties) {
    VkResult result = instance_dispatch_table.GetPhysicalDeviceQueueFamilyDataGraphEngineOperationPropertiesARM(
        physicalDevice, queueFamilyIndex, pQueueFamilyDataGraphProperties, pProperties);

    return result;
}

void DispatchDevice::CmdSetAttachmentFeedbackLoopEnableEXT(VkCommandBuffer commandBuffer, VkImageAspectFlags aspectMask) {
    device_dispatch_table.CmdSetAttachmentFeedbackLoopEnableEXT(commandBuffer, aspectMask);
}
#ifdef VK_USE_PLATFORM_SCREEN_QNX

VkResult DispatchDevice::GetScreenBufferPropertiesQNX(VkDevice device, const struct _screen_buffer* buffer,
                                                      VkScreenBufferPropertiesQNX* pProperties) {
    VkResult result = device_dispatch_table.GetScreenBufferPropertiesQNX(device, buffer, pProperties);

    return result;
}
#endif  // VK_USE_PLATFORM_SCREEN_QNX

void DispatchDevice::CmdBindTileMemoryQCOM(VkCommandBuffer commandBuffer, const VkTileMemoryBindInfoQCOM* pTileMemoryBindInfo) {
    if (!wrap_handles) return device_dispatch_table.CmdBindTileMemoryQCOM(commandBuffer, pTileMemoryBindInfo);
    vku::safe_VkTileMemoryBindInfoQCOM var_local_pTileMemoryBindInfo;
    vku::safe_VkTileMemoryBindInfoQCOM* local_pTileMemoryBindInfo = nullptr;
    {
        if (pTileMemoryBindInfo) {
            local_pTileMemoryBindInfo = &var_local_pTileMemoryBindInfo;
            local_pTileMemoryBindInfo->initialize(pTileMemoryBindInfo);

            if (pTileMemoryBindInfo->memory) {
                local_pTileMemoryBindInfo->memory = Unwrap(pTileMemoryBindInfo->memory);
            }
        }
    }
    device_dispatch_table.CmdBindTileMemoryQCOM(commandBuffer, (const VkTileMemoryBindInfoQCOM*)local_pTileMemoryBindInfo);
}

void DispatchDevice::CmdDecompressMemoryEXT(VkCommandBuffer commandBuffer,
                                            const VkDecompressMemoryInfoEXT* pDecompressMemoryInfoEXT) {
    device_dispatch_table.CmdDecompressMemoryEXT(commandBuffer, pDecompressMemoryInfoEXT);
}

void DispatchDevice::CmdDecompressMemoryIndirectCountEXT(VkCommandBuffer commandBuffer,
                                                         VkMemoryDecompressionMethodFlagsEXT decompressionMethod,
                                                         VkDeviceAddress indirectCommandsAddress,
                                                         VkDeviceAddress indirectCommandsCountAddress,
                                                         uint32_t maxDecompressionCount, uint32_t stride) {
    device_dispatch_table.CmdDecompressMemoryIndirectCountEXT(commandBuffer, decompressionMethod, indirectCommandsAddress,
                                                              indirectCommandsCountAddress, maxDecompressionCount, stride);
}

VkResult DispatchDevice::CreateExternalComputeQueueNV(VkDevice device, const VkExternalComputeQueueCreateInfoNV* pCreateInfo,
                                                      const VkAllocationCallbacks* pAllocator,
                                                      VkExternalComputeQueueNV* pExternalQueue) {
    VkResult result = device_dispatch_table.CreateExternalComputeQueueNV(device, pCreateInfo, pAllocator, pExternalQueue);

    return result;
}

void DispatchDevice::DestroyExternalComputeQueueNV(VkDevice device, VkExternalComputeQueueNV externalQueue,
                                                   const VkAllocationCallbacks* pAllocator) {
    device_dispatch_table.DestroyExternalComputeQueueNV(device, externalQueue, pAllocator);
}

void DispatchDevice::GetExternalComputeQueueDataNV(VkExternalComputeQueueNV externalQueue,
                                                   VkExternalComputeQueueDataParamsNV* params, void* pData) {
    device_dispatch_table.GetExternalComputeQueueDataNV(externalQueue, params, pData);
}

void DispatchDevice::GetClusterAccelerationStructureBuildSizesNV(VkDevice device,
                                                                 const VkClusterAccelerationStructureInputInfoNV* pInfo,
                                                                 VkAccelerationStructureBuildSizesInfoKHR* pSizeInfo) {
    device_dispatch_table.GetClusterAccelerationStructureBuildSizesNV(device, pInfo, pSizeInfo);
}

void DispatchDevice::CmdBuildClusterAccelerationStructureIndirectNV(
    VkCommandBuffer commandBuffer, const VkClusterAccelerationStructureCommandsInfoNV* pCommandInfos) {
    device_dispatch_table.CmdBuildClusterAccelerationStructureIndirectNV(commandBuffer, pCommandInfos);
}

void DispatchDevice::GetPartitionedAccelerationStructuresBuildSizesNV(
    VkDevice device, const VkPartitionedAccelerationStructureInstancesInputNV* pInfo,
    VkAccelerationStructureBuildSizesInfoKHR* pSizeInfo) {
    device_dispatch_table.GetPartitionedAccelerationStructuresBuildSizesNV(device, pInfo, pSizeInfo);
}

void DispatchDevice::CmdBuildPartitionedAccelerationStructuresNV(VkCommandBuffer commandBuffer,
                                                                 const VkBuildPartitionedAccelerationStructureInfoNV* pBuildInfo) {
    device_dispatch_table.CmdBuildPartitionedAccelerationStructuresNV(commandBuffer, pBuildInfo);
}

void DispatchDevice::GetGeneratedCommandsMemoryRequirementsEXT(VkDevice device,
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

void DispatchDevice::CmdPreprocessGeneratedCommandsEXT(VkCommandBuffer commandBuffer,
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

void DispatchDevice::CmdExecuteGeneratedCommandsEXT(VkCommandBuffer commandBuffer, VkBool32 isPreprocessed,
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

VkResult DispatchDevice::CreateIndirectCommandsLayoutEXT(VkDevice device, const VkIndirectCommandsLayoutCreateInfoEXT* pCreateInfo,
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
    if (result == VK_SUCCESS) {
        *pIndirectCommandsLayout = WrapNew(*pIndirectCommandsLayout);
    }
    return result;
}

void DispatchDevice::DestroyIndirectCommandsLayoutEXT(VkDevice device, VkIndirectCommandsLayoutEXT indirectCommandsLayout,
                                                      const VkAllocationCallbacks* pAllocator) {
    if (!wrap_handles) return device_dispatch_table.DestroyIndirectCommandsLayoutEXT(device, indirectCommandsLayout, pAllocator);
    indirectCommandsLayout = Erase(indirectCommandsLayout);
    device_dispatch_table.DestroyIndirectCommandsLayoutEXT(device, indirectCommandsLayout, pAllocator);
}

void DispatchDevice::DestroyIndirectExecutionSetEXT(VkDevice device, VkIndirectExecutionSetEXT indirectExecutionSet,
                                                    const VkAllocationCallbacks* pAllocator) {
    if (!wrap_handles) return device_dispatch_table.DestroyIndirectExecutionSetEXT(device, indirectExecutionSet, pAllocator);
    indirectExecutionSet = Erase(indirectExecutionSet);
    device_dispatch_table.DestroyIndirectExecutionSetEXT(device, indirectExecutionSet, pAllocator);
}

void DispatchDevice::UpdateIndirectExecutionSetPipelineEXT(VkDevice device, VkIndirectExecutionSetEXT indirectExecutionSet,
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

void DispatchDevice::UpdateIndirectExecutionSetShaderEXT(VkDevice device, VkIndirectExecutionSetEXT indirectExecutionSet,
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
#ifdef VK_USE_PLATFORM_OHOS

VkResult DispatchInstance::CreateSurfaceOHOS(VkInstance instance, const VkSurfaceCreateInfoOHOS* pCreateInfo,
                                             const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    if (!wrap_handles) return instance_dispatch_table.CreateSurfaceOHOS(instance, pCreateInfo, pAllocator, pSurface);

    VkResult result = instance_dispatch_table.CreateSurfaceOHOS(instance, pCreateInfo, pAllocator, pSurface);
    if (result == VK_SUCCESS) {
        *pSurface = WrapNew(*pSurface);
    }
    return result;
}
#endif  // VK_USE_PLATFORM_OHOS

VkResult DispatchInstance::GetPhysicalDeviceCooperativeMatrixFlexibleDimensionsPropertiesNV(
    VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount, VkCooperativeMatrixFlexibleDimensionsPropertiesNV* pProperties) {
    VkResult result = instance_dispatch_table.GetPhysicalDeviceCooperativeMatrixFlexibleDimensionsPropertiesNV(
        physicalDevice, pPropertyCount, pProperties);

    return result;
}
#ifdef VK_USE_PLATFORM_METAL_EXT

VkResult DispatchDevice::GetMemoryMetalHandleEXT(VkDevice device, const VkMemoryGetMetalHandleInfoEXT* pGetMetalHandleInfo,
                                                 void** pHandle) {
    if (!wrap_handles) return device_dispatch_table.GetMemoryMetalHandleEXT(device, pGetMetalHandleInfo, pHandle);
    vku::safe_VkMemoryGetMetalHandleInfoEXT var_local_pGetMetalHandleInfo;
    vku::safe_VkMemoryGetMetalHandleInfoEXT* local_pGetMetalHandleInfo = nullptr;
    {
        if (pGetMetalHandleInfo) {
            local_pGetMetalHandleInfo = &var_local_pGetMetalHandleInfo;
            local_pGetMetalHandleInfo->initialize(pGetMetalHandleInfo);

            if (pGetMetalHandleInfo->memory) {
                local_pGetMetalHandleInfo->memory = Unwrap(pGetMetalHandleInfo->memory);
            }
        }
    }
    VkResult result = device_dispatch_table.GetMemoryMetalHandleEXT(
        device, (const VkMemoryGetMetalHandleInfoEXT*)local_pGetMetalHandleInfo, pHandle);

    return result;
}

VkResult DispatchDevice::GetMemoryMetalHandlePropertiesEXT(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType,
                                                           const void* pHandle,
                                                           VkMemoryMetalHandlePropertiesEXT* pMemoryMetalHandleProperties) {
    VkResult result =
        device_dispatch_table.GetMemoryMetalHandlePropertiesEXT(device, handleType, pHandle, pMemoryMetalHandleProperties);

    return result;
}
#endif  // VK_USE_PLATFORM_METAL_EXT

VkResult DispatchInstance::EnumeratePhysicalDeviceQueueFamilyPerformanceCountersByRegionARM(
    VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, uint32_t* pCounterCount, VkPerformanceCounterARM* pCounters,
    VkPerformanceCounterDescriptionARM* pCounterDescriptions) {
    VkResult result = instance_dispatch_table.EnumeratePhysicalDeviceQueueFamilyPerformanceCountersByRegionARM(
        physicalDevice, queueFamilyIndex, pCounterCount, pCounters, pCounterDescriptions);

    return result;
}

VkResult DispatchInstance::EnumeratePhysicalDeviceShaderInstrumentationMetricsARM(
    VkPhysicalDevice physicalDevice, uint32_t* pDescriptionCount, VkShaderInstrumentationMetricDescriptionARM* pDescriptions) {
    VkResult result = instance_dispatch_table.EnumeratePhysicalDeviceShaderInstrumentationMetricsARM(
        physicalDevice, pDescriptionCount, pDescriptions);

    return result;
}

VkResult DispatchDevice::CreateShaderInstrumentationARM(VkDevice device, const VkShaderInstrumentationCreateInfoARM* pCreateInfo,
                                                        const VkAllocationCallbacks* pAllocator,
                                                        VkShaderInstrumentationARM* pInstrumentation) {
    if (!wrap_handles)
        return device_dispatch_table.CreateShaderInstrumentationARM(device, pCreateInfo, pAllocator, pInstrumentation);

    VkResult result = device_dispatch_table.CreateShaderInstrumentationARM(device, pCreateInfo, pAllocator, pInstrumentation);
    if (result == VK_SUCCESS) {
        *pInstrumentation = WrapNew(*pInstrumentation);
    }
    return result;
}

void DispatchDevice::DestroyShaderInstrumentationARM(VkDevice device, VkShaderInstrumentationARM instrumentation,
                                                     const VkAllocationCallbacks* pAllocator) {
    if (!wrap_handles) return device_dispatch_table.DestroyShaderInstrumentationARM(device, instrumentation, pAllocator);
    instrumentation = Erase(instrumentation);
    device_dispatch_table.DestroyShaderInstrumentationARM(device, instrumentation, pAllocator);
}

void DispatchDevice::CmdBeginShaderInstrumentationARM(VkCommandBuffer commandBuffer, VkShaderInstrumentationARM instrumentation) {
    if (!wrap_handles) return device_dispatch_table.CmdBeginShaderInstrumentationARM(commandBuffer, instrumentation);
    {
        instrumentation = Unwrap(instrumentation);
    }
    device_dispatch_table.CmdBeginShaderInstrumentationARM(commandBuffer, instrumentation);
}

void DispatchDevice::CmdEndShaderInstrumentationARM(VkCommandBuffer commandBuffer) {
    device_dispatch_table.CmdEndShaderInstrumentationARM(commandBuffer);
}

VkResult DispatchDevice::GetShaderInstrumentationValuesARM(VkDevice device, VkShaderInstrumentationARM instrumentation,
                                                           uint32_t* pMetricBlockCount, void* pMetricValues,
                                                           VkShaderInstrumentationValuesFlagsARM flags) {
    if (!wrap_handles)
        return device_dispatch_table.GetShaderInstrumentationValuesARM(device, instrumentation, pMetricBlockCount, pMetricValues,
                                                                       flags);
    {
        instrumentation = Unwrap(instrumentation);
    }
    VkResult result =
        device_dispatch_table.GetShaderInstrumentationValuesARM(device, instrumentation, pMetricBlockCount, pMetricValues, flags);

    return result;
}

void DispatchDevice::ClearShaderInstrumentationMetricsARM(VkDevice device, VkShaderInstrumentationARM instrumentation) {
    if (!wrap_handles) return device_dispatch_table.ClearShaderInstrumentationMetricsARM(device, instrumentation);
    {
        instrumentation = Unwrap(instrumentation);
    }
    device_dispatch_table.ClearShaderInstrumentationMetricsARM(device, instrumentation);
}

void DispatchDevice::CmdEndRendering2EXT(VkCommandBuffer commandBuffer, const VkRenderingEndInfoKHR* pRenderingEndInfo) {
    device_dispatch_table.CmdEndRendering2EXT(commandBuffer, pRenderingEndInfo);
}

void DispatchDevice::CmdBeginCustomResolveEXT(VkCommandBuffer commandBuffer,
                                              const VkBeginCustomResolveInfoEXT* pBeginCustomResolveInfo) {
    device_dispatch_table.CmdBeginCustomResolveEXT(commandBuffer, pBeginCustomResolveInfo);
}

void DispatchDevice::CmdSetComputeOccupancyPriorityNV(VkCommandBuffer commandBuffer,
                                                      const VkComputeOccupancyPriorityParametersNV* pParameters) {
    device_dispatch_table.CmdSetComputeOccupancyPriorityNV(commandBuffer, pParameters);
}
#ifdef VK_USE_PLATFORM_UBM_SEC

VkResult DispatchInstance::CreateUbmSurfaceSEC(VkInstance instance, const VkUbmSurfaceCreateInfoSEC* pCreateInfo,
                                               const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    if (!wrap_handles) return instance_dispatch_table.CreateUbmSurfaceSEC(instance, pCreateInfo, pAllocator, pSurface);

    VkResult result = instance_dispatch_table.CreateUbmSurfaceSEC(instance, pCreateInfo, pAllocator, pSurface);
    if (result == VK_SUCCESS) {
        *pSurface = WrapNew(*pSurface);
    }
    return result;
}

VkBool32 DispatchInstance::GetPhysicalDeviceUbmPresentationSupportSEC(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
                                                                      struct ubm_device* device) {
    VkBool32 result = instance_dispatch_table.GetPhysicalDeviceUbmPresentationSupportSEC(physicalDevice, queueFamilyIndex, device);

    return result;
}
#endif  // VK_USE_PLATFORM_UBM_SEC

void DispatchDevice::CmdSetPrimitiveRestartIndexEXT(VkCommandBuffer commandBuffer, uint32_t primitiveRestartIndex) {
    device_dispatch_table.CmdSetPrimitiveRestartIndexEXT(commandBuffer, primitiveRestartIndex);
}

VkResult DispatchDevice::CreateAccelerationStructureKHR(VkDevice device, const VkAccelerationStructureCreateInfoKHR* pCreateInfo,
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
    if (result == VK_SUCCESS) {
        *pAccelerationStructure = WrapNew(*pAccelerationStructure);
    }
    return result;
}

void DispatchDevice::DestroyAccelerationStructureKHR(VkDevice device, VkAccelerationStructureKHR accelerationStructure,
                                                     const VkAllocationCallbacks* pAllocator) {
    if (!wrap_handles) return device_dispatch_table.DestroyAccelerationStructureKHR(device, accelerationStructure, pAllocator);
    accelerationStructure = Erase(accelerationStructure);
    device_dispatch_table.DestroyAccelerationStructureKHR(device, accelerationStructure, pAllocator);
}

void DispatchDevice::CmdBuildAccelerationStructuresIndirectKHR(VkCommandBuffer commandBuffer, uint32_t infoCount,
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

VkResult DispatchDevice::CopyAccelerationStructureKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
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

VkResult DispatchDevice::CopyAccelerationStructureToMemoryKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
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

VkResult DispatchDevice::CopyMemoryToAccelerationStructureKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
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

VkResult DispatchDevice::WriteAccelerationStructuresPropertiesKHR(VkDevice device, uint32_t accelerationStructureCount,
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

void DispatchDevice::CmdCopyAccelerationStructureKHR(VkCommandBuffer commandBuffer,
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

void DispatchDevice::CmdCopyAccelerationStructureToMemoryKHR(VkCommandBuffer commandBuffer,
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

void DispatchDevice::CmdCopyMemoryToAccelerationStructureKHR(VkCommandBuffer commandBuffer,
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

VkDeviceAddress DispatchDevice::GetAccelerationStructureDeviceAddressKHR(VkDevice device,
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

void DispatchDevice::CmdWriteAccelerationStructuresPropertiesKHR(VkCommandBuffer commandBuffer, uint32_t accelerationStructureCount,
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

void DispatchDevice::GetDeviceAccelerationStructureCompatibilityKHR(VkDevice device,
                                                                    const VkAccelerationStructureVersionInfoKHR* pVersionInfo,
                                                                    VkAccelerationStructureCompatibilityKHR* pCompatibility) {
    device_dispatch_table.GetDeviceAccelerationStructureCompatibilityKHR(device, pVersionInfo, pCompatibility);
}

void DispatchDevice::CmdTraceRaysKHR(VkCommandBuffer commandBuffer,
                                     const VkStridedDeviceAddressRegionKHR* pRaygenShaderBindingTable,
                                     const VkStridedDeviceAddressRegionKHR* pMissShaderBindingTable,
                                     const VkStridedDeviceAddressRegionKHR* pHitShaderBindingTable,
                                     const VkStridedDeviceAddressRegionKHR* pCallableShaderBindingTable, uint32_t width,
                                     uint32_t height, uint32_t depth) {
    device_dispatch_table.CmdTraceRaysKHR(commandBuffer, pRaygenShaderBindingTable, pMissShaderBindingTable, pHitShaderBindingTable,
                                          pCallableShaderBindingTable, width, height, depth);
}

VkResult DispatchDevice::GetRayTracingCaptureReplayShaderGroupHandlesKHR(VkDevice device, VkPipeline pipeline, uint32_t firstGroup,
                                                                         uint32_t groupCount, size_t dataSize, void* pData) {
    if (!wrap_handles)
        return device_dispatch_table.GetRayTracingCaptureReplayShaderGroupHandlesKHR(device, pipeline, firstGroup, groupCount,
                                                                                     dataSize, pData);
    {
        pipeline = Unwrap(pipeline);
    }
    VkResult result = device_dispatch_table.GetRayTracingCaptureReplayShaderGroupHandlesKHR(device, pipeline, firstGroup,
                                                                                            groupCount, dataSize, pData);

    return result;
}

void DispatchDevice::CmdTraceRaysIndirectKHR(VkCommandBuffer commandBuffer,
                                             const VkStridedDeviceAddressRegionKHR* pRaygenShaderBindingTable,
                                             const VkStridedDeviceAddressRegionKHR* pMissShaderBindingTable,
                                             const VkStridedDeviceAddressRegionKHR* pHitShaderBindingTable,
                                             const VkStridedDeviceAddressRegionKHR* pCallableShaderBindingTable,
                                             VkDeviceAddress indirectDeviceAddress) {
    device_dispatch_table.CmdTraceRaysIndirectKHR(commandBuffer, pRaygenShaderBindingTable, pMissShaderBindingTable,
                                                  pHitShaderBindingTable, pCallableShaderBindingTable, indirectDeviceAddress);
}

VkDeviceSize DispatchDevice::GetRayTracingShaderGroupStackSizeKHR(VkDevice device, VkPipeline pipeline, uint32_t group,
                                                                  VkShaderGroupShaderKHR groupShader) {
    if (!wrap_handles) return device_dispatch_table.GetRayTracingShaderGroupStackSizeKHR(device, pipeline, group, groupShader);
    {
        pipeline = Unwrap(pipeline);
    }
    VkDeviceSize result = device_dispatch_table.GetRayTracingShaderGroupStackSizeKHR(device, pipeline, group, groupShader);

    return result;
}

void DispatchDevice::CmdSetRayTracingPipelineStackSizeKHR(VkCommandBuffer commandBuffer, uint32_t pipelineStackSize) {
    device_dispatch_table.CmdSetRayTracingPipelineStackSizeKHR(commandBuffer, pipelineStackSize);
}

void DispatchDevice::CmdDrawMeshTasksEXT(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY,
                                         uint32_t groupCountZ) {
    device_dispatch_table.CmdDrawMeshTasksEXT(commandBuffer, groupCountX, groupCountY, groupCountZ);
}

void DispatchDevice::CmdDrawMeshTasksIndirectEXT(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                 uint32_t drawCount, uint32_t stride) {
    if (!wrap_handles) return device_dispatch_table.CmdDrawMeshTasksIndirectEXT(commandBuffer, buffer, offset, drawCount, stride);
    {
        buffer = Unwrap(buffer);
    }
    device_dispatch_table.CmdDrawMeshTasksIndirectEXT(commandBuffer, buffer, offset, drawCount, stride);
}

void DispatchDevice::CmdDrawMeshTasksIndirectCountEXT(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
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
}  // namespace vvl

// NOLINTEND
