/* Copyright (c) 2018-2025 The Khronos Group Inc.
 * Copyright (c) 2018-2025 Valve Corporation
 * Copyright (c) 2018-2025 LunarG, Inc.
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

#include <vector>

#include "external/inplace_function.h"
#include "gpuav/descriptor_validation/gpuav_descriptor_set.h"
#include "gpuav/resources/gpuav_vulkan_objects.h"

// We pull in most the core state tracking files
// gpuav_state_trackers.h should NOT be included by any other header file
#include "state_tracker/buffer_state.h"
#include "state_tracker/image_state.h"
#include "state_tracker/cmd_buffer_state.h"
#include "state_tracker/queue_state.h"
#include "state_tracker/sampler_state.h"
#include "state_tracker/ray_tracing_state.h"
#include "state_tracker/shader_object_state.h"

namespace gpuav {

class Validator;
struct DescriptorBindingCommand;

struct DebugPrintfBufferInfo {
    vko::Buffer output_mem_buffer;
    VkPipelineBindPoint pipeline_bind_point;
    uint32_t action_command_index;
    DebugPrintfBufferInfo(vko::Buffer output_mem_buffer, VkPipelineBindPoint pipeline_bind_point, uint32_t action_command_index)
        : output_mem_buffer(output_mem_buffer),
          pipeline_bind_point(pipeline_bind_point),
          action_command_index(action_command_index){};
};

class CommandBufferSubState : public vvl::CommandBufferSubState {
  public:
    // One item per vkCmdBindDescriptorSet() called
    // We really need this information at the draw/dispatch/action, but assume most apps will either
    //  1. Update once and only change pipelines, so this information doesn't change between draws
    //  2. Change once per draw, then this would be the same length as doing this per-draw
    //
    // Note: If the app calls vkCmdBindDescriptorSet 10 times to set descriptor set [0, 9] one at a time instead of setting [0, 9]
    // in a single vkCmdBindDescriptorSet call then this will allocate a lot of redundant memory
    std::vector<DescriptorBindingCommand> descriptor_binding_commands;

    // Buffer to be bound every draw/dispatch/action
    VkBuffer descriptor_indexing_buffer = VK_NULL_HANDLE;
    VkBuffer post_process_buffer_lut = VK_NULL_HANDLE;

    // Used to track which spot in the command buffer the error came from
    bool max_actions_cmd_validation_reached_ = false;
    uint32_t draw_index = 0;
    uint32_t compute_index = 0;
    uint32_t trace_rays_index = 0;
    uint32_t action_command_count = 0;

    CommandBufferSubState(Validator &gpuav, vvl::CommandBuffer &cb);
    ~CommandBufferSubState();

    [[nodiscard]] bool PreProcess(const Location &loc);
    void PostProcess(VkQueue queue, const std::vector<std::string> &initial_label_stack, const Location &loc);
    struct LabelLogging {
        const std::vector<std::string> &initial_label_stack;
        const vvl::unordered_map<uint32_t, uint32_t> &action_cmd_i_to_label_cmd_i_map;
    };
    [[nodiscard]] bool ValidateBindlessDescriptorSets(const Location &loc, const LabelLogging &label_logging);

    const VkDescriptorSetLayout &GetInstrumentationDescriptorSetLayout() const {
        assert(instrumentation_desc_set_layout_ != VK_NULL_HANDLE);
        return instrumentation_desc_set_layout_;
    }

    // Bindings: {error output buffer}
    const VkDescriptorSet &GetErrorLoggingDescSet() const {
        assert(error_logging_desc_set_ != VK_NULL_HANDLE);
        return error_logging_desc_set_;
    }

    const VkDescriptorSetLayout &GetErrorLoggingDescSetLayout() const {
        assert(error_logging_desc_set_layout_ != VK_NULL_HANDLE);
        return error_logging_desc_set_layout_;
    }

    uint32_t GetValidationErrorBufferDescSetIndex() const { return 0; }

    const vko::BufferRange &GetErrorOutputBufferRange() const {
        assert(error_output_buffer_range_.buffer != VK_NULL_HANDLE);
        return error_output_buffer_range_;
    }

    VkDeviceSize GetCmdErrorsCountsBufferByteSize() const { return 8192 * sizeof(uint32_t); }

    const VkBuffer &GetCmdErrorsCountsBuffer() const {
        assert(cmd_errors_counts_buffer_.VkHandle() != VK_NULL_HANDLE);
        return cmd_errors_counts_buffer_.VkHandle();
    }

    const vko::Buffer &GetBdaRangesSnapshot() const { return bda_ranges_snapshot_; }

    void IncrementCommandCount(Validator &gpuav, VkPipelineBindPoint bind_point, const Location &loc);

    std::string GetDebugLabelRegion(uint32_t label_command_i, const std::vector<std::string> &initial_label_stack) const;

    void Destroy() final;
    void Reset(const Location &loc) final;

    vko::GpuResourcesManager gpu_resources_manager;
    // Using stdext::inplace_function over std::function to allocate memory in place
    using ErrorLoggerFunc =
        stdext::inplace_function<bool(const uint32_t *error_record, const LogObjectList &objlist,
                                      const std::vector<std::string> &initial_label_stack),
                                 280 /*lambda storage size (bytes), large enough to store biggest error lambda*/>;
    std::vector<ErrorLoggerFunc> per_command_error_loggers;
    vvl::unordered_map<uint32_t, uint32_t> action_cmd_i_to_label_cmd_i_map;

    using ValidationCommandFunc = stdext::inplace_function<void(Validator &gpuav, CommandBufferSubState &cb_state), 192>;

    std::vector<ValidationCommandFunc> per_render_pass_validation_commands;

    std::vector<DebugPrintfBufferInfo> debug_printf_buffer_infos;

  private:
    void AllocateResources(const Location &loc);
    void ResetCBState(bool should_destroy);
    bool NeedsPostProcess();

    VkDeviceSize GetBdaRangesBufferByteSize() const;
    [[nodiscard]] bool UpdateBdaRangesBuffer(const Location &loc);

    Validator &gpuav_;
    VkDescriptorSetLayout instrumentation_desc_set_layout_ = VK_NULL_HANDLE;

    VkDescriptorSetLayout error_logging_desc_set_layout_ = VK_NULL_HANDLE;
    VkDescriptorSet error_logging_desc_set_ = VK_NULL_HANDLE;
    VkDescriptorPool validation_cmd_desc_pool_ = VK_NULL_HANDLE;

    // Buffer storing GPU-AV errors
    vko::BufferRange error_output_buffer_range_;
    // Buffer storing an error count per validated commands.
    // Used to limit the number of errors a single command can emit.
    vko::Buffer cmd_errors_counts_buffer_;
    // Buffer storing a snapshot of buffer device address ranges
    vko::Buffer bda_ranges_snapshot_;
    uint32_t bda_ranges_snapshot_version_ = 0;
};

