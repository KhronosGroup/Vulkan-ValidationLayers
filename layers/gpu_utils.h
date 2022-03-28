/* Copyright (c) 2020-2022 The Khronos Group Inc.
 * Copyright (c) 2020-2022 Valve Corporation
 * Copyright (c) 2020-2022 LunarG, Inc.
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
#include "cmd_buffer_state.h"
#include "state_tracker.h"
#include "vk_mem_alloc.h"
#include "queue_state.h"

class GpuAssistedBase;

class UtilDescriptorSetManager {
  public:
    UtilDescriptorSetManager(VkDevice device, uint32_t numBindingsInSet);
    ~UtilDescriptorSetManager();

    VkResult GetDescriptorSet(VkDescriptorPool *desc_pool, VkDescriptorSetLayout ds_layout, VkDescriptorSet *desc_sets);
    VkResult GetDescriptorSets(uint32_t count, VkDescriptorPool *pool, VkDescriptorSetLayout ds_layout,
                               std::vector<VkDescriptorSet> *desc_sets);
    void PutBackDescriptorSet(VkDescriptorPool desc_pool, VkDescriptorSet desc_set);

  private:
    std::unique_lock<std::mutex> Lock() const { return std::unique_lock<std::mutex>(lock_); }

    static const uint32_t kItemsPerChunk = 512;
    struct PoolTracker {
        uint32_t size;
        uint32_t used;
    };
    VkDevice device;
    uint32_t numBindingsInSet;
    layer_data::unordered_map<VkDescriptorPool, struct PoolTracker> desc_pool_map_;
    mutable std::mutex lock_;
};

namespace gpu_utils_state {
class Queue : public QUEUE_STATE {
  public:
    Queue(GpuAssistedBase &state, VkQueue q, uint32_t index, VkDeviceQueueCreateFlags flags);
    virtual ~Queue();
    void SubmitBarrier();

  private:
    GpuAssistedBase &state_;
    VkCommandPool barrier_command_pool_{VK_NULL_HANDLE};
    VkCommandBuffer barrier_command_buffer_{VK_NULL_HANDLE};
};
}  // namespace gpu_utils_state
VALSTATETRACK_DERIVED_STATE_OBJECT(VkQueue, gpu_utils_state::Queue, QUEUE_STATE);

VkResult UtilInitializeVma(VkPhysicalDevice physical_device, VkDevice device, VmaAllocator *pAllocator);
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
    static const SafeType &GetPipelineCI(const PIPELINE_STATE *pipeline_state) {
        return pipeline_state->GetUnifiedCreateInfo().graphics;
    }
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
    static const SafeType &GetPipelineCI(const PIPELINE_STATE *pipeline_state) {
        return pipeline_state->GetUnifiedCreateInfo().compute;
    }
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
    static const SafeType &GetPipelineCI(const PIPELINE_STATE *pipeline_state) {
        return pipeline_state->GetUnifiedCreateInfo().raytracing;
    }
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
    static const SafeType &GetPipelineCI(const PIPELINE_STATE *pipeline_state) {
        return pipeline_state->GetUnifiedCreateInfo().raytracing;
    }
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
        const auto &pipe = pipe_state[pipeline];

        if (!pipe->IsGraphicsLibrary()) {
            bool replace_shaders = false;
            if (pipe->active_slots.find(object_ptr->desc_set_bind_index) != pipe->active_slots.end()) {
                replace_shaders = true;
            }
            // If the app requests all available sets, the pipeline layout was not modified at pipeline layout creation and the
            // already instrumented shaders need to be replaced with uninstrumented shaders
            const auto pipeline_layout = pipe->PipelineLayoutState();
            if (pipeline_layout->set_layouts.size() >= object_ptr->adjusted_max_desc_sets) {
                replace_shaders = true;
            }

            if (replace_shaders) {
                for (uint32_t stage = 0; stage < stageCount; ++stage) {
                    const auto module_state =
                        object_ptr->template Get<SHADER_MODULE_STATE>(Accessor::GetShaderModule(pCreateInfos[pipeline], stage));

                    VkShaderModule shader_module;
                    auto create_info = LvlInitStruct<VkShaderModuleCreateInfo>();
                    create_info.pCode = module_state->words.data();
                    create_info.codeSize = module_state->words.size() * sizeof(uint32_t);
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
}
// For every pipeline:
// - For every shader in a pipeline:
//   - If the shader had to be replaced in PreCallRecord (because the pipeline is using the debug desc set index):
//     - Destroy it since it has been bound into the pipeline by now.  This is our only chance to delete it.
//   - Track the shader in the shader_map
//   - Save the shader binary if it contains debug code
template <typename CreateInfo, typename ObjectType>
void UtilPostCallRecordPipelineCreations(const uint32_t count, const CreateInfo *pCreateInfos,
                                         const VkAllocationCallbacks *pAllocator, const VkPipeline *pPipelines,
                                         const VkPipelineBindPoint bind_point, ObjectType *object_ptr) {
    using Accessor = CreatePipelineTraits<CreateInfo>;
    if (bind_point != VK_PIPELINE_BIND_POINT_GRAPHICS && bind_point != VK_PIPELINE_BIND_POINT_COMPUTE &&
        bind_point != VK_PIPELINE_BIND_POINT_RAY_TRACING_NV) {
        return;
    }
    for (uint32_t pipeline = 0; pipeline < count; ++pipeline) {
        auto pipeline_state = object_ptr->template Get<PIPELINE_STATE>(pPipelines[pipeline]);
        if (!pipeline_state || pipeline_state->IsGraphicsLibrary()) continue;

        const uint32_t stageCount = static_cast<uint32_t>(pipeline_state->stage_state.size());
        assert(stageCount > 0);

        for (uint32_t stage = 0; stage < stageCount; ++stage) {
            if (pipeline_state->active_slots.find(object_ptr->desc_set_bind_index) != pipeline_state->active_slots.end()) {
                DispatchDestroyShaderModule(object_ptr->device, Accessor::GetShaderModule(pCreateInfos[pipeline], stage),
                                            pAllocator);
            }

            std::shared_ptr<const SHADER_MODULE_STATE> module_state;
            if (bind_point == VK_PIPELINE_BIND_POINT_GRAPHICS) {
                module_state = object_ptr->template Get<SHADER_MODULE_STATE>(
                    pipeline_state->GetUnifiedCreateInfo().graphics.pStages[stage].module);
            } else if (bind_point == VK_PIPELINE_BIND_POINT_COMPUTE) {
                assert(stage == 0);
                module_state =
                    object_ptr->template Get<SHADER_MODULE_STATE>(pipeline_state->GetUnifiedCreateInfo().compute.stage.module);
            } else if (bind_point == VK_PIPELINE_BIND_POINT_RAY_TRACING_NV) {
                module_state = object_ptr->template Get<SHADER_MODULE_STATE>(
                    pipeline_state->GetUnifiedCreateInfo().raytracing.pStages[stage].module);
            } else {
                assert(false);
            }

            std::vector<unsigned int> code;
            // Save the shader binary
            // The core_validation ShaderModule tracker saves the binary too, but discards it when the ShaderModule
            // is destroyed.  Applications may destroy ShaderModules after they are placed in a pipeline and before
            // the pipeline is used, so we have to keep another copy.
            if (module_state && module_state->has_valid_spirv) code = module_state->words;

            object_ptr->shader_map[module_state->gpu_validation_shader_id].pipeline = pipeline_state->pipeline();
            // Be careful to use the originally bound (instrumented) shader here, even if PreCallRecord had to back it
            // out with a non-instrumented shader.  The non-instrumented shader (found in pCreateInfo) was destroyed above.
            VkShaderModule shader_module = VK_NULL_HANDLE;
            if (bind_point == VK_PIPELINE_BIND_POINT_GRAPHICS) {
                shader_module = pipeline_state->GetUnifiedCreateInfo().graphics.pStages[stage].module;
            } else if (bind_point == VK_PIPELINE_BIND_POINT_COMPUTE) {
                assert(stage == 0);
                shader_module = pipeline_state->GetUnifiedCreateInfo().compute.stage.module;
            } else if (bind_point == VK_PIPELINE_BIND_POINT_RAY_TRACING_NV) {
                shader_module = pipeline_state->GetUnifiedCreateInfo().raytracing.pStages[stage].module;
            } else {
                assert(false);
            }
            object_ptr->shader_map[module_state->gpu_validation_shader_id].shader_module = shader_module;
            object_ptr->shader_map[module_state->gpu_validation_shader_id].pgm = std::move(code);
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

VkResult UtilInitializeVma(VkPhysicalDevice physical_device, VkDevice device, VmaAllocator *pAllocator);

void UtilGenerateStageMessage(const uint32_t *debug_record, std::string &msg);
void UtilGenerateCommonMessage(const debug_report_data *report_data, const VkCommandBuffer commandBuffer,
                               const uint32_t *debug_record, const VkShaderModule shader_module_handle,
                               const VkPipeline pipeline_handle, const VkPipelineBindPoint pipeline_bind_point,
                               const uint32_t operation_index, std::string &msg);
void UtilGenerateSourceMessages(const std::vector<uint32_t> &pgm, const uint32_t *debug_record, bool from_printf,
                                std::string &filename_msg, std::string &source_msg);

struct GpuAssistedShaderTracker {
    VkPipeline pipeline;
    VkShaderModule shader_module;
    std::vector<uint32_t> pgm;
};

class GpuAssistedBase : public ValidationStateTracker {
  public:
    void PreCallRecordCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo *pCreateInfo,
                                   const VkAllocationCallbacks *pAllocator, VkDevice *pDevice, void *modified_create_info) override;
    void CreateDevice(const VkDeviceCreateInfo *pCreateInfo) override;

    void PreCallRecordDestroyDevice(VkDevice device, const VkAllocationCallbacks *pAllocator) override;

    void PostCallRecordQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo *pSubmits, VkFence fence,
                                   VkResult result) override;
    void RecordQueueSubmit2(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2KHR *pSubmits, VkFence fence, VkResult result);
    void PostCallRecordQueueSubmit2KHR(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2KHR *pSubmits, VkFence fence,
                                       VkResult result) override;
    void PostCallRecordQueueSubmit2(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2 *pSubmits, VkFence fence,
                                    VkResult result) override;
    template <typename T>
    void ReportSetupProblem(T object, const char *const specific_message) const {
        LogError(object, setup_vuid, "Setup Error. Detail: (%s)", specific_message);
    }

  protected:
    virtual bool CommandBufferNeedsProcessing(VkCommandBuffer command_buffer) = 0;
    virtual void ProcessCommandBuffer(VkQueue queue, VkCommandBuffer command_buffer) = 0;

    void SubmitBarrier(VkQueue queue) {
        auto queue_state = Get<gpu_utils_state::Queue>(queue);
        if (queue_state) {
            queue_state->SubmitBarrier();
        }
    }

    std::shared_ptr<QUEUE_STATE> CreateQueue(VkQueue q, uint32_t index, VkDeviceQueueCreateFlags flags) override {
        return std::static_pointer_cast<QUEUE_STATE>(std::make_shared<gpu_utils_state::Queue>(*this, q, index, flags));
    }

  public:
    bool aborted = false;
    PFN_vkSetDeviceLoaderData vkSetDeviceLoaderData;
    const char *setup_vuid;
    VkPhysicalDeviceFeatures supported_features{};
    VkPhysicalDeviceFeatures desired_features{};
    uint32_t adjusted_max_desc_sets = 0;
    uint32_t unique_shader_module_id = 0;
    uint32_t output_buffer_size = 0;
    VkDescriptorSetLayout debug_desc_layout = VK_NULL_HANDLE;
    VkDescriptorSetLayout dummy_desc_layout = VK_NULL_HANDLE;
    uint32_t desc_set_bind_index = 0;
    VmaAllocator vmaAllocator = {};
    std::unique_ptr<UtilDescriptorSetManager> desc_set_manager;
    layer_data::unordered_map<uint32_t, GpuAssistedShaderTracker> shader_map;
    std::vector<VkDescriptorSetLayoutBinding> bindings_;
};

