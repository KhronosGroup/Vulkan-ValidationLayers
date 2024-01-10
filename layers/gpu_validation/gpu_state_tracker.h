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
#pragma once
#include "generated/chassis.h"
#include "state_tracker/cmd_buffer_state.h"
#include "vma/vma.h"

namespace gpu_tracker {

class Validator;

class Queue : public vvl::Queue {
  public:
    Queue(Validator &state, VkQueue q, uint32_t index, VkDeviceQueueCreateFlags flags,
          const VkQueueFamilyProperties &queueFamilyProperties);
    virtual ~Queue();
    void SubmitBarrier();

  private:
    Validator &state_;
    VkCommandPool barrier_command_pool_{VK_NULL_HANDLE};
    VkCommandBuffer barrier_command_buffer_{VK_NULL_HANDLE};
};

class CommandBuffer : public vvl::CommandBuffer {
  public:
    CommandBuffer(Validator *ga, VkCommandBuffer cb, const VkCommandBufferAllocateInfo *pCreateInfo, const vvl::CommandPool *pool);

    virtual bool NeedsProcessing() const = 0;
    virtual void Process(VkQueue queue, const Location &loc) = 0;
};
}  // namespace gpu_tracker

VALSTATETRACK_DERIVED_STATE_OBJECT(VkQueue, gpu_tracker::Queue, vvl::Queue)
VALSTATETRACK_DERIVED_STATE_OBJECT(VkCommandBuffer, gpu_tracker::CommandBuffer, vvl::CommandBuffer)

namespace gpu_tracker {

static const VkShaderStageFlags kShaderStageAllRayTracing =
    VK_SHADER_STAGE_ANY_HIT_BIT_KHR | VK_SHADER_STAGE_CALLABLE_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR |
    VK_SHADER_STAGE_INTERSECTION_BIT_KHR | VK_SHADER_STAGE_MISS_BIT_KHR | VK_SHADER_STAGE_RAYGEN_BIT_KHR;

class DescriptorSetManager {
  public:
    DescriptorSetManager(VkDevice device, uint32_t num_bindings_in_set);
    ~DescriptorSetManager();

    VkResult GetDescriptorSet(VkDescriptorPool *out_desc_pool, VkDescriptorSetLayout ds_layout, VkDescriptorSet *out_desc_sets);
    VkResult GetDescriptorSets(uint32_t count, VkDescriptorPool *out_pool, VkDescriptorSetLayout ds_layout,
                               std::vector<VkDescriptorSet> *out_desc_sets);
    void PutBackDescriptorSet(VkDescriptorPool desc_pool, VkDescriptorSet desc_set);

  private:
    std::unique_lock<std::mutex> Lock() const { return std::unique_lock<std::mutex>(lock_); }

    static const uint32_t kItemsPerChunk = 512;
    struct PoolTracker {
        uint32_t size;
        uint32_t used;
    };
    VkDevice device;
    uint32_t num_bindings_in_set;
    vvl::unordered_map<VkDescriptorPool, struct PoolTracker> desc_pool_map_;
    mutable std::mutex lock_;
};

VkResult InitializeVma(VkInstance instance, VkPhysicalDevice physical_device, VkDevice device, bool use_buffer_device_address,
                       VmaAllocator *pAllocator);

struct GpuAssistedShaderTracker {
    VkPipeline pipeline;
    VkShaderModule shader_module;
    VkShaderEXT shader_object;
    std::vector<uint32_t> pgm;
};

class Validator : public ValidationStateTracker {
  public:
    using BaseClass = ValidationStateTracker;

    ReadLockGuard ReadLock() const override;
    WriteLockGuard WriteLock() override;
    void PreCallRecordCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo *pCreateInfo,
                                   const VkAllocationCallbacks *pAllocator, VkDevice *pDevice, const RecordObject &record_obj,
                                   void *modified_create_info) override;
    void CreateDevice(const VkDeviceCreateInfo *pCreateInfo) override;
    void PreCallRecordDestroyDevice(VkDevice device, const VkAllocationCallbacks *pAllocator,
                                    const RecordObject &record_obj) override;

