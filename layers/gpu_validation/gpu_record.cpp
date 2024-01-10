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
#if defined(__linux__) || defined(__FreeBSD__) || defined(__OpenBSD__)
#include <unistd.h>
#endif
#include "utils/cast_utils.h"
#include "utils/shader_utils.h"
#include "utils/hash_util.h"
#include "gpu_validation/gpu_validation.h"
#include "spirv-tools/instrument.hpp"
#include "spirv-tools/linker.hpp"
#include "generated/layer_chassis_dispatch.h"

// Generated shaders
#include "generated/gpu_inst_shader_hash.h"

void gpuav::Validator::PreCallRecordCreateBuffer(VkDevice device, const VkBufferCreateInfo *pCreateInfo,
                                                 const VkAllocationCallbacks *pAllocator, VkBuffer *pBuffer,
                                                 const RecordObject &record_obj, void *cb_state_data) {
    create_buffer_api_state *cb_state = reinterpret_cast<create_buffer_api_state *>(cb_state_data);
    if (cb_state) {
        // Ray tracing acceleration structure instance buffers also need the storage buffer usage as
        // acceleration structure build validation will find and replace invalid acceleration structure
        // handles inside of a compute shader.
        if (cb_state->modified_create_info.usage & VK_BUFFER_USAGE_RAY_TRACING_BIT_NV) {
            cb_state->modified_create_info.usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        }

        // Indirect buffers will require validation shader to bind the indirect buffers as a storage buffer.
        if (gpuav_settings.validate_indirect_buffer &&
            cb_state->modified_create_info.usage & VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT) {
            cb_state->modified_create_info.usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        }
    }

    BaseClass::PreCallRecordCreateBuffer(device, pCreateInfo, pAllocator, pBuffer, record_obj, cb_state_data);
}

void gpuav::Validator::PreCallRecordCmdBuildAccelerationStructureNV(VkCommandBuffer commandBuffer,
                                                                    const VkAccelerationStructureInfoNV *pInfo,
                                                                    VkBuffer instanceData, VkDeviceSize instanceOffset,
                                                                    VkBool32 update, VkAccelerationStructureNV dst,
                                                                    VkAccelerationStructureNV src, VkBuffer scratch,
                                                                    VkDeviceSize scratchOffset, const RecordObject &record_obj) {
    BaseClass::PreCallRecordCmdBuildAccelerationStructureNV(commandBuffer, pInfo, instanceData, instanceOffset, update, dst, src,
                                                            scratch, scratchOffset, record_obj);
    if (pInfo == nullptr || pInfo->type != VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_NV) {
        return;
    }

    auto &as_validation_state = acceleration_structure_validation_state;
    if (!as_validation_state.initialized) {
        return;
    }

    // Empty acceleration structure is valid according to the spec.
    if (pInfo->instanceCount == 0 || instanceData == VK_NULL_HANDLE) {
        return;
    }

    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    assert(cb_state != nullptr);

    std::vector<uint64_t> current_valid_handles;
    ForEach<vvl::AccelerationStructureNV>([&current_valid_handles](const vvl::AccelerationStructureNV &as_state) {
        if (as_state.built && as_state.create_infoNV.info.type == VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV) {
            current_valid_handles.push_back(as_state.opaque_handle);
        }
    });

    AccelerationStructureBuildValidationInfo as_validation_info = {};
    as_validation_info.acceleration_structure = dst;

    const VkDeviceSize validation_buffer_size =
        // One uint for number of instances to validate
        4 +
        // Two uint for the replacement acceleration structure handle
        8 +
        // One uint for number of invalid handles found
        4 +
        // Two uint for the first invalid handle found
        8 +
        // One uint for the number of current valid handles
        4 +
        // Two uint for each current valid handle
        (8 * current_valid_handles.size());

    VkBufferCreateInfo validation_buffer_create_info = vku::InitStructHelper();
    validation_buffer_create_info.size = validation_buffer_size;
    validation_buffer_create_info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

    VmaAllocationCreateInfo validation_buffer_alloc_info = {};
    validation_buffer_alloc_info.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;

    VkResult result = vmaCreateBuffer(vmaAllocator, &validation_buffer_create_info, &validation_buffer_alloc_info,
                                      &as_validation_info.buffer, &as_validation_info.buffer_allocation, nullptr);
    if (result != VK_SUCCESS) {
        ReportSetupProblem(device, "Unable to allocate device memory. Device could become unstable.");
        aborted = true;
        return;
    }

    glsl::AccelerationStructureBuildValidationBuffer *mapped_validation_buffer = nullptr;
    result = vmaMapMemory(vmaAllocator, as_validation_info.buffer_allocation, reinterpret_cast<void **>(&mapped_validation_buffer));
    if (result != VK_SUCCESS) {
        ReportSetupProblem(device, "Unable to map device memory for acceleration structure build validation buffer.");
        aborted = true;
        return;
    }

    mapped_validation_buffer->instances_to_validate = pInfo->instanceCount;
    {
        const auto replacement_as_handle = vvl_bit_cast<std::array<uint32_t, 2>>(as_validation_state.replacement_as_handle);
        mapped_validation_buffer->replacement_handle_bits_0 = replacement_as_handle[0];
        mapped_validation_buffer->replacement_handle_bits_1 = replacement_as_handle[1];
    }
    mapped_validation_buffer->invalid_handle_found = 0;
    mapped_validation_buffer->invalid_handle_bits_0 = 0;
    mapped_validation_buffer->invalid_handle_bits_1 = 0;
    mapped_validation_buffer->valid_handles_count = static_cast<uint32_t>(current_valid_handles.size());

    uint32_t *mapped_valid_handles = reinterpret_cast<uint32_t *>(&mapped_validation_buffer[1]);
    for (std::size_t i = 0; i < current_valid_handles.size(); i++) {
        const auto current_valid_handle = vvl_bit_cast<std::array<uint32_t, 2>>(current_valid_handles[i]);

        *mapped_valid_handles = current_valid_handle[0];
        ++mapped_valid_handles;
        *mapped_valid_handles = current_valid_handle[1];
        ++mapped_valid_handles;
    }

    vmaUnmapMemory(vmaAllocator, as_validation_info.buffer_allocation);

    static constexpr const VkDeviceSize k_instance_size = 64;
    const VkDeviceSize instance_buffer_size = k_instance_size * pInfo->instanceCount;

    result = desc_set_manager->GetDescriptorSet(&as_validation_info.descriptor_pool, debug_desc_layout,
                                                &as_validation_info.descriptor_set);
    if (result != VK_SUCCESS) {
        ReportSetupProblem(device, "Unable to get descriptor set for acceleration structure build.");
        aborted = true;
        return;
    }

    VkDescriptorBufferInfo descriptor_buffer_infos[2] = {};
    descriptor_buffer_infos[0].buffer = instanceData;
    descriptor_buffer_infos[0].offset = instanceOffset;
    descriptor_buffer_infos[0].range = instance_buffer_size;
    descriptor_buffer_infos[1].buffer = as_validation_info.buffer;
    descriptor_buffer_infos[1].offset = 0;
    descriptor_buffer_infos[1].range = validation_buffer_size;

    VkWriteDescriptorSet descriptor_set_writes[2] = {
        vku::InitStruct<VkWriteDescriptorSet>(),
        vku::InitStruct<VkWriteDescriptorSet>(),
    };
    descriptor_set_writes[0].dstSet = as_validation_info.descriptor_set;
    descriptor_set_writes[0].dstBinding = 0;
    descriptor_set_writes[0].descriptorCount = 1;
    descriptor_set_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptor_set_writes[0].pBufferInfo = &descriptor_buffer_infos[0];
    descriptor_set_writes[1].dstSet = as_validation_info.descriptor_set;
    descriptor_set_writes[1].dstBinding = 1;
    descriptor_set_writes[1].descriptorCount = 1;
    descriptor_set_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptor_set_writes[1].pBufferInfo = &descriptor_buffer_infos[1];

    DispatchUpdateDescriptorSets(device, 2, descriptor_set_writes, 0, nullptr);

    // Issue a memory barrier to make sure anything writing to the instance buffer has finished.
    VkMemoryBarrier memory_barrier = vku::InitStructHelper();
    memory_barrier.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
    memory_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    DispatchCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 1,
                               &memory_barrier, 0, nullptr, 0, nullptr);

    // Save a copy of the compute pipeline state that needs to be restored.
    RestorablePipelineState restorable_state(cb_state.get(), VK_PIPELINE_BIND_POINT_COMPUTE);

    // Switch to and launch the validation compute shader to find, replace, and report invalid acceleration structure handles.
    DispatchCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, as_validation_state.pipeline);
    DispatchCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, as_validation_state.pipeline_layout, 0, 1,
                                  &as_validation_info.descriptor_set, 0, nullptr);
    DispatchCmdDispatch(commandBuffer, 1, 1, 1);

    // Issue a buffer memory barrier to make sure that any invalid bottom level acceleration structure handles
    // have been replaced by the validation compute shader before any builds take place.
    VkBufferMemoryBarrier instance_buffer_barrier = vku::InitStructHelper();
    instance_buffer_barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    instance_buffer_barrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV;
    instance_buffer_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    instance_buffer_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    instance_buffer_barrier.buffer = instanceData;
    instance_buffer_barrier.offset = instanceOffset;
    instance_buffer_barrier.size = instance_buffer_size;
    DispatchCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                               VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV, 0, 0, nullptr, 1, &instance_buffer_barrier, 0,
                               nullptr);

    // Restore the previous compute pipeline state.
    restorable_state.Restore(commandBuffer);

    cb_state->as_validation_buffers.emplace_back(std::move(as_validation_info));
}