static inline CommandBufferSubState &SubState(vvl::CommandBuffer &cb) {
    return *static_cast<CommandBufferSubState *>(cb.SubState(LayerObjectTypeGpuAssisted));
}

class QueueSubState : public vvl::QueueSubState {
  public:
    QueueSubState(Validator &gpuav, vvl::Queue &q);
    virtual ~QueueSubState();

    void PreSubmit(std::vector<vvl::QueueSubmission> &submissions) override;
    void PostSubmit(vvl::QueueSubmission &) override;
    void Retire(vvl::QueueSubmission &) override;

  protected:
    void SubmitBarrier(const Location &loc, uint64_t seq);

    Validator &gpuav_;
    VkCommandPool barrier_command_pool_{VK_NULL_HANDLE};
    VkCommandBuffer barrier_command_buffer_{VK_NULL_HANDLE};
    VkSemaphore barrier_sem_{VK_NULL_HANDLE};
    std::deque<std::vector<vvl::CommandBufferSubmission>> retiring_;
    const bool timeline_khr_;
};

// Descriptor Ids are used on the GPU to identify if a given descriptor is valid.
// In some applications there are very large bindless descriptor arrays where it isn't feasible to track validity
// via the StateObject::parent_nodes_ map as usual. Instead, these ids are stored in a giant GPU accessible bitmap
// so that the instrumentation can decide if a descriptor is actually valid when it is used in a shader.
class DescriptorIdTracker {
  public:
    DescriptorIdTracker(DescriptorHeap &heap_, VulkanTypedHandle handle) : heap(heap_), id(heap_.NextId(handle)) {}