    void PostCallRecordQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo *pSubmits, VkFence fence,
                                   const RecordObject &record_obj) override;
    void RecordQueueSubmit2(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2KHR *pSubmits, VkFence fence,
                            const RecordObject &record_obj);
    void PostCallRecordQueueSubmit2KHR(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2KHR *pSubmits, VkFence fence,
                                       const RecordObject &record_obj) override;
    void PostCallRecordQueueSubmit2(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2 *pSubmits, VkFence fence,
                                    const RecordObject &record_obj) override;
    bool ValidateCmdWaitEvents(VkCommandBuffer command_buffer, VkPipelineStageFlags2 src_stage_mask, const Location &loc) const;
    bool PreCallValidateCmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                      VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask,
                                      uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers,
                                      uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier *pBufferMemoryBarriers,
                                      uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier *pImageMemoryBarriers,
                                      const ErrorObject &error_obj) const override;
    bool PreCallValidateCmdWaitEvents2KHR(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                          const VkDependencyInfoKHR *pDependencyInfos, const ErrorObject &error_obj) const override;
    bool PreCallValidateCmdWaitEvents2(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                       const VkDependencyInfo *pDependencyInfos, const ErrorObject &error_obj) const override;
    void PreCallRecordCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo *pCreateInfo,
                                           const VkAllocationCallbacks *pAllocator, VkPipelineLayout *pPipelineLayout,
                                           const RecordObject &record_obj, void *cpl_state_data) override;
    void PostCallRecordCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo *pCreateInfo,
                                            const VkAllocationCallbacks *pAllocator, VkPipelineLayout *pPipelineLayout,
                                            const RecordObject &record_obj) override;
    void PreCallRecordCreateShadersEXT(VkDevice device, uint32_t createInfoCount, const VkShaderCreateInfoEXT *pCreateInfos,
                                       const VkAllocationCallbacks *pAllocator, VkShaderEXT *pShaders,
                                       const RecordObject &record_obj, void *csm_state_data) override;
    void PostCallRecordCreateShadersEXT(VkDevice device, uint32_t createInfoCount, const VkShaderCreateInfoEXT *pCreateInfos,
                                        const VkAllocationCallbacks *pAllocator, VkShaderEXT *pShaders,
                                        const RecordObject &record_obj, void *state_data) override;
    void PreCallRecordDestroyShaderEXT(VkDevice device, VkShaderEXT shader, const VkAllocationCallbacks *pAllocator,
                                       const RecordObject &record_obj) override;

    void PreCallRecordCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                              const VkGraphicsPipelineCreateInfo *pCreateInfos,
                                              const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                              const RecordObject &record_obj, void *cgpl_state_data) override;
    void PreCallRecordCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                             const VkComputePipelineCreateInfo *pCreateInfos,
                                             const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                             const RecordObject &record_obj, void *ccpl_state_data) override;
    void PreCallRecordCreateRayTracingPipelinesNV(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                  const VkRayTracingPipelineCreateInfoNV *pCreateInfos,
                                                  const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                  const RecordObject &record_obj, void *crtpl_state_data) override;
    void PreCallRecordCreateRayTracingPipelinesKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                   VkPipelineCache pipelineCache, uint32_t count,
                                                   const VkRayTracingPipelineCreateInfoKHR *pCreateInfos,
                                                   const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                   const RecordObject &record_obj, void *crtpl_state_data) override;
    void PostCallRecordCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                               const VkGraphicsPipelineCreateInfo *pCreateInfos,
                                               const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                               const RecordObject &record_obj, void *cgpl_state_data) override;
    void PostCallRecordCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                              const VkComputePipelineCreateInfo *pCreateInfos,
                                              const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                              const RecordObject &record_obj, void *ccpl_state_data) override;
    void PostCallRecordCreateRayTracingPipelinesNV(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                   const VkRayTracingPipelineCreateInfoNV *pCreateInfos,
                                                   const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                   const RecordObject &record_obj, void *crtpl_state_data) override;
    void PostCallRecordCreateRayTracingPipelinesKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                    VkPipelineCache pipelineCache, uint32_t count,
                                                    const VkRayTracingPipelineCreateInfoKHR *pCreateInfos,
                                                    const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                    const RecordObject &record_obj, void *crtpl_state_data) override;
    void PreCallRecordDestroyPipeline(VkDevice device, VkPipeline pipeline, const VkAllocationCallbacks *pAllocator,
                                      const RecordObject &record_obj) override;

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
        Location loc(vvl::Func::vkCreateDevice);
        LogError(setup_vuid, object, loc, "Setup Error. Detail: (%s)", logit.c_str());
    }
    bool CheckForGpuAvEnabled(const void *pNext);

  protected:
    bool CommandBufferNeedsProcessing(VkCommandBuffer command_buffer) const;
    void ProcessCommandBuffer(VkQueue queue, VkCommandBuffer command_buffer, const Location &loc);

    void SubmitBarrier(VkQueue queue) {
        auto queue_state = Get<Queue>(queue);
        if (queue_state) {
            queue_state->SubmitBarrier();
        }
    }

    std::shared_ptr<vvl::Queue> CreateQueue(VkQueue q, uint32_t index, VkDeviceQueueCreateFlags flags,
                                            const VkQueueFamilyProperties &queueFamilyProperties) override {
        return std::static_pointer_cast<vvl::Queue>(std::make_shared<Queue>(*this, q, index, flags, queueFamilyProperties));
    }

    template <typename CreateInfo, typename SafeCreateInfo, typename GPUAVState>
    void PreCallRecordPipelineCreations(uint32_t count, const CreateInfo *pCreateInfos, const VkAllocationCallbacks *pAllocator,
                                        VkPipeline *pPipelines, std::vector<std::shared_ptr<vvl::Pipeline>> &pipe_state,
                                        std::vector<SafeCreateInfo> *new_pipeline_create_infos,
                                        const VkPipelineBindPoint bind_point, const RecordObject &record_obj,
                                        GPUAVState &cgpl_state);
    template <typename CreateInfo, typename SafeCreateInfo>
    void PostCallRecordPipelineCreations(const uint32_t count, const CreateInfo *pCreateInfos,
                                         const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                         const VkPipelineBindPoint bind_point, const SafeCreateInfo &modified_create_infos);

    virtual bool InstrumentShader(const vvl::span<const uint32_t> &input, std::vector<uint32_t> &new_pgm, uint32_t unique_shader_id,
                                  const Location &loc) = 0;

  public:
    mutable bool aborted = false;
    bool force_buffer_device_address;
    vvl::unordered_map<uint32_t, std::pair<size_t, std::vector<uint32_t>>> instrumented_shaders;
    PFN_vkSetDeviceLoaderData vkSetDeviceLoaderData;
    const char *setup_vuid;
    VkPhysicalDeviceFeatures supported_features{};
    VkPhysicalDeviceFeatures desired_features{};
    uint32_t adjusted_max_desc_sets = 0;
    std::atomic<uint32_t> unique_shader_module_id = 1;  // zero represents no shader module found
    uint32_t output_buffer_size = 0;
    VkDescriptorSetLayout debug_desc_layout = VK_NULL_HANDLE;
    VkDescriptorSetLayout dummy_desc_layout = VK_NULL_HANDLE;
    VkPipelineLayout debug_pipeline_layout = VK_NULL_HANDLE;
    uint32_t desc_set_bind_index = 0;
    VmaAllocator vmaAllocator = {};
    VmaPool output_buffer_pool = VK_NULL_HANDLE;
    std::unique_ptr<DescriptorSetManager> desc_set_manager;
    vl_concurrent_unordered_map<uint32_t, GpuAssistedShaderTracker> shader_map;
    std::vector<VkDescriptorSetLayoutBinding> bindings_;
};

}  // namespace gpu_tracker