void gpuav::Validator::PostCallRecordBindAccelerationStructureMemoryNV(VkDevice device, uint32_t bindInfoCount,
                                                                       const VkBindAccelerationStructureMemoryInfoNV *pBindInfos,
                                                                       const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;
    BaseClass::PostCallRecordBindAccelerationStructureMemoryNV(device, bindInfoCount, pBindInfos, record_obj);
    for (uint32_t i = 0; i < bindInfoCount; i++) {
        const VkBindAccelerationStructureMemoryInfoNV &info = pBindInfos[i];
        auto as_state = Get<vvl::AccelerationStructureNV>(info.accelerationStructure);
        if (as_state) {
            DispatchGetAccelerationStructureHandleNV(device, info.accelerationStructure, 8, &as_state->opaque_handle);
        }
    }
}

void gpuav::Validator::PostCallRecordGetPhysicalDeviceProperties(VkPhysicalDevice physicalDevice,
                                                                 VkPhysicalDeviceProperties *device_props,
                                                                 const RecordObject &record_obj) {
    // There is an implicit layer that can cause this call to return 0 for maxBoundDescriptorSets - Ignore such calls
    if (enabled[gpu_validation_reserve_binding_slot] && device_props->limits.maxBoundDescriptorSets > 0) {
        if (device_props->limits.maxBoundDescriptorSets > 1) {
            device_props->limits.maxBoundDescriptorSets -= 1;
        } else {
            LogWarning("WARNING-GPU-Assisted-Validation-Setup", physicalDevice, record_obj.location,
                       "Unable to reserve descriptor binding slot on a device with only one slot.");
        }
    }

    BaseClass::PostCallRecordGetPhysicalDeviceProperties(physicalDevice, device_props, record_obj);
}

void gpuav::Validator::PostCallRecordGetPhysicalDeviceProperties2(VkPhysicalDevice physicalDevice,
                                                                  VkPhysicalDeviceProperties2 *device_props2,
                                                                  const RecordObject &record_obj) {
    // There is an implicit layer that can cause this call to return 0 for maxBoundDescriptorSets - Ignore such calls
    if (enabled[gpu_validation_reserve_binding_slot] && device_props2->properties.limits.maxBoundDescriptorSets > 0) {
        if (device_props2->properties.limits.maxBoundDescriptorSets > 1) {
            device_props2->properties.limits.maxBoundDescriptorSets -= 1;
        } else {
            LogWarning("WARNING-GPU-Assisted-Validation-Setup", physicalDevice, record_obj.location,
                       "Unable to reserve descriptor binding slot on a device with only one slot.");
        }
    }
    // override all possible places maxUpdateAfterBindDescriptorsInAllPools can be set
    auto *desc_indexing_props = vku::FindStructInPNextChain<VkPhysicalDeviceDescriptorIndexingProperties>(device_props2->pNext);
    if (desc_indexing_props &&
        desc_indexing_props->maxUpdateAfterBindDescriptorsInAllPools > glsl::kDebugInputBindlessMaxDescSets) {
        desc_indexing_props->maxUpdateAfterBindDescriptorsInAllPools = glsl::kDebugInputBindlessMaxDescSets;
    }

    auto *vk12_props = vku::FindStructInPNextChain<VkPhysicalDeviceVulkan12Properties>(device_props2->pNext);
    if (vk12_props && vk12_props->maxUpdateAfterBindDescriptorsInAllPools > glsl::kDebugInputBindlessMaxDescSets) {
        vk12_props->maxUpdateAfterBindDescriptorsInAllPools = glsl::kDebugInputBindlessMaxDescSets;
    }

    BaseClass::PostCallRecordGetPhysicalDeviceProperties2(physicalDevice, device_props2, record_obj);
}

