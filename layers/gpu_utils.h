/* Copyright (c) 2020-2021 The Khronos Group Inc.
 * Copyright (c) 2020-2021 Valve Corporation
 * Copyright (c) 2020-2021 LunarG, Inc.
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
 * Author: Tony Barbour <tony@lunarg.com>
 */
#pragma once
#include "chassis.h"
#include "shader_validation.h"
class UtilDescriptorSetManager {
  public:
    UtilDescriptorSetManager(VkDevice device, uint32_t numBindingsInSet);
    ~UtilDescriptorSetManager();

    VkResult GetDescriptorSet(VkDescriptorPool *desc_pool, VkDescriptorSetLayout ds_layout, VkDescriptorSet *desc_sets);
    VkResult GetDescriptorSets(uint32_t count, VkDescriptorPool *pool, VkDescriptorSetLayout ds_layout,
                               std::vector<VkDescriptorSet> *desc_sets);
    void PutBackDescriptorSet(VkDescriptorPool desc_pool, VkDescriptorSet desc_set);

  private:
    static const uint32_t kItemsPerChunk = 512;
    struct PoolTracker {
        uint32_t size;
        uint32_t used;
    };
    VkDevice device;
    uint32_t numBindingsInSet;
    std::unordered_map<VkDescriptorPool, struct PoolTracker> desc_pool_map_;
};
struct UtilQueueBarrierCommandInfo {
    VkCommandPool barrier_command_pool = VK_NULL_HANDLE;
    VkCommandBuffer barrier_command_buffer = VK_NULL_HANDLE;
};
VkResult UtilInitializeVma(VkPhysicalDevice physical_device, VkDevice device, VmaAllocator *pAllocator);
void UtilPreCallRecordCreateDevice(VkPhysicalDevice gpu, safe_VkDeviceCreateInfo *modified_create_info,
                                   VkPhysicalDeviceFeatures supported_features, VkPhysicalDeviceFeatures desired_features);
