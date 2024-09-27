/* Copyright (c) 2018-2024 The Khronos Group Inc.
 * Copyright (c) 2018-2024 Valve Corporation
 * Copyright (c) 2018-2024 LunarG, Inc.
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

#include <cmath>
#include <fstream>
#include "utils/hash_util.h"
#include "gpu/core/gpuav.h"
#include "gpu/cmd_validation/gpuav_draw.h"
#include "gpu/cmd_validation/gpuav_dispatch.h"
#include "gpu/cmd_validation/gpuav_trace_rays.h"
#include "gpu/cmd_validation/gpuav_copy_buffer_to_image.h"
#include "gpu/descriptor_validation/gpuav_descriptor_validation.h"
#include "gpu/descriptor_validation/gpuav_image_layout.h"
#include "gpu/resources/gpuav_subclasses.h"
#include "gpu/instrumentation/gpuav_instrumentation.h"
#include "gpu/shaders/gpu_shaders_constants.h"
#include "chassis/chassis_modification_state.h"
#include "gpu/core/gpu_shader_cache_hash.h"

// Generated shaders
#include "generated/gpu_av_shader_hash.h"

namespace gpuav {

void Validator::PreCallRecordCreateBuffer(VkDevice device, const VkBufferCreateInfo *pCreateInfo,
                                          const VkAllocationCallbacks *pAllocator, VkBuffer *pBuffer,
                                          const RecordObject &record_obj, chassis::CreateBuffer &chassis_state) {
    // Ray tracing acceleration structure instance buffers also need the storage buffer usage as
    // acceleration structure build validation will find and replace invalid acceleration structure
    // handles inside of a compute shader.
    if (chassis_state.modified_create_info.usage & VK_BUFFER_USAGE_RAY_TRACING_BIT_NV) {
        chassis_state.modified_create_info.usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    }

    // Indirect buffers will require validation shader to bind the indirect buffers as a storage buffer.
    if (gpuav_settings.IsBufferValidationEnabled() &&
        chassis_state.modified_create_info.usage & VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT) {
        chassis_state.modified_create_info.usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    }

    BaseClass::PreCallRecordCreateBuffer(device, pCreateInfo, pAllocator, pBuffer, record_obj, chassis_state);
}

void Validator::PostCallRecordGetPhysicalDeviceProperties2(VkPhysicalDevice physicalDevice,
                                                           VkPhysicalDeviceProperties2 *device_props2,
                                                           const RecordObject &record_obj) {
    // override all possible places maxUpdateAfterBindDescriptorsInAllPools can be set
    auto *desc_indexing_props = vku::FindStructInPNextChain<VkPhysicalDeviceDescriptorIndexingProperties>(device_props2->pNext);
    if (desc_indexing_props &&
        desc_indexing_props->maxUpdateAfterBindDescriptorsInAllPools > glsl::kDebugInputBindlessMaxDescSets) {
        std::stringstream ss;
        ss << "Setting VkPhysicalDeviceDescriptorIndexingProperties::maxUpdateAfterBindDescriptorsInAllPools to "
           << glsl::kDebugInputBindlessMaxDescSets;
        InternalWarning(physicalDevice, record_obj.location, ss.str().c_str());
        desc_indexing_props->maxUpdateAfterBindDescriptorsInAllPools = glsl::kDebugInputBindlessMaxDescSets;
    }

    auto *vk12_props = vku::FindStructInPNextChain<VkPhysicalDeviceVulkan12Properties>(device_props2->pNext);
    if (vk12_props && vk12_props->maxUpdateAfterBindDescriptorsInAllPools > glsl::kDebugInputBindlessMaxDescSets) {
        std::stringstream ss;
        ss << "Setting VkPhysicalDeviceVulkan12Properties::maxUpdateAfterBindDescriptorsInAllPools to "
           << glsl::kDebugInputBindlessMaxDescSets;
        InternalWarning(physicalDevice, record_obj.location, ss.str().c_str());
        vk12_props->maxUpdateAfterBindDescriptorsInAllPools = glsl::kDebugInputBindlessMaxDescSets;
    }

    BaseClass::PostCallRecordGetPhysicalDeviceProperties2(physicalDevice, device_props2, record_obj);
}

void Validator::PreCallRecordDestroyRenderPass(VkDevice device, VkRenderPass renderPass, const VkAllocationCallbacks *pAllocator,
                                               const RecordObject &record_obj) {
    DestroyRenderPassMappedResources(*this, renderPass);
    BaseClass::PreCallRecordDestroyRenderPass(device, renderPass, pAllocator, record_obj);
}

// Clean up device-related resources
void Validator::PreCallRecordDestroyDevice(VkDevice device, const VkAllocationCallbacks *pAllocator,
                                           const RecordObject &record_obj) {
    desc_heap_.reset();

    shared_resources_manager.Clear();

    if (gpuav_settings.cache_instrumented_shaders && !instrumented_shaders_cache_.IsEmpty()) {
        std::ofstream file_stream(instrumented_shader_cache_path_, std::ofstream::out | std::ofstream::binary);
        if (file_stream) {
            ShaderCacheHash shader_cache_hash(gpuav_settings.shader_instrumentation);
            file_stream.write(reinterpret_cast<const char *>(&shader_cache_hash), sizeof(shader_cache_hash));
            uint32_t datasize = static_cast<uint32_t>(instrumented_shaders_cache_.spirv_shaders_.size());
            file_stream.write(reinterpret_cast<char *>(&datasize), sizeof(uint32_t));
            for (auto &record : instrumented_shaders_cache_.spirv_shaders_) {
                // Hash of shader
                file_stream.write(reinterpret_cast<const char *>(&record.first), sizeof(uint32_t));
                const size_t spirv_dwords_count = record.second.size();
                file_stream.write(reinterpret_cast<const char *>(&spirv_dwords_count), sizeof(uint32_t));
                file_stream.write(reinterpret_cast<const char *>(record.second.data()), spirv_dwords_count * sizeof(uint32_t));
            }
            file_stream.close();
        }
    }
    BaseClass::PreCallRecordDestroyDevice(device, pAllocator, record_obj);
}

void Validator::RecordCmdBeginRenderPassLayouts(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                                const VkSubpassContents contents) {
    if (!pRenderPassBegin) {
        return;
    }
    auto cb_state = GetWrite<vvl::CommandBuffer>(commandBuffer);
    auto render_pass_state = Get<vvl::RenderPass>(pRenderPassBegin->renderPass);
    if (cb_state && render_pass_state) {
        // transition attachments to the correct layouts for beginning of renderPass and first subpass
        TransitionBeginRenderPassLayouts(*cb_state, *render_pass_state);
    }
}

void Validator::PreCallRecordCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                                VkSubpassContents contents, const RecordObject &record_obj) {
    BaseClass::PreCallRecordCmdBeginRenderPass(commandBuffer, pRenderPassBegin, contents, record_obj);
    RecordCmdBeginRenderPassLayouts(commandBuffer, pRenderPassBegin, contents);
}

void Validator::PreCallRecordCmdBeginRenderPass2KHR(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                                    const VkSubpassBeginInfo *pSubpassBeginInfo, const RecordObject &record_obj) {
    PreCallRecordCmdBeginRenderPass2(commandBuffer, pRenderPassBegin, pSubpassBeginInfo, record_obj);
}

void Validator::PreCallRecordCmdBeginRenderPass2(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                                 const VkSubpassBeginInfo *pSubpassBeginInfo, const RecordObject &record_obj) {
    BaseClass::PreCallRecordCmdBeginRenderPass2(commandBuffer, pRenderPassBegin, pSubpassBeginInfo, record_obj);
    RecordCmdBeginRenderPassLayouts(commandBuffer, pRenderPassBegin, pSubpassBeginInfo->contents);
}

void Validator::RecordCmdEndRenderPassLayouts(VkCommandBuffer commandBuffer) {
    auto cb_state = GetWrite<vvl::CommandBuffer>(commandBuffer);
    if (cb_state) {
        TransitionFinalSubpassLayouts(*cb_state);
    }
}

void Validator::PostCallRecordCmdEndRenderPass(VkCommandBuffer commandBuffer, const RecordObject &record_obj) {
    // Record the end at the CoreLevel to ensure StateTracker cleanup doesn't step on anything we need.
    RecordCmdEndRenderPassLayouts(commandBuffer);
    BaseClass::PostCallRecordCmdEndRenderPass(commandBuffer, record_obj);
}

void Validator::PostCallRecordCmdEndRenderPass2KHR(VkCommandBuffer commandBuffer, const VkSubpassEndInfo *pSubpassEndInfo,
                                                   const RecordObject &record_obj) {
    PostCallRecordCmdEndRenderPass2(commandBuffer, pSubpassEndInfo, record_obj);
}

void Validator::PostCallRecordCmdEndRenderPass2(VkCommandBuffer commandBuffer, const VkSubpassEndInfo *pSubpassEndInfo,
                                                const RecordObject &record_obj) {
    RecordCmdEndRenderPassLayouts(commandBuffer);
    BaseClass::PostCallRecordCmdEndRenderPass2(commandBuffer, pSubpassEndInfo, record_obj);
}

void Validator::RecordCmdNextSubpassLayouts(VkCommandBuffer commandBuffer, VkSubpassContents contents) {
    auto cb_state = GetWrite<vvl::CommandBuffer>(commandBuffer);
    TransitionSubpassLayouts(*cb_state, *cb_state->activeRenderPass, cb_state->GetActiveSubpass());
}

void Validator::PostCallRecordCmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents,
                                             const RecordObject &record_obj) {
    BaseClass::PostCallRecordCmdNextSubpass(commandBuffer, contents, record_obj);
    RecordCmdNextSubpassLayouts(commandBuffer, contents);
}

void Validator::PostCallRecordCmdNextSubpass2KHR(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo *pSubpassBeginInfo,
                                                 const VkSubpassEndInfo *pSubpassEndInfo, const RecordObject &record_obj) {
    PostCallRecordCmdNextSubpass2(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo, record_obj);
}

void Validator::PostCallRecordCmdNextSubpass2(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo *pSubpassBeginInfo,
                                              const VkSubpassEndInfo *pSubpassEndInfo, const RecordObject &record_obj) {
    BaseClass::PostCallRecordCmdNextSubpass2(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo, record_obj);
    RecordCmdNextSubpassLayouts(commandBuffer, pSubpassBeginInfo->contents);
}

void Validator::PostCallRecordCmdBindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                              VkPipeline pipeline, const RecordObject &record_obj) {
    BaseClass::PostCallRecordCmdBindPipeline(commandBuffer, pipelineBindPoint, pipeline, record_obj);

    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    if (!cb_state) {
        InternalError(commandBuffer, record_obj.location, "Unrecognized command buffer.");
        return;
    }
    UpdateBoundPipeline(*this, *cb_state, pipelineBindPoint, pipeline, record_obj.location);
}

void Validator::PostCallRecordCmdBindDescriptorSets(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                    VkPipelineLayout layout, uint32_t firstSet, uint32_t descriptorSetCount,
                                                    const VkDescriptorSet *pDescriptorSets, uint32_t dynamicOffsetCount,
                                                    const uint32_t *pDynamicOffsets, const RecordObject &record_obj) {
    BaseClass::PostCallRecordCmdBindDescriptorSets(commandBuffer, pipelineBindPoint, layout, firstSet, descriptorSetCount,
                                                   pDescriptorSets, dynamicOffsetCount, pDynamicOffsets, record_obj);

    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    if (!cb_state) {
        InternalError(commandBuffer, record_obj.location, "Unrecognized command buffer.");
        return;
    }
    UpdateBoundDescriptors(*this, *cb_state, pipelineBindPoint, record_obj.location);
}
void Validator::PostCallRecordCmdBindDescriptorSets2KHR(VkCommandBuffer commandBuffer,
                                                        const VkBindDescriptorSetsInfoKHR *pBindDescriptorSetsInfo,
                                                        const RecordObject &record_obj) {
    BaseClass::PostCallRecordCmdBindDescriptorSets2KHR(commandBuffer, pBindDescriptorSetsInfo, record_obj);

    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    if (!cb_state) {
        InternalError(commandBuffer, record_obj.location, "Unrecognized command buffer.");
        return;
    }

    if (IsStageInPipelineBindPoint(pBindDescriptorSetsInfo->stageFlags, VK_PIPELINE_BIND_POINT_GRAPHICS)) {
        UpdateBoundDescriptors(*this, *cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS, record_obj.location);
    }
    if (IsStageInPipelineBindPoint(pBindDescriptorSetsInfo->stageFlags, VK_PIPELINE_BIND_POINT_COMPUTE)) {
        UpdateBoundDescriptors(*this, *cb_state, VK_PIPELINE_BIND_POINT_COMPUTE, record_obj.location);
    }
    if (IsStageInPipelineBindPoint(pBindDescriptorSetsInfo->stageFlags, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR)) {
        UpdateBoundDescriptors(*this, *cb_state, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, record_obj.location);
    }
}

void Validator::PreCallRecordCmdPushDescriptorSetKHR(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                     VkPipelineLayout layout, uint32_t set, uint32_t descriptorWriteCount,
                                                     const VkWriteDescriptorSet *pDescriptorWrites,
                                                     const RecordObject &record_obj) {
    BaseClass::PreCallRecordCmdPushDescriptorSetKHR(commandBuffer, pipelineBindPoint, layout, set, descriptorWriteCount,
                                                    pDescriptorWrites, record_obj);

    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    if (!cb_state) {
        InternalError(commandBuffer, record_obj.location, "Unrecognized command buffer.");
        return;
    }
    UpdateBoundDescriptors(*this, *cb_state, pipelineBindPoint, record_obj.location);
}

void Validator::PreCallRecordCmdPushDescriptorSet2KHR(VkCommandBuffer commandBuffer,
                                                      const VkPushDescriptorSetInfoKHR *pPushDescriptorSetInfo,
                                                      const RecordObject &record_obj) {
    BaseClass::PreCallRecordCmdPushDescriptorSet2KHR(commandBuffer, pPushDescriptorSetInfo, record_obj);
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    if (!cb_state) {
        InternalError(commandBuffer, record_obj.location, "Unrecognized command buffer.");
        return;
    }

    if (IsStageInPipelineBindPoint(pPushDescriptorSetInfo->stageFlags, VK_PIPELINE_BIND_POINT_GRAPHICS)) {
        UpdateBoundDescriptors(*this, *cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS, record_obj.location);
    }
    if (IsStageInPipelineBindPoint(pPushDescriptorSetInfo->stageFlags, VK_PIPELINE_BIND_POINT_COMPUTE)) {
        UpdateBoundDescriptors(*this, *cb_state, VK_PIPELINE_BIND_POINT_COMPUTE, record_obj.location);
    }
    if (IsStageInPipelineBindPoint(pPushDescriptorSetInfo->stageFlags, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR)) {
        UpdateBoundDescriptors(*this, *cb_state, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, record_obj.location);
    }
}

void Validator::PreCallRecordCmdBindDescriptorBuffersEXT(VkCommandBuffer commandBuffer, uint32_t bufferCount,
                                                         const VkDescriptorBufferBindingInfoEXT *pBindingInfos,
                                                         const RecordObject &record_obj) {
    BaseClass::PreCallRecordCmdBindDescriptorBuffersEXT(commandBuffer, bufferCount, pBindingInfos, record_obj);
    // TODO - Unsupported
}

void Validator::PreCallRecordCmdBindDescriptorBufferEmbeddedSamplersEXT(VkCommandBuffer commandBuffer,
                                                                        VkPipelineBindPoint pipelineBindPoint,
                                                                        VkPipelineLayout layout, uint32_t set,
                                                                        const RecordObject &record_obj) {
    BaseClass::PreCallRecordCmdBindDescriptorBufferEmbeddedSamplersEXT(commandBuffer, pipelineBindPoint, layout, set, record_obj);
    // TODO - Unsupported
}

void Validator::PreCallRecordCmdBindDescriptorBufferEmbeddedSamplers2EXT(
    VkCommandBuffer commandBuffer, const VkBindDescriptorBufferEmbeddedSamplersInfoEXT *pBindDescriptorBufferEmbeddedSamplersInfo,
    const RecordObject &record_obj) {
    BaseClass::PreCallRecordCmdBindDescriptorBufferEmbeddedSamplers2EXT(commandBuffer, pBindDescriptorBufferEmbeddedSamplersInfo,
                                                                        record_obj);
    // TODO - Unsupported
}

void Validator::PreCallRecordCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount,
                                     uint32_t firstVertex, uint32_t firstInstance, const RecordObject &record_obj) {
    BaseClass::PreCallRecordCmdDraw(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance, record_obj);

    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    if (!cb_state) {
        InternalError(commandBuffer, record_obj.location, "Unrecognized command buffer.");
        return;
    }

    PreCallSetupShaderInstrumentationResources(*this, *cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS, record_obj.location);
}

void Validator::PostCallRecordCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount,
                                      uint32_t firstVertex, uint32_t firstInstance, const RecordObject &record_obj) {
    BaseClass::PostCallRecordCmdDraw(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance, record_obj);

    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    if (!cb_state) {
        InternalError(commandBuffer, record_obj.location, "Unrecognized command buffer.");
        return;
    }

    PostCallSetupShaderInstrumentationResources(*this, *cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS, record_obj.location);
}

void Validator::PreCallRecordCmdDrawMultiEXT(VkCommandBuffer commandBuffer, uint32_t drawCount,
                                             const VkMultiDrawInfoEXT *pVertexInfo, uint32_t instanceCount, uint32_t firstInstance,
                                             uint32_t stride, const RecordObject &record_obj) {
    BaseClass::PreCallRecordCmdDrawMultiEXT(commandBuffer, drawCount, pVertexInfo, instanceCount, firstInstance, stride,
                                            record_obj);

    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    if (!cb_state) {
        InternalError(commandBuffer, record_obj.location, "Unrecognized command buffer.");
        return;
    }

    PreCallSetupShaderInstrumentationResources(*this, *cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS, record_obj.location);
}

void Validator::PostCallRecordCmdDrawMultiEXT(VkCommandBuffer commandBuffer, uint32_t drawCount,
                                              const VkMultiDrawInfoEXT *pVertexInfo, uint32_t instanceCount, uint32_t firstInstance,
                                              uint32_t stride, const RecordObject &record_obj) {
    BaseClass::PostCallRecordCmdDrawMultiEXT(commandBuffer, drawCount, pVertexInfo, instanceCount, firstInstance, stride,
                                             record_obj);

    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    if (!cb_state) {
        InternalError(commandBuffer, record_obj.location, "Unrecognized command buffer.");
        return;
    }

    PostCallSetupShaderInstrumentationResources(*this, *cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS, record_obj.location);
}

void Validator::PreCallRecordCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount,
                                            uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance,
                                            const RecordObject &record_obj) {
    BaseClass::PreCallRecordCmdDrawIndexed(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance,
                                           record_obj);
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    if (!cb_state) {
        InternalError(commandBuffer, record_obj.location, "Unrecognized command buffer.");
        return;
    }
    PreCallSetupShaderInstrumentationResources(*this, *cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS, record_obj.location);
}

void Validator::PostCallRecordCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount,
                                             uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance,
                                             const RecordObject &record_obj) {
    BaseClass::PostCallRecordCmdDrawIndexed(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance,
                                            record_obj);
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    if (!cb_state) {
        InternalError(commandBuffer, record_obj.location, "Unrecognized command buffer.");
        return;
    }
    PostCallSetupShaderInstrumentationResources(*this, *cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS, record_obj.location);
}

void Validator::PreCallRecordCmdDrawMultiIndexedEXT(VkCommandBuffer commandBuffer, uint32_t drawCount,
                                                    const VkMultiDrawIndexedInfoEXT *pIndexInfo, uint32_t instanceCount,
                                                    uint32_t firstInstance, uint32_t stride, const int32_t *pVertexOffset,
                                                    const RecordObject &record_obj) {
    BaseClass::PreCallRecordCmdDrawMultiIndexedEXT(commandBuffer, drawCount, pIndexInfo, instanceCount, firstInstance, stride,
                                                   pVertexOffset, record_obj);
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    if (!cb_state) {
        InternalError(commandBuffer, record_obj.location, "Unrecognized command buffer.");
        return;
    }
    for (uint32_t i = 0; i < drawCount; i++) {
        // #ARNO_TODO calling Setup drawCount times seems weird...
        PreCallSetupShaderInstrumentationResources(*this, *cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS, record_obj.location);
    }
}

void Validator::PostCallRecordCmdDrawMultiIndexedEXT(VkCommandBuffer commandBuffer, uint32_t drawCount,
                                                     const VkMultiDrawIndexedInfoEXT *pIndexInfo, uint32_t instanceCount,
                                                     uint32_t firstInstance, uint32_t stride, const int32_t *pVertexOffset,
                                                     const RecordObject &record_obj) {
    BaseClass::PostCallRecordCmdDrawMultiIndexedEXT(commandBuffer, drawCount, pIndexInfo, instanceCount, firstInstance, stride,
                                                    pVertexOffset, record_obj);
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    if (!cb_state) {
        InternalError(commandBuffer, record_obj.location, "Unrecognized command buffer.");
        return;
    }

    PostCallSetupShaderInstrumentationResources(*this, *cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS, record_obj.location);
}

void Validator::PreCallRecordCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t count,
                                             uint32_t stride, const RecordObject &record_obj) {
    BaseClass::PreCallRecordCmdDrawIndirect(commandBuffer, buffer, offset, count, stride, record_obj);

    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    if (!cb_state) {
        InternalError(commandBuffer, record_obj.location, "Unrecognized command buffer.");
        return;
    }

    InsertIndirectDrawValidation(*this, record_obj.location, *cb_state, buffer, offset, count, VK_NULL_HANDLE, 0, stride);
    PreCallSetupShaderInstrumentationResources(*this, *cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS, record_obj.location);
}

void Validator::PostCallRecordCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t count,
                                              uint32_t stride, const RecordObject &record_obj) {
    BaseClass::PostCallRecordCmdDrawIndirect(commandBuffer, buffer, offset, count, stride, record_obj);

    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    if (!cb_state) {
        InternalError(commandBuffer, record_obj.location, "Unrecognized command buffer.");
        return;
    }

    PostCallSetupShaderInstrumentationResources(*this, *cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS, record_obj.location);
}

void Validator::PreCallRecordCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                    uint32_t count, uint32_t stride, const RecordObject &record_obj) {
    BaseClass::PreCallRecordCmdDrawIndexedIndirect(commandBuffer, buffer, offset, count, stride, record_obj);

    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    if (!cb_state) {
        InternalError(commandBuffer, record_obj.location, "Unrecognized command buffer.");
        return;
    }

    InsertIndirectDrawValidation(*this, record_obj.location, *cb_state, buffer, offset, count, VK_NULL_HANDLE, 0, stride);
    PreCallSetupShaderInstrumentationResources(*this, *cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS, record_obj.location);
}

void Validator::PostCallRecordCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                     uint32_t count, uint32_t stride, const RecordObject &record_obj) {
    BaseClass::PostCallRecordCmdDrawIndexedIndirect(commandBuffer, buffer, offset, count, stride, record_obj);

    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    if (!cb_state) {
        InternalError(commandBuffer, record_obj.location, "Unrecognized command buffer.");
        return;
    }

    PostCallSetupShaderInstrumentationResources(*this, *cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS, record_obj.location);
}

void Validator::PreCallRecordCmdDrawIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                     VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                     uint32_t stride, const RecordObject &record_obj) {
    PreCallRecordCmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride,
                                      record_obj);
}

void Validator::PostCallRecordCmdDrawIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                      VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                      uint32_t stride, const RecordObject &record_obj) {
    PostCallRecordCmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride,
                                       record_obj);
}

void Validator::PreCallRecordCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                  VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                  uint32_t stride, const RecordObject &record_obj) {
    BaseClass::PreCallRecordCmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount,
                                                 stride, record_obj);

    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    if (!cb_state) {
        InternalError(commandBuffer, record_obj.location, "Unrecognized command buffer.");
        return;
    }
    InsertIndirectDrawValidation(*this, record_obj.location, *cb_state, buffer, offset, maxDrawCount, countBuffer,
                                 countBufferOffset, stride);
    PreCallSetupShaderInstrumentationResources(*this, *cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS, record_obj.location);
}

void Validator::PostCallRecordCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                   VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                   uint32_t stride, const RecordObject &record_obj) {
    BaseClass::PostCallRecordCmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount,
                                                  stride, record_obj);

    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    if (!cb_state) {
        InternalError(commandBuffer, record_obj.location, "Unrecognized command buffer.");
        return;
    }

    PostCallSetupShaderInstrumentationResources(*this, *cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS, record_obj.location);
}

void Validator::PreCallRecordCmdDrawIndirectByteCountEXT(VkCommandBuffer commandBuffer, uint32_t instanceCount,
                                                         uint32_t firstInstance, VkBuffer counterBuffer,
                                                         VkDeviceSize counterBufferOffset, uint32_t counterOffset,
                                                         uint32_t vertexStride, const RecordObject &record_obj) {
    BaseClass::PreCallRecordCmdDrawIndirectByteCountEXT(commandBuffer, instanceCount, firstInstance, counterBuffer,
                                                        counterBufferOffset, counterOffset, vertexStride, record_obj);
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    if (!cb_state) {
        InternalError(commandBuffer, record_obj.location, "Unrecognized command buffer.");
        return;
    }
    PreCallSetupShaderInstrumentationResources(*this, *cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS, record_obj.location);
}

void Validator::PostCallRecordCmdDrawIndirectByteCountEXT(VkCommandBuffer commandBuffer, uint32_t instanceCount,
                                                          uint32_t firstInstance, VkBuffer counterBuffer,
                                                          VkDeviceSize counterBufferOffset, uint32_t counterOffset,
                                                          uint32_t vertexStride, const RecordObject &record_obj) {
    BaseClass::PostCallRecordCmdDrawIndirectByteCountEXT(commandBuffer, instanceCount, firstInstance, counterBuffer,
                                                         counterBufferOffset, counterOffset, vertexStride, record_obj);
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    if (!cb_state) {
        InternalError(commandBuffer, record_obj.location, "Unrecognized command buffer.");
        return;
    }
    PostCallSetupShaderInstrumentationResources(*this, *cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS, record_obj.location);
}

void Validator::PreCallRecordCmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                            VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                            uint32_t maxDrawCount, uint32_t stride,
                                                            const RecordObject &record_obj) {
    PreCallRecordCmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride,
                                             record_obj);
}

void Validator::PostCallRecordCmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                             VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                             uint32_t maxDrawCount, uint32_t stride,
                                                             const RecordObject &record_obj) {
    PostCallRecordCmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride,
                                              record_obj);
}

void Validator::PreCallRecordCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                         VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                         uint32_t maxDrawCount, uint32_t stride, const RecordObject &record_obj) {
    BaseClass::PreCallRecordCmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount,
                                                        stride, record_obj);

    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    if (!cb_state) {
        InternalError(commandBuffer, record_obj.location, "Unrecognized command buffer.");
        return;
    }
    InsertIndirectDrawValidation(*this, record_obj.location, *cb_state, buffer, offset, maxDrawCount, countBuffer,
                                 countBufferOffset, stride);
    PreCallSetupShaderInstrumentationResources(*this, *cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS, record_obj.location);
}

void Validator::PostCallRecordCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                          VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                          uint32_t maxDrawCount, uint32_t stride, const RecordObject &record_obj) {
    BaseClass::PostCallRecordCmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset,
                                                         maxDrawCount, stride, record_obj);

    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    if (!cb_state) {
        InternalError(commandBuffer, record_obj.location, "Unrecognized command buffer.");
        return;
    }

    PostCallSetupShaderInstrumentationResources(*this, *cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS, record_obj.location);
}

void Validator::PreCallRecordCmdDrawMeshTasksNV(VkCommandBuffer commandBuffer, uint32_t taskCount, uint32_t firstTask,
                                                const RecordObject &record_obj) {
    ValidationStateTracker::PreCallRecordCmdDrawMeshTasksNV(commandBuffer, taskCount, firstTask, record_obj);
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    if (!cb_state) {
        InternalError(commandBuffer, record_obj.location, "Unrecognized command buffer.");
        return;
    }
    PreCallSetupShaderInstrumentationResources(*this, *cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS, record_obj.location);
}

void Validator::PostCallRecordCmdDrawMeshTasksNV(VkCommandBuffer commandBuffer, uint32_t taskCount, uint32_t firstTask,
                                                 const RecordObject &record_obj) {
    ValidationStateTracker::PostCallRecordCmdDrawMeshTasksNV(commandBuffer, taskCount, firstTask, record_obj);
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    if (!cb_state) {
        InternalError(commandBuffer, record_obj.location, "Unrecognized command buffer.");
        return;
    }
    PostCallSetupShaderInstrumentationResources(*this, *cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS, record_obj.location);
}

void Validator::PreCallRecordCmdDrawMeshTasksIndirectNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                        uint32_t drawCount, uint32_t stride, const RecordObject &record_obj) {
    BaseClass::PreCallRecordCmdDrawMeshTasksIndirectNV(commandBuffer, buffer, offset, drawCount, stride, record_obj);

    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    if (!cb_state) {
        InternalError(commandBuffer, record_obj.location, "Unrecognized command buffer.");
        return;
    }
    InsertIndirectDrawValidation(*this, record_obj.location, *cb_state, buffer, offset, drawCount, VK_NULL_HANDLE, 0, stride);
    PreCallSetupShaderInstrumentationResources(*this, *cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS, record_obj.location);
}

void Validator::PostCallRecordCmdDrawMeshTasksIndirectNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                         uint32_t drawCount, uint32_t stride, const RecordObject &record_obj) {
    BaseClass::PostCallRecordCmdDrawMeshTasksIndirectNV(commandBuffer, buffer, offset, drawCount, stride, record_obj);

    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    if (!cb_state) {
        InternalError(commandBuffer, record_obj.location, "Unrecognized command buffer.");
        return;
    }
    PostCallSetupShaderInstrumentationResources(*this, *cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS, record_obj.location);
}

void Validator::PreCallRecordCmdDrawMeshTasksIndirectCountNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                             VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                             uint32_t maxDrawCount, uint32_t stride,
                                                             const RecordObject &record_obj) {
    BaseClass::PreCallRecordCmdDrawMeshTasksIndirectCountNV(commandBuffer, buffer, offset, countBuffer, countBufferOffset,
                                                            maxDrawCount, stride, record_obj);

    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    if (!cb_state) {
        InternalError(commandBuffer, record_obj.location, "Unrecognized command buffer.");
        return;
    }
    InsertIndirectDrawValidation(*this, record_obj.location, *cb_state, buffer, offset, maxDrawCount, countBuffer,
                                 countBufferOffset, stride);
    PreCallSetupShaderInstrumentationResources(*this, *cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS, record_obj.location);
}

void Validator::PostCallRecordCmdDrawMeshTasksIndirectCountNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                              VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                              uint32_t maxDrawCount, uint32_t stride,
                                                              const RecordObject &record_obj) {
    BaseClass::PostCallRecordCmdDrawMeshTasksIndirectCountNV(commandBuffer, buffer, offset, countBuffer, countBufferOffset,
                                                             maxDrawCount, stride, record_obj);

    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    if (!cb_state) {
        InternalError(commandBuffer, record_obj.location, "Unrecognized command buffer.");
        return;
    }

    PostCallSetupShaderInstrumentationResources(*this, *cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS, record_obj.location);
}

void Validator::PreCallRecordCmdDrawMeshTasksEXT(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY,
                                                 uint32_t groupCountZ, const RecordObject &record_obj) {
    BaseClass::PreCallRecordCmdDrawMeshTasksEXT(commandBuffer, groupCountX, groupCountY, groupCountZ, record_obj);
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    if (!cb_state) {
        InternalError(commandBuffer, record_obj.location, "Unrecognized command buffer.");
        return;
    }
    PreCallSetupShaderInstrumentationResources(*this, *cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS, record_obj.location);
}

void Validator::PostCallRecordCmdDrawMeshTasksEXT(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY,
                                                  uint32_t groupCountZ, const RecordObject &record_obj) {
    BaseClass::PostCallRecordCmdDrawMeshTasksEXT(commandBuffer, groupCountX, groupCountY, groupCountZ, record_obj);
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    if (!cb_state) {
        InternalError(commandBuffer, record_obj.location, "Unrecognized command buffer.");
        return;
    }
    PostCallSetupShaderInstrumentationResources(*this, *cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS, record_obj.location);
}

void Validator::PreCallRecordCmdDrawMeshTasksIndirectEXT(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                         uint32_t drawCount, uint32_t stride, const RecordObject &record_obj) {
    BaseClass::PreCallRecordCmdDrawMeshTasksIndirectEXT(commandBuffer, buffer, offset, drawCount, stride, record_obj);

    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    if (!cb_state) {
        InternalError(commandBuffer, record_obj.location, "Unrecognized command buffer.");
        return;
    }
    InsertIndirectDrawValidation(*this, record_obj.location, *cb_state, buffer, offset, drawCount, VK_NULL_HANDLE, 0, stride);
    PreCallSetupShaderInstrumentationResources(*this, *cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS, record_obj.location);
}

void Validator::PostCallRecordCmdDrawMeshTasksIndirectEXT(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                          uint32_t drawCount, uint32_t stride, const RecordObject &record_obj) {
    BaseClass::PostCallRecordCmdDrawMeshTasksIndirectEXT(commandBuffer, buffer, offset, drawCount, stride, record_obj);

    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    if (!cb_state) {
        InternalError(commandBuffer, record_obj.location, "Unrecognized command buffer.");
        return;
    }
    PostCallSetupShaderInstrumentationResources(*this, *cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS, record_obj.location);
}

void Validator::PreCallRecordCmdDrawMeshTasksIndirectCountEXT(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                              VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                              uint32_t maxDrawCount, uint32_t stride,
                                                              const RecordObject &record_obj) {
    BaseClass::PreCallRecordCmdDrawMeshTasksIndirectCountEXT(commandBuffer, buffer, offset, countBuffer, countBufferOffset,
                                                             maxDrawCount, stride, record_obj);

    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    if (!cb_state) {
        InternalError(commandBuffer, record_obj.location, "Unrecognized command buffer.");
        return;
    }
    InsertIndirectDrawValidation(*this, record_obj.location, *cb_state, buffer, offset, maxDrawCount, countBuffer,
                                 countBufferOffset, stride);
    PreCallSetupShaderInstrumentationResources(*this, *cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS, record_obj.location);
}

void Validator::PostCallRecordCmdDrawMeshTasksIndirectCountEXT(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                               VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                               uint32_t maxDrawCount, uint32_t stride,
                                                               const RecordObject &record_obj) {
    BaseClass::PostCallRecordCmdDrawMeshTasksIndirectCountEXT(commandBuffer, buffer, offset, countBuffer, countBufferOffset,
                                                              maxDrawCount, stride, record_obj);

    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    if (!cb_state) {
        InternalError(commandBuffer, record_obj.location, "Unrecognized command buffer.");
        return;
    }

    PostCallSetupShaderInstrumentationResources(*this, *cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS, record_obj.location);
}

void Validator::PreCallRecordCmdDispatch(VkCommandBuffer commandBuffer, uint32_t x, uint32_t y, uint32_t z,
                                         const RecordObject &record_obj) {
    BaseClass::PreCallRecordCmdDispatch(commandBuffer, x, y, z, record_obj);

    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    if (!cb_state) {
        InternalError(commandBuffer, record_obj.location, "Unrecognized command buffer.");
        return;
    }
    PreCallSetupShaderInstrumentationResources(*this, *cb_state, VK_PIPELINE_BIND_POINT_COMPUTE, record_obj.location);
}

void Validator::PostCallRecordCmdDispatch(VkCommandBuffer commandBuffer, uint32_t x, uint32_t y, uint32_t z,
                                          const RecordObject &record_obj) {
    BaseClass::PostCallRecordCmdDispatch(commandBuffer, x, y, z, record_obj);

    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    if (!cb_state) {
        InternalError(commandBuffer, record_obj.location, "Unrecognized command buffer.");
        return;
    }
    PostCallSetupShaderInstrumentationResources(*this, *cb_state, VK_PIPELINE_BIND_POINT_COMPUTE, record_obj.location);
}

void Validator::PreCallRecordCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                 const RecordObject &record_obj) {
    BaseClass::PreCallRecordCmdDispatchIndirect(commandBuffer, buffer, offset, record_obj);

    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    if (!cb_state) {
        InternalError(commandBuffer, record_obj.location, "Unrecognized command buffer.");
        return;
    }
    InsertIndirectDispatchValidation(*this, record_obj.location, *cb_state, buffer, offset);
    PreCallSetupShaderInstrumentationResources(*this, *cb_state, VK_PIPELINE_BIND_POINT_COMPUTE, record_obj.location);
}

void Validator::PostCallRecordCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                  const RecordObject &record_obj) {
    BaseClass::PostCallRecordCmdDispatchIndirect(commandBuffer, buffer, offset, record_obj);

    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    if (!cb_state) {
        InternalError(commandBuffer, record_obj.location, "Unrecognized command buffer.");
        return;
    }

    PostCallSetupShaderInstrumentationResources(*this, *cb_state, VK_PIPELINE_BIND_POINT_COMPUTE, record_obj.location);
}

void Validator::PreCallRecordCmdDispatchBase(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY,
                                             uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ,
                                             const RecordObject &record_obj) {
    BaseClass::PreCallRecordCmdDispatchBase(commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY,
                                            groupCountZ, record_obj);

    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    if (!cb_state) {
        InternalError(commandBuffer, record_obj.location, "Unrecognized command buffer.");
        return;
    }
    PreCallSetupShaderInstrumentationResources(*this, *cb_state, VK_PIPELINE_BIND_POINT_COMPUTE, record_obj.location);
}

void Validator::PostCallRecordCmdDispatchBase(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY,
                                              uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ,
                                              const RecordObject &record_obj) {
    BaseClass::PostCallRecordCmdDispatchBase(commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY,
                                             groupCountZ, record_obj);

    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    if (!cb_state) {
        InternalError(commandBuffer, record_obj.location, "Unrecognized command buffer.");
        return;
    }
    PostCallSetupShaderInstrumentationResources(*this, *cb_state, VK_PIPELINE_BIND_POINT_COMPUTE, record_obj.location);
}

void Validator::PreCallRecordCmdDispatchBaseKHR(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY,
                                                uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY,
                                                uint32_t groupCountZ, const RecordObject &record_obj) {
    PreCallRecordCmdDispatchBase(commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ,
                                 record_obj);
}

void Validator::PostCallRecordCmdDispatchBaseKHR(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY,
                                                 uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY,
                                                 uint32_t groupCountZ, const RecordObject &record_obj) {
    PostCallRecordCmdDispatchBase(commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ,
                                  record_obj);
}

void Validator::PreCallRecordCmdTraceRaysNV(VkCommandBuffer commandBuffer, VkBuffer raygenShaderBindingTableBuffer,
                                            VkDeviceSize raygenShaderBindingOffset, VkBuffer missShaderBindingTableBuffer,
                                            VkDeviceSize missShaderBindingOffset, VkDeviceSize missShaderBindingStride,
                                            VkBuffer hitShaderBindingTableBuffer, VkDeviceSize hitShaderBindingOffset,
                                            VkDeviceSize hitShaderBindingStride, VkBuffer callableShaderBindingTableBuffer,
                                            VkDeviceSize callableShaderBindingOffset, VkDeviceSize callableShaderBindingStride,
                                            uint32_t width, uint32_t height, uint32_t depth, const RecordObject &record_obj) {
    BaseClass::PreCallRecordCmdTraceRaysNV(commandBuffer, raygenShaderBindingTableBuffer, raygenShaderBindingOffset,
                                           missShaderBindingTableBuffer, missShaderBindingOffset, missShaderBindingStride,
                                           hitShaderBindingTableBuffer, hitShaderBindingOffset, hitShaderBindingStride,
                                           callableShaderBindingTableBuffer, callableShaderBindingOffset,
                                           callableShaderBindingStride, width, height, depth, record_obj);

    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    if (!cb_state) {
        InternalError(commandBuffer, record_obj.location, "Unrecognized command buffer.");
        return;
    }
    PreCallSetupShaderInstrumentationResources(*this, *cb_state, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, record_obj.location);
}

void Validator::PostCallRecordCmdTraceRaysNV(VkCommandBuffer commandBuffer, VkBuffer raygenShaderBindingTableBuffer,
                                             VkDeviceSize raygenShaderBindingOffset, VkBuffer missShaderBindingTableBuffer,
                                             VkDeviceSize missShaderBindingOffset, VkDeviceSize missShaderBindingStride,
                                             VkBuffer hitShaderBindingTableBuffer, VkDeviceSize hitShaderBindingOffset,
                                             VkDeviceSize hitShaderBindingStride, VkBuffer callableShaderBindingTableBuffer,
                                             VkDeviceSize callableShaderBindingOffset, VkDeviceSize callableShaderBindingStride,
                                             uint32_t width, uint32_t height, uint32_t depth, const RecordObject &record_obj) {
    BaseClass::PostCallRecordCmdTraceRaysNV(commandBuffer, raygenShaderBindingTableBuffer, raygenShaderBindingOffset,
                                            missShaderBindingTableBuffer, missShaderBindingOffset, missShaderBindingStride,
                                            hitShaderBindingTableBuffer, hitShaderBindingOffset, hitShaderBindingStride,
                                            callableShaderBindingTableBuffer, callableShaderBindingOffset,
                                            callableShaderBindingStride, width, height, depth, record_obj);

    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    if (!cb_state) {
        InternalError(commandBuffer, record_obj.location, "Unrecognized command buffer.");
        return;
    }
    PostCallSetupShaderInstrumentationResources(*this, *cb_state, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, record_obj.location);
}

void Validator::PreCallRecordCmdTraceRaysKHR(VkCommandBuffer commandBuffer,
                                             const VkStridedDeviceAddressRegionKHR *pRaygenShaderBindingTable,
                                             const VkStridedDeviceAddressRegionKHR *pMissShaderBindingTable,
                                             const VkStridedDeviceAddressRegionKHR *pHitShaderBindingTable,
                                             const VkStridedDeviceAddressRegionKHR *pCallableShaderBindingTable, uint32_t width,
                                             uint32_t height, uint32_t depth, const RecordObject &record_obj) {
    BaseClass::PreCallRecordCmdTraceRaysKHR(commandBuffer, pRaygenShaderBindingTable, pMissShaderBindingTable,
                                            pHitShaderBindingTable, pCallableShaderBindingTable, width, height, depth, record_obj);

    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    if (!cb_state) {
        InternalError(commandBuffer, record_obj.location, "Unrecognized command buffer.");
        return;
    }
    PreCallSetupShaderInstrumentationResources(*this, *cb_state, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, record_obj.location);
}

void Validator::PostCallRecordCmdTraceRaysKHR(VkCommandBuffer commandBuffer,
                                              const VkStridedDeviceAddressRegionKHR *pRaygenShaderBindingTable,
                                              const VkStridedDeviceAddressRegionKHR *pMissShaderBindingTable,
                                              const VkStridedDeviceAddressRegionKHR *pHitShaderBindingTable,
                                              const VkStridedDeviceAddressRegionKHR *pCallableShaderBindingTable, uint32_t width,
                                              uint32_t height, uint32_t depth, const RecordObject &record_obj) {
    BaseClass::PostCallRecordCmdTraceRaysKHR(commandBuffer, pRaygenShaderBindingTable, pMissShaderBindingTable,
                                             pHitShaderBindingTable, pCallableShaderBindingTable, width, height, depth, record_obj);

    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    if (!cb_state) {
        InternalError(commandBuffer, record_obj.location, "Unrecognized command buffer.");
        return;
    }
    PostCallSetupShaderInstrumentationResources(*this, *cb_state, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, record_obj.location);
}

void Validator::PreCallRecordCmdTraceRaysIndirectKHR(VkCommandBuffer commandBuffer,
                                                     const VkStridedDeviceAddressRegionKHR *pRaygenShaderBindingTable,
                                                     const VkStridedDeviceAddressRegionKHR *pMissShaderBindingTable,
                                                     const VkStridedDeviceAddressRegionKHR *pHitShaderBindingTable,
                                                     const VkStridedDeviceAddressRegionKHR *pCallableShaderBindingTable,
                                                     VkDeviceAddress indirectDeviceAddress, const RecordObject &record_obj) {
    BaseClass::PreCallRecordCmdTraceRaysIndirectKHR(commandBuffer, pRaygenShaderBindingTable, pMissShaderBindingTable,
                                                    pHitShaderBindingTable, pCallableShaderBindingTable, indirectDeviceAddress,
                                                    record_obj);

    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    if (!cb_state) {
        InternalError(commandBuffer, record_obj.location, "Unrecognized command buffer.");
        return;
    }
    InsertIndirectTraceRaysValidation(*this, record_obj.location, *cb_state, indirectDeviceAddress);
    PreCallSetupShaderInstrumentationResources(*this, *cb_state, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, record_obj.location);
}

void Validator::PostCallRecordCmdTraceRaysIndirectKHR(VkCommandBuffer commandBuffer,
                                                      const VkStridedDeviceAddressRegionKHR *pRaygenShaderBindingTable,
                                                      const VkStridedDeviceAddressRegionKHR *pMissShaderBindingTable,
                                                      const VkStridedDeviceAddressRegionKHR *pHitShaderBindingTable,
                                                      const VkStridedDeviceAddressRegionKHR *pCallableShaderBindingTable,
                                                      VkDeviceAddress indirectDeviceAddress, const RecordObject &record_obj) {
    BaseClass::PostCallRecordCmdTraceRaysIndirectKHR(commandBuffer, pRaygenShaderBindingTable, pMissShaderBindingTable,
                                                     pHitShaderBindingTable, pCallableShaderBindingTable, indirectDeviceAddress,
                                                     record_obj);

    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    if (!cb_state) {
        InternalError(commandBuffer, record_obj.location, "Unrecognized command buffer.");
        return;
    }
    PostCallSetupShaderInstrumentationResources(*this, *cb_state, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, record_obj.location);
}

void Validator::PreCallRecordCmdTraceRaysIndirect2KHR(VkCommandBuffer commandBuffer, VkDeviceAddress indirectDeviceAddress,
                                                      const RecordObject &record_obj) {
    BaseClass::PreCallRecordCmdTraceRaysIndirect2KHR(commandBuffer, indirectDeviceAddress, record_obj);

    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    if (!cb_state) {
        InternalError(commandBuffer, record_obj.location, "Unrecognized command buffer.");
        return;
    }
    PreCallSetupShaderInstrumentationResources(*this, *cb_state, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, record_obj.location);
}

void Validator::PostCallRecordCmdTraceRaysIndirect2KHR(VkCommandBuffer commandBuffer, VkDeviceAddress indirectDeviceAddress,
                                                       const RecordObject &record_obj) {
    BaseClass::PostCallRecordCmdTraceRaysIndirect2KHR(commandBuffer, indirectDeviceAddress, record_obj);

    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    if (!cb_state) {
        InternalError(commandBuffer, record_obj.location, "Unrecognized command buffer.");
        return;
    }
    PostCallSetupShaderInstrumentationResources(*this, *cb_state, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, record_obj.location);
}

void Validator::PreCallRecordCmdExecuteGeneratedCommandsEXT(VkCommandBuffer commandBuffer, VkBool32 isPreprocessed,
                                                            const VkGeneratedCommandsInfoEXT *pGeneratedCommandsInfo,
                                                            const RecordObject &record_obj) {
    BaseClass::PreCallRecordCmdExecuteGeneratedCommandsEXT(commandBuffer, isPreprocessed, pGeneratedCommandsInfo, record_obj);

    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    if (!cb_state) {
        InternalError(commandBuffer, record_obj.location, "Unrecognized command buffer.");
        return;
    }
    const VkPipelineBindPoint bind_point = ConvertToPipelineBindPoint(pGeneratedCommandsInfo->shaderStages);
    PreCallSetupShaderInstrumentationResources(*this, *cb_state, bind_point, record_obj.location);
};

void Validator::PostCallRecordCmdExecuteGeneratedCommandsEXT(VkCommandBuffer commandBuffer, VkBool32 isPreprocessed,
                                                             const VkGeneratedCommandsInfoEXT *pGeneratedCommandsInfo,
                                                             const RecordObject &record_obj) {
    BaseClass::PostCallRecordCmdExecuteGeneratedCommandsEXT(commandBuffer, isPreprocessed, pGeneratedCommandsInfo, record_obj);

    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    if (!cb_state) {
        InternalError(commandBuffer, record_obj.location, "Unrecognized command buffer.");
        return;
    }
    const VkPipelineBindPoint bind_point = ConvertToPipelineBindPoint(pGeneratedCommandsInfo->shaderStages);
    PostCallSetupShaderInstrumentationResources(*this, *cb_state, bind_point, record_obj.location);
}

}  // namespace gpuav