void gpuav::Validator::PreCallRecordDestroyRenderPass(VkDevice device, VkRenderPass renderPass,
                                                      const VkAllocationCallbacks *pAllocator, const RecordObject &record_obj) {
    auto pipeline = common_draw_resources.renderpass_to_pipeline.pop(renderPass);
    if (pipeline != common_draw_resources.renderpass_to_pipeline.end()) {
        DispatchDestroyPipeline(device, pipeline->second, nullptr);
    }
    BaseClass::PreCallRecordDestroyRenderPass(device, renderPass, pAllocator, record_obj);
}

// Create the instrumented shader data to provide to the driver.
void gpuav::Validator::PreCallRecordCreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo *pCreateInfo,
                                                       const VkAllocationCallbacks *pAllocator, VkShaderModule *pShaderModule,
                                                       const RecordObject &record_obj, void *csm_state_data) {
    BaseClass::PreCallRecordCreateShaderModule(device, pCreateInfo, pAllocator, pShaderModule, record_obj, csm_state_data);
    create_shader_module_api_state *csm_state = static_cast<create_shader_module_api_state *>(csm_state_data);
    if (gpuav_settings.select_instrumented_shaders && !CheckForGpuAvEnabled(pCreateInfo->pNext)) return;
    uint32_t shader_id;
    if (gpuav_settings.cache_instrumented_shaders) {
        const uint32_t shader_hash = hash_util::ShaderHash(pCreateInfo->pCode, pCreateInfo->codeSize);
        if (gpuav_settings.cache_instrumented_shaders && CheckForCachedInstrumentedShader(shader_hash, csm_state)) {
            return;
        }
        shader_id = shader_hash;
    } else {
        shader_id = unique_shader_module_id++;
    }
    const bool pass = InstrumentShader(vvl::make_span(pCreateInfo->pCode, pCreateInfo->codeSize / sizeof(uint32_t)),
                                       csm_state->instrumented_spirv, shader_id, record_obj.location);
    if (pass) {
        csm_state->instrumented_create_info.pCode = csm_state->instrumented_spirv.data();
        csm_state->instrumented_create_info.codeSize = csm_state->instrumented_spirv.size() * sizeof(uint32_t);
        csm_state->unique_shader_id = shader_id;
        if (gpuav_settings.cache_instrumented_shaders) {
            instrumented_shaders.emplace(shader_id,
                                         std::make_pair(csm_state->instrumented_spirv.size(), csm_state->instrumented_spirv));
        }
    }
}

void gpuav::Validator::PreCallRecordCreateShadersEXT(VkDevice device, uint32_t createInfoCount,
                                                     const VkShaderCreateInfoEXT *pCreateInfos,
                                                     const VkAllocationCallbacks *pAllocator, VkShaderEXT *pShaders,
                                                     const RecordObject &record_obj, void *csm_state_data) {
    BaseClass::PreCallRecordCreateShadersEXT(device, createInfoCount, pCreateInfos, pAllocator, pShaders, record_obj,
                                             csm_state_data);
    create_shader_object_api_state *csm_state = static_cast<create_shader_object_api_state *>(csm_state_data);
    for (uint32_t i = 0; i < createInfoCount; ++i) {
        if (gpuav_settings.select_instrumented_shaders && !CheckForGpuAvEnabled(pCreateInfos[i].pNext)) continue;
        if (gpuav_settings.cache_instrumented_shaders) {
            const uint32_t shader_hash = hash_util::ShaderHash(pCreateInfos[i].pCode, pCreateInfos[i].codeSize);
            if (CheckForCachedInstrumentedShader(i, csm_state->unique_shader_ids[i], csm_state)) {
                continue;
            }
            csm_state->unique_shader_ids[i] = shader_hash;
        } else {
            csm_state->unique_shader_ids[i] = unique_shader_module_id++;
        }
        const bool pass = InstrumentShader(
            vvl::make_span(static_cast<const uint32_t *>(pCreateInfos[i].pCode), pCreateInfos[i].codeSize / sizeof(uint32_t)),
            csm_state->instrumented_spirv[i], csm_state->unique_shader_ids[i], record_obj.location);
        if (pass) {
            csm_state->instrumented_create_info[i].pCode = csm_state->instrumented_spirv[i].data();
            csm_state->instrumented_create_info[i].codeSize = csm_state->instrumented_spirv[i].size() * sizeof(uint32_t);
            if (gpuav_settings.cache_instrumented_shaders) {
                instrumented_shaders.emplace(
                    csm_state->unique_shader_ids[i],
                    std::make_pair(csm_state->instrumented_spirv[i].size(), csm_state->instrumented_spirv[i]));
            }
        }
    }
}

// Clean up device-related resources
void gpuav::Validator::PreCallRecordDestroyDevice(VkDevice device, const VkAllocationCallbacks *pAllocator,
                                                  const RecordObject &record_obj) {
    desc_heap.reset();
    acceleration_structure_validation_state.Destroy(device, vmaAllocator);
    common_draw_resources.Destroy(device);
    common_dispatch_resources.Destroy(device);
    common_trace_rays_resources.Destroy(device, vmaAllocator);
    if (app_buffer_device_addresses.buffer) {
        vmaDestroyBuffer(vmaAllocator, app_buffer_device_addresses.buffer, app_buffer_device_addresses.allocation);
    }
    if (gpuav_settings.cache_instrumented_shaders && !instrumented_shaders.empty()) {
        std::ofstream file_stream(instrumented_shader_cache_path, std::ofstream::out | std::ofstream::binary);
        if (file_stream) {
            file_stream.write(INST_SHADER_GIT_HASH, sizeof(INST_SHADER_GIT_HASH));
            uint32_t datasize = static_cast<uint32_t>(instrumented_shaders.size());
            file_stream.write(reinterpret_cast<char *>(&datasize), sizeof(uint32_t));
            for (auto &record : instrumented_shaders) {
                // Hash of shader
                file_stream.write(reinterpret_cast<const char *>(&record.first), sizeof(uint32_t));
                // Size of vector of code
                auto vector_size = record.second.first;
                file_stream.write(reinterpret_cast<const char *>(&vector_size), sizeof(uint32_t));
                // Vector contents
                file_stream.write(reinterpret_cast<const char *>(record.second.second.data()), vector_size * sizeof(uint32_t));
            }
            file_stream.close();
        }
    }
    BaseClass::PreCallRecordDestroyDevice(device, pAllocator, record_obj);
}

