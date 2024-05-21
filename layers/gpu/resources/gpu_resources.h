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

#pragma once

#include "containers/custom_containers.h"
#include "error_message/logging.h"
#include "generated/error_location_helper.h"
#include "vma/vma.h"

#include <vector>
#include <vulkan/vulkan.h>

namespace gpu {

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
    vvl::unordered_map<VkDescriptorPool, PoolTracker> desc_pool_map_;
    mutable std::mutex lock_;
};

struct DeviceMemoryBlock {
    VkBuffer buffer = VK_NULL_HANDLE;
    VmaAllocation allocation = VK_NULL_HANDLE;
    void Destroy(VmaAllocator allocator) {
        if (buffer != VK_NULL_HANDLE) {
            vmaDestroyBuffer(allocator, buffer, allocation);
            buffer = VK_NULL_HANDLE;
            allocation = VK_NULL_HANDLE;
        }
    }
    bool IsNull() { return buffer == VK_NULL_HANDLE; }
};

}  // namespace gpu

namespace gpuav {
class Validator;
struct DescBindingInfo;

class GpuResourcesManager {
  public:
    void AddPipeline(VkPipeline);

  private:
    std::vector<VkPipeline> pipelines_;
    std::vector<VkDescriptorPool> descriptor_pools_;
    std::vector<VkDescriptorSet> descriptor_sets_;
    std::vector<gpu::DeviceMemoryBlock> buffers_;
    std::vector<VkShaderModule> shader_modules_;
    std::vector<VkDescriptorSetLayout> descriptor_set_layouts_;
    std::vector<VkPipelineLayout> pipeline_layouts_;
    std::vector<VkShaderEXT> shader_objects_;
};

// Every recorded action command needs the validation resources listed in this function
// If adding validation for a new command reveals the need to allocate specific resources for it, create a new class that derives
// from this one
class CommandResources {
  public:
    virtual ~CommandResources() {}
    virtual void Destroy(Validator &validator);
    CommandResources() = default;
    CommandResources(const CommandResources &) = default;
    CommandResources &operator=(const CommandResources &) = default;

    // Return iff an error was logged
    bool LogValidationMessage(Validator &validator, VkQueue queue, VkCommandBuffer cmd_buffer, uint32_t *error_record,
                              const uint32_t operation_index, const LogObjectList &objlist);
    // Return iff an error was logged
    virtual bool LogCustomValidationMessage(Validator &validator, const uint32_t *error_record, const uint32_t operation_index,
                                            const LogObjectList &objlist) {
        return false;
    }

    // Used by gpu av inserted validation pipelines
    // ---
    vvl::Func command = vvl::Func::Empty;  // Should probably use Location instead
    // Draw/dispatch/trace rays index in cmd buffer. 0 for all other operations (TODO: maintain it correctly)
    uint32_t operation_index = 0;

    // Only used for shader instrumentation
    // ---
    bool uses_shader_object = false;  // Only used in error message logging, to select VUID
    VkDescriptorSet instrumentation_desc_set = VK_NULL_HANDLE;
    VkDescriptorPool instrumentation_desc_pool = VK_NULL_HANDLE;

    VkPipelineBindPoint pipeline_bind_point = VK_PIPELINE_BIND_POINT_MAX_ENUM;
    bool uses_robustness =
        false;  // Only used in AnalyseAndeGenerateMessages, to output using LogWarning instead of LogError. It needs to be removed

    // desc_binding list and index are only used to help generate an error message
    uint32_t desc_binding_index = vvl::kU32Max;
    std::vector<DescBindingInfo> *desc_binding_list = nullptr;
};

struct SharedValidationResources {
    virtual ~SharedValidationResources() {}
    virtual void Destroy(Validator &validator) = 0;
};

class PreDrawResources : public CommandResources {
  public:
    ~PreDrawResources() {}