template <typename ObjectType>
void UtilPostCallRecordCreateDevice(const VkDeviceCreateInfo *pCreateInfo, std::vector<VkDescriptorSetLayoutBinding> bindings,
                                    ObjectType *object_ptr, VkPhysicalDeviceProperties physical_device_properties) {
    // If api version 1.1 or later, SetDeviceLoaderData will be in the loader
    auto chain_info = get_chain_info(pCreateInfo, VK_LOADER_DATA_CALLBACK);
    assert(chain_info->u.pfnSetDeviceLoaderData);
    object_ptr->vkSetDeviceLoaderData = chain_info->u.pfnSetDeviceLoaderData;

    // Some devices have extremely high limits here, so set a reasonable max because we have to pad
    // the pipeline layout with dummy descriptor set layouts.
    object_ptr->adjusted_max_desc_sets = physical_device_properties.limits.maxBoundDescriptorSets;
    object_ptr->adjusted_max_desc_sets = std::min(33U, object_ptr->adjusted_max_desc_sets);

    // We can't do anything if there is only one.
    // Device probably not a legit Vulkan device, since there should be at least 4. Protect ourselves.
    if (object_ptr->adjusted_max_desc_sets == 1) {
        object_ptr->ReportSetupProblem(object_ptr->device, "Device can bind only a single descriptor set.");
        object_ptr->aborted = true;
        return;
    }
    object_ptr->desc_set_bind_index = object_ptr->adjusted_max_desc_sets - 1;

    VkResult result1 = UtilInitializeVma(object_ptr->physicalDevice, object_ptr->device, &object_ptr->vmaAllocator);
    assert(result1 == VK_SUCCESS);
    std::unique_ptr<UtilDescriptorSetManager> desc_set_manager(
        new UtilDescriptorSetManager(object_ptr->device, static_cast<uint32_t>(bindings.size())));

    const VkDescriptorSetLayoutCreateInfo debug_desc_layout_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO, NULL, 0,
                                                                    static_cast<uint32_t>(bindings.size()), bindings.data()};

    const VkDescriptorSetLayoutCreateInfo dummy_desc_layout_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO, NULL, 0, 0,
                                                                    NULL};

    result1 = DispatchCreateDescriptorSetLayout(object_ptr->device, &debug_desc_layout_info, NULL, &object_ptr->debug_desc_layout);

    // This is a layout used to "pad" a pipeline layout to fill in any gaps to the selected bind index.
    VkResult result2 =
        DispatchCreateDescriptorSetLayout(object_ptr->device, &dummy_desc_layout_info, NULL, &object_ptr->dummy_desc_layout);

    assert((result1 == VK_SUCCESS) && (result2 == VK_SUCCESS));
    if ((result1 != VK_SUCCESS) || (result2 != VK_SUCCESS)) {
        object_ptr->ReportSetupProblem(object_ptr->device, "Unable to create descriptor set layout.");
        if (result1 == VK_SUCCESS) {
            DispatchDestroyDescriptorSetLayout(object_ptr->device, object_ptr->debug_desc_layout, NULL);
        }
        if (result2 == VK_SUCCESS) {
            DispatchDestroyDescriptorSetLayout(object_ptr->device, object_ptr->dummy_desc_layout, NULL);
        }
        object_ptr->debug_desc_layout = VK_NULL_HANDLE;
        object_ptr->dummy_desc_layout = VK_NULL_HANDLE;
        object_ptr->aborted = true;
        return;
    }
    object_ptr->desc_set_manager = std::move(desc_set_manager);

    // Register callback to be called at any ResetCommandBuffer time
    object_ptr->SetCommandBufferResetCallback(
        [object_ptr](VkCommandBuffer command_buffer) -> void { object_ptr->ResetCommandBuffer(command_buffer); });
}
template <typename ObjectType>
void UtilPreCallRecordDestroyDevice(ObjectType *object_ptr) {
    for (auto &queue_barrier_command_info_kv : object_ptr->queue_barrier_command_infos) {
        UtilQueueBarrierCommandInfo &queue_barrier_command_info = queue_barrier_command_info_kv.second;

        DispatchFreeCommandBuffers(object_ptr->device, queue_barrier_command_info.barrier_command_pool, 1,
                                   &queue_barrier_command_info.barrier_command_buffer);
        queue_barrier_command_info.barrier_command_buffer = VK_NULL_HANDLE;

        DispatchDestroyCommandPool(object_ptr->device, queue_barrier_command_info.barrier_command_pool, NULL);
        queue_barrier_command_info.barrier_command_pool = VK_NULL_HANDLE;
    }
    object_ptr->queue_barrier_command_infos.clear();
    if (object_ptr->debug_desc_layout) {
        DispatchDestroyDescriptorSetLayout(object_ptr->device, object_ptr->debug_desc_layout, NULL);
        object_ptr->debug_desc_layout = VK_NULL_HANDLE;
    }
    if (object_ptr->dummy_desc_layout) {
        DispatchDestroyDescriptorSetLayout(object_ptr->device, object_ptr->dummy_desc_layout, NULL);
        object_ptr->dummy_desc_layout = VK_NULL_HANDLE;
    }
}

template <typename ObjectType>
void UtilPreCallRecordCreatePipelineLayout(create_pipeline_layout_api_state *cpl_state, ObjectType *object_ptr,
                                           const VkPipelineLayoutCreateInfo *pCreateInfo) {
    // Modify the pipeline layout by:
    // 1. Copying the caller's descriptor set desc_layouts
    // 2. Fill in dummy descriptor layouts up to the max binding
    // 3. Fill in with the debug descriptor layout at the max binding slot
    cpl_state->new_layouts.reserve(object_ptr->adjusted_max_desc_sets);
    cpl_state->new_layouts.insert(cpl_state->new_layouts.end(), &pCreateInfo->pSetLayouts[0],
                                  &pCreateInfo->pSetLayouts[pCreateInfo->setLayoutCount]);
    for (uint32_t i = pCreateInfo->setLayoutCount; i < object_ptr->adjusted_max_desc_sets - 1; ++i) {
        cpl_state->new_layouts.push_back(object_ptr->dummy_desc_layout);
    }
    cpl_state->new_layouts.push_back(object_ptr->debug_desc_layout);
    cpl_state->modified_create_info.pSetLayouts = cpl_state->new_layouts.data();
    cpl_state->modified_create_info.setLayoutCount = object_ptr->adjusted_max_desc_sets;
}

template <typename CreateInfo>
struct CreatePipelineTraits {};
template <>
struct CreatePipelineTraits<VkGraphicsPipelineCreateInfo> {
    using SafeType = safe_VkGraphicsPipelineCreateInfo;
    static const SafeType &GetPipelineCI(const PIPELINE_STATE *pipeline_state) { return pipeline_state->graphicsPipelineCI; }
    static uint32_t GetStageCount(const VkGraphicsPipelineCreateInfo &createInfo) { return createInfo.stageCount; }
    static VkShaderModule GetShaderModule(const VkGraphicsPipelineCreateInfo &createInfo, uint32_t stage) {
        return createInfo.pStages[stage].module;
    }
    static void SetShaderModule(SafeType *createInfo, VkShaderModule shader_module, uint32_t stage) {
        createInfo->pStages[stage].module = shader_module;
    }
};