void gpuav::Validator::RecordCmdBeginRenderPassLayouts(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                                       const VkSubpassContents contents) {
    if (!pRenderPassBegin) {
        return;
    }
    auto cb_state = GetWrite<vvl::CommandBuffer>(commandBuffer);
    auto render_pass_state = Get<vvl::RenderPass>(pRenderPassBegin->renderPass);
    if (cb_state && render_pass_state) {
        // transition attachments to the correct layouts for beginning of renderPass and first subpass
        TransitionBeginRenderPassLayouts(cb_state.get(), *render_pass_state);
    }
}

void gpuav::Validator::PreCallRecordCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                                       VkSubpassContents contents, const RecordObject &record_obj) {
    BaseClass::PreCallRecordCmdBeginRenderPass(commandBuffer, pRenderPassBegin, contents, record_obj);
    RecordCmdBeginRenderPassLayouts(commandBuffer, pRenderPassBegin, contents);
}

void gpuav::Validator::PreCallRecordCmdBeginRenderPass2KHR(VkCommandBuffer commandBuffer,
                                                           const VkRenderPassBeginInfo *pRenderPassBegin,
                                                           const VkSubpassBeginInfo *pSubpassBeginInfo,
                                                           const RecordObject &record_obj) {
    PreCallRecordCmdBeginRenderPass2(commandBuffer, pRenderPassBegin, pSubpassBeginInfo, record_obj);
}

void gpuav::Validator::PreCallRecordCmdBeginRenderPass2(VkCommandBuffer commandBuffer,
                                                        const VkRenderPassBeginInfo *pRenderPassBegin,
                                                        const VkSubpassBeginInfo *pSubpassBeginInfo,
                                                        const RecordObject &record_obj) {
    BaseClass::PreCallRecordCmdBeginRenderPass2(commandBuffer, pRenderPassBegin, pSubpassBeginInfo, record_obj);
    RecordCmdBeginRenderPassLayouts(commandBuffer, pRenderPassBegin, pSubpassBeginInfo->contents);
}

void gpuav::Validator::RecordCmdEndRenderPassLayouts(VkCommandBuffer commandBuffer) {
    auto cb_state = GetWrite<vvl::CommandBuffer>(commandBuffer);
    if (cb_state) {
        TransitionFinalSubpassLayouts(cb_state.get());
    }
}

void gpuav::Validator::PostCallRecordCmdEndRenderPass(VkCommandBuffer commandBuffer, const RecordObject &record_obj) {
    // Record the end at the CoreLevel to ensure StateTracker cleanup doesn't step on anything we need.
    RecordCmdEndRenderPassLayouts(commandBuffer);
    BaseClass::PostCallRecordCmdEndRenderPass(commandBuffer, record_obj);
}

void gpuav::Validator::PostCallRecordCmdEndRenderPass2KHR(VkCommandBuffer commandBuffer, const VkSubpassEndInfo *pSubpassEndInfo,
                                                          const RecordObject &record_obj) {
    PostCallRecordCmdEndRenderPass2(commandBuffer, pSubpassEndInfo, record_obj);
}

void gpuav::Validator::PostCallRecordCmdEndRenderPass2(VkCommandBuffer commandBuffer, const VkSubpassEndInfo *pSubpassEndInfo,
                                                       const RecordObject &record_obj) {
    RecordCmdEndRenderPassLayouts(commandBuffer);
    BaseClass::PostCallRecordCmdEndRenderPass2(commandBuffer, pSubpassEndInfo, record_obj);
}

void gpuav::Validator::RecordCmdNextSubpassLayouts(VkCommandBuffer commandBuffer, VkSubpassContents contents) {
    auto cb_state = GetWrite<vvl::CommandBuffer>(commandBuffer);
    TransitionSubpassLayouts(cb_state.get(), *cb_state->activeRenderPass, cb_state->GetActiveSubpass());
}

void gpuav::Validator::PostCallRecordCmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents,
                                                    const RecordObject &record_obj) {
    BaseClass::PostCallRecordCmdNextSubpass(commandBuffer, contents, record_obj);
    RecordCmdNextSubpassLayouts(commandBuffer, contents);
}

void gpuav::Validator::PostCallRecordCmdNextSubpass2KHR(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo *pSubpassBeginInfo,
                                                        const VkSubpassEndInfo *pSubpassEndInfo, const RecordObject &record_obj) {
    PostCallRecordCmdNextSubpass2(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo, record_obj);
}

void gpuav::Validator::PostCallRecordCmdNextSubpass2(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo *pSubpassBeginInfo,
                                                     const VkSubpassEndInfo *pSubpassEndInfo, const RecordObject &record_obj) {
    BaseClass::PostCallRecordCmdNextSubpass2(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo, record_obj);
    RecordCmdNextSubpassLayouts(commandBuffer, pSubpassBeginInfo->contents);
}