    DescriptorIdTracker(const DescriptorIdTracker &) = delete;
    DescriptorIdTracker &operator=(const DescriptorIdTracker &) = delete;

    ~DescriptorIdTracker() { heap.DeleteId(id); }

    DescriptorHeap &heap;
    const DescriptorId id{};
};

class ImageSubState : public vvl::ImageSubState {
  public:
    ImageSubState(vvl::Image &obj, DescriptorHeap &heap);
    void Destroy() override;
    void NotifyInvalidate(const vvl::StateObject::NodeList &invalid_nodes, bool unlink) override;

    DescriptorId Id() const { return id_tracker ? id_tracker->id : 0; }
    std::optional<DescriptorIdTracker> id_tracker;
};
static inline ImageSubState &SubState(vvl::Image &obj) {
    return *static_cast<ImageSubState *>(obj.SubState(LayerObjectTypeGpuAssisted));
}
static inline const ImageSubState &SubState(const vvl::Image &obj) {
    return *static_cast<const ImageSubState *>(obj.SubState(LayerObjectTypeGpuAssisted));
}

class ImageViewSubState : public vvl::ImageViewSubState {
  public:
    ImageViewSubState(vvl::ImageView &obj, DescriptorHeap &heap);
    void Destroy() override;
    void NotifyInvalidate(const vvl::StateObject::NodeList &invalid_nodes, bool unlink) override;

    DescriptorId Id() const { return id_tracker ? id_tracker->id : 0; }
    std::optional<DescriptorIdTracker> id_tracker;
};
static inline ImageViewSubState &SubState(vvl::ImageView &obj) {
    return *static_cast<ImageViewSubState *>(obj.SubState(LayerObjectTypeGpuAssisted));
}
static inline const ImageViewSubState &SubState(const vvl::ImageView &obj) {
    return *static_cast<const ImageViewSubState *>(obj.SubState(LayerObjectTypeGpuAssisted));
}

class BufferSubState : public vvl::BufferSubState {
  public:
    BufferSubState(vvl::Buffer &obj, DescriptorHeap &heap);
    void Destroy() override;
    void NotifyInvalidate(const vvl::StateObject::NodeList &invalid_nodes, bool unlink) override;

    DescriptorId Id() const { return id_tracker ? id_tracker->id : 0; }
    std::optional<DescriptorIdTracker> id_tracker;
};
static inline BufferSubState &SubState(vvl::Buffer &obj) {
    return *static_cast<BufferSubState *>(obj.SubState(LayerObjectTypeGpuAssisted));
}
static inline const BufferSubState &SubState(const vvl::Buffer &obj) {
    return *static_cast<const BufferSubState *>(obj.SubState(LayerObjectTypeGpuAssisted));
}

class BufferViewSubState : public vvl::BufferViewSubState {
  public:
    BufferViewSubState(vvl::BufferView &obj, DescriptorHeap &heap);
    void Destroy() override;
    void NotifyInvalidate(const vvl::StateObject::NodeList &invalid_nodes, bool unlink) override;

    DescriptorId Id() const { return id_tracker ? id_tracker->id : 0; }
    std::optional<DescriptorIdTracker> id_tracker;
};
static inline BufferViewSubState &SubState(vvl::BufferView &obj) {
    return *static_cast<BufferViewSubState *>(obj.SubState(LayerObjectTypeGpuAssisted));
}
static inline const BufferViewSubState &SubState(const vvl::BufferView &obj) {
    return *static_cast<const BufferViewSubState *>(obj.SubState(LayerObjectTypeGpuAssisted));
}