template <>
struct CreatePipelineTraits<VkComputePipelineCreateInfo> {
    using SafeType = safe_VkComputePipelineCreateInfo;
    static const SafeType &GetPipelineCI(const PIPELINE_STATE *pipeline_state) { return pipeline_state->computePipelineCI; }
    static uint32_t GetStageCount(const VkComputePipelineCreateInfo &createInfo) { return 1; }
    static VkShaderModule GetShaderModule(const VkComputePipelineCreateInfo &createInfo, uint32_t stage) {
        return createInfo.stage.module;
    }
    static void SetShaderModule(SafeType *createInfo, VkShaderModule shader_module, uint32_t stage) {
        assert(stage == 0);
        createInfo->stage.module = shader_module;
    }
};

template <>
struct CreatePipelineTraits<VkRayTracingPipelineCreateInfoNV> {
    using SafeType = safe_VkRayTracingPipelineCreateInfoCommon;
    static const SafeType &GetPipelineCI(const PIPELINE_STATE *pipeline_state) { return pipeline_state->raytracingPipelineCI; }
    static uint32_t GetStageCount(const VkRayTracingPipelineCreateInfoNV &createInfo) { return createInfo.stageCount; }
    static VkShaderModule GetShaderModule(const VkRayTracingPipelineCreateInfoNV &createInfo, uint32_t stage) {
        return createInfo.pStages[stage].module;
    }
    static void SetShaderModule(SafeType *createInfo, VkShaderModule shader_module, uint32_t stage) {
        createInfo->pStages[stage].module = shader_module;
    }
};

template <>
struct CreatePipelineTraits<VkRayTracingPipelineCreateInfoKHR> {
    using SafeType = safe_VkRayTracingPipelineCreateInfoCommon;
    static const SafeType &GetPipelineCI(const PIPELINE_STATE *pipeline_state) { return pipeline_state->raytracingPipelineCI; }
    static uint32_t GetStageCount(const VkRayTracingPipelineCreateInfoKHR &createInfo) { return createInfo.stageCount; }
    static VkShaderModule GetShaderModule(const VkRayTracingPipelineCreateInfoKHR &createInfo, uint32_t stage) {
        return createInfo.pStages[stage].module;
    }
    static void SetShaderModule(SafeType *createInfo, VkShaderModule shader_module, uint32_t stage) {
        createInfo->pStages[stage].module = shader_module;
    }
};