void gpuav::Validator::PostCallRecordCmdBindDescriptorSets(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                           VkPipelineLayout layout, uint32_t firstSet, uint32_t descriptorSetCount,
                                                           const VkDescriptorSet *pDescriptorSets, uint32_t dynamicOffsetCount,
                                                           const uint32_t *pDynamicOffsets, const RecordObject &record_obj) {
    BaseClass::PostCallRecordCmdBindDescriptorSets(commandBuffer, pipelineBindPoint, layout, firstSet, descriptorSetCount,
                                                   pDescriptorSets, dynamicOffsetCount, pDynamicOffsets, record_obj);
    UpdateBoundDescriptors(commandBuffer, pipelineBindPoint);
}
void gpuav::Validator::PostCallRecordCmdBindDescriptorSets2KHR(VkCommandBuffer commandBuffer,
                                                               const VkBindDescriptorSetsInfoKHR *pBindDescriptorSetsInfo,
                                                               const RecordObject &record_obj) {
    BaseClass::PostCallRecordCmdBindDescriptorSets2KHR(commandBuffer, pBindDescriptorSetsInfo, record_obj);

    if (IsStageInPipelineBindPoint(pBindDescriptorSetsInfo->stageFlags, VK_PIPELINE_BIND_POINT_GRAPHICS)) {
        UpdateBoundDescriptors(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS);
    }
    if (IsStageInPipelineBindPoint(pBindDescriptorSetsInfo->stageFlags, VK_PIPELINE_BIND_POINT_COMPUTE)) {
        UpdateBoundDescriptors(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE);
    }
    if (IsStageInPipelineBindPoint(pBindDescriptorSetsInfo->stageFlags, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR)) {
        UpdateBoundDescriptors(commandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR);
    }
}

void gpuav::Validator::PreCallRecordCmdPushDescriptorSetKHR(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                            VkPipelineLayout layout, uint32_t set, uint32_t descriptorWriteCount,
                                                            const VkWriteDescriptorSet *pDescriptorWrites,
                                                            const RecordObject &record_obj) {
    BaseClass::PreCallRecordCmdPushDescriptorSetKHR(commandBuffer, pipelineBindPoint, layout, set, descriptorWriteCount,
                                                    pDescriptorWrites, record_obj);
    UpdateBoundDescriptors(commandBuffer, pipelineBindPoint);
}

void gpuav::Validator::PreCallRecordCmdPushDescriptorSet2KHR(VkCommandBuffer commandBuffer,
                                                             const VkPushDescriptorSetInfoKHR *pPushDescriptorSetInfo,
                                                             const RecordObject &record_obj) {
    BaseClass::PreCallRecordCmdPushDescriptorSet2KHR(commandBuffer, pPushDescriptorSetInfo, record_obj);

    if (IsStageInPipelineBindPoint(pPushDescriptorSetInfo->stageFlags, VK_PIPELINE_BIND_POINT_GRAPHICS)) {
        UpdateBoundDescriptors(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS);
    }
    if (IsStageInPipelineBindPoint(pPushDescriptorSetInfo->stageFlags, VK_PIPELINE_BIND_POINT_COMPUTE)) {
        UpdateBoundDescriptors(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE);
    }
    if (IsStageInPipelineBindPoint(pPushDescriptorSetInfo->stageFlags, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR)) {
        UpdateBoundDescriptors(commandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR);
    }
}

void gpuav::Validator::PreRecordCommandBuffer(VkCommandBuffer command_buffer) {
    auto cb_state = GetWrite<CommandBuffer>(command_buffer);
    UpdateInstrumentationBuffer(cb_state.get());
    for (auto *secondary_cmd_buffer : cb_state->linkedCommandBuffers) {
        auto guard = secondary_cmd_buffer->WriteLock();
        UpdateInstrumentationBuffer(static_cast<CommandBuffer *>(secondary_cmd_buffer));
    }
}

void gpuav::Validator::PreCallRecordQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo *pSubmits, VkFence fence,
                                                const RecordObject &record_obj) {
    BaseClass::PreCallRecordQueueSubmit(queue, submitCount, pSubmits, fence, record_obj);
    for (uint32_t submit_idx = 0; submit_idx < submitCount; submit_idx++) {
        const VkSubmitInfo *submit = &pSubmits[submit_idx];
        for (uint32_t i = 0; i < submit->commandBufferCount; i++) {
            PreRecordCommandBuffer(submit->pCommandBuffers[i]);
        }
    }
    UpdateBDABuffer(app_buffer_device_addresses);
}

void gpuav::Validator::PreCallRecordQueueSubmit2KHR(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2KHR *pSubmits,
                                                    VkFence fence, const RecordObject &record_obj) {
    PreCallRecordQueueSubmit2(queue, submitCount, pSubmits, fence, record_obj);
}

void gpuav::Validator::PreCallRecordQueueSubmit2(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2 *pSubmits, VkFence fence,
                                                 const RecordObject &record_obj) {
    BaseClass::PreCallRecordQueueSubmit2(queue, submitCount, pSubmits, fence, record_obj);
    for (uint32_t submit_idx = 0; submit_idx < submitCount; submit_idx++) {
        const VkSubmitInfo2 *submit = &pSubmits[submit_idx];
        for (uint32_t i = 0; i < submit->commandBufferInfoCount; i++) {
            PreRecordCommandBuffer(submit->pCommandBufferInfos[i].commandBuffer);
        }
    }
    UpdateBDABuffer(app_buffer_device_addresses);
}

void gpuav::Validator::PostCallRecordQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo *pSubmits, VkFence fence,
                                                 const RecordObject &record_obj) {
    BaseClass::PostCallRecordQueueSubmit(queue, submitCount, pSubmits, fence, record_obj);

    if (record_obj.result != VK_SUCCESS) return;

    GlobalImageLayoutMap overlay_image_layout_map;
    // The triply nested for duplicates that in the StateTracker, but avoids the need for two additional callbacks.
    for (uint32_t submit_idx = 0; submit_idx < submitCount; submit_idx++) {
        const VkSubmitInfo *submit = &pSubmits[submit_idx];
        for (uint32_t i = 0; i < submit->commandBufferCount; i++) {
            auto cb_state = GetWrite<vvl::CommandBuffer>(submit->pCommandBuffers[i]);
            if (cb_state) {
                for (auto *secondary_cmd_buffer : cb_state->linkedCommandBuffers) {
                    UpdateCmdBufImageLayouts(*secondary_cmd_buffer);
                }
                UpdateCmdBufImageLayouts(*cb_state);
            }
        }
    }
}

void gpuav::Validator::PostCallRecordQueueSubmit2KHR(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2KHR *pSubmits,
                                                     VkFence fence, const RecordObject &record_obj) {
    PostCallRecordQueueSubmit2(queue, submitCount, pSubmits, fence, record_obj);
}