class SamplerSubState : public vvl::SamplerSubState {
  public:
    SamplerSubState(vvl::Sampler &obj, DescriptorHeap &heap);
    void Destroy() override;
    void NotifyInvalidate(const vvl::StateObject::NodeList &invalid_nodes, bool unlink) override;

    DescriptorId Id() const { return id_tracker ? id_tracker->id : 0; }
    std::optional<DescriptorIdTracker> id_tracker;
};
static inline SamplerSubState &SubState(vvl::Sampler &obj) {
    return *static_cast<SamplerSubState *>(obj.SubState(LayerObjectTypeGpuAssisted));
}
static inline const SamplerSubState &SubState(const vvl::Sampler &obj) {
    return *static_cast<const SamplerSubState *>(obj.SubState(LayerObjectTypeGpuAssisted));
}

class AccelerationStructureNVSubState : public vvl::AccelerationStructureNVSubState {
  public:
    AccelerationStructureNVSubState(vvl::AccelerationStructureNV &obj, DescriptorHeap &heap);
    void Destroy() override;
    void NotifyInvalidate(const vvl::StateObject::NodeList &invalid_nodes, bool unlink) override;

    DescriptorId Id() const { return id_tracker ? id_tracker->id : 0; }
    std::optional<DescriptorIdTracker> id_tracker;
};
static inline AccelerationStructureNVSubState &SubState(vvl::AccelerationStructureNV &obj) {
    return *static_cast<AccelerationStructureNVSubState *>(obj.SubState(LayerObjectTypeGpuAssisted));
}
static inline const AccelerationStructureNVSubState &SubState(const vvl::AccelerationStructureNV &obj) {
    return *static_cast<const AccelerationStructureNVSubState *>(obj.SubState(LayerObjectTypeGpuAssisted));
}
class AccelerationStructureKHRSubState : public vvl::AccelerationStructureKHRSubState {
  public:
    AccelerationStructureKHRSubState(vvl::AccelerationStructureKHR &obj, DescriptorHeap &heap);
    void Destroy() override;
    void NotifyInvalidate(const vvl::StateObject::NodeList &invalid_nodes, bool unlink) override;

    DescriptorId Id() const { return id_tracker ? id_tracker->id : 0; }
    std::optional<DescriptorIdTracker> id_tracker;
};
static inline AccelerationStructureKHRSubState &SubState(vvl::AccelerationStructureKHR &obj) {
    return *static_cast<AccelerationStructureKHRSubState *>(obj.SubState(LayerObjectTypeGpuAssisted));
}
static inline const AccelerationStructureKHRSubState &SubState(const vvl::AccelerationStructureKHR &obj) {
    return *static_cast<const AccelerationStructureKHRSubState *>(obj.SubState(LayerObjectTypeGpuAssisted));
}

class ShaderObjectSubState : public vvl::ShaderObjectSubState {
  public:
    explicit ShaderObjectSubState(vvl::ShaderObject &obj);

    bool was_instrumented = false;
    uint32_t unique_shader_id = 0;
    // We need to keep incase the user calls vkGetShaderBinaryDataEXT
    vku::safe_VkShaderCreateInfoEXT original_create_info;
    VkShaderEXT original_handle = VK_NULL_HANDLE;
};

static inline ShaderObjectSubState &SubState(vvl::ShaderObject &obj) {
    return *static_cast<ShaderObjectSubState *>(obj.SubState(LayerObjectTypeGpuAssisted));
}
static inline const ShaderObjectSubState &SubState(const vvl::ShaderObject &obj) {
    return *static_cast<const ShaderObjectSubState *>(obj.SubState(LayerObjectTypeGpuAssisted));
}

}  // namespace gpuav