// Examine the pipelines to see if they use the debug descriptor set binding index.
// If any do, create new non-instrumented shader modules and use them to replace the instrumented
// shaders in the pipeline.  Return the (possibly) modified create infos to the caller.
template <typename CreateInfo, typename SafeCreateInfo, typename ObjectType>
void UtilPreCallRecordPipelineCreations(uint32_t count, const CreateInfo *pCreateInfos, const VkAllocationCallbacks *pAllocator,
                                        VkPipeline *pPipelines, std::vector<std::shared_ptr<PIPELINE_STATE>> &pipe_state,
                                        std::vector<SafeCreateInfo> *new_pipeline_create_infos,
                                        const VkPipelineBindPoint bind_point, ObjectType *object_ptr) {
    using Accessor = CreatePipelineTraits<CreateInfo>;
    if (bind_point != VK_PIPELINE_BIND_POINT_GRAPHICS && bind_point != VK_PIPELINE_BIND_POINT_COMPUTE &&
        bind_point != VK_PIPELINE_BIND_POINT_RAY_TRACING_NV) {
        return;
    }

    // Walk through all the pipelines, make a copy of each and flag each pipeline that contains a shader that uses the debug
    // descriptor set index.
    for (uint32_t pipeline = 0; pipeline < count; ++pipeline) {
        uint32_t stageCount = Accessor::GetStageCount(pCreateInfos[pipeline]);
        new_pipeline_create_infos->push_back(Accessor::GetPipelineCI(pipe_state[pipeline].get()));

        bool replace_shaders = false;
        if (pipe_state[pipeline]->active_slots.find(object_ptr->desc_set_bind_index) != pipe_state[pipeline]->active_slots.end()) {
            replace_shaders = true;
        }
        // If the app requests all available sets, the pipeline layout was not modified at pipeline layout creation and the already
        // instrumented shaders need to be replaced with uninstrumented shaders
        if (pipe_state[pipeline]->pipeline_layout->set_layouts.size() >= object_ptr->adjusted_max_desc_sets) {
            replace_shaders = true;
        }

        if (replace_shaders) {
            for (uint32_t stage = 0; stage < stageCount; ++stage) {
                const SHADER_MODULE_STATE *shader =
                    object_ptr->GetShaderModuleState(Accessor::GetShaderModule(pCreateInfos[pipeline], stage));

                VkShaderModuleCreateInfo create_info = {};
                VkShaderModule shader_module;
                create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
                create_info.pCode = shader->words.data();
                create_info.codeSize = shader->words.size() * sizeof(uint32_t);
                VkResult result = DispatchCreateShaderModule(object_ptr->device, &create_info, pAllocator, &shader_module);
                if (result == VK_SUCCESS) {
                    Accessor::SetShaderModule(&(*new_pipeline_create_infos)[pipeline], shader_module, stage);
                } else {
                    object_ptr->ReportSetupProblem(object_ptr->device,
                                                   "Unable to replace instrumented shader with non-instrumented one.  "
                                                   "Device could become unstable.");
                }
            }
        }
    }
}
// For every pipeline:
// - For every shader in a pipeline:
//   - If the shader had to be replaced in PreCallRecord (because the pipeline is using the debug desc set index):
//     - Destroy it since it has been bound into the pipeline by now.  This is our only chance to delete it.
//   - Track the shader in the shader_map
//   - Save the shader binary if it contains debug code
template <typename CreateInfo, typename ObjectType>
void UtilPostCallRecordPipelineCreations(const uint32_t count, const CreateInfo *pCreateInfos,
                                         const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                         const VkPipelineBindPoint bind_point, ObjectType *object_ptr) {
    using Accessor = CreatePipelineTraits<CreateInfo>;
    if (bind_point != VK_PIPELINE_BIND_POINT_GRAPHICS && bind_point != VK_PIPELINE_BIND_POINT_COMPUTE &&
        bind_point != VK_PIPELINE_BIND_POINT_RAY_TRACING_NV) {
        return;
    }
    for (uint32_t pipeline = 0; pipeline < count; ++pipeline) {
        auto pipeline_state = object_ptr->ValidationStateTracker::GetPipelineState(pPipelines[pipeline]);
        if (nullptr == pipeline_state) continue;

        uint32_t stageCount = 0;
        if (bind_point == VK_PIPELINE_BIND_POINT_GRAPHICS) {
            stageCount = pipeline_state->graphicsPipelineCI.stageCount;
        } else if (bind_point == VK_PIPELINE_BIND_POINT_COMPUTE) {
            stageCount = 1;
        } else if (bind_point == VK_PIPELINE_BIND_POINT_RAY_TRACING_NV) {
            stageCount = pipeline_state->raytracingPipelineCI.stageCount;
        } else {
            assert(false);
        }

        for (uint32_t stage = 0; stage < stageCount; ++stage) {
            if (pipeline_state->active_slots.find(object_ptr->desc_set_bind_index) != pipeline_state->active_slots.end()) {
                DispatchDestroyShaderModule(object_ptr->device, Accessor::GetShaderModule(pCreateInfos[pipeline], stage),
                                            pAllocator);
            }

            const SHADER_MODULE_STATE *shader_state = nullptr;
            if (bind_point == VK_PIPELINE_BIND_POINT_GRAPHICS) {
                shader_state = object_ptr->GetShaderModuleState(pipeline_state->graphicsPipelineCI.pStages[stage].module);
            } else if (bind_point == VK_PIPELINE_BIND_POINT_COMPUTE) {
                assert(stage == 0);
                shader_state = object_ptr->GetShaderModuleState(pipeline_state->computePipelineCI.stage.module);
            } else if (bind_point == VK_PIPELINE_BIND_POINT_RAY_TRACING_NV) {
                shader_state = object_ptr->GetShaderModuleState(pipeline_state->raytracingPipelineCI.pStages[stage].module);
            } else {
                assert(false);
            }

            std::vector<unsigned int> code;
            // Save the shader binary
            // The core_validation ShaderModule tracker saves the binary too, but discards it when the ShaderModule
            // is destroyed.  Applications may destroy ShaderModules after they are placed in a pipeline and before
            // the pipeline is used, so we have to keep another copy.
            if (shader_state && shader_state->has_valid_spirv) code = shader_state->words;

            object_ptr->shader_map[shader_state->gpu_validation_shader_id].pipeline = pipeline_state->pipeline;
            // Be careful to use the originally bound (instrumented) shader here, even if PreCallRecord had to back it
            // out with a non-instrumented shader.  The non-instrumented shader (found in pCreateInfo) was destroyed above.
            VkShaderModule shader_module = VK_NULL_HANDLE;
            if (bind_point == VK_PIPELINE_BIND_POINT_GRAPHICS) {
                shader_module = pipeline_state->graphicsPipelineCI.pStages[stage].module;
            } else if (bind_point == VK_PIPELINE_BIND_POINT_COMPUTE) {
                assert(stage == 0);
                shader_module = pipeline_state->computePipelineCI.stage.module;
            } else if (bind_point == VK_PIPELINE_BIND_POINT_RAY_TRACING_NV) {
                shader_module = pipeline_state->raytracingPipelineCI.pStages[stage].module;
            } else {
                assert(false);
            }
            object_ptr->shader_map[shader_state->gpu_validation_shader_id].shader_module = shader_module;
            object_ptr->shader_map[shader_state->gpu_validation_shader_id].pgm = std::move(code);
        }
    }
}
template <typename CreateInfos, typename SafeCreateInfos>
void UtilCopyCreatePipelineFeedbackData(const uint32_t count, CreateInfos *pCreateInfos, SafeCreateInfos *pSafeCreateInfos) {
    for (uint32_t i = 0; i < count; i++) {
        auto src_feedback_struct = LvlFindInChain<VkPipelineCreationFeedbackCreateInfoEXT>(pSafeCreateInfos[i].pNext);
        if (!src_feedback_struct) return;
        auto dst_feedback_struct = const_cast<VkPipelineCreationFeedbackCreateInfoEXT *>(
            LvlFindInChain<VkPipelineCreationFeedbackCreateInfoEXT>(pCreateInfos[i].pNext));
        *dst_feedback_struct->pPipelineCreationFeedback = *src_feedback_struct->pPipelineCreationFeedback;
        for (uint32_t j = 0; j < src_feedback_struct->pipelineStageCreationFeedbackCount; j++) {
            dst_feedback_struct->pPipelineStageCreationFeedbacks[j] = src_feedback_struct->pPipelineStageCreationFeedbacks[j];
        }
    }
}