void gpuav::Validator::PostCallRecordQueueSubmit2(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2 *pSubmits, VkFence fence,
                                                  const RecordObject &record_obj) {
    BaseClass::PostCallRecordQueueSubmit2(queue, submitCount, pSubmits, fence, record_obj);
    if (record_obj.result != VK_SUCCESS) return;

    GlobalImageLayoutMap overlay_image_layout_map;
    // The triply nested for duplicates that in the StateTracker, but avoids the need for two additional callbacks.
    for (uint32_t submit_idx = 0; submit_idx < submitCount; submit_idx++) {
        const VkSubmitInfo2KHR *submit = &pSubmits[submit_idx];
        for (uint32_t i = 0; i < submit->commandBufferInfoCount; i++) {
            auto cb_state = GetWrite<vvl::CommandBuffer>(submit->pCommandBufferInfos[i].commandBuffer);
            if (cb_state) {
                for (auto *secondary_cmd_buffer : cb_state->linkedCommandBuffers) {
                    UpdateCmdBufImageLayouts(*secondary_cmd_buffer);
                }
                UpdateCmdBufImageLayouts(*cb_state);
            }
        }
    }
}

void gpuav::Validator::PreCallRecordCmdBindDescriptorBuffersEXT(VkCommandBuffer commandBuffer, uint32_t bufferCount,
                                                                const VkDescriptorBufferBindingInfoEXT *pBindingInfos,
                                                                const RecordObject &record_obj) {
    BaseClass::PreCallRecordCmdBindDescriptorBuffersEXT(commandBuffer, bufferCount, pBindingInfos, record_obj);
    gpuav_settings.validate_descriptors = false;
}

void gpuav::Validator::PreCallRecordCmdBindDescriptorBufferEmbeddedSamplersEXT(VkCommandBuffer commandBuffer,
                                                                               VkPipelineBindPoint pipelineBindPoint,
                                                                               VkPipelineLayout layout, uint32_t set,
                                                                               const RecordObject &record_obj) {
    BaseClass::PreCallRecordCmdBindDescriptorBufferEmbeddedSamplersEXT(commandBuffer, pipelineBindPoint, layout, set, record_obj);
    gpuav_settings.validate_descriptors = false;
}

void gpuav::Validator::PreCallRecordCmdBindDescriptorBufferEmbeddedSamplers2EXT(
    VkCommandBuffer commandBuffer, const VkBindDescriptorBufferEmbeddedSamplersInfoEXT *pBindDescriptorBufferEmbeddedSamplersInfo,
    const RecordObject &record_obj) {
    BaseClass::PreCallRecordCmdBindDescriptorBufferEmbeddedSamplers2EXT(commandBuffer, pBindDescriptorBufferEmbeddedSamplersInfo,
                                                                        record_obj);
    gpuav_settings.validate_descriptors = false;
}

void gpuav::Validator::PreCallRecordCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount,
                                            uint32_t firstVertex, uint32_t firstInstance, const RecordObject &record_obj) {
    BaseClass::PreCallRecordCmdDraw(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance, record_obj);

    CommandResources cmd_resources =
        AllocateCommandResources(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, record_obj.location.function);
    auto cmd_resources_ptr = std::make_unique<CommandResources>(cmd_resources);
    StoreCommandResources(commandBuffer, std::move(cmd_resources_ptr));
}

void gpuav::Validator::PreCallRecordCmdDrawMultiEXT(VkCommandBuffer commandBuffer, uint32_t drawCount,
                                                    const VkMultiDrawInfoEXT *pVertexInfo, uint32_t instanceCount,
                                                    uint32_t firstInstance, uint32_t stride, const RecordObject &record_obj) {
    BaseClass::PreCallRecordCmdDrawMultiEXT(commandBuffer, drawCount, pVertexInfo, instanceCount, firstInstance, stride,
                                            record_obj);
    for (uint32_t i = 0; i < drawCount; i++) {
        CommandResources cmd_resources =
            AllocateCommandResources(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, record_obj.location.function);
        auto cmd_resources_ptr = std::make_unique<CommandResources>(cmd_resources);
        StoreCommandResources(commandBuffer, std::move(cmd_resources_ptr));
    }
}

void gpuav::Validator::PreCallRecordCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount,
                                                   uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance,
                                                   const RecordObject &record_obj) {
    BaseClass::PreCallRecordCmdDrawIndexed(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance,
                                           record_obj);
    CommandResources cmd_resources =
        AllocateCommandResources(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, record_obj.location.function);
    auto cmd_resources_ptr = std::make_unique<CommandResources>(cmd_resources);
    StoreCommandResources(commandBuffer, std::move(cmd_resources_ptr));
}

void gpuav::Validator::PreCallRecordCmdDrawMultiIndexedEXT(VkCommandBuffer commandBuffer, uint32_t drawCount,
                                                           const VkMultiDrawIndexedInfoEXT *pIndexInfo, uint32_t instanceCount,
                                                           uint32_t firstInstance, uint32_t stride, const int32_t *pVertexOffset,
                                                           const RecordObject &record_obj) {
    BaseClass::PreCallRecordCmdDrawMultiIndexedEXT(commandBuffer, drawCount, pIndexInfo, instanceCount, firstInstance, stride,
                                                   pVertexOffset, record_obj);
    for (uint32_t i = 0; i < drawCount; i++) {
        CommandResources cmd_resources =
            AllocateCommandResources(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, record_obj.location.function);
        auto cmd_resources_ptr = std::make_unique<CommandResources>(cmd_resources);
        StoreCommandResources(commandBuffer, std::move(cmd_resources_ptr));
    }
}

void gpuav::Validator::PreCallRecordCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                    uint32_t count, uint32_t stride, const RecordObject &record_obj) {
    BaseClass::PreCallRecordCmdDrawIndirect(commandBuffer, buffer, offset, count, stride, record_obj);

    auto draw_resources = AllocatePreDrawIndirectValidationResources(record_obj.location.function, commandBuffer, buffer, offset,
                                                                     count, VK_NULL_HANDLE, 0, stride);
    StoreCommandResources(commandBuffer, std::move(draw_resources));
}

void gpuav::Validator::PreCallRecordCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                           uint32_t count, uint32_t stride, const RecordObject &record_obj) {
    BaseClass::PreCallRecordCmdDrawIndexedIndirect(commandBuffer, buffer, offset, count, stride, record_obj);

    auto draw_resources = AllocatePreDrawIndirectValidationResources(record_obj.location.function, commandBuffer, buffer, offset,
                                                                     count, VK_NULL_HANDLE, 0, stride);
    StoreCommandResources(commandBuffer, std::move(draw_resources));
}