    VkDescriptorPool desc_pool = VK_NULL_HANDLE;
    // Store a descriptor for the indirect buffer or count buffer
    VkDescriptorSet buffer_desc_set = VK_NULL_HANDLE;
    VkBuffer indirect_buffer = VK_NULL_HANDLE;
    VkDeviceSize indirect_buffer_offset = 0;
    uint32_t indirect_buffer_stride = 0;
    VkDeviceSize indirect_buffer_size = 0;
    static constexpr uint32_t push_constant_words = 11;
    bool emit_task_error = false;  // Used to decide between mesh error and task error

    void Destroy(Validator &validator) final;
    bool LogCustomValidationMessage(Validator &validator, const uint32_t *error_record, const uint32_t operation_index,
                                    const LogObjectList &objlist);

    struct SharedResources : SharedValidationResources {
        VkShaderModule shader_module = VK_NULL_HANDLE;
        VkDescriptorSetLayout ds_layout = VK_NULL_HANDLE;
        VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;
        vvl::concurrent_unordered_map<VkRenderPass, VkPipeline> renderpass_to_pipeline;
        VkShaderEXT shader_object = VK_NULL_HANDLE;

        void Destroy(Validator &validator);
    };
};

class PreDispatchResources : public CommandResources {
  public:
    ~PreDispatchResources() {}

    VkDescriptorPool desc_pool = VK_NULL_HANDLE;
    VkDescriptorSet indirect_buffer_desc_set = VK_NULL_HANDLE;
    VkBuffer indirect_buffer = VK_NULL_HANDLE;
    VkDeviceSize indirect_buffer_offset = 0;
    static constexpr uint32_t push_constant_words = 4;

    void Destroy(Validator &validator) final;
    bool LogCustomValidationMessage(Validator &validator, const uint32_t *error_record, const uint32_t operation_index,
                                    const LogObjectList &objlist);

    struct SharedResources : SharedValidationResources {
        VkDescriptorSetLayout ds_layout = VK_NULL_HANDLE;
        VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;
        VkPipeline pipeline = VK_NULL_HANDLE;
        VkShaderEXT shader_object = VK_NULL_HANDLE;

        void Destroy(Validator &validator);
    };
};

class PreTraceRaysResources : public CommandResources {
  public:
    ~PreTraceRaysResources() {}

    VkDeviceAddress indirect_data_address = 0;
    static constexpr uint32_t push_constant_words = 5;

    void Destroy(Validator &validator) final;
    bool LogCustomValidationMessage(Validator &validator, const uint32_t *error_record, const uint32_t operation_index,
                                    const LogObjectList &objlist);

    struct SharedResources : SharedValidationResources {
        VkDescriptorSetLayout ds_layout = VK_NULL_HANDLE;
        VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;
        VkPipeline pipeline = VK_NULL_HANDLE;
        VmaPool sbt_pool = VK_NULL_HANDLE;
        VkBuffer sbt_buffer = VK_NULL_HANDLE;
        VmaAllocation sbt_allocation = {};
        VkDeviceAddress sbt_address = 0;
        uint32_t shader_group_handle_size_aligned = 0;

        void Destroy(Validator &validator);
    };
};

class PreCopyBufferToImageResources : public CommandResources {
  public:
    ~PreCopyBufferToImageResources() {}

    VkDescriptorPool desc_pool = VK_NULL_HANDLE;
    VkDescriptorSet desc_set = VK_NULL_HANDLE;
    VkBuffer src_buffer = VK_NULL_HANDLE;

    // Buffer holding the copy regions obtained from pRegions
    VkBuffer copy_src_regions_buffer = VK_NULL_HANDLE;
    VmaAllocation copy_src_regions_allocation = VK_NULL_HANDLE;

    void Destroy(Validator &validator) final;
    bool LogCustomValidationMessage(Validator &validator, const uint32_t *error_record, const uint32_t operation_index,
                                    const LogObjectList &objlist);

    struct SharedResources : SharedValidationResources {
        VkDescriptorSetLayout ds_layout = VK_NULL_HANDLE;
        VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;
        VkPipeline pipeline = VK_NULL_HANDLE;
        VmaPool copy_regions_pool = VK_NULL_HANDLE;

        void Destroy(Validator &validator);
    };
};

}  // namespace gpuav