template <typename ObjectType>
// For the given command buffer, map its debug data buffers and read their contents for analysis.
void UtilProcessInstrumentationBuffer(VkQueue queue, CMD_BUFFER_STATE *cb_node, ObjectType *object_ptr) {
    if (cb_node && (cb_node->hasDrawCmd || cb_node->hasTraceRaysCmd || cb_node->hasDispatchCmd)) {
        auto gpu_buffer_list = object_ptr->GetBufferInfo(cb_node->commandBuffer);
        uint32_t draw_index = 0;
        uint32_t compute_index = 0;
        uint32_t ray_trace_index = 0;

        for (auto &buffer_info : gpu_buffer_list) {
            char *pData;

            uint32_t operation_index = 0;
            if (buffer_info.pipeline_bind_point == VK_PIPELINE_BIND_POINT_GRAPHICS) {
                operation_index = draw_index;
            } else if (buffer_info.pipeline_bind_point == VK_PIPELINE_BIND_POINT_COMPUTE) {
                operation_index = compute_index;
            } else if (buffer_info.pipeline_bind_point == VK_PIPELINE_BIND_POINT_RAY_TRACING_NV) {
                operation_index = ray_trace_index;
            } else {
                assert(false);
            }

            VkResult result = vmaMapMemory(object_ptr->vmaAllocator, buffer_info.output_mem_block.allocation, (void **)&pData);
            if (result == VK_SUCCESS) {
                object_ptr->AnalyzeAndGenerateMessages(cb_node->commandBuffer, queue, buffer_info,
                                                       operation_index, (uint32_t *)pData);
                vmaUnmapMemory(object_ptr->vmaAllocator, buffer_info.output_mem_block.allocation);
            }

            if (buffer_info.pipeline_bind_point == VK_PIPELINE_BIND_POINT_GRAPHICS) {
                draw_index++;
            } else if (buffer_info.pipeline_bind_point == VK_PIPELINE_BIND_POINT_COMPUTE) {
                compute_index++;
            } else if (buffer_info.pipeline_bind_point == VK_PIPELINE_BIND_POINT_RAY_TRACING_NV) {
                ray_trace_index++;
            } else {
                assert(false);
            }
        }
    }
}
template <typename ObjectType>
// Submit a memory barrier on graphics queues.
// Lazy-create and record the needed command buffer.
void UtilSubmitBarrier(VkQueue queue, ObjectType *object_ptr) {
    auto queue_barrier_command_info_it = object_ptr->queue_barrier_command_infos.emplace(queue, UtilQueueBarrierCommandInfo{});
    if (queue_barrier_command_info_it.second) {
        UtilQueueBarrierCommandInfo &queue_barrier_command_info = queue_barrier_command_info_it.first->second;

        uint32_t queue_family_index = 0;

        auto queue_state_it = object_ptr->queueMap.find(queue);
        if (queue_state_it != object_ptr->queueMap.end()) {
            queue_family_index = queue_state_it->second.queueFamilyIndex;
        }

        VkResult result = VK_SUCCESS;

        VkCommandPoolCreateInfo pool_create_info = {};
        pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        pool_create_info.queueFamilyIndex = queue_family_index;
        result = DispatchCreateCommandPool(object_ptr->device, &pool_create_info, nullptr,
                                           &queue_barrier_command_info.barrier_command_pool);
        if (result != VK_SUCCESS) {
            object_ptr->ReportSetupProblem(object_ptr->device, "Unable to create command pool for barrier CB.");
            queue_barrier_command_info.barrier_command_pool = VK_NULL_HANDLE;
            return;
        }

        VkCommandBufferAllocateInfo buffer_alloc_info = {};
        buffer_alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        buffer_alloc_info.commandPool = queue_barrier_command_info.barrier_command_pool;
        buffer_alloc_info.commandBufferCount = 1;
        buffer_alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        result = DispatchAllocateCommandBuffers(object_ptr->device, &buffer_alloc_info,
                                                &queue_barrier_command_info.barrier_command_buffer);
        if (result != VK_SUCCESS) {
            object_ptr->ReportSetupProblem(object_ptr->device, "Unable to create barrier command buffer.");
            DispatchDestroyCommandPool(object_ptr->device, queue_barrier_command_info.barrier_command_pool, nullptr);
            queue_barrier_command_info.barrier_command_pool = VK_NULL_HANDLE;
            queue_barrier_command_info.barrier_command_buffer = VK_NULL_HANDLE;
            return;
        }

        // Hook up command buffer dispatch
        object_ptr->vkSetDeviceLoaderData(object_ptr->device, queue_barrier_command_info.barrier_command_buffer);

        // Record a global memory barrier to force availability of device memory operations to the host domain.
        VkCommandBufferBeginInfo command_buffer_begin_info = {};
        command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        result = DispatchBeginCommandBuffer(queue_barrier_command_info.barrier_command_buffer, &command_buffer_begin_info);
        if (result == VK_SUCCESS) {
            VkMemoryBarrier memory_barrier = {};
            memory_barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
            memory_barrier.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
            memory_barrier.dstAccessMask = VK_ACCESS_HOST_READ_BIT;

            DispatchCmdPipelineBarrier(queue_barrier_command_info.barrier_command_buffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                                       VK_PIPELINE_STAGE_HOST_BIT, 0, 1, &memory_barrier, 0, nullptr, 0, nullptr);
            DispatchEndCommandBuffer(queue_barrier_command_info.barrier_command_buffer);
        }
    }

    UtilQueueBarrierCommandInfo &queue_barrier_command_info = queue_barrier_command_info_it.first->second;
    if (queue_barrier_command_info.barrier_command_buffer != VK_NULL_HANDLE) {
        VkSubmitInfo submit_info = {};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &queue_barrier_command_info.barrier_command_buffer;
        DispatchQueueSubmit(queue, 1, &submit_info, VK_NULL_HANDLE);
    }
}
void UtilGenerateStageMessage(const uint32_t *debug_record, std::string &msg);
void UtilGenerateCommonMessage(const debug_report_data *report_data, const VkCommandBuffer commandBuffer,
                               const uint32_t *debug_record, const VkShaderModule shader_module_handle,
                               const VkPipeline pipeline_handle, const VkPipelineBindPoint pipeline_bind_point,
                               const uint32_t operation_index, std::string &msg);
void UtilGenerateSourceMessages(const std::vector<unsigned int> &pgm, const uint32_t *debug_record, bool from_printf,
                                std::string &filename_msg, std::string &source_msg);