void gpuav::Validator::PreCallRecordCmdDrawIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                            VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                            uint32_t maxDrawCount, uint32_t stride,
                                                            const RecordObject &record_obj) {
    PreCallRecordCmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride,
                                      record_obj);
}

void gpuav::Validator::PreCallRecordCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                         VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                         uint32_t maxDrawCount, uint32_t stride, const RecordObject &record_obj) {
    BaseClass::PreCallRecordCmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount,
                                                 stride, record_obj);

    auto draw_resources = AllocatePreDrawIndirectValidationResources(record_obj.location.function, commandBuffer, buffer, offset,
                                                                     maxDrawCount, countBuffer, countBufferOffset, stride);
    StoreCommandResources(commandBuffer, std::move(draw_resources));
}

void gpuav::Validator::PreCallRecordCmdDrawIndirectByteCountEXT(VkCommandBuffer commandBuffer, uint32_t instanceCount,
                                                                uint32_t firstInstance, VkBuffer counterBuffer,
                                                                VkDeviceSize counterBufferOffset, uint32_t counterOffset,
                                                                uint32_t vertexStride, const RecordObject &record_obj) {
    BaseClass::PreCallRecordCmdDrawIndirectByteCountEXT(commandBuffer, instanceCount, firstInstance, counterBuffer,
                                                        counterBufferOffset, counterOffset, vertexStride, record_obj);
    CommandResources cmd_resources =
        AllocateCommandResources(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, record_obj.location.function);
    auto cmd_resources_ptr = std::make_unique<CommandResources>(cmd_resources);
    StoreCommandResources(commandBuffer, std::move(cmd_resources_ptr));
}

void gpuav::Validator::PreCallRecordCmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                   VkDeviceSize offset, VkBuffer countBuffer,
                                                                   VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                                   uint32_t stride, const RecordObject &record_obj) {
    PreCallRecordCmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride,
                                             record_obj);
}

void gpuav::Validator::PreCallRecordCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                                VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                                uint32_t maxDrawCount, uint32_t stride,
                                                                const RecordObject &record_obj) {
    BaseClass::PreCallRecordCmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount,
                                                        stride, record_obj);

    auto draw_resources = AllocatePreDrawIndirectValidationResources(record_obj.location.function, commandBuffer, buffer, offset,
                                                                     maxDrawCount, countBuffer, countBufferOffset, stride);
    StoreCommandResources(commandBuffer, std::move(draw_resources));
}

void gpuav::Validator::PreCallRecordCmdDrawMeshTasksNV(VkCommandBuffer commandBuffer, uint32_t taskCount, uint32_t firstTask,
                                                       const RecordObject &record_obj) {
    ValidationStateTracker::PreCallRecordCmdDrawMeshTasksNV(commandBuffer, taskCount, firstTask, record_obj);
    CommandResources cmd_resources =
        AllocateCommandResources(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, record_obj.location.function);
    auto cmd_resources_ptr = std::make_unique<CommandResources>(cmd_resources);
    StoreCommandResources(commandBuffer, std::move(cmd_resources_ptr));
}

void gpuav::Validator::PreCallRecordCmdDrawMeshTasksIndirectNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                               uint32_t drawCount, uint32_t stride,
                                                               const RecordObject &record_obj) {
    BaseClass::PreCallRecordCmdDrawMeshTasksIndirectNV(commandBuffer, buffer, offset, drawCount, stride, record_obj);
    auto draw_resources = AllocatePreDrawIndirectValidationResources(record_obj.location.function, commandBuffer, buffer, offset,
                                                                     drawCount, VK_NULL_HANDLE, 0, stride);
    StoreCommandResources(commandBuffer, std::move(draw_resources));
}

void gpuav::Validator::PreCallRecordCmdDrawMeshTasksIndirectCountNV(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                    VkDeviceSize offset, VkBuffer countBuffer,
                                                                    VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                                    uint32_t stride, const RecordObject &record_obj) {
    BaseClass::PreCallRecordCmdDrawMeshTasksIndirectCountNV(commandBuffer, buffer, offset, countBuffer, countBufferOffset,
                                                            maxDrawCount, stride, record_obj);
    auto draw_resources = AllocatePreDrawIndirectValidationResources(record_obj.location.function, commandBuffer, buffer, offset,
                                                                     maxDrawCount, countBuffer, countBufferOffset, stride);
    StoreCommandResources(commandBuffer, std::move(draw_resources));
}

void gpuav::Validator::PreCallRecordCmdDrawMeshTasksEXT(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY,
                                                        uint32_t groupCountZ, const RecordObject &record_obj) {
    BaseClass::PreCallRecordCmdDrawMeshTasksEXT(commandBuffer, groupCountX, groupCountY, groupCountZ, record_obj);
    CommandResources cmd_resources =
        AllocateCommandResources(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, record_obj.location.function);
    auto cmd_resources_ptr = std::make_unique<CommandResources>(cmd_resources);
    StoreCommandResources(commandBuffer, std::move(cmd_resources_ptr));
}

void gpuav::Validator::PreCallRecordCmdDrawMeshTasksIndirectEXT(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                                uint32_t drawCount, uint32_t stride,
                                                                const RecordObject &record_obj) {
    BaseClass::PreCallRecordCmdDrawMeshTasksIndirectEXT(commandBuffer, buffer, offset, drawCount, stride, record_obj);
    auto draw_resources = AllocatePreDrawIndirectValidationResources(record_obj.location.function, commandBuffer, buffer, offset,
                                                                     drawCount, VK_NULL_HANDLE, 0, stride);
    StoreCommandResources(commandBuffer, std::move(draw_resources));
}

void gpuav::Validator::PreCallRecordCmdDrawMeshTasksIndirectCountEXT(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                     VkDeviceSize offset, VkBuffer countBuffer,
                                                                     VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                                     uint32_t stride, const RecordObject &record_obj) {
    BaseClass::PreCallRecordCmdDrawMeshTasksIndirectCountEXT(commandBuffer, buffer, offset, countBuffer, countBufferOffset,
                                                             maxDrawCount, stride, record_obj);
    auto draw_resources = AllocatePreDrawIndirectValidationResources(record_obj.location.function, commandBuffer, buffer, offset,
                                                                     maxDrawCount, countBuffer, countBufferOffset, stride);
    StoreCommandResources(commandBuffer, std::move(draw_resources));
}

