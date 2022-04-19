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

class CommandBuffer : public CMD_BUFFER_STATE {
  public:
    CommandBuffer(GpuAssistedBase *ga, VkCommandBuffer cb, const VkCommandBufferAllocateInfo *pCreateInfo,
                  const COMMAND_POOL_STATE *pool);

    virtual bool NeedsProcessing() const = 0;
    virtual void Process(VkQueue queue) = 0;
};
}  // namespace gpu_utils_state
VALSTATETRACK_DERIVED_STATE_OBJECT(VkQueue, gpu_utils_state::Queue, QUEUE_STATE);
VALSTATETRACK_DERIVED_STATE_OBJECT(VkCommandBuffer, gpu_utils_state::CommandBuffer, CMD_BUFFER_STATE);

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
    ReadLockGuard ReadLock() override;
    WriteLockGuard WriteLock() override;
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
    void PreCallRecordCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo *pCreateInfo,
                                           const VkAllocationCallbacks *pAllocator, VkPipelineLayout *pPipelineLayout,
                                           void *cpl_state_data) override;
    void PostCallRecordCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo *pCreateInfo,
                                            const VkAllocationCallbacks *pAllocator, VkPipelineLayout *pPipelineLayout,
                                            VkResult result) override;

    void PreCallRecordCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                              const VkGraphicsPipelineCreateInfo *pCreateInfos,
                                              const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                              void *cgpl_state_data) override;
    void PreCallRecordCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                             const VkComputePipelineCreateInfo *pCreateInfos,
                                             const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                             void *ccpl_state_data) override;
    void PreCallRecordCreateRayTracingPipelinesNV(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                  const VkRayTracingPipelineCreateInfoNV *pCreateInfos,
                                                  const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                  void *crtpl_state_data) override;
    void PreCallRecordCreateRayTracingPipelinesKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                   VkPipelineCache pipelineCache, uint32_t count,
                                                   const VkRayTracingPipelineCreateInfoKHR *pCreateInfos,
                                                   const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                   void *crtpl_state_data) override;
    void PostCallRecordCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                               const VkGraphicsPipelineCreateInfo *pCreateInfos,
                                               const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines, VkResult result,
                                               void *cgpl_state_data) override;
    void PostCallRecordCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                              const VkComputePipelineCreateInfo *pCreateInfos,
                                              const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines, VkResult result,
                                              void *ccpl_state_data) override;
    void PostCallRecordCreateRayTracingPipelinesNV(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                   const VkRayTracingPipelineCreateInfoNV *pCreateInfos,
                                                   const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines, VkResult result,
                                                   void *crtpl_state_data) override;
    void PostCallRecordCreateRayTracingPipelinesKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                    VkPipelineCache pipelineCache, uint32_t count,
                                                    const VkRayTracingPipelineCreateInfoKHR *pCreateInfos,
                                                    const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                    VkResult result, void *crtpl_state_data) override;
    void PreCallRecordDestroyPipeline(VkDevice device, VkPipeline pipeline, const VkAllocationCallbacks *pAllocator) override;

    template <typename T>
    void ReportSetupProblem(T object, const char *const specific_message, bool vma_fail = false) const {
        std::string logit = specific_message; 
        if (vma_fail) {
            char *stats_string;
            vmaBuildStatsString(vmaAllocator, &stats_string, false);
            logit += " VMA statistics = ";
            logit += stats_string;
            vmaFreeStatsString(vmaAllocator, stats_string);
        }
        LogError(object, setup_vuid, "Setup Error. Detail: (%s)", logit.c_str());
    }
    bool GpuGetOption(const char *option, bool default_value) {
        std::string option_string = getLayerOption(option);
        transform(option_string.begin(), option_string.end(), option_string.begin(), ::tolower);
        return !option_string.empty() ? !option_string.compare("true") : default_value;
    }

  protected:
    bool CommandBufferNeedsProcessing(VkCommandBuffer command_buffer) const;
    void ProcessCommandBuffer(VkQueue queue, VkCommandBuffer command_buffer);

    void SubmitBarrier(VkQueue queue) {
        auto queue_state = Get<gpu_utils_state::Queue>(queue);
        if (queue_state) {
            queue_state->SubmitBarrier();
        }
    }

    std::shared_ptr<QUEUE_STATE> CreateQueue(VkQueue q, uint32_t index, VkDeviceQueueCreateFlags flags) override {
        return std::static_pointer_cast<QUEUE_STATE>(std::make_shared<gpu_utils_state::Queue>(*this, q, index, flags));
    }

    template <typename CreateInfo, typename SafeCreateInfo>
    void PreCallRecordPipelineCreations(uint32_t count, const CreateInfo *pCreateInfos, const VkAllocationCallbacks *pAllocator,
                                        VkPipeline *pPipelines, std::vector<std::shared_ptr<PIPELINE_STATE>> &pipe_state,
                                        std::vector<SafeCreateInfo> *new_pipeline_create_infos,
                                        const VkPipelineBindPoint bind_point);
    template <typename CreateInfo>
    void PostCallRecordPipelineCreations(const uint32_t count, const CreateInfo *pCreateInfos,
                                         const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                         const VkPipelineBindPoint bind_point);

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
    vl_concurrent_unordered_map<uint32_t, GpuAssistedShaderTracker> shader_map;
    std::vector<VkDescriptorSetLayoutBinding> bindings_;
};