void gpuav::Validator::PreCallRecordCmdDispatch(VkCommandBuffer commandBuffer, uint32_t x, uint32_t y, uint32_t z,
                                                const RecordObject &record_obj) {
    BaseClass::PreCallRecordCmdDispatch(commandBuffer, x, y, z, record_obj);
    CommandResources cmd_resources =
        AllocateCommandResources(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, record_obj.location.function);
    auto cmd_resources_ptr = std::make_unique<CommandResources>(cmd_resources);
    StoreCommandResources(commandBuffer, std::move(cmd_resources_ptr));
}

void gpuav::Validator::PreCallRecordCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                        const RecordObject &record_obj) {
    BaseClass::PreCallRecordCmdDispatchIndirect(commandBuffer, buffer, offset, record_obj);

    auto dispatch_info =
        AllocatePreDispatchIndirectValidationResources(record_obj.location.function, commandBuffer, buffer, offset);
    StoreCommandResources(commandBuffer, std::move(dispatch_info));
}

void gpuav::Validator::PreCallRecordCmdDispatchBase(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY,
                                                    uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY,
                                                    uint32_t groupCountZ, const RecordObject &record_obj) {
    BaseClass::PreCallRecordCmdDispatchBase(commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY,
                                            groupCountZ, record_obj);

    CommandResources cmd_resources =
        AllocateCommandResources(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, record_obj.location.function);
    auto cmd_resources_ptr = std::make_unique<CommandResources>(cmd_resources);
    StoreCommandResources(commandBuffer, std::move(cmd_resources_ptr));
}

void gpuav::Validator::PreCallRecordCmdDispatchBaseKHR(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY,
                                                       uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY,
                                                       uint32_t groupCountZ, const RecordObject &record_obj) {
    PreCallRecordCmdDispatchBase(commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ,
                                 record_obj);
}

void gpuav::Validator::PreCallRecordCmdTraceRaysNV(VkCommandBuffer commandBuffer, VkBuffer raygenShaderBindingTableBuffer,
                                                   VkDeviceSize raygenShaderBindingOffset, VkBuffer missShaderBindingTableBuffer,
                                                   VkDeviceSize missShaderBindingOffset, VkDeviceSize missShaderBindingStride,
                                                   VkBuffer hitShaderBindingTableBuffer, VkDeviceSize hitShaderBindingOffset,
                                                   VkDeviceSize hitShaderBindingStride, VkBuffer callableShaderBindingTableBuffer,
                                                   VkDeviceSize callableShaderBindingOffset,
                                                   VkDeviceSize callableShaderBindingStride, uint32_t width, uint32_t height,
                                                   uint32_t depth, const RecordObject &record_obj) {
    BaseClass::PreCallRecordCmdTraceRaysNV(commandBuffer, raygenShaderBindingTableBuffer, raygenShaderBindingOffset,
                                           missShaderBindingTableBuffer, missShaderBindingOffset, missShaderBindingStride,
                                           hitShaderBindingTableBuffer, hitShaderBindingOffset, hitShaderBindingStride,
                                           callableShaderBindingTableBuffer, callableShaderBindingOffset,
                                           callableShaderBindingStride, width, height, depth, record_obj);

    CommandResources cmd_resources =
        AllocateCommandResources(commandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, record_obj.location.function);
    auto cmd_resources_ptr = std::make_unique<CommandResources>(cmd_resources);
    StoreCommandResources(commandBuffer, std::move(cmd_resources_ptr));
}

void gpuav::Validator::PreCallRecordCmdTraceRaysKHR(VkCommandBuffer commandBuffer,
                                                    const VkStridedDeviceAddressRegionKHR *pRaygenShaderBindingTable,
                                                    const VkStridedDeviceAddressRegionKHR *pMissShaderBindingTable,
                                                    const VkStridedDeviceAddressRegionKHR *pHitShaderBindingTable,
                                                    const VkStridedDeviceAddressRegionKHR *pCallableShaderBindingTable,
                                                    uint32_t width, uint32_t height, uint32_t depth,
                                                    const RecordObject &record_obj) {
    BaseClass::PreCallRecordCmdTraceRaysKHR(commandBuffer, pRaygenShaderBindingTable, pMissShaderBindingTable,
                                            pHitShaderBindingTable, pCallableShaderBindingTable, width, height, depth, record_obj);

    CommandResources cmd_resources =
        AllocateCommandResources(commandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, record_obj.location.function);
    auto cmd_resources_ptr = std::make_unique<CommandResources>(cmd_resources);
    StoreCommandResources(commandBuffer, std::move(cmd_resources_ptr));
}

void gpuav::Validator::PreCallRecordCmdTraceRaysIndirectKHR(VkCommandBuffer commandBuffer,
                                                            const VkStridedDeviceAddressRegionKHR *pRaygenShaderBindingTable,
                                                            const VkStridedDeviceAddressRegionKHR *pMissShaderBindingTable,
                                                            const VkStridedDeviceAddressRegionKHR *pHitShaderBindingTable,
                                                            const VkStridedDeviceAddressRegionKHR *pCallableShaderBindingTable,
                                                            VkDeviceAddress indirectDeviceAddress, const RecordObject &record_obj) {
    BaseClass::PreCallRecordCmdTraceRaysIndirectKHR(commandBuffer, pRaygenShaderBindingTable, pMissShaderBindingTable,
                                                    pHitShaderBindingTable, pCallableShaderBindingTable, indirectDeviceAddress,
                                                    record_obj);

    auto trace_rays_info =
        AllocatePreTraceRaysValidationResources(record_obj.location.function, commandBuffer, indirectDeviceAddress);
    StoreCommandResources(commandBuffer, std::move(trace_rays_info));
}

void gpuav::Validator::PreCallRecordCmdTraceRaysIndirect2KHR(VkCommandBuffer commandBuffer, VkDeviceAddress indirectDeviceAddress,
                                                             const RecordObject &record_obj) {
    BaseClass::PreCallRecordCmdTraceRaysIndirect2KHR(commandBuffer, indirectDeviceAddress, record_obj);

    CommandResources cmd_resources =
        AllocateCommandResources(commandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, record_obj.location.function);
    auto cmd_resources_ptr = std::make_unique<CommandResources>(cmd_resources);
    StoreCommandResources(commandBuffer, std::move(cmd_resources_ptr));
}
